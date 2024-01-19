// see http://www.opensmiles.org/opensmiles.html

//#define BOOST_SPIRIT_X3_DEBUG

#include "Smiles.hpp"

#include <mod/Config.hpp>
#include <mod/Error.hpp>
#include <mod/lib/Algorithm/ConnectedComponents.hpp>
#include <mod/lib/Chem/MoleculeUtil.hpp>
#include <mod/lib/Graph/Properties/Molecule.hpp>
#include <mod/lib/Graph/Properties/String.hpp>
#include <mod/lib/Graph/Properties/Stereo.hpp>
#include <mod/lib/IO/IO.hpp>
#include <mod/lib/IO/Parsing.hpp>
#include <mod/lib/Stereo/Inference.hpp>

#include <boost/fusion/include/std_pair.hpp>
#include <boost/lexical_cast.hpp>

#include <boost/spirit/home/x3/auxiliary/attr.hpp>
#include <boost/spirit/home/x3/auxiliary/eoi.hpp>
#include <boost/spirit/home/x3/auxiliary/eps.hpp>
#include <boost/spirit/home/x3/char/char.hpp>
#include <boost/spirit/home/x3/core/action.hpp>
#include <boost/spirit/home/x3/numeric/uint.hpp>
#include <boost/spirit/home/x3/operator/alternative.hpp>
#include <boost/spirit/home/x3/operator/and_predicate.hpp>
#include <boost/spirit/home/x3/operator/difference.hpp>
#include <boost/spirit/home/x3/operator/kleene.hpp>
#include <boost/spirit/home/x3/operator/not_predicate.hpp>
#include <boost/spirit/home/x3/operator/optional.hpp>
#include <boost/spirit/home/x3/string/literal_string.hpp>
#include <boost/spirit/home/x3/string/symbols.hpp>

#include <iostream>

namespace mod::lib::Chem::Smiles {
using Vertex = lib::Graph::Vertex;
using Edge = lib::Graph::Edge;

struct Charge {
	Charge() = default;

	Charge(std::pair<char, int> data) : charge(data.second) {
		switch(data.first) {
		case 0:
			charge = 0;
			break;
		case '-':
			charge *= -1;
			break;
		case '+':
			break;
		default:
			MOD_ABORT;
		}
	}

	operator int() const {
		return charge;
	}

	friend std::ostream &operator<<(std::ostream &s, Charge c) {
		if(c.charge != 0) {
			s << (c.charge < 0 ? '-' : '+');
			if(std::abs(c.charge) > 1) s << std::abs(c.charge);
		}
		return s;
	}

public:
	int charge = 0;
};

struct Chiral {
	std::string specifier;
	unsigned int k;
private:
	Chiral(std::string specifier, unsigned int k) : specifier(specifier), k(k) {}

public:
	Chiral() = default;

	static Chiral makeSimple(std::string specifier) {
		return Chiral(specifier, 0);
	}

	static Chiral makeTB(unsigned int k) {
		return Chiral("TB", k);
	}

	static Chiral makeOH(unsigned int k) {
		return Chiral("OH", k);
	}

	friend std::ostream &operator<<(std::ostream &s, const Chiral &ch) {
		if(!ch.specifier.empty()) {
			s << ch.specifier;
			if(ch.specifier == "@TB" || ch.specifier == "@OH") s << ch.k;
		}
		return s;
	}
};

struct Atom {
	unsigned int isotope = 0;
	std::string symbol;
	bool isImplicit = false;
	Chiral chiral;
	unsigned int hCount = 0;
	Charge charge;
	bool radical;
	int class_ = -1;
public:
	bool isAromatic = false;
	Vertex vertex;
	AtomId atomId = AtomIds::Invalid;
	std::vector<Vertex> hydrogens;
public:
	int connectedComponentID;
public:
	Atom() = default;

	Atom(const std::string &shorthandSymbol)
			: symbol(shorthandSymbol), isImplicit(true) {}

	friend std::ostream &operator<<(std::ostream &s, const Atom &a) {
		if(!a.isImplicit) {
			s << "[";
			if(a.isotope > 0) s << a.isotope;
		}
		s << a.symbol;
		if(!a.isImplicit) {
			s << a.chiral;
			if(a.hCount > 0) {
				s << "H";
				if(a.hCount > 1) s << a.hCount;
			}
			s << a.charge;
			if(a.radical) s << '.';
			if(a.class_ != -1) s << ":" << a.class_;
			s << "]";
		}
		return s;
	}
};

struct BranchedAtom;
std::ostream &operator<<(std::ostream &s, const BranchedAtom &ba);

struct BondBranchedAtomPair {
	unsigned int bond;
	x3::forward_ast<BranchedAtom> atom;
public:
	Edge edge; // may be uninitialized if bond == '.'
public:
	friend std::ostream &operator<<(std::ostream &s, const BondBranchedAtomPair &bba) {
		if(bba.bond != 0) s << char(bba.bond);
		return s << bba.atom.get();
	}
};

struct ChainTail {
	std::vector<BondBranchedAtomPair> elements;
	int dummy;
public:
	friend std::ostream &operator<<(std::ostream &s, const ChainTail &ct) {
		for(auto &&e: ct.elements) s << e;
		return s;
	}
};

struct RingBond {
	unsigned int bond; // set on the closing end to be the right bond
	unsigned int ringId;
public:
	Edge edge;
	Atom *otherAtom = nullptr; // set on the closing end
	RingBond *otherRingBond = nullptr; // set on the closing end
public:
	friend std::ostream &operator<<(std::ostream &s, const RingBond &rb) {
		if(rb.bond != 0) s << char(rb.bond);
		if(rb.ringId > 9) s << "%";
		return s << rb.ringId;
	}
};

struct BranchedAtom {
	Atom atom;
	std::vector<RingBond> ringBonds;
	std::vector<ChainTail> branches;
public:
	friend std::ostream &operator<<(std::ostream &s, const BranchedAtom &ba) {
		s << ba.atom;
		for(const auto &rb: ba.ringBonds) s << rb;
		for(const auto &b: ba.branches) s << '(' << b << ')';
		return s;
	}

