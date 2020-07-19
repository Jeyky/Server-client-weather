/**
 * A simple program that decreace number of requsets to weather API.
 *
 *	Copyright 2020, Volodymyr Teslenko <Jeyky @ github>
 *
 * This is server program that decrease number of requests to
 * API(In this case to weather API) by saving data in it's own storage
 * and comparing new client's requests with relevance of data that already
 * exist on server. If client makes request to the data which is not relevant,
 * server makes request to api saves in owns storage and sends to client.
 * For example if several clients makes request "Kyiv", server makes only one
 * request to api and sends the same data to clients.
 * Takes maximum one command line argument 'port'. Defaukt port 3622
 * Creates directory "Cities" where all the data storaged.
 * Create logs.txt and saves there logging information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "curl/curl.h"
#include "time.h"
#include "pthread.h"
#include "json-c/json.h"
#include "sys/stat.h"


#define PORT 3622
#define SERVER_BACKLOG 45
#define BUFFER_SIZE 1024

FILE *logs;
int port;
/**
 * to_lower() - Lowercase incoming string
 * @str: the (char *)argument to lowercase.
 * 
 * We need this function to lowercase input city name, so
 * we can save file with city name that will not be repeated.
 * for example: (input 1:Kyiv->kyiv; input 2: KYIV->kyiv).
 *
 * Return: Does not have return.
 */
void to_lower(char *str)
{
	for(int i=0; str[i]!='\0'; i++)
    {
        if(str[i]>='A' && str[i]<='Z')
        {
            str[i] = str[i] + 32;
        }
    }
}
/**
 * get_time() - returns current time.
 *
 * this function used to show time in server logs.
 * time format "DAY.MONTH.YEAR HOURS:MINS:SECONDS" 
 *
 * Return: returns allocated (char *timing) with time. size 20 bytes.
 *
 * The return value description can also have multiple paragraphs, and should
 * be placed at the end of the comment block.
 */

char *get_time(void)
{
	/* Returns time in format "DAY.MONTH.YEAR HOURS:MINS:SECONDS" */
	time_t check_time;
	time(&check_time);
	struct tm *ct = localtime(&check_time);
	char *timing = calloc(72,sizeof(char));
	sprintf(timing, "%d.%d.%d %d:%d:%d", ct->tm_mday,ct->tm_mon,ct->tm_year +1900, ct->tm_hour, ct->tm_min, ct->tm_sec);
	return timing;
}
/**
 * make_log() - Writes message in log.txt file.
 * @message: Describe the first argument.
 *
 * puts income message with current time in log.txt file.
 *
 * Return: Does not have return.
 */

void make_log(const char *message)
{
	char *timing = get_time();
	logs = fopen("logs.txt", "a+");
	fprintf(logs, "%s\t\t%s\n", timing, message);
	fclose(logs);
}
/**
 * struct string - used to save requested data from API.
 * @ptr: char pointer.
 * @len: length of char array.        
 *
 * We use this to request data from API using curl/curl.h 
 */
struct string {
	char *ptr;
	size_t len;
};
/**
 * struct thread_data - passes this info in pthread_create().
 * @NewSocket: socket descriptor.
 * @buff: our buffer that we used to receive or send data.
 * @NewAddr: structure that contains ip and port.        
 * @Addr_size: addres size;
 *
 * We pass this structure in pthread_create to have ability to write
 * ip and port of client if he disconnects. Also receive and send 
 * data without creating new buffer variable.
 */
struct thread_data{
	int NewSocket;
	char *buff;
	struct sockaddr_in NewAddr;
	socklen_t Addr_size;
};
/**
 * init_string() - allocates memory for our string structure.
 * @s: our string structure to be initialized.
 *
 * Allocates memory for our string structure that we use to collect
 * requested data from weather API. 
 * Context: If malloc() failes, logs it in logs.txt and exit the program.
 *
 * Return: Does not have return.
 */
void init_string(struct string *s)
{
	s->len = 0;
	s->ptr = malloc(s->len+1);
	if(s->ptr == NULL){
		fprintf(stderr, "malloc() failed;\n");
		make_log("malloc() failed");
		exit(1);
	}
	s->ptr[0]='\0';
}
/**
 * check_socket() - logs things related to socket.
 * @val: value that returned by socket functions
 * @msg: message to log.
 *
 * logs sockets creating, binding and listening with current time.
 *
 * Context: if socket function that checked with this one returnes error, exit the program.
 * Return: Does not have return.
 */
