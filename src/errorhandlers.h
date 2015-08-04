/*----------------------------------------------
                 Fatal Error
    Aborts runtime with location and _error
    text if called.
-----------------------------------------------*/
#define FatalError(_error)                            \
{                                                     \
    std::stringstream _location,_message;             \
    _location << __FILE__ << ":" << __LINE__;         \
    _message << "Error -- "+_error.str();             \
    std::cerr << "File "                              \
              << _location.str() << "\n"              \
              << _message.str() << "\n"               \
              << "Aborting!" << "\n";                 \
    return(EXIT_FAILURE);                             \
};

/*----------------------------------------------
                 Check Arguments
    Checks for if an input is null and gives
    the proper syntax for launching the program
    if called, then calls fatal passing it the
    error string.
-----------------------------------------------*/
#define checkArgs(_args)                                     \
{                                                            \
    if(_args==NULL)                                          \
    {                                                        \
        std::stringstream _error;                            \
        _error << "Invalid arguments\n";                     \
        _error << "Proper Syntax: ";                         \
        _error << " DNNTSBuilder [inputfile] [outputfile]\n";\
        FatalError(_error);                                  \
    }                                                        \
};

/*----------------------------------------------
             Standard Error
    Checks for if an input string is empty,
    if it is pass the string to FatalError.
-----------------------------------------------*/
#define dnntsErrorcatch(_errchk)                             \
{                                                            \
    if(!_errchk.empty())                                     \
    {                                                        \
        std::stringstream _error;                            \
        _error << std::string(_errchk);                      \
        FatalError(_error);                                  \
    }                                                        \
};

/*----------------------------------------------
               Throw Exception
    Pass the macro a char string with an error
    and throw a std::string exception.
-----------------------------------------------*/
#define throwException(_errstr)                              \
{                                                            \
    if (!std::string(_errstr).empty())                       \
    {                                                        \
        throw std::string("In Function: ")                   \
            + std::string(__FUNCTION__)                      \
            + std::string("() -- ")                          \
            + std::string(_errstr);                          \
    }                                                        \
};
