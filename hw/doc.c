/**
 * @brief Hardware manual
 * 
 * @file doc.c
 * @author Rozum
 */

/**
 * @page servo_box 
 * @section sect_descr 1. Product overview
 * A <b>servo box</b> is a solution designed to control the motion of one or more RDrive servos. The solution comprises the following components:
 * - one or more energy eaters to dissipate the dynamic braking energy
 * - one or more capacitor modules
 * - a CAN-USB dongle to provide CANOpen communication between the box and the servos
 * 
 * <b>Note:</b> Never connect eater+capacitor+servos to the working power supply. Capacitor charging current can break the power supply or injure the user!
 * At first turn off the power, assembly the circuits, and then power on the Servo box.
 * 
 * @section sect_conn 2. Connecting servos to a servobox
 * To connect a RDrive servo to a servo box, you need to connect the following two cables:
 * - a two-wire cable for connecting the servo to power supply
 * - a two-wire cable for enabling CAN communication
 * Sections 2.1 and 2.2 feature connection diagrams and requirements.
 * 
 * @subsection sect_21 2.1. Electrical connection
 * The configuration of the servo box solution (e.g., how many eaters and capacitors it uses) and the electrical connection diagram depend on whether your intention is:
 * - to connect a single servo
 * @image html "single_servo_conn.png" width=800
 * - to connect multiple servos
 * @image html "multiple_servo_conn.png" width=800
 * 
 * In any case, make sure to meet the following electrical connection requirements:
 * - Typically, the total circuit length from the power supply to the servo motor must not exceed 10 meters.
 * - Length "L1" must not be longer than 10 meters.
 * - Length "L2" (from an eater to a capacitor) should not exceed the values from Table 1.
 * - Length "L3" (from a capacitor to a motor) should not exceed the values from Table 1.
 * 
 * <b>Table 1: Line segment lengths vs. cross-sections</b> 
 * |Servo model|L1||||||L2||||||
 * |-----------|-|-|-|-|-|-|-|-|-|-|-|-|
 * |           |0.75 mm2|1.0 mm2|1.5 mm2|2.5 mm2|4.0 mm2|6.0 mm2|0.75 mm2|1.0 mm2|1.5 mm2|2.5 mm2|4.0 mm2|6.0 mm2|
 * |RD50	   |4 m	    |5 m	|8 m	|10 m	|10 m	|10 m	|0,2 m	 |0,2 m	 |0,4 m	 |0,7 m	 |1,0 m	 |1,0 m  |
 * |RD60	   |2 m	    |3 m	|5 m	|9 m	|10 m	|10 m	|0,1 m	 |0,1 m	 |0,2 m	 |0,4 m	 |1,0 m	 |1,0 m  |
 * |RD85	   |0,8 m	|1 m	|1 m	|2 m	|4 m	|6 m	|0,04 m	 |0,05 m |0,08 m |0,13 m |0,21 m |0,32 m |
 * 
 * For length 1, make sure the cable cross-section is as specified below:
 * - When the total connected motor power is <b>less than 250 W</b>,  the cable cross-section within the segment must be at least 1.00 mm2.
 * - When the total connected motor power is <b>less than 500 W</b>,  the cable cross-section within the segment must be at least 2.00 mm2.
 * 
 * @subsection sect_22 2.2. CAN connection
 * The CAN connection of RDrive servos is a two-wire bus line transmitting differential signals: CAN_HIGH and CAN_LOW. 
 * The configuration of the bus line is as illustrated below:
 * @image html "can_conn.png" "Connecting RDrive servos to USB-CAN" width=800 
 * 
 * Providing the CAN connection, make sure to comply with the following requirements:
 * - The CAN bus lines should be terminated with 120 Ohm resistors at both ends. You have to provide only one resistor because one is already integrated into the CAN-USB dongle supplied as part of the servo box solution.
 * - The bus line cable must be a twisted pair cable with the lay length of 2 to 4 cm.
 * - The cross section of the bus line cable must be between 0.12 mm2 to 0.3 mm2.
 * - To ensure the baud rate required for your application, LΣ should meet the specific values as indicated in Table 2.
 * 
 * <b>Table 2: CAN line length vs. baud rate</b> 
 * |Baud Rate|50 kbit/s|100 kbit/s|250 kbit/s|500 kbit/s|1 Mbit/s|
 * |------------------------|---------|---------|---------|---------|---------|
 * |Total line length, LΣ, m|< 1000 m|< 500 m|< 200 m|< 100 m|< 40 m|
 * 
 * @section sect1 3. DIY
 * @subsection eater 3.1 Eater Module
 * Eater module is used to dissipate the dynamic braking energy when the servo generates the voltage higher than the power supply voltage.
 * @image html "eater.png" "Eater module schematic" width=400
 * <b>Part List:</b>
 * |Component|Value|Replacement|Comment|
 * |---------|-----|-----------|-------|
 * |D1|APT30S20BG|Schottkey diode, I<SUB>f</SUB> ≥ 20 A, V<SUB>r</SUB> ≥ 96 V|I<SUB>f</SUB> ≥ 1.5 × Total current of all connected servos|
 * |Q1|TIP147|PNP darlington transistor, V<SUB>ce</SUB> ≥ 96V, I<SUB>c</SUB> ≥ 10 A| |
 * |R1|1K Ohm, 1 W| | |
 * |R2|4.7 Ohm, P<SUB>d</SUB> ≥ 25 W| | |
 * 
 * <b>Note:</b> D1, Q1 and R2 should be attached to the appropriate heatsink. 
 * Active cooling should be applied if the dissisipation power is too high (dynamic brake power is more than 50?W) .
 * 
 * @subsection capacitor 3.2 Capacitor Module
 * Capacitor module is used to deliver the stored electric energy to the servo. The main idea is to provide the "pulse" power to the servo, 
 * which can't be provided by the power supply unit cause of the long power wires between the power supply and the servo (most of cases).
 * That's why it should be placed to the servo as close as possible.
 * @image html "capacitor.png" "Capacitor module schematic" width=400
 * <b>Part List:</b>
 * |Component|Value|Comment|
 * |---------|-----|-------|
 * |С1...Cn|Aluminum Electrolytic Capacitor or Tantalum/Polymer Capacitor, U ≥ 80 V, ESR ≤ 0.1 Ohm|Total capacitance should be ≥ 5 uF per 1 W of connected servo|
 * @ingroup hw_manual
 */