//
// Created by Justin Smith and Dustin Tracy on 7/5/15.
//

#ifndef SYSTEM_TOOLS_HPP
#define SYSTEM_TOOLS_HPP

/*   The System Tools Namespace   */
namespace systls {

/*----------------------------------------
    Execute Command on the System

    Function opens a pipe and runs the
    command while storing any cout data
    in a buffer. This buffer is then
    returned.
------------------------------------------*/
inline std::string exec(const std::string &cmd,size_t maxbuf) {

    // Open a pipe and run command
    FILE* pipe = popen(cmd.c_str(), "r");

    // Throw an exception if pipe not open
    if (!pipe) throwException("Unable to open pipe.");

    // Extract buffer data and return it.
    char buffer[maxbuf];
    std::string result = "";
    while(!feof(pipe)) {
        if(fgets(buffer, maxbuf, pipe) != NULL)
            result += buffer;
    }
    pclose(pipe);

    return result;
};

/*----------------------------------------
  Execute Gaussian Command on the System

  Function returns true if cerr returns
  a segmentation violation. This occurs
  most notably when the SCF fails to
  converge.
------------------------------------------*/
inline bool execg09(const std::string &input) {
    // Build bash command for launching g09
    std::stringstream sscmd;
    sscmd << "#!/bin/sh\ng09 <<END 2>&1 " << input.c_str() << "END\n"; // Redirect cerr to cout

    // Open a pipe and run g09 command -- output saved in string 'out'.
    std::string out(exec(sscmd.str().c_str(),1000));

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

    return true;
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
inline std::string buildInputg09(std::string lot,std::string additional,const std::vector<std::string> &type,const std::vector<glm::vec3> &xyz,int mult,int charge,int nproc) {
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

    for (uint32_t i = 0;i<type.size();++i)
        tmpipt << type[i] << std::setprecision(5) << " " << xyz[i].x << " " << xyz[i].y << " " << xyz[i].z << "\n";

    tmpipt << "\n";

    // Return input string
    return tmpipt.str();
};

};

#endif
