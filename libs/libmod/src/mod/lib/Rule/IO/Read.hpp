#ifndef MOD_LIB_RULE_IO_READ_HPP
#define MOD_LIB_RULE_IO_READ_HPP

#include <mod/Config.hpp>
#include <mod/lib/IO/Result.hpp>
#include <mod/lib/Rule/LabelledRule.hpp>

#include <optional>
#include <ostream>
#include <string>
#include <string_view>

namespace mod::lib::rule {
struct Rule;
} // namespace mod::lib::rule
namespace mod::lib::rule::Read {

struct Data {
	std::optional<lib::rule::LabelledRule> rule;
	std::optional<std::string> name;
	std::optional<LabelType> labelType;
	// maps external IDs to vertex IDs in the combined graph
	std::map<int, std::size_t> externalToInternalIds;
};

lib::IO::Result<Data> gml(lib::IO::Warnings &warnings, std::string_view input, bool printStereoWarnings);
lib::IO::Result<Data> dfs(lib::IO::Warnings &warnings, std::string_view input);

} // namespace mod::lib::rule::Read

#endif // MOD_LIB_RULE_IO_READ_HPP