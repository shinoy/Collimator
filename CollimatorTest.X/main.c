/* 
 * File:   main.c
 * Author: Administrator
 *
 * Created on May 24, 2020, 10:35 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pic16f15354.h>
#include "HardwareConfig.H"

// Work Status 
enum Status
{
    Normal =0,
    Init = 1,
    Error = 2
};


// The reason of error status
enum ErrReason
{
    NoErr = 0,
    InitErr = 1,
    LEDHiTempErr = 2,
    BoardHiTempErr = 3,
    LEDDrvErr = 4
};

struct {
    unsigned int TSLR2;
    unsigned int TSHR2;
    unsigned int FVRA1X;
    unsigned int FVRA2X;
    unsigned int FVRA4X;
    unsigned int FVRC1X;
    unsigned int FVRC2X;
    unsigned int FVRC4X;
} DIA_Table;

#define TempChkInterval 5
/*
 * 
 */

unsigned long timerCounter = 0;
unsigned long lightingOffDelay = 230;
enum Status currentStatus;
enum ErrReason errReason;

//high 100C -> 0.86V  -> 216
//low  95C -> 1.03V   -> 258
const float upperLimit = 0.86;
const float restoreLevel = 1.03;

const float FanFBUpLimit = 1.2;
const float FanFBLowLimit = 0.5;

float intNTCV = 0;
float extNTCV = 0;


// read values in DIA fields 
unsigned int read_DIA (unsigned int Add)
{
    NVMCON1bits.NVMREGS = 0x01;
    NVMADRH = ((Add >> 8) & 0xFF);
    NVMADRL = (Add & 0xFF);
    NVMCON1bits.RD = 0x01;
    while (NVMCON1bits.RD == 1);
    return ((NVMDATH<<8)+NVMDATL);
    NVMCON1bits.NVMREGS = 0x00;
}

void initDIA(void){
    DIA_Table.TSLR2 = read_DIA(DIA_TSLR2);
    DIA_Table.TSHR2 = read_DIA(DIA_TSHR2);
    DIA_Table.FVRA1X = read_DIA(DIA_FVRA1X);
    DIA_Table.FVRA2X = read_DIA(DIA_FVRA2X);
    DIA_Table.FVRA4X = read_DIA(DIA_FVRA4X);
    DIA_Table.FVRC1X = read_DIA(DIA_FVRC1X);
    DIA_Table.FVRC2X = read_DIA(DIA_FVRC2X);
    DIA_Table.FVRC4X = read_DIA(DIA_FVRC4X);
}

// PWM output assignment
//void pinInit(void){
//    PPSLOCK = 0x55;
//    PPSLOCK = 0xAA;
//    PPSLOCK = 0x00;
//    
//    // PWM3 -> RC2 for LED dimming
//    RC2PPS = 0x0B;
//    
//    PPSLOCK = 0x55;
//    PPSLOCK = 0xAA;
//    PPSLOCK = 0x01;
//}


/*********     Use PWM for lighting, not used in manual model now   **************/
//void PWMInit(void){
//    // disable PWM output driver
//    //TRISCbits.TRISC2 = IN;
//    
//    TRISCbits.TRISC2 = IN;
//    // High level output
//    PWM3POL = 0;
//    
//    
//    // set timer2 used by PWM3
//    
//    T2CLKCONbits.CS = 1;
//    T2CONbits.CKPS = 1;
//    
//    T2CONbits.OUTPS = 0;
//    // sync mode
//    T2HLTbits.PSYNC = 1;
//    T2HLTbits.CKPOL = 0;
//    T2HLTbits.CKSYNC = 1;
//    T2HLTbits.MODE = 0;
//    
//    
//    // 50% duty cycle ratio, PWM Freq 250K Hz
//    PR2 = 15;
//    PWM3DC = (8*4) << 6;
//    
//    TMR2IF = 0;
//    T2CONbits.ON = 0;
//    T2CONbits.ON = 1;
//    //delay for timer2 start
//    __delay_ms(10);
//    PWM3EN = 1;
//}




