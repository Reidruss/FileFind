#ifndef SUPPORT_H
#define SUPPORT_H

// gui
void activate(GtkApplication*, gpointer);

// ops
void print_hello(GtkWidget *, gpointer);
void on_directory_search(GtkEntry *, gpointer);
void setup_list_item(GtkListItemFactory *, GtkListItem *, gpointer);
void bind_list_item(GtkListItemFactory *, GtkListItem *, gpointer);


#endif