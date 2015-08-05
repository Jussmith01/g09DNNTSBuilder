#ifndef INTERNALS_H
#define INTERNALS_H

namespace itrnl
{

/*----------------------------
           Bond Index

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
struct bndindex {int v1,v2;} __attribute__ ((__aligned__(8)));

// Function for creating bond indexes
inline bndindex CreateBondIndex(const int v1,const int v2)
{
    bndindex tmpidx;
    tmpidx.v1=v1;
    tmpidx.v2=v2;
    return tmpidx;
};

// Function for less than comparing bndindex types -- for use by std::sort
inline bool bndComparelt (bndindex i,bndindex j) { return (i.v1<j.v1); }

// Function for equality comparing bndindex types
inline bool bndCompareeq (bndindex i,bndindex j) { return ((i.v1==j.v1) && (i.v2==j.v2)); }

/*----------------------------
        Angle Index

Consists of:
angindex - 8 byte aligned
           container for 3
           integer indicies.

CreateAngleIndex() - A function
           that produces a
           angindex type from
           3 values.
------------------------------*/
// 8 byte alignment forced to prevent cache mis-alignment
struct angindex {int v1,v2,v3;} __attribute__ ((__aligned__(8)));

inline angindex CreateAngleIndex(const int v1,const int v2,const int v3)
{
    angindex tmpidx;
    tmpidx.v1=v1;
    tmpidx.v2=v2;
    tmpidx.v3=v3;
    return tmpidx;
};

/*-----------------------------
         Dihedral Index

Consists of:
dhlindex - 8 byte aligned
           container for 4
           integer indicies.

CreateDihedralIndex() - A
           function that
           produces a dhlindex
           type from 4 values.
------------------------------*/
// 8 byte alignment forced to prevent cache mis-alignment
struct dhlindex {int v1,v2,v3,v4;} __attribute__ ((__aligned__(8)));

inline dhlindex CreateDihedralIndex(const int v1,const int v2,const int v3,const int v4)
{
    dhlindex tmpidx;
    tmpidx.v1=v1;
    tmpidx.v2=v2;
    tmpidx.v3=v3;
    tmpidx.v4=v4;
    return tmpidx;
};


/*---------------------------
 Internal Coordinates Class

This class stores the indexes
for the bonds, angles and
dihedrals of the molecule.
---------------------------*/
class Internalcoordinates
{
    std::vector<bndindex> bidx; // Bonding index
    std::vector<angindex> aidx; // Angle index
    std::vector<dhlindex> didx; // Dihedral index

    Internalcoordinates () {}; // Private default constructor

    // Calculate the bonding index
    void m_calculateBondIndex(std::vector< std::pair<int,int> > &mbond);

    // Calculate the angle index
    void m_calculateAngleIndex();

    // Calculate the dihedral index
    void m_calculateDihedralIndex();

public:

    Internalcoordinates (std::vector< std::pair<int,int> > &mbond)
    {
        m_calculateBondIndex(mbond);
        m_calculateAngleIndex();
        m_calculateDihedralIndex();
    };
};

};
#endif
