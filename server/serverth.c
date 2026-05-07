#include <stdio.h>      // printf
#include <string.h>     // strlen, memcpy
#include <unistd.h>     // close, read
#include <arpa/inet.h>  // sockaddr_in, htons
#include <stdbool.h>    // bool, true, false
#include <stdlib.h>     // malloc, free, exit
#include <pthread.h>    // pthreads

// PTT - Protocolo de Transferencia de Telemetría
struct Message {
    char header[4];   // "PTT"
    char action[12];  // "UPLOAD" o "DOWNLOAD"
    char data[150];   // Datos de telemetría
    char footer[4];   // "END"
};

void parseMessage(struct Message msg, char *buffer) {
    memcpy(buffer, msg.header, sizeof(msg.header));
    memcpy(buffer + sizeof(msg.header), msg.action, sizeof(msg.action));
    memcpy(buffer + sizeof(msg.header) + sizeof(msg.action), msg.data, sizeof(msg.data));
    memcpy(buffer + sizeof(msg.header) + sizeof(msg.action) + sizeof(msg.data), msg.footer, sizeof(msg.footer));
}

// Función que manejará cada cliente en un hilo
void *handle_client(void *arg) {
    int client_fd = *(int *)arg;
    free(arg);  // liberamos memoria reservada en el main

    char buffer[1024];
    bool connection_active = true;

    printf("[HILO] Cliente conectado (fd=%d)\n", client_fd);

    while (connection_active) {
        memset(buffer, 0, sizeof(buffer));
        int bytes = read(client_fd, buffer, sizeof(buffer));

        if (bytes <= 0) {
            printf("[HILO] Cliente (fd=%d) desconectado.\n", client_fd);
            break;
        }

        printf("[HILO] Mensaje recibido de fd=%d: %s\n", client_fd, buffer);

        if (strncmp(buffer, "EXIT", 4) == 0) {
            printf("[HILO] Cliente pidió salir.\n");
            break;
        }

        // Crear mensaje de respuesta
        struct Message response;
        strcpy(response.header, "PTT");
        strcpy(response.action, "DATA");
        strcpy(response.data, "speed=45;temp=22.5;fuel=78.9");
        strcpy(response.footer, "END");

        char msg_buffer[sizeof(response)];
        memset(msg_buffer, 0, sizeof(msg_buffer));
        parseMessage(response, msg_buffer);

        // Enviar respuesta al cliente
        send(client_fd, msg_buffer, sizeof(response), 0);
    }

    close(client_fd);
    pthread_exit(NULL);
}

int main() {
    int domain = AF_INET;   // IPv4
    int type = SOCK_STREAM; // TCP
    int protocol = 0;

    int socket_fd = socket(domain, type, protocol);  // crear socket
    if (socket_fd < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(2000);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(socket_fd, 5) < 0) {
        perror("listen failed");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    printf("[SERVIDOR] Escuchando en el puerto 2000...\n");

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    while (1) {
        int *client_fd = malloc(sizeof(int));
        *client_fd = accept(socket_fd, (struct sockaddr *)&client_addr, &client_len);

        if (*client_fd < 0) {
            perror("accept failed");
            free(client_fd);
            continue;
        }

        // Crear un hilo para manejar este cliente
        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, client_fd);
        pthread_detach(tid);  // para que el hilo se libere solo al terminar

        printf("[SERVIDOR] Nuevo cliente conectado (fd=%d)\n", *client_fd);
    }

    close(socket_fd);
    return 0;
}