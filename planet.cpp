//
//  planet.cpp
//  Undiscovered Worlds
//
//  Created by Jonathan Hill on 22/07/2019.
//
//  Please see functions.hpp for notes.

#include <iostream>
#include <cmath>
#include <fstream>
#include <stdio.h>

#include "planet.hpp"
#include "functions.hpp"

planet::planet() //constructor
{
    itswidth=1024;
    itsheight=512;
}

planet::~planet()
{
}

bool planet::outline(int x, int y) const
{
    if (y<1||y>itsheight-1)
        return 0;
    
    if (sea(x,y)==0)
    {
        if (seawrap(x-1,y)==1)
            return 1;
        
        if (seawrap(x,y-1)==1)
            return 1;
        
        if (seawrap(x+1,y)==1)
            return 1;
        
        if (seawrap(x,y+1)==1)
            return 1;
    }
    
    return 0;
}

bool planet::coast(int x, int y) const
{
    if (y<1||y>itsheight-1 || x<0 || x>itswidth)
        return 0;
    
    if (sea(x,y)==1)
    {
        if (seawrap(x-1,y)==0)
            return 1;
        
        if (seawrap(x,y-1)==0)
            return 1;
        
        if (seawrap(x+1,y)==0)
            return 1;
        
        if (seawrap(x,y+1)==0)
            return 1;
    }
    
    return 0;
}

int planet::latitude(int x, int y) const
{
    if (y<0 || y>itsheight || x<0 || x>itswidth)
        return 0;
    
    float yy=y;
    
    float equator=itsheight/2;
    
    float pixelsperlat=equator/90.0;
    
    float flat=yy/pixelsperlat;
    
    int lat;
    
    if (y<equator)
        flat=90.0-flat;
    
    else
        flat=flat-90.0;
    
    lat=flat;
    
    return lat;
}

int planet::reverselatitude(int lat) const
{
    float flat=90-lat;
    
    float equator=itsheight/2;
    
    float pixelsperlat=equator/90.0;
    
    float yy=flat*pixelsperlat;
    
    int y=yy;
    
    return y;
}

// Now wrap versions of all of those.

int planet::mapwrap(int x, int y) const
{
    x=wrapx(x);
    y=clipy(y);
    
    return mapnom[x][y]+mountainheights[x][y]+extraelevmap[x][y];
}

int planet::nomwrap(int x, int y) const
{
    x=wrapx(x);
    y=clipy(y);
    
    return mapnom[x][y];
}

void planet::setnomwrap(int x, int y, int amount)
{
    x=wrapx(x);
    y=clipy(y);
    
    mapnom[x][y]=amount;
}

int planet::extraelevwrap(int x, int y) const
{
    x=wrapx(x);
    y=clipy(y);
    
    return extraelevmap[x][y];
}

void planet::setextraelevwrap(int x, int y, int amount)
{
    x=wrapx(x);
    y=clipy(y);
    
    extraelevmap[x][y]=amount;
}

int planet::maxtempwrap(int x, int y) const
{
    x=wrapx(x);
    y=clipy(y);
    
    return maxtempmap[x][y];
}

void planet::setmaxtempwrap(int x, int y, int amount)
{
    x=wrapx(x);
    y=clipy(y);
    
    maxtempmap[x][y]=amount;
}

int planet::mintempwrap(int x, int y) const
{
    x=wrapx(x);
    y=clipy(y);
    
    return mintempmap[x][y];
}

void planet::setmintempwrap(int x, int y, int amount)
{
    x=wrapx(x);
    y=clipy(y);
    
    mintempmap[x][y]=amount;
}

int planet::avetempwrap(int x, int y) const
{
    x=wrapx(x);
    y=clipy(y);
    
    return ((mintempmap[x][y]+maxtempmap[x][y])/2);
}


int planet::summerrainwrap(int x, int y) const
{
    x=wrapx(x);
    y=clipy(y);
    
    return summerrainmap[x][y];
}

void planet::setsummerrainwrap(int x, int y, int amount)
{
    x=wrapx(x);
    y=clipy(y);
    
    summerrainmap[x][y]=amount;
}

int planet::winterrainwrap(int x, int y) const
{
    x=wrapx(x);
    y=clipy(y);
    
    return winterrainmap[x][y];
}

void planet::setwinterrainwrap(int x, int y, int amount)
{
    x=wrapx(x);
    y=clipy(y);
    
    winterrainmap[x][y]=amount;
}

short planet::climatewrap(int x, int y) const
{
    x=wrapx(x);
    y=clipy(y);
    
    return climatemap[x][y];
}

