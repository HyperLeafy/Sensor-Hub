# CMake generated Testfile for 
# Source directory: /home/blank/Projects/Internship/sentienc/onboarding-task/sensor-hub-test/test
# Build directory: /home/blank/Projects/Internship/sentienc/onboarding-task/sensor-hub-test/build/test
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(SerializationTest "/home/blank/Projects/Internship/sentienc/onboarding-task/sensor-hub-test/build/test/core_tests")
set_tests_properties(SerializationTest PROPERTIES  _BACKTRACE_TRIPLES "/home/blank/Projects/Internship/sentienc/onboarding-task/sensor-hub-test/test/CMakeLists.txt;23;add_test;/home/blank/Projects/Internship/sentienc/onboarding-task/sensor-hub-test/test/CMakeLists.txt;0;")
add_test(QueueTest "/home/blank/Projects/Internship/sentienc/onboarding-task/sensor-hub-test/build/test/queue_tests")
set_tests_properties(QueueTest PROPERTIES  _BACKTRACE_TRIPLES "/home/blank/Projects/Internship/sentienc/onboarding-task/sensor-hub-test/test/CMakeLists.txt;30;add_test;/home/blank/Projects/Internship/sentienc/onboarding-task/sensor-hub-test/test/CMakeLists.txt;0;")
add_test(EndToEndTest "/home/blank/Projects/Internship/sentienc/onboarding-task/sensor-hub-test/build/test/e2e_tests")
set_tests_properties(EndToEndTest PROPERTIES  _BACKTRACE_TRIPLES "/home/blank/Projects/Internship/sentienc/onboarding-task/sensor-hub-test/test/CMakeLists.txt;37;add_test;/home/blank/Projects/Internship/sentienc/onboarding-task/sensor-hub-test/test/CMakeLists.txt;0;")
