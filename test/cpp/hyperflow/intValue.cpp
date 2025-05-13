#include <mod/lib/CombiOpt/Value.hpp>

#undef NDEBUG

#include <cassert>

using Int = mod::lib::CombiOpt::Int;

int main() {
	Int INF_POS = Int::INF_POS;
	Int INF_NEG = Int::INF_NEG;

	assert(Int(std::numeric_limits<int>::max()) == INF_POS);
	assert(Int(std::numeric_limits<int>::min()) == INF_NEG);
	assert(Int(std::numeric_limits<long int>::max()) == INF_POS);
	assert(Int(std::numeric_limits<long int>::min()) == INF_NEG);
	assert(Int(std::numeric_limits<long long int>::max()) == INF_POS);
	assert(Int(std::numeric_limits<long long int>::min()) == INF_NEG);

	assert(INF_POS + INF_POS == INF_POS);
	assert(INF_POS + 1 == INF_POS);
	assert(INF_POS + 2 == INF_POS);
	assert(INF_POS + 3 == INF_POS);
	assert(INF_POS + 10 == INF_POS);
	assert(INF_NEG + -1 == INF_NEG);
	assert(INF_NEG + -2 == INF_NEG);
	assert(INF_NEG + -3 == INF_NEG);
	assert(INF_NEG + -10 == INF_NEG);

	assert(INF_NEG + INF_NEG == INF_NEG);
	assert(INF_NEG - 1 == INF_NEG);
	assert(INF_NEG - 2 == INF_NEG);
	assert(INF_NEG - 3 == INF_NEG);
	assert(INF_NEG - 10 == INF_NEG);
	assert(INF_POS - -1 == INF_POS);
	assert(INF_POS - -2 == INF_POS);
	assert(INF_POS - -3 == INF_POS);
	assert(INF_POS - -10 == INF_POS);

	assert(INF_POS * 2 == INF_POS);
	assert(INF_POS * 4 == INF_POS);
	assert(INF_POS * 8 == INF_POS);

	assert(INF_NEG * 2 == INF_NEG);
	assert(INF_NEG * 4 == INF_NEG);
	assert(INF_NEG * 8 == INF_NEG);
}