#include "Base.hpp"

#include <mod/Error.hpp>
#include <mod/lib/CombiOpt/LoadedSolution.hpp>
#include <mod/lib/CombiOpt/Model.hpp>
#include <mod/lib/CombiOpt/Solver.hpp>
#include <mod/lib/DG/IO/Read.hpp>
#include <mod/lib/Graph/Graph.hpp>
#include <mod/lib/HyperFlow/Model.hpp>
#include <mod/lib/HyperFlow/ModuleRegistry.hpp>
#include <mod/lib/HyperFlow/Specification.hpp>
#include <mod/lib/HyperFlow/Modules/Utils.hpp>
#include <mod/lib/IO/IO.hpp>
#include <mod/lib/IO/Json.hpp>

#include <jla_boost/graph/PairToRangeAdaptor.hpp>

#include <boost/lexical_cast.hpp>

#include <iostream>

namespace mod::lib::HyperFlow {

struct BaseModel::StaticInit {
	StaticInit() {
		auto &r = ModuleRegistry::get();
		std::type_index id = typeid(BaseSpecification);
		r.addModule(id, "Base", [](Specification &spec, bool setDefaults) {
			spec.addModule<BaseSpecification>(setDefaults);
		});
#define SUM_CALLBACK_INT(name, member)                                         \
        [](const Model &m) {                                                   \
            if(m.specification->getModule<BaseSpecification>().relaxed)        \
                throw LogicError("Can not compile variable set '" name         \
                                 "'. It does not exist in relaxed mode.");     \
            const auto &mm = m.getModule<BaseModel>();                         \
            CombiOpt::LinExpAny res;                                           \
            for(const auto &p : mm.member)                                     \
                res += p.second;                                               \
            return res;                                                        \
        }
#define IDX_CALLBACK_INT(name, member)                                         \
        [](const Model &m, const lib::DG::HyperVertex &x) {                    \
            if(m.specification->getModule<BaseSpecification>().relaxed)        \
                throw LogicError("Can not compile indexed variable set '" name \
                                 "'. It does not exist in relaxed mode.");     \
            const auto &mm = m.getModule<BaseModel>();                         \
            const auto iter = mm.member.find(x);                               \
            assert(iter != end(mm.member));                                    \
            CombiOpt::LinExpAny res;                                           \
            res += iter->second;                                               \
            return res;                                                        \
        }
#define SUM_CALLBACK_BOTH(member)                                              \
        [](const Model &m) {                                                   \
            const auto &mm = m.getModule<BaseModel>();                         \
            CombiOpt::LinExpAny res;                                           \
            if(m.specification->getModule<BaseSpecification>().relaxed)        \
                for(const auto &p : mm.relaxedMode.member)                     \
                    res += p.second;                                           \
            else                                                               \
                for(const auto &p : mm.member)                                 \
                    res += p.second;                                           \
            return res;                                                        \
        }
#define IDX_CALLBACK_BOTH(name, member)                                        \
        [](const Model &m, const lib::DG::HyperVertex &x) {                    \
            const auto &mm = m.getModule<BaseModel>();                         \
            if(m.specification->getModule<BaseSpecification>().relaxed) {      \
                const auto iter = mm.relaxedMode.member.find(x);               \
                assert(iter != end(mm.relaxedMode.member));                    \
                CombiOpt::LinExpAny res;                                       \
                res += iter->second;                                           \
                return res;                                                    \
            } else {                                                           \
                const auto iter = mm.member.find(x);                           \
                assert(iter != end(mm.member));                                \
                CombiOpt::LinExpAny res;                                       \
                res += iter->second;                                           \
                return res;                                                    \
            }                                                                  \
        }
#define MAKE_VERTEX_BOTH(name, member) {                                       \
            auto vs = makeVertexVariableSet(id,                                \
                SUM_CALLBACK_BOTH(member),                                     \
                IDX_CALLBACK_BOTH(name, member));                              \
            r.addVariableSet(name, std::move(vs));                             \
        }
#define MAKE_VERTEX_INT(name, member) {                                        \
            auto vs = makeVertexVariableSet(id,                                \
                SUM_CALLBACK_INT(name, member),                                \
                IDX_CALLBACK_INT(name, member));                               \
            r.addVariableSet(name, std::move(vs));                             \
        }
		MAKE_VERTEX_BOTH("inFlow", inFlow);
		MAKE_VERTEX_BOTH("outFlow", outFlow);
		MAKE_VERTEX_INT("isInUsed", isInUsed);
		MAKE_VERTEX_INT("isOutUsed", isOutUsed);
		MAKE_VERTEX_INT("isInLessOut", isInLessOut);
		MAKE_VERTEX_INT("isInGreaterOut", isInGreaterOut);
		MAKE_VERTEX_INT("isInOutZero", isInOutZero);
		MAKE_VERTEX_BOTH("vertexFlow", vertexFlow);
		MAKE_VERTEX_INT("isVertexUsed", isVertexUsed);
		{
			auto vs = makeVertexVariableSet(
					id,
					[](const Model &m) {
						const auto &mm = m.getModule<BaseModel>();
						if(!mm.hasAllTransitInternalFlow) {
							throw LogicError("Can not compile sum of variable set transitInternalFlow."
							                 " Not all vertices have IO and internal transit flow separated."
							                 " Use the separateIOInternalTransit() method on all vertices"
							                 " before using the sum-version of this specifier.");
						}
						CombiOpt::LinExpAny res;
						if(m.specification->getModule<BaseSpecification>().relaxed)
							for(const auto &p : mm.relaxedMode.transitInternalFlow)
								res += p.second;
						else
							for(const auto &p : mm.transitInternalFlow)
								res += p.second;
						return res;
					},
					[](const Model &m, const lib::DG::HyperVertex &v) {
						const auto &mm = m.getModule<BaseModel>();
						const auto doIt = [&m, &v](const auto &vars) {
							const auto iter = vars.find(v);
							if(iter == end(vars)) {
								const auto vInt = m.specification->dgHyper.getInterfaceVertex(v);
								throw LogicError("Can not compile indexed variable set transitInternalFlow"
								                 " for vertex " + boost::lexical_cast<std::string>(vInt)
								                 + " (" + vInt.getGraph()->getName()
								                 + ")."
								                   " It does not have IO and internal transit flow separated."
								                   " Use the separateIOInternalTransit() method on the vertex first.");
							}
							CombiOpt::LinExpAny res;
							res += iter->second;
							return res;
						};
						if(m.specification->getModule<BaseSpecification>().relaxed)
							return doIt(mm.relaxedMode.transitInternalFlow);
						else
							return doIt(mm.transitInternalFlow);
					}
			);
			r.addVariableSet("transitInternalFlow", std::move(vs));
		}
#define MAKE_EDGE_BOTH(name, member) {                                         \
            auto vs = makeEdgeVariableSet(id,                                  \
                SUM_CALLBACK_BOTH(member),                                     \
                IDX_CALLBACK_BOTH(name, member));                              \
            r.addVariableSet(name, std::move(vs));                             \
        }
