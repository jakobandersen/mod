#ifndef MOD_LIB_DG_WRITE_HPP
#define MOD_LIB_DG_WRITE_HPP

#include <mod/Post.hpp>
#include <mod/lib/DG/Hyper.hpp>

#include <mod/lib/IO/IO.hpp>
#include <mod/lib/IO/Json.hpp>

#include <iosfwd>
#include <string>
#include <unordered_map>

namespace mod::lib::DG {
struct Hyper;
struct NonHyper;
} // namespace mod::lib::DG
namespace mod::lib::DG::Write {
using Vertex = HyperVertex;
using Edge = HyperEdge;

nlohmann::json dumpToJson(const NonHyper &dg);

std::string dotNonHyper(const NonHyper &nonHyper);
std::string pdfNonHyper(const NonHyper &nonHyper);

//------------------------------------------------------------------------------
// Old/New delimiter
//------------------------------------------------------------------------------

struct SyntaxPrinter;

struct Options {
	struct DupVProp {
		Vertex v;
		int dupNum;
	};
	using DupGraphType = boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, DupVProp>;
	using DupVertex = boost::graph_traits<DupGraphType>::vertex_descriptor;
	using DupEdge = boost::graph_traits<DupGraphType>::edge_descriptor;
public:
	Options &Non() {
		return WithShortcutEdges(false).WithGraphImages(false).LabelsAsLatexMath(false);
	}

	Options &All() {
		return WithShortcutEdges(true).WithGraphImages(true).LabelsAsLatexMath(true);
	}

	Options &WithShortcutEdges(bool v) {
		withShortcutEdges = v;
		return *this;
	}

	Options &WithGraphImages(bool v) {
		withGraphImages = v;
		return *this;
	}

	Options &LabelsAsLatexMath(bool v) {
		labelsAsLatexMath = v;
		return *this;
	}

	Options &WithShortcutEdgesAfterVisibility(bool v) {
		withShortcutEdgesAfterVisibility = v;
		return *this;
	}

	Options &WithInlineGraphs(bool v) {
		withInlineGraphs = v;
		return *this;
	}

	Options &Suffix(std::string s) {
		suffix = s;
		return *this;
	}

	operator std::string() const {
		auto toChar = [](bool b) {
			return b ? '1' : '0';
		};
		std::string res;
		res += toChar(withShortcutEdges);
		res += toChar(withGraphImages);
		res += toChar(labelsAsLatexMath);
		res += toChar(withShortcutEdgesAfterVisibility);
		res += toChar(withInlineGraphs);
		if(!suffix.empty()) res += "_" + suffix;
		return res;
	}

	// stuff not giving state

	bool isVertexVisible(Vertex v, const Hyper &dg) const {
		return vertexVisible ? vertexVisible(v, dg) : true;
	}

	std::string getVertexLabel(const Hyper &dg, DupVertex vDup) const {
		std::string label;
		const auto v = dupGraph[vDup].v;
		const auto dupNum = dupGraph[vDup].dupNum;
		if(dupVertexLabel) label += dupVertexLabel(v, dupNum, dg);
		if(dupVertexLabel && vertexLabel) label += ", ";
		if(vertexLabel) label += vertexLabel(v, dg);
		return label;
	}

	bool isHyperedgeVisible(Vertex v, const Hyper &dg) const {
		return hyperedgeVisible ? hyperedgeVisible(v, dg) : true;
	}

	std::string getHyperedgeLabel(Vertex v, const Hyper &dg) const {
		return hyperedgeLabel ? hyperedgeLabel(v, dg) : "";
	}

	std::string getVertexColour(DupVertex vDup, const Hyper &dg) const {
		const auto v = dupGraph[vDup].v;
		const auto dupNum = dupGraph[vDup].dupNum;
		if(dupVertexColour) {
			return dupVertexColour(v, dupNum, dg);
		} else if(vertexColour) {
			return vertexColour(v, dg);
		} else {
			return "";
		}
	}

	std::string getHyperedgeColour(Vertex v, const Hyper &dg) const {
		return hyperedgeColour ? hyperedgeColour(v, dg) : "";
	}

	std::string getTailColour(Vertex v, Vertex e, const Hyper &dg) const {
		return tailColour ? tailColour(v, e, dg) : "";
	}

	std::string getHeadColour(Vertex e, Vertex v, const Hyper &dg) const {
		return headColour ? headColour(e, v, dg) : "";
	}

