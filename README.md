# Avy-Dawgs: Avalanche Beacon

### Group Members 
* Aidan Leary
* Hiram Perez
* Dax Jennings
* Noah Sikorski

## Project Purpose/Functionality
This pupose of this project is to receive the 457kHz signal from a transmitting avalanche beacon and guide the user to it. This is a hanheld device that displays its calculated distance from the transmitting beacon as well as an arrow to guide the user to the transmitting beacon. Because the range of our antenna isn't very long, we also wrote code to simulate the output of a transmitting beacon from a waveform generator in order to show the display of output and direction output by our device.

## Repository Structure 
This repository is organized into the following folders. 

### ad2 
Beacon waveform simulation on Analog Discovery 2.
### beaconTracking 
Pathfinding algorithm.
### documentation/datasheets
Data sheet and manuals for STM board.
### fpga
HDL files, constraint files, Vivado files.
### mcu 
All libraries and custom software.
### pcb 
KiCAD project for RF front-end and ADC PCB.
### scripts
Generates C array to use as buffer for DSP code.
