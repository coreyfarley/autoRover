# System Requirements Specification

## Introduction & Purpose
This document defines the requirements for an autonomous mobile rover platform, the AutoRover.
The vehicle will navigate a defined test area, collect environmental data via onboard sensors,
and make operational decisions based on system state. This document establishes the functional and
non-functional requirements that the system must satisfy, and serves as the basis for design, implementation, 
and verification.
## Scope
This document covers the firmware system for AutoRover, including low-level peripheral drivers, autonomous navigation and decision logic, onboard sensor integration, onboard SD card data logging, and UART-based serial output for diagnostics and bench testing. It also covers the wiring-level schematic defining connections between the MCU and all hardware components.

The following areas fall outside the scope of this project: mechanical design and chassis fabrication, custom PCB design, wireless communication, off-board or host-side software, cloud connectivity / IoT platform integration.

## System Overview
AutoRover is an autonomous mobile platform built on an STM32 microcontroller. The system
navigates a test area, collects environmental data from onboard sensors, and records
mission data to an onboard SD card.

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
data and continuously samples the temperature and humidity sensors, outputting new
readings only when values change beyond defined thresholds. All sensor data and system
events are recorded to an onboard SD card log file during mission execution. UART output
is available as a diagnostic interface during bench testing and development.

The mission ends after a predefined number of soil samples have been collected, at which 
point the rover enters a shutdown state and ceases movement.
## Functional Requirements
### System Initialization & Control
**FR_001** - The system shall enter an idle state upon power-on, during which all peripherals (GPIO, timers, I2C, SPI, UART, ADC, PWM) and onboard sensors are initialized.

**FR_002** - While in idle state, the system shall report sensor status and SD card status over UART, allowing the operator to verify system readiness before mission start.

**FR_003** - The system shall begin the mission upon receiving a button press while in idle state.

**FR_004** - The system shall abort the mission and return to idle state upon receiving a button press while in mission mode. If the soil moisture probe is in the deployed position at the time of abort, the system shall retract the probe to the stowed position before completing the transition to idle state.

**FR_005** - The system shall terminate the mission and enter a low-power sleep state after successfully collecting N soil moisture samples. N shall be a configurable parameter.
### Motor Control & Navigation
**FR_006** - The system shall drive the rover forward at a constant cruise speed during driving mode. The system shall control motor outputs independently to implement differential steering for heading adjustments.

**FR_007** - The system shall track distance traveled using wheel encoder pulse counts.

**FR_008** - The system shall continuously monitor the forward-facing distance sensor for obstacles during driving mode.

**FR_009** - When an obstacle is detected within the defined threshold distance during driving mode, the system shall bring the rover to a complete stop and enter obstacle avoidance mode. The system shall pivot in place, alternating turn direction on successive avoidance events, while continuously polling the distance sensor. The rover shall continue rotating — past any additional obstacles encountered during the sweep — until the distance sensor confirms the forward path is clear. If no clear path is found within a full 360° rotation, the system shall halt the rover, transition to fault state, and output a fault event over UART. Upon returning to driving mode, the system shall apply proportional heading correction using IMU yaw data to steer the rover back toward the stored mission heading.
### Soil Probe Deployment & Adaptive Sampling
**FR_010** - The system shall adjust the travel distance between sampling stops based on the moisture delta (|ΔM|) between the two most recent readings.

- The first sampling interval shall use an initial configurable value. The travel interval shall be bounded by configurable minimum and maximum values at all times.

- If |ΔM| exceeds a configurable threshold, the system shall decrease the travel interval for the subsequent sample.
    
- If |ΔM| is below the threshold, the system shall increase the travel interval for the subsequent sample.  

**FR_011** - When a sampling location is reached, the system shall bring the rover to a complete stop before deploying the soil moisture probe.

**FR_012** - The system shall lower the soil moisture probe to the deployed position via servo actuation.

