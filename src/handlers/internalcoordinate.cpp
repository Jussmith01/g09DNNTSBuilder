// Standary Library
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include <vector>
#include <algorithm>

// Error Handlers
#include "../errorhandlers.h"

// Namespace header
#include "internalcoordinate.h"

/*---------------------------
    Store the bond index
---------------------------*/
void itrnl::Internalcoordinates::m_calculateBondIndex(std::vector< std::pair<int,int> > &mbond)
{
    bidx.reserve(mbond.size());
    for (auto && bnd : mbond)
    {
        int first(bnd.first);
        int second(bnd.second);

        // Some error checking
        if (first==second)
            throwException("Self bonding detected. Check the input file.");
        if (first<0||second<0)
            throwException("Negative atom index detected. Check the input file.");

        // Order the atoms by atom number
        if (first < second)
            bidx.push_back(itrnl::CreateBondIndex(bnd.first,bnd.second));
        else
            bidx.push_back(itrnl::CreateBondIndex(bnd.second,bnd.first));

        // Check that bond is unique
        for (int i=0;i<int(bidx.size())-1;++i)
        {
            if (itrnl::bndCompareeq(bidx.back(),bidx[i]))
                throwException("Duplicate bond detected. Check the input file.");
        }
    }

    // Sort by the first element from lowest to highest
    std::sort(bidx.begin(),bidx.end(),itrnl::bndComparelt);

    for (auto && bi : bidx)
    {
        std::cout << bi.v1 << ":" << bi.v2 << std::endl;
    }
};

/*---------------------------
 Calculate the Angle index
---------------------------*/
void itrnl::Internalcoordinates::m_calculateAngleIndex()
{

};

/*---------------------------
Calculate the Dihedral index
---------------------------*/
void itrnl::Internalcoordinates::m_calculateDihedralIndex()
{

};