void planet::setclimatewrap(int x, int y, short amount)
{
    x=wrapx(x);
    y=clipy(y);
    
    climatemap[x][y]=amount;
}

int planet::seaicewrap(int x, int y) const
{
    x=wrapx(x);
    y=clipy(y);
    
    return seaicemap[x][y];
}

void planet::setseaicewrap(int x, int y, int amount)
{
    x=wrapx(x);
    y=clipy(y);
    
    seaicemap[x][y]=amount;
}

int planet::riverdirwrap(int x, int y) const
{
    x=wrapx(x);
    y=clipy(y);
    
    return rivermapdir[x][y];
}
void planet::setriverdirwrap(int x, int y, int amount)
{
    x=wrapx(x);
    y=clipy(y);
    
    rivermapdir[x][y]=amount;
}

int planet::riverjanwrap(int x, int y) const
{
    x=wrapx(x);
    y=clipy(y);
    
    return rivermapjan[x][y];
}

void planet::setriverjanwrap(int x, int y, int amount)
{
    x=wrapx(x);
    y=clipy(y);
    
    rivermapjan[x][y]=amount;
}

int planet::riverjulwrap(int x, int y) const
{
    x=wrapx(x);
    y=clipy(y);
    
    return rivermapjul[x][y];
}

void planet::setriverjulwrap(int x, int y, int amount)
{
    x=wrapx(x);
    y=clipy(y);
    
    rivermapjul[x][y]=amount;
}

int planet::riveravewrap(int x, int y) const
{
    x=wrapx(x);
    y=clipy(y);
    
    return (rivermapjan[x][y]+rivermapjul[x][y])/2;
}

int planet::windwrap(int x, int y) const
{
    x=wrapx(x);
    y=clipy(y);
    
    return windmap[x][y];
}

void planet::setwindwrap(int x, int y, int amount)
{
    x=wrapx(x);
    y=clipy(y);
    
    windmap[x][y]=amount;
}

int planet::lakesurfacewrap(int x, int y) const
{
    x=wrapx(x);
    y=clipy(y);
    
    return lakemap[x][y];
}

void planet::setlakesurfacewrap(int x, int y, int amount)
{
    x=wrapx(x);
    y=clipy(y);
    
    lakemap[x][y]=amount;
}

float planet::roughnesswrap(int x, int y) const
{
    x=wrapx(x);
    y=clipy(y);
    
    return roughnessmap[x][y];
}

void planet::setroughnesswrap(int x, int y, float amount)
{
    x=wrapx(x);
    y=clipy(y);
    
    roughnessmap[x][y]=amount;
}

int planet::mountainridgewrap(int x, int y) const
{
    x=wrapx(x);
    y=clipy(y);
    
    return mountainridges[x][y];
}

void planet::setmountainridgewrap(int x, int y, int amount)
{
    x=wrapx(x);
    y=clipy(y);
    
    mountainridges[x][y]=amount;
}

int planet::mountainheightwrap(int x, int y) const
{
    x=wrapx(x);
    y=clipy(y);
    
    return mountainheights[x][y];
}

void planet::setmountainheightwrap(int x, int y, int amount)
{
    x=wrapx(x);
    y=clipy(y);
    
    mountainheights[x][y]=amount;
}

int planet::tidewrap(int x, int y) const
{
    x=wrapx(x);
    y=clipy(y);
    
    return tidalmap[x][y];
}

void planet::settidewrap(int x, int y, int amount)
{
    x=wrapx(x);
    y=clipy(y);
    
    tidalmap[x][y]=amount;
}

int planet::riftlakesurfacewrap(int x, int y) const
{
    x=wrapx(x);
    y=clipy(y);
    
    return riftlakemapsurface[x][y];
}

void planet::setriftlakesurfacewrap(int x, int y, int amount)
{
    x=wrapx(x);
    y=clipy(y);
    
    riftlakemapsurface[x][y]=amount;
}

int planet::riftlakebedwrap(int x, int y) const
{
    x=wrapx(x);
    y=clipy(y);
    
    return riftlakemapbed[x][y];
}

void planet::setriftlakebedwrap(int x, int y, int amount)
{
    x=wrapx(x);
    y=clipy(y);
    
    riftlakemapbed[x][y]=amount;
}

int planet::specialwrap(int x, int y) const
{
    x=wrapx(x);
    y=clipy(y);
    
    return specials[x][y];
}

