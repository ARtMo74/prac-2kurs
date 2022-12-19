#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include "parser.h"


static int is_metachar(int c)
{
    switch(c)
    {
        case ' ':
        case '\t':
        case '\n':
        case '&':
        case '|':
        case '(':
        case ')':
        case ';':
        case '<':
        case '>':
            return 1;
        default:
            return 0;
    }
}


static int is_2symbol(int c)
{
    switch(c)
    {
        case '>':
        case '|':
        case '&':
            return 1;
        default:
            return 0;
    }
}


static int is_term(str opr)
{
    return strcmp(opr, "&") == 0 || strcmp(opr, ";") == 0;
}


static int is_logic(str opr)
{
    return strcmp(opr, "&&") == 0 || strcmp(opr, "||") == 0;
}


static void add_letter(int c, str *word_ptr, int *letter_count)
{
    (*letter_count)++;
    *word_ptr = (str)realloc(*word_ptr, (*letter_count) + 1);
    (*word_ptr)[(*letter_count) - 1] = (char)c;
}


static void add_word(str word, words_array *ptr, int *word_count)
{
    (*word_count)++;
    *ptr = (words_array)realloc(*ptr, sizeof(str) * ((*word_count) + 1));
    (*ptr)[(*word_count) - 1] = word;
}


static void add_char_as_word(int c, words_array *ptr, int *word_count)
{
    str word = (str)malloc(2);
    word[0] = (char)c; word[1] = '\0';
    add_word(word, ptr, word_count);
}


static str get_line()
{
    int c, n = 0, quot_marks_flag = 0, eof_warning_flag = 0;
    str line = (str)malloc(n + 1);

    while ((c = getchar()) != '\n' && c != EOF)
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

    if (quot_marks_flag)
    {
        fputs("ERROR: unexpected EOF while "
              "looking for matching `\"'\n", stderr);
        return get_line();
    }
    if (eof_warning_flag == 1)
    {
        fputs("WARNING: no newline at the end of the file.\n", stderr);
        ungetc(EOF, stdin);
    }
    return line;
}


static words_array parse_line()
{
    str line = get_line();
    if (line == NULL) return NULL;
    int i = 0, word_count = 0;
    words_array words = (words_array)malloc(sizeof(str) * (word_count + 1));
    while (line[i] != '\0')
    {
        int letter_count = 0;
        str word = (str)malloc(letter_count + 1);

        while (line[i] == ' ' || line[i] == '\t') i++;
        while (line[i] != '\0' && !is_metachar(line[i]))
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

        if (is_2symbol(line[i]) && line[i] == line[i + 1])
        {
            word = (str)malloc(3);
            word[0] = word[1] = line[i];
            word[2] = '\0';
            add_word(word, &words, &word_count);
            i += 2;
        }
        else if (!isspace(line[i]) && line[i] != '\0')
        {
            add_char_as_word(line[i], &words, &word_count);
            i++;
        }
    }
    words[word_count] = NULL;
    free(line);
    return words;
}


static Simple_command make_simple_command(words_array words)
{
    int i = 0, n = 0;
    Simple_command cmd;
    cmd.argv = (words_array)malloc(sizeof(str));
    cmd.fd[0] = cmd.fd[1] = -1;
    while (words[i] != NULL)
    {
        if (strcmp(words[i], ">") == 0)
        {
            cmd.fd[1] = open(words[i + 1], O_CREAT|O_WRONLY|O_TRUNC, 0666);
            i += 2;
        }
        else if (strcmp(words[i], "<") == 0)
        {
            cmd.fd[0] = open(words[i + 1], O_RDONLY, 0666);
            i += 2;
        }
        else
        {
            n++;
            cmd.argv = (words_array)realloc(cmd.argv, (n + 1) * sizeof(str));
            cmd.argv[n - 1] = words[i];
            i++;
        }
    }
    cmd.argv[n] = NULL;
    return cmd;
}


static Pipeline make_pipeline(words_array words)
{
    int i = 0, offset;
    Pipeline pipeline = NULL, *tmp = &pipeline;
    for (;;)
    {
        *tmp = (Pipeline)malloc(sizeof(Pipeline_node));
        str tmp_word;
        (*tmp)->next = NULL;
        offset = i;

        while (words[i] != NULL && strcmp(words[i], "|") != 0) i++;
        tmp_word = words[i];
        words[i] = NULL;
        (*tmp)->cmd = make_simple_command(&words[offset]);
        words[i] = tmp_word;

        if (words[i] == NULL) break;
        i++;
        tmp = &(*tmp)->next;
    }
    return pipeline;
}


static And_or_list make_ao_list(words_array words)
{
    int i = 0, offset;
    And_or_list ao_list = NULL, *tmp = &ao_list;
    for (;;)
    {
        *tmp = (And_or_list)malloc(sizeof(And_or_list_node));
        str tmp_word;
        (*tmp)->next = NULL;
        offset = i;

        while (words[i] != NULL && !is_logic(words[i])) i++;
        if (words[i] == NULL) (*tmp)->opr = null_opr;
        else if (strcmp(words[i], "&&") == 0) (*tmp)->opr = and_opr;
        else (*tmp)->opr = or_opr;
        tmp_word = words[i];
        words[i] = NULL;
        (*tmp)->pl = make_pipeline(&words[offset]);
        words[i] = tmp_word;

        if (words[i] == NULL) break;
        i++;
        tmp = &(*tmp)->next;
    }
    return ao_list;
}


List make_command()
{
    words_array words = parse_line();
    if (words == NULL) return NULL;
    int i = 0, offset;
    List command = NULL, *tmp = &command;
    for (;;)
    {
        *tmp = (List)malloc(sizeof(List_node));
        str tmp_word;
        (*tmp)->next = NULL;
        (*tmp)->is_in_bckg = 0;
        offset = i;

        while (words[i] != NULL && !is_term(words[i])) i++;
        if (words[i] != NULL && strcmp(words[i], "&") == 0)
            (*tmp)->is_in_bckg = 1;
        tmp_word = words[i];
        words[i] = NULL;
        (*tmp)->ao_list = make_ao_list(&words[offset]);
        words[i] = tmp_word;

        if (words[i] == NULL) break;
        i++;
        tmp = &(*tmp)->next;
    }
    return command;
}
