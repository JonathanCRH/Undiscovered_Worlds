//
//  region.hpp
//  Undiscovered Worlds
//
//  Created by Jonathan Hill on 27/10/2019.
//
//  Please see functions.hpp for notes.

#ifndef region_hpp
#define region_hpp

#include <stdio.h>
#include <string>

#define RARRAYWIDTH 1000
#define RARRAYHEIGHT 1000
#define STRIPWIDTH 6

using namespace std;

class region
{
public:
    
    region(); // constructor
    ~region();  // destructor
    
    // Accessor functions
    
    inline int tilewidth() const; // tilewidth
    inline void settilewidth(int amount);
    
    int tileheight() const; // tileheight
    void settileheight(int amount);
    
    int centrex() const; // centrex
    void setcentrex(int amount);
    
    int centrey() const; // centrey
    void setcentrey(int amount);
    
    int leftx() const; // leftx
    void setleftx(int amount);
    
    int lefty() const; // lefty
    void setlefty(int amount);
    
    int rwidth() const; // rwidth
    int rheight() const; // rheight
    
    int rstart() const; // rstart
    void setrstart(int amount);
    
    int regwidthbegin() const; // regwidthbegin
    int regwidthend() const; // regwidthend
    
    int regheightbegin() const; // regheightbegin
    int regheightend() const; // regheightend
    
    int sealevel() const; // sea level
    void setsealevel(int amount);
    
    int pixelmetres() const; // pixelmetres
    void setpixelmetres(int amount);
    
    int gridlines() const; // grid lines
    void setgridlines(int amount);
    
    // Accessor functions for location-specific information
    
    int map(int x, int y) const; // terrain elevation
    void setmap(int , int y, int amount);
    
    int surface(int x, int y) const; // surface elevation (terrain or lake, whichever is higher)
    
    bool sea(int x, int y) const; // whether this is sea or not
    
    bool outline(int x, int y) const;   // whether this is coast, next to sea
    
    int maxtemp(int x, int y) const; // maximum temperature
    void setmaxtemp(int x, int y, int amount);
    
    int mintemp(int x, int y) const; // minimum temperature
    void setmintemp(int x, int y, int amount);
    
    int extramaxtemp(int x, int y) const; // maximum temperature for map drawing
    void setextramaxtemp(int x, int y, int amount);
    
    int extramintemp(int x, int y) const; // minimum temperature for map drawing
    void setextramintemp(int x, int y, int amount);
    
    int avetemp(int x, int y) const; // average temperature
    
    int summerrain(int x, int y) const;  // summer precipitation
    void setsummerrain(int x, int y, int amount);
    
    int winterrain(int x, int y) const;  // winter precipitation
    void setwinterrain(int x, int y, int amount);
    
    int averain(int x, int y) const; // average precipitation
    
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
    
    int fakedir(int x, int y) const;    // fake river flow direction
    void setfakedir(int x, int y, int amount);
    
    int fakejan(int x, int y) const;    // January fake river flow volume
    void setfakejan(int x, int y, int amount);
    
    int fakejul(int x, int y) const;    // July fake river flow volume
    void setfakejul(int x, int y, int amount);
    
    int fakeaveflow(int x, int y) const; // average fake river flow
    
    int wind(int x, int y) const;    // wind direction
    void setwind(int x, int y, int amount);
    
    int lakesurface(int x, int y) const; // lake surface elevation
    void setlakesurface(int x, int y, int amount);
    
    int truelake(int x, int y) const; // whether this is a true lake
    
    int special(int x, int y) const; // special features
    void setspecial(int x, int y, int amount);
    
    int deltadir(int x, int y) const;    // delta branch flow direction (reversed)
    void setdeltadir(int x, int y, int amount);
    
    int deltajan(int x, int y) const;    // January delta branch flow volume
    void setdeltajan(int x, int y, int amount);
    
    int deltajul(int x, int y) const;    // July delta branch flow volume
    void setdeltajul(int x, int y, int amount);
    
    int waterdir(int x, int y, bool delta) const;    // river/delta flow direction
    void setwaterdir(int x, int y, bool delta, int amount);
    
    int waterjan(int x, int y, bool delta) const;    // January river/delta flow volume
    void setwaterjan(int x, int y, bool delta, int amount);
    
    int waterjul(int x, int y, bool delta) const;    // July river/delta flow volume
    void setwaterjul(int x, int y, bool delta, int amount);
    
    float roughness(int x, int y) const; // roughness
    void setroughness(int x, int y, float amount);
    
    bool rivervalley(int x, int y) const; // Whether this is a river valley.
    
    bool mountainsdone(int x, int y) const; // Whether this cell has had mountains applied to it.
    void setmountainsdone(int x, int y, bool amount);
    
