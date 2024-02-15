#ifndef __OT_COAP_SERVER_H__
#define __OT_COAP_SERVER_H__

#include <stdint.h>

#define PAYLOAD_SIZE 256

struct ot_coap_server_get_payload
{
    char buffer[PAYLOAD_SIZE];
    uint16_t length;
};

struct ot_coap_server_settings
{
    const char *coap_uri_path;
    uint16_t coap_port;
};

int ot_coap_server_init(struct ot_coap_server_settings *settings);
int ot_coap_server_get_payload(char *buffer, uint16_t *length);

#endif
