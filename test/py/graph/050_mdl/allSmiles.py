post.disableInvokeMake()

from openbabel import pybel
from openbabel import openbabel as ob

include("../030_smiles/mass/loadGraphs.py")

def loadSmiles(s):
	mol = pybel.readstring("smi", s)
	mol.addh()
	return mol

def loadMol(s):
	mol = pybel.readstring("mol", s)
	mol.addh()
	return mol

def bonds(mol):
	for b in ob.OBMolBondIter(mol.OBMol):
		yield b

def gmlFromOb(mol):
	gml = "graph [\n"
	for a in mol.atoms:
		aid = AtomId(a.atomicnum)
		iso = Isotope(a.isotope) if a.isotope != 0 else Isotope()
		c = Charge(a.formalcharge)
		spinMult = a.spin
		if spinMult == 0:
			rad = False
		elif spinMult == 2:
			rad = True
		else:
			rad = False
			print("Warning: non-zero spin-multiplicity,", spinMult)
		ad = AtomData(aid, iso, c, rad)
		gml += "\tnode [ id %d label \"%s\" ]\n" % (a.idx, str(ad))
	for b in bonds(mol):
		src = b.GetBeginAtom().GetIdx()
		tar = b.GetEndAtom().GetIdx()
		b = b.GetBondOrder()
		if b == 1:
			bt = BondType.Single
		elif b == 2:
			bt = BondType.Double
		elif b == 3:
			bt = BondType.Triple
		else:
			print(bt)
			assert False
		gml += "\tedge [ source %d target %d label \"%s\" ]\n" % (src, tar, str(bt))
	gml += "]\n"
	return gml


gs = inputGraphs[:]
count = 0
for g in gs:
	count += 1
	s = g.smiles
	try:
		Graph.fromSMILES(s, allowAbstract=False)
	except InputError as e:
		if "SMILES string has abstract vertex label" in str(e):
			continue
		else:
			raise
	# Open Babel may mangle the molecule as it gets loaded,
	# so the base for comparison is the loaded OBMol.
	obInput = loadSmiles(g.smiles)
	# Writing the MOL and reading it can yield a different molecule.
	# We just want to test MOL parsing,
	# so let's do that round-trip in OB as well.
	# The OB V3000 read/write does not handle the valence field.
	obInput = loadMol(obInput.write("mol"))
	obInput3 = loadMol(obInput.write("mol", opt={"3": None}))
	molInput = obInput.write("mol")
	mol3Input = obInput3.write("mol", opt={"3": None})
	gFromInput = graphGMLString(gmlFromOb(obInput))
	gFromInput3 = graphGMLString(gmlFromOb(obInput3))

	# actual check
	try:
		gFromMol = Graph.fromMOLString(molInput)
	except:
		print(molInput)
		raise
	if gFromInput.isomorphism(gFromMol) == 0:
		print("gFromInput:", gFromInput.smiles)
		print("gFromMol:  ", gFromMol.smiles)
		gFromInput.print()
		gFromMol.print()
		assert False

	# actual check
	try:
		gFromMol3 = Graph.fromMOLString(mol3Input)
	except:
		print(mol3Input)
		raise
	if gFromInput3.isomorphism(gFromMol3) == 0:
		print("gFromInput3:", gFromInput.smiles)
		print("gFromMol3:  ", gFromMol3.smiles)
		post.enableInvokeMake()
		gFromInput.print()
		gFromMol3.print()
		assert False
