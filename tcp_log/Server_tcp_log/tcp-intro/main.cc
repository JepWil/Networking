// main.cc // server side

#include <string.h>
#include <stdio.h>
#include <vector>
#include "source/networking.h"
#include <time.h>

bool is_containing(string s, char c1, char c2);
bool is_containing(string s, string target);
int get_number_from_parsed_line(string s, char c);
uint8 parse_message(string message);
string createMessage(int res);
int64 epoch();
bool checkReciveMessage(string buffer);
int get_number_from_parsed_line(string s, char c, uint8 instance);
bool is_header_finished(string s);

struct client {
	client() {}
	client(ip_address address, tcp_socket sock) {
		address_ = address;
		socket_  = sock;
	}
	ip_address address_;
	tcp_socket socket_;
};
int64 ping = 0;

void print_message(string message){
	printf("> ");
	for(char c : message){
			printf("%c", c);
	}
	printf("\n");
}

static int run_server(const ip_address &address) {
	tcp_listener listener;
	
	std::vector<client> clients;
	char buffer[1024] = { 'a' };

	if (!listener.open(address)) {
		auto errcode = network::error::get_error();
		auto errmsg = network::error::as_string(errcode);
		printf("could not open listener - %s (%d)", errmsg, errcode);
		return 0;
	}

	printf("Server has been opened");

	while (true) {

		auto result = listener.accept();
		if (result.is_success()) {

			client cli(result.address_, result.socket_);
			clients.push_back(cli);

			uint8 remote_ip[4] = {0};
			remote_ip[0] = ((uint32)result.address_.host_ >> 24);
			remote_ip[1] = ((uint32)result.address_.host_ >> 16);
			remote_ip[2] = ((uint32)result.address_.host_ >> 8);
			remote_ip[3] = ((uint32)result.address_.host_ >> 0);

			printf("\n*Connection has been made from %i.%i.%i.%i:%i\a", remote_ip[0], remote_ip[1], remote_ip[2], remote_ip[3], result.address_.port_);
		}

		for (int i = 0; i < clients.size(); i++) {
			if (clients[i].socket_.is_valid()) {
				auto result = clients[i].socket_.receive(sizeof(buffer), (uint8*)& buffer); // loop in here
				
				if (result.code_ == network_error_code::NETERR_NO_ERROR && checkReciveMessage(buffer)) {
					printf("\n>%s\n", buffer);
					uint8 result = parse_message(buffer);

					string message = createMessage(result);

					auto response_result = clients[i].socket_.send(message.size(), (uint8*)message.c_str()); // loop in here
					if (network::error::is_non_critical(response_result.code_)) {
						printf("\n--Sending response to client\n");
						for (i = 0; i < 1024; i++) {
							buffer[i] = '\0';
						}
					}
					
				}
				if (result.code_ == network_error_code::NETERR_REMOTE_DISCONNECT) {
					printf("*Client has disconnect");
					clients.erase(clients.begin() + i);
				}
			}
		}
	}

	return 0;
}

// Parse
uint8 parse_message(string message){

	string file_path = "";
	int body_lenght = 0;
	
	string parsed_string = "";
	int success_counter = 0;
	bool empty_line = false;
	for (uint16 i = 0; i < message.length(); i++) {
		if (message[i] == NULL || message[i] == '\0') {
			return 30;
		}
		if (is_containing(parsed_string, '\r','\n')) { // checks if it has reached an end of line
			empty_line = true;
			success_counter++;
			if (is_containing(parsed_string, "PUT")) { // checks if it is the first part of the head
				empty_line = false;
				file_path = get_number_from_parsed_line(parsed_string, '/');
				if(file_path[0] == '/'){
					return 20;
				}
				//check file_path by fething file if error return 20 bad request
			}
			else if (success_counter == 0 && !is_containing(parsed_string, "PUT")) { // if it has not found the PUT command it is a bad request
				return 20;
			}
			else if (is_containing(parsed_string, "Timestamp")) { // Same as the above but for the timestamp
				empty_line = false;
				ping = epoch() - get_number_from_parsed_line(parsed_string, ':');
			}
			else if (is_containing(parsed_string, "Length")) { // Same as the above but for the length
				empty_line = false;
				body_lenght = get_number_from_parsed_line(parsed_string, ':');
			}
			parsed_string.clear();
		}

		parsed_string += message[i];
	}
	if (empty_line) { // it has reached the body and checks if the body is the same length as the promised 
		if (body_lenght == parsed_string.length()) {
			return 10;
		}
		else {
			return 20;
		}
	}
	return 30; // something has gone wrong
}

