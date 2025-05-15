//****************** ECE319K_Lab1.s ***************
// Your solution to Lab 1 in assembly code
// Author: Neela Rajesh
// Last Modified: 1/28/25
// Spring 2025
        .data
        .align 2
// Declare global variables here if needed
 .data
myEID: .ascii "NR25992\0"

// with the .space assembly directive

        .text
        .thumb
        .align 2
        .global EID
EID:    .string "NR25992" // replace ZZZ123 with your EID here

        .global Phase
        .align 2
Phase:  .long 10
// Phase= 0 will display your objective and some of the test cases, 
// Phase= 1 to 5 will run one test case (the ones you have been given)
// Phase= 6 to 7 will run one test case (the inputs you have not been given)
// Phase=10 will run the grader (all cases 1 to 7)
        .global Lab1
// Input: R0 points to the list
// Return: R0 as specified in Lab 1 assignment and terminal window
// According to AAPCS, you must save/restore R4-R7
// If your function calls another function, you must save/restore LR
Lab1: PUSH {R4-R7,LR}
       // your solution goes here
       LDR R3, =-1 //R3=-1, default for if there's no match in the array

       compareloop:
        LDR R2, =EID //R2=pointer to my EID
        LDR R1, [R0] //R1=pointer to current EID
        CMP R1, #0 //If R1=0, we've reached the end of the array and there's no match
        BEQ nomatch

        stringcompare:
                LDRB R4, [R2] //current char in my EID
                LDRB R5, [R1] //current char in current EID
                CMP R5, #0 //check if we've reached the end of the current EID
                BEQ equal //if we haven't branched out by the end, then the two EIDs are equal
                CMP R4, R5 //check if the chars are equal
                BNE unequal 
                ADDS R1, R1, #1 //increment to next char in both EIDs
                ADDS R2, R2, #1
                B stringcompare

        unequal:
                ADDS R0, R0, #8 //increment to next EID
                B compareloop

        equal:
                LDR R3, [R0, #4]
                MOVS R0, R3
                B end

        nomatch:
                MOVS R0, R3

        end:        
                POP  {R4-R7,PC} // return


        .align 2
        .global myClass
myClass: .long pAB123  // pointer to EID
         .long 95      // Score
         .long pXYZ1   // pointer to EID
         .long 96      // Score
         .long pAB5549 // pointer to EID
         .long 94      // Score
         .long 0       // null pointer means end of list
         .long 0
pAB123:  .string "AB123"
pXYZ1:   .string "XYZ1"
pAB5549: .string "AB5549"
        .end
