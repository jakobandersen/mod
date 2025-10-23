include("1xx_execute_helpers.py")

gO = Graph.fromSMILES('[O]', "gO")
gC = Graph.fromSMILES('[C]', "gC")
gN = Graph.fromSMILES('[N]', "gN")
gOC = Graph.fromSMILES('[O][C]', "gOC")

rConvertOC = Rule.fromGMLString("""rule [
	ruleID "Convert O -> C"
	left  [ node [ id 0 label "O" ] ]
	right [ node [ id 0 label "C" ] ]
]""")

rConnectOC = Rule.fromGMLString("""rule [
	ruleID "Connect O C"
	context [
		node [ id 0 label "O" ]
		node [ id 1 label "C" ]
	]
	right [
		edge [ source 0 target 1 label "-" ]
	]
]""")

print("Subset, not universe")
print("="*80)
exeStrat(addSubset(gO) >> rConvertOC, [gC], [gO, gC],
	graphDatabase=inputGraphs, verbosity=10)
exeStrat(addUniverse(gO) >> rConvertOC, [], [gO],
	graphDatabase=inputGraphs, verbosity=10)

print("Multiple, subset")
print("="*80)
exeStrat(addSubset(gO, gC) >> rConnectOC, [gOC], [gO, gC, gOC],
	graphDatabase=inputGraphs, verbosity=10)

print("Multiple, 1 subset, 1 universe")
print("="*80)
exeStrat(addSubset(gO) >> addUniverse(gC) >> rConnectOC, [gOC], [gO, gC, gOC],
	graphDatabase=inputGraphs, verbosity=10)

print("Empty result")
rRemove = Rule.fromGMLString('rule [ left [ node [ id 0 label "C" ] ] ]')
exeStrat(addSubset(gC) >> rRemove, [], [gC], verbosity=10)
