#include "Write.hpp"

#include <mod/Function.hpp>
#include <mod/Post.hpp>
#include <mod/lib/Chem/MoleculeUtil.hpp>
#include <mod/lib/Graph/Single.hpp>
#include <mod/lib/Graph/IO/DepictionData.hpp>
#include <mod/lib/Graph/Properties/String.hpp>
#include <mod/lib/Graph/Properties/Term.hpp>
#include <mod/lib/IO/GraphWriteGeneric.hpp>
#include <mod/lib/IO/DFS.hpp>
#include <mod/lib/IO/IO.hpp>
#include <mod/lib/Stereo/IO/WriteConfiguration.hpp>
#include <mod/lib/Term/WAM.hpp>
#include <mod/lib/Term/IO/Write.hpp>

#include <jla_boost/graph/PairToRangeAdaptor.hpp>

#include <boost/lexical_cast.hpp>

#include <cassert>
#include <iostream>
#include <map>

namespace mod::lib::Graph::Write {
namespace {

// returns the filename _without_ extension

std::string getFilePrefix(const std::size_t gId) {
	return lib::IO::makeUniqueFilePrefix() + "g_" + boost::lexical_cast<std::string>(gId);
}

void escapeLabelForDot(const std::string &label, std::ostream &s) {
	for(char c: label) {
		if(c == '"') s << "\\\"";
		else if(c == '\\') s << "\\\\";
		else s << c;
	}
}

} // namespace

void gml(const LabelledGraph &gLabelled, const DepictionData &depict, const std::size_t gId,
         bool withCoords, std::ostream &s) {
	if(!depict.getHasCoordinates() && withCoords) MOD_ABORT;
	const auto &g = get_graph(gLabelled);
	const auto &pString = get_string(gLabelled);
	s << "graph [\n";
	for(auto v: asRange(vertices(g))) {
		s << "\tnode [ id " << get(boost::vertex_index_t(), g, v) << " label \"" << pString[v] << "\"";
		if(withCoords)
			s << " vis2d [ x " << depict.getX(v, true) << " y " << depict.getY(v, true) << " ]";
		s << " ]\n";
	}
	for(auto e: asRange(edges(g))) {
		s << "\tedge [ source " << get(boost::vertex_index_t(), g, source(e, g))
		  << " target " << get(boost::vertex_index_t(), g, target(e, g))
		  << " label \"" << pString[e] << "\" ]\n";
	}
	s << "]\n";
}

std::string gml(const Single &g, bool withCoords) {
	static std::map<std::pair<std::size_t, bool>, std::string> cache;
	const auto iter = cache.find({g.getId(), withCoords});
	if(iter != end(cache)) return iter->second;

	std::string fileNoExt = getFilePrefix(g.getId());
	post::FileHandle s(fileNoExt + ".gml");
	gml(g.getLabelledGraph(), g.getDepictionData(), g.getId(), withCoords, s);

	cache[{g.getId(), withCoords}] = s;
	return s;
}

namespace {
namespace dfsDetail {
using namespace IO::DFS;
using Vertex = IO::DFS::Vertex;
using Edge = IO::DFS::Edge;

using GVertex = lib::Graph::Vertex;
using GEdge = lib::Graph::Edge;

enum class Colour {
	White, Grey, Black
};

struct Printer {
	Printer(std::ostream &s, const std::map<int, int> &idMap) : s(s), idMap(idMap) {}

	void operator()(const Chain &chain) {
		(*this)(chain.head);
		(*this)(chain.tail);
	}

	void operator()(const Vertex &v) {
		boost::apply_visitor(*this, v.vertex);
		for(const Branch &b: v.branches) {
			s << "(";
			(*this)(b.tail);
			s << ")";
		}
	}

	void operator()(const LabelVertex &v) {
		s << "[";
		escapeLabel(s, v.label, ']');
		s << "]";
		if(v.id)
			if(const auto iter = idMap.find(*v.id); iter != idMap.end())
				s << iter->second;
	}

	void operator()(const RingClosure &v) {
		assert(v.id != std::numeric_limits<unsigned int>::max());
		const auto iter = idMap.find(v.id);
		assert(iter != idMap.end());
		s << iter->second;
	}

