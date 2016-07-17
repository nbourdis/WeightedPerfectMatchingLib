/* BipartiteMatchingGraph.cpp (created on 05/06/2016 by Nicolas) */



#include <algorithm>
#include <fstream>
#include "WPMASSERT.h"
#include "BipartiteMatchingGraph.h"



namespace wpm {



///////////////////////////////////////////////////////////////////////////////////////////////////////////



BipartiteMatchingGraph::Edge::Edge()
	: v_source(NULL)
	, v_target(NULL)
{
}

BipartiteMatchingGraph::Edge::Edge(Vertex* v_source, Vertex* v_target, int score)
	: score(score)
	, v_source(v_source)
	, v_target(v_target)
{
}

bool BipartiteMatchingGraph::Edge::serialize(std::ostream &os) const
{
	os << "e " << v_source->parent->cid << " " << v_target->parent->cid << " " << score << "\n";
	return true;
}

bool BipartiteMatchingGraph::Edge::deserialize(std::istream &is, std::vector<Clique> &cliques)
{
	// First read the source and target IDs
	char c[2];
	int s;
	unsigned int cid_source, cid_target;
	is.read(c, 2);
	if(c[0]!='e' || c[1]!=' ' || is.bad()) return false;
	is >> std::noskipws >> cid_source;
	is.read(c, 1);
	if(c[0]!=' ' || is.bad()) return false;
	is >> std::noskipws >> cid_target;
	is.read(c, 1);
	if(c[0]!=' ' || is.bad()) return false;
	is >> std::noskipws >> s;
	is.read(c, 1);
	if(c[0]!='\n' || is.bad()) return false;
	if(cid_source>=cliques.size()) return false;
	if(cid_target>=cliques.size()) return false;
	// Then define the member pointers
	v_source = &cliques[cid_source].v_source;
	v_target = &cliques[cid_target].v_target;
	score = s;
	return true;
}

BipartiteMatchingGraph::Vertex::Vertex(Clique *parent)
	: parent(parent)
{
	// /!\ parent pointer should not be used in the Vertex constructor, because the pointed clique may not be fully constructed yet.
}

BipartiteMatchingGraph::Clique::Clique(unsigned int cid)
	: cid(cid)
	, v_source(this)
	, v_target(this)
{
}

BipartiteMatchingGraph::Clique::Clique(const Clique &c)
	: cid(c.cid)
	, v_source(this)
	, v_target(this)
{
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////



BipartiteMatchingGraph::BipartiteMatchingGraph()
{
}

BipartiteMatchingGraph::~BipartiteMatchingGraph()
{
}

bool BipartiteMatchingGraph::save(const std::string &filepath) const
{
	std::ofstream os(filepath, std::ios::out+std::ios::trunc);
	if(!os.is_open()) return false;
	if(!serialize(os)) return false;
	os.close();
	return true;
}

bool BipartiteMatchingGraph::serialize(std::ostream &os) const
{
	os << "nc " << _cliques.size() << "\n";
	os << "ne " << _edges.size() << "\n";
	for(std::vector<Edge>::const_iterator it_e=_edges.begin(); it_e!=_edges.end(); ++it_e) {
		if(!it_e->serialize(os))
			return false;
	}
	return true;
}

bool BipartiteMatchingGraph::load(const std::string &filepath)
{
	std::ifstream is(filepath, std::ios::in);
	if(!is.is_open()) return false;
	if(!deserialize(is)) return false;
	is.close();
	return true;
}

bool BipartiteMatchingGraph::deserialize(std::istream &is)
{
	// First read the number of cliques
	char c[3];
	unsigned int ncliques;
	is.read(c, 3);
	if(c[0]!='n' || c[1]!='c' || c[2]!=' ' || is.bad()) return false;
	is >> std::noskipws >> ncliques;
	is.read(c, 1);
	if(c[0]!='\n' || is.bad()) return false;
	// Then read the number of edges
	unsigned int nedges;
	is.read(c, 3);
	if(c[0]!='n' || c[1]!='e' || c[2]!=' ' || is.bad()) return false;
	is >> std::noskipws >> nedges;
	is.read(c, 1);
	if(c[0]!='\n' || is.bad()) return false;
	// Start a new graph with the specified number of cliques
	startNewGraph(ncliques);
	// Then deserialize each edge
	_edges.reserve(nedges);
	for(unsigned int n=0; n<nedges; ++n) {
		Edge e;
		if(!e.deserialize(is, _cliques)) {
			startNewGraph(0);
			return false;
		}
		_addEdge(e);
	}
	return true;
}

void BipartiteMatchingGraph::startNewGraph(unsigned int ncliques)
{
	// Clear the graph
	_cliques.clear();
	_edges.clear();
	// Start a new graph
	_cliques.reserve(ncliques);
	for(unsigned int cid=0; cid<ncliques; ++cid)
		_cliques.push_back(Clique(cid));
	_edges.reserve(ncliques*ncliques);
}

void BipartiteMatchingGraph::addDirectedEdge(unsigned int cid_source, unsigned int cid_target, int score)
{
	WPMASSERT(cid_source<_cliques.size() && cid_target<_cliques.size(), "Input clique ID does not exist!");
	WPMASSERT(cid_source!=cid_target,"A clique cannot be matched to itself!");
	Edge e(&_cliques[cid_source].v_source, &_cliques[cid_target].v_target, score);
	_addEdge(e);
}

unsigned int BipartiteMatchingGraph::getNumberCliques() const
{
	return _cliques.size();
}

unsigned int BipartiteMatchingGraph::getNumberVertices() const
{
	return 2*_cliques.size();	// There are two vertices per clique
}

void BipartiteMatchingGraph::getEdgeIterators(std::vector<Edge>::const_iterator &ibegin, std::vector<Edge>::const_iterator &iend) const
{
	ibegin = _edges.begin();
	iend = _edges.end();
}

void BipartiteMatchingGraph::getEdgeIterators(std::vector<Edge>::iterator &ibegin, std::vector<Edge>::iterator &iend)
{
	ibegin = _edges.begin();
	iend = _edges.end();
}

void BipartiteMatchingGraph::getEdgesInRandomOrder(std::vector<const Edge*> &shuffled_edges) const
{
	shuffled_edges.reserve(_edges.size());
	for(std::vector<Edge>::const_iterator it_e=_edges.begin(); it_e!=_edges.end(); ++it_e)
		shuffled_edges.push_back(&(*it_e));
	std::random_shuffle(shuffled_edges.begin(), shuffled_edges.end());
}

void BipartiteMatchingGraph::getCliqueIterators(std::vector<Clique>::const_iterator &ibegin, std::vector<Clique>::const_iterator &iend) const
{
	ibegin = _cliques.begin();
	iend = _cliques.end();
}

void BipartiteMatchingGraph::getCliqueIterators(std::vector<Clique>::iterator &ibegin, std::vector<Clique>::iterator &iend)
{
	ibegin = _cliques.begin();
	iend = _cliques.end();
}

void BipartiteMatchingGraph::getCliquesInRandomOrder(std::vector<const Clique*> &shuffled_cliques) const
{
	shuffled_cliques.reserve(_cliques.size());
	for(std::vector<Clique>::const_iterator it_c=_cliques.begin(); it_c!=_cliques.end(); ++it_c)
		shuffled_cliques.push_back(&(*it_c));
	std::random_shuffle(shuffled_cliques.begin(), shuffled_cliques.end());
}

const BipartiteMatchingGraph::Clique* BipartiteMatchingGraph::getClique(unsigned int cid) const
{
	return &_cliques[cid];
}

void BipartiteMatchingGraph::_addEdge(Edge &edge)
{
	_edges.push_back(edge);
	edge.v_source->edges.push_back(&_edges.back());
	edge.v_target->edges.push_back(&_edges.back());
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////



std::istream& operator>>(std::istream &is, BipartiteMatchingGraph &bmgraph)
{
	bmgraph.deserialize(is);
	return is;
}

std::ostream& operator<<(std::ostream &os, const BipartiteMatchingGraph &bmgraph)
{
	bmgraph.serialize(os);
	return os;
}



} //namespace wpm
