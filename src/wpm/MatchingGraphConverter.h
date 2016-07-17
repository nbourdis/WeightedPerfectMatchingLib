/* MatchingGraphConverter.h (created on 05/06/2016 by Nicolas) */

#ifndef MATCHING_GRAPH_CONVERTER_H
#define MATCHING_GRAPH_CONVERTER_H



namespace wpm {



// Predeclarations
class NonMatchingGraph;
class BipartiteMatchingGraph;



//! MatchingGraphConverter class, in charge of converting between different graph or matching formats.
class MatchingGraphConverter
{
public:

	//! Default constructor.
	MatchingGraphConverter();
	//! Destructor.
	~MatchingGraphConverter();

	//! Function to convert a NonMatchingGraph into a BipartiteMatchingGraph.
	void toBipartiteMatchingGraph(const NonMatchingGraph &nmgraph, BipartiteMatchingGraph &bmgraph) const;

	//! Function to convert a BipartiteMatchingGraph into a NonMatchingGraph.
	void toNonMatchingGraph(const BipartiteMatchingGraph &bmgraph, NonMatchingGraph &nmgraph) const;

	//! Function to convert a matching on a BipartiteMatchingGraph into a generic matching.
	void toGenericMatching(const std::vector<const BipartiteMatchingGraph::Edge*> &bmg_matching, std::vector<unsigned int> &generic_matching) const;

	//! Function to convert a generic matching into a string matching.
	void toStringMatching(const std::vector<unsigned int> &generic_matching, std::string &matching_str) const;
};



}	// namespace wpm



#endif //MATCHING_GRAPH_CONVERTER_H
