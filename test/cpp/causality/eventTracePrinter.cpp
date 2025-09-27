#include <mod/Error.hpp>
#include <mod/causality/EventTracePrinter.hpp>

#undef NDEBUG

#include <cassert>

using namespace mod;


template<typename F, typename ...Ts>
void test(F f, const std::string &msg, const Ts &...ts) {
	causality::EventTracePrinter p;
	try {
		(p.*f)({}, ts...);
		assert(false);
	} catch(const LogicError &e) {
		assert(e.what() == msg);
	}
}

template<typename F, typename ...Ts>
void testPush(F f, const Ts &...ts) {
	test(f, "Can not push empty callback.", ts...);
}

template<typename F, typename ...Ts>
void testSet(F f, const Ts &...ts) {
	test(f, "Can not set empty callback.", ts...);
}

int main() {
	testPush(&causality::EventTracePrinter::pushOptions);
	testPush(&causality::EventTracePrinter::pushVertexVisible);
	testPush(&causality::EventTracePrinter::pushVertexOptions);
	testSet(&causality::EventTracePrinter::setPreContent);
	testSet(&causality::EventTracePrinter::setPostContent);
}