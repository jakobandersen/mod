post.disableInvokeMake()

def fail(s, options=MDLOptions()):
	try:
		Graph.fromMOLString(s, options=options)
		assert False
	except InputError as e:
		print(e)

def sep(name):
	print("\n%s\n" % name + "="*80)


sep("CTAB")
fail("\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 """)
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30  BEGIN CTAB""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN  CTAB""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAx""")

sep("CTAB counts")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 blah""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 1 2 3 4""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 1a 2 3 4 5""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 1 2a 3 4 5""")

sep("Atom block")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 1 2 3 4 5""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 1 2 3 4 5
M  V30 BEGIn ATOM""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 1 2 3 4 5
M  V30 BEGIN ATOm""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 0 0 0 0 0
M  V30 BEGIN ATOM""")

fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 1 0 0 0 0
M  V30 BEGIN ATOM
M  V30 0 0 0 0 0""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 1 0 0 0 0
M  V30 BEGIN ATOM
M  V30 x 0 0 0 0 0""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 1 0 0 0 0
M  V30 BEGIN ATOM
M  V30 0 0 0 0 0 0""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 1 0 0 0 0
M  V30 BEGIN ATOM
M  V30 1 "blah" 0 0 0 0""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 1 0 0 0 0
M  V30 BEGIN ATOM
M  V30 1 blah 0 0 0 0""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 1 0 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 x""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 2 0 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0
M  V30 1 C 0 0 0 0""")

sep("Atom optional arguments")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 1 0 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0 blah""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 1 0 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0 blah""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 1 0 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0 blah=""")

fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 1 0 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0 CHG=""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 1 0 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0 CHG=x""")


sep("Bond block")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 0 0 0 0 0
M  V30 BEGIN ATOM
M  V30 END ATOM""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 0 0 0 0 0
M  V30 BEGIN ATOM
M  V30 END ATOM
M  V30 BEGIN BONd""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 0 0 0 0 0
M  V30 BEGIN ATOM
M  V30 END ATOM
M  V30 BEGIN BOND""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 0 0 0 0 0
M  V30 BEGIN ATOM
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 END BONd""")

fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 0 1 0 0 0
M  V30 BEGIN ATOM
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 0 0 0""")

fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 0 1 0 0 0
M  V30 BEGIN ATOM
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 x 0 0 0""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 0 1 0 0 0
M  V30 BEGIN ATOM
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 0 0 0 0""")

fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 0 1 0 0 0
M  V30 BEGIN ATOM
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 0 x 0 0""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 0 1 0 0 0
M  V30 BEGIN ATOM
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 0 0 0""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 0 1 0 0 0
M  V30 BEGIN ATOM
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 5 0 0""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 0 1 0 0 0
M  V30 BEGIN ATOM
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 6 0 0""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 0 1 0 0 0
M  V30 BEGIN ATOM
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 7 0 0""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 0 1 0 0 0
M  V30 BEGIN ATOM
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 9 0 0""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 0 1 0 0 0
M  V30 BEGIN ATOM
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 10 0 0""")

fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 0 1 0 0 0
M  V30 BEGIN ATOM
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 x 0""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 2 1 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0
M  V30 2 C 0 0 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 0 0""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 2 1 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0
M  V30 2 C 0 0 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 3 0""")

fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 2 1 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0
M  V30 2 C 0 0 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 1 x""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 2 1 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0
M  V30 2 C 0 0 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 1 0""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 2 1 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0
M  V30 2 C 0 0 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 1 3""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 2 1 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0
M  V30 2 C 0 0 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 1 1""")

fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 3 2 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0
M  V30 2 C 0 0 0 0
M  V30 3 C 0 0 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 1 2
M  V30 1 1 2 3""")

fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 3 2 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0
M  V30 2 C 0 0 0 0
M  V30 3 C 0 0 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 1 2
M  V30 2 1 2 3""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 3 2 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0
M  V30 2 C 0 0 0 0
M  V30 3 C 0 0 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 1 2
M  V30 2 1 2 3
M  V30 END BONd""")

fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 3 2 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0
M  V30 2 C 0 0 0 0
M  V30 3 C 0 0 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 5 1 2
M  V30 END BOND
M  V30 END CTAB""")
o = MDLOptions()
o.onUnsupportedQueryBondType = Action.Warn
Graph.fromMOLString("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 2 1 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0
M  V30 2 C 0 0 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 5 1 2
M  V30 END BOND
M  V30 END CTAB
M  END""", options=o)
o.onUnsupportedQueryBondType = Action.Ignore
Graph.fromMOLString("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 2 1 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0
M  V30 2 C 0 0 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 5 1 2
M  V30 END BOND
M  V30 END CTAB
M  END""", options=o).isomorphism(Graph.fromDFS("C([H])([H])([H]){_Q_1_2_5}C([H])([H])([H])")) == 1

sep("CTAB end")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 3 2 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0
M  V30 2 C 0 0 0 0
M  V30 3 C 0 0 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 1 2
M  V30 2 1 2 3
M  V30 END BOND""")


