#include <pebble.h>
#define ALPHA 0.8
#define UPPER_THRESHOLD 2500
#define LOWER_THRESHOLD 50
#define TIME_INTERVAL 0.01 //frequency

typedef enum {INITIAL, CALIBRATION, STANDBY, PUNCH, RECOIL, ON_PAUSE} state_type;

state_type state = INITIAL;

int weight = 0;

long gravityStat[3] = {0,0,0};
int gravityStatNum = -200;//frequency

int currentVelocityI = 0;
int historyAcceleration[1000];//frequency
int historyAccSize = 0;

char pushing = 0;
int maxStopAcceleration = 0;

float my_sqrt(const float num) {
  const uint MAX_STEPS = 40;
  const float MAX_ERROR = 0.001;
  
  float answer = num;
  float ans_sqr = answer * answer;
  uint step = 0;
  while((ans_sqr - num > MAX_ERROR) && (step++ < MAX_STEPS)) {
    answer = (answer + (num / answer)) / 2;
    ans_sqr = answer * answer;
  }
  return answer;
}

static void data_handler(AccelData *data, uint32_t num_samples) {
  if(state==INITIAL){//before click select button
      static char s_buffer[200];
      snprintf(s_buffer, sizeof(s_buffer),
               "Please press middle button\nand get into boxing stance. Punch 2 seconds after vibration."
      );
      set_text(s_buffer);
  }else if(state==CALIBRATION){//after click select button, before finishes calibration
    if(gravityStatNum<0) {gravityStatNum++; return;}
    if(gravityStatNum==0) vibes_short_pulse();
    gravityStat[0] += data[0].x;
    gravityStat[1] += data[0].y;
    gravityStat[2] += data[0].z;
    gravityStatNum++;
    if(gravityStatNum==200){
      gravityStat[0] = gravityStat[0]/gravityStatNum;
      gravityStat[1] = gravityStat[1]/gravityStatNum;
      gravityStat[2] = gravityStat[2]/gravityStatNum;
      state = PUNCH;
    }
  }else if(state == PUNCH){//after calibration, acceleration
    set_text("Punching...");
    data[0].x = data[0].x - gravityStat[0];
    data[0].y = data[0].y - gravityStat[1];
    data[0].z = data[0].z - gravityStat[2];
    historyAcceleration[historyAccSize] = my_sqrt(data[0].x*data[0].x+data[0].y*data[0].y+data[0].z*data[0].z);
    currentVelocityI += historyAcceleration[historyAccSize]/100.0 * TIME_INTERVAL;
    historyAccSize++;
    if((!pushing) && historyAcceleration[historyAccSize-1]>1000) pushing = true;
    if((pushing && historyAcceleration[historyAccSize-1]<1000) ||
       historyAccSize == 998){//check where it starts decelerating
      state = RECOIL;
      maxStopAcceleration = historyAccSize-1;
    }
  }else if(state == RECOIL){//deceleration
    set_text("Recoil.");
    data[0].x = data[0].x - gravityStat[0];
    data[0].y = data[0].y - gravityStat[1];
    data[0].z = data[0].z - gravityStat[2];
    historyAcceleration[historyAccSize] = my_sqrt(data[0].x*data[0].x+data[0].y*data[0].y+data[0].z*data[0].z);
    currentVelocityI -= historyAcceleration[historyAccSize++]/100.0 * TIME_INTERVAL;
    if(currentVelocityI <= 0 || historyAccSize == 999){
      int maxAcc = 0;
      for(int i=maxStopAcceleration-1; i<historyAccSize; i++){
        if(historyAcceleration[i]>maxAcc) maxAcc = historyAcceleration[i];
      }
      static char s_buffer[128];
      snprintf(s_buffer, sizeof(s_buffer),
               "maximum acceleration:\n%d m/s2.\nPress middle button to play again.",
               (int)(maxAcc / 100.00)
      );
      set_text(s_buffer);
//      accel_data_service_unsubscribe();
      state = ON_PAUSE;
    }
  }else if(state == ON_PAUSE){
    
  }
  
}

void select_single_click_handler(ClickRecognizerRef recognizer, void *context){
  state = CALIBRATION;
  set_text("stand by for calibration");
  gravityStatNum=-200;
  currentVelocityI = 0;
  historyAccSize = 0;
  gravityStat[0] = 0;
  gravityStat[1] = 0;
  gravityStat[2] = 0;
  maxStopAcceleration = 0;
  pushing = false;
}

void up_single_click_handler(ClickRecognizerRef recognizer, void *context){
  weight+=1;
}
void down_single_click_handler(ClickRecognizerRef recognizer, void *context){
  if(weight>0)weight-=1;
}

void config_provider(Window *window) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_single_click_handler);
}


void handle_init(Window *window) {
  //attach accelerometer handler
  uint32_t num_samples = 1;
  accel_data_service_subscribe(num_samples, data_handler);
  accel_service_set_sampling_rate(ACCEL_SAMPLING_100HZ);
  
  window_set_click_config_provider(window, (ClickConfigProvider) config_provider);
}

void handle_deinit(Window *window) {
}