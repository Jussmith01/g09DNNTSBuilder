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
#include "../utils/conservation.h"

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
void print_for_file(int tid,int N,int i,int gcfail,int gdfail) {
    int trdcomp = static_cast<int>(round((i/float(N))*100.0));

    if (trdcomp % 5 == 0) {
        std::cout << "Thread " << tid << " is " << trdcomp << "% complete. G09 Convergence Fails " << gcfail << " Distance Fails: " << gdfail << "\n";
    }
};

void print_for_cout(int tid,int N,int i,int gcfail,int gdfail) {
    std::cout << "\033["<< tid+1 <<"A\033[K\033[1;30mThread " << tid << " is " << round((i/float(N))*100.0) << "% complete. G09 Convergence Fails " << gcfail << " Distance Fails: " << gdfail << "\033["<< tid+1 <<"B\033[100D\033[0m";
};


/*--------Calculate Training Set---------

This function contians the main loop for
building the training set.
----------------------------------------*/
void Trainingsetbuilder::calculateTrainingSet() {
    std::cout << "------------------------------" << std::endl;
    std::cout << "  Begin building training set " << std::endl;
    std::cout << "------------------------------\n" << std::endl;

    // Store working parameters
    ipt::Params params(iptData->getparams());
    params.printdata();

    // Store input coordinates and atom types locally
    std::vector<glm::vec3> ixyz(iptData->getxyz());
    std::vector<std::string> types(iptData->gettypes());
    std::vector<double> masses(iptData->getmasses());

    // Local pointer to icrd for passing a private class to threads
    itrnl::Internalcoordinates* licrd = &icrd;
    licrd->printdata();

    // Get the maximum number of threads
    int MaxT = omp_get_max_threads();
    std::cout << "Using " << MaxT << " threads." << std::endl;

    // Setup loop output function
    void (*loopPrinter)(int tid,int N,int i,int gcfail,int gdfail);
    switch ((int)routecout) {
    case 0: {
        std::cout << "Output setup for terminal writing." << std::endl;
        loopPrinter = &print_for_cout;
        for (int i=0; i<MaxT; ++i) {
            std::cout << std::endl;
        }
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
    for (it = outname.begin(); it != outname.end(); it++)
        *it << iptData->getoname() << "_thread" << it - outname.begin();

    // This is passed to each thread and contain unique seeds for each
    ParallelSeedGenerator seedGen(MaxT);

    // This is the termination string. If an error is caught it
    // saves it here and the threads then exit.
    std::string termstr("");

    // Begin parallel region
    #pragma omp parallel default(shared) firstprivate(types,ixyz,masses,params,MaxT,licrd)
    {
        std::vector<glm::vec3> ixyz_center;
        ixyz_center = ixyz;

        // Create the conservation object;
        //conservation water(ixyz_center,masses);

        // Thread ID
        int tid = omp_get_thread_num();

        // Some local variables
        int nrpg = params.nrpg;

        // Minimimum number of sets for this thread to calculate
        int N = floor(params.tts/MaxT);
        if (tid < params.tts % MaxT) {
            ++N;
        }

        // Prepare the random number seeds
        std::vector<int> seedarray;
        seedGen.getThreadSeeds(tid,seedarray);

        // Prepare the random number generator
        RandomReal rnGen(seedarray,params.mean,params.std,args->getflag("-r"));

        // Allocate space for new coordinates
        std::vector<glm::vec3> wxyz(params.Na*nrpg);

        // Initialize counters
        int i(0); // Loop counter
        int gcf(0); // Gaussian Convergence Fail counter
        int gdf(0); // Geometry distance failure

        // Initialize some containers
        std::string datapoint;
        std::string input;
        std::vector<std::string> outsll(nrpg);
        std::vector<bool> chkoutsll(nrpg);
        std::vector<std::string> outshl(nrpg);
        std::vector<bool> chkoutshl(nrpg);

        // Define and open thread output
        std::ofstream tsoutt;
        tsoutt.open(outname[tid].str());

        // Thread timers
        MicroTimer mttimer; // Time the whole loop
        MicroTimer mrtimer; // Time the random structure generation
        MicroTimer mgtimer; // Time the gaussian 09 runs
        //MicroTimer mgtimer; // Time the gaussian 09 runs
        MicroTimer mstimer; // Time the string generation

        // Begin main loop
        mttimer.start_point();
        while (i<N && termstr.empty()) {
            try {
                //std::cout << i << std::endl;

                /*------Structure Generation------

                ---------------------------------*/
                // Generate the random structure
                mrtimer.start_point();
                m_generateRandomStructure(nrpg,ixyz,wxyz,rnGen);

                /*ASDUJASIDHIUADHASD*/
                //water.conserve(wxyz);
                mrtimer.end_point();

                /*------Gaussian 09 Running-------

                ---------------------------------*/
                mgtimer.start_point();
                // Build the g09 input file for the low level of theory
                //g09::buildInputg09(nrpg,input,params.llt,"force",types,wxyz,0,1,1);

                // Execute the g09 run, if failure occures we restart the loop
                //g09::execg09(nrpg,input,outsll,chkoutsll);

                // Build the g09 input file for the high level of theory
                g09::buildInputg09(nrpg,input,params.hlt,"force",types,wxyz,0,1,1);

                // Execute the g09 run, if failure occures we restart the loop
                g09::execg09(nrpg,input,outshl,chkoutshl);
                std::stringstream sso;
                std::stringstream ssi;
                sso << "g09output." << tid << "." << i << ".dat";
                ssi << "g09input." << tid << "." << i << ".dat";
                std::ofstream instream(ssi.str().c_str());
                if (instream)
                {
                    instream << input;
                }
                else
                {
                    std::cerr << "bad dustin" << std::endl;
                }
                std::ofstream ostream(sso.str().c_str());
                if (ostream)
                {
                    ostream << outshl[0];
                }
                else
                {
                    std::cerr << "bad dustin" << std::endl;
                }
                instream.close();
                ostream.close();

                mgtimer.end_point();

                /*----Creating CSV Datapoint------

                ---------------------------------*/
                mstimer.start_point();
                // Append the data to the datapoint string
                for (int j=0; j<nrpg; ++j) {
                    //if (!chkoutshl[j] && !chkoutsll[j]) {
                    if (!chkoutshl[j]) {
                        std::vector<glm::vec3> xyzind(ixyz.size());
                        std::memcpy(&xyzind[0],&wxyz[j*ixyz.size()],ixyz.size()*sizeof(glm::vec3));
                        datapoint.append(licrd->calculateCSVInternalCoordinates(xyzind));
                        //datapoint.append(g09::forceFinder(outsll[j]));
                        datapoint.append(g09::forceFinder(outshl[j]));

                        // Save the data point to the threads private output file output
                        tsoutt << datapoint << std::endl;
                        datapoint.clear();
                        ++i;
                    } else {
                        ++gcf;
                    }
                }
                mstimer.end_point();

                // Loop printer.
                #pragma omp critical
                {
                    loopPrinter(tid,N,i,gcf,gdf);
                }

            } catch (std::string error) {
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
            loopPrinter(tid,1,1,gcf,gdf);
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
            std::cout << "Number of gaussian convergence failures: " << gcf << std::endl;
            std::cout << "Number of geometry distance check failures: " << gcf << std::endl;
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
    for (nameit = outname.begin(); nameit != outname.end(); nameit++) {
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
void Trainingsetbuilder::m_generateRandomStructure(int nrpg,const std::vector<glm::vec3> &ixyz,std::vector<glm::vec3> &oxyz,RandomReal &rnGen) {
    // Number of coords per molecules
    int N = ixyz.size();

    // Do this for all nrpg gaussian inputs
    for (int j=0; j<nrpg; ++j) {
        bool sgot = false;
        while (!sgot) {
            std::vector<glm::vec3> wxyz(ixyz.size());
            std::vector<float> rn;

            rnGen.fillVector(rn,3*ixyz.size());

            for (uint32_t i=0; i<ixyz.size(); ++i) {
                //wxyz[i].x = ixyz[i].x + rn[i*3];
                wxyz[i].x = rn[i*3];
                //wxyz[i].x = ixyz[i].x;
                //wxyz[i].y = ixyz[i].y + rn[i*3+1];
                wxyz[i].y = 0.0f; //rn[i*3+1];
                //wxyz[i].y = ixyz[i].y;
                //wxyz[i].z = ixyz[i].z + rn[i*3+2];
                wxyz[i].z = 0.0f; //rn[i*3+2];
                //wxyz[i] = ixyz[i] * rn[0];
            }

            // Check structure distances for viability
            if (!m_checkRandomStructure(wxyz)) {
                // Save structure in output xyz vector
                for (uint32_t i=0; i<ixyz.size(); ++i) {
                    oxyz[j*N+i] = wxyz[i];
                }
                sgot=true;
            }
        }
    }
};

/*------Check a Random Structure--------

----------------------------------------*/
bool Trainingsetbuilder::m_checkRandomStructure(const std::vector<glm::vec3> &xyz) {
    bool failchk = false; // Defaults to no failure

    for (uint32_t i=0; i<xyz.size(); ++i) {
        for (uint32_t j=i+1; j<xyz.size(); ++j) {
            if (glm::length(xyz[i]-xyz[j]) < 0.5) {
                failchk = true;
                break;
            }
        }

        if (failchk) {
            break;
        }
    }

    return failchk;
};

