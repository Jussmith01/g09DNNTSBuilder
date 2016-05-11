#ifndef __TSNMBUILDER_INCLUDE__
#define __TSNMBUILDER_INCLUDE__

/*------------Training Set Builder-------------

----------------------------------------------*/
class TrainingsetNormModebuilder {
    // Class for calculating input coordinates
    ipt::inputParameters *iptData;
    itrnl::RandomStructureNormalMode rnmcrd;
    //itrnl::RandomNormalMode nmcrd;
    FlagHandler *args;

    bool routecout;

    // This checks if a random structure is okay for gaussian
    bool m_checkRandomStructure(const std::vector<glm::vec3> &xyz);

public:
    // Constructor
    TrainingsetNormModebuilder (FlagHandler *args,ipt::inputParameters *iptData) :
        iptData(iptData),
        rnmcrd(iptData->getCoordinatesStr(),iptData->getNormModeStr()),
        args(args),
        routecout(false) {
        using namespace std;

        // Save a pointer to the input data class
        this->args = args;
        if (this->args == NULL)
            dnntsErrorcatch(string("Arguments have not been declared!"));

        // Route cout to output file if output was supplied
        if (!this->args->getflag("-o").empty()) {
            routecout = true;
            FILE* stdo = freopen(this->args->getflag("-o").c_str(),"w",stdout);
            if (stdo) {};
        }

        cout << "-----Beginning Data Set Computation-----\n" << endl;

        cout << "Using " << omp_get_max_threads() << " threads.\n" << endl;
    };

    // Destructor
    ~TrainingsetNormModebuilder() {};

    bool optimizer(std::string LOT
                  ,std::string g09Args
                  ,const std::vector<std::string> &itype
                  ,std::vector<glm::vec3> &xyz
                  ,unsigned charge
                  ,unsigned multip);

    bool normalmodecalc(std::string LOT
                        ,std::string g09Args
                        ,const std::vector<std::string> &itype
                        ,const std::vector<glm::vec3> &xyz
                        ,std::vector<std::vector<glm::vec3>> &nc
                        ,std::vector<float> &fc
                        ,unsigned charge
                        ,unsigned multip);

    void optimizeStoredStructure();

    void calculateNormalModes();

    // This holds the main loop for calculating the training set
    void calculateTrainingSet();

    void calculateValidationSet();

    void calculateTestSet();
};



#endif
