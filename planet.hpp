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

#define ARRAYWIDTH 512

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

    int edge() const;  // edge length of faces
    void setedge(int amount);

    int type() const;  // planetary type
    void settype(int amount);

    bool rotation() const;   // global rotation
    void setrotation(bool amount);

    float tilt() const; // axial tilt
    void settilt(float amount);

    float eccentricity() const; // orbital eccentricity
    void seteccentricity(float amount);

    int perihelion() const; // perihelion
    void setperihelion(int amount);

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

    int seaicetemp() const; // equatorial temperature
    void setseaicetemp(int amount);

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

    int snowchange() const;
    void setsnowchange(int amount); // 1=abrupt; 2=speckled; 3=gradual

    float normal() const;
    void setnormal(float amount);

    float ambient() const;
    void setambient(float amount);

    float gamma() const;
    void setgamma(float amount);

    float haze() const;
    void sethaze(float amount);

    float specular() const;
    void setspecular(float amount);

    float cloudcover() const;
    void setcloudcover(float amount);

    float bloomdist() const;
    void setbloomdist(float amount);

    float sunglare() const;
    void setsunglare(float amount);

    float sunlens() const;
    void setsunlens(float amount);

    int sunrayno() const;
    void setsunrayno(int amount);

    float sunraystrength() const;
    void setsunraystrength(float amount);

    float starbright() const;
    void setstarbright(float amount);

    float starcolour() const;
    void setstarcolour(float amount);

    float starhaze() const;
    void setstarhaze(float amount);

    float starnebula() const;
    void setstarnebula(float amount);

    int seaiceappearance() const;
    void setseaiceappearance(int amount);

    bool colourcliffs() const;
    void setcolourcliffs(bool amount);

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

    int sea1() const { return itssea1; };
    void setsea1(int amount) { itssea1 = amount; };

    int sea2() const { return itssea2; };
    void setsea2(int amount) { itssea2 = amount; };

    int sea3() const { return itssea3; };
    void setsea3(int amount) { itssea3 = amount; };

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

    int atmos1() const { return itsatmos1; };
    void setatmos1(int amount) { itsatmos1 = amount; };

    int atmos2() const { return itsatmos2; };
    void setatmos2(int amount) { itsatmos2 = amount; };

    int atmos3() const { return itsatmos3; };
    void setatmos3(int amount) { itsatmos3 = amount; };

    int dusk1() const { return itsdusk1; };
    void setdusk1(int amount) { itsdusk1 = amount; };

    int dusk2() const { return itsdusk2; };
    void setdusk2(int amount) { itsdusk2 = amount; };

    int dusk3() const { return itsdusk3; };
    void setdusk3(int amount) { itsdusk3 = amount; };

    int sun1() const { return itssun1; };
    void setsun1(int amount) { itssun1 = amount; };

    int sun2() const { return itssun2; };
    void setsun2(int amount) { itssun2 = amount; };

    int sun3() const { return itssun3; };
    void setsun3(int amount) { itssun3 = amount; };

    int gal1haze1() const { return itsgal1haze1; };
    void setgal1haze1(int amount) { itsgal1haze1 = amount; };

    int gal1haze2() const { return itsgal1haze2; };
    void setgal1haze2(int amount) { itsgal1haze2 = amount; };

    int gal1haze3() const { return itsgal1haze3; };
    void setgal1haze3(int amount) { itsgal1haze3 = amount; };

    int gal2haze1() const { return itsgal2haze1; };
    void setgal2haze1(int amount) { itsgal2haze1 = amount; };

    int gal2haze2() const { return itsgal2haze2; };
    void setgal2haze2(int amount) { itsgal2haze2 = amount; };

    int gal2haze3() const { return itsgal2haze3; };
    void setgal2haze3(int amount) { itsgal2haze3 = amount; };

    int gal1nebula1() const { return itsgal1nebula1; };
    void setgal1nebula1(int amount) { itsgal1nebula1 = amount; };

    int gal1nebula2() const { return itsgal1nebula2; };
    void setgal1nebula2(int amount) { itsgal1nebula2 = amount; };

    int gal1nebula3() const { return itsgal1nebula3; };
    void setgal1nebula3(int amount) { itsgal1nebula3 = amount; };

    int gal2nebula1() const { return itsgal2nebula1; };
    void setgal2nebula1(int amount) { itsgal2nebula1 = amount; };

    int gal2nebula2() const { return itsgal2nebula2; };
    void setgal2nebula2(int amount) { itsgal2nebula2 = amount; };

    int gal2nebula3() const { return itsgal2nebula3; };
    void setgal2nebula3(int amount) { itsgal2nebula3 = amount; };

    int bloom1() const { return itsbloom1; };
    void setbloom1(int amount) { itsbloom1 = amount; };

    int bloom2() const { return itsbloom2; };
    void setbloom2(int amount) { itsbloom2 = amount; };

    int bloom3() const { return itsbloom3; };
    void setbloom3(int amount) { itsbloom3 = amount; };

    // These accessor functions are for location-specific information.
    // They don't check that x and y are valid coordinates.

    int map(int face, int x, int y) const; // total terrain elevation

    int nom(int face, int x, int y) const; // no-mountains terrain elevation
    void setnom(int face, int x, int y, int amount);

    int noise(int face, int x, int y) const; // The noise map - used for adding the terraced decoration to parts of the regional map.
    void setnoise(int face, int x, int y, int amount);

    bool sea(int face, int x, int y) const; // whether this is sea or not

    bool outline(int face, int x, int y) const;   // whether this is coast, next to sea

    bool coast(int face, int x, int y) const; // whether this is coast, next to land

    void longitude(int face, int x, int& degrees, int& minutes, int& seconds, bool& negative) const;   // returns the longitude of this point

    void latitude(int face, int y, int& degrees, int& minutes, int& seconds, bool& negative) const;   // returns the latitude of this point

    int reverselatitude(int lat) const; // returns the y coordinate of a latitude

    int nomridge(int face, int x, int y) const; // no-mountains, with undersea ridge terrain elevation

    int oceanridges(int face, int x, int y) const; // undersea ridges
    void setoceanridges(int face, int x, int y, int amount);

    int oceanridgeheights(int face, int x, int y) const; // undersea ridges
    void setoceanridgeheights(int face, int x, int y, int amount);

    int oceanrifts(int face, int x, int y) const; // undersea rifts
    void setoceanrifts(int face, int x, int y, int amount);

    int oceanridgeoffset(int face, int x, int y) const; // offset map for the ridges
    void setoceanridgeoffset(int face, int x, int y, int amount);

    int oceanridgeangle(int face, int x, int y) const; // the angle perpendicular to the line of the ridge
    void setoceanridgeangle(int face, int x, int y, int amount);

    int volcano(int face, int x, int y) const; // isolated peaks (negative value means it's extinct)
    void setvolcano(int face, int x, int y, int amount);

    bool strato(int face, int x, int y) const; // stratovolcanoes
    void setstrato(int face, int x, int y, bool amount);

    int clouds(int face, int x, int y) const; // clouds
    void setclouds(int face, int x, int y, int amount);

    int extraelev(int face, int x, int y) const;   // extra terrain elevation
    void setextraelev(int face, int x, int y, int amount);

    int maxtemp(int face, int x, int y) const; // maximum temperature
    void setmaxtemp(int face, int x, int y, int amount);

    int mintemp(int face, int x, int y) const; // minimum temperature
    void setmintemp(int face, int x, int y, int amount);

    int jantemp(int face, int x, int y) const; // January temperature
    void setjantemp(int face, int x, int y, int amount);

    int jultemp(int face, int x, int y) const; // July temperature
    void setjultemp(int face, int x, int y, int amount);

    int equinoxtemp(int face, int x, int y, vector<float>& latitude) const; // Temperature at the equinox

    int monthtemp(int face, int x, int y, int thismonth, vector<float>& latitude) const; // Temperature at a given month

    void monthlytemps(int face, int x, int y, float arr[12], vector<float>& latitude) const; // All the monthly temperatures for this point

    int avetemp(int face, int x, int y) const; // average temperature

    int summerrain(int face, int x, int y) const;  // summer precipitation
    void setsummerrain(int face, int x, int y, int amount);

    int winterrain(int face, int x, int y) const;  // winter precipitation
    void setwinterrain(int face, int x, int y, int amount);

    int janrain(int face, int x, int y) const; // January precipitation
    void setjanrain(int face, int x, int y, int amount);

    int julrain(int face, int x, int y) const; // July precipitation
    void setjulrain(int face, int x, int y, int amount);

    void monthlyrain(int face, int x, int y, float temp[12], float rain[12]) const; // All the monthly precipitation for this poitn

    int aprrain(int face, int x, int y) const; // April precipitation
    int octrain(int face, int x, int y) const; // October precipitation

    int averain(int face, int x, int y) const; // average precipitation

    int janmountainrain(int face, int x, int y) const;  // jan precipitation on mountains
    void setjanmountainrain(int face, int x, int y, int amount);

    int julmountainrain(int face, int x, int y) const;  // jul precipitation on mountains
    void setjulmountainrain(int face, int x, int y, int amount);

    int wintermountainrain(int face, int x, int y) const;  // winter precipitation on mountains
    void setwintermountainrain(int face, int x, int y, int amount);

    int summermountainrain(int face, int x, int y) const;  // winter precipitation on mountains
    void setsummermountainrain(int face, int x, int y, int amount);

    int janmountainraindir(int face, int x, int y) const;  // direction of jan precipitation on mountains
    void setjanmountainraindir(int face, int x, int y, int amount);

    int julmountainraindir(int face, int x, int y) const;  // direction of jul precipitation on mountains
    void setjulmountainraindir(int face, int x, int y, int amount);

    int wintermountainraindir(int face, int x, int y) const;  // direction of winter precipitation on mountains
    void setwintermountainraindir(int face, int x, int y, int amount);

    int summermountainraindir(int face, int x, int y) const;  // direction of winter precipitation on mountains
    void setsummermountainraindir(int face, int x, int y, int amount);

    int climate(int face, int x, int y) const; // climate type
    void setclimate(int face, int x, int y, int amount);

    int seaice(int face, int x, int y) const;  // sea ice (roughly)

    int seatempreduce(int face, int x, int y) const;  // temperature reduction for working out sea ice
    void setseatempreduce(int face, int x, int y, int amount);

    int riverdir(int face, int x, int y) const;    // river flow direction
    void setriverdir(int face, int x, int y, int amount);

    int riverjan(int face, int x, int y) const;    // January river flow volume
    void setriverjan(int face, int x, int y, int amount);

    int riverjul(int face, int x, int y) const;    // July river flow volume
    void setriverjul(int face, int x, int y, int amount);

    int riveraveflow(int face, int x, int y) const; // average river flow
    void monthlyflow(int face, int x, int y, float temp[12], float rain[12], int flow[12]) const; // river flow month by month

    int wind(int face, int x, int y) const;    // wind direction
    void setwind(int face, int x, int y, int amount);

    int winddir(int face, int x, int y) const; // easterly = -1, westerly = 1, none = 0

    int lakesurface(int face, int x, int y) const; // lake surface elevation
    void setlakesurface(int face, int x, int y, int amount);

    int truelake(int face, int x, int y) const; // whether this is a true lake

    float roughness(int face, int x, int y) const;   // roughness
    void setroughness(int face, int x, int y, float amount);

    int mountainridge(int face, int x, int y) const;   // mountain ridge directions
    void setmountainridge(int face, int x, int y, int amount);

    int mountainheight(int face, int x, int y) const;  // mountain elevation
    void setmountainheight(int face, int x, int y, int amount);

    int craterrim(int face, int x, int y) const;  // crater rims
    void setcraterrim(int face, int x, int y, int amount);

    int cratercentre(int face, int x, int y) const;  // crater centre heights
    void setcratercentre(int face, int x, int y, int amount);

    int craterface(int n) const; // crater centre face
    void setcraterface(int n, int amount);

    int craterx(int n) const; // crater centre x
    void setcraterx(int n, int amount);

    int cratery(int n) const; // crater centre y
    void setcratery(int n, int amount);

    int craterelev(int n) const; // crater peak elevation
    void setcraterelev(int n, int amount);

    int craterradius(int n) const; // crater radius
    void setcraterradius(int n, int amount);

    int tide(int face, int x, int y) const;    // tidal strength
    void settide(int face, int x, int y, int amount);

    int riftlakesurface(int face, int x, int y) const; // rift lake surface elevation
    void setriftlakesurface(int face, int x, int y, int amount);

    int riftlakebed(int face, int x, int y) const; // rift lake bed elevation
    void setriftlakebed(int face, int x, int y, int amount);

    int special(int face, int x, int y) const; // special features. 100: salt lake. 110: salt pan. 120: dunes. 130: fresh wetlands. 131: brackish wetlands. 132: salt wetlands.
    void setspecial(int face, int x, int y, int amount);

    int deltadir(int face, int x, int y) const;    // delta branch flow direction (reversed)
    int deltadir_no_bounds_check(int face, int x, int y) const { return deltamapdir[face][x][y]; }
    void setdeltadir(int face, int x, int y, int amount);

    int deltajan(int face, int x, int y) const;    // January delta branch flow volume
    void setdeltajan(int face, int x, int y, int amount);

    int deltajul(int face, int x, int y) const;    // July delta branch flow volume
    void setdeltajul(int face, int x, int y, int amount);

    bool lakestart(int face, int x, int y) const; //  Whether a rift lake starts here
    void setlakestart(int face, int x, int y, int amount);

    bool island(int face, int x, int y) const; // Whether this is a one-tile island
    void setisland(int face, int x, int y, bool amount);

    bool mountainisland(int face, int x, int y) const; // Whether this is an island that's basically a mountain in the sea (if it is, the elevation of what would be the sea bed here is recorded)
    void setmountainisland(int face, int x, int y, bool amount);

    bool noshade(int face, int x, int y) const; // Whether no shading should be applied here on the global map.
    void setnoshade(int face, int x, int y, bool amount);

    int horse(int x, int y) const;   // horse latitudes
    void sethorse(int x, int y, int amount);

    float sunlat(int n) const; // Sun latitudes
    void setsunlat(int n, float amount);

    float sunlong(int n) const; // Sun longitudes
    void setsunlong(int n, float amount);

    float sundist(int n) const; // Sun distances
    void setsundist(int n, float amount);

    int test(int face, int x, int y) const;    // Test array.
    void settest(int face, int x, int y, int amount);

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

    int itsedge; // Length/width of each face of the cube. Default is 512.

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
    int itsseaicetemp; // Sea temperatures of this value or lower will have sea ice
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

    short itssnowchange = 1; // Kind of snow border.
    float itsnormal = 0.5f; // How much effect the normal map has/
    float itsgamma = 0.5f; // Gamma correction.
    float itsambient = 0.0f; // Ambient lighting strength.
    float itshaze = 0.2f; // Thickness of the atmosphere.
    float itsspecular = 0.5f; // Shininess of the planet.
    float itscloudcover = 0.5f; // Extent of the cloud cover.

    float itsbloomdist = 1.0f; // How extensive the bloom effect on the sun is.
    float itssunglare = 0.2f; // How strong the horizontal glare effect is.
    float itssunlens = 0.5f; // How strong the lens effect is.
    short itssunrayno = 8; // Radial symmetry of the sun's rays.
    float itssunraystrength = 0.2f; // Strength of the sun ray effect.
    float itsstarbright = 1.0f;
    float itsstarcolour = 1.0f;
    float itsstarhaze = 1.0f;
    float itsstarnebula = 1.0f;

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

    int itssea1;
    int itssea2;
    int itssea3;

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

    int itsatmos1;
    int itsatmos2;
    int itsatmos3;

    int itsdusk1;
    int itsdusk2;
    int itsdusk3;

    int itssun1;
    int itssun2;
    int itssun3;

    int itsgal1haze1;
    int itsgal1haze2;
    int itsgal1haze3;

    int itsgal2haze1;
    int itsgal2haze2;
    int itsgal2haze3;

    int itsgal1nebula1;
    int itsgal1nebula2;
    int itsgal1nebula3;

    int itsgal2nebula1;
    int itsgal2nebula2;
    int itsgal2nebula3;

    int itsbloom1;
    int itsbloom2;
    int itsbloom3;

    // Maps are stored in the following format.
    // [faceno][x][y]
    // - where faceno is 0-5. Faces are numbered: 0 = front; 1 = right; 2 = back; 3 = left; 4 = top; 5 = bottom (bearing mind 0-3 are largely arbitrary!)

    short jantempmap[6][ARRAYWIDTH][ARRAYWIDTH];
    short jultempmap[6][ARRAYWIDTH][ARRAYWIDTH];
    short climatemap[6][ARRAYWIDTH][ARRAYWIDTH];
    short janrainmap[6][ARRAYWIDTH][ARRAYWIDTH];
    short julrainmap[6][ARRAYWIDTH][ARRAYWIDTH];
    short janmountainrainmap[6][ARRAYWIDTH][ARRAYWIDTH];
    short julmountainrainmap[6][ARRAYWIDTH][ARRAYWIDTH];
    short janmountainraindirmap[6][ARRAYWIDTH][ARRAYWIDTH];
    short julmountainraindirmap[6][ARRAYWIDTH][ARRAYWIDTH];
    short seatempreducemap[6][ARRAYWIDTH][ARRAYWIDTH]; // Amount to reduce sea temperatures when working out sea ice
    short rivermapdir[6][ARRAYWIDTH][ARRAYWIDTH];
    int rivermapjan[6][ARRAYWIDTH][ARRAYWIDTH];
    int rivermapjul[6][ARRAYWIDTH][ARRAYWIDTH];
    int windmap[6][ARRAYWIDTH][ARRAYWIDTH];
    int lakemap[6][ARRAYWIDTH][ARRAYWIDTH];
    float roughnessmap[6][ARRAYWIDTH][ARRAYWIDTH];
    short mountainridges[6][ARRAYWIDTH][ARRAYWIDTH];
    short mountainheights[6][ARRAYWIDTH][ARRAYWIDTH];
    short craterrims[6][ARRAYWIDTH][ARRAYWIDTH];
    short cratercentres[6][ARRAYWIDTH][ARRAYWIDTH];
    short mapnom[6][ARRAYWIDTH][ARRAYWIDTH];
    short tidalmap[6][ARRAYWIDTH][ARRAYWIDTH];
    int riftlakemapsurface[6][ARRAYWIDTH][ARRAYWIDTH];
    int riftlakemapbed[6][ARRAYWIDTH][ARRAYWIDTH];
    bool lakestartmap[6][ARRAYWIDTH][ARRAYWIDTH];
    short specials[6][ARRAYWIDTH][ARRAYWIDTH];
    short extraelevmap[6][ARRAYWIDTH][ARRAYWIDTH];
    short deltamapdir[6][ARRAYWIDTH][ARRAYWIDTH];
    int deltamapjan[6][ARRAYWIDTH][ARRAYWIDTH];
    int deltamapjul[6][ARRAYWIDTH][ARRAYWIDTH];
    bool islandmap[6][ARRAYWIDTH][ARRAYWIDTH];
    bool mountainislandmap[6][ARRAYWIDTH][ARRAYWIDTH];
    short noisemap[6][ARRAYWIDTH][ARRAYWIDTH];
    short oceanridgemap[6][ARRAYWIDTH][ARRAYWIDTH];
    short oceanridgeheightmap[6][ARRAYWIDTH][ARRAYWIDTH];
    short oceanriftmap[6][ARRAYWIDTH][ARRAYWIDTH];
    short oceanridgeoffsetmap[6][ARRAYWIDTH][ARRAYWIDTH];
    short oceanridgeanglemap[6][ARRAYWIDTH][ARRAYWIDTH];
    short volcanomap[6][ARRAYWIDTH][ARRAYWIDTH];
    bool stratomap[6][ARRAYWIDTH][ARRAYWIDTH];
    bool noshademap[6][ARRAYWIDTH][ARRAYWIDTH];
    short cloudsmap[6][ARRAYWIDTH][ARRAYWIDTH];
    int testmap[6][ARRAYWIDTH][ARRAYWIDTH];

    int horselats[ARRAYWIDTH][6];

    fiveintegers cratercentreslist[MAXCRATERS]; // This duplicates the information in the cratercentres array, but it tells us the order in which to draw the craters, and their radius, which is also important.

    float sunlats[12]; // Latitudes of the sun for each month.
    float sunlongs[12]; // Longitudes of the sun for each month.
    float sundists[12]; // Distances of the sun for each month (1.0 = mean distance).

    // reused temporary state
    string line_for_file_read;

    // Private functions.

    int wrapx(int x) const;
    int clipy(int y) const;
    void smooth(int arr[][ARRAYWIDTH], int amount, bool vary, bool avoidmountains);
    void smooth(int short[][ARRAYWIDTH], int amount, bool vary, bool avoidmountains);
    void smoothoverland(int arr[][ARRAYWIDTH][ARRAYWIDTH], int amount, bool vary);
    void smoothoverland(int short[][ARRAYWIDTH][ARRAYWIDTH], int amount, bool vary);

    template<typename T> void shift(T arr[][ARRAYWIDTH], int offset);
    template<typename T> void writevariable(ofstream& outfile, T val);
    template<typename T> void writedata(ofstream& outfile, T const arr[ARRAYWIDTH][ARRAYWIDTH]);
    template<typename T> void readvariable(ifstream& infile, T& val);
    template<typename T> void readdata(ifstream& infile, T arr[ARRAYWIDTH][ARRAYWIDTH]);
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

