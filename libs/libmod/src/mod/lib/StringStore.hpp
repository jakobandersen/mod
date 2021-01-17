#ifndef MOD_LIB_STRINGSTORE_HPP
#define MOD_LIB_STRINGSTORE_HPP

#include <map>
#include <string>
#include <vector>

namespace mod::lib {

struct StringStore {
	StringStore() = default;
	StringStore(const StringStore&) = delete;
	StringStore(StringStore&&) = delete;
	StringStore &operator=(const StringStore&) = delete;
	StringStore &operator=(StringStore&&) = delete;
	bool hasString(const std::string &s) const;
	std::size_t getIndex(const std::string &s) const;
	const std::string &getString(std::size_t index) const;
private:
	mutable std::vector<std::string> strings;
	mutable std::map<std::string, std::size_t> index;
};

} // namespace mod::lib

#endif // MOD_LIB_STRINGSTORE_HPP