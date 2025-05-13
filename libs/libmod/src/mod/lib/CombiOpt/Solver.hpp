#ifndef MOD_LIB_COMBIOPT_SOLVER_HPP
#define MOD_LIB_COMBIOPT_SOLVER_HPP

#include <mod/lib/CombiOpt/Model.hpp>

#include <iosfwd>
#include <unordered_set>

namespace mod::lib::ILP {
struct Model;
} // namespace mod::lib::ILP
namespace mod::lib::CombiOpt {
struct LoadedSolution;
struct Result;

struct Solver {
	// Verbosity:
	// - 0: nothing, and force ilpVerbosity == 0
	// - 1: basic status updates and termination message, and force ilpVerbosity to max 1
	// - 2: detailed output, basically debug messages
	// ILP verbosity:
	// - 0: nothing
	// - 1: normal output on root node
	// - 2: normal output in all nodes
	// throws LogicError if non-integral objective function and a solution has already been found
	explicit Solver(Model model, std::unordered_set<Var> enumerationVars, const std::string &solver,
					int verbosity, std::ostream &log);
	~Solver();
	const Model &getModel() const;
	bool isEnumVarsIntegral() const;
	bool isObjValueIntegral() const;
	void printEnumerationTree(std::ostream &s) const;
	const ILP::Model &getILP() const;
	Result next(int verbosity, int ilpVerbosity);
	Result loadSolution(LoadedSolution &&s, int verbosity);
private:
	struct Pimpl;
	friend class Result;
	std::unique_ptr<Pimpl> p;
};

struct Result {
	struct Pimpl;
public:
	// Constructs a "no solution" result.
	Result() = default;
	explicit Result(std::unique_ptr<Pimpl> p);
	Result(Result &&other);
	Result &operator=(Result &&other);
	~Result();
	explicit operator bool() const { return static_cast<bool>(p); }
	// all the next functions require bool(*this)
public: // integral: requires Solver->isObjValueIntegral()
	Int getObjValInt() const;
public: // non-integral: requires !Solver->isObjValueIntegral()
	Float getObjValFloat() const;
public:
	Value getValue(Var v) const;
	Int getValue(IntVar v) const;
	Int getValue(BoolVar v) const;
	Float getValue(FloatVar v) const;
	Float getValueCastFloat(Var v) const;
	Value evaluate(const CombiOpt::LinExpAny &expr) const;
private:
	Int getValueIntegral(IntegralVar v) const;
	Float getValueFloat(FloatVar v) const;
private:
	std::unique_ptr<Pimpl> p;
};

} // namespace mod::lib::CombiOpt

#endif // MOD_LIB_COMBIOPT_SOLVER_HPP
