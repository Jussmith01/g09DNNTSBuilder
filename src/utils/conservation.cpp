//
// Created by Dustin on 8/13/15.
//

#include "conservation.h"

conservation::conservation(std::vector<glm::vec3> &xyz, const std::vector<double> &m) : _m(m) {
    using namespace arma;
    using namespace std;
    _N = static_cast<int>(m.size());
    /* Importing the coordinates into an armadillo matrix, note that each row is an atom */
    mat X = coord_read(xyz);
    xyz = matrix_read(X);
    move_center(X);
    mat I = inertia_tensor(X);
    _IX = I * X.t();
}

void conservation::zero_round(arma::mat &M) {
    int n_rows = M.n_rows;
    int n_cols = M.n_cols;
    for (int i = 0; i != n_rows; ++i)
    {
        for (int j = 0; j != n_cols; ++j)
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
void conservation::normalize(arma::vec &V) {
    int N = V.n_rows;
    double normal = 0;
    for (int i = 0; i != N; ++i)
    {
        normal += pow(V[i], 2);
    }
    normal = pow(normal, 0.5);
    for (int i = 0; i != N; ++i)
    {
        V[i] /= normal;
    }
    zero_round(V);
}

/************** Move to origin ******************/
/* Takes parameter matrix of coordinates (in row form)
 * and a vector of masses, to yield the a row vector of
 * the center of mass. */
void conservation::move_center(arma::mat &A) {
    using namespace arma;
    using namespace std;
    double total_mass = 0;
    for (auto i : _m)
    {
        total_mass += i;
    }
    int _N = A.n_rows;
    vector<double> v_center_mass;
    for (int i = 0; i != 3; ++i)
    {
        double coord_center = 0;
        for (int j = 0; j != _N; ++j)
        {
            coord_center += (A(j, i) / total_mass) * _m[j];
        }
        v_center_mass.push_back(coord_center);
    }
    rowvec center_mass(v_center_mass);
    for (int i = 0; i != _N; ++i)
    {
        A.row(i) = A.row(i) - center_mass;
    }
    zero_round(A);
}

/**************** Inertia Tensor *****************/
/* Takes parameters matrix of coordinates (in row form)
 * and a vector of masses, to yield the inertia tensor */
arma::mat conservation::inertia_tensor(const arma::mat &M) {
    using namespace arma;
    int _N = M.n_rows;
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
    mat I;
    I << _I_xx << _I_xy << _I_xz << endr
    << _I_xy << _I_yy << _I_yz << endr
    << _I_xz << _I_yz << _I_zz << endr;
    zero_round(I);
    return I;
}

arma::mat conservation::coord_read(const std::vector<glm::vec3> &xyz) {
    using namespace arma;
    mat X(_N, 3);
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
    using namespace arma;
    using namespace std;
    mat Y = coord_read(xyz_n);
    move_center(Y);
    mat I = inertia_tensor(Y);
    mat IY = I * Y.t();
    mat H(3,3);
    for (int i=0; i!=3; ++i)
    {
        H += _IX.col(i) * IY.col(i).t();
    }
    zero_round(H);
    mat U;
    vec s;
    mat V;
    svd(U,s,V,H);
    mat R = V*U.t();
    Y = (R * Y.t()).t();
    zero_round(Y);
    xyz_n = matrix_read(Y);
}

std::vector<glm::vec3> conservation::matrix_read(const arma::mat &M) {
    using namespace arma;
    using namespace std;
    vector<glm::vec3> xyz_temp(3);
    for (int i=0; i!=_N; ++i)
    {
        xyz_temp[i].x = M(i,0);
        xyz_temp[i].y = M(i,1);
        xyz_temp[i].z = M(i,2);
    }
    return xyz_temp;
}
