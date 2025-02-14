#include "LabelledRule.hpp"

#include <mod/Config.hpp>
#include <mod/lib/GraphMorphism/Finder.hpp>
#include <mod/lib/Stereo/CloneUtil.hpp>
#include <mod/lib/Stereo/Inference.hpp>

#include <boost/graph/connected_components.hpp>

#include <iostream>

namespace mod::lib::rule {

LabelledRule::LabelledRule(std::unique_ptr<lib::DPO::CombinedRule> rule,
                           std::unique_ptr<PropStringType> pString,
                           std::unique_ptr<PropStereoType> pStereo)
		: rule(std::move(rule)), pString(std::move(pString)), pStereo(std::move(pStereo)) {
	initComponents();
}

LabelledRule::LabelledRule(std::unique_ptr<lib::DPO::CombinedRule> rule,
                           std::unique_ptr<PropTermType> pTerm,
                           std::unique_ptr<PropStereoType> pStereo)
		: rule(std::move(rule)), pTerm(std::move(pTerm)), pStereo(std::move(pStereo)) {
	initComponents();
}

// LabelledRule
//------------------------------------------------------------------------------

LabelledRule::LabelledRule() : rule(new lib::DPO::CombinedRule()) {}

LabelledRule::LabelledRule(const LabelledRule &other, bool withConstraints) : LabelledRule() {
	const auto &cg = rule->getCombinedGraph();
	this->pString = std::make_unique<PropStringType>(getRule());
	auto &pString = *this->pString;
	const auto &ruleOther = other.getRule();
	const auto &cgOther = ruleOther.getCombinedGraph();
	const auto &pStringOther = get_string(other);
	std::vector<lib::DPO::CombinedRule::CombinedVertex> vNewFromOther(num_vertices(cgOther));
	for(const auto vcgOther: asRange(vertices(cgOther))) {
		switch(cgOther[vcgOther].membership) {
		case Membership::L: {
			const auto vL = addVertexL(*rule);
			pString.addL(vL, pStringOther.getLeft()[get_inverse(ruleOther.getLtoCG(), getL(ruleOther),
			                                                    cgOther, vcgOther)]);
			vNewFromOther[get(boost::vertex_index_t(), cgOther, vcgOther)]
					= get(rule->getLtoCG(), getL(*rule), cg, vL);
			break;
		}
		case Membership::R: {
			const auto vR = addVertexR(*rule);
			pString.addR(vR, pStringOther.getRight()[get_inverse(ruleOther.getRtoCG(), getR(ruleOther),
			                                                     cgOther, vcgOther)]);
			vNewFromOther[get(boost::vertex_index_t(), cgOther, vcgOther)]
					= get(rule->getRtoCG(), getL(*rule), cg, vR);
			break;
		}
		case Membership::K: {
			const auto vK = addVertexK(*rule);
			pString.addK(vK,
			             pStringOther.getLeft()[get_inverse(ruleOther.getLtoCG(), getL(ruleOther),
			                                                cgOther, vcgOther)],
			             pStringOther.getRight()[get_inverse(ruleOther.getRtoCG(), getR(ruleOther),
			                                                 cgOther, vcgOther)]);
			vNewFromOther[get(boost::vertex_index_t(), cgOther, vcgOther)] = vK;
			break;
		}
		}
	}
	for(const auto ecgOther: asRange(edges(cgOther))) {
		const auto vcgNewSrc = vNewFromOther[get(boost::vertex_index_t(), cgOther, source(ecgOther, cgOther))];
		const auto vcgNewTar = vNewFromOther[get(boost::vertex_index_t(), cgOther, target(ecgOther, cgOther))];
		switch(cgOther[ecgOther].membership) {
		case Membership::L: {
			const auto vLSrc = get_inverse(rule->getLtoCG(), getL(*rule), cg, vcgNewSrc);
			const auto vLTar = get_inverse(rule->getLtoCG(), getL(*rule), cg, vcgNewTar);
			const auto eL = addEdgeL(*rule, vLSrc, vLTar);
			pString.addL(eL, pStringOther.getLeft()[get_inverse(ruleOther.getLtoCG(), getL(ruleOther),
			                                                    cgOther, ecgOther)]);
			break;
		}
		case Membership::R: {
			const auto vRSrc = get_inverse(rule->getRtoCG(), getR(*rule), cg, vcgNewSrc);
			const auto vRTar = get_inverse(rule->getRtoCG(), getR(*rule), cg, vcgNewTar);
			const auto eR = addEdgeR(*rule, vRSrc, vRTar);
			pString.addR(eR, pStringOther.getRight()[get_inverse(ruleOther.getRtoCG(), getR(ruleOther),
			                                                     cgOther, ecgOther)]);
			break;
		}
		case Membership::K: {
			const auto eK = addEdgeK(*rule, vcgNewSrc, vcgNewTar);
			pString.addK(eK,
			             pStringOther.getLeft()[get_inverse(ruleOther.getLtoCG(), getL(ruleOther),
			                                                cgOther, ecgOther)],
			             pStringOther.getRight()[get_inverse(ruleOther.getRtoCG(), getR(ruleOther),
			                                                 cgOther, ecgOther)]);
			break;
		}
		}
	}
	if(other.pStereo) {
		const auto &lgLeft = get_labelled_left(other);
		const auto &lgRight = get_labelled_right(other);
		const auto infLeft = Stereo::makeCloner(lgLeft, get_L_projected(*this), jla_boost::Identity(),
		                                        jla_boost::Identity());
		const auto infRight = Stereo::makeCloner(lgRight, get_R_projected(*this), jla_boost::Identity(),
		                                         jla_boost::Identity());
		const auto inContext = [&](const auto &ve) {
			return get_stereo(other).inContext(ve);
		};
		this->pStereo.reset(new PropStereo(getRule(), infLeft, infRight, inContext, inContext));
	}
	if(withConstraints) {
		leftData.matchConstraints.reserve(other.leftData.matchConstraints.size());
		rightData.matchConstraints.reserve(other.rightData.matchConstraints.size());
		for(auto &&c: other.leftData.matchConstraints)
			leftData.matchConstraints.push_back(c->clone());
		for(auto &&c: other.rightData.matchConstraints)
			rightData.matchConstraints.push_back(c->clone());
	}
}

lib::DPO::CombinedRule &LabelledRule::getRule() { return *rule; }
const lib::DPO::CombinedRule &LabelledRule::getRule() const { return *rule; }

void LabelledRule::initComponents() { // TODO: structure this better
	if(leftData.numComponents != -1) MOD_ABORT;
	leftData.component.resize(num_vertices(get_graph(*this)), -1);
	rightData.component.resize(num_vertices(get_graph(*this)), -1);
	leftData.numComponents = boost::connected_components(get_graph(get_labelled_left(*this)),
	                                                     leftData.component.data());
	rightData.numComponents = boost::connected_components(get_graph(get_labelled_right(*this)),
	                                                      rightData.component.data());
}

void LabelledRule::invert() {
	// invert the underlying rule
	using lib::DPO::invert;
	invert(*rule);
	// invert props
	if(pString) pString->invert();
	if(pTerm) pTerm->invert();
	if(pMolecule) pMolecule->invert();
	if(pStereo) {
		// TODO: this requires change all the offsets are recalculated
		throw FatalError("Missing implementation of rule inversion with stereo info.");
	}
	// also invert the component stuff
	using std::swap;
	swap(leftData, rightData);
}

GraphType &get_graph(LabelledRule &r) {
	return r.rule->getCombinedGraph();
}

const GraphType &get_graph(const LabelledRule &r) {
	return r.rule->getCombinedGraph();
}

const LabelledRule::PropStringType &get_string(const LabelledRule &r) {
	assert(r.pString || r.pTerm);
	if(!r.pString) {
		r.pString.reset(new LabelledRule::PropStringType(
				r.getRule(), r.leftData.matchConstraints, r.rightData.matchConstraints,
				get_term(r), lib::Term::getStrings()
		));
	}
	return *r.pString;
}

const LabelledRule::PropTermType &get_term(const LabelledRule &r) {
	assert(r.pString || r.pTerm);
	if(!r.pTerm) {
		r.pTerm.reset(new LabelledRule::PropTermType(
				r.getRule(), r.leftData.matchConstraints, r.rightData.matchConstraints,
				get_string(r), lib::Term::getStrings()
		));
	}
	return *r.pTerm;
}

bool has_stereo(const LabelledRule &r) {
	return bool(r.pStereo);
}

const LabelledRule::PropStereoType &get_stereo(const LabelledRule &r) {
	if(!has_stereo(r)) {
		constexpr bool printStereoWarnings = true;
		auto gLeft = get_labelled_left(r);
		auto gRight = get_labelled_right(r);
		auto pMoleculeLeft = get_molecule(gLeft);
		auto pMoleculeRight = get_molecule(gRight);
		auto leftInference = lib::Stereo::Inference(get_graph(gLeft), pMoleculeLeft, true);
		auto rightInference = lib::Stereo::Inference(get_graph(gRight), pMoleculeRight, true);

		{
			lib::IO::Warnings warnings;
			auto res = leftInference.finalize(warnings, printStereoWarnings, [&r](LabelledRule::Vertex v) {
				return std::to_string(get(boost::vertex_index_t(), get_graph(r), v)) + " left";
			});
			std::cout << warnings;
			res.throwIfError<StereoDeductionError>();
		}
		{
			lib::IO::Warnings warnings;
			auto res = rightInference.finalize(warnings, printStereoWarnings, [&r](LabelledRule::Vertex v) {
				return std::to_string(get(boost::vertex_index_t(), get_graph(r), v)) + " right";
			});
			std::cout << warnings;
			res.throwIfError<StereoDeductionError>();
		}

		r.pStereo.reset(new PropStereo(r.getRule(),
		                               std::move(leftInference), std::move(rightInference), jla_boost::AlwaysTrue(),
		                               jla_boost::AlwaysTrue()));
	}
	return *r.pStereo;
}

const LabelledRule::PropMoleculeType &get_molecule(const LabelledRule &r) {
	if(!r.pMolecule) {
		r.pMolecule.reset(new LabelledRule::PropMoleculeType(r.getRule(), get_string(r)));
	}
	return *r.pMolecule;
}

const LabelledRule::SideProjectedGraphType &get_L_projected(const LabelledRule &r) {
	return r.rule->getLProjected();
}

const LabelledRule::SideProjectedGraphType &get_R_projected(const LabelledRule &r) {
	return r.rule->getRProjected();
}

lib::DPO::Membership membership(const LabelledRule &r, const Vertex &v) {
	return get_graph(r)[v].membership;
}

lib::DPO::Membership membership(const LabelledRule &r, const Edge &e) {
	return get_graph(r)[e].membership;
}

void put_membership(LabelledRule &r, const Vertex &v, lib::DPO::Membership m) {
	get_graph(r)[v].membership = m;
}

void put_membership(LabelledRule &r, const Edge &e, lib::DPO::Membership m) {
	get_graph(r)[e].membership = m;
}

LabelledRule::Side get_labelled_left(const LabelledRule &r) {
	return LabelledRule::Side(r, getL(r.getRule()),
	                          &PropString::getLeft, &PropTerm::getLeft,
	                          &PropStereo::getLeft, &PropMolecule::getLeft,
	                          r.leftData);
}

LabelledRule::Side get_labelled_right(const LabelledRule &r) {
	return LabelledRule::Side(r, getR(r.getRule()),
	                          &PropString::getRight, &PropTerm::getRight,
	                          &PropStereo::getRight, &PropMolecule::getRight,
	                          r.rightData);
}


// LabelledLeftGraph
//------------------------------------------------------------------------------

LabelledRule::Side::Side(const LabelledRule &r, const GraphType &g,
                         PropStringType (LabelledRule::PropStringType::*fString)() const,
                         PropTermType (LabelledRule::PropTermType::*fTerm)() const,
                         PropStereoType (LabelledRule::PropStereoType::*fStereo)() const,
                         PropMoleculeType (LabelledRule::PropMoleculeType::*fMol)() const,
                         const SideData &data)
		: r(r), g(g), fString(fString), fTerm(fTerm), fStereo(fStereo), fMol(fMol), data(data) {}

const LabelledRule::Side::GraphType &get_graph(const LabelledRule::Side &g) {
	return g.g;
}

LabelledRule::Side::PropStringType get_string(const LabelledRule::Side &g) {
	return (get_string(g.r).*g.fString)();
}

LabelledRule::Side::PropTermType get_term(const LabelledRule::Side &g) {
	return (get_term(g.r).*g.fTerm)();
}

bool has_stereo(const LabelledRule::Side &g) {
	return has_stereo(g.r);
}

LabelledRule::Side::PropStereoType get_stereo(const LabelledRule::Side &g) {
	return (get_stereo(g.r).*g.fStereo)();
}

LabelledRule::Side::PropMoleculeType get_molecule(const LabelledRule::Side &g) {
	return (get_molecule(g.r).*g.fMol)();
}

const std::vector<std::unique_ptr<LabelledRule::MatchConstraint>> &
get_match_constraints(const LabelledRule::Side &g) {
	return g.data.matchConstraints;
}

std::size_t get_num_connected_components(const LabelledRule::Side &g) {
	return g.data.numComponents;
}

const std::vector<std::size_t> get_component(const LabelledRule::Side &g) {
	return g.data.component;
}

LabelledRule::Side::ComponentGraph
get_component_graph(std::size_t i, const LabelledRule::Side &g) {
	assert(i < get_num_connected_components(g));
	LabelledRule::Side::ComponentFilter filter(&get_graph(g), &g.data.component, i);
	return LabelledRule::Side::ComponentGraph(get_graph(g), filter, filter);
}

const std::vector<boost::graph_traits<GraphType>::vertex_descriptor> &
get_vertex_order_component(std::size_t i, const LabelledRule::Side &g) {
	assert(i < get_num_connected_components(g));
	auto &vertex_orders = g.data.vertex_orders;
	// the number of connected components is initialized externally after construction, so we have this annoying hax
	if(vertex_orders.empty()) vertex_orders.resize(get_num_connected_components(g));
	if(vertex_orders[i].empty())
		vertex_orders[i] = get_vertex_order(lib::GraphMorphism::DefaultFinderArgsProvider(), get_component_graph(i, g));
	return vertex_orders[i];
}

} // namespace mod::lib::rule