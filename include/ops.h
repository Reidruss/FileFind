#ifndef OPS_H
#define OPS_H

void on_directory_search(GtkEntry *entry, gpointer user_data);
void setup_list_item(GtkListItemFactory *factory, GtkListItem *list_item, gpointer user_data);
void bind_list_item(GtkListItemFactory *factory, GtkListItem *list_item, gpointer user_data);
void on_create_file(GtkWidget *button, gpointer user_data);
void on_delete_file(GtkWidget *button, gpointer user_data);
void on_maximize_clicked(GtkWindow *window, GtkButton *button);
void on_entry_selected(GtkListView *view, guint position, gpointer user_data);
void on_home_button_clicked(GtkWidget *button, gpointer user_data);


#endif
