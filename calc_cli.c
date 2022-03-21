#include <calc.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input> <output>\n", argv[0]);
        return 1;
    }

    int exit_code = 1;

    FILE *input;
    if ((input = fopen(argv[1], "r")) == NULL) {
        fprintf(stderr, "Unable to open '%s' for reading\n", argv[1]);
        goto input_file_closed;
    }

    FILE *output;
    if ((output = fopen(argv[2], "w")) == NULL) {
        fprintf(stderr, "Unable to open '%s' for writing\n", argv[2]);
        goto output_file_closed;
    }

    while (!feof(input)) {
        char *buf = malloc(1024 * sizeof(char));
        if (!buf) {
            fprintf(stderr, "Unable to allocate buffer\n");
            goto buf_cleared;
        }

        int capacity = 1024;
        int buf_size = 0;
        char cur_char = 1;

        while (!feof(input) && cur_char != 0) {
            cur_char = (char)fgetc(input);
            if (cur_char == EOF && !feof(input)) {
                fprintf(stderr, "Error while reading from the input\n");
                free(buf);
                goto buf_cleared;
            }
            if (cur_char == '\n' || cur_char == EOF) {
                cur_char = 0;
            }
            buf[buf_size++] = cur_char;
            if (buf_size == capacity) {
                capacity *= 2;
                char *buf_ = realloc(buf, capacity * sizeof(char));
                if (!buf_) {
                    fprintf(stderr, "Unable to allocate buffer\n");
                    free(buf);
                    goto buf_cleared;
                }
                buf = buf_;
            }
        }

        if (buf[0] != 0) {
            calc_result res;
            CALC_ERROR_TYPE error_type = calc_evaluate(buf, &res, NULL);
            if (error_type == CALC_ERROR_OK) {
                fprintf(output, "%.3f\n", res.value);
            } else {
                fprintf(output, "Error %d:\n  %s\n  ", error_type, buf);
                for (int i = 0; i < res.error_position; i++) {
                    fprintf(output, ".");
                }
                fprintf(output, "^\n");
            }
        }
        free(buf);
    }
    exit_code = 0;

buf_cleared:
    fclose(output);
output_file_closed:
    fclose(input);
input_file_closed:
    return exit_code;
}