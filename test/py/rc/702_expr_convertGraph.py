include("7xx_expr_common.py")


def convert(name, cls, f):
	print("="*80)
	print(name)
	print("="*80)

	fail(lambda: cls(None), "The graph is null.")

	post.summarySection(name)
	print(name + ", explicit -------------------------------------------------")
	handleExp(cls(formaldehyde))
	print(name + ", implicit -------------------------------------------------")
	handleExp(f(formaldehyde))
	print(name + ", multi ----------------------------------------------------")
	handleExp(f(inputGraphs), True)
	print()
convert("Bind", RCExpBind, rcBind)
convert("Id", RCExpId, rcId)
convert("Unbind", RCExpUnbind, rcUnbind)
