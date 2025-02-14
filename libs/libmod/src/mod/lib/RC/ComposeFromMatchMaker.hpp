#ifndef MOD_LIB_RC_COMPOSE_FROM_MATCH_MAKER_HPP
#define MOD_LIB_RC_COMPOSE_FROM_MATCH_MAKER_HPP

#include <mod/Config.hpp>

#include <functional>
#include <memory>

namespace mod::lib::IO {
struct Logger;
} // namespace mod::lib::IO
namespace mod::lib::rule {
struct Rule;
} // namespace mod::lib::rule
namespace mod::lib::RC {
struct Common;
struct Parallel;
struct Sub;
struct Super;
struct ResultMaps;

// For explicitly instantiating the generic function separately.
#define MOD_RC_COMPOSE_FROM_MATCH_MAKER(MM)                                                                            \
   void composeFromMatchMaker(const lib::rule::Rule &rFirst, const lib::rule::Rule &rSecond,                           \
         const MM &mm, std::function<bool(std::unique_ptr<lib::rule::Rule>, const ResultMaps&)> rr,                    \
         LabelSettings labelSettings);
MOD_RC_COMPOSE_FROM_MATCH_MAKER(Common)
MOD_RC_COMPOSE_FROM_MATCH_MAKER(Parallel)
MOD_RC_COMPOSE_FROM_MATCH_MAKER(Sub)
MOD_RC_COMPOSE_FROM_MATCH_MAKER(Super)
#undef MOD_RC_COMPOSE_FROM_MATCH_MAKER

#define MOD_RC_COMPOSE_FROM_MATCH_MAKER_IMPL(MM)                                                                       \
	void composeFromMatchMaker(const lib::rule::Rule &rFirst, const lib::rule::Rule &rSecond,                          \
			const MM &mm, std::function<bool(std::unique_ptr<lib::rule::Rule>, const ResultMaps&)> rr,                 \
			LabelSettings labelSettings) {                                                                             \
		composeFromMatchMakerImpl(rFirst, rSecond, mm, rr, labelSettings);                                             \
	}

} // namespace mod::lib::RC

#endif // MOD_LIB_RC_COMPOSE_FROM_MATCH_MAKER_HPP