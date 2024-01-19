#ifndef MOD_LIB_LABELLEDUNIONGRAPH_HPP
#define MOD_LIB_LABELLEDUNIONGRAPH_HPP

#include <mod/lib/LabelledGraph.hpp>

#include <jla_boost/graph/UnionGraph.hpp>

#include <optional>

namespace mod::lib {
namespace detail {

template<typename LGraph>
struct UnionPropBase {
	using InnerGraph = typename LabelledGraphTraits<LGraph>::GraphType;
public:
	using GraphType = jla_boost::union_graph<InnerGraph>;
	using Vertex = typename boost::graph_traits<GraphType>::vertex_descriptor;
	using Edge = typename boost::graph_traits<GraphType>::edge_descriptor;
public:
	UnionPropBase(const std::vector<const LGraph *> &lgs) : lgs(lgs) {}
protected:
	const std::vector<const LGraph *> &lgs;
};

#define MOD_MAKE_UNION_PROP_MIN(Name, Func)                                     \
   template<typename LGraph>                                                    \
   struct UnionProp ## Name : UnionPropBase<LGraph> {                           \
      using Base = UnionPropBase<LGraph>;                                       \
      using Handler = typename LGraph::Prop ## Name ## Type::Handler;           \
   public:                                                                      \
      template<typename K>                                                      \
      friend decltype(auto) get(const UnionProp ## Name &up, const K &k) {      \
         return up[k];                                                          \
      }

#define MOD_MAKE_UNION_PROP(Name, Func) MOD_MAKE_UNION_PROP_MIN(Name, Func)     \
      using Base::Base;                                                         \
                                                                                \
      decltype(auto) operator[](const typename Base::Vertex v) const {          \
         return get_ ## Func(*this->lgs[v.gIdx])[v.v];                          \
      }                                                                         \
                                                                                \
      decltype(auto) operator[](const typename  Base::Edge e) const {           \
         return get_ ## Func(*this->lgs[e.gIdx])[e.e];                          \
      }

MOD_MAKE_UNION_PROP(String, string)
};
MOD_MAKE_UNION_PROP_MIN(Term, term)
	UnionPropTerm(const std::vector<const LGraph *> &lgs) : Base(lgs) {
		update();
	}

	decltype(auto) operator[](const typename Base::Vertex v) const {
		return get_term(*this->lgs[v.gIdx])[v.v] + offsets[v.gIdx];
	}

	decltype(auto) operator[](const typename Base::Edge e) const {
		return get_term(*this->lgs[e.gIdx])[e.e] + offsets[e.gIdx];
	}

	void update() {
		for(unsigned int i = offsets.size(); i != this->lgs.size(); ++i) {
			offsets.push_back(m.getHeap().size());
			m.appendHeapFrom(getMachine(get_term(*this->lgs[i])));
		}
	}

	const std::string &getParsingError() const {
		assert(!isValid(*this));
		for(const auto *g: this->lgs)
			if(!isValid(get_term(*g)))
				return get_term(*g).getParsingError();
		__builtin_unreachable();
	}

	friend bool isValid(const UnionPropTerm &up) {
		for(const auto *g: up.lgs)
			if(!isValid(get_term(*g)))
				return false;
		return true;
	}

	friend const lib::Term::Wam &getMachine(const UnionPropTerm &up) {
		return up.m;
	}
private:
	lib::Term::Wam m;
	std::vector<int> offsets;
};
MOD_MAKE_UNION_PROP(Stereo, stereo)
};
MOD_MAKE_UNION_PROP(Molecule, molecule)
};
#undef MOD_MAKE_UNION_PROP

} // namespace detail

template<typename LGraph>
struct LabelledUnionGraph {
	using InnerGraph = typename LabelledGraphTraits<LGraph>::GraphType;
public:
	using GraphType = jla_boost::union_graph<InnerGraph>;
	using PropStringType = detail::UnionPropString<LGraph>;
	using PropTermType = detail::UnionPropTerm<LGraph>;
	using PropStereoType = detail::UnionPropStereo<LGraph>;
	using PropMolecule = detail::UnionPropMolecule<LGraph>;
public:
	void push_back(const LGraph *lg) {
		lgs.push_back(lg);
		ug.push_back(&get_graph(*lg));
		if(pTerm) pTerm->update();
	}
public:
	friend const GraphType &get_graph(const LabelledUnionGraph &lug) {
		return lug.ug;
	}

	friend PropStringType get_string(const LabelledUnionGraph &lug) {
		return PropStringType(lug.lgs);
	}

	friend const PropTermType &get_term(const LabelledUnionGraph &lug) {
		if(!lug.pTerm) lug.pTerm.emplace(lug.lgs);
		return *lug.pTerm;
	}

	friend PropStereoType get_stereo(const LabelledUnionGraph &lug) {
		return PropStereoType(lug.lgs);
	}

	friend bool has_stereo(const LabelledUnionGraph &lug) {
		return std::any_of(lug.lgs.begin(), lug.lgs.end(), [](const LGraph *lg) {
			return has_stereo(*lg);
		});
	}

	friend PropMolecule get_molecule(const LabelledUnionGraph &lug) {
		return PropMolecule(lug.lgs);
	}
private:
	std::vector<const LGraph *> lgs;
	GraphType ug;
	mutable std::optional<PropTermType> pTerm;
};

} // namesapce mod::lib

#endif // MOD_LIB_LABELLEDUNIONGRAPH_HPP