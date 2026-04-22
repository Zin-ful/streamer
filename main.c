#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#define ROOT "./"
#define PORT 8080
#define BUFFER 4096

char *html_header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";

struct RequestData {
    char method[16];
    char filepath[128];
    char search[64];
    char catagory[18];
    char sort[16];
};

void clear_buffer(char buffer[], int size) {
    int i;
    for (i = 0; i != size; i++) {
        buffer[i] = 0;
    }
    printf("Set %d bytes to 0\n", i);
}


void extract_parameters(char *parameters) {
    if (strlen(parameters) > 1) {
        if (parameters[1] != '?') {
            printf("No arguments in search\n");
            return;
        }
    } else {
        printf("No arguments in search\n");
        return;
    }
    printf("Getting search\n");
    int amount = 1;
    int i;
    for (int i = 0; i < strlen(parameters) - 1; i++){
        if (parameters[i] == '&') {
            amount++;
        }
    }
    printf("Arguments: %d\n", amount);
    char *parameter = parameters;
    char search[64] = {0};
    char catagory[18] = {0};
    char sort[16] = {0};
    i = 9;
    parameter = parameter + i;
    if (!parameter[0]) {
        printf("Search all, no other arguments\n");
        search[0] = '*';
        search[1] = '\0';
    } else {
        for (i = 0; parameter[i] != '&'; i++){
            search[i] = parameter[i]; 
    
        }
        search[i++] = '\0';
        amount--;
        parameter = parameter + i;
        if (amount) {
            for (i = 0; parameter[i] != '&' && parameter[i] != ' '; i++){
                catagory[i] = parameter[i]; 
            }
            catagory[i++] = '\0';
        }
        amount--;
        parameter = parameter + i;
        if (amount) {
            for (i = 0; parameter[i] != '&' && parameter[i] != ' '; i++){
                sort[i] = parameter[i]; 
            }
            sort[i++] = '\0';
        }

        printf("Search: %s\nCatagory: %s\nSort: %s\n", search, catagory, sort);
    }
}


int string_find_position(char *string1, char *string2) {
    char *result = strstr(string1, string2);
    if (!result) {
        return 0;
    }
    int pos = result - string1;
    return pos;
}

void string_find_path(char *string, char path[]) {
    int pos = string_find_position(string, "/");
    int i = 0;
    for (; string[pos] != ' '; pos++) {
        path[i++] = string[pos];
    }
    path[i] = '\0';
    printf("Found path: %s\n", path);
}

void send_html(char *new_path, int socket) {
    char path[128];
    snprintf(path, sizeof(path), "%s.html", new_path);
    
    FILE *file = fopen(path, "r");
   
    fseek(file, 0L, SEEK_END);
    int size = ftell(file);
    fseek(file, 0L, SEEK_SET);
    char html[size];
    send(socket, html_header, strlen(html_header), 0);
    while (fgets(html, sizeof(html), file)) {
        send(socket, html, strlen(html), 0);
    }
    fclose(file);
    close(socket);
}

void parse_request(char *request, struct RequestData data) {
    printf("Parsing request\n");
    char *method;
    if (strstr(request, "GET /")) {
        method = "GET";    
    } else if (strstr(request, "POST")) {
        method = "POST";
    } else {
        method = "UNKNOWN";
    }

    char path[128] = {0};
    string_find_path(request, path);
    strcpy(data.filepath, path);
    strcpy(data.method, method);
    extract_parameters(data.filepath);
    printf("Request parsed.\nMethod: %s\nRequested file: %s\n", data.method, data.filepath);
}

void *client(void *new_socket) {
    int socket = *(int *)new_socket;
    char buffer[BUFFER] = {0};
    read(socket, buffer, BUFFER);
    printf("\n------------REQUEST--------\n\n%s\n-----------------------\n", buffer);
    struct RequestData request;
    parse_request(buffer, request);
    
    send_html("index", socket);
    printf("Client disconnected\n");
    return NULL;
}

int main(void) {
    printf("Root directory: %s\nListening port: %d\n", ROOT, PORT);
    pthread_t thread;
    int client_socket, server;
    int opt = 1;

    struct sockaddr_in addr = {AF_INET, htons(PORT), INADDR_ANY};
    socklen_t addrlen = sizeof(addr);

    if ((server = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socked failed");
        exit(EXIT_FAILURE);
    }
    
    
    if (setsockopt(server, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        perror("Set options failed");
        exit(EXIT_FAILURE);
    }

    
    if (bind(server, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    while (1) {
        if (listen(server, 3) < 0) {
            perror("Listen failed");
            exit(EXIT_FAILURE);
        }
        
        client_socket = accept(server, (struct sockaddr*)&addr, &addrlen);
        if (client_socket < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }
        
        int *thread_client = malloc(sizeof(int));
        *thread_client = client_socket;
        pthread_create(&thread, NULL, &client, thread_client);
        
        printf("Client connected\n");
        
    }
    return 0;

}
