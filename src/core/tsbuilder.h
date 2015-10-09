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
    //std::streambuf *coutbuf;

    // This generates a random structure from the minimum
    void m_generateRandomStructure(int nrpg,const std::vector<glm::vec3> &ixyz,std::vector<glm::vec3> &oxyz,RandomReal &rnGen);

    // This checks if a random structure is okay for gaussian
    bool m_checkRandomStructure(const std::vector<glm::vec3> &xyz);

public:
    // Constructor
    Trainingsetbuilder (FlagHandler *args) :
        iptData(args->getflag("-i"),args->getflag("-d")),
        icrd(iptData.getCoordinatesStr()),
        routecout(false) {

        icrd.getRandRng().setRandomRanges(iptData.getRandStr());

        // Save a pointer to the input data class
        this->args = args;
        if (this->args == NULL)
            dnntsErrorcatch(std::string("Arguments has not been declared!"));

        // Route cout to output file if output was supplied
        if (!this->args->getflag("-o").empty()) {
            routecout = true;
            freopen(this->args->getflag("-o").c_str(),"w",stdout);
        }
    };

    // Destructor
    ~Trainingsetbuilder() {};

    // This holds the main loop for calculating the training set
    void calculateTrainingSet();
};

#endif
