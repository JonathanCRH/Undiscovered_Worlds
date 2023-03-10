//
//  planet.hpp
//  Undiscovered Worlds
//
//  Created by Jonathan Hill on 22/07/2019.
//
//  Please see functions.hpp for notes.

#ifndef planet_hpp
#define planet_hpp

#include <stdio.h>
#include <string>

#include "classes.hpp"

#define ARRAYWIDTH 2049
#define ARRAYHEIGHT 1025

#define MAXCRATERS 100000

using namespace std;

class planet
{
public:

    planet(); // constructor
    ~planet();  // destructor

    // accessor functions

    int saveversion() const; // save format
    void setsaveversion(int amount);

    int settingssaveversion() const; // save format for settings
    void setsettingssaveversion(int amount);

    long seed() const;   // seed
    void setseed(long amount);

    int size() const;   // size
    void setsize(int amount);

    int width() const;   // global width
    void setwidth(int amount);

    int height() const;  // global height
    void setheight(int amount);

    int type() const;  // planetary type
    void settype(int amount);

    bool rotation() const;   // global rotation
    void setrotation(bool amount);

    float tilt() const; // axial tilt
    void settilt(float amount);

    float eccentricity() const; // orbital eccentricity
    void seteccentricity(float amount);

    int perihelion() const; // perihelion
    void setperihelion (int amount);

    float gravity() const; // surface gravity
    void setgravity(float amount);

    float lunar() const; // strength of lunar gravity
    void setlunar(float amount);

    float tempdecrease() const; // amount temperatures decrease by per 1000 metres
    void settempdecrease(float amount);

    int northpolaradjust() const; // adjustment to north polar temperature
    void setnorthpolaradjust(int amount);

    int southpolaradjust() const; // adjustment to south polar temperature
    void setsouthpolaradjust(int amount);

    int averagetemp() const; // average world temperature
    void setaveragetemp(int amount);

    int northpolartemp() const; // north polar temperature
    void setnorthpolartemp(int amount);

    int southpolartemp() const; // south polar temperature
    void setsouthpolartemp(int amount);

    int eqtemp() const; // equatorial temperature
    void seteqtemp(int amount);

    float waterpickup() const; // amount of water to pick up over oceans
    void setwaterpickup(float amount);

    float riverfactor() const;   // global factor for calculating flow in cubic metres/second
    void setriverfactor(float amount);

    int riverlandreduce() const; // global factor for carving river valleys
    void setriverlandreduce(int amount);

    int estuarylimit() const; // global minimum river flow for estuaries
    void setestuarylimit(int amount);

    int glacialtemp() const; // global maximum temperature for glacially carved terrain
    void setglacialtemp(int amount);

    int glaciertemp() const; // global maximum temperature for rivers to become glaciers
    void setglaciertemp(int amount);

    float mountainreduce() const;    // global factor to reduce mountain heights by a little
    void setmountainreduce(float amount);

    int climatenumber() const;   // total number of possible climates
    void setclimatenumber(int amount);

    int maxelevation() const;    // global maximum elevation
    void setmaxelevation(int amount);

    int sealevel() const;    // global sea level
    void setsealevel(int amount);

    float landshading() const;
    void setlandshading(float amount);

    float lakeshading() const;
    void setlakeshading(float amount);

    float seashading() const;
    void setseashading(float amount);

    int snowchange() const;
    void setsnowchange(int amount); // 1=abrupt; 2=speckled; 3=gradual

    int seaiceappearance() const;
    void setseaiceappearance(int amount);

    bool colourcliffs() const;
    void setcolourcliffs(bool amount);

    int shadingdir() const;
    void setshadingdir(int amount);

    float landmarbling() const;
    void setlandmarbling(float amount);

    float lakemarbling() const;
    void setlakemarbling(float amount);

    float seamarbling() const;
    void setseamarbling(float amount);

    int minriverflowglobal() const;
    void setminriverflowglobal(int amount);

    int minriverflowregional() const;
    void setminriverflowregional(int amount);

    bool showmangroves() const;
    void setshowmangroves(bool amount);

    int landtotal() const;
    void setlandtotal(int amount);

    int seatotal() const;
    void setseatotal(int amount);

    int craterno() const;
    void setcraterno(int amount);

    int maxriverflow() const;    // greatest river flow

    // These are the accessor functions for the map colours.

    int seaice1() const { return itsseaice1; };
    void setseaice1(int amount) { itsseaice1 = amount; };

    int seaice2() const { return itsseaice2; };
    void setseaice2(int amount) { itsseaice2 = amount; };

    int seaice3() const { return itsseaice3; };
    void setseaice3(int amount) { itsseaice3 = amount; };

    int ocean1() const { return itsocean1; };
    void setocean1(int amount) { itsocean1 = amount; };

    int ocean2() const { return itsocean2; };
    void setocean2(int amount) { itsocean2 = amount; };

    int ocean3() const { return itsocean3; };
    void setocean3(int amount) { itsocean3 = amount; };

    int deepocean1() const { return itsdeepocean1; };
    void setdeepocean1(int amount) { itsdeepocean1 = amount; };

    int deepocean2() const { return itsdeepocean2; };
    void setdeepocean2(int amount) { itsdeepocean2 = amount; };

    int deepocean3() const { return itsdeepocean3; };
    void setdeepocean3(int amount) { itsdeepocean3 = amount; };

    int base1() const { return itsbase1; };
    void setbase1(int amount) { itsbase1 = amount; };

    int base2() const { return itsbase2; };
    void setbase2(int amount) { itsbase2 = amount; };

    int base3() const { return itsbase3; };
    void setbase3(int amount) { itsbase3 = amount; };

    int basetemp1() const { return itsbasetemp1; };
    void setbasetemp1(int amount) { itsbasetemp1 = amount; };

    int basetemp2() const { return itsbasetemp2; };
    void setbasetemp2(int amount) { itsbasetemp2 = amount; };

    int basetemp3() const { return itsbasetemp3; };
    void setbasetemp3(int amount) { itsbasetemp3 = amount; };

    int highbase1() const { return itshighbase1; };
    void sethighbase1(int amount) { itshighbase1 = amount; };

    int highbase2() const { return itshighbase2; };
    void sethighbase2(int amount) { itshighbase2 = amount; };

    int highbase3() const { return itshighbase3; };
    void sethighbase3(int amount) { itshighbase3 = amount; };

    int desert1() const { return itsdesert1; };
    void setdesert1(int amount) { itsdesert1 = amount; };

    int desert2() const { return itsdesert2; };
    void setdesert2(int amount) { itsdesert2 = amount; };

    int desert3() const { return itsdesert3; };
    void setdesert3(int amount) { itsdesert3 = amount; };

    int highdesert1() const { return itshighdesert1; };
    void sethighdesert1(int amount) { itshighdesert1 = amount; };

