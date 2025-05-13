#ifndef MOD_LIB_COMBIOPT_VARIABLE_HPP
#define MOD_LIB_COMBIOPT_VARIABLE_HPP

#include <cassert>
#include <functional>
#include <type_traits>

namespace mod::lib::CombiOpt {

#define MOD_MAKE_VAR(Type)                                                      \
    struct Type ## Var {                                                        \
        Type ## Var() = default;                                                \
        explicit Type ## Var(std::size_t data) : data(data) { }                 \
        explicit operator bool() { return *this != Type ## Var(); }             \
        bool operator==(Type ## Var v) const { return data == v.data; }         \
        bool operator!=(Type ## Var v) const { return data != v.data; }         \
        bool operator<(Type ## Var v) const  { return data <  v.data; }         \
        std::size_t getData() const { return data; }                            \
    private:                                                                    \
        std::size_t data = -1;                                                  \
    }

MOD_MAKE_VAR(Bool);
MOD_MAKE_VAR(Int);
MOD_MAKE_VAR(Float);

#undef MOD_MAKE_VAR

enum class VarType {
	Bool, Int, Float
};

struct IntegralVar {
	IntegralVar(BoolVar v) : type(VarType::Bool), data(v.getData()) {}
	IntegralVar(IntVar v) : type(VarType::Int), data(v.getData()) {}
	explicit IntegralVar(VarType type, std::size_t data) : type(type), data(data) {
		assert(type == VarType::Bool || type == VarType::Int);
	}

	friend bool operator==(IntegralVar v1, IntegralVar v2) {
		return v1.type == v2.type && v1.data == v2.data;
	}

	friend bool operator!=(IntegralVar v1, IntegralVar v2) {
		return !(v1 == v2);
	}

	friend bool operator<(IntegralVar v1, IntegralVar v2) {
		if(v1.type != v2.type)
			return static_cast<std::underlying_type<VarType>::type> (v1.type) <
				   static_cast<std::underlying_type<VarType>::type> (v2.type);
		return v1.data < v2.data;
	}

	VarType getType() const {
		return type;
	}

	std::size_t getData() const {
		return data;
	}
private:
	VarType type;
	std::size_t data;
};

struct Var {
	Var(BoolVar v) : type(VarType::Bool), data(v.getData()) {}
	Var(IntVar v) : type(VarType::Int), data(v.getData()) {}
	Var(FloatVar v) : type(VarType::Float), data(v.getData()) {}
	Var(IntegralVar v) : type(v.getType()), data(v.getData()) {}
	explicit Var(VarType type, std::size_t data) : type(type), data(data) {}

	friend bool operator==(Var v1, Var v2) {
		return v1.type == v2.type && v1.data == v2.data;
	}

	friend bool operator!=(Var v1, Var v2) {
		return !(v1 == v2);
	}

	friend bool operator<(Var v1, Var v2) {
		if(v1.type != v2.type)
			return static_cast<std::underlying_type<VarType>::type> (v1.type) <
				   static_cast<std::underlying_type<VarType>::type> (v2.type);
		return v1.data < v2.data;
	}

	VarType getType() const {
		return type;
	}

	std::size_t getData() const {
		return data;
	}
private:
	VarType type;
	std::size_t data;
};

} // namespace mod::lib::CombiOpt
namespace std {

template<>
struct hash<mod::lib::CombiOpt::IntegralVar> {
	std::size_t operator()(mod::lib::CombiOpt::IntegralVar v) const {
		return hash<decltype(v.getData())>()(v.getData());
	}
};

template<>
struct hash<mod::lib::CombiOpt::FloatVar> {
	std::size_t operator()(mod::lib::CombiOpt::FloatVar v) const {
		return hash<decltype(v.getData())>()(v.getData());
	}
};

template<>
struct hash<mod::lib::CombiOpt::Var> {
	std::size_t operator()(mod::lib::CombiOpt::Var v) const {
		return hash<decltype(v.getData())>()(v.getData());
	}
};

} // namespace std

#endif // MOD_LIB_COMBIOPT_VARIABLE_HPP