#include "Term.hpp"

#include <mod/lib/StringStore.hpp>
#include <mod/lib/Term/WAM.hpp>
#include <mod/lib/Term/IO/Write.hpp>

#include <iostream>

namespace mod::internal {

std::size_t StringStore_getIndex(const std::string &s) {
	return lib::Term::getStrings().getIndex(s);
}

void Wam_write(const lib::Term::Wam &machine, int indent) {
	lib::IO::Logger logger(std::cout);
	logger.indentLevel += indent;
	lib::Term::Write::wam(machine, lib::Term::getStrings(), logger);
}

} // namespace mod::internal