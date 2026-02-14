#include <gtk/gtk.h>
#include <dirent.h>
#include <string.h>
#include <glib/gstdio.h>
#include "ops.h"

#define ITEM_ROW_SPACING 12
#define CONTEXT_MENU_SPACING 0
#define DIALOG_CONTENT_SPACING 10
#define DIALOG_BUTTON_SPACING 10
#define DIALOG_MARGIN 20
#define CREATE_DIALOG_WIDTH 300
#define RENAME_DIALOG_WIDTH 320

static char *cur_filepath = NULL;
static void open_file(char *filepath);

static GtkWidget *create_modal_input_dialog(
    const gchar *title,
    const gchar *placeholder,
    const gchar *initial_text,
    const gchar *accept_label,
    GtkWidget **entry_out,
    GtkWidget **accept_button_out
)
{
    GtkWidget *window = gtk_window_new();
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, DIALOG_CONTENT_SPACING);
    GtkWidget *entry = gtk_entry_new();
    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DIALOG_BUTTON_SPACING);
    GtkWidget *cancel_btn = gtk_button_new_with_label("Cancel");
    GtkWidget *accept_btn = gtk_button_new_with_label(accept_label);

    gtk_window_set_title(GTK_WINDOW(window), title);
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_window_set_default_size(
        GTK_WINDOW(window),
        g_strcmp0(title, "Rename") == 0 ? RENAME_DIALOG_WIDTH : CREATE_DIALOG_WIDTH,
        -1
    );

    gtk_widget_set_margin_top(box, DIALOG_MARGIN);
    gtk_widget_set_margin_bottom(box, DIALOG_MARGIN);
    gtk_widget_set_margin_start(box, DIALOG_MARGIN);
    gtk_widget_set_margin_end(box, DIALOG_MARGIN);
    gtk_window_set_child(GTK_WINDOW(window), box);

    if (placeholder)
    {
        gtk_entry_set_placeholder_text(GTK_ENTRY(entry), placeholder);
    }
    if (initial_text)
    {
        gtk_editable_set_text(GTK_EDITABLE(entry), initial_text);
    }

    gtk_box_append(GTK_BOX(box), entry);

    gtk_widget_set_halign(button_box, GTK_ALIGN_END);
    gtk_box_append(GTK_BOX(button_box), cancel_btn);
    gtk_box_append(GTK_BOX(button_box), accept_btn);
    gtk_box_append(GTK_BOX(box), button_box);

    g_signal_connect_swapped(cancel_btn, "clicked", G_CALLBACK(gtk_window_destroy), window);

    if (entry_out)
    {
        *entry_out = entry;
    }
    if (accept_button_out)
    {
        *accept_button_out = accept_btn;
    }

    return window;
}

static gboolean resolve_list_item_data(
    GtkListItem *list_item,
    const gchar **item_name,
    GtkStringList **files
)
{
    if (!list_item || !item_name || !files)
    {
        return FALSE;
    }

    GtkStringObject *string_obj = GTK_STRING_OBJECT(gtk_list_item_get_item(list_item));
    if (!string_obj)
    {
        return FALSE;
    }

    *item_name = gtk_string_object_get_string(string_obj);
    *files = g_object_get_data(G_OBJECT(list_item), "file_list");

    return (*item_name != NULL && *files != NULL);
}

static GtkEntry *get_search_entry_from_widget(GtkWidget *widget)
{
    GtkRoot *root = gtk_widget_get_root(widget);
    if (!root)
    {
        return NULL;
    }

    return g_object_get_data(G_OBJECT(root), "search_entry");
}

static void navigate_window_to_path(GtkWindow *window, const gchar *path)
{
    GtkEntry *search_entry = NULL;
    GtkStringList *file_list = NULL;

    if (!window || !path)
    {
        return;
    }

    search_entry = g_object_get_data(G_OBJECT(window), "search_entry");
    file_list = g_object_get_data(G_OBJECT(window), "file_list");
    if (!search_entry || !file_list)
    {
        return;
    }

    gtk_editable_set_text(GTK_EDITABLE(search_entry), path);
    on_directory_search(search_entry, file_list);
}

