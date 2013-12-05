// © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.

#include "../graphics/ui.hpp"
#include "../utils/utils.hpp"
#include "gridmap.hpp"
#include "traffic.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <vector>
#include <SDL/SDL_opengl.h>

class WatchUi : public Ui {
public:
	WatchUi(unsigned int, unsigned int, bool, GridMap*, std::vector<unsigned int>);
	virtual bool frame();
	virtual void key(int, bool) { /* ignore */ }
	virtual void motion(int, int, int, int) { /* ignore */ }
	virtual void click(int, int, int, bool) { /* ignore */ }

	geom2d::Pt loc;
	geom2d::Pt goal;

private:
	void move();
	void draw();

	GridMap *gridmap;
	unsigned int height;
	std::vector<unsigned int> controls;
	std::vector<unsigned int>::iterator iter;
	unsigned int currentframe;
	unsigned int currentobstacletime;
	bool reset;
	std::vector<geom2d::Poly> obstacles;
	std::vector<Color> obstacleColors;
};

static unsigned long frametime = 50;	// in milliseconds
static unsigned long delay = 0;	// in milliseconds
static bool echo;
static bool save;
static std::string level;
static std::vector<unsigned int> controls;
static GridMap *trafficMap;
static unsigned int windowWidth, windowHeight;

static unsigned int zoomSquare = 0;

static geom2d::Pt scale(1,1);
static geom2d::Pt start;
static geom2d::Pt goal;

static bool minimap;
static geom2d::Pt miniMapSize;
static double miniMapScale;
static double miniMapBorderWidth;
static double minimapXOffset;
static double minimapYOffset;

static void parseargs(int, const char*[]);
static void helpmsg(int);
static void dfline(std::vector<std::string>&, void*);

int main(int argc, const char *argv[]) {
	parseargs(argc, argv);
	dfread(stdin, dfline, NULL, echo ? stdout : NULL);
	if (level == "")
		fatal("No level key in the datafile");
	if (controls.size() == 0)
		fatal("No controls");

	FILE *f = fopen(level.c_str(), "r");
	if (!f)
		fatalx(errno, "Failed to open %s", level.c_str());

	Traffic traffic(f);
	fclose(f);

	trafficMap = traffic.map;

	windowWidth = 800;
	windowHeight = 600;

	if(zoomSquare == trafficMap->w && zoomSquare == trafficMap->h) {
		zoomSquare = 0;
		fprintf(stderr, "zoom size is too big, disabling zoom option\n");
	}
	else {
		if(zoomSquare >= trafficMap->w) {
			zoomSquare = trafficMap->w;
			fprintf(stderr, "zoom size is too big, clipping to map width\n");
		}
		else if(zoomSquare >= trafficMap->h) {
			zoomSquare = trafficMap->h;
			fprintf(stderr, "zoom size is too big, clipping to map height\n");
		}
	}

	if(zoomSquare != 0) {
		scale.x = (double)windowWidth / (double)zoomSquare;
		scale.y = (double)windowHeight / (double)zoomSquare;

		minimap = true;
		miniMapBorderWidth = 2.;
		if(trafficMap->w >= trafficMap->h) {
			miniMapSize.x = (double)windowWidth * 0.25;
			miniMapScale = miniMapSize.x / (double)trafficMap->w;
			miniMapSize.y = miniMapScale * (double)trafficMap->h;
		}
		else {
			miniMapSize.y = (double)windowHeight * 0.25;
			miniMapScale = miniMapSize.y / (double)trafficMap->h;
			miniMapSize.x = miniMapScale * (double)trafficMap->w;
		}
		minimapXOffset = (double)windowWidth - miniMapSize.x - miniMapBorderWidth - 5.;
		minimapYOffset = miniMapBorderWidth + 2.;

	} else {
		scale.x = (double)windowWidth / (double)traffic.map->w;
		scale.y = (double)windowHeight / (double)traffic.map->h;
	}

	WatchUi ui(windowWidth, windowHeight, save, traffic.map, controls);

	std::pair<int,int> s = traffic.map->coord(traffic.start);
	std::pair<int,int> g = traffic.map->coord(traffic.finish);

	start.x = s.first;
	start.y = s.second;

	ui.loc.x = s.first;
	ui.loc.y = s.second;
	ui.goal.x = g.first;
	ui.goal.y = g.second;
	ui.run(frametime);
}

