#include <iostream>
#include <cstring>
#include <cstdlib>
#include <mosquitto.h>

// 콜백 함수 - 메시지를 받았을 때 호출됨
void message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message) {
    if (message->payloadlen) {
        std::cout << "토픽: " << message->topic << ", 메시지: " << (char *)message->payload << std::endl;
    } else {
        std::cout << "빈 메시지" << std::endl;
    }
}

int main() {
    struct mosquitto *mosq = NULL;
    const char *host = "localhost"; // MQTT 브로커 호스트 주소
    int port = 1883; // MQTT 브로커 포트
    const char *topic = "test"; // 구독할 토픽
    const char *message = "Hello, MQTT!"; // 발행할 메시지

    mosquitto_lib_init(); // MQTT 라이브러리 초기화

    // MQTT 클라이언트 생성
    mosq = mosquitto_new(NULL, true, NULL);
    if (!mosq) {
        std::cerr << "MQTT 클라이언트 생성 실패" << std::endl;
        return 1;
    }

    // 메시지 콜백 함수 등록
    mosquitto_message_callback_set(mosq, message_callback);

    // MQTT 브로커에 연결
    if (mosquitto_connect(mosq, host, port, 60) != MOSQ_ERR_SUCCESS) {
        std::cerr << "MQTT 브로커에 연결 실패" << std::endl;
        return 1;
    }

    // 토픽 구독
    if (mosquitto_subscribe(mosq, NULL, topic, 0) != MOSQ_ERR_SUCCESS) {
        std::cerr << "토픽 구독 실패" << std::endl;
        return 1;
    }

    // 메시지 발행
    if (mosquitto_publish(mosq, NULL, topic, strlen(message), message, 0, false) != MOSQ_ERR_SUCCESS) {
        std::cerr << "메시지 발행 실패" << std::endl;
        return 1;
    }

    // 메시지 수신 및 처리
    mosquitto_loop_forever(mosq, -1, 1);

    // MQTT 클라이언트 종료
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    return 0;
}
