            Analog Devices, Inc. Sensor Pack Application Example


Project Name: adxl362_example_noos

Description:  Demonstrates how to use ADXL362 acceleromter sensor with the ADICUP3029.
 

Overview:
=========
    This example demonstrates how to use the ADXL362 accelerometer sensor class in the Sensor Pack.
    The single threaded, non-RTOS program reads acceleromter x, y, z acceleration values and then 
    either sends the data to the host application via Bluetooth or prints the data, depending on the user 
    configuration.

    In debug mode, the data will be printed to the console. In release mode the data will be routed through 
    the UART to a host terminal. 


User Configuration Macros:
==========================
    ADI_APP_USE_BLUETOOTH    (adxl362_app.h) - This macro can be used to enable or disable Bluetooth connectivity.
    ADI_APP_DISPATCH_TIMEOUT (adxl362_app.h) - This macro controls how frequently accelerometer samples are sent.

Hardware Setup:
===============
    Connect the PmodACL2 acceleromter to the SPI1 PMOD connector on the ADICUP3029 board. PMOD connectors are on the opposite end of the board from the power connector. The underside of the board labels which of the two PMOD connectors is for SPI1.

    In release mode the data will be printed to the host terminal. The UART redirection setup on the ADICUP3029 is detailed below:
    
        Switch       Position
        ======       ========
        S2           USB

Host App Setup:
===============
    Please refer to section "Installation" in the Android_Application_Users_Guide.pdf for details on
    how to install the Android app.

External connections:
=====================
    Connect the ADICUP3029 to the host PC using the USB cable.

How to build and run:
=====================
    In CCES, build the project ("Project" --> "Build All"). Then click one of the ".launch" files located in the Project
    Explorer, and then press "Debug" icon. This will open the "Debug" perspective in CCES. Run the program ("Run" -> "Resume").
    A shortcut in CCES to begin a debug session is to click on a ".launch" file and press F5 on the keyboard. 

    For a release build the output will be redirected to the host terminal via the UART. Click on "ADICUP3029_Release.launch" and 
    press F5 on the keyboard to begin the release build debug session. Before running the example, open a terminal (TeraTerm, Putty, etc) on the hose machine. Set up the serial port with the following settings:
        
        Serial Setup             Value
        ===========              ========
        Baud rate                9600
        Data                     8-bit
        Parity                   None
        Stop                     1-bit
        Flow control             None

    If ADI_APP_USE_BLUETOOTH is set to 1u, follow the instructions below: 
        1. Build and run the demo. 
        2. On the phone/table open the Android application ADIotNode.
        3. Press the "SCAN" button.
        4. Click on "ADXL362 Demo" once it becomes available on the list of devices. This will create a connection to the ADICUP3029 board.
        5. Click "DISCONNECT" to disconnect from the ADICUP3029 board.

    If ADI_APP_USE_BLUETOOTH is set to 0u, the demo does not require user intervention. 

Expected Result:
================
    If ADI_APP_USE_BLUETOOTH is set to 0u, the following should be printed to the console/terminal:

        Starting ADXL362 accelerometer demo application

        X value = -29 		

	    Y value = -21 		

	    Z value = 1283 		

        (repeat forever)

    If ADI_APP_USE_BLUETOOTH is set to 1u, the following should be printed to the console/terminal: 
        
        Starting ADXL362 accelerometer demo application
                
        Waiting for connection. Initiate connection on central device please.

        Connected!

        GAP mode changed.

        Data sent!

        Data sent!

        Data sent!

        (repeat until other BLE events occur)

References:
===========
    PmodACL2 Schematic.
    ADICUP3029 Schematic.
    ADI-SensorSoftware_1.0.0_Release_Notes.pdf
    EVAL-ADICUP3029_Users_Guide.pdf
    IoT_Node_Users_Guide.pdf