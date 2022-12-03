#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "parser.h"


enum sep_type{
    not_separator = 0,
    ordinary_separator = 1,
    special_separator = 2,
    space = 3,
    end_of_line = 4
};


enum sep_type sep_check(int c)
{
    if (c == '(' || c == ')' || c == ';' || c == '<') return ordinary_separator;

    else if (c == '&' || c == '>' || c == '|') return special_separator;

    else if (c == ' ') return space;

    else if (c == '\0') return end_of_line;

    else return not_separator;
}


void add_letter(int c, str *word_ptr, int *letter_count)
{
    (*letter_count)++;
    *word_ptr = (str)realloc(*word_ptr, (*letter_count) + 1);
    (*word_ptr)[(*letter_count) - 1] = (char)c;
}


void add_word(str word, words_array *ptr, int *word_count)
{
    (*word_count)++;
    *ptr = (words_array)realloc(*ptr, sizeof(str) * ((*word_count) + 1));
    (*ptr)[(*word_count) - 1] = word;
}


void add_char_as_word(int c, words_array *ptr, int *word_count)
{
    str word = (str)malloc(2);
    word[0] = (char)c; word[1] = '\0';
    add_word(word, ptr, word_count);
}


str get_line(FILE *stream)
{
    int c, n = 0, quot_marks_flag = 0, eof_warning_flag = 0;
    str line = (str)malloc(n + 1);

    char *username = getenv("USER");
    char *current_working_dir = getcwd(NULL, 0);
    char hostname[65];
    gethostname(hostname, sizeof(hostname));
    printf("%s@%s:%s$ ", username, hostname, current_working_dir);
    free(current_working_dir);

    while ((c = fgetc(stream)) != '\n' && c != EOF)
    {
        n++;
        line = realloc(line, n + 1);
        line[n - 1] = (char)c;
        if (c == '"') quot_marks_flag ^= 1;
    }
    line[n] = '\0';
    if (c == EOF)
    {
        if (n == 0)
        {
            free(line);
            return NULL;
        }
        else eof_warning_flag = 1;
    }


    if (stream != stdin) puts(line);
    if (quot_marks_flag)
    {
        fputs("ERROR: no closing quotation marks found.\n", stderr);
        return get_line(stream);
    }
    if (eof_warning_flag == 1)
    {
        fputs("WARNING: no newline at the end of the file.\n", stderr);
        ungetc(EOF, stream);
    }
    return line;
}


words_array parse_line(str line)
{
    int i = 0, word_count = 0;
    words_array words = (words_array)malloc(sizeof(str) * (word_count + 1));
    while (line[i] != '\0')
    {
        int letter_count = 0;
        str word = (str)malloc(letter_count + 1);
        enum sep_type sep;

        while (line[i] == ' ') i++;
        while ((sep = sep_check(line[i])) == not_separator)
        {
            if (line[i] == '"')
            {
                i++;
                while (line[i] != '"')
                {
                    add_letter(line[i], &word, &letter_count);
                    i++;
                }
                i++;
            }
            else
            {
                add_letter(line[i], &word, &letter_count);
                i++;
            }
        }

        if (letter_count != 0)
        {
            word[letter_count] = '\0';
            add_word(word, &words, &word_count);
        }
        else free(word);

        if (sep == special_separator && line[i] == line[i + 1])
        {
            word = (str)malloc(3);
            word[0] = word[1] = line[i];
            word[2] = '\0';
            add_word(word, &words, &word_count);
            i += 2;
        }
        else if (sep == ordinary_separator || sep == special_separator)
        {
            add_char_as_word(line[i], &words, &word_count);
            i++;
        }
    }
    words[word_count] = NULL;
    return words;
}