	void operator()(const std::vector<EVPair> &evPairs) {
		for(const EVPair &p: evPairs) {
			(*this)(p.first);
			(*this)(p.second);
		}
	}

	void operator()(const Edge &e) {
		if(e.label.size() == 1) {
			char c = e.label[0];
			switch(c) {
			case '-':
				return;
			case ':':
			case '=':
			case '#':
				s << c;
				return;
			}
		}
		s << "{";
		escapeLabel(s, e.label, '}');
		s << "}";
	}
private:
	std::ostream &s;
	const std::map<int, int> &idMap;
};

struct Prettyfier {
	struct EndsWithNumber {
		bool operator()(const LabelVertex &v) const {
			return v.id.has_value();
		}

		bool operator()(const RingClosure &v) const {
			return true;
		}
	};

	struct IsRingClorsure {
		bool operator()(const LabelVertex &v) const {
			return false;
		}

		bool operator()(const RingClosure &v) const {
			return true;
		}
	};
public:
	Prettyfier(const std::vector<bool> &targetForRing) : targetForRing(targetForRing) {}

	void operator()(Chain &chain) {
		(*this)(chain.head);
		(*this)(chain.tail);
		if(chain.head.branches.size() > 0 && chain.tail.size() == 0) {
			// V(B)..(B)(B)
			chain.tail = std::move(chain.head.branches.back().tail);
			chain.head.branches.pop_back();
			// V(B)..(B)B
		}
	}

	void operator()(Vertex &v) {
		boost::apply_visitor(*this, v.vertex);
		for(Branch &b: v.branches) (*this)(b.tail);
	}

	void operator()(LabelVertex &v) {
		if(v.id)
			if(!targetForRing[*v.id]) v.id.reset();
	}

	void operator()(RingClosure &v) {
		assert(v.id != std::numeric_limits<unsigned int>::max());
		assert(targetForRing[v.id]);
	}

	void operator()(std::vector<EVPair> &evPairs) {
		for(EVPair &p: evPairs) {
			(*this)(p.first);
			(*this)(p.second);
		}
		if(evPairs.empty()) return;
		// -EV...-EV
		auto &lastVertex = evPairs.back().second;
		if(lastVertex.branches.empty()) return;
		// -EV...-EV(B)..(B)(B)
		// pull up the last B to be more EV-pairs
		if(lastVertex.branches.size() == 1 && boost::apply_visitor(EndsWithNumber(), lastVertex.vertex)) {
			// -EV...-EVN(B)
			// or
			// -EV...-N(B)
			assert(!lastVertex.branches.front().tail.empty());
			const auto &firstBranchEV = lastVertex.branches.front().tail.front();
			if(firstBranchEV.first.label == "-" && boost::apply_visitor(IsRingClorsure(), firstBranchEV.second.vertex)) {
				// -EV...-EVN(M)
				// or
				// -EV...-N(M)
				return;
			}
		}
		std::vector<EVPair> lastTail = std::move(evPairs.back().second.branches.back().tail);
		evPairs.back().second.branches.pop_back();
		evPairs.insert(end(evPairs), std::move_iterator(lastTail.begin()), std::move_iterator(lastTail.end()));
	}