inline int planet::edge() const { return itsedge; }
inline void planet::setedge(int amount) { itsedge = amount; }

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

inline int planet::seaicetemp() const { return itsseaicetemp; }
inline void planet::setseaicetemp(int amount) { itsseaicetemp = amount; }

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

inline int planet::snowchange() const { return (int)itssnowchange; };
inline void planet::setsnowchange(int amount) { itssnowchange = (short)amount; };

inline float planet::normal() const { return itsnormal; };
inline void planet::setnormal(float amount) { itsnormal = amount; };

inline float planet::ambient() const { return itsambient; };
inline void planet::setambient(float amount) { itsambient = amount; };

inline float planet::gamma() const { return itsgamma; };
inline void planet::setgamma(float amount) { itsgamma = amount; };

inline float planet::haze() const { return itshaze; };
inline void planet::sethaze(float amount) { itshaze = amount; };

inline float planet::specular() const { return itsspecular; };
inline void planet::setspecular(float amount) { itsspecular = amount; };

inline float planet::cloudcover() const { return itscloudcover; };
inline void planet::setcloudcover(float amount) { itscloudcover = amount; };

inline float planet::bloomdist() const { return itsbloomdist; };
inline void planet::setbloomdist(float amount) { itsbloomdist = amount; };

inline float planet::sunglare() const { return itssunglare; };
inline void planet::setsunglare(float amount) { itssunglare = amount; };

