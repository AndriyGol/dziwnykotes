/******************************************************************************
  Title          : main.c
  Author         : Andriy Goltsev
  Created on     : April 26, 2011
  Description    : a two-dimensional simulation of a rather unusual colony of single-celled
                   organisms called dziwnykotes. These organisms reproduce in an unusual way, and
                   they are also very sensitive to their surroundings. If they are too crowded, they die or, and if they are not
                   surrounded by enough of their kind, they also die. They reproduce only under very specifc constraints: a
                   new organism is born as a result of a three-way interaction of the organisms, a menage-a-trois, so to speak.

  Usage          : Begin the simulation with 'b' (only in the initial state.)
                   Control the speed of the simulation by pressing '+' to increase speed and '-' to decrease speed (only if the simulation is not paused.)
                   Terminate the simulation by pressing 'q' (at any time.)
                   Pause the simulation with 'p' (only if the simulation is not already paused.)
                   Resume the simulation with 'r' (only if it is paused.)
                   Jump ahead to future state with 'j' (only if it is paused or in the initial state.) When the user presses
                            'j', the program will then wait for the user to enter a positive number. If the user types anything else
                            it should be ignored. If the user types a positive number n, the simulation will advance n states, or
                            to the terminal state if all organisms die before n steps. The number should be delimited by a visible
                            keyboard character other than a digit.
                   To save type 's'. this operation disables all other function that can change the surface array, so it should be safe.
  Build with:  gcc -o dziwnykotes -lncurses timers.c timers.h bacteria.h main.c

  Notes:       Based on bouncestr_async3.c written by Stewart Weiss.
               The assignment was done on the rush in the last minute. I am not very happy about the implemenatation.


******************************************************************************/

#include    <stdio.h>
#include    <string.h>
#include    <stdlib.h>
#include    <curses.h>
#include    <signal.h>
#include    <sys/time.h>
#include    <fcntl.h>
#include    <time.h>
#include    <ctype.h>

#include    "timers.h"
#include    "bacteria.h"



#define    INITIAL_SPEED   1
#define    MAX_SPEED   20
#define    MIN_SPEED   1
#define    MESSAGE      "command:"

#define    DEFAULT_FILENAME           ".genesis"


int         _paused = 1;
int        row;           // current row
int        col;           // current column
int        dir;           // Global variable to store direction of movement
int        speed;         // Current speed in chars/second
volatile   sig_atomic_t   input_ready;
int        finished;
int initiated = 0;
int longOption = 0;
int input_disabled = 0;

char  mssge[10];

/*****************************************************************************/
/*                       Signal Handler Prototypes                           */
/*****************************************************************************/
void    on_alarm(int);    /* handler for alarm    */
void    on_input(int);    /* handler for keybd    */
int update_from_input( int c, int *speed); /*handle user input. I followed the advice from  cs493.66 Google group*/

/* starts or resumes the simulation based on global var _pause */
void startResumeSimulation();

/*calls the set_timer() with zeros aguments */
void pauseSimulation();
/*read an input file*/
int readFile(const char *filename);
/*prints the "donut" surface on the screen */
void displaySurface();
/*sets timer */
void setResetTimer();
/*saves the output to file*/
int dumpToFile();
//jupm n steps
void njump(int n);


/*****************************************************************************/
/*                                    Main                                   */
/*****************************************************************************/
int main( int argc, char * argv[])
{
    if(argc > 2){
        fprintf(stderr, "Cannot read more than one .genesis file %s\n", argv[2]);
        return 1;
    }

    struct sigaction newhandler;      // for installing handlers
    sigset_t         blocked;         // to set mask for handler
    int  fd_flags;
    int    c;
    mssge[0] = '\0';


    // Set up signal handling
    newhandler.sa_handler = on_input; // name of handler
    newhandler.sa_flags = SA_RESTART; // flag is just RESTART
    sigemptyset(&blocked);            // clear all bits of blocked set
    newhandler.sa_mask = blocked;     // set this empty set to be the mask
    if ( sigaction(SIGIO, &newhandler, NULL) == -1 ) {
        perror("sigaction");
        return (1);
    }

    sigemptyset(&blocked);            // clear all bits of blocked set
    newhandler.sa_mask = blocked;     // set this empty set to be the mask
    newhandler.sa_handler = on_alarm; // SIGALRM handler function
    if ( sigaction(SIGALRM, &newhandler, NULL) == -1 ){  // try to install
        perror("sigaction");
        return (1);
    }

    // Prepare the terminal for the animation
    initscr();     // initialize the library and screen
    cbreak();      // put terminal into non-blocking input mode
    noecho();      // turn off echo
    clear();       // clear the screen
    //curs_set(0);   // hide the cursor

    finished   = 0;
    speed      = INITIAL_SPEED ;
    input_ready = 0;


    getmaxyx(stdscr,_rows,_cols);
    _rows--;
    allocSurfaces();

    //randInit(); //used for testing
    //load file
    if(argc == 2 && readFile(argv[1]))
        {endwin(); return 1;}
    if(argc == 1 && readFile(DEFAULT_FILENAME))
        {endwin(); return 1;}

    // Turn on keyboard signals
    fcntl(0, F_SETOWN, getpid());
    fd_flags = fcntl(0, F_GETFL);
    fcntl(0, F_SETFL, (fd_flags|O_ASYNC));

    displaySurface();

    while( !finished  ) {
        if ( input_ready ) {
                    c = getch();
            finished = update_from_input(c, &speed);
            input_ready = 0;
        }
        else
            pause();
    }
    clear();
    endwin();
    return 0;
}

