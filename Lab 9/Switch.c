/*
 * Switch.c
 *
 *  Created on: Nov 5, 2023
 *      Author:Neela Rajesh, Neve Durcan
 */
#include <ti/devices/msp/msp.h>
#include "../inc/LaunchPad.h"
// LaunchPad.h defines all the indices into the PINCM table
void Switch_Init(void){
    // write this
    IOMUX->SECCFG.PINCM[PB19INDEX] = 0x00040081; // GPIO input with pull-up
    GPIOB->DOE31_0 &= ~(1<<19);

    // Pause = PA20
    IOMUX->SECCFG.PINCM[PB20INDEX] = 0x00040081;
    GPIOB->DOE31_0 &= ~(1<<20);
 
}
// return current state of switches
uint32_t Switch_In(void){
     Clock_Delay1ms(20); // debounce
    // write this
     uint32_t raw = GPIOB->DIN31_0;
     //return ((~raw >> 19) & 0x01) | (((~raw >> 20) & 0x01) << 1);
     if(((raw >> 19) & 0x01) != 0) {
        return 1;//PB19 Pressed- English/Shoot
     }

    if(((raw >> 20) & 0x01) != 0) {
        return 2; //PB20 Pressed- Spanish/Pause-Play
     }
    return 0; //if neither switch is pressed
    
}
