# CANCABDC
Firmware for a cab control panel for a DC layout.

##Introduction

A panel supporting one controller and up to 16 DC sections. 
Multiple CANCABDCs may be used to control a layout. 
CANCABDCs communicate between themselves to negotiate which panel 
has control of any section. Different CANCABDC panels can control 
different sections so that any number of sections can be used on a layout.

A potentiometer (e.g. 10K linear) is used for speed control. Each section has 2 LEDs to indicate:

* If any panel has control of the section
* If this panel has control of the section

Each section normally has 2 push buttons:

* Request control
* Release control

Optionally a single push button can be used in toggle mode.

The CANCABDC panels communicate between themselves using CBUS. CBUS is also used to 
communicate speed information to CAN4DC modules. The CAN4DC modules convert the 
speed information to a PWM signal for the DC motors.

## CANPAN Hardware

CANCABDC firmware can be run on CANPAN hardware. It is recommended that the IDC connector with
a ribbon cable is used to connect to the LED and switch matrices as the J4 connector provides
the needed +5V connection. 
A potentiometer is connected to the PIC's analogue input on pin 7. 
The hardware modification to allow a connection to be made to pin 7 involves drilling a hole 
through the PCB between the 25 way D type connector and the PIC. The copper flood fill on the
underside of the PCB should be cleared from around the hole. A 1-way connector can be glued
into place and a wire soldered from the underside of the connector to pin 7 of the PIC.
A flexible wire can be attached to the 1-way connector alongside the ribbon cable to the 
centre terminal of the potentiometer. The outer two terminals of the potentiometer are connected
to +5V and 0V wires of the ribbon cable.


## CANPAN LED and Switch assignments


|      |section   | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |   | 9 | 10| 11| 12| 13| 14| 15| 16|
|------|----------|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
|Type  |Row       |   |   |   |   |   |   |   |   |Row|   |   |   |   |   |   |   |   |
|      |Pin       |   |   |   |   |   |   |   |   |Pin|   |   |   |   |   |   |   |   |
|------|----------|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
|switch|17        | 1 | 5 | 9 | 13| 17| 21| 25| 29| 15| 3 | 7 | 11| 15| 19| 23| 27| 31|
|switch|16        | 2 | 6 | 10| 14| 18| 22| 26| 30| 14| 4 | 8 | 12| 16| 20| 24| 28| 32|
|      |column pin| 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |   | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
|------|----------|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
|led   |18        | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 20| 17| 18| 19| 20| 21| 22| 23| 24|
|led   |19        | 9 | 10| 11| 12| 13| 14| 15| 16| 21| 25| 26| 27| 28| 29| 30| 31| 32|
|      |column pin| 9 | 10| 11| 12| 25| 24| 23| 22|   | 9 | 10| 11| 12| 25| 24| 23| 22|

##Test mode

If the FLiM push button is held down during power up the module enters a test mode to allow
wiring to be checked.

By default test#1 is run which repeats the following sequence:
1. light up all the LEDs (8) on each row (4) in turn for 1 second each
2. light up all the LEDs (4) on each column (8) in turn for 1 second each
3. light up the LED for a switch that is pressed for 10 seconds

test#2 can be selected by also holing down switch SW2 during power up. Test#2 repeats the
following sequence:
1. Light each LED in turn for 1 second

test#3 can be selected by also holing down switch SW3 during power up. Test#3 repeats the
following sequence:
1. Read the potentiometer value and light the LED corresponding to the potentiometer setting.

