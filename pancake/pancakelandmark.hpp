#pragma once

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <vector>

extern "C" unsigned long hashbytes(unsigned char[], unsigned int);

class PancakeLandmark {
public:
	enum { Ncakes = NCAKES };
	unsigned int landmarkPancake;

	typedef int Cake;
	typedef int Oper;
	enum { Nop = -1 };
	typedef int Cost;

	struct State {
		bool eq(const PancakeLandmark* pl, const State &o) const {
			for (unsigned int i = 0; i < Ncakes; i++) {
				if (cakes[i] >= pl->landmarkPancake && cakes[i] != o.cakes[i])
					return false;
			}
			return true;
		}

		unsigned long hash(const PancakeLandmark*) const {
			return hashbytes((unsigned char *) cakes,
						Ncakes * sizeof(Cake));
		}

	private:
		friend class PancakeLandmark;
		friend class Undo;

		void flip(Oper op) {
			assert (op > 0);
			assert (op < Ncakes);

			for (int n = 0; n <= op / 2; n++) {
				Cake tmp = cakes[n];
				cakes[n] = cakes[op - n];
				cakes[op - n] = tmp;
			}
		}

		Cake cakes[Ncakes];
		Cost h;
	};

	typedef State PackedState;

	PancakeLandmark(Pancake::State &s, unsigned int landmarkPancake) :
		landmarkPancake(landmarkPancake) {

		for (unsigned int i = 0; i < Ncakes; i++)
			init[i] = s.cakes[i];
	}

	Pancake::State toBaseDomain(PancakeLandmark::State &pls) {
		Pancake::State s;

		for (unsigned int i = 0; i < Ncakes; i++)
			s.cakes[i] = pls.cakes[i];

		s.h = Pancake::ngaps(s.cakes);

		return s;
	}

	State initialstate() {
		State s;

		for (unsigned int i = 0; i < Ncakes; i++)
			s.cakes[i] = init[i];
		s.h = ngaps(s.cakes, landmarkPancake);

		return s;
	}

	Cost h(const State &s) const {
		return s.h;
	}

	Cost d(const State &s) const {
		return s.h;
	}

	bool isgoal(const State &s) const {
		return s.h == 0;
	}

	struct Operators {
		Operators(const PancakeLandmark&, const State&) { }

		unsigned int size() const {
			return Ncakes -1 ;
		}

		Oper operator[](unsigned int i) const {
			return i + 1;
		}
	};

	struct Edge {
		Cost cost;
		Oper revop;
		Cost revcost;
		State &state;

		Edge(PancakeLandmark &d, State &s, Oper op) :
				cost(1), revop(op), revcost(1), state(s), oldh(s.h) {
			bool wasgap = gap(state.cakes, op, d.landmarkPancake);
			state.flip(op);

			bool hasgap = gap(state.cakes, op, d.landmarkPancake);
			if (wasgap && !hasgap)
				state.h--;
			if (!wasgap && hasgap)
				state.h++;
		}

		~Edge() {
			state.h = oldh;
			state.flip(revop);
		}

	private:
		Cost oldh;
};

	void pack(PackedState &dst, const State &s) const {
		dst = s;
	}

	State &unpack(const State &dst, PackedState &pkd) const {
		return pkd;
	}

	void dumpstate(FILE *out, State &s) const {
		for (unsigned int i = 0; i < Ncakes; i++) {
			fprintf(out, "%u", s.cakes[i]);
			if (i < Ncakes - 1)
				fputc(' ', out);
		}
		fputc('\n', out);
	}

	Cost pathcost(const std::vector<State> &path, const std::vector<Oper> &ops) {
		State state = initialstate();
		Cost cost(0);
		for (int i = ops.size() - 1; i >= 0; i--) {
			State copy(state);
			Edge e(*this, copy, ops[i]);
			assert (e.state.eq(this, path[i]));
			state = e.state;
			cost += e.cost;
		}
		assert (isgoal(state));
		return cost;
	}

private:

	static Cost ngaps(Cake cakes[], unsigned int landmarkPancake) {
		Cost gaps = 0;

		for (unsigned int i = 0; i < Ncakes; i++) {
			if (gap(cakes, i, landmarkPancake))
				gaps++;
		}

		return gaps;
	}

	// Is there a gap between cakes n and n+1?
	static bool gap(Cake cakes[], unsigned int n, unsigned int landmarkPancake) {
		assert(n < Ncakes);

		if (n == Ncakes-1)
			return cakes[Ncakes-1] != Ncakes-1;

		if(cakes[n] < landmarkPancake &&
		   cakes[n+1] < landmarkPancake) return false;

		return abs(cakes[n] - cakes[n+1]) != 1;
	}

	Cake init[Ncakes];
};
