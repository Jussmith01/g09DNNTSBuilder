// Standary Library
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <iomanip>
#include <vector>
#include <algorithm>

#include <glm/glm.hpp>

// Error Handling
#include "errorhandlers.h" // Contains the error handlers.

unsigned nsum(unsigned n) {
    return ( n * ( n - 1 ) )/ 2;
};

unsigned indexTriangle(unsigned i, unsigned j, unsigned n) {

    /* Error Checking */
    if (i==j) {
        dnntsErrorcatch(std::string("Index i cannot equal j!"));
    }

    unsigned I,J;

    /* Symmetry of Matrix */
    if (j < i) {
        I = j;
        J = i;
    } else {
        I = i;
        J = j;
    }

    /* Return linear index */
    return nsum(n) - nsum(n-I) + J - I - 1;
};

/*------Produce a Distance Matrix from Positions------

------------------------------------------------------*/
void prodDistMat(unsigned N,const std::vector<glm::vec3> &pos,std::vector<float> &R) {
    R.resize(nsum(N));

    for (unsigned i = 0; i < N; ++i) {
        for (unsigned j = i + 1; j < N; ++j) {
            R[indexTriangle(i,j,N)] = glm::length( pos[i] - pos[j] );
            //std::cout << R[sfl::indexTriangle(i,j,N)] << " | ";
        }
        //std::cout << std::endl;
    }
};

/* Data Contrainer */
struct datacont {
    unsigned Na;
    unsigned Nm;

    std::vector<glm::vec3  > ixyz; // Input coordinates of the entire dataset (size Na*Nm)
    std::vector<double     > enrg; // Type index  (size Nm)
    std::vector<unsigned   > tidx; // Type index  (size Nm)
    std::vector<std::string> tstr; // Type string (size Nm)

    /* Cleanup Class */
    void cleanup () {
        ixyz.clear();
        tidx.clear();
        enrg.clear();
        tstr.clear();
    };
};

struct datadist {
    std::vector<float> d;
};

void csvreader(const std::string &line,std::vector<std::string> &data) {

    unsigned pIdx1(0);
    unsigned pIdx2(0);
    unsigned cnt(0);
    for (unsigned int i=0; i<line.size(); ++i) {
        if (line[i]==',') {
            //std::cout << atof(line.substr(pIdx+1,i-1-pIdx).c_str()) << std::endl;
            pIdx2 = i;
            data[cnt] = line.substr(pIdx1,pIdx2-pIdx1);
            //std::cout << "ELEM:" << data[cnt] << std::endl;
            pIdx1 = pIdx2+1;

            ++cnt;
        }
    }
};

unsigned countlines(std::fstream &file) {
    file.seekg(std::ios::beg);

    unsigned counter(0);
    while ( !file.eof() ) {
        file.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
        ++counter;
    }

    file.seekg(std::ios::beg);
    return counter-1;
};

unsigned csvelementcount(const std::string &line) {
    //std::string tmpline(line);

    unsigned int cnt = 0;
    for (unsigned int i=0; i<line.size(); ++i) {
        if (line[i]==',') ++cnt;
    }
    return cnt;
};

void loadDataToHost(const std::string &filename,datacont &data) {
    using namespace std;

    fstream dfile(filename.c_str());
    string line;

    if (dfile.is_open()) {

        data.Nm = countlines(dfile); // Count data points

        getline( dfile,line );
        unsigned dsize = csvelementcount(line);
        vector<string> elements(dsize);
        csvreader( line,elements );

        data.Na = atoi( elements[0].c_str() );

        for (unsigned i = 1; i < 1 + data.Na; ++i) {
            data.tstr.push_back(elements[i]);
        }

        data.ixyz.reserve(data.Nm * data.Na);
        data.enrg.reserve(data.Nm * data.Na);

        //cout << "Nm: " << data.Nm << " Na: " << data.Na << " dsize: " << dsize << endl;

        dfile.seekg(std::ios::beg); // Move back to start of file

        stringstream s;
        s << dfile.rdbuf();
        dfile.close();

        //while ( !dfile.eof() ) {
        //getline(dfile,line);
        while (getline(s,line)) {
            // Store Coordinate and energy data
            csvreader(line,elements);
            if ( elements.size() == dsize ) {
                csvreader(line,elements);
                for (unsigned i = 1 + data.Na; i < data.Na + data.Na * 3 + 1; i+=3) {
                    data.ixyz.push_back(glm::vec3(atof(elements[i  ].c_str())
                                                  ,atof(elements[i+1].c_str())
                                                  ,atof(elements[i+2].c_str())));
                }

                data.enrg.push_back((double)atof( elements[data.Na + data.Na * 3 + 1].c_str() ));

            } else {
                dnntsErrorcatch(string("ERROR: Data file has wrong shape!"));
            }
        }

    } else {
        stringstream _err;
        _err << "Cannot open data file " << filename << "!!" << std::endl;
        dnntsErrorcatch(_err.str());
    }
};

