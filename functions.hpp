//
//  functions.hpp
//  Undiscovered Worlds
//
//  Created by Jonathan Hill on 24/07/2019.
//
//  The code for this project is released under the GNU General Public Licence v3.0f - https://choosealicense.com/licenses/gpl-3.0/
//
//  Please note that this code uses Dear ImGui and Raylib, and requires the following library to work:
// 
//  rlImGui - https://github.com/raylib-extras/rlImGui
//
//  main.cpp contains the functions for drawing the map images (as well as the main loop, of course), and all functions that require external libraries.
//  misc.cpp contains various utility functions that are used throughout the program.
//  globalterrain.cpp contains the functions used to generate the global terrain.
//  globalclimate.cpp contains the functions used to calculate the global precipitation and temperature, as well as rivers, lakes, etc.
//
//  There are no asset files other than the app icon and font, and the shaders. The code uses a number of templates, but these are stored in the code itself, in assetdata.cpp.


#ifndef functions_hpp
#define functions_hpp

#include <stdio.h>
#include <stdint.h>

#include "imgui.h"
#include "imgui_impl_raylib.h"
#include "raylib.h"

//#include <glm.hpp>
//#include <gtx\\string_cast.hpp>

#include "classes.hpp"
#include "planet.hpp"
#include "region.hpp"

#define ARRAYWIDTH 512

#define NOISEWIDTH 1025
#define NOISEHEIGHT 513

#define GLOBALMAPTYPES 7

#define REGIONALTILEWIDTH 32
#define REGIONALTILEHEIGHT 32

#define MOUNTAINTEMPLATESTOTAL 27

#define MAXCRATERRADIUS 50

using namespace std;

// Define some enums.

enum screenmodeenum { quit, createworldscreen, creatingworldscreen, globalmapscreen, regionalmapscreen, generatingregionscreen, importscreen, completingimportscreen, movingtoglobalmapscreen, exportareascreen, exportingareascreen, loadingworldscreen, savingworldscreen, generatingtectonicscreen, generatingnontectonicscreen, loadfailure, settingsloadfailure };
enum mapviewenum { elevation, temperature, precipitation, climate, rivers, relief, space };
enum globedirenum { north, east, south, west };

// Declare functions that are in main.cpp

void fast_srand(long seed);
int fast_rand(void);
void createworld(planet& world, simpleplanet& simpleworld, int& edge, vector<float>& longitude, vector<float>& latitude, vector<fourglobepoints>& dirpoint, vector<int>& squareroot, vector<Image>& globalspaceimage, vector<Image>& globalspecularimage, vector<Image>& starsimage, vector <Image>& starhazeimage, vector<Image>& globalnormalimage, int globaltexturesize, int globallargetexturesize, int skytexturesize, float& skyrotate, float& skytilt, float& skypitch, boolshapetemplate landshape[], boolshapetemplate chainland[], boolshapetemplate smalllake[], boolshapetemplate largelake[], vector<float>& proportions, int rainpoint, int panels, vector<vector<vector<Model>>>& regionspherepanels, Shader planetshader, vector<fourdirs>& dircode, int& progressval, string& progresstext);
void createregion(planet& world, int panels, int maxregions, int thisregionobject, globepoint thisregionlocation, vector<globepoint>& regionID, vector<int>& regionloc, vector<short>& regioncreated, vector<short>& regioncreatedloc, int regionaltexturesize, vector<Image>& regionalelevationimage, vector<Image>& regionaltemperatureimage, vector<Image>& regionalprecipitationimage, vector<Image>& regionalclimateimage, vector<Image>& regionalriversimage, vector<Image>& regionalreliefimage, vector<Image>& regionalspaceimage, vector<Image>& regionalspecularimage, vector<Texture2D>& regionaltexture, vector<Texture2D>& regionalspeculartexture, vector<Texture2D>& regionalnormaltexture);
void threadinitialise();
void threadtest(int& val, string& text);
Mesh generatecustommesh(Vector3 p1, Vector3 p2, Vector3 p3, Vector3 p4, Vector3 n1, Vector3 n2, Vector3 n3, Vector3 n4, Vector2 t1, Vector2 t2, Vector2 t3, Vector2 t4);
void setupspherepanels(vector<vector<vector<Model>>>& spherepanels, int panels, float size);
void setupregionalspherepanels(vector<vector<vector<Model>>>& spherepanels, int panels, int worldsize, float size);
float getlatestversion();
void updatereport(int& progressval, string& progresstext, string text);
bool standardbutton(const char* label);
void updatetextures(vector<Image>& images, vector<Texture2D>& textures);
void updatetextures(vector<Image>& images, vector<Texture2D>& textures, int month);
void updateshader(vector<vector<vector<Model>>>& spherepanels, Shader shader, int panels);
void applyspeculartextures(vector<vector<vector<Model>>>& spherepanels, vector<Texture2D>& speculartexture, int panels);
void applynormaltextures(vector<vector<vector<Model>>>& spherepanels, vector<Texture2D>& normaltexture, int panels);
void applyoldspacetextures(vector<vector<vector<Model>>>& spherepanels, vector<Texture2D>& spacetextureold, int panels);
void applyoldspeculartextures(vector<vector<vector<Model>>>& spherepanels, vector<Texture2D>& speculartextureold, int panels);
void applystarhazetextures(vector<vector<vector<Model>>>& skypanels, vector<Texture2D>& starhazetexture, int spanels);

void drawglobe(vector<vector<vector<Model>>>& spherepanels, vector<vector<vector<Model>>>& regionspherepanels, int worldsize, vector<int>& regionloc, vector<Texture2D>& globaltexture, vector<Texture2D>& regionaltexture, int panels, Vector3 globepos, float globerotate, float globetilt, float globepitch, bool draw[6], vector<short>& regioncreated);
void drawbasicglobe(vector<vector<vector<Model>>>& spherepanels, vector<Texture2D>& globaltexture, int panels, Vector3 globepos, float globerotate, float globetilt, float globepitch, bool draw[6]);