**FR_013** - The system shall hold the rover stationary while the probe is in the deployed position and wait for a valid soil moisture reading from the ADC.

    A reading shall be considered valid if the raw ADC value falls within a configurable range [MOISTURE_MIN, MOISTURE_MAX], distinguishing soil contact from an open-air or disconnected probe.

**FR_014** - Upon capturing a valid soil moisture reading, the system shall return the probe to the stowed position via servo actuation.

**FR_015** - The system shall track servo travel time using a timer and shall not block the main loop during this interval.

**FR_016** - The system shall resume driving mode after the probe is stowed and the reading has been logged.
### Sensor Data Collection
**FR_017** - The system shall sample the IMU at a target rate of 20 Hz.

**FR_018** - The system shall continuously sample the temperature/humidity sensor. A new temperature value shall be flagged for output only when the reading changes by ±1°C or more from the last reported value. A new humidity value shall be flagged for output only when the reading changes by a configurable threshold (in %RH) from the last reported value.

**FR_019** - The system shall read the soil moisture sensor via ADC only when the probe is in the deployed position.
### Data Output
**FR_020** - The system shall produce formatted sensor data and system event output as defined in FR_021 through FR_025.

**FR_021** - All data output shall include a timestamp derived from the system tick counter.

**FR_022** - IMU data shall be output at a rate of 5 Hz, formatted as a snapshot of the most recent filtered accelerometer and gyroscope values.

**FR_023** - Temperature and humidity data shall each be output only upon their respective change events, as defined in FR_018.

**FR_024** - Each soil moisture reading shall be output immediately upon capture, including the sample number and moisture value.

**FR_025** - System events (mission start, mission end, faults, state transitions) shall be logged.

**FR_026** - During mission execution, all formatted output defined in FR_021 through FR_025 shall be written to the active SD card log file, and shall simultaneously be transmitted over UART.

**FR_027** - The system shall initialize the SD card upon every entry into idle state. The result shall be reported over UART as part of the readiness check defined in FR_002. If the SD card is not present or not writable, the system shall reject mission start and report the cause over UART.

**FR_028** - If a write to the SD card fails during mission execution, the system shall retry the write up to a configurable number of consecutive attempts. If all retry attempts fail, the system shall log the failure over UART, transition to fault state, and halt the rover.

**FR_029** - If the SD card reaches capacity during mission execution, the system shall handle it as a write failure per FR_028.
### LED Status Indication
**FR_030** - The system shall blink the green LED at a slow, visible rate to indicate idle state (powered on, awaiting mission start).

**FR_031** - The system shall drive a green LED to indicate driving mode, including during obstacle avoidance.

**FR_032** - The system shall drive an amber/yellow LED to indicate soil sampling mode (rover is stationary, probe is deployed).

**FR_033** - The system shall drive a red LED to indicate a fault condition or low battery state.

**FR_034** - All LEDs shall be off during sleep state.

**FR_035** - Only one LED state shall be active at a time, reflecting the current system state.
### Fault Handling
**FR_036** - If a non-critical sensor (temperature/humidity) fails to return a valid reading, the system shall log an error and continue the mission without that sensor's data.

**FR_037** - If the probe deployment sequence fails to produce a valid soil moisture reading (as defined in FR_013) within a configurable deployment timeout, the system shall retract the probe, wait for probe stow travel time, and retry the deployment once. If the retry also fails to produce a valid reading within the timeout, the system shall halt the rover, transition to fault state, and output a fault event over UART. The system shall remain in fault state with the red LED active until the operator clears the fault via a button press, at which point the system shall return to idle state.

**FR_038** - The system shall monitor battery voltage via ADC through a voltage divider circuit.

**FR_039** - If the battery voltage falls below a defined critical threshold, the system shall log a low-battery warning, activate the red LED, and hold in a low-battery warning state for a minimum configurable duration before initiating a shutdown sequence into sleep mode. All LEDs shall be turned off upon entering sleep state per FR_034.
## Non-Functional Requirements
### Architecture
**NFR_001** - The main loop shall be entirely non-blocking. No busy-wait delays (e.g., `HAL_Delay`) shall be used during mission execution.

