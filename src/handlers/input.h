#ifndef READINPUT_H
#define READINPUT_H

#include <string>
#include <regex>

// GLM Mathematics Library
#include <glm/glm.hpp>
#include "../../include/glm/detail/type_vec.hpp"

namespace ipt
{

    class input
    {
        /*----------------------------------------
          Data Container for the Program
        ------------------------------------------*/
        input()
        { };
        int Nd; // Number of Data points to obtain
        int Na; // Number of atoms
        int tts;    // Training set size
        float std;  // Standard deviation of random coordinates

        std::string llt; // Low Level of Theory
        std::string hlt; // High Level of Theory

        std::vector<glm::vec3> xyz; // xyz coords of atoms
        std::vector<glm::ivec2> bonds;

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
                fname(input), oname(output)
        {
            readinput();
        }

        // Member access functions
        std::vector<glm::vec3>* getxyz()
        {
            return &xyz;
        }

    };

};
#endif
