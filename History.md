
2014-01-06: 1st sketch from OpenVario
  remote control for XCSoar, emulates a keyboard and mouse
  hardware is just pushbuttons connected between pins of an Arduino Leonardo and Gnd
  for each button press a keystroke or mouse action is sent
  code is a wild mix of various snippets found on the net mixed up by an software illiterate 
  I started from http://forum.arduino.cc/index.php?topic=80913.msg1077713#msg1077713 and 
  modified by copy & paste trial and error. Kudos to Paul for the great starting point! 
 
  in my remote buttons are arranged in the following pattern
 
 (OV-)Layout
 ======
     1  2  3
 0   4  5  6
     7  8  9
 
  0 switches between mouse and keyboard emulation
  2, 4, 6, 8 are arrow keys
  7 is ESC, 9 Enter
  1, 3, 5 call screens I want to access directly during flight
 
(SteFly-)Layout
 ======
    (1)
   5      (2)
 7 0 8  
   6      (3)
    (4)
 
  1 : switches between mouse and keyboard emulation, a long press sends > 1 Sec a 'E' - could be managed as PilotEvent 
      in the XCSoar/OpenSoar - or to another user defined Event in the xci-file
  2 : call the QuickMenu ('F1'), long press > 1 Sec Audio/Vario-Menu ('M')
  3 : is ESC, a long press calls the Quit program command ('Q')
  5 (up), 6 (down), 7 (left), 8 (right) :  are arrow keys
  0 : is Enter, (a long press > 1 Sec should reset the RemoteStick!)
  4 : is an additional button connected to the PTT (not managed by the RemoteControl itself)
  9 (out of this layout): is a switch (backside of stick) between Vario and STF (Speed-to-Fly) mode
 