**NFR_002** - The system shall be implemented as a finite state machine with clearly defined states, including: idle, driving, obstacle avoidance, soil sampling, fault, and sleep.

**NFR_003** - Firmware shall maintain clear separation between hardware abstraction and application logic, such that application code does not 
directly access peripheral registers.

**NFR_004** - The system shall store a mission heading reference as an IMU yaw value at mission start. During driving mode, the heading error (current yaw minus mission heading) shall be computed continuously and used to apply proportional differential motor correction.
### Performance
**NFR_005** - The distance sensor shall be polled at a minimum rate of 10 Hz during driving mode. The configured obstacle detection threshold distance shall be set to exceed the sum of the rover's braking distance and the maximum distance traveled per polling interval at cruise speed, ensuring the rover can come to a complete stop before reaching a detected obstacle.

**NFR_006** - UART output shall be transmitted at a minimum 115200 baud rate to support the combined data throughput of IMU snapshots, event-driven temperature updates, and soil moisture readings without data loss.
### Reliability
**NFR_007** - All physical button inputs shall be debounced in software to prevent false triggers from contact bounce.

**NFR_008** - IMU data shall be filtered to reduce sensor noise before being used in data output.

**NFR_009** - A sensor shall not be declared faulted on a single failed read, but rather a configurable number of consecutive failed reads before triggering fault handling logic defined in FR_036 or FR_037.
### Usability
**NFR_010** - UART output shall be formatted in structured, human-readable text to ensure it's easily digestible for the user.

**NFR_011** - LED states shall be mutually exclusive and shall transition without delay upon state change, providing immediate visual feedback of system state.
### Testability
**NFR_012** - Subsystem functionality (state machine transitions, UART output formatting, sampling logic) shall be verifiable via UART diagnostic output without requiring the rover to be physically driving.
## Interface Requirements
### Hardware Interfaces
 
**IF_001** - The IMU shall communicate with the MCU over I2C.
 
**IF_002** - The temperature/humidity sensor shall communicate with the MCU over I2C.
 
**IF_003** - The soil moisture sensor shall provide an analog voltage to the MCU via an ADC input channel.
 
**IF_004** - The distance sensor shall interface with the MCU via GPIO (trigger/echo) or ADC input. *(Interface type TBD based on sensor selection.)*
 
**IF_005** - The servo shall be driven by a PWM output from the MCU.
 
**IF_006** - The drive motors shall be controlled via PWM outputs from the MCU through a motor driver module.
 
**IF_007** - The wheel encoders shall provide pulse outputs to the MCU via GPIO input channels configured for edge detection.
 
**IF_008** - The mission start/stop button shall be connected to a GPIO input with an internal pull-up or pull-down resistor.
 
**IF_009** - The battery voltage shall be read by the MCU via an ADC input channel through a resistive voltage divider circuit.

**IF_010** - The SD card module shall interface with the MCU via SPI.
### Log Format
 
**IF_011** - The system shall output data as tagged, human-readable lines. Each line shall begin with a category tag enclosed in brackets, followed by a timestamp and relevant data fields in key-value format.
 
**IF_012** - The following category tags shall be used:
- `[IMU]` - Accelerometer and gyroscope snapshot
- `[TEMP]` - Temperature change event
- `[HUM]` - Humidity change event
- `[SOIL]` - Soil moisture sample reading
- `[SYS]` - System events (state transitions, mission start/end, faults, low battery)
 
Example output:
```
[IMU]  t=12340  ax=0.12  ay=-0.03  az=9.81  gx=1.2  gy=0.4  gz=-0.1
[TEMP] t=12340  temp=76
[HUM]  t=12340  hum=54
[SOIL] t=45200  sample=3  moisture=482
[SYS]  t=45200  event=SAMPLING_COMPLETE
```

### Operator Interface
 
