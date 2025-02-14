#ifndef MOD_LIB_CHEM_MDL_HPP
#define MOD_LIB_CHEM_MDL_HPP

#include <mod/lib/Graph/IO/Read.hpp>
#include <mod/lib/IO/Result.hpp>

#include <string_view>
#include <vector>

namespace mod::lib::Chem {

auto readMDLMOL(lib::IO::Warnings &warnings, std::string_view src,
                const MDLOptions &options) -> lib::IO::Result<std::vector<lib::graph::Read::Data>>;
auto readMDLSD(lib::IO::Warnings &warnings, std::string_view src,
               const MDLOptions &options) -> lib::IO::Result<std::vector<std::vector<lib::graph::Read::Data>>>;

} // namespace mod::lib::Chem

#endif // MOD_LIB_CHEM_MDL_HPP