    int highdesert2() const { return itshighdesert2; };
    void sethighdesert2(int amount) { itshighdesert2 = amount; };

    int highdesert3() const { return itshighdesert3; };
    void sethighdesert3(int amount) { itshighdesert3 = amount; };

    int colddesert1() const { return itscolddesert1; };
    void setcolddesert1(int amount) { itscolddesert1 = amount; };

    int colddesert2() const { return itscolddesert2; };
    void setcolddesert2(int amount) { itscolddesert2 = amount; };

    int colddesert3() const { return itscolddesert3; };
    void setcolddesert3(int amount) { itscolddesert3 = amount; };

    int grass1() const { return itsgrass1; };
    void setgrass1(int amount) { itsgrass1 = amount; };

    int grass2() const { return itsgrass2; };
    void setgrass2(int amount) { itsgrass2 = amount; };

    int grass3() const { return itsgrass3; };
    void setgrass3(int amount) { itsgrass3 = amount; };

    int cold1() const { return itscold1; };
    void setcold1(int amount) { itscold1 = amount; };

    int cold2() const { return itscold2; };
    void setcold2(int amount) { itscold2 = amount; };

    int cold3() const { return itscold3; };
    void setcold3(int amount) { itscold3 = amount; };

    int tundra1() const { return itstundra1; };
    void settundra1(int amount) { itstundra1 = amount; };

    int tundra2() const { return itstundra2; };
    void settundra2(int amount) { itstundra2 = amount; };

    int tundra3() const { return itstundra3; };
    void settundra3(int amount) { itstundra3 = amount; };

    int eqtundra1() const { return itseqtundra1; };
    void seteqtundra1(int amount) { itseqtundra1 = amount; };

    int eqtundra2() const { return itseqtundra2; };
    void seteqtundra2(int amount) { itseqtundra2 = amount; };

    int eqtundra3() const { return itseqtundra3; };
    void seteqtundra3(int amount) { itseqtundra3 = amount; };

    int saltpan1() const { return itssaltpan1; };
    void setsaltpan1(int amount) { itssaltpan1 = amount; };

    int saltpan2() const { return itssaltpan2; };
    void setsaltpan2(int amount) { itssaltpan2 = amount; };

    int saltpan3() const { return itssaltpan3; };
    void setsaltpan3(int amount) { itssaltpan3 = amount; };

    int erg1() const { return itserg1; };
    void seterg1(int amount) { itserg1 = amount; };

    int erg2() const { return itserg2; };
    void seterg2(int amount) { itserg2 = amount; };

    int erg3() const { return itserg3; };
    void seterg3(int amount) { itserg3 = amount; };

    int wetlands1() const { return itswetlands1; };
    void setwetlands1(int amount) { itswetlands1 = amount; };

    int wetlands2() const { return itswetlands2; };
    void setwetlands2(int amount) { itswetlands2 = amount; };

    int wetlands3() const { return itswetlands3; };
    void setwetlands3(int amount) { itswetlands3 = amount; };

    int lake1() const { return itslake1; };
    void setlake1(int amount) { itslake1 = amount; };

    int lake2() const { return itslake2; };
    void setlake2(int amount) { itslake2 = amount; };

    int lake3() const { return itslake3; };
    void setlake3(int amount) { itslake3 = amount; };

    int river1() const { return itsriver1; };
    void setriver1(int amount) { itsriver1 = amount; };

    int river2() const { return itsriver2; };
    void setriver2(int amount) { itsriver2 = amount; };

    int river3() const { return itsriver3; };
    void setriver3(int amount) { itsriver3 = amount; };

    int glacier1() const { return itsglacier1; };
    void setglacier1(int amount) { itsglacier1 = amount; };

    int glacier2() const { return itsglacier2; };
    void setglacier2(int amount) { itsglacier2 = amount; };

    int glacier3() const { return itsglacier3; };
    void setglacier3(int amount) { itsglacier3 = amount; };

    int sand1() const { return itssand1; };
    void setsand1(int amount) { itssand1 = amount; };

    int sand2() const { return itssand2; };
    void setsand2(int amount) { itssand2 = amount; };

    int sand3() const { return itssand3; };
    void setsand3(int amount) { itssand3 = amount; };

    int mud1() const { return itsmud1; };
    void setmud1(int amount) { itsmud1 = amount; };

    int mud2() const { return itsmud2; };
    void setmud2(int amount) { itsmud2 = amount; };

    int mud3() const { return itsmud3; };
    void setmud3(int amount) { itsmud3 = amount; };

    int shingle1() const { return itsshingle1; };
    void setshingle1(int amount) { itsshingle1 = amount; };

    int shingle2() const { return itsshingle2; };
    void setshingle2(int amount) { itsshingle2 = amount; };

    int shingle3() const { return itsshingle3; };
    void setshingle3(int amount) { itsshingle3 = amount; };

    int mangrove1() const { return itsmangrove1; };
    void setmangrove1(int amount) { itsmangrove1 = amount; };

    int mangrove2() const { return itsmangrove2; };
    void setmangrove2(int amount) { itsmangrove2 = amount; };

    int mangrove3() const { return itsmangrove3; };
    void setmangrove3(int amount) { itsmangrove3 = amount; };

    int highlight1() const { return itshighlight1; };
    void sethighlight1(int amount) { itshighlight1 = amount; };

    int highlight2() const { return itshighlight2; };
    void sethighlight2(int amount) { itshighlight2 = amount; };

    int highlight3() const { return itshighlight3; };
    void sethighlight3(int amount) { itshighlight3 = amount; };

    // These accessor functions are for location-specific information.
    // They don't check that x and y are valid coordinates.

    int map(int x, int y) const; // total terrain elevation

    bool sea(int x, int y) const; // whether this is sea or not

    bool outline(int x, int y) const;   // whether this is coast, next to sea

    bool coast(int x, int y) const; // whether this is coast, next to land

    void longitude(int x, int& degrees, int& minutes, int& seconds, bool& negative) const;   // returns the longitude of this point

    void latitude(int y, int& degrees, int& minutes, int& seconds, bool& negative) const;   // returns the latitude of this point

    int reverselatitude(int lat) const; // returns the y coordinate of a latitude

    int nom(int x, int y) const; // no-mountains terrain elevation
    void setnom(int x, int y, int amount);

    //int nomridge(int x, int y) const; // no-mountains, with undersea ridge terrain elevation

    int oceanridges(int x, int y) const; // undersea ridges
    void setoceanridges(int x, int y, int amount);

    int oceanridgeheights(int x, int y) const; // undersea ridges
    void setoceanridgeheights(int x, int y, int amount);

    int oceanrifts(int x, int y) const; // undersea rifts
    void setoceanrifts(int x, int y, int amount);

