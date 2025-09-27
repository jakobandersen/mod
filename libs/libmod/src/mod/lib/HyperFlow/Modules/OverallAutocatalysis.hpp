#ifndef MOD_LIB_HYPERFLOW_OVERALLAUTOCATALYSIS_HPP
#define MOD_LIB_HYPERFLOW_OVERALLAUTOCATALYSIS_HPP

#include <mod/lib/DG/GraphDecl.hpp>
#include <mod/lib/CombiOpt/Variable.hpp>
#include <mod/lib/HyperFlow/Modules/Module.hpp>

namespace mod::lib::HyperFlow {

struct OverallAutocatalysisSpecification : SpecificationModule {
	OverallAutocatalysisSpecification(Specification &owner, bool setDefaults);
	virtual ~OverallAutocatalysisSpecification() = default;
	virtual std::string getName() const override;
	virtual void addDefaultObjective(hyperflow::LinExp &exp) const override;
private:
	virtual void listImpl(std::ostream &s) const override;
	virtual nlohmann::json dumpImpl() const override;
	virtual bool loadImpl(const nlohmann::json &j, std::ostream &err) override;
	virtual Transits getTransits() const override;
	virtual std::unique_ptr<ModelModule> createModel(Model &owner) const override;
public:
	bool forceExistence = true, strictTransit = true, bfsExclusive = true;
};

struct OverallAutocatalysisModel : ModelModule {
	using SpecificationType = OverallAutocatalysisSpecification;
	OverallAutocatalysisModel(Model &owner);
	virtual ~OverallAutocatalysisModel() = default;
private:
	virtual const OverallAutocatalysisSpecification &getSpec() const override;
	virtual void createVariablesImpl(CombiOpt::Model &model) override;
	virtual void createConstraintsImpl(CombiOpt::Model &model) override;
private:
	virtual nlohmann::json dumpImpl(const CombiOpt::Result &sol) const override;
	virtual bool loadImpl(const nlohmann::json &j, CombiOpt::LoadedSolution &s, std::ostream &err) const override;
	virtual void loadSolutionSetDependentVarsImpl(CombiOpt::LoadedSolution &s) const override;
	virtual bool hasListEntry(lib::DG::HyperVertex vHyper, const CombiOpt::Result &sol) const override;
	virtual std::vector<std::string> listHeaderEntries() const override;
	virtual std::vector<std::string>
	listEntries(lib::DG::HyperVertex vHyper, const CombiOpt::Result &sol) const override;
private:
	std::unordered_map<lib::DG::HyperVertex, CombiOpt::BoolVar> isOverallAutocatalytic;
public:
	struct StaticInit;
};

} // namespace mod::lib::HyperFlow

#endif // MOD_LIB_HYPERFLOW_OVERALLAUTOCATALYSIS_HPP