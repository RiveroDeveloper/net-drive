#include <stdio.h>      // printf
#include <string.h>     // strlen
#include <unistd.h>     // close, read
#include <arpa/inet.h>  // sockaddr_in, htons
#include <stdbool.h>    // bool, true, false
#include <stdlib.h>     // malloc, free

//PTT- Protocolo de Transferencia de Telemetría
struct Message {
    char *header; // "PTT"
    char *action; // "UPLOAD" o "DOWNLOAD"
    char *data;   // Datos de telemetría (entrada por teclado)
    char *footer; // "END"
};

// Genera un buffer contiguo con todos los campos concatenados
char *parseMessage(struct Message msg) {
    int total_size = strlen(msg.header) + strlen(msg.action) + strlen(msg.data) + strlen(msg.footer) + 4;
    char *buffer = malloc(total_size);
    if (!buffer) {
        perror("malloc");
        exit(1);
    }

    // Concatenar los campos en orden
    sprintf(buffer, "%s%s%s%s", msg.header, msg.action, msg.data, msg.footer);
    return buffer;
}

int main() {
    int domain = AF_INET;       // IPv4
    int type = SOCK_STREAM;     // TCP
    int protocol = 0;           // default protocol

    int socket_fd = socket(domain, type, protocol);  // create socket

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;              
    server_addr.sin_port = htons(2000);            
    server_addr.sin_addr.s_addr = INADDR_ANY;      

    bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(socket_fd, 1);

    printf("Server listening on port 2000...\n");

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(socket_fd, (struct sockaddr *)&client_addr, &client_len);

    printf("Client connected!\n");

    char buffer[1024];
    bool connection_active = true;

    while (connection_active) {
        memset(buffer, 0, sizeof(buffer));
        int bytes = read(client_fd, buffer, sizeof(buffer));

        if (bytes <= 0) break; // conexión cerrada por el cliente

        printf("Client message: %s\n", buffer);

        if (strncmp(buffer, "EXIT", 4) == 0) {
            printf("Exit command received. Closing connection.\n");
            break;
        }

        //Leer mensaje de telemetría desde teclado
        char input[256];
        printf("Escribe el mensaje de telemetría a enviar: ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0; // quitar salto de línea

        //Crear mensaje dinámico
        struct Message data;
        data.header = malloc(strlen("PTT") + 1);
        data.action = malloc(strlen("SEND") + 1);
        data.data   = malloc(strlen(input) + 1);
        data.footer = malloc(strlen("END") + 1);

        strcpy(data.header, "PTT");
        strcpy(data.action, "SEND");
        strcpy(data.data, input);
        strcpy(data.footer, "END");

        // Convertir mensaje a buffer para enviar
        char *msg_buffer = parseMessage(data);

        // Enviar el mensaje
        send(client_fd, msg_buffer, strlen(msg_buffer), 0);

        //Limpiar memoria
        free(data.header);
        free(data.action);
        free(data.data);
        free(data.footer);
        free(msg_buffer);
    }

    close(client_fd);
    close(socket_fd);
    return 0;
}



