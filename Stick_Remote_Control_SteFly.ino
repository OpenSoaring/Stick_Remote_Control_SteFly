// remote control for XCSoar, emulates a keyboard and mouse
// hardware is just pushbuttons connected between pins of an Arduino Leonardo and Gnd
// for each button press a keystroke or mouse action is sent
// Button layout Stefly Remote (5 button type) as shown on http://www.openvario.org/doku.php?id=projects:remote_00:top 
// additional Speed to Fly switch between Arduino pin 4 and GND

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

// define on which pins the buttons are connected
const int Button_1_pin = 9;    // top left button
const int Button_2_pin = 14;   // top right button
const int Button_3_pin = 15;   // low right button
//        Button_4_pin = N/A   // low left (PTT) button
const int Joy_button_pin = 6;  // joystick button
const int Joy_up_pin = 2;
const int Joy_down_pin = 4;
const int Joy_left_pin = 5;
const int Joy_right_pin = 3;
const int STF_button_pin = 7;  // STF switch 

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
// M for vario menu
// Q to quit


const char Button_1_press_key = 0;                // this is the mouse switch button
const char Button_1_hold_key = 0;                 // not used on SteFly now (2024-12-31)
const char Button_2_press_key = KEY_F1;           // F1 for QuickMenu
const char Button_2_hold_key = 'M';               // M for vario menu
const char Button_3_press_key = KEY_ESC;          // ESC
const char Button_3_hold_key = 'Q';               // Q to quit XCSoar
//         Button_4 = N/A                         // PTT switch
const char Joy_button_press_key = KEY_RETURN;     // Enter
const char Joy_button_hold_key = 0;               // unused
const char STF_Vario = 'V';                       // V for vario mode when switch is on
const char STF_SpeedToFly = 'S';                  // S for STF mode when switch is off

// define timing for buttons etc.
const int Mouse_Move_Distance = 1;
const int joy_rebounce_interval = 3;
const int joy_key_rebounce_threshold = 20;
const int joy_key_first_pressed_threshold = 100;
const int button_hold_threshold = 500;
const int joy_hold_threshold = 1;
//const int debounce_delay = 10;                   //Debounce delay in milliseconds, not used

// define variables
boolean mouse_active = 0;
boolean first_pressed = 1;
int joy_key_counter = 0;

char STF_key = STF_Vario; 

// Create instances of PushButtons on digital pins
PushButton STF_button = PushButton(STF_button_pin, ENABLE_INTERNAL_PULLUP);
PushButton Joy_up = PushButton(Joy_up_pin, ENABLE_INTERNAL_PULLUP);
PushButton Joy_down = PushButton(Joy_down_pin, ENABLE_INTERNAL_PULLUP);
PushButton Joy_left = PushButton(Joy_left_pin, ENABLE_INTERNAL_PULLUP);
PushButton Joy_right = PushButton(Joy_right_pin, ENABLE_INTERNAL_PULLUP);
PushButton Button_1 = PushButton(Button_1_pin, ENABLE_INTERNAL_PULLUP);
PushButton Button_2 = PushButton(Button_2_pin, ENABLE_INTERNAL_PULLUP);
PushButton Button_3 = PushButton(Button_3_pin, ENABLE_INTERNAL_PULLUP);
// PushButton Button_4 = PushButton(Button_4_pin, ENABLE_INTERNAL_PULLUP);
PushButton Joy_button = PushButton(Joy_button_pin, ENABLE_INTERNAL_PULLUP);