static void parseargs(int argc, const char *argv[]) {
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-h") == 0) {
			helpmsg(0);
		} else if (strcmp(argv[i], "-f") == 0) {
			frametime = strtol(argv[++i], NULL, 10);
		} else if (strcmp(argv[i], "-d") == 0) {
			delay = strtol(argv[++i], NULL, 10);
			delay *= 1000;
		} else if (strcmp(argv[i], "-e") == 0) {
			echo = true;
		} else if (strcmp(argv[i], "-s") == 0) {
			save = true;
		} else if (strcmp(argv[i], "-z") == 0) {
			zoomSquare = strtol(argv[++i], NULL, 10);
		} else {
			printf("Unknown option %s", argv[i]);
			helpmsg(1);
		}
	}
}

static void helpmsg(int status) {
	puts("Usage: watch [options]");
	puts("Options:");
	puts("	-h	print this help message");
	puts("	-z	zoom size");
	puts("	-d	delay in seconds before playing");
	puts("	-f	frame rate in milliseconds (default 20)");
	puts("	-e	echo the input to standard output");
	puts("	-s	saves an mpeg");
	exit(status);
}

static void dfline(std::vector<std::string> &toks, void*) {
	if (toks[1] == "level")
		level = toks[2];
	else if (toks[1] == "controls")
		controls = controlvec(toks[2]);
}

WatchUi::WatchUi(unsigned int w, unsigned int h, bool save,
		GridMap *g, std::vector<unsigned int> cs) :
		Ui(w, h, save), gridmap(g),
		height(h), controls(cs), currentframe(0), currentobstacletime(0), reset(false) {

	// flip everything over
	glTranslated(0.0, height, 0.0);
	glScaled(1.0, -1.0, 1.0);

	iter = controls.begin();
}

bool WatchUi::frame() {
	if (iter == controls.end()) {
		if (delay == 0)
			delay = 1000;
		else
			delay -= frametime;
		if (delay <= 0)
			return false;
		return true;
	}
	if (delay > 0) {
		delay -= frametime;
		return true;
	}
	move();
	draw();
	return true;
}

void WatchUi::move() {
	obstacles.clear();

	if(currentframe > 0) {
		if(reset) {
			reset = false;
			currentobstacletime = 0;
			loc.x = start.x;
			loc.y = start.y;
		}
		else {
			loc.x += gridmap->mvs[*iter].dx;
			loc.y += gridmap->mvs[*iter].dy;
			*iter++;
		}
	}
	

	for(unsigned int i = 0; i < gridmap->obstacles.size(); i++) {
		GridMap::Obstacle &o = gridmap->obstacles[i];
		std::pair< int, int> o2 = o.positionAt(gridmap->w, gridmap->h, currentobstacletime);

//		assert(loc.x != o2.first || loc.y != o2.second);
		if(loc.x == o2.first && loc.y == o2.second)
			reset = true;

		geom2d::Poly obs(4,
				 (double)o2.first, (double)o2.second,
				 o2.first + 1., (double)o2.second,
				 o2.first + 1., o2.second + 1.,
				 (double)o2.first, o2.second + 1.
			);
		obstacles.push_back(obs);
	}

	if(obstacleColors.size() <= 0)
		for(unsigned int i = 0; i < obstacles.size(); i++)
			obstacleColors.push_back(Color(0, 0, rand() / (double)RAND_MAX));
//			obstacleColors.push_back(Image::black);

	currentframe++;
	currentobstacletime++;
}

