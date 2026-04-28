cmake_minimum_required(VERSION 3.16)
project(qt_jsengine_example LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# 同时支持 Qt5 和 Qt6
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Qml)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Qml)

add_executable(qt_jsengine_example
    main.cpp
)

target_link_libraries(qt_jsengine_example
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Qml
)

# 把 scripts/ 拷到可执行文件旁边，运行时 main.cpp 会从那里加载
add_custom_command(TARGET qt_jsengine_example POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/scripts
        $<TARGET_FILE_DIR:qt_jsengine_example>/scripts
)
