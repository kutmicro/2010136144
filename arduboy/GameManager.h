#include <Arduino.h>
#define STATUS_MENU 0
#define STATUS_PLAYING 1
#define STATUS_RESULT 2
#define STATUS_RECORD 3
#define STATUS_PAUSE 4
#define STATUS_GAMEOVER 5
#define STATUS_HIGH_SCORE 6

#define MAP_LEFT_MARGIN 24
#define BLOCK_WIDTH 16
#define BLOCK_HEIGHT 16

#define STAGE_INFO 0
#define STAGE_INIT_MINE 1
#define STAGE_SHOW_MINE 2
#define STAGE_HIDE_MINE 3
#define STAGE_PLAYING 4

#define EMPTY 0
#define START 1
#define END 2
#define MINE 3
#define ROAD 4

#define MINE_ROW 4
#define MINE_COL 5

class GameManager {
  private:
    int gameStatus = STATUS_MENU;
    int stage = 1;
    int stageStatus;
  public:
    GameManager();
    int getGameStatus();
    int getStageStatus();
    void nextStageStatus();
    void setGameStatus(int gameStatus);
    void setStageStatus(int stageStatus);
    void drawMap();
    int getStage();
    void setStage(int stage);
    void clearStage();
};
