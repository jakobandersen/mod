.. py:currentmodule:: mod
.. cpp:namespace:: mod

.. _flowCommon:

****************
Hyperflow Model
****************

The hyperflow model on derivation graphs, accessible through
:cpp:class:`hyperflow::Model` (C++) and :py:class:`hyperflow.Model` (Python),
consists of a base model and several modules.
Each module defines one or more sets of variables with associated constraints.

If you use the hyperflow functionality in your research you may want to cite [AFMS-Hyperflows]_.
Results on computational complexity of finding hyperflows can be found in [AFMS-NPFlow]_.


Mathematical Description
########################

Given is first of all a directed multi-hypergraph (e.g., modelling a chemical
reaction network), in the form of a derivation graph object
(:cpp:class:`dg::DG`/:py:class:`DG`).
Let this hypergraph be called :math:`\mathcal{H} = (V, E)`, with :math:`V`
being the set of vertices (e.g., molecules) and :math:`E` being the set of
directed hyperedges (e.g., reactions).
As a running example, we use the network below.

.. tikz:: Example of a directed multi-hypergraph.

	\dgExBegin
	\dgEx

Each hyperedge :math:`e\in E` is an ordered pair of multisets of vertices
:math:`e = (e^{out}, e^{in})`, the sources and targets of the edge (e.g., the
reactants and products of the reaction).
To denote the multiplicity of a vertex :math:`v` in a multiset `s` we write
:math:`m_v(s)`.
For example, in the network above, the edge :math:`e_3 = (\{B, D, D\}, \{C\})`
could model a reaction :math:`B + 2\,D\rightarrow C`.
We have :math:`m_D(e_3^{out}) = 2` as :math:`D` is used twice as a reactant.
The multiplicity function thus gives the stoichiometric coefficients in a reaction.
In the context of stoichiometric matrices the multiplicities for the educts and
products correcpond to the entries of :math:`\mathbf{S}^-` and
:math:`\mathbf{S}^+` respectively.

To model input and output of the network we extend it with half-edges at each vertex,
and construct the *extended hypergraph*: :math:`\overline{\mathcal{H}} = (V,
\overline{E})`,
with :math:`\overline{E} = E\cup E^{in} \cup E^{out}` where 
:math:`E^{in} = \{e^{in}_v = (\emptyset, \{v\}) \mid v\in V\}` and 
:math:`E^{out} = \{e^{out}_v = (\{v\}, \emptyset) \mid v\in V\}`.
If we extend our example network we arrive at the extended network shown below.

.. tikz:: Example of an extended network.
	Each vertex has an additional input edge and output edge.

	\dgExBegin
	\dgEx
	\dgExAllIO


Basic Hyperflow Model
---------------------

Given an extended hypergraph :math:`\overline{\mathcal{H}} = (V, \overline{E})`,
an integer hyperflow is a function :math:`f\colon \overline{E}\rightarrow \mathbb{N}_0`
that assigns a non-negative integer to each hyperedge, including the input/output edges.
It must satisfy the flow conservation constraint

.. math::

	\begin{align*}
	\sum_{e\in \text{out}_{\overline{E}}(v)} m_v(e^{out})f(e) - 
	\sum_{e\in \text{in}_{\overline{E}}(v)} m_v(e^{in})f(e) &= 0
		& \forall v\in V
	\end{align*}

where :math:`\text{out}_{\overline{E}}(v)` and
:math:`\text{in}_{\overline{E}}(v)` are the sets of out-edges and in-edges of a
vertex :math:`v` respectively.
An example of an integer hyperflow is shown below.

.. tikz:: Example of an integer hyperflow on the extended example network.

	\dgExBegin
		\def\labAB{$1$}\def\labBA{$1$}
		\def\labBC{$1$}
		\def\labCAB{$1$}
		\def\labFD{$2$}
		\def\labAI{$1$}\def\labAO{$2$}
		\def\labBI{$0$}\def\labBO{$0$}
		\def\labCI{$0$}\def\labCO{$0$}
		\def\labDI{$0$}\def\labDO{$0$}
		\def\labFI{$2$}\def\labFO{$0$}
	\dgEx
	\dgExAllIO

.. tikz:: The same integer hyperflow, where edges with no flow are hidden.

	\dgExBegin
		\def\labAB{1}\def\labBA{1}
		\def\labBC{1}
		\def\labCAB{1}
		\def\labFD{2}
	\dgEx
	\makeIO{A}{180}{1}{2}
	\draw[edge] ($(F.180) + (180:\ioEdgeDist)$) to node[auto, every label] {2} (F.180);


Expanded Network and Hyperflow Model
------------------------------------

For many uses, the basic model described above is sufficient,
but often it is advantageous to have more fine-grained control of how flow is routed.
The network is therefore transformed further by replacing each vertex with a small
network (a complete bipartite graph), such that one can restrict how incomming
flow to a vertex is routed to outgoing edges.
Formally we create the *expanded network*
:math:`\widetilde{\mathcal{H}} = (\widetilde{V}, \widetilde{E})`,
with