string createMessage(int result) {
	string response_ = "LOG: ";

	string message_body_ = "";

	string success_ = "10 \r\nDescription: Successful\r\n\r\n";
	string bad_request_ = "20 \r\nDescription: Bad Request\r\n\r\n";
	string server_error_ = "30 \r\nDescription: Internal Server Error\r\n\r\n";

	if (result == 10) {
		response_ += success_;
		message_body_ += "Ping: " + std::to_string(ping); // we calculate the ping wrongly  
		message_body_ += "\r\n";
		response_ += message_body_;

		print_message(response_);
	}
	else if (result == 20) {
		response_ += bad_request_;
		message_body_ += "Pong: " + std::to_string(ping);
		message_body_ += "\r\n";
		response_ += message_body_;

		print_message(response_);
	}
	else {
		response_ += server_error_;
		print_message(response_);
	}

	return response_;
}

bool is_containing(string s, char c1, char c2) {
	for (uint8 i = 0; i < s.length(); i++) {
		if (s[i] == c1 && s[i+1] == c2) {
			return true;
		}
	}
	return false;
}

bool is_header_finished(string s) {
	string end_header = "\r\n\r\n";
	for (uint8 i = 0; i < s.length(); i++) {
		if (s[i] == end_header[0] && s[i + 1] == end_header[1] && s[i+2] == end_header[2] && s[i + 3] == end_header[3]) {
			return true;
		}
	}
	return false;
}

bool is_containing(string s, string target) {
	if (s.find(target)) {
		return false;
	}
	return true;
}

int get_number_from_parsed_line(string s, char c){
	string tmp = "";
	bool found_char = false;
	for (uint8 index = 0; index < s.length(); index++) {
		if (s[index] == c) {
			index++;
			found_char = true;
		}
		if (found_char) {
			tmp += s[index];
		}
		if (s[index] == '\r' && s[index + 1] == '\n') {
			break;
		}
	}
	return atoi(tmp.c_str());
}

int get_number_from_parsed_line(string s, char c, uint8 instance) {
	string tmp = "";
	bool found_char = false;
	uint8 appereance_of_char = 0;
	for (uint8 index = 0; index < s.length(); index++) {
		if (s[index] == c) {
			index++;
			appereance_of_char++;
			found_char = true;
		}
		if (found_char && appereance_of_char == instance ) {
			tmp += s[index];
		}
		if (s[index] == '\r' && s[index + 1] == '\n' && appereance_of_char >= instance) {
			break;
		}
	}
	return atoi(tmp.c_str());
}

int64 epoch() {
	time_t t;
	::time(&t);
	return t;
}

bool checkReciveMessage(string buffer) {
	string parsed_string = "";
	int body_lenght = 0;
	for (uint16 i = 0; i < buffer.length(); i++) {
		if (is_header_finished(parsed_string)) {
			body_lenght = get_number_from_parsed_line(parsed_string, ':', 2);
			parsed_string.clear();
		}
		parsed_string += buffer[i];
	}

	if (body_lenght == parsed_string.length()) {
		return true;
	}
	return false;
}

int main(int argc, char** argv) {
	char user_input_[15];
	uint8 ip_[4];


	network::init();
	ip_address server_ip;

	printf("Enter the IP address as 127.0.0.1\n> ");
	gets_s(user_input_);

	int index = 0;
	string temp;
	for (int i = 0; i < 15; i++) {
		if (user_input_[i] == '\0' || user_input_[i] == '.') {
			ip_[index] = atoi(temp.c_str());
			index++;
			temp.clear();
			i++;
		}
		if (user_input_[i] == '\0') {
			break;
		}

		temp += user_input_[i];
		user_input_[i] = '\0';
	}
	
	printf("Enter the the port\n> ");
	gets_s(user_input_);

	server_ip.set_host(ip_[0], ip_[1], ip_[2], ip_[3]); // local host
	server_ip.set_port(atoi(user_input_));

	int result = run_server(server_ip);

	network::shut();
	
	return 0;
}


