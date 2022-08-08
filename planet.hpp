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

#define ARRAYWIDTH 2049
#define ARRAYHEIGHT 1025

#define NOISEWIDTH 1025
#define NOISEHEIGHT 513

using namespace std;

class planet
{
public:
    
    planet(); // constructor
    ~planet();  // destructor
    
    // accessor functions
    
    long seed() const;   // seed
    void setseed(long amount);
    
    int width() const;   // global width
    void setwidth(int amount);
    
    int height() const;  // global height
    void setheight(int amount);
    
    bool rotation() const;   // global rotation
    void setrotation(bool amount);
    
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
    
    int noisewidth() const;
    void setnoisewidth(int amount);
    
    int noiseheight() const;
    void setnoiseheight(int amount); // dimensions of the noise map
    
    float landshading() const;
    void setlandshading(float amount);
    
    float lakeshading() const;
    void setlakeshading(float amount);
    
    float seashading() const;
    void setseashading(float amount);
    
    short snowchange() const;
    void setsnowchange(short amount); // 1=abrupt; 2=speckled; 3=gradual
    
    short seaiceappearance() const;
    void setseaiceappearance(short amount);
    
    short shadingdir() const;
    void setshadingdir(short amount);
    
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
    
    int maxriverflow() const;    // greatest river flow
    
    // These are the accessor functions for the map colours.
    
    int seaice1() const{return itsseaice1;};
    void setseaice1(int amount){itsseaice1=amount;};
    
    int seaice2() const{return itsseaice2;};
    void setseaice2(int amount){itsseaice2=amount;};
    
    int seaice3() const{return itsseaice3;};
    void setseaice3(int amount){itsseaice3=amount;};
    
    int ocean1() const{return itsocean1;};
    void setocean1(int amount){itsocean1=amount;};
    
    int ocean2() const{return itsocean2;};
    void setocean2(int amount){itsocean2=amount;};
    
    int ocean3() const{return itsocean3;};
    void setocean3(int amount){itsocean3=amount;};
    
    int deepocean1() const{return itsdeepocean1;};
    void setdeepocean1(int amount){itsdeepocean1=amount;};
    
    int deepocean2() const{return itsdeepocean2;};
    void setdeepocean2(int amount){itsdeepocean2=amount;};
    
    int deepocean3() const{return itsdeepocean3;};
    void setdeepocean3(int amount){itsdeepocean3=amount;};
    
    int base1() const{return itsbase1;};
    void setbase1(int amount){itsbase1=amount;};
    
    int base2() const{return itsbase2;};
    void setbase2(int amount){itsbase2=amount;};
    
    int base3() const{return itsbase3;};
    void setbase3(int amount){itsbase3=amount;};
    
    int basetemp1() const{return itsbasetemp1;};
    void setbasetemp1(int amount){itsbasetemp1=amount;};
    
    int basetemp2() const{return itsbasetemp2;};
    void setbasetemp2(int amount){itsbasetemp2=amount;};
    
    int basetemp3() const{return itsbasetemp3;};
    void setbasetemp3(int amount){itsbasetemp3=amount;};
    
    int highbase1() const{return itshighbase1;};
    void sethighbase1(int amount){itshighbase1=amount;};
    
    int highbase2() const{return itshighbase2;};
    void sethighbase2(int amount){itshighbase2=amount;};
    
    int highbase3() const{return itshighbase3;};
    void sethighbase3(int amount){itshighbase3=amount;};
    
    int desert1() const{return itsdesert1;};
    void setdesert1(int amount){itsdesert1=amount;};
    
    int desert2() const{return itsdesert2;};
    void setdesert2(int amount){itsdesert2=amount;};
    
    int desert3() const{return itsdesert3;};
    void setdesert3(int amount){itsdesert3=amount;};
    
    int highdesert1() const{return itshighdesert1;};
    void sethighdesert1(int amount){itshighdesert1=amount;};
    
