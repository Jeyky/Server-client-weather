/**
 * Simple GUI client program to send requests to server and get weather info.
 * maximum 1 command line argument port. Default port 3622 for client and server.
 * how to use:
 * print city name in text field, press the button and you can see result on display.
*/

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "gtk/gtk.h"
#include "sys/socket.h"
#include "sys/types.h"
#include "netinet/in.h"
#include"arpa/inet.h"
#include "gtk/gtk.h"
#include "json-c/json.h"


#define PORT 3622
#define BUFFER_SIZE 1024


static GtkWidget *city;
static GtkWidget *temprature;
static GtkWidget *humidity;
static GtkWidget *error;
static GtkWidget *situation;
static GtkWidget *grid;
static GtkWidget *quit;

int clientSocket;
int port;
char buffer[1024];

void leave(){
	memset(buffer, '\0', BUFFER_SIZE); /////
	strcpy(buffer,"quit");
	send(clientSocket, buffer, strlen(buffer), 0);
	close(clientSocket);

	printf("Disconnected from server\n");
	gtk_main_quit();
}
/**
 * do_call() - makes request and shows weather on button click.
 *
 * Makes request to server. Extract data from received JSON file and set 
 * labels values from this data.
 *
 * Context: if input = quit disconnects froms server and closes program.
 * Return: Does not have return;
 */
void do_call()
{
	struct json_object *parsed_json;
	struct json_object *hum;
	struct json_object *temp;
	struct json_object *temporary;
	struct json_object *cod;
	struct json_object *weather;
	struct json_object *main_arg;
	struct json_object *description;
	char b[100];
	strcpy(b,gtk_entry_get_text(GTK_ENTRY(city)));
	if(strlen(b) == 0){
		memset(buffer, '\0', BUFFER_SIZE);
		strcpy(buffer,"qq");
		send(clientSocket, buffer, strlen(buffer), 0);
	}else{
		memset(buffer, '\0', BUFFER_SIZE);
		strcpy(buffer,gtk_entry_get_text(GTK_ENTRY(city)));
		send(clientSocket, buffer, strlen(buffer), 0);
	}
	if(!(strcmp(buffer,"quit"))){
		leave();
	}
	if(recv(clientSocket, buffer, BUFFER_SIZE, 0) < 0){
			gtk_label_set_text(GTK_LABEL(error), "ERROR");

		}
	char temp_label[30];
	char hum_label[30];
	char error_label[30];
	char situation_label[30];

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

		snprintf(temp_label, sizeof(temp_label), "temprature: %.1f°C", json_object_get_double(temp)-273.15);
		snprintf(hum_label, sizeof(hum_label), "humidity: %d%%", json_object_get_int(hum));
		snprintf(situation_label,sizeof(situation_label), "%s", json_object_get_string(description));

		gtk_label_set_text(GTK_LABEL(temprature), temp_label);
		gtk_label_set_text(GTK_LABEL(humidity), hum_label);
		gtk_label_set_text(GTK_LABEL(situation), situation_label);
		snprintf(error_label,sizeof(error_label), "%s", " ");
		gtk_label_set_text(GTK_LABEL(error), error_label);
	}else{
		snprintf(error_label,sizeof(error_label), "%s", "Wrong city name");
		gtk_label_set_text(GTK_LABEL(error), error_label);
	}

}

int main(int argc, char *argv[])
{
	if(argc == 2){
		port = atoi(argv[1]);
	}else{
		port = PORT;
	}
	struct sockaddr_in serverAddr;
	
	memset(buffer, '\0', BUFFER_SIZE); 

	clientSocket =socket(AF_INET, SOCK_STREAM, 0);
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



	


	GtkWidget *window, *call;
	gtk_init(&argc, &argv);

	

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(window, "destroy", G_CALLBACK(leave), NULL);



	gtk_window_set_default_size(GTK_WINDOW(window), 450, 350);
	gtk_window_set_resizable (GTK_WINDOW(window), TRUE);

	grid = gtk_grid_new();
	gtk_container_add(GTK_CONTAINER(window), grid);

	city = gtk_entry_new();	
	gtk_grid_attach(GTK_GRID(grid), city, 0, 0, 1, 1);

	call = gtk_button_new_with_label("Show");
	g_signal_connect(call, "clicked", G_CALLBACK(do_call), NULL);
	gtk_grid_attach(GTK_GRID(grid), call, 0,1,1,1);
	gtk_container_set_border_width(GTK_CONTAINER(call), 10);

	quit = gtk_label_new("type: 'quit' to close");
	gtk_grid_attach(GTK_GRID(grid), quit, 0, 2, 1, 1);
	

	temprature = gtk_label_new("temprature:");
	gtk_grid_attach(GTK_GRID(grid), temprature, 2, 0, 1, 1);

	humidity = gtk_label_new("humidity:");
	gtk_grid_attach(GTK_GRID(grid), humidity, 2, 1, 1, 1);

	situation = gtk_label_new("-");
	gtk_grid_attach(GTK_GRID(grid), situation, 2, 2, 1, 1);

	error = gtk_label_new("");
	gtk_grid_attach(GTK_GRID(grid), error, 0, 3, 1, 1);
	
	
	gtk_widget_set_hexpand (temprature, TRUE);
	gtk_widget_set_halign (temprature, GTK_ALIGN_FILL);
	gtk_widget_set_hexpand (grid, TRUE);
	gtk_widget_set_halign (grid, GTK_ALIGN_FILL);

	gtk_widget_show_all(window);
	gtk_main();
	return 0;

}