    bool volcano(int x, int y) const; // Whether this cell is volcanic.
    void setvolcano(int x, int y, bool amount);
    
    int test(int x, int y) const; // Test array.
    void settest(int x, int y, int amount);
    
    int test2(int x, int y) const; // Test array 2.
    void settest2(int x, int y, int amount);
    
    float testfloat(int x, int y) const; // Float test array.
    void settestfloat(int x, int y, float amount);
    
    // Other public functions
    
    void clear();   // Clears all of the maps.
    
    /*
     void shiftright(); // Shifts everything in the maps a tile to the right.
     
     void copylefttostrip(); // Copies a line of tiles from the left-hand side of the map onto the strip.
     void copyleftfromstrip(); // Copies that line back again onto the map.
     */
    
private:
    
    // Private variables.
    
    int itstilewidth;
    int itstileheight; // Dimensions of the region in tiles.
    
    int itscentrex;
    int itscentrey; // Location of central tile in global coordinates.
    int itsleftx;
    int itslefty; // Location of top-left tile in global coordinates.
    
    int itsrwidth;
    int itsrheight; // Dimensions of the regional map.
    
    int itsrstart; // Amount to ignore on the edge of the regional map for most purposes.
    
    int itsregwidthbegin; // Amount to ignore on the left of the regional map for creating the images.
    int itsregwidthend; // Amount to ignore on the right of the regional map for creating the images.
    int itsregheightbegin; // Amount to ignore on the top of the regional map for creating the images.
    int itsregheightend; // Amount to ignore on the bottom of the regional map for creating the images.
    
    int itssealevel; // Sea level.
    
    int itspixelmetres; // Number of metres each regional map pixel represents.
    
    bool itsgridlines; // Whether to display grid lines on the map.
    
    int rmap[RARRAYWIDTH][RARRAYHEIGHT];
    int rmaxtempmap[RARRAYWIDTH][RARRAYHEIGHT];
    int rmintempmap[RARRAYWIDTH][RARRAYHEIGHT];
    int rextramaxtempmap[RARRAYWIDTH][RARRAYHEIGHT];
    int rextramintempmap[RARRAYWIDTH][RARRAYHEIGHT];
    int rsummerrainmap[RARRAYWIDTH][RARRAYHEIGHT];
    int rwinterrainmap[RARRAYWIDTH][RARRAYHEIGHT];
    short rclimatemap[RARRAYWIDTH][RARRAYHEIGHT];
    int rlakemap[RARRAYWIDTH][RARRAYHEIGHT];
    int rrivermapdir[RARRAYWIDTH][RARRAYHEIGHT];
    int rrivermapjan[RARRAYWIDTH][RARRAYHEIGHT];
    int rrivermapjul[RARRAYWIDTH][RARRAYHEIGHT];
    int rseaicemap[RARRAYWIDTH][RARRAYHEIGHT];
    int rfakeriversdir[RARRAYWIDTH][RARRAYHEIGHT];
    int rfakeriversjan[RARRAYWIDTH][RARRAYHEIGHT];
    int rfakeriversjul[RARRAYWIDTH][RARRAYHEIGHT];
    int rspecials[RARRAYWIDTH][RARRAYHEIGHT]; // 140: glacier.
    int rdeltamapdir[RARRAYWIDTH][RARRAYHEIGHT];
    int rdeltamapjan[RARRAYWIDTH][RARRAYHEIGHT];
    int rdeltamapjul[RARRAYWIDTH][RARRAYHEIGHT];
    float rroughnessmap[RARRAYWIDTH][RARRAYHEIGHT];
    bool rmountainsdone[RARRAYWIDTH][RARRAYHEIGHT];
    bool rvolcanomap[RARRAYWIDTH][RARRAYHEIGHT];
    
    int testmap[RARRAYWIDTH][RARRAYHEIGHT];
    int testmap2[RARRAYWIDTH][RARRAYHEIGHT];
    float testmapfloat[RARRAYWIDTH][RARRAYHEIGHT];
};

inline int region::tilewidth() const{return itstilewidth;}
inline void region::settilewidth(int amount)
{
    itstilewidth=amount;
    itsrwidth=amount*16+33;
    itsregwidthbegin=48; //32;
    itsregwidthend=itsrwidth-48; //32;
}

inline int region::tileheight() const{return itstileheight;}
inline void region::settileheight(int amount)
{
    itstileheight=amount;
    itsrheight=amount*16+33;
    itsregheightbegin=48; //32;
    itsregheightend=itsrheight-48; //32;
}

inline int region::centrex() const{return itscentrex;}
inline void region::setcentrex(int amount)
{
    itscentrex=amount;
    itsleftx=amount-itstilewidth/2;
}

inline int region::centrey() const{return itscentrey;}
inline void region::setcentrey(int amount)
{
    itscentrey=amount;
    itslefty=amount-itstileheight/2;
}

