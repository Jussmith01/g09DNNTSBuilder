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
#include "tsbuilder.h"

/*      Optimizer

    Returns true if opt succeeded

*/
bool Trainingsetbuilder::optimizer(std::string LOT
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

void Trainingsetbuilder::optimizeStoredStructure() {
    using namespace std;

    // Store working parameters
    ipt::inputParameters params(*iptData);

    vector<  glm::vec3 > xyz  (rcrd.getixyz());
    vector<string> itype(rcrd.getitype());
    string HOT(params.getParameter<string>("LOT"));
    stringstream _args;

    // Some local variables
    int charge (params.getParameter<int>("charge"));
    int multip (params.getParameter<int>("multip"));

    _args.clear();
    _args << "opt";
    if ( !optimizer(HOT,_args.str(),itype,xyz,charge,multip) ) {

        string iguess("Huckel");

        //-----------------------------
        // Low level minimization
        //-----------------------------
        _args.clear();
        _args << "opt(cartesian,MaxStep=100,MaxCycles=1000) Guess(" << iguess << ")";
        optimizer("AM1",_args.str(),itype,xyz,charge,multip);

        //-----------------------------
        // Medium level minimization
        //-----------------------------
        _args.clear();
        _args << "opt(cartesian,MaxStep=100,MaxCycles=1000) Guess(" << iguess << ")";
        optimizer("HF/6-31g*",_args.str(),itype,xyz,charge,multip);

        //-----------------------------
        // Medium-High level minimization
        //-----------------------------
        _args.clear();
        _args << "opt(cartesian,MaxStep=100,MaxCycles=1000) Guess(" << iguess << ")";
        optimizer("MP2/6-31g*",_args.str(),itype,xyz,charge,multip);

        //-----------------------------
        // High level minimization
        //-----------------------------


        bool mini(false);
        unsigned cnt(0);
        string conv("");
        while (!mini) {

            _args.clear();
            _args << "opt(cartesian" << conv << ",MaxStep=500,MaxCycles=1000) Guess(Huckel)";

            if ( !optimizer(HOT,_args.str(),itype,xyz,charge,multip) ) {
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

    };

    // Print Results
    cout << "------------------------------------" << endl;
    cout << " Optimized Coordinates:\n" << endl;
    cout.setf( ios::fixed, ios::floatfield );
    for (unsigned i=0; i<xyz.size(); ++i) {
        cout << " " << itype[i] << setprecision(7) << " " << setw(10) << xyz[i].x << " " << setw(10) << xyz[i].y << " " << setw(10) << xyz[i].z << endl;
    }

    rcrd.setixyz(xyz);
    cout << "------------------------------------" << endl;
    cout << "Optimization Complete.\n" << endl;
    iptData->storeInputWithOptCoords(xyz,true);
    cout << "Updated coordinates saved to the input file.\n" << endl;
}

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
    std::cout << "\033["<< tid+1 <<"A\033[K\033[1;30mThread " << tid << " is " << static_cast<int>(round((i/float(N))*100.0)) << "% complete. G09 Convergence Fails " << gcfail << " Distance Fails: " << gdfail << "\033["<< tid+1 <<"B\033[100D\033[0m";
};

/*--------Calculate Validation Set---------


----------------------------------------*/
void Trainingsetbuilder::calculateRandomValidationSet() {

    iptData->setParameter("dfname",iptData->getParameter<std::string>("vdfname") );
    iptData->setParameter("TSS",iptData->getParameter<std::string>("VSS") );

    calculateRandomTrainingSet();
}

/*--------Calculate Training Set---------

This function contians the main loop for
building the training set.
----------------------------------------*/
void Trainingsetbuilder::calculateRandomTrainingSet() {
    using namespace std;
    cout << "------------------------------" << endl;
    cout << "  Begin building training set " << endl;
    cout << "------------------------------\n" << endl;

    ///std::cout << "CHECK: " << simtls::countUnique(10,4) << std::endl;

    // Store working parameters
    ipt::inputParameters params(*iptData);
    //params.printdata();

    // Local pointer to icrd for passing a private class to threads
    //rcrd.printdata();

    string typescsv( simtls::stringsToCSV(rcrd.getotype()) );
    unsigned    atoms   ( rcrd.getNa() );

    cout << "atoms: " << atoms << endl;

    // Get the maximum number of threads
    int MaxT = omp_get_max_threads();
    // Setup loop output function
    void (*loopPrinter)(int tid,int N,int i,int gcfail,int gdfail);
    switch ((int)routecout) {
    case 0: {
        cout << "Output setup for terminal writing." << endl;
        loopPrinter = &print_for_cout;
        for (int i=0; i<MaxT; ++i) {
            cout << endl;
        }
        break;
    }
    case 1: {
        cout << "Output setup for file writing." << endl;
        loopPrinter = &print_for_file;
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

    /* Setup thread seeds */
    unsigned seed1 ( std::chrono::system_clock::now().time_since_epoch().count() );
    unsigned seed2 ( clock() );

    std::seed_seq seedgen = {seed1,seed2};
    std::vector<unsigned> sequence (MaxT);

    seedgen.generate(sequence.begin(),sequence.end());

    for (unsigned i = 0; i < sequence.size(); ++i) {
        cout << "Random Seed for thread (" << i << "): " << sequence[i] << endl;
    }

    // This is the termination string. If an error is caught it
    // saves it here and the threads then exit.
    string termstr("");

    // Begin parallel region
    #pragma omp parallel default(shared) firstprivate(params,MaxT)
    {
        // Thread safe copy of the internal coords calculator
        itrnl::RandomCartesian licrd = rcrd;

        // Thread ID
        unsigned tid = omp_get_thread_num();

        // Some local variables
        unsigned ngpr = params.getParameter<unsigned>("ngpr");
        unsigned tss = params.getParameter<unsigned>("TSS");

        // Minimimum number of sets for this thread to calculate
        int N = floor(tss/MaxT);
        if (tid < tss % MaxT) {
            ++N;
        }

        std::mt19937 rgenerator (sequence[tid]);

        // Prepare the random number seeds
        //std::vector<int> seedarray;
        //seedGen.getThreadSeeds(tid,seedarray);

        // Prepare the random number generator


        //RandomReal rnGen(seedarray,params.getParameter<std::string>("rdm"));

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

                //std::cout << "Generate Random Coords" << std::endl;
                licrd.generateRandomCoordsSpherical(tcart,rgenerator);
                //licrd.generateRandomCoordsBox(tcart,rnGen);
                //licrd.generateRandomCoordsDistmat(tcart,rnGen);

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
                //std::cout << "Store Data" << std::endl;
                for (unsigned j=0; j<ngpr; ++j) {
                    //if (!chkoutshl[j] && !chkoutsll[j]) {
                    //std::cout << "|***************************************|" << std::endl;
                    if (!chkoutshl[j]) {
                        //std::vector<glm::vec3> xyzind(ixyz.size());
                        //std::memcpy(&xyzind[0],&wxyz[j*ixyz.size()],ixyz.size()*sizeof(glm::vec3));

                        //datapoint.append(licrd.getCSVStringWithIC(icord[j]));

                        //datapoint.append(licrd.calculateCSVInternalCoordinates(xyzind));
                        //datapoint.append(g09::ipcoordinateFinder(outsll[j],tcart));
                        //datapoint.append(g09::ipcoordinateFinder(outsll[j],tcart));

                        //g09::ipcoordinateFinder(outsll[j],tcart);
                        //g09::forceFinder(outsll[j],tfrce);
                        //datapoint.append(simtls::cartesianToStandardSpherical(0,1,2,tfrce,tcart));

                        //g09::ipcoordinateFinder(outshl[j],tcart);
                        //g09::forceFinder(outshl[j],tfrce);
                        //g09::ipcoordinateFinder(outshl[j],tcart);
                        //itrnl::iCoordToXYZ(icord[j],tcart);
                        //icord[j] = simtls::xyzToCSV(tcart);

                        if (m_checkRandomStructure(tcart)) {
                            ++gdf;
                        }
                        //datapoint.append( simtls::calculateDistMatrixCSV(tcart) );
                        //std::stringstream ss;
                        //ss << tcart.size();

                        //datapoint.append( ss.str().c_str() );
                        //datapoint.append( "," );
                        //datapoint.append( typescsv );
                        datapoint.append( simtls::xyzToCSV(tcart) );
                        //datapoint.append( itrnl::getCsvICoordStr(icord[j],"radians") );
                        //datapoint.append( simtls::cartesianToStandardSpherical(0,1,2,tfrce,tcart) );
                        datapoint.append( g09::energyFinder(outshl[j]) );
                        //datapoint.append(g09::forceFinder(outsll[j]));
                        //datapoint.append(g09::forceFinder(outshl[j]));

                        // Save the data point to the threads private output file output
                        tsoutt << datapoint << std::endl;
                        //std::cout << "DATAPOINT(" << i << "," << j << ")" << std::endl;
                        datapoint.clear();
                        ++i;
                    } else {
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

/*--------Calculate Validation Set---------


----------------------------------------*/
void Trainingsetbuilder::calculateMDValidationSet() {

    iptData->setParameter("dfname",iptData->getParameter<std::string>("vdfname") );
    iptData->setParameter("TSS",iptData->getParameter<std::string>("VSS") );

    calculateMDTrainingSet();
}

/*--------Calculate Training Set---------

This function contians the main loop for
building the training set.
----------------------------------------*/
void Trainingsetbuilder::calculateMDTrainingSet() {
    using namespace std;
    cout << "------------------------------" << endl;
    cout << "  Begin building training set " << endl;
    cout << "------------------------------\n" << endl;

    ///std::cout << "CHECK: " << simtls::countUnique(10,4) << std::endl;

    // Store working parameters
    ipt::inputParameters params(*iptData);

    string typescsv( simtls::stringsToCSV(rcrd.getotype()) );
    unsigned    atoms   ( rcrd.getNa() );

    cout << "atoms: " << atoms << endl;

    // Get the maximum number of threads
    int MaxT = omp_get_max_threads();

    std::vector<double>    allenergy;
    std::vector<glm::vec3> allcoords;

    // This is passed to each thread and contain unique seeds for each
    ParallelSeedGenerator seedGen(MaxT);

    vector<unsigned> counts(MaxT,0);

    // Begin parallel region
    #pragma omp parallel default(shared) firstprivate(params,MaxT,atoms)
    {
        // Thread safe copy of the internal coords calculator
        itrnl::RandomCartesian licrd = rcrd;

        // Thread ID
        unsigned tid = omp_get_thread_num();

        // Some local variables
        unsigned ngpr = params.getParameter<unsigned>("ngpr");
        unsigned tss = params.getParameter<unsigned>("TSS");

        // Minimimum number of sets for this thread to calculate
        int N = floor(tss/MaxT);
        if (tid < tss % MaxT) {
            ++N;
        }

        unsigned Nreq (N);

        // Prepare the random number seeds
        std::vector<int> seedarray;
        seedGen.getThreadSeeds(tid,seedarray);

        // Prepare the random number generator
        RandomReal rnGen(seedarray,params.getParameter<std::string>("rdm"));

        // Allocate space for new coordinates
        unsigned na = licrd.getNa();//params.getCoordinatesStr().size();
        std::vector<glm::vec3> wxyz(na*ngpr);

        // Initialize counters
        //int i(0); // Loop counter
        //int gcf(0); // Gaussian Convergence Fail counter
        //int gdf(0); // Geometry distance failure

        int charge (params.getParameter<int>("charge"));
        int multip (params.getParameter<int>("multip"));
        int stsize (params.getParameter<int>("stsize"));
        int Ntraj (params.getParameter<int>("Ntraj"));
        int MaxKE (params.getParameter<int>("MaxKE"));

        // Initialize some containers
        std::string datapoint;
        std::string input;
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
        vector< glm::vec3 > tcart(na);
        vector< glm::vec3 > tfrce(na);

        // Setup random int generator for KE
        default_random_engine generator;
        generator.seed(seedarray[0]);
        uniform_int_distribution<int> randomKE(50000,MaxKE);
        uniform_int_distribution<int> seedgen(1000000000,0);

        // Setup random seed generator
        seed_seq seed = {seedarray[0],seedarray[1],seedarray[3]};

        // Data Store
        stringstream _dfnamet;
        _dfnamet << iptData->getParameter<std::string>("dfname") << "_thread" << tid;
        ofstream tdout;
        tdout.open(_dfnamet.str().c_str(),std::ios_base::binary);

        // Traj Visual Store
        stringstream _trname;
        _trname << iptData->getParameter<std::string>("dfname") << "_trajdata" << tid;
        ofstream trout;
        trout.open(_trname.str().c_str(),std::ios_base::binary);

        // Terminator
        bool terminate(false);
        unsigned Nrun(0);
        unsigned counter(0);

        // Thread timers
        MicroTimer mttimer; // Time the whole loop
        mttimer.start_point();

        while (!terminate) {

            unsigned steps;
            if (Nreq > (unsigned)Ntraj)
                steps = Ntraj;
            else
                steps = Nreq;

            stringstream _add;
            _add << "SCF=" << SCF << " ADMP(MaxPoints=" << steps+1 << ",StepSize=" << stsize << ",Seed=" << seedgen(generator) << ",NKE=" << randomKE(generator) << ",FullSCF)";

            std::cout << "THIS CLASS IS CURRENTLY NOT WORKING!!!!" << std::endl;
            //licrd.generateRandomCoordsSpherical(tcart,rnGen);

            // Begin MD calculation
            ++Nrun;
            g09::buildCartesianInputg09(ngpr,input,HOT,_add.str(),itype,tcart,multip,charge,1);
            g09::execg09(ngpr,input,outshl,chkoutshl);

            // Containers
            std::vector<double>   energy;
            std::vector<glm::vec3> coords;

            energy.reserve(N);
            coords.reserve(N*atoms);

            // Get data from output
            g09::admpcrdenergyFinder(outshl[0],coords,energy);

            /*stringstream outname;
            outname << "output-" << _dfnamet.str() << "-traj" << Nrun;
            ofstream ofile(outname.str().c_str());
            ofile << outshl[0];
            ofile.close();*/

            if (chkoutshl[0]) {
                if (!energy.empty())
                    energy.pop_back();

                if (!energy.empty())
                    energy.pop_back();
            }

            // Save data in file
            double bohr(0.529177249);
            for (unsigned i = 1; i < energy.size(); ++i) {
                // Save Datapoint information
                std::stringstream _datap;
                _datap.setf(ios::scientific,ios::floatfield);
                for (unsigned j = 0; j < atoms; ++j) {
                    _datap << setprecision(8) << bohr*coords[j + i * atoms].x << ","
                           << bohr*coords[j + i * atoms].y << ","
                           << bohr*coords[j + i * atoms].z << ",";
                }
                _datap << setprecision(11) << energy[i] << ",";
                tdout << _datap.str() << endl;

                // Save Trajector visualtion data
                std::stringstream _datat;
                _datat.setf(ios::scientific,ios::floatfield);
                _datat << counter << ",";
                _datat << tid << ",";
                _datat << i-1 << ",";
                _datat << setprecision(11) << energy[i] << ",";
                _datat << setprecision(11) << abs(energy[i]-energy[i-1]) << ",";
                trout << _datat.str() << endl;
                ++counter;
            }

            counts[tid] += energy.size()-1;

            if ( Nreq - (energy.size()-1) != 0 ) {
                Nreq = Nreq - (energy.size()-1);
            } else {
                terminate = true;
            }

            #pragma omp master
            {
                unsigned sum(0);
                for (auto& i : counts)
                    sum+=i;
                cout << "Calculated Points: " << sum << endl;
            }

        }

        mttimer.end_point();

        tdout.close();
        trout.close();

        // Wait for the whole team to finish
        // Print stats for each thread in the team
        #pragma omp critical
        {
            std::cout << "\n|----Thread " << tid << " info----|" << std::endl;
            mttimer.print_generic_to_cout(std::string("Total"));
            std::cout << "Total Gaussian Traj: " << Nrun << std::endl;
            std::cout << "|---------------------|\n" << std::endl;
        }

        #pragma omp barrier
    }

    unsigned sum(0);
    for (auto& i : counts)
        sum+=i;
    cout << "Calculated Points: " << sum << endl;

    std::cout << std::endl;

    // Combine all threads output
    std::ofstream tsout;
    std::string dfname(iptData->getParameter<std::string>("dfname"));
    tsout.open(dfname.c_str(),std::ios_base::binary);
    tsout << params.getParameter<std::string>("LOT") << std::endl;
    tsout << params.getParameter<std::string>("TSS") << std::endl;
    tsout << atoms << ","<< typescsv << std::endl;

    MicroTimer fttimer;
    fttimer.start_point();
    for (unsigned i = 0; i < (unsigned)MaxT; ++i) {
        stringstream _dfnamet;
        _dfnamet << iptData->getParameter<std::string>("dfname") << "_thread" << i;
        // Move files individualy into the main output
        std::ifstream infile(_dfnamet.str().c_str(),std::ios_base::binary);
        std::cout << "Transferring file " << _dfnamet.str() << " -> " << dfname << std::endl;
        tsout << infile.rdbuf();

        // Remove old output once moved
        std::stringstream rm;
        rm << "rm " << _dfnamet.str();
        systls::exec(rm.str(),100);
    }
    fttimer.end_point();
    fttimer.print_generic_to_cout(std::string("File transfer"));
    tsout.close();

    std::cout << "\n------------------------------" << std::endl;
    std::cout << "Finished building training set" << std::endl;
    std::cout << "------------------------------" << std::endl;
};

