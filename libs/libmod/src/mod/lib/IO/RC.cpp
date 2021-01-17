#include "RC.hpp"

#include <mod/Config.hpp>
#include <mod/Post.hpp>
#include <mod/lib/IO/IO.hpp>
#include <mod/lib/IO/Rule.hpp>
#include <mod/lib/RC/Evaluator.hpp>
#include <mod/lib/Rules/Real.hpp>
#include <mod/lib/Rules/Properties/Depiction.hpp>
#include <mod/lib/Rules/Properties/String.hpp>

namespace mod::lib::IO::RC::Write {

std::string dot(const lib::RC::Evaluator &rc) {
	typedef lib::RC::Evaluator::Vertex Vertex;
	typedef lib::RC::Evaluator::Edge Edge;
	const lib::RC::Evaluator::GraphType &rcg = rc.getGraph();
	post::FileHandle s(getUniqueFilePrefix() + "rc.dot");
	std::string fileNoExt = s;
	fileNoExt.erase(end(fileNoExt) - 4, end(fileNoExt));
	s << "digraph g {" << std::endl;

	for(Vertex v : asRange(vertices(rcg))) {
		s << "\t" << get(boost::vertex_index_t(), rcg, v) << " [";
		switch(rcg[v].kind) {
		case lib::RC::Evaluator::VertexKind::Rule:
			s << " label=\"" << rcg[v].rule->getName() << "\"";
			break;
		case lib::RC::Evaluator::VertexKind::Composition:
			s << " shape=point";
			break;
		}
		s << " ];" << std::endl;
	}

	for(Edge e : asRange(edges(rcg))) {
		s << "\t" << get(boost::vertex_index_t(), rcg, source(e, rcg))
		  << " -> " << get(boost::vertex_index_t(), rcg, target(e, rcg)) << " [";
		switch(rcg[e].kind) {
		case lib::RC::Evaluator::EdgeKind::First:
			s << " label=1";
			break;
		case lib::RC::Evaluator::EdgeKind::Second:
			s << " label=2";
			break;
		case lib::RC::Evaluator::EdgeKind::Result:
			break;
		}
		s << " ];" << std::endl;
	}
	s << "}" << std::endl;
	return fileNoExt;
}

std::string svg(const lib::RC::Evaluator &rc) {
	std::string fileNoExt = dot(rc);
	IO::post() << "gv rc \"" << fileNoExt << "\" svg" << std::endl;
	return fileNoExt;
}

std::string pdf(const lib::RC::Evaluator &rc) {
	std::string fileNoExt = svg(rc);
	IO::post() << "svgToPdf \"" << fileNoExt << "\"" << std::endl;
	return fileNoExt;
}

void test(const lib::Rules::Real &rFirst, const lib::Rules::Real &rSecond, const CoreCoreMap &match,
          const lib::Rules::Real &rNew) {
	if(getConfig().rc.printMatchesOnlyHaxChem.get()) {
		const auto &lg = get_labelled_left(rNew.getDPORule());
		const auto &g = get_graph(lg);
		const auto &mol = get_molecule(lg);
		for(const auto v : asRange(vertices(g))) {
			const auto ad = mol[v];
			if(ad.getRadical()) continue;
			if(ad.getCharge() != 0) continue;
			if(ad.getIsotope() != Isotope()) continue;
			int valence = 0;
			for(const auto e : asRange(out_edges(v, g))) {
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
	using CoreVertex = lib::Rules::Vertex;
	using CoreEdge = lib::Rules::Edge;
	IO::Rules::Write::Options options;
	options.CollapseHydrogens(true);
	options.EdgesAsBonds(true);
	if(getConfig().rc.matchesWithIndex.get())
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
	lib::Rules::LabelledRule dpoCommon(rFirst.getDPORule(), false);
	lib::Rules::GraphType &gComon = get_graph(dpoCommon);
	lib::Rules::PropStringCore &pStringCommon = *dpoCommon.pString;
	for(const CoreVertex v : asRange(vertices(rFirst.getGraph())))
		vFirstToCommon[v] = v;
	// TODO: this will completely break if vertices are deleted in the composed rule
	for(const CoreVertex v : asRange(vertices(rNew.getGraph())))
		vNewToCommon[v] = v;
	// copy rSecond vertices
	for(const CoreVertex v : asRange(vertices(rSecond.getGraph()))) {
		const auto rightIter = match.right.find(v);
		if(rightIter != match.right.end()) {
			vSecondToCommon[v] = rightIter->second;
		} else {
			const CoreVertex vCommon = add_vertex(gComon);
			vSecondToCommon[v] = vCommon;
			gComon[vCommon].membership = lib::Rules::Membership::Context;
			const std::string &label = rSecond.getGraph()[v].membership == lib::Rules::Membership::Left
			                           ? rSecond.getStringState().getLeft()[v]
			                           : rSecond.getStringState().getRight()[v];
			pStringCommon.add(vCommon, label, label);
		}
	}
	// copy rSecond edges
	for(const CoreEdge e : asRange(edges(rSecond.getGraph()))) {
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
		pEdge = add_edge(vSrc, vTar, gComon);
		gComon[pEdge.first].membership = lib::Rules::Membership::Context;
		const std::string &label = rSecond.getGraph()[e].membership == lib::Rules::Membership::Left
		                           ? rSecond.getStringState().getLeft()[e]
		                           : rSecond.getStringState().getRight()[e];
		pStringCommon.add(pEdge.first, label, label);
	}
	lib::Rules::Real rCommon(std::move(dpoCommon), rFirst.getLabelType());
	lib::Rules::Real rFirstCopy(lib::Rules::LabelledRule(rFirst.getDPORule(), false), rFirst.getLabelType());
	lib::Rules::Real rSecondCopy(lib::Rules::LabelledRule(rSecond.getDPORule(), false), rSecond.getLabelType());
	lib::Rules::Real rNewCopy(lib::Rules::LabelledRule(rNew.getDPORule(), false), rNew.getLabelType());
	rFirstCopy.getDepictionData().copyCoords(rCommon.getDepictionData(), vFirstToCommon);
	rSecondCopy.getDepictionData().copyCoords(rCommon.getDepictionData(), vSecondToCommon);
	rNewCopy.getDepictionData().copyCoords(rCommon.getDepictionData(), vNewToCommon);

	const auto secondIdOffset = num_vertices(rFirst.getGraph());
	std::set<CoreVertex> matchVerticesInCommon;
	for(const CoreVertex v : asRange(vertices(rFirst.getGraph()))) {
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
	const auto rawFilesFirst = IO::Rules::Write::tikz(rFirstCopy, 0, options, "L", "K", "R",
	                                                  IO::Rules::Write::BaseArgs{visible, vColour, eColour},
	                                                  disallowCollapseFirst);
	const auto rawFilesSecond = IO::Rules::Write::tikz(rSecondCopy, secondIdOffset, options, "L", "K", "R",
	                                                   IO::Rules::Write::BaseArgs{visible, vColour, eColour},
	                                                   disallowCollapseSecond);
	const auto rawFilesNew = IO::Rules::Write::tikz(rNewCopy, 0, options, "L", "K", "R",
	                                                IO::Rules::Write::BaseArgs{visible, vColour, eColour},
	                                                disallowCollapseNew);
	post::FileHandle s(getUniqueFilePrefix() + "rcMatch.tex");
	{
		s << "\\rcMatchFig";
		s << '{' << rawFilesFirst.first << '}'
		  << '{' << rFirst.getId() << '}';
		s << '{';
		bool first = true;
		for(const auto[vFirst, vSecond] : match.left) {
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
	post::FileHandle sAux(getUniqueFilePrefix() + "rcMatch_aux.tex");
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
		for(const auto[vFirst, vSecond] : match.left) {
			if(!first) sAux << ", ";
			sAux << "$" << vFirst << "\\rightarrow " << vSecond << "$\n";
			first = false;
		}
	}
	IO::post() << "summaryInput \"" << std::string(s) << "\"\n";
	IO::post() << "summaryInput \"" << std::string(sAux) << "\"\n";
	IO::post() << std::flush;
}

} // namespace mod::lib::IO::RC::Write