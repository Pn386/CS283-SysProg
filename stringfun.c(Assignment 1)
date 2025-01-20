#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SZ 50

// Function Prototypes
void usage(char *);
void print_buff(char *, int);
int setup_buff(char *, char *, int);
int count_words(char *, int, int);
void reverse_string(char *, int);
void print_words(char *, int);
int search_replace(char *, char *, char *, int);

int setup_buff(char *buff, char *user_str, int len) {
    char *src = user_str;
    char *dest = buff;
    int count = 0;
    int space_flag = 0;
    
    // Skip leading spaces
    while (*src == ' ' || *src == '\t') src++;
    
    while (*src) {
        if (count >= len) return -1;  // String too long
        
        if (*src == ' ' || *src == '\t') {
            if (!space_flag && count > 0) {
                *dest++ = ' ';
                count++;
                space_flag = 1;
            }
        } else {
            *dest++ = *src;
            count++;
            space_flag = 0;
        }
        src++;
    }
    
    // Remove trailing space if exists
    if (count > 0 && *(dest-1) == ' ') {
        dest--;
        count--;
    }
    
    // Fill remainder with dots
    while (count < len) {
        *dest++ = '.';
        count++;
    }
    
    return count;
}

int count_words(char *buff, int len, int str_len) {
    if (!buff || len <= 0 || str_len <= 0) return -1;
    
    int word_count = 0;
    int in_word = 0;
    char *ptr = buff;
    char *end = buff + str_len;
    
    while (ptr < end && *ptr != '.') {
        if (*ptr != ' ') {
            if (!in_word) {
                word_count++;
                in_word = 1;
            }
        } else {
            in_word = 0;
        }
        ptr++;
    }
    
    return word_count;
}

void reverse_string(char *buff, int str_len) {
    char *start = buff;
    char *end = buff;
    
    // Find end of actual string (before dots)
    while (*end != '.' && end < buff + str_len) end++;
    end--;
    
    // Reverse the string
    while (start < end) {
        char temp = *start;
        *start = *end;
        *end = temp;
        start++;
        end--;
    }
}

void print_words(char *buff, int str_len) {
    if (!buff || str_len <= 0) return;

    printf("Word Print\n");
    printf("----------\n");
    
    int word_index = 1;
    int char_count = 0;
    char *ptr = buff;
    char *word_start = ptr;
    int total_words = 0;
    
    while (ptr < buff + str_len && *ptr != '.') {
        if (*ptr != ' ') {
            if (char_count == 0) word_start = ptr;
            char_count++;
        } else if (char_count > 0) {
            printf("%d. ", word_index++);
            for (char *p = word_start; p < ptr; p++) putchar(*p);
            printf("(%d)\n", char_count);
            char_count = 0;
            total_words++;
        }
        ptr++;
    }
    
    if (char_count > 0) {
        printf("%d. ", word_index);
        for (char *p = word_start; p < ptr; p++) putchar(*p);
        printf("(%d)\n", char_count);
        total_words++;
    }
    
    printf("Number of words returned: %d\n", total_words);
}

int search_replace(char *buff, char *search, char *replace, int len) {
    if (!buff || !search || !replace) return -1;
    
    char *ptr = buff;
    int search_len = 0;
    int replace_len = 0;
    
    while (search[search_len]) search_len++;
    while (replace[replace_len]) replace_len++;
    
    while (*ptr != '.' && ptr < buff + len) {
        if (*ptr == *search) {
            char *s = search;
            char *b = ptr;
            int matched = 1;
            
            while (*s) {
                if (*b != *s) {
                    matched = 0;
                    break;
                }
                s++;
                b++;
            }
            
            if (matched) {
                // Calculate new string length
                char *end = ptr;
                while (*end != '.' && end < buff + len) end++;
                int curr_len = end - buff;
                int new_len = curr_len - search_len + replace_len;
                
                if (new_len >= len) {
                    // If replacement would exceed buffer, truncate
                    memmove(ptr + replace_len, ptr + search_len, 
                            len - (ptr - buff) - replace_len);
                    memcpy(ptr, replace, len - (ptr - buff));
                } else {
                    memmove(ptr + replace_len, ptr + search_len, 
                            curr_len - (ptr - buff) - search_len);
                    memcpy(ptr, replace, replace_len);
                    
                    // Fill remainder with dots
                    char *dot_start = buff + new_len;
                    while (dot_start < buff + len) *dot_start++ = '.';
                }
                return 0;
            }
        }
        ptr++;
    }
    
    return -1;  // Search string not found
}

void print_buff(char *buff, int len) {
    printf("Buffer:  [");
    for (int i = 0; i < len; i++) {
        putchar(buff[i]);
    }
    printf("]");
}

void usage(char *exename) {
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]", exename);
}

int main(int argc, char *argv[]) {
    char *buff;
    char *input_string;
    char opt;
    int rc;
    int user_str_len;

    if (argc < 2 || *argv[1] != '-') {
        usage(argv[0]);
        printf("\n");
        exit(1);
    }

    opt = *(argv[1] + 1);

    if (opt == 'h') {
        usage(argv[0]);
        printf("\n");
        exit(0);
    }

    if (argc < 3) {
        usage(argv[0]);
        printf("\n");
        exit(1);
    }

    input_string = argv[2];
    buff = (char *)malloc(BUFFER_SZ);
    
    if (!buff) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(2);
    }

    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);
    if (user_str_len < 0) {
        printf("error: Provided input string is too long\n");
        free(buff);
        exit(3);
    }

    switch (opt) {
        case 'c':
            rc = count_words(buff, BUFFER_SZ, user_str_len);
            if (rc < 0) {
                printf("Error counting words\n");
                free(buff);
                exit(3);
            }
            printf("Word Count: %d\n", rc);
            break;

        case 'r':
            reverse_string(buff, BUFFER_SZ);
            break;

        case 'w':
            print_words(buff, BUFFER_SZ);
            break;

        case 'x':
            if (argc != 5) {
                usage(argv[0]);
                printf("\n");
                free(buff);
                exit(1);
            }
            rc = search_replace(buff, argv[3], argv[4], BUFFER_SZ);
            if (rc < 0) {
                printf("Search string not found\n");
                free(buff);
                exit(3);
            }
            break;

        default:
            usage(argv[0]);
            printf("\n");
            free(buff);
            exit(1);
    }

    print_buff(buff, BUFFER_SZ);
    printf("\n");
    
    free(buff);
    return 0;
}
