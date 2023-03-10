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

//#define ENABLE_PROFILER
#ifdef ENABLE_PROFILER
#include "profiler.h"
#endif

planet::planet() //constructor
{
    itswidth = 1024;
    itsheight = 512;
}

planet::~planet()
{
}

bool planet::outline(int x, int y) const
{
    if (y<1 || y>itsheight - 1)
        return 0;

    if (sea(x, y) == 0)
    {
        if (seawrap(x - 1, y) == 1)
            return 1;

        if (seawrap(x, y - 1) == 1)
            return 1;

        if (seawrap(x + 1, y) == 1)
            return 1;

        if (seawrap(x, y + 1) == 1)
            return 1;
    }

    return 0;
}

bool planet::coast(int x, int y) const
{
    if (y<1 || y>itsheight - 1 || x<0 || x>itswidth)
        return 0;

    if (sea(x, y) == 1)
    {
        if (seawrap(x - 1, y) == 0)
            return 1;

        if (seawrap(x, y - 1) == 0)
            return 1;

        if (seawrap(x + 1, y) == 0)
            return 1;

        if (seawrap(x, y + 1) == 0)
            return 1;
    }

    return 0;
}

void planet::longitude(int x, int &degrees, int &minutes, int &seconds, bool& negative) const
{
    if ( x<0 || x>itswidth)
        return;

    float fx = (float)x;

    float worldwidth = (float)itswidth + 1.0f;

    float pixelsperlong = worldwidth / 360.0f;

    float longitude = fx / pixelsperlong;

    longitude = longitude - 180.0f;

    degrees = (int)longitude;

    float dec = (longitude - (float)degrees) * 60.0f;

    minutes = (int)dec;

    float dec2 = (dec - (float)minutes) * 60;

    seconds = (int)dec2;

    if (degrees < 0)
        degrees = 0 - degrees;

    if (minutes < 0)
        minutes = 0 - minutes;

    if (seconds < 0)
        seconds = 0 - seconds;

    negative = 0;

    if (fx < worldwidth / 2.f)
        negative = 1;

    return;
}

void planet::latitude(int y, int& degrees, int& minutes, int& seconds, bool &negative) const
{
    if (y<0 || y>itsheight)
        return;

    bool hemisphere = 0; // 1 for southern

    float fy = (float)y;

    float worldheight = (float)itsheight;
    float worldhalfheight = worldheight / 2.0f;

    float pixelsperlat = worldhalfheight / 90.0f;

    float latitude;

    if (fy <= worldhalfheight)
        latitude = (worldhalfheight - fy) / pixelsperlat;
    else
    {
        latitude = (fy - worldhalfheight) / pixelsperlat;
        hemisphere = 1;
    }

    degrees = (int)latitude;

    float dec = (latitude - (float)degrees) * 60.0f;

    minutes = (int)dec;

    float dec2 = (dec - (float)minutes) * 60;

    seconds = (int)dec2;

    negative = 0;

    if (hemisphere == 1)
        negative = 1;

    return;
}

int planet::reverselatitude(int lat) const
{
    float flat = 90.0f - (float)lat;

    float equator = (float)itsheight / 2.0f;

    float pixelsperlat = equator / 90.0f;

    float y = flat * pixelsperlat;

    return (int)y;
}

// slightly more complicated accessor functions

bool planet::seawrap(int x, int y) const
{
    x = wrapx(x);
    y = clipy(y);

    if (mapnom[x][y] <= itssealevel && lakemap[x][y] == 0)
        return 1;

    else
        return 0;
}

bool planet::outlinewrap(int x, int y) const
{
    x = wrapx(x);

    if (y<1 || y>itsheight - 1)
        return 0;

    if (sea(x, y) == 0)
    {
        if (seawrap(x - 1, y) == 1)
            return 1;

        if (seawrap(x, y - 1) == 1)
            return 1;

        if (seawrap(x + 1, y) == 1)
            return 1;

        if (seawrap(x, y + 1) == 1)
            return 1;
    }

    return 0;
}

int planet::mountainheightwrap(int x, int y) const
{
    x = wrapx(x);
    y = clipy(y);

    return mountainheights[x][y];
}

// Other public functions.

