
import collections.abc
import ctypes
import inspect
import sys
from typing import (
	Any, Callable, cast, Generic, Iterable, List, Optional, Sequence,
	TextIO, Tuple, Type, TypeVar, Union
)

_redirected = False

_oldFlags = sys.getdlopenflags()
sys.setdlopenflags(_oldFlags | ctypes.RTLD_GLOBAL)
from . import libpymod
from .libpymod import *
from . import post
sys.setdlopenflags(_oldFlags)

# from http://mail.python.org/pipermail/tutor/2003-November/026645.html
class _Unbuffered:
	def __init__(self, stream: TextIO) -> None:
		self.stream = stream
	def write(self, data: Any) -> None:
		self.stream.write(data)
		self.stream.flush()
	def __getattr__(self, attr: str) -> Any:
		return getattr(self.stream, attr)
sys.stdout = _Unbuffered(sys.stdout)  # type: ignore

def _NoNew__setattr__(self: Any, name: str, value: Any) -> None:
	if hasattr(self, "_frozen") and self._frozen:
		msg = "Can not modify object '%s' of type '%s'. It has been frozen." % (self, type(self))
		raise AttributeError(msg)
	if hasattr(self, name):
		object.__setattr__(self, name, value)
	else:
		msg = "Can not create new attribute '%s' on object '%s' of type '%s'." % (name, self, type(self))
		msg += "\ndir(" + str(self) + "):\n"
		for name in dir(self): msg += "\t" + name + "\n"
		raise AttributeError(msg)


def _fixClass(name: str, c: Any, indent: int) -> None:
	if not name.startswith("_Func_"):
		c.__setattr__ = _NoNew__setattr__

	if name.startswith("_Func_") or name.startswith("_Vec"):
		c.__hash__ = None
	elif name.endswith("Vertex"):
		assert c.__hash__ is not None and c.__hash__ != object.__hash__
	elif issubclass(c, int):
		# Enums fall into this.
		assert c.__hash__ != object.__hash__
	elif c.__hash__ == object.__hash__:
		c.__hash__ = None

	if not (name.startswith("_Func_") or name.startswith("_Vec")):
		if name.endswith("Vertex") or name.endswith("Edge"):
			assert c.__bool__ is not None

	for a in inspect.getmembers(c, inspect.isclass):
		if a[0] == "__class__": continue
		_fixClass(a[0], a[1], indent + 1)

def _fixModule(modObj):
	classes = inspect.getmembers(modObj, inspect.isclass)
	for c in classes:
		_fixClass(c[0], c[1], 0)

_fixModule(libpymod)
_fixModule(post)

#----------------------------------------------------------

def _deprecation(msg: str) -> None:
	if config.common.ignoreDeprecation:
		print("WARNING: {} Use config.common.ignoreDeprecation = False to make this an exception.".format(msg))
	else:
		raise DeprecationWarning("{} Use config.common.ignoreDeprecation = True to make this just a warning.".format(msg))

#----------------------------------------------------------
# Script Inclusion Support
#----------------------------------------------------------

class CWDPath:
	def __init__(self, f: str) -> None:
		self.f = f

_filePrefixes: List[str] = []

def prefixFilename(name: str) -> str:
	if isinstance(name, CWDPath):
		return name.f
	if len(name) == 0 or name[0] == '/':
		return name
	prefixed: List[str] = []
	for s in _filePrefixes:
		if len(s) != 0 and s[0] == '/':
			prefixed[:] = [s]
		else:
			prefixed.append(s)
	prefixed.append(name)
	return ''.join(prefixed)

def pushFilePrefix(prefix: str) -> None:
	_filePrefixes.append(prefix)

def popFilePrefix() -> None:
	if len(_filePrefixes) == 0:
		raise LogicError("popFilePrefix failed; the stack is empty")
	_filePrefixes.pop()


#----------------------------------------------------------
# Wrappers
#----------------------------------------------------------

def _wrap(C: Type["Vec[T]"], l: Union["Vec[T]", Iterable["T"]]) -> "Vec[T]":
	if isinstance(l, C):
		return l
	lcpp = C()
	lcpp.extend(l)
	return lcpp
def _unwrap(lcpp: Iterable["T"]) -> List["T"]:
	l: List[T] = []
	l.extend(lcpp)
	return l

def _funcWrap(F: Type["U"], f,
		resultWrap: Optional[Type["Vec[T]"]] = None,
		module: Any=libpymod) -> "U":
	if hasattr(f, "__call__"):
		class FuncWrapper(F):  # type: ignore
			def __init__(self, f) -> None:
				self.f = f
				F.__init__(self)
			def clone(self) -> "FuncWrapper":
				return module._sharedToStd(FuncWrapper(self.f))
			def __str__(self) -> str:
				lines, lnum = inspect.getsourcelines(self.f)
				source = ''.join(lines)
				filename = inspect.getfile(self.f)
				return "FuncWrapper(%s)\nCode from %s:%d >>>>>\n%s<<<<< Code from %s:%d" % (str(self.f), filename, lnum, source, filename, lnum)
			def __call__(self, *args: List[Any]) -> Union["T", "Vec[T]"]:
				try:
					if resultWrap is not None:
						return _wrap(resultWrap,
							cast(Iterable["T"], self.f(*args)))
					else:
						return cast("T", self.f(*args))
				except:
					print("Error in wrapped function when called:", str(self))
					print("Base type is '" + str(F) + "'")
					raise
										
		res = FuncWrapper(f)
	else:							# assume constant
		class Constant(F):  # type: ignore
			def __init__(self, c: "T") -> None:
				self.c = c
				F.__init__(self)
			def clone(self) -> "Constant":
				return module._sharedToStd(Constant(self.c))
			def __str__(self) -> str:
				return "Constant(" + str(self.c) + ")"
			def __call__(self, *args: List[Any]) -> "T":
				return self.c
		res = Constant(cast("T", f))
	return module._sharedToStd(res)


#----------------------------------------------------------
# Common stuff
#----------------------------------------------------------

_lsString = LabelSettings(LabelType.String, LabelRelation.Isomorphism)


###########################################################
# Chem
###########################################################

BondType.__str__ = libpymod._bondTypeToString  # type: ignore


###########################################################
# Config
###########################################################

LabelType.__str__ = libpymod._LabelType__str__  # type: ignore
LabelRelation.__str__ = libpymod._LabelRelation__str__  # type: ignore
IsomorphismPolicy.__str__ = libpymod._IsomorphismPolicy__str__  # type: ignore
SmilesClassPolicy.__str__ = libpymod._SmilesClassPolicy__str__  # type: ignore
Action.__str__ = libpymod._Action__str__  # type: ignore

config = getConfig()


###########################################################
# Derivation
###########################################################