/*********    Common startup/reset initialize   **************/
void init(void){
   currentStatus = Init;
   
  // enable global interrupt  
   GIE = 1;
   INTCONbits.PEIE = 1;
    
   //NTC I/O settings
   IntNTCTRIS = IN;
   IntNTCPinMode = ANALOG;
   
   ExtNTCTRIS = IN;
   ExtNTCPinMode = ANALOG;
   
   //FAN feedback I/O setting
   FANFBTRIS = IN;
   FANFBPinMode = ANALOG;
   
   // LED FAULT I/O setting
    FaultTRIS = IN;
    FaultPinMode = DIGITAL;
   
   
   // Btn I/O setting
    BtnTRIS = IN;
    BtnPinMode = DIGITAL;
    
    // Led indicator I/O setting
    LedIndPinMode = DIGITAL;
    LedInd = 0;
    LedIndTRIS = IN;
    
    
    // Lighting LED I/O setting
    Led = 0;
    LedPinMode = DIGITAL;
    LedTRIS = OUT;
    
    //Laser I/O setting
    Laser = 0;
    LaserPinMode = DIGITAL;
    LaserTRIS = OUT;
    
    //FAN I/O setting
    FAN = 0;
    FANPinMode = DIGITAL;
    FANTRIS = OUT;
    
    // JP1 & 2 setting
    JP1TRIS = IN;
    JP1PinMode = DIGITAL;
    
    JP2TRIS = IN;
    JP2PinMode = DIGITAL;
    
    
    // enable IOC interrupts 
 
    PIE0bits.IOCIE = 1;
    IOCAN7 = 1;
 
    
    // enable FVR and comparator 
    FVRCONbits.ADFVR = 3; // FVR channel 2
    FVRCONbits.CDAFVR = 3; // FVR channel 1
    FVREN = 1;
    while(!FVRRDY){
      // wait for FVR ready.
    }
  
    //ADC enable
    ADCON1bits.ADCS = 0x01;
    ADCON1bits.ADPREF = 0;
    ADCON0bits.CHS = 0x3F;
    ADFM = 1;
    ADCON0bits.ADON = 0;
    PIR1bits.ADIF = 0;
    PIE1bits.ADIE = 0;
    
    // auto-off delay setting
    if(JP1 == 0){
        if(JP2 == 0){
         lightingOffDelay = 460; // 60s delay
        } else 
        {
         lightingOffDelay = 307; // 40s delay
        }
    } else {
        if(JP2 ==0 ){
           lightingOffDelay = 383; //50s delay
        }
    }
    
    
    //auto-off lighting timer.
    //  20s -> auto off
     T0EN = 1;  // Timer0 enable
     TMR0IE = 0;   // Timer0 interrupt disable
     T016BIT = 0;  // 8bit mode
     T0CON1bits.T0ASYNC = 1; // Asynchronized mode
     T0CON1bits.T0CS = 5;  // 500 kHZ clock
     T0CON1bits.T0CKPS = 8;  // Prescaler 1:256
     
     
     
     initDIA();
     
     currentStatus = Normal;
  // PWMInit();
  
}

/*********   get internal/external NTC voltage to compare   **************/
float getNTCV(unsigned channel){

   // unsigned int totalOffset = 0;
    float VDD = 0;
    float voltage = 0;
    
    ADCON0bits.ADON = 1;
   
    // get VDD 
   
        ADCON0bits.CHS = 0x3F;
        __delay_us(100);
        ADCON0bits.GO = 1;
        while(ADCON0bits.GOnDONE){
            __delay_us(8);
        }
        VDD = ((float)1024/(float)ADRES)*DIA_Table.FVRA4X;
        //totalOffset += countOffset;
        
        ADCON0bits.CHS = channel;
       //  ADCON0bits.CHS = 0x3B;
        __delay_us(100);
        ADCON0bits.GO = 1;
        while(ADCON0bits.GOnDONE){
            __delay_us(8);
        }
        voltage = ((float)ADRES/(float)1023)*VDD*0.001;
   
    return voltage;
}




//
//float getNTCVBack(unsigned channel){
//
//  // to be done
//    unsigned int totalCount = 0;
//    unsigned int actualCount = 0;
//    unsigned int countOffset = 0;
//   // unsigned int totalOffset = 0;
//    float voltage = 0;
//    
//    ADCON0bits.ADON = 1;
//   
//    for(int i=0; i< 10; i++){
//        ADCON0bits.CHS = 0x3B;
//        __delay_us(100);
//        ADCON0bits.GO = 1;
//        while(ADCON0bits.GOnDONE){
//            __delay_us(8);
//        }
//        countOffset = ADRES;
//        //totalOffset += countOffset;
//        
//        ADCON0bits.CHS = channel;
//       //  ADCON0bits.CHS = 0x3B;
//        __delay_us(100);
//        ADCON0bits.GO = 1;
//        while(ADCON0bits.GOnDONE){
//            __delay_us(8);
//        }
//        totalCount += ADRES - countOffset;
//    }
//    
//    actualCount = totalCount / 10;
//    voltage = ((float)actualCount/1023)*DIA_Table.FVRA4X;
//    //voltage =  (float)(DIA_Table.FVRA4X/1023)*actualCount; // ((float)(actualCount*DIA_Table.FVRA4X))/1023;
//    return voltage;
//}


void HeartBeatFlush(void){
 
    LedIndTRIS = IN;
    LedInd = 0;
    __delay_ms(250);
    LedIndTRIS = OUT;
    __delay_ms(250);
    LedIndTRIS = IN;
}

void TestFlush(void)
{
    LedIndTRIS = IN;
    LedInd = 0;
    __delay_ms(50);
    LedIndTRIS = OUT;
    __delay_ms(50);
    LedIndTRIS = IN;
}

void ErrFlush(int count){
    LedIndTRIS = IN;
    __delay_ms(1000);
    LedInd = 1;
    for(int i=0;i<count; i++){
        LedIndTRIS = OUT;
        __delay_ms(300);
        LedIndTRIS = IN;
        __delay_ms(300);
    }
}


