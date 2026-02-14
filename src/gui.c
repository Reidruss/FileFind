#include <gtk/gtk.h>
#include "ops.h"
#include "gui.h"

#define WINDOW_DEFAULT_WIDTH 1400
#define WINDOW_DEFAULT_HEIGHT 900
#define ROOT_SPACING 10
#define ACTIONS_SPACING 8
#define SIDEBAR_WIDTH 100

typedef struct {
    const char *icon_name;
    const char *label;
    GCallback callback;
} SidebarItem;

static GtkWidget *build_top_bar(GtkWidget *vbox, GtkStringList *file_list)
{
    GtkWidget *search_entry = gtk_entry_new();
    GtkWidget *actions_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, ACTIONS_SPACING);
    GtkWidget *create_file_button = gtk_button_new_with_label("Create File");

    gtk_widget_set_hexpand(search_entry, TRUE);
    gtk_entry_set_placeholder_text(GTK_ENTRY(search_entry), "Enter directory path...");
    gtk_box_append(GTK_BOX(vbox), search_entry);

    gtk_box_append(GTK_BOX(actions_box), create_file_button);
    gtk_box_append(GTK_BOX(vbox), actions_box);

    g_signal_connect(create_file_button, "clicked", G_CALLBACK(on_create_file), file_list);

    return search_entry;
}

static GtkWidget *build_sidebar(GtkWidget *window)
{
    GtkWidget *sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, ROOT_SPACING);
    GtkWidget *places_label = gtk_label_new(NULL);

    const SidebarItem sidebar_items[] = {
        { "go-home-symbolic", "Home", G_CALLBACK(on_home_button_clicked) },
        { "user-desktop-symbolic", "Desktop", G_CALLBACK(on_desktop_button_clicked) },
        { "x-office-document-symbolic", "Documents", G_CALLBACK(on_documents_button_clicked) },
        { "folder-music-symbolic", "Music", G_CALLBACK(on_music_button_clicked) },
        { "folder-pictures-symbolic", "Pictures", G_CALLBACK(on_pictures_button_clicked) },
        { "folder-pictures-symbolic", "Videos", G_CALLBACK(on_videos_button_clicked) },
        { "user-trash-symbolic", "Trash", G_CALLBACK(on_trash_button_clicked) }
    };

    gtk_widget_set_size_request(sidebar, SIDEBAR_WIDTH, -1);
    gtk_label_set_markup(GTK_LABEL(places_label), "<b>FILES</b>");
    gtk_box_append(GTK_BOX(sidebar), places_label);

    for (guint i = 0; i < G_N_ELEMENTS(sidebar_items); i++)
    {
        GtkWidget *button = add_sidebar_button_with_icon_and_label(
            sidebar_items[i].icon_name,
            sidebar_items[i].label
        );
        gtk_box_append(GTK_BOX(sidebar), button);
        g_signal_connect(button, "clicked", sidebar_items[i].callback, window);
    }

    return sidebar;
}

static GtkWidget *build_file_view(GtkStringList *file_list, GtkWidget *search_entry)
{
    GtkSingleSelection *single_selection = gtk_single_selection_new(G_LIST_MODEL(file_list));
    GtkListItemFactory *factory = gtk_signal_list_item_factory_new();
    GtkWidget *file_view = NULL;

    g_signal_connect(factory, "setup", G_CALLBACK(setup_list_item), file_list);
    g_signal_connect(factory, "bind", G_CALLBACK(bind_list_item), NULL);

    file_view = gtk_list_view_new(GTK_SELECTION_MODEL(single_selection), factory);
    g_signal_connect(file_view, "activate", G_CALLBACK(on_entry_selected), search_entry);

    return file_view;
}

static GtkWidget *build_main_content(GtkWidget *window, GtkWidget *vbox, GtkWidget *search_entry, GtkStringList *file_list)
{
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, ROOT_SPACING);
    GtkWidget *sidebar = build_sidebar(window);
    GtkWidget *file_view = build_file_view(file_list, search_entry);
    GtkWidget *scroll_window = gtk_scrolled_window_new();

    gtk_widget_set_vexpand(hbox, TRUE);
    gtk_box_append(GTK_BOX(vbox), hbox);

    gtk_box_append(GTK_BOX(hbox), sidebar);

    gtk_widget_set_hexpand(scroll_window, TRUE);
    gtk_widget_set_vexpand(scroll_window, TRUE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll_window), file_view);
    gtk_box_append(GTK_BOX(hbox), scroll_window);

    return hbox;
}


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

GtkWidget *add_sidebar_button_with_icon_and_label(const char *icon_name, const char *label_name)
{
    GtkWidget *new_icon = gtk_image_new_from_icon_name(icon_name);
    GtkWidget *new_label = gtk_label_new(label_name);

    GtkWidget *new_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_append(GTK_BOX(new_box), new_icon);
    gtk_box_append(GTK_BOX(new_box), new_label);

    GtkWidget *new_button_sb = gtk_button_new();
    gtk_button_set_has_frame(GTK_BUTTON(new_button_sb), FALSE);
    gtk_button_set_child(GTK_BUTTON(new_button_sb), new_box);

    return new_button_sb;
}

GtkWidget *create_main_window(GtkApplication *app)
{
    GtkWidget *window = gtk_application_window_new(app);
    GtkWidget *vbox;
    GtkWidget *header;
    GtkWidget *search_entry;
    GtkStringList *file_list;

    gtk_window_set_title(GTK_WINDOW(window), "FileFind");
    gtk_window_set_default_size(GTK_WINDOW(window), WINDOW_DEFAULT_WIDTH, WINDOW_DEFAULT_HEIGHT);
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);

    header = gtk_header_bar_new();
    gtk_header_bar_set_show_title_buttons(GTK_HEADER_BAR(header), FALSE);
    gtk_header_bar_set_title_widget(GTK_HEADER_BAR(header), gtk_label_new("FileFind"));
    header = add_header_control_buttons(window, header);
    gtk_window_set_titlebar(GTK_WINDOW(window), header);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, ROOT_SPACING);
    gtk_window_set_child(GTK_WINDOW(window), vbox);

    file_list = gtk_string_list_new(NULL);
    search_entry = build_top_bar(vbox, file_list);
    build_main_content(window, vbox, search_entry, file_list);

    gtk_editable_set_text(GTK_EDITABLE(search_entry), g_get_home_dir());
    g_signal_connect(search_entry, "activate", G_CALLBACK(on_directory_search), file_list);
    on_directory_search(GTK_ENTRY(search_entry), file_list);

    g_object_set_data(G_OBJECT(window), "search_entry", search_entry);
    g_object_set_data(G_OBJECT(window), "file_list", file_list);

    return window;
}