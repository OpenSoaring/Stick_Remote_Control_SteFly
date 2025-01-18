// remote control for XCSoar, emulates a keyboard and mouse
// hardware is just pushbuttons connected between pins of an Arduino Leonardo and Gnd
// for each button press a keystroke or mouse action is sent
// Button layout Stefly Remote (5 button type) as shown on http://www.openvario.org/doku.php?id=projects:remote_00:top 
// additional Speed to Fly switch or button between Arduino pin 7 and GND

// uses libraries from
// https://github.com/r89m/PushButton 
// https://github.com/r89m/Button
// https://github.com/thomasfredericks/Bounce2

#include <Mouse.h>
#include <Keyboard.h>
#include <Button.h>
#include <ButtonEventCallback.h>
#include <PushButton.h>
#include <Bounce2.h>

#include "avr/wdt.h"

#define VERBOSE  2
// disable the next define if you have a BUTTON instead a TOGGLE SWITCH: 
// #define USE_STF_SWITCH

// define press (short press) and hold (long press) functions for each button
// settings in XCSoar default.xci are
// F1 QuickMenu
// F2 Analysis
// F3 Checklist
// F4 FlarmTraffic
// F5 GotoLookup (Select Waypoint)
// F6 Setup Alternates
// F7 Setup Task
// F8 Setup Basic (wingload,bugs, QNH)
// F9
//    Status all
//    Calculator (tasks)
//
// settings in openvario.xci
// V for vario mode
// S for speed to fly mode
// M for vario menu (audio)
// Q to quit


// define timing for buttons etc.
const int Mouse_Move_Distance = 1;
const int joy_rebounce_interval = 3;
const int joy_key_rebounce_threshold = 20;
const int joy_key_first_pressed_threshold = 100;
const int button_hold_threshold = 900;
const int button_long_hold = 5000;  // = 5 seconds
const int joy_hold_threshold = 1;
//const int debounce_delay = 10;                   //Debounce delay in milliseconds, not used

// define variables
boolean mouse_active = 0;
boolean first_pressed = 1;
int joy_key_counter = 0;
int joy_counter[10] = {0,0,0,0,0,0,0,0,0,0};
int btn_timer[10] = {0,0,0,0,0,0,0,0,0,0};

unsigned long timeNow = 0;
unsigned long timeWDT = 0;

// Create instances of PushButtons on digital pins
enum {btnJoy, btnTopLeft, btnTopRight, btnBottomRight, btnBottomLeft, 
      joyUp, joyDown, joyLeft, joyRight, btnSTF, btnSize};

const uint8_t pullup = ENABLE_INTERNAL_PULLUP;
PushButton button[] = {
    { 6, pullup}, // btnJoy: joystick button
    { 9, pullup}, // btnTopLeft: mouse switch (and 'P')
    {14, pullup}, // btnTopRight: QuickMenu and AudioMenue
    {15, pullup}, // btnBottomRight: ESC and 'Q'uit
    { 0, pullup}, // btnBottomLeft: PTT -> not connected
    { 2, pullup}, // joyUp
    { 4, pullup}, // joyDown
    { 5, pullup}, // joyLeft
    { 3, pullup}, // joyRight
    { 7, pullup}  // btnSTF
};
enum {JOY_KEY, JOY_X, JOY_Y};
enum {KEY_PRESSED, KEY_LONG, KEY_VERYLONG};
const int event[][3] = {
   // short pressed    >= 1 Second           very long (not used up to now) 
    { KEY_RETURN,      '>' /* 0 */,          0},                     // btnJoy: joystick button
    { 0,               'P' /*KEY_F2*/,       0},                     // btnTopLeft: mouse switch (and 'P')
    { KEY_F1,          'M',                  0},                     // btnTopRight: QuickMenu and AudioMenue
    { KEY_ESC,         'Q',                  'X'},                   // btnBottomRight: ESC and 'Q'uit, or Shutdown
    { 0,               0,                    0},                     // btnBottomLeft: PTT

//   JOY_KEY           JOY_X                 JOY_Y                     
    { KEY_UP_ARROW,    0,                    -Mouse_Move_Distance},  // joyUp
    { KEY_DOWN_ARROW,  0,                    +Mouse_Move_Distance},  // joyDown
    { KEY_LEFT_ARROW,  -Mouse_Move_Distance, 0},                     // joyLeft
    { KEY_RIGHT_ARROW, +Mouse_Move_Distance, 0},                     // joyRight

  //  Switch On        Switch Off
    { 'V',             'S',                  0}                      // btnSTF
};
char STF_key = 'V'; 


void watchdogInit() {                                         
        timeWDT = timeNow = millis();
        wdt_reset();
        WDTCSR |= (1 << WDCE) | (1 << WDE);  // set relevant bits to enter watchdog timer configuration mode
        WDTCSR = (1<< WDE) | WDTO_2S;        // turn on WatchDog timer
}
void wdtReset() {
    timeNow = millis();
    if (timeNow - timeWDT >= 450) {  // 2 in 1 second
#if VERBOSE & 1        
        Serial.println(timeNow);
#endif
        timeWDT = timeNow;
        wdt_reset();
    }
}

