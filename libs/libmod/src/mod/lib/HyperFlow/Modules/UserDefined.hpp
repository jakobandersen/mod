#ifndef MOD_LIB_HYPERFLOW_USERDEFINED_HPP
#define MOD_LIB_HYPERFLOW_USERDEFINED_HPP

#include <mod/hyperflow/LinExp.hpp>
#include <mod/lib/HyperFlow/Modules/Module.hpp>
#include <mod/lib/CombiOpt/Variable.hpp>

#include <set>
#include <unordered_map>
#include <vector>

namespace mod::hyperflow {
struct VarCustom;
} // namespace mod::hyperflow
namespace mod::lib::HyperFlow {

struct UserDefinedSpecification : SpecificationModule {
	UserDefinedSpecification(Specification &owner, bool setDefaults);
	virtual std::string getName() const override;
	virtual void addDefaultObjective(hyperflow::LinExp &exp) const override;
	bool hasVar(const std::string &name) const;
	// throws LogicError if hasVar(name)
	hyperflow::VarCustom addBoolVar(std::string name);
	hyperflow::VarCustom addIntVar(std::string name);
	hyperflow::VarCustom addFloatVar(std::string name);
	const std::set<std::string> &getBoolVars() const;
	const std::set<std::string> &getIntVars() const;
	const std::set<std::string> &getFloatVars() const;
	void addConstraint(hyperflow::LinConstraint cons);
	const std::vector<hyperflow::LinConstraint> &getConstraints() const;
public:
	static std::string getVariableSetName();
private:
	virtual void listImpl(std::ostream &s) const override;
	virtual nlohmann::json dumpImpl() const override;
	virtual bool loadImpl(const nlohmann::json &j, std::ostream &err) override;
	virtual Transits getTransits() const override;
	virtual std::unique_ptr<ModelModule> createModel(Model &owner) const override;
private:
	std::set<std::string> boolVars, intVars, floatVars; // use ordered to have reproducible order
	std::vector<hyperflow::LinConstraint> constraints;
};

struct UserDefinedModel : ModelModule {
	using SpecificationType = UserDefinedSpecification;
	UserDefinedModel(Model &owner, const UserDefinedSpecification &specification);
private:
	virtual const UserDefinedSpecification &getSpec() const override;
	virtual void createVariablesImpl(CombiOpt::Model &model) override;
	virtual void createConstraintsImpl(CombiOpt::Model &model) override;
private:
	virtual nlohmann::json dumpImpl(const CombiOpt::Result &sol) const override;
	virtual bool loadImpl(const nlohmann::json &j, CombiOpt::LoadedSolution &s, std::ostream &err) const override;
	virtual void loadSolutionSetDependentVarsImpl(CombiOpt::LoadedSolution &s) const override;
	virtual bool hasListEntry(lib::DG::HyperVertex vHyper, const CombiOpt::Result &sol) const override;
	virtual std::vector<std::string> listHeaderEntries() const override;
	virtual std::vector<std::string> listEntries(lib::DG::HyperVertex vHyper, const CombiOpt::Result &sol) const override;
private:
	const UserDefinedSpecification &specification;
	std::map<std::string, CombiOpt::BoolVar> boolVars; // use ordered to have reproducible order
	std::map<std::string, CombiOpt::IntVar> intVars; // use ordered to have reproducible order
	std::map<std::string, CombiOpt::FloatVar> floatVars; // use ordered to have reproducible order
public:
	struct StaticInit;
};

} // namespace mod::lib::HyperFlow

#endif // MOD_LIB_HYPERFLOW_USERDEFINED_HPP