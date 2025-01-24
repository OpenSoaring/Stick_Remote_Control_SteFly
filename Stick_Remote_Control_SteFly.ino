// remote control for OpenSoar, XCSoar, emulates a keyboard and mouse
// hardware is just 3 pushbuttons connected between pins of an Arduino Leonardo and GND
// and a joy stick button for arrow keys or mouse action and Enter

// Button layout Stefly Remote (4 button type with joy stick) as shown on 
// https://www.stefly.aero/product/stefly-leather-remote-stick (deeper on this side)
// additional Speed to Fly switch or button between Arduino pin 7 and GND 
// at the back side of the stick
// PTT button for radio isn't connected to the Arduino

// uses libraries from
// https://github.com/thomasfredericks/Bounce2  - part of PushButton
// https://github.com/r89m/Button               - part of PushButton
// https://github.com/r89m/PushButton 

#include <Mouse.h>
#include <Keyboard.h>
#include <PushButton.h>

#include "avr/wdt.h"

#define _VERBOSE   0  // or: ((1<<2) | (1<<3) | (1<<4))  // 4 is not used up to now
#define VERBOSE(x)  (((_VERBOSE) & (1<<x)) == (1<<x))
#define MOUSE_ACCELERATOR

// disable the next define if you have a BUTTON instead a TOGGLE SWITCH: 
// #define USE_STF_TOGGLE

const char* Device  = "Stick Remote Control (SteFly)";
const char* Version = "1.0";
const char*  Date    = __DATE__;  // = Compile Date: "2025-01-24";
// check Linux delimiter and/or Window delimiter: 
const char* FileName  = (strrchr(__FILE__, '/') ?                     \
            strrchr(__FILE__, '/') + 1 : (strrchr(__FILE__, '\\') ?   \
            strrchr(__FILE__, '\\') + 1 : __FILE__));

// define timing for buttons etc.
const int joy_rebounce_interval = 3;
const int button_hold_threshold = 900;
const int button_long_hold      = 5000;  // = 5 seconds
const int joy_hold_threshold    = 80;

const int Mouse_Move_Distance = 4;     // mouse one step (-> mouse speed and accurancy)
const int mouse_speed_switch = 800;   // faster mouse after this time (in ms)
//                             KEY, MOUSE
const int joy_repeats[][2] = {{  5,   0},     // 1st reaction
                              {150, 120},     // pause after 1st press
#ifdef MOUSE_ACCELERATOR
                              { 20,   4},     // repeat interval (long press < 800ms)
                              { 20,   1}};    // accelerated repeat interval (long press > 800ms)
#else 
                              { 20,   4}};     // repeat interval (long press < 800ms)
#endif                              

// Create instances of PushButtons on digital pins
enum {btnEnter, btnFn, btnMenu, btnX, btnPTT, 
      joyUp, joyDown, joyLeft, joyRight, btnSTF, btnSize};

// define variables
boolean mouse_active = false;
// boolean first_pressed = 1;
int first_pressed = 0;
int joy_active = 0;
int key_bounce = 0;
int joy_counter[btnSize]         = {0,0,0,0,0,0,0,0,0,0};
int repeat_mode[btnSize]         = {0,0,0,0,0,0,0,0,0,0};
unsigned long btn_timer[btnSize] = {0,0,0,0,0,0,0,0,0,0};
int x_repeat = 0;

unsigned long timeNow = 0;
unsigned long timeWDT = 0;

enum {PIN, JOY_KEY, JOY_X, JOY_Y};
enum {KEY_PIN, KEY_PRESSED, KEY_LONG, KEY_VERYLONG};
const int mouse_x = Mouse_Move_Distance;
const int mouse_y = Mouse_Move_Distance;

