//
//  functions.hpp
//  Undiscovered Worlds
//
//  Created by Jonathan Hill on 24/07/2019.
//  
//  The code for this project is released under the GNU General Public Licence v3.0 - https://choosealicense.com/licenses/gpl-3.0/
//
//  Please note that this code requires the following libraries to work:
//  NanoGUI - https://github.com/mitsuba-renderer/nanogui
//  SFML - https://www.sfml-dev.org/
//  stb_image - https://github.com/nothings/stb
//
//  main.cpp contains various utility functions and the functions for drawing the map images (as well as the main loop, of course).
//  globalterrain.cpp contains the functions used to generate the global terrain.
//  globalclimate.cpp contains the functions used to calculate the global precipitation and temperature, as well as rivers, lakes, etc.
//  regionalmap.cpp contains the functions used to generate the regional terrain.
//
//  There are no asset files. The code uses a number of templates, but these are stored in the code itself, in assetdata.cpp.
//
//  The global terrain/climate information is stored in an object of the planet class. The regional terrain/climate information is stored in an object of the region class. Both of these classes are defined in the relevant hpp/cpp files.
//
//  One oddity to be aware of: variables that hold the width/height of objects typically hold the index number of the final item, rather than the actual size. E.g. if planet.width() is set to 100, that means the width has 101 elements, with the index of the last one being 100. Hence the frequent use of loops such as for (int i=0; i<=width; i++). I'm aware that this is eccentric but I find it more intuitive.


#ifndef functions_hpp
#define functions_hpp

#include <stdio.h>
#include <SFML/Graphics.hpp>
#include <nanogui/nanogui.h>

#include "stb_image.h"

#include "classes.hpp"
#include "planet.hpp"
#include "region.hpp"

#define ARRAYWIDTH 2049
#define ARRAYHEIGHT 1025

#define NOISEWIDTH 1025
#define NOISEHEIGHT 513

#define GLOBALMAPTYPES 7

#define REGIONALTILEWIDTH 32
#define REGIONALTILEHEIGHT 32

#define MOUNTAINTEMPLATESTOTAL 27

using namespace std;

// Define some enums.

enum screenmodeenum{quit,createworld,globalmap,regionalmap};
enum mapviewenum{elevation,winds,temperature,precipitation,climate,rivers,relief};

// Declare functions that are in main.cpp

void fast_srand(long seed);
int fast_rand(void);
void setregionalminimap(planet &world, region &region, stbi_uc globalreliefimage[], nanogui::Texture &regionalminimap, int globalimagewidth, int globalimageheight, int globalimagechannels);
screenmodeenum regionalmapscreen(planet &world, region &region, sf::RenderWindow &window, sf::Font &font, boolshapetemplate &globalreliefimage, vector<string> climatename, boolshapetemplate &regionalelevationimage, boolshapetemplate &regionaltemperatureimage, boolshapetemplate &regionalprecipitationimage, boolshapetemplate &regionalclimateimage, boolshapetemplate &regionalriversimage, boolshapetemplate &regionalreliefimage, boolshapetemplate smalllake[], boolshapetemplate island[], peaktemplate peaks, vector<vector<float>> &riftblob, int riftblobsize);
void addbranchblocks(planet &world, region &region, boolshapetemplate &mapimage);
void drawzoommap(region &region, int poix, int poiy, int squaresize, boolshapetemplate &zoommapimage, sf::Texture &zoommaptexture, sf:: Sprite &zoommapsprite);
void saveworld(planet &world);
bool loadworld(planet &world);
void savesettings(planet &world);
bool loadsettings(planet &world);
//void savesettings(settings &settings);
//bool loadsettings(settings &settings);
void saveimage(stbi_uc source[], int globalimagechannels, int width, int height, string filename);
void createriftblob(vector<vector<float>> &riftblob, int size);
int random(int a, int b);
int randomsign(int a);
int altrandom(int a, int b);
int altrandomsign(int a);
string decimaltobinarystring(int a);
int wrap(int value, int max);
int wrappedaverage(int x, int y, int max);
int normalise(int x, int y, int max);
int tilt(int a, int b, int percentage);
void shift(vector<vector<int>> &map, int width, int height, int offset);
void flip(vector<vector<int>> &arr, int awidth, int aheight, bool vert, bool horiz);
void smooth(vector<vector<int>> &arr, int width, int height, int maxelev, int amount, bool vary);
bool edge(vector<vector<bool>> &arr, int width, int height, int i, int j);
void drawline(vector<vector<bool>> &arr, int x1, int y1, int x2, int y2);
void drawcircle(vector<vector<int>> &arr, int x, int y, int col, int radius);
void box(vector<vector<int>> &arr, int x1, int y1, int x2, int y2, int col);
void drawcircle3d(vector<vector<vector<int>>> &arr, int x, int y, int col, int radius, int index);
void box3d(vector<vector<vector<int>>> &arr, int x1, int y1, int x2, int y2, int col, int index);
sf::Vector2f curvepos(sf::Vector2f p0, sf::Vector2f p1, sf::Vector2f p2, sf::Vector2f p3, float t);
void fill (vector<vector<bool>> &arr, int width, int height, int x, int y, int replacement);
void fillcontinent (vector<vector<bool>> &arr, vector<vector<short>> &mask, short maskcheck, int width, int height, int startx, int starty, int replacement);
int tempelevadd(planet &world, int temp, int i, int j);
int tempelevadd(region &region, int temp, int i, int j);
int tempelevremove(planet &world, int temp, int i, int j);
int tempelevremove(region &region, int temp, int i, int j);
string getdirstring(int dir);
int getdir(int x, int y, int xx, int yy);
sf::Vector2i getdestination(int x, int y, int dir);
int findlowestdir(planet &world, int neighbours[8][2], int x, int y);
int findlowestdirriver(planet &world, int neighbours[8][2], int x, int y, vector<vector<int>> &mountaindrainage);
int getslope(planet &world, int x, int y, int xx, int yy);
int getslope(region &region, int x, int y, int xx, int yy);
int getflatness(planet &world, int x, int y);
int getflatelevation(planet &world, int x, int y);
int landdistance(planet &world, int x, int y);
sf::Vector2i nearestsea(planet &world, int i, int j, bool land, int limit, int grain);
sf::Vector2i nearestsea(region &region, int leftx, int lefty, int rightx, int righty, int i, int j);
bool vaguelycoastal(planet &world, int x, int y);
bool vaguelycoastal(region &region, int x, int y);
bool northwestlandonly(planet &world, int x, int y);
bool northwestlandonly(region &region, int x, int y);
bool lakenorthwestlandonly(region &region, int x, int y);
bool northeastlandonly(planet &world, int x, int y);
bool northeastlandonly(region &region, int x, int y);
bool lakenortheastlandonly(region &region, int x, int y);
bool southwestlandonly(planet &world, int x, int y);
bool southwestlandonly(region &region, int x, int y);
bool lakesouthwestlandonly(region &region, int x, int y);
bool southeastlandonly(planet &world, int x, int y);
bool southeastlandonly(region &region, int x, int y);
bool lakesoutheastlandonly(region &region, int x, int y);
sf::Vector2i findseatile(planet &world, int x, int y, int dir);
sf::Vector2i getflowdestination(planet &world, int x, int y, int dir);
sf::Vector2i getregionalflowdestination(region &region, int x, int y, int dir);
sf::Vector2i getupstreamcell(planet &world, int x, int y);
int checkwaterinflow(planet &world, int x, int y);
int checkregionalwaterinflow(region &region, int x, int y);
sf::Vector2i gettotalinflow(planet &world, int x, int y);
sf::Vector2i gettotalinflow(region &region, int x, int y);
sf::Vector2i findlowesthigher(region &region, int dx, int dy, int x, int y, int janload, int julload, int crount, vector<vector<bool>> &mainriver);
int nearlake(planet &world, int x, int y, int dist, bool rift);
int getlakeedge(planet &world, int x, int y);
int nexttolake(region &region, int x, int y);
int getnearestlakelevel(region &region, int x, int y);
int getnearestlakespecial(region &region, int x, int y);
sf::Vector2i findclosestriver(region &region, int x, int y, vector<vector<vector<int>>> &circletemplates, bool delta);
sf::Vector2i findclosestriverquickly(region &region, int x, int y);
void createcircletemplates(vector<vector<vector<int>>> &circletemplates);
int countinflows(region &region, int x, int y);
void initialiseworld(planet &world);
void initialisemapcolours(planet &world);
void initialiseregion(planet &world, region &region);
sf::Color getclimatecolours(int climate);
void drawglobalmapimage(mapviewenum mapview, planet &world, bool globalmapimagecreated[], stbi_uc globalelevationimage[], stbi_uc globalwindsimage[], stbi_uc globaltemperatureimage[], stbi_uc globalprecipitationimage[], stbi_uc globalclimateimage[], stbi_uc globalriversimage[], stbi_uc globalreliefimage[], int globalimagewidth, int globalimageheight, int globalimagechannels);
void drawregionalmapimage(mapviewenum mapview, planet &world, region &region, bool regionalmapimagecreated[], stbi_uc regionalelevationimage[], stbi_uc regionaltemperatureimage[], stbi_uc regionalprecipitationimage[],  stbi_uc regionalclimateimage[], stbi_uc regionalriversimage[], stbi_uc regionalreliefimage[], int regionalimagewidth, int regionalimageheight, int regionalimagechannels);
void blankregionalreliefimage(region &region, stbi_uc regionalreliefimage[], int regionalimagewidth, int regionalimageheight, int regionalimagechannels);
void addgridlines(boolshapetemplate &mapimage);
void shiftregionalmapimage(region &region, boolshapetemplate &image, int shifting);
void displaytemplates(planet &world, stbi_uc globalreliefimage[], int globalimagewidth, int globalimagechannels, boolshapetemplate shape[]);