void planet::clear()
{
    for (int i = 0; i < ARRAYWIDTH; i++) // Set all the maps to 0.
    {
        for (int j = 0; j < ARRAYHEIGHT; j++)
        {
            jantempmap[i][j] = 0;
            jultempmap[i][j] = 0;
            climatemap[i][j] = 0;
            janrainmap[i][j] = 0;
            julrainmap[i][j] = 0;
            janmountainrainmap[i][j] = 0;
            julmountainrainmap[i][j] = 0;
            janmountainraindirmap[i][j] = 0;
            julmountainraindirmap[i][j] = 0;
            seaicemap[i][j] = 0;
            rivermapdir[i][j] = 0;
            rivermapjan[i][j] = 0;
            rivermapjul[i][j] = 0;
            windmap[i][j] = 0;
            lakemap[i][j] = 0;
            roughnessmap[i][j] = 0;
            mountainridges[i][j] = 0;
            mountainheights[i][j] = 0;
            craterrims[i][j] = 0;
            cratercentres[i][j] = 0;
            mapnom[i][j] = 0;
            tidalmap[i][j] = 0;
            riftlakemapsurface[i][j] = 0;
            riftlakemapbed[i][j] = 0;
            specials[i][j] = 0;
            extraelevmap[i][j] = 0;
            deltamapdir[i][j] = 0;
            deltamapjan[i][j] = 0;
            deltamapjul[i][j] = 0;
            oceanridgemap[i][j] = 0;
            oceanridgeheightmap[i][j] = 0;
            oceanriftmap[i][j] = 0;
            oceanridgeoffsetmap[i][j] = 0;
            islandmap[i][j] = 0;
            noshademap[i][j] = 0;
            mountainislandmap[i][j] = 0;
            volcanomap[i][j] = 0;
            stratomap[i][j] = 0;
            noisemap[i][j] = 0;
            testmap[i][j] = 0;
        }

        for (int j = 0; j < 6; j++)
        {
            horselats[i][j] = 0;
        }
    }

    for (int i = 0; i < MAXCRATERS; i++)
    {
        cratercentreslist[i].w = 0;
        cratercentreslist[i].x = 0;
        cratercentreslist[i].y = 0;
        cratercentreslist[i].z = 0;
    }
}

void planet::smoothnom(int amount)
{
    smooth(mapnom, amount, 1, 1);
}

void planet::smoothextraelev(int amount)
{
    smoothoverland(extraelevmap, amount, 0);
}

void planet::shiftterrain(int offset)
{
    shift(mapnom, offset);
    shift(mountainheights, offset);
    shift(mountainridges, offset);
    shift(craterrims, offset);
    shift(cratercentres, offset);
    shift(extraelevmap, offset);
    shift(oceanridgemap, offset);
    shift(oceanridgeheightmap, offset);
    shift(oceanriftmap, offset);
    shift(oceanridgeanglemap, offset);
    shift(mountainislandmap, offset);
    shift(noshademap, offset);
    shift(volcanomap, offset);
    shift(stratomap, offset);
    shift(testmap, offset);

    for (int i = 0; i < itscraterno; i++)
    {
        cratercentreslist[i].x = cratercentreslist[i].x - offset;

        if (cratercentreslist[i].x < 0)
            cratercentreslist[i].x = cratercentreslist[i].x + itswidth;
    }
}

void planet::smoothrainmaps(int amount)
{
    smoothoverland(janrainmap, amount, 0);
    smoothoverland(julrainmap, amount, 0);
}

void planet::setmaxriverflow()
{
    int largest = 0;
    int current = 0;

    for (int i = 0; i <= itswidth; i++)
    {
        for (int j = 0; j <= itsheight; j++)
        {
            current = riveraveflow(i, j);

            if (current > largest)
                largest = current;
        }
    }
    itsmaxriverflow = largest;
}

