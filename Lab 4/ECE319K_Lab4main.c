/* ECE319K_Lab4main.c
 * Traffic light FSM
 * ECE319H students must use pointers for next state
 * ECE319K students can use indices or pointers for next state
 * Put your names here or look silly: Neve Durcan, Neela Rajesh
  */

#include <ti/devices/msp/msp.h>
#include "../inc/LaunchPad.h"
#include "../inc/Clock.h"
#include "../inc/UART.h"
#include "../inc/Timer.h"
#include "../inc/Dump.h"  // student's Lab 3
#include <stdio.h>
#include <string.h>
// put both EIDs in the next two lines
const char EID1[] = "NGD445"; //  ;replace abc123 with your EID
const char EID2[] = "NR25992"; //  ;replace abc123 with your EID
// Hint implement Traffic_Out before creating the struct, make struct match your Traffic_Out

#define goN 0
#define waitN  1
#define allRedN 2
#define goW 3
#define whiteWarn 4
#define redWarn1 5
#define off1 6
#define redWarn2 7
#define off2 8
#define allRedW 9
#define goE 10
#define waitE 11
#define allRedE 12

uint32_t current_state;
uint32_t in;

struct state{
    uint32_t output;
    uint32_t wait;
    uint32_t Next[8];
};

//sensors: 000- nothing detected, 00000
//         001- only E, 08000
//         010- only N, 10000
//         011- N and E, 18000
//         100- only walk, 20000 
//         101- E and walk, 28000
//         110- N and walk, 30000
//         111- all 3, 38000

typedef struct state st_t;

const st_t FSM[13] = {{0x4000101, 40, {goN, waitN, goN, waitN, waitN, waitN, waitN, waitN}}, //goN
                {0x4000102, 20, {allRedN, allRedN, allRedN, allRedN, allRedN, allRedN, allRedN, allRedN}}, //waitN
                {0x4000104, 20, {allRedN, goE, goN, goE, goW, goW, goW, goW}}, //allRedN
                {0xC400104, 40, {goW, whiteWarn, whiteWarn, whiteWarn, goW, whiteWarn, whiteWarn, whiteWarn}}, //goW
                {0xC400104, 10, {redWarn1, redWarn1, redWarn1, redWarn1, redWarn1, redWarn1, redWarn1, redWarn1}}, //whiteWarn
                {0x4000104, 10, {off1, off1, off1, off1, off1, off1, off1, off1}}, //redWarn1
                {0x0000104, 10, {redWarn2, redWarn2, redWarn2, redWarn2, redWarn2, redWarn2, redWarn2, redWarn2}}, //off1
                {0x4000104, 10, {off2, off2, off2, off2, off2, off2, off2, off2}}, //redWarn2
                {0x0000104, 10, {allRedW, allRedW, allRedW, allRedW, allRedW, allRedW, allRedW, allRedW}}, //off2
                {0x4000104, 20, {allRedW, goE, goN, goE, goW, goE, goN, goE}}, //allRedW
                {0x4000044, 40, {goE, goE, waitE, waitE, waitE, waitE, waitE, waitE}}, //goE
                {0x4000084, 20, {allRedE, allRedE, allRedE, allRedE, allRedE, allRedE, allRedE, allRedE}}, //waitE
                {0x4000104, 20, {allRedE, goE, goN, goN, goW, goW, goN, goN}}}; //allRedE
                
// initialize all 6 LED outputs and 3 switch inputs
// assumes LaunchPad_Init resets and powers A and B
void Traffic_Init(void){ // assumes LaunchPad_Init resets and powers A and B
 // write this 
 IOMUX->SECCFG.PINCM[PB17INDEX] =  0x00040081; 
 IOMUX->SECCFG.PINCM[PB16INDEX] =  0x00040081;
 IOMUX->SECCFG.PINCM[PB15INDEX] =  0x00040081;

  // Enable South Output
  IOMUX->SECCFG.PINCM[PB2INDEX] =  0x00000081; // red
  IOMUX->SECCFG.PINCM[PB1INDEX] =  0x00000081;//yellow
  IOMUX->SECCFG.PINCM[PB0INDEX] =  0x00000081;//green
  GPIOB->DOE31_0 |= (1<<2) | (1<<1) | 1;

  // Enable West Output
  IOMUX->SECCFG.PINCM[PB8INDEX] =  0x00000081;
  IOMUX->SECCFG.PINCM[PB7INDEX] =  0x00000081;
  IOMUX->SECCFG.PINCM[PB6INDEX] =  0x00000081;
  GPIOB->DOE31_0 |= (1<<8) | (1<<7) | (1<<6);
}

