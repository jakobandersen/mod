#ifndef MOD_LIB_DG_IO_READ_HPP
#define MOD_LIB_DG_IO_READ_HPP

#include <mod/Post.hpp>
#include <mod/lib/DG/Hyper.hpp>

#include <mod/lib/IO/IO.hpp>
#include <mod/lib/IO/Json.hpp>

#include <iosfwd>
#include <optional>
#include <string>
#include <unordered_map>

namespace mod::lib::DG {
struct Hyper;
struct NonHyper;
} // namespace mod::lib::DG
namespace mod::lib::DG::Read {

struct AbstractDerivation {
	using List = std::vector<std::pair<unsigned int, std::string>>;
public:
	std::optional<std::string> id;
	List left;
	bool reversible;
	List right;
};

std::optional<nlohmann::json> loadDump(const std::string &file, std::ostream &err);

std::unique_ptr<NonHyper> dump(const std::vector<std::shared_ptr<mod::graph::Graph> > &graphDatabase,
                               const std::vector<std::shared_ptr<mod::rule::Rule> > &ruleDatabase,
                               const std::string &file,
                               IsomorphismPolicy graphPolicy,
                               std::ostream &err, int verbosity);
std::optional<std::vector<AbstractDerivation>> abstract(const std::string &s, std::ostream &err);

// utilities for those referring to DG elements in their dumps
bool dumpDigest(const HyperGraphType &dg, const nlohmann::json &j, std::ostream &err, const std::string &errType);
std::optional<HyperVertex>
vertexOrEdge(const HyperGraphType &dg, std::size_t id, std::ostream &err, const std::string &errPrefix);
std::optional<HyperVertex>
vertex(const HyperGraphType &dg, std::size_t id, std::ostream &err, const std::string &errPrefix);
std::optional<HyperVertex>
edge(const HyperGraphType &dg, std::size_t id, std::ostream &err, const std::string &errPrefix);

} // namespace mod::lib::DG::Read

#endif // MOD_LIB_DG_IO_READ_HPP