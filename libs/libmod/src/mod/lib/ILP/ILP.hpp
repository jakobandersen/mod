#ifndef MOD_LIB_ILP_ILP_HPP
#define MOD_LIB_ILP_ILP_HPP

#include <mod/lib/CombiOpt/Value.hpp>

#include <boost/operators.hpp>

#include <cmath>
#include <functional>
#include <memory>
#include <limits>
#include <optional>
#include <unordered_set>
#include <vector>
#include <string>

namespace mod::lib::ILP {
using Int = CombiOpt::Int;
using Float = CombiOpt::Float;

struct Var {
	friend struct std::hash<Var>;
	friend class Model;

	explicit Var(unsigned int data) : data(data) {}

	bool operator==(const Var &other) const {
		return data == other.data;
	}

	bool operator!=(const Var &other) const {
		return data != other.data;
	}

	bool operator<(const Var &other) const {
		return data < other.data;
	}
private:
	unsigned int data;
};

} // namespace mod::lib::ILP

template<>
struct std::hash<mod::lib::ILP::Var> {
	std::size_t operator()(const mod::lib::ILP::Var &var) const {
		return hash<decltype(mod::lib::ILP::Var::data)>()(var.data);
	}
};

namespace mod::lib::ILP {

struct LinExp : boost::addable<LinExp,
		boost::subtractable<LinExp, boost::subtractable<LinExp, Var,
				boost::multipliable2<LinExp, Int,
						boost::multipliable2<LinExp, Float
						>>>>> {
	using IntElement = std::pair<Int, Var>;
	using DoubleElement = std::pair<Float, Var>;
public:
	LinExp() = default;
	LinExp(Var var);
	const std::vector<IntElement> &getIntElements() const;
	const std::vector<DoubleElement> &getDoubleElements() const;
	LinExp &operator+=(const LinExp &e);
	LinExp &operator-=(const LinExp &e);
	LinExp &operator*=(Int c);
	LinExp &operator*=(Float c);
	LinExp &operator/=(Int c);
	LinExp &operator/=(Float c);
private:
	std::vector<IntElement> intElements;
	std::vector<DoubleElement> doubleElements;
};

inline LinExp operator-(LinExp e) {
	return LinExp() - e;
}

#define MOD_MAKE_OP(Op)                                                         \
    inline LinExp operator Op(Var lhs, Var rhs) {                               \
        LinExp res(lhs);                                                        \
        return res Op##= rhs;                                                   \
    }
MOD_MAKE_OP(+)
MOD_MAKE_OP(-)
#undef MOD_MAKE_OP

inline LinExp operator*(Int lhs, Var rhs) {
	LinExp res(rhs);
	return res * lhs;
}

inline LinExp operator*(Float lhs, Var rhs) {
	LinExp res(rhs);
	return res * lhs;
}

struct LinConstraint {
	enum class Relation {
		Leq, Eq, Geq
	};
public: // represents: exp relation bound
	LinConstraint(LinExp exp, Relation relation, Float bound);
	const LinExp &getExp() const;
	const Relation &getRelation() const;
	Float getBound() const;
private:
	LinExp exp;
	Relation relation;
	Float bound;
};

#define MOD_MAKE_OP(Op1, Op2, Rel, Type)                                        \
    inline LinConstraint operator Op1(LinExp e, Type c) {                       \
        return LinConstraint(e, LinConstraint::Relation::Rel, Float(c));        \
    }                                                                           \
    inline LinConstraint operator Op2(Type c, LinExp e) {                       \
        return e Op1 c;                                                         \
    }
MOD_MAKE_OP(<=, >=, Leq, int)
MOD_MAKE_OP(<=, >=, Leq, double)
MOD_MAKE_OP(<=, >=, Leq, Int)
MOD_MAKE_OP(<=, >=, Leq, Float)
MOD_MAKE_OP(>=, <=, Geq, int)
MOD_MAKE_OP(>=, <=, Geq, double)
MOD_MAKE_OP(>=, <=, Geq, Int)
MOD_MAKE_OP(>=, <=, Geq, Float)
MOD_MAKE_OP(==, ==, Eq, int)
MOD_MAKE_OP(==, ==, Eq, double)
MOD_MAKE_OP(==, ==, Eq, Int)
MOD_MAKE_OP(==, ==, Eq, Float)
#undef MOD_MAKE_OP

enum class Response {
	Optimal, Feasible, TimedOut, Failed
};

std::ostream &operator<<(std::ostream &s, const Response &res);

struct CallbackResponse {
	enum class SolutionType : char {
		New, Duplicate, Rejected
	};

