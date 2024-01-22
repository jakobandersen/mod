#include "Write.hpp"

#include <mod/Config.hpp>
#include <mod/Post.hpp>
#include <mod/rule/Rule.hpp>
#include <mod/lib/DG/Hyper.hpp>
#include <mod/lib/DG/VertexMapping.hpp>
#include <mod/lib/Graph/Single.hpp>
#include <mod/lib/Graph/Properties/Stereo.hpp>
#include <mod/lib/Graph/Properties/String.hpp>
#include <mod/lib/Graph/Properties/Term.hpp>
#include <mod/lib/GraphMorphism/LabelledMorphism.hpp>
#include <mod/lib/GraphMorphism/VF2Finder.hpp>
#include <mod/lib/GraphMorphism/IO/WriteConstraints.hpp>
#include <mod/lib/IO/IO.hpp>
#include <mod/lib/Rules/Real.hpp>
#include <mod/lib/Rules/IO/DepictionData.hpp>
#include <mod/lib/Rules/Properties/Term.hpp>

#include <boost/lexical_cast.hpp>

namespace mod::lib::DG::Write {
namespace {
namespace GM = jla_boost::GraphMorphism;

template<typename F>
void forEachMatch(const NonHyper &dg, HyperVertex v, const lib::Rules::Real &rReal, const int verbosity, F f) {
	IO::Logger logger(std::cout);
	bool derivationFound = false;

	const HyperGraphType &dgGraph = dg.getHyper().getGraph();
	assert(v != dgGraph.null_vertex());
	assert(dgGraph[v].kind == HyperVertexKind::Edge);
	const auto ls = dg.getLabelSettings();
	VertexMapping::Input input{rReal};
	for(const auto vAdj: asRange(inv_adjacent_vertices(v, dgGraph)))
		input.lugG.push_back(&dgGraph[vAdj].graph->getLabelledGraph());
	for(const auto vAdj: asRange(adjacent_vertices(v, dgGraph)))
		input.lugH.push_back(&dgGraph[vAdj].graph->getLabelledGraph());

	if(verbosity != 0) {
		logger.indent() << "WriteDerivation: ls=" << ls << std::endl;
		++logger.indentLevel;
	}

	VertexMapping::foreachRuleMatchedToGandH(
			ls, input, logger, verbosity, true, 1,
			[ls, &input, &f, &derivationFound](IO::Logger logger, const Rules::Real *rPtr, const auto &rightMap) {
				auto mr = [&f, &derivationFound, &rPtr, matchCount = 0](
						auto &&m,
						const lib::Rules::GraphType &gUpper,
						const lib::Rules::GraphType &gLower) mutable {
					derivationFound = true;
					std::string strMatch = boost::lexical_cast<std::string>(matchCount);
					f(*rPtr, gUpper, gLower, m, strMatch);
					++matchCount;
					return true;
				};
				//		mod::postSection("Bah");
				//		lib::IO::Graph::Write::Options options;
				//		options.withRawStereo = true;
				//		options.withColour = true;
				//		options.edgesAsBonds = true;
				//		options.withIndex = true;
				//		lib::IO::Rules::Write::summary(rReal, options, options);
				//		lib::IO::Rules::Write::summary(*rLower, options, options);
				//		std::cout << "morphismSelectByLabelSettings: " << dg.getLabelSettings() << std::endl;
				lib::GraphMorphism::morphismSelectByLabelSettings(input.r->getDPORule(),
				                                                  rPtr->getDPORule(),
				                                                  ls,
				                                                  lib::GraphMorphism::VF2Monomorphism(),
				                                                  std::ref(mr),
				                                                  lib::Rules::MembershipPredWrapper());
				//		std::cout << "morphismSelectByLabelSettings done" << std::endl;
				return true;
			});
	if(!derivationFound) {
		std::string msg = "No derivation exists for rule " + rReal.getName() + ".";
		throw LogicError(std::move(msg));
	}
}

} // namespace

std::vector<std::pair<std::string, std::string>>
summaryDerivation(const NonHyper &dg,
                  HyperVertex v,
                  const IO::Graph::Write::Options &options,
                  const std::string &nomatchColour,
                  const std::string &matchColour,
                  const int verbosity) {
	const auto &dgHyper = dg.getHyper();
	const auto &dgGraph = dgHyper.getGraph();
	const auto &rules = dgHyper.getRulesFromEdge(v);
	assert(!rules.empty());
	std::vector<std::pair<std::string, std::string>> res;
	for(const lib::Rules::Real *rPtr: rules) {
		const lib::Rules::Real &rReal = *rPtr;
		const auto printHeader = [v, &dgGraph, &rReal]() {
			IO::post() << "summarySectionNoEscape \"Derivation " << get(boost::vertex_index_t(), dgGraph, v);
			IO::post() << ", $r_{" << rReal.getId() << "}$";
			IO::post() << ", \\{";
			bool first = true;
			for(auto vIn: asRange(inv_adjacent_vertices(v, dgGraph))) {
				if(!first) IO::post() << ", ";
				else first = false;
				IO::post() << "\\texttt{\\ensuremath{\\mathrm{" << dgGraph[vIn].graph->getName() << "}}}";
			}
			IO::post() << "\\} \\texttt{->} \\{";
			first = true;
			for(auto vOut: asRange(adjacent_vertices(v, dgGraph))) {
				if(!first) IO::post() << ", ";
				else first = false;
				IO::post() << "\\texttt{\\ensuremath{\\mathrm{" << dgGraph[vOut].graph->getName() << "}}}";
			}
			IO::post() << "\\}\"" << std::endl;
			{
				std::string file = IO::makeUniqueFilePrefix() + "der_constraints.tex";
				post::FileHandle s(file);
				for(const auto &c: get_match_constraints(get_labelled_left(rReal.getDPORule())))
					lib::GraphMorphism::Write::texConstraint(s, getL(rReal.getDPORule().getRule()), *c);
				IO::post() << "summaryInput \"" << file << "\"" << std::endl;
			}
		};

		// a copy where we change the coordinates at each match
		lib::Rules::Real rUpperCopy(lib::Rules::LabelledRule(rReal.getDPORule(), false), rReal.getLabelType());
		bool first = true;
		const auto f = [&res, &options, &nomatchColour, &matchColour, &rUpperCopy, printHeader, &first](
				const lib::Rules::Real &rLower,
				const lib::Rules::GraphType &gUpper, const lib::Rules::GraphType &gLower,
				const auto &m, const std::string &strMatch) {
			if(!getConfig().dg.dryDerivationPrinting.get()) {
				if(first) printHeader();
				first = false;

				const auto visibleRule = jla_boost::AlwaysTrue();
				const auto vColourRule = jla_boost::Nop<std::string>();
				const auto eColourRule = jla_boost::Nop<std::string>();
				const auto visibleInstantiation = jla_boost::AlwaysTrue();
				const auto vColourInstantiation = [&nomatchColour, &matchColour, &gUpper, &gLower, &m](
						lib::Rules::Vertex v) -> std::string {
					const auto vUpper = get_inverse(m, gUpper, gLower, v);
					const bool matched = vUpper != boost::graph_traits<lib::Rules::GraphType>::null_vertex();
					return matched ? matchColour : nomatchColour;
				};
				const auto eColourInstantiation = [&nomatchColour, &matchColour, &gUpper, &gLower, &m](
						lib::Rules::Edge e) -> std::string {
					const auto vSrc = source(e, gLower);
					const auto vTar = target(e, gLower);
					const auto vSrcUpper = get_inverse(m, gUpper, gLower, vSrc);
					const auto vTarUpper = get_inverse(m, gUpper, gLower, vTar);
					if(vSrcUpper == boost::graph_traits<lib::Rules::GraphType>::null_vertex()) return nomatchColour;
					if(vTarUpper == boost::graph_traits<lib::Rules::GraphType>::null_vertex()) return nomatchColour;
					const auto matched = edge(vSrcUpper, vTarUpper, gUpper).second;
					return matched ? matchColour : nomatchColour;
				};
				using Vertex = boost::graph_traits<lib::Rules::GraphType>::vertex_descriptor;
				std::map<Vertex, Vertex> map;
				for(const auto vUpper: asRange(vertices(gUpper)))
					map.emplace(vUpper, get(m, gUpper, gLower, vUpper));
				rUpperCopy.getDepictionData().copyCoords(rLower.getDepictionData(), map);
				std::string fileNoExt1 = Rules::Write::pdf(
						rUpperCopy, options, strMatch + "_derL", strMatch + "_derK",
						strMatch + "_derR",
						Rules::Write::BaseArgs{visibleRule, vColourRule, eColourRule});
				std::string fileNoExt2 = Rules::Write::pdf(
						rLower, options, strMatch + "_derG", strMatch + "_derD",
						strMatch + "_derH",
						Rules::Write::BaseArgs{visibleInstantiation, vColourInstantiation, eColourInstantiation});
				IO::post() << "summaryDerivation \"" << fileNoExt1 << "_" << strMatch << "\" \"" << fileNoExt2 << "_"
				           << strMatch << "\"" << std::endl;
				res.emplace_back(fileNoExt1 + "_" + strMatch, fileNoExt2 + "_" + strMatch);
			}
		};
		forEachMatch(dg, v, rReal, verbosity, f);
	}
	return res;
}

} // namespace mod::lib::DG::Write