#include "util.h"

void read_file(char **buffer, const char *filename)
{
    char *contents;
    size_t filesize;
    FILE *file;

    // open the file
    file = fopen(filename, "rb");
    
    // search the file size
    fseek(file, 0, SEEK_END);
    filesize = ftell(file);

    // rewind
    rewind(file);

    // allocate memory block and copy bits into memory
    contents = (char *)malloc(filesize * sizeof(char));
    fread(contents, sizeof(char), filesize, file);

    // don't forget the 0x0 at the end of the string
    contents[filesize-1] = 0;

    // finish
    fclose(file);
    (*buffer) = contents;
}

void NOT_IMPLEMENTED(const char *filename, int line, const char *function)
{
    printf("%s:%d:%s(): NOT IMPLEMENTED\n", filename, line, function);
    abort();
}

int __indent_size = 0;

struct node_s {
    char *string;
    struct node_s *next;
};

void INFUNC(const char *function)
{
//    if(__indent_size <= 0) {
//        printf("<%s>\n", function);
//        return;
//    }
//
//    char indent[__indent_size+1];
//    for(int i = 0; i < __indent_size; i++) {
//        indent[i] = ' ';
//    }
//    indent[__indent_size] = 0;
//
    char *indent = "    ";
    fprintf(stderr, "%*s" "+ %s()\n", __indent_size, " ", function);
    __indent_size += 4;
}

void SUCCESSED(const char *function)
{
    __indent_size -= 4;
    fprintf(stderr, "%*s" "- %s()\n", __indent_size, " ", function);
}
