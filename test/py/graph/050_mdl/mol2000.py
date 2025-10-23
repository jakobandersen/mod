post.disableInvokeMake()

def fail(s, options=MDLOptions()):
	try:
		Graph.fromMOLString(s, options=options)
		assert False
	except InputError as e:
		print(e)

def sep(name):
	print("\n%s\n" % name + "="*80)


sep("Header")
fail("")
fail("$MDL\n")
fail("$$$$")
fail("$RXN")
fail("$RDFILE")

fail("\n")

fail("\n\n")

fail("\n\n\n")

sep("Counts")
fail("\n\n\n\n")
fail("\n\n\n  1  0  0  0  0  0  0  0  0  0999 a2000")

fail("\n\n\n     0  0  0  0  0  0  0  0  0999 V2000")
fail("\n\n\n  1     0  0  0  0  0  0  0  0999 V2000")
fail("\n\n\n1 1  0  0  0  0  0  0  0  0  0999 V2000")
fail("\n\n\n  11 0  0  0  0  0  0  0  0  0999 V2000")

sep("Atoms")
fail("""\n\n\n  2  1  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000     0  0  0  0  0  0  0  0  0  0  0  0
""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 LP  0  0  0  0  0  0  0  0  0  0  0  0
""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 L   0  0  0  0  0  0  0  0  0  0  0  0
""")

fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   1  0  0  0  0  0  0  0  0  0  0  0
""")

fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  -  0  0  0  0  0  0  0  0  0  0
""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  4  0  0  0  0  0  0  0  0  0  0
""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  8  0  0  0  0  0  0  0  0  0  0
""")

fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0abc  0  0  0  0  0  0
""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0 16  0  0  0  0  0  0
""")


sep("Charge 4")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  4  0  0  0  0  0  0  0  0  0  0
""")
o = MDLOptions()
o.onV2000Charge4 = Action.Warn
Graph.fromMOLString("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  4  0  0  0  0  0  0  0  0  0  0
M  END""", options=o)
o.onV2000Charge4 = Action.Ignore
Graph.fromMOLString("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  4  0  0  0  0  0  0  0  0  0  0
M  END""", options=o)

