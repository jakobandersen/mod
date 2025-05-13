#ifndef MOD_LIB_COMBIOPT_VALUE_HPP
#define MOD_LIB_COMBIOPT_VALUE_HPP

#include <mod/Error.hpp>

// TODO: remove <utility> when Boost 1.76 is not longer used
//       see https://github.com/boostorg/math/pull/670
#include <utility>
#include <boost/math/special_functions/relative_difference.hpp>
#include <boost/operators.hpp>

#include <limits>
#include <optional>
#include <variant>

namespace mod::lib::CombiOpt {

enum class Relation {
	Leq, Eq, Geq
};

std::ostream &operator<<(std::ostream &s, Relation r);

struct Int :
/* */ boost::operators<Int>,
/* */ boost::operators<Int, int>,
/* */ boost::operators<Int, unsigned int>,
/* */ boost::operators<Int, long int>,
/* */ boost::operators<Int, long long int> {
	using ImplType = std::int_fast32_t;
	using ModelType = std::int32_t;
public:
	// INF_POS should be at most the value of infinity that the solvers are using.
	// Gurobi seems to be the most restrictive.
	static constexpr ImplType INF_POS = 2'000'000'000;
	static constexpr ImplType MAX = INF_POS - 1;
	static constexpr ImplType INF_NEG = -INF_POS;
	static constexpr ImplType MIN = -MAX;
public:
	constexpr Int() = default;
	constexpr Int(double) = delete;
	constexpr Int(long double) = delete;

	constexpr Int(int v) : v(std::clamp<ModelType>(v, INF_NEG, INF_POS)) {
		static_assert(sizeof(int) <= sizeof(ImplType));
	}
	constexpr Int(long int v) : v(std::clamp<long long int>(v, INF_NEG, INF_POS)) {}
	constexpr Int(long long int v) : v(std::clamp<long long int>(v, INF_NEG, INF_POS)) {
		static_assert(sizeof(ImplType) <= sizeof(long long int));
	}

	constexpr Int(const Int &v) = default;
	constexpr Int &operator=(const Int &v) = default;

	constexpr ImplType getValue() const {
		return v;
	}

	constexpr friend bool operator==(Int a, Int b) {
		return a.v == b.v;
	}

	constexpr friend bool operator<(Int a, Int b) {
		return a.v < b.v;
	}

	constexpr Int operator-() const {
		return -v;
	}

	// TODO: use add_sat, sub_sat, mul_sat when C++26
	// Use the implementation of those in libstdc++
	constexpr Int &operator+=(Int other) {
		ImplType res = 0;
		if (!__builtin_add_overflow(v, other.v, &res)) {
			v = std::clamp(res, INF_NEG, INF_POS);
		} else if(v < 0)
			v = INF_NEG;
		else
			v = INF_POS;
		return *this;
	}

	constexpr Int &operator-=(Int other) {
		ImplType res = 0;
		if (!__builtin_sub_overflow(v, other.v, &res)) {
			v = std::clamp(res, INF_NEG, INF_POS);
		} else if(v < 0)
			v = INF_NEG;
		else
			v = INF_POS;
		return *this;
	}

	constexpr Int &operator*=(Int other) {
		ImplType res = 0;
		if (!__builtin_mul_overflow(v, other.v, &res)) {
			v = std::clamp(res, INF_NEG, INF_POS);
		} else if(v < 0)
			v = INF_NEG;
		else
			v = INF_POS;
		return *this;
	}

	friend std::ostream &operator<<(std::ostream &s, Int v);
private:
	ImplType v = 0;
};

struct Float : boost::operators<Float> {
	using ImplType = double;
public:
	static constexpr ImplType INF_POS = Int::INF_POS;
	static constexpr ImplType MAX = INF_POS - 1;
	static constexpr ImplType INF_NEG = -INF_POS;
	static constexpr ImplType MIN = -MAX;
public:
	constexpr Float() = default;
	constexpr Float(ImplType v) : v(v) {}
	explicit constexpr Float(int v) : v(v) {}
	explicit constexpr Float(long int v) : v(v) {}
	explicit constexpr Float(Int v) : v(v.getValue()) {}

	template<typename T>
	Float(T) = delete;

	constexpr ImplType getValue() const {
		return v;
	}

	constexpr friend bool operator==(Float a, Float b) {
		return a.v == b.v;
	}

	constexpr friend bool operator<(Float a, Float b) {
		return a.v < b.v;
	}

	bool isClose(Float b) const {
		return boost::math::relative_difference(getValue(), b.getValue()) < 1e-6;
	}

	constexpr Float operator-() const {
		return -v;
	}

	constexpr Float &operator+=(Float other) {
		v += other.v;
		return *this;
	}

	constexpr Float &operator-=(Float other) {
		v -= other.v;
		return *this;
	}

	constexpr Float &operator*=(Float other) {
		v *= other.v;
		return *this;
	}

	constexpr Float &operator/=(Float other) {
		v /= other.v;
		return *this;
	}

	friend std::ostream &operator<<(std::ostream &s, Float v);
private:
	ImplType v = 0.0;
};

struct Value {
	using ImplType = std::variant<Int, Float>;
public:
	Value() = default;
	Value(Int v) : v(v) {}
	Value(Float v) : v(v) {}

	const ImplType &getValue() const {
		return v;
	}

	std::optional<Int> getAsInt() const {
		if(auto *i = std::get_if<Int>(&v)) return *i;
		else return {};
	}

	std::optional<Float> getAsFloat() const {
		if(auto *i = std::get_if<Float>(&v)) return *i;
		else return {};
	}

	std::variant<int, double> getAsImpl() const {
		if(auto *i = std::get_if<Int>(&v)) return int(i->getValue());
		else if(auto *f = std::get_if<Float>(&v)) return f->getValue();
		else
			MOD_ABORT;
	}

	bool isZero() const {
		if(auto *i = std::get_if<Int>(&v)) return *i == 0;
		else if(auto *f = std::get_if<Float>(&v)) return f->getValue() == 0.0;
		else
			MOD_ABORT;
	}
public:
	friend bool operator==(Value a, Value b) {
		return a.v == b.v;
	}

	friend bool operator!=(Value a, Value b) {
		return a.v != b.v;
	}

	friend bool operator<(Value a, Value b) {
		return a.v < b.v;
	}

	friend bool operator>(Value a, Value b) {
		return a.v > b.v;
	}

	friend bool operator<=(Value a, Value b) {
		return a.v <= b.v;
	}

	friend bool operator>=(Value a, Value b) {
		return a.v >= b.v;
	}
public:
	friend Value operator-(Value a, Value b) {
		struct VisitorSub {
			Value operator()(Int a, Int b) const {
				return a - b;
			}

			Value operator()(Float a, Float b) const {
				return a - b;
			}

			Value operator()(Int, Float) const {
				MOD_ABORT;
			}

			Value operator()(Float, Int) const {
				MOD_ABORT;
			}
		};
		return std::visit(VisitorSub(), a.v, b.v);
	}
public:
	friend std::ostream &operator<<(std::ostream &s, Value v) {
		if(auto *i = std::get_if<Int>(&v.v)) return s << *i;
		else if(auto *f = std::get_if<Float>(&v.v)) return s << *f;
		else
			MOD_ABORT;
	}
private:
	std::variant<Int, Float> v;
};

} // namespace mod::lib::CombiOpt

#endif // MOD_LIB_COMBIOPT_INT_HPP
