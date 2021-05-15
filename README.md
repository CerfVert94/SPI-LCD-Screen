# SPI-LCD-Screen
A pet project to program SPI communication protocol from the scratch to control a LCD module with Raspberry Pi.<br>
So it uses a bit banging with GPIO instead of the standard SPI driver.
GPIO is manipulated by mapping the memory and controlling directly the register at the address specified by /dev/gpiomem.<br>

The module number of the LCD screen is <b><i>COG-C144MVGI-08</i></b>, whose driving IC is Samsung <b><i>S6B3306 </b></i>.<br>
In order to manipulate the hardware registers and know their address, I referred to BCM2835 ARM peripheral documentation.
At some point, I used wiringPi(http://wiringpi.com/) library to compare the GPIO switching time, etc.


## Timing (to be verified.)
While a sleep function like "nanosleep" is provided for Raspberry Pi, the time measurement showed a huge discrepancy between the desired sleep duration and the actual duration at. This was measured by using oscilloscope and switching GPIO on and off. I recall that the switching time of GPIO measured by the oscilloscope was about 36 - 40 nanoseconds, but this is to be verified.<br>

Anyway, in order to implement a sleep function with correct duration, I used the timer of BCM2835 by mapping the memory at /dev/mem + 0x20003000.<br>
While this seemed to increase the precision, it didn't work well below 400us, according to the <b>delay_fnc_result.txt</b> (Some part of this text file seems rather cryptic, and I do not know what I was thinking, but what matters is fortunately interpretable). <br>
  
The problem seems that we don't have a control over the OS scheduling and resource allocation. To increase the precision and avoid sudden spikes in sleep duration, I probably should use a Real Time OS. 



## Video Demo
https://youtu.be/34jqGsdNCsc 

[![Video Demo](https://img.youtube.com/vi/34jqGsdNCsc/0.jpg)](https://www.youtube.com/watch?v=34jqGsdNCsc)

The video used for the demo is not actually a video but rather a GIF animation (but not exactly. Refer to the <b>PixToMx.m</b>, the matlab file.) 
It's just a series of bitmap images drawn at a high speed.

