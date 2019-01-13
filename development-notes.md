# Development notes

First of all please note these notes describe my process of getting the whole project working including the physical side of things
like actually building the adapter and dumpster diving for some additional parts. For more patch-notey information see readme

## Motivation and origins of the project

The project began after I had found IBM Model M with finnish layout.
![](https://raw.githubusercontent.com/hirsimaki-markus/arduino-PS2-to-USB/master/images/ibm-model-m-fin.png)
As a keyboard enthusiast the need to get it working as my
daily driver immediately hit me and soon after this I also realized the PS2-to-USB adapters I already owned would not be fit
for the purpose. The reason here being that these adapters were "dumb" as they only connected wire to wire and didn't translate
between the old and new protocols.

After briefly checking the prices and delivery times for smart adapters I soon decided that this would be excellent learning
opportunity. The journey began with the very helpfull guidance of Buutti OY's robotics club. The initial plan was to piece together
various libraries and use them on Arduino Pro Micro but this proved to be nearly impossible as all libraries meant to parse PS2
protocol produced some mystical errors I still can't decipher to this day.


## Tools used and different iterations

The library is built with Arduino IDE excluding some table bases that were generated with Python to create needed translation tables
for PS2 scancodes and USB scancodes. The very first version of this project was naive polling based test that, after manual parsing,
proved Pro Micro indeed can function fast enough to catch all needed data from the keyboard.

It soon became obvious that the only sensible way to perform the data capture from keyboard was to use interrupts instead of polling.
After I had finally managed to get the interrupt based system to work, similarities between my solution's and other libraries' source
code were striking even thought everything was cleanroom designed to suit my needs.

At this point I also had constructed various versions of the actual cables used and would later settle to use the setup shown here.
![](https://raw.githubusercontent.com/hirsimaki-markus/arduino-PS2-to-USB/master/images/ps2-to-pin.png)
Even thought the PS2 cable actually has more than 4 wires only 4 are used for the protocol; v+, ground, data and clock.

Interestingly enough, there were multiple bugs and one of them resulted in me being unable to check which one of the three most
commonly used scancode sets the keyboard used. This was solved by hooking the keyboard onto an oscilloscope and carefully decoding
signal manually which showned that the keyboard used [scancode set 2](https://wiki.osdev.org/PS/2_Keyboard#Scan_Code_Set_2) as expected.
Later on another tool used was USB protocol sniffer which revealed incorrect documentation on Arduino USB keyboard library. This was
one of the hardest problems during the whole project.


## On dificulties

The project as a whole has had various dificulties, most of them being related to software. On the hardware side the only notable
dificulty was the physical assembly of wires and leads as I had limited amount of materials at hand. The actual PS2 adapter head was
made from previously mentioned "dumb" adapter that had rather shobby wire quality which resulted in soldered wires snapping in half
multiple times.

Software side the single most frustrating problem was Arduino Pro Micro in itself as it is notoriously dificult to program. Pro Micro
lacks any sort of physical reset button so only way to even attempt to reset it is to short RST and GND pins. This reset doesn't always
work and after getting the board programmed with wrong settings and faulty scetch at one point only solution was to reinstall
all Windows drivers for Arduino along with all related software and immediately reset the board twice after connecting it to pc and then
upload empty scetch. Debugging in general was also tedious as there are no easy to use debuggers all debugging had to be done with
"print to console" method.

The most complex single item that had to be created was the actual parsing of PS2 bytes into keystrokes after raw data had been parsed
into bytes. This is the result of scancode set 2 presenting some invidual keystrokes as 8 separate bytes. After the multiple byte
scancodes have been translated into sets consisting of single bytes they are used as keys to lookup table which will return
corresponding values for USB protocol.

The next significant problem was actually building the lookup table. Getting list of PS2 scancodes was exercise in clickng keys, but
building the list of USB scancodes was all but trivial. In the end it included a small code snippet that scanned throught all possible
values that could be sent to the receiving PC over keyboard library. Arduino documentation does not list all important scancodes,
for example one for printscreen button is completely missing even thought the value for it does exist. Some of these values
vary by keyboard layout chosen by OS an this allows some keyboards to include umlauts instead of some other keys.


## On things learned

This project has allowed me to learn multitude of things. Maybe the most important of these is how interrupt based systems work in
practice with keyboard layout maps coming close second. The project has also been valuable practice in writing (Arduino variant) of C++.
On the physical side of developing embedded systems collecting various parts for future development has also been interesting task.
In the image rotatory encoder and laptop pointing stick can be seen that ave been salvaged from old laptop and keyboard
![](https://raw.githubusercontent.com/hirsimaki-markus/arduino-PS2-to-USB/master/images/additional-parts.png)



mitä opin? protokollan toimintaa ja scancode siujen merkitystä yms ainakin

## Future of the project

Current state of the project
![](https://raw.githubusercontent.com/hirsimaki-markus/arduino-PS2-to-USB/master/images/adapter-assembled.png)
