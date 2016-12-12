#include "InputController.h"


InputController::InputController(){

}

int InputController::getInput(){
  int input = analogRead(JOYSTICK_PIN);

  return input;
}
