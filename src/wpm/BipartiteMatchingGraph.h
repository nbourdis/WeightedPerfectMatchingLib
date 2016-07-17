/* BipartiteMatchingGraph.h (created on 05/06/2016 by Nicolas) */

#ifndef BIPARTITE_MATCHING_GRAPH_H
#define BIPARTITE_MATCHING_GRAPH_H



#include <string>
#include <vector>



namespace wpm {



/*! BipartiteMatchingGraph class, representing a set of cliques to be matched. Each clique contains one source and one target
 *  vertices. The edges of the graph, which represent possible clique matches, link one source vertex to one target vertex of
 *  two different cliques.
 */
class BipartiteMatchingGraph
{
public:

	// Nested classes

	// Predeclarations
	struct Vertex;
	struct Clique;

	/*! Edge structure, linking a source and a target vertices from two different
	 *  cliques and representing a possible match between two vertex cliques.
	 */
	struct Edge
	{
		friend class BipartiteMatchingGraph;

		int score;				//!< Score of the edge, related to the interest of matching these two vertices.
		Vertex* v_source;		//!< Pointer to the source vertex.
		Vertex* v_target;		//!< Pointer to the target vertex.

		Edge();
		Edge(Vertex* v_source, Vertex* v_target, int score);

	private:
		bool serialize(std::ostream &os) const;
		bool deserialize(std::istream &is, std::vector<Clique> &cliques);
	};

	//! Vertex structure.
	struct Vertex
	{
		Clique *parent;				//!< Parent clique for this vertex.
		std::vector<Edge*> edges;	//!< Set of all edges incident to this vertex.

		Vertex(Clique *parent=NULL);
	};

	//! Clique structure, to be matched with another clique and formed of one source and one target vertices.
	struct Clique
	{
		unsigned int cid;	//!< Unique positive integer assigned to the clique.
		Vertex v_source;	//!< Child source vertex.
		Vertex v_target;	//!< Child target vertex.

		Clique(unsigned int cid=-1);
		Clique(const Clique &c);
	};

private:

	// BipartiteMatchingGraph data members
	std::vector<Clique> _cliques;	//!< Set of cliques in the graph.
	std::vector<Edge> _edges;		//!< Set of edges in the graph.

public:

	//! Default constructor.
	BipartiteMatchingGraph();
	//! Destructor.
	~BipartiteMatchingGraph();

	// Graph I/O functions

	bool save(const std::string &filepath) const;
	bool serialize(std::ostream &os) const;
	bool load(const std::string &filepath);
	bool deserialize(std::istream &is);

	// Graph modification functions

	void startNewGraph(unsigned int ncliques);
	void addDirectedEdge(unsigned int cid_source, unsigned int cid_target, int score);

	// Graph getter functions

	unsigned int getNumberCliques() const;
	unsigned int getNumberVertices() const;
	void getEdgeIterators(std::vector<Edge>::const_iterator &ibegin, std::vector<Edge>::const_iterator &iend) const;
	void getEdgeIterators(std::vector<Edge>::iterator &ibegin, std::vector<Edge>::iterator &iend);
	void getEdgesInRandomOrder(std::vector<const Edge*> &shuffled_edges) const;
	void getCliqueIterators(std::vector<Clique>::const_iterator &ibegin, std::vector<Clique>::const_iterator &iend) const;
	void getCliqueIterators(std::vector<Clique>::iterator &ibegin, std::vector<Clique>::iterator &iend);
	void getCliquesInRandomOrder(std::vector<const Clique*> &shuffled_cliques) const;
	const BipartiteMatchingGraph::Clique* getClique(unsigned int cid) const;

private:

	//! Private function to append a new edge to the graph.
	void _addEdge(Edge &e);

};



// Serialization operators for BipartiteMatchingGraph
std::istream& operator>>(std::istream &is, BipartiteMatchingGraph &bmgraph);
std::ostream& operator<<(std::ostream &os, const BipartiteMatchingGraph &bmgraph);



}	// namespace wpm



#endif //BIPARTITE_MATCHING_GRAPH_H
