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

void planet::saveworld(string filename)
{
    ofstream outfile;
    outfile.open(filename, ios::out);

    writevariable(outfile,itswidth);
    writevariable(outfile,itsheight);
    writevariable(outfile,itsseed);
    writevariable(outfile,itsrotation);
    writevariable(outfile,itsriverfactor);
    writevariable(outfile,itsriverlandreduce);
    writevariable(outfile,itsestuarylimit);
    writevariable(outfile,itsglacialtemp);
    writevariable(outfile,itsglaciertemp);
    writevariable(outfile,itsmountainreduce);
    writevariable(outfile,itsclimateno);
    writevariable(outfile,itsmaxheight);
    writevariable(outfile,itssealevel);

    writevariable(outfile,itslandshading);
    writevariable(outfile,itslakeshading);
    writevariable(outfile,itsseashading);
    writevariable(outfile,itsshadingdir);
    writevariable(outfile,itssnowchange);
    writevariable(outfile,itsseaiceappearance);
    writevariable(outfile,itslandmarbling);
    writevariable(outfile,itslakemarbling);
    writevariable(outfile,itsseamarbling);
    writevariable(outfile,itsminriverflowglobal);
    writevariable(outfile,itsminriverflowregional);
    writevariable(outfile,itsseaice1);
    writevariable(outfile,itsseaice2);
    writevariable(outfile,itsseaice3);
    writevariable(outfile,itsocean1);
    writevariable(outfile,itsocean2);
    writevariable(outfile,itsocean3);
    writevariable(outfile,itsdeepocean1);
    writevariable(outfile,itsdeepocean2);
    writevariable(outfile,itsdeepocean3);
    writevariable(outfile,itsbase1);
    writevariable(outfile,itsbase2);
    writevariable(outfile,itsbase3);
    writevariable(outfile,itsbasetemp1);
    writevariable(outfile,itsbasetemp2);
    writevariable(outfile,itsbasetemp3);
    writevariable(outfile,itshighbase1);
    writevariable(outfile,itshighbase2);
    writevariable(outfile,itshighbase3);
    writevariable(outfile,itsdesert1);
    writevariable(outfile,itsdesert2);
    writevariable(outfile,itsdesert3);
    writevariable(outfile,itshighdesert1);
    writevariable(outfile,itshighdesert2);
    writevariable(outfile,itshighdesert3);
    writevariable(outfile,itscolddesert1);
    writevariable(outfile,itscolddesert2);
    writevariable(outfile,itscolddesert3);
    writevariable(outfile,itsgrass1);
    writevariable(outfile,itsgrass2);
    writevariable(outfile,itsgrass3);
    writevariable(outfile,itscold1);
    writevariable(outfile,itscold2);
    writevariable(outfile,itscold3);
    writevariable(outfile,itstundra1);
    writevariable(outfile,itstundra2);
    writevariable(outfile,itstundra3);
    writevariable(outfile,itseqtundra1);
    writevariable(outfile,itseqtundra2);
    writevariable(outfile,itseqtundra3);
    writevariable(outfile,itssaltpan1);
    writevariable(outfile,itssaltpan2);
    writevariable(outfile,itssaltpan3);
    writevariable(outfile,itserg1);
    writevariable(outfile,itserg2);
    writevariable(outfile,itserg3);
    writevariable(outfile,itswetlands1);
    writevariable(outfile,itswetlands2);
    writevariable(outfile,itswetlands3);
    writevariable(outfile,itslake1);
    writevariable(outfile,itslake2);
    writevariable(outfile,itslake3);
    writevariable(outfile,itsriver1);
    writevariable(outfile,itsriver2);
    writevariable(outfile,itsriver3);
    writevariable(outfile,itsglacier1);
    writevariable(outfile,itsglacier2);
    writevariable(outfile,itsglacier3);
    writevariable(outfile,itshighlight1);
    writevariable(outfile,itshighlight2);
    writevariable(outfile,itshighlight3);

    writedata(outfile,maxtempmap);
    writedata(outfile,mintempmap);
    writedata(outfile, climatemap);
    writedata(outfile,summerrainmap);
    writedata(outfile,winterrainmap);
    writedata(outfile,wintermountainrainmap);
    writedata(outfile,summermountainrainmap);
    writedata(outfile, wintermountainraindirmap);
    writedata(outfile, summermountainraindirmap);
    writedata(outfile,seaicemap);
    writedata(outfile,rivermapdir);
    writedata(outfile,rivermapjan);
    writedata(outfile,rivermapjul);
    writedata(outfile,windmap);
    writedata(outfile,lakemap);
    writedata(outfile, roughnessmap);
    writedata(outfile,mountainridges);
    writedata(outfile,mountainheights);
    writedata(outfile,mapnom);
    writedata(outfile,tidalmap);
    writedata(outfile,riftlakemapsurface);
    writedata(outfile,riftlakemapbed);
    writedata(outfile, lakestartmap);
    writedata(outfile,specials);
    writedata(outfile,extraelevmap);
    writedata(outfile,deltamapdir);
    writedata(outfile,deltamapjan);
    writedata(outfile,deltamapjul);
    writedata(outfile, islandmap);
    writedata(outfile, mountainislandmap);
    writedata(outfile,oceanridgemap);
    writedata(outfile,oceanridgeheightmap);
    writedata(outfile,oceanriftmap);
    writedata(outfile,oceanridgeoffsetmap);
    writedata(outfile,oceanridgeanglemap);
    writedata(outfile,volcanomap);
    writedata(outfile, stratomap);
    writedata(outfile, noshademap);
    writedata(outfile,testmap);

    for (int i = 0; i < NOISEWIDTH; i++)
    {
        for (int j = 0; j < NOISEHEIGHT; j++)
            writevariable(outfile,itsnoisemap[i][j]);
    }

    for (int i = 0; i < ARRAYWIDTH; i++)
    {
        for (int j = 0; j < 6; j++)
            writevariable(outfile,horselats[i][j]);
    }

    if (!outfile.good())
    {
        cerr << "Error writing world '" << filename << "'" << endl;
    }
}

