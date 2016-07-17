/* MatchingGraphConverter.cpp (created on 05/06/2016 by Nicolas) */



#include <sstream>
#include "NonMatchingGraph.h"
#include "BipartiteMatchingGraph.h"
#include "MatchingGraphConverter.h"
#include "WPMASSERT.h"



namespace wpm {



///////////////////////////////////////////////////////////////////////////////////////////////////////////



MatchingGraphConverter::MatchingGraphConverter()
{
}

MatchingGraphConverter::~MatchingGraphConverter()
{
}



namespace impl {

//! Converts the cost of a non-matching edge into the score of a bipartite matching edge.
inline int convertNMCostToBMScore(unsigned int nmcost)
{
	if(nmcost==(unsigned int)(-1))
		return std::numeric_limits<int>::min();
	return -(int)nmcost;
}

//! Converts the score of a bipartite matching edge into the cost of a non-matching edge.
inline unsigned int convertBMScoreToNMCost(int bmscore)
{
	if(bmscore==std::numeric_limits<int>::min())
		return (unsigned int)(-1);
	return (unsigned int)(-bmscore);
}

}



void MatchingGraphConverter::toBipartiteMatchingGraph(const NonMatchingGraph &nmgraph, BipartiteMatchingGraph &bmgraph) const
{
	// Initialize the BipartiteMatchingGraph
	unsigned int ncliques = nmgraph.getNumberVertices();
	bmgraph.startNewGraph(ncliques);
	// Loop over each vertex of the NonMatchingGraph and enumerate the matching candidates based on the non-matching constraints
	std::vector<NonMatchingGraph::Vertex>::const_iterator it_nmv, it_nmv_end;
	nmgraph.getVertexIterators(it_nmv, it_nmv_end);
	for(; it_nmv!=it_nmv_end; ++it_nmv) {
		// Eliminate the matching candidates for which a non-matching constraint exist
		std::vector<int> score_matching_candidates(ncliques, 0);
		score_matching_candidates[it_nmv->id] = std::numeric_limits<int>::min();
		for(std::vector<NonMatchingGraph::Edge>::const_iterator it_e=it_nmv->constraints.begin(); it_e!=it_nmv->constraints.end(); ++it_e)
			score_matching_candidates[it_e->v_target->id] = impl::convertNMCostToBMScore(it_e->cost);
		// Add one edge in the BipartiteMatchingGraph for each matching candidate
		for(unsigned int cid=0; cid<ncliques; ++cid) {
			if(score_matching_candidates[cid]!=std::numeric_limits<int>::min())
				bmgraph.addDirectedEdge(it_nmv->id, cid, score_matching_candidates[cid]);
		}
	}
}



namespace impl {

//! Analyzes the edges incident to the input vertex to infer the corresponding non-matching edges in the NonMatchingGraph.
void bmVertexEdgesToNMEdges(const BipartiteMatchingGraph::Vertex &v, NonMatchingGraph &nmgraph, bool is_source_vertex)
{
	// Loop over all the matching edges incident to the current vertex
	unsigned int nvertices = nmgraph.getNumberVertices();
	std::vector<unsigned int> costs(nvertices, (unsigned int)(-1));
	for(std::vector<BipartiteMatchingGraph::Edge*>::const_iterator it_e=v.edges.begin(); it_e!=v.edges.end(); ++it_e) {
		unsigned int cid_match = (is_source_vertex ? (*it_e)->v_target->parent->cid : (*it_e)->v_source->parent->cid);
		costs[cid_match] = impl::convertBMScoreToNMCost((*it_e)->score);
	}
	// Add edges towards the vertices associated with a strictly positive cost
	for(unsigned int id=0; id<nvertices; ++id) {
		if(id==v.parent->cid || costs[id]==0)	// Checks that the edge is a valid non-matching constraint
			continue;
		if(id<v.parent->cid)	// Avoids adding the same edge twice
			continue;
		if(is_source_vertex)
			nmgraph.addDirectedEdge(v.parent->cid, id, costs[id]);
		else nmgraph.addDirectedEdge(id, v.parent->cid, costs[id]);
	}
}

}



void MatchingGraphConverter::toNonMatchingGraph(const BipartiteMatchingGraph &bmgraph, NonMatchingGraph &nmgraph) const
{
	// Initialize the NonMatchingGraph
	unsigned int nvertices = bmgraph.getNumberCliques();
	nmgraph.startNewGraph(nvertices);
	// Loop over each vertex of the BipartiteMatchingGraph and enumerate the non-matching constraints based on the existing matching edges
	std::vector<BipartiteMatchingGraph::Clique>::const_iterator it_bmc, it_bmc_end;
	bmgraph.getCliqueIterators(it_bmc, it_bmc_end);
	for(; it_bmc!=it_bmc_end; ++it_bmc) {
		impl::bmVertexEdgesToNMEdges(it_bmc->v_source, nmgraph, true);
		impl::bmVertexEdgesToNMEdges(it_bmc->v_target, nmgraph, false);
	}
}

void MatchingGraphConverter::toGenericMatching(const std::vector<const BipartiteMatchingGraph::Edge*> &bmg_perfect_matching, std::vector<unsigned int> &generic_perfect_matching) const
{
	unsigned int size = bmg_perfect_matching.size();
	generic_perfect_matching.resize(size);
	for(unsigned int i=0; i<size; ++i) {
		unsigned int source_id = bmg_perfect_matching[i]->v_source->parent->cid;
		unsigned int target_id = bmg_perfect_matching[i]->v_target->parent->cid;
		WPMASSERT(source_id<size,"The input matching is not perfect!");
		generic_perfect_matching[source_id] = target_id;
	}
}

void MatchingGraphConverter::toStringMatching(const std::vector<unsigned int> &generic_matching, std::string &matching_str) const
{
	std::stringstream ss;
	for(std::vector<unsigned int>::const_iterator it_m=generic_matching.begin(); it_m!=generic_matching.end(); ++it_m)
		ss << (it_m==generic_matching.begin()?"":",") << *it_m;
	matching_str = ss.str();
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////



} //namespace wpm
