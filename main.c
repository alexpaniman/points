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

// --> Widgets from     <Preview> tab <-- //
GtkWidget* drawing_area;
GtkWidget* save_project_file_picker;
GtkWidget* save_image_file_picker;

// --> Widgets from <Point Lists> tab <-- //
GtkWidget* tree_view_for_points;
GtkWidget* x_entry;
GtkWidget* y_entry;
GtkWidget* choose_path_text_combo_box;

// --> Widgets from    <Settings> tab <-- //
GtkWidget* line_width_entry;
GtkWidget* draw_grid_switch;
GtkWidget* draw_every_path_with_own_color_switch;
// ------------------------------------------------ //

enum { // <-- Tree store columns
  X_COORDINATE_COLUMN,
  Y_COORDINATE_COLUMN,

  N_COLUMNS // It corresponds to number of columns
};

GtkTreeStore* tree_store = NULL;

void initialize_tree_store(void) {
  tree_store = gtk_tree_store_new(N_COLUMNS,
                                  G_TYPE_STRING, /* --> X coordinate column */
                                  G_TYPE_STRING  /* --> Y coordinate column */);
}

// It appends columns to `tree_view_for_columns` declared in the top of this file
void append_column_to_tree_view(char* name, gint column_id,
                                void (*on_tree_view_cell_edited)(
                                  GtkCellRendererText *cell,
                                  gchar *path_string,
                                  gchar *new_text,
                                  gpointer user_data)) {
  GtkCellRenderer* column_renderer =
    gtk_cell_renderer_text_new(); // Use simple renderer
                                  // To render text as... text

  // This makes column cells editable (via entry)
  g_object_set(column_renderer, "editable", TRUE, NULL);
  g_signal_connect(column_renderer, "edited", (GCallback) on_tree_view_cell_edited, NULL);

  // Declare column name, how to render column cells (via cell renderer)
  //         it's type (text) and link it with corresponding column id
  GtkTreeViewColumn* column =
    gtk_tree_view_column_new_with_attributes(
      name, column_renderer,
      "text", column_id, NULL);

  // Make columns resizable & clickable
  gtk_tree_view_column_set_clickable(column, TRUE);
  gtk_tree_view_column_set_resizable(column, TRUE);

  // Make columns take all the available space
  gtk_tree_view_column_set_expand(column, TRUE);

  // And finally append our column
  gtk_tree_view_append_column(
    GTK_TREE_VIEW(tree_view_for_points),
    column);
}

void update_tree_model_cell(gchar* path, gint column_id, gchar* new_text) {
  GtkTreeIter iter;
  gtk_tree_model_get_iter(GTK_TREE_MODEL(tree_store), &iter,
                          gtk_tree_path_new_from_string(path));

  gtk_tree_store_set(tree_store, &iter,
                     column_id, new_text,
                     -1);
}

gboolean is_number(char* string) { // TODO: make it smarter
  do {
    if (*string == '.')
      continue;

    int current = *string - '0';
    if (current <= 0 || current >= 9)
      return FALSE;
  } while(*(string = string + 1) != '\0');

  return TRUE;
}

void on_tree_view_x_cell_edited(GtkCellRendererText *cell,
                                gchar *path_string,
                                gchar *new_text,
                                gpointer user_data) {

  if (is_number(new_text))
    update_tree_model_cell(path_string, X_COORDINATE_COLUMN, new_text);
}

void on_tree_view_y_cell_edited(GtkCellRendererText *cell,
                                gchar *path_string,
                                gchar *new_text,
                                gpointer user_data) {

  if (is_number(new_text))
    update_tree_model_cell(path_string, Y_COORDINATE_COLUMN, new_text);
}

void initialize_tree_view_columns(void) {
  append_column_to_tree_view("X Coordinate", X_COORDINATE_COLUMN, on_tree_view_x_cell_edited);
  append_column_to_tree_view("Y Coordinate", Y_COORDINATE_COLUMN, on_tree_view_y_cell_edited);
}

// TODO: better name
gboolean on_key_press (GtkWidget *widget, GdkEventKey *event, gpointer data) {
    if (event->keyval == GDK_KEY_Delete){
      GtkTreeIter iter;
      GtkTreeSelection* selection =
        gtk_tree_view_get_selection(
          GTK_TREE_VIEW(tree_view_for_points)
        );

      GtkTreeModel* model = GTK_TREE_MODEL(tree_store);
      gtk_tree_selection_get_selected(selection, & model, &iter);

      gtk_tree_store_remove(tree_store, &iter);
      return TRUE;
    }
    return FALSE;
}