/* -----------------------------------------------------------------
 define press (short press) and hold (long press) functions for each button
 settings in XCSoar default.xci are
 F1 QuickMenu
 F2 Analysis
 F3 Checklist
 F4 FlarmTraffic
 F5 GotoLookup (Select Waypoint)
 F6 Setup Alternates
 F7 Setup Task
 F8 Setup Basic (wingload,bugs, QNH)
 F9
    Status all
    Calculator (tasks)

 settings in openvario.xci
 V for vario mode
 S for speed to fly mode
 M for vario menu (audio)
 Q for quit
 X for shutdown
 E for event (Pilot Event or something else)
------------------------------------------------------------------*/
const int event[btnSize][4] = {
   // Pin,short pressed    >= 1 Second  very long (not used up to now) 
    { 6, KEY_RETURN,      0,           0},         // btnEnter: joystick button
    { 9, 0,               'E',         0},         // btnFn: mouse switch (and 'P')
    {14, KEY_F1,          'M',         0},         // btnMenu: QuickMenu and AudioMenue
    {15, KEY_ESC,         'Q',         'X'},       // btnX: ESC and 'Q'uit, or Shutdown
    { 0, 0,               0,           0},         // btnPTT - not connected here

//   Pin,JOY_KEY           JOY_X        JOY_Y
    { 2, KEY_UP_ARROW,    0,           -mouse_y},  // joyUp
    { 4, KEY_DOWN_ARROW,  0,           +mouse_y},  // joyDown
    { 5, KEY_LEFT_ARROW,  -mouse_x,    0},         // joyLeft
    { 3, KEY_RIGHT_ARROW, +mouse_x,    0},         // joyRight

  //  Pin,Switch On        Switch Off
    { 7, 'V',             'S',         0}          // btnSTF
};
char STF_key = 'V'; 
const uint8_t pullup = ENABLE_INTERNAL_PULLUP;
PushButton button[btnSize] = {
    {event[btnEnter][PIN], pullup}, // joystick button
    {event[btnFn]   [PIN], pullup}, // mouse switch (and 'P')
    {event[btnMenu] [PIN], pullup}, // QuickMenu and AudioMenue
    {event[btnX]    [PIN], pullup}, // ESC and 'Q'uit
    {event[btnPTT]  [PIN], pullup}, // PTT -> not connected
    {event[joyUp]   [PIN], pullup}, //
    {event[joyDown] [PIN], pullup}, //
    {event[joyLeft] [PIN], pullup}, //
    {event[joyRight][PIN], pullup}, //
    {event[btnSTF]  [PIN], pullup}  // Vario <-> Speed to Fly (STF)
};

void watchdogInit() {                                         
        timeWDT = timeNow = millis();
        wdt_reset();
        WDTCSR |= (1 << WDCE) | (1 << WDE);  // set relevant bits to enter watchdog timer configuration mode
        WDTCSR = (1<< WDE) | WDTO_2S;        // turn on WatchDog timer
}
void wdtReset() {
    timeNow = millis();
    if (timeNow - timeWDT >= 450) {  // 2 in 1 second
#if VERBOSE(1)
        Serial.println(timeNow);
#endif
        timeWDT = timeNow;
        wdt_reset();
    }
}

void setup() {
  Serial.begin(9600);
  watchdogInit();
  
  // btnEnter:
  button[btnEnter].onRelease(0,button_hold_threshold-1,onBtnReleasedJoy);
  button[btnEnter].onRelease(button_hold_threshold,onReset);
  
  // joy stick arrow button
  for (int i=joyUp;i<=joyRight;i++) {
     button[i].onHoldRepeat(joy_hold_threshold, joy_rebounce_interval, onJoyArrow);
     button[i].onRelease(onJoyRelease);
     joy_counter[i] = 0;
  }

  // btnFn and btnMenu
  for (int i=btnFn;i < btnX; i++) {
    button[i].onRelease(0,button_hold_threshold-1,(i == btnFn) ? 
                                                onMouseMode : onButtonReleased);
    button[i].onHold(button_hold_threshold, onButtonLong);
  }
  // btnX with Short, Long and VeryLong press
  button[btnX].onHoldRepeat(80, 10, onXRepeat);
  button[btnX].onRelease(onXRelease);

  // btnSTF
  button[btnSTF].onPress(onSTF_switch);
#ifdef USE_STF_TOGGLE
  button[btnSTF].onRelease(onSTF_switch);
#endif

  Keyboard.begin();
  Mouse.begin();
}

void loop() {
  for (int i=0;i<btnSize;i++)
    button[i].update();

  wdtReset();
  switch (char c = Serial.read()) {  // one char allowed only
    case 'v': Serial.println(Version);break;
    case 'd': Serial.println(Device);break;
    case 'u': Serial.println(Date);break;
    case 'f': Serial.println(FileName);break;
    default: if (isprint(c)) Serial.println(c);
          break;
  } 
}

void onBtnPress(PushButton& btn) {
  int index = &btn - button;
  btn_timer[index] = millis();
#if VERBOSE(1)
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
    // Keyboard.press(event[btnFn][KEY_PRESSED]);
    // Keyboard.releaseAll();
}
void onButtonReleased(PushButton& btn){
  int index = &btn - button;
#if VERBOSE(1)
  Serial.print("Button Released: ");
  Serial.print(index);
  Serial.print(", ");
  Serial.println(millis());
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
#if VERBOSE(1)
  Serial.print("TopLeft Held: ");
  Serial.print(index);
  Serial.print(", ");
  Serial.println(millis());
#endif  
  Keyboard.press(event[index][KEY_LONG]);
  Keyboard.releaseAll();  
}

void onButtonLong(PushButton& btn){
  int index = &btn - button;
#if VERBOSE(1)
  Serial.print("Button Held: ");
  Serial.print(index);
  Serial.print(", ");
  Serial.println(millis());
#endif  
  if (event[index][KEY_LONG]) {
    Keyboard.press(event[index][KEY_LONG]);
    Keyboard.releaseAll();  
  }
}
 