static void navigate_window_to_special_dir(GtkWindow *window, GUserDirectory dir)
{
    const gchar *path = g_get_user_special_dir(dir);
    if (!path)
    {
        return;
    }

    navigate_window_to_path(window, path);
}

static void open_entry_by_name(const gchar *name, GtkStringList *files, GtkEntry *search_entry)
{
    if (!name || !files || !cur_filepath)
    {
        return;
    }

    gchar *new_path = g_build_filename(cur_filepath, name, NULL);
    if (g_file_test(new_path, G_FILE_TEST_IS_DIR))
    {
        if (search_entry)
        {
            gtk_editable_set_text(GTK_EDITABLE(search_entry), new_path);
            on_directory_search(search_entry, files);
        }
    }
    else
    {
        open_file(new_path);
    }

    g_free(new_path);
}

static gboolean delete_entry_by_name(const gchar *file_name, GtkStringList *files)
{
    if (!file_name || !files)
    {
        return FALSE;
    }

    if (!cur_filepath)
    {
        g_print("No active directory selected.\n");
        return FALSE;
    }

    gchar *full_path = g_build_filename(cur_filepath, file_name, NULL);
    gboolean is_directory = g_file_test(full_path, G_FILE_TEST_IS_DIR);
    int delete_status = is_directory ? g_rmdir(full_path) : g_remove(full_path);
    if (delete_status != 0)
    {
        g_print("Failed to delete file: %s\n", full_path);
        g_free(full_path);
        return FALSE;
    }

    guint count = g_list_model_get_n_items(G_LIST_MODEL(files));
    for (guint idx = 0; idx < count; idx++)
    {
        const char *entry_name = gtk_string_list_get_string(files, idx);
        if (g_strcmp0(entry_name, file_name) == 0)
        {
            gtk_string_list_remove(files, idx);
            break;
        }
    }

    g_print("File deleted: %s\n", full_path);
    g_free(full_path);
    return TRUE;
}

static void on_context_delete_clicked(GtkWidget *button, gpointer user_data)
{
    GtkPopover *popover = GTK_POPOVER(user_data);
    GtkListItem *list_item = g_object_get_data(G_OBJECT(button), "list_item");
    const gchar *item_name = NULL;
    GtkStringList *files = NULL;

    if (!resolve_list_item_data(list_item, &item_name, &files))
    {
        return;
    }

    delete_entry_by_name(item_name, files);
    gtk_popover_popdown(popover);
}

static void on_context_open_clicked(GtkWidget *button, gpointer user_data)
{
    GtkPopover *popover = GTK_POPOVER(user_data);
    GtkListItem *list_item = g_object_get_data(G_OBJECT(button), "list_item");
    const gchar *item_name = NULL;
    GtkStringList *files = NULL;

    if (!resolve_list_item_data(list_item, &item_name, &files))
    {
        return;
    }

    open_entry_by_name(item_name, files, get_search_entry_from_widget(GTK_WIDGET(popover)));
    gtk_popover_popdown(popover);
}

static void on_rename_accept(GtkWidget *button, gpointer user_data)
{
    GtkWindow *window = GTK_WINDOW(user_data);
    GtkWidget *entry = g_object_get_data(G_OBJECT(window), "entry");
    GtkStringList *files = g_object_get_data(G_OBJECT(window), "files");
    GtkEntry *search_entry = g_object_get_data(G_OBJECT(window), "search_entry");
    gchar *old_name = g_object_get_data(G_OBJECT(window), "old_name");

    if (!entry || !files || !old_name || !cur_filepath)
    {
        gtk_window_destroy(window);
        return;
    }

    const gchar *new_name = gtk_editable_get_text(GTK_EDITABLE(entry));
    if (!new_name || strlen(new_name) == 0 || g_strcmp0(new_name, old_name) == 0)
    {
        gtk_window_destroy(window);
        return;
    }

    gchar *old_path = g_build_filename(cur_filepath, old_name, NULL);
    gchar *new_path = g_build_filename(cur_filepath, new_name, NULL);

    if (g_rename(old_path, new_path) == 0)
    {
        if (search_entry)
        {
            on_directory_search(search_entry, files);
        }
        else
        {
            guint count = g_list_model_get_n_items(G_LIST_MODEL(files));
            for (guint idx = 0; idx < count; idx++)
            {
                const char *entry_name = gtk_string_list_get_string(files, idx);
                if (g_strcmp0(entry_name, old_name) == 0)
                {
                    gtk_string_list_remove(files, idx);
                    gtk_string_list_append(files, new_name);
                    break;
                }
            }
        }
    }
    else
    {
        g_print("Failed to rename: %s -> %s\n", old_path, new_path);
    }

    g_free(old_path);
    g_free(new_path);
    gtk_window_destroy(window);
}

