# ARINC-POC

![Preview](P:\qinno\25_ARINC-POC\25_ARINC-POC_si\github\photos\ARINC-POC-top.jpg)
---

# About

This project is a (successful) proof of concept for a low-cost ARINC-429 transmitter/receiver based on Raspberry's RP2xxx family.

<img title="" src="file:///P:/qinno/25_ARINC-POC/25_ARINC-POC_si/github/photos/Aufbau12Channel.jpg" alt="Aufbau12Channel.jpg" data-align="inline">

12-channel (6 x TX, 6 x RX) loopback test with Raspberry PICO-2

# Capabilities

It demonstrates the capabilities of the integrated PIO without the need for an FPGA. In principle, this method should also work for MIL-STD-1553 with 1 MBit/s and a modified driver circuit.
With an RP2040 you have 8 channels, with an RP2350 you have 12 channels.
Each channel can act as either a transmitter or receiver, limited only by the circuitry connected.

# Built With

- an ARINC-429 optimized driver circuit on a PCB inspired by a design by Stephen Woodward [schematic](./schematic/ARINC-POC.pdf)

<img title="" src="file:///P:/qinno/25_ARINC-POC/25_ARINC-POC_si/github/photos/ARINC-POC-bot.jpg" alt="ARINC-POC-top.jpg" data-align="center">

- an RP2040/RP2350 module like Raspberry PICO/PICO2, also the -W variants
  
  <img src="file:///P:/qinno/25_ARINC-POC/25_ARINC-POC_si/github/photos/PICO2.png" title="" alt="PICO2.png" data-align="center">

- the lab software written for the Arduino IDE + Earl Philhower's great Arduino-Pico BSP. [source](./src/Arinc429)

# ToDo

- Electronic/SW switching of rise/fall times for 100k/12.5k transfer rates
- Improved error detection and signalling
- Interrupt and DMA control of RX/TX units
- ESD and EMC optimisation of electronics
- Galvanic isolation of the terminal interface (WiFi, Ethernet?)
- MIL-STD-1553 testing 
- Electronic rework in case of commercial product (downsizing, integration)

# Warning

This Proof of Concept is not intended for installation in any type of aircraft or military equipment. If you have any such requirements, [please contact us.](https://www.qinno.de)

# License

This project is licensed under the **GPL license**. 

See LICENSE for more information.

# Acknowledgements

Thanks for these awesome resources that were used during the development of  **ARINC-POC**:

* [the KiCAD team for their great CAE tool](https://www.kicad.org/)
* [Earle F. Philhower, III for his Arduino library for the RP2040](https://github.com/earlephilhower/arduino-pico)
* [and the Arduino team for their easy to use IDE](https://www.arduino.cc/en/software)
