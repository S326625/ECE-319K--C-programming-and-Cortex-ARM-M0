// StringConversion.s
// Student names: Neve Durcan, Neela Rajesh
// Last modification date: 3/9/25
// Runs on any Cortex M0
// ECE319K lab 6 number to string conversion
//
// You write udivby10 and Dec2String
     .data
     .align 2
// no globals allowed for Lab 6
    .global OutChar    // virtual output device
    .global OutDec     // your Lab 6 function
    .global Test_udivby10

    .text
    .align 2
// **test of udivby10**
// since udivby10 is not AAPCS compliant, we must test it in assembly
Test_udivby10:
    PUSH {LR}

    MOVS R0,#123
    BL   udivby10
// put a breakpoint here
// R0 should equal 12 (0x0C)
// R1 should equal 3

    LDR R0,=12345
    BL   udivby10
// put a breakpoint here
// R0 should equal 1234 (0x4D2)
// R1 should equal 5

    MOVS R0,#0
    BL   udivby10
// put a breakpoint here
// R0 should equal 0
// R1 should equal 0
    POP {PC}

//****************************************************
// divisor=10
// Inputs: R0 is 16-bit dividend
// quotient*10 + remainder = dividend
// Output: R0 is 16-bit quotient=dividend/10
//         R1 is 16-bit remainder=dividend%10 (modulus)
// not AAPCS compliant because it returns two values
udivby10:
   PUSH {R2, R3, LR}
// write this
   MOVS R2, #0 //R2 = quotient
   MOVS R3, #10 //R3=divisor (10)
   udivby10loop:
      CMP  R0, R3
      BLO udivdone
      ADDS R2, R2, #1 
      SUBS R0, R0, R3
      B udivby10loop
   udivdone:
      MOVS R1, R0 //R1 = remainder
      MOVS R0, R2 //R0 = quotient
   POP  {R2, R3, PC}

  
//-----------------------OutDec-----------------------
// Convert a 16-bit number into unsigned decimal format
// Call the function OutChar to output each character
// You will call OutChar 1 to 5 times
// OutChar does not do actual output, OutChar does virtual output used by the grader
// Input: R0 (call by value) 16-bit unsigned number
// Output: none
// Invariables: This function must not permanently modify registers R4 to R11
    
.equ idx,    0   //Binding 
.equ num,    1

OutDec: 
   PUSH {R4-R7, LR}
   MOVS R4, #0
   MOVS R6, #0

// write this
   //Allocation
   OutDecLoop:  
      SUB SP, #4 //Allocation 
      MOV R7, SP
      BL udivby10 //R0=R0/10, R1=remainder
      STR R1, [R7, #idx] //Access
      CMP R0, #0
      BEQ OutDecDone
      ADDS R4, R4, #num
      B OutDecLoop

      OutDecDone:
         ADDS R4, R4, #num
         MOVS R6, R4
         OutDecDoneLoop:
            LDR R5, [R7, #idx] //Access
            ADD SP, #4 //Deallocation
            MOV R0, R5
            ADDS R0, R0, #0x30
            BL OutChar
            SUBS R6, R6, #num
            CMP R6, #0
            BEQ OutDecPrintDone
            B OutDecDoneLoop

   OutDecPrintDone:
      POP{R4-R7, PC}
//* * * * * * * * End of OutDec * * * * * * * *

// ECE319H recursive version
// Call the function OutChar to output each character
// You will call OutChar 1 to 5 times
// Input: R0 (call by value) 16-bit unsigned number
// Output: none
// Invariables: This function must not permanently modify registers R4 to R11

OutDec2:
   PUSH {LR}
// write this

   POP  {PC}



     .end
