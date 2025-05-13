#ifndef MOD_HYPERFLOW_SOLUTION_HPP
#define MOD_HYPERFLOW_SOLUTION_HPP

#include <mod/BuildConfig.hpp>
#include <mod/dg/ForwardDecl.hpp>
#include <mod/hyperflow/ForwardDecl.hpp>

#include <memory>
#include <optional>
#include <variant>

namespace mod::hyperflow {

// rst-class: hyperflow::SolutionRange
// rst:
// rst:		A range of solutions for a flow model.
// rst:
// rst-class-start:
struct MOD_DECL SolutionRange {
	// rst: .. type:: const_iterator
	// rst:           iterator
	// rst:
	// rst:		Random-access iterator types for solutions in a range.
	struct const_iterator {
		using difference_type = int;
		using value_type = Solution;
		using reference = Solution;
		using iterator_category = std::random_access_iterator_tag;
		using pointer = void;
	public:
		const_iterator() = default;
	private:
		friend class SolutionRange;
		const_iterator(std::shared_ptr<Model> flow_, int i);
	public:
		MOD_DECL friend bool operator==(const const_iterator &a, const const_iterator &b);
		MOD_DECL friend bool operator!=(const const_iterator &a, const const_iterator &b);
		const_iterator &operator++();
		const_iterator operator++(int);
		MOD_DECL friend const_iterator operator+(const const_iterator &a, int i);
		Solution operator*() const;
		Solution operator[](int i) const;
	private:
		std::shared_ptr<Model> flow_;
		int i = 0;
	};
	using iterator = const_iterator;
private:
	friend class Model;
	SolutionRange(std::shared_ptr<Model> model, int first, int last);
public:
	// rst: .. function:: std::shared_ptr<Model> getModel()
	// rst:
	// rst:		:returns: the hyperflow model that produced these solutions.
	std::shared_ptr<Model> getModel() const;
	// rst: .. function:: friend std::ostream &operator<<(std::ostream &s, const SolutionRange &r)
	// rst:
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, const SolutionRange &r);
	// rst: .. function:: int size() const
	// rst:
	// rst:		:returns: the number of solutions in the range.
	int size() const;
	// rst: .. function:: const_iterator begin() const
	// rst:               const_iterator end() const
	// rst:
	// rst:		:returns: the begin and end iterators for the range.
	const_iterator begin() const;
	const_iterator end() const;
	// rst: .. function:: Solution operator[](int i) const
	// rst:
	// rst:		:returns: the `i` th solution in the range.
	// rst:		:throws: :class:`LogicError` if the index is out of bounds.
	Solution operator[](int i) const;
public:
	// rst: .. function:: void list() const
	// rst:
	// rst:		List overall information about each solution.
	void list() const;
	// rst: .. function:: void print() const
	// rst:               void print(const Printer &printer, const dg::PrintData &data) const
	// rst:
	// rst:		Print solutions using the settings in the given printer and the structure information in the data object.
	// rst:
	// rst:		The `data` defaults to `dg::PrintData(getModel()->getDG())` and `printer` defaults to `Printer()`.
	// rst:
	// rst:		:throws: :class:`LogicError` if the given print data does not belong to the underlying derivation graph.
	void print() const;
	void print(const Printer &printer, const dg::PrintData &data) const;
private:
	std::shared_ptr<Model> model;
	int first = 0, last = 0;
};
// rst-class-end:

// rst-class: hyperflow::Solution
// rst:
// rst:		A solution for a flow model.
// rst:
// rst-class-start:
class MOD_DECL Solution {
	friend class SolutionRange::const_iterator;
	Solution(std::shared_ptr<Model> model, int i);
public:
	// rst: .. function:: std::shared_ptr<Model> getModel()
	// rst:
	// rst:		:returns: the hyperflow model that produced the solution.
	std::shared_ptr<Model> getModel() const;
	// rst: .. function:: int getId() const
	// rst:
	// rst:		:returns: an ID for the solution which is unique among all solutions from the parent :class:`Model` object.
	int getId() const;
	// rst: .. function:: friend std::ostream &operator<<(std::ostream &s, const Solution &sol)
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, const Solution &sol);
	// rst: .. function:: friend bool operator==(const Solution &a, const Solution &b)
	// rst:               friend bool operator!=(const Solution &a, const Solution &b)
	// rst:               friend bool operator<(const Solution &a, const Solution &b)
	// rst:
	// rst:		Compares the solution objects, i.e., the values of `getModel()` and `getId()`.
	MOD_DECL friend bool operator==(const Solution &a, const Solution &b);
	MOD_DECL friend bool operator!=(const Solution &a, const Solution &b);
	MOD_DECL friend bool operator<(const Solution &a, const Solution &b);
public:
	// rst: .. function:: std::variant<int, double> getObjectiveValue() const
	// rst:
	// rst:		:returns: the objective value of the solution.
	std::variant<int, double> getObjectiveValue() const;
	// rst: .. function:: std::variant<int, double> eval(const LinExp &exp) const
	// rst:
	// rst:		:returns: the value of the given linear expression evaluated on the solution.
	std::variant<int, double> eval(const LinExp &exp) const;
	// rst: .. function:: void list() const
	// rst:
	// rst:		List overall information about the solution.
	void list() const;
	// rst: .. function:: std::pair<std::optional<std::string>, std::optional<std::string>> \
	// rst:               print() const
	// rst:               std::pair<std::optional<std::string>, std::optional<std::string>> \
	// rst:               print(const Printer &printer, const dg::PrintData &data) const
	// rst:
	// rst:		Print the solution using the settings in the given printer and the structure information in the data object.
	// rst:
	// rst:		The `data` defaults to `dg::PrintData(getModel()->getDG())` and `printer` defaults to `Printer()`.
	// rst:
	// rst:		:returns: the name of the PDF-files that will be compiled in post-processing for respectively the filtered and unfiltered depictions.
	// rst:			If either is not requested by the given `printer` the corresponding entry has no string.
	// rst:		:throws: :class:`LogicError` if the given print data does not belong to the underlying derivation graph.
	std::pair<std::optional<std::string>, std::optional<std::string>>
	print() const;
	std::pair<std::optional<std::string>, std::optional<std::string>>
	print(const Printer &printer, const dg::PrintData &data) const;
private:
	std::shared_ptr<Model> model;
	int i;
};
// rst-class-end:

}  // namespace mod::hyperflow

#endif // MOD_HYPERFLOW_SOLUTION_HPP