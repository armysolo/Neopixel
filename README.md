I have a project I'm working on that uses an Arduino Pro Mini driving 3x 16 WS2812 NeoPixel rings.  
The LED data in are wired to individual data pins and the code currently works.  
What I would like is to be able to modify the sketch so the NeoPixels are driven by a single pin daisy chained together.  
The end goal is to be able to move from an Arduino based MCU to a Raspberry Pi.  

I know you need to define the sketch with something like:  
(index 0  
led count 16)  
(index 16  
led count 16)  
(index 32  
led count 16)  
