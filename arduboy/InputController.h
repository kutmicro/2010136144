#include <Arduino.h>


//pin define
#define JOYSTICK_PIN 0
#define BUTTON_A_PIN 2
#define BUTTON_B_PIN 32

//direction value define
#define LEFT 0
#define RIGHT 627
#define UP 825
#define DOWN 409
#define PUSH 205

class InputController{
  public:
    InputController();
    int getInput();
  private:
};