void planet::setspecialwrap(int x, int y, int amount)
{
    x=wrapx(x);
    y=clipy(y);
    
    specials[x][y]=amount;
}

int planet::deltadirwrap(int x, int y) const
{
    x=wrapx(x);
    y=clipy(y);
    
    return deltamapdir[x][y];
}

void planet::setdeltadirwrap(int x, int y, int amount)
{
    x=wrapx(x);
    y=clipy(y);
    
    deltamapdir[x][y]=amount;
}

int planet::deltajanwrap(int x, int y) const
{
    x=wrapx(x);
    y=clipy(y);
    
    return deltamapjan[x][y];
}

void planet::setdeltajanwrap(int x, int y, int amount)
{
    x=wrapx(x);
    y=clipy(y);
    
    deltamapjan[x][y]=amount;
}

int planet::deltajulwrap(int x, int y) const
{
    x=wrapx(x);
    y=clipy(y);
    
    return deltamapjul[x][y];
}

void planet::setdeltajulwrap(int x, int y, int amount)
{
    x=wrapx(x);
    y=clipy(y);
    
    deltamapjul[x][y]=amount;
}

int planet::horsewrap(int x, int y) const
{
    x=wrapx(x);
    y=clipy(y);
    
    return horselats[x][y];
}

void planet::sethorsewrap(int x, int y, int amount)
{
    x=wrapx(x);
    y=clipy(y);
    
    horselats[x][y]=amount;
}

// slightly more complicated accessor functions

bool planet::seawrap(int x, int y) const
{
    x=wrapx(x);
    y=clipy(y);
    
    if (mapnom[x][y]<=itssealevel && lakemap[x][y]==0)
        return 1;
    
    else
        return 0;
}

bool planet::outlinewrap(int x, int y) const
{
    x=wrapx(x);
    
    if (y<1||y>itsheight-1)
        return 0;
    
    if (sea(x,y)==0)
    {
        if (seawrap(x-1,y)==1)
            return 1;
        
        if (seawrap(x,y-1)==1)
            return 1;
        
        if (seawrap(x+1,y)==1)
            return 1;
        
        if (seawrap(x,y+1)==1)
            return 1;
    }
    
    return 0;
}

int planet::latitudewrap(int x, int y) const
{
    x=wrapx(x);
    y=clipy(y);
    
    float yy=y;
    
    float equator=itsheight/2;
    
    float pixelsperlat=equator/90.0;
    
    float flat=yy/pixelsperlat;
    
    int lat;
    
    if (y<equator)
        flat=90.0-flat;
    
    else
        flat=flat-90.0;
    
    lat=flat;
    
    return lat;
}

int planet::jantempwrap(int x, int y) const
{
    x=wrapx(x);
    y=clipy(y);
    
    if (y<itsheight/2)
        return(mintempmap[x][y]);
    else
        return(maxtempmap[x][y]);
}

void planet::setjantempwrap(int x, int y, int amount)
{
    x=wrapx(x);
    y=clipy(y);
    
    if (y<itsheight/2)
        mintempmap[x][y]=amount;
    else
        maxtempmap[x][y]=amount;
}

int planet::jultempwrap(int x, int y) const
{
    x=wrapx(x);
    y=clipy(y);
    
    if (y<itsheight/2)
        return(maxtempmap[x][y]);
    else
        return(mintempmap[x][y]);
}

void planet::setjultempwrap(int x, int y, int amount)
{
    x=wrapx(x);
    y=clipy(y);
    
    if (y<itsheight/2)
        maxtempmap[x][y]=amount;
    else
        mintempmap[x][y]=amount;
}

int planet::janrainwrap(int x, int y) const
{
    x=wrapx(x);
    y=clipy(y);
    
    if (y<itsheight/2)
        return(winterrainmap[x][y]);
    else
        return(summerrainmap[x][y]);
}

void planet::setjanrainwrap(int x, int y, int amount)
{
    x=wrapx(x);
    y=clipy(y);
    
    if (y<itsheight/2)
        winterrainmap[x][y]=amount;
    else
        summerrainmap[x][y]=amount;
}

int planet::julrainwrap(int x, int y) const
{
    x=wrapx(x);
    y=clipy(y);
    
    if (y<itsheight/2)
        return(summerrainmap[x][y]);
    else
        return(winterrainmap[x][y]);
}

void planet::setjulrainwrap(int x, int y, int amount)
{
    x=wrapx(x);
    y=clipy(y);
    
    if (y<itsheight/2)
        summerrainmap[x][y]=amount;
    else
        winterrainmap[x][y]=amount;
}