	friend std::ostream &operator<<(std::ostream &s, const x3::forward_ast<BranchedAtom> &ba) {
		if(auto *p = ba.get_pointer()) return s << *p;
		else return s << "NullBranchedAtom";
	}
};

struct SmilesChain {
	BranchedAtom branchedAtom;
	ChainTail tail;
public:
	friend std::ostream &operator<<(std::ostream &s, const SmilesChain &c) {
		return s << c.branchedAtom << c.tail;
	}
};

namespace {
namespace parser {

struct BondSymbol : x3::symbols<unsigned int> {
	BondSymbol() {
		name("bondSymbol");
		for(char c: {'-', '=', '#', '$', ':', '/', '\\'})
			add(std::string(1, c), c);
	}
} bondSymbol;

struct ShorthandSymbol : x3::symbols<Atom> {
	ShorthandSymbol() {
		name("aliphaticOrganic|aromaticOrganic|wildcardSymbol");
		for(auto atomId: getSmilesOrganicSubset())
			add(symbolFromAtomId(atomId), symbolFromAtomId(atomId));
		for(std::string str: {"b", "c", "n", "o", "s", "p"})
			add(str, str);
		add("*", std::string("*"));
	}
} shorthandSymbol;

struct ElementSymbol : x3::symbols<std::string> {
	ElementSymbol() {
		name("elementSymbol");
		for(unsigned char i = 1; i <= AtomIds::Max; i++)
			add(symbolFromAtomId(AtomId(i)), symbolFromAtomId(AtomId(i)));
	}
} elementSymbol;

struct AromaticSymbol : x3::symbols<std::string> {
	AromaticSymbol() {
		name("aromaticSymbol");
		for(std::string str: {"b", "c", "n", "o", "p", "s", "se", "as"}) add(str, str);
	}
} aromaticSymbol;

struct ChiralSymbl : x3::symbols<Chiral> {
	ChiralSymbl() {
		name("chiralSymbol");
		for(std::string str: {"@", "@@", "@TH1", "@TH2", "@AL1", "@AL2", "@SP1", "@SP2", "@SP3"})
			add(str, Chiral::makeSimple(str));
		for(unsigned int i = 1; i < 20; i++)
			add("@TB" + boost::lexical_cast<std::string>(i), Chiral::makeTB(i));
		for(unsigned int i = 1; i < 30; i++)
			add("@OH" + boost::lexical_cast<std::string>(i), Chiral::makeOH(i));
	}
} chiralSymbol;

const auto abstractSymbolChar = x3::char_ - x3::char_("[]:");
const auto nestedAbstractSymbol = x3::rule<struct nestedAbstractSymbol, std::string>("nestedAbstractSymbol");
const auto nestedAbstractSymbol_def =
		*abstractSymbolChar > *(x3::char_('[') > nestedAbstractSymbol > x3::char_(']') > *abstractSymbolChar);
// no recursion
const auto singleDigit = x3::uint_parser<int, 10, 1, 1>();
const auto singleDoubleDigit = x3::rule<struct singleDoubleDigit, int>{"singleDoubleDigit"}
		                               = x3::uint_parser<int, 10, 1, 2>();
const auto doubleDigit = x3::rule<struct doubleDigit, int>{"doubleDigit"}
		                         = x3::uint_parser<int, 10, 2, 2>();
const auto classLabel = x3::rule<struct classLabel, int>{"classLabel"}
		                        = x3::uint_;
const auto class_ = x3::rule<struct class_, int>{"class label"}
		                    = (x3::lit(':') > classLabel)
		                      | (&x3::lit(']') >> x3::attr(-1));
const auto radical = x3::rule<struct radical, bool>{"radical"}
		                     = (x3::lit('.') >> x3::attr(true)) | x3::attr(false);
const auto charge = x3::rule<struct charge, std::pair<char, int> >{"charge"}
		                    = (x3::lit("+++") > x3::attr(std::make_pair('+', 3)))
		                      | (x3::lit("++") > x3::attr(std::make_pair('+', 2)))
		                      | (x3::lit("---") > x3::attr(std::make_pair('-', 3)))
		                      | (x3::lit("--") > x3::attr(std::make_pair('-', 2)))
		                      | (x3::char_("+-") >> singleDoubleDigit)
		                      | (x3::char_("+-") > x3::attr(1))
		                      | x3::attr(std::make_pair('\0', 0));
const auto hcount = x3::rule<struct hcount, unsigned int>{"hydrogen count"}
		                    = (x3::lit('H') > (singleDigit | x3::attr(1))) // singleDigit may NOT parse unary + or -
		                      | x3::attr(0);
const auto chiral = x3::rule<struct chiral, Chiral>{"chiral"}
		                    = chiralSymbol | x3::attr(Chiral());
const auto symbol = x3::rule<struct symbol, std::string>{"symbol"}
		                    = elementSymbol | aromaticSymbol;
const auto isotope = x3::rule<struct isotope, int>{"isotope (not 0)"}
		                     = !x3::lit('0') >> (x3::uint_ | x3::attr(0));
const auto abstractSymbol = x3::rule<struct abstractSymbol, std::string>{"abstractSymbol"}
		                            = (!x3::char_("]:")) > nestedAbstractSymbol;
const auto realAtom = x3::rule<struct realAtom, Atom>{"realAtom"}
		                      = isotope >> symbol >> chiral >> hcount >> charge >> radical >> class_;
const auto wildcardAtom = x3::rule<struct wildcardAtom, Atom>{"wildcardAtom"}
		                          = (x3::attr(0) >> x3::string("*"))
		                            > x3::attr(Chiral()) > hcount > x3::attr(Charge())
		                            > x3::attr(false) > class_;
// See https://github.com/boostorg/spirit/issues/523
// We must reset the attribute, using a semantic action, but that means the attribute
// writing gets disabled, so use semantic actions for that as well.
const auto clearAtom = [](auto &ctx) {
	_val(ctx) = Atom();
};
const auto setSymbol = [](auto &ctx) {
	_val(ctx).symbol = _attr(ctx);
};
const auto setClass = [](auto &ctx) {
	_val(ctx).class_ = _attr(ctx);
};
const auto abstractAtom = x3::rule<struct abstractAtom, Atom>{"abstractAtom"}
		                          = x3::eps[clearAtom] > abstractSymbol[setSymbol] > class_[setClass];
const auto bracketAtomInner = x3::rule<struct bracketAtom, Atom>{"realAtom, wildcardAtom, or abstractAtom"}
		                              = realAtom | wildcardAtom | abstractAtom;
const auto bracketAtom = x3::rule<struct bracketAtom, Atom>{"bracketAtom"}
		                         = x3::lit('[') > bracketAtomInner > ']';
const auto atom = x3::rule<struct atom, Atom>{"atom"}
		                  = bracketAtom | shorthandSymbol;
const auto ringId = singleDigit | (x3::lit('%') > doubleDigit);
const auto ringBond = x3::rule<struct ringBond, RingBond>{"ringBond"}
		                      = (bondSymbol | x3::attr(0)) >> ringId;
const auto bond = bondSymbol | x3::char_('.');

// part of recursion
const x3::rule<struct chainTail, ChainTail> chainTail = "chainTail";
const auto branch = x3::rule<struct branch, ChainTail>{"branch"}
		                    = x3::lit('(') > chainTail > ')';
const auto setAtom = [](auto &ctx) {
	_val(ctx).atom = _attr(ctx);
};
const auto pushRingBond = [](auto &ctx) {
	_val(ctx).ringBonds.push_back(_attr(ctx));
};
const auto pushBranch = [](auto &ctx) {
	_val(ctx).branches.push_back(_attr(ctx));
};
const auto branchedAtom = x3::rule<struct branchedAtom, BranchedAtom>{"branched atom"}
		                          = atom[setAtom] > *(ringBond[pushRingBond] | branch[pushBranch]);
const auto bondBranchedAtomPair = x3::rule<struct bondBranchedAtomPair, BondBranchedAtomPair>{"bondBranchedAtomPair"}
		                                  = (bond > branchedAtom) | (x3::attr(0) >> branchedAtom);
const auto chainTail_def =
		*bondBranchedAtomPair >> x3::attr(0); // dummy attr(0) because of a quirk in the attribute collapsing
BOOST_SPIRIT_DEFINE(nestedAbstractSymbol, chainTail)

// no recursion
const auto smiles = x3::rule<struct smiles, SmilesChain>{"smiles"}
		                    = branchedAtom > chainTail > x3::eoi;

} // namespace

struct RingResolver {
	lib::IO::Result<> operator()(SmilesChain &c) {
		auto res = (*this)(c.branchedAtom);
		if(!res) return res;
		return (*this)(c.tail, c.branchedAtom.atom);
	}