static void on_context_rename_clicked(GtkWidget *button, gpointer user_data)
{
    GtkPopover *popover = GTK_POPOVER(user_data);
    GtkListItem *list_item = g_object_get_data(G_OBJECT(button), "list_item");
    const gchar *old_name = NULL;
    GtkStringList *files = NULL;
    GtkEntry *search_entry = NULL;

    if (!resolve_list_item_data(list_item, &old_name, &files))
    {
        return;
    }

    search_entry = get_search_entry_from_widget(GTK_WIDGET(popover));

    GtkWidget *entry = NULL;
    GtkWidget *rename_btn = NULL;
    GtkWidget *window = create_modal_input_dialog(
        "Rename",
        NULL,
        old_name,
        "Rename",
        &entry,
        &rename_btn
    );

    g_signal_connect(rename_btn, "clicked", G_CALLBACK(on_rename_accept), window);

    g_object_set_data(G_OBJECT(window), "entry", entry);
    g_object_set_data(G_OBJECT(window), "files", files);
    g_object_set_data(G_OBJECT(window), "search_entry", search_entry);
    g_object_set_data_full(G_OBJECT(window), "old_name", g_strdup(old_name), g_free);

    gtk_widget_set_visible(window, true);
    gtk_popover_popdown(popover);
}

static void on_row_right_click(GtkGestureClick *gesture, gint n_press, gdouble x, gdouble y, gpointer user_data)
{
    if (n_press != 1)
    {
        return;
    }

    GtkListItem *list_item = GTK_LIST_ITEM(user_data);
    GtkWidget *box = gtk_list_item_get_child(list_item);
    GtkPopover *popover = g_object_get_data(G_OBJECT(box), "context_popover");
    GtkStringObject *string_obj = GTK_STRING_OBJECT(gtk_list_item_get_item(list_item));
    const gchar *file_name = NULL;

    if (!popover)
    {
        return;
    }

    if (!string_obj || !cur_filepath)
    {
        return;
    }

    file_name = gtk_string_object_get_string(string_obj);
    if (!file_name)
    {
        return;
    }

    gchar *full_path = g_build_filename(cur_filepath, file_name, NULL);
    gboolean is_directory = g_file_test(full_path, G_FILE_TEST_IS_DIR);
    g_free(full_path);

    if (is_directory)
    {
        return;
    }

    GdkRectangle rect = { (int)x, (int)y, 1, 1 };
    gtk_popover_set_pointing_to(popover, &rect);
    gtk_popover_popup(popover);

    gtk_gesture_set_state(GTK_GESTURE(gesture), GTK_EVENT_SEQUENCE_CLAIMED);
}

static void clear_string_list(GtkStringList *files)
{
    guint count = g_list_model_get_n_items(G_LIST_MODEL(files));
    while (count > 0)
    {
        gtk_string_list_remove(files, count - 1);
        count--;
    }
}

void on_directory_search(GtkEntry *entry, gpointer user_data)
{
    GtkEntryBuffer *buffer;
    GtkStringList *files;

    buffer = gtk_entry_get_buffer(entry);
    const char *path_text = gtk_entry_buffer_get_text(buffer);

    /* Update current path safely */
    if (cur_filepath) {
        g_free(cur_filepath);
    }
    cur_filepath = g_strdup(path_text);

    files = GTK_STRING_LIST(user_data);

    clear_string_list(files);

    DIR *dir = opendir(cur_filepath);
    if (!dir)
    {
        g_print("Invalid Path: %s\n", cur_filepath);
        return;
    }

    struct dirent *dent;
    while ((dent = readdir(dir)) != NULL)
    {
        if (strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0 || dent->d_name[0] == '.')
        {
            continue;
        }
        gtk_string_list_append(files, dent->d_name);
    }
    closedir(dir);
}

