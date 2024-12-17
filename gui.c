#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "support.h"


void activate(GtkApplication* app, gpointer user_data){

    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *search_bar;
    GtkWidget *file_view;
    GtkWidget *scroll_window;
    //char entry[64];

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "FileFind");
    gtk_window_set_default_size(GTK_WINDOW (window), 1400, 900);
    g_signal_connect(window, "close-request", G_CALLBACK(gtk_window_destroy), NULL);

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_window_set_child(GTK_WINDOW(window), box);

    search_bar = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(search_bar), "Enter directory path...");
    gtk_box_append(GTK_BOX(box), search_bar);

    GtkStringList *file_list = gtk_string_list_new(NULL);

    GtkSingleSelection *single_selection = gtk_single_selection_new(G_LIST_MODEL(file_list));
    GtkSelectionModel *selection = GTK_SELECTION_MODEL(single_selection);

    GtkListItemFactory *factory = gtk_signal_list_item_factory_new();
    g_signal_connect(factory, "setup", G_CALLBACK(setup_list_item), NULL);
    g_signal_connect(factory, "bind", G_CALLBACK(bind_list_item), NULL);

    file_view = gtk_list_view_new(selection, factory);

    scroll_window = gtk_scrolled_window_new();
    //gtk_widget_set_size_request(scroll_window, -1, -1);
    gtk_widget_set_vexpand(scroll_window, TRUE);    

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll_window), file_view);

    gtk_box_append(GTK_BOX(box), scroll_window);

    g_signal_connect(search_bar, "activate", G_CALLBACK(on_directory_search), file_list);
    
    

    gtk_widget_show(window);

}

