#include "Term.hpp"

#include <mod/Error.hpp>
#include <mod/lib/GraphMorphism/Constraints/AllVisitor.hpp>
#include <mod/lib/IO/IO.hpp>
#include <mod/lib/IO/ParsingError.hpp>
#include <mod/lib/Rules/Properties/String.hpp>
#include <mod/lib/Term/IO/Read.hpp>

#include <jla_boost/graph/PairToRangeAdaptor.hpp>

namespace mod::lib::Rules {

PropTerm::PropTerm(const RuleType &rule,
                   const std::vector<ConstraintPtr> &leftMatchConstraints,
                   const std::vector<ConstraintPtr> &rightMatchConstraints,
                   const PropString &pString, const StringStore &stringStore) : Base(rule) {
	// take every unique string and parse it
	std::unordered_map<std::string, std::size_t> labelToAddress;
	Term::RawAppendStore varToAddr;
	const auto handleLabel = [&stringStore, &labelToAddress, &varToAddr, this](const std::string &label) {
		// if there is a * in the label, then we can not just use label caching
		if(label.find('*') == std::string::npos) {
			auto iter = labelToAddress.find(label);
			if(iter != end(labelToAddress)) {
				return iter->second;
			}
		}
		Term::RawTerm rawTerm;
		try {
			rawTerm = Term::Read::rawTerm(label, stringStore);
		} catch(const IO::ParsingError &e) {
			parsingError = e.msg;
			return std::numeric_limits<std::size_t>::max();
		}
		Term::Address addr = Term::append(machine, rawTerm, varToAddr);
		labelToAddress[label] = addr.addr;
		return addr.addr;
	};

	vPropL.reserve(num_vertices(getL(rule)));
	vPropR.reserve(num_vertices(getR(rule)));
	for(const auto vL: asRange(vertices(getL(rule)))) {
		assert(get(boost::vertex_index_t(), getL(rule), vL) >= vPropL.size());
		vPropL.resize(get(boost::vertex_index_t(), getL(rule), vL));
		assert(get(boost::vertex_index_t(), getL(rule), vL) == vPropL.size());
		vPropL.push_back(handleLabel(pString.getLeft()[vL]));
	}
	for(const auto vR: asRange(vertices(getR(rule)))) {
		assert(get(boost::vertex_index_t(), getR(rule), vR) >= vPropR.size());
		vPropR.resize(get(boost::vertex_index_t(), getR(rule), vR));
		assert(get(boost::vertex_index_t(), getR(rule), vR) == vPropR.size());
		const auto vK = get_inverse(getMorR(rule), getK(rule), getR(rule), vR);
		if(vK != lib::DPO::CombinedRule::KGraphType::null_vertex()) {
			const auto vL = get(getMorL(rule), getK(rule), getL(rule), vK);
			if(pString.getLeft()[vL] == pString.getRight()[vR]) {
				// copy the term from left
				vPropR.push_back(vPropL[get(boost::vertex_index_t(), getL(rule), vL)]);
				continue;
			}
		}
		vPropR.push_back(handleLabel(pString.getRight()[vR]));
	}
	assert(vPropL.size() <= num_vertices(rule.getCombinedGraph()));
	assert(vPropR.size() <= num_vertices(rule.getCombinedGraph()));
	vPropL.resize(num_vertices(rule.getCombinedGraph()));
	vPropR.resize(num_vertices(rule.getCombinedGraph()));

	ePropL.reserve(num_edges(getL(rule)));
	ePropR.reserve(num_edges(getR(rule)));
	for(const auto eL: asRange(edges(getL(rule)))) {
		assert(get(boost::edge_index_t(), getL(rule), eL) >= ePropL.size());
		ePropL.resize(get(boost::edge_index_t(), getL(rule), eL));
		assert(get(boost::edge_index_t(), getL(rule), eL) == ePropL.size());
		ePropL.push_back(handleLabel(pString.getLeft()[eL]));
	}
	for(const auto eR: asRange(edges(getR(rule)))) {
		assert(get(boost::edge_index_t(), getR(rule), eR) >= ePropR.size());
		ePropR.resize(get(boost::edge_index_t(), getR(rule), eR));
		assert(get(boost::edge_index_t(), getR(rule), eR) == ePropR.size());
		const auto eK = get_inverse(getMorR(rule), getK(rule), getR(rule), eR);
		if(eK != lib::DPO::CombinedRule::KGraphType::edge_descriptor()) {
			const auto eL = get(getMorL(rule), getK(rule), getL(rule), eK);
			if(pString.getLeft()[eL] == pString.getRight()[eR]) {
				// copy the term from left
				ePropR.push_back(ePropL[get(boost::edge_index_t(), getL(rule), eL)]);
				continue;
			}
		}
		ePropR.push_back(handleLabel(pString.getRight()[eR]));
	}
	assert(ePropL.size() <= num_edges(rule.getCombinedGraph()));
	assert(ePropR.size() <= num_edges(rule.getCombinedGraph()));
	ePropL.resize(num_edges(rule.getCombinedGraph()));
	ePropR.resize(num_edges(rule.getCombinedGraph()));

	using HandlerType = decltype(handleLabel);
	using SideGraphType = lib::DPO::CombinedRule::SideGraphType;

	struct Visitor : GraphMorphism::Constraints::AllVisitorNonConst<SideGraphType> {
		Visitor(HandlerType &handleLabel) : handleLabel(handleLabel) {}

		virtual void operator()(GraphMorphism::Constraints::VertexAdjacency<SideGraphType> &c) override {
			assert(c.vertexTerms.size() == 0);
			assert(c.edgeTerms.size() == 0);
			for(const std::string &s: c.vertexLabels)
				c.vertexTerms.insert(handleLabel(s));
			for(const std::string &s: c.edgeLabels)
				c.edgeTerms.insert(handleLabel(s));
		}

		virtual void operator()(GraphMorphism::Constraints::LabelAny<SideGraphType> &c) override {
			assert(c.terms.size() == 0);
			c.term = handleLabel(c.label);
			for(const std::string &s: c.labels)
				c.terms.push_back(handleLabel(s));
		}

		virtual void operator()(GraphMorphism::Constraints::ShortestPath<SideGraphType> &c) override {}
	private:
		HandlerType handleLabel;
	};

	const auto handleConstraints = [&](const auto &cs) {
		Visitor vis(handleLabel);
		for(auto &c: cs) c->accept(vis);
	};
	handleConstraints(leftMatchConstraints);
	handleConstraints(rightMatchConstraints);
	verify();
}

PropTerm::PropTerm(const RuleType &rule, Term::Wam machine) : Base(rule), machine(std::move(machine)) {}

bool isValid(const PropTerm &core) {
	return !core.parsingError.has_value();
}

const std::string &PropTerm::getParsingError() const {
	assert(!isValid(*this));
	return *parsingError;
}

Term::Wam &getMachine(PropTerm &core) {
	return core.machine;
}

const Term::Wam &getMachine(const PropTerm &core) {
	return core.machine;
}

} // namespace mod::lib::Rules