#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer, *s_date_layer;
static int s_battery_level;
static Layer *s_battery_layer;

//Battery level
static void battery_callback(BatteryChargeState state) {
  // Record the new battery level
  s_battery_level = state.charge_percent;
}

static void battery_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Find the width of the bar
  int width = (int)(float)(((float)s_battery_level / 100.0F) * 114.0F);

  // Draw the background
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Draw the bar
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h), 0, GCornerNone);
}


static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00:00";
	static char date_buffer[16];

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strftime(buffer, sizeof("00:00:00"), "%H:%M:%S", tick_time);
  } else {
    // Use 12 hour format
    strftime(buffer, sizeof("00:00:00"), "%I:%M:%S", tick_time);
  }

	//Date
	strftime(date_buffer, sizeof(date_buffer), "%a %d %b", tick_time);
	
	
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
	text_layer_set_text(s_date_layer, date_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {

	update_time();
}

static void main_window_load(Window *window) {
	
	//Create time TextLayer
	s_time_layer = text_layer_create(GRect(0, 10, 144, 50));
	text_layer_set_background_color(s_time_layer, GColorClear);
	text_layer_set_text_color(s_time_layer, GColorBlack);
	text_layer_set_text(s_time_layer, "00:00:00");
	
	//Improve the layout to be more like a watchface
	text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_LECO_32_BOLD_NUMBERS));
	text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
	
	// Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));

	
	//Create date TextLayer
	s_date_layer = text_layer_create(GRect(0, 40, 144, 50));
	text_layer_set_background_color(s_date_layer, GColorClear);
	text_layer_set_text_color(s_date_layer, GColorBlack);
	text_layer_set_text(s_date_layer, "Sun, Sep 27");
	
	//Improve the layout to be more like a watchface
	text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
	text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
	
	// Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
	
	// Create battery meter Layer
	s_battery_layer = layer_create(GRect(14, 65, 115, 2));
	layer_set_update_proc(s_battery_layer, battery_update_proc);

	// Add to Window
	layer_add_child(window_get_root_layer(window), s_battery_layer);
	
	// Update meter
	layer_mark_dirty(s_battery_layer);
	
}

static void main_window_unload(Window *window) {
	
	// Destroy TextLayer
  text_layer_destroy(s_time_layer);
	
	// Destroy TextLayer
  text_layer_destroy(s_date_layer);
	
	//Destroy Battery level
	layer_destroy(s_battery_layer);
}

static void init() {
  
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
	
	//Background Color of main window
	window_set_background_color(s_main_window, GColorVividCerulean);

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
	
	// Register with TickTimerService
	tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
	
	// Register for battery level updates
	battery_state_service_subscribe(battery_callback);
	
	// Ensure battery level is displayed from the start
	battery_callback(battery_state_service_peek());

}

static void deinit() {
  
  // Destroy Window
    window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