	std::pair<int, DupVertex> inDegreeVisible(DupVertex e, const Hyper &dg) const;
	std::pair<int, DupVertex> outDegreeVisible(DupVertex e, const Hyper &dg) const;
	bool isShortcutEdge(DupVertex e,
	                    const Hyper &dg,
	                    int inDegreeVisible, int outDegreeVisible) const;
	std::string vDupToId(DupVertex vDup, const Hyper &dg) const;
public:
	bool withShortcutEdges = true;
	bool withGraphImages = true;
	bool labelsAsLatexMath = true;
	bool withShortcutEdgesAfterVisibility = false;
	bool withInlineGraphs = false;
	std::string suffix;
	// appearance (not giving state)
	std::function<bool(Vertex, const Hyper &)> vertexVisible;
	std::function<std::string(Vertex, const Hyper &)> vertexLabel;
	std::function<std::string(Vertex, int dupNum, const Hyper &)> dupVertexLabel;
	std::function<bool(Vertex, const Hyper &)> hyperedgeVisible;
	std::function<std::string(Vertex, const Hyper &)> hyperedgeLabel;
	std::function<std::string(Vertex, const Hyper &)> vertexColour;
	std::function<std::string(Vertex, int dupNum, const Hyper &)> dupVertexColour;
	std::function<std::string(Vertex, const Hyper &)> hyperedgeColour;
	std::function<std::string(Vertex, Vertex, const Hyper &)> tailColour, headColour;
	std::function<void(const Hyper &, const Options &, SyntaxPrinter &)> auxPrinter;
	// GraphPrinter overrides
	std::function<int(std::shared_ptr<mod::graph::Graph>)> rotationOverwrite;
	std::function<bool(std::shared_ptr<mod::graph::Graph>)> mirrorOverwrite;
	// Graph overrides
	std::function<std::pair<std::string, std::string>(Vertex v, int dupNum, const Hyper &)> imageOverwrite;
	// rendering engine things
	std::string graphvizPrefix;
	std::string tikzpictureOption = "scale=\\modDGHyperScale";
	// duplication (not giving state)
	// all vertices must be first and then all edges
	// all incarnations must be contiguous and in increasing order
	// this is only for prettier tikz code
	// Eh, what? it should correspond to what the user specified
	DupGraphType dupGraph;
};

struct SyntaxPrinter {
	SyntaxPrinter(std::string file) : s(file) {}

	virtual std::string getName() const = 0;
	virtual void begin() = 0;
	virtual void end() = 0;
	virtual void comment(const std::string &str) = 0;
	virtual void vertex(const std::string &id,
	                    const std::string &label,
	                    const std::string &image,
	                    const std::string &colour) = 0;
	virtual void vertexHidden(const std::string &id, bool large) = 0;
	virtual void hyperEdge(const std::string &id, const std::string &label, const std::string &colour) = 0;
	virtual void tailConnector(const std::string &idVertex,
	                           const std::string &idHyperEdge,
	                           const std::string &colour,
	                           int num, int maxNum) = 0;
	virtual void headConnector(const std::string &idHyperEdge,
	                           const std::string &idVertex,
	                           const std::string &colour,
	                           int num, int maxNum) = 0;
	virtual void shortcutEdge(const std::string &idTail,
	                          const std::string &idHead,
	                          const std::string &label,
	                          const std::string &colour,
	                          bool hasReverse) = 0;
	virtual std::function<std::string(const Hyper &,
	                                  HyperVertex,
	                                  Options::DupVertex,
	                                  const std::string &)> getImageCreator() = 0;
public:
	post::FileHandle s;
};

struct TikzPrinter : SyntaxPrinter {
	TikzPrinter(std::string file,
	            std::string coords,
	            const Options &options,
	            const IO::Graph::Write::Options &graphOptions)
			: SyntaxPrinter(file), coords(coords), options(options), graphOptions(graphOptions) {}

	std::string getName() const override {
		return "tikz";
	}

	void begin() override;
	void end() override;
	void comment(const std::string &str) override;
	void vertex(const std::string &id,
	            const std::string &label,
	            const std::string &image,
	            const std::string &colour) override;
	void vertexHidden(const std::string &id, bool large) override;
	void transitVertex(const std::string &idHost,
	                   const std::string &idTransit,
	                   const std::string &angle,
	                   const std::string &label);
	void hyperEdge(const std::string &id, const std::string &label, const std::string &colour) override;
	void connector(const std::string &idTail,
	               const std::string &idHead,
	               const std::string &colour,
	               int num, int maxNum);
	void tailConnector(const std::string &idVertex,
	                   const std::string &idHyperEdge,
	                   const std::string &colour,
	                   int num, int maxNum) override;
	void headConnector(const std::string &idHyperEdge,
	                   const std::string &idVertex,
	                   const std::string &colour,
	                   int num, int maxNum) override;
	void shortcutEdge(const std::string &idTail,
	                  const std::string &idHead,
	                  const std::string &label,
	                  const std::string &colour,
	                  bool hasReverse) override;
	void transitEdge(const std::string &idTail,
	                 const std::string &idHead,
	                 const std::string &label,
	                 const std::string &colour);
	std::function<std::string(const Hyper &,
	                          HyperVertex,
	                          Options::DupVertex,
	                          const std::string &)> getImageCreator() override;
public:
	std::string coords;
	const Options &options;
	const IO::Graph::Write::Options &graphOptions;
};

struct Data {
	struct Connections {
		Connections(int numTails, int numHeads) : tail(numTails, 0), head(numHeads, 0) {}

