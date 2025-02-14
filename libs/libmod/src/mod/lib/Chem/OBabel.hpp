#ifndef MOD_LIB_CHEM_OBABEL_HPP
#define MOD_LIB_CHEM_OBABEL_HPP

#include <mod/BuildConfig.hpp>
#include <mod/lib/Graph/GraphDecl.hpp>

#ifndef MOD_HAVE_OPENBABEL
#include <mod/lib/IO/IO.hpp>
#else

#include <mod/lib/Rule/LabelledRule.hpp>

#include <iosfwd>

#endif

namespace mod {
struct AtomData;
enum class BondType;
} // namespace mod
namespace mod::lib::IO::Graph::Write {
enum struct EdgeFake3DType;
} // namespace mod::lib::IO::Graph::Write
namespace mod::lib::graph {
struct PropStereo;
struct Graph;
} // namespace mod::lib::graph
namespace mod::lib::rule {
struct PropStereo;
} // namespace mod::lib::rule
namespace mod::lib::Chem {

#ifndef MOD_HAVE_OPENBABEL
#define MOD_NO_OPENBABEL_ERROR_STR                        \
	std::string("Call to '") + __func__ + "' failed.\n"    \
	+ "Open Babel features are not available. Rebuild with Open Babel enabled."
#else

struct OBMolHandle {
	struct Pimpl;
public:
	OBMolHandle();
	OBMolHandle(OBMolHandle &&);
	OBMolHandle &operator=(OBMolHandle &&);
	OBMolHandle(std::unique_ptr<Pimpl> p);
	~OBMolHandle();
	explicit operator bool() const;
	void setCoordinates(const std::vector<double> &x, const std::vector<double> &y);
public:
	double getEnergy(bool verbose) const;
	void print2Dsvg(std::ostream &s) const;
public:
	double getCoordScaling() const;
	bool hasAtom(int id) const;
	double getAtomX(int id) const;
	double getAtomY(int id) const;
	lib::IO::Graph::Write::EdgeFake3DType getBondFake3D(int idSrc, int idTar) const;
public:
	std::unique_ptr<Pimpl> p;
};

OBMolHandle copyOBMol(const OBMolHandle &mol);
OBMolHandle makeOBMol(const lib::graph::GraphType &g,
                      std::function<const AtomData &(lib::graph::Vertex)> atomData,
                      std::function<BondType(lib::graph::Edge)> bondData,
                      std::function<bool(lib::graph::Vertex)> hasImportantStereo,
                      bool withHydrogen, const lib::graph::PropStereo *pStereo);
std::tuple<OBMolHandle, OBMolHandle, OBMolHandle> makeOBMol(
		const lib::rule::LabelledRule &lr,
		std::function<const AtomData &(lib::DPO::CombinedRule::CombinedVertex)> cgAtom,
		std::function<BondType(lib::DPO::CombinedRule::CombinedEdge)> cgBond,
		std::function<const AtomData &(lib::DPO::CombinedRule::SideVertex)> leftAtom,
		std::function<BondType(lib::DPO::CombinedRule::SideEdge)> leftBond,
		std::function<const AtomData &(lib::DPO::CombinedRule::SideVertex)> rightAtom,
		std::function<BondType(lib::DPO::CombinedRule::SideEdge)> rightBond,
		std::function<bool(lib::DPO::CombinedRule::CombinedVertex)> mayCollapse,
		const bool withHydrogen);
#endif

} // namespace mod::lib::Chem

#endif // MOD_LIB_CHEM_OBABEL_HPP