inline int region::leftx() const{return itsleftx;}
inline void region::setleftx(int amount){itsleftx=amount;}

inline int region::lefty() const{return itslefty;}
inline void region::setlefty(int amount){itslefty=amount;}

inline int region::rwidth() const{return itsrwidth;}
inline int region::rheight() const{return itsrheight;}

inline int region::rstart() const{return itsrstart;}
inline void region::setrstart(int amount){itsrstart=amount;}

inline int region::regwidthbegin() const{return itsregwidthbegin;}
inline int region::regwidthend() const{return itsregwidthend;}

inline int region::regheightbegin() const{return itsregheightbegin;}
inline int region::regheightend() const{return itsregheightend;}

inline int region::sealevel() const{return itssealevel;}
inline void region::setsealevel(int amount){itssealevel=amount;}

inline int region::pixelmetres() const{return itspixelmetres;}
inline void region::setpixelmetres(int amount){itspixelmetres=amount;}

inline int region::gridlines() const{return itsgridlines;}
inline void region::setgridlines(int amount){itsgridlines=amount;}

// Accessor functions for location-specific information

inline int region::map(int x, int y) const
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return 0;
    
    return rmap[x][y];
}

inline void region::setmap(int x, int y, int amount)
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return;

    rmap[x][y]=amount;
}

inline int region::surface(int x, int y) const
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return 0;
    
    if (rlakemap[x][y]!=0)
        return rlakemap[x][y];
    else
        return rmap[x][y];
}

inline bool region::sea(int x, int y) const
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return 0;
    
    if (rmap[x][y]<=itssealevel && rlakemap[x][y]==0)
        return 1;
    
    else
        return 0;
}

inline int region::maxtemp(int x, int y) const
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return 0;
    
    return rmaxtempmap[x][y];
}

inline void region::setmaxtemp(int x, int y, int amount)
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return;

    rmaxtempmap[x][y]=amount;
}

inline int region::mintemp(int x, int y) const
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return 0;
    
    return rmintempmap[x][y];
}

inline void region::setmintemp(int x, int y, int amount)
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return;

    rmintempmap[x][y]=amount;
}

inline int region::extramaxtemp(int x, int y) const
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return 0;

    return rextramaxtempmap[x][y];
}

inline void region::setextramaxtemp(int x, int y, int amount)
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return;

    rextramaxtempmap[x][y]=amount;
}

inline int region::extramintemp(int x, int y) const
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return 0;

    return rextramintempmap[x][y];
}

inline void region::setextramintemp(int x, int y, int amount)
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return;

    rextramintempmap[x][y]=amount;
}

inline int region::avetemp(int x, int y) const
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return 0;

    return (rmintempmap[x][y]+rmaxtempmap[x][y])/2;
}

inline int region::summerrain(int x, int y) const
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return 0;

    return rsummerrainmap[x][y];
}

inline void region::setsummerrain(int x, int y, int amount)
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return;

    rsummerrainmap[x][y]=amount;
}

inline int region::winterrain(int x, int y) const
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return 0;

    return rwinterrainmap[x][y];
}

inline void region::setwinterrain(int x, int y, int amount)
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return;

    rwinterrainmap[x][y]=amount;
}

inline int region::averain(int x, int y) const
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return 0;

    return (rwinterrainmap[x][y]+rsummerrainmap[x][y])/2;
}

inline short region::climate(int x, int y) const
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return 0;

    return rclimatemap[x][y];
}

inline void region::setclimate(int x, int y, short amount)
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return;

    rclimatemap[x][y]=amount;
}

inline int region::seaice(int x, int y) const
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return 0;

    return rseaicemap[x][y];
}

inline void region::setseaice(int x, int y, int amount)
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return;

    rseaicemap[x][y]=amount;
}

inline int region::riverdir(int x, int y) const
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return 0;

    return rrivermapdir[x][y];
}

inline void region::setriverdir(int x, int y, int amount)
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return;

    rrivermapdir[x][y]=amount;
}

inline int region::riverjan(int x, int y) const
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return 0;

    return rrivermapjan[x][y];
}

inline void region::setriverjan(int x, int y, int amount)
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return;

    rrivermapjan[x][y]=amount;
}

inline int region::riverjul(int x, int y) const
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return 0;

    return rrivermapjul[x][y];
}

inline void region::setriverjul(int x, int y, int amount)
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return;

    rrivermapjul[x][y]=amount;
}

inline int region::riveraveflow(int x, int y) const
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return 0;

    return (rrivermapjan[x][y]+rrivermapjul[x][y])/2;
}

inline int region::fakedir(int x, int y) const
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return 0;

    return rfakeriversdir[x][y];
}

inline void region::setfakedir(int x, int y, int amount)
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return;

    rfakeriversdir[x][y]=amount;
}

