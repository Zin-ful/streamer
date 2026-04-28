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

#define PAGE_SIZE 55024

#define DIR_AMNT 24
#define DIR_LENGTH 312


#define MEDIA_AMNT 4096
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

int count(char *string1, char string2) {
    int current_count = 1;
    for (int i = 0; string1[i] != '\0'; i++) {
        if (string1[i] == string2) {
            current_count++;
        }
    }
    return current_count;
}

char *isolate(char *name, char string) {
    int current_count = count(name, string);
    int i = 0;
    for (i = 0; current_count > 1; i++) {
        if (name[i] == string) {
            current_count--;
        }
    }
    return name + i;
}

char* remove_extention(char *name) {
    char *dot = strrchr(name, '.');
    if (dot) {
        *dot = '\0';
    }
    return name;
}

void find_number(char *string, char new_string[BUFFER / 4]) {
    if (!strstr(string, "-")) {
        printf("No dash\n");
        strcpy(new_string, string);
        return;
    }
    char numbers[11] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '\0'};    
    char isolated_numbers[32];

    int position;
    int new_position = 0;
    int i;
    int found = 0;
    
    printf("Converting: %s\n", string);

    for (position = 0; string[position] != '-'; position++);
    position++;
    for (i = 0; numbers[i] != '\0'; i++) {
        if (numbers[i] == string[position]) {
            found = 1;
        }
    };
    if (!found) {
        printf("Not found\n");
        strcpy(new_string, string);
        return;    
    }
    for (i = position; string[i] != '\0'; i++) {
        isolated_numbers[new_position++] = string[i];    
    }
    isolated_numbers[new_position] = '\0';

    string[position] = '\0';
    strcpy(new_string, string);
    
    int converted_numbers[5] = {0};
    int current_count = 0;
    for (int j = 0; isolated_numbers[j] != '\0' && isolated_numbers[j] != '.'; j++) {
        for (i = 0; numbers[i] != '\0'; i++) {
            if (numbers[i] == isolated_numbers[j]) {
                converted_numbers[current_count] = i;
                current_count++;
                break;
            }
        }
    }
    
    converted_numbers[current_count - 1] += 1;
    printf("%d\n", converted_numbers[current_count]);
    for (i = current_count - 1; i >= 0; i--) {
        char temp_string[2] = {0};
        sprintf(temp_string + strlen(temp_string), "%d", converted_numbers[i]);
        strcat(new_string, temp_string);
    }
    printf("New String %s\n", new_string);
}

int count_char(char *string) {
    int i;
    for (i = 0; string[i] != '\0'; i++);
    return i;
}