    int oceanridgeoffset(int x, int y) const; // offset map for the ridges
    void setoceanridgeoffset(int x, int y, int amount);

    int oceanridgeangle(int x, int y) const; // the angle perpendicular to the line of the ridge
    void setoceanridgeangle(int x, int y, int amount);

    int volcano(int x, int y) const; // isolated peaks (negative value means it's extinct)
    void setvolcano(int x, int y, int amount);

    bool strato(int x, int y) const; // stratovolcanoes
    void setstrato(int x, int y, bool amount);

    int extraelev(int x, int y) const;   // extra terrain elevation
    void setextraelev(int x, int y, int amount);

    int maxtemp(int x, int y) const; // maximum temperature
    void setmaxtemp(int x, int y, int amount);

    int mintemp(int x, int y) const; // minimum temperature
    void setmintemp(int x, int y, int amount);

    int jantemp(int x, int y) const; // January temperature
    void setjantemp(int x, int y, int amount);

    int jultemp(int x, int y) const; // July temperature
    void setjultemp(int x, int y, int amount);

    int aprtemp(int x, int y) const; // April temperature
    int octtemp(int x, int y) const; // October temperature

    int avetemp(int x, int y) const; // average temperature

    int summerrain(int x, int y) const;  // summer precipitation
    void setsummerrain(int x, int y, int amount);

    int winterrain(int x, int y) const;  // winter precipitation
    void setwinterrain(int x, int y, int amount);

    int janrain(int x, int y) const; // January precipitation
    void setjanrain(int x, int y, int amount);

    int julrain(int x, int y) const; // July precipitation
    void setjulrain(int x, int y, int amount);

    int aprrain(int x, int y) const; // April precipitation
    int octrain(int x, int y) const; // October precipitation

    int averain(int x, int y) const; // average precipitation

    int janmountainrain(int x, int y) const;  // jan precipitation on mountains
    void setjanmountainrain(int x, int y, int amount);

    int julmountainrain(int x, int y) const;  // jul precipitation on mountains
    void setjulmountainrain(int x, int y, int amount);

    int wintermountainrain(int x, int y) const;  // winter precipitation on mountains
    void setwintermountainrain(int x, int y, int amount);

    int summermountainrain(int x, int y) const;  // winter precipitation on mountains
    void setsummermountainrain(int x, int y, int amount);

    int janmountainraindir(int x, int y) const;  // direction of jan precipitation on mountains
    void setjanmountainraindir(int x, int y, int amount);

    int julmountainraindir(int x, int y) const;  // direction of jul precipitation on mountains
    void setjulmountainraindir(int x, int y, int amount);

    int wintermountainraindir(int x, int y) const;  // direction of winter precipitation on mountains
    void setwintermountainraindir(int x, int y, int amount);

    int summermountainraindir(int x, int y) const;  // direction of winter precipitation on mountains
    void setsummermountainraindir(int x, int y, int amount);

    int climate(int x, int y) const; // climate type
    void setclimate(int x, int y, int amount);

    int seaice(int x, int y) const;  // sea ice
    void setseaice(int x, int y, int amount);

    int riverdir(int x, int y) const;    // river flow direction
    void setriverdir(int x, int y, int amount);

    int riverjan(int x, int y) const;    // January river flow volume
    void setriverjan(int x, int y, int amount);

    int riverjul(int x, int y) const;    // July river flow volume
    void setriverjul(int x, int y, int amount);

    int riveraveflow(int x, int y) const; // average river flow

    int wind(int x, int y) const;    // wind direction
    void setwind(int x, int y, int amount);

    int lakesurface(int x, int y) const; // lake surface elevation
    void setlakesurface(int x, int y, int amount);

    int truelake(int x, int y) const; // whether this is a true lake

    float roughness(int x, int y) const;   // roughness
    void setroughness(int x, int y, float amount);

    int mountainridge(int x, int y) const;   // mountain ridge directions
    void setmountainridge(int x, int y, int amount);

    int mountainheight(int x, int y) const;  // mountain elevation
    void setmountainheight(int x, int y, int amount);

    int craterrim(int x, int y) const;  // crater rims
    void setcraterrim(int x, int y, int amount);

    int cratercentre(int x, int y) const;  // crater centre heights
    void setcratercentre(int x, int y, int amount);

    int craterx(int n) const; // crater centre x
    void setcraterx(int n, int amount);

    int cratery(int n) const; // crater centre y
    void setcratery(int n, int amount);

    int craterelev(int n) const; // crater peak elevation
    void setcraterelev(int n, int amount);

    int craterradius(int n) const; // crater radius
    void setcraterradius(int n, int amount);

    int tide(int x, int y) const;    // tidal strength
    void settide(int x, int y, int amount);

    int riftlakesurface(int x, int y) const; // rift lake surface elevation
    void setriftlakesurface(int x, int y, int amount);

    int riftlakebed(int x, int y) const; // rift lake bed elevation
    void setriftlakebed(int x, int y, int amount);

    int special(int x, int y) const; // special features. 100: salt lake. 110: salt pan. 120: dunes. 130: fresh wetlands. 131: brackish wetlands. 132: salt wetlands.
    void setspecial(int x, int y, int amount);

    int deltadir(int x, int y) const;    // delta branch flow direction (reversed)
    int deltadir_no_bounds_check(int x, int y) const { return deltamapdir[x][y]; }
    void setdeltadir(int x, int y, int amount);

    int deltajan(int x, int y) const;    // January delta branch flow volume
    void setdeltajan(int x, int y, int amount);

    int deltajul(int x, int y) const;    // July delta branch flow volume
    void setdeltajul(int x, int y, int amount);

    bool lakestart(int x, int y) const; //  Whether a rift lake starts here
    void setlakestart(int x, int y, int amount);

    bool island(int x, int y) const; // Whether this is a one-tile island
    void setisland(int x, int y, bool amount);

    bool mountainisland(int x, int y) const; // Whether this is an island that's basically a mountain in the sea (if it is, the elevation of what would be the sea bed here is recorded)
    void setmountainisland(int x, int y, bool amount);

    bool noshade(int x, int y) const; // Whether no shading should be applied here on the global map.
    void setnoshade(int x, int y, bool amount);

    int noise(int x, int y) const; // The noise map - used for adding the terraced decoration to parts of the regional map.
    void setnoise(int x, int y, int amount);

    int horse(int x, int y) const;   // horse latitudes
    void sethorse(int x, int y, int amount);

    int test(int x, int y) const;    // Test array.
    void settest(int x, int y, int amount);

    // Now we have wrapped versions of all of those.
    // For these, the function will automatically wrap the X coordinate and clip the Y coordinate if need be.

    bool seawrap(int x, int y) const;
    bool outlinewrap(int x, int y) const;
    int mountainheightwrap(int x, int y) const;

    // Other public functions.

