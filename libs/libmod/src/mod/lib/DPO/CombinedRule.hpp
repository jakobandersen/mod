#ifndef MOD_LIB_DPO_COMBINEDRULE_HPP
#define MOD_LIB_DPO_COMBINEDRULE_HPP

#include <mod/lib/DPO/FilteredGraphProjection.hpp>

#include <jla_boost/graph/EdgeIndexedAdjacencyList.hpp>
#include <jla_boost/graph/morphism/models/InvertibleVector.hpp>
#include <jla_boost/graph/morphism/models/Vector.hpp>

namespace mod::lib::DPO {

// A DPO rule model which contains a combined graph of the L <- K -> R span.
// That is, besides getting views of L, K, and R, there is a view which represents
// the pushout object C of the rule span.
// There are two views of L and R: an ordinary one, and then a projected one.
// The projected views are adaptations of the combined view C, i.e.,
// vertex/edge descriptors from LProjected and RProjected are valid in C.
// The K-view is currently also just a projected view.
struct CombinedRule {
public: // CombinedGraph
	struct CombinedVProp {
		Membership membership;
	};

	struct CombinedEProp {
		Membership membership;
	};

	using CombinedGraphType = jla_boost::EdgeIndexedAdjacencyList<boost::undirectedS, CombinedVProp, CombinedEProp>;
	using CombinedVertex = boost::graph_traits<CombinedGraphType>::vertex_descriptor;
	using CombinedEdge = boost::graph_traits<CombinedGraphType>::edge_descriptor;

	struct MembershipPropertyMap {
		MembershipPropertyMap(const CombinedRule &r) : g(r.getCombinedGraph()) {}
		friend Membership get(MembershipPropertyMap m, CombinedVertex v) {
			return get(&CombinedVProp::membership, m.g, v);
		}

		friend Membership get(MembershipPropertyMap m, CombinedEdge e) {
			return get(&CombinedEProp::membership, m.g, e);
		}
	public:
		const CombinedGraphType &g;
	};
public:
	using SideProjectedGraphType = lib::DPO::FilteredGraphProjection<CombinedGraphType>;
	using KProjectedGraphType = lib::DPO::FilteredGraphProjection<CombinedGraphType>;
public: // RuleConcept
	using SideGraphType = SideProjectedGraphType;
	using SideVertex = boost::graph_traits<SideGraphType>::vertex_descriptor;
	using SideEdge = boost::graph_traits<SideGraphType>::edge_descriptor;
	using KGraphType = KProjectedGraphType;
	using KVertex = boost::graph_traits<KGraphType>::vertex_descriptor;
	using KEdge = boost::graph_traits<KGraphType>::edge_descriptor;
	struct MorphismType {
		// Both domain and codomain are projected so vertex descriptors mean the same in both.
		// But a side-vertex should map to a null-vertex if it is not in K.
		using GraphDom = KGraphType;
		using GraphCodom = SideGraphType;
		using Storable = std::false_type;
	public:
		MorphismType(const GraphDom &, const GraphCodom &) {}
	public:
		friend SideVertex get(const MorphismType &, const GraphDom &gDom, const GraphCodom &, KVertex v) {
			assert(gDom.gInner[v].membership == Membership::K);
			return v;
		}

		friend KVertex get_inverse(const MorphismType &, const GraphDom &gDom, const GraphCodom &gCodom, SideVertex v) {
			if(gCodom.gInner[v].membership == Membership::K) return v;
			else return GraphDom::null_vertex();
		}
	public:
		friend SideEdge get(const MorphismType &, const GraphDom &gDom, const GraphCodom &, KEdge e) {
			assert(gDom.gInner[e].membership == Membership::K);
			return e;
		}

		friend KEdge get_inverse(const MorphismType &, const GraphDom &gDom, const GraphCodom &gCodom, SideEdge e) {
			if(gCodom.gInner[e].membership == Membership::K) return e;
			else return {};
		}
	};
public: // Other, {L, K, R} -> Combined Graph
	struct ToCombinedMorphismK {
		// The identity morphism
		using GraphDom = KGraphType;
		using GraphCodom = CombinedGraphType;
		using Storable = std::false_type;
	public:
		ToCombinedMorphismK(const GraphDom &, const GraphCodom &) {}

		friend CombinedVertex
		get(const ToCombinedMorphismK &, const GraphDom &gDom, const GraphCodom &, CombinedVertex v) {
			return v;
		}

		friend CombinedVertex
		get_inverse(const ToCombinedMorphismK &, const GraphDom &gDom, const GraphCodom &, CombinedVertex v) {
			return v;
		}

		friend CombinedEdge
		get(const ToCombinedMorphismK &, const GraphDom &gDom, const GraphCodom &, CombinedEdge e) {
			return e;
		}

		friend CombinedEdge
		get_inverse(const ToCombinedMorphismK &, const GraphDom &gDom, const GraphCodom &, CombinedEdge e) {
			return e;
		}
	};
	using ToCombinedMorphismSide = ToCombinedMorphismK;
public:
	CombinedRule();
	// internally pointers are used, so don't allow moving and copying
	CombinedRule(CombinedRule &&) = delete;
	CombinedRule &operator=(CombinedRule &&) = delete;
public: // RuleConcept
	friend const SideGraphType &getL(const CombinedRule &r);
	friend const KGraphType &getK(const CombinedRule &r);
	friend const SideGraphType &getR(const CombinedRule &r);
	friend const MorphismType &getMorL(const CombinedRule &r);
	friend const MorphismType &getMorR(const CombinedRule &r);
public: // WriteableRuleConcept
	friend void invert(CombinedRule &r);
	friend SideVertex addVertexL(CombinedRule &r);
	friend KVertex addVertexK(CombinedRule &r);
	friend SideVertex addVertexR(CombinedRule &r);
	friend SideVertex promoteVertexL(CombinedRule &r, SideVertex vL);
	friend SideEdge addEdgeL(CombinedRule &r, SideVertex v1, SideVertex v2);
	friend KEdge addEdgeK(CombinedRule &r, KVertex v1, KVertex v2);
	friend SideEdge addEdgeR(CombinedRule &r, SideVertex v1, SideVertex v2);
public: // Other, Combined Graph
	CombinedGraphType &getCombinedGraph(); // TODO: remove non-const version
	const CombinedGraphType &getCombinedGraph() const;
	MembershipPropertyMap makeMembershipPropertyMap() const;
	const ToCombinedMorphismSide &getLtoCG() const;
	const ToCombinedMorphismK &getKtoCG() const;
	const ToCombinedMorphismSide &getRtoCG() const;
public: // Other, Projections of Combined Graph
	const SideProjectedGraphType &getLProjected() const;
	const KProjectedGraphType &getKProjected() const;
	const SideProjectedGraphType &getRProjected() const;
private:
	CombinedGraphType gCombined;
	KProjectedGraphType gProjectedK;
	SideProjectedGraphType gProjectedL, gProjectedR;
	MorphismType mL, mR;
private:
	ToCombinedMorphismK mKtoCG;
	ToCombinedMorphismSide mLtoCG, mRtoCG;
};

} // namespace mod::lib::DPO

#endif // MOD_LIB_DPO_COMBINEDRULE_HPP