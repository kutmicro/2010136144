#include <EEPROM.h>
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

//game datas
#define EEP_SAVED_ADD 0
#define EEP_BEST_ADD 1

int lastInput;
unsigned long lTime;
int gameFps = 1000 / 30;

int bestScore; //최고기록, 시작시 EEPROM에서 로드
int savedScore; //마지막으로 저장한 기록, 시작 메뉴에서 불러오기 선택시 로드

int startRow;   //player point
int startCol;
int endRow;     //end point
int endCol;
int currentRow; //current Point
int currentCol;


#define MENU_ITEMS 3
#define SAVE_MENU_ITEMS 2
char *menu_strings[MENU_ITEMS] = {"New Game", "Continue Game", "Best Score"};
char *save_menu_strings[SAVE_MENU_ITEMS] = {"Continue", "Save & Quit"};

uint8_t menu_current = 0;
int menu_redraw_required = 0;

//methods
//menu methods
void drawMenu();
void updateMenu();


static int mines[MINE_ROW][MINE_COL] = {{0}};

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
//  Serial.println(EEPROM.length());  -> 4096 = 4KB
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


    //메뉴화면
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
      int stageStatus = gameManager.getStageStatus();

      //스테이지 정보 보여주는 화면 ex) Stage 1, Stage 2
      if (stageStatus == STAGE_INFO) {
        //show stage info
        u8g.firstPage();
        do {
          drawStageIntro();
        } while (u8g.nextPage());

        gameManager.nextStageStatus();
        delay(1500);
      }
      //지뢰 생성
      else if (stageStatus == STAGE_INIT_MINE) {
        initMines();
        gameManager.nextStageStatus();
      }
      //지뢰 3초동안 보여주기
      else if (stageStatus == STAGE_SHOW_MINE) {
        u8g.firstPage();
        do {
          drawMap();
          drawMines();
        } while (u8g.nextPage());

        delay(700);
        gameManager.nextStageStatus();
        gameManager.nextStageStatus();
      }
      //지뢰 가리기
      else if (stageStatus == STAGE_PLAYING) {
        //플레이어 이동 처리
        movePlayer();
        //이동한 지점이 지뢰인지, 도착점인지 체크
        int result = checkArea();

        if (result == MINE) {
          //이동한 지점이 지뢰이면
          //모든 지뢰의 위치 보여줌
          u8g.firstPage();
          do {
            drawMap();
            drawMines();
          } while (u8g.nextPage());
          //점수를 보여주고
          //기록을 경신했는지 확인
          //기록을 경신했으면 기록하는 화면을 띄워줌
          gameManager.setGameStatus(STATUS_RECORD);
          //          gameManager.setGameStatus()
        } else if (result == END) {
          //도착 지점에 도착하면 지도와 지뢰의 위치, 도착지점 보여줌
          u8g.firstPage();
          do {
            drawMap();
            drawMines();
          } while (u8g.nextPage());
          //다음 스테이지로 이동
          //1초동안 보여줌
          delay(1000);

          gameManager.clearStage(); //stage +1 증가
          gameManager.nextStageStatus(); //
        } else {
          //움직인 위치가 지뢰, 도착점이 아니면 그냥 플레이어를 표시
          u8g.firstPage();
          do {
            drawMap();
            drawPlayer();
          } while (u8g.nextPage());
        }
      }
    }
    //결과 화면
    else if (gameManager.getGameStatus() == STATUS_RESULT)
    {
      //결과를 출력해줌
    }
    //기록 화면
    else if (gameManager.getGameStatus() == STATUS_RECORD)
    {
      //기록 입력하는 화면 띄워줌
      drawScore();
    }
    else if(gameManager.getGameStatus() == STATUS_PAUSE)
    {
      updatePauseMenu(); // outside picture loop
      if (menu_redraw_required != 0)
      {
        u8g.firstPage();
        do
        {
          drawPauseMenu(); // inside picture loop
        } while (u8g.nextPage());
        menu_redraw_required = 0; // menu updated, reset redraw flag
      }
    }

    initInputs();
  }
}

//draw_menu
void drawMenu()
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

void drawPauseMenu(){
  uint8_t i, h;
  u8g_uint_t w, d;
  u8g.setFont(u8g_font_6x13);
  u8g.setFontRefHeightText();
  u8g.setFontPosTop();
  h = u8g.getFontAscent() - u8g.getFontDescent();
  w = u8g.getWidth();
  for (i = 0; i < SAVE_MENU_ITEMS; i++)
  { // draw all menu items
    d = (w - u8g.getStrWidth(save_menu_strings[i])) / 2;
    u8g.setDefaultForegroundColor();
    if (i == menu_current)
    { // current selected menu item
      u8g.drawBox(0, i * h + 1, w, h); // draw cursor bar
      u8g.setDefaultBackgroundColor();
    }
    u8g.drawStr(d, i * h, save_menu_strings[i]);
  }
}

