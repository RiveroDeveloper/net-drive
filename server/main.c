#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>

#include "car.h"     // Lógica del carro
#include "auth.h"    // Sistema de autenticación
#include "protocol.h" // Protocolo PTT

#include "logger.h"
#define printf(...) log_printf(__VA_ARGS__)

// ====== Variables globales ======
#define MAX_CLIENTS 10
int clients_fds[MAX_CLIENTS];
int num_clients = 0;
struct CarState car;
pthread_mutex_t clients_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t car_lock = PTHREAD_MUTEX_INITIALIZER;

// Protocolo PTT v2 - Ahora definido en protocol.h

// ====== Gestión de clientes ======
void add_client(int client_fd) {
    pthread_mutex_lock(&clients_lock);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients_fds[i] == 0) {
            clients_fds[i] = client_fd;
            num_clients++;
            printf("[GESTIÓN] Cliente fd=%d agregado. Total: %d\n", client_fd, num_clients);
            break;
        }
    }
    pthread_mutex_unlock(&clients_lock);
}

void remove_client(int client_fd) {
    pthread_mutex_lock(&clients_lock);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients_fds[i] == client_fd) {
            clients_fds[i] = 0;
            num_clients--;
            printf("[GESTIÓN] Cliente fd=%d eliminado. Total: %d\n", client_fd, num_clients);
            break;
        }
    }
    pthread_mutex_unlock(&clients_lock);
}

