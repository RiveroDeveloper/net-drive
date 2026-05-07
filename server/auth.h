#ifndef AUTH_H
#define AUTH_H

#include <stdbool.h>
#include <time.h>

#define MAX_USERNAME 32
#define MAX_PASSWORD 64
#define MAX_TOKEN 64
#define MAX_SESSIONS 10

// Roles de usuario
typedef enum {
    ROLE_NONE = 0,
    ROLE_OBSERVER = 1,
    ROLE_ADMIN = 2
} UserRole;

// Usuario registrado en el sistema
typedef struct {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    UserRole role;
} User;

// Sesión activa de un cliente
typedef struct {
    int client_fd;
    char username[MAX_USERNAME];
    char token[MAX_TOKEN];
    UserRole role;
    char ip[16];
    int port;
    time_t login_time;
    bool is_authenticated;
} Session;

// Base de datos de usuarios hardcodeada (en producción sería en archivo/DB)
extern User users_db[];
extern int users_db_count;

// Array de sesiones activas
extern Session active_sessions[MAX_SESSIONS];

// Funciones de autenticación
bool authenticate_user(const char *username, const char *password, UserRole *role);
char* generate_token(const char *username);
Session* create_session(int client_fd, const char *username, UserRole role, const char *ip, int port);
Session* get_session_by_fd(int client_fd);
Session* get_session_by_token(const char *token);
void remove_session(int client_fd);
void init_auth_system();
int get_active_users_count();
void get_active_users_list(char *buffer, size_t size);

#endif

