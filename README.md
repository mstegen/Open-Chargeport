Open ChargePort
===============

What is it?
As the name suggests, it's a transmitter that will open the charge port of a Model S.

When I received my EU spec model S, i charged a few times with the cable Tesla supplies with the car.
The button to open the Charge port door, really is a nice feature.
As there is no such thing on a standard Type 2 charge cable, i now have to open the car door, touch the battery icon, and select open charge port. Not very convenient.

Therefore i wanted to make my own transmitter that opens the chargeport.

Credits to Banahogg who documented his attempts to analyze the RF data here
which helped me finish this project in a few days.

The components needed:

I used a 433 Mhz module connected to a PIC microcontroller. (US model S, requires a 315Mhz module)
Only a few other components are required.

1x PIC12F1822 or PIC10F200 microcontroller 
1x TX-SAW MID/3V
1x Lithium cell, 3V
2x 100nf capacitor
1x 10uf capacitor
1x pushbutton
 
Please note that the PCB layout in /eagle was designed for the PIC10F200 microcontroller.

=======================================================================================

There is now a new version, based on the same chip Tesla uses in it's Charging Cable.

The chip is a Silabs Si4010, only one external component (capacitor) is required.
I converted the Tesla specific bitstream for use with this microcontroller. 
As Silabs also provided a PCB layout file for 5 buttons, but only one button is needed, these files were modified.
The PCB layout fit's the Polycase FB-20 keyfob housing.

All files for this version can be found in the /Si4010 folder.

Fully assembled Open Chargeport remote's are available here:
http://www.stegen.com/ocp

for USA, you can contact lolachampcar on teslamotorsclub.com
 



