#include "Write.hpp"

#include <mod/Config.hpp>
#include <mod/Post.hpp>
#include <mod/graph/Printer.hpp>
#include <mod/lib/Algorithm/Point.hpp>
#include <mod/lib/Chem/MoleculeUtil.hpp>
#include <mod/lib/Chem/OBabel.hpp>
#include <mod/lib/GraphMorphism/IO/WriteConstraints.hpp>
#include <mod/lib/IO/GraphWriteGeneric.hpp>
#include <mod/lib/Rules/IO/DepictionData.hpp>
#include <mod/lib/Rules/Real.hpp>
#include <mod/lib/Rules/Properties/String.hpp>
#include <mod/lib/Rules/Properties/Term.hpp>
#include <mod/lib/Stereo/IO/Write.hpp>
#include <mod/lib/Stereo/IO/WriteConfiguration.hpp>
#include <mod/lib/Term/WAM.hpp>
#include <mod/lib/Term/IO/Write.hpp>

#include <boost/lexical_cast.hpp>

namespace mod::lib::Rules::Write {
namespace {

// returns the filename _without_ extension

std::string getFilePrefix(const Real &r) {
	return IO::makeUniqueFilePrefix() + "r_" + boost::lexical_cast<std::string>(r.getId());
}

void gmlSide(const Real &r, std::ostream &s, Membership printMembership, bool withCoords) {
	if(withCoords) {
		const auto &depict = r.getDepictionData();
		if(!depict.getHasCoordinates()) MOD_ABORT;
	}
	const auto &lr = r.getDPORule();
	const auto &rDPO = lr.getRule();
	const auto &gCombined = rDPO.getCombinedGraph();
	const auto &pString = get_string(lr);

	for(const auto vCG: asRange(vertices(gCombined))) {
		const auto vMembership = gCombined[vCG].membership;
		if(printMembership == Membership::K) {
			if(vMembership != Membership::K) continue;
			if(pString.isChanged(vCG)) continue;
		} else {
			if(vMembership == Membership::K) {
				if(!pString.isChanged(vCG)) continue;
			} else {
				if(printMembership != vMembership) continue;
			}
		}
		s << "\t\tnode [ id " << get(boost::vertex_index_t(), gCombined, vCG) << " label \"";
		switch(printMembership) {
		case Membership::L:
		case Membership::K:
			s << pString.getLeft()[get_inverse(rDPO.getLtoCG(), getL(rDPO), gCombined, vCG)];
			break;
		case Membership::R:
			s << pString.getRight()[get_inverse(rDPO.getRtoCG(), getR(rDPO), gCombined, vCG)];
			break;
		}
		s << "\"";
		if(withCoords) {
			const auto &depict = r.getDepictionData();
			s << " vis2d [ x " << depict.getX(vCG, true) << " y " << depict.getY(vCG, true) << " ]";
		}
		s << " ]\n";
	}

	// we want the GML to be sorted, so iterate based on the vertices
	for(const auto vCGSrc: asRange(vertices(gCombined))) {
		for(const auto eCG: asRange(out_edges(vCGSrc, gCombined))) {
			const auto vCGTar = target(eCG, gCombined);
			const auto vSrcId = get(boost::vertex_index_t(), gCombined, vCGSrc);
			const auto vTarId = get(boost::vertex_index_t(), gCombined, vCGTar);
			if(vSrcId > vTarId) continue;
			const auto eMembership = gCombined[eCG].membership;
			if(printMembership == Membership::K) {
				if(eMembership != Membership::K) continue;
				if(pString.isChanged(eCG)) continue;
			} else {
				if(eMembership == Membership::K) {
					if(!pString.isChanged(eCG)) continue;
				} else {
					if(printMembership != eMembership) continue;
				}
			}
			s << "\t\tedge [ source " << vSrcId << " target " << vTarId << " label \"";
			switch(printMembership) {
			case Membership::L:
			case Membership::K:
				s << pString.getLeft()[get_inverse(rDPO.getLtoCG(), getL(rDPO), gCombined, eCG)];
				break;
			case Membership::R:
				s << pString.getRight()[get_inverse(rDPO.getRtoCG(), getR(rDPO), gCombined, eCG)];
				break;
			}
			s << "\" ]\n";
		}
	}
}

void printEdgeStyle(std::ostream &s, Membership eSide, int src, int tar) {
	s << "\t" << src << " -- " << tar << " [ ";
	switch(eSide) {
	case Membership::L:
		s << "style=dashed ";
		break;
	case Membership::R:
		s << "style=dotted ";
		break;
	default:
		break;
	}
}

} // namespace

void gml(const Real &r, bool withCoords, std::ostream &s) {
	s << "rule [\n";
	s << "\truleID \"" << r.getName() << "\"\n";
	if(r.getLabelType()) {
		s << "\tlabelType \"";
		switch(*r.getLabelType()) {
		case LabelType::String:
			s << "string";
			break;
		case LabelType::Term:
			s << "term";
			break;
		}
		s << "\"\n";
	}
	{
		std::stringstream str;
		gmlSide(r, str, Membership::L, withCoords);
		if(str.str().size() > 0) s << "\tleft [\n" << str.str() << "\t]\n";
	}
	{
		std::stringstream str;
		gmlSide(r, str, Membership::K, withCoords);
		if(str.str().size() > 0) s << "\tcontext [\n" << str.str() << "\t]\n";
	}
	{
		std::stringstream str;
		gmlSide(r, str, Membership::R, withCoords);
		if(str.str().size() > 0) s << "\tright [\n" << str.str() << "\t]\n";
	}
	for(const auto &c: get_match_constraints(get_labelled_left(r.getDPORule())))
		lib::GraphMorphism::Write::gmlConstraint(s, getL(r.getDPORule().getRule()), "\t", *c);
	s << "]";
}

std::string gml(const Real &r, bool withCoords) {
	post::FileHandle s(getFilePrefix(r) + ".gml");
	gml(r, withCoords, s);
	return s;
}

std::string dotCombined(const Real &r) {
	std::stringstream fileName;
	fileName << "r_" << r.getId() << "_combined.dot";
	post::FileHandle s(IO::makeUniqueFilePrefix() + fileName.str());
	std::string fileNoExt = s;
	fileNoExt.erase(fileNoExt.end() - 4, fileNoExt.end());

	const auto &rDPO = r.getDPORule().getRule();
	const auto &gCombined = rDPO.getCombinedGraph();
	const auto &pString = get_string(r.getDPORule());
	s << "graph G {\n";
	for(const auto vCG: asRange(vertices(gCombined))) {
		const auto membership = gCombined[vCG].membership;
		s << "\t" << get(boost::vertex_index_t(), gCombined, vCG) << " [ label=\"";
		switch(membership) {
		case Membership::L:
			s << pString.getLeft()[get_inverse(rDPO.getLtoCG(), getL(rDPO), gCombined, vCG)];
			break;
		case Membership::K:
			s << pString.getLeft()[get_inverse(rDPO.getLtoCG(), getL(rDPO), gCombined, vCG)] << " | "
			  << pString.getRight()[get_inverse(rDPO.getRtoCG(), getR(rDPO), gCombined, vCG)];
			break;
		case Membership::R:
			s << pString.getRight()[get_inverse(rDPO.getRtoCG(), getR(rDPO), gCombined, vCG)];
			break;
		}
		s << "\"";
		switch(membership) {
		case Membership::L:
			s << " style=dashed";
			break;
		case Membership::R:
			s << " style=dotted";
			break;
		default:
			break;
		}
		s << " ]\n";
	}

	// we want the output to be sorted, so iterate based on the vertices
	for(const auto vCGSrc: asRange(vertices(gCombined))) {
		for(const auto eCG: asRange(out_edges(vCGSrc, gCombined))) {
			const auto vCGTar = target(eCG, gCombined);
			const auto vSrcId = get(boost::vertex_index_t(), gCombined, vCGSrc);
			const auto vTarId = get(boost::vertex_index_t(), gCombined, vCGTar);
			if(vSrcId > vTarId) continue;
			const auto membership = gCombined[eCG].membership;
			std::string label;
			switch(membership) {
			case Membership::L:
				label = pString.getLeft()[get_inverse(rDPO.getLtoCG(), getL(rDPO), gCombined, eCG)];
				break;
			case Membership::K:
				label = pString.getLeft()[get_inverse(rDPO.getLtoCG(), getL(rDPO), gCombined, eCG)] + " | " +
				        pString.getRight()[get_inverse(rDPO.getRtoCG(), getR(rDPO), gCombined, eCG)];
				break;
			case Membership::R:
				label = pString.getRight()[get_inverse(rDPO.getRtoCG(), getR(rDPO), gCombined, eCG)];
				break;
			}
			switch(label[0]) {
				//		case '=': // fall through to make two edges
				//			//						assert(false);
				//			printEdgeStyle(s, membership, vSrcId, vTarId);
				//			s << "]\n";
				//		case '-': // print the rest of the label
				//			printEdgeStyle(s, membership, vSrcId, vTarId);
				//			s << "label=\"" << (label.c_str() + 1) << "\" ]\n";
				//			break;
			default:
				printEdgeStyle(s, membership, vSrcId, vTarId);
				s << "label=\"" << label << "\" ]\n";
				break;
			}
		}
	}
	s << "}\n";
	return fileNoExt;
}

std::string svgCombined(const Real &r) {
	std::string fileNoExt = dotCombined(r);
	IO::post() << "gv ruleCombined \"" << fileNoExt << "\" svg\n";
	return fileNoExt;
}

std::string pdfCombined(const Real &r) {
	std::string fileNoExt = svgCombined(r);
	IO::post() << "svgToPdf \"" << fileNoExt << "\"\n";
	return fileNoExt;
}

namespace {

struct DotCacheEntry {
	std::size_t id;
	std::string prefix;
public:
	friend bool operator<(const DotCacheEntry &a, const DotCacheEntry &b) {
		return std::tie(a.id, a.prefix) < std::tie(b.id, b.prefix);
	}
};

} // namespace

std::string dot(const Real &r, const Options &options) {
	static std::map<DotCacheEntry, std::string> cache;
	const auto iter = cache.find({r.getId(), options.graphvizPrefix});
	if(iter != end(cache)) return iter->second;

	std::string fileNoExt = getFilePrefix(r);
	post::FileHandle s(fileNoExt + ".dot");

	const auto &rDPO = r.getDPORule().getRule();
	const auto &gCombined = rDPO.getCombinedGraph();
	const auto &pString = get_string(r.getDPORule());

	s << "graph G {\n";
	if(!options.graphvizPrefix.empty())
		s << "\t" << options.graphvizPrefix << '\n';

	for(const auto vCG: asRange(vertices(gCombined))) {
		const auto vId = get(boost::vertex_index_t(), gCombined, vCG);
		s << vId << " [ label=\"";
		const auto membership = gCombined[vCG].membership;
		switch(membership) {
		case Membership::L:
			s << pString.getLeft()[get_inverse(rDPO.getLtoCG(), getL(rDPO), gCombined, vCG)];
			break;
		case Membership::K:
			s << pString.getLeft()[get_inverse(rDPO.getLtoCG(), getL(rDPO), gCombined, vCG)] << " | "
			  << pString.getRight()[get_inverse(rDPO.getRtoCG(), getR(rDPO), gCombined, vCG)];
			break;
		case Membership::R:
			s << pString.getRight()[get_inverse(rDPO.getRtoCG(), getR(rDPO), gCombined, vCG)];
			break;
		}
		s << "\" ];\n";
	}

	// we want the output to be sorted, so iterate based on the vertices
	for(const auto vCGSrc: asRange(vertices(gCombined))) {
		for(const auto eCG: asRange(out_edges(vCGSrc, gCombined))) {
			const auto vCGTar = target(eCG, gCombined);
			const auto vSrcId = get(boost::vertex_index_t(), gCombined, vCGSrc);
			const auto vTarId = get(boost::vertex_index_t(), gCombined, vCGTar);
			if(vSrcId > vTarId) continue;
			s << vSrcId << " -- " << vTarId << " [ label=\"";
			const auto membership = gCombined[eCG].membership;
			switch(membership) {
			case Membership::L:
				s << pString.getLeft()[get_inverse(rDPO.getLtoCG(), getL(rDPO), gCombined, eCG)];
				break;
			case Membership::K:
				s << pString.getLeft()[get_inverse(rDPO.getLtoCG(), getL(rDPO), gCombined, eCG)] << " | "
				  << pString.getRight()[get_inverse(rDPO.getRtoCG(), getR(rDPO), gCombined, eCG)];
				break;
			case Membership::R:
				s << pString.getRight()[get_inverse(rDPO.getRtoCG(), getR(rDPO), gCombined, eCG)];
				break;
			}
			s << "\" ];\n";
		}
	}
	s << "}\n";

	cache[{r.getId(), options.graphvizPrefix}] = fileNoExt;
	return fileNoExt;
}

namespace {

struct OpenBabelCoordsCacheEntry {
	std::size_t id;
	bool collapseHydrogens;
	int rotation;
	bool mirror;
	int idOffset;
public:
	friend bool operator<(const OpenBabelCoordsCacheEntry &a, const OpenBabelCoordsCacheEntry &b) {
		return std::tie(a.id, a.collapseHydrogens, a.rotation, a.mirror, a.idOffset)
		       < std::tie(b.id, b.collapseHydrogens, b.rotation, b.mirror, b.idOffset);
	}
};

bool disallowHydrogenCollapseByChangeThenCallback(const Real &r, const CombinedVertex vCG,
                                                  std::function<bool(CombinedVertex)> disallowCollapse) {
	if(getConfig().rule.collapseChangedHydrogens.get())
		return disallowCollapse(vCG);
	const auto &gCombined = r.getDPORule().getRule().getCombinedGraph();
	// if we are changed, then disallow
	if(gCombined[vCG].membership != Membership::K)
		return true;
	// and if any incident edges are changed, then disallow
	for(const auto eCG: asRange(out_edges(vCG, gCombined))) {
		if(gCombined[eCG].membership != Membership::K)
			return true;
	}
	return disallowCollapse(vCG);
}

} // namespace

std::string coords(const Real &r, int idOffset, const Options &options,
                   std::function<bool(CombinedVertex)> disallowCollapse_) {
	assert(idOffset >= 0);
	const auto &depict = r.getDepictionData();
	if(options.withGraphvizCoords || !depict.getHasCoordinates()) {
		if(idOffset != 0)
			throw FatalError("Blame the lazy programmer. Offset other than 0 not yet supported in dot coords.");

		// we map 1-to-1 a dot file to a coord file, so cache by the dot filename
		static std::map<std::string, std::string> cache;
		const auto fileNoExt = dot(r, options);
		const auto iter = cache.find(fileNoExt);
		if(iter != end(cache)) return iter->second;

		IO::post() << "coordsFromGV rule \"" << fileNoExt << "\" noOverlay\n";
		// the coord file is still for the tex coord file which is just then created in post
		std::string file = fileNoExt + "_coord";
		cache[fileNoExt] = file;
		return file;
	} else {
		static std::map<OpenBabelCoordsCacheEntry, std::string> cache;
		const auto iter = cache.find({r.getId(), options.collapseHydrogens, options.rotation, options.mirror, idOffset});
		if(iter != end(cache)) return iter->second;

		const auto &gCombined = r.getDPORule().getRule().getCombinedGraph();
		const std::string fileNoExt = [&]() {
			auto f = getFilePrefix(r);
			if(options.collapseHydrogens) f += "_mol";
			if(options.rotation != 0) f += "_r" + std::to_string(options.rotation);
			if(options.mirror) f += "_m" + std::to_string(options.mirror);
			if(idOffset != 0) f += "_id" + std::to_string(idOffset);
			f += "_coord";
			return f;
		}();
		post::FileHandle s(fileNoExt + ".tex");
		s << "% dummy\n";
		const bool useCollapsedCoords = [&]() {
			// if the options didn't request collapsing, don't use it
			if(!options.collapseHydrogens) return false;
			// if there is _any_ user-defined collapsing, then make all coordinates available
			for(const auto vCG: asRange(vertices(gCombined)))
				if(disallowCollapse_(vCG))
					return false;
			return true;
		}();

		for(const auto vCG: asRange(vertices(gCombined))) {
			const auto vId = get(boost::vertex_index_t(), gCombined, vCG);
			// if we are in the collapsed case and the depictor collapsed it, don't print
			if(useCollapsedCoords && depict.mayCollapse(vCG)) continue;
			const auto [x, y] = pointTransform(
					depict.getX(vCG, !useCollapsedCoords),
					depict.getY(vCG, !useCollapsedCoords),
					options.rotation, options.mirror);
			s << "\\coordinate[overlay] (\\modIdPrefix v-coord-" << (vId + idOffset) << ") at ("
			  << std::fixed << x << ", " << y << ") {};\n";
		}
		if(options.collapseHydrogens && !useCollapsedCoords) {
			// don't cache these as the user predicate influences it
		} else {
			cache[{r.getId(), options.collapseHydrogens, options.rotation, options.mirror, idOffset}] = fileNoExt;
		}
		return fileNoExt;
	}
}

namespace {

using SideGraph = lib::DPO::CombinedRule::SideGraphType;
using SideVertex = lib::DPO::CombinedRule::SideVertex;
using SideEdge = lib::DPO::CombinedRule::SideEdge;
using MorphismType = lib::DPO::CombinedRule::MorphismType;
using ToCombinedMorphismSide = lib::DPO::CombinedRule::ToCombinedMorphismSide;
using KVertex = lib::DPO::CombinedRule::KVertex;
using KEdge = lib::DPO::CombinedRule::KEdge;
using DepictSide = DepictionData::Side;
using LabelledSide = LabelledRule::Side;

struct AdvOptionsSide {
	AdvOptionsSide(const Real &r, int idOffset, const BaseArgs &args,
	               std::function<bool(CombinedVertex)> disallowHydrogenCollapse,
	               std::string changeColour,
	               const SideGraph &g, const MorphismType &mToSide, const ToCombinedMorphismSide &mToCG,
	               DepictSide depict, LabelledSide lg)
			: idOffset(idOffset), changeColour(std::move(changeColour)), r(r), rDPO(r.getDPORule().getRule()),
			  args(args), disallowHydrogenCollapse_(disallowHydrogenCollapse),
			  g(g), mToCG(mToCG), depict(depict), lg(lg) {}
public:
	std::string getColour(SideVertex vS) const {
		const auto vCG = get(mToCG, g, rDPO.getCombinedGraph(), vS);
		const bool isChanged = r.getDPORule().getRule().getCombinedGraph()[vCG].membership != Membership::K
		                       || get_string(r.getDPORule()).isChanged(vCG);
		if(isChanged) return changeColour;
		else return args.vColour(vCG);
	}