void WatchUi::draw() {
	scene.clear();



	unsigned int xQuadrant = minimap ? (unsigned int)loc.x / zoomSquare : 0;
	unsigned int yQuadrant = minimap ? (unsigned int)loc.y / zoomSquare : 0;

	double xmin = (double)xQuadrant * (double)zoomSquare;
	double ymin = (double)yQuadrant * (double)zoomSquare;
	double xmax = (double)xQuadrant * (double)zoomSquare + (double)zoomSquare;
	double ymax = (double)yQuadrant * (double)zoomSquare + (double)zoomSquare;

	if(xmin < 0) xmin = 0;
	else if(xmax >= trafficMap->w) xmin = trafficMap->w- zoomSquare;
	if(ymin < 0) ymin = 0;
	else if(ymax >= trafficMap->h) ymin =  trafficMap->h - zoomSquare;

	geom2d::Poly g(4,
		       goal.x, goal.y,
		       goal.x + 1., goal.y,
		       goal.x + 1., goal.y + 1.,
		       goal.x, goal.y + 1.
		);

	geom2d::Poly miniG = g;
	miniG.scale(miniMapScale, miniMapScale);
	miniG.translate(minimapXOffset, minimapYOffset);

	if(minimap) {
		g.translate(-xmin, -ymin);
	}
	g.scale(scale.x, scale.y);
	scene.add(new Scene::Poly(g, Image::green, -1));

	geom2d::Poly l(4,
		loc.x, loc.y,
		loc.x + 1., loc.y,
		loc.x + 1., loc.y + 1.,
		loc.x, loc.y + 1.
	);

	geom2d::Poly miniL = l;
	miniL.scale(miniMapScale, miniMapScale);
	miniL.translate(minimapXOffset, minimapYOffset);

	if(minimap) {
		l.translate(-xmin, -ymin);
	}
	l.scale(scale.x, scale.y);
	scene.add(new Scene::Poly(l, Image::red, -1));

	std::vector<geom2d::Poly> miniObs;
	for(unsigned int i = 0; i < obstacles.size(); i++) {
		if(minimap) {
			geom2d::Poly miniOb = obstacles[i];
			miniOb.scale(miniMapScale, miniMapScale);
			miniOb.translate(minimapXOffset, minimapYOffset);
			miniObs.push_back(miniOb);

			obstacles[i].translate(-xmin, -ymin);
		}
		obstacles[i].scale(scale.x, scale.y);

		scene.add(new Scene::Poly(obstacles[i], obstacleColors[i], -1));
	}

	if(minimap) {
		geom2d::Poly minimapBackground(4,0.,0.,1.,0.,1.,1.,0.,1.);
		minimapBackground.scale(miniMapSize.x + miniMapBorderWidth, 
								miniMapSize.y + miniMapBorderWidth);
		minimapBackground.translate(minimapXOffset, minimapYOffset);
		scene.add(new Scene::Poly(minimapBackground, Image::white,-1));

		scene.add(new Scene::Poly(miniG, Image::green, -1));
		scene.add(new Scene::Poly(miniL, Image::red, -1));

		for(unsigned int i = 0; i < miniObs.size(); i++)
			scene.add(new Scene::Poly(miniObs[i], obstacleColors[i], -1));

		geom2d::Poly minimapBorder(4,0.,0.,1.,0.,1.,1.,0.,1.);
		minimapBorder.scale(miniMapSize.x+2, miniMapSize.y+2);
		minimapBorder.translate(minimapXOffset-1, minimapYOffset-1);
		scene.add(new Scene::Poly(minimapBorder, Image::black, miniMapBorderWidth));

		geom2d::Poly z(4,0.,0.,1.,0.,1.,1.,0.,1.);
		z.scale(miniMapScale, miniMapScale);
		z.scale((double)zoomSquare, (double)zoomSquare);
		z.translate(xmin * miniMapScale, ymin * miniMapScale);
		z.translate(minimapXOffset, minimapYOffset);
	
		scene.add(new Scene::Poly(z, Image::red));
	}
}
