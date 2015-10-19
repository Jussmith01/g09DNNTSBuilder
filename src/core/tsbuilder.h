#ifndef TSBUILDER_H
#define TSBUILDER_H

/*------------Training Set Builder-------------

----------------------------------------------*/
class Trainingsetbuilder {
    // Class for calculating input coordinates
    ipt::inputParameters iptData;
    itrnl::Internalcoordinates icrd;
    FlagHandler *args;

    bool routecout;

    // This checks if a random structure is okay for gaussian
    bool m_checkRandomStructure(const std::vector<glm::vec3> &xyz);

public:
    // Constructor
    Trainingsetbuilder (FlagHandler *args) :
        iptData(args->getflag("-i"),args->getflag("-d")),
        icrd(iptData.getCoordinatesStr()),
        routecout(false) {

        if        (iptData.getRandStr().size() > 0) {
            icrd.getRandRng().setRandomRanges(iptData.getRandStr());
        } else if (iptData.getScanStr().size() > 0) {
            icrd.getScanRng().setScanRanges(iptData.getScanStr());
        } else {dnntsErrorcatch(std::string("Structure generation scan or random range has not been set!"));}

        // Save a pointer to the input data class
        this->args = args;
        if (this->args == NULL)
            dnntsErrorcatch(std::string("Arguments has not been declared!"));

        // Route cout to output file if output was supplied
        if (!this->args->getflag("-o").empty()) {
            routecout = true;
            FILE* stdo = freopen(this->args->getflag("-o").c_str(),"w",stdout);
            if (stdo) {};
        }
    };

    // Destructor
    ~Trainingsetbuilder() {};

    // This holds the main loop for calculating the training set
    void calculateTrainingSet();
};

#endif
