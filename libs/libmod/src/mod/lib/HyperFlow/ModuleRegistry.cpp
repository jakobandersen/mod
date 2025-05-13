#include "ModuleRegistry.hpp"

#include <mod/Error.hpp>

#include <cassert>

namespace mod::lib::HyperFlow {

const ModuleRegistry::VariableSetVertex *ModuleRegistry::VariableSet::asVertex() const {
	assert(varType == VarType::Vertex);
	return static_cast<const VariableSetVertex *>(this);
}

const ModuleRegistry::VariableSetEdge *ModuleRegistry::VariableSet::asEdge() const {
	assert(varType == VarType::Edge);
	return static_cast<const VariableSetEdge *>(this);
}

const ModuleRegistry::VariableSetCustom *ModuleRegistry::VariableSet::asCustom() const {
	assert(varType == VarType::Custom);
	return static_cast<const VariableSetCustom *>(this);
}

CombiOpt::LinExpAny ModuleRegistry::VariableSet::compileExpr(const Model &m) const {
	return compileExprImpl(m);
}

CombiOpt::LinExpAny ModuleRegistry::VariableSetVertex::compileExpr(const Model &m, lib::DG::HyperVertex v) const {
	return compileExprImpl(m, v);
}

CombiOpt::LinExpAny ModuleRegistry::VariableSetEdge::compileExpr(const Model &m, lib::DG::HyperVertex e) const {
	return compileExprImpl(m, e);
}

CombiOpt::LinExpAny ModuleRegistry::VariableSetCustom::compileExpr(const Model &m, const std::string &name) const {
	return compileExprImpl(m, name);
}

// ============================================================================

void ModuleRegistry::addModule(std::type_index id, std::string name,
                               std::function<void(Specification &, bool)> enable) {
	if(modules.find(id) != end(modules))
		throw LogicError("Module '" + name + "' already added.");
	assert(idFromName.find(name) == end(idFromName));
	idFromName.emplace(name, id);
	modules.emplace(id, Module{id, name, enable});
}

void ModuleRegistry::addVariableSet(std::string name, std::unique_ptr<VariableSet> vs) {
	if(modules.find(vs->owner) == end(modules))
		throw LogicError("Module for new variable set '" + name + "' has not been added.");
	if(variableSets.find(name) != end(variableSets))
		throw LogicError("Variable set '" + name + "' already added.");
	variableSets.emplace(std::move(name), std::move(vs));
}

const ModuleRegistry::Module *ModuleRegistry::getModule(std::type_index id) const {
	const auto iter = modules.find(id);
	return iter == end(modules) ? nullptr : &iter->second;
}

const ModuleRegistry::Module *ModuleRegistry::getModule(const std::string &name) const {
	const auto iter = idFromName.find(name);
	if(iter == end(idFromName)) return nullptr;
	const auto *m = getModule(iter->second);
	assert(m);
	return m;
}

const ModuleRegistry::VariableSet *ModuleRegistry::getVariableSet(const std::string &name) const {
	const auto iter = variableSets.find(name);
	if(iter == end(variableSets)) return nullptr;
	return iter->second.get();
}

const std::unordered_map<std::type_index, ModuleRegistry::Module> &ModuleRegistry::getAllModules() const {
	return modules;
}

ModuleRegistry &ModuleRegistry::get() {
	static ModuleRegistry store;
	return store;
}

std::ostream &operator<<(std::ostream &s, ModuleRegistry::VarType vt) {
	switch(vt) {
	case ModuleRegistry::VarType::Vertex:
		return s << "vertex";
	case ModuleRegistry::VarType::Edge:
		return s << "edge";
	case ModuleRegistry::VarType::Custom:
		return s << "custom";
	}
	return s << "<unknown>";
}

} // namespace mod::lib::HyperFlow