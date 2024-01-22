#include "Evaluator.hpp"

#include <mod/Config.hpp>
#include <mod/graph/Graph.hpp>
#include <mod/rule/CompositionExpr.hpp>
#include <mod/rule/Rule.hpp>
#include <mod/lib/RC/ComposeRuleReal.hpp>
#include <mod/lib/RC/IO/Write.hpp>
#include <mod/lib/RC/MatchMaker/Common.hpp>
#include <mod/lib/RC/MatchMaker/Parallel.hpp>
#include <mod/lib/RC/MatchMaker/Sub.hpp>
#include <mod/lib/RC/MatchMaker/Super.hpp>

#include <boost/variant/static_visitor.hpp>

namespace mod::lib::RC {
namespace {

struct EvalVisitor : public boost::static_visitor<std::vector<std::shared_ptr<rule::Rule>>> {
	EvalVisitor(bool onlyUnique, int verbosity, IO::Logger logger, Evaluator &evaluator)
			: onlyUnique(onlyUnique), verbosity(verbosity), logger(logger), evaluator(evaluator) {}

	// Nullary/unary
	//----------------------------------------------------------------------

	std::vector<std::shared_ptr<rule::Rule> > operator()(std::shared_ptr<rule::Rule> r) {
		return {r};
	}

	std::vector<std::shared_ptr<rule::Rule>> operator()(const rule::RCExp::Union &par) {
		const auto doIt = [&par, this](auto &result) {
			for(const auto &subExp : par.getExpressions())
				for(const auto &r: subExp.applyVisitor(*this))
					result.insert(result.end(), r);
		};
		if(onlyUnique) {
			std::unordered_set<std::shared_ptr<rule::Rule>> result;
			doIt(result);
			return {result.begin(), result.end()};
		} else {
			std::vector<std::shared_ptr<rule::Rule>> result;
			doIt(result);
			return result;
		}
	}

	std::vector<std::shared_ptr<rule::Rule>> operator()(const rule::RCExp::Bind &bind) {
		std::vector<std::shared_ptr<rule::Rule>> result;
		result.push_back(evaluator.graphAsRuleCache.getBindRule(&bind.getGraph()->getGraph()));
		return result;
	}

	std::vector<std::shared_ptr<rule::Rule> > operator()(const rule::RCExp::Id &id) {
		std::vector<std::shared_ptr<rule::Rule> > result;
		result.push_back(evaluator.graphAsRuleCache.getIdRule(&id.getGraph()->getGraph()));
		return result;
	}

	std::vector<std::shared_ptr<rule::Rule> > operator()(const rule::RCExp::Unbind &unbind) {
		std::vector<std::shared_ptr<rule::Rule> > result;
		result.push_back(evaluator.graphAsRuleCache.getUnbindRule(&unbind.getGraph()->getGraph()));
		return result;
	}

	// Binary
	//----------------------------------------------------------------------

	template<typename Composer>
	std::vector<std::shared_ptr<rule::Rule>> composeTemplate(
			const rule::RCExp::ComposeBase &compose, Composer composer) {
		const auto doIt = [&compose, &composer, this](auto &result) {
			auto firstResult = compose.getFirst().applyVisitor(*this);
			auto secondResult = compose.getSecond().applyVisitor(*this);
			for(auto rFirst : firstResult) {
				for(auto rSecond : secondResult) {
					std::vector<lib::Rules::Real *> resultVec;
					auto reporter = [&resultVec](std::unique_ptr<lib::Rules::Real> r) {
						resultVec.push_back(r.release());
						return true;
					};
					composer(rFirst->getRule(), rSecond->getRule(), reporter);
					for(auto *r: resultVec) {
						if(compose.getDiscardNonchemical() && !r->isChemical()) {
							delete r;
							continue;
						}
						auto rWrapped = evaluator.checkIfNew(r);
						bool isNew = evaluator.addRule(rWrapped);
						if(isNew) evaluator.giveProductStatus(rWrapped);
						evaluator.suggestComposition(&rFirst->getRule(), &rSecond->getRule(), &rWrapped->getRule());
						result.insert(result.end(), rWrapped);
					}
				}
			}
		};
		if(onlyUnique) {
			std::unordered_set<std::shared_ptr<rule::Rule>> result;
			doIt(result);
			return {result.begin(), result.end()};
		} else {
			std::vector<std::shared_ptr<rule::Rule>> result;
			doIt(result);
			return result;
		}
	}

