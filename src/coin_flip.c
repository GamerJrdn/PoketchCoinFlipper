#include <pebble.h>

static Window *window;
static TextLayer *text_layer;
static BitmapLayer *s_background_layer;
static BitmapLayer *s_coin_layer;
static GBitmap *s_background_bitmap;
static GBitmap *s_coin_bitmap1;
static GBitmap *s_coin_bitmap2;
static GBitmap *s_coin_bitmap3;
static PropertyAnimation *s_animate_bg;
static PropertyAnimation *s_animate_bg_back;
static GRect s_start_rect;
static GRect s_end_rect;
static int frameCount;
static time_t lastStep;
static AppTimer * timer_handle;
static bool finishSide;
static bool flipping;
static int offset;
static bool startingSide;

//Update function for the app
static void update_time()
{
  //Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  double dtime = difftime(lastStep,temp);
  int roundTime = (dtime*100);
  
  //Have the coin update every 4 update
  frameCount++;
  if(frameCount > 4)
  {
    frameCount = 1;
  }
  //Change the next picture in the animation
  switch(frameCount){
    case 1:
    bitmap_layer_set_bitmap(s_coin_layer, s_coin_bitmap1);
    break;
    case 3:
    bitmap_layer_set_bitmap(s_coin_layer, s_coin_bitmap3);
    break;
    default:
    bitmap_layer_set_bitmap(s_coin_layer, s_coin_bitmap2);
    break;
  }

  //Set up the update to be called again
  timer_handle = app_timer_register(100, update_time, NULL);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
  update_time();
}

//Stop the coin from flipping
static void stop_coin_flip(){
  //tick_timer_service_unsubscribe();
  app_timer_cancel(timer_handle);
  if(finishSide){
    bitmap_layer_set_bitmap(s_coin_layer, s_coin_bitmap1);
    startingSide = true;
  }else{
    bitmap_layer_set_bitmap(s_coin_layer, s_coin_bitmap3);
    startingSide = false;
  }
  flipping = false;
}

void animation_started(Animation *animation, void *data) {
   // Animation started!
  //Register with TickTimerService
  //tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  timer_handle = app_timer_register(100, update_time, NULL);
  
  lastStep = time(NULL);
  localtime(&lastStep);
  srand(lastStep);
}

void animation_stopped(Animation *animation, bool finished, void *data) {
  // Animation stopped!
  s_animate_bg_back = property_animation_create_layer_frame(bitmap_layer_get_layer(s_background_layer), &s_end_rect, &s_start_rect);
  animation_set_duration((Animation*) s_animate_bg_back,1050+offset);
  
   animation_set_handlers((Animation*) s_animate_bg_back, (AnimationHandlers) {
    .stopped = (AnimationStoppedHandler) stop_coin_flip,
  }, NULL);
  
  animation_schedule((Animation*) s_animate_bg_back);
}

//Called when the select button is clicked
static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  //Check if the coin isn't already flipping
  if(!flipping){
    //The coin is now flipping
    flipping = true;
    
    //Randomly determine if it will land on head or tails and adjust the animation time accordingly
    lastStep = time(NULL);
    srand(lastStep);
    finishSide = (bool)(rand()%2);
    if(finishSide == startingSide){
      offset = 200;
    }
    
    //Set up the background's start and end position and animation duration
    s_animate_bg = property_animation_create_layer_frame(bitmap_layer_get_layer(s_background_layer), &s_start_rect, &s_end_rect);
    animation_set_duration((Animation*) s_animate_bg,1050+offset);
    
     //Set handlers to listen for the start and stop events
    animation_set_handlers((Animation*) s_animate_bg, (AnimationHandlers) {
      .started = (AnimationStartedHandler) animation_started,
      .stopped = (AnimationStoppedHandler) animation_stopped,
    }, NULL);
    
    //Schedule the background animation
    animation_schedule((Animation*) s_animate_bg);
  }
}

//Bind the Pebble buttons to a function
static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void window_load(Window *window) {
  //Create GBitmaps and set in Bitmaplayer
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  s_coin_bitmap1 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_COIN1);
  s_coin_bitmap2 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_COIN2);
  s_coin_bitmap3 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_COIN3);
  
  //Place bitmaps into the bitmaplayers
  s_background_layer = bitmap_layer_create(GRect(0, -30, 144, 168));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
  
  s_coin_layer = bitmap_layer_create(GRect(31, 71, 80, 73));
  bitmap_layer_set_bitmap(s_coin_layer, s_coin_bitmap1);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_coin_layer));
  
  //Set up start and end frames for background movement
  s_start_rect = GRect(0, -30, 144, 168);
  s_end_rect = GRect(0, 150, 144, 168);
  
  //Set up the start frame for the coin animation
  frameCount = 1;
  
  //Coin doesn't start out animated
  flipping = false;
  
  //Coin starts on heads
  startingSide = true;
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
}

static void init(void) {
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
	.load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}