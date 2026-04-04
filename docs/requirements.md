# Requirements Document Version <1.0>

## Introduction & Purpose
This document defines the requirements for an autonomous mobile rover platform, the AutoRover
The vehicle will navigate a defined test area, collect environmental data via onboard sensors,
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
### System Initialization & Control
**FR_001** — The system shall enter an idle state upon power-on, during which all peripherals (GPIO, timers, I2C, UART, ADC, PWM) and onboard sensors are initialized.

**FR_002** - While in idle state, the system shall report sensor health and status over UART, allowing the operator to verify system readiness before mission start.

**FR_003** — The system shall begin the mission upon receiving a button press while in idle state.

**FR_004** — The system shall abort the mission and return to idle state upon receiving a button press while in mission mode.

**FR_005** — The system shall terminate the mission and enter a low-power sleep state after successfully collecting N soil moisture samples. N will be a configurable parameter.
### Motor Control & Navigation
**FR_006** — The system shall drive the rover forward at a single speed during exploration mode.

**FR_007** — The system shall track distance traveled using wheel encoder pulse counts.

**FR_008** — The system shall continuously monitor the forward-facing distance sensor for obstacles during driving mode.

**FR_009** — The system shall begin a heading adjustment when an obstacle is detected within a defined distance threshold.
### Soil Probe Deployment & Adaptive Sampling
**FR_010** — The system shall determine soil sampling locations using an adaptive gradient-chasing strategy: if the delta between the current and previous soil moisture reading exceeds a defined threshold, the next sample shall be taken at a shorter travel interval; if the delta is below the threshold, the next sample shall be taken at a longer travel interval.

**FR_011** — When a sampling location is reached, the system shall bring the rover to a complete stop before deploying the soil moisture probe.

**FR_012** — The system shall lower the soil moisture probe to the deployed position via servo actuation.

**FR_013** — The system shall hold the rover stationary while the probe is in the deployed position and wait for a valid soil moisture reading from the ADC.

**FR_014** — Upon capturing a valid soil moisture reading, the system shall return the probe to the stowed position via servo actuation.

**FR_015** — The system shall not resume driving until the probe has been confirmed in the stowed position.

**FR_016** — The system shall resume driving mode after the probe is stowed and the reading has been logged.
### Sensor Data Collection
**FR_017** — The system shall sample the IMU (accelerometer and gyroscope) at the highest practical rate supported by the main loop cycle.

**FR_018** — The system shall continuously sample the temperature sensor, but shall only flag a new temperature value for output when the reading changes by ±1 degree or more from the last reported value.

**FR_019** — The system shall read the soil moisture sensor via ADC only when the probe is in the deployed position.
### Data Output (UART)
**FR_020** — The system shall transmit formatted sensor data over UART to a connected serial terminal.

**FR_021** — All UART output shall include a timestamp derived from the system tick counter (milliseconds since boot).

**FR_022** — IMU data shall be output at an easily readable rate, formatted as a snapshot of the most recent accelerometer and gyroscope values.

**FR_023** — Temperature data shall be output only upon a change event, as defined in FR_018.

**FR_024** — Each soil moisture reading shall be output immediately upon capture, including the sample number, moisture value, and timestamp.

**FR_025** — System events (mission start, mission end, faults, state transitions) shall be logged over UART with timestamps.
### LED Status Indication
**FR_026** — The system shall drive a green LED to indicate exploration mode (rover is driving and navigating).

**FR_027** — The system shall drive an amber/yellow LED to indicate soil sampling mode (rover is stationary, probe is deployed).

**FR_028** — The system shall drive a red LED to indicate a fault condition or low battery state.

**FR_029** — Only one LED state shall be active at a time, reflecting the current system state.
### Fault Handling
**FR_030** — If a non-critical sensor (temperature/humidity) fails to return a valid reading, the system shall log an error over UART and continue the mission without that sensor's data.

**FR_031** — If a critical component (servo) fails to actuate, the system shall halt the rover, retry the operation once, and if the retry fails, transition to idle state and output an alert over UART.

**FR_032** — The system shall monitor battery voltage via ADC through a voltage divider circuit.

**FR_033** — If the battery voltage falls below a defined critical threshold, the system shall log a low-battery warning over UART, activate the red LED, and initiate a graceful shutdown sequence into sleep mode.
## Non-Functional Requirements
### Architecture
**NFR_001** — The main loop shall be entirely non-blocking. No busy-wait delays (e.g., `HAL_Delay`) shall be used during mission execution.

**NFR_002** — The system shall be implemented as a finite state machine with clearly defined states, including: idle, driving, obstacle avoidance, soil sampling, fault, and sleep.

**NFR_003** — Firmware shall maintain clear separation between hardware abstraction and application logic, such that application code does not directly access peripheral registers.
### Performance
**NFR_004** — The obstacle detection and avoidance response shall be fast enough to initiate a heading adjustment before the rover reaches the obstacle.

**NFR_005** — UART output shall be transmitted at a baud rate sufficient to support the combined data throughput of IMU snapshots, event-driven temperature updates, and soil moisture readings without data loss.
### Reliability
**NFR_006** — All physical button inputs shall be debounced in software to prevent false triggers from contact bounce.

**NFR_007** — IMU data shall be filtered to reduce sensor noise before being reported over UART.

**NFR_008** — A sensor shall not be declared faulted on a single failed read, but rather a configurable number of consecutive failed reads before triggering fault handling logic defined in FR_030 or FR_031.
### Usability
**NFR_009** — UART output shall be formatted in structured, human-readable text to ensure it's easily digestible for the user.

**NFR_010** — LED states shall be mutually exclusive and shall transition without delay upon state change, providing immediate visual feedback of system state.
### Testability
**NFR_011** — Subsystem functionality (state machine transitions, UART output formatting, sampling logic) shall be verifiable via UART diagnostic output without requiring the rover to be physically driving.
## Interface Requirements

## Constraints

## Assumptions & Dependencies

## Verification & Validation