//
// Created by Dustin on 8/13/15.
//

#include "conservation.h"

conservation::conservation(std::vector<glm::vec3> &xyz, const std::vector<double> &m) : _m(m) {
    using namespace Eigen;
    using namespace std;
    _N = static_cast<int>(m.size());
    MatrixXd X = coord_read(xyz);
    xyz = matrix_read(X);
    move_center(X);
    Matrix3d I = inertia_tensor(X);
    _IX = I * X.transpose();
    vector<Vector3d> X_vecs = determine_vectors(X);
    X_plane = determine_init_plane_vector(X_vecs);
    X_inert = determine_init_inertia_vector(X_vecs);
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
    vector<Vector3d> Y_vecs = determine_vectors(Y);
    Vector3d Y_plane = determine_init_plane_vector(Y_vecs);

    /* Rotate to plane */
    Matrix3d R = rotation_matrix(Y_plane, X_plane);
    Y = Y * R.transpose();

    /* Rotate to inertia vector */
    Y_vecs = determine_vectors(Y);
    Vector3d Y_inert = determine_init_inertia_vector(Y_vecs);
    R = rotation_matrix(Y_inert, X_inert);
    Y = Y * R.transpose();
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

Eigen::Matrix3d conservation::rotation_matrix(const Eigen::Vector3d &v1, const Eigen::Vector3d &v2) {
    using namespace Eigen;
    Vector3d v = v1.cross(v2);
    double s = sqrt(v.dot(v));
    double c = v1.dot(v2);
    Matrix3d R;
    if (s > 1e-12 || s < -1e-12)
    {
        Matrix3d vx;
        vx << 0, -v(2), v(1),
                v(2), 0, -v(0),
                -v(1), v(0), 0;
        Matrix3d I = Matrix3d::Identity();
        R = I + vx + vx * vx * (1 - c) / (s * s);
    }
    else
    {
        R = Matrix3d::Identity();
    }
    return  R;
}

Eigen::Vector3d conservation::determine_init_inertia_vector(const std::vector<Eigen::Vector3d> &V) {
    using namespace Eigen;
    Vector3d v1 = V[0];
    Vector3d v2 = V[1];
    Vector3d v1_weighted;
    v1_weighted = _m[1] * v1.dot(v1) * unit(v1);
    Vector3d v2_weighted;
    v2_weighted = _m[2] * v2.dot(v2) * unit(v2);
    Vector3d init_inerteria_vector;
    init_inerteria_vector = v1_weighted + v2_weighted;
    init_inerteria_vector = unit(init_inerteria_vector);
    return init_inerteria_vector;
}

/// Sould this be returning something?
Eigen::Vector3d conservation::unit(const Eigen::Vector3d &v) {
    return v/sqrt(v.dot(v));
}

std::vector<Eigen::Vector3d> conservation::determine_vectors(const Eigen::Matrix3d X) {
    using namespace std;
    using namespace Eigen;
    vector<Vector3d> vectors;
    /* Determining the plane vector using the cross product of the O-H bonds */
    Vector3d v1, v2;
    v1 << X(1, 0) - X(0, 0), X(1, 1) - X(0, 1), X(1, 2) - X(0, 2);
    v2 << X(2, 0) - X(0, 0), X(2, 1) - X(0, 1), X(2, 2) - X(0, 2);
    vectors.push_back(v1);
    vectors.push_back(v2);
    return vectors;
}

Eigen::Vector3d conservation::determine_init_plane_vector(const std::vector<Eigen::Vector3d> &V) {
    using namespace Eigen;
    using namespace std;
    Vector3d init_plane_vector = V[0].cross(V[1]);
    init_plane_vector = init_plane_vector / sqrt(init_plane_vector.dot(init_plane_vector));
    return  init_plane_vector;
}
