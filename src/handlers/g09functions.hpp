//
// Created by fury on 8/4/15.
//

#ifndef G09FUNCTIONS_H
#define G09FUNCTIONS_H

namespace g09 {

/*----------------------------------------

 Get Forces from a Gaussian Output String

------------------------------------------*/
inline void forceFinder(const std::string &filename,std::vector<glm::vec3> &tfrce) {
    using namespace std;

    ///stringstream force_csv;
    ///force_csv.setf( std::ios::scientific, std::ios::floatfield );

    regex pattern_force("Hartrees/Bohr");
    regex pattern_cart("Cartesian");
    regex pattern_value("\\-?[[:d:]]+\\.[[:d:]]+");
    string line;
    istringstream stream(filename);
    while (getline(stream, line)) {
        if (regex_search(line, pattern_force)) {
            string line2;
            int ln(-2);
            while (getline(stream, line2) && !regex_search(line2, pattern_cart)) {
                sregex_iterator pos(line2.begin(), line2.end(), pattern_value);
                sregex_iterator end;
                int ax(0);
                for (; pos != end; ++pos) {
                    tfrce[ln][ax] = atof(pos->str(0).c_str());
                    //std::cout << tfrce[ln][ax] << ",";
                    ++ax;
                }
                //std::cout << ln << std::endl;
                ++ln;
            }
        }
    }

    ///return force_csv.str();
};

/*----------------------------------------

 Get input coordinates from a Gaussian
 Output String

------------------------------------------*/
inline void ipcoordinateFinder(const std::string &filename,std::vector<glm::vec3> &tcart) {
    using namespace std;

    stringstream cart_csv;
    //force_csv.setf( std::ios::scientific, std::ios::floatfield );

    regex pattern_force("Input orientation:");
    regex pattern_cart("Distance matrix");
    regex pattern_value("\\-?[[:d:]]+\\.[[:d:]]+");
    string line;
    istringstream stream(filename);
    while (getline(stream, line)) {
        if (regex_search(line, pattern_force)) {
            string line2;
            int ln(-4);
            while (getline(stream, line2) && !regex_search(line2, pattern_cart)) {
                sregex_iterator pos(line2.begin(), line2.end(), pattern_value);
                sregex_iterator end;
                int ax(0);
                for (; pos != end; ++pos) {
                    tcart[ln][ax] = atof(pos->str(0).c_str());
                    //std::cout << tcart[ln][ax] << ",";
                    ++ax;
                }
                //std::cout << ln << std::endl;
                ++ln;
                //std::cout << std::endl;
            }
        }
    }
};

/*----------------------------------------
  Get G09 Internal Coordinate Forces

------------------------------------------*/
inline std::string icforceFinder(const std::string &filename) {
    using namespace std;

    stringstream force_csv;
    force_csv.setf( std::ios::scientific, std::ios::floatfield );

    regex pattern_force("Internal");
    regex pattern_cart("Internal");
    regex pattern_value("\\-?[[:d:]]+\\.[[:d:]]+");
    string line;
    istringstream stream(filename);
    while (getline(stream, line)) {
        if (regex_search(line, pattern_force)) {
            string line2;
            while (getline(stream, line2) && !regex_search(line2, pattern_cart)) {
                //std::cout << line2 << std::endl;
                sregex_iterator pos(line2.begin(), line2.end(), pattern_value);
                sregex_iterator end;
                for (; pos != end; ++pos) {
                    force_csv << std::setprecision(7) << atof(pos->str(0).c_str()) << ",";
                }
            }
        }
    }

    //std::cout << " FORCE: " << force_csv.str() << std::endl;

    return force_csv.str();
};

/*----------------------------------------
  Get G09 Internal Coordinate Forces

------------------------------------------*/
inline std::string energyFinder(const std::string &filename) {
    using namespace std;

    string energy;
    //energy.setf( std::ios::scientific, std::ios::floatfield );

    regex patt_energy("\\sSCF Done:.*=\\s+(-*\\d+\\.\\d+)\\s");
    string line;
    istringstream stream(filename);
    while (getline(stream, line)) {
        std::smatch m;
        if (regex_search(line, m, patt_energy)) {
            energy = m.str(1) + ",";
        }
    }

    //std::cout << " FORCE: " << force_csv.str() << std::endl;

    return energy;
};

/*----------------------------------------
        Parse a Multi Gaussian Run
Parse many output into individual outputs.
------------------------------------------*/
inline void parseg09OutputLinks(int nrpg,std::string &multioutput,std::vector<std::string> &indoutputs) {
    size_t lpos = multioutput.find("Entering Link 1");
    multioutput = multioutput.substr(lpos+1);

    for (int i=0; i<nrpg; ++i) {
        lpos = multioutput.find("Entering Link 1");
        indoutputs[i] = multioutput.substr(0,lpos);
        multioutput=multioutput.substr(lpos+1);
    }
};

/*----------------------------------------
  Execute Gaussian Command on the System

  Function returns true if cerr returns
  a segmentation violation. This occurs
  most notably when the SCF fails to
  converge.
------------------------------------------*/
inline void execg09(int nrpg,const std::string &input,std::vector<std::string> &out,std::vector<bool> &chkout) {
    // Build bash command for launching g09
    std::stringstream sscmd;
    sscmd << "#!/bin/sh\ng09 <<END 2>&1 " << input.c_str() << "END\n"; // Redirect cerr to cout

    // Open a pipe and run g09 command -- output saved in string 'out'.
    std::string mout(systls::exec(sscmd.str().c_str(),10000));

    parseg09OutputLinks(nrpg,mout,out);

    for (int j=0; j<nrpg; ++j) {
        // If normal termination is detected the the program returns false.
        if (out[j].find("Normal termination of Gaussian 09")!=std::string::npos) {
            chkout[j]=false;
        }

        // Check if gaussian failed to converge - return true if it fails.
        else if (out[j].find("Convergence failure -- run terminated")!=std::string::npos) {
            //std::cout << "CVF!" << std::endl;
            chkout[j]=true;
        }

        // Check if interatomic distances were too close  - return true if it fails.
        else if (out[j].find("Small interatomic distances encountered")!=std::string::npos) {
            //std::cout << "SIADF!" << std::endl;
            chkout[j]=true;
        }

        // Check if g09 was found, if not the
        else if (out[j].find("g09: not found")!=std::string::npos) {
            throwException("Gaussian 09 program not found; make sure it is exported to PATH.");
        }

        else {
            // If the function has not returned yet then something is wrong
            std::ofstream gaue("gauerror.log");
            gaue << out[j] << std::endl;
            gaue.close();
            chkout[j]=true;
            //throwException("Unrecognized Gaussian 09 Failure; saving output as gauerror.log");
        }
    }
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
inline void buildCartesianInputg09(int nrpg,std::string &input,std::string lot,std::string additional,const std::vector<std::string> &type,const std::vector<glm::vec3> &xyz,int mult,int charge,int nproc) {
    // Number of coords per molecule
    int N = xyz.size()/nrpg;

    // Error check
    if (type.size()!=static_cast<unsigned int>(N))
        throwException("type and xyz are not the same size.");

    input="";

    for (int j=0; j<nrpg; ++j) {
        // Build gaussian 09 input
        std::stringstream tmpipt;
        tmpipt.setf( std::ios::scientific, std::ios::floatfield );
        tmpipt << "\n%nproc=" << nproc << "\n";
        tmpipt << "#p " << lot << " " << additional << "\n\n";
        tmpipt << "COMMENT LINE\n\n";
        tmpipt << charge << "  " << mult << "\n";

        for (uint32_t i = 0; i<type.size(); ++i)
            tmpipt << type[i] << std::setprecision(7) << " " << xyz[j*N+i].x << " " << xyz[j*N+i].y << " " << xyz[j*N+i].z << "\n";

        tmpipt << "\n";

        if (j < nrpg-1) {
            tmpipt << "--link1--\n";
        };

        // Return input string
        input.append(tmpipt.str());
        //std::cout << "---------------------------" << std::endl;
        //std::cout << input << std::endl;
        //std::cout << "---------------------------" << std::endl;
    }
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
inline void buildZmatInputg09(int nrpg,std::string &input,std::string lot,std::string additional,const std::vector< std::string > &zmat,int mult,int charge,int nproc) {

    input="";

    for (int j=0; j<nrpg; ++j) {
        // Build gaussian 09 input
        std::stringstream tmpipt;
        tmpipt.setf( std::ios::scientific, std::ios::floatfield );
        tmpipt << "\n%nproc=" << nproc << "\n";
        tmpipt << "\n%mem=" << 500 << "MB\n";
        tmpipt << "#p " << lot << " " << additional << "\n\n";
        tmpipt << "COMMENT LINE\n\n";
        tmpipt <<  charge << "  " << mult << "\n";

        tmpipt << zmat[j] << "\n";

        tmpipt << "\n";

        if (j < nrpg-1) {
            tmpipt << "--link1--\n";
        };

        // Return input string
        input.append(tmpipt.str());
    }
};

};
#endif //G09DNNTSBUILDER_READERS_H