	lib::IO::Result<> operator()(ChainTail &ct, Atom &parent) {
		for(BondBranchedAtomPair &bba: ct.elements) {
			auto res = (*this)(bba.atom.get());
			if(!res) return res;
		}
		return lib::IO::Result<>();
	}

	lib::IO::Result<> operator()(BranchedAtom &bAtom) {
		for(auto &rb: bAtom.ringBonds) {
			const auto iter = openRings.find(rb.ringId);
			if(iter == end(openRings)) {
				openRings.insert(std::make_pair(rb.ringId, std::make_pair(&bAtom.atom, &rb)));
			} else {
				unsigned int &bondOpen = iter->second.second->bond;
				unsigned int &bondClose = rb.bond;
				if(bondOpen != 0 && bondClose != 0 && bondOpen != bondClose) {
					return lib::IO::Result<>::Error(
							"Error in SMILES conversion: ring closure " + std::to_string(rb.ringId) + " can not be both '"
							+ static_cast<char>(bondOpen) + "' and '" + static_cast<char>(bondClose) + "'.");
				}
				if(bondOpen == 0) bondOpen = bondClose;
				if(bondClose == 0) bondClose = bondOpen;
				rb.otherAtom = iter->second.first;
				rb.otherRingBond = iter->second.second;
				openRings.erase(iter);
			}
		}

		for(auto &b: bAtom.branches) {
			auto res = (*this)(b, bAtom.atom);
			if(!res) return res;
		}
		return lib::IO::Result<>();
	}

public:
	std::map<char, std::pair<Atom *, RingBond *>> openRings;
};

Vertex addHydrogen(lib::Graph::GraphType &g, lib::Graph::PropString &pString, Vertex p) {
	Vertex v = add_vertex(g);
	pString.addVertex(v, "H");
	Edge e = add_edge(v, p, g).first;
	pString.addEdge(e, "-");
	return v;
}

lib::IO::Result<> addBond(lib::IO::Warnings &warnings,
                          lib::Graph::GraphType &g, lib::Graph::PropString &pString,
                          Atom &p, Atom &v,
                          char bond, Edge &e,
                          const bool printStereoWarnings) {
	std::string edgeLabel;
	switch(bond) {
	case '/': // note: fall-through to make / and \ implicit
	case '\\':
		warnings.add("up/down bonds are not supported, converted to '-' instead.",
		             printStereoWarnings);
		[[fallthrough]];
	case 0:
		if(p.isAromatic && v.isAromatic) edgeLabel += ':';
		else edgeLabel += '-';
		break;
	case '-':
	case '=':
	case '#':
	case ':':
		edgeLabel += bond;
		break;
	case '.':
		return {};
	case '$':
		return lib::IO::Result<>::Error("Error in SMILES conversion: bond type '$' is not supported.");
	default:
		std::cout << "Internal error, unknown bond '" << bond << "' (" << (int) bond << ")" << std::endl;
		std::cout << "p = '" << pString[p.vertex] << "'(" << p.vertex << ")" << std::endl;
		std::cout << "v = '" << pString[v.vertex] << "'(" << v.vertex << ")" << std::endl;
		MOD_ABORT;
	}
	const auto ePair = add_edge(v.vertex, p.vertex, g);
	assert(ePair.second);
	e = ePair.first;
	assert(!edgeLabel.empty());
	pString.addEdge(e, edgeLabel);
	return {};
}

struct AssignConnectedComponentID {
	int operator()(SmilesChain &c) {
		(*this)(c.branchedAtom);
		(*this)(c.tail);
		return nextID;
	}

