#include "Read.hpp"

#include <mod/Error.hpp>
#include <mod/dg/DG.hpp>
#include <mod/lib/CombiOpt/LoadedSolution.hpp>
#include <mod/lib/Graph/Graph.hpp>
#include <mod/lib/HyperFlow/Flow.hpp>
#include <mod/lib/HyperFlow/Modules/Base.hpp>
#include <mod/lib/HyperFlow/Modules/OverallCatalysis.hpp>
#include <mod/lib/HyperFlow/Modules/OverallAutocatalysis.hpp>
#include <mod/lib/IO/IO.hpp>
#include <mod/lib/IO/Json.hpp>
#include <mod/lib/IO/Parsing.hpp>
#include <mod/lib/Rule/Rule.hpp>

#include <jla_boost/graph/PairToRangeAdaptor.hpp>

#include <boost/fusion/include/std_pair.hpp>
#include <boost/spirit/home/x3/auxiliary/attr.hpp>
#include <boost/spirit/home/x3/auxiliary/eoi.hpp>
#include <boost/spirit/home/x3/auxiliary/eol.hpp>
#include <boost/spirit/home/x3/char/char.hpp>
#include <boost/spirit/home/x3/char/char_class.hpp>
#include <boost/spirit/home/x3/directive/lexeme.hpp>
#include <boost/spirit/home/x3/directive/repeat.hpp>
#include <boost/spirit/home/x3/numeric/bool.hpp>
#include <boost/spirit/home/x3/numeric/int.hpp>
#include <boost/spirit/home/x3/numeric/uint.hpp>
#include <boost/spirit/home/x3/operator/alternative.hpp>
#include <boost/spirit/home/x3/operator/difference.hpp>
#include <boost/spirit/home/x3/operator/kleene.hpp>
#include <boost/spirit/home/x3/operator/sequence.hpp>
#include <boost/spirit/home/x3/string/literal_string.hpp>
#include <boost/spirit/include/support_multi_pass.hpp>

#include <iostream>

