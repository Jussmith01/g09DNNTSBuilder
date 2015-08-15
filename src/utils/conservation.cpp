//
// Created by Dustin on 8/13/15.
//

#include "conservation.h"

conservation::conservation(std::vector<glm::vec3> &xyz, const std::vector<double> &m) : _m(m) {
    using namespace Eigen;
    using namespace std;
    _N = static_cast<int>(m.size());
    /* Importing the coordinates into an armadillo matrix, note that each row is an atom */
    MatrixXd X = coord_read(xyz);
    xyz = matrix_read(X);
    move_center(X);
    Matrix3d I = inertia_tensor(X);
    _IX = I * X.transpose();
}

void conservation::zero_round(Eigen::MatrixXd &M) {
    for (auto i = 0; i < M.rows(); ++i)
    {
        for (auto j = 0; j != M.cols(); ++j)
        {
            if (M(i, j) < 1e-10 && M(i, j) > -1e-10)
            {
                M(i, j) = 0.0;
            }
        }
    }
}

/************** Normalizer ***********************/
/* Simply makes a unit vector out of an armadillo
 * vector */
void conservation::normalize(Eigen::VectorXd &V) {
    double normal = 0;
    for (auto i = 0; i < V.rows(); ++i)
    {
        normal += pow(V(i), 2);
    }
    normal = pow(normal, 0.5);
    for (auto i = 0; i < V.rows(); ++i)
    {
        V(i) /= normal;
    }
}

/************** Move to origin ******************/
/* Takes parameter matrix of coordinates (in row form)
 * and a vector of masses, to yield the a row vector of
 * the center of mass. */
void conservation::move_center(Eigen::MatrixXd &A) {
    using namespace Eigen;
    using namespace std;
    double total_mass = 0;
    for (auto i : _m)
    {
        total_mass += i;
    }
    RowVector3d v_center_mass;
    for (int i = 0; i != 3; ++i)
    {
        double coord_center = 0;
        for (auto j = 0; j != _N; ++j)
        {
            coord_center += (A(j, i) / total_mass) * _m[j];
        }
        v_center_mass(i) = coord_center;
    }
    for (int i = 0; i != _N; ++i)
    {
        A.row(i) = A.row(i) - v_center_mass;
    }
    zero_round(A);
}

/**************** Inertia Tensor *****************/
/* Takes parameters matrix of coordinates (in row form)
 * and a vector of masses, to yield the inertia tensor */
Eigen::Matrix3d conservation::inertia_tensor(const Eigen::MatrixXd &M) {
    using namespace Eigen;
    double _I_xx = 0;
    double _I_yy = 0;
    double _I_zz = 0;
    double _I_xy = 0;
    double _I_xz = 0;
    double _I_yz = 0;
    for (int i = 0; i != _N; ++i)
    {
        _I_xx += (pow(M(i, 1), 2) + pow(M(i, 2), 2)) * _m[i];
    }
    for (int i = 0; i != _N; ++i)
    {
        _I_yy += (pow(M(i, 0), 2) + pow(M(i, 1), 2)) * _m[i];
    }
    for (int i = 0; i != _N; ++i)
    {
        _I_zz += (pow(M(i, 0), 2) + pow(M(i, 1), 2)) * _m[i];
    }
    for (int i = 0; i != _N; ++i)
    {
        _I_xy -= M(i, 0) * M(i, 1);
    }
    for (int i = 0; i != _N; ++i)
    {
        _I_xz -= M(i, 0) * M(i, 2);
    }
    for (int i = 0; i != _N; ++i)
    {
        _I_yz -= M(i, 1) * M(i, 2);
    }
    // Create the matrix
    Matrix3d I;
    I << _I_xx , _I_xy , _I_xz,
         _I_xy , _I_yy , _I_yz,
         _I_xz , _I_yz , _I_zz;
    return I;
}

Eigen::MatrixXd conservation::coord_read(const std::vector<glm::vec3> &xyz) {
    using namespace Eigen;
    MatrixXd X(_N, 3);
    for (int i = 0; i != _N; ++i)
    {
        X(i,0) = xyz[i].x;
        X(i,1) = xyz[i].y;
        X(i,2) = xyz[i].z;
    }
    return X;
}

/**************** Conserve ***********************/
/* This is the primary function of the class. It take a reference of coordinates
 * and manipulates them in such as way as to converve angular and translational
 * momentum */
void conservation::conserve(std::vector<glm::vec3> &xyz_n) {
    using namespace Eigen;
    using namespace std;
    MatrixXd Y = coord_read(xyz_n);
    move_center(Y);
    Matrix3d I = inertia_tensor(Y);
    MatrixXd IY = I * Y.transpose();
    MatrixXd R_t = IY.colPivHouseholderQr().solve(_IX);
    Y = R_t.transpose() * Y;
    xyz_n = matrix_read(Y);
}

std::vector<glm::vec3> conservation::matrix_read(const Eigen::MatrixXd &M) {
    using namespace Eigen;
    using namespace std;
    vector<glm::vec3> xyz_temp(3);
    for (int i=0; i!=_N; ++i)
    {
        xyz_temp[i].x = static_cast<float>(M(i,0));
        xyz_temp[i].y = static_cast<float>(M(i,1));
        xyz_temp[i].z = static_cast<float>(M(i,2));
    }
    return xyz_temp;
}


