#ifndef __SSBUILDER_INCLUDED__
#define __SSBUILDER_INCLUDED__

/*------------Scan Set Builder-------------

------------------------------------------*/
class Scansetbuilder {
    // Class for calculating input coordinates
    ipt::inputParameters *iptData;
    itrnl::ScanCartesian scrd;
    FlagHandler *args;

    // This checks if a random structure is okay for gaussian
    bool m_checkRandomStructure(const std::vector<glm::vec3> &xyz,const std::vector<std::string> &type);

public:
    // Constructor
    Scansetbuilder (FlagHandler *args,ipt::inputParameters *iptData) :
        iptData(iptData),
        scrd(iptData->getCoordinatesStr(),iptData->getConnStr(),iptData->getScanStr()),
        args(args) {

    };

    // Destructor
    ~Scansetbuilder() {};

    // This holds the main loop for calculating the training set
    void calculateScanSet();
};

#endif
