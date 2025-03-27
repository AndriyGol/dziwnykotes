/******************************************************************************
  Title          : bacteria.c
  Author         : Andriy Goltsev
  Created on     : April 26, 2011
  Description    : Represents the "donut" surface.

******************************************************************************/

#ifndef _BACTERIA_H
#define _BACTERIA_H

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

typedef int DZIWNYKOTES;


DZIWNYKOTES** _surface;
DZIWNYKOTES** _surfaceCopy;
int _cols;
int _rows;

//the rules by which bacterias live//
int rule(int alive, int neigbours){
    if(alive && (neigbours == 2 || neigbours == 3)) return 1;
    if(!alive && neigbours == 3) return 1;
    return 0;
}
//allocate space
void allocSurfaces(){
    _surface = malloc(sizeof(DZIWNYKOTES*) * _rows);
    _surfaceCopy  = malloc(sizeof(DZIWNYKOTES*) * _rows);
    int i = 0;
    for( ; i < _rows; i++){
        _surface[i] = calloc(_cols, sizeof(DZIWNYKOTES));
        _surfaceCopy[i]  = calloc( _cols, sizeof(DZIWNYKOTES));
    }
}
//deallocate
void freeSurfaces(){
    int i = 0;
    for( ; i < _rows; i++){
        free( _surface[i]);
        free( _surfaceCopy[i] );
    }
    free(_surface);
    free(_surfaceCopy);
}
//saves the bacteria's state to the temp array
void saveSurface() {
    int i, j;
    for(i = 0; i < _rows; i++){
        for(j = 0; j < _cols; j++){
            _surfaceCopy[i][j] = _surface[i][j];
        }
    }
}
//number of Dziwnykotes around x,y coord
int aliveDziwnykotesAround(int x, int y){
    int aliveCells = 0;
    x+=_rows;
    y+=_cols;
    int i,j;
    for(i = -1; i < 2; i++)
        for(j = -1; j < 2; j++){
            if(0 == i && 0 == j) continue;
            aliveCells += _surfaceCopy[(x+i)%_rows][(y+j)%_cols];
    }
    return aliveCells;
}
//Simulate one time unit
void exeStep() {
    int i, j, aliveNeighbours;
    for(i = 0; i < _rows; i++){
        for(j = 0; j < _cols; j++){
            aliveNeighbours =  aliveDziwnykotesAround(i,j);
            _surface[i][j] = rule(_surfaceCopy[i][j], aliveNeighbours);
        }
    }
    saveSurface();
}
//used for testing. random initialization of the surface
void randInit() {
    srand(time(NULL));
    int i, j;
    for(i = 0; i < _rows; i++){
        for(j = 0; j < _cols; j++){
            _surface[i][j] = _surfaceCopy[i][j] = rand()%2;
        }
    }
}

#endif
