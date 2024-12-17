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
    
    for (int j = 0; j < i ; j++) {
        gtk_string_list_remove(files, 0);
    }

    // Open directory and append file names
    DIR *dir = opendir(path);
    if (!dir) {
        g_print("Invalid Path: %s\n", path);
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
