#ifndef MOD_LIB_TERM_WAM_HPP
#define MOD_LIB_TERM_WAM_HPP

#include <mod/Error.hpp>

#include <cassert>
#include <limits>
#include <stack>
#include <utility>
#include <vector>

// see http://wambook.sourceforge.net/

namespace mod::lib {
struct StringStore;
} // namespace mod::lib
namespace mod::lib::Term {

const StringStore &getStrings();

enum class AddressType : int { // order matters, used in operator<
	Heap = 0, Temp = 1
};

struct Address {
	AddressType type;
	std::size_t addr;
public:
	friend bool operator==(Address lhs, Address rhs) {
		return lhs.type == rhs.type && lhs.addr == rhs.addr;
	}

	friend bool operator!=(Address lhs, Address rhs) {
		return !(lhs == rhs);
	}

	friend bool operator<(Address lhs, Address rhs) {
		if(lhs.type != rhs.type) return lhs.type < rhs.type;
		return lhs.addr < rhs.addr;
	}

	friend Address operator+(Address addr, std::size_t k) {
		return Address{addr.type, addr.addr + k};
	}

	// implemented in IO/Write.cpp file
	friend std::ostream &operator<<(std::ostream &s, Address addr);
};

struct Cell {
	enum class Tag : char {
		STR, // structure pointer
		REF, // variable pointer / variable
		Structure // actual structure
	};
	struct Ref {
		Address addr;
	};
	struct Struct {
		int arity;
		std::size_t name;
	};
public:
	Tag tag;
	union {
		Ref STR;
		Ref REF;
		Struct Structure;
	};
public:
	static Cell makeSTR(Address addr) {
		Cell cell;
		cell.tag = Tag::STR;
		cell.STR.addr = addr;
		return cell;
	}

	static Cell makeREF(Address addr) {
		Cell cell;
		cell.tag = Tag::REF;
		cell.REF.addr = addr;
		return cell;
	}

	static Cell makeStructure(std::size_t arity, std::size_t name) {
		Cell cell;
		cell.tag = Tag::Structure;
		cell.Structure.arity = arity;
		cell.Structure.name = name;
		return cell;
	}
};

struct Wam;

struct MGU {
	MGU(std::size_t preHeapSize) : preHeapSize(preHeapSize) {}
	bool isRenaming(const Wam &machine) const;
	bool isSpecialisation(const Wam &machine) const;
public:
	std::size_t preHeapSize;
	std::vector<Address> bindings; // stack of addresses of REFs that were self-references before
	enum class Status {
		Exists, Fail
	} status = Status::Exists;
	Address errorLeft, errorRight;
};

struct Wam {
	void verify() const;

	Address putStructurePtr(std::size_t addr) {
		heap.push_back(Cell::makeSTR({AddressType::Heap, addr}));
		return Address{AddressType::Heap, heap.size() - 1};
	}

	Address putRefPtr() {
		heap.push_back(Cell::makeREF({AddressType::Heap, heap.size()}));
		return Address{AddressType::Heap, heap.size() - 1};
	}

	Address putStructure(std::size_t name, std::size_t arity) {
		heap.push_back(Cell::makeStructure(arity, name));
		return Address{AddressType::Heap, heap.size() - 1};
	}

