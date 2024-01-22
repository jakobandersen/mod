#ifndef MOD_LIB_TERM_IO_WRITE_HPP
#define MOD_LIB_TERM_IO_WRITE_HPP

#include <mod/lib/IO/IO.hpp>
#include <mod/lib/Term/RawTerm.hpp>
#include <mod/lib/Term/WAM.hpp>

#include <functional>
#include <optional>

namespace mod::lib {
struct StringStore;
} // namespace mod::lib
namespace mod::lib::Term::Write {

std::ostream &rawTerm(const RawTerm &term, const StringStore &strings, std::ostream &s);
std::ostream &element(Cell cell, const StringStore &strings, std::ostream &s);
void wam(const Wam &machine, const StringStore &strings, IO::Logger logger);
void wam(const Wam &machine, const StringStore &strings, IO::Logger logger,
         std::function<void(Address, std::ostream &s)> addressCallback);
std::ostream &term(const Wam &machine, Address addr, const StringStore &strings, std::ostream &s);
std::ostream &mgu(const Wam &machine, const MGU &mgu, const StringStore &strings, std::ostream &s);

} // namespace mod::lib::Term::Write

#endif // MOD_LIB_TERM_IO_WRITE_HPP