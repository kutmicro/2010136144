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

#define PIEZO_PIN 4

//game datas
#define EEP_SAVED_ADD 0

char bestUsersNames[3][3]; //EEPROM 주소 1번~9번
char bestUserName[3];
char name[3] = {'A', 'A', 'A'};

int bestUserPoints[3]; // EEPROM 주소 10번 ~ 12번

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

int name_current = 0;

#define MENU_ITEMS 3
#define SAVE_MENU_ITEMS 2
#define GAMEOVER_MENU_ITEMS 2

char *menu_strings[MENU_ITEMS] = {"New Game", "Continue Game", "Best Score"};
char *save_menu_strings[SAVE_MENU_ITEMS] = {"Continue", "Save & Quit"};
char *game_over_menu_strings[SAVE_MENU_ITEMS] = {"Retry", "Quit"};

uint8_t menu_current = 0;
int menu_redraw_required = 1;



//methods
//menu methods
void drawMenu();
void updateMenu();


static int mines[MINE_ROW][MINE_COL] = {{0}};

void draw(void)
{
  // graphic commands to redraw the complete screen should be placed here
  u8g.setFont(u8g_font_6x13);
}

void setup(void)
{
  //Menu 선택화면 띄우기
  u8g.setContrast(0); // Config the contrast to the best effect
  u8g.setRot180();    // rotate screen, if required
  u8g.setFont(u8g_font_6x13);

  pinMode(PIEZO_PIN, OUTPUT);
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

    if(left || right || up || down || push){
      tone(PIEZO_PIN, 600, 100);
    }


    //메뉴화면
    if (gameManager.getGameStatus() == STATUS_MENU)
    {
      // outside picture loop
      u8g.firstPage();
      do
      {
        drawMenu(); // inside picture loop
      } while (u8g.nextPage());
      updateMenu();
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
        int countMine=0;
        
        do{
          countMine=0;
          initMines();

          for(int i=0; i<MINE_ROW; i++){
            for(int j=0; j<MINE_COL; j++){
              if(mines[i][j]==MINE) countMine++;
            }
          }
        } while(countMine<4 || countMine > 6);
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

        u8g.firstPage();
        do {
          drawMap();
        } while (u8g.nextPage());
        delay(2000);

        u8g.firstPage();
        do {
          drawMap();
          drawEndPoint();
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

          soundSad(PIEZO_PIN);
          delay(1000);

          //최종 점수 보여주기
          u8g.firstPage();
          do {
            drawScore();
          } while (u8g.nextPage());

          delay(2000);

          //기록 불러오기
          loadBestScores();

          //현재 기록이랑 비교
          if (isBestScore(gameManager.getStage())) {
            //최고 기록이면 기록화면으로
            gameManager.setGameStatus(STATUS_HIGH_SCORE);
          } else {
            gameManager.setGameStatus(STATUS_GAMEOVER);
          }
          //아니면 그냥 결과 보여줌


        } else if (result == END) {
          //도착 지점에 도착하면 지도와 지뢰의 위치, 도착지점 보여줌
          u8g.firstPage();
          do {
            drawMap();
            drawMines();
          } while (u8g.nextPage());

          soundVictory(PIEZO_PIN);
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
    //기록 화면
    else if (gameManager.getGameStatus() == STATUS_RECORD)
    {
      if (push) {
        gameManager.setGameStatus(STATUS_MENU);
      }

      u8g.firstPage();
      do {
        drawHighScoreList();
      } while (u8g.nextPage());
    }
    else if (gameManager.getGameStatus() == STATUS_PAUSE)
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
    else if (gameManager.getGameStatus() == STATUS_GAMEOVER)
    {
      updateGameOverMenu(); // outside picture loop

      u8g.firstPage();
      do
      {
        drawGameOverMenu(); // inside picture loop
      } while (u8g.nextPage());
    }
    else if (gameManager.getGameStatus() == STATUS_HIGH_SCORE)
    {
      updateHighScoreMenu();

      u8g.firstPage();
      do {
        drawHighScoreMenu();
      } while (u8g.nextPage());

    }


    initInputs();
  }
}


void drawHighScoreList() {
  for (int i = 0; i < 3; i++) {
    if(i==0){
      u8g.drawStr(22 , 10 + i * 15, "1st");
    } else if(i==1){
      u8g.drawStr(22 , 10 + i * 15, "2nd");
    } else if(i==2){
      u8g.drawStr(22 , 10 + i * 15, "3rd");
    }

    if(bestUserPoints[i]==0) continue;
    for (int j = 0; j < 3; j++) {
      u8g.drawStr(48 + j * 8 , 10 + i * 15, String(bestUsersNames[i][j]).c_str());
    }
    u8g.drawStr(80, 10 + i * 15, String(bestUserPoints[i]).c_str());
  }
}


//high score menu
void updateHighScoreMenu() {
  if (up) {
    if (name_current >= 0 && name_current <= 2) {
      name[name_current] += 1;
    }
  } else if (down) {
    if (name_current >= 0 && name_current <= 2) {
      name[name_current] -= 1;
    }
  } else if (left) {
    if (name_current != 0) {
      name_current--;
    }
  } else if (right) {
    if (name_current != 3) {
      name_current++;
    }
  } else if (push) {
    if (name_current == 3) {
      //저장하고 기록 화면으로
      //      String name = String(name[0]);
      //      name.concat(name[1]);
      //      name.concat(name[2]);
      //유저 이니셜은 1~9번 주소에 1바이트당 1캐릭터씩 저장되어있음.
      for (int i = 0; i < 3; i++) {
        bestUsersNames[2][i] = name[i];
      }
      //유저 점수는 10~12에 저장되어있음
      bestUserPoints[2] = gameManager.getStage();
      writeScoresToEEPROM();
      //init name_current
      name_current = 0;
      //데이터 로드 후 최고기록 목록을 보여줌
      loadBestScores();
      gameManager.setGameStatus(STATUS_RECORD);
    }
  }
}

void writeScoresToEEPROM(){
  int k=1;
  for(int i=0; i<3; i++){
      EEPROM.write(i+10, bestUserPoints[i]);
      for(int j=0; j<3; j++){
        EEPROM.write(k, bestUsersNames[i][j]);
        k++;
      }
  }
}

void drawHighScoreMenu() {
  for (int i = 0; i < 3; i++) {
    u8g.setDefaultForegroundColor();
    if (i == name_current) {
      u8g.drawBox(23 + i * 10, 20, 8, 10);
      u8g.setDefaultBackgroundColor();
    }
    u8g.drawStr(25 + i * 10, 20, String(name[i]).c_str());
  }

  u8g.setDefaultForegroundColor();

  if (name_current == 3) {
    u8g.drawBox(54, 20, 24, 10);
    u8g.setDefaultBackgroundColor();
  }
  u8g.drawStr(55, 20, "Done");
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

void drawPauseMenu() {
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

void drawGameOverMenu() {
  uint8_t i, h;
  u8g_uint_t w, d;
  u8g.setFont(u8g_font_6x13);
  u8g.setFontRefHeightText();
  u8g.setFontPosTop();
  h = u8g.getFontAscent() - u8g.getFontDescent();
  w = u8g.getWidth();
  for (i = 0; i < GAMEOVER_MENU_ITEMS; i++)
  { // draw all menu items
    d = (w - u8g.getStrWidth(game_over_menu_strings[i])) / 2;
    u8g.setDefaultForegroundColor();
    if (i == menu_current)
    { // current selected menu item
      u8g.drawBox(0, i * h + 1, w, h); // draw cursor bar
      u8g.setDefaultBackgroundColor();
    }
    u8g.drawStr(d, i * h, game_over_menu_strings[i]);
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
      if (savedScore < 1) savedScore = 1;

      //로딩 완료 후 플레이 화면으로 이동
      menu_redraw_required = 1;
      menu_current = 0;
      gameManager.setGameStatus(STATUS_MENU);
    }
  }
}

void updateGameOverMenu() {
  if (down)
  {
    menu_current++;
    if (menu_current >= GAMEOVER_MENU_ITEMS)
      menu_current = 0;
    menu_redraw_required = 1;
  }
  else if (up)
  {
    if (menu_current == 0)
      menu_current = GAMEOVER_MENU_ITEMS;
    menu_current--;
    menu_redraw_required = 1;
  }
  else if (push)
  {
    if (menu_current == 0)
    {
      //재시작 선택
      gameManager.setGameStatus(STATUS_PLAYING);
      gameManager.setStageStatus(STAGE_INFO);
      gameManager.setStage(1);

      //게임 데이터 초기화

    }
    else if (menu_current == 1)
    {
      //종료 선택
      //메뉴 화면으로 이동
      menu_redraw_required = 1;
      menu_current = 0;
      gameManager.setGameStatus(STATUS_MENU);

    }
  }
}

void drawScore() {
  int score = gameManager.getStage(); //지금 몇 스테이지인지 가져옴
  String scoreStr = String(score);
  String str = String("Your Score is ");
  str.concat(scoreStr);

  u8g.drawStr(25, 32, str.c_str());
  u8g.drawStr(25, 18, "Game Over");
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
      gameManager.setStage(1);
      gameManager.setGameStatus(STATUS_PLAYING);
      gameManager.setStageStatus(STAGE_INFO);
    }
    else if (menu_current == 1)
    {
      savedScore = EEPROM.read(EEP_SAVED_ADD);
      if (savedScore < 1) {
        u8g.firstPage();
        do {
          u8g.drawStr(25, 20, "No saved data!");
        }
        while (u8g.nextPage());

        delay(1200);

        return;
      }
      //불러오기 완료 후 저장된 데이터 지워주기
      EEPROM.write(EEP_SAVED_ADD, 0);

      //불러온 스테이지로 설정
      gameManager.setStage(savedScore);
      //로딩 완료 후 플레이 화면으로 이동
      gameManager.setGameStatus(STATUS_PLAYING);
      gameManager.setStageStatus(STAGE_INFO);
    }
    else if (menu_current == 2)
    {
      //최고 기록을 보여줌
      loadBestScores();
      u8g.setDefaultForegroundColor();
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
    }
  }
}

