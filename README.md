 Open ChargePort
 ===============

 What is it?
 As the name suggests, it's a transmitter that will open the charge port of a Model S.

 Last week I received my EU spec model S, and charged a few times with the cable Tesla supplies with the car.
 The button to open the Charge port door, really is a nice feature.
 As there is no such thing on a standard Type 2 charge cable, i now have to open the car door, touch the battery icon, and select open charge port. Not very convenient.

 Therefore i wanted to make my own transmitter that opens the chargeport, and that can be fitted into a standard Type2 plug.

 Credits to Banahogg who documented his attempts to analyze the RF data here
 which helped me finish this project in a few days.


 The components needed:

 I used a 433 Mhz module connected to a PIC microcontroller. (US model S, might require a 315Mhz module)
 Only a few other components are required.

 1x PIC12F1822 or PIC10F200 microcontroller
 1x TX-SAW MID/3V
 1x Lithium cell, 3V
 2x 100nf capacitor
 1x 10uf capacitor
 1x pushbutton
 
 