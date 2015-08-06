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

// Error Handling
#include "errorhandlers.h" // Contains the error handlers.

// Utilities
#include "utils/randnormflt.h"
#include "utils/simpletools.hpp"
#include "utils/systools.hpp"

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
    checkArgs(argv[1]);
    checkArgs(argv[2]);

    //--------------------------------
    //          Read input
    //--------------------------------
    ipt::input iptdata(argv[1],argv[2]);

    //--------------------------------
    //        Main Loop Class
    //--------------------------------
    // Construct/prepare the class
    Trainingsetbuilder tsb(&iptdata);

    //
    tsb.calculateTrainingSet();

    //try {
        //Trainingsetbuilder tsb(iptdata);
    //} catch (std::string error) dnntsErrorcatch(error);

    /*try {
        std::string input(g09::buildInputg09("AM1","force",type,xyz,0,1,1));

        //std::cout << "G09 ERROR: " << g09::execg09(input) << std::endl;
    } catch (std::string error) dnntsErrorcatch(error);*/

    return 0;
};
