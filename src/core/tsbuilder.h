#ifndef TSBUILDER_H
#define TSBUILDER_H

/*------------Training Set Builder-------------

----------------------------------------------*/
class Trainingsetbuilder {
    // Output to the training set
    std::ofstream tsout;

    // Class for calculating input coordinates
    itrnl::Internalcoordinates icrd;
    ipt::input *iptData;

    // This generates a random structure from the minimum
    std::vector<glm::vec3> m_generateRandomStructure(const std::vector<glm::vec3> &ixyz,NormRandomReal &rnGen);

    // This checks if a random structure is okay for gaussian
    bool m_checkRandomStructure(const std::vector<glm::vec3> &xyz);

public:
    // Constructor
    Trainingsetbuilder (ipt::input *iptData) :
        icrd(iptData->getbonds())
    {
        // Save a point to the input data class
        this->iptData = iptData;
        if (this->iptData == NULL)
            dnntsErrorcatch(std::string("Input data has not been declared!"));

        // Open the training set output
        tsout.open(this->iptData->getoname().c_str());
    };

    // Destructor
    ~Trainingsetbuilder()
    {
        // Close training set output
        tsout.close();
    };

    // This holds the main loop for calculating the training set
    void calculateTrainingSet();
};

#endif
