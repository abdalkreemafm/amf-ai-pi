cmake_minimum_required(VERSION 3.16)
project(hadeed_host_checks C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(hadeed_protocol STATIC protocol/hadeed_protocol.c)
target_include_directories(hadeed_protocol PUBLIC protocol)
target_compile_options(hadeed_protocol PRIVATE -Wall -Wextra -Werror -pedantic)

add_executable(test_protocol_c tests/test_protocol_c.c)
target_link_libraries(test_protocol_c PRIVATE hadeed_protocol)
target_compile_options(test_protocol_c PRIVATE -Wall -Wextra -Werror -pedantic)

enable_testing()
add_test(NAME protocol_c COMMAND test_protocol_c)