/* Activate LEDs
* Inputs: west is 3-bit value to three east/west LEDs
*         south is 3-bit value to three north/south LEDs
*         walk is 3-bit value to 3-color positive logic LED on PB22,PB26,PB27
* Output: none
* - west =1 sets west green
* - west =2 sets west yellow
* - west =4 sets west red
* - south =1 sets south green
* - south =2 sets south yellow
* - south =4 sets south red
* - walk=0 to turn off LED
* - walk bit 22 sets blue color
* - walk bit 26 sets red color
* - walk bit 27 sets green color
* Feel free to change this. But, if you change the way it works, change the test programs too
* Be friendly*/
void Traffic_Out(uint32_t west, uint32_t south, uint32_t walk){
    GPIOB->DOUT31_0 &= ~( (1<<2) | (1<<1) | 1 );
    GPIOB->DOUT31_0 &= ~( (1<<8) | (1<<7) | (1<<6) );
    GPIOB->DOUT31_0 &= ~( (1<<22) | (1<<26) | (1<<27) );

    current_state = FSM[current_state].Next[in]; // change to next state passed
    GPIOB->DOUT31_0 |= FSM[current_state].output;
    SysTick_Wait10ms(FSM[current_state].wait);
}

/* Read sensors
 * Input: none
 * Output: sensor values
 * - bit 0 is west car sensor
 * - bit 1 is south car sensor
 * - bit 2 is walk people sensor
* Feel free to change this. But, if you change the way it works, change the test programs too
 */
