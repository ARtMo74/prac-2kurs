#include <stdio.h>
#include "cli.c"

typedef unsigned char u_char;


void swap_bytes(u_char *bytes)
{
    char tmp = *bytes;
    *bytes = *(bytes + 1);
    *(bytes + 1) = tmp;
}


void convert_and_write(u_char *utf16bytes, FILE *output)
{
    u_char utf8bytes[3];
    if (utf16bytes[0] == 0 && (utf16bytes[1] & 0x80) == 0)
        fputc(utf16bytes[1], output);

    else if ((utf16bytes[0] & 0xF8) == 0)
    {
        utf8bytes[0] = 0xC0 | (utf16bytes[0] & 0x07) << 2 | utf16bytes[1] >> 6;
        utf8bytes[1] = 0x80 | (utf16bytes[1] & 0x3F);
        fwrite(utf8bytes, sizeof(u_char), 2, output);
    }

    else
    {
        utf8bytes[0] = 0xE0 | utf16bytes[0] >> 4;
        utf8bytes[1] = 0x80 | (utf16bytes[0] & 0x0F) << 2 | utf16bytes[1] >> 6;
        utf8bytes[2] = 0x80 | (utf16bytes[1] & 0x3f);
        fwrite(utf8bytes, sizeof(u_char), 3, output);
    }
}


int main(int argc, char *const *argv)
{
    FILE *input = NULL, *output = NULL;
    u_char utf16bytes[2];
    int le_order, count;

    process_args(argc, argv, &input, &output, &le_order);
    if (input == stdin)
    {
        int c;
        input = tmpfile();
        while ((c = getchar()) != EOF) fputc(c, input);
        rewind(input);
    }

    fread(utf16bytes, sizeof(u_char), 2, input);
    if (utf16bytes[0] == 0xFF && utf16bytes[1] == 0xFE) le_order = 1;
    else if (utf16bytes[0] == 0xFE && utf16bytes[1] == 0xFF) le_order = 0;
    else rewind(input);

    while ((count = fread(utf16bytes, sizeof(u_char), 2, input)) == 2)
    {
        if (le_order) swap_bytes(utf16bytes);
        convert_and_write(utf16bytes, output);
    }
    if (count == 1)
    {
        fprintf(stderr, "File must consist of even number of bytes.\n");
        return 1;
    }
    fclose(input);
    fclose(output);
}
