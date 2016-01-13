#ifndef flaghandler_header
#define flaghandler_header

//      *************************************************************     //
//                            Flag Handler Class
//                  	    Handles all input flags
//      *************************************************************     //
class FlagHandler {
    //------------------------------------
    //  Hash map to hold flags and values
    //------------------------------------
    std::unordered_map<std::string,std::string> flagshash;

    //------------------------------------
    //  Hash map to hold flags and values
    //------------------------------------
    void Defaults() {
        std::unordered_map<std::string,std::string>
        defaults = {
            //{flag , default      descrip  },
            {"-v" , "0"        /*Unused*/ }, // Verbose
            {"-p" , "-1"       /*Unused*/ }, // Number of Processors
            {"-i" , ""         /*Error*/  }, // Input file
            {"-o" , ""         /*cout */  }, // Output file
            {"-r" , "normal"   /*random */}, // Random type normal or uniform distribution
            {"-d" , "trainingdata.dat"    }  // Training data file
        };

        flagshash.insert(defaults.begin(),defaults.end());
    };

    //------------------------------
    //Private Member Class Functions
    //------------------------------
    void setFlags(std::vector<std::string> &flags) {
        std::vector<std::string>::iterator it;

        std::unordered_map<std::string,double>::const_iterator get;

        //regex pattern_unrecflag("-^[n,v,p,i,o,d]$|-[a-zA-Z0-9]{2,}");

        for (it=flags.begin(); it!=flags.end(); it++) {
            if ((*it).find("-h")!=std::string::npos) {
                std::cout << "Help menu: " << std::endl;
                std::cout << "  -v [0-3] is verbosity of output (unused)" << std::endl;
                std::cout << "  -p [1-Max Threads] is number of processors: " << std::endl;
                std::cout << "  -i [inputfilename] must have .ipt extension (REQUIRED)" << std::endl;
                std::cout << "  -o [outputfilename] must have .opt extension " << std::endl;
                std::cout << "  -d [dataoutfilename] must have .dat extension " << std::endl;

                std::cout << "Example execution: dnntsbuilder -i input.ipt -v 0 -p 8\n" << std::endl;
                //exit(0);
            } else if ((*it).find("-v")!=std::string::npos) {
                std::unordered_map<std::string,std::string>::
                iterator get = flagshash.find("-v");
                std::regex pattern_verbose("^[0-3](\\s)*(?!.)");
                if (!std::regex_search(*(it+1),pattern_verbose)) {
                    throwException("-v flag expects a positive integer!");
                };
                get->second = *(it+1);
            } else if ((*it).find("-p")!=std::string::npos) {
                std::unordered_map<std::string,std::string>::
                iterator get = flagshash.find("-p");
                std::regex pattern_processor("[[:d:]]+");
                if (!std::regex_search(*(it+1),pattern_processor)) {
                    throwException("-p flag expects an integer!");
                };
                get->second = *(it+1);
            } else if ((*it).find("-i")!=std::string::npos) {
                std::unordered_map<std::string,std::string>::
                iterator get = flagshash.find("-i");
                std::regex pattern_inputfile("[^-].*\\.ipt");
                if (!std::regex_search(*(it+1),pattern_inputfile)) {
                    throwException("-i flag expects a .ipt filename!");
                };
                get->second = *(it+1);
            } else if ((*it).find("-o")!=std::string::npos) {
                std::unordered_map<std::string,std::string>::
                iterator get = flagshash.find("-o");
                std::regex pattern_outputfile("[^-].*\\.opt");
                if (!std::regex_search(*(it+1),pattern_outputfile)) {
                    throwException("-o flag expects a .opt filename!");
                };
                get->second = *(it+1);
            } else if ((*it).find("-d")!=std::string::npos) {
                std::unordered_map<std::string,std::string>::
                iterator get = flagshash.find("-d");
                std::regex pattern_dataoutfile("[^-].*\\.dat");
                if (!std::regex_search(*(it+1),pattern_dataoutfile)) {
                    throwException("-d flag expects a .dat filename!!");
                };
                get->second = *(it+1);
            } else if ((*it).find("-r")!=std::string::npos) {
                std::unordered_map<std::string,std::string>::
                iterator get = flagshash.find("-r");

                get->second = *(it+1);

                if (get->second.find("uniform")==std::string::npos && get->second.find("normal")==std::string::npos) {
                    throwException("-r flag expects 'uniform' or 'normal'!!");
                };
            }
        }

        std::unordered_map<std::string,std::string>::const_iterator getichk = flagshash.find("-i");
        if (getichk->second.empty())
            throwException("-i is required. Execute with -h for more information.");

    };

public:
    //-----------------------------
    //	Class Constructor
    //-----------------------------
    FlagHandler(int argc,char *argv[]) {
        std::vector<std::string> flags(argc);
        std::vector<std::string>::iterator it;

        for (it=flags.begin(); it!=flags.end(); it++)
            *it=argv[it-flags.begin()];

        if (argc == 1) {
            flags.push_back("-h");
        }

        Defaults(); // Set the default flags

        try {
            setFlags(flags); // Set flags from command arguments
        } catch (std::string error) dnntsErrorcatch(error);
    };

    std::string getflag(std::string flag) {
        std::unordered_map<std::string,std::string>::const_iterator get = flagshash.find(flag);
        return get->second;
    };
};
#endif

