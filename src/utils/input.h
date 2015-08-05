#ifndef READINPUT_H
#define READINPUT_H

#include <string>
#include <regex>
#include <glm.hpp>

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
        std::vector<int> bonds;

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
            using namespace std;
            //  Initiate search parameters
            regex pattern_tts("TTS", regex_constants::icase);
            regex pattern_lot("LOT[[:s:]]*=[[:s:]]*([[:w:]]+)", regex_constants::icase);
            regex pattern_hot("HOT[[:s:]]*=[[:s:]]*([[:w:]]+)", regex_constants::icase);
            regex pattern_std("STD", regex_constants::icase);
            regex pattern_coords("coordinates", regex_constants::icase);
            regex pattern_bonds("bonds", regex_constants::icase);
            regex pattern_end("end", regex_constants::icase);
            regex pattern_atom("^[A-Z[:d:]][[:s:]]");
            regex pattern_integer("\\-?[[:d:]]+");
            regex pattern_float("\\-?[[:d:]]+\\.[[:d:]]+");

            //  Search file
            std::string line;
            std::ifstream ifile(fname.c_str());
            if (ifile.is_open())
            {
                while (getline(ifile, line))
                {
                    //  Load the training set size
                    smatch m;
                    //  Find Training Set Size
                    if (regex_search(line, pattern_tts))
                    {
                        if (regex_search(line, m, pattern_integer))
                        {
                            tts = atoi(m.str(0).c_str());
                        }
                    }
                    // Find Standard Deviation
                    if (regex_search(line, pattern_std))
                    {
                        if (regex_search(line, m, pattern_float))
                        {
                            std = atof(m.str(0).c_str());
                        }
                    }
                    //  Find Coordinates
                    if (regex_search(line, pattern_coords))
                    {
                        while (getline(ifile, line) && !std::regex_search(line, pattern_end))
                        {
                            vector<float> coord_temp;
                            sregex_iterator pos(line.begin(), line.end(), pattern_float);
                            sregex_iterator end;
                            for (; pos != end; ++pos)
                            {
                                coord_temp.push_back(atof(pos->str().c_str()));
                            }
                            xyz.push_back(glm::vec3(coord_temp[0], coord_temp[1], coord_temp[2]));
                        }
                        Na = static_cast<int>(xyz.size());
                    }
                    // Find low level of theory
                    if (regex_search(line, m, pattern_lot))
                    {
                        llt = m.str(1);
                    }
                    // Find high level of theory
                    if (regex_search(line, m, pattern_hot))
                    {
                        hlt = m.str(1);
                    }
                    //  Find the bond links
                    if (regex_search(line, pattern_bonds))
                    {
                        while (getline(ifile, line), !regex_search(line, pattern_end))
                        {
                            sregex_iterator pos(line.begin(), line.end(), pattern_integer);
                            sregex_iterator end;
                            for (; pos !=end; ++pos)
                            {
                                bonds.push_back(atoi(pos->str().c_str()));
                            }
                        }
                        if (static_cast<int>(bonds.size()) >= Na)
                        {
                            throwException("Too many bonds! There should be less bonds than atoms.")
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
