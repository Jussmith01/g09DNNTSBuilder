//
// Created by Justin Smith and Dustin Tracy on 7/5/15.
//

#ifndef SIMPLE_TOOLS_HPP
#define SIMPLE_TOOLS_HPP

/*   The Simple Tools Namespace   */
namespace simtls {
/*----------------------------------------
    Trim Whitespace from String Function
------------------------------------------*/
inline std::string trim (std::string line)
{
    size_t first = line.find_first_not_of(" \t\r\n\x0b");
    if (first!=std::string::npos) // first should always return npos if space only contains white space or is empty
        return line.substr(first,line.find_last_not_of(" \t\r\n\x0b")+1-first);
    else return std::string("");
};

/*----------------------------------------
        Test if a file is present
------------------------------------------*/
inline std::string fileTest(std::string filename)
{
    std::ifstream test;
    test.open(filename.c_str());
    if (!test.is_open()) {
        std::stringstream err;
        err << "Cannot open file: " << filename;
        return err.str();
    }
    test.close();

    return "";
};

};

#endif