void setup_list_item(GtkListItemFactory *factory, GtkListItem *list_item, gpointer user_data)
{
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, ITEM_ROW_SPACING);
    GtkWidget *icon = gtk_image_new();
    GtkWidget *label = gtk_label_new(NULL);
    GtkWidget *menu_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, CONTEXT_MENU_SPACING);
    GtkWidget *open_item_btn = gtk_button_new_with_label("Open");
    GtkWidget *rename_item_btn = gtk_button_new_with_label("Rename");
    GtkWidget *delete_item_btn = gtk_button_new_with_label("Delete");
    GtkPopover *popover = GTK_POPOVER(gtk_popover_new());
    GtkGesture *secondary_click = gtk_gesture_click_new();

    gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(secondary_click), GDK_BUTTON_SECONDARY);
    g_signal_connect(secondary_click, "pressed", G_CALLBACK(on_row_right_click), list_item);
    gtk_widget_add_controller(box, GTK_EVENT_CONTROLLER(secondary_click));

    gtk_box_append(GTK_BOX(box), icon);
    gtk_box_append(GTK_BOX(box), label);

    gtk_widget_set_parent(GTK_WIDGET(popover), box);
    gtk_box_append(GTK_BOX(menu_box), open_item_btn);
    gtk_box_append(GTK_BOX(menu_box), rename_item_btn);
    gtk_box_append(GTK_BOX(menu_box), delete_item_btn);
    gtk_popover_set_child(popover, menu_box);
    g_signal_connect(open_item_btn, "clicked", G_CALLBACK(on_context_open_clicked), popover);
    g_signal_connect(rename_item_btn, "clicked", G_CALLBACK(on_context_rename_clicked), popover);
    g_signal_connect(delete_item_btn, "clicked", G_CALLBACK(on_context_delete_clicked), popover);

    g_object_set_data(G_OBJECT(open_item_btn), "list_item", list_item);
    g_object_set_data(G_OBJECT(rename_item_btn), "list_item", list_item);
    g_object_set_data(G_OBJECT(delete_item_btn), "list_item", list_item);
    g_object_set_data(G_OBJECT(box), "context_popover", popover);
    g_object_set_data(G_OBJECT(list_item), "file_list", user_data);

    gtk_list_item_set_child(list_item, box);
}

void bind_list_item(GtkListItemFactory *factory, GtkListItem *list_item, gpointer user_data)
{
    GtkWidget *box = gtk_list_item_get_child(list_item);
    GtkWidget *icon = gtk_widget_get_first_child(box);
    GtkWidget *label = gtk_widget_get_next_sibling(icon);

    GtkStringObject *string_obj = gtk_list_item_get_item(list_item);
    const char *filename = gtk_string_object_get_string(string_obj);

    gtk_label_set_text(GTK_LABEL(label), filename);

    /* Determine icon based on file type */
    char *full_path = g_build_filename(cur_filepath, filename, NULL);

    if (g_file_test(full_path, G_FILE_TEST_IS_DIR)) {
        gtk_image_set_from_icon_name(GTK_IMAGE(icon), "folder-symbolic");
    } else {
        gtk_image_set_from_icon_name(GTK_IMAGE(icon), "text-x-generic-symbolic");
    }

    g_free(full_path);
}

static void on_create_file_accept(GtkWidget *button, gpointer user_data)
{
    GtkWindow *window = GTK_WINDOW(user_data);
    GtkWidget *entry = g_object_get_data(G_OBJECT(window), "entry");
    GtkStringList *files = g_object_get_data(G_OBJECT(window), "files");

    GtkEntryBuffer *buffer = gtk_entry_get_buffer(GTK_ENTRY(entry));
    const gchar *filename = gtk_entry_buffer_get_text(buffer);

    if (filename && strlen(filename) > 0)
    {
        if (!cur_filepath)
        {
            g_print("No active directory selected.\n");
            gtk_window_destroy(window);
            return;
        }

        gchar *full_path = g_build_filename(cur_filepath, filename, NULL);
        FILE *file = fopen(full_path, "w");
        if (file)
        {
            fclose(file);
            if (files)
            {
                GtkWidget *search_entry = g_object_get_data(G_OBJECT(window), "search_entry");
                if (search_entry)
                {
                    on_directory_search(GTK_ENTRY(search_entry), files);
                }
                else
                {
                    gtk_string_list_append(files, filename);
                }
            }
        }
        else
        {
            g_print("Failed to create file: %s\n", full_path);
        }

        g_free(full_path);
    }
    else
    {
        g_print("No file name provided.\n");
    }

    gtk_window_destroy(window);
}

