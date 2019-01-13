# Development notes

First I would like to mention these notes describe my process of getting the whole project working including the physical side of things
like actually building the adapter and dumpster diving for some additional parts. For more patch note -like information see README.

## Motivation and origins of the project

The project began after I had found IBM Model M with Finnish layout.
![](https://raw.githubusercontent.com/hirsimaki-markus/arduino-PS2-to-USB/master/images/ibm-model-m-fin.png)
As a keyboard enthusiast the need to get it working as my daily driver immediately hit me. Soon after this I also realized the
PS2-to-USB adapters I already owned would not be fit for the purpose. The reason here being that these adapters were "dumb" as they only
connected wire to wire and didn't translate between the old and new protocols.

After briefly checking the prices and delivery times for smart adapters I soon decided that this would be excellent learning
opportunity. The journey began with the very helpful guidance of Buutti OY's robotics club. The initial plan was to piece together
various libraries and use them on Arduino Pro Micro as Pro Micro could function natively as USB-device. This however, proved to be
nearly impossible as all libraries meant to parse PS2 protocol produced some mystical errors when ran on Pro Micro.


## Tools used and different iterations

The library is built with Arduino IDE excluding some table bases that were generated with Python to create needed lookup tables
for PS2 scancodes and USB scancodes. The very first version of this project was naive polling based test that, after manual parsing,
proved Pro Micro indeed can function fast enough to catch all needed data from the keyboard.

It soon became obvious that the only sensible way to perform the data capture from keyboard was to use interrupts instead of polling.
After I had finally managed to get the interrupt based system to work, similarities between my solution's and other libraries' source
code were striking even thought everything was cleanroom designed to suit my needs.

At this point I also had constructed various versions of the actual cables used and would later settle to use the setup shown here.
![](https://raw.githubusercontent.com/hirsimaki-markus/arduino-PS2-to-USB/master/images/ps2-to-pin.png)
Even thought the PS2 cable actually has more than 4 wires only 4 are used for the protocol; v+, ground, data and clock.

Interestingly enough, there were multiple bugs and one of them resulted in me being unable to check which one of the three most
commonly used scancode sets the keyboard used. This was solved by hooking the keyboard on to an oscilloscope and carefully decoding
signal manually which showed that the keyboard used [scancode set 2](https://wiki.osdev.org/PS/2_Keyboard#Scan_Code_Set_2) as expected.
Later on another tool used was USB protocol sniffer which revealed incorrect documentation on Arduino USB keyboard library. This was
one of the hardest problems during the whole project.


## On difficulties

The project as a whole has had various difficulties, most of them being related to software. On the hardware side the only notable
difficulty was the physical assembly of wires and leads as I had limited amount of materials at hand. The actual PS2 adapter head was
made from previously mentioned "dumb" adapter that had rather shabby wire quality which resulted in soldered wires snapping in half
multiple times.

Software side the single most frustrating problem was Arduino Pro Micro in itself as it is notoriously difficult to program. Pro Micro
lacks any sort of physical reset button so only way to even attempt to reset it is to short RST and GND pins. This reset doesn't always
work and after getting the board programmed with wrong settings and faulty sketch at one point, the only solution was to reinstall
all Windows drivers for Arduino along with all related software. Then after this immediately reset the board twice after connecting it
to PC and upload empty sketch. Debugging in general was also tedious as there are no easy to use debuggers all debugging had to be done
with "print to console" method.

The most complex single item that had to be created was the actual parsing of PS2 bytes into keystrokes after raw data had been parsed
into bytes. This is the result of scancode set 2 presenting some invidual keystrokes as 8 separate bytes. After the multiple byte
scancodes have been translated into sets consisting of single bytes they are used as keys to lookup table which will return
corresponding values for USB protocol.

The next significant problem was actually building the lookup table. Getting list of PS2 scancodes was exercise in clicking keys, but
building the list of USB scancodes was all but trivial. In the end it included a small code snippet that scanned through all possible
values that could be sent to the receiving PC over keyboard library. Arduino documentation does not list all important scancodes,
for example one for printscreen button is completely missing even thought the value for it does exist. Some of these values
vary by keyboard layout chosen by OS and this allows some keyboards to include umlauts instead of some other keys.


## On things learned

This project has allowed me to learn multitude of things. Maybe the most important of these is how interrupt based systems work in
practice with keyboard layout maps coming close second. The project has also been valuable practice in writing Arduino variant of C++.
On the physical side of developing embedded systems collecting various parts for future development has also been interesting task.
In the image rotatory encoder and laptop pointing stick can be seen that have been salvaged from keyboard and old laptop.
![](https://raw.githubusercontent.com/hirsimaki-markus/arduino-PS2-to-USB/master/images/additional-parts.png)


## Future of the project

In it's current state the project is more like a working prototype than a finished product. For future development the next things to be
added will be the pointing stick and rotatory encoder to better support mouse simulation. This is in preparation of adding Bluetooth
cabability so the keyboard can be used as an actual keyboard and mouse combo. Adding the Bluetooth will require the adding of battery
which in turn will be able to support RGB backlighting.

I'm expecting for the addition of pointing stick to be the hardest upcoming task as it requires large amounts of tinkering on hardware
level and robust software implementation to make the mouse actually move in ways that are pleasant for the user. If the pointing stick
proves out to be too hard to interface with, a trackball based system will be valid replacement. Later on adding Bluetooth should not
be too hard to implement as routing USB data over Bluetooth is well known technology.

_- Markus Hirsimäki_
