#ifndef TSBUILDER_H
#define TSBUILDER_H

/*------------Training Set Builder-------------

----------------------------------------------*/
class Trainingsetbuilder {
    // Class for calculating input coordinates
    itrnl::Internalcoordinates icrd;
    ipt::input *iptData;
    FlagHandler *args;

    bool routecout;
    //std::streambuf *coutbuf;

    // This generates a random structure from the minimum
    void m_generateRandomStructure(int nrpg,const std::vector<glm::vec3> &ixyz,std::vector<glm::vec3> &oxyz,RandomReal &rnGen);

    // This checks if a random structure is okay for gaussian
    bool m_checkRandomStructure(const std::vector<glm::vec3> &xyz);

public:
    // Constructor
    Trainingsetbuilder (ipt::input *iptData,FlagHandler *args) :
        icrd(iptData->getbonds()),routecout(false)
    {
        // Save a pointer to the input data class
        this->iptData = iptData;
        if (this->iptData == NULL)
            dnntsErrorcatch(std::string("Input data has not been declared!"));

        // Save a pointer to the input data class
        this->args = args;
        if (this->args == NULL)
            dnntsErrorcatch(std::string("Arguments has not been declared!"));

        // Route cout to output file if output was supplied
        if (!this->args->getflag("-o").empty())
        {
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