	void operator()(ChainTail &ct) {
		for(auto &bap: ct.elements)
			(*this)(bap.atom.get());
	}

	void operator()(BranchedAtom &bAtom) {
		(*this)(bAtom.atom);
		for(auto &b: bAtom.branches)
			(*this)(b);
	}

	void operator()(Atom &a) {
		a.connectedComponentID = nextID;
		++nextID;
	}

public:
	int nextID = 0;
};

struct JoinConnected {
	JoinConnected(ConnectedComponents &components) : components(components) {}

	void operator()(const SmilesChain &c) {
		(*this)(c.branchedAtom);
		(*this)(c.tail, c.branchedAtom.atom);
	}

	void operator()(const ChainTail &ct, const Atom &parent) {
		auto *parentPtr = &parent;
		for(const BondBranchedAtomPair &bba: ct.elements) {
			(*this)(bba, *parentPtr);
			parentPtr = &bba.atom.get().atom;
		}
	}

	void operator()(const BondBranchedAtomPair &bba, const Atom &parent) {
		(*this)(bba.atom.get());
		if(bba.bond != '.')
			join(parent, bba.atom.get().atom);
	}

	void operator()(const BranchedAtom &bAtom) {
		for(const auto &rb: bAtom.ringBonds) {
			if(!rb.otherAtom) continue;
			if(rb.bond != '.')
				join(*rb.otherAtom, bAtom.atom);
		}

		for(auto &b: bAtom.branches)
			(*this)(b, bAtom.atom);
	}

private:
	void join(const Atom &a, const Atom &b) {
		components.join(a.connectedComponentID, b.connectedComponentID);
	}

public:
	ConnectedComponents &components;
};

struct Converter {
	Converter(std::vector<std::unique_ptr<lib::Graph::GraphType>> &gPtrs,
	          std::vector<std::unique_ptr<lib::Graph::PropString>> &pStringPtrs,
	          const ConnectedComponents &components,
	          lib::IO::Warnings &warnings, bool printStereoWarnings,
	          bool allowAbstract)
			: gPtrs(gPtrs), pStringPtrs(pStringPtrs), components(components), warnings(warnings),
			  printStereoWarnings(printStereoWarnings),
			  allowAbstract(allowAbstract) {}

	lib::IO::Result<> operator()(SmilesChain &c) {
		if(auto res = (*this)(c.branchedAtom); !res) return res;
		return (*this)(c.tail, c.branchedAtom.atom);
	}

	lib::IO::Result<> operator()(ChainTail &ct, Atom &parent) {
		auto *parentPtr = &parent;
		for(BondBranchedAtomPair &bba: ct.elements) {
			if(auto res = (*this)(bba, *parentPtr); !res) return res;
			parentPtr = &bba.atom.get().atom;
		}
		return {};
	}

