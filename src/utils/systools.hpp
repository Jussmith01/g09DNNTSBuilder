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
    // Build command
    std::stringstream sscmd;

    sscmd << "#!/bin/sh\ng09 <<END 2>&1 " << input.c_str() << "END\n"; // Redirect cerr to cout

    std::cout << "\"" << sscmd.str() << "\"" << std::endl;

    // Open a pipe and run command -- output saved in string 'out'.
    std::string out(exec(sscmd.str().c_str(),1000));

    // Check for gaussian error in buffer
    if (simtls::trim(out).find("Convergence failure -- run terminated")!=std::string::npos) {
        return true;
    }

    if (simtls::trim(out).find("Normal termination of Gaussian 09")==std::string::npos) {
        std::ofstream gaue;
        gaue.open("gauerror.log");
        gaue << out << std::endl;
        gaue.close();

        throwException("Unrecognized Gaussian 09 Failure; saving output as gauerror.log");
    }

    return false;
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

    // Build input
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
