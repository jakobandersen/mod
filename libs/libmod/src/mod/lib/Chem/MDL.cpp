#include "MDL.hpp"

//#define BOOST_SPIRIT_X3_DEBUG

#include <mod/Config.hpp>
#include <mod/lib/Algorithm/ConnectedComponents.hpp>
#include <mod/lib/Chem/MoleculeUtil.hpp>
#include <mod/lib/Graph/Properties/String.hpp>
#include <mod/lib/IO/Parsing.hpp>

#include <boost/spirit/home/x3/char/char.hpp>
#include <boost/spirit/home/x3/char/char_class.hpp>
#include <boost/spirit/home/x3/directive/lexeme.hpp>
#include <boost/spirit/home/x3/directive/raw.hpp>
#include <boost/spirit/home/x3/operator/alternative.hpp>
#include <boost/spirit/home/x3/operator/difference.hpp>
#include <boost/spirit/home/x3/operator/kleene.hpp>
#include <boost/spirit/home/x3/operator/not_predicate.hpp>
#include <boost/spirit/home/x3/operator/plus.hpp>
#include <boost/spirit/home/x3/string/literal_string.hpp>

#include <optional>
#include <vector>

template<typename T = void>
using Result = mod::lib::IO::Result<T>;

#ifdef BOOST_SPIRIT_X3_DEBUG

namespace boost::spirit::x3::traits {

template<typename Out, typename Iter>
struct print_attribute_debug<Out, iterator_range<Iter>, void> {
	static void call(Out &out, const iterator_range<Iter> &val) {
		out << "boost::iterator_range<>";
	}
};

} // namespace boost::spirit::x3::traits

#endif