void drawEndPoint() {
  for (int i = 0; i < MINE_ROW; i++) {
    for (int j = 0; j < MINE_COL; j++) {
      if (mines[i][j] == START) {
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
  } else if (push) {
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
  //  std::string s = std::to_string(stage);
  //  char const *pchar = s.c_str();  //use char const* as target type
  String stageStr = String(stage);
  String text = String("Stage ");
  text.concat(stageStr);
  u8g.drawStr(25, 20, text.c_str());
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

  do {
    e_j = random(3, 5);
  } while (s_j == e_j);

  //배열 초기화
  for (int i = 0; i < MINE_ROW; i++) {
    for (int j = 0; j < MINE_COL; j++) {
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


  int k = random(0, 20);
  for (int i = 0; i < MINE_ROW; i++) {
    for (int j = 0; j < MINE_COL; j++) {
      if(mines[i][j] == EMPTY && k >15){
        mines[i][j] = MINE;
      }
      k = random(0, 20);
    }
  }
}

void loadBestScores() {
  int k=1;



  //로딩 완료
  for (int i=0; i<3; i++){
    bestUserPoints[i] = EEPROM.read(i+10);
    for (int j=0; j<3; j++){
        bestUsersNames[i][j] = EEPROM.read(k);
        k++;
    }
  }

  Serial.println(bestUserPoints[0]);
  Serial.println(bestUserPoints[1]);
  Serial.println(bestUserPoints[2]);

  //정렬
  for (int i=2; i>0; i--){
    if(bestUserPoints[i-1] < bestUserPoints[i]){
      int temp = bestUserPoints[i-1];
      bestUserPoints[i-1] = bestUserPoints[i];
      bestUserPoints[i] = temp;

      for (int j=0; j<3; j++){
        char temp = bestUsersNames[i][j];
        bestUsersNames[i][j] = bestUsersNames[i-1][j];
        bestUsersNames[i-1][j] = temp;
      }
    }
  }
}

bool isBestScore(int score) {
  Serial.println(score);
  Serial.println(bestUserPoints[0]);

  if (score > bestUserPoints[0]) {
    return true;
  }
  return false;
}


void soundVictory(int pinNo) {
 tone(pinNo, 523, 200);
  delayMicroseconds(1000 - 200);

  tone(pinNo, 523, 200);
  delayMicroseconds(1000 - 200);

  tone(pinNo, 523, 200);
  delayMicroseconds(1000 - 200);

  tone(pinNo, 659, 700);
  tone(pinNo, 784, 500);

  tone(pinNo, 523, 200);
  delayMicroseconds(1000 - 200);

  tone(pinNo, 523, 200);
  delayMicroseconds(1000 - 200);

  tone(pinNo, 523, 200);
  delayMicroseconds(1000 - 200);

  tone(pinNo, 659, 700);

  tone(pinNo, 784, 500);
  delayMicroseconds(800 - 500);

  tone(pinNo, 784, 400);
  tone(pinNo, 884, 200);
  tone(pinNo, 784, 200);
  tone(pinNo, 687, 200);
  tone(pinNo, 659, 200);
  tone(pinNo, 519, 400);
}


void soundSad(int pinNo) {
  tone(pinNo, 784, 500);
  tone(pinNo, 738, 500);
  tone(pinNo, 684, 500);
  tone(pinNo, 644, 1000);
}