    void clear();   // Clears all of the maps.
    void smoothnom(int amount); // Smoothes the no-mountain map to a given amount.
    void smoothextraelev(int amount); // Smoothes the extra elevation map to a given amount.
    void shiftterrain(int offset); // Shifts the physical terrain by a given amount.
    void smoothrainmaps(int amount); // Smoothes the rain maps by a given amount.
    void setmaxriverflow(); // Calculates the largest river flow on the map.

    void saveworld(string filename); // Saves the world.
    bool loadworld(string filename); // Loads the world.


private:

    // Private variables.

    int itssaveversion; // The save version. Only saves of this version can be loaded in.
    int itssettingssaveversion; // As above, but for settings files.

    int itssize; // Size of planet. 1=tiny; 2=small; 3=Earthlike.
    
    int itswidth;   // width of global map
    int itsheight;  // height of global map

    int itstype; // Type of world
    long itsseed;    // seed number of this world
    bool itsrotation;    // 1 (true) like the Earth, 0 (false) the other way
    float itstilt; // Axial tilt (affects seasonal differences). Earthlike = 22.5
    float itseccentricity; // How elliptical the orbit is. Earthlike = 0.0167.
    short itsperihelion; // Closest point to the sun. Earthlike = 0 (Jan)
    float itsgravity; // Strength of gravity on the surface. Earthlike = 1.0
    float itslunar; // Strength of lunar attraction. Earthlike = 1.0

    float itstempdecrease; // How much temperature decreases with elevation. Earthlike = 6.5/km
    int itsnorthpolaradjust; // Amount to adjust north pole temperature. Earthlike = +3
    int itssouthpolaradjust; // Amount to adjust south pole temperature. Earthlike = -3
    int itsaveragetemp; // Average global temperature. Earthlike = 14
    int itsnorthpolartemp; // Average north polar temperature
    int itssouthpolartemp; // Average south polar temperature
    int itseqtemp; // Average equatorial temperature
    float itswaterpickup; // How much water to pick up over oceans. Default = 1.0
    float itsriverfactor;   // Divide river flow by this to get it in cubic metres/second
    int itsriverlandreduce; // Rivers will carve channels at the base elevation of the tile, minus this
    int itsestuarylimit;    // Rivers this size or above might have estuaries
    int itsglacialtemp;     // Areas with this average temperature or lower will have glacial terrain
    int itsglaciertemp;     // Rivers in areas with this maximum temperature or lower will be glaciers
    float itsmountainreduce;    // Amount to reduce the heights by, to make the global map match the regional map a little better
    int itsclimateno;   // Total number of climate types
    int itsmaxheight;   // Maximum elevation
    int itssealevel;    // Sea level
    int itsmaxriverflow;    // Largest river flow
    int itslandtotal; // Total cells of land
    int itsseatotal; // Total cells of sea

    int itscraterno; // Number of craters

    float itslandshading;
    float itslakeshading;
    float itsseashading; // Amount of shading to do on these areas.

    short itsshadingdir = 4; // Direction of light source for shading.

    short itssnowchange = 1; // Kind of snow border.

    short itsseaiceappearance = 1; // How much sea ice to display.

    bool itscolourcliffs = 1; // If this is 1, high grass colours will only be used on steep slopes.

    float itslandmarbling;
    float itslakemarbling;
    float itsseamarbling; // Amount of marbling to do on these areas on the regional map.

    int itsminriverflowglobal; // Rivers greater than this will be shown on the global map.
    int itsminriverflowregional; // Rivers greater than this will be shown on the regional map.

    bool itsmangroves;

    int itsseaice1;
    int itsseaice2;
    int itsseaice3;

    int itsocean1;
    int itsocean2;
    int itsocean3;

    int itsdeepocean1;
    int itsdeepocean2;
    int itsdeepocean3;

    int itsbase1;
    int itsbase2;
    int itsbase3;

    int itsbasetemp1;
    int itsbasetemp2;
    int itsbasetemp3;

    int itshighbase1;
    int itshighbase2;
    int itshighbase3;

    int itsdesert1;
    int itsdesert2;
    int itsdesert3;

    int itshighdesert1;
    int itshighdesert2;
    int itshighdesert3;

    int itscolddesert1;
    int itscolddesert2;
    int itscolddesert3;

    int itsgrass1;
    int itsgrass2;
    int itsgrass3;

    int itscold1;
    int itscold2;
    int itscold3;

    int itstundra1;
    int itstundra2;
    int itstundra3;

    int itseqtundra1;
    int itseqtundra2;
    int itseqtundra3;

    int itssaltpan1;
    int itssaltpan2;
    int itssaltpan3;

    int itserg1;
    int itserg2;
    int itserg3;

    int itswetlands1;
    int itswetlands2;
    int itswetlands3;

    int itslake1;
    int itslake2;
    int itslake3;

    int itsriver1;
    int itsriver2;
    int itsriver3;

    int itsglacier1;
    int itsglacier2;
    int itsglacier3;

    int itssand1;
    int itssand2;
    int itssand3;

    int itsmud1;
    int itsmud2;
    int itsmud3;

    int itsshingle1;
    int itsshingle2;
    int itsshingle3;

    int itsmangrove1;
    int itsmangrove2;
    int itsmangrove3;

    int itshighlight1;
    int itshighlight2;
    int itshighlight3;

