// Standary Library
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include <vector>

// Error Handlers
#include "../errorhandlers.h"

// Namespace header
#include "internalcoordinate.h"

void itrnl::Internalcoordinates::m_calculateInternalIndex(std::vector< std::pair<int,int> > &mbond)
{
    m_calculateBondIndex(mbond);
    m_calculateAngleIndex();
    m_calculateDihedralIndex();
};

void itrnl::Internalcoordinates::m_calculateBondIndex(std::vector< std::pair<int,int> > &mbond)
{

};

void itrnl::Internalcoordinates::m_calculateAngleIndex()
{

};

void itrnl::Internalcoordinates::m_calculateDihedralIndex()
{

};
