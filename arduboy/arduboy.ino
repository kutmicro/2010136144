#include <U8glib.h>
#include "InputController.h"
#include "GameManager.h"
#include "Img.h"

U8GLIB_NHD_C12864 u8g(13, 11, 10, 9, 8); // SPI Com: SCK = 13, MOSI = 11, CS = 10, CD = 9, RST = 8

//Init class
InputController inputController;
GameManager gameManager;

//input flag
bool up, down, left, right, push, aBut, bBut;

int lastInput;
unsigned long lTime;
int gameFps = 1000 / 10;

#define MENU_ITEMS 3
char *menu_strings[MENU_ITEMS] = {"Game Start", "Load Game", "Record"};
uint8_t menu_current = 0;
int menu_redraw_required = 0;

//methods
//menu methods
void drawMenu();
void updateMenu();


void draw(void)
{
  // graphic commands to redraw the complete screen should be placed here
  u8g.setFont(u8g_font_unifont);
}

void setup(void)
{
  //Menu 선택화면 띄우기
  u8g.setContrast(0); // Config the contrast to the best effect
  u8g.setRot180();    // rotate screen, if required

  u8g.firstPage();
  do
  {
    drawMenu();
  } while (u8g.nextPage()); //      u8g.firstPage();
  Serial.begin(9600);
}

void loop(void)
{
  //get input
  int input = inputController.getInput();
  
//  Serial.println(lastInput);
  //연속 입력을 방지하기 위해서
  //마지막 입력이 아무 입력도 없을 때 입력이 있는 것으로 간주
  if (lastInput == NIG) {
    if (input == LEFT) left = true;
    if (input == RIGHT) right = true;
    if (input == UP) up = true;
    if (input == DOWN) down = true;
    if (input == PUSH) push = true;
  }
  lastInput = input;


  if (millis() > lTime + gameFps)
  {
    //init lTime
    lTime = millis();


    //로고화면 띄우기
    if (gameManager.getGameStatus() == STATUS_MENU)
    {
      updateMenu(); // outside picture loop
      if (menu_redraw_required != 0)
      {
        u8g.firstPage();
        do
        {
          drawMenu(); // inside picture loop
        } while (u8g.nextPage());
        menu_redraw_required = 0; // menu updated, reset redraw flag
      }
    }
    //플레이 화면
    else if (gameManager.getGameStatus() == STATUS_PLAYING)
    {
      u8g.firstPage();
        do
        {
          drawMap();//show paying display
          drawMines();
        } while (u8g.nextPage());
    }
    //결과 화면
    else if (gameManager.getGameStatus() == STATUS_RESULT)
    {
    }
    //기록 화면
    else if (gameManager.getGameStatus() == STATUS_RECORD)
    {
    }

    initInputs();
  }
}

//draw_menu
void drawMenu(void)
{
  uint8_t i, h;
  u8g_uint_t w, d;
  u8g.setFont(u8g_font_6x13);
  u8g.setFontRefHeightText();
  u8g.setFontPosTop();
  h = u8g.getFontAscent() - u8g.getFontDescent();
  w = u8g.getWidth();
  for (i = 0; i < MENU_ITEMS; i++)
  { // draw all menu items
    d = (w - u8g.getStrWidth(menu_strings[i])) / 2;
    u8g.setDefaultForegroundColor();
    if (i == menu_current)
    { // current selected menu item
      u8g.drawBox(0, i * h + 1, w, h); // draw cursor bar
      u8g.setDefaultBackgroundColor();
    }
    u8g.drawStr(d, i * h, menu_strings[i]);
  }
}

void updateMenu(void)
{
  if (down)
  {
    menu_current++;
    if (menu_current >= MENU_ITEMS)
      menu_current = 0;
    menu_redraw_required = 1;
  }
  else if (up)
  {
    if (menu_current == 0)
      menu_current = MENU_ITEMS;
    menu_current--;
    menu_redraw_required = 1;
  }
  else if (push) {
    if (menu_current == 0){
      gameManager.setGameStatus(STATUS_PLAYING);
    }
    else if (menu_current == 1) {
      //game load
    }
    else if (menu_current == 2) gameManager.setGameStatus(STATUS_RECORD);
  }
}

void initInputs() {
  up = false;
  right = false;
  left = false;
  down = false;
  push = false;
  aBut = false;
  bBut = false;
}

void drawMap(){
  for(int i=0; i<BLOCK_X_NUM; i++){
    for(int j=0; j<BLOCK_Y_NUM; j++){
      u8g.drawFrame(MAP_LEFT_MARGIN + i*BLOCK_WIDTH,j*BLOCK_HEIGHT, 16, 16);    
    }
  }
}

void drawMines(){
  for(int i=0; i<BLOCK_X_NUM; i++){
    for(int j=0; j<BLOCK_Y_NUM; j++){
      u8g.drawXBM(MAP_LEFT_MARGIN + 4 + i*BLOCK_WIDTH,j*BLOCK_HEIGHT + 4, MINE_WIDTH, MINE_HEIGHT, mineXBM); 
    }
  }
  
}