// ====== Manejo de clientes (cada uno en su hilo) ======
void *handle_client(void *arg) {
    int client_fd = *(int *)arg;
    free(arg);

    char buffer[1024];
    bool connection_active = true;
    Session *session = NULL;

    int err_count = 0;

    // Obtener IP y puerto del cliente
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    getpeername(client_fd, (struct sockaddr *)&client_addr, &addr_len);
    char client_ip[16];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
    int client_port = ntohs(client_addr.sin_port);

    printf("[HILO] Cliente conectado (fd=%d) desde %s:%d\n", client_fd, client_ip, client_port);
    
    // Agregar cliente a la lista
    add_client(client_fd);

    while (connection_active) {
        memset(buffer, 0, sizeof(buffer));
        int bytes = read(client_fd, buffer, sizeof(buffer));

        if (bytes <= 0) {
            printf("[HILO] Cliente (fd=%d) desconectado.\n", client_fd);
            break;
        }

        // Parsear mensaje del protocolo
        ProtocolMessage msg;
        if (parse_message(buffer, &msg) < 0) {
            printf("[HILO] Mensaje inválido de fd=%d\n", client_fd);
            
            ProtocolMessage error_msg;
            create_message(&error_msg, ACTION_ERROR, RESP_INVALID);
            char error_buffer[170];
            serialize_message(error_msg, error_buffer);
            send(client_fd, error_buffer, strlen(error_buffer), 0);
            continue;
        }

        printf("[HILO] fd=%d Action=%s Data=%s\n", client_fd, msg.action, msg.data);

        // Comando STATUS
        if (strncmp(msg.action, "STATUS", 6) == 0) {
            printf("[STATUS] (fd=%d) %s\n", client_fd, msg.data);

            if(strncmp(msg.data, "OK", 2) == 0){
                err_count = 0;
                continue;
            }
            
            if(strncmp(msg.data, "ERROR", 5) == 0){
                err_count++;
                if(err_count > 2){
                    printf("[STATUS] Conexión finalizada (fd=%d) límite de errores alcanzado", client_fd);
                    remove_session(client_fd);
                }
            }
        }

        // ====== MANEJO DE AUTENTICACIÓN ======
        if (strcmp(msg.action, ACTION_LOGIN) == 0) {
            printf("[AUTH] Procesando LOGIN | Data: '%.50s'\n", msg.data);
            
            char username[MAX_USERNAME], password[MAX_PASSWORD];
            memset(username, 0, sizeof(username));
            memset(password, 0, sizeof(password));
            
            parse_login_data(msg.data, username, password);
            
            printf("[AUTH] Intentando autenticar: user='%s' pass='%s'\n", username, password);

            UserRole role;
            if (authenticate_user(username, password, &role)) {
                session = create_session(client_fd, username, role, client_ip, client_port);
                
                printf("[AUTH] Login exitoso: %s (role=%d) fd=%d\n", username, role, client_fd);
                
                char response_data[150];
                const char *role_str = (role == ROLE_ADMIN) ? "ADMIN" : "OBSERVER";
                snprintf(response_data, sizeof(response_data), "%s;role=%s;token=%s", 
                         RESP_AUTH_OK, role_str, session->token);
                
                ProtocolMessage response;
                create_message(&response, ACTION_OK, response_data);
                char resp_buffer[170];
                serialize_message(response, resp_buffer);
                send(client_fd, resp_buffer, strlen(resp_buffer), 0);
                
                printf("[AUTH] Respuesta enviada a fd=%d\n", client_fd);
            } else {
                printf("[AUTH] Login fallido: user='%s' pass='%s' fd=%d\n", username, password, client_fd);
                
                ProtocolMessage response;
                create_message(&response, ACTION_ERROR, RESP_AUTH_FAIL);
                char resp_buffer[170];
                serialize_message(response, resp_buffer);
                send(client_fd, resp_buffer, strlen(resp_buffer), 0);
            }
            continue;
        }

        // ====== VERIFICAR AUTENTICACIÓN PARA OTROS COMANDOS ======
        session = get_session_by_fd(client_fd);
        if (!session || !session->is_authenticated) {
            printf("[AUTH] Cliente no autenticado intenta acceder: fd=%d\n", client_fd);
            
            ProtocolMessage response;
            create_message(&response, ACTION_DENIED, "NOT_AUTHENTICATED");
            char resp_buffer[170];
            serialize_message(response, resp_buffer);
            send(client_fd, resp_buffer, strlen(resp_buffer), 0);
            continue;
        }

        // ====== MANEJO DE COMANDOS (solo ADMIN) ======
        if (strcmp(msg.action, ACTION_COMMAND) == 0) {
            if (session->role != ROLE_ADMIN) {
                printf("[AUTH] Cliente sin permisos intenta comando: %s fd=%d\n", session->username, client_fd);
                
                ProtocolMessage response;
                create_message(&response, ACTION_DENIED, RESP_PERM_DENIED);
                char resp_buffer[170];
                serialize_message(response, resp_buffer);
                send(client_fd, resp_buffer, strlen(resp_buffer), 0);
                continue;
            }

            // Ejecutar comando
            pthread_mutex_lock(&car_lock);
            updateCarTelemetry(&car, msg.data);
            pthread_mutex_unlock(&car_lock);

            printf("[COMMAND] Ejecutado '%s' por %s fd=%d\n", msg.data, session->username, client_fd);

            ProtocolMessage response;
            create_message(&response, ACTION_OK, RESP_CMD_OK);
            char resp_buffer[170];
            serialize_message(response, resp_buffer);
            send(client_fd, resp_buffer, strlen(resp_buffer), 0);
            continue;
        }

        // ====== LISTAR USUARIOS (solo ADMIN) ======
        if (strcmp(msg.action, ACTION_LIST) == 0) {
            if (session->role != ROLE_ADMIN) {
                ProtocolMessage response;
                create_message(&response, ACTION_DENIED, RESP_PERM_DENIED);
                char resp_buffer[170];
                serialize_message(response, resp_buffer);
                send(client_fd, resp_buffer, strlen(resp_buffer), 0);
                continue;
            }

            char users_list[150];
            get_active_users_list(users_list, sizeof(users_list));

            printf("[LIST] Usuarios solicitados por %s fd=%d\n", session->username, client_fd);

            ProtocolMessage response;
            create_message(&response, ACTION_OK, users_list);
            char resp_buffer[170];
            serialize_message(response, resp_buffer);
            send(client_fd, resp_buffer, strlen(resp_buffer), 0);
            continue;
        }

        // Comando EXIT
        if (strncmp(msg.data, "EXIT", 4) == 0) {
            printf("[HILO] Cliente %s pidió salir.\n", session->username);
            break;
        }       

    }

    // Remover sesión y cliente
    if (session) {
        printf("[HILO] Cerrando sesión de %s (fd=%d)\n", session->username, client_fd);
        remove_session(client_fd);
    }
    remove_client(client_fd);
    close(client_fd);
    pthread_exit(NULL);
}


