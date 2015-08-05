#ifndef READINPUT_H
#define READINPUT_H

#include <string>
#include <regex>
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
        int tts;    // Training set size
        float std;  // Standard deviation of random coordinates

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
            //  Initiate search parameters
            std::regex pattern_tts("TTS", std::regex_constants::icase);
            std::regex pattern_std("STD", std::regex_constants::icase);
            std::regex pattern_coords("coordinates", std::regex_constants::icase);
            std::regex pattern_end("end", std::regex_constants::icase);
            std::regex pattern_atom("^[A-Z[:d:]][[:s:]]");
            std::regex pattern_integer("\\-?[[:d:]]+");
            std::regex pattern_float("\\-?[[:d:]]*\\.[[:d:]]+");

            //  Search file
            std::string line;
            std::ifstream ifile(fname.c_str());
            if (ifile.is_open())
            {
                while (getline(ifile, line))
                {
                    //  Load the training set size
                    std::smatch m;
                    if (std::regex_search(line, pattern_tts))
                    {
                        if (std::regex_search(line, m, pattern_integer))
                        {
                            tts = atoi(m.str(0).c_str());
                        }
                    }
                    if (std::regex_search(line, pattern_std))
                    {
                        if (std::regex_search(line, m, pattern_float))
                        {
                            std = atof(m.str(0).c_str());
                        }
                    }
                    if (std::regex_search(line, pattern_coords))
                    {
                        if(std::regex_search(line, pattern_float))
                        {
                            std::cout << "found" << std::endl;
                        }
                    }
                }
                ifile.close();
            } else
            throwException("Unable to open file");
        };
    };

};
#endif
