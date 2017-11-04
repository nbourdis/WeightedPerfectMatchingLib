/* NonMatchingGraph.cpp (created on 04/06/2016 by Nicolas) */



#include <fstream>
#include "WPMASSERT.h"
#include "NonMatchingGraph.h"



namespace wpm {



///////////////////////////////////////////////////////////////////////////////////////////////////////////



NonMatchingGraph:: Edge::Edge()
	: v_source(NULL)
	, v_target(NULL)
	, cost (0)
{
}

NonMatchingGraph::Edge::Edge(Vertex* v_source, Vertex* v_target, unsigned int cost)
	: v_source(v_source)
	, v_target(v_target)
	, cost(cost)
{
}

NonMatchingGraph::Vertex::Vertex()
	: id(-1)
{
}

NonMatchingGraph::Vertex::Vertex(unsigned int id)
	: id(id)
{
}

void NonMatchingGraph::Vertex::addNonMatchingConstraint(Edge& e)
{
	constraints.push_back(e);
}

bool NonMatchingGraph::Vertex::serialize(std::ostream &os) const
{
	os << "v " << id;
	for(std::vector<Edge>::const_iterator it_e=constraints.begin(); it_e!=constraints.end(); ++it_e) {
		os << " " << it_e->v_target->id << "(" << it_e->cost << ")";
	}
	os << "\n";
	return true;
}

bool NonMatchingGraph::Vertex::deserialize(std::istream &is, std::vector<Vertex> &vertices)
{
	// First read the vertex ID
	char c[2];
	is.read(c, 2);
	if(c[0]!='v' || c[1]!=' ' || is.bad()) return false;
	unsigned int id;
	is >> std::noskipws >> id;
	if(id>=vertices.size()) return false;
	// Then read the constraints
	std::vector< std::pair<unsigned int, unsigned int> > tmp_constraints;
	while(true) {
		// Check the next character to see if there are more constraints
		is.read(c, 1);
		if(is.bad()) return false;
		if(c[0]=='\n') break;
		// Read the constraint properties
		if(c[0]!=' ') return false;
		std::pair<unsigned int, unsigned int> pair_target_cost;	// 'first' is the ID of the target vertex, 'second' is the cost value for this constraint
		is >> std::noskipws >> pair_target_cost.first;
		if(pair_target_cost.first>=vertices.size()) return false;
		is.read(c, 1);
		if(c[0]!='(' || is.bad()) return false;
		is >> std::noskipws >> pair_target_cost.second;
		is.read(c, 1);
		if(c[0]!=')' || is.bad()) return false;
		// Store the properties of this constraint and read the next one
		tmp_constraints.push_back(pair_target_cost);
	}
	// If some constraints were read, create the associated edges
	vertices[id].constraints.clear();
	vertices[id].constraints.reserve(tmp_constraints.size());
	for(std::vector< std::pair<unsigned int, unsigned int> >::const_iterator it_c=tmp_constraints.begin(); it_c!=tmp_constraints.end(); ++it_c)
		vertices[id].constraints.push_back(Edge(&vertices[id],&vertices[it_c->first],it_c->second));
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

NonMatchingGraph::NonMatchingGraph()
{
}

NonMatchingGraph::~NonMatchingGraph()
{
}

bool NonMatchingGraph::save(const std::string &filepath) const
{
	std::ofstream os(filepath, std::ios::out+std::ios::trunc);
	if(!os.is_open()) return false;
	if(!serialize(os)) return false;
	os.close();
	return true;
}

bool NonMatchingGraph::serialize(std::ostream &os) const
{
	os << "nv " << _vertices.size() << "\n";
	for(std::vector<Vertex>::const_iterator it_v=_vertices.begin(); it_v!=_vertices.end(); ++it_v) {
		if(!it_v->serialize(os))
			return false;
	}
	return true;
}

bool NonMatchingGraph::load(const std::string &filepath)
{
	std::ifstream is(filepath, std::ios::in);
	if(!is.is_open()) return false;
	if(!deserialize(is)) return false;
	is.close();
	return true;
}

bool NonMatchingGraph::deserialize(std::istream &is)
{
	// First read the number of vertices
	char c[3];
	unsigned int nvertices;
	is.read(c, 3);
	if(c[0]!='n' || c[1]!='v' || c[2]!=' ' || is.bad()) return false;
	is >> std::noskipws >> nvertices;
	is.read(c, 1);
	if(c[0]!='\n' || is.bad()) return false;
	// Start a new graph with the specified number of vertices
	startNewGraph(nvertices);
	// Then deserialize each vertex
    for(unsigned int n=0; n<nvertices; ++n) {
        if(!Vertex::deserialize(is, _vertices))
            return false;
    }
	if(!_checkGraphValidity()) {
		startNewGraph(0);
		return false;
	}
	return true;
}

void NonMatchingGraph::startNewGraph(unsigned int nvertices)
{
	// Clear the graph
	_vertices.clear();
	// Start a new graph
	_vertices.reserve(nvertices);
	for(unsigned int id=0; id<nvertices; ++id)
		_vertices.push_back(Vertex(id));
}

void NonMatchingGraph::addUndirectedEdge(unsigned int id1, unsigned int id2, unsigned int cost)
{
	WPMASSERT(id1!=id2,"An edge between a vertex and itself must be a directed one!")
	addDirectedEdge(id1, id2, cost);
	addDirectedEdge(id2, id1, cost);
}

void NonMatchingGraph::addDirectedEdge(unsigned int id_source, unsigned int id_target, unsigned int cost)
{
	WPMASSERT(id_source<_vertices.size() && id_target<_vertices.size(), "Input ID does not exist!");
	Edge e(&_vertices[id_source], &_vertices[id_target], cost);
	_vertices[id_source].addNonMatchingConstraint(e);
}

void NonMatchingGraph::updateConstraints(const std::vector<unsigned int> &matching, bool avoid_deterministic_matching)
{
	unsigned int nvertices = _vertices.size();
	WPMASSERT(nvertices==matching.size(), "The input matching is incompatible with this graph!")
	// First decrement the cost for each edge with finite cost
	unsigned int high_cost = nvertices;
	for(std::vector<Vertex>::iterator it_v=_vertices.begin(); it_v!=_vertices.end(); ++it_v) {
		// Loop over each edge incident to the current vertex and decrement the cost
		std::vector<Edge>::iterator it_e = it_v->constraints.begin();
		while(it_e!=it_v->constraints.end()) {
			if(it_e->cost<=1) {
				// This cost would be decremented to 0, hence it would not represent a non-matching constraint anymore, so remove it
				it_e = it_v->constraints.erase(it_e);
				continue;
			}
			if(it_e->cost!=(unsigned int)(-1))
				--it_e->cost;
			++it_e;
		}
		// The current vertex has been matched, hence add a new non-matching constraint with a high cost,
		// in order to avoid matching these two vertices in the near future.
		unsigned int matched_id = matching[it_v->id];
		for(it_e=it_v->constraints.begin(); it_e!=it_v->constraints.end(); ++it_e) {
			if(it_e->v_target->id==matched_id)
				break;
		}
		if(it_e!=it_v->constraints.end()) {
			WPMASSERT(it_e->cost!=(unsigned int)(-1),"The current vertex was matched with a vertex for which the cost was infinte!")
			it_e->cost = high_cost;
		}
		else addDirectedEdge(it_v->id, matched_id, high_cost);
		// If the number of cost edges becomes equal to nvertices-1, which means that the next matching is (almost) garanteed to select
		// the remaining one, erase all edges with cost below high_cost*2/3, to introduce some non-determinism for the next matching
		if(avoid_deterministic_matching && it_v->constraints.size()==nvertices-1) {
			unsigned int cost_threshold = (high_cost*2)/3;
			it_e = it_v->constraints.begin();
			while(it_e!=it_v->constraints.end()) {
				if(it_e->cost<=cost_threshold) {
					it_e = it_v->constraints.erase(it_e);
					continue;
				}
				++it_e;
			}
		}
	}
}

void NonMatchingGraph::getVertexIterators(std::vector<Vertex>::const_iterator &ibegin, std::vector<Vertex>::const_iterator &iend) const
{
	ibegin = _vertices.begin();
	iend = _vertices.end();
}

void NonMatchingGraph::getVertexIterators(std::vector<Vertex>::iterator &ibegin, std::vector<Vertex>::iterator &iend)
{
	ibegin = _vertices.begin();
	iend = _vertices.end();
}

bool NonMatchingGraph::_checkGraphValidity() const
{
	// Check the existance and unicity of each vertex indice between 0 and _vertices.size()-1
	std::vector<unsigned char> flags_indices(_vertices.size(),0);
	for(std::vector<Vertex>::const_iterator it_v=_vertices.begin(); it_v!=_vertices.end(); ++it_v) {
		if(std::distance(_vertices.begin(), it_v)!=it_v->id)
			return false;	// The current vertex has the ID 'id' but is not stored at _vertices[id]
		if(flags_indices[it_v->id]!=0)
			return false;	// Duplicate index detected
		flags_indices[it_v->id]=1;
	}
	for(std::vector<unsigned char>::const_iterator it_f=flags_indices.begin(); it_f!=flags_indices.end(); ++it_f) {
		if(*it_f!=1) return false;	// Missing index detected
	}
#ifdef _DEBUG
	// Debug checks (these checks should be garanteed by construction)
	for(std::vector<Vertex>::const_iterator it_v=_vertices.begin(); it_v!=_vertices.end(); ++it_v) {
		if(std::distance(_vertices.begin(), it_v)!=it_v->id)
			return false;	// The current vertex has the ID 'id' but is not stored at _vertices[id]
		for(std::vector<Edge>::const_iterator it_e=it_v->constraints.begin(); it_e!=it_v->constraints.end(); ++it_e) {
			if(it_e->v_source==NULL || it_e->v_target==NULL)
				return false;	// One of the vertex pointer was null
			int id_source = it_e->v_source-&_vertices[0];
			int id_target = it_e->v_target-&_vertices[0];
			if(id_source<0 || id_source>=(int)_vertices.size() || id_target<0 || id_target>=(int)_vertices.size())
				return false;	// One of the vertex pointers do not point to an item of '_vertices'
		}
	}
#endif //_DEBUG
	return true;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////



std::istream& operator>>(std::istream &is, NonMatchingGraph &nmgraph)
{
	nmgraph.deserialize(is);
	return is;
}

std::ostream& operator<<(std::ostream &os, const NonMatchingGraph &nmgraph)
{
	nmgraph.serialize(os);
	return os;
}



} //namespace wpm
