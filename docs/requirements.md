# Requirements Document

## Introduction & Purpose
This document defines the requirements for an autonomous mobile rover platform. 
The AutoRover will navigate a defined test area, collect environmental data via onboard sensors,
and make operational decisions based on system state. This document establishes the functional and
non-functional requirements that the system must satisfy, and serves as the basis for design, implementation, 
and verification.
## Scope
This document covers the firmware system for AutoRover, including low-level peripheral drivers, autonomous navigation and decision logic, onboard sensor integration, and UART-based serial output for data logging and diagnostics. It also covers the wiring-level schematic defining connections between the MCU and all hardware components.

The following areas fall outside the scope of this project: mechanical design and chassis fabrication, custom PCB design, wireless communication, off-board or host-side software, cloud connectivity / IoT platform integration.

## System Overview
AutoRover is an autonomous mobile platform built on an STM32 microcontroller. The system
navigates a test area, collects environmental data from onboard sensors, and outputs 
mission data over a serial interface.

### Concept of Operations
Upon power-on, the system initializes all peripherals and onboard sensors. The rover 
then begins its mission by driving forward. A distance sensor mounted in the front will
provide obstacle detection, and the rover adjusts its heading when an obstacle 
is detected within a defined threshold.

At distance-based intervals determined by wheel encoder counts, the rover stops and 
deploys a soil moisture probe via servo. The rover remains stationary while the probe 
is in the lowered position. Once a valid reading is captured and the probe is returned 
to its stowed position, the rover resumes driving.

Throughout the mission, the system continuously logs Inertial Measurement Unit (IMU)
data and periodically samples temperature, computing a windowed average rather than 
streaming raw readings. All sensor data and system status are output over UART to a 
connected serial terminal.

The mission ends after a predefined number of soil samples have been collected, at which 
point the rover enters a shutdown state and ceases movement.
## Functional Requirements

## Non-Functional Requirements

## Interface Requirements

## Constraints

## Assumptions & Dependencies

## Verification & Validation