#ifndef MOD_LIB_GRAPHMORPHISM_LABEL_ANY_HPP
#define MOD_LIB_GRAPHMORPHISM_LABEL_ANY_HPP

#include <mod/lib/GraphMorphism/LabelledMorphism.hpp>
#include <mod/lib/GraphMorphism/TermVertexMap.hpp>
#include <mod/lib/GraphMorphism/Constraints/Constraint.hpp>
#include <mod/lib/Term/WAM.hpp>

#include <jla_boost/graph/morphism/Concepts.hpp>
#include <jla_boost/graph/morphism/models/PropertyVertexMap.hpp>

namespace mod::lib::GraphMorphism::Constraints {

template<typename Graph>
struct LabelAny : Constraint<Graph> {
	MOD_VISITABLE();
public:
	LabelAny(std::string label, std::vector<std::string> labels)
			: label(std::move(label)), labels(std::move(labels)) {}

	virtual std::unique_ptr<Constraint < Graph> >
	clone() const override {
		return std::make_unique<LabelAny>(label, labels);
	}

	virtual std::string name() const override {
		return "LabelAny";
	}
private:
	template<typename Visitor, typename LabelledGraphCodom, typename VertexMap>
	bool matchesImpl(Visitor &vis, const Graph &gDom, const LabelledGraphCodom &lgCodom, VertexMap &m,
	                 const LabelSettings ls, std::false_type) const {
		// this is kind of stupid, as it doesn't depend on the graphs or the match
		assert(ls.type == LabelType::String); // otherwise someone forgot to add the TermData prop
		return labels.end() != std::find(labels.begin(), labels.end(), label);
	}

	template<typename Visitor, typename LabelledGraphCodom, typename VertexMap>
	int matchesImpl(Visitor &vis, const Graph &gDom, const LabelledGraphCodom &lgCodom, VertexMap &m,
	                const LabelSettings ls, std::true_type) const {
		assert(ls.type == LabelType::Term); // otherwise someone did something very strange
		lib::Term::Wam &machine = get_prop(TermDataT(), m).machine;
		lib::Term::MGU mgu(machine.getHeap().size());
		const auto aTerm = machine.copyFromTemp(term);
		const bool res = std::any_of(terms.begin(), terms.end(), [aTerm, &machine](std::size_t t) {
			lib::Term::MGU mgu = machine.unifyHeapTemp(aTerm.addr, t);
			machine.revert(mgu);
			return mgu.status == lib::Term::MGU::Status::Exists;
		});
		machine.revert(mgu);
		return res;
	}
public:
	template<typename Visitor, typename LabelledGraphCodom, typename VertexMap>
	bool matches(Visitor &vis, const Graph &gDom, const LabelledGraphCodom &lgCodom, VertexMap &m,
	             const LabelSettings ls) const {
		using GraphCodom = typename LabelledGraphTraits<LabelledGraphCodom>::GraphType;
		static_assert(std::is_same<Graph, typename jla_boost::GraphMorphism::VertexMapTraits<VertexMap>::GraphDom>::value,
		              "");
		static_assert(
				std::is_same<GraphCodom, typename jla_boost::GraphMorphism::VertexMapTraits<VertexMap>::GraphCodom>::value,
				"");
		using HasTerm = GraphMorphism::HasTermData<VertexMap>;
		return matchesImpl(vis, gDom, lgCodom, m, ls, HasTerm());
	}
public:
	std::string label;
	std::vector<std::string> labels;
	std::size_t term;
	std::vector<std::size_t> terms;
};

} // namespace mod::lib::GraphMorphism::Constraints

#endif // MOD_LIB_GRAPHMORPHISM_LABEL_ANY_HPP