void on_create_file(GtkWidget *button, gpointer user_data)
{
    GtkWidget *window;
    GtkWidget *entry = NULL;
    GtkWidget *create_btn = NULL;
    GtkStringList *files = GTK_STRING_LIST(user_data);
    GtkRoot *parent_root = NULL;

    if (GTK_IS_WIDGET(button))
    {
        parent_root = gtk_widget_get_root(button);
    }

    window = create_modal_input_dialog(
        "Create File",
        "Enter file name...",
        NULL,
        "Create",
        &entry,
        &create_btn
    );

    g_object_set_data(G_OBJECT(window), "entry", entry);
    g_object_set_data(G_OBJECT(window), "files", files);
    if (parent_root)
    {
        g_object_set_data(
            G_OBJECT(window),
            "search_entry",
            g_object_get_data(G_OBJECT(parent_root), "search_entry")
        );
    }

    g_signal_connect(create_btn, "clicked", G_CALLBACK(on_create_file_accept), window);

    gtk_widget_set_visible(window, true);
}

/* === FILE OPERATIONS === */


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

    delete_entry_by_name(file_name, GTK_STRING_LIST(gtk_single_selection_get_model(single_selection)));
}

static void open_file(char *filepath)
{
    if (!filepath)
    {
        return;
    }

    GError *error = NULL;
    gchar *uri = g_filename_to_uri(filepath, NULL, &error);
    if (!uri)
    {
        g_print("Failed to convert path to URI: %s\n", error ? error->message : "unknown error");
        g_clear_error(&error);
        return;
    }

    if (!g_app_info_launch_default_for_uri(uri, NULL, &error))
    {
        g_print("Failed to open file: %s\n", error ? error->message : "unknown error");
        g_clear_error(&error);
    }

    g_free(uri);
}

void on_maximize_clicked(GtkWindow *window, GtkButton *button)
{
    gtk_window_is_maximized(window) ? gtk_window_unmaximize(window) : gtk_window_maximize(window);
}


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

    if (!cur_filepath)
    {
        g_print("No active directory selected.\n");
        return;
    }

    open_entry_by_name(name, strings, search_entry);
}


/* === BUTTON CLICK ACTIONS === */

void on_home_button_clicked(GtkWidget *button, gpointer user_data)
{
    GtkWindow *window = GTK_WINDOW(user_data);
    navigate_window_to_path(window, g_get_home_dir());
}

void on_documents_button_clicked(GtkWidget *button, gpointer user_data)
{
    GtkWindow *window = GTK_WINDOW(user_data);
    navigate_window_to_special_dir(window, G_USER_DIRECTORY_DOCUMENTS);
}

void on_desktop_button_clicked(GtkWidget *button, gpointer user_data)
{
    GtkWindow *window = GTK_WINDOW(user_data);
    navigate_window_to_special_dir(window, G_USER_DIRECTORY_DESKTOP);
}

void on_music_button_clicked(GtkWidget *button, gpointer user_data)
{
    GtkWindow *window = GTK_WINDOW(user_data);
    navigate_window_to_special_dir(window, G_USER_DIRECTORY_MUSIC);
}
void on_pictures_button_clicked(GtkWidget *button, gpointer user_data)
{
    GtkWindow *window = GTK_WINDOW(user_data);
    navigate_window_to_special_dir(window, G_USER_DIRECTORY_PICTURES);
}
void on_videos_button_clicked(GtkWidget *button, gpointer user_data)
{
    GtkWindow *window = GTK_WINDOW(user_data);
    navigate_window_to_special_dir(window, G_USER_DIRECTORY_VIDEOS);
}
void on_trash_button_clicked(GtkWidget *button, gpointer user_data)
{
    GtkWindow *window = GTK_WINDOW(user_data);
    gchar *trash_path = g_build_filename(g_get_user_data_dir(), "Trash", "files", NULL);
    navigate_window_to_path(window, trash_path);
    g_free(trash_path);
}