    short jantempmap[ARRAYWIDTH][ARRAYHEIGHT];
    short jultempmap[ARRAYWIDTH][ARRAYHEIGHT];
    short climatemap[ARRAYWIDTH][ARRAYHEIGHT];
    short janrainmap[ARRAYWIDTH][ARRAYHEIGHT];
    short julrainmap[ARRAYWIDTH][ARRAYHEIGHT];
    short janmountainrainmap[ARRAYWIDTH][ARRAYHEIGHT];
    short julmountainrainmap[ARRAYWIDTH][ARRAYHEIGHT];
    short janmountainraindirmap[ARRAYWIDTH][ARRAYHEIGHT];
    short julmountainraindirmap[ARRAYWIDTH][ARRAYHEIGHT];
    short seaicemap[ARRAYWIDTH][ARRAYHEIGHT]; // 1=seasonal, 2=permanent
    short rivermapdir[ARRAYWIDTH][ARRAYHEIGHT];
    int rivermapjan[ARRAYWIDTH][ARRAYHEIGHT];
    int rivermapjul[ARRAYWIDTH][ARRAYHEIGHT];
    int windmap[ARRAYWIDTH][ARRAYHEIGHT];
    int lakemap[ARRAYWIDTH][ARRAYHEIGHT];
    float roughnessmap[ARRAYWIDTH][ARRAYHEIGHT];
    short mountainridges[ARRAYWIDTH][ARRAYHEIGHT];
    short mountainheights[ARRAYWIDTH][ARRAYHEIGHT];
    short craterrims[ARRAYWIDTH][ARRAYHEIGHT];
    short cratercentres[ARRAYWIDTH][ARRAYHEIGHT];
    short mapnom[ARRAYWIDTH][ARRAYHEIGHT];
    short tidalmap[ARRAYWIDTH][ARRAYHEIGHT];
    int riftlakemapsurface[ARRAYWIDTH][ARRAYHEIGHT];
    int riftlakemapbed[ARRAYWIDTH][ARRAYHEIGHT];
    bool lakestartmap[ARRAYWIDTH][ARRAYHEIGHT];
    short specials[ARRAYWIDTH][ARRAYHEIGHT];
    short extraelevmap[ARRAYWIDTH][ARRAYHEIGHT];
    short deltamapdir[ARRAYWIDTH][ARRAYHEIGHT];
    int deltamapjan[ARRAYWIDTH][ARRAYHEIGHT];
    int deltamapjul[ARRAYWIDTH][ARRAYHEIGHT];
    bool islandmap[ARRAYWIDTH][ARRAYHEIGHT];
    bool mountainislandmap[ARRAYWIDTH][ARRAYHEIGHT];
    short noisemap[ARRAYWIDTH][ARRAYHEIGHT];
    short oceanridgemap[ARRAYWIDTH][ARRAYHEIGHT];
    short oceanridgeheightmap[ARRAYWIDTH][ARRAYHEIGHT];
    short oceanriftmap[ARRAYWIDTH][ARRAYHEIGHT];
    short oceanridgeoffsetmap[ARRAYWIDTH][ARRAYHEIGHT];
    short oceanridgeanglemap[ARRAYWIDTH][ARRAYHEIGHT];
    short volcanomap[ARRAYWIDTH][ARRAYHEIGHT];
    bool stratomap[ARRAYWIDTH][ARRAYHEIGHT];
    bool noshademap[ARRAYWIDTH][ARRAYHEIGHT];
    int testmap[ARRAYWIDTH][ARRAYHEIGHT];

    int horselats[ARRAYWIDTH][6];

    fourshorts cratercentreslist[MAXCRATERS]; // This duplicates the information in the cratercentres array, but it tells us the order in which to draw the craters, and their radius, which is also important.

    // reused temporary state
    string line_for_file_read;

    // Private functions.

    int wrapx(int x) const;
    int clipy(int y) const;
    void smooth(int arr[][ARRAYHEIGHT], int amount, bool vary, bool avoidmountains);
    void smooth(int short[][ARRAYHEIGHT], int amount, bool vary, bool avoidmountains);
    void smoothoverland(int arr[][ARRAYHEIGHT], int amount, bool vary);
    void smoothoverland(int short[][ARRAYHEIGHT], int amount, bool vary);

    template<typename T> void shift(T arr[][ARRAYHEIGHT], int offset);
    template<typename T> void writevariable(ofstream& outfile, T val);
    template<typename T> void writedata(ofstream& outfile, T const arr[ARRAYWIDTH][ARRAYHEIGHT]);
    template<typename T> void readvariable(ifstream& infile, T& val);
    template<typename T> void readdata(ifstream& infile, T arr[ARRAYWIDTH][ARRAYHEIGHT]);
};

inline int planet::saveversion() const { return itssaveversion; }
inline void planet::setsaveversion(int amount) { itssaveversion = amount; }

inline int planet::settingssaveversion() const { return itssettingssaveversion; }
inline void planet::setsettingssaveversion(int amount) { itssettingssaveversion = amount; }

inline long planet::seed() const { return itsseed; }
inline void planet::setseed(long amount) { itsseed = amount; }

inline int planet::size() const { return itssize; }
inline void planet::setsize(int amount) { itssize = amount; }

inline int planet::width() const { return itswidth; }
inline void planet::setwidth(int amount) { itswidth = amount; }

inline int planet::height() const { return itsheight; }
inline void planet::setheight(int amount) { itsheight = amount; }

inline int planet::type() const { return itstype; }
inline void planet::settype(int amount) { itstype = amount; }

inline bool planet::rotation() const { return itsrotation; }
inline void planet::setrotation(bool amount) { itsrotation = amount; }

inline float planet::tilt() const { return itstilt; }
inline void planet::settilt(float amount) { itstilt = amount; }

inline float planet::eccentricity() const { return itseccentricity; }
inline void planet::seteccentricity(float amount) { itseccentricity = amount; }

inline int planet::perihelion() const { return (int)itsperihelion; }
inline void planet::setperihelion(int amount) { itsperihelion = (short)amount; }

inline float planet::gravity() const { return itsgravity; }
inline void planet::setgravity(float amount) { itsgravity = amount; }

inline float planet::lunar() const { return itslunar; }
inline void planet::setlunar(float amount) { itslunar = amount; }

inline float planet::tempdecrease() const { return itstempdecrease; }
inline void planet::settempdecrease(float amount) { itstempdecrease = amount; }

inline int planet::northpolaradjust() const { return itsnorthpolaradjust; }
inline void planet::setnorthpolaradjust(int amount) { itsnorthpolaradjust = amount; }

inline int planet::southpolaradjust() const { return itssouthpolaradjust; }
inline void planet::setsouthpolaradjust(int amount) { itssouthpolaradjust = amount; }

inline int planet::averagetemp() const { return itsaveragetemp; }
inline void planet::setaveragetemp(int amount) { itsaveragetemp = amount; }

inline int planet::northpolartemp() const { return itsnorthpolartemp; }
inline void planet::setnorthpolartemp(int amount) { itsnorthpolartemp = amount; }

inline int planet::southpolartemp() const { return itssouthpolartemp; }
inline void planet::setsouthpolartemp(int amount) { itssouthpolartemp = amount; }

inline int planet::eqtemp() const { return itseqtemp; }
inline void planet::seteqtemp(int amount) { itseqtemp = amount; }

inline float planet::waterpickup() const { return itswaterpickup; }
inline void planet::setwaterpickup(float amount) { itswaterpickup = amount; }

inline float planet::riverfactor() const { return itsriverfactor; }
inline void planet::setriverfactor(float amount) { itsriverfactor = amount; }

inline int planet::riverlandreduce() const { return itsriverlandreduce; }
inline void planet::setriverlandreduce(int amount) { itsriverlandreduce = amount; }

inline int planet::estuarylimit() const { return itsestuarylimit; }
inline void planet::setestuarylimit(int amount) { itsestuarylimit = amount; }

inline int planet::glacialtemp() const { return itsglacialtemp; }
inline void planet::setglacialtemp(int amount) { itsglacialtemp = amount; }

inline int planet::glaciertemp() const { return itsglaciertemp; }
inline void planet::setglaciertemp(int amount) { itsglaciertemp = amount; }

