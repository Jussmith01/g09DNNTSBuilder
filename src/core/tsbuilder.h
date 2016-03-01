#ifndef __TSBUILDER_INCLUDE__
#define __TSBUILDER_INCLUDE__

/*------------Training Set Builder-------------

----------------------------------------------*/
class Trainingsetbuilder {
    // Class for calculating input coordinates
    ipt::inputParameters *iptData;
    itrnl::RandomCartesian rcrd;
    FlagHandler *args;

    bool routecout;

    // This checks if a random structure is okay for gaussian
    bool m_checkRandomStructure(const std::vector<glm::vec3> &xyz);

public:
    // Constructor
    Trainingsetbuilder (FlagHandler *args,ipt::inputParameters *iptData) :
        iptData(iptData),
        rcrd(iptData->getCoordinatesStr(),iptData->getConnStr(),iptData->getRandStr()),
        args(args),
        routecout(false) {
        using namespace std;

        // Save a pointer to the input data class
        this->args = args;
        if (this->args == NULL)
            dnntsErrorcatch(string("Arguments has not been declared!"));

        // Route cout to output file if output was supplied
        if (!this->args->getflag("-o").empty()) {
            routecout = true;
            FILE* stdo = freopen(this->args->getflag("-o").c_str(),"w",stdout);
            if (stdo) {};
        }

        cout << "-----Beginning Data Set Computation-----\n" << endl;

        cout << "Using " << omp_get_max_threads() << " threads.\n" << endl;

        if (iptData->getParameter<unsigned>("optimize") == 1) {
            optimizeStoredStructure();
        } else {
            cout << "Optimization Turned Off." << endl;
        }

    };

    // Destructor
    ~Trainingsetbuilder() {};

    void optimizeStoredStructure();

    // This holds the main loop for calculating the training set
    void calculateTrainingSet();

    void calculateValidationSet();
};

#endif
