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
 * <b>A servobox</b> is a solution designed to ensure safe and correct operation of one or more RDrive servo motors and to control their motion. The solution comprises the following components:
 * - one or more energy eaters (see Section 2.1)
 * - one or more capacitor modules (see Section 2.2)
 * - a CAN-USB dongle to provide communication between the servobox and the servos
 * 
 * It is the user's responsibility to additionally provide a power supply and USB-A to Micro USB cable for connecting the CAN-USB dongle to PC.
 * 
 * The USB-A to Micro USB cable should not be longer than 5 meters.
 * The power supply must meet the following requirements:
 * - its supply voltage should be 48 V max
 * - its power should be equal to the total nominal power of all servo motors connected to it multiplied by a factor of 2.5 to 3.
 * 
 * @section sect1 2. Servobox components
 * @subsection eater 2.1 Energy eater
 * An energy eater is used to dissipate dynamic braking energy. When not dissipated, this energy can cause servos to generate voltages in excess of the power supply voltage, which can damage servos beyond repair.
 * You can either buy the eater module from Rozum Robotics or use the schematic below to assemble it on your own:  
 * @image html "eater.png" "Eater module schematic" width=400
 * @image latex "eater.png" "Eater module schematic" width=150pt
 * <b>Required components:</b>
 * |Component|Type|Other options|Comments|
 * |---------|-----|-----------|-------|
 * |D1 - Diode|APT30S20BG|Schottky diode, I<SUB>f</SUB> &ge; 20 A, V<SUB>r</SUB> &ge; 96 V|I<SUB>f</SUB> &ge; 1.5 × Total current of all connected servos|
 * |Q1 - Transistor|TIP147|PNP darlington transistor, V<SUB>ce</SUB> &ge; 96V, I<SUB>c</SUB> &ge; 10 A| |
 * |R1 - Resistor|1K Ohm, 1 W| | |
 * |R2 - Resistor|4.7 Ohm, P<SUB>d</SUB> &ge; 25 W| | |
 * |X1 - Connector| | |Input connector (from the power source to the power supply) |
 * |X2 - Connector| | |Output connector (from the power consumer to the capacitor and servo) |
 * 
 * <b>Heatsink requirements</b><br>
 * <p>As you can see in the <b>Eater module schematic</b>, D1, Q1, and R2 should be connected to an appropriate heatsink.</p>
 * <p>The simple energy eater that you can assemble based on the schematic
 * can dissipate 25 W of average power at the ambient temperature of 60 degrees C max.
 * Therefore, you will need a heatsink with thermal resistance of at least 1W/deg C.</p>
 * <p>To comply with the requirement, meet the following characteristics:
 * <ul><li>forced air convection with the flow rate of 15m3/h</li>
 * <li>heat dissipating surface of 600 cm2</li></ul></p>
 * <p>For example, this can be a heatsink as described below:
 * <ul><li>heatsink size—100 x 100 mm</li>
 * <li>fan size—40 x 40 mm</li>
 * <li>fin quantity—12</li>
 * <li>fin height—25 mm</li></ul></p>
 * 
 * 
 * @subsection capacitor 2.2 Capacitor module
 * In the servobox solution, capacitors are intended to accumulate and supply electric energy to servos. The devices allow for compensating
 * short-term power consumption peaks that are due to inductive resistance. Since servos are usually located at a long distance from the power supply unit,
 * inductive resistance values can be rather high.
 * For the same reason, make sure to place capacitors as close to servos as possible.
 * <p>To assemble a capacitor module, use the schematic below:</p>
 * @image html "capacitor.png" "Capacitor module schematic" width=400
 * @image latex "capacitor.png" "Capacitor module schematic" width=200pt
 * <b>Requirements:</b>
 * |Component|Type|Comments|
 * |---------|-----|-------|
 * |X1 - Connector| |Input connector (power source) |
 * |X2 - Connector| |Output connector (from the power consumer to the servo) |
 * |C1...Cn|Aluminum electrolytic capacitor or tantalum/polymer capacitor, U &ge; 80 V|Total capacitance should be &ge; 5 uF per 1 W of connected servo power|
 * <b>Note:</b> Total capacitor ESR &le; 0.1 Ohm
 *  
 * @section sect_conn 3. Connecting servos to a power supply and a servobox
 * 
 * To integrate an RDrive servo into one circuit with a power supply and a servobox, you need to provide the following connections:
 * 
 * - power supply connection (two wires on the servo housing)
 * - CAN communication connection (two wires on the servo housing)
 * 
 * For connection diagrams and requirements, see Sections 3.1 and 3.2.
 * For eater and capacitor requirements and schematic, see Section 2.1 and 2.2.
 * 
 * @subsection sect_21 3.1. Power supply connection
 * 
 * <b>Note:</b> Never supply power before a servo (servos) is (are) fully integrated with a servo box and a power supply into one circuit.
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
 * - The total circuit length from the power supply to any servo motor must not exceed 10 meters.
 * - The L1 length must not be longer than 10 meters.
 *     - When the total connected motor power is <b>less than 250 W</b>,  the cable cross-section within the segment must be at least 1.00 mm2.
 *     - When the total connected motor power is <b>less than 500 W</b>,  the cable cross-section within the segment must be at least 2.00 mm2.
 * - The L2 length (from the eater to the capacitor) must not exceed the values from Table 1.
 * - The L3 length (from the capacitor to any servo) must not exceed the values from Table 1.
 * 
 * <b>Table 1: Line segment lengths vs. cross-sections</b> 
 * |Servo model|L2||||||L3||||||
 * |-----------|-|-|-|-|-|-|-|-|-|-|-|-|
 * |           |0.75 mm<SUP>2</SUP>|1.0 mm<SUP>2</SUP>|1.5 mm<SUP>2</SUP>|2.5 mm<SUP>2</SUP>|4.0 mm<SUP>2</SUP>|6.0 mm<SUP>2</SUP>|0.75 mm<SUP>2</SUP>|1.0 mm<SUP>2</SUP>|1.5 mm<SUP>2</SUP>|2.5 mm<SUP>2</SUP>|4.0 mm<SUP>2</SUP>|6.0 mm<SUP>2</SUP>|
 * |RD50	   |4 m	    |5 m	|8 m	|10 m	|10 m	|10 m	|0,2 m	 |0,2 m	 |0,4 m	 |0,7 m	 |1,0 m	 |1,0 m  |
 * |RD60	   |2 m	    |3 m	|5 m	|9 m	|10 m	|10 m	|0,1 m	 |0,1 m	 |0,2 m	 |0,4 m	 |1,0 m	 |1,0 m  |
 * |RD85	   |0,8 m	|1 m	|1 m	|2 m	|4 m	|6 m	|0,04 m	 |0,05 m |0,08 m |0,13 m |0,21 m |0,32 m |
 * 
 * @subsection sect_22 3.2. CAN connection
 * The CAN connection of RDrive servos is a two-wire bus line transmitting differential signals: CAN_HIGH and CAN_LOW. 
 * The configuration of the bus line is as illustrated below:
 * @image html "servobox_CAN_new_1.png" "Connecting RDrive servos to USB-CAN" width=800 
 * @image latex "servobox_CAN_new_1.png" "Connecting RDrive servos to USB-CAN" width=300pt
 * 
 * Providing the CAN connection, make sure to comply with the following requirements:
 * - CAN bus lines of less than 40 m long should be terminated with 120 Ohm resistors at both ends. For bus lines of over 40 m long, use 150-300 Ohm resistors.<br>
 * <b>Note:</b> You have to provide only one resistor because one is already integrated into the CAN-USB dongle supplied as part of the servobox solution.
 * - The bus line cable must be a twisted pair cable with the lay length of 2 to 4 cm.
 * - The cross section of the bus line cable must be between 0.12 mm2 to 0.3 mm2.
 * - To ensure the baud rate required for your application, L&Sigma; should meet the specific values as indicated in Table 2.
 * 
 * <b>Table 2: CAN line parameters</b> 
 * |Baud Rate |50 kbit/s|100 kbit/s|250 kbit/s|500 kbit/s|1 kbit/s|
 * |------------------------|---------|---------|---------|---------|---------|
 * |Total line length, L&Sigma;, m|< 1000|< 500 |< 200|< 100|< 40|
 * |Cross-section, mm2|0.75 to 0.8|0.5 to 0.6|0.34 to 0.6|0.34 to 0.6|0.25 to 0.34|
 * 
 * <p><b>Connecting multiple servos to a CAN bus</b></p>
 * 
 * To avoid collisions, each servo connected to a CAN bus line must have a unique CAN identifier. However, RDrive servo motors are all 
 * supplied with the same <b>default ID—32</b>. Therefore, an important step of connecting multiple RDrive servos
 * to a single CAN bus line is to change their default IDs to unique ones.
 * 
 * <p>Start with connecting each of your multiple servos, one by one, to a CAN bus line, following the sequence as described below:<br>
 * 
 * <b>Caution!</b> Never connect or disconnect servos when power supply is on.<br>
 * 
 * 1. Take servo 1 and connect it to the CAN bus line as desribed in this section above.<br>
 * 2. Run the \ref tutor_c_changeID1 tutorial to change the servo's default ID.<br>
 * 3. Remember or write down the newly assigned CAN ID and disconnect the servo.<br> 
 * 4. Repeat Steps 1 to 3 for all the servos you want to connect to the same CAN bus line.<br>
 * 
 * Once you have changed all CAN IDs as appropriate, you can connect all the servos back to the CAN bus line and start working with them.
 * 
 * <b>Note</b>: In total, you can connect up to 127 devices to a single CAN bus. So, the admissible CAN ID pool is from 1 to 127.
 * 
 * @ingroup hw_manual
 */
