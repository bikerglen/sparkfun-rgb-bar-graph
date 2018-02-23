Some notes:

Altera and Xilinx versions are the same except for the interface used to interface to the CPU. For Xilinx, it's an AXI4-Lite bus. For Intel/Altera it's an Avalon bus.

Minimum value of BASE_DELAY in both versions of code is 22 + (2 * (number of columns)). For the SparkFun RGB bar graph this is 22 + 2 * 16 =  22 + 32 = 54.

Refresh rate is calculated as

	(input clock frequency) / ((number of rows) * (BASE_DELAY) * (2^(bit depth) - 1)

For the SparkFun RGB LED bar graph this works out to

	(50e6) / (16 * 54 * (2^8 - 1)) = 227 Hz

For the DF Robot / Adafruit 64x64 RGB LED panels this works out to

	(50e6) / (32 * (22+2*64) * (2^6 - 1)) = 165 Hz

For my 96 x 64 video "wall" this works out to 

	(50e6) / (16 * 960 * (2^4 - 1)) = 217 Hz

In theory the base delay of 960 for these panels could be lowered to 22 + 2 * 96 = 214.
