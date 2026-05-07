#ifndef PROTOCOL_H
#define PROTOCOL_H

// Protocolo PTT v2 - Telemetry Transfer Protocol con autenticación

// Estructura de mensaje del protocolo
typedef struct {
    char header[4];   // "PTT"
    char action[12];  // Tipo de acción
    char data[150];   // Datos del mensaje
    char footer[4];   // "END"
} ProtocolMessage;

// Acciones del protocolo
#define ACTION_LOGIN      "LOGIN"       // Autenticación: username;password
#define ACTION_DATA       "DATA"        // Telemetría: speed=X;dir=Y;battery=Z;temp=W
#define ACTION_COMMAND    "COMMAND"     // Comando: SPEED UP, SLOW DOWN, etc.
#define ACTION_LIST       "LIST"        // Listar usuarios activos (admin)
#define ACTION_OK         "OK"          // Respuesta exitosa
#define ACTION_ERROR      "ERROR"       // Respuesta de error
#define ACTION_DENIED     "DENIED"      // Permiso denegado

// Códigos de respuesta
#define RESP_OK           "OK"
#define RESP_AUTH_OK      "AUTH_OK"
#define RESP_AUTH_FAIL    "AUTH_FAILED"
#define RESP_PERM_DENIED  "PERMISSION_DENIED"
#define RESP_CMD_OK       "COMMAND_OK"
#define RESP_CMD_FAIL     "COMMAND_FAILED"
#define RESP_INVALID      "INVALID_MESSAGE"

// Funciones del protocolo
void create_message(ProtocolMessage *msg, const char *action, const char *data);
void serialize_message(ProtocolMessage msg, char *buffer);
int parse_message(const char *raw, ProtocolMessage *msg);
void parse_login_data(const char *data, char *username, char *password);

#endif

