/* WeightedPerfectMatchingCLib.cpp (created on 06/06/2016 by Nicolas) */



#include "WeightedPerfectMatchingLib.h"
#include "WeightedPerfectMatchingCLib.h"

#include <sstream>
#include "NonMatchingGraph.h"
#include "BipartiteMatchingGraph.h"
#include "MatchingGraphConverter.h"
#include "PerfectMatchingFinder.h"



// Internal C++ library functions ////////////////////////////////////////////////////////////////////////////////////////////////////////////////



namespace impl {



//! Function to check the validity of the specified matching.
bool checkPerfectMatchingValidity(unsigned int ncliques, const std::vector<unsigned int> &matching)
{
	std::vector< std::pair<bool, bool> > clique_flags(ncliques, std::make_pair(false, false));
	for(std::vector<unsigned int>::const_iterator it_m=matching.begin(); it_m!=matching.end(); ++it_m) {
		unsigned int id_source = std::distance(matching.begin(), it_m);
		unsigned int id_target = matching[id_source];
		if(clique_flags[id_source].first)
			return false;	// The matching is not valid: the vertex with id 'id_source' is already a source for another target vertex
		if(clique_flags[id_target].second)
			return false;	// The matching is not valid: the vertex with id 'id_target' is already a target for another source vertex
		clique_flags[id_source].first = true;
		clique_flags[id_target].second = true;
	}
	for(unsigned int id=0; id<ncliques; ++id) {
		const std::pair<bool, bool> &clique_flag = clique_flags[id];
		if(!clique_flag.first)
			return false;	// The matching is not valid: the vertex with id 'id' has no target vertex
		if(!clique_flag.second)
			return false;	// The matching is not valid: the vertex with id 'id' has no source vertex
	}
	return true;
}

//! Find a random perfect matching with minimal cost in the specified graph, and return the results via the specified callback.
ResultCode findBestPerfectMatching(const std::string &graph_str, std::vector<unsigned int> &matching, std::string &updated_graph_str)
{
	std::stringstream updated_graph_ss;
	try {

		// Deserialize the input graph
		std::istringstream is(graph_str);
		wpm::NonMatchingGraph nmg;
		if(!nmg.deserialize(is))
			return ResCode_InvalidGraph;

		// Convert the non-matching graph into a bipartite matching graph
		wpm::MatchingGraphConverter gconverter;
		wpm::BipartiteMatchingGraph bmg;
		gconverter.toBipartiteMatchingGraph(nmg, bmg);

		// Find a random perfect matching
		wpm::PerfectMatchingFinder pmfinder;
		std::vector<const wpm::BipartiteMatchingGraph::Edge*> bmg_matching;
		if(!pmfinder.findRandomPerfectMatching(bmg, bmg_matching))
			return ResCode_MatchingFailure;

		// Convert the matching to a generic matching and update the graph
		gconverter.toGenericMatching(bmg_matching, matching);
		if(!checkPerfectMatchingValidity(bmg.getNumberCliques(), matching))
			return ResCode_InvalidMatching;

		// Update the constraints in the non-matching graph
		nmg.updateConstraints(matching,true);
		updated_graph_ss << nmg;

	}
	catch(const std::exception &e) {
		updated_graph_str = e.what();
		return ResCode_KnownException;
	}
	catch(...) {
		return ResCode_UnknownException;
	}

	// Call the callback with the result
	updated_graph_str = updated_graph_ss.str();
	return ResCode_Success;
}

//! Wrapper function for the C library interface.
void findBestPerfectMatching_CWrapper(const char* graph_str, invoke_oncomplete_callback_t callback)
{
	// Compute the matching and updated graph
	std::string matching_str, updated_graph_str;
	try {
		// Compute the matching and updated graph string
		std::vector<unsigned int> matching;
		ResultCode rescode = impl::findBestPerfectMatching(std::string(graph_str), matching, updated_graph_str);
		if(rescode!=ResCode_Success) {
			switch(rescode) {
			case ResCode_InvalidGraph:
				callback(RESCODE_INVALID_GRAPH, "", "");
				break;
			case ResCode_MatchingFailure:
				callback(RESCODE_MATCHING_FAILURE, "", "");
				break;
			case ResCode_InvalidMatching:
				callback(RESCODE_INVALID_MATCHING, "", "");
				break;
			case ResCode_KnownException:
				callback(RESCODE_KNOWN_EXCEPTION, "", updated_graph_str.c_str());
				break;
			case ResCode_UnknownException:
				callback(RESCODE_UNKNOWN_EXCEPTION, "", "");
				break;
			}
			return;
		}

		// Convert the matching into a string
		wpm::MatchingGraphConverter gconverter;
		gconverter.toStringMatching(matching, matching_str);
	}
	catch(const std::exception &e) {
		callback(RESCODE_KNOWN_EXCEPTION, e.what(), "");
		return;
	}
	catch(...) {
		callback(RESCODE_UNKNOWN_EXCEPTION, "", "");
		return;
	}

	// Call the callback with the result
	callback(RESCODE_SUCCESS, matching_str.c_str(), updated_graph_str.c_str());
}



}	// namespace impl



// Exposed C++ library function ////////////////////////////////////////////////////////////////////////////////////////////////////////////////



//! C++ function exposed by the library.
ResultCode findBestPerfectMatching(const std::string &graph_str, std::vector<unsigned int> &matching, std::string &updated_graph_str)
{
	return impl::findBestPerfectMatching(graph_str, matching, updated_graph_str);
}



// Exposed C library function ////////////////////////////////////////////////////////////////////////////////////////////////////////////////



//! C function exposed by the library.
void findBestPerfectMatching(const char* graph_str, invoke_oncomplete_callback_t callback)
{
	impl::findBestPerfectMatching_CWrapper(graph_str, callback);
}
