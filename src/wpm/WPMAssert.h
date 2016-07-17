/* WPMASSERT.h (created on 04/06/2016 by Nicolas) */

#include <iostream>

#ifdef _DEBUG
#define WPMASSERT(prop, msg) if(!(prop)){std::cout << "FAILED ASSERTION in '" << __FILE__ << "':" << __LINE__ << ": " << msg << std::endl; std::cout.flush(); throw std::runtime_error(msg);}
#else //_DEBUG
#define WPMASSERT(prop, msg)
#endif //_DEBUG
