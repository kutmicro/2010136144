#include "GameManager.h"


GameManager::GameManager(){
  
}

int GameManager::getGameStatus(){
    return gameStatus;
}

void GameManager::setGameStatus(int gameStatus){
    this->gameStatus = gameStatus; 
}
