/*

  Because it's not a *cough* Brawn watch. :)

 */

#include <pebble.h>


Window *window;

GBitmap *background_image;

GBitmap *hour_hand_image;
GBitmap *minute_hand_image;
GBitmap *second_hand_image;

GBitmap *center_circle_white_image;
GBitmap *center_circle_black_image;

BitmapLayer *background_image_container;

RotBitmapLayer *hour_hand_image_container;
RotBitmapLayer *minute_hand_image_container;
RotBitmapLayer *second_hand_image_container;

BitmapLayer *center_circle_white_image_container;
BitmapLayer *center_circle_black_image_container;


void set_hand_angle(RotBitmapLayer *hand_image_container, unsigned int hand_angle) {

  signed short x_fudge = 0;
  signed short y_fudge = 0;


  rot_bitmap_layer_set_angle(hand_image_container, TRIG_MAX_ANGLE * hand_angle / 360);

  //
  // Due to rounding/centre of rotation point/other issues of fitting
  // square pixels into round holes by the time hands get to 6 and 9
  // o'clock there's off-by-one pixel errors.
  //
  // The `x_fudge` and `y_fudge` values enable us to ensure the hands
  // look centred on the minute marks at those points. (This could
  // probably be improved for intermediate marks also but they're not
  // as noticable.)
  //
  // I think ideally we'd only ever calculate the rotation between
  // 0-90 degrees and then rotate again by 90 or 180 degrees to
  // eliminate the error.
  //
  if (hand_angle == 180) {
    x_fudge = -1;
  } else if (hand_angle == 270) {
    y_fudge = -1;
  }

  // (144 = screen width, 168 = screen height)
  GRect frame = layer_get_frame(bitmap_layer_get_layer((BitmapLayer *)hand_image_container));
  frame.origin.x = (144/2) - (frame.size.w/2) + x_fudge;
  frame.origin.y = (168/2) - (frame.size.h/2) + y_fudge;

  layer_set_frame(bitmap_layer_get_layer((BitmapLayer *)hand_image_container), frame);
  layer_mark_dirty(bitmap_layer_get_layer((BitmapLayer *)hand_image_container));
}


void update_hand_positions(struct tm *t) {

  set_hand_angle(hour_hand_image_container, ((t->tm_hour % 12) * 30) + (t->tm_min/2)); // ((((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6));

  set_hand_angle(minute_hand_image_container, t->tm_min * 6);

  set_hand_angle(second_hand_image_container, t->tm_sec * 6);
}


void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {

  update_hand_positions(tick_time);
}


void handle_init() {

  window = window_create();
  window_stack_push(window, true);

  background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);

  hour_hand_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_HOUR_HAND);
  minute_hand_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MINUTE_HAND);
  second_hand_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SECOND_HAND);

  center_circle_white_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CENTER_CIRCLE_WHITE);
  center_circle_black_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CENTER_CIRCLE_BLACK);

  // Set up a layer for the static watch face background
  background_image_container = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(background_image_container, background_image);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(background_image_container));


  // Set up a layer for the hour hand
  hour_hand_image_container = rot_bitmap_layer_create(hour_hand_image);

  rot_bitmap_set_compositing_mode(hour_hand_image_container, GCompOpClear);

  rot_bitmap_set_src_ic(hour_hand_image_container, GPoint(4, 44));

  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer((BitmapLayer *)hour_hand_image_container));


  // Set up a layer for the minute hand
  minute_hand_image_container = rot_bitmap_layer_create(minute_hand_image);

  rot_bitmap_set_compositing_mode(minute_hand_image_container, GCompOpClear);

  rot_bitmap_set_src_ic(minute_hand_image_container, GPoint(4, 66));

  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer((BitmapLayer *)minute_hand_image_container));


  // Set up a layer for the second hand
  second_hand_image_container = rot_bitmap_layer_create(second_hand_image);

  rot_bitmap_set_compositing_mode(second_hand_image_container, GCompOpClear);

  rot_bitmap_set_src_ic(second_hand_image_container, GPoint(4, 66));

  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer((BitmapLayer *)second_hand_image_container));


  time_t rawtime;
  time(&rawtime);

  struct tm *timeinfo = localtime(&rawtime);

  update_hand_positions(timeinfo);


  // Setup the black and white circle in the centre of the watch face
  // (We use a bitmap rather than just drawing it because it means not having
  // to stuff around with working out the circle center etc.)

  // (144 = screen width, 168 = screen height)
  center_circle_white_image_container = bitmap_layer_create(GRect((144/2) - (16/2), (168/2) - (16/2), 16, 16));
  center_circle_black_image_container = bitmap_layer_create(GRect((144/2) - (16/2), (168/2) - (16/2), 16, 16));

  bitmap_layer_set_bitmap(center_circle_white_image_container, center_circle_white_image);
  bitmap_layer_set_bitmap(center_circle_black_image_container, center_circle_black_image);

  bitmap_layer_set_compositing_mode(center_circle_white_image_container, GCompOpOr);
  bitmap_layer_set_compositing_mode(center_circle_black_image_container, GCompOpClear);

  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(center_circle_white_image_container));
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(center_circle_black_image_container));


  tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
}


void handle_deinit() {
  tick_timer_service_unsubscribe();

  bitmap_layer_destroy(background_image_container);

  rot_bitmap_layer_destroy(hour_hand_image_container);
  rot_bitmap_layer_destroy(minute_hand_image_container);
  rot_bitmap_layer_destroy(second_hand_image_container);

  bitmap_layer_destroy(center_circle_white_image_container);
  bitmap_layer_destroy(center_circle_black_image_container);

  gbitmap_destroy(background_image);

  gbitmap_destroy(hour_hand_image);
  gbitmap_destroy(minute_hand_image);
  gbitmap_destroy(second_hand_image);

  gbitmap_destroy(center_circle_white_image);
  gbitmap_destroy(center_circle_black_image);

  window_destroy(window);
}


int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