inline float planet::sunlens() const { return itssunlens; };
inline void planet::setsunlens(float amount) { itssunlens = amount; };

inline int planet::sunrayno() const { return (int)itssunrayno; };
inline void planet::setsunrayno(int amount) { itssunrayno = (short)amount; };

inline float planet::sunraystrength() const { return itssunraystrength; };
inline void planet::setsunraystrength(float amount) { itssunraystrength = amount; };

inline float planet::starbright() const { return itsstarbright; };
inline void planet::setstarbright(float amount) { itsstarbright = amount; };

inline float planet::starcolour() const { return itsstarcolour; };
inline void planet::setstarcolour(float amount) { itsstarcolour = amount; };

inline float planet::starhaze() const { return itsstarhaze; };
inline void planet::setstarhaze(float amount) { itsstarhaze = amount; };

inline float planet::starnebula() const { return itsstarnebula; };
inline void planet::setstarnebula(float amount) { itsstarnebula = amount; };

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

inline int planet::map(int face, int x, int y) const
{
    int thisvolcano = abs(volcanomap[face][x][y]);

    if (mapnom[face][x][y] <= itssealevel)
        return mapnom[face][x][y] + oceanridgeheightmap[face][x][y] + thisvolcano;

    if (thisvolcano > mountainheights[face][x][y])
        return mapnom[face][x][y] + thisvolcano + extraelevmap[face][x][y] + craterrims[face][x][y] + cratercentres[face][x][y];
    else
        return mapnom[face][x][y] + mountainheights[face][x][y] + extraelevmap[face][x][y] + craterrims[face][x][y] + cratercentres[face][x][y];

    return 0;
}

