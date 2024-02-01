// MQTTConsole.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include <stdio.h>
#include "ClientMQTT.h"
#include <string>
#include <iostream>

int main()
{
	ClientMQTT client;
	client.Setting();
	client.Initialize();

	printf("q to quit\n");
	printf("s to send testmsg\n");
	printf("p to receive testmsg\n");

	while (true)
	{
		printf("repeat\n");
		std::string cmd;
		std::cin >> cmd;

		if (cmd == "q")
		{
			break;
		}
		else if (cmd == "s")
		{
			//client.Publish("test", "test msg from me");
			printf("publish\n");
			client.Publish("test", "{\"ip\":\"123\",\"port\":\"567\",\"function\":\"789\"}");
		}
		else if (cmd == "p")
		{
			printf("Subscribe\n");
			client.Subscribe("test");
		}
	}

	printf("program exited\n");

    return 0;
}

