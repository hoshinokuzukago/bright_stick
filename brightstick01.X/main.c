/* 
 * File:   main.c
 * Author: aquarium
 *
 * Created on 2014/11/23, 21:48
 */

/* illumi Mode 2015/09/22 */
#include <stdio.h>
#include <stdlib.h>
#include <xc.h>



#define _XTAL_FREQ 4000000
#define LED_RED     LATCbits.LATC5
#define LED_GREEN   LATCbits.LATC7
#define LED_BLUE    LATCbits.LATC6

#define MTX_OUT1    LATCbits.LATC0
#define MTX_OUT2    LATCbits.LATC1
#define MTX_OUT3    LATCbits.LATC2
#define MTX_IN1     LATBbits.LATB7
#define MTX_IN2     LATBbits.LATB6
#define MTX_IN3     LATBbits.LATB5
#define PORTB_IO    0b11111111
#define PORTC_IO    0b00011000

#define MTXPORT     PORTB
#define MTX_MASK    0b11100000
#define MTX_LEFT    0b11000000
#define MTX_CENTOR  0b10100000
#define MTX_RIGHT   0b01100000


#define LED_ON  1
#define LED_OFF 0

#define INDEX_RED   0
#define INDEX_GREEN 1
#define INDEX_BLUE  2

#define KEY_TIMEUP      54          // key check span
#define LIGHT_UP        20          // LED Light turn on Span
#define KEY_PUSH_CNT    3           // KEy Push Count
#define ILLUMI_MOVE_CHG  500        // Illumi mode move timeup
#define ILLUMI_MOVE_STAY 10         // Illumi mode move stayColor
#define ILLUMI_FLASH_CHG 5000       // Illumi mode flash timeup 

#define KEYNO_NONE          0x00    // None
#define KEYNO_F1            0x01    // F1
#define KEYNO_F2            0x02    // F2 Select Color
#define KEYNO_F3            0x03    // F3
#define KEYNO_RED_UP        0x04    // Color Red Bright
#define KEYNO_RED_DOWN      0x05    // Color Red Dark
#define KEYNO_GREEN_UP      0x06    // Color Green Bright
#define KEYNO_GREEN_DOWN    0x07    // Color Green Dark
#define KEYNO_BLUE_UP       0x08    // Color Blue Bright
#define KEYNO_BLUE_DOWN     0x09    // Color Blue Dark


#pragma config FOSC = IRC        //  inner timer
#pragma config USBDIV = OFF, CPUDIV = NOCLKDIV
#pragma config IESO  = OFF, FCMEN = OFF, PLLEN  = ON
#pragma config BORV  = 30,  BOREN = OFF, PWRTEN = OFF
#pragma config WDTPS = 32768, WDTEN = OFF
#pragma config MCLRE = OFF, HFOFST = OFF, XINST  = OFF
#pragma config BBSIZ = OFF, LVP    = OFF, STVREN = ON
#pragma config CP1  = OFF, CP0  = OFF, CPD  = OFF, CPB  = OFF
#pragma config WRT1 = OFF, WRT0 = OFF, WRTB = OFF, WRTC = OFF
#pragma config EBTR1 = OFF, EBTR0 = OFF, EBTRB  = OFF

enum{
    COLOR_RED =0,
    COLOR_GREEN,
    COLOR_BLUE,
    COLOR_YERROW,
    COLOR_PINK,
    COLOR_SKY,
    COLOR_ORANGE,
    COLOR_PARPLE,
    COLOR_WHITE,
    COLOR_MAX
};


enum{
    NORMAL_MODE = 0,
    FLASH_MODE,
    MOVE_MODE,
    MODE_MAX

};

enum{
    FLASH_RED =0,
    FLASH_GREEN,
    FLASH_BLUE,
    FLASH_YERROW,
    FLASH_PINK,
    FLASH_SKY,
    FLASH_ORANGE,
    FLASH_PARPLE,
    FLASH_WHITE,
    FLASH_MAX
};

