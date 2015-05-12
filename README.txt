Insulin Pump Prototype Code

Created for the University of Michigan W2015 ENG 490 Senior Design Class

------------------------------------------------------------------------

Split into 2 different prototypes, this repository contains the code
required to run the prototypes.

The electrical system prototype simulates a continuous glucose monitor
(CGM), insulin pump, and phone all communicating via bluetooth. The
folder CGM_data contains the code for the simulated CGM. The folder
Insulin_pump contains the code for the simulated insulin pump. Both
require Arduinos.

The pumping mechanism prototype simulates the newly designed pumping
mechanism of an insulin pump. The folder mechanical_system_code contains
the code to run on the required Arduino. The folder rotary_files
contains the library files required for the Arduino to communicate with
the required AMS AS5048B magnetic rotary encoder.