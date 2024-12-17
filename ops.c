#include <gtk/gtk.h>
#include <stdio.h>
#include <fcntl.h>
#include <dirent.h>

#include "support.h"

void on_directory_search(GtkEntry *entry, gpointer user_data) {
    GtkEntryBuffer *buffer = gtk_entry_get_buffer(entry);
    GtkStringList *files = GTK_STRING_LIST(user_data);
    const gchar *path = gtk_entry_buffer_get_text(buffer);
    static unsigned int i = 0;
    static unsigned int j = 0;

    for (j = 0; j < i; ++j) {
        gtk_string_list_remove(files, 0);
    }

    struct dirent *dent;
    DIR *dir = opendir(path);

    if (dir == NULL) {
        g_print("Invalid Path!\n");
        return;
    }
    /* Just printing file ina*/
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

// Function to set up the list items in the GtkListView
void setup_list_item(GtkListItemFactory *factory, GtkListItem *list_item, gpointer user_data) {
    GtkWidget *label = gtk_label_new(NULL);
    gtk_list_item_set_child(list_item, label);
}

// Function to bind data (file names) to list items
void bind_list_item(GtkListItemFactory *factory, GtkListItem *list_item, gpointer user_data) {
    GtkWidget *label = gtk_list_item_get_child(list_item);
    GtkStringObject *string_obj = gtk_list_item_get_item(list_item);
    gtk_label_set_text(GTK_LABEL(label), gtk_string_object_get_string(string_obj));
}

