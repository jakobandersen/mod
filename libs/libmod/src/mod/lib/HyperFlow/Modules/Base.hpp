#ifndef MOD_LIB_HYPERFLOW_BASE_HPP
#define MOD_LIB_HYPERFLOW_BASE_HPP

#include <mod/dg/DG.hpp>
#include <mod/lib/CombiOpt/Variable.hpp>
#include <mod/lib/DG/GraphDecl.hpp>
#include <mod/lib/HyperFlow/Modules/Module.hpp>

#include <set>
#include <unordered_map>

namespace mod {
template<typename Sig>
struct Function;
} // namespace mod
namespace mod::lib::CombiOpt {
struct LoadedSolution;
} // namespace mod::lib::CombiOpt
namespace mod::lib::HyperFlow {

struct BaseSpecification : SpecificationModule {
	BaseSpecification(Specification &owner, bool setDefaults);
	virtual ~BaseSpecification() = default;
	virtual std::string getName() const override;
	virtual void addDefaultObjective(hyperflow::LinExp &exp) const override;
	void addSource(lib::DG::HyperVertex v);
	const std::set<lib::DG::HyperVertex> &getSources() const;
	void addSink(lib::DG::HyperVertex v);
	const std::set<lib::DG::HyperVertex> &getSinks() const;
	void exclude(lib::DG::HyperVertex v);
	const std::set<lib::DG::HyperVertex> &getExcluded() const;
	void separateIOInternalTransit(lib::DG::HyperVertex v);
	const std::set<lib::DG::HyperVertex> &getSeparatedIOInternalTransit() const;
	void setAllowHyperLoops(bool v);
	bool getAllowHyperLoops() const;
	void setAllowReversal(bool value);
	bool getAllowReversal() const;
	void setAllowIOReversal(bool value);
	bool getAllowIOReversal() const;
	void setRelaxed(bool value);
	bool getRelaxed() const;
private:
	virtual void listImpl(std::ostream &s) const override;
	virtual nlohmann::json dumpImpl() const override;
	virtual bool loadImpl(const nlohmann::json &j, std::ostream &err) override;
	virtual Transits getTransits() const override;
	virtual std::unique_ptr<ModelModule> createModel(Model &owner) const override;
private:
	std::set<lib::DG::HyperVertex>
			sources, sinks, excluded,
			verticesWithForcedTransitSeparation;
	bool allowHyperLoops = false;
	bool allowReversal = false, allowIOReversal = true;
	bool relaxed = false;
};

struct BaseModel : ModelModule {
	using SpecificationType = BaseSpecification;
	BaseModel(Model &owner, const BaseSpecification &specification);
	virtual ~BaseModel() = default;
public:
	const std::vector<bool> &getIsExcluded() const;
private:
	virtual const BaseSpecification &getSpec() const override;
	virtual void createVariablesImpl(CombiOpt::Model &model) override;
	virtual void createConstraintsImpl(CombiOpt::Model &model) override;
public:
	CombiOpt::IntVar getEdge(lib::DG::ExpandedVertex vExp) const;
	CombiOpt::IntVar getIn(lib::DG::HyperVertex v) const;
	CombiOpt::IntVar getOut(lib::DG::HyperVertex v) const;
	CombiOpt::IntVar getVertex(lib::DG::HyperVertex v) const;
	CombiOpt::BoolVar getIsEdgeUsed(lib::DG::HyperVertex v) const;
	CombiOpt::BoolVar getIsEdgeBothReverseUsed(lib::DG::HyperVertex v) const;
	CombiOpt::BoolVar getIsInUsed(lib::DG::HyperVertex v) const;
	CombiOpt::BoolVar getIsOutUsed(lib::DG::HyperVertex v) const;
	CombiOpt::BoolVar getIsInLessOut(lib::DG::HyperVertex v) const;
	CombiOpt::BoolVar getIsInGreaterOut(lib::DG::HyperVertex v) const;
	CombiOpt::BoolVar getIsInOutZero(lib::DG::HyperVertex v) const;
	CombiOpt::BoolVar getIsVertexUsed(lib::DG::HyperVertex v) const;
public:
	CombiOpt::FloatVar getEdgeRelaxed(lib::DG::ExpandedVertex vExp) const;
	CombiOpt::FloatVar getInRelaxed(lib::DG::HyperVertex v) const;
	CombiOpt::FloatVar getOutRelaxed(lib::DG::HyperVertex v) const;
	CombiOpt::FloatVar getVertexRelaxed(lib::DG::HyperVertex v) const;
public:
	void loadSolution_v2to7(CombiOpt::LoadedSolution &s,
	                        const std::vector<std::pair<unsigned int, unsigned int>> &edgeFlow,
	                        const std::vector<std::pair<unsigned int, unsigned int>> &inFlow,
	                        const std::vector<std::pair<unsigned int, unsigned int>> &outFlow,
	                        const std::vector<std::tuple<unsigned int, std::vector<int>, std::vector<int>, unsigned int>> &transitFlow) const;
public:
	virtual nlohmann::json dumpImpl(const CombiOpt::Result &sol) const override;
	virtual bool loadImpl(const nlohmann::json &j, CombiOpt::LoadedSolution &s, std::ostream &err) const override;
	virtual void loadSolutionSetDependentVarsImpl(CombiOpt::LoadedSolution &s) const override;
	virtual bool hasListEntry(lib::DG::HyperVertex vHyper, const CombiOpt::Result &sol) const override;
	virtual std::vector<std::string> listHeaderEntries() const override;
	virtual std::vector<std::string> listEntries(lib::DG::HyperVertex vHyper,
	                                             const CombiOpt::Result &sol) const override;
private:
	const BaseSpecification &specification;
	// meta
	bool hasAllTransitInternalFlow = true;
	std::vector<bool> isExcluded; // vertices with incident edges computed from spec.excludedVertices
	// ordinary
	std::unordered_map<lib::DG::ExpandedVertex, CombiOpt::IntVar> allFlowVars; // except aggregate variables
	std::unordered_map<lib::DG::HyperVertex, CombiOpt::IntVar>
	/**/ edgeFlow, inFlow, outFlow, // the defining variables
	/**/ vertexFlow, transitInternalFlow; // aggregate variables
	std::unordered_map<lib::DG::HyperVertex, CombiOpt::BoolVar>
	/**/ isEdgeUsed, isEdgeBothReverseUsed,
	/**/ isInUsed, isOutUsed,
	/**/ isInLessOut, isInGreaterOut, isInOutZero,
	/**/ isVertexUsed;

	// relaxed mode
	struct {
		std::unordered_map<lib::DG::ExpandedVertex, CombiOpt::FloatVar> allFlowVars; // except aggregate variables
		std::unordered_map<lib::DG::HyperVertex, CombiOpt::FloatVar>
		/**/ edgeFlow, inFlow, outFlow, // the defining variables
		/**/ vertexFlow, transitInternalFlow; // aggregate variables
	} relaxedMode;
public:
	struct StaticInit;
};

} // namespace mod::lib::HyperFlow

#endif // MOD_LIB_HYPERFLOW_BASE_HPP