void setup() {
  Serial.begin(9600);
  watchdogInit();
#ifdef USE_STF_SWITCH
  button[btnSTF].onPress(onSTF_switch);
  button[btnSTF].onRelease(onSTF_switch);
#else
  button[btnSTF].onPress(onSTF_button);
#endif
  
  for (int i=joyUp;i<=joyRight;i++) {
     button[i].onRelease(onJoyRelease);
     button[i].onHoldRepeat(joy_hold_threshold, joy_rebounce_interval, onJoy);
  }

  button[btnJoy]        .onRelease(0,button_hold_threshold-1,onBtnReleasedJoy);
  button[btnJoy]        .onRelease(button_hold_threshold,onReset);
  
//  button[btnTopLeft]    .onRelease(0,button_hold_threshold-1,onMouseMode);
//  button[btnTopLeft]    .onRelease(button_hold_threshold,onButtonLong);

  for (int i=btnTopRight;i<=btnBottomRight;i++) {
    button[i].onRelease(0,button_hold_threshold-1,onButtonReleased);
    button[i].onRelease(button_hold_threshold, button_long_hold-1,onButtonLong);
    // button[i].onRelease(onButtonVeryLong);
    button[i].onHold(button_long_hold, onButtonVeryLong);
    // button[i].onRelease(button_long_hold,0xFFFF,onButtonVeryLong);
  }

//  button[btnBottomRight].onRelease(0,button_hold_threshold-1,onButtonReleased);

  button[btnTopLeft]    .onRelease(0,button_hold_threshold-1,onMouseMode);  // overwrite onButtonReleased
 
  Keyboard.begin();
  Mouse.begin();
  joy_key_counter = 0;
  for (int i=joyUp;i<=joyRight;i++)
    joy_counter[i] = 0;

}

void loop() {
  for (int i=0;i<btnSize;i++)
    button[i].update();

  wdtReset();
  if (Serial.read() == 'v')
    Serial.println("Version");
}

void onBtnPress(PushButton& btn) {
  int index = &btn - button;
  btn_timer[index] = millis();
#if VERBOSE & 2        
  Serial.print("Button Press (JOY): ");
  Serial.print(index);
  Serial.print(", ");
  Serial.println(btn_timer[index]);
#endif
}
void onBtnReleasedJoy(PushButton& btn){
    if (mouse_active) 
      Mouse.click(MOUSE_LEFT);
    else {
      if (event[0][KEY_PRESSED]) {
        Keyboard.press(event[0][KEY_PRESSED]);
        Keyboard.releaseAll();
      }
    }
}

void onMouseMode(PushButton& btn){
    mouse_active = !mouse_active;
    // Keyboard.press(event[btnTopLeft][KEY_PRESSED]);
    // Keyboard.releaseAll();
}
void onButtonReleased(PushButton& btn){
  int index = &btn - button;
#if VERBOSE & 2        
  Serial.print("Button Released: ");
  Serial.print(index);
  Serial.print(", ");
  Serial.println(timeNow);
#endif
  if (event[index][KEY_PRESSED]) {
    Keyboard.press(event[index][KEY_PRESSED]);
    Keyboard.releaseAll();
  }
}

void onReset(PushButton& btn){
  if (event[0][KEY_LONG]) {
    Keyboard.press(event[0][KEY_LONG]);
    Keyboard.releaseAll();  
  }
  exit(1);
} 

void onBtnHeldTopLeft(PushButton& btn){
  int index = &btn - button;
#if VERBOSE & 2        
  Serial.print("TopLeft Held: ");
  Serial.print(index);
  Serial.print(", ");
  Serial.println(timeNow);
#endif  
  Keyboard.press(event[index][KEY_LONG]);
  Keyboard.releaseAll();  
} 
void onButtonLong(PushButton& btn){
  int index = &btn - button;
#if VERBOSE & 2        
  Serial.print("Button Held: ");
  Serial.print(index);
  Serial.print(", ");
  Serial.println(timeNow);
#endif  
  if (event[index][KEY_LONG]) {
    Keyboard.press(event[index][KEY_LONG]);
    Keyboard.releaseAll();  
  }
} 
void onButtonVeryLong(PushButton& btn){
  int index = &btn - button;
#if VERBOSE & 2        
  Serial.print("Button Very Long: ");
  Serial.print(index);
  Serial.print(", ");
  Serial.println(timeNow);
#endif  
  if (event[index][KEY_VERYLONG]) {
    Keyboard.press(event[index][KEY_VERYLONG]);
    Keyboard.releaseAll();  
  } else if (event[index][KEY_LONG]) {
    Keyboard.press(event[index][KEY_LONG]);
    Keyboard.releaseAll();  
  }
} 

void arrowBtn(PushButton& btn, int id) {
  if (!button[btnJoy].isPressed()) {
    if (mouse_active && btn.isPressed()) {
       Mouse.move(event[id][JOY_X], event[id][JOY_Y]);
    }

    if(!mouse_active && btn.isPressed() && joy_counter[id] == 5) {
      Keyboard.press(event[id][JOY_KEY]);
      Keyboard.releaseAll(); 
    }
    joy_counter[id]++;  // = joy_counter[id] + 1; 
    if (first_pressed && joy_counter[id] > joy_key_first_pressed_threshold){
      joy_counter[id] = 0;
      first_pressed = 0;
    }
    if (!first_pressed && joy_counter[id] > joy_key_rebounce_threshold)
        joy_counter[id] = 0;  
  }
}
void onJoy(PushButton& btn) { arrowBtn(btn, &btn - button);}

void onJoyRelease(PushButton& btn){
  joy_key_counter = 0;
  for (int i=joyUp;i<=joyRight;i++)
    joy_counter[i] = 0;
  first_pressed = 1;
}

#ifdef USE_STF_SWITCH
// use a SWITCH to enable Vario or Speed -To-fly mode
void onSTF_switch(PushButton& btn){
  if (button[btnSTF].isPressed()) 
#else
// use a BUTTON to switch between Vario or Speed -To-fly mode
void onSTF_button(PushButton& btn){
  if (STF_key == event[btnSTF][KEY_LONG]) 
#endif
    STF_key = (event[btnSTF][KEY_PRESSED]);
  else
    STF_key = (event[btnSTF][KEY_LONG]); 
  Keyboard.press(STF_key);
  Keyboard.releaseAll(); 
}
