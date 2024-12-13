#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *fpath;
bool multineCommentsOnly = FALSE;

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
    int in_code_block = 0;
    char *start_marker = "/*CODE*/";
    char *end_marker = "/*/CODE*/";

    while (fgets(line, sizeof(line), input)) {
        // Check if we are entering a CODE block
        if (!in_code_block && strstr(line, start_marker)) {
            in_code_block = 1;
            // Print the starting backticks before the code block
            fprintf(output, "```\n");
            // Skip everything before "CODE" starts, don't print start_marker
            char *code_start = strstr(line, start_marker) + strlen(start_marker);
            fprintf(output, "%s", code_start);  // Print everything after "CODE" start
            continue;
        }

        // If we are inside a CODE block, check if we find the end marker
        if (in_code_block) {
            if (strstr(line, end_marker)) {
                // Print everything before the end_marker and exit the block
                char *code_end = strstr(line, end_marker);
                *code_end = '\0'; // Terminate the line before the end_marker
                fprintf(output, "%s\n", line);  // Print the code content
                // Print the ending backticks after the code block
                fprintf(output, "```\n");
                in_code_block = 0;  // End the CODE block
            } else {
                // Print the content of the CODE block as is
                fprintf(output, "%s", line);
            }
        } else {
            // Handle normal comments
            char *single_line_comment = strstr(line, "//");
            char *multi_line_start = strstr(line, "/*");
            char *multi_line_end = strstr(line, "*/");

            if (in_multiline_comment) {
                // If inside a multi-line comment, look for the end
                if (multi_line_end) {
                    *multi_line_end = '\0'; // End the comment
                    // Skip leading space if the first character is a space
                    if (line[0] == ' ') {
                        fprintf(output, "%s\n", line + 1); // Skip the leading space
                    } else {
                        fprintf(output, "%s\n", line); // Output comment content
                    }
                    in_multiline_comment = 0;
                } else {
                    // Skip leading space if the first character is a space
                    if (line[0] == ' ') {
                        fprintf(output, "%s", line + 1); // Skip the leading space
                    } else {
                        fprintf(output, "%s", line); // Output content of the multi-line comment
                    }
                }
            } else if (single_line_comment && (!multi_line_start || single_line_comment < multi_line_start) && !multineCommentsOnly) {
                // Handle single-line comments
                // Skip leading space if the first character is a space
                if (single_line_comment[2] == ' ') {
                    fprintf(output, "%s\n", single_line_comment + 3);  // Skip the space after "//"
                } else {
                    fprintf(output, "%s\n", single_line_comment + 2);  // Print after "//"
                }
            } else if (multi_line_start) {
                // If we find the start of a multi-line comment
                if (multi_line_end && multi_line_end > multi_line_start) {
                    *multi_line_end = '\0'; // End the comment
                    // Skip leading space if the first character is a space
                    if (multi_line_start[2] == ' ') {
                        fprintf(output, "%s\n", multi_line_start + 3); // Skip the space after "/*"
                    } else {
                        fprintf(output, "%s\n", multi_line_start + 2); // Print content between /* and */
                    }
                } else {
                    in_multiline_comment = 1;
                    // Skip leading space if the first character is a space
                    if (multi_line_start[2] == ' ') {
                        fprintf(output, "%s", multi_line_start + 3); // Skip the space after "/*"
                    } else {
                        fprintf(output, "%s", multi_line_start + 2); // Start multi-line comment block
                    }
                }
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

static void multilineOnly(GObject *source_object, gpointer user_data){
    multineCommentsOnly = TRUE;
}

static void activate(GtkApplication *app, gpointer user_data) {
    // Load the UI from the .ui file
    GtkBuilder *builder = gtk_builder_new_from_file("GTK4GUI.ui");

    // Get the window object from the UI file
    GtkWidget *window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));

    // Get the button object from the UI file
    GtkWidget *file_choose = GTK_WIDGET(gtk_builder_get_object(builder, "file_choose"));

    GtkWidget *execute = GTK_WIDGET(gtk_builder_get_object(builder, "execute"));

    // Get the checkbox object from the UI File

    GtkWidget *multilineCheck = GTK_WIDGET(gtk_builder_get_object(builder, "multilineCheck"));

    // Get the label object from the UI file
    GtkLabel *filepath_label = GTK_LABEL(gtk_builder_get_object(builder, "filepath"));

    g_object_set_data(G_OBJECT(window), "filepath_label", filepath_label);

    // Connect the signal for the button click
    g_signal_connect(file_choose, "clicked", G_CALLBACK(ShowDialog), window);
    g_signal_connect(execute, "clicked", G_CALLBACK(extract_comments), NULL);

    // Connect the signal for the Check Button
    g_signal_connect(multilineCheck, "activated", G_CALLBACK(multilineOnly), NULL);

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
