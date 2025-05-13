#include "Value.hpp"

#include <ostream>

namespace mod {
namespace lib {
namespace CombiOpt {

std::ostream &operator<<(std::ostream &s, Relation r) {
	switch(r) {
	case Relation::Leq:
		return s << "<=";
	case Relation::Eq:
		return s << "==";
	case Relation::Geq:
		return s << ">=";
	}
	return s;
}

std::ostream &operator<<(std::ostream &s, Int v) {
	return s << v.v;
}

std::ostream &operator<<(std::ostream &s, Float v) {
	return s << v.v;
}

} // namespace CombiOpt
} // lib
} // mod