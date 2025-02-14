#ifndef MOD_LIB_GRAPH_PROP_LABEL_HPP
#define MOD_LIB_GRAPH_PROP_LABEL_HPP

#include <mod/lib/Graph/Properties/Property.hpp>

namespace mod::lib::graph {

struct PropString : Prop<PropString, std::string, std::string> {
	using Base = Prop<PropString, std::string, std::string>;
public:
	explicit PropString(const GraphType &g) : Base(g) {
		Base::verify(&g);
	}

	PropString(const PropString &other, const GraphType &g) : Base(other, g) {
		Base::verify(&g);
	}
};

} // namespace mod::lib::graph

#endif // MOD_LIB_GRAPH_PROP_LABEL_HPP
