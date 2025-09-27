#ifndef MOD_MISC_HPP
#define MOD_MISC_HPP

#include <mod/BuildConfig.hpp>
#include <mod/graph/ForwardDecl.hpp>
#include <mod/hyperflow/ForwardDecl.hpp>

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
// rst:		:returns: a uniformly random real number in the half-open interval :math:`[0, 1)`.
MOD_DECL double rngUniformReal();

MOD_DECL std::string strFromDump(const std::string &file);
MOD_DECL void showDump(const std::string &file);

MOD_DECL void printGeometryGraph();


// internal function for testing purposes
MOD_DECL void compareModels_only_for_testing(std::shared_ptr<hyperflow::Model> a, std::shared_ptr<hyperflow::Model> b);

} // namespace mod

#endif // MOD_MISC_HPP