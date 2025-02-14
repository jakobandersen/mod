#ifndef MOD_LIB_GRAPH_IO_WRITE_HPP
#define MOD_LIB_GRAPH_IO_WRITE_HPP

#include <mod/lib/LabelledUnionGraph.hpp>
#include <mod/lib/Graph/GraphDecl.hpp>
#include <mod/lib/IO/GraphWrite.hpp>
#include <mod/lib/Stereo/Configuration/Configuration.hpp>

#include <memory>
#include <ostream>
#include <string>
#include <string_view>

namespace mod::lib::graph {
struct LabelledGraph;
struct Graph;
} // namespace mod::lib::graph
namespace mod::lib::graph::Write {
struct DepictionData;
using Options = lib::IO::Graph::Write::Options;

// all return the filename _with_ extension
void gml(const LabelledGraph &gLabelled, const DepictionData &depict,
         const std::size_t gId, bool withCoords, std::ostream &s);
std::string gml(const Graph &g, bool withCoords);
std::pair<std::string, bool> dfs(const LabelledGraph &gLabelled, bool withIds);

std::string dot(const LabelledGraph &gLabelled, const std::size_t gId, const Options &options);
std::string coords(const LabelledGraph &gLabelled, const DepictionData &depict,
                   const std::size_t gId, const Options &options);
std::pair<std::string, std::string> tikz(const LabelledGraph &gLabelled,
                                         const DepictionData &depict,
                                         const std::size_t gId, const Options &options,
                                         bool asInline, const std::string &idPrefix);
std::string pdf(const LabelledGraph &gLabelled, const DepictionData &depict,
                const std::size_t gId, const Options &options);
std::string svg(const LabelledGraph &gLabelled, const DepictionData &depict,
                const std::size_t gId, const Options &options);
std::pair<std::string, std::string> summary(const Graph &g, const Options &first, const Options &second);
void termState(const Graph &g);
void termState(const lib::LabelledUnionGraph<lib::graph::LabelledGraph> &g, std::string name);

std::string stereoSummary(const Graph &g, Vertex v, const lib::Stereo::Configuration &conf,
                          const IO::Graph::Write::Options &options, int shownIdOffset, const std::string &nameSuffix);

// simplified interface for lib::graph::Graph
void gml(const Graph &g, bool withCoords, std::ostream &s);
std::string tikz(const Graph &g, const Options &options, bool asInline, const std::string &idPrefix);
std::string pdf(const Graph &g, const Options &options);
std::string svg(const Graph &g, const Options &options);

} // namespace mod::lib::graph::Write

#endif // MOD_LIB_GRAPH_IO_WRITE_HPP