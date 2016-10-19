/* WeightedPerfectMatchingLib.h (created on 06/06/2016 by Nicolas)
* This file exposes the entry point functions for the C++ interface of the library. */

#ifndef WEIGHTED_PERFECT_MATCHING_LIB_H
#define WEIGHTED_PERFECT_MATCHING_LIB_H



#include <string>
#include <vector>



//! Possible result codes
enum ResultCode {
	ResCode_Success,
	ResCode_InvalidGraph,
	ResCode_MatchingFailure,
	ResCode_InvalidMatching,
	ResCode_KnownException,
	ResCode_UnknownException
};



//! Find a perfect matching over the specified graph, which is selected randomly among those with minimal cost. The result code is passed as return value, and the matching vector and updated graph string are passed via reference arguments.
ResultCode findBestPerfectMatching(const std::string &graph_str, std::vector<unsigned int> &matching, std::string &updated_graph_str);




#endif //WEIGHTED_PERFECT_MATCHING_LIB_H
