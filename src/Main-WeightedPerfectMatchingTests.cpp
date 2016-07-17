/* Main-WeightedPerfectMatchingTests.cpp (created on 04/06/2016 by Nicolas) */



#define NOMINMAX
#include <Windows.h>
#include <random>
#include <time.h>
#include <iostream>
#include <iomanip>
#include <sstream>

#include "wpm/NonMatchingGraph.h"
#include "wpm/BipartiteMatchingGraph.h"
#include "wpm/MatchingGraphConverter.h"
#include "wpm/PerfectMatchingFinder.h"



// Test macros
#define FLUSHED_CONSOLE_MSG(msg) std::cout << msg; std::cout.flush();

#define ANNONCE_TEXT_BLOCK                                                                                                                                                 \
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BACKGROUND_BLUE|FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY|BACKGROUND_INTENSITY);   \
	FLUSHED_CONSOLE_MSG("### " << __FUNCTION__ << " ###" << std::endl)                                                                                                      \
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);

#define EXECUTE_TEST(name,test_fct)                                                                                  \
	FLUSHED_CONSOLE_MSG("Testing '" << name << "'..." << std::endl)                                                  \
	if(test_fct()) {                                                                                                 \
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN|FOREGROUND_INTENSITY);             \
		FLUSHED_CONSOLE_MSG("Test '" << name << "' PASSED" << std::endl)                                             \
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);   \
	}                                                                                                                \
	else {                                                                                                           \
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED|FOREGROUND_INTENSITY);               \
		FLUSHED_CONSOLE_MSG("Test '" << name << "' FAILED" << std::endl)                                             \
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);   \
	}

#define ENQUEUE_ERROR_MSG(error_msgs_vector,error_msg_stream) \
	{                                                         \
		std::ostringstream ss;                                \
		ss << error_msg_stream;                               \
		error_msgs_vector.push_back(ss.str());                \
	}                                                         \



// Predeclarations
void tests_NonMatchingGraph();
bool test_NonMatchingGraph_DirectCreation();
bool test_NonMatchingGraph_Deserialization();
void tests_BipartiteMatchingGraph();
bool test_BipartiteMatchingGraph_DirectCreation();
bool test_BipartiteMatchingGraph_Deserialization();
void tests_MatchingGraphConverter();
bool test_MatchingGraphConverter_NM2BMConversion();
bool test_MatchingGraphConverter_BM2NMConversion();
void tests_PerfectMatchingFinder();
bool test_PerfectMatchingFinder_FindRandomValidMatch();
bool test_PerfectMatchingFinder_FindSuccessiveMatch();
bool test_PerfectMatchingFinder_FindHighestWeightMatch();
void tests_QuantitativeEvaluations();
bool test_QuantitativeEvaluations_TimeTilKnownMatching();
bool test_QuantitativeEvaluations_MatchingProbabilities();



