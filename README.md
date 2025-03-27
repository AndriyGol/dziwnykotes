# dziwnykotes
A two-dimensional simulation of a rather unusual colony of single-celled organisms called dziwnykotes.

## Build 

```
gcc -o dziwnykotes -lncurses timers.c timers.h bacteria.h main.c
```

  Author         : Andriy Goltsev
  
  Created on     : April 26, 2011

  
 ## Description     
  
A two-dimensional simulation of a rather unusual colony of single-celled organisms called dziwnykotes. These organisms reproduce in an unusual way, and they are also very sensitive to their surroundings. If they are too crowded, they die or, and if they are not surrounded by enough of their kind, they also die. They reproduce only under very specifc constraints: a new organism is born as a result of a three-way interaction of the organisms, a menage-a-trois, so to speak.

  
  ## Usage 
  
  Begin the simulation with 'b' (only in the initial state.)
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
  
  Notes:       Based on bouncestr_async3.c written by Stewart Weiss.
               The assignment was done on the rush in the last minute. I am not very happy about the implemenatation.
