#ifndef MOD_LIB_RC_MATCH_MAKER_COMPONENTWISE_UTIL_HPP
#define MOD_LIB_RC_MATCH_MAKER_COMPONENTWISE_UTIL_HPP

#include <mod/lib/GraphMorphism/LabelledMorphism.hpp>
#include <mod/lib/GraphMorphism/VF2Finder.hpp>
#include <mod/lib/GraphMorphism/Constraints/CheckVisitor.hpp>
#include <mod/lib/Rule/Rule.hpp>

#include <jla_boost/graph/FilteredWrapper.hpp>
#include <jla_boost/graph/morphism/callbacks/Limit.hpp>
#include <jla_boost/graph/morphism/callbacks/SliceProps.hpp>
#include <jla_boost/graph/morphism/callbacks/Store.hpp>
#include <jla_boost/graph/morphism/callbacks/Transform.hpp>
#include <jla_boost/graph/morphism/callbacks/Unwrapper.hpp>
#include <jla_boost/graph/morphism/models/Vector.hpp>

namespace mod::lib::RC {

namespace GM = jla_boost::GraphMorphism;
namespace GM_MOD = lib::GraphMorphism;

template<typename RuleSideDom, typename RuleSideCodom>
struct FilteredWrapperReinterpretMRWrapper {
	template<typename OuterDomain, typename OuterCodomain, typename MR>
	auto operator()(const OuterDomain &gDomain, const OuterCodomain &gCodomain, MR mr) const {
		return GM::unwrapBoth(mr);
	}
};

template<typename Rule>
struct WrappedComponentGraph {
	using ComponentGraph = typename Rule::ComponentGraph;
	using GraphType = jla_boost::FilteredWrapper<ComponentGraph>;
	using PropStringType = typename Rule::PropStringType;
	using PropTermType = typename Rule::PropTermType;
	using PropStereoType = typename Rule::PropStereoType;
public:
	WrappedComponentGraph(const ComponentGraph &g, std::size_t i, const Rule &r)
			: g(jla_boost::makeFilteredWrapper(g)), i(i), r(r) {}

	friend const GraphType &get_graph(const WrappedComponentGraph<Rule> &g) {
		return g.g;
	}

	friend PropStringType get_string(const WrappedComponentGraph<Rule> &g) {
		return get_string(g.r);
	}

	friend PropTermType get_term(const WrappedComponentGraph<Rule> &g) {
		return get_term(g.r);
	}

	friend bool has_stereo(const WrappedComponentGraph<Rule> &g) {
		return has_stereo(g.r);
	}

	friend PropStereoType get_stereo(const WrappedComponentGraph<Rule> &g) {
		return get_stereo(g.r);
	}

	friend const std::vector<typename boost::graph_traits<GraphType>::vertex_descriptor> &
	get_vertex_order(const WrappedComponentGraph<Rule> &g) {
		return get_vertex_order_component(g.i, g.r);
	}
private:
	GraphType g;
	std::size_t i;
	const Rule &r;
};

template<typename Rule>
WrappedComponentGraph<Rule> makeWrappedComponentGraph(const typename Rule::ComponentGraph &g,
                                                      std::size_t i,
                                                      const Rule &r) {
	return WrappedComponentGraph<Rule>(g, i, r);
}

template<typename RuleSideDom, typename RuleSideCodom>
struct RuleRuleComponentMonomorphism {
	using Morphism = GM::VectorVertexMap<typename RuleSideDom::GraphType, typename RuleSideCodom::GraphType>;
public:
	RuleRuleComponentMonomorphism(const RuleSideDom &rsDom,
	                              const RuleSideCodom &rsCodom,
	                              bool enforceConstraints,
	                              LabelSettings labelSettings,
	                              bool verbose, IO::Logger &logger)
			: rsDom(rsDom), rsCodom(rsCodom), enforceConstraints(enforceConstraints), labelSettings(labelSettings),
			  verbose(verbose), logger(logger), haxMorphismLimit(getConfig().rc.componentWiseMorphismLimit) {}

