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
#include <time.h>
#include <chrono>
#include <random>

// GLM Mathematics
//#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

// Error Handlers
#include "../errorhandlers.h"

// Custom Libs
#include <internalcoordinate.h>

// Utilities
#include "../utils/simpletools.hpp"
#include "../utils/systools.hpp"
#include "../utils/randnormflt.h"
#include "../utils/micro_timer.h"
#include "../utils/flaghandler.h"

// Handlers
#include "../handlers/g09functions.hpp"
#include "../handlers/input.h"

// Class Definition
#include "tsnmbuilder.h"

/*      Optimizer

    Returns true if opt succeeded

*/
bool TrainingsetNormModebuilder::optimizer(std::string LOT
                                   ,std::string g09Args
                                   ,const std::vector<std::string> &itype
                                   ,std::vector<glm::vec3> &xyz
                                   ,unsigned charge
                                   ,unsigned multip) {

    using namespace std;

    string input;
    g09::buildCartesianInputg09(1,input,LOT,g09Args,itype,xyz,multip,charge,omp_get_max_threads());

    vector<string>    output(1);
    vector< bool > chkoutshl(1);

    cout << "Optimizing Structure at " << LOT << " level..." << endl;
    g09::execg09(1,input,output,chkoutshl);

    if ( !chkoutshl[0] ) {
        cout << LOT << " optimization complete... continuing." << endl;
        g09::ipcoordinateFinder(output[0],xyz,false);
    } else {
        cout << LOT << " optimization failed... continuing." << endl;
        g09::ipcoordinateFinder(output[0],xyz,true);
    }

    return !chkoutshl[0];
}

void TrainingsetNormModebuilder::optimizeStoredStructure() {
    using namespace std;

    // Store working parameters
    ipt::inputParameters params(*iptData);

    vector<  glm::vec3 > xyz  (rnmcrd.getixyz());
    vector<string> itype(rnmcrd.getitype());
    string HOT(params.getParameter<string>("LOT"));

    // Some local variables
    int charge (params.getParameter<int>("charge"));
    int multip (params.getParameter<int>("multip"));

    stringstream _args;
    _args << "opt";
    cout << " ARGUMENT1: " << _args.str() << endl;
    if ( !optimizer(HOT,_args.str(),itype,xyz,charge,multip) ) {
        if ( !optimizer(HOT,_args.str(),itype,xyz,charge,multip) ) {
            string iguess("Huckel");

            //-----------------------------
            // Low level minimization
            //-----------------------------
            stringstream _args2;
            cout << " ARGUMENT2: " << _args2.str() << endl;
            _args2 << "opt(cartesian,MaxStep=100,MaxCycles=1000) Guess(" << iguess << ")";
            optimizer("AM1",_args2.str(),itype,xyz,charge,multip);

            //-----------------------------
            // Medium level minimization
            //-----------------------------
            stringstream _args3;
            _args3 << "opt(cartesian,MaxStep=100,MaxCycles=1000) Guess(" << iguess << ")";
            optimizer("HF/6-31g*",_args3.str(),itype,xyz,charge,multip);

            //-----------------------------
            // Medium-High level minimization
            //-----------------------------
            stringstream _args4;
            _args4 << "opt(cartesian,MaxStep=100,MaxCycles=1000) Guess(" << iguess << ")";
            optimizer("MP2/6-31g*",_args4.str(),itype,xyz,charge,multip);

            //-----------------------------
            // High level minimization
            //-----------------------------


            bool mini(false);
            unsigned cnt(0);
            string conv("");
            while (!mini) {

                stringstream _args5;
                _args5 << "opt(cartesian" << conv << ",MaxStep=500,MaxCycles=1000) Guess(Huckel)";

                if ( !optimizer(HOT,_args5.str(),itype,xyz,charge,multip) ) {
                    ++cnt;

                    if (cnt == 2) {
                        cout << "Optimization Failed!! -- Aborting\n";
                        dnntsErrorcatch(string("Optimization Failed!!"));
                    }

                    string conv(",Loose");
                    cout << "Optimization Failed, switching to loose convergence and trying again..." << endl;
                } else {
                    mini = true;
                }
            }
        }
    };

    // Print Results
    cout << "------------------------------------" << endl;
    cout << " Optimized Coordinates:\n" << endl;
    cout.setf( ios::fixed, ios::floatfield );
    for (unsigned i=0; i<xyz.size(); ++i) {
        cout << " " << itype[i] << setprecision(7) << " " << setw(10) << xyz[i].x << " " << setw(10) << xyz[i].y << " " << setw(10) << xyz[i].z << endl;
    }

    rnmcrd.setixyz(xyz);
    cout << "------------------------------------" << endl;
    cout << "Optimization Complete.\n" << endl;
    iptData->storeInputWithOptCoords(xyz,true);
    cout << "Updated coordinates saved to the input file.\n" << endl;
}

