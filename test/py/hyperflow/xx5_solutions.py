include("xxx_common.py")
include("011_base_var_spec.py")

import math

verbosity = 1

class SolChoice(list):
	pass

def _checkSols(solutions, sols, *, objFunc, doTypeCheck=True):
	sols = sols[:]
	if len(solutions) != len(sols):
		solutions.print()
		post.enableInvokeMake()
		raise Exception(f"Num solutions: {len(solutions)} != {len(sols)}")

	def strFromSol(sol, prefix=""):
		if type(sol) is SolChoice:
			res = "[\n"
			res += "\n".join(strFromSol(s, "  ") for s in sol)
			res += "]\n"
			return res
		else:
			if len(sol) == 0:
				return prefix + "(all 0)\n"
			res = ""
			for varCand, idxCand, val in sol:
				res += prefix + str(varCand[idxCand]) + " = " + str(val) + "\n"
			return res

	usedSols = []
	for i in range(len(solutions)):
		s = solutions[i]

		# perform checks defined by the variable specification
		for module, varTuples in varRegistry.items():
			for typ, name, alsoRelaxed, solCheck in varTuples:
				if not s.model.relaxed or alsoRelaxed:
					try:
						solCheck(s)
					except AssertionError:
						post.enableInvokeMake()
						s.print()
						raise

		def findSol():
			for j in range(len(sols)):
				def solMatches(sol):
					def compVal(var, idx, val):
						for varCand, idxCand, expVal in sol:
							if var.id == varCand.id and idx == idxCand:
								if doTypeCheck and type(expVal) != type(val):
									return False
								if expVal != val:
									return False
								return True
						return val == 0
					def check(var, idxs):
						for idx in idxs:
							res = compVal(var, idx, s.eval(var[idx]))
							if not res:
								return False
						return True
					res = check(inFlow, dg.vertices)
					res = res and check(outFlow, dg.vertices)
					res = res and check(edgeFlow, dg.edges)
					res = res and check(hyperflow.VarSumCustom("userDefined"),
						[v.name for v in s.model.customBoolVariables])
					res = res and check(hyperflow.VarSumCustom("userDefined"),
						[v.name for v in s.model.customIntVariables])
					res = res and check(hyperflow.VarSumCustom("userDefined"),
						[v.name for v in s.model.customFloatVariables])
					if s.model.overallAutocatalysis.isEnabled:
						res = res and check(isOverallAutocata, dg.vertices)
					if s.model.overallCatalysis.isEnabled:
						res = res and check(isOverallCata, dg.vertices)
					return res
				if type(sols[j]) is SolChoice:
					matching = [sol for sol in sols[j] if solMatches(sol)]
					assert len(matching) <= 1
					res = len(matching) == 1
				else:
					res = solMatches(sols[j])
				if res:
					usedSols.append(sols[j])
					del sols[j]
					return True
			return False
		def printCurrentSol():
			msg = ""
			hasNonZero = False
			toPrint = [
				(inFlow, dg.vertices),
				(outFlow, dg.vertices),
				(edge, dg.edges),
				(hyperflow.VarSumCustom("userDefined"),
					[v.name for v in s.model.customBoolVariables]),
				(hyperflow.VarSumCustom("userDefined"),
					[v.name for v in s.model.customIntVariables]),
				(hyperflow.VarSumCustom("userDefined"),
					[v.name for v in s.model.customFloatVariables]),
			]
			if s.model.overallAutocatalysis.isEnabled:
				toPrint.append((isOverallAutocata, dg.vertices))
			if s.model.overallCatalysis.isEnabled:
				toPrint.append((isOverallCata, dg.vertices))
			for var, idxs in toPrint:
				for idx in idxs:
					v = s.eval(var[idx])
					if v != 0:
						hasNonZero = True
						msg += str(var[idx]) + " = " + str(v) + "\n"
			if not hasNonZero:
				msg += "(all 0)\n"
			return msg
		if not findSol():
			solutions.print()
			msg = f"\nUsed expected solutions: {len(usedSols)}\n"
			for sol in usedSols:
				msg += strFromSol(sol) + "\n"
			msg += f"Unused expected solutions: {len(sols)}\n"
			for sol in sols:
				msg += strFromSol(sol) + "\n"
			msg += f"Current flow solution: solution {i}\n"
			msg += printCurrentSol()
			post.enableInvokeMake()
			raise Exception(msg)
		objCand = s.objectiveValue
		objExp = s.eval(objFunc)
		if isinstance(objCand, float):
			comp = lambda a, b: math.isclose(a, b)
		else:
			comp = lambda a, b: a == b
		if not comp(objCand, objExp):
			msg = f"Wrong objective value, {objCand}, expected {objExp}. Solution:\n"
			msg += printCurrentSol()
			raise Exception(msg)


