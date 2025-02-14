#include "Write.hpp"

#include <mod/Config.hpp>
#include <mod/Post.hpp>
#include <mod/lib/IO/IO.hpp>
#include <mod/lib/RC/Evaluator.hpp>
#include <mod/lib/Rule/Rule.hpp>
#include <mod/lib/Rule/IO/DepictionData.hpp>
#include <mod/lib/Rule/IO/Write.hpp>
#include <mod/lib/Rule/Properties/String.hpp>

namespace mod::lib::RC::Write {

std::string dot(const Evaluator &rc) {
	using Vertex = Evaluator::Vertex;
	using Edge = Evaluator::Edge;
	const Evaluator::GraphType &rcg = rc.getGraph();
	post::FileHandle s(IO::makeUniqueFilePrefix() + "rc.dot");
	std::string fileNoExt = s;
	fileNoExt.erase(end(fileNoExt) - 4, end(fileNoExt));
	s << "digraph g {" << std::endl;

	for(Vertex v: asRange(vertices(rcg))) {
		s << "\t" << get(boost::vertex_index_t(), rcg, v) << " [";
		switch(rcg[v].kind) {
		case Evaluator::VertexKind::Rule:
			s << " label=\"" << rcg[v].rule->getName() << "\"";
			break;
		case Evaluator::VertexKind::Composition:
			s << " shape=point";
			break;
		}
		s << " ];" << std::endl;
	}

	for(Edge e: asRange(edges(rcg))) {
		s << "\t" << get(boost::vertex_index_t(), rcg, source(e, rcg))
		  << " -> " << get(boost::vertex_index_t(), rcg, target(e, rcg)) << " [";
		switch(rcg[e].kind) {
		case Evaluator::EdgeKind::First:
			s << " label=1";
			break;
		case Evaluator::EdgeKind::Second:
			s << " label=2";
			break;
		case Evaluator::EdgeKind::Result:
			break;
		}
		s << " ];" << std::endl;
	}
	s << "}" << std::endl;
	return fileNoExt;
}

std::string svg(const Evaluator &rc) {
	std::string fileNoExt = dot(rc);
	IO::post() << "gv rc \"" << fileNoExt << "\" svg" << std::endl;
	return fileNoExt;
}

std::string pdf(const Evaluator &rc) {
	std::string fileNoExt = svg(rc);
	IO::post() << "svgToPdf \"" << fileNoExt << "\"" << std::endl;
	return fileNoExt;
}

void test(const lib::rule::Rule &rFirst, const lib::rule::Rule &rSecond, const CoreCoreMap &match,
          const lib::rule::Rule &rNew) {
	if(getConfig().rc.printMatchesOnlyHaxChem) {
		const auto &lg = get_labelled_left(rNew.getDPORule());
		const auto &g = get_graph(lg);
		const auto &mol = get_molecule(lg);
		for(const auto v: asRange(vertices(g))) {
			const auto ad = mol[v];
			if(ad.getRadical()) continue;
			if(ad.getCharge() != 0) continue;
			if(ad.getIsotope() != Isotope()) continue;
			int valence = 0;
			for(const auto e: asRange(out_edges(v, g))) {
				switch(mol[e]) {
				case BondType::Single:
					valence += 1;
					break;
				case BondType::Double:
					valence += 2;
					break;
				case BondType::Triple:
					valence += 3;
					break;
				default:
					valence -= 100;
					break;
				}
			}
			switch(ad.getAtomId()) {
			case AtomIds::H:
				if(valence > 1) return;
				break;
			case AtomIds::C:
				if(valence > 4) return;
				break;
			case AtomIds::O:
				if(valence > 2) return;
				break;
			}
		}
	}
	using CoreVertex = lib::rule::Vertex;
	using CoreEdge = lib::rule::Edge;
	rule::Write::Options options;
	options.CollapseHydrogens(true);
	options.EdgesAsBonds(true);
	if(getConfig().rc.matchesWithIndex)
		options.WithIndex(true);
	const auto visible = [](CoreVertex) {
		return true;
	};
	const auto vColour = [](CoreVertex) {
		return std::string();
	};
	const auto eColour = [](CoreEdge) {
		return std::string();
	};
	// make a fake rule with all the vertices and edges, just for coords
	std::map<CoreVertex, CoreVertex> vFirstToCommon, vSecondToCommon, vNewToCommon;
	lib::rule::LabelledRule dpoCommon(rFirst.getDPORule(), false);
	lib::rule::GraphType &gComon = get_graph(dpoCommon);
	lib::rule::PropString &pStringCommon = *dpoCommon.pString;
	for(const CoreVertex v: asRange(vertices(rFirst.getGraph())))
		vFirstToCommon[v] = v;
	// TODO: this will completely break if vertices are deleted in the composed rule
	for(const CoreVertex v: asRange(vertices(rNew.getGraph())))
		vNewToCommon[v] = v;
	// copy rSecond vertices
	for(const CoreVertex v: asRange(vertices(rSecond.getGraph()))) {
		const auto rightIter = match.right.find(v);
		if(rightIter != match.right.end()) {
			vSecondToCommon[v] = rightIter->second;
		} else {
			const CoreVertex vCommon = add_vertex(gComon);
			vSecondToCommon[v] = vCommon;
			gComon[vCommon].membership = lib::rule::Membership::K;
			const std::string &label = rSecond.getGraph()[v].membership == lib::rule::Membership::L
			                           ? get_string(rSecond.getDPORule()).getLeft()[v]
			                           : get_string(rSecond.getDPORule()).getRight()[v];
			pStringCommon.add(vCommon, label, label);
		}
	}
	// copy rSecond edges
	for(const CoreEdge e: asRange(edges(rSecond.getGraph()))) {
		const CoreVertex vSrcSecond = source(e, rSecond.getGraph());
		const CoreVertex vTarSecond = target(e, rSecond.getGraph());
		const auto iterSrc = vSecondToCommon.find(vSrcSecond);
		const auto iterTar = vSecondToCommon.find(vTarSecond);
		assert(iterSrc != end(vSecondToCommon));
		assert(iterTar != end(vSecondToCommon));
		const CoreVertex vSrc = iterSrc->second;
		const CoreVertex vTar = iterTar->second;
		auto pEdge = edge(vSrc, vTar, gComon);
		if(pEdge.second) continue;
		pEdge = add_edge(vSrc, vTar, {lib::rule::Membership::K}, gComon);
		const std::string &label = rSecond.getGraph()[e].membership == lib::rule::Membership::L
		                           ? get_string(rSecond.getDPORule()).getLeft()[e]
		                           : get_string(rSecond.getDPORule()).getRight()[e];
		pStringCommon.add(pEdge.first, label, label);
	}
	lib::rule::Rule rCommon(std::move(dpoCommon), rFirst.getLabelType());
	lib::rule::Rule rFirstCopy(lib::rule::LabelledRule(rFirst.getDPORule(), false), rFirst.getLabelType());
	lib::rule::Rule rSecondCopy(lib::rule::LabelledRule(rSecond.getDPORule(), false), rSecond.getLabelType());
	lib::rule::Rule rNewCopy(lib::rule::LabelledRule(rNew.getDPORule(), false), rNew.getLabelType());
	rFirstCopy.getDepictionData().copyCoords(rCommon.getDepictionData(), vFirstToCommon);
	rSecondCopy.getDepictionData().copyCoords(rCommon.getDepictionData(), vSecondToCommon);
	rNewCopy.getDepictionData().copyCoords(rCommon.getDepictionData(), vNewToCommon);

	const auto secondIdOffset = num_vertices(rFirst.getGraph());
	std::set<CoreVertex> matchVerticesInCommon;
	for(const CoreVertex v: asRange(vertices(rFirst.getGraph()))) {
		if(match.left.find(v) == match.left.end()) continue;
		const auto iter = vFirstToCommon.find(v);
		assert(iter != end(vFirstToCommon));
		matchVerticesInCommon.insert(iter->second);
	}
	const auto disallowCollapseFirst = [&matchVerticesInCommon, &vFirstToCommon](CoreVertex v) {
		const auto iter = vFirstToCommon.find(v);
		assert(iter != end(vFirstToCommon));
		return matchVerticesInCommon.find(iter->second) != end(matchVerticesInCommon);
	};
	const auto disallowCollapseSecond = [&matchVerticesInCommon, &vSecondToCommon](CoreVertex v) {
		const auto iter = vSecondToCommon.find(v);
		assert(iter != end(vSecondToCommon));
		return matchVerticesInCommon.find(iter->second) != end(matchVerticesInCommon);
	};
	const auto disallowCollapseNew = [&matchVerticesInCommon, &vNewToCommon](CoreVertex v) {
		const auto iter = vNewToCommon.find(v);
		assert(iter != end(vNewToCommon));
		return matchVerticesInCommon.find(iter->second) != end(matchVerticesInCommon);
	};
	const auto rawFilesFirst = rule::Write::tikz(rFirstCopy, 0, options, "L", "K", "R",
	                                              rule::Write::BaseArgs{visible, vColour, eColour},
	                                              disallowCollapseFirst);
	const auto rawFilesSecond = rule::Write::tikz(rSecondCopy, secondIdOffset, options, "L", "K", "R",
	                                               rule::Write::BaseArgs{visible, vColour, eColour},
	                                               disallowCollapseSecond);
	const auto rawFilesNew = rule::Write::tikz(rNewCopy, 0, options, "L", "K", "R",
	                                            rule::Write::BaseArgs{visible, vColour, eColour},
	                                            disallowCollapseNew);
	post::FileHandle s(IO::makeUniqueFilePrefix() + "rcMatch.tex");
	{
		s << "\\rcMatchFig";
		s << '{' << rawFilesFirst.first << '}'
		  << '{' << rFirst.getId() << '}';
		s << '{';
		bool first = true;
		for(const auto[vFirst, vSecond]: match.left) {
			const auto vIdFirst = get(boost::vertex_index_t(), rFirst.getGraph(), vFirst);
			auto vIdSecond = get(boost::vertex_index_t(), rSecond.getGraph(), vSecond);
			vIdSecond += num_vertices(rFirst.getGraph());
			if(first) first = false;
			else s << ", ";
			s << vIdFirst << "/" << vIdSecond;
		}
		s << '}';
		s << '{' << rawFilesSecond.first << '}'
		  << '{' << rSecond.getId() << '}';
		s << '{' << rawFilesNew.first << '}'
		  << '{' << rNew.getId() << '}';
		s << '\n';
	}
	post::FileHandle sAux(IO::makeUniqueFilePrefix() + "rcMatch_aux.tex");
	{
		sAux << "\\\\\n";
		sAux << "Files:\\\\\n \\texttt{" << IO::escapeForLatex(rawFilesFirst.first)
		     << "},\\\\\n \\texttt{" << IO::escapeForLatex(rawFilesSecond.first)
		     << "},\\\\\n \\texttt{" << IO::escapeForLatex(rawFilesNew.first)
		     << "},\\\\\n \\texttt{" << IO::escapeForLatex(s)
		     << "},\\\\\n \\texttt{" << IO::escapeForLatex(sAux)
		     << "}\\\\\n";
		sAux << "Match: \n";
		bool first = true;
		for(const auto[vFirst, vSecond]: match.left) {
			if(!first) sAux << ", ";
			sAux << "$" << vFirst << "\\rightarrow " << vSecond << "$\n";
			first = false;
		}
	}
	IO::post() << "summaryInput \"" << std::string(s) << "\"\n";
	IO::post() << "summaryInput \"" << std::string(sAux) << "\"\n";
	IO::post() << std::flush;
}

} // namespace mod::lib::RC::Write