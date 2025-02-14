#include "Composer.hpp"

#include <mod/rule/Rule.hpp>
#include <mod/lib/RC/Evaluator.hpp>

namespace mod::rule {

struct Composer::Pimpl {
	Pimpl(const std::unordered_set<std::shared_ptr<Rule>> &database, LabelSettings labelSettings)
			: evaluator(database, labelSettings) {}
public:
	lib::RC::Evaluator evaluator;
};

Composer::Composer(const std::unordered_set<std::shared_ptr<Rule>> &database, LabelSettings labelSettings)
		: p(new Pimpl(database, labelSettings)) {}

Composer::~Composer() = default;

const std::unordered_set<std::shared_ptr<Rule>> &Composer::getRuleDatabase() const {
	return p->evaluator.getRuleDatabase();
}

const std::unordered_set<std::shared_ptr<Rule>> &Composer::getCreatedRules() const {
	return p->evaluator.getCreatedRules();
}

std::vector<std::shared_ptr<Rule>> Composer::eval(const RCExp::Expression &exp, bool onlyUnique, int verbosity) {
	return p->evaluator.eval(exp, onlyUnique, verbosity);
}

void Composer::print() const {
	p->evaluator.print();
}

} // namespace mod::rule