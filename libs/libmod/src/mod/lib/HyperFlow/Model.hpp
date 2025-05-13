#ifndef MOD_LIB_HYPERFLOW_MODEL_HPP
#define MOD_LIB_HYPERFLOW_MODEL_HPP

#include <mod/lib/HyperFlow/Specification.hpp>

namespace mod::lib::CombiOpt {
struct LinConstraintAny;
struct LinExpAny;
struct LoadedSolution;
struct Model;
struct Solver;
struct Result;
} // mod::lib::CombiOpt
namespace mod::lib::DG {
struct Expanded;
} // namespace mod::lib::DG
namespace mod::lib::HyperFlow {
struct SolutionRange;

// A model is a compiled version of the specification.
// This compilation happens in the constructor and no further modifications are possible.
// Modules go through the following steps in lock-step:
// 1. Construction from specification. (No other model modules are available yet)
//    The module must check if it can be created or throw. E.g., not all modules can be created in relaxed mode.
// 2. Variable creation.
// 3. Constraint creation.

struct Model {
	// TODO: if the ILP solver does not exist the ctor throws and the specification is lost
	explicit Model(std::unique_ptr<const Specification> specification, const std::string &ilpSolver, int verbosity);
public:
	~Model();
	const lib::DG::Expanded &getExpanded() const;
	bool hasModule(std::type_index id) const;
	const ModelModule &getModule(std::type_index id) const;
	template<typename Module>
	// pre: hasModule<Module>
	const Module &getModule() const;
	// throws LogicError if there is a mismatch of data
	CombiOpt::LinConstraintAny compileConstraint(const hyperflow::LinConstraint &c) const;
	// throws LogicError if there is a mismatch of data
	CombiOpt::LinExpAny compileExpr(const hyperflow::LinExp &e) const;
	nlohmann::json dump() const;
	bool load(const nlohmann::json &j, int verbosity, std::ostream &err);
public:
	const CombiOpt::Solver &getSolver() const;
	bool isEnumVarsIntegral() const;
	bool isObjectiveValueIntegral() const;
	void printEnumerationTree(std::ostream &s) const;
	SolutionRange findSolutions(int maxNumSolutions, int verbosity, int ilpVerbosity);
	SolutionRange loadSolution(CombiOpt::LoadedSolution &&s, int verbosity);
	SolutionRange getSolutions() const;
	void listSolution(std::ostream &s, const CombiOpt::Result &sol) const;
public:
	const std::unique_ptr<const Specification> specification;
private:
	std::unique_ptr<const lib::DG::Expanded> dgExpanded;
	// indexed by the specification type_index
	std::map<std::type_index, std::unique_ptr<ModelModule> > modules;
private:
	std::unique_ptr<CombiOpt::Solver> solver;
	friend class SolutionRange;
	std::vector<CombiOpt::Result> solutions;
};

template<typename Module>
const Module &Model::getModule() const {
	using Spec = typename Module::SpecificationType;
	auto iter = modules.find(std::type_index(typeid(Spec)));
	assert(iter != modules.end());
	assert(dynamic_cast<Module *> (iter->second.get()));
	return *static_cast<Module *> (iter->second.get());
}

struct SolutionRange {
	using iterator = std::vector<CombiOpt::Result>::const_iterator;
private:
	friend class Model;
	SolutionRange(const Model *model, int first, int last)
			: model(model), first(first), last(last) {}
public:
	SolutionRange() = default;
	iterator begin() const;
	iterator end() const;
	int size() const;
// see https://www.spinics.net/lists/fedora-devel/msg312638.html
#pragma GCC diagnostic push
#if __GNUC__ >= 13
#pragma GCC diagnostic ignored "-Wdangling-reference"
#endif
	const CombiOpt::Result &operator[](int i) const;
#pragma GCC diagnostic pop
private:
	const Model *model = nullptr;
	int first = 0, last = 0;
};

} // namespace mod::lib::HyperFlow

#endif // MOD_LIB_DGFLOW_MODEL_HPP