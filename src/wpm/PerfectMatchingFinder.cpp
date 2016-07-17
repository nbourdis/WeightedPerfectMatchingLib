/* PerfectMatchingFinder.cpp (created on 05/06/2016 by Nicolas) */



#include <algorithm>
#include <queue>
#include <sstream>
#include "WPMASSERT.h"
#include "NonMatchingGraph.h"
#include "BipartiteMatchingGraph.h"
#include "PerfectMatchingFinder.h"



namespace wpm {



// Convenience typedefs
typedef BipartiteMatchingGraph::Edge Edge;
typedef BipartiteMatchingGraph::Vertex Vertex;
typedef BipartiteMatchingGraph::Clique Clique;



/////////////////////////////////////////////////////////////////////////////////////////////////////////////



namespace impl {


// Forward declarations
class SlackArray;
class VertexLabeling;
class Matching;
class AlternatingTree;



const bool VERBOSE = false;



//! Internal structure representing a matching over the vertex in the BipartiteMatchingGraph.
class Matching
{
private:

	std::vector<unsigned int> _source_matches;	//!< Array containing, for each source vertex, the clique ID of the matched target vertex if it exists and -1 otherwise.
	std::vector<const Edge*> _source_edges;		//!< Array containing, for each source vertex, the associated edge in the matching if it exists and NULL otherwise.
	std::vector<unsigned int> _target_matches;	//!< Array containing, for each target vertex, the clique ID of the matched source vertex if it exists and -1 otherwise.
	std::vector<const Edge*> _target_edges;		//!< Array containing, for each target vertex, the associated edge in the matching if it exists and NULL otherwise.
	std::vector<const Edge*> _edges;			//!< Array of all the edges in the matching.
	unsigned int _ncliques;						//!< Number of cliques in the graph.

public:

	//! Constructor, initializing to the empty matching.
	Matching(size_t ncliques)
	{
		_ncliques = ncliques;
		reset();
	}

	//! Function to return the number of edges in the matching.
	inline size_t getNumberEdges() const { return _edges.size(); }

	//! Function to indicate whether the clique with specified ID is a source in this matching.
	inline bool isSource(unsigned int source_cid) const { return (_source_matches[source_cid]!=(unsigned int)(-1)); }

	//! Function to indicate whether the clique with specified ID is a target in this matching.
	inline bool isTarget(unsigned int target_cid) const { return (_target_matches[target_cid]!=(unsigned int)(-1)); }

	//! Function to retrieve the clique ID of the matched target vertex for the input source vertex.
	inline unsigned int getMatchedTarget(unsigned int source_cid) const { return _source_matches[source_cid]; }

	//! Function to retrieve the edge in the matching for the input source vertex.
	inline const Edge* getEdgeFromSource(unsigned int source_cid) const { return _source_edges[source_cid]; }

	//! Function to retrieve the clique ID of the matched source vertex for the input target vertex.
	inline unsigned int getMatchedSource(unsigned int target_cid) const { return _target_matches[target_cid]; }

	//! Function to retrieve the edge in the matching for the input target vertex.
	inline const Edge* getEdgeFromTarget(unsigned int target_cid) const { return _target_edges[target_cid]; }

	//! Function to retrieve the set of edges in the matching.
	void getMatchingEdges(std::vector<const Edge*> &edges) const
	{
		edges.resize(_edges.size());
		std::copy(_edges.begin(), _edges.end(), edges.begin());
	}

	//! Function to reset the matching.
	void reset()
	{
		_source_matches.resize(_ncliques, (unsigned int)(-1));
		_source_edges.resize(_ncliques, NULL);
		_target_matches.resize(_ncliques, (unsigned int)(-1));
		_target_edges.resize(_ncliques, NULL);
		_edges.clear();
		_edges.reserve(_ncliques);
	}

