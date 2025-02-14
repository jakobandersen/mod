#include "MoleculeUtil.hpp"

#include <mod/Config.hpp>
#include <mod/Error.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

namespace mod::lib::Chem {

//------------------------------------------------------------------------------
// Other
//------------------------------------------------------------------------------

AtomId atomIdFromSymbol(std::string_view label) {
	using namespace AtomIds;
	const auto len = label.size();
	if(len == 1) {
		switch(label[0]) {
		case 'B': return B;
		case 'C': return C;
		case 'F': return F;
		case 'H': return H;
		case 'I': return I;
		case 'K': return K;
		case 'N': return N;
		case 'O': return O;
		case 'P': return P;
		case 'S': return S;
		case 'U': return U;
		case 'V': return V;
		case 'W': return W;
		case 'Y': return Y;
		default: return Invalid;
		}
	} else if(len == 2) {
		char c1 = label[0], c2 = label[1];
		switch(c1) {
		case 'Z':
			switch(c2) {
			case 'r': return Zr;
			case 'n': return Zn;
			default: return Invalid;
			}
		case 'Y':
			switch(c2) {
			case 'b': return Yb;
			default: return Invalid;
			}
		case 'X':
			switch(c2) {
			case 'e': return Xe;
			default: return Invalid;
			}
		case 'S':
			switch(c2) {
			case 'r': return Sr;
			case 'i': return Si;
			case 'n': return Sn;
			case 'm': return Sm;
			case 'c': return Sc;
			case 'b': return Sb;
			case 'g': return Sg;
			case 'e': return Se;
			default: return Invalid;
			}
		case 'R':
			switch(c2) {
			case 'u': return Ru;
			case 'h': return Rh;
			case 'n': return Rn;
			case 'b': return Rb;
			case 'a': return Ra;
			case 'g': return Rg;
			case 'f': return Rf;
			case 'e': return Re;
			default: return Invalid;
			}
		case 'P':
			switch(c2) {
			case 'r': return Pr;
			case 'u': return Pu;
			case 't': return Pt;
			case 'o': return Po;
			case 'm': return Pm;
			case 'b': return Pb;
			case 'a': return Pa;
			case 'd': return Pd;
			default: return Invalid;
			}
		case 'T':
			switch(c2) {
			case 'i': return Ti;
			case 'h': return Th;
			case 'm': return Tm;
			case 'l': return Tl;
			case 'c': return Tc;
			case 'b': return Tb;
			case 'a': return Ta;
			case 'e': return Te;
			default: return Invalid;
			}
		case 'K':
			switch(c2) {
			case 'r': return Kr;
			default: return Invalid;
			}
		case 'I':
			switch(c2) {
			case 'r': return Ir;
			case 'n': return In;
			default: return Invalid;
			}
		case 'H':
			switch(c2) {
			case 's': return Hs;
			case 'o': return Ho;
			case 'g': return Hg;
			case 'f': return Hf;
			case 'e': return He;
			default: return Invalid;
			}
		case 'O':
			switch(c2) {
			case 's': return Os;
			default: return Invalid;
			}
		case 'N':
			switch(c2) {
			case 'p': return Np;
			case 'i': return Ni;
			case 'o': return No;
			case 'b': return Nb;
			case 'a': return Na;
			case 'e': return Ne;
			case 'd': return Nd;
			default: return Invalid;
			}
		case 'M':
			switch(c2) {
			case 'o': return Mo;
			case 't': return Mt;
			case 'g': return Mg;
			case 'n': return Mn;
			case 'd': return Md;
			default: return Invalid;
			}
		case 'L':
			switch(c2) {
			case 'r': return Lr;
			case 'a': return La;
			case 'i': return Li;
			case 'u': return Lu;
			case 'v': return Lv;
			default: return Invalid;
			}
		case 'C':
			switch(c2) {
			case 's': return Cs;
			case 'r': return Cr;
			case 'u': return Cu;
			case 'o': return Co;
			case 'n': return Cn;
			case 'm': return Cm;
			case 'l': return Cl;
			case 'a': return Ca;
			case 'f': return Cf;
			case 'e': return Ce;
			case 'd': return Cd;
			default: return Invalid;
			}
		case 'B':
			switch(c2) {
			case 'r': return Br;
			case 'k': return Bk;
			case 'i': return Bi;
			case 'h': return Bh;
			case 'a': return Ba;
			case 'e': return Be;
			default: return Invalid;
			}
		case 'A':
			switch(c2) {
			case 's': return As;
			case 'r': return Ar;
			case 'u': return Au;
			case 't': return At;
			case 'm': return Am;
			case 'l': return Al;
			case 'c': return Ac;
			case 'g': return Ag;
			default: return Invalid;
			}
		case 'G':
			switch(c2) {
			case 'a': return Ga;
			case 'e': return Ge;
			case 'd': return Gd;
			default: return Invalid;
			}
		case 'F':
			switch(c2) {
			case 'r': return Fr;
			case 'm': return Fm;
			case 'e': return Fe;
			case 'l': return Fl;
			default: return Invalid;
			}
		case 'E':
			switch(c2) {
			case 's': return Es;
			case 'r': return Er;
			case 'u': return Eu;
			default: return Invalid;
			}
		case 'D':
			switch(c2) {
			case 's': return Ds;
			case 'b': return Db;
			case 'y': return Dy;
			default: return Invalid;
			}
		default: return Invalid;
		}
	} else if(len == 3) {
		char c1 = label[0], c2 = label[1], c3 = label[2];
		switch(c1) {
		case 'U':
			switch(c2) {
			case 'u':
				switch(c3) {
				case 't': return Uut;
				case 'o': return Uuo;
				case 'p': return Uup;
				case 's': return Uus;
				default: return Invalid;
				}
			default: return Invalid;
			}
		default: return Invalid;
		}
	} else return Invalid;
}

VertexLabelParseResult parseVertexLabel(std::string_view label) {
	assert(!label.empty());
	auto first = label.begin();
	// note: the range is inclusive, i.e., [first, end], _not_ [first, last[
	auto end = label.end() - 1;
	// radical
	bool radical = false;
	if(first != end && *end == '.') {
		radical = true;
		--end;
	}
	// charge
	signed char charge = 0;
	if(end != first // "+" and "-" as labels do not have charge
	   && (*end == '+' || *end == '-')) {
		charge = *end == '+' ? 1 : -1;
		--end;
		if(first != end) { // "4+" have charge +1, and label "4"
			if(*end >= '0' && *end <= '9') {
				charge *= *end - '0';
				--end;
			}
		}
	}
	// isotope
	int isotope = 0;
	if(first != end) { // still inclusive range, leave at least 1 char (in the end) for the label
		// leave at least 1 char for the atom type
		if(*first != '0') {
			for(; first != end && *first >= '0' && *first <= '9'; ++first) {
				isotope *= 10;
				isotope += *first - '0';
			}
		}
	}
	if(isotope == 0) isotope = -1;
//	std::cout << "Prefix: " << std::string(label.begin(), first) << ", len=" << (first - label.begin()) << std::endl;
//	std::cout << "Suffix: " << std::string(end + 1, label.end()) << ", len=" << (label.end() - (end + 1)) << std::endl;
//	std::cout << "Label before: " << label << std::endl;
	label.remove_suffix(label.end() - (end + 1));
	label.remove_prefix(first - label.begin());
//	std::cout << "Label after:  " << label << std::endl;
	return {{Isotope(isotope), Charge(charge), radical}, label};
}

VertexLabelDecodeResult decodeVertexLabel(std::string_view label) {
	// This is a kind of "best effort", meaning that 42Lol4+. is decoded to isotope=42, charge=4, radical=.
	const auto ex = parseVertexLabel(label);
	auto atomId = atomIdFromSymbol(ex.rest);
	return {ex, atomId};
}

BondType decodeEdgeLabel(std::string_view label) {
	if(label.size() == 1) {
		switch(label[0]) {
		case '-': return BondType::Single;
		case ':': return BondType::Aromatic;
		case '=': return BondType::Double;
		case '#': return BondType::Triple;
		default: return BondType::Invalid;
		}
	} else return BondType::Invalid;
}

void markSpecialAtomsUsed(std::vector<bool> &used) {
	used[AtomIds::Hydrogen] = true;
	used[AtomIds::Carbon] = true;
	used[AtomIds::Nitrogen] = true;
	used[AtomIds::Oxygen] = true;
}

std::string symbolFromAtomId(AtomId atomId) {
	switch(atomId) {
#define MOD_toString(s) MOD_toString1(s)
#define MOD_toString1(s) #s
#define MDO_CHEM_atomIdIter(r, data, t)                                         \
    case BOOST_PP_TUPLE_ELEM(MOD_CHEM_ATOM_DATA_ELEM_SIZE(), 0, t): return MOD_toString(BOOST_PP_TUPLE_ELEM(3, 1, t));
	BOOST_PP_SEQ_FOR_EACH(MDO_CHEM_atomIdIter, ~, MOD_CHEM_ATOM_DATA())
	default: MOD_ABORT;
	}
#undef MDO_CHEM_atomIdIter
#undef MOD_toString1
#undef MOD_toString
}

void appendSymbolFromAtomId(std::string &s, AtomId atomId) {
	switch(atomId) {
#define MOD_toString(s) MOD_toString1(s)
#define MOD_toString1(s) #s
#define MDO_CHEM_atomIdIter(r, data, t)                                                      \
    case BOOST_PP_TUPLE_ELEM(MOD_CHEM_ATOM_DATA_ELEM_SIZE(), 0, t):                            \
        for(unsigned int i = 0; i < sizeof(MOD_toString(BOOST_PP_TUPLE_ELEM(3, 1, t))) - 1; i++) \
            s += MOD_toString(BOOST_PP_TUPLE_ELEM(3, 1, t))[i];                                    \
        break;
	BOOST_PP_SEQ_FOR_EACH(MDO_CHEM_atomIdIter, ~, MOD_CHEM_ATOM_DATA())
	default: MOD_ABORT;
	}
#undef MDO_CHEM_atomIdIter
#undef MOD_toString1
#undef MOD_toString
}

char bondToChar(BondType bt) {
	switch(bt) {
	case BondType::Invalid: MOD_ABORT;
		break;
	case BondType::Single: return '-';
	case BondType::Aromatic: return ':';
	case BondType::Double: return '=';
	case BondType::Triple: return '#';
	default: MOD_ABORT;
	}
}

std::string chargeSuffix(Charge c) {
	std::string s;
	if(c == 0) return s;
	if(c > 1 || c < -1) s += boost::lexical_cast<std::string>(std::abs(c));
	if(c < 0) s += '-';
	else s += '+';
	return s;
}

} // namespace mod::lib::Chem