	std::string getColour(SideEdge eS) const {
		const auto eCG = get(mToCG, g, rDPO.getCombinedGraph(), eS);
		const bool isChanged = r.getDPORule().getRule().getCombinedGraph()[eCG].membership != Membership::K
		                       || get_string(r.getDPORule()).isChanged(eCG);
		if(isChanged) return changeColour;
		else return args.eColour(eCG);
	}

	bool isVisible(SideVertex vS) const {
		const auto v = get(mToCG, g, rDPO.getCombinedGraph(), vS);
		return args.visible(v);
	}

	std::string getShownId(SideVertex vS) const {
		const auto v = get(mToCG, g, rDPO.getCombinedGraph(), vS);
		return std::to_string(get(boost::vertex_index_t(), rDPO.getCombinedGraph(), v));
	}

	bool overwriteWithIndex(SideVertex) const {
		return false;
	}

	lib::IO::Graph::Write::EdgeFake3DType getEdgeFake3DType(SideEdge eS, bool withHydrogen) const {
		return depict.getEdgeFake3DType(eS, withHydrogen);
	}

	std::string getOpts(SideVertex vS) const {
		return std::string();
	}
private:
	template<typename F>
	std::string getStereoStringVertex(SideVertex vS, const F f) const {
//		assert(false); // TODO: map vS
		const auto &conf = *get_stereo(lg)[vS];
		const auto getNeighbourId = [&](const lib::Stereo::EmbeddingEdge &emb) {
			return get(boost::vertex_index_t(), g, target(emb.getEdge(vS, g), g));
		};
		std::string res = f(conf, getNeighbourId);
		const auto v = get(mToCG, g, rDPO.getCombinedGraph(), vS);
		if(!get_stereo(r.getDPORule()).inContext(v)) {
			res = "{\\color{" + changeColour + "}" + res + "}";
		}
		return res;
	}
public:
	std::string getRawStereoString(SideVertex vS) const {
//		assert(false); // TODO: anything to map?
		return getStereoStringVertex(vS, [&](const auto &conf, auto getNId) {
			return conf.asRawString(getNId);
		});
	}

