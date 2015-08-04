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
inline std::string exec(std::string cmd,size_t maxbuf) {

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
inline bool execg09(std::string ipt,std::string opt) {
    // Build command
    std::stringstream sscmd;
    sscmd << "g09 " << ipt << " " << opt << " 2>&1"; // Redirect cerr to cout

    // Open a pipe and run command -- output saved in string 'out'.
    std::string out(exec(sscmd.str().c_str(),1000));

    // Check for gaussian error in buffer
    if (simtls::trim(out).find("Error: segmentation violation")!=std::string::npos) {
        return true;
    }

    return false;
};

};

#endif