enum{
    MOVE_RED =0,
    MOVE_GREEN,
    MOVE_BLUE,
    MOVE_YERROW,
    MOVE_PINK,
    MOVE_SKY,
    MOVE_ORANGE,
    MOVE_PARPLE,
    MOVE_WHITE,
    MOVE_MAX
};



static unsigned char keySelect();
static unsigned char getKeyNo();
static void LEDTurnOn();
static void changeBright(unsigned char count);
static void patternCopy(unsigned char colorType);
static void patternCopyFirst(unsigned char colorType);
static void lightUp(unsigned char colorIndex);
static void lightDown(unsigned char colorIndex);
static unsigned char nextColorType(unsigned char nowType);
static unsigned char beforeColorType(unsigned char nowType);
static unsigned char nextIllumiMode(unsigned char nowType);
static void changeFlashIllumi(unsigned char* colorIndex);
static void changeMoveIllumi(unsigned char* colorIndex, unsigned char* stay);


static unsigned char ledBright[3];
static unsigned char ledBrightNext[3];
static unsigned char pushKeyCount;
static unsigned char saveKeyNo;                      //SaveKey no
static unsigned char keyStanBy;
void DelayUs(int dlyus);

static const unsigned char LedColors[COLOR_MAX][3] =
{
     {10,0,0}            /* Red    */
    ,{0,10,0}            /* Green  */
    ,{0,0,10}            /* Blue   */
    ,{10,4,0}            /* Yerrow */
    ,{7,0,2}             /* Pink   */
    ,{0,5,5}             /* sky    */
    ,{12,2,0}            /* Orange */
    ,{5,0,5}             /* parple */
    ,{7,3,4}             /* white  */
};

static const unsigned char IllumiMoveColors[MOVE_MAX][3] =
{
     {10,0,0}            /* Red    */
    ,{0,10,0}            /* Green  */
    ,{0,0,10}            /* Blue   */
    ,{10,4,0}            /* Yerrow */
    ,{7,0,2}             /* Pink   */
    ,{0,5,5}             /* sky    */
    ,{12,2,0}            /* Orange */
    ,{5,0,5}             /* parple */
    ,{7,3,4}             /* white  */
};

static const unsigned char IllumiFlashColors[FLASH_MAX][3] =
{
     {10,0,0}            /* Red    */
    ,{0,10,0}            /* Green  */
    ,{0,0,10}            /* Blue   */
    ,{10,4,0}            /* Yerrow */
    ,{7,0,2}             /* Pink   */
    ,{0,5,5}             /* sky    */
    ,{12,2,0}            /* Orange */
    ,{5,0,5}             /* parple */
    ,{7,3,4}             /* white  */
};