void planet::loadworld(string filename)
{
    ifstream infile;
    infile.open(filename, ios::in);

    readvariable(infile,itswidth);
    readvariable(infile, itsheight);
    readvariable(infile, itsseed);
    readvariable(infile, itsrotation);
    readvariable(infile, itsriverfactor);
    readvariable(infile, itsriverlandreduce);
    readvariable(infile, itsestuarylimit);
    readvariable(infile, itsglacialtemp);
    readvariable(infile, itsglaciertemp);
    readvariable(infile, itsmountainreduce);
    readvariable(infile, itsclimateno);
    readvariable(infile, itsmaxheight);
    readvariable(infile, itssealevel);
    readvariable(infile, itslandshading);
    readvariable(infile, itslakeshading);
    readvariable(infile, itsseashading);
    readvariable(infile, itsshadingdir);
    readvariable(infile, itssnowchange);
    readvariable(infile, itsseaiceappearance);
    readvariable(infile, itslandmarbling);
    readvariable(infile, itslakemarbling);
    readvariable(infile, itsseamarbling);
    readvariable(infile, itsminriverflowglobal);
    readvariable(infile, itsminriverflowregional);
    readvariable(infile, itsseaice1);
    readvariable(infile, itsseaice2);
    readvariable(infile, itsseaice3);
    readvariable(infile, itsocean1);
    readvariable(infile, itsocean2);
    readvariable(infile, itsocean3);
    readvariable(infile, itsdeepocean1);
    readvariable(infile, itsdeepocean2);
    readvariable(infile, itsdeepocean3);
    readvariable(infile, itsbase1);
    readvariable(infile, itsbase2);
    readvariable(infile, itsbase3);
    readvariable(infile, itsbasetemp1);
    readvariable(infile, itsbasetemp2);
    readvariable(infile, itsbasetemp3);
    readvariable(infile, itshighbase1);
    readvariable(infile, itshighbase2);
    readvariable(infile, itshighbase3);
    readvariable(infile, itsdesert1);
    readvariable(infile, itsdesert2);
    readvariable(infile, itsdesert3);
    readvariable(infile, itshighdesert1);
    readvariable(infile, itshighdesert2);
    readvariable(infile, itshighdesert3);
    readvariable(infile, itscolddesert1);
    readvariable(infile, itscolddesert2);
    readvariable(infile, itscolddesert3);
    readvariable(infile, itsgrass1);
    readvariable(infile, itsgrass2);
    readvariable(infile, itsgrass3);
    readvariable(infile, itscold1);
    readvariable(infile, itscold2);
    readvariable(infile, itscold3);
    readvariable(infile, itstundra1);
    readvariable(infile, itstundra2);
    readvariable(infile, itstundra3);
    readvariable(infile, itseqtundra1);
    readvariable(infile, itseqtundra2);
    readvariable(infile, itseqtundra3);
    readvariable(infile, itssaltpan1);
    readvariable(infile, itssaltpan2);
    readvariable(infile, itssaltpan3);
    readvariable(infile, itserg1);
    readvariable(infile, itserg2);
    readvariable(infile, itserg3);
    readvariable(infile, itswetlands1);
    readvariable(infile, itswetlands2);
    readvariable(infile, itswetlands3);
    readvariable(infile, itslake1);
    readvariable(infile, itslake2);
    readvariable(infile, itslake3);
    readvariable(infile, itsriver1);
    readvariable(infile, itsriver2);
    readvariable(infile, itsriver3);
    readvariable(infile, itsglacier1);
    readvariable(infile, itsglacier2);
    readvariable(infile, itsglacier3);
    readvariable(infile, itshighlight1);
    readvariable(infile, itshighlight2);
    readvariable(infile, itshighlight3);

    readdata(infile, maxtempmap);
    readdata(infile, mintempmap);
    readdata(infile, climatemap);
    readdata(infile, summerrainmap);
    readdata(infile, winterrainmap);
    readdata(infile, wintermountainrainmap);
    readdata(infile, summermountainrainmap);
    readdata(infile, wintermountainraindirmap);
    readdata(infile, summermountainraindirmap);
    readdata(infile, seaicemap);
    readdata(infile, rivermapdir);
    readdata(infile, rivermapjan);
    readdata(infile, rivermapjul);
    readdata(infile, windmap);
    readdata(infile, lakemap);
    readdata(infile, roughnessmap);
    readdata(infile, mountainridges);
    readdata(infile, mountainheights);
    readdata(infile, mapnom);
    readdata(infile, tidalmap);
    readdata(infile, riftlakemapsurface);
    readdata(infile, riftlakemapbed);
    readdata(infile, lakestartmap);
    readdata(infile, specials);
    readdata(infile, extraelevmap);
    readdata(infile, deltamapdir);
    readdata(infile, deltamapjan);
    readdata(infile, deltamapjul);
    readdata(infile, islandmap);
    readdata(infile, mountainislandmap);
    readdata(infile, oceanridgemap);
    readdata(infile, oceanridgeheightmap);
    readdata(infile, oceanriftmap);
    readdata(infile, oceanridgeoffsetmap);
    readdata(infile, oceanridgeanglemap);
    readdata(infile, volcanomap);
    readdata(infile, stratomap);
    readdata(infile, noshademap);
    readdata(infile, testmap);

    for (int i = 0; i < NOISEWIDTH; i++)
    {
        for (int j = 0; j < NOISEHEIGHT; j++)
            readvariable(infile, itsnoisemap[i][j]);
    }

    for (int i = 0; i < ARRAYWIDTH; i++)
    {
        for (int j = 0; j < 6; j++)
            readvariable(infile, horselats[i][j]);
    }

    setmaxriverflow();

    if (!infile.good())
    {
        cerr << "Error reading world '" << filename << "'" << endl;
    }
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

template<typename T> void planet::shift(T arr[][ARRAYHEIGHT], int offset)
{
    vector<vector<T>> dummy(ARRAYWIDTH, vector<T>(ARRAYHEIGHT, 0));

    //T dummy[ARRAYWIDTH][ARRAYHEIGHT];

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

// Functions for saving member variables.

template<typename T> void planet::writevariable(ofstream& outfile, T val)
{
    outfile << val << '\n';
}

// Functions for saving member arrays.

template<typename T> void planet::writedata(ofstream& outfile, T const arr[ARRAYWIDTH][ARRAYHEIGHT])
{
    for (int i = 0; i < ARRAYWIDTH; i++)
    {
        for (int j = 0; j < ARRAYHEIGHT; j++)
            outfile << arr[i][j] << '\n';
    }
}

// Functions for loading member variables.

void read_val(string const &line, int &val) {
    val = stoi(line);
}
void read_val(string const &line, bool &val) {
    val = stob(line);
}
void read_val(string const &line, short &val) {
    val = stos(line);
}
void read_val(string const &line, float &val) {
    val = stof(line);
}
void read_val(string const &line, long &val) {
    val = stol(line);
}

template<typename T> void planet::readvariable(ifstream& infile, T &val)
{
    string line;
    getline(infile, line);
    read_val(line, val);
}

// Functions for loading member arrays.

template<typename T> void planet::readdata(ifstream& infile, T arr[ARRAYWIDTH][ARRAYHEIGHT])
{
    string line;

    for (int i = 0; i < ARRAYWIDTH; i++)
    {
        for (int j = 0; j < ARRAYHEIGHT; j++)
        {
            getline(infile, line);
            read_val(line, arr[i][j]);
        }
    }
}

