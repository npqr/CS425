# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++20 -DUNIT_TEST -I$(GTEST_DIR)/googletest/include -I$(GTEST_DIR)/googlemock/include -pthread

# Directories
SRC_DIR = ..
TEST_DIR = .
BUILD_DIR = ./build

# Google Test
GTEST_DIR = $(BUILD_DIR)/googletest
GTEST_LIB = $(GTEST_DIR)/build/lib/libgtest.a
GMOCK_LIB = $(GTEST_DIR)/build/lib/libgmock.a

# Source files
SRCS = $(SRC_DIR)/server_grp.cpp
TEST_SRCS = $(TEST_DIR)/server_grp_test.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)
TEST_OBJS = $(TEST_SRCS:.cpp=.o)

# Executable
TARGET = $(BUILD_DIR)/server_grp_test

# Default target
all: $(TARGET)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build Google Test and Google Mock
$(GTEST_LIB) $(GMOCK_LIB): | $(BUILD_DIR)
	git clone https://github.com/google/googletest.git $(GTEST_DIR)
	cd $(GTEST_DIR) && mkdir -p build && cd build && cmake .. && make

# Build object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# $(BUILD_DIR)/%.o: $(TEST_DIR)/%.cpp | $(BUILD_DIR)
# 	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build the test executable
$(TARGET): $(OBJS) $(TEST_OBJS) $(GTEST_LIB) $(GMOCK_LIB)
	$(CXX) $(CXXFLAGS) $(OBJS) $(TEST_OBJS) $(GTEST_LIB) $(GMOCK_LIB) -o $(TARGET)

# Clean up
clean:
	rm -rf $(BUILD_DIR)/*.o $(TARGET)

# Run tests
test: $(TARGET)
	./$(TARGET)