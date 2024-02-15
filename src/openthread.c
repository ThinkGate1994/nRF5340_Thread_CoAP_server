#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/openthread.h>
#include <openthread/thread.h>
#include <dk_buttons_and_leds.h>

#include "openthread.h"

LOG_MODULE_REGISTER(openthread);

#define OT_CONNECTION_LED DK_LED1

static void on_thread_state_changed(otChangedFlags flags, struct openthread_context *ot_context, void *user_data)
{
	if (flags & OT_CHANGED_THREAD_ROLE)
	{
		switch (otThreadGetDeviceRole(ot_context->instance))
		{
		case OT_DEVICE_ROLE_CHILD:
			LOG_INF("thread child\n");
		case OT_DEVICE_ROLE_ROUTER:
			LOG_INF("thread router\n");
		case OT_DEVICE_ROLE_LEADER:
			LOG_INF("thread leader\n");
			dk_set_led_on(OT_CONNECTION_LED);
			break;
		case OT_DEVICE_ROLE_DISABLED:
			LOG_INF("thread role disabled\n");
		case OT_DEVICE_ROLE_DETACHED:
			LOG_INF("thread role detached\n");
		default:
			dk_set_led_off(OT_CONNECTION_LED);
			break;
		}
	}
}

static struct openthread_state_changed_cb ot_state_chaged_cb = {.state_changed_cb = on_thread_state_changed};

void openthread_init()
{
	openthread_state_changed_cb_register(openthread_get_default_context(), &ot_state_chaged_cb);
	openthread_start(openthread_get_default_context());
}