	public:
		// indices are offsets on the in-/out-edge iterators
		// values are the duplicate numbers
		std::vector<int> tail, head;
	};
	using ConnectionsStore = std::unordered_map<Vertex, std::unordered_map<int, Connections>>;
public:
	explicit Data(const Hyper &dg);
	void compile(Options &options) const;
	// returns: was newly created
	bool makeDuplicate(Vertex e, int eDup);
	// returns: was removed
	bool removeDuplicate(Vertex e, int eDup);
	void reconnectSource(Vertex e, int eDup, Vertex tail, int vDupTar, int vDupSrc);
	void reconnectTarget(Vertex e, int eDup, Vertex head, int vDupTar, int vDupSrc);
	void removeVertexIfDegreeZero(Vertex v);
public:
	const Hyper &dg;
private:
	ConnectionsStore connections; // hyper-edge -> dupNum -> Connections
	std::set<Vertex> removedIfDegreeZero;
public:
	std::function<std::string(Vertex, int dupNum, const Hyper &)> dupVertexLabel;
	std::function<std::string(Vertex, int dupNum, const Hyper &)> dupVertexColour;
};

struct Printer {
	std::pair<std::string, std::string> printHyper(const Data &data, const IO::Graph::Write::Options &graphOptions);
	void pushSuffix(const std::string suffix);
	void popSuffix();
	void pushVertexVisible(std::function<bool(Vertex, const Hyper &)> f); // visible(v) <=> all of pushed f(v))
	void popVertexVisible();
	bool hasVertexVisible() const;
	void pushEdgeVisible(std::function<bool(Vertex, const Hyper &)> f); // visible(v) <=> all of pushed f(v))
	void popEdgeVisible();
	bool hasEdgeVisible() const;
	void pushVertexLabel(std::function<std::string(Vertex, const Hyper &)> f);
	void popVertexLabel();
	bool hasVertexLabel() const;
	void pushEdgeLabel(std::function<std::string(Vertex, const Hyper &)> f);
	void popEdgeLabel();
	bool hasEdgeLabel() const;
	void pushVertexColour(std::function<std::string(Vertex, const Hyper &)> f,
	                      bool extendToEdges); // colour(v) == first f(v) != ""
	void popVertexColour();
	bool hasVertexColour() const;
	void pushEdgeColour(std::function<std::string(Vertex, const Hyper &)> f); // colour(v) == first f(v) != ""
	void popEdgeColour();
	bool hasEdgeColour() const;
public: // GraphPrinter overrides
	void setRotationOverwrite(std::function<int(std::shared_ptr<mod::graph::Graph>)> f);
	void setMirrorOverwrite(std::function<bool(std::shared_ptr<mod::graph::Graph>)> f);
public: // Graph overrides
	void setImageOverwrite(std::function<std::pair<std::string, std::string>(
			Vertex v, int dupNum, const Hyper &)> f);
public: // rendering engine things
	void setGraphvizPrefix(const std::string &prefix);
	const std::string &getGraphvizPrefix() const;
	void setTikzpictureOption(const std::string &option);
	const std::string &getTikzpictureOption() const;
public:
	Options prePrint(const Data &data);
	void postPrint();
private:
	void setup(Options &options);
public:
	Options baseOptions; // only save stuff giving state, and the rendering engine things, dynamically add the rest
private:
	std::vector<std::string> suffixes;
	// not giving state
	std::vector<std::function<bool(Vertex, const Hyper &)> > vertexVisibles, edgeVisibles;
	std::vector<std::function<std::string(Vertex, const Hyper &)> > vertexLabels, edgeLabels;
	std::vector<std::pair<std::function<std::string(Vertex, const Hyper &)>, bool> > vertexColour;
	std::vector<std::function<std::string(Vertex, const Hyper &)> > edgeColour;
public:
	std::string vertexLabelSep = ", ", edgeLabelSep = ", ";
	bool withGraphName = true, withRuleName = false, withRuleId = true;
};

void generic(const Hyper &dg, const Options &options, SyntaxPrinter &print);
std::string dot(const Hyper &dg, const Options &options, const IO::Graph::Write::Options &graphOptions);
std::string coords(const Hyper &dg, const Options &options, const IO::Graph::Write::Options &graphOptions);
std::pair<std::string, std::string> tikz(const Hyper &dg, const Options &options,
                                         const IO::Graph::Write::Options &graphOptions);
std::string pdfFromDot(const Hyper &dg, const Options &options, const IO::Graph::Write::Options &graphOptions);
std::pair<std::string, std::string> pdf(const Hyper &dg, const Options &options,
                                        const IO::Graph::Write::Options &graphOptions);
std::pair<std::string, std::string> summary(const Data &data, Printer &printer,
                                            const IO::Graph::Write::Options &graphOptions);
std::string summaryNonHyper(const NonHyper &dg);

std::vector<std::pair<std::string, std::string>>
summaryDerivation(const NonHyper &dg, HyperVertex v, const IO::Graph::Write::Options &options,
                  const std::string &nomatchColour, const std::string &matchColour,
						const int verbosity);

// utilities for those referring to DG elements in their dumps
nlohmann::json dumpDigest(const HyperGraphType &dg);

} // namespace mod::lib::DG::Write

#endif // MOD_LIB_DG_WRITE_HPP