    int highdesert2() const{return itshighdesert2;};
    void sethighdesert2(int amount){itshighdesert2=amount;};
    
    int highdesert3() const{return itshighdesert3;};
    void sethighdesert3(int amount){itshighdesert3=amount;};
    
    int colddesert1() const{return itscolddesert1;};
    void setcolddesert1(int amount){itscolddesert1=amount;};
    
    int colddesert2() const{return itscolddesert2;};
    void setcolddesert2(int amount){itscolddesert2=amount;};
    
    int colddesert3() const{return itscolddesert3;};
    void setcolddesert3(int amount){itscolddesert3=amount;};
    
    int grass1() const{return itsgrass1;};
    void setgrass1(int amount){itsgrass1=amount;};
    
    int grass2() const{return itsgrass2;};
    void setgrass2(int amount){itsgrass2=amount;};
    
    int grass3() const{return itsgrass3;};
    void setgrass3(int amount){itsgrass3=amount;};
    
    int cold1() const{return itscold1;};
    void setcold1(int amount){itscold1=amount;};
    
    int cold2() const{return itscold2;};
    void setcold2(int amount){itscold2=amount;};
    
    int cold3() const{return itscold3;};
    void setcold3(int amount){itscold3=amount;};
    
    int tundra1() const{return itstundra1;};
    void settundra1(int amount){itstundra1=amount;};
    
    int tundra2() const{return itstundra2;};
    void settundra2(int amount){itstundra2=amount;};
    
    int tundra3() const{return itstundra3;};
    void settundra3(int amount){itstundra3=amount;};
    
    int eqtundra1() const{return itseqtundra1;};
    void seteqtundra1(int amount){itseqtundra1=amount;};
    
    int eqtundra2() const{return itseqtundra2;};
    void seteqtundra2(int amount){itseqtundra2=amount;};
    
    int eqtundra3() const{return itseqtundra3;};
    void seteqtundra3(int amount){itseqtundra3=amount;};
    
    int saltpan1() const{return itssaltpan1;};
    void setsaltpan1(int amount){itssaltpan1=amount;};
    
    int saltpan2() const{return itssaltpan2;};
    void setsaltpan2(int amount){itssaltpan2=amount;};
    
    int saltpan3() const{return itssaltpan3;};
    void setsaltpan3(int amount){itssaltpan3=amount;};
    
    int erg1() const{return itserg1;};
    void seterg1(int amount){itserg1=amount;};
    
    int erg2() const{return itserg2;};
    void seterg2(int amount){itserg2=amount;};
    
    int erg3() const{return itserg3;};
    void seterg3(int amount){itserg3=amount;};
    
    int wetlands1() const{return itswetlands1;};
    void setwetlands1(int amount){itswetlands1=amount;};
    
    int wetlands2() const{return itswetlands2;};
    void setwetlands2(int amount){itswetlands2=amount;};
    
    int wetlands3() const{return itswetlands3;};
    void setwetlands3(int amount){itswetlands3=amount;};
    
    int lake1() const{return itslake1;};
    void setlake1(int amount){itslake1=amount;};
    
    int lake2() const{return itslake2;};
    void setlake2(int amount){itslake2=amount;};
    
    int lake3() const{return itslake3;};
    void setlake3(int amount){itslake3=amount;};
    
    int river1() const{return itsriver1;};
    void setriver1(int amount){itsriver1=amount;};
    
    int river2() const{return itsriver2;};
    void setriver2(int amount){itsriver2=amount;};
    
    int river3() const{return itsriver3;};
    void setriver3(int amount){itsriver3=amount;};
    
    int glacier1() const{return itsglacier1;};
    void setglacier1(int amount){itsglacier1=amount;};
    
    int glacier2() const{return itsglacier2;};
    void setglacier2(int amount){itsglacier2=amount;};
    
    int glacier3() const{return itsglacier3;};
    void setglacier3(int amount){itsglacier3=amount;};
    
    int highlight1() const{return itshighlight1;};
    void sethighlight1(int amount){itshighlight1=amount;};
    
