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
    //char *indent = "    ";
    fprintf(stderr, "%*s" "+ %s()\n", __indent_size, " ", function);
    __indent_size += 4;
}

void SUCCESSED(const char *function)
{
    __indent_size -= 4;
    fprintf(stderr, "%*s" "- %s()\n", __indent_size, " ", function);
}

int time_subtract(struct timeval *result, struct timeval *t2, struct timeval *t1)
{
    long int diff = (t2->tv_usec + 1000000 * t2->tv_sec) - (t1->tv_usec + 1000000 * t1->tv_sec);
    result->tv_sec = diff / 1000000;
    result->tv_usec = diff % 1000000;

    return (diff<0);
}

void time_print(struct timeval *tv)
{
    char buffer[30];
    time_t curtime;

    printf("%d.%06d", (int)tv->tv_sec, (int)tv->tv_usec);
    curtime = tv->tv_sec;
    strftime(buffer, 30, "%m-%d-%Y  %T", localtime(&curtime));
    printf(" = %s.%06d\n", buffer, (int)tv->tv_usec);
}