	void operator()(Edge &e) {}
private:
	const std::vector<bool> &targetForRing;
};

std::pair<Chain, std::map<int, int>>
write(const GraphType &g, const PropString &pString, bool withIds) {
	using namespace detail;
	using GEdgeIter = GraphType::out_edge_iterator;
	using VertexInfo = std::pair<GVertex, std::pair<GEdgeIter, GEdgeIter>>;
	std::vector<Vertex *> realVertices(num_vertices(g), nullptr);
	Chain chain;
	chain.hasNonSmilesRingClosure = false;

	std::vector<Colour> colour(num_vertices(g), Colour::White);
	// note: if withIds, pretend *all* vertices are targets for ring closures
	std::vector<bool> targetForRing(num_vertices(g), withIds);
	std::map<GEdge, Colour> edgeColour;
	for(GEdge e: asRange(edges(g))) edgeColour[e] = Colour::White;
	std::stack<VertexInfo> stack;
	{ // discover root
		GVertex cur = *vertices(g).first;
		unsigned int curId = get(boost::vertex_index_t(), g, cur);
		assert(curId < colour.size());
		colour[curId] = Colour::Grey;
		assert(!realVertices[curId]);
		realVertices[curId] = &chain.head;
		LabelVertex lv;
		lv.id = curId;
		lv.label = pString[cur];
		realVertices[curId]->vertex = lv;
		stack.push(std::make_pair(cur, out_edges(cur, g)));
	}
	while(!stack.empty()) {
		GVertex cur = stack.top().first;
		GEdgeIter iter, iterEnd;
		boost::tie(iter, iterEnd) = stack.top().second;
		stack.pop();
		unsigned int curId = get(boost::vertex_index_t(), g, cur);
		assert(realVertices[curId]);
		Vertex *curVertex = realVertices[curId];
		//		std::cout << "CurVertex: " << pString(cur) << std::endl;
		while(iter != iterEnd) {
			GVertex next = target(*iter, g);
			GEdge test;
			unsigned int nextId = get(boost::vertex_index_t(), g, next);
			Edge edge;
			edge.label = pString[*iter];
			// mark edge
			Colour oldEdgeColour = edgeColour[*iter];
			edgeColour[*iter] = Colour::Black;
			//			std::cout << "\tEdge: " << pString(cur)
			//					<< " ->(" << edge.label << ", " << (oldEdgeColour == Black ? "black" : "white") << ") "
			//					<< pString(next) << "\t";
			if(colour[nextId] == Colour::White) { // tree edge, new vertex
				//				std::cout << "white" << std::endl;
				// create the new vertex
				assert(!realVertices[nextId]);
				LabelVertex lv;
				lv.id = nextId;
				lv.label = pString[next];
				Vertex newVertex;
				newVertex.vertex = lv;
				curVertex->branches.push_back((Branch()));
				curVertex->branches.back().tail.push_back(std::make_pair(edge, newVertex));
				Vertex *nextVertex = &curVertex->branches.back().tail.back().second;
				realVertices[nextId] = nextVertex;
				colour[nextId] = Colour::Grey;
				// switch to the new vertex
				iter++;
				stack.push(std::make_pair(cur, std::make_pair(iter, iterEnd)));
				cur = next;
				curId = nextId;
				curVertex = nextVertex;
				boost::tie(iter, iterEnd) = out_edges(next, g);
				//				std::cout << "CurVertex: " << pString(cur)<< std::endl;
			} else if(colour[nextId] == Colour::Grey) { // back edge, maybe an already traversed edge
				//				std::cout << "grey" << std::endl;
				if(oldEdgeColour == Colour::Black) iter++; // already traversed
				else {
					RingClosure rc;
					rc.id = nextId;
					Vertex backVertex;
					backVertex.vertex = rc;
					curVertex->branches.push_back((Branch()));
					curVertex->branches.back().tail.push_back(std::make_pair(edge, backVertex));
					if(targetForRing[nextId]) chain.hasNonSmilesRingClosure = true;
					targetForRing[nextId] = true;
					iter++;
				}
			} else {
				//				std::cout << "black" << std::endl;
				iter++;
			}
		}
		colour[curId] = Colour::Black;
	}

	std::map<int, int> idMap;
	int nextMappedId = 1;
	for(int id = 0; id != targetForRing.size(); id++)
		if(targetForRing[id])
			idMap[id] = nextMappedId++;
	Prettyfier pretty(targetForRing);
	pretty(chain);
	return std::make_pair(chain, idMap);
}

} // namespace dfsDetail
} // namespace

std::pair<std::string, bool> dfs(const LabelledGraph &gLabelled, bool withIds) {
	const auto &g = get_graph(gLabelled);
	const auto &pString = get_string(gLabelled);
	if(num_vertices(g) == 0) return std::pair("", false);
	auto [chain, idMap] = dfsDetail::write(g, pString, withIds);

	std::stringstream graphDFS;
	dfsDetail::Printer p(graphDFS, idMap);
	p(chain);
	return std::pair(graphDFS.str(), chain.hasNonSmilesRingClosure);
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

std::string dot(const LabelledGraph &gLabelled, const std::size_t gId, const Options &options) {
	static std::map<DotCacheEntry, std::string> cache;
	const auto iter = cache.find({gId, options.graphvizPrefix});
	if(iter != end(cache)) return iter->second;

	// use _gv suffix so a coord file will not clash with non-gv coord files
	post::FileHandle s(getFilePrefix(gId) + "_gv.dot");
	const auto &g = get_graph(gLabelled);
	const auto &pString = get_string(gLabelled);
	s << "graph G {\n";
	s << "\tnode [shape=plaintext]\n";
	if(!options.graphvizPrefix.empty())
		s << "\t" << options.graphvizPrefix << '\n';

	for(const auto v: asRange(vertices(g))) {
		s << '\t' << get(boost::vertex_index_t(), g, v) << " [ label=\"";
		escapeLabelForDot(pString[v], s);
		s << "\" ]\n";
	}
	for(const auto e: asRange(edges(g))) {
		const auto srcId = get(boost::vertex_index_t(), g, source(e, g));
		const auto tarId = get(boost::vertex_index_t(), g, target(e, g));
		const std::string &label = pString[e];
		if(label == "-" || label == "=" || label == "#") {
			s << srcId << " -- " << tarId << '\n';
			if(label != "-") s << srcId << " -- " << tarId << '\n';
			if(label == "#") s << srcId << " -- " << tarId << '\n';
		} else {
			s << srcId << " -- " << tarId << " [ label=\"";
			escapeLabelForDot(label, s);
			s << "\" ]\n";
		}
	}
	s << "}\n";
	cache[{gId, options.graphvizPrefix}] = s;
	return s;
}

namespace {

struct OpenBabelCoordsCacheEntry {
	std::size_t id;
	bool collapseHydrogens;
	int rotation;
	bool mirror;
public:
	friend bool operator<(const OpenBabelCoordsCacheEntry &a, const OpenBabelCoordsCacheEntry &b) {
		return std::tie(a.id, a.collapseHydrogens, a.rotation, a.mirror)
		       < std::tie(b.id, b.collapseHydrogens, b.rotation, b.mirror);
	}
};

} // namespace

std::string coords(const LabelledGraph &gLabelled, const DepictionData &depict,
                   const std::size_t gId, const Options &options) {
	if(options.withGraphvizCoords || !depict.getHasCoordinates()) {
		// we map 1-to-1 a dot file to a coord file, so cache by the dot filename
		static std::map<std::string, std::string> cache;

		auto fileNoExt = dot(gLabelled, gId, options);
		fileNoExt.erase(fileNoExt.end() - 4, fileNoExt.end());
		const auto iter = cache.find(fileNoExt);
		if(iter != end(cache)) return iter->second;

		lib::IO::post() << "coordsFromGV graph \"" << fileNoExt << "\"\n";

		// the coord file is still for the tex coord file which is just then created in post
		std::string file = fileNoExt + "_coord.tex";
		cache[fileNoExt] = file;
		return file;
	} else {
		static std::map<OpenBabelCoordsCacheEntry, std::string> cache;
		const auto iter = cache.find({gId, options.collapseHydrogens, options.rotation, options.mirror});
		if(iter != end(cache)) return iter->second;

		const auto &g = get_graph(gLabelled);
		std::string f = getFilePrefix(gId);
		if(options.collapseHydrogens) f += "_mol";
		if(options.rotation != 0) f += "_r" + std::to_string(options.rotation);
		if(options.mirror) f += "_m" + std::to_string(options.mirror);
		post::FileHandle s(f + "_coord.tex");
		s << "% dummy\n";
		for(const auto v: asRange(vertices(g))) {
			const auto vId = get(boost::vertex_index_t(), g, v);
			if(options.collapseHydrogens && Chem::isCollapsibleHydrogen(v, g, depict, depict, [&depict](const auto v) {
				return depict.hasImportantStereo(v);
			}))
				continue;
			double x, y;
			std::tie(x, y) = pointTransform(
					depict.getX(v, !options.collapseHydrogens),
					depict.getY(v, !options.collapseHydrogens),
					options.rotation, options.mirror);
			s << "\\coordinate[overlay] (\\modIdPrefix v-coord-" << vId << ") at ("
			  << std::fixed << x << ", " << y << ") {};\n";
		}
		std::string file = s;
		cache[{gId, options.collapseHydrogens, options.rotation, options.mirror}] = file;
		return file;
	}
}

namespace {

struct TikzCacheEntry {
	std::size_t id;
	std::string coordFile;
	std::string options;
	bool asInline;
	std::string idPrefix;
public:
	friend bool operator<(const TikzCacheEntry &a, const TikzCacheEntry &b) {
		return std::tie(a.id, a.coordFile, a.options, a.asInline, a.idPrefix)
		       < std::tie(b.id, b.coordFile, b.options, b.asInline, b.idPrefix);
	}
};

} // namespace

std::pair<std::string, std::string>
tikz(const LabelledGraph &gLabelled, const DepictionData &depict, const std::size_t gId,
     const Options &options,
     bool asInline, const std::string &idPrefix) {
	static std::map<TikzCacheEntry, std::string> cache;

	std::string strOptions = options.getStringEncoding();
	std::string fileCoordsExt = coords(gLabelled, depict, gId, options);

	const auto iter = cache.find({gId, fileCoordsExt, strOptions, asInline, idPrefix});
	if(iter != end(cache)) return std::pair(iter->second, fileCoordsExt);

	std::string file = getFilePrefix(gId) + "_" + strOptions;
	if(asInline) file += "i";
	file += ".tex";
	post::FileHandle s(file);
	tikz(s, options, get_graph(gLabelled), depict, fileCoordsExt, asInline, idPrefix);

	cache[{gId, fileCoordsExt, strOptions, asInline, idPrefix}] = file;
	return std::pair(file, fileCoordsExt);
}

std::string
pdf(const LabelledGraph &gLabelled, const DepictionData &depict, const std::size_t gId,
    const Options &options) {
	{ // user-specified depiction
		static std::map<std::size_t, std::string> userCache;
		auto iter = userCache.find(gId);
		if(iter != end(userCache)) return iter->second;
		auto fImage = depict.getImage();
		if(fImage) {
			std::string imageNoExt = (*fImage)();
			if(imageNoExt.empty()) {
				std::cout << "User-specified depiction file for graph with id " << gId << " can not be empty." << std::endl;
				throw 0;
			}
			std::string cmd = depict.getImageCommand();
			if(!cmd.empty()) lib::IO::post() << cmd << '\n';
			std::string image = imageNoExt + ".pdf";
			userCache[gId] = image;
			return image;
		}
	}
	// auto-generated depiction
	// maps 1-to-1 a (coord, tikz) pair to a PDF
	static std::map<std::pair<std::string, std::string>, std::string> cache;
	auto tikzFiles = tikz(gLabelled, depict, gId, options, false, "");

	const auto iter = cache.find(tikzFiles);
	if(iter != end(cache)) return iter->second;

	std::string fileNoExt = tikzFiles.first.substr(0, tikzFiles.first.size() - 4);
	std::string fileCoordsNoExt = tikzFiles.second.substr(0, tikzFiles.second.size() - 4);
	lib::IO::post() << "compileTikz \"" << fileNoExt << "\" \"" << fileCoordsNoExt << "\"\n";

	fileNoExt += ".pdf";
	cache[tikzFiles] = fileNoExt;
	return fileNoExt;
}

std::string svg(const LabelledGraph &gLabelled, const DepictionData &depict, const std::size_t gId,
                const Options &options) {
	// maps 1-to-1 a PDF to an SVG
	static std::map<std::string, std::string> cache;

	std::string pdfFile = pdf(gLabelled, depict, gId, options);

	const auto iter = cache.find(pdfFile);
	if(iter != end(cache)) return iter->second;

	std::string fileNoExt = pdfFile.substr(0, pdfFile.size() - 4);
	lib::IO::post() << "pdfToSvg \"" << fileNoExt << "\" \"" << fileNoExt << "\"\n";

	std::string file = fileNoExt + ".svg";
	cache[pdfFile] = file;
	return file;
}

std::pair<std::string, std::string> summary(const Single &g, const Options &first, const Options &second) {
	std::string graphLike = pdf(g, first);
	std::string molLike = first == second ? "" : pdf(g, second);
	lib::IO::post() << "summaryGraph \"" << g.getName() << "\" \""
	                << std::string(begin(graphLike), end(graphLike) - 4) << "\" \"";

	if(!molLike.empty())
		lib::IO::post() << std::string(begin(molLike), end(molLike) - 4);
	lib::IO::post() << "\"\n";

	if(molLike.empty())
		return std::pair(graphLike, graphLike);
	else
		return std::pair(graphLike, molLike);
}

namespace {

template<typename LGraph>
void termStateImpl(const std::string &name, const LGraph &g) {
	lib::IO::post() << "summarySubsection \"Term State for " << name << "\"\n";
	using namespace lib::Term;
	post::FileHandle s(lib::IO::makeUniqueFilePrefix() + "termState.tex");
	s << "\\begin{verbatim}\n";
	const auto &termState = get_term(g);
	const auto &gg = get_graph(g);
	if(isValid(termState)) {
		std::unordered_map<Address, std::vector<std::string>> annotations;
		for(const auto v: asRange(vertices(get_graph(g)))) {
			Address a{AddressType::Heap, termState[v]};
			annotations[a].push_back("v" + std::to_string(get(boost::vertex_index_t(), gg, v)));
		}
		for(const auto e: asRange(edges(get_graph(g)))) {
			Address a{AddressType::Heap, termState[e]};
			annotations[a].push_back(
					"e(" + std::to_string(get(boost::vertex_index_t(), gg, source(e, gg)))
					+ ", " + std::to_string(get(boost::vertex_index_t(), gg, target(e, gg))) + ")");
		}
		lib::Term::Write::wam(getMachine(termState), lib::Term::getStrings(),
		                      IO::Logger(s), [&](Address addr, std::ostream &s) {
					bool first = true;
					for(const auto &str: annotations[addr]) {
						if(!first) s << ", ";
						first = false;
						s << str;
					}
				});
	} else {
		std::string msg = "Parsing failed for graph '" + name + "'. " + termState.getParsingError();
		throw TermParsingError(std::move(msg));
	}
	s << "\\end{verbatim}\n";
	lib::IO::post() << "summaryInput \"" << std::string(s) << "\"\n";
}

} // namespace

void termState(const Single &g) {
	termStateImpl(g.getName(), g.getLabelledGraph());
}

void termState(const lib::LabelledUnionGraph<lib::Graph::LabelledGraph> &g, std::string name) {
	termStateImpl(name, g);
}

std::string stereoSummary(const Single &gLib, Vertex v, const lib::Stereo::Configuration &conf,
                          const IO::Graph::Write::Options &options, int shownIdOffset, const std::string &nameSuffix) {
	const auto &g = gLib.getGraph();
	const auto vId = get(boost::vertex_index_t(), g, v);
	std::string name = "g_" + boost::lexical_cast<std::string>(gLib.getId()) + "_stereo_" +
	                   boost::lexical_cast<std::string>(vId);
	IO::post() << "summarySubsection \"Stereo, g " << gLib.getId() << ", v " << vId
	           << nameSuffix << "\"\n";
	std::string f = lib::Stereo::Write::pdf(g, v, conf, name, gLib.getDepictionData(), options,
	                                        [shownIdOffset](const auto &g, const auto v) {
		                                        return get(boost::vertex_index_t(), g, v) + shownIdOffset;
	                                        });
	post::FileHandle s(IO::makeUniqueFilePrefix() + "stereo.tex");
	s << "\\begin{center}\n";
	s << "\\includegraphics{" << f << "}\\\\\n";
	s << "File: \\texttt{" << IO::escapeForLatex(f) << "}\n";
	s << "\\end{center}\n";
	IO::post() << "summaryInput \"" << std::string(s) << "\"\n";
	return f;
}

//------------------------------------------------------------------------------
// Simplified interface for Single
//------------------------------------------------------------------------------

void gml(const Single &g, bool withCoords, std::ostream &s) {
	gml(g.getLabelledGraph(), g.getDepictionData(), g.getId(), withCoords, s);
}

std::string tikz(const Single &g, const Options &options, bool asInline, const std::string &idPrefix) {
	auto res = tikz(g.getLabelledGraph(), g.getDepictionData(), g.getId(), options, asInline, idPrefix);
	return res.first;
}

std::string pdf(const Single &g, const Options &options) {
	return pdf(g.getLabelledGraph(), g.getDepictionData(), g.getId(), options);
}

std::string svg(const Single &g, const Options &options) {
	return svg(g.getLabelledGraph(), g.getDepictionData(), g.getId(), options);
}

} // namespace mod::lib::Graph::Write