inline int planet::nom(int face, int x, int y) const
{
    return (int)mapnom[face][x][y];
}

inline void planet::setnom(int face, int x, int y, int amount)
{
    mapnom[face][x][y] = (short)amount;
}

inline int planet::oceanridges(int face, int x, int y) const
{
    return (int)oceanridgemap[face][x][y];
}

inline void planet::setoceanridges(int face, int x, int y, int amount)
{
    oceanridgemap[face][x][y] = (short)amount;
}

inline int planet::oceanridgeheights(int face, int x, int y) const
{
    return (int)oceanridgeheightmap[face][x][y];
}

inline void planet::setoceanridgeheights(int face, int x, int y, int amount)
{
    oceanridgeheightmap[face][x][y] = (short)amount;
}

inline int planet::oceanrifts(int face, int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;

    return (int)oceanriftmap[face][x][y];
}

inline void planet::setoceanrifts(int face, int x, int y, int amount)
{
    oceanriftmap[face][x][y] = (short)amount;
}

inline int planet::oceanridgeoffset(int face, int x, int y) const
{
    return (int)oceanridgeoffsetmap[face][x][y];
}

inline void planet::setoceanridgeoffset(int face, int x, int y, int amount)
{
    oceanridgeoffsetmap[face][x][y] = (short)amount;
}

inline int planet::oceanridgeangle(int face, int x, int y) const
{
    return (int)oceanridgeanglemap[face][x][y];
}

inline void planet::setoceanridgeangle(int face, int x, int y, int amount)
{
    oceanridgeanglemap[face][x][y] = (short)amount;
}

inline int planet::volcano(int face, int x, int y) const
{
    return (int)volcanomap[face][x][y];
}

inline void planet::setvolcano(int face, int x, int y, int amount)
{
    volcanomap[face][x][y] = (short)amount;
}

inline bool planet::strato(int face, int x, int y) const
{
    return stratomap[face][x][y];
}

inline void planet::setstrato(int face, int x, int y, bool amount)
{
    stratomap[face][x][y] = amount;
}

inline int planet::clouds(int face, int x, int y) const
{
    return (int)cloudsmap[face][x][y];
}

inline void planet::setclouds(int face, int x, int y, int amount)
{
    cloudsmap[face][x][y] = (short)amount;
}

inline int planet::extraelev(int face, int x, int y) const
{
    return (int)extraelevmap[face][x][y];
}

inline void planet::setextraelev(int face, int x, int y, int amount)
{
    extraelevmap[face][x][y] = (short)amount;
}

inline int planet::jantemp(int face, int x, int y) const
{
    return (int)jantempmap[face][x][y];
}

inline void planet::setjantemp(int face, int x, int y, int amount)
{
    jantempmap[face][x][y] = (short)amount;
}

inline int planet::jultemp(int face, int x, int y) const
{
    return (int)jultempmap[face][x][y];
}

inline void planet::setjultemp(int face, int x, int y, int amount)
{
    jultempmap[face][x][y] = (short)amount;
}

inline int planet::equinoxtemp(int face, int x, int y, vector<float>& latitude) const
{
    float summertemp = (float)jultempmap[face][x][y];
    float wintertemp = (float)jantempmap[face][x][y];

    if (itsperihelion == 1)
    {
        summertemp = (float)jantempmap[face][x][y];
        wintertemp = (float)jultempmap[face][x][y];
    }

    float winterstrength = 0.5f + itseccentricity * 0.5f; // The higher the eccentricity, the shorter the "summer".
    float summerstrength = 1.0f - winterstrength;

    //float thistemp = summertemp * summerstrength + wintertemp * winterstrength;

    float thistemp = summertemp * 0.5f + wintertemp * 0.5f;

    // Now adjust for four-seasonality (if the obliquity is high)

    //if (face < 4)
    //{
        float fourseason = itstilt * 0.294592f - 2.45428f;

        /*
        float lat = (float)y;

        if (y > itsedge / 2.0f)
            lat = (float)(itsedge - y);

        float fourseasonstrength = (float)lat / ((float)itsedge / 2.0f); // This measures the strength of the four-season cycle as determined by proximity to the equator.
        */

        float reversestrength = latitude[face * itsedge * itsedge + x * itsedge + y] / 90.0f;

        reversestrength = reversestrength * reversestrength; // Do a sort of reverse-square on it

        float fourseasonstrength = 1.0f - reversestrength;

        float thistempdiff = (fourseason * fourseasonstrength) / 2.0f;

        thistemp = thistemp + thistempdiff;
    //}

    return (int)thistemp;
}

inline int planet::monthtemp(int face, int x, int y, int thismonth, vector<float>& latitude) const
{
    if (thismonth == 0)
        return (int)jantempmap[face][x][y];

    if (thismonth == 6)
        return(int)jultempmap[face][x][y];

    thismonth = thismonth - 1; // Because temperature lags behind the sun's position.

    float maxdist = sundists[5];
    float mindist = sundists[11];

    if (maxdist < mindist)
    {
        float a = maxdist;
        maxdist = mindist;
        mindist = a;
    }

    float eqdist = (maxdist + mindist) / 2.0f;
    float eqtemp = (float)equinoxtemp(face, x, y, latitude);

    float thisdist = sundists[thismonth];

    if (thisdist == eqdist) // If we're exactly at the equinox!
        return (int)eqtemp;

    float maxdistdiff = maxdist - eqdist;

    float neartemp = (float)jantempmap[face][x][y];
    float fartemp = (float)jultempmap[face][x][y];

    if (itsperihelion == (int)1)
    {
        neartemp = (float)jultempmap[face][x][y];
        fartemp = (float)jantempmap[face][x][y];
    }

    if (thisdist < eqdist) // If the planet is closer than average to the sun
    {
        float maxtempdiff = eqtemp - neartemp;
        
        float maxdiff = eqdist - mindist;

        float thisdistdiff = (thisdist - mindist) / maxdiff;

        float thistemp = neartemp + maxtempdiff * thisdistdiff;

        return (int)thistemp;
    }
    else // If the planet is further than average from the sun
    {
        float maxtempdiff = fartemp - eqtemp;

        float maxdiff = maxdist - eqdist;

        float thisdistdiff = (thisdist - eqdist) / maxdiff;

        float thistemp = eqtemp + maxtempdiff * thisdistdiff;

        return (int)thistemp;
    }

    return 0;
}

inline void planet::monthlytemps(int face, int x, int y, float arr[12], vector<float>& latitude) const
{
    float maxdist = sundists[5];
    float mindist = sundists[11];

    if (maxdist < mindist)
    {
        float a = maxdist;
        maxdist = mindist;
        mindist = a;
    }

    float eqdist = (maxdist + mindist) / 2.0f;
    float eqtemp = (float)equinoxtemp(face, x, y, latitude);

    arr[0] = jantempmap[face][x][y];
    arr[6]= jultempmap[face][x][y];

    for (int n = 1; n < 12; n++)
    {
        if (n != 6)
        {
            int thismonth = n - 1; // Because temperature lags behind the sun's position.

            float thisdist = sundists[thismonth];

            if (thisdist == eqdist) // If we're exactly at the equinox!
                arr[n] = eqtemp;
            else
            {
                float maxdistdiff = maxdist - eqdist;

                float neartemp = jantempmap[face][x][y];
                float fartemp = jultempmap[face][x][y];

                if (itsperihelion == (int)1)
                {
                    neartemp = jultempmap[face][x][y];
                    fartemp = jantempmap[face][x][y];
                }

                if (thisdist < eqdist) // If the planet is closer than average to the sun
                {
                    float maxtempdiff = eqtemp - neartemp;

                    float maxdiff = eqdist - mindist;

                    float thisdistdiff = (thisdist - mindist) / maxdiff;

                    float thistemp = neartemp + maxtempdiff * thisdistdiff;

                    arr[n] = thistemp;
                }
                else // If the planet is further than average from the sun
                {
                    float maxtempdiff = fartemp - eqtemp;

                    float maxdiff = maxdist - eqdist;

                    float thisdistdiff = (thisdist - eqdist) / maxdiff;

                    float thistemp = eqtemp + maxtempdiff * thisdistdiff;

                    arr[n] = thistemp;
                }
            }
        }
    }
}


