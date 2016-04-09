#How To Set Up an OpenVPN Server on Ubuntu 14.04

* This is the OS running on the DigitalOcean droplet instance *

### OpenVPN Configuration

* apt-get update
* apt-get install openvpn easy-rsa
* gunzip -c /usr/share/doc/openvpn/examples/sample-config-files/server.conf.gz > /etc/openvpn/server.conf
* vim /etc/openvpn/server.conf
* Edit _dh dh1024.pem_ to _dh dh2048.pem_ - this will use 2048 bit keys now instead of 1024 bit
* Uncomment _push "redirect-gateway def1 bypass-dhcp"_ - passes on clients' web traffic
* Uncomment _push "dhcp-option DNS 208.67.222.222"_ and _push "dhcp-option DNS 208.67.220.220"_ - push OpenDNS to clients for DNS resolution
* Uncomment both _user nobody_ and _group nogroup_ - May not be required on Windows systems

### Packet Forwarding

* echo 1 > /proc/sys/net/ipv4/ip\_forward
* vim /etc/sysctl.conf
* Uncomment _net.ipv4.ip\_forward=1_

### UFW

* ufw allow ssh
* ufw allow 1194/udp
* vim /etc/default/ufw
* Edit _DEFAULT_FORWARD_POLICY="DROP"_ to _DEFAULT_FORWARD_POLICY="ACCEPT"_
* vim /etc/ufw/before.rules
* Add the following after the rules.before section:
> # START OPENVPN RULES
> # NAT table rules
> *nat
> :POSTROUTING ACCEPT [0:0] 
> # Allow traffic from OpenVPN client to eth0
> -A POSTROUTING -s 10.8.0.0/8 -o eth0 -j MASQUERADE
> COMMIT
> # END OPENVPN RULES
* ufw enable - Answer y
* ufw status

### Configuring the CA

* cp -r /usr/share/easy-rsa/ /etc/openvpn
* mkdir /etc/openvpn/easy-rsa/keys
* vim /etc/openvpn/easy-rsa/vars
* Edit the following as necessary (these are the default parameters and can still be changed on build):
> export KEY_COUNTRY="US"
> export KEY_PROVINCE="TX"
> export KEY_CITY="Dallas"
> export KEY_ORG="My Company Name"
> export KEY_EMAIL="sammy@example.com"
> export KEY_OU="MYOrganizationalUnit"
* Also ensure _export KEY_NAME="server"_ exists in the file
* openssl dhparam -out /etc/openvpn/dh2048.pem 2048

### Building the CA

* cd /etc/openvpn/easy-rsa
* . ./vars
* ./clean-all
* ./build-ca

### Generating a Certificate and Key - Server

* ./build-key-server server
* cp /etc/openvpn/easy-rsa/keys/{server.crt,server.key,ca.crt} /etc/openvpn
* service openvpn start
* service openvpn status

### Generating a Certificate and Key - Client

* cd /etc/openvpn/easy-rsa
* ./build-key client1
* cp /usr/share/doc/openvpn/examples/sample-config-files/client.conf /etc/openvpn/easy-rsa/keys/client.ovpn

Then simply copy _/etc/openvpn/easy-rsa/keys/client1.crt_, _/etc/openvpn/easy-rsa/keys/client1.key_, _/etc/openvpn/easy-rsa/keys/client.ovpn_ and _/etc/openvpn/ca.crt_ to the client device.

### Creating a Unified profile for VPN

** This modifies the _client.ovpn_ file to include the CA, certificate and key, so only one file needs to be passed around. **

* Open _client.ovpn_ and change _remote my-server-1 1194_ to read _remote *serverIPAddress* 1194_
* If not running on windows on the client, also uncomment _user nobody_ and _group nogroup_
* Comment out the _ca ca.crt_, _cert client.crt_ and _key client.key_ references so we can include them in the same file
* Then, copy the contents of the ca, cert and key into xml tags with the same names at the end of the file (i.e. <ca>.. CA's contents ..</ca>)

### Installing the Client Profile

* Install OpenVPN
* Copy the unified _.ovpn_ file into _C:\Program Files\OpenVPN\config\
* OpenVPN will automatically register this file and offer it as an available element to connect to
* Ensure you open OpenVPN as administrator

Other OpenVPN derived clients can be used instead (i.e. Tunnelblick for OSX, OpenVPN apps etc).
