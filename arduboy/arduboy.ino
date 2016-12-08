#include <U8glib.h>
#include "InputController.h"
#include "Block.h"

U8GLIB_NHD_C12864 u8g(13, 11, 10, 9, 8); // SPI Com: SCK = 13, MOSI = 11, CS = 10, CD = 9, RST = 8

//Input
InputController inputController;

#define STATUS_MENU 0
#define STATUS_PLAYING 1
#define STATUS_RESULT 2
#define STATUS_RECORD 3

#define MENU_ITEMS 3
char *menu_strings[MENU_ITEMS] = {"Game Start", "Load Game", "Record"};
uint8_t menu_current = 0;
int menu_redraw_required = 0;

//init game status
int gameStatus = STATUS_MENU;

int level = 1;
int balls = 1;
int ballPointX = 64;
int ballPointY = 64;

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

  u8g.setContrast(0); // Config the contrast to the best effect
  u8g.setRot180();    // rotate screen, if required

        u8g.firstPage();
      do
      {
        drawMenu();
      } while (u8g.nextPage());//      u8g.firstPage();

}

void loop(void)
{
  //check game status
  //if gameStatus is menu
  //  if (gameStatus == STATUS_MENU)
  //  {
  //    if (menu_redraw_required != 0) {
  //      u8g.firstPage();
  //      do
  //      {
  //        drawMenu();
  //      } while (u8g.nextPage());
  //    }
  //    updateMenu();
  //  }
  //  else if (gameStatus == STATUS_PLAYING)
  //  {
  //
  //  }
  //  else if (gameStatus == STATUS_RECORD)
  //  {
  //
  //  }
  //  else if (gameStatus == STATUS_RESULT) {
  //
  //  }
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
  int input = inputController.getInput();
  if (input == DOWN ) {
    menu_current++;
    if ( menu_current >= MENU_ITEMS )
      menu_current = 0;
    menu_redraw_required = 1;
  }
  else if (input == UP ) {
    if ( menu_current == 0 )
      menu_current = MENU_ITEMS;
    menu_current--;
    menu_redraw_required = 1;
  }
}

