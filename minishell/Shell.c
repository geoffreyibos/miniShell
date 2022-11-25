#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <sys/stat.h>
#include <signal.h>


#include "Shell.h"
#include "StringVector.h"
/*
struct job{
    pid_t pid;
    struct StringVector sv;
};*/
struct Shell *globalShell;
static void 
liberer_espaceJobs(int sig){
    pid_t p = wait(NULL);   
    printf("le fils is dead %d\n ",p);
    for (int i=0;i<globalShell->compteur;i++){
        if(globalShell->jobs[i]==p){
            globalShell->jobs[i]=NULL;
        }
    }
}

void
shell_init( struct Shell *this )
{
    globalShell=this;

    this->running     = false;
    this->line        = NULL;
    this->line_number = 0;
    this->line_length = 0;
    this->jobs[255]   = "0\n";
    this->compteur    =0;

    struct sigaction sa;
    sa.sa_handler = liberer_espaceJobs;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    int retval = sigaction(SIGCHLD, &sa, NULL);
    
    //Initialiser le reste du shell
    
}

void
shell_free( struct Shell *this )
{
    if ( NULL != this->line ) {
        free( this->line );
        this->line = NULL;
    }
    this->line_length = 0;
}



void
shell_run( struct Shell *this )
{

    this->running = true;
    printf( "* Shell started\n" );
    while ( this->running ) {      
        shell_read_line( this );
        shell_execute_line( this );
    }
    printf( "* Shell stopped\n" );
}

void
shell_read_line( struct Shell *this )
{
    this->line_number++;
    char *buf = getcwd( NULL, 0 );
    printf( "%d: %s> ", this->line_number, buf );
    free( buf );
    getline( &this->line, &this->line_length, stdin );
}

static void
do_help( struct Shell *this, const struct StringVector *args )
{
    printf( "-> commands: exit, cd, help, ?, pwd, mkdir.\n" );
    //(void)this;
    (void)args;
}



static void
do_system( struct Shell *this, const struct StringVector *args )
{
    char *cmd=string_vector_get(args,1);
    
    
    char * options[string_vector_size(args)];
    char * asynchrome ="&";
    int estAsynchrome=0;
    // buffer pour concatener
    char buffer[100];

    //boucle pour avoir toutes les options
    for (int i=1;i<string_vector_size(args);i++){
        options[i-1]=string_vector_get(args,i);
    }

    if(strcmp(asynchrome, options[string_vector_size(args)-2])==0){
        options[string_vector_size(args)-2]=NULL;
        estAsynchrome=1;
    }else{
        options[string_vector_size(args)-1]=NULL;
    }

    
    pid_t pid_fils =fork();
    if(pid_fils==0){
        
        printf(" Vous êtes sur un shell fils\n");
        strcat(strcpy(buffer, "/bin/"), cmd);
        execv(buffer,options);

    }
    if(estAsynchrome==1){
        //char * job = pid_fils;
        
        this->jobs[this->compteur]=pid_fils;
        this->compteur+=1;
        printf("test3\n");
    }else{
        wait();
    }


    //quand il y a un signal child il faut supprimer dans le tableau le fils(jobs) mettre à null
    //dcp il faut eviter les NULL
    //README avec ce qu'on a réussi à faire et pas réussi+
    //
    (void)this;
    (void)args;
}
static void
do_jobs( struct Shell *this, const struct StringVector *args )
{
    printf("Affichage des processus en cours\n");
    for(int i=0;i<this->compteur;i++){
        if(this->jobs[i]!=0){
            printf(" : %d \n",this->jobs[i]);
        }
    }
    
    (void)this;
}


static void
do_cd( struct Shell *this, const struct StringVector *args )
{
    int   nb_tokens = string_vector_size( args );
    char *tmp;
    if ( 1 == nb_tokens ) {
        tmp = getenv( "HOME" );
    }
    else {
        tmp = string_vector_get( args, 1 );
    }
    int rc = chdir( tmp );
    if ( 0 != rc )
        printf( "directory '%s' not valid\n", tmp );
    (void)this;
}

static void
do_pwd( struct Shell *this, const struct StringVector *args )
{
    int   nb_tokens = string_vector_size( args );
    char *tmp;
    if ( 1 == nb_tokens ) {
        printf("\"%s\"\n", getenv("PWD") );
    }else{
        printf("Veuillez rentrer le bon nombre d'argument");
    }
    (void)this;
    (void)args;
}

char *dernieresCmd[256];
int cpt = 1;

static void
do_rappel( struct Shell *this, const struct StringVector *args )
{
    if (dernieresCmd[cpt-2] == NULL) {
        printf("Vous n'avez écrit aucune commande\n");
    }
    else {
        printf("Dernière commande : %s\n", dernieresCmd[cpt-2]);
    }
    (void)this;
    (void)args;
}

static void
do_execute( struct Shell *this, const struct StringVector *args )
{
    (void)this;
    (void)args;
}

static void
do_exit( struct Shell *this, const struct StringVector *args )
{
    this->running = false;
    (void)this;
    (void)args;
}


static void
do_mkdir( struct Shell *this, const struct StringVector *args )
{
    int   nb_tokens = string_vector_size( args );
    char *tmp;
    char *name;
    if ( 2 == nb_tokens ) {
        name=string_vector_get(args,1);
        if ( mkdir( name, 0755 ) != 0 ) {
            printf("Impossible de créer ce fichier\n");
        }
    }else{
        printf("veuillez rentrer le bon nombre d'argument\n");
    }
    (void)this;
    (void)args;
}

static void
do_rmdir( struct Shell *this, const struct StringVector *args )
{
    int   nb_tokens = string_vector_size( args );
    char *tmp;
    char *name;
    if ( 2 == nb_tokens ) {
        name=string_vector_get(args,1);
        if ( rmdir( name) != 0 ) {
            printf("Impossible de supprimer ce dossier\n");
        } else{
            printf("Dossier supprimé\n");
        }
    }
    else{
        printf("veuillez rentrer le bon nombre d'argument\n");
    }
    (void)this;
    (void)args;
}

typedef void ( *Action )( struct Shell *, const struct StringVector * );

static struct {
    const char *name;
    Action      action;
} actions[] = { { .name = "exit", .action = do_exit },  { .name = "mkdir", .action = do_mkdir }, 
                { .name = "rmdir", .action = do_rmdir },    { .name = "rappel", .action = do_rappel }, 
                { .name = "help", .action = do_help },  { .name = "?", .action = do_help },
                { .name = "!", .action = do_system },   { .name = "cd", .action = do_cd },
                { .name = "pwd", .action = do_pwd }, 
                  { .name = NULL, .action = do_execute }};


Action
get_action( char *name )
{
    int i = 0;
    while ( actions[i].name != NULL && strcmp( actions[i].name, name ) != 0 ) {
        i++;
    }
    dernieresCmd[cpt] = actions[i].name;
    cpt++;
    return actions[i].action;
}

void
shell_execute_line( struct Shell *this )
{
    struct StringVector tokens    = split_line( this->line );
    int nb_tokens = string_vector_size( &tokens );
    if ( nb_tokens == 0 ) {
        printf( "-> Nothing to do !\n" );
    }
    else {
        char  *name   = string_vector_get( &tokens, 0 );
        Action action = get_action( name );
        action( this, &tokens );
    }

    string_vector_free( &tokens );
}
