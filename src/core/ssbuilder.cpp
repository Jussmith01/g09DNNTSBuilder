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
#include "ssbuilder.h"


/*--------Calculate Scan Set---------

This function contains the main loop for
building the scan set.
----------------------------------------*/
void Scansetbuilder::calculateScanSet() {
    std::cout << "------------------------------" << std::endl;
    std::cout << "    Begin building scan set   " << std::endl;
    std::cout << "------------------------------\n" << std::endl;

    // Get the maximum number of threads
    int MaxT = omp_get_max_threads();
    std::cout << "Using " << MaxT << " threads." << std::endl;

    // Prepare private thread output filenames
    std::string outname(iptData->getParameter<std::string>("dfname"));

    // Some local variables
    unsigned ngpr = iptData->getParameter<unsigned>("ngpr");

    std::string typescsv( simtls::stringsToCSV(scrd.getotype()) );

    // Allocate space for new coordinates
    unsigned na = iptData->getCoordinatesStr().size();
    std::vector<glm::vec3> wxyz(na*ngpr);

    // Initialize counters
    int i(0); // Loop counter
    int gcf(0); // Gaussian Convergence Fail counter
    int gdf(0); // Geometry distance failure

    // Initialize some containers
    std::string datapoint;
    std::string input;
    std::vector<std::string> outsll(ngpr);
    std::vector<bool> chkoutsll(ngpr);
    std::vector<std::string> outshl(ngpr);
    std::vector<bool> chkoutshl(ngpr);

    std::vector<std::string> itype(scrd.getitype());

    int charge (iptData->getParameter<int>("charge"));
    int multip (iptData->getParameter<int>("multip"));

    std::string HOT(iptData->getParameter<std::string>("LOT"));

    // Z-matrix Stuff
    std::vector<std::string> zmat(ngpr);
    std::vector<std::string> cord(ngpr);

    // Cartesian and force temp storage
    std::vector< glm::vec3 > tcart(na);
    std::vector< glm::vec3 > tfrce(na);

    // Define and open thread output
    std::ofstream tsoutt;
    tsoutt.open(outname.c_str());

    // Thread timers
    MicroTimer mttimer; // Time the whole loop
    MicroTimer mrtimer; // Time the random structure generation
    MicroTimer mgtimer; // Time the gaussian 09 runs
    MicroTimer mstimer; // Time the string generation

    // Begin main loop
    mttimer.start_point();

    bool complete(false);
    while (!complete) { // TERMINATES WHEN
        //std::cout << "Complete: " << complete << std::endl;
        try {
            //std::cout << i << std::endl;

            /*------Structure Generation------

            ---------------------------------*/
            // Generate the random structures
            mrtimer.start_point();
            //std::cout << "Generate Random Coords" << std::endl;
            complete = scrd.generateNextScanStructure(tcart);
            mrtimer.end_point();

            /*------Gaussian 09 Running-------

            ---------------------------------*/
            mgtimer.start_point();

            g09::buildCartesianInputg09(ngpr,input,HOT,"",itype,tcart,multip,charge,1);

            //std::cout << input << std::endl;

            g09::execg09(ngpr,input,outshl,chkoutshl);

            //std::cout << outshl[0] << std::endl;

            mgtimer.end_point();

            /*----Creating CSV Datapoint------

            ---------------------------------*/
            mstimer.start_point();

            // Append the data to the datapoint string
            for (unsigned j=0; j<ngpr; ++j) {
                if (!chkoutshl[j]) {

                    if (m_checkRandomStructure(tcart)) {
                        ++gdf;
                    }

                    std::stringstream ss;
                    ss << tcart.size();

                    datapoint.append( ss.str().c_str() );
                    datapoint.append( "," );
                    datapoint.append( typescsv );
                    datapoint.append( simtls::xyzToCSV(tcart) );
                    datapoint.append( g09::energyFinder(outshl[j]) );


                    // Save the data point to the threads private output file output
                    tsoutt << datapoint << std::endl;

                    datapoint.clear();
                    ++i;
                } else {
                    std::cout << "Gaussian Failure!" << std::endl;
                    ++gcf;
                }
            }
            mstimer.end_point();

            // Loop printer.


        } catch (std::string error) {
            dnntsErrorcatch(error);
        }
    }

    mttimer.end_point();

    // Close the threads output
    tsoutt.close();

    // Print stats for each thread in the team
    std::cout << "\n|----Scan Information----|" << std::endl;
    mttimer.print_generic_to_cout(std::string("Total"));
    mrtimer.print_generic_to_cout(std::string("Struc. Gen."));
    mgtimer.print_generic_to_cout(std::string("Gau. 09."));
    mstimer.print_generic_to_cout(std::string("CSV Gen."));
    std::cout << "Number of gaussian convergence failures: " << gcf << std::endl;
    std::cout << "Number of geometry distance check failures: " << gcf << std::endl;
    std::cout << "|---------------------|\n" << std::endl;

    std::cout << "\n------------------------------" << std::endl;
    std::cout << "  Finished building scan set  " << std::endl;
    std::cout << "------------------------------" << std::endl;
};

/*------Check a Random Structure--------

----------------------------------------*/
bool Scansetbuilder::m_checkRandomStructure(const std::vector<glm::vec3> &xyz) {
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

