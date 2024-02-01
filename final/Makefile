# Compiler and linker flags
CXX = g++
CXXFLAGS = -Wall -std=c++11
LDFLAGS = -lmodbus -ljsoncpp -lpaho-mqtt3a -lmariadbz

# Source files and target executable
SRCS = main.cpp Server.cpp Client.cpp MQTT.cpp 
TARGET = my_program

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(SRCS) $(CXXFLAGS) $(LDFLAGS) -o $(TARGET) -g -Wno-unused-variable -Wno-unused-but-set-variable

clean:
	rm -f $(TARGET)