	std::vector<std::shared_ptr<rule::Rule>> operator()(const rule::RCExp::ComposeCommon &common) {
		const auto composer = [&common, this](const lib::Rules::Real &rFirst, const lib::Rules::Real &rSecond,
		                                      std::function<bool(std::unique_ptr<lib::Rules::Real>)> reporter) {
			RC::Common mm(matchMakerVerbosity(), logger, common.getMaxmimum(), common.getConnected());
			lib::RC::composeRuleRealByMatchMaker(rFirst, rSecond, mm, reporter, evaluator.labelSettings);
		};
		auto res = composeTemplate(common, composer);
		if(common.getIncludeEmpty()) {
			auto resEmpty = (*this)(rule::RCExp::ComposeParallel(
					common.getFirst(), common.getSecond(), common.getDiscardNonchemical()));
			res.insert(res.end(), resEmpty.begin(), resEmpty.end());
		}
		return res;
	}

	std::vector<std::shared_ptr<rule::Rule>> operator()(const rule::RCExp::ComposeParallel &common) {
		const auto composer = [this](const lib::Rules::Real &rFirst, const lib::Rules::Real &rSecond,
		                             std::function<bool(std::unique_ptr<lib::Rules::Real>)> reporter) {
			lib::RC::composeRuleRealByMatchMaker(rFirst, rSecond, lib::RC::Parallel(verbosity, logger),
			                                     reporter, evaluator.labelSettings);
		};
		return composeTemplate(common, composer);
	}

	std::vector<std::shared_ptr<rule::Rule>> operator()(const rule::RCExp::ComposeSub &sub) {
		const auto composer = [&sub, this](const lib::Rules::Real &rFirst, const lib::Rules::Real &rSecond,
		                                   std::function<bool(std::unique_ptr<lib::Rules::Real>)> reporter) {
			RC::Sub mm(matchMakerVerbosity(), logger, sub.getAllowPartial());
			lib::RC::composeRuleRealByMatchMaker(rFirst, rSecond, mm, reporter, evaluator.labelSettings);
		};
		return composeTemplate(sub, composer);
	}

	std::vector<std::shared_ptr<rule::Rule>> operator()(const rule::RCExp::ComposeSuper &super) {
		const auto composer = [&super, this](const lib::Rules::Real &rFirst, const lib::Rules::Real &rSecond,
		                                     std::function<bool(std::unique_ptr<lib::Rules::Real>)> reporter) {
			RC::Super mm(matchMakerVerbosity(), logger, super.getAllowPartial(), super.getEnforceConstraints());
			lib::RC::composeRuleRealByMatchMaker(rFirst, rSecond, mm, reporter, evaluator.labelSettings);
		};
		return composeTemplate(super, composer);
	}