int planet::averainwrap(int x, int y) const
{
    x=wrapx(x);
    y=clipy(y);
    
    return ((winterrainmap[x][y]+summerrainmap[x][y])/2);
}

int planet::truelakewrap(int x, int y) const
{
    x=wrapx(x);
    y=clipy(y);
    
    if (lakesurface(x,y)!=0 && special(x,y)<110)
        return (1);
    else
        return(0);
}

// Other public functions.

void planet::clear()
{
    for (int i=0; i<ARRAYWIDTH; i++) // Set all the maps to 0.
    {
        for (int j=0; j<ARRAYHEIGHT; j++)
        {
            maxtempmap[i][j]=0;
            mintempmap[i][j]=0;
            climatemap[i][j]=0;
            summerrainmap[i][j]=0;
            winterrainmap[i][j]=0;
            wintermountainrainmap[i][j]=0;
            summermountainrainmap[i][j]=0;
            wintermountainraindirmap[i][j]=0;
            summermountainraindirmap[i][j]=0;
            seaicemap[i][j]=0;
            rivermapdir[i][j]=0;
            rivermapjan[i][j]=0;
            rivermapjul[i][j]=0;
            windmap[i][j]=0;
            lakemap[i][j]=0;
            roughnessmap[i][j]=0;
            mountainridges[i][j]=0;
            mountainheights[i][j]=0;
            mapnom[i][j]=0;
            tidalmap[i][j]=0;
            riftlakemapsurface[i][j]=0;
            riftlakemapbed[i][j]=0;
            specials[i][j]=0;
            extraelevmap[i][j]=0;
            deltamapdir[i][j]=0;
            deltamapjan[i][j]=0;
            deltamapjul[i][j]=0;
            oceanridgemap[i][j]=0;
            oceanridgeheightmap[i][j]=0;
            oceanriftmap[i][j]=0;
            oceanridgeoffsetmap[i][j]=0;
            islandmap[i][j]=0;
            noshademap[i][j]=0;
            mountainislandmap[i][j]=0;
            volcanomap[i][j]=0;
            stratomap[i][j]=0;
            
            testmap[i][j]=0;
        }
        
        for (int j=0; j<6; j++)
        {
            horselats[i][j]=0;
        }
    }
}

void planet::smoothnom(int amount)
{
    smooth(mapnom,amount,1,1);
}

void planet::smoothextraelev(int amount)
{
    smoothoverland(extraelevmap,amount,0);
}

void planet::shiftterrain(int offset)
{
    shift(mapnom,offset);
    shift(mountainheights,offset);
    shift(mountainridges,offset);
    shift(extraelevmap,offset);
    shift(oceanridgemap,offset);
    shift(oceanridgeheightmap,offset);
    shift(oceanriftmap,offset);
    shift(oceanridgeanglemap,offset);
    shift(mountainislandmap,offset);
    shift(noshademap,offset);
    shift(volcanomap,offset);
    shift(stratomap,offset);
    shift(testmap,offset);
}

void planet::smoothrainmaps(int amount)
{
    smoothoverland(winterrainmap,amount,0);
    smoothoverland(summerrainmap,amount,0);
}

void planet::setmaxriverflow()
{
    int largest=0;
    int current=0;
    
    for (int i=0; i<=itswidth; i++)
    {
        for (int j=0; j<=itsheight; j++)
        {
            current=riveraveflow(i,j);
            
            if (current>largest)
                largest=current;
        }
    }
    itsmaxriverflow=largest;
}

// Private member functions.

int planet::wrapx(int x) const // This wraps X coordinates so they point to proper locations on the map.
{
    while (x>itswidth) // If it's too large, wrap it.
    {
        x=x-itswidth;
    }
    
    while (x<0) // If it's too small, wrap it.
    {
        x=x+itswidth;
    }
    
    return(x);
}

int planet::clipy(int y) const // This clips Y coordinates so they can't be off the map.
{
    if (y<0)
        y=0;
    
    if (y>itsheight)
        y=itsheight;
    
    return(y);
}

void planet::smooth(int arr[][ARRAYHEIGHT], int amount, bool vary, bool avoidmountains) // This smoothes the given array by the given amount.
{
    for (int i=0; i<=itswidth; i++)
    {
        for (int j=1; j<itsheight; j++)
        {
            if (avoidmountains==0 || mountainheights[i][j]==0)
            {
                int crount=0;
                int ave=0;
                
                for (int k=i-amount; k<=i+amount; k++)
                {
                    int kk=k;
                    
                    if (kk<0)
                        kk=itswidth;
                    
                    if (kk>itswidth)
                        kk=0;
                    
                    for (int l=j-amount; l<=j+amount; l++)
                    {
                        ave=ave+mapnom[kk][l];
                        crount++;
                    }
                }
                ave=ave/crount;
                
                if (ave>0 && ave<itsmaxheight)
                    mapnom[i][j]=ave;
            }
            
        }
    }
}

