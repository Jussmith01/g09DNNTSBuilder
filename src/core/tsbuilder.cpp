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
#include <unordered_map>

// GLM Mathematics
#include <glm/glm.hpp>

// Error Handlers
#include "../errorhandlers.h"

// Utilities
#include "../utils/simpletools.hpp"
#include "../utils/systools.hpp"
#include "../utils/randnormflt.h"
#include "../utils/micro_timer.h"
#include "../utils/flaghandler.h"

// Handlers
#include "../handlers/g09functions.hpp"
#include "../handlers/input.h"
#include "../handlers/internalcoordinate.h"

// Class Definition
#include "tsbuilder.h"

/*--------Loop Printer Functions---------

These functions supply different file output
styles for different output interfaces

print_for_file -- Formatted for file output
Contains no ANSI Escape Sequences

print_for_cout -- Formatted for terminal output
Contains ANSI Escape Sequences

A function pointer called loopPrinter in
Trainingsetbuilder::calculateTrainingSet
is set before the loop and used to point
whcih of these functions is requested.
----------------------------------------*/
void print_for_file(int tid,int N,int i,int gfail)
{
    int trdcomp = static_cast<int>(round((i/float(N))*100.0));

    if (trdcomp % 5 == 0)
    {
        std::cout << "Thread " << tid << " is " << trdcomp << "% complete. G09 Fails " << gfail << "\n";
    }
};

void print_for_cout(int tid,int N,int i,int gfail)
{
    std::cout << "\033["<< tid+1 <<"A\033[K\033[1;30mThread " << tid << " is " << round((i/float(N))*100.0) << "% complete. G09 Fails " << gfail << "\033["<< tid+1 <<"B\033[100D\033[0m";
};