sep("End")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 3 2 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0
M  V30 2 C 0 0 0 0
M  V30 3 C 0 0 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 1 2
M  V30 2 1 2 3
M  V30 END BOND
M  V30 END CTAB""") # TODO: may be allowed later
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 3 2 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0
M  V30 2 C 0 0 0 0
M  V30 3 C 0 0 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 1 2
M  V30 2 1 2 3
M  V30 END BOND
M  V30 END CTAB
M  ENd""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 3 2 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0
M  V30 2 C 0 0 0 0
M  V30 3 C 0 0 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 1 2
M  V30 2 1 2 3
M  V30 END BOND
M  V30 END CTAB
M  END
x""")

sep("Implicit valence of abstract atom")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 1 0 0 0 0
M  V30 BEGIN ATOM
M  V30 1 * 0 0 0 0
M  V30 END ATOM
M  V30 END CTAB
M  END""")
o = MDLOptions()
o.onImplicitValenceOnAbstract = Action.Ignore
Graph.fromMOLString("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 1 0 0 0 0
M  V30 BEGIN ATOM
M  V30 1 * 0 0 0 0
M  V30 END ATOM
M  V30 END CTAB
M  END""", options=o)
o.onImplicitValenceOnAbstract = Action.Warn
Graph.fromMOLString("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 1 0 0 0 0
M  V30 BEGIN ATOM
M  V30 1 * 0 0 0 0
M  V30 END ATOM
M  V30 END CTAB
M  END""", options=o)


for rad in 1, 3, 4, 5, 6:
	sep("RAD {}".format(rad))
	data = """\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 1 0 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0 RAD={}
M  V30 END ATOM
M  V30 END CTAB
M  END""".format(rad)
	fail(data)
	o = MDLOptions()
	setattr(o, "onRAD{}".format(rad), Action.Ignore)
	Graph.fromMOLString(data, options=o)
	setattr(o, "onRAD{}".format(rad), Action.Warn)
	Graph.fromMOLString(data, options=o)


sep("Parallel Edges")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 2 2 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0
M  V30 2 C 0 0 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 1 2
M  V30 2 1 2 1
M  V30 END BOND
M  V30 END CTAB
M  END""")


sep("Pattern Atoms")
for s, opt in (('MASS', 'onPatternIsotope'), ('CHG', 'onPatternCharge'), ('RAD', 'onPatternRadical')):
	data = """\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 1 0 0 0 0
M  V30 BEGIN ATOM
M  V30 1 * 0 0 0 0 {}=2
M  V30 END ATOM
M  V30 END CTAB
M  END""".format(s)
	fail(data)
	o = MDLOptions()
	o.onImplicitValenceOnAbstract = Action.Ignore
	setattr(o, opt, Action.Warn)
	Graph.fromMOLString(data, options=o)
	setattr(o, opt, Action.Ignore)


sep("Posval vs. keyword val")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 1 0 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0 CHG=1 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 END BOND
M  V30 END CTAB
M  END""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 1 0 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0 CHG=1 0 RAD=2
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 END BOND
M  V30 END CTAB
M  END""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 1 0 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0 CHG=  
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 END BOND
M  V30 END CTAB
M  END""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 1 0 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0 CHG=
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 END BOND
M  V30 END CTAB
M  END""")

fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 1 0 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0 RGROUPS=(1 1
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 END BOND
M  V30 END CTAB
M  END""")

o = MDLOptions()
assert o.onV3000UnhandledAtomProperty == Action.Warn
Graph.fromMOLString("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 1 0 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0 RGROUPS=(1 1) RGROUPS=(2 1 1)
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 END BOND
M  V30 END CTAB
M  END""")
o.onV3000UnhandledAtomProperty = Action.Error
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 1 0 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0 RGROUPS=(1 1) RGROUPS=(2 1 1)
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 END BOND
M  V30 END CTAB
M  END""", options=o)
o.onV3000UnhandledAtomProperty = Action.Ignore
Graph.fromMOLString("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 1 0 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0 RGROUPS=(1 1) RGROUPS=(2 1 1)
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 END BOND
M  V30 END CTAB
M  END""", options=o)


sep("String args")
o = MDLOptions()
o.onV3000UnhandledAtomProperty = Action.Ignore
Graph.fromMOLString("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 1 0 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0 Something="hello ( ) ) = world"
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 END BOND
M  V30 END CTAB
M  END""", options=o)
Graph.fromMOLString("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 1 0 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0 SomethingElse="hello "" ( ) ) = world"
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 END BOND
M  V30 END CTAB
M  END""", options=o)


sep("Multiline")
Graph.fromMOLString("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 1 0 0 0 0
M  V30 BEGIN ATOM
M  V30 1-
M  V30 -
M  V30  C 0 -
M  V30 0 0 0 CHG=2 -
M  V30 RAD=2 RGROUPS=(-
M  V30 1 1)
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 END BOND
M  V30 END CTAB
M  END""", options=o)


sep("Empty")
fail("""\n\n\n  0  0  0  0  0  0  0  0  0  0999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 0 0 0 0 0
M  V30 BEGIN ATOM
M  V30 END ATOM
M  V30 END CTAB
M  END""")
