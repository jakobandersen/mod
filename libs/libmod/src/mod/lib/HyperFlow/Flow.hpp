#ifndef MOD_LIB_HYPERFLOW_FLOW_HPP
#define MOD_LIB_HYPERFLOW_FLOW_HPP

#include <mod/lib/HyperFlow/Model.hpp>
#include <mod/lib/HyperFlow/Specification.hpp>
#include <mod/lib/HyperFlow/Modules/Base.hpp>
#include <mod/lib/HyperFlow/Modules/OverallCatalysis.hpp>
#include <mod/lib/HyperFlow/Modules/OverallAutocatalysis.hpp>
#include <mod/lib/IO/Json.hpp>

// This is our use of the rest of the HyperFlow library.

namespace mod::lib::HyperFlow {

struct Flow {
	explicit Flow(std::shared_ptr<dg::DG> dg_, const std::string &ilpSolver);
	bool isSpecWritable() const;
	// pre: isSpecWritable()
	Specification &writeSpec();
	void enableOverallAutocatalysis(bool setDefaults);
	void enableOverallCatalysis(bool setDefaults);
	// pre: isSpecWritable()
	// post: !isSpecWritable()
	void makeModel(int verbosity);
public:
	// pre: !isSpecWritable()
	Model &getModel();
	const Model &getModel() const;
public:
	void listModel(std::ostream &s) const;
	const Specification &readSpec() const;
	nlohmann::json dump() const;
	// TODO: this should return flow once the old FlowBase is gone
	static bool load(Flow &flow, const nlohmann::json &j, int verbosity, std::ostream &err);
public:
	const std::shared_ptr<dg::DG> dg_;
	const std::string ilpSolver;
private:
	std::unique_ptr<Specification> spec;
	std::unique_ptr<Model> mod;
};

} // namespace mod::lib::HyperFlow

#endif // MOD_LIB_HYPERFLOW_FLOW_HPP