	//! Function to add a new edge to the matching.
	void addEdge(const Edge *edge)
	{
		_edges.push_back(edge);
		unsigned int s = edge->v_source->parent->cid;
		unsigned int t = edge->v_target->parent->cid;
		WPMASSERT(!isSource(s), "The source vertex of this edge is already matched!");
		WPMASSERT(!isTarget(t), "The target vertex of this edge is already matched!");
		_source_matches[s] = t;
		_target_matches[t] = s;
		_source_edges[s] = edge;
		_target_edges[t] = edge;
	}

	//! Function to remove an edge from the matching.
	void removeEdge(const Edge *edge)
	{
		// Find the edge to be removed
		std::vector<const Edge*>::iterator it_e = _edges.begin();
		for(; it_e!=_edges.end(); ++it_e) {
			if(*it_e==edge)
				break;
		}
		if(it_e==_edges.end())
			return;
		// Remove the edge from the matching
		_edges.erase(it_e);
		// Update the matched vertex and edge sets
		unsigned int s = edge->v_source->parent->cid;
		if(_source_edges[s]==edge) {
			_source_matches[s] = (unsigned int)(-1);
			_source_edges[s] = NULL;
		}
		unsigned int t = edge->v_target->parent->cid;
		if(_target_edges[t]==edge) {
			_target_matches[t] = (unsigned int)(-1);
			_target_edges[t] = NULL;
		}
	}

	//! Function to convert the matching into a human-readable string.
	std::string serialize() const
	{
		std::stringstream ss;
		for(unsigned int source_cid=0; source_cid<_ncliques; ++source_cid) {
			if(_source_matches[source_cid]!=(unsigned int)(-1))
				ss << (source_cid>0?", ":"") << "s" << source_cid << "->t" << _source_matches[source_cid];
		}
		return ss.str();
	}
};

/*! Internal structure memorizing the intermediary matched vertices until an augmenting path is found.
 *  Once an exposed target vertex is found, this tree structure enables efficiently computing the path
 *  to the root exposed source vertex. Also, it is called alternating because the edges along the final
 *  path are alternatingly inside and outside the matching.
 */
class AlternatingTree
{
private:

	std::vector<bool> _set_S;	//!< Set S, containing the source vertices from which candidate exposed target vertices are searched.
	std::vector<bool> _set_T;	//!< Set T, containing the target vertices which have been checked while looking for an exposed target vertex.
	std::vector<unsigned int> _source_prev;	//!< For each source vertex, contains the previous target vertex along the alternating tree.
	std::vector<unsigned int> _target_prev;	//!< For each target vertex, contains the previous source vertex along the alternating tree.
	std::vector<const Edge*> _source_prev_edge;	//!< For each source vertex, contains the pointer to the previous edge in the alternating tree if it exists or NULL otherwise.
	std::vector<const Edge*> _target_prev_edge;	//!< For each target vertex, contains the pointer to the previous edge in the alternating tree if it exists or NULL otherwise.
	unsigned int _root_exposed_source_vertex;	//!< Clique ID for the root exposed source vertex.
	unsigned int _end_exposed_target_vertex;	//!< Clique ID for the end exposed target vertex.

public:

	//! Constructor, initializing the root source vertex of the alternating tree.
	AlternatingTree(unsigned int ncliques, unsigned int cid_root_exposed_source_vertex)
	{
		_set_S.resize(ncliques, false);
		_set_T.resize(ncliques, false);
		_source_prev.resize(ncliques, -1);
		_source_prev_edge.resize(ncliques, NULL);
		_target_prev.resize(ncliques, -1);
		_target_prev_edge.resize(ncliques, NULL);
		_root_exposed_source_vertex = cid_root_exposed_source_vertex;
		_end_exposed_target_vertex = (unsigned int)(-1);
		_set_S[_root_exposed_source_vertex] = true;
	}

