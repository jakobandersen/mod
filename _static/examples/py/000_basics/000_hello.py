# Normal printing to the terminal:
print("Hello world")
# Make some headers in the summary:
post.summaryChapter("Hello")
post.summarySection("World")
# Load a moleucle from a SMILES string:
mol = smiles("Cn1cnc2c1c(=O)n(c(=O)n2C)C", name="Caffeine")
# Put a visualisation of the molecule in the summary:
mol.print()
# rst-name: Hello World
# rst: These examples use the Python 3 interface for the software.
# rst: After each run a PDF summary is compiled.
# rst: The content can be specified via the Python script.
