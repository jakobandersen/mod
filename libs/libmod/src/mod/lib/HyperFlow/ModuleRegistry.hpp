#ifndef MOD_LIB_HYPERFLOW_MODULEREGISTRY_HPP
#define MOD_LIB_HYPERFLOW_MODULEREGISTRY_HPP

#include <mod/lib/CombiOpt/LinExpAny.hpp>
#include <mod/lib/DG/GraphDecl.hpp>

#include <functional>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace mod::lib::graph {
struct Graph;
} // namespace mod::lib::graph
namespace mod::lib::HyperFlow {
struct Specification;
struct Model;

struct ModuleRegistry {
	struct Module {
		std::type_index id;
		std::string name;
		// spec, setDefaults
		std::function<void(Specification &, bool)> enable;
	};

	enum struct VarType {
		Vertex, Edge, Custom
	};
	struct VariableSetVertex;
	struct VariableSetEdge;
	struct VariableSetCustom;

	class VariableSet {
		friend class VariableSetVertex;
		friend class VariableSetEdge;
		friend class VariableSetCustom;
		VariableSet(VarType varType, std::type_index owner)
				: varType(varType), owner(owner) {}
	public:
		virtual ~VariableSet() = default;
		const VariableSetVertex *asVertex() const;
		const VariableSetEdge *asEdge() const;
		const VariableSetCustom *asCustom() const;
		// requires the owner to be a module of the given model
		CombiOpt::LinExpAny compileExpr(const Model &m) const;
	private:
		virtual CombiOpt::LinExpAny compileExprImpl(const Model &m) const = 0;
	public:
		const VarType varType;
		const std::type_index owner;
	};

	struct VariableSetVertex : VariableSet {
		VariableSetVertex(std::type_index owner) : VariableSet(VarType::Vertex, owner) {}
		using VariableSet::compileExpr;
		// requires the owner to be a module of the given model
		// also requires v to be a valid HyperVertex in the underlying DG of the model
		CombiOpt::LinExpAny compileExpr(const Model &m, lib::DG::HyperVertex v) const;
	private:
		using VariableSet::compileExprImpl;
		virtual CombiOpt::LinExpAny compileExprImpl(const Model &m, const lib::DG::HyperVertex &v) const = 0;
	};

	struct VariableSetEdge : VariableSet {
		VariableSetEdge(std::type_index owner) : VariableSet(VarType::Edge, owner) {}
		using VariableSet::compileExpr;
		// requires the owner to be a module of the given model
		// also requires v to be a valid HyperEdge in the underlying DG of the model
		CombiOpt::LinExpAny compileExpr(const Model &m, lib::DG::HyperVertex e) const;
	private:
		using VariableSet::compileExprImpl;
		virtual CombiOpt::LinExpAny compileExprImpl(const Model &m, const lib::DG::HyperVertex &e) const = 0;
	};

	struct VariableSetCustom : VariableSet {
		VariableSetCustom(std::type_index owner) : VariableSet(VarType::Custom, owner) {}
		using VariableSet::compileExpr;
		// requires the owner to be a module of the given model
		CombiOpt::LinExpAny compileExpr(const Model &m, const std::string &name) const;
	private:
		using VariableSet::compileExprImpl;
		virtual CombiOpt::LinExpAny compileExprImpl(const Model &m, const std::string &name) const = 0;
	};
private:
	ModuleRegistry() = default;
	ModuleRegistry(ModuleRegistry &&) = delete;
	ModuleRegistry &operator=(ModuleRegistry &&) = delete;
public:
	// throws LogicError if either the type_index or name is already taken
	// enable(spec, setDefaults) must be basically equivalent to
	//   spec.addModule<Class>(setDefaults)
	void addModule(std::type_index ti, std::string name, std::function<void(Specification &, bool)> enable);
	// throws LogicError if the module has not been added or the name already exists
	void addVariableSet(std::string name, std::unique_ptr<VariableSet> vs);
public:
	const Module *getModule(std::type_index id) const;
	const Module *getModule(const std::string &name) const;
	const VariableSet *getVariableSet(const std::string &name) const;
	const std::unordered_map<std::type_index, Module> &getAllModules() const;
public:
	static ModuleRegistry &get();
private:
	std::unordered_map<std::type_index, Module> modules;
	std::unordered_map<std::string, std::type_index> idFromName;
	std::unordered_map<std::string, std::unique_ptr<VariableSet>> variableSets;
};

std::ostream &operator<<(std::ostream &s, ModuleRegistry::VarType vt);

template<typename Base, typename Index, typename WholeSet, typename Indexed>
struct VariableSetHelper final : Base {
	VariableSetHelper(std::type_index owner, WholeSet ws, Indexed idxed)
			: Base(owner), ws(ws), indxed(idxed) {}
private:
	virtual CombiOpt::LinExpAny compileExprImpl(const Model &m) const override {
		return ws(m);
	}

	virtual CombiOpt::LinExpAny compileExprImpl(const Model &m, const Index &idx) const override {
		return indxed(m, idx);
	}
private:
	WholeSet ws;
	Indexed indxed;
};

template<typename WholeSet, typename Indexed>
auto makeVertexVariableSet(std::type_index owner, WholeSet ws, Indexed idx) {
	return std::make_unique<VariableSetHelper<
			ModuleRegistry::VariableSetVertex, lib::DG::HyperVertex, WholeSet, Indexed>>(owner, ws, idx);
}

template<typename WholeSet, typename Indexed>
auto makeEdgeVariableSet(std::type_index owner, WholeSet ws, Indexed idx) {
	return std::make_unique<VariableSetHelper<
			ModuleRegistry::VariableSetEdge, lib::DG::HyperVertex, WholeSet, Indexed>>(owner, ws, idx);
}

template<typename WholeSet, typename Indexed>
auto makeCustomVariableSet(std::type_index owner, WholeSet ws, Indexed idx) {
	return std::make_unique<VariableSetHelper<
			ModuleRegistry::VariableSetCustom, const std::string &, WholeSet, Indexed>>(owner, ws, idx);
}

} // namespace mod::lib::HyperFlow

#endif // MOD_LIB_HYPERFLOW_MODULEREGISTRY_HPP