    int highlight2() const{return itshighlight2;};
    void sethighlight2(int amount){itshighlight2=amount;};
    
    int highlight3() const{return itshighlight3;};
    void sethighlight3(int amount){itshighlight3=amount;};
    
    // These accessor functions are for location-specific information.
    // They don't check that x and y are valid coordinates.
    
    int map(int x, int y) const; // total terrain elevation
    
    bool sea(int x, int y) const; // whether this is sea or not
    
    bool outline(int x, int y) const;   // whether this is coast, next to sea
    
    bool coast(int x, int y) const; // whether this is coast, next to land
    
    int latitude(int x, int y) const;   // returns the latitude of this point
    
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
    
    int avetemp(int x, int y) const; // average temperature
    
    int summerrain(int x, int y) const;  // summer precipitation
    void setsummerrain(int x, int y, int amount);
    
    int winterrain(int x, int y) const;  // winter precipitation
    void setwinterrain(int x, int y, int amount);
    
    int janrain(int x, int y) const; // January precipitation
    void setjanrain(int x, int y, int amount);
    
    int julrain(int x, int y) const; // July precipitation
    void setjulrain(int x, int y, int amount);
    
    int averain(int x, int y) const; // average precipitation
    
    int wintermountainrain(int x, int y) const;  // winter precipitation on mountains
    void setwintermountainrain(int x, int y, int amount);
    
    int summermountainrain(int x, int y) const;  // summer precipitation on mountains
    void setsummermountainrain(int x, int y, int amount);
    
    short wintermountainraindir(int x, int y) const;  // direction of winter precipitation on mountains
    void setwintermountainraindir(int x, int y, short amount);
    
    short summermountainraindir(int x, int y) const;  // direction of summer precipitation on mountains
    void setsummermountainraindir(int x, int y, short amount);
    
    short climate(int x, int y) const; // climate type
    void setclimate(int x, int y, short amount);
    
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
    
    int tide(int x, int y) const;    // tidal strength
    void settide(int x, int y, int amount);
    
    int riftlakesurface(int x, int y) const; // rift lake surface elevation
    void setriftlakesurface(int x, int y, int amount);
    
    int riftlakebed(int x, int y) const; // rift lake bed elevation
    void setriftlakebed(int x, int y, int amount);
    
    int special(int x, int y) const; // special features. 100: salt lake. 110: salt pan. 120: dunes. 130: fresh wetlands. 131: brackish wetlands. 132: salt wetlands.
    void setspecial(int x, int y, int amount);
    
    int deltadir(int x, int y) const;    // delta branch flow direction (reversed)
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
    
    int noisemap(int x, int y) const; // The noise map - used for adding the terraced decoration to parts of the regional map.
    void setnoisemap(int x, int y, int amount);
    
    int horse(int x, int y) const;   // horse latitudes
    void sethorse(int x, int y, int amount);
    
    int test(int x, int y) const;    // Test array.
    void settest(int x, int y, int amount);
    
    // Now we have wrapped versions of all of those.
    // For these, the function will automatically wrap the X coordinate and clip the Y coordinate if need be.
    
    int mapwrap(int x, int y) const;
    
    bool seawrap(int x, int y) const;
    
    bool outlinewrap(int x, int y) const;
    
    int latitudewrap(int x, int y) const;
    
    int nomwrap(int x, int y) const;
    void setnomwrap(int x, int y, int amount);
    
    int extraelevwrap(int x, int y) const;
    void setextraelevwrap(int x, int y, int amount);
    
    int maxtempwrap(int x, int y) const;
    void setmaxtempwrap(int x, int y, int amount);
    
    int mintempwrap(int x, int y) const;
    void setmintempwrap(int x, int y, int amount);
    
    int jantempwrap(int x, int y) const;
    void setjantempwrap(int x, int y, int amount);
    
    int jultempwrap(int x, int y) const;
    void setjultempwrap(int x, int y, int amount);
    
