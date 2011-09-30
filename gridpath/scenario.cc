#include "scenario.hpp"
#include "../utils/utils.hpp"
#include "../search/main.hpp"
#include <iostream>

static const float Epsilon = 0.0001;

Scenario::Scenario(int ac, char *av[]) : argc(ac), argv(av), lastmap(NULL) { }

Scenario::~Scenario(void) {
	if (lastmap)
		delete lastmap;
}

void Scenario::run(std::istream &in) {
	checkver(in);

	ScenarioEntry ent(*this);
	while (in >> ent) {
		Search<GridPath> *srch = getsearch<GridPath>(argc, argv);
		Result<GridPath> r = ent.run(srch);
		ent.outputrow(stdout, r);
		res.add(r);
		delete srch;
	}

	res.output(stdout);
}

void Scenario::checkver(std::istream &in) {
	float ver;
	std::string verstr;
	std::cin >> verstr >> ver;
	if (verstr != "version")
		fatal("Expected a version header");
	if (ver != 1.0)
		fatal("Version %g is unsupported.  Latest supported version is 1.0", ver);
}

void Scenario::outputhdr(FILE *out) {
	dfrowhdr(out, "run", 14,
		"bucket", "width", "height", "start x", "start y",
		"finish x", "finish y", "optimal sol",
		"nodes expanded", "nodes generated",
		"sol cost", "sol length", "wall time", "cpu time");
}

GridMap *Scenario::getmap(std::string mapfile) {
	if (!lastmap || lastmap->filename() != mapfile) {
		if (lastmap)
			delete lastmap;
		lastmap = new GridMap(mapfile);
	}
	return lastmap;
}

ScenarioEntry::ScenarioEntry(Scenario &s) : scen(s) { }

Result<GridPath> ScenarioEntry::run(Search<GridPath> *srch) {
	GridPath d(scen.getmap(mapfile), x0, y0, x1, y1);
	GridPath::State s0 = d.initialstate();
	return srch->search(d, s0);
}

void ScenarioEntry::outputrow(FILE *out, Result<GridPath> &r) {
	dfrow(out, "run", "uuuuuuuguugugg",
		bucket, w, h, (unsigned long) x0, (unsigned long) y0,
		(unsigned long)  x1, (unsigned long) y1, opt, r.expd, r.gend,
		r.cost, (unsigned long) r.path.size(), r.wallend - r.wallstrt,
		r.cpuend - r.cpustrt);
} 

std::istream &operator>>(std::istream &in, ScenarioEntry &s) {
	in >> s.bucket;
	in >> s.mapfile;
	in >> s.w >> s.h;
	in >> s.x0 >> s.y0;
	in >> s.x1 >> s.y1;
	in >> s.opt;
	return in;
}