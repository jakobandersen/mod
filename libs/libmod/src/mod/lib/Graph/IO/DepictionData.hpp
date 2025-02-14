#ifndef MOD_LIB_GRAPH_IO_DEPICTIONDATA_HPP
#define MOD_LIB_GRAPH_IO_DEPICTIONDATA_HPP

#include <mod/BuildConfig.hpp>
#include <mod/Chem.hpp>
#include <mod/lib/Chem/OBabel.hpp>
#include <mod/lib/Graph/LabelledGraph.hpp>

namespace mod {
struct AtomId;
struct Charge;
struct AtomData;
enum class BondType;
template<typename Sig>
class Function;
} // namespace mod
namespace mod::lib::IO::Graph::Write {
enum struct EdgeFake3DType;
} // namespace mod::lib::IO::Graph::Write
namespace mod::lib::graph {
struct PropMolecule;
struct PropString;
} // namespace mod::lib::Grpah
namespace mod::lib::graph::Write {

struct DepictionData {
	DepictionData(const DepictionData &) = delete;
	DepictionData &operator=(const DepictionData &) = delete;
public:
	DepictionData(const LabelledGraph &lg);
public: // used in GraphWriteGeneric
	AtomId getAtomId(Vertex v) const; // shortcut to PropMolecule
	Isotope getIsotope(Vertex v) const; // shortcut to PropMolecule
	Charge getCharge(Vertex v) const; // shortcut to PropMolecule
	bool getRadical(Vertex v) const; // shortcut to PropMolecule
	std::string getVertexLabelNoIsotopeChargeRadical(Vertex v) const;
	BondType getBondData(Edge e) const; // shortcut to PropMolecule
	std::string getEdgeLabel(Edge e) const;
	bool hasImportantStereo(Vertex v) const;
	lib::IO::Graph::Write::EdgeFake3DType getEdgeFake3DType(Edge e, bool withHydrogen) const;
	std::string getRawStereoString(Vertex v) const;
	std::string getPrettyStereoString(Vertex v) const;
	std::string getStereoString(Edge e) const;
	bool getHasCoordinates() const;
	// pre: getHasCoordinates()
	double getX(Vertex v, bool withHydrogen) const;
	// pre: getHasCoordinates()
	double getY(Vertex v, bool withHydrogen) const;
public:
	int getOutputId(Vertex v) const;
public: // used for Coordinate handling
	const AtomData &operator()(Vertex v) const; // fake data
	BondType operator()(Edge e) const; // fake data
public: // custom depiction
	void setImage(std::shared_ptr<mod::Function<std::string()> > image);
	std::shared_ptr<mod::Function<std::string()> > getImage() const;
	void setImageCommand(std::string cmd);
	std::string getImageCommand() const;
private:
#ifdef MOD_HAVE_OPENBABEL
	const lib::Chem::OBMolHandle &getOB(bool withHydrogen) const;
#endif
private:
	const LabelledGraph &lg;
	bool hasMoleculeEncoding;
	std::map<Vertex, AtomData> nonAtomToPhonyAtom;
	std::map<AtomId, std::string> phonyAtomToStringNoStuff;
	std::map<Edge, std::string> nonBondEdges;
#ifdef MOD_HAVE_OPENBABEL
	mutable lib::Chem::OBMolHandle obMolAll, obMolNoHydrogen;
#endif
	std::shared_ptr<mod::Function<std::string()>> image;
	std::string imageCmd;
};

} // namespace mod::lib::graph::Write

#endif // MOD_LIB_GRAPH_IO_DEPICTIONDATA_HPP