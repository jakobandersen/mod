#include "DG.hpp"

#include <mod/Error.hpp>
#include <mod/dg/Builder.hpp>
#include <mod/dg/GraphInterface.hpp>
#include <mod/dg/Printer.hpp>
#include <mod/graph/Printer.hpp>
#include <mod/lib/DG/Hyper.hpp>
#include <mod/lib/DG/NonHyper.hpp>
#include <mod/lib/DG/NonHyperBuilder.hpp>
#include <mod/lib/DG/Strategies/GraphState.hpp>
#include <mod/lib/DG/Strategies/Strategy.hpp>
#include <mod/lib/DG/IO/Read.hpp>
#include <mod/lib/DG/IO/Write.hpp>
#include <mod/lib/Graph/Single.hpp>
#include <mod/lib/IO/IO.hpp>

#include <boost/lexical_cast.hpp>

#include <iostream>

namespace mod::dg {

//------------------------------------------------------------------------------
// DG
//------------------------------------------------------------------------------

struct DG::Pimpl {
	Pimpl(std::unique_ptr<lib::DG::NonHyper> dg) : dg(std::move(dg)) {}
public:
	const std::unique_ptr<lib::DG::NonHyper> dg;
};

DG::DG(std::unique_ptr<lib::DG::NonHyper> dg) : p(new Pimpl(std::move(dg))) {}

DG::~DG() = default;

std::size_t DG::getId() const {
	return p->dg->getId();
}

std::ostream &operator<<(std::ostream &s, const DG &dg) {
	return s << dg.p->dg->getType() << "(" << dg.p->dg->getId() << ")";
}

const lib::DG::NonHyper &DG::getNonHyper() const {
	return *p->dg;
}

const lib::DG::Hyper &DG::getHyper() const {
	return getNonHyper().getHyper();
}

LabelSettings DG::getLabelSettings() const {
	return getNonHyper().getLabelSettings();
}

//------------------------------------------------------------------------------

bool DG::hasActiveBuilder() const {
	return p->dg->getHasStartedCalculation() && !p->dg->getHasCalculated();
}

bool DG::isLocked() const {
	return p->dg->getHasCalculated();
}

//------------------------------------------------------------------------------

std::size_t DG::numVertices() const {
	if(!(hasActiveBuilder() || isLocked()))
		throw LogicError("The DG neither has an active builder nor is locked yet.");
	using boost::vertices;
	auto vs = vertices(getHyper().getGraph());
	return std::count_if(vs.first, vs.second, [this](lib::DG::HyperVertex v) {
		return getHyper().getGraph()[v].kind == lib::DG::HyperVertexKind::Vertex;
	});
}

DG::VertexRange DG::vertices() const {
	if(!(hasActiveBuilder() || isLocked()))
		throw LogicError("The DG neither has an active builder nor is locked yet.");
	return VertexRange(getNonHyper().getAPIReference());
}

std::size_t DG::numEdges() const {
	if(!(hasActiveBuilder() || isLocked()))
		throw LogicError("The DG neither has an active builder nor is locked yet.");
	using boost::vertices;
	auto vs = vertices(getHyper().getGraph());
	return std::count_if(vs.first, vs.second, [this](lib::DG::HyperVertex v) {
		return getHyper().getGraph()[v].kind == lib::DG::HyperVertexKind::Edge;
	});
}

DG::EdgeRange DG::edges() const {
	if(!(hasActiveBuilder() || isLocked()))
		throw LogicError("The DG neither has an active builder nor is locked yet.");
	return EdgeRange(getNonHyper().getAPIReference());
}

//------------------------------------------------------------------------------

DG::Vertex DG::findVertex(std::shared_ptr<graph::Graph> g) const {
	if(!(hasActiveBuilder() || isLocked()))
		throw LogicError("The DG neither has an active builder nor is locked yet.");
	if(!g) throw LogicError("The graph is a nullptr.");
	const auto &dg = getHyper();
	const auto v = dg.getVertexOrNullFromGraph(&g->getGraph());
	return dg.getInterfaceVertex(v);
}

DG::HyperEdge DG::findEdge(const std::vector<Vertex> &sources, const std::vector<Vertex> &targets) const {
	if(!(hasActiveBuilder() || isLocked()))
		throw LogicError("The DG neither has an active builder nor is locked yet.");
	using boost::vertices;
	const auto &dg = getHyper().getGraph();
	const auto vs = vertices(dg).first;
	std::vector<lib::DG::Hyper::Vertex> vSources, vTargets;
	for(auto v : sources) {
		if(v.isNull()) throw LogicError("Source vertex descriptor is null.");
		if(v.getDG() != getNonHyper().getAPIReference())
			throw LogicError("Source vertex descriptor does not belong to this derivation graph: "
			                 + boost::lexical_cast<std::string>(v));
		vSources.push_back(vs[v.getId()]);
	}
	for(auto v : targets) {
		if(v.isNull()) throw LogicError("Target vertex descriptor is null.");
		if(v.getDG() != getNonHyper().getAPIReference())
			throw LogicError("Target vertex descriptor does not belong to this derivation graph: "
			                 + boost::lexical_cast<std::string>(v));
		vTargets.push_back(vs[v.getId()]);
	}
	const auto vInner = getNonHyper().findHyperEdge(vSources, vTargets);
	return getHyper().getInterfaceEdge(vInner);
}

DG::HyperEdge DG::findEdge(const std::vector<std::shared_ptr<graph::Graph> > &sources,
                           const std::vector<std::shared_ptr<graph::Graph> > &targets) const {
	if(!(hasActiveBuilder() || isLocked()))
		throw LogicError("The DG neither has an active builder nor is locked yet.");
	std::vector<Vertex> vSources, vTargets;
	for(auto g : sources) vSources.push_back(findVertex(g));
	for(auto g : targets) vTargets.push_back(findVertex(g));
	return findEdge(vSources, vTargets);
}

//------------------------------------------------------------------------------

Builder DG::build() {
	return build(nullptr, nullptr);
}

Builder DG::build(std::shared_ptr<Function<void(dg::DG::Vertex)>> onNewVertex,
                  std::shared_ptr<Function<void(dg::DG::HyperEdge)>> onNewHyperEdge) {
	if(isLocked())
		throw LogicError("The DG is locked.");
	if(hasActiveBuilder())
		throw LogicError("Another build is already in progress.");
	if(auto *ptr = dynamic_cast<lib::DG::NonHyperBuilder *> (p->dg.get())) {
		return Builder(*ptr, onNewVertex, onNewHyperEdge);
	} else throw LogicError("Only DGs from DG::builder can be built\n");
}

const std::vector<std::shared_ptr<graph::Graph>> &DG::getGraphDatabase() const {
	return getNonHyper().getGraphDatabase().asList();
}

const std::vector<std::shared_ptr<graph::Graph> > &DG::getProducts() const {
	return getNonHyper().getProducts();
}

std::pair<std::string, std::string> DG::print(const Printer &printer, const PrintData &data) const {
	if(data.getDG() != getNonHyper().getAPIReference()) {
		std::ostringstream err;
		err << "PrintData is for another derivation graph (id=" << data.getDG()->getId()
		    << ") than this (id=" << getId() << ")" << std::endl;
		throw LogicError(err.str());
	}
	return lib::DG::Write::summary(data.getData(), printer.getPrinter(), printer.getGraphPrinter().getOptions());
}

std::string DG::printNonHyper() const {
	return lib::DG::Write::summaryNonHyper(getNonHyper());
}

std::string DG::dump() const {
	return dump("");
}

std::string DG::dump(const std::string &filename) const {
	if(!isLocked()) throw LogicError("Can not dump DG before it is locked.");
	if(filename.empty()) {
		std::string name = lib::IO::makeUniqueFilePrefix() + "DG.dg";
		lib::IO::writeJsonFile(name, lib::DG::Write::dumpToJson(getNonHyper()));
		return name;
	} else {
		lib::IO::writeJsonFile(filename, lib::DG::Write::dumpToJson(getNonHyper()));
		return filename;
	}
}

void DG::listStats() const {
	if(!isLocked()) throw LogicError("No stats can be printed before calculation.");
	else p->dg->getHyper().printStats(std::cout);
}

//------------------------------------------------------------------------------
// Static
//------------------------------------------------------------------------------

namespace {

std::shared_ptr<DG> wrapIt(DG *dgPtr) {
	std::shared_ptr<DG> dg(dgPtr);
	const lib::DG::NonHyper &cNonHyper = dg->getNonHyper();
	const_cast<lib::DG::NonHyper &> (cNonHyper).setAPIReference(dg);
	return dg;
}

} // namespace

std::shared_ptr<DG> DG::make(LabelSettings labelSettings,
                             const std::vector<std::shared_ptr<graph::Graph> > &graphDatabase,
                             IsomorphismPolicy graphPolicy) {
	if(std::any_of(graphDatabase.begin(), graphDatabase.end(), [](const auto &g) {
		return !g;
	}))
		throw LogicError("Nullptr in graph database.");
	auto dgInternal = std::make_unique<lib::DG::NonHyperBuilder>(labelSettings, graphDatabase, graphPolicy);
	return wrapIt(new DG(std::move(dgInternal)));
}

std::shared_ptr<DG> DG::load(const std::vector<std::shared_ptr<graph::Graph>> &graphDatabase,
                             const std::vector<std::shared_ptr<rule::Rule>> &ruleDatabase,
                             const std::string &file,
                             IsomorphismPolicy graphPolicy) {
	return load(graphDatabase, ruleDatabase, file, graphPolicy, 2);
}

std::shared_ptr<DG> DG::load(const std::vector<std::shared_ptr<graph::Graph>> &graphDatabase,
                             const std::vector<std::shared_ptr<rule::Rule>> &ruleDatabase,
                             const std::string &file,
                             IsomorphismPolicy graphPolicy, int verbosity) {
	if(std::any_of(graphDatabase.begin(), graphDatabase.end(), [](const auto &g) {
		return !g;
	})) {
		throw LogicError("Nullptr in graph database.");
	}
	if(std::any_of(ruleDatabase.begin(), ruleDatabase.end(), [](const auto &r) {
		return !r;
	})) {
		throw LogicError("Nullptr in rule database.");
	}

	std::ostringstream err;
	std::unique_ptr<lib::DG::NonHyper> dgInternal(
			lib::DG::Read::dump(graphDatabase, ruleDatabase, file, graphPolicy, err, verbosity));
	if(!dgInternal) throw InputError("DG load error: " + err.str());
	return wrapIt(new DG(std::move(dgInternal)));
}

void DG::diff(std::shared_ptr<DG> dg1, std::shared_ptr<DG> dg2) {
	lib::DG::NonHyper::diff(dg1->getNonHyper(), dg2->getNonHyper());
}

} // namespace mod::dg