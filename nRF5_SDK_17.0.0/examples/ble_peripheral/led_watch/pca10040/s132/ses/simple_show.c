#include "display.h"

extern const location_t posi1, posi2, posi3, posi4;

void just_show_start(display_t* display){
  display_set_time(display, posi1, posi2, posi3, posi4);
  display_show(display);
}

const display_watch_animation_t simple_show_anim = {
  .fcall  = just_show_start
};

