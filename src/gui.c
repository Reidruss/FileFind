#include <stdio.h>
#include <stdbool.h>
#include <gtk/gtk.h>

#include "ops.h"
#include "gui.h"

GtkWidget *create_main_window(GtkApplication *app) 
{
    GtkWidget *window = gtk_application_window_new(app);

    gtk_window_set_title(GTK_WINDOW(window), "FileFind");
    gtk_window_set_default_size(GTK_WINDOW(window), 1400, 900);
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);
    gtk_window_set_decorated(GTK_WINDOW(window), TRUE);

    GtkWidget *header = gtk_header_bar_new();
    gtk_header_bar_set_show_title_buttons(GTK_HEADER_BAR(header), FALSE);

    GtkWidget *title_label = gtk_label_new("FileFind");
    gtk_header_bar_set_title_widget(GTK_HEADER_BAR(header), title_label);

    header = add_header_buttons(window, header);
    gtk_window_set_titlebar(GTK_WINDOW(window), header);

    return window;
}

GtkWidget *add_header_buttons(GtkWidget *window, GtkWidget *header)
{
    GtkWidget *close_button = gtk_button_new_from_icon_name("window-close-symbolic");
    GtkWidget *max_button = gtk_button_new_from_icon_name("window-maximize-symbolic");
    GtkWidget *min_button = gtk_button_new_from_icon_name("window-minimize-symbolic");
    
    gtk_widget_add_css_class(min_button, "flat");
    gtk_widget_add_css_class(max_button, "flat");
    gtk_widget_add_css_class(close_button, "flat");

    g_signal_connect_swapped(close_button, "clicked", G_CALLBACK(gtk_window_close), window);
    g_signal_connect_swapped(max_button, "clicked", G_CALLBACK(on_maximize_clicked), window);
    g_signal_connect_swapped(min_button, "clicked", G_CALLBACK(gtk_window_minimize), window);
    
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), close_button);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), max_button);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), min_button);

    return header;
}