	lib::IO::Result<> operator()(BondBranchedAtomPair &bba, Atom &parent) {
		if(auto res = (*this)(bba.atom.get()); !res) return res;
		if(bba.bond == '.') return {};
		assert(components[bba.atom.get().atom.connectedComponentID]
		       == components[parent.connectedComponentID]);
		const auto comp = components[parent.connectedComponentID];
		auto res = addBond(warnings, *gPtrs[comp], *pStringPtrs[comp],
		                   parent, bba.atom.get().atom, bba.bond, bba.edge,
		                   printStereoWarnings);
		return res;
	}

	lib::IO::Result<> operator()(BranchedAtom &bAtom) {
		if(auto res = (*this)(bAtom.atom); !res) return res;
		// process ring bonds
		for(auto &rb: bAtom.ringBonds) {
			if(!rb.otherAtom) continue; // link ring bond from the other side
			assert(rb.otherRingBond);
			assert(rb.bond != '.');
			assert(components[rb.otherAtom->connectedComponentID]
			       == components[bAtom.atom.connectedComponentID]);
			const auto comp = components[rb.otherAtom->connectedComponentID];
			if(auto res = addBond(warnings, *gPtrs[comp], *pStringPtrs[comp],
			                      *rb.otherAtom, bAtom.atom, rb.bond, rb.edge,
			                      printStereoWarnings);
					!res)
				return res;
			rb.otherRingBond->edge = rb.edge;
		}

		for(auto &b: bAtom.branches)
			if(auto res = (*this)(b, bAtom.atom); !res) return res;
		return {};
	}

	lib::IO::Result<> operator()(Atom &a) {
		const auto comp = components[a.connectedComponentID];
		auto &g = *gPtrs[comp];
		auto &pString = *pStringPtrs[comp];
		if(a.symbol == "*") {
			a.vertex = add_vertex(g);
			pString.addVertex(a.vertex, "*");
			if(a.class_ != -1)
				classToVertexId.emplace(a.class_, std::pair(comp, a.vertex));
			return {};
		}

		a.atomId = atomIdFromSymbol(a.symbol);
		if(a.atomId == AtomIds::Invalid) {
			using namespace AtomIds;
			static const std::map<std::string, AtomId> aromaticSymbols{
					{"b",  Boron},
					{"c",  Carbon},
					{"n",  Nitrogen},
					{"o",  Oxygen},
					{"p",  Phosphorus},
					{"s",  Sulfur},
					{"se", Selenium},
					{"as", Arsenic}};
			auto iter = aromaticSymbols.find(a.symbol);
			if(iter != end(aromaticSymbols)) {
				a.isAromatic = true;
				a.atomId = iter->second;
			} // else it must be an abstract one
		}

		a.vertex = add_vertex(g);
		if(a.atomId == AtomIds::Invalid) {
			if(!allowAbstract)
				return lib::IO::Result<>::Error(
						"SMILES string has abstract vertex label '" + a.symbol + "'. Use allowAbstract=True to allow it.");
			assert(a.isotope == 0);
			assert(!a.isImplicit);
			std::string label = a.symbol;
			if(a.class_ != -1) {
				if(getConfig().graph.appendSmilesClass.get()) {
					label += ":";
					label += boost::lexical_cast<std::string>(a.class_);
				}
				classToVertexId.emplace(a.class_, std::pair(comp, a.vertex));
			}
			pString.addVertex(a.vertex, label);
			return {};
		}

		// an actual real atom!
		std::string label;
		if(a.isotope > 0)
			label += boost::lexical_cast<std::string>(a.isotope);
		label += symbolFromAtomId(a.atomId);

		if(!a.isImplicit) {
			(*this)(a.chiral);
			{ // charge
				unsigned char absCharge = std::abs(a.charge);
				if(absCharge > 1) {
					if(absCharge > 9)
						return lib::IO::Result<>::Error(
								"Error in SMILES conversion, charge 'abs(" + std::to_string(a.charge)
								+ ")' is too large. The lazy developer currently assumes 9 is maximum.");
					label += ('0' + absCharge);
				}
				if(a.charge < 0) label += '-';
				else if(a.charge > 0) label += '+';
			}
			if(a.radical) label += '.';
			if(a.class_ != -1) {
				if(getConfig().graph.appendSmilesClass.get()) {
					label += ":";
					label += boost::lexical_cast<std::string>(a.class_);
				}
				classToVertexId.emplace(a.class_, std::pair(comp, a.vertex));
			}
		}
		pString.addVertex(a.vertex, label);
		return {};
	}

	void operator()(Chiral &ch) {
		if(!ch.specifier.empty())
			hasStereo = true;
	}

private:
	std::vector<std::unique_ptr<lib::Graph::GraphType>> &gPtrs;
	std::vector<std::unique_ptr<lib::Graph::PropString>> &pStringPtrs;
	const ConnectedComponents &components;
	lib::IO::Warnings &warnings;
	const bool printStereoWarnings;
public:
	std::multimap<int, std::pair<int, Vertex>> classToVertexId;
	bool hasStereo = false;
private:
	const bool allowAbstract;
};

struct ExplicitHydrogenAdder {
	ExplicitHydrogenAdder(std::vector<std::unique_ptr<lib::Graph::GraphType>> &gPtrs,
	                      std::vector<std::unique_ptr<lib::Graph::PropString>> &pStringPtrs,
	                      const ConnectedComponents &components)
			: gPtrs(gPtrs), pStringPtrs(pStringPtrs), components(components) {}