	std::string getPrettyStereoString(SideVertex vS) const {
//		assert(false); // TODO: anything to map?
		return getStereoStringVertex(vS, [&](const auto &conf, auto getNId) {
			return conf.asPrettyString(getNId);
		});
	}

	std::string getStereoString(SideEdge eS) const {
//		assert(false); // TODO: map eS
		const auto cat = get_stereo(lg)[eS];
		std::string res = boost::lexical_cast<std::string>(cat);
		const auto e = get(mToCG, g, rDPO.getCombinedGraph(), eS);
		if(!get_stereo(r.getDPORule()).inContext(e)) {
			res = "{\\color{" + changeColour + "}" + res + "}";
		}
		return res;
	}

	std::string getEdgeAnnotation(SideEdge) const {
		return {};
	}

	bool disallowHydrogenCollapse(SideVertex vS) const {
		const auto v = get(mToCG, g, rDPO.getCombinedGraph(), vS);
		return disallowHydrogenCollapseByChangeThenCallback(r, v, disallowHydrogenCollapse_);
	}
public:
	auto getOutputId(SideVertex vS) const {
		const auto v = get(mToCG, g, rDPO.getCombinedGraph(), vS);
		return idOffset + get(boost::vertex_index_t(), rDPO.getCombinedGraph(), v);
	}
private:
	const int idOffset;
	const std::string changeColour;
	const Real &r;
	const lib::DPO::CombinedRule &rDPO;
	const BaseArgs &args;
	std::function<bool(CombinedVertex)> disallowHydrogenCollapse_;
	const SideGraph &g;
	const ToCombinedMorphismSide &mToCG;
	const DepictSide depict;
	const LabelledSide lg;
};

struct AdvOptionsK {
	AdvOptionsK(const Real &r, int idOffset, const BaseArgs &args,
	            std::function<bool(CombinedVertex)> disallowHydrogenCollapse,
	            std::string changeColour)
			: idOffset(idOffset), changeColour(std::move(changeColour)), r(r), rDPO(r.getDPORule().getRule()),
			  args(args), disallowHydrogenCollapse_(disallowHydrogenCollapse) {}
public:
	std::string getColour(KVertex vK) const {
		const auto v = get(rDPO.getKtoCG(), getK(rDPO), rDPO.getCombinedGraph(), vK);
		const bool isChanged = get_string(r.getDPORule()).isChanged(v);
		if(isChanged) return changeColour;
		else return args.vColour(v);
	}