void search(char *search, char files[MEDIA_AMNT][MEDIA_LENGTH], char result[PAGE_SIZE]) {
    printf("Searching...\n");
    
    char page_start[BUFFER];

    snprintf(page_start, sizeof(page_start),
    "<!DOCTYPE html><html lang='en'>\n"
    "<body style='background-color:black;'>\n"
    "<html>\n<head><title>Searching for: %s</title></head>\n"
    "<h1 style='color:white';><a style='color:white'; href='/'>Streamer</a></h1>\n"
    "<form>\n"
    "   <label style='color:white;'>Search for:</label>\n"
    "       <input type='search' id='site-search' name='search' placeholder='def not pirated'><br>\n"
    "    <input type='radio' id='tv' name='choice' value='movies'>\n"
    "       <label style='color:white;' for='option2'>Movies</label>\n"
    "    <input type='radio' id='movie' name='choice' value='tv'>\n"
    "       <label style='color:white;' for='option1'>Television Series</label><br><br>\n"     
    "    <button>Search</button>\n"
    "</form><br>\n", search);



    strcpy(result, page_start);
    
    int len = count_char(result);
    
    for (int i = 0; files[i][0] != 0; i++) {
        if (strstr(files[i], search) || search[0] == '*') {
            if (strcmp(files[i], "init") != 0) {
                strcat(result, "<p style='color:white';><a style='color:white'; href='");
                strcat(result, files[i]);
                strcat(result, "'>");
                strcat(result, remove_extention(isolate(files[i], '/')));
                strcat(result, "</a></p>\n");
            }
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

void clear_buffer(char buffer[]) {
    int i;
    for (i = 0; buffer[i] != '\0'; i++) {
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
    int i;
    for (i = 0; string[pos] != ' ' && string[pos] != '\0'; pos++) {
        path[i++] = string[pos];
    }
    path[i] = '\0';
}


char *verify_query(char *query) {
    char *result = strstr(query, "?");
    if (!result) {
        return NULL;
    }
    int skip_value = result - query;
    return query + skip_value;
}

void extract_parameters(char *parameters, char search[], char catagory[], char sort[]) {
    if (strlen(parameters) > 1) {
        if (parameters[1] != '?') {
            printf("failed first query check\n");
            if ((parameters = verify_query(parameters))) {
                printf("query verified: %s\n", parameters);
            } else {
                return;
            }
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
    memset(data->filepath, 0, sizeof(data->filepath));
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
        char temp[128];
        strcpy(temp, data->filepath + 1);
        strcpy(data->filepath, temp);
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


void send_custom_page(int socket, char *page) {
    send(socket, html_header, strlen(html_header), 0);
    send(socket, page, strlen(page), 0);
    close(socket);    
}

void send_video(int socket, char *path, char *client_request, char *ext) {
    printf("Preparing video\n");

    FILE *file = fopen(path, "rb");
    if (!file) {
        printf("File not found. Sending 404\n");
        fof(socket);
        return;
    }
    fseek(file, 0L, SEEK_END);
    long size = ftell(file);
    fseek(file, 0L, SEEK_SET);

    long start = 0;
    long end = size - 1;
    char *range = strstr(client_request, "Range: bytes=");

    char *type = strstr(path, ".mkv") ? "video/x-matroska" : "video/mp4"; //inline if statement
    char video_header[BUFFER];
        
    if (range) {
        range += 13;
        sscanf(range, "%ld-%ld", &start, &end);
        if (end <= 0 || end >= size) {
            end = size - 1;
        }
        fseek(file, start, SEEK_SET);
        
        snprintf(video_header, sizeof(video_header),
            "HTTP/1.1 206 Partial Content\r\n"
            "Content-Type: %s\r\n"
            "Accept-Ranges: bytes\r\n"
            "Content-Range: bytes %ld-%ld/%ld\r\n"
            "Content-Length: %ld\r\n\r\n",
            type, start, end, size, (end - start + 1));
        send(socket, video_header, strlen(video_header), 0);

    } else {
        snprintf(video_header, sizeof(video_header), 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: %s\r\n"
            "Accept-Ranges: bytes"
            "Content-Length: %ld"
            "\r\n",
            type, size);

        char video_player[BUFFER];
        char copy_of_path[BUFFER / 4];
        strcpy(copy_of_path, path);
        char next_up[BUFFER / 4];

        find_number(copy_of_path, next_up);

        snprintf(video_player, sizeof(video_player),
            "<!DOCTYPE html>\n"
            "<html>\n<head><title>Watching: %s</title></head>\n"
            "<h1 style='color:white;'><a href='/' style='color:white;'>Streamer</a></h1><br><br>\n"
            "<body style='background-color:black; color:white; text-align:center;'>\n"
            "<form>\n"
            "   <label style='color:white;'>Search for:</label>\n"
            "       <input type='search' id='site-search' name='search' placeholder='def not pirated'><br>\n"
            "    <input type='radio' id='tv' name='choice' value='movies'>\n"
            "       <label style='color:white;' for='option2'>Movies</label>\n"
            "    <input type='radio' id='movie' name='choice' value='tv'>\n"
            "       <label style='color:white;' for='option1'>Television Series</label><br><br>\n"
            "    <button>Search</button>\n"
            "</form><br>\n"
            "<h2>Watching %s</h2>\n"
            "<video width='1280' height='720' controls preload='metadata'>\n"
            "<source src='/%s' type='video/mp4'>\n"
            "</video>\n<br>"
            "<p><a href=/%s>Next Episode: %s</a></p>"
            "<button name='watchparty' value='%s'>Start Watch Party</button>"
            "</body></html>",
            isolate(path, '/'), isolate(path, '/'), path, strcat(next_up, ext), remove_extention(isolate(next_up, '/')), path);
            send(socket, video_header, strlen(video_header), 0);
            send_custom_page(socket, video_player);
            return;

    }

    char video_buffer[BUFFER];
    long remaining_bytes = end - start + 1;

    while (remaining_bytes > 0) {
        int chunk = (remaining_bytes < BUFFER) ? remaining_bytes : BUFFER;
        int bytes_read = fread(video_buffer, 1, chunk, file);
        if (bytes_read <= 0) break;

        if (send(socket, video_buffer, bytes_read, MSG_NOSIGNAL) <= 0) break;
        remaining_bytes -= bytes_read;
    }
    fclose(file);
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

void *client(void *new_socket) {
    pthread_detach(pthread_self());
    int socket = *(int *)new_socket;
    char buffer[BUFFER] = {0};
    struct RequestData request = {{0}, {0}, {0}, {0}, {0}, {0}};

    read(socket, buffer, BUFFER);
    if (strstr(buffer, "favicon.ico")) {
        printf("Sending fake favicon to satify dumb browsers\n");
    }
    printf("\n------------REQUEST--------\n\n%s\n-----------------------\n", buffer);
    
    parse_request(buffer, &request);
    
    if (request.search[0]) {
        printf("Client is searching for %s in the catagory of %s with a sorting criteria of: %s\n", request.search, request.catagory, request.sort);
        
        char (*files)[MEDIA_LENGTH] = malloc(MEDIA_AMNT * MEDIA_LENGTH);
        memset(files, 0, MEDIA_AMNT * MEDIA_LENGTH);
        char (*directories)[DIR_LENGTH] = malloc(DIR_AMNT * DIR_LENGTH);
        memset(directories, 0, DIR_AMNT * DIR_LENGTH);
        char *page = malloc(PAGE_SIZE);
        memset(page, 0, PAGE_SIZE);

        get_directories(directories);
        get_files(directories, files);
        search(request.search, files, page);
        
        send_custom_page(socket, page);
    } else {
        
        printf("Client is requesting the file %s via the %s method with a filetype of %s\n", request.filepath, request.method, request.filetype);
        
        if (strcmp(request.filetype, ".html") == 0 || strcmp(request.filetype, ".txt") == 0) {
            printf("Request is for webpages, sending %s\n", request.filepath);
            send_page(request.filepath, socket);
        } else if (strcmp(request.filetype, ".mp4") == 0 || strcmp(request.filetype, ".mkv") == 0) {
            printf("Request is for videos, sending %s\n", request.filepath);
            send_video(socket, request.filepath, buffer, request.filetype);
        }
    }
    
    printf("Client disconnected\n");
    free(new_socket);
    return NULL; //required by pthreads
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
    if (listen(server, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
        }
    while (1) {        
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