#define MAKE_EDGE_INT(name, member) {                                          \
            auto vs = makeEdgeVariableSet(id,                                  \
                SUM_CALLBACK_INT(name, member),                                \
                IDX_CALLBACK_INT(name, member));                               \
            r.addVariableSet(name, std::move(vs));                             \
        }
		MAKE_EDGE_BOTH("edgeFlow", edgeFlow);
		MAKE_EDGE_INT("isEdgeUsed", isEdgeUsed);
		{
			auto vs = makeEdgeVariableSet(
					id,
					SUM_CALLBACK_INT("isBothReverseUsed", isEdgeBothReverseUsed),
					[](const Model &m, const lib::DG::HyperVertex &e) {
						if(m.specification->getModule<BaseSpecification>().relaxed)
							throw LogicError("Can not compile indexed variable set '"
							                 "isBothReverseUsed'. It does not exist in relaxed mode.");
						const auto &mm = m.getModule<BaseModel>();
						const auto &dgHyper = m.specification->dgHyper;
						const auto eReverse = dgHyper.getReverseEdge(e);
						if(eReverse == dgHyper.getGraph().null_vertex())
							throw LogicError(
									"Can not compile indexed variable set 'isBothReverseUsed' for edge "
									+ boost::lexical_cast<std::string>(dgHyper.getInterfaceEdge(e))
									+ ". It does not have an inverse.");
						const auto eId = get(boost::vertex_index_t(), dgHyper.getGraph(), e);
						const auto eReverseId = get(boost::vertex_index_t(), dgHyper.getGraph(), eReverse);
						const auto iter = eId < eReverseId
						                  ? mm.isEdgeBothReverseUsed.find(e)
						                  : mm.isEdgeBothReverseUsed.find(eReverse);
						assert(iter != end(mm.isEdgeBothReverseUsed));
						CombiOpt::LinExpAny res;
						res += iter->second;
						return res;
					}
			);
			r.addVariableSet("isBothReverseUsed", std::move(vs));
		}
	}
};

namespace {
BaseModel::StaticInit staticInit;
} // namespace


BaseModel::BaseModel(Model &owner, const BaseSpecification &specification)
		: ModelModule(owner), specification(specification) {
	const auto &dg = owner.specification->dgHyper.getGraph();
	const auto &&idx = get(boost::vertex_index_t(), dg);
	isExcluded.resize(num_vertices(dg));
	for(const auto v : specification.getExcluded()) {
		isExcluded[idx[v]] = true;
		for(const auto vIn : asRange(inv_adjacent_vertices(v, dg)))
			isExcluded[idx[vIn]] = true;
		for(const auto vOut : asRange(adjacent_vertices(v, dg)))
			isExcluded[idx[vOut]] = true;
	}
}

const std::vector<bool> &BaseModel::getIsExcluded() const {
	return isExcluded;
}

const BaseSpecification &BaseModel::getSpec() const {
	return specification;
}

void BaseModel::createVariablesImpl(CombiOpt::Model &model) {
	const auto &dg = owner.specification->dgHyper;
	const auto &dgHyper = dg.getGraph();
	const auto &dgExpandedWrapper = owner.getExpanded();
	const auto &dgExpanded = dgExpandedWrapper.getGraph();

	// per edge variables
	for(const auto vHyper : asRange(vertices(dgHyper))) {
		if(dgHyper[vHyper].kind != lib::DG::HyperVertexKind::Edge) continue;
		const std::string nameSuffix =
				"(" + boost::lexical_cast<std::string>(get(boost::vertex_index_t(), dgHyper, vHyper)) + ")";
		// edge
		if(!specification.relaxed) {
			const auto var = model.addIntVariable("edge" + nameSuffix);
			edgeFlow.emplace(vHyper, var);
			allFlowVars.emplace(dgExpandedWrapper.getExpandedFromHyperEdge(vHyper), var);
		} else {
			const auto var = model.addFloatVariable("edge" + nameSuffix);
			relaxedMode.edgeFlow.emplace(vHyper, var);
			relaxedMode.allFlowVars.emplace(dgExpandedWrapper.getExpandedFromHyperEdge(vHyper), var);
		}
		{ // isEdgeUsed
			const auto var = model.addBoolVar("isEdgeUsed" + nameSuffix);
			isEdgeUsed.emplace(vHyper, var);
		}
		{ // isEdgeBothReverseUsed
			const auto vHyperReverse = dg.getReverseEdge(vHyper);
			if(vHyperReverse != dgHyper.null_vertex()) {
				const auto vHyperId = get(boost::vertex_index_t(), dgHyper, vHyper);
				const auto vHyperReverseId = get(boost::vertex_index_t(), dgHyper, vHyperReverse);
				if(vHyperId <= vHyperReverseId) { // equal means it's a null edge
					const auto var = model.addBoolVar(
							"isBothReverseUsed(" + boost::lexical_cast<std::string>(vHyperId) + ", "
							+ boost::lexical_cast<std::string>(vHyperReverseId) + ")");
					isEdgeBothReverseUsed.emplace(vHyper, var);
				}
			}
		}
	};
	// transit variables
	for(const auto vExpanded : asRange(vertices(dgExpanded))) {
		if(dgExpanded[vExpanded].kind != lib::DG::ExpandedVertexKind::TransitEdge) continue;
		const auto vHyper = dgExpanded[vExpanded].vHyper;
		std::string name = "t_" + dgHyper[vHyper].graph->getName() + '(';
		assert(in_degree(vExpanded, dgExpanded) == 1);
		assert(out_degree(vExpanded, dgExpanded) == 1);
		const auto vIn = *inv_adjacent_vertices(vExpanded, dgExpanded).first;
		const auto vOut = *adjacent_vertices(vExpanded, dgExpanded).first;
		const auto &vertexData = dgExpandedWrapper.getVertexData(vHyper);
		if(dgExpanded[vIn].eRepr == dgExpanded.null_vertex()
		   && vertexData.hasInCatchAll
		   && vIn == vertexData.inVertices.front()) {
			name += "rest";
		} else {
			name += boost::lexical_cast<std::string>(get(boost::vertex_index_t(), dgHyper, dgExpanded[vIn].eRepr));
		}
		name += ", ";
		if(dgExpanded[vOut].eRepr == dgExpanded.null_vertex()
		   && vertexData.hasOutCatchAll
		   && vOut == vertexData.outVertices.front()) {
			name += "rest";
		} else {
			name += boost::lexical_cast<std::string>(get(boost::vertex_index_t(), dgHyper, dgExpanded[vOut].eRepr));
		}
		name += ')';
		//std::cout << "TransitEdge: " << name << std::endl;
		if(!specification.relaxed) {
			const auto var = model.addIntVariable(std::move(name));
			allFlowVars.emplace(vExpanded, var);
		} else {
			const auto var = model.addFloatVariable(std::move(name));
			relaxedMode.allFlowVars.emplace(vExpanded, var);
		}
	}
	// per vertex variables, except transit
	for(const auto vHyper : asRange(vertices(dgHyper))) {
		if(dgHyper[vHyper].kind != lib::DG::HyperVertexKind::Vertex) continue;
		const std::string nameSuffix = "(" + dgHyper[vHyper].graph->getName() + ")";
		// inFlow
		if(!specification.relaxed) {
			const auto flowVar = model.addIntVariable("in" + nameSuffix);
			inFlow.emplace(vHyper, flowVar);
			allFlowVars.emplace(dgExpandedWrapper.getVertexData(vHyper).inputEdge, flowVar);
		} else {
			const auto flowVar = model.addFloatVariable("in" + nameSuffix);
			relaxedMode.inFlow.emplace(vHyper, flowVar);
			relaxedMode.allFlowVars.emplace(dgExpandedWrapper.getVertexData(vHyper).inputEdge, flowVar);
		}
		{ // isInUsed
			const auto indVar = model.addBoolVar("isInUsed" + nameSuffix);
			isInUsed.emplace(vHyper, indVar);
		}
		{ // isOutUsed
			const auto indVar = model.addBoolVar("isOutUsed" + nameSuffix);
			isOutUsed.emplace(vHyper, indVar);
		}
		// outFlow
		if(!specification.relaxed) {
			const auto flowVar = model.addIntVariable("out" + nameSuffix);
			outFlow.emplace(vHyper, flowVar);
			allFlowVars.emplace(dgExpandedWrapper.getVertexData(vHyper).outputEdge, flowVar);
		} else {
			const auto flowVar = model.addFloatVariable("out" + nameSuffix);
			relaxedMode.outFlow.emplace(vHyper, flowVar);
			relaxedMode.allFlowVars.emplace(dgExpandedWrapper.getVertexData(vHyper).outputEdge, flowVar);
		}
		{ // isInLessOutVars, isInGreaterOutVars, isInOutZeroVars
			const auto inLessOut = model.addBoolVar("Z<" + nameSuffix);
			const auto inGreaterOut = model.addBoolVar("Z>" + nameSuffix);
			const auto inOutZero = model.addBoolVar("Z0" + nameSuffix);
			isInLessOut.emplace(vHyper, inLessOut);
			isInGreaterOut.emplace(vHyper, inGreaterOut);
			isInOutZero.emplace(vHyper, inOutZero);
		}
		// vertex
		if(!specification.relaxed) {
			const auto vertex = model.addIntVariable("vertex" + nameSuffix);
			vertexFlow.emplace(vHyper, vertex);
		} else {
			const auto vertex = model.addFloatVariable("vertex" + nameSuffix);
			relaxedMode.vertexFlow.emplace(vHyper, vertex);
		}
		{ // isVertexUsed
			const auto var = model.addBoolVar("isVertexUsed" + nameSuffix);
			isVertexUsed.emplace(vHyper, var);
		}
		// transitInternalFlow
		if(dgExpandedWrapper.hasIOInternalTransitSeparated(vHyper)) {
			if(!specification.relaxed) {
				const auto vertex = model.addIntVariable("transitInternal" + nameSuffix);
				transitInternalFlow.emplace(vHyper, vertex);
			} else {
				const auto vertex = model.addFloatVariable("transitInternal" + nameSuffix);
				relaxedMode.transitInternalFlow.emplace(vHyper, vertex);
			}
		} else {
			hasAllTransitInternalFlow = false;
		}
	}
}