	std::string getColour(KEdge eK) const {
		const auto eCG = get(rDPO.getKtoCG(), getK(rDPO), rDPO.getCombinedGraph(), eK);
		assert(r.getDPORule().getRule().getCombinedGraph()[eCG].membership == Membership::K);
		const bool isChanged = get_string(r.getDPORule()).isChanged(eCG);
		if(isChanged) return changeColour;
		else return args.eColour(eCG);
	}

	bool isVisible(KVertex vK) const {
		const auto v = get(rDPO.getKtoCG(), getK(rDPO), rDPO.getCombinedGraph(), vK);
		return args.visible(v);
	}

	std::string getShownId(KVertex vK) const {
		const auto v = get(rDPO.getKtoCG(), getK(rDPO), rDPO.getCombinedGraph(), vK);
		return std::to_string(get(boost::vertex_index_t(), rDPO.getCombinedGraph(), v));
	}

	bool overwriteWithIndex(KVertex) const {
		return false;
	}

	lib::IO::Graph::Write::EdgeFake3DType getEdgeFake3DType(KEdge eK, bool withHydrogen) const {
		const auto e = get(rDPO.getKtoCG(), getK(rDPO), rDPO.getCombinedGraph(), eK);
		return r.getDepictionData().getContext().getEdgeFake3DType(e, withHydrogen);
	}

