#include "display.h"

void display_pomodoro_init (void){

}


void display_pomodoro_fsm(display_t* display){
  
  switch(display->display_pomodoro_state){
    case DISPLAY_POMODORO_IDLE:
      break;

    case DISPLAY_POMODORO_START:
      break;

    case DISPLAY_POMODORO_TIME_ALMOST_UP:  
      break;

    case DISPLAY_POMODORO_TIME_UP:
      break; 

    case DISPLAY_POMODORO_FINISH:
      break;

    default:
      break;
  }
}


