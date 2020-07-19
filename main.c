#include <gtk/gtk.h>

/*  Almost all the interface is done via `Glade`
 *  Here some important highlights from that file:
 *
 *  Functions linked to signals described in layout.glade:
 *
 *      In <Preview> tab:
 *          `on_drawing_area_configure_event`
 *          `on_drawing_area_draw`
 *
 *          `on_open_button_clicked`
 *
 *          `on_save_project_button_clicked`
 *          `on_save_image_button_clicked`
 *
 *      In <Points List> tab:
 *          `on_add_path_button_clicked`
 *          `on_add_point_button_clicked`
 *
 *  Functions are linked dynamically so you need to compile this
 *  program with `-rdynamic` GCC option for GTK to recognize them.
 *
 *  Signals are being handled by functions and you have to look up
 *  signature for each of them.
 *
 *  Here only three types of signals are being handled:
 *      `clicked` signal emited by button
 *      when the button has been activated:
 *          `void <function-name>(GtkButton* button, gpointer user_data)`
 *
 *      `configure_event` signal emited by all widgets
 *      when the size, position or stacking of the widget's window has changed:
 *          `gboolean <function-name>(GtkWidget* widget, GdkEventConfigure *event)`
 *
 *      `draw` signal emited also by all widgets
 *      when widget is supposed to render itself:
 *          `gboolean <function-name>(GtkWidget* widget, cairo_t* cairo)`
 *
 *
 *  Some widgets will be borrowed from inside builder
 *  constructed from `layout.glade`.
 *
 *  Here's list of important named widgets described in layout.glade:
 *
 *      In <Preview> tab:
 *          `drawing_area`
 *
 *          `save_project_file_picker`
 *          `save_image_file_picker`
 *
 *      In <Points List> tab:
 *          `tree_view_for_points`
 *
 *          `x_entry`
 *          `y_entry`
 *          `choose_path_text_combo_box`
 *
 *      In <Settings> tab:
 *          `line_width_entry`
 *          `draw_grid_switch`
 *          `draw_every_path_with_own_color_switch`
 *  */

// ----> Widgets borrowed from `layout.glade` <---- //

GtkWidget* main_window;

// --> Widgets from <Preview> tab <-- //
GtkWidget* drawing_area;
GtkWidget* save_project_file_picker;
GtkWidget* save_image_file_picker;

// --> Widgets from <Point Lists> tab <-- //
GtkWidget* tree_view_for_points;
GtkWidget* x_entry;
GtkWidget* y_entry;
GtkWidget* choose_path_text_combo_box;

// --> Widgets from <Point Lists> tab <-- //
GtkWidget* line_width_entry;
GtkWidget* draw_grid_switch;
GtkWidget* draw_every_path_with_own_color_switch;


// Use define to get `GtkWidget*` from `GtkBuilder*` called `builder`
#define GET_WIDGET(widget) GTK_WIDGET(          \
    gtk_builder_get_object(builder, widget)  \
)

int main(int argc, char **argv) {
  // Initialize GTK with command line arguments so it will recognize
  // GTK options passed via command line arguments
  gtk_init(&argc, &argv);

  // Load glade file with all the widgets
  GtkBuilder* builder = gtk_builder_new_from_file("layout.glade");

  // --> Get widgets from just loaded layout file <--

  main_window = GET_WIDGET("main_window");
  drawing_area = GET_WIDGET("drawing_area");

  save_project_file_picker = GET_WIDGET("save_project_file_picker");
  save_image_file_picker = GET_WIDGET("save_image_file_picker");

  tree_view_for_points = GET_WIDGET("tree_view_for_points");
  x_entry = GET_WIDGET("x_entry");
  y_entry = GET_WIDGET("y_entry");
  choose_path_text_combo_box = GET_WIDGET("choose_path_text_combo_box");

  line_width_entry = GET_WIDGET("line_width_entry");
  draw_grid_switch = GET_WIDGET("draw_grid_switch");
  draw_every_path_with_own_color_switch = GET_WIDGET("draw_every_path_with_own_color_switch");

  // Make GTK listen to signals and callback program
  // when signals are being emited
  gtk_builder_connect_signals(builder, NULL);

  // Free builder object
  g_object_unref(builder);

  // Show window with all the children widgets
  gtk_widget_show(main_window);

  // Start main event loop
  // It will not exit until `gtk_main_quit()` call
  gtk_main();

  // Exit with success
  return EXIT_SUCCESS;
}