inline int planet::avetemp(int face, int x, int y) const
{
    return (int)((jantempmap[face][x][y] + jultempmap[face][x][y]) / 2);
}

inline int planet::janrain(int face, int x, int y) const
{
    return (int)janrainmap[face][x][y];
}

inline void planet::setjanrain(int face, int x, int y, int amount)
{
    janrainmap[face][x][y] = (short)amount;
}

inline int planet::julrain(int face, int x, int y) const
{
    return (int)julrainmap[face][x][y];
}

inline void planet::setjulrain(int face, int x, int y, int amount)
{
    julrainmap[face][x][y] = (short)amount;
}

inline void planet::monthlyrain(int face, int x, int y, float temp[12], float rain[12]) const
{
    float jantemp = temp[0];
    float jultemp = temp[6];

    float maxtempdiff = jultemp - jantemp;

    float janrain = (float)janrainmap[face][x][y];
    float julrain = (float)julrainmap[face][x][y];

    float maxrain = julrain;
    float minrain = janrain;

    if (janrain > julrain)
    {
        maxrain = janrain;
        minrain = julrain;
    }

    float maxraindiff = julrain - janrain;

    rain[0] = janrain;
    rain[6] = julrain;

    float maxlatdiff = sunlats[6] - sunlats[0];

    // First, calculate a simple rain variation based on the months. We will later blend this with the more complex calculation.

    float simplerain[12];
    simplerain[0] = rain[0];
    simplerain[6] = rain[6];

    /*
    float simplefactor = maxraindiff / 300.0f;
    simplefactor = simplefactor * simplefactor;
    simplefactor = 1.0f - simplefactor;

    if (simplefactor < 0.0f)
        simplefactor = 0.0f;

    //simplefactor = simplefactor * 0.3f;

    simplefactor = 1.0f;

    if (simplefactor > 0.0f)
    {
        */
    for (int n = 1; n < 12; n++)
    {
        if (n != 6)
        {
            if (sunlats[0] != sunlats[6]) // Do this by sun latitude.
            {
                float thislatdiff = sunlats[6] - sunlats[n];

                float factor = thislatdiff / maxlatdiff;
                float raintoremove = maxraindiff * factor;

                simplerain[n] = julrain - raintoremove;

            }
            else // Just do it by time of year.
            {
                float janstrength = 1.0f;
                float julstrength = 1.0f;

                if (n == 1 || n == 11)
                {
                    janstrength = 5.0f;
                    julstrength = 1.0f;
                }

                if (n == 2 || n == 10)
                {
                    janstrength = 4.0f;
                    julstrength = 2.0f;
                }

                if (n == 3 || n == 9)
                {
                    janstrength = 3.0f;
                    julstrength = 3.0f;
                }

                if (n == 4 || n == 8)
                {
                    janstrength = 2.0f;
                    julstrength = 4.0f;
                }

                if (n == 5 || n == 7)
                {
                    janstrength = 1.0f;
                    julstrength = 5.0f;
                }

                rain[n] = (janrain * janstrength + julrain * julstrength) / 6.0f;
            }
        }
    }
    //}

    for (int n = 1; n < 12; n++)
    {
        if (n != 6)
        {
            /*
            if (maxtempdiff != 0.0f) // Do this by temperature if possible.
            {
                float thistempdiff = jultemp - temp[n];

                float factor = thistempdiff / maxtempdiff;
                float raintoremove = maxraindiff * factor;

                rain[n] = julrain - raintoremove;

                if (simplefactor > 0.0f)
                    rain[n] = rain[n] * (1.0f - simplefactor) + simplerain[n] * simplefactor;
            }
            else
                */
            rain[n] = simplerain[n];

        }
    }

    if ((jantemp > jultemp && janrain > julrain) || (jultemp > jantemp && julrain > janrain)) // Monsoon!
    {
        float monstrength = 1.0f; // The less seasonal variation in rainfall, the less pronounced the monsoon effect.
      
        if (janrain > julrain)

            monstrength = monstrength - julrain / janrain;
        else

            monstrength = monstrength - janrain / julrain;        

        if (monstrength > 0.0f)
        {
            float cutoff = maxrain * 0.85f; // Any rainfall below this will be reduced.

            float origrain[12];

            for (int n = 0; n < 12; n++)
                origrain[n] = rain[n];

            for (int n = 1; n < 12; n++)
            {
                if (n != 6)
                {
                    int prevmonth = n - 1;

                    if (origrain[n] < cutoff)
                    {
                        if (origrain[prevmonth] < origrain[n]) // If this month's rainfall is greater than last month's, reduce it more.
                            rain[n] = (origrain[n] + minrain * 2.0f) / 3.0f;
                        else // If it's after the main monsoon month, the rain reduces more slowly.
                            rain[n] = (origrain[n] * 2.0f + minrain) / 3.0f;
                    }
                    else
                    {
                        if (origrain[prevmonth] < origrain[n]) // If this month's rainfall is greater than last month's, reduce it more.
                            rain[n] = (origrain[n] * 3.0f + minrain) / 4.0f;
                        else // If it's after the main monsoon month, the rain reduces more slowly.
                            rain[n] = (origrain[n] * 5.0f + minrain) / 6.0f;
                    }
                }
            }
        }
    }

    for (int n = 0; n < 12; n++)
    {
        if (rain[n] < minrain)
            rain[n] = minrain;
    }
}

inline int planet::aprrain(int face, int x, int y) const
{
    float janfactor = 0.5f;
    float julfactor = 0.5f;

    float janrain = (float)janrainmap[face][x][y];
    float julrain = (float)julrainmap[face][x][y];

    if (jultempmap[face][x][y] > jantempmap[face][x][y] && julrainmap[face][x][y] > janrainmap[face][x][y]) // Monsoon in July
    {
        float monsoonfactor = 1.0f - janrain / julrain;

        janfactor = monsoonfactor * 0.9f; // The stronger the July monsoon, the more the April rain should be like January (low).
        julfactor = 1.0f - janfactor;
    }

    if (jultempmap[face][x][y] < jantempmap[face][x][y] && julrainmap[face][x][y] < janrainmap[face][x][y]) // Monsoon in January
    {
        float monsoonfactor = 1.0f - julrain / janrain;

        janfactor = monsoonfactor * 0.7f; // The stronger the January monsoon, the more the April rain should be like January (high) (though not *too* close too it!).
        julfactor = 1.0f - janfactor;
    }

    float total = janrain * janfactor + julrain * julfactor;

    return (int)total;
}

inline int planet::octrain(int face, int x, int y) const
{
    float janfactor = 0.5f;
    float julfactor = 0.5f;

    float janrain = (float)janrainmap[face][x][y];
    float julrain = (float)julrainmap[face][x][y];

    if (jultempmap[face][x][y] > jantempmap[face][x][y] && julrainmap[face][x][y] > janrainmap[face][x][y]) // Monsoon in July
    {
        float monsoonfactor = 1.0f - janrain / julrain;

        julfactor = monsoonfactor * 0.7f; // The stronger the July monsoon, the more the October rain should be like July (high) (though not *too* close too it!).
        janfactor = 1.0f - julfactor;
    }

    if (jultempmap[face][x][y] < jantempmap[face][x][y] && julrainmap[face][x][y] < janrainmap[face][x][y]) // Monsoon in January
    {
        float monsoonfactor = 1.0f - julrain / janrain;

        julfactor = monsoonfactor * 0.9f; // The stronger the January monsoon, the more the April rain should be like July (low).
        janfactor = 1.0f - julfactor;
    }

    float total = janrain * janfactor + julrain * julfactor;

    return (int)total;
}

inline int planet::averain(int face, int x, int y) const
{
    return (int)((janrainmap[face][x][y] + julrainmap[face][x][y]) / 2);
}

inline int planet::janmountainrain(int face, int x, int y) const
{
    return (int)janmountainrainmap[face][x][y];
}

