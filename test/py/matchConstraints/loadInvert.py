rStr = """rule [
	context [
		node [ id 0 label "s" ]
	]
	constrainAdj [
		id 0 op "=" count 0
	]
]"""
r = Rule.fromGMLString(rStr)
try:
	r.makeInverse()
	assert False
except LogicError:
	pass
try:
	Rule.fromGMLString(rStr, invert=True)
	assert False
except InputError:
	pass
config.rule.ignoreConstraintsDuringInversion = True
r.makeInverse()
Rule.fromGMLString(rStr, invert=True)
