# Stick Remote Control (SteFly)

Arduino sketch for Stefly stick remote. 
You can see photos of the stick and button layout here:: **[SteFly Remote Stick](https://www.stefly.aero/product/stefly-leather-remote-stick)** <BR/>
History of detailed developing description you can find **[here](History.md)!**
### (SteFly-)Layout Overview
```
           1
                2
         5
       7 0 8  
         6      3
         
          4 
```
  * 1 : switches between mouse and keyboard emulation, **a long pressure ( > 1 Sec) sends a 'E' - could be managed as PilotEvent**
      in the XCSoar/OpenSoar - or to another user defined event in the xci-file
  * 2 : call the QuickMenu ('F1'), long pressure (> 1 sec) Audio/Vario-Menu ('M')
  * 3 : is ESC, **a long pressure calls the Quit program command ('Q'), and a very long pressure ( > 4 sec) sends a 'X' - can be used as exit command or for a shutdown event**
  * 4 : is an additional button connected to the PTT (not managed by the RemoteControl itself)
  * 5 (up), 6 (down), 7 (left), 8 (right) :  are arrow keys **with an accelerated movement after 1 second of continuous pressure**
  * 0 : is Enter, **a long pressure (> 1 sec) should reset the RemoteStick!**
  * 9 (out of this layout): is a switch (backside of stick) between Vario and STF (Speed-to-Fly) mode