void BaseModel::createConstraintsImpl(CombiOpt::Model &model) {
	const auto &dg = owner.specification->dgHyper;
	const auto &dgHyper = dg.getGraph();
	const auto &dgExpandedWrapper = owner.getExpanded();
	const auto &dgExpanded = dgExpandedWrapper.getGraph();
	const auto findVar = [](const auto &c, const auto v) {
		const auto iter = c.find(v);
		assert(iter != end(c));
		return iter->second;
	};

	// flow conservation
	const auto makeConservationConstraints = [&](const auto linExp, const auto &vs) {
		for(const auto vExp : asRange(vertices(dgExpanded))) {
			if(!lib::DG::isKindVertex(dgExpanded[vExp].kind)) continue;
			auto exp = linExp;
			for(const auto vIn : asRange(inv_adjacent_vertices(vExp, dgExpanded)))
				exp += findVar(vs.allFlowVars, vIn);
			for(const auto vOut : asRange(adjacent_vertices(vExp, dgExpanded)))
				exp -= findVar(vs.allFlowVars, vOut);
			model.addConstraint(std::move(exp) == 0);
		}
		// flow conservation, it's already in the transit network, so could be removed
		for(const auto vHyper : asRange(vertices(dgHyper))) {
			if(dgHyper[vHyper].kind != lib::DG::HyperVertexKind::Vertex) continue;
			auto exp = linExp;
			exp += findVar(vs.inFlow, vHyper);
			for(const auto vAdj : asRange(inv_adjacent_vertices(vHyper, dgHyper)))
				exp += findVar(vs.edgeFlow, vAdj);
			exp -= findVar(vs.outFlow, vHyper);
			for(const auto vAdj : asRange(adjacent_vertices(vHyper, dgHyper)))
				exp -= findVar(vs.edgeFlow, vAdj);
			model.addConstraint(exp == 0);
		}
	};
	// vertexFlow and vertexFilter
	const auto handleVertexFlow = [&](const auto linExp, const auto &vs) {
		for(const auto vHyper : asRange(vertices(dgHyper))) {
			if(dgHyper[vHyper].kind != lib::DG::HyperVertexKind::Vertex) continue;
			const auto var = findVar(vs.vertexFlow, vHyper);
			{ // vertex
				// vertex = sum in-edges
				auto exp = linExp;
				exp += findVar(vs.inFlow, vHyper);
				for(const auto vAdj : asRange(inv_adjacent_vertices(vHyper, dgHyper)))
					exp += findVar(vs.edgeFlow, vAdj);
				exp -= findVar(vs.vertexFlow, vHyper);
				model.addConstraint(exp == 0);
			}
			if(isExcluded[get(boost::vertex_index_t(), dgHyper, vHyper)])
				model.setUB(var, {});
		}
	};
	// actually call those functions above
	if(!specification.relaxed) {
		makeConservationConstraints(CombiOpt::LinExpInt(), *this);
		handleVertexFlow(CombiOpt::LinExpInt(), *this);
	} else {
		makeConservationConstraints(CombiOpt::LinExpFloat(), relaxedMode);
		handleVertexFlow(CombiOpt::LinExpFloat(), relaxedMode);
	}

	// per-vertex variables
	const auto handleSourceSinks = [&](const auto &vars, const auto vHyper) {
		const auto inIter = vars.inFlow.find(vHyper);
		const auto outIter = vars.outFlow.find(vHyper);
		assert(inIter != end(vars.inFlow));
		assert(outIter != end(vars.outFlow));
		const auto in = inIter->second;
		const auto out = outIter->second;
		{ // in, sources
			const auto &sources = specification.getSources();
			const auto iter = sources.find(vHyper);
			if(iter == end(sources))
				model.setUB(in, {});
		}
		{ // out, sinks
			const auto &sinks = specification.getSinks();
			const auto iter = sinks.find(vHyper);
			if(iter == end(sinks))
				model.setUB(out, {});
		}
	};
	const auto handleTransitInternal = [&](auto linExp, const auto &vars, const auto vHyper) {
		constexpr bool DEBUG = false;
		const auto iter = vars.transitInternalFlow.find(vHyper);
		if(iter == end(vars.transitInternalFlow)) return;
		const auto tif = iter->second;
		using LinExp = decltype(linExp);
		LinExp expr = -LinExp(tif);
		const auto &gExpanded = dgExpandedWrapper.getGraph();
		const auto &vertexData = dgExpandedWrapper.getVertexData(vHyper);
		const auto vInput = vertexData.inputEdge;
		const auto vOutput = vertexData.outputEdge;
		assert(out_degree(vInput, gExpanded) == 1);
		assert(in_degree(vOutput, gExpanded) == 1);
		const auto vInputTrans = *adjacent_vertices(vInput, gExpanded).first;
		const auto vOutputTrans = *inv_adjacent_vertices(vOutput, gExpanded).first;
		assert(in_degree(vInputTrans, gExpanded) == 1);
		assert(out_degree(vOutputTrans, gExpanded) == 1);
		for(const auto vInTrans : vertexData.inVertices) {
			if(DEBUG)
				std::cout << "handleTransitInternal(" << model.getVarName(iter->second) << "): in? " << vInTrans
				          << std::endl;
			if(vInTrans == vInputTrans) // skip inFlow edge
				continue;
			if(DEBUG)
				std::cout << "handleTransitInternal(" << model.getVarName(iter->second) << "): " << vInTrans << std::endl;
			for(const auto vTrans : asRange(adjacent_vertices(vInTrans, gExpanded))) {
				assert(out_degree(vTrans, gExpanded) == 1);
				const auto vOutTrans = *adjacent_vertices(vTrans, gExpanded).first;
				if(DEBUG)
					std::cout << "handleTransitInternal(" << model.getVarName(iter->second) << "): out? " << vOutTrans
					          << std::endl;
				if(vOutTrans == vOutputTrans) // skip outFlow edge
					continue;
				if(DEBUG)
					std::cout << "handleTransitInternal(" << model.getVarName(iter->second) << "): " << vOutTrans
					          << std::endl;
				assert(gExpanded[vTrans].kind == lib::DG::ExpandedVertexKind::TransitEdge);
				const auto tVar = findVar(vars.allFlowVars, vTrans);
				expr += tVar;
			}
		}
		model.addConstraint(expr == 0);
	};
	if(specification.relaxed) {
		for(const auto vHyper : asRange(vertices(dgHyper))) {
			if(dgHyper[vHyper].kind != lib::DG::HyperVertexKind::Vertex) continue;
			handleSourceSinks(this->relaxedMode, vHyper);
			handleTransitInternal(CombiOpt::LinExpFloat(), this->relaxedMode, vHyper);

			const auto isInUsed = findVar(this->isInUsed, vHyper);
			const auto isOutUsed = findVar(this->isOutUsed, vHyper);
			const auto isInLessOut = findVar(this->isInLessOut, vHyper);
			const auto isInGreaterOut = findVar(this->isInGreaterOut, vHyper);
			const auto isInOutZero = findVar(this->isInOutZero, vHyper);
			const auto isVertexUsed = findVar(this->isVertexUsed, vHyper);
			for(auto v :{isInUsed, isOutUsed, isInLessOut, isInGreaterOut, isInOutZero, isVertexUsed})
				model.addConstraint(!v);
		}
	} else {
		for(const auto vHyper : asRange(vertices(dgHyper))) {
			if(dgHyper[vHyper].kind != lib::DG::HyperVertexKind::Vertex) continue;
			handleSourceSinks(*this, vHyper);
			handleTransitInternal(CombiOpt::LinExpInt(), *this, vHyper);

			const auto in = findVar(this->inFlow, vHyper);
			const auto out = findVar(this->outFlow, vHyper);
			const auto isInUsed = findVar(this->isInUsed, vHyper);
			const auto isOutUsed = findVar(this->isOutUsed, vHyper);
			const auto isInLessOut = findVar(this->isInLessOut, vHyper);
			const auto isInGreaterOut = findVar(this->isInGreaterOut, vHyper);
			const auto isInOutZero = findVar(this->isInOutZero, vHyper);
			const auto vertex = findVar(this->vertexFlow, vHyper);
			const auto isVertexUsed = findVar(this->isVertexUsed, vHyper);
			{ // isInUsed
				// in >= 1   <=> isInUsed
				model.addEquivalence(in >= 1, isInUsed);
				//				// isInPositive <= in	<=>		in - isInPositive >= 0
				//				base.p->addConstraint(in - isInPositive >= 0);
				//				// M*isInPositive >= in		<=>		-in + M*isInPositive  >= 0
				//				base.p->addConstraint(-in + base.M * isInPositive >= 0);
			}
			// isOutUsed
			model.addEquivalence(out >= 1, isOutUsed);
			{ // isInLessOut
				// in < out   <=> isInLessOut
				// in + 1 <= out
				// out - in >= 1
				model.addEquivalence(out - in >= 1, isInLessOut);
				//				// in < out + M * (1 - inLessOut)
				//				// <=> in + 1 <= out + M * (1 - inLessOut)
				//				// <=> out - in - M * inLessOut >= 1 - M
			}
			{ // isInGreaterOut
				// in > out   <=> isInGreatOut
				// in - out >= 1
				model.addEquivalence(in - out >= 1, isInGreaterOut);
				//				// out < in + M * (1 - inGreaterOut)
				//				// <=> out + 1 <= in + M * (1 - inGreaterOut)
				//				// <=> in - out - M * inGreaterOut >= 1 - M
				//				base.p->addConstraint(in - out - base.M * isInGreaterOut >= -base.M + 1);
				//				// out >= in - M * inGreaterOut
				//				// <=> M * inGreaterOut - in + out >= 0
				//				base.p->addConstraint(base.M * isInGreaterOut - in + out >= 0);
			}
			{ // isInOutZero
				model.addEquivalence(in + out <= 0, isInOutZero);
				//      1 − inOutZero  <= in + out   <=>   in + out +     inOutZero >= 1
				// M * (1 − inOutZero) >= in + out   <=>   in + out + M * inOutZero <= M
			}
			{ // isVertexUsed
				model.addEquivalence(vertex >= 1, isVertexUsed);
				//     isVertexUsed <= vertex	<=>		vertex -     isVertexUsed >= 0
				// M * isVertexUsed >= vertex	<=>		vertex - M * isVertexUsed =< 0
			}
		}
	} // end if relaxed
	// per-edge variables
	if(specification.relaxed) {
		// isEdgeUsed
		for(const auto &p : isEdgeUsed)
			model.addConstraint(!p.second);
		// isEdgeBothReverseUsed
		for(const auto &p : isEdgeBothReverseUsed)
			model.addConstraint(!p.second);
	} else {
		for(const auto vHyper : asRange(vertices(dgHyper))) {
			if(dgHyper[vHyper].kind != lib::DG::HyperVertexKind::Edge) continue;
			const auto edge = findVar(this->edgeFlow, vHyper);
			const auto isEdgeUsed = findVar(this->isEdgeUsed, vHyper);
			// isEdgeUsed
			model.addEquivalence(edge >= 1, isEdgeUsed);
			// isEdgeBothReverseUsed
			const auto vHyperReverse = dg.getReverseEdge(vHyper);
			if(vHyperReverse != dgHyper.null_vertex()) {
				const auto vHyperId = get(boost::vertex_index_t(), dgHyper, vHyper);
				const auto vHyperReverseId = get(boost::vertex_index_t(), dgHyper, vHyperReverse);
				if(vHyperId <= vHyperReverseId) { // equal means it's a loop edge
					const auto isEdgeBothReverseUsed = findVar(this->isEdgeBothReverseUsed, vHyper);
					const auto ea = isEdgeUsed;
					const auto eb = findVar(this->isEdgeUsed, vHyperReverse);
					// isBoth => ea AND eb
					model.addImplication(isEdgeBothReverseUsed, ea);
					model.addImplication(isEdgeBothReverseUsed, eb);
					// ea AND eb => isBoth
					model.addImplication(ea && eb, isEdgeBothReverseUsed);
				}
			}
			// hyper-loops
			if(!specification.allowHyperLoops && vHyperReverse == vHyper)
				model.setUB(edge, 0);
		}
	} // end if relaxed
}