    int avetempwrap(int x, int y) const;
    
    int summerrainwrap(int x, int y) const;
    void setsummerrainwrap(int x, int y, int amount);
    
    int winterrainwrap(int x, int y) const;
    void setwinterrainwrap(int x, int y, int amount);
    
    int janrainwrap(int x, int y) const;
    void setjanrainwrap(int x, int y, int amount);
    
    int julrainwrap(int x, int y) const;
    void setjulrainwrap(int x, int y, int amount);
    
    int averainwrap(int x, int y) const;
    
    short climatewrap(int x, int y) const;
    void setclimatewrap(int x, int y, short amount);
    
    int seaicewrap(int x, int y) const;
    void setseaicewrap(int x, int y, int amount);
    
    int riverdirwrap(int x, int y) const;
    void setriverdirwrap(int x, int y, int amount);
    
    int riverjanwrap(int x, int y) const;
    void setriverjanwrap(int x, int y, int amount);
    
    int riverjulwrap(int x, int y) const;
    void setriverjulwrap(int x, int y, int amount);
    
    int riveravewrap(int x, int y) const;
    
    int windwrap(int x, int y) const;
    void setwindwrap(int x, int y, int amount);
    
    int lakesurfacewrap(int x, int y) const;
    void setlakesurfacewrap(int x, int y, int amount);
    
    int truelakewrap(int x, int y) const;
    
    float roughnesswrap(int x, int y) const;
    void setroughnesswrap(int x, int y, float amount);
    
    int mountainridgewrap(int x, int y) const;
    void setmountainridgewrap(int x, int y, int amount);
    
    int mountainheightwrap(int x, int y) const;
    void setmountainheightwrap(int x, int y, int amount);
    
    int tidewrap(int x, int y) const;
    void settidewrap(int x, int y, int amount);
    
    int riftlakesurfacewrap(int x, int y) const;
    void setriftlakesurfacewrap(int x, int y, int amount);
    
    int riftlakebedwrap(int x, int y) const;
    void setriftlakebedwrap(int x, int y, int amount);
    
    int specialwrap(int x, int y) const;
    void setspecialwrap(int x, int y, int amount);
    
    int deltadirwrap(int x, int y) const;
    void setdeltadirwrap(int x, int y, int amount);
    
    int deltajanwrap(int x, int y) const;
    void setdeltajanwrap(int x, int y, int amount);
    
    int deltajulwrap(int x, int y) const;
    void setdeltajulwrap(int x, int y, int amount);
    
    int horsewrap(int x, int y) const;
    void sethorsewrap(int x, int y, int amount);
    
    // Other public functions.
    
    void clear();   // Clears all of the maps.
    void smoothnom(int amount); // Smoothes the no-mountain map to a given amount.
    void smoothextraelev(int amount); // Smoothes the extra elevation map to a given amount.
    void shiftterrain(int offset); // Shifts the physical terrain by a given amount.
    void smoothrainmaps(int amount); // Smoothes the rain maps by a given amount.
    void setmaxriverflow(); // Calculates the largest river flow on the map.

    void saveworld(string filename); // Saves the world.
    void loadworld(string filename); // Loads the world.


private:
    
    // Private variables.
    
    int itswidth;   // width of global map
    int itsheight;  // height of global map
    
    long itsseed;    // seed number of this world
    
    bool itsrotation;    // 1 (true) like the Earth, 0 (false) the other way
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
    
    float itslandshading;
    float itslakeshading;
    float itsseashading; // Amount of shading to do on these areas.
    
    short itsshadingdir=4; // Direction of light source for shading.
    
    short itssnowchange=1; // Kind of snow border.
    
    short itsseaiceappearance=1; // How much sea ice to display.
    
    float itslandmarbling;
    float itslakemarbling;
    float itsseamarbling; // Amount of marbling to do on these areas on the regional map.
    