namespace mod::lib::Chem {
namespace {
// defined in the bottom
// Returns: line, hasLine
std::pair<std::string_view, bool> getLine(std::string_view &src);
unsigned int MDLValence(unsigned int elem, int q, unsigned int val);

Result<> handleAction(lib::IO::Warnings &warnings, const Action action, std::string msg) {
	switch(action) {
	case Action::Ignore:
		warnings.add(std::move(msg), false);
		return {};
	case Action::Warn:
		warnings.add(std::move(msg), true);
		return {};
	case Action::Error:
		return Result<>::Error(std::move(msg));
	}
	__builtin_unreachable();
}


struct Atom {
	std::string symbol;
	Isotope iso;
	Charge chg;
	bool radical = false;
	int valence = -1; // optional in V3000
public: // V3000 data
	int id;
	int aamap;
public: // derived data
	AtomId aId;
};

struct Bond {
	int src, tar;
	std::string type;
public: // V3000 data
	int id;
public: // derived data
	BondType bondType;
};

struct MOL {
	std::vector<Atom> atoms;
	std::vector<Bond> bonds;
	int lineFirst, lineLast;
public:
	Result<std::vector<lib::graph::Read::Data>> convert(lib::IO::Warnings &warnings, const MDLOptions &options) &&{
		auto res = convertImpl(warnings, options);
		if(!res)
			return Result<>::Error(res.extractError() + "\nCould not convert MOL at line "
			                       + std::to_string(lineFirst) + " to " + std::to_string(lineLast) + ".");
		return res;
	}
private:
	Result<std::vector<lib::graph::Read::Data>> convertImpl(lib::IO::Warnings &warnings, const MDLOptions &options) {
		lib::ConnectedComponents components(atoms.size());
		for(const Bond &b : bonds)
			components.join(b.src - 1, b.tar - 1);
		const auto numComponents = components.finalize();
		if(numComponents == 0) return Result<>::Error("Molecule has no atoms.");

		std::vector<lib::graph::Read::Data> datas;
		datas.resize(numComponents);
		const auto onError = [&datas](std::string msg) -> Result<> {
			for(auto &d : datas) d.reset();
			return Result<>::Error(std::move(msg));
		};
		for(int i = 0; i != numComponents; ++i) {
			datas[i].g = std::make_unique<lib::graph::GraphType>();
			datas[i].pString = std::make_unique<lib::graph::PropString>(*datas[i].g);
		}

		for(int i = 0; i != atoms.size(); ++i) {
			const Atom &a = atoms[i];
			const auto comp = components[i];
			const auto v = add_vertex(*datas[comp].g);
			datas[comp].externalToInternalIds[i + 1] = get(boost::vertex_index_t(), *datas[comp].g, v);
			if(a.symbol != "*") {
				std::string s;
				if(a.iso != Isotope())
					s += std::to_string(a.iso);
				s += a.symbol;
				s += chargeSuffix(a.chg);
				if(a.radical) s += '.';
				datas[comp].pString->addVertex(v, std::move(s));
			} else {
				if(a.iso != Isotope()) {
					auto res = handleAction(warnings, options.onPatternIsotope,
					                        "Pattern atom with ID " + std::to_string(i + 1)
					                        + " has isotope " + boost::lexical_cast<std::string>(a.iso) + ".");
					if(!res) return onError(res.extractError());
				}
				if(a.chg != Charge()) {
					auto res = handleAction(warnings, options.onPatternCharge,
					                        "Pattern atom with ID " + std::to_string(i + 1)
					                        + " has charge " + boost::lexical_cast<std::string>(a.chg) + ".");
					if(!res) return onError(res.extractError());
				}
				if(a.radical) {
					auto res = handleAction(warnings, options.onPatternRadical,
					                        "Pattern atom with ID " + std::to_string(i + 1) + " has radical.");
					if(!res) return onError(res.extractError());
				}
				datas[comp].pString->addVertex(v, "*");
			}
		}

		std::vector<int> valences(atoms.size(), 0);
		std::vector<int> numAromatic(atoms.size(), 0);
		std::vector<bool> hasAnyIncident(atoms.size(), false);
		for(const Bond &b : bonds) {
			assert(components[b.src - 1] == components[b.tar - 1]);
			const auto comp = components[b.src - 1];
			auto &g = *datas[comp].g;
			const auto vSrc = vertices(g).first[datas[comp].externalToInternalIds.find(b.src)->second];
			const auto vTar = vertices(g).first[datas[comp].externalToInternalIds.find(b.tar)->second];
			if(const auto eQuery = edge(vSrc, vTar, g); eQuery.second)
				return onError("Parallel edges in MOL file between atom " + std::to_string(b.src)
				               + " and " + std::to_string(b.tar) + ".");
			const auto e = add_edge(vSrc, vTar, g).first;
			datas[comp].pString->addEdge(e, b.type);
			if(b.bondType == BondType::Invalid) {
				hasAnyIncident[b.src - 1] = true;
				hasAnyIncident[b.tar - 1] = true;
			} else {
				const int order = [&]() {
					switch(b.bondType) {
					case BondType::Single:
						return 1;
					case BondType::Double:
						return 2;
					case BondType::Triple:
						return 3;
					case BondType::Aromatic:
						return 1;
					case BondType::Invalid:
						__builtin_unreachable();
					}
					__builtin_unreachable();
				}();
				valences[b.src - 1] += order;
				valences[b.tar - 1] += order;
				if(b.bondType == BondType::Aromatic) {
					++numAromatic[b.src - 1];
					++numAromatic[b.tar - 1];
				}
			}
		}
		// Handle valence
		for(int i = 0; i != atoms.size(); ++i) {
			const Atom &a = atoms[i];
			int valence;
			if(a.valence == -1) {
				if(!options.addHydrogens) continue;
				if(a.aId == AtomIds::Invalid) {
					auto res = handleAction(warnings, options.onImplicitValenceOnAbstract,
					                        "Implicit valence for atom " + std::to_string(i + 1)
					                        + " can not be handled. Atom has no concrete atom symbol."
					                        +
					                        "\nSet onImplicitValenceOnAbstract=Ignore or Warn to not try to add hydrogens to such abstract atoms."
					                        + " Or set addHydrogens=False to disable addition of hydrogens on all atoms.");
					if(!res) return onError(res.extractError());
				}
				valence = MDLValence(a.aId, a.chg, valences[i]);
				if(a.radical) --valence;
			} else {
				valence = a.valence;
			}
			if(valence > valences[i]) {
				const auto comp = components[i];
				auto &g = *datas[comp].g;
				const auto v = vertices(g).first[datas[comp].externalToInternalIds.find(i + 1)->second];
				for(int j = valences[i]; j != valence; ++j) {
					const auto vH = add_vertex(g);
					datas[comp].pString->addVertex(vH, "H");
					const auto eH = add_edge(v, vH, g).first;
					datas[comp].pString->addEdge(eH, "-");
				}
			}
		}
		return std::move(datas); // TODO: remove std::move when C++20/P1825R0 is available
	}
};

template<typename Iter, typename Desc>
Result<int> getUInt(Iter first, const Iter last, Desc &&desc) {
	const auto origFirst = first;
	for(; first != last && *first == ' '; ++first);
	if(first == last)
		return Result<>::Error(std::string("Expected ") + desc + ". Got '" + std::string(origFirst, last) + "'.");
	int res = 0;
	for(; first != last; ++first) {
		const char c = *first;
		if(c < '0' || c > '9')
			return Result<>::Error(std::string("Expected ") + desc + ". Got '" + std::string(origFirst, last) + "'.");
		res *= 10;
		res += c - '0';
	}
	return res;
}

template<typename Iter, typename Desc>
Result<int> getInt(Iter first, const Iter last, Desc &&desc) {
	const auto origFirst = first;
	for(; first != last && *first == ' '; ++first);
	if(first == last)
		return Result<>::Error(std::string("Expected ") + desc + ". Got '" + std::string(origFirst, last) + "'.");
	bool neg = false;
	int res = 0;
	if(*first == '-') {
		neg = true;
		++first;
		if(first == last)
			return Result<>::Error(std::string("Expected ") + desc + ". Got '" + std::string(origFirst, last) + "'.");
	}
	for(; first != last; ++first) {
		const char c = *first;
		if(c < '0' || c > '9')
			return Result<>::Error(std::string("Expected ") + desc + ". Got '" + std::string(origFirst, last) + "'.");
		res *= 10;
		res += c - '0';
	}
	if(neg) return -res;
	else return res;
}

Result<> handleRAD(lib::IO::Warnings &warnings, Atom &a, const int radical, const MDLOptions &options) {
	switch(radical) {
	case 0:
		a.radical = false;
		return {};
	case 1:
		break;
	case 2:
		a.radical = true;
		return {};
	case 3:
	case 4:
	case 5:
	case 6:
		break;
	default:
		return Result<>::Error("Radical out of range. Got " + std::to_string(radical) + ", must be in 0 to 6.");
	}
	const Action act = [radical, &options]() {
		switch(radical) {
		case 1:
			return options.onRAD1;
		case 3:
			return options.onRAD3;
		case 4:
			return options.onRAD4;
		case 5:
			return options.onRAD5;
		case 6:
			return options.onRAD6;
		}
		__builtin_unreachable();
	}();
	return handleAction(warnings, act,
	                    "Radical value " + std::to_string(radical) + " is currently not supported in atom.");
}

#define FETCH_LINE(var, desc)                                       \
   std::string_view var;                                            \
   {                                                                \
      bool hasLine;                                                 \
      if(std::tie(var, hasLine) = getLine(src); !hasLine)           \
         return Result<>::Error("Expected " + std::string(desc) + ". Got nothing."); \
   }

Result<MOL>
parseMOLV2000(lib::IO::Warnings &warnings, std::string_view &src, const MDLOptions &options,
              const std::string_view counts, int &lineCount) {
	// Counts line:
	// aaabbblllfffcccsssxxxrrrpppiiimmmvvvvvv
	// - a: #atoms
	// - b: #bonds
	// - l: #atom lists [query]
	// - f: obsolete
	// - c: chiral flag
	// - s: #stext entries
	// - x, r, p, i: obsolete
	// - m: #properties lines, but is now just always 999

	auto numAtoms = getUInt(counts.begin(), counts.begin() + 3, "V2000 atom count");
	if(!numAtoms) return std::move(numAtoms);
	auto numBonds = getUInt(counts.begin() + 3, counts.begin() + 6, "V2000 bond count");
	if(!numBonds) return std::move(numBonds);
	++lineCount;

	MOL mol;
	mol.atoms.reserve(*numAtoms);
	mol.bonds.reserve(*numBonds);

	// Atom block:
	for(int i = 0; i != *numAtoms; ++i) {
		// xxxxx.xxxxyyyyy.yyyyzzzzz.zzzz aaaddcccssshhhbbbvvvHHHrrriiimmmnnneee
		// x, y, z: coordinates
		// a  31:34  - entry in periodic table
		//           - L for atom list
		//           - A, Q, * for unspecified atom
		//           - LP for lone pair
		// d  34:36  mass difference, i.e., isotope stuff. Can be overwritten by an ISO line.
		// c  36:39  charge. Can be overwritten by CHG and RAD lines.
		//           - 0: neutral
		//           - 1,2,3: +3,+2,+1
		//           - 4: "doublet radical"
		//           - 5,6,7: -1,-2,-3
		// s  39:42  atom stereo parity. Ignored.
		// h  42:45  hydrogen count. Only query.
		// b  45:48  stereo care box. Only query.
		// v  48:51  valence, the total valence. Missing bonds implies implicit hydrogens.
		//           - 0: nothing special
		//           - 1-14: that valence
		//           - 15: 0
		// H  51:54  H0 designator
		// r  54:57  not used
		// i  57:60  not used
		// m  60:63  atom-atom map: 1 to numAtoms
		// n  63:66  inversion/retention flag
		// e  66:69  exact change flag
		FETCH_LINE(line, "V2000 atom block line")
		constexpr int Len = 69;
		if(line.size() != Len)
			return Result<>::Error("Wrong length of V2000 atom block line. Expected length "
			                       + std::to_string(Len) + ", got " + std::to_string(line.size())
			                       + ".\nLine: >>>" + std::string(line) + "<<<");
		Atom atom;
		// =========================================================================
		auto fSymbol = line.begin() + 31;
		auto lSymbol = line.begin() + 34;
		for(; fSymbol != lSymbol && *fSymbol == ' '; ++fSymbol);
		for(; fSymbol != lSymbol && *(lSymbol - 1) == ' '; --lSymbol);
		atom.symbol = std::string(fSymbol, lSymbol);
		if(atom.symbol == "LP") {
			// consider it as abstract
		} else if(atom.symbol == "A" || atom.symbol == "Q" || atom.symbol == "*") {
			atom.symbol = "*";
		} else if(atom.symbol == "L") {
			// consider it as abstract
		} else {
			const auto aId = atomIdFromSymbol(atom.symbol);
			if(aId == AtomIds::Invalid && !options.allowAbstract)
				return Result<>::Error("Unknown atom symbol. Got '" + atom.symbol + "'."
				                       + " Set allowAbstract=True to allow it.");
			atom.aId = aId;
		}
		// =========================================================================
		auto massDiff = getInt(line.begin() + 34, line.begin() + 36, "V2000 mass difference in atom block line");
		if(!massDiff) return std::move(massDiff);
		if(*massDiff != 0) {
			if(atom.aId != AtomIds::Invalid) {
				atom.iso = Isotope(atom.aId + *massDiff);
			} else {
				auto res = handleAction(warnings, options.onV2000AbstractISO,
				                        "Can not compute isotope from mass difference and non-concrete atom in V2000 atom block.");
				if(!res) return res;
			}
		}
		// =========================================================================
		auto charge = getUInt(line.begin() + 36, line.begin() + 39, "V2000 charge in atom block line");
		if(!charge) return std::move(charge);
		switch(*charge) {
		case 0:
			break;
		case 1:
		case 2:
		case 3:
		case 5:
		case 6:
		case 7:
			atom.chg = Charge(4 - *charge);
			break;
		case 4: {
			auto res = handleAction(warnings, options.onV2000Charge4,
			                        "Charge '4' (doublet radical) not yet supported in V2000 atom block line.");
			if(!res) return res;
			break;
		}
		default:
			return Result<>::Error("Unknown charge in V2000 atom block line. Got " + std::to_string(*charge) + ".");
		}
		// =========================================================================
		auto valence = getUInt(line.begin() + 48, line.begin() + 51, "V2000 valence in atom block line");
		if(!valence) return std::move(valence);
		if(*valence == 0) atom.valence = -1;
		else if(*valence >= 1 && *valence <= 14) atom.valence = *valence;
		else if(*valence == 15) atom.valence = 0;
		else return Result<>::Error("Unknown valence in V2000 atom block line. Got " + std::to_string(*valence) + ".");
		// =========================================================================
		auto aamapRes = getUInt(line.begin() + 60, line.begin() + 63, "V2000 atom-atom map in atom block line");
		if(!aamapRes) return std::move(aamapRes);
		atom.aamap = *aamapRes;
		// =========================================================================
		mol.atoms.push_back(std::move(atom));
		++lineCount;
	} // for each atom line
	// Bond block:
	for(int i = 0; i != *numBonds; ++i) {
		// 111222tttsssxxxrrrccc
		FETCH_LINE(line, "V2000 bond block line")
		constexpr int Len = 7 * 3;
		if(line.size() != Len)
			return Result<>::Error("Wrong length of V2000 atom block line. Expected length "
			                       + std::to_string(Len) + ", got " + std::to_string(line.size())
			                       + ".\nLine: >>>" + std::string(line) + "<<<");
		Bond bond;
		// =========================================================================
		{ // src, tar
			auto src = getUInt(line.begin(), line.begin() + 3, "atom id in V2000 bond block line");
			if(!src) return std::move(src);
			if(*src < 1 || *src > mol.atoms.size())
				return Result<>::Error("Atom id in V2000 bond block line is out of range. Got "
				                       + std::to_string(*src) + ", but should be in 1 to "
				                       + std::to_string(mol.atoms.size()) + ".");
			auto tar = getUInt(line.begin() + 3, line.begin() + 6, "atom id in V2000 bond block line");
			if(!tar) return std::move(tar);
			if(*tar < 1 || *tar > mol.atoms.size())
				return Result<>::Error("Atom id in V2000 bond block line is out of range. Got "
				                       + std::to_string(*tar) + ", but should be in 1 to "
				                       + std::to_string(mol.atoms.size()) + ".");
			if(*src == *tar)
				return Result<>::Error("Atom IDs for bond endpoints in V2000 bond line are the same. Got "
				                       + std::to_string(*src) + " and " + std::to_string(*tar) + ".");
			bond.src = *src;
			bond.tar = *tar;
		} // src, tar
		if(auto order = getUInt(line.begin() + 6, line.begin() + 9, "bond order in V2000 bond block line")) {
			switch(*order) {
			case 1:
				bond.type = "-";
				bond.bondType = BondType::Single;
				break;
			case 2:
				bond.type = "=";
				bond.bondType = BondType::Double;
				break;
			case 3:
				bond.type = "#";
				bond.bondType = BondType::Triple;
				break;
			case 4:
				bond.type = ":";
				bond.bondType = BondType::Aromatic;
				break;
			case 5:
			case 6:
			case 7: {
				auto res = handleAction(warnings, options.onUnsupportedQueryBondType,
				                        "Unsupported bond type in V2000 bond block line. Got "
				                        + std::to_string(*order) + ".");
				switch(options.onUnsupportedQueryBondType) {
				case Action::Ignore:
				case Action::Warn:
					bond.type = "_Q" + std::to_string(bond.src) + "_" + std::to_string(bond.tar)
					            + "_" + std::to_string(*order);
					bond.bondType = BondType::Invalid;
					assert(res);
					break;
				case Action::Error:
					assert(!res);
					return res;
				}
				break;
			}
			case 8:
				bond.type = "*";
				bond.bondType = BondType::Invalid;
				break;
			default:
				return Result<>::Error("Bond type in V2000 bond block line is out of range. Got "
				                       + std::to_string(*order) + ", but should be in 1 to 8.");
			}
		} else return std::move(order);
		mol.bonds.push_back(std::move(bond));
		++lineCount;
	}
	// ===========================================================================
	// Properties
	auto handleChgRad = [&mol, chargeRadicalCleared = false](
			std::string_view line, const std::string type, const std::string desc, auto output) mutable -> Result<> {
		if(!chargeRadicalCleared) {
			for(Atom &a : mol.atoms) {
				a.chg = Charge();
				a.radical = false;
			}
			chargeRadicalCleared = true;
		}
		if(line.size() < 9)
			return Result<>::Error("V2000 " + type + " line too short. Line of length "
			                       + std::to_string(line.size()) + " was >>>" + std::string(line) + "<<<");
		auto numEntries = getUInt(line.begin() + 6, line.begin() + 9, "number of " + desc + "s in V2000 CHG line");
		if(!numEntries) return std::move(numEntries);
		const auto len = 9 + *numEntries * 4 * 2;
		if(line.size() != len)
			return Result<>::Error("V2000 " + type + " line has wrong size. Expected " + std::to_string(len)
			                       + ", got line of length " + std::to_string(line.size())
			                       + " >>>" + std::string(line) + "<<<");
		for(int i = 0; i != *numEntries; ++i) {
			const auto f = line.begin() + 9 + i * 8;
			const auto l = f + 4;
			auto aId = getUInt(f + 1, l, "atom id in V2000 " + type + " line");
			if(!aId) return std::move(aId);
			if(*aId < 1 || *aId > mol.atoms.size())
				return Result<>::Error("Atom id in V2000 " + type + " line out of range. Got "
				                       + std::to_string(*aId) + ", must be in 1 to "
				                       + std::to_string(mol.atoms.size()) + ".");
			auto val = getInt(f + 4 + 1, l + 4, desc + " in V2000 " + type + " line");
			if(!val) return std::move(val);
			auto res = output(mol.atoms[*aId - 1], *val);
			if(!res) return res;
		}
		return {};
	};
	while(true) {
		// hax to support certain bad writers that don't use 'M  END'
		if(!src.empty() && src[0] == '$')
			break; // emulate that 'M  END' was read

		// and now the normal parsing
		FETCH_LINE(line, "V2000 property line or 'M  END'")
		const auto cmd = line.substr(0, 6);
		if(cmd == "M  END") break;
		if(cmd == "M  CHG") {
			auto res = handleChgRad(line, "CHG", "charge", [&](Atom &a, int charge) {
				a.chg = Charge(charge);
				return Result<>();
			});
			if(!res) return res;
		} else if(cmd == "M  RAD") {
			auto res = handleChgRad(line, "RAD", "radical", [&](Atom &a, int rad) {
				return handleRAD(warnings, a, rad, options);
			});
			if(!res) return res;
		} else if(cmd == "M  ISO") {
			auto numEntries = getUInt(line.begin() + 6, line.begin() + 9, "number of entries in V2000 ISO line");
			if(!numEntries) return std::move(numEntries);
			for(int i = 0; i != *numEntries; ++i) {
				const auto f = line.begin() + 9 + i * 8;
				const auto l = f + 4;
				auto aId = getUInt(f + 1, l, "atom id in V2000 ISO line");
				if(!aId) return std::move(aId);
				if(*aId < 1 || *aId > mol.atoms.size())
					return Result<>::Error(
							"Atom id in V2000 ISO line out of range. Got " + std::to_string(*aId) + ", must be in 1 to " +
							std::to_string(mol.atoms.size()) + ".");
				auto val = getInt(f + 4 + 1, l + 4, "ISO value in V2000 ISO line");
				if(!val) return std::move(val);
				mol.atoms[*aId - 1].iso = Isotope(*val);
			}
		} else if(line.substr(0, 3) == "A  ") { // Atom Alias
			if(line.size() != 6)
				return Result<>::Error("V2000 alias line has wrong size. Expected 6, got line of length "
				                       + std::to_string(line.size()) + " >>>" + std::string(line) + "<<<");
			auto aId = getUInt(line.begin() + 3, line.begin() + 6, "atom ID in V2000 atom alias line");
			if(!aId) return std::move(aId);
			if(*aId < 1 || *aId > mol.atoms.size())
				return Result<>::Error("Atom id in V2000 alias line out of range. Got "
				                       + std::to_string(*aId) + ", must be in 1 to "
				                       + std::to_string(mol.atoms.size()) + ".");
			FETCH_LINE(labelLine, "alias text line in V2000 atom alias")
			if(options.applyV2000AtomAliases) {
				mol.atoms[*aId - 1].symbol = labelLine;
				mol.atoms[*aId - 1].aId = AtomIds::Invalid;
				mol.atoms[*aId - 1].valence = 0;
			}
		} else {
			if(options.fullyIgnoreV2000UnhandledKnownProperty) {
				if(line.substr(0, 3) == "V  ") { // Atom Value
				} else if(line.substr(0, 3) == "G  ") { // Group Abbreviation
				} else if(cmd == "M  RBC") { // Ring Bond Count [Query]
				} else if(cmd == "M  SUB") { // Substitution Count [Query]
				} else if(cmd == "M  UNS") { // Unsaturated Atom [Query]
				} else if(cmd == "M  LIN") { // Link Atom [Query]
				} else if(cmd == "M  ALS") { // Atom List [Query]
				} else if(cmd == "M  APO") { // Attachment Point [Rgroup]
				} else if(cmd == "M  AAL") { // Atom Attachment Order [Rgroup]
				} else if(cmd == "M  RGP") { // Rgroup Label Location [Rgroup]
				} else if(cmd == "M  LOG") { // Rgroup Logic, Unsatisfied Sites, Range of Occurrence [Rgroup]
				} else if(cmd == "M  STY") { // Sgroup Type [Sgroup]
				} else if(cmd == "M  SST") { // Sgroup Subtype [Sgroup]
				} else if(cmd == "M  SLB") { // Sgroup Labels [Sgroup]
				} else if(cmd == "M  SCN") { // Sgroup Connectivity [Sgroup]
				} else if(cmd == "M  SDS") { // Sgroup Expansion [Sgroup]
				} else if(cmd == "M  SAL") { // Sgroup Atom List [Sgroup]
				} else if(cmd == "M  SBL") { // Sgroup Bond List [Sgroup]
				} else if(cmd == "M  SPA") { // Multiple Group Parent Atom List [Sgroup]
				} else if(cmd == "M  SMT") { // Sgroup Subscript [Sgroup]
				} else if(cmd == "M  CRS") { // Sgroup Correspondence [Sgroup]
				} else if(cmd == "M  SDI") { // Sgroup Display Information [Sgroup]
				} else if(cmd == "M  SBV") { // Abbreviation Sgroup Bond and Vector Information [Sgroup]
				} else if(cmd == "M  SDT") { // Data Sgroup Field Description [Sgroup]
				} else if(cmd == "M  SDD") { // Data Sgroup Display Information [Sgroup]
				} else if(cmd == "M  SCD") { // Data Sgroup Data [Sgroup] (initial lines)
				} else if(cmd == "M  SED") { // Data Sgroup Data [Sgroup] (final line)
				} else if(cmd == "M  SPL") { // Sgroup Hierarchy Information [Sgroup]
				} else if(cmd == "M  SCN") { // Sgroup Component Numbers [Sgroup]
				} else if(cmd == "M  $3D") { // 3D Feature Properties [3D]
				} else if(cmd == "M  PXA") { // Phantom Extra Atom
				} else if(cmd == "M  SAP") { // Abbreviation Sgroup Attachment Point
				} else if(cmd == "M  SCL") { // Abbreviation Sgroup Class
				} else if(cmd == "M  SCL") { // Abbreviation Sgroup Class
				} else if(cmd == "M  REG") { // Large REGNO
					// the small REGNO is in line 2
				} else if(cmd == "M  SBT") { // Sgroup Bracket Style
				} else if(cmd == "M  SBT") { // Sgroup Bracket Style
				} else {
					auto res = handleAction(warnings, options.onV2000UnhandledProperty,
					                        "Property line in MOL V2000 not supported. Line was >>>" + std::string(line) +
					                        "<<<");
					if(!res) return res;
				}
			} else {
				auto res = handleAction(warnings, options.onV2000UnhandledProperty,
				                        "Property line in MOL V2000 not supported. Line was >>>" + std::string(line) +
				                        "<<<");
				if(!res) return res;
			}
		}
		++lineCount;
	}
	++lineCount;
	return mol;
}

struct V3000LineResult {
	std::vector<std::string_view> lines;
	std::vector<std::string_view> args;
	std::unique_ptr<std::string> fullLine; // a pointer so it doesn't move if small
public:
	friend std::ostream &operator<<(std::ostream &s, const V3000LineResult &res) {
		if(res.lines.size() == 1) {
			s << "Line >>>" << res.lines.front() << "<<<";
		} else {
			s << "Lines >>>\n";
			for(const auto line : res.lines)
				s << line << '\n';
			s << "<<<";
		}
		return s;
	}
};

namespace argparser {

using Iter = const char *;
using PosIter = IO::PositionIter<Iter>;
using IterRange = boost::iterator_range<PosIter>;

const auto basicValue = x3::rule<struct basicValue>{"non-string value"}
		                        = x3::lexeme[+(x3::char_ - x3::char_("\" =()"))];
const auto string = x3::rule<struct string>{"string"}
		                    = x3::lexeme[x3::char_('"')
		                                 > +((x3::char_ - x3::char_('"')) | x3::lit("\"\""))
		                                 > x3::lit('"')];
const auto val = x3::rule<struct val>{"value"}
		                 = string | basicValue;
const auto listvalInner = x3::rule<struct listvalInner>{"list values"}
		                          = +val;
const auto listval = x3::rule<struct listval>{"list"}
		                     = x3::lit('(') > listvalInner > ')';
const auto optval = x3::rule<struct optval>{"optional value"}
		                    = listval | val;
const auto posarg = x3::rule<struct posarg, IterRange>{"positional argument"}
		                    = x3::raw[val >> !x3::char_('=')];
const auto optarg = x3::rule<struct optarg, IterRange>{"optional argument"}
		                    = x3::raw[basicValue > '=' > optval];
const auto args = x3::rule<struct args, std::vector<IterRange>>{"arguments"}
		                  = *posarg > *optarg;

} // namespace argparser

template<typename Desc>
Result<> parseV3000Args(V3000LineResult &res, const std::string_view line, Desc &&desc) {
	std::vector<argparser::IterRange> args;
	try {
		IO::parse(line.begin(), line.end(), argparser::args, args, false, x3::ascii::space);
	} catch(const lib::IO::ParsingError &e) {
		return Result<>::Error("Parsing error in " + std::string(desc) + ".\n" + e.msg);
	}
	res.args.reserve(args.size());
	for(const auto &arg : args)
		res.args.emplace_back(arg.begin().base(), arg.end().base() - arg.begin().base());
	if(res.args.empty()) return Result<>::Error("Unexpected empty " + std::string(desc) + ".");
	return {};
}

template<typename Desc>
Result<V3000LineResult> parseV3000Line(std::string_view &src, Desc &&desc) {
	V3000LineResult res;
	FETCH_LINE(line, desc)
	if(line.find("M  V30 ") != 0)
		return Result<>::Error("Expected " + std::string(desc) + ", staring with 'M  V30 '."
		                       + "\nLine: >>>" + std::string(line) + "<<<");
	res.lines.push_back(line);
	if(line.back() == '-') {
		res.fullLine.reset(new std::string(line.substr(7, line.size() - 8)));
		while(true) {
			FETCH_LINE(line, desc)
			if(line.find("M  V30 ") != 0)
				return Result<>::Error("Expected " + std::string(desc) + ", staring with 'M  V30 '."
				                       + "\nLine: >>>" + std::string(line) + "<<<");
			res.lines.push_back(line);
			if(line.back() == '-') {
				*res.fullLine += line.substr(7, line.size() - 8);
			} else {
				*res.fullLine += line.substr(7, line.size() - 7);
				break;
			}
		}
		if(auto subres = parseV3000Args(res, *res.fullLine, desc); !subres) return subres;
	} else {
		if(auto subres = parseV3000Args(res, line.substr(7), desc); !subres) return subres;
	}
	return std::move(res); // TODO: remove std::move when C++20/P1825R0 is available
}

Result<MOL>
parseMOLV3000(lib::IO::Warnings &warnings, std::string_view &src, const MDLOptions &options, int &lineCount) {
	if(auto line = parseV3000Line(src, "V3000 BEGIN CTAB line")) {
		if(line->args.size() < 2 || line->args[0] != "BEGIN" || line->args[1] != "CTAB")
			return Result<>::Error("Expected beginning of V3000 CTAB block ('M  V30 BEGIN CTAB').\n"
			                       + boost::lexical_cast<std::string>(*line));
		lineCount += line->lines.size();
	} else return std::move(line);
	auto counts = parseV3000Line(src, "V3000 counts line");
	if(!counts) return std::move(counts);
	if(counts->args.front() != "COUNTS" || counts->args.size() < 6)
		return Result<>::Error("Expected V3000 counts line.\n" + boost::lexical_cast<std::string>(*counts));
	auto numAtoms = getUInt(counts->args[1].begin(), counts->args[1].end(), "atom count");
	if(!numAtoms) return std::move(numAtoms);

	auto numBonds = getUInt(counts->args[2].begin(), counts->args[2].end(), "bond count");
	if(!numBonds) return std::move(numBonds);
	lineCount += counts->lines.size();

	MOL mol;
	mol.atoms.reserve(*numAtoms);
	mol.bonds.reserve(*numBonds);

	// Atom block:
	if(auto line = parseV3000Line(src, "V3000 BEGIN ATOM line")) {
		if(line->args.size() < 2 || line->args[0] != "BEGIN" || line->args[1] != "ATOM")
			return Result<>::Error("Expected V3000 BEGIN ATOM line.\n" + boost::lexical_cast<std::string>(*line));
		lineCount += line->lines.size();
	} else return std::move(line);

	std::map<int, int> lineFromAtomId;
	for(int i = 0; i != *numAtoms; ++i) {
		auto line = parseV3000Line(src, "V3000 atom line");
		if(!line) return std::move(line);
		if(line->args.size() < 6)
			return Result<>::Error("Too few arguments in V3000 atom line.\n" + boost::lexical_cast<std::string>(*line));
		Atom atom;

		auto id = getUInt(line->args[0].begin(), line->args[0].end(), "atom ID");
		if(!id) return std::move(id);
		if(const auto iter = lineFromAtomId.find(*id); iter != end(lineFromAtomId))
			return Result<>::Error("Duplicate atom ID " + std::to_string(*id) + ". Also defined in line "
			                       + std::to_string(iter->second) + ".\n" + boost::lexical_cast<std::string>(*line));
		lineFromAtomId.emplace(*id, lineCount);
		atom.id = *id;
		if(atom.id == 0) return Result<>::Error("Atom ID must be non-zero.\n" + boost::lexical_cast<std::string>(*line));

		atom.symbol = line->args[1];
		if(atom.symbol[0] == '"')
			return Result<>::Error("V3000 quoted atom types not supported.\n" + boost::lexical_cast<std::string>(*line));
		if(atom.symbol == "A" || atom.symbol == "Q" || atom.symbol == "*") {
			// TODO: Q is really "not C or H"
			atom.symbol = "*";
		} else {
			const auto aId = atomIdFromSymbol(atom.symbol);
			if(aId == AtomIds::Invalid && !options.allowAbstract)
				return Result<>::Error("Unknown atom symbol. Got '" + atom.symbol + "'."
				                       + " Set allowAbstract=True to allow it.");
			atom.aId = aId;
		}

		auto aamap = getUInt(line->args[5].begin(), line->args[5].end(), "atom aamap ID");
		if(!aamap) return std::move(aamap);
		atom.aamap = *aamap;

		for(int iArg = 6; iArg != line->args.size(); ++iArg) {
			const auto arg = line->args[iArg];
			const auto eqPos = arg.find('=');
			if(eqPos == std::string_view::npos)
				return Result<>::Error("Optional argument for V3000 atom does not contain '='. Got '" + std::string(arg)
				                       + "'.\n" + boost::lexical_cast<std::string>(*line));
			const auto argName = arg.substr(0, eqPos);
			const auto argVal = arg.substr(eqPos + 1);
			if(argName == "CHG") {
				auto charge = getInt(argVal.begin(), argVal.end(), "V3000 atom CHG");
				if(!charge) return std::move(charge);
				atom.chg = Charge(*charge);
			} else if(argName == "RAD") {
				auto radical = getInt(argVal.begin(), argVal.end(), "V3000 atom RAD");
				if(!radical) return std::move(radical);
				if(auto res = handleRAD(warnings, atom, *radical, options); !res) return res;
			} else if(argName == "VAL") {
				auto val = getInt(argVal.begin(), argVal.end(), "V3000 atom VAL");
				if(!val) return std::move(val);
				if(*val == 0) atom.valence = -1;
				else if(*val == -1) atom.valence = 0;
				else atom.valence = 0;
			} else if(argName == "MASS") {
				auto iso = getUInt(argVal.begin(), argVal.end(), "V3000 atom MASS");
				if(!iso) return std::move(iso);
				atom.iso = Isotope(*iso);
			} else {
				auto res = handleAction(warnings, options.onV3000UnhandledAtomProperty,
				                        "Unknown optional argument to V3000 atom. Got '" + std::string(arg) + "'.\n");
				if(!res) return res;
			}
		}
		mol.atoms.push_back(std::move(atom));
		lineCount += line->lines.size();
	} // for each atom
	if(auto line = parseV3000Line(src, "V3000 END ATOM line")) {
		if(line->args.size() < 2 || line->args[0] != "END" || line->args[1] != "ATOM")
			return Result<>::Error("Expected V3000 END ATOM line.\n" + boost::lexical_cast<std::string>(*line));
		lineCount += line->lines.size();
	} else return std::move(line);

	// Bond block:
	// it's mandatory, but some leave it out if empty
	const auto srcBeforeBondBlock = src;
	auto bondBlockBegin = parseV3000Line(src, "V3000 BEGIN BOND line");
	if(!bondBlockBegin) return std::move(bondBlockBegin);
	if(bondBlockBegin->args.size() < 2 || bondBlockBegin->args[0] != "BEGIN" ||
	   bondBlockBegin->args[1] != "BOND") {
		if(*numBonds > 0) {
			return Result<>::Error(
					"Expected V3000 BEGIN BOND line.\n" + boost::lexical_cast<std::string>(*bondBlockBegin));
		} else {
			src = srcBeforeBondBlock;
		}
	} else { // actual bond block
		lineCount += bondBlockBegin->lines.size();

		std::map<int, int> lineFromBondId;
		for(int i = 0; i != *numBonds; ++i) {
			auto line = parseV3000Line(src, "V3000 bond line");
			if(!line) return std::move(line);
			if(line->args.size() < 4)
				return Result<>::Error("Too few arguments in V3000 bond line.\n" + boost::lexical_cast<std::string>(*line));

			Bond bond;

			auto id = getUInt(line->args[0].begin(), line->args[0].end(), "bond ID");
			if(!id) return std::move(id);
			if(const auto iter = lineFromBondId.find(*id); iter != end(lineFromBondId))
				return Result<>::Error("Duplicate bond ID " + std::to_string(*id)
				                       + ". Also defined in line " + std::to_string(iter->second) + ".\n"
				                       + boost::lexical_cast<std::string>(*line));
			lineFromBondId.emplace(*id, lineCount);
			bond.id = *id;
			if(bond.id == 0)
				return Result<>::Error("Bond ID must be non-zero.\n" + boost::lexical_cast<std::string>(*line));

			// do the src and tar before the type to have them available for the type handling
			auto src = getUInt(line->args[2].begin(), line->args[2].end(), "first atom ID for bond");
			if(!src) return std::move(src);
			if(*src < 1 || *src > mol.atoms.size())
				return Result<>::Error("First atom ID in V3000 bond line is out of range. Got "
				                       + std::to_string(*src) + ", but should be in 1 to " +
				                       std::to_string(mol.atoms.size()) + ".");
			auto tar = getUInt(line->args[3].begin(), line->args[3].end(), "second atom ID for bond");
			if(!tar) return std::move(tar);
			if(*tar < 1 || *tar > mol.atoms.size())
				return Result<>::Error("Second atom ID in V3000 bond line is out of range. Got "
				                       + std::to_string(*src) + ", but should be in 1 to " +
				                       std::to_string(mol.atoms.size()) + ".");
			if(*src == *tar)
				return Result<>::Error("Atom IDs for bond endpoints in V3000 bond line are the same. Got "
				                       + std::to_string(*src) + " and " + std::to_string(*tar) + ".");
			bond.src = *src;
			bond.tar = *tar;

			auto type = getUInt(line->args[1].begin(), line->args[1].end(), "bond type");
			if(!type) return std::move(type);
			switch(*type) {
			case 1:
				bond.type = "-";
				bond.bondType = BondType::Single;
				break;
			case 2:
				bond.type = "=";
				bond.bondType = BondType::Double;
				break;
			case 3:
				bond.type = "#";
				bond.bondType = BondType::Triple;
				break;
			case 4:
				bond.type = ":";
				bond.bondType = BondType::Aromatic;
				break;
			case 5:
			case 6:
			case 7: {
				auto res = handleAction(warnings, options.onUnsupportedQueryBondType,
				                        "Unsupported bond type in V3000 bond block line. Got "
				                        + std::to_string(*type) + ".");
				switch(options.onUnsupportedQueryBondType) {
				case Action::Ignore:
				case Action::Warn:
					bond.type = "_Q" + std::to_string(*src) + "_" + std::to_string(*tar) + "_" + std::to_string(*type);
					bond.bondType = BondType::Invalid;
					assert(res);
					break;
				case Action::Error:
					assert(!res);
					return res;
				}
				break;
			}
			case 8:
				bond.type = "*";
				bond.bondType = BondType::Invalid;
				break;
			case 9:
			case 10:
				return Result<>::Error("Unsupported bond type in V3000 bond type. Got " + std::to_string(*type) + ".");
			default:
				return Result<>::Error("Bond type in V3000 bond line is out of range. Got " + std::to_string(*type)
				                       + ", but should be in 1 to 10.");
			}

			mol.bonds.push_back(std::move(bond));
			lineCount += line->lines.size();
		} // for each bond
		if(auto line = parseV3000Line(src, "V3000 END BOND line")) {
			if(line->args.size() < 2 || line->args[0] != "END" || line->args[1] != "BOND")
				return Result<>::Error("Expected V3000 BOND END line.\n" + boost::lexical_cast<std::string>(*line));
			lineCount += line->lines.size();
		} else return std::move(line);
	} // end of bond block

	// Optional blocks: skip
	while(true) {
		auto line = parseV3000Line(src, "V3000 END CTAB line or BEGIN line");
		if(!line) return std::move(line);
		if(line->args.size() >= 2 && line->args[0] == "END" && line->args[1] == "CTAB") {
			lineCount += line->lines.size();
			break;
		}
		// should be an optional block
		if(line->args.size() < 2 || line->args[0] != "BEGIN")
			return Result<>::Error(
					"Expected V3000 END CTAB line or beginning of block.\n" + boost::lexical_cast<std::string>(*line));
		const auto block = line->args[1];
		lineCount += line->lines.size();
		while(true) {
			auto line = parseV3000Line(src, "V3000 END line or content line");
			if(!line) return std::move(line);
			if(line->args[0] == "BEGIN")
				return Result<>::Error(
						"Nested blocks not supported in optional V3000 blocks.\n" + boost::lexical_cast<std::string>(*line));
			lineCount += line->lines.size();
			if(line->args.size() >= 2 && line->args[0] == "END" && line->args[1] == block)
				break;
		}
	}
	return mol;
}

lib::IO::Result<MOL>
parseMOL(lib::IO::Warnings &warnings, std::string_view &src, const MDLOptions &options, int &lineCount) {
	const int lineFirst = lineCount;
	// Name
	FETCH_LINE(name, "name line in MOL");
	// May not contain:
	// - $MDL (RGfile)
	// - $$$$ (SDfile record separator)
	// - $RXN (rxnfile)
	// - $RDFILE (RDfile headers).
	for(auto &&tag :{"$MDL", "$$$$", "$RXN", "$RDFILE"})
		if(name.find(tag) != name.npos)
			return Result<>::Error(std::string("Expected name line in MOL. Found '") + tag + "'.");
	++lineCount;

	// Various stuff:
	// IIPPPPPPPPMMDDYYHHmmddSSssssssssssEEEEEEEEEEEERRRRRR
	// or blank
	// - I           0: 2  user's first and last initials
	// - P           2:10  program name
	// - M,D,Y,H,m  10:20  date/time
	// - d          20:22  dimensional codes
	// - S          22:24  scaling factor
	// - s          24:34  scaling factor, float
	// - E          34:46  energy
	// - R          46:52  internal registry number
	FETCH_LINE(line2, "'line 2 in MOL'");
	// Note: we do not parse that line.
	++lineCount;

	// Comment
	FETCH_LINE(comment, "comment line in MOL");
	++lineCount;

	// Counts (with version)
	FETCH_LINE(counts, "counts line in MOL");
	constexpr int CountsLen = 11 * 3 + 6;
	if(counts.size() > CountsLen || counts.size() < 6)
		return Result<>::Error("Wrong length of counts line in MOL. Expected length " + std::to_string(CountsLen)
		                       + ", got " + std::to_string(counts.size()) + ".\nLine: >>>" + std::string(counts) + "<<<");
	std::string version;
	if(counts.size() < CountsLen) {
		warnings.add(
				"Counts line in MOL too short. Assuming version=' V2000'. Expected length " + std::to_string(CountsLen) +
				", got " + std::to_string(counts.size()) + ".\nLine: >>>" + std::string(counts) + "<<<\n");
		version = " V2000";
	} else {
		version = std::string(counts.end() - 6, counts.end());
	}
	if(version == " V2000") {
		auto res = parseMOLV2000(warnings, src, options, counts, lineCount);
		if(res) {
			res->lineFirst = lineFirst;
			res->lineLast = lineCount;
			return res;
		} else {
			return Result<>::Error(res.extractError() + "\nMOL started at line " + std::to_string(lineFirst) + ".");
		}
	} else if(version == " V3000") {
		++lineCount;
		auto res = parseMOLV3000(warnings, src, options, lineCount);
		if(res) {
			res->lineFirst = lineFirst;
			res->lineLast = lineCount;
		} else {
			return Result<>::Error(res.extractError() + "\nMOL started at line " + std::to_string(lineFirst) + ".");
		}
		FETCH_LINE(endLine, "V3000 end line ('M  END')")
		if(endLine.substr(0, 6) != "M  END")
			return Result<>::Error("Expected V3000 end line ('M  END').\nLine: >>>" + std::string(endLine) + "<<<");
		++lineCount;
		return res;
	} else {
		return Result<>::Error("Expected version ' V2000' or ' V3000'. Got '" + version + "'.");
	}
}

Result<std::vector<MOL>>
parseSD(lib::IO::Warnings &warnings, std::string_view &src, const MDLOptions &options, int &lineCount) {
	std::vector<MOL> res;
	while(true) {
		auto mol = parseMOL(warnings, src, options, lineCount);
		if(!mol) return std::move(mol);
		res.push_back(std::move(*mol));
		{
			std::string_view line;
			bool hasLine;
			if(std::tie(line, hasLine) = getLine(src); !hasLine)
				return Result<>::Error("Expected SD property line or '$$$$'. Got nothing.");
			if(line == "$$$$") {
				++lineCount;
				if(src.empty()) break;
				continue;
			}
			if(line.empty()) return Result<>::Error("Expected SD property line or '$$$$'. Got empty line.");
			// skip properties
			if(line.front() != '>')
				return Result<>::Error("Expected SD property line or '$$$$'. Got >>>" + std::string(line) + "<<<");
		}
		while(true) {
			++lineCount;
			std::string_view line;
			bool hasLine;
			if(std::tie(line, hasLine) = getLine(src); !hasLine)
				return Result<>::Error("Expected SD property line or blank line. Got nothing.");
			if(line.empty()) break;
			if(line.front() != '>')
				return Result<>::Error("Expected SD property line or blank line. Got >>>" + std::string(line) + "<<<");
		}
		++lineCount;
		{
			std::string_view line;
			bool hasLine;
			if(std::tie(line, hasLine) = getLine(src); !hasLine)
				return Result<>::Error("Expected '$$$$' to end MOL and properties in SD. Got nothing.");
			if(line != "$$$$")
				return Result<>::Error(
						"Expected '$$$$' to end MOL and properties in SD. Got >>>" + std::string(line) + "<<<");
		}
		if(src.empty()) break;
	}
	return res;
}

} // namespace

lib::IO::Result<std::vector<lib::graph::Read::Data>>
readMDLMOL(lib::IO::Warnings &warnings, std::string_view src, const MDLOptions &options) {
	int lineCount = 1;
	auto res = parseMOL(warnings, src, options, lineCount);
	if(!res) return Result<>::Error(res.extractError() + "\nError at line " + std::to_string(lineCount) + ".");
	std::string line;
	while(!src.empty()) {
		const auto[line, hasLine] = getLine(src);
		if(line.empty()) continue;
		return Result<>::Error("Expected end of input. Got line >>>" + std::string(line)
		                       + "<<<\nError at line " + std::to_string(lineCount) + ".");
	}
	return std::move(*res).convert(warnings, options);
}

Result<std::vector<std::vector<lib::graph::Read::Data>>>
readMDLSD(lib::IO::Warnings &warnings, std::string_view src, const MDLOptions &options) {
	std::vector<std::vector<lib::graph::Read::Data>> data;
	int lineCount = 1;
	auto res = parseSD(warnings, src, options, lineCount);
	if(!res) return Result<>::Error(res.extractError() + "\nError at line " + std::to_string(lineCount) + ".");
	std::string_view line;
	bool hasLine;
	if(std::tie(line, hasLine) = getLine(src); hasLine)
		return Result<>::Error("Expected end of input. Got line >>>" + std::string(line)
		                       + "<<<\nError at line " + std::to_string(lineCount) + ".");
	data.reserve(res->size());
	for(auto &mol : *res) {
		auto d = std::move(mol).convert(warnings, options);
		if(!d) return std::move(d);
		data.push_back(std::move(*d));
	}
	return std::move(data); // TODO: remove std::move when C++20/P1825R0 is available
}

namespace {

std::pair<std::string_view, bool> getLine(std::string_view &src) {
	if(src.empty()) return {{}, false};

	const char *first = src.begin();
	int count = 0;
	for(const char *next = first; next != src.end(); ++next) {
		switch(*next) {
		case '\n':
			src.remove_prefix(count + 1);
			return {std::string_view(first, count), true};
		case '\r':
			++next;
			if(next != src.end() && *next == '\n') {
				src.remove_prefix(count + 2);
			} else {
				src.remove_prefix(count + 1);
			}
			return {std::string_view(first, count), true};
		default:
			++count;
		}
	}
	// no trailing newline
	src.remove_prefix(count);
	return {std::string_view(first, count), true};
}

// The following is a copy from Open Babel, of most of src/formats/mdlvalence.h
// Only formatting and a few syntactic things have been changed.

/**********************************************************************
mdlvalence.h - Implement MDL valence model.

Copyright (C) 2012 by NextMove Software

This file is part of the Open Babel project.
For more information, see <http://openbabel.org/>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
 ***********************************************************************/

/* Return the implicit MDL valence for element "elem" with charge "q".  */
unsigned int MDLValence(unsigned int elem, int q, unsigned int val) {
	switch(elem) {
	case 1: // H
	case 3: // Li
	case 11: // Na
	case 19: // K
	case 37: // Rb
	case 55: // Cs
	case 87: // Fr
		if(q == 0 && val <= 1)
			return 1;
		break;

	case 4: // Be
	case 12: // Mg
	case 20: // Ca
	case 38: // Sr
	case 56: // Ba
	case 88: // Ra
		switch(q) {
		case 0:
			if(val <= 2) return 2;
			break;
		case 1:
			if(val <= 1) return 1;
			break;
		}
		break;

	case 5: // B
		switch(q) {
		case -4:
			if(val <= 1) return 1;
			break;
		case -3:
			if(val <= 2) return 2;
			break;
		case -2:
			if(val <= 3) return 3;
			if(val <= 5) return 5;
			break;
		case -1:
			if(val <= 4) return 4;
			break;
		case 0:
			if(val <= 3) return 3;
			break;
		case 1:
			if(val <= 2) return 2;
			break;
		case 2:
			if(val <= 1) return 1;
			break;
		}
		break;

	case 6: // C
		switch(q) {
		case -3:
			if(val <= 1) return 1;
			break;
		case -2:
			if(val <= 2) return 2;
			break;
		case -1:
			if(val <= 3) return 3;
			if(val <= 5) return 5;
			break;
		case 0:
			if(val <= 4) return 4;
			break;
		case 1:
			if(val <= 3) return 3;
			break;
		case 2:
			if(val <= 2) return 2;
			break;
		case 3:
			if(val <= 1) return 1;
			break;
		}
		break;

	case 7: // N
		switch(q) {
		case -2:
			if(val <= 1) return 1;
			break;
		case -1:
			if(val <= 2) return 2;
			break;
		case 0:
			if(val <= 3) return 3;
			if(val <= 5) return 5;
			break;
		case 1:
			if(val <= 4) return 4;
			break;
		case 2:
			if(val <= 3) return 3;
			break;
		case 3:
			if(val <= 2) return 2;
			break;
		case 4:
			if(val <= 1) return 1;
			break;
		}
		break;

	case 8: // O
		switch(q) {
		case -1:
			if(val <= 1) return 1;
			break;
		case 0:
			if(val <= 2) return 2;
			break;
		case 1:
			if(val <= 3) return 3;
			if(val <= 5) return 5;
			break;
		case 2:
			if(val <= 4) return 4;
			break;
		case 3:
			if(val <= 3) return 3;
			break;
		case 4:
			if(val <= 2) return 2;
			break;
		case 5:
			if(val <= 1) return 1;
			break;
		}
		break;

	case 9: // F
		switch(q) {
		case 0:
			if(val <= 1) return 1;
			break;
		case 1:
			if(val <= 2) return 2;
			break;
		case 2:
			if(val <= 3) return 3;
			if(val <= 5) return 5;
			break;
		case 3:
			if(val <= 4) return 4;
			break;
		case 4:
			if(val <= 3) return 3;
			break;
		case 5:
			if(val <= 2) return 2;
			break;
		case 6:
			if(val <= 1) return 1;
			break;
		}
		break;

	case 13: // Al
		switch(q) {
		case -4:
			if(val <= 1) return 1;
			if(val <= 3) return 3;
			if(val <= 5) return 5;
			if(val <= 7) return 7;
			break;
		case -3:
			if(val <= 2) return 2;
			if(val <= 4) return 4;
			if(val <= 6) return 6;
			break;
		case -2:
			if(val <= 3) return 3;
			if(val <= 5) return 5;
			break;
		case -1:
			if(val <= 4) return 4;
			break;
		case 0:
			if(val <= 3) return 3;
			break;
		case 1:
			if(val <= 2) return 2;
			break;
		case 2:
			if(val <= 1) return 1;
			break;
		}
		break;

	case 14: // Si
		switch(q) {
		case -3:
			if(val <= 1) return 1;
			if(val <= 3) return 3;
			if(val <= 5) return 5;
			if(val <= 7) return 7;
			break;
		case -2:
			if(val <= 2) return 2;
			if(val <= 4) return 4;
			if(val <= 6) return 6;
			break;
		case -1:
			if(val <= 3) return 3;
			if(val <= 5) return 5;
			break;
		case 0:
			if(val <= 4) return 4;
			break;
		case 1:
			if(val <= 3) return 3;
			break;
		case 2:
			if(val <= 2) return 2;
			break;
		case 3:
			if(val <= 1) return 1;
			break;
		}
		break;

	case 15: // P
		switch(q) {
		case -2:
			if(val <= 1) return 1;
			if(val <= 3) return 3;
			if(val <= 5) return 5;
			if(val <= 7) return 7;
			break;
		case -1:
			if(val <= 2) return 2;
			if(val <= 4) return 4;
			if(val <= 6) return 6;
			break;
		case 0:
			if(val <= 3) return 3;
			if(val <= 5) return 5;
			break;
		case 1:
			if(val <= 4) return 4;
			break;
		case 2:
			if(val <= 3) return 3;
			break;
		case 3:
			if(val <= 2) return 2;
			break;
		case 4:
			if(val <= 1) return 1;
			break;
		}
		break;

	case 16: // S
		switch(q) {
		case -1:
			if(val <= 1) return 1;
			if(val <= 3) return 3;
			if(val <= 5) return 5;
			if(val <= 7) return 7;
			break;
		case 0:
			if(val <= 2) return 2;
			if(val <= 4) return 4;
			if(val <= 6) return 6;
			break;
		case 1:
			if(val <= 3) return 3;
			if(val <= 5) return 5;
			break;
		case 2:
			if(val <= 4) return 4;
			break;
		case 3:
			if(val <= 3) return 3;
			break;
		case 4:
			if(val <= 2) return 2;
			break;
		case 5:
			if(val <= 1) return 1;
			break;
		}
		break;

	case 17: // Cl
		switch(q) {
		case 0:
			if(val <= 1) return 1;
			if(val <= 3) return 3;
			if(val <= 5) return 5;
			if(val <= 7) return 7;
			break;
		case 1:
			if(val <= 2) return 2;
			if(val <= 4) return 4;
			if(val <= 6) return 6;
			break;
		case 2:
			if(val <= 3) return 3;
			if(val <= 5) return 5;
			break;
		case 3:
			if(val <= 4) return 4;
			break;
		case 4:
			if(val <= 3) return 3;
			break;
		case 5:
			if(val <= 2) return 2;
			break;
		case 6:
			if(val <= 1) return 1;
			break;
		}
		break;

	case 31: // Ga
		switch(q) {
		case -4:
			if(val <= 1) return 1;
			if(val <= 3) return 3;
			if(val <= 5) return 5;
			if(val <= 7) return 7;
			break;
		case -3:
			if(val <= 2) return 2;
			if(val <= 4) return 4;
			if(val <= 6) return 6;
			break;
		case -2:
			if(val <= 3) return 3;
			if(val <= 5) return 5;
			break;
		case -1:
			if(val <= 4) return 4;
			break;
		case 0:
			if(val <= 3) return 3;
			break;
		case 2:
			if(val <= 1) return 1;
			break;
		}
		break;

	case 32: // Ge
		switch(q) {
		case -3:
			if(val <= 1) return 1;
			if(val <= 3) return 3;
			if(val <= 5) return 5;
			if(val <= 7) return 7;
			break;
		case -2:
			if(val <= 2) return 2;
			if(val <= 4) return 4;
			if(val <= 6) return 6;
			break;
		case -1:
			if(val <= 3) return 3;
			if(val <= 5) return 5;
			break;
		case 0:
			if(val <= 4) return 4;
			break;
		case 1:
			if(val <= 3) return 3;
			break;
		case 3:
			if(val <= 1) return 1;
			break;
		}
		break;

	case 33: // As
		switch(q) {
		case -2:
			if(val <= 1) return 1;
			if(val <= 3) return 3;
			if(val <= 5) return 5;
			if(val <= 7) return 7;
			break;
		case -1:
			if(val <= 2) return 2;
			if(val <= 4) return 4;
			if(val <= 6) return 6;
			break;
		case 0:
			if(val <= 3) return 3;
			if(val <= 5) return 5;
			break;
		case 1:
			if(val <= 4) return 4;
			break;
		case 2:
			if(val <= 3) return 3;
			break;
		case 4:
			if(val <= 1) return 1;
			break;
		}
		break;

	case 34: // Se
		switch(q) {
		case -1:
			if(val <= 1) return 1;
			if(val <= 3) return 3;
			if(val <= 5) return 5;
			if(val <= 7) return 7;
			break;
		case 0:
			if(val <= 2) return 2;
			if(val <= 4) return 4;
			if(val <= 6) return 6;
			break;
		case 1:
			if(val <= 3) return 3;
			if(val <= 5) return 5;
			break;
		case 2:
			if(val <= 4) return 4;
			break;
		case 3:
			if(val <= 3) return 3;
			break;
		case 5:
			if(val <= 1) return 1;
			break;
		}
		break;

	case 35: // Br
		switch(q) {
		case 0:
			if(val <= 1) return 1;
			if(val <= 3) return 3;
			if(val <= 5) return 5;
			if(val <= 7) return 7;
			break;
		case 1:
			if(val <= 2) return 2;
			if(val <= 4) return 4;
			if(val <= 6) return 6;
			break;
		case 2:
			if(val <= 3) return 3;
			if(val <= 5) return 5;
			break;
		case 3:
			if(val <= 4) return 4;
			break;
		case 4:
			if(val <= 3) return 3;
			break;
		case 6:
			if(val <= 1) return 1;
			break;
		}
		break;

	case 49: // In
		switch(q) {
		case -4:
			if(val <= 1) return 1;
			if(val <= 3) return 3;
			if(val <= 5) return 5;
			if(val <= 7) return 7;
			break;
		case -3:
			if(val <= 2) return 2;
			if(val <= 4) return 4;
			if(val <= 6) return 6;
			break;
		case -2:
			if(val <= 3) return 3;
			if(val <= 5) return 5;
			break;
		case -1:
			if(val <= 2) return 2;
			if(val <= 4) return 4;
			break;
		case 0:
			if(val <= 3) return 3;
			break;
		case 2:
			if(val <= 1) return 1;
			break;
		}
		break;

	case 50: // Sn
	case 82: // Pb
		switch(q) {
		case -3:
			if(val <= 1) return 1;
			if(val <= 3) return 3;
			if(val <= 5) return 5;
			if(val <= 7) return 7;
			break;
		case -2:
			if(val <= 2) return 2;
			if(val <= 4) return 4;
			if(val <= 6) return 6;
			break;
		case -1:
			if(val <= 3) return 3;
			if(val <= 5) return 5;
			break;
		case 0:
			if(val <= 2) return 2;
			if(val <= 4) return 4;
			break;
		case 1:
			if(val <= 3) return 3;
			break;
		case 3:
			if(val <= 1) return 1;
			break;
		}
		break;

	case 51: // Sb
	case 83: // Bi
		switch(q) {
		case -2:
			if(val <= 1) return 1;
			if(val <= 3) return 3;
			if(val <= 5) return 5;
			if(val <= 7) return 7;
			break;
		case -1:
			if(val <= 2) return 2;
			if(val <= 4) return 4;
			if(val <= 6) return 6;
			break;
		case 0:
			if(val <= 3) return 3;
			if(val <= 5) return 5;
			break;
		case 1:
			if(val <= 2) return 2;
			if(val <= 4) return 4;
			break;
		case 2:
			if(val <= 3) return 3;
			break;
		case 4:
			if(val <= 1) return 1;
			break;
		}
		break;

	case 52: // Te
	case 84: // Po
		switch(q) {
		case -1:
			if(val <= 1) return 1;
			if(val <= 3) return 3;
			if(val <= 5) return 5;
			if(val <= 7) return 7;
			break;
		case 0:
			if(val <= 2) return 2;
			if(val <= 4) return 4;
			if(val <= 6) return 6;
			break;
		case 1:
			if(val <= 3) return 3;
			if(val <= 5) return 5;
			break;
		case 2:
			if(val <= 2) return 2;
			if(val <= 4) return 4;
			break;
		case 3:
			if(val <= 3) return 3;
			break;
		case 5:
			if(val <= 1) return 1;
			break;
		}
		break;

	case 53: // I
	case 85: // At
		switch(q) {
		case 0:
			if(val <= 1) return 1;
			if(val <= 3) return 3;
			if(val <= 5) return 5;
			if(val <= 7) return 7;
			break;
		case 1:
			if(val <= 2) return 2;
			if(val <= 4) return 4;
			if(val <= 6) return 6;
			break;
		case 2:
			if(val <= 3) return 3;
			if(val <= 5) return 5;
			break;
		case 3:
			if(val <= 2) return 2;
			if(val <= 4) return 4;
			break;
		case 4:
			if(val <= 3) return 3;
			break;
		case 6:
			if(val <= 1) return 1;
			break;
		}
		break;

	case 81: // Tl
		switch(q) {
		case -4:
			if(val <= 1) return 1;
			if(val <= 3) return 3;
			if(val <= 5) return 5;
			if(val <= 7) return 7;
			break;
		case -3:
			if(val <= 2) return 2;
			if(val <= 4) return 4;
			if(val <= 6) return 6;
			break;
		case -2:
			if(val <= 3) return 3;
			if(val <= 5) return 5;
			break;
		case -1:
			if(val <= 2) return 2;
			if(val <= 4) return 4;
			break;
		case 0:
			if(val <= 1) return 1;
			if(val <= 3) return 3;
			break;
		}
		break;

	}
	return val;
}

} // namespace
} // namespace mod::lib::Chem