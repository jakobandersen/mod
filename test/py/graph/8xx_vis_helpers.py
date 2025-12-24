
def p(strs, *, h_=None, c_=None, chg_=None, iso_=None, i_=None, sp_=None, sr_=None):
	for s in strs:
		Graph.fromSMILES(s)

	for g in inputGraphs:
		for gr in False, True:
			post.summaryChapter(f"{g.smiles}, Graphviz = {gr}")
			for h in False, True:
				if h_ is not None and h_ != h:
					continue
				for c in False, True:
					if c_ is not None and c_ != c:
						continue
					for chg in False, True:
						if chg_ is not None and chg_ != chg:
							continue
						for iso in False, True:
							if iso_ is not None and iso_ != iso:
								continue
							post.summarySection(f"{g.smiles}, h={h}, c={c}, chg={chg}, iso={iso}")
							for i in False, True:
								if i_ is not None and i_ != i:
									continue
								for sp in False, True:
									if sp_ is not None and sp_ != sp:
										continue
									for sr in False, True:
										if sr_ is not None and sr_ != sr:
											continue
										g.name = f"{g.smiles}, i={i}, sp={sp}, sr={sr}"
										p = GraphPrinter()
										p.withGraphvizCoords = gr
										p.collapseHydrogens = h
										p.simpleCarbons = c
										p.raiseCharges = chg
										p.raiseIsotopes = iso
										p.withIndex = i
										p.withPrettyStereo = sp
										p.withRawStereo = sr
										g.print(p)
