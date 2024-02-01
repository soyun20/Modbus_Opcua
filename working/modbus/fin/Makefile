all: gen watch server client

gen: Generate_Sensor.c
	gcc Generate_Sensor.c -o gen -lm -pthread



watch: Sensor_watch.c
	gcc Sensor_watch.c -o watch -pthread



server: ModbusServer.cpp
	g++ ModbusServer.cpp  Modbus_TCP_Class.cpp -pthread -o server


client: ModbusClient.c
	gcc ModbusClient.c -o client -lmysqlclient
