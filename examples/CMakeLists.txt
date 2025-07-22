project(redisClient_examples)

add_executable(example_publisher example_publisher.cpp)
add_executable(example_subscriber example_subscriber.cpp)

target_link_libraries(example_publisher AVS::REDIS_CLIENT)
target_link_libraries(example_subscriber AVS::REDIS_CLIENT)

set_target_properties(
    example_publisher
    example_subscriber
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)