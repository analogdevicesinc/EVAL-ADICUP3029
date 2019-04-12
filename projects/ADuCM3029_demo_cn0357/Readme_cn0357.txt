            Analog Devices, Inc. Sensor Pack Application Example


Project Name: cn0357_example_noos

Description:  Demonstrates how to use the EVAL-CN0357-ARDZ with the ADICUP3029.
 

Overview:
=========
    This example demonstrates how to use the CN0357 Carbon Monoxide sensor class in the Sensor Pack.
    The single threaded, non-RTOS program will read gas concentration in parts per million (PPM) from
    the CN0357 and then either send the data to the host application via Bluetooth or print the data,
    depending on the user configuration. 

    In debug mode, the data will be printed to the console. In release mode the data will be routed through 
    the UART to a host terminal. 

User Configuration Macros:
==========================
    ADI_APP_USE_BLUETOOTH    (cn0357_app.h) - This macro can be used to enable or disable Bluetooth connectivity.
    ADI_APP_DISPATCH_TIMEOUT (cn0357_app.h) - This macro controls how frequently gas concentration samples are sent.

Hardware Setup:
===============
    Connect the EVAL-CN0357-ARDZ to the Arduino compatible shield located on the ADICUP3029. There is only one 
    method of aligning these two boards to attach them, but you can align the four connectors as described below.

        ADICUP3029          EVAL-CN0357-ARDZ
        ==========          ================
        P4                  POWER
        P3                  ANALOG
        P6                  DIGI1
        P7                  DIGI0

    The two jumpers located on the EVAL-CN0357-ARDZ must be configured properly. 

        Jumper       Position
        ======       ========
        AD7790_CS    1 - Top (closest to the big circle)
        AD5270_CS    2 - Middle

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
        4. Click on "CN0357 Demo" once it becomes available on the list of devices. This will create a connection to the ADICUP3029 board.
        5. Click "DISCONNECT" to disconnect from the ADICUP3029 board.

    If ADI_APP_USE_BLUETOOTH is set to 0u, the demo does not require user intervention. 

Expected Result:
================
    If ADI_APP_USE_BLUETOOTH is set to 0u, the following should be printed to the console/terminal:
        
        Starting CN0357 Demo application

        Gas Concentration = 3.662113 PPM

        Gas Concentration = 3.662113 PPM

        (repeat forever)

    If ADI_APP_USE_BLUETOOTH is set to 1u, the following should be printed to the console/terminal:

        Starting CN0357 Demo application

        Waiting for connection. Initiate connection on central device please.

        Connected!

        GAP mode changed.

        Data sent!

        Data sent!

        (repeat until other BLE events occur)

Gas Concentration Values:
========================
The values are expected to be between [0:9] PPM. However, we have observed that
the values start very large and after a period of time (10-15 minutes) decrease
to a lower value that is within expected range.

References:
===========
    EVAL-CN0357-ARDZ Schematic.
    ADICUP3029 Schematic.
    ADI-SensorSoftware_1.0.0_Release_Notes.pdf
    EVAL-ADICUP3029_Users_Guide.pdf
    IoT_Node_Users_Guide.pdf
