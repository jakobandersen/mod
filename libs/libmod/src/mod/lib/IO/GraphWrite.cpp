#include "GraphWrite.hpp"

namespace mod::lib::IO::Graph::Write {

EdgeFake3DType invertEdgeFake3DType(EdgeFake3DType t) {
	switch(t) {
	case EdgeFake3DType::None:
		return t;
	case EdgeFake3DType::WedgeSL:
		return EdgeFake3DType::WedgeLS;
	case EdgeFake3DType::WedgeLS:
		return EdgeFake3DType::WedgeSL;
	case EdgeFake3DType::HashSL:
		return EdgeFake3DType::HashLS;
	case EdgeFake3DType::HashLS:
		return EdgeFake3DType::HashSL;
	case EdgeFake3DType::Unkown:
		return EdgeFake3DType::Unkown;
	}
	__builtin_unreachable();
}

} // namespace mod::lib::IO::Graph::Write