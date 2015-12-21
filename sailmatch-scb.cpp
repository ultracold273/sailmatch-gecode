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
 *
 * Usage: <prg_name> -boats <num_boats> -skippers <num_skippers>
 */

#include <gecode/int.hh>
#include <gecode/search.hh>
#include <gecode/driver.hh>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cmath>

using namespace Gecode;

#define CAL_FLIGHT(x, y) ((int) ceil(((float) x) * (x - 1) / y))
#define CAL_MATCH(x) (x * (x - 1) / 2)


class SailMatchOptions : public Options {
private:
	int _boats;
	int _skippers;
public:
	SailMatchOptions(const char * s, int boats0, int skippers0) : Options(s), _boats(boats0), _skippers(skippers0){}
	void parse(int& argc, char * argv[]) {
		Options::parse(argc, argv);
		for(int i = 0;i < argc;i++) {
			if (!strncmp(argv[i], "-boats", 7) && i + 1 < argc) {
				_boats = atoi(argv[i+1]);
				if (_boats == 0) {
					std::cerr << "Invalid Argument for boats" << argv[i + 1] << std::endl;
				}
				i++;
			}
			if (!strncmp(argv[i], "-skippers", 10) && i + 1 < argc) {
				_skippers = atoi(argv[i+1]);
				if (_skippers == 0) {
					std::cerr << "Invalid Argument for skippers" << argv[i + 1] << std::endl;
				}
				i++;
			}
		}
		std::cerr << "Echo Boats: " << _boats << std::endl;
		std::cerr << "Echo Skippers: " << _skippers << std::endl;
	}
	inline int boats() const {return _boats;}
	inline int skippers() const {return _skippers;}
};