	Address copyFromTemp(std::size_t addrTemp) {
		const auto addr = deref({AddressType::Temp, addrTemp});
		if(addr.type == AddressType::Heap) return addr;
		Cell &cellTemp = getCell(addr);
		if(cellTemp.tag == Cell::Tag::REF) {
			// put a new variable on the heap, and bind the temp variable to it
			const auto addrHeap = putRefPtr();
			cellTemp.REF.addr = addrHeap;
			return addrHeap;
		}
		// Copy the structure recursively
		assert(cellTemp.tag == Cell::Tag::Structure);
		// first copy the header and overwrite it in temp
		const auto arity = cellTemp.Structure.arity;
		const auto addrHeap = putStructure(cellTemp.Structure.name, arity);
		cellTemp.tag = Cell::Tag::STR;
		cellTemp.STR.addr = addrHeap;
		// now make space for each argument, and copy singleton cells
		std::vector<bool> doCopy(arity, false);
		for(int i = 1; i <= arity; ++i) {
			const auto addrArg = deref(addr + i);
			assert(getCell(addrArg).tag != Cell::Tag::STR);
			if(getCell(addrArg).tag == Cell::Tag::REF) {
				const auto heapAddrArg = putRefPtr();
				//				getCell(addrArg).REF.addr = heapAddrArg; // TODO: why was this here?
				if(addrArg.type == AddressType::Heap) {
					// bind the new to the old
					getCell(heapAddrArg).REF.addr = addrArg;
				} else {
					// bind the temp to the heap
					getCell(addrArg).REF.addr = heapAddrArg;
				}
			} else {
				assert(getCell(addrArg).tag == Cell::Tag::Structure);
				const auto arity = getCell(addrArg).Structure.arity;
				if(arity == 0) {
					// just copy it inline
					const auto addrNew = putStructure(getCell(addrArg).Structure.name, arity);
					getCell(addrArg).tag = Cell::Tag::STR;
					getCell(addrArg).STR.addr = addrNew;
				} else {
					// put a dummy to reserve the space
					putStructurePtr(-1);
					doCopy[i - 1] = true;
				}
			}
		}
		// copy them recursively
		for(int i = 1; i <= arity; ++i) {
			if(!doCopy[i - 1]) continue;
			const auto argAddr = deref(addr + i);
			assert(getCell(argAddr).tag == Cell::Tag::Structure);
			const auto targetAddr = addrHeap + i;
			auto &targetCell = getCell(targetAddr);
			targetCell.tag = Cell::Tag::STR;
			if(argAddr.type == AddressType::Temp) {
				// the targetCell reference may now dangle
				const auto argCopyAddr = copyFromTemp(argAddr.addr);
				getCell(targetAddr).STR.addr = argCopyAddr;
			} else {
				targetCell.STR.addr = argAddr;
			}
		}
		return addrHeap;
	}

	//	MGU unifyHeapHeap(std::size_t lhs, std::size_t rhs) {
	//		MGU mgu(heap.size());
	//		unifyHeapHeap(lhs, rhs, mgu);
	//		return mgu;
	//	}

	void unifyHeapHeap(std::size_t lhs, std::size_t rhs, MGU &mgu);

	MGU unifyHeapTemp(std::size_t lhs, std::size_t rhs) {
		MGU mgu(heap.size());
		unifyHeapTemp(lhs, rhs, mgu);
		return mgu;
	}

	void unifyHeapTemp(std::size_t lhs, std::size_t rhs, MGU &mgu);

	Address deref(Address addr) const {
		Cell cell = getCell(addr);
		switch(cell.tag) {
		case Cell::Tag::STR:
			return deref(cell.STR.addr);
		case Cell::Tag::REF:
			return cell.REF.addr == addr ? addr : deref(cell.REF.addr);
		case Cell::Tag::Structure:
			return addr;
		}
		MOD_ABORT;
	}

	Cell &getCell(Address addr) {
		switch(addr.type) {
		case AddressType::Heap:
			return heap[addr.addr];
		case AddressType::Temp:
			return temp[addr.addr];
		}
		MOD_ABORT;
	}

	const Cell &getCell(Address addr) const {
		switch(addr.type) {
		case AddressType::Heap:
			return heap[addr.addr];
		case AddressType::Temp:
			return temp[addr.addr];
		}
		MOD_ABORT;
	}

	const std::vector<Cell> &getHeap() const {
		return heap;
	}

	const std::vector<Cell> &getTemp() const {
		return temp;
	}

	void setTemp(const Wam &other) {
		temp = other.heap;
		for(Cell &cell: temp) {
			switch(cell.tag) {
			case Cell::Tag::REF:
				assert(cell.REF.addr.type == AddressType::Heap);
				cell.REF.addr.type = AddressType::Temp;
				break;
			case Cell::Tag::STR:
				assert(cell.STR.addr.type == AddressType::Heap);
				cell.STR.addr.type = AddressType::Temp;
				break;
			case Cell::Tag::Structure:
				break;
			}
		}
	}

	void swapHeapTemp() {
		using std::swap;
		swap(heap, temp);
	}

	void revert(const MGU &mgu) {
		for(const Address &a: mgu.bindings) {
			Cell &c = getCell(a);
			assert(c.tag == Cell::Tag::REF);
			assert(c.REF.addr != a);
			c.REF.addr = a;
		}
		assert(heap.size() >= mgu.preHeapSize);
		heap.resize(mgu.preHeapSize);
	}

