
g = Graph.fromSMILES("C")

def test(f, *, raises=False):
	fw = mod._funcWrap(libpymod._Func_StringGraph, f)
	s = str(fw)
	try:
		fw(g)
		if raises:
			assert False, "Did not raise"
	except Exception as e:
		if not raises:
			assert False, "Raised: " + str(e)
		print("Got expected exception:", e)

print("="* 80)
test("const")


print("="* 80)
def func(g):
	return "func"
def funcRaise(g):
	raise Exception("bah")

test(func)
test(funcRaise, raises=True)


print("="* 80)
test(lambda g: "lambda")
test(lambda g: funcRaise(g), raises=True)


print("="* 80)
class Class:
	def __call__(self, g):
		return "class"
class ClassRaises:
	def __call__(self, g):
		funcRaise(g)

test(Class())
test(ClassRaises(), raises=True)
