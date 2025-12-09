#ifndef GUI_H
#define GUI_H

GtkWidget *create_main_window(GtkApplication *app);
GtkWidget *add_header_control_buttons(GtkWidget *window, GtkWidget *header);
GtkWidget *add_sidebar_button_with_icon_and_label(const char *icon_name, const char *label_name);

#endif
