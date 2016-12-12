#include "GameManager.h"


GameManager::GameManager() {

}

int GameManager::getGameStatus() {
  return gameStatus;
}

void GameManager::setGameStatus(int gameStatus) {
  this->gameStatus = gameStatus;
}


int GameManager::getStage() {
  return stage;
}

int GameManager::getStageStatus() {
  return stageStatus;
}

void GameManager::nextStageStatus(){
  if(stageStatus == STAGE_PLAYING){
    stageStatus = STAGE_INFO;
  } else{
    stageStatus++;
  }
}

void GameManager::clearStage() {
  stage++;
}

