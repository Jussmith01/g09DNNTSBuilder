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
    void move_center(Eigen::MatrixXd &A);
    Eigen::Vector3d unit(const Eigen::Vector3d &v);
    Eigen::Matrix3d inertia_tensor(const Eigen::MatrixXd &M);
    Eigen::Matrix3d rotation_matrix(const Eigen::Vector3d &v1, const Eigen::Vector3d &v2);
    std::vector<Eigen::Vector3d> determine_vectors(const Eigen::Matrix3d X);
    Eigen::Vector3d determine_init_plane_vector(const std::vector<Eigen::Vector3d> &V);
    Eigen::Vector3d determine_init_inertia_vector(const std::vector<Eigen::Vector3d> &V);


    Eigen::Vector3d X_plane;
    Eigen::Vector3d X_inert;
    Eigen::Matrix3d _IX;
    std::vector<double> _m; // Mass of each atom
    int _N;  // Number of atoms
};


#endif //G09DNNTSBUILDER_CONSERVATION_H