inline float planet::mountainreduce() const { return itsmountainreduce; }
inline void planet::setmountainreduce(float amount) { itsmountainreduce = amount; }

inline int planet::climatenumber() const { return itsclimateno; }
inline void planet::setclimatenumber(int amount) { itsclimateno = amount; }

inline int planet::maxelevation() const { return itsmaxheight; }
inline void planet::setmaxelevation(int amount) { itsmaxheight = amount; }

inline int planet::sealevel() const { return itssealevel; }
inline void planet::setsealevel(int amount) { itssealevel = amount; }

inline float planet::landshading() const { return itslandshading; };
inline void planet::setlandshading(float amount) { itslandshading = amount; };

inline float planet::lakeshading() const { return itslakeshading; };
inline void planet::setlakeshading(float amount) { itslakeshading = amount; };

inline float planet::seashading() const { return itsseashading; };
inline void planet::setseashading(float amount) { itsseashading = amount; };

inline int planet::shadingdir() const { return (int)itsshadingdir; };
inline void planet::setshadingdir(int amount) { itsshadingdir = (short)amount; };

inline int planet::snowchange() const { return (int)itssnowchange; };
inline void planet::setsnowchange(int amount) { itssnowchange = (short)amount; };

inline int planet::seaiceappearance() const { return (int)itsseaiceappearance; };
inline void planet::setseaiceappearance(int amount) { itsseaiceappearance = (short)amount; };

inline bool planet::colourcliffs() const { return itscolourcliffs; };
inline void planet::setcolourcliffs(bool amount) { itscolourcliffs = amount; };

inline float planet::landmarbling() const { return itslandmarbling; };
inline void planet::setlandmarbling(float amount) { itslandmarbling = amount; };

inline float planet::lakemarbling() const { return itslakemarbling; };
inline void planet::setlakemarbling(float amount) { itslakemarbling = amount; };

inline float planet::seamarbling() const { return itsseamarbling; };
inline void planet::setseamarbling(float amount) { itsseamarbling = amount; };

inline int planet::minriverflowglobal() const { return itsminriverflowglobal; };
inline void planet::setminriverflowglobal(int amount) { itsminriverflowglobal = amount; };

inline int planet::minriverflowregional() const { return itsminriverflowregional; };
inline void planet::setminriverflowregional(int amount) { itsminriverflowregional = amount; };

inline bool planet::showmangroves() const { return itsmangroves; };
inline void planet::setshowmangroves(bool amount) { itsmangroves = amount; };

inline int planet::landtotal() const { return itslandtotal; };
inline void planet::setlandtotal(int amount) { itslandtotal = amount; };

inline int planet::seatotal() const { return itsseatotal; };
inline void planet::setseatotal(int amount) { itsseatotal = amount; };

inline int planet::craterno() const { return itscraterno; };
inline void planet::setcraterno(int amount) { itscraterno = amount; };

inline int planet::maxriverflow() const { return itsmaxriverflow; }

inline int planet::map(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    int thisvolcano = abs(volcanomap[x][y]);

    if (mapnom[x][y] <= itssealevel)
        return mapnom[x][y] + oceanridgeheightmap[x][y] + thisvolcano;

    if (thisvolcano > mountainheights[x][y])
        return mapnom[x][y] + thisvolcano + extraelevmap[x][y] + craterrims[x][y] + cratercentres[x][y];
    else
        return mapnom[x][y] + mountainheights[x][y] + extraelevmap[x][y] + craterrims[x][y] + cratercentres[x][y];

    return 0;
}

inline int planet::nom(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return (int)mapnom[x][y];
}

inline void planet::setnom(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    mapnom[x][y] = (short)amount;
}

inline int planet::oceanridges(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return (int)oceanridgemap[x][y];
}

inline void planet::setoceanridges(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    oceanridgemap[x][y] = (short)amount;
}

inline int planet::oceanridgeheights(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return (int)oceanridgeheightmap[x][y];
}

inline void planet::setoceanridgeheights(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    oceanridgeheightmap[x][y] = (short)amount;
}

inline int planet::oceanrifts(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return (int)oceanriftmap[x][y];
}

inline void planet::setoceanrifts(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    oceanriftmap[x][y] = (short)amount;
}

inline int planet::oceanridgeoffset(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return (int)oceanridgeoffsetmap[x][y];
}

inline void planet::setoceanridgeoffset(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    oceanridgeoffsetmap[x][y] = (short)amount;
}

inline int planet::oceanridgeangle(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return (int)oceanridgeanglemap[x][y];
}

inline void planet::setoceanridgeangle(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    oceanridgeanglemap[x][y] = (short)amount;
}

inline int planet::volcano(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return (int)volcanomap[x][y];
}

inline void planet::setvolcano(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    volcanomap[x][y] = (short)amount;
}

inline bool planet::strato(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return stratomap[x][y];
}

inline void planet::setstrato(int x, int y, bool amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    stratomap[x][y] = amount;
}

inline int planet::extraelev(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return (int)extraelevmap[x][y];
}

inline void planet::setextraelev(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    extraelevmap[x][y] = (short)amount;
}

inline int planet::jantemp(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return (int)jantempmap[x][y];
}

inline void planet::setjantemp(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    jantempmap[x][y] = (short)amount;
}

inline int planet::jultemp(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return (int)jultempmap[x][y];
}

inline void planet::setjultemp(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    jultempmap[x][y] = (short)amount;
}

inline int planet::aprtemp(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    float summertemp = (float)jultempmap[x][y];
    float wintertemp = (float)jantempmap[x][y];

    if (itsperihelion == 1)
    {
        summertemp = (float)jantempmap[x][y];
        wintertemp = (float)jultempmap[x][y];
    }

    float winterstrength = 0.5f + itseccentricity * 0.5f; // The higher the eccentricity, the shorter the "summer".
    float summerstrength = 1.0f - winterstrength;

    float thistemp = summertemp * summerstrength + wintertemp * winterstrength;

    // Now adjust for four-seasonality (if the obliquity is high)

    float fourseason = itstilt * 0.294592f - 2.45428f;

    float lat = (float)y;

    if (y > itsheight/2.0f)
        lat = (float)(itsheight - y);

    float fourseasonstrength = (float)lat / ((float)itsheight / 2.0f); // This measures the strength of the four-season cycle as determined by proximity to the equator.

    float thistempdiff = (fourseason * fourseasonstrength) / 2.0f;

    thistemp = thistemp + thistempdiff;

    return (int)thistemp;
}

