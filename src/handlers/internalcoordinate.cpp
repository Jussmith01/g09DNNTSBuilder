// Standary Library
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <cmath>
#include <iomanip>

// Error Handlers
#include "../errorhandlers.h"

// GLM Vector Mathematics
#include <glm/glm.hpp>

// Namespace header
#include "internalcoordinate.h"

#define rad 180.0/M_PI; /*radians to degrees*/

/*---------Store the bond index------------

------------------------------------------*/
void itrnl::Internalcoordinates::m_calculateBondIndex(const std::vector< glm::ivec2 > &mbond) {

    bidx.reserve(mbond.size());
    for (auto && bnd : mbond) {
        int first(bnd.x);
        int second(bnd.y);

        // Some error checking
        if (first==second)
            throwException("Self bonding detected. Check the input file.");
        if (first<0||second<0)
            throwException("Negative atom index detected. Check the input file.");

        // Order the atoms by atom number
        if (first < second)
            bidx.push_back(itrnl::CreateBondIndex(first,second));
        else
            bidx.push_back(itrnl::CreateBondIndex(second,first));

        // Check that bond is unique
        for (int i=0; i<int(bidx.size())-1; ++i) {
            if (itrnl::bndCompareeq(bidx.back(),bidx[i]))
                throwException("Duplicate bond detected. Check the input file.");
        }
    }

    // Sort by the first element from lowest to highest
    std::sort(bidx.begin(),bidx.end(),itrnl::bndComparelt);
};

/*---------Calculate the Angle index-----------

Requires the bond index bidx to be populated.
----------------------------------------------*/
void itrnl::Internalcoordinates::m_calculateAngleIndex() {
    if (bidx.empty()) throwException("Cannot calculate angles; Bonds have not been defined");

    for (uint32_t i=0; i<bidx.size(); ++i) {
        for (uint32_t j=i+1; j<bidx.size(); ++j) {
            if (bidx[j].v1==bidx[i].v1)
                aidx.push_back(itrnl::CreateAngleIndex(bidx[i].v1,bidx[i].v2,bidx[j].v2));

            if (bidx[j].v1>bidx[i].v1) {
                break;
            }
        }

        for (uint32_t j=i+1; j<bidx.size(); ++j) {
            if (bidx[j].v1==bidx[i].v2)
                aidx.push_back(itrnl::CreateAngleIndex(bidx[i].v2,bidx[i].v1,bidx[j].v2));

            if (bidx[j].v1>bidx[i].v2) {
                break;
            }
        }
    }
};

/*----------Calculate the Dihedral index------------


Requires the angle index aidx to be populated.
----------------------------------------------------*/
void itrnl::Internalcoordinates::m_calculateDihedralIndex() {
    //if (aidx.empty()) throwException("Cannot calculate dihedrals; Angles have not been defined");

    for (uint32_t i=0; i<aidx.size(); ++i) {
        if(aidx[i].v2 > aidx[i].v1) {
            for (uint32_t j=i+1; j<aidx.size(); ++j) {
                if (aidx[j].v1==aidx[i].v2) {
                    if (aidx[j].v2==aidx[i].v1) {
                        didx.push_back(itrnl::CreateDihedralIndex(aidx[i].v3,aidx[i].v1,aidx[j].v1,aidx[j].v3));
                    }

                    if (aidx[j].v3==aidx[i].v1) {
                        didx.push_back(itrnl::CreateDihedralIndex(aidx[i].v3,aidx[i].v1,aidx[j].v1,aidx[j].v2));
                    }
                }
            }
        }

        if(aidx[i].v3 > aidx[i].v1) {
            for (uint32_t j=i+1; j<aidx.size(); ++j) {
                if (aidx[j].v1==aidx[i].v3) {
                    if (aidx[j].v2==aidx[i].v1) {
                        didx.push_back(itrnl::CreateDihedralIndex(aidx[i].v2,aidx[i].v1,aidx[j].v1,aidx[j].v3));
                    }

                    if (aidx[j].v3==aidx[i].v1) {
                        didx.push_back(itrnl::CreateDihedralIndex(aidx[i].v2,aidx[i].v1,aidx[j].v1,aidx[j].v2));
                    }
                }
            }
        }
    }
};

/*----------Calculate the Bonds------------

Requires the bond index, bidx, to be populated.
------------------------------------------*/
void itrnl::Internalcoordinates::m_calculateBonds(const std::vector<glm::vec3> &xyz) {
    if (bidx.empty()) throwException("Cannot calculate internals without bonds");
    if (xyz.empty()) throwException("Cannot calculate internals without coordinates");

    //std::cout << "Bonds: " << " \n";
    for (uint32_t i=0; i<bidx.size(); ++i) {
        bnds[i] = glm::length(xyz[bidx[i].v1]-xyz[bidx[i].v2]);
        //std::cout << bidx[i].v1 << ":" << bidx[i].v2 << " = " << bnds[i] << std::endl;
    }
};

/*----------Calculate the Angles------------

Calculates the angle.  Returns empty string
if aidx is not populated.
------------------------------------------*/
void itrnl::Internalcoordinates::m_calculateAngles(const std::vector<glm::vec3> &xyz) {
    if (aidx.empty()) return;

    // Calculate angles
    glm::vec3 v1;
    glm::vec3 v2;

    for (uint32_t i=0; i<aidx.size(); ++i) {
        v1 = glm::normalize(xyz[aidx[i].v1]-xyz[aidx[i].v2]);
        v2 = glm::normalize(xyz[aidx[i].v1]-xyz[aidx[i].v3]);

        angs[i] = glm::dot(v1,v2);
        angs[i] = std::acos(angs[i]);
    }

    //std::cout << "Angles: \n";
    //for (uint32_t i=0;i<aidx.size();++i)
    //{
    //   std::cout << aidx[i].v1 << ":" << aidx[i].v2 << ":" << aidx[i].v3 << " = " << angs[i] << std::endl;
    //}
};

/*--------Calculate the Dihedrals----------

Calculates the dihedrals. Returns empty string
if didx is not populated.
------------------------------------------*/
void itrnl::Internalcoordinates::m_calculateDihedrals(const std::vector<glm::vec3> &xyz) {
    if (didx.empty()) return;

    // Calculate Dihedrals
    glm::vec3 b1;
    glm::vec3 b2;
    glm::vec3 b3;

    glm::vec3 n1;
    glm::vec3 n2;

    for (uint32_t i=0; i<didx.size(); ++i) {
        b1 = xyz[didx[i].v1]-xyz[didx[i].v2];
        b2 = xyz[didx[i].v2]-xyz[didx[i].v3];
        b3 = xyz[didx[i].v3]-xyz[didx[i].v4];

        n1 = glm::normalize(glm::cross( b1,b2));
        n2 = glm::normalize(glm::cross(-b3,b2));

        dhls[i] = glm::dot(n1,n2);
        dhls[i] = std::acos(dhls[i]);
    }

    //std::cout << "Dihedrals: \n";
    //for (uint32_t i=0;i<didx.size();++i)
    //{
    //    std::cout << didx[i].v1 << ":" << didx[i].v2 << ":" << didx[i].v3 << ":" << didx[i].v4 << " = " << dhls[i] <<  std::endl;
    //}
};

/*--------Create CVSIC String----------

Create and return Comma Separated Values
Internal Coordinates string. Values put in
the string with the following format:
Nb,Na,Nd,[Bonds CSV],[Angles CSV],[Dihedrals CSV]
------------------------------------------*/
std::string itrnl::Internalcoordinates::m_createCSVICstring(const std::vector<glm::vec3> &xyz) {
    std::stringstream csvint;

    csvint << bnds.size() << "," << angs.size() << "," << dhls.size() << ",";
    csvint.setf( std::ios::fixed, std::ios::floatfield );

    for (auto&& b : bnds)
        csvint << std::setprecision(7) << b << ",";

    for (auto&& a : angs)
        csvint << std::setprecision(7) << a << ",";

    for (auto&& d : dhls)
        csvint << std::setprecision(7) << d << ",";

    for (auto&& c : xyz)
        csvint << std::setprecision(7) << c.x << "," << c.y << "," << c.y << ",";

    std::string rtn(csvint.str());
    //rtn.pop_back();

    return rtn;
};

/*---Calculate IC and Create CVSIC String----

Calculates internal coordinates of an xyz input
based on stored Internal Coordinate (IC) index.
Then creates a Comma Separated Value (CSV)
string of the IC.
------------------------------------------*/
std::string itrnl::Internalcoordinates::calculateCSVInternalCoordinates(const std::vector<glm::vec3> &xyz) {
    try {

        m_calculateBonds(xyz);
        m_calculateAngles(xyz);
        m_calculateDihedrals(xyz);

    } catch (std::string error) dnntsErrorcatch(error);

    return m_createCSVICstring(xyz);;
};