void drawglobalpickingimage(planet& world, int globaltexturesize, vector<Image>& globalpickingimage);
Color getclimatecolours(short climate);
void drawstarfield(planet& world, vector<Image>& starsimage, vector<Image>& starhazeimage, int skytexturesize, float& skyrotate, float& skytilt, float& skypitch);
void drawglobalnormalimage(planet& world, simpleplanet& simpleworld, int globaltexturesize, vector<Image>& globalnormalimage, vector<fourglobepoints>& dirpoint, vector<fourdirs>& dircode);
void getlidnormals(planet& world, simpleplanet& simpleworld, int face, int i, int j, int smallindex, int sector, float sectorwidth, float cornerdist, float max, vector<fourdirs>& dircode, Vector3& n);
void drawglobalelevationmapimage(planet& world, int globaltexturesize, vector<Image>& globalelevationimage);
void drawglobaltemperaturemapimage(planet& world, int globaltexturesize, vector<Image>& globaltemperatureimage, vector<float>& latitude);
void drawglobalprecipitationmapimage(planet& world, int globaltexturesize, vector<Image>& globalprecipitationimage);
void drawglobalclimatemapimage(planet& world, int globaltexturesize, vector<Image>& globalclimateimage);
void drawglobalriversmapimage(planet& world, int globaltexturesize, vector<Image>& globalriversimage);
void drawglobalreliefmapimage(planet& world, int globaltexturesize, vector<Image>& globalreliefimage, vector<float>& latitude);
void drawglobalspacemapimage(planet& world, simpleplanet& simpleworld, int globaltexturesize, int globallargetexturesize, vector<Image>& globalspaceimage, vector<Image>& globalspecularimage, vector<float>& latitude, vector<float>& proportions, float rainpoint, int frommonth, int tomonth, int fromface, int toface);


// Declare functions that are in misc.cpp

