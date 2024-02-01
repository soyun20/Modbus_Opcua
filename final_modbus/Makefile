# Compiler and linker flags
CXX = g++
CXXFLAGS = -w
LDFLAGS = -lmodbus -ljsoncpp -lpaho-mqtt3a -lmariadb

# Source files and target executable
SRCS = main.cpp Server.cpp Client.cpp MQTT.cpp 
TARGET = my_program

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(SRCS) $(CXXFLAGS) $(LDFLAGS) -o $(TARGET) -g 

clean:
	rm -f $(TARGET)
