/**
 * @brief Hardware manual
 * 
 * @file doc.c
 * @author Rozum
 */

/**
 * @page servo_box 
 * @section sect_descr 1. Product overview
 * 
 * <b>A servobox</b> is a solution designed to control motion of one or more RDrive servos. The solution comprises the following components:
 * - one or more energy eaters (see Section 3.1)
 * - one or more capacitor modules (see Section 3.2)
 * - a CAN-USB dongle to provide CANOpen communication between the servobox and the servos
 * 
 * Additionally, to ensure operation of the servobox, the user has to provide a power supply and USB-A to Micro USB cable to connect the CAN-USB dongle to PC.
 * 
 * <b>Note:</b>The USB-A to Micro USB cable should not be longer than
 * 
 * The power supply should meet the following requirements:
 * - its supply voltage should be 48 V max
 * - its power should be equal to the total nominal power of all servo motors connected to it multiplied by a factor of 2.5 to 3.
 * 
 * @section sect_conn 2. Integrating servos with a power supply and a servobox
 * 
 * To integrate a RDrive servo into one circuit with a power supply and a servobox, you need to provide the following connections:
 * 
 * - power supply connection (two wires on the servo housing)
 * - CAN communication connection (two wires on the servo housing)
 * 
 * For connection diagrams and requirements, see Sections 2.1 and 2.2.
 * For eater and capacitor requirements and schematic, see Section 3.1 and 3.2.
 * 
 * @subsection sect_21 2.1. Power supply connection
 * 
 * <b>Note:</b> Never supply power before a servo (servos) is (are) fully integrated with a servobox and a power supply into one circuit.
 * Charging current of the capacitor(s) can damage the power supply or injure the user!
 * 
 * The configuration of the servo box solution (e.g., how many eaters and capacitors it uses) and the electrical connection diagram depend on whether your intention is:
 * - to connect a single servo, in which case the configuration and the connection diagram are as below:
 * @image html "single_servo_conn_1.png" width=800
 * @image latex "single_servo_conn_1.png" "Connecting single RDrive servo to power supply" width=300pt
 * - to connect multiple servos, in which case the configuration and the connection diagram are as below:
 * @image html "multiple_servo_conn_1.png" width=800
 * @image latex "multiple_servo_conn_1.png" "Connecting multiple RDrive servos to power supply" width=300pt
 * 
 * In any case, make sure to meet the following electrical connection requirements:
 * - Typically, the total circuit length from the power supply to any servo motor must not exceed 10 meters.
 * - Length "L1" must not be longer than 10 meters.
 *     - When the total connected motor power is <b>less than 250 W</b>,  the cable cross-section within the segment must be at least 1.00 mm2.
 *     - When the total connected motor power is <b>less than 500 W</b>,  the cable cross-section within the segment must be at least 2.00 mm2.
 * - Length "L2" (from the eater to the capacitor) should not exceed the values from Table 1.
 * - Length "L3" (from the capacitor to any servo) should not exceed the values from Table 1.
 * 
 * <b>Table 1: Line segment lengths vs. cross-sections</b> 
 * |Servo model|L2||||||L3||||||
 * |-----------|-|-|-|-|-|-|-|-|-|-|-|-|
 * |           |0.75 mm<SUP>2</SUP>|1.0 mm<SUP>2</SUP>|1.5 mm<SUP>2</SUP>|2.5 mm<SUP>2</SUP>|4.0 mm<SUP>2</SUP>|6.0 mm<SUP>2</SUP>|0.75 mm<SUP>2</SUP>|1.0 mm<SUP>2</SUP>|1.5 mm<SUP>2</SUP>|2.5 mm<SUP>2</SUP>|4.0 mm<SUP>2</SUP>|6.0 mm<SUP>2</SUP>|
 * |RD50	   |4 m	    |5 m	|8 m	|10 m	|10 m	|10 m	|0,2 m	 |0,2 m	 |0,4 m	 |0,7 m	 |1,0 m	 |1,0 m  |
 * |RD60	   |2 m	    |3 m	|5 m	|9 m	|10 m	|10 m	|0,1 m	 |0,1 m	 |0,2 m	 |0,4 m	 |1,0 m	 |1,0 m  |
 * |RD85	   |0,8 m	|1 m	|1 m	|2 m	|4 m	|6 m	|0,04 m	 |0,05 m |0,08 m |0,13 m |0,21 m |0,32 m |
 * 
 * @subsection sect_22 2.2. CAN connection
 * The CAN connection of RDrive servos is a two-wire bus line transmitting differential signals: CAN_HIGH and CAN_LOW. 
 * The configuration of the bus line is as illustrated below:
 * @image html "servobox_CAN_new_1.png" "Connecting RDrive servos to USB-CAN" width=800 
 * @image latex "servobox_CAN_new_1.png" "Connecting RDrive servos to USB-CAN" width=300pt
 * 
 * Providing the CAN connection, make sure to comply with the following requirements:
 * - The CAN bus lines should be terminated with 120 Ohm resistors at both ends. You have to provide only one resistor because one is already integrated into the CAN-USB dongle supplied as part of the servobox solution.
 * - The bus line cable must be a twisted pair cable with the lay length of 2 to 4 cm.
 * - The cross section of the bus line cable must be between 0.12 mm2 to 0.3 mm2.
 * - To ensure the baud rate required for your application, L&Sigma; should meet the specific values as indicated in Table 2.
 * 
 * <b>Table 2: CAN line length vs. baud rate</b> 
 * |Baud Rate|50 kbit/s|100 kbit/s|250 kbit/s|500 kbit/s|1 Mbit/s|
 * |------------------------|---------|---------|---------|---------|---------|
 * |Total line length, L&Sigma;, m|< 1000 m|< 500 m|< 200 m|< 100 m|< 40 m|
 * 
 * @section sect1 3. Servobox components
 * @subsection eater 3.1 Energy eater
 * An energy eater is used to dissipate the dynamic braking energy that can result from servos generating voltages in excess of the power supply voltage.
 * Use the schematic below to assemble the device:  
 * @image html "eater.png" "Eater module schematic" width=400
 * @image latex "eater.png" "Eater module schematic" width=150pt
 * <b>Required components:</b>
 * |Component|Type|Other options|Comment|
 * |---------|-----|-----------|-------|
 * |D1 - Diode|APT30S20BG|Schottky diode, I<SUB>f</SUB> &ge; 20 A, V<SUB>r</SUB> &ge; 96 V|I<SUB>f</SUB> &ge; 1.5 × Total current of all connected servos|
 * |Q1 - Transistor|TIP147|PNP darlington transistor, V<SUB>ce</SUB> &ge; 96V, I<SUB>c</SUB> &ge; 10 A| |
 * |R1 - Resistor|1K Ohm, 1 W| | |
 * |R2 - Resistor|4.7 Ohm, P<SUB>d</SUB> &ge; 25 W| | |
 * |X1 - Connector| | |Input connector (power source - power supply) |
 * |X2 - Connector| | |Output connector (power consumer - capacitor module & servo) |
 * 
 * <b>Note:</b> D1, Q1, and R2 should be connected to an appropriate heatsink. The maximum dissipated power of the heatsink should be equal to the maximum dynamic braking energy in your circuit.   
 * When the power to dissipate is too high (dynamic braking power is more than 50 W), it also is essential to provide active cooling, such as a fan.
 * 
 * @subsection capacitor 3.2 Capacitor module
 * In the servobox solution, capacitors are intended to accumulate and supply electric energy to servos. The devices allow for compensating
 * short-duration power consumption peaks that are due to servos located at a distance (usually quite long) from the power supply unit.
 * For the same reason, make sure to place capacitors as close as possible to the servo.
 * To assemble the device, use the schematic below.
 * @image html "capacitor.png" "Capacitor module schematic" width=400
 * @image latex "capacitor.png" "Capacitor module schematic" width=200pt
 * <b>Requirements:</b>
 * |Component|Type|Comment|
 * |---------|-----|-------|
 * |X1 - Connector| |Input connector (power source) |
 * |X2 - Connector| |Output connector (power consumer - servo) |
 * |C1...Cn|Aluminum electrolytic capacitor or tantalum/polymer capacitor, U &ge; 80 V, ESR &le; 0.1 Ohm|Total capacitance should be &ge; 5 uF per 1 W of connected servo|
 * 
 * 
 * @ingroup hw_manual
 */
 */
