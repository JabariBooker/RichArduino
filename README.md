# RichArduino
The RichArduino is a FPGA-based alternative to the Arduino platform. The Arduino platform provides support for a swath of useful functions: I2C communication, digital I/O, etc. Because of this, the Computer Science and Engineering department of Washington University in St Louis uses Arduino to teach CSE 132, an introduction to Computer Engineering. However, the platform also limits users to a specific processor as the microcontroller of the board. This does not allow for the exploration of microcontroller design for higher level classes. Therefore, we aim to create a new platform inspired by Arduino that allows developers to define a microcontroller for the board, the RichArduino.

The RichArduino replaces the Atmel 8-bit AVR microcontroller found on most Arduino boards with an FPGA. This FPGA is connected to a USB port, an HDMI port, and digital I/O pins. The RichArduino platform is designed to allow developers to program the FPGA with a microcontroller description that can drive and respond to these ports and pins.

**Developed by:**

* _Nathan Jarvis_, undergraduate at Washington University in. St Louis

* _Jackson Hyde_, undergraduate at Washington University in. St Louis

* _Dean Choi_, undergraduate at Washington University in. St Louis

* _Jabari Booker_, undergraduate at Washington University in. St Louis

**Supervised by:**

* _Dr. William D. Richard_, Associate Professor of Computer Engineering at Washington University in. St Louis