void updatePauseMenu(void)
{
  if (down)
  {
    menu_current++;
    if (menu_current >= SAVE_MENU_ITEMS)
      menu_current = 0;
    menu_redraw_required = 1;
  }
  else if (up)
  {
    if (menu_current == 0)
      menu_current = SAVE_MENU_ITEMS;
    menu_current--;
    menu_redraw_required = 1;
  }
  else if (push)
  {
    if (menu_current == 0)
    {
      //계속 하기 선택
      gameManager.setGameStatus(STATUS_PLAYING);
    }
    else if (menu_current == 1)
    {
      //저장하기 선택
      EEPROM.write(EEP_SAVED_ADD, gameManager.getStage());
      if(savedScore<1) savedScore=1;
      //로딩 완료 후 플레이 화면으로 이동
      gameManager.setGameStatus(STATUS_MENU);
      menu_current=0;
      menu_redraw_required = 1;
    }
  }
}

void drawScore(){
  int score = gameManager.getStage(); //지금 몇 스테이지인지 가져옴
  //TODO int to char*
  u8g.drawStr(20, 20, "Score : ");
//  u8g.drawStr(70, 20, score);
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
  else if (push)
  {
    if (menu_current == 0)
    {
      //새로운 게임 선택
      gameManager.setGameStatus(STATUS_PLAYING);
    }
    else if (menu_current == 1)
    {
      savedScore = EEPROM.read(EEP_SAVED_ADD);
      if(savedScore<1) savedScore=1;

      //불러온 스테이지로 설정
      gameManager.setStage(savedScore);
      //로딩 완료 후 플레이 화면으로 이동
      gameManager.setGameStatus(STATUS_PLAYING);
    }
    else if (menu_current == 2)
    {
      //최고 기록을 보여줌
      gameManager.setGameStatus(STATUS_RECORD);
    }
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

void drawMap() {
  for (int i = 0; i < MINE_ROW; i++) {
    for (int j = 0; j < MINE_COL; j++) {
      u8g.drawFrame(MAP_LEFT_MARGIN + j * BLOCK_HEIGHT, i * BLOCK_WIDTH, BLOCK_WIDTH, BLOCK_HEIGHT);
    }
  }
}

void drawMines() {
  for (int i = 0; i < MINE_ROW; i++) {
    for (int j = 0; j < MINE_COL; j++) {
      if (mines[i][j] == MINE) {
        u8g.drawXBM(MAP_LEFT_MARGIN + 4 + j * BLOCK_WIDTH, i * BLOCK_HEIGHT + 4, MINE_WIDTH, MINE_HEIGHT, mineXBM);
      }
      else if (mines[i][j] == START) {
        u8g.drawBox(MAP_LEFT_MARGIN + j * BLOCK_WIDTH, i * BLOCK_HEIGHT, 16, 16);
      }
      else if (mines[i][j] == END) {
        u8g.drawXBM(MAP_LEFT_MARGIN + 4 + j * BLOCK_WIDTH, i * BLOCK_HEIGHT + 4, MINE_WIDTH, MINE_HEIGHT, endPointXBM);
      }
    }
  }
}

void drawPlayer() {
  u8g.drawBox(MAP_LEFT_MARGIN + currentCol * BLOCK_WIDTH, currentRow * BLOCK_HEIGHT, 16, 16);
}

void movePlayer() {
  if (left && currentCol > 0) {
    currentCol--;
  } else if (right && currentCol < MINE_COL - 1) {
    currentCol++;
  } else if (up && currentRow > 0) {
    currentRow--;
  } else if (down && currentRow < MINE_ROW - 1) {
    currentRow++;
  } else if (push){
    gameManager.setGameStatus(STATUS_PAUSE);
    menu_current = 0;
    menu_redraw_required = 1;
  }
}

int checkArea() {
  int currentArea = mines[currentRow][currentCol];
  if (currentArea == EMPTY || currentArea == START) {
    return EMPTY;
  } else if (currentArea == MINE) {
    return MINE;
  } else if (currentArea == END) {
    return END;
  }
}


//스테이지 시작 전 몇 스테이지인지 보여주는 화면
void drawStageIntro() {
  int stage = gameManager.getStage();
  String stageStr = String(stage);
  String text = String("Stage ");

  text.concat(stageStr);

  u8g.drawStr(20, 20, text.c_str());
}

void initMines() {
  randomSeed(millis());
  int s_i = random(0, 4); //0~3의 숫자는 시작 지점을 정하는 숫자
  int s_j = 0;
  int e_i;

  int e_j = MINE_COL - 1;

  do {
    e_i = random(0, 4);
  } while (s_i == e_i);

  //배열 초기화
  for(int i=0; i<MINE_ROW; i++){
    for(int j=0; j<MINE_COL; j++){
      mines[i][j] = EMPTY;
    }
  }

  //시작지점과 도착지점 설정
  mines[s_i][s_j] = START;
  mines[e_i][e_j] = END;

  //시작지점과 도착지점을 변수로 저장
  startRow = s_i;
  startCol = s_j;
  endRow = e_i;
  endCol = e_j;
  currentCol = s_j; //시작지점을 현재 위치로 설정
  currentRow = s_i;


  int k = random(1, 4);
  for (int j = 1; j < 4; j++) {
    int i = random(0, 4);
    mines[i][j] = MINE;
    if (k == j) {
      mines[(i + 2) % 4][j] = MINE;
    }
  }
}
