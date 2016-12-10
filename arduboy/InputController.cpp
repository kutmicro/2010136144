#include "InputController.h"


InputController::InputController(){

}


/*uint8_t InputController::getButtonA(){
  uint8_t value = 0;
    if (digitalRead(BUTTON_A_PIN) == LOW){
        value =  0;
    } else{
        value =  1;
    }

    return value;
}
*/
int InputController::getInput(){
  int input = analogRead(JOYSTICK_PIN);

  return input;
}
