#ifndef MOD_LIB_HYPERFLOW_SPECIFICATION_HPP
#define MOD_LIB_HYPERFLOW_SPECIFICATION_HPP

#include <mod/hyperflow/LinExp.hpp>
#include <mod/lib/DG/Expanded.hpp>
#include <mod/lib/IO/Json.hpp>

#include <iosfwd>
#include <map>
#include <memory>
#include <optional>
#include <vector>

namespace mod {
template<typename Sig>
struct Function;
} // namespace mod
namespace mod::lib::DG {
struct Hyper;
} // namespace mod::lib::DG
namespace mod::lib::HyperFlow {
struct Model;
struct SpecificationModule;
struct ModelModule;

// Contains high-level data and constraints (i.e., in terms of interface classes and indirect information used for the model)

struct Specification {
	struct EnumerationVar {
		hyperflow::Var v;
	};
	struct TransitEnumeration {
		lib::DG::HyperVertex v;
	};
public:
	explicit Specification(const lib::DG::Hyper &dgHyper);
	template<typename Module, typename ...Args>
	// args is the extra args to the constructor of Module
	auto addModule(bool setDefaults, Args &&...args);
	void addModulesFromLinExp(const hyperflow::LinExp &exp);
	template<typename Module>
	bool hasModule() const;
	bool hasModule(std::type_index id) const;
	template<typename Module>
	// requires: hasModule<Module>()
	Module &getModule();
	template<typename Module>
	const Module &getModule() const;
	// pre: hasModule(id)
	SpecificationModule &getModule(std::type_index id);
public:
	void setObjectiveFunction(hyperflow::LinExp exp);
	hyperflow::LinExp getObjectiveFunction() const; // may return the default if non has been set
	void addEnumerationVar(hyperflow::Var v);
	bool hasExplicitEnumerationVars() const;
	std::vector<EnumerationVar> getEnumerationVars() const;
	void addTransitEnumeration(lib::DG::HyperVertex v);
	const std::vector<TransitEnumeration> &getTransitEnumeration() const;
	void setAbsGap(int absGap); // use a negative number to unset
	std::optional<int> getAbsGap() const;
	void list(std::ostream &s) const;
	nlohmann::json dump() const;
	// TODO: once the old flow model is gone, make it
	// static std::unique_ptr<Specification> load(const lib::DG::Hyper &dg, const nlohmann::json &j, std::ostream &err);
	static bool load(Specification &spec, const nlohmann::json &j, std::ostream &err);
public:
	const lib::DG::Hyper &dgHyper;
	const int id;
private: // solution control
	std::optional<hyperflow::LinExp> objectiveFunction;
	std::optional<std::vector<EnumerationVar>> enumerationVars;
	std::vector<TransitEnumeration> transitEnumeration;
	std::optional<int> absGap;
private:
	friend class ModelCreator;
	std::map<std::type_index, std::unique_ptr<SpecificationModule> > modules;
};

template<typename Module, typename ...Args>
auto Specification::addModule(bool setDefaults, Args &&...args) {
	assert(!hasModule<Module>());
	Module *m = new Module(*this, setDefaults, std::forward<Args>(args)...);
	modules.emplace(std::type_index(typeid(Module)), m);
	return m;
}

template<typename Module>
bool Specification::hasModule() const {
	return hasModule(std::type_index(typeid(Module)));
}

template<typename Module>
Module &Specification::getModule() {
	auto iter = modules.find(std::type_index(typeid(Module)));
	assert(iter != modules.end());
	assert(dynamic_cast<Module *> (iter->second.get()));
	return *static_cast<Module *> (iter->second.get());
}

template<typename Module>
const Module &Specification::getModule() const {
	auto iter = modules.find(std::type_index(typeid(Module)));
	assert(iter != modules.end());
	assert(dynamic_cast<Module *> (iter->second.get()));
	return *static_cast<Module *> (iter->second.get());
}

struct Transits {
	lib::DG::Expanded::TransitSet expressed, deleted;
};

class ModelCreator {
	friend class Model;
	static Transits getTransits(const Specification &spec);
	static std::map<std::type_index, std::unique_ptr<ModelModule> >
	/**/ createModules(const Specification &spec, Model &owner);
};

} // namespace mod::lib::HyperFlow

#endif // MOD_LIB_HYPERFLOW_SPECIFICATION_HPP