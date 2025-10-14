#ifndef OPS_H
#define OPS_H

void on_directory_search(GtkEntry *entry, gpointer user_data);
void setup_list_item(GtkListItemFactory *factory, GtkListItem *list_item, gpointer user_data);
void bind_list_item(GtkListItemFactory *factory, GtkListItem *list_item, gpointer user_data);

#endif
