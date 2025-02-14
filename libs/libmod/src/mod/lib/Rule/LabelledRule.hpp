#ifndef MOD_LIB_RULES_LABELLED_RULE_HPP
#define MOD_LIB_RULES_LABELLED_RULE_HPP

#include <mod/lib/DPO/CombinedRule.hpp>
#include <mod/lib/GraphMorphism/Constraints/Constraint.hpp>
#include <mod/lib/Rule/ConnectedComponent.hpp>
#include <mod/lib/Rule/GraphDecl.hpp>
#include <mod/lib/Rule/Properties/Molecule.hpp>
#include <mod/lib/Rule/Properties/Stereo.hpp>
#include <mod/lib/Rule/Properties/String.hpp>
#include <mod/lib/Rule/Properties/Term.hpp>

#include <vector>

namespace mod::lib::rule {

struct LabelledRule {
	using RuleType = lib::DPO::CombinedRule;
public: // LabelledGraphConcept
	using GraphType = RuleType::CombinedGraphType;
	using PropStringType = PropString;
	using PropTermType = PropTerm;
	using PropStereoType = PropStereo;
public:
	using PropMoleculeType = PropMolecule;
public: // RuleConcept
	using SideGraphType = RuleType::SideGraphType;
	using KGraphType = RuleType::KGraphType;
public: // Projected view
	using SideProjectedGraphType = RuleType::SideProjectedGraphType;
public: // Other
	using Vertex = boost::graph_traits<GraphType>::vertex_descriptor;
	using Edge = boost::graph_traits<GraphType>::edge_descriptor;
	using MatchConstraint = GraphMorphism::Constraints::Constraint<SideGraphType>;
public:
	struct SideData;
public:
	struct Side {
		using GraphType = SideGraphType;
		using ComponentFilter = ConnectedComponentFilter<GraphType, std::vector<std::size_t>>;
		using ComponentGraph = boost::filtered_graph<GraphType, ComponentFilter, ComponentFilter>;
		using PropStringType = LabelledRule::PropStringType::Side;
		using PropTermType = LabelledRule::PropTermType::Side;
		using PropStereoType = LabelledRule::PropStereoType::Side;
	public:
		using PropMoleculeType = LabelledRule::PropMoleculeType::Side;
	public:
		// We don't want to convert data unless needed, so instead of storing the
		// side prop proxy objects, we store member function pointers to retrieve them.
		explicit Side(const LabelledRule &r, const GraphType &g,
		              PropStringType (LabelledRule::PropStringType::*fString)() const,
		              PropTermType (LabelledRule::PropTermType::*fTerm)() const,
		              PropStereoType (LabelledRule::PropStereoType::*fStereo)() const,
		              PropMoleculeType (LabelledRule::PropMoleculeType::*fMol)() const,
		              const SideData &data);
	public:
		friend const GraphType &get_graph(const Side &g);
		friend PropStringType get_string(const Side &g);
		friend PropTermType get_term(const Side &g);
		friend bool has_stereo(const Side &g);
		friend PropStereoType get_stereo(const Side &g);
	public:
		friend PropMoleculeType get_molecule(const Side &g);
	public:
		friend const std::vector<std::unique_ptr<MatchConstraint>> &
		get_match_constraints(const Side &g);
	public:
		friend std::size_t get_num_connected_components(const Side &g);
		friend const std::vector<std::size_t> get_component(const Side &g);
		friend ComponentGraph get_component_graph(std::size_t i, const Side &g);
	public:
		friend const std::vector<boost::graph_traits<GraphType>::vertex_descriptor> &
		get_vertex_order_component(std::size_t i, const Side &g);
	public:
		const LabelledRule &r;
		const GraphType &g;
		PropStringType (LabelledRule::PropStringType::*fString)() const;
		PropTermType (LabelledRule::PropTermType::*fTerm)() const;
		PropStereoType (LabelledRule::PropStereoType::*fStereo)() const;
		PropMoleculeType (LabelledRule::PropMoleculeType::*fMol)() const;
		const SideData &data;
	};
public:
	explicit LabelledRule(std::unique_ptr<RuleType> rule,
	                      std::unique_ptr<PropStringType> pString,
	                      std::unique_ptr<PropStereoType> pStereo);
	explicit LabelledRule(std::unique_ptr<RuleType> rule,
	                      std::unique_ptr<PropTermType> pTerm,
	                      std::unique_ptr<PropStereoType> pStereo);
	LabelledRule(); // TODO: remove
	LabelledRule(const LabelledRule &other, bool withConstraints); // TODO: hmm
	lib::DPO::CombinedRule &getRule(); // TODO: remove non-const version?
	const lib::DPO::CombinedRule &getRule() const;
public:
	void initComponents(); // TODO: this is a huge hax
	void invert();
public: // LabelledGraphConcept
	friend GraphType &get_graph(LabelledRule &r);
	friend const GraphType &get_graph(const LabelledRule &r);
	friend const PropStringType &get_string(const LabelledRule &r);
	friend const PropTermType &get_term(const LabelledRule &r);
	friend bool has_stereo(const LabelledRule &r);
	friend const PropStereoType &get_stereo(const LabelledRule &r);
public:
	friend const PropMoleculeType &get_molecule(const LabelledRule &r);
public:
	friend const SideProjectedGraphType &get_L_projected(const LabelledRule &r);
	friend const SideProjectedGraphType &get_R_projected(const LabelledRule &r);
	friend lib::DPO::Membership membership(const LabelledRule &r, const Vertex &v);
	friend lib::DPO::Membership membership(const LabelledRule &r, const Edge &e);
	friend void put_membership(LabelledRule &r, const Vertex &v, lib::DPO::Membership m);
	friend void put_membership(LabelledRule &r, const Edge &e, lib::DPO::Membership m);
public:
	friend Side get_labelled_left(const LabelledRule &r);
	friend Side get_labelled_right(const LabelledRule &r);
private:
	std::unique_ptr<lib::DPO::CombinedRule> rule;
public:
	mutable std::unique_ptr<PropStringType> pString;
	mutable std::unique_ptr<PropTermType> pTerm;
	mutable std::unique_ptr<PropStereoType> pStereo;
private:
	mutable std::unique_ptr<PropMoleculeType> pMolecule;
public:
	struct SideData {
		std::size_t numComponents = -1;
		std::vector<std::size_t> component;
		std::vector<std::unique_ptr<MatchConstraint>> matchConstraints;
		mutable std::vector<std::vector<Vertex>> vertex_orders;
	} leftData, rightData;
};

} // namespace mod::lib::rule

#endif // MOD_LIB_RULES_LABELLED_RULE_HPP