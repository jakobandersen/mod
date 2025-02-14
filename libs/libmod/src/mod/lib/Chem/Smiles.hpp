#ifndef MOD_LIB_CHEM_SMILES_HPP
#define MOD_LIB_CHEM_SMILES_HPP

#include <mod/lib/Graph/IO/Read.hpp>
#include <mod/lib/IO/Result.hpp>

#include <string>
#include <vector>

namespace mod {
struct AtomId;
enum class SmilesClassPolicy;
} // namespace mod
namespace mod::lib::graph {
struct PropMolecule;
struct PropString;
} // namespace mod::lib::graph
namespace mod::lib::Chem {

struct SmilesWriteResult {
	std::string str;
	bool isAbstract;
};

lib::IO::Result<SmilesWriteResult> getSmiles(const lib::graph::GraphType &g,
                                             const lib::graph::PropString &pString,
                                             const lib::graph::PropMolecule &pMol,
                                             const std::vector<int> *ranks, bool withIds);
lib::IO::Result<std::vector<lib::graph::Read::Data>>
readSmiles(lib::IO::Warnings &warnings, bool printStereoWarnings,
           std::string_view smiles, bool allowAbstract, SmilesClassPolicy classPolicy);
const std::vector<AtomId> &getSmilesOrganicSubset();
bool isInSmilesOrganicSubset(AtomId atomId);
void addImplicitHydrogens(lib::graph::GraphType &g, lib::graph::PropString &pString, lib::graph::Vertex v,
                          AtomId atomId, std::function<void(lib::graph::GraphType &, lib::graph::PropString &,
                                                            lib::graph::Vertex)> hydrogenAdder);

} // namespace mod::lib::Chem

#endif // MOD_LIB_CHEM_SMILES_HPP