void main(void){
    OSCCON = 0b01010010;                   // clock 4Mhz
    TRISB  = TRISB & PORTB_IO;             // port B I/O setting
    TRISC  = PORTC_IO;                     // port C I/O setting
    
    WPUBbits.WPUB7=1;
    WPUBbits.WPUB6=1;
    WPUBbits.WPUB5=1;
    //INTCON2 = 0b00000000;
    INTCON2bits.RABPU=0;
    ANSEL  = ANSEL & 0b00000111;
    ANSELH =  ANSELH & 0b11110000;

    //T0CON  =    0b11010001;                // Timer 0  (1024uSec)
    //                                       // 8BIT PS_1/4
    T0CON  =    0b11010000;                // Timer 0  (512uSec?)
                                           // 8BIT PS_1/2
    unsigned char keyChkCount;             // Counter for KeyCheck
    unsigned char BrightCount;             //Counter for Bright Adjust
    unsigned short illumiMoveCount;
    unsigned short illumiFlashCount;
    unsigned char moveStayCount;
    unsigned char keyNo;

    unsigned char colorTypeIndex;

    unsigned char illumiMoveIndex;
    unsigned char illumiFlashIndex;

    keyChkCount = 0;
    BrightCount = 0;
    illumiMoveCount = 0;
    illumiFlashCount = 0;
    moveStayCount = 0;
    keyNo = KEYNO_NONE;
    saveKeyNo  = KEYNO_NONE;
    pushKeyCount = 0;
    colorTypeIndex = COLOR_RED;
    illumiMoveIndex = FLASH_RED;
    illumiFlashIndex = MOVE_RED;

    keyStanBy=1;
    illumiMode = NORMAL_MODE;

    patternCopy(colorTypeIndex);
    patternCopyFirst(colorTypeIndex);

    MTX_OUT1=1;
    MTX_OUT2=1;
    MTX_OUT3=1;

    while(1){                          // Eternal Loop
        while( INTCONbits.TMR0IF==0);    // Timer 0 Time up
        INTCONbits.TMR0IF = 0;         // Timer 0 Start

        /* count up */
        keyChkCount++;
        BrightCount++;
        illumiMoveCount++;
        illumiFlashCount++;

        /* Bright Adjust */
        changeBright(BrightCount);

        /* Led turn on */
        if(BrightCount >= LIGHT_UP){
            LEDTurnOn();
            BrightCount=0;
        }

	/* Illumi Mode Flash */
        if(illumiFlashCount >= ILLUMI_FLASH_CHG){
            if(illumiMode == FLASH_MODE ){
              changeFlashIllumi(&illumiMoveIndex);
            }
            illumiFlashCount = 0;
        }

        /* Illumi Mode Move */
        if(illumiMoveCount >= ILLUMI_MOVE_CHG){
            if(illumiMode == MOVE_MODE ){
               changeMoveIllumi(&illumiMoveIndex, &moveStayCount);
            }
            illumiMoveCount = 0;
        }


        /* Key push check */
        if(keyChkCount >= KEY_TIMEUP){
            keyChkCount = 0;
            keyNo = getKeyNo();
         /* Mode Process */
       
            switch(keyNo){
                case KEYNO_NONE:
                   
                    break;

                case KEYNO_F1:
                    illumiMode = nextIllumiMode(illumiMode);
                    break;

                case KEYNO_F2:
                    illumiMode = NORMAL_MODE;
                    colorTypeIndex = nextColorType(colorTypeIndex);
                    patternCopy(colorTypeIndex);
                    break;

                case KEYNO_F3:
                    illumiMode = NORMAL_MODE;
                    colorTypeIndex =  beforeColorType(colorTypeIndex);
                    patternCopy(colorTypeIndex);
                    break;

                case KEYNO_RED_UP:
                    lightUp(INDEX_RED);
                    break;

                case KEYNO_RED_DOWN:
                    lightDown(INDEX_RED);
                    break;

                case KEYNO_GREEN_UP:
                    lightUp(INDEX_GREEN);
                    break;

                case KEYNO_GREEN_DOWN:
                    lightDown(INDEX_GREEN);
                    break;

                case KEYNO_BLUE_UP:
                    lightUp(INDEX_BLUE);
                    break;

                case KEYNO_BLUE_DOWN:
                    lightDown(INDEX_BLUE);
                    break;

                default:
                    break;
            }

        }
    }/*while*/
}