	//! Adds edges (s1,t) and (t,s2) to the alternating tree.
	void addTwoEdges(unsigned int cid_s1, unsigned int cid_t, unsigned int cid_s2, const Edge *edge_s1_t, const Edge *edge_t_s2)
	{
		WPMASSERT(_set_S[cid_s1]==true,"Adding edge from a source vertex which is not in S!");	// s1 should already be in S
		_set_T[cid_t] = true;
		_set_S[cid_s2] = true;
		_source_prev[cid_s2] = cid_t;
		_source_prev_edge[cid_s2] = edge_t_s2;
		_target_prev[cid_t] = cid_s1;
		_target_prev_edge[cid_t] = edge_s1_t;
	}

	//! Function to add the input target vertex to set T.
	void addSingleEdge(unsigned int cid_s1, unsigned int cid_t, const Edge *edge_s1_t)
	{
		WPMASSERT(_set_S[cid_s1]==true, "Adding edge from a source vertex which is not in S!");	// s1 should already be in S
		_set_T[cid_t] = true;
		_target_prev[cid_t] = cid_s1;
		_target_prev_edge[cid_t] = edge_s1_t;
	}

	//! Function to set the end exposed target vertex, which defines the augmenting path.
	void setEndExposedTargetVertex(unsigned int cid_s1, unsigned int cid_t, const Edge* edge_s1_t)
	{
		_end_exposed_target_vertex = cid_t;
		_target_prev[cid_t] = cid_s1;
		_target_prev_edge[cid_t] = edge_s1_t;
	}

	//! Function to check whether an augmenting path has been found.
	inline bool isAugmentingPathFound() { return (_end_exposed_target_vertex!=(unsigned int)(-1)); }

	//! Function to test whether the input source vertex is in the set S.
	inline bool isInS(unsigned int source_cid) const { return _set_S[source_cid]; }
	//! Function to test whether the input target vertex is in the set T.
	inline bool isInT(unsigned int target_cid) const { return _set_T[target_cid]; }

	//! Function to apply the augmenting path to augment the specified matching.
	void applyAugmentingPath(impl::Matching &matching)
	{
		// Preconditions, in particular an augmenting path must have been found
		WPMASSERT(_end_exposed_target_vertex!=(unsigned int)(-1), "End target vertex is not defined!");
		WPMASSERT(!matching.isSource(_root_exposed_source_vertex),"Root source vertex is not an exposed vertex!");
		WPMASSERT(!matching.isTarget(_end_exposed_target_vertex), "End target vertex is not an exposed vertex!");
		unsigned int n_edges_matches_before = matching.getNumberEdges();
		// Loop over all edges in the augmenting path, to determine the edges to remove from the matching and those to add in the matching
		std::vector<const Edge*> edges_to_be_added, edges_to_be_removed;
		unsigned int crt_target_cid = _end_exposed_target_vertex;
		do {
			const Edge* edge_s_t1 = _target_prev_edge[crt_target_cid];
			unsigned int matched_source_cid = _target_prev[crt_target_cid];
			WPMASSERT(matched_source_cid!=(unsigned int)(-1), "Current target vertex has no previous source vertex!");
			WPMASSERT(edge_s_t1!=NULL, "Current target vertex has no associated edge!");
			edges_to_be_added.push_back(edge_s_t1);
			crt_target_cid = _source_prev[matched_source_cid];
			if(crt_target_cid!=(unsigned int)(-1)) {
				const Edge* edge_t2_s = _source_prev_edge[matched_source_cid];
				WPMASSERT(edge_t2_s!=NULL, "Current source vertex has no associated edge!");
				edges_to_be_removed.push_back(edge_t2_s);
			}
		} while(crt_target_cid!=(unsigned int)(-1));
		// Remove the edges to be removed
		for(std::vector<const Edge*>::const_iterator it_e=edges_to_be_removed.begin(); it_e!=edges_to_be_removed.end(); ++it_e)
			matching.removeEdge(*it_e);
		// Add the edges to be added
		for(std::vector<const Edge*>::const_iterator it_e=edges_to_be_added.begin(); it_e!=edges_to_be_added.end(); ++it_e)
			matching.addEdge(*it_e);
		WPMASSERT(matching.getNumberEdges()==n_edges_matches_before+1, "Applying the augmenting path did not augment the matching!");
	}

