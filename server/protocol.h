#ifndef PROTOCOL_H
#define PROTOCOL_H

// PTT v2 — telemetry transfer with authentication

typedef struct {
    char header[4];
    char action[12];
    char data[150];
    char footer[4];
} ProtocolMessage;

#define ACTION_LOGIN      "LOGIN"       // username;password
#define ACTION_DATA       "DATA"       // speed=X;dir=Y;battery=Z;temp=W
#define ACTION_COMMAND    "COMMAND"
#define ACTION_LIST       "LIST"        // active users (admin)
#define ACTION_OK         "OK"
#define ACTION_ERROR      "ERROR"
#define ACTION_DENIED     "DENIED"

#define RESP_OK           "OK"
#define RESP_AUTH_OK      "AUTH_OK"
#define RESP_AUTH_FAIL    "AUTH_FAILED"
#define RESP_PERM_DENIED  "PERMISSION_DENIED"
#define RESP_CMD_OK       "COMMAND_OK"
#define RESP_CMD_FAIL     "COMMAND_FAILED"
#define RESP_INVALID      "INVALID_MESSAGE"

void create_message(ProtocolMessage *msg, const char *action, const char *data);
void serialize_message(ProtocolMessage msg, char *buffer);
int parse_message(const char *raw, ProtocolMessage *msg);
void parse_login_data(const char *data, char *username, char *password);

#endif