	void operator()(SmilesChain &c) {
		(*this)(c.branchedAtom);
		(*this)(c.tail);
	}

	void operator()(ChainTail &ct) {
		for(auto &&bba: ct.elements)
			(*this)(bba.atom.get());
	}

	void operator()(BranchedAtom &bAtom) {
		(*this)(bAtom.atom);
		for(auto &b: bAtom.branches) (*this)(b);
	}

	void operator()(Atom &a) {
		if(!a.isImplicit) {
			for(unsigned int i = 0; i < a.hCount; i++) {
				const auto comp = components[a.connectedComponentID];
				const auto v = addHydrogen(*gPtrs[comp], *pStringPtrs[comp], a.vertex);
				a.hydrogens.push_back(v);
			}
		}
	}

private:
	std::vector<std::unique_ptr<lib::Graph::GraphType>> &gPtrs;
	std::vector<std::unique_ptr<lib::Graph::PropString>> &pStringPtrs;
	const ConnectedComponents &components;
};

struct ImplicitHydrogenAdder {
	ImplicitHydrogenAdder(std::vector<std::unique_ptr<lib::Graph::GraphType>> &gPtrs,
	                      std::vector<std::unique_ptr<lib::Graph::PropString>> &pStringPtrs,
	                      const ConnectedComponents &components)
			: gPtrs(gPtrs), pStringPtrs(pStringPtrs), components(components) {}

	void operator()(SmilesChain &c) {
		(*this)(c.branchedAtom);
		(*this)(c.tail);
	}

	void operator()(ChainTail &ct) {
		for(auto &&bba: ct.elements)
			(*this)(bba.atom.get());
	}

	void operator()(BranchedAtom &bAtom) {
		(*this)(bAtom.atom);
		for(auto &b: bAtom.branches) (*this)(b);
	}

	void operator()(Atom &a) {
		// only implicit atoms, and only non-wildcard atoms
		if(a.isImplicit && a.atomId != AtomIds::Invalid) {
			const auto comp = components[a.connectedComponentID];
			addImplicitHydrogens(*gPtrs[comp], *pStringPtrs[comp], a.vertex, a.atomId, &addHydrogen);
		}
	}

private:
	std::vector<std::unique_ptr<lib::Graph::GraphType>> &gPtrs;
	std::vector<std::unique_ptr<lib::Graph::PropString>> &pStringPtrs;
	const ConnectedComponents &components;
};

struct StereoConverter {
	StereoConverter(std::vector<std::unique_ptr<lib::Graph::GraphType>> &gPtrs,
	                const std::vector<lib::Graph::PropMolecule> &pMols,
	                const ConnectedComponents &components,
	                lib::IO::Warnings &warnings,
	                bool printStereoWarnings)
			: gPtrs(gPtrs), pMols(pMols), components(components),
			  warnings(warnings), printStereoWarnings(printStereoWarnings),
			  hasAssigned(gPtrs.size(), false) {
		infs.reserve(gPtrs.size());
		for(int i = 0; i != gPtrs.size(); ++i)
			infs.emplace_back(*gPtrs[i], pMols[i], false);
	}

	lib::IO::Result<> operator()(const SmilesChain &c) {
		const Atom *next = nullptr;
		if(!c.tail.elements.empty())
			next = &c.tail.elements.front().atom.get().atom;
		if(auto res = (*this)(c.branchedAtom, nullptr, next); !res) return res;
		return (*this)(c.tail, c.branchedAtom.atom);
	}

	lib::IO::Result<> operator()(const ChainTail &ct, const Atom &prev) {
		lib::IO::Result<> result;
		const Atom *prevPtr = &prev;
		for(int i = 0; i != ct.elements.size(); ++i) {
			const Atom *nextPtr = i + 1 == ct.elements.size() ? nullptr
			                                                  : &ct.elements[i + 1].atom.get().atom;
			const auto &bba = ct.elements[i];
			if(auto res = (*this)(ct.elements[i].atom.get(), prevPtr, nextPtr); !res) return res;
			prevPtr = &bba.atom.get().atom;
		}
		return result;
	}

