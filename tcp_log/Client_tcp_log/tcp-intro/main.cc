// main.cc // client side

#include <string.h>
#include <stdio.h>
#include <iostream>
#include "source/networking.h"
#include <time.h>
#include <thread> 
#include <conio.h> 

string createMessage();
ip_address enter_ip();
int64 epoch();
bool isRunning = true;

void GetESCButtonDown() {
	const uint8 ESC = 27;
	char c;
	while (true) {
		c = _getch();
		if (c == ESC) {
			isRunning = false;
			break;
		}
	}
}


static int run_client(const ip_address& address) {
	const uint16 response_byte_size = 1024;
	char responseFromServer[response_byte_size] = { '\0' };
	bool hasRecived = false;

	int64 timeout_reciveTimer = epoch();
	int64 timeout_timer = epoch();

	tcp_socket sock;
	tcp_socket::result res = network_error_code::NETERR_HOST_DOWN;
	if (sock.connect(address).is_success()) {
		std::thread InputThread (GetESCButtonDown);

		string message = createMessage();
		printf("> ");
		for (auto c : message) {
			printf("%c", c);
		}
		bool has_sent = false;
		while (timeout_timer - timeout_reciveTimer < 60 && isRunning ) { // times out after 60 sec
			timeout_timer = epoch();

			if (!has_sent) {
				string message = createMessage();

				printf("> ");
				for (auto c : message) {
					printf("%c", c);
				}

				res = sock.send(strlen(message.c_str()), (uint8*)message.c_str()); // send make sure that all of the packets it sent;
				has_sent = true;
				timeout_reciveTimer = epoch();
			}

			if (timeout_timer - timeout_reciveTimer > 1) {
				has_sent = false;
				hasRecived = false;
			}

			if (network::error::is_non_critical(res.code_)) {
				hasRecived = false;
				int waitTimer = 5;
				while (!hasRecived) {

					//loop
					auto result = sock.receive(sizeof(responseFromServer), (uint8*)&responseFromServer); // receive make sure that all of the packets it sent;
					if (result.code_ == NETERR_NO_ERROR) {
						hasRecived = true;
						timeout_reciveTimer = epoch();
						printf("\n- Response from the server \n%s\n", responseFromServer);
						for (int i = 0; i < response_byte_size; i++) {
							responseFromServer[i] = '\0';
						}
					}
					else if (waitTimer > 0) {
						waitTimer--;
					}
					else if (waitTimer <= 0) {
						break;
					}
				}
			}
		}
		printf("\n> ");
		message = "Session timed out or cloesed. Closing the socket\n";
		for (auto c : message) {
			printf("%c", c);
		}
		InputThread.join();
	}
	else {
		uint16 sleep = 1000;
		while (sleep != 0) {
			sleep--;
		}
		printf("\n> ");
		string error = "Failed to connect trying again\n";
		for (auto c : error) {
			printf("%c", c);
		}


		run_client(enter_ip());
	}

	for (int i = 0; i < response_byte_size; i++) {
		responseFromServer[i] = { '\0' };
	}

	sock.close();
	return 0;
}

int main(int argc, char** argv) {

	network::init();

	int result = 0;
	result = run_client(enter_ip());

	network::shut();

	return 0;
}

static uint8 sendMessageCount = 0;
string createMessage() {
	string message = "PUT /game";
	sendMessageCount = (sendMessageCount + 1) % 5;
	string message_body[5] = { "Game init ok.", "Keeping the contection alive.", "TCP message over the net", "The client sends its regards", "This is a placeholder message please diregard me :D" };

	message += "\r\nTimestamp: " + std::to_string(epoch());
	message += "\r\nLength: " + std::to_string(message_body[sendMessageCount].length());
	message += "\r\n\r\n" + message_body[sendMessageCount];
	return message;
}

ip_address enter_ip() {
	ip_address address;


	char user_input[15];
	uint8 ip[4];

	printf("Enter the IP address as 127.0.0.1\n> ");
	gets_s(user_input);

	int index = 0;
	string temp;
	for (int i = 0; i < 15; i++) {
		if (user_input[i] == '\0' || user_input[i] == '.') {
			ip[index] = atoi(temp.c_str());
			index++;
			temp.clear();
			i++;
		}
		if (user_input[i] == '\0') {
			break;
		}
		temp += user_input[i];
		user_input[i] = '\0';
	}

	printf("Enter the the port\n> ");
	gets_s(user_input);

	address.set_host(ip[0], ip[1], ip[2], ip[3]); // local host
	address.set_port(atoi(user_input));
	return address;
}

int64 epoch() {
	time_t t;
	::time(&t);
	return t;
}