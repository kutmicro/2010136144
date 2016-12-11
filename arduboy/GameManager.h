#define STATUS_MENU 0
#define STATUS_PLAYING 1
#define STATUS_RESULT 2
#define STATUS_RECORD 3

#define MAP_LEFT_MARGIN 24
#define BLOCK_WIDTH 16
#define BLOCK_HEIGHT 16

#define BLOCK_X_NUM 5
#define BLOCK_Y_NUM 4 


class GameManager{
  private:
    int gameStatus = STATUS_MENU;
    bool mines[5][4];
    int stage;
  public:
    GameManager();
    int getGameStatus();
    void setGameStatus(int gameStatus);
    void drawMap();
};