Derivation.__repr__ = Derivation.__str__  # type: ignore

def _Derivation__setattr__(self: Derivation, name: str, value: Any) -> None:
	if name in ("left", "right"):
		object.__setattr__(self, name, _wrap(libpymod._VecGraph, value))
	else:
		_NoNew__setattr__(self, name, value)
Derivation.__setattr__ = _Derivation__setattr__  # type: ignore


Derivations.__repr__ = Derivations.__str__  # type: ignore

def _Derivations__setattr__(self: Derivation, name: str, value: Any) -> None:
	if name in ("left", "right"):
		object.__setattr__(self, name, _wrap(libpymod._VecGraph, value))
	elif name == "rules":
		object.__setattr__(self, name, _wrap(libpymod._VecRule, value))
	else:
		_NoNew__setattr__(self, name, value)
Derivations.__setattr__ = _Derivations__setattr__  # type: ignore


###########################################################
# DG
###########################################################

def dgDerivations(ders: Iterable[Derivation]) -> DG:
	_deprecation("dgDerivations is deprecated. Use the new build interface.")
	dg = DG()
	with dg.build() as b:
		for d in ders:
			b.addDerivation(d)
	return dg

def dgRuleComp(graphs: Iterable[Graph], strat: DGStrat,
		labelSettings: LabelSettings=_lsString,
		ignoreRuleLabelTypes: bool=False) -> DG:
	_deprecation("dgRuleComp is deprecated. Use the new build interface.")
	dg = DG(labelSettings=labelSettings, graphDatabase=graphs)
	object.__setattr__(dg, "_ruleCompData", {
		"ignoreRuleLabelTypes": ignoreRuleLabelTypes,
		"strat": strat
	})
	def _DG_calc(dg: DG, printInfo: bool=True) -> None:
		_deprecation("DG.calc() is deprecated. Use the new build interface.")
		d = dg._ruleCompData  # type: ignore
		dg.build().execute(d["strat"], ignoreRuleLabelTypes=d["ignoreRuleLabelTypes"])
		object.__setattr__(dg, "_ruleCompData", None)
		object.__setattr__(dg, "calc", None)
	import types
	object.__setattr__(dg, "calc", types.MethodType(_DG_calc, dg))
	return dg

_DG_load_orig = DG.load
def _DG_load(
		graphDatabase: List[Graph], ruleDatabase: List[Rule], f: str,
		graphPolicy: IsomorphismPolicy = IsomorphismPolicy.Check,
		verbosity: int = 2) -> DG:
	return _DG_load_orig(
		_wrap(libpymod._VecGraph, graphDatabase),
		_wrap(libpymod._VecRule, ruleDatabase),
		prefixFilename(f), graphPolicy, verbosity)
DG.load = _DG_load  # type: ignore

_DG__init__old = DG.__init__
def _DG__init__(self: DG, *,
		labelSettings: LabelSettings=_lsString,
		graphDatabase: List[Graph] = [],
		graphPolicy: IsomorphismPolicy = IsomorphismPolicy.Check) -> None:
	return _DG__init__old(self,  # type: ignore
	                      labelSettings,
	                      _wrap(libpymod._VecGraph, graphDatabase),
	                      graphPolicy)
DG.__init__ = _DG__init__  # type: ignore

_DG_print_orig = DG.print
def _DG_print(self: DG, printer: Optional[DGPrinter] = None, data: Optional[DGPrintData] = None) -> Tuple[str, str]:
	if printer is None: printer = DGPrinter()
	if data is None: data = DGPrintData(self)
	return _DG_print_orig(self, printer, data)
DG.print = _DG_print  # type: ignore

_DG_findEdge_orig = DG.findEdge
def _DG_findEdge(self: DG,
		srcsI: Union[Sequence[Graph], Sequence[DG.Vertex]],
		tarsI: Union[Sequence[Graph], Sequence[DG.Vertex]]) -> DG.HyperEdge:
	srcs = srcsI
	tars = tarsI

	s: Union[None, Type[libpymod._VecGraph], Type[libpymod._VecDGVertex]]
	t: Union[None, Type[libpymod._VecGraph], Type[libpymod._VecDGVertex]]

	if len(srcs) == 0:
		s = None
	elif isinstance(srcs[0], Graph):
		s = libpymod._VecGraph
	else:
		s = libpymod._VecDGVertex

	if len(tars) == 0:
		t = None
	elif isinstance(tars[0], Graph):
		t = libpymod._VecGraph
	else:
		t = libpymod._VecDGVertex

	if s is None and t is None:
		s = libpymod._VecDGVertex
		t = libpymod._VecDGVertex
	elif s is None:
		s = t
	elif t is None:
		t = s
	return _DG_findEdge_orig(self, _wrap(s, srcs), _wrap(t, tars))  # type: ignore
DG.findEdge = _DG_findEdge  # type: ignore

DG.__repr__ = DG.__str__  # type: ignore

def _DG__getattribute__(self: DG, name: str) -> Any:
	if name == "graphDatabase":
		return _unwrap(self._graphDatabase)  # type: ignore
	elif name == "products":
		return _unwrap(self._products)  # type: ignore
	else:
		return object.__getattribute__(self, name)
DG.__getattribute__ = _DG__getattribute__  # type: ignore

DG.__eq__ = lambda self, other: self.id == other.id  # type: ignore
DG.__hash__ = lambda self: self.id  # type: ignore


class DGBuildContextManager:
	_builder: Optional[DG.Builder]

	def __init__(self, dg: DG, onNewVertex, onNewHyperEdge) -> None:
		assert dg is not None
		self._builder = _DG_build_orig(dg,
			None if onNewVertex is None else _funcWrap(libpymod._Func_VoidDGVertex, onNewVertex),
			None if onNewHyperEdge is None else _funcWrap(libpymod._Func_VoidDGHyperEdge, onNewHyperEdge)
		)

	def __enter__(self) -> "DGBuildContextManager":
		return self

	def __exit__(self, exc_type, exc_val, exc_tb) -> None:
		del self._builder
		self._builder = None

	@property
	def dg(self) -> DG:
		assert self._builder
		return self._builder.dg

	@property
	def isActive(self) -> bool:
		assert self._builder
		return self._builder.isActive

	def addDerivation(self, d: Derivations,
			graphPolicy: IsomorphismPolicy = IsomorphismPolicy.Check) -> DG.HyperEdge:
		assert self._builder
		return self._builder.addDerivation(d, graphPolicy)

	def addHyperEdge(self, e: DG.HyperEdge,
			graphPolicy: IsomorphismPolicy = IsomorphismPolicy.Check) -> DG.HyperEdge:
		assert self._builder
		return self._builder.addHyperEdge(e, graphPolicy)

	def execute(self, strategy: DGStrat, *, verbosity: int=2, ignoreRuleLabelTypes: bool=False) -> DG.Builder.ExecuteResult:
		assert self._builder
		return self._builder.execute(dgStrat(strategy), verbosity, ignoreRuleLabelTypes)  # type: ignore
	
	def apply(self, graphs: Iterable[Graph], rule: Rule, onlyProper: bool=True, verbosity: int=0,
			graphPolicy: IsomorphismPolicy=IsomorphismPolicy.Check) -> List[DG.HyperEdge]:
		assert self._builder
		return _unwrap(self._builder.apply(_wrap(libpymod._VecGraph, graphs), rule, onlyProper, verbosity, graphPolicy))

	def addAbstract(self, description: str) -> None:
		assert self._builder
		return self._builder.addAbstract(description)

	def load(self, ruleDatabase: List[Rule], f: str, verbosity: int = 2) -> None:
		assert self._builder
		return self._builder.load(
			_wrap(libpymod._VecRule, ruleDatabase),
			prefixFilename(f), verbosity)

