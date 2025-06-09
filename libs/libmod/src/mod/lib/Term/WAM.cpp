#include "WAM.hpp"

#include <mod/lib/StringStore.hpp>

namespace mod::lib::Term {

const StringStore &getStrings() {
	static StringStore strings;
	return strings;
}

} // nnamespace mod::lib::Term