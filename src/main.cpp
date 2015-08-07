// Standary Library
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <iomanip>
#include <vector>
#include <omp.h>
#include <regex>
#include <unordered_map>

// Error Handling
#include "errorhandlers.h" // Contains the error handlers.

// Utilities
#include "utils/randnormflt.h"
#include "utils/simpletools.hpp"
#include "utils/systools.hpp"
#include "utils/micro_timer.h"
#include "utils/flaghandler.h"

// Handlers
#include "handlers/input.h"
#include "handlers/g09functions.hpp"
#include "handlers/internalcoordinate.h"

// Core
#include "core/tsbuilder.h"

int main(int argc, char *argv[])
{
    //--------------------------------
    // Check input arguments via macro
    //--------------------------------
    FlagHandler args(argc,argv);

    // Setup timer
    MicroTimer mtimer;

    //--------------------------------
    //          Read input
    //--------------------------------
    ipt::input iptdata(args.getflag("-i"),args.getflag("-d"));

    //--------------------------------
    //        Main Loop Class
    //--------------------------------
    // Construct/prepare the class
    mtimer.start_point();
    Trainingsetbuilder tsb(&iptdata,&args);
    mtimer.end_point();
    mtimer.print_generic_to_cout(std::string("Training set builder class preparation time --\n"));

    // Calculate the training set
    mtimer.start_point();
    tsb.calculateTrainingSet();
    mtimer.end_point();
    mtimer.print_generic_to_cout(std::string("Calculate training set time --\n"));

    return 0;
};