_DG_build_orig = DG.build
DG.build = lambda self, *, onNewVertex=None, onNewHyperEdge=None: DGBuildContextManager(self, onNewVertex, onNewHyperEdge)  # type: ignore


#----------------------------------------------------------
# DG.Builder.ExecuteResult
#----------------------------------------------------------

def _DGExecuteResult__getattribute__(self: DG.Builder.ExecuteResult, name: str) -> Any:
	if name in ("subset", "universe"):
		return _unwrap(object.__getattribute__(self, name))
	return object.__getattribute__(self, name)

DG.Builder.ExecuteResult.__getattribute__ = _DGExecuteResult__getattribute__  # type: ignore

_DGExecuteResult_list_orig = DG.Builder.ExecuteResult.list
def _DGExecuteResult_list(self: DG.Builder.ExecuteResult, *, withUniverse: bool=False) -> None:
	return _DGExecuteResult_list_orig(self, withUniverse)  # type: ignore
DG.Builder.ExecuteResult.list = _DGExecuteResult_list  # type: ignore


#----------------------------------------------------------
# DG.HyperEdge
#----------------------------------------------------------

_DGHyperEdge_print_orig = DG.HyperEdge.print
DG.HyperEdge.print = lambda self, *args, **kwargs: _unwrap(_DGHyperEdge_print_orig(self, *args, **kwargs))  # type: ignore


#----------------------------------------------------------
# DGPrinter
#----------------------------------------------------------

def _makeGraphToVertexCallback(orig, name, func):
	def callback(self, f, *args, **kwargs):
		if hasattr(f, "__call__"):
			import inspect
			spec = inspect.getfullargspec(f)
			if len(spec.args) == 2:
				_deprecation("The callback for {} seems to take two arguments, a graph and a derivation graph. This is deprecated, the callback should take a single DG.Vertex argument.".format(name))
				fOrig = f
				f = lambda v, fOrig=fOrig: fOrig(v.graph, v.dg)
		return orig(self, _funcWrap(func, f), *args, **kwargs)
	return callback

_DGPrinter_pushVertexVisible_orig = DGPrinter.pushVertexVisible
DGPrinter.pushVertexVisible = _makeGraphToVertexCallback(  # type: ignore
	_DGPrinter_pushVertexVisible_orig, "pushVertexVisible", libpymod._Func_BoolDGVertex)

_DGPrinter_pushEdgeVisible_orig = DGPrinter.pushEdgeVisible
DGPrinter.pushEdgeVisible = (  # type: ignore
	lambda self, f: _DGPrinter_pushEdgeVisible_orig(self, _funcWrap(libpymod._Func_BoolDGHyperEdge, f)))

_DGPrinter_pushVertexLabel_orig = DGPrinter.pushVertexLabel
DGPrinter.pushVertexLabel = _makeGraphToVertexCallback(  # type: ignore
	_DGPrinter_pushVertexLabel_orig , "pushVertexLabel", libpymod._Func_StringDGVertex)

_DGPrinter_pushEdgeLabel_orig = DGPrinter.pushEdgeLabel
DGPrinter.pushEdgeLabel = (  # type: ignore
	lambda self, f: _DGPrinter_pushEdgeLabel_orig(self, _funcWrap(libpymod._Func_StringDGHyperEdge, f)))

_DGPrinter_pushVertexColour_orig = DGPrinter.pushVertexColour
_DGPrinter_pushVertexColour_inner = _makeGraphToVertexCallback(
	_DGPrinter_pushVertexColour_orig, "pushVertexColour", libpymod._Func_StringDGVertex)
DGPrinter.pushVertexColour = (  # type: ignore
	lambda self, f, extendToEdges=True: _DGPrinter_pushVertexColour_inner(self, f, extendToEdges))

_DGPrinter_pushEdgeColour_orig = DGPrinter.pushEdgeColour
DGPrinter.pushEdgeColour = (  # type: ignore
	lambda self, f: _DGPrinter_pushEdgeColour_orig(self, _funcWrap(libpymod._Func_StringDGHyperEdge, f)))

_DGPrinter_setRotationOverwrite_orig = DGPrinter.setRotationOverwrite
DGPrinter.setRotationOverwrite = (  # type: ignore
	lambda self, f: _DGPrinter_setRotationOverwrite_orig(self, _funcWrap(libpymod._Func_IntGraph, f)))

_DGPrinter_setMirrorOverwrite_orig = DGPrinter.setMirrorOverwrite
DGPrinter.setMirrorOverwrite = (  # type: ignore
	lambda self, f: _DGPrinter_setMirrorOverwrite_orig(self, _funcWrap(libpymod._Func_BoolGraph, f)))

_DGPrinter_setImageOverwrite_orig = DGPrinter.setImageOverwrite
def _DGPrinter_setImageOverwrite(self, f):
	if f is None:
		wrapped = None
	else:
		wrapped = _funcWrap(
			libpymod._Func_PairStringStringDGVertexInt, f)
	return _DGPrinter_setImageOverwrite_orig(self, wrapped)
DGPrinter.setImageOverwrite = _DGPrinter_setImageOverwrite  # type: ignore


#----------------------------------------------------------
# Strategy
#----------------------------------------------------------

def _DGStratGraphState__getattribute__(self: DGStrat.GraphState, name: str) -> Any:
	if name == "subset":
		return _unwrap(self._subset)  # type: ignore
	elif name == "universe":
		return _unwrap(self._universe)  # type: ignore
	else:
		return object.__getattribute__(self, name)
DGStrat.GraphState.__getattribute__ = _DGStratGraphState__getattribute__  # type: ignore

