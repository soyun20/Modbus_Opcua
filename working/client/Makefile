CXX = g++
# 링크 플래그 지정. 라이브러리 링크를 포함합니다.
LDFLAGS = -lmodbus -ljsoncpp -lpaho-mqtt3a -lpthread -lmariadbclient
# 소스 파일 및 객체 파일 지정
SRCS = main.cpp Server.cpp Client.cpp MQTT.cpp
OBJS = $(SRCS:.cpp=.o)
# 최종 실행 파일 이름
TARGET = myProgram

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
