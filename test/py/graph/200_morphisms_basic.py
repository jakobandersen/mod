include("2xx_morphisms_helpers.py")

g = smiles("O")
fail(lambda: g.isomorphism(None), "codomain is null.")
fail(lambda: g.monomorphism(None), "codomain is null.")

fail(lambda: g.enumerateIsomorphisms(None, callback=False),
	"codomain is null.")
fail(lambda: g.enumerateMonomorphisms(None, callback=False),
	"codomain is null.")
