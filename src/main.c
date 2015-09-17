#include <pebble.h>
#define KEY_BG 0
static Window *s_main_window;
static TextLayer *s_time_layer;

static BitmapLayer *s_bitmap_layer;
static GBitmap *s_bg_bitmap;

static GBitmap *s_dog_bg;
static GBitmap *s_cat_bg;
static GBitmap *s_bird_bg;

static GFont *s_time_font;

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00 XX";
  // Write the current hour
  strftime(buffer, sizeof(buffer), "%l:%M %p", tick_time);
  
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
}
static void main_window_load(Window *window) {
  //Initialize the backgrounds we can select
  s_dog_bg = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BG_DOG);
  s_cat_bg = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BG_CAT);
  s_bird_bg = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BG_BIRD);
  
  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(0, 0, 144, 32));
  
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  
  // Improve the layout to be more like a watchface
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MULI_24));
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  //Check for saved background option
  char facebg[20];
  persist_read_string(KEY_BG, facebg, sizeof(facebg));
  
  if(strcmp(facebg,"dog") == 0){
    s_bg_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BG_DOG);
  }else if(strcmp(facebg,"cat") == 0){
    s_bg_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BG_CAT);
  }else if(strcmp(facebg,"bird") == 0){
    s_bg_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BG_BIRD);
  }
  
  //Declare a bitmap layer
  //static BitmapLayer *s_bitmap_layer;
  
  //Create bitmap layer and set it to show the GBitmap
  s_bitmap_layer = bitmap_layer_create(GRect(0,0,144,168));
  bitmap_layer_set_bitmap(s_bitmap_layer,s_bg_bitmap);
  
  //Add the bitmaplayer as a child layer to the window:
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bitmap_layer));
  
  // Add text layer as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
}
static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  
  //Destroy bitmap layer
  bitmap_layer_destroy(s_bitmap_layer);
  gbitmap_destroy(s_bg_bitmap);
  fonts_unload_custom_font(s_time_font);
}
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}
static void in_recv_handler(DictionaryIterator *iterator, void *context){
  //Get Tuple
  Tuple *t = dict_read_first(iterator);
  if(t)
  {
    switch(t->key)
    {
    case KEY_BG:
      //strcmp compares binary value of two strings, 0 means the two strings are equal
      if(strcmp(t->value->cstring, "dog") == 0){
        bitmap_layer_set_bitmap(s_bitmap_layer, s_dog_bg);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Dog selected");
        persist_write_string(KEY_BG, "dog");
      }
      else if(strcmp(t->value->cstring, "cat") == 0){
        bitmap_layer_set_bitmap(s_bitmap_layer, s_cat_bg);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Cat selected");
        persist_write_string(KEY_BG, "cat");
      }
      else if(strcmp(t->value->cstring, "bird") == 0){
        bitmap_layer_set_bitmap(s_bitmap_layer, s_bird_bg);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Bird selected");
        persist_write_string(KEY_BG, "bird");
      }
      else{
        APP_LOG(APP_LOG_LEVEL_DEBUG, "You shouldn't see this in the app log. Something went wrong.");
      }
    }
  }
}
static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Make sure the time is displayed from the start
  update_time();
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  app_message_register_inbox_received((AppMessageInboxReceived) in_recv_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

  window_stack_push(s_main_window, true);
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