.. math::

	\begin{align*}
	\widetilde{V} &= \bigcup_{v\in V} V_v^{in} \cup \bigcup_{v\in V} V_v^{out}   \\
	V_v^{in}  &= \{u^{in}_{ve} \mid \forall e\in \text{in}_{\overline{E}}(v)\}\\
	V_v^{out} &= \{u^{out}_{ve} \mid \forall e\in \text{out}_{\overline{E}}(v)\}\\
	\end{align*}

and

.. math::

	\begin{align*}
	\widetilde{E} &= \bigcup_{v\in V}E_v \cup \{\widetilde{e}\mid e\in \overline{E}\} \\
	\widetilde{e}       &= (\widetilde{e}^{out}, \widetilde{e}^{in}) \\
	\widetilde{e}^{in}  &= \{u^{in}_{ve}  \mid v \in e^{in}\}        \\
	\widetilde{e}^{out} &= \{u^{out}_{ve} \mid v \in e^{out}\}       \\
	E_v &= \left\{\left(\{u^{in}\}, \{u^{out}\}\right)
		\mid u^{in}\in V_v^{in}, u^{out}\in V_v^{out}\right\}
	\end{align*}

That is, each original vertex is replaced with a vertex for each in-edge and for each
out-edge of that replaced vertex.
The original hyperedges are then reconnected to these new vertices,
and a set of new 1-to-1 hyperedges, :math:`E_v` are added for each of the
original vertices that connect all in-vertices to all out-vertices.
We call these new edges *transit edges*.
An example of network expansion is shown below.

.. tikz:: Example of an expanded network.
	The vertices are the small black dots, while the large circles are a visual
	aid to group the vertices and edges related to the orignal vertices.
	The blue transit edges are those that connect pairs of reverse internal hyperedges,
	while the red transit edges are those that connect the input and output edges.

	\dgExBegin
		\def\labAB{}\def\labBA{}
		\def\labBC{}\def\labFD{}\def\labCAB{}
	\dgExExp
		\def\labAI{}\def\labAO{}
		\def\labBI{}\def\labBO{}
		\def\labCI{}\def\labCO{}
		\def\labDI{}\def\labDO{}
		\def\labFI{}\def\labFO{}
	\dgExExpAllIO
	
	% all reversible, IO
	\foreach \s/\t/\b in {A-in-IO/A-out-IO, B-in-IO/B-out-IO, C-in-IO/C-out-IO, D-in-IO/D-out-IO, F-in-IO/F-out-IO} {
		\draw[tedge, sol-blue] (t-\s) to [bend right=-45, looseness=1.2] (t-\t);
	}
	% all reversible, non-IO
	\foreach \s/\t/\b in {B-in-sc-A/B-out-sc-A, A-in-sc-B/A-out-sc-B} {
		\draw[tedge, sol-red] (t-\s) to [bend right=-45, looseness=1.2] (t-\t);
	}
	% all normal
	\foreach \s/\t/\b in {A-in-IO/A-out-sc-B/30,    A-in-sc-B/A-out-IO/-40, A-in-CAB/A-out-IO/-5, A-in-CAB/A-out-sc-B/-40,
			B-in-IO/B-out-sc-A/-40, B-in-IO/B-out-BC/-5,    B-in-sc-A/B-out-IO/40, B-in-sc-A/B-out-BC/-40,  B-in-CAB/B-out-sc-A/30, B-in-CAB/B-out-IO/5, B-in-CAB/B-out-BC/-60,
			C-in-BC/C-out-CAB/-45, C-in-BC/C-out-IO/-5,  C-in-IO/C-out-CAB/40,
			D-in-IO/D-out-BC/30,	D-in-sc-F/D-out-BC/-45, D-in-sc-F/D-out-IO/0,
			F-in-IO/F-out-sc-D/0} {
		\draw[tedge] (t-\s) to [bend right=\b, looseness=1] (t-\t);
	}

It will be convenient to specify some particular transit edges:

- The IO transit edges :math:`T_{IO}` (blue edges in the example above):
  in each vertex, the transit edge from the input to the output.
- The reverse edge transit edges :math:`T_{rev}` (red edges in the example blue):
  in each vertex, the transit edges that go between edges that are reverse of each other.
  That is, for each pair of hyperedges :math:`e_1, e_2\in \overline{E}`
  with :math:`e_1 = (V_a, V_b), e_2 = (V_b, V_a)`, each transit edge
  :math:`(u_{ve_1}^{in}, u_{ve_2}^{out})` for :math:`v\in V_b` and
  :math:`(u_{ve_2}^{in}, u_{ve_1}^{out})` for :math:`v\in V_a` are in :math:`T_{rev}`.
  Note that a loop edge :math:`e = (V_a, V_a)` is also considered to be a
  reverse of it self, and are thus considered here.

An integer hyperflow on this expanded network is then defined as before, but
just on :math:`\widetilde{\mathcal{H}}`.
A hyperflow on our example of an expanded hypergraph is shown below.
When projecting it back on to the non-expanded network, it is equal to the previously
shown hyperflow.