def _checkFlow(f, sols, *, objFunc):
	print("=" * 80)
	print(f"_checkFlow: {f}")
	dumpName = f.dump()
	print(f"  _checkFlow: before load to f2 from CWDPath({dumpName})")
	f2 = hyperflow.Model.load(f.dg, CWDPath(dumpName),
		ilpSolver="not a solver", verbosity=verbosity)
	print("  _checkFlow: after load to f2")
	dumpName2 = f2.dump()

	assert len(f.solutions) == len(f2.solutions)
	print("  _checkFlow: solutions")
	_checkSols(f.solutions, sols, objFunc=objFunc)
	print("  _checkFlow: loaded solutions")
	_checkSols(f2.solutions, sols, objFunc=objFunc)

	print("  _checkFlow: compare orig and dumped models and solutions")
	mod.libpymod._compareModels_only_for_testing(f, f2)
	assert len(f.solutions) == len(f2.solutions)
	for i in range(len(f.solutions)):
		s1 = f.solutions[i]
		s2 = f2.solutions[i]
		def comp(v):
			v1 = s1.eval(v)
			v2 = s2.eval(v)
			if isinstance(v1, float):
				comp = lambda a, b: math.isclose(a, b)
			else:
				comp = lambda a, b: a == b
			if type(v1) != type(v2) or not comp(v1, v2):
				assert False, f"\nvar: {v}\nv1: {type(v1)} {v1}\nv2: {type(v2)} {v2}"
			
		for module, varTuples in varRegistry.items():
			for typ, name, alsoRelaxed, solCheck in varTuples:
				if s1.model.relaxed and not alsoRelaxed: continue
				if typ == 'vertex':
					v = hyperflow.VarSumVertex(name)
					range_ = f.dg.vertices
				elif typ == 'edge':
					v = hyperflow.VarSumEdge(name)
					range_ = f.dg.edges
				elif typ == 'custom':
					v = hyperflow.VarSumCustom(name)
					comp(v)
					continue
				for x in range_:
					if module == "base" and name == "transitInternalFlow":
						if x not in flow.separatedIOInternalTransit:
							continue
					if module == "base" and name == "isBothReverseUsed":
						if not x.inverse:
							continue
					vIdx = v[x]
					comp(vIdx)

	_compareDumps(dumpName, dumpName2)


def checkSolutions(f, sols, *, maxNumSolutions=1, absGap=None, old=True, objFunc=None):
	if objFunc is None:
		objFunc = edgeFlow + inFlow
		if f.overallAutocatalysis.isEnabled:
			objFunc += isOverallAutocata
		if f.overallCatalysis.isEnabled:
			objFunc += isOverallCata
	print("Calculating and checking solutions #######################")
	for sol in sols:
		def checkIdx(s):
			for var, idx, val in s:
				if not idx:
					post.enableInvokeMake()
					assert False, f"Bad idx for '{var}': {idx} (val={val})"
		if type(sol) is SolChoice:
			for s in sol:
				checkIdx(s)
		else:
			checkIdx(sol)
	f.absGap = absGap

	# dump before solving and see if the loaded spec gives the same result
	specDump = f.dump()
	fSpec = hyperflow.Model.load(f.dg, CWDPath(specDump))
	fEnum = hyperflow.Model.load(f.dg, CWDPath(specDump))
	specDump2 = fSpec.dump()
	if False:
		print("specDump")
		print("="*80)
		showDump(specDump)
		print("specDump2")
		print("="*80)
		showDump(specDump2)
	_compareDumps(specDump, specDump2)

	f.findSolutions(
		maxNumSolutions=maxNumSolutions, verbosity=verbosity, ilpVerbosity=verbosity)
	fSpec.findSolutions(
		maxNumSolutions=maxNumSolutions, verbosity=verbosity, ilpVerbosity=verbosity)
	remaining = maxNumSolutions
	while remaining != 0:
		res = fEnum.findSolutions(verbosity=verbosity, ilpVerbosity=verbosity)
		assert len(res) <= 1
		if len(res) == 0:
			break
		assert res[0].id == maxNumSolutions - remaining
		remaining -= 1

	print("#" * 80)
	print("Checking original flow solutions")
	_checkFlow(f, sols, objFunc=objFunc)
	print("#" * 80)
	print("Checking flow solutions from loaded specification")
	_checkFlow(fSpec, sols, objFunc=objFunc)
	print("#" * 80)
	print("Checking individually enumerated flow solutions")
	_checkFlow(fEnum, sols, objFunc=objFunc)

	print("=" * 80)
	_compareFiles(f.implementationView.printEnumerationTree(), fSpec.implementationView.printEnumerationTree())
	f.solutions.list()
