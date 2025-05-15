// FIFO1.c
// Runs on any microcontroller
// Provide functions that implement the Software FiFo Buffer
// Last Modified: April 3, 2025
// Student names: Neve Durcan, Neela Rajesg
// Last modification date: change this to the last modification date or look very silly
#include <stdint.h>


// Declare state variables for FiFo
//        size, buffer, put and get indexes


// *********** Fifo1_Init**********
// Initializes a software FIFO1 of a
// fixed size and sets up indexes for
// put and get operations
#define FIFO1_SIZE 16
static uint32_t PutI;  // index to put new
static uint32_t GetI;  // index of oldest
static uint32_t isEmpty;
static char FIFO1[FIFO1_SIZE];


void Fifo1_Init(){
//Complete this
  PutI = 0;
  GetI = 0;
  isEmpty = 1;


}


// *********** Fifo1_Put**********
// Adds an element to the FIFO1
// Input: data is character to be inserted
// Output: 1 for success, data properly saved
//         0 for failure, FIFO1 is full
uint32_t Fifo1_Put(char data){
  //Complete this routine
  uint32_t next;
  next = PutI + 1;
  
  if(next == FIFO1_SIZE) {
    next = 0;
  }

  if((isEmpty == 0) && (next == GetI)) {
    return 0;
  }

    FIFO1[PutI] = data;  
    PutI = next;
    isEmpty = 0;
    return 1;
}


// *********** Fifo1_Get**********
// Gets an element from the FIFO1
// Input: none
// Output: If the FIFO1 is empty return 0
//         If the FIFO1 has data, remove it, and return it
char Fifo1_Get(void){
  //Complete this routine
  if(isEmpty == 1) {
    return 0;
  }

  char data = FIFO1[GetI];  
  uint32_t nextGet;
  nextGet = GetI + 1;
  
  if(nextGet == FIFO1_SIZE) {
    nextGet = 0;
  } 
  
  GetI = nextGet;

  if(GetI == PutI) {
    isEmpty = 1;
  }

  return data;
}