CombiOpt::IntVar BaseModel::getEdge(lib::DG::ExpandedVertex vExp) const {
	assert(!lib::DG::isKindVertex(owner.getExpanded().getGraph()[vExp].kind));
	const auto iter = allFlowVars.find(vExp);
	assert(iter != end(allFlowVars));
	return iter->second;
}

CombiOpt::IntVar BaseModel::getIn(lib::DG::HyperVertex v) const {
	const auto iter = inFlow.find(v);
	assert(iter != end(inFlow));
	return iter->second;
}

CombiOpt::IntVar BaseModel::getOut(lib::DG::HyperVertex v) const {
	const auto iter = outFlow.find(v);
	assert(iter != end(outFlow));
	return iter->second;
}

CombiOpt::IntVar BaseModel::getVertex(lib::DG::HyperVertex v) const {
	const auto iter = vertexFlow.find(v);
	assert(iter != end(vertexFlow));
	return iter->second;
}

CombiOpt::BoolVar BaseModel::getIsEdgeUsed(lib::DG::HyperVertex v) const {
	const auto iter = isEdgeUsed.find(v);
	assert(iter != end(isEdgeUsed));
	return iter->second;
}

CombiOpt::BoolVar BaseModel::getIsEdgeBothReverseUsed(lib::DG::HyperVertex v) const {
	const auto iter = isEdgeBothReverseUsed.find(v);
	assert(iter != end(isEdgeBothReverseUsed));
	return iter->second;
}

