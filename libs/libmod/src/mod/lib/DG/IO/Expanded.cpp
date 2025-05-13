#include "Expanded.hpp"

#include <mod/Error.hpp>
#include <mod/Post.hpp>
#include <mod/lib/DG/GraphDecl.hpp>
#include <mod/lib/DG/Expanded.hpp>
#include <mod/lib/DG/IO/WriteDetail.hpp>
#include <mod/lib/Graph/IO/Write.hpp>
#include <mod/lib/IO/IO.hpp>

#include <jla_boost/graph/PairToRangeAdaptor.hpp>

#include <iostream>

namespace mod::lib::DG::Write::Expanded {
using DupVertex = Options::DupVertex;
using DupEdge = Options::DupEdge;

std::pair<std::string, std::string> tikz(const lib::DG::Expanded &dg, const Options &options) {
	const auto &dgHyper = dg.dgHyper.getGraph();
	auto vExpandedTransitToId = [&dg, &options](DupVertex vDup, ExpandedVertex vExpanded) {
		assert(dg.getGraph()[vExpanded].vHyper == options.dupGraph[vDup].v);
		unsigned int vExpandedId = get(boost::vertex_index_t(), dg.getGraph(), vExpanded);
		return options.vDupToId(vDup, dg.dgHyper) + "-t" + toStr(vExpandedId);
	};

	IO::Graph::Write::Options graphOptions; // we don't actually need these
	std::string coordFile = coords(dg.dgHyper, options, graphOptions);
	TikzPrinter p(IO::makeUniqueFilePrefix() + "DGExpanded.tex", coordFile, options, graphOptions);
	p.begin();
	// create the circles for the vertices
	detail::forEachVertex(dg.dgHyper, options, p, true, [&dg, &options, &p](DupVertex vDup) {
		std::string label = options.getVertexLabel(dg.dgHyper, vDup);
		std::string colour = options.getVertexColour(vDup, dg.dgHyper);
		std::string id = options.vDupToId(vDup, dg.dgHyper);
		std::ostream &s = p.s;

		bool haveLabel = label != "";
		s << "\\node[modStyleDGHyperVertex, at=(v-coord-" << id << ")";
		if(!colour.empty()) s << ", draw=" << colour;
		if(haveLabel) {
			s << ", label=below:{";
			if(options.labelsAsLatexMath) s << IO::asLatexMath(label);
			else s << IO::escapeForLatex(label);
			s << "}";
		}
		s << ", circle, minimum size=75";
		s << "] (v-" << id << ") {";
		s << "};" << std::endl;
	});
	// print all the explicit hyperedges
	detail::forEachExplicitHyperEdge(dg.dgHyper, options, p, [&dg, &options, &p](DupVertex vDup) {
		HyperVertex v = options.dupGraph[vDup].v;
		std::string id = options.vDupToId(vDup, dg.dgHyper);
		std::string label = options.getHyperedgeLabel(v, dg.dgHyper);
		std::string colour = options.getHyperedgeColour(v, dg.dgHyper);
		p.hyperEdge(id, label, colour);
	});
	// prepare all the transit nodes, they need angles
	std::unordered_map<DupVertex, std::vector<std::pair<ExpandedVertex, int> > > transitAngles; // dupVertex -> [transitVertex x angle]
	detail::forEachVertex(dg.dgHyper, options, p, false,
	                      [&transitAngles, &dg, &options](DupVertex vDup) {
		                      auto &angles = transitAngles[vDup];
		                      HyperVertex vHyper = options.dupGraph[vDup].v;
		                      const auto &vertexData = dg.getVertexData(vHyper);
		                      const unsigned int totalNumTransit =
				                      vertexData.inVertices.size() + vertexData.outVertices.size();
		                      const unsigned int angleSep = std::max(360 / totalNumTransit, 7u);
		                      unsigned int nextAngle = 0;
		                      for(ExpandedVertex vExpanded : vertexData.inVertices) {
			                      angles.emplace_back(vExpanded, nextAngle);
			                      nextAngle += angleSep;
		                      }
		                      for(ExpandedVertex vExpanded : vertexData.outVertices) {
			                      angles.emplace_back(vExpanded, nextAngle);
			                      nextAngle += angleSep;
		                      }
	                      });
	// for now we just give the same angle in all duplicates, it should be based on where they are connected instead
	// TODO: change the angles
	// create the transit vertices
	detail::forEachVertex(dg.dgHyper, options, p, true,
	                      [&vExpandedTransitToId, &p, &transitAngles, &dg, &dgHyper, &options](
			                      DupVertex vDup) {
		                      const auto &angles = transitAngles[vDup];
		                      std::string idVertex = options.vDupToId(vDup, dg.dgHyper);
		                      ExpandedVertex vInExp = dg.getVertexData(
				                      options.dupGraph[vDup].v).inputEdge;
		                      ExpandedVertex vOutExp = dg.getVertexData(
				                      options.dupGraph[vDup].v).outputEdge;
		                      assert(in_degree(vInExp, dg.getGraph()) +
		                             out_degree(vInExp, dg.getGraph()) == 1);
		                      assert(in_degree(vOutExp, dg.getGraph()) +
		                             out_degree(vOutExp, dg.getGraph()) == 1);
		                      ExpandedVertex vInTransitExp = *adjacent_vertices(vInExp,
		                                                                        dg.getGraph()).first;
		                      ExpandedVertex vOutTransitExp = *inv_adjacent_vertices(vOutExp,
		                                                                             dg.getGraph()).first;
		                      for(const auto &vTransit : angles) {
			                      std::string idTransit = vExpandedTransitToId(vDup, vTransit.first);
			                      HyperVertex vRepr = dg.getGraph()[vTransit.first].eRepr;
			                      std::string label;
			                      if(vRepr == dgHyper.null_vertex() && dg.isCatchAll(vTransit.first)) {
				                      label = "catchAll";
			                      }
			                      p.transitVertex(idVertex, idTransit, toStr(vTransit.second), label);
			                      if(vTransit.first == vInTransitExp ||
			                         vTransit.first == vOutTransitExp) {
				                      std::string ioId = "IO-" + idVertex + "-";
				                      if(vTransit.first == vInTransitExp) {
					                      ioId += toStr(get(boost::vertex_index_t(), dg.getGraph(),
					                                        vInTransitExp));
				                      } else {
					                      ioId += toStr(get(boost::vertex_index_t(), dg.getGraph(),
					                                        vOutTransitExp));
				                      }
				                      p.s << "\\node[modStyleDGHyperVertexHidden";
				                      p.s << ", at=($(v-" << idTransit << "." << vTransit.second << ")+("
				                          << vTransit.second << ":1em)$)";
				                      p.s << "] (v-" << ioId << ") {};" << std::endl;
				                      if(vTransit.first == vInTransitExp) {
					                      p.connector(ioId, idTransit, "", 1, 1);
				                      } else {
					                      p.connector(idTransit, ioId, "", 1, 1);
				                      }
			                      }
		                      }
	                      });
	// and now all the connectors
	auto vDupToTransit = [&dg, &options](const DupVertex vDupEdge, const DupVertex vDupVertex, const bool isTail) {
		HyperVertex vHyperVertex = options.dupGraph[vDupVertex].v;
		ExpandedVertex vExpandedEdge = dg.getExpandedFromHyperEdge(options.dupGraph[vDupEdge].v);
		if(isTail) {
			auto adjIterPair = inv_adjacent_vertices(vExpandedEdge, dg.getGraph());
			auto adjIter = std::find_if(adjIterPair.first, adjIterPair.second,
			                            [&dg, vHyperVertex](ExpandedVertex vExpandedAdj) {
				                            return dg.getGraph()[vExpandedAdj].vHyper == vHyperVertex;
			                            });
			assert(adjIter != adjIterPair.second);
			return *adjIter;
		} else {
			auto adjIterPair = adjacent_vertices(vExpandedEdge, dg.getGraph());
			auto adjIter = std::find_if(adjIterPair.first, adjIterPair.second,
			                            [&dg, vHyperVertex](ExpandedVertex vExpandedAdj) {
				                            return dg.getGraph()[vExpandedAdj].vHyper == vHyperVertex;
			                            });
			assert(adjIter != adjIterPair.second);
			return *adjIter;
		}
	};
	auto fCommonConnector = [&vDupToTransit, &vExpandedTransitToId, &dg, &options, &p]
			(const DupVertex vDupEdge, const DupVertex vDupVertex, const int count, const int maxCount,
			 const bool isTail) {
		HyperVertex vHyperEdge = options.dupGraph[vDupEdge].v;
		HyperVertex vHyperVertex = options.dupGraph[vDupVertex].v;
		ExpandedVertex vExpandedTransit = vDupToTransit(vDupEdge, vDupVertex, isTail);
		std::string edgeId = options.vDupToId(vDupEdge, dg.dgHyper);
		std::string transitId = vExpandedTransitToId(vDupVertex, vExpandedTransit);
		std::string colour = isTail ?
		                     options.getTailColour(vHyperVertex, vHyperEdge, dg.dgHyper)
		                            : options.getHeadColour(vHyperEdge, vHyperVertex, dg.dgHyper);
		for(int i = 1; i <= count; i++) {
			if(isTail) p.tailConnector(transitId, edgeId, colour, i, maxCount);
			else p.headConnector(edgeId, transitId, colour, i, maxCount);
		}
	};
	auto fTail = [&fCommonConnector](const DupVertex vDupEdge, const DupVertex vDupVertex,
	                                 const int count, const int maxCount) {
		fCommonConnector(vDupEdge, vDupVertex, count, maxCount, true);
	};
	auto fHead = [&fCommonConnector](const DupVertex vDupEdge, const DupVertex vDupVertex,
	                                 const int count, const int maxCount) {
		fCommonConnector(vDupEdge, vDupVertex, count, maxCount, false);
	};
	auto fShortcut = [&vExpandedTransitToId, &vDupToTransit, &dg, &options, &p](
			DupVertex vDupEdge, DupVertex vDupTail, DupVertex vDupHead, bool hasReverse) {
		ExpandedVertex vExpandedTail = vDupToTransit(vDupEdge, vDupTail, true);
		ExpandedVertex vExpandedHead = vDupToTransit(vDupEdge, vDupHead, false);
		std::string idTail = vExpandedTransitToId(vDupTail, vExpandedTail);
		std::string idHead = vExpandedTransitToId(vDupHead, vExpandedHead);
		HyperVertex vHyperEdge = options.dupGraph[vDupEdge].v;
		std::string label = options.getHyperedgeLabel(vHyperEdge, dg.dgHyper);
		std::string colour = options.getHyperedgeColour(vHyperEdge, dg.dgHyper);
		p.shortcutEdge(idTail, idHead, label, colour, hasReverse);
	};
	detail::forEachConnector(dg.dgHyper, options, p, fTail, fHead, fShortcut);
	// TODO: still missing IO edges
	// transit edges
	auto fTransit = [&vExpandedTransitToId, &dg, &options, &p](DupVertex vDup) {
		HyperVertex vHyper = options.dupGraph[vDup].v;
		const auto &vertexData = dg.getVertexData(vHyper);
		const auto &expGraph = dg.getGraph();
		for(ExpandedVertex vExpandedIn : vertexData.inVertices) {
			assert(expGraph[vExpandedIn].kind == ExpandedVertexKind::InVertex);
			for(ExpandedVertex vExpandedEdge : asRange(adjacent_vertices(vExpandedIn, expGraph))) {
				assert(expGraph[vExpandedEdge].kind == ExpandedVertexKind::TransitEdge);
				assert(in_degree(vExpandedEdge, expGraph) == 1);
				assert(out_degree(vExpandedEdge, expGraph) == 1);
				ExpandedVertex vExpandedOut = *(adjacent_vertices(vExpandedEdge, expGraph).first);
				assert(expGraph[vExpandedOut].kind == ExpandedVertexKind::OutVertex);
				std::string tailId = vExpandedTransitToId(vDup, vExpandedIn);
				std::string headId = vExpandedTransitToId(vDup, vExpandedOut);
				std::string label = "";
				std::string colour = options.getVertexColour(vDup, dg.dgHyper);
				p.transitEdge(tailId, headId, label, colour);
			}
		}
	};
	detail::forEachVertex(dg.dgHyper, options, p, true, fTransit);
	p.end();
	if(false) {
		auto &s = std::cout;
		for(ExpandedVertex vEdge : asRange(vertices(dg.getGraph()))) {
			auto kind = dg.getGraph()[vEdge].kind;
			if(isKindVertex(kind)) continue;
			if(kind == ExpandedVertexKind::Edge) s << "edge";
			else if(kind == ExpandedVertexKind::TransitEdge) s << "transit";
			else if(kind == ExpandedVertexKind::IOEdge) s << "io";
			else
				MOD_ABORT;
			s << "(" << get(boost::vertex_index_t(), dg.getGraph(), vEdge) << ", hyper=";
			s << get(boost::vertex_index_t(), dg.dgHyper.getGraph(), dg.getGraph()[vEdge].vHyper);
			s << "): ";
			auto printVertex = [&](ExpandedVertex vAdj) {
				s << "(" << get(boost::vertex_index_t(), dg.getGraph(), vAdj);
				s << ", ";
				auto adjKind = dg.getGraph()[vAdj].kind;
				if(adjKind == ExpandedVertexKind::InVertex) s << "in";
				else if(adjKind == ExpandedVertexKind::OutVertex) s << "out";
				else
					MOD_ABORT;
				s << ")";
			};
			s << "{";
			for(ExpandedVertex vAdj : asRange(inv_adjacent_vertices(vEdge, dg.getGraph()))) {
				s << " ";
				printVertex(vAdj);
			}
			s << " } -> {";
			for(ExpandedVertex vAdj : asRange(adjacent_vertices(vEdge, dg.getGraph()))) {
				s << " ";
				printVertex(vAdj);
			}
			s << " }" << std::endl;
		}
	}
	return std::make_pair(static_cast<std::string> (p.s), coordFile);
}

std::string pdf(const lib::DG::Expanded &dg, const DG::Write::Data &data, DG::Write::Printer &printer) {
	DG::Write::Options options = printer.prePrint(data);
	auto tikzFiles = tikz(dg, options);
	printer.postPrint();
	std::string fileNoExt = tikzFiles.first.substr(0, tikzFiles.first.length() - 4);
	std::string fileCoordsNoExt = tikzFiles.second.substr(0, tikzFiles.second.length() - 4);
	IO::post() << "compileTikz \"" << fileNoExt << "\" \"" << fileCoordsNoExt << "\"" << std::endl;
	return fileNoExt + ".pdf";
}

std::string tikzVertex(const lib::DG::Expanded &dgExpandedWrapped, HyperVertex vHyper) {
	const auto &dgExp = dgExpandedWrapped.getGraph();
	const auto &data = dgExpandedWrapped.getVertexData(vHyper);
	assert(vHyper != dgExpandedWrapped.dgHyper.getGraph().null_vertex());
	assert(dgExpandedWrapped.dgHyper.getGraph()[vHyper].kind == HyperVertexKind::Vertex);

	// make 4 columns: in-edges, in-expanded vertices, out-expanded vertices, out-edges
	// the expanded vertices are exactly those in data
	std::vector<ExpandedVertex> inEdges, outEdges;
	std::unordered_map<ExpandedVertex, std::pair<ExpandedVertex, int>> inEdgeMultiplicity, outEdgeMultiplicity;
	for(const ExpandedVertex v : data.inVertices) {
		for(const ExpandedVertex vInEdge : asRange(inv_adjacent_vertices(v, dgExp))) {
			const auto iter = inEdgeMultiplicity.find(vInEdge);
			if(iter == end(inEdgeMultiplicity)) {
				inEdgeMultiplicity.emplace(vInEdge, std::make_pair(v, 1));
				inEdges.push_back(vInEdge);
			} else {
				assert(iter->second.first == v);
				++iter->second.second;
			}
		}
	}
	for(const ExpandedVertex v : data.outVertices) {
		for(const ExpandedVertex vOutEdge : asRange(adjacent_vertices(v, dgExp))) {
			const auto iter = outEdgeMultiplicity.find(vOutEdge);
			if(iter == end(outEdgeMultiplicity)) {
				outEdgeMultiplicity.emplace(vOutEdge, std::make_pair(v, 1));
				outEdges.push_back(vOutEdge);
			} else {
				assert(iter->second.first == v);
				++iter->second.second;
			}
		}
	}
	post::FileHandle s(IO::makeUniqueFilePrefix()
	                   + "ExpandedVertex_" + std::to_string(dgExpandedWrapped.dgHyper.getNonHyper().getId())
	                   + "_" + std::to_string(get(boost::vertex_index_t(), dgExpandedWrapped.dgHyper.getGraph(), vHyper))
	                   + ".tex");
	s << R"X(\begin{tikzpicture})X" << '\n';
	s << R"X(\matrix[row sep=5em] {)X" << '\n';
	const auto max = std::max(std::max(data.inVertices.size(), data.outVertices.size()),
	                          std::max(inEdges.size(), outEdges.size()));
	for(int i = 0; i != max; ++i) {
		if(i < inEdges.size()) {
			const auto v = inEdges[i];
			const auto vId = get(boost::vertex_index_t(), dgExp, v);
			s << "\\node[modStyleDGHyperEdge] (e-in-" << vId << ") {";
			if(dgExp[v].kind == ExpandedVertexKind::IOEdge)
				s << "in";
			else
				s << "$e_{" << vId << "}$";
			s << "};\n";
		}
		s << "\t&[2em] ";
		if(i < data.inVertices.size()) {
			const auto v = data.inVertices[i];
			const auto vId = get(boost::vertex_index_t(), dgExp, v);
			s << "\\node[modStyleDGTransitVertex] (t-in-" << vId << ") {};\n";
		}
		s << "\t&[8em] ";
		if(i < data.outVertices.size()) {
			const auto v = data.outVertices[i];
			const auto vId = get(boost::vertex_index_t(), dgExp, v);
			s << "\\node[modStyleDGTransitVertex] (t-out-" << vId << ") {};\n";
		}
		s << "\t&[2em] ";
		if(i < outEdges.size()) {
			const auto v = outEdges[i];
			const auto vId = get(boost::vertex_index_t(), dgExp, v);
			s << "\\node[modStyleDGHyperEdge] (e-out-" << vId << ") {";
			if(dgExp[v].kind == ExpandedVertexKind::IOEdge)
				s << "out";
			else
				s << "$e_{" << vId << "}$";
			s << "};\n";
		}
		s << R"(\\)" << std::endl;
	}
	s << "};\n";
	for(const ExpandedVertex vIn : data.inVertices) {
		for(const ExpandedVertex eTransit : asRange(adjacent_vertices(vIn, dgExp))) {
			assert(out_degree(eTransit, dgExp) == 1);
			const auto vOut = *adjacent_vertices(eTransit, dgExp).first;
			s << "\\path[modStyleDGHyperConnector] (t-in-"
			  << get(boost::vertex_index_t(), dgExp, vIn) << ") to (t-out-"
			  << get(boost::vertex_index_t(), dgExp, vOut) << ");\n";
		}
	}
	const auto getBend = [&s](const int mult, const int position) -> std::string {
		if(mult == 1) return "";
		double factor = -mult / 2.0 + position + 0.5;
		factor *= 2;
		s << "\\pgfmathparse{" << factor << "*\\modDGHyperConnectorBend}\n";
		s << "\\pgfmathsetmacro{\\modDGHyperConnectorResultAngle}{\\pgfmathresult}\n";
		return "[bend right=\\modDGHyperConnectorResultAngle]";
	};
	for(const auto &p : inEdgeMultiplicity) {
		const auto vEdge = p.first;
		const auto vIn = p.second.first;
		const int mult = p.second.second;
		for(int i = 0; i != mult; ++i) {
			const auto bend = getBend(mult, i);
			s << "\\path[modStyleDGHyperConnector] (e-in-"
			  << get(boost::vertex_index_t(), dgExp, vEdge) << ") to " << bend << "(t-in-"
			  << get(boost::vertex_index_t(), dgExp, vIn) << ");\n";
		}
	}
	for(const auto &p : outEdgeMultiplicity) {
		const auto vEdge = p.first;
		const auto vOut = p.second.first;
		const int mult = p.second.second;
		for(int i = 0; i != mult; ++i) {
			const auto bend = getBend(mult, i);
			s << "\\path[modStyleDGHyperConnector] (t-out-"
			  << get(boost::vertex_index_t(), dgExp, vOut) << ") to " << bend << "(e-out-"
			  << get(boost::vertex_index_t(), dgExp, vEdge) << ");\n";
		}
	}
	s << R"X(\end{tikzpicture})X" << '\n';
	return s;
}

std::string pdfVertex(const lib::DG::Expanded &dg, HyperVertex vHyper) {
	std::string f = tikzVertex(dg, vHyper);
	f.erase(f.end() - 4, f.end());
	IO::post() << "compileTikz \"" << f << "\" \"" << f << "\"" << std::endl;
	return f + ".pdf";
}

} // namespace mod::lib::DG::Write::Expanded