	std::vector<Morphism> operator()(const std::size_t idDom, const std::size_t idCodom) const {
		const auto doIt = [this, idDom, idCodom](auto mrStore) {
			const auto &gDom = get_component_graph(idDom, rsDom);
			const auto &gCodom = get_component_graph(idCodom, rsCodom);
			auto wgDom = makeWrappedComponentGraph(gDom, idDom, rsDom);
			auto wgCodom = makeWrappedComponentGraph(gCodom, idCodom, rsCodom);

			auto makeCheckConstraints = [&](auto &&mrNext) {
				const auto &constraints = get_match_constraints(rsDom);
				auto constraintsIterEnd = enforceConstraints ? constraints.end() : constraints.begin();
				if(verbose)
					logger.indent() << "RuleRuleComponentMonomorphism(" << idDom << ", " << idCodom
					                << ")::makeCheckConstraints: "
					                << std::distance(constraints.begin(), constraintsIterEnd) << std::endl;
				return GraphMorphism::Constraints::makeChecker(
						asRange(std::make_pair(constraints.begin(), constraintsIterEnd)),
						rsCodom, labelSettings, mrNext);
			};
			// First reinterpret the vertex descriptors from the reindexed graphs to their parent graphs.
			auto mrWrapper = FilteredWrapperReinterpretMRWrapper<RuleSideDom, RuleSideCodom>();
			// Then do whatever checked is needed by the labelled morphisms (morphismSelectByLabelSettings injects those).
			// And now process the final morphisms:
			auto mr =
					// The next two unwrappings will increase the domain and codomain,
					// so we first need to capture the mapping:
					// Store them in a vector (which runs through the domain graph):
					GM::makeTransform(GM::ToVectorVertexMap(),
							// Unwrap the filtering by connected component, so we get side graphs:
							            GM::unwrapBoth(
										            // Check constraints using the side graphs:
										            makeCheckConstraints(
												            //				// Unwrap the filtering by rule side, so we get to core graphs:
												            //				GM::makeUnwrapperLeft<typename RuleSideDom::GraphType > (
												            //				GM::makeUnwrapperRight<typename RuleSideCodom::GraphType > (
												            // Slice away the properties, the user must recreate that.
												            GM::makeSliceProps(
														            // And finally push it into our storage:
														            mrStore
												            ))))//))
			;
			auto predWrapper = lib::GraphMorphism::IdentityWrapper();

			//				auto mrPrinter = GraphMorphism::Callback::makePrint(std::cout, patternWrapped, targetWrapped, mrCheckConstraints);
			lib::GraphMorphism::morphismSelectByLabelSettings(wgDom, wgCodom, labelSettings, GM_MOD::VF2Monomorphism(), mr,
			                                                  predWrapper, mrWrapper);
		};
		std::vector<Morphism> morphisms;
		if(haxMorphismLimit == 0) {
			doIt(GM::makeStore(std::back_inserter(morphisms)));
		} else {
			auto limit = GM::makeLimit(haxMorphismLimit, GM::makeStore(std::back_inserter(morphisms)));
			doIt(std::ref(limit));
		}
		if(verbose) {
			logger.indent() << "RuleRuleComponentMonomorphism(" << idDom << ", " << idCodom;
			if(haxMorphismLimit != 0) std::cout << ", limit=" << haxMorphismLimit;
			std::cout << "): got " << morphisms.size() << std::endl;
		}
		return morphisms;
	}
private:
	const RuleSideDom &rsDom;
	const RuleSideCodom &rsCodom;
	const bool enforceConstraints;
	const LabelSettings labelSettings;
	const bool verbose;
	IO::Logger &logger;
	int haxMorphismLimit;
};

template<typename RuleSideDom, typename RuleSideCodom>
auto makeRuleRuleComponentMonomorphism(const RuleSideDom &rsDom,
                                       const RuleSideCodom &rsCodom,
                                       bool enforceConstraints,
                                       LabelSettings labelSettings,
                                       bool verbose, IO::Logger &logger) {
	return RuleRuleComponentMonomorphism<RuleSideDom, RuleSideCodom>(
			rsDom, rsCodom, enforceConstraints, labelSettings, verbose, logger);
}

} // namespace mod::lib::RC

#endif // MOD_LIB_RC_MATCH_MAKER_COMPONENTWISE_UTIL_HPP