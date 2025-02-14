#include "CompositionMatch.hpp"

#include <mod/rule/GraphInterface.hpp>
#include <mod/lib/RC/MatchBuilder.hpp>

#include <boost/lexical_cast.hpp>

namespace mod::rule {

struct CompositionMatch::Pimpl {
	Pimpl(std::shared_ptr<Rule> rFirst, std::shared_ptr<Rule> rSecond, LabelSettings labelSettings)
			: rFirst(rFirst), rSecond(rSecond), mb(rFirst->getRule(), rSecond->getRule(), labelSettings) {}
public:
	std::shared_ptr<Rule> rFirst, rSecond;
	lib::RC::MatchBuilder mb;
};

CompositionMatch::CompositionMatch(std::shared_ptr<Rule> rFirst, std::shared_ptr<Rule> rSecond,
                                   LabelSettings labelSettings) {
	if(!rFirst) throw LogicError("rFirst is null.");
	if(!rSecond) throw LogicError("rSecond is null.");
	p.reset(new Pimpl(rFirst, rSecond, labelSettings));
}

CompositionMatch::CompositionMatch(const CompositionMatch &other) : p(new Pimpl(*other.p)) {}

CompositionMatch &CompositionMatch::operator=(const CompositionMatch &other) {
	p.reset(new Pimpl(*other.p));
	return *this;
}

CompositionMatch::~CompositionMatch() = default;

std::ostream &operator<<(std::ostream &s, const CompositionMatch &mb) {
	return s << "RCMatch(" << *mb.p->rFirst << ", " << *mb.p->rSecond << ", " << mb.getLabelSettings() << ")";
}

std::shared_ptr<Rule> CompositionMatch::getFirst() const {
	return p->rFirst;
}

std::shared_ptr<Rule> CompositionMatch::getSecond() const {
	return p->rSecond;
}

LabelSettings CompositionMatch::getLabelSettings() const {
	return p->mb.labelSettings;
}

int CompositionMatch::size() const {
	return p->mb.size();
}

Rule::LeftGraph::Vertex CompositionMatch::operator[](Rule::RightGraph::Vertex vFirst) const {
	if(!vFirst) throw LogicError("Can not get mapped vertex for a null vertex in the first rule.");
	if(vFirst.getRule() != p->rFirst) throw LogicError("The vertex does not belong to the first rule.");
	const auto &gFirst = p->rFirst->getRule().getGraph();
	const auto &gSecond = p->rSecond->getRule().getGraph();
	const auto vFirstInner = vertices(gFirst).first[vFirst.getId()];
	const auto vSecondInner = p->mb.getSecondFromFirst(vFirstInner);
	if(vSecondInner == boost::graph_traits<lib::rule::GraphType>::null_vertex())
		return {};
	const auto vCore = Rule::Vertex(p->rSecond, get(boost::vertex_index_t(), gSecond, vSecondInner));
	assert(get(boost::vertex_index_t(), gSecond, vSecondInner) == vCore.getId());
	assert(vCore);
	return vCore.getLeft();
}

Rule::RightGraph::Vertex CompositionMatch::operator[](Rule::LeftGraph::Vertex vSecond) const {
	if(!vSecond) throw LogicError("Can not get mapped vertex for a null vertex in the second rule.");
	if(vSecond.getRule() != p->rSecond) throw LogicError("The vertex does not belong to the second rule.");
	const auto &gFirst = p->rFirst->getRule().getGraph();
	const auto &gSecond = p->rSecond->getRule().getGraph();
	const auto vSecondInner = vertices(gSecond).first[vSecond.getId()];
	const auto vFirstInner = p->mb.getFirstFromSecond(vSecondInner);
	if(vFirstInner == boost::graph_traits<lib::rule::GraphType>::null_vertex())
		return {};
	const auto vCore = Rule::Vertex(p->rFirst, get(boost::vertex_index_t(), gFirst, vFirstInner));
	assert(get(boost::vertex_index_t(), gFirst, vFirstInner) == vCore.getId());
	assert(vCore);
	return vCore.getRight();
}

void CompositionMatch::push(Rule::RightGraph::Vertex vFirst, Rule::LeftGraph::Vertex vSecond) {
	if(!vFirst) throw LogicError("Can not add null vertex as the first component to the match.");
	if(!vSecond) throw LogicError("Can not add null vertex as the second component to the match.");
	if(vFirst.getRule() != p->rFirst) throw LogicError("The first vertex does not belong to the first rule.");
	if(vSecond.getRule() != p->rSecond) throw LogicError("The second vertex does not belong to the second rule.");
	if(auto v = (*this)[vFirst])
		throw LogicError("First vertex already mapped, to " + boost::lexical_cast<std::string>(v));
	if(auto v = (*this)[vSecond])
		throw LogicError("Second vertex already mapped, to " + boost::lexical_cast<std::string>(v));
	const auto &gFirst = p->rFirst->getRule().getGraph();
	const auto &gSecond = p->rSecond->getRule().getGraph();
	const auto vFirstInner = vertices(gFirst).first[vFirst.getId()];
	const auto vSecondInner = vertices(gSecond).first[vSecond.getId()];
	const bool res = p->mb.push(vFirstInner, vSecondInner);
	if(!res)
		throw LogicError("Match extension infeasible ("
		                 + boost::lexical_cast<std::string>(vFirst) + " <-> "
		                 + boost::lexical_cast<std::string>(vSecond) + ").");
}

void CompositionMatch::pop() {
	if(size() == 0) throw LogicError("Can not pop from empty match.");
	p->mb.pop();
}

std::shared_ptr<Rule> CompositionMatch::compose(bool verbose) const {
	auto inner = p->mb.compose(verbose);
	if(inner) return rule::Rule::makeRule(std::move(inner));
	else return nullptr;
}

std::vector<std::shared_ptr<Rule>> CompositionMatch::composeAll(bool maximum, bool verbose) const {
	std::vector<std::shared_ptr<Rule>> res;
	auto inners = p->mb.composeAll(maximum, verbose);
	res.reserve(inners.size());
	for(auto &&inner : inners)
		res.push_back(rule::Rule::makeRule(std::move(inner)));
	return res;
}

} // namespace mod::rule