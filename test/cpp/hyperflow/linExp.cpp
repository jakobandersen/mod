#include <mod/Config.hpp>
#include <mod/dg/DG.hpp>
#include <mod/dg/Builder.hpp>
#include <mod/hyperflow/LinExp.hpp>

#include <boost/preprocessor/seq/enum.hpp>

int main() {
	auto dg = mod::dg::DG::make(mod::LabelSettings{
			                            mod::LabelType::String,
			                            mod::LabelRelation::Specialisation},
	                            {}, mod::IsomorphismPolicy::Check);
	dg->build().addAbstract("A -> B");
	const auto vDG = *dg->vertices().begin();
	const auto g = vDG.getGraph();
	const auto eDG = *dg->edges().begin();
	const mod::hyperflow::VarSumVertex vsv("vsv");
	const mod::hyperflow::VarVertex vv("vv", vDG);
	const mod::hyperflow::VarVertexGraph vvg("vvg", g);
	const mod::hyperflow::VarSumEdge vse("vse");
	const mod::hyperflow::VarEdge ve("ve", eDG);
	const mod::hyperflow::Var v(vsv);
	const mod::hyperflow::LinExp exp;

#define MOD_varTypes() (vsv)(vv)(vvg)(vse)(ve)(v)(exp)
#define MOD_linExp(e) {const mod::hyperflow::LinExp res = e;}
#define MOD_linCons(e) {const mod::hyperflow::LinConstraint res = e;}

#define MOD_iterOuter(r, data, first)                                             \
	{auto res = first;}                                                           \
	MOD_linExp(first)                                                             \
	MOD_linExp(+first)                                                            \
	MOD_linExp(-first)                                                            \
	MOD_linExp(mod::hyperflow::LinExp() += first)                                 \
	MOD_linExp(mod::hyperflow::LinExp() -= first)                                 \
	MOD_linExp(42 * first)                                                        \
	MOD_linExp(first * 42)                                                        \
	MOD_linExp(3.14 * first)                                                      \
	MOD_linExp(first * 3.14)                                                      \
	MOD_linCons(first <= 42)                                                      \
	MOD_linCons(first <= 3.14)                                                    \
	MOD_linCons(first == 42)                                                      \
	MOD_linCons(first == 3.14)                                                    \
	MOD_linCons(first >= 42)                                                      \
	MOD_linCons(first >= 3.14)                                                    \
	MOD_linCons(42 <= first)                                                      \
	MOD_linCons(3.14 <= first)                                                    \
	MOD_linCons(42 == first)                                                      \
	MOD_linCons(3.14 == first)                                                    \
	MOD_linCons(42 >= first)                                                      \
	MOD_linCons(3.14 >= first)                                                    \
	BOOST_PP_SEQ_FOR_EACH_I(MOD_iterInner, first, MOD_varTypes())
#define MOD_iterInner(r, first, i, second)                                        \
	MOD_linExp(first + second)                                                    \
	MOD_linExp(first - second)

	BOOST_PP_SEQ_FOR_EACH(MOD_iterOuter, ~, MOD_varTypes())
}
