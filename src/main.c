#include <zephyr/kernel.h>
#include <dk_buttons_and_leds.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/openthread.h>
#include <openthread/thread.h>

#include "ot_coap_server.h"
#include "openthread.h"

LOG_MODULE_REGISTER(main_c, CONFIG_COAP_SERVER_LOG_LEVEL);

int main(void)
{
	int ret;

	LOG_INF("Start CoAP-server sample");

	ret = dk_leds_init();
	if (ret)
	{
		LOG_ERR("Could not initialize leds, err code: %d", ret);
		goto end;
	}

	addIPv6Address();
	openthread_init();

	// Initialize CoAP server settings
	struct ot_coap_server_settings coap_settings = {
		.coap_uri_path = "data",
		.coap_port = 5683, // #default 5683, You can replace it with your desired port
	};

	ret = ot_coap_server_init(&coap_settings);
	if (ret)
	{
		LOG_ERR("Could not initialize OpenThread CoAP");
		goto end;
	}

	char payload_buffer[PAYLOAD_SIZE];
	uint16_t payload_length;
	while (1)
	{
		int result = ot_coap_server_get_payload(payload_buffer, &payload_length);

		if (result == 0)
		{
			// printk("CoAP Payload: %s\n", payload_buffer);
		}
		else
		{
			printk("Failed to get CoAP payload\n");
		}
		k_msleep(1000);
	}

end:

	return 0;
}
