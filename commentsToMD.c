#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>




// Function to extract comments from a file
void extract_comments(const char *input_file, const char *output_file) {
    FILE *input = fopen(input_file, "r");
    FILE *output = fopen(output_file, "w");

    if (input == NULL) {
        perror("Error opening input file");
        return;
    }

    if (output == NULL) {
        perror("Error opening output file");
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
}







// Main function
int main(int argc, char *argv[]) {


}
