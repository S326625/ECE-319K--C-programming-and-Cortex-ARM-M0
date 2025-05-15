// Sound.c
// Runs on MSPM0
// Sound assets in sounds/sounds.h
// your name: Neve Durcan, Neela Rajesh
// your data 
#include <stdint.h>
#include <ti/devices/msp/msp.h>
#include "Sound.h"
#include "sounds/sounds.h"
#include "../inc/DAC5.h"
#include "../inc/Timer.h"

static const uint8_t *SoundPt = 0;
static uint32_t SoundCount = 0;

void SysTick_IntArm(uint32_t period, uint32_t priority){
  // write this
    NVIC->ISER[0] = 1 << (SysTick_IRQn & 0x1F); // enable interrupt
    NVIC->IP[SysTick_IRQn & 0x1F] = priority << 5;
    SysTick->LOAD = period - 1;
    SysTick->VAL = 0;                    // any write clears current
    SysTick->CTRL = 0x00000007;         // enable, core clock, interrupt
}
// initialize a 11kHz SysTick, however no sound should be started
// initialize any global variables
// Initialize the 5-bit DAC
void Sound_Init(void){
  DAC5_Init(); // uses PB0–PB4
  SysTick->CTRL = 0; // keep disabled until needed
  SysTick_IntArm(7273, 0); // ~11kHz
  SysTick->CTRL = 0; // turn back off until sound plays
// write this
  
}
void SysTick_Handler(void){ // called at 11 kHz
  // output one value to DAC if a sound is active
    // output one value to DAC if a sound is active
    if(SoundCount){
        DAC5_Out(*SoundPt);  // send value to DAC
        SoundPt++;
        SoundCount--;
    } else {
        SysTick->CTRL = 0; // turn off SysTick when done
    }

}

//******* Sound_Start ************
// This function does not output to the DAC. 
// Rather, it sets a pointer and counter, and then enables the SysTick interrupt.
// It starts the sound, and the SysTick ISR does the output
// feel free to change the parameters
// Sound should play once and stop
// Input: pt is a pointer to an array of DAC outputs
//        count is the length of the array
// Output: none
// special cases: as you wish to implement
void Sound_Start(const uint8_t *pt, uint32_t count){
// write this
    SoundPt = pt;
    SoundCount = count;
    SysTick->VAL = 0;       // any write clears current
    SysTick->CTRL = 0x00000007; // enable with core clock and interrupt
  
}
void Sound_Shoot(void){
// write this
  Sound_Start( shoot, 4080);
}
void Sound_Killed(void){
// write this
  Sound_Start(invaderkilled, 3360);
}

void Sound_Explosion(void){
  Sound_Start(explosion, 2000);
// write this

}

void Sound_Fastinvader1(void){
  Sound_Start(fastinvader1, 3120);

}
void Sound_Fastinvader2(void){
  Sound_Start(fastinvader2, 3120);

}
void Sound_Fastinvader3(void){
  Sound_Start(fastinvader3, 3120);

}
void Sound_Fastinvader4(void){
   Sound_Start(fastinvader4, 3120);

}
void Sound_Highpitch(void){
  Sound_Start(highpitch, 1800);

}

