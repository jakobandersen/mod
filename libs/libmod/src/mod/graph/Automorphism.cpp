#include "Automorphism.hpp"

#include <mod/graph/GraphInterface.hpp>
#include <mod/lib/Graph/Single.hpp>

#include <perm_group/permutation/io.hpp>

namespace mod::graph {

//==============================================================================
// Aut
//==============================================================================

Graph::Aut::Aut(std::shared_ptr<Graph> g, LabelType lt, bool withStereo, std::size_t i)
		: g(g), lt(lt), withStereo(withStereo), i(i) {}

Graph::Vertex Graph::Aut::operator[](Vertex v) const {
	const auto &group = g->getGraph().getAutGroup(lt, withStereo);
	const auto &p = group.generators()[i];
	const auto img = perm_group::get(p, v.getId());
	return Vertex(g, img);
}

std::ostream &operator<<(std::ostream &s, const Graph::Aut &a) {
	const auto &group = a.g->getGraph().getAutGroup(a.lt, a.withStereo);
	const auto &p = group.generators()[a.i];
	return perm_group::write_permutation_cycles(s, p);
}

//==============================================================================
// Gens
//==============================================================================

Graph::AutGroup::Gens::Gens(std::shared_ptr<Graph> g, LabelType lt, bool withStereo)
		: g(g), lt(lt), withStereo(withStereo) {}

Graph::AutGroup::Gens::iterator Graph::AutGroup::Gens::begin() const {
	// there is always the identity aut
	return iterator(g, lt, withStereo, 0);
}

Graph::AutGroup::Gens::iterator Graph::AutGroup::Gens::end() const {
	const auto &group = g->getGraph().getAutGroup(lt, withStereo);
	return iterator(g, lt, withStereo, group.generators().size());
}

Graph::Aut Graph::AutGroup::Gens::operator[](std::size_t i) const {
	return begin()[i];
}

std::size_t Graph::AutGroup::Gens::size() const {
	return end() - begin();
}

// Iterator
//------------------------------------------------------------------------------

Graph::AutGroup::Gens::iterator::iterator(std::shared_ptr<Graph> g, LabelType lt, bool withStereo, std::size_t i)
		: g(g), lt(lt), withStereo(withStereo), i(i) {}

bool Graph::AutGroup::Gens::iterator::equal(const Graph::AutGroup::Gens::iterator &other) const {
	return std::tie(g, lt, withStereo, i) == std::tie(other.g, other.lt, other.withStereo, other.i);
}

void Graph::AutGroup::Gens::iterator::increment() {
	++i;
}

Graph::Aut Graph::AutGroup::Gens::iterator::dereference() const {
	return Aut(g, lt, withStereo, i);
}

std::size_t Graph::AutGroup::Gens::iterator::distance_to(const iterator &other) const {
	return other.i - i;
}

void Graph::AutGroup::Gens::iterator::advance(std::size_t n) {
	i += n;
}

//==============================================================================
// AutGroup
//==============================================================================

Graph::AutGroup::AutGroup(std::shared_ptr<Graph> g, LabelSettings labelSettings)
		: g(g), lt(labelSettings.type), withStereo(labelSettings.withStereo) {}

Graph::AutGroup::Gens Graph::AutGroup::gens() const {
	return Gens(g, lt, withStereo);
}

std::ostream &operator<<(std::ostream &s, const Graph::AutGroup &a) {
	s << "AutGroup(" << *a.g << ", " << a.lt << ", " << std::boolalpha << a.withStereo << ", [";
	bool first = true;
	for(const auto &g: a.gens()) {
		if(first) first = false;
		else s << ", ";
		s << g;
	}
	s << "])";
	return s;
}

} // namespace mod::graph