// `tree_view_for_points` is the widget declared in the top of the file
void initialize_tree_view_for_points(void) {
  initialize_tree_view_columns();
  initialize_tree_store(); // Initialize it's model

  gtk_tree_view_set_model(
    GTK_TREE_VIEW(tree_view_for_points),
    GTK_TREE_MODEL(tree_store)
  );

  gtk_widget_add_events(tree_view_for_points, GDK_KEY_PRESS_MASK);
  g_signal_connect(G_OBJECT(tree_view_for_points), "key_press_event", G_CALLBACK (on_key_press), NULL);
}

// We will load `layout.glade` in this `builder`
GtkBuilder* builder;

// Use define to get widget by name from `builder`
#define GET_WIDGET(widget) GTK_WIDGET(       \
    gtk_builder_get_object(builder, widget)  \
)

int main(int argc, char **argv) {
  // Initialize GTK with command line arguments so it will recognize
  // GTK options passed via command line arguments
  gtk_init(&argc, &argv);

  // Load glade file with all the widgets
  builder = gtk_builder_new_from_file("layout.glade");

  // --> Get widgets from just loaded layout file <--
  main_window                = GET_WIDGET(               "main_window");

  drawing_area               = GET_WIDGET(              "drawing_area");
  save_project_file_picker   = GET_WIDGET(  "save_project_file_picker");
  save_image_file_picker     = GET_WIDGET(    "save_image_file_picker");

  tree_view_for_points       = GET_WIDGET(      "tree_view_for_points");
  x_entry                    = GET_WIDGET(                   "x_entry");
  y_entry                    = GET_WIDGET(                   "y_entry");
  choose_path_text_combo_box = GET_WIDGET("choose_path_text_combo_box");

  line_width_entry           = GET_WIDGET(          "line_width_entry");
  draw_grid_switch           = GET_WIDGET(          "draw_grid_switch");
  draw_every_path_with_own_color_switch =
    GET_WIDGET(                "draw_every_path_with_own_color_switch");
  // ------------------------------------------------

  // Initialize tree_view & it's model
  initialize_tree_view_for_points();

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

// Cairo surface that represents `drawing_area`
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
void draw_grid(cairo_t* cr, int padding,
               int hcells, int vcells,
               int width , int height) {
  int delta_x = (width  - 2 * padding) / hcells;
  int delta_y = (height - 2 * padding) / vcells;

  for (int i = 0; i < hcells; ++ i)
    cairo_line(cr, padding + i * delta_x, padding, padding + i * delta_x, height - padding);

  for (int i = 0; i < vcells; ++ i)
    cairo_line(cr, padding, padding + i * delta_y, width - padding, padding + i * delta_y);
}

void draw_paths_and_points(cairo_t* cr, int padding, int point_radius,
                           int  hcells, int vcells,
                           int   width, int height,
                           gdouble r, gdouble g, gdouble b) {

  int delta_x = (width  - 2 * padding) / hcells;
  int delta_y = (height - 2 * padding) / vcells;

  GtkTreeIter parent;
  gtk_tree_model_get_iter(GTK_TREE_MODEL(tree_store), &parent,
                          gtk_tree_path_new_first());

  if (!gtk_tree_store_iter_is_valid(tree_store, &parent))
    return;

  do {
    GtkTreeIter iter;
    if (gtk_tree_model_iter_n_children(GTK_TREE_MODEL(tree_store), &parent) == 0)
      continue;

    gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(tree_store), &iter, &parent, 0);

    gboolean is_first = TRUE;
    gdouble x_from, y_from;

    do {
      GValue x_value = G_VALUE_INIT;
      gtk_tree_model_get_value(GTK_TREE_MODEL(tree_store), &iter,
                               X_COORDINATE_COLUMN, &x_value);

      const gchar* x_string = g_value_get_string(&x_value);

      double x;
      sscanf(x_string, "%lf", &x);

      GValue y_value = G_VALUE_INIT;
      gtk_tree_model_get_value(GTK_TREE_MODEL(tree_store), &iter,
                               Y_COORDINATE_COLUMN, &y_value);

      const gchar* y_string = g_value_get_string(&y_value);

      double y;
      sscanf(y_string, "%lf", &y);

      double x_real = x * delta_x + padding;
      double y_real = y * delta_y + padding;

      // Draw a point
      cairo_arc(cr, x_real, y_real, point_radius, 0, 2 * 3.1415926);
      cairo_fill(cr);

      // Draw a line
      if (!is_first) {
        cairo_line(cr, x_from, y_from, x_real, y_real);
        cairo_stroke(cr);
      } else {
        is_first = FALSE;
      }

      x_from = x_real;
      y_from = y_real;
    } while(gtk_tree_model_iter_next(GTK_TREE_MODEL(tree_store), &iter));

  } while(gtk_tree_model_iter_next(GTK_TREE_MODEL(tree_store), &parent));
}

