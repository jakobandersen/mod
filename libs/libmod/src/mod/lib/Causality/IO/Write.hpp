#ifndef MOD_LIB_CAUSALITY_IO_WRITE_HPP
#define MOD_LIB_CAUSALITY_IO_WRITE_HPP

#include <mod/lib/DG/GraphDecl.hpp>

#include <map>
#include <string>
#include <vector>

namespace mod::lib::DG {
struct Hyper;
} // mod::lib::DG
namespace mod::lib::Causality {
struct EventTrace;
} // namespace mod::lib::Causality
namespace mod::lib::Causality::Write {


struct EventTracePrinter {
	std::string allOptions(const lib::DG::Hyper &dg) const;
	bool isVertexVisible(lib::DG::HyperVertex v, const lib::DG::Hyper &dg) const;
	std::string allVertexOptions(lib::DG::HyperVertex v, const lib::DG::Hyper &dg) const;
public:
	int maxPointsPerMolecule = 500;
	bool logTime = false;
	bool logCount = false;
	std::vector<std::function<std::string(const lib::DG::Hyper &)>> options;
	std::vector<std::function<bool(lib::DG::HyperVertex, const lib::DG::Hyper &)>> vertexVisibles;
	std::vector<std::function<std::string(lib::DG::HyperVertex, const lib::DG::Hyper &)>> vertexOptions;
	std::function<std::string(const lib::DG::Hyper &)> preContent, postContent;
};

using EventTraceData = std::map<lib::DG::HyperVertex, std::vector<std::pair<double, int>>>;

std::string makeEventTraceFilename(const lib::Causality::EventTrace &trace, const std::string &prefix,
                                   const lib::DG::HyperVertex v);
// Returns the common prefix for all the printed files:
// - <prefix>v-<idx>.txt, for each graph in the data
std::pair<std::string, EventTraceData>
dataEventTrace(const lib::Causality::EventTrace &trace, int maxPointsPerMolecule);
std::string texEventTrace(const lib::Causality::EventTrace &trace, const EventTracePrinter &printer,
                          const std::string &prefix, EventTraceData data);
std::string pdfEventTrace(const lib::Causality::EventTrace &trace, const EventTracePrinter &printer);
std::string summaryEventTrace(const lib::Causality::EventTrace &trace, const EventTracePrinter &printer);

} // namespace namespace mod::lib::Causality

#endif // MOD_LIB_CAUSALITY_IO_WRITE_HPP