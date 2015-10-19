#ifndef READINPUT_H
#define READINPUT_H

#include <string>
//#include <regex>
#include <tr1/unordered_map>
#include <typeinfo>

// GLM Mathematics Library
#include "../../include/glm/detail/type_vec.hpp"
#include <glm/glm.hpp>

namespace ipt
{

/*----- Input Parameters-------

This class is in charge of loading all
parameters defined in the input file
used to construct the class.

EXTERNAL API:
nninputParamters(std::string inputfname)
    Argument 1: Input file name
    Constructor for the class. This loads
    all parameters from the input file with
    syntax: x = y !comment here
    Also prints all loaded parameters.

typename T getParameter<T>(std::string param)
    Argument 1: Requested Parameter
    Returns a parameter, converted to the
    explicitly requested type. This also
    checks for parameter existence in map
    and throws and error std::string if
    not found.

bool checkParameter(std::string param)
    Argument 1: Requested Parameter
    Returns true if requested parameter
    exists in the map else false.


enum Initializers getRunType()
    Returns the requested run type via
    enum  Initializers.
----------------------------------------*/
class inputParameters {
    //------------------------------------
    //  Hash map to hold flags and values
    //------------------------------------
    /*NOTE! std::unordered_map does not work with NVCC for
    some reason, so using tr1 */
    std::tr1::unordered_map<std::string,std::string> m_params;
    std::vector< std::string > m_coords;
    std::vector< std::string > m_rand;
    std::vector< std::string > m_scan;


    //------------------------------------
    //  Hash map to hold flags and values
    //------------------------------------
    void m_setDefaults() {
        /*NOTE! since std::unordered_map does not work with
        NVCC and tr1 does not have this constructor I use
        vector here */
        std::vector<std::pair<std::string,std::string>>
        defaults({
            //{param , default      descrip  },
            {"dfname"  , "trainingData.dat"    },  // GPUID
        });

        m_params.insert(defaults.begin(),defaults.end());
    };

    //------------------------------------
    //       Read the input file
    //------------------------------------
    void m_readInput() {
        std::ifstream ipt(getParameter<std::string>("ifname").c_str());
        bool readcoords(false);
        bool readrand(false);
        bool readscan(false);
        if (ipt.is_open()) {
            std::string(line);
            while ( getline (ipt,line) ) {
                line = line.substr(0,line.find("#"));

                if (readcoords) {
                    if (simtls::trim(line).compare("$endcoordinates")!=0) {
                        //std::cout << "COORDS: " << line << std::endl;
                        m_coords.push_back(simtls::trim(line));
                    } else {
                        //readcoords = false;
                        readcoords=false;
                    }
                }

                if (readrand) {
                    if (simtls::trim(line).compare("$endrandomrange")!=0) {
                        //std::cout << "COORDS: " << line << std::endl;
                        m_rand.push_back(simtls::trim(line));
                    } else {
                        //readcoords = false;
                        readrand=false;
                    }
                }

                if (readscan) {
                    if (simtls::trim(line).compare("$endscanrange")!=0) {
                        //std::cout << "COORDS: " << line << std::endl;
                        m_scan.push_back(simtls::trim(line));
                    } else {
                        //readcoords = false;
                        readscan=false;
                    }
                }

                if (simtls::trim(line).compare("$coordinates")==0) {
                    readcoords = true;
                }

                if (simtls::trim(line).compare("$randomrange")==0) {
                    readrand = true;
                }

                if (simtls::trim(line).compare("$scanrange")==0) {
                    readscan = true;
                }

                if (!readcoords && !readrand && !readscan)
                    //std::cout << "PARM: " << line << std::endl;
                    m_setParameter(line);
            }
            ipt.close();
        } else {
            std::stringstream _err;
            _err << "Unable to open file: " << getParameter<std::string>("inputfname") << std::endl;
            dnntsErrorcatch(_err.str());
        }
    };


    //------------------------------------
    //     Set Parameter from the Map
    //------------------------------------
    void m_setParameter(std::string line) {
        using namespace std;

        if (line.find("!")!=string::npos)
            line=line.substr(0,line.find("!"));

        size_t epos = line.find("=");
        if (epos!=string::npos) {
            string arg1(simtls::trim(line.substr(0,epos)));
            string arg2(simtls::trim(line.substr(epos+1)));

            std::tr1::unordered_map<std::string,std::string>::iterator it = m_params.find(arg1);
            if (it == m_params.end()) {
                pair<string,string> pset(arg1,arg2);
                m_params.insert(pset);
            } else {
                it->second = arg2;
            }
        }
    };

