include("xxx_helpers.py")

def check(s1, s2):
	print("#" * 80)
	data1 = "rule [ left [ {} ] right [ {} ] ]".format(s1, s2)
	data2 = "rule [ left [ {} ] right [ {} ] ]".format(s2, s1)
	print("data1:")
	print(data1)
	print("data2:")
	print(data2)
	print("=" * 80)
	print("r1 load")
	print("-" * 80)
	r1 = Rule.fromGMLString(data1)
	print("r1 invert")
	print("-" * 80)
	r1inv = r1.makeInverse()

	print("=" * 80)
	print("r2 load")
	print("-" * 80)
	r2 = Rule.fromGMLString(data2)
	print("r2 invert")
	print("-" * 80)
	r2inv = r2.makeInverse()

	print("=" * 80)
	print("r1inv iso r2")
	print("-" * 80)
	assert r1inv.isomorphism(r2) != 0
	print("r2inv iso r1")
	print("-" * 80)
	assert r2inv.isomorphism(r1) != 0

	commonChecks(r1)
	commonChecks(r2)

# vertices
check('node [ id 0 label "A" ]', '')
check('', 'node [ id 0 label "A" ]')
check('node [ id 0 label "A" ]', 'node [ id 0 label "B" ]')
check('node [ id 0 label "C" ]', 'node [ id 0 label "C" ]')
# in particular to test external ID propagation when none are present:
Rule.fromDFS("[A]>>[B]").makeInverse()
# edges
check('''
	node [ id 0 label "X" ]
	node [ id 1 label "Y" ]
	edge [ source 0 target 1 label "A" ]
''', '''
	node [ id 0 label "X" ]
	node [ id 1 label "Y" ]
''')
check('''
	node [ id 0 label "X" ]
	node [ id 1 label "Y" ]
''', '''
	node [ id 0 label "X" ]
	node [ id 1 label "Y" ]
	edge [ source 0 target 1 label "A" ]
''')
check('''
	node [ id 0 label "X" ]
	node [ id 1 label "Y" ]
	edge [ source 0 target 1 label "A" ]
''', '''
	node [ id 0 label "X" ]
	node [ id 1 label "Y" ]
	edge [ source 0 target 1 label "B" ]
''')
check('''
	node [ id 0 label "X" ]
	node [ id 1 label "Y" ]
	edge [ source 0 target 1 label "C" ]
''', '''
	node [ id 0 label "X" ]
	node [ id 1 label "Y" ]
	edge [ source 0 target 1 label "C" ]
''')

# TODO: stereo tests

# constraints
data = """rule [
	left [ node [ id 0 label "X" ] ]
	constrainAdj [ id 0 count 0 op "=" ]
]"""
r = Rule.fromGMLString(data)
fail(lambda: Rule.fromGMLString(data, invert=True), err=InputError,
	pattern="has matching constraints and can not be reversed. Use Rule::ignoreConstraintsDuringInversion == true to strip constraints.")
