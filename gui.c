#include <stdio.h>
#include <gtk/gtk.h>
#include "ops.h"

GtkWidget *create_main_window(GtkApplication *app) {
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *search_bar;
    GtkWidget *file_view;
    GtkWidget *scroll_window;

    /* Setting up the window. */
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "FileFind");
    gtk_window_set_default_size(GTK_WINDOW(window), 1400, 900);

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_window_set_child(GTK_WINDOW(window), box);

    /* Setting up Search Bar for directories. */
    search_bar = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(search_bar), "Enter directory path...");
    gtk_box_append(GTK_BOX(box), search_bar);

    /* Setting up list for viewing files in a directory. */
    GtkStringList *file_list = gtk_string_list_new(NULL);
    GtkSingleSelection *single_selection = gtk_single_selection_new(G_LIST_MODEL(file_list));
    GtkListItemFactory *factory = gtk_signal_list_item_factory_new();

    g_signal_connect(factory, "setup", G_CALLBACK(setup_list_item), NULL);
    g_signal_connect(factory, "bind", G_CALLBACK(bind_list_item), NULL);

    file_view = gtk_list_view_new(GTK_SELECTION_MODEL(single_selection), factory);

    /* Adding scroll function. */
    scroll_window = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scroll_window, TRUE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll_window), file_view);
    gtk_scrolled_window_set_has_frame(GTK_SCROLLED_WINDOW(scroll_window), TRUE);

    /* Add Create and Delete buttons */
    GtkWidget *create_button = gtk_button_new_with_label("Create File");
    GtkWidget *delete_button = gtk_button_new_with_label("Delete File");

    gtk_box_append(GTK_BOX(box), create_button);
    gtk_box_append(GTK_BOX(box), delete_button);

    // Connect signals to their respective callbacks
    g_signal_connect(create_button, "clicked", G_CALLBACK(on_create_file), file_list);
    g_signal_connect(delete_button, "clicked", G_CALLBACK(on_delete_file), single_selection);

    gtk_box_append(GTK_BOX(box), scroll_window);

    /* Search bar signal. */ 
    g_signal_connect(search_bar, "activate", G_CALLBACK(on_directory_search), file_list);

    return window;
}



