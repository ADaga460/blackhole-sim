cmake_minimum_required(VERSION 3.15)
project(blackhole-sim VERSION 0.1.0 LANGUAGES CXX)

# Options
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(ENABLE_SANITIZERS "Enable Address/UndefinedBehavior sanitizers (gcc/clang)" OFF)

# Compiler settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Source discovery: put sources in src/ and headers in include/
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
         "${CMAKE_SOURCE_DIR}/src/*.cpp"
         "${CMAKE_SOURCE_DIR}/src/*.c")

if(PROJECT_SOURCES)
    add_executable(blackhole-sim ${PROJECT_SOURCES})
else()
    # Placeholder target if no sources yet
    add_executable(blackhole-sim WIN32 EXCLUDE_FROM_ALL "")
    message(WARNING "No sources found in src/. Add sources to src/ or update CMakeLists.txt")
endif()

target_include_directories(blackhole-sim PRIVATE "${CMAKE_SOURCE_DIR}/include")

# Warnings
if(MSVC)
    target_compile_options(blackhole-sim PRIVATE /W4 /permissive-)
else()
    target_compile_options(blackhole-sim PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Optional sanitizers
if(ENABLE_SANITIZERS AND (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang"))
    target_compile_options(blackhole-sim PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(blackhole-sim PRIVATE -fsanitize=address,undefined)
endif()

# Threading
find_package(Threads REQUIRED)
target_link_libraries(blackhole-sim PRIVATE Threads::Threads)

# Install
install(TARGETS blackhole-sim RUNTIME DESTINATION bin)

# Testing (expects a test/ subdirectory with its own CMakeLists.txt)
include(CTest)
if(BUILD_TESTING)
    enable_testing()
    if(EXISTS "${CMAKE_SOURCE_DIR}/test/CMakeLists.txt")
        add_subdirectory(test)
    endif()
endif()