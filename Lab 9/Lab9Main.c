// Lab9Main.c
// Runs on MSPM0G3507
// Lab 9 ECE319K
// Your name: Neve Durcan, Neela Rajesh
// Last Modified: 12/26/2024

#include <stdio.h>
#include <stdint.h>
#include <ti/devices/msp/msp.h>
#include "../inc/ST7735.h"
#include "../inc/Clock.h"
#include "../inc/LaunchPad.h"
#include "../inc/TExaS.h"
#include "../inc/Timer.h"
#include "../inc/ADC1.h"
#include "../inc/DAC5.h"
#include "../inc/Arabic.h"
#include "SmallFont.h"
#include "LED.h"
#include "Switch.h"
#include "Sound.h"
#include "images/images.h"


// ===== Constants =====
#define PLAYER_WIDTH     18
#define PLAYER_HEIGHT     8
#define ENEMY_WIDTH      16
#define ENEMY_HEIGHT     10
#define BULLET_WIDTH      2
#define BULLET_HEIGHT     6
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   160
#define MAX_ENEMIES     10
#define MAX_BULLETS     5
#define PLAYER_Y        (SCREEN_HEIGHT - 10)
#define ENEMY_ROWS       2
#define ENEMY_COLS       5


// Languages
typedef enum {English, Spanish} Language_t;
typedef enum {PHRASE_HELLO, PHRASE_SCORE, PHRASE_YOUWIN, PHRASE_GAMEOVER, PHRASE_LANGUAGE, NUM_PHRASES} phrase_t;


const char Hello_English[]     = "Hello";
const char Hello_Spanish[]     = "\xADHola!";
const char Score_English[]     = "Score:";
const char Score_Spanish[]     = "Puntos:";
const char Win_English[]       = "You Win!";
const char Win_Spanish[]       = "\xA1Ganaste!";
const char GameOver_English[]  = "Game Over";
const char GameOver_Spanish[]  = "Juego Terminado";
const char Lang_English[]      = "English";
const char Lang_Spanish[]      = "Espa\xA4ol";


const char *Phrases[NUM_PHRASES][2] = { //2D array of pointers to const char strings bc we use the phrase_t and language_t objects as indexes
  {Hello_English, Hello_Spanish},
  {Score_English, Score_Spanish},
  {Win_English, Win_Spanish},
  {GameOver_English, GameOver_Spanish},
  {Lang_English, Lang_Spanish}
};


Language_t GameLanguage = English;


const char *GetPhrase(phrase_t p){ //input is any of the above-defined phrase_t objects (PHRASE_HELLO, YOUWIN, etc), produces output of that phrase in the Game Language
  return Phrases[p][GameLanguage];
}


// Game Structs
typedef struct { //Sprite has attributes of (x,y) position, image, and life/lives, sprites include Player and Enemies
  int16_t x, y;
  const uint16_t *image;
  uint8_t life;
} Sprite_t;


typedef struct { //Bullet has attributes of (x,y) position, and active (active if on screen, inactive if not on screen or collision happens)
  int16_t x, y;
  int8_t active;
} Bullet_t;


Sprite_t Player;
Sprite_t Enemies[ENEMY_ROWS][ENEMY_COLS]; //Enemies is an array of sprites, 2 rows of enemies, 5 enemies per row
Bullet_t Bullets[MAX_BULLETS]; //MAX_BULLETS is an old feature from when I was trying to make the game so the player had a limited number of bullets to shoot (5), doesnt really matter now
uint32_t Score = 0;
uint8_t GameOver = 0;
uint8_t GamePaused = 0;
uint8_t BulletCooldown = 0; //BulletCooldown means you can't spam


//  Functions 
static void InitPlayer(void){ //Player has initial x position predetermined, y position based on screen height
  Player.x = 52;
  Player.y = PLAYER_Y;
  Player.image = PlayerShip0;
  Player.life = 3; //this was from back when I was planning to have the invaders shoot at the player as well, so the player would have limited lives, but this attribute doesnt really matter for the player now
}


static void InitEnemies(void){
  for(int r = 0; r < ENEMY_ROWS; r++){
    for(int c = 0; c < ENEMY_COLS; c++){
      Enemies[r][c].x = 10 + c * 20;     //sets enemies initial positions
      Enemies[r][c].y = 20 + r * 15;
      Enemies[r][c].image = SmallEnemy10pointA;
      Enemies[r][c].life = 1;
    }
  }
}