unsigned indexgrid(std::vector<unsigned> &bmult,std::vector<unsigned> &idxvec) {

    unsigned idx(0);
    for (unsigned i = 0; i < idxvec.size(); ++i) {
        idx += idxvec[i] * bmult[i];
    }
    return idx;
};

int main(int argc, char *argv[]) {
    using namespace std;

    if (argc < 3) {
        dnntsErrorcatch(string("Error: Missing argument."));
    }

    string fname (argv[1]);
    datacont data;

    float gs ( atof(argv[2]) );

    loadDataToHost(fname,data);

    unsigned Nm ( data.Nm );
    unsigned Na ( data.Na );
    unsigned Nr ( nsum(Na) );

    vector<double> E(Nm);

    vector<datadist> vx( Nr );
    vector<float> maxvx( Nr );
    vector<float> minvx( Nr ); //stores min vals
    vector<unsigned> Ng( Nr ); // Grid size for each dimension
    vector<unsigned> Im( Nr ); // Index Multiplier

    for (unsigned i = 0; i < Nr;++i) {
        vx[i].d.resize(Nm);
    }

    for (unsigned i = 0; i < Nm;++i) {
        vector<glm::vec3> pos (data.ixyz.begin()+i*Na,data.ixyz.begin()+i*Na+Na);
        vector<float> R;

        prodDistMat(Na,pos,R);

        E[i] = data.enrg[i];

        cout << i << ") E: " << E[i] <<  " R: ";
        for (unsigned j = 0; j < Nr; ++j) {
            cout << R[j] << " ";
            vx[j].d[i] = R[j];
        }
        cout << endl;
    }

    for (unsigned i = 0; i < Nr;++i) {
        minvx[i] = *std::min_element(vx[i].d.begin(),vx[i].d.end());
        maxvx[i] = *std::max_element(vx[i].d.begin(),vx[i].d.end());
        Ng[i] = ceil( (maxvx[i] - minvx[i])  / gs );
        cout << "(" << i << "): " << " MAX: " << maxvx[i] << " MIN: " << minvx[i] << " Ng: " << Ng[i] << endl;
    }

    unsigned Nbins(1);
    vector<unsigned> bmult (Nr);
    for (unsigned i = 0; i < Nr;++i) {
        bmult[i] = Nbins;
        Nbins = Nbins * Ng[i];
    }

    vector<unsigned> ebcnt(Nbins,0); // element per bin count
    vector<double> benergy(Nbins,0.0f); // bin energy
    for (unsigned i = 0; i < Nm; ++i) {

        vector<unsigned> idxvec(Nr);
        //cout << "Index(" << i << "): ";
        for (unsigned j = 0; j < Nr;++j) {
            idxvec[j] = floor((vx[j].d[i] - minvx[j]) / gs);
            //cout << floor((vx[j].d[i] - minvx[j]) / gs) << " ";
        }
        //cout << endl;
        unsigned idx (indexgrid(bmult,idxvec));

        benergy[idx] += E[i];

        ++ebcnt[idx];

    }

    for (unsigned i = 0; i < ebcnt.size(); ++i) {
        benergy[i] = (ebcnt[i] > 0) ? benergy[i]/static_cast<double>( ebcnt[i] ) : 0.0;
        cout << "bin(" << i << "): " << ebcnt[i] << " AvgE: " << benergy[i] << endl;
    }

    return 0;
};