// Declare functions that are in assetdata.cpp

void loadpeaktemplates(peaktemplate &peaks);
int createmountainrangetemplate(vector<vector<unsigned char>> &dirtemplate, vector<vector<unsigned char>> &heighttemplate, int tempno, int dir);
int createsmalllaketemplates(boolshapetemplate smalllake[]);
int createislandtemplates(boolshapetemplate island[]);
int createsmudgetemplates(byteshapetemplate smudge[]);
int createsmallsmudgetemplates(byteshapetemplate smallsmudge[]);
int createlargelaketemplates(boolshapetemplate largelake[]);
int createlandshapetemplates(boolshapetemplate landshape[]);
int createchainlandtemplates(boolshapetemplate chainland[]);

// Declare functions that are in globalterrain.cpp

void generateglobalterrain(planet &world, short terraintype, nanogui::Screen &screen, nanogui::Window &worldgenerationwindow, nanogui::Label &worldgenerationlabel, nanogui::ProgressBar &worldprogress, float progressstep, boolshapetemplate landshape[], boolshapetemplate chainland[], vector<vector<int>> &mountaindrainage, vector<vector<bool>> &shelves);
void generateglobalterraintype1(planet &world, nanogui::Screen &screen, nanogui::Window &worldgenerationwindow, nanogui::Label &worldgenerationlabel, nanogui::ProgressBar &worldprogress, float progressstep, boolshapetemplate landshape[], vector<vector<int>> &mountaindrainage, vector<vector<bool>> &shelves, boolshapetemplate chainland[]);
void generateglobalterraintype2(planet &world, nanogui::Screen &screen, nanogui::Window &worldgenerationwindow, nanogui::Label &worldgenerationlabel, nanogui::ProgressBar &worldprogress, float progressstep, boolshapetemplate landshape[], vector<vector<int>> &mountaindrainage, vector<vector<bool>> &shelves, boolshapetemplate chainland[]);
void createfractal(vector<vector<int>> &arr, int awidth, int aheight, int grain, float valuemod, float valuemod2, int min, int max, bool extreme, bool wrapped);
void newfractalinit(vector<vector<int>> &arr, int awidth, int aheight, int grain, int min, int max, bool extreme);
void newfractal(vector<vector<int>> &arr, int awidth, int aheight, int grain, float valuemod, float valuemod2, int min, int max, bool wrapped);
int square(vector<vector<int>> &arr, int awidth, int aheight, int s, int x, int y, int value, int min, int max, bool wrapped);
int diamond(vector<vector<int>> &arr, int awidth, int aheight, int s, int x, int y, int value, int min, int max, bool wrapped);
void createfractalformodifiedmerging(vector<vector<int>> &arr, int awidth, int aheight, int grain, float valuemod, float valuemod2, int min, int max, bool extreme);
void newfractalinitformodifiedmerging(vector<vector<int>> &arr, int awidth, int aheight, int grain, int min, int max, bool extreme);
void smallcontinents(planet &world, int baseheight, int conheight, vector<vector<int>> &fractal, vector<vector<int>> &plateaumap, boolshapetemplate landshape[], boolshapetemplate chainland[]);
void largecontinents(planet &world, nanogui::Screen &screen, nanogui::Window &worldgenerationwindow, nanogui::Label &worldgenerationlabel, nanogui::ProgressBar &worldprogress, float progressstep, int baseheight, int conheight, vector<vector<int>> &fractal, vector<vector<int>> &plateaumap, vector<vector<bool>> &shelves, boolshapetemplate landshape[], boolshapetemplate chainland[]);
void makecontinentedgemountains(planet &world, short thiscontinent, vector<vector<short>> &continentnos, vector<vector<short>> &overlaps, int baseheight, int conheight, vector<vector<int>> &fractal, boolshapetemplate landshape[], boolshapetemplate chainland[], int contstartpointx, int contstartpointy, int contwidth, int contheight, int startnearx, int startneary, short contdir, int &startpointx, int &startpointy);
void makevoronoi(vector<vector<short>> &voronoi, int width, int height, int points);
void makeshelvesvoronoi(planet &world, vector<vector<short>> &voronoi, vector<vector<bool>> &outline, int pointdist);
void makecontinent(planet &world, vector<vector<bool>> &continent, vector<vector<short>> &voronoi, int points, int width, int height, int &leftx, int &rightx, int &lefty, int &righty);
void makecontinentcircles(vector<vector<bool>> &circlemap, int width, int height, int circleno, int circlesize, int &circleleftx, int &circlerightx, int &circlelefty, int &circlerighty);
void createchains(planet &world, int baseheight, int conheight, vector<vector<int>> &fractal, vector<vector<int>> &plateaumap, boolshapetemplate landshape[], boolshapetemplate chainland[], sf::Vector2i focuspoints[], int focustotal, int focaldistance, int mode);
void createdirectedchain(planet &world, int baseheight, int conheight, short thiscontinent, vector<vector<short>> &continentnos, vector<vector<int>> &fractal, boolshapetemplate landshape[], boolshapetemplate chainland[], int chainstartx, int chainstarty, int chainendx, int chainendy, int mode, vector<sf::Vector2i> &rangepoints, vector<vector<bool>> &currentland, int volcanochance);
short getmountaindir(planet &world, int startx, int starty, int endx, int endy);
void drawshape(planet &world, int shapenumber, int centrex, int centrey, bool land, int baseheight, int conheight, boolshapetemplate shape[]);
void drawmarkedshape(planet &world, int shapenumber, int centrex, int centrey, bool land, int baseheight, int conheight, vector<vector<bool>> &markedmap, boolshapetemplate shape[]);
void drawplateaushape(planet &world, int shapenumber, int centrex, int centrey, int baseheight, int platheight, vector<vector<int>> &plateaumap, boolshapetemplate chainland[]);
void cuts(planet &world, int cutno, int baseheight, int conheight, boolshapetemplate shape[]);
void fractalmerge(planet &world, vector<vector<int>> &fractal);
void fractalmergemodified(planet &world, vector<vector<int>> &fractal, vector<vector<int>> &plateaumap, vector<vector<bool>> &removedland);
void fractalmergeland(planet &world, vector<vector<int>> &fractal,int conheight);
void fractaladdland(planet &world, vector<vector<int>> &fractal);
void removeseas(planet &world, int level);
void removestraights(planet &world);
void disruptseacoastline(planet &world, int centrex, int centrey, int avedepth, bool raise, int size);
int areacheck(planet &world, vector<vector<bool>> &checked, int startx, int starty);
int markseasize (planet &world, vector<vector<bool>> &checked, vector<vector<int>> &area, int size, int startx, int starty);
void widenchannels(planet &world);
void loweroceans(planet &world);
void removefloatingmountains(planet &world);
void cleanmountainridges(planet &world);
int getcode(int dir);
int getridge(planet &world, int x, int y, int dir);
int getridge(vector<vector<int>> &arr, int x, int y, int dir);
int getoceanridge(planet &world, int x, int y, int dir);
void deleteridge(planet &world, int x, int y, int dir);
void deleteridge(planet &world, vector<vector<int>> &ridgesarr, vector<vector<int>> &heightsarr, int x, int y, int dir);
void deleteoceanridge(planet &world, int x, int y, int dir);
int getdestinationland(planet &world, int x, int y, int dir);
void raisemountainbases(planet &world, vector<vector<int>> &mountaindrainage);
void erodeplateaux(planet &world, vector<vector<int>> &plateaumap);
void addplateaux(planet &world, vector<vector<int>> &plateaumap, int conheight);
void smoothland(planet &world, int amount);
void createextraelev(planet &world);
void adjustforsea(planet &world);
void removeseam(planet &world, int i);
int landcheck(planet &world, int n);
void depressionfill(planet &world);
void seadepressionfill(planet &world, vector<vector<bool>> &shelves);
bool checkdepressiontile(planet &world, vector<vector<int>> &filledmap, int i, int j, int e, int neighbours[8][2], bool somethingdone, vector<vector<int>> &noise);
sf::Vector2i lowestfill(planet &world, vector<vector<int>> &filledmap, int x, int y, int neighbours[8][2]);
void dryupwardcell(planet &world, vector<vector<int>> &filledmap, int i, int j, int e, int neighbours[8][2], vector<vector<int>> &noise);
void addlandnoise(planet &world);
void lowercoasts(planet &world);
void clamp(planet &world);
void shallowcoasts(planet &world);
void deepcoasts(planet &world);
void normalisecoasts(planet &world, int landheight, int seadepth, int severity);
void checkislands(planet &world);
void extendnoshade(planet &world);
void addfjordmountains(planet &world);
void makecontinentalshelves(planet &world, vector<vector<bool>> &shelves, int pointdist);
void removeshelfgaps(planet &world, vector<vector<bool>> &shelves);
int nonshelfareacheck (planet &world, vector<vector<bool>> &shelves, vector<vector<bool>> &checked, int startx, int starty);
void createoceanridges(planet &world, vector<vector<bool>> &shelves);
bool shelfedge(planet &world, vector<vector<bool>> &shelves, int x, int y);
void drawoceanridgeline(planet &world, int fromx, int fromy, int tox, int toy, vector<vector<int>> &array, int value);
void createoceanfault(planet &world, int midx, int midy, int mindist, int maxdist, vector<vector<int>> &ridgesmap, vector<vector<bool>> &checked, int masksize);
void createoceantrenches(planet &world, vector<vector<bool>> &shelves);
void createmountainsfromraw(planet &world, vector<vector<int>> &rawmountains);
void makearchipelagos(planet &world, vector<vector<bool>> &removedland, boolshapetemplate landshape[]);
void makemountainisland(planet &world, int x, int y, int peakheight);
void createisolatedvolcano(planet &world, int x, int y, vector<vector<bool>> &shelves, vector<vector<int>> &volcanodirection, int peakheight, bool strato);
void getseaslopes(planet &world, vector<vector<int>> &slopes);
void removeunderseabumps(planet &world);
void checkpoles(planet &world);

