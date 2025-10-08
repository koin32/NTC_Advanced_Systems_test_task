cmake_minimum_required(VERSION 3.16)
project(MyProject VERSION 1.0)

# Устанавливаем стандарт C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Настройки для Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Если используем Qt6
find_package(Qt6 REQUIRED COMPONENTS Core)

# Добавляем поддиректории
add_subdirectory(task1)
add_subdirectory(task2)

# Устанавливаем пути для поиска библиотек
if(UNIX AND NOT APPLE)
    set(CMAKE_INSTALL_RPATH "$ORIGIN")
endif()