/*         Normal Modes

    Returns true if freq succeeded

*/
bool TrainingsetNormModebuilder::normalmodecalc(std::string LOT
                                        ,std::string g09Args
                                        ,const std::vector<std::string> &itype
                                        ,const std::vector<glm::vec3> &xyz
                                        ,std::vector<std::vector<glm::vec3>> &nc
                                        ,std::vector<float> &fc
                                        ,unsigned charge
                                        ,unsigned multip) {

    using namespace std;

    string input;

    g09Args = " freq(noraman) NoSymmetry" + g09Args;

    g09::buildCartesianInputg09(1,input,LOT,g09Args,itype,xyz,multip,charge,omp_get_max_threads());

    vector<string>    output(1);
    vector< bool > chkoutshl(1);

    cout << "Freq Calculation at " << LOT << " level..." << endl;
    g09::execg09(1,input,output,chkoutshl);

    //cout << "FREQ:---------------------\n" << output[0] << endl;

    g09::normalmodeFinder(output[0],nc,fc,itype.size());

    return !chkoutshl[0];
}

void TrainingsetNormModebuilder::calculateNormalModes() {
    using namespace std;

    // Store working parameters
    ipt::inputParameters params(*iptData);

    vector<  glm::vec3 > xyz  (rnmcrd.getixyz());
    vector<string> itype(rnmcrd.getitype());
    string HOT(params.getParameter<string>("LOT"));

    // Some local variables
    int charge (params.getParameter<int>("charge"));
    int multip (params.getParameter<int>("multip"));

    std::vector<std::vector<glm::vec3>> nc; // Normal mode vectors
    vector<  float > fc;

    string args;
    if ( !normalmodecalc(HOT,args,itype,xyz,nc,fc,charge,multip) ) {
        throwException(string("ERROR: Normal mode calculation failed."));
    };

    rnmcrd.setixyz(xyz);
    cout << "------------------------------------" << endl;
    cout << "Normal Mode Calculation Complete.\n" << endl;
    iptData->storeInputWithNormModes(nc,fc,itype.size(),true);
    cout << "Updated normal modes and saved to the input file.\n" << endl;
}

/*--------Loop Printer Functions---------

These functions supply different file output
styles for different output interfaces

print_for_file -- Formatted for file output
Contains no ANSI Escape Sequences


print_for_cout -- Formatted for terminal output
Contains ANSI Escape Sequences

A function pointer called loopPrinter in
TrainingsetNormModebuilder::calculateTrainingSet
is set before the loop and used to point
whcih of these functions is requested.
----------------------------------------*/
void print_for_file2(int tid,int N,int i,int gcfail,int gdfail) {
    int trdcomp = static_cast<int>(round((i/float(N))*100.0));

    if (trdcomp % 5 == 0) {
        std::cout << "Thread " << tid << " is " << trdcomp << "% complete. G09 Convergence Fails " << gcfail << " Distance Fails: " << gdfail << "\n";
    }
};

void print_for_cout2(int tid,int N,int i,int gcfail,int gdfail) {
    std::cout << "\033["<< tid+1 <<"A\033[K\033[1;30mThread " << tid << " is " << static_cast<int>(round((i/float(N))*100.0)) << "% complete. G09 Convergence Fails " << gcfail << " Distance Fails: " << gdfail << "\033["<< tid+1 <<"B\033[100D\033[0m";
};

/*--------Calculate Validation Set---------


----------------------------------------*/
void TrainingsetNormModebuilder::calculateValidationSet() {

    iptData->setParameter("dfname",iptData->getParameter<std::string>("vdfname") );
    iptData->setParameter("TSS",iptData->getParameter<std::string>("VSS") );

    calculateTrainingSet();
}

