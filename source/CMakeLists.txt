cmake_minimum_required(VERSION 3.14)

project(NQueensViz LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Core Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Core Gui)

# 添加源文件
set(PROJECT_SOURCES
        src/main.cpp
        src/core/NQueensSolver.cpp
        src/core/NQueensSolver.h
        src/ui/ChessboardWidget.cpp
        src/ui/ChessboardWidget.h
        src/ui/MainWindow.cpp
        src/ui/MainWindow.h
        src/common/Config.h
        src/common/Types.h
        src/common/Types.h
        src/common/Config.h
        src/core/NQueensSolver.h
        src/core/NQueensSolver.cpp
        src/ui/ChessboardWidget.cpp
        src/ui/ChessboardWidget.h
        src/ui/MainWindow.cpp
        src/ui/MainWindow.h
        src/main.cpp
)

add_executable(NQueensViz ${PROJECT_SOURCES}
        src/common/Types.h
        src/common/Types.h)

target_link_libraries(NQueensViz PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# 包含路径设置
target_include_directories(NQueensViz PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)