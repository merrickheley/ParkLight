#ifndef CONFIG_H
#define	CONFIG_H

#pragma config FOSC = XT        // Oscillator Selection (XT : Medium power crystal on OSC1/OSC2)
#pragma config WDTE = SWDTEN    // WDT controlled by the SWDTEN bit in the WDTCON register  
#pragma config PWRTE = OFF      // Power-up Timer Enable (PWRT disabled)    
#pragma config MCLRE = OFF      // MCLR Pin Function Select (MCLR/VPP pin function is digital input)    
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)    
#pragma config CPD = OFF        // Data Memory Code Protection (Data memory code protection is disabled)    
#pragma config BOREN = OFF      // Brown-out Reset Enable (Brown-out Reset disabled)    
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)    
#pragma config IESO = ON        // Internal/External Switchover (Internal/External Switchover mode is enabled)    
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is enabled)    
 
// CONFIG2    
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)    
#pragma config PLLEN = OFF      // PLL Enable (4x PLL enabled)    
#pragma config STVREN = OFF     // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)    
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)    
#pragma config LVP = OFF        // Low-Voltage Programming Enable (High-voltage on MCLR/VPP must be used for programming)     

#endif	/* CONFIG_H */