CombiOpt::BoolVar BaseModel::getIsInUsed(lib::DG::HyperVertex v) const {
	const auto iter = isInUsed.find(v);
	assert(iter != end(isInUsed));
	return iter->second;
}

CombiOpt::BoolVar BaseModel::getIsOutUsed(lib::DG::HyperVertex v) const {
	const auto iter = isOutUsed.find(v);
	assert(iter != end(isOutUsed));
	return iter->second;
}

CombiOpt::BoolVar BaseModel::getIsInLessOut(lib::DG::HyperVertex v) const {
	const auto iter = isInLessOut.find(v);
	assert(iter != end(isInLessOut));
	return iter->second;
}

CombiOpt::BoolVar BaseModel::getIsInGreaterOut(lib::DG::HyperVertex v) const {
	const auto iter = isInGreaterOut.find(v);
	assert(iter != end(isInGreaterOut));
	return iter->second;
}

CombiOpt::BoolVar BaseModel::getIsInOutZero(lib::DG::HyperVertex v) const {
	const auto iter = isInOutZero.find(v);
	assert(iter != end(isInOutZero));
	return iter->second;
}

CombiOpt::BoolVar BaseModel::getIsVertexUsed(lib::DG::HyperVertex v) const {
	const auto iter = isVertexUsed.find(v);
	assert(iter != end(isVertexUsed));
	return iter->second;
}

// ===========================================================================

CombiOpt::FloatVar BaseModel::getEdgeRelaxed(lib::DG::ExpandedVertex vExp) const {
	assert(!lib::DG::isKindVertex(owner.getExpanded().getGraph()[vExp].kind));
	const auto iter = relaxedMode.allFlowVars.find(vExp);
	assert(iter != end(relaxedMode.allFlowVars));
	return iter->second;
}

CombiOpt::FloatVar BaseModel::getInRelaxed(lib::DG::HyperVertex v) const {
	const auto iter = relaxedMode.inFlow.find(v);
	assert(iter != end(relaxedMode.inFlow));
	return iter->second;
}

CombiOpt::FloatVar BaseModel::getOutRelaxed(lib::DG::HyperVertex v) const {
	const auto iter = relaxedMode.outFlow.find(v);
	assert(iter != end(relaxedMode.outFlow));
	return iter->second;
}

CombiOpt::FloatVar BaseModel::getVertexRelaxed(lib::DG::HyperVertex v) const {
	const auto iter = relaxedMode.vertexFlow.find(v);
	assert(iter != end(relaxedMode.vertexFlow));
	return iter->second;
}

// ===========================================================================

void BaseModel::loadSolution_v2to7(CombiOpt::LoadedSolution &s,
                                   const std::vector<std::pair<unsigned int, unsigned int>> &edgeFlow,
                                   const std::vector<std::pair<unsigned int, unsigned int>> &inFlow,
                                   const std::vector<std::pair<unsigned int, unsigned int>> &outFlow,
                                   const std::vector<std::tuple<unsigned int, std::vector<int>, std::vector<int>, unsigned int>> &transitFlow) const {
	const auto &dg = owner.specification->dgHyper;
	const auto &dgHyper = dg.getGraph();
	const auto &dgExpandedWrapper = owner.getExpanded();
	const auto &dgExpanded = dgExpandedWrapper.getGraph();
	if(specification.relaxed) {
		MOD_ABORT;
	} else {
		for(const auto &p : edgeFlow) {
			const auto e = vertex(p.first, dgHyper);
			if(dgHyper[e].kind != DG::HyperVertexKind::Edge) MOD_ABORT;
			const auto eExpanded = dgExpandedWrapper.getExpandedFromHyperEdge(e);
			const auto var = getEdge(eExpanded);
			if(s.integralValues.find(var) != end(s.integralValues)) MOD_ABORT;
			s.integralValues.emplace(var, CombiOpt::Int(static_cast<int>(p.second)));
		}
		for(const auto &p : inFlow) {
			const auto v = vertex(p.first, dgHyper);
			if(dgHyper[v].kind != DG::HyperVertexKind::Vertex) MOD_ABORT;
			const auto var = getIn(v);
			if(s.integralValues.find(var) != end(s.integralValues)) MOD_ABORT;
			s.integralValues.emplace(var, CombiOpt::Int(static_cast<int>(p.second)));
		}
		for(const auto &p : outFlow) {
			const auto v = vertex(p.first, dgHyper);
			if(dgHyper[v].kind != DG::HyperVertexKind::Vertex) MOD_ABORT;
			const auto var = getOut(v);
			if(s.integralValues.find(var) != end(s.integralValues)) MOD_ABORT;
			s.integralValues.emplace(var, CombiOpt::Int(static_cast<int>(p.second)));
		}
		for(const auto &t : transitFlow) {
			const auto v = vertex(std::get<0>(t), dgHyper);
			if(dgHyper[v].kind != DG::HyperVertexKind::Vertex) MOD_ABORT;
			const auto &vData = dgExpandedWrapper.getVertexData(v);
			const auto convert = [&dgHyper, v](const auto &eIds) {
				std::vector<DG::HyperVertex> res;
				res.reserve(eIds.size());
				for(const auto eId : eIds) {
					if(eId >= 0) {
						const auto e = vertex(eId, dgHyper);
						if(dgHyper[e].kind != DG::HyperVertexKind::Edge) MOD_ABORT;
						res.push_back(e);
					} else {
						res.push_back(v);
					}
				}
				std::sort(res.begin(), res.end());
				return res;
			};
			const auto inEdges = convert(std::get<1>(t)),
					outEdges = convert(std::get<2>(t));
			const auto inIter = std::find_if(
					vData.inVertices.begin(), vData.inVertices.end(),
					[&dgExpanded, &inEdges](const auto vIn) {
						std::vector<DG::HyperVertex> cand;
						for(const auto vInExpanded : asRange(inv_adjacent_vertices(vIn, dgExpanded)))
							cand.push_back(dgExpanded[vInExpanded].vHyper);
						std::sort(cand.begin(), cand.end());
						return cand == inEdges;
					});
			const auto outIter = std::find_if(
					vData.outVertices.begin(), vData.outVertices.end(),
					[&dgExpanded, &outEdges](const auto vOut) {
						std::vector<DG::ExpandedVertex> cand;
						for(const auto vOutExpanded : asRange(adjacent_vertices(vOut, dgExpanded)))
							cand.push_back(dgExpanded[vOutExpanded].vHyper);
						std::sort(cand.begin(), cand.end());
						return cand == outEdges;
					});
			const auto print = [&vData, &inEdges, &outEdges, &t, &dgExpanded]() {
				auto &s = std::cout;
				s << "setTransitFlow(vertexId=" << std::get<0>(t) << ",\n";
				s << "\tinEdges={";
				for(auto e : inEdges) s << " " << e;
				s << " },\n";
				s << "\toutEdges={";
				for(auto e : outEdges) s << " " << e;
				s << " }, flow=" << std::get<3>(t) << ")" << std::endl;
				for(const auto vIn : vData.inVertices) {
					s << "in:";
					for(const auto v : asRange(inv_adjacent_vertices(vIn, dgExpanded)))
						s << " " << dgExpanded[v].vHyper;
					s << "\n";
				}
				for(const auto vOut : vData.outVertices) {
					s << "out:";
					for(const auto v : asRange(adjacent_vertices(vOut, dgExpanded)))
						s << " " << dgExpanded[v].vHyper;
					s << "\n";
				}
			};
			if(inIter == end(vData.inVertices)) {
				print();
				MOD_ABORT;
			}
			if(outIter == end(vData.outVertices)) {
				print();
				MOD_ABORT;
			}
			const auto vsTrans = adjacent_vertices(*inIter, dgExpanded);
			const auto eTransIter = std::find_if(
					vsTrans.first, vsTrans.second, [&dgExpanded, outIter](const auto vCand) {
						assert(out_degree(vCand, dgExpanded) == 1);
						return *adjacent_vertices(vCand, dgExpanded).first == *outIter;
					});
			if(eTransIter == vsTrans.second) {
				print();
				MOD_ABORT;
			}
			const auto var = getEdge(*eTransIter);
			assert(s.integralValues.find(var) == end(s.integralValues));
			s.integralValues.emplace(var, static_cast<int>(std::get<3>(t)));
		}

		for(const auto v : asRange(vertices(dgHyper))) {
			if(dgHyper[v].kind == DG::HyperVertexKind::Vertex) {
				const auto inFlow = s.getVal(getIn(v));
				const auto outFlow = s.getVal(getOut(v));
				if(inFlow > 0) s.integralValues.emplace(getIsInUsed(v), CombiOpt::Int(1));
				if(inFlow < outFlow) s.integralValues.emplace(getIsInLessOut(v), CombiOpt::Int(1));
				if(inFlow > outFlow) s.integralValues.emplace(getIsInGreaterOut(v), CombiOpt::Int(1));
				if(inFlow == 0 && outFlow == 0) s.integralValues.emplace(getIsInOutZero(v), CombiOpt::Int(1));
				auto vertexFlow = inFlow;
				for(const auto vAdj : asRange(inv_adjacent_vertices(v, dgHyper))) {
					const auto eExpanded = dgExpandedWrapper.getExpandedFromHyperEdge(vAdj);
					const auto var = getEdge(eExpanded);
					vertexFlow += s.getVal(var);
				}
				if(vertexFlow > 0) {
					s.integralValues.emplace(getVertex(v), vertexFlow);
					s.integralValues.emplace(getIsVertexUsed(v), 1);
				}
			} else { // HyperEdge
				const auto eExpanded = dgExpandedWrapper.getExpandedFromHyperEdge(v);
				const auto var = getEdge(eExpanded);
				const auto edgeFlow = s.getVal(var);
				if(edgeFlow > 0) s.integralValues.emplace(getIsEdgeUsed(v), CombiOpt::Int(1));

				const auto vReverse = dg.getReverseEdge(v);
				if(vReverse != dgHyper.null_vertex()) {
					// only if we are the lowest numbered
					if(get(boost::vertex_index_t(), dgHyper, v) <
					   get(boost::vertex_index_t(), dgHyper, vReverse)) {
						const auto eReverseExpanded = dgExpandedWrapper.getExpandedFromHyperEdge(vReverse);
						const auto var = getEdge(eReverseExpanded);
						const auto reverseFlow = s.getVal(var);
						if(edgeFlow > 0 && reverseFlow > 0)
							s.integralValues.emplace(getIsEdgeBothReverseUsed(v), 1);
					}
				}
			}
		}
	}
}

