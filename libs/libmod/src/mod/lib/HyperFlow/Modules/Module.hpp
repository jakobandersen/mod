#ifndef MOD_LIB_HYPERFLOW_MODULE_HPP
#define MOD_LIB_HYPERFLOW_MODULE_HPP

#include <mod/lib/DG/GraphDecl.hpp>
#include <mod/lib/IO/Json.hpp>

#include <iosfwd>
#include <memory>

namespace mod::hyperflow {
struct LinExp;
} // namespace mod::hyperflow
namespace mod::lib::CombiOpt {
struct LoadedSolution;
struct Model;
struct Result;
} // namespace mod::lib::CombiOpt
namespace mod::lib::HyperFlow {
struct Model;
struct Transits;
struct ModelCreator;
struct ModelModule;
struct Specification;

// Derived classes must have a constructor starting with,
//   Specification &owner, bool setDefaults
// The constructor can not assume that all modules it depends on have
// been added yet if setDefaults == false.
struct SpecificationModule {
	SpecificationModule(SpecificationModule &&) = delete;
	SpecificationModule &operator=(SpecificationModule &&) = delete;
protected:
	explicit SpecificationModule(Specification &owner);
public:
	virtual ~SpecificationModule() = default;
	virtual std::string getName() const = 0;
	virtual void addDefaultObjective(hyperflow::LinExp &exp) const = 0;
	void list(std::ostream &s) const;
	nlohmann::json dump() const;
	// no validation of the JSON has been done, except that it is an object
	bool load(const nlohmann::json &j, std::ostream &err);
private:
	virtual void listImpl(std::ostream &s) const = 0;
	virtual nlohmann::json dumpImpl() const = 0;
	virtual bool loadImpl(const nlohmann::json &j, std::ostream &err) = 0;
	friend class ModelCreator;
	virtual Transits getTransits() const = 0;
	virtual std::unique_ptr<ModelModule> createModel(Model &owner) const = 0;
protected:
	Specification &owner;
};

struct ModelModule {
protected:
	ModelModule(Model &owner);
public:
	virtual ~ModelModule() = default;
public:
	nlohmann::json dump(const CombiOpt::Result &sol) const;
	bool load(const nlohmann::json &j, CombiOpt::LoadedSolution &s, std::ostream &err) const;
	void loadSolutionSetDependentVars(CombiOpt::LoadedSolution &s) const;
	virtual bool hasListEntry(lib::DG::HyperVertex vHyper, const CombiOpt::Result &sol) const = 0;
	virtual std::vector<std::string> listHeaderEntries() const = 0;
	virtual std::vector<std::string> listEntries(lib::DG::HyperVertex vHyper, const CombiOpt::Result &sol) const = 0;
private:
	friend class Model;
	void createVariables(CombiOpt::Model &model);
	void createConstraints(CombiOpt::Model &model);
private:
	virtual const SpecificationModule &getSpec() const = 0;
	virtual void createVariablesImpl(CombiOpt::Model &model) = 0;
	virtual void createConstraintsImpl(CombiOpt::Model &model) = 0;
private:
	virtual nlohmann::json dumpImpl(const CombiOpt::Result &sol) const = 0;
	// set non-dependent vars in s based on the JSON
	// can not assume any other modules has loaded yet
	virtual bool loadImpl(const nlohmann::json &j, CombiOpt::LoadedSolution &s, std::ostream &err) const = 0;
	// all modules have set non-dependent stuff, so now set all the other variables
	virtual void loadSolutionSetDependentVarsImpl(CombiOpt::LoadedSolution &s) const = 0;
protected:
	Model &owner;
};

} // namespace mod::lib::HyperFlow

#endif // MOD_LIB_HYPERFLOW_MODULE_HPP