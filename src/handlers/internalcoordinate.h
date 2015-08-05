#ifndef INTERNALS_H
#define INTERNALS_H

namespace itrnl
{

class Internalcoordinates
{
    std::vector<int[2]> bidx; // Bonding index
    std::vector<int[3]> aidx;
    std::vector<int[4]> didx;

    Internalcoordinates () {};

    void m_calculateInternalIndex(std::vector< std::pair<int,int> > &mbond);
    void m_calculateBondIndex(std::vector< std::pair<int,int> > &mbond);
    void m_calculateAngleIndex();
    void m_calculateDihedralIndex();

public:

    Internalcoordinates (std::vector< std::pair<int,int> > &mbond)
    {
        m_calculateInternalIndex(mbond);
    };

};

};
#endif
