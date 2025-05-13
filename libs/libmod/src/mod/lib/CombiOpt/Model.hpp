#ifndef MOD_LIB_COMBIOPT_MODEL_HPP
#define MOD_LIB_COMBIOPT_MODEL_HPP

#include <mod/lib/CombiOpt/LinConstraintAny.hpp>
#include <mod/lib/CombiOpt/LinConstraintInt.hpp>
#include <mod/lib/CombiOpt/LinConstraintFloat.hpp>
#include <mod/lib/CombiOpt/LinExpAny.hpp>
#include <mod/lib/CombiOpt/BoolExpr.hpp>
#include <mod/lib/CombiOpt/Variable.hpp>

#include <memory>
#include <optional>
#include <vector>

namespace mod::lib::CombiOpt {
struct ILPImplementation;

struct Model {
	BoolVar addBoolVar(std::string name);
	IntVar addIntVariable(std::string name);
	FloatVar addFloatVariable(std::string name);
	void addConstraint(Conjunction c);
	void addConstraint(Disjunction c);
	void addConstraint(LinConstraintInt c);
	void addConstraint(LinConstraintFloat c);
	void addConstraint(LinConstraintAny c);
	// If v then c must hold.
	void addImplication(Conjunction v, LinConstraintInt c);
	void addImplication(Conjunction v, LinConstraintFloat c);
	void addImplication(Conjunction v, BoolVar c);
	void addImplication(Conjunction v, NegatedBoolVar c);
	void addEquivalence(LinConstraintInt c, Conjunction v);
	void setLB(BoolVar var, Int value);
	void setUB(BoolVar var, Int value);
	void setLB(IntVar var, Int value);
	void setUB(IntVar var, Int value);
	void setLB(FloatVar var, Float value);
	void setUB(FloatVar var, Float value);
	void setObjectiveFunction(LinExpAny e);
	const LinExpAny &getObjectiveFunction() const;
public:
	// hax access to all variables
	struct VarIter {
		VarIter(const Model *m, int offset) : m(m), offset(offset) {}

		friend bool operator!=(VarIter a, VarIter b) {
			return std::tie(a.m, a.offset) != std::tie(b.m, b.offset);
		}

		VarIter &operator++() {
			++offset;
			return *this;
		}

		Var operator*() const {
			return Var(m->vars[offset].type, offset);
		}
	private:
		const Model *m = nullptr;
		int offset = 0;
	};
	struct VariableRange {
		VariableRange(const Model *m, int first, int last)
				: m(m), first(first), last(last) {}
		VarIter begin() const { return {m, first}; }
		VarIter end() const { return {m, last}; }
	private:
		const Model *m;
		int first, last;
	};
	VariableRange getVariables() const;
	const std::string &getVarName(Var var) const;
	Int getLB(BoolVar var) const;
	Int getUB(BoolVar var) const;
	Int getLB(IntVar var) const;
	Int getUB(IntVar var) const;
	Float getLB(FloatVar var) const;
	Float getUB(FloatVar var) const;
	Int getLB(IntegralVar var) const;
	Int getUB(IntegralVar var) const;
	Float getLB(Var var) const;
	Float getUB(Var var) const;
	Int getLB(const LinExpInt &e) const;
	Int getUB(const LinExpInt &e) const;
	Float getLB(const LinExpFloat &e) const;
	Float getUB(const LinExpFloat &e) const;
private:
	struct VariableInfo {
		VarType type;
		std::string name;
		std::size_t offset; // offset in separate variable arrays
	public:
		friend bool operator==(const VariableInfo &a, const VariableInfo &b) {
			return a.name == b.name;
		}

		friend bool operator<(const VariableInfo &a, const VariableInfo &b) {
			return a.name < b.name;
		}
	};

	struct BoolVarInfo {
		std::size_t id; // variable ID
		Int lb = 0;
		Int ub = 1;
	};

	struct IntVarInfo {
		std::size_t id; // variable ID
		Int lb = 0;
		Int ub = Int::INF_POS;
	};

	struct FloatVarInfo {
		std::size_t id; // variable ID
		Float lb = 0.0;
		Float ub = Float::INF_POS;
	};

