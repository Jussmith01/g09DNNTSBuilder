//
// Created by Justin Smith and Dustin Tracy on 7/5/15.
//

#ifndef SIMPLE_TOOLS_HPP
#define SIMPLE_TOOLS_HPP

#include <cmath>
#include <iomanip>

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
    float theta = atan2(cart.y,cart.x);
    float phi = acos(cart.z/r);
    //std::cout << "x: " << cart.x << " y: " << cart.y << " z: " << cart.z << std::endl;
    //std::cout << "r: " << r << " theta: " << theta << " phi: " << phi << std::endl;
    //std::cout << std::endl;
    sphr.x = r;
    sphr.y = theta;
    sphr.z = phi;
};

inline void frcsphereConvert(glm::vec3 &sphr,const glm::vec3 &cart) {
    float r;
    float theta;
    float phi;

    if (cart.z>=0) {
        r = glm::length(cart);
        theta = atan2(cart.x,cart.z);
        phi = atan2(cart.y,cart.z);
    } else {
        r = -glm::length(cart);
        theta = M_PI-atan2(cart.x,cart.z);
        phi = M_PI-atan2(cart.y,cart.z);
    }

    //std::cout << "x: " << cart.x << " y: " << cart.y << " z: " << cart.z << std::endl;
    //std::cout << "r: " << r << " theta: " << theta << " phi: " << phi << std::endl;
    //std::cout << std::endl;
    sphr.x = r;
    sphr.y = theta;
    sphr.z = phi;
};

inline void vec3nanchk(glm::vec3 &vec) {
    if (isnan(vec.x) || isnan(vec.y) || isnan(vec.z)) {
        vec.x=1.0f;
        vec.y=0.0f;
        vec.z=0.0f;
    }
};

inline void setZero(glm::vec3 &vec) {
    if (fabs(vec.x)<5.0E-7) {
        vec.x=0.0f;
    }
    if (fabs(vec.y)<5.0E-7) {
        vec.y=0.0f;
    }
    if (fabs(vec.z)<5.0E-7) {
        vec.z=0.0f;
    }
};

inline void standardOrient(unsigned center,unsigned atom1,unsigned atom2,std::vector<glm::vec3> &tfrce,std::vector<glm::vec3> &tcart) {
    glm::mat4 trans;
    trans = glm::translate(trans,tcart[center]); // Add translation

    for (auto&& c : tcart)
        c = glm::vec3(trans*glm::vec4(c.x,c.y,c.z,1.0f));

    /*std::cout <<  "|----ORIENT----|" << std::endl;
    std::cout.setf( std::ios::fixed, std::ios::floatfield );
    std::cout << std::setprecision(7) <<  "fvector1: " << tfrce[center].x << "," << tfrce[center].y << "," << tfrce[center].z << std::endl;
    std::cout << std::setprecision(7) <<  "vector1: " << tcart[atom1].x << "," << tcart[atom1].y << "," << tcart[atom1].z << std::endl;
    std::cout << std::setprecision(7) <<  "vector2: " << tcart[atom2].x << "," << tcart[atom2].y << "," << tcart[atom2].z << "\n" << std::endl;
    */

    glm::vec3 za(0.0f,0.0f,1.0f);
    float anglea = (-glm::dot(za,glm::normalize(tcart[atom1]))+1.0f)*(M_PI/2.0f);
    glm::vec3 ax1 = glm::normalize(glm::cross(glm::normalize(za),glm::normalize(tcart[atom1])));
    vec3nanchk(ax1);
    glm::mat4 rota;
    rota = glm::rotate(rota,anglea,ax1);

    /*std::cout << std::setprecision(7) <<  "dot xza: " << anglea << std::endl;
    std::cout << std::setprecision(7) <<  "rot axi: " << ax1.x << "," << ax1.y << "," << ax1.z << "\n" << std::endl;
    */

    for (auto&& c : tcart) {
        c = glm::vec3(rota*glm::vec4(c.x,c.y,c.z,1.0f));
        setZero(c);
    }

    for (auto&& c : tfrce) {
        c = glm::vec3(rota*glm::vec4(c.x,c.y,c.z,1.0f));
        setZero(c);
    }

    glm::vec2 zb(1.0f,0.0f);
    float angleb = (-glm::dot(zb,glm::normalize(glm::vec2(tcart[atom2].x,tcart[atom2].y)))+1.0f)*(M_PI/2.0f);
    glm::vec3 ax2 = glm::vec3(0.0f,0.0f,1.0f);
    glm::mat4 rotb;
    rotb = glm::rotate(rotb,angleb,ax2);

    for (auto&& c : tcart) {
        c = glm::vec3(rotb*glm::vec4(c.x,c.y,c.z,1.0f));
        setZero(c);
    }

    for (auto&& c : tfrce) {
        c = glm::vec3(rotb*glm::vec4(c.x,c.y,c.z,1.0f));
        setZero(c);
    }

    /*std::cout << std::setprecision(7) <<  "dot xza: " << angleb << std::endl;
    std::cout << std::setprecision(7) <<  "rot axi: " << ax2.x << "," << ax2.y << "," << ax2.z << "\n" << std::endl;

    std::cout << std::setprecision(7) <<  "fvector1: " << tfrce[center].x << "," << tfrce[center].y << "," << tfrce[center].z << std::endl;
    std::cout << std::setprecision(7) <<  "pvector1: " << tcart[atom1].x << "," << tcart[atom1].y << "," << tcart[atom1].z << std::endl;
    std::cout << std::setprecision(7) <<  "pvector2: " << tcart[atom2].x << "," << tcart[atom2].y << "," << tcart[atom2].z << std::endl;

    std::cout <<  "|--------------|" << std::endl;
    */
};

