#include "protocol.h"
#include <string.h>
#include <stdio.h>

#include "logger.h"
#define printf(...) log_printf(__VA_ARGS__)


void create_message(ProtocolMessage *msg, const char *action, const char *data) {
    strncpy(msg->header, "PTT", 3);
    msg->header[3] = '\0';
    
    strncpy(msg->action, action, 11);
    msg->action[11] = '\0';
    
    strncpy(msg->data, data, 149);
    msg->data[149] = '\0';
    
    strncpy(msg->footer, "END", 3);
    msg->footer[3] = '\0';
}

void serialize_message(ProtocolMessage msg, char *buffer) {
    char action_padded[13];
    snprintf(action_padded, sizeof(action_padded), "%-12s", msg.action);
    action_padded[12] = '\0';
    
    char data_padded[151];
    snprintf(data_padded, sizeof(data_padded), "%-150s", msg.data);
    data_padded[150] = '\0';
    
    snprintf(buffer, 170, "PTT%s%sEND", action_padded, data_padded);
}

int parse_message(const char *raw, ProtocolMessage *msg) {
    const char *ptt = strstr(raw, "PTT");
    const char *end = strstr(raw, "END");
    
    if (!ptt || !end || end <= ptt) {
        return -1;
    }
    
    memcpy(msg->action, ptt + 3, 12);
    msg->action[11] = '\0';
    
    for (int i = 11; i >= 0; i--) {
        if (msg->action[i] == ' ' || msg->action[i] == '\0') {
            msg->action[i] = '\0';
        } else {
            break;
        }
    }
    
    const char *data_start = ptt + 3 + 12;
    int data_len = end - data_start;
    if (data_len > 149) data_len = 149;
    if (data_len > 0) {
        memcpy(msg->data, data_start, data_len);
        msg->data[data_len] = '\0';
    } else {
        msg->data[0] = '\0';
    }
    
    return 0;
}

void parse_login_data(const char *data, char *username, char *password) {
    printf("[PARSE_LOGIN] Data: '%s' (len=%zu)\n", data, strlen(data));
    
    const char *semicolon = strchr(data, ';');
    
    if (semicolon) {
        int username_len = semicolon - data;
        if (username_len > 31) username_len = 31;
        
        strncpy(username, data, username_len);
        username[username_len] = '\0';
        
        const char *pwd_start = semicolon + 1;
        int pwd_len = 0;
        while (pwd_start[pwd_len] && pwd_start[pwd_len] != ' ' && 
               pwd_start[pwd_len] != '\n' && pwd_start[pwd_len] != '\r' && 
               pwd_start[pwd_len] != '\0' && pwd_len < 63) {
            password[pwd_len] = pwd_start[pwd_len];
            pwd_len++;
        }
        password[pwd_len] = '\0';
        
        printf("[PARSE_LOGIN] Username: '%s', Password: '%s'\n", username, password);
    } else {
        printf("[PARSE_LOGIN] No ';' in data\n");
        username[0] = '\0';
        password[0] = '\0';
    }
}