// Check LED and Board temperature
void temperatureCheck(void){
    intNTCV = getNTCV(0x00);
    extNTCV = getNTCV(0x01);
    if(currentStatus == Normal){
        if( intNTCV < upperLimit){
            currentStatus = Error;
            errReason = BoardHiTempErr;
        }
        if( extNTCV < upperLimit) {
            currentStatus = Error;
            errReason = LEDHiTempErr;
        }
    }
    else  if(currentStatus == Error){
        if(((intNTCV > restoreLevel) && (errReason == BoardHiTempErr)) || ((extNTCV > restoreLevel) && (errReason == LEDHiTempErr))){
            currentStatus = Normal;
            errReason = NoErr;
        }
    }
}


// Check LED Driver Fault
void ledFaultCheck(void){
    if( (currentStatus == Normal) && (Led == 1) && (Fault == 0)){
     
            currentStatus = Error;
            errReason = LEDDrvErr;
    }
    if((currentStatus == Error) && (errReason == LEDDrvErr) &&(Led == 1) && (Fault == 1)){
        currentStatus = Normal;
        errReason = NoErr;
    }
}

// Check FAN Fault, in this error, we only change the LED indicator, so it's also thought as normal status
bool FANFaultCheck(void){
    
    if(FAN == 0){
        return true;
    }
    float fanFBVoltage;
    fanFBVoltage = getNTCV(0x02);
    if( (fanFBVoltage > FanFBUpLimit) || (fanFBVoltage < FanFBLowLimit)){
        ErrFlush(1);
        return false;
    } else {
        return true;
    }
}



// handle error status, restore system after error gone
void ErrHanler(){
     
    if(currentStatus == Error){
    
        switch(errReason)
        {
            case InitErr:
                 LedInd = 0;
                 LedIndTRIS = OUT;
                 break;
            
            case LEDHiTempErr:
                ErrFlush(2);
                temperatureCheck();
                break;
            case BoardHiTempErr:
                 ErrFlush(3);
                temperatureCheck();
                 break;
            case LEDDrvErr:
                 ErrFlush(4);
                 ledFaultCheck();
            break;
            default:
                 ErrFlush(5);
            break;
        }
    }

}



/*********************          interrupt handler              ****************/
void __interrupt() int_handler(){
    
    // IOC handler, we enable 
    if(PIE0bits.IOCIE && IOCAF7 ){
         IOCAF7 = 0;
         if(currentStatus == Normal){
         
            __delay_ms(150);
            if(PORTAbits.RA7 == 0){
             
                Led = ~Led;
                Laser = ~Laser;
                       
                // if restore from error status, FAN is still running, need to decide by Led status
                if(Led == 1){
                    FAN = 1;
                }else {
                    FAN = 0;
                }
            } // end of RA7 == 0

                               
          //start timer0 to auto lighting off
            TMR0H = 0xFD;
            TMR0L = 0;
            TMR0IF = 0;
            timerCounter = 0;
            TMR0IE = 1;
         }     
    }
    
    // Timer0 handler
    if(TMR0IE && TMR0IF){
        
        TMR0IF = 0;  // clear timer0 flag
        // we only need to turn off everything during normal status
        if(currentStatus == Normal){
        
            if(timerCounter <= lightingOffDelay){
            
                timerCounter++; // increment if no timeout
            }
            else{
                TMR0IE = 0;   // timeout, disable timer0 interrupt
                Led = 0;
                Laser = 0;
                FAN = 0;
          
            }
        }
    }
    
    // AD interrupt handler
    if(PIE1bits.ADIE && PIR1bits.ADIF){
       
       // we don't use AD interrupt to handle result now
    }
}



int main(int argc, char** argv) {
    //start to initialize peripherals
    init();
    int checkCount = 0;
    
    while(1){
        switch(currentStatus)
        {
            case Normal:
                if(FANFaultCheck()){
                  HeartBeatFlush();
                }
               
                // we shutdown FAN when no high temperature error and Led off
                if(Led == 0){
                    FAN = 0;
                }
                if(checkCount >= TempChkInterval){
                  temperatureCheck();
                  checkCount = 0;
                } else {
                    checkCount ++;
                }
               ledFaultCheck();
               
              
            break;
  
            case Error:
                // turn off LED and Laser, start up FAN for cooling. 
                if((errReason == BoardHiTempErr) || (errReason == LEDHiTempErr))
                {
                  Led = 0;
                  Laser = 0;
                  FAN = 1;
                }
               ErrHanler();
            break;
            
            default:
                
            break;
        
        }
        
        
        
//        TRISCbits.TRISC0 = OUT;
//        PORTCbits.RC0 = 1;
//        __delay_ms(500);
//        TRISCbits.TRISC0 = IN;
//      //  PORTCbits.RC0 = 0;
//        __delay_ms(500);
   }

    return (EXIT_SUCCESS);
}