inline int planet::octtemp(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    float summertemp = (float)jultempmap[x][y];
    float wintertemp = (float)jantempmap[x][y];

    if (itsperihelion == (int)1)
    {
        summertemp = (float)jantempmap[x][y];
        wintertemp = (float)jultempmap[x][y];
    }

    float winterstrength = 0.5f + itseccentricity * 0.5f; // The higher the eccentricity, the shorter the "summer".
    float summerstrength = 1.0f - winterstrength;

    float thistemp = summertemp * summerstrength + wintertemp * winterstrength;

    // Now adjust for four-seasonality (if the obliquity is high)

    float fourseason = itstilt * 0.294592f - 2.45428f;

    float lat = (float)y;

    if (y > itsheight / 2)
        lat = (float)(itsheight - y);

    float fourseasonstrength = (float)lat / ((float)itsheight / 2.0f); // This measures the strength of the four-season cycle as determined by proximity to the equator.

    float thistempdiff = (fourseason * fourseasonstrength) / 2.0f;

    thistemp = thistemp + thistempdiff;

    return (int)thistemp;
}


inline int planet::avetemp(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return (int)((jantempmap[x][y] + jultempmap[x][y]) / 2);
}

inline int planet::janrain(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return (int)janrainmap[x][y];
}

inline void planet::setjanrain(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    janrainmap[x][y] = (short)amount;
}

inline int planet::julrain(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return (int)julrainmap[x][y];
}

inline void planet::setjulrain(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    julrainmap[x][y] = (short)amount;
}

inline int planet::aprrain(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    float janfactor = 0.5f;
    float julfactor = 0.5f;

    float janrain = (float)janrainmap[x][y];
    float julrain = (float)julrainmap[x][y];

    if (jultempmap[x][y] > jantempmap[x][y] && julrainmap[x][y] > janrainmap[x][y]) // Monsoon in July
    {
        float monsoonfactor = 1.0f - janrain / julrain;

        janfactor = monsoonfactor * 0.9f; // The stronger the July monsoon, the more the April rain should be like January (low).
        julfactor = 1.0f - janfactor;
    }

    if (jultempmap[x][y] < jantempmap[x][y] && julrainmap[x][y] < janrainmap[x][y]) // Monsoon in January
    {
        float monsoonfactor = 1.0f - julrain / janrain;

        janfactor = monsoonfactor * 0.7f; // The stronger the January monsoon, the more the April rain should be like January (high) (though not *too* close too it!).
        julfactor = 1.0f - janfactor;
    }

    float total = janrain * janfactor + julrain * julfactor;

    return (int)total;
}

inline int planet::octrain(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    float janfactor = 0.5f;
    float julfactor = 0.5f;

    float janrain = (float)janrainmap[x][y];
    float julrain = (float)julrainmap[x][y];

    if (jultempmap[x][y] > jantempmap[x][y] && julrainmap[x][y] > janrainmap[x][y]) // Monsoon in July
    {
        float monsoonfactor = 1.0f - janrain / julrain;

        julfactor = monsoonfactor * 0.7f; // The stronger the July monsoon, the more the October rain should be like July (high) (though not *too* close too it!).
        janfactor = 1.0f - julfactor;
    }

    if (jultempmap[x][y] < jantempmap[x][y] && julrainmap[x][y] < janrainmap[x][y]) // Monsoon in January
    {
        float monsoonfactor = 1.0f - julrain / janrain;

        julfactor = monsoonfactor * 0.9f; // The stronger the January monsoon, the more the April rain should be like July (low).
        janfactor = 1.0f - julfactor;
    }
        
    float total = janrain * janfactor + julrain * julfactor;

    return (int)total;
}

inline int planet::averain(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return (int)((janrainmap[x][y] + julrainmap[x][y]) / 2);
}

inline int planet::janmountainrain(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return (int)janmountainrainmap[x][y];
}

inline void planet::setjanmountainrain(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    janmountainrainmap[x][y] = (short)amount;
}

inline int planet::julmountainrain(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return (int)julmountainrainmap[x][y];
}

inline void planet::setjulmountainrain(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    julmountainrainmap[x][y] = (short)amount;
}

inline int planet::wintermountainrain(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    if (jantempmap[x][y] < jultempmap[x][y])
        return (short)janmountainrainmap[x][y];
    else
        return (short)julmountainrainmap[x][y];
}

inline void planet::setwintermountainrain(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    if (jantempmap[x][y] < jultempmap[x][y])
        janmountainrainmap[x][y] = (short)amount;
    else
        julmountainrainmap[x][y] = (short)amount;
}

inline int planet::summermountainrain(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    if (jantempmap[x][y] >= jultempmap[x][y])
        return (int)janmountainrainmap[x][y];
    else
        return (int)julmountainrainmap[x][y];
}

inline void planet::setsummermountainrain(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    if (jantempmap[x][y] >= jultempmap[x][y])
        janmountainrainmap[x][y] = (short)amount;
    else
        julmountainrainmap[x][y] = (short)amount;
}

inline int planet::janmountainraindir(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return (int)janmountainraindirmap[x][y];
}

inline void planet::setjanmountainraindir(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    janmountainraindirmap[x][y] = (short)amount;
}

inline int planet::julmountainraindir(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return (int)julmountainraindirmap[x][y];
}

inline void planet::setjulmountainraindir(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    julmountainraindirmap[x][y] = (short)amount;
}

inline int planet::wintermountainraindir(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    if (jantempmap[x][y] < jultempmap[x][y])
        return (int)janmountainraindirmap[x][y];
    else
        return (int)julmountainraindirmap[x][y];
}

inline int planet::summermountainraindir(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    if (jantempmap[x][y] >= jultempmap[x][y])
        return (int)janmountainraindirmap[x][y];
    else
        return (int)julmountainraindirmap[x][y];
}

inline void planet::setwintermountainraindir(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    if (jantempmap[x][y] < jultempmap[x][y])
        janmountainraindirmap[x][y] = (short)amount;
    else
        julmountainraindirmap[x][y] = (short)amount;
}

inline void planet::setsummermountainraindir(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    if (jantempmap[x][y] >= jultempmap[x][y])
        janmountainraindirmap[x][y] = (short)amount;
    else
        julmountainraindirmap[x][y] = (short)amount;
}

inline int planet::climate(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return (int)climatemap[x][y];
}

inline void planet::setclimate(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    climatemap[x][y] = (short)amount;
}

inline int planet::seaice(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return (int)seaicemap[x][y];
}

inline void planet::setseaice(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    seaicemap[x][y] = (short)amount;
}

inline int planet::riverdir(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return (short)rivermapdir[x][y];
}

inline void planet::setriverdir(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    rivermapdir[x][y] = (short)amount;
}

inline int planet::riverjan(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return rivermapjan[x][y];
}

inline void planet::setriverjan(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    rivermapjan[x][y] = amount;
}

inline int planet::riverjul(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return rivermapjul[x][y];
}

inline void planet::setriverjul(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    rivermapjul[x][y] = amount;
}

inline int planet::riveraveflow(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return (rivermapjan[x][y] + rivermapjul[x][y]) / 2;
}

