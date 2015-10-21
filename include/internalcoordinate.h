#ifndef INTERNALS_H
#define INTERNALS_H
/*----------------------------------------------
        Written by Justin Smith ~August 2015
        E-Mail Jussmith48@gmail.com
        Copyright the Roitberg research group
        Chemistry Department
        University of Florida
        Gainesville FL.
------------------------------------------------*/
//________________________________________________________________________//
//      *************************************************************     //
//                      Compiler Error Handling
//      *************************************************************     //
/* Check for sufficient compiler version */
#if defined(__GNUC__) || defined(__GNUG__)
    #if !(__GNUC__ >= 4 && __GNUC_MINOR__ >= 9)
        #error "Insufficient GNU compiler version to install this library-- 4.9 or greater required"
    #endif
#else
    #warning "Currently only GNU compilers are supported and tested, but go ahead if you know what you're doing."
#endif

/*----------------------------------------------
                 Fatal Error
    Aborts runtime with location and _error
    text if called.
-----------------------------------------------*/
#define itrnlFatalError(_error)                            \
{                                                     \
    std::stringstream _location,_message;             \
    _location << __FILE__ << ":" << __LINE__;         \
    _message << "Error -- "+_error.str();             \
    std::cerr << "File "                              \
              << _location.str() << "\n"              \
              << _message.str() << "\n"               \
              << "Aborting!" << "\n";                 \
    exit(EXIT_FAILURE);                               \
};

/*----------------------------------------------
             Standard Error
    Checks for if an input string is empty,
    if it is pass the string to FatalError.
-----------------------------------------------*/
#define itrnlErrorcatch(_errchk)                             \
{                                                            \
    if(!_errchk.empty())                                     \
    {                                                        \
        std::stringstream _error;                            \
        _error << std::string(_errchk);                      \
        itrnlFatalError(_error);                                  \
    }                                                        \
};

/*----------------------------------------------
               Throw Exception
    Pass the macro a char string with an error
    and throw a std::string exception.
-----------------------------------------------*/
#define itrnlThrowException(_errstr)                              \
{                                                            \
    if (!std::string(_errstr).empty())                       \
    {                                                        \
        throw std::string("In Function: ")                   \
            + std::string(__FUNCTION__)                      \
            + std::string("() -- ")                          \
            + std::string(_errstr);                          \
    }                                                        \
};

// Random
#include "randnormflt.h"
#include <fstream>
#include <regex>
#include <utility>

// GLM Vector Math
//#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

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

/*--------Internal Coordinates Type----------


This type stores the indexes for the bonds,
angles and dihedrals along with the values
of the coordinates and types of atoms in
the molecule.
----------------------------------------------*/
class t_iCoords {
public:
    std::vector<bndindex> bidx; // Bonding index
    std::vector<angindex> aidx; // Angle index
    std::vector<dhlindex> didx; // Dihedral index

    std::vector<std::string> type; // Atom Types

    std::vector<float> bnds; // Storage for bonds
    std::vector<float> angs; // Storage for angles
    std::vector<float> dhls; // Storage for dihedrals

    void clear () {
        bidx.clear();aidx.clear();didx.clear();
        type.clear();
        bnds.clear();angs.clear();dhls.clear();
    };
};

/*--------Internal Coordinates RandRng----------


This type stores the Ranges of the random
purturbations of the IC.

----------------------------------------------*/
class t_ICRandRng {

    bool rset;

    std::vector< std::pair<float,float> > rngb; // Range of bonds
    std::vector< std::pair<float,float> > rnga; // Range of angles
    std::vector< std::pair<float,float> > rngd; // Range of dihedrals

public:

    t_ICRandRng () :
        rset(false)
    {};

    // Returns true if the range values are set.
    bool isset() {return rset;};

    // Const Access Functions
    const std::pair<float,float>& getRngBnd(unsigned i) {return rngb[i];};
    const std::pair<float,float>& getRngAng(unsigned i) {return rnga[i];};
    const std::pair<float,float>& getRngDhl(unsigned i) {return rngd[i];};

    // Function for defining the random ranges
    void setRandomRanges (std::vector< std::string > &rngin);

    void clear () {
        rnga.clear();
        rngb.clear();
        rngd.clear();
    };
};

/*--------Internal Coordinates RandRng----------


This type stores the Ranges of the random
purturbations of the IC.

----------------------------------------------*/
class t_ICScanRng {

    bool sset;
    unsigned scnt;

    std::vector< std::pair<float,float> > rngb; // Range of bonds
    std::vector< std::pair<float,float> > rnga; // Range of angles
    std::vector< std::pair<float,float> > rngd; // Range of dihedrals

public:

    t_ICScanRng () :
        sset(false),scnt(0)
    {};

    // Returns true if the range values are set.
    bool isset() {return sset;};

    // Returns the scan counter and increment it by one.
    unsigned getCounter() {
        unsigned tmp(scnt);
        ++scnt;
        return tmp;
    };

