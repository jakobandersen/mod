#ifndef MOD_LIB_RULES_PROP_HPP
#define MOD_LIB_RULES_PROP_HPP

#include <mod/Error.hpp>
#include <mod/lib/DPO/CombinedRule.hpp>
#include <mod/lib/DPO/Membership.hpp>
#include <mod/lib/LabelledGraph.hpp>
#include <mod/lib/IO/IO.hpp>

#include <jla_boost/graph/PairToRangeAdaptor.hpp>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/optional/optional.hpp>

#include <vector>

namespace mod::lib::rule {
using lib::DPO::Membership;

#define MOD_RULE_PROP_TEMPLATE_PARAMS typename Derived, typename VertexProp, typename EdgeProp
#define MOD_RULE_PROP_TEMPLATE_ARGS   Derived, VertexProp, EdgeProp

template<typename Derived, typename VertexPropT, typename EdgePropT>
struct PropBase {
	using RuleType = lib::DPO::CombinedRule;
public:
	using VertexProp = VertexPropT;
	using EdgeProp = EdgePropT;
public:
	struct Side {
		using RuleType = lib::DPO::CombinedRule;
	public:
		const VertexProp &operator[](RuleType::SideVertex v) const;
		const EdgeProp &operator[](RuleType::SideEdge e) const;

		friend auto get(const Side &p, const RuleType::SideVertex &v) -> decltype(p[v]) { return p[v]; }

		friend auto get(const Side &p, const RuleType::SideEdge &e) -> decltype(p[e]) { return p[e]; }

	public:
		const PropBase &p;
		const std::vector<VertexProp> &pV;
		const std::vector<EdgeProp> &pE;
		const RuleType::SideGraphType &g;
	public:
		using Handler = IdentityPropertyHandler;
	};
public:
	using ValueTypeVertex = std::pair<boost::optional<const VertexProp &>, boost::optional<const VertexProp &>>;
	using ValueTypeEdge = std::pair<boost::optional<const EdgeProp &>, boost::optional<const EdgeProp &>>;
public:
	void verify() const;

	explicit PropBase(const RuleType &rule) : rule(rule) {}

	Side getLeft() const { return {*this, vPropL, ePropL, getL(rule)}; }

	Side getRight() const { return {*this, vPropR, ePropR, getR(rule)}; }

public:
	void invert();
public: // vertex
	void addL(RuleType::SideVertex v, VertexProp p);
	void addR(RuleType::SideVertex v, VertexProp p);
	void addK(RuleType::KVertex v, VertexProp pL, VertexProp pR);
	void promoteL(RuleType::SideVertex vL, RuleType::SideVertex vR, VertexProp pR);
public: // edge
	void addL(RuleType::SideEdge e, EdgeProp p);
	void addR(RuleType::SideEdge e, EdgeProp p);
	void addK(RuleType::KEdge e, EdgeProp pL, EdgeProp pR);
public:
	ValueTypeVertex operator[](RuleType::CombinedVertex v) const;
	ValueTypeEdge operator[](RuleType::CombinedEdge e) const;
public:
	friend auto get(const PropBase &p, RuleType::CombinedVertex v) -> decltype(p[v]) { return p[v]; }