void onButtonVeryLong(PushButton& btn){
  int index = &btn - button;
#if VERBOSE(1)
  Serial.print("Button Very Long: ");
  Serial.print(index);
  Serial.print(", ");
  Serial.println(millis());
#endif  
  if (event[index][KEY_VERYLONG]) {
    Keyboard.press(event[index][KEY_VERYLONG]);
    Keyboard.releaseAll();  
  } else if (event[index][KEY_LONG]) {
    Keyboard.press(event[index][KEY_LONG]);
    Keyboard.releaseAll();  
  }
} 

void onJoyArrow(PushButton& btn) { //
  int index = &btn - button;
  unsigned current_time = millis();
  bool first = !(first_pressed & (1 << index));
  // first_pressed |= (1 << index) && (current_time - btn_timer[index] > 150);
  first_pressed |= (1 << index);
  if (!button[btnEnter].isPressed()) {
    if (first) {
      btn_timer[index] = current_time;
      repeat_mode[index] = 0;
      key_bounce = joy_repeats[0][mouse_active];  // immediately / smooth
      joy_counter[index] = 0;
#if VERBOSE(2)
      Serial.print("first: ");
      Serial.print(index);
      Serial.print(", ");
      Serial.println(btn_timer[index]);
#endif
    }
    if (joy_counter[index] >= key_bounce) {
#if VERBOSE(2)
      Serial.print("joy: ");
      Serial.print(index);
      Serial.print(", ");
      Serial.print(mouse_active ? "Mouse" : "Key");
      Serial.print(", ");
      Serial.print(repeat_mode[index]);
      Serial.print(", ");
      Serial.println(current_time - btn_timer[index]);
#endif
      if (mouse_active) {
        Mouse.move(event[index][JOY_X], event[index][JOY_Y]);
      } else { // mouse not active
        Keyboard.press(event[index][JOY_KEY]);
        Keyboard.releaseAll(); 
      }
      switch(repeat_mode[index]) {
        case 0: 
        case 1: repeat_mode[index]++;break; 
#ifdef MOUSE_ACCELERATOR
        case 2: if (current_time - btn_timer[index] > mouse_speed_switch)
                   repeat_mode[index]++;break; 
#endif                              
        default: break;
      }
      key_bounce = joy_repeats[repeat_mode[index]][mouse_active];
      joy_counter[index] = 0;
    } else { 
      joy_counter[index]++;
    }
  } else {  // Btn Enter is pressed!
#if VERBOSE(2)
    Serial.print("+ ");
    Serial.println(current_time);
#endif
  }
}

void onJoyRelease(PushButton& btn){
  int index = &btn - button;
#if VERBOSE(2)
  if (first_pressed & ( 1 << index)) {
    Serial.print("Joy Released: ");
    Serial.print(index);
    Serial.print(", ");
    Serial.println(millis() - btn_timer[index]);
  }
#endif
  joy_counter[index] = 0;
  // first = 1;
  first_pressed &= ~(1 << index);
  btn_timer[index] = millis();
}

void onXRepeat(PushButton& btn){
  
  if (!x_repeat++)
      btn_timer[btnX] = millis();
#if VERBOSE(8)
      Serial.print("btnX: ");
      Serial.print(btnX);
      Serial.print(", ");
      Serial.print(x_repeat);
      Serial.print(", ");
      Serial.println(millis() - btn_timer[btnX]);
#endif
}

void onXRelease(PushButton& btn) {
  unsigned long release_time = millis() - btn_timer[btnX];
char key = 0;
  if (x_repeat < 3 ) 
      key = 0;
  if (x_repeat < 100)  // ca. 1sec
      key = event[btnX][KEY_PRESSED];
  else if (x_repeat < 400)   // ca. 4sec
     key = event[btnX][KEY_LONG]; 
  else // x_repeat >= 200)
     key = event[btnX][KEY_VERYLONG]; 
#if VERBOSE(4)
      Serial.print("btnX - Released: ");
      Serial.print(x_repeat);
      Serial.print(", ");
      switch ((int)key) {
        case 0:  Serial.print("-");break;
        case KEY_ESC: Serial.print("ESC");break;
        case -79: Serial.print("ESC");break;
        default: 
           if(isalnum(key))
                Serial.print(key);
           else
                Serial.print((int)key);
           break;
      }
      Serial.print(", ");
      Serial.println(release_time);
#endif
  if (key) {
    Keyboard.press(key); 
    Keyboard.releaseAll(); 
  }
  x_repeat = 0;
}

void onSTF_switch(PushButton& btn){
#ifdef USE_STF_TOGGLE
  // use a SWITCH to enable Vario or Speed To Fly mode
  if (button[btnSTF].isPressed()) 
#else
  // use a BUTTON to switch between Vario or Speed To Fly mode
  if (STF_key == event[btnSTF][KEY_LONG]) 
#endif
    STF_key = (event[btnSTF][KEY_PRESSED]);
  else
    STF_key = (event[btnSTF][KEY_LONG]); 
  Keyboard.press(STF_key);
  Keyboard.releaseAll(); 
}
