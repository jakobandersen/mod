
.. py:currentmodule:: mod
.. cpp:namespace:: mod


.. _dgStrat:

****************************
Derivation Graph Strategies
****************************

The strategy framework is a domain specific programming language for
specifying the application of transformation rules.
It can therefore be used to perform computations with graphs and graph
rewriting.
During evaluation of a strategy the framework will remember each graph
derivation performed, and store them as a directed multi-hypergraph,
i.e., a derivation graph.
When graphs model molecules and transformation rules model reaction
patterns, then the resulting derivation graph can be seen as a chemical
reaction network.
Here we describe the semantics of the strategy language,
while the API is described seperately for the
:ref:`C++ <cpp-dg/Strategies>` and :ref:`Python <py-dg/Strategies>` interface.


---------------------
Virtual Machine
---------------------

The strategies are evaluated in a virtual machine.
Its state is a pair :math:`F = (\mathcal{U}, \mathcal{S})`
of sets of graphs, where :math:`\mathcal{U}` is called the `universe`.
The set :math:`\mathcal{S}` is a distinguished subset of the universe
called the `active subset`.
The machine additionally keeps track of a directed multi-hypergraph
:math:`\mathcal{H} = (V, E)`, called a `derivation graph`.
The vertices of :math:`\mathcal{H}` each has an associated graph,
while each hyperedge has an associated list of rules.

Initially the state is empty, i.e.,
:math:`(\mathcal{U}, \mathcal{S}) = (\emptyset, \emptyset)`
and the derivation graph is the empty hypergraph.


------------------
Strategies
------------------

Each strategy is a function taking a graph state :math:`F = (\mathcal{U}, \mathcal{S})` as
input and returning a new state :math:`F' = (\mathcal{U}', \mathcal{S}')`.
While the universe and active subset of a state are described as sets of graphs,
they are implemented as lists of unique graphs.
The result of most strategies do not depend on the order of the graphs and make no guarentees
about the order in the results.


.. _strat-addUniverse:

Add Universe
############

Given a single graph, a set of graphs, or a function returning a set of graphs,
this strategy returns the graph state with the additional graphs added to the universe.
That is, if :math:`\mathcal{G}` is the set of graphs to be added,
then the result is :math:`(\mathcal{U}\cup \mathcal{G}, \mathcal{S})`.


.. _strat-addSubset:

Add Subset
##########

This strategy is analogous to the previous strategy,
except the graphs are added to both the universe and the active subset.
That is, the result is :math:`(\mathcal{U}\cup \mathcal{G}, \mathcal{S}\cup \mathcal{G})`.


.. _strat-execute:

Execute
#######

The `execute` strategy is simply the identity function, but it can be used to execute arbitrary code
at a given point during evalutation.


.. _strat-filterUniverse:

Filter Universe
###############

This strategy can be used to remove graphs from the input state, using a given predicate.
Assuming this predicate is :math:`p`, then the result is :math:`F' = (\mathcal{U}', \mathcal{S}')`,
with :math:`\mathcal{U}' = \{g\in \mathcal{U}\mid p(g)\}`
and :math:`\mathcal{S}' = \{g\in \mathcal{S}\mid p(g)\}`.


.. _strat-filterSubset:

Filter Subset
#############

As the previous strategy this one also filters the input state, but only the active subset.
The result is thus :math:`F' = (\mathcal{U}, \mathcal{S}')`,
with :math:`\mathcal{S}' = \{g\in \mathcal{S}\mid p(g)\}`.


.. _strat-rule:

Rule
####