globepoint getglobepoint(int edge, int face, int x, int y, int xshift, int yshift);
globepoint getdirglobepoint(int edge, int face, int x, int y, int xshift, int yshift);
threefloats getcube(int edge, int face, int x, int y);
void getlonglat(int edge, int face, float x, float y, float& longitude, float& latitude);
threeintegers converttodms(float val);
threefloats getcubepoint(int edge, int face, int x, int y);
globepoint getnorthpoint(int edge, int face, int x, int y, vector<float>& longitude, vector<float>& latitude, float origlong);
globepoint getsouthpoint(int edge, int face, int x, int y, vector<float>& longitude, vector<float>& latitude, float origlong);
globepoint geteastpoint(int edge, int face, int x, int y, vector<float>& longitude, vector<float>& latitude, float origlat);
globepoint getwestpoint(int edge, int face, int x, int y, vector<float>& longitude, vector<float>& latitude, float origlat);
globepoint getnorthpointrough(int edge, int face, int x, int y, vector<float>& longitude, vector<float>& latitude);
globepoint getsouthpointrough(int edge, int face, int x, int y, vector<float>& longitude, vector<float>& latitude);
void createdirectiontable(int edge, vector<fourglobepoints>& dirpoint, vector<float>& longitude, vector<float>& latitude);
void createdircodetable(int edge, vector<fourglobepoints>& dirpoint, vector<fourdirs>& dircode);
globepoint getglobepointfromcode(int edge, int face, int x, int y, int dir);
int getinterdir(int dir1, int dir2);
int getpointdist(int edge, int face1, int x1, int y1, int face2, int x2, int y2);
int getpointdistsquared(int edge, int face1, int x1, int y1, int face2, int x2, int y2);
globepoint getfacepoint(int edge, globepoint startpoint, globepoint destpoint);
void toggle(bool& val);
string formatnumber(int val);
string numbertok(int val);
bool stob(string const& instring);
short stos(string const& instring);
unsigned short stous(string const& instring);
char stoc(string const& instring);
unsigned char stouc(string const& instring);
void saverecord(planet & world, string filename);
void saveendtime(planet& world, string filename);
void savesettings(planet& world, string filename);
bool loadsettings(planet& world, string filename);
void savecontrols(vector<int>& keybindings, int length, float& camerazoomspeed, float& camerarotatespeed, float& planetrotatespeed, float& monthspeed, bool& cameralock, bool& monthrotate, string filename);
void loadcontrols(vector<int>& keybindings, int length, float& camerazoomspeed, float& camerarotatespeed, float& planetrotatespeed, float& monthspeed, bool& cameralock, bool& monthrotate, string filename);
void createriftblob(vector<vector<float>>& riftblob, int size);
int random(int a, int b);
int randomsign(int a);
float randomsign(float a);
int altrandom(int a, int b);
int altrandomsign(int a);
int wrap(int value, int max);
int wrappedaverage(int x, int y, int max);
int normalise(int x, int y, int max);
int tilt(int a, int b, int percentage);
float tilt(float a, float b, int percentage);
void shift(vector<vector<int>>& map, int width, int height, int offset);
void flip(vector<vector<int>>& arr, int awidth, int aheight, bool vert, bool horiz);
void smooth(vector<vector<int>>& arr, int width, int height, int maxelev, int amount, bool vary);
void smooth(vector<vector<vector<int>>>& arr, int edge, int maxelev, int amount);
void smooth(vector<int>& arr, int edge, int maxelev, int amount);
bool edge(vector<vector<bool>>& arr, int width, int height, int i, int j);
void drawline(vector<vector<bool>>& arr, int x1, int y1, int x2, int y2);
void drawlinewrapped(vector<vector<bool>>& arr, int width, int height, int x1, int y1, int x2, int y2);
void drawcircle(vector<vector<int>>& arr, int x, int y, int col, int radius);
void box(vector<vector<int>>& arr, int x1, int y1, int x2, int y2, int col);
void drawcircle3d(vector<vector<vector<int>>>& arr, int x, int y, int col, int radius, int index);
void box3d(vector<vector<vector<int>>>& arr, int x1, int y1, int x2, int y2, int col, int index);
twofloats curvepos(twofloats p0, twofloats p1, twofloats p2, twofloats p3, float t);
void warp(vector<int>& map, int edge, int maxelev, int warpfactorright, int warpfactordown, int warpdetail, bool vary, vector<fourglobepoints>& dirpoint);
void warpold(vector<vector<vector<int>>>& map, int edge, int maxelev, int warpfactorright, int warpfactordown, int warpdetail, bool vary, vector<fourglobepoints>& dirpoint);
void monstrouswarp(vector<int>& map, int edge, int maxelev, int warpfactor);
void monstrouswarpold(vector<vector<vector<int>>>& map, int edge, int maxelev, int warpfactor);
void makevoronoi(vector<vector<vector<int>>>& voronoi, int edge, int points);
void makeregularvoronoi(vector<int>& voronoi, int edge, int pointdist);
void make2Dvoronoi(vector<vector<int>>& voronoi, int width, int height, int points);
void makeworley(vector<int>& worley, int edge, int pointdist, int max, float dropoff);
void makemounds(int edge, vector<int>& arr, int totalmounds, float maxelev);
void curlwarp(int edge, float maxelev, vector<int>& source, vector<int>& warpvector, vector<int>& varyvector, vector<fourglobepoints>& dirpoint);
void drawvectorshape(int edge, int shapenumber, int face, int centrex, int centrey, int val, vector<vector<vector<int>>>& drawmap, boolshapetemplate shape[]);
void drawvectorshape(planet& world, int edge, int shapenumber, int face, int centrex, int centrey, bool val, vector<vector<vector<bool>>>& drawmap, boolshapetemplate shape[]);
void fill(vector<vector<bool>>& arr, int width, int height, int startx, int starty, int replacement);
twointegers nearestsea(planet& world, int face, int i, int j, bool land, int limit, int grain);
twointegers nearestpoint(vector<bool>& arr, int width, int height, int i, int j, int limit, int grain);
twointegers nearestpointold(vector<vector<bool>>& arr, int width, int height, int i, int j, int limit, int grain);
bool coast(planet& world, int face, int x, int y);
bool vaguelycoastal(planet& world, int face, int x, int y);
int getdir(int x, int y, int xx, int yy);
int getdir(int edge, int face, int x, int y, int fface, int xx, int yy);
twointegers getdestination(int x, int y, int dir);
globepoint getdestination(int edge, int face, int x, int y, int dir);
globepoint getflowdestination(planet& world, int face, int x, int y, int dir);
int getridge(planet& world, int face, int x, int y, int dir);
int getridge(vector<vector<int>>& arr, int x, int y, int dir);
int getoceanridge(planet& world, int face, int x, int y, int dir);
void deleteridge(planet& world, int face, int x, int y, int dir);
void deleteridge(planet& world, vector<vector<int>>& ridgesarr, vector<vector<int>>& heightsarr, int x, int y, int dir);
void deleteoceanridge(planet& world, int face, int x, int y, int dir);
int getcode(int dir);
short getmountaindir(int startx, int starty, int endx, int endy);
void cleanmountainridges(planet& world);
int getdestinationland(planet& world, int face, int x, int y, int dir);
void getlandandseatotals(planet& world);
int tempelevadd(planet& world, int temp, int face, int i, int j);
int tempelevadd(planet& world, simpleplanet& simpleworld, int temp, int face, int i, int j);
int tempelevadd(planet& world, region& region, int temp, int i, int j);
int tempelevremove(planet& world, int temp, int face, int i, int j);
int tempelevremove(planet& world, simpleplanet& simpleworld, int temp, int face, int i, int j);
int tempelevremove(planet& world, region& region, int temp, int i, int j);
int getslope(planet& world, int face, int x, int y, int fface, int xx, int yy);
int getflatness(planet& world, int face, int x, int y);
int getflatelevation(planet& world, int face, int x, int y);
int findlowestdir(planet& world, int neighbours[8][2], int face, int x, int y);
int findlowestdir(planet& world, int face, int x, int y, vector<fourglobepoints>& dirpoint);
globepoint findseatile(planet& world, int face, int x, int y, int dir);
twointegers getupstreamcell(planet& world, int face, int x, int y);
twointegers getupstreamcell(planet& world, int x, int y, vector<int>& riverjan, vector<int>& riverjul, vector<int>& riverdir);
twointegers getupstreamcellold(planet& world, int x, int y, vector<vector<int>>& riverjan, vector<vector<int>>& riverjul, vector<vector<int>>& riverdir);
int checkwaterinflow(planet& world, int face, int x, int y);
twointegers gettotalinflow(planet& world, int face, int x, int y);
void makeswirl(vector<int>& arr, int edge, int centreface, int centrex, int centrey, int maxradius, float twists, vector<bool>& done, vector<fourglobepoints>& dirpoint);
threefloats getsunposition(float angle, float distance);
threefloats getcoordinatesfromlatlong(float lat, float lon, float radius);
void createkeycodelist(vector<string>& keycodelist);
void initialisekeybindings(vector<int>& keybindings);
void initialisemovementsettings(float& camerazoomspeed, float& camerarotatespeed, float& planetrotatespeed, float& monthspeed, bool& cameralock, bool& monthrotate);
void initialiseworld(planet& world);
void initialisemapcolours(planet& world);
void initialisespacesettings(planet& world);
void changeworldproperties(planet& world);
void calculatesunpositions(planet& world);
void createsimpleplanet(planet& world, simpleplanet& simpleworld, int& progressval, string& progresstext);
void addsimplemountainpoint(int edge, int face, int x, int y, vector<int>& mountains, int height);
void addsimpleriverpoint(simpleplanet& simpleworld, int face, int x, int y, int janflow, int julflow, vector<int>& addedjanrain, vector<int>& addedjulrain);
void createproportionstable(vector<float>& proportions);
void monthlytemps(planet& world, int face, int x, int y, float arr[12], vector<float>& latitude, float eqtemp, float jantemp, float jultemp);
void monthlyrain(planet& world, int face, int x, int y, float temp[12], float rain[12], float janrain, float julrain);
void monthlyflow(planet& world, float temp[12], float rain[12], int flow[12], float janflow, float julflow);
void expand(vector<int>& from, vector<int>& dest, int fromedge, int destedge, int min, int max, float valuemod);
float getrelativelong(float globerotate, float camlong);
void drawfacecheck(bool drawface[6], float relativelong, float relativelat);
globepoint findregion(planet& world, int panels, vector<short>& regioncreated, vector<short>& regioncreatedloc, vector<globepoint>& regionID, vector<int>& regionloc, globepoint targetregion, bool created);

