# Sensor Hub – Multithreaded DDS Telemetry System

A C++17 telemetry simulation platform that models multiple sensors (Temperature, Pressure, Flow), serializes measurements using **Protobuf**, publishes data via **Cyclone DDS**, logs all activity using **spdlog (async)**, and displays a live terminal dashboard.  
The system is built with producer–consumer queues, atomic thread control flags, and end-to-end tests using **GoogleTest**.

---

## Features

- Multithreaded sensor simulation (temperature, pressure, flow)
- DDS-based publisher/subscriber messaging (**CycloneDDS-CXX**)
- Sensor data serialization with **Google Protobuf**
- Thread-safe bounded queues
- **Asynchronous rotating file logging** using **spdlog**
- Real-time terminal dashboard
- Runtime interactive shutdown controls
- Unit + integration testing using **GoogleTest**

---

## Dependencies

### Core

- **C++17**

- **CMake ≥ 3.16**

### Messaging

- **CycloneDDS**

- **CycloneDDS-CXX**
  
  - IDL generation with:
    
    - `message_schema.idl`
    
    - `Sensor_wrapper.idl`

### Serialization

- **Google Protobuf**
  
  - `sensor.proto` → `sensor.pb.cc / sensor.pb.h`

### Logging

- **spdlog**
  
  - Asynchronous logging (`async_factory`)
  
  - Rotating file sink

### Testing

- **GoogleTest (GTest)**

---

### Build Instructions

`mkdir build
cd build
cmake ..
make`

---

## Run

To be run inside build directory

#### Start Publisher

`./sensorPublisher`

#### Start Subscriber / Dashboard

`./sensorSubscriber`

---

## Architecture

```text
[Sensor Threads] --> [Safe Queues] --> [Aggregator]
                                         |
                                         v
                              [Protobuf Serialization]
                                         |
                 ------------------------------------------------
                 |                                              |
     [CycloneDDS Publisher]                                [Async Logger]
                 |
                 v
            [DDS Subscriber]
                 |
                 v
         [Live Terminal Dashboard]
```

### Data Flow Diagram
![ashboard preview](./data_flow_diagram.png) 