A rule (:cpp:class:`C++ <rule::Rule>`/:py:class:`Python <Rule>`) can be used directly as a strategy.
It will search for proper derivations using a multiset of graphs drawn from the input unvierse.
However, each candidate multiset will have at least one graph from the active subset.
The active subset of the output will be comprised of the newly discovered graphs.
That is, if :math:`D = \{G\Rightarrow^{p} H\mid G\subseteq \mathcal{U} \wedge G\cap \mathcal{S} \neq \emptyset\}`
is the set of all proper derivations using at least one graph from the active input subset,
then the result is :math:`F' = (\mathcal{U}', \mathcal{S}')` with
:math:`\mathcal{S}' = \bigcup_{G\Rightarrow^{p} H\in D} H\backslash \mathcal{U}`,
and :math:`\mathcal{U}' = \mathcal{U}\cup \mathcal{S}'`.

As a side-effect of evaluating a rule strategy the underlying derivation graph is augmented with vertices for every new graph discovered.
The derivations in :math:`D` is additionally added as directed multi-hyperedges in the graph.


.. _strat-leftPredicate:
.. _strat-rightPredicate:

Derivation Predicates
#####################

A derivation predicate strategy changes the execution environment for a given substrategy :math:`Q`.
Whenever a derivation is discovered during the evaluation of :math:`Q` a predicate :math:`p` will be consulted before
the derivation is finally accepted.
There are two flavours of the derivation predicate strategy: left predicate and right predicate.
The difference between them is that only the left-hand side of a potential derivation and the rule is available in the left predicate,
while the whole derivation is available in the right predicate.
Left predicatates are thus not strictly necessary, but can potentially be slightly more efficient than right predicates.


.. _strat-parallel:

Parallel
########

A `parallel` strategy aggregates a set of substrategies :math:`\{Q_1, Q_2, \dots, Q_n\}` and evaluates them on the same input state.
This evaluation produces a set of output states :math:`\{F_1', F_2', \dots, F_n'\}`, and the final result is the union of those states:
:math:`\mathcal{U}' = \bigcup_{1\leq i\leq n} \mathcal{U}_i'`,
:math:`\mathcal{S}' = \bigcup_{1\leq i\leq n} \mathcal{S}_i'`.


.. _strat-sequence:

Sequence
########

Given two substrategies :math:`Q_1` and :math:`Q_2`, the sequence strategy evaluates the composition of the strategies, i.e.,
with the input state :math:`F` the output is :math:`Q_2(Q_1(F))`.


.. _strat-repeat:

Repeat
######

The repetition strategy acts as a loop that evaluates a given substrategy :math:`Q` in sequence with it self a certain number of times.
Let :math:`Q^k(F)` be the :math:`k`-fold composition of the strategy :math:`Q` on the input state :math:`F`.
Notably, for :math:`k = 0` we have the identity function.
Given a constant :math:`n\geq 0`, the repeatition strategy on :math:`Q` results in :math:`F' = Q`k(F)`,
where :math:`k = min\{0, 1, \dots, n\}` such that either
:math:`k = n`, or
:math:`Q^{k+1}(F) = Q^{k}(F)`, or
:math:`Q^{k+1}(F) = (\emptyset, \overline{\mathcal{U}})`
for an abitrary universe :math:`\overline{\mathcal{U}}`.


.. _strat-revive:

Revive
######

A revive strategy is manipulating the output of an inner strategy :math:`Q`, depending on which derivations are discovered by :math:`Q`.
Let :math:`F = (\mathcal{U}, \mathcal{S})` be the input state and :math:`\overline{F} = (\overline{\mathcal{U}}, \overline{\mathcal{S}}) = Q(F)`.
Further, let :math:`D` be the set of derivations discovered (and accepted by the predicates) in the evaluation of :math:`Q(F)`.
We then define the set of `consumed` graphs as those being on the right side on any derivation in :math:`D`: :math:`C = \bigcup_{G\Rightarrow H \in D} G`.
As output of the revive strategy we do not modify the universe, i.e., :math:`\mathcal{U}' = \overline{\mathcal{U}}`.
The output subset is however extended by non-consumed graphs that were in the input subset:
:math:`\mathcal{S}' = \overline{\mathcal{S}}\cup \mathcal{S}\backslash C`.

