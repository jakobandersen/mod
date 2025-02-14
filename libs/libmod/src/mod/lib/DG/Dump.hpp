#ifndef MOD_LIB_DG_DUMP_HPP
#define MOD_LIB_DG_DUMP_HPP

#include <mod/graph/ForwardDecl.hpp>
#include <mod/rule/ForwardDecl.hpp>

#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

namespace mod::lib::DG {
class NonHyper;
} // namespace mod::lib::DG
namespace mod::lib::DG::Dump {

std::unique_ptr<NonHyper> load(const std::vector<std::shared_ptr<mod::graph::Graph> > &graphs,
                               const std::vector<std::shared_ptr<mod::rule::Rule> > &rules,
                               const std::string &file,
                               std::ostream &err);

} // namespace mod::lib::DG::Dump

#endif // MOD_LIB_DG_DUMP_HPP