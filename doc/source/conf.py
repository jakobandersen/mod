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

intersphinx_mapping = {
	'python': ('https://docs.python.org/3', None)
}

cpp_index_common_prefix = ["mod::"]

todo_include_todos = True

templates_path = ['_templates']

source_suffix = '.rst'

master_doc = 'index'

project = u'MØD'
copyright = u'2013-2026, Jakob Lykke Andersen'

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

# used for the hyperflow model description
tikz_dg_flow = R"""
\newcommand\dgExBegin{%
	\def\labAB{$e_1$}
	\def\labBA{$e_2$}
	\def\labBC{$e_3$}
	\def\labCAB{$e_4$}
	\def\labFD{$e_5$}
	\def\labAI{$e_A^{in}$}\def\labAO{$e_A^{out}$}
	\def\labBI{$e_B^{in}$}\def\labBO{$e_B^{out}$}
	\def\labCI{$e_C^{in}$}\def\labCO{$e_C^{out}$}
	\def\labDI{$e_D^{in}$}\def\labDO{$e_D^{out}$}
	\def\labFI{$e_F^{in}$}\def\labFO{$e_F^{out}$}
	\tikzset{
		styleA/.style={},
		styleB/.style={},
		styleC/.style={},
		styleAB/.style={},	styleBA/.style={},
		styleBC/.style={},	styleBBC/.style={},	styleBCB/.style={},
		styleCAB/.style={},	styleCCAB/.style={},	styleCABA/.style={},	styleCABB/.style={},
	}
}
\newcommand\dgEx{%
	\matrix[row sep=20, column sep=20, matrixInnerSep] {
		\node[hnode] (F) {$F$};\& \node[hnode] (D) {$D$};		\\
		\node[hnode, styleB] (B) {$B$};	\& \node[hedge, styleBC, label=below:{\labBC}] (BC) {};	\& \node[hnode, styleC] (C) {$C$};	\\
		\node[hnode, styleA] (A) {$A$};\&\& \node[hedge, styleCAB, label=below:{\labCAB}] (CAB) {};		\\
	};
	
	\draw[edge, styleAB] (A.90+\isomerizationOffset) to node[auto, every label] {\labAB} (B.-90-\isomerizationOffset);
	\draw[edge, styleBA] (B.-90+\isomerizationOffset) to node[auto, every label] {\labBA} (A.90-\isomerizationOffset);
	\draw[edge] (F) to node[auto, every label] {\labFD} (D);
	
	\draw[edge, styleBBC] (B) to (BC);\draw[edge, styleBCB] (BC) to (C);
	\multiedge{D}{BC}{2}
	\draw[edge, styleCCAB] (C) to (CAB);\draw[edge, styleCABA] (CAB) to (A);\draw[edge, styleCABB] (CAB) to (B);
}
\newcommand\dgExRestricted{%
	\matrix[row sep=20, column sep=20, ampersand replacement=\&] {
		\node[hnode] (B) {$B$};	\& \node[hedge, label=below:{\labBC}] (BC) {};	\& \node[hnode] (C) {$C$};	\\
		\&\& \node[hedge, label=below:{\labCAB}] (CAB) {};		\\
	};

	\draw[edge, styleBBC] (B) to (BC);
	\draw[edge, styleBCB] (BC) to (C);
	\multiedge{C}{CAB}{1}\multiedge{CAB}{B}{1}
}
\newcommand\dgExAllIO{
	\makeIO{A}{180}{\labAI}{\labAO}
	\makeIO{B}{180}{\labBI}{\labBO}
	\makeIO{C}{0}{\labCI}{\labCO}
	\makeIO{D}{0}{\labDI}{\labDO}
	\makeIO{F}{180}{\labFI}{\labFO}
}


\newcommand\dgExExpPruned{
	\matrix[row sep=16, column sep=16] {
		\node[hxnode,label=above:$F$] (F) {};		\& \node[hxnode,label=above:$D$] (D) {};       \\
		\node[hxnode, label={[overlay]above:$B$}] (B) {};	\& \node[hedge, label=below:{\labBC}] (BC) {}; \& \node[hxnode,label={[overlay]above:$C$}] (C) {};    \\
		\node[hxnode, label=below:$A$] (A) {}; \&\& \node[hedge, label=below:{\labCAB}] (CAB) {};       \\
	};
	
	\makeShortcutEdge{A}{90+\isomerizationOffset}{B}{-90-\isomerizationOffset}{\labAB}
	\makeShortcutEdge{B}{-90+\isomerizationOffset}{A}{90-\isomerizationOffset}{\labBA}
	\makeShortcutEdge{F}{\isomerizationOffset}{D}{180-\isomerizationOffset}{\labFD}
	
	\makeEdge{D/-90/\isomerizationOffset, D/-90/-\isomerizationOffset, B/0/0}{BC}{C/180/0}
	\makeEdge{C/-90/0}{CAB}{A/0/0, B/-35/2}
}

\newcommand\dgExExp{
	\dgExExpPruned
}



\newcommand\makeIOExp[5][\ioEdgeDist]{{
	\makeIO[#1]{#2}{#3}{#4}{#5}
	\newcommand{\Angle}{#3}
	\newcommand{\Anchor}{#2}
	\newcommand\Offset\isomerizationOffset
	\node[tnode] (t-\Anchor-out-IO) at (\Anchor.\Angle+\Offset) {};
	\node[tnode] (t-\Anchor-in-IO) at (\Anchor.\Angle-\Offset) {};
}}
\newcommand\makeIExp[4][\ioEdgeDist]{{
	\newcommand{\Angle}{#3}
	\newcommand{\Anchor}{#2}
	\newcommand{\Dist}{#1}
	\newcommand{\InText}{#4}
	\newcommand\Offset\isomerizationOffset
	\draw[edge] ($(\Anchor.\Angle-\Offset) + (\Angle:\Dist)$) to node[auto, every label]{\InText} (\Anchor.\Angle-\Offset);
	\node[tnode] (t-\Anchor-in-IO) at (\Anchor.\Angle-\Offset) {};
}}
\newcommand\makeOExp[4][\ioEdgeDist]{{
	\newcommand{\Angle}{#3}
	\newcommand{\Anchor}{#2}
	\newcommand{\Dist}{#1}
	\newcommand{\OutText}{#4}
	\newcommand\Offset\isomerizationOffset
	\draw[edge] (\Anchor.\Angle+\Offset) to node[auto, every label]{\OutText} ($(\Anchor.\Angle+\Offset) + (\Angle:\Dist)$);
	\node[tnode] (t-\Anchor-out-IO) at (\Anchor.\Angle+\Offset) {};
}}
\newcommand\makeOExpStyle[5][\ioEdgeDist]{{
	\newcommand{\Angle}{#3}
	\newcommand{\Anchor}{#2}
	\newcommand{\Dist}{#1}
	\newcommand{\OutText}{#4}
	\newcommand\Offset\isomerizationOffset
	\draw[edge, #5] (\Anchor.\Angle+\Offset) to node[auto, every label]{\OutText} ($(\Anchor.\Angle+\Offset) + (\Angle:\Dist)$);
	\node[tnode, #5] (t-\Anchor-out-IO) at (\Anchor.\Angle+\Offset) {};
}}
\newcommand\dgExExpAllIO{
	\makeIOExp{A}{180}{\labAI}{\labAO}
	\makeIOExp{B}{180}{\labBI}{\labBO}
	\makeIOExp{C}{0}{\labCI}{\labCO}
	\makeIOExp{D}{0}{\labDI}{\labDO}
	\makeIOExp{F}{180}{\labFI}{\labFO}
}
\newcommand{\makeShortcutEdge}[5]{{
    \newcommand{\Source}{#1}
    \newcommand{\Target}{#3}
    \newcommand{\SourceAngle}{#2}
    \newcommand{\TargetAngle}{#4}
    \newcommand{\lab}{#5}
    \node[tnode] (t-\Source-out-sc-\Target) at (\Source.\SourceAngle) {};
    \node[tnode] (t-\Target-in-sc-\Source) at (\Target.\TargetAngle) {};
    \draw[edge] (t-\Source-out-sc-\Target) to node[auto, every label] {\lab} (t-\Target-in-sc-\Source);
}}
\newcommand{\makeShortcutEdgeStyle}[6]{{
    \newcommand{\Source}{#1}
    \newcommand{\Target}{#3}
    \newcommand{\SourceAngle}{#2}
    \newcommand{\TargetAngle}{#4}
    \newcommand{\lab}{#5}
    \node[tnode, #6] (t-\Source-out-sc-\Target) at (\Source.\SourceAngle) {};
    \node[tnode, #6] (t-\Target-in-sc-\Source) at (\Target.\TargetAngle) {};
    \draw[edge, #6] (t-\Source-out-sc-\Target) to node[auto, every label] {\lab} (t-\Target-in-sc-\Source);
}}
\newcommand{\makeEdge}[3]{{
    \newcommand{\Sources}{#1}
    \newcommand{\EdgeNode}{#2}
    \newcommand{\Targets}{#3}
    \foreach \Source/\SourceAngle/\Bend in \Sources {
        \node[tnode] (t-\Source-out-\EdgeNode) at (\Source.\SourceAngle) {};
        \draw[edge] (t-\Source-out-\EdgeNode) to [bend right=\Bend] (\EdgeNode);
    }
    \foreach \Target/\TargetAngle/\Bend in \Targets {
        \node[tnode] (t-\Target-in-\EdgeNode) at (\Target.\TargetAngle) {};
        \draw[edge] (\EdgeNode) to [bend right=\Bend] (t-\Target-in-\EdgeNode);
    }
}}
\newcommand{\makeEdgeStyle}[4]{{
    \newcommand{\Sources}{#1}
    \newcommand{\EdgeNode}{#2}
    \newcommand{\Targets}{#3}
    \foreach \Source/\SourceAngle/\Bend in \Sources {
        \node[tnode, #4] (t-\Source-out-\EdgeNode) at (\Source.\SourceAngle) {};
        \draw[edge, #4] (t-\Source-out-\EdgeNode) to [bend right=\Bend] (\EdgeNode);
    }
    \foreach \Target/\TargetAngle/\Bend in \Targets {
        \node[tnode, #4] (t-\Target-in-\EdgeNode) at (\Target.\TargetAngle) {};
        \draw[edge, #4] (\EdgeNode) to [bend right=\Bend] (t-\Target-in-\EdgeNode);
    }
}}
"""