sep("Abstract ISO")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 *   1  0  0  0  0  0  0  0  0  0  0  0
M  END""")

o = MDLOptions()
o.onImplicitValenceOnAbstract = Action.Warn
o.onV2000AbstractISO = Action.Warn
Graph.fromMOLString("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 *   1  0  0  0  0  0  0  0  0  0  0  0
M  END""", options=o)
o.onV2000AbstractISO = Action.Ignore
Graph.fromMOLString("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 *   1  0  0  0  0  0  0  0  0  0  0  0
M  END""", options=o)


sep("Bonds")
fail("""\n\n\n  2  1  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
""")

fail("""\n\n\n  2  1  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
     2  1  0  0  0  0
""")
fail("""\n\n\n  2  1  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  0  2  1  0  0  0  0
""")
fail("""\n\n\n  2  1  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  3  2  1  0  0  0  0
""")

fail("""\n\n\n  2  1  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1     1  0  0  0  0
""")
fail("""\n\n\n  2  1  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  0  1  0  0  0  0
""")
fail("""\n\n\n  2  1  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  3  1  0  0  0  0
""")

fail("""\n\n\n  2  1  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2     0  0  0  0
""")
fail("""\n\n\n  2  1  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  0  0  0  0  0
""")
fail("""\n\n\n  2  1  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  9  0  0  0  0
""")

fail("""\n\n\n  2  1  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  5  0  0  0  0
""")
o = MDLOptions()
o.onUnsupportedQueryBondType = Action.Warn
Graph.fromMOLString("""\n\n\n  2  1  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  5  0  0  0  0
M  END
""", options=o)
o.onUnsupportedQueryBondType = Action.Ignore
Graph.fromMOLString("""\n\n\n  2  1  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  5  0  0  0  0
M  END
""", options=o).isomorphism(Graph.fromDFS("C([H])([H])([H]){_Q_1_2_5}C([H])([H])([H])")) == 1


sep("Properties")
fail("""\n\n\n  2  1  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
blah
""")
fail("""\n\n\n  2  1  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
M  CHGxx
""")
fail("""\n\n\n  2  1  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
M  CHG   
""")
fail("""\n\n\n  2  1  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
M  CHG  112345678x
""")
fail("""\n\n\n  2  1  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
M  CHG  1    xxxx
""")
fail("""\n\n\n  2  1  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
M  CHG  1   0xxxx
""")
fail("""\n\n\n  2  1  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
M  CHG  1   3xxxx
""")
fail("""\n\n\n  2  1  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
M  CHG  1   1    
""")
fail("""\n\n\n  2  1  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
M  CHG  1   1   -
""")

sep("End")
fail("""\n\n\n  2  1  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
""")
fail("""\n\n\n  2  1  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
M  END
a
""")

sep("Missing M  End")
fail("""\n\n\n  2  1  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
""")


sep("Unhandled property")
def makeInput(p):
	return """\n\n\n  2  1  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
{}
M  END""".format(p)

o = MDLOptions()
a = Graph.fromMOLString(makeInput("blah"), options=o)
assert len(a.loadingWarnings) == 1

o.fullyIgnoreV2000UnhandledKnownProperty = True
a = Graph.fromMOLString(makeInput("blah"), options=o)
assert len(a.loadingWarnings) == 1
a = Graph.fromMOLString(makeInput("M  REG 123456489"), options=o)
assert len(a.loadingWarnings) == 0

o.onV2000UnhandledProperty = Action.Error
a = Graph.fromMOLString(makeInput("M  REG 123456489"), options=o)
assert len(a.loadingWarnings) == 0


sep("Implicit valence of abstract atom")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 *   0  0  0  0  0  0  0  0  0  0  0  0
M  END
""")
o = MDLOptions()
o.onImplicitValenceOnAbstract = Action.Ignore
Graph.fromMOLString("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 *   0  0  0  0  0  0  0  0  0  0  0  0
M  END
""", options=o)
o.onImplicitValenceOnAbstract = Action.Warn
Graph.fromMOLString("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 *   0  0  0  0  0  0  0  0  0  0  0  0
M  END
""", options=o)


for rad in 1, 3, 4, 5, 6:
	sep("RAD {}".format(rad))
	data = """\n\n\n  1  0  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
M  RAD  1   1   {}
M  END""".format(rad)
	fail(data)
	o = MDLOptions()
	setattr(o, "onRAD{}".format(rad), Action.Ignore)
	Graph.fromMOLString(data, options=o)
	setattr(o, "onRAD{}".format(rad), Action.Warn)
	Graph.fromMOLString(data, options=o)


sep("Atom alias")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
A  
M  END
""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
A    1 
M  END
""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
A    x
M  END
""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
A    0
M  END
""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
A    2
M  END
""")
fail("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
A    1
""")
assert Graph.fromMOLString("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
A    1
$strangeLabel
M  END
""").isomorphism(Graph.fromDFS("[$strangeLabel]")) == 1
o = MDLOptions()
o.applyV2000AtomAliases = False
assert Graph.fromMOLString("""\n\n\n  1  0  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
A    1
$strangeLabel
M  END
""", options=o).isomorphism(Graph.fromSMILES("C")) == 1


sep("Parallel Edges")
fail("""\n\n\n  2  2  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  2  1  1  0  0  0  0
M  END""")


sep("Empty")
fail("""\n\n\n  0  0  0  0  0  0  0  0  0  0999 V2000
M  END""")


sep("Pattern Atoms")
for s, opt in (('ISO', 'onPatternIsotope'), ('CHG', 'onPatternCharge'), ('RAD', 'onPatternRadical')):
	data = """\n\n\n  1  0  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 *   0  0  0  0  0  0  0  0  0  0  0  0
M  {}  1   1   2
M  END""".format(s)
	fail(data)
	o = MDLOptions()
	o.onImplicitValenceOnAbstract = Action.Ignore
	setattr(o, opt, Action.Warn)
	Graph.fromMOLString(data, options=o)
	setattr(o, opt, Action.Ignore)