static unsigned char keySelect(){
    unsigned char selectKey;

    selectKey = KEYNO_NONE;

    MTX_OUT1=0;
    MTX_OUT2=1;
    MTX_OUT3=1;
    //DelayUs(3000);
   //if(PORTBbits.RB6 == 1)  return KEYNO_F2;

    /* F key */
    switch (MTXPORT & MTX_MASK)
    {
        case MTX_LEFT:
            selectKey = KEYNO_F1;
            break;

        case MTX_CENTOR:
            selectKey = KEYNO_F2;
            break;

        case MTX_RIGHT:
            selectKey = KEYNO_F3;
            break;

        default:
            break;

    }
    MTX_OUT1=1;
    //if(selectKey!=KEYNO_NONE){

   //     return selectKey;
 //   }
    /* up */
    MTX_OUT2=0;
    //DelayUs(3000);
        switch (MTXPORT & MTX_MASK)
    {
        case MTX_LEFT:
            selectKey = KEYNO_RED_UP;
            break;

        case MTX_CENTOR:
            selectKey = KEYNO_GREEN_UP;
            break;

        case MTX_RIGHT:
            selectKey = KEYNO_BLUE_UP;
            break;

        default:
            break;

    }

    MTX_OUT2=1;

   //if(selectKey!=KEYNO_NONE){

    //    return selectKey;
    //}

    /* down */
    MTX_OUT3=0;
    //DelayUs(3000);
        switch (MTXPORT & MTX_MASK)
    {
        case MTX_LEFT:
            selectKey = KEYNO_RED_DOWN;
            break;

        case MTX_CENTOR:
            selectKey = KEYNO_GREEN_DOWN;
            break;

        case MTX_RIGHT:
            selectKey = KEYNO_BLUE_DOWN;
            break;

        default:
            break;

    }
    MTX_OUT3=1;

    return selectKey;
}

static unsigned char getKeyNo(){

    unsigned char keyno;
    unsigned char getKey;
    keyno = KEYNO_NONE;  
    getKey = KEYNO_NONE;
    
    getKey = keySelect();



    if((saveKeyNo == KEYNO_NONE)&&(getKey != KEYNO_NONE) ){
        /* Count Start */
        pushKeyCount=1;
    }else if((getKey == saveKeyNo )&&(pushKeyCount >=1 )){
        pushKeyCount++;
    }else{
        pushKeyCount=0;
    }

    if(pushKeyCount >= KEY_PUSH_CNT){
        //Key Push Count OK!
         pushKeyCount=0;
         keyno= getKey;
    }

    saveKeyNo = getKey;

    return keyno;
   //return getKey;
    
}

static void changeBright(unsigned char count){

    if((ledBright[INDEX_RED]<= count)
            && (ledBright[INDEX_RED] < LIGHT_UP) ){
        LED_RED = LED_OFF;
    }


    if((ledBright[INDEX_GREEN]<= count)
            && (ledBright[INDEX_GREEN] < LIGHT_UP)){
       LED_GREEN = LED_OFF;
    }
    if((ledBright[INDEX_BLUE]<= count)
            && (ledBright[INDEX_BLUE] < LIGHT_UP)){
      LED_BLUE = LED_OFF;  
    }

    return;
}

static void LEDTurnOn(){

    /* Next Bright Get */
    ledBright[INDEX_RED] = ledBrightNext[INDEX_RED];
    ledBright[INDEX_GREEN] = ledBrightNext[INDEX_GREEN];
    ledBright[INDEX_BLUE] = ledBrightNext[INDEX_BLUE];

    /* LED Turn ON */
     if(ledBright[INDEX_RED] > 0){
        LED_RED = LED_ON;
     }else{
         LED_RED =LED_OFF;
     }
      
     if(ledBright[INDEX_GREEN] > 0){
        LED_GREEN = LED_ON;
     }else{
         LED_GREEN =LED_OFF;
     }

     if(ledBright[INDEX_BLUE] > 0){
        LED_BLUE = LED_ON;
     }else{
         LED_BLUE =LED_OFF;
     }

    return;
}

static void patternCopy(unsigned char colorType){
    
    if(colorType<COLOR_MAX){
        ledBrightNext[INDEX_RED]=LedColors[colorType][INDEX_RED];
        ledBrightNext[INDEX_GREEN]=LedColors[colorType][INDEX_GREEN];
        ledBrightNext[INDEX_BLUE]=LedColors[colorType][INDEX_BLUE];
    }

    return;
}

static void patternCopyFirst(unsigned char colorType){

    if(colorType<COLOR_MAX){
        ledBright[INDEX_RED]=LedColors[colorType][INDEX_RED];
        ledBright[INDEX_GREEN]=LedColors[colorType][INDEX_GREEN];
        ledBright[INDEX_BLUE]=LedColors[colorType][INDEX_BLUE];

    }

    return;
}


