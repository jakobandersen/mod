include("xxx_helpers.py")

data = """rule [
left  [ node [ id 0 label "L" ] ]
right [ node [ id 0 label "R" ] ]
]"""
fGML = 'myRule.gml'
with open("myRule.gml", "w") as f:
	f.write(data)

inputRules[:] = []
r1 = Rule.fromGMLString(data)
assert inputRules == [r1]
r2 = Rule.fromGMLString(data)
assert inputRules == [r1, r2]
Rule.fromGMLString(data, add=False)
assert inputRules == [r1, r2]

assert Rule.fromGMLString == ruleGMLString

inputRules[:] = []
r1 = Rule.fromGMLFile(CWDPath(fGML))
assert inputRules == [r1]
r2 = Rule.fromGMLFile(CWDPath(fGML))
assert inputRules == [r1, r2]
Rule.fromGMLFile(CWDPath(fGML), add=False)
assert inputRules == [r1, r2]
assert Rule.fromGMLFile == ruleGML

fail(lambda: Rule.fromGMLFile("doesNotExist.gml"),
	"Could not open rule GML file ", err=InputError, isSubstring=True)

p = """rule [
	context [
		node [ id 0 label "C" stereo "tetrahedral" ]
		node [ id 1 label "H" ] edge [ source 0 target 1 label "=" ]
		node [ id 2 label "H" ] edge [ source 0 target 2 label "-" ]
		node [ id 3 label "H" ] edge [ source 0 target 3 label "-" ]
		node [ id 4 label "H" ] edge [ source 0 target 4 label "-" ]
	]
]"""
print("GML: with stereo warnings")
Rule.fromGMLString(p)
print("GML: without stereo warnings")
Rule.fromGMLString(p, printStereoWarnings=False)
print("GML: end of stereo warnings")