static void InitBullets(void){   
  for(int i = 0; i < MAX_BULLETS; i++){
    Bullets[i].active = 0;  //all bullets initially inactive
  }
}


static void FireBullet(void){ //this function is for when the player shoots a bullet
  if(BulletCooldown) return; //if bullet cooldown is nonzero, don't fire right now, prevents bullets being shot in quick succession
  for(int i = 0; i < MAX_BULLETS; i++){
    if(!Bullets[i].active){ //you can only shoot a currently inactive bullet
      Bullets[i].active = 1; //bullet is active once you shoot
      Bullets[i].x = Player.x + PLAYER_WIDTH/2; //changes bullet x and y position
      Bullets[i].y = Player.y - BULLET_HEIGHT;
      Sound_Shoot();
      BulletCooldown = 3; //set bullet cooldown after shooting
      break;
    }
  }
}


static void MoveBullets(void){ //moves bullets that have just been shot upwards and removes them if they go off-screen, also resets cooldown
  for(int i = 0; i < MAX_BULLETS; i++){
    if(Bullets[i].active){ //if a bullet is active move it up by 4 pixels, if it goes off the top of the screen (y<0) mark as inactive
      Bullets[i].y -= 4;
      if(Bullets[i].y < 0) Bullets[i].active = 0;
    }
  }
  if(BulletCooldown) BulletCooldown--; //decrease cooldown 
}


static void DetectCollisions(void){ //this one was so fucking hard istg //removes/kills an enemy if a bullet hits it, increases player score, and plays killed sound
  for(int i = 0; i < MAX_BULLETS; i++){
    if(Bullets[i].active){ //if a bullet is active
      for(int r = 0; r < ENEMY_ROWS; r++){ //loop through all enemies
        for(int c = 0; c < ENEMY_COLS; c++){
          Sprite_t *enemy = &Enemies[r][c];
          if(enemy->life && Bullets[i].x + BULLET_WIDTH > enemy->x && Bullets[i].x < enemy->x + ENEMY_WIDTH &&
             Bullets[i].y < enemy->y + ENEMY_HEIGHT && Bullets[i].y + BULLET_HEIGHT > enemy->y){ //for each enemy check if its still alive, and if the bullet position overlaps with the enemy's position
            enemy->life = 0; //if there's a collision, the enemy's life is set to 0, bullet is set to inactive, and sound effect is played
            Bullets[i].active = 0;
            Score += 10;
            Sound_Killed();
          }
        }
      }
    }
  }
}


static void MoveEnemies(void){ //moves enemies down 
  static int8_t dir = 1;
  static uint8_t step = 0;
  step++;
  if(step >= 15){
    step = 0;
    dir = -dir;
    for(int r = 0; r < ENEMY_ROWS; r++){
      for(int c = 0; c < ENEMY_COLS; c++){
        Enemies[r][c].y += 8;
      }
    }
  }
  for(int r = 0; r < ENEMY_ROWS; r++){
    for(int c = 0; c < ENEMY_COLS; c++){
      Enemies[r][c].x += dir;
    }
  }
}


static uint8_t Game_Won(void){ //returns 1 if all enemies have 0 lives
  for(int r = 0; r < ENEMY_ROWS; r++){
    for(int c = 0; c < ENEMY_COLS; c++){
      if(Enemies[r][c].life) return 0;
    }
  }
  return 1;
}

static uint8_t EnemyReachedBottom(void) { //checks if enemies have reached the bottom of the screen bc then the game will be over
  for(int r = 0; r < ENEMY_ROWS; r++)
    for(int c = 0; c < ENEMY_COLS; c++)
      if(Enemies[r][c].life && Enemies[r][c].y + ENEMY_HEIGHT >= SCREEN_HEIGHT - 8)
        return 1;
  return 0;
}