// Declare functions that are in globalterrain.cpp

void generateglobalterrain(planet& world, bool customgenerate, int iterations, int mergefactor, int clusterno, int clustersize, boolshapetemplate landshape[], boolshapetemplate chainland[], vector<vector<vector<int>>>& mountaindrainage, vector<vector<vector<bool>>>& shelves, vector<float>& longitude, vector<float>& latitude, vector<fourglobepoints>& dirpoint, vector<int>& squareroot, int& progressval, string& progresstext);
void generateglobalterraintype1(planet& world, bool customgenerate, int mergefactor, boolshapetemplate landshape[], vector<vector<vector<int>>>& mountaindrainage, vector<vector<vector<bool>>>& shelves, vector<float>& longitude, vector<float>& latitude, vector<fourglobepoints>& dirpoint, boolshapetemplate chainland[], vector<int>& squareroot, int& progressval, string& progresstext);
void generateglobalterraintype2(planet& world, bool customgenerate, int mergefactor, int clusterno, int clustersize, boolshapetemplate landshape[], vector<vector<vector<int>>>& mountaindrainage, vector<vector<vector<bool>>>& shelves, vector<float>& longitude, vector<float>& latitude, vector<fourglobepoints>& dirpoint, boolshapetemplate chainland[], vector<int>& squareroot, int& progressval, string& progresstext);
void generateglobalterraintype3(planet& world, bool customgenerate, int mergefactor, boolshapetemplate landshape[], vector<vector<vector<int>>>& mountaindrainage, vector<vector<vector<bool>>>& shelves, boolshapetemplate chainland[], int& progressval, string& progresstext);
void generateglobalterraintype4(planet& world, bool customgenerate, int iterations, boolshapetemplate landshape[], vector<vector<vector<int>>>& mountaindrainage, vector<vector<vector<bool>>>& shelves, vector<float>& longitude, vector<float>& latitude, vector<fourglobepoints>& dirpoint, boolshapetemplate chainland[], vector<int>& squareroot, int& progressval, string& progresstext);
void createfractal(vector<int>& arr, int edge, int grain, float valuemod, float valuemod2, int min, int max, bool extreme, bool wrapped);
void createfractalold(vector<vector<vector<int>>>& arr, int edge, int grain, float valuemod, float valuemod2, int min, int max, bool extreme, bool wrapped);
void createfractalformodifiedmerging(vector<vector<vector<int>>>& arr, int edge, int grain, float valuemod, float valuemod2, int min, int max, bool extreme);
void newfractalinit(vector<int>& arr, int edge, int awidth, int aheight, int grain, int min, int max, bool extreme);
void newfractalinitold(vector<vector<int>>& arr, int awidth, int aheight, int grain, int min, int max, bool extreme);
void newfractalinitformodifiedmerging(vector<int>& arr, int awidth, int aheight, int grain, int min, int max, bool extreme);
void createfractalformodifiedmerging(vector<int>& arr, int edge, int grain, float valuemod, float valuemod2, int min, int max, bool extreme);
void createfractalforedgemask(vector<vector<vector<int>>>& arr, int edge, int grain, float valuemod, float valuemod2, int min, int max, bool extreme, bool wrapped);
void newfractal(vector<int>& arr, int edge, int awidth, int aheight, int grain, float valuemod, float valuemod2, int min, int max, bool wrapped);
void newfractalold(vector<vector<int>>& arr, int awidth, int aheight, int grain, float valuemod, float valuemod2, int min, int max, bool wrapped);
void createmilkywayfractal(vector<int>& arr, int edge, int grain, float valuemod, float valuemod2, int min, int max, int milkywaymin, int milkywaymax, bool extreme, bool wrapped);
void newmilkywayfractalinit(vector<int>& arr, int edge, int awidth, int aheight, int grain, int min, int max, int milkywaymin, int milkywaymax, bool extreme);
int square(vector<int>& arr, int edge, int awidth, int aheight, int s, int x, int y, int value, int min, int max, bool wrapped);
int squareold(vector<vector<int>>& arr, int awidth, int aheight, int s, int x, int y, int value, int min, int max, bool wrapped);
int diamond(vector<int>& arr, int edge, int awidth, int aheight, int s, int x, int y, int value, int min, int max, bool wrapped);
int diamondold(vector<vector<int>>& arr, int awidth, int aheight, int s, int x, int y, int value, int min, int max, bool wrapped);
void newlidfractal(vector<int>& arr, vector<int>& cubearr, int edge, int awidth, int aheight, int grain, float valuemod, float valuemod2, int min, int max, bool wrapped, bool top);
void newlidfractalold(vector<vector<int>>& arr, vector<vector<vector<int>>>& cubearr, int awidth, int aheight, int grain, float valuemod, float valuemod2, int min, int max, bool wrapped, bool top);
int lidsquare(vector<int>& arr, vector<int>& cubearr, int edge, int awidth, int aheight, int s, int x, int y, int value, int min, int max, bool wrapped, bool top);
int lidsquareold(vector<vector<int>>& arr, vector<vector<vector<int>>>& cubearr, int awidth, int aheight, int s, int x, int y, int value, int min, int max, bool wrapped, bool top);
int liddiamond(vector<int>& arr, vector<int>& cubearr, int edge, int awidth, int aheight, int s, int x, int y, int value, int min, int max, bool wrapped, bool top);
int liddiamondold(vector<vector<int>>& arr, vector<vector<vector<int>>>& cubearr, int awidth, int aheight, int s, int x, int y, int value, int min, int max, bool wrapped, bool top);
void create2dfractal(vector<int>& arr, int edge, int awidth, int aheight, int grain, float valuemod, float valuemod2, int min, int max, bool extreme, bool wrapped);
void create2dfractalold(vector<vector<int>>& arr, int awidth, int aheight, int grain, float valuemod, float valuemod2, int min, int max, bool extreme, bool wrapped);
void drawshape(planet& world, int shapenumber, int face, int centrex, int centrey, bool land, int baseheight, int conheight, boolshapetemplate shape[]);
void drawmarkedshape(planet& world, int shapenumber, int face, int centrex, int centrey, bool land, int baseheight, int conheight, vector<vector<vector<bool>>>& markedmap, boolshapetemplate shape[]);
void smallcontinents(planet& world, int baseheight, int conheight, vector<vector<vector<int>>>& fractal, vector<vector<vector<int>>>& plateaumap, boolshapetemplate landshape[], boolshapetemplate chainland[]);
void largecontinents(planet& world, int baseheight, int conheight, int clusterno, int clustersize, vector<vector<vector<int>>>& fractal, vector<vector<vector<int>>>& plateaumap, vector<vector<vector<bool>>>& shelves, vector<float>& longitude, vector<float>& latitude, vector<fourglobepoints>& dirpoint, boolshapetemplate landshape[], boolshapetemplate chainland[], int& progressval, string& progresstext);
void makecontinent(planet& world, vector<vector<bool>>& continent, vector<vector<int>>& voronoi, int points, int width, int height, int& leftx, int& rightx, int& lefty, int& righty);
void makecontinentcircles(vector<vector<bool>>& circlemap, int width, int height, int circleno, int circlesize, int& circleleftx, int& circlerightx, int& circlelefty, int& circlerighty);
void removeedgelines(planet& world, int baseheight, int conheight);
void removeweirdstraights(planet& world, int baseheight, int conheight, boolshapetemplate landshape[]);
void removebigstraights(planet& world, int baseheight, int conheight, boolshapetemplate landshape[]);
void markedgemountains(planet& world, vector<vector<vector<int>>>& mountainsraw);
void createmountainsfromraw(planet& world, int face, vector<vector<vector<int>>>& rawmountains);
void createcentralmountains(planet& world, int baseheight, int conheight, boolshapetemplate landshape[], boolshapetemplate chainland[]);
void createdirectedchain(planet& world, int face, int baseheight, int conheight, vector<vector<int>>& fractal, boolshapetemplate landshape[], boolshapetemplate chainland[], int chainstartx, int chainstarty, int chainendx, int chainendy, bool crossingcontinent, bool muststayonland, bool muststayonsea, float heightmult, int volcanochance, int landdelete);
void createchains(planet& world, int baseheight, int conheight, vector<vector<vector<int>>>& fractal, boolshapetemplate landshape[], boolshapetemplate chainland[], twointegers focuspoints[], int focustotal, int focaldistance, int mode);
void removefloatingmountains(planet& world);
void removeinlandseas(planet& world, int level);
void removesmallseas(planet& world, int minseasize, int level);
int areacheck(planet& world, vector<bool>& checked, int startface, int startx, int starty);
void removeshelfgaps(planet& world, vector<vector<vector<bool>>>& shelves);
int nonshelfareacheck(planet& world, vector<vector<vector<bool>>>& shelves, vector<bool>& checked, int startface, int startx, int starty);
void makecontinentalshelves(planet& world, vector<vector<vector<bool>>>& shelves, vector<fourglobepoints>& dirpoint, int pointdist, boolshapetemplate landshape[]);
void cuts(planet& world, int cuttotal, int baseheight, int conheight, boolshapetemplate shape[]);
void fractalmerge(planet& world, int adjust, vector<vector<vector<int>>>& fractal);
void fractalmergeland(planet& world, vector<vector<vector<int>>>& fractal, int conheight);
void fractalmergemodified(planet& world, int adjust, vector<vector<vector<int>>>& fractal, vector<vector<vector<bool>>>& removedland);
void widenchannels(planet& world);
void createoceanridges(planet& world, vector<vector<vector<bool>>>& shelves, vector<float>& longitude, vector<float>& latitude);
bool shelfedge(planet& world, vector<vector<vector<bool>>>& shelves, int face, int x, int y);
void createoceanfault(planet& world, int face, int midx, int midy, int mindist, int maxdist, vector<vector<vector<int>>>& ridgesmap, vector<vector<vector<bool>>>& checked, int masksize);
void createoceantrenches(planet& world, vector<vector<vector<bool>>>& shelves);
void createisolatedvolcano(planet& world, int face, int x, int y, vector<vector<vector<bool>>>& shelves, vector<vector<vector<int>>>& volcanodirection, int peakheight, bool strato);
void raisemountainbases(planet& world, vector<vector<vector<int>>>& mountaindrainage, vector<vector<vector<bool>>>& OKmountains);
void smoothland(planet& world, int amount);
void smoothonlylandvariable(planet& world, int amount);
void createextraelev(planet& world);
void depressionfill(planet& world);
bool checkdepressiontile(planet& world, vector<int>& filledmap, int face, int i, int j, int e, int neighbours[8][2], bool somethingdone, vector<int>& noise);
void checkdepressionedges(planet& world);
void addlandnoise(planet& world);
void normalisecoasts(planet& world, int landheight, int seadepth, int severity);
void clamp(planet& world);
void checkislands(planet& world);
void makemountainisland(planet& world, int face, int x, int y, int peakheight);
void removeunderseabumps(planet& world);
void addfjordmountains(planet& world);
short getmountaindir(planet& world, int startx, int starty, int endx, int endy);
void ridgestomountains(planet& world);
void makebasicterrain(planet& world, float landscale, vector<vector<vector<int>>>& terrain, vector<fourglobepoints>& dirpoint, boolshapetemplate landshape[], bool first);
void scaledownland(float scaleelev, float maxelev, float landscale, int edge, vector<vector<vector<int>>>& terrain);
void removelinesatedges(planet& world);
void createcratermap(planet& world, int cratertotal, vector<int>& squareroot, bool custom);
void makecrater(planet& world, vector <int>& squareroot, int thiscraterno, int centreface, int centrex, int centrey, int size);
void blurinterrain(planet& world, vector<fourglobepoints>& dirpoint, boolshapetemplate landshape[]);
void adjustsealevel(planet& world);
void removeedgeartefacts(planet& world);

