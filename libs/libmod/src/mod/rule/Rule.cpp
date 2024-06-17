#include "Rule.hpp"

#include <mod/Misc.hpp>
#include <mod/graph/Printer.hpp>
#include <mod/rule/GraphInterface.hpp>
#include <mod/lib/IO/IO.hpp>
#include <mod/lib/Rules/Real.hpp>
#include <mod/lib/Rules/IO/DepictionData.hpp>
#include <mod/lib/Rules/IO/Read.hpp>
#include <mod/lib/Rules/IO/Write.hpp>
#include <mod/lib/Rules/Properties/Molecule.hpp>
#include <mod/lib/Rules/Properties/Stereo.hpp>

#include <boost/iostreams/device/mapped_file.hpp>

#include <fstream>
#include <iostream>
#include <sstream>

namespace mod::rule {

struct Rule::Pimpl {
	Pimpl(std::unique_ptr<lib::Rules::Real> r) : r(std::move(r)) {
		assert(this->r);
	}
public:
	const std::unique_ptr<lib::Rules::Real> r;
	std::unique_ptr<const std::map<int, std::size_t> > externalToInternalIds;
};

Rule::Rule(std::unique_ptr<lib::Rules::Real> r) : p(new Pimpl(std::move(r))) {}

Rule::~Rule() = default;

unsigned int Rule::getId() const {
	return getRule().getId();
}

std::ostream &operator<<(std::ostream &s, const Rule &r) {
	return s << "'" << r.getName() << "'";
}

const lib::Rules::Real &Rule::getRule() const {
	return *p->r.get();
}

//------------------------------------------------------------------------------

std::size_t Rule::numVertices() const {
	return num_vertices(p->r->getDPORule().getRule().getCombinedGraph());
}

Rule::VertexRange Rule::vertices() const {
	return VertexRange(p->r->getAPIReference());
}

std::size_t Rule::numEdges() const {
	return num_edges(p->r->getDPORule().getRule().getCombinedGraph());
}

Rule::EdgeRange Rule::edges() const {
	return EdgeRange(p->r->getAPIReference());
}

Rule::LeftGraph Rule::getLeft() const {
	return LeftGraph(p->r->getAPIReference());
}

Rule::ContextGraph Rule::getContext() const {
	return ContextGraph(p->r->getAPIReference());
}

Rule::RightGraph Rule::getRight() const {
	return RightGraph(p->r->getAPIReference());
}

//------------------------------------------------------------------------------

std::shared_ptr<Rule> Rule::makeInverse() const {
	lib::Rules::LabelledRule dpoRule(getRule().getDPORule(), true);
	if(getConfig().rule.ignoreConstraintsDuringInversion) {
		if(get_match_constraints(get_labelled_left(dpoRule)).size() > 0
		   || get_match_constraints(get_labelled_right(dpoRule)).size() > 0) {
			std::cout << "WARNING: inversion of rule strips constraints.\n";
		}
	} else {
		if(get_match_constraints(get_labelled_left(dpoRule)).size() > 0) {
			throw LogicError("Can not invert rule with left-side component constraints.");
		}
		if(get_match_constraints(get_labelled_right(dpoRule)).size() > 0) {
			throw LogicError("Can not invert rule with right-side component constraints.");
		}
	}
	dpoRule.invert();
	const bool ignore = getConfig().rule.ignoreConstraintsDuringInversion;
	if(ignore) dpoRule.rightData.matchConstraints.clear();
	auto rInner = std::make_unique<lib::Rules::Real>(std::move(dpoRule), getLabelType());
	rInner->setName(this->getName() + ", inverse");
	if(p->externalToInternalIds)
		return makeRule(std::move(rInner), *p->externalToInternalIds);
	else
		return makeRule(std::move(rInner), {});
}

std::pair<std::string, std::string> Rule::print() const {
	return print(false);
}

std::pair<std::string, std::string> Rule::print(bool printCombined) const {
	graph::Printer first;
	graph::Printer second;
	second.setReactionDefault();
	return print(first, second, printCombined);
}

std::pair<std::string, std::string>
Rule::print(const graph::Printer &first, const graph::Printer &second) const {
	return print(first, second, false);
}

std::pair<std::string, std::string>
Rule::print(const graph::Printer &first, const graph::Printer &second, bool printCombined) const {
	return lib::Rules::Write::summary(getRule(), first.getOptions(), second.getOptions(), printCombined);
}

void Rule::printTermState() const {
	lib::Rules::Write::termState(getRule());
}

std::string Rule::getGMLString(bool withCoords) const {
	if(withCoords && !getRule().getDepictionData().getHasCoordinates())
		throw LogicError("Coordinates are not available for this rule (" + getName() + ").");
	std::stringstream ss;
	lib::Rules::Write::gml(getRule(), withCoords, ss);
	return ss.str();
}

std::string Rule::printGML(bool withCoords) const {
	if(withCoords && !getRule().getDepictionData().getHasCoordinates())
		throw LogicError("Coordinates are not available for this rule (" + getName() + ").");
	return lib::Rules::Write::gml(*p->r, withCoords);
}

const std::string &Rule::getName() const {
	return getRule().getName();
}

void Rule::setName(std::string name) {
	p->r->setName(name);
}

std::optional<LabelType> Rule::getLabelType() const {
	return p->r->getLabelType();
}

std::size_t Rule::getNumLeftComponents() const {
	return get_num_connected_components(get_labelled_left(getRule().getDPORule()));
}

std::size_t Rule::getNumRightComponents() const {
	return get_num_connected_components(get_labelled_right(getRule().getDPORule()));
}

namespace {

void checkTermParsing(const lib::Rules::Real &r, LabelSettings ls) {
	if(ls.type == LabelType::Term) {
		const auto &term = get_term(r.getDPORule());
		if(!isValid(term)) {
			std::string msg = "Parsing failed for rule '" + r.getName() + "'. " + term.getParsingError();
			throw TermParsingError(std::move(msg));
		}
	}
}

} // namespace

std::size_t Rule::isomorphism(std::shared_ptr<Rule> r, std::size_t maxNumMatches, LabelSettings labelSettings) const {
	checkTermParsing(this->getRule(), labelSettings);
	checkTermParsing(r->getRule(), labelSettings);
	return lib::Rules::Real::isomorphism(this->getRule(), r->getRule(), maxNumMatches, labelSettings);
}

std::size_t Rule::monomorphism(std::shared_ptr<Rule> r, std::size_t maxNumMatches, LabelSettings labelSettings) const {
	checkTermParsing(this->getRule(), labelSettings);
	checkTermParsing(r->getRule(), labelSettings);
	return lib::Rules::Real::monomorphism(this->getRule(), r->getRule(), maxNumMatches, labelSettings);
}

bool Rule::isomorphicLeftRight(std::shared_ptr<Rule> r, LabelSettings labelSettings) const {
	checkTermParsing(this->getRule(), labelSettings);
	checkTermParsing(r->getRule(), labelSettings);
	return lib::Rules::Real::isomorphicLeftRight(this->getRule(), r->getRule(), labelSettings);
}

Rule::Vertex Rule::getVertexFromExternalId(int id) const {
	if(!p->externalToInternalIds) return Vertex();
	auto iter = p->externalToInternalIds->find(id);
	if(iter == end(*p->externalToInternalIds)) return Vertex();
	return Vertex(p->r->getAPIReference(), iter->second);
}

int Rule::getMinExternalId() const {
	if(!p->externalToInternalIds) return 0;
	return p->externalToInternalIds->begin()->first;
}

int Rule::getMaxExternalId() const {
	if(!p->externalToInternalIds) return 0;
	return (--p->externalToInternalIds->end())->first;
}

//------------------------------------------------------------------------------
// Static
//------------------------------------------------------------------------------

namespace {

std::shared_ptr<Rule> handleLoadedRule(lib::IO::Result<lib::Rules::Read::Data> dataRes,
                                       lib::IO::Warnings warnings,
                                       bool invert,
                                       const std::string &dataSource) {
	std::cout << warnings << std::flush;
	if(!dataRes)
		throw InputError(dataRes.extractError()
		                 + "\nCould not load rule from " + dataSource + ".");
	auto data = std::move(*dataRes);
	assert(data.rule->pString);
	if(invert) {
		if(!get_match_constraints(get_labelled_left(*data.rule)).empty()) {
			const bool ignore = getConfig().rule.ignoreConstraintsDuringInversion;
			std::string msg = "The rule '";
			if(data.name) msg += *data.name;
			else msg += "anon";
			msg += "' from ";
			msg += dataSource;
			msg += " has matching constraints ";
			if(!ignore) {
				msg += "and can not be reversed. Use config.rule.ignoreConstraintsDuringInversion == True to strip constraints.";
				throw InputError(std::move(msg));
			} else {
				msg += "and these will be stripped from the reversed rule.";
				data.rule->leftData.matchConstraints.clear();
				std::cout << "WARNING: " << msg << '\n';
			}
		}
		data.rule->invert();
		if(data.name) *data.name += ", inverse";
	}
	auto libRes = std::make_unique<lib::Rules::Real>(std::move(*data.rule), data.labelType);
	if(data.name) libRes->setName(std::move(*data.name));
	return Rule::makeRule(std::move(libRes), std::move(data.externalToInternalIds));
}

} // namespace

std::shared_ptr<Rule> Rule::fromGMLString(const std::string &data, bool invert, bool printStereoWarnings) {
	lib::IO::Warnings warnings;
	auto res = lib::Rules::Read::gml(warnings, data, printStereoWarnings);
	return handleLoadedRule(std::move(res), std::move(warnings), invert, "<inline GML string>");
}

std::shared_ptr<Rule> Rule::fromGMLFile(const std::string &file, bool invert, bool printStereoWarnings) {
	boost::iostreams::mapped_file_source ifs;
	try {
		ifs.open(file);
	} catch(const BOOST_IOSTREAMS_FAILURE &e) {
		throw InputError("Could not open rule GML file '" + file + "':\n" + e.what());
	}
	if(!ifs) throw InputError("Could not open rule GML file '" + file + "'.\n");
	lib::IO::Warnings warnings;
	auto res = lib::Rules::Read::gml(warnings, {ifs.begin(), ifs.size()}, printStereoWarnings);
	return handleLoadedRule(std::move(res), std::move(warnings), invert, "file '" + file + "'");
}

std::shared_ptr<Rule> Rule::fromDFS(const std::string &data, bool invert) {
	lib::IO::Warnings warnings;
	auto res = lib::Rules::Read::dfs(warnings, data);
	return handleLoadedRule(std::move(res), std::move(warnings), invert, "<inline DFS string>");
}

std::shared_ptr<Rule> Rule::makeRule(std::unique_ptr<lib::Rules::Real> r) {
	return makeRule(std::move(r), {});
}

std::shared_ptr<Rule> Rule::makeRule(std::unique_ptr<lib::Rules::Real> r,
                                     std::map<int, std::size_t> externalToInternalIds) {
	std::shared_ptr<Rule> rWrapped(new Rule(std::move(r)));
	rWrapped->p->r->setAPIReference(rWrapped);
	if(!externalToInternalIds.empty()) {
		rWrapped->p->externalToInternalIds = std::make_unique<std::map<int, std::size_t> >(
				std::move(externalToInternalIds));
	}
	return rWrapped;
}

} // namespace mod::rule