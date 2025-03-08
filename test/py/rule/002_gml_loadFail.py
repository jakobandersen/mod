include("xxx_helpers.py")

# for Boost < 1.77 it's column 12, for 1.77 it's 13
gmlFail('blah', "Parsing failed at 1:")

gmlFail('labelType "blah"', "Unknown labelType 'blah'.")

# Vertex properties
# ============================================================================
gmlFail('''
	left    [ node [ id 0 label "A" ] ]
	context [ node [ id 0 label "A" ] ]
''', "Vertex 0 has a label both in 'context' and 'left'.")
gmlFail('''
	right   [ node [ id 0 label "A" ] ]
	context [ node [ id 0 label "A" ] ]
''', "Vertex 0 has a label both in 'context' and 'right'.")

gmlFail('''
	left    [ node [ id 0 label "B" ] ]
	context [ node [ id 0 label "A" ] ]
	right   [ node [ id 0 label "A" ] ]
''', "but the labels in 'left' and 'context' differ ('B' and 'A').")
gmlFail('''
	left    [ node [ id 0 label "A" ] ]
	context [ node [ id 0 label "B" ] ]
	right   [ node [ id 0 label "A" ] ]
''', "but the labels in 'left' and 'context' differ ('A' and 'B').")
gmlFail('''
	left    [ node [ id 0 label "A" ] ]
	context [ node [ id 0 label "A" ] ]
	right   [ node [ id 0 label "B" ] ]
''', "but the labels in 'right' and 'context' differ ('B' and 'A').")
r = Rule.fromGMLString('''rule [
	left    [ node [ id 0 label "A" ] ]
	context [ node [ id 0 label "A" ] ]
	right   [ node [ id 0 label "A" ] ]
]''')
assert r.getVertexFromExternalId(0).left.stringLabel == "A"
assert r.getVertexFromExternalId(0).right.stringLabel == "A"

gmlFail('left  [ node [ id 0 ] ]', "Vertex 0 is in L, but has no label.")
gmlFail('right [ node [ id 0 ] ]', "Vertex 0 is in R, but has no label.")

# Edge properties
# ============================================================================
gmlFail('''
	left [    edge [ source 0 target 1 label "-" ] ]
	context [ edge [ source 0 target 1 label "-" ]
		node [ id 0 label "A" ]
		node [ id 1 label "B" ]
	]''', "Edge (0, 1) has a label both in 'context' and 'left'.")
gmlFail('''
	right [   edge [ source 0 target 1 label "-" ] ]
	context [ edge [ source 0 target 1 label "-" ]
		node [ id 0 label "A" ]
		node [ id 1 label "B" ]
	]''', "Edge (0, 1) has a label both in 'context' and 'right'.")

gmlFail('''
	left [    edge [ source 0 target 1 label "=" ] ]
	context [ edge [ source 0 target 1 label "-" ]
		node [ id 0 label "A" ]
		node [ id 1 label "B" ]
	]
	right [   edge [ source 0 target 1 label "-" ] ]
''', "but the labels in 'left' and 'context' differ ('=' and '-').")
gmlFail('''
	left [    edge [ source 0 target 1 label "-" ] ]
	context [ edge [ source 0 target 1 label "=" ]
		node [ id 0 label "A" ]
		node [ id 1 label "B" ]
	]
	right [   edge [ source 0 target 1 label "-" ] ]
''', "but the labels in 'left' and 'context' differ ('-' and '=').")
gmlFail('''
	left [    edge [ source 0 target 1 label "-" ] ]
	context [ edge [ source 0 target 1 label "-" ]
		node [ id 0 label "A" ]
		node [ id 1 label "B" ]
	]
	right [   edge [ source 0 target 1 label "=" ] ]
''', "but the labels in 'right' and 'context' differ ('=' and '-').")
r = Rule.fromGMLString('''rule [
	left [    edge [ source 0 target 1 label "-" ] ]
	context [ edge [ source 0 target 1 label "-" ]
		node [ id 0 label "A" ]
		node [ id 1 label "B" ]
	]
	right [   edge [ source 0 target 1 label "-" ] ]
]''')

gmlFail('''
	left [ edge [ source 0 target 1 ] ]
	context [ node [ id 0 label "A" ] node [ id 1 label "B" ] ]''',
	"Edge (0, 1) is in L, but has no label.")
gmlFail('''
	right [ edge [ source 0 target 1 ] ]
	context [ node [ id 0 label "A" ] node [ id 1 label "B" ] ]''',
	"Edge (0, 1) is in R, but has no label.")
