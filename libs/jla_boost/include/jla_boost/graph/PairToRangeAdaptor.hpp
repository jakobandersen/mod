#ifndef JLA_BOOST_GRAPH_PAIRTORANGEADAPTOR_H
#define JLA_BOOST_GRAPH_PAIRTORANGEADAPTOR_H

#include <utility>

namespace jla_boost {

// adapted from http://stackoverflow.com/questions/6167598/why-was-pair-range-access-removed-from-c11
// (e.g., for used with Boost.Graph, which returns pairs of iterators)

template<typename Iter>
struct range : std::pair<Iter, Iter> {
	using iterator = Iter;
	using const_iterator = iterator;

	inline range(const std::pair<Iter, Iter> &x) : std::pair<Iter, Iter>(x) { }

	inline Iter begin() const {
		return this->first;
	}

	inline Iter end() const {
		return this->second;
	}

	inline decltype(auto) operator[](int i) const {
		return begin()[i];
	}
};

template<typename Iter>
inline range<Iter> asRange(const std::pair<Iter, Iter> &x) {
	return range<Iter>(x);
}

template<typename Iter>
inline range<Iter> asRange(Iter first, Iter last) {
	return asRange(std::make_pair(first, last));
}

} // namespace jla_boost
namespace mod { // HAX!
using jla_boost::asRange;
} // namespace mod

#endif /* JLA_BOOST_GRAPH_PAIRTORANGEADAPTOR_H */
