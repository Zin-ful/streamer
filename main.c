#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <dirent.h>


#define ROOT "./"
#define PORT 8080
#define BUFFER 4096

#define PAGE_SIZE 32512

#define DIR_AMNT 24
#define DIR_LENGTH 312


#define MEDIA_AMNT 8128
#define MEDIA_LENGTH 512


char *html_header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";

struct RequestData {
    char method[16];
    char filepath[128];
    char filetype[16];
    char search[64];
    char catagory[18];
    char sort[16];
};

void send_video(int socket, char *path) {
    printf("Preparing video\n");
}

int count_backslash(char *string) {
    int count = 1;
    for (int i = 0; string[i] != '\0'; i++) {
        if (string[i] == '/') {
            count++;
        }
    }
    return count;
}

char *isolate_filename(char *name) {
    int count = count_backslash(name);
    int i = 0;
    for (i = 0; count > 1; i++) {
        if (name[i] == '/') {
            count--;
        }
    }
    return name + i;
}

int count_char(char *string) {
    int i;
    for (i = 0; string[i] != '\0'; i++);
    return i;
}

void search(char *search, char files[MEDIA_AMNT][MEDIA_LENGTH], char result[PAGE_SIZE]) {
    printf("Searching...\n");
    
    char *page_start = 
    "<!DOCTYPE html><html lang='en'>\n"
    "<body style='background-color:black;'>\n"
    "<h1 style='color:white';><a style='color:white'; href='/'>Streamer</a></h1>\n";
    strcpy(result, page_start);
    
    int len = count_char(result);
    
    for (int i = 0; files[i][0] != 0; i++) {
        if (strstr(files[i], search) || search[0] == '*') {
            strcat(result, "<p style='color:white';><a style='color:white'; href='");
            strcat(result, files[i]);
            strcat(result, "'>");
            strcat(result, isolate_filename(files[i]));
            strcat(result, "</a></p>\n");
        }
    }

    if (len == count_char(result)) {
        strcat(result, "<p style='color:white';>No files with that name found! (Check spelling or capitalization).</p>\n");         
    }

    char *page_end =
    "</body>\n"
    "</html>";

    strcat(result, page_end);

}

void get_files(char directories[DIR_AMNT][DIR_LENGTH], char files[MEDIA_AMNT][MEDIA_LENGTH]) {
    struct dirent *entry;
    int count = 0;
    printf("Getting media content\n");
    

    char buffer[MEDIA_LENGTH];
    for (int i = 0; directories[i][0] != 0; i++) {
        DIR *media = opendir(directories[i]);
        while ((entry = readdir(media)) != NULL) {
            if (entry->d_name[0] != '.') {
                snprintf(buffer, sizeof(buffer), "%s/%s", directories[i], entry->d_name);
                strcpy(files[count], buffer);
                count++;
            }
        }
    }
    printf("Counted %d media\n", count);
}

void get_directories(char directories[DIR_AMNT][DIR_LENGTH]) {
    printf("Getting directories\n");
    struct dirent *mov_entry;
    struct dirent *tv_entry;
    
    DIR *movies = opendir("./media/movies");
    DIR *tv = opendir("./media/tv");
    
    if (!tv) {
        printf("Couldnt find directory ./media/tv\n");
    }
    if (!movies) {
        printf("Couldnt find directory ./media/movies\n");
    }
    int i = 0;
    char buffer[DIR_LENGTH];
    while ((mov_entry = readdir(movies)) != NULL) {
        if (mov_entry->d_name[0] != '.') {
            snprintf(buffer, sizeof(buffer), "./media/movies/%s", mov_entry->d_name);
            strcpy(directories[i], buffer);
            i++;
        }
    }
    while ((tv_entry = readdir(tv)) != NULL) {
        if (tv_entry->d_name[0] != '.') {
            snprintf(buffer, sizeof(buffer), "./media/tv/%s", tv_entry->d_name);
            strcpy(directories[i], buffer);
            i++;
        }
    }
    closedir(movies);
    closedir(tv);
}

void clear_buffer(char buffer[], int size) {
    int i;
    for (i = 0; i != size; i++) {
        buffer[i] = 0;
    }
    printf("Set %d bytes to 0\n", i);
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
}