	lib::IO::Result<> operator()(const BranchedAtom &bAtom, const Atom *prev, const Atom *next) {
		if(!bAtom.atom.isImplicit) {
			const auto &ch = bAtom.atom.chiral;
			if(!ch.specifier.empty()) {
				if(ch.specifier == "@" || ch.specifier == "@@") {
					if(auto res = assignTetrahedral(ch.specifier, bAtom, prev, next); !res) return res;
				} else { // not @ or @@
					warnings.add("Stereochemical information ("
					             + boost::lexical_cast<std::string>(ch)
					             + ") in SMILES string not handled.",
					             printStereoWarnings);
				}
			}
		}
		for(auto &b: bAtom.branches)
			if(auto res = (*this)(b, bAtom.atom); !res) return res;
		return {};
	}

private:
	lib::IO::Result<>
	assignTetrahedral(const std::string &winding, const BranchedAtom &bAtom, const Atom *prev, const Atom *next) {
		const auto v = bAtom.atom.vertex;
		const auto comp = components[bAtom.atom.connectedComponentID];
		const auto &g = *gPtrs[comp];
		const std::vector<Edge> oes(out_edges(v, g).first, out_edges(v, g).second);
		if(oes.size() != 4) {
			warnings.add("Ignoring stereo information in SMILES. Can not add tetrahedral geometry to vertex ("
			             + boost::lexical_cast<std::string>(bAtom.atom) + ") with degree " +
			             std::to_string(oes.size()) + ". Must be 4.",
			             printStereoWarnings);
			return {};
		}
		std::vector<bool> used(oes.size(), false);
		std::vector<Edge> edgesToAdd;
		const auto addNeighbour = [&](const auto &v) {
			const auto eIter = std::find_if(oes.begin(), oes.end(), [v, &g](const auto &e) {
				return target(e, g) == v;
			});
			assert(eIter != oes.end());
			assert(!used[eIter - oes.begin()]);
			edgesToAdd.push_back(*eIter);
			used[eIter - oes.begin()] = true;
		};
		// first the previous atom
		if(prev) addNeighbour(prev->vertex);
		// ring closures
		for(const auto &rb: bAtom.ringBonds) {
			const auto vSrc = source(rb.edge, g);
			const auto vTar = target(rb.edge, g);
			assert(vSrc != vTar);
			addNeighbour(vSrc == v ? vTar : vSrc);
		}
		// hydrogens from atom property
		for(const auto &vH: bAtom.atom.hydrogens)
			addNeighbour(vH);
		// now add all branches
		for(const ChainTail &ct: bAtom.branches) {
			const auto vBranch = ct.elements.front().atom.get().atom.vertex;
			addNeighbour(vBranch);
		}
		// the tail
		if(next) addNeighbour(next->vertex);
		// And finally try to add them to the inferrence.
		// But for now, only if we are sure to add all edges.
		assert(edgesToAdd.size() == oes.size());
		hasAssigned[comp] = true;
		auto &inf = infs[comp];
		if(auto res = inf.assignGeometry(v, lib::Stereo::getGeometryGraph().tetrahedral); !res) return res;
		inf.initEmbedding(v);
		inf.fixSimpleGeometry(v);
		if(winding == "@@" && !edgesToAdd.empty())
			std::reverse(edgesToAdd.begin() + 1, edgesToAdd.end());
		for(const auto &e: edgesToAdd) inf.addEdge(v, e);
		return {};
	}

public:
	std::vector<std::unique_ptr<lib::Graph::GraphType>> &gPtrs;
	const std::vector<lib::Graph::PropMolecule> &pMols;
	const ConnectedComponents &components;
	lib::IO::Warnings &warnings;
	const bool printStereoWarnings;
public:
	std::vector<lib::Stereo::Inference<lib::Graph::GraphType, lib::Graph::PropMolecule>> infs;
	std::vector<bool> hasAssigned;
};

lib::IO::Result<std::vector<lib::Graph::Read::Data>>
parseSmiles(lib::IO::Warnings &warnings, const bool printStereoWarnings,
            std::string_view smiles, const bool allowAbstract,
            SmilesClassPolicy classPolicy) {
	SmilesChain ast;
	try {
		lib::IO::parse(smiles.begin(), smiles.end(), parser::smiles, ast, true);
	} catch(const lib::IO::ParsingError &e) {
		return lib::IO::Result<>::Error(e.msg);
	}
	if(getConfig().graph.smilesCheckAST.get()) {
		std::stringstream astStr;
		astStr << ast;
		if(smiles != astStr.str()) {
			std::cout << "Converting: >>>" << smiles << "<<< " << smiles.size() << std::endl;
			std::cout << "Ast:        >>>" << astStr.str() << "<<< " << astStr.str().size() << std::endl;
			for(auto c: smiles) {
				if(std::isprint(c)) std::cout << c << ' ';
				else std::cout << int(c) << ' ';
			}
			std::cout << std::endl;
			for(auto c: astStr.str()) {
				if(std::isprint(c)) std::cout << c << ' ';
				else std::cout << int(c) << ' ';
			}
			std::cout << std::endl;
			MOD_ABORT;
		}
	}

	{ // Rings
		RingResolver rings;
		if(auto res = rings(ast); !res) return res;
		if(!rings.openRings.empty()) {
			auto iter = begin(rings.openRings);
			std::string msg = "Error in SMILES conversion: unclosed rings ("
			                  + std::to_string(static_cast<int>(iter->first));
			for(iter++; iter != end(rings.openRings); iter++) {
				msg += ", ";
				msg += std::to_string(static_cast<int>(iter->first));
			}
			msg += ")\n";
			return lib::IO::Result<>::Error(std::move(msg));
		}
	}

	const int numAtoms = AssignConnectedComponentID()(ast);
	ConnectedComponents components(numAtoms);
	(JoinConnected(components)(ast));
	const int numComponents = components.finalize();
	for(int i = 0; i != numAtoms; ++i) {
		assert(components[i] >= 0);
		assert(components[i] < numComponents);
	}

	std::vector<std::unique_ptr<lib::Graph::GraphType>> gPtrs(numComponents);
	std::vector<std::unique_ptr<lib::Graph::PropString>> pStringPtrs(numComponents);
	for(int i = 0; i != numComponents; ++i) {
		gPtrs[i] = std::make_unique<lib::Graph::GraphType>();
		pStringPtrs[i] = std::make_unique<lib::Graph::PropString>(*gPtrs[i]);
	}
	Converter conv(gPtrs, pStringPtrs, components, warnings, printStereoWarnings, allowAbstract);
	if(auto res = conv(ast); !res) return res;
	(ExplicitHydrogenAdder(gPtrs, pStringPtrs, components))(ast);
	(ImplicitHydrogenAdder(gPtrs, pStringPtrs, components)(ast));

	std::vector<lib::Graph::Read::Data> datas(numComponents);

	const auto iter = std::find_if(conv.classToVertexId.begin(), conv.classToVertexId.end(), [&conv](auto &vp) {
		return conv.classToVertexId.count(vp.first) > 1;
	});
	bool assignExternalIds = true;
	switch(classPolicy) {
	case SmilesClassPolicy::NoneOnDuplicate:
		if(iter != conv.classToVertexId.end())
			assignExternalIds = false;
		break;
	case SmilesClassPolicy::ThrowOnDuplicate:
		if(iter != conv.classToVertexId.end()) {
			return lib::IO::Result<>::Error(
					"Error in SMILES conversion: class label " + std::to_string(iter->first) + " is used more than once ("
					+ std::to_string(conv.classToVertexId.count(iter->first)) + "), and the class label policy is "
					+ boost::lexical_cast<std::string>(SmilesClassPolicy::ThrowOnDuplicate) + ".");
		}
		break;
	case SmilesClassPolicy::MapUnique:
		break;
	}
	if(assignExternalIds) {
		if(classPolicy == SmilesClassPolicy::MapUnique) {
			for(auto &&vp: conv.classToVertexId) {
				if(conv.classToVertexId.count(vp.first) > 1) continue;
				datas[vp.second.first].externalToInternalIds[vp.first]
						= get(boost::vertex_index_t(), *gPtrs[vp.second.first], vp.second.second);
			}
		} else {
			for(auto &&vp: conv.classToVertexId) {
				datas[vp.second.first].externalToInternalIds[vp.first]
						= get(boost::vertex_index_t(), *gPtrs[vp.second.first], vp.second.second);
			}
		}
	}

	if(conv.hasStereo) {
		std::vector<lib::Graph::PropMolecule> pMols;
		for(int i = 0; i != numComponents; ++i)
			pMols.emplace_back(*gPtrs[i], *pStringPtrs[i]);
		StereoConverter stereoConv(gPtrs, pMols, components, warnings, printStereoWarnings);
		if(auto res = stereoConv(ast); !res) return res;
		for(int i = 0; i != numComponents; ++i) {
			if(stereoConv.hasAssigned[i]) {
				auto deductionResult = stereoConv.infs[i].finalize(warnings, printStereoWarnings, [&](auto v) {
					return std::to_string(get(boost::vertex_index_t(), *gPtrs[i], v));
				});
				if(!deductionResult) return deductionResult;
				datas[i].pStereo = std::make_unique<lib::Graph::PropStereo>(*gPtrs[i], std::move(stereoConv.infs[i]));
			}
		}
	}
	for(int i = 0; i != numComponents; ++i) {
		assert(gPtrs[i]);
		datas[i].g = std::move(gPtrs[i]);
		datas[i].pString = std::move(pStringPtrs[i]);
	}
	return lib::IO::Result<std::vector<lib::Graph::Read::Data>>(std::move(datas));
}

} // namespace
} // namespace mod::lib::Chem::Smiles
namespace mod::lib::Chem {

lib::IO::Result<std::vector<lib::Graph::Read::Data>>
readSmiles(lib::IO::Warnings &warnings, bool printStereoWarnings,
           std::string_view src, const bool allowAbstract,
           SmilesClassPolicy classPolicy) {
	return Smiles::parseSmiles(warnings, printStereoWarnings,
	                           src, allowAbstract, classPolicy);
}

} // namespace mod::lib::Chem

