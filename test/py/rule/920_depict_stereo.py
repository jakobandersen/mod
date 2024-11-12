include("xxx_helpers.py")
include("xxx_gml_stereo.py")

def f(conf, group, title, gml):
	r = Rule.fromGMLString(gml, title)
	p = GraphPrinter()
	p.setReactionDefault()
	r.print(p)

	p = GraphPrinter()
	p.setReactionDefault()
	p.withPrettyStereo = True
	r.print(p)

	p = GraphPrinter()
	p.setReactionDefault()
	p.withRawStereo = True
	r.print(p)

	p = GraphPrinter()
	p.setReactionDefault()
	p.withIndex = True
	r.print(p)

	commonChecks(r)

foreachStereoExample(f)