	std::string getOpts(KVertex) const {
		return {};
	}
private:
	template<typename F>
	std::string getStereoStringVertex(const KVertex vK, const F f) const {
		const auto v = get(rDPO.getKtoCG(), getK(rDPO), rDPO.getCombinedGraph(), vK);
		const auto &lr = r.getDPORule();
		if(get_stereo(lr).inContext(v)) {
			const auto &lgLeft = get_labelled_left(r.getDPORule());
			const auto vL = get(getMorL(rDPO), getK(rDPO), getL(rDPO), vK);
			const auto &conf = *get_stereo(lgLeft)[vL];
			const auto getNeighbourId = [&](const lib::Stereo::EmbeddingEdge &emb) {
				const auto &gLeft = get_graph(lgLeft);
				return get(boost::vertex_index_t(), gLeft, target(emb.getEdge(vL, gLeft), gLeft));
			};
			return f(conf, getNeighbourId);
		} else return {};
	}
public:
	std::string getRawStereoString(KVertex vK) const {
		return getStereoStringVertex(vK, [&](const auto &conf, auto getNId) {
			return conf.asRawString(getNId);
		});
	}

	std::string getPrettyStereoString(KVertex vK) const {
		return getStereoStringVertex(vK, [&](const auto &conf, auto getNId) {
			return conf.asPrettyString(getNId);
		});
	}

	std::string getStereoString(KEdge eK) const {
		const auto e = get(rDPO.getKtoCG(), getK(rDPO), rDPO.getCombinedGraph(), eK);
		const auto &lr = r.getDPORule();
		if(get_stereo(lr).inContext(e)) {
			const auto &lgLeft = get_labelled_left(r.getDPORule());
			const auto eL = get(getMorL(rDPO), getK(rDPO), getL(rDPO), eK);
			const auto cat = get_stereo(lgLeft)[eL];
			return boost::lexical_cast<std::string>(cat);
		} else return {};
	}

	std::string getEdgeAnnotation(KEdge) const {
		return {};
	}

