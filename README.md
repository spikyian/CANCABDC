# CANCABDC
Firmware for a cab control panel for a DC layout.

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

CANCABDC firmware can be run on CANPAN hardware with the potentiometer connected to PIC pin 7.