/*****************************************************************************/

int update_from_input( int c, int *speed )
{
    int len = strlen(mssge);
    /******** Handle 'j' potion **************/
    if(longOption){
        if(c >= '0' && c <= '9'){
            mssge[len] = c;
            mssge[len+1] = '\0';
        }
        else {
            njump(atoi(mssge+1));
            mssge[0] = '\n';
            mssge[1] = '\0';
            longOption = 0;
        }
    }
    /******************************************/
    else {
        mssge[0] = c;
        mssge[1] = '\0';

        int is_changed = 0;

        switch (c) {
        case 'Q':
        case 'q':
            freeSurfaces();                   //clean up
            return 1;                         // quit program
        case 'b':
            if(!initiated){
                startResumeSimulation();      //begin
                initiated = 1;
            }
            break;
        case 'p':
            if(initiated) pauseSimulation();    //pause
            break;
        case 'r':
            if(initiated) startResumeSimulation(); //resume
            break;
        case '+':
            if(initiated && !_paused)
                if (*speed < MAX_SPEED ) {
                *speed = *speed + 1;            // increase
                is_changed = 1;
            }
            break;
        case '-':
            if(initiated && !_paused)
                if (*speed >  MIN_SPEED ) {
                *speed = *speed - 1 ;           // decrease
                is_changed = 1;
            }
            break;
        case 'j':
            if(!initiated || _paused){
                longOption = 1;
            }
            break;
        case 's' :
            dumpToFile();
            break;
        }

        if ( is_changed ) {
            setResetTimer();
        }
    }


    mvaddstr( _rows, 0 ,   MESSAGE);
    addstr(mssge);
    refresh();

    return 0;
}

/*****************************************************************************/


void on_input(int signum)
{
    if(!input_disabled)
        input_ready = 1;
}

/*****************************************************************************/

void on_alarm(int signum)
{
    exeStep();
    displaySurface();
}
/******************************************************************************/
void displaySurface(){
    int i,j;
    for(i=0; i < _rows; i++)
        for(j=0; j < _cols; j++)
            mvaddch(i,j,_surface[i][j]?'O':' ');


    mvaddstr( _rows, 0 ,   MESSAGE);
    addstr(mssge);
    refresh();
}

/******************************************************************************/
void startResumeSimulation(){
    if(_paused){
        setResetTimer();
        _paused = 0;
    }
}
/******************************************************************************/
void pauseSimulation(){
    if(!_paused){
        set_timer( ITIMER_REAL, 0, 0);
        _paused = 1;
    }
}
/******************************************************************************/
void njump(int n){
    int i = 0;
    for(;i<n;i++)
        exeStep();
    displaySurface();
}
/******************************************************************************/
void setResetTimer(){
    set_timer( ITIMER_REAL, 2000/speed, 2000/speed );
}
/******************************************************************************/
int readFile(const char *filename)
{
    int r,c,line_count=0;
    int max_lines = _rows * _cols;
    // Open file
    FILE *fp;
    if (!(fp = fopen(filename, "r"))) {
        fprintf(stderr, "Unable to open file %s\n", filename);
        return 1;
    }

    int i,j;
    for(i=0; i < _rows; i++)
        for(j=0;j<_cols;j++)
            _surface[i][j] = 0;

    char buffer[1024];
    while (fgets(buffer, 1023, fp) && max_lines > line_count) {

        char *bufferp = buffer;
        while (isspace(*bufferp)) bufferp++;

        // Skip blank lines and comments
        if (*bufferp == '#') continue;
        if (*bufferp == '\0') continue;

        if (sscanf(bufferp, "%d%d", &r, &c) == 2){
            r-- ; c--;
            line_count++;
        }
        else {
            fprintf(stderr, "Syntax error reading header on line %d in file %s\n", line_count, filename);
            fclose(fp);
            return 1;
        }
        _surface[r % _rows][c % _cols] = 1;
    }
    saveSurface();
    fclose(fp);
    return 0;
}
/******************************************************************************/
int dumpToFile(){
    input_disabled = 1;
    char buffer [20];
    sprintf (buffer, "%s_%d", DEFAULT_FILENAME, getpid());

    FILE *fp;
    if (!(fp = fopen(buffer, "w"))) {
        fprintf(stderr, "Unable to open file %s\n", buffer);
        return 1;
    }

    int i,j;
    for(i=0; i < _rows; i++)
        for(j=0; j < _cols; j++)
            if(_surface[i][j])
                fprintf(fp, "%d %d \n", i+1, j+1);


    fclose(fp);
    input_disabled = 0;
    return 0;
}