BOOST_FUSION_ADAPT_STRUCT(mod::lib::Chem::Smiles::Chiral,
                          (std::string, specifier)
		                          (unsigned int, k)
)
BOOST_FUSION_ADAPT_STRUCT(mod::lib::Chem::Smiles::Atom,
                          (unsigned int, isotope)
		                          (std::string, symbol)
		                          (mod::lib::Chem::Smiles::Chiral, chiral)
		                          (unsigned int, hCount)
		                          (mod::lib::Chem::Smiles::Charge, charge)
		                          (bool, radical)
		                          (int, class_)
)
BOOST_FUSION_ADAPT_STRUCT(mod::lib::Chem::Smiles::BondBranchedAtomPair,
                          (unsigned int, bond)
		                          (x3::forward_ast<mod::lib::Chem::Smiles::BranchedAtom>, atom)
)
BOOST_FUSION_ADAPT_STRUCT(mod::lib::Chem::Smiles::ChainTail,
                          (std::vector<mod::lib::Chem::Smiles::BondBranchedAtomPair>, elements)
		                          (int, dummy)
)
BOOST_FUSION_ADAPT_STRUCT(mod::lib::Chem::Smiles::RingBond,
                          (unsigned int, bond)
		                          (unsigned int, ringId)
)
BOOST_FUSION_ADAPT_STRUCT(mod::lib::Chem::Smiles::SmilesChain,
                          (mod::lib::Chem::Smiles::BranchedAtom, branchedAtom)
		                          (mod::lib::Chem::Smiles::ChainTail, tail)
)