_DGStrat_makeAddStatic_orig = DGStrat.makeAddStatic
def _DGStrat_makeAddStatic(onlyUniverse: bool, graphs: Iterable[Graph], graphPolicy: IsomorphismPolicy) -> DGStrat:
	return _DGStrat_makeAddStatic_orig(onlyUniverse, _wrap(libpymod._VecGraph, graphs), graphPolicy)
DGStrat.makeAddStatic = _DGStrat_makeAddStatic  # type: ignore

_DGStrat_makeAddDynamic_orig = DGStrat.makeAddDynamic
def _DGStrat_makeAddDynamic(onlyUniverse: bool, generator: Callable[[], List[Graph]], graphPolicy: IsomorphismPolicy) -> DGStrat:
	return _DGStrat_makeAddDynamic_orig(onlyUniverse, _funcWrap(libpymod._Func_VecGraph, generator, resultWrap=libpymod._VecGraph), graphPolicy)
DGStrat.makeAddDynamic = _DGStrat_makeAddDynamic  # type: ignore

_DGStrat_makeSequence_orig = DGStrat.makeSequence
def _DGStrat_makeSequence(l: Iterable[DGStrat]) -> DGStrat:
	return _DGStrat_makeSequence_orig(_wrap(libpymod._VecDGStrat, l))
DGStrat.makeSequence = _DGStrat_makeSequence  # type: ignore

_DGStrat_makeParallel_orig = DGStrat.makeParallel
def _DGStrat_makeParallel(l: Iterable[DGStrat]) -> DGStrat:
	return _DGStrat_makeParallel_orig(_wrap(libpymod._VecDGStrat, l))
DGStrat.makeParallel = _DGStrat_makeParallel  # type: ignore

_DGStrat_makeFilter_orig = DGStrat.makeFilter
def _DGStrat_makeFilter(alsoUniverse: bool, filterFunc: Callable[[Graph, DGStrat.GraphState, bool], bool]) -> DGStrat:
	return _DGStrat_makeFilter_orig(alsoUniverse, _funcWrap(libpymod._Func_BoolGraphDGStratGraphStateBool, filterFunc))
DGStrat.makeFilter = _DGStrat_makeFilter  # type: ignore

_DGStrat_makeExecute_orig = DGStrat.makeExecute
def _DGStrat_makeExecute(func: Callable[[DGStrat.GraphState], None]) -> DGStrat:
	return _DGStrat_makeExecute_orig(_funcWrap(libpymod._Func_VoidDGStratGraphState, func))
DGStrat.makeExecute = _DGStrat_makeExecute  # type: ignore

_DGStrat_makeLeftPredicate_orig = DGStrat.makeLeftPredicate
def _DGStrat_makeLeftPredicate(pred: Callable[[Derivation], bool], strat: DGStrat) -> DGStrat:
	return _DGStrat_makeLeftPredicate_orig(_funcWrap(libpymod._Func_BoolDerivation, pred), strat)
DGStrat.makeLeftPredicate = _DGStrat_makeLeftPredicate  # type: ignore

_DGStrat_makeRightPredicate_orig = DGStrat.makeRightPredicate
def _DGStrat_makeRightPredicate(pred: Callable[[Derivation], bool], strat: DGStrat) -> DGStrat:
	return _DGStrat_makeRightPredicate_orig(_funcWrap(libpymod._Func_BoolDerivation, pred), strat)
DGStrat.makeRightPredicate = _DGStrat_makeRightPredicate  # type: ignore


#----------------------------------------------------------
# DG Strategy Prettification
#----------------------------------------------------------

_DGStratType = Union[DGStrat, Rule, "_DGStrat_sequenceProxy", Iterable['_DGStratType']]

def dgStrat(s: _DGStratType) -> DGStrat:
	if isinstance(s, DGStrat):
		return s
	elif isinstance(s, Rule):
		return DGStrat.makeRule(s)
	elif isinstance(s, _DGStrat_sequenceProxy):
		return DGStrat.makeSequence(s.strats)
	elif isinstance(s, collections.abc.Iterable):
		# do deep dgStrat
		l = [dgStrat(a) for a in s]
		return DGStrat.makeParallel(l)	
	else:
		raise TypeError("Can not convert type '" + str(type(s)) + "' to DGStrat.")

# add
#----------------------------------------------------------

_DGStratAddStaticGraphType = Union[Graph, Iterable[Graph]]

def _DGStrat_add(doUniverse: bool, g: Union[_DGStratAddStaticGraphType, Callable[[], List[Graph]]],
		gs: Tuple[_DGStratAddStaticGraphType, ...], graphPolicy: IsomorphismPolicy) -> DGStrat:
	if hasattr(g, "__call__"): # assume the dynamic version is meant
		if len(gs) > 0:
			raise TypeError("The dynamic version of addSubset/addUniverse takes exactly 1 argument (" + str(len(gs) + 1) + " given).")
		return DGStrat.makeAddDynamic(doUniverse, cast(Callable[[], List[Graph]], g), graphPolicy)
	else: # assume the static version was meant
		def convertGraphs(graphs: List[Graph], g: Union[Graph, Iterable[Graph]]) -> None:
			if isinstance(g, Graph):
				graphs.append(g)
			else:
				graphs.extend(a for a in g)
		graphs = []  # type: List[Graph]
		convertGraphs(graphs, cast(Union[Graph, Iterable[Graph]], g))
		for a in gs:
			convertGraphs(graphs, a)
		return DGStrat.makeAddStatic(doUniverse, graphs, graphPolicy)
def addUniverse(g: _DGStratAddStaticGraphType , *gs: _DGStratAddStaticGraphType,
		graphPolicy: IsomorphismPolicy=IsomorphismPolicy.Check) -> DGStrat:
	return _DGStrat_add(True, g, gs, graphPolicy)
def addSubset(g: _DGStratAddStaticGraphType , *gs: _DGStratAddStaticGraphType,
		graphPolicy: IsomorphismPolicy=IsomorphismPolicy.Check) -> DGStrat:
	return _DGStrat_add(False, g, gs, graphPolicy)

# derivation predicates
#----------------------------------------------------------

class _DGStrat_DerivationPredicateProxyPredicateHolder:
	def __init__(self, isLeft: bool, predicate: Callable[[Derivation], bool]) -> None:
		self.isLeft = isLeft
		self.predicate = predicate
	def __call__(self, strat: DGStrat) -> DGStrat:
		if self.isLeft:
			return DGStrat.makeLeftPredicate(self.predicate, dgStrat(strat))
		else:
			return DGStrat.makeRightPredicate(self.predicate, dgStrat(strat))

class _DGStrat_DerivationPredicateProxy:
	def __init__(self, isLeft: bool):
		self.isLeft = isLeft
	def __getitem__(self, predicate: Callable[[Derivation], bool]) -> _DGStrat_DerivationPredicateProxyPredicateHolder:
		return _DGStrat_DerivationPredicateProxyPredicateHolder(self.isLeft, predicate)