// Declare functions that are in globalclimate.cpp

void generateglobalclimate(planet& world, bool dorivers, bool dolakes, bool dodeltas, boolshapetemplate smalllake[], boolshapetemplate largelake[], boolshapetemplate landshape[], vector<vector<vector<int>>>& mountaindrainage, vector<vector<vector<bool>>>& shelves, vector<float>& longitude, vector<float>& latitude, vector<fourglobepoints>& dirpoint, int& progressval, string& progresstext);
void createclimatemap(planet& world);
short getclimate(planet& world, int face, int x, int y);
short getclimate(region& region, int x, int y);
short calculateclimate(int elev, int sealevel, float wrain, float srain, float mintemp, float maxtemp);
string getclimatename(short climate);
string getclimatecode(short climate);
void createsaltlakes(planet& world, int& lakesplaced, vector<twointegers>& saltlakemap, vector<int>& nolake, vector<int>& basins, boolshapetemplate smalllake[]);
void placesaltlake(planet& world, int face, int centrex, int centrey, bool large, bool dodepressions, vector<twointegers>& saltlakemap, vector<int>& basins, vector<int>& avoid, vector<int>& nolake, boolshapetemplate smalllake[]);
void createlakedepression(planet& world, int face, int centrex, int centrey, int origlevel, int steepness, vector<int>& basins, int limit, bool up, vector<int>& avoid);
void convertsaltlakes(planet& world, vector<twointegers>& saltlakemap);
void createwindmap(planet& world, vector<bool>& outsidehorse, vector<float>& longitude, vector<float>& latitude);
void createtemperaturemap(planet& world, vector<int>& fractal, vector<float>& latitude);
void createseaicemap(planet& world, vector<int>& fractal, vector<float>& longitude, vector<float>& latitude);
void createrainmap(planet& world, vector<int>& fractal, int landtotal, int seatotal, vector<bool>& outsidehorse, boolshapetemplate smalllake[], boolshapetemplate shape[], vector<float>& longitude, vector<float>& latitude, vector<fourglobepoints>& dirpoint, int& progressval, string& progresstext);
void createoceanrain(planet& world, vector<fourglobepoints>& dirpoint);
void createprevailinglandrain(planet& world, vector<int>& inland, vector<bool>& outsidehorse, int maxmountainheight, int slopewaterreduce, vector<float>& longitude, vector<fourglobepoints>& dirpoint);
void createmonsoons(planet& world, int maxmountainheight, int slopewaterreduce, vector<float>& latitude, vector<fourglobepoints>& dirpoint);
void adjustseasonalrainfall(planet& world, vector<int>& inland, vector<float>& latitude, vector<fourglobepoints>& dirpoint, vector<bool>& outsidehorse);
void smoothrainfall(planet& world, int maxmountainheight, vector<fourglobepoints>& dirpoint);
void caprainfall(planet& world);
void adjusttemperatures(planet& world, vector<int>& inland);
void adjustcontinentaltemperatures(planet& world, vector<int>& inland);
void smoothtemperatures(planet& world, vector<fourglobepoints>& dirpoint);
void removesubpolarstreaks(planet& world, vector<fourglobepoints>& dirpoint);
void correcttemperatures(planet& world);
void createmountainprecipitation(planet& world);
void createdesertworldrain(planet& world);
void createtidalmap(planet& world, vector<fourglobepoints>& dirpoint);
int gettidalrange(planet& world, int startface, int startx, int starty, vector<fourglobepoints>& dirpoint);
void createrivermap(planet& world, vector<vector<vector<int>>>& mountaindrainage, vector<fourglobepoints>& dirpoint);
void tracedrop(planet& world, int face, int x, int y, int minimum, int dropno, vector<int>& thisdrop, int maxrepeat, int neighbours[8][2], vector<fourglobepoints>& dirpoint);
void removediagonalrivers(planet& world);
void adddiagonalriverjunctions(planet& world);
void removeparallelrivers(planet& world);
void avoidvolcanoes(planet& world);
void removecrossingrivers(planet& world);
void removefacediagonalrivers(planet& world);
void removerivermountains(planet& world);
void createlakemap(planet& world, vector<int>& nolake, boolshapetemplate smalllake[], boolshapetemplate largelake[], vector<fourglobepoints>& dirpoint);
void drawlake(planet& world, int shapenumber, int centreface, int centrex, int centrey, vector<int>& thislake, int lakeno, vector<int>& checked, vector<int>& nolake, int templatesize, int minrain, int maxtemp, boolshapetemplate smalllake[], boolshapetemplate largelake[]);
void drawspeciallake(planet& world, int shapenumber, int centreface, int centrex, int centrey, int lakeno, vector<int>& thislake, boolshapetemplate laketemplate[], bool canoverlap, int special);
void makelakestartpoint(planet& world, vector<int>& thislake, int lakeno, int face, int leftx, int lefty, int rightx, int righty);
int lakeoutline(planet& world, vector<int>& thislake, int lakeno, int face, int x, int y);
void sortlakerivers(planet& world, int face, int leftx, int lefty, int rightx, int righty, int centrex, int centrey, int outflowx, int outflowy, vector<int>& thislake, int lakeno);
void markriver(planet& world, int face, int x, int y, vector<int>& markedarray, int extra);
int nexttolake(planet& world, int face, int x, int y);
int divertriver(planet& world, int face, int x, int y, int destface, int destx, int desty, vector<int>& removedrivers, int riverno);
int divertlakeriver(planet& world, int face, int x, int y, int destface, int destx, int desty, vector<int>& removedrivers, int riverno, int outflowface, int outflowx, int outflowy, vector<int>& avoidarray);
void removeriver(planet& world, vector<int>& removedrivers, int riverno, int face, int x, int y);
void reduceriver(planet& world, int janreduce, int julreduce, vector<int>& removedrivers, int riverno, int face, int x, int y);
void addtoriver(planet& world, int face, int x, int y, int janload, int julload);
void cleanlakes(planet& world);
void lakerain(planet& world, vector<int>& lakewinterrainmap, vector<int>& lakesummerrainmap, vector<fourglobepoints>& dirpoint);
void lakerivers(planet& world, vector<int>& lakewinterrainmap, vector<int>& lakesummerrainmap);
void tracelakedrop(planet& world, int face, int x, int y, int minimum, int dropno, vector<int>& thisdrop, int maxrepeat, int neighbours[8][2], vector<int>& lakewinterrainmap, vector<int>& lakesummerrainmap);
void checkglobalflows(planet& world);
int checkthisflow(planet& world, int face, int x, int y, int found);
void createriftlakemap(planet& world, vector<int>& nolake);
twointegers createriftlake(planet& world, int face, int startx, int starty, int lakelength, vector<int>& nolake);
void createergs(planet& world, boolshapetemplate smalllake[], boolshapetemplate largelake[], boolshapetemplate shape[]);
void checkergelevation(planet& world);
int lowestergelevation(planet& world, int face, int startx, int starty, vector<bool>& thiserg);
void createsaltpans(planet& world, boolshapetemplate smalllake[], boolshapetemplate largelake[]);
void createriverdeltas(planet& world);
void placedelta(planet& world, int centrex, int centrey, int upriver, vector<int>& deltarivers, vector<int>& riverjan, vector<int>& riverjul, vector<int>& riverdir, vector<int>& deltajan, vector<int>& deltajul, vector<int>& deltadir, vector<int>& nom, vector<int>& reducejan, vector<int>& reducejul, vector<bool>& sea, vector<bool>& mask);
void divertdeltarivers(planet& world, vector<int>& deltarivers);
void checkrivers(planet& world);
void createwetlands(planet& world, boolshapetemplate smalllake[]);
void pastewetland(planet& world, int face, int centrex, int centrey, int shapenumber, boolshapetemplate smalllake[]);
void removeexcesswetlands(planet& world);
void refineroughnessmap(planet& world);
void removesealakes(planet& world);
void connectlakes(planet& world);
void correctseasonalrainfall(planet& world);
void correctcoastaltemperatures(planet& world);
void makeclouds(planet& world, vector<fourglobepoints>& dirpoint);





