#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

const char* content_type_text = "text/plain";
const char* content_type_octet_stream = "application/octet-stream";

void create_ok_response(char* content, char** response, char* content_type) {
	int content_len = strlen(content);

	char s_content_len[10];
	sprintf(s_content_len, "%d", content_len);

	int response_buffer_size = content_len + 100;
	*response = (char*) malloc((response_buffer_size+1) * sizeof(char));

	strcpy(*response, "HTTP/1.1 200 OK\r\nContent-Type: ");
	strcat(*response, content_type);
	strcat(*response, "\r\nContent-Length: ");
	strcat(*response, s_content_len);
	strcat(*response, "\r\n\r\n");
	strcat(*response, content);
}

void process_request(char* request_body, char** response, int argc, char** argv) {
	char* method = strtok(request_body, " ");
	char* path = strtok(NULL, " ");
	char* protocol = strtok(NULL, "\r\n");
	char* user_agent;

	char* header;
	while((header = strtok(NULL, "\r\n"))) {
		if(strncmp("User-Agent: ", header, 12) == 0) {
			user_agent = strdup(header+12);
		}
	}

	if (strcmp("/", path) == 0) {
		*response = strdup("HTTP/1.1 200 OK\r\n\r\n");
	} else if (strncmp("/echo/", path, 6) == 0) {
		char* content = path + 6;
		create_ok_response(content, response, content_type_text);
	} else if (strncmp("/user-agent", path, 11) == 0) {
		create_ok_response(user_agent, response, content_type_text);
	} else if (strncmp("/files/", path, 7) == 0) {
		if (argc < 2 || strcmp(argv[1], "--directory") != 0) {
			return;
		}
		char* file_name = path + 7;
		char* file_path = malloc(strlen(argv[2]) + strlen(file_name) + 1);
		strcpy(file_path, argv[2]);
		strcat(file_path, file_name);

		FILE* req_file;
		req_file = fopen(file_path, "r");

		if (req_file == NULL) {
			*response = strdup("HTTP/1.1 404 Not Found\r\n\r\n");
		} else {
			fseek(req_file, 0, SEEK_END);
			long file_size = ftell(req_file);
			rewind(req_file);

			char* file_content = malloc(file_size + 1);
			if (file_content == NULL) {
				printf("error: couldn't allocate %ld bytes to read file", file_size);
				fclose(req_file);
				*response = strdup("HTTP/1.1 500 Internal Server Error\r\n\r\n");
				return;
        	}

			size_t read_size = fread(file_content, sizeof(char), file_size, req_file);
			if (read_size != file_size) {
				printf("error: file size is %ld bytes, read %ld bytes", file_size, read_size);
				fclose(req_file);
            	free(file_content);
            	*response = strdup("HTTP/1.1 500 Internal Server Error\r\n\r\n");
            	return;
			}
			file_content[file_size] = '\0';

			fclose(req_file);
			create_ok_response(file_content, response, content_type_octet_stream);
			free(file_content);
		}
	} else {
		*response = strdup("HTTP/1.1 404 Not Found\r\n\r\n");
	}
}

void handle_client(int client_fd, int argc, char** argv) {
	char request_body[BUFFER_SIZE] = {0};
	int bytes_read = recv(client_fd, request_body, BUFFER_SIZE, 0);

	if (bytes_read < 0) {
		printf("Error in request: %s \n", strerror(errno));
		close(client_fd);
		return;
	}

	char* response;
	process_request(request_body, &response, argc, argv);

	if(send(client_fd, response, strlen(response), 0) == -1) {
		printf("Response failed: %s \n", strerror(errno));
		close(client_fd);
		return;
	}

	printf("Response sent\n");
	free(response);
	close(client_fd);
}

int main(int argc, char *argv[]) {
	// Disable output buffering
	setbuf(stdout, NULL);
 	setbuf(stderr, NULL);
	
	int server_fd, client_addr_len;
	struct sockaddr_in client_addr;
	
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1) {
		printf("Socket creation failed: %s...\n", strerror(errno));
		return 1;
	}
	
	// Since the tester restarts your program quite often, setting SO_REUSEADDR
	// ensures that we don't run into 'Address already in use' errors
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		printf("SO_REUSEADDR failed: %s \n", strerror(errno));
		close(server_fd);
		return 1;
	}
	
	struct sockaddr_in serv_addr = { .sin_family = AF_INET ,
									 .sin_port = htons(4221),
									 .sin_addr = { htonl(INADDR_ANY) },
									};
	
	if (bind(server_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0) {
		printf("Bind failed: %s \n", strerror(errno));
		close(server_fd);
		return 1;
	}
	
	int connection_backlog = 5;
	if (listen(server_fd, connection_backlog) != 0) {
		printf("Listen failed: %s \n", strerror(errno));
		close(server_fd);
		return 1;
	}
	
	printf("Waiting for a client to connect...\n");
	client_addr_len = sizeof(client_addr);

	while (1) {
		int client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
		printf("Client connected\n");

		pid_t pid = fork();
		if (pid == 0) {
			// child process
			close(server_fd);
			handle_client(client_fd, argc, argv);
			exit(0);
		} else if (pid > 0) {
			// parent process
			close(client_fd);
		} else {
			printf("Fork failed: %s \n", strerror(errno));
			close(client_fd);
		}

		while (waitpid(-1, NULL, WNOHANG) > 0);
	}

	close(server_fd);

	return 0;
}