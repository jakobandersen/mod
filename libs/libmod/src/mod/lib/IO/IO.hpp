#ifndef MOD_LIB_IO_IO_HPP
#define MOD_LIB_IO_IO_HPP

#include <iosfwd>
#include <string>

namespace mod::lib::IO {

std::string makeUniqueFilePrefix();
std::string escapeForLatex(const std::string &str);
std::string asLatexMath(const std::string &str);

std::ostream &nullStream();
std::ostream &post();

void postDisable();
void postEnable();
void postReopenCommandFile();

struct Logger {
	explicit Logger(std::ostream &s) : s(s) {}
	std::ostream &indent(int offset = 0) const;
	std::ostream &sep(char c) const;
public:
	std::ostream &s;
	int indentLevel = 0;
};

} // namespace mod::lib::IO

#endif // MOD_LIB_IO_IO_HPP
