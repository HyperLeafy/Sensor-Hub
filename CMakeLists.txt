cmake_minimum_required(VERSION 3.16)
project(sensor_hub LANGUAGES CXX)

# Some cxx config
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Checking for packages
find_package(CycloneDDS REQUIRED)
find_package(CycloneDDS-CXX REQUIRED)

# IDL GENERATOR IMPORTED 
include(${CycloneDDS-CXX_DIR}/idlcxx/Generate.cmake)

# Genrating idl files
idlcxx_generate(TARGET message_lib FILES message_schema.idl)

# Executable Definations
# 
add_executable(publishApp ddsPublisher.cxx)
add_executable(sensorPublisher sensor_publsiher.cxx)

# 
add_executable(SubApp ddsSubscriber.cxx)
add_executable(sensorSubscriber monitor_subscriber.cxx)
# add_executable(SensorApp sensor_publsiher.cxx)

# Linking libraries to those executable
target_link_libraries(publishApp PRIVATE message_lib CycloneDDS-CXX::ddscxx)
target_link_libraries(sensorPublisher PRIVATE message_lib CycloneDDS-CXX::ddscxx)

target_link_libraries(sensorSubscriber PRIVATE message_lib CycloneDDS-CXX::ddscxx)
target_link_libraries(SubApp PRIVATE message_lib CycloneDDS-CXX::ddscxx)
# target_link_libraries(SensorApp PRIVATE message_lib CycloneDDS-CXX::ddscxx)
