#ifndef MOD_LIB_RULE_IO_WRITE_HPP
#define MOD_LIB_RULE_IO_WRITE_HPP

#include <mod/lib/IO/GraphWrite.hpp>
#include <mod/lib/Rule/LabelledRule.hpp>

namespace mod::lib::rule {
struct Rule;
} // namespace mod::lib::rule
namespace mod::lib::rule::Write {

using Options = IO::Graph::Write::Options;
using CombinedVertex = lib::DPO::CombinedRule::CombinedVertex;
using CombinedEdge = lib::DPO::CombinedRule::CombinedEdge;

struct BaseArgs {
	std::function<bool(CombinedVertex)> visible;
	std::function<std::string(CombinedVertex)> vColour;
	std::function<std::string(CombinedEdge)> eColour;
};

// returns the filename _with_ extension
void gml(const Rule &r, bool withCoords, std::ostream &s);
std::string gml(const Rule &r, bool withCoords);
// returns the filename without extension
std::string dotCombined(const Rule &r);
std::string svgCombined(const Rule &r);
std::string pdfCombined(const Rule &r);
// returns the filename _without_ extension
std::string
dot(const Rule &r, const Options &options); // does not handle labels correctly, is for coordinate generation
std::string coords(const Rule &r, int idOffset, const Options &options,
                   std::function<bool(CombinedVertex)> disallowCollapse_);
std::pair<std::string, std::string>
tikz(const std::string &fileCoordsNoExt, const Rule &r, unsigned int idOffset, const Options &options,
     const std::string &suffixL, const std::string &suffixK, const std::string &suffixR, const BaseArgs &args,
     std::function<bool(CombinedVertex)> disallowCollapse);
std::pair<std::string, std::string> tikz(const Rule &r, unsigned int idOffset, const Options &options,
                                         const std::string &suffixL, const std::string &suffixK,
                                         const std::string &suffixR, const BaseArgs &args,
                                         std::function<bool(CombinedVertex)> disallowCollapse);
std::string pdf(const Rule &r, const Options &options,
                const std::string &suffixL, const std::string &suffixK, const std::string &suffixR,
                const BaseArgs &args);
std::pair<std::string, std::string>
tikzTransitionState(const std::string &fileCoordsNoExt, const Rule &r, unsigned int idOffset,
                    const Options &options,
                    const std::string &suffix, const BaseArgs &args);
std::pair<std::string, std::string>
tikzTransitionState(const Rule &r, unsigned int idOffset, const Options &options,
                    const std::string &suffix, const BaseArgs &args);
std::string pdfTransitionState(const Rule &r, const Options &options,
                               const std::string &suffix, const BaseArgs &args);
//std::string pdfCombined(const Real &r, const Options &options); // TODO
std::pair<std::string, std::string> summary(const Rule &r, bool printCombined);
std::pair<std::string, std::string>
summary(const Rule &r, const Options &first, const Options &second, bool printCombined);
void termState(const Rule &r);

std::string stereoSummary(const Rule &r, lib::DPO::CombinedRule::CombinedVertex v, Membership m,
                          const IO::Graph::Write::Options &options);

} // namespace mod::lib::rule::Write

#endif // MOD_LIB_RULE_IO_WRITE_HPP