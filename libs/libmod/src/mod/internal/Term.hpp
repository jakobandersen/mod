#ifndef MOD_INTERNAL_TERM_HPP
#define MOD_INTERNAL_TERM_HPP

#include <mod/Config.hpp>

namespace mod::lib::Term {
struct Wam;
} // namespace mod::lib::Term
namespace mod::internal {

// Only here for testing purposes, don't rely on them being stable.
MOD_DECL std::size_t StringStore_getIndex(const std::string &s);
MOD_DECL void Wam_write(const lib::Term::Wam &machine, int indent);

} // namespace mod::internal

#endif // MOD_INTERNAL_TERM_HPP