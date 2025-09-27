#ifndef MOD_LIB_STEREO_EMBEDDINGEDGE_HPP
#define MOD_LIB_STEREO_EMBEDDINGEDGE_HPP

#include <mod/lib/Stereo/EdgeCategory.hpp>

#include <boost/graph/graph_traits.hpp>

#include <cassert>

namespace mod::lib::Stereo {

struct EmbeddingEdge {
	enum class Type {
		Edge, LonePair, Radical
	};
public:
	EmbeddingEdge(std::size_t offset, Type type, EdgeCategory cat);

	template<typename Graph>
	typename boost::graph_traits<Graph>::edge_descriptor
	getEdge(const typename boost::graph_traits<Graph>::vertex_descriptor &v, const Graph &g) const {
		assert(type == Type::Edge);
		assert(offset < out_degree(v, g));
		auto iter = out_edges(v, g).first;
		std::advance(iter, offset);
		return *iter;
	}
public:
	std::size_t offset;
	Type type;
	EdgeCategory cat;
};

} // namespace mod::lib::Stereo

#endif // MOD_LIB_STEREO_EMBEDDINGEDGE_HPP