inline void planet::setjanmountainrain(int face, int x, int y, int amount)
{
    janmountainrainmap[face][x][y] = (short)amount;
}

inline int planet::julmountainrain(int face, int x, int y) const
{
    return (int)julmountainrainmap[face][x][y];
}

inline void planet::setjulmountainrain(int face, int x, int y, int amount)
{
    julmountainrainmap[face][x][y] = (short)amount;
}

inline int planet::wintermountainrain(int face, int x, int y) const
{
    if (jantempmap[face][x][y] < jultempmap[face][x][y])
        return (short)janmountainrainmap[face][x][y];
    else
        return (short)julmountainrainmap[face][x][y];
}

inline void planet::setwintermountainrain(int face, int x, int y, int amount)
{
    if (jantempmap[face][x][y] < jultempmap[face][x][y])
        janmountainrainmap[face][x][y] = (short)amount;
    else
        julmountainrainmap[face][x][y] = (short)amount;
}

inline int planet::summermountainrain(int face, int x, int y) const
{
    if (jantempmap[face][x][y] >= jultempmap[face][x][y])
        return (int)janmountainrainmap[face][x][y];
    else
        return (int)julmountainrainmap[face][x][y];
}

inline void planet::setsummermountainrain(int face, int x, int y, int amount)
{
    if (jantempmap[face][x][y] >= jultempmap[face][x][y])
        janmountainrainmap[face][x][y] = (short)amount;
    else
        julmountainrainmap[face][x][y] = (short)amount;
}

inline int planet::janmountainraindir(int face, int x, int y) const
{
    return (int)janmountainraindirmap[face][x][y];
}

inline void planet::setjanmountainraindir(int face, int x, int y, int amount)
{
    janmountainraindirmap[face][x][y] = (short)amount;
}

inline int planet::julmountainraindir(int face, int x, int y) const
{
    return (int)julmountainraindirmap[face][x][y];
}

inline void planet::setjulmountainraindir(int face, int x, int y, int amount)
{
    julmountainraindirmap[face][x][y] = (short)amount;
}

inline int planet::wintermountainraindir(int face, int x, int y) const
{
    if (jantempmap[face][x][y] < jultempmap[face][x][y])
        return (int)janmountainraindirmap[face][x][y];
    else
        return (int)julmountainraindirmap[face][x][y];
}

inline int planet::summermountainraindir(int face, int x, int y) const
{
    if (jantempmap[face][x][y] >= jultempmap[face][x][y])
        return (int)janmountainraindirmap[face][x][y];
    else
        return (int)julmountainraindirmap[face][x][y];
}

inline void planet::setwintermountainraindir(int face, int x, int y, int amount)
{
    if (jantempmap[face][x][y] < jultempmap[face][x][y])
        janmountainraindirmap[face][x][y] = (short)amount;
    else
        julmountainraindirmap[face][x][y] = (short)amount;
}

inline void planet::setsummermountainraindir(int face, int x, int y, int amount)
{
    if (jantempmap[face][x][y] >= jultempmap[face][x][y])
        janmountainraindirmap[face][x][y] = (short)amount;
    else
        julmountainraindirmap[face][x][y] = (short)amount;
}

inline int planet::climate(int face, int x, int y) const
{
    return (int)climatemap[face][x][y];
}

inline void planet::setclimate(int face, int x, int y, int amount)
{
    climatemap[face][x][y] = (short)amount;
}

inline int planet::seaice(int face, int x, int y) const
{
    if (jantempmap[face][x][y] - seatempreducemap[face][x][y] <= itsseaicetemp && jultempmap[face][x][y] - seatempreducemap[face][x][y] <= itsseaicetemp)
        return 2;

    if (jantempmap[face][x][y] - seatempreducemap[face][x][y] <= itsseaicetemp || jultempmap[face][x][y] - seatempreducemap[face][x][y] <= itsseaicetemp)
        return 1;

    return 0;
}

inline int planet::seatempreduce(int face, int x, int y) const
{
    return (int)seatempreducemap[face][x][y];
}

inline void planet::setseatempreduce(int face, int x, int y, int amount)
{
    seatempreducemap[face][x][y] = (short)amount;
}

inline int planet::riverdir(int face, int x, int y) const
{
    return (short)rivermapdir[face][x][y];
}

inline void planet::setriverdir(int face, int x, int y, int amount)
{
    rivermapdir[face][x][y] = (short)amount;
}

inline int planet::riverjan(int face, int x, int y) const
{
    return rivermapjan[face][x][y];
}

inline void planet::setriverjan(int face, int x, int y, int amount)
{
    rivermapjan[face][x][y] = amount;
}

inline int planet::riverjul(int face, int x, int y) const
{
    return rivermapjul[face][x][y];
}

inline void planet::setriverjul(int face, int x, int y, int amount)
{
    rivermapjul[face][x][y] = amount;
}

inline int planet::riveraveflow(int face, int x, int y) const
{
    return (rivermapjan[face][x][y] + rivermapjul[face][x][y]) / 2;
}

inline void planet::monthlyflow(int face, int x, int y, float temp[12], float rain[12], int flow[12]) const
{
    float janrain = (float)rain[0];
    float julrain = (float)rain[6];

    float maxraindiff = julrain - janrain;

    float janflow = (float)rivermapjan[face][x][y];
    float julflow = (float)rivermapjul[face][x][y];

    float maxflow = julflow;
    float minflow = janflow;

    if (janflow > julflow)
    {
        maxflow = janflow;
        minflow = julflow;
    }

    float maxflowdiff = julflow - janflow;

    float freezefactor = 1.2f; // Multiply the flow by this for every preceding month of freezing temperatures.

    flow[0] = (int)janflow;
    flow[6] = (int)julflow;

    float maxlatdiff = sunlats[6] - sunlats[0];

    // First, calculate a simple flow variation based on the months. We will later blend this with the more complex calculation.

    float simpleflow[12];
    simpleflow[0] = (float)flow[0];
    simpleflow[6] = (float)flow[6];

    float simplefactor = 1.0f - (maxflowdiff / 200.0f);

    if (simplefactor < 0.0f)
        simplefactor = 0.0f;

    simplefactor = simplefactor * 0.3f;

    if (simplefactor > 0.0f)
    {
        for (int n = 1; n < 12; n++)
        {
            if (n != 6)
            {
                if (sunlats[0] != sunlats[6]) // Do this by sun latitude.
                {
                    float thislatdiff = sunlats[6] - sunlats[n];

                    float factor = thislatdiff / maxlatdiff;
                    float flowtoremove = maxflowdiff * factor;

                    simpleflow[n] = julflow - flowtoremove;

                }
                else // Just do it by time of year.
                {
                    float janstrength = 1.0f;
                    float julstrength = 1.0f;

                    if (n == 1 || n == 11)
                    {
                        janstrength = 5.0f;
                        julstrength = 1.0f;
                    }

                    if (n == 2 || n == 10)
                    {
                        janstrength = 4.0f;
                        julstrength = 2.0f;
                    }

                    if (n == 3 || n == 9)
                    {
                        janstrength = 3.0f;
                        julstrength = 3.0f;
                    }

                    if (n == 4 || n == 8)
                    {
                        janstrength = 2.0f;
                        julstrength = 4.0f;
                    }

                    if (n == 5 || n == 7)
                    {
                        janstrength = 1.0f;
                        julstrength = 5.0f;
                    }

                    flow[n] = (int)((janflow * janstrength + julflow * julstrength) / 6.0f);
                }
            }
        }
    }

    for (int n = 1; n < 12; n++)
    {
        if (n != 6)
        {
            if (temp[n] > 0.0f)
            {
                if (maxflowdiff != 0.0f) // Do this by rainfall if possible.
                {
                    float thisraindiff = julrain - rain[n];

                    float factor = thisraindiff / maxraindiff;
                    float flowtoremove = maxflowdiff * factor;

                    flow[n] = (int)(julflow - flowtoremove);

                    if (simplefactor > 0.0f)
                        flow[n] = (int)((float)flow[n] * (1.0f - simplefactor) + simpleflow[n] * simplefactor);
                }
                else
                    flow[n] = (int)simpleflow[n];
            }
            else
                flow[n] = (int)minflow;
        }
    }

    /*
    for (int n = 0; n < 12; n++)
    {
        if (temp[n] <= 0.0f)
        {
            int next = n + 1;

            if (next == 12)
                next = 0;

            if (temp[next] > 0.0f)
                flow[next] = (int)((float)flow[next] * freezefactor);
        }
    }
    */

    for (int n = 0; n < 12; n++) // For each non-freezing month after a freezing month, increase the flow to simulate floods during the thaw
    {
        if (temp[n] > 0.0f)
        {
            for (int m = n - 1; m > n - 5; m--)
            {
                int mm = m;

                if (mm < 0)
                    mm = mm + 12;

                if (temp[mm] > 0.0f)
                    m = n - 5;
                else
                    flow[n] = (int)((float)flow[n] * freezefactor);

            }
        }
    }

    /*
    for (int n = 1; n < 12; n++)
    {
        if (n != 6)
        {
            if (temp[n] > 0.0f)
            {
                float mult = (rain[n] - janrain);

                if (maxraindiff == 0.0f)
                    mult = 0.0f;
                else
                    mult = mult / maxflowdiff;

                flow[n] = (int)(janflow + maxflowdiff * mult);

                for (int m = 1; m < 4; m++) // If the temperature was previously freezing, increase the flow to simulate floods during the thaw
                {
                    int nn = n - m;

                    if (nn < 0)
                        nn = nn + 12;

                    if (temp[nn] > 0.0f)
                        m = 4;
                    else
                        flow[n] = (int)((float)flow[n] * freezefactor);
                }
            }
            else // If it's below freezing, the flow is reduced.
                flow[n] = (int)minflow;
        }
    }
    */

    for (int n = 0; n < 12; n++)
    {
        if (flow[n] < (int)minflow)
            flow[n] = (int)minflow;

        if (flow[n] > (int)maxflow)
            flow[n] = (int)maxflow;
    }
}