inline int region::fakejan(int x, int y) const
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return 0;

    return rfakeriversjan[x][y];
}

inline void region::setfakejan(int x, int y, int amount)
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return;

    rfakeriversjan[x][y]=amount;
}

inline int region::fakejul(int x, int y) const
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return 0;

    return rfakeriversjul[x][y];
}

inline void region::setfakejul(int x, int y, int amount)
{
    rfakeriversjul[x][y]=amount;
}

inline int region::fakeaveflow(int x, int y) const
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return 0;

    return (rfakeriversjan[x][y]+rfakeriversjul[x][y])/2;
}

inline int region::lakesurface(int x, int y) const
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return 0;

    return rlakemap[x][y];
}

inline void region::setlakesurface(int x, int y, int amount)
{
    rlakemap[x][y]=amount;
}

inline int region::truelake(int x, int y) const
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return 0;

    if (rlakemap[x][y]!=0 && rspecials[x][y]<110)
        return (1);
    else
        return(0);
}

inline int region::special(int x, int y) const
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return 0;

    return rspecials[x][y];
}

inline void region::setspecial(int x, int y, int amount)
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return;

    rspecials[x][y]=amount;
}

inline int region::deltadir(int x, int y) const
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return 0;

    return rdeltamapdir[x][y];
}

inline void region::setdeltadir(int x, int y, int amount)
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return;

    rdeltamapdir[x][y]=amount;
}

inline int region::deltajan(int x, int y) const
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return 0;

    return rdeltamapjan[x][y];
}

inline void region::setdeltajan(int x, int y, int amount)
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return;

    rdeltamapjan[x][y]=amount;
}

inline int region::deltajul(int x, int y) const
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return 0;

    return rdeltamapjul[x][y];
}

inline void region::setdeltajul(int x, int y, int amount)
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return;

    rdeltamapjul[x][y]=amount;
}

inline bool region::mountainsdone(int x, int y) const
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return 0;

    return rmountainsdone[x][y];
} // Whether this cell has had mountains applied to it.

inline void region::setmountainsdone(int x, int y, bool amount)
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return;

    rmountainsdone[x][y]=amount;
};

inline bool region::volcano(int x, int y) const
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return 0;

    return rvolcanomap[x][y];
} // Whether this cell is a volcano.

inline void region::setvolcano(int x, int y, bool amount)
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return;

    rvolcanomap[x][y]=amount;
};

inline int region::waterdir(int x, int y, bool delta) const
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return 0;

    if (delta)
        return rdeltamapdir[x][y];
    
    return rrivermapdir[x][y];
}

inline void region::setwaterdir(int x, int y, bool delta, int amount)
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return;

    if (delta)
        rdeltamapdir[x][y]=amount;
    else
        rrivermapdir[x][y]=amount;
}

inline int region::waterjan(int x, int y, bool delta) const
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return 0;

    if (delta)
        return rdeltamapjan[x][y];
    
    return rrivermapjan[x][y];
}

inline void region::setwaterjan(int x, int y, bool delta, int amount)
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return;

    if (delta)
        rdeltamapjan[x][y]=amount;
    else
        rrivermapjan[x][y]=amount;
}

inline int region::waterjul(int x, int y, bool delta) const
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return 0;

    if (delta)
        return rdeltamapjul[x][y];
    
    return rrivermapjul[x][y];
}

inline void region::setwaterjul(int x, int y, bool delta, int amount)
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return;

    if (delta)
        rdeltamapjul[x][y]=amount;
    else
        rrivermapjul[x][y]=amount;
}

inline float region::roughness(int x, int y) const
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return 0;

    return rroughnessmap[x][y];
}

inline void region::setroughness(int x, int y, float amount)
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return;

    rroughnessmap[x][y]=amount;
}

inline bool region::rivervalley(int x, int y) const
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return 0;

    if (rfakeriversdir[x][y]==-1 && rmap[x][y]>itssealevel && rspecials[x][y]!=140)
        return 1;
    
    return 0;
}

inline int region::test(int x, int y) const
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return 0;

    return testmap[x][y];
} // Test array.

inline void region::settest(int x, int y, int amount)
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return;

    testmap[x][y]=amount;
}

inline int region::test2(int x, int y) const
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return 0;

    return testmap2[x][y];
} // Test array.

inline void region::settest2(int x, int y, int amount)
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return;

    testmap2[x][y]=amount;
}

inline float region::testfloat(int x, int y) const
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return 0;

    return testmapfloat[x][y];
}

inline void region::settestfloat(int x, int y, float amount)
{
    if (x < 0 || x >= RARRAYWIDTH || y < 0 || y >= RARRAYHEIGHT)
        return;

    testmapfloat[x][y]=amount;
}



#endif /* region_hpp */
