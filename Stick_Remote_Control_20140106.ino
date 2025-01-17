// remote control for XCSoar, emulates a keyboard and mouse
// hardware is just pushbuttons connected between pins of an Arduino Leonardo and Gnd
// for each button press a keystroke or mouse action is sent
// code is a wild mix of various snippets found on the net mixed up by an software illiterate 
// I started from http://forum.arduino.cc/index.php?topic=80913.msg1077713#msg1077713 and 
// modified by copy & paste trial and error. Kudos to Paul for the great starting point! 
//
// in my remote buttons are arranged in the following pattern
//
//    1  2  3
//0   4  5  6
//    7  8  9
//
// 0 switches between mouse and keyboard emulation
// 2, 4, 6, 8 are arrow keys
// 7 is ESC, 9 Enter
// 1, 3, 5 call screens I want to access directly during flight
//


#include <Bounce.h>

byte buttons[]={10,0,4,9,2,12,8,3,1,6};//seperate array from definitions to set up the pins
#define NUMBUTTONS sizeof(buttons)//gives size of array *helps for adding buttons

int debounce_delay = 20;       //Debounce delay in milliseconds
int rebounce_interval = 5;
int button_pressed = 99;
boolean Mouse_Active = 0;
const int Mouse_Move_Distance = 1;

// I really dont see getting around doing this manually
Bounce bouncer[] = {//would guess thats what the fuss is about
  Bounce(10,debounce_delay),
  Bounce(0,debounce_delay),
  Bounce(4,debounce_delay),
  Bounce(9,debounce_delay),
  Bounce(2,debounce_delay),
  Bounce(12,debounce_delay),
  Bounce(8,debounce_delay),
  Bounce(3,debounce_delay),
  Bounce(1,debounce_delay),
  Bounce(6,debounce_delay)
  };

void setup() {
   for (byte set=0;set<=NUMBUTTONS;set++){//sets the button pins
     pinMode(buttons[set],INPUT);
     digitalWrite(buttons[set],HIGH);//<-comment out this line if not using internal pull ups
   }//-----------------------------------and change read()==to high if your set up requires
 // pinMode(LED,OUTPUT);//------------------otherwise event will occure on release

// Wait five seconds since the HID drivers need a bit of time to re-mount after upload.
  delay(5000);
  Keyboard.begin();
  Mouse.begin();
//  Serial.begin(57600);
//  Serial.println("Pushbutton Bounce library test:");
}

void loop() {
  
 while (button_pressed == 99) {
  for(int num=0;num<NUMBUTTONS;num++){
   if ( bouncer[num].update()) {
    if ( bouncer[num].fallingEdge()) {
       button_pressed = num;
//       Serial.println("Button Pressed");
//       Serial.println(button_pressed);
       break;
     }
   }
  } 
 }
 
  if (Mouse_Active) {
   if (button_pressed == 0) Mouse_Active = 0;
   if (button_pressed == 1) Keyboard.press(KEY_F9);
   if (button_pressed == 2){
    while (bouncer[2].read() == 0) {
     Mouse.move(0, -Mouse_Move_Distance);
     bouncer[2].update();
     delay(rebounce_interval);}}
     //Serial.println("Bouncer Read");
     //Serial.println(bouncer[2].read());  
   if (button_pressed == 3) Keyboard.press(KEY_F6);
   if (button_pressed == 4) {
    while (bouncer[4].read() == 0) {
     Mouse.move(-Mouse_Move_Distance, 0);
     bouncer[4].update();
     delay(rebounce_interval);}} 
   if (button_pressed == 5) Keyboard.press(KEY_F2);
   if (button_pressed == 6) {
    while (bouncer[6].read() == 0) {
     Mouse.move(Mouse_Move_Distance, 0);
     bouncer[6].update();
     delay(rebounce_interval);}}     
   if (button_pressed == 7) {
     Keyboard.press(KEY_ESC);
     Keyboard.releaseAll();
     Keyboard.press(KEY_ESC);}
   if (button_pressed == 8) {
    while (bouncer[8].read() == 0) {
     Mouse.move(0, Mouse_Move_Distance);
     bouncer[8].update();
     delay(rebounce_interval);}}        
   if (button_pressed == 9) Mouse.click(MOUSE_LEFT);
  }
  else {
   if (button_pressed == 0) Mouse_Active = 1;
   if (button_pressed == 1) Keyboard.press(KEY_F9);
   if (button_pressed == 2) Keyboard.press(KEY_UP_ARROW);
   if (button_pressed == 3) Keyboard.press(KEY_F6);
   if (button_pressed == 4) Keyboard.press(KEY_LEFT_ARROW);
   if (button_pressed == 5) Keyboard.press(KEY_F2);
   if (button_pressed == 6) Keyboard.press(KEY_RIGHT_ARROW);
   if (button_pressed == 7) {
     Keyboard.press(KEY_ESC);
     Keyboard.releaseAll();
     Keyboard.press(KEY_ESC);}
   if (button_pressed == 8) Keyboard.press(KEY_DOWN_ARROW);
   if (button_pressed == 9) Keyboard.press(KEY_RETURN);
  }

Keyboard.releaseAll();
button_pressed = 99; 
}