namespace {

template<typename Range>
std::vector<std::size_t> makeIOVertexIdsFromExpanded(
		const lib::DG::HyperGraphType &dg, const lib::DG::ExpandedGraphType &dgExpanded, Range range) {
	std::vector<std::size_t> res;
	res.reserve(std::distance(range.first, range.second));
	for(const auto v : asRange(range)) {
		switch(dgExpanded[v].kind) {
		case lib::DG::ExpandedVertexKind::Edge:
		case lib::DG::ExpandedVertexKind::IOEdge:
			res.push_back(get(boost::vertex_index_t(), dg, dgExpanded[v].vHyper));
			break;
		case lib::DG::ExpandedVertexKind::TransitEdge:
		case lib::DG::ExpandedVertexKind::InVertex:
		case lib::DG::ExpandedVertexKind::OutVertex:
			assert(false); // should not happen
			break;
		}
	}
	std::sort(res.begin(), res.end());
	return res;
}

} // namespace

nlohmann::json BaseModel::dumpImpl(const CombiOpt::Result &sol) const {
	nlohmann::json j;
	auto edge = nlohmann::json::array(),
			in = nlohmann::json::array(),
			out = nlohmann::json::array(),
			transit = nlohmann::json::array();
	const auto assign = [&](const auto &flowVars) {
		const auto &dgHyper = owner.specification->dgHyper;
		const auto &dg = dgHyper.getGraph();
		const auto &dgExpanded = owner.getExpanded().getGraph();
		for(const auto &vp : flowVars) {
			const auto value = sol.getValue(vp.second).getValue();
			if(value == 0) continue;
			const auto vExpanded = vp.first;
			const auto vHyper = dgExpanded[vExpanded].vHyper;
			const auto vId = get(boost::vertex_index_t(), dg, vHyper);
			switch(dgExpanded[vExpanded].kind) {
			case lib::DG::ExpandedVertexKind::Edge:
				edge.push_back({vId, value});
				break;
			case lib::DG::ExpandedVertexKind::IOEdge:
				assert(
						in_degree(vExpanded, dgExpanded) + out_degree(vExpanded, dgExpanded) == 1);
				if(in_degree(vExpanded, dgExpanded) == 0)
					in.push_back({vId, value});
				else
					out.push_back({vId, value});
				break;
			case lib::DG::ExpandedVertexKind::TransitEdge: {
				auto data = nlohmann::json::array();
				data.push_back(vId);
				assert(in_degree(vExpanded, dgExpanded) == 1);
				assert(out_degree(vExpanded, dgExpanded) == 1);
				const auto vIn = *inv_adjacent_vertices(vExpanded, dgExpanded).first;
				const auto vOut = *adjacent_vertices(vExpanded, dgExpanded).first;
				assert(dgExpanded[vIn].kind == lib::DG::ExpandedVertexKind::InVertex);
				assert(dgExpanded[vOut].kind == lib::DG::ExpandedVertexKind::OutVertex);
				std::vector<std::size_t>
						in = makeIOVertexIdsFromExpanded(dg, dgExpanded, inv_adjacent_vertices(vIn, dgExpanded)),
						out = makeIOVertexIdsFromExpanded(dg, dgExpanded, adjacent_vertices(vOut, dgExpanded));
				data.push_back(std::move(in));
				data.push_back(std::move(out));
				data.push_back(value);
				transit.push_back(std::move(data));
				break;
			}
			case lib::DG::ExpandedVertexKind::InVertex:
			case lib::DG::ExpandedVertexKind::OutVertex:
				break;
			}
		}
	};
	if(getSpec().relaxed)
		assign(relaxedMode.allFlowVars);
	else
		assign(allFlowVars);
	j["edge"] = std::move(edge);
	j["in"] = std::move(in);
	j["out"] = std::move(out);
	j["transit"] = std::move(transit);
	return j;
}