leftPredicate = _DGStrat_DerivationPredicateProxy(True)
rightPredicate = _DGStrat_DerivationPredicateProxy(False)

# execute
#----------------------------------------------------------

execute = lambda func: DGStrat.makeExecute(func)

# filter
#----------------------------------------------------------

class _DGStrat_FilterProxy:
	def __init__(self, alsoUniverse: bool) -> None:
		self.alsoUniverse = alsoUniverse
	def __call__(self, filterFunc: Callable[[Graph, DGStrat.GraphState, bool], bool]) -> DGStrat:
		return DGStrat.makeFilter(self.alsoUniverse, filterFunc)

filterUniverse = _DGStrat_FilterProxy(True)
filterSubset = _DGStrat_FilterProxy(False)

# repeat
#----------------------------------------------------------

class _DGStrat_RepeatProxyBoundHolder:
	def __init__(self, bound: int) -> None:
		self.bound = bound
		if bound < 0:
			raise ArgumentError("The number of repetitions in a repeat strategy must be non-negative. Got '" + str(bound))  # type: ignore
	def __call__(self, strat: DGStrat) -> DGStrat:
		return DGStrat.makeRepeat(self.bound, dgStrat(strat))

class _DGStrat_RepeatProxy:
	def __getitem__(self, bound: int) -> _DGStrat_RepeatProxyBoundHolder:
		return _DGStrat_RepeatProxyBoundHolder(bound)
	def __call__(self, strat: DGStrat) -> DGStrat:
		return self[2**31 - 1](strat)
		
repeat = _DGStrat_RepeatProxy()

# revive
#----------------------------------------------------------

revive = lambda s: DGStrat.makeRevive(dgStrat(s))

# sequence
#----------------------------------------------------------

def _DGStrat_sequence__rshift__(a: "_DGStrat_sequenceProxy", b: DGStrat) -> "_DGStrat_sequenceProxy":
	strats = []  # type: List[DGStrat]
	if isinstance(a, _DGStrat_sequenceProxy):
		strats.extend(s for s in a.strats)
	else:
		strats.append(dgStrat(a))
	if isinstance(b, _DGStrat_sequenceProxy):
		strats.extend(s for s in b.strats)
	else:
		strats.append(dgStrat(b))
	return _DGStrat_sequenceProxy(strats)


class _DGStrat_sequenceProxy:
	def __init__(self, strats: List[DGStrat]) -> None:
		self.strats = strats

	def  __rshift__(self, other: DGStrat) -> "_DGStrat_sequenceProxy":
		return _DGStrat_sequence__rshift__(self, other)

DGStrat.__rshift__ = _DGStrat_sequence__rshift__  # type: ignore
DGStrat.__rrshift__ = lambda self, other: _DGStrat_sequence__rshift__(other, self)  # type: ignore
Rule.__rshift__ = _DGStrat_sequence__rshift__  # type: ignore


###########################################################
# Graph
###########################################################

inputGraphs = []

def _Graph__getattribute__(self, name):
	if name == "loadingWarnings":
		return _unwrap(object.__getattribute__(self, name))
	else:
		return object.__getattribute__(self, name)
Graph.__getattribute__ = _Graph__getattribute__  # type: ignore

_Graph_print_orig = Graph.print
def _Graph_print(self: Graph, first: Optional[GraphPrinter]=None, second: Optional[GraphPrinter]=None) -> Tuple[str, str]:
	if first is None:
		return _Graph_print_orig(self)
	if second is None:
		second = first
	return _Graph_print_orig(self, first, second)
Graph.print = _Graph_print  # type: ignore

_Graph_aut = Graph.aut
Graph.aut = lambda self, labelSettings=_lsString: _Graph_aut(self, labelSettings)  # type: ignore

_Graph_isomorphism = Graph.isomorphism
Graph.isomorphism = lambda self, g, maxNumMatches=1, labelSettings=_lsString: _Graph_isomorphism(self, g, maxNumMatches, labelSettings)  # type: ignore

_Graph_monomorphism = Graph.monomorphism
Graph.monomorphism = lambda self, g, maxNumMatches=1, labelSettings=_lsString: _Graph_monomorphism(self, g, maxNumMatches, labelSettings)  # type: ignore

_Graph_enumerateIsomorphisms = Graph.enumerateIsomorphisms  # type: ignore
Graph.enumerateIsomorphisms = lambda self, codomain, *, callback, labelSettings=_lsString: _Graph_enumerateIsomorphisms(  # type: ignore
	self, codomain, _funcWrap(libpymod._Func_BoolVertexMapGraphGraph, callback), labelSettings)  # type: ignore

_Graph_enumerateMonomorphisms = Graph.enumerateMonomorphisms  # type: ignore
Graph.enumerateMonomorphisms = lambda self, codomain, *, callback, labelSettings=_lsString: _Graph_enumerateMonomorphisms(  # type: ignore
	self, codomain, _funcWrap(libpymod._Func_BoolVertexMapGraphGraph, callback), labelSettings)  # type: ignore

_Graph_getGMLString = Graph.getGMLString
Graph.getGMLString = lambda self, withCoords=False: _Graph_getGMLString(self, withCoords)  # type: ignore
_Graph_printGML = Graph.printGML
Graph.printGML = lambda self, withCoords=False: _Graph_printGML(self, withCoords)  # type: ignore

# Loading
###########################################################

def _graphLoad(a: Graph, name: Optional[str], add: bool) -> Graph:
	if name is not None:
		a.name = name
	if add:
		inputGraphs.append(a)
	return a

def _graphsLoad(gs: List[Graph], add: bool) -> List[Graph]:
	us = _unwrap(gs)
	res = [_graphLoad(a, name=None, add=add) for a in us]
	return res

def _graphssLoad(gs: List[List[Graph]], add: bool) -> List[List[Graph]]:
	us = _unwrap(gs)
	res = [_graphsLoad(a, add=add) for a in us]
	return res

_Graph_fromGMLString_orig      = Graph.fromGMLString
_Graph_fromGMLFile_orig        = Graph.fromGMLFile
_Graph_fromGMLStringMulti_orig = Graph.fromGMLStringMulti
_Graph_fromGMLFileMulti_orig   = Graph.fromGMLFileMulti
_Graph_fromDFS_orig            = Graph.fromDFS
_Graph_fromDFSMulti_orig       = Graph.fromDFSMulti
_Graph_fromSMILES_orig         = Graph.fromSMILES
_Graph_fromSMILESMulti_orig    = Graph.fromSMILESMulti
_Graph_fromMOLString_orig      = Graph.fromMOLString
_Graph_fromMOLFile_orig        = Graph.fromMOLFile
_Graph_fromMOLStringMulti_orig = Graph.fromMOLStringMulti
_Graph_fromMOLFileMulti_orig   = Graph.fromMOLFileMulti
_Graph_fromSDString_orig       = Graph.fromSDString
_Graph_fromSDFile_orig         = Graph.fromSDFile
_Graph_fromSDStringMulti_orig  = Graph.fromSDStringMulti
_Graph_fromSDFileMulti_orig    = Graph.fromSDFileMulti