void planet::saveworld(string filename)
{
#ifdef ENABLE_PROFILER
    highres_timer_t timer("Save World"); // 26.5s => 10.9s
#endif
    ofstream outfile;
    outfile.open(filename, ios::out);

    writevariable(outfile, itssaveversion);
    writevariable(outfile, itssize);
    writevariable(outfile, itswidth);
    writevariable(outfile, itsheight);
    writevariable(outfile, itstype);
    writevariable(outfile, itsseed);
    writevariable(outfile, itsrotation);
    writevariable(outfile, itstilt);
    writevariable(outfile, itseccentricity);
    writevariable(outfile, itsperihelion);
    writevariable(outfile, itsgravity);
    writevariable(outfile, itslunar);
    writevariable(outfile, itstempdecrease);
    writevariable(outfile, itsnorthpolaradjust);
    writevariable(outfile, itssouthpolaradjust);
    writevariable(outfile, itsaveragetemp);
    writevariable(outfile, itsnorthpolartemp);
    writevariable(outfile, itssouthpolartemp);
    writevariable(outfile, itseqtemp);
    writevariable(outfile, itswaterpickup);
    writevariable(outfile, itsriverfactor);
    writevariable(outfile, itsriverlandreduce);
    writevariable(outfile, itsestuarylimit);
    writevariable(outfile, itsglacialtemp);
    writevariable(outfile, itsglaciertemp);
    writevariable(outfile, itsmountainreduce);
    writevariable(outfile, itsclimateno);
    writevariable(outfile, itsmaxheight);
    writevariable(outfile, itssealevel);
    writevariable(outfile, itslandtotal);
    writevariable(outfile, itsseatotal);
    writevariable(outfile, itscraterno);

    writevariable(outfile, itslandshading);
    writevariable(outfile, itslakeshading);
    writevariable(outfile, itsseashading);
    writevariable(outfile, itsshadingdir);
    writevariable(outfile, itssnowchange);
    writevariable(outfile, itsseaiceappearance);
    writevariable(outfile, itslandmarbling);
    writevariable(outfile, itslakemarbling);
    writevariable(outfile, itsseamarbling);
    writevariable(outfile, itsminriverflowglobal);
    writevariable(outfile, itsminriverflowregional);
    writevariable(outfile, itsmangroves);
    writevariable(outfile, itscolourcliffs);
    writevariable(outfile, itsseaice1);
    writevariable(outfile, itsseaice2);
    writevariable(outfile, itsseaice3);
    writevariable(outfile, itsocean1);
    writevariable(outfile, itsocean2);
    writevariable(outfile, itsocean3);
    writevariable(outfile, itsdeepocean1);
    writevariable(outfile, itsdeepocean2);
    writevariable(outfile, itsdeepocean3);
    writevariable(outfile, itsbase1);
    writevariable(outfile, itsbase2);
    writevariable(outfile, itsbase3);
    writevariable(outfile, itsbasetemp1);
    writevariable(outfile, itsbasetemp2);
    writevariable(outfile, itsbasetemp3);
    writevariable(outfile, itshighbase1);
    writevariable(outfile, itshighbase2);
    writevariable(outfile, itshighbase3);
    writevariable(outfile, itsdesert1);
    writevariable(outfile, itsdesert2);
    writevariable(outfile, itsdesert3);
    writevariable(outfile, itshighdesert1);
    writevariable(outfile, itshighdesert2);
    writevariable(outfile, itshighdesert3);
    writevariable(outfile, itscolddesert1);
    writevariable(outfile, itscolddesert2);
    writevariable(outfile, itscolddesert3);
    writevariable(outfile, itsgrass1);
    writevariable(outfile, itsgrass2);
    writevariable(outfile, itsgrass3);
    writevariable(outfile, itscold1);
    writevariable(outfile, itscold2);
    writevariable(outfile, itscold3);
    writevariable(outfile, itstundra1);
    writevariable(outfile, itstundra2);
    writevariable(outfile, itstundra3);
    writevariable(outfile, itseqtundra1);
    writevariable(outfile, itseqtundra2);
    writevariable(outfile, itseqtundra3);
    writevariable(outfile, itssaltpan1);
    writevariable(outfile, itssaltpan2);
    writevariable(outfile, itssaltpan3);
    writevariable(outfile, itserg1);
    writevariable(outfile, itserg2);
    writevariable(outfile, itserg3);
    writevariable(outfile, itswetlands1);
    writevariable(outfile, itswetlands2);
    writevariable(outfile, itswetlands3);
    writevariable(outfile, itslake1);
    writevariable(outfile, itslake2);
    writevariable(outfile, itslake3);
    writevariable(outfile, itsriver1);
    writevariable(outfile, itsriver2);
    writevariable(outfile, itsriver3);
    writevariable(outfile, itsglacier1);
    writevariable(outfile, itsglacier2);
    writevariable(outfile, itsglacier3);
    writevariable(outfile, itssand1);
    writevariable(outfile, itssand2);
    writevariable(outfile, itssand3);
    writevariable(outfile, itsmud1);
    writevariable(outfile, itsmud2);
    writevariable(outfile, itsmud3);
    writevariable(outfile, itsshingle1);
    writevariable(outfile, itsshingle2);
    writevariable(outfile, itsshingle3);
    writevariable(outfile, itsmangrove1);
    writevariable(outfile, itsmangrove2);
    writevariable(outfile, itsmangrove3);
    writevariable(outfile, itshighlight1);
    writevariable(outfile, itshighlight2);
    writevariable(outfile, itshighlight3);

    writedata(outfile, jantempmap);
    writedata(outfile, jultempmap);
    writedata(outfile, climatemap);
    writedata(outfile, janrainmap);
    writedata(outfile, julrainmap);
    writedata(outfile, janmountainrainmap);
    writedata(outfile, julmountainrainmap);
    writedata(outfile, janmountainraindirmap);
    writedata(outfile, julmountainraindirmap);
    writedata(outfile, seaicemap);
    writedata(outfile, rivermapdir);
    writedata(outfile, rivermapjan);
    writedata(outfile, rivermapjul);
    writedata(outfile, windmap);
    writedata(outfile, lakemap);
    writedata(outfile, roughnessmap);
    writedata(outfile, mountainridges);
    writedata(outfile, mountainheights);
    writedata(outfile, craterrims);
    writedata(outfile, cratercentres);
    writedata(outfile, mapnom);
    writedata(outfile, tidalmap);
    writedata(outfile, riftlakemapsurface);
    writedata(outfile, riftlakemapbed);
    writedata(outfile, lakestartmap);
    writedata(outfile, specials);
    writedata(outfile, extraelevmap);
    writedata(outfile, deltamapdir);
    writedata(outfile, deltamapjan);
    writedata(outfile, deltamapjul);
    writedata(outfile, islandmap);
    writedata(outfile, mountainislandmap);
    writedata(outfile, oceanridgemap);
    writedata(outfile, oceanridgeheightmap);
    writedata(outfile, oceanriftmap);
    writedata(outfile, oceanridgeoffsetmap);
    writedata(outfile, oceanridgeanglemap);
    writedata(outfile, volcanomap);
    writedata(outfile, stratomap);
    writedata(outfile, noshademap);
    writedata(outfile, noisemap);
    writedata(outfile, testmap);

    for (int i = 0; i < ARRAYWIDTH; i++)
    {
        for (int j = 0; j < 6; j++)
            writevariable(outfile, horselats[i][j]);
    }

    for (int i = 0; i < itscraterno; i++)
    {
        writevariable(outfile, cratercentreslist[i].w);
        writevariable(outfile, cratercentreslist[i].x);
        writevariable(outfile, cratercentreslist[i].y);
        writevariable(outfile, cratercentreslist[i].z);
    }

    if (!outfile.good())
    {
        cerr << "Error writing world '" << filename << "'" << endl;
    }
}

