#include "EdgeCategory.hpp"

#include <mod/Error.hpp>

#include <cassert>
#include <ostream>

namespace mod::lib::Stereo {

bool carriesPlane(EdgeCategory cat) {
	switch(cat) {
	case EdgeCategory::Any:
	case EdgeCategory::Undefined:
	case EdgeCategory::Single: return false;
	case EdgeCategory::Double: return true;
	case EdgeCategory::Triple: return false;
	case EdgeCategory::Aromatic: return true;
	}
	MOD_ABORT;
}

std::ostream &operator<<(std::ostream &s, EdgeCategory c) {
	switch(c) {
	case EdgeCategory::Any: return s << "X";
	case EdgeCategory::Undefined: return s << "U";
	case EdgeCategory::Single: return s << "S";
	case EdgeCategory::Double: return s << "D";
	case EdgeCategory::Triple: return s << "T";
	case EdgeCategory::Aromatic: return s << "A";
	}
	MOD_ABORT; // make GCC happy
}

EdgeCategory bondTypeToEdgeCategory(BondType bt) {
	switch(bt) {
	case BondType::Invalid: return EdgeCategory::Undefined;
	case BondType::Single: return EdgeCategory::Single;
	case BondType::Double: return EdgeCategory::Double;
	case BondType::Triple: return EdgeCategory::Triple;
	case BondType::Aromatic: return EdgeCategory::Aromatic;
	}
	__builtin_unreachable();
}

// EdgeCategorySubset
//------------------------------------------------------------------------------

EdgeCategorySubset EdgeCategorySubset::all() && {
	bits.set();
	return *this;
}

EdgeCategorySubset EdgeCategorySubset::operator+(EdgeCategory cat) && {
	bits.set(static_cast<std::underlying_type<EdgeCategory>::type> (cat));
	return *this;
}

EdgeCategorySubset EdgeCategorySubset::operator-(EdgeCategory cat) && {
	bits.reset(static_cast<std::underlying_type<EdgeCategory>::type> (cat));
	return *this;
}

EdgeCategorySubset operator&(EdgeCategorySubset a, EdgeCategorySubset b) {
	EdgeCategorySubset res;
	res.bits = a.bits & b.bits;
	return res;
}

bool operator==(EdgeCategorySubset a, EdgeCategorySubset b) {
	return a.bits == b.bits;
}

bool EdgeCategorySubset::operator()(EdgeCategory cat) const {
	return bits.test(static_cast<std::underlying_type<EdgeCategory>::type> (cat));
}

unsigned char EdgeCategorySubset::count() const {
	return bits.count();
}

EdgeCategory EdgeCategorySubset::selectFirst() const {
	assert(count() == 1);
	for(std::size_t i = 0; i < bits.size(); i++) {
		if(bits.test(i))
			return static_cast<EdgeCategory> (i);
	}
	assert(false);
	std::abort();
}

std::ostream &operator<<(std::ostream &s, const EdgeCategorySubset &ec) {
	s << "{";
	for(std::size_t i = 0; i < ec.bits.size(); i++) {
		if(ec.bits.test(i))
			s << " " << static_cast<EdgeCategory>(i);
	}
	return s << " }";
}

// EdgeCategoryCount
//------------------------------------------------------------------------------

EdgeCategoryCount::EdgeCategoryCount() {
	count.fill(0);
}

unsigned char EdgeCategoryCount::sum() const {
	return std::accumulate(begin(), end(), 0);
}

unsigned char &EdgeCategoryCount::operator[](EdgeCategory cat) {
	std::size_t index = static_cast<std::underlying_type<EdgeCategory>::type> (cat);
	assert(index < count.size());
	return count[index];
}

const unsigned char &EdgeCategoryCount::operator[](EdgeCategory cat) const {
	std::size_t index = static_cast<std::underlying_type<EdgeCategory>::type> (cat);
	assert(index < count.size());
	return count[index];
}

std::ostream &operator<<(std::ostream &s, const EdgeCategoryCount &c) {
	bool hasPrinted = false;
	for(std::size_t i = 0; i < c.count.size(); i++) {
		if(static_cast<int> (c.count[i]) == 0) continue;
		if(hasPrinted) s << ", ";
		hasPrinted = true;
		s << static_cast<EdgeCategory> (i) << " = " << static_cast<int> (c.count[i]);
	}
	return s;
}

EdgeCategoryCount::Storage::const_iterator EdgeCategoryCount::begin() const {
	return count.begin();
}

EdgeCategoryCount::Storage::const_iterator EdgeCategoryCount::end() const {
	return count.end();
}

bool operator==(const EdgeCategoryCount &lhs, const EdgeCategoryCount &rhs) {
	return lhs.count == rhs.count;
}

bool operator!=(const EdgeCategoryCount &lhs, const EdgeCategoryCount &rhs) {
	return lhs.count != rhs.count;
}

bool componentWiseLEQ(const EdgeCategoryCount &lhs, const EdgeCategoryCount &rhs) {
	for(auto i = 0; i < EdgeCategorySize; ++i) {
		auto cat = static_cast<EdgeCategory> (i);
		if(lhs[cat] > rhs[cat]) return false;
	}
	return true;
}

} // namespace mod::lib::Stereo