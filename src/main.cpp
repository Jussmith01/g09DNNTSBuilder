// Standary Library
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <iomanip>

// Error Handling
#include "errorhandlers.h" // Contains the error handlers.

// Utilities
#include "utils/randnormflt.h"
#include "utils/input.h"
#include "utils/simpletools.hpp"
#include "utils/systools.hpp"

int main (int argc, char *argv[])
{
    //--------------------------------
    // Check input arguments via macro
    //--------------------------------
    checkArgs(argv[1]);
    checkArgs(argv[2]);

    //::inputData.fname = argv[1];
    //ipt::inputData.oname = argv[2];

    //--------------------------------
    //          Read input
    //--------------------------------
    try {
        ipt::input testing(argv[1],argv[2]);
    } catch (std::string error) dnntsErrorcatch(error);

    //--------------------------------
    //     Generate Random Numbers
    //--------------------------------
    int Na = 3;
    int Nd = 10;
    long int Nr(3*Na*Nd); // Number of random numbers

    try {
        NormRandomReal randflt(Nr,clock());

        std::vector<float> rdnflts;
        randflt.fillVector(0.0,0.1,rdnflts,Nr);
    } catch (std::string error) dnntsErrorcatch(error);

    //--------------------------------
    //          Run G09 Jobs
    //--------------------------------
    std::vector<std::pair<unsigned int,unsigned int>> bonds;
    bonds.push_back(std::pair<unsigned int,unsigned int>(0,1));
    bonds.push_back(std::pair<unsigned int,unsigned int>(0,2));
    bonds.push_back(std::pair<unsigned int,unsigned int>(1,3));

    std::vector<std::string> type;
    type.push_back("O");
    type.push_back("H");
    type.push_back("H");

    std::vector<glm::vec3> xyz;
    xyz.push_back(glm::vec3(0.000,0.000,0.000));
    xyz.push_back(glm::vec3(0.750,0.000,0.520));
    xyz.push_back(glm::vec3(0.750,0.000,-0.52));


    try {
        std::string input(systls::buildInputg09("AM1","force",type,xyz,0,1,1));

        //std::string input = "\n#p AM1 force\n\nwater\n\n0  1\nO 0.0000 0.0000 0.0000\nH 0.7500 0.0000 0.5200\nH 0.7500 0.0000 -0.520\n\n";
        std::cout << "G09 ERROR: " << systls::execg09(input) << std::endl;
    } catch (std::string error) dnntsErrorcatch(error);

    return 0;
};
