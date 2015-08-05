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
    try {
        ipt::input testing(argv[1],argv[2]);
    } catch (std::string error) dnntsErrorcatch(error);

    //--------------------------------
    //     Generate Random Numbers
    //--------------------------------
    // This is passed to each thread and contain unique seeds for each
    ParallelSeedGenerator seedGen(omp_get_max_threads());

    // This is the termination string. If an error is caught it
    // saves it here and the threads then exit.
    std::string termstr("");

    // Begin parallel region
    #pragma omp parallel
    {
        // Number of sets to calculate
        int N=100;

        // Thread ID
        int tid = omp_get_thread_num();

        // Prepare the random number seeds
        std::vector<int> seedarray;
        seedGen.getThreadSeeds(tid,seedarray);

        // Prepare the random number generator
        NormRandomReal rnGen;

        int i=0;
        while (i<N && termstr.empty()) {
            try
            {
            std::vector<float> rn;
            rnGen.fillVector(0.0,1.0,rn,10,seedarray);


            ++i;
            }
            catch (std::string error)
            {
                #pragma omp critical
                {
                    termstr=error;
                }
            }
        }
    }

    // Catch any errors from the threads
    if (!termstr.empty()) dnntsErrorcatch(termstr);

    //--------------------------------
    //          Run G09 Jobs
    //--------------------------------
    std::vector< glm::ivec2 > bonds;
    bonds.push_back(glm::ivec2(0,1));
    bonds.push_back(glm::ivec2(0,2));

    std::vector<std::string> type;
    type.push_back("O");
    type.push_back("H");
    type.push_back("H");

    std::vector<glm::vec3> xyz;
    xyz.push_back(glm::vec3(0.000,0.000,0.000));
    xyz.push_back(glm::vec3(0.750,0.000,0.520));
    xyz.push_back(glm::vec3(0.750,0.000,-0.52));

    try {
        itrnl::Internalcoordinates icrd(bonds);
        std::cout << "STRING: " << icrd.calculateCSVInternalCoordinates(xyz) << std::endl;;

    } catch (std::string error) dnntsErrorcatch(error);

    try {
        std::string input(g09::buildInputg09("AM1","force",type,xyz,0,1,1));

        //std::string input = "\n#p AM1 force\n\nwater\n\n0  1\nO 0.0000 0.0000 0.0000\nH 0.7500 0.0000 0.5200\nH 0.7500 0.0000 -0.520\n\n";
        //std::cout << "G09 ERROR: " << g09::execg09(input) << std::endl;
    } catch (std::string error) dnntsErrorcatch(error);

    return 0;
};
