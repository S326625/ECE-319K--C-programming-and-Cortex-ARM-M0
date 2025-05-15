/* ECE319K_Lab5main.c
 * Digital Piano using a Digital to Analog Converter
 * December 28, 2024
 * 5-bit binary-weighted DAC connected to PB4-PB0
 * 4-bit keyboard connected to PB19-PB16
*/

#include <ti/devices/msp/msp.h>
#include "../inc/LaunchPad.h"
#include "../inc/Clock.h"
#include "../inc/UART.h"
#include "../inc/Timer.h"
#include "../inc/Dump.h"  // student's Lab 3
#include "../inc/DAC5.h"  // student's Lab 5
#include "../inc/Key.h"   // student's Lab 5
#include <stdio.h>
#include <string.h>
// put both EIDs in the next two lines
const char EID1[] = "NGD445"; // replace abc123 with your EID
const char EID2[] = "NR25992"; // replace abc123 with your EID

// prototypes to your low-level Lab 5 code
void Sound_Init(uint32_t period, uint32_t priority);
void Sound_Start(uint32_t period);
void Sound_Stop(void);

// use main1 to determine Lab5 assignment
void Lab5Grader(int mode);
void Grader_Init(void);
int main1(void){ // main1
  Clock_Init80MHz(0);
  LaunchPad_Init();
  Lab5Grader(0); // print assignment, no grading
  while(1){
  }
}
const uint32_t Inputs[12]={0, 1, 7, 8, 15, 16, 17, 23, 24, 25, 30, 31};
uint32_t Testdata;

// use main2a to perform static testing of DAC, if you have a voltmeter
int main2(void){ // main2a
  Clock_Init80MHz(0);
  LaunchPad_Init();
  Grader_Init();   // execute this line before your code
  DAC5_Init();     // your Lab 5 initialization
  if((GPIOB->DOE31_0 & 0x20)==0){
    UART_OutString("access to GPIOB->DOE31_0 should be friendly.\n\r");
  }
  Debug_Init();    // Lab 3 debugging
  while(1){
    for(uint32_t i=0; i<12; i++){ //0-11
      Testdata = Inputs[i];
      DAC5_Out(Testdata);
      // put a breakpoint on the next line and use meter to measure DACout
      // place data in Table 5.3
      Debug_Dump(Testdata);
    }
    if((GPIOB->DOUT31_0&0x20) == 0){
      UART_OutString("DOUT not friendly\n\r");
    }
  }
}
// use main2b to perform static testing of DAC, if you do not have a voltmeter
// attach PB20 (scope uses PB20 as ADC input) to your DACout
// TExaSdisplay scope uses TimerG7, ADC0
// To use the scope, there can be no breakpoints in your code
int main3(void){ // main2b
  Clock_Init80MHz(0);
  LaunchPad_Init();
  Grader_Init();   // execute this line before your code
  Lab5Grader(2);   // Scope
  DAC5_Init();     // your Lab 5 initialization
  if((GPIOB->DOE31_0 & 0x20)==0){
    while(1){}; // access to GPIOB->DOE31_0 should be friendly
  }
  Debug_Init();    // Lab 3 debugging
  while(1){
    for(uint32_t i=0; i<12; i++){ //0-11
      Testdata = Inputs[i];
      DAC5_Out(Testdata);
      Debug_Dump(Testdata);
        // use TExaSdisplay scope to measure DACout
        // place data in Table 5.3
        // touch and release S2 to continue
      while(LaunchPad_InS2()==0){}; // wait for S2 to be touched
      while(LaunchPad_InS2()!=0){}; // wait for S2 to be released
      if((GPIOB->DOUT31_0&0x20) == 0){
         while(1){}; // DOUT not friendly
      }
    }
  }
}
// use main3 to perform dynamic testing of DAC,
// In lab, attach your DACout to the real scope
// If you do not have a scope attach PB20 (scope uses PB20 as ADC input) to your DACout
// TExaSdisplay scope uses TimerG7, ADC0
// To perform dynamic testing, there can be no breakpoints in your code
// DACout will be a monotonic ramp with period 32ms,
int main4(void){ // main3
  Clock_Init80MHz(0);
  LaunchPad_Init();
  Grader_Init();   // execute this line before your code
  Lab5Grader(2);   // Scope
  DAC5_Init();     // your Lab 5 initialization
  Debug_Init();    // Lab 3 debugging
  while(1){
    for(uint32_t i=0; i<32; i++){ //0-31
      DAC5_Out(i);
      Debug_Dump(i);
        // scope to observe waveform
        // place data in Table 5.3
      Clock_Delay1ms(1);
    }
  }
}

// use main4 to debug the four input switches
int main5(void){ // main4
  uint32_t last=0,now;
  Clock_Init80MHz(0);
  LaunchPad_Init();
  Key_Init(); // your Lab 5 initialization
  Debug_Init();   // Lab 3 debugging
  UART_Init();
  __enable_irq(); // UART uses interrupts
  UART_OutString("Lab 5, Spring 2025, Step 4. Debug switches\n\r");
  UART_OutString("EID1= "); UART_OutString((char*)EID1); UART_OutString("\n\r");
  UART_OutString("EID2= "); UART_OutString((char*)EID2); UART_OutString("\n\r");
  while(1){
    now = Key_In(); // Your Lab5 input
    if(now != last){ // change
      UART_OutString("Switch= 0x"); UART_OutUHex(now); UART_OutString("\n\r");
      Debug_Dump(now);
    }
    last = now;
    Clock_Delay(800000); // 10ms, to debounce switch
  }
}

