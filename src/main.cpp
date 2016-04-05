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

// Custom Libs
#include <internalcoordinate.h>

// Utilities
#include "utils/randnormflt.h"
#include "utils/simpletools.hpp"
#include "utils/systools.hpp"
#include "utils/micro_timer.h"
#include "utils/flaghandler.h"

// Handlers
#include "handlers/input.h"
#include "handlers/g09functions.hpp"

// Core
#include "core/tsbuilder.h"
#include "core/ssbuilder.h"

int main(int argc, char *argv[]) {
    //--------------------------------
    // Check input arguments via macro
    //--------------------------------
    FlagHandler args(argc,argv);

    // Setup timer
    MicroTimer mtimer;

    //--------------------------------
    //          Read input
    //--------------------------------
    ipt::inputParameters iptdata(args.getflag("-i"),args.getflag("-d"));

    //--------------------------------
    //        Main Loop Class
    //--------------------------------
    /** Random Training Set Building **/
    if (iptdata.getParameter<std::string>("type").compare("random")==0) {

        unsigned threads ( iptdata.getParameter<unsigned>("threads") );
        if (threads != 0) {
            omp_set_num_threads( threads );
        }

        // Construct/prepare the class
        mtimer.start_point();
        Trainingsetbuilder tsb(&args,&iptdata);
        mtimer.end_point();
        mtimer.print_generic_to_cout(std::string("Training set builder class preparation time --\n"));

        if (iptdata.getParameter<unsigned>("optimize") == 1) {
            tsb.optimizeStoredStructure();
        } else {
            std::cout << "Optimization Turned Off." << std::endl;

            // Calculate the training set
            mtimer.start_point();
            tsb.calculateRandomTrainingSet();
            mtimer.end_point();
            mtimer.print_generic_to_cout(std::string("Calculate training set time --\n"));

            // Calculate the validation set
            mtimer.start_point();
            tsb.calculateRandomValidationSet();
            mtimer.end_point();
            mtimer.print_generic_to_cout(std::string("Calculate validation set time --\n"));
        }
    } else if (iptdata.getParameter<std::string>("type").compare("moldyn")==0) {

        unsigned threads ( iptdata.getParameter<unsigned>("threads") );
        if (threads != 0) {
            omp_set_num_threads( threads );
        }

        // Construct/prepare the class
        mtimer.start_point();
        Trainingsetbuilder tsb(&args,&iptdata);
        mtimer.end_point();
        mtimer.print_generic_to_cout(std::string("Training set builder class preparation time --\n"));

        if (iptdata.getParameter<unsigned>("optimize") == 1) {
            tsb.optimizeStoredStructure();
        } else {
            std::cout << "Optimization Turned Off." << std::endl;

            // Calculate the training set
            mtimer.start_point();
            tsb.calculateMDTrainingSet();
            mtimer.end_point();
            mtimer.print_generic_to_cout(std::string("Calculate training set time --\n"));

            // Calculate the validation set
            mtimer.start_point();
            tsb.calculateMDValidationSet();
            mtimer.end_point();
            mtimer.print_generic_to_cout(std::string("Calculate validation set time --\n"));
        }
    /** Scan Testing Set Building **/
    } else if (iptdata.getParameter<std::string>("type").compare("scan")==0) {
        unsigned threads ( iptdata.getParameter<unsigned>("threads") );
        if (threads != 0) {
            omp_set_num_threads( threads );
        }
        // Construct/prepare the class
        mtimer.start_point();
        Scansetbuilder ssb(&args,&iptdata);
        mtimer.end_point();
        mtimer.print_generic_to_cout(std::string("Scan set builder class preparation time --\n"));

        // Calculate the training set
        mtimer.start_point();
        ssb.calculateScanSet();
        mtimer.end_point();
        mtimer.print_generic_to_cout(std::string("Calculate scan set time --\n"));
    } else {
        dnntsErrorcatch(std::string("ERROR: type parameter does not match any known instances."));
    }


    return 0;
};
