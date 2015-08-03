#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>

#include "errorhandlers.h"

#define Fatal(_error)                                 \
{                                                     \
    std::stringstream _location,_message;             \
    _location << __FILE__ << ":" << __LINE__ << "\n"; \
    _message << "Error -- " << _error << "\n";        \
    std::cerr << "In " << _location << _message;      \
    return(EXIT_FAILURE);                             \
};

#define checkArgs(_args)                            \
{                                                   \
    if(_args==NULL)                                 \
    {                                               \
        std::stringstream _error;                   \
        _error << "Invalid arguments";              \
        Fatal(_error);                              \
    }                                               \
};

int main (int argc, char *argv[])
{
    checkArgs(argv[1]);

    /*if (argc != 4)
    {
        std::cout << "Failure, wrong number of arguments.\n";
        std::cout << "[executable] [inputfilename] [number of atoms] [number of steps]\n";
        exit(EXIT_FAILURE);
    }*/

    return 0;
};
