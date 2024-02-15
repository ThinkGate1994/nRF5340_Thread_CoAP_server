#include <zephyr/logging/log.h>
#include <zephyr/net/net_pkt.h>
#include <zephyr/net/net_l2.h>
#include <zephyr/net/openthread.h>
#include <openthread/coap.h>
#include <openthread/ip6.h>
#include <openthread/message.h>
#include <openthread/thread.h>

#include "ot_coap_server.h"

LOG_MODULE_REGISTER(ot_coap_server);

struct server_context
{
	struct otInstance *ot;
};

static struct server_context srv_context = {
	.ot = NULL};

static otCoapResource coap_resource = {
	.mUriPath = NULL,
	.mHandler = NULL,
	.mContext = NULL,
	.mNext = NULL,
};

static struct ot_coap_server_get_payload payload = {
	.buffer = "",
	.length = 0,
};

static otError storedata_response_send(otMessage *request_message, const otMessageInfo *message_info);

static void CoAP_request_handler(void *context, otMessage *message, const otMessageInfo *message_info)
{
	printk("in\n");

	ARG_UNUSED(context);

	if ((otCoapMessageGetType(message) != OT_COAP_TYPE_CONFIRMABLE) && (otCoapMessageGetType(message) != OT_COAP_TYPE_NON_CONFIRMABLE))
	{
		LOG_ERR("CoAP handler - Unexpected type of message");
		goto end;
	}

	if ((otCoapMessageGetCode(message) != OT_COAP_CODE_PUT) && (otCoapMessageGetCode(message) != OT_COAP_CODE_POST))
	{
		LOG_ERR("CoAP handler - Unexpected CoAP code");
		goto end;
	}

	payload.length = otMessageRead(message, otMessageGetOffset(message), payload.buffer, PAYLOAD_SIZE - 1);

	if (payload.length > 0)
	{
		payload.buffer[payload.length] = '\0';
		LOG_INF("CoAP message: %s , %d", payload.buffer, payload.length);
	}
	else
	{
		LOG_ERR("Error reading payload");
	}

	if (otCoapMessageGetType(message) == OT_COAP_TYPE_CONFIRMABLE)
	{
		storedata_response_send(message, message_info);
	}
end:
	return;
}

int ot_coap_server_get_payload(char *buffer, uint16_t *length)
{
	if (buffer == NULL || length == NULL)
	{
		return -1;
	}

	memcpy(buffer, payload.buffer, PAYLOAD_SIZE);
	*length = payload.length;

	return 0;
}

static otError storedata_response_send(otMessage *request_message, const otMessageInfo *message_info)
{
	otError error = OT_ERROR_NO_BUFS;
	otMessage *response;
	otInstance *p_instance = openthread_get_default_instance();
	const char *payload;
	// uint16_t payload_size;

	response = otCoapNewMessage(p_instance, NULL);
	if (response == NULL)
	{
		goto end;
	}

	otCoapMessageInit(response, OT_COAP_TYPE_NON_CONFIRMABLE,
					  OT_COAP_CODE_CONTENT);

	error = otCoapMessageSetToken(
		response, otCoapMessageGetToken(request_message),
		otCoapMessageGetTokenLength(request_message));
	if (error != OT_ERROR_NONE)
	{
		goto end;
	}

	error = otCoapMessageSetPayloadMarker(response);
	if (error != OT_ERROR_NONE)
	{
		goto end;
	}

	payload = "Test response payload";

	error = otMessageAppend(response, payload, strlen(payload));
	if (error != OT_ERROR_NONE)
	{
		goto end;
	}

	error = otCoapSendResponse(p_instance, response, message_info);

	// LOG_HEXDUMP_INF(payload, payload_size, "Sent provisioning response:");

end:
	if (error != OT_ERROR_NONE && response != NULL)
	{
		otMessageFree(response);
	}

	return error;
}

static void CoAP_default_handler(void *context, otMessage *message, const otMessageInfo *message_info)
{
	ARG_UNUSED(context);
	ARG_UNUSED(message);
	ARG_UNUSED(message_info);

	LOG_INF("Received CoAP message that does not match any request or resource");
}

void addIPv6Address(void)
{
	otInstance *myInstance = openthread_get_default_instance();
	otNetifAddress aAddress;
	const otMeshLocalPrefix *ml_prefix = otThreadGetMeshLocalPrefix(myInstance);
	uint8_t interfaceID[8] = {0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x01};

	memcpy(&aAddress.mAddress.mFields.m8[0], ml_prefix, 8);
	memcpy(&aAddress.mAddress.mFields.m8[8], interfaceID, 8);

	otError error = otIp6AddUnicastAddress(myInstance, &aAddress);

	if (error != OT_ERROR_NONE)
	{
		printk("addIPv6Address error: %d\n", error);
	}
}

int ot_coap_server_init(struct ot_coap_server_settings *settings)
{
	otError error;

	srv_context.ot = openthread_get_default_instance();
	if (!srv_context.ot)
	{
		LOG_ERR("There is no valid OpenThread instance");
		error = OT_ERROR_FAILED;
		goto end;
	}

	coap_resource.mUriPath = settings->coap_uri_path;
	coap_resource.mHandler = CoAP_request_handler;
	coap_resource.mContext = srv_context.ot;

	otCoapSetDefaultHandler(srv_context.ot, CoAP_default_handler, NULL);
	otCoapAddResource(srv_context.ot, &coap_resource);

	error = otCoapStart(srv_context.ot, settings->coap_port);
	if (error != OT_ERROR_NONE)
	{
		LOG_ERR("Failed to start OT CoAP. Error: %d", error);
		goto end;
	}

end:
	return error == OT_ERROR_NONE ? 0 : 1;
}
