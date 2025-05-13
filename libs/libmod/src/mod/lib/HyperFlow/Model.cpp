#include "Model.hpp"

#include <mod/Error.hpp>
#include <mod/hyperflow/LinExp.hpp>
#include <mod/lib/CombiOpt/LinConstraintAny.hpp>
#include <mod/lib/CombiOpt/LoadedSolution.hpp>
#include <mod/lib/CombiOpt/Model.hpp>
#include <mod/lib/CombiOpt/Solver.hpp>
#include <mod/lib/DG/Expanded.hpp>
#include <mod/lib/DG/Hyper.hpp>
#include <mod/lib/Graph/Graph.hpp>
#include <mod/lib/HyperFlow/ModuleRegistry.hpp>
#include <mod/lib/HyperFlow/Specification.hpp>
#include <mod/lib/HyperFlow/Modules/Base.hpp>
#include <mod/lib/HyperFlow/Modules/Module.hpp>
#include <mod/lib/IO/IO.hpp>
#include <mod/lib/IO/Json.hpp>

#include <jla_boost/graph/PairToRangeAdaptor.hpp>

#include <boost/lexical_cast.hpp>

#include <iomanip>
#include <iostream>

namespace mod::lib::HyperFlow {

Model::Model(std::unique_ptr<const Specification> specification_, const std::string &ilpSolver, int verbosity)
		: specification(std::move(specification_)) {
	Transits transits = ModelCreator::getTransits(*specification);
	this->dgExpanded = std::make_unique<lib::DG::Expanded>(
			this->specification->dgHyper,
			transits.expressed,
			transits.deleted);
	modules = ModelCreator::createModules(*specification, *this);

	CombiOpt::Model model;
	for(auto &m: modules) m.second->createVariables(model);
	for(auto &m: modules) m.second->createConstraints(model);

	model.setObjectiveFunction(compileExpr(specification->getObjectiveFunction()));

	std::unordered_set<CombiOpt::Var> enumerationVars;
	for(const auto &v: specification->getEnumerationVars()) {
		const auto expr = compileExpr(v.v);
		expr.forEach([&enumerationVars](const auto coef, const auto var) {
			enumerationVars.insert(var);
		});
	}

	const auto &dgExp = dgExpanded->getGraph();
	const auto &baseModel = getModule<BaseModel>();
	for(const auto e: specification->getTransitEnumeration()) {
		const auto &vData = dgExpanded->getVertexData(e.v);
		for(const auto vInExp: vData.inVertices) {
			for(const auto vTransit: asRange(adjacent_vertices(vInExp, dgExp))) {
				assert(dgExp[vTransit].kind == DG::ExpandedVertexKind::TransitEdge);
				if(!specification->getModule<BaseSpecification>().getRelaxed()) {
					const auto var = baseModel.getEdge(vTransit);
					enumerationVars.insert(var);
				} else {
					const auto var = baseModel.getEdgeRelaxed(vTransit);
					enumerationVars.insert(var);
				}
			}
		}
	}
	solver = std::make_unique<CombiOpt::Solver>(
			std::move(model), std::move(enumerationVars), ilpSolver, verbosity, std::cout);
}

Model::~Model() = default;

const lib::DG::Expanded &Model::getExpanded() const {
	return *dgExpanded;
}

bool Model::hasModule(std::type_index id) const {
	return modules.find(id) != modules.end();
}

const ModelModule &Model::getModule(std::type_index id) const {
	const auto iter = modules.find(id);
	assert(iter != modules.end());
	return *iter->second;
}

namespace {

struct CompileExprVisitor {
	CompileExprVisitor(const Model &m) : m(m) {}

	CombiOpt::LinExpAny operator()(const hyperflow::VarSumCustom &v) const {
		const auto *vs = getVarSetCustom(v.id);
		return vs->compileExpr(m);
	}

	CombiOpt::LinExpAny operator()(const hyperflow::VarCustom &v) const {
		const auto *vs = getVarSetCustom(v.id);
		return vs->compileExpr(m, v.name);
	}

	CombiOpt::LinExpAny operator()(const hyperflow::VarSumVertex &v) const {
		const auto *vs = getVarSetVertex(v.id);
		return vs->compileExpr(m);
	}

	CombiOpt::LinExpAny operator()(const hyperflow::VarVertex &v) const {
		const auto *vs = getVarSetVertex(v.id);
		const auto &dg = m.specification->dgHyper;
		if(v.v.getDG() != dg.getNonHyper().getAPIReference())
			throw LogicError("Can not compile linear expression with vertex indexed variable set '"
			                 + v.id + "'. Vertex '" + boost::lexical_cast<std::string>(v.v)
			                 + "' is not from the underlying derivation graph.");
		const auto vertex = vertices(dg.getGraph()).first[v.v.getId()];
		return vs->compileExpr(m, vertex);
	}