	int matchMakerVerbosity() const {
		return std::max(0, verbosity - 10 + V_MorphismGen);
	}
private:
	const bool onlyUnique;
	const int verbosity;
	IO::Logger logger;
	Evaluator &evaluator;
};

} // namespace 

Evaluator::Evaluator(std::unordered_set<std::shared_ptr<rule::Rule>> database, LabelSettings labelSettings)
		: labelSettings(labelSettings), database(database) {
	if(labelSettings.type == LabelType::Term) {
		for(const auto &r : database) {
			const auto &term = get_term(r->getRule().getDPORule());
			if(!isValid(term)) {
				std::string msg = "Parsing failed for rule '" + r->getName() + "'. " + term.getParsingError();
				throw TermParsingError(std::move(msg));
			}
		}
	}
}

const std::unordered_set<std::shared_ptr<rule::Rule> > &Evaluator::getRuleDatabase() const {
	return database;
}

const std::unordered_set<std::shared_ptr<rule::Rule>> &Evaluator::getProducts() const {
	return products;
}

std::vector<std::shared_ptr<rule::Rule>> Evaluator::eval(const rule::RCExp::Expression &exp, bool onlyUnique, int verbosity) {
	struct PreEvalVisitor : public boost::static_visitor<void> {
		PreEvalVisitor(Evaluator &evaluator) : evaluator(evaluator) {}

		// Nullary/unary
		//----------------------------------------------------------------------

		void operator()(std::shared_ptr<rule::Rule> r) {
			if(evaluator.labelSettings.type == LabelType::Term) {
				const auto &term = get_term(r->getRule().getDPORule());
				if(!isValid(term)) {
					std::string msg = "Parsing failed for rule '" + r->getName() + "'. " + term.getParsingError();
					throw TermParsingError(std::move(msg));
				}
			}
			evaluator.addRule(r);
		}

		void operator()(const rule::RCExp::Union &par) {
			for(const auto &e : par.getExpressions()) e.applyVisitor(*this);
		}

		void operator()(const rule::RCExp::Bind &bind) {
			evaluator.addRule(evaluator.graphAsRuleCache.getBindRule(&bind.getGraph()->getGraph()));
		}

		void operator()(const rule::RCExp::Id &id) {
			evaluator.addRule(evaluator.graphAsRuleCache.getIdRule(&id.getGraph()->getGraph()));
		}

		void operator()(const rule::RCExp::Unbind &unbind) {
			evaluator.addRule(evaluator.graphAsRuleCache.getUnbindRule(&unbind.getGraph()->getGraph()));
		}

		// Binary
		//----------------------------------------------------------------------

		void operator()(const rule::RCExp::ComposeCommon &compose) {
			compose.getFirst().applyVisitor(*this);
			compose.getSecond().applyVisitor(*this);
		}

		void operator()(const rule::RCExp::ComposeParallel &compose) {
			compose.getFirst().applyVisitor(*this);
			compose.getSecond().applyVisitor(*this);
		}

		void operator()(const rule::RCExp::ComposeSub &compose) {
			compose.getFirst().applyVisitor(*this);
			compose.getSecond().applyVisitor(*this);
		}

		void operator()(const rule::RCExp::ComposeSuper &compose) {
			compose.getFirst().applyVisitor(*this);
			compose.getSecond().applyVisitor(*this);
		}
	private:
		Evaluator &evaluator;
	};
	exp.applyVisitor(PreEvalVisitor(*this));
	auto result = exp.applyVisitor(EvalVisitor(onlyUnique, verbosity, IO::Logger(std::cout), *this));
	return result;
}

void Evaluator::print() const {
	std::string fileNoExt = RC::Write::pdf(*this);
	IO::post() << "summaryRC \"" << fileNoExt << "\"" << std::endl;
}

const Evaluator::GraphType &Evaluator::getGraph() const {
	return rcg;
}

bool Evaluator::addRule(std::shared_ptr<rule::Rule> r) {
	return database.insert(r).second;
}

void Evaluator::giveProductStatus(std::shared_ptr<rule::Rule> r) {
	products.insert(r);
}

std::shared_ptr<rule::Rule> Evaluator::checkIfNew(lib::Rules::Real *rCand) const {
	for(auto rOther : database) {
		if(lib::Rules::makeIsomorphismPredicate(labelSettings.type, labelSettings.withStereo)
				(&rOther->getRule(), rCand)) {
			delete rCand;
			return rOther;
		}
	}
	return rule::Rule::makeRule(std::unique_ptr<lib::Rules::Real>(rCand));
}

void Evaluator::suggestComposition(const lib::Rules::Real *rFirst,
                                   const lib::Rules::Real *rSecond,
                                   const lib::Rules::Real *rResult) {
	Vertex vComp = getVertexFromArgs(rFirst, rSecond);
	Vertex vResult = getVertexFromRule(rResult);
	for(Vertex vOut : asRange(adjacent_vertices(vComp, rcg))) {
		if(vOut == vResult) return;
	}
	[[maybe_unused]] std::pair<Edge, bool> pResult = add_edge(vComp, vResult, {EdgeKind::Result}, rcg);
	assert(pResult.second);
}

Evaluator::Vertex Evaluator::getVertexFromRule(const lib::Rules::Real *r) {
	auto iter = ruleToVertex.find(r);
	if(iter == end(ruleToVertex)) {
		Vertex v = add_vertex(rcg);
		rcg[v].kind = VertexKind::Rule;
		rcg[v].rule = r;
		iter = ruleToVertex.insert(std::make_pair(r, v)).first;
	}
	return iter->second;
}

Evaluator::Vertex Evaluator::getVertexFromArgs(const lib::Rules::Real *rFirst, const lib::Rules::Real *rSecond) {
	Vertex vFirst = getVertexFromRule(rFirst),
			vSecond = getVertexFromRule(rSecond);
	auto iter = argsToVertex.find(std::make_pair(rFirst, rSecond));
	if(iter != end(argsToVertex)) return iter->second;
	Vertex vComp = add_vertex(rcg);
	argsToVertex.insert(std::make_pair(std::make_pair(rFirst, rSecond), vComp));
	rcg[vComp].kind = VertexKind::Composition;
	rcg[vComp].rule = nullptr;
	[[maybe_unused]] std::pair<Edge, bool>
			pFirst = add_edge(vFirst, vComp, {EdgeKind::First}, rcg),
			pSecond = add_edge(vSecond, vComp, {EdgeKind::Second}, rcg);
	assert(pFirst.second);
	assert(pSecond.second);
	return vComp;
}

} // namespace mod::lib::RCd