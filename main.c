#include <gtk/gtk.h>

/*  Almost all the interface is done via `Glade`
 *  Here some important highlights from that file:
 *
 *  Functions linked to signals described in layout.glade:
 *
 *      In <Preview> tab:
 *          `on_drawing_area_draw`
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
 *  Here only two types of signals (except defined in the code) are being handled:
 *      `clicked` signal emited by button
 *      when the button has been activated:
 *          `void <function-name>(GtkButton* button, gpointer user_data)`
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
 *      In <Points List> tab:
 *          `tree_view_for_points`
 *
 *          `x_entry`
 *          `y_entry`
 *          `choose_path_text_combo_box`
 *
 *      In <Settings> tab:
 *          `line_width_entry`
 *          `randomize_colors_switch`
 *          `line_color_picker`
 *          `draw_grid_switch`
 *          `grid_color_picker`
 *          `point_radius_entry`
 *          `point_color_picker`
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
GtkWidget* randomize_colors_switch;
GtkWidget* line_color_picker;
GtkWidget* draw_grid_switch;
GtkWidget* grid_color_picker;
GtkWidget* point_radius_entry;
GtkWidget* point_color_picker;
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

gboolean is_number(const char* string) {
  char* end;
  strtod(string, &end);

  if (end == string || *end != '\0')
    return FALSE;

  return TRUE;
}

void on_tree_view_x_cell_edited(GtkCellRendererText *cell,
                                gchar *path_string,
                                gchar *new_text,
                                gpointer user_data) {
  GtkTreeIter iter;
  gtk_tree_model_get_iter(GTK_TREE_MODEL(tree_store), &iter,
                          gtk_tree_path_new_from_string(path_string));

  if ((gtk_tree_store_iter_depth(tree_store, &iter) == 0 ||
      is_number(new_text)) && g_strcmp0(new_text, "") != 0)
    update_tree_model_cell(path_string, X_COORDINATE_COLUMN, new_text);
}

void on_tree_view_y_cell_edited(GtkCellRendererText *cell,
                                gchar *path_string,
                                gchar *new_text,
                                gpointer user_data) {
  GtkTreeIter iter;
  gtk_tree_model_get_iter(GTK_TREE_MODEL(tree_store), &iter,
                          gtk_tree_path_new_from_string(path_string));

  if (gtk_tree_store_iter_depth(tree_store, &iter) != 0 &&
      is_number(new_text) && g_strcmp0(new_text, "") != 0)
    update_tree_model_cell(path_string, Y_COORDINATE_COLUMN, new_text);
}

void initialize_tree_view_columns(void) {
  append_column_to_tree_view("X Coordinate", X_COORDINATE_COLUMN, on_tree_view_x_cell_edited);
  append_column_to_tree_view("Y Coordinate", Y_COORDINATE_COLUMN, on_tree_view_y_cell_edited);
}

void update_paths_in_combo_box(void) {
  gtk_combo_box_text_remove_all(
    GTK_COMBO_BOX_TEXT(choose_path_text_combo_box)
  );

  GtkTreeIter iter;
  gtk_tree_model_get_iter(GTK_TREE_MODEL(tree_store), &iter,
                          gtk_tree_path_new_first());

  if (!gtk_tree_store_iter_is_valid(tree_store, &iter))
    return;

  int count = 0; do {
    GValue value = G_VALUE_INIT;
    gtk_tree_model_get_value(GTK_TREE_MODEL(tree_store), &iter,
                             X_COORDINATE_COLUMN, &value);

    const gchar* name = g_value_get_string(&value);
    gtk_combo_box_text_append_text(
      GTK_COMBO_BOX_TEXT(choose_path_text_combo_box),
      name
    );

    count ++;
  } while(gtk_tree_model_iter_next(GTK_TREE_MODEL(tree_store), &iter));

  gtk_combo_box_set_active(GTK_COMBO_BOX(choose_path_text_combo_box), count - 1);
}

gboolean on_tree_view_key_pressed(GtkWidget *widget, GdkEventKey *event, gpointer data) {
  if (event->keyval == GDK_KEY_Delete){
    GtkTreeIter iter;
    GtkTreeSelection* selection = gtk_tree_view_get_selection(
      GTK_TREE_VIEW(tree_view_for_points)
    );

    GtkTreeModel* model = GTK_TREE_MODEL(tree_store);
    gtk_tree_selection_get_selected(selection, &model, &iter);

    gtk_tree_store_remove(tree_store, &iter);

    if (gtk_tree_store_iter_depth(tree_store, &iter) == 0)
      update_paths_in_combo_box();
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
  g_signal_connect(G_OBJECT(tree_view_for_points), "key_press_event",
                   G_CALLBACK(on_tree_view_key_pressed), NULL);
}

// line (color = #555753, size   = 5)
// grid (color = #D3D7CF)
// point(color = #2E3436; radius = 5)
void initialize_defaults(void) {
  GdkRGBA grid_default_color  = { 0x55 / 256.0,
                                  0x57 / 256.0,
                                  0x53 / 256.0, 1.0 };
  gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(grid_color_picker),
                            &grid_default_color);

  GdkRGBA line_default_color  = { 0xD3 / 256.0,
                                  0xD7 / 256.0,
                                  0xCF / 256.0, 1.0 };
  gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(line_color_picker),
                            &line_default_color);

  GdkRGBA point_default_color = { 0x2E / 256.0,
                                  0x34 / 256.0,
                                  0x36 / 256.0, 1.0 };
  gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(point_color_picker),
                            &point_default_color);

  gtk_entry_set_text(GTK_ENTRY(  line_width_entry), "5");

  gtk_entry_set_text(GTK_ENTRY(point_radius_entry), "5");

  gtk_switch_set_state(GTK_SWITCH(draw_grid_switch),
                       TRUE);
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

  tree_view_for_points       = GET_WIDGET(      "tree_view_for_points");
  x_entry                    = GET_WIDGET(                   "x_entry");
  y_entry                    = GET_WIDGET(                   "y_entry");
  choose_path_text_combo_box = GET_WIDGET("choose_path_text_combo_box");

  line_width_entry           = GET_WIDGET(          "line_width_entry");
  randomize_colors_switch    = GET_WIDGET(   "randomize_colors_switch");
  line_color_picker          = GET_WIDGET(         "line_color_picker");
  draw_grid_switch           = GET_WIDGET(          "draw_grid_switch");
  grid_color_picker          = GET_WIDGET(         "grid_color_picker");
  point_radius_entry         = GET_WIDGET(        "point_radius_entry");
  point_color_picker         = GET_WIDGET(        "point_color_picker");
  // ------------------------------------- -----------

  // Initialize tree_view & it's model
  initialize_tree_view_for_points();

  // Set default values for drawing
  initialize_defaults();

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

// Draw a line from (from_x, from_y) to (to_x, to_y)
void cairo_line(cairo_t* cr, double from_x, double from_y, double to_x, double to_y) {
  cairo_move_to(cr, from_x, from_y);
  cairo_line_to(cr, to_x, to_y);
}

// Draw border with `padding` around it
void draw_border(cairo_t* cr, int padding,
                 int width, int height,
                 GdkRGBA* border_color) {

  cairo_set_source_rgba(cr,
                        border_color->red , border_color->green,
                        border_color->blue, border_color->alpha);

  cairo_rectangle(cr, padding, padding, width - 2 * padding, height - 2 * padding);

  cairo_stroke(cr);
}

void draw_grid(cairo_t* cr, int padding,
               int hcells, int vcells,
               int width , int height,
               GdkRGBA* grid_color) {

  cairo_set_source_rgba(cr,
                        grid_color->red , grid_color->green,
                        grid_color->blue, grid_color->alpha);

  int delta_x = (width  - 2 * padding) / hcells;
  int delta_y = (height - 2 * padding) / vcells;

  for (int i = 0; i < hcells; ++ i)
    cairo_line(cr, padding + i * delta_x, padding, padding + i * delta_x, height - padding);

  for (int i = 0; i < vcells; ++ i)
    cairo_line(cr, padding, padding + i * delta_y, width - padding, padding + i * delta_y);

  cairo_stroke(cr);
}

void draw_paths_and_points(cairo_t* cr, int padding,      int point_radius,
                           int  hcells, int  vcells,
                           int   width, int  height,
                           gdouble       line_width,
                           GdkRGBA*     point_color, GdkRGBA*  line_color) {

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

    gdouble x_from , y_from;

    int i = 0; do {
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

      // Draw a line
      if (i != 0) {
        cairo_set_source_rgba(cr,
                              line_color->red , line_color->green,
                              line_color->blue, line_color->alpha);

        cairo_set_line_width(cr, line_width);
        cairo_line(cr, x_from, y_from, x_real, y_real);

        cairo_stroke(cr);
      }

      if (i != 0) {
        // Draw a point
        cairo_set_source_rgba(cr,
                              point_color->red , point_color->green,
                              point_color->blue, point_color->alpha);

        cairo_arc(cr, x_from, y_from, point_radius, 0, 2 * 3.1415926);

        cairo_fill(cr);
      }

      x_from = x_real;
      y_from = y_real;

      ++ i;
    } while(gtk_tree_model_iter_next(GTK_TREE_MODEL(tree_store), &iter));

    // Draw the last point
    cairo_set_source_rgba(cr,
                          point_color->red , point_color->green,
                          point_color->blue, point_color->alpha);

    cairo_arc(cr, x_from, y_from, point_radius, 0, 2 * 3.1415926);

    cairo_fill(cr);
  } while(gtk_tree_model_iter_next(GTK_TREE_MODEL(tree_store), &parent));
}

void redraw(cairo_t* cr) {
  int width = gtk_widget_get_allocated_width(drawing_area);
  int height = gtk_widget_get_allocated_height(drawing_area);

  int padding = 10;

  int hcells = 10;
  int vcells = 10;

  /* GdkRGBA* border_color; */
  /* gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(border_color), border_color); */

  gboolean is_grid_enabled = gtk_switch_get_active(GTK_SWITCH(draw_grid_switch));

  if (is_grid_enabled) {
    GdkRGBA grid_color;
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(grid_color_picker), &grid_color);

    draw_grid(cr, padding, hcells, vcells, width, height, &grid_color);
    draw_border(cr, padding, width, height, &grid_color);
  }

  GdkRGBA line_color;
  gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(line_color_picker),
                             &line_color);

  GdkRGBA point_color;
  gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(point_color_picker),
                             &point_color);

  const gchar* point_radius_text =
    gtk_entry_get_text(GTK_ENTRY(point_radius_entry));

  gchar* end_text;
  double point_radius = strtod(point_radius_text, &end_text);

  const gchar* line_width_text =
    gtk_entry_get_text(GTK_ENTRY(line_width_entry));

  double line_width = strtod(line_width_text, &end_text);

  draw_paths_and_points(cr, padding, point_radius,
                        hcells, vcells,
                        width , height,  line_width,
                          &point_color, &line_color);
}

