#ifndef READINPUT_H
#define READINPUT_H

#include <string>
#include <regex>
#include <tr1/unordered_map>
#include <typeinfo>

// GLM Mathematics Library
#include "../../include/glm/detail/type_vec.hpp"
#include <glm/glm.hpp>

namespace ipt {

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
    std::string m_crds;
    std::string m_conn;
    std::string m_rand;
    std::string m_scan;

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
            {"dfname", "trainingData.dat"    },    // GPUID
        });

        m_params.insert(defaults.begin(),defaults.end());
    };

    //------------------------------------
    //       Read the input file
    //------------------------------------
    void m_readInput() {
        using namespace std;

        // Open File
        string msfile(getParameter<std::string>("ifname"));
        ifstream ipt(msfile.c_str(), ios::in | ios::binary);

        if (!ipt) {
            cout << "NO FILE!" << endl;
            std::string __err(string("Cannot open file ") + msfile);
            dnntsErrorcatch(__err);
        }

        // Load entire file into a string on memory
        string instr( (istreambuf_iterator<char>(ipt)), istreambuf_iterator<char>() );
        ipt.close();

        regex pattern_parametr("([^\\s]+)\\s*=\\s*([^\\s]+)\\s*\\#*",regex_constants::optimize);
        if (regex_search(instr,pattern_parametr)) {
            sregex_iterator items(instr.begin(),instr.end(),pattern_parametr);
            sregex_iterator end;
            for (; items != end; ++items) {
                m_setParameter(items->str(1),items->str(2));
            }
        }

        regex pattern_crdblock("\\$coordinates.*\\n([^&]*)",regex_constants::optimize);
        smatch cm;
        if (regex_search(instr,cm,pattern_crdblock))
            m_crds = cm.str(1);

        regex pattern_conblock("\\$connectivity.*\\n([^&]*)",regex_constants::optimize);
        smatch cn;
        if (regex_search(instr,cn,pattern_conblock))
            m_conn = cn.str(1);

        regex pattern_rndblock("\\$randrange.*\\n([^&]*)",regex_constants::optimize);
        smatch rm;
        if (regex_search(instr,rm,pattern_rndblock))
            m_rand = rm.str(1);

        regex pattern_scnblock("\\$scanrange.*\\n([^&]*)",regex_constants::optimize);
        smatch sm;
        if (regex_search(instr,sm,pattern_scnblock)) {
            m_scan = sm.str(1);
        }

        if ((m_rand.size() > 0 || m_rand.size() > 0) && m_conn.size() == 0 ) {
            dnntsErrorcatch(std::string("ERROR: If randrange or scanrange is set then connectivity must also be set."));
        }
    };


    //------------------------------------
    //     Set Parameter from the Map
    //------------------------------------
    void m_setParameter(std::string param,std::string value) {
        using namespace std;

        std::tr1::unordered_map<std::string,std::string>::iterator it = m_params.find(param);
        if (it == m_params.end()) {
            pair<string,string> pset(param,value);
            m_params.insert(pset);
        } else {
            it->second = value;
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

        std::cout << "\nCoordinates: " << std::endl;
        std::cout << m_crds << std::endl;

        std::cout << "\nConnectivity: " << std::endl;
        std::cout << m_conn << std::endl;

        if (getParameter<std::string>("type").compare("random")==0 && !m_rand.empty()) {
            std::cout << "\nRandom Range: " << std::endl;
            std::cout << m_rand << std::endl;
        } else if (getParameter<std::string>("type").compare("random")==0 && m_rand.empty()) {
            dnntsErrorcatch(std::string("ERROR: For type random, randrange must be set in input."));
        }

        if (getParameter<std::string>("type").compare("scan")==0 && !m_scan.empty()) {
            std::cout << "\nScan Range: " << std::endl;
            std::cout << m_scan << std::endl;
        } else if (getParameter<std::string>("type").compare("scan")==0 && m_scan.empty()) {
            dnntsErrorcatch(std::string("ERROR: For type scan, scanrange must be set in input."));
        }

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
            if (!simtls::stristype(it->second,"unsigned")) {
                std::stringstream ss;
                ss << "Requested parameter not type unsigned int in input file!\n";
                ss << "              Parameter: " << param << "\n";
                ss << "              Value: " << it->second << "\n";
                ss << "              Make sure the above parameter is correct!";
                dnntsErrorcatch(ss.str());
            }
        } else if (typeid(T)==typeid(float)) {
            if (!simtls::stristype(it->second,"float")) {
                std::stringstream ss;
                ss << "Requested parameter not type float in input file!\n";
                ss << "              Parameter: " << param << "\n";
                ss << "              Value: " << it->second << "\n";
                ss << "              Make sure the above parameter is correct!";
                dnntsErrorcatch(ss.str());
            }
        } else if (typeid(T)==typeid(int)) {
            if (!simtls::stristype(it->second,"int")) {
                std::stringstream ss;
                ss << "Requested parameter not type int in input file!\n";
                ss << "              Parameter: " << param << "\n";
                ss << "              Value: " << it->second << "\n";
                ss << "              Make sure the above parameter is correct!";
                dnntsErrorcatch(ss.str());
            }
        } else if (typeid(T)==typeid(bool)) {
            if (!simtls::stristype(it->second,"bool")) {
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
    //      Get coordinates String
    //------------------------------------
    const std::string& getCoordinatesStr() {
        return m_crds;
    };

    //------------------------------------
    //      Get Connectivity String
    //------------------------------------
    const std::string& getConnStr() {
        return m_conn;
    };


    //------------------------------------
    //     Get Random Tranform String
    //------------------------------------
    const std::string& getRandStr() {
        return m_rand;
    };

    //------------------------------------
    //     Get Scan Tranform String
    //------------------------------------
    const std::string& getScanStr() {
        return m_scan;
    };
};

};
#endif
