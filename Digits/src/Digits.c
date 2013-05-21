#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

//Repro of http://blog.tokyoflash.com/2012/09/digits-united-to-tell-time/
//Code forked from ninety_one_dub (sdk app)

#define MY_UUID { 0x71, 0x7A, 0x67, 0x61, 0xAB, 0x71, 0x45, 0x1D, 0xAC, 0x1E, 0x8E, 0x9E, 0x21, 0x1B, 0xA5, 0xDF }
#define TOTAL_TIME_DIGITS 4
PBL_APP_INFO(MY_UUID,
             "Digits", "kardain",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_WATCH_FACE);

//Define main window
Window window;

//Define images (second and fourth digits)
const int BIG_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_NUM_L_0,
  RESOURCE_ID_IMAGE_NUM_L_1,
  RESOURCE_ID_IMAGE_NUM_L_2,
  RESOURCE_ID_IMAGE_NUM_L_3,
  RESOURCE_ID_IMAGE_NUM_L_4,
  RESOURCE_ID_IMAGE_NUM_L_5,
  RESOURCE_ID_IMAGE_NUM_L_6,
  RESOURCE_ID_IMAGE_NUM_L_7,
  RESOURCE_ID_IMAGE_NUM_L_8,
  RESOURCE_ID_IMAGE_NUM_L_9
};

//Define images (first and third digits)
const int SMALL_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_NUM_S_0,
  RESOURCE_ID_IMAGE_NUM_S_1,
  RESOURCE_ID_IMAGE_NUM_S_2,
  RESOURCE_ID_IMAGE_NUM_S_3,
  RESOURCE_ID_IMAGE_NUM_S_4,
  RESOURCE_ID_IMAGE_NUM_S_5
};

BmpContainer time_digits_images[TOTAL_TIME_DIGITS];

unsigned short get_display_hour(unsigned short hour) {
  if (clock_is_24h_style()) {
    return hour;
  }
  unsigned short display_hour = hour % 12;
  // Converts "0" to "12"
  return display_hour ? display_hour : 12;
}

void set_container_image(BmpContainer *bmp_container, const int resource_id, GPoint origin) {
  layer_remove_from_parent(&bmp_container->layer.layer);
  bmp_deinit_container(bmp_container);
  bmp_init_container(resource_id, bmp_container);
  GRect frame = layer_get_frame(&bmp_container->layer.layer);
  frame.origin.x = origin.x;
  frame.origin.y = origin.y;
  layer_set_frame(&bmp_container->layer.layer, frame);
  layer_add_child(&window.layer, &bmp_container->layer.layer);
}

void update_display(PblTm *current_time) {
  unsigned short display_hour = get_display_hour(current_time->tm_hour);
  set_container_image(&time_digits_images[0], SMALL_IMAGE_RESOURCE_IDS[display_hour/10], GPoint(39, 23));
  set_container_image(&time_digits_images[1], BIG_IMAGE_RESOURCE_IDS[display_hour%10], GPoint(0, 0));
  set_container_image(&time_digits_images[2], SMALL_IMAGE_RESOURCE_IDS[current_time->tm_min/10], GPoint(39, 106));
  set_container_image(&time_digits_images[3], BIG_IMAGE_RESOURCE_IDS[current_time->tm_min%10], GPoint(0, 83));
  if (!clock_is_24h_style()) {
    if (current_time->tm_hour >= 12) {
      set_container_image(&time_format_image, RESOURCE_ID_IMAGE_PM_MODE, GPoint(17, 68));
    } else {
      layer_remove_from_parent(&time_format_image.layer.layer);
      bmp_deinit_container(&time_format_image);
    }

    if (display_hour/10 == 0) {
      layer_remove_from_parent(&time_digits_images[0].layer.layer);
      bmp_deinit_container(&time_digits_images[0]);
    }
  }
}

void handle_minute_tick(AppContextRef ctx, PebbleTickEvent *t) {
  (void)ctx;
  update_display(t->tick_time);
}

void handle_init(AppContextRef ctx) {
  (void)ctx;
  window_init(&window, "Digits");
  window_stack_push(&window, true /* Animated */);
  resource_init_current_app(&APP_RESOURCES);
  // Avoids a blank screen on watch start.
  PblTm tick_time;
  get_time(&tick_time);
  update_display(&tick_time);

}

void handle_deinit(AppContextRef ctx) {
  (void)ctx;
  for (int i = 0; i < TOTAL_TIME_DIGITS; i++) {
    bmp_deinit_container(&time_digits_images[i]);
  }
}

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .deinit_handler = &handle_deinit,
    .tick_info = {
      .tick_handler = &handle_minute_tick,
      .tick_units = MINUTE_UNIT
    }
  };
  app_event_loop(params, &handlers);
}

