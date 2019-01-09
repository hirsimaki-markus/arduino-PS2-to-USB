# arduino-PS2-to-USB
This project allows you to use Arduino as USB adapter for PS2 keyboards like IBM model M

To use Arduino as adapter you should first connect PS/2 keyboard cables wires to arduino

    V+    to  VCC
    GND   to  GND
    Data  to  pin2
    Clock to  pin3
    
Then connect arduinos USB to PC with any suitable cable and upload the code as a normal scetch to arduino

* First release confirmed to be working with Windows7 and Windows10. Most likely works on most operating systems with some degree of accuracy with keymaps

* Requires arduino with USB host cababilities. Confirmed working on Arduino Pro Micro

* Currently only supports Finnish keymap with 104 keys or less. If you want to help adding more keymaps feel free to email me 

* Pausebreak key toggles numpad numbers to work as mouse (0 and 5 for mousebuttons)
