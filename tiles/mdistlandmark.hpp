// Copyright © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.
#pragma once
#include <cstring>
#include <vector>
#include "tiles.hpp"
#include "mdist.hpp"
#include "packed.hpp"

class TilesMdistLandmark : public Tiles {
public:

	typedef PackedTiles<Ntiles> PackedState;

	struct State {
		bool operator==(const State &o) const {
			if (b != o.b)
				return false;
			for (unsigned int i = 0; i < Ntiles; i++) {
				if (ts[i] != o.ts[i] && i != b)
					return false;
			}
			return true;
		}
	private:
		friend class TilesMdistLandmark;
		Tile ts[Ntiles];
		Pos b;
		Cost h;
	};

	TilesMdistLandmark(TilesMdist&, TilesMdist::State&, unsigned int);

	State initialstate();

	Cost h(State &s) const { return s.h; }

	Cost d(State &s) const { return s.h; }

	bool isgoal(State &s) const { return s.h == 0; }

	struct Operators {
		Operators(TilesMdistLandmark& d, State &s) :
			n(d.ops[s.b].n), mvs(d.ops[s.b].mvs) { }

		unsigned int size() const {
			return n;
		}

		Oper operator[](unsigned int n) const {
			return mvs[n];
		}

	private:
		unsigned int n;
		const Pos *mvs;
	};

	std::pair<Oper, Cost> ident(const State &) const {
		return std::make_pair(Ident, 1);
	}

	struct Edge {
		Cost cost;
		Oper revop;
		Cost revcost;
		State &state;

		Edge(TilesMdistLandmark &d, State &s, Oper op) :
				cost(1), revop(s.b), revcost(1), state(s), oldh(s.h) {

			if (op == Ident) {
				revop = Ident;
				return;
			}

			Tile t = state.ts[op];
			state.ts[state.b] = t;
			state.h = d.computeMdist(state);
			state.b = op;
		}

		~Edge() {
			if (revop == Ident)
				return;
			state.ts[state.b] = state.ts[revop];
			state.b = revop;
			state.h = oldh;
		}

	private:
		friend class TilesMdistLandmark;
		Cost oldh;
	};

	void pack(PackedState &dst, State &s) const {
		s.ts[s.b] = 0;
		dst.pack(s.ts);
	}

	State &unpack(State &buf, PackedState &pkd) const {
		buf.b = pkd.unpack_md(md, buf.ts, &buf.h);
		return buf;
	}

	void dumpstate(FILE *out, State &s) const {
		s.ts[s.b] = 0;
		Tiles::dumptiles(out, s.ts);
	}

	Cost pathcost(const std::vector<State>&, const std::vector<Oper>&);

	TilesMdist::State toBaseDomain(TilesMdistLandmark::State& tl) const {
		TilesMdist::State s;
		for(unsigned int i = 0; i < Ntiles; i++) {
			s.ts[i] = tl.ts[i];
		}
		s.b = tl.b;
		s.h = 0;

		for (unsigned int i = 0; i < Ntiles; i++) {
			if (s.ts[i] != 0) {
				s.h += md[s.ts[i]][i];
			}
		}

		return s;
	}

protected:
	unsigned int md[Ntiles][Ntiles];
	Cost computeMdist(const State&) const;

private:
	bool tilesToConsider[Ntiles];
	unsigned int howManyLs;
};
