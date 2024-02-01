# Compiler and linker flags
CXX = g++
CXXFLAGS = -w
LDFLAGS = -lmodbus -ljsoncpp -lpaho-mqtt3a -lmariadb -lopen62541 -lssl -lcrypto -DUA_ENABLE_ENCRYPTION -lmbedtls -lmbedx509 -lmbedcrypto -DUA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS -UA_ENABLE_HISTORIZING

# Source files and target executable
SRCS = main.cpp MQTT.cpp Server.cpp Client.cpp
TARGET = my_program

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(SRCS) $(CXXFLAGS) $(LDFLAGS) -o $(TARGET) -g 

clean:
	rm -f $(TARGET)