// Redraw the entire picture
void redraw(cairo_t* cr) {
  int width  = gtk_widget_get_allocated_width (drawing_area);
  int height = gtk_widget_get_allocated_height(drawing_area);

  int padding = 20;

  cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);

  // TODO: make border optional
  draw_border(cr, padding, width, height);

  // TODO: make grid optional
  draw_grid(cr, padding, 10, 10, width, height);

  cairo_stroke(cr);

  cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);

  draw_paths_and_points(cr, padding, 10, 10, 10, width, height, 1.0, 1.0, 1.0);

  // Show all the lines
  cairo_stroke(cr);

  // Redraw widget. It will cause drawing_area to emit "draw" signal
  gtk_widget_queue_draw(drawing_area);
}

// Handler for `drawing_area` `draw` signal
void on_drawing_area_draw(GtkWidget *drawing_area, cairo_t *cr, gpointer data) {
  cairo_set_source_surface(cr, surface, 0, 0);
  cairo_paint(cr);
}

int path_number = 1;

void on_add_path_button_clicked(GtkButton* button, gpointer user_data){
  GtkTreeIter iter;

  int  len     = snprintf(NULL, 0,"%d", path_number);
  char num[len];  sprintf(num, "%d", path_number ++);

  gchar* name = "Контур ";
  gchar* path_name = g_strconcat(name, num, NULL);

  gtk_tree_store_append(tree_store, &iter, NULL);
  gtk_tree_store_set(tree_store, &iter,
                     X_COORDINATE_COLUMN, path_name,
                     Y_COORDINATE_COLUMN, "<name>",
                     -1);

  for (int i = 0; i < 10; ++ i) {
    GtkTreeIter piter;
    gtk_tree_store_append(tree_store, &piter, &iter);
    gtk_tree_store_set(tree_store, &piter,
                       X_COORDINATE_COLUMN, "0",
                       Y_COORDINATE_COLUMN, "0",
                       -1);
  }

  g_free(path_name);
}

void on_add_point_button_clicked(GtkButton* button, gpointer user_data) {
 /* `x_entry`                     */
 /* `y_entry`                     */
 /* `choose_path_text_combo_box`  */

  gchar x_text[9];
  gtk_entry_set_text(GTK_ENTRY(x_entry), x_text);

  gchar y_text[9];
  gtk_entry_set_text(GTK_ENTRY(y_entry), y_text);

  gchar* path_name = gtk_combo_box_text_get_active_text(
    GTK_COMBO_BOX_TEXT(choose_path_text_combo_box)
  );

  GtkTreeModel* model = GTK_TREE_MODEL(tree_store);

  GtkTreeIter iter;
  gtk_tree_model_get_iter(model, &iter,
                          gtk_tree_path_new_first());

  while(gtk_tree_model_iter_next(model, &iter)) {
    GValue* x_value;

    gtk_tree_model_get_value(model, &iter, X_COORDINATE_COLUMN, x_value);
    const gchar* x = g_value_get_string(x_value);

    g_print("Hey");
  }
}

// [Open] button for opening projects
void on_open_button_clicked(GtkButton* button, gpointer user_data) {
  cairo_t* cr = cairo_create(surface);
  redraw(cr);
  cairo_destroy(cr);
  gtk_widget_queue_draw(drawing_area);
  // TODO: open file manager and get project
  // TODO: remove current contents
}
