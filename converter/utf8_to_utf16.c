#include <stdio.h>
#include "cli.c"

typedef unsigned char u_char;


void convert_and_write(u_char *utf8ch, int mode, FILE *output, int le_order)
{
    u_char c1, c2;
    if (mode == 1)
    {
        c1 = 0x00;
        c2 = utf8ch[0];
    }
    else if (mode == 2)
    {
        c1 = (utf8ch[0] & 0x1F) >> 2;
        c2 = (utf8ch[0] & 0x03) << 6 | (utf8ch[1] & 0x3F);
    }
    else
    {
        c1 = (utf8ch[0] & 0x0F) << 4 | (utf8ch[1] >> 2 & 0x0F);
        c2 = (utf8ch[1] & 0x03) << 6 | (utf8ch[2] & 0x3F);
    }

    if (le_order)
    {
        fputc(c2, output);
        fputc(c1, output);
    }
    else
    {
        fputc(c1, output);
        fputc(c2, output);
    }
}


void shutdown(FILE *input, FILE *output)
{
    fprintf(stderr, "Not enough bytes to finish a sequence.\n");
    fclose(input);
    fclose(output);
    exit(4);
}


int main(int argc, char *const *argv)
{
    FILE *input = NULL, *output = NULL;
    u_char utf8ch[3];
    int le_order, is_bom, is_bom_rev; /* flags */
    char *bad_seq_msg = "corrupted byte '0x%02X' at position %ld, "
                       "bad sequence ignored\n";

    process_args(argc, argv, &input, &output, &le_order);
    if (input == stdin)
    {
        int c;
        input = tmpfile();
        while ((c = getchar()) != EOF) fputc(c, input);
        rewind(input);
    }

    fread(utf8ch, sizeof(u_char), 3, input);
    is_bom = utf8ch[0] == 0xEF && utf8ch[0] == 0xBB && utf8ch[0] == 0xBF;
    is_bom_rev = utf8ch[0] == 0xEF && utf8ch[0] == 0xBF && utf8ch[0] == 0xBE;
    if (!is_bom && !is_bom_rev)
        rewind(input);

    if (le_order)
    {
        fputc(0xFF, output);
        fputc(0xFE, output);
    }
    else
    {
        fputc(0xFE, output);
        fputc(0xFF, output);
    }

    while (fread(utf8ch, sizeof(u_char), 1, input))
    {
        if (utf8ch[0] >> 7 == 0)
            convert_and_write(utf8ch, 1, output, le_order);

        else if (utf8ch[0] >> 5 == 0x06)
        {
            int count = fread(utf8ch + 1, sizeof(u_char), 1, input);
            if (count != 1)
                shutdown(input, output);
            else if (utf8ch[1] >> 6 != 0x02)
                fprintf(stderr, bad_seq_msg, utf8ch[1], ftell(input));
            else
                convert_and_write(utf8ch, 2, output, le_order);
        }

        else if (utf8ch[0] >> 4 == 0x0E)
        {
            int count = fread(utf8ch + 1, sizeof(u_char), 2, input);
            if (count != 2)
                shutdown(input, output);
            else if (utf8ch[1] >> 6 != 0x02)
                fprintf(stderr, bad_seq_msg, utf8ch[1], ftell(input));
            else if (utf8ch[2] >> 6 != 0x02)
                fprintf(stderr, bad_seq_msg, utf8ch[2], ftell(input));
            else
                convert_and_write(utf8ch, 3, output, le_order);
        }

        else
            fprintf(stderr, bad_seq_msg, utf8ch[0], ftell(input));
    }
    fclose(input);
    fclose(output);
}