// Declare functions that are in globalclimate.cpp

void generateglobalclimate(planet &world, nanogui::Screen &screen, nanogui::Window &worldgenerationwindow, nanogui::Label &worldgenerationlabel, nanogui::ProgressBar &worldprogress, float progressstep, boolshapetemplate smalllake[], boolshapetemplate largelake[], boolshapetemplate landshape[], vector<vector<int>> &mountaindrainage, vector<vector<bool>> &shelves);
void createwindmap(planet &world);
void createtemperaturemap(planet &world, vector<vector<int>> &fractal);
void createseaicemap(planet &world, vector<vector<int>> &fractal);
int carefuladd(planet &world, int x, int y, int amount);
void createrainmap(planet &world, nanogui::Screen &screen, nanogui::Window &worldgenerationwindow, nanogui::Label &worldgenerationlabel, nanogui::ProgressBar &worldprogress, float progressstep, vector<vector<int>> &fractal, boolshapetemplate smalllake[], boolshapetemplate shape[]);
void drawmonsoonblob(planet &world, vector<vector<int>> &monsoonmap, int centrex, int centrey, int strength, int monsoonsplashradius, short dir, boolshapetemplate smalllake[]);
void removesubpolarstreaks(planet &world);
void createsaltlakes(planet & world, vector<vector<vector<int>>> &saltlakemap, vector<vector<int>> &nolake, boolshapetemplate smalllake[]);
void placesaltlake(planet & world, int centrex, int centrey, vector<vector<vector<int>>> &saltlakemap, vector<vector<int>> &basins, vector<vector<int>> &avoid, vector<vector<int>> &nolake, boolshapetemplate smalllake[]);
void createlakedepression(planet &world, int centrex, int centrey, int origlevel, int steepness, vector<vector<int>> &basins, int limit, bool up, vector<vector<int>> &avoid);
void createrivermap(planet &world, vector<vector<int>> &mountaindrainage);
void removediagonalrivers(planet &world);
void adddiagonalriverjunctions(planet &world);
void avoidvolcanoes(planet &world);
void removeparallelrivers(planet &world);
void removecrossingrivers(planet &world);
void removerivermountains(planet &world);
void tracedrop(planet &world, int x, int y, int minimum, int dropno, vector<vector<int>> &thisdrop, int maxrepeat, int neighbours[8][2]);
void convertsaltlakes(planet &world, vector<vector<vector<int>>> &saltlakemap);
void createlakemap(planet &world, vector<vector<int>> &nolake, boolshapetemplate smalllake[], boolshapetemplate largelake[]);
void drawlake(planet &world, int shapenumber, int centrex, int centrey, vector<vector<int>> &thislake, int lakeno, vector<vector<int>> &checked, vector<vector<int>> &nolake, int templatesize, int minrain, int maxtemp, boolshapetemplate smalllake[], boolshapetemplate largelake[]);
void makelakestartpoint(planet &world, vector<vector<int>> &thislake, int lakeno, int leftx, int lefty, int rightx, int righty);
int toolowinflow(planet &world, int x, int y, int elev, int surfacelevel);
int checkoutflow(planet &world, vector<vector<int>> &thislake, int lakeno, int x, int y);
int lakeoutline(planet &world, vector<vector<int>> &thislake, int lakeno, int x, int y);
int nexttolake(planet &world, int x, int y);
int nexttolakenodiag(planet &world, int x, int y);
void sortlakerivers(planet &world, int leftx, int lefty, int rightx, int righty, int centrex, int centrey, int outflowx, int outflowy, vector<vector<int>> &thislake, int lakeno);
void markriver(planet &world, int x, int y, vector<vector<int>> &markedarray, int extra);
int divertriver(planet & world, int x, int y, int destx, int desty, vector<vector<int>> &removedrivers, int riverno);
int divertlakeriver(planet &world, int x, int y, int destx, int desty, vector<vector<int>> &removedrivers, int riverno, int outflowx, int outflowy, vector<vector<int>> &avoidarray);
void removeriver(planet &world, vector<vector<int>> &removedrivers, int riverno, int x, int y);
void reduceriver(planet &world, int janreduce, int julreduce, vector<vector<int>> &removedrivers, int riverno, int x, int y);
void addtoriver(planet &world, int x, int y, int janload, int julload);
void cleanlakes(planet &world);
void lakerain(planet &world, vector<vector<int>> &lakewinterrainmap, vector<vector<int>> &lakesummerrainmap);
void riftlakerain(planet &world, vector<vector<int>> &lakewinterrainmap, vector<vector<int>> &lakesummerrainmap);
void lakerivers(planet &world, vector<vector<int>> &lakewinterrainmap, vector<vector<int>> &lakesummerrainmap);
void tracelakedrop(planet &world, int x, int y, int minimum, int dropno, vector<vector<int>> &thisdrop, int maxrepeat, int neighbours[8][2], vector<vector<int>> &lakesummerrainmap, vector<vector<int>> &lakewinterrainmap);
int findnearbyriver(planet &world, int x, int y, int dropno, vector<vector<int>> &thisdrop, int neighbours[8][2]);
bool swervecheck(planet &world, int x, int y, int origx, int origy, int lowered, int neighbours[8][2]);
void checkglobalflows(planet &world);
int checkthisflow(planet &world, int x, int y, int found);
void createriftlakemap(planet &world, vector<vector<int>> &nolake);
sf::Vector2i createriftlake(planet &world, int startx, int starty, int lakelength, vector<vector<int>> &nolake);
void createclimatemap(planet &world);
string getclimate(planet &world, int x, int y);
string getclimate(region &region, int x, int y);
string calculateclimate(int elev, int sealevel, float wrain, float srain, float mintemp, float maxtemp, bool onlyfirst);
string getclimatename(string climate);
void createergs(planet &world, boolshapetemplate smalllake[], boolshapetemplate largelake[]);
void createsaltpans(planet &world, boolshapetemplate smalllake[], boolshapetemplate largelake[]);
void drawspeciallake(planet &world, int shapenumber, int centrex, int centrey, int lakeno, vector<vector<int>> &thislake, boolshapetemplate laketemplate[], int special);
void createtidalmap(planet &world);
int gettidalrange(planet &world, int startx, int starty);
void createriverdeltas(planet &world);
void placedelta(planet &world, int centrex, int centrey, int upriver, vector<vector<int>> &deltarivers);
void divertdeltarivers(planet &world, vector<vector<int>> &deltarivers);
void checkrivers(planet &world);
void createwetlands(planet &world, boolshapetemplate smalllake[]);
void pastewetland(planet &world, int centrex, int centrey, int shapenumber, boolshapetemplate smalllake[]);
void removeexcesswetlands(planet &world);
void refineroughnessmap(planet &world);
void createunderseachannels(planet &world, vector<vector<bool>> &shelves);
void removediagonalchannels(planet &world);
void adddiagonalchanneljunctions(planet &world);
void removeparallelchannels(planet &world);
void traceseadrop(planet &world, int x, int y, int dropno, vector<vector<int>> &thisdrop, int maxrepeat, int oceanfloor, int neighbours[8][2], vector<vector<int>> &sediment);
void checkpoleclimates(planet &world);
void removesealakes(planet &world);
void connectlakes(planet &world);