void *send_telemetry(void *arg) {
    (void)arg; // No se usa

    while (1) {
        sleep(10); // Enviar cada 10 segundos (según requerimiento)

        // Preparar telemetría
        char data[150];
        pthread_mutex_lock(&car_lock);
        generateCarTelemetry(car, data, sizeof(data));
        pthread_mutex_unlock(&car_lock);

        ProtocolMessage telemetry;
        create_message(&telemetry, ACTION_DATA, data);

        char msg_buffer[170];
        serialize_message(telemetry, msg_buffer);

        // Enviar solo a clientes autenticados
        pthread_mutex_lock(&clients_lock);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int fd = clients_fds[i];

            // Verificar que esté autenticado
            Session *session = get_session_by_fd(fd);
            if (fd > 0 && session && session->is_authenticated) {
                if (send(fd, msg_buffer, strlen(msg_buffer), 0) < 0) {
                    perror("[TELEMETRIA] Error enviando a cliente");
                } else {
                    printf("[TELEMETRIA] Enviada a %s (fd=%d)\n", session->username, fd);
                }
        }
        }
        pthread_mutex_unlock(&clients_lock);
    }

    pthread_exit(NULL);
}

// ====== MAIN ======
int main(int argc, char *argv[]) {
    int port = 2000;  // Puerto por defecto
    char logfile[256] = "server.log";  // Archivo de logs por defecto
    
    // Procesar argumentos de línea de comandos
    if (argc >= 2) {
        port = atoi(argv[1]);
        if (port <= 0 || port > 65535) {
            fprintf(stderr, "Error: Puerto inválido. Debe estar entre 1 y 65535\n");
            fprintf(stderr, "Uso: %s <puerto> <archivo_logs>\n", argv[0]);
            fprintf(stderr, "Ejemplo: %s 2000 server.log\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    
    if (argc >= 3) {
        strncpy(logfile, argv[2], sizeof(logfile) - 1);
        logfile[sizeof(logfile) - 1] = '\0';
    }
    
    // Inicializar sistema de logging
    init_logger(logfile);
    
    // Inicializar array de clientes
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients_fds[i] = 0;
    }

    initCar(&car);
    init_auth_system();
    
    printf("==============================================\n");
    printf("   SERVIDOR NETDRIVE - PTT v2\n");
    printf("==============================================\n");
    printf("Puerto: %d\n", port);
    printf("Archivo de logs: %s\n", logfile);
    printf("Usuarios disponibles:\n");
    printf("  - admin / admin123 (ADMIN)\n");
    printf("  - observer / observer123 (OBSERVER)\n");
    printf("  - user1 / pass1 (OBSERVER)\n");
    printf("  - root / root (ADMIN)\n");
    printf("==============================================\n\n");

    int server_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket failed"); exit(EXIT_FAILURE); }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("[SERVIDOR] Escuchando en el puerto %d...\n", port);

    pthread_t broadcast_thread; // Se crea un hilo encargado de enviar telemetría a todos los clientes
    pthread_create(&broadcast_thread, NULL, send_telemetry, NULL);
    pthread_detach(broadcast_thread); 

    //pthread_t logwriter_thread;
    //pthread_create()

    while (1) {

        int *client_fd = malloc(sizeof(int));
        *client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);

        if (*client_fd < 0) {
            perror("accept failed");
            free(client_fd);
            continue;
        }

        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, client_fd);
        pthread_detach(tid);

        printf("[SERVIDOR] Nuevo cliente conectado (fd=%d)\n", *client_fd);
    }

    close(server_fd);
    return 0;
}