void initialize_tree_view_for_points() {
  GtkTreeIter* iter;
  GtkTreeStore* store = gtk_tree_store_new(3,
    G_TYPE_INT, G_TYPE_STRING, GTK_TYPE_BUTTON);

  // TODO: remove --->
  gtk_tree_store_set(store, iter,
                     2, "hey", gtk_button_new());
  // <---

  gtk_tree_view_set_model(
     GTK_TREE_VIEW(tree_view_for_points),
    GTK_TREE_MODEL(tree_view_for_points)
  );
}

// Cairo surface that represents `drawing_area'
cairo_surface_t* surface = NULL;

// Make surface white(1, 1, 1)
static void clear_surface(void) {
  cairo_t* cr = cairo_create(surface);

  cairo_set_source_rgb(cr, 1, 1, 1);
  cairo_paint(cr);

  cairo_destroy(cr);
}

// Initialize `surface` to create cairo on demand later
gboolean on_drawing_area_configure_event(GtkWidget* widget, GdkEventConfigure *event, gpointer data) {
  if (surface) {
    cairo_surface_destroy(surface);
  }

  surface = gdk_window_create_similar_surface(
    gtk_widget_get_window(widget),
    CAIRO_CONTENT_COLOR,
    gtk_widget_get_allocated_width(widget),
    gtk_widget_get_allocated_height(widget)
  );

  // Initialize surface to white
  clear_surface();

  // We've handled the configure event, no need for further processing
  return TRUE;
}

// Draw a line from (from_x, from_y) to (to_x, to_y)
void cairo_line(cairo_t* cr, double from_x, double from_y, double to_x, double to_y) {
  cairo_move_to(cr, from_x, from_y);
  cairo_line_to(cr, to_x, to_y);
}

// Draw border with `padding` around it
void draw_border(cairo_t* cr, int padding, int width, int height) {
  cairo_rectangle(cr, padding, padding, width - 2 * padding, height - 2 * padding);
}

// Draw grid with `padding` around it and hcells * vcells number of cells
void draw_grid(cairo_t* cr, int padding, int hcells, int vcells, int width, int height) {
  int delta_x = (width  - 2 * padding) / hcells;
  int delta_y = (height - 2 * padding) / vcells;

  for (int i = 0; i < hcells; ++ i)
    cairo_line(cr, padding + i * delta_x, padding, padding + i * delta_x, height - padding);

  for (int i = 0; i < vcells; ++ i)
    cairo_line(cr, padding, padding + i * delta_y, width - padding, padding + i * delta_y);
}

// Redraw the entire picture
void redraw(cairo_t* cr) {
  int width = gtk_widget_get_allocated_width(drawing_area);
  int height = gtk_widget_get_allocated_height(drawing_area);

  int padding = 20;

  cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);

  // TODO: make border optional
  draw_border(cr, padding, width, height);

  // TODO: make grid optional
  draw_grid(cr, padding, 10, 10, width, height);

  // Show all the lines
  cairo_stroke(cr);

  // Redraw widget. It will cause drawing_area to emit "draw" signal
  gtk_widget_queue_draw(drawing_area);
}

// Handler for `drawing_area` `draw` signal
void on_drawing_area_draw(GtkWidget *drawing_area, cairo_t *cr, gpointer data) {
  cairo_set_source_surface(cr, surface, 0, 0);
  cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
  cairo_paint(cr);
  redraw(cr);
}

// [Open] button for opening projects
void on_open_button_clicked(GtkButton* button, gpointer user_data) {
  // TODO: open file manager and get project
  // TODO: remove current contents
  cairo_t* cr = cairo_create(surface);

  redraw(cr);
  g_print("Open!\n");

  cairo_destroy(cr);
}
