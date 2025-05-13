#ifndef MOD_LIB_HYPERFLOW_IO_READ_HPP
#define MOD_LIB_HYPERFLOW_IO_READ_HPP

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
namespace mod::lib::HyperFlow::Read {

std::unique_ptr<Flow>
dump(std::shared_ptr<dg::DG> dg, const std::string &ilpSolver, std::istream &s, std::ostream &err, int verbosity);

} // namespace mod::lib::HyperFlow::Read

#endif // MOD_LIB_HYPERFLOW_IO_READ_HPP