uint32_t Traffic_In(void){
    uint32_t result;
    result = ((GPIOB->DIN31_0 & (1<<15)) | (GPIOB->DIN31_0 & (1<<16)) | (GPIOB->DIN31_0 & (1<<17))) >> 15;
    return result; // write this
}
// use main1 to determine Lab4 assignment
void Lab4Grader(int mode);
void Grader_Init(void);
int main2(void){ // main1
  Clock_Init80MHz(0);
  LaunchPad_Init();
  Lab4Grader(0); // print assignment, no grading
  while(1){
    //GPIOB->DOUT31_0 = (GPIOB->DOUT31_0 & 0xC4001C7) | FSM[current_state].output;
    //SysTick_Wait10ms(FSM[current_state].wait);
    //in = (GPIOB->DIN31_0 & 0x38000) >> 15;
    //current_state = FSM[current_state].Next[in]; 
  }
}
// use main2 to debug LED outputs
// at this point in ECE319K you need to be writing your own test functions
// modify this program so it tests your Traffic_Out  function
int main3(void){ // main2
  Clock_Init80MHz(0);
  LaunchPad_Init();
  Grader_Init(); // execute this line before your code
  LaunchPad_LED1off();
  Traffic_Init(); // your Lab 4 initialization
  if((GPIOB->DOE31_0 & 0x20)==0){
    UART_OutString("access to GPIOB->DOE31_0 should be friendly.\n\r");
  }
  UART_Init();
  UART_OutString("Lab 4, Spring 2025, Step 1. Debug LEDs\n\r");
  UART_OutString("EID1= "); UART_OutString((char*)EID1); UART_OutString("\n\r");
  UART_OutString("EID2= "); UART_OutString((char*)EID2); UART_OutString("\n\r");
  //current_state = goN;
  Debug_Init();
  SysTick_Init();
  uint32_t n = 1;
  while(1){
    // write code to test your Traffic_Out
    //GPIOB->DOUT31_0 = (GPIOB->DOUT31_0 & 0x3BFFE38) | FSM[current_state].output;
    //Traffic_Out(((FSM[current_state].output) & 0x00001C0),((FSM[current_state].output) & 0x0000007), ((FSM[current_state].output) & 0xC400000));
    //SysTick_Wait10ms(FSM[current_state].wait);
    //in = (GPIOB->DIN31_0 & 0x38000) >> 15;
   // current_state = FSM[current_state].Next[in]; 
   // uint32_t data = GPIOB->DOUT31_0;
   
    Traffic_Out(1, 0, 0);
    uint32_t data = GPIOB->DOUT31_0;
    Debug_Dump(data);

    Traffic_Out(2, 0, 0);
    data = GPIOB->DOUT31_0;
    Debug_Dump(data);

    Traffic_Out(4, 0, 0);
    data = GPIOB->DOUT31_0;
    Debug_Dump(data);

    Traffic_Out(0, 1, 0);
    data = GPIOB->DOUT31_0;
    Debug_Dump(data);

    Traffic_Out(0, 2, 0);
    data = GPIOB->DOUT31_0;
    Debug_Dump(data);

    Traffic_Out(0, 4, 0);
    data = GPIOB->DOUT31_0;
    Debug_Dump(data);

    Traffic_Out(0, 0, 1);
    data = GPIOB->DOUT31_0;
    Debug_Dump(data);

    Traffic_Out(0, 0, 2);
    data = GPIOB->DOUT31_0;
    Debug_Dump(data);

    Traffic_Out(0, 0, 4);
    data = GPIOB->DOUT31_0;
    Debug_Dump(data);


    if((GPIOB->DOUT31_0&0x20) == 0){
      UART_OutString("DOUT not friendly\n\r");
    }
  }
}
// use main3 to debug the three input switches
// at this point in ECE319K you need to be writing your own test functions
// modify this program so it tests your Traffic_In  function
int main4(void){ // main3
  uint32_t last=0,now;
  Clock_Init80MHz(0);
  LaunchPad_Init();
  Traffic_Init(); // your Lab 4 initialization
  Debug_Init();   // Lab 3 debugging
  UART_Init();
  __enable_irq(); // UART uses interrupts
  UART_OutString("Lab 4, Spring 2025, Step 2. Debug switches\n\r");
  UART_OutString("EID1= "); UART_OutString((char*)EID1); UART_OutString("\n\r");
  UART_OutString("EID2= "); UART_OutString((char*)EID2); UART_OutString("\n\r");
  while(1){
    now = Traffic_In(); // Your Lab4 input
    if(now != last){ // change
      UART_OutString("Switch= 0x"); UART_OutUHex(now); UART_OutString("\n\r");
      Debug_Dump(now);
    }
    last = now;
    Clock_Delay(800000); // 10ms, to debounce switch
  }
}
// use main4 to debug using your dump
// proving your machine cycles through all states
int main5(void){// main4
uint32_t input;
  Clock_Init80MHz(0);
  LaunchPad_Init();
  LaunchPad_LED1off();
  Traffic_Init(); // your Lab 4 initialization
 // set initial state
  Debug_Init();   // Lab 3 debugging
  UART_Init();
  __enable_irq(); // UART uses interrupts
  UART_OutString("Lab 4, Spring 2025, Step 3. Debug FSM cycle\n\r");
  UART_OutString("EID1= "); UART_OutString((char*)EID1); UART_OutString("\n\r");
  UART_OutString("EID2= "); UART_OutString((char*)EID2); UART_OutString("\n\r");
// initialize your FSM
  SysTick_Init();   // Initialize SysTick for software waits

  current_state = goN;
  while(1){
    
    Traffic_Out((((FSM[current_state].output) & 0x00001C0)>>6),(((FSM[current_state].output) & 0x0000007)), (((FSM[current_state].output) & 0xC400000)>>22));
    uint32_t data = (current_state << 24) | ((GPIOB->DOUT31_0 & 0x00001C0)<<10) | ((GPIOB->DOUT31_0 & 0x0000007)<<8) | ((GPIOB->DOUT31_0 & 0xC400000)>>20);
    Debug_Dump(data);
    in = 7; 
    //current_state = FSM[current_state].Next[in]; 
  ;

      // 1) output depending on state using Traffic_Out
      // call your Debug_Dump logging your state number and output
      // 2) wait depending on state
      // 3) hard code this so input always shows all switches pressed
      // 4) next depends on state and input
  }
}
// use main5 to grade
int main(void){// main5
  Clock_Init80MHz(0);
  LaunchPad_Init();
  Grader_Init(); // execute this line before your code
  LaunchPad_LED1off();
  Traffic_Init(); // your Lab 4 initialization
// initialize your FSM
  SysTick_Init();   // Initialize SysTick for software waits
  // initialize your FSM
  Lab4Grader(1); // activate UART, grader and interrupts
  current_state = goN;
  while(1){
    Traffic_Out((((FSM[current_state].output) & 0x00001C0)>>6),(((FSM[current_state].output) & 0x0000007)), (((FSM[current_state].output) & 0xC400000)>>22));
    uint32_t data = (current_state << 24) | ((GPIOB->DOUT31_0 & 0x00001C0)<<10) | ((GPIOB->DOUT31_0 & 0x0000007)<<8) | ((GPIOB->DOUT31_0 & 0xC400000)>>20);
    Debug_Dump(data);
    in = Traffic_In(); 
    //current_state = FSM[current_state].Next[in]; 
      // 1) output depending on state using Traffic_Out
      // call your Debug_Dump logging your state number and output
      // 2) wait depending on state
      // 3) input from switches
      // 4) next depends on state and input
  }
}