void check_socket(const int val, const char *msg)
{
	logs=fopen("logs.txt","a+");
	char *timing = get_time();
	fprintf(logs, "%s\t\t",timing);

	if(val<0){
		fprintf(logs, "%s FAILED\n", msg);
		fprintf(stderr, "%s FAILED\n", msg);
		exit(-1);
	}else if(!strcmp(msg, "Binding")){
		fprintf(logs,"Binded to port %d\n", port);
		fprintf(stderr,"Binded to port %d\n", port);
	}else{
		fprintf(logs, "%s SUCCESSFUL\n", msg);
		fprintf(stderr, "%s SUCCESSFUL\n", msg);
	}
	fclose(logs);
}
/**
 * create_socket() - creates socket.
 * @port: socket's port.
 *
 * Creates socket file descriptor with tcp, IPv4 protocols.
 * binds socket with address structure and listens. default port
 * is 3622 or you can change by passing port in argv[1].
 *
 * Context: if socket creating/binding/listening failes exit the program.
 * Return: returnes socket file descriptor.
 */
int create_socket(int port)
{
	int sock, ret;
	struct sockaddr_in serverAddr;

	check_socket(sock = socket(AF_INET, SOCK_STREAM, 0),"Socket creating");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	check_socket(ret = bind(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)), "Binding");

	check_socket(listen(sock, SERVER_BACKLOG), "Listening ...");

	return sock;
}
/**
 * add_time() - adds time argument to JSON requested from weather API.
 * @buff: buffer which contains JSON file.
 * @buff_len: buffers length..
 *
 * Adds time argument that we can compare lately to make decision to
 * make new request to weather API or no. thats help to decrease number of
 * requests if clients request the same thing, so server does not make requests to API,
 * but takes them from own directory and send to clients.
 *
 * Return: Doesn't have return.
 *
 * The return value description can also have multiple paragraphs, and should
 * be placed at the end of the comment block.
 */

void add_time(char *buff, int buff_len)
{
	time_t current_time;
	time(&current_time);
	char additional_info[20]=",\"time\":";
	char time[13];
	sprintf(time, "%ld", current_time);
	strcat(additional_info,time);

	for(unsigned int i=buff_len-1, j=0; j<strlen(additional_info); j++, i++){
		buff[i]=additional_info[j];
		buff[i+1]='}';
		buff[i+2]='\0';
	}
}
/**
 * cmp_time() - compares current time with time in json file.
 * @path: Path to JSON file which we want to compare with.
 *
 * A longer description, with more discussion of the function function_name()
 * that might be useful to those using or modifying it. Begins with an
 * empty comment line, and may include additional embedded empty
 * comment lines.
 *
 * The longer description may have multiple paragraphs.
 *
 * Context: Describes whether the function can sleep, what locks it takes,
 *          releases, or expects to be held. It can extend over multiple
 *          lines.
 * Return: Describe the return value of function_name.
 *
 * The return value description can also have multiple paragraphs, and should
 * be placed at the end of the comment block.
 */

double cmp_time(const char *path)
{	
	char buffer[1024];
	struct json_object *parsed_json;
	struct json_object *request_time;

	FILE *fp;
	fp =fopen(path, "r");
	fread(buffer, BUFFER_SIZE, 1, fp);

	parsed_json = 	json_tokener_parse(buffer);
	json_object_object_get_ex(parsed_json, "time", &request_time);

	int timing = json_object_get_int(request_time);

	time_t file_time = (time_t)timing;
	time_t current;
	time(&current);


	return difftime(current, file_time);
}
/**
 * write_func() - set callback for writes received data.
 * @ptr: Pointer to the delivered data
 * @size: Is always 1.
 * @nmemb: Size of that data, number of memory bites.
 * @s: pointer to the string data structure.
 *
 * This callback function cb() gets called by libcurl as soon as there is 
 * data received that needs to be saved.

 * The longer description may have multiple paragraphs.
 *
 * Context: For most transfers, this callback gets called many times and each invoke
 * delivers another chunk of data. The data passed to this function will not be null-terminated!
 * Return: The number of bytes actually taken care of.
 *
 * If that amount differs from the amount passed to your callback function,
 * it’ll signal an error condition to the library.
 */
size_t write_func(void *ptr, size_t size, size_t nmemb, struct string *s)
{
	size_t new_len =s->len +size*nmemb;
	s->ptr = realloc(s->ptr, new_len + 1);
	if(s->ptr == NULL){
		fprintf(stderr, "realloc() failed;\n" );
		exit(EXIT_FAILURE);
	}
	memcpy(s->ptr+s->len, ptr, size*nmemb);
	s->ptr[new_len]='\0';
	s->len = new_len;

	return size*nmemb; 
}

/**
 * curl_request() - makes request to weather API.
 * @buff: buffer with city name which our client sent.
 *
 * Function concatenates given city name with other parts of link
 * with which we make request to API. Uses functions from curl library.
 *
 * Return: Returns string with JSON data requested from weather API.
 */