	struct ImplicationInt {
		Conjunction left;
		LinConstraintInt right;
	public:
		friend bool operator==(const ImplicationInt &a, const ImplicationInt &b) {
			return std::tie(a.left, a.right) == std::tie(b.left, b.right);
		}

		friend bool operator<(const ImplicationInt &a, const ImplicationInt &b) {
			return std::tie(a.left, a.right) < std::tie(b.left, b.right);
		}
	};

	struct ImplicationFloat {
		Conjunction left;
		LinConstraintFloat right;
	public:
		friend bool operator==(const ImplicationFloat &a, const ImplicationFloat &b) {
			return std::tie(a.left, a.right) == std::tie(b.left, b.right);
		}

		friend bool operator<(const ImplicationFloat &a, const ImplicationFloat &b) {
			return std::tie(a.left, a.right) < std::tie(b.left, b.right);
		}
	};

	struct ImplicationBool {
		Conjunction left;
		BoolVar right;
		bool rightNegated;
	public:
		friend bool operator==(const ImplicationBool &a, const ImplicationBool &b) {
			return std::tie(a.left, a.right, a.rightNegated) == std::tie(b.left, b.right, b.rightNegated);
		}

		friend bool operator<(const ImplicationBool &a, const ImplicationBool &b) {
			return std::tie(a.left, a.right, a.rightNegated) < std::tie(b.left, b.right, b.rightNegated);
		}
	};

	struct Equivalence {
		LinConstraintInt left;
		Conjunction right;
	public:
		friend bool operator==(const Equivalence &a, const Equivalence &b) {
			return std::tie(a.left, a.right) == std::tie(b.left, b.right);
		}

		friend bool operator<(const Equivalence &a, const Equivalence &b) {
			return std::tie(a.left, a.right) < std::tie(b.left, b.right);
		}
	};
public:
	std::ostream &print(std::ostream &s, const VariableInfo &v) const;
	std::ostream &print(std::ostream &s, const BoolVar &v) const;
	std::ostream &print(std::ostream &s, const IntVar &v) const;
	std::ostream &print(std::ostream &s, const IntegralVar &v) const;
	std::ostream &print(std::ostream &s, const FloatVar &v) const;
	std::ostream &print(std::ostream &s, const Var &v) const;
	std::ostream &print(std::ostream &s, const Conjunction &v) const;
	std::ostream &print(std::ostream &s, const Disjunction &v) const;
	std::ostream &print(std::ostream &s, const LinConstraintInt &e) const;
	std::ostream &print(std::ostream &s, const LinConstraintFloat &e) const;
	std::ostream &print(std::ostream &s, const LinConstraintAny &e) const;
	std::ostream &print(std::ostream &s, const LinExpInt &e) const;
	std::ostream &print(std::ostream &s, const LinExpFloat &e) const;
	std::ostream &print(std::ostream &s, const LinExpAny &e) const;
	std::ostream &print(std::ostream &s, const ImplicationInt &e) const;
	std::ostream &print(std::ostream &s, const ImplicationFloat &e) const;
	std::ostream &print(std::ostream &s, const ImplicationBool &e) const;
	std::ostream &print(std::ostream &s, const Equivalence &e) const;
private:
	std::vector<VariableInfo> vars;
	// keep separate arrays for bool and int just to make it easy to iterate variables of a certain type
	std::vector<BoolVarInfo> boolVars;
	std::vector<IntVarInfo> intVars;
	std::vector<FloatVarInfo> floatVars;
private:
	std::vector<Conjunction> conjunctions;
	std::vector<Disjunction> disjunctions;
	std::vector<LinConstraintInt> linConstraintsInt;
	std::vector<LinConstraintFloat> linConstraintsFloat;
	std::vector<LinConstraintAny> linConstraintsAny;
	std::vector<ImplicationInt> implicationsInt;
	std::vector<ImplicationFloat> implicationsFloat;
	std::vector<ImplicationBool> implicationsBool;
	std::vector<Equivalence> equivalences;
	LinExpAny objectiveFunction;
private:
	friend ILPImplementation makeILPModel(const Model &model, Int bigM, const std::string &solver);
public:
	static void compare(const Model &ma, const Model &mb);
};

} // namespace mod::lib::CombiOpt

#endif // MOD_LIB_COMBIOPT_MODEL_HPP