inline int planet::wind(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return windmap[x][y];
}

inline void planet::setwind(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    windmap[x][y] = amount;
}

inline int planet::lakesurface(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return lakemap[x][y];
}

inline void planet::setlakesurface(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    lakemap[x][y] = amount;
}

inline float planet::roughness(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return roughnessmap[x][y];
}

inline void planet::setroughness(int x, int y, float amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    roughnessmap[x][y] = amount;
}

inline int planet::mountainridge(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return (int)mountainridges[x][y];
}

inline void planet::setmountainridge(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    mountainridges[x][y] = (short)amount;
}

inline int planet::mountainheight(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return (int)mountainheights[x][y];
}

inline void planet::setmountainheight(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    mountainheights[x][y] = (short)amount;
}

inline int planet::craterrim(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return (int)craterrims[x][y];
}

inline void planet::setcraterrim(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    craterrims[x][y] = (short)amount;
}

inline int planet::cratercentre(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return (int)cratercentres[x][y];
}

inline void planet::setcratercentre(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    cratercentres[x][y] = (short)amount;
}

inline int planet::craterx(int n) const
{
    if (n<0 || n>=MAXCRATERS)
        return 0;

    return (int)cratercentreslist[n].x;
}

inline void planet::setcraterx(int n, int amount)
{
    if (n < 0 || n >= MAXCRATERS)
        return;

    cratercentreslist[n].x=short(amount);
}

inline int planet::cratery(int n) const
{
    if (n < 0 || n >= MAXCRATERS)
        return 0;

    return (int)cratercentreslist[n].y;
}

inline void planet::setcratery(int n, int amount)
{
    if (n < 0 || n >= MAXCRATERS)
        return;

    cratercentreslist[n].y = short(amount);
}

inline int planet::craterelev(int n) const
{
    if (n < 0 || n >= MAXCRATERS)
        return 0;

    return (int)cratercentreslist[n].w;
}

inline void planet::setcraterelev(int n, int amount)
{
    if (n < 0 || n >= MAXCRATERS)
        return;

    cratercentreslist[n].w = short(amount);
}

    inline int planet::craterradius(int n) const
{
    if (n<0 || n>=MAXCRATERS)
        return 0;

    return (int)cratercentreslist[n].z;
}

inline void planet::setcraterradius(int n, int amount)
{
    if (n < 0 || n >= MAXCRATERS)
        return;

    cratercentreslist[n].z=short(amount);
}

inline int planet::tide(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return (int)tidalmap[x][y];
}

inline void planet::settide(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    tidalmap[x][y] = (short)amount;
}

inline int planet::riftlakesurface(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return riftlakemapsurface[x][y];
}

inline void planet::setriftlakesurface(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    riftlakemapsurface[x][y] = amount;
}

inline int planet::riftlakebed(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return riftlakemapbed[x][y];
}

inline void planet::setriftlakebed(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    riftlakemapbed[x][y] = amount;
}

inline int planet::special(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return specials[x][y];
}

inline void planet::setspecial(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    specials[x][y] = (short)amount;
}

inline int planet::deltadir(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return (int)deltamapdir[x][y];
}

inline void planet::setdeltadir(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    deltamapdir[x][y] = (short)amount;
}

inline int planet::deltajan(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return deltamapjan[x][y];
}

inline void planet::setdeltajan(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    deltamapjan[x][y] = amount;
}

inline int planet::deltajul(int x, int y) const
{
    return deltamapjul[x][y];
}

inline void planet::setdeltajul(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    deltamapjul[x][y] = amount;
}

inline bool planet::lakestart(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return lakestartmap[x][y];
}

inline void planet::setlakestart(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    lakestartmap[x][y] = amount;
}

inline bool planet::island(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return islandmap[x][y];
}

inline void planet::setisland(int x, int y, bool amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    islandmap[x][y] = amount;
}

inline bool planet::mountainisland(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return mountainislandmap[x][y];
}

inline void planet::setmountainisland(int x, int y, bool amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    mountainislandmap[x][y] = amount;
}

inline bool planet::noshade(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return noshademap[x][y];
}

inline void planet::setnoshade(int x, int y, bool amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    noshademap[x][y] = amount;
}

inline int planet::noise(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return (int)noisemap[x][y];
}

inline void planet::setnoise(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    noisemap[x][y] = (short)amount;
}

inline int planet::horse(int x, int y) const
{
    if (x<0 || x>itswidth)
        return 0;

    return horselats[x][y];
}

inline void planet::sethorse(int x, int y, int amount)
{
    if (x<0 || x>itswidth)
        return;

    horselats[x][y] = amount;
}

inline int planet::test(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return testmap[x][y];
}

inline void planet::settest(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    testmap[x][y] = amount;
}

inline bool planet::sea(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    if (mapnom[x][y] <= itssealevel && lakemap[x][y] == 0)
        return 1;

    if (volcanomap[x][y] == 0)
        return 0;

    int thisvolcano = abs(volcanomap[x][y]);

    if (mapnom[x][y] + thisvolcano <= itssealevel)
        return 1;
    else
        return 0;
}

inline int planet::mintemp(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    if (jantempmap[x][y] < jultempmap[x][y])
        return (int)jantempmap[x][y];
    else
        return (int)jultempmap[x][y];
}

inline void planet::setmintemp(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    if (jantempmap[x][y] < jultempmap[x][y])
        jantempmap[x][y] = (short)amount;
    else
        jultempmap[x][y] = (short)amount;
}

inline int planet::maxtemp(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    if (jantempmap[x][y] > jultempmap[x][y])
        return (int)jantempmap[x][y];
    else
        return (int)jultempmap[x][y];
}

inline void planet::setmaxtemp(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    if (jantempmap[x][y] > jultempmap[x][y])
        jantempmap[x][y] = (short)amount;
    else
        jultempmap[x][y] = (short)amount;
}

inline int planet::winterrain(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    if (jantempmap[x][y] < jultempmap[x][y])
        return (int)janrainmap[x][y];
    else
        return (int)julrainmap[x][y];
}

inline void planet::setwinterrain(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    if (jantempmap[x][y] < jultempmap[x][y])
        janrainmap[x][y] = (short)amount;
    else
        julrainmap[x][y] = (short)amount;
}

inline int planet::summerrain(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    if (jantempmap[x][y] >= jultempmap[x][y])
        return (int)janrainmap[x][y];
    else
        return (int)julrainmap[x][y];
}

inline void planet::setsummerrain(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    if (jantempmap[x][y] >= jultempmap[x][y])
        janrainmap[x][y] = (short)amount;
    else
        julrainmap[x][y] = (short)amount;
}

inline int planet::truelake(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    if (lakesurface(x, y) != 0 && special(x, y) < 110)
        return (1);
    else
        return(0);
}


#endif /* planet_hpp */