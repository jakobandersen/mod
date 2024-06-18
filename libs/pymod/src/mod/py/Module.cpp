#include <mod/py/Common.hpp>

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/tuple/size.hpp>

#define MOD_ROOT_FILES()                                                         \
   (Chem)(Collections)(Config)(Derivation)                                       \
   (Error)(Function)                                                             \
   (Misc)(Term)

#define MOD_NAMESPACED_FILES()                                                   \
   ((graph, (Printer))) /* this must be before DGGraphInterface due to default arg */ \
   /* DG first, as others makes nested classes */                                \
   ((dg, (DG) (Builder) (GraphInterface) (Printer) (Strategy) (VertexMapper)))   \
   ((graph, (Graph) (Union)))                                                    \
   ((graph, (Automorphism) (GraphInterface))) /* nested classes of Graph, so must be after */ \
   ((rule, (CompositionMatch) (Composition) (Rule) (GraphInterface)))            \
   ((post, (Post)))

namespace mod {
#define MOD_forwardDeclare(a) void BOOST_PP_CAT(a, _doExport)();
#define MOD_iterDecl(r, data, t) namespace Py { MOD_forwardDeclare(t) }
BOOST_PP_SEQ_FOR_EACH(MOD_iterDecl, _, MOD_ROOT_FILES())
#undef MOD_iterDecl

#define MOD_iterNs(r, data, i, t) MOD_forwardDeclare(t)
#define MOD_iterDecl(r, data, t)                                              \
   namespace BOOST_PP_TUPLE_ELEM(2, 0, t)::Py {                               \
      BOOST_PP_SEQ_FOR_EACH_I(MOD_iterNs, ~, BOOST_PP_TUPLE_ELEM(2, 1, t))    \
   }
BOOST_PP_SEQ_FOR_EACH(MOD_iterDecl, _, MOD_NAMESPACED_FILES())
#undef MOD_iterDecl
#undef MOD_iterNs
#undef MOD_forwardDeclare
} // namespace mod

BOOST_PYTHON_MODULE(libpymod) {
	// specify that this module is actually a package
	py::object package = py::scope();
	package.attr("__path__") = "libpymod";
	#define MOD_call(a, prefix) mod::prefix Py:: BOOST_PP_CAT(a, _doExport)();
	#define MOD_iterDecl(r, data, t) MOD_call(t,)
	BOOST_PP_SEQ_FOR_EACH(MOD_iterDecl, _, MOD_ROOT_FILES())
	#undef MOD_iterDecl

	#define MOD_iterNs(r, data, i, t) MOD_call(t, data::)
	#define MOD_iterDecl(r, data, t)                                                \
	BOOST_PP_SEQ_FOR_EACH_I(MOD_iterNs, BOOST_PP_TUPLE_ELEM(2, 0, t), BOOST_PP_TUPLE_ELEM(2, 1, t))
	BOOST_PP_SEQ_FOR_EACH(MOD_iterDecl, _, MOD_NAMESPACED_FILES())
	#undef MOD_iterDecl
	#undef MOD_iterNs
	#undef MOD_call
}
