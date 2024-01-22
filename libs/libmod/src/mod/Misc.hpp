#ifndef MOD_MISC_H
#define MOD_MISC_H

#include <mod/BuildConfig.hpp>
#include <mod/graph/ForwardDecl.hpp>
#include <mod/dg/ForwardDecl.hpp>

#include <memory>
#include <string>

namespace mod {

// rst: .. function:: std::string version()
// rst:
// rst:		:returns: the version of MØD.
MOD_DECL std::string version();

// rst: .. function:: void rngReseed(unsigned int seed)
// rst:
// rst:		Reseed the random bit generator used in the library.
MOD_DECL void rngReseed(unsigned int seed);
// rst: .. function:: double rngUniformReal()
// rst:
// rst:		:returns: a uniformly random real number between 0 and 1.
MOD_DECL double rngUniformReal();

MOD_DECL std::string strFromDump(const std::string &file);
MOD_DECL void showDump(const std::string &file);

MOD_DECL void printGeometryGraph();


} // namespace mod

#endif	/* MOD_MISC_H */