bool planet::loadworld(string filename)
{
#ifdef ENABLE_PROFILER
    highres_timer_t timer("Load World"); // 9.1s => 8.8s
#endif
    ifstream infile;
    infile.open(filename, ios::in);

    int val;
    readvariable(infile, val);

    if (val != itssaveversion) // Incompatible file format!
        return 0;

    readvariable(infile, itssize);
    readvariable(infile, itswidth);
    readvariable(infile, itsheight);
    readvariable(infile, itstype);
    readvariable(infile, itsseed);
    readvariable(infile, itsrotation);
    readvariable(infile, itstilt);
    readvariable(infile, itseccentricity);
    readvariable(infile, itsperihelion);
    readvariable(infile, itsgravity);
    readvariable(infile, itslunar);
    readvariable(infile, itstempdecrease);
    readvariable(infile, itsnorthpolaradjust);
    readvariable(infile, itssouthpolaradjust);
    readvariable(infile, itsaveragetemp);
    readvariable(infile, itsnorthpolartemp);
    readvariable(infile, itssouthpolartemp);
    readvariable(infile, itseqtemp);
    readvariable(infile, itswaterpickup);
    readvariable(infile, itsriverfactor);
    readvariable(infile, itsriverlandreduce);
    readvariable(infile, itsestuarylimit);
    readvariable(infile, itsglacialtemp);
    readvariable(infile, itsglaciertemp);
    readvariable(infile, itsmountainreduce);
    readvariable(infile, itsclimateno);
    readvariable(infile, itsmaxheight);
    readvariable(infile, itssealevel);
    readvariable(infile, itslandtotal);
    readvariable(infile, itsseatotal);
    readvariable(infile, itscraterno);

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
    readvariable(infile, itsmangroves);
    readvariable(infile, itscolourcliffs);
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
    readvariable(infile, itssand1);
    readvariable(infile, itssand2);
    readvariable(infile, itssand3);
    readvariable(infile, itsmud1);
    readvariable(infile, itsmud2);
    readvariable(infile, itsmud3);
    readvariable(infile, itsshingle1);
    readvariable(infile, itsshingle2);
    readvariable(infile, itsshingle3);
    readvariable(infile, itsmangrove1);
    readvariable(infile, itsmangrove2);
    readvariable(infile, itsmangrove3);
    readvariable(infile, itshighlight1);
    readvariable(infile, itshighlight2);
    readvariable(infile, itshighlight3);

    readdata(infile, jantempmap);
    readdata(infile, jultempmap);
    readdata(infile, climatemap);
    readdata(infile, janrainmap);
    readdata(infile, julrainmap);
    readdata(infile, janmountainrainmap);
    readdata(infile, julmountainrainmap);
    readdata(infile, janmountainraindirmap);
    readdata(infile, julmountainraindirmap);
    readdata(infile, seaicemap);
    readdata(infile, rivermapdir);
    readdata(infile, rivermapjan);
    readdata(infile, rivermapjul);
    readdata(infile, windmap);
    readdata(infile, lakemap);
    readdata(infile, roughnessmap);
    readdata(infile, mountainridges);
    readdata(infile, mountainheights);
    readdata(infile, craterrims);
    readdata(infile, cratercentres);
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
    readdata(infile, noisemap);
    readdata(infile, testmap);

    for (int i = 0; i < ARRAYWIDTH; i++)
    {
        for (int j = 0; j < 6; j++)
            readvariable(infile, horselats[i][j]);
    }

    for (int i = 0; i < itscraterno; i++)
    {
        readvariable(infile, cratercentreslist[i].w);
        readvariable(infile, cratercentreslist[i].x);
        readvariable(infile, cratercentreslist[i].y);
        readvariable(infile, cratercentreslist[i].z);
    }

    setmaxriverflow();

    if (!infile.good())
    {
        cerr << "Error reading world '" << filename << "'" << endl;
    }

    return 1;
}

