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
        seedgen(threadseeds.begin(),threadseeds.end())
    {
        getThreadSafeSeeds(seeds);

        for (int i=0;i<4;++i)
        {
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
class NormRandomReal
{
    std::default_random_engine generator;

public:

    NormRandomReal() {};

    void fillVector(float mean,float stdev,std::vector<float> &rnv,int N,std::vector<int> &threadseeds) {
        rnv.resize(N);

        std::vector<int> seeds(N);
        ThreadSeedGenerator seedGen(threadseeds,seeds);

        for (int i=0;i<N;++i) {
                generator.seed(seeds[i]);//Seed the generator
                std::normal_distribution<float> distribution(mean,stdev);//Setup the distribution
                rnv[i] = distribution(generator);//Denerate the random number
        }
    };
};

#endif