void setup() {
  STF_button.onPress(onSTF_button);
  
  Joy_up.onRelease(onJoyRelease);
  Joy_down.onRelease(onJoyRelease);
  Joy_left.onRelease(onJoyRelease);
  Joy_right.onRelease(onJoyRelease);

  Joy_up.onHoldRepeat(joy_hold_threshold, joy_rebounce_interval, onJoy);
  Joy_down.onHoldRepeat(joy_hold_threshold, joy_rebounce_interval, onJoy);
  Joy_left.onHoldRepeat(joy_hold_threshold, joy_rebounce_interval, onJoy);
  Joy_right.onHoldRepeat(joy_hold_threshold, joy_rebounce_interval, onJoy);
  
  Button_1.onRelease(0,button_hold_threshold-1,onButtonReleased);
  Button_2.onRelease(0,button_hold_threshold-1,onButtonReleased);
  Button_3.onRelease(0,button_hold_threshold-1,onButtonReleased);
  Joy_button.onRelease(0,button_hold_threshold-1,onButtonReleased);
  
  Button_1.onHold(button_hold_threshold,onButtonHeld);
  Button_2.onHold(button_hold_threshold,onButtonHeld);
  Button_3.onHold(button_hold_threshold,onButtonHeld);
 
  Keyboard.begin();
  Mouse.begin();
  joy_key_counter = 0;
}

void loop() {
  STF_button.update();
  Button_1.update();
  Button_2.update();
  Button_3.update();
  Joy_button.update();
  Joy_up.update();
  Joy_down.update();
  Joy_left.update();
  Joy_right.update();
}

void onButtonReleased(Button& btn){
  if(btn.is(Button_1)) 
      mouse_active = !mouse_active;
  if(btn.is(Button_2)) Keyboard.press(Button_2_press_key);
  if(btn.is(Button_3)) Keyboard.press(Button_3_press_key);
  if(btn.is(Joy_button)) 
    if(mouse_active) Mouse.click(MOUSE_LEFT);
    else Keyboard.press(Joy_button_press_key);
  Keyboard.releaseAll();
}

void onButtonHeld(Button& btn){
  if(btn.is(Button_1) && Button_1_hold_key)
      Keyboard.press(Button_1_hold_key);
  if(btn.is(Button_2) && Button_2_hold_key)
      Keyboard.press(Button_2_hold_key);
  if(btn.is(Button_3) && Button_3_hold_key)
      Keyboard.press(Button_3_hold_key);
  if(btn.is(Joy_button) && Joy_button_hold_key)
      Keyboard.press(Joy_button_hold_key);
  Keyboard.releaseAll();  
} 


void onJoy(Button& btn){
  if(mouse_active && btn.isPressed()) {
    if(btn.is(Joy_up)) Mouse.move(0, -Mouse_Move_Distance);
    if(btn.is(Joy_down)) Mouse.move(0, Mouse_Move_Distance);
    if(btn.is(Joy_left)) Mouse.move(-Mouse_Move_Distance, 0);
    if(btn.is(Joy_right)) Mouse.move(Mouse_Move_Distance, 0);
  }
  if(!mouse_active && btn.isPressed() && joy_key_counter == 5) {
    if(btn.is(Joy_up)) Keyboard.press(KEY_UP_ARROW);
    if(btn.is(Joy_down)) Keyboard.press(KEY_DOWN_ARROW);
    if(btn.is(Joy_left)) Keyboard.press(KEY_LEFT_ARROW);
    if(btn.is(Joy_right)) Keyboard.press(KEY_RIGHT_ARROW);
    Keyboard.releaseAll(); 
  } 
  joy_key_counter = joy_key_counter +1; 
  if (first_pressed && joy_key_counter > joy_key_first_pressed_threshold){
    joy_key_counter = 0;
    first_pressed = 0;
  }
  if (!first_pressed && joy_key_counter > joy_key_rebounce_threshold)joy_key_counter = 0;  
}

void onJoyRelease(Button& btn){
  joy_key_counter = 0;
  first_pressed = 1;
}

void onSTF_button(Button& btn){
  if (STF_key == STF_Vario) 
    STF_key = (STF_SpeedToFly);
  else
    STF_key = (STF_Vario); 
  Keyboard.press(STF_key);
  Keyboard.releaseAll(); 
}
