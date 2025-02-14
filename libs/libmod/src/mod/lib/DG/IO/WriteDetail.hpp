#ifndef MOD_LIB_DG_IO_WRITEDETAIL_HPP
#define MOD_LIB_DG_IO_WRITEDETAIL_HPP

#include <mod/lib/Graph/Graph.hpp>
#include <mod/lib/DG/IO/Write.hpp>

#include <jla_boost/graph/PairToRangeAdaptor.hpp>

#include <boost/lexical_cast.hpp>

namespace mod::lib::DG::Write {

template<typename T>
std::string toStr(const T &t) {
	return boost::lexical_cast<std::string>(t);
}

namespace detail {

inline std::string hyperEdgeComment(const Hyper &dg, Vertex v) {
	unsigned int vId = get(boost::vertex_index_t(), dg.getGraph(), v);
	return "id = " + toStr(vId) + toStr(dg.getDerivation(v));
}

template<typename Body>
void
forEachVertex(const Hyper &dg, const Options &options, SyntaxPrinter &print, bool printHeader, Body body) {
	using DupVertex = Options::DupVertex;
	const auto &g = dg.getGraph();
	const auto &dupGraph = options.dupGraph;
	DupVertex prevVertex = dupGraph.null_vertex();
	for(const DupVertex vDup : asRange(vertices(dupGraph))) {
		const Vertex v = dupGraph[vDup].v;
		if(g[v].kind != HyperVertexKind::Vertex) continue;
		unsigned int vId = get(boost::vertex_index_t(), g, v);
		const auto &graph = *g[v].graph;
		if(printHeader && (prevVertex == dupGraph.null_vertex() || dupGraph[prevVertex].v != dupGraph[vDup].v)) {
			// print header for vertex
			std::string comment = "id = " + toStr(vId);
			comment += ", graphName = ";
			comment += graph.getName();
			print.comment(comment);
		}
		prevVertex = vDup;
		if(!options.isVertexVisible(v, dg)) continue;
		body(vDup);
	}
}

template<typename Body>
void forEachExplicitHyperEdge(const Hyper &dg, const Options &options, SyntaxPrinter &print, Body body) {
	using DupVertex = Options::DupVertex;
	const auto &g = dg.getGraph();
	const auto &dupGraph = options.dupGraph;
	DupVertex prevVertex = dupGraph.null_vertex();
	for(const DupVertex vDup : asRange(vertices(dupGraph))) {
		const Vertex v = dupGraph[vDup].v;
		if(g[v].kind != HyperVertexKind::Edge) continue;
		if(!options.isHyperedgeVisible(v, dg)) continue;
		unsigned int inDegreeVisible = options.inDegreeVisible(vDup, dg).first;
		unsigned int outDegreeVisible = options.outDegreeVisible(vDup, dg).first;

		// if all sources and targets are hidden, then hide this edge
		if(inDegreeVisible == 0 && outDegreeVisible == 0) continue;

		const bool isShortcutEdge = options.isShortcutEdge(vDup, dg, inDegreeVisible, outDegreeVisible);
		if(!isShortcutEdge) {
			if(prevVertex == dupGraph.null_vertex() || dupGraph[prevVertex].v != dupGraph[vDup].v) {
				print.comment(hyperEdgeComment(dg, v));
			}
			prevVertex = vDup;
			body(vDup);
		}
	}
}

template<typename TailBody, typename HeadBody, typename ShortcutBody>
void forEachConnector(const Hyper &dg, const Options &options, SyntaxPrinter &print,
                      TailBody tailBody, HeadBody headBody, ShortcutBody shortcutBody) {
	using DupVertex = Options::DupVertex;
	const auto &g = dg.getGraph();
	const auto &dupGraph = options.dupGraph;
	DupVertex prevVertex = dupGraph.null_vertex();
	for(const DupVertex vDup : asRange(vertices(dupGraph))) {
		const Vertex v = dupGraph[vDup].v;
		if(g[v].kind != HyperVertexKind::Edge) continue;
		if(prevVertex == dupGraph.null_vertex() || dupGraph[prevVertex].v != dupGraph[vDup].v) {
			print.comment(detail::hyperEdgeComment(dg, v));
		} else {
			print.comment("");
		}
		prevVertex = vDup;

		if(!options.isHyperedgeVisible(v, dg)) continue;

		int inDegreeVisible, outDegreeVisible;
		DupVertex vDupTailFirst, vDupHeadFirst;
		std::tie(inDegreeVisible, vDupTailFirst) = options.inDegreeVisible(vDup, dg);
		std::tie(outDegreeVisible, vDupHeadFirst) = options.outDegreeVisible(vDup, dg);

		// if all sources and targets are hidden, then hide this edge
		if(inDegreeVisible == 0 && outDegreeVisible == 0) continue;

		const bool isShortcutEdge = options.isShortcutEdge(vDup, dg, inDegreeVisible, outDegreeVisible);
		if(!isShortcutEdge) {
			// dupVertex -> count
			std::map<DupVertex, int> tailCount, headCount;
			for(const DupVertex vDupIn : asRange(inv_adjacent_vertices(vDup, dupGraph))) {
				auto iter = tailCount.find(vDupIn);
				if(iter == end(tailCount)) tailCount[vDupIn] = 1;
				else iter->second++;
			}
			for(const DupVertex vDupOut : asRange(adjacent_vertices(vDup, dupGraph))) {
				auto iter = headCount.find(vDupOut);
				if(iter == end(headCount)) headCount[vDupOut] = 1;
				else iter->second++;
			}

			for(const auto[vDupAdj, count] : tailCount) {
				const Vertex vAdj = options.dupGraph[vDupAdj].v;
				if(!options.isVertexVisible(vAdj, dg)) continue;
				const auto invIter = headCount.find(vDupAdj);
				int maxCount = count;
				if(invIter != end(headCount)) maxCount += invIter->second;
				tailBody(vDup, vDupAdj, count, maxCount);
			}
			for(const auto[vDupAdj, count] : headCount) {
				const Vertex vAdj = options.dupGraph[vDupAdj].v;
				if(!options.isVertexVisible(vAdj, dg)) continue;
				const auto invIter = tailCount.find(vDupAdj);
				int maxCount = count;
				if(invIter != end(tailCount)) maxCount += invIter->second;
				headBody(vDup, vDupAdj, count, maxCount);
			}
		} else {
			const auto vReverse = dg.getReverseEdge(v);
			const bool hasReverse = vReverse != g.null_vertex()
			                        && options.isHyperedgeVisible(vReverse, dg);
			if(hasReverse) {
				// check if the reverse goes between the same vertex incarnations
				// TODO
			}
			shortcutBody(vDup, vDupTailFirst, vDupHeadFirst, hasReverse);
		}
	}
}

} // namespace detail
} // namespace namespace mod::lib::DG::Write

#endif // MOD_LIB_DG_IO_WRITEDETAIL_HPP