bool BaseModel::loadImpl(const nlohmann::json &j, CombiOpt::LoadedSolution &s, std::ostream &err) const {
	static const nlohmann::json schema = R"({
		"$schema": "http://json-schema.org/draft-07/schema#",
		"type": "object",
		"properties": {
			"edge":
				{"type": "array", "items":
					{"type": "array", "additionalItems": false, "items": [
						{"type": "integer", "minimum": 0},
						{"type": "number", "minimum": 0}
					]}
				},
			"in":
				{"type": "array", "items":
					{"type": "array", "additionalItems": false, "items": [
						{"type": "integer", "minimum": 0},
						{"type": "number", "minimum": 0}
					]}
				},
			"out":
				{"type": "array", "items":
					{"type": "array", "additionalItems": false, "items": [
						{"type": "integer", "minimum": 0},
						{"type": "number", "minimum": 0}
					]}
				},
			"transit":
				{"type": "array", "items":
					{"type": "array", "additionalItems": false, "items": [
						{"type": "integer", "minimum": 0},
						{"type": "array", "items": {"type": "integer", "minimum": 0}},
						{"type": "array", "items": {"type": "integer", "minimum": 0}},
						{"type": "number", "minimum": 0}
					]}
				}
		},
		"required": ["edge", "in", "out", "transit"]
	})"_json;
	static const nlohmann::json_schema::json_validator validator(schema);
	if(!IO::validateJson(j, validator, err, "Base solution data does not conform to schema:"))
		return false;

	const auto &dg = owner.specification->dgHyper.getGraph();
	if(specification.relaxed) {
		const auto handleElements = [&j, &dg, &err, &s](
				const std::string &name, const std::string &nameUpper,
				const auto &loader, const auto &vars) {
			for(const auto &e : j[name]) {
				assert(e.size() == 2);
				const std::size_t vId = e[0];
				const auto vHyper = loader(dg, vId, err, "Base solution data, " + name + ".");
				if(!vHyper) return false;
				const auto &num = e[1];
				if(!num.is_number_float()) {
					err << "Base solution data. "
					    << nameUpper << " value is not a float, but the model is relaxed.";
					return false;
				}
				const auto iter = vars.find(*vHyper);
				assert(iter != vars.end());
				const auto var = iter->second;
				s.floatValues.emplace(var, num.get<double>());
			}
			return true;
		};
		const bool res = handleElements("edge", "Edge", lib::DG::Read::edge, relaxedMode.edgeFlow)
		                 && handleElements("in", "In", lib::DG::Read::vertex, relaxedMode.inFlow)
		                 && handleElements("out", "Out", lib::DG::Read::vertex, relaxedMode.outFlow);
		if(!res) return false;
	} else {
		const auto handleElements = [&j, &dg, &err, &s](
				const std::string &name, const std::string &nameUpper,
				const auto &loader, const auto &vars) {
			for(const auto &e : j[name]) {
				assert(e.size() == 2);
				const std::size_t vId = e[0];
				const auto vHyper = loader(dg, vId, err, "Base solution data, " + name + ".");
				if(!vHyper) return false;
				const auto &num = e[1];
				if(!num.is_number_integer()) {
					err << "Base solution data. "
					    << nameUpper << " value is not an int, but the model is not relaxed.";
					return false;
				}
				const auto iter = vars.find(*vHyper);
				assert(iter != vars.end());
				const auto var = iter->second;
				s.integralValues.emplace(var, num.get<int>());
			}
			return true;
		};
		const bool res = handleElements("edge", "Edge", lib::DG::Read::edge, edgeFlow)
		                 && handleElements("in", "In", lib::DG::Read::vertex, inFlow)
		                 && handleElements("out", "Out", lib::DG::Read::vertex, outFlow);
		if(!res) return false;
	}

	const auto convertTransitVertex = [](const auto &j) {
		std::vector<std::size_t> ids;
		ids.reserve(j.size());
		for(const auto &id : j) {
			const std::size_t i = id;
			ids.push_back(i);
		}
		std::sort(ids.begin(), ids.end());
		return ids;
	};
	const auto handleTransit = [&j, &dg, &err, this, convertTransitVertex](const auto &vars, const auto assign) {
		const auto &dgExpandedWrapper = owner.getExpanded();
		const auto &dgExpanded = dgExpandedWrapper.getGraph();
		// TODO: probably memoize the result of makeIOVertexIdsFromExpanded
		//       there can be many transit entries for a single vId
		for(const auto &t : j["transit"]) {
			assert(t.size() == 4);
			const std::size_t vId = t[0];
			const auto vHyperOpt = lib::DG::Read::vertex(dg, vId, err, "Base solution data, transit vertex ID.");
			if(!vHyperOpt) return false;
			const auto vHyper = *vHyperOpt;
			const auto inEdges = convertTransitVertex(t[1]);
			const auto outEdges = convertTransitVertex(t[2]);
			const auto &vData = dgExpandedWrapper.getVertexData(vHyper);
			const auto inIter = std::find_if(
					vData.inVertices.begin(), vData.inVertices.end(),
					[&dg, &dgExpanded, &inEdges](const auto vIn) {
						const auto ids = makeIOVertexIdsFromExpanded(dg, dgExpanded, inv_adjacent_vertices(vIn, dgExpanded));
						return ids == inEdges;
					});
			const auto outIter = std::find_if(
					vData.outVertices.begin(), vData.outVertices.end(),
					[&dg, &dgExpanded, &outEdges](const auto vOut) {
						const auto ids = makeIOVertexIdsFromExpanded(dg, dgExpanded, adjacent_vertices(vOut, dgExpanded));
						return ids == outEdges;
					});
			//		const auto print = [&vData, &inEdges, &outEdges, &t, &dgExpanded]() {
			//			auto &s = std::cout;
			//			s << "setTransitFlow(vertexId=" << std::get<0>(t) << ",\n";
			//			s << "\tinEdges={";
			//			for(auto e : inEdges) s << " " << e;
			//			s << " },\n";
			//			s << "\toutEdges={";
			//			for(auto e : outEdges) s << " " << e;
			//			s << " }, flow=" << std::get<3>(t) << ")" << std::endl;
			//			for(const auto vIn : vData.inVertices) {
			//				s << "in:";
			//				for(const auto v : asRange(inv_adjacent_vertices(vIn, dgExpanded)))
			//					s << " " << dgExpanded[v].vHyper;
			//				s << "\n";
			//			}
			//			for(const auto vOut : vData.outVertices) {
			//				s << "out:";
			//				for(const auto v : asRange(adjacent_vertices(vOut, dgExpanded)))
			//					s << " " << dgExpanded[v].vHyper;
			//				s << "\n";
			//			}
			//		};
			if(inIter == end(vData.inVertices)) {
				err << "Base solution data. Could not find in-vertex for transit edge.";
				//print();
				return false;
			}
			if(outIter == end(vData.outVertices)) {
				err << "Base solution data. Could not find out-vertex for transit edge.";
				//print();
				return false;
			}
			const auto vsTrans = adjacent_vertices(*inIter, dgExpanded);
			const auto eTransIter = std::find_if(
					vsTrans.first, vsTrans.second, [&dgExpanded, outIter](const auto vCand) {
						assert(out_degree(vCand, dgExpanded) == 1);
						return *adjacent_vertices(vCand, dgExpanded).first == *outIter;
					});
			if(eTransIter == vsTrans.second) {
				err << "Base solution data. In-vertex and out-vertex are not connected by a transit edge.";
				//print();
				return false;
			}
			const auto res = assign(*eTransIter, t[3]);
			if(!res) return false;
		}
		return true;
	};
	if(specification.relaxed) {
		return handleTransit(
				relaxedMode.allFlowVars,
				[&s, &err, this](const lib::DG::ExpandedVertex e, const nlohmann::json &value) {
					if(!value.is_number_float()) {
						err << "Base solution data. Transit value is not a float, but the model is relaxed.";
						return false;
					}
					const auto var = getEdgeRelaxed(e);
					s.floatValues.emplace(var, value.get<double>());
					return true;
				});
	} else {
		return handleTransit(
				allFlowVars,
				[&s, &err, this](const lib::DG::ExpandedVertex e, const nlohmann::json &value) {
					if(!value.is_number_integer()) {
						err << "Base solution data. Transit value is not an int, but the model is not relaxed.";
						return false;
					}
					const auto var = getEdge(e);
					s.integralValues.emplace(var, value.get<int>());
					return true;
				});
	}
}

