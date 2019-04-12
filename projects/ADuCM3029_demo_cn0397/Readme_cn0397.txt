            Analog Devices, Inc. Sensor Pack Application Example


Project Name: cn0397_example_noos

Description:  Demonstrates how to use the EVAL-CN0397-ARDZ with the ADICUP3029.
 

Overview:
=========
    This example demonstrates how to use the CN0397 Visible Light sensor class in the Sensor Pack.
    The single threaded, non-RTOS program will first run a zero calibration on the visible light sensor. This will require the sensor to be covered by the user during this calibration. Then it will read the light intensity in lux, the SI unit of illuminance, from the CN0397 and then either send the data to the host application via Bluetooth or print the data, depending on the user configuration. 

    In debug mode, the data will be printed to the console. In release mode the data will be routed through 
    the UART to a host terminal. 

User Configuration Macros:
==========================
    ADI_APP_USE_BLUETOOTH    (cn0397_app.h) - This macro can be used to enable or disable Bluetooth connectivity.
    ADI_APP_DISPATCH_TIMEOUT (cn0397_app.h) - This macro controls how frequently light intensity samples are sent.

Hardware Setup:
===============
    Connect the EVAL-CN0397-ARDZ to the Arduino compatible shield located on the ADICUP3029. There is only one 
    method of aligning these two boards to attach them, but you can align the four connectors as described below.

        ADICUP3029          EVAL-CN0397-ARDZ
        ==========          ================
        P4                  POWER
        P3                  ANALOG
        P6                  DIGI1
        P7                  DIGI0

    The jumper located on the EVAL-CN0397-ARDZ must be configured properly. 

        Jumper       Position
        ======       ========
        P1           CS1 - closest to the photodiodes

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

    For a release build the output will be redirected to the host terminal via the UART. Click on "ADICUP3029_Release.launch" and press F5 on the keyboard to begin the release build debug session. Before running the example, open a terminal (TeraTerm, Putty, etc) on the hose machine. Set up the serial port with the following settings:
        
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
        4. Click on "CN0397 Demo" once it becomes available on the list of devices. This will create a connection to the ADICUP3029 board.
        5. Click "DISCONNECT" to disconnect from the ADICUP3029 board.

    If ADI_APP_USE_BLUETOOTH is set to 0u, the demo does not require user intervention. 

Expected Result:
================
    If ADI_APP_USE_BLUETOOTH is set to 0u, in debug mode following should be printed to the console:
        
		Starting visual light sensor demo application

		Please cover the sensor for the calibrations. 

		Calibration complete. You no longer need to cover the sensor.

		RED   CHANNEL						GREEN CHANNEL					BLUE  CHANNEL				
		Light Intensity = 0.00 lux			Light Intensity = 0.00 lux		Light Intensity = 0.00 lux		
		Light Concentration = 0.00 %		Light Concentration = 0.00 %	Light Concentration = 0.00 %		


		RED   CHANNEL						GREEN CHANNEL					BLUE  CHANNEL				
		Light Intensity = 2.12 lux			Light Intensity = 458.78 lux	Light Intensity = 90.86 lux		
		Light Concentration = 0.01 %		Light Concentration = 5.17 %	Light Concentration = 0.34 %	

	    (repeat forever)

	If ADI_APP_USE_BLUETOOTH is set to 0u, in release mode following should be printed to the terminal:
		
		Starting visual light sensor demo application

		Please cover the sensor for the calibrations. 

		Calibration complete. You no longer need to cover the sensor.

		RED channel:[                     ]             Green channel:[                     ]           Blue channel:[                     ]
        Light Intensity = 0.00 lux                      Light Intensity = 463.03 lux                    Light Intensity = 97.20 lux
        Light Concentration = 0.00 %                    Light Concentration = 5.21 %                    Light Concentration = 0.36 %

        RED channel:[                     ]             Green channel:[                     ]           Blue channel:[                     ]
        Light Intensity = 0.00 lux                      Light Intensity = 463.03 lux                    Light Intensity = 95.08 lux
        Light Concentration = 0.00 %                    Light Concentration = 5.21 %                    Light Concentration = 0.35 %

    If ADI_APP_USE_BLUETOOTH is set to 1u, the following should be printed to the console/terminal:

		Starting visual light sensor demo application

        Waiting for connection. Initiate connection on central device please.

        Connected!

        GAP mode changed.

        Data sent!

        Data sent!

        (repeat until other BLE events occur)

References:
===========
    EVAL-CN0397-ARDZ Schematic.
    ADICUP3029 Schematic.
    ADI-SensorSoftware_1.0.0_Release_Notes.pdf
    EVAL-ADICUP3029_Users_Guide.pdf
    Android_Application_Users_Guide.pdf
