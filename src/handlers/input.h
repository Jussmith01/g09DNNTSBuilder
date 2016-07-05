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
    std::string m_norm;

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
            {"dfname"  , "trainingData.dat"    },   // Output data filename
            {"charge"  , "0"                   },   // Molecule charge
            {"multip"  , "1"                   },   // Molecule Multiplicity
            {"minimize", "0"                   },   // Molecule Multiplicity
            {"threads" , "0"/* 0 Uses Default*/},   // Number of threads to use (if set to 0, the program uses the max threads it can)
            {"ngpr"    , "1"                   }    // Number gaussian inputs per pipe opened (one is fine)
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

        unsigned cntr = 0;
        ++cntr; cout << "TEST " << cntr << endl;
        regex pattern_parametr("([^\\s]+)\\s*=\\s*([^\\s^#^!]+)\\s*",regex_constants::optimize);
        if (regex_search(instr,pattern_parametr)) {
            sregex_iterator items(instr.begin(),instr.end(),pattern_parametr);
            sregex_iterator end;
            for (; items != end; ++items) {
                m_setParameter(items->str(1),items->str(2));
            }
        }

        ++cntr; cout << "TEST " << cntr << endl;
        regex pattern_crdblock("\\$coordinates.*\\n([^&]*)",regex_constants::optimize);
        smatch cm;
        if (regex_search(instr,cm,pattern_crdblock))
            m_crds = cm.str(1);

        ++cntr; cout << "TEST " << cntr << endl;
        regex pattern_conblock("\\$connectivity.*\\n([^&]*)",regex_constants::optimize);
        smatch cn;
        if (regex_search(instr,cn,pattern_conblock))
            m_conn = cn.str(1);

        ++cntr; cout << "TEST " << cntr << endl;
        regex pattern_rndblock("\\$randrange.*\\n([^&]*)",regex_constants::optimize);
        smatch rm;
        if (regex_search(instr,rm,pattern_rndblock))
            m_rand = rm.str(1);

        ++cntr; cout << "TEST " << cntr << endl;
        regex pattern_scnblock("\\$scanrange.*\\n([^&]*)",regex_constants::optimize);
        smatch sm;
        if (regex_search(instr,sm,pattern_scnblock)) {
            m_scan = sm.str(1);
        }

        ++cntr; cout << "TEST " << cntr << endl;
        regex pattern_ncblock("\\$normalmodes.*\\n([^&]*)",regex_constants::optimize);
        smatch nm;
        if (regex_search(instr,sm,pattern_ncblock)) {
            m_norm = sm.str(1);
        }

        ++cntr; cout << "TEST " << cntr << endl;

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
        using namespace std;
        if (checkParameter("tss") && checkParameter("tbatchsz")) {
            if (int(getParameter<unsigned>("tss"))%int(getParameter<unsigned>("tbatchsz"))!=0)
                dnntsErrorcatch(string("Batch size must evenly divide training set size!"));
        }

        cout << "|--------Input Parameters-------|" << endl;
        for (auto& x : m_params)
            cout << " " << x.first << " = " << x.second << endl;

        cout << "\nCoordinates: " << endl;
        cout << m_crds << endl;

        std::cout << "\nConnectivity: " << std::endl;
        std::cout << m_conn << std::endl;

        if (getParameter<std::string>("type").compare("random")==0 && !m_rand.empty()) {
            std::cout << "\nRandom Range: " << std::endl;
            std::cout << m_rand << std::endl;
        } else if (getParameter<std::string>("type").compare("random")==0 && m_rand.empty()) {
            dnntsErrorcatch(std::string("ERROR: For type random, randrange must be set in input."));
        }

        if (getParameter<std::string>("type").compare("moldyn")==0 && !m_rand.empty()) {
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

        if (getParameter<std::string>("type").compare("nmrandom")==0 && !m_norm.empty()) {
            std::cout << "\nScan Range: " << std::endl;
            std::cout << m_norm << std::endl;
        } else if (getParameter<std::string>("type").compare("nmrandom")==0 && m_norm.empty()) {
            dnntsErrorcatch(std::string("ERROR: For type nmrandom, normalcoords must be set in input."));
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
        //m_printInputParameters();
    };

    //-----------------------------------
    //          Constructor
    //-----------------------------------
    inputParameters(std::string inputfname,std::string datafname) {
        using namespace std;

        m_setDefaults();
        pair<string,string> iset("ifname",inputfname);
        pair<string,string> dset("dfname",datafname);
        m_params.insert(iset);
        m_params.insert(dset);
        m_readInput();
        //m_printInputParameters();
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
    //     Set Parameter from the Map
    //------------------------------------
    template <typename T>
    void setParameter(std::string param,T value) {
        using namespace std;

        std::stringstream _in;
        _in << value;

        std::tr1::unordered_map<std::string,std::string>::iterator it = m_params.find(param);
        if (it == m_params.end()) {
            pair<string,string> pset(param,_in.str());
            m_params.insert(pset);
        } else {
            it->second = _in.str();
        }
    };

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
    void storeInputWithOptCoords(const std::vector<glm::vec3> &coords, bool disableopt) {
        using namespace std;

        regex pattern_crds("(\\s*[A-Z][a-z]*\\s+[A-Z][a-z]*\\d*)\\s+([-,+]*\\d+\\.\\d+\\s+[-,+]*\\d+\\.\\d+\\s+[-,+]*\\d+\\.\\d+)(.*)");
        regex pattern_opt("(optimize=)(.*)");

        stringstream newfile;
        ifstream iptfile(getParameter<string>("ifname").c_str());

        string line;
        unsigned cidx(0);
        while (getline(iptfile,line)) {

            if (regex_match(line,pattern_crds)) {
                stringstream ss;
                ss.setf( ios::fixed, ios::floatfield );
                ss << "$1 " << setprecision(7) << coords[cidx].x << " " << coords[cidx].y << " " << coords[cidx].z << " $3";
                line = regex_replace (line,pattern_crds,ss.str().c_str());
                ++cidx;
            }

            if (disableopt && regex_search(line,pattern_opt)) {
                newfile << regex_replace (line,pattern_opt,"$1 0 !Coordinates pre-opt") << endl;
            } else {
                newfile << line << endl;
            }
        }

        iptfile.close();

        //cout << newfile.str();

        ofstream newiptfile (getParameter<string>("ifname").c_str());
        newiptfile << newfile.str();
        newiptfile.close();
    };

    //------------------------------------
    //      Get coordinates String
    //------------------------------------
    void storeInputWithNormModes(const std::vector<std::vector<glm::vec3>> &nc,const std::vector<float> &fc, unsigned Na, bool disablefreq) {

        using namespace std;

        regex pattern_freq("(frequency=)(.*\\n)");
        regex pattern_ncblock("(\\$normalmodes.*\\n)([^&]*)",regex_constants::optimize);

        // Open File
        string msfile(getParameter<std::string>("ifname"));
        ifstream ipt(msfile.c_str(), ios::in | ios::binary);

        if (!ipt) {
            cout << "NO FILE!" << endl;
            string __err(string("Cannot open file ") + msfile);
            dnntsErrorcatch(__err);
        }

        // Load entire file into a string on memory
        string instr( (istreambuf_iterator<char>(ipt)), istreambuf_iterator<char>() );
        ipt.close();

        string addnc;

        //cout << "TEST!!" << endl;
        if (regex_search(instr,pattern_ncblock)) {
            stringstream ss;
            ss.setf( ios::scientific, ios::floatfield );

            //cout << "TEST2!! Na: " << Na << endl;
            ss << "$1";
            for (unsigned i = 0; i < fc.size(); ++i) {
                ss << "FRCCNST=" << fc[i] << " {\n";
                for (unsigned j = 0; j < Na; ++j) {
                    ss << setprecision(7) << " " << nc[i][j].x << " " << nc[i][j].y << " " << nc[i][j].z << endl;
                }
                ss << "}\n";
                //cout << ss.str() << endl;
            }
            addnc = regex_replace (instr,pattern_ncblock,ss.str().c_str());
        }

        stringstream newfile;
        if (disablefreq && regex_search(addnc,pattern_freq)) {
            newfile << regex_replace (addnc,pattern_freq,"$1 0 !Freq pre-calc\n") << endl;
        } else {
            newfile << addnc << endl;
        }

        //cout << newfile.str() << endl;

        ofstream newiptfile (getParameter<string>("ifname").c_str());
        newiptfile << newfile.str();
        newiptfile.close();

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

    //------------------------------------
    //     Get Scan Tranform String
    //------------------------------------
    const std::string& getNormModeStr() {
        return m_norm;
    };
};

};
#endif