class SailMatch : public Script {
protected:
	const int boats;
	const int skippers;
	const int matches;
	const int flights;
	const int matchesPerFlight;
	IntVarArray timeSlots;
	IntVarArray state;
	IntVarArray change;
	IntVarArray totalChanges;
	IntVarArray match;
	IntVarArray modMatch;
	IntVarArray time;
	IntVar totalBoatChanges;
	IntVarArray positions;
	IntVarArray skipImbalance;
	IntVarArray skipMaxImbalance;
	IntVar maxImbalance;
	const SailMatchOptions& opt;
public:
	SailMatch(const SailMatchOptions& _opt) : Script(_opt), boats(_opt.boats()), skippers(_opt.skippers()), matches(CAL_MATCH(_opt.skippers())), flights(CAL_FLIGHT(_opt.skippers(), _opt.boats())), matchesPerFlight(_opt.boats()/2), opt(_opt) {
		/*
		std::cout << "Boats Num: " << boats << std::endl;
		std::cout << "Skippers Num: " << skippers << std::endl;
		std::cout << "Matches Num: " << matches << std::endl;
		std::cout << "Flights Num: " << flights << std::endl;
		std::cout << "Matches per Flights Num: " << matchesPerFlight << std::endl;
		*/
	
		// Constraints for each skippers
		// Variable Definition
		// V1
		timeSlots = IntVarArray(*this, skippers * matches, -2, skippers - 1);
		Matrix<IntVarArray> tSlotMat(timeSlots, skippers, matches);
		for(int i = 0;i < skippers;i++) {
			int intArr[skippers + 1];
			int * intArrPtr = intArr;
			for (int k = -2;k < skippers;k++) if (k != i) *(intArrPtr++) = k;
			IntSet d(intArr, skippers + 1);
			for (int j = 0; j < matches;j++) {
				dom(*this, tSlotMat(i, j), d);
			}
		}
		// V2
		state = IntVarArray(*this, skippers * flights, 0, 4);
		Matrix<IntVarArray> stateMat(state, skippers, flights);
		// V3
		change = IntVarArray(*this, skippers * (flights - 1), 0, 1);
		Matrix<IntVarArray> changeMat(change, skippers, flights - 1);
		// V4
		totalChanges = IntVarArray(*this, skippers, 0, flights - 1);
		// Constraint for Each Skippers
		// C1
		for(int i = 0;i < skippers;i++) {
			for (int j = 0;j < flights;j++) {
				BoolVar b1(*this, 0, 1);
				BoolVar b2(*this, 0, 1); 
				BoolVar b3(*this, 0, 1);
				BoolVar b4(*this, 0, 1);
				BoolVar b5(*this, 0, 1);
				// state == FIRST
				rel(*this, stateMat(i, j), IRT_EQ, 0, b1);
				rel(*this, tSlotMat(i, j * matchesPerFlight), IRT_GQ, 0, b1);
				for (int k = 1;k < matchesPerFlight;k++) {
					rel(*this, tSlotMat(i, j * matchesPerFlight + k), IRT_EQ, -1, b1);
				}
				// state == MID
				IntVar cntCulmOne(*this, 0, matchesPerFlight - 2);
				IntVar cntCulmTwo(*this, 0, matchesPerFlight - 2);
				rel(*this, stateMat(i, j), IRT_EQ, 1, b2);
				count(*this, tSlotMat.slice(i, i + 1, j * matchesPerFlight + 1, (j + 1) * matchesPerFlight - 1), -1, IRT_EQ, cntCulmOne);
				rel(*this, cntCulmOne, IRT_EQ, matchesPerFlight - 3, b2);
				count(*this, tSlotMat.slice(i, i + 1, j * matchesPerFlight + 1, (j + 1) * matchesPerFlight - 1), -2, IRT_EQ, cntCulmTwo);
				rel(*this, cntCulmTwo, IRT_EQ, 0, b2);
				rel(*this, tSlotMat(i, j * matchesPerFlight), IRT_EQ, -1, b2);
				rel(*this, tSlotMat(i, (j+1) * matchesPerFlight - 1), IRT_EQ, -1, b2);
				// state == LAST
				rel(*this, stateMat(i, j), IRT_EQ, 2, b3);
				rel(*this, tSlotMat(i, (j + 1) * matchesPerFlight - 1), IRT_GQ, 0, b3);
				for (int k = 0;k < matchesPerFlight - 1;k++) {
					rel(*this, tSlotMat(i, j * matchesPerFlight + k), IRT_EQ, -1, b1);
				}
				// state == BYE
				IntVar cntEQmOne(*this, 0, matchesPerFlight);
				rel(*this, stateMat(i, j), IRT_EQ, 3, b4);
				count(*this, tSlotMat.slice(i, i + 1, j * matchesPerFlight, (j + 1) * matchesPerFlight), -1, IRT_EQ, cntEQmOne);
				rel(*this, cntEQmOne, IRT_EQ, matchesPerFlight, b4);
				// state == END
				IntVar cntEQmTwo(*this, 0, matchesPerFlight);
				rel(*this, stateMat(i, j), IRT_EQ, 4, b5);
				count(*this, tSlotMat.slice(i, i + 1, j * matchesPerFlight, (j +1) * matchesPerFlight), -2, IRT_EQ, cntEQmTwo);
				rel(*this, cntEQmTwo, IRT_EQ, matchesPerFlight, b5);
			}
		}
		// C2
		for (int i = 0;i < skippers;i++) {
			for (int j = 0;j < skippers;j++) {
				if (i != j) {
					count(*this, tSlotMat.col(i), j, IRT_EQ, 1); 
				}
			}
		}
		// C3
		// DFA alphabet in state from 0 to 4
		// DFA state 6 DEAD state
		// DFA state 5 START state
		// DFA state 0 FIRST state
		// DFA state 1 MID state
		// DFA state 2 LAST state
		// DFA state 3 BYE state
		// DFA state 4 END state
		int starState = (skippers == boats)?3:6;
		
		DFA::Transition t[] = {{5, 0, 0}, {5, 1, 1}, {5, 2, 2}, {5, 3, 3}, {5, 4, 6},
							   {0, 0, 0}, {0, 1, 1}, {0, 2, 2}, {0, 3, 3}, {0, 4, 4},
							   {1, 0, 0}, {1, 1, 1}, {1, 2, 2}, {1, 3, 3}, {1, 4, 4},
							   {2, 0, 6}, {2, 1, 1}, {2, 2, 2}, {2, 3, starState}, {2, 4, 4},
							   {3, 0, 6}, {3, 1, 1}, {3, 2, 2}, {3, 3, 3}, {3, 4, 6},
							   {4, 0, 6}, {4, 1, 6}, {4, 2, 6}, {4, 3, 6}, {4, 4, 4},
							   {6, 0, 6}, {6, 1, 6}, {6, 2, 6}, {6, 3, 6}, {6, 4, 6},
							   {-1, 0, 0}};
		// Accepts FIRST, MID, LAST and END
		int finState[5] = {0, 1, 2, 4, -1};
		// Construct DFA
		DFA d(5, t, finState);
		for (int i = 0;i < skippers;i++) {
			extensional(*this, stateMat.col(i), d);
		}
		// C4
		for (int i = 0;i < skippers;i++) {
			for(int j = 0;j < flights - 1;j++) {
				rel(*this, changeMat(i, j) != 1 || (stateMat(i, j) == 3 && stateMat(i, j + 1) != 3));
			}
		}
		// C5
		for (int i = 0;i < skippers;i++) {
			rel(*this, totalChanges[i] == sum(changeMat.col(i)));
		}

		// V5
		match = IntVarArray(*this, skippers * skippers, -1, matches - 1);
		Matrix<IntVarArray> matchMat(match, skippers, skippers);
		for(int i = 0;i < skippers;i++) {
			for(int j = i;j < skippers;j++) {
				if (j == i) rel(*this, matchMat(i, j) == -1);
				else {
					rel(*this, matchMat(i, j) >= 0);
					rel(*this, matchMat(i, j) == matchMat(j, i));
				}
			}
		}
		// C6
		for (int i = 0;i < skippers;i++) {
			for (int j = i + 1;j < skippers;j++) {
				for (int k = 0;k < matches;k++) {
					rel(*this, matchMat(i, j) != k || (tSlotMat(i, k) == j && tSlotMat(j, k) == i));
				}
			}
		}
		// V6
		modMatch = IntVarArray(*this, skippers * skippers, -1, flights - 1);
		Matrix<IntVarArray> modMatchMat(modMatch, skippers, skippers);
		for(int i = 0;i < skippers;i++) {
			for(int j = i;j < skippers;j++) {
				if (j == i) rel(*this, matchMat(i, j) == -1);
				else {
					rel(*this, matchMat(i, j) >= 0);
					rel(*this, matchMat(i, j) == matchMat(j, i));
				}
			}
		}
		// C7
		for (int i = 0;i < skippers;i++) {
			for (int j = i + 1;j < skippers;j++) {
				rel(*this, modMatchMat(i, j) == matchMat(i, j) / matchesPerFlight);
			}
		}
		// C8
		for (int i = 0;i < skippers;i++) {
			distinct(*this, modMatchMat.col(i));
		}
		// V7
		time = IntVarArray(*this, matches * 2, 0, skippers - 1);
		Matrix<IntVarArray> timeMat(time, matches, 2);
		IntSet d0(0, skippers - 2);
		IntSet d1(1, skippers - 1);
		dom(*this, timeMat.row(0), d0);
		dom(*this, timeMat.row(1), d1);
		for(int i = 0;i < matches;i++) {
			rel(*this, timeMat(i, 0) < timeMat(i, 1));
		}
		for (int i = 0;i < matches;i++) {
			for (int j = i + 1;j < matches;j++) {
				rel(*this, timeMat(i, 0) != timeMat(j, 0) || timeMat(i, 1) != timeMat(j, 1));
			}
		}
		// Skippers in a flight differs
		for(int i = 0;i < flights;i++) {
			distinct(*this, timeMat.slice(i * matchesPerFlight,  (i + 1) * matchesPerFlight, 0, 2));
		}
		// C9
		for(int k = 0;k < matches;k++) {
			for(int i = 0;i < skippers;i++) {
				for (int j = i + 1;j < skippers;j++) {
					rel(*this, matchMat(i, j) != k || (timeMat(k, 0) == i && timeMat(k, 1) == j));
				}
			}
		}
		// V8
		totalBoatChanges = expr(*this, sum(totalChanges));
		// C11
		//rel(*this, totalBoatChanges <= 6);
		// Symmetric Breaking
		for(int i = 0;i < 2 * matchesPerFlight;i++) {
			rel(*this, timeMat(i/2, i%2) == i);
		}
		// V9
		positions = IntVarArray(*this, skippers * matches, 0, 1);
		Matrix<IntVarArray> posMat(positions, skippers, matches);
		// C12
		for(int i = 0;i < skippers;i++) {
			for(int j = 0;j < flights;j++) {
				for(int k = 0;k < matchesPerFlight;k++) {
					rel(*this, posMat(i, j * matchesPerFlight + k) != 1 || tSlotMat(i, j * matchesPerFlight + k) >= 0);
				}
			}
		}
		// V10
		skipImbalance = IntVarArray(*this, skippers * matchesPerFlight, 0, skippers - 1);
		Matrix<IntVarArray> skipImbalanceMat(skipImbalance, skippers, matchesPerFlight);
		// C13
		IntVarArgs posCollector(flights);
		for (int i = 0;i < skippers;i++) {
			for (int j = 0;j < matchesPerFlight;j++) {
				for (int k = 0;k < flights;k++) {
					posCollector[k] = posMat(i, k * matchesPerFlight + j);
				}
				rel(*this, skipImbalanceMat(i, j) == abs((skippers - 1) / matchesPerFlight - sum(posCollector)));
			}
		}
		// V11
		skipMaxImbalance = IntVarArray(*this, skippers, 0, skippers);
		// C14
		for(int i = 0;i < skippers;i++) {
			rel(*this, skipMaxImbalance[i] == max(skipImbalanceMat.col(i)));
		}
		// V12 imbalance is just skipMaxImbalance
		// V13 and C15
		maxImbalance = expr(*this, max(skipMaxImbalance));
		
		branch(*this, time, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
	}
	// Optimization Criteria
	virtual void constrain(const Space& _cur) {
		const SailMatch& cur = static_cast<const SailMatch&>(_cur);
		IntVar cpMaxImbalance = expr(*this, maxImbalance);
		int curMaxImbalance = 0;
		for (int i = 0;i < skippers;i++) {
			if (curMaxImbalance < cur.skipImbalance[i].val()) {
				curMaxImbalance = cur.skipMaxImbalance[i].val();
			}
		}
		rel(*this, cpMaxImbalance < curMaxImbalance);
		/* Stage 1 Optimization */
		// V8_copy
		IntVar cpTotalBoatChanges = expr(*this, totalBoatChanges);
		// C10
		int curTotalBoatChanges = 0;
		for (int i = 0;i < skippers;i++) {
			curTotalBoatChanges += cur.totalChanges[i].val();
		}
		rel(*this, cpTotalBoatChanges <= curTotalBoatChanges);
		//*/
	}
	// Search Support
	SailMatch(bool share, SailMatch& s) : Script(share, s), boats(s.boats), skippers(s.skippers), matches(s.matches), flights(s.flights), matchesPerFlight(s.matchesPerFlight), opt(s.opt) {
		timeSlots.update(*this, share, s.timeSlots);
		state.update(*this, share, s.state);
		change.update(*this, share, s.change);
		totalChanges.update(*this, share, s.totalChanges);
		match.update(*this, share, s.match);
		modMatch.update(*this, share, s.modMatch);
		time.update(*this, share, s.time);
		totalBoatChanges.update(*this, share, s.totalBoatChanges);
		positions.update(*this, share, s.positions);
		skipImbalance.update(*this, share, s.skipImbalance);
		skipMaxImbalance.update(*this, share, s.skipMaxImbalance);
		maxImbalance.update(*this, share, s.maxImbalance);
	}
	virtual Script * copy(bool share) {
		return new SailMatch(share, *this);
	}
	// print Solution
	void print(std::ostream& os) const {
		os << "Basic Parameter Printing: " << std::endl;
		os << "Boats Num: " << boats << std::endl;
		os << "Skippers Num: " << skippers << std::endl;
		os << "Flights Num: " << flights << std::endl;
		os << "Matches Per Filght: " << matchesPerFlight << std::endl;
		os << std::endl << "Constraint Programming Result: " << std::endl;
		os << "Info for Each Skippers: " << std::endl;
		for (int i = 0;i < skippers;i++) {
			os << "Skipper " << i << ": ";
			for (int j = 0; j < flights;j++) {
				int stVal = state[i * flights + j].val(); 
				char stChar;
				switch(stVal) {
					case 0: stChar = 'F';break;
					case 1: stChar = 'M';break;
					case 2: stChar = 'L';break;
					case 3: stChar = 'B';break;
					case 4: stChar = 'E';break;
					default: stChar = '?';break;
				}
				os << stChar << "|";
			}
			os << std::endl;
			for (int j = 0;j < flights;j++) {
				os << "Flight " << j << ": ";
				for (int k = 0;k < matchesPerFlight;k++) {
					os << timeSlots[i * matches + j * matchesPerFlight + k].val() << " ";
				}
				os << std::endl;
			}
		}
		os << "Print Schedule: " << std::endl;
		for (int i = 0;i < flights;i++) {
			os << i << ": ";
			for (int j = 0;j < matchesPerFlight;j++) {
				os << "(" << time[2 * (i * matchesPerFlight + j)];
				os << "," << time[2 * (i * matchesPerFlight + j)];
				os << ") ";
			}
			os << std::endl;
		}
	}
};

int main(int argc, char * argv[]) {
	SailMatchOptions opt("SailMatchScheduling", 8, 8);
	opt.solutions(0);
	opt.parse(argc, argv);

	Script::run<SailMatch, BAB, SailMatchOptions>(opt);
	return 0;
}
