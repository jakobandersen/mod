#include "Write.hpp"

#include <mod/Post.hpp>
#include <mod/hyperflow/Solution.hpp>
#include <mod/lib/CombiOpt/Solver.hpp>
#include <mod/lib/DG/IO/Write.hpp>
#include <mod/lib/HyperFlow/Model.hpp>
#include <mod/lib/HyperFlow/Specification.hpp>
#include <mod/lib/Graph/Graph.hpp>
#include <mod/lib/IO/IO.hpp>
#include <mod/lib/IO/Json.hpp>

#include <jla_boost/graph/PairToRangeAdaptor.hpp>

#include <boost/lexical_cast.hpp>

namespace mod::lib::HyperFlow::Write {

//------------------------------------------------------------------------------
// Printer
//------------------------------------------------------------------------------

void Printer::pushInEdgeLabel(std::function<std::string(dg::DG::Vertex)> f) {
	inEdgeLabels.push_back(f);
}

void Printer::popInEdgeLabel() {
	assert(!inEdgeLabels.empty());
	inEdgeLabels.pop_back();
}

void Printer::pushOutEdgeLabel(std::function<std::string(dg::DG::Vertex)> f) {
	outEdgeLabels.push_back(f);
}

void Printer::popOutEdgeLabel() {
	assert(!outEdgeLabels.empty());
	outEdgeLabels.pop_back();
}

//------------------------------------------------------------------------------
// Algorithms
//------------------------------------------------------------------------------

std::pair<std::optional<std::string>, std::optional<std::string>>
summary(const Model &model, const int solId, const DG::Write::Data &dgData,
        DG::Write::Printer &dgPrinter,
        const IO::Graph::Write::Options &graphOptions,
        Printer &flowPrinter) {
	IO::post() << "summarySubsection \"Solution " << solId << "\"\n";
	const std::string commonName = "f_" + boost::lexical_cast<std::string>(model.specification->id)
	                               + "_" + boost::lexical_cast<std::string>(solId);
	const CombiOpt::Result &sol = model.getSolutions()[solId];
	{ // listing
		post::FileHandle s(IO::makeUniqueFilePrefix() + commonName + "_listing.tex");
		s << "\\begin{verbatim}\n";
		model.listSolution(s, sol);
		s << "\\end{verbatim}\n";
		IO::post() << "summarySubsubsection \"Overall Data\"\n";
		IO::post() << "summaryInput \"" << static_cast<std::string>(s) << "\"\n";
	}

	const auto getEdgeFlow = [&model, &sol](const auto vHyper, const lib::DG::Hyper &dg) {
		return sol.evaluate(model.compileExpr(hyperflow::vars::edgeFlow[dg.getInterfaceEdge(vHyper)]));
	};
	const auto getVertexFlow = [&model, &sol](const auto vHyper, const lib::DG::Hyper &dg) {
		return sol.evaluate(model.compileExpr(hyperflow::vars::vertexFlow[dg.getInterfaceVertex(vHyper)]));
	};
	const auto makeFlowLabel = [](const auto &val) {
		return "f " + boost::lexical_cast<std::string>(val);
	};
	if(flowPrinter.withFlowLabel) {
		dgPrinter.pushEdgeLabel([getEdgeFlow, makeFlowLabel](const auto vHyper, const lib::DG::Hyper &dg) {
			return makeFlowLabel(getEdgeFlow(vHyper, dg));
		});
	}

	const auto oldAuxPrinter = dgPrinter.baseOptions.auxPrinter;
	const auto rawAuxPrinter = [&model, &sol, makeFlowLabel, &dgPrinter, &flowPrinter]
			(const lib::DG::Hyper &dg, const DG::Write::Options &options, DG::Write::SyntaxPrinter &print,
			 std::string colour) {
		const auto &g = dg.getGraph();
		for(const auto vHyper : asRange(vertices(g))) {
			if(g[vHyper].kind != lib::DG::HyperVertexKind::Vertex) continue;
			const auto vId = get(boost::vertex_index_t(), g, vHyper);
			const auto &graph = *g[vHyper].graph;
			const auto inFlow = sol.evaluate(model.compileExpr(hyperflow::vars::inFlow[dg.getInterfaceVertex(vHyper)]));
			const auto outFlow = sol.evaluate(model.compileExpr(hyperflow::vars::outFlow[dg.getInterfaceVertex(vHyper)]));
			if(inFlow.isZero() && outFlow.isZero()) continue;

			const auto vsDup = vertices(options.dupGraph);
			const auto vDupFirst = std::find_if(vsDup.first, vsDup.second, [&options, vHyper](const auto vDup) {
				return options.dupGraph[vDup].v == vHyper;
			});
			const auto vDupLast = std::find_if(vDupFirst, vsDup.second, [&options, vHyper](const auto vDup) {
				return options.dupGraph[vDup].v != vHyper;
			});

			std::string comment = "inFlow/outFlow, id = ";
			if(vDupFirst == vDupLast) {
				comment += boost::lexical_cast<std::string>(vId) + "-no-dup";
			} else {
				comment += options.vDupToId(*vDupFirst, dg);
			}
			comment += ", graphName = " + graph.getName()
			           + ", inFlow = " + boost::lexical_cast<std::string>(inFlow)
			           + ", outFlow = " + boost::lexical_cast<std::string>(outFlow);
			print.comment(comment);
			if(vDupFirst == vDupLast) continue;
			if(!options.isVertexVisible(vHyper, dg)) continue;

			const std::string id = options.vDupToId(*vDupFirst, dg);
			const std::string ioId = id + "-IOFlow";
			print.vertexHidden(ioId, options.withGraphImages);

			const auto makeIOLabel = [&dgPrinter, &flowPrinter, &dg, vHyper, &makeFlowLabel]
					(const auto &callbacks, const auto &flow) {
				std::string label;
				if(!callbacks.empty())
					label += callbacks.front()(dg.getInterfaceVertex(vHyper));
				for(unsigned int i = 1; i < callbacks.size(); i++) {
					label += dgPrinter.edgeLabelSep;
					label += callbacks[i](dg.getInterfaceVertex(vHyper));
				}
				if(flowPrinter.withFlowLabel) {
					if(!callbacks.empty()) label += dgPrinter.edgeLabelSep;
					label += makeFlowLabel(flow);
				}
				return label;
			};

			if(!inFlow.isZero()) {
				std::string label = makeIOLabel(flowPrinter.inEdgeLabels, inFlow);
				print.shortcutEdge(ioId, id, label, colour, !outFlow.isZero());
			}
			if(!outFlow.isZero()) {
				std::string label = makeIOLabel(flowPrinter.outEdgeLabels, outFlow);
				print.shortcutEdge(id, ioId, label, colour, !inFlow.isZero());
			}
		}
	};
	const auto auxPrinterColour = [oldAuxPrinter, rawAuxPrinter, &flowPrinter](
			const lib::DG::Hyper &dg, const DG::Write::Options &options, DG::Write::SyntaxPrinter &print) {
		if(oldAuxPrinter) oldAuxPrinter(dg, options, print);
		rawAuxPrinter(dg, options, print, flowPrinter.unfilteredFlowColour);
	};
	const auto auxPrinterNoColour = [oldAuxPrinter, rawAuxPrinter](
			const lib::DG::Hyper &dg, const DG::Write::Options &options, DG::Write::SyntaxPrinter &print) {
		if(oldAuxPrinter) oldAuxPrinter(dg, options, print);
		rawAuxPrinter(dg, options, print, "");
	};

	std::optional<std::string> resUnfiltered, resFiltered;
	if(flowPrinter.printUnfiltered) {
		dgPrinter.pushVertexColour([getVertexFlow, &flowPrinter](const auto vHyper, const lib::DG::Hyper &dg) {
			return getVertexFlow(vHyper, dg).isZero() ? "" : flowPrinter.unfilteredFlowColour;
		}, false);
		dgPrinter.pushEdgeColour([getEdgeFlow, &flowPrinter](const auto vHyper, const lib::DG::Hyper &dg) {
			return getEdgeFlow(vHyper, dg).isZero() ? "" : flowPrinter.unfilteredFlowColour;
		});
		dgPrinter.baseOptions.auxPrinter = auxPrinterColour;
		dgPrinter.pushSuffix(commonName + "_unfilt");

		const auto files = dgPrinter.printHyper(dgData, graphOptions);
		resUnfiltered = files.first;
		std::string fileNoExt = files.first;
		fileNoExt.erase(end(fileNoExt) - 4, end(fileNoExt));
		IO::post() << "summaryFlowUnfiltered \"dgFlow_" << model.specification->id << ", "
		           << solId << "\" \"" << fileNoExt << "\"\n";

		dgPrinter.popSuffix();
		dgPrinter.baseOptions.auxPrinter = oldAuxPrinter;
		dgPrinter.popEdgeColour();
		dgPrinter.popVertexColour();
	}
	if(flowPrinter.printFiltered) {
		dgPrinter.pushVertexVisible([getVertexFlow](const auto vHyper, const lib::DG::Hyper &dg) {
			return !getVertexFlow(vHyper, dg).isZero();
		});
		dgPrinter.pushEdgeVisible([getEdgeFlow](const auto vHyper, const lib::DG::Hyper &dg) {
			return !getEdgeFlow(vHyper, dg).isZero();
		});
		dgPrinter.baseOptions.auxPrinter = auxPrinterNoColour;
		dgPrinter.pushSuffix(commonName + "_filt");

		const auto files = dgPrinter.printHyper(dgData, graphOptions);
		resFiltered = files.first;
		std::string fileNoExt = files.first;
		fileNoExt.erase(end(fileNoExt) - 4, end(fileNoExt));
		IO::post() << "summaryFlowFiltered \"dgFlow_" << model.specification->id << ", "
		           << solId << "\" \"" << fileNoExt << "\"\n";

		dgPrinter.popSuffix();
		dgPrinter.baseOptions.auxPrinter = oldAuxPrinter;
		dgPrinter.popEdgeVisible();
		dgPrinter.popVertexVisible();
	}

	if(flowPrinter.withFlowLabel) dgPrinter.popEdgeLabel();
	IO::post().flush();
	return {resFiltered, resUnfiltered};
}

} // namespace mod::lib::HyperFlow::Write