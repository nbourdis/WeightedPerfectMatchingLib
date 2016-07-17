/* PerfectMatchingFinder.h (created on 05/06/2016 by Nicolas) */

#ifndef PERFECT_MATCHING_FINDER_H
#define PERFECT_MATCHING_FINDER_H



namespace wpm {



// Forward declarations
class NonMatchingGraph;
class BipartiteMatchingGraph;



/*! PerfectMatchingFinder class, in charge of finding a perfect matching with maximal score in a BipartiteMatchingGraph.
 *  This class implements the Hungarian algorithm (see https://en.wikipedia.org/wiki/Hungarian_algorithm).
 */
class PerfectMatchingFinder
{
public:

	//! Default constructor.
	PerfectMatchingFinder();
	//! Destructor.
	~PerfectMatchingFinder();

	//! Function to find a perfect matching in a BipartiteMatchingGraph, selected randomly among those with maximal score.
	bool findRandomPerfectMatching(const BipartiteMatchingGraph &bmgraph,
								   std::vector<const BipartiteMatchingGraph::Edge*> &matching);
};



}	// namespace wpm



#endif //PERFECT_MATCHING_FINDER_H