tikz_strict_transit = R"""
% Ain, Aout
\newcommand\strictTransitCommon[2]{
	\node[circle, minimum width=60, minimum height=120] (skeleton) {};
	\node[hxnode, label=above:$A$] (A) at (skeleton.90) {};
	\node[hedge] (ABtoC) at (skeleton.0) {2};
	\node[hnode] (C) at (skeleton.-90) {$C$};
	\node[hedge] (CtoA) at (skeleton.180) {2};
	\draw[edge] (A) to [in=95, out=-20, out looseness=0.5] (ABtoC);	\node[tnode] (t-A-out-ABtoC) at (A.-20) {};

	\draw[edge] (ABtoC) to [out=-95, in=10, in looseness=0.5] (C);
	\draw[edge] (C) to [in=-85, out=170, out looseness=0.5] (CtoA);
	
	\node[tnode] (t-A-in-CtoA) at (A.-150) {};
	\draw[edge] (CtoA) to [out=70, in=-130, in looseness=0.5] (t-A-in-CtoA);		
	\draw[edge] (CtoA) to [out=90, in=-170, in looseness=0.5] (t-A-in-CtoA);
	
	\node[hnode] (B) [above right=of ABtoC, xshift=-10] {$B$};
	% for bounding box
	\node[hnodeNoDraw] (X) [above left=of CtoA, xshift=10] {\phantom{$B$}};
	\draw[edge] (B) to [bend right=15] (ABtoC);
	
	\node[tnode] (t-A-in-IO) at (A.130) {};
	\node[tnode] (t-A-out-IO) at (A.50) {};
	
	\draw[edge] ($(A.130) + (145:2em)$) to node[auto] {#1} (t-A-in-IO);
	\draw[edge] (t-A-out-IO) to node[auto] {#2} ($(A.50) + (35:2em)$);
	
	\draw[edge] ($(B.90) + (90:2em)$) to node[auto] {2} (B.90);
	
	\draw[tedge] (t-A-in-IO) to node[above, pos=0.29] {#1} (t-A-out-ABtoC);
	\draw[tedge] (t-A-in-CtoA) to node[above, pos=0.75] {#2} (t-A-out-IO);
}
"""

tikz_latex_preamble = \
	tikz_general      \
	+ tikz_dg_styles  \
	+ tikz_dg_flow    \
	+ tikz_strict_transit
