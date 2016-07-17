/* NonMatchingGraph.h (created on 04/06/2016 by Nicolas) */

#ifndef NON_MATCHING_GRAPH_H
#define NON_MATCHING_GRAPH_H



#include <string>
#include <vector>



namespace wpm {



/*! NonMatchingGraph class, representing a graph of vertices to be matched, where edges represent non-matching constraints from a
 *  vertex to another one.
 */
class NonMatchingGraph
{
public:

	// Nested classes

	// Predeclarations
	struct Vertex;

	//! Edge structure, representing a non-matching constraint from one vertices to another.
	struct Edge
	{
		Vertex* v_source;		//!< Pointer to the source vertex.
		Vertex* v_target;		//!< Pointer to the potentially matched vertex.
		unsigned int cost;		//!< Integer representing the cost of matching these two vertices (-1 means these vertices cannot be matched).

		Edge();
		Edge(Vertex* v_source, Vertex* v_target, unsigned int cost);
	};

	//! Vertex structure, representing something to be matched and its non-matching constraints.
	struct Vertex
	{
		friend class NonMatchingGraph;

		unsigned int id;				//!< Unique positive integer assigned to the vertex.
		std::vector<Edge> constraints;	//!< Set of non-matching constraints for this vertex.

		Vertex();
		Vertex(unsigned int id);

		void addNonMatchingConstraint(Edge& e);

	private:
		bool serialize(std::ostream &os) const;
		static bool deserialize(std::istream &is, std::vector<Vertex> &vertices);
	};

private:

	// NonMatchingGraph data members
	std::vector<Vertex> _vertices;	//!< Set of vertices.

public:

	//! Default constructor.
	NonMatchingGraph();
	//! Destructor.
	~NonMatchingGraph();

	// Graph I/O functions

	bool save(const std::string &filepath) const;
	bool serialize(std::ostream &os) const;
	bool load(const std::string &filepath);
	bool deserialize(std::istream &is);

	// Graph modification functions

	void startNewGraph(unsigned int nvertices);
	void addUndirectedEdge(unsigned int id1, unsigned int id2, unsigned int cost);
	void addDirectedEdge(unsigned int id_source, unsigned int id_target, unsigned int cost);

	/*! Function to decrease the cost of all edges with finite cost and set the cost of the edges in the
	 *  matching to the number of vertices in the graph (to prevent matching these two again in the near future).
	 *  Optionnally, a flag may be set to avoid entering into non-deterministic matching cycles.
	 */
	void updateConstraints(const std::vector<unsigned int> &matching, bool avoid_deterministic_matching);

	// Graph getter functions

	inline unsigned int getNumberVertices() const { return _vertices.size(); };
	void getVertexIterators(std::vector<Vertex>::const_iterator &ibegin, std::vector<Vertex>::const_iterator &iend) const;
	void getVertexIterators(std::vector<Vertex>::iterator &ibegin, std::vector<Vertex>::iterator &iend);

private:

	//! Function used to check the validity of a graph after a deserialization.
	bool _checkGraphValidity() const;
};



// Serialization operators
std::istream& operator>>(std::istream &is, NonMatchingGraph &nmgraph);
std::ostream& operator<<(std::ostream &os, const NonMatchingGraph &nmgraph);



}	// namespace wpm



#endif //NON_MATCHING_GRAPH_H
