//
// Created by fury on 8/13/15.
//

#ifndef G09DNNTSBUILDER_CONSERVATION_H
#define G09DNNTSBUILDER_CONSERVATION_H

#include <vector>
#include <armadillo>
#include <glm/glm.hpp>

class conservation {

public:

    conservation(std::vector<glm::vec3> &xyz, const std::vector<double> &m);
    void conserve(std::vector<glm::vec3> &xyz_n);

private:
    arma::mat coord_read(const std::vector<glm::vec3> &xyz);
    std::vector<glm::vec3> matrix_read(const arma::mat &M);
    void zero_round(arma::mat &M);
    void normalize(arma::vec &V);
    void move_center(arma::mat &A);
    arma::mat inertia_tensor(const arma::mat &M);


    arma::mat _IX;
    std::vector<double> _m; // Mass of each atom
    int _N;  // Number of atoms
};


#endif //G09DNNTSBUILDER_CONSERVATION_H