inline int planet::wind(int face, int x, int y) const
{
    return windmap[face][x][y];
}

inline void planet::setwind(int face, int x, int y, int amount)
{
    windmap[face][x][y] = amount;
}

inline int planet::winddir(int face, int x, int y) const
{
    if (windmap[face][x][y] > 0 && windmap[face][x][y] < 50)
        return 1;

    if (windmap[face][x][y] < 0)
        return -1;
    
    return 0;
}

inline int planet::lakesurface(int face, int x, int y) const
{
    return lakemap[face][x][y];
}

inline void planet::setlakesurface(int face, int x, int y, int amount)
{
    lakemap[face][x][y] = amount;
}

inline float planet::roughness(int face, int x, int y) const
{
    return roughnessmap[face][x][y];
}

inline void planet::setroughness(int face, int x, int y, float amount)
{
    roughnessmap[face][x][y] = amount;
}

inline int planet::mountainridge(int face, int x, int y) const
{
    return (int)mountainridges[face][x][y];
}

inline void planet::setmountainridge(int face, int x, int y, int amount)
{
    mountainridges[face][x][y] = (short)amount;
}

inline int planet::mountainheight(int face, int x, int y) const
{
    return (int)mountainheights[face][x][y];
}

inline void planet::setmountainheight(int face, int x, int y, int amount)
{
    mountainheights[face][x][y] = (short)amount;
}

inline int planet::craterrim(int face, int x, int y) const
{
    return (int)craterrims[face][x][y];
}

inline void planet::setcraterrim(int face, int x, int y, int amount)
{
    craterrims[face][x][y] = (short)amount;
}

inline int planet::cratercentre(int face, int x, int y) const
{
    return (int)cratercentres[face][x][y];
}

inline void planet::setcratercentre(int face, int x, int y, int amount)
{
    cratercentres[face][x][y] = (short)amount;
}

inline int planet::craterface(int n) const
{
    if (n < 0 || n >= MAXCRATERS)
        return 0;

    return (int)cratercentreslist[n].v;
}

inline void planet::setcraterface(int n, int amount)
{
    if (n < 0 || n >= MAXCRATERS)
        return;

    cratercentreslist[n].v = short(amount);
}

inline int planet::craterx(int n) const
{
    if (n < 0 || n >= MAXCRATERS)
        return 0;

    return (int)cratercentreslist[n].x;
}

inline void planet::setcraterx(int n, int amount)
{
    if (n < 0 || n >= MAXCRATERS)
        return;

    cratercentreslist[n].x = short(amount);
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
    if (n < 0 || n >= MAXCRATERS)
        return 0;

    return (int)cratercentreslist[n].z;
}

inline void planet::setcraterradius(int n, int amount)
{
    if (n < 0 || n >= MAXCRATERS)
        return;

    cratercentreslist[n].z = short(amount);
}

inline int planet::tide(int face, int x, int y) const
{
    return (int)tidalmap[face][x][y];
}

inline void planet::settide(int face, int x, int y, int amount)
{
    tidalmap[face][x][y] = (short)amount;
}

inline int planet::riftlakesurface(int face, int x, int y) const
{
    return riftlakemapsurface[face][x][y];
}

inline void planet::setriftlakesurface(int face, int x, int y, int amount)
{
    riftlakemapsurface[face][x][y] = amount;
}

inline int planet::riftlakebed(int face, int x, int y) const
{
    return riftlakemapbed[face][x][y];
}

inline void planet::setriftlakebed(int face, int x, int y, int amount)
{
    riftlakemapbed[face][x][y] = amount;
}

inline int planet::special(int face, int x, int y) const
{
    return specials[face][x][y];
}

inline void planet::setspecial(int face, int x, int y, int amount)
{
    specials[face][x][y] = (short)amount;
}

inline int planet::deltadir(int face, int x, int y) const
{
    return (int)deltamapdir[face][x][y];
}

inline void planet::setdeltadir(int face, int x, int y, int amount)
{
    deltamapdir[face][x][y] = (short)amount;
}

inline int planet::deltajan(int face, int x, int y) const
{
    return deltamapjan[face][x][y];
}

inline void planet::setdeltajan(int face, int x, int y, int amount)
{
    deltamapjan[face][x][y] = amount;
}

inline int planet::deltajul(int face, int x, int y) const
{
    return deltamapjul[face][x][y];
}

inline void planet::setdeltajul(int face, int x, int y, int amount)
{
    deltamapjul[face][x][y] = amount;
}

inline bool planet::lakestart(int face, int x, int y) const
{
    return lakestartmap[face][x][y];
}

inline void planet::setlakestart(int face, int x, int y, int amount)
{
    lakestartmap[face][x][y] = amount;
}

inline bool planet::island(int face, int x, int y) const
{
    return islandmap[face][x][y];
}

inline void planet::setisland(int face, int x, int y, bool amount)
{
    islandmap[face][x][y] = amount;
}

inline bool planet::mountainisland(int face, int x, int y) const
{
    return mountainislandmap[face][x][y];
}

inline void planet::setmountainisland(int face, int x, int y, bool amount)
{
    mountainislandmap[face][x][y] = amount;
}

inline bool planet::noshade(int face, int x, int y) const
{
    return noshademap[face][x][y];
}

inline void planet::setnoshade(int face, int x, int y, bool amount)
{
    noshademap[face][x][y] = amount;
}

inline int planet::noise(int face, int x, int y) const
{
    return (int)noisemap[face][x][y];
}

