#include <pebble.h>

Window *s_main_window;
static Layer *s_canvas_layer;

TextLayer *s_textlayer_1 = NULL;

void set_text(char *x) {
//   GFont *font, GTextAlignment *alignment
  if(s_textlayer_1==NULL) return;
  text_layer_set_text(s_textlayer_1, x);
  text_layer_set_background_color(s_textlayer_1, GColorClear);
  text_layer_set_font(s_textlayer_1, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_textlayer_1, GTextAlignmentCenter);
}


#include <statemachine.h>

static void canvas_update_proc(Layer *this_layer, GContext *ctx) {
  
  
  //set_text("Fight!");
  
  GRect bounds = layer_get_bounds(this_layer);
  GPoint head = GPoint(bounds.size.w / 2, 45);
  
  // Draw
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, head, 15);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, head, 10);
  
    // Draw the body
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(bounds.size.w / 2 - 2, 60, 5, 25), 0, GCornerNone);

  // hands
  graphics_context_set_stroke_width(ctx, 4);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  GPoint x = GPoint(bounds.size.w / 2 + 15,80);
  GPoint y = GPoint(bounds.size.w / 2,60);
  graphics_draw_line(ctx, x,y);
  GPoint i = GPoint(bounds.size.w / 2 - 15,80);
  GPoint j = GPoint(bounds.size.w / 2,60);
  graphics_draw_line(ctx, i,j);
  //feet
  GPoint left_init_foot = GPoint(72,85);
  GPoint left_end_foot = GPoint(60,95);
  graphics_draw_line(ctx, left_init_foot,left_end_foot);
  GPoint right_init_foot = GPoint(72,85);
  GPoint right_end_foot = GPoint(84,95);
  graphics_draw_line(ctx, right_init_foot,right_end_foot);
  
  GPoint left_glove = GPoint(bounds.size.w / 2 + 15,80);
  GPoint right_glove = GPoint(bounds.size.w /2 -15,80);
  // Draw
  graphics_context_set_fill_color(ctx, GColorRed);
  graphics_fill_circle(ctx, left_glove, 4);
  graphics_fill_circle(ctx, right_glove, 4);
  
  GPoint under = GPoint(bounds.size.w / 2 - 25,110);
  GPoint line = GPoint(bounds.size.w / 2 + 25,110);
  graphics_draw_line(ctx, under,line);
  
  handle_init(s_main_window);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  // Create Layer
  s_canvas_layer = layer_create(GRect(0, 0, window_bounds.size.w, window_bounds.size.h));
  layer_add_child(window_layer, s_canvas_layer);

  // Set the update_proc
  APP_LOG(APP_LOG_LEVEL_DEBUG, "2");
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "3");
  layer_add_child(window_get_root_layer(s_main_window), text_layer_get_layer(s_textlayer_1));  
}

static void main_window_unload(Window *window) {
  // Destroy Layer
  layer_destroy(s_canvas_layer);
  text_layer_destroy(s_textlayer_1);
}

static void init(void) {
  // Create main Window
  s_main_window = window_create();
  s_textlayer_1 = text_layer_create(GRect(10, 10, 120, 300));
  window_set_background_color(s_main_window, GColorCyan);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "1");
}

static void deinit(void) {
  // Destroy main Window
  window_destroy(s_main_window);
  
  handle_deinit(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}