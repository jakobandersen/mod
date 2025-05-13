#ifndef MOD_LIB_IO_GRAPHWRITE_HPP
#define MOD_LIB_IO_GRAPHWRITE_HPP

#include <string>

namespace mod::lib::IO::Graph::Write {

struct Options {
	Options &Non() {
		return EdgesAsBonds(false).CollapseHydrogens(false).RaiseIsotopes(false).RaiseCharges(false)
				.SimpleCarbons(false).Thick(false).WithColour(false).WithIndex(false);
	}

	Options &All() {
		return EdgesAsBonds(true).CollapseHydrogens(true).RaiseIsotopes(true).RaiseCharges(true)
				.SimpleCarbons(true).Thick(true).WithColour(true).WithIndex(true);
	}

	Options &EdgesAsBonds(bool v) {
		edgesAsBonds = v;
		return *this;
	}

	Options &CollapseHydrogens(bool v) {
		collapseHydrogens = v;
		return *this;
	}

	Options &RaiseIsotopes(bool v) {
		raiseIsotopes = v;
		return *this;
	}

	Options &RaiseCharges(bool v) {
		raiseCharges = v;
		return *this;
	}

	Options &SimpleCarbons(bool v) {
		simpleCarbons = v;
		return *this;
	}

	Options &Thick(bool v) {
		thick = v;
		return *this;
	}

	Options &WithColour(bool v) {
		withColour = v;
		return *this;
	}

	Options &WithIndex(bool v) {
		withIndex = v;
		return *this;
	}

	Options &WithTexttt(bool v) {
		withTexttt = v;
		return *this;
	}

	Options &WithRawStereo(bool v) {
		withRawStereo = v;
		return *this;
	}

	Options &WithPrettyStereo(bool v) {
		withPrettyStereo = v;
		return *this;
	}

	Options &Rotation(int degrees) {
		rotation = degrees;
		return *this;
	}

	Options &Mirror(bool v) {
		mirror = v;
		return *this;
	}

	Options &WithGraphvizCoords(bool v) {
		withGraphvizCoords = v;
		return *this;
	}

	std::string getStringEncoding() const {
		auto toChar = [](bool b) {
			return b ? '1' : '0';
		};
		std::string res;
		res += toChar(edgesAsBonds);
		res += toChar(collapseHydrogens);
		char raise = 0;
		if(raiseCharges) raise += 1;
		if(raiseIsotopes) raise += 2;
		res += '0' + raise;
		res += toChar(simpleCarbons);
		res += toChar(thick);
		res += toChar(withColour);
		res += toChar(withIndex);
		res += toChar(withTexttt);
		char stereo = 0;
		if(withRawStereo) stereo += 1;
		if(withPrettyStereo) stereo += 2;
		if(stereo != 0) res += '0' + stereo;
		if(rotation != 0 || mirror) {
			res += "_";
			res += std::to_string(rotation);
			if(mirror) {
				res += "_";
				res += toChar(mirror);
			}
		}
		return res;
	}

	friend bool operator==(const Options &a, const Options &b) {
		return a.getStringEncoding() == b.getStringEncoding()
		       && a.graphvizPrefix == b.graphvizPrefix;
	}

	friend bool operator!=(const Options &a, const Options &b) {
		return !(a == b);
	}
public:
	bool edgesAsBonds = false;
	bool collapseHydrogens = false;
	bool raiseCharges = false;
	bool raiseIsotopes = false;
	bool simpleCarbons = false;
	bool thick = false;
	bool withColour = false;
	bool withIndex = false;
	bool withTexttt = false;
	bool withRawStereo = false;
	bool withPrettyStereo = false;
	int rotation = 0;
	bool mirror = false;
	bool withGraphvizCoords = false;
public: // not participating in string encoding
	std::string graphvizPrefix;
};

enum class EdgeFake3DType {
	None, WedgeSL, WedgeLS, HashSL, HashLS, Unkown
};

EdgeFake3DType invertEdgeFake3DType(EdgeFake3DType t);

} // namespace mod::lib::IO::Graph::Write

#endif // MOD_LIB_IO_GRAPHWRITE_HPP
