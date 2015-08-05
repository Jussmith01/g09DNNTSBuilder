// Std Lib Includes
#include <string>
#include <regex>
#include <fstream>

// GLM Mathematics Library
#include <glm/glm.hpp>

// Error Handlers
#include "../errorhandlers.h"

// Class definition
#include "input.h"

/*----------Read the Input File------------


------------------------------------------*/
void ipt::input::readinput() {
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
    if (ifile.is_open()) {
        while (getline(ifile, line)) {
            //  Load the training set size
            smatch m;
            //  Find Training Set Size
            if (regex_search(line, pattern_tts)) {
                if (regex_search(line, m, pattern_integer)) {
                    tts = atoi(m.str(0).c_str());
                }
            }
            // Find Standard Deviation
            if (regex_search(line, pattern_std)) {
                if (regex_search(line, m, pattern_float)) {
                    std = atof(m.str(0).c_str());
                }
            }
            //  Find Coordinates
            if (regex_search(line, pattern_coords)) {
                while (getline(ifile, line) && !std::regex_search(line, pattern_end)) {
                    vector<float> coord_temp;
                    sregex_iterator pos(line.begin(), line.end(), pattern_float);
                    sregex_iterator end;
                    for (; pos != end; ++pos) {
                        coord_temp.push_back(atof(pos->str().c_str()));
                    }
                    xyz.push_back(glm::vec3(coord_temp[0], coord_temp[1], coord_temp[2]));
                }
                Na = static_cast<int>(xyz.size());
            }
            // Find low level of theory
            if (regex_search(line, m, pattern_lot)) {
                llt = m.str(1);
            }
            // Find high level of theory
            if (regex_search(line, m, pattern_hot)) {
                hlt = m.str(1);
            }
            //  Find the bond links
            if (regex_search(line, pattern_bonds)) {
                int bond_idex = 0;
                while (getline(ifile, line), !regex_search(line, pattern_end)) {
                    ++bond_idex;
                    if (bond_idex >= Na)
                    {
                        throwException("Too many bonds for number of atoms");
                    }
                    vector<int> bonds_temp;
                    sregex_iterator pos(line.begin(), line.end(), pattern_integer);
                    sregex_iterator end;
                    for (; pos != end; ++pos) {
                        bonds_temp.push_back(atoi(pos->str().c_str()));
                    }

                    bonds.push_back(glm::ivec2(bonds_temp[0], bonds_temp[1]));
                }
            }
        }
        ifile.close();
    } else throwException("Unable to open file");
};