    int itsminriverflowglobal; // Rivers greater than this will be shown on the global map.
    int itsminriverflowregional; // Rivers greater than this will be shown on the regional map.
    
    int itsnoisewidth;
    int itsnoiseheight; // Dimensions of the noise map
    
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
    
    int itshighlight1;
    int itshighlight2;
    int itshighlight3;
    
    int maxtempmap[ARRAYWIDTH][ARRAYHEIGHT];
    int mintempmap[ARRAYWIDTH][ARRAYHEIGHT];
    short climatemap[ARRAYWIDTH][ARRAYHEIGHT];
    int summerrainmap[ARRAYWIDTH][ARRAYHEIGHT];
    int winterrainmap[ARRAYWIDTH][ARRAYHEIGHT];
    int wintermountainrainmap[ARRAYWIDTH][ARRAYHEIGHT];
    int summermountainrainmap[ARRAYWIDTH][ARRAYHEIGHT];
    short wintermountainraindirmap[ARRAYWIDTH][ARRAYHEIGHT];
    short summermountainraindirmap[ARRAYWIDTH][ARRAYHEIGHT];
    int seaicemap[ARRAYWIDTH][ARRAYHEIGHT];
    int rivermapdir[ARRAYWIDTH][ARRAYHEIGHT];
    int rivermapjan[ARRAYWIDTH][ARRAYHEIGHT];
    int rivermapjul[ARRAYWIDTH][ARRAYHEIGHT];
    int windmap[ARRAYWIDTH][ARRAYHEIGHT];
    int lakemap[ARRAYWIDTH][ARRAYHEIGHT];
    float roughnessmap[ARRAYWIDTH][ARRAYHEIGHT];
    int mountainridges[ARRAYWIDTH][ARRAYHEIGHT];
    int mountainheights[ARRAYWIDTH][ARRAYHEIGHT];
    int mapnom[ARRAYWIDTH][ARRAYHEIGHT];
    int tidalmap[ARRAYWIDTH][ARRAYHEIGHT];
    int riftlakemapsurface[ARRAYWIDTH][ARRAYHEIGHT];
    int riftlakemapbed[ARRAYWIDTH][ARRAYHEIGHT];
    bool lakestartmap[ARRAYWIDTH][ARRAYHEIGHT];
    int specials[ARRAYWIDTH][ARRAYHEIGHT];
    int extraelevmap[ARRAYWIDTH][ARRAYHEIGHT];
    int deltamapdir[ARRAYWIDTH][ARRAYHEIGHT];
    int deltamapjan[ARRAYWIDTH][ARRAYHEIGHT];
    int deltamapjul[ARRAYWIDTH][ARRAYHEIGHT];
    bool islandmap[ARRAYWIDTH][ARRAYHEIGHT];
    bool mountainislandmap[ARRAYWIDTH][ARRAYHEIGHT];
    int itsnoisemap[NOISEWIDTH][NOISEHEIGHT];
    int oceanridgemap[ARRAYWIDTH][ARRAYHEIGHT];
    int oceanridgeheightmap[ARRAYWIDTH][ARRAYHEIGHT];
    int oceanriftmap[ARRAYWIDTH][ARRAYHEIGHT];
    int oceanridgeoffsetmap[ARRAYWIDTH][ARRAYHEIGHT];
    int oceanridgeanglemap[ARRAYWIDTH][ARRAYHEIGHT];
    int volcanomap[ARRAYWIDTH][ARRAYHEIGHT];
    bool stratomap[ARRAYWIDTH][ARRAYHEIGHT];
    bool noshademap[ARRAYWIDTH][ARRAYHEIGHT];
    int testmap[ARRAYWIDTH][ARRAYHEIGHT];
    
    int horselats[ARRAYWIDTH][6];

    // reused temporary state
    string line_for_file_read;
    
    // Private functions.
    
    int wrapx(int x) const;
    int clipy(int y) const;
    void smooth(int arr[][ARRAYHEIGHT], int amount, bool vary, bool avoidmountains);
    void smoothoverland(int arr[][ARRAYHEIGHT], int amount, bool vary);

