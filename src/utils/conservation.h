//
// Created by fury on 8/13/15.
//

#ifndef G09DNNTSBUILDER_CONSERVATION_H
#define G09DNNTSBUILDER_CONSERVATION_H

#include <vector>
#include <Eigen/Dense>
#include <glm/glm.hpp>

class conservation {

public:

    conservation(std::vector<glm::vec3> &xyz, const std::vector<double> &m);
    void conserve(std::vector<glm::vec3> &xyz_n);

private:
    Eigen::MatrixXd coord_read(const std::vector<glm::vec3> &xyz);
    std::vector<glm::vec3> matrix_read(const Eigen::MatrixXd &M);
    void zero_round(Eigen::MatrixXd &M);
    void normalize(Eigen::VectorXd &V);
    void move_center(Eigen::MatrixXd &A);
    Eigen::Matrix3d inertia_tensor(const Eigen::MatrixXd &M);


    Eigen::Matrix3d _IX;
    std::vector<double> _m; // Mass of each atom
    int _N;  // Number of atoms
};


#endif //G09DNNTSBUILDER_CONSERVATION_H
