#include <gtk/gtk.h>
#include <dirent.h>
#include <string.h>
#include "ops.h"

void on_directory_search(GtkEntry *entry, gpointer user_data) {
    GtkEntryBuffer *buffer = gtk_entry_get_buffer(entry);
    const gchar *path = gtk_entry_buffer_get_text(buffer);
    GtkStringList *files = GTK_STRING_LIST(user_data);

    static unsigned int i = 0;
    unsigned int j;

    /* Erasing the list to add new files. */
    for (int j = 0; j < i ; j++) {
        gtk_string_list_remove(files, 0);
    }
    
    DIR *dir = opendir(path);
    if (!dir) {
        g_print("Invalid Path: %s\n", path);
        i = 0;
        return;
    }

    struct dirent *dent;
    i = 0;
    while ((dent = readdir(dir)) != NULL) {
        if (strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0) {
            continue;
        }
        gtk_string_list_append(files, dent->d_name);
        i++;
    }
    closedir(dir);
}

void setup_list_item(GtkListItemFactory *factory, GtkListItem *list_item, gpointer user_data) {
    GtkWidget *label = gtk_label_new(NULL);
    gtk_list_item_set_child(list_item, label);
}

void bind_list_item(GtkListItemFactory *factory, GtkListItem *list_item, gpointer user_data) {
    GtkWidget *label = gtk_list_item_get_child(list_item);
    GtkStringObject *string_obj = gtk_list_item_get_item(list_item);
    gtk_label_set_text(GTK_LABEL(label), gtk_string_object_get_string(string_obj));
}

void on_create_file(GtkWidget *button, gpointer user_data) {
    GtkWidget *dialog, *entry, *content_area;
    GtkStringList *files = GTK_STRING_LIST(user_data);

    // Create the dialog
    dialog = gtk_dialog_new_with_buttons(
        "Create File",
        NULL,
        GTK_DIALOG_MODAL,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Create", GTK_RESPONSE_ACCEPT,
        NULL);

    // Add an entry to the content area
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Enter file name...");
    gtk_box_append(GTK_BOX(content_area), entry);
    gtk_widget_show(entry);

    // Connect the response signal
    g_signal_connect(dialog, "response", G_CALLBACK(on_create_file_response), files);

    gtk_widget_show(dialog);
}

void on_create_file_response(GtkDialog *dialog, int response_id, gpointer user_data) {
    GtkWidget *entry = gtk_widget_get_first_child(gtk_dialog_get_content_area(dialog));
    GtkStringList *files = GTK_STRING_LIST(user_data);

    if (response_id == GTK_RESPONSE_ACCEPT) {
        GtkEntryBuffer *buffer = gtk_entry_get_buffer(GTK_ENTRY(entry));
        const gchar *filename = gtk_entry_buffer_get_text(buffer);

        if (strlen(filename) > 0) {
            FILE *file = fopen(filename, "w");
            if (file) {
                fclose(file);
                gtk_string_list_append(files, filename);
            } else {
                g_print("Failed to create file: %s\n", filename);
            }
        } else {
            g_print("No file name provided.\n");
        }
    }

    gtk_window_destroy(GTK_WINDOW(dialog));
}


void on_delete_file(GtkWidget *button, gpointer user_data) {
    GtkSingleSelection *single_selection = GTK_SINGLE_SELECTION(user_data);

    /* Finding the postion of the selected file. */
    guint position = gtk_single_selection_get_selected(single_selection);
    if (position == GTK_INVALID_LIST_POSITION) {
        g_print("No file selected for deletion.\n");
        return;
    }

    GtkStringObject *string_obj = GTK_STRING_OBJECT(gtk_single_selection_get_selected_item(single_selection));
    const gchar *file_name = gtk_string_object_get_string(string_obj);

    /* Deleting the file if able to and removing from the StringList. */
    if (remove(file_name) == 0) {
        g_print("File deleted: %s\n", file_name);
        gtk_string_list_remove(GTK_STRING_LIST(gtk_single_selection_get_model(single_selection)), position);
    } else {
        g_print("Failed to delete file: %s\n", file_name);
    }
    
}
