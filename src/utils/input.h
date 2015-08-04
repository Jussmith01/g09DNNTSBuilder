#ifndef READINPUT_H
#define READINPUT_H

// GLM vector types and math
#include <glm/glm.hpp>

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

        std::string llt; // Low Level of Theory
        std::string hlt; // High Level of Theory

        std::vector<glm::vec3> xyz; // xyz coords of atoms

        std::string fname; // Input filename
        std::string oname; // Output filename

    public:
        /*----------------------------------------
              Function to Read the Input File
        ------------------------------------------*/
        input(const std::string &input, const std::string &output) :
                fname(input), oname(output)
        {
            readinput();
        }

        void readinput()
        {
            std::string line;
            std::ifstream ifile(fname.c_str());
            if (ifile.is_open())
            {
                while (getline(ifile, line))
                {
                    std::cout << line << '\n';
                }
                ifile.close();
            } else
            throwException("Unable to open file");
        };
    };

};
#endif