	CombiOpt::LinExpAny operator()(const hyperflow::VarVertexGraph &v) const {
		const auto *vs = getVarSetVertex(v.id);
		const auto &dg = m.specification->dgHyper;
		if(!dg.isVertexGraph(&v.g->getGraph()))
			throw LogicError("Can not compile linear expression with graph indexed variable set '"
			                 + v.id + "'. Graph '" + boost::lexical_cast<std::string>(*v.g)
			                 + "' is not represented in the underlying derivation graph.");
		const auto vertex = dg.getVertexFromGraph(&v.g->getGraph());
		return vs->compileExpr(m, vertex);
	}

	CombiOpt::LinExpAny operator()(const hyperflow::VarSumEdge &v) const {
		const auto *vs = getVarSetEdge(v.id);
		return vs->compileExpr(m);
	}

	CombiOpt::LinExpAny operator()(const hyperflow::VarEdge &v) const {
		const auto *vs = getVarSetEdge(v.id);
		const auto &dg = m.specification->dgHyper;
		const auto &dgGraph = dg.getGraph();
		if(!v.edge)
			throw LogicError("Can not compile linear expression with hyperedge indexed variabled set '"
			                 + v.id + "'. The given hyperedge is null.");
		if(&v.edge.getDG()->getHyper() != &dg)
			throw LogicError("Can not compile linear expression with hyperedge indexed variable set '"
			                 + v.id + "'. The given hyperedge, " + boost::lexical_cast<std::string>(v.edge)
			                 + ", is not from the underlying derivation graph.");
		const auto eId = v.edge.getId();
		const auto vertex = *(vertices(dgGraph).first + eId);
		assert(get(boost::vertex_index_t(), dgGraph, vertex) == eId);
		assert(dgGraph[vertex].kind == lib::DG::HyperVertexKind::Edge);
		return vs->compileExpr(m, vertex);
	}
private:
	const ModuleRegistry::VariableSetVertex *getVarSetVertex(const std::string &varId) const {
		const auto *vs = getVarSet(varId);
		if(vs->varType != ModuleRegistry::VarType::Vertex) {
			throw LogicError("Can not compile linear expression. Variable set '"
			                 + varId + "' is not a vertex set, but a(n) " +
			                 boost::lexical_cast<std::string>(vs->varType) + " set.");
		}
		if(!m.specification->hasModule(vs->owner)) {
			throw LogicError("Can not compile linear expression. The module for variable set "
			                 + varId + " is not part of the flow model.");
		}
		return vs->asVertex();
	}

	const ModuleRegistry::VariableSetEdge *getVarSetEdge(const std::string &varId) const {
		const auto *vs = getVarSet(varId);
		if(vs->varType != ModuleRegistry::VarType::Edge) {
			throw LogicError("Can not compile linear expression. Variable set '"
			                 + varId + "' is not an edge set, but a " +
			                 boost::lexical_cast<std::string>(vs->varType) + " set.");
		}
		if(!m.specification->hasModule(vs->owner)) {
			throw LogicError("Can not compile linear expression. The module for variable set "
			                 + varId + " is not part of the flow model.");
		}
		return vs->asEdge();
	}

	const ModuleRegistry::VariableSetCustom *getVarSetCustom(const std::string &varId) const {
		const auto *vs = getVarSet(varId);
		if(vs->varType != ModuleRegistry::VarType::Custom) {
			throw LogicError("Can not compile linear expression. Variable set '"
			                 + varId + "' is not a custom set, but a " +
			                 boost::lexical_cast<std::string>(vs->varType) + " set.");
		}
		if(!m.specification->hasModule(vs->owner)) {
			throw LogicError("Can not compile linear expression. The module for variable set "
			                 + varId + " is not part of the flow model.");
		}
		return vs->asCustom();
	}