inline void planet::setnoise(int face, int x, int y, int amount)
{
    noisemap[face][x][y] = (short)amount;
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

inline float planet::sunlat(int n) const
{
    if (n<0 || n>11)
        return 0;

    return sunlats[n];
}

inline void planet::setsunlat(int n, float amount)
{
    if (n<0 || n>11)
        return;

    sunlats[n] = amount;
}

inline float planet::sunlong(int n) const
{
    if (n < 0 || n>11)
        return 0;

    return sunlongs[n];
}

inline void planet::setsunlong(int n, float amount)
{
    if (n < 0 || n>11)
        return;

    sunlongs[n] = amount;
}

inline float planet::sundist(int n) const
{
    if (n < 0 || n>11)
        return 0;

    return sundists[n];
}

inline void planet::setsundist(int n, float amount)
{
    if (n < 0 || n>11)
        return;

    sundists[n] = amount;
}


inline int planet::test(int face, int x, int y) const
{
    return testmap[face][x][y];
}

inline void planet::settest(int face, int x, int y, int amount)
{
    testmap[face][x][y] = amount;
}

inline bool planet::sea(int face, int x, int y) const
{
    if (mapnom[face][x][y] <= itssealevel && lakemap[face][x][y] == 0)
        return 1;

    if (volcanomap[face][x][y] == 0)
        return 0;

    int thisvolcano = abs(volcanomap[face][x][y]);

    if (mapnom[face][x][y] + thisvolcano <= itssealevel)
        return 1;
    else
        return 0;
}

inline int planet::mintemp(int face, int x, int y) const
{
    if (jantempmap[face][x][y] < jultempmap[face][x][y])
        return (int)jantempmap[face][x][y];
    else
        return (int)jultempmap[face][x][y];
}

inline void planet::setmintemp(int face, int x, int y, int amount)
{
    if (jantempmap[face][x][y] < jultempmap[face][x][y])
        jantempmap[face][x][y] = (short)amount;
    else
        jultempmap[face][x][y] = (short)amount;
}

inline int planet::maxtemp(int face, int x, int y) const
{
    if (jantempmap[face][x][y] > jultempmap[face][x][y])
        return (int)jantempmap[face][x][y];
    else
        return (int)jultempmap[face][x][y];
}

inline void planet::setmaxtemp(int face, int x, int y, int amount)
{
    if (jantempmap[face][x][y] > jultempmap[face][x][y])
        jantempmap[face][x][y] = (short)amount;
    else
        jultempmap[face][x][y] = (short)amount;
}

inline int planet::winterrain(int face, int x, int y) const
{
    if (jantempmap[face][x][y] < jultempmap[face][x][y])
        return (int)janrainmap[face][x][y];
    else
        return (int)julrainmap[face][x][y];
}

inline void planet::setwinterrain(int face, int x, int y, int amount)
{
    if (jantempmap[face][x][y] < jultempmap[face][x][y])
        janrainmap[face][x][y] = (short)amount;
    else
        julrainmap[face][x][y] = (short)amount;
}

inline int planet::summerrain(int face, int x, int y) const
{
    if (jantempmap[face][x][y] >= jultempmap[face][x][y])
        return (int)janrainmap[face][x][y];
    else
        return (int)julrainmap[face][x][y];
}

inline void planet::setsummerrain(int face, int x, int y, int amount)
{
    if (jantempmap[face][x][y] >= jultempmap[face][x][y])
        janrainmap[face][x][y] = (short)amount;
    else
        julrainmap[face][x][y] = (short)amount;
}

inline int planet::truelake(int face, int x, int y) const
{
    if (lakesurface(face, x, y) != 0 && special(face, x, y) < 110)
        return (1);
    else
        return(0);
}

// Now a simple planet class. This is larger scale but much simpler, used just for generating higher-resolution space textures.

class simpleplanet
{
public:

    simpleplanet(); // constructor
    ~simpleplanet();  // destructor

    // accessor functions

    int edge() const;   // global edge
    void setedge(int amount);

    // These accessor functions are for location-specific information.
    // They don't check that x and y are valid coordinates.

    int map(int face, int x, int y) const; // total terrain elevation
    void setmap(int face, int x, int y, int amount);

    int clouds(int face, int x, int y) const; // clouds
    void setclouds(int face, int x, int y, int amount);

    int jantemp(int face, int x, int y) const; // January temperature
    void setjantemp(int face, int x, int y, int amount);

    int jultemp(int face, int x, int y) const; // July temperature
    void setjultemp(int face, int x, int y, int amount);

    int janrain(int face, int x, int y) const; // January precipitation
    void setjanrain(int face, int x, int y, int amount);

    int julrain(int face, int x, int y) const; // July precipitation
    void setjulrain(int face, int x, int y, int amount);

    int seatempreduce(int face, int x, int y) const; // Amount to reduce sea temperature
    void setseatempreduce(int face, int x, int y, int amount);

    int riverjan(int face, int x, int y) const;    // January river flow volume
    void setriverjan(int face, int x, int y, int amount);

    int riverjul(int face, int x, int y) const;    // July river flow volume
    void setriverjul(int face, int x, int y, int amount);

    bool lake(int face, int x, int y) const; // lake surface elevation
    void setlake(int face, int x, int y, bool amount);

    // Other public functions.

    void clear();   // Clears all of the maps.

private:

    // Private variables.

    int itsedge; // Length/width of each face of the cube. Default is 512.

    short jantempmap[6][ARRAYWIDTH * 4][ARRAYWIDTH * 4];
    short jultempmap[6][ARRAYWIDTH * 4][ARRAYWIDTH * 4];
    short janrainmap[6][ARRAYWIDTH * 4][ARRAYWIDTH * 4];
    short julrainmap[6][ARRAYWIDTH * 4][ARRAYWIDTH * 4];
    short seatempreducemap[6][ARRAYWIDTH * 4][ARRAYWIDTH * 4];

    int rivermapjan[6][ARRAYWIDTH * 4][ARRAYWIDTH * 4];
    int rivermapjul[6][ARRAYWIDTH * 4][ARRAYWIDTH * 4];

    bool lakemap[6][ARRAYWIDTH * 4][ARRAYWIDTH * 4];

    short mapnom[6][ARRAYWIDTH * 4][ARRAYWIDTH * 4];

    short cloudsmap[6][ARRAYWIDTH * 4][ARRAYWIDTH * 4];

};

inline int simpleplanet::edge() const { return itsedge; }
inline void simpleplanet::setedge(int amount) { itsedge = amount; }

inline int simpleplanet::map(int face, int x, int y) const
{
    return (int)mapnom[face][x][y];
}

inline void simpleplanet::setmap(int face, int x, int y, int amount)
{
    mapnom[face][x][y] = (short)amount;
}

inline int simpleplanet::clouds(int face, int x, int y) const
{
    return (int)cloudsmap[face] [x] [y] ;
}

inline void simpleplanet::setclouds(int face, int x, int y, int amount)
{
    cloudsmap[face][x][y] = (short)amount;
}

inline int simpleplanet::jantemp(int face, int x, int y) const
{
    return (int)jantempmap[face][x][y];
}

inline void simpleplanet::setjantemp(int face, int x, int y, int amount)
{
    jantempmap[face][x][y] = (short)amount;
}

inline int simpleplanet::jultemp(int face, int x, int y) const
{
    return (int)jultempmap[face][x][y];
}

inline void simpleplanet::setjultemp(int face, int x, int y, int amount)
{
    jultempmap[face][x][y] = (short)amount;
}

inline int simpleplanet::janrain(int face, int x, int y) const
{
    return (int)janrainmap[face][x][y];
}

inline void simpleplanet::setjanrain(int face, int x, int y, int amount)
{
    janrainmap[face][x][y] = (short)amount;
}

inline int simpleplanet::julrain(int face, int x, int y) const
{
    return (int)julrainmap[face][x][y];
}

inline void simpleplanet::setjulrain(int face, int x, int y, int amount)
{
    julrainmap[face][x][y] = (short)amount;
}

inline int simpleplanet::seatempreduce(int face, int x, int y) const
{
    return (int)seatempreducemap[face][x][y];
}

inline void simpleplanet::setseatempreduce(int face, int x, int y, int amount)
{
    seatempreducemap[face][x][y] = (short)amount;
}

inline int simpleplanet::riverjan(int face, int x, int y) const
{
    return (int)rivermapjan[face][x][y];
}

inline void simpleplanet::setriverjan(int face, int x, int y, int amount)
{
    rivermapjan[face][x][y] = (short)amount;
}

inline int simpleplanet::riverjul(int face, int x, int y) const
{
    return (int)rivermapjul[face][x][y];
}

inline void simpleplanet::setriverjul(int face, int x, int y, int amount)
{
    rivermapjul[face][x][y] = (short)amount;
}

inline bool simpleplanet::lake(int face, int x, int y) const
{
    return lakemap[face][x][y];
}

inline void simpleplanet::setlake(int face, int x, int y, bool amount)
{
    lakemap[face][x][y] = amount;
}


#endif /* planet_hpp */