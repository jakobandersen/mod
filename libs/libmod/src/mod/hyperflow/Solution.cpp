#include "Solution.hpp"

#include <mod/Misc.hpp>
#include <mod/Post.hpp>
#include <mod/dg/Printer.hpp>
#include <mod/hyperflow/Model.hpp>
#include <mod/hyperflow/Printer.hpp>
#include <mod/graph/Printer.hpp>
#include <mod/lib/CombiOpt/LinExpAny.hpp>
#include <mod/lib/CombiOpt/Solver.hpp>
#include <mod/lib/HyperFlow/Flow.hpp>
#include <mod/lib/HyperFlow/IO/Write.hpp>
#include <mod/lib/IO/IO.hpp>

#include <boost/lexical_cast.hpp>

#include <iostream>

namespace mod::hyperflow {

//------------------------------------------------------------------------------
// SolutionRange
//------------------------------------------------------------------------------

SolutionRange::const_iterator::const_iterator(std::shared_ptr<Model> flow_, int i)
		: flow_(flow_), i(i) {}

bool operator==(const SolutionRange::const_iterator &a, const SolutionRange::const_iterator &b) {
	return std::tie(a.flow_, a.i) == std::tie(b.flow_, b.i);
}

bool operator!=(const SolutionRange::const_iterator &a, const SolutionRange::const_iterator &b) {
	return !(a == b);
}

SolutionRange::const_iterator &SolutionRange::const_iterator::operator++() {
	++i;
	return *this;
}

SolutionRange::const_iterator SolutionRange::const_iterator::operator++(int) {
	const_iterator prev(*this);
	++*this;
	return prev;
}

SolutionRange::const_iterator operator+(const SolutionRange::const_iterator &a, int i) {
	return SolutionRange::const_iterator(a.flow_, a.i + i);
}

Solution SolutionRange::const_iterator::operator*() const {
	return Solution(flow_, i);
}

Solution SolutionRange::const_iterator::operator[](int i) const {
	return *(*this + i);
}

SolutionRange::SolutionRange(std::shared_ptr<Model> model, int first, int last)
		: model(std::move(model)), first(first), last(last) {
	assert(this->model);
	assert(first <= last);
}

std::shared_ptr<Model> SolutionRange::getModel() const {
	return model;
}

std::ostream &operator<<(std::ostream &s, const SolutionRange &r) {
	return s << "HyperflowSolutionRange(model=" << *r.getModel() << ", size=" << r.size() << ")";
}

int SolutionRange::size() const {
	return last - first;
}

SolutionRange::const_iterator SolutionRange::begin() const {
	return const_iterator(model, first);
}

SolutionRange::const_iterator SolutionRange::end() const {
	return const_iterator(model, last);
}

Solution SolutionRange::operator[](int i) const {
	if(i < 0 || i >= size()) throw LogicError("Index " + std::to_string(i) + " out of bounds.");
	return begin()[i];
}

void SolutionRange::list() const {
	auto &s = std::cout;
	s << std::string(70, '-') << '\n';
	s << "Listing solution range with " << size() << " solution";
	if(size() != 1) s << "s";
	s << '\n';
	for(const auto &sol : *this) {
		s << std::string(70, '-') << '\n';
		sol.list();
	}
	s << "End of solution range.\n";
	s << std::string(70, '-') << '\n';
	s << std::flush;
}

namespace {

void checkDG(std::shared_ptr<dg::DG> dg1, std::shared_ptr<dg::DG> dg2) {
	if(dg1 != dg2) {
		std::string text = "Print data is for another derivation graph, "
		                   + boost::lexical_cast<std::string>(*dg1)
		                   + ", than the flow solution, "
		                   + boost::lexical_cast<std::string>(*dg2)
		                   + ".";
		throw LogicError(std::move(text));
	}
}

} // namespace

void SolutionRange::print() const {
	return print(Printer(), dg::PrintData(getModel()->getDG()));
}

void SolutionRange::print(const Printer &printer, const dg::PrintData &data) const {
	// check print data before we do anything
	checkDG(data.getDG(), getModel()->getDG());
	post::summarySection("Hyperflow Solutions, id " + boost::lexical_cast<std::string>(getModel()->getId()));
	for(const auto &sol : *this) sol.print(printer, data);
}

//------------------------------------------------------------------------------
// Solution
//------------------------------------------------------------------------------

Solution::Solution(std::shared_ptr<Model> model, int i) : model(model), i(i) {}

std::shared_ptr<Model> Solution::getModel() const {
	return model;
}

int Solution::getId() const {
	return i;
}

std::ostream &operator<<(std::ostream &s, const Solution &sol) {
	return s << "HyperflowSolution(model=" << *sol.model << ", id=" << sol.i << ")";
}

bool operator==(const Solution &a, const Solution &b) {
	return std::tie(a.model, a.i) == std::tie(b.model, b.i);
}

bool operator!=(const Solution &a, const Solution &b) {
	return std::tie(a.model, a.i) != std::tie(b.model, b.i);
}

bool operator<(const Solution &a, const Solution &b) {
	return std::tie(a.model, a.i) < std::tie(b.model, b.i);
}

std::variant<int, double> Solution::getObjectiveValue() const {
	const auto &sol = model->getFlow().getModel().getSolutions()[i];
	if(model->getFlow().getModel().isObjectiveValueIntegral())
		return static_cast<int>(sol.getObjValInt().getValue());
	else
		return sol.getObjValFloat().getValue();
}

std::variant<int, double> Solution::eval(const LinExp &exp) const {
	const auto expr = model->getFlow().getModel().compileExpr(exp);
	const auto &sol = model->getFlow().getModel().getSolutions()[i];
	const auto res = sol.evaluate(expr);
	return res.getAsImpl();
}

void Solution::list() const {
	const auto &sol = model->getFlow().getModel().getSolutions()[i];
	auto &s = std::cout;
	s << "Solution " << i << " from hyperflow model " << getModel()->getId() << '\n';
	model->getFlow().getModel().listSolution(s, sol);
	s << std::flush;
}

std::pair<std::optional<std::string>, std::optional<std::string>>
Solution::print() const {
	return print(Printer(), dg::PrintData(getModel()->getDG()));
}

std::pair<std::optional<std::string>, std::optional<std::string>>
Solution::print(const Printer &printer, const dg::PrintData &data) const {
	checkDG(data.getDG(), getModel()->getDG());
	return lib::HyperFlow::Write::summary(model->getFlow().getModel(), i, data.getData(),
	                                      printer.getDGPrinter().getPrinter(),
	                                      printer.getDGPrinter().getGraphPrinter().getOptions(),
	                                      printer.getPrinter());
}

} // namespace mod::hyperflow