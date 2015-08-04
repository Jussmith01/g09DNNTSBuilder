#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>

#include "errorhandlers.h" // Contains the error handlers.
#include "utils/randnormflt.h"
#include "utils/readinput.h"

int main (int argc, char *argv[])
{
    // Check input arguments via macro
    //checkArgs(argv[1]);
    //checkArgs(argv[2]);

    // Read input


    // Generate Random Numbers
    long int Nr(1000); // Number of random numbers

    try
    {
        NormRandomReal randflt(Nr,clock());

        std::vector<float> rdnflts;
        randflt.fillVector(0.0,0.1,rdnflts,Nr);
    }
    catch (std::string error) dnntserrorcatch(error);



    return 0;
};