// This does the same, but only over land.

void planet::smoothoverland(int arr[][ARRAYHEIGHT], int amount, bool uponly)
{
    for (int i=0; i<=itswidth; i++)
    {
        for (int j=1; j<itsheight; j++)
        {
            if (sea(i,j)==0)
            {
                int crount=0;
                int ave=0;
                
                for (int k=i-amount; k<=i+amount; k++)
                {
                    int kk=k;
                    
                    if (kk<0)
                        kk=itswidth;
                    
                    if (kk>itswidth)
                        kk=0;
                    
                    for (int l=j-amount; l<=j+amount; l++)
                    {
                        ave=ave+arr[kk][l];
                        crount++;
                    }
                }
                
                if (crount>0)
                {
                    ave=ave/crount;
                    
                    if (ave>0 && ave<itsmaxheight)
                    {
                        if (uponly==0)
                            arr[i][j]=ave;
                        else
                        {
                            if (ave>arr[i][j])
                                arr[i][j]=ave;
                        }
                    }
                }
            }
        }
    }
}

// This function shifts everything in an array to the left by a given number of pixels.

void planet::shift(int arr[][ARRAYHEIGHT], int offset)
{
    vector<vector<int>> dummy(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

    //int dummy[ARRAYWIDTH][ARRAYHEIGHT];
    
    for (int i=0; i<=itswidth; i++)
    {
        for (int j=0; j<=itsheight; j++)
            dummy[i][j]=arr[i][j];
    }
    
    for (int i=0; i<=itswidth; i++)
    {
        int ii=i+offset;
        
        if (ii<0 || ii>itswidth)
            ii=wrap(ii,itswidth);
        
        for (int j=0; j<=itsheight; j++)
            arr[i][j]=dummy[ii][j];
    }
}

// The same thing, but for a float array.

void planet::shift(float arr[][ARRAYHEIGHT], int offset)
{
    vector<vector<float>> dummy(ARRAYWIDTH, vector<float>(ARRAYHEIGHT, 0));
    
    //float dummy[ARRAYWIDTH][ARRAYHEIGHT];
    
    for (int i=0; i<=itswidth; i++)
    {
        for (int j=0; j<=itsheight; j++)
            dummy[i][j]=arr[i][j];
    }
    
    for (int i=0; i<=itswidth; i++)
    {
        int ii=i+offset;
        
        if (ii<0 || ii>itswidth)
            ii=wrap(ii,itswidth);
        
        for (int j=0; j<=itsheight; j++)
            arr[i][j]=dummy[ii][j];
    }
}

// The same thing, but for a bool array.

void planet::shift(bool arr[][ARRAYHEIGHT], int offset)
{
    vector<vector<bool>> dummy(ARRAYWIDTH, vector<bool>(ARRAYHEIGHT, 0));
    
    //bool dummy[ARRAYWIDTH][ARRAYHEIGHT];
    
    for (int i=0; i<=itswidth; i++)
    {
        for (int j=0; j<=itsheight; j++)
            dummy[i][j]=arr[i][j];
    }
    
    for (int i=0; i<=itswidth; i++)
    {
        int ii=i+offset;
        
        if (ii<0 || ii>itswidth)
            ii=wrap(ii,itswidth);
        
        for (int j=0; j<=itsheight; j++)
            arr[i][j]=dummy[ii][j];
    }
}

// The same thing, but for a uint8_t array.

void planet::shift(uint8_t arr[][ARRAYHEIGHT], int offset)
{
    vector<vector<uint8_t>> dummy(ARRAYWIDTH, vector<uint8_t>(ARRAYHEIGHT, 0));

    for (int i = 0; i <= itswidth; i++)
    {
        for (int j = 0; j <= itsheight; j++)
            dummy[i][j] = arr[i][j];
    }

    for (int i = 0; i <= itswidth; i++)
    {
        int ii = i + offset;

        if (ii<0 || ii>itswidth)
            ii = wrap(ii, itswidth);

        for (int j = 0; j <= itsheight; j++)
            arr[i][j] = dummy[ii][j];
    }
}

