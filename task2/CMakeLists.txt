cmake_minimum_required(VERSION 3.16)

find_package(Qt6 REQUIRED COMPONENTS Core Network)

add_executable(task2
    main.cpp
    CoordinateParser.cpp
    CoordinateService.cpp
    HttpServer.cpp
)

target_include_directories(task2 PRIVATE
    .
)

target_link_libraries(task2
    Qt6::Core
    Qt6::Network
)