    // Const Access Functions
    const std::pair<float,float>& getRngBnd(unsigned i) {return rngb[i];};
    const std::pair<float,float>& getRngAng(unsigned i) {return rnga[i];};
    const std::pair<float,float>& getRngDhl(unsigned i) {return rngd[i];};

    // Function for defining the scan ranges
    void setScanRanges (std::vector< std::string > &scnin);

    void clear () {
        rnga.clear();
        rngb.clear();
        rngd.clear();
    };
};

/*--------Internal Coordinates Class----------


This class stores the indexes for the bonds,
angles and dihedrals of the molecule.
----------------------------------------------*/
class Internalcoordinates {

    t_iCoords iic; // Initial Internal Coordinates
    t_ICRandRng rrg; // Random Range Container
    t_ICScanRng srg; // Scan Range Container


    /** Member Fucntions **/
    // Calculate the bonding index
    void m_calculateBondIndex(const std::vector< glm::ivec2 > &mbond);

    // Calculate the angle index
    void m_calculateAngleIndex();

    // Calculate the dihedral index
    void m_calculateDihedralIndex();

    // Get atom types
    void m_getAtomTypes(const std::vector< std::string > &icoords);

    // Get the bonding index
    void m_getBondIndex(const std::vector< std::string > &icoords);

    // Get the angle index
    void m_getAngleIndex(const std::vector< std::string > &icoords);

    // Get the dihedral index
    void m_getDihedralIndex(const std::vector< std::string > &icoords);

    // Calculate bond lengths
    void m_calculateBonds(const std::vector<glm::vec3> &xyz);

    // Calculate angles
    void m_calculateAngles(const std::vector<glm::vec3> &xyz);

    // Calculate dihedrals
    void m_calculateDihedrals(const std::vector<glm::vec3> &xyz);

    // Create and return Comma Separated Values Internal Coordinates string
    std::string m_createCSVICstring(const std::vector<glm::vec3> &xyz);

    // Calculate the values for the internal coords based on xyz input
    void m_setInternalCoordinatesFromXYZ(const std::vector<glm::vec3> &xyz);

public:

    // Class index constructor
    Internalcoordinates (const std::vector< glm::ivec2 > &mbond) {
        try {
            /* Determing Internal Coords */
            m_calculateBondIndex(mbond);
            m_calculateAngleIndex();
            m_calculateDihedralIndex();

        } catch (std::string error) itrnlErrorcatch(error);
    };

    // Class index and initial iternals constructor
    Internalcoordinates (const std::vector< glm::ivec2 > &mbond,const std::vector<glm::vec3> &ixyz) {
        try {
            /* Determing (IC) Internal Coords Index */
            m_calculateBondIndex(mbond);
            m_calculateAngleIndex();
            m_calculateDihedralIndex();

            /* Calculate and store the initial IC */
            m_setInternalCoordinatesFromXYZ(ixyz);

        } catch (std::string error) itrnlErrorcatch(error);
    };

    // Class index and initial iternals constructor
    Internalcoordinates (const std::vector< std::string > &icoords) {
        try {
            /* Determing (IC) Internal Coords Index */
            m_getAtomTypes(icoords);
            m_getBondIndex(icoords);
            m_getAngleIndex(icoords);
            m_getDihedralIndex(icoords);

        } catch (std::string error) itrnlErrorcatch(error);
    };

    // Calculate the CSV (Comma Separated Values) string of internal coords based on xyz input
    std::string calculateCSVInternalCoordinates(const std::vector<glm::vec3> &xyz);

    // Generate a random structure
    t_iCoords generateRandomICoords(RandomReal &rnGen);

    // Generate a random structure
    t_iCoords generateScanICoords();

    // Data Printer
    void printdata() {
        std::cout << "Internal Coordinates Class Setup" << std::endl;
        std::cout << "Bonds: " << iic.bidx.size() << std::endl;
        std::cout << "Angles: " << iic.aidx.size() << std::endl;
        std::cout << "Dihedrals: " << iic.didx.size() << std::endl;
        std::cout << std::endl;
    };

    t_iCoords& getiCoords() {
        return iic;
    }

    t_ICRandRng& getRandRng() {
        return rrg;
    }

    t_ICScanRng& getScanRng() {
        return srg;
    }

    // Destructor
    ~Internalcoordinates() {
        iic.clear();
    };
};

// Produce a Z-mat string from input internal coordinates
extern void iCoordToZMat(const t_iCoords &ics,std::string &zmats);

// Calculate the CSV (Comma Separated Values) string of internal coords based on xyz input
extern std::string getCsvICoordStr(const t_iCoords &ics, std::string units="degrees");

// Convert internal coordinates to XYZ
extern void iCoordToXYZ(const t_iCoords &ics,std::vector<glm::vec3> &xyz);
};
#endif
