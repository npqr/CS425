CXX = g++
CXXFLAGS = -std=c++20 -DUNIT_TEST -I$(GTEST_DIR)/googletest/include -I$(GTEST_DIR)/googlemock/include -pthread

SRC_DIR = ..
TEST_DIR = .
BUILD_DIR = ./build

GTEST_DIR = $(BUILD_DIR)/googletest
GTEST_LIB = $(GTEST_DIR)/build/lib/libgtest.a
GMOCK_LIB = $(GTEST_DIR)/build/lib/libgmock.a

SRCS = $(SRC_DIR)/server_grp.cpp
TEST_SRCS = $(TEST_DIR)/server_grp_test.cpp

OBJS = $(SRCS:.cpp=.o)
TEST_OBJS = $(TEST_SRCS:.cpp=.o)

TARGET = $(BUILD_DIR)/server_grp_test

all: $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(GTEST_LIB) $(GMOCK_LIB): | $(BUILD_DIR)
	git clone https://github.com/google/googletest.git $(GTEST_DIR)
	cd $(GTEST_DIR) && mkdir -p build && cd build && cmake .. && make

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(OBJS) $(TEST_OBJS) $(GTEST_LIB) $(GMOCK_LIB)
	$(CXX) $(CXXFLAGS) $(OBJS) $(TEST_OBJS) $(GTEST_LIB) $(GMOCK_LIB) -o $(TARGET)

clean:
	rm -rf $(TEST_DIR)/*.o $(SRC_DIR)/*.o $(BUILD_DIR)/*.o $(TARGET)

test: $(TARGET)
	./$(TARGET)
