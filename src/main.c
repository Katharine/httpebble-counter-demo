#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "http.h"
#include "itoa.h"

#define COUNTER_COOKIE 1
#define REQUEST_ID 45
	
PBL_APP_INFO_SIMPLE(HTTP_UUID, "Counter Demo", "Demo Corp", 1 /* App version */);
Window window;

TextLayer textLayer;
int gCounterValue = 0;

void update_count() {
	text_layer_set_text(&textLayer, itoa(gCounterValue));
}

// Modify these common button handlers

void up_single_click_handler(ClickRecognizerRef recognizer, Window *window) {
	++gCounterValue;
	update_count();
}


void down_single_click_handler(ClickRecognizerRef recognizer, Window *window) {
	--gCounterValue;
	update_count();
}


void select_single_click_handler(ClickRecognizerRef recognizer, Window *window) {
	http_cookie_set_int32(0, COUNTER_COOKIE, gCounterValue);
}

void select_long_click_handler(ClickRecognizerRef recognizer, Window *window) {
	http_cookie_delete(0, COUNTER_COOKIE);
}

void click_config_provider(ClickConfig **config, Window *window) {
	config[BUTTON_ID_SELECT]->click.handler = (ClickHandler) select_single_click_handler;
	config[BUTTON_ID_SELECT]->long_click.handler = (ClickHandler)select_long_click_handler;

	config[BUTTON_ID_UP]->click.handler = (ClickHandler) up_single_click_handler;
	config[BUTTON_ID_UP]->click.repeat_interval_ms = 100;

	config[BUTTON_ID_DOWN]->click.handler = (ClickHandler) down_single_click_handler;
	config[BUTTON_ID_DOWN]->click.repeat_interval_ms = 100;
}

void failed(int32_t request_id, int http_status, void* context) {
	text_layer_set_text(&textLayer, itoa(http_status));
}

void cookie_get(int32_t request_id, DictionaryIterator* iter, void* context) {
	if(request_id != REQUEST_ID) return;
	Tuple *counter = dict_find(iter, COUNTER_COOKIE);
	if(counter) {
		gCounterValue = counter->value->int32;
	} else {
		gCounterValue = 0;
	}
	update_count();
}

void cookie_set(int32_t request_id, bool successful, void* context) {
	if(successful) {
		text_layer_set_text(&textLayer, "Value saved");
	} else {
		text_layer_set_text(&textLayer, "Value not saved.");
	}
}

void cookie_delete(int32_t request_id, bool successful, void* context) {
	if(successful) {
		text_layer_set_text(&textLayer, "Value deleted");
	} else {
		text_layer_set_text(&textLayer, "Value not deleted.");
	}
}

// Standard app initialisation
void handle_init(AppContextRef ctx) {
	window_init(&window, "Counter Demo");
	window_stack_push(&window, true /* Animated */);
	
	text_layer_init(&textLayer, window.layer.frame);
	text_layer_set_text(&textLayer, "Loading...");
	text_layer_set_font(&textLayer, fonts_get_system_font(FONT_KEY_GOTHAM_30_BLACK));
	layer_add_child(&window.layer, &textLayer.layer);
	
	// Attach our desired button functionality
	window_set_click_config_provider(&window, (ClickConfigProvider) click_config_provider);
	
	// Set up HTTP callbacks.
	http_register_callbacks((HTTPCallbacks){
		.failure=failed,
		.cookie_batch_get=cookie_get,
		.cookie_set=cookie_set,
		.cookie_delete=cookie_delete
	}, NULL);
	http_cookie_get(REQUEST_ID, COUNTER_COOKIE);
}

void pbl_main(void *params) {
	PebbleAppHandlers handlers = {
		.init_handler = &handle_init,
		.messaging_info = {
			.buffer_sizes = {
				.inbound = 256,
				.outbound = 256,
			}
		},
	};
	app_event_loop(params, &handlers);
}
