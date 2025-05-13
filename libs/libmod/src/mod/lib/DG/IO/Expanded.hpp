#ifndef MOD_LIB_DG_IO_EXPANDED_HPP
#define MOD_LIB_DG_IO_EXPANDED_HPP

#include <mod/lib/DG/GraphDecl.hpp>

#include <string>

namespace mod::lib::DG {
struct Expanded;
} // namespace mod::lib::DG
namespace mod::lib::DG::Write {
struct Data;
struct Options;
struct Printer;
} // namespace mod::lib::DG::Write
namespace mod::lib::DG::Write::Expanded {

std::pair<std::string, std::string> tikz(const lib::DG::Expanded &dg, const Options &options);
std::string pdf(const lib::DG::Expanded &dg, const Data &data, Printer &printer);

std::string tikzVertex(const lib::DG::Expanded &dgExpandedWrapped, lib::DG::HyperVertex vHyper);
std::string pdfVertex(const lib::DG::Expanded &dg, lib::DG::HyperVertex vHyper);

} // namespace mod::lib::DG::Write::Expanded

#endif // MOD_LIB_DG_IO_EXPANDED_HPP