	bool disallowHydrogenCollapse(KVertex vK) const {
		const auto v = get(rDPO.getKtoCG(), getK(rDPO), rDPO.getCombinedGraph(), vK);
		return disallowHydrogenCollapseByChangeThenCallback(r, v, disallowHydrogenCollapse_);
	}
public:
	int getOutputId(KVertex vK) const {
		const auto v = get(rDPO.getKtoCG(), getK(rDPO), rDPO.getCombinedGraph(), vK);
		return idOffset + get(boost::vertex_index_t(), rDPO.getCombinedGraph(), v);
	}
private:
	const int idOffset;
	const std::string changeColour;
	const Real &r;
	const lib::DPO::CombinedRule &rDPO;
	const BaseArgs &args;
	std::function<bool(CombinedVertex)> disallowHydrogenCollapse_;
};

} // namespace

std::pair<std::string, std::string>
tikz(const std::string &fileCoordsNoExt, const Real &r, unsigned int idOffset, const Options &options,
     const std::string &suffixL, const std::string &suffixK, const std::string &suffixR, const BaseArgs &args,
     std::function<bool(CombinedVertex)> disallowCollapse) {
	std::string strOptions = options.getStringEncoding();
	std::string fileNoExt = IO::makeUniqueFilePrefix() + "r_" + boost::lexical_cast<std::string>(r.getId());
	fileNoExt += "_" + strOptions;

	const auto &rDPO = r.getDPORule().getRule();

	std::string fileCoords = fileCoordsNoExt + ".tex";
	{ // left
		post::FileHandle s(fileNoExt + "_" + suffixL + ".tex");
		const auto &g = getL(rDPO);
		const auto &depict = r.getDepictionData().getLeft();
		const auto adv = AdvOptionsSide(r, idOffset, args, disallowCollapse,
		                                getConfig().rule.changeColourL.get(),
		                                g, getMorL(rDPO), rDPO.getLtoCG(),
		                                r.getDepictionData().getLeft(),
		                                get_labelled_left(r.getDPORule()));
		IO::Graph::Write::tikz(s, options, g, depict, fileCoords, adv, jla_boost::Nop<>(), "");
	}
	{ // context
		post::FileHandle s(fileNoExt + "_" + suffixK + ".tex");
		const auto &g = getK(rDPO);
		const auto &depict = r.getDepictionData().getContext();

		struct EdgeVisible {
			EdgeVisible() = default;

			EdgeVisible(const Real &r) : r(&r) {}

			bool operator()(const CombinedEdge e) const {
				if(getConfig().rule.printChangedEdgesInContext.get()) return true;
				return !get_string(r->getDPORule()).isChanged(e);
			}
		private:
			const Real *r = nullptr;
		};
		boost::filtered_graph<lib::DPO::CombinedRule::KGraphType, EdgeVisible> gFiltered(g, EdgeVisible(r));
		const auto adv = AdvOptionsK(r, idOffset, args, disallowCollapse,
		                             getConfig().rule.changeColourK.get());
		IO::Graph::Write::tikz(s, options, gFiltered, depict, fileCoords, adv, jla_boost::Nop<>(), "");
	}
	{ // right
		post::FileHandle s(fileNoExt + "_" + suffixR + ".tex");
		const auto &g = getR(rDPO);
		const auto &depict = r.getDepictionData().getRight();
		const auto adv = AdvOptionsSide(r, idOffset, args, disallowCollapse,
		                                getConfig().rule.changeColourR.get(),
		                                g, getMorR(rDPO), rDPO.getRtoCG(),
		                                r.getDepictionData().getRight(),
		                                get_labelled_right(r.getDPORule()));
		IO::Graph::Write::tikz(s, options, g, depict, fileCoords, adv, jla_boost::Nop<>(), "");
	}
	return std::make_pair(fileNoExt, fileCoordsNoExt);
}

std::pair<std::string, std::string> tikz(const Real &r, unsigned int idOffset, const Options &options,
                                         const std::string &suffixL, const std::string &suffixK,
                                         const std::string &suffixR, const BaseArgs &args,
                                         std::function<bool(CombinedVertex)> disallowCollapse) {
	std::string fileCoordsNoExt = coords(r, idOffset, options, disallowCollapse);
	return tikz(fileCoordsNoExt, r, idOffset, options, suffixL, suffixK, suffixR, args, disallowCollapse);
}

std::string pdf(const Real &r, const Options &options,
                const std::string &suffixL, const std::string &suffixK, const std::string &suffixR,
                const BaseArgs &args) {
	std::string fileNoExt, fileCoordsNoExt;
	const unsigned int idOffset = 0;
	std::tie(fileNoExt, fileCoordsNoExt) = tikz(r, idOffset, options, suffixL, suffixK, suffixR, args,
	                                            jla_boost::AlwaysFalse());
	IO::post() << "compileTikz \"" << fileNoExt << "_" << suffixL << "\" \"" << fileCoordsNoExt << "\"\n";
	IO::post() << "compileTikz \"" << fileNoExt << "_" << suffixK << "\" \"" << fileCoordsNoExt << "\"\n";
	IO::post() << "compileTikz \"" << fileNoExt << "_" << suffixR << "\" \"" << fileCoordsNoExt << "\"\n";
	IO::post().flush();
	return fileNoExt;
}

std::pair<std::string, std::string>
tikzTransitionState(const std::string &fileCoordsNoExt, const Real &r, unsigned int idOffset,
                    const Options &options,
                    const std::string &suffix, const BaseArgs &args) {
	MOD_ABORT;
}

std::pair<std::string, std::string>
tikzTransitionState(const Real &r, unsigned int idOffset, const Options &options,
                    const std::string &suffix, const BaseArgs &args) {
	MOD_ABORT;
}

std::string pdfTransitionState(const Real &r, const Options &options,
                               const std::string &suffix, const BaseArgs &args) {
	std::string fileNoExt, fileCoordsNoExt;
	const unsigned int idOffset = 0;
	std::tie(fileNoExt, fileCoordsNoExt) = tikzTransitionState(r, idOffset, options, suffix, args);
	IO::post() << "compileTikz \"" << fileNoExt << "_" << suffix << "\" \"" << fileCoordsNoExt << "\"\n";
	return fileNoExt;
}

std::string pdfCombined(const Real &r, const Options &options) {
	MOD_ABORT;
}

std::pair<std::string, std::string> summary(const Real &r, bool printCombined) {
	graph::Printer first;
	graph::Printer second;
	second.setReactionDefault();
	return summary(r, first.getOptions(), second.getOptions(), printCombined);
}

std::pair<std::string, std::string>
summary(const Real &r, const Options &first, const Options &second, bool printCombined) {
	auto visible = jla_boost::AlwaysTrue();
	auto vColour = jla_boost::Nop<std::string>();
	auto eColour = jla_boost::Nop<std::string>();
	const BaseArgs args{visible, vColour, eColour};
	std::string graphLike = pdf(r, first, "L", "K", "R", args);
	std::string molLike = first == second ? "" : pdf(r, second, "L", "K", "R", args);
	std::string combined = printCombined
	                       ? pdfCombined(r /*, Options().EdgesAsBonds().RaiseCharges()*/)
	                       : "";
	std::string constraints =
			IO::makeUniqueFilePrefix() + "r_" + boost::lexical_cast<std::string>(r.getId()) + "_constraints.tex";
	{
		post::FileHandle s(constraints);
		for(const auto &c: get_match_constraints(get_labelled_left(r.getDPORule()))) {
			lib::GraphMorphism::Write::texConstraint(s, getL(r.getDPORule().getRule()), *c);
			s << "\n";
		}
	}
	IO::post() << "summaryRule \"" << r.getName() << "\" \"" << graphLike << "\" \"" << molLike << "\" \"" << combined
	           << "\" \"" << constraints << "\"\n";
	if(molLike.empty())
		return std::pair(graphLike, graphLike);
	else
		return std::pair(graphLike, molLike);
}

void termState(const Real &r) {
	using namespace lib::Term;
	using CombinedVertex = lib::DPO::CombinedRule::CombinedVertex;
	using CombinedEdge = lib::DPO::CombinedRule::CombinedEdge;
	IO::post() << "summarySubsection \"Term State for " << r.getName() << "\"\n";
	post::FileHandle s(IO::makeUniqueFilePrefix() + "termState.tex");

	const auto &rDPO = r.getDPORule().getRule();
	const auto &gCombined = rDPO.getCombinedGraph();
	const auto &pTerm = get_term(r.getDPORule());

	s << "\\begin{verbatim}\n";
	if(isValid(pTerm)) {
		std::unordered_map<Address, std::set<std::pair<CombinedVertex, Membership>>> addrToVertex;
		std::unordered_map<Address, std::set<std::pair<CombinedEdge, Membership>>> addrToEdge;
		std::unordered_map<Address, std::set<std::string>> addrToConstraintInfo;
		const auto insertVertex = [&addrToVertex](std::size_t addr, CombinedVertex v, Membership membership) {
			Address a{AddressType::Heap, addr};
			addrToVertex[a].insert({v, membership});
		};
		const auto insertEdge = [&addrToEdge](std::size_t addr, CombinedEdge e, Membership membership) {
			Address a{AddressType::Heap, addr};
			addrToEdge[a].insert({e, membership});
		};
		for(const auto vCG: asRange(vertices(gCombined))) {
			switch(gCombined[vCG].membership) {
			case Membership::L:
				insertVertex(pTerm.getLeft()[get_inverse(rDPO.getLtoCG(), getL(rDPO), gCombined, vCG)], vCG, Membership::L);
				break;
			case Membership::K:
				insertVertex(pTerm.getLeft()[get_inverse(rDPO.getLtoCG(), getL(rDPO), gCombined, vCG)], vCG, Membership::L);
				insertVertex(pTerm.getRight()[get_inverse(rDPO.getRtoCG(), getR(rDPO), gCombined, vCG)], vCG,
				             Membership::R);
				break;
			case Membership::R:
				insertVertex(pTerm.getRight()[get_inverse(rDPO.getRtoCG(), getR(rDPO), gCombined, vCG)], vCG,
				             Membership::R);
				break;
			}
		}
		for(const auto eCG: asRange(edges(r.getDPORule().getRule().getCombinedGraph()))) {
			switch(gCombined[eCG].membership) {
			case Membership::L:
				insertEdge(pTerm.getLeft()[get_inverse(rDPO.getLtoCG(), getL(rDPO), gCombined, eCG)], eCG, Membership::L);
				break;
			case Membership::K:
				insertEdge(pTerm.getLeft()[get_inverse(rDPO.getLtoCG(), getL(rDPO), gCombined, eCG)], eCG, Membership::L);
				insertEdge(pTerm.getRight()[get_inverse(rDPO.getRtoCG(), getR(rDPO), gCombined, eCG)], eCG, Membership::R);
				break;
			case Membership::R:
				insertEdge(pTerm.getRight()[get_inverse(rDPO.getRtoCG(), getR(rDPO), gCombined, eCG)], eCG, Membership::R);
				break;
			}
		}

		using SideGraphType = lib::DPO::CombinedRule::SideGraphType;

		struct Visitor : lib::GraphMorphism::Constraints::AllVisitor<SideGraphType> {
			Visitor(std::unordered_map<Address, std::set<std::string>> &addrMap,
			        const lib::DPO::CombinedRule::CombinedGraphType &gCombined)
					: addrMap(addrMap), gCombined(gCombined) {}

			virtual void operator()(const lib::GraphMorphism::Constraints::VertexAdjacency<SideGraphType> &c) override {
				const auto vStr = boost::lexical_cast<std::string>(get(boost::vertex_index_t(), gCombined, c.vConstrained));
				for(const auto a: c.vertexTerms) {
					Address addr{AddressType::Heap, a};
					std::string msg = "VertexAdj(" + vStr + ", " + side + ", V)";
					addrMap[addr].insert(std::move(msg));
				}
				for(const auto a: c.edgeTerms) {
					Address addr{AddressType::Heap, a};
					std::string msg = "VertexAdj(" + vStr + ", " + side + ", E)";
					addrMap[addr].insert(std::move(msg));
				}
			}

			virtual void operator()(const lib::GraphMorphism::Constraints::ShortestPath<SideGraphType> &c) override {}
		public:
			std::unordered_map<Address, std::set<std::string>> &addrMap;
			const lib::DPO::CombinedRule::CombinedGraphType &gCombined;
			std::string side;
		};
		Visitor vis(addrToConstraintInfo, gCombined);
		vis.side = "L";
		for(const auto &c: get_match_constraints(get_labelled_left(r.getDPORule())))
			c->accept(vis);
		vis.side = "R";
		for(const auto &c: get_match_constraints(get_labelled_right(r.getDPORule())))
			c->accept(vis);

		Term::Write::wam(getMachine(pTerm), lib::Term::getStrings(),
		                 IO::Logger(s), [&](Address addr, std::ostream &s) {
					s << "        ";
					bool first = true;
					for(auto vm: addrToVertex[addr]) {
						if(!first) s << ", ";
						first = false;
						s << "v(" << get(boost::vertex_index_t(), gCombined, vm.first) << ", ";
						switch(vm.second) {
						case Membership::L:
							s << "L";
							break;
						case Membership::R:
							s << "R";
							break;
						case Membership::K:
							s << "K";
							break;
						}
						s << ")";
					}
					for(auto em: addrToEdge[addr]) {
						if(!first) s << ", ";
						first = false;
						s << "e("
						  << get(boost::vertex_index_t(), gCombined, source(em.first, gCombined))
						  << ", "
						  << get(boost::vertex_index_t(), gCombined, target(em.first, gCombined))
						  << ", ";
						switch(em.second) {
						case Membership::L:
							s << "L";
							break;
						case Membership::R:
							s << "R";
							break;
						case Membership::K:
							s << "K";
							break;
						}
						s << ")";
					}
					for(auto &msg: addrToConstraintInfo[addr]) {
						if(!first) s << ", ";
						first = false;
						s << msg;
					}
				});
	} else {
		std::string msg = "Parsing failed for rule '" + r.getName() + "'. " + pTerm.getParsingError();
		throw TermParsingError(std::move(msg));
	}
	s << "\\end{verbatim}\n";
	IO::post() << "summaryInput \"" << std::string(s) << "\"\n";
}

std::string stereoSummary(const Real &r, lib::DPO::CombinedRule::CombinedVertex vcg, Membership m,
                          const IO::Graph::Write::Options &options) {
	assert(m != Membership::K);
	const auto &lr = r.getDPORule();
	const auto &rDPO = lr.getRule();
	const auto &gCombined = rDPO.getCombinedGraph();
	if(m == Membership::L) assert(gCombined[vcg].membership != Membership::R);
	if(m == Membership::R) assert(gCombined[vcg].membership != Membership::L);
	const std::string side = m == Membership::L ? "L" : "R";
	std::string name = "r_" + std::to_string(r.getId()) + "_" + side + "_stereo_" +
	                   std::to_string(get(boost::vertex_index_t(), gCombined, vcg));
	IO::post() << "summarySubsection \"Stereo, r " << r.getId() << ", v " << get(boost::vertex_index_t(), gCombined, vcg)
	           << " " << side << "\"\n";
	const auto handler = [&](const auto &lgSide, const auto &mSideToCG, const auto &depict) {
		const auto &gSide = get_graph(lgSide);
		const auto vSide = get_inverse(mSideToCG, gSide, gCombined, vcg);
		return lib::Stereo::Write::pdf(gSide, vSide, *get_stereo(lgSide)[vSide], name, depict, options,
		                               [](const auto &gSide, const auto vSide) {
			                               return get(boost::vertex_index_t(), gSide, vSide);
		                               });
	};
	std::string f = m == Membership::L
	                ? handler(get_labelled_left(lr), rDPO.getLtoCG(), r.getDepictionData().getLeft())
	                : handler(get_labelled_right(lr), rDPO.getRtoCG(), r.getDepictionData().getRight());
	post::FileHandle s(IO::makeUniqueFilePrefix() + "stereo.tex");
	s << "\\begin{center}\n";
	s << "\\includegraphics{" << f << "}\\\\\n";
	s << "File: \\texttt{" << IO::escapeForLatex(f) << "}\n";
	s << "\\end{center}\n";
	IO::post() << "summaryInput \"" << std::string(s) << "\"\n";
	return f;
}

} // namespace mod::lib::Rules::Write