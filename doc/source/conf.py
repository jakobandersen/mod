import sys
import os

# -- General configuration ------------------------------------------------
needs_sphinx = '3.5.0'

sys.path.append(os.path.abspath('extensions'))
extensions = [
    'sphinx.ext.todo',
    'sphinx.ext.mathjax',
	'sphinx.ext.githubpages',
	'sphinx.ext.intersphinx',
	'sphinx_bootstrap_theme',
	'ignore_missing_refs',
	'sphinx_design',
]

autoapi_type = 'python'
autoapi_dirs = ['../../libs/pymod/lib/mod']
autoapi_generate_api_docs = False
autoapi_keep_files = True

intersphinx_mapping = {
	'python': ('https://docs.python.org/3', None)
}

cpp_index_common_prefix = ["mod::"]

todo_include_todos = True

templates_path = ['_templates']

source_suffix = '.rst'

master_doc = 'index'

project = u'MØD'
copyright = u'2013-2024, Jakob Lykke Andersen'

with open("../../VERSION") as f:
	version = f.read()
	version = version.strip() # remove the newline
# The full version, including alpha/beta/rc tags.
release = version

exclude_patterns = [
	'libmod/Toc.rst',
	'pymod/Toc.rst',
	'formats/dfs.rst',
	'formats/dg.rst',
	'formats/dot.rst',
	'formats/gml.rst',
	'formats/mdl.rst',
	'formats/smiles.rst',
	'formats/tikz.rst',
]

# https://stackoverflow.com/questions/48615629/how-to-include-pygments-styles-in-a-sphinx-project
pygments_style = 'solarized_modified.light'
highlight_language = 'none'

# -- Options for HTML output ----------------------------------------------

html_theme = 'bootstrap'
html_css_files = [
	"haxes.css",
]

html_theme_options = { 
    "body_max_width": None,
	'navbar_links': [
		("Index", "genindex"),
	], 
	'source_link_position': "footer",
	'bootswatch_theme': "litera",
}

html_static_path = ['_static']
