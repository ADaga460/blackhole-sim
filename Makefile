# Compiler and Flags
CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -static
DEBUG    := -g

# Include Directories
INCLUDES := -I"C:\Users\aarav\Downloads\glfw-3.4.bin.WIN64\glfw-3.4.bin.WIN64\include" \
            -I"C:\Users\aarav\Downloads\glew-2.3.0-win32\glew-2.3.0\include" \
            -I"src"

# Library Directories
LDFLAGS  := -L"C:\Users\aarav\Downloads\glfw-3.4.bin.WIN64\glfw-3.4.bin.WIN64\lib-mingw-w64" \
            -L"C:\Users\aarav\Downloads\glew-2.3.0-win32\glew-2.3.0\lib\Release\x64"

# Libraries to Link
LIBS     := -lglfw3 -lglew32s -lopengl32 -lgdi32

# Project Structure
SRC_DIR  := src
BUILD_DIR := build
BIN_DIR  := bin
TARGET   := $(BIN_DIR)/blackhole_sim.exe

# Find all .cpp files
SRCS     := $(SRC_DIR)/main.cpp \
            $(SRC_DIR)/renderer/Renderer2D.cpp \
            $(SRC_DIR)/sim/BlackHoleSim.cpp

# Object files mapping
OBJS     := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Helper to fix paths for Windows mkdir
FIX_PATH = $(subst /,\,$1)

all: $(TARGET)

# Link
$(TARGET): $(OBJS)
	@if not exist $(BIN_DIR) mkdir $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS) $(LIBS)
	@echo Build Complete: $(TARGET)

# Compile
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@if not exist $(call FIX_PATH,$(dir $@)) mkdir $(call FIX_PATH,$(dir $@))
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean
clean:
	@if exist $(BUILD_DIR) rd /s /q $(BUILD_DIR)
	@if exist $(BIN_DIR) rd /s /q $(BIN_DIR)

.PHONY: all clean