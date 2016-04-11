//
//
//

#ifndef G09FUNCTIONS_H
#define G09FUNCTIONS_H

namespace g09 {

/*----------------------------------------

 Get Forces from a Gaussian Output String

------------------------------------------*/
inline void forceFinder(const std::string &filename,std::vector<glm::vec3> &tfrce) {
    using namespace std;

    ///stringstream force_csv;
    ///force_csv.setf( std::ios::scientific, std::ios::floatfield );

    regex pattern_force("Hartrees/Bohr");
    regex pattern_cart("Cartesian");
    regex pattern_value("\\-?[[:d:]]+\\.[[:d:]]+");
    string line;
    istringstream stream(filename);
    while (getline(stream, line)) {
        if (regex_search(line, pattern_force)) {
            string line2;
            int ln(-2);
            while (getline(stream, line2) && !regex_search(line2, pattern_cart)) {
                sregex_iterator pos(line2.begin(), line2.end(), pattern_value);
                sregex_iterator end;
                int ax(0);
                for (; pos != end; ++pos) {
                    tfrce[ln][ax] = atof(pos->str(0).c_str());
                    //std::cout << tfrce[ln][ax] << ",";
                    ++ax;
                }
                //std::cout << ln << std::endl;
                ++ln;
            }
        }
    }

    ///return force_csv.str();
};

inline std::string fftcf(const std::string& value) {
    using namespace std;
    string tmp( value );
    size_t pos( tmp.find("D") );

    //cout << "POS: " << pos << endl;

    tmp.replace(pos,1,"E");

    return tmp;
}

/*----------------------------------------

 Get input coordinates from a Gaussian
 Output String

------------------------------------------*/
inline void admpcrdenergyFinder(const std::string &output,std::vector<glm::vec3> &totalcartesians,std::vector<double> &totalenergies) {
    using namespace std;

    regex pattern_energy( "\\s*SCF Done:\\s*E\\(.*\\)\\s*=\\s*([^\\s]+)\\s*[Aa]\\.[Uu]\\." );
    //regex pattern_crdblk( "\\sOptimization completed\\.\\n[\\s\\S]+Input orientation:\\n[\\s\\S].+\\n.+\\n.+\\n.+\\n((?:\\s+\\d+\\s+\\d+\\s+\\d+\\s+[^\\s]+\\s+[^\\s]+\\s+[^\\s]+\\n)+)" );
    regex pattern_crdblk( " Cartesian coordinates:\\n((?:\\sI=\\s*\\d+\\s*X=\\s*[^\\s]+\\s*Y=\\s*[^\\s]+\\s*Z=\\s*[^\\s]+\\n)+)" );
    regex pattern_crd(" I=\\s*\\d+\\s*X=\\s*([^\\s]+)\\s*Y=\\s*([^\\s]+)\\s*Z=\\s*([^\\s]+)");

    /*if ( !regex_search ( output, sm, pattern_crdblk ) ) {
        cout << "Coordinate Pattern Not Found in admpcrdenergyFinder!" << endl;
    }*/

    cout.setf(ios::scientific,ios::floatfield);
    unsigned cnt(0);
    if (regex_search(output,pattern_energy)) {
        sregex_iterator items(output.begin(),output.end(),pattern_energy);
        sregex_iterator end;
        for (; items != end; ++items) {
            if (cnt != 0) {
                totalenergies.push_back(atof(items->str(1).c_str()));
                //cout << setprecision(8) << "Total Energy: " << totalenergies.back() << endl;
            }
            ++cnt;
        }
    } else {
        cout << "Energy Pattern Not Found in admpcrdenergyFinder!" << endl;
    }

    cnt = 0;
    string wkstr;
    if (regex_search(output,pattern_crdblk)) {
        sregex_iterator items(output.begin(),output.end(),pattern_crdblk);
        sregex_iterator end;

        for (; items != end; ++items) {
            if (cnt != 0) {
                wkstr = items->str(1);
                //cout << wkstr << endl;
                if (regex_search(wkstr,pattern_crd)) {
                    sregex_iterator crds(wkstr.begin(),wkstr.end(),pattern_crd);
                    sregex_iterator end;
                    for (; crds != end; ++crds) {
                        //cout << "   " << fftcf( crds->str(1) ) << " " <<  fftcf( crds->str(2) ) << " " <<  fftcf( crds->str(3) ) << endl;

                        //cout << "   " << atof(fftcf( crds->str(1) ).c_str()) << " " <<  atof(fftcf( crds->str(2) ).c_str()) << " " <<  atof(fftcf( crds->str(3) ).c_str()) << endl;
                        totalcartesians.push_back(glm::vec3(atof(fftcf( crds->str(1) ).c_str())
                                                           ,atof(fftcf( crds->str(2) ).c_str())
                                                           ,atof(fftcf( crds->str(3) ).c_str())));
                        //cout << " Coord: " << totalcartesians.back().x << " " << totalcartesians.back().y << " " << totalcartesians.back().z << " " << endl;
                    }
                }
            }
            ++cnt;
        }
    } else {
        cout << "Coordinate Pattern Not Found in admpcrdenergyFinder!" << endl;
    }
};

/*----------------------------------------

 Get input coordinates from a Gaussian
 Output String

------------------------------------------*/
inline void ipcoordinateFinder(const std::string &output,std::vector<glm::vec3> &tcart,bool fail) {
    using namespace std;

    regex pattern_opt;
    if (!fail) {
        pattern_opt = "\\s(?:Optimization completed)\\.\\n[\\s\\S]+(?:Input orientation:)\\s+.+\\s+.+\\s+.+\\s+.+((?:\\s+\\d+\\s+\\d+\\s+\\d+\\s+[^\\s]+\\s+[^\\s]+\\s+[^\\s]+\\s)+)";
    } else {
        pattern_opt = "\\s(?:Optimization stopped)\\.\\n[\\s\\S]+(?:Input orientation:)\\s+.+\\s+.+\\s+.+\\s+.+((?:\\s+\\d+\\s+\\d+\\s+\\d+\\s+[^\\s]+\\s+[^\\s]+\\s+[^\\s]+\\s)+)";
    }
    regex pattern_crd("\\s+\\d+\\s+\\d+\\s+\\d+\\s+([+-]*\\d+\\.\\d+)\\s+([+-]*\\d+\\.\\d+)\\s+([+-]*\\d+\\.\\d+)");

    string coordstr;
    smatch sm;
    if ( regex_search ( output, sm, pattern_opt ) ) {
        coordstr = sm.str(1);
    } else {
        ofstream _failout("cannotfindcrds.log");
        _failout << output;
        _failout.close();
        cout << "Coordinate Pattern Not Found in ipcoordinateFinder!" << endl;
    }

    unsigned atcnt(0);
    if (regex_search(coordstr,pattern_crd)) {
        sregex_iterator items(coordstr.begin(),coordstr.end(),pattern_crd);
        sregex_iterator end;
        for (; items != end; ++items) {
            tcart[atcnt].x = atof(items->str(1).c_str());
            tcart[atcnt].y = atof(items->str(2).c_str());
            tcart[atcnt].z = atof(items->str(3).c_str());
            //cout << "[" << tcart[atcnt].x << "," << tcart[atcnt].y << "," << tcart[atcnt].z << "]\n";
            ++atcnt;
        }
    }
};

/*----------------------------------------
  Get G09 Internal Coordinate Forces

------------------------------------------*/
inline std::string icforceFinder(const std::string &filename) {
    using namespace std;

    stringstream force_csv;
    force_csv.setf( std::ios::scientific, std::ios::floatfield );

    regex pattern_force("Internal");
    regex pattern_cart("Internal");
    regex pattern_value("\\-?[[:d:]]+\\.[[:d:]]+");
    string line;
    istringstream stream(filename);
    while (getline(stream, line)) {
        if (regex_search(line, pattern_force)) {
            string line2;
            while (getline(stream, line2) && !regex_search(line2, pattern_cart)) {
                //std::cout << line2 << std::endl;
                sregex_iterator pos(line2.begin(), line2.end(), pattern_value);
                sregex_iterator end;
                for (; pos != end; ++pos) {
                    force_csv << std::setprecision(7) << atof(pos->str(0).c_str()) << ",";
                }
            }
        }
    }

    //std::cout << " FORCE: " << force_csv.str() << std::endl;

    return force_csv.str();
};

/*----------------------------------------
  Get G09 Internal Coordinate Forces

------------------------------------------*/
inline std::string energyFinder(const std::string &filename) {
    using namespace std;

    string energy;
    //energy.setf( std::ios::scientific, std::ios::floatfield );

    //regex patt_energy("\\sSCF Done:.*=\\s+(-*\\d+\\.\\d+)\\s");
    regex patt_energy("\\sSCF Done:.*=\\s+([^\\s]+)\\s");
    string line;
    istringstream stream(filename);
    while (getline(stream, line)) {
        std::smatch m;
        if (regex_search(line, m, patt_energy)) {
            energy = m.str(1) + ",";
        }
    }

    //std::cout << " FORCE: " << force_csv.str() << std::endl;

    return energy;
};

/*----------------------------------------
        Parse a Multi Gaussian Run
Parse many output into individual outputs.
------------------------------------------*/
inline void parseg09OutputLinks(int nrpg,std::string &multioutput,std::vector<std::string> &indoutputs) {
    size_t lpos = multioutput.find("Entering Link 1");
    multioutput = multioutput.substr(lpos+1);

    for (int i=0; i<nrpg; ++i) {
        lpos = multioutput.find("Entering Link 1");
        indoutputs[i] = multioutput.substr(0,lpos);
        multioutput=multioutput.substr(lpos+1);
    }
};

/*----------------------------------------
  Execute Gaussian Command on the System

  Function returns true if cerr returns
  a segmentation violation. This occurs
  most notably when the SCF fails to
  converge.
------------------------------------------*/
inline void execg09(int nrpg,const std::string &input,std::vector<std::string> &out,std::vector<bool> &chkout) {
    // Build bash command for launching g09
    std::stringstream sscmd;
    sscmd << "#!/bin/sh\ng09 <<END 2>&1 " << input << "END\n"; // Redirect cerr to cout

    // Open a pipe and run g09 command -- output saved in string 'out'.
    std::string mout(systls::exec(sscmd.str(),10000));

    parseg09OutputLinks(nrpg,mout,out);

    for (int j=0; j<nrpg; ++j) {
        // If normal termination is detected the the program returns false.
        if (out[j].find("Normal termination of Gaussian 09")!=std::string::npos) {
            chkout[j]=false;
        }

        // Check if gaussian failed to converge - return true if it fails.
        else if (out[j].find("Convergence failure -- run terminated")!=std::string::npos) {
            //std::cout << "CVF!" << std::endl;
            chkout[j]=true;
        }

        // Check if interatomic distances were too close  - return true if it fails.
        else if (out[j].find("Small interatomic distances encountered")!=std::string::npos) {
            //std::cout << "SIADF!" << std::endl;
            chkout[j]=true;
        }

        // Check if g09 was found, if not the
        else if (out[j].find("g09: not found")!=std::string::npos) {
            throwException("Gaussian 09 program not found; make sure it is exported to PATH.");
        }

        else {
            // If the function has not returned yet then something is wrong
            std::ofstream gaue("gauerror.log");
            gaue << out[j] << std::endl;
            gaue.close();
            chkout[j]=true;
            //throwException("Unrecognized Gaussian 09 Failure; saving output as gauerror.log");
        }
    }
};

/*----------------------------------------
          Build G09 Input String
    lot = level of theory
    additional = more g09 parameters...
       i.e. forces, opt
    type = atom type .i.e. O or 8 for Oxygen
    xyz = coordinates for the atom
    mult = molecular multiplicity
    charge = molecular charge
    nproc = number of processors to use

    Note: type and xyz must be of same size
------------------------------------------*/
inline void buildCartesianInputg09(int nrpg,std::string &input,std::string lot,std::string additional,const std::vector<std::string> &type,const std::vector<glm::vec3> &xyz,int mult,int charge,int nproc) {
    // Number of coords per molecule
    int N = xyz.size()/nrpg;

    // Error check
    if (type.size()!=static_cast<unsigned>(N))
        throwException("type and xyz are not the same size.");

    input="";

    for (int j=0; j<nrpg; ++j) {
        // Build gaussian 09 input
        std::stringstream tmpipt;
        tmpipt.setf( std::ios::scientific, std::ios::floatfield );
        tmpipt << "\n%NProcShared=" << nproc << "\n";
        tmpipt << "%Mem=" << 500 << "mb" << "\n";
        tmpipt << "# " << lot << " " << additional << "\n\n";
        tmpipt << "COMMENT LINE\n\n";
        tmpipt << charge << "  " << mult << "\n";

        for (uint32_t i = 0; i<type.size(); ++i) {
            if ( type[i].compare("X") != 0 ) {
                tmpipt << type[i] << std::setprecision(7) << " " << xyz[j*N+i].x << " " << xyz[j*N+i].y << " " << xyz[j*N+i].z << "\n";
            }
        }

        tmpipt << "\n";

        if (j < nrpg-1) {
            tmpipt << "--link1--\n";
        };

        // Return input string
        input.append(tmpipt.str());
        //std::cout << "---------------------------" << std::endl;
        //std::cout << input << std::endl;
        //std::cout << "---------------------------" << std::endl;
    }
};

/*----------------------------------------
          Build G09 Input String
    lot = level of theory
    additional = more g09 parameters...
       i.e. forces, opt
    type = atom type .i.e. O or 8 for Oxygen
    xyz = coordinates for the atom
    mult = molecular multiplicity
    charge = molecular charge
    nproc = number of processors to use

    Note: type and xyz must be of same size
------------------------------------------*/
inline void buildZmatInputg09(int nrpg,std::string &input,std::string lot,std::string additional,const std::vector< std::string > &zmat,int mult,int charge,int nproc) {

    input="";

    for (int j=0; j<nrpg; ++j) {
        // Build gaussian 09 input
        std::stringstream tmpipt;
        tmpipt.setf( std::ios::scientific, std::ios::floatfield );
        tmpipt << "\n%nproc=" << nproc << "\n";
        tmpipt << "\n%mem=" << 500 << "MB\n";
        tmpipt << "#p " << lot << " " << additional << "\n\n";
        tmpipt << "COMMENT LINE\n\n";
        tmpipt <<  charge << "  " << mult << "\n";

        tmpipt << zmat[j] << "\n";

        tmpipt << "\n";

        if (j < nrpg-1) {
            tmpipt << "--link1--\n";
        };

        // Return input string
        input.append(tmpipt.str());
    }
};

};
#endif //G09DNNTSBUILDER_READERS_H
