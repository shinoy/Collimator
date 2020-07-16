/* 
 * File:   HardwareConfig.H
 * Author: Administrator
 *
 * Created on May 24, 2020, 10:35 AM
 */

#ifndef HARDWARECONFIG_H
#define	HARDWARECONFIG_H

#ifdef	__cplusplus
extern "C" {
#endif

// CONFIG1
#pragma config FEXTOSC = OFF    // External Oscillator mode selection bits (Oscillator not enabled)
#pragma config RSTOSC = HFINTPLL// Power-up default value for COSC bits (HFINTOSC with 2x PLL, with OSCFRQ = 16 MHz and CDIV = 1:1 (FOSC = 32 MHz))
#pragma config CLKOUTEN = OFF   // Clock Out Enable bit (CLKOUT function is disabled; i/o or oscillator function on OSC2)
#pragma config CSWEN = ON      // Clock Switch Enable bit (Writing to NOSC and NDIV is allowed)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit (FSCM timer enabled)

// CONFIG2
#pragma config MCLRE = ON       // Master Clear Enable bit (MCLR pin is Master Clear function)
#pragma config PWRTE = OFF       // Power-up Timer Enable bit (PWRT enabled)
#pragma config LPBOREN = OFF    // Low-Power BOR enable bit (ULPBOR disabled)
#pragma config BOREN = OFF      // Brown-out reset enable bits (Brown-out reset disabled)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (VBOR) set to 1.9V on LF, and 2.45V on F Devices)
#pragma config ZCD = OFF        // Zero-cross detect disable (Zero-cross detect circuit is disabled at POR.)
#pragma config PPS1WAY = ON     // Peripheral Pin Select one-way control (The PPSLOCK bit can be cleared and set only once in software)
#pragma config STVREN = OFF     // Stack Overflow/Underflow Reset Enable bit (Stack Overflow or Underflow will not cause a reset)

// CONFIG3
#pragma config WDTCPS = WDTCPS_31// WDT Period Select bits (Divider ratio 1:65536; software control of WDTPS)
#pragma config WDTE = OFF       // WDT operating mode (WDT Disabled, SWDTEN is ignored)
#pragma config WDTCWS = WDTCWS_7// WDT Window Select bits (window always open (100%); software control; keyed access not required)
#pragma config WDTCCS = SC      // WDT input clock selector (Software Control)

// CONFIG4
#pragma config BBSIZE = BB512   // Boot Block Size Selection bits (512 words boot block size)
#pragma config BBEN = OFF       // Boot Block Enable bit (Boot Block disabled)
#pragma config SAFEN = OFF      // SAF Enable bit (SAF disabled)
#pragma config WRTAPP = OFF     // Application Block Write Protection bit (Application Block not write protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot Block not write protected)
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration Register not write protected)
#pragma config WRTSAF = OFF     // Storage Area Flash Write Protection bit (SAF not write protected)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (High Voltage on MCLR/Vpp must be used for programming)

// CONFIG5
#pragma config CP = OFF         // UserNVM Program memory code protection bit (UserNVM code protection disabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>

/*********************** On-chip common definition  *********************/
#define _XTAL_FREQ 32000000
    
#define IN 1
#define OUT 0
    
#define DIGITAL 0
#define ANALOG 1
    
    
/*********************** All Peripheral Pins Definition *********************/
//internal NTC
#define IntNTCTRIS TRISAbits.TRISA0
#define IntNTC PORTAbits.RA0
#define IntNTCPinMode ANSELAbits.ANSA0
    
    
//Ext NTC
#define ExtNTCTRIS TRISAbits.TRISA1
#define ExtNTC PORTAbits.RA1
#define ExtNTCPinMode ANSELAbits.ANSA1

    
// the FAULT IRQ
#define FaultTRIS  TRISBbits.TRISB0
#define Fault  PORTBbits.RB0  // not use LAT here for input signal
#define FaultPinMode  ANSELBbits.ANSB0
    
// the lighting button
#define BtnTRIS  TRISAbits.TRISA7
#define Btn  PORTAbits.RA7  // not use LAT here for input signal
#define BtnPinMode  ANSELAbits.ANSA7
    
// the status/error indicator 
#define LedIndTRIS  TRISCbits.TRISC0
#define LedInd  LATCbits.LATC0
#define LedIndPinMode  ANSELCbits.ANSC0

    
// the lighting LED
#define LedTRIS  TRISCbits.TRISC2
#define Led  LATCbits.LATC2
#define LedPinMode  ANSELCbits.ANSC2


// the laser 
#define LaserTRIS  TRISCbits.TRISC5
#define Laser  LATCbits.LATC5
#define LaserPinMode   ANSELCbits.ANSC5

    
// the cooling FAN
#define FANTRIS  TRISCbits.TRISC7
#define FAN  LATCbits.LATC7
#define FANPinMode  ANSELCbits.ANSC7

#ifdef	__cplusplus
}
#endif

#endif	/* HARDWARECONFIG_H */