    template<typename T> void shift(T arr[][ARRAYHEIGHT], int offset);
    template<typename T> void writevariable(ofstream& outfile, T val);
    template<typename T> void writedata(ofstream& outfile, T const arr[ARRAYWIDTH][ARRAYHEIGHT]);
    template<typename T> void readvariable(ifstream& infile, T &val);
    template<typename T> void readdata(ifstream& infile, T arr[ARRAYWIDTH][ARRAYHEIGHT]);
};

inline long planet::seed() const {return itsseed;}
inline void planet::setseed(long amount) {itsseed=amount;}

inline int planet::width() const {return itswidth;}
inline void planet::setwidth(int amount) {itswidth=amount;}

inline int planet::height() const {return itsheight;}
inline void planet::setheight(int amount) {itsheight=amount;}

inline bool planet::rotation() const {return itsrotation;}
inline void planet::setrotation(bool amount) {itsrotation=amount;}

inline float planet::riverfactor() const {return itsriverfactor;}
inline void planet::setriverfactor(float amount) {itsriverfactor=amount;}

inline int planet::riverlandreduce() const {return itsriverlandreduce;}
inline void planet::setriverlandreduce(int amount) {itsriverlandreduce=amount;}

inline int planet::estuarylimit() const {return itsestuarylimit;}
inline void planet::setestuarylimit(int amount) {itsestuarylimit=amount;}

inline int planet::glacialtemp() const {return itsglacialtemp;}
inline void planet::setglacialtemp(int amount) {itsglacialtemp=amount;}

inline int planet::glaciertemp() const {return itsglaciertemp;}
inline void planet::setglaciertemp(int amount) {itsglaciertemp=amount;}

inline float planet::mountainreduce() const {return itsmountainreduce;}
inline void planet::setmountainreduce(float amount) {itsmountainreduce=amount;}

inline int planet::climatenumber() const {return itsclimateno;}
inline void planet::setclimatenumber(int amount) {itsclimateno=amount;}

inline int planet::maxelevation() const {return itsmaxheight;}
inline void planet::setmaxelevation(int amount) {itsmaxheight=amount;}

inline int planet::sealevel() const {return itssealevel;}
inline void planet::setsealevel(int amount) {itssealevel=amount;}

inline int planet::noisewidth() const {return itsnoisewidth;};
inline void planet::setnoisewidth(int amount) {itsnoisewidth=amount;};

inline int planet::noiseheight() const {return itsnoiseheight;};
inline void planet::setnoiseheight(int amount) {itsnoiseheight=amount;};

inline float planet::landshading() const {return itslandshading;};
inline void planet::setlandshading(float amount) {itslandshading=amount;};

inline float planet::lakeshading() const {return itslakeshading;};
inline void planet::setlakeshading(float amount) {itslakeshading=amount;};

inline float planet::seashading() const {return itsseashading;};
inline void planet::setseashading(float amount) {itsseashading=amount;};

inline short planet::shadingdir() const {return itsshadingdir;};
inline void planet::setshadingdir(short amount) {itsshadingdir=amount;};

inline short planet::snowchange() const {return itssnowchange;};
inline void planet::setsnowchange(short amount) {itssnowchange=amount;};

inline short planet::seaiceappearance() const {return itsseaiceappearance;};
inline void planet::setseaiceappearance(short amount) {itsseaiceappearance=amount;};

inline float planet::landmarbling() const {return itslandmarbling;};
inline void planet::setlandmarbling(float amount) {itslandmarbling=amount;};

inline float planet::lakemarbling() const {return itslakemarbling;};
inline void planet::setlakemarbling(float amount) {itslakemarbling=amount;};

inline float planet::seamarbling() const {return itsseamarbling;};
inline void planet::setseamarbling(float amount) {itsseamarbling=amount;};

