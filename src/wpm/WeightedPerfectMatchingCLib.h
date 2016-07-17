/* WeightedPerfectMatchingCLib.h (created on 04/06/2016 by Nicolas)
 * This file exposes the entry point functions for the C interface of the library. */

#ifndef WEIGHTED_PERFECT_MATCHING_CLIB_H
#define WEIGHTED_PERFECT_MATCHING_CLIB_H



extern "C" {



//! Possible result codes
#define RESCODE_SUCCESS 0
#define RESCODE_INVALID_GRAPH 1
#define RESCODE_MATCHING_FAILURE 2
#define RESCODE_INVALID_MATCHING 3
#define RESCODE_KNOWN_EXCEPTION 4
#define RESCODE_UNKNOWN_EXCEPTION 5



//! Declaration of callback invokers (called by the library to pass results).
typedef void(*invoke_oncomplete_callback_t)(int /*error code*/, const char* /*matching*/, const char* /*updated graph or known exception message*/);



//! Find a perfect matching over the specified graph, which is selected randomly among those with minimal cost. The error code, matching string and updated graph string are passed via the specified callback.
void findBestPerfectMatching(const char* graph_str, invoke_oncomplete_callback_t callback);



}



#endif //WEIGHTED_PERFECT_MATCHING_CLIB_H