	friend auto get(const PropBase &p, RuleType::CombinedEdge e) -> decltype(p[e]) { return p[e]; }

public: // old stuff not yet fully evaluated
	void add(RuleType::CombinedVertex v, const VertexProp &valueLeft, const VertexProp &valueRight);
	void add(RuleType::CombinedEdge e, const EdgeProp &valueLeft, const EdgeProp &valueRight);
	// does not modify the other side
	void setLeft(RuleType::CombinedVertex v, const VertexProp &value);
	void setRight(RuleType::CombinedVertex v, const VertexProp &value);
	void setLeft(RuleType::CombinedEdge e, const EdgeProp &value);
	void setRight(RuleType::CombinedEdge e, const EdgeProp &value);
public: // is updated
	bool isChanged(RuleType::CombinedVertex v) const;
	bool isChanged(RuleType::CombinedEdge e) const;
	void print(std::ostream &s, RuleType::CombinedVertex v) const;
	void print(std::ostream &s, RuleType::CombinedEdge e) const;
	const Derived &getDerived() const;
protected:
	const RuleType &rule;
protected:
	std::vector<VertexProp> vPropL, vPropR;
	std::vector<EdgeProp> ePropL, ePropR;
public:
	struct Handler {
		template<typename VEProp, typename LabGraphDom, typename LabGraphCodom, typename F, typename ...Args>
		static auto fmap2(const VEProp &l, const VEProp &r, const LabGraphDom &gDom, const LabGraphCodom &gCodom, F &&f,
		                  Args &&... args) {
			assert(l.first.is_initialized() == r.first.is_initialized());
			assert(l.second.is_initialized() == r.second.is_initialized());
			using First = decltype(f(*l.first, *r.first, get_labelled_left(gDom), get_labelled_left(gCodom), args...));
			using Second = decltype(f(*l.second, *r.second, get_labelled_left(gDom), get_labelled_left(gCodom), args...));
			return std::pair<boost::optional<First>, boost::optional<Second> >(
					l.first.is_initialized()
					? f(*l.first, *r.first, get_labelled_left(gDom), get_labelled_left(gCodom), args...)
					: boost::optional<First>(),
					l.second.is_initialized()
					? f(*l.second, *r.second, get_labelled_left(gDom), get_labelled_left(gCodom), args...)
					: boost::optional<Second>()
			);
		}

