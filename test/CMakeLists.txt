cmake_minimum_required(VERSION 3.16)

enable_testing()
find_package(GTest REQUIRED)
find_package(Protobuf REQUIRED)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}/../Serializer   # For sensor.pb.h
    ${CMAKE_CURRENT_SOURCE_DIR}/..             # For project headers (safe_queue, message_schema)
)


# # Protobuf sources
# set(PROTO_SRCS
#     ${CMAKE_CURRENT_BINARY_DIR}/../Serializer/sensor.pb.cc
# )

# -------------------------------
# Serialization test
# -------------------------------
add_executable(core_tests test_serialization.cxx)
target_link_libraries(core_tests PRIVATE GTest::gtest GTest::gtest_main sensor_hub_lib)
add_test(NAME SerializationTest COMMAND core_tests)

# -------------------------------
# Queue test
# -------------------------------
add_executable(queue_tests test_safeQueue.cxx)
target_link_libraries(queue_tests PRIVATE GTest::gtest GTest::gtest_main)
add_test(NAME QueueTest COMMAND queue_tests)

# -------------------------------
# End-to-End test
# -------------------------------
add_executable(e2e_tests test_E2E.cxx)
target_link_libraries(e2e_tests PRIVATE GTest::gtest GTest::gtest_main sensor_hub_lib)
add_test(NAME EndToEndTest COMMAND e2e_tests)
