#include "auth.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "logger.h"

#define printf(...) log_printf(__VA_ARGS__)

// Base de datos de usuarios (hardcodeada para simplificar)
User users_db[] = {
    {"admin", "admin123", ROLE_ADMIN},
    {"observer", "observer123", ROLE_OBSERVER},
    {"user1", "pass1", ROLE_OBSERVER},
    {"root", "root", ROLE_ADMIN}
};
int users_db_count = 4;

// Sesiones activas
Session active_sessions[MAX_SESSIONS] = {0};
pthread_mutex_t sessions_lock = PTHREAD_MUTEX_INITIALIZER;

// Inicializar sistema de autenticación
void init_auth_system() {
    pthread_mutex_lock(&sessions_lock);
    for (int i = 0; i < MAX_SESSIONS; i++) {
        active_sessions[i].client_fd = 0;
        active_sessions[i].is_authenticated = false;
    }
    pthread_mutex_unlock(&sessions_lock);
}

// Autenticar usuario
bool authenticate_user(const char *username, const char *password, UserRole *role) {
    for (int i = 0; i < users_db_count; i++) {
        if (strcmp(users_db[i].username, username) == 0 &&
            strcmp(users_db[i].password, password) == 0) {
            *role = users_db[i].role;
            return true;
        }
    }
    return false;
}

// Generar token simple (en producción usar UUID o JWT)
char* generate_token(const char *username) {
    static char token[MAX_TOKEN];
    snprintf(token, MAX_TOKEN, "TKN_%s_%ld", username, time(NULL));
    return token;
}

// Crear sesión
Session* create_session(int client_fd, const char *username, UserRole role, const char *ip, int port) {
    pthread_mutex_lock(&sessions_lock);
    
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (active_sessions[i].client_fd == 0) {
            active_sessions[i].client_fd = client_fd;
            strncpy(active_sessions[i].username, username, MAX_USERNAME - 1);
            strncpy(active_sessions[i].token, generate_token(username), MAX_TOKEN - 1);
            active_sessions[i].role = role;
            strncpy(active_sessions[i].ip, ip, 15);
            active_sessions[i].port = port;
            active_sessions[i].login_time = time(NULL);
            active_sessions[i].is_authenticated = true;
            
            pthread_mutex_unlock(&sessions_lock);
            return &active_sessions[i];
        }
    }
    
    pthread_mutex_unlock(&sessions_lock);
    return NULL;
}

// Obtener sesión por FD
Session* get_session_by_fd(int client_fd) {
    pthread_mutex_lock(&sessions_lock);
    
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (active_sessions[i].client_fd == client_fd && active_sessions[i].is_authenticated) {
            pthread_mutex_unlock(&sessions_lock);
            return &active_sessions[i];
        }
    }
    
    pthread_mutex_unlock(&sessions_lock);
    return NULL;
}

// Obtener sesión por token
Session* get_session_by_token(const char *token) {
    pthread_mutex_lock(&sessions_lock);
    
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (active_sessions[i].is_authenticated && 
            strcmp(active_sessions[i].token, token) == 0) {
            pthread_mutex_unlock(&sessions_lock);
            return &active_sessions[i];
        }
    }
    
    pthread_mutex_unlock(&sessions_lock);
    return NULL;
}

// Remover sesión
void remove_session(int client_fd) {
    pthread_mutex_lock(&sessions_lock);
    
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (active_sessions[i].client_fd == client_fd) {
            memset(&active_sessions[i], 0, sizeof(Session));
            break;
        }
    }
    
    pthread_mutex_unlock(&sessions_lock);
}

// Contar usuarios activos
int get_active_users_count() {
    pthread_mutex_lock(&sessions_lock);
    
    int count = 0;
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (active_sessions[i].is_authenticated) {
            count++;
        }
    }
    
    pthread_mutex_unlock(&sessions_lock);
    return count;
}

// Obtener lista de usuarios activos (formato CSV: user:role:ip,user:role:ip)
void get_active_users_list(char *buffer, size_t size) {
    pthread_mutex_lock(&sessions_lock);
    
    int offset = 0;
    bool first = true;
    
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (active_sessions[i].is_authenticated) {
            if (!first) {
                offset += snprintf(buffer + offset, size - offset, ",");
            }
            
            const char *role_str = (active_sessions[i].role == ROLE_ADMIN) ? "ADMIN" : "OBSERVER";
            offset += snprintf(buffer + offset, size - offset,
                "%s:%s:%s",
                active_sessions[i].username,
                role_str,
                active_sessions[i].ip
            );
            first = false;
        }
    }
    
    // Si no hay usuarios, poner un mensaje
    if (offset == 0) {
        snprintf(buffer, size, "user:NONE:none");
    }
    
    printf("[AUTH] Lista de usuarios generada: '%s'\n", buffer);
    
    pthread_mutex_unlock(&sessions_lock);
}