.. tikz:: Example of an integer hyperflow on an expanded hypergraph.

	\dgExBegin
		\def\labAB{1}\def\labBA{1}
		\def\labBC{1}\def\labFD{2}\def\labCAB{1}
	\dgExExpPruned
		\def\labAI{1}\def\labAO{2}
		\def\labBI{}\def\labBO{}
		\def\labCI{}\def\labCO{}
		\def\labDI{}\def\labDO{}
		\def\labFI{2}\def\labFO{}
	\makeIOExp{A}{180}{\labAI}{\labAO}
	\makeIExp{F}{180}{\labFI}
	
	% all normal
	\foreach \s/\t/\b in {A-in-IO/A-out-sc-B/30}		\draw[tedge] (t-\s) to [bend right=\b, looseness=1] node[auto, swap, every label, inner sep=1] {1} (t-\t);
	\foreach \s/\t/\b in {A-in-sc-B/A-out-IO/-40}		\draw[tedge] (t-\s) to [bend right=\b, looseness=1] node[auto, every label, inner sep=1, pos=0.1] {1} (t-\t);
	\foreach \s/\t/\b in {A-in-CAB/A-out-IO/-5}		\draw[tedge] (t-\s) to [bend right=\b, looseness=1] node[auto, every label, inner sep=1] {1} (t-\t);
	\foreach \s/\t/\b/\l in {B-in-sc-A/B-out-BC/-40,
			C-in-BC/C-out-CAB/-45}{
		\draw[tedge] (t-\s) to [bend right=\b, looseness=1] node[auto, every label, inner sep=1] {1} (t-\t);
	}
	\foreach \s/\t/\b/\l in {D-in-sc-F/D-out-BC/-45}	\draw[tedge] (t-\s) to [bend right=\b, looseness=1] node[auto, every label, inner sep=1] {2} (t-\t);
	\foreach \s/\t/\b/\l in {B-in-CAB/B-out-sc-A/30}	\draw[tedge] (t-\s) to [bend right=\b, looseness=1] node[auto, swap, every label, inner sep=1] {1} (t-\t);
	\foreach \s/\t/\b/\l in {F-in-IO/F-out-sc-D/0}		\draw[tedge] (t-\s) to [bend right=\b, looseness=1] node[auto, every label] {2} (t-\t);


Base Model Specification
########################

Hyperflows are found by creating a
:cpp:class:`hyperflow::Model`/:py:class:`hyperflow.Model`
object, where a network is given as :cpp:class:`dg::DG`/:py:class:`DG`.
The model object contains first a specification of a hyperflow problem,
which can modified, after which one or more hyperflow solutions can be found.
Each solution is a hyperflow :math:`f` as mathematically defined in the previous section.
It is represented by a set of variables :math:`x_e` for each :math:`e\in \widetilde{E}`.
Multiple sets of auxiliary variables are defined for convenience,
predominantly *indicator variables*, i.e., variables of value 0 or 1, that indicate
various conditions.
For example, for each edge of the original network :math:`e\in E` there is a
flow variable :math:`x_e` and then an indicator variable :math:`z_e` that
indicates the condition :math:`x_e > 0`.
That is, :math:`z_e` is 1 if there is non-zero flow on the edge :math:`e` and 0
if there is no flow.

An initial model object specifies that nothing can flow in and out of the network,
i.e., :math:`x_v^{in} = x_v^{out} = 0` for each vertex :math:`v\in V`.
To remove this constraint, can specify a vertex as a source or sink using the methods
:cpp:func:`hyperflow::Model::addSource`/:py:meth:`hyperflow.Model.addSource`
and
:cpp:func:`hyperflow::Model::addSink`/:py:meth:`hyperflow.Model.addSink`.