	//! Function to convert the augmenting path into a human-readable string.
	std::string serializeAugmentingPath() const
	{
		if(_end_exposed_target_vertex==(unsigned int)(-1))
			return "not-found";
		std::stringstream ss;
		unsigned int crt_target_cid = _end_exposed_target_vertex;
		do {
			unsigned int matched_source_cid = _target_prev[crt_target_cid];
			unsigned int next_target_cid = _source_prev[matched_source_cid];
			ss << "t" << crt_target_cid << "->s" << matched_source_cid << (next_target_cid!=(unsigned int)(-1)?"->":"");
			crt_target_cid = next_target_cid;
		} while(crt_target_cid!=(unsigned int)(-1));
		return ss.str();
	}
};

/*! Internal structure representing a vertex labeling over the graph, which implicitely represent the 'equality subgraph'
 *  for this graph. This enables finding a perfect matching with maximal score, since it was shown that a perfect
 *  matching in the equality subgraph corresponds to a perfect matching with maximal score in the original graph.
 */
class VertexLabeling
{
private:

	const unsigned int _ncliques;	//!< Number of cliques in the considered graph.
	std::vector<int> _ls;	//!< Array containing the labels for each source vertex.
	std::vector<int> _lt;	//!< Array containing the labels for each target vertex.

public:

	//! Constructor which initializes the vertex labeling to a trivially feasible one.
	VertexLabeling(const BipartiteMatchingGraph &bmgraph)
		: _ncliques(bmgraph.getNumberCliques())
	{
		_setToTriviallyFeasibleVertexLabeling(bmgraph);
	}

	//! Function to return the label of the specified source vertex.
	inline int getSourceVertexLabel(unsigned int source_cid) const { return _ls[source_cid]; }

	//! Function to return the label of the specified target vertex.
	inline int getTargetVertexLabel(unsigned int target_cid) const { return _lt[target_cid]; }

	//! Function to update the vertex labeling using the input value.
	void update(int delta, const AlternatingTree &atree)
	{
		for(unsigned int source_cid=0; source_cid<_ncliques; ++source_cid)
			if(atree.isInS(source_cid)) _ls[source_cid] -= delta;
		for(unsigned int target_cid=0; target_cid<_ncliques; ++target_cid)
			if(atree.isInT(target_cid)) _lt[target_cid] += delta;
	}

private:

	//! Function to compute the maximum score for the incident edges of a given vertex.
	static int _getVertexMaxEdgeScore(const Vertex &v)
	{
		int max_score = std::numeric_limits<int>::min();
		for(std::vector<Edge*>::const_iterator it_e=v.edges.begin(); it_e!=v.edges.end(); ++it_e) {
			if((*it_e)->score > max_score)
				max_score = (*it_e)->score;
		}
		return max_score;
	}

	//! Function to initialize the vertex labeling to a trivially feasible one.
	void _setToTriviallyFeasibleVertexLabeling(const BipartiteMatchingGraph &bmgraph)
	{
		_ls.resize(_ncliques);
		_lt.resize(_ncliques);
		// A feasible vertex labeling l is such that for any source vertex 's' and target vertex 't'
		// linked by an edge with score w(s,t), we have l(s)+l(t) >= w(s,t).
		// This function implements a simple algorithm to obtain a trivially feasible vertex
		// labeling, which consists in assigning to each source vertex the maximum score of all its
		// incident edges, and 0 to each target vertex.
		std::vector<Clique>::const_iterator it_c_begin, it_c_end;
		bmgraph.getCliqueIterators(it_c_begin, it_c_end);
		for(std::vector<Clique>::const_iterator it_c=it_c_begin; it_c!=it_c_end; ++it_c) {
			_ls[it_c->cid] = _getVertexMaxEdgeScore(it_c->v_source);
			_lt[it_c->cid] = 0;
		}
	}
};

/*! Internal structure memorizing, for a given labeling 'l' and for each given target vertex 't', the minimum
 *  of l(s)+l(t)-w(s,t) for all source vertices 's' in the alternating tree (i.e. set S in AlternatingTree).
 *  The way this structure is maintained enables reaching a O(n^3) complexity, instead of a O(n^4) one with
 *  the naive approach.
 */
class SlackArray
{
private:

