#ifndef MICROTIMER_HPP
#define MICROTIMER_HPP

//________________________________________________________________________//
//      *************************************************************     //
//                               Timer Class
//                  Holds timer variables and class functions
//      *************************************************************     //
class MicroTimer {
    //--------------------------
    //Private Class Declarations
    //--------------------------

    double start_wall_time; //Holds start time
    long int wt_count; //Holds end time
    double accumtime;

    clock_t start_clock_time;
    long int ct_count;
    double accumclock;

    //------------------------------
    //Private Member Class Functions
    //------------------------------

    //Intakes a value of time in seconds and returns a string formmatted as:
    //Computation Wall Time: d days h hours m minutes s seconds
    std::string mk_time_string(double time_val);

public:
    // Default constructor
    MicroTimer() :
        wt_count(0),accumtime(0),ct_count(0),accumclock(0) {
    };

    //-----------------------------
    //Public Member Class Functions
    //-----------------------------
    //Sets the start time for the timer
    void start_point(void);

    //Sets the end time for the timer
    void end_point(void);

    //Prints run_timer = end_time - start_time
    void print_wall_time(std::string message,std::ostream &output);

    //Prints the clock time
    void print_clock_time(std::string message,std::ostream &output);

    // Generic Time Print
    void print_generic_to_file(std::string message,std::ostream &output);

    // Return generic print string
    std::string get_generic_print_string(std::string &message);

    // Generic Time Print to std::cout
    void print_generic_to_cout(std::string message);

    //Resets the timer if needed
    void reset(void);
};

#endif

