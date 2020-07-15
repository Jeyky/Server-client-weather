/**
 * simple console client program to send requests to server and get weather info.
 * maximum 1 command line argument port. Default port 3622 for client and server.
 * how to use:
 *		client: Kyiv
 *		Server:
 *		temrature: 21.0°C
 *		humidity: 46%
 *		overcast clouds
 *		Client: quit
 *		Disconnected from server
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unistd.h"
#include "sys/socket.h"
#include "sys/types.h"
#include "netinet/in.h"
#include"arpa/inet.h"
#include "json-c/json.h"


#define PORT 3622
#define BUFFER_SIZE 1024

/**
 * show_weather() - shows temprature, humidity and description of weather.
 * @buffer: buffer that contain requested JSON file.
 *
 * Extracts temrature, huidity and description arguments from requested JSON file
 * and checks if this data is what we need.
 * 
 *
 * The longer description may have multiple paragraphs.
 *
 * Return: Does not have return.
 */

void show_weather(char *buffer)
{
	struct json_object *parsed_json;
	struct json_object *hum;
	struct json_object *temp;
	struct json_object *temporary;
	struct json_object *cod;
	struct json_object *weather;
	struct json_object *main_arg;
	struct json_object *description;

	parsed_json = json_tokener_parse(buffer);
	json_object_object_get_ex(parsed_json, "main", &temporary);
	json_object_object_get_ex(parsed_json, "cod", &cod);
	if(json_object_get_int(cod) == 200){
		json_object_object_get_ex(json_tokener_parse(json_object_get_string(temporary)), "temp", &temp);
		json_object_object_get_ex(json_tokener_parse(json_object_get_string(temporary)), "humidity", &hum);

		json_object_object_get_ex(parsed_json, "weather", &weather);
		weather = json_object_array_get_idx(weather, 0);
		parsed_json = json_tokener_parse(json_object_get_string(weather));
		json_object_object_get_ex(parsed_json, "main", &main_arg);
		json_object_object_get_ex(parsed_json, "description", &description);

		printf("temrature: %.1f°C\n", json_object_get_double(temp)-273.15);
		printf("humidity: %d%%\n", json_object_get_int(hum));
		printf("%s\n", json_object_get_string(description));
			}else{
				printf("Wrong city name ¯\\_(ツ)_/¯\n");
			}
}

int main(int argc, char *argv[])
{
	int port;
	int clientSocket;
	struct sockaddr_in serverAddr;
	char buffer[1024];
	memset(buffer, '\0', BUFFER_SIZE); 

	if(argc == 2){
		port = atoi(argv[1]);
	}else{
		port = PORT;
	}
	
	

	//creates socket's file descriptor
	clientSocket =socket(AF_INET, SOCK_STREAM, 0);		// IPv4, tcp
	if(clientSocket < 0 ){
		fprintf(stderr, "Error socket creating\n");
		exit(1);
	}
	printf("Client socket is created\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;	
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	int ret;
	ret = connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
	if (ret<0){
		fprintf(stderr, "Error socket connection\n");
		exit(1);
	}
	printf("server connected\n");
	printf("\nprint: 'quit' to close program.\n");

	while(1){
		printf("Client: ");
		scanf("%s", &buffer[0]);
		send(clientSocket, buffer, strlen(buffer), 0);

		if (strcmp(buffer, "quit") == 0){
			memset(buffer, '\0', BUFFER_SIZE); 
			close(clientSocket);
			printf("Disconnected from server\n");
			exit(1);
		}
		if(recv(clientSocket, buffer, 1024, 0) < 0){
			fprintf(stderr ,"error in receiving data\n");
		}else{
			show_weather(buffer);
			memset(buffer, '\0', BUFFER_SIZE);  	
		}
	}
	return 0;
}
