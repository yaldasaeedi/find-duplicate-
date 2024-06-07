#include <gtk/gtk.h>

void button_clicked(GtkWidget *widget, gpointer data) {
    g_print("Button clicked!\n");
}

int main(int argc, char *argv[]) {
    // Initialize GTK+
    gtk_init(&argc, &argv);

    // Create the main window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Table Example");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    gtk_widget_set_size_request(window, 400, 300);
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Create a grid
    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    // Create a button
    GtkWidget *button = gtk_button_new_with_label("Click Me");
    g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(button_clicked), NULL);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 0, 1, 1);

    // Create a text field
    GtkWidget *entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), entry, 0, 1, 1, 1);

    // Show all widgets
    gtk_widget_show_all(window);

    // Run the GTK+ main loop
    gtk_main();

    return 0;
}
