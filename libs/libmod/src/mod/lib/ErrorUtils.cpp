#include "ErrorUtils.hpp"

#include <mod/Config.hpp>
#include <mod/Error.hpp>

namespace mod::lib {

void errorIfNotCategory(LabelSettings ls) {
	if(ls.relation == LabelRelation::Unification)
		throw LogicError("The label settings does not form a category. The relation is Unification, but must be Isomorphism or Specialisation.");
	if(ls.stereoRelation == LabelRelation::Unification)
		throw LogicError("The label settings does not form a category. The stereoRelation is Unification, but must be Isomorphism or Specialisation.");
}

} // namespace mod::lib