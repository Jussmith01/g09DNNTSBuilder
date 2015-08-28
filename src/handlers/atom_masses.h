//
// Created by fury on 8/14/15.
//

#ifndef G09DNNTSBUILDER_ATOM_MASSES_H
#define G09DNNTSBUILDER_ATOM_MASSES_H

#include <vector>
#include <string>

extern inline std::vector<double> find_masses(std::vector<std::string> s_mass) {
    std::vector<double> mass;
    for (auto i : s_mass) {
        double mass_element = 0;
        if (i == "H" || i == "1") {
            mass_element = 1.00794;
        } else if (i == "Li" || i == "3") {
            mass_element = 6.941;
        } else if (i == "Be" || i == "4") {
            mass_element = 9.012183;
        } else if (i == "B" || i == "5") {
            mass_element = 10.811;
        } else if (i == "C" || i == "6") {
            mass_element = 12.0107;
        } else if (i == "N" || i == "7") {
            mass_element = 14.0067;
        } else if (i == "O" || i == "8") {
            mass_element = 15.9994;
        } else if (i == "F" || i == "9") {
            mass_element = 18.9984032;
        } else if (i == "Na" || i == "11") {
            mass_element = 22.98976628;
        } else if (i == "Mg" || i == "12") {
            mass_element = 24.3060;
        } else if (i == "Al" || i == "13") {
            mass_element = 26.9815386;
        } else if (i == "Si" || i == "14") {
            mass_element = 28.0855;
        } else if (i == "P" || i == "15") {
            mass_element = 30.973762;
        } else if (i == "S" || i == "16") {
            mass_element = 32.065;
        } else if (i == "Cl" || i == "17") {
            mass_element = 35.453;
        } else {
            throwException("Unrecognized atom elements");
        }
        mass.push_back(mass_element);
    }
    return mass;
}

#endif //G09DNNTSBUILDER_ATOM_MASSES_H
