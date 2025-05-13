#ifndef MOD_LIB_HYPERFLOW_IO_WRITE_HPP
#define MOD_LIB_HYPERFLOW_IO_WRITE_HPP

#include <mod/dg/DG.hpp>
#include <mod/lib/DG/Hyper.hpp>

#include <memory>
#include <string>
#include <vector>

namespace mod::lib::HyperFlow {
struct Flow;
struct Model;
} // namespace mod::lib::HyperFlow
namespace mod::lib::DG::Write {
struct Data;
struct Printer;
} // namespace mod::lib::DG::Write
namespace mod::lib::graph::Write {
struct Options;
} // namespace mod::lib::graph::Write
namespace mod::lib::HyperFlow::Write {

struct Printer {
	Printer() = default;
	void pushInEdgeLabel(std::function<std::string(dg::DG::Vertex)> f);
	void popInEdgeLabel();
	void pushOutEdgeLabel(std::function<std::string(dg::DG::Vertex)> f);
	void popOutEdgeLabel();
public:
	bool withFlowLabel = true;
	bool printUnfiltered = false, printFiltered = true;
	std::string unfilteredFlowColour = "blue";
public:
	std::vector<std::function<std::string(dg::DG::Vertex)>> inEdgeLabels;
	std::vector<std::function<std::string(dg::DG::Vertex)>> outEdgeLabels;
};

std::pair<std::optional<std::string>, std::optional<std::string>>
summary(const lib::HyperFlow::Model &model, const int solId, const DG::Write::Data &dgData,
        DG::Write::Printer &dgPrinter,
        const IO::Graph::Write::Options &graphOptions,
        Printer &flowPrinter);

} // namespace mod::lib::HyperFlow::Write

#endif // MOD_LIB_HYPERFLOW_IO_WRITE_HPP