#ifndef MOD_LIB_ERRORUTILS_HPP
#define MOD_LIB_ERRORUTILS_HPP

namespace mod {
struct LabelSettings;
} // namespace mod
namespace mod::lib {

void errorIfNotCategory(LabelSettings ls);

} // namespace mod::lib

#endif // MOD_LIB_ERRORUTILS_HPP