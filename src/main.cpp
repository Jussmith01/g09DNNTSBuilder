// Standary Library
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>

// Error Handling
#include "errorhandlers.h" // Contains the error handlers.

// Utilities
#include "utils/randnormflt.h"
#include "utils/readinput.h"
#include "utils/simpletools.hpp"
#include "utils/systools.hpp"

int main (int argc, char *argv[])
{
    //--------------------------------
    // Check input arguments via macro
    //--------------------------------
    checkArgs(argv[1]);
    checkArgs(argv[2]);

    inputData.fname = argv[1];
    inputData.oname = argv[2];

    //--------------------------------
    //          Read input
    //--------------------------------
    try
    {

    }
    catch (std::string error) dnntsErrorcatch(error);

    //--------------------------------
    //     Generate Random Numbers
    //--------------------------------
    long int Nr(1000); // Number of random numbers

    try
    {
        NormRandomReal randflt(Nr,clock());

        std::vector<float> rdnflts;
        randflt.fillVector(0.0,0.1,rdnflts,Nr);
    }
    catch (std::string error) dnntsErrorcatch(error);

    //--------------------------------
    //          Run G09 Jobs
    //--------------------------------
    try
    {
        std::cout << "G09 ERROR: " << systls::execg09("gtest.com","gtest.log") << std::endl;
    }
    catch (std::string error) dnntsErrorcatch(error);

    return 0;
};
