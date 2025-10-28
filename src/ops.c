#include <gtk/gtk.h>
#include <dirent.h>
#include <string.h>
#include "ops.h"

static const char *cur_filepath;

void on_directory_search(GtkEntry *entry, gpointer user_data) 
{
    GtkEntryBuffer *buffer;
    GtkStringList *files;
    const char *prev_path;

    buffer = gtk_entry_get_buffer(entry);
    cur_filepath = gtk_entry_buffer_get_text(buffer);
    files = GTK_STRING_LIST(user_data);
    //const char *path = gtk_entry_buffer_get_text(buffer);

    static unsigned int i = 0;
    unsigned int j;

    /* Erasing old entries to replace with new entries. */
    for (int j = 0; j < i ; j++) 
    {
        gtk_string_list_remove(files, 0);
    }
    
    DIR *dir = opendir(cur_filepath);
    if (!dir) 
    {
        g_print("Invalid Path: %s\n", cur_filepath);
        i = 0;
        return;
    }
    prev_path = cur_filepath;
    struct dirent *dent;
    i = 0;
    while ((dent = readdir(dir)) != NULL) 
    {
        if (strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0 || dent->d_name[0] == '.') 
        {
            continue;
        }
        gtk_string_list_append(files, dent->d_name);
        i++;
    }
    closedir(dir);
}

void setup_list_item(GtkListItemFactory *factory, GtkListItem *list_item, gpointer user_data) 
{
    GtkWidget *label = gtk_label_new(NULL);
    gtk_list_item_set_child(list_item, label);
}

void bind_list_item(GtkListItemFactory *factory, GtkListItem *list_item, gpointer user_data) 
{
    GtkWidget *label = gtk_list_item_get_child(list_item);
    GtkStringObject *string_obj = gtk_list_item_get_item(list_item);
    gtk_label_set_text(GTK_LABEL(label), gtk_string_object_get_string(string_obj));
}

void on_create_file(GtkWidget *button, gpointer user_data) 
{
    GtkWidget *dialog, *entry, *content_area;
    GtkStringList *files = GTK_STRING_LIST(user_data);

    dialog = gtk_dialog_new_with_buttons(
        "Create File",
        NULL,
        GTK_DIALOG_MODAL,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Create", GTK_RESPONSE_ACCEPT,
        NULL
    );

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Enter file name...");
    gtk_box_append(GTK_BOX(content_area), entry);
    gtk_widget_show(entry);

    g_signal_connect(dialog, "response", G_CALLBACK(on_create_file_response), files);

    gtk_widget_show(dialog);
}

/* === FILE OPERATIONS === */

void on_create_file_response(GtkDialog *dialog, int response_id, gpointer user_data) 
{
    GtkWidget *entry = gtk_widget_get_first_child(gtk_dialog_get_content_area(dialog));
    GtkStringList *files = GTK_STRING_LIST(user_data);

    if (response_id == GTK_RESPONSE_ACCEPT) 
    {
        GtkEntryBuffer *buffer = gtk_entry_get_buffer(GTK_ENTRY(entry));
        const gchar *filename = gtk_entry_buffer_get_text(buffer);

        if (strlen(filename) > 0) 
        {
            FILE *file = fopen(filename, "w");
            if (file) 
            {
                fclose(file);
                gtk_string_list_append(files, filename);
            } 
            else 
            {
                g_print("Failed to create file: %s\n", filename);
            }
        } 
        else 
        {
            g_print("No file name provided.\n");
        }
    }

    gtk_window_destroy(GTK_WINDOW(dialog));
}


void on_delete_file(GtkWidget *button, gpointer user_data) 
{
    GtkSingleSelection *single_selection = GTK_SINGLE_SELECTION(user_data);

    /* Finding the postion of the selected file. */
    guint position = gtk_single_selection_get_selected(single_selection);
    if (position == GTK_INVALID_LIST_POSITION) 
    {
        g_print("No file selected for deletion.\n");
        return;
    }

    GtkStringObject *string_obj = GTK_STRING_OBJECT(gtk_single_selection_get_selected_item(single_selection));
    const gchar *file_name = gtk_string_object_get_string(string_obj);

    /* Deleting the file if able to and removing from the StringList. */
    if (remove(file_name) == 0) 
    {
        g_print("File deleted: %s\n", file_name);
        gtk_string_list_remove(GTK_STRING_LIST(gtk_single_selection_get_model(single_selection)), position);
    } 
    else 
    {
        g_print("Failed to delete file: %s\n", file_name);
    }
}

void open_file(char *filepath)
{
    g_print("Opening filepath: %s\n", filepath);
}

void on_maximize_clicked(GtkWindow *window, GtkButton *button)
{
    gtk_window_is_maximized(window) ? gtk_window_unmaximize(window) : gtk_window_maximize(window);
}


/* THIS DOES NOT WORK AT THE MOMENT */
void on_entry_selected(GtkListView *list, guint position, gpointer user_data)
{
    GtkEntry *search_entry = GTK_ENTRY(user_data);

    GtkSelectionModel *model = gtk_list_view_get_model(list);
    GtkSingleSelection *sel = GTK_SINGLE_SELECTION(model);
    GtkStringList *strings = GTK_STRING_LIST(gtk_single_selection_get_model(sel));

    if (position == GTK_INVALID_LIST_POSITION) 
    {
        g_print("Invalid selection position\n");
        return;
    }
    
    const char *name = gtk_string_list_get_string(strings, position);
    if (!name) 
    {
        g_print("Failed to get selected name\n");
        return;
    }

    /* Build the new path from the current entry text and the selected name */
    GtkEntryBuffer *buffer = gtk_entry_get_buffer(search_entry);
    const char *cur_path = gtk_entry_buffer_get_text(buffer);
    gchar *new_path = g_build_filename(cur_path, name, NULL);

    /* If it's a directory, update the entry and refresh the list. Otherwise open the file. */
    if (g_file_test(new_path, G_FILE_TEST_IS_DIR)) 
    {
        GtkEntryBuffer *new_buffer = gtk_entry_buffer_new(new_path, -1);
        gtk_entry_set_buffer(search_entry, new_buffer);
        g_object_unref(new_buffer);
        on_directory_search(search_entry, strings);
    } 
    else 
    {
        open_file(new_path);
    }

    g_free(new_path);
}


void on_home_button_clicked(GtkWidget *button, gpointer user_data)
{
    GtkWindow *window = GTK_WINDOW(user_data);
    GtkEntry *search_entry = g_object_get_data(G_OBJECT(window), "search_entry");
    GtkStringList *file_list = g_object_get_data(G_OBJECT(window), "file_list");

    gtk_editable_set_text(GTK_EDITABLE(search_entry), g_get_home_dir());
    on_directory_search(search_entry, file_list);
}