	void appendHeapFrom(const Wam &mOther) {
		const int offset = heap.size();
		heap.reserve(heap.size() + mOther.heap.size());
		for(Cell c: mOther.heap) {
			switch(c.tag) {
			case Cell::Tag::STR:
				assert(c.STR.addr.type == AddressType::Heap);
				c.STR.addr.addr += offset;
				break;
			case Cell::Tag::REF:
				assert(c.REF.addr.type == AddressType::Heap);
				c.REF.addr.addr += offset;
				break;
			case Cell::Tag::Structure:
				break;
			}
			heap.push_back(c);
		}
	}
private:
	std::vector<Cell> heap;
	std::vector<Cell> temp;
};

//------------------------------------------------------------------------------
// Implementation
//------------------------------------------------------------------------------

// MGU
//------------------------------------------------------------------------------

inline bool MGU::isRenaming(const Wam &machine) const {
	assert(status == Status::Exists);
	if(preHeapSize != machine.getHeap().size()) return false;
	std::vector<bool> isTarget(preHeapSize, false);
	for(auto addr: bindings) {
		// addr may be in either temp or heap
		addr = machine.deref(addr);
		assert(addr.type == AddressType::Heap);
		Cell target = machine.getCell(addr);
		if(target.tag != Cell::Tag::REF) return false;
		if(isTarget[addr.addr]) return false;
		isTarget[addr.addr] = true;
	}
	return true;
}

inline bool MGU::isSpecialisation(const Wam &machine) const {
	assert(status == Status::Exists);
	if(preHeapSize != machine.getHeap().size()) return false;
	for(auto addr: bindings) {
		// only temp addresses should be bound
		if(addr.type == AddressType::Heap) return false;
	}
	return true;
}

// WAM
//------------------------------------------------------------------------------

inline void Wam::unifyHeapHeap(std::size_t lhs, std::size_t rhs, MGU &mgu) {
	using P = std::pair<std::size_t, std::size_t>;
	std::stack<P, std::vector<P> > stack;
	stack.emplace(lhs, rhs);
	while(!stack.empty()) {
		Address lhsAddr = deref(Address{AddressType::Heap, stack.top().first});
		Address rhsAddr = deref(Address{AddressType::Heap, stack.top().second});
		assert(lhsAddr.type == AddressType::Heap);
		assert(rhsAddr.type == AddressType::Heap);
		std::size_t lhs = lhsAddr.addr, rhs = rhsAddr.addr;
		stack.pop();
		if(lhs == rhs) continue;
		Cell leftCell = heap[lhs], rightCell = heap[rhs];
		assert(leftCell.tag != Cell::Tag::STR);
		assert(rightCell.tag != Cell::Tag::STR);
		if(leftCell.tag != Cell::Tag::REF && rightCell.tag == Cell::Tag::REF) {
			std::swap(lhs, rhs);
			std::swap(leftCell, rightCell);
		}

		if(leftCell.tag == Cell::Tag::REF) {
			// bind
			if(rightCell.tag == Cell::Tag::REF && rightCell.REF.addr < leftCell.REF.addr) {
				// right = left
				assert(heap[rhs].REF.addr.type == AddressType::Heap);
				heap[rhs].REF.addr.addr = lhs;
				mgu.bindings.push_back(Address{AddressType::Heap, rhs});
			} else {
				// left = right
				assert(heap[lhs].REF.addr.type == AddressType::Heap);
				heap[lhs].REF.addr.addr = rhs;
				mgu.bindings.push_back(Address{AddressType::Heap, lhs});
			}
		} else {
			assert(leftCell.tag == Cell::Tag::Structure);
			assert(rightCell.tag == Cell::Tag::Structure);
			if(leftCell.Structure.name != rightCell.Structure.name
			   || leftCell.Structure.arity != rightCell.Structure.arity) {
				mgu.status = MGU::Status::Fail;
				mgu.errorLeft = Address{AddressType::Heap, lhs};
				mgu.errorRight = Address{AddressType::Heap, rhs};
				return;
			}
			for(std::size_t i = leftCell.Structure.arity; i > 0; i--)
				stack.emplace(lhs + i, rhs + i);
		}
	}
}

inline void Wam::verify() const {
	// nothing in the heap should point at temp
	for(Cell cell: heap) {
		switch(cell.tag) {
		case Cell::Tag::REF:
			if(cell.REF.addr.type != AddressType::Heap) MOD_ABORT;
			break;
		case Cell::Tag::STR:
			if(cell.STR.addr.type != AddressType::Heap) MOD_ABORT;
			break;
		case Cell::Tag::Structure:
			break;
		}
	}
}

inline void Wam::unifyHeapTemp(std::size_t lhsIndex, std::size_t rhsIndex, MGU &mgu) {
	verify();
	// deref(rhsIndex) will point to heap
	// This destroys temp.
	// This method acts as the outer-most loop that creates get_structure, unify_variable and unify_value instructions
	using P = std::pair<Address, Address>;
	std::stack<P, std::vector<P> > stack;
	stack.emplace(Address{AddressType::Heap, lhsIndex}, Address{AddressType::Temp, rhsIndex});
	while(!stack.empty()) {
		// this is similar to get_structure, unify_variable and unify_value from the WAM book
		Address lhsAddr = deref(stack.top().first),
				rhsAddr = deref(stack.top().second);
		stack.pop();
		assert(lhsAddr.type == AddressType::Heap);
		if(rhsAddr.type == AddressType::Heap) {
			unifyHeapHeap(lhsAddr.addr, rhsAddr.addr, mgu);
		} else {
			Cell rhsCell = getCell(rhsAddr);
			if(rhsCell.tag == Cell::Tag::REF) {
				assert(rhsCell.REF.addr == rhsAddr);
				getCell(rhsAddr).REF.addr = lhsAddr;
				mgu.bindings.push_back(rhsAddr);
			} else if(rhsCell.tag == Cell::Tag::Structure) {
				Cell lhsCell = getCell(lhsAddr);
				if(lhsCell.tag == Cell::Tag::REF) {
					assert(lhsCell.REF.addr == lhsAddr);
					// copy the structure to the heap, and bind lhs to it
					Address rhsAddrNew = putStructure(rhsCell.Structure.name, rhsCell.Structure.arity);
					getCell(lhsAddr).REF.addr = rhsAddrNew;
					mgu.bindings.push_back(lhsAddr);
					// overwrite rhs
					getCell(rhsAddr).tag = Cell::Tag::STR;
					getCell(rhsAddr).STR.addr = rhsAddrNew;
					// copy arguments
					for(int i = 1; i <= rhsCell.Structure.arity; i++) {
						Address rhsSubAddr = rhsAddr + i;
						Cell rhsSubCell = getCell(rhsSubAddr);
						switch(rhsSubCell.tag) {
						case Cell::Tag::REF:
						case Cell::Tag::STR:
							// create a variable and schedule unification
							stack.emplace(putRefPtr(), rhsSubAddr);
							break;
						case Cell::Tag::Structure:
							assert(rhsSubCell.Structure.arity == 0);
							// overwrite rhs and append structure
							getCell(rhsSubAddr).tag = Cell::Tag::STR;
							getCell(rhsSubAddr).STR.addr = putStructure(rhsSubCell.Structure.name, rhsSubCell.Structure.arity);
							break;
						}
					}
				} else if(lhsCell.tag == Cell::Tag::Structure) {
					if(lhsCell.Structure.name == rhsCell.Structure.name &&
					   lhsCell.Structure.arity == rhsCell.Structure.arity) {
						for(std::size_t i = lhsCell.Structure.arity; i > 0; i--)
							stack.emplace(lhsAddr + i, rhsAddr + i);
						// overwrite rhs to point to heap
						getCell(rhsAddr).tag = Cell::Tag::STR;
						getCell(rhsAddr).STR.addr = lhsAddr;
					} else {
						mgu.status = MGU::Status::Fail;
						mgu.errorLeft = lhsAddr;
						mgu.errorRight = rhsAddr;
						return;
					}
				} else
					MOD_ABORT;
			} else
				MOD_ABORT;
		}
	}
	verify();
}

} // namespace mod::lib::Term

template<>
struct std::hash<mod::lib::Term::Address> {
	std::size_t operator()(mod::lib::Term::Address addr) const {
		return addr.addr; // probably good enough
	}
};

#endif // MOD_LIB_TERM_WAM_HPP