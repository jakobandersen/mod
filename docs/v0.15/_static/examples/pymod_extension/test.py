import awesome

# sanity check for multiple copies of libMØD
modValue = mod.magicLibraryValue()
ourValue = awesome.magicLibraryValue()
if modValue != ourValue:
	print("mod =", modValue)
	print("our =", ourValue)
	raise Exception("Magic values differ! I.e., more than one instance of libMØD has been loaded.")
# end if check

g = awesome.doStuff()
print("Got a graph:", g.name)
g.print()
