//****************** ECE319K_Lab2.s ***************
// Your solution to Lab 2 in assembly code
// Author: Neela Rajesh
// Last Modified: 2/3/25
// ECE319K Spring 2025 (ECE319H students do Lab2H)
// I/O port addresses
    .include "../inc/msp.s"

        .equ GPIOB_DOUT31_0,    0x400A3280
        .equ GPIOB_DIN31_0,     0x400A3380
        .equ GPIOB_DOE31_0,     0x400A32C0

        .data
        .align 2
// Declare global variables here if needed
// with the .space assembly directive
        .text
        .thumb
        .align 2
        .global EID
EID:    .string "NR25992" // replace ZZZ123 with your EID here

        .align 2
        .global CountArr //lists H and L count values for each duty cycle calculated using t=12.5ns*count
CountArr: .long 160000  //10%
          .long 1440000
          .long 560000 //35%
          .long 1040000
          .long 800000 //50%
          .long 800000
          .long 1360000 //85%
          .long 240000
          

// this allow your Lab2 programs to the Lab2 grader
        .global Lab2Grader
// this allow the Lab2 grader to call your Lab2 program
        .global Lab2
// these two allow your Lab2 programs to all your Lab3 solutions
        .global Debug_Init
        .global Dump

// Switch input: PB2 PB1 or PB0, depending on EID
// LED output:   PB18 PB17 or PB16, depending on EID
// logic analyzer pins PB18 PB17 PB16 PB2 PB1 PB0
// analog scope pin PB20
Lab2:
// Initially the main program will
//   set bus clock at 80 MHz,
//   reset and power enable both Port A and Port B
// Lab2Grader will
//   configure interrupts  on TIMERG0 for grader or TIMERG7 for TExaS
//   initialize ADC0 PB20 for scope,
//   initialize UART0 for grader or TExaS
     MOVS R0,#10
// 0 for info,
// 1 debug with logic analyzer,
// 2 debug with scope,
// 3 debug without scope or logic analyzer
// 10 for grade
     BL   Lab2Grader
     BL   Debug_Init // your Lab3 (ignore this line while doing Lab 2)
     BL   Lab2Init
        LDR R4, =CountArr //R4 will be the array pointer
        LDR R3, =8 //R3 will be our counter to count through each duty cycle (each cycle is 2 counts)
        BL Interface
        m_loop: BL Switch_In
                CMP R0, #0 //Checking if switch is released
                BNE m_loop //If switch is not released, we keep looping until it's released, because it should be off when change is pressed
                BL LED_ON //turn on LED
                BL Dump
                BL Delay //Delay (High)
                BL LED_OFF //turn off LED
                BL Dump
                ADDS R4, R4, #4 //move to next spot in array for L delay
                BL Delay //Delay (Low)
                //Check for press and release to switch to next duty cycle
                BL Switch_In
                CMP R0, #1 //Check if switch has been pressed
                BNE p_loop //If it hasn't been pressed, we keep looping on the same duty cycle
        r_loop: BL LED_OFF //If we get here, it's being pressed, so now we turn the LED off and check for release
                BL Switch_In //Check for release
                CMP R0, #0 
                BNE r_loop //If it hasn't been released, we keep looping to check for a release
                ADDS R4, R4, #4 //If we get here, we have a press-and-release, so we need to move to the next duty cycle
                SUBS R3, R3, #2 //decrement R3 bc we've finished 1 duty cycle (2 counts)
                CMP R3, #0
                BEQ reset
                B m_loop 
        p_loop: SUBS R4, R4, #4 //if we get here, we're looping on the same duty cycle while we wait for the switch to be pressed, so we reset R4
                B m_loop
        reset:  ADDS R3, R3, #8 //if we get here, we went through 4 duty cycles so now we're resetting
                SUBS R4, R4, #32 //subtract R4 by 32 bc it's gone through 8 counts, each count is 4 spaces 
                B m_loop

        Switch_In: //Returns R0=0 if switch is open, 1 if it's closed
                LDR R0, =GPIOB_DIN31_0
                LDR R1, [R0]
                LDR R2, =0x00000004 //mask to isolate bit 2 for PB2 input
                ANDS R1, R1, R2 //R1 shows isolated bit 2
                LSRS R0, R1, #2 //R0 = divide R1 by 4 so if R1=4, R0=1
                BX LR  

        LED_ON: //turns LED on
                LDR R0, =GPIOB_DOUT31_0
                LDR R1, [R0]
                LDR R2, =0x00040000
                ORRS R1, R1 ,R2 //turn on bit 18, keep everything else the same 
                STR R1, [R0]
                BX LR

        LED_OFF: //turns off LED
                LDR R0, =GPIOB_DOUT31_0
                LDR R1, [R0]
                LDR R2, =0xFFFBFFFF
                ANDS R1, R1, R2 //turn off bit 18, keep everything else the same
                STR R1, [R0]
                BX LR

        Delay:
                LDR R5, [R4]
                SUBS R5, R5, #2
        dloop:  SUBS R5, R5, #4
                NOP
                BHS dloop
                BX LR


// make switch an input, LED an output
// PortB is already reset and powered
// Set IOMUX for your input and output
// Set GPIOB_DOE31_0 for your output (be friendly)
Lab2Init:
// ***do not reset/power Port A or Port B, already done****

//Interfacing pins
Interface:
        //PB2 Input Setup 
        LDR R0, =IOMUXPB2
        LDR R1, =0x00040081 //bit 18 is 1 for input enable, 7 for software connected, then #1 for GPIO in bits[3:0]
        STR R1, [R0] //storing at IOMUXPB2 address
        //PB18 Output Setup
        LDR R0, =IOMUXPB18
        LDR R1, =0x00000081 //bit 7 for software connected, then #1 for GPIO in bits[3:0]
        STR R1, [R0] //storing at IOMUXPB18 address
        LDR R0, =GPIOB_DOE31_0
        LDR R2, [R0]
        LDR R1, =0x00040000 //mask to set bit 18 to 1
        ORRS R2, R2, R1 //set bit 18 to 1, keep everything else the same
        STR R2, [R0]
        BX LR

.end