// Private member functions.

int planet::wrapx(int x) const // This wraps X coordinates so they point to proper locations on the map.
{
    while (x > itswidth) // If it's too large, wrap it.
    {
        x = x - itswidth;
    }

    while (x < 0) // If it's too small, wrap it.
    {
        x = x + itswidth;
    }

    return(x);
}

int planet::clipy(int y) const // This clips Y coordinates so they can't be off the map.
{
    if (y < 0)
        y = 0;

    if (y > itsheight)
        y = itsheight;

    return(y);
}

void planet::smooth(int arr[][ARRAYHEIGHT], int amount, bool vary, bool avoidmountains) // This smoothes the given array by the given amount.
{
    for (int i = 0; i <= itswidth; i++)
    {
        for (int j = 1; j < itsheight; j++)
        {
            if (avoidmountains == 0 || mountainheights[i][j] == 0)
            {
                int crount = 0;
                int ave = 0;

                for (int k = i - amount; k <= i + amount; k++)
                {
                    int kk = k;

                    if (kk < 0)
                        kk = itswidth;

                    if (kk > itswidth)
                        kk = 0;

                    for (int l = j - amount; l <= j + amount; l++)
                    {
                        ave = ave + (int)mapnom[kk][l];
                        crount++;
                    }
                }
                ave = ave / crount;

                if (ave > 0 && ave < itsmaxheight)
                    mapnom[i][j] = (short)ave;
            }
        }
    }
}

void planet::smooth(short arr[][ARRAYHEIGHT], int amount, bool vary, bool avoidmountains) // This smoothes the given array by the given amount.
{
    for (int i = 0; i <= itswidth; i++)
    {
        for (int j = 1; j < itsheight; j++)
        {
            if (avoidmountains == 0 || mountainheights[i][j] == 0)
            {
                int crount = 0;
                int ave = 0;

                for (int k = i - amount; k <= i + amount; k++)
                {
                    int kk = k;

                    if (kk < 0)
                        kk = itswidth;

                    if (kk > itswidth)
                        kk = 0;

                    for (int l = j - amount; l <= j + amount; l++)
                    {
                        ave = ave + (int)mapnom[kk][l];
                        crount++;
                    }
                }
                ave = ave / crount;

                if (ave > 0 && ave < itsmaxheight)
                    mapnom[i][j] = (short)ave;
            }
        }
    }
}

// This does the same, but only over land.

