# cmake_minimum_required(VERSION 3.16)
# project(sensor_hub LANGUAGES CXX)

# # Some cxx config
# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

# # Add the protobuf generated source
# set(PROTO_SRCS
#     ${CMAKE_CURRENT_BINARY_DIR}/build/Serializer/sensor.pb.cc
# )

# # Checking for packages
# find_package(CycloneDDS REQUIRED)
# find_package(CycloneDDS-CXX REQUIRED)
# find_package(Protobuf REQUIRED)


# enable_testing()
# add_subdirectory(test)

# # IDL GENERATOR IMPORTED 
# include(${CycloneDDS-CXX_DIR}/idlcxx/Generate.cmake)

# # Add the protobuf generated source
# set(PROTO_SRCS
#     ${CMAKE_CURRENT_BINARY_DIR}/Serializer/sensor.pb.cc
# )


# # Genrating idl files
# idlcxx_generate(TARGET message_lib FILES message_schema.idl)
# idlcxx_generate(TARGET dds_wrap FILES Sensor_wrapper.idl)

# # Include the directory for protobuf headers
# include_directories(${CMAKE_CURRENT_BINARY_DIR}/Serializer)
# include_directories(${Protobuf_INCLUDE_DIRS})

# # Executable Definitions
# add_executable(sensorPublisher sensor_publsiher.cxx ${PROTO_SRCS})
# add_executable(sensorSubscriber monitor_subscriber.cxx ${PROTO_SRCS})

# # Linking
# target_link_libraries(sensorPublisher PRIVATE message_lib dds_wrap CycloneDDS-CXX::ddscxx ${Protobuf_LIBRARIES})
# target_link_libraries(sensorSubscriber PRIVATE message_lib dds_wrap CycloneDDS-CXX::ddscxx ${Protobuf_LIBRARIES})

cmake_minimum_required(VERSION 3.16)
project(sensor_hub LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Protobuf
find_package(Protobuf REQUIRED)

# DDS
find_package(CycloneDDS REQUIRED)
find_package(CycloneDDS-CXX REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common
    ${CMAKE_CURRENT_SOURCE_DIR}/Serializer
    ${Protobuf_INCLUDE_DIRS}
)

# Protobuf generation
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS src/Serializer/sensor.proto)

# Create a real library that builds the Protobuf sources
add_library(sensor_hub_lib STATIC ${PROTO_SRCS})

target_include_directories(sensor_hub_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Serializer
    ${Protobuf_INCLUDE_DIRS}
)

target_link_libraries(sensor_hub_lib PUBLIC ${Protobuf_LIBRARIES})

# DDS IDL generation
include(${CycloneDDS-CXX_DIR}/idlcxx/Generate.cmake)
idlcxx_generate(TARGET message_lib FILES src/common/idl/message_schema.idl)
idlcxx_generate(TARGET dds_wrap FILES src/common/idl/Sensor_wrapper.idl)

# Publisher executable
add_executable(sensorPublisher src/publisher/sensor_publsiher.cxx ${PROTO_SRCS})
target_link_libraries(sensorPublisher PRIVATE message_lib dds_wrap CycloneDDS-CXX::ddscxx sensor_hub_lib)

# Subscriber executable
add_executable(sensorSubscriber src/subscriber/monitor_subscriber.cxx ${PROTO_SRCS})
target_link_libraries(sensorSubscriber PRIVATE message_lib dds_wrap CycloneDDS-CXX::ddscxx sensor_hub_lib)

# Tests
enable_testing()
add_subdirectory(test)