inline int planet::minriverflowglobal() const {return itsminriverflowglobal;};
inline void planet::setminriverflowglobal(int amount) {itsminriverflowglobal=amount;};

inline int planet::minriverflowregional() const {return itsminriverflowregional;};
inline void planet::setminriverflowregional(int amount) {itsminriverflowregional=amount;};

inline int planet::maxriverflow() const {return itsmaxriverflow;}

inline int planet::map(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;
    
    int thisvolcano=abs(volcanomap[x][y]);
    
    if (mapnom[x][y]<=itssealevel)
        return mapnom[x][y]+oceanridgeheightmap[x][y]+thisvolcano;
    
    if (thisvolcano>mountainheights[x][y])
        return mapnom[x][y]+thisvolcano+extraelevmap[x][y];
    else
        return mapnom[x][y]+mountainheights[x][y]+extraelevmap[x][y];
    
    return 0;
}

inline int planet::nom(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;
    
    return mapnom[x][y];
}

inline void planet::setnom(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    mapnom[x][y]=amount;
}

inline int planet::oceanridges(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;
    
    return oceanridgemap[x][y];
}

inline void planet::setoceanridges(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    oceanridgemap[x][y]=amount;
}

inline int planet::oceanridgeheights(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return oceanridgeheightmap[x][y];
}

inline void planet::setoceanridgeheights(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    oceanridgeheightmap[x][y]=amount;
}

inline int planet::oceanrifts(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return oceanriftmap[x][y];
}

inline void planet::setoceanrifts(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    oceanriftmap[x][y]=amount;
}

inline int planet::oceanridgeoffset(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return oceanridgeoffsetmap[x][y];
}

inline void planet::setoceanridgeoffset(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    oceanridgeoffsetmap[x][y]=amount;
}

inline int planet::oceanridgeangle(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return oceanridgeanglemap[x][y];
}

inline void planet::setoceanridgeangle(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    oceanridgeanglemap[x][y]=amount;
}

inline int planet::volcano(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return volcanomap[x][y];
}

inline void planet::setvolcano(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    volcanomap[x][y]=amount;
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

    stratomap[x][y]=amount;
}

inline int planet::extraelev(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return extraelevmap[x][y];
}

inline void planet::setextraelev(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    extraelevmap[x][y]=amount;
}

inline int planet::maxtemp(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return maxtempmap[x][y];
}

inline void planet::setmaxtemp(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    maxtempmap[x][y]=amount;
}

inline int planet::mintemp(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return mintempmap[x][y];
}

inline void planet::setmintemp(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    mintempmap[x][y]=amount;
}

inline int planet::avetemp(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return ((mintempmap[x][y]+maxtempmap[x][y])/2);
}

inline int planet::summerrain(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return summerrainmap[x][y];
}

inline void planet::setsummerrain(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    summerrainmap[x][y]=amount;
}

inline int planet::winterrain(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return winterrainmap[x][y];
}

inline void planet::setwinterrain(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    winterrainmap[x][y]=amount;
}

inline int planet::averain(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return ((winterrainmap[x][y]+summerrainmap[x][y])/2);
}

inline int planet::wintermountainrain(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return wintermountainrainmap[x][y];
}

inline void planet::setwintermountainrain(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    wintermountainrainmap[x][y]=amount;
}

inline int planet::summermountainrain(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return summermountainrainmap[x][y];
}

inline void planet::setsummermountainrain(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    summermountainrainmap[x][y]=amount;
}

inline short planet::wintermountainraindir(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return wintermountainraindirmap[x][y];
}

inline void planet::setwintermountainraindir(int x, int y, short amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    wintermountainraindirmap[x][y]=amount;
}

inline short planet::summermountainraindir(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return summermountainraindirmap[x][y];
}

inline void planet::setsummermountainraindir(int x, int y, short amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    summermountainraindirmap[x][y]=amount;
}

inline short planet::climate(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return climatemap[x][y];
}

inline void planet::setclimate(int x, int y, short amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    climatemap[x][y]=amount;
}

