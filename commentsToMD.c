#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *fpath;


// Function to extract comments from a file
void extract_comments(GtkButton *execute, gpointer user_data) {
    char *input_file = g_strdup(fpath);
    FILE *input = fopen(input_file, "r");
    size_t len = strlen(input_file);
    char *output_file = g_malloc(len + 3);
    strcpy(output_file, input_file);
    output_file[len - 2] = '\0';
    strcat(output_file, ".md");
    g_print("%s\n", output_file);
    FILE *output = fopen(output_file, "w");

    if (input == NULL) {
        perror("Error opening input file");
        g_free(input_file);
        g_free(output_file);

        return;
    }

    if (output == NULL) {
        perror("Error opening output file");
        g_free(input_file);
        g_free(output_file);
        fclose(input);
        return;
    }

    char line[1024];
    int in_multiline_comment = 0;

    while (fgets(line, sizeof(line), input)) {
        char *single_line_comment = strstr(line, "//");
        char *multi_line_start = strstr(line, "/*");
        char *multi_line_end = strstr(line, "*/");

        if (in_multiline_comment) {
            if (multi_line_end) {
                *multi_line_end = '\0';
                fprintf(output, "%s\n", line);
                in_multiline_comment = 0;
            } else {
                fprintf(output, "%s", line);
            }
        } else if (single_line_comment && (!multi_line_start || single_line_comment < multi_line_start)) {
            fprintf(output, "%s\n", single_line_comment + 2);
        } else if (multi_line_start) {
            if (multi_line_end && multi_line_end > multi_line_start) {
                *multi_line_end = '\0';
                fprintf(output, "%s\n", multi_line_start + 2);
            } else {
                in_multiline_comment = 1;
                fprintf(output, "%s", multi_line_start + 2);
            }
        }
    }

    fclose(input);
    fclose(output);
    g_free(input_file);
    g_free(output_file);


}


// Callback to handle the file dialog response
static void on_file_dialog_response(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GFile *file = gtk_file_dialog_open_finish(GTK_FILE_DIALOG(source_object), res, NULL);
    if (file != NULL) {
        // A file was selected
        char *filename = g_file_get_path(file);
        g_print("Selected file: %s\n", filename);
        fpath = g_strdup(filename);  // Make a copy of the string for fpath
        GtkWindow *parent = GTK_WINDOW(user_data);
        GtkWidget *label = g_object_get_data(G_OBJECT(parent), "filepath_label");
        if (label != NULL){
            gtk_label_set_text(GTK_LABEL(label), filename);
        }
        // Free resources
        g_free(filename);
        g_object_unref(file);
    } else {
        // No file was selected (dialog was canceled)
        g_print("No file selected. The dialog was canceled.\n");
    }
}


static void ShowDialog(GtkButton *file_choose, gpointer user_data) {
    GtkWindow *parent = GTK_WINDOW(user_data); // Parent window


    

    // Create a new GtkFileDialog
    GtkFileDialog *file_dialog = gtk_file_dialog_new();

    // Show the file dialog asynchronously
    gtk_file_dialog_open(file_dialog, parent, NULL, 
        (GAsyncReadyCallback)on_file_dialog_response, parent);
}



static void activate(GtkApplication *app, gpointer user_data) {
    // Load the UI from the .ui file
    GtkBuilder *builder = gtk_builder_new_from_file("GTK4GUI.ui");

    // Get the window object from the UI file
    GtkWidget *window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));

    // Get the button object from the UI file
    GtkWidget *file_choose = GTK_WIDGET(gtk_builder_get_object(builder, "file_choose"));

    GtkWidget *execute = GTK_WIDGET(gtk_builder_get_object(builder, "execute"));

    // Get the label object from the UI file
    GtkLabel *filepath_label = GTK_LABEL(gtk_builder_get_object(builder, "filepath"));

    g_object_set_data(G_OBJECT(window), "filepath_label", filepath_label);

    // Connect the signal for the button click
    g_signal_connect(file_choose, "clicked", G_CALLBACK(ShowDialog), window);
    g_signal_connect(execute, "clicked", G_CALLBACK(extract_comments), NULL);

    // Set the window as the main window
    gtk_window_set_application(GTK_WINDOW(window), app);

    // Show the window and its contents
    gtk_window_present(GTK_WINDOW(window));

    // Clean up
    g_object_unref(builder);  // Don't forget to free the builder
}

int main(int argc, char *argv[]) {
    // Create the GtkApplication
    GtkApplication *app = gtk_application_new("com.example.gtk4", G_APPLICATION_DEFAULT_FLAGS);

    // Connect the activate signal
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    // Run the application
    int status = g_application_run(G_APPLICATION(app), argc, argv);

    // Free the GtkApplication object
    g_object_unref(app);

    return status;
}
