// Std Lib Includes
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
#include <cmath>

// GLM Mathematics
#include <glm/glm.hpp>

// Error Handlers
#include "../errorhandlers.h"

// Utilities
#include "../utils/simpletools.hpp"
#include "../utils/systools.hpp"
#include "../utils/randnormflt.h"

// Handlers
#include "../handlers/g09functions.hpp"
#include "../handlers/input.h"
#include "../handlers/internalcoordinate.h"

// Class Definition
#include "tsbuilder.h"

/*--------Calculate Training Set---------

----------------------------------------*/
void Trainingsetbuilder::calculateTrainingSet()
{
    std::cout << "\033[1;30m------------------------------" << std::endl;
    std::cout << "  Begin building training set " << std::endl;
    std::cout << "------------------------------\033[0m\n" << std::endl;

    // Store working parameters
    ipt::Params params(iptData->getparams());
    params.printdata();

    // Store input coordinates and atom types locally
    std::vector<glm::vec3> ixyz(iptData->getxyz());
    std::vector<std::string> types(iptData->gettypes());

    // Local pointer to icrd for passing a private class to threads
    itrnl::Internalcoordinates* licrd = &icrd;
    licrd->printdata();

    // This is passed to each thread and contain unique seeds for each
    int MaxT = omp_get_max_threads();
    std::cout << "Using " << MaxT << " threads." << std::endl;
    for (int i=0;i<MaxT;++i) {std::cout << std::endl;}

    ParallelSeedGenerator seedGen(MaxT);

    // This is the termination string. If an error is caught it
    // saves it here and the threads then exit.
    std::string termstr("");

    // Begin parallel region
    #pragma omp parallel default(shared) firstprivate(types,ixyz,params,MaxT,licrd)
    {
        // Thread ID
        int tid = omp_get_thread_num();

        // Number of sets for this thread to calculate
        int N = floor(params.tts/MaxT);
        if (tid < params.tts % MaxT) {++N;}

        // Prepare the random number seeds
        std::vector<int> seedarray;
        seedGen.getThreadSeeds(tid,seedarray);

        // Prepare the random number generator
        NormRandomReal rnGen(seedarray);

        // Allocate space for new coordinates
        std::vector<glm::vec3> wxyz(params.Na);

        // Initialize counter
        int i(0);

        // Initialize some containers
        std::string datapoint;
        std::string input;
        std::string output;

        // Begin main loop
        while (i<N && termstr.empty())
        {
            try
            {
                bool gchk = true;

                while (gchk)
                {
                    gchk = false;
                    wxyz = m_generateRandomStructure(ixyz,rnGen);
                    gchk = m_checkRandomStructure(wxyz);

                    datapoint.append(licrd->calculateCSVInternalCoordinates(wxyz));

                    g09::buildInputg09(input,params.llt,"force",types,wxyz,0,1,1);
                    if (g09::execg09(input,output)) {gchk=true;}
                    datapoint.append(g09::forceFinder(output));

                    g09::buildInputg09(input,params.hlt,"force",types,wxyz,0,1,1);
                    if (g09::execg09(input,output)) {gchk=true;}
                    datapoint.append(g09::forceFinder(output));

                    //if (gchk)
                }

                #pragma omp critical
                {
                    datapoint.pop_back();
                    tsout << datapoint << std::endl;
                    datapoint.clear();
                    std::cout << "\033["<< tid+1 <<"A\033[K\033[1;30mThread " << tid << " is " << round((i/float(N))*100.0) << "% complete.\033["<< tid+1 <<"B\033[100D";
                }

                ++i;
            }
            catch (std::string error)
            {
                #pragma omp critical
                {
                    termstr=error;
                    #pragma omp flush (termstr)
                }
            }
        }

        #pragma omp critical
        {
            std::cout << "\033["<< tid+1 <<"A\033[K\033[1;30mThread " << tid << " is " << 100 << "% complete.\033["<< tid+1 <<"B\033[100D";
        }
    }

    // Catch any errors from the threads
    if (!termstr.empty()) dnntsErrorcatch(termstr);

    std::cout << "\n\033[1;30m------------------------------" << std::endl;
    std::cout << "Finished building training set" << std::endl;
    std::cout << "------------------------------\033[1;0m" << std::endl;
};

/*-----Generate a Random Structure-------

----------------------------------------*/
std::vector<glm::vec3> Trainingsetbuilder::m_generateRandomStructure(const std::vector<glm::vec3> &ixyz,NormRandomReal &rnGen)
{
    std::vector<glm::vec3> wxyz(ixyz.size());

    std::vector<float> rn;
    rnGen.fillVector(0.0,1.0,rn,3*ixyz.size());

    for (uint32_t i=0;i<ixyz.size();++i)
    {
        wxyz[i].x = ixyz[i].x + rn[i*3];
        wxyz[i].y = ixyz[i].y + rn[i*3+1];
        wxyz[i].z = ixyz[i].z + rn[i*3+2];
    }

    return wxyz;
};

/*------Check a Random Structure--------

----------------------------------------*/
bool Trainingsetbuilder::m_checkRandomStructure(const std::vector<glm::vec3> &xyz)
{

    return false;
};

