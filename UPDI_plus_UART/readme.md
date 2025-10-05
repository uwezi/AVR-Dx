# UPDI
The new AVR chips use a single-wire serial programming interface UPDI. There are plenty of sources on the web on how to build an avrdude-compatible programmer
using just a standard USB-UART TTL dongle.

But now I took the general idea of my USBasp clone which I had developed for my students about 10 years ago, where the programmer snaps onto the breadboard and 
automatically provides power to the breadboard's side rails as well. As usual I let [PCBgogo](https://www.pcbgogo.com/) manufacture my 2-layer boards and was very satisfied 
with their services.

<img width="672" height="568" alt="image" src="https://github.com/user-attachments/assets/d81acdd6-1bb7-4d86-88f5-67d012e74c1d" />

The design uses the lesser known [CH342K](https://docs.sparkfun.com/SparkFun_RTK_Postcard/assets/component_documentation/CH342%20Datasheet.pdf) 
which can be sourced from LCSC and ALiexpress for about € 1,50 a piece - I found no "western" source for this chip. This chip is similar to the 
CH340-series of USB-UARTs, but it even contains two independent serial ports in a single 10pin SMD package, needs no external oscillator and
seems to be already supported by all modern operating systems.

## KiCAD 3D rendering
<img width="1569" height="845" alt="image" src="https://github.com/user-attachments/assets/d088a427-184c-41e3-acdc-ba960df3a401" />

## KiCAD view of both front and back layer
<img width="6136" height="2456" alt="image" src="https://github.com/user-attachments/assets/d5e750f2-c559-4e88-9f6d-d164812b02ab" />

You find the KiCAD and gerber files in this directory.

