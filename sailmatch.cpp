/**
 * Author: Tian Ye
 * SID: 1155073201
 *
 * CSCI 5240 Project
 *
 * Sailing Match Race Schedules: Round-Robin Pairing Lists
 * http://www.dcs.gla.ac.uk/~ciaran/papers/cp2015-boats-paper.pdf
 * by Craig Macdonald, Ciaran McCreesh, ALice Miller, Patrick Prosser
 * on CP2015
 */

#include <gecode/int.hh>
#include <gecode/search.hh>
#include <gecode/driver.hh>
#include <iostream>
#include <cstdlib>
#include <cstring>

using namespace Gecode;

class SailMatchOptions : public Options {
private:
	int numboats;
	int numskippers;
public:
	SailMatchOptions(const char * s, int boats0, int skippers0) : Options(s), numboats(boats0), numskippers(skippers0){}
	void parse(int& argc, char * argv[]) {
		Options::parse(argc, argv);
		for(int i = 0;i < argc;i++) {
			if (!strncmp(argv[i], "-boats", 7) && i + 1 < argc) {
				numboats = atoi(argv[i+1]);
				if (numboats == 0) {
					std::cerr << "Invalid Argument for boats" << argv[i + 1] << std::endl;
				}
				i++;
			}
			if (!strncmp(argv[i], "-skippers", 10) && i + 1 < argc) {
				numskippers = atoi(argv[i+1]);
				if (numskippers == 0) {
					std::cerr << "Invalid Argument for skippers" << argv[i + 1] << std::endl;
				}
				i++;
			}
		}
	}
	inline int boats() const {return numboats;}
	inline int skippers() const {return numskippers;}
};


class SailMatch : public Script {
protected:
	const SailMatchOptions& opt;
	IntVarArray timeSlots;
public:
	SailMatch(const SailMatchOptions& _opt) : Script(_opt), opt(_opt), timeSlots(*this, 1) {
		std::cout << "Boats Num: " << opt.boats() << std::endl;
		std::cout << "Skippers Num: " << opt.skippers() << std::endl;
	}
	// Search Support
	SailMatch(bool share, SailMatch& s) : Script(share, s), opt(s.opt) {
		timeSlots.update(*this, share, s.timeSlots);
	}
	virtual Script * copy(bool share) {
		return new SailMatch(share, *this);
	}
	// print Solution
	void print(std::ostream& os) const {
		os << "Boats Num: " << opt.boats() << std::endl;
		os << "Skippers Num: " << opt.skippers() << std::endl;
	}
};

int main(int argc, char * argv[]) {
	SailMatchOptions opt("SailMatchScheduling", 8, 8);
	opt.solutions(1);
	opt.parse(argc, argv);

	Script::run<SailMatch, BAB, SailMatchOptions>(opt);
	return 0;
}
