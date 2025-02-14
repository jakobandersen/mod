import sys
import os

# -- General configuration ------------------------------------------------
needs_sphinx = '8.0.2'

sys.path.insert(0, os.path.abspath('extensions'))
extensions = [
	'sphinx.ext.todo',
	'sphinx.ext.mathjax',
	'sphinx.ext.githubpages',
	'sphinx.ext.intersphinx',
	'sphinx_bootstrap_theme',
	'sphinxcontrib.jquery',
	'sphinxcontrib.tikz',
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
copyright = u'2013-2025, Jakob Lykke Andersen'

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


tikz_tikzlibraries = "arrows.meta, calc, matrix, positioning, shapes"
#shadows,decorations.markings}

tikz_general = R"""
\usepackage[prefix=sol-]{xcolor-solarized}

\tikzset{
	every matrix/.style={ampersand replacement=\&},
}
"""

tikz_dg_styles = R"""
\tikzset{
	matrixInnerSep/.style={inner sep=0, nodes={inner sep=.3333em)}}
}
\tikzset{hnodeNoDraw/.style={ellipse,minimum height=12, minimum width=17}}
\tikzset{hnode/.style={draw,hnodeNoDraw}}
\tikzset{hxnode/.style={hnode, minimum size=50}}
\tikzset{tnode/.style={draw,circle,fill=black,inner sep=0,minimum size=3,text height=0ex,text depth=0ex}}
\tikzset{hedgeNoDraw/.style={rectangle,minimum height=8, minimum width=8}}
\tikzset{hedge/.style={draw,hedgeNoDraw}}
\tikzset{edge/.style={->,>=Stealth,thick}}
\tikzset{tedge/.style={->,>=Stealth}}

% from, to, numEdges; assumes the style 'edge' is defined
\newcommand{\multiedge}[4][20]{
	\pgfmathsetmacro{\num}{#4-1}
	\foreach \i in {0, ..., \num}{
		\pgfmathsetmacro{\angleSep}{40}
		\pgfmathsetmacro{\angle}{\angleSep*(\i-#4/2+.5)}
		\draw[edge] (#2) to [bend right=\angle] (#3);
	}
}

\newcommand\ioEdgeDist{0.9}
\newcommand\isomerizationOffset{15}
% [dist], anchor, angle, inText, outText
\newcommand\makeIO[5][\ioEdgeDist]{{
	\newcommand{\Angle}{#3}
	\newcommand{\Anchor}{#2}
	\newcommand{\Dist}{#1}
	\newcommand{\InText}{#4}
	\newcommand{\OutText}{#5}
	\newcommand\Offset\isomerizationOffset
	\draw[edge] (\Anchor.\Angle+\Offset) to node[auto, every label]{\OutText} ($(\Anchor.\Angle+\Offset) + (\Angle:\Dist)$);
	\draw[edge] ($(\Anchor.\Angle-\Offset) + (\Angle:\Dist)$) to node[auto, every label]{\InText} (\Anchor.\Angle-\Offset);
}}
"""

tikz_latex_preamble = \
	tikz_general      \
	+ tikz_dg_styles
