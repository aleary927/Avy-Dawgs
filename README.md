# Avy-Dawgs: Avalanche Beacon

### Group Members 
* Aidan Leary
* Hiram Perez
* Dax Jennings
* Noah Sikorski

## Project Purpose/Functionality
This pupose of this project is to receive the 457kHz signal from a transmitting avalanche beacon and guide the user to it. 
This is a hanheld device that displays its received power from the transmitting beacon as well as a direction that guides the user to the transmitting beacon. 

## Repository Structure 
This repository is organized into the following folders. 

### ad2 
Beacon waveform simulation on Analog Discovery 2.
### beaconTracking 
Pathfinding algorithm and MATLAB simulations to test guidance algorithm.
### documentation/datasheets
Data sheet and manuals for STM board.
### mcu 
All libraries and custom software.
### pcb 
KiCAD project for ADC PCB.
### scripts
Miscellaneous scripts.

## Hardware Setup

### MCU 
STM32F722
### Antennas
Orthogonally mounted ferrite rod antennas on XY plane.
Antennas have two windings, a primary for a resonant LC circuit, and a secondary to take signal from.
### RF Front-End
RF amplifiying circuit designed using a TL084 to amplifiy inital signal with, followed by a 455 KHz ceramic filter with a 6dB insertion and +- 5KHz
passband, lastly followed by a copy of the RF amplifiying circut to boost the initial signal to roughly 22dB after losing gain through the ceramic filter. 
Total gain of about 22 dB.
### User Interface
UART serial output.
Provides direction, and received signal power.

## Software Overview

The software processes input from the on-board analog-to-digital converters to produce a power reading at 457 kHz.

- Removes DC bias and applies proper window
- Calculates received power at 457 kHz using Goertzel algorithm.
- Buffers results and computes rolling averages
- Determines direction to travel
- Outputs direction and power via UART

## Guidance Algorithm
This algorithm uses signal strength trends and angular ratio between x and y channels to select a movement direction.

- Tells user to reverses if signal is weakening
- Otherwise steer using angle and recent ratio trends
- Cooldown prevents flip-flopping between directions

Available outputs:
- `STRAIGHT_AHEAD`
- `TURN_LEFT`
- `TURN_RIGHT`
- `TURN_AROUND`

## Running the Project