char *curl_request(const char *buff)
{	
	CURL *curl = curl_easy_init();

	char link_fp[100]="http://api.openweathermap.org/data/2.5/weather?q=";
	char link_sp[100]="&appid=b19f6b88e922b3a3d763afe77f74ff4e";
	strcat(link_fp, buff);
	strcat(link_fp,link_sp);

	struct string s;
	init_string(&s);

	curl_easy_setopt(curl, CURLOPT_URL, link_fp);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_func);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
	//res = curl_easy_perform(curl);
	curl_easy_perform(curl);
	return s.ptr;
}
/**
 * function_name() - receives and sends data.
 * @arg: void pointer. We put here structure thread_data.
 *
 * Function receives data(city name) in buffer. If received data equals "quit",
 * closes socket and makes log. Else if file with city name exists on server compares
 * current time with time written in file. If time difference less than 20 minutes,
 * takes JSON file with weather data from it's own directory "Cities" and sends to client.
 * If since last request of this city data passed more than 20 minutes, server makes new request,
 * saves new JSON in directory "Cities" and sends JSON to the clients.
 * Otherwise (file does not exist on server), server makes request if cod argument from json file
 * equals 200(API has info for this city), than creates JSON file in "Cities" and sends JSON to client.
 * If code isn't 200, so this isn't city or happened some issues with API/connection and server logs it,
 * doesn't save JSON file, but sends JSON to client.
 *
 * Return: Returns NULL;
 */
void * handle_data(void * arg)
{	
	struct thread_data *data;
	data = (struct thread_data *)arg;
	int newSocket=data->NewSocket;
	char *buffer = data->buff;
	struct sockaddr_in newAddr=data->NewAddr;

	FILE * fp;

	struct json_object *parsed_json;
	struct json_object *cod;
	int cods;

	while(1){
			memset(buffer, '\0', BUFFER_SIZE);
			recv(newSocket, buffer, BUFFER_SIZE, 0);
			//fprintf(stderr,"b=%s",buffer);
			make_log("Client requested");
			make_log(buffer);
			to_lower(buffer);
			char dir[65]="./Cities/";
			char *test;
			//get_cod(buffer);
			strcat(dir,buffer);
			if(strcmp(buffer, "quit") == 0){
				logs = fopen("logs.txt", "a+");
				fprintf(logs,"%s\tDisconnected from %s:%d\n",get_time(), inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
				fclose(logs);
				break;
			}else{
				if((fp=fopen(dir, "r")) != NULL ){
					make_log("File exists on server");

					if(cmp_time(dir) < 1200.0){
						make_log("Passed less than 20 minutes, sending data from file to client");
						memset(buffer, '\0', BUFFER_SIZE);
						fread(buffer, BUFFER_SIZE, 1, fp);
					}else{
						make_log("Passed more than 20 minutes, making request and sending to client");
						fp=fopen(dir, "w");
						test = curl_request(buffer);
						memset(buffer, '\0', BUFFER_SIZE);
						strcpy(buffer, test);
						add_time(buffer, strlen(buffer));
						fprintf(fp, "%s", buffer);
						fclose(fp);
					}

				}else{
					make_log("File does not exist on server");

					test = curl_request(buffer);
					memset(buffer, '\0', BUFFER_SIZE);
					strcpy(buffer, test);
					add_time(buffer, strlen(buffer));

					parsed_json = json_tokener_parse(buffer);
					json_object_object_get_ex(parsed_json, "cod", &cod);
					cods = json_object_get_int(cod);

					if (cods == 200){
						make_log("Saving file on server");
						fp=fopen(dir, "w");
						fprintf(fp, "%s", buffer);
						fclose(fp);
					}else{
						make_log("Received wrong city name");
					}
				}
				memset(dir, '\0', sizeof(dir));

				send(newSocket, buffer, strlen(buffer), 0);
				memset(buffer, '\0', BUFFER_SIZE);
			}
		}
	return NULL;
}

int main(int argc, char *argv[])
{
	
	if(2 == argc){
		port = atoi(argv[1]);
	}else{
		port=PORT;
	}
	char *tims = get_time();
	fprintf(stderr,"TIMS=%s", tims);

	struct string s;
	init_string(&s);

	int sockfd;

	int newSocket;
	struct sockaddr_in newAddr;
	socklen_t addr_size;

	char buffer[1024];
	bzero(buffer, sizeof(buffer));

	mkdir("Cities", 0700);
	sockfd = create_socket(port);

	while(1){
		newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addr_size);
		logs = fopen("logs.txt", "a+");
		fprintf(logs,"%s\t\tConnection accepted from %s:%d\n", get_time(),inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
		fclose(logs);

		pthread_t t;
		
		struct thread_data to_thread;
		to_thread.NewSocket= newSocket;
		to_thread.buff = buffer;
		to_thread.NewAddr = newAddr;
		to_thread.Addr_size = addr_size;;
		pthread_create(&t, NULL, handle_data, &to_thread);
	}
	return 0;
}






