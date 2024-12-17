#include <gtk/gtk.h>
#include <stdio.h>
#include "support.h"



int main(int argc, char *argv[]) {
    gtk_init();
    GtkApplication *app;
    int status;

    app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);
    g_signal_connect (app, "activate", G_CALLBACK(activate), NULL);

    status = g_application_run (G_APPLICATION(app), argc, argv);
    g_object_unref(app);

  return status;
}


