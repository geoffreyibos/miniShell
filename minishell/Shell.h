#include <stdio.h>
#include <stdbool.h>
#include "StringVector.h"
#ifndef SHELL_H
#define SHELL_H

/*
struct job{
    pid_t pid;
    struct StringVector sv;
};*/

struct Shell {
    bool   running;
    int    line_number;
    char * line;
    size_t line_length;
    //struct job jobs[256];
    pid_t * jobs[256];
    int    compteur;
};

void shell_init( struct Shell *s );
void shell_run( struct Shell *s );
void shell_free( struct Shell *s );

void shell_read_line( struct Shell *s );
void shell_execute_line( struct Shell *s );

#endif /* SHELL_H */
