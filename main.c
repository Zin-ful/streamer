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

void clear_buffer(char buffer[], int size) {
    int i;
    for (i = 0; i != size; i++) {
        buffer[i] = 0;
    }
    printf("Set %d bytes to 0\n", i);
}

void html_head(int socket) {
    send(socket, html_header, strlen(html_header), 0);
}

void send_html(char *new_path, int socket) {
    char path[128];
    snprintf(path, sizeof(path), "%s.html", new_path);
    
    FILE *file = fopen(path, "r");
   
    fseek(file, 0L, SEEK_END);
    int size = ftell(file);
    fseek(file, 0L, SEEK_SET);
    char html[size];
    html_head(socket);
    while (fgets(html, sizeof(html), file)) {
        send(socket, html, strlen(html), 0);
    }
    fclose(file);
    close(socket);
}

void *client(void *new_socket) {
    int socket = *(int *)new_socket;
    char buffer[BUFFER] = {0};
    read(socket, buffer, BUFFER);
    printf("\n------------REQUEST--------\n\n%s\n-----------------------\n", buffer);
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
