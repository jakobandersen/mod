#ifndef MOD_LIB_RULE_PROP_STEREO_HPP
#define MOD_LIB_RULE_PROP_STEREO_HPP

#include <mod/lib/Stereo/Configuration/Configuration.hpp>
#include <mod/lib/Stereo/EdgeCategory.hpp>
#include <mod/lib/Rule/Properties/Molecule.hpp>

#include <optional>
#include <tuple>

namespace mod::lib::rule {

struct PropStereo
		: private PropBase<PropStereo, std::unique_ptr<const lib::Stereo::Configuration>, lib::Stereo::EdgeCategory> {
	// read-only of data
	using Base = PropBase<PropStereo, std::unique_ptr<const lib::Stereo::Configuration>, lib::Stereo::EdgeCategory>;
	using Base::VertexProp;
	using Base::EdgeProp;
	using Base::Side;

	struct ValueTypeVertex {
		boost::optional<const VertexProp &> left, right;
		bool inContext;
	};

	struct ValueTypeEdge {
		std::optional<EdgeProp> left, right;
		bool inContext;
	};
public:
	template<typename InferenceLeft, typename InferenceRight, typename VertexInContext, typename EdgeInContext>
	PropStereo(const RuleType &rule, InferenceLeft &&leftInference, InferenceRight &&rightInference,
	           VertexInContext vCallback, EdgeInContext eCallback) : Base(rule) {
		// The eCallback is only responsible for the information about being in context.
		// This function will ensure that it's valid to put in context as well.
		// However, the vCallback is also responsible for ensuring that it is valid to put it in context.
		// This function only checks what is easy, i.e., whether the vertex and its incident edges are in context.
		const auto handleVertices = [](const lib::DPO::CombinedRule::SideGraphType &g,
		                               std::vector<VertexProp> &vProp, auto &inference) {
			for(const auto vS: asRange(vertices(g))) {
				assert(get(boost::vertex_index_t(), g, vS) >= vProp.size());
				vProp.resize(get(boost::vertex_index_t(), g, vS));
				assert(get(boost::vertex_index_t(), g, vS) == vProp.size());
				auto p = inference.extractConfiguration(vS);
				{ // verify
#ifndef NDEBUG
					const auto d = degree(vS, g);
					int dConf = 0;
					for(const auto &emb: *p) {
						if(emb.type != lib::Stereo::EmbeddingEdge::Type::Edge) {
							assert(emb.offset >= d);
						} else { // edge
							++dConf;
							assert(emb.offset < d);
						};
					}
					assert(dConf == d);
#endif
				} // end verify
				vProp.push_back(std::move(p));
			}
		};
		vPropL.reserve(num_vertices(getL(rule)));
		vPropR.reserve(num_vertices(getR(rule)));
		vertexInContext.reserve(num_vertices(rule.getCombinedGraph()));
		handleVertices(getL(rule), vPropL, leftInference);
		handleVertices(getR(rule), vPropR, rightInference);
		assert(vPropL.size() <= num_vertices(rule.getCombinedGraph()));
		assert(vPropR.size() <= num_vertices(rule.getCombinedGraph()));
		vPropL.resize(num_vertices(rule.getCombinedGraph()));
		vPropR.resize(num_vertices(rule.getCombinedGraph()));

		const auto &cg = rule.getCombinedGraph();
		for(const auto v: asRange(vertices(cg))) {
			const bool inContext = [&]() {
				if(cg[v].membership != Membership::K) return false;
				if(!vCallback(v)) return false;
				for(const auto eOut: asRange(out_edges(v, rule.getCombinedGraph())))
					if(cg[eOut].membership != Membership::K)
						return false;
				return true;
			}();
			vertexInContext.push_back(inContext);
		}

		const auto handleEdges = [](const lib::DPO::CombinedRule::SideGraphType &g,
		                            std::vector<EdgeProp> &eProp, auto &inference) {
			for(const auto eS: asRange(edges(g))) {
				assert(get(boost::edge_index_t(), g, eS) >= eProp.size());
				eProp.resize(get(boost::edge_index_t(), g, eS));
				assert(get(boost::edge_index_t(), g, eS) == eProp.size());
				auto cat = inference.getEdgeCategory(eS);
				eProp.push_back(cat);
			}
		};
		ePropL.reserve(num_edges(cg));
		ePropR.reserve(num_edges(cg));
		edgeInContext.reserve(num_edges(cg));
		handleEdges(getL(rule), ePropL, leftInference);
		handleEdges(getR(rule), ePropR, rightInference);
		assert(ePropL.size() <= num_edges(rule.getCombinedGraph()));
		assert(ePropR.size() <= num_edges(rule.getCombinedGraph()));
		ePropL.resize(num_edges(rule.getCombinedGraph()));
		ePropR.resize(num_edges(rule.getCombinedGraph()));

		for(const auto e: asRange(edges(cg))) {
			const bool inContext = [&]() {
				const auto m = cg[e].membership;
				if(m != Membership::K) return false;
				const auto eL = get(getMorL(rule), getK(rule), getL(rule), e);
				const auto eR = get(getMorR(rule), getK(rule), getR(rule), e);
				const auto catL = ePropL[get(boost::edge_index_t(), getL(rule), eL)];
				const auto catR = ePropR[get(boost::edge_index_t(), getR(rule), eR)];
				if(catL != catR) return false;
				return eCallback(e);
			}();
			edgeInContext.push_back(inContext);
		}
		verify();
	}

	ValueTypeVertex operator[](lib::DPO::CombinedRule::CombinedVertex v) const {
		assert(v != boost::graph_traits<lib::DPO::CombinedRule::CombinedGraphType>::null_vertex());
		ValueTypeVertex res;
		if(rule.getCombinedGraph()[v].membership != Membership::R) res.left = getLeft()[v];
		if(rule.getCombinedGraph()[v].membership != Membership::L) res.right = getRight()[v];
		res.inContext = inContext(v);
		return res;
	}

	ValueTypeEdge operator[](lib::DPO::CombinedRule::CombinedEdge e) const {
		ValueTypeEdge res;
		if(rule.getCombinedGraph()[e].membership != Membership::R) res.left = getLeft()[e];
		if(rule.getCombinedGraph()[e].membership != Membership::L) res.right = getRight()[e];
		res.inContext = inContext(e);
		return res;
	}
public: // to be able to form pointers to getLeft and getRight it is not enough to 'using' them
	Side getLeft() const { return {*this, vPropL, ePropL, getL(rule)}; }
	Side getRight() const { return {*this, vPropR, ePropR, getR(rule)}; }
public: // we have redefined operator[], so get should be redefined as well
	friend auto get(const PropStereo &p, RuleType::CombinedVertex v) -> decltype(p[v]) { return p[v]; }
	friend auto get(const PropStereo &p, RuleType::CombinedEdge e) -> decltype(p[e]) { return p[e]; }
public:
	using Base::verify;
	using Base::print;

	struct Handler {
		template<typename VEProp, typename LabGraphDom, typename LabGraphCodom, typename F, typename ...Args>
		static auto fmap2(const VEProp &l,
		                  const VEProp &r,
		                  const LabGraphDom &gDom,
		                  const LabGraphCodom &gCodom,
		                  F &&f,
		                  Args &&... args) {
			assert(l.left.has_value() || l.right.has_value());
			if(l.inContext) {
				assert(l.left.has_value());
				assert(l.right.has_value());
			}
			assert(r.left.has_value() || r.right.has_value());
			if(r.inContext) {
				assert(r.left.has_value());
				assert(r.right.has_value());
			}
			assert(l.left.has_value() == r.left.has_value());
			assert(l.right.has_value() == r.right.has_value());
			using Left = decltype(f(*l.left, *r.left, get_labelled_left(gDom), get_labelled_left(gCodom), args...));
			using Right = decltype(f(*l.right, *r.right, get_labelled_right(gDom), get_labelled_right(gCodom), args...));
			using InContext = decltype(f(l.inContext, r.inContext, get_labelled_left(gDom), get_labelled_left(gCodom),
			                             args...));
			return std::tuple<boost::optional<Left>, boost::optional<Right>, InContext>(
					l.left.has_value() ? f(*l.left, *r.left, get_labelled_left(gDom), get_labelled_left(gCodom),
					                       args...) : boost::optional<Left>(),
					l.right.has_value() ? f(*l.right, *r.right, get_labelled_right(gDom), get_labelled_right(gCodom),
					                        args...) : boost::optional<Right>(),
					f(l.inContext, r.inContext, args...)
			);
		}

		template<typename Op, typename Val>
		static auto reduce(Op &&op, Val &&val) {
			const auto &left = std::get<0>(val);
			const auto &right = std::get<1>(val);
			const auto &inContext = std::get<2>(val);
			assert(left.is_initialized() || right.is_initialized());
			if(!left.is_initialized()) return op(*right, inContext);
			if(!right.is_initialized()) return op(*left, inContext);
			return op(op(*left, *right), inContext);
		}
	};
public:
	bool inContext(lib::DPO::CombinedRule::CombinedVertex v) const {
		return vertexInContext[get(boost::vertex_index_t(), rule.getCombinedGraph(), v)];
	}

	bool inContext(lib::DPO::CombinedRule::CombinedEdge e) const {
		return edgeInContext[get(boost::edge_index_t(), rule.getCombinedGraph(), e)];
	}
private:
	std::vector<bool> vertexInContext, edgeInContext;
};

} // namespace mod::lib::rule

#endif // MOD_LIB_RULE_PROP_STEREO_HPP