def _Graph_fromGMLString(     s: str, name: Optional[str] = None,                                     add: bool = True, printStereoWarnings: bool = True) -> Graph:
	return _graphLoad(_Graph_fromGMLString_orig(                   s         , printStereoWarnings), name, add)
def _Graph_fromGMLFile(       f: str, name: Optional[str] = None,                                     add: bool = True, printStereoWarnings: bool = True) -> Graph:
	return _graphLoad(_Graph_fromGMLFile_orig(      prefixFilename(f)        , printStereoWarnings), name, add)
def _Graph_fromGMLStringMulti(s: str,                                                                 add: bool = True, printStereoWarnings: bool = True) -> List[Graph]:
	return _graphsLoad(_Graph_fromGMLStringMulti_orig(             s         , printStereoWarnings),       add)
def _Graph_fromGMLFileMulti(  f: str,                                                                 add: bool = True, printStereoWarnings: bool = True) -> List[Graph]:
	return _graphsLoad(_Graph_fromGMLFileMulti_orig(prefixFilename(f)        , printStereoWarnings),       add)
def _Graph_fromDFS(           s: str, name: Optional[str] = None,                                     add: bool = True) -> Graph:
	return _graphLoad(_Graph_fromDFS_orig(                         s                              ), name, add)
def _Graph_fromDFSMulti(      s: str,                                                                 add: bool = True) -> List[Graph]:
	return _graphsLoad(_Graph_fromDFSMulti_orig(                   s                              ),       add)
def _Graph_fromSMILES(        s: str, name: Optional[str] = None, allowAbstract: bool = False, classPolicy: SmilesClassPolicy = SmilesClassPolicy.NoneOnDuplicate,
                                                                                                      add: bool = True, printStereoWarnings: bool = True) -> Graph:
	return _graphLoad(_Graph_fromSMILES_orig(                      s, allowAbstract, classPolicy, printStereoWarnings), name, add)
def _Graph_fromSMILESMulti(   s: str,                             allowAbstract: bool = False, classPolicy: SmilesClassPolicy = SmilesClassPolicy.NoneOnDuplicate,
                                                                                                      add: bool = True, printStereoWarnings: bool = True) -> List[Graph]:
	return _graphsLoad(_Graph_fromSMILESMulti_orig(                s, allowAbstract, classPolicy, printStereoWarnings),       add)
def _Graph_fromMOLString(     s: str, name: Optional[str] = None, options: MDLOptions = MDLOptions(), add: bool = True) -> Graph:
	return _graphLoad(_Graph_fromMOLString_orig(                   s,  options                    ), name, add)
def _Graph_fromMOLFile(       f: str, name: Optional[str] = None, options: MDLOptions = MDLOptions(), add: bool = True) -> Graph:
	return _graphLoad(_Graph_fromMOLFile_orig(      prefixFilename(f), options                    ), name, add)
def _Graph_fromMOLStringMulti(s: str,                             options: MDLOptions = MDLOptions(), add: bool = True) -> List[Graph]:
	return _graphsLoad(_Graph_fromMOLStringMulti_orig(             s,  options                    ),       add)
def _Graph_fromMOLFileMulti(  f: str,                             options: MDLOptions = MDLOptions(), add: bool = True) -> List[Graph]:
	return _graphsLoad(_Graph_fromMOLFileMulti_orig(prefixFilename(f), options                    ),       add)
def _Graph_fromSDString(      s: str,                             options: MDLOptions = MDLOptions(), add: bool = True) -> List[Graph]:
	return _graphsLoad(_Graph_fromSDString_orig(                   s,  options                    ),       add)
def _Graph_fromSDFile(        f: str,                             options: MDLOptions = MDLOptions(), add: bool = True) -> List[Graph]:
	return _graphsLoad(_Graph_fromSDFile_orig(      prefixFilename(f), options                    ),       add)
def _Graph_fromSDStringMulti( s: str,                             options: MDLOptions = MDLOptions(), add: bool = True) -> List[List[Graph]]:
	return _graphssLoad(_Graph_fromSDStringMulti_orig(             s,  options                    ),       add)
def _Graph_fromSDFileMulti(   f: str,                             options: MDLOptions = MDLOptions(), add: bool = True) -> List[List[Graph]]:
	return _graphssLoad(_Graph_fromSDFileMulti_orig(prefixFilename(f), options                    ),       add)

Graph.fromGMLString      = _Graph_fromGMLString  # type: ignore
Graph.fromGMLFile        = _Graph_fromGMLFile  # type: ignore
Graph.fromGMLStringMulti = _Graph_fromGMLStringMulti  # type: ignore
Graph.fromGMLFileMulti   = _Graph_fromGMLFileMulti  # type: ignore
Graph.fromDFS            = _Graph_fromDFS  # type: ignore
Graph.fromDFSMulti       = _Graph_fromDFSMulti  # type: ignore
Graph.fromSMILES         = _Graph_fromSMILES  # type: ignore
Graph.fromSMILESMulti    = _Graph_fromSMILESMulti  # type: ignore
Graph.fromMOLString      = _Graph_fromMOLString  # type: ignore
Graph.fromMOLFile        = _Graph_fromMOLFile  # type: ignore
Graph.fromMOLStringMulti = _Graph_fromMOLStringMulti  # type: ignore
Graph.fromMOLFileMulti   = _Graph_fromMOLFileMulti  # type: ignore
Graph.fromSDString       = _Graph_fromSDString  # type: ignore
Graph.fromSDFile         = _Graph_fromSDFile  # type: ignore
Graph.fromSDStringMulti  = _Graph_fromSDStringMulti  # type: ignore
Graph.fromSDFileMulti    = _Graph_fromSDFileMulti  # type: ignore

graphGMLString = Graph.fromGMLString
graphGML       = Graph.fromGMLFile
graphDFS       = Graph.fromDFS
smiles         = Graph.fromSMILES

###########################################################

Graph.__repr__ = lambda self: str(self) + "(" + str(self.id) + ")"  # type: ignore
Graph.__eq__ = lambda self, other: self.id == other.id  # type: ignore
Graph.__lt__ = lambda self, other: self.id < other.id  # type: ignore
Graph.__hash__ = lambda self: self.id  # type: ignore