namespace mod::lib::HyperFlow::Read {
namespace {

using BaseIteratorType = std::istream_iterator<char>;
using IteratorType = spirit::multi_pass<BaseIteratorType>;
using PosIter = IO::PositionIter<IteratorType>;

template<typename Parser, typename Attr>
bool parse(IteratorType &textFirst, PosIter &first, const PosIter &last, const Parser &p, Attr &attr,
           bool failureIsError = true) {
	try {
		bool res = IO::detail::ParseDispatch<x3::ascii::space_type>::parse(first, last, p, attr, x3::ascii::space);
		if(!res && failureIsError) {
			std::cout << "Error while parsing Flow dump.\n"
			          << IO::detail::makeParserError(textFirst, first, last, true)
			          << '\n';
			return false;
		}
		return res;
	} catch(const x3::expectation_failure<PosIter> &e) {
		std::cout << "Error while parsing Flow dump.\n"
		          << IO::detail::makeParserExpectationError(e, textFirst, last, true)
		          << '\n';
		return false;
	}
}

#define FAIL do {       \
        return nullptr; \
    } while(false)

#define PARSE(p, a) if(!parse(textFirst, first, last, p, a)) FAIL
#define TRY_PARSE(p, a) parse(textFirst, first, last, p, a, false)

using Vertex = lib::DG::HyperVertex;
using Edge = lib::DG::HyperEdge;

std::unique_ptr<lib::HyperFlow::Flow>
loadVersion_2_3_4_5_6_7(std::shared_ptr<dg::DG> dg, const std::string &ilpSolver,
                        IteratorType &textFirst, PosIter &first, const PosIter &last,
                        IteratorType textLast, unsigned int version, std::ostream &err,
                        int verbosity) {
	const lib::DG::HyperGraphType &dgGraph = dg->getHyper().getGraph();
	auto fPtr = std::make_unique<Flow>(dg, ilpSolver);
	auto &f = *fPtr;

#define MOD_parseExtensionHeader(Name)                                          \
    PARSE("Extension: " #Name, x3::unused);                                     \
    if(!TRY_PARSE("disabled", x3::unused))

	MOD_parseExtensionHeader(Base) {
		auto &mod = f.writeSpec().getModule<BaseSpecification>();
		{ // sources
			std::vector<unsigned int> sourceIds;
			PARSE("sources:" >> *x3::uint_, sourceIds);
			for(unsigned int srcId: sourceIds) {
				Vertex v = vertex(srcId, dgGraph);
				if(dgGraph[v].kind != lib::DG::HyperVertexKind::Vertex) {
					err << "Invalid source id, " << srcId << std::endl;
					FAIL;
				}
				mod.addSource(v);
			}
		}
		{ // sinks
			std::vector<unsigned int> sinkIds;
			PARSE("sinks:" >> *x3::uint_, sinkIds);
			for(unsigned int sinkId: sinkIds) {
				Vertex v = vertex(sinkId, dgGraph);
				if(dgGraph[v].kind != lib::DG::HyperVertexKind::Vertex) {
					err << "Invalid sink id, " << sinkId << std::endl;
					FAIL;
				}
				mod.addSink(v);
			}
		}
		if(version >= 7) {
			std::vector<unsigned int> separatedIds;
			PARSE("separatedIOInternalTransit:" >> *x3::uint_, separatedIds);
			for(unsigned int sepId: separatedIds) {
				Vertex v = vertex(sepId, dgGraph);
				if(dgGraph[v].kind != lib::DG::HyperVertexKind::Vertex) {
					err << "Invalid separatedIOInternalTransit id, " << sepId << std::endl;
					FAIL;
				}
				mod.separateIOInternalTransit(v);
			}
		}
		if(version == 3) { // disallowNullDerivations / allowHyperLoops
			bool disallowNullDerivations;
			PARSE("disallowNullDerivations:" >> x3::bool_, disallowNullDerivations);
			mod.allowHyperLoops = !disallowNullDerivations;
		} else if(version >= 4) {
			bool allowHyperLoops = false;
			PARSE("allowHyperLoops:" >> x3::bool_, allowHyperLoops);
			mod.allowHyperLoops = allowHyperLoops;
		}
		if(version >= 4) {
			bool allowReversal = true, allowIOReverse = true; // initialisation to make GCC shut up
			PARSE("allowReverse:" >> x3::bool_, allowReversal);
			PARSE("allowIOReverse:" >> x3::bool_, allowIOReverse);
			mod.allowReversal = allowReversal;
			mod.allowIOReversal = allowIOReverse;
		}
		if(version >= 6) {
			bool relaxed = false; // initialisation to make GCC shut up
			PARSE("relaxed:" >> x3::bool_, relaxed);
			mod.relaxed = relaxed;
			if(relaxed) {
				throw InputError("Relaxed models can not be loaded.");
			}
		}
	}

	if(version >= 4) {
		MOD_parseExtensionHeader(OverallCatalysis) {
			f.enableOverallCatalysis(false);
			auto &mod = f.writeSpec().getModule<OverallCatalysisSpecification>();
			bool forceExistence, strictTransit;
			PARSE("forceExistence:" >> ("true" >> x3::attr(true) | "false" >> x3::attr(false)), forceExistence);
			PARSE("strictTransit:" >> ("true" >> x3::attr(true) | "false" >> x3::attr(false)), strictTransit);
			mod.forceExistence = forceExistence;
			mod.strictTransit = strictTransit;
		}
	}

	MOD_parseExtensionHeader(OverallAutocatalysis) {
		f.enableOverallAutocatalysis(false);
		auto &mod = f.writeSpec().getModule<OverallAutocatalysisSpecification>();
		bool forceExistence, strictTransit, bfsExclusive;
		PARSE("forceExistence:" >> ("true" >> x3::attr(true) | "false" >> x3::attr(false)), forceExistence);
		PARSE("strictTransit:" >> ("true" >> x3::attr(true) | "false" >> x3::attr(false)), strictTransit);
		PARSE("bfsExclusive:" >> ("true" >> x3::attr(true) | "false" >> x3::attr(false)), bfsExclusive);
		mod.forceExistence = forceExistence;
		mod.strictTransit = strictTransit;
		mod.bfsExclusive = bfsExclusive;
	}

	if(version < 4) {
		MOD_parseExtensionHeader(OverallCatalysis) {
			MOD_ABORT;
		}
	}

	if(version < 4) {
		MOD_parseExtensionHeader(Transit) {
			auto &mod = f.writeSpec().getModule<BaseSpecification>();
			bool allowReversal, allowIOReverse;
			PARSE("allowReverse:" >> x3::bool_, allowReversal);
			PARSE("allowInOutReverse:" >> x3::bool_, allowIOReverse);
			mod.allowReversal = allowReversal;
			mod.allowIOReversal = allowIOReverse;
		}
	}

#undef MOD_parseExtensionHeader

	int maxNumSolutions = 1;
	{ // solver
		int absGap = 1 << 30;
		PARSE("solver:", x3::unused);
		if(TRY_PARSE("Solve", x3::unused)) {
		} else if(TRY_PARSE("EnumerateBy", x3::unused)) {
			PARSE("absGap:" >> x3::int_, absGap);
			PARSE("maxNumSolutions:" >> x3::uint_, maxNumSolutions);
			PARSE("enumerationVars: <not implemented>", x3::unused);
			if(version >= 5) {
				PARSE("transitEnumeration:" >> *x3::uint_, x3::unused);
			}
		} else {
			err << "Could not detect solver" << std::endl;
			FAIL;
		}
		if(absGap >= 0) f.writeSpec().setAbsGap(absGap);
	}

	PARSE("objectiveFunction:" >> x3::lexeme[*(x3::char_ - x3::eol)], x3::unused);
	std::cout << "Notice: objective function ignored (no parser implemented)" << std::endl;

	while(parse(textFirst, first, last, "intVar:", x3::unused, false)) {
		PARSE(x3::lexeme[*(x3::char_ - x3::eol)], x3::unused);
		err << "Notice: intVar ignored (no parser implemented)" << std::endl;
	}

	while(parse(textFirst, first, last, "constraint:", x3::unused, false)) {
		PARSE(x3::lexeme[*(x3::char_ - x3::eol)], x3::unused);
		err << "Notice: constraint ignored (no parser implemented)" << std::endl;
	}

	f.makeModel(verbosity);

	unsigned int numSolutions;
	PARSE("numSolutions:" >> x3::uint_, numSolutions);
	for(unsigned int solNum = 0; solNum < numSolutions; solNum++) {
		const auto parsePrefix = [&](const char *str) {
			unsigned int solNumCand;
			bool res = parse(textFirst, first, last, x3::lit(str) > x3::uint_ > ':', solNumCand);
			if(!res) return false;
			if(solNum != solNumCand) {
				err << "Error while parsing Flow dump.\n"
				    << "Expected solution number " << solNum << ", got " << solNumCand << ".\n"
				    << IO::detail::makeParserError(textFirst, first, last, true) << '\n';
				return false;
			}
			return true;
		};

#define PARSE_PREFIX(Str) if(!parsePrefix(Str)) FAIL

		int objVal;
		PARSE_PREFIX("objectiveValue");
		PARSE(x3::int_, objVal);
		std::cout << "Loaded obj val for " << solNum << " is " << objVal << std::endl;

		std::vector<std::pair<unsigned int, unsigned int> > edgeFlow;
		PARSE_PREFIX("edgeFlow");
		PARSE(*(x3::uint_ >> x3::uint_), edgeFlow);

		std::vector<std::pair<unsigned int, unsigned int> > inFlow;
		PARSE_PREFIX("inFlow");
		PARSE(*(x3::uint_ >> x3::uint_), inFlow);

		std::vector<std::pair<unsigned int, unsigned int> > outFlow;
		PARSE_PREFIX("outFlow");
		PARSE(*(x3::uint_ >> x3::uint_), outFlow);

		std::vector<std::tuple<unsigned int, std::vector<int>, std::vector<int>, unsigned int>> transitFlow;

		CombiOpt::LoadedSolution sol;
		sol.objVal = CombiOpt::Int(objVal);
		const auto &mod = f.getModel().getModule<BaseModel>();
		mod.loadSolution_v2to7(sol, edgeFlow, inFlow, outFlow, transitFlow);
		f.getModel().loadSolution(std::move(sol), verbosity);
	}
	PARSE(x3::eoi, x3::unused);
	return fPtr;
}

} // namespace 

std::unique_ptr<lib::HyperFlow::Flow>
dump(std::shared_ptr<dg::DG> dg, const std::string &ilpSolver, std::istream &s, std::ostream &err, int verbosity) {
	struct FlagsHolder {
		FlagsHolder(std::istream &s) : s(s), flags(s.flags()) {}

		~FlagsHolder() {
			s.flags(flags);
		}
	private:
		std::istream &s;
		std::ios::fmtflags flags;
	} flagsHolder(s);
	s.unsetf(std::ios::skipws);
	IteratorType textFirst = spirit::make_default_multi_pass(BaseIteratorType(s));
	IteratorType textLast;
	PosIter first(textFirst), last(textLast);

	unsigned int version = 0;
	// check version (if the version line is not there, the 'version' variable is not touched)
	if(!TRY_PARSE(x3::ascii::lit("version:") >> x3::uint_, version)) {
		// assume one of the newer versions in json format
		std::vector<std::uint8_t> data(textFirst, textLast);
		const auto jOpt = IO::readJson(data, err);
		if(!jOpt) return nullptr;
		auto f = std::make_unique<lib::HyperFlow::Flow>(dg, ilpSolver);
		const auto res = lib::HyperFlow::Flow::load(*f, *jOpt, verbosity, err);
		if(res) return f;
		else return nullptr;
	}

	const auto &dgHyper = dg->getHyper();
	const lib::DG::HyperGraphType &dgGraph = dgHyper.getGraph();
	{ // check DG stats
		unsigned int numVertices = 0, numEdges = 0;

		for(Vertex v: asRange(vertices(dgGraph))) {
			if(dgGraph[v].kind == lib::DG::HyperVertexKind::Vertex) numVertices++;
			else numEdges++;
		}

		std::set<const lib::rule::Rule *, lib::rule::LessById> rules;

		for(Vertex v: asRange(vertices(dgGraph))) {
			if(dgGraph[v].kind != lib::DG::HyperVertexKind::Edge) continue;
			for(const auto *r: dgHyper.getRulesFromEdge(v))
				rules.insert(r);
		}

		unsigned int dgNumVertices, dgNumEdges, dgNumRules;
		PARSE("DG::numVertices:" >> x3::uint_, dgNumVertices);
		PARSE("DG::numEdges:" >> x3::uint_, dgNumEdges);
		PARSE("DG::numRules:" >> x3::uint_, dgNumRules);
		if(dgNumVertices != numVertices) {
			err << "Incompatible DG (numVertices): loaded = "
			    << dgNumVertices << " != " << numVertices << " = provided dg" << std::endl;
			FAIL;
		}
		if(dgNumEdges != numEdges) {
			err << "Incompatible DG (numEdges): loaded = "
			    << dgNumEdges << " != " << numEdges << " = provided dg" << std::endl;
			FAIL;
		}
		if(dgNumRules != rules.size()) {
			err << "Incompatible DG (numRules): loaded = "
			    << dgNumRules << " != " << rules.size() << " = provided dg" << std::endl;
			FAIL;
		}
		PARSE("DG::vertices:", x3::unused);
		for(unsigned int i = 0; i < dgNumVertices; i++) {
			unsigned int vId;
			PARSE(x3::uint_, vId);
			Vertex v = vertex(vId, dgGraph);
			assert(get(boost::vertex_index_t(), dgGraph, v) == vId);
			if(dgGraph[v].kind != lib::DG::HyperVertexKind::Vertex) {
				err << "Incompatible DG (vertices): id " << vId << " is not a vertex." << std::endl;
				FAIL;
			}
		}
		PARSE("DG::edges:", x3::unused);
		for(unsigned int i = 0; i < dgNumEdges; i++) {
			unsigned int vId;
			PARSE(x3::uint_, vId);
			Vertex v = vertex(vId, dgGraph);
			assert(get(boost::vertex_index_t(), dgGraph, v) == vId);
			if(dgGraph[v].kind != lib::DG::HyperVertexKind::Edge) {
				err << "Incompatible DG (edges): id " << vId << " is not an edge." << std::endl;
				FAIL;
			}
		}
	} // end of DG stats check

	switch(version) {
	case 0:
	case 1:
		throw InputError("Flow dump version " + std::to_string(version) +
		                 " is too old. Please load and dump it with an old version of MØD.");
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
		return loadVersion_2_3_4_5_6_7(dg, ilpSolver, textFirst, first, last, textLast, version, err, verbosity);
	default:
		throw InputError("Unknown Flow dump version: " + std::to_string(version));
	}
}

} // namespace mod::lib::HyperFlow::Read