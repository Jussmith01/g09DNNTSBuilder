#ifndef RANDNORMFLT_C
#define RANDNORMFLT_C

#include "ctime"
#include "random"
#include "cstring"

class ParallelSeedGenerator {
    std::vector<int> thrdseeds;

    void Setup(int numthreads) {
        time_t Time;
        time(&Time);

        thrdseeds.resize(4*numthreads);

        std::seed_seq initseedseq = {(int)Time,(int)clock()};
        initseedseq.generate(thrdseeds.begin(),thrdseeds.end());//Seed the generator
    };

    ParallelSeedGenerator() {};

public:

    ParallelSeedGenerator(int numthreads) {
        Setup(numthreads);
    };

    void getThreadSeeds(int thread,std::vector<int> &seedfill) {
        seedfill.resize(4);
        memcpy(&seedfill[0],&thrdseeds[thread*4],4*sizeof(int));
    };
};

/*-----------------------------------------------
  ***********************************************
  |                                             |
  ***********************************************

-------------------------------------------------*/
class ThreadSeedGenerator {
    std::seed_seq seedgen;

    void getThreadSafeSeeds(std::vector<int> &seedfill) {
        seedfill.resize(seedfill.size()+4);
        seedgen.generate(seedfill.begin(),seedfill.end());
    };

public:

    ThreadSeedGenerator(std::vector<int> &threadseeds,std::vector<int> &seeds) :
        seedgen(threadseeds.begin(),threadseeds.end()) {
        getThreadSafeSeeds(seeds);

        for (int i=0; i<4; ++i) {
            threadseeds[i]=seeds.back();
            seeds.pop_back();
        }
    };
};

/*-----------------------------------------------
  ***********************************************
  |Class for Generating a Flt from a Normal Dist|
  ***********************************************
Example Use:
NormRandomReal NR(w,i); //int w=num seeds
                    	//int i=thread seed

float someflt = GenRandReal(float mean,float std)
                   //mean is mean, duh
                   //std is standard deviation
-------------------------------------------------*/
class RandomReal {
    std::default_random_engine generator;
    std::vector<int> threadseeds;

    void (RandomReal::*randGenVec)(float a1,float a2,std::vector<float> &rnv,int N)=0;
    void (RandomReal::*randGenFlt)(float a1,float a2,float &rnv)      =0;

    float arg1;
    float arg2;

public:
    // Constructor
    RandomReal(std::vector<int> seedarray,float a1,float a2,std::string dist) :
        threadseeds(seedarray),arg1(a1),arg2(a2)
    {
        if (dist.compare("uniform")==0)
        {
            //std::cout << "Random number generator: Using uniform distribution from " << arg1 << " to " << arg2 << std::endl;
            randGenVec=&RandomReal::fillVectorUniformDist;
            randGenFlt=&RandomReal::floatUniformDist;
        }
        else if (dist.compare("normal")==0)
        {
            //std::cout << "Random number generator: Using normal distribution w/ mean " << arg1 << " std. dev. " << arg2 << std::endl;
            randGenVec=&RandomReal::fillVectorNormalDist;
            randGenFlt=&RandomReal::floatNormalDist;
        } else
            dnntsErrorcatch(std::string("Random Distribution not found!"));
    };

    // Set the random value range
    void setRandomRange(float arg1, float arg2) {
        this->arg1 = arg1;
        this->arg2 = arg2;
    };

    /*--------------------------
       Vector Filling Functions
     --------------------------*/
    // Fill a vector with random floats
    void fillVector(std::vector<float> &rnv,int N) {
        threadseeds.push_back(clock());
        (this->*RandomReal::randGenVec)(arg1,arg2,rnv,N);
    };

    // Fill a vector with random floats
    void fillVectorNormalDist(float mean,float stdev,std::vector<float> &rnv,int N) {
        rnv.resize(N);

        std::vector<int> seeds(N);
        ThreadSeedGenerator seedGen(threadseeds,seeds);

        for (int i=0; i<N; ++i) {
            generator.seed(seeds[i]);//Seed the generator
            std::normal_distribution<float> distribution(mean,stdev);//Setup the distribution
            rnv[i] = distribution(generator);//Denerate the random number
        }
    };

    // Fill a vector with random floats
    void fillVectorUniformDist(float maxi,float mini,std::vector<float> &rnv,int N) {
        rnv.resize(N);

        std::vector<int> seeds(N);
        ThreadSeedGenerator seedGen(threadseeds,seeds);

        for (int i=0; i<N; ++i) {
            generator.seed(seeds[i]);//Seed the generator
            std::uniform_real_distribution<float> distribution(mini,maxi);//Setup the distribution
            rnv[i] = distribution(generator);//Denerate the random number
        }
    };

    /*--------------------------
      Vector Filling Functions
    ---------------------------*/
    // Get a random float
    void getRandom(float &rnv) {
        threadseeds.push_back(clock());
        (this->*RandomReal::randGenFlt)(arg1,arg2,rnv);
    };

    // Fill a vector with random floats
    void floatNormalDist(float mean,float stdev,float &rnv) {
        int N=1;

        std::vector<int> seeds(N);
        ThreadSeedGenerator seedGen(threadseeds,seeds);

        for (int i=0; i<N; ++i) {
            generator.seed(seeds[i]);//Seed the generator
            std::normal_distribution<float> distribution(mean,stdev);//Setup the distribution
            rnv = distribution(generator);//Denerate the random number
        }
    };

    // Fill a vector with random floats
    void floatUniformDist(float maxi,float mini,float &rnv) {
        int N=1;

        std::vector<int> seeds(N);
        ThreadSeedGenerator seedGen(threadseeds,seeds);

        for (int i=0; i<N; ++i) {
            generator.seed(seeds[i]);//Seed the generator
            std::uniform_real_distribution<float> distribution(mini,maxi);//Setup the distribution
            rnv = distribution(generator);//Denerate the random number
        }
    };
};

#endif