def _Graph__setattr__(self: Graph, name: str, value: Any) -> None:
	if name == "image":
		object.__setattr__(self, "image", _funcWrap(libpymod._Func_String, value))
	else:
		_NoNew__setattr__(self, name, value)
Graph.__setattr__ = _Graph__setattr__  # type: ignore


###########################################################
# Post
###########################################################

def _post_command(self, cmd: str) -> None:
	_deprecation("'post(cmd)' is deprecated. Use 'post.command(cmd)'.")
	return post.command(cmd)
post.__init__ = _post_command  # type: ignore

def postFlush() -> None:
	_deprecation("'postFlush()' is deprecated. Use 'post.flushCommands()'.")
	return post.flushCommands()
def postDisable() -> None:
	_deprecation("'postDisable()' is deprecated. Use 'post.disableCommands()'.")
	return post.disableCommands()
def postEnable() -> None:
	_deprecation("'postEnable()' is deprecated. Use 'post.enableCommands()'.")
	return post.enableCommands()
def postReset() -> None:
	_deprecation("'postReset()' is deprecated. Use 'post.reopenCommandFile()'.")
	return post.reopenCommandFile()

def postChapter(heading: str) -> None:
	_deprecation("'postChapter(heading)' is deprecated. Use 'post.summaryChapter(heading)'.")
	post.summaryChapter(heading)
def postSection(heading: str) -> None:
	_deprecation("'postSection(heading)' is deprecated. Use 'post.summarySection(heading)'.")
	post.summarySection(heading)


###########################################################

inputRules = []

_Rule_print_orig = Rule.print
def _Rule_print(self: Rule, first: Optional[GraphPrinter]=None, second: Optional[GraphPrinter]=None,
		printCombined: bool=False) -> Tuple[str, str]:
	if first is None:
		return _Rule_print_orig(self, printCombined)
	if second is None:
		second = first
	return _Rule_print_orig(self, first, second, printCombined)
Rule.print = _Rule_print  # type: ignore

_Rule_isomorphism = Rule.isomorphism
Rule.isomorphism = lambda self, r, maxNumMatches=1, labelSettings=_lsString: _Rule_isomorphism(self, r, maxNumMatches, labelSettings)  # type: ignore
_Rule_monomorphism = Rule.monomorphism
Rule.monomorphism = lambda self, r, maxNumMatches=1, labelSettings=_lsString: _Rule_monomorphism(self, r, maxNumMatches, labelSettings)  # type: ignore

_Rule_getGMLString = Rule.getGMLString
Rule.getGMLString = lambda self, withCoords=False: _Rule_getGMLString(self, withCoords)  # type: ignore
_Rule_printGML = Rule.printGML
Rule.printGML = lambda self, withCoords=False: _Rule_printGML(self, withCoords)  # type: ignore

def _ruleLoad(a: Rule, name: Optional[str], add: bool) -> Rule:
	if name is not None:
		a.name = name
	if add:
		inputRules.append(a)
	return a

_Rule_fromGMLString_orig = Rule.fromGMLString
_Rule_fromGMLFile_orig   = Rule.fromGMLFile
_Rule_fromDFS_orig       = Rule.fromDFS

def _Rule_fromGMLString(s: str, name=None, *, invert: bool=False, add: bool=True, printStereoWarnings: bool=True) -> Rule:
	return _ruleLoad(_Rule_fromGMLString_orig(s, invert, printStereoWarnings), name, add)
def _Rule_fromGMLFile(f: str, name=None, *, invert: bool=False, add: bool=True, printStereoWarnings: bool=True) -> Rule:
	return _ruleLoad(_Rule_fromGMLFile_orig(prefixFilename(f), invert, printStereoWarnings), name, add)
def _Rule_fromDFS(s: str, name=None, *, invert: bool=False, add: bool=True) -> Rule:
	return _ruleLoad(_Rule_fromDFS_orig(s, invert), name, add)

Rule.fromGMLString = _Rule_fromGMLString  # type: ignore
Rule.fromGMLFile   = _Rule_fromGMLFile  # type: ignore
Rule.fromDFS       = _Rule_fromDFS  # type: ignore

ruleGMLString = Rule.fromGMLString
ruleGML       = Rule.fromGMLFile

Rule.__repr__ = lambda self: str(self) + "(" + str(self.id) + ")"  # type: ignore
Rule.__eq__ = lambda self, other: self.id == other.id  # type: ignore
Rule.__lt__ = lambda self, other: self.id < other.id  # type: ignore
Rule.__hash__ = lambda self: self.id  # type: ignore


#----------------------------------------------------------
# Composition
#----------------------------------------------------------

def _RCEvaluator__getattribute__(self: RCEvaluator, name: str) -> Any:
	if name == "ruleDatabase":
		return _unwrap(self._ruleDatabase)  # type: ignore
	elif name == "products":
		return _unwrap(self._products)  # type: ignore
	else:
		return object.__getattribute__(self, name)
RCEvaluator.__getattribute__ = _RCEvaluator__getattribute__  # type: ignore

_RCEvaluator_eval = RCEvaluator.eval
RCEvaluator.eval = lambda self, exp, *, onlyUnique=True, verbosity=0: _unwrap(_RCEvaluator_eval(self, exp, onlyUnique, verbosity))  # type: ignore

def rcEvaluator(rules: Iterable[Rule], labelSettings: LabelSettings=_lsString) -> RCEvaluator:
	return libpymod._rcEvaluator(_wrap(libpymod._VecRule, rules), labelSettings)


#----------------------------------------------------------
# RCMatch
#----------------------------------------------------------

_RCMatch_compose_orig = RCMatch.compose
def _RCMatch_compose(self, *, verbose=False):
	return _RCMatch_compose_orig(self, verbose)
RCMatch.compose = _RCMatch_compose  # type: ignore


_RCMatch_composeAll_orig = RCMatch.composeAll
def _RCMatch_composeAll(self, *, maximum=False, verbose=False):
	return _unwrap(_RCMatch_composeAll_orig(self, maximum, verbose))
RCMatch.composeAll = _RCMatch_composeAll  # type: ignore


#----------------------------------------------------------
# RCExp prettification
#----------------------------------------------------------

_rcExpType = Union[RCExpExp, RCExpBind, RCExpComposeCommon, RCExpComposeParallel, RCExpComposeSub, RCExpComposeSuper, Iterable["_rcExpType"]]

