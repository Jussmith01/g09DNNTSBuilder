//
// Created by Justin Smith and Dustin Tracy on 7/5/15.
//

#ifndef SIMPLE_TOOLS_HPP
#define SIMPLE_TOOLS_HPP

// GLM Mathematics
#include <glm/glm.hpp>
#include <cmath>
#include <iomanip>

/*   The Simple Tools Namespace   */
namespace simtls {
/*----------------------------------------
    Trim Whitespace from String Function
------------------------------------------*/
inline std::string trim (std::string line) {
    size_t first = line.find_first_not_of(" \t\r\n\x0b");
    if (first!=std::string::npos) // first should always return npos if space only contains white space or is empty
        return line.substr(first,line.find_last_not_of(" \t\r\n\x0b")+1-first);
    else return std::string("");
};

/*----------------------------------------
        Test if a file is present
------------------------------------------*/
inline std::string fileTest(std::string filename) {
    std::ifstream test;
    test.open(filename.c_str());
    if (!test.is_open()) {
        std::stringstream err;
        err << "Cannot open file: " << filename;
        return err.str();
    }
    test.close();

    return "";
};

inline void recursiveBuild(const std::vector<unsigned> &in,std::vector<unsigned> &out) {
    for (unsigned i=1; i<=in.size();++i) {
        out[i-1] = 0;
        for (unsigned j=0; j<i;++j) {
            out[i-1]+=in[j];
        }
    }
};

inline int countUnique(unsigned na,unsigned nt) {
    unsigned cnt(0);

    std::vector<unsigned> wk1(nt);
    std::vector<unsigned> wk2(nt);

    for (unsigned i = 1;i<=nt;++i) {
        wk1[i-1] = i;
    }

    for (unsigned i = 0;i<na-2;++i) {
        recursiveBuild(wk1,wk2);
        wk1 = wk2;
    }

    for (auto&& i : wk1) {
        cnt += i;
    }

    return cnt;
};

inline void sphereConvert(glm::vec3 &sphr,const glm::vec3 &cart) {
    float r = glm::length(cart);
    float phi = acos(cart.x/r);
    float theta = asin(cart.y/(r*sin(phi)));
    std::cout << "x: " << cart.x << " y: " << cart.y << " z: " << cart.z << std::endl;
    std::cout << "r: " << r << " theta: " << theta << " phi: " << phi << std::endl;
    std::cout << std::endl;
    sphr.x = r;
    sphr.y = theta;
    sphr.z = phi;
};

inline void centerPositions(const glm::vec3 &cart,std::vector<glm::vec3> &tcart) {
    for (auto&& i : tcart)
        i = i-cart;
};

inline std::string cartesianToCenteredSpherical(unsigned center,const std::vector<glm::vec3> &tfrce,std::vector<glm::vec3> &tcart) {
    glm::vec3 translation(tcart[center]);
    centerPositions(translation,tcart);

    std::vector<glm::vec3> spherepos(tcart.size()-1);

    glm::vec3 cforcesphere;
    sphereConvert(cforcesphere,tfrce[center]);

    std::cout << "CENTER: \n";
    for (auto&& i : tcart)
        std::cout << i.x << "," << i.y << "," << i.z << "," << std::endl;

    unsigned k(0);
    for (unsigned i=0;i<tcart.size();++i) {
        if (i != center) {
            std::cout << "i: " << i << " k: " << k << std::endl;
            sphereConvert(spherepos[k],tcart[i]);
            ++k;
        }
    }

    for (auto&& i : spherepos)
        std::cout << i.x << "," << i.y << "," << i.z << "," << std::endl;

    k=0;
    std::stringstream sphrcsv;
    sphrcsv.setf( std::ios::fixed, std::ios::floatfield );
    sphrcsv << "ATM," << spherepos.size() << ",";
    for (auto&& i : spherepos) {
            sphrcsv << std::setprecision(7) << i.x << "," << i.y << "," << i.z << ",";
    }

    sphrcsv << std::setprecision(7) << "FRC," << cforcesphere.x << "," << cforcesphere.y << "," << cforcesphere.z << ",";
    return sphrcsv.str();
};

};

#endif
