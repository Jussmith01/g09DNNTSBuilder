#ifndef READINPUT_H
#define READINPUT_H

#include <string>
#include <regex>

// GLM Mathematics Library
#include "../../include/glm/detail/type_vec.hpp"
#include <glm/glm.hpp>

// Atom Masses
#include "atom_masses.h"

namespace ipt
{

    struct Params {
        int Na; // Number of atoms
        int tts;    // Training set size to obtain
        int nrpg;   // Number of runs per gaussian
        float std;  // Standard deviation of random coordinates
        float mean;

        std::string llt; // Low Level of Theory
        std::string hlt; // High Level of Theory

        void printdata()
        {
            std::cout << "Input Parameters" << std::endl;
            std::cout << "Number of Atoms: " << Na << std::endl;
            std::cout << "Training Set size: " << tts << std::endl;
            std::cout << "STd. Dev. of random numbers: " << std << std::endl;
            std::cout << "Low Level of Theory: " << llt << std::endl;
            std::cout << "High Level of Theory: " << hlt << std::endl;
            std::cout << "Number of Runs Gaussians Per Inputs: " << nrpg << std::endl;
            std::cout << std::endl;
        };
    };
};

class input {
    /*----------------------------------------
      Data Container for the Program
    ------------------------------------------*/
    Params params;

    std::vector<glm::vec3> xyz; // xyz coords of atoms
    std::vector<std::string> types; //  Atom types
    std::vector<glm::ivec2> bonds; // bonding index
    std::vector<double> _m; // masses of the atoms


    std::string fname; // Input filename
    std::string oname; // Output filename

    // Read the input file
    void readinput();

public:
    /*----------------------------------------
          Function to Read the Input File
    ------------------------------------------*/

    // Constructor
    input(const std::string &input, const std::string &output) :
        fname(input), oname(output) {
        try {
            readinput();
        } catch (std::string error) dnntsErrorcatch(error);
    }

    ~input() {
        xyz.clear();
        bonds.clear();
    }

    // Member access functions

    //Access xyz
    const std::vector<glm::vec3>& getxyz() {
        return xyz;
    }

    const std::vector<std::string>& gettypes() {
        return types;
    }

    const std::vector<glm::ivec2>& getbonds() {
        return bonds;
    }

    const std::vector<double>& getmasses() {
        return _m;
    }

    const std::string& getoname() {
        return oname;
    }

    const Params& getparams() {
        return params;
    }
};

};
#endif