	std::vector<int> _min_slack;			//!< Array containing, for each given target vertex 't', the value of the minimum of l(s)+l(t)-w(s,t) for all source vertices 's' in the alternating tree (i.e. set S in AlternatingTree).
	std::vector<unsigned int> _source_cid;	//!< Array containing, for each given target vertex 't', the clique ID of a source vertex 's' for which _min_cost[t] = l(s)+l(t)-w(s,t).
	std::vector<const Edge*> _edge_cid;		//!< Array containing, for each given target vertex 't', the edge linking 't' with '_source_cid[t]'.

public:

	//! Constructor, initializing the slack array using the specified labeling and source vertex.
	SlackArray(unsigned int ncliques, const Vertex &root_exposed_source_vertex, const VertexLabeling &labeling)
	{
		_min_slack.resize(ncliques, std::numeric_limits<int>::max());
		_source_cid.resize(ncliques, -1);
		_edge_cid.resize(ncliques, NULL);
		for(std::vector<Edge*>::const_iterator it_e=root_exposed_source_vertex.edges.begin(); it_e!=root_exposed_source_vertex.edges.end(); ++it_e) {
			const Vertex *target = (*it_e)->v_target;
			_min_slack[target->parent->cid] = labeling.getSourceVertexLabel(root_exposed_source_vertex.parent->cid)+labeling.getTargetVertexLabel(target->parent->cid)-(*it_e)->score;
			_source_cid[target->parent->cid] = root_exposed_source_vertex.parent->cid;
			_edge_cid[target->parent->cid] = *it_e;
		}
	}

	//! Function to return the slack value, for the specified target vertex.
	inline int getMinSlackValue(unsigned int target_cid) const { return _min_slack[target_cid]; }

	//! Function to return the source vertex associated to the minimum slack value, for the specified target vertex.
	inline int getMinSlackVertex(unsigned int target_cid) const { return _source_cid[target_cid]; }

	//! Function to return the edge associated to the minimum slack value, for the specified target vertex.
	inline const Edge* getMinSlackEdge(unsigned int target_cid) const { return _edge_cid[target_cid]; }

	//! Function to calculate the global minimum cost slack and reduce all slack values by this minimum cost slack.
	void simplifyMinCostSlack(int &delta, const AlternatingTree &atree)
	{
		// Compute the global minimum cost slack
		delta = std::numeric_limits<int>::max();
		const unsigned int ncliques = _source_cid.size();
		for(unsigned int target_cid=0; target_cid<ncliques; ++target_cid) {
			if(_source_cid[target_cid]!=(unsigned int)(-1) && !atree.isInT(target_cid))
				delta = std::min(delta, _min_slack[target_cid]);
		}
		WPMASSERT(delta!=std::numeric_limits<int>::max(), "Failed to compute the global minimum cost slack!");
		// Update the slack array
		for(unsigned int target_cid=0; target_cid<ncliques; ++target_cid) {
			if(_source_cid[target_cid]!=(unsigned int)(-1) && !atree.isInT(target_cid))
				_min_slack[target_cid] -= delta;
		}
	}

