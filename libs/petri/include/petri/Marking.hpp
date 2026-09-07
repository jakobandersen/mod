#ifndef PETRI_MARKING_HPP
#define PETRI_MARKING_HPP

#include <petri/Net.hpp>

#include <boost/iterator/iterator_facade.hpp>

#include <vector>

namespace petri {

struct Marking {
	struct EnabledIterator : boost::iterator_facade<
			/**/ EnabledIterator,
			/**/ Transition,
			/**/ std::forward_iterator_tag,
			/**/ Transition> {
		EnabledIterator() = default;
		EnabledIterator(const Marking *m, int id);
	private:
		const Marking *m = nullptr;
		int id = -1;
	};

	struct EnabledRange {
		EnabledRange(const Marking &m);

		EnabledIterator begin() const {
			return {&m, 0};
		}

		EnabledIterator end() const {
			return {};
		}
	private:
		const Marking &m;
	};
	friend class EnabledIterator;
	friend class EnabledRange;
public:
	explicit Marking(const Net &net);
public:
	void syncSize();
	// Returns: the new amount of tokens.
	// Requires: m >= 0
	int add(Place p, int m);
	// (same)
	int remove(Place p, int m);
	int getNumTokens() const;
	bool isEnabled(Transition t) const;
	bool isEnabledNoCache(Transition t) const;
	bool isEnabledNoCache(Net::Vertex vt) const;
	void fire(Transition t);
	void backfire(Transition t);
	int operator[](Place p) const;
	// compares the raw counts, so both markings should have been synced first
	friend bool operator==(const Marking &a, const Marking &b);
	friend bool operator!=(const Marking &a, const Marking &b) { return !(a == b); }
private:
	void enable(Net::Vertex vt);
	void disable(Net::Vertex vt);
	void updateToEnabled(Net::Vertex vp);
	void updateToDisabled(Net::Vertex vp);
	void disableAll(Net::Vertex vp);
private:
	struct Trans {
		bool enabled = false;
		int next = -1, prev = -1;
	};
	const Net &net;
	std::vector<int> counts;
	std::vector<Trans> transitions;
};

// ============================================================================

inline bool Marking::isEnabled(Transition t) const {
	return transitions[t.getId()].enabled;
}

inline int Marking::operator[](Place p) const {
	return counts[p.getId()];
}

} // namespace petri

#endif // PETRI_MARKING_HPP