	const ModuleRegistry::VariableSet *getVarSet(const std::string &varId) const {
		const ModuleRegistry &reg = ModuleRegistry::get();
		const auto *varSet = reg.getVariableSet(varId);
		if(!varSet) {
			throw LogicError("Can not compile linear expression. Variable set '"
			                 + varId + "' has not been registered.");
		}
		return varSet;
	}
private:
	const Model &m;
};

} // namespace

CombiOpt::LinConstraintAny Model::compileConstraint(const hyperflow::LinConstraint &c) const {
	CombiOpt::LinExpAny exp = compileExpr(c.exp);
	CombiOpt::Relation rel = [&c]() {
		switch(c.relation) {
		case hyperflow::LinConstraint::Relation::Leq:
			return CombiOpt::Relation::Leq;
		case hyperflow::LinConstraint::Relation::Eq:
			return CombiOpt::Relation::Eq;
		case hyperflow::LinConstraint::Relation::Geq:
			return CombiOpt::Relation::Geq;
		}
		__builtin_unreachable();
	}();
	return CombiOpt::LinConstraintAny(std::move(exp), rel, c.bound);
}

CombiOpt::LinExpAny Model::compileExpr(const mod::hyperflow::LinExp &e) const {
	CombiOpt::LinExpAny res;
	CompileExprVisitor vis(*this);
	for(const auto &el: e.getIntElements())
		res += CombiOpt::Int(el.first) * el.second.applyVisitor(vis);
	for(const auto &el: e.getFloatElements())
		res += CombiOpt::Float(el.first) * el.second.applyVisitor(vis);
	return res;
}

nlohmann::json Model::dump() const {
	auto j = nlohmann::json::array();
	for(int i = 0; i != solutions.size(); ++i) {
		const auto &sol = solutions[i];
		nlohmann::json jSol;
		jSol["id"] = i;
		if(solver->isObjValueIntegral())
			jSol["objectiveValue"] = sol.getObjValInt().getValue();
		else
			jSol["objectiveValue"] = sol.getObjValFloat().getValue();

		nlohmann::json ms;
		for(const auto &mp: modules)
			ms[mp.second->getSpec().getName()] = mp.second->dump(sol);
		jSol["modules"] = std::move(ms);
		j.push_back(std::move(jSol));
	}
	return j;
}

bool Model::load(const nlohmann::json &j, int verbosity, std::ostream &err) {
	static const nlohmann::json schema = R"({
		"$schema": "http://json-schema.org/draft-07/schema#",
		"type": "array",
		"items": {
			"type": "object",
			"properties": {
				"id": {"type": "integer", "minimum": 0},
				"objectiveValue": {"type": "number"},
				"modules": {"type": "object"}
			},
			"required": ["id", "objectiveValue", "modules"]
		}
	})"_json;
	static const nlohmann::json_schema::json_validator validator(schema);
	if(!IO::validateJson(j, validator, err, "Data does not conform to schema:"))
		return false;

	int solCount = 0;
	for(const auto &jSol: j) {
		const int id = jSol["id"];
		if(solCount != id) {
			err << "Solution offset " << solCount << " has wrong ID, " << id << ".";
			return false;
		}
		++solCount;

		CombiOpt::LoadedSolution sol;
		const auto &jObj = jSol["objectiveValue"];
		if(jObj.is_number_float())
			sol.objVal = CombiOpt::Float(jObj.get<double>());
		else
			sol.objVal = CombiOpt::Int(jObj.get<int>());

		for(const auto &mp: jSol["modules"].items()) {
			const auto &name = mp.key();
			const auto *mod = ModuleRegistry::get().getModule(name);
			if(!mod) {
				err << "Module in solution, '" << name << "', has not been registered.";
				err << "\nRegistered modules:";
				for(const auto &m: ModuleRegistry::get().getAllModules())
					err << "\n   " << m.second.name;
				return false;
			}
			assert(hasModule(mod->id));
			const auto res = getModule(mod->id).load(mp.value(), sol, err);
			if(!res) return false;
		}
		for(const auto &mp: modules)
			mp.second->loadSolutionSetDependentVars(sol);

		auto res = solver->loadSolution(std::move(sol), verbosity);
		if(!res) {
			err << "Solution load failed for ID " << id << ".";
			return false;
		}
		solutions.push_back(std::move(res));
	}
	return true;
}

const CombiOpt::Solver &Model::getSolver() const {
	return *solver;
}

bool Model::isEnumVarsIntegral() const {
	return solver->isEnumVarsIntegral();
}

bool Model::isObjectiveValueIntegral() const {
	return solver->isObjValueIntegral();
}

void Model::printEnumerationTree(std::ostream &s) const {
	solver->printEnumerationTree(s);
}

