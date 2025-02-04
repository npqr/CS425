CXX = g++
CXXFLAGS = -std=c++20 -DUNIT_TEST -I$(GTEST_DIR)/googletest/include -I$(GTEST_DIR)/googlemock/include -pthread

GTEST_DIR = googletest
GTEST_LIB = $(GTEST_DIR)/build/lib/libgtest.a
GMOCK_LIB = $(GTEST_DIR)/build/lib/libgmock.a

SRCS = ../server_grp.cpp
TEST_SRCS = server_grp_test.cpp

OBJS = server_grp.o
TEST_OBJS = server_grp_test.o

TARGET = server_grp_test

all: $(TARGET)

google:
	./build_gtest.sh

server_grp.o: ../server_grp.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

server_grp_test.o: server_grp_test.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): server_grp.o server_grp_test.o
	$(CXX) $(CXXFLAGS) $(OBJS) $(TEST_OBJS) $(GTEST_LIB) $(GMOCK_LIB) -o $(TARGET)

clean:
	rm -rf ../*.o ./*.o $(TARGET)

test: $(TARGET)
	./$(TARGET)