inline int planet::seaice(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return seaicemap[x][y];
}

inline void planet::setseaice(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    seaicemap[x][y]=amount;
}

inline int planet::riverdir(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return rivermapdir[x][y];
}

inline void planet::setriverdir(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    rivermapdir[x][y]=amount;
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

    rivermapjan[x][y]=amount;
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

    rivermapjul[x][y]=amount;
}

inline int planet::riveraveflow(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return (rivermapjan[x][y]+rivermapjul[x][y])/2;
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

    windmap[x][y]=amount;
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

    lakemap[x][y]=amount;
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

    roughnessmap[x][y]=amount;
}

inline int planet::mountainridge(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return mountainridges[x][y];
}

inline void planet::setmountainridge(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    mountainridges[x][y]=amount;
}

inline int planet::mountainheight(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return mountainheights[x][y];
}

inline void planet::setmountainheight(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    mountainheights[x][y]=amount;
}

inline int planet::tide(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return tidalmap[x][y];
}

inline void planet::settide(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    tidalmap[x][y]=amount;
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

    riftlakemapsurface[x][y]=amount;
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

    riftlakemapbed[x][y]=amount;
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

    specials[x][y]=amount;
}

inline int planet::deltadir(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return deltamapdir[x][y];
}

inline void planet::setdeltadir(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    deltamapdir[x][y]=amount;
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

    deltamapjan[x][y]=amount;
}

inline int planet::deltajul(int x, int y) const
{
    return deltamapjul[x][y];
}

inline void planet::setdeltajul(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    deltamapjul[x][y]=amount;
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

    lakestartmap[x][y]=amount;
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

    islandmap[x][y]=amount;
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

    mountainislandmap[x][y]=amount;
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

    noshademap[x][y]=amount;
}

inline int planet::noisemap(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return itsnoisemap[x][y];
}

inline void planet::setnoisemap(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;

    itsnoisemap[x][y]=amount;
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

    horselats[x][y]=amount;
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

    testmap[x][y]=amount;
}

inline bool planet::sea(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;
    
    if (mapnom[x][y]<=itssealevel && lakemap[x][y]==0)
        return 1;
    
    if (volcanomap[x][y]==0)
        return 0;
    
    int thisvolcano=abs(volcanomap[x][y]);
    
    if (mapnom[x][y]+thisvolcano<=itssealevel)
        return 1;
    else
        return 0;
}

inline int planet::jantemp(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;
    
    if (y<itsheight/2)
        return(mintempmap[x][y]);
    else
        return(maxtempmap[x][y]);
}

inline void planet::setjantemp(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;
    
    if (y<itsheight/2)
        mintempmap[x][y]=amount;
    else
        maxtempmap[x][y]=amount;
}

inline int planet::jultemp(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;
    
    if (y<itsheight/2)
        return(maxtempmap[x][y]);
    else
        return(mintempmap[x][y]);
}

inline void planet::setjultemp(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;
    
    if (y<itsheight/2)
        maxtempmap[x][y]=amount;
    else
        mintempmap[x][y]=amount;
}

inline int planet::janrain(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;
    
    if (y<itsheight/2)
        return(winterrainmap[x][y]);
    else
        return(summerrainmap[x][y]);
}

inline void planet::setjanrain(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;
    
    if (y<itsheight/2)
        winterrainmap[x][y]=amount;
    else
        summerrainmap[x][y]=amount;
}

inline int planet::julrain(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;
    
    if (y<itsheight/2)
        return(summerrainmap[x][y]);
    else
        return(winterrainmap[x][y]);
}

inline void planet::setjulrain(int x, int y, int amount)
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return;
    
    if (y<itsheight/2)
        summerrainmap[x][y]=amount;
    else
        winterrainmap[x][y]=amount;
}

inline int planet::truelake(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;
    
    if (lakesurface(x,y)!=0 && special(x,y)<110)
        return (1);
    else
        return(0);
}


#endif /* planet_hpp */
