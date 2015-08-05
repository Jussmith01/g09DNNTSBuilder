//
// Created by fury on 8/4/15.
//

#ifndef G09FUNCTIONS_H
#define G09FUNCTIONS_H

#include "../utils/systools.hpp"

namespace g09 {

/*----------------------------------------
 Get Forces from a Gaussian Output String

------------------------------------------*/
std::string forceFinder(const std::string &filename)
{
    using namespace std;
    string force_csv;
    regex pattern_force("Hartrees/Bohr");
    regex pattern_cart("Cartesian");
    regex pattern_value("\\-?[[:d:]]+\\.[[:d:]]+");
    string line;
    /*while (getline(inFile, filename)) {
        if (regex_search(line, pattern_force)) {
            string line2;
            while (getline(inFile, line2) && !regex_search(line2, pattern_cart)) {
                sregex_iterator pos(line2.begin(), line2.end(), pattern_value);
                sregex_iterator end;
                for (; pos != end; ++pos) {
                    force_csv += pos->str(0) + ",";
                }
            }
        }
    }*/
    return force_csv;
};

/*----------------------------------------
  Execute Gaussian Command on the System

  Function returns true if cerr returns
  a segmentation violation. This occurs
  most notably when the SCF fails to
  converge.
------------------------------------------*/
inline bool execg09(const std::string &input)
{
    // Build bash command for launching g09
    std::stringstream sscmd;
    sscmd << "#!/bin/sh\ng09 <<END 2>&1 " << input.c_str() << "END\n"; // Redirect cerr to cout

    // Open a pipe and run g09 command -- output saved in string 'out'.
    std::string out(systls::exec(sscmd.str().c_str(),1000));

    // If normal termination is detected the the program returns false.
    if (out.find("Normal termination of Gaussian 09")!=std::string::npos) {
        return false;
    }

    // Check if gaussian failed to converge - return true if it fails.
    if (out.find("Convergence failure -- run terminated")!=std::string::npos) {
        std::cout << "CVF!" << std::endl;
        return true;
    }

    // Check if interatomic distances were too close  - return true if it fails.
    if (out.find("Small interatomic distances encountered")!=std::string::npos) {
        std::cout << "SIADF!" << std::endl;
        return true;
    }

    // Check if g09 was found, if not the
    if (out.find("g09: not found")!=std::string::npos) {
        throwException("Gaussian 09 program not found; make sure it is exported to PATH.");
    }

    // If the function has not returned yet then something is wrong
    std::ofstream gaue;
    gaue.open("gauerror.log");
    gaue << out << std::endl;
    gaue.close();

    throwException("Unrecognized Gaussian 09 Failure; saving output as gauerror.log");

    return true; // Does nothing but make the compiler happy
};

/*----------------------------------------
          Build G09 Input String
    lot = level of theory
    additional = more g09 parameters...
       i.e. forces, opt
    type = atom type .i.e. O or 8 for Oxygen
    xyz = coordinates for the atom
    mult = molecular multiplicity
    charge = molecular charge
    nproc = number of processors to use

    Note: type and xyz must be of same size
------------------------------------------*/
inline std::string buildInputg09(std::string lot,std::string additional,const std::vector<std::string> &type,const std::vector<glm::vec3> &xyz,int mult,int charge,int nproc)
{
    // Error check
    if (type.size()!=xyz.size())
        throwException("type and xyz are not the same size.");

    // Build gaussian 09 input
    std::stringstream tmpipt;
    tmpipt.setf( std::ios::fixed, std::ios::floatfield );
    tmpipt << "\n%nproc=" << nproc << "\n";
    tmpipt << "#p " << lot << " " << additional << "\n\n";
    tmpipt << "Something\n\n";
    tmpipt << mult << "  " << charge << "\n";

    for (uint32_t i = 0; i<type.size(); ++i)
        tmpipt << type[i] << std::setprecision(5) << " " << xyz[i].x << " " << xyz[i].y << " " << xyz[i].z << "\n";

    tmpipt << "\n";

    // Return input string
    return tmpipt.str();
};

};
#endif //G09DNNTSBUILDER_READERS_H