**IF_013** - The system shall provide visual status indication to the operator via three onboard LEDs (green, amber, red) driven by GPIO outputs, as defined in FR_030 through FR_035.
## Constraints
**CON_001** - The system shall be built on a single STM32 microcontroller. All firmware must operate within the GPIO, ADC, timer, I2C, SPI, UART, and memory resources available on the selected MCU.
 
**CON_002** - All hardware components (sensors, actuators, motor drivers, chassis) shall be commercially available off-the-shelf modules. No custom PCB fabrication or custom mechanical fabrication is in scope.
 
**CON_003** - Firmware shall be written in C using the STM32 HAL library, developed in STM32CubeIDE and VS Code, and compiled with GCC.
 
**CON_004** - The system shall operate from onboard battery power with no tethered connection during mission execution. Battery capacity shall support a minimum mission runtime of 20 minutes under nominal operating conditions. Total current draw from motors, servo, sensors, and MCU must be accounted for during component selection.
 
**CON_005** - All components and wiring must fit on a single rover chassis. Chassis size shall be determined after component selection to ensure adequate mounting space.
 
**CON_006** - The rover shall operate on natural outdoor terrain including soil, dirt, and grass. Wheel selection, ground clearance, and traction must be suitable for unpaved, potentially uneven surfaces.
## Assumptions & Dependencies
### Assumptions
 
**ASMP_001** - All selected sensors and modules shall be compatible with 3.3V logic levels, either natively or through onboard level shifting provided by the breakout board. Components that output 5V logic will require external level shifting or voltage division to avoid damage to the STM32 GPIO pins.
 
**ASMP_002** - All I2C devices connected to the same bus shall have unique default addresses, or shall support configurable addressing to avoid bus conflicts. This shall be verified during component selection.
 
**ASMP_003** - The selected STM32 variant shall provide sufficient peripheral resources for the complete system, including: a minimum of two ADC input channels (soil moisture, battery voltage), sufficient PWM-capable timer channels for servo and dual motor control, at least one I2C bus, at least one UART interface, at least one SPI bus for the SD card module, and adequate GPIO for encoders, button input, LEDs, and distance sensor.
 
**ASMP_004** - The servo mechanism shall operate in an open-loop configuration. The selected servo shall provide sufficient torque to deploy and retract the soil moisture probe mechanism. Probe position (stowed/deployed) shall be inferred from commanded PWM duty cycles rather than physical position feedback.
 
**ASMP_005** - The operating terrain shall be approximately level and free of conditions that could cause the rover to tip, become stuck, or prevent the soil probe from making contact with the ground surface.
 
**ASMP_006** - The onboard battery shall deliver stable voltage output throughout the mission duration. Voltage drop-off characteristics shall be gradual enough for the ADC-based battery monitor (FR_038) to detect the low-battery threshold before the system browns out.
 
**ASMP_007** - The motor driver module shall accept PWM input and provide a simple directional interface (e.g., PWM + direction pin or dual PWM per motor) without requiring additional communication protocols.
 
**ASMP_008** - The SD card shall be formatted as FAT32. Log files shall be written as plain text and shall be directly readable on a host computer without additional tooling.

### Dependencies
 
**DEP_001** - The firmware depends on the STM32 HAL library provided by STMicroelectronics. A specific HAL version shall be selected at project start and held constant throughout development to avoid API-breaking changes.
 
**DEP_002** - The project depends on the continued commercial availability of all selected sensors and modules. If a specific component becomes unavailable, a substitute with a compatible interface may require a new driver implementation. The architectural separation defined in NFR_003 is intended to minimize the impact of such substitutions.
 
**DEP_003** - Driver development for each sensor depends on the availability of adequate technical documentation (datasheets, register maps, communication protocols) from the component manufacturer or vendor.

**DEP_004** - The firmware depends on the FatFs middleware library for SD card file system access. A specific version shall be selected at project start and held constant throughout development.
## Verification & Validation