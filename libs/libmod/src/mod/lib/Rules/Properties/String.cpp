#include "String.hpp"

#include <mod/lib/GraphMorphism/Constraints/AllVisitor.hpp>
#include <mod/lib/Rules/Properties/Term.hpp>
#include <mod/lib/StringStore.hpp>
#include <mod/lib/Term/IO/Write.hpp>

namespace mod::lib::Rules {

PropString::PropString(const RuleType &rule) : PropBase(rule) {
	verify();
}

PropString::PropString(const RuleType &rule,
                       const std::vector<ConstraintPtr> &leftMatchConstraints,
                       const std::vector<ConstraintPtr> &rightMatchConstraints,
                       const PropTerm &term, const StringStore &strings)
		: PropBase(rule) {
	std::stringstream ss;
	const auto termToString = [&ss, &term, &strings](std::size_t addr) -> std::string {
		ss.str(std::string());

		Term::Write::term(getMachine(term), {
				Term::AddressType::Heap, addr
		}, strings, ss);
		return ss.str();
	};
	vPropL.resize(num_vertices(rule.getCombinedGraph()));
	vPropR.resize(num_vertices(rule.getCombinedGraph()));
	for(auto v: asRange(vertices(rule.getCombinedGraph()))) {
		auto vId = get(boost::vertex_index_t(), rule.getCombinedGraph(), v);
		switch(rule.getCombinedGraph()[v].membership) {
		case Membership::L:
			vPropL[vId] = termToString(term.getLeft()[v]);
			break;
		case Membership::R:
			vPropR[vId] = termToString(term.getRight()[v]);
			break;
		case Membership::K:
			vPropL[vId] = termToString(term.getLeft()[v]);
			vPropR[vId] = termToString(term.getRight()[v]);
			break;
		}
	}
	ePropL.resize(num_edges(rule.getCombinedGraph()));
	ePropR.resize(num_edges(rule.getCombinedGraph()));
	for(auto e: asRange(edges(rule.getCombinedGraph()))) {
		auto eId = get(boost::edge_index_t(), rule.getCombinedGraph(), e);
		switch(rule.getCombinedGraph()[e].membership) {
		case Membership::L:
			ePropL[eId] = termToString(term.getLeft()[e]);
			break;
		case Membership::R:
			ePropR[eId] = termToString(term.getRight()[e]);
			break;
		case Membership::K:
			ePropL[eId] = termToString(term.getLeft()[e]);
			ePropR[eId] = termToString(term.getRight()[e]);
			break;
		}
	}

	using HandlerType = decltype(termToString);
	using SideGraphType = lib::DPO::CombinedRule::SideGraphType;

	struct Visitor : lib::GraphMorphism::Constraints::AllVisitorNonConst<SideGraphType> {
		Visitor(HandlerType &termToString) : termToString(termToString) {}

		virtual void operator()(lib::GraphMorphism::Constraints::VertexAdjacency<SideGraphType> &c) override {
			assert(c.vertexLabels.size() == 0);
			assert(c.edgeLabels.size() == 0);
			for(const auto &t: c.vertexTerms)
				c.vertexLabels.insert(termToString(t));
			for(const auto &t: c.edgeTerms)
				c.edgeLabels.insert(termToString(t));
		}

		virtual void operator()(lib::GraphMorphism::Constraints::LabelAny<SideGraphType> &c) override {
			assert(c.label.size() == 0);
			assert(c.labels.size() == 0);
			c.label = termToString(c.term);
			for(const auto &t: c.terms)
				c.labels.push_back(termToString(t));
		}

		virtual void operator()(lib::GraphMorphism::Constraints::ShortestPath<SideGraphType> &c) override {}
	private:
		HandlerType termToString;
	};

	const auto handleConstraints = [&](const auto &cs) {
		Visitor vis(termToString);
		for(auto &c: cs) c->accept(vis);
	};
	handleConstraints(leftMatchConstraints);
	handleConstraints(rightMatchConstraints);
}

} // namespace mod::lib::Rules