// Declare functions that are in regionalmap.cpp

void generateregionalmap(planet &world, region &region, nanogui::Screen &screen, nanogui::ProgressBar &regionprogress, float progressstep, vector<vector<vector<int>>> &circletemplates, boolshapetemplate smalllake[], boolshapetemplate island[], peaktemplate &peaks, vector<vector<float>> &riftblob, int riftblobsize, int partial, byteshapetemplate smudge[], byteshapetemplate smallsmudge[]);
sf::Vector2i convertregionaltoglobal(planet &world, region &region, int x, int y);
void addlakeborders(planet &world, region &region, int dx, int dy, int sx, int sy);
void removesealakes(planet &world, region &region, int leftx, int lefty, int rightx, int righty);
void makerivertile(planet &world, region &region, int dx, int dy, int sx, int sy, vector<vector<int>> &rriverscarved, boolshapetemplate smalllake[], vector<vector<bool>> &rivercurves);
void makedeltatile(planet &world, region &region, int dx, int dy, int sx, int sy, vector<vector<int>> &rriverscarved);
sf::Vector3i calculateregionalriver(planet &world, region &region, int dx, int dy, int sx, int sy, sf::Vector2f pt, sf::Vector2f mm1, sf::Vector2f mm2, sf::Vector2f mm3, int janinflow, int julinflow, int riverlength, int isittributary, int goingtolake, bool delta);
void removeweirdelevations(planet &world, region &region, int dx, int dy, int sx, int sy);
void removelowflow(region &region, int dx, int dy);
void addtoexistingregionalriver(planet &world, region &region, int dx, int dy, int sx, int sy, int x, int y, int janinflow, int julinflow, bool delta);
void carveriver(planet &world, region &region, int dx, int dy, int sx, int sy, int riverlength, int riverstartx, int riverstarty, int riverendx, int riverendy, int startlandlevel, int endlandlevel, bool goingtolake, vector<vector<int>> &rriverscarved);
void carverivertributary(planet &world, region &region, int dx, int dy, int sx, int sy, int thisriverlength, int thisriverstartx, int thisriverstarty, int thisriverendx, int thisriverendy, int thisstartlandlevel, int thisendlandlevel, int riverendx, int riverendy, int endlandlevel, vector<vector<int>> &rriverscarved);
sf::Vector2i gettributaryendpoint(region &region, int startx, int starty);
void addsprings(planet &world, region &region, int sx, int sy, int dx, int dy, int junctionpointx, int junctionpointy, int riverendx, int riverendy, int maxmidvar, vector<vector<int>> &rriverscarved);
sf::Vector2i nearestlowerriver(region &region, int dx, int dy, int i, int j, int landlevel);
bool riverdircheck(region &region, int x, int y, int x2, int y2, int delta);
int removeriverorphans(planet &world, region &region, int sx, int sy, int dx, int dy, int riverlength);
void removeriverwidows(planet &world, region &region, int sx, int sy, int dx, int dy, int junctionpointx, int junctionpointy);
void removeregionalriverstraights(region &region, int dx, int dy, bool delta, vector<vector<int>> &rriverscarved);
int findriverlength(region &region, int startx, int starty, int endx, int endy, bool delta);
void joinuprivers(region &region, int dx, int dy, int sx, int sy);
void removenegativeflow(region &region, int dx, int dy);
void createsmalllake(planet &world, region &region, int dx, int dy, int sx, int sy, int centrex, int centrey, vector<vector<int>> &mainriver, boolshapetemplate smalllake[]);
void createsmallsaltlake (planet &world, region &region, int dx, int dy, int sx, int sy, int centrex, int centrey, int surfacelevel, vector<vector<bool>> &safesaltlakes, boolshapetemplate smalllake[]);
void createsmallglaciallake(planet &world, region &region, int dx, int dy, int sx, int sy, int centrex, int centrey, vector<vector<int>> &mainriver);
void expandrivers(planet &world, region &region, int dx, int dy, int sx, int sy, vector<vector<vector<int>>> &circletemplates, bool delta, vector<vector<int>> &fakesourcex, vector<vector<int>> &fakesourcey);
int getriverwidth(region &region, int x, int y, bool delta, int season);
void pasterivercircle(region &region, int centrex, int centrey, int pixels, bool river, vector<vector<vector<int>>> &circletemplates, int dir, int janload, int julload, bool delta, vector<vector<int>> &fakesourcex, vector<vector<int>> &fakesourcey);
void turnriverstolakes(planet &world, region &region, int dx, int dy, int sx, int sy);
void finishrivers(planet &world, region &region, int leftx, int lefty, int rightx, int righty);
void drawriverline (region &region, int leftx, int lefty, int rightx, int righty, int startx, int starty, int endx, int endy, int janload, int julload, int elev);
void makeelevationtile(planet &world, region &region, int dx, int dy, int sx, int sy, int coords[4][2], boolshapetemplate smalllake[]);
int rsquare(region &region, int dx, int dy, int s, int x, int y, int value, int min, int max, int coords[4][2], bool onlyup, int negchance, int sealevel, bool nosea);
int rcoastsquare(region &region, int dx, int dy, int s, int x, int y, int value, int min, int max, int coords[4][2], int sealevel);
int rlakesquare(region &region, int dx, int dy, int s, int x, int y, int value, int min, int max, int coords[4][2], int lakesurface);
int rlakediamond(region &region, int dx, int dy, int s, int x, int y, int value, int min, int max, int coords[4][2], int lakesurface);
int rdiamond(region &region, int dx, int dy, int s, int x, int y, int value, int min, int max, int coords[4][2], bool onlyup, int negchance, int sealevel, bool nosea);
int rcoastdiamond(region &region, int dx, int dy, int s, int x, int y, int value, int min, int max, int coords[4][2], int sealevel);
void landfill(planet &world, region &region, int dx, int dy, int sx, int sy, int surfaceleve, boolshapetemplate smalllake[]);
void removestraights(planet &world, region &region, int dx, int dy, int sx, int sy, boolshapetemplate smalllake[]);
void disruptseacoastline(planet &world, region &region, int dx, int dy, int centrex, int centrey, int avedepth, bool raise, int maxsize, bool stayintile, boolshapetemplate smalllake[]);
void disruptlakecoastline(planet &world, region &region, int dx, int dy, int centrex, int centrey, int surfacelevel, int avedepth, bool raise, int size, bool stayintile, int special, boolshapetemplate smalllake[]);
void removesearivers(planet &world, region &region, int dx, int dy);
void removeextrasearivers(planet &world, region &region, int dx, int dy, int sx, int sy);
void removeriverscomingfromsea(planet &world, region &region, int dx, int dy, int sx, int sy, vector<vector<int>> &fakesourcex, vector<vector<int>> &fakesourcey);
void removeregionalriver(region &region, int dx, int dy, int sx, int sy, int startx, int starty, vector<vector<int>> &fakesourcex, vector<vector<int>> &fakesourcey);
int findsurroundingsea(region &region, int x, int y);
int findinflowinglandriver(region &region, int x, int y);
int checknearbyriver(region &region, int x, int y);
void removeriverlakeloops(planet &world, region &region, int dx, int dy, int sx, int sy, boolshapetemplate smalllake[]);
void makemountaintile(planet &world, region &region, int dx, int dy, int sx, int sy, peaktemplate &peaks, vector<vector<int>> &rmountainmap, vector<vector<int>> &ridgeids, short markgap);
void calculateridges(planet &world, region &region, int dx, int dy, int sx, int sy, sf::Vector2f pt, sf::Vector2f mm1, sf::Vector2f mm2, sf::Vector2f mm3, int newheight, int midheight, int destheight, peaktemplate &peaks, vector<vector<int>> &rmountainmap, int ridgedir, vector<vector<int>> &ridgeids, short markgap);
void assignridgeregions(planet &world, region &region, int dx, int dy, int sx, int sy, vector<vector<int>> &rmountainmap, vector<vector<int>> &ridgeids, vector<vector<int>> &ridgeregions, vector<vector<int>> &nearestridgepointx, vector<vector<int>> &nearestridgepointy, int smallermaxdist, int maxdist);
void findmountainedges(planet &world, region &region, int dx, int dy, int sx, int sy, vector<vector<int>> &nearestridgepointdist, vector<vector<int>> &nearestridgepointx, vector<vector<int>> &nearestridgepointy, vector<vector<bool>> &mountainedges);
void findbuttresspoints(planet &world, region &region, int dx, int dy, int sx, int sy, vector<vector<int>> &ridgeids, vector<vector<int>> &nearestridgepointdist, vector<vector<int>> &nearestridgepointx, vector<vector<int>> &nearestridgepointy, vector<vector<bool>> &mountainedges, vector<vector<bool>> &buttresspoints, int maxdist, int spacing);
void makebuttresses(planet &world, region &region, int dx, int dy, int sx, int sy, vector<vector<int>> &rmountainmap, peaktemplate &peaks, vector<vector<int>> &nearestridgepointx, vector<vector<int>> &nearestridgepointy, vector<vector<int>> &nearestridgepointdist, int maxdist, vector<vector<bool>> &buttresspoints, vector<vector<int>> &ridgeids, short markgap, bool minibuttresses);
void drawpeak(planet &world, region &region, int sx, int sy, int dx, int dy, int x, int y, int peakheight, peaktemplate &peaks, vector<vector<int>> &rmountainmap, bool buttress);
void pastepeak(planet &world, region &region, int x, int y, float peakheight, int templateno, bool leftr, bool downr, peaktemplate &peaks, vector<vector<int>> &rmountainmap);
void removepools(planet &world, region &region, int dx, int dy, int sx, int sy, vector<vector<int>> &pathchecked, int &checkno);
bool findpath(region &region, int &leftx, int &lefty, int &rightx, int &righty, int &fromx, int &fromy, int &destx, int &desty, int &checkno, vector<vector<int>> &pathchecked, int &recursion);
void turnpoolstolakes(planet &world, region &region, int dx, int dy, int sx, int sy, vector<vector<int>> &pathchecked, int &checkno);
bool poolcheck(region &region, int &currentx, int &currenty, int &tally, int maxtally, int &checkno, vector<vector<int>> &pathchecked);
void turntosea(region &region, int leftx, int rightx, int lefty, int righty, int x, int y, int newheight, int sealevel);
void removediagonalwater(region &region, int leftx, int lefty, int rightx, int righty, int sealevel);
void removelakesbysea(region &region, int leftx, int lefty, int rightx, int righty, int sealevel);
void removesinks(planet &world, region &region, int dx, int dy, int sx, int sy);
void fillregionaldepression(region &region, int dx, int dy, int x, int y, int elev);
void checkdepression(region &region, int dx, int dy, int x, int y, int elev, bool &overrun, vector<vector<bool>> &checked);
void filldepression(region &region, int dx, int dy, int x, int y, int elev);
void addinlets(planet &world, region &region, int dx, int dy, int sx, int sy, vector<vector<bool>> &riverinlets);
void pasteinletcircle(region &region, int centrex, int centrey, int depth, int pixels, vector<vector<bool>> &riverinlets);
void addmountainsprings(planet &world, region &region, int dx, int dy, int sx, int sy);
void makewetlandtile(planet &world ,region &region, int dx, int dy, int sx, int sy, boolshapetemplate smalllake[]);
void pasteregionalwetlands(region &region, int centrex, int centrey, int special, int elev, int shapenumber, boolshapetemplate smalllake[]);
void convertlakestospecials(planet &world, region &region, int dx, int dy, int sx, int sy, vector<vector<bool>> &safesaltlakes);
void removedeltasea(planet &world, region &region, int dx, int dy, int sx, int sy);
void adddeltamap(planet &world, region &region, int leftx, int lefty, int rightx, int righty);
void makegenerictile(planet &world, int dx, int dy, int sx, int sy, float valuemod, int coords[4][2], vector<vector<int>> &source, vector<vector<int>> &dest, int max, int min, bool interpolate);
int genericsquare(int dx, int dy, int s, int x, int y, int value, int min, int max, int coords[4][2], vector<vector<int>> &dest);
int genericdiamond(int dx, int dy, int s, int x, int y, int value, int min, int max, int coords[4][2], vector<vector<int>> &dest);
void smoothprecipitation(region &region, int leftx, int lefty, int rightx, int righty, int amount);
int getsurroundingice(planet &world, int x, int y);
void makesmallsaltpans(planet &world, region &region, int dx, int dy, int sx, int sy, vector<vector<bool>> &safesaltlakes, boolshapetemplate smalllake[]);
void makesaltpantile(planet &world, region &region, int dx, int dy, int sx, int sy, boolshapetemplate smalllake[]);
void pastesaltpan(region &region, int centrex, int centrey, int surfacelevel, boolshapetemplate smalllake[]);
void removewetsaltpans(region &region, int leftx, int lefty, int rightx, int righty);
void addbarrierislands(planet &world, region &region, int dx, int dy, int sx, int sy, vector<vector<bool>> &riverinlets);
void addregionalglaciers(planet &world, region &region, int dx, int dy, int sx, int sy);
void makeriftlaketemplates(planet &world, region &region, int dx, int dy, int sx, int sy, int extra, vector<vector<bool>> &riftlakemap);
void makeriftlaketile(planet &world, region &region, int dx, int dy, int sx, int sy, int extra, vector<vector<bool>> &riftlakemap, vector<vector<float>> &riftblob, int riftblobsize);
void marklakeedges(planet &world, region &region, int dx, int dy, int sx, int sy, int extra, vector<vector<bool>> &lakemap);
void makelaketemplates(planet &world, region &region, int dx, int dy, int sx, int sy, int extra, vector<vector<bool>> &lakemap);
void makelaketile(planet &world, region &region, int dx, int dy, int sx, int sy, int extra, vector<vector<bool>> &lakemap, int surfacelevel, int coords[4][2], vector<vector<int>> &source, vector<vector<int>> &dest, vector<vector<bool>> &safesaltlakes, boolshapetemplate smalllake[]);
void makelakeislands(planet &world, region &region, int dx, int dy, int sx, int sy, int surfacelevel, boolshapetemplate island[], vector<vector<bool>> &lakeislands);
void createlakeisland(planet &world, region &region, int centrex, int centrey, int surfacelevel, boolshapetemplate island[], vector<vector<bool>> &lakeislands, bool nooverlap, int special);
void complicatecoastlines(planet &world, region &region, int dx, int dy, int sx, int sy, boolshapetemplate smalllake[], int chance);
void addregionalmountainprecipitation(planet &world, region &region, int dx, int dy, int sx, int sy, bool summer);
void removeregionalstraightrivers(planet &world, region &region, int dx, int dy, int sx, int sy, vector<vector<bool>> &rivercurves);
bool checkrivercurve(region &region, int dx, int dy, sf::Vector2f pt, sf::Vector2f mm1, sf::Vector2f mm2, sf::Vector2f mm3);
void makenewrivercurve(region &region, sf::Vector2f pt, sf::Vector2f mm1, sf::Vector2f mm2, sf::Vector2f mm3, int riverjan, int riverjul);
void addterraces(planet &world, region &region, int dx, int dy, int sx, int sy, boolshapetemplate smalllake[], byteshapetemplate smudge[]);
void addlaketerraces(planet &world, region &region, int dx, int dy, int sx, int sy, boolshapetemplate smalllake[], byteshapetemplate smudge[]);
void disruptcliff(planet &world, region &region, int dx, int dy, int sx, int sy, int startx, int starty, int endx, int endy, boolshapetemplate smalllake[], byteshapetemplate smudge[]);
void disruptlakecliff(planet &world, region &region, int dx, int dy, int sx, int sy, int startx, int starty, int endx, int endy, boolshapetemplate smalllake[], byteshapetemplate smudge[]);
void disruptland(region &region, int centrex, int centrey, int newheight, boolshapetemplate smalllake[]);
void disruptlakebed(region &region, int centrex, int centrey, int newheight, boolshapetemplate smalllake[]);
void checkgrid(planet &world, region &region, int dx, int dy, int sx, int sy, vector<vector<int>> &elevs, vector<vector<int>> &severities);
void makesubmarineelevationtile(planet &world, region &region, int dx, int dy, int sx, int sy, vector<vector<int>> &underseamap, int coords[4][2], int extra);
int submarinesquare(vector<vector<int>> &underseamap, int dx, int dy, int s, int x, int y, int value, int min, int max, int coords[4][2], bool onlyup);
int submarinediamond(vector<vector<int>> &underseamap, int dx, int dy, int s, int x, int y, int value, int min, int max, int coords[4][2], bool onlyup);
int disruptsubmarineelevationtile(planet &world, region &region, int dx, int dy, int sx, int sy, vector<vector<int>> &underseamap, byteshapetemplate smudge[], int extra);
void smudgesubmarineterrain(planet &world, region &region, int centrex, int centrey, int searchdist, vector<vector<int>> &underseamap, byteshapetemplate smudge[]);
void smudgeterrain(planet &world, region &region, int centrex, int centrey, int searchdist, byteshapetemplate smudge[]);
void makesubmarinechannelstile (planet &world, region &region, int dx, int dy, int sx, int sy, vector<vector<int>> &underseabeforechannels, byteshapetemplate smudge[]);
void calculatechannel(planet &world, region &region, sf::Vector2f pt, sf::Vector2f mm1, sf::Vector2f mm2, sf::Vector2f mm3, int startelev, float stepdist, byteshapetemplate smudge[]);
void makesubmarinechannelsmudge(planet &world, region &region, int centrex, int centrey, int lowestelev, byteshapetemplate smudge[]);
void makesubmarineridgelines(planet &world, region &region, int dx, int dy, int s, int sy, vector<vector<bool>> &undersearidgelines);
void marksubmarineridgeline(region &region, vector<vector<bool>> &undersearidgelines, int fromx, int fromy, int tox, int toy);
void drawsubmarineridges(planet &world, region &region, int dx, int dy, int sx, int sy, vector<vector<bool>> &undersearidgelines, peaktemplate &peaks, vector<vector<int>> &undersearidges);
void pastesubmarinepeak(planet &world, region &region, int x, int y, float peakheight, int templateno, peaktemplate &peaks, vector<vector<int>> &undersearidges, int rwidth, int rheight);
void makesubmarineriftradiations(planet &world, region &region, int dx, int dy, int sx, int sy, vector<vector<int>> &undersearidges, peaktemplate &peaks, int extra);
void drawriftspine(planet &world, region &region, int dx, int dy, int sx, int sy, int x1, int y1, int x2, int y2, float peakheight, float heightstep, vector<vector<int>> &underseaspikes, peaktemplate &peaks, bool lower);
void makesubmarineriftmountains(planet &world, region &region, int dx, int dy, int sx, int sy, vector<vector<int>> &undersearidges, peaktemplate &peaks);
void makeoceanicriftmountains(planet &world, region &region, int sx, int sy, int fromx, int fromy, int tox, int toy, vector<vector<int>> &undersearidges, peaktemplate &peaks);
void makesubmarinerift(planet &world, region &region, int dx, int dy, int sx, int sy, vector<vector<int>> &undersearidges, byteshapetemplate smudge[]);
void makesubmarineriftvalley(planet &world, region &region, int sx, int sy, int fromx, int fromy, int tox, int toy, int tileriftheight, vector<vector<int>> &undersearidges, byteshapetemplate smudge[]);
void trimmountainislands(planet &world, region &region, int dx, int dy, int sx, int sy, vector<vector<int>> &rmountainmap);
void removetoohighelevations(planet &world, region &region, int dx, int dy, int sx, int sy);
void makevolcano(planet &world, region &region, int dx, int dy, int sx, int sy, peaktemplate &peaks, vector<vector<int>> &rmountainmap, vector<vector<int>> &ridgeids, int templateno);
void makesubmarinevolcano(planet &world, region &region, int dx, int dy, int sx, int sy, peaktemplate &peaks, vector<vector<int>> &undersearidges);
void fixrivers(planet &world, region &region, int leftx, int lefty, int rightx, int righty);
void findcoastdiagonals(planet &world, region &region, int dx, int dy, int sx, int sy, vector<vector<bool>> &disruptpoints);
void removecoastdiagonals(planet &world, region &region, int dx, int dy, int sx, int sy, vector<vector<bool>> &disruptpoints, boolshapetemplate smalllake[]);
void findlakecoastdiagonals(planet &world, region &region, int dx, int dy, int sx, int sy, vector<vector<bool>> &disruptpoints);
void removelakecoastdiagonals(planet &world, region &region, int dx, int dy, int sx, int sy, vector<vector<bool>> &disruptpoints, boolshapetemplate smalllake[]);
void rotatetileedges(planet &world, region &region, int dx, int dy, int sx, int sy, vector<vector<int>> &rotatearray, bool lakes);
void rotateland(planet &world, region &region, int centrex, int centrey, int maxradius, int angle, vector<vector<int>> &rotatearray);
void rotatelakes(planet &world, region &region, int centrex, int centrey, int maxradius, int angle, vector<vector<int>> &rotatearray);
void rotatetileedgesarray(planet &world, region &region, int dx, int dy, int sx, int sy, vector<vector<int>> &destarray, vector<vector<int>> &rotatearray, int min);
void rotatelandarray(planet &world, region &region, int centrex, int centrey, int maxradius, int angle, vector<vector<int>> &destarray, vector<vector<int>> &rotatearray, int min);
void removesmallislands(planet &world, region &region, int dx, int dy, int sx, int sy);
void removeparallelislands(region &region, int leftx, int lefty, int rightx, int righty, int sealevel);
void removeseaiceglitches(region &region);
void smoothlakebeds(region &region);
void removetoolow(planet &world, region &region, int dx, int dy, int sx, int sy);
void removelakeseas(planet &world, region &region, int dx, int dy, int sx, int sy);
void checklakebeds(region &region, int leftx, int lefty, int rightx, int righty);

#endif /* functions_hpp */