	enum class ObjectiveMode {
		Integral, Real
	};

	bool continueSearch;

	SolutionType solType;
	unsigned int solutionId; // only valid if it was New or Duplicate

	// only the one corresponding to the mode is used
	std::optional<Int> newIntegralObjValBound;
	std::optional<Float> newRealObjValBound;
};

class Model {
protected:
	Model();
public:
	virtual ~Model();
public: // construction
	// The returned variable will have bounds [0; infinity[
	Var addVariableFloat(const std::string &name);
	// The returned variable will have bounds [0; infinity[
	Var addVariableInteger(const std::string &name);
	Var addVariableBinary(const std::string &name);
	void setLB(Var v, double bound);
	void setUB(Var v, double bound);
	void setBranchPriority(Var var, unsigned int prio);
	void setObjectiveFunction(const LinExp &exp);
	bool getIsObjectiveFunctionIntegral() const;
	void addConstraint(const LinConstraint &constraint);
	void popConstraint();
private: // construction
	// The returned variable must have bounds [0; infinity[
	virtual Var addVariableFloatImpl(const std::string &name) = 0;
	// The returned variable must have bounds [0; infinity[
	virtual Var addVariableIntegerImpl(const std::string &name) = 0;
	virtual Var addVariableBinaryImpl(const std::string &name) = 0;
	virtual void setLBImpl(Var v, double bound) = 0;
	virtual void setUBImpl(Var v, double bound) = 0;
	virtual void setBranchPriorityImpl(Var var, unsigned int prio) = 0;
	virtual void setObjectiveFunctionImpl(const LinExp &exp) = 0;
	virtual void addConstraintImpl(const LinConstraint &constraint) = 0;
	virtual void popConstraintImpl() = 0;
public: // model querying
	struct VarIter {
		VarIter();
		VarIter(const Model &model, Var currentVar);
		bool operator!=(const VarIter &other) const;
		VarIter operator++();
		Var operator*() const;
	private:
		const Model *model;
		Var currentVar;
	};

	struct VariablesProxy {
		VariablesProxy(const Model &model);
		VarIter begin() const;
		VarIter end() const;
	private:
		const Model &model;
	};
	VariablesProxy Variables() const;
	virtual Int getLBInt(Var v) const = 0;
	virtual Int getUBInt(Var v) const = 0;
	virtual bool isVariableReal(Var var) const = 0;
	virtual std::string getVarName(Var var) const = 0;
public:
	virtual int getNumVariables() const = 0;
	virtual int getNumConstraints() const = 0;
private:
	virtual VarIter variablesBeginImpl() const = 0;
	virtual VarIter variablesIncrImpl(Var var) const = 0;
public: // calculation
	virtual Response realSolve(std::ostream &log, int verbosity) = 0;
	// solutionReady must return (continueSearch, solutionId)
private:
	template<typename ObjHandler>
	void solveImpl(const std::unordered_set<Var> &enumerationVarsAsSet,
	               std::function<void(CallbackResponse &, CallbackResponse::ObjectiveMode)> solutionReady,
	               int verbosity, int ilpVerbosity);
public:
	void solve(const std::unordered_set<Var> &enumerationVarsAsSet,
	           std::function<void(CallbackResponse &, CallbackResponse::ObjectiveMode)> solutionReady,
	           int verbosity, int ilpVerbosity);
	struct CalculationProxy;
private: // calculation
	virtual void pushBranchingConstraint(const LinConstraint &constraint) = 0;
	virtual void popBranchingConstraint() = 0;
	virtual void popAllBranchingConstraints() = 0;
public: // calculated data querying
	virtual Int getVarValIntegral(Var var) const = 0;
	virtual Float getVarValReal(Var var) const = 0;
	virtual Int getObjValIntegral() const = 0;
	virtual Float getObjValReal() const = 0;
protected:
	std::string initVariableName(const std::string &name, Var v);
protected:
	static unsigned int getData(Var v);
private:
	bool externallyModified;
	bool isObjectiveFunctionIntegral;
protected:
	const bool debugOutput;
public:
	virtual void write(const std::string &filename) const = 0;
public:
	// implemented in createModel.cpp
	static std::vector<std::string> getAvailableSolvers();
	static std::unique_ptr<Model> createModel(std::string solver);
};

} // namespace mod::lib::ILP

#endif /// MOD_LIB_ILP_ILP_HPP