void extract_parameters(char *parameters, char search[], char catagory[], char sort[]) {
    if (strlen(parameters) > 1) {
        if (parameters[1] != '?') {
            return;
        }
    } else {
        return;
    }
    int amount = 1;
    int i;
    for (size_t i = 0; i < strlen(parameters) - 1; i++){
        if (parameters[i] == '&') {
            amount++;
        }
    }
    char *parameter = parameters;
    i = 9;
    parameter = parameter + i;
    if (!parameter[0]) {
        search[0] = '*';
        search[1] = '\0';
        catagory[0] = '*';
        catagory[1] = '\0';
        sort[0] = '*';
        sort[1] = '\0';
    } else {
        
        for (i = 0; parameter[i] != '&' && parameter[i] != ' ' && parameter[i] != '\0'; i++){
            search[i] = parameter[i]; 
        }
        search[i] = '\0';
        parameter = parameter + i + 8;
        amount--;
        if (amount) {
            for (i = 0; parameter[i] != '&' && parameter[i] != ' ' && parameter[i] != '\0'; i++){
                catagory[i] = parameter[i];
            }
            catagory[i] = '\0';
            parameter = parameter + i;
            amount--;            
        }
        
        if (amount) {
            for (i = 0; parameter[i] != '&' && parameter[i] != ' ' && parameter[i] != '\0'; i++){
                sort[i] = parameter[i]; 
            }
            sort[i] = '\0';           
            amount--;
        }

        if (!search[0]) {
            search[0] = '*';
            search[1] = '\0';
        }
        
        if (!catagory[0]) {
            catagory[0] = '*';
            catagory[1] = '\0';
        }

        if (!sort[0]) {
            sort[0] = '*';
            sort[1] = '\0';
        }
    }
}


void parse_request(char *request, struct RequestData *data) {
    if (strstr(request, "GET /")) {
        strcpy(data->method, "GET");
    } else if (strstr(request, "POST")) {
        strcpy(data->method, "POST");
    } else {
        strcpy(data->method, "UNKNOWN");
    }

    string_find_path(request, data->filepath);
    
    
    if (strstr(data->filepath, ".mp4")) {
        strcpy(data->filetype, ".mp4");
    } else if (strstr(data->filepath, ".mkv")) {
        strcpy(data->filetype, ".mkv");
    } else if (strstr(data->filepath, ".html") || strstr(request, "GET / ")) {
        strcpy(data->filetype, ".html");
    } else if (!strstr(data->filepath, ".") && !strstr(data->filepath, "=")) {
        strcpy(data->filetype, "folder");
    } else if (strstr(data->filepath, "=")) {
        strcpy(data->filetype, "function");
    } else {
        strcpy(data->filetype, "unknown");
    }

    extract_parameters(data->filepath, data->search, data->catagory, data->sort);
    if (strcmp(data->filepath, "/") == 0) {
        strcpy(data->filepath, "index.html");
    } else {
        char *path = data->filepath;
        strcpy (data->filepath, path + 1);
    }
}

void fof(int socket) {
    char *header = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n";    
    send(socket, header, strlen(header), 0);
    char *page = 
    "<!DOCTYPE html><html lang='en'>"
    "<body style='background-color:black;'>"
    "<h1 style='color:white';><a style='color:white'; href='/'>Streamer</a></h1>"
    "<h2 style='color:white';> This is not the page you were looking for.. ooooooohhhhh</h2>"
    "</body>"
    "</html>";
    send(socket, page, strlen(page), 0); 
    close(socket);
}

void send_page(char *path, int socket) {
    FILE *file = fopen(path, "r");
    if (!file) {
        printf("File not found. Sending 404\n");
        fof(socket);
        return;
    }
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

void send_custom_page(int socket, char *page) {
    send(socket, html_header, strlen(html_header), 0);
    send(socket, page, strlen(page), 0);
    close(socket);    
}

void *client(void *new_socket) {
    int socket = *(int *)new_socket;
    char buffer[BUFFER] = {0};
    struct RequestData request = {{0}, {0}, {0}, {0}, {0}, {0}};

    read(socket, buffer, BUFFER);
    printf("\n------------REQUEST--------\n\n%s\n-----------------------\n", buffer);
    
    parse_request(buffer, &request);
    
    if (request.search[0]) {
        printf("Client is searching for %s in the catagory of %s with a sorting criteria of: %s\n", request.search, request.catagory, request.sort);
        
        char files[MEDIA_AMNT][MEDIA_LENGTH] = {0};
        char directories[DIR_AMNT][DIR_LENGTH] = {0};
        char page[PAGE_SIZE] = {0};

        get_directories(directories);
        get_files(directories, files);
        search(request.search, files, page);
        
        send_custom_page(socket, page);
    } else {
        printf("Client is requesting the file %s via the %s method with a filetype of %s\n", request.filepath, request.method, request.filetype);
        
        if (strcmp(request.filetype, ".html") == 0 || strcmp(request.filetype, ".txt") == 0) {
            printf("Request is for webpages, sending %s\n", request.filepath);
            send_page(request.filepath, socket);
        } elif (strcmp(request.filetype, ".mp4") == 0 || strcmp(request.filetype, ".mkv") == 0){
            printf("Request is for videos, sending %s\n", request.filepath);
            send_video(socket, request.filepath);
        }
    }
    
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