def rcExp(e: _rcExpType) -> RCExpExp:
	if isinstance(e, RCExpExp) or isinstance(e, Rule) or isinstance(e, RCExpUnion):
		return e
	elif isinstance(e, RCExpBind) or isinstance(e, RCExpId) or isinstance(e, RCExpUnbind):
		return e
	elif isinstance(e, RCExpComposeCommon) or isinstance(e, RCExpComposeParallel) or isinstance(e, RCExpComposeSub) or isinstance(e, RCExpComposeSuper):
		return e
	elif isinstance(e, collections.abc.Iterable):
		return RCExpUnion(_wrap(libpymod._VecRCExpExp, [rcExp(a) for a in e]))
	else:
		raise TypeError("Can not convert type '" + str(type(e)) + "' to RCExpExp")

_GraphOrGraphs = Union[Graph, Iterable[Graph]]

def _rcConvertGraph(g: _GraphOrGraphs, cls: Type[Union[RCExpBind, RCExpId, RCExpUnbind]], f: Callable[[Graph], RCExpExp]) -> RCExpExp:
	if isinstance(g, Graph):
		return cls(g)
	elif isinstance(g, collections.abc.Iterable):
		l = [f(a) for a in g]
		return rcExp(l)
	else:
		raise TypeError("Can not convert type '" + str(type(g)) + "' to " + str(cls))


def rcBind(g: _GraphOrGraphs ) -> RCExpExp:
	return _rcConvertGraph(g, RCExpBind, rcBind)


def rcId(g: _GraphOrGraphs) -> RCExpExp:
	return _rcConvertGraph(g, RCExpId, rcId)


def rcUnbind(g: _GraphOrGraphs) -> RCExpExp:
	return _rcConvertGraph(g, RCExpUnbind, rcUnbind)


class _RCCommonOpFirstBound:
	def __init__(self, discardNonchemical: bool, maximum: bool, connected: bool, includeEmpty: bool, first: RCExpExp) -> None:
		self.discardNonchemical = discardNonchemical
		self.maximum = maximum
		self.connected = connected
		self.includeEmpty = includeEmpty
		self.first = first

	def __mul__(self, second: RCExpExp) -> RCExpExp:
		return RCExpComposeCommon(rcExp(self.first), rcExp(second), self.discardNonchemical, self.maximum, self.connected, self.includeEmpty)


class _RCCommonOpArgsBound:
	def __init__(self, discardNonchemical: bool, maximum: bool, connected: bool, includeEmpty: bool) -> None:
		self.discardNonchemical = discardNonchemical
		self.maximum = maximum
		self.connected = connected
		self.includeEmpty = includeEmpty

	def __rmul__(self, first: RCExpExp) -> _RCCommonOpFirstBound:
		return _RCCommonOpFirstBound(self.discardNonchemical, self.maximum, self.connected, self.includeEmpty, first)


class _RCCommonOp:
	def __call__(self, discardNonchemical: bool=True, maximum: bool=False, connected: bool=True, includeEmpty: bool=False) -> _RCCommonOpArgsBound:
		return _RCCommonOpArgsBound(discardNonchemical, maximum, connected, includeEmpty)

	def __rmul__(self, first: RCExpExp) -> _RCCommonOpFirstBound:
		return first * self()


rcCommon = _RCCommonOp()


class _RCParallelOpFirstBound:
	def __init__(self, discardNonchemical: bool, first: RCExpExp) -> None:
		self.discardNonchemical = discardNonchemical
		self.first = first

	def __mul__(self, second: RCExpExp) -> RCExpExp:
		return RCExpComposeParallel(rcExp(self.first), rcExp(second), self.discardNonchemical)


class _RCParallelOpArgsBound:
	def __init__(self, discardNonchemical: bool) -> None:
		self.discardNonchemical = discardNonchemical

	def __rmul__(self, first: RCExpExp) -> _RCParallelOpFirstBound:
		return _RCParallelOpFirstBound(self.discardNonchemical, first)


class _RCParallelOp:
	def __call__(self, discardNonchemical: bool=True) -> _RCParallelOpArgsBound:
		return _RCParallelOpArgsBound(discardNonchemical)

	def __rmul__(self, first: RCExpExp) -> _RCParallelOpFirstBound:
		return first * self()


rcParallel = _RCParallelOp()


class _RCSubOpFirstBound:
	def __init__(self, discardNonchemical: bool, allowPartial: bool, first: RCExpExp) -> None:
		self.discardNonchemical = discardNonchemical
		self.allowPartial = allowPartial
		self.first = first

	def __mul__(self, second: RCExpExp) -> RCExpExp:
		return RCExpComposeSub(rcExp(self.first), rcExp(second), self.discardNonchemical, self.allowPartial)


class _RCSubOpArgsBound:
	def __init__(self, discardNonchemical: bool, allowPartial: bool) -> None:
		self.discardNonchemical = discardNonchemical
		self.allowPartial = allowPartial

	def __rmul__(self, first: RCExpExp) -> _RCSubOpFirstBound:
		return _RCSubOpFirstBound(self.discardNonchemical, self.allowPartial, first)


class _RCSubOp:
	def __call__(self, discardNonchemical: bool=True, allowPartial: bool=True) -> _RCSubOpArgsBound:
		return _RCSubOpArgsBound(discardNonchemical, allowPartial)

	def __rmul__(self, first: RCExpExp) -> _RCSubOpFirstBound:
		return first * self()


rcSub = _RCSubOp()


class _RCSuperOpFirstBound:
	def __init__(self, discardNonchemical: bool, allowPartial: bool, enforceConstraints: bool, first: RCExpExp) -> None:
		self.discardNonchemical = discardNonchemical
		self.allowPartial = allowPartial
		self.enforceConstraints = enforceConstraints
		self.first = first

	def __mul__(self, second: RCExpExp) -> RCExpExp:
		return RCExpComposeSuper(rcExp(self.first), rcExp(second), self.discardNonchemical, self.allowPartial, self.enforceConstraints)


class _RCSuperOpArgsBound:
	def __init__(self, discardNonchemical: bool, allowPartial: bool, enforceConstraints: bool) -> None:
		self.discardNonchemical = discardNonchemical
		self.allowPartial = allowPartial
		self.enforceConstraints = enforceConstraints

	def __rmul__(self, first: RCExpExp) -> _RCSuperOpFirstBound:
		return _RCSuperOpFirstBound(self.discardNonchemical, self.allowPartial, self.enforceConstraints, first)


class _RCSuperOp:
	def __call__(self, discardNonchemical: bool=True, allowPartial: bool=True, enforceConstraints: bool=False) -> _RCSuperOpArgsBound:
		return _RCSuperOpArgsBound(discardNonchemical, allowPartial, enforceConstraints)

	def __rmul__(self, first: RCExpExp) -> _RCSuperOpFirstBound:
		return first * self()


rcSuper = _RCSuperOp()


# ----------------------------------------------------------
# Util
# ----------------------------------------------------------

def showDump(f: str) -> None:
	return libpymod.showDump(prefixFilename(f))  # type: ignore
