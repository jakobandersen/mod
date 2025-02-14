#include "OBabel.hpp"

#ifdef MOD_HAVE_OPENBABEL

#include <mod/Error.hpp>

#include <mod/lib/Chem/MoleculeUtil.hpp>
#include <mod/lib/Graph/Properties/Stereo.hpp>
#include <mod/lib/IO/GraphWrite.hpp>
#include <mod/lib/IO/IO.hpp>
#include <mod/lib/Stereo/Configuration/Tetrahedral.hpp>

#include <jla_boost/graph/PairToRangeAdaptor.hpp>

// The Open Babel plugin infrastructure needs some symbols to be visible,
// but they all seem to be declared inline in the header files, so we need to compile
// such that they are visible.
// Assuming GCC and Clang the following define seems to do the trick:
#define HAVE_GCC_VISIBILITY 1

#include <openbabel/atom.h>
#include <openbabel/bond.h>
#include <openbabel/builder.h>
#include <openbabel/forcefield.h>
#include <openbabel/mol.h>
#include <openbabel/obconversion.h>
#include <openbabel/op.h>
#include <openbabel/stereo/tetrahedral.h>

#include <map>

namespace mod::lib::Chem {

struct OBMolHandle::Pimpl {
	std::unique_ptr<OpenBabel::OBMol> m;
	std::map<OpenBabel::OBBond *, enum OpenBabel::OBStereo::BondDirection> updown;
	std::map<OpenBabel::OBBond *, OpenBabel::OBStereo::Ref> from;
};

OBMolHandle::OBMolHandle() = default;
OBMolHandle::OBMolHandle(OBMolHandle &&o) = default;
OBMolHandle &OBMolHandle::operator=(OBMolHandle &&o) = default;

OBMolHandle::OBMolHandle(std::unique_ptr<Pimpl> p) : p(std::move(p)) {}

OBMolHandle::~OBMolHandle() = default;

OBMolHandle::operator bool() const {
	return bool(p);
}

void OBMolHandle::setCoordinates(const std::vector<double> &x, const std::vector<double> &y) {
	assert(x.size() == y.size());
	auto &mol = const_cast<OpenBabel::OBMol &> (*p->m); // becuase bah
	mol.BeginModify();
	for(std::size_t i = 0; i != x.size(); ++i) {
		auto *aPtr = mol.GetAtomById(i);
		if(!aPtr) continue;
		aPtr->SetVector(x[i], y[i], 0);
	}
	p->updown.clear();
	p->from.clear();
	TetStereoToWedgeHash(mol, p->updown, p->from);
	mol.EndModify();
}

double OBMolHandle::getEnergy(bool verbose) const {
	auto &mol = const_cast<OpenBabel::OBMol &> (*p->m); // becuase bah
	if(verbose)
		std::cout << "OBMolHandle::getEnergy: '" << mol.GetTitle() << "'"
		          << "\t" << mol.NumAtoms() << " atoms\t" << mol.NumBonds() << " bonds"
		          << std::endl;
	// code originally from GGL

	OpenBabel::OBForceField *pFF = nullptr;

	constexpr int conformers = 25;
	constexpr int geomSteps = 100;

	{ // gen3D stuff
		if(verbose) std::cout << "OBMolHandle::getEnergy:   gen 3D" << std::endl;
		pFF = OpenBabel::OBForceField::FindForceField("MMFF94");
		assert(pFF);
		OpenBabel::OBBuilder builder;
		builder.Build(mol);
		mol.SetDimension(3);

		const bool setupOK = pFF->Setup(mol);
		if(!setupOK) throw FatalError("OBMolHandle::getEnergy: ERROR: Setup of Open Babel forcefield failed.");

		// Since we only want a rough geometry, use distance cutoffs for VDW, Electrostatics
		pFF->EnableCutOff(true);
		pFF->SetVDWCutOff(10.0);
		pFF->SetElectrostaticCutOff(20.0);
		pFF->SetUpdateFrequency(10); // update non-bonded distances

		pFF->SteepestDescent(250, 1.0e-4);
		pFF->WeightedRotorSearch(conformers, geomSteps);
		pFF->ConjugateGradients(250, 1.0e-6);
		pFF->UpdateCoordinates(mol);
	}

	{ // conformer stuff
		if(verbose) std::cout << "OBMolHandle::getEnergy:   conformer" << std::endl;
		pFF = OpenBabel::OBForceField::FindForceField("MMFF94");
		assert(pFF);
		const bool setupOK = pFF->Setup(mol);
		assert(setupOK /*ensure setup is working*/);
		if(!setupOK) MOD_ABORT;
		pFF->Setup(mol);
		pFF->WeightedRotorSearch(conformers, geomSteps);
		pFF->ConjugateGradients(geomSteps); // final cleanup
		pFF->UpdateCoordinates(mol);
	}

	// calculate energy
	if(verbose) std::cout << "OBMolHandle::getEnergy:   energy" << std::endl;
	const double energy = pFF->Energy(false);

	// calculate appropriate unit scaling
	std::string unit = pFF->GetUnit();
	if(unit == "kcal/mol")
		return energy * 4.184; // convert kcal -> kJ
	else
		return energy;
}

void OBMolHandle::print2Dsvg(std::ostream &s) const {
	OpenBabel::OBConversion conv;
	conv.SetOutFormat("svg");
	conv.AddOption("svgwritechemobject"); // hack to make it not output fixed size svgs, is this really the best way?
	//	conv.AddOption("C"); // do not show terminal carbons
	//	conv.AddOption("i"); // put indices on
	conv.AddOption("h");
	conv.Write(p->m.get(), &s);
	//	conv.SetOutFormat("smi");
	//	conv.Write(&mol, &std::cout);
}

double OBMolHandle::getCoordScaling() const {
	// this is an adaptation of the code from Open Babel (src/depict/depict.cpp)
	if(p->m->NumBonds() == 0) return 1;
	auto &obMolNoConst = const_cast<OpenBabel::OBMol &> (*p->m); // TODO: and Open Babel, please get your shit together!
	double sumLengths = 0;
	for(auto bIter = obMolNoConst.BeginBonds(); bIter != obMolNoConst.EndBonds(); ++bIter)
		sumLengths += (*bIter)->GetLength();
	return p->m->NumBonds() / sumLengths; // == 1 / (length / count);
}

bool OBMolHandle::hasAtom(int id) const {
	return p->m->GetAtomById(id) != nullptr;
}

double OBMolHandle::getAtomX(int id) const {
	const auto *a = p->m->GetAtomById(id);
	assert(a);
	return a->GetX();
}

double OBMolHandle::getAtomY(int id) const {
	const auto *a = p->m->GetAtomById(id);
	assert(a);
	return a->GetY();
}

lib::IO::Graph::Write::EdgeFake3DType OBMolHandle::getBondFake3D(int idSrc, int idTar) const {
	// mimic what OpenBabel is doing in openbabel/depict/depict.cpp
	using Type = lib::IO::Graph::Write::EdgeFake3DType;
	auto *aSrc = p->m->GetAtomById(idSrc);
	auto *aTar = p->m->GetAtomById(idTar);
	auto *bond = p->m->GetBond(aSrc, aTar);
	if(!bond) return Type::None;
	const auto iter = p->from.find(bond);
	if(iter == end(p->from)) return Type::None;
	const bool swapped = bond->GetBeginAtom()->GetId() != iter->second;
	const auto iterUpDown = p->updown.find(bond);
	assert(iterUpDown != end(p->updown));
	Type type = Type::None;
	if(iterUpDown->second == OpenBabel::OBStereo::UpBond) type = Type::WedgeSL;
	else if(iterUpDown->second == OpenBabel::OBStereo::DownBond) type = Type::HashSL;
	if(swapped) return lib::IO::Graph::Write::invertEdgeFake3DType(type);
	else return type;
}

namespace {

void generateCoordinates(OpenBabel::OBMol &mol) {
	// this is based on OpenBabel formats/svgformat.cpp and depict/depict.cpp
	OpenBabel::OBConversion conv; // because the constructor apparently takes care of lib loading
	OpenBabel::OBOp *op = OpenBabel::OBOp::FindType("gen2D");
	if(!op) {
		std::string msg = "MØD Error: Could not load Open Babel gen2D operation."
		                  " This can happen if the MØD library or an extension library was loaded with dlopen,"
		                  " but without the RTLD_GLOBAL flag. If you are executing from Python then you can insert\n"
		                  "\timport ctypes\n"
		                  "\timport sys\n"
		                  "\tsys.setdlopenflags(sys.getdlopenflags() | ctypes.RTLD_GLOBAL)\n"
		                  "before importing any modules. If this does not work, please open an issue.";
		throw FatalError(std::move(msg));
	}
	bool res = op->Do(&mol);
	if(!res) MOD_ABORT;
}

template<typename Graph, typename MayCollapse, typename Callback>
OBMolHandle makeOBMolImpl(
		const Graph &g,
		std::function<const AtomData &(typename boost::graph_traits<Graph>::vertex_descriptor)> atomData,
		std::function<BondType(typename boost::graph_traits<Graph>::edge_descriptor)> bondData,
		MayCollapse mayCollapse,
		const bool ignoreDuplicateBonds, const bool withCoordinates, Callback callback) {
	auto mol = std::make_unique<OpenBabel::OBMol>();
	mol->BeginModify();

	// add all visible vertices
	for(const auto v: asRange(vertices(g))) {
		const auto vId = get(boost::vertex_index_t(), g, v);
		if(mayCollapse(v)) continue;
		OpenBabel::OBAtom *atom = mol->NewAtom(vId);
		assert(atomData(v).getAtomId() != AtomIds::Invalid);
		atom->SetAtomicNum(atomData(v).getAtomId());
		atom->SetFormalCharge(atomData(v).getCharge());
		atom->SetType(symbolFromAtomId(atomData(v).getAtomId()));
	}

	// add all edges
	for(const auto e: asRange(edges(g))) {
		const auto vSrc = source(e, g);
		const auto vTar = target(e, g);
		const auto srcId = get(boost::vertex_index_t(), g, vSrc);
		const auto tarId = get(boost::vertex_index_t(), g, vTar);
		if(mayCollapse(vSrc)) continue;
		if(mayCollapse(vTar)) continue;
		OpenBabel::OBAtom *src = mol->GetAtomById(srcId), *tar = mol->GetAtomById(tarId);
		assert(src);
		assert(tar);
		int order;
		switch(bondData(e)) {
		case BondType::Invalid:
		case BondType::Single:
			order = 1;
			break;
		case BondType::Aromatic:
			order = 5;
			break;
		case BondType::Double:
			order = 2;
			break;
		case BondType::Triple:
			order = 3;
			break;
		default:
			std::abort();
		}
		OpenBabel::OBBond *bond = mol->GetBond(src, tar);
		if(!ignoreDuplicateBonds && bond) std::abort();
		if(!bond) {
			bond = mol->NewBond();
			bond->SetBegin(src);
			bond->SetEnd(tar);
			bond->SetBondOrder(order);
			src->AddBond(bond);
			tar->AddBond(bond);
			assert(mol->GetBond(src, tar));
		}
	}
	callback(*mol);
	if(withCoordinates)
		generateCoordinates(*mol);
	auto pimpl = std::make_unique<OBMolHandle::Pimpl>();
	TetStereoToWedgeHash(*mol, pimpl->updown, pimpl->from);
	mol->EndModify();

	pimpl->m = std::move(mol);
	return OBMolHandle(std::move(pimpl));
}

template<typename Graph, typename Stereo>
void convertStereo(const Graph &g,
                   std::function<const AtomData &(typename boost::graph_traits<Graph>::vertex_descriptor)> atomData,
                   std::function<BondType(typename boost::graph_traits<Graph>::edge_descriptor)> bondData,
                   std::function<bool(typename boost::graph_traits<Graph>::vertex_descriptor)> hasImportantStereo,
                   const bool withHydrogen, const Stereo &pStereo, OpenBabel::OBMol &mol) {
	using Emb = lib::Stereo::EmbeddingEdge;
	for(const auto v: asRange(vertices(g))) {
		const auto vId = get(boost::vertex_index_t(), g, v);
		auto *atom = mol.GetAtomById(vId);
		if(!atom) continue;
		const auto &geo = lib::Stereo::getGeometryGraph();
		const auto &conf = *pStereo[v];
		const auto vGeo = conf.getGeometryVertex();
		if(vGeo == geo.tetrahedral) {
			const auto &c = static_cast<const lib::Stereo::Tetrahedral &> (conf);
			if(!c.getFixation().asSimple()) continue;
			auto tetra = std::make_unique<OpenBabel::OBTetrahedralStereo>(&mol);
			OpenBabel::OBTetrahedralStereo::Config config;
			config.center = vId;
			config.winding = OpenBabel::OBStereo::Winding::AntiClockwise;
			config.view = OpenBabel::OBStereo::View::ViewFrom;
			config.specified = true;
			const auto embToRef = [&](const auto &emb) -> unsigned int {
				if(emb.type != Emb::Type::Edge) return OpenBabel::OBStereo::ImplicitRef;
				const auto e = emb.getEdge(v, g);
				const auto vAdj = target(e, g);
				const auto vIdAdj = get(boost::vertex_index_t(), g, vAdj);
				if(!withHydrogen && isCollapsibleHydrogen(vAdj, g, atomData, bondData, hasImportantStereo)) {
					return OpenBabel::OBStereo::ImplicitRef;
				} else {
					return vIdAdj;
				}
			};
			config.from = embToRef(c.begin()[0]);
			for(const auto *embPtr = c.begin() + 1; embPtr != c.end(); ++embPtr) {
				config.refs.push_back(embToRef(*embPtr));
			}
			// OpenBabel will loop infinitely if all refs are implicit
			const auto numImplicit = std::count_if(config.refs.begin(), config.refs.end(), [](auto &ref) {
				return ref == OpenBabel::OBStereo::ImplicitRef;
			});
			switch(numImplicit) {
			case 0:
				break; // Ok
			case 1:
				break; // hmm, does OpenBabel like this?
			case 2:
				continue; // otherwise, OpenBabel seg. faults (GetVector on a nullptr Atom)
			case 3:
				continue; // otherwise, OpenBabel loops infinitely
			}
			//							{ // debug
			//								std::cout << "OBabelTetra(" << config.center << "): " << config.from;
			//								for(const auto &r : config.refs) std::cout << ", " << r;
			//										std::cout << "     " << config.winding;
			//										std::cout << std::endl;
			//								}
			tetra->SetConfig(config);
			mol.SetData(tetra.release());
		} // end if tetrahedral
	} // end foreach vertex
}

} // namespace

OBMolHandle copyOBMol(const OBMolHandle &mol) {
	auto pimpl = std::make_unique<OBMolHandle::Pimpl>();
	pimpl->m = std::make_unique<OpenBabel::OBMol>(*mol.p->m);
	return OBMolHandle(std::move(pimpl));
}

OBMolHandle makeOBMol(const lib::graph::GraphType &g,
                      std::function<const AtomData &(lib::graph::Vertex)> atomData,
                      std::function<BondType(lib::graph::Edge)> bondData,
                      std::function<bool(lib::graph::Vertex)> hasImportantStereo,
                      const bool withHydrogen, const lib::graph::PropStereo *pStereo) {
	const auto mayCollapse = [&](const auto v) {
		if(withHydrogen) return false;
		return isCollapsibleHydrogen(v, g, atomData, bondData, hasImportantStereo);
	};
	auto res = makeOBMolImpl(g, atomData, bondData, mayCollapse, false, true, [&](OpenBabel::OBMol &mol) {
		if(pStereo) convertStereo(g, atomData, bondData, hasImportantStereo, withHydrogen, *pStereo, mol);
	});
	return res;
}

std::tuple<OBMolHandle, OBMolHandle, OBMolHandle> makeOBMol(
		const lib::rule::LabelledRule &lr,
		std::function<const AtomData &(lib::DPO::CombinedRule::CombinedVertex)> cgAtom,
		std::function<BondType(lib::DPO::CombinedRule::CombinedEdge)> cgBond,
		std::function<const AtomData &(lib::DPO::CombinedRule::SideVertex)> leftAtom,
		std::function<BondType(lib::DPO::CombinedRule::SideEdge)> leftBond,
		std::function<const AtomData &(lib::DPO::CombinedRule::SideVertex)> rightAtom,
		std::function<BondType(lib::DPO::CombinedRule::SideEdge)> rightBond,
		std::function<bool(lib::DPO::CombinedRule::CombinedVertex)> mayCollapse,
		const bool withHydrogen) {
	const auto mayCollapseCG = [mayCollapse, withHydrogen](const auto v) {
		if(withHydrogen) return false;
		return mayCollapse(v);
	};
	const auto mayCollapseL = [&mayCollapseCG](const auto vL) {
		// TODO: map to CG
		return mayCollapseCG(vL);
	};
	const auto mayCollapseR = [&mayCollapseCG](const auto vR) {
		// TODO: map to CG
		return mayCollapseCG(vR);
	};
	OBMolHandle obMol = makeOBMolImpl(get_graph(lr), cgAtom, cgBond, mayCollapseCG, true, true,
	                                  [](OpenBabel::OBMol &mol) {});
	const auto &lgLeft = get_labelled_left(lr);
	OBMolHandle obMolLeft = makeOBMolImpl(
			get_graph(lgLeft), leftAtom, leftBond, mayCollapseL, false, false,
			[&lgLeft, &lr, leftAtom, leftBond, &obMol](OpenBabel::OBMol &mol) {
				const auto hasImportantStereoLeft = [&lr](const auto v) {
					const auto &lg = get_labelled_left(lr);
					if(!has_stereo(lg)) return false;
					return !get_stereo(lg)[v]->morphismDynamicOk();
				};
				const auto &g = get_graph(lgLeft);
				if(has_stereo(lr))
					convertStereo(g, leftAtom, leftBond, hasImportantStereoLeft, true,
					              get_stereo(lgLeft), mol);
				for(const auto v: asRange(vertices(g))) {
					const auto vId = get(boost::vertex_index_t(), g, v);
					const auto *aBase = obMol.p->m->GetAtomById(vId);
					if(!aBase) continue;
					auto *aSide = mol.GetAtomById(vId);
					if(!aSide) continue;
					aSide->SetVector(aBase->GetX(), aBase->GetY(), 0);
				}
				mol.SetDimension(2);
			});
	const auto &lgRight = get_labelled_right(lr);
	OBMolHandle obMolRight = makeOBMolImpl(
			get_graph(lgRight), rightAtom, rightBond, mayCollapseR, false, false,
			[&lr, &lgRight, rightAtom, rightBond, &obMol](OpenBabel::OBMol &mol) {
				const auto hasImportantStereoRight = [&lr](const auto v) {
					const auto &lg = get_labelled_right(lr);
					if(!has_stereo(lg)) return false;
					return !get_stereo(lg)[v]->morphismDynamicOk();
				};
				const auto &g = get_graph(lgRight);
				if(has_stereo(lr))
					convertStereo(g, rightAtom, rightBond, hasImportantStereoRight, true,
					              get_stereo(lgRight), mol);
				for(const auto v: asRange(vertices(g))) {
					const auto vId = get(boost::vertex_index_t(), g, v);
					const auto *aBase = obMol.p->m->GetAtomById(vId);
					if(!aBase) continue;
					auto *aSide = mol.GetAtomById(vId);
					if(!aSide) continue;
					aSide->SetVector(aBase->GetX(), aBase->GetY(), 0);
				}
				mol.SetDimension(2);
			});
	return std::make_tuple(std::move(obMol), std::move(obMolLeft), std::move(obMolRight));
}

} // namespace mod::lib::Chem

#endif // MOD_HAVE_OPENBABEL