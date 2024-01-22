#ifndef MOD_LIB_GRAPH_IO_READ_HPP
#define MOD_LIB_GRAPH_IO_READ_HPP

#include <mod/lib/Graph/GraphDecl.hpp>
#include <mod/lib/IO/Result.hpp>

#include <memory>
#include <ostream>
#include <string>
#include <string_view>

namespace mod {
enum class SmilesClassPolicy;
struct MDLOptions;
} // namespace mod
namespace mod::lib::Graph {
struct PropStereo;
struct PropString;
} // namespace mod::lib::Graph
namespace mod::lib::Graph::Read {

struct Data {
	Data();
	Data(std::unique_ptr<lib::Graph::GraphType> graph, std::unique_ptr<lib::Graph::PropString> label);
	Data(Data &&other);
	~Data();
	void reset();
public:
	std::unique_ptr<lib::Graph::GraphType> g;
	std::unique_ptr<lib::Graph::PropString> pString;
	std::unique_ptr<lib::Graph::PropStereo> pStereo;
	std::map<int, std::size_t> externalToInternalIds;
};

struct RXNFileData {
	std::vector<std::vector<Data>> reactants, products;
	// maps external IDs to )productOffset, external ID)
	// if no map, then (-1, -1)
	// std::vector<std::vector<std::pair<int, int>>> aamap;
};

lib::IO::Result<std::vector<Data>> gml(lib::IO::Warnings &warnings, std::string_view src, bool printStereoWarnings);
lib::IO::Result<std::vector<Data>> dfs(lib::IO::Warnings &warnings, std::string_view src);
lib::IO::Result<std::vector<Data>>
smiles(lib::IO::Warnings &warnings, std::string_view smiles, bool printStereoWarnings,
       bool allowAbstract, SmilesClassPolicy classPolicy);
lib::IO::Result<std::vector<Data>> MDLMOL(lib::IO::Warnings &warnings, std::string_view src, const MDLOptions &options);
lib::IO::Result<std::vector<std::vector<Data>>>
MDLSD(lib::IO::Warnings &warnings, std::string_view src, const MDLOptions &options);

} // namespace mod::lib::Graph::Read

#endif // MOD_LIB_GRAPH_IO_READ_HPP