By default a the model does not allowed to have flow on the edges in
:math:`T_{rev}` (see the section above), i.e., flow can not go through one edge
and then reversing directly back through the inverse of that edge (unless it is
the input/output edges).
If you want to allow this, you can use
:cpp:func:`hyperflow::Model::setAllowReversal`/:py:attr:`hyperflow.Model.allowReversal`.
By default this flow reversal is however allowed for the input/output edges (on
the edges in :math:`T_{IO}`. This means that if too much flow is going into the
network, it is allowed to flow directly out again.
You can disallow this through
:cpp:func:`hyperflow::Model::setAllowIOReversal`/:py:attr:`hyperflow.Model.allowIOReversal`.
For example, consider the left-most flow below. 

.. list-table::

	* - .. tikz:: Example of a flow through pairs of reverse edges.
		In particular, the flow through :math:`B + C\rightarrow D` is unambiguously going
		back through the inverse, :math:`D\rightarrow B + C`, which often is undesirable.

		\matrix[matrixInnerSep, row sep=8, column sep=8] {
		\&\& \node[hnode] (AB) {\phantom{$A$}};	\&\& \node[hnode, draw=sol-cyan] (C) {$C$};
			\&\&\& \node[hedge, draw=sol-violet, label={[overlay]right:1}] (BCe) {}; \\
		\& \node[hedge, label={[overlay]1}] (Ae) {};   \&\&  \node[hedge, label={[overlay]1}] (Be) {};
			\&\&\&\&\& \node[hnode, draw=sol-violet] (D) {$D$}; \\
		\node[hnode] (A) {$A$}; \&\& \node[hnode] (BA) {\phantom{$A$}}; \&\& \node[hnode] (B) {$B$};
			\&\&\&  \node[hedge, draw=sol-violet, label={[overlay]right:1}] (De) {};   \\
		};
		\foreach \s/\t in {A/Ae, Ae/AB, AB/Be, Be/B, Be/BA, BA/Ae} {
			\draw[edge] (\s) to (\t);
		};
		\draw[edge] ($(A.-90) + (-90:\ioEdgeDist)$) to node[auto, every label] {1} (A.-90);
		\makeIO{B}{-90}{1}{2}

		\draw[edge, draw=sol-cyan] (B.90+\isomerizationOffset) to node[auto, every label] {1} (C.-90-\isomerizationOffset);
		\draw[edge, draw=sol-cyan] (C.-90+\isomerizationOffset) to node[auto, every label] {1} (B.90-\isomerizationOffset);
		\foreach \s/\t in {B/BCe, C/BCe, BCe/D, D/De, De/B, De/C} {
			\draw[edge, draw=sol-violet] (\s) to (\t);
		};

	  - .. tikz:: A simplified version of the flow where the flow on the purple edges
		has been cancelled out.
		Now the flow through the edge :math:`B\rightarrow C` is unambiguously going through
		the inverse, :math:`C\rightarrow B`.

		\matrix[matrixInnerSep, row sep=8, column sep=8] {
		\&\& \node[hnode] (AB) {\phantom{$A$}};	\&\& \node[hnode, draw=sol-cyan] (C) {$C$};
			\&\&\& \\
		\& \node[hedge, label={[overlay]1}] (Ae) {};   \&\&  \node[hedge, label={[overlay]1}] (Be) {};
			\&\&\&\&\& \\
		\node[hnode] (A) {$A$}; \&\& \node[hnode] (BA) {\phantom{$A$}}; \&\& \node[hnode] (B) {$B$};
			\&\&\& \\
		};
		\foreach \s/\t in {A/Ae, Ae/AB, AB/Be, Be/B, Be/BA, BA/Ae} {
			\draw[edge] (\s) to (\t);
		};
		\draw[edge] ($(A.-90) + (-90:\ioEdgeDist)$) to node[auto, every label] {1} (A.-90);
		\makeIO{B}{-90}{1}{2}

		\draw[edge, draw=sol-cyan] (B.90+\isomerizationOffset) to node[auto, every label] {1} (C.-90-\isomerizationOffset);
		\draw[edge, draw=sol-cyan] (C.-90+\isomerizationOffset) to node[auto, every label] {1} (B.90-\isomerizationOffset);

	  - .. tikz:: A further simplified version of the flow where also the flow on the green edges
		has been cancelled out.
		Now the input flow to :math:`B` is unambiguously going back out again.

		\matrix[matrixInnerSep, row sep=8, column sep=8] {
		\&\& \node[hnode] (AB) {\phantom{$A$}};	\&\& 
			\&\&\& \\
		\& \node[hedge, label={[overlay]1}] (Ae) {};   \&\&  \node[hedge, label={[overlay]1}] (Be) {};
			\&\&\&\&\& \\
		\node[hnode] (A) {$A$}; \&\& \node[hnode] (BA) {\phantom{$A$}}; \&\& \node[hnode] (B) {$B$};
			\&\&\& \\
		};
		\foreach \s/\t in {A/Ae, Ae/AB, AB/Be, Be/B, Be/BA, BA/Ae} {
			\draw[edge] (\s) to (\t);
		};
		\draw[edge] ($(A.-90) + (-90:\ioEdgeDist)$) to node[auto, every label] {1} (A.-90);
		\makeIO{B}{-90}{1}{2}

	  - .. tikz:: A fully simplified version of the flow where also the input/output flow has
		been cancelled out.

		\matrix[matrixInnerSep, row sep=8, column sep=8] {
		\&\& \node[hnode] (AB) {\phantom{$A$}};	\&\& 
			\&\&\& \\
		\& \node[hedge, label={[overlay]1}] (Ae) {};   \&\&  \node[hedge, label={[overlay]1}] (Be) {};
			\&\&\&\&\& \\
		\node[hnode] (A) {$A$}; \&\& \node[hnode] (BA) {\phantom{$A$}}; \&\& \node[hnode] (B) {$B$};
			\&\&\& \\
		};
		\foreach \s/\t in {A/Ae, Ae/AB, AB/Be, Be/B, Be/BA, BA/Ae} {
			\draw[edge] (\s) to (\t);
		};
		\draw[edge] ($(A.-90) + (-90:\ioEdgeDist)$) to node[auto, every label] {1} (A.-90);
		\draw[edge] (B.-90) to node[auto, every label] {1} ($(B.-90) + (-90:\ioEdgeDist)$);

The network expansion as explained in the previous section introduced many new vertices
and transit edges. In the implementation the expansion is only being done as necessary.
E.g., if all flow reversal is allowed, then each original vertex will be replaced by just
a single transit edge and two vertices. All in-edges are connected to the same new vertex
and all out-edges are similarly connected to the other new vertex.

..
	In some applications of the hyperflow model, it can be interesting to constrain or query
	how flow is going through a vertex, and in particular how much flow is coming
	from the network (i.e., not through the input edge), and then flowing back into
	the network (i.e., not out through the output edge).
.. separateIOInternalTransit


Relaxed Mode
------------

The flow function is defined as assigning *integers* to each hyperedges,
encoding how many times teach edge must be used to achieve balance.
If you want to relaxed the model such that the flow values become floating
point numbers, you can do that through
:cpp:func:`hyperflow::Model::setRelaxed`/:py:attr:`hyperflow.Model.relaxed`.
The hyperflow model thus degenerate to be equivalent to the model in Flux
Balance Analysis (FBA).
However, note that many features of the model are disabled in relaxed mode,
e.g., solution enumeration, indicator variables, and several modules,
because they are not well-defined with non-integer hyperflows.


.. _flowCommon-varSpecLinExp:

Variable Specifiers, Linear Expressions, and Constraints
########################################################

The base model does not provide any constraints on the flow apart from which
vertices are allowed to have input and output flow, and if allowed, then it is
unbounded.
The model allows for adding arbitrary linear constraints, but this requires
us a way of specifying model variables in C++ and Python.
What we will arrive at, is that we can write the following Python code (and a
quite similar version can be written in C++).

.. literalinclude:: var_spec.py
	:language: python

Here we have a three variable specifiers named:

1. :py:data:`~hyperflow.vars.inFlow`, which is used just as it is, thereby representing the sum of all in-flow variables, i.e., :math:`\sum_{v\in V}x_v^{in}`.
2. :py:data:`~hyperflow.vars.edgeFlow`, which is indexed by a :py:class:`DG.HyperEdge`
   object, in two places, to retrive a variable specifier for the specific
   edge-flow variable.
3. :py:data:`~hyperflow.vars.outFlow`, which is similarly used twice to retrieve
   particular out-flow variables. In this example we index by a
   :py:class:`Graph`, but we could also index with a :py:class:`DG.Vertex`.

No matter if a variable specifier is indexed or not, it can be used to create a linear
expression, and they can again be used to create linear constraints.
That is, when using arithmetic and relational operators on these objects, you are not
actually evaluating anything, but merely creating objects that represent those
expressions.
Variable specifiers and linear expressions can also be used to query a found flow
solution, using :py:meth:`hyperflow.Solution.eval`/:cpp:func:`hyperflow::Solution::eval`.


Named Variable Specifiers
-------------------------

The named variable specifiers are defined both in the Python and C++ interface.
In C++ the variable specifiers are defined in the namespace ``mod::hyperflow::vars``,
it may thus be useful to use the using-declaration
``using namespace mod::hyperflow::vars;``
when defining linear expressions/constraints.
In Python the variable specifiers are defined in the submodule ``mod.hyperflow.vars``,
but the root module, ``mod``, imports them so they are available directly.

Each defined specifier represents a sum of variables, and indexing the
specifier retrieves a specifier for a single variable (or a sum of a subset of
the variables) as indicated by the argument.
E.g., ``inFlow`` represents the sum :math:`\sum_{v\in V}x^{in}_v` while
``inFlow[a]`` retrieves the specifier for :math:`x^{in}_v` where ``a`` is either
the vertex :math:`v` or a graph associated with that vertex
in the underlying derivation graph :math:`\mathcal{H} = (V, E)`.
In the table below, the "Type" column essentially indicates which objects can be used for
indexing. See also the types
:cpp:class:`hyperflow::VarSumVertex`/:py:class:`hyperflow.VarSumVertex`
and
:cpp:class:`hyperflow::VarSumEdge`/:py:class:`hyperflow.VarSumEdge`.
The "Module" column refers to which part of the flow model the variables are defined in,
with "Base" being the core model.
When a variable specifier is used for specification, e.g., in the objective function or
a constraint, then the corresponding module will automatically be enabled.

.. list-table::
  :header-rows: 1

  * - Specifier (Python/C++)
    - Type
    - Module
    - Description
  * - :py:data:`~hyperflow.vars.inFlow` / :cpp:var:`inFlow <hyperflow::vars::inFlow>`
    - Vertex
    - Base
    - The input flow variables, :math:`x^{in}_v`.
      When listing a solution the column header for this variable is ``In``.
  * - :py:data:`~hyperflow.vars.outFlow` / :cpp:var:`outFlow <hyperflow::vars::outFlow>`
    - Vertex
    - Base
    - The output flow variables, :math:`x^{out}_v`.
      When listing a solution the column header for this variable is ``Out``.
  * - :py:data:`~hyperflow.vars.isInUsed` / :cpp:var:`isInUsed <hyperflow::vars::isInUsed>`
    - Vertex
    - Base
    - Indicator variables for :math:`x^{in}_v > 0`.
  * - :py:data:`~hyperflow.vars.isOutUsed` / :cpp:var:`isOutUsed <hyperflow::vars::isOutUsed>`
    - Vertex
    - Base
    - Indicator variables for :math:`x^{out}_v > 0`.
  * - :py:data:`~hyperflow.vars.isInLessOut` / :cpp:var:`isInLessOut <hyperflow::vars::isInLessOut>`
    - Vertex
    - Base
    - Indicator variables for :math:`x^{in}_v < x^{out}_v`.
  * - :py:data:`~hyperflow.vars.isInGreaterOut` / :cpp:var:`isInGreaterOut <hyperflow::vars::isInGreaterOut>`
    - Vertex
    - Base
    - Indicator variables for :math:`x^{in}_v > x^{out}_v`.
  * - :py:data:`~hyperflow.vars.isInOutZero` / :cpp:var:`isInOutZero <hyperflow::vars::isInOutZero>`
    - Vertex
    - Base
    - Indicator variables for :math:`x^{in}_v = x^{out}_v = 0`.
  * - :py:data:`~hyperflow.vars.vertexFlow` / :cpp:var:`vertexFlow <hyperflow::vars::vertexFlow>`
    - Vertex
    - Base
    - Variables for flow through each vertex, :math:`\sum_{v\in V}\sum_{e\in \delta^{in}(v)} x_e`.
      The indexed variable specifier retrieves the inner sum for some :math:`v`, :math:`\sum_{e\in \delta^{in}(v)} x_e`.
  * - :py:data:`~hyperflow.vars.isVertexUsed` / :cpp:var:`isVertexUsed <hyperflow::vars::isVertexUsed>`
    - Vertex
    - Base
    - Indicator variables for :math:`\sum_{e\in \delta^{in}(v)} x_e`.
  * - :py:data:`~hyperflow.vars.transitInternalFlow` / :cpp:var:`transitInternalFlow <hyperflow::vars::transitInternalFlow>`
    - Vertex
    - Base
    - Variables for flow through each vertex, which enters and exits the vertex from/to the network.
      That is, excluding the flow that is covered by ``inFlow`` and that covered by ``outFlow``.
      Note, the indexed specifier can only be used if the separation of the transit flow is possible
      in the underlying expanded derivation graph.
      The non-indexed specifier can only be used if all indexed specifiers exist.
      The necessary network expansion can be triggered by disabling IO flow reversal
      (:cpp:texpr:`hyperflow::Model::setAllowIOReversal(false)` or set :py:attr:`hyperflow.Model.allowIOReversal` to ``False``),
      or by explicitly calling
      :cpp:func:`hyperflow::Model::separateIOInternalTransit` / :py:meth:`hyperflow.Model.separateIOInternalTransit`
      to make the variable available for the relevant vertices.
  * - :py:data:`~hyperflow.vars.edgeFlow` / :cpp:var:`edgeFlow <hyperflow::vars::edgeFlow>`
    - Edge
    - Base
    - The edge flow variables, :math:`x_e`, without the virtual input/output edges.
  * - :py:data:`~hyperflow.vars.isEdgeUsed` / :cpp:var:`isEdgeUsed <hyperflow::vars::isEdgeUsed>`
    - Edge
    - Base
    - Indicator variables for :math:`x_e > 0` for the non-input/output edges.
  * - :py:data:`~hyperflow.vars.isBothReverseUsed` / :cpp:var:`isBothReverseUsed <hyperflow::vars::isBothReverseUsed>`
    - Edge
    - Base
    - For each unique unordered pair of edges (not input/output)
      :math:`e_a = (S_a, T_a), e_b = (T_a, S_a)` there is an indicator variable indicating
      :math:`x_{e_a} > 0 \wedge x_{e_b} > 0`.
      The indexed specifier can only be retrieved if the given
      :cpp:class:`dg::DG::HyperEdge`/:py:class:`DG.HyperEdge` actually has an inverse.
      Two edges being each others inverse maps to the *same* indexed indicator variable.

  * - :py:data:`~hyperflow.vars.isOverallAutocata` / :cpp:var:`isOverallAutocata <hyperflow::vars::isOverallAutocata>`
    - Vertex
    - :ref:`Overall Autocatalysis <flowCommon-overallAutocata>`
    - Indicator variables for overall autocatalysis.
      When listing a solution the column header for this variable is ``OA``.

  * - :py:data:`~hyperflow.vars.isOverallCata` / :cpp:var:`isOverallCata <hyperflow::vars::isOverallCata>`
    - Vertex
    - :ref:`Overall Catalysis <flowCommon-overallCata>`
    - Indicator variables for overall catalysis.
      When listing a solution the column header for this variable is ``OC``.


Linear Expressions
------------------

When creating a model or querying solutions it can be useful to form linear
expressions or constraints, where variable specifiers acts as the variables.
The operators ``+``, ``-``, and ``*`` are overloaded on variable specifiers
and linear expressions (:py:class:`hyperflow.LinExp` and :cpp:class:`hyperflow::LinExp`),
such that you can build linear expressions as if they were normal expressions.
Examples:

- The variable specifier ``inFlow`` is a linear expression.
- So is ``inFlow[A]``.
- The expression ``2 * inFlow + edgeFlow[B]`` is a linear expression.
- So is ``2 * (inFlow + edgeFlow[B]) + outFlow[A]``.

Generally the following expressions create linear expressions:

- A named variable specifier.
- An indexed named variable specifier.
- Addition or subtraction of two linear expressions.
- Multiplication of an integer or floating-point number with a linear expression.
- Negation of a linear expression.
- A default-constructed :py:class:`hyperflow.LinExp`/:cpp:class:`hyperflow::LinExp`
  represents the linear expression of value 0.

The last case can be quite useful if you need to dynamically build a linear expression,
e.g., the sum of all ``inFlow`` of vertices satisfying some predicate:

.. code-block:: python

	expr = hyperflow.LinExp()
	for v in flow.dg.vertices:
		if myPredicate(v):
			expr += inFlow[v]


Linear Constraints
------------------

Similar to how linear expressions can be created using ordinary operators, so
can linear constraints.
They are created by using one of the operators ``==``, ``<=``, and ``>=`` with a linear
expression on one side and either an integer or floating-point number on the other side.
The constraints can be pased to
:py:meth:`hyperflow.Model.addConstraint`/:cpp:func:`hyperflow::Model::addConstraint`,
as shown in the example above.


Objective Function
##################

When solutions are found to the specified model, they are enumerated in order of optimality, with respect to the objective function.
It is set by giving a linear expression to :py:attr:`hyperflow.Model.objectiveFunction`/:cpp:func:`hyperflow::Model::setObjectiveFunction`,
and a solution is then considered better than another if it has a *lower*
objective value, i.e., the objective function is minimized by the internal solver.
So, if you want to maximize instead, simply negate your linear expression.

If no objective function is set, a default is determined by the enabled modules.
Each module will add some linear expression to the objective function if it is enabled.
If the objective function is explicitly specified by the user,
it is not modified by the modules.

.. list-table::
  :header-rows: 1

  * - Module
    - Added Linear Expression
  * - Base
    - ``edgeFlow`` :math:`+` ``inFlow``
  * - :ref:`Overall Autocatalysis <flowCommon-overallAutocata>`
    - ``isOverallAutocata``
  * - :ref:`Overall Catalysis <flowCommon-overallCata>`
    - ``isOverallCata``


Extending with Additional Variables
###################################

The base model and the extension modules each define one or more sets of variables.
You can also extend the model by addding additional variables, either boolean, integer, or floating-point variables.
This is done through
:py:meth:`hyperflow.Model.addBoolVariable` / :cpp:func:`hyperflow::Model::addBoolVariable`,
:py:meth:`hyperflow.Model.addIntVariable` / :cpp:func:`hyperflow::Model::addIntVariable`, and
:py:meth:`hyperflow.Model.addFloatVariable` / :cpp:func:`hyperflow::Model::addFloatVariable`, respectively.
Each of these functions need a unique name for the variable to be created, and will then return a variable specifier
(specifically an object of type :py:class:`hyperflow.VarCustom`/:cpp:class:`hyperflow::VarCustom`),
which can then be used as any of the variable specifiers from the built-in modules.


.. _flowCommon-overallAutocata:

Overall Autocatalysis Module
############################

Access: :py:attr:`~hyperflow.Model.overallAutocatalysis` (Python) / :cpp:var:`overallAutocatalysis <hyperflow::Model::overallAutocatalysis>` (C++)

If you use the overall autocatalysis module in your research you may want to cite [AFMS-Hyperflows]_ and [AFMS-Autocata]_.

This module adds one type of model of autocatalysis which requires a certain relation between the input and output flow,
i.e., the model adds a requirement on the overall reaction each flow solution implements.

Specifically, the module introduces the variable specifier ``isOverallAutocata`` which represents a set of indicator variables;
:math:`z^a_v` for each vertex :math:`v\in V`.
Each variable is constrained such that :math:`z^a_v` is 1 if and only if :math:`0 < f(e^{in}_v) < f(e^{out}_v`.
That is, a vertex is considered overall autocatalytic if it has non-zero in-flow, and has greater out-flow than in-flow.

When the module is enabled it will disable reversal of flow through pairs of reverse hyperedges
(:cpp:func:`hyperflow::Model::setAllowReversal` / :py:attr:`hyperflow.Model.allowReversal`),
including the IO edges
(:cpp:func:`hyperflow::Model::setAllowIOReversal` / :py:attr:`hyperflow.Model.allowIOReversal`).


Breadth-First Exclusivity
-------------------------

The core constraints of the module mere requires that some compound is consumed and then produced in a higher quantity.
In some cases, this can lead to surprising results, e.g., the one in the figure below.

.. tikz:: Example of an overall autocatalytic flow which is not really chemicall autocatalytic.
	The compound :math:`C` is the overall autocatalytic because it is put into
	the network once, and two copies are produced.

	\node[hnode] (A) {$A$};
	\node[hnode] (C) at ($(A) + (-20:4em)$) {$C$};
	\node[hnode] (B) at ($(C) + (180+20:4em)$) {$B$};
	\draw[edge] (A) to node[auto,above] {1} (C);
	\draw[edge] (C) to node[auto,below] {1} (B);
	\draw[edge] ($(A) + (-3em, 0)$) to node[above]{2} (A);
	\draw[edge] (B) to node[above]{1} ($(B) + (-3em, 0)$);
	\draw[edge] ($(C) + (20:3em)$) to node[auto,above]{1} (C);
	\draw[edge] (C) to node[auto,below]{2} ($(C) + (-20:3em)$);

One way to rule this flow out is to refine autocatalysis to define that the autocatalytic compound must
be *exclusively* overall autocatalytic, and there can not be a way to create it without it self being put in.
That is, let :math:`F\subseteq V` be the set of vertices with allowed input flow, then for each :math:`v\in F`,
it is not allowed to be overall autocatalytic if it can be reached by a hyper-breadth-first traversal of the
network, starting from :math:`F\backslash \{v\}`.
Equivalently, consider a flow model where all vertices are allowed to have output flow, and only vertices in
:math:`F\backslash \{v\}` are allowed to have input flow, if there exists a flow which has output flow from
:math:`v`, then :math:`v` is not allowed to be overall autocatalytic in the original model.

This pre-computation to disallow vertices to be overall autocatalytic is enabled by default,
but can be disabled with
:cpp:func:`hyperflow::Model::OverallAutocatalysis::setBFSExclusive` /
:py:attr:`hyperflow.Model.OverallAutocatalysis.bfsExclusive`,
which, depending on the use-case, in particular is relevant in cases where the
set of source compounds is very large.

Note, the pre-computation is not affected by custom constraints.
If a vertex should not be considered part of the network for this pre-computation,
use :cpp:func:`hyperflow::Model::exclude` / :py:meth:`hyperflow.Model.exclude`.


Strict Transit
--------------

The core constraints of the module mere requires that some compound is consumed and then produced in a higher quantity.
However, that does not mean that such a compound can not servce as an intermediary compound as well, which can be considered misleadning.
An example of such a flow is shown in the figure below.

.. tikz:: Example of an overall autocatalytic flow
	where an overall autocatalytic compound is also an intermediary compound.
	The shown network is the expanded network, and :math:`A`, is the overall
	autocatalytic compound. As :math:`A` also has transit flow from the network
	that goes into the network again, it is also an intermediary compound.

	\strictTransitCommon{1}{3}
	\draw[tedge] (t-A-in-CtoA) to node[below, pos=0.5] {1} (t-A-out-ABtoC);

We can create a related flow without any network-to-network transit flow by re-routing
the transit flow to become input and output instead.
This is illustrated in the figure below.

.. tikz:: A flow related to the one in the previous figure, with the same edge flow,
	but without the overall autocatalytic compound :math:`A` being an intermediary.

	\strictTransitCommon{2}{4}


By default the module adds constraints such that if a vertex :math:`v\in V` is
overall autocatalytic then it can not have any network-to-network transit flow,
all flow through :math:`v` must come from the input edge or go to the output edge.

The addition of these constraints can be disabled with
:cpp:func:`hyperflow::Model::OverallAutocatalysis::setStrictTransit` /
:py:attr:`hyperflow.Model.OverallAutocatalysis.strictTransit`.



.. _flowCommon-overallCata:

Overall Catalysis Module
########################

Access: :py:attr:`~hyperflow.Model.overallCatalysis` (Python) / :cpp:var:`overallCatalysis <hyperflow::Model::overallCatalysis>` (C++)

If you use the overall catalysis module in your research you may want to cite [AFMS-Hyperflows]_.

This module adds one type of model of catalysis which requires a certain relation between the input and output flow,
i.e., the model adds a requirement on the overall reaction each flow solution implements.

Specifically, the module introduces the variable specifier ``isOverallCata`` which represents a set of indicator variables;
:math:`z^c_v` for each vertex :math:`v\in V`.
Each variable is constrained such that :math:`z^c_v` is 1 if and only if :math:`0 < f(e^{in}_v) = f(e^{out}_v`.
That is, a vertex is considered overall atalytic if it has non-zero in-flow, and has the same out-flow as in-flow.

When the module is enabled it will disable reversal of flow through pairs of reverse hyperedges
(:cpp:func:`hyperflow::Model::setAllowReversal` / :py:attr:`hyperflow.Model.allowReversal`),
including the IO edges
(:cpp:func:`hyperflow::Model::setAllowIOReversal` / :py:attr:`hyperflow.Model.allowIOReversal`).


Strict Transit
--------------

Similarly to the :ref:`flowCommon-overallAutocata`, this module also has constraints for restricting the transit flow of overall catalytic vertices.

The strict transit constraints are enabled by default, but can be disabled with
:cpp:func:`hyperflow::Model::OverallCatalysis::setStrictTransit` /
:py:attr:`hyperflow.Model.OverallCatalysis.strictTransit`.