// Main function
void main()
{
	std::srand((unsigned int)time(NULL));
	FLUSHED_CONSOLE_MSG("Starting tests:" << std::endl)
	tests_NonMatchingGraph();
	tests_BipartiteMatchingGraph();
	tests_MatchingGraphConverter();
	tests_PerfectMatchingFinder();
	tests_QuantitativeEvaluations();
	system("pause");
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////



namespace impl {



void displayNonMatchingGraph(const wpm::NonMatchingGraph &nmg)
{
	std::stringstream ss;
	ss << nmg << std::endl;
	FLUSHED_CONSOLE_MSG(ss.str())
}

void displayGenericMatching(const std::vector<unsigned int> &matching)
{
	std::stringstream ss;
	ss << "Matching: ";
	for(std::vector<unsigned int>::const_iterator it_m=matching.begin(); it_m!=matching.end(); ++it_m)
		ss << (it_m==matching.begin() ? "" : ", ") << std::distance(matching.begin(), it_m) << "->" << *it_m;
	ss << std::endl;
	FLUSHED_CONSOLE_MSG(ss.str())
}

void displayBMGMatching(const std::vector<const wpm::BipartiteMatchingGraph::Edge*> &matching)
{
	std::stringstream ss;
	ss << "Matching: ";
	int total_score = 0;
	for(std::vector<const wpm::BipartiteMatchingGraph::Edge*>::const_iterator it_e=matching.begin(); it_e!=matching.end(); ++it_e) {
		total_score += (*it_e)->score;
		ss << (it_e==matching.begin() ? "" : ", ") << (*it_e)->v_source->parent->cid << "->" << (*it_e)->v_target->parent->cid;
	}
	ss << " (score=" << total_score << ")" << std::endl;
	FLUSHED_CONSOLE_MSG(ss.str())
}

bool checkPerfectMatchingValidity(unsigned int ncliques, const std::vector<unsigned int> &matching)
{
	std::vector<std::pair<bool, bool>> clique_flags(ncliques, std::make_pair(false, false));
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



}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////



void tests_NonMatchingGraph()
{
	ANNONCE_TEXT_BLOCK
	EXECUTE_TEST("DirectCreation",test_NonMatchingGraph_DirectCreation)
	EXECUTE_TEST("Deserialization",test_NonMatchingGraph_Deserialization)
}

bool test_NonMatchingGraph_DirectCreation()
{
	wpm::NonMatchingGraph g;
	g.startNewGraph(5);
	g.addUndirectedEdge(0, 4, -1);
	g.addUndirectedEdge(0, 2, -1);
	g.addUndirectedEdge(1, 3, -1);
	g.addDirectedEdge(4, 1, 1);
	g.addDirectedEdge(1, 2, 1);
	g.addDirectedEdge(2, 4, 1);
	g.addDirectedEdge(3, 0, 1);
	g.addDirectedEdge(0, 3, 1);
	std::stringstream ss;
	ss << g;
	//FLUSHED_CONSOLE_MSG(ss.str() << std::endl)
	return (ss.str()=="nv 5\nv 0 4(4294967295) 2(4294967295) 3(1)\nv 1 3(4294967295) 2(1)\nv 2 0(4294967295) 4(1)\nv 3 1(4294967295) 0(1)\nv 4 0(4294967295) 1(1)\n");
}

bool test_NonMatchingGraph_Deserialization()
{
	const char* graph_str = "nv 5\nv 0 4(4294967295) 2(4294967295) 3(1)\nv 1 3(4294967295) 2(1)\nv 2 0(4294967295) 4(1)\nv 3 1(4294967295) 0(1)\nv 4 0(4294967295) 1(1)\n";
	std::istringstream is(graph_str);
	wpm::NonMatchingGraph g;
	g.deserialize(is);
	std::stringstream ss;
	ss << g;
	//FLUSHED_CONSOLE_MSG(ss.str() << std::endl)
	return (ss.str()==std::string(graph_str));
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////



void tests_BipartiteMatchingGraph()
{
	ANNONCE_TEXT_BLOCK
	EXECUTE_TEST("DirectCreation", test_BipartiteMatchingGraph_DirectCreation)
	EXECUTE_TEST("Deserialization", test_BipartiteMatchingGraph_Deserialization)
}

bool test_BipartiteMatchingGraph_DirectCreation()
{
	wpm::BipartiteMatchingGraph g;
	g.startNewGraph(5);
	g.addDirectedEdge(0, 1, 0);
	g.addDirectedEdge(0, 3, -1);
	g.addDirectedEdge(1, 0, 0);
	g.addDirectedEdge(1, 2, -1);
	g.addDirectedEdge(1, 4, 0);
	g.addDirectedEdge(2, 1, 0);
	g.addDirectedEdge(2, 3, 0);
	g.addDirectedEdge(2, 4, -1);
	g.addDirectedEdge(3, 0, -1);
	g.addDirectedEdge(3, 2, 0);
	g.addDirectedEdge(3, 4, 0);
	g.addDirectedEdge(4, 1, -1);
	g.addDirectedEdge(4, 2, 0);
	g.addDirectedEdge(4, 3, 0);
	std::stringstream ss;
	ss << g;
	//FLUSHED_CONSOLE_MSG(ss.str() << std::endl)
	return (ss.str()=="nc 5\nne 14\ne 0 1 0\ne 0 3 -1\ne 1 0 0\ne 1 2 -1\ne 1 4 0\ne 2 1 0\ne 2 3 0\ne 2 4 -1\ne 3 0 -1\ne 3 2 0\ne 3 4 0\ne 4 1 -1\ne 4 2 0\ne 4 3 0\n");
}

bool test_BipartiteMatchingGraph_Deserialization()
{
	const char* graph_str = "nc 5\nne 14\ne 0 1 0\ne 0 3 -1\ne 1 0 0\ne 1 2 -1\ne 1 4 0\ne 2 1 0\ne 2 3 0\ne 2 4 -1\ne 3 0 -1\ne 3 2 0\ne 3 4 0\ne 4 1 -1\ne 4 2 0\ne 4 3 0\n";
	std::istringstream is(graph_str);
	wpm::BipartiteMatchingGraph g;
	g.deserialize(is);
	std::stringstream ss;
	ss << g;
	//FLUSHED_CONSOLE_MSG(ss.str() << std::endl)
	return (ss.str()==std::string(graph_str));
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////



void tests_MatchingGraphConverter()
{

	EXECUTE_TEST("NM2BMConversion", test_MatchingGraphConverter_NM2BMConversion)
	EXECUTE_TEST("BM2NMConversion", test_MatchingGraphConverter_BM2NMConversion)
}

bool test_MatchingGraphConverter_NM2BMConversion()
{
	const char* nmgraph_str = "nv 5\nv 0 4(4294967295) 2(4294967295) 3(1)\nv 1 3(4294967295) 2(1)\nv 2 0(4294967295) 4(1)\nv 3 1(4294967295) 0(1)\nv 4 0(4294967295) 1(1)\n";
	const char* bmgraph_str = "nc 5\nne 14\ne 0 1 0\ne 0 3 -1\ne 1 0 0\ne 1 2 -1\ne 1 4 0\ne 2 1 0\ne 2 3 0\ne 2 4 -1\ne 3 0 -1\ne 3 2 0\ne 3 4 0\ne 4 1 -1\ne 4 2 0\ne 4 3 0\n";
	// Deserialize the NonMatchingGraph
	std::istringstream is(nmgraph_str);
	wpm::NonMatchingGraph nmg;
	nmg.deserialize(is);
	// Convert it into a BipartiteMatchingGraph
	wpm::MatchingGraphConverter gconverter;
	wpm::BipartiteMatchingGraph bmg;
	gconverter.toBipartiteMatchingGraph(nmg, bmg);
	// Serialize the obtained BipartiteMatchingGraph
	std::stringstream ss;
	ss << bmg;
	//FLUSHED_CONSOLE_MSG(ss.str() << std::endl)
	return (ss.str()==std::string(bmgraph_str));
}

bool test_MatchingGraphConverter_BM2NMConversion()
{
	const char* bmgraph_str = "nc 5\nne 14\ne 0 1 0\ne 0 3 -1\ne 1 0 0\ne 1 2 -1\ne 1 4 0\ne 2 1 0\ne 2 3 0\ne 2 4 -1\ne 3 0 -1\ne 3 2 0\ne 3 4 0\ne 4 1 -1\ne 4 2 0\ne 4 3 0\n";
	const char* nmgraph_str = "nv 5\nv 0 2(4294967295) 3(1) 4(4294967295)\nv 1 2(1) 3(4294967295)\nv 2 0(4294967295) 4(1)\nv 3 0(1) 1(4294967295)\nv 4 0(4294967295) 1(1)\n";
	// Deserialize the BipartiteMatchingGraph
	std::istringstream is(bmgraph_str);
	wpm::BipartiteMatchingGraph bmg;
	bmg.deserialize(is);
	// Convert it into a BipartiteMatchingGraph
	wpm::MatchingGraphConverter gconverter;
	wpm::NonMatchingGraph nmg;
	gconverter.toNonMatchingGraph(bmg, nmg);
	// Serialize the obtained BipartiteMatchingGraph
	std::stringstream ss;
	ss << nmg;
	//FLUSHED_CONSOLE_MSG(ss.str() << std::endl)
	return (ss.str()==std::string(nmgraph_str));
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////



void tests_PerfectMatchingFinder()
{
	ANNONCE_TEXT_BLOCK
	EXECUTE_TEST("FindRandomValidMatch", test_PerfectMatchingFinder_FindRandomValidMatch)
	EXECUTE_TEST("FindSuccessiveMatch", test_PerfectMatchingFinder_FindSuccessiveMatch)
	EXECUTE_TEST("FindHighestWeightMatch", test_PerfectMatchingFinder_FindHighestWeightMatch)
}

bool test_PerfectMatchingFinder_FindRandomValidMatch()
{
	const char* graph_str = "nc 5\nne 14\ne 0 1 2\ne 0 3 1\ne 1 0 2\ne 1 2 1\ne 1 4 2\ne 2 1 2\ne 2 3 2\ne 2 4 1\ne 3 0 1\ne 3 2 2\ne 3 4 2\ne 4 1 1\ne 4 2 2\ne 4 3 2\n";
	// Deserialize the BipartiteMatchingGraph
	std::istringstream is(graph_str);
	wpm::BipartiteMatchingGraph bmg;
	bmg.deserialize(is);
	// Find random matching
	wpm::PerfectMatchingFinder pmfinder;
	std::vector<const wpm::BipartiteMatchingGraph::Edge*> bmg_matching;
	pmfinder.findRandomPerfectMatching(bmg, bmg_matching);
	// Display the result
	//impl::displayBMGMatching(bmg_matching);
	// Convert into a generic matching
	wpm::MatchingGraphConverter gconverter;
	std::vector<unsigned int> generic_matching;
	gconverter.toGenericMatching(bmg_matching, generic_matching);
	// Check that the perfect matching is valid
	return impl::checkPerfectMatchingValidity(bmg.getNumberCliques(), generic_matching);
}

bool test_PerfectMatchingFinder_FindSuccessiveMatch()
{
	const char* nmgraph_str = "nv 5\nv 0 2(4294967295) 4(4294967295)\nv 1 3(4294967295)\nv 2 0(4294967295)\nv 3 1(4294967295)\nv 4 0(4294967295)\n";
	// Deserialize the NonMatchingGraph
	std::istringstream is(nmgraph_str);
	wpm::NonMatchingGraph nmg;
	nmg.deserialize(is);
	// Find four successive matchings
	const unsigned int niterations = 10;
	std::vector<unsigned int> matching[niterations];
	bool success = true;
	for(unsigned int i=0; i<niterations; ++i) {
		// Convert the non-matching graph into a bipartite matching graph
		wpm::MatchingGraphConverter gconverter;
		wpm::BipartiteMatchingGraph bmg;
		gconverter.toBipartiteMatchingGraph(nmg, bmg);
		// Find a random perfect matching
		wpm::PerfectMatchingFinder pmfinder;
		std::vector<const wpm::BipartiteMatchingGraph::Edge*> bmg_matching;
		success = pmfinder.findRandomPerfectMatching(bmg, bmg_matching);
		if(!success)
			break;
		// Convert it to a generic matching and check its validity
		gconverter.toGenericMatching(bmg_matching, matching[i]);
		success = impl::checkPerfectMatchingValidity(bmg.getNumberCliques(), matching[i]);
		if(!success)
			break;
		// Update the constraints in the non-matching graph
		nmg.updateConstraints(matching[i],true);
		//impl::displayBMGMatching(bmg_matching);
		//impl::displayNonMatchingGraph(nmg);
	}
	return success;
}

bool test_PerfectMatchingFinder_FindHighestWeightMatch()
{
	const char* graph_str = "nc 3\nne 6\ne 0 1 -5\ne 0 2 -20\ne 1 0 0\ne 1 2 -5\ne 2 0 -5\ne 2 1 0\n";
	// Deserialize the BipartiteMatchingGraph
	std::istringstream is(graph_str);
	wpm::BipartiteMatchingGraph bmg;
	bmg.deserialize(is);
	// Find random matching
	wpm::PerfectMatchingFinder pmfinder;
	std::vector<const wpm::BipartiteMatchingGraph::Edge*> bmg_matching;
	pmfinder.findRandomPerfectMatching(bmg, bmg_matching);
	// Compute the score of the matching
	double total_score = 0;
	for(std::vector<const wpm::BipartiteMatchingGraph::Edge*>::const_iterator it_e=bmg_matching.begin(); it_e!=bmg_matching.end(); ++it_e)
		total_score += (*it_e)->score;
	// Display the result
	//impl::displayBMGMatching(bmg_matching);
	// Convert into a generic matching
	wpm::MatchingGraphConverter gconverter;
	std::vector<unsigned int> generic_matching;
	gconverter.toGenericMatching(bmg_matching, generic_matching);
	// Check that the perfect matching is valid
	return (impl::checkPerfectMatchingValidity(bmg.getNumberCliques(), generic_matching) && total_score==-15);
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////



void tests_QuantitativeEvaluations()
{
	ANNONCE_TEXT_BLOCK
	EXECUTE_TEST("TimeTilKnownMatching", test_QuantitativeEvaluations_TimeTilKnownMatching)
	EXECUTE_TEST("MatchingProbabilities", test_QuantitativeEvaluations_MatchingProbabilities)
}

bool test_QuantitativeEvaluations_TimeTilKnownMatching()
{
	// Build graph string
	std::ostringstream ss;
	const unsigned int nvertices = 8;
	ss << "nv " << nvertices << "\n";
	for(unsigned int i=0; i<nvertices/2; ++i) {
		ss << "v " << 2*i << " " << 2*i+1 << "(" << (unsigned int)(-1) << ")\n";
		ss << "v " << 2*i+1 << " " << 2*i << "(" << (unsigned int)(-1) << ")\n";
	}
	std::string graph_str = ss.str();
	// Initialize the statistics counters
	const unsigned int n_mc_iterations = 100;
	unsigned int tmin=std::numeric_limits<unsigned int>::max(), tmax=0;
	unsigned int tsum=0, tsum2=0;
	// Monte-Carlo drawing
	for(unsigned int i=0; i<n_mc_iterations; ++i) {

		// Deserialize the NonMatchingGraph
		std::istringstream is(graph_str);
		wpm::NonMatchingGraph nmg;
		nmg.deserialize(is);

		// Compute initial matching
		wpm::BipartiteMatchingGraph bmg;
		wpm::MatchingGraphConverter converter;
		converter.toBipartiteMatchingGraph(nmg, bmg);
		wpm::PerfectMatchingFinder pmfinder;
		std::vector<const wpm::BipartiteMatchingGraph::Edge*> tmp_matching;
		pmfinder.findRandomPerfectMatching(bmg,tmp_matching);
		std::vector<unsigned int> initial_matching;
		converter.toGenericMatching(tmp_matching, initial_matching);

		// Update the graph and find successive matchings until the initial one is obtained again
		std::vector<unsigned int> new_matching(initial_matching);
		unsigned int t = 0;
		bool is_initial_matching_obtained_again = false;
		do {
			// Update the NonMatchingGraph and find a new matching
			nmg.updateConstraints(new_matching, true);
			converter.toBipartiteMatchingGraph(nmg, bmg);
			pmfinder.findRandomPerfectMatching(bmg, tmp_matching);
			converter.toGenericMatching(tmp_matching, new_matching);

			// Check whether the new matching is the same than the initial matching
			bool is_same_matching = true;
			for(unsigned int idx=0; idx<nvertices; ++idx) {
				if(new_matching[idx]!=initial_matching[idx]) {
					is_same_matching = false;
					break;
				}
			}
			if(is_same_matching)
				is_initial_matching_obtained_again = true;
			else ++t;

		} while(!is_initial_matching_obtained_again);

		// Update the statistics counters
		tsum += t;
		tsum2 += t*t;
		if(t<tmin) tmin = t;
		if(t>tmax) tmax = t;
	}

	// Display the final statistics
	double tavg=tsum/(double)n_mc_iterations, tsdv=std::sqrt(tsum2/(double)n_mc_iterations-tavg*tavg);
	std::cout << "Time until known matching (" << nvertices/2 << " pairs of participants, " << n_mc_iterations << " iterations):" << std::endl;
	std::cout << "   avg=" << tavg << std::endl;
	std::cout << "   sdv=" << tsdv << std::endl;
	std::cout << "   min=" << tmin << std::endl;
	std::cout << "   max=" << tmax << std::endl;

	return (tavg>nvertices);
}

bool test_QuantitativeEvaluations_MatchingProbabilities()
{
	// Build graph string
	std::ostringstream ss;
	const unsigned int nvertices = 10;
	ss << "nv " << nvertices << "\n";
	for(unsigned int i=0; i<nvertices/2; ++i) {
		ss << "v " << 2*i << " " << 2*i+1 << "(" << (unsigned int)(-1) << ")\n";
		ss << "v " << 2*i+1 << " " << 2*i << "(" << (unsigned int)(-1) << ")\n";
	}
	std::string graph_str = ss.str();

	// Initialize the statistics counters
	const unsigned int n_successive_matches = 1000;
	std::vector<double> histo(nvertices*nvertices,0);

	// Deserialize the NonMatchingGraph
	std::istringstream is(graph_str);
	wpm::NonMatchingGraph nmg;
	nmg.deserialize(is);

	// Compute N successive matchings and update the matching histogram
	for(unsigned int i=0; i<n_successive_matches; ++i) {
		// Compute the next matching and update the graph
		wpm::BipartiteMatchingGraph bmg;
		wpm::MatchingGraphConverter converter;
		converter.toBipartiteMatchingGraph(nmg, bmg);
		wpm::PerfectMatchingFinder pmfinder;
		std::vector<const wpm::BipartiteMatchingGraph::Edge*> tmp_matching;
		pmfinder.findRandomPerfectMatching(bmg, tmp_matching);
		std::vector<unsigned int> matching;
		converter.toGenericMatching(tmp_matching, matching);
		nmg.updateConstraints(matching, true);
		// Update the histogram
		for(unsigned int isrc=0; isrc<nvertices; ++isrc) {
			unsigned int bin = isrc*nvertices+matching[isrc];
			++histo[bin];
		}
	}

	// Display the final histogram
	std::vector<std::string> err_msgs;
	std::cout << "Matching histogram (" << nvertices/2 << " pairs of participants, " << n_successive_matches << " successive matches):" << std::endl;
	std::cout << "     |";
	for(unsigned int itgt=0; itgt<nvertices; ++itgt)
		std::cout << "    " << itgt << "    |";
	std::cout << std::endl;
	std::streamsize old_precision_value = std::cout.precision();
	std::cout.precision(5);
	std::cout << std::fixed;
	for(unsigned int isrc=0; isrc<nvertices; ++isrc) {
		std::cout << isrc << " -> |";
		for(unsigned int itgt=0; itgt<nvertices; ++itgt) {
			if(histo[isrc*nvertices+itgt]==0)
				std::cout << " 0.00000 |";
			else std::cout << " " << histo[isrc*nvertices+itgt]/(double)n_successive_matches << " |";
			if(itgt!=isrc && itgt!=(2*(isrc/2)+(isrc%2==0?1:0)) && std::abs(histo[isrc*nvertices+itgt]/(double)n_successive_matches-1./(nvertices-2))>1e-3) ENQUEUE_ERROR_MSG(err_msgs, "Biaised matching likelihood from "<<isrc<<" to "<<itgt<<"!")
		}
		std::cout << std::endl;
		if(histo[isrc*nvertices+isrc]>0) ENQUEUE_ERROR_MSG(err_msgs, "Vertex "<<isrc<<" was matched with itself!")
		if(histo[isrc*nvertices+(2*(isrc/2)+(isrc%2==0?1:0))]>0) ENQUEUE_ERROR_MSG(err_msgs, "Vertex "<<isrc<<" was matched with its unmatchable vertex!")
	}
	std::cout.precision(old_precision_value);

	// Display error messages if any have been generated
	if(!err_msgs.empty()) {
		std::cout << "Error messages:" << std::endl;
		for(std::vector<std::string>::const_iterator it_em=err_msgs.begin(); it_em!=err_msgs.end(); ++it_em)
			std::cout << " - " << *it_em << std::endl;
	}

	return err_msgs.empty();
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
