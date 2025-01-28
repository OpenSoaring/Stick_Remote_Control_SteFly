## 2025-01-26: Version v2.1
sketch: **Stick_Remote_Control_SteFly_2.1.ino**
#### Layout see below at '(2018-01-01): Version BR'
  * 1 : switches between mouse and keyboard emulation, **a long pressure ( > 1 Sec) sends a 'E' - could be managed as PilotEvent**
      in the XCSoar/OpenSoar - or to another user defined event in the xci-file
  * 2 : call the QuickMenu ('F1'), long pressure (> 1 sec) Audio/Vario-Menu ('M')
  * 3 : is ESC, **a long pressure calls the Quit program command ('Q'), and a very long pressure ( > 4 sec) sends a 'X' - can be used as exit command or for a shutdown event**
  * 4 : is an additional button connected to the PTT (not managed by the RemoteControl itself)
  * 5 (up), 6 (down), 7 (left), 8 (right) :  are arrow keys **with an accelerated movement after 1 second of continuous pressure**
  * 0 : is Enter, a long pressure (> 1 sec) should reset the RemoteStick!
  * 9 (out of this layout): is a switch (backside of stick) between Vario and STF (Speed-to-Fly) mode

#### Benefits
  * WatchDog, manually triggered reset, diagonal mouse run and better debouncing like 2.0
  * additional button assignments with long press
    * Fn: sending **'E'** after 1 sec (f.e. to use as a PilotEvent)
    * X:  sending **'Q'** after 1 sec (f.e. to close the OpenSoar)<BR/>
          sending **'X'** after 4 sec (f.e. to shutdown the OpenVario)
  * **Mouse** has an **accelerated** mode

## 2025-01-26: Version v2.0
sketch: **Stick_Remote_Control_SteFly_2.0.ino**
#### Layout see below at '(2018-01-01): Version BR'
  * 1 : switches between mouse and keyboard emulation
  * 2 : call the QuickMenu ('F1'), long pressure (> 1 sec) Audio/Vario-Menu ('M')
  * 3 : is ESC, a long pressure (> 1 sec) calls the Quit program command ('Q')
  * 4 : is an additional button connected to the PTT (not managed by the RemoteControl itself)
  * 5 (up), 6 (down), 7 (left), 8 (right) :  are arrow keys
  * 0 : is Enter, ****a long pressure (> 1 sec) should reset the RemoteStick!***
  * 9 (out of this layout): is a switch (backside of stick) between Vario and STF (Speed-to-Fly) mode

#### Benefits
  * WatchDog is resetting the stick if the program stops (in case of outside radio disturbance)
  * Manually triggering a reset is also possible by long pressing the joystick button ('Enter') - this means that a possible communication problem between the stick and OpenVario can be remedied by re-registering the stick 

## 2025-01-26: Version v1.0
sketch: **Stick_Remote_Control_SteFly_1.0.ino**
#### Layout see below at '(2018-01-01): Version BR'
  * 1 : switches between mouse and keyboard emulation
  * 2 : call the QuickMenu ('F1'), long pressure (> 1 sec) Audio/Vario-Menu ('M')
  * 3 : is ESC, a long pressure (> 1 sec) calls the Quit program command ('Q')
  * 4 : is an additional button connected to the PTT (not managed by the RemoteControl itself)
  * 5 (up), 6 (down), 7 (left), 8 (right) :  are arrow keys
  * 0 : is Enter, ****a long pressure (> 1 sec) should reset the RemoteStick!***
  * 9 (out of this layout): is a switch (backside of stick) between Vario and STF (Speed-to-Fly) mode
Button assignment similar to Stick_Remote_Control_br
#### Differences/Benefits
  * Better debounce behavior
  * Mouse cursor has a (intuitive) diagonal run possibility

## (2018-01-01): Version BR
sketch: **Stick_Remote_Control_br.ino**
  * You can see photos of the stick and button layout here:: https://www.stefly.aero/product/stefly-leather-remote-stick
  * The SteFly layout - see below

### (SteFly-)Layout
```
           1
                2
         5
       7 0 8  
         6      3
         
          4 
```
  * 1 : switches between mouse and keyboard emulation
  * 2 : call the QuickMenu ('F1'), long pressure > 1 Sec Audio/Vario-Menu ('M')
  * 3 : is ESC
  * 5 (up), 6 (down), 7 (left), 8 (right) :  are arrow keys
  * 0 : is Enter
  * 4 : is an additional button connected to the PTT (not managed by the RemoteControl itself)
  * 9 (out of this layout): is a switch (backside of stick) between Vario and STF (Speed-to-Fly) mode
 

## 2017-06-29: new sketch from OpenVario with better debounce behavior 
  * library Bounce2 instead of Bounce, both developed from Thomas Fredericks 
  * Button layout: OpenVario (not new SteFly layout)

## 2014-01-06: 1st sketch from OpenVario
  remote control for XCSoar, emulates a keyboard and mouse
  hardware is just pushbuttons connected between pins of an Arduino Leonardo and Gnd
  for each button pressure a keystroke or mouse action is sent
  code is a wild mix of various snippets found on the net mixed up by an software illiterate 
  I started from http://forum.arduino.cc/index.php?topic=80913.msg1077713#msg1077713 and 
  modified by copy & paste trial and error. Kudos to Paul for the great starting point! 
 
  in my remote buttons are arranged in the following pattern
 
### (OV-)Layout
```
             1  2  3
         0   4  5  6
             7  8  9
```
  * 0 switches between mouse and keyboard emulation
  * 2, 4, 6, 8 are arrow keys
  * 7 sends a (keyboard) ESC, 
  * 9 sends a (keyboard) Enter
  * 1, 3, 5 call screens wanted to access directly during flight
