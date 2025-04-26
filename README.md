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
Miscallanious scripts.

## Hardware Setup

### MCU 
STM32F722
### Signal Input
Orthogonally mounted ferrite rod antennas on X and Y planes.
### Signal Source
457kHz analog waveform (from real avalanche beacon or simulated)
### Analog Front-End
Custom PCB with amplifier/filter
### User Interface
UART serial output

## Software Overview

The firmware processes beacon signals in real-time using Goertzel filtering:

- Samples X and Y channels
- Removes DC bias and applies proper window
- Filters signal for 457kHz signal using Goertzel algorithm
- Buffers results and computes rolling averages
- Feeds values to `guidance_step()` to determine direction
- Outputs results via UART

## Guidance Logic Summary
This function compares signal strength trends and angular ratio between channels to select a movement direction.

- Enter reverse mode if signal is weakening
- Otherwise steer using angle and recent ratio trends
- Cooldown prevents flip-flopping between directions

Available outputs:
- `STRAIGHT_AHEAD`
- `TURN_LEFT`
- `TURN_RIGHT`
- `TURN_AROUND`

## Running the Project