	//! Function to update all slack values after a new source vertex was added to the alternating tree.
	void updateWithNewSourceVertex(const Vertex &added_source_vertex, const VertexLabeling &labeling)
	{
		for(std::vector<Edge*>::const_iterator it_e=added_source_vertex.edges.begin(); it_e!=added_source_vertex.edges.end(); ++it_e) {
			const Vertex *target = (*it_e)->v_target;
			int tmp_slack_value = labeling.getSourceVertexLabel(added_source_vertex.parent->cid)+labeling.getTargetVertexLabel(target->parent->cid)-(*it_e)->score;
			if(tmp_slack_value < _min_slack[target->parent->cid]) {
				_min_slack[target->parent->cid] = tmp_slack_value;
				_source_cid[target->parent->cid] = added_source_vertex.parent->cid;
				_edge_cid[target->parent->cid] = *it_e;
			}
		}
	}
};

//! Function to find a source vertex which is exposed under the specified matching.
void findExposedSourceVertex(const BipartiteMatchingGraph &bmgraph, const impl::Matching &matching, const Vertex* &root_vertex)
{
	root_vertex = NULL;
	std::vector<const Clique*> random_cliques;
	bmgraph.getCliquesInRandomOrder(random_cliques);
	for(std::vector<const Clique*>::const_iterator it_c=random_cliques.begin(); it_c!=random_cliques.end(); ++it_c) {
		if(matching.isSource((*it_c)->cid)==false) {
			root_vertex = &((*it_c)->v_source);
			break;
		}
	}
}

//! Function to update the labeling from the current state of the slack array, and then update the slack array.
bool updateLabelingAndSlack(const AlternatingTree &atree, SlackArray &slack, VertexLabeling &labeling)
{
	// Calculate delta using the slack
	int delta;
	slack.simplifyMinCostSlack(delta, atree);
	// If the global minimum cost slack is zero, then we cannot update the labeling
	if(delta==0)
		return false;
	// Update the labeling
	labeling.update(delta, atree);
	return true;
}

//! Function to update the alternating tree and slack array until the queue is empty or augmenting path is found.
bool findAugmentingPathAroundCandidatesSourceVertices(std::queue<const Vertex*> &queue_candidates_source_vertices,
													  AlternatingTree &atree,
													  impl::SlackArray &slack,
													  const impl::Matching &matching,
													  const impl::VertexLabeling &labeling)
{
	// Loop over each source vertex in the queue
	while(!atree.isAugmentingPathFound() && !queue_candidates_source_vertices.empty()) {
		const Vertex *source_v = queue_candidates_source_vertices.front();
		queue_candidates_source_vertices.pop();
		// Iterate over each edge incident to the current source vertex in the current equality subgraph
		for(std::vector<Edge*>::const_iterator it_e=source_v->edges.begin(); it_e!=source_v->edges.end(); ++it_e) {
			const Vertex *target_v = (*it_e)->v_target;
			int slack_value = labeling.getSourceVertexLabel(source_v->parent->cid)+labeling.getTargetVertexLabel(target_v->parent->cid)-(*it_e)->score;
			if(!atree.isInT(target_v->parent->cid) && slack_value==0) {
				// If this target vertex is exposed, we found the augmenting path !
				if(!matching.isTarget(target_v->parent->cid)) {
					atree.setEndExposedTargetVertex(source_v->parent->cid, target_v->parent->cid, *it_e);
					break;
				}
				// Otherwise, update the alternating tree with the edges (source_v,target_v) and (target_v,matched_v)
				const Edge *edge_t_s2 = matching.getEdgeFromTarget(target_v->parent->cid);
				WPMASSERT(edge_t_s2!=NULL, "Matching structure has no edge for a matched target vertex!");
				const Vertex *matched_v = edge_t_s2->v_source;
				queue_candidates_source_vertices.push(matched_v);
				atree.addTwoEdges(source_v->parent->cid, target_v->parent->cid, matched_v->parent->cid, *it_e, edge_t_s2);
				// The source vertex matched_v has been added to S, hence update the slack array
				slack.updateWithNewSourceVertex(*matched_v, labeling);
			}
		}
	}
	return atree.isAugmentingPathFound();
}

//! Function to search for an exposed target vertex among the newly reachable target vertices. This is done after the vertex labeling was updated, leading to new edges in the equality subgraph.
bool findAugmentingPathFromNewReachableTargetVertices(std::queue<const Vertex*> &queue_candidates_source_vertices,
													  AlternatingTree &atree,
													  impl::SlackArray &slack,
													  const BipartiteMatchingGraph &bmgraph,
													  const impl::Matching &matching,
													  const impl::VertexLabeling &labeling)
{
	// Loop over all target vertices in the graph
	std::vector<Clique>::const_iterator it_c, it_c_end;
	bmgraph.getCliqueIterators(it_c,it_c_end);
	for(; !atree.isAugmentingPathFound() && it_c!=it_c_end; ++it_c) {
		const Vertex *target_v = &it_c->v_target;
		// Check if this target vertex is a newly reachable one in the equality subgraph
		if(!atree.isInT(it_c->cid) && slack.getMinSlackValue(it_c->cid)==0) {
			// If it is, retrieve the source vertex from which it is reachable and the associated edge
			unsigned int min_slack_vertex_cid = slack.getMinSlackVertex(it_c->cid);
			WPMASSERT(min_slack_vertex_cid!=(unsigned int)(-1), "The min slack vertex does not exist!");	// This should never happen since we checked that 'slack.getMinSlackValue(it_c->cid)==0'.
			const Edge* edge_s1_t = slack.getMinSlackEdge(it_c->cid);
			WPMASSERT(edge_s1_t!=NULL, "The SlackArray structure has no edge for the specified target vertex!");
			// If the target vertex is exposed, we found the augmenting path !
			if(!matching.isTarget(it_c->cid)) {
				atree.setEndExposedTargetVertex(min_slack_vertex_cid, target_v->parent->cid, edge_s1_t);
				break;
			}
			// Otherwise, check if the matched source vertex is already inside the alternating tree
			const Edge *edge_t_s2 = matching.getEdgeFromTarget(target_v->parent->cid);
			WPMASSERT(edge_t_s2!=NULL, "The matching structure has no edge for a matched target vertex!");
			const Vertex *matched_v = edge_t_s2->v_source;
			if(!atree.isInS(matched_v->parent->cid)) {
				// If it is not, add the matched source vertex to the candidates queue and update the alternating tree with the edges (source_v,target_v) and (target_v, matched_v)
				queue_candidates_source_vertices.push(matched_v);
				atree.addTwoEdges(min_slack_vertex_cid, target_v->parent->cid, matched_v->parent->cid, edge_s1_t, edge_t_s2);
				slack.updateWithNewSourceVertex(*matched_v, labeling);
			}
			else {
				// Otherwise, just update the alternating tree with the edges (target_v, matched_v)
				atree.addSingleEdge(min_slack_vertex_cid, it_c->cid, edge_s1_t);
			}
		}
	}
	return atree.isAugmentingPathFound();
}

//! Function to augment the specified matching by finding an augmenting path.
bool augmentMatching(const BipartiteMatchingGraph &bmgraph, impl::Matching &matching, impl::VertexLabeling &labeling)
{
	unsigned int ncliques = bmgraph.getNumberCliques();
	if(matching.getNumberEdges()==ncliques)	// If the matching is perfect, it cannot be augmented.
		return false;
	if(VERBOSE) std::cout << " augmentMatching (|matching|=" << matching.getNumberEdges() << "):" << std::endl;
	// Since the matching is not perfect, there is an exposed source vertex, so find it
	const Vertex* root_exposed_source_vertex = NULL;
	impl::findExposedSourceVertex(bmgraph, matching, root_exposed_source_vertex);
	WPMASSERT(root_exposed_source_vertex!=NULL, "Found no exposed source vertex!");	// Since the matching is not perfect, this should never happen.
	if(VERBOSE) std::cout << "  root_exposed_source_vertex = s" << root_exposed_source_vertex->parent->cid << std::endl;
	// Initialize the slack array and alternating tree using the root exposed source vertex
	SlackArray slack(ncliques, *root_exposed_source_vertex, labeling);
	AlternatingTree atree(ncliques, root_exposed_source_vertex->parent->cid);
	// Iteratively update the vertex labeling and build the alternating tree, until an augmenting path along the equality subgraph is found
	std::queue<const Vertex*> queue_candidates_source_vertices;
	queue_candidates_source_vertices.push(root_exposed_source_vertex);
	while(!atree.isAugmentingPathFound()) {
		// Breadth-first search on the equality subgraph for the current labeling, in order to find an exposed target vertex
		if(findAugmentingPathAroundCandidatesSourceVertices(queue_candidates_source_vertices, atree, slack, matching, labeling))
			break;	// An augmenting path has been found!
		// Update the labeling, or return and indicate that we failed to augment the matching
		if(!updateLabelingAndSlack(atree, slack, labeling))
			return false;
		// Updating the labeling added new edges in the equality subgraph, so check them to find an augmenting path
		WPMASSERT(queue_candidates_source_vertices.empty(), "The graph search queue is not empty!");	// We did not find an augmenting path yet, this should never happen.
		if(findAugmentingPathFromNewReachableTargetVertices(queue_candidates_source_vertices, atree, slack, bmgraph, matching, labeling))
			break;	// An augmenting path has been found!
		WPMASSERT(!queue_candidates_source_vertices.empty(), "The graph search queue is empty!");
	}
	// An augmenting path was found, hence augment the matching
	if(VERBOSE) std::cout << "  Augmenting path found: " << atree.serializeAugmentingPath() << std::endl;
	atree.applyAugmentingPath(matching);
	if(VERBOSE) std::cout << "  New matching: " << matching.serialize() << std::endl;
	return true;
}

//! Function to iteratively augment the matching until a perfect matching is found.
bool findPerfectMatching(const BipartiteMatchingGraph &bmgraph,
						 std::vector<const Edge*> &matching)
{
	if(VERBOSE) std::cout << "findPerfectMatching:" << std::endl;
	// Initialize the Hungarian algorithm
	unsigned int ncliques = bmgraph.getNumberCliques();
	impl::VertexLabeling labeling(bmgraph);	// Initialized to a trivially feasible matching
	impl::Matching tmp_matching(ncliques);	// Initialized to an empty matching
	for(unsigned int i=0; i<ncliques; ++i) {	// Each iteration augments the matching by one edge
		if(!impl::augmentMatching(bmgraph, tmp_matching, labeling))
			return false;
	}
	// Retrieve the set of edges in the matching
	tmp_matching.getMatchingEdges(matching);
	return true;
}



}	// namespace impl



/////////////////////////////////////////////////////////////////////////////////////////////////////////////



PerfectMatchingFinder::PerfectMatchingFinder()
{
}

PerfectMatchingFinder::~PerfectMatchingFinder()
{
}

bool PerfectMatchingFinder::findRandomPerfectMatching(const BipartiteMatchingGraph &bmgraph,
													  std::vector<const Edge*> &matching)
{
	// Find a perfect matching in the graph
	std::vector<const Edge*> tmp_matching;
	bool success = impl::findPerfectMatching(bmgraph, tmp_matching);
	// Check if it is a perfect matching (a perfect matching spans all the
	// vertices in the graph, hence the number of edges in the perfect matching
	// is equal to the number of cliques in the bipartite graph)
	if(!success || tmp_matching.size()<bmgraph.getNumberCliques())
		return false;
	matching.swap(tmp_matching);
	return true;
}



} //namespace wpm
