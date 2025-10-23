include("common.py")

lr = Rule.fromGMLString("""rule [
	left [
		node [ id 0 stereo "any" ]
	]
	context [
		node [ id 0 label "Z" ]
	]
	right [
		node [ id 0 stereo "any" ]
	]
]""")
c = Rule.fromGMLString("""rule [
	context [
		node [ id 0 label "Z" stereo "any" ]
	]
]""")
for a in inputRules: a.print()
assert not c.isomorphism(lr, labelSettings=isoLabelSettings)
assert lr.isomorphism(c, labelSettings=specLabelSettings)