SolutionRange Model::findSolutions(int maxNumSolutions, int verbosity, int ilpVerbosity) {
	const auto first = solutions.size();
	const auto absGap = specification->getAbsGap();
	for(int i = 0; i != maxNumSolutions; ++i) {
		if(i == 0 && verbosity > 0)
			std::cout << "Enumerating " << maxNumSolutions << " solutions\n";
		CombiOpt::Result res = solver->next(verbosity, ilpVerbosity);
		if(!res) break;
		//			for(auto &pred : base.solutionPredicates) {
		//				if(!pred(rawSol)) {
		//					response.solType = ILP::CallbackResponse::SolutionType::Rejected;
		//					return;
		//				}
		//			}
		// handle softAbsGap
		if(!solutions.empty() && absGap) {
			const bool isInt = solver->isObjValueIntegral();
			const bool stop =
					isInt ? solutions.front().getObjValInt() + *absGap < res.getObjValInt()
					      : solutions.front().getObjValFloat() + lib::CombiOpt::Float(*absGap) <
					        res.getObjValFloat();
			if(stop) {
				if(verbosity > 0) {
					if(isInt)
						std::cout << "Enumeration stopped due to softAbsGap. Obj = " << res.getObjValInt() << " > "
						          << solutions.front().getObjValInt() << " + " << *absGap << std::endl;
					else
						std::cout << "Enumeration stopped due to softAbsGap. Obj = " << res.getObjValFloat() << " > "
						          << solutions.front().getObjValFloat() << " + " << *absGap << std::endl;
				}
				break;
			}
		}
		solutions.push_back(std::move(res));
	}
	const auto last = solutions.size();
	return SolutionRange(this, first, last);
}

SolutionRange Model::loadSolution(CombiOpt::LoadedSolution &&s, int verbosity) {
	for(const auto &p: modules)
		p.second->loadSolutionSetDependentVars(s);
	const auto first = solutions.size();
	CombiOpt::Result res = solver->loadSolution(std::move(s), verbosity);
	if(!res) MOD_ABORT;
	solutions.push_back(std::move(res));
	const auto last = solutions.size();
	return SolutionRange(this, first, last);
}

SolutionRange Model::getSolutions() const {
	return SolutionRange(this, 0, solutions.size());
}

void Model::listSolution(std::ostream &s, const CombiOpt::Result &sol) const {
	if(solver->isObjValueIntegral())
		s << "Objective value (integral): " << sol.getObjValInt() << '\n';
	else
		s << "Objective value (non-integral): " << sol.getObjValFloat() << '\n';
	const auto &dg = specification->dgHyper.getGraph();
	std::vector<lib::DG::HyperVertex> vs;
	for(const auto vHyper: asRange(vertices(dg))) {
		if(dg[vHyper].kind != lib::DG::HyperVertexKind::Vertex) continue;
		const bool print = std::any_of(modules.begin(), modules.end(), [vHyper, &sol](const auto &p) {
			const auto &mod = *p.second;
			return mod.hasListEntry(vHyper, sol);
		});
		if(print) vs.push_back(vHyper);
	}
	// print in order of graph names
	std::sort(begin(vs), end(vs), [&dg](const auto a, const auto b) {
		return dg[a].graph->getName() < dg[b].graph->getName();
	});

	std::vector<std::vector<std::string>> table;
	std::vector<std::string> header = {"Vertex/Graph"};
	for(const auto &mod: modules) {
		auto res = mod.second->listHeaderEntries();
		header.insert(header.end(), std::make_move_iterator(res.begin()), std::make_move_iterator(res.end()));
	}
	table.push_back(std::move(header));
	for(const auto vHyper: vs) {
		std::vector<std::string> line = {dg[vHyper].graph->getName()};
		for(const auto &mod: modules) {
			auto res = mod.second->listEntries(vHyper, sol);
			line.insert(line.end(), std::make_move_iterator(res.begin()), std::make_move_iterator(res.end()));
		}
		table.push_back(std::move(line));
	}

	std::vector<std::size_t> columnWidths(table.front().size(), 0);
	for(const auto &line: table) {
		assert(line.size() == columnWidths.size());
		for(int i = 0; i != columnWidths.size(); ++i)
			columnWidths[i] = std::max(columnWidths[i], 1 + line[i].size());
	}
	for(const auto &line: table) {
		for(int i = 0; i != columnWidths.size(); ++i)
			s << std::left << std::setw(columnWidths[i]) << line[i];
		s << '\n';
	}
}

// ========================================================================================================

SolutionRange::iterator SolutionRange::begin() const {
	if(model) return model->solutions.begin() + first;
	else return {};
}

SolutionRange::iterator SolutionRange::end() const {
	if(model) return model->solutions.begin() + last;
	else return {};
}

int SolutionRange::size() const {
	return end() - begin();
}

const CombiOpt::Result &SolutionRange::operator[](int i) const {
	return begin()[i];
}

} // namespace mod::lib::HyperFlow