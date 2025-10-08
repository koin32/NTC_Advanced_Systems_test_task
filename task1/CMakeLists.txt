cmake_minimum_required(VERSION 3.16)

# Находим ZeroMQ
find_package(PkgConfig REQUIRED)
pkg_check_modules(ZMQ REQUIRED libzmq)

# Находим Qt6
find_package(Qt6 REQUIRED COMPONENTS Core)

# Клиентская часть
add_executable(client_task1
    client/main.cpp
    client/Student.cpp
    client/ZmqClient.cpp
)

target_include_directories(client_task1 PRIVATE
    client
    ${ZMQ_INCLUDE_DIRS}
)

target_link_libraries(client_task1
    Qt6::Core
    ${ZMQ_LIBRARIES}
)

# Серверная часть
add_executable(server_task1
    server/main.cpp
    server/Student.cpp
    server/StudentManager.cpp
    server/StudentParser.cpp
    server/ZmqServer.cpp
)

target_include_directories(server_task1 PRIVATE
    server
    ${ZMQ_INCLUDE_DIRS}
)

target_link_libraries(server_task1
    Qt6::Core
    ${ZMQ_LIBRARIES}
)

# Копируем файлы с данными студентов
file(COPY student_file_1.txt student_file_2.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
