#include <gtk/gtk.h>
#include "gui.h"

void activate_app(GtkApplication *app, gpointer user_data) {
    GtkWidget *window = create_main_window(app);
    gtk_widget_show(window);
}
