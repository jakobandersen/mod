post.disableInvokeMake()

def fail(f, pattern, err=LogicError, isSubstring=False):
	try:
		f()
		assert False, "Expected an {} exception.".format(err)
	except err as e:
		if isSubstring:
			res = pattern in str(e)
		else:
			res = str(e).endswith(pattern)
		if not res:
			print("Err:", err)
			print("Expected suffix:", pattern)
			print("str(e):         ", str(e), "<<<")
			raise


def checkDeprecated(f):
	old = config.common.ignoreDeprecation
	config.common.ignoreDeprecation = True
	res = f()
	config.common.ignoreDeprecation = False
	try:
		f()
		assert False
	except DeprecationWarning:
		pass
	config.common.ignoreDeprecation = old
	return res


def _compareLines(lines1, lines2):
	import difflib
	differ = difflib.Differ()
	cand = list(line for line in differ.compare(lines1, lines2) if line[0] != ' ')
	if len(cand) != 0:
		msg = "Diff is:\n"
		msg += "".join(differ.compare(lines1, lines2))
		return msg
	else:
		return None


def _compareDumps(f1, f2):
	lines1 = [f"{l}\n" for l in strFromDump(f1).split("\n")]
	lines2 = [f"{l}\n" for l in strFromDump(f2).split("\n")]
	res = _compareLines(lines1, lines2)
	if res is not None:
		raise Exception(f"Files differ: {f1} vs. {f2}\n{res}")


def _compareFiles(f1, f2):
	with open(f1, 'r') as file1, open(f2, 'r') as file2:
		lines1 = file1.readlines()
		lines2 = file2.readlines()
	res = _compareLines(lines1, lines2)
	if res is not None:
		raise Exception(f"Files differ: {f1} vs. {f2}\n{res}")
