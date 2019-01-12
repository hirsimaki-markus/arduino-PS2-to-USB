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


## On dificulties and things learned



## jatkokehitys


(kuvia sinne tönne)

dyykkaus

kirjastojen toimimattomuus(samankaltainen lopputulos). vaihtoehtoinen toteutus

adapterin teko ja uudelleenteko

naiivi systeemi ilman keskeytyksiä, raakadataa käsin

keskeytysohjaus

skoopilla scancode sivu

ps2 ja usb koodien mäppäys

usb haistelua kirjaston toiminnasta. vajaa/virheellinen dokumentaatio

jatkokehitys ja lisäosien dyykkays
