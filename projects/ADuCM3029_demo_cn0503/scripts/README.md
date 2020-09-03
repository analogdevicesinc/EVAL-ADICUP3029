Prerequisites:
 - Python3
 - Python modules: PyQt5, numpy, pyserial, matplotlib and scipy
   Install them with:
      pip install -r py_requirements.txt

Folder to include python scripts.

> config_channel.py:

This script make use of cn0503 wrapper module over serial command line to
configure the cn0503 channels.

usage: config_channel.py [-h] [-f config_file] [-select_port] channel_number measurment_type

Configure CN0503 optical channel

Note: When a prompt is shown user can answare with:
      Yes, No, y, n (case insensitive) or press enter to choose the default
      answare (in parentesis). Eg:
      - Do you want to configure relative ratio baseline ? (Yes) :
      By pressing enter Yes will be selected.

Examples:

    Start fluorimetry configuration for channel 1
    >python config_channel.py 1 FLUO

    Start colorimetry configuration for channel 4
    >python config_channel.py 4 COLO

    Start turbidity configuration for channel 3 and manual select of serial port
    >python config_channel.py 3 TURB -select_port

positional arguments:
  channel_number   From 1 to 4
  measurment_type  FLUO (fluorimetry), COLO (colorimetry) or TURB (turbidity)

optional arguments:
  -h, --help       show this help message and exit
  -f config_file   File where to save the configuration commands
                   The saved file can be used to configure a new chip with load_cfg.py
                   If the file exist a prompt will ask to append or overwrite it.
                   For example, you can call config_channel 1 FLUO -f file1.cfg . Do the
                   configuration and then do the same for the second channel and append the
                   configuration to file1.cfg. Also, configuration can be appended to default chip
                   configuration files.
  -select_port     If select_port option is set, a list of available serialports will be shown.
                   Then the user can choose a port where the board is connected at.
                   Otherwise, the hardcoded DEFUALT_SERIAL_PORT value for the port will be used

> get_data.py:

usage: get_data.py [-h] [-s samples] [-select_port] {1,2,3,4} {FLUO,TURB,COLO} {CODE,ARAT,RRAT,INS1,INS2}

Get samples from a specific channel

Examples:

    Get 3(default) samples of fluorimetry data fron channel 1 displyed as
    instrumentation unit 1
    >python get_data.py 1 FLUO INS1

    Get 10 samples of data fron channel 4 displyed as relative ratio and
    selecting the communication port
    >python get_data.py 4 COLO RRAT -s 10 -select_port

positional arguments:
  {1,2,3,4}             Channel number
  {FLUO,TURB,COLO}      Measurment type: FLUO (fluorimetry), COLO (colorimetry) or TURB (turbidity)
  {CODE,ARAT,RRAT,INS1,INS2}
                        Display type: ARAT (absolut ration), RRAT (relative ratio),
                        INS1/2 (instrumentation unit 1/2)

optional arguments:
  -h, --help            show this help message and exit
  -s samples            Number of samples to read. Default is 3
  -select_port          If select_port option is set, a list of available serialports will be shown.
                        Then the user can choose a port where the board is connected at.
                        Otherwise, the hardcoded DEFUALT_SERIAL_PORT value for the port will be used

>load_cfg.py:

usage: load_cfg.py [-h] [-select_port] config_file

Load config file on CN0503
Examples:

    >python load_cfg.py -h
    >python load_cfg.py config_file
    >python load_cfg.py config_file -select_port

positional arguments:
  config_file   File with commands to configure the chip.
                Commands will be appended as they are to the FL_WRITE CLI command

optional arguments:
  -h, --help    show this help message and exit
  -select_port  If select_port option is set, a list of available serialports will be shown.
                Then the user can choose a port where the board is connected at.
                Otherwise, the hardcoded DEFUALT_SERIAL_PORT value for the port will be used
