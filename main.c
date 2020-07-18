#include <gtk/gtk.h>

// Widgets
GtkWidget* main_window;
GtkWidget* drawing_area;

int main(int argc, char **argv) {
  gtk_init(&argc, &argv);

  // Load glade file with all the layout
  GtkBuilder* builder = gtk_builder_new_from_file("layout.glade");

  // Get widgets from just loaded layout file

  main_window = GTK_WIDGET(
      gtk_builder_get_object(builder, "main-window")
  );

  drawing_area = GTK_WIDGET(
      gtk_builder_get_object(builder, "drawing-area")
  );

  // Make gtk listen to signals and callback program
  gtk_builder_connect_signals(builder, NULL);

  // Free buffer
  g_object_unref(builder);

  // Show window with all child windows
  gtk_widget_show(main_window);

  // Start main event loop
  gtk_main();

  // Exit with success
  return EXIT_SUCCESS;
}

// Cairo surface that represents `drawing_area'
cairo_surface_t* surface = NULL;

// Initialize surface to create cairo on demand later
gboolean on_drawing_area_configure_event(GtkWidget* widget, GdkEventConfigure *event, gpointer data) {
  if (surface) {
    cairo_surface_destroy (surface);
  }

  surface = gdk_window_create_similar_surface(
    gtk_widget_get_window(widget),
    CAIRO_CONTENT_COLOR,
    gtk_widget_get_allocated_width(widget),
    gtk_widget_get_allocated_height(widget)
  );

  // Initialize surface to white


  // We've handled the configure event, no need for further processing
  return TRUE;
}

void cairo_line(cairo_t* cr, double from_x, double from_y, double to_x, double to_y) {
  cairo_move_to(cr, from_x, from_y);
  cairo_line_to(cr, to_x, to_y);
}

void draw_border(cairo_t* cr, int padding, int width, int height) {
  cairo_line(cr, padding, padding, width - padding, padding);
  cairo_line(cr, width - padding, padding, width - padding, height - padding);
  cairo_line(cr, width - padding, height - padding, padding, height - padding);
  cairo_line(cr, padding, height - padding, padding, padding);
}

void draw_grid(cairo_t* cr, int padding, int hcells, int vcells, int width, int height) {
  int delta_x = (width  - 2 * padding) / hcells;
  int delta_y = (height - 2 * padding) / vcells;

  for (int i = 0; i < hcells; ++ i)
    cairo_line(cr, padding + i * delta_x, padding, padding + i * delta_x, height - padding);

  for (int i = 0; i < vcells; ++ i)
    cairo_line(cr, padding, padding + i * delta_y, width - padding, padding + i * delta_y);
}

void redraw(cairo_t* cr) {
  int width = gtk_widget_get_allocated_width(drawing_area);
  int height = gtk_widget_get_allocated_height(drawing_area);

  int padding = 20;

  cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);

  draw_border(cr, padding, width, height);
  draw_grid(cr, padding, 10, 10, width, height);

  cairo_stroke(cr);

  gtk_widget_queue_draw(drawing_area);
}

void on_drawing_area_draw(GtkWidget *drawing_area, cairo_t *cr, gpointer data) {
  cairo_set_source_surface(cr, surface, 0, 0);
  cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
  cairo_paint(cr);
  redraw(cr);
}

// [Open] button for opening projects
void on_open_button_clicked(GtkButton* button) {
  // TODO: open file manager and get project
  cairo_t* cr = cairo_create(surface);

  redraw(cr);
  g_print("Open!\n");

  cairo_destroy(cr);
}