		template<typename Op, typename Val>
		static auto reduce(Op &&op, Val &&val) {
			assert(val.first.is_initialized() || val.second.is_initialized());
			if(!val.first.is_initialized()) return *val.second;
			if(!val.second.is_initialized()) return *val.first;
			return op(*val.first, *val.second);
		}
	};
};

//------------------------------------------------------------------------------
// Implementation
//------------------------------------------------------------------------------

template<MOD_RULE_PROP_TEMPLATE_PARAMS>
const VertexProp &PropBase<MOD_RULE_PROP_TEMPLATE_ARGS>::Side::operator[](RuleType::SideVertex v) const {
	const auto vId = get(boost::vertex_index_t(), g, v);
	assert(vId < pV.size());
	return pV[vId];
}

template<MOD_RULE_PROP_TEMPLATE_PARAMS>
const EdgeProp &PropBase<MOD_RULE_PROP_TEMPLATE_ARGS>::Side::operator[](RuleType::SideEdge e) const {
	const auto eId = get(boost::edge_index_t(), g, e);
	assert(eId < pE.size());
	return pE[eId];
}

namespace detail {

void PropVerify(const lib::DPO::CombinedRule &rule,
                std::size_t vLSize, std::size_t vRSize,
                std::size_t eLSize, std::size_t eRSize);

} // namespace detail

template<MOD_RULE_PROP_TEMPLATE_PARAMS>
void PropBase<MOD_RULE_PROP_TEMPLATE_ARGS>::verify() const {
	// dispatch to a non-templated function so the iostream stuff is not in a header
	detail::PropVerify(rule, vPropL.size(), vPropR.size(), ePropL.size(), ePropR.size());
}

// ===============================================================================================

template<MOD_RULE_PROP_TEMPLATE_PARAMS>
void PropBase<MOD_RULE_PROP_TEMPLATE_ARGS>::invert() {
	using std::swap;
	swap(vPropL, vPropR);
	swap(ePropL, ePropR);
}

template<MOD_RULE_PROP_TEMPLATE_PARAMS>
void PropBase<MOD_RULE_PROP_TEMPLATE_ARGS>::addL(RuleType::SideVertex v, VertexProp p) {
	assert(num_vertices(getL(rule)) == vPropL.size() + 1);
	assert(get(boost::vertex_index_t(), getL(rule), v) == vPropL.size());
	vPropL.push_back(std::move(p));
	vPropR.emplace_back();
	verify();
}

template<MOD_RULE_PROP_TEMPLATE_PARAMS>
void PropBase<MOD_RULE_PROP_TEMPLATE_ARGS>::addR(RuleType::SideVertex v, VertexProp p) {
	assert(num_vertices(getR(rule)) == vPropR.size() + 1);
	assert(get(boost::vertex_index_t(), getR(rule), v) == vPropR.size());
	vPropL.emplace_back();
	vPropR.push_back(std::move(p));
	verify();
}

template<MOD_RULE_PROP_TEMPLATE_PARAMS>
void PropBase<MOD_RULE_PROP_TEMPLATE_ARGS>::addK(RuleType::KVertex vK, VertexProp pL, VertexProp pR) {
	assert(num_vertices(getL(rule)) == vPropL.size() + 1);
	assert(num_vertices(getR(rule)) == vPropR.size() + 1);
	assert(get(boost::vertex_index_t(), getL(rule),
	           get(getMorL(rule), getK(rule), getL(rule), vK))
	       == vPropL.size());
	assert(get(boost::vertex_index_t(), getR(rule),
	           get(getMorR(rule), getK(rule), getR(rule), vK))
	       == vPropR.size());
	vPropL.push_back(std::move(pL));
	vPropR.push_back(std::move(pR));
	verify();
}

template<MOD_RULE_PROP_TEMPLATE_PARAMS>
void PropBase<MOD_RULE_PROP_TEMPLATE_ARGS>::promoteL(RuleType::SideVertex vL, RuleType::SideVertex vR, VertexProp pR) {
	assert(vL == vR);
	const auto vRId = get(boost::vertex_index_t(), getR(rule), vR);
	vPropR[vRId] = std::move(pR);
	verify();
}

template<MOD_RULE_PROP_TEMPLATE_PARAMS>
void PropBase<MOD_RULE_PROP_TEMPLATE_ARGS>::addL(RuleType::SideEdge e, EdgeProp p) {
	assert(num_edges(getL(rule)) == ePropL.size() + 1);
	assert(get(boost::edge_index_t(), getL(rule), e) == ePropL.size());
	ePropL.push_back(std::move(p));
	ePropR.emplace_back();
	verify();
}

template<MOD_RULE_PROP_TEMPLATE_PARAMS>
void PropBase<MOD_RULE_PROP_TEMPLATE_ARGS>::addR(RuleType::SideEdge e, EdgeProp p) {
	assert(num_edges(getR(rule)) == ePropR.size() + 1);
	assert(get(boost::edge_index_t(), getR(rule), e) == ePropR.size());
	ePropL.emplace_back();
	ePropR.push_back(std::move(p));
	verify();
}

template<MOD_RULE_PROP_TEMPLATE_PARAMS>
void PropBase<MOD_RULE_PROP_TEMPLATE_ARGS>::addK(RuleType::KEdge eK, EdgeProp pL, EdgeProp pR) {
	assert(num_edges(getL(rule)) == ePropL.size() + 1);
	assert(num_edges(getR(rule)) == ePropR.size() + 1);
	assert(get(boost::edge_index_t(), getL(rule),
	           get(getMorL(rule), getK(rule), getL(rule), eK))
	       == ePropL.size());
	assert(get(boost::edge_index_t(), getR(rule),
	           get(getMorR(rule), getK(rule), getR(rule), eK))
	       == ePropR.size());
	ePropL.push_back(std::move(pL));
	ePropR.push_back(std::move(pR));
	verify();
}

// ===============================================================================================

template<MOD_RULE_PROP_TEMPLATE_PARAMS>
std::pair<boost::optional<const VertexProp &>, boost::optional<const VertexProp &>>
PropBase<MOD_RULE_PROP_TEMPLATE_ARGS>::operator[](RuleType::CombinedVertex v) const {
	assert(v != boost::graph_traits<RuleType::CombinedGraphType>::null_vertex());
	boost::optional<const VertexProp &> l;
	boost::optional<const VertexProp &> r;
	if(rule.getCombinedGraph()[v].membership != Membership::R)
		l = getLeft()[get_inverse(rule.getLtoCG(), getL(rule), rule.getCombinedGraph(), v)];
	if(rule.getCombinedGraph()[v].membership != Membership::L)
		r = getRight()[get_inverse(rule.getRtoCG(), getR(rule), rule.getCombinedGraph(), v)];
	return {l, r};
}

template<MOD_RULE_PROP_TEMPLATE_PARAMS>
std::pair<boost::optional<const EdgeProp &>, boost::optional<const EdgeProp &>>
PropBase<MOD_RULE_PROP_TEMPLATE_ARGS>::operator[](RuleType::CombinedEdge e) const {
	boost::optional<const EdgeProp &> l;
	boost::optional<const EdgeProp &> r;
	const auto &cg = rule.getCombinedGraph();
	if(cg[e].membership != Membership::R)
		l = getLeft()[get_inverse(rule.getLtoCG(), getL(rule), cg, e)];
	if(cg[e].membership != Membership::L)
		r = getRight()[get_inverse(rule.getRtoCG(), getR(rule), cg, e)];
	return {l, r};
}


template<MOD_RULE_PROP_TEMPLATE_PARAMS>
void PropBase<MOD_RULE_PROP_TEMPLATE_ARGS>::add(RuleType::CombinedVertex v, const VertexProp &valueLeft,
                                                const VertexProp &valueRight) {
	assert(num_vertices(rule.getCombinedGraph()) == vPropL.size() + 1);
	assert(num_vertices(rule.getCombinedGraph()) == vPropR.size() + 1);
	assert(get(boost::vertex_index_t(), rule.getCombinedGraph(), v) == vPropL.size());
	assert(get(boost::vertex_index_t(), rule.getCombinedGraph(), v) == vPropR.size());
	vPropL.push_back(valueLeft);
	vPropR.push_back(valueRight);
	verify();
}

template<MOD_RULE_PROP_TEMPLATE_PARAMS>
void PropBase<MOD_RULE_PROP_TEMPLATE_ARGS>::add(RuleType::CombinedEdge e, const EdgeProp &valueLeft,
                                                const EdgeProp &valueRight) {
	assert(num_edges(rule.getCombinedGraph()) == ePropL.size() + 1);
	assert(num_edges(rule.getCombinedGraph()) == ePropR.size() + 1);
	assert(get(boost::edge_index_t(), rule.getCombinedGraph(), e) == ePropL.size());
	assert(get(boost::edge_index_t(), rule.getCombinedGraph(), e) == ePropR.size());
	ePropL.push_back(valueLeft);
	ePropR.push_back(valueRight);
	verify();
}

template<MOD_RULE_PROP_TEMPLATE_PARAMS>
void PropBase<MOD_RULE_PROP_TEMPLATE_ARGS>::setLeft(RuleType::CombinedVertex v, const VertexProp &value) {
	const auto vId = get(boost::vertex_index_t(), rule.getCombinedGraph(), v);
	assert(vId < vPropL.size());
	assert(rule.getCombinedGraph()[v].membership != Membership::R);
	vPropL[vId] = value;
	verify();
}

template<MOD_RULE_PROP_TEMPLATE_PARAMS>
void PropBase<MOD_RULE_PROP_TEMPLATE_ARGS>::setRight(RuleType::CombinedVertex v, const VertexProp &value) {
	const auto vId = get(boost::vertex_index_t(), rule.getCombinedGraph(), v);
	assert(vId < vPropR.size());
	assert(rule.getCombinedGraph()[v].membership != Membership::L);
	vPropR[vId] = value;
	verify();
}

template<MOD_RULE_PROP_TEMPLATE_PARAMS>
void PropBase<MOD_RULE_PROP_TEMPLATE_ARGS>::setLeft(RuleType::CombinedEdge e, const EdgeProp &value) {
	const auto eId = get(boost::edge_index_t(), rule.getCombinedGraph(), e);
	assert(eId < ePropL.size());
	assert(rule.getCombinedGraph()[e].membership != Membership::R);
	ePropL[eId] = value;
	verify();
}

template<MOD_RULE_PROP_TEMPLATE_PARAMS>
void PropBase<MOD_RULE_PROP_TEMPLATE_ARGS>::setRight(RuleType::CombinedEdge e, const EdgeProp &value) {
	const auto eId = get(boost::edge_index_t(), rule.getCombinedGraph(), e);
	assert(eId < ePropR.size());
	assert(rule.getCombinedGraph()[e].membership != Membership::L);
	ePropR[eId] = value;
	verify();
}

template<MOD_RULE_PROP_TEMPLATE_PARAMS>
bool PropBase<MOD_RULE_PROP_TEMPLATE_ARGS>::isChanged(RuleType::CombinedVertex v) const {
	if(rule.getCombinedGraph()[v].membership != Membership::K) return false;
	const auto vL = get_inverse(rule.getLtoCG(), getL(rule), rule.getCombinedGraph(), v);
	const auto vR = get_inverse(rule.getRtoCG(), getR(rule), rule.getCombinedGraph(), v);
	const auto vLid = get(boost::vertex_index_t(), getL(rule), vL);
	const auto vRid = get(boost::vertex_index_t(), getR(rule), vR);
	assert(vLid < vPropL.size());
	assert(vRid < vPropR.size());
	return vPropL[vLid] != vPropR[vRid];
}

template<MOD_RULE_PROP_TEMPLATE_PARAMS>
bool PropBase<MOD_RULE_PROP_TEMPLATE_ARGS>::isChanged(RuleType::CombinedEdge e) const {
	if(rule.getCombinedGraph()[e].membership != Membership::K) return false;
	const auto eL = get_inverse(rule.getLtoCG(), getL(rule), rule.getCombinedGraph(), e);
	const auto eR = get_inverse(rule.getRtoCG(), getR(rule), rule.getCombinedGraph(), e);
	const auto eLid = get(boost::edge_index_t(), getL(rule), eL);
	const auto eRid = get(boost::edge_index_t(), getR(rule), eR);
	assert(eLid < ePropL.size());
	assert(eRid < ePropR.size());
	return ePropL[eLid] != ePropR[eRid];
}

template<MOD_RULE_PROP_TEMPLATE_PARAMS>
void PropBase<MOD_RULE_PROP_TEMPLATE_ARGS>::print(std::ostream &s, RuleType::CombinedVertex v) const {
	const auto membership = rule.getCombinedGraph()[v].membership;
	if(membership == Membership::R) s << "<>";
	else {
		const auto vL = get_inverse(rule.getLtoCG(), getL(rule), rule.getCombinedGraph(), v);
		const auto vLid = get(boost::vertex_index_t(), getL(rule), vL);
		assert(vLid < vPropL.size());
		s << '\'' << vPropL[vLid] << '\'';
	}
	s << " -> ";
	if(membership == Membership::L) s << "<>";
	else {
		const auto vR = get_inverse(rule.getRtoCG(), getR(rule), rule.getCombinedGraph(), v);
		const auto vRid = get(boost::vertex_index_t(), getR(rule), vR);
		assert(vRid < vPropR.size());
		s << '\'' << vPropR[vRid] << '\'';
	}
}

template<MOD_RULE_PROP_TEMPLATE_PARAMS>
void PropBase<MOD_RULE_PROP_TEMPLATE_ARGS>::print(std::ostream &s, RuleType::CombinedEdge e) const {
	const auto membership = rule.getCombinedGraph()[e].membership;
	if(membership == Membership::R) s << "<>";
	else {
		const auto eL = get_inverse(rule.getLtoCG(), getL(rule), rule.getCombinedGraph(), e);
		const auto eLid = get(boost::edge_index_t(), getL(rule), eL);
		assert(eLid < ePropL.size());
		s << '\'' << ePropL[eLid] << '\'';
	}
	s << " -> ";
	if(membership == Membership::L) s << "<>";
	else {
		const auto eR = get_inverse(rule.getRtoCG(), getR(rule), rule.getCombinedGraph(), e);
		const auto eRid = get(boost::edge_index_t(), getR(rule), eR);
		assert(eRid < ePropR.size());
		s << '\'' << ePropR[eRid] << '\'';
	}
}

template<MOD_RULE_PROP_TEMPLATE_PARAMS>
const Derived &PropBase<MOD_RULE_PROP_TEMPLATE_ARGS>::getDerived() const {
	return static_cast<const Derived &>(*this);
}

} // namespace mod::lib::rule

#endif // MOD_LIB_RULES_PROP_HPP