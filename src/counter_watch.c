#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "mini-printf.h" 
	
enum {
  CMD_KEY = 0x0, // TUPLE_INTEGER
  COUNT_KEY = 0x1, // TUPLE_INTEGER
  TIME_KEY = 0x3, // TUPLE_INTEGER

};

enum {
  CMD_UP = 0x01,
  CMD_DOWN = 0x02,
};


#define MY_UUID {0xEC, 0x7E, 0xE5, 0xC6, 0x8D, 0xDF, 0x40, 0x89, 0xAA, 0x84, 0xC3, 0x39, 0x6A, 0x11, 0xCC, 0x99}
PBL_APP_INFO(MY_UUID,
             "Counter 3.5", "Pebble",
             3, 5, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_STANDARD_APP);

Window window;
TextLayer timeLayer;
static bool callbacks_registered;
static AppMessageCallbacksNode app_callbacks;
static int count=0;
static int  pebble_counter_error_state=0; 
//static unsigned int count_buffer_size=32;
static char count_text[32];
//static char timeText[] = "00:00:00"; // Needs to be static because it's used by the system later.

void updateDisplay() {

// PblTm currentTime;
 // get_time(&currentTime);
 // string_format_time(timeText, sizeof(timeText), "%T", &currentTime);
//  text_layer_set_text(&timeLayer, timeText);
	
 
	//count_text[0]= count+'0';
	//count_text[1]= 0; 
	mini_snprintf(count_text,32,"count=%d",count);
	text_layer_set_text(&timeLayer, count_text);
	
	//sprintf(buffer,"Counted=%d",count);
 
}
static void app_send_failed(DictionaryIterator* failed, AppMessageResult reason, void* context) {
  pebble_counter_error_state=-1;
}

static void app_received_msg(DictionaryIterator* received, void* context) {
  pebble_counter_error_state=1;
  vibes_short_pulse();
}

bool register_callbacks() {
	if (callbacks_registered) {
		if (app_message_deregister_callbacks(&app_callbacks) == APP_MSG_OK)
			callbacks_registered = false;
	}
	if (!callbacks_registered) {
		app_callbacks = (AppMessageCallbacksNode){
			.callbacks = {
				.out_failed = app_send_failed,
        .in_received = app_received_msg
			},
			.context = NULL
		};
		if (app_message_register_callbacks(&app_callbacks) == APP_MSG_OK) {
      callbacks_registered = true;
    }
	}
	return callbacks_registered;
}

static void send_count() {
  Tuplet value = TupletInteger(COUNT_KEY, count);
  PblTm currentTime;
  get_time(&currentTime);
  DictionaryIterator *iter;
  app_message_out_get(&iter);
  
  if (iter == NULL)
    return;
  
  dict_write_tuplet(iter, &value);
  dict_write_end(iter);
  
  app_message_out_send();
  app_message_out_release();
}

void up_single_click_handler(ClickRecognizerRef recognizer, Window *window) {
  (void)recognizer;
  (void)window;
  count++;
  updateDisplay();
 
}

void down_single_click_handler(ClickRecognizerRef recognizer, Window *window) {
  (void)recognizer;
  (void)window;
  count--;
  updateDisplay();
  
}
void select_single_click_handler(ClickRecognizerRef recognizer, Window *window) {
  (void)recognizer;
  (void)window;
   updateDisplay();
   send_count();
}


void click_config_provider(ClickConfig **config, Window *window) {
  (void)window;
  
  config[BUTTON_ID_UP]->click.handler = (ClickHandler) up_single_click_handler;
  config[BUTTON_ID_UP]->click.repeat_interval_ms = 100;
  
  config[BUTTON_ID_DOWN]->click.handler = (ClickHandler) down_single_click_handler;
  config[BUTTON_ID_DOWN]->click.repeat_interval_ms = 100;
	
  config[BUTTON_ID_SELECT]->click.handler = (ClickHandler) select_single_click_handler;

}

void handle_second_tick(AppContextRef ctx, PebbleTickEvent *t) {
  (void)t;
  (void)ctx;

  
  updateDisplay();
  

}

void handle_init(AppContextRef ctx) {
  (void)ctx;
  
  window_init(&window, "Counter");
  window_stack_push(&window, true /* Animated */);
  window_set_background_color(&window, GColorBlack);
  
  text_layer_init(&timeLayer, GRect(29, 54, 144-40 /* width */, 168-54 /* height */));
  text_layer_set_text_color(&timeLayer, GColorWhite);
  text_layer_set_background_color(&timeLayer, GColorClear);
  text_layer_set_font(&timeLayer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  
  handle_second_tick(ctx, NULL);
  
  layer_add_child(&window.layer, &timeLayer.layer);
  
  register_callbacks();
  window_set_click_config_provider(&window, (ClickConfigProvider) click_config_provider);
}

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .tick_info = {
      .tick_handler = &handle_second_tick,
      .tick_units = SECOND_UNIT
    },
    .messaging_info = {
      .buffer_sizes = {
        .inbound = 256,
        .outbound = 256,
      }
    }
  };
  app_event_loop(params, &handlers);
}