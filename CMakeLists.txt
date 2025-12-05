cmake_minimum_required(VERSION 3.16)
project(sensor_hub LANGUAGES CXX)

# Some cxx config
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the protobuf generated source
set(PROTO_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/build/Serializer/sensor.pb.cc
)

# Checking for packages
find_package(CycloneDDS REQUIRED)
find_package(CycloneDDS-CXX REQUIRED)
find_package(Protobuf REQUIRED)

# IDL GENERATOR IMPORTED 
include(${CycloneDDS-CXX_DIR}/idlcxx/Generate.cmake)

# Add the protobuf generated source
set(PROTO_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/Serializer/sensor.pb.cc
)


# Genrating idl files
idlcxx_generate(TARGET message_lib FILES message_schema.idl)
idlcxx_generate(TARGET dds_wrap FILES Sensor_wrapper.idl)

# Include the directory for protobuf headers
include_directories(${CMAKE_CURRENT_BINARY_DIR}/Serializer)
include_directories(${Protobuf_INCLUDE_DIRS})

# Executable Definitions
add_executable(sensorPublisher sensor_publsiher.cxx ${PROTO_SRCS})
add_executable(sensorSubscriber monitor_subscriber.cxx ${PROTO_SRCS})

# Linking
target_link_libraries(sensorPublisher PRIVATE message_lib dds_wrap CycloneDDS-CXX::ddscxx ${Protobuf_LIBRARIES})
target_link_libraries(sensorSubscriber PRIVATE message_lib dds_wrap CycloneDDS-CXX::ddscxx ${Protobuf_LIBRARIES})