static void lightUp(unsigned char colorIndex){

    if(ledBrightNext[colorIndex]+1 <= LIGHT_UP){

        ledBrightNext[colorIndex]++;
    }


    return;
}
static void lightDown(unsigned char colorIndex){

    if(ledBrightNext[colorIndex] >0){

        ledBrightNext[colorIndex]--;
    }


    return;
}

static unsigned char nextColorType(unsigned char nowType){
    unsigned char colorType;
    colorType = nowType;

    if(colorType + 1 < COLOR_MAX){
        colorType++;
    }else{
        colorType=0;
    }

    return colorType;
}

static unsigned char beforeColorType(unsigned char nowType){
    
     unsigned char colorType;
    colorType = nowType;

    if(colorType >0){
        colorType--;
    }else{
        colorType=COLOR_MAX-1;
    }

    return colorType;
    
}


static unsigned char nextIllumiMode(unsigned char nowType){
    unsigned char illumi;
    illumi = nowType;

    if(illumi + 1 < MODE_MAX){
        illumi++;
    }else{
        illumi=0;
    }

    return illumi;
}


static void changeFlashIllumi(unsigned char* colorIndex){
   if(colorIndex + 1 < MOVE_MAX){
      colorIndex++;
   }else{
      colorIndex=0;
   }
   ledBrightNext[INDEX_RED]=IllumiFlashColors[colorIndex][INDEX_RED];
   ledBrightNext[INDEX_GREEN]=IllumiFlashColors[colorIndex][INDEX_GREEN];
   ledBrightNext[INDEX_BLUE]=IllumiFlashColors[colorIndex][INDEX_BLUE];
   return;
}

static void changeMoveIllumi(unsigned char* colorIndex, unsigned char* stay){
   

   if((ledBrightNext[INDEX_RED] == IllumiMoveColors[INDEX_RED]) 
     &&(ledBrightNext[INDEX_GREEN] == IllumiMoveColors[INDEX_GREEN])
     &&(ledBrightNext[INDEX_BLUE] == IllumiMoveColors[INDEX_BLUE])){

       if(stay + 1 < ILLUMI_MOVE_STAY){
           stay++;
       }else{
           stay=0;
          
          if(colorIndex + 1 < MOVE_MAX){
             colorIndex++;
          }else{
             colorIndex=0;
          }
        ledBrightNext[INDEX_RED]=IllumiMoveColors[colorIndex][INDEX_RED];
        ledBrightNext[INDEX_GREEN]=IllumiMoveColors[colorIndex][INDEX_GREEN];
        ledBrightNext[INDEX_BLUE]=IllumiMoveColors[colorIndex][INDEX_BLUE];

       }
       return;
   }


   if( ledBrightNext[INDEX_RED] < IllumiMoveColors[colorIndex][INDEX_RED] ){
       ledBrightNext[INDEX_RED]++;
   }

   if( ledBrightNext[INDEX_RED] > IllumiMoveColors[colorIndex][INDEX_RED] ){
       ledBrightNext[INDEX_RED]--;
   }

   if( ledBrightNext[INDEX_GREEN] < IllumiMoveColors[colorIndex][INDEX_GREEN] ){
       ledBrightNext[INDEX_GREEN]++;
   }

   if( ledBrightNext[INDEX_GREEN] > IllumiMoveColors[colorIndex][INDEX_GREEN] ){
       ledBrightNext[INDEX_GREEN]--;
   }

   if( ledBrightNext[INDEX_BLUE] < IllumiMoveColors[colorIndex][INDEX_BLUE] ){
       ledBrightNext[INDEX_BLUE]++;
   }

   if( ledBrightNext[INDEX_BLUE] > IllumiMoveColors[colorIndex][INDEX_BLUE] ){
       ledBrightNext[INDEX_BLUE]--;
   }


    return;
}


void DelayUs(int dlyus){
    while(dlyus--) __delay_us(1);  // dlyms x 1mS??
}