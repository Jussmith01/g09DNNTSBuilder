#ifndef INTERNALS_H
#define INTERNALS_H

namespace itrnl {

/*----------Bond Index-------------

Consists of:
bndindex - 8 byte aligned
           container for two
           integer indicies.

CreateBondIndex() - A function
           that produces a
           bndindex type from
           two values.

bndCompare() - A function that
            can be used with a
            std::sort for
            sorting the bonds.
-----------------------------*/
// 8 byte alignment forced to prevent cache mis-alignment
struct bndindex {
    int v1,v2;
} __attribute__ ((__aligned__(8)));

// Function for creating bond indexes
inline bndindex CreateBondIndex(const int v1,const int v2) {
    bndindex tmpidx;
    tmpidx.v1=v1;
    tmpidx.v2=v2;
    return tmpidx;
};

// Function for less than comparing bndindex types -- for use by std::sort
inline bool bndComparelt (bndindex i,bndindex j) {
    return ((i.v1<=j.v1) && (i.v2<j.v2));
}

// Function for equality comparing bndindex types
inline bool bndCompareeq (bndindex i,bndindex j) {
    return ((i.v1==j.v1) && (i.v2==j.v2));
}

/*---------Angle Index------------

Consists of:
angindex - 8 byte aligned
           container for 3
           integer indicies.

CreateAngleIndex() - A function
           that produces a
           angindex type from
           3 values.
----------------------------------*/
// 8 byte alignment forced to prevent cache mis-alignment
struct angindex {
    int v1,v2,v3;
} __attribute__ ((__aligned__(16)));

inline angindex CreateAngleIndex(const int v1,const int v2,const int v3) {
    angindex tmpidx;
    tmpidx.v1=v1;
    tmpidx.v2=v2;
    tmpidx.v3=v3;
    return tmpidx;
};

// Function for less than comparing bndindex types -- for use by std::sort
inline bool angComparelt (angindex i,angindex j) {
    return (i.v1<j.v1);
}

/*----------Dihedral Index------------

Consists of:
dhlindex - 8 byte aligned
           container for 4
           integer indicies.

CreateDihedralIndex() - A
           function that
           produces a dhlindex
           type from 4 values.
-------------------------------------*/
// 8 byte alignment forced to prevent cache mis-alignment
struct dhlindex {
    int v1,v2,v3,v4;
} __attribute__ ((__aligned__(16)));

inline dhlindex CreateDihedralIndex(const int v1,const int v2,const int v3,const int v4) {
    dhlindex tmpidx;
    tmpidx.v1=v1;
    tmpidx.v2=v2;
    tmpidx.v3=v3;
    tmpidx.v4=v4;
    return tmpidx;
};


/*--------Internal Coordinates Class----------


This class stores the indexes for the bonds,
angles and dihedrals of the molecule.
----------------------------------------------*/
class Internalcoordinates {
    std::vector<bndindex> bidx; // Bonding index
    std::vector<angindex> aidx; // Angle index
    std::vector<dhlindex> didx; // Dihedral index

    std::vector<float> bnds; // Working storage for bonds
    std::vector<float> angs; // Working storage for angles
    std::vector<float> dhls; // Working storage for dihedrals

    //Internalcoordinates () {}; // Private default constructor

    // Calculate the bonding index
    void m_calculateBondIndex(const std::vector< glm::ivec2 > &mbond);

    // Calculate the angle index
    void m_calculateAngleIndex();

    // Calculate the dihedral index
    void m_calculateDihedralIndex();

    // Calculate bond lengths
    void m_calculateBonds(const std::vector<glm::vec3> &xyz);

    // Calculate angles
    void m_calculateAngles(const std::vector<glm::vec3> &xyz);

    // Calculate dihedrals
    void m_calculateDihedrals(const std::vector<glm::vec3> &xyz);

    // Create and return Comma Separated Values Internal Coordinates string
    std::string m_createCSVICstring(const std::vector<glm::vec3> &xyz);

public:

    // Only public constructor
    Internalcoordinates (const std::vector< glm::ivec2 > &mbond) {
        try {

            m_calculateBondIndex(mbond);
            m_calculateAngleIndex();
            m_calculateDihedralIndex();

            bnds.resize(bidx.size());
            angs.resize(aidx.size());
            dhls.resize(didx.size());

        } catch (std::string error) dnntsErrorcatch(error);
    };

    // Calculate the CSV (Comma Separated Values) string of internal coords based on xyz input
    std::string calculateCSVInternalCoordinates(const std::vector<glm::vec3> &xyz);

    // Data Printer
    void printdata() {
        std::cout << "Internal Coordinates" << std::endl;
        std::cout << "Bonds: " << bidx.size() << std::endl;
        std::cout << "Angles: " << aidx.size() << std::endl;
        std::cout << "Dihedrals: " << didx.size() << std::endl;
        std::cout << std::endl;
    };

    // Destructor
    ~Internalcoordinates() {
        bidx.clear();
        aidx.clear();
        didx.clear();

        bnds.clear();
        angs.clear();
        dhls.clear();
    }
};

};
#endif