// use main5 to debug your system
// In lab, attach your DACout to the real scope
// If you do not have a scope attach PB20 (scope uses PB20 as ADC input) to your DACout
// TExaSdisplay scope uses TimerG7, ADC0
// To perform dynamic testing, there can be no breakpoints in your code
// DACout will be a sine wave with period/frequency depending on which key is pressed
int main6(void){// main5
  Clock_Init80MHz(0);
  LaunchPad_Init();
  Lab5Grader(2);   // 1=logic analyzer, 2=Scope, 3=grade
  DAC5_Init();     // DAC initialization
  Sound_Init(1,0); // SysTick initialization, initially off, priority 0
  Key_Init();      // Keyboard initialization
  Sound_Start(9956); // start one continuous wave
  while(1){
  }
}
uint32_t last,key;
// use main6 to debug/grade your final system
// In lab, attach your DACout to the real scope
// If you do not have a scope attach PB20 (scope uses PB20 as ADC input) to your DACout
// TExaSdisplay scope uses TimerG7, ADC0
// To perform dynamic testing, there can be no breakpoints in your code
// DACout will be a sine wave with period/frequency depending on which key is pressed
uint32_t Previous = 0;
int main(void){// main6
  Clock_Init80MHz(0);
  LaunchPad_Init();
  Grader_Init();   // execute this line before your code
  Lab5Grader(3);   // 1=logic analyzer, 2=Scope, 3=grade
  DAC5_Init();     // DAC initialization
  Sound_Init(1,0); // SysTick initialization, initially off, priority 0
  Key_Init();      // Keyboard initialization
  Debug_Init();    // Lab 3 debugging
  while(1){
// if key goes from not pressed to pressed
//   -call Sound_Start with the appropriate period
//   -call Debug_Dump with period
// if key goes from pressed to not pressed
//   -call Sound_Stop
// I.e., if key has not changed DO NOT CALL start or stop
uint32_t boop= Key_In();

 if(boop != Previous ) //loop until key pressed?
 //loop key released




 { Sound_Stop();
 
  if(boop != 0){
            uint32_t val = 0;


            if(boop == 1){
                val = 8035; //frequencys make period
            }else if(boop == 2){
                val = 6378;
            }else if(boop == 4){
                val =5363 ;
            } else{
                val = 4778 ;
            }


            Sound_Start(val);
            Debug_Dump(val);
    }

    else {
      Sound_Stop();
    }


        Previous = boop;
    }


 }

    Clock_Delay(800000); // 10ms, to debounce switch
  }

// To grade you must connect PB20 to your DACout
// Run main5 with Lab5Grader(3);   // Grader
// Observe Terminal window

const uint8_t SineWave[32] = {16,19,22,24,27,28,30,31,31,31,30,28,27,24,22,19,16,13,10,8,5,4,2,1,1,1,2,4,5,8,10,13};
// Input: interrupts every 12.5ns*period
//float melody_frequencies[] = { 261.63, 261.63, 392.00, 392.00, 440.00, 440.00, 392.00, 349.23, 349.23, 329.63, 329.63, 293.66, 293.66, 261.63, 392.00, 392.00, 349.23, 349.23, 329.63, 329.63, 293.66,392.00, 392.00, 349.23, 349.23, 329.63, 329.63, 293.66, 261.63, 261.63, 392.00, 392.00, 440.00, 440.00, 392.00  };
//        priority is 0 (highest) to 3 (lowest)
//const uint32_t melody_length = sizeof(melody_frequencies) / sizeof(melody_frequencies[0]);

//uint32_t melody_tempo[] = {
 //   500, 500, 500, 500, 500, 500, 500,
 //   500, 500, 500, 500, 500, 500, 500,
  //  500, 500, 500, 500, 500, 500, 500,
   // 500, 500, 500, 500, 500, 500, 500,
   // 500, 500, 500, 500, 500, 500, 500
//};
uint32_t Index=0;  

void Sound_Init(uint32_t period, uint32_t priority){
  // write this
  SysTick_Init();          // Port B is DAC
  SysTick->CTRL = 0;         // disable SysTick during setup
  SysTick->LOAD = period-1;  // reload value
  SysTick->VAL = 0;          // any write to current clears it
  SCB->SHP[1] = SCB->SHP[1]&(~0xC0000000)|0x40000000; // set priority = 1
  SysTick->CTRL = 0x007;
  //TimerG0_IntArm(uint16_t period, uint32_t prescale, uint32_t priority);
}

void Sound_Stop(void){
  // either set LOAD to 0 or clear bit 1 in CTRL
  SysTick->LOAD = 0;
}


void Sound_Start(uint32_t period){
    // set reload value
    // write any value to VAL, cause reload
    // write this
    SysTick->LOAD = period-1;
    SysTick->VAL = 0;
}

// Interrupt service routine
// Executed every 12.5ns*(period)
void SysTick_Handler(void){
    // write this
    // output one value to DAC
  DAC5_Out(SineWave[Index]);    // output one value each interrupt multiply index times 2 to get louder
  Index = (Index+1)&0x1F;
}




