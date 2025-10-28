#include <gtk/gtk.h>
#include "ops.h"
#include "gui.h"


GtkWidget *create_flat_header_button(GtkWidget *window, const char *icon_name, GCallback function)
{
    GtkWidget *button = gtk_button_new_from_icon_name(icon_name);
    gtk_widget_add_css_class(button, "flat");
    g_signal_connect_swapped(button, "clicked", G_CALLBACK(function), window);

    return button;
}

GtkWidget *add_header_control_buttons(GtkWidget *window, GtkWidget *header) 
{ 

    GtkWidget *close_button = create_flat_header_button(window, "window-close-symbolic", 
                                                        G_CALLBACK(gtk_window_close));
    GtkWidget *maximize_button = create_flat_header_button(window, "window-maximize-symbolic", 
                                                        G_CALLBACK(on_maximize_clicked));
    GtkWidget *minimize_button = create_flat_header_button(window, "window-minimize-symbolic", 
                                                        G_CALLBACK(gtk_window_minimize));

    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), close_button); 
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), maximize_button); 
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), minimize_button); 

    return header; 
}

GtkWidget *create_main_window(GtkApplication *app)
{
    /* --- WINDOW SETUP --- */
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "FileFind");
    gtk_window_set_default_size(GTK_WINDOW(window), 1400, 900);
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);

    /* --- HEADER BAR --- */
    GtkWidget *header = gtk_header_bar_new();
    gtk_header_bar_set_show_title_buttons(GTK_HEADER_BAR(header), FALSE);
    gtk_header_bar_set_title_widget(GTK_HEADER_BAR(header), gtk_label_new("FileFind"));
    header = add_header_control_buttons(window, header);
    gtk_window_set_titlebar(GTK_WINDOW(window), header);

    /* --- MAIN VERTICAL BOX --- */
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_window_set_child(GTK_WINDOW(window), vbox);

    /* --- SEARCH BAR AT TOP --- */
    GtkWidget *search_entry = gtk_entry_new();
    gtk_widget_set_hexpand(search_entry, TRUE);
    gtk_entry_set_placeholder_text(GTK_ENTRY(search_entry), "Enter directory path...");
    gtk_box_append(GTK_BOX(vbox), search_entry);

    /* --- HORIZONTAL BOX FOR SIDEBAR + MAIN CONTENT --- */
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_vexpand(hbox, TRUE);
    gtk_box_append(GTK_BOX(vbox), hbox);

    /* --- SIDEBAR --- */
    GtkWidget *sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_size_request(sidebar, 100, -1);

    /* --- PLACES LABEL --- */
    GtkWidget *places_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(places_label), "<b>PLACES</b>");

    /* --- HOME BUTTON (icon + label) --- */
    GtkWidget *home_icon = gtk_image_new_from_icon_name("go-home-symbolic");
    GtkWidget *home_label = gtk_label_new("Home");

    GtkWidget *home_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_append(GTK_BOX(home_box), home_icon);
    gtk_box_append(GTK_BOX(home_box), home_label);

    GtkWidget *home_button_sb = gtk_button_new();
    gtk_button_set_has_frame(GTK_BUTTON(home_button_sb), FALSE);
    gtk_button_set_child(GTK_BUTTON(home_button_sb), home_box);   

    /* --- ADD TO SIDEBAR --- */
    gtk_box_append(GTK_BOX(sidebar), places_label);
    gtk_box_append(GTK_BOX(sidebar), home_button_sb);
    g_signal_connect(home_button_sb, "clicked", G_CALLBACK(on_home_button_clicked), window);

    /* -- ADD SIDEBAR TO MAIN BOX*/
    gtk_box_append(GTK_BOX(hbox), sidebar);

    /* --- SCROLLABLE FILE LIST --- */
    GtkStringList *file_list = gtk_string_list_new(NULL);
    GtkSingleSelection *single_selection = gtk_single_selection_new(G_LIST_MODEL(file_list));
    GtkListItemFactory *factory = gtk_signal_list_item_factory_new();

    g_signal_connect(factory, "setup", G_CALLBACK(setup_list_item), NULL);
    g_signal_connect(factory, "bind", G_CALLBACK(bind_list_item), NULL);

    GtkWidget *file_view = gtk_list_view_new(GTK_SELECTION_MODEL(single_selection), factory);

    /* Pass the search entry as user_data so the activate handler can update it */
    g_signal_connect(file_view, "activate", G_CALLBACK(on_entry_selected), search_entry);
    
    GtkWidget *scroll_window = gtk_scrolled_window_new();
    gtk_widget_set_hexpand(scroll_window, TRUE);
    gtk_widget_set_vexpand(scroll_window, TRUE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll_window), file_view);
    gtk_box_append(GTK_BOX(hbox), scroll_window);

    /* --- HOOK UP SEARCH --- */
    gtk_editable_set_text(GTK_EDITABLE(search_entry), g_get_home_dir());
    g_signal_connect(search_entry, "activate", G_CALLBACK(on_directory_search), file_list);
    on_directory_search(GTK_ENTRY(search_entry), file_list);

    /* --- STORE REFERENCES --- */
    g_object_set_data(G_OBJECT(window), "search_entry", search_entry);
    g_object_set_data(G_OBJECT(window), "file_list", file_list);

    /* --- CONNECT HOME BUTTON ---*/
    g_signal_connect(home_button_sb, "clicked", G_CALLBACK(on_home_button_clicked), window);


    return window;
}