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
 * - a CAN-USB dongle to provide CANOpen communication between the box and the servos
 * 
 * Additionally, to ensure operation of the servo box, the user has to provide a power supply and USB-A to Micro USB cable to connect the CAN-USB dongle to your PC.
 * 
 * The power supply should meet the following requirements:
 * - its supply voltage should not exceed 48 V
 * - its power should be equal to the total peak power of all servo motors connected to it
 * 
 * @section sect_conn 2. Integrating servos with a power supply and a servobox
 * 
 * To integrate a RDrive servo into one circuit with a power supply and a servobox, you need to connect the following two cables:
 * 
 * - a two-wire cable for connecting the servo to power supply
 * - a two-wire cable for enabling CAN communication
 * 
 * For connection diagrams and requirements, see Sections 2.1 and 2.2.
 * 
 * @subsection sect_21 2.1. Electrical connection
 * 
 * <b>Note:</b> Never connect power supply before a servo(s) are fully integrated with a servobox and a power supply into one circuit. Charging current of the capacitor(s) can damage the power supply or injure the user!
 * 
 * The configuration of the servo box solution (e.g., how many eaters and capacitors it uses) and the electrical connection diagram depend on whether your intention is:
 * - to connect a single servo (for connection diagram, see figure below)
 * @image html "single_servo_conn.png" width=800
 * - to connect multiple servos (for connection diagram, see figure below)
 * @image html "multiple_servo_conn.png" width=800
 * 
 * In any case, make sure to meet the following electrical connection requirements:
 * - Typically, the total circuit length from the power supply any servo motor must not exceed 10 meters.
 * - Length "L1" must not be longer than 10 meters.
 * - Length "L2" (from the eater to the capacitor) should not exceed the values from Table 1.
 * - Length "L3" (from the capacitor to any servo) should not exceed the values from Table 1.
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
 * @image html "can_con.png" "Connecting RDrive servos to USB-CAN" width=800 
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
 * @section sect1 3. Servobox components
 * @subsection eater 3.1 Energy eater
 * An energy eater is used to dissipate the dynamic braking energy that can result from a servo generating voltages higher than the power supply voltage.
 * @image html "eater.png" "Eater module schematic" width=400
 * <b>Part List:</b>
 * |Component|Value|Replacement|Comment|
 * |---------|-----|-----------|-------|
 * |D1|APT30S20BG|Schottkey diode, I<SUB>f</SUB> ≥ 20 A, V<SUB>r</SUB> ≥ 96 V|I<SUB>f</SUB> ≥ 1.5 × Total current of all connected servos|
 * |Q1|TIP147|PNP darlington transistor, V<SUB>ce</SUB> ≥ 96V, I<SUB>c</SUB> ≥ 10 A| |
 * |R1|1K Ohm, 1 W| | |
 * |R2|4.7 Ohm, P<SUB>d</SUB> ≥ 25 W| | |
 * 
 * <b>Note:</b> D1, Q1, and R2 should be connected to an appropriate heatsink. 
 * Active cooling should be provided if the dissisipation power is too high (dynamic braking power is more than 50 W) .
 * 
 * @subsection capacitor 3.2 Capacitor module
 * Capacitors accumulate and deliver electric energy to servos. The main idea is to supply servos with "pulse" power that
 * the power supply unit cannot provide because the wiring between the power supply and the servo is too long (in most of cases).
 * For this reason, capacitors should be placed as close as possible to the servo.
 * @image html "capacitor.png" "Capacitor module schematic" width=400
 * <b>Part List:</b>
 * |Component|Value|Comment|
 * |---------|-----|-------|
 * |С1...Cn|Aluminum Electrolytic Capacitor or Tantalum/Polymer Capacitor, U ≥ 80 V, ESR ≤ 0.1 Ohm|Total capacitance should be ≥ 5 uF per 1 W of connected servo|
 * @ingroup hw_manual
 */