/*--------Calculate Training Set---------

This function contians the main loop for
building the training set.
----------------------------------------*/
void Trainingsetbuilder::calculateTrainingSet()
{
    std::cout << "------------------------------" << std::endl;
    std::cout << "  Begin building training set " << std::endl;
    std::cout << "------------------------------\n" << std::endl;

    // Store working parameters
    ipt::Params params(iptData->getparams());
    params.printdata();

    // Store input coordinates and atom types locally
    std::vector<glm::vec3> ixyz(iptData->getxyz());
    std::vector<std::string> types(iptData->gettypes());

    // Local pointer to icrd for passing a private class to threads
    itrnl::Internalcoordinates* licrd = &icrd;
    licrd->printdata();

    // Get the maximum number of threads
    int MaxT = omp_get_max_threads();
    std::cout << "Using " << MaxT << " threads." << std::endl;

    // Setup loop output function
    void (*loopPrinter)(int tid,int N,int i,int gfail);
    switch (routecout) {
        case 0: {
            std::cout << "Output setup for terminal writing." << std::endl;
            loopPrinter = &print_for_cout;
            for (int i=0;i<MaxT;++i) {std::cout << std::endl;}
            break;
        }
        case 1: {
            std::cout << "Output setup for file writing." << std::endl;
            loopPrinter = &print_for_file;
            break;
        }
    }

    // Prepare private thread output filenames
    std::vector<std::stringstream> outname(MaxT);
    std::vector<std::stringstream>::iterator it;
    for (it = outname.begin();it != outname.end();it++)
        *it << iptData->getoname() << "_thread" << it - outname.begin();

    // This is passed to each thread and contain unique seeds for each
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

        // Initialize counters
        int i(0); // Loop counter
        int f(0); // Fail counter

        // Initialize some containers
        std::string datapoint;
        std::string input;
        std::string outputll;
        std::string outputhl;

        // Define and open thread output
        std::ofstream tsoutt;
        tsoutt.open(outname[tid].str());

        // Thread timers
        MicroTimer mttimer; // Time the whole loop
        MicroTimer mrtimer; // Time the random structure generation
        MicroTimer mgtimer; // Time the gaussian 09 runs
        MicroTimer mstimer; // Time the string generation

        // Begin main loop
        mttimer.start_point();
        while (i<N && termstr.empty())
        {
            try
            {
                bool gchk = true;

                // This loop continues as long as the structure fails.
                // This ensures that we get the N requested data points
                while (gchk)
                {
                    // Default to no failures detected
                    gchk = false;

                    /*------Structure Generation------

                    ---------------------------------*/
                    // Generate the random structure
                    mrtimer.start_point();
                    wxyz = m_generateRandomStructure(ixyz,rnGen);

                    // Determine if structure is viable, if it is not, restart the loop.
                    if (m_checkRandomStructure(wxyz)) {
                        gchk=true;
                        mrtimer.end_point();
                        ++f;
                        continue;
                    }
                    mrtimer.end_point();

                    /*------Gaussian 09 Running-------

                    ---------------------------------*/
                    mgtimer.start_point();
                    // Build the g09 input file for the low level of theory
                    g09::buildInputg09(input,params.llt,"force",types,wxyz,0,1,1);

                    // Execute the g09 run, if failure occures we restart the loop
                    if (g09::execg09(input,outputll)) {
                        gchk=true;
                        ++f;
                        mgtimer.end_point();
                        continue;
                    }

                    // Build the g09 input file for the high level of theory
                    g09::buildInputg09(input,params.hlt,"force",types,wxyz,0,1,1);

                    // Execute the g09 run, if failure occures we restart the loop
                    if (g09::execg09(input,outputhl)) {
                        gchk=true;
                        ++f;
                        mgtimer.end_point();
                        continue;
                    }
                    mgtimer.end_point();
                }

                /*----Creating CSV Datapoint------

                ---------------------------------*/
                mstimer.start_point();

                // Append the data to the datapoint string
                datapoint.append(licrd->calculateCSVInternalCoordinates(wxyz));
                datapoint.append(g09::forceFinder(outputll));
                datapoint.append(g09::forceFinder(outputhl));
                mstimer.end_point();

                // Save the data point to the threads private output file output
                datapoint.pop_back();
                tsoutt << datapoint << std::endl;
                datapoint.clear();

                // Loop printer.
                #pragma omp critical
                {
                    loopPrinter(tid,N,i,f);
                }

                ++i;
            }
            catch (std::string error)
            {
                #pragma omp critical
                {
                    // Close the output if failure is detected
                    termstr=error;
                    #pragma omp flush (termstr)
                }
            }
        }
        mttimer.end_point();

        // Final print, shows 100%
        #pragma omp critical
        {
            loopPrinter(tid,1,1,f);
        }

        // Close the threads output
        tsoutt.close();

        // Wait for the whole team to finish
        #pragma omp barrier

        // Print stats for each thread in the team
        #pragma omp critical
        {
            std::cout << "\n|----Thread " << tid << " info----|" << std::endl;
            mttimer.print_generic_to_cout(std::string("Total"));
            mrtimer.print_generic_to_cout(std::string("Struc. Gen."));
            mgtimer.print_generic_to_cout(std::string("Gau. 09."));
            mstimer.print_generic_to_cout(std::string("CSV Gen."));
            std::cout << "Number of failed structures: " << f << std::endl;
            std::cout << "|---------------------|\n" << std::endl;
        }
    }

    std::cout << std::endl;

    // Combine all threads output
    MicroTimer fttimer;
    std::ofstream tsout;
    tsout.open(iptData->getoname().c_str(),std::ios_base::binary);

    fttimer.start_point();
    std::vector<std::stringstream>::iterator nameit;
    for (nameit = outname.begin();nameit != outname.end();nameit++)
    {
        // Move files individualy into the main output
        std::ifstream infile((*nameit).str().c_str(),std::ios_base::binary);
        std::cout << "Transferring file " << (*nameit).str() << " -> " << iptData->getoname() << std::endl;
        tsout << infile.rdbuf();

        // Remove old output once moved
        std::stringstream rm;
        rm << "rm " << (*nameit).str();
        systls::exec(rm.str(),100);
    }
    fttimer.end_point();
    fttimer.print_generic_to_cout(std::string("File transfer"));

    tsout.close();

    // Catch any errors from the threads
    if (!termstr.empty()) dnntsErrorcatch(termstr);

    std::cout << "\n------------------------------" << std::endl;
    std::cout << "Finished building training set" << std::endl;
    std::cout << "------------------------------" << std::endl;
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