void planet::smoothoverland(int arr[][ARRAYHEIGHT], int amount, bool uponly)
{
    for (int i = 0; i <= itswidth; i++)
    {
        for (int j = 1; j < itsheight; j++)
        {
            if (sea(i, j) == 0)
            {
                int crount = 0;
                int ave = 0;

                for (int k = i - amount; k <= i + amount; k++)
                {
                    int kk = k;

                    if (kk < 0)
                        kk = itswidth;

                    if (kk > itswidth)
                        kk = 0;

                    for (int l = j - amount; l <= j + amount; l++)
                    {
                        ave = ave + arr[kk][l];
                        crount++;
                    }
                }

                if (crount > 0)
                {
                    ave = ave / crount;

                    if (ave > 0 && ave < itsmaxheight)
                    {
                        if (uponly == 0)
                            arr[i][j] = ave;
                        else
                        {
                            if (ave > arr[i][j])
                                arr[i][j] = ave;
                        }
                    }
                }
            }
        }
    }
}

void planet::smoothoverland(short arr[][ARRAYHEIGHT], int amount, bool uponly)
{
    for (int i = 0; i <= itswidth; i++)
    {
        for (int j = 1; j < itsheight; j++)
        {
            if (sea(i, j) == 0)
            {
                int crount = 0;
                int ave = 0;

                for (int k = i - amount; k <= i + amount; k++)
                {
                    int kk = k;

                    if (kk < 0)
                        kk = itswidth;

                    if (kk > itswidth)
                        kk = 0;

                    for (int l = j - amount; l <= j + amount; l++)
                    {
                        ave = ave + (int)arr[kk][l];
                        crount++;
                    }
                }

                if (crount > 0)
                {
                    ave = ave / crount;

                    if (ave > 0 && ave < itsmaxheight)
                    {
                        if (uponly == 0)
                            arr[i][j] = (short)ave;
                        else
                        {
                            if (ave > arr[i][j])
                                arr[i][j] = (short)ave;
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

// Functions for saving member variables.

template<typename T> void write_val(T const val, ostream& out) { // default
    out << val;
}
void write_int_val(int val, ostream& out) {
    if (val < 0) { out.put('-'); val = -val; } // negative
    if (val < 10) { out.put('0' + char(val)); } // 1 digit
    else if (val < 100) { out.put('0' + char(val / 10)); out.put('0' + char(val % 10)); } // 2 digits
    else if (val < 1000) { out.put('0' + char(val / 100)); out.put('0' + char((val / 10) % 10)); out.put('0' + char(val % 10)); } // 3 digits
    else { out << val; } // 4+ digits
}
void write_val(int   const val, ostream& out) { write_int_val(val, out); }
void write_val(short const val, ostream& out) { write_int_val(val, out); }

void write_val(bool const val, ostream& out) {
    out.put(val ? '1' : '0');
}

template<typename T> void planet::writevariable(ofstream& outfile, T val)
{
    write_val(val, outfile);
    outfile.put('\n');
}

// Functions for saving member arrays.

template<typename T> void planet::writedata(ofstream& outfile, T const arr[ARRAYWIDTH][ARRAYHEIGHT])
{
    for (int i = 0; i <= itswidth; i++)
    {
        for (int j = 0; j <= itsheight; j++)
        {
            write_val(arr[i][j], outfile);
            outfile.put('\n');
        }
    }
}

// Functions for loading member variables.

void read_val(string const& line, int& val)
{
    val = stoi(line);
}
void read_val(string const& line, bool& val)
{
    val = stob(line);
}
void read_val(string const& line, short& val)
{
    val = stos(line);
}
void read_val(string const& line, unsigned short& val)
{
    val = stous(line);
}
void read_val(string const& line, float& val)
{
    val = stof(line);
}
void read_val(string const& line, long& val)
{
    val = stol(line);
}
void read_val(string const& line, char& val)
{
    val = stoc(line);
}
void read_val(string const& line, unsigned char& val)
{
    val = stouc(line);
}

template<typename T> void planet::readvariable(ifstream& infile, T& val)
{
    getline(infile, line_for_file_read);
    read_val(line_for_file_read, val);
}

// Functions for loading member arrays.

template<typename T> void planet::readdata(ifstream& infile, T arr[ARRAYWIDTH][ARRAYHEIGHT])
{
    for (int i = 0; i <= itswidth; i++)
    {
        for (int j = 0; j <= itsheight; j++)
        {
            getline(infile, line_for_file_read);
            read_val(line_for_file_read, arr[i][j]);
        }
    }
}
