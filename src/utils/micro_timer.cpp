#include <iostream>
#include <sstream>
#include <string>
#include <time.h>
#include <cmath>
#include <omp.h>

#include "micro_timer.h"

void MicroTimer::start_point () {
    start_wall_time = omp_get_wtime();
    start_clock_time = clock();
};

void MicroTimer::end_point () {
    accumtime+=omp_get_wtime()-start_wall_time;
    accumclock+=clock()-start_clock_time / (double)CLOCKS_PER_SEC;

    ++wt_count;
    ++ct_count;

    start_clock_time=0;
    start_wall_time=0;
};

//template<typename T>
void MicroTimer::print_wall_time(std::string message,std::ostream &output) {
    //file1 << message.c_str() << mk_time_string((double)accumtime/(double)wt_count).c_str() << "\n";
    output << message.c_str() << mk_time_string((double)accumtime).c_str() << "\n";
    reset();
};

//template<typename T>
void MicroTimer::print_clock_time(std::string message,std::ostream &output) {
    output << message.c_str() << mk_time_string((double)accumclock/(double)ct_count).c_str() << "\n";
    reset();
};

void MicroTimer::print_generic_to_file(std::string message,std::ostream &output) {
    std::stringstream ss1;
    ss1 << message << " Wall Time: ";

    //stringstream ss2;
    //ss2 << message << " Clock Time: ";

    print_wall_time(ss1.str(),output);
    //print_clock_time(ss2.str(),file1);
};

std::string MicroTimer::get_generic_print_string(std::string &message) {
    std::stringstream ss1;
    ss1 << message << " Wall Time: " << mk_time_string((double)accumtime).c_str();

    reset();
    return ss1.str();
};

void MicroTimer::print_generic_to_cout(std::string message) {
    std::cout << get_generic_print_string(message) << std::endl;
};

void MicroTimer::reset () {
    wt_count=0;
    ct_count=0;

    accumtime=0;
    accumclock=0;
};

std::string MicroTimer::mk_time_string(double time_val) {
    int days, hours, minutes;
    double seconds;

    days = floor(time_val/86400);
    hours = floor((time_val-(days*86400))/3600);
    minutes = floor((time_val - (days * 86400)-(hours * 3600))/60);
    seconds = (double)(time_val - (days * 86400) - (hours * 3600) - (minutes * 60));

    std::stringstream time_str;
    time_str << days << " dy. " << hours  << " hr. " << minutes << " mn. " << seconds  << " sc.";
    return time_str.str();
};

