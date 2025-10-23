#include "Config.hpp"

#include <mod/lib/ILP/ILP.hpp>

#include <ostream>
#include <tuple>

namespace mod {

std::ostream &operator<<(std::ostream &s, const LabelType lt) {
	switch(lt) {
	case LabelType::String:
		return s << "string";
	case LabelType::Term:
		return s << "term";
	}
	__builtin_unreachable();
}

std::ostream &operator<<(std::ostream &s, const LabelRelation lt) {
	switch(lt) {
	case LabelRelation::Isomorphism:
		return s << "isomorphism";
	case LabelRelation::Specialisation:
		return s << "specialisation";
	case LabelRelation::Unification:
		return s << "unification";
	}
	__builtin_unreachable();
}

bool operator==(LabelSettings a, LabelSettings b) {
	return std::tie(a.type, a.relation, a.withStereo, a.stereoRelation)
	       == std::tie(b.type, b.relation, b.withStereo, b.stereoRelation);
}

bool operator!=(LabelSettings a, LabelSettings b) {
	return !(a == b);
}

std::ostream &operator<<(std::ostream &s, const LabelSettings ls) {
	s << "LabelSettings{" << ls.type;
	switch(ls.type) {
	case LabelType::String:
		break;
	case LabelType::Term:
		s << "(" << ls.relation << ")";
		break;
	}
	if(ls.withStereo)
		s << ", stereo(" << ls.stereoRelation << ")";
	return s << "}";
}

bool LabelSettings::formCategory() const {
	return relation != LabelRelation::Unification && stereoRelation != LabelRelation::Unification;
}

std::ostream &operator<<(std::ostream &s, IsomorphismPolicy p) {
	switch(p) {
	case IsomorphismPolicy::Check:
		return s << "check";
	case IsomorphismPolicy::TrustMe:
		return s << "trustMe";
	}
	__builtin_unreachable();
}

std::ostream &operator<<(std::ostream &s, SmilesClassPolicy p) {
	switch(p) {
	case SmilesClassPolicy::NoneOnDuplicate:
		return s << "noneOnDuplicate";
	case SmilesClassPolicy::ThrowOnDuplicate:
		return s << "throwOnDuplicate";
	case SmilesClassPolicy::MapUnique:
		return s << "mapUnique";
	}
	__builtin_unreachable();
}

std::ostream &operator<<(std::ostream &s, Action a) {
	switch(a) {
	case Action::Error:
		return s << "error";
	case Action::Warn:
		return s << "warn";
	case Action::Ignore:
		return s << "ignore";
	}
	__builtin_unreachable();
}

std::vector<std::string> getAvailableILPSolvers() {
	return lib::ILP::Model::getAvailableSolvers();
}

Config &getConfig() {
	static Config config;
	return config;
}

} // namespace mod