// Declare functions that are in regionalmap.cpp



// Declare functions that are in assetdata.cpp

void loadpeaktemplates(peaktemplate& peaks);
void loadpeaktemplate0(peaktemplate& peaks);
void loadpeaktemplate1(peaktemplate& peaks);
void loadpeaktemplate2(peaktemplate& peaks);
void loadpeaktemplate3(peaktemplate& peaks);
int createmountainrangetemplate(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate, int tempno, int dir);
int createmountainrangetemplate1(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate, int dir);
int createmountainrangetemplate1_1(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate1_2(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate1_3(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate1_4(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate1_5(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate1_6(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate1_7(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate1_8(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate2(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate, int dir);
int createmountainrangetemplate2_1(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate2_2(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate2_3(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate2_4(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate2_5(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate2_6(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate2_7(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate2_8(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate3(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate, int dir);
int createmountainrangetemplate3_1(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate3_2(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate3_3(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate3_4(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate3_5(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate3_6(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate3_7(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate3_8(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate4(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate, int dir);
int createmountainrangetemplate4_1(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate4_2(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate4_3(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate4_4(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate4_5(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate4_6(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate4_7(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate4_8(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate5(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate, int dir);
int createmountainrangetemplate5_1(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate5_2(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate5_3(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate5_4(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate5_5(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate5_6(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate5_7(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate5_8(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate6(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate, int dir);
int createmountainrangetemplate6_1(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate6_2(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate6_3(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate6_4(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate6_5(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate6_6(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate6_7(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate6_8(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate7(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate, int dir);
int createmountainrangetemplate7_1(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate7_2(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate7_3(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate7_4(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate7_5(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate7_6(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate7_7(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate7_8(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate8(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate, int dir);
int createmountainrangetemplate8_1(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate8_2(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate8_3(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate8_4(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate8_5(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate8_6(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate8_7(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate8_8(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate9(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate, int dir);
int createmountainrangetemplate9_1(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate9_2(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate9_3(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate9_4(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate9_5(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate9_6(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate9_7(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate9_8(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate10(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate, int dir);
int createmountainrangetemplate10_1(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate10_2(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate10_3(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate10_4(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate10_5(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate10_6(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate10_7(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate10_8(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate11(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate, int dir);
int createmountainrangetemplate11_1(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate11_2(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate11_3(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate11_4(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate11_5(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate11_6(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate11_7(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate11_8(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate12(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate, int dir);
int createmountainrangetemplate12_1(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate12_2(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate12_3(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate12_4(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate12_5(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate12_6(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate12_7(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate12_8(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate13(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate, int dir);
int createmountainrangetemplate13_1(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate13_2(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate13_3(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate13_4(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate13_5(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate13_6(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate13_7(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate13_8(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate14(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate, int dir);
int createmountainrangetemplate14_1(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate14_2(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate14_3(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate14_4(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate14_5(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate14_6(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate14_7(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate14_8(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate15(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate, int dir);
int createmountainrangetemplate15_1(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate15_2(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate15_3(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate15_4(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate15_5(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate15_6(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate15_7(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate15_8(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate16(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate, int dir);
int createmountainrangetemplate16_1(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate16_2(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate16_3(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate16_4(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate16_5(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate16_6(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate16_7(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate16_8(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate17(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate, int dir);
int createmountainrangetemplate17_1(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate17_2(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate17_3(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate17_4(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate17_5(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate17_6(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate17_7(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate17_8(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate18(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate, int dir);
int createmountainrangetemplate18_1(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate18_2(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate18_3(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate18_4(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate18_5(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate18_6(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate18_7(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate18_8(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate19(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate, int dir);
int createmountainrangetemplate19_1(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate19_2(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate19_3(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate19_4(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate19_5(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate19_6(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate19_7(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate19_8(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate20(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate, int dir);
int createmountainrangetemplate20_1(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate20_2(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate20_3(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate20_4(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate20_5(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate20_6(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate20_7(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate20_8(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate21(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate, int dir);
int createmountainrangetemplate21_1(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate21_2(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate21_3(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate21_4(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate21_5(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate21_6(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate21_7(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate21_8(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate22(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate, int dir);
int createmountainrangetemplate22_1(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate22_2(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate22_3(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate22_4(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate22_5(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate22_6(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate22_7(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate22_8(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate23(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate, int dir);
int createmountainrangetemplate23_1(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate23_2(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate23_3(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate23_4(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate23_5(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate23_6(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate23_7(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate23_8(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate24(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate, int dir);
int createmountainrangetemplate24_1(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate24_2(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate24_3(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate24_4(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate24_5(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate24_6(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate24_7(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate24_8(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate25(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate, int dir);
int createmountainrangetemplate25_1(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate25_2(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate25_3(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate25_4(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate25_5(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate25_6(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate25_7(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate25_8(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate26(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate, int dir);
int createmountainrangetemplate26_1(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate26_2(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate26_3(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate26_4(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate26_5(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate26_6(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate26_7(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate26_8(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate27(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate, int dir);
int createmountainrangetemplate27_1(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate27_2(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate27_3(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate27_4(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate27_5(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate27_6(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate27_7(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createmountainrangetemplate27_8(vector<vector<unsigned char>>& dirtemplate, vector<vector<unsigned char>>& heighttemplate);
int createsmalllaketemplates(boolshapetemplate smalllake[]);
void createsmalllaketemplate0(boolshapetemplate smalllake[]);
void createsmalllaketemplate1(boolshapetemplate smalllake[]);
void createsmalllaketemplate2(boolshapetemplate smalllake[]);
void createsmalllaketemplate3(boolshapetemplate smalllake[]);
void createsmalllaketemplate4(boolshapetemplate smalllake[]);
void createsmalllaketemplate5(boolshapetemplate smalllake[]);
void createsmalllaketemplate6(boolshapetemplate smalllake[]);
void createsmalllaketemplate7(boolshapetemplate smalllake[]);
void createsmalllaketemplate8(boolshapetemplate smalllake[]);
void createsmalllaketemplate9(boolshapetemplate smalllake[]);
void createsmalllaketemplate10(boolshapetemplate smalllake[]);
void createsmalllaketemplate11(boolshapetemplate smalllake[]);
int createislandtemplates(boolshapetemplate island[]);
void createislandtemplate0(boolshapetemplate island[]);
void createislandtemplate1(boolshapetemplate island[]);
void createislandtemplate2(boolshapetemplate island[]);
void createislandtemplate3(boolshapetemplate island[]);
void createislandtemplate4(boolshapetemplate island[]);
void createislandtemplate5(boolshapetemplate island[]);
void createislandtemplate6(boolshapetemplate island[]);
void createislandtemplate7(boolshapetemplate island[]);
void createislandtemplate8(boolshapetemplate island[]);
void createislandtemplate9(boolshapetemplate island[]);
void createislandtemplate10(boolshapetemplate island[]);
void createislandtemplate11(boolshapetemplate island[]);
int createsmudgetemplates(byteshapetemplate smudge[]);
void createsmudgetemplate1(byteshapetemplate smudge[]);
void createsmudgetemplate2(byteshapetemplate smudge[]);
void createsmudgetemplate3(byteshapetemplate smudge[]);
void createsmudgetemplate4(byteshapetemplate smudge[]);
void createsmudgetemplate5(byteshapetemplate smudge[]);
int createsmallsmudgetemplates(byteshapetemplate smallsmudge[]);
void createsmallsmudgetemplate1(byteshapetemplate smallsmudge[]);
void createsmallsmudgetemplate2(byteshapetemplate smallsmudge[]);
void createsmallsmudgetemplate3(byteshapetemplate smallsmudge[]);
void createsmallsmudgetemplate4(byteshapetemplate smallsmudge[]);
void createsmallsmudgetemplate5(byteshapetemplate smallsmudge[]);
int createlargelaketemplates(boolshapetemplate largelake[]);
void createlargelaketemplate0(boolshapetemplate largelake[]);
void createlargelaketemplate1(boolshapetemplate largelake[]);
void createlargelaketemplate2(boolshapetemplate largelake[]);
void createlargelaketemplate3(boolshapetemplate largelake[]);
void createlargelaketemplate4(boolshapetemplate largelake[]);
void createlargelaketemplate5(boolshapetemplate largelake[]);
void createlargelaketemplate6(boolshapetemplate largelake[]);
void createlargelaketemplate7(boolshapetemplate largelake[]);
void createlargelaketemplate8(boolshapetemplate largelake[]);
void createlargelaketemplate9(boolshapetemplate largelake[]);
int createlandshapetemplates(boolshapetemplate landshape[]);
void createlandshapetemplate0(boolshapetemplate landshape[]);
void createlandshapetemplate1(boolshapetemplate landshape[]);
void createlandshapetemplate2(boolshapetemplate landshape[]);
void createlandshapetemplate3(boolshapetemplate landshape[]);
void createlandshapetemplate4(boolshapetemplate landshape[]);
void createlandshapetemplate5(boolshapetemplate landshape[]);
void createlandshapetemplate6(boolshapetemplate landshape[]);
void createlandshapetemplate7(boolshapetemplate landshape[]);
void createlandshapetemplate8(boolshapetemplate landshape[]);
void createlandshapetemplate9(boolshapetemplate landshape[]);
void createlandshapetemplate10(boolshapetemplate landshape[]);
void createlandshapetemplate11(boolshapetemplate landshape[]);
int createchainlandtemplates(boolshapetemplate chainland[]);
void createchainlandtemplate0(boolshapetemplate chainland[]);
void createchainlandtemplate1(boolshapetemplate chainland[]);

#endif /* functions_hpp */