static void DrawScreen(void){
  ST7735_FillScreen(ST7735_BLACK);
  ST7735_DrawBitmap(Player.x, Player.y + PLAYER_HEIGHT, Player.image, PLAYER_WIDTH, PLAYER_HEIGHT); //redraws player for each frame
  for(int r = 0; r < ENEMY_ROWS; r++){
    for(int c = 0; c < ENEMY_COLS; c++){
      if(Enemies[r][c].life)
        ST7735_DrawBitmap(Enemies[r][c].x, Enemies[r][c].y + ENEMY_HEIGHT, Enemies[r][c].image, ENEMY_WIDTH, ENEMY_HEIGHT); //redraw enemies for each frame bc their position will update
    }
  }
  for(int i = 0; i < MAX_BULLETS; i++){
    if(Bullets[i].active)
      ST7735_FillRect(Bullets[i].x, Bullets[i].y, BULLET_WIDTH, BULLET_HEIGHT, ST7735_YELLOW); //draws a yellow rectangle for the bullets, redraws bullets moving for each frame
  }
  ST7735_SetCursor(0, 0);
  ST7735_OutString(GetPhrase(PHRASE_SCORE)); //redraws score for each frame (some issues here, score is blurry)
  ST7735_OutUDec(Score);
}

//Game skeleton

void Game_Init(void){
  InitPlayer();
  InitEnemies();
  InitBullets();
  Score = 0;
  GameOver = 0;
  BulletCooldown = 0;
}


void Game_MovePlayer(uint32_t slideVal){ //changes player x position based on sampled slidepot value
  Player.x = slideVal * (SCREEN_WIDTH - PLAYER_WIDTH) / 4095; //maximum possible value of 12-bit ADC is 4095
}


void Game_Shoot(void){
  FireBullet();
}


uint8_t Game_IsOver(void){
  return GameOver;
}


void Game_SelectLanguage(void){ //checks Switch_In to determine game language, displays select language screen, this will happen before Game_MainLogic() is called
  ST7735_FillScreen(ST7735_BLACK);
  ST7735_SetCursor(0, 2);
  ST7735_OutString("Select Language:");
  ST7735_SetCursor(0, 4);
  ST7735_OutString("PB19 = English");
  ST7735_SetCursor(0, 5);
  ST7735_OutString("PB20 = Espanol");
  uint32_t last = 0;
  uint32_t in;
  while(1){
    in = Switch_In();
    if(((in & 0x01) != 0) && ((last & 0x01) == 0)){
      GameLanguage = English;
      break;
    }
    if(((in & 0x02) != 0) && ((last & 0x02) == 0)){
      GameLanguage = Spanish;
      break;
    }
    last = in;
    Clock_Delay1ms(50);
  }
  ST7735_FillScreen(ST7735_BLACK);
  ST7735_SetCursor(0, 0);
  ST7735_OutString(GetPhrase(PHRASE_HELLO));
  Clock_Delay1ms(1000);
}


void Game_MainLogic(void){
  if(GamePaused) return; //if paused, skip everything so when PB20 clicked during game, Game_MainLogic() does nothing
  MoveEnemies(); 
  MoveBullets(); 
  DetectCollisions();
  if(Game_Won() || EnemyReachedBottom()) GameOver = 1;
  DrawScreen();
}

//main code

// ****note to ECE319K students****
// the data sheet says the ADC does not work when clock is 80 MHz
// however, the ADC seems to work on my boards at 80 MHz
// I suggest you try 80MHz, but if it doesn't work, switch to 40MHz
void PLL_Init(void){ // set phase lock loop (PLL)
  // Clock_Init40MHz(); // run this line for 40MHz
  Clock_Init80MHz(0);   // run this line for 80MHz
}

uint8_t UpdateFlag = 0;
uint32_t Slide = 0;
uint32_t LastSwitchState = 0;

Arabic_t ArabicAlphabet[]={
alif,ayh,baa,daad,daal,dhaa,dhaal,faa,ghayh,haa,ha,jeem,kaaf,khaa,laam,meem,noon,qaaf,raa,saad,seen,sheen,ta,thaa,twe,waaw,yaa,zaa,space,dot,null
};
Arabic_t Hello[]={alif,baa,ha,raa,meem,null}; // hello
Arabic_t WeAreHonoredByYourPresence[]={alif,noon,waaw,ta,faa,raa,sheen,null}; // we are honored by your presence
int main0(void){ // main 0, demonstrate Arabic output
  Clock_Init80MHz(0);
  LaunchPad_Init();
  ST7735_InitR(INITR_REDTAB);
  ST7735_FillScreen(ST7735_WHITE);
  Arabic_SetCursor(0,15);
  Arabic_OutString(Hello);
  Arabic_SetCursor(0,31);
  Arabic_OutString(WeAreHonoredByYourPresence);
  Arabic_SetCursor(0,63);
  Arabic_OutString(ArabicAlphabet);
  while(1){
  }
}
uint32_t M=1;
uint32_t Random32(void){
  M = 1664525*M+1013904223;
  return M;
}
uint32_t Random(uint32_t n){
  return (Random32()>>16)%n;
}


// games  engine runs at 30Hz
void TIMG12_IRQHandler(void){uint32_t pos,msg;
  if((TIMG12->CPU_INT.IIDX) == 1){ // this will acknowledge
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
// game engine goes here
    // 1) sample slide pot
    // 2) read input switches
    // 3) move sprites
    // 4) start sounds
    // 5) set semaphore
    // NO LCD OUTPUT IN INTERRUPT SERVICE ROUTINES //jokes on you FireBullet() doesnt do LCD output
    Slide = ADCin(); //sample slidepot
    uint32_t switches = Switch_In(); //read switches

    if(((LastSwitchState & 0x01) == 0) && ((switches & 0x01) != 0)){ //checking rising edge on switch input
      Game_Shoot(); // PB19 shoot
    }
    if(((LastSwitchState & 0x02) == 0) && ((switches & 0x02) != 0)){
      GamePaused ^= 1; // PB20 toggle pause
    }

    LastSwitchState = switches;

    if(!GamePaused){ 
      Game_MovePlayer(Slide);
      UpdateFlag = 1; //lets main game loop know to update the screen for player position when not paused
    }

    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
  }
}
uint8_t TExaS_LaunchPadLogicPB27PB26(void){
  return (0x80|((GPIOB->DOUT31_0>>26)&0x03));
}

/*typedef enum {English, Spanish, Portuguese, French} Language_t;
Language_t myLanguage=English;
typedef enum {HELLO, GOODBYE, LANGUAGE} phrase_t;
const char Hello_English[] ="Hello";
const char Hello_Spanish[] ="\xADHola!";
const char Hello_Portuguese[] = "Ol\xA0";
const char Hello_French[] ="All\x83";
const char Goodbye_English[]="Goodbye";
const char Goodbye_Spanish[]="Adi\xA2s";
const char Goodbye_Portuguese[] = "Tchau";
const char Goodbye_French[] = "Au revoir";
const char Language_English[]="English";
const char Language_Spanish[]="Espa\xA4ol";
const char Language_Portuguese[]="Portugu\x88s";
const char Language_French[]="Fran\x87" "ais";
const char *Phrases[3][4]={
  {Hello_English,Hello_Spanish,Hello_Portuguese,Hello_French},
  {Goodbye_English,Goodbye_Spanish,Goodbye_Portuguese,Goodbye_French},
  {Language_English,Language_Spanish,Language_Portuguese,Language_French}
};*/
// use main1 to observe special characters
int main1(void){ // main1
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf();
  ST7735_FillScreen(0x0000);            // set screen to black
  for(phrase_t p = PHRASE_HELLO; p <= PHRASE_GAMEOVER; p++){
    for(Language_t lang = English; lang <= Spanish; lang++){
      ST7735_OutString(Phrases[PHRASE_LANGUAGE][lang]);
      ST7735_OutChar(' ');
      ST7735_OutString(Phrases[p][lang]);
      ST7735_OutChar(13);
    }
  }
  Clock_Delay1ms(3000);
  ST7735_FillScreen(0x0000);       // set screen to black
  char l = 128;
  while(1){
    Clock_Delay1ms(2000);
    for(int j=0; j < 3; j++){
      for(int i=0;i<16;i++){
        ST7735_SetCursor(7*j+0,i);
        ST7735_OutUDec(l);
        ST7735_OutChar(' ');
        ST7735_OutChar(' ');
        ST7735_SetCursor(7*j+4,i);
        ST7735_OutChar(l);
        l++;
      }
    }
  }
}

// use main2 to observe graphics
int main2(void){ // main2
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf();
    //note: if you colors are weird, see different options for
    // ST7735_InitR(INITR_REDTAB); inside ST7735_InitPrintf()
  ST7735_FillScreen(ST7735_BLACK);
  ST7735_DrawBitmap(22, 159, PlayerShip0, 18,8); // player ship bottom
  ST7735_DrawBitmap(53, 151, Bunker0, 18,5);
  ST7735_DrawBitmap(42, 159, PlayerShip1, 18,8); // player ship bottom
  ST7735_DrawBitmap(62, 159, PlayerShip2, 18,8); // player ship bottom
  ST7735_DrawBitmap(82, 159, PlayerShip3, 18,8); // player ship bottom
  ST7735_DrawBitmap(0, 9, SmallEnemy10pointA, 16,10);
  ST7735_DrawBitmap(20,9, SmallEnemy10pointB, 16,10);
  ST7735_DrawBitmap(40, 9, SmallEnemy20pointA, 16,10);
  ST7735_DrawBitmap(60, 9, SmallEnemy20pointB, 16,10);
  ST7735_DrawBitmap(80, 9, SmallEnemy30pointA, 16,10);

  for(uint32_t t=500;t>0;t=t-5){
    SmallFont_OutVertical(t,104,6); // top left
    Clock_Delay1ms(50);              // delay 50 msec
  }
  ST7735_FillScreen(0x0000);   // set screen to black
  ST7735_SetCursor(1, 1);
  ST7735_OutString("GAME OVER");
  ST7735_SetCursor(1, 2);
  ST7735_OutString("Nice try,");
  ST7735_SetCursor(1, 3);
  ST7735_OutString("Earthling!");
  ST7735_SetCursor(2, 4);
  ST7735_OutUDec(1234);
  while(1){
  }
}

// use main3 to test switches and LEDs
int main3(void){ // main3
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  Switch_Init(); // initialize switches
  LED_Init(); // initialize LED
  while(1){
    // write code to test switches and LEDs
    uint32_t input = Switch_In();
    ST7735_SetCursor(0, 1);
    ST7735_OutString("Shoot: ");
    ST7735_OutChar((input & 0x01) ? '1' : '0');
    ST7735_SetCursor(0, 2);
    ST7735_OutString("Pause: ");
    ST7735_OutChar((input & 0x02) ? '1' : '0');
    Clock_Delay1ms(100);
    
  }
}
// use main4 to test sound outputs
int main4(void){ uint32_t last=0,now;
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  Switch_Init(); // initialize switches
  LED_Init(); // initialize LED
  Sound_Init();  // initialize sound
  TExaS_Init(ADC0,6,0); // ADC1 channel 6 is PB20, TExaS scope
  __enable_irq();
  while(1){
    now = Switch_In(); // one of your buttons
    if(((last & 0x01) == 0) && ((now & 0x01) != 0)){
      Sound_Shoot(); // call one of your sounds
    }
    if(((last & 0x02) == 0) && ((now & 0x02) != 0)){
      Sound_Killed(); // call one of your sounds
    }
    if(LaunchPad_Input() == 1){
      Sound_Explosion(); // call one of your sounds
    }
    if(LaunchPad_Input() == 2){
      Sound_Fastinvader1(); // call one of your sounds
    }
    last = now;
    Clock_Delay1ms(20);
    // modify this to test all your sounds
  }
}

// ALL ST7735 OUTPUT MUST OCCUR IN MAIN
int main(void){ // final main 
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf();
    //note: if you colors are weird, see different options for
    // ST7735_InitR(INITR_REDTAB); inside ST7735_InitPrintf()
  ST7735_FillScreen(ST7735_BLACK);
  ADCinit();     //PB18 = ADC1 channel 5, slidepot
  Switch_Init(); // initialize switches
  LED_Init();    // initialize LED
  Sound_Init();  // initialize sound

  TExaS_Init(0,0,&TExaS_LaunchPadLogicPB27PB26); // PB27 and PB26
    // initialize interrupts on TimerG12 at 30 Hz
  TimerG12_IntArm(80000000/30,2);
  // initialize all data structures

  Game_SelectLanguage(); //initialize everything and enable interrupts before main code
  Game_Init();
  __enable_irq();

  while(1){
    // wait for semaphore
       // clear semaphore
       // update ST7735R
    // check for end game or level switch
      if(UpdateFlag){ //if screen needs to be updated (which it will be if not paused since enemies are constantly moving down each frame), call main game logic to do that, and clear the semaphore
      UpdateFlag = 0;
      Game_MainLogic();

      if(Game_IsOver()){
        ST7735_SetCursor(4, 6);
        ST7735_OutString(GetPhrase(PHRASE_GAMEOVER));

        if(Score >= 100){ //if score at end of game is 100, add you win phrase below game over phrase
        ST7735_SetCursor(4, 8);
        ST7735_OutString(GetPhrase(PHRASE_YOUWIN));
        }

        while(1);
      }
    }
  }
}