inline std::string cartesianToStandardSpherical(unsigned center,unsigned atom1,unsigned atom2,std::vector<glm::vec3> &tfrce,std::vector<glm::vec3> &tcart) {

    standardOrient(center,atom1,atom2,tfrce,tcart);

    std::vector<glm::vec3> spherepos(tcart.size()-1);

    glm::vec3 cforcesphere;
    //std::cout <<  "\nForce Spherical" << std::endl;
    frcsphereConvert(cforcesphere,tfrce[center]);

    /*std::cout << "CENTER: \n";
    for (auto&& i : tcart)
        std::cout << i.x << "," << i.y << "," << i.z << "," << std::endl;
    */

    unsigned k(0);
    //std::cout <<  "Coord Spherical" << std::endl;
    for (unsigned i=0;i<tcart.size();++i) {
        if (i != center) {
            sphereConvert(spherepos[k],tcart[i]);
            ++k;
        }
    }

    /*for (auto&& i : spherepos)
        std::cout << i.x << "," << i.y << "," << i.z << "," << std::endl;
    */

    k=0;
    std::stringstream sphrcsv;
    sphrcsv.setf( std::ios::fixed, std::ios::floatfield );
    //sphrcsv << "ATMS," << spherepos.size() << ",";
    for (auto&& i : spherepos) {
            sphrcsv << std::setprecision(7) << i.x << "," << i.y << "," << i.z << ",";
    }

    //sphrcsv << "ATMX," << tcart.size() << ",";
    /*for (auto&& i : tcart) {
            sphrcsv << std::setprecision(7) << i.x << "," << i.y << "," << i.z << ",";
    }*/

    //if (cforcesphere.y>1.57079633)
    //    cforcesphere = glm::vec3(cforcesphere.x,0.0f,cforcesphere.z));

    sphrcsv << std::setprecision(7) << cforcesphere.x << "," << cforcesphere.y << "," << cforcesphere.z << ",";
    //sphrcsv << std::setprecision(7) << cforcesphere.x << "," << cforcesphere.y << "," << cforcesphere.z << ",";
    sphrcsv << std::setprecision(7) << tfrce[center].x << "," << tfrce[center].y << "," << tfrce[center].z << ",";
    return sphrcsv.str();
};

/*---------String Is Type-------------

Checks if a string is of the type given.
Currently supports float,unsigned and
ints.

--------------------------------------*/
inline bool stristype(std::string value,std::string type) {

    std::vector<char> cs;
    if (type.compare("float")==0) {
        cs = {'0','1','2','3','4','5','6','7','8','9','-','+','.'};
        if (value.find(".")==std::string::npos) {
            return false;
        }
    } else if (type.compare("unsigned")==0) {
        cs = {'0','1','2','3','4','5','6','7','8','9','+'};
    } else if (type.compare("int")==0) {
        cs = {'0','1','2','3','4','5','6','7','8','9','-','+'};
    } else if (type.compare("bool")==0) {
        cs = {'0','1'};
    }

    unsigned match(0);
    for (auto& c : value) {
        for (auto& cc : cs) {
            if (c == cc)
                ++match;
        }

        if (match == 0) {
            return false;
        }

        match = 0;
    }

    return true;
};

inline std::string calculateDistMatrixCSV(std::vector< glm::vec3 > &vec) {
    std::stringstream dmat_ss;
    dmat_ss.setf( std::ios::fixed, std::ios::floatfield );

    unsigned N(vec.size());
    unsigned cnt(0);
    for (unsigned i=0;i<N;++i) {
        for (unsigned j=i+1;j<N;++j) {
            dmat_ss << std::setprecision(7) << glm::length(vec[i]-vec[j]) << ",";
            ++cnt;
        }
    }

    return dmat_ss.str();
};

};

#endif