void BaseModel::loadSolutionSetDependentVarsImpl(CombiOpt::LoadedSolution &s) const {
	const auto &dgHyper = owner.specification->dgHyper;
	const auto &dg = dgHyper.getGraph();
	const auto &dgExpanded = owner.getExpanded();
	const auto handleTransitInternal = [&s, &dgExpanded](auto acc,
	                                                     const auto &vars,
	                                                     const auto vHyper,
	                                                     auto &targetVals) {
//		constexpr bool DEBUG = false;
		const auto iter = vars.transitInternalFlow.find(vHyper);
		if(iter == end(vars.transitInternalFlow)) return;
		const auto tif = iter->second;
		const auto &gExpanded = dgExpanded.getGraph();
		const auto &vertexData = dgExpanded.getVertexData(vHyper);
		const auto vInput = vertexData.inputEdge;
		const auto vOutput = vertexData.outputEdge;
		assert(out_degree(vInput, gExpanded) == 1);
		assert(in_degree(vOutput, gExpanded) == 1);
		const auto vInputTrans = *adjacent_vertices(vInput, gExpanded).first;
		const auto vOutputTrans = *inv_adjacent_vertices(vOutput, gExpanded).first;
		assert(in_degree(vInputTrans, gExpanded) == 1);
		assert(out_degree(vOutputTrans, gExpanded) == 1);
		for(const auto vInTrans : vertexData.inVertices) {
//			if(DEBUG)
//				std::cout << "load: handleTransitInternal(" << model.getVarName(iter->second) << "): in? " << vInTrans
//				          << std::endl;
			if(vInTrans == vInputTrans) // skip inFlow edge
				continue;
//			if(DEBUG)
//				std::cout << "handleTransitInternal(" << model.getVarName(iter->second) << "): " << vInTrans << std::endl;
			for(const auto vTrans : asRange(adjacent_vertices(vInTrans, gExpanded))) {
				assert(out_degree(vTrans, gExpanded) == 1);
				const auto vOutTrans = *adjacent_vertices(vTrans, gExpanded).first;
//				if(DEBUG)
//					std::cout << "handleTransitInternal(" << model.getVarName(iter->second) << "): out? " << vOutTrans
//					          << std::endl;
				if(vOutTrans == vOutputTrans) // skip outFlow edge
					continue;
//				if(DEBUG)
//					std::cout << "handleTransitInternal(" << model.getVarName(iter->second) << "): " << vOutTrans
//					          << std::endl;
				assert(gExpanded[vTrans].kind == lib::DG::ExpandedVertexKind::TransitEdge);
				const auto iter = vars.allFlowVars.find(vTrans);
				assert(iter != vars.allFlowVars.end());
				const auto tVar = iter->second;
				acc += s.getVal(tVar);
			}
		}
		using AccType = decltype(acc);
		if(acc != AccType())
			targetVals.emplace(tif, acc);
	};
	if(specification.relaxed) {
		for(const auto v : asRange(vertices(dg))) {
			if(dg[v].kind != lib::DG::HyperVertexKind::Vertex) continue;
			const auto inFlow = s.getVal(getInRelaxed(v));
			auto vertexFlow = inFlow;
			for(const auto vAdj : asRange(inv_adjacent_vertices(v, dg)))
				vertexFlow += s.getVal(getEdgeRelaxed(dgExpanded.getExpandedFromHyperEdge(vAdj)));
			if(vertexFlow > 0.0)
				s.floatValues.emplace(getVertexRelaxed(v), vertexFlow);
			handleTransitInternal(CombiOpt::Float(), this->relaxedMode, v, s.floatValues);
		}
	} else {
		for(const auto v : asRange(vertices(dg))) {
			if(dg[v].kind == lib::DG::HyperVertexKind::Vertex) {
				const auto inFlow = s.getVal(getIn(v));
				const auto outFlow = s.getVal(getOut(v));
				if(inFlow > 0) s.integralValues.emplace(getIsInUsed(v), 1);
				if(outFlow > 0) s.integralValues.emplace(getIsOutUsed(v), 1);
				if(inFlow < outFlow) s.integralValues.emplace(getIsInLessOut(v), 1);
				if(inFlow > outFlow) s.integralValues.emplace(getIsInGreaterOut(v), 1);
				if(inFlow == 0 && outFlow == 0) s.integralValues.emplace(getIsInOutZero(v), 1);
				auto vertexFlow = inFlow;
				for(const auto vAdj : asRange(inv_adjacent_vertices(v, dg)))
					vertexFlow += s.getVal(getEdge(dgExpanded.getExpandedFromHyperEdge(vAdj)));
				if(vertexFlow > 0) {
					s.integralValues.emplace(getVertex(v), vertexFlow);
					s.integralValues.emplace(getIsVertexUsed(v), 1);
				}
				handleTransitInternal(CombiOpt::Int(), *this, v, s.integralValues);
			} else { // Edge
				const auto edgeFlow = s.getVal(getEdge(dgExpanded.getExpandedFromHyperEdge(v)));
				if(edgeFlow > 0) s.integralValues.emplace(getIsEdgeUsed(v), 1);

				const auto vReverse = dgHyper.getReverseEdge(v);
				if(vReverse != dg.null_vertex()) {
					// only if we are the lowest numbered
					if(get(boost::vertex_index_t(), dg, v) <= get(boost::vertex_index_t(), dg, vReverse)) {
						const auto reverseFlow = s.getVal(getEdge(dgExpanded.getExpandedFromHyperEdge(vReverse)));
						if(edgeFlow > 0 && reverseFlow > 0)
							s.integralValues.emplace(getIsEdgeBothReverseUsed(v), 1);
					}
				}
			}
		}
	}
}

bool BaseModel::hasListEntry(lib::DG::HyperVertex vHyper, const CombiOpt::Result &sol) const {
	if(specification.relaxed) {
		if(sol.getValue(getInRelaxed(vHyper)) != 0.0) return true;
		if(sol.getValue(getOutRelaxed(vHyper)) != 0.0) return true;
	} else {
		if(sol.getValue(getIn(vHyper)) != 0) return true;
		if(sol.getValue(getOut(vHyper)) != 0) return true;
	}
	return false;
}

std::vector<std::string> BaseModel::listHeaderEntries() const {
	return {"In", "Out"};
}

std::vector<std::string> BaseModel::listEntries(lib::DG::HyperVertex vHyper, const CombiOpt::Result &sol) const {
	if(specification.relaxed) {
		return {
				boost::lexical_cast<std::string>(sol.getValue(getInRelaxed(vHyper))),
				boost::lexical_cast<std::string>(sol.getValue(getOutRelaxed(vHyper)))
		};
	} else {
		return {
				boost::lexical_cast<std::string>(sol.getValue(getIn(vHyper))),
				boost::lexical_cast<std::string>(sol.getValue(getOut(vHyper)))
		};
	}
}

} // namespace mod::lib::HyperFlow