#ifndef RANDNORMFLT_C
#define RANDNORMFLT_C

#include "ctime"
#include "random"

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
        std::vector<int> seeds;
        int index;

        public:
        NormRandomReal(){};
        NormRandomReal(int w,int seed){Setup(w,seed);};

        void Setup(int w,int i)
        {
                time_t Time;
                time(&Time);
                int seedOffset=(int)Time;

                seeds.resize(w);

                std::seed_seq seed = {seedOffset,i};
                seed.generate(seeds.begin(),seeds.end());//Seed the generator
                index = 0;
        };

        float genflt(float mean,float stdev)
        {
                if (index>=int(seeds.size()))
                {
                    throw std::string("In Function: ")
                        + std::string(__FUNCTION__)
                        + std::string("() -- Out of random numbers!");
                } else {
                    generator.seed(seeds[index]);//Seed the generator
                    std::normal_distribution<float> distribution(mean,stdev);//Setup the distribution
                    float RN = (float)distribution(generator);//Denerate the random number
                    ++index;//Increase seed offset
                    return RN;
                }
        };

        void fillVector(float mean,float stdev,std::vector<float> &rnv,int N)
        {
            rnv.resize(N);
            for (auto && rn : rnv)
            {
                if (index>=int(seeds.size()))
                {
                    throw std::string("In Function: ")
                        + std::string(__FUNCTION__)
                        + std::string("() -- Out of random numbers!");
                } else {
                    generator.seed(seeds[index]);//Seed the generator
                    std::normal_distribution<float> distribution(mean,stdev);//Setup the distribution
                    rn = (float)distribution(generator);//Denerate the random number
                    ++index;//Increase seed index
                }
            }
        };
};

#endif