// Handler for `drawing_area` `draw` signal
void on_drawing_area_draw(GtkWidget *drawing_area, cairo_t *cr, gpointer data) {
  redraw(cr);
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
                     Y_COORDINATE_COLUMN, "",
                     -1);

  g_free(path_name);

  update_paths_in_combo_box();
}

void on_add_point_button_clicked(GtkButton* button, gpointer user_data) {
  const gchar* x_text = gtk_entry_get_text(GTK_ENTRY(x_entry));
  const gchar* y_text = gtk_entry_get_text(GTK_ENTRY(y_entry));

  // Defaults for x in y
  if (g_strcmp0(x_text, "") == 0)
    x_text = "0";

  if (g_strcmp0(y_text, "") == 0)
    y_text = "0";

  if (!is_number(y_text)) {
    gtk_entry_set_text(GTK_ENTRY(y_entry), "");
    return;
  }

  if (!is_number(x_text)) {
    gtk_entry_set_text(GTK_ENTRY(x_entry), "");
    return;
  }

  gchar* path_name = gtk_combo_box_text_get_active_text(
    GTK_COMBO_BOX_TEXT(choose_path_text_combo_box)
  );

  if (path_name == NULL)
    return;

  GtkTreeIter iter;
  gtk_tree_model_get_iter(GTK_TREE_MODEL(tree_store), &iter,
                          gtk_tree_path_new_first());

  if (!gtk_tree_store_iter_is_valid(tree_store, &iter))
    return;

  do {
    GValue value = G_VALUE_INIT;
    gtk_tree_model_get_value(GTK_TREE_MODEL(tree_store), &iter, X_COORDINATE_COLUMN, &value);

    const gchar* name = g_value_get_string(&value);
    if (g_strcmp0(path_name, name) == 0) {
      GtkTreeIter append_iter;

      gtk_tree_store_append(tree_store, &append_iter, &iter);
      gtk_tree_store_set(tree_store, &append_iter,
                         X_COORDINATE_COLUMN, x_text,
                         Y_COORDINATE_COLUMN, y_text,
                         -1);

      return;
    }
  } while(gtk_tree_model_iter_next(GTK_TREE_MODEL(tree_store), &iter));
}