    //------------------------------------
    //   Print the parameters from Input
    //------------------------------------
    void m_printInputParameters() {
        if (checkParameter("tss") && checkParameter("tbatchsz")) {
            if (int(getParameter<unsigned>("tss"))%int(getParameter<unsigned>("tbatchsz"))!=0)
                dnntsErrorcatch(std::string("Batch size must evenly divide training set size!"));
        }

        std::cout << "|--------Input Parameters-------|" << std::endl;
        for (auto& x : m_params)
            std::cout << " " << x.first << " = " << x.second << std::endl;

        std::cout << "Coordinates: " << std::endl;
        for (auto& x : m_coords)
            std::cout << x << std::endl;

        std::cout << "|-------------------------------|" << std::endl;
    };

public:

    //-----------------------------------
    //          Constructor
    //-----------------------------------
    inputParameters(std::string inputfname) {
        m_setDefaults();
        std::pair<std::string,std::string> iset("ifname",inputfname);
        m_params.insert(iset);
        m_readInput();
        m_printInputParameters();
    };

    //-----------------------------------
    //          Constructor
    //-----------------------------------
    inputParameters(std::string inputfname,std::string datafname) {
        m_setDefaults();
        std::pair<std::string,std::string> iset("ifname",inputfname);
        std::pair<std::string,std::string> dset("dfname",datafname);
        m_params.insert(iset);
        m_params.insert(dset);
        m_readInput();
        m_printInputParameters();
    };

    //------------------------------------
    //     Get Parameter from the Map
    //------------------------------------
    template <typename T>
    T getParameter(std::string param) {
        std::tr1::unordered_map<std::string,std::string>::const_iterator it = m_params.find(param);
        if (it == m_params.end()) {
            std::stringstream ss;
            ss << "Requested parameter not found in input file!\n";
            ss << "              Parameter: " << param << "\n";
            ss << "              Check your input file for the above parameter!";
            dnntsErrorcatch(ss.str());
        }

        if (typeid(T)==typeid(unsigned)) {
            if (!simtls::stristype(it->second,"unsigned")){
                std::stringstream ss;
                ss << "Requested parameter not type unsigned int in input file!\n";
                ss << "              Parameter: " << param << "\n";
                ss << "              Value: " << it->second << "\n";
                ss << "              Make sure the above parameter is correct!";
                dnntsErrorcatch(ss.str());
            }
        } else if (typeid(T)==typeid(float)) {
            if (!simtls::stristype(it->second,"float")){
                std::stringstream ss;
                ss << "Requested parameter not type float in input file!\n";
                ss << "              Parameter: " << param << "\n";
                ss << "              Value: " << it->second << "\n";
                ss << "              Make sure the above parameter is correct!";
                dnntsErrorcatch(ss.str());
            }
        } else if (typeid(T)==typeid(int)) {
            if (!simtls::stristype(it->second,"int")){
                std::stringstream ss;
                ss << "Requested parameter not type int in input file!\n";
                ss << "              Parameter: " << param << "\n";
                ss << "              Value: " << it->second << "\n";
                ss << "              Make sure the above parameter is correct!";
                dnntsErrorcatch(ss.str());
            }
        } else if (typeid(T)==typeid(bool)) {
            if (!simtls::stristype(it->second,"bool")){
                std::stringstream ss;
                ss << "Requested parameter not type bool in input file!\n";
                ss << "              Parameter: " << param << "\n";
                ss << "              Value: " << it->second << "\n";
                ss << "              Make sure the above parameter is correct!";
                dnntsErrorcatch(ss.str());
            }
        }

        std::istringstream iss( it->second );

        T value;
        iss >> value;

        return value;
    }

    //------------------------------------
    //    Check if Parameter is in Map
    //------------------------------------
    bool checkParameter(std::string param) {
        std::tr1::unordered_map<std::string,std::string>::const_iterator it = m_params.find(param);
        if (it == m_params.end()) {
            return false;
        }

        return true;
    };

    //------------------------------------
    //         Get coordinates
    //------------------------------------
    const std::vector< std::string >& getCoordinatesStr() {
        return m_coords;
    };

    //------------------------------------
    //         Get coordinates
    //------------------------------------
    std::vector< std::string >& getRandStr() {
        return m_rand;
    };

    //------------------------------------
    //         Get coordinates
    //------------------------------------
    std::vector< std::string >& getScanStr() {
        return m_scan;
    };
};

};
#endif