/*--------Calculate Validation Set---------


----------------------------------------*/
void TrainingsetNormModebuilder::calculateTestSet() {

    iptData->setParameter("dfname",iptData->getParameter<std::string>("edfname") );
    iptData->setParameter("TSS",iptData->getParameter<std::string>("ESS") );

    calculateTrainingSet();
}

/*--------Calculate Training Set---------

This function contians the main loop for
building the training set.
----------------------------------------*/
void TrainingsetNormModebuilder::calculateTrainingSet() {
    using namespace std;
    cout << "------------------------------" << endl;
    cout << "  Begin building training set " << endl;
    cout << "  Normal Mode Structure Gen " << endl;
    cout << "------------------------------\n" << endl;

    // Store working parameters
    ipt::inputParameters params(*iptData);
    //params.printdata();

    // Get the maximum number of threads
    int MaxT = omp_get_max_threads();

    /* Setup thread seeds */
    unsigned seed1 ( std::chrono::system_clock::now().time_since_epoch().count() );
    unsigned seed2 ( clock() );

    std::seed_seq seedgen = {seed1,seed2};
    std::vector<unsigned> sequence (MaxT);

    seedgen.generate(sequence.begin(),sequence.end());

    for (unsigned i = 0; i < sequence.size(); ++i) {
        cout << "Random Seed for thread (" << i << "): " << sequence[i] << endl;
    }

    // Local pointer to icrd for passing a private class to threads
    //rnmcrd.printdata();

    string typescsv( simtls::stringsToCSV(rnmcrd.getotype()) );
    unsigned    atoms   ( rnmcrd.getNa() );

    cout << "atoms: " << atoms << endl;

    // Setup loop output function
    void (*loopPrinter)(int tid,int N,int i,int gcfail,int gdfail);
    switch ((int)routecout) {
    case 0: {
        cout << "Output setup for terminal writing." << endl;
        loopPrinter = &print_for_cout2;
        for (int i=0; i<MaxT; ++i) {
            cout << endl;
        }
        break;
    }
    case 1: {
        cout << "Output setup for file writing." << endl;
        loopPrinter = &print_for_file2;
        break;
    }
    }

    // Prepare private thread output filenames
    vector<stringstream> outname(MaxT);
    vector<stringstream>::iterator it;
    for (it = outname.begin(); it != outname.end(); it++)
        *it << iptData->getParameter<string>("dfname") << "_thread" << it - outname.begin();

    // This is passed to each thread and contain unique seeds for each
    //ParallelSeedGenerator seedGen(MaxT);

    // This is the termination string. If an error is caught it
    // saves it here and the threads then exit.
    string termstr("");

    unsigned convfail(0);

    // vector of energies
    std::vector<double> energies;

    // Begin parallel region
    #pragma omp parallel default(shared) firstprivate(params,MaxT)
    {
        // Thread safe copy of the internal coords calculator
        itrnl::RandomStructureNormalMode licrd = rnmcrd;

        // Thread ID
        unsigned tid = omp_get_thread_num();

        // Some local variables
        unsigned ngpr = params.getParameter<unsigned>("ngpr");
        unsigned tss = params.getParameter<unsigned>("TSS");
        float temp = params.getParameter<float>("Temp");

        // Minimimum number of sets for this thread to calculate
        int N = floor(tss/MaxT);
        if (tid < tss % MaxT) {
            ++N;
        }

        std::mt19937 rgenerator (sequence[tid]);

        // Allocate space for new coordinates
        unsigned na = licrd.getNa();//params.getCoordinatesStr().size();
        std::vector<glm::vec3> wxyz(na*ngpr);

        // Initialize counters
        int i(0); // Loop counter
        int gcf(0); // Gaussian Convergence Fail counter
        int gdf(0); // Geometry distance failure

        int charge (params.getParameter<int>("charge"));
        int multip (params.getParameter<int>("multip"));

        // Initialize some containers
        std::string datapoint;
        std::string input;
        std::vector<std::string> outsll(ngpr);
        std::vector<bool> chkoutsll(ngpr);
        std::vector<std::string> outshl(ngpr);
        std::vector<bool> chkoutshl(ngpr);

        std::vector<std::string> itype(licrd.getitype());

        std::string HOT(params.getParameter<std::string>("LOT"));
        std::string SCF(params.getParameter<std::string>("SCF"));

        // Z-matrix Stuff
        std::vector<std::string> zmat(ngpr);
        //std::vector< itrnl::t_iCoords > icord(ngpr);
        std::vector<std::string> cord(ngpr);

        // Cartesian and force temp storage
        std::vector< glm::vec3 > tcart(na);
        std::vector< glm::vec3 > tfrce(na);

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
                // Generate the random structures
                mrtimer.start_point();

                bool gs(true);
                while (gs) {
                    licrd.generateRandomCoords(tcart,temp,rgenerator);

                    if (m_checkRandomStructure(tcart)) {
                        ++gdf;
                    } else {
                        gs = false;
                    }
                }

                mrtimer.end_point();

                /*------Gaussian 09 Running-------

                ---------------------------------*/
                mgtimer.start_point();

                // Build the g09 input file for the high level of theory
                g09::buildCartesianInputg09(ngpr,input,HOT,"SCF="+SCF,itype,tcart,multip,charge,1);

                // Execute the g09 run, if failure occures we restart the loop
                g09::execg09(ngpr,input,outshl,chkoutshl);

                mgtimer.end_point();

                /*----Creating CSV Datapoint------

                ---------------------------------*/
                mstimer.start_point();
                // Append the data to the datapoint string
                for (unsigned j=0; j<ngpr; ++j) {
                    //if (!chkoutshl[j] && !chkoutsll[j]) {
                    //std::cout << "|***************************************|" << std::endl;
                    if (!chkoutshl[j]) {

                        datapoint.append( simtls::xyzToCSV(tcart) );
                        std::string energy( g09::energyFinder(outshl[j]) );
                        datapoint.append( energy );

                        #pragma omp critical
                        {
                            energies.push_back( atof(energy.c_str()) );
                        }

                        // Save the data point to the threads private output file output
                        tsoutt << datapoint << std::endl;
                        datapoint.clear();
                        ++i;
                    } else {
                        #pragma omp master
                        {
                            std::cout << input << std::endl;
                        }
                        ++gcf;
                    }
                    //std::cout << "COMPLETE" << std::endl;
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
            convfail += gcf;

            std::cout << "\n|----Thread " << tid << " info----|" << std::endl;
            mttimer.print_generic_to_cout(std::string("Total"));
            mrtimer.print_generic_to_cout(std::string("Struc. Gen."));
            mgtimer.print_generic_to_cout(std::string("Gau. 09."));
            mstimer.print_generic_to_cout(std::string("CSV Gen."));
            std::cout << "Number of gaussian convergence failures: " << gcf << std::endl;
            std::cout << "Number of geometry distance check failures: " << gdf << std::endl;
            std::cout << "|---------------------|\n" << std::endl;
        }
    }

    std::cout << std::endl;

    if (!energies.empty()) {
        double mini (std::min( *energies.begin(),*energies.end() ));
        double maxi (*std::max_element( energies.begin(),energies.end() ));

        std::cout << "\nEnergy Data: " << std::setprecision(16) << " MIN: " << mini << " " << maxi << " dE: " << abs( maxi - mini ) << std::endl << std::endl;
    }

    std::cout << "\nTotal Convergence Fails: " << convfail << std::endl << std::endl;

    // Combine all threads output
    MicroTimer fttimer;
    std::ofstream tsout;
    std::string dfname(iptData->getParameter<std::string>("dfname"));
    tsout.open(dfname.c_str(),std::ios_base::binary);

    tsout << params.getParameter<std::string>("LOT") << std::endl;
    tsout << params.getParameter<std::string>("TSS") << std::endl;
    tsout << atoms << ","<< typescsv << std::endl;

    fttimer.start_point();
    std::vector<std::stringstream>::iterator nameit;
    for (nameit = outname.begin(); nameit != outname.end(); nameit++) {
        // Move files individualy into the main output
        std::ifstream infile((*nameit).str().c_str(),std::ios_base::binary);
        std::cout << "Transferring file " << (*nameit).str() << " -> " << dfname << std::endl;
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

/*------Check a Random Structure--------

----------------------------------------*/
bool TrainingsetNormModebuilder::m_checkRandomStructure(const std::vector<glm::vec3> &xyz) {
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
