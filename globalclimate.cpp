//
//  globalclimate.cpp
//  Undiscovered Worlds
//
//  Created by Jonathan Hill on 04/09/2019.
//
//  Please see functions.hpp for notes.

#include <iostream>
#include <cmath>
#include <fstream>
#include <stdio.h>

#include "classes.hpp"
#include "planet.hpp"
#include "region.hpp"
#include "functions.hpp"
//#include "profiler.h"

using namespace std;

// This function creates the global climate.

void generateglobalclimate(planet& world, bool dorivers, bool dolakes, bool dodeltas, boolshapetemplate smalllake[], boolshapetemplate largelake[], boolshapetemplate landshape[], vector<vector<vector<int>>>& mountaindrainage, vector<vector<vector<bool>>>& shelves, vector<float>& longitude, vector<float>& latitude, vector<fourglobepoints>& dirpoint, int& progressval, string& progresstext)
{
    //highres_timer_t timer("Generate Global Climate"); // 9.4s => 8.2s
    long seed = world.seed();
    fast_srand(seed);

    int edge = world.edge();
    int maxelev = world.maxelevation();
    int sealevel = world.sealevel();
    int seatotal = world.seatotal();
    int landtotal = world.landtotal();

    // If there is no sea on a world, it may still have rain, provided we can find somewhere to put some salt lakes for the rivers to run into.

    int desertrainchance = 4; // On worlds with no sea, chance of trying to create rain anyway.
    int lakeattempts = random(1, 8); // On worlds with no sea where there may be rain, make this many attempts to place a salt lake.

    int saltlakesplaced = 0;

    // If there's no sea, we will probably raise the sea level to be just below the lowest land.

    int raisesealevelchance = 6; // The higher this is, the *more* likely we are to try this.

    if (seatotal == 0 && random(1, raisesealevelchance) != 1)
    {
        int lowest = maxelev;

        for (int face = 0; face < 6; face++)
        {
            for (int i = 0; i < edge; i++)
            {
                for (int j = 0; j < edge; j++)
                {
                    int thiselev = world.nom(face, i, j);

                    if (thiselev < lowest)
                        lowest = thiselev;
                }
            }
        }

        sealevel = lowest - random(5, 10);

        if (sealevel < 1)
            sealevel = 1;

        world.setsealevel(sealevel);
    }

    // If there is no sea but there is rain, we need to try to create some small bits of sea that will later become saltwater lakes, and fill depressions, to ensure that rivers run towards them.

    // First we need to prepare a no-lake template, marking out areas too close to the coasts, where lakes can't go. (We'll use this later whether or not the world has sea, so may as well do it now.)

    int minseadistance = 15; // Points closer to the shore than this can't be the centre of lakes, normally.
    int minseadistance2 = 8; // This is for any lake tile, not just the centre.

    vector<twointegers> saltlakemap(6 * edge * edge);
    vector<int> nolake(6 * edge * edge);
    vector<int> basins(6 * edge * edge);

    if (dolakes)
    {
        for (int face = 0; face < 6; face++)
        {
            int vface = face * edge * edge;

            for (int i = 0; i < edge; i++)
            {
                int vi = vface + i * edge;

                for (int j = 0; j < edge; j++)
                {
                    int index = vi + j;

                    saltlakemap[index].x = 0;
                    saltlakemap[index].y = 0;
                }
            }
        }

        for (int face = 0; face < 6; face++)
        {
            for (int i = 0; i < edge; i++)
            {
                for (int j = 0; j < edge; j++)
                {
                    if (world.outline(face, i, j) == 1)
                    {
                        for (int k = - minseadistance2; k <= minseadistance2; k++)
                        {
                            for (int l = -minseadistance2; l <= minseadistance2; l++)
                            {
                                globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                                if (lpoint.face != -1)
                                    nolake[lpoint.face * edge * edge + lpoint.x * edge + lpoint.y] = 1;
                            }
                        }

                        for (int k = -minseadistance2; k <= minseadistance2; k++)
                        {
                            for (int l = -minseadistance2; l <= minseadistance2; l++)
                            {
                                globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                                int lindex = lpoint.face * edge * edge + lpoint.x * edge + lpoint.y;

                                if (lpoint.face != -1 && nolake[lindex] == 0)
                                    nolake[lindex] = 2;
                            }
                        }
                    }
                }
            }
        }
    }

    bool desertworldrain = 0; // If this is 1, then this is a world with no sea but which will have rain.

    if (dorivers && seatotal == 0 && random(1, desertrainchance) == 1)
    {
        int highestelev = 0;
        int lowestelev = maxelev;

        for (int face = 0; face < 6; face++)
        {
            for (int i = 0; i < edge; i++)
            {
                for (int j = 0; j < edge; j++)
                {
                    int thiselev = world.nom(face, i, j);

                    if (thiselev < lowestelev)
                        lowestelev = thiselev;

                    if (thiselev > highestelev)
                        highestelev = thiselev;
                }
            }
        }

        int elevdiff = highestelev - lowestelev;

        int maxlakeelev = lowestelev + elevdiff / 4;

        vector<int> avoid(6 * edge * edge, 0); // This is for cells *not* to alter. This will remain empty (it's just here so we can use the depression-creating routine for normal lakes as well, where we don't want to mess with the path of the outflowing river.)

        for (int n = 0; n < lakeattempts; n++)
        {
            int face = random(0, 5);
            int x = random(1, edge - 1);
            int y = random(10, edge - 11);

            for (int m = 0; m < 1000; m++)
            {
                if (world.nom(face, x, y) > maxlakeelev)
                {
                    int face = random(0, 5);
                    x = random(1, edge - 1);
                    y = random(10, edge - 11);
                }
                else
                    m = 1000;
            }

            placesaltlake(world, face, x, y, 1, 0, saltlakemap, basins, avoid, nolake, smalllake);
        }

        bool foundsea = 0; // Now look to see whether that worked. If it didn't, then there will be no rain on this world.

        for (int face = 0; face < 6; face++)
        {
            for (int i = 0; i < edge; i++)
            {
                for (int j = 0; j < edge; j++)
                {
                    if (world.nom(face, i, j) <= sealevel)
                    {
                        desertworldrain = 1;
                        face = 6;
                        i = edge;
                        j = edge;
                    }
                }
            }
        }
    }

    if (desertworldrain)
    {
        // Now remove depressions.

        updatereport(progressval, progresstext, "Filling depressions");

        depressionfill(world);

        addlandnoise(world); // Add a bit of noise, then do remove depressions again. This is to add variety to the river courses.

        depressionfill(world);
    }

    // Now, set the river land reduce factor.

    float riverlandreduce = 20.0f * world.gravity() * world.gravity();
    world.setriverlandreduce((int)riverlandreduce);

    // Now, do the wind map.

    updatereport(progressval, progresstext, "Generating wind map");

    vector<bool> outsidehorse(6 * edge * edge, 0);

    createwindmap(world, outsidehorse, longitude, latitude);

    // Now create the temperature map.

    updatereport(progressval, progresstext, "Generating global temperature map");

    // Start by generating a new fractal map.

    int grain = 8; // Level of detail on this fractal map.
    float valuemod = 0.2f;
    float valuemod2 = 3.0f;

    vector<int> fractal(6 * edge * edge, 0);

    createfractal(fractal, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    createtemperaturemap(world, fractal, latitude);

    // Now do the sea ice

    if (seatotal > 0)
    {
        updatereport(progressval, progresstext, "Generating sea ice map");

        for (int face = 0; face < 6; face++)
        {
            int vface = face * edge * edge;

            for (int i = 0; i < edge; i++)
            {
                int vi = vface + i * edge;

                for (int j = 0; j < edge; j++)
                    fractal[vi + j] = 0;
            }
        }

        createfractal(fractal, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

        createseaicemap(world, fractal, longitude, latitude);

        // Work out the tidal ranges.

        updatereport(progressval, progresstext, "Calculating tides");

        createtidalmap(world, dirpoint);
    }

    // Now do rainfall.

    if (seatotal > 0 || desertworldrain)
    {
        for (int face = 0; face < 6; face++)
        {
            int vface = face * edge * edge;

            for (int i = 0; i < edge; i++)
            {
                int vi = vface + i * edge;

                for (int j = 0; j < edge; j++)
                    fractal[vi + j] = 0;
            }
        }

        createfractal(fractal, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

        createrainmap(world, fractal, landtotal, seatotal, outsidehorse, smalllake, landshape, longitude, latitude, dirpoint, progressval, progresstext);
    }

    // Now add fjord mountains.

    if (seatotal > 0)
    {
        updatereport(progressval, progresstext, "Carving fjords");

        addfjordmountains(world);
    }

    if (dorivers && (seatotal > 0 || desertworldrain))
    {
        // Now work out the rivers initially. We do this the first time so that after the first time we can place the salt lakes in appropriate places, and then we work out the rivers again.

        updatereport(progressval, progresstext, "Planning river courses");

        createrivermap(world, mountaindrainage, dirpoint);

        if (dolakes || seatotal == 0) // If there's no sea we need at least one salt lake.
        {
            // Now create salt lakes.

            updatereport(progressval, progresstext, "Placing hydrological basins");

            createsaltlakes(world, saltlakesplaced, saltlakemap, nolake, basins, smalllake);

            addlandnoise(world);
            depressionfill(world);

            for (int face = 0; face < 6; face++)
            {
                for (int i = 0; i < edge; i++)
                {
                    for (int j = 0; j < edge; j++)
                    {
                        world.setriverdir(face, i, j, 0);
                        world.setriverjan(face, i, j, 0);
                        world.setriverjul(face, i, j, 0);
                    }
                }
            }

            // Now work out the rivers again.

            updatereport(progressval, progresstext, "Generating rivers");

            createrivermap(world, mountaindrainage, dirpoint);
        }

        // Now check river valleys in mountains.

        updatereport(progressval, progresstext, "Checking mountain river valleys");

        removerivermountains(world);

        if (dolakes)
        {
            // Now create the lakes.

            updatereport(progressval, progresstext, "Generating lakes");

            convertsaltlakes(world, saltlakemap);

            createlakemap(world, nolake, smalllake, largelake, dirpoint);

            createriftlakemap(world, nolake);
        }
        else
        {
            if (seatotal == 0)
                convertsaltlakes(world, saltlakemap);
        }
    }

    world.setmaxriverflow();

    // Now correct places where something has gone wrong with seasonal rainfall.

    updatereport(progressval, progresstext, "Correcting rainfall");

    correctseasonalrainfall(world);

    // Now create the climate map.

    updatereport(progressval, progresstext, "Calculating climates");

    createclimatemap(world);

    // Now specials.

    updatereport(progressval, progresstext, "Generating sand dunes");

    createergs(world, smalllake, largelake, landshape);

    updatereport(progressval, progresstext, "Generating salt pans");

    createsaltpans(world, smalllake, largelake);

    // Add river deltas. (off for now as it doesn't work properly)

    if (1 == 0) //(dodeltas)
    {
        updatereport(progressval, progresstext, "Generating river deltas");

        createriverdeltas(world);
        checkrivers(world); // Turned off as it seems buggy.
    }

    // Now wetlands.

    updatereport(progressval, progresstext, "Generating wetlands");

    createwetlands(world, smalllake);

    removeexcesswetlands(world);

    // Now it's time to finesse the roughness map.

    updatereport(progressval, progresstext, "Refining roughness map");

    refineroughnessmap(world);

    // Check the rift lake map too.

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.lakestart(face, i, j) == 1 && world.riftlakesurface(face, i, j) == 0 && world.lakesurface(face, i, j) == 0)
                    world.setlakestart(face, i, j, 0);
            }
        }
    }

    if (dolakes == 0)
    {
        for (int face = 0; face < 6; face++)
        {
            for (int i = 0; i < edge; i++)
            {
                for (int j = 0; j < edge; j++)
                {
                    if (world.special(face, i, j) != 110)
                    {
                        world.setlakesurface(face, i, j, 0);
                        world.setlakestart(face, i, j, 0);
                        world.setriftlakesurface(face, i, j, 0);
                        world.setriftlakebed(face, i, j, 0);
                    }
                }
            }
        }
    }

    removesealakes(world); // Also, make sure there are no weird bits of sea next to lakes.

    connectlakes(world); // Make sure lakes aren't fragmented.

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++) // Check erg/salt pans are the right depth.
        {
            for (int j = 0; j < edge; j++)
            {
                int special = world.special(face, i, j);

                if (special == 110 || special == 120)
                {
                    int level = world.lakesurface(face, i, j);

                    if (level <= sealevel)
                    {
                        level = sealevel + 1;
                        world.setlakesurface(face, i, j, level);
                    }

                    world.setnom(face, i, j, level);
                }
            }
        }
    }

    for (int face = 0; face < 6; face++) // Remove any craters where there are special features such as ergs or salt pans.
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.special(face, i, j) != 0)
                {
                    world.setcraterrim(face, i, j, 0);
                    world.setcratercentre(face, i, j, 0);
                }
            }
        }
    }

    // Now check for edge artefacts.

    removeedgeartefacts(world);

    // Now check the coastal temperatures.

    updatereport(progressval, progresstext, "Checking coastal temperatures");

    correctcoastaltemperatures(world);
}

// This creates the climate map.

void createclimatemap(planet& world)
{
    int edge = world.edge();

    short climate;

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                climate = getclimate(world, face, i, j);

                world.setclimate(face, i, j, climate);
            }
        }
    }
}

// This returns the climate type of the given point.

short getclimate(planet& world, int face, int x, int y)
{
    if (world.sea(face, x, y) == 1)
        return (0);

    int elev = world.map(face, x, y);
    int sealevel = world.sealevel();


    float wrain = (float)world.winterrain(face, x, y);
    float srain = (float)world.summerrain(face, x, y);
    float mintemp = (float)world.mintemp(face, x, y);
    float maxtemp = (float)world.maxtemp(face, x, y);

    short climate = calculateclimate(elev, sealevel, wrain, srain, mintemp, maxtemp);

    return climate;
}

// The same thing, but for the regional map.

short getclimate(region& region, int x, int y)
{
    if (region.sea(x, y) == 1)
        return (0);

    int elev = region.map(x, y);
    int sealevel = region.sealevel();

    float wrain = (float)region.winterrain(x, y);
    float srain = (float)region.summerrain(x, y);
    float mintemp = (float)region.mintemp(x, y);
    float maxtemp = (float)region.maxtemp(x, y);

    short climate = calculateclimate(elev, sealevel, wrain, srain, mintemp, maxtemp);

    return climate;
}

// This does the actual climate calculations for the above two functions.

short calculateclimate(int elev, int sealevel, float wrain, float srain, float mintemp, float maxtemp)
{
    float totalannualrain = 0;

    if (srain > wrain) // If there's more rain in summer, assume a shorter rainy season. The greater the imbalance, the shorter the season.
    {
        float factor = (float)wrain / (float)srain; // 0-1. The higher it is, the more balanced the distribution of rain.

        factor = factor * 12.0f;

        if (factor < 4.0)
            factor = 4.0f;

        int sfactor = (int)factor;

        int wfactor = 12 - sfactor;

        totalannualrain = wrain * wfactor + srain * sfactor;
    }
    else
        totalannualrain = (wrain + srain) * 6;

    float meanannualrain = (wrain + srain) / 2;
    float meanannualtemp = (mintemp + maxtemp) / 2;

    float minrain = wrain;
    float maxrain = srain;

    if (wrain > srain)
    {
        minrain = srain;
        maxrain = wrain;
    }

    string group, preptype, heattype; // These three variables define each climate type.

    // First, establish the group (first letter).

    if (maxtemp <= 10)
    {
        if (maxtemp >= 0)
            group = "ET";

        if (maxtemp < 0)
            group = "EF";
    }

    if (group == "")
    {
        float precthreshold = (maxtemp + mintemp) * 10;
        float checkpercent = (srain * 6) / totalannualrain;

        if (checkpercent >= 0.7f)
            precthreshold = precthreshold + 280.0f;

        if (checkpercent >= 0.3 && checkpercent < 0.7f)
            precthreshold = precthreshold + 140.0f;

        if (totalannualrain < precthreshold * 0.5f)
            group = "BW";

        if (totalannualrain >= precthreshold * 0.5 && totalannualrain <= precthreshold)
            group = "BS";
    }

    if (group == "" && mintemp >= 18)
        group = "A";

    if (group == "" && mintemp > -3 && mintemp < 18)
        group = "C";

    if (group == "" && mintemp <= -3)
        group = "D";

    // Now, establish the precipitation type (second letter).

    if (group == "A")
    {
        if (minrain >= 60)
            preptype = "f";

        if (preptype == "" && minrain >= (100 - (totalannualrain / 25))) //  meanannualrain>=(100-minrain))
            preptype = "m";

        if (preptype == "" && srain < 60)
            preptype = "s";

        if (preptype == "" && wrain < 60)
            preptype = "w";
    }

    if (group == "C" || group == "D")
    {
        if (srain < wrain / 3 && srain < 40)
            preptype = "s";

        if (preptype == "" && wrain < srain / 10)
            preptype = "w";

        if (preptype == "")
            preptype = "f";
    }

    // Now, establish the heat type (third letter).

    if (group == "BW" || group == "BS")
    {
        if (meanannualtemp >= 18)
            heattype = "h";

        if (meanannualtemp < 18)
            heattype = "k";
    }

    if (group != "A" && group != "BW" && group != "BS" && group != "ET" && group != "EF")
    {
        if (heattype == "" && maxtemp >= 22)
            heattype = "a";

        if (heattype == "" && maxtemp < 22 && maxtemp >= 14) // Should be: at least four months are >=10, but we can't track that accurately on our model.
            heattype = "b";

        if (heattype == "" && mintemp <= -38)
            heattype = "d";

        if (heattype == "")
            heattype = "c";
    }

    string climate = group + preptype + heattype;

    if (climate == "Af") // Af
        return 1;

    if (climate == "Am") // Am
        return 2;

    if (climate == "Aw") // Aw
        return 3;

    if (climate == "As") // As
        return 4;

    if (climate == "BWh") // Bwh
        return 5;

    if (climate == "BWk") // BWk
        return 6;

    if (climate == "BSh") // BSh
        return 7;

    if (climate == "BSk") // BSk
        return 8;

    if (climate == "Csa") // Csa
        return 9;

    if (climate == "Csb") // Csb
        return 10;

    if (climate == "Csc") // Csc
        return 11;

    if (climate == "Cwa") // Cwa
        return 12;

    if (climate == "Cwb") // Cwb
        return 13;

    if (climate == "Cwc") // Cwc
        return 14;

    if (climate == "Cfa") // Cfa
        return 15;

    if (climate == "Cfb") // Cfb
        return 16;

    if (climate == "Cfc") // Cfc
        return 17;

    if (climate == "Dsa") // Dsa
        return 18;

    if (climate == "Dsb") // Dsb
        return 19;

    if (climate == "Dsc") // Dsc
        return 20;

    if (climate == "Dsd") // Dsd
        return 21;

    if (climate == "Dwa") // Dwa
        return 22;

    if (climate == "Dwb") // Dwb
        return 23;

    if (climate == "Dwc") // Dwc
        return 24;

    if (climate == "Dwd") // Dwd
        return 25;

    if (climate == "Dfa") // Dfa
        return 26;

    if (climate == "Dfb") // Dfb
        return 27;

    if (climate == "Dfc") // Dfc
        return 28;

    if (climate == "Dfd") // Dfd
        return 29;

    if (climate == "ET") // ET
        return 30;

    if (climate == "EF") // EF
        return 31;

    return 0;
}

// This function gives the names of the climate types.

string getclimatename(short climate)
{
    if (climate == 1) // Af
        return "Tropical rainforest";

    if (climate == 2) // Am
        return "Monsoon";

    if (climate == 3) // Aw
        return "Savannah";

    if (climate == 4) // As
        return "Savannah";

    if (climate == 5) // Bwh
        return "Hot desert";

    if (climate == 6) // BWk
        return "Cold desert";

    if (climate == 7) // BSh
        return "Hot semi-arid";

    if (climate == 8) // BSk
        return "Cold steppe";

    if (climate == 9) // Csa
        return "Hot, dry-summer Mediterranean";

    if (climate == 10) // Csb
        return "Warm, dry-summer Mediterranean";

    if (climate == 11) // Csc
        return "Cold, dry-summer Mediterranean";

    if (climate == 12) // Cwa
        return "Dry-winter humid subtropical";

    if (climate == 13) // Cwb
        return "Dry-winter subtropical highland";

    if (climate == 14) // Cwc
        return "Dry-winter subpolar oceanic";

    if (climate == 15) // Cfa
        return "Humid subtropical";

    if (climate == 16) // Cfb
        return "Temperate oceanic";

    if (climate == 17) // Cfc
        return "Subpolar oceanic";

    if (climate == 18) // Dsa
        return "Mediterranean-influenced hot-summer humid continental";

    if (climate == 19) // Dsb
        return "Mediterranean-influenced warm-summer humid continental";

    if (climate == 20) // Dsc
        return "Mediterranean-influenced subarctic";

    if (climate == 21) // Dsd
        return "Mediterranean-influenced extremely cold subarctic";

    if (climate == 22) // Dwa
        return "Monsoon-influenced hot-summer humid continental";

    if (climate == 23) // Dwb
        return "Monsoon-influenced warm-summer humid continental";

    if (climate == 24) // Dwc
        return "Monsoon-influenced subarctic";

    if (climate == 25) // Dwd
        return "Monsoon-influenced extremely cold subarctic";

    if (climate == 26) // Dfa
        return "Hot-summer humid continental";

    if (climate == 27) // Dfb
        return "Warm-summer humid continental";

    if (climate == 28) // Dfc
        return "Subarctic";

    if (climate == 29) // Dfd
        return "Extremely cold subarctic";

    if (climate == 30) // ET
        return "Tundra";

    if (climate == 31) // EF
        return "Frost";

    return "";
}

// This function gives the codes of the climate types.

string getclimatecode(short climate)
{
    if (climate == 1) // Af
        return "Af";

    if (climate == 2) // Am
        return "Am";

    if (climate == 3) // Aw
        return "Aw";

    if (climate == 4) // As
        return "As";

    if (climate == 5) // Bwh
        return "BWh";

    if (climate == 6) // BWk
        return "BWk";

    if (climate == 7) // BSh
        return "BSh";

    if (climate == 8) // BSk
        return "BSk";

    if (climate == 9) // Csa
        return "Csa";

    if (climate == 10) // Csb
        return "Csb";

    if (climate == 11) // Csc
        return "Csc";

    if (climate == 12) // Cwa
        return "Cwa";

    if (climate == 13) // Cwb
        return "Cwb";

    if (climate == 14) // Cwc
        return "Cwc";

    if (climate == 15) // Cfa
        return "Cfa";

    if (climate == 16) // Cfb
        return "Cfb";

    if (climate == 17) // Cfc
        return "Cfc";

    if (climate == 18) // Dsa
        return "Dsa";

    if (climate == 19) // Dsb
        return "Dsb";

    if (climate == 20) // Dsc
        return "Dsc";

    if (climate == 21) // Dsd
        return "Dsd";

    if (climate == 22) // Dwa
        return "Dwa";

    if (climate == 23) // Dwb
        return "Dwb";

    if (climate == 24) // Dwc
        return "Dwc";

    if (climate == 25) // Dwd
        return "Dwd";

    if (climate == 26) // Dfa
        return "Dfa";

    if (climate == 27) // Dfb
        return "Dfb";

    if (climate == 28) // Dfc
        return "Dfc";

    if (climate == 29) // Dfd
        return "Dfd";

    if (climate == 30) // ET
        return "ET";

    if (climate == 31) // EF
        return "EF";

    return "";
}

// This function creates areas of sea that will later become salt lakes.

void createsaltlakes(planet& world, int& lakesplaced, vector<twointegers>& saltlakemap, vector<int>& nolake, vector<int>& basins, boolshapetemplate smalllake[])
{
    int edge = world.edge();

    int maxsaltrain = 30; // Salt lakes may form if there is less rain than this.
    int minsalttemp = 15; // Salt lakes require more heat than this.
    int minflow = 200; // Salt lakes can only form on a river of this size or greater.

    int saltlakechance = 100; //1000;

    bool dodepressions = 0;

    if (random(1, 100) == 1) // Only do this very occasionally, as it has the odd side-effect of turning high ground in terrain 4-type worlds into plateaux. Which is good as an occasional thing but not all the time!
        dodepressions = 1;

    vector<int> avoid(6 * edge * edge, 0);

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 20; i < edge - 20; i++) // Don't want them too close to the edges, because it won't be possible to make lake start points if they wrap over the edges.
        {
            int vi = vface + i * edge;

            for (int j = 20; j < edge - 20; j++)
            {
                int index = vi + j;

                if (basins[index] == 0 && nolake[index] == 0 && world.sea(face, i, j) == 0 && world.riverjan(face, i, j) + world.riverjul(face, i, j) >= minflow)
                {
                    if (random(1, saltlakechance) == 1)
                    {
                        int averain = (world.winterrain(face, i, j) + world.summerrain(face, i, j)) / 2;
                        int avetemp = (world.maxtemp(face, i, j) + world.mintemp(face, i, j)) / 2;

                        if (averain<maxsaltrain && avetemp>minsalttemp)
                        {
                            lakesplaced++;
                            placesaltlake(world, face, i, j, 0, dodepressions, saltlakemap, basins, avoid, nolake, smalllake);
                        }
                    }
                }
            }
        }
    }
}

// This function puts a patch of sea on the map that will later be turned into a salt lake.

void placesaltlake(planet& world, int face, int centrex, int centrey, bool large, bool dodepressions, vector<twointegers>& saltlakemap, vector<int>& basins, vector<int>& avoid, vector<int>& nolake, boolshapetemplate smalllake[])
{
    int edge = world.edge();
    int width = edge - 1;
    int height = edge - 1;
    int sealevel = world.sealevel();
    int riverlandreduce = world.riverlandreduce();

    int vface = face * edge * edge;

    int shapenumber;

    if (large || random(1, 8) == 1)
        shapenumber = random(5, 11);
    else
        shapenumber = random(2, 5);

    int depth = random(5, 30);

    // The surfaceheight will be a lot lower than this point originally is.

    int origheight = world.nom(face, centrex, centrey);
    int surfaceheight = (sealevel + (origheight - sealevel) / 2) - riverlandreduce;

    if (surfaceheight <= sealevel)
        surfaceheight = sealevel + 1;

    int imheight = smalllake[shapenumber].ysize() - 1;
    int imwidth = smalllake[shapenumber].xsize() - 1;

    int x = centrex - imwidth / 2; // Coordinates of the top left corner.
    int y = centrey - imheight / 2;

    if (x<0 || x>width || y<0 || y>height)
        return;

    if (x + imwidth > width || y + imheight > height)
        return;

    int leftr = random(0, 1); // If it's 1 then we reverse it left-right
    int downr = random(0, 1); // If it's 1 then we reverse it top-bottom

    int istart = 0, desti = imwidth + 1, istep = 1;
    int jstart = 0, destj = imheight + 1, jstep = 1;

    if (leftr == 1)
    {
        istart = imwidth;
        desti = -1;
        istep = -1;
    }

    if (downr == 1)
    {
        jstart = imwidth;
        destj = -1;
        jstep = -1;
    }

    int leftx = centrex;
    int lefty = centrey;
    int rightx = centrex;
    int righty = centrey; // These are the coordinates of the furthermost pixels of the lake.
    bool wrapped = 0; // If this is 1, the lake wraps over the edge of the map.

    bool tooclose = 0;

    int mapi = -1;
    int mapj = -1;

    for (int i = istart; i != desti; i = i + istep)
    {
        mapi++;
        mapj = -1;

        for (int j = jstart; j != destj; j = j + jstep)
        {
            mapj++;

            if (smalllake[shapenumber].point(i, j) == 1)
            {
                int xx = x + mapi;
                int yy = y + mapj;

                if (nolake[vface + xx * edge + yy] == 1 || world.nom(face, xx, yy) <= sealevel) // Don't try to put lakes on top of sea...
                    tooclose = 1;
                else
                {
                    if (surfaceheight > world.nom(face,xx, yy) - riverlandreduce) // Lakes can't extend onto areas where local rivers will be lower than the surface of the lake.
                        tooclose = 1;
                    else
                    {

                        for (int k = xx - 2; k <= xx + 2; k++) // Check nearby cells for other lakes.
                        {
                            if (k >= 0 && k <= width)
                            {
                                int vk = vface + k * edge;

                                for (int l = yy - 2; l <= yy + 2; l++)
                                {
                                    if (l >= 0 && l <= height)
                                    {
                                        int lindex = vk + l;

                                        if (saltlakemap[lindex].x != 0 && saltlakemap[lindex].x != surfaceheight)
                                            tooclose = 1;

                                        if (world.nom(face,k, l) <= sealevel && saltlakemap[lindex].x != surfaceheight)
                                            tooclose = 1;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (tooclose == 1)
        return;

    // Now actually do the lake.

    mapi = -1;
    mapj = -1;

    for (int i = istart; i != desti; i = i + istep)
    {
        mapi++;
        mapj = -1;

        for (int j = jstart; j != destj; j = j + jstep)
        {
            mapj++;

            if (smalllake[shapenumber].point(i, j) == 1)
            {
                int xx = x + mapi;
                int yy = y + mapj;

                int index = vface + xx * edge + yy;

                world.setnom(face,xx, yy, sealevel - 1);
                saltlakemap[index].x = surfaceheight;
                saltlakemap[index].y = surfaceheight - (depth + randomsign(random(0, 4)));

                if (saltlakemap[index].y < 1)
                    saltlakemap[index].y = 1;

                if (xx < leftx)
                    leftx = xx;

                if (xx > rightx)
                    rightx = xx;

                if (yy < lefty)
                    lefty = yy;
                if (yy > righty)
                    righty = yy;

                if (world.mountainheight(face, xx, yy) != 0) // Remove any mountains that might be here.
                {
                    for (int dir = 1; dir <= 8; dir++)
                        deleteridge(world, face, xx, yy, dir);
                }
            }
        }
    }

    if (wrapped == 1)
    {
        leftx = 0;
        lefty = 0;
        rightx = width;
        righty = height;
    }

    // Now we need to lower the land around this sea/lake, to ensure that all rivers for a wide radius will flow into it.

    int steepness = 20; // Steepness of the basin around the lake.
    int limit = 0; // size*3 ` Maximum extent of the basin.

    if (dodepressions)
        createlakedepression(world, face, centrex, centrey, surfaceheight, steepness, basins, limit, 0, avoid);

    // Now we need to create a "start point" on the sea/lake.

    twointegers startpoint;
    startpoint.x = -1;
    startpoint.y = -1;

    int crount = 0;

    do
    {
        for (int i = leftx; i <= rightx; i++)
        {
            int vi = vface + i * edge;

            for (int j = lefty; j <= righty; j++)
            {
                if (saltlakemap[vi + j].x == surfaceheight)
                {
                    if (vaguelycoastal(world, face, i, j) == 1) // If this is on the edge of the sea/lake
                    {
                        if (random(1, 3) == 1)
                        {
                            startpoint.x = i;
                            startpoint.y = j;
                        }
                    }
                }
            }
        }

        crount++;

        if (crount > 1000) // Something's gone horribly wrong
            return;

    } while (startpoint.x == -1);

    world.setlakestart(face, startpoint.x, startpoint.y, 1);
}

// This function lowers land around a given point to put it at the centre of a large depression.

void createlakedepression(planet& world, int face, int centrex, int centrey, int origlevel, int steepness, vector<int>& basins, int limit, bool up, vector<int>& avoid)
{
    int edge = world.edge();
    int width = edge - 1;
    int height = edge - 1;

    int vface = face * edge * edge;

    if (limit == 0)
        limit = 1000;

    int maxmountains = 200; // Mountains larger than this will be left alone.

    for (int radius = 1; radius <= limit; radius++)
    {
        int numberchanged = 0;

        int currentlevel = origlevel + (steepness * (radius - 1)); // Cells checked on this round must be no higher than this.

        for (int i = centrex - radius; i <= centrex + radius; i++)
        {
            if (i >= 0 && i <= width)
            {
                int vi = vface + i * edge;

                for (int j = centrey - radius; j <= centrey + radius; j++)
                {
                    if (j >= 0 && j <= height)
                    {
                        if (up == 1 || world.nom(face,i, j) > currentlevel) // If this cell is higher than the current limit (or if we're raising cells as well as lowering them)
                        {
                            int index = vi + j;

                            if (basins[index] == 0 && avoid[index] == 0 && world.sea(face,i, j) == 0 && world.truelake(face,i, j) == 0 && world.mountainheight(face,i, j) < maxmountains)
                            {
                                if ((i - centrex) * (i - centrex) + (j - centrey) * (j - centrey) <= radius * radius) // If this cell is within the current radius
                                {
                                    int thislevel = currentlevel; // The level of this particular cell, which will be a variation on the current level of the whole radius.

                                    if (radius > 1 && up == 0) // Add a bit of variation to the slope.
                                        thislevel = thislevel - random(0, steepness);

                                    world.setnom(face, i, j, thislevel);
                                    numberchanged++;
                                    basins[index] = 1;
                                }
                            }
                        }
                    }
                }
            }
        }

        if (numberchanged == 0)
            return;
    }
}

// This function turns the inland seas into proper salt lakes.

void convertsaltlakes(planet& world, vector<twointegers>& saltlakemap)
{
    int edge = world.edge();

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                if (saltlakemap[index].x != 0)
                {
                    world.setlakesurface(face, i, j, saltlakemap[index].x);
                    world.setnom(face, i, j, saltlakemap[index].y);

                    world.setspecial(face, i, j, 100);
                }
            }
        }
    }
}

// Wind map creator.

void createwindmap(planet& world, vector<bool>& outsidehorse, vector<float>& longitude, vector<float>& latitude)
{
    int edge = world.edge();

    int wwidth = edge * 4 ;
    int hheight = edge * 2 ;

    int width = wwidth - 1;
    int height = hheight - 1;

    bool rotation = world.rotation();

    vector<int> flatwindmap(wwidth * hheight, 0); // We will draw the lines of wind on this array and then wrap it onto the globe at the end.

    int borders[11]; // Holds the starting latitudes of the different wind zone borders.

    borders[1] = 28; // Bottom of polar easterlies
    borders[2] = 32; // Top of prevailing westerlies
    borders[3] = 58; // Horse 1
    borders[4] = 64; // Horse 2
    borders[5] = 86; // Bottom of trade winds
    borders[6] = 94; // Top of southern trade winds
    borders[7] = 116;// Horse 3
    borders[8] = 122;// Horse 4
    borders[9] = 148;// Bottom of prevailing westerlies
    borders[10] = 152;// Top of polar easterlies

    int variation = random(1,10); // random(10, 30); //random(8,10); // Maximum amount a node can vary from the base amount.
    float div = height / 180.0f; // This is the amount that corresponds to each degree latitude

    int linecol = 1000000; // Value for the lines between wind zones.
    int pets = 20; // This is the step width between each node.

    int n = width / pets; // This is how many nodes there will be.
    int nn = n + 1;

    vector<int> windlines(12 * nn, 0); // 0 holds the x coordinate of that node. 1 to 10 hold the y coordinates of the different lines.

    for (int i = 0; i <= n; i++)
        windlines[i] = i * pets;

    for (int i = 0; i <= width; i++)
    {
        int vi = i * nn;

        for (int j = 0; j <= height; j++)
            flatwindmap[vi + j] = -1;
    }

    // First we fill in the basic y coordinate of each node.

    for (int i = 0; i <= n; i++)
    {
        for (int line = 1; line <= 10; line++)
        {
            float y = (float)borders[line] * div;

            windlines[line * nn + i] = (int)y;
        }
    }

    for (int line = 1; line <= 10; line++)
    {
        float y = (float)borders[line] * div;

        windlines[line * nn] = (int)y;
    }

    int maxoffset = 25;
    int maxoffsetchange = random(1, 3); // 3;
    int yoffsetchange = 0;
    int yoffset = 0;
    int maxextraoffset = 5;

    // Now we adjust the y coordinates of each node.

    for (int i = 0; i <= n; i++)
    {
        if (yoffset > 0)
        {
            if (random(0, maxoffset) < yoffset)
                yoffsetchange = yoffsetchange - random(0, maxoffsetchange);
            else
                yoffsetchange = yoffsetchange + random(0, maxoffsetchange);
        }
        else
        {
            if (random(0, maxoffset) < abs(yoffset))
                yoffsetchange = yoffsetchange + random(0, maxoffsetchange);
            else
                yoffsetchange = yoffsetchange - random(0, maxoffsetchange);
        }

        yoffset = yoffset + yoffsetchange;

        int yextraoffset = randomsign(random(1, maxextraoffset));

        if (yoffset > maxoffset)
            yoffset = maxoffset;

        if (yoffset < -maxoffset)
            yoffset = -maxoffset;

        for (int line = 1; line <= 10; line++)
        {
            int vline = line * nn;

            int jj = windlines[vline + i] + yoffset + yextraoffset;

            if (yoffset != 0)
            {
                int amount2 = yoffset / 2;

                if (line < 10) // Move all the lower ones accordingly.
                {
                    for (int line2 = line + 1; line2 <= 10; line2++)
                    {
                        int vline2 = line2 * nn;

                        if (yoffset > 1)
                        {
                            amount2 = (yoffset - (line2 - line)) / 2;
                            if (amount2 < 0)
                                amount2 = 0;
                        }
                        else
                        {
                            amount2 = (yoffset + (line2 - line)) / 2;
                            if (amount2 > 0)
                                amount2 = 0;
                        }

                        int jjj = windlines[vline2 + i];
                        jjj = jjj + amount2;
                        windlines[vline2 + i] = jjj + randomsign(random(1, maxextraoffset));
                    }
                }

                if (line > 1) // Move all the higher ones accordingly.
                {
                    for (int line2 = 1; line2 <= line - 1; line2++)
                    {
                        int vline2 = line2 * nn;

                        if (yoffset > 1)
                        {
                            amount2 = (yoffset - (line - line2)) / 2;
                            if (amount2 < 0)
                                amount2 = 0;
                        }
                        else
                        {
                            amount2 = (yoffset + (line - line2)) / 2;
                            if (amount2 > 0)
                                amount2 = 0;
                        }

                        int jjj = windlines[vline2 + i];
                        jjj = jjj + amount2;
                        windlines[vline2 + i] = jjj + randomsign(random(1, maxextraoffset));
                    }
                }
            }
        }
    }

    // Now make sure none of them overlaps.

    for (int i = 0; i <= n; i++)
    {
        for (int line = 2; line <= 10; line++)
        {
            int vline = line * nn;

            if (windlines[vline + i] < windlines[vline - nn + i] + 4)
                windlines[vline + i] = windlines[vline - nn + i] + 4;
        }
    }

    // Now we draw lines between the nodes.

    twofloats pt, mm1, mm2, mm3, mm4;

    for (int m1 = 0; m1 < n; m1++) // (int m1=0; m1<=n; m1++)
    {
        int m2 = wrap(m1 + 1, n - 1);
        int m3 = wrap(m1 + 2, n - 1);
        int m4 = wrap(m1 + 3, n - 1);

        for (int line = 1; line <= 10; line++)
        {
            int vline = line * nn;

            mm1.x = (float)windlines[m1];
            mm1.y = (float)windlines[vline + m1];

            mm2.x = (float)windlines[m2];
            mm2.y = (float)windlines[vline + m2];

            mm3.x = (float)windlines[m3];
            mm3.y = (float)windlines[vline + m3];

            mm4.x = (float)windlines[m4];
            mm4.y = (float)windlines[vline + m4];

            if (mm2.x < mm1.x) // This dewraps them all, so some may extend beyond the eastern edge of the map.
                mm2.x = (float)mm2.x + width;

            if (mm3.x < mm2.x)
                mm3.x = (float)mm3.x + width;

            if (mm4.x < mm3.x)
                mm4.x = (float)mm4.x + width;

            for (float t = 0.0f; t <= 1.0f; t = t + 0.01f)
            {
                pt = curvepos(mm1, mm2, mm3, mm4, t);

                int x = (int)pt.x;
                int y = (int)pt.y;

                if (x<0 || x>width)
                    x = wrap(x, width);

                int col = linecol + line;

                if (y >= 0 && y <= height)
                    flatwindmap[x * hheight + y] = col;
            }
        }
    }

    // Now we fill in the wind directions behind the lines.

    int winddir[11][3];

    for (int i = 0; i < 11; i++)
    {
        for (int j = 0; j < 3; j++)
            winddir[i][j] = 0;
    }

    winddir[0][0] = -1;
    winddir[0][1] = 2;
    winddir[0][2] = 6;

    winddir[1][0] = 0;
    winddir[1][1] = 0;
    winddir[1][2] = 0;

    winddir[2][0] = 1;
    winddir[2][1] = 4;
    winddir[2][2] = 10;

    winddir[3][0] = 0;
    winddir[3][1] = 0;
    winddir[3][2] = 0;

    winddir[4][0] = -1;
    winddir[4][1] = 5;
    winddir[4][2] = 10;

    winddir[5][0] = -1;
    winddir[5][1] = 1;
    winddir[5][2] = 4;

    winddir[6][0] = -1;
    winddir[6][1] = 5;
    winddir[6][2] = 10;

    winddir[7][0] = 0;
    winddir[7][1] = 0;
    winddir[7][2] = 0;

    winddir[8][0] = 1;
    winddir[8][1] = 4;
    winddir[8][2] = 10;

    winddir[9][0] = 0;
    winddir[9][1] = 0;
    winddir[9][2] = 0;

    winddir[10][0] = -1;
    winddir[10][1] = 2;
    winddir[10][2] = 6;

    // Now set out the horse latitudes.

    vector<int> horselats(wwidth * 6, 0);

    for (int i = 0; i <= width; i++)
    {
        int vi = i * hheight;
        int hvi = i * 6;

        int zone = 0;

        for (int j = 0; j <= height; j++)
        {
            int index = vi + j;

            if (flatwindmap[index] >= linecol)
            {
                zone = flatwindmap[index] - linecol;

                if (zone > 10)
                    zone = 10;

                if (zone == 3)
                    horselats[hvi + 1] = j;

                if (zone == 4)
                    horselats[hvi + 2] = j;

                if (zone == 7)
                    horselats[hvi + 3] = j;

                if (zone == 8)
                    horselats[hvi + 4] = j;
            }
            else
            {
                int wind;

                if (winddir[zone][0] == 0)
                    wind = 0;
                else
                {
                    if (rotation == 1)
                    {
                        if (winddir[zone][0] == 1)
                            wind = 10;
                        else
                            wind = -10;
                    }
                    else
                    {
                        if (winddir[zone][0] == 1)
                            wind = -10;
                        else
                            wind = 10;
                    }
                }

                flatwindmap[index] = wind;
            }
        }
    }

    // Now we clean up the edge.

    for (int j = 0; j <= height; j++)
        flatwindmap[j] = flatwindmap[width * hheight + j];

    // Now we remove the borders.

    for (int i = 0; i <= width; i++)
    {
        int vi = i * hheight;

        for (int j = 0; j <= height; j++)
        {
            int index = wwidth + j;

            if (flatwindmap[index] >= linecol)
            {
                bool alldone = 0;
                int jj = j;

                do
                {
                    jj--;

                    if (flatwindmap[vi + jj] < linecol)
                    {
                        flatwindmap[index] = flatwindmap[vi + jj];
                        alldone = 1;
                    }

                } while (alldone == 0);
            }
        }
    }

    // Now we need to put provisional values in the zones without winds.

    for (int i = 0; i <= width; i++)
    {
        int vi = i * hheight;

        for (int j = 0; j <= height; j++)
        {
            int index = vi + j;

            if (flatwindmap[index] == 0)
            {
                int up = 0;
                bool found = 0;
                int jj = j;
                bool northpositive = 0;

                do
                {
                    if (jj >= 0 && jj <= height)
                    {
                        int jindex = vi + jj;

                        if (flatwindmap[jindex] != 0 && flatwindmap[jindex] < 50) // If there is a proper value here
                        {
                            found = 1;

                            if (flatwindmap[jindex] > 0)
                                northpositive = 1;

                            else
                                northpositive = 0;
                        }

                        jj--;
                        up++;

                        if (up > height)
                            found = 1;
                    }
                    else
                        found = 1;

                } while (found == 0);

                int down = 0;
                found = 0;
                jj = j;
                bool southpositive = 0;

                do
                {
                    if (jj >= 0 && jj <= height)
                    {
                        int jindex = vi + jj;

                        if (flatwindmap[jindex] != 0 && flatwindmap[jindex] < 50) // If there is a proper value here
                        {
                            found = 1;

                            if (flatwindmap[jindex] > 0)
                                southpositive = 1;

                            else
                                southpositive = 0;
                        }

                        jj++;
                        down++;

                        if (down > height)
                            found = 1;
                    }
                    else
                        found = 1;

                } while (found == 0);

                if (down > up)
                {
                    if (northpositive)
                        flatwindmap[index] = 101;
                    else
                        flatwindmap[index] = 99;

                }
                else
                {
                    if (southpositive)
                        flatwindmap[index] = 101;
                    else
                        flatwindmap[index] = 99;
                }
            }
        }
    }

    // Now we need to wrap that onto the sphere!

    // Faces 0-3 are easy...

    for (int face = 0; face < 4; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            int ii = face * edge + i;
            
            for (int j = 0; j < edge; j++)
            {
                int jj = j + edge / 2;
                
                world.setwind(face, i, j, flatwindmap[ii * hheight + jj]);

                if (jj < horselats[ii * 6 + 1] || jj > horselats[ii * 6 + 4])
                    outsidehorse[vi + j] = 1;
            }
        }
    }

    // Face 4...

    int jj = 0;

    int vface4 = 4 * edge * edge;

    for (int j = edge / 2; j < edge; j++)
    {
        jj++;
        
        float div = ((float)width / 4.0f) / (float)(jj * 2);

        float ii = 0.0f;

        for (int i = edge / 2 - jj; i <= edge / 2 + jj; i++)
        {
            if (i >= 0 && i < edge)
            {
                int vi = vface4 + i * edge;

                ii = ii + div;

                if (ii > (float)edge - 1.0f)
                    ii = (float)edge - 1.0f;

                int vii = (int)ii * hheight;

                world.setwind(4, i, j, flatwindmap[vii + jj]);

                if (jj < horselats[(int)ii * 6 + 1] || jj > horselats[(int)ii * 6 + 4])
                    outsidehorse[vi + j] = 1;
            }
        }
    }

    jj = 0;

    for (int j = edge / 2; j >= 0; j--)
    {
        jj++;

        float div = ((float)width / 4.0f) / (float)(jj * 2);

        float ii = (float)(edge * 2);

        for (int i = edge / 2 + jj; i >= edge / 2 - jj; i--)
        {
            if (i >= 0 && i < edge)
            {
                int vi = vface4 + i * edge;

                ii = ii + div;

                if (ii > (float)(edge * 3) - 1.0f)
                    ii = (float)(edge * 3) - 1.0f;

                int vii = (int)ii * hheight;

                world.setwind(4, i, j, flatwindmap[vii + jj]);

                if (jj < horselats[(int)ii * 6 + 1] || jj > horselats[(int)ii * 6 + 4])
                    outsidehorse[vi + j] = 1;
            }
        }
    }

    int ii = 0;

    for (int i = edge / 2; i < edge; i++)
    {
        int vi = vface4 + i * edge;

        ii++;

        float div = ((float)width / 4.0f) / (float)(ii * 2);

        float jj = (float)edge;

        for (int j = edge / 2 + ii; j >= edge / 2 - ii; j--)
        {
            if (j >= 0 && j < edge)
            {
                jj = jj + div;

                if (jj > (float)(edge * 2) - 1.0f)
                    jj = (float)(edge * 2) - 1.0f;

                world.setwind(4, i, j, flatwindmap[(int)jj * hheight + ii]);

                if (jj < horselats[(int)ii * 6 + 1] || jj > horselats[(int)ii * 6 + 4])
                    outsidehorse[vi + j] = 1;
            }
        }
    }

    ii = 0;

    for (int i = edge / 2; i >=0 ; i--)
    {
        int vi = vface4 + i * edge;

        ii++;

        float div = ((float)width / 4.0f) / (float)(ii * 2);

        float jj = (float)(edge * 3);

        for (int j = edge / 2 - ii; j <= edge / 2 + ii; j++)
        {
            if (j >= 0 && j < edge)
            {
                jj = jj + div;

                if (jj > (float)(edge * 4) - 1.0f)
                    jj = (float)(edge * 4) - 1.0f;

                world.setwind(4, i, j, flatwindmap[(int)jj * hheight + ii]);

                if (jj < horselats[(int)ii * 6 + 1] || jj > horselats[(int)ii * 6 + 4])
                    outsidehorse[vi + j] = 1;
            }
        }
    }

    // Face 5...

    jj = 0;

    int vface5 = 5 * edge * edge;

    for (int j = edge / 2; j >= 0; j--)
    {
        jj++;

        float div = ((float)width / 4.0f) / (float)(jj * 2);

        float ii = 0.0f;

        int jjj = edge * 2 - jj;

        for (int i = edge / 2 - jj; i <= edge / 2 + jj; i++)
        {
            if (i >= 0 && i < edge)
            {
                ii = ii + div;

                if (ii > (float)edge - 1.0f)
                    ii = (float)edge - 1.0f;

                world.setwind(5, i, j, flatwindmap[(int)ii * hheight + jjj]);

                if (jj < horselats[(int)ii * 6 + 1] || jj > horselats[(int)ii * 6 + 4])
                    outsidehorse[vface5 + i * edge + j] = 1;
            }
        }
    }

    jj = 0;

    for (int j = edge / 2; j < edge; j++)
    {
        jj++;

        float div = ((float)width / 4.0f) / (float)(jj * 2);

        float ii = (float)(edge * 2);

        int jjj = edge * 2 - jj;

        for (int i = edge / 2 + jj; i >= edge / 2 - jj; i--)
        {
            if (i >= 0 && i < edge)
            {
                ii = ii + div;

                if (ii > (float)(edge * 3) - 1.0f)
                    ii = (float)(edge * 3) - 1.0f;

                world.setwind(5, i, j, flatwindmap[(int)ii * hheight + jjj]);

                if (jj < horselats[(int)ii * 6 + 1] || jj > horselats[(int)ii * 6 + 4])
                    outsidehorse[vface5 + i * edge + j] = 1;
            }
        }
    }

    ii = 0;

    for (int i = edge / 2; i < edge; i++)
    {
        ii++;

        float div = ((float)width / 4.0f) / (float)(ii * 2);

        float jj = (float)edge;

        int iii = edge * 2 - ii;

        for (int j = edge / 2 - ii; j <= edge / 2 + ii; j++)
        {
            if (j >= 0 && j < edge)
            {
                jj = jj + div;

                if (jj > (float)(edge * 2) - 1.0f)
                    jj = (float)(edge * 2) - 1.0f;

                world.setwind(5, i, j, flatwindmap[(int)jj * hheight + iii]);

                if (jj < horselats[(int)ii * 6 + 1] || jj > horselats[(int)ii * 6 + 4])
                    outsidehorse[vface5 + i * edge + j] = 1;
            }
        }
    }

    ii = 0;

    for (int i = edge / 2; i >= 0; i--)
    {
        ii++;

        float div = ((float)width / 4.0f) / (float)(ii * 2);

        float jj = (float)(edge * 3);

        int iii = edge * 2 - ii;

        for (int j = edge / 2 + ii; j >= edge / 2 - ii; j--)
        {
            if (j >= 0 && j < edge)
            {
                jj = jj + div;

                if (jj > (float)(edge * 4) - 1.0f)
                    jj = (float)(edge * 4) - 1.0f;

                world.setwind(5, i, j, flatwindmap[(int)jj * hheight + iii]);

                if (jj < horselats[(int)ii * 6 + 1] || jj > horselats[(int)ii * 6 + 4])
                    outsidehorse[vface5 + i * edge + j] = 1;
            }
        }
    }
}

// Temperature map creator.

void createtemperaturemap(planet& world, vector<int>& fractal, vector<float>& latitude)
{
    int edge = world.edge();
    int maxelev = world.maxelevation();

    float fracrange = 8.0f; // This is the range of variation given by the fractal.

    float tilt = world.tilt();

    // First work out what average temperature the poles and equator should be.

    float tempdiff = -0.045f * (tilt * tilt) + 6.872f * tilt - 231.7f; // This is the difference in temperature between equator and poles.

    if (tilt < 22.5f) // For low tilt, bring the tempdiff up a bit.
    {
        float adjust = 22.5f - tilt;
        tempdiff = tempdiff + adjust * 3.5f;
    }

    float avetemp = (float)world.averagetemp(); // Average temperature of the world.

    float northpolartemp = avetemp + tempdiff * 0.55f + (float)world.northpolaradjust(); // Note that tempdiff is usually negative, except for worlds with extreme tilts.
    float southpolartemp = avetemp + tempdiff * 0.55f + (float)world.southpolaradjust();
    float equatorialtemp = avetemp - tempdiff * 0.36f;

    world.setnorthpolartemp((int)northpolartemp);
    world.setsouthpolartemp((int)southpolartemp);
    world.seteqtemp((int)equatorialtemp);

    // Now sort out the seasonality (i.e. difference between summer and winter temperatures for any given point).

    float polarvariation = tilt * 1.55555f; // This is the seasonal variation in temperature at the poles. At the equator it will be zero. (Note: there is a twice-annual dip in equatorial temperature at the equinoxes, which is significant in worlds with high axial tilt, but we will deal with that elsewhere.)

    polarvariation = polarvariation * 1.5f; // 2.0f // This seems necessary to get closer to the correct temperatures (bear in mind many areas will have the difference toned down later because of rainfall)

    float fracvar = (float)maxelev / fracrange;

    float equatorlat = 90.0f;

    float northglobaldiff = equatorialtemp - northpolartemp; // This is the range in average temperatures from equator to pole, in the northern hemisphere.
    float southglobaldiff = equatorialtemp - southpolartemp; // This is the range in average temperatures from equator to pole, in the southern hemisphere.

    float northdiffperlat = northglobaldiff / equatorlat;
    float southdiffperlat = southglobaldiff / equatorlat;

    float variationperlat = polarvariation / equatorlat;

    // Now work out the actual temperatures. We do this in horizontal strips.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                bool docout = 0;

                if (random(1, 10000) == 1)
                    docout = 1;
                
                float thislatitude = latitude[index]; // +90.0f; // So it's in the range 0-180. 90 is the equator.
                
                // First get the base temperature for this latitude.

                float lattemp;

                if (thislatitude < 0.0f)
                    lattemp = equatorialtemp + thislatitude * northdiffperlat;
                else
                    lattemp = equatorialtemp - thislatitude * southdiffperlat;

                float lat = abs(thislatitude);

                // Now get the seasonal variation for this latitude.

                float latvariation = (lat * variationperlat) * 0.5f;

                float temperature = lattemp;

                // Now we adjust for sea temperatures.

                if (world.sea(face, i, j))
                    temperature = temperature + 2.0f;

                // Now add variation from the fractal.

                float var = (float)fractal[index];

                var = var / fracvar;

                var = var - (fracrange / 2.0f);

                temperature = temperature + var;

                // Now we add variation based on latitude.

                int newmintemp = (int)(temperature - latvariation);
                int newmaxtemp = (int)(temperature + latvariation);

                // Rule out really ridiculous temperatures

                if (newmintemp < (int)avetemp - 150)
                    newmintemp = (int)avetemp - 150;

                if (newmaxtemp > (int)avetemp + 100)
                    newmaxtemp = (int)avetemp + 100;

                if (thislatitude < 0.0f)
                {
                    world.setjantemp(face, i, j, newmintemp);
                    world.setjultemp(face, i, j, newmaxtemp);
                }
                else
                {
                    world.setjultemp(face, i, j, newmintemp);
                    world.setjantemp(face, i, j, newmaxtemp);
                }
            }
        }
    }

    /*
    // Now blur it.

    for (int n = 0; n < 4; n++)
    {
        for (int face = 0; face < 6; face++)
        {
            for (int i = 0; i < edge; i++)
            {
                for (int j = 0; j < edge; j++)
                {
                    float crount = 0;
                    float jantotal = 0;
                    float jultotal = 0;

                    for (int k = i - 2; k <= i + 2; k++)
                    {

                        if (k >= 0 && k < edge)
                        {
                            for (int l = j - 2; l <= j + 2; l++)
                            {
                                if (l >= 0 && l < edge)
                                {
                                    jantotal = jantotal + (float)world.jantemp(face, k, l);
                                    jultotal = jultotal + (float)world.jultemp(face, k, l);

                                    crount++;
                                }
                            }
                        }
                    }

                    float newjantotal = jantotal / crount;
                    float newjultotal = jultotal / crount;

                    world.setjantemp(face, i, j, (int)newjantotal);
                    world.setjultemp(face, i, j, (int)newjultotal);
                }
            }
        }
    }
    */

    // At high obliquity, equatorial regions have a two-summmer/two-winter seasonal cycle. 
    // Here, we adjust Jan/Jul temperatures accordingly.

    float fourseason = tilt * 0.294592f - 2.45428f; // This is the difference in temperature between the "summers" and the "winters".

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                float lat = 90.0f-abs(latitude[index]); // 0 at the poles, 90 at the equator.

                float fourseasonstrength = lat / equatorlat; // This measures the strength of the four-season cycle as determined by proximity to the equator.

                float thistempdiff = (fourseason * fourseasonstrength) / 2.0f;

                int jantemp = world.jantemp(face, i, j);
                int jultemp = world.jultemp(face, i, j);

                world.setjantemp(face, i, j, jantemp - (int)thistempdiff); // Remove the temp difference from the solstices.
                world.setjultemp(face, i, j, jultemp - (int)thistempdiff);
            }
        }
    }

    // Now adjust all the temperatures for eccentricity.
    // The more elliptical the orbit, the greater the adjustment. Also note: it is greater at the equator, not the poles.

    int perihelion = world.perihelion();
    float eccentricity = world.eccentricity();

    float betweenhot = 0.5f * (1.0f - eccentricity); // The higher the eccentricity, the shorter the "summer" will be.
    float betweencold = 1.0f - betweenhot;

    float eccendiff = (eccentricity * eccentricity) * 100.0f + eccentricity * 30.0f; // Temperature difference between "summer" and "winter".

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                float lat = 90.0f - abs(latitude[index]); // 0 at the poles, 90 at the equator.

                float eccenstrength = lat / equatorlat; // This measures the strength of the eccentricity "seasons". They're stronger at the equator.

                eccenstrength = (1.0f + eccenstrength) / 2.0f; // Half-strength at the poles.

                float thistempdiffhot = (eccendiff * eccenstrength) / 2.0f;
                float thistempdiffcold = 0.0f - thistempdiffhot;

                float jantemp = (float)world.jantemp(face, i, j);
                float jultemp = (float)world.jultemp(face, i, j);

                if (perihelion == 0)
                {
                    jantemp = jantemp + thistempdiffhot;
                    jultemp = jultemp + thistempdiffcold;
                }

                if (perihelion == 1)
                {
                    jantemp = jantemp + thistempdiffcold;
                    jultemp = jultemp + thistempdiffhot;
                }

                world.setjantemp(face, i, j, (int)jantemp);
                world.setjultemp(face, i, j, (int)jultemp);
            }
        }
    }

    // Now adjust for altitude.

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                int jantemp = world.jantemp(face,i, j);
                int jultemp = world.jultemp(face, i, j);

                jantemp = tempelevadd(world, jantemp, face, i, j);
                jultemp = tempelevadd(world, jultemp, face, i, j);

                world.setjantemp(face, i, j, jantemp);
                world.setjultemp(face, i, j, jultemp);
            }
        }
    }
}

// Work out the sea ice. (Effectively turned off for now.)

void createseaicemap(planet& world, vector<int>& fractal, vector<float>& longitude, vector<float>& latitude)
{
    int edge = world.edge();
    int sealevel = world.sealevel();
    int maxelev = world.maxelevation();

    /*
    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
                world.setseatempreduce(face, i, j, 0);
        }
    }

    return;
    */




    int icetemp = world.seaicetemp();

    int maxadjust = 12; // Maximum amount temperatures can be adjusted by
    int adjustfactor = maxelev / maxadjust;

    int tempdist = 4;// 12; // Distance from the current point where we'll check temperatures (reduced to 4 to speed things up - can increase again if it doesn't look good...)
    float landfactor = 20.0f; // The higher this is, the less effect land will have on sea ice

    // First, check that the whole world isn't frozen

    bool foundnoperm = 0;

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.jantemp(face, i, j) <= icetemp && world.jultemp(face, i, j) <= icetemp)
                {
                    foundnoperm = 1;
                    i = edge;
                    j = edge;
                }
            }
        }
    }

    if (foundnoperm == 0)
        return;

    // Now work out temperature reductions

    int range = 10;

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                if (world.nom(face, i, j) <= sealevel)
                {
                    bool looknorth = 0;
                    
                    if (latitude[index] < 0.0f)
                    {
                        looknorth = 1;
                        
                        if (world.northpolartemp() > world.eqtemp())
                            looknorth = 0;
                    }
                    else
                    {
                        looknorth = 0;

                        if (world.southpolartemp() > world.eqtemp())
                            looknorth = 1;
                    }
                                       
                    float total = 0.0f;

                    for (int k = -range; k <= range; k++)
                    {
                        for (int l = -range; l <= range; l++)
                        {
                            if (k * k + l * l < range * range)
                            {
                                globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                                if (lpoint.face != -1)
                                {
                                    if (world.nom(lpoint.face, lpoint.x, lpoint.y) > sealevel)
                                    {
                                        int lindex = lpoint.face * edge * edge + lpoint.x * edge + lpoint.y;

                                        bool adding = 0;

                                        if (looknorth)
                                        {
                                            if (latitude[lindex] < latitude[index])
                                                adding = 1;
                                        }
                                        else
                                        {
                                            if (latitude[lindex] > latitude[index])
                                                adding = 1;
                                        }

                                        if (adding && random(1, 2) == 1)
                                            total = total + 0.3f;
                                    }
                                }
                            }
                        }
                    }

                    total = total * 0.2f;

                    world.setseatempreduce(face, i, j, (int)total);
                }
            }
        }
    }

    // Now ensure that it doesn't drop off at coasts.

    for (int n = 0; n < 4; n++)
    {
        for (int face = 0; face < 6; face++)
        {
            for (int i = 0; i < edge; i++)
            {
                for (int j = 0; j < edge; j++)
                {
                    if (world.sea(face,i,j) == 0 || vaguelycoastal(world, face, i, j))
                    {
                        int thisreduce = world.seatempreduce(face, i, j);

                        for (int k = -1; k <= 1; k++)
                        {
                            for (int l = -1; l <= 1; l++)
                            {
                                globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                                if (lpoint.face != -1)
                                {
                                    int thatreduce = world.seatempreduce(lpoint.face, lpoint.x, lpoint.y);

                                    if (thatreduce > thisreduce)
                                        thisreduce = thatreduce;
                                }
                            }
                        }

                        world.setseatempreduce(face, i, j, thisreduce);
                    }
                }
            }
        }
    }
}

// Rain map creator.

void createrainmap(planet& world, vector<int>& fractal, int landtotal, int seatotal, vector<bool>& outsidehorse, boolshapetemplate smalllake[], boolshapetemplate shape[], vector<float>& longitude, vector<float>& latitude, vector<fourglobepoints>& dirpoint, int& progressval, string& progresstext)
{
    int edge = world.edge();

    int slopewaterreduce = 20; // The higher this is, the less extra rain falls on slopes.
    int maxmountainheight = 100;

    vector<int> inland(6 * edge * edge, 0);

    // First, do rainfall over the oceans.

    if (seatotal > 0)
    {
        updatereport(progressval, progresstext, "Calculating ocean rainfall");

        createoceanrain(world,  dirpoint);
    }

    if (landtotal > 0)
    {
        // Now we do the rainfall over land.

        if (seatotal > 0)
        {
            updatereport(progressval, progresstext, "Calculating rainfall from prevailing winds");

            createprevailinglandrain(world, inland, outsidehorse, maxmountainheight, slopewaterreduce, longitude, dirpoint);

            // Now for monsoons!

            updatereport(progressval, progresstext, "Calculating monsoons");

            createmonsoons(world, maxmountainheight, slopewaterreduce, latitude, dirpoint);
        }
        else
        {
            updatereport(progressval, progresstext, "Calculating rainfall");

            createdesertworldrain(world);

            for (int face = 0; face < 6; face++)
            {
                int vface = face * edge * edge;

                for (int i = 0; i < edge; i++)
                {
                    int vi = vface + i * edge;

                    for (int j = 0; j < edge; j++)
                        inland[vi + j] = 10000;
                }
            }
        }
    }

    // Now increase the seasonal variation in rainfall in certain areas, to encourage various climates.

    updatereport(progressval, progresstext, "Calculating seasonal rainfall");

    adjustseasonalrainfall(world, inland, latitude, dirpoint, outsidehorse);

    // Now smooth the rainfall.

    updatereport(progressval, progresstext, "Smoothing rainfall");

    smoothrainfall(world, maxmountainheight, dirpoint);

    // Now cap excessive rainfall.

    updatereport(progressval, progresstext, "Capping rainfall");

    caprainfall(world);

    // Also ensure that if there is any rainfall, every month has at least some.

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                int janrain = world.janrain(face, i, j);
                int julrain = world.julrain(face, i, j);

                if (janrain > 0 && julrain == 0)
                    world.setjulrain(face, i, j, 1);

                if (julrain > 0 && janrain == 0)
                    world.setjanrain(face, i, j, 1);
            }
        }
    }

    // Now smooth the rainfall again.

    updatereport(progressval, progresstext, "Smoothing rainfall");

    smoothrainfall(world, maxmountainheight, dirpoint);

    // Now we adjust temperatures in light of rainfall.

    updatereport(progressval, progresstext, "Adjusting temperatures");

    adjusttemperatures(world, inland);

    // Now make temperatures a little more extreme when further from the sea.

    updatereport(progressval, progresstext, "Adjusting continental temperatures");

    adjustcontinentaltemperatures(world, inland);

    // Now just smooth the temperatures a bit. Any temperatures that are lower than their neighbours to north and south get bumped up, to avoid the appearance of streaks.

    updatereport(progressval, progresstext, "Smoothing temperatures");

    smoothtemperatures(world, dirpoint);

    // Now just prevent subpolar climates from turning into other continental types when further from the sea.

    updatereport(progressval, progresstext, "Checking subpolar regions");

    //removesubpolarstreaks(world, dirpoint);

    // Now correct any issues with temperature fluctuating oddly.

    //correcttemperatures(world);

    // Now we just sort out the mountain precipitation arrays, which will be used at the regional level for ensuring that higher mountain precipitation isn't splashed too far.

    updatereport(progressval, progresstext, "Calculating mountain rainfall");

    createmountainprecipitation(world);
}

// This creates rain over the oceans.

void createoceanrain(planet& world, vector<fourglobepoints>& dirpoint)
{
    int edge = world.edge();
    int sealevel = world.sealevel();
    int maxelev = world.maxelevation();

    int landmult = 5; // This is the amount we multiply the wind factor by to calculate land shadows.
    int fracrange = 60; // This is the range of variation given by the fractal.
    int maxoceanrain = 1500; // Maximum amount of rain per month over the ocean.
    int landshadowfactor = 4; // The amount that the land shadows affect ocean rainfall.

    // First set some basic rain over the whole ocean.

    for (int face = 0; face < 6; face++)
    {
        for (int j = 0; j < edge; j++)
        {
            for (int i = 0; i < edge; i++)
            {
                if (world.nom(face, i, j) <= sealevel)
                {
                    int winterrain = world.mintemp(face, i, j) + 15;
                    int summerrain = world.maxtemp(face, i, j) + 15;

                    if (winterrain < 0)
                        winterrain = 0;

                    if (summerrain < 0)
                        summerrain = 0;

                    winterrain = winterrain * 4;
                    summerrain = summerrain * 4;

                    if (winterrain > maxoceanrain)
                        winterrain = maxoceanrain;

                    if (summerrain > maxoceanrain)
                        summerrain = maxoceanrain;

                    world.setwinterrain(face, i, j, winterrain);
                    world.setsummerrain(face, i, j, summerrain);
                }
            }
        }
    }

    vector<bool> done(6 * edge * edge, 0);

    // Now do the land shadows over oceans.
    // First the westerly ones.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 1; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                if (world.wind(face, i, j) > 0 && world.wind(face, i, j) < 50)
                {
                    if (world.nom(face, i, j) <= sealevel)
                    {
                        globepoint westpoint = dirpoint[index].west;
                        
                        if (world.nom(westpoint.face, westpoint.x, westpoint.y) > sealevel)
                        {
                            int crount = world.wind(face, i, j) * landmult;

                            int dryness = 0; // This will lot how much dryness we're dealing with here.

                            // First check to the west and see how much land there is.

                            while (crount > 0)
                            {
                                if (world.nom(westpoint.face, westpoint.x, westpoint.y) > sealevel)
                                    dryness++;

                                westpoint = dirpoint[westpoint.face * edge * edge + westpoint.x * edge + westpoint.y].west;

                                crount--;
                            }

                            crount = 0;

                            // Now we remove rainfall from the sea to the east.

                            if (dryness > 0)
                            {
                                globepoint eastpoint;
                                eastpoint.face = face;
                                eastpoint.x = i;
                                eastpoint.y = j;
                                
                                for (int z = 1; z <= dryness; z++)
                                {
                                    eastpoint = dirpoint[eastpoint.face * edge * edge + eastpoint.x * edge + eastpoint.y].east;

                                    done[eastpoint.face * edge * edge + eastpoint.x * edge + eastpoint.y] = 1;

                                    int amounttoremove = (dryness - z) * landshadowfactor;

                                    world.setsummerrain(eastpoint.face, eastpoint.x, eastpoint.y, world.summerrain(eastpoint.face, eastpoint.x, eastpoint.y) - amounttoremove);
                                    world.setwinterrain(eastpoint.face, eastpoint.x, eastpoint.y, world.winterrain(eastpoint.face, eastpoint.x, eastpoint.y) - amounttoremove);

                                    if (world.summerrain(eastpoint.face, eastpoint.x, eastpoint.y) < 0)
                                        world.setsummerrain(eastpoint.face, eastpoint.x, eastpoint.y, 0);

                                    if (world.winterrain(eastpoint.face, eastpoint.x, eastpoint.y) < 0)
                                        world.setwinterrain(eastpoint.face, eastpoint.x, eastpoint.y, 0);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Then the easterly ones.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                if (world.wind(face, i, j) < 0)
                {
                    if (world.nom(face, i, j) <= sealevel)
                    {
                        globepoint eastpoint = dirpoint[index].east;

                        if (world.nom(eastpoint.face, eastpoint.x, eastpoint.y) > sealevel)
                        {
                            int crount = 0 - world.wind(face, i, j) * landmult;
                            int dryness = 0; // This will lot how much dryness we're dealing with here.

                            // First check to the east and see how much land there is.

                            while (crount > 0)
                            {
                                if (world.nom(eastpoint.face, eastpoint.x, eastpoint.y) > sealevel)
                                    dryness++;

                                eastpoint = dirpoint[eastpoint.face * edge * edge + eastpoint.x * edge + eastpoint.y].east;

                                crount--;
                            }

                            crount = 0;

                            // Now we remove rainfall from the sea to the west.

                            if (dryness > 0)
                            {
                                globepoint westpoint;
                                westpoint.face = face;
                                westpoint.x = i;
                                westpoint.y = j;

                                for (int z = 1; z <= dryness; z++)
                                {
                                    westpoint = dirpoint[westpoint.face * edge * edge + westpoint.x * edge + westpoint.y].west;

                                    done[westpoint.face * edge * edge + westpoint.x * edge + westpoint.y] = 1;

                                    int amounttoremove = (dryness - z) * landshadowfactor;

                                    world.setsummerrain(westpoint.face, westpoint.x, westpoint.y, world.summerrain(westpoint.face, westpoint.x, westpoint.y) - amounttoremove);
                                    world.setwinterrain(westpoint.face, westpoint.x, westpoint.y, world.winterrain(westpoint.face, westpoint.x, westpoint.y) - amounttoremove);

                                    if (world.summerrain(westpoint.face, westpoint.x, westpoint.y) < 0)
                                        world.setsummerrain(westpoint.face, westpoint.x, westpoint.y, 0);

                                    if (world.winterrain(westpoint.face, westpoint.x, westpoint.y) < 0)
                                        world.setwinterrain(westpoint.face, westpoint.x, westpoint.y, 0);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Now fill in any gaps.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                if (done[index] == 0 && world.nom(face, i, j) <= sealevel)
                {
                    globepoint uppoint = getglobepoint(edge, face, i, j, 0, -1);
                    globepoint downpoint = getglobepoint(edge, face, i, j, 0, 1);
                    globepoint leftpoint = getglobepoint(edge, face, i, j, -1, 0);
                    globepoint rightpoint = getglobepoint(edge, face, i, j, 1, 0);

                    if (uppoint.face != -1 && downpoint.face != -1 && leftpoint.face != -1 && rightpoint.face != -1)
                    {
                        int newsummer = -1;
                        int newwinter = -1;

                        if (done[uppoint.face * edge * edge + uppoint.x * edge + uppoint.y])
                        {
                            newsummer = world.summerrain(uppoint.face, uppoint.x, uppoint.y);
                            newwinter = world.winterrain(uppoint.face, uppoint.x, uppoint.y);
                        }

                        if (done[downpoint.face * edge * edge + downpoint.x * edge + downpoint.y])
                        {
                            newsummer = world.summerrain(downpoint.face, downpoint.x, downpoint.y);
                            newwinter = world.winterrain(downpoint.face, downpoint.x, downpoint.y);
                        }

                        if (done[leftpoint.face * edge * edge + leftpoint.x * edge + leftpoint.y])
                        {
                            newsummer = world.summerrain(leftpoint.face, leftpoint.x, leftpoint.y);
                            newwinter = world.winterrain(leftpoint.face, leftpoint.x, leftpoint.y);
                        }

                        if (done[rightpoint.face * edge * edge + rightpoint.x * edge + rightpoint.y])
                        {
                            newsummer = world.summerrain(rightpoint.face, rightpoint.x, rightpoint.y);
                            newwinter = world.winterrain(rightpoint.face, rightpoint.x, rightpoint.y);
                        }

                        if (newsummer != -1)
                        {
                            world.setsummerrain(face, i, j, newsummer);
                            world.setwinterrain(face, i, j, newwinter);
                        }
                    }
                }
            }
        }
    }

    // Now blur.

    vector<int> newsummer(6 * edge * edge, -1);
    vector<int> newwinter(6 * edge * edge, -1);

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                if (world.nom(face, i, j) <= sealevel)
                {
                    globepoint neighbour[3][3];

                    neighbour[1][1].face = face;
                    neighbour[1][1].x = i;
                    neighbour[1][1].y = j;

                    neighbour[1][0] = getdirglobepoint(edge, face, i, j, 0, -1);
                    neighbour[1][2] = getdirglobepoint(edge, face, i, j, 0, 1);
                    neighbour[0][1] = getdirglobepoint(edge, face, i, j, -1, 0);
                    neighbour[2][1] = getdirglobepoint(edge, face, i, j, 1, 0);

                    if (neighbour[1][0].face != -1 && neighbour[1][2].face != -1 && neighbour[0][1].face != -1 && neighbour[2][1].face != -1)
                    {
                        neighbour[0][0] = getdirglobepoint(edge, neighbour[1][0].face, neighbour[1][0].x, neighbour[1][0].y, -1, 0);
                        neighbour[2][0] = getdirglobepoint(edge, neighbour[1][0].face, neighbour[1][0].x, neighbour[1][0].y, 1, 0);
                        neighbour[0][2] = getdirglobepoint(edge, neighbour[1][2].face, neighbour[1][2].x, neighbour[1][2].y, -1, 0);
                        neighbour[2][2] = getdirglobepoint(edge, neighbour[1][2].face, neighbour[1][2].x, neighbour[1][2].y, 1, 0);
                    
                        if (neighbour[0][0].face != -1 && neighbour[2][0].face != -1 && neighbour[0][2].face != -1 && neighbour[2][2].face != -1)
                        {
                            float summertotal = 0.0f;
                            float wintertotal = 0.0f;

                            float crount = 0.0f;
                            
                            for (int k = 0; k < 3; k++)
                            {
                                for (int l = 0; l < 3; l++)
                                {
                                    if (world.nom(neighbour[k][l].face, neighbour[k][l].x, neighbour[k][l].y) <= sealevel)
                                    {
                                        summertotal = summertotal + (float)world.summerrain(neighbour[k][l].face, neighbour[k][l].x, neighbour[k][l].y);
                                        wintertotal = wintertotal + (float)world.winterrain(neighbour[k][l].face, neighbour[k][l].x, neighbour[k][l].y);

                                        crount++;
                                    }
                                }
                            }

                            newsummer[index] = (int)(summertotal / crount);
                            newwinter[index] = (int)(wintertotal / crount);
                        }
                    }
                }
            }
        }
    }

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                if (newsummer[index] != -1)
                {
                    world.setsummerrain(face, i, j, newsummer[index]);
                    world.setwinterrain(face, i, j, newwinter[index]);
                }
            }
        }
    }

    // Now add variation from a fractal.

    vector<int> newfractal(6 * edge * edge, 0);

    int grain = 8; // Level of detail on this fractal map.
    float valuemod = 0.2f;
    float valuemod2 = 3.0f;

    createfractal(newfractal, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    float fracvar = (float)maxelev / (float)fracrange;

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                if (world.nom(face,i,j) <= sealevel)
                {
                    float v = (float)newfractal[index];
                    v = v / fracvar;
                    v = v - ((float)fracrange / 2.0f);

                    if (world.winterrain(face, i, j) > 0)
                        world.setwinterrain(face, i, j, world.winterrain(face, i, j) + (int)v);
                    else
                        world.setwinterrain(face, i, j, (int)v);

                    if (world.summerrain(face, i, j) > 0)
                        world.setsummerrain(face, i, j, world.summerrain(face, i, j) + (int)v);
                    else
                        world.setsummerrain(face, i, j, (int)v);
                }
            }
        }
    }

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.winterrain(face, i, j) < 0)
                    world.setwinterrain(face, i, j, 0);

                if (world.summerrain(face, i, j) < 0)
                    world.setsummerrain(face, i, j, 0);
            }
        }
    }
}

// This creates prevailing rain over the land.

void createprevailinglandrain(planet& world, vector<int>& inland, vector<bool>& outsidehorse, int maxmountainheight, int slopewaterreduce, vector<float>& longitude, vector<fourglobepoints>& dirpoint)
{
    int edge = world.edge();
    int eedge = edge * edge;
    int sealevel = world.sealevel();
    int maxelev = world.maxelevation();
    float tilt = world.tilt();
    float tempdecrease = world.tempdecrease() * 20.0f; // The default is 6.5 * 20.0f = 130.0.

    int seamult = 80;// 60; // This is the amount we multiply the wind factor by to get the number of sea tiles that will provide rain. The higher this is, the more rain will extend onto the land.
    float tempfactor = 80.0f; // The amount temperature affects moisture pickup over ocean. The higher it is, the more difference it makes.
    float mintemp = 0.15f; // The minimum fraction that low temperatures can reduce the pickup rate to.
    int dumprate = 100; // 80; // The higher this number, the less rain gets deposited, but the further across the land it is distributed.
    float fpickuprate = world.waterpickup() * 350.0f; // 200.0f;
    int pickuprate = (int)fpickuprate; // The higher this number, the more rain gets picked up over ocean tiles.
    int landpickuprate = 40; // 40; // This is the amount of rain that gets acquired while passing over land.
    int swervechance = 2; // 3; // The lower this number, the more variation in where the rain lands.
    int spreadchance = 2; // The lower this number, the more precipitation will spread to north and south.
    float newseedproportion = 0.85f; // 0.95f; // When precipitation spreads, it's reduced to this proportion.
    float horseseedproportion = 0.95f; // 0.8f; // 0.4f; // If spreading into horse latitudes, it's reduced to this proportion.
    int splashsize = 1; // The higher this number, the larger area gets splashed around each target tile.    
    float elevationfactor = 0.002f;// 0.002f; // This determines how much elevation affects the degree to which gradient affects rainfall.
    int slopemin = 300; //200; // This is how high land has to be before the gradient affects rainfall.
    float seasonalvar = tilt / 3750.f; // 0.006; // Variation in rainfall between seasons.
    float tropicalseasonalvar = tilt / 2250.f; // 0.01; // Variation in tropical rainfall between seasons.
    int maxseasonaldistance = 50; // After this point, being far from the sea won't make more difference to seasonal rainfall variation.
    float heatpickuprate = 0.008f; // The higher this is, the more heat will come from prevailing winds over ocean.
    float heatdepositrate = 0.15f; // The higher this is, the less far the heating effect will extend onto the land.
    float summerfactor = tilt / 75.f; // 0.3f; // The amount that the sea warms land in summer, as a proportion of how much it warms it in winter.
    float maxwinterheatfactor = 3.5f; // The maximum amount of extra warming that the land can get from the sea.
    float wintericefactor = 0.05f; // If there's seasonal sea ice, multiply the winter rain by this.
    float tropicalrainreduce = 0.4f; // Amount to lower rain in tropical areas.
    float extrarainrate = 0.00f; // The proportion of the waterlog that gets added to the rainfall each tick. The higher this is, the more extra rainfall there is.

    float slopefactor = 160.0f - tempdecrease; //30; // This determines how much gradient affects rainfall. The lower it is, the more it affects it.

    if (slopefactor < 1)
        slopefactor = 1;

    vector<int> rainseed(6 * eedge, 0);
    vector<int> raintick(6 * eedge, 0);
    vector<float> rainheatseed(6 * eedge, 0.0f);
    vector<short> raindir(6 * eedge, 0);

    // First, set the seeds for each bit of rain.

    // Westerly first.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * eedge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                if (world.nom(face, i, j) > sealevel)
                {
                    if (world.winddir(face, i, j) == 1) //(world.wind(face, i, j) > 0 && world.wind(face, i, j) != 99)
                    {
                        int currentwind = world.wind(face, i, j);

                        //if (currentwind == 101)
                            currentwind = 4;

                            globepoint westpoint = dirpoint[index].west;

                        if (world.nom(westpoint.face, westpoint.x, westpoint.y) <= sealevel) // If this is a coastal tile
                        {
                            int crount = currentwind * seamult;
                            int waterlog = 0; // This is the amount of water being carried.
                            float waterheat = 0.0f; // This is the amount of extra heat being carried.
                            bool seaice = 0; // This will be 1 if any seasonal sea ice is found.

                            while (crount > 0)
                            {
                                if (world.seaice(westpoint.face, westpoint.x, westpoint.y) == 1)
                                    seaice = 1;

                                float temp = (float)((world.maxtemp(westpoint.face, westpoint.x, westpoint.y) + world.mintemp(westpoint.face, westpoint.x, westpoint.y)) / 2);
                                temp = temp / tempfactor; // Less water is picked up from colder oceans.

                                if (temp < mintemp)
                                    temp = mintemp;

                                temp = (temp / 100.0f) + 1.0f;

                                if (seaice == 1)
                                    temp = temp / 2.0f;

                                waterlog = waterlog + (int)((float)pickuprate * temp);
                                waterheat = waterheat + heatpickuprate;

                                if (waterheat > maxwinterheatfactor)
                                    waterheat = maxwinterheatfactor;

                                rainseed[index] = waterlog;
                                rainheatseed[index] = waterheat;
                                raindir[index] = 1;
                                raintick[index] = 1;

                                westpoint = dirpoint[westpoint.face * eedge + westpoint.x * edge + westpoint.y].west;

                                crount--;
                            }
                        }
                    }
                }
            }
        }
    }

    // Now easterly.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * eedge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                if (world.nom(face, i, j) > sealevel)
                {
                    if (world.winddir(face,i,j)==-1) //(world.wind(face, i, j) < 0 || world.wind(face, i, j) == 99)
                    {
                        int index = vi + j;

                        int currentwind = world.wind(face, i, j);

                        //if (currentwind == 101)
                            currentwind = 4;

                        globepoint eastpoint = dirpoint[index].east;

                        if (world.nom(eastpoint.face, eastpoint.x, eastpoint.y) <= sealevel) // If this is a coastal tile
                        {
                            int crount = currentwind * seamult;
                            int waterlog = 0; // This is the amount of water being carried.
                            float waterheat = 0.0f; // This is the amount of extra heat being carried.
                            bool seaice = 0; // This will be 1 if any seasonal sea ice is found.

                            while (crount > 0)
                            {
                                if (world.seaice(eastpoint.face, eastpoint.x, eastpoint.y) == 1)
                                    seaice = 1;

                                float temp = (float)((world.maxtemp(eastpoint.face, eastpoint.x, eastpoint.y) + world.mintemp(eastpoint.face, eastpoint.x, eastpoint.y)) / 2);
                                temp = temp / tempfactor; // Less water is picked up from colder oceans.

                                if (temp < mintemp)
                                    temp = mintemp;

                                temp = (temp / 100.0f) + 1.0f;

                                if (seaice == 1)
                                    temp = temp / 2.0f;

                                waterlog = waterlog + (int)((float)pickuprate * temp);
                                waterheat = waterheat + heatpickuprate;

                                if (waterheat > maxwinterheatfactor)
                                    waterheat = maxwinterheatfactor;

                                rainseed[index] = waterlog;
                                rainheatseed[index] = waterheat;
                                raindir[index] = -1;
                                raintick[index] = 1;

                                eastpoint = dirpoint[eastpoint.face * eedge + eastpoint.x * edge + eastpoint.y].east;

                                crount--;
                            }
                        }
                    }
                }
            }
        }
    }

    // Now blur those seeds.

    int dist = 4;

    for (int face = 0; face < 6; face++)
    {
        int vface = face * eedge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                if (rainseed[index] != 0)
                {
                    int crount = 0;
                    short dir = 0;
                    int raintotal = 0;
                    float heattotal = 0;
                    float olat = 0.0f;

                    for (int k = -dist; k <= dist; k++)
                    {
                        for (int l = -dist; l <= dist; l++)
                        {
                            globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                            if (lpoint.face != -1)
                            {
                                if (world.nom(lpoint.face, lpoint.x, lpoint.y) > sealevel)
                                {
                                    int lindex = lpoint.face * eedge + lpoint.x * edge + lpoint.y;

                                    globepoint westpoint = dirpoint[lindex].west;
                                    globepoint eastpoint = dirpoint[lindex].east;

                                    if (world.nom(westpoint.face, westpoint.x, westpoint.y) <= sealevel || world.nom(eastpoint.face, eastpoint.x, eastpoint.y) <= sealevel)
                                    {
                                        crount++;
                                        raintotal = raintotal + rainseed[lindex];
                                        heattotal = heattotal + rainheatseed[lindex];

                                        if (raindir[lindex] != 0)
                                            dir = raindir[lindex];
                                    }
                                }
                            }
                        }
                    }

                    if (raintotal > 0)
                        rainseed[index] = raintotal / crount;

                    if (heattotal > 0)
                        rainheatseed[index] = heattotal / crount;

                    if (dir != 0)
                    {
                        raindir[index] = dir;
                        raintick[index] = 1;
                    }
                }
            }
        }
    }

    // Now we take those seeds and spread them across the land, depositing rain as we go.

    int thistick = 1; // Tracks the time. Only do cells that are set to be done on this turn.
    bool found = 0;

    do
    {
        found = 0;

        for (int face = 0; face < 6; face++)
        {
            int vface = face * eedge;

            for (int x = 0; x < edge; x++)
            {
                int vx = vface + x * edge;

                for (int y = 0; y < edge; y++)
                {
                    int index = vx + y;

                    if (raintick[index] == thistick && world.sea(face, x, y) == 0)
                    {
                        found = 1;

                        float waterlog = (float)rainseed[index];
                        float waterheat = rainheatseed[index];

                        short dir = raindir[index];

                        inland[index] = thistick; // This is how far we are from the sea!

                        globepoint downwind; // The next point along.
                        globepoint upwind;

                        if (dir == -1)
                        {
                            downwind = dirpoint[index].west;
                            upwind = dirpoint[index].east;
                        }
                        else
                        {
                            upwind = dirpoint[index].west;
                            downwind = dirpoint[index].east;
                        }

                        // First, work out how much water to dump.

                        float waterdumped = waterlog / (float)dumprate;
                        float noslopewaterdumped = waterdumped;

                        float slope = 0.0f;

                        if (world.nom(face, x, y) - sealevel > slopemin) // If it's going uphill, increase the amount of water being dumped.
                        {
                            slope = (float)getslope(world, upwind.face, upwind.x, upwind.y, face, x, y);
                            slope = slope / slopefactor;

                            if (slope > 1) // If it's going uphill
                            {
                                waterdumped = waterdumped * slope;
                                float elevation = (float)(world.nom(face, x, y) - sealevel);
                                waterdumped = waterdumped * elevationfactor * elevation;

                                if (waterdumped > waterlog)
                                    waterdumped = waterlog;
                            }

                            // If this is a crater rim, reduce its height.

                            if (world.craterrim(face, x, y) > 0)
                            {
                                float rimelev = (float)world.craterrim(face, x, y);

                                rimelev = rimelev - waterlog * 10.0f; // 1.2f; // This will reduce crater rims quite drastically in areas of high rainfall, which means they'll only be clearly visible in deserts - which is what we want.

                                if (rimelev < 0.0f)
                                    rimelev = 0.0f;

                                world.setcraterrim(face, x, y, (int)rimelev);
                            }
                        }

                        waterdumped = waterdumped - noslopewaterdumped; // Only reduce the waterlog as it goes over mountains etc.

                        // Here we removed the dumped water from the waterlog. In fact we don't remove it all, to ensure that precipitation keeps falling even over large continents.

                        if (slope > 0)
                            waterlog = waterlog - (waterdumped / 2);
                        else
                            waterlog = waterlog - (waterdumped / 4);

                        waterlog = waterlog + landpickuprate;

                        waterdumped = waterdumped + waterlog * extrarainrate;
                        noslopewaterdumped = noslopewaterdumped + waterlog * extrarainrate;

                        // Now dump the water.

                        for (int i = -splashsize; i <= splashsize; i++)
                        {
                            globepoint ipoint = getglobepoint(edge, face, x, y, i, 0);

                            if (ipoint.face != -1)
                            {
                                for (int j = -splashsize; j <= splashsize; j++)
                                {
                                    globepoint jpoint = getglobepoint(edge, ipoint.face, ipoint.x, ipoint.y, 0, j);

                                    if (jpoint.face != -1)
                                    {
                                        int jindex = jpoint.face * eedge + jpoint.x * edge + jpoint.y;
                                        bool splashok = 1;

                                        if (world.map(jpoint.face, jpoint.x, jpoint.y) < world.map(face, x, y)) // Don't splash onto lower ground that's ahead.
                                        {
                                            if (dir == 1 && longitude[jindex] > longitude[index])
                                                splashok = 0;

                                            if (dir == -1 && longitude[jindex] < longitude[index])
                                                splashok = 0;
                                        }

                                        if (slope > 1)
                                        {
                                            float slopewater = waterdumped - noslopewaterdumped;
                                            waterdumped = noslopewaterdumped + slopewater / slopewaterreduce;
                                        }

                                        if (splashok == 1)
                                        {
                                            int newrain = (int)(waterdumped / 2.0f);

                                            if (world.summerrain(jpoint.face, jpoint.x, jpoint.y) < newrain)
                                                world.setsummerrain(jpoint.face, jpoint.x, jpoint.y, newrain);

                                            if (world.winterrain(jpoint.face, jpoint.x, jpoint.y) < newrain)
                                                world.setwinterrain(jpoint.face, jpoint.x, jpoint.y, newrain);

                                            if (outsidehorse[jindex])
                                            {
                                                world.setmintemp(jpoint.face, jpoint.x, jpoint.y, world.mintemp(jpoint.face, jpoint.x, jpoint.y) + (int)waterheat);
                                                world.setmaxtemp(jpoint.face, jpoint.x, jpoint.y, world.maxtemp(jpoint.face, jpoint.x, jpoint.y) + (int)(waterheat * summerfactor));
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        // Reduce the heat being carried.

                        waterheat = waterheat - heatdepositrate;

                        if (waterheat < 0.0f)
                            waterheat = 0.0f;

                        // Now create a new seed downwind.

                        if (random(1, swervechance) == 1)
                        {
                            if (random(1, 2) == 1)
                                downwind = dirpoint[downwind.face * eedge + downwind.x * edge + downwind.y].north;
                            else
                                downwind = dirpoint[downwind.face * eedge + downwind.x * edge + downwind.y].south;
                        }

                        int downindex = downwind.face * eedge + downwind.x * edge + downwind.y;

                        if (world.sea(downwind.face, downwind.x, downwind.y) == 0)
                        {
                            rainseed[downindex] = (int)waterlog;
                            rainheatseed[downindex] = waterheat;
                            raindir[downindex] = dir;
                            raintick[downindex] = thistick + 1;
                        }

                        // Now maybe create new seeds to the north or south.

                        for (int n = 0; n < 2; n++)
                        {
                            globepoint newpoint;

                            bool goingnorth = 0;

                            if (n == 0)
                            {
                                newpoint = dirpoint[downindex].north;
                                goingnorth = 1;
                            }
                            else
                                newpoint = dirpoint[downindex].south;

                            if (world.sea(newpoint.face, newpoint.x, newpoint.y) == 0)
                            {
                                int newindex = newpoint.face * eedge + newpoint.x * edge + newpoint.y;

                                int thisspreadchance = spreadchance;

                                bool horse = 0;

                                if (world.winddir(newpoint.face, newpoint.x, newpoint.y) == 0) //(world.wind(newpoint.face, newpoint.x, newpoint.y) == 99 || world.wind(newpoint.face, newpoint.x, newpoint.y) == 101 || world.wind(newpoint.face, newpoint.x, newpoint.y) == 0)
                                {
                                    horse = 1;
                                    thisspreadchance = thisspreadchance * 3;
                                }

                                if (random(1, thisspreadchance) == 1)
                                {
                                    float fwaterlog = waterlog;
                                    fwaterlog = fwaterlog * newseedproportion;

                                    if (horse == 1)
                                        fwaterlog = fwaterlog * horseseedproportion;

                                    int newwaterlog = (int)fwaterlog;

                                    globepoint checkpoint;

                                    if (goingnorth)
                                        checkpoint = dirpoint[index].north;
                                    else
                                        checkpoint = dirpoint[index].south;

                                    if (rainseed[newindex] < newwaterlog && rainseed[checkpoint.face * eedge + checkpoint.x * edge + checkpoint.y] < newwaterlog)
                                    {
                                        float waterdumped = (float)newwaterlog / (float)dumprate;
                                        float noslopewaterdumped = waterdumped;

                                        if (world.nom(newpoint.face, newpoint.x, newpoint.y) - sealevel > slopemin)
                                        {
                                            float slope = (float)getslope(world, face, x, y, newpoint.face, newpoint.x, newpoint.y);
                                            slope = slope / slopefactor;

                                            if (slope > 1.0f) // If it's going uphill
                                            {
                                                float waterdumped2 = waterdumped * slope;
                                                float elevation = (float)(world.nom(newpoint.face, newpoint.x, newpoint.y) - sealevel);
                                                waterdumped2 = waterdumped2 * elevationfactor * elevation;
                                                waterdumped = waterdumped2;

                                                if (waterdumped > waterlog)
                                                    waterdumped = waterlog;
                                            }
                                        }

                                        //waterlog = waterlog - waterdumped;
                                        waterlog = newwaterlog - waterdumped;

                                        if (slope > 1)
                                        {
                                            float slopewater = waterdumped - noslopewaterdumped;
                                            waterdumped = noslopewaterdumped + slopewater / (float)slopewaterreduce;
                                        }

                                        if (world.summerrain(newpoint.face, newpoint.x, newpoint.y) < (int)(waterdumped / 2.0f))
                                            world.setsummerrain(newpoint.face, newpoint.x, newpoint.y, (int)(waterdumped / 2.0f));

                                        if (world.winterrain(newpoint.face, newpoint.x, newpoint.y) < (int)(waterdumped / 2.0f))
                                            world.setwinterrain(newpoint.face, newpoint.x, newpoint.y, (int)(waterdumped / 2.0f));

                                        if (outsidehorse[newindex])
                                        {
                                            world.setmintemp(newpoint.face, newpoint.x, newpoint.y, world.mintemp(newpoint.face, newpoint.x, newpoint.y) + (int)waterheat);
                                            world.setmaxtemp(newpoint.face, newpoint.x, newpoint.y, world.maxtemp(newpoint.face, newpoint.x, newpoint.y) + (int)(waterheat * summerfactor));
                                        }

                                        raindir[newindex] = dir;
                                        raintick[newindex] = thistick + 1;
                                        rainseed[newindex] = newwaterlog;
                                        rainheatseed[newindex] = waterheat;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        thistick++;

        if (thistick > 10000) // Don't allow infinite loops
            found = 0;

    } while (found == 1);

    // Now adjust for seasonal variation on land.

    // Make a fractal to vary the fake monsoon effect.

    vector<int> fractal(6 * eedge, 0);

    int grain = 8; // Level of detail on this fractal map.
    float valuemod = 0.2f;
    float valuemod2 = 3.0f;

    createfractal(fractal, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    float minuslevel = 16.0f; // 14.0f;

    for (int face = 0; face < 6; face++)
    {
        int vface = face * eedge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                if (world.nom(face, i, j) > sealevel)
                {
                    float tempdiff = (float)(world.maxtemp(face, i, j) - world.mintemp(face, i, j));
                    float tempsim = minuslevel - tempdiff;

                    if (tempsim < 0.0f)
                        tempsim = 0.0f;

                    float minus = (minuslevel - tempdiff); 

                    if (minus < 0.0f)
                        minus = 0.0f;

                    minus = minus * ((float)fractal[index] / (float)maxelev) * 50.0f; // 60.0f;

                    if (minus > 400.0f)
                        minus = 400.0f;

                    minus = minus / 400.0f; // Square it to reduce the effect further from the equator.
                    minus = minus * minus;
                    minus = minus * 400.0f;
                       
                    if (world.winddir(face, i, j) == 0)
                        minus = minus / 2.0f;

                    float landelev = (float)(world.map(face, i, j) - sealevel);

                    minus = minus - (landelev / 10.0f);

                    if (minus < 0.0f)
                        minus = 0.0f;

                    //minus = 0.0f;

                    float multfactor = 0.03f;
                    int distance = inland[index];

                    if (distance > maxseasonaldistance)
                        distance = maxseasonaldistance;

                    float landdistvar = (float)distance * multfactor; // The further from land, the greater the seasonal variation

                    if (landdistvar < 1.0f)
                        landdistvar = 1.0f;

                    float winterrain = (float)world.winterrain(face, i, j);

                    if (outsidehorse[index])
                        tempdiff = tempdiff * seasonalvar * winterrain;
                    else
                        tempdiff = tempdiff * tropicalseasonalvar * winterrain;

                    tempdiff = tempdiff * landdistvar;

                    float oldwinterrain = (float)world.winterrain(face, i, j);
                    float oldsummerrain = (float)world.summerrain(face, i, j);

                    float newwinterrain = oldwinterrain + (int)(tempdiff - minus * 2.0f); // 3.5f // 2.0f
                    float newsummerrain = oldsummerrain - (int)(tempdiff - minus * 0.6f);

                    if (minus > 0.0f && newsummerrain > oldwinterrain * (tempsim / 1.5f))
                        newsummerrain = oldwinterrain * (tempsim / 1.5f);

                    if (newsummerrain < 0.0f)
                        newsummerrain = 0.0f;

                    if (newwinterrain < 0.0f)
                        newwinterrain = 0.0f;

                    world.setwinterrain(face, i, j, (int)newwinterrain);
                    world.setsummerrain(face, i, j, (int)newsummerrain);
                }
            }
        }
    }

    // Now increase the amounts.

    vector<int> rainadd(6 * eedge, 0);

    for (int face = 0; face < 6; face++)
    {
        int vface = face * eedge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                float tempdiff = (float)abs(world.mintemp(face, i, j) + world.maxtemp(face, i, j) - 30);

                tempdiff = 20.0f - tempdiff; // 15

                if (tempdiff < 0.0f)
                    tempdiff = 0.0f;

                int thisrainadd = (int)(tempdiff * 4.0f);

                float coastal = (float)(250 - inland[index]);

                if (coastal < 1.0f)
                    coastal = 1.0f;

                coastal = coastal / 250.0f;

                thisrainadd = (int)((float)thisrainadd * coastal);

                rainadd[index] = thisrainadd;
            }
        }
    }

    short span = 2;

    for (int face = 0; face < 6; face++)
    {
        int vface = face * eedge;

        for (int i = 0; i < edge; i++) // Blur this.
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                if (world.sea(face, i, j) == 0)
                {
                    int crount = 0;
                    int total = 0;

                    for (int k = - span; k <= span; k++)
                    {
                        for (int l = -span; l <= span; l++)
                        {
                            globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                            if (lpoint.face != -1)
                            {
                                crount++;
                                total = total + rainadd[lpoint.face * eedge + lpoint.x * edge + lpoint.y];
                            }
                        }
                    }

                    if (crount > 0)
                        rainadd[vi + j] = total / crount;
                }
            }
        }
    }

    for (int face = 0; face < 6; face++)
    {
        int vface = face * eedge;

        for (int i = 0; i < edge; i++) // Now add it, together with a general rainfall increase.
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                float summerrain = (float)world.summerrain(face, i, j);
                float winterrain = (float)world.winterrain(face, i, j);

                summerrain = summerrain * 1.20f + (float)rainadd[index] / 3.0f; // 1.3
                winterrain = winterrain * 1.20f + (float)rainadd[index];

                world.setsummerrain(face, i, j, (int)summerrain);
                world.setwinterrain(face, i, j, (int)winterrain);
            }
        }
    }

    // Possible blurring here

    short blurdist = 3; // 1;

    for (int face = 0; face < 6; face++)
    {
        int vface = face * eedge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                if (world.nom(face, i, j) > sealevel)
                {
                    short winddir = 0; // This is to avoid blurring rain over rain shadows.

                    if (world.wind(face, i, j) > 0 && world.wind(face, i, j) != 99)
                        winddir = 1; // Westerly

                    if (world.wind(face, i, j) < 0 || world.wind(face, i, j) == 99)
                        winddir = -1; // Easterly

                    short crount = 0;

                    int summertotal = 0;
                    int wintertotal = 0;

                    for (int k = -blurdist; k <= blurdist; k++)
                    {
                        for (int l = -blurdist; l <= blurdist; l++)
                        {
                            globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                            if (lpoint.face != -1 && world.nom(lpoint.face, lpoint.x, lpoint.y) > sealevel)
                            {
                                int lindex = lpoint.face * eedge + lpoint.x * edge + lpoint.y;

                                bool goahead = 1;

                                if (world.mountainheight(lpoint.face, lpoint.x, lpoint.y) > maxmountainheight)
                                    goahead = 0;

                                float thislong = longitude[index];
                                float thatlong = longitude[lindex];

                                if (abs(thislong - thatlong) > 100.0f) // If the split is this big, they must be crossing the international date line.
                                {
                                    float a = thislong;
                                    thislong = thatlong;
                                    thatlong = a;
                                }

                                if (winddir == 1)
                                {
                                    if (thatlong<thislong && world.map(lpoint.face, lpoint.x, lpoint.y)>world.map(face, i, j))
                                        goahead = 0;

                                    if (thatlong > thislong && world.map(lpoint.face, lpoint.x, lpoint.y) < world.map(face, i, j))
                                        goahead = 0;
                                }

                                if (winddir == -1)
                                {
                                    if (thatlong > thislong && world.map(lpoint.face, lpoint.x, lpoint.y) > world.map(face, i, j))
                                        goahead = 0;

                                    if (thatlong < thislong && world.map(lpoint.face, lpoint.x, lpoint.y) < world.map(face, i, j))
                                        goahead = 0;
                                }

                                if (goahead == 1)
                                {
                                    summertotal = summertotal + world.summerrain(lpoint.face, lpoint.x, lpoint.y);
                                    wintertotal = wintertotal + world.winterrain(lpoint.face, lpoint.x, lpoint.y);
                                    crount++;
                                }
                            }
                        }
                    }

                    if (crount > 0)
                    {
                        world.setsummerrain(face, i, j, summertotal / crount);
                        world.setwinterrain(face, i, j, wintertotal / crount);
                    }
                }
            }
        }
    }

    // Reduce seasonality near the equator.

    float limit = 0.5f; // 2.0f;

    for (int face = 0; face < 6; face++)
    {
        int vface = face * eedge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                float tempdiff = (float)(world.maxtemp(face, i, j) - world.mintemp(face, i, j));

                float thislimit = limit + ((float)fractal[vi + j] / (float)maxelev) * 4.0f;

                if (tempdiff < thislimit)
                {
                    float summerrain = (float)world.summerrain(face, i, j);
                    float winterrain = (float)world.winterrain(face, i, j);

                    float averain = (summerrain + winterrain) * 0.5f;

                    summerrain = (summerrain * tempdiff + averain * (thislimit - tempdiff)) / thislimit;
                    winterrain = (winterrain * tempdiff + averain * (thislimit - tempdiff)) / thislimit;

                    if (summerrain < 0.0f)
                        summerrain = 0.0f;

                    if (winterrain < 0.0f)
                        winterrain = 0.0f;

                    world.setsummerrain(face, i, j, (int)summerrain);
                    world.setwinterrain(face, i, j, (int)winterrain);
                }
            }
        }
    }
}

// This creates monsoons.

void createmonsoons(planet& world, int maxmountainheight, int slopewaterreduce, vector<float>& latitude, vector<fourglobepoints>& dirpoint)
{
    int edge = world.edge();
    int sealevel = world.sealevel();
    int maxelev = world.maxelevation();
    float tempdecrease = world.tempdecrease() * 10.0f; // The default is 6.5 * 10.0f = 65.0.

    float monstrength = 2.0f - (float)(abs(32.5f - world.tilt()) / 10.0f); // Overall strength of monsoons is affected by axial tilt. High with medium tilt, low with low or high tilt.

    monstrength = monstrength * 3.0f;

    if (monstrength > 0.0)
    {
        int minmonsoondiff = 1; // 2; // Winter/summer temperatures must be this far apart for monsoons to happen.
        int minmonsoontemp = 15; // Average annual temperature must be at least this for monsoons to happen.
        float monsoondifffactor = 2.5f; // 1.0f;  // For every degree of difference between summer and winter temperatures, add this to get the base monsoon strength.
        float monsoontempfactor = 400.0f; //250.0f; // Multiply this by the average temperature and add it to the monsoon strength.
        float monsoononinlandtempfactor = 25.0f; // The higher this is, the more colder temperatures will reduce the monsoon as it travels inland.
        float mintidefactor = 0.2f; // Lowest amount tides affect monsoon strength.
        float monsoonincrease = 1.8f; // Initial amount to increase monsoon strength by as it spreads inland.
        float monsoonincreasedecrease = 0.015f; //0.02; // Amount to reduce monsoonincrease by each tick.
        float minmonsoonincrease = 0.99f; // The moonsoon strength can't be reduced more than this per tick.
        int monsoondumprate = 15; // Amount monsoon gets dumped along the way.
        float monsoonelevationfactor = 0.01f; // This determines how much elevation affects the degree to which gradient affects monsoons.
        int monsoonslopemin = 200; // This is how high land has to be before the gradient affects monsoons.
        int monsoonswervechance = 2; // The lower this is, the more swervy monsoons will be.
        float maxsummerrain = 410.0f; // Maximum amount of new rain that the monsoon can bring in the summer.
        int monsoonequatordist = 30; // If it's closer to the equator than this, start reducing the monsoon.
        float monsoonequatorfactor = 0.25f; // If it's close to the equator, multiply by this.
        int monsoonminequatordist = 10; // No monsoon at all closer to the equator than this.
        int maxtick = 1500; // Stop after this many ticks.

        int monsoonslopefactor = 77 - (int)tempdecrease; //12; // This determines how much gradient affects rainfall. The lower it is, the more it affects it.

        if (monsoonslopefactor < 1)
            monsoonslopefactor = 1;

        float wintermonsoonfactor = 1.0f; // Multiply monsoon strength by this to see how much rain is suppressed in winter.
        float summermonsoonfactor = 1.0f; // Multiply monsoon strength by this to see how much rain is added in summer.

        vector<int> raintick(6 * edge * edge, 0);
        vector<short> raindir(6 * edge * edge, 0);
        vector<float> monsoonstrength(6 * edge * edge, 0);
        vector<int> monsoonmap(6 * edge * edge, 0);

        vector<uint8_t> is_sea(6 * edge * edge, 0);

        for (int face = 0; face < 6; face++)
        {
            int vface = face * edge * edge;

            for (int i = 0; i < edge; i++)
            {
                int vi = vface + i * edge;

                for (int j = 0; j < edge; j++)
                {
                    is_sea[vi + j] = world.sea(face, i, j);
                }
            }
        }

        // First, set the initial seeds along the coasts.

        for (int face = 0; face < 6; face++)
        {
            int vface = face * edge * edge;

            for (int i = 0; i < edge; i++)
            {
                int vi = vface + i * edge;

                for (int j = 0; j < edge; j++)
                {
                    int index = vi + j;

                    if (1==1) //(latitude[face][i][j]<-monsoonminequatordist || latitude[face][i][j]>monsoonminequatordist)
                    {
                        if (is_sea[index] == 1) // && j >= world.horse(face, i, 2) && j <= world.horse(face, i, 3))
                        {
                            bool found = 0;

                            for (int k = - 1; k <= 1; k++)
                            {
                                for (int l = -1; l <= 1; l++)
                                {
                                    globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                                    if (lpoint.face != -1)
                                    {
                                        if (is_sea[lpoint.face * edge * edge + lpoint.x * edge + lpoint.y] == 0)
                                        {
                                            found = 1;
                                            k = 1;
                                            l = 1;
                                        }
                                    }
                                }
                            }

                            if (found == 1)
                            {
                                int avetemp = (world.maxtemp(face, i, j) + world.mintemp(face, i, j)) / 2;
                                int tempdiff = world.maxtemp(face, i, j) - world.mintemp(face, i, j);

                                if (avetemp >= minmonsoontemp && tempdiff >= minmonsoondiff)
                                {
                                    float monsoon = tempdiff * monsoondifffactor;
                                    monsoon = monsoon + (avetemp * monsoontempfactor);

                                    float tidefactor = (float)world.tide(face, i, j);
                                    tidefactor = tidefactor / 10.0f;

                                    if (tidefactor < mintidefactor)
                                        tidefactor = mintidefactor;

                                    monsoon = monsoon * tidefactor;

                                    monsoonstrength[index] = monsoon;
                                    raintick[index] = 1;
                                }
                            }
                        }
                    }
                }
            }
        }

        // Now we spread those seeds across the land.

        int thistick = 1; // Tracks the time. Only do cells that are set to be done on this turn.
        bool found = 0;

        do
        {
            for (int face = 0; face < 6; face++)
            {
                int vface = face * edge * edge;

                for (int a = 0; a < edge; a++)
                {
                    int va = vface + a * edge;

                    for (int b = 0; b < edge; b++)
                    {
                        int index = va + b;

                        int x = a;
                        int y = b; // This is so we can move them later without messing up the loop.

                        if (is_sea[index] == 0 && raintick[index] == 0) // && random(1,maxelev)<spreadfractal[x][y])
                        {
                            bool nearby = 0;

                            for (int i = - 1; i <= 1; i++)
                            {
                                globepoint ipoint = getglobepoint(edge, face, x, y, i, 0);
                                
                                if (ipoint.face != -1)
                                {
                                    for (int j = - 1; j <= 1; j++)
                                    {
                                        globepoint jpoint = getglobepoint(edge, ipoint.face, ipoint.x, ipoint.y, 0, j);

                                        if (jpoint.face!=-1)
                                        {
                                            if (raintick[jpoint.face * edge * edge + jpoint.x * edge + jpoint.y] == thistick)
                                            {
                                                nearby = 1;
                                                i = 1;
                                                j = 1;
                                            }
                                        }
                                    }
                                }
                            }

                            if (nearby == 1) // This is a cell next to a monsoon cell!
                            {
                                found = 1;

                                raintick[index] = thistick + 1; // This will be a seed for the next round.

                                // Work out the strength and direction of the monsoon here.

                                float waterlog = 0; // It's more complicated than that, because in winter it's dryness, but we'll call it this for simplicity's sake.
                                float crount = 0;
                                float total = 0;

                                short north = 0;
                                short northeast = 0;
                                short east = 0;
                                short southeast = 0;
                                short south = 0;
                                short southwest = 0;
                                short west = 0;
                                short northwest = 0;

                                globepoint northpoint = dirpoint[index].north;
                                globepoint eastpoint = dirpoint[index].east;
                                globepoint southpoint = dirpoint[index].south;
                                globepoint westpoint = dirpoint[index].west;

                                int northindex = northpoint.face * edge * edge + northpoint.x * edge + northpoint.y;
                                int southindex = southpoint.face * edge * edge + southpoint.x * edge + southpoint.y;
                                int eastindex = eastpoint.face * edge * edge + eastpoint.x * edge + eastpoint.y;
                                int westindex = westpoint.face * edge * edge + westpoint.x * edge + westpoint.y;

                                globepoint northeastpoint = dirpoint[northindex].east;
                                globepoint northwestpoint = dirpoint[northindex].west;
                                globepoint southeastpoint = dirpoint[southindex].east;
                                globepoint southwestpoint = dirpoint[southindex].west;

                                int northeastindex = northeastpoint.face * edge * edge + northeastpoint.x * edge + northeastpoint.y;
                                int northwestindex = northwestpoint.face * edge * edge + northwestpoint.x * edge + northwestpoint.y;
                                int southeastindex = southeastpoint.face * edge * edge + southeastpoint.x * edge + southeastpoint.y;
                                int southwestindex = southwestpoint.face * edge * edge + southwestpoint.x * edge + southwestpoint.y;

                                if (raintick[northindex] == thistick)
                                {
                                    if (monsoonstrength[northindex] > 0)
                                    {
                                        northwest++;
                                        north = north + 2;
                                        northeast++;
                                    }

                                    crount++;
                                    total = total + monsoonstrength[northindex];
                                }

                                if (raintick[northeastindex] == thistick)
                                {
                                    if (monsoonstrength[northeastindex] > 0)
                                    {
                                        north++;
                                        northeast = northeast + 2;
                                        east++;
                                    }

                                    crount++;
                                    total = total + monsoonstrength[northeastindex];
                                }

                                if (raintick[eastindex] == thistick)
                                {
                                    if (monsoonstrength[eastindex] > 0)
                                    {
                                        northeast++;
                                        east = east + 2;
                                        southeast++;
                                    }

                                    crount++;
                                    total = total + monsoonstrength[eastindex];
                                }

                                if (raintick[southeastindex] == thistick)
                                {
                                    if (monsoonstrength[southeastindex] > 0)
                                    {
                                        east++;
                                        southeast = southeast + 2;
                                        south++;
                                    }

                                    crount++;
                                    total = total + monsoonstrength[southeastindex];
                                }

                                if (raintick[southindex] == thistick)
                                {
                                    if (monsoonstrength[southindex] > 0)
                                    {
                                        southeast++;
                                        south = south + 2;
                                        southwest++;
                                    }

                                    crount++;
                                    total = total + monsoonstrength[southindex];
                                }

                                if (raintick[southwestindex] == thistick)
                                {
                                    if (monsoonstrength[southwestindex] > 0)
                                    {
                                        south++;
                                        southwest = southwest + 2;
                                        west++;
                                    }

                                    crount++;
                                    total = total + monsoonstrength[southwestindex];
                                }

                                if (raintick[westindex] == thistick)
                                {
                                    if (monsoonstrength[westindex] > 0)
                                    {
                                        southwest++;
                                        west = west + 2;
                                        northwest++;
                                    }

                                    crount++;
                                    total = total + monsoonstrength[westindex];
                                }

                                if (raintick[northwestindex] == thistick)
                                {
                                    if (monsoonstrength[northwestindex] > 0)
                                    {
                                        west++;
                                        northwest = northwest + 2;
                                        north++;
                                    }

                                    crount++;
                                    total = total + monsoonstrength[northwestindex];
                                }

                                waterlog = total / crount;
                                waterlog = waterlog * monsoonincrease;

                                float tempadjust = (float)world.avetemp(face, x, y);
                                tempadjust = tempadjust / monsoononinlandtempfactor;

                                if (tempadjust > 1.0f)
                                    tempadjust = 1.0f;

                                waterlog = waterlog * tempadjust;

                                int equatordist = abs((int)latitude[index]);

                                if (equatordist > monsoonminequatordist)
                                {
                                    if (equatordist < monsoonequatordist)
                                    {
                                        if (random(1, monsoonequatordist) > equatordist)
                                            waterlog = waterlog * monsoonequatorfactor;
                                    }

                                    tempadjust = (float)world.avetemp(face, x, y);

                                    tempadjust = tempadjust * 0.05f;

                                    if (tempadjust < 0.0f)
                                        tempadjust = 0.0f;

                                    if (tempadjust > 1.0f)
                                        tempadjust = 1.0f;

                                    waterlog = waterlog * tempadjust;

                                    short dir = 5;

                                    short dirtotal = north;

                                    if (northeast > dirtotal)
                                    {
                                        dirtotal = northeast;
                                        dir = 6;
                                    }

                                    if (east > dirtotal)
                                    {
                                        dirtotal = east;
                                        dir = 7;
                                    }

                                    if (southeast > dirtotal)
                                    {
                                        dirtotal = southeast;
                                        dir = 8;
                                    }

                                    if (south > dirtotal)
                                    {
                                        dirtotal = south;
                                        dir = 1;
                                    }

                                    if (southwest > dirtotal)
                                    {
                                        dirtotal = southwest;
                                        dir = 2;
                                    }

                                    if (west > dirtotal)
                                    {
                                        dirtotal = west;
                                        dir = 3;
                                    }

                                    if (northwest > dirtotal)
                                    {
                                        dirtotal = northwest;
                                        dir = 4;
                                    }

                                    // We have the monsoon strength and the direction. Now we just need to deposit some monsoon here and reduce the strength accordingly.

                                    int waterdumped = (int)(waterlog / (float)monsoondumprate);
                                    int noslopewaterdumped = waterdumped;

                                    float slopereduce = 0.0f;

                                    float slope = 0.0f;

                                    if (thistick > 2 && world.nom(face, x, y) - sealevel > monsoonslopemin)
                                    {
                                        globepoint upwind;

                                        if (dir == 1)
                                            upwind = northpoint;

                                        if (dir == 2)
                                            upwind = northeastpoint;

                                        if (dir == 3)
                                            upwind = eastpoint;

                                        if (dir == 4)
                                            upwind = southeastpoint;

                                        if (dir == 5)
                                            upwind = southpoint;

                                        if (dir == 6)
                                            upwind = southwestpoint;

                                        if (dir == 7)
                                            upwind = westpoint;

                                        if (dir == 8)
                                            upwind = northwestpoint;

                                        slope = (float)getslope(world, upwind.face, upwind.x, upwind.y, face, x, y);
                                        slope = slope / monsoonslopefactor;

                                        if (slope > 1.0f) // If it's going uphill
                                        {
                                            float waterdumped2 = waterdumped * slope;
                                            float elevation = (float)(world.nom(face, x, y) - sealevel);
                                            waterdumped2 = waterdumped2 * monsoonelevationfactor * elevation;
                                            waterdumped = (int)waterdumped2;

                                            if (waterdumped > (int)waterlog)
                                                waterdumped = (int)waterlog;
                                        }
                                    }

                                    waterlog = waterlog - (float)waterdumped;

                                    globepoint newpoint;
                                    newpoint.face = face;
                                    newpoint.x = x;
                                    newpoint.y = y;

                                    if (random(1, monsoonswervechance) == 1)
                                    {
                                        raintick[index] = 0;

                                        if (dir == 1 || dir == 5)
                                        {
                                            if (random(1, 2) == 1)
                                                newpoint = eastpoint;
                                            else
                                                newpoint = westpoint;
                                        }

                                        if (dir == 2 || dir == 6)
                                        {
                                            if (random(1, 2) == 1)
                                                newpoint = northwestpoint;
                                            else
                                                newpoint = southeastpoint;
                                        }

                                        if (dir == 3 || dir == 7)
                                        {
                                            if (random(1, 2) == 1)
                                                newpoint = northpoint;
                                            else
                                                newpoint = southpoint;
                                        }

                                        if (dir == 4 || dir == 8)
                                        {
                                            if (random(1, 2) == 1)
                                                newpoint = southwestpoint;
                                            else
                                                newpoint = northeastpoint;
                                        }

                                        raintick[newpoint.face * edge * edge + newpoint.x * edge + newpoint.y] = thistick + 1;
                                    }

                                    if (slope > 0.0f)
                                    {
                                        int slopewater = waterdumped - noslopewaterdumped;
                                        waterdumped = waterdumped + (int)((float)slopewater / ((float)slopewaterreduce * 5.0f));
                                    }

                                    int nindex = newpoint.face * edge * edge + newpoint.x * edge + newpoint.y;

                                    if (waterdumped > monsoonmap[nindex])
                                        monsoonmap[nindex] = waterdumped;

                                    monsoonstrength[nindex] = waterlog;
                                }
                                else
                                    monsoonmap[index] = 0;
                            }
                        }
                    }
                }
            }

            thistick++;

            if (thistick > maxtick)
                found = 0;

            monsoonincrease = monsoonincrease - monsoonincreasedecrease;

            if (monsoonincrease < minmonsoonincrease)
                monsoonincrease = minmonsoonincrease;


        } while (found == 1);

        for (int n = 0; n < 15; n++) // Blur it.
        {
            for (int face = 0; face < 6; face++)
            {
                int vface = face * edge * edge;

                for (int i = 0; i < edge; i++) // Normal blur.
                {
                    int vi = vface + i * edge;

                    for (int j = 0; j < edge; j++)
                    {
                        int index = vi + j;

                        if (is_sea[index] == 1)
                            monsoonmap[index] = 0;
                        else
                        {
                            if (world.mountainheight(face, i, j) < maxmountainheight)
                            {
                                float crount = 0;
                                float total = 0;

                                globepoint grid[5][5]; // This grid will contain the coordinates of the surrounding points.

                                grid[2][2].face = face;
                                grid[2][2].x = i;
                                grid[2][2].y = j;

                                grid[2][1] = dirpoint[index].north;
                                grid[3][2] = dirpoint[index].east;
                                grid[2][3] = dirpoint[index].south;
                                grid[1][2] = dirpoint[index].west;

                                int northindex = grid[2][1].face * edge * edge + grid[2][1].x * edge + grid[2][1].y;
                                int southindex = grid[2][3].face * edge * edge + grid[2][3].x * edge + grid[2][3].y;
                                int eastindex = grid[3][2].face * edge * edge + grid[3][2].x * edge + grid[3][2].y;
                                int westindex = grid[1][2].face * edge * edge + grid[1][2].x * edge + grid[1][2].y;

                                grid[1][1] = dirpoint[northindex].west;
                                grid[3][1] = dirpoint[northindex].east;
                                grid[1][3] = dirpoint[southindex].west;
                                grid[3][3] = dirpoint[southindex].east;

                                int northwestindex = grid[1][1].face * edge * edge + grid[1][1].x * edge + grid[1][1].y;
                                int northeastindex = grid[3][1].face * edge * edge + grid[3][1].x * edge + grid[3][1].y;
                                int southwestindex = grid[1][3].face * edge * edge + grid[1][3].x * edge + grid[1][3].y;
                                int southeastindex = grid[3][3].face * edge * edge + grid[3][3].x * edge + grid[3][3].y;

                                grid[1][0] = dirpoint[northwestindex].north;
                                grid[2][0] = dirpoint[northindex].north;
                                grid[3][0] = dirpoint[northeastindex].north;

                                grid[1][4] = dirpoint[southwestindex].south;
                                grid[2][4] = dirpoint[southindex].south;
                                grid[3][4] = dirpoint[southeastindex].south;

                                grid[0][1] = dirpoint[northwestindex].west;
                                grid[0][2] = dirpoint[westindex].west;
                                grid[0][3] = dirpoint[southwestindex].west;

                                grid[4][1] = dirpoint[northeastindex].east;
                                grid[4][2] = dirpoint[eastindex].east;
                                grid[4][3] = dirpoint[southeastindex].east;

                                grid[0][0] = dirpoint[grid[1][0].face * edge * edge + grid[1][0].x * edge + grid[1][0].y].west;
                                grid[4][0] = dirpoint[grid[3][0].face * edge * edge + grid[3][0].x * edge + grid[3][0].y].east;
                                grid[4][4] = dirpoint[grid[3][4].face * edge * edge + grid[3][4].x * edge + grid[3][4].y].east;
                                grid[0][4] = dirpoint[grid[1][4].face * edge * edge + grid[1][4].x * edge + grid[1][4].y].west;

                                for (int k = 0; k < 5; k++)
                                {
                                    for (int l = 0; l < 5; l++)
                                    {
                                        if (world.mountainheight(grid[k][l].face, grid[k][l].x, grid[k][l].y) < maxmountainheight)
                                        {
                                            total = total + monsoonmap[grid[k][l].face * edge * edge + grid[k][l].x * edge + grid[k][l].y];
                                            crount++;
                                        }
                                    }
                                }

                                if (crount > 0)
                                {
                                    total = total / crount;

                                    monsoonmap[index] = (int)total;
                                }
                            }
                        }
                    }
                }
            }
        }

        // Now we need to use the monsoon map to tinker with the rainfall map.

        float monsoonelevfactor = 0.5f; // 8.0f; // The higher this is, the more elevation will reduce the extra monsoon rain in summer.

        vector<int> summermonsoon(6 * edge * edge, 0);
        vector<int> wintermonsoon(6 * edge * edge, 0);

        for (int face = 0; face < 6; face++)
        {
            int vface = face * edge * edge;

            for (int i = 0; i < edge; i++)
            {
                int vi = vface + i * edge;

                for (int j = 0; j < edge; j++)
                {
                    int index = vi + j;

                    if (monsoonmap[index] > 0)
                    {
                        float landelev = (float)(world.nom(face, i, j) - sealevel);

                        landelev = landelev * monsoonelevfactor;

                        monsoonmap[index] = monsoonmap[index] - (int)landelev;

                        if (monsoonmap[index] < 0)
                            monsoonmap[index] = 0;
                    }
                }
            }
        }

        for (int face = 0; face < 6; face++)
        {
            int vface = face * edge * edge;

            for (int i = 0; i < edge; i++) // Make arrays of the new monsoon rainfall.
            {
                int vi = vface + i * edge;

                for (int j = 0; j < edge; j++)
                {
                    int index = vi + j;

                    float monsoonstrength = (float)monsoonmap[index];

                    if (monsoonstrength != 0.0f && monsoonstrength != -1.0f)
                    {
                        wintermonsoon[index] = (int)(monsoonstrength * wintermonsoonfactor);
                        summermonsoon[index] = (int)(monsoonstrength * summermonsoonfactor);

                        if (summermonsoon[index] > (int)maxsummerrain)
                            summermonsoon[index] = (int)maxsummerrain;
                    }
                }
            }
        }

        // Now just blur that monsoon rainfall too.

        for (int n = 0; n < 20; n++)
        {
            for (int face = 0; face < 6; face++)
            {
                int vface = face * edge * edge;

                for (int i = 0; i < edge; i++) // Normal blur.
                {
                    int vi = vface + i * edge;

                    for (int j = 0; j < edge; j++)
                    {
                        int index = vface + j;

                        if (is_sea[index] == 0)
                        {
                            if (world.mountainheight(face, i, j) < maxmountainheight)
                            {
                                float crount = 0;
                                float summertotal = 0;

                                globepoint grid[5][5]; // This grid will contain the coordinates of the surrounding points.

                                grid[2][2].face = face;
                                grid[2][2].x = i;
                                grid[2][2].y = j;

                                grid[2][1] = dirpoint[index].north;
                                grid[3][2] = dirpoint[index].east;
                                grid[2][3] = dirpoint[index].south;
                                grid[1][2] = dirpoint[index].west;

                                int northindex = grid[2][1].face * edge * edge + grid[2][1].x * edge + grid[2][1].y;
                                int southindex = grid[2][3].face * edge * edge + grid[2][3].x * edge + grid[2][3].y;
                                int eastindex = grid[3][2].face * edge * edge + grid[3][2].x * edge + grid[3][2].y;
                                int westindex = grid[1][2].face * edge * edge + grid[1][2].x * edge + grid[1][2].y;

                                grid[1][1] = dirpoint[northindex].west;
                                grid[3][1] = dirpoint[northindex].east;
                                grid[1][3] = dirpoint[southindex].west;
                                grid[3][3] = dirpoint[southindex].east;

                                int northwestindex = grid[1][1].face * edge * edge + grid[1][1].x * edge + grid[1][1].y;
                                int northeastindex = grid[3][1].face * edge * edge + grid[3][1].x * edge + grid[3][1].y;
                                int southwestindex = grid[1][3].face * edge * edge + grid[1][3].x * edge + grid[1][3].y;
                                int southeastindex = grid[3][3].face * edge * edge + grid[3][3].x * edge + grid[3][3].y;

                                grid[1][0] = dirpoint[northwestindex].north;
                                grid[2][0] = dirpoint[northindex].north;
                                grid[3][0] = dirpoint[northeastindex].north;

                                grid[1][4] = dirpoint[southwestindex].south;
                                grid[2][4] = dirpoint[southindex].south;
                                grid[3][4] = dirpoint[southeastindex].south;

                                grid[0][1] = dirpoint[northwestindex].west;
                                grid[0][2] = dirpoint[westindex].west;
                                grid[0][3] = dirpoint[southwestindex].west;

                                grid[4][1] = dirpoint[northeastindex].east;
                                grid[4][2] = dirpoint[eastindex].east;
                                grid[4][3] = dirpoint[southeastindex].east;

                                grid[0][0] = dirpoint[grid[1][0].face * edge * edge + grid[1][0].x * edge + grid[1][0].y].west;
                                grid[4][0] = dirpoint[grid[3][0].face * edge * edge + grid[3][0].x * edge + grid[3][0].y].east;
                                grid[4][4] = dirpoint[grid[3][4].face * edge * edge + grid[3][4].x * edge + grid[3][4].y].east;
                                grid[0][4] = dirpoint[grid[1][4].face * edge * edge + grid[1][4].x * edge + grid[1][4].y].west;

                                for (int k = 0; k < 5; k++) 
                                {
                                    for (int l = 0; l < 5; l++)
                                    {
                                        int gindex = grid[k][l].face * edge * edge + grid[k][l].x * edge + grid[k][l].y;
                                        if (is_sea[gindex] == 0 && world.mountainheight(grid[k][l].face, grid[k][l].x, grid[k][l].y) < maxmountainheight)
                                        {
                                            summertotal = summertotal + summermonsoon[gindex];
                                            crount++;
                                        }
                                    }
                                }

                                if (crount > 0)
                                {
                                    summertotal = summertotal / crount;

                                    summermonsoon[index] = (int)summertotal;
                                }
                            }
                        }
                    }
                }
            }
        }

        // Now use a fractal to add variation to the monsoon rainfall.

        vector<int> monsoonvary(6 * edge * edge, 0);

        int grain = 16; // Level of detail on this fractal map.
        float valuemod = 0.2f;
        float valuemod2 = 3.0f;

        createfractal(monsoonvary, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

        float monsoondiv = 1.6f / (float)maxelev;

        for (int face = 0; face < 6; face++)
        {
            int vface = face * edge * edge;

            for (int i = 0; i < edge; i++) // Now apply that fractal.
            {
                int vi = vface + i * edge;

                for (int j = 0; j < edge; j++)
                {
                    int index = vi + j;

                    float summerrain = (float)summermonsoon[index];
                    float winterrain = (float)wintermonsoon[index];

                    if (summerrain != 0.0f || winterrain != 0.0f)
                    {
                        float amount = (float)monsoonvary[index] * monsoondiv;

                        amount = amount - 0.8f;

                        summerrain = summerrain + summerrain * amount;
                        winterrain = winterrain + winterrain * amount;

                        summermonsoon[index] = (int)summerrain;
                        wintermonsoon[index] = (int)winterrain;
                    }
                }
            }
        }

        for (int face = 0; face < 6; face++)
        {
            int vface = face * edge * edge;

            for (int i = 0; i < edge; i++) // Now apply that rainfall.
            {
                int vi = vface + i * edge;

                for (int j = 0; j < edge; j++)
                {
                    int index = vi + j;

                    float wmonsoon = (float)wintermonsoon[index];
                    float smonsoon = (float)summermonsoon[index];

                    wmonsoon = wmonsoon * monstrength;
                    smonsoon = smonsoon * monstrength;

                    int winterrain = world.winterrain(face, i, j) - (int)wmonsoon;

                    if (winterrain < 0)
                        winterrain = 0;

                    int summerrain = world.summerrain(face, i, j) + (int)smonsoon;

                    if (summerrain > (int)maxsummerrain)
                        summerrain = (int)maxsummerrain;

                    world.setwinterrain(face, i, j, winterrain);
                    world.setsummerrain(face, i, j, summerrain);
                }
            }
        }
    }
}

// This adjusts the seasonal rainfall (to ensure certain climates)

void adjustseasonalrainfall(planet& world, vector<int>& inland, vector<float>& latitude, vector<fourglobepoints>& dirpoint, vector<bool>& outsidehorse)
{
    int edge = world.edge();
    int sealevel = world.sealevel();
    int maxelev = world.maxelevation();
    float tilt = world.tilt();

    // First, do some tinkering to encourage Mediterranean climates.

    vector<int> mediterranean(6 * edge * edge, 0);

    float medstrength = 1.8f - (abs(31.5f - tilt) / 10.0f); // The higher this is, the more of an effect we'll see. It's high for moderate tilt values, but low for low or high tilt values.

    if (medstrength > 0.0f)
    {
        int avemedmaxtemp = 19;
        float medmaxtempdifffactor = 0.001f;
        float minmedcoldtemp = 2.0f;
        float medmintempdifffactor = 0.001f;
        int maxmedrain = 500;
        float medmaxraindifffactor = 0.001f;
        int maxmedinland = 40; //40;
        float medmaxinlanddifffactor = 0.01f;
        float medhorsedifffactor = 0.02f;

        for (int face = 0; face < 6; face++)
        {
            int vface = face * edge * edge;

            for (int i = 0; i < edge; i++)
            {
                int vi = vface + i * edge;

                for (int j = 0; j < edge; j++)
                {
                    int index = vi + j;

                    if (world.sea(face, i, j) == 0)
                    {
                        float med = 1.0f;

                        float diff = (float)(abs(world.maxtemp(face, i, j) - avemedmaxtemp));

                        med = med - diff * medmaxtempdifffactor;

                        diff = minmedcoldtemp - world.mintemp(face, i, j);

                        if (diff > 0.0f)
                            med = med - diff * medmintempdifffactor;

                        diff = (float)(world.averain(face, i, j) - maxmedrain);

                        if (diff > 0.0f)
                            med = med - diff * medmaxraindifffactor;

                        diff = (float)(inland[index] - maxmedinland);

                        if (diff > 0.0f)
                            med = med - diff * medmaxinlanddifffactor;

                        if (latitude[index] < 0.0f)
                        {
                            if (outsidehorse[index]) // North of the horse latitudes
                            {
                                int thisdist = 0;

                                globepoint thispoint;
                                thispoint.face = face;
                                thispoint.x = i;
                                thispoint.y = j;

                                bool keepgoing = 1;

                                do
                                {
                                    thispoint = dirpoint[thispoint.face * edge * edge + thispoint.x * edge + thispoint.y].south;

                                    thisdist++;

                                    if (outsidehorse[thispoint.face * edge * edge + thispoint.x * edge + thispoint.y] == 0)
                                        keepgoing = 0;

                                    if (thisdist > edge)
                                        keepgoing = 0;

                                } while (keepgoing);

                                diff = (float)thisdist;
                            }
                            else
                            {
                                int thisdist = 0;

                                globepoint thispoint;
                                thispoint.face = face;
                                thispoint.x = i;
                                thispoint.y = j;

                                bool keepgoing = 1;

                                do
                                {
                                    thispoint = dirpoint[thispoint.face * edge * edge + thispoint.x * edge + thispoint.y].north;

                                    thisdist++;

                                    if (outsidehorse[thispoint.face * edge * edge + thispoint.x * edge + thispoint.y])
                                        keepgoing = 0;

                                    if (thisdist > edge)
                                        keepgoing = 0;

                                } while (keepgoing);

                                diff = (float)(thisdist * 2);
                            }
                        }
                        else
                        {
                            if (outsidehorse[index]) // South of the horse latitudes
                            {
                                int thisdist = 0;

                                globepoint thispoint;
                                thispoint.face = face;
                                thispoint.x = i;
                                thispoint.y = j;

                                bool keepgoing = 1;

                                do
                                {
                                    thispoint = dirpoint[thispoint.face * edge * edge + thispoint.x * edge + thispoint.y].north;

                                    thisdist++;

                                    if (outsidehorse[thispoint.face * edge * edge + thispoint.x * edge + thispoint.y] == 0)
                                        keepgoing = 0;

                                    if (thisdist > edge)
                                        keepgoing = 0;

                                } while (keepgoing);

                                diff = (float)thisdist;
                            }
                            else
                            {
                                int thisdist = 0;

                                globepoint thispoint;
                                thispoint.face = face;
                                thispoint.x = i;
                                thispoint.y = j;

                                bool keepgoing = 1;

                                do
                                {
                                    thispoint = dirpoint[thispoint.face * edge * edge + thispoint.x * edge + thispoint.y].south;

                                    thisdist++;

                                    if (outsidehorse[thispoint.face * edge * edge + thispoint.x * edge + thispoint.y])
                                        keepgoing = 0;

                                    if (thisdist > edge)
                                        keepgoing = 0;

                                } while (keepgoing);

                                diff = (float)(thisdist * 2);
                            }
                        }

                        med = med - diff * medhorsedifffactor;

                        if (med > 0.0f)
                            mediterranean[index] = (int)med;

                    }
                }
            }
        }

        int dist = 3;

        for (int face = 0; face < 6; face++) // Smooth it
        {
            int vface = face * edge * edge;

            for (int i = 0; i < edge; i++)
            {
                int vi = vface + i * edge;

                for (int j = 0; j < edge; j++)
                {
                    float total = 0;
                    float crount = 0.0f;

                    for (int k = -dist; k <= dist; k++)
                    {
                        for (int l = -dist; l <= dist; l++)
                        {
                            globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                            if (lpoint.face != -1)
                            {
                                total = total + mediterranean[lpoint.face * edge * edge + lpoint.x * edge + lpoint.y];
                                crount++;
                            }
                        }
                    }

                    if (crount > 0.0f)
                        mediterranean[vi + j] = (int)(total / crount);
                }
            }
        }

        for (int face = 0; face < 6; face++) // Now use it to alter the seasonal variation in rainfall.
        {
            int vface = face * edge * edge;

            for (int i = 0; i < edge; i++)
            {
                int vi = vface + i * edge;

                for (int j = 0; j < edge; j++)
                {
                    int index = vi + j;

                    if (mediterranean[index] != 0)
                    {
                        float winterrain = (float)world.winterrain(face, i, j);
                        float summerrain = (float)world.summerrain(face, i, j);

                        float rainshift = summerrain * medstrength * mediterranean[index];

                        winterrain = winterrain + rainshift;
                        summerrain = summerrain - rainshift;

                        world.setwinterrain(face, i, j, (int)winterrain);
                        world.setsummerrain(face, i, j, (int)summerrain);
                    }
                }
            }
        }
    }

    // Now some tinkering to encourage rainforest near the equator.

    /*

    float rainstrength = 2.0f - (abs(32.5f - tilt) / 10.0f); // As for medstrength above.

    if (rainstrength > 0.0f)
    {
        float winteradd = 0.3f * rainstrength; // 1.0f;
        float summeradd = 0.4f * rainstrength; // 1.6f; // 0.4f;

        float equator = float(edge / 2);

        for (int face = 0; face < 4; face++)
        {
            for (int i = 0; i < edge; i++)
            {
                float tropicnorth = (float)edge / 4;
                float tropicnorthheight = equator - tropicnorth;
                float winterstep = winteradd / tropicnorthheight;
                float summerstep = summeradd / tropicnorthheight;

                float currentwinteradd = 0.0f;
                float currentsummeradd = 0.0f;

                for (int j = (int)tropicnorth; j <= (int)equator; j++)
                {
                    currentwinteradd = currentwinteradd + winterstep;
                    currentsummeradd = currentsummeradd + summerstep;

                    float currentjan = float(world.janrain(face, i, j));
                    float currentjul = float(world.julrain(face, i, j));

                    currentjan = currentjan + currentjan * currentwinteradd;
                    currentjul = currentjul + currentjul * currentsummeradd;

                    world.setjanrain(face, i, j, (int)currentjan);
                    world.setjulrain(face, i, j, (int)currentjul);
                }

                float tropicsouth = edge-tropicnorth;
                float tropicsouthheight = tropicsouth - equator;
                winterstep = winteradd / tropicsouthheight;
                summerstep = summeradd / tropicsouthheight;

                currentwinteradd = 0.0f;
                currentsummeradd = 0.0f;

                for (int j = (int)tropicsouth; j > (int)equator; j--)
                {
                    currentwinteradd = currentwinteradd + winterstep;
                    currentsummeradd = currentsummeradd + summerstep;

                    float currentjan = float(world.janrain(face, i, j));
                    float currentjul = float(world.julrain(face, i, j));

                    currentjan = currentjan + currentjan * currentsummeradd;
                    currentjul = currentjul + currentjul * currentwinteradd;

                    world.setjanrain(face, i, j, (int)currentjan);
                    world.setjulrain(face, i, j, (int)currentjul);
                }
            }
        }

        // And now some more tinkering, to encourage savannah nearer the edge of the tropics.

        float janadd = -1.2f * rainstrength; // 0.8f;
        float juladd = 0.0f * rainstrength; // 1.0f; // 0.6f;
        float bandwidth = 8.0f; // The strip affected will extend by this much to the north and south of the boundary of the horse latitudes.

        float tropicnorth = -12.0f;
        float tropicsouth = 12.0f;

        for (int face = 0; face < 6; face++)
        {
            for (int i = 0; i < edge; i++)
            {
                float currentjanadd = 0;
                float currentjuladd = 0;

                for (int j = 0; j < edge; j++)
                {
                    float dist = 0.0f;

                    if (latitude[face][i][j] > tropicnorth - bandwidth && latitude[face][i][j] < tropicnorth + bandwidth)
                        dist = abs(latitude[face][i][j] - tropicnorth);

                    if (latitude[face][i][j] > tropicsouth - bandwidth && latitude[face][i][j] < tropicsouth + bandwidth)
                        dist = abs(latitude[face][i][j] - tropicsouth);

                    if (dist != 0.0f)
                    {
                        dist = (bandwidth - dist) / bandwidth;

                        float currentjanadd = janadd * dist;
                        float currentjuladd = juladd * dist;

                        float currentjan = float(world.janrain(face, i, j));
                        float currentjul = float(world.julrain(face, i, j));

                        if (latitude[face][i][j] < 0.0f)
                        {
                            currentjan = currentjan + currentjan * currentjanadd;
                            currentjul = currentjul + currentjul * currentjuladd;
                        }
                        else
                        {
                            currentjul = currentjul + currentjul * currentjanadd;
                            currentjan = currentjan + currentjan * currentjuladd;
                        }

                        int newjan = (int)currentjan;
                        int newjul = (int)currentjul;

                        world.setjanrain(face, i, j, newjan);
                        world.setjulrain(face, i, j, newjul);
                    }
                }
            }
        }
    }
    */
}

// This smooths the rainfall.

void smoothrainfall(planet& world, int maxmountainheight, vector<fourglobepoints>& dirpoint)
{
    int edge = world.edge();
    int sealevel = world.sealevel();

    vector<int> smoothedjanrain(6 * edge * edge, 0);
    vector<int> smoothedjulrain(6 * edge * edge, 0);

    for (int n = 0; n < 5; n++) // Quick basic smooth, which will include smoothing from one mountain tile to the next.
    {
        for (int face = 0; face < 6; face++)
        {
            int vface = face * edge * edge;

            for (int i = 0; i < edge; i++)
            {
                int vi = vface + i * edge;

                for (int j = 0; j < edge; j++)
                {
                    int index = vi + j;

                    int crount = 0;
                    int jantotal = 0;
                    int jultotal = 0;

                    for (int k = - 1; k <= 1; k++)
                    {
                        for (int l = -1; l <= 1; l++)
                        {
                            globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                            if (lpoint.face != -1)
                            {
                                if (abs(world.map(face, i, j) - world.map(lpoint.face, lpoint.x, lpoint.y)) < 150)
                                {
                                    jantotal = jantotal + world.janrain(lpoint.face, lpoint.x, lpoint.y);
                                    jultotal = jultotal + world.julrain(lpoint.face, lpoint.x, lpoint.y);
                                    crount++;
                                }
                            }
                        }
                    }

                    jantotal = jantotal / crount;
                    jultotal = jultotal / crount;

                    smoothedjanrain[index] = jantotal;
                    smoothedjulrain[index] = jultotal;
                }
            }
        }

        for (int face = 0; face < 6; face++)
        {
            int vface = face * edge * edge;

            for (int i = 0; i < edge; i++)
            {
                int vi = vface + i * edge;

                for (int j = 0; j < edge; j++)
                {
                    int index = vi + j;

                    world.setjanrain(face, i, j, smoothedjanrain[index]);
                    world.setjulrain(face, i, j, smoothedjulrain[index]);
                }
            }
        }
    }

    for (int n = 0; n < 5; n++) // Smooth with wider scope, avoiding mountains altogether.
    {
        for (int face = 0; face < 6; face++)
        {
            int vface = face * edge * edge;

            for (int i = 0; i < edge; i++) // Normal blur.
            {
                int vi = vface + i * edge;

                for (int j = 0; j < edge; j++)
                {
                    if (world.sea(face, i, j) == 0)
                    {
                        if (world.mountainheight(face, i, j) < maxmountainheight)
                        {
                            int index = vi + j;

                            globepoint grid[5][5]; // This grid will contain the coordinates of the surrounding points.

                            grid[2][2].face = face;
                            grid[2][2].x = i;
                            grid[2][2].y = j;

                            grid[2][1] = dirpoint[index].north;
                            grid[3][2] = dirpoint[index].east;
                            grid[2][3] = dirpoint[index].south;
                            grid[1][2] = dirpoint[index].west;

                            int northindex = grid[2][1].face * edge * edge + grid[2][1].x * edge + grid[2][1].y;
                            int southindex = grid[2][3].face * edge * edge + grid[2][3].x * edge + grid[2][3].y;
                            int eastindex = grid[3][2].face * edge * edge + grid[3][2].x * edge + grid[3][2].y;
                            int westindex = grid[1][2].face * edge * edge + grid[1][2].x * edge + grid[1][2].y;

                            grid[1][1] = dirpoint[northindex].west;
                            grid[3][1] = dirpoint[northindex].east;
                            grid[1][3] = dirpoint[southindex].west;
                            grid[3][3] = dirpoint[southindex].east;

                            int northwestindex = grid[1][1].face * edge * edge + grid[1][1].x * edge + grid[1][1].y;
                            int northeastindex = grid[3][1].face * edge * edge + grid[3][1].x * edge + grid[3][1].y;
                            int southwestindex = grid[1][3].face * edge * edge + grid[1][3].x * edge + grid[1][3].y;
                            int southeastindex = grid[3][3].face * edge * edge + grid[3][3].x * edge + grid[3][3].y;

                            grid[1][0] = dirpoint[northwestindex].north;
                            grid[2][0] = dirpoint[northindex].north;
                            grid[3][0] = dirpoint[northeastindex].north;

                            grid[1][4] = dirpoint[southwestindex].south;
                            grid[2][4] = dirpoint[southindex].south;
                            grid[3][4] = dirpoint[southeastindex].south;

                            grid[0][1] = dirpoint[northwestindex].west;
                            grid[0][2] = dirpoint[westindex].west;
                            grid[0][3] = dirpoint[southwestindex].west;

                            grid[4][1] = dirpoint[northeastindex].east;
                            grid[4][2] = dirpoint[eastindex].east;
                            grid[4][3] = dirpoint[southeastindex].east;

                            grid[0][0] = dirpoint[grid[1][0].face * edge * edge + grid[1][0].x * edge + grid[1][0].y].west;
                            grid[4][0] = dirpoint[grid[3][0].face * edge * edge + grid[3][0].x * edge + grid[3][0].y].east;
                            grid[4][4] = dirpoint[grid[3][4].face * edge * edge + grid[3][4].x * edge + grid[3][4].y].east;
                            grid[0][4] = dirpoint[grid[1][4].face * edge * edge + grid[1][4].x * edge + grid[1][4].y].west;

                            float crount = 0;
                            float jantotal = 0;
                            float jultotal = 0;

                            for (int k = 1; k <= 3; k++) // First, the neighbouring tiles.
                            {
                                for (int l = 1; l <= 3; l++)
                                {
                                    if (world.sea(grid[k][l].face, grid[k][l].x, grid[k][l].y) == 0 && world.mountainheight(grid[k][l].face, grid[k][l].x, grid[k][l].y) < maxmountainheight)
                                    {
                                        jantotal = jantotal + world.janrain(grid[k][l].face, grid[k][l].x, grid[k][l].y);
                                        jultotal = jultotal + world.julrain(grid[k][l].face, grid[k][l].x, grid[k][l].y);
                                        crount++;
                                    }
                                }
                            }

                            // Now, check the cells bordering those ones.

                            // Cells to the north

                            if (world.sea(grid[1][0].face, grid[1][0].x, grid[1][0].y) == 0 && world.mountainheight(grid[1][0].face, grid[1][0].x, grid[1][0].y) < maxmountainheight && (world.mountainheight(grid[1][1].face, grid[1][1].x, grid[1][1].y) < maxmountainheight || world.mountainheight(grid[2][1].face, grid[2][1].x, grid[2][1].y) < maxmountainheight))
                            {
                                jantotal = jantotal + world.janrain(grid[1][0].face, grid[1][0].x, grid[1][0].y);
                                jultotal = jultotal + world.julrain(grid[1][0].face, grid[1][0].x, grid[1][0].y);
                                crount++;
                            }

                            if (world.sea(grid[2][0].face, grid[2][0].x, grid[2][0].y) == 0 && world.mountainheight(grid[2][0].face, grid[2][0].x, grid[2][0].y) < maxmountainheight && (world.mountainheight(grid[1][1].face, grid[1][1].x, grid[1][1].y) < maxmountainheight || world.mountainheight(grid[2][1].face, grid[2][1].x, grid[2][1].y) < maxmountainheight || world.mountainheight(grid[3][1].face, grid[3][1].x, grid[3][1].y) < maxmountainheight))
                            {
                                jantotal = jantotal + world.janrain(grid[2][0].face, grid[2][0].x, grid[2][0].y);
                                jultotal = jultotal + world.julrain(grid[2][0].face, grid[2][0].x, grid[2][0].y);
                                crount++;
                            }

                            if (world.sea(grid[3][0].face, grid[3][0].x, grid[3][0].y) == 0 && world.mountainheight(grid[3][0].face, grid[3][0].x, grid[3][0].y) < maxmountainheight && (world.mountainheight(grid[2][1].face, grid[2][1].x, grid[2][1].y) < maxmountainheight || world.mountainheight(grid[3][1].face, grid[3][1].x, grid[3][1].y) < maxmountainheight))
                            {
                                jantotal = jantotal + world.janrain(grid[2][0].face, grid[2][0].x, grid[2][0].y);
                                jultotal = jultotal + world.julrain(grid[2][0].face, grid[2][0].x, grid[2][0].y);
                                crount++;
                            }

                            // Cells to the south

                            if (world.sea(grid[1][4].face, grid[1][4].x, grid[1][4].y) == 0 && world.mountainheight(grid[1][4].face, grid[1][4].x, grid[1][4].y) < maxmountainheight && (world.mountainheight(grid[1][3].face, grid[1][3].x, grid[1][3].y) < maxmountainheight || world.mountainheight(grid[2][4].face, grid[2][4].x, grid[2][4].y) < maxmountainheight))
                            {
                                jantotal = jantotal + world.janrain(grid[1][4].face, grid[1][4].x, grid[1][4].y);
                                jultotal = jultotal + world.julrain(grid[1][4].face, grid[1][4].x, grid[1][4].y);
                                crount++;
                            }

                            if (world.sea(grid[2][4].face, grid[2][4].x, grid[2][4].y) == 0 && world.mountainheight(grid[2][4].face, grid[2][4].x, grid[2][4].y) < maxmountainheight && (world.mountainheight(grid[1][3].face, grid[1][3].x, grid[1][3].y) < maxmountainheight || world.mountainheight(grid[2][3].face, grid[2][3].x, grid[2][3].y) < maxmountainheight || world.mountainheight(grid[3][3].face, grid[3][3].x, grid[3][3].y) < maxmountainheight))
                            {
                                jantotal = jantotal + world.janrain(grid[2][4].face, grid[2][4].x, grid[2][4].y);
                                jultotal = jultotal + world.julrain(grid[2][4].face, grid[2][4].x, grid[2][4].y);
                                crount++;
                            }

                            if (world.sea(grid[3][4].face, grid[3][4].x, grid[3][4].y) == 0 && world.mountainheight(grid[3][4].face, grid[3][4].x, grid[3][4].y) < maxmountainheight && (world.mountainheight(grid[2][3].face, grid[2][3].x, grid[2][3].y) < maxmountainheight || world.mountainheight(grid[3][3].face, grid[3][3].x, grid[3][3].y) < maxmountainheight))
                            {
                                jantotal = jantotal + world.janrain(grid[3][4].face, grid[3][4].x, grid[3][4].y);
                                jultotal = jultotal + world.julrain(grid[3][4].face, grid[3][4].x, grid[3][4].y);
                                crount++;
                            }

                            // Cells to the west

                            if (world.sea(grid[0][1].face, grid[0][1].x, grid[0][1].y) == 0 && world.mountainheight(grid[0][1].face, grid[0][1].x, grid[0][1].y) < maxmountainheight && (world.mountainheight(grid[1][1].face, grid[1][1].x, grid[1][1].y) < maxmountainheight || world.mountainheight(grid[1][0].face, grid[1][0].x, grid[1][0].y) < maxmountainheight))
                            {
                                jantotal = jantotal + world.janrain(grid[0][1].face, grid[0][1].x, grid[0][1].y);
                                jultotal = jultotal + world.julrain(grid[0][1].face, grid[0][1].x, grid[0][1].y);
                                crount++;
                            }

                            if (world.sea(grid[0][2].face, grid[0][2].x, grid[0][2].y) == 0 && world.mountainheight(grid[0][2].face, grid[0][2].x, grid[0][2].y) < maxmountainheight && (world.mountainheight(grid[1][1].face, grid[1][1].x, grid[1][1].y) < maxmountainheight || world.mountainheight(grid[1][0].face, grid[1][0].x, grid[1][0].y) < maxmountainheight || world.mountainheight(grid[1][3].face, grid[1][3].x, grid[1][3].y) < maxmountainheight))
                            {
                                jantotal = jantotal + world.janrain(grid[0][2].face, grid[0][2].x, grid[0][2].y);
                                jultotal = jultotal + world.julrain(grid[0][2].face, grid[0][2].x, grid[0][2].y);
                                crount++;
                            }

                            if (world.sea(grid[0][3].face, grid[0][3].x, grid[0][3].y) == 0 && world.mountainheight(grid[0][3].face, grid[0][3].x, grid[0][3].y) < maxmountainheight && (world.mountainheight(grid[1][0].face, grid[1][0].x, grid[1][0].y) < maxmountainheight || world.mountainheight(grid[1][3].face, grid[1][3].x, grid[1][3].y) < maxmountainheight))
                            {
                                jantotal = jantotal + world.janrain(grid[0][3].face, grid[0][3].x, grid[0][3].y);
                                jultotal = jultotal + world.julrain(grid[0][3].face, grid[0][3].x, grid[0][3].y);
                                crount++;
                            }

                            // Cells to the east

                            if (world.sea(grid[4][1].face, grid[4][1].x, grid[4][1].y) == 0 && world.mountainheight(grid[4][1].face, grid[4][1].x, grid[4][1].y) < maxmountainheight && (world.mountainheight(grid[3][1].face, grid[3][1].x, grid[3][1].y) < maxmountainheight || world.mountainheight(grid[3][2].face, grid[3][2].x, grid[3][2].y) < maxmountainheight))
                            {
                                jantotal = jantotal + world.janrain(grid[4][1].face, grid[4][1].x, grid[4][1].y);
                                jultotal = jultotal + world.julrain(grid[4][1].face, grid[4][1].x, grid[4][1].y);
                                crount++;
                            }

                            if (world.sea(grid[4][2].face, grid[4][2].x, grid[4][2].y) == 0 && world.mountainheight(grid[4][2].face, grid[4][2].x, grid[4][2].y) < maxmountainheight && (world.mountainheight(grid[3][1].face, grid[3][1].x, grid[3][1].y) < maxmountainheight || world.mountainheight(grid[3][2].face, grid[3][2].x, grid[3][2].y) < maxmountainheight || world.mountainheight(grid[3][3].face, grid[3][3].x, grid[3][3].y) < maxmountainheight))
                            {
                                jantotal = jantotal + world.janrain(grid[4][2].face, grid[4][2].x, grid[4][2].y);
                                jultotal = jultotal + world.julrain(grid[4][2].face, grid[4][2].x, grid[4][2].y);
                                crount++;
                            }

                            if (world.sea(grid[4][3].face, grid[4][3].x, grid[4][3].y) == 0 && world.mountainheight(grid[4][3].face, grid[4][3].x, grid[4][3].y) < maxmountainheight && (world.mountainheight(grid[3][2].face, grid[3][2].x, grid[3][2].y) < maxmountainheight || world.mountainheight(grid[3][3].face, grid[3][3].x, grid[3][3].y) < maxmountainheight))
                            {
                                jantotal = jantotal + world.janrain(grid[4][3].face, grid[4][3].x, grid[4][3].y);
                                jultotal = jultotal + world.julrain(grid[4][3].face, grid[4][3].x, grid[4][3].y);
                                crount++;
                            }


                            if (crount > 0)
                            {
                                jantotal = jantotal / crount;
                                jultotal = jultotal / crount;

                                world.setjanrain(face, i, j, (int)jantotal);
                                world.setjulrain(face, i, j, (int)jultotal);
                            }
                        }
                    }
                }
            }
        }
    }
}

// This caps excessive rainfall.

void caprainfall(planet& world)
{
    int edge = world.edge();
    float tilt = world.tilt();
    float eccentricity = world.eccentricity();

    float maxrain = 1000.0f; // Any rainfall over this will be greatly reduced.
    float capfactor = 0.1f; // Amount to multiply excessive rain by.

    float rain[2];

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                rain[0] = (float)world.janrain(face, i, j);
                rain[1] = (float)world.julrain(face, i, j);

                for (int n = 0; n <= 1; n++)
                {
                    if (rain[n] > maxrain)
                    {
                        rain[n] = rain[n] - maxrain;
                        rain[n] = rain[n] * capfactor;
                        rain[n] = rain[n] + maxrain;
                    }

                    if (rain[n] < 0.0f)
                        rain[n] = 0.0f;
                }

                world.setjanrain(face, i, j, (int)rain[0]);
                world.setjulrain(face, i, j, (int)rain[1]);
            }
        }
    }

    if (tilt < 10.f && eccentricity < 0.1f) // For worlds with low obliquity and low eccentricity, reduce any seasonal difference in rainfall.
    {
        float adjustfactor = tilt;

        float reducefactor = 0.5f + tilt / 20.0f;

        for (int face = 0; face < 6; face++)
        {
            for (int i = 0; i < edge; i++)
            {
                for (int j = 0; j < edge; j++)
                {
                    float averain = (float)((world.janrain(face, i, j) + world.julrain(face, i, j)) / 2);

                    if (averain > 0.0f)
                    {
                        float thisjanrain = (float)world.janrain(face, i, j) * adjustfactor + averain * (10.0f - adjustfactor);
                        float thisjulrain = (float)world.julrain(face, i, j) * adjustfactor + averain * (10.0f - adjustfactor);

                        thisjanrain = thisjanrain * reducefactor;
                        thisjulrain = thisjulrain * reducefactor;

                        world.setjanrain(face, i, j, (int)thisjanrain);
                        world.setjulrain(face, i, j, (int)thisjulrain);
                    }
                }
            }
        }
    }
}

// This adjusts temperatures in the light of rainfall.

void adjusttemperatures(planet& world, vector<int>& inland)
{
    int edge = world.edge();
    int sealevel = world.sealevel();
    float tilt = world.tilt();
    float averagetemp = (float)world.averagetemp();

    float changestrength = abs(tilt - 22.5f); // The closer the tilt is to 22.5, the more effect there is.
    changestrength = changestrength / 10.0f;
    changestrength = 1.0f - changestrength;

    if (changestrength > 1.0f)
        changestrength = 1.0f;

    float changestrength2 = abs(averagetemp - 14.0f);
    changestrength2 = changestrength2 / 20.0f;
    changestrength2 = 1.0f - changestrength2;

    if (changestrength2 > 1.0f)
        changestrength2 = 1.0f;

    if (changestrength > 0.0f && changestrength2 > 0.0f)
    {
        float winterrainwarmth = 0.08f; // Factor to increase temperature in winter because of rain.
        float maxwinterrainwarmth = 10.0f; //6; // Most that can be added.
        float summerraincold = 0.0025f;
        float norainheat = 1.3f; // Amount that the summer temperature increases where there's no rain at all.
        float noraincold = 1.2f; //0.8f; // Amount that the winter temperature decreases where there's no rain at all.
        float maxwintervar = 15.0f; // Maximum amount that rainfall can increase temperature in winter.
        float maxsummervar = 10.0f; // Maximum amount that rainfall can decrease temperature in summer.

        float maxvar = 40.0f; // Temperatures higher than this won't be affected.
        float minvar = -20.0f; // Temperatures lower than this won't be affected.

        float midvar = (maxvar + minvar) / 2.0f; // Most effect at this temperature.
        float varstep = 1.0f / (maxvar - midvar);

        float changemult = changestrength * changestrength2 * 100.00f; // This is anything up to 100.
        float remainmult = 100.00f - changemult;

        for (int face = 0; face < 6; face++)
        {
            int vface = face * edge * edge;

            for (int i = 0; i < edge; i++)
            {
                int vi = vface + i * edge;

                for (int j = 0; j < edge; j++)
                {
                    int index = vi + j;

                    if (world.nom(face, i, j) > sealevel)
                    {
                        float mintemp = (float)world.mintemp(face, i, j);
                        float maxtemp = (float)world.maxtemp(face, i, j);

                        float minchangemult = abs(mintemp - midvar) * varstep; // Multiply the final adjustment by these so the effect tails off with colder or hotter temperatures.
                        float maxchangemult = abs(maxtemp - midvar) * varstep;

                        float minremainmult = 1.0f - minchangemult;
                        float maxremainmult = 1.0f - maxchangemult;

                        float winterrain = (float)world.winterrain(face, i, j);
                        float summerrain = (float)world.summerrain(face, i, j);

                        float wintervar = winterrain * winterrainwarmth;
                        float summervar = summerrain * summerraincold;

                        float continental = (float)inland[index]; // Further inland, there is less warming effect from winter rain.

                        continental = continental - 20.0f; // Offset the effect to ensure we do get temperate oceanic regions

                        if (continental > 0.0f)
                        {
                            continental = 5.0f / continental;

                            if (continental > 1.0f)
                                continental = 1.0f;

                            wintervar = wintervar * continental;
                        }

                        if (wintervar > maxwintervar)
                            wintervar = maxwintervar;

                        if (summervar > maxsummervar)
                            summervar = maxsummervar;

                        float newmintemp = mintemp + wintervar;
                        float newmaxtemp = maxtemp - summervar;

                        if (world.summerrain(face, i, j) == 0 && maxtemp > 0.0f) // If there's no rain at all, temperatures are more extreme.
                            newmaxtemp = maxtemp * norainheat;

                        if (world.winterrain(face, i, j) == 0)
                            newmintemp = mintemp * noraincold;

                        if (newmintemp - (float)world.mintemp(face, i, j) > maxwinterrainwarmth)
                            newmintemp = (float)world.mintemp(face, i, j) + maxwinterrainwarmth;

                        newmintemp = (newmintemp * changemult + mintemp * remainmult) / 100.0f; // Here we modify the change according to axial tilt (i.e. this is the same modification for everywhere).
                        newmaxtemp = (newmaxtemp * changemult + maxtemp * remainmult) / 100.0f;

                        newmintemp = newmintemp * minchangemult + mintemp * minremainmult; // Here we modify the change according to how hot this point is (i.e. this modification will be different for each point).
                        newmaxtemp = newmaxtemp * maxchangemult + maxtemp * maxremainmult;

                        float tempdiff = maxtemp - mintemp;
                        float tempdiffchangemult = tempdiff / 10.0f;

                        if (tempdiffchangemult > 1.0f)
                            tempdiffchangemult = 1.0f;

                        if (tempdiffchangemult < 1.0f)
                        {
                            float tempdiffremainmult = 1.0f - tempdiffchangemult;

                            newmintemp = newmintemp * tempdiffchangemult + mintemp * tempdiffremainmult; // Here we modify the change according to how much seasonality there is here (i.e. this modification will be different for each point).
                            newmaxtemp = newmaxtemp * tempdiffchangemult + maxtemp * tempdiffremainmult;
                        }

                        world.setmintemp(face, i, j, (int)newmintemp);
                        world.setmaxtemp(face, i, j, (int)newmaxtemp);

                        if (world.maxtemp(face, i, j) < world.mintemp(face, i, j))
                            world.setmaxtemp(face, i, j, world.mintemp(face, i, j));
                    }
                }
            }
        }
    }
}

// This makes temperatures a bit more extreme further from the sea.

void adjustcontinentaltemperatures(planet& world, vector<int>& inland)
{
    float tilt = world.tilt();

    if (tilt == 0.0)
        return;

    int edge = world.edge();
    int sealevel = world.sealevel();
    int averagetemp = world.averagetemp();

    float maxvar = 30.0f; // Temperatures higher than this won't be affected.
    float minvar = -10.0f; // Temperatures lower than this won't be affected.

    float midvar = (maxvar + minvar) / 2.0f; // Most effect at this temperature.
    float varstep = 1.0f / (maxvar - midvar);

    float winterremovefactor = 0.6f;
    float summeraddfactor = 0.05f;

    float contstrength = tilt / 22.5f; // The higher the axial tilt, the more effect this will have.

    if (contstrength > 1.0f)
        contstrength = 1.0f;

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                if (world.sea(face, i, j) == 0)
                {
                    float thisstrength = (float)inland[vi + j]; // The further inland, the higher the strength of the effect.

                    thisstrength = thisstrength / 100.0f;

                    if (thisstrength > 1.5f)
                        thisstrength = 1.5f;

                    thisstrength = thisstrength * contstrength; // The more global tilt there is, the higher the strength of the effect.

                    float maxtemp = (float)world.maxtemp(face, i, j);
                    float mintemp = (float)world.mintemp(face, i, j);

                    float tempdiff = maxtemp - mintemp; // Take the existing difference between summer and winter.

                    float mintempremove = tempdiff * winterremovefactor * thisstrength; // Multiply it by the factors to get the amount to lower in winter and raise in summer.
                    float maxtempadd = tempdiff * summeraddfactor * thisstrength;

                    if (mintempremove > 20)
                        mintempremove = 20;

                    if (maxtempadd > 10)
                        maxtempadd = 10;

                    float newmaxtemp = maxtemp + maxtempadd;
                    float newmintemp = mintemp - mintempremove;

                    world.setmintemp(face, i, j, (int)newmintemp);
                    world.setmaxtemp(face, i, j, (int)newmaxtemp);
                }
            }
        }
    }

    // Now just a slight tweak to make tundra areas a little warmer, to encourage more subpolar.

    if (world.northpolartemp() > world.eqtemp() || world.southpolartemp() > world.eqtemp()) // Don't do this if the poles are hotter than the equator
        return;

    float tweakstrength = (tilt - 22.5f) / 30.0f; // This is to lessen the effect when the tilt is higher.
    tweakstrength = 1.0f - tweakstrength;

    if (tweakstrength <= 0.0)
        return;

    if (tweakstrength > 1.0)
        tweakstrength = 1.0f;

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.sea(face, i, j) == 0)
                {
                    int maxtemp = world.maxtemp(face, i, j);
                    int mintemp = world.mintemp(face, i, j);

                    if (maxtemp <= 12)
                    {
                        float factor = 10.0f; // 3

                        float elev = (float)world.map(face, i, j) - (float)sealevel;

                        factor = factor - elev / 400.0f; // Reduce this effect at higher elevations.

                        factor = factor * contstrength; // Reduce this for lower axial tilt.

                        factor = factor * tweakstrength; // Reduce it for higher axial tilt, too.

                        if (factor > 0.0)
                        {
                            int newmaxtemp = maxtemp + (int)factor;

                            if (newmaxtemp > 13)
                                newmaxtemp = 13;

                            int newmintemp = mintemp + (int)factor;

                            world.setmaxtemp(face, i, j, newmaxtemp);
                            world.setmintemp(face, i, j, newmintemp);
                        }
                    }
                }
            }
        }
    }
}

// This smooths the temperatures in light of rainfall.

void smoothtemperatures(planet& world, vector<fourglobepoints>& dirpoint)
{
    int edge = world.edge();

    vector<int> jantemp(6 * edge * edge, 0);
    vector<int> jultemp(6 * edge * edge, 0);

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                if (world.sea(face, i, j) == 0)
                {
                    int index = vi + j;

                    jantemp[index] = tempelevremove(world, world.jantemp(face, i, j), face, i, j);
                    jultemp[index] = tempelevremove(world, world.jultemp(face, i, j), face, i, j);
                }
            }
        }
    }

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                if (world.sea(face, i, j) == 0)
                {
                    int index = vi + j;

                    globepoint northpoint = dirpoint[index].north;
                    globepoint southpoint = dirpoint[index].south;
                    
                    if (world.sea(northpoint.face,northpoint.x,northpoint.y) == 0 && world.sea(southpoint.face,southpoint.x,southpoint.y) == 0)
                    {
                        int nindex = northpoint.face * edge * edge + northpoint.x * edge + northpoint.y;
                        int sindex = southpoint.face * edge * edge + southpoint.x * edge + southpoint.y;

                        if (jantemp[index] < jantemp[nindex] && jantemp[index] < jantemp[sindex])
                        {
                            jantemp[index] = (jantemp[nindex] + jantemp[sindex]) / 2;
                        }

                        if (jultemp[index] < jultemp[nindex] && jultemp[index] < jultemp[sindex])
                        {
                            jultemp[index] = (jultemp[nindex] + jultemp[sindex]) / 2;
                        }
                    }
                }
            }
        }
    }

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                if (world.sea(face, i, j) == 0)
                {
                    int index = vi + j;

                    world.setjantemp(face, i, j, tempelevadd(world, jantemp[index], face, i, j));
                    world.setjultemp(face, i, j, tempelevadd(world, jultemp[index], face, i, j));
                }
            }
        }
    }
}

// This function removes weird lines of subpolar climate.

void removesubpolarstreaks(planet& world, vector<fourglobepoints>& dirpoint)
{
    int edge = world.edge();

    vector<int> origmaxtemp(6 * edge * edge, 0);
    vector<int> origmintemp(6 * edge * edge, 0);

    for (int times = 1; times <= 2; times++)
    {
        for (int n = 1; n <= 4; n++)
        {
            for (int face = 0; face < 6; face++)
            {
                int vface = face * edge * edge;

                for (int i = 0; i < edge; i++)
                {
                    int vi = vface + i * edge;

                    for (int j = 0; j < edge; j++)
                    {
                        int index = vi + j;

                        origmaxtemp[index] = world.maxtemp(face, i, j);
                        origmintemp[index] = world.mintemp(face, i, j);
                    }
                }
            }

            for (int face = 0; face < 6; face++)
            {
                int vface = face * edge * edge;

                for (int i = 0; i < edge; i++)
                {
                    int vi = vface + i * edge;

                    for (int j = 0; j < edge; j++)
                    {
                        if (world.sea(face, i, j) == 0)
                        {
                            int climate = calculateclimate(world.map(face, i, j), world.sealevel(), (float)world.winterrain(face, i, j), (float)world.summerrain(face, i, j), (float)world.mintemp(face, i, j), (float)world.maxtemp(face, i, j));

                            if (world.mountainheight(face, i, j) < 200 && ((climate > 4 && climate < 9) || (climate > 17 && climate < 30))) //  climate=="D" || climate=="BW" || climate=="BS"))
                            {
                                int index = vi + j;

                                globepoint northpoint = dirpoint[index].north;
                                globepoint southpoint = dirpoint[index].south;

                                if (world.mintemp(northpoint.face, northpoint.x, northpoint.y) <= -3 && world.maxtemp(northpoint.face, northpoint.x, northpoint.y) > 10 && world.mintemp(southpoint.face, southpoint.x, southpoint.y) <= -3 && world.maxtemp(southpoint.face, southpoint.x, southpoint.y) > 10)
                                {
                                    if (world.mountainheight(northpoint.face, northpoint.x, northpoint.y) < 200 && world.mountainheight(southpoint.face, southpoint.x, southpoint.y) < 200)
                                    {
                                        int nindex = northpoint.face * edge * edge + northpoint.x * edge + northpoint.y;
                                        int sindex = southpoint.face * edge * edge + southpoint.x * edge + southpoint.y;

                                        world.setmintemp(face, i, j, (origmintemp[index] + origmintemp[nindex] + origmintemp[sindex]) / 3);
                                        world.setmaxtemp(face, i, j, (origmaxtemp[index] + origmaxtemp[nindex] + origmaxtemp[sindex]) / 3);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                origmaxtemp[index] = world.maxtemp(face, i, j);
                origmintemp[index] = world.mintemp(face, i, j);
            }
        }
    }

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                if (world.sea(face, i, j) == 0)
                {
                    int climate = calculateclimate(world.map(face, i, j), world.sealevel(), (float)world.winterrain(face, i, j), (float)world.summerrain(face, i, j), (float)world.mintemp(face, i, j), (float)world.maxtemp(face, i, j));

                    if (world.mountainheight(face, i, j) < 200 && ((climate > 4 && climate < 9) || (climate > 17 && climate < 30))) // (climate=="D" || climate=="BW" || climate=="BS"))
                    {
                        int index = vi + j;

                        globepoint westpoint = dirpoint[index].west;
                        globepoint eastpoint = dirpoint[index].east;

                        if (world.mintemp(westpoint.face, westpoint.x, westpoint.y) <= -3 && world.maxtemp(westpoint.face, westpoint.x, westpoint.y) > 10 && world.mintemp(eastpoint.face, eastpoint.x, eastpoint.y) <= -3 && world.maxtemp(eastpoint.face, eastpoint.x, eastpoint.y) > 10)
                        {
                            if (world.mountainheight(westpoint.face, westpoint.x, westpoint.y) < 200)
                            {
                                int windex = westpoint.face * edge * edge + westpoint.face * edge + westpoint.face + j;
                                int eindex = eastpoint.face * edge * edge + eastpoint.face * edge + eastpoint.face + j;

                                world.setmintemp(face, i, j, (origmintemp[index] + origmintemp[windex] + origmintemp[eindex]) / 3);
                                world.setmaxtemp(face, i, j, (origmaxtemp[index] + origmaxtemp[windex] + origmaxtemp[eindex]) / 3);
                            }
                        }
                    }
                }
            }
        }
    }
}

// This removes odd lines of temperature.

void correcttemperatures(planet& world)
{
    int edge = world.edge();
    int eedge = edge * edge;

    // First, remove odd vertical streaks of higher temperatures.

    int margin = 3; // Any difference greater than this will be dealt with.
    int elevmargin = 300; // Ignore cases where there's more elevation difference than this.

    for (int face = 0; face < 4; face++)
    {
        int vface = face * eedge;

        for (int i = 1; i < edge - 1; i++)
        {
            int vi = vface + i * edge;
            int vleft = vface + (i - 1) * edge;
            int vright = vface + (i + 1) * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;
                int leftindex = vleft + j;
                int rightindex = vright + j;

                int jantemp = world.jantemp(face, i, j);
                int jultemp = world.jultemp(face, i, j);

                int leftjantemp = world.jantemp(face, i - 1, j);
                int leftjultemp = world.jultemp(face, i - 1, j);

                int rightjantemp = world.jantemp(face, i + 1, j);
                int rightjultemp = world.jultemp(face, i + 1, j);

                if ((jantemp > leftjantemp + margin || jultemp > leftjultemp + margin) || (jantemp > rightjantemp + margin || jultemp > rightjultemp + margin))
                {
                    int elev = world.map(face, i, j);
                    int leftelev = world.map(face, i - 1, j);
                    int rightelev = world.map(face, i + 1, j);

                    if (leftelev < elev + elevmargin || rightelev < elev + elevmargin) // Definitely an anomaly.
                    {
                        world.setjantemp(face, i, j, (leftjantemp + rightjantemp) / 2);
                        world.setjultemp(face, i, j, (leftjultemp + rightjultemp) / 2);
                    }
                }
            }
        }
    }

    // Now, remove odd lines of higher temperatures at the edge of face 4.

    for (int j = 0; j < edge; j++)
    {
        int jantemp = world.jantemp(4, 0, j);
        int jultemp = world.jultemp(4, 0, j);

        int northjantemp = world.jantemp(4, 1, j);
        int northjultemp = world.jultemp(4, 1, j);

        int southjantemp = world.jantemp(3, j, 0);
        int southjultemp = world.jultemp(3, j, 0);

        int newjantemp = (northjantemp + southjantemp) / 2;
        int newjultemp = (northjultemp + southjultemp) / 2;

        world.setjantemp(4, 0, j, newjantemp);
        world.setjultemp(4, 0, j, newjultemp);
    }
}

// This creates mountain precipitation arrays (which are used at the regional level).

void createmountainprecipitation(planet& world)
{
    int edge = world.edge();

    float totalmountaineffect = 1000; // Mountains higher than this will get the full effect.

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.mountainheight(face, i, j) == 0 && world.craterrim(face, i, j) == 0)
                {
                    world.setwintermountainraindir(face, i, j, 0);
                    world.setsummermountainraindir(face, i, j, 0);
                }
            }
        }
    }

    for (int n = 0; n < 1; n++)
    {
        // Using average neighbouring rainfall

        for (int face = 0; face < 6; face++)
        {
            for (int i = 0; i < edge; i++) // Winter precipitation
            {
                for (int j = 0; j < edge; j++)
                {
                    if ((world.mountainheight(face, i, j) != 0 || world.craterrim(face, i, j) != 0) && world.wintermountainraindir(face, i, j) == 0)
                    {
                        // First, find the cell that the wind's coming from. It's the non-mountain cell with the highest precipitation.

                        globepoint windfromcell;
                        windfromcell.face = -1;

                        int highestamount = -1;

                        for (int k = - 1; k <= 1; k++)
                        {
                            for (int l = -1; l <= 1; l++)
                            {
                                globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                                if (lpoint.face != -1)
                                {
                                    if (world.sea(lpoint.face, lpoint.x, lpoint.y) == 0 && world.mountainheight(lpoint.face, lpoint.x, lpoint.y) == 0 && world.craterrim(lpoint.face, lpoint.x, lpoint.y) == 0)
                                    {
                                        if (world.winterrain(lpoint.face, lpoint.x, lpoint.y) > highestamount)
                                        {
                                            highestamount = world.winterrain(lpoint.face, lpoint.x, lpoint.y);
                                            windfromcell = lpoint;
                                        }
                                    }
                                }
                            }
                        }

                        if (windfromcell.face == -1) // We didn't find one!
                        {
                            for (int k = - 1; k <= 1; k++)
                            {
                                for (int l = -1; l <= 1; l++)
                                {
                                    globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                                    if (lpoint.face != -1)
                                    {
                                        if (world.wintermountainraindir(lpoint.face, lpoint.x, lpoint.y) != 0)
                                        {
                                            if (world.winterrain(lpoint.face, lpoint.x, lpoint.y) > highestamount)
                                            {
                                                highestamount = world.winterrain(lpoint.face, lpoint.x, lpoint.y);
                                                windfromcell = lpoint;
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        // Now find the average precipitation of neighbouring cells.

                        short crount = 0;
                        int total = 0;

                        for (int k = -1; k <= 1; k++)
                        {
                            for (int l = -1; l <= 1; l++)
                            {
                                globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                                if (lpoint.face != -1)
                                {
                                    if (world.sea(lpoint.face, lpoint.x, lpoint.y) == 0 && world.mountainheight(lpoint.face, lpoint.x, lpoint.y) == 0 && world.craterrim(lpoint.face, lpoint.x, lpoint.y) == 0)
                                    {
                                        crount++;
                                        total = total + world.winterrain(lpoint.face, lpoint.x, lpoint.y);
                                    }
                                }
                            }
                        }

                        if (crount == 0) // We didn't find one!
                        {
                            for (int k = -1; k <= 1; k++)
                            {
                                for (int l = -1; l <= 1; l++)
                                {
                                    globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                                    if (lpoint.face != -1)
                                    {
                                        if (world.wintermountainraindir(lpoint.face, lpoint.x, lpoint.y) != 0)
                                        {
                                            crount++;
                                            total = total + world.winterrain(lpoint.face, lpoint.x, lpoint.y);
                                        }
                                    }
                                }
                            }
                        }

                        if (windfromcell.face != -1 && crount != 0) // If we found one!
                        {
                            float thisheight = (float)world.mountainheight(face, i, j);
                            float craterheight = (float)world.craterrim(face, i, j);

                            if (craterheight > thisheight)
                                thisheight = craterheight;

                            float effect = 1.0f;

                            if (thisheight < totalmountaineffect) // Reduce the strength of this.
                                effect = thisheight / totalmountaineffect;

                            float rainamount = (float)total / (float)crount;
                            float rainamount2 = (float)world.winterrain(face, i, j);

                            float newrainamount = (rainamount * effect) + (rainamount2 * (1.0f - effect));

                            world.setwintermountainrain(face, i, j, (int)newrainamount);

                            int dir = getdir(edge, windfromcell.face, windfromcell.x, windfromcell.y, face, i, j);
                            world.setwintermountainraindir(face, i, j, dir);
                        }
                    }
                }
            }
        }

        for (int face = 0; face < 6; face++)
        {
            for (int i = 0; i < edge; i++) // summer precipitation
            {
                for (int j = 0; j < edge; j++)
                {
                    if ((world.mountainheight(face, i, j) != 0 || world.craterrim(face, i, j) != 0) && world.summermountainraindir(face, i, j) == 0)
                    {
                        // First, find the cell that the wind's coming from. It's the non-mountain cell with the highest precipitation.

                        globepoint windfromcell;
                        windfromcell.face = -1;

                        int highestamount = -1;

                        for (int k = -1; k <= 1; k++)
                        {
                            for (int l = -1; l <= 1; l++)
                            {
                                globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                                if (lpoint.face != -1)
                                {
                                    if (world.sea(lpoint.face, lpoint.x, lpoint.y) == 0 && world.mountainheight(lpoint.face, lpoint.x, lpoint.y) == 0 && world.craterrim(lpoint.face, lpoint.x, lpoint.y) == 0)
                                    {
                                        if (world.summerrain(lpoint.face, lpoint.x, lpoint.y) > highestamount)
                                        {
                                            highestamount = world.summerrain(lpoint.face, lpoint.x, lpoint.y);
                                            windfromcell = lpoint;
                                        }
                                    }
                                }
                            }
                        }

                        if (windfromcell.face == -1) // We didn't find one!
                        {
                            for (int k = -1; k <= 1; k++)
                            {
                                for (int l = -1; l <= 1; l++)
                                {
                                    globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                                    if (lpoint.face != -1)
                                    {
                                        if (world.summermountainraindir(lpoint.face, lpoint.x, lpoint.y) != 0)
                                        {
                                            if (world.summerrain(lpoint.face, lpoint.x, lpoint.y) > highestamount)
                                            {
                                                highestamount = world.summerrain(lpoint.face, lpoint.x, lpoint.y);
                                                windfromcell = lpoint;
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        // Now find the average precipitation of neighbouring cells.

                        short crount = 0;
                        int total = 0;

                        for (int k = -1; k <= 1; k++)
                        {
                            for (int l = -1; l <= 1; l++)
                            {
                                globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                                if (lpoint.face != -1)
                                {
                                    if (world.sea(lpoint.face, lpoint.x, lpoint.y) == 0 && world.mountainheight(lpoint.face, lpoint.x, lpoint.y) == 0 && world.craterrim(lpoint.face, lpoint.x, lpoint.y) == 0)
                                    {
                                        crount++;
                                        total = total + world.summerrain(lpoint.face, lpoint.x, lpoint.y);
                                    }
                                }
                            }
                        }

                        if (crount == 0) // We didn't find one!
                        {
                            for (int k = -1; k <= 1; k++)
                            {
                                for (int l = -1; l <= 1; l++)
                                {
                                    globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                                    if (lpoint.face != -1)
                                    {
                                        if (world.summermountainraindir(lpoint.face, lpoint.x, lpoint.y) != 0)
                                        {
                                            crount++;
                                            total = total + world.summerrain(lpoint.face, lpoint.x, lpoint.y);
                                        }
                                    }
                                }
                            }
                        }

                        if (windfromcell.face != -1 && crount != 0) // If we found one!
                        {
                            float thisheight = (float)world.mountainheight(face, i, j);
                            float craterheight = (float)world.craterrim(face, i, j);

                            if (craterheight > thisheight)
                                thisheight = craterheight;

                            float effect = 1.0f;

                            if (thisheight < totalmountaineffect) // Reduce the strength of this.
                                effect = thisheight / totalmountaineffect;

                            float rainamount = (float)total / (float)crount;
                            float rainamount2 = (float)world.summerrain(face, i, j);

                            float newrainamount = (rainamount * effect) + (rainamount2 * (1.0f - effect));

                            world.setsummermountainrain(face, i, j, (int)newrainamount);

                            int dir = getdir(edge, windfromcell.face, windfromcell.x, windfromcell.y, face, i, j);
                            world.setsummermountainraindir(face, i, j, dir);
                        }
                    }
                }
            }
        }
    }
}

// This adds a bit of rainfall to worlds with no sea.

void createdesertworldrain(planet& world)
{
    int edge = world.edge();
    int sealevel = world.sealevel();
    float maxelev = (float)world.maxelevation();

    float slopefactor = 130.0f;
    float idealtemp = 20.0f; // The closer it is to this temperature, the more rain there is.
    float maxdiff = 40.0f; // If it's more than this hotter/colder than the ideal temperature, there will be no rain.

    float maxrain = (float)(random(20, 200));

    int grain = 8; // Level of detail on this fractal map.
    float valuemod = 0.2f;
    float valuemod2 = 3.0f;

    vector<int> fractal(6 * edge * edge, 0);

    createfractal(fractal, edge, grain, valuemod, valuemod2, 1, (int)maxelev, 0, 0);

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface * edge + i;

            for (int j = 0; j < edge; j++)
            {
                int elev = world.map(face, i, j);

                int biggestdiff = 0;

                for (int k = - 1; k <= 1; k++)
                {
                    for (int l = -1; l <= 1; l++)
                    {
                        globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                        if (lpoint.face != -1)
                        {
                            int thisdiff = elev - world.map(lpoint.face, lpoint.x, lpoint.y);

                            if (thisdiff > biggestdiff)
                                biggestdiff = thisdiff;
                        }
                    }
                }

                float thisslopemult = (float)biggestdiff / slopefactor;

                float thisfractalmult = (float)fractal[vi + j] / maxelev;

                float thisrain = maxrain * thisslopemult * thisfractalmult;

                world.setjanrain(face, i, j, (int)thisrain);
                world.setjulrain(face, i, j, (int)thisrain);
            }
        }
    }

    // Now adjust for seasonal variation.

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                float oldjanrain = (float)world.janrain(face, i, j);
                float oldjulrain = (float)world.julrain(face, i, j);

                float jandiff = (float)world.jantemp(face, i, j) - idealtemp;
                float juldiff = (float)world.jultemp(face, i, j) - idealtemp;

                if (jandiff < 0.0f)
                    jandiff = 0.0f - jandiff;

                if (juldiff < 0.0f)
                    juldiff = 0.0f - juldiff;

                jandiff = maxdiff - jandiff;
                juldiff = maxdiff - juldiff;

                if (jandiff < 0.0f)
                    jandiff = 0.0f;

                if (juldiff < 0.0f)
                    juldiff = 0.0f;

                float janmult = jandiff / maxdiff;
                float julmult = juldiff / maxdiff;

                float newjanrain = oldjanrain * janmult;
                float newjulrain = oldjulrain * julmult;

                world.setjanrain(face, i, j, (int)newjanrain);
                world.setjulrain(face, i, j, (int)newjulrain);
            }
        }
    }

    // Now blur.

    int dist = 1;

    vector<int> finaljanrain(6 * edge * edge, 0);
    vector<int> finaljulrain(6 * edge * edge, 0);

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                int crount = 0;
                int jantotal = 0;
                int jultotal = 0;

                for (int k = - dist; k <= dist; k++)
                {
                    for (int l = -dist; l <= dist; l++)
                    {
                        globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                        if (lpoint.face != -1)
                        {
                            jantotal = jantotal + world.janrain(face, i, j);
                            jultotal = jultotal + world.julrain(face, i, j);

                            crount++;
                        }
                    }
                }

                float newjanrain = (float)jantotal / (float)crount;
                float newjulrain = (float)jultotal / (float)crount;

                finaljanrain[index] = (int)newjanrain;
                finaljulrain[index] = (int)newjulrain;
            }
        }
    }

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                world.setjanrain(face, i, j, finaljanrain[index]);
                world.setjulrain(face, i, j, finaljulrain[index]);
            }
        }
    }
}

// This function calculates the tides.

void createtidalmap(planet& world, vector<fourglobepoints>& dirpoint)
{
    int edge = world.edge();
    int lunar = (int)world.lunar();

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
                world.settide(face, i, j, -1);
        }
    }

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.sea(face, i, j) == 1)
                {
                    bool landfound = 0;

                    for (int k = - 1; k <= 1; k++)
                    {
                        for (int l = -1; l <= 1; l++)
                        {
                            globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                            if (lpoint.face != -1)
                            {
                                if (world.sea(lpoint.face, lpoint.x, lpoint.y) == 0)
                                {
                                    landfound = 1;
                                    k = 1;
                                    l = 1;
                                }
                            }
                        }
                    }

                    if (landfound == 1)
                    {
                        int tide = gettidalrange(world, face, i, j, dirpoint);

                        for (int k = - 1; k <= 1; k++)
                        {
                            for (int l = -1; l <= 1; l++)
                            {
                                globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                                if (lpoint.face != -1)
                                {
                                    if (world.tide(lpoint.face, lpoint.x, lpoint.y) == -1)
                                        world.settide(lpoint.face, lpoint.x, lpoint.y, tide);
                                }
                            }
                        }

                        if (tide < lunar * 2)
                            tide = lunar * 2;

                        world.settide(face, i, j, tide);
                    }
                }
            }
        }
    }
}

// This function finds the tidal range of a given point.

int gettidalrange(planet& world, int startface, int startx, int starty, vector<fourglobepoints>& dirpoint)
{
    int edge = world.edge();
    int sealevel = world.sealevel();

    float mult1 = 0.01f;

    // We cast in each of the eight directions from the starting point until we hit land.
    // The longer each cast, the more tidal range it generates.

    int north = 0;
    int northeast = 0;
    int east = 0;
    int southeast = 0;
    int south = 0;
    int southwest = 0;
    int west = 0;
    int northwest = 0;

    bool hitland;

    // North

    hitland = 0;

    globepoint thispoint;
    thispoint.face = startface;
    thispoint.x = startx;
    thispoint.y = starty;

    for (int n = 1; n <= 1000; n++)
    {
        thispoint = dirpoint[thispoint.face * edge * edge + thispoint.x * edge + thispoint.y].north;

        if (world.nom(thispoint.face,thispoint.x,thispoint.y) > sealevel)
            hitland = 1;
        else
            north++;

        if (hitland == 1)
            n = 1000;
    }

    // South

    hitland = 0;
    thispoint.face = startface;
    thispoint.x = startx;
    thispoint.y = starty;

    for (int n = 1; n <= 1000; n++)
    {
        thispoint = dirpoint[thispoint.face * edge * edge + thispoint.x * edge + thispoint.y].south;

        if (world.nom(thispoint.face, thispoint.x, thispoint.y) > sealevel)
            hitland = 1;
        else
            south++;

        if (hitland == 1)
            n = 1000;
    }

    // East

    hitland = 0;
    thispoint.face = startface;
    thispoint.x = startx;
    thispoint.y = starty;

    for (int n = 1; n <= 1000; n++)
    {
        thispoint = dirpoint[thispoint.face * edge * edge + thispoint.x * edge + thispoint.y].east;

        if (world.nom(thispoint.face, thispoint.x, thispoint.y) > sealevel)
            hitland = 1;
        else
            east++;

        if (hitland == 1)
            n = 1000;
    }

    // West

    hitland = 0;
    thispoint.face = startface;
    thispoint.x = startx;
    thispoint.y = starty;

    for (int n = 1; n <= 1000; n++)
    {
        thispoint = dirpoint[thispoint.face * edge * edge + thispoint.x * edge + thispoint.y].west;

        if (world.nom(thispoint.face, thispoint.x, thispoint.y) > sealevel)
            hitland = 1;
        else
            west++;

        if (hitland == 1)
            n = 1000;
    }

    // Northeast

    hitland = 0;
    thispoint.face = startface;
    thispoint.x = startx;
    thispoint.y = starty;

    for (int n = 1; n <= 1000; n++)
    {
        thispoint = dirpoint[thispoint.face * edge * edge + thispoint.x * edge + thispoint.y].north;
        thispoint = dirpoint[thispoint.face * edge * edge + thispoint.x * edge + thispoint.y].east;

        if (world.nom(thispoint.face, thispoint.x, thispoint.y) > sealevel)
            hitland = 1;
        else
            northeast++;

        if (hitland == 1)
            n = 1000;
    }

    // Southeast

    hitland = 0;
    thispoint.face = startface;
    thispoint.x = startx;
    thispoint.y = starty;

    for (int n = 1; n <= 1000; n++)
    {
        thispoint = dirpoint[thispoint.face * edge * edge + thispoint.x * edge + thispoint.y].south;
        thispoint = dirpoint[thispoint.face * edge * edge + thispoint.x * edge + thispoint.y].east;

        if (world.nom(thispoint.face, thispoint.x, thispoint.y) > sealevel)
            hitland = 1;
        else
            southeast++;

        if (hitland == 1)
            n = 1000;
    }

    // Southwest

    hitland = 0;
    thispoint.face = startface;
    thispoint.x = startx;
    thispoint.y = starty;

    for (int n = 1; n <= 1000; n++)
    {
        thispoint = dirpoint[thispoint.face * edge * edge + thispoint.x * edge + thispoint.y].south;
        thispoint = dirpoint[thispoint.face * edge * edge + thispoint.x * edge + thispoint.y].west;

        if (world.nom(thispoint.face, thispoint.x, thispoint.y) > sealevel)
            hitland = 1;
        else
            southwest++;

        if (hitland == 1)
            n = 1000;
    }

    // Northwest

    hitland = 0;
    thispoint.face = startface;
    thispoint.x = startx;
    thispoint.y = starty;

    for (int n = 1; n <= 1000; n++)
    {
        thispoint = dirpoint[thispoint.face * edge * edge + thispoint.x * edge + thispoint.y].north;
        thispoint = dirpoint[thispoint.face * edge * edge + thispoint.x * edge + thispoint.y].west;

        if (world.nom(thispoint.face, thispoint.x, thispoint.y) > sealevel)
            hitland = 1;
        else
            northwest++;

        if (hitland == 1)
            n = 1000;
    }

    float total = (float)(north + south + east + west + southeast + southwest + northeast + northwest);

    total = total * mult1;

    total = total * world.lunar(); // Lunar pull multiplies tides.

    int tide = (int)total; // In theory this could be as high as 80, but never in practice assuming lunar = 1.0. The highest in the real world is 16 metres, so divide this value by 4 to get it in metres.

    return (tide);
}

// This function creates the rivers.

void createrivermap(planet& world, vector<vector<vector<int>>>& mountaindrainage, vector<fourglobepoints>& dirpoint)
{
    int edge = world.edge();
    float tilt = world.tilt();

    int mountainheightlimit = 600; // Rivers won't go from mountains of this height or over to other mountains of this height or over.
    int minimum = 40; // We won't bother with tiles with lower average flow than this.
    int maxrepeat = 2; // If a river goes in the same direction for this long, it will try to change course.

    int neighbours[8][2];

    neighbours[0][0] = 0;
    neighbours[0][1] = -1;

    neighbours[1][0] = 1;
    neighbours[1][1] = -1;

    neighbours[2][0] = 1;
    neighbours[2][1] = 0;

    neighbours[3][0] = 1;
    neighbours[3][1] = 1;

    neighbours[4][0] = 0;
    neighbours[4][1] = 1;

    neighbours[5][0] = -1;
    neighbours[5][1] = 1;

    neighbours[6][0] = -1;
    neighbours[6][1] = -0;

    neighbours[7][0] = -1;
    neighbours[7][1] = -1;

    // First, we go through the map and mark the direction of water flow on every land tile.
    // Note that these directions are relative to the current face, so not really north, south, etc.

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.sea(face, i, j) == 0)
                {
                    int dir = findlowestdir(world, neighbours, face, i, j);

                    if (dir == -1) // No lowest neighbour! (This never actually happens.)
                        dir = random(1, 8);

                    world.setriverdir(face, i, j, dir);
                }
                else
                    world.setriverdir(face, i, j, 0);
            }
        }
    }

    // Now remove many of the diagonals if possible.

    removediagonalrivers(world);

    // Now add diagonals at some junctions.

    adddiagonalriverjunctions(world);

    // Now make parallel rivers flow into each other.

    removeparallelrivers(world);

    // Now divert rivers away from volcanoes.

    avoidvolcanoes(world);

    // Now check for crossing rivers.

    removecrossingrivers(world);

    // Now ensure that there are no diagonals going between faces 0-3 and faces 4-5.

    removefacediagonalrivers(world);

    // Now, we go through the map tile by tile. Take the rainfall in each tile and add it to every downstream tile.

    vector<int> thisdrop(6 * edge * edge, 0);

    int dropno = 1;
    bool goahead = 1;

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.sea(face, i, j) == 0 && world.strato(face, i, j) == 0)
                {
                    goahead = 1;

                    if (world.mountainheight(face, i, j) > mountainheightlimit) // Don't trace rivers from one high mountainous point to another, as it erodes the mountains too much.
                    {
                        int dir = world.riverdir(face, i, j);

                        int x = i;
                        int y = j;

                        globepoint thispoint;
                        thispoint.face = face;
                        thispoint.x = i;
                        thispoint.y = j;

                        if (dir == 8 || dir == 1 || dir == 2)
                            thispoint = getglobepoint(edge, thispoint.face, thispoint.x, thispoint.y, 0, -1);

                        if (thispoint.face != -1)
                        {
                            if (dir == 4 || dir == 5 || dir == 6)
                                thispoint = getglobepoint(edge, thispoint.face, thispoint.x, thispoint.y, 0, 1);;

                            if (thispoint.face != -1)
                            {
                                if (dir == 2 || dir == 3 || dir == 4)
                                    thispoint = getglobepoint(edge, thispoint.face, thispoint.x, thispoint.y, 1, 0);

                                if (thispoint.face != -1)
                                {
                                    if (dir == 6 || dir == 7 || dir == 8)
                                        thispoint = getglobepoint(edge, thispoint.face, thispoint.x, thispoint.y, -1, 0);
                                }
                            }
                        }

                        if (thispoint.face = -1 || world.mountainheight(thispoint.face, thispoint.x, thispoint.y) > mountainheightlimit)
                            goahead = 0;
                    }

                    if (goahead == 1)
                    {
                        tracedrop(world, face, i, j, minimum, dropno, thisdrop, maxrepeat, neighbours,dirpoint);
                        dropno++;
                    }
                }
            }
        }
    }

    if (tilt < 10.f) // For worlds with low axial tilt, reduce any seasonal difference in river flow.
    {
        int adjustfactor = (int)tilt;

        for (int face = 0; face < 6; face++)
        {
            for (int i = 0; i < edge; i++)
            {
                for (int j = 0; j < edge; j++)
                {
                    int aveflow = (world.riverjan(face, i, j) + world.riverjul(face, i, j)) / 2;

                    if (aveflow > 0)
                    {
                        int thisjanflow = world.riverjan(face, i, j) * adjustfactor + aveflow * (10 - adjustfactor);
                        int thisjulflow = world.riverjul(face, i, j) * adjustfactor + aveflow * (10 - adjustfactor);

                        world.setriverjan(face, i, j, thisjanflow);
                        world.setriverjul(face, i, j, thisjulflow);
                    }
                }
            }
        }
    }
}

// This function traces a drop downhill to the sea, depositing water as it goes.

void tracedrop(planet& world, int face, int x, int y, int minimum, int dropno, vector<int>& thisdrop, int maxrepeat, int neighbours[8][2], vector<fourglobepoints>& dirpoint)
{
    globepoint newseatile;
    globepoint dest;

    int edge = world.edge();
    float riverfactor = world.riverfactor();

    float janload = (float)world.janrain(face, x, y);
    float julload = (float)world.julrain(face, x, y);

    if ((janload + julload) / 2.0f < (float)minimum)
        return;

    janload = janload / riverfactor;
    julload = julload / riverfactor;

    bool keepgoing = 1;

    // This bit moves some water from the winter load to the summer. This is because winter precipitation feeds into the river more slowly than summer precipitation does.

    float amount = 10.0f;

    if (world.mintemp(face, x, y) < 0) // In cold areas, more winter water gets held over until summer because it's snow.
        amount = 3.0f;

    if (face == 4 || (face != 5 && y < edge / 2)) // Northern hemisphere
    {
        float diff = janload / amount;

        janload = janload - diff;
        julload = julload + diff;
    }
    else // Southern hemisphere
    {
        float diff = julload / amount;

        julload = julload - diff;
        janload = janload + diff;
    }

    if (janload < 0.0f)
        janload = 0.0f;

    if (julload < 0.0f)
        julload = 0.0f;

    // Now we just trace the drop through the map, increasing the water flow wherever it goes.

    int dir = -1;

    int index = face * edge * edge + x * edge + y;

    do
    {
        thisdrop[index] = dropno;

        world.setriverjan(face, x, y, world.riverjan(face, x, y) + (int)janload);
        world.setriverjul(face, x, y, world.riverjul(face, x, y) + (int)julload);

        dir = world.riverdir(face, x, y);

        globepoint thispoint;
        thispoint.face = face;
        thispoint.x = x;
        thispoint.y = y;

        if (dir == 8 || dir == 1 || dir == 2)
            thispoint = getglobepoint(edge, thispoint.face, thispoint.x, thispoint.y, 0, -1);

        if (dir == 4 || dir == 5 || dir == 6)
            thispoint = getglobepoint(edge, thispoint.face, thispoint.x, thispoint.y, 0, 1);

        if (dir == 2 || dir == 3 || dir == 4)
        {
            if (thispoint.face != -1)
                thispoint = getglobepoint(edge, thispoint.face, thispoint.x, thispoint.y, 1, 0);
        }

        if (dir == 6 || dir == 7 || dir == 8)
        {
            if (thispoint.face != -1)
                thispoint = getglobepoint(edge, thispoint.face, thispoint.x, thispoint.y, -1, 0);
        }

        // Correct problems with some of the edges. (Going 3->5 is fine.)
        /*
        if (face == 3 && thispoint.face == 4 && thispoint.x == 1)
            thispoint.x = 0;

        if (face == 4 && thispoint.face == 3 && thispoint.y == 1)
            thispoint.y = 0;

        if (face == 2 && thispoint.face == 4 && thispoint.y == 1)
            thispoint.y = 0;
        */
    

        face = thispoint.face;
        x = thispoint.x;
        y = thispoint.y;

        index = face * edge * edge + x * edge + y;

        if (face == -1)
            keepgoing = 0;
        else
        {
            if (world.sea(face, x, y) == 1) // We'll continue the river into the sea for a couple of tiles, to help with the regional map generation later.
            {
                for (int n = 1; n <= 3; n++)
                {
                    world.setriverjan(face, x, y, world.riverjan(face, x, y) + (int)janload);
                    world.setriverjul(face, x, y, world.riverjul(face, x, y) + (int)julload);

                    newseatile = findseatile(world, face, x, y, dir);

                    dir = getdir(edge, face, x, y, newseatile.face, newseatile.x, newseatile.y);

                    world.setriverdir(face, x, y, dir);

                    face = newseatile.face;
                    x = newseatile.x;
                    y = newseatile.y;
                }

                return;
            }

            if (thisdrop[index] == dropno)
            {
                keepgoing = 0;
                world.setlakesurface(face, x, y, world.maxelevation() * 2); // Put a lake seed at the problem point to hide it!
            }
        }

    } while (keepgoing == 1);
}

// This function forces rivers to avoid diagonals, as they don't look so good on the regional map.

void removediagonalrivers(planet& world)
{
    int edge = world.edge();

    int adjustchance = 1; // Probability of adjusting these bits. The higher it is, the less likely.

    int desti, destj, adji, adjj, newdir1, newdir2, newheight;

    for (int face = 0; face < 6; face++)
    {
        for (int i = 1; i < edge -1; i++)
        {
            for (int j = 1; j < edge -1; j++)
            {
                if (random(1, adjustchance) == 1)
                {
                    bool candoit = 1;

                    // Going northeast

                    if (world.riverdir(face, i, j) == 2)
                    {
                        desti = i + 1;
                        destj = j - 1;

                        if (random(0, 1) == 1)
                        {
                            adji = i;
                            adjj = j - 1;

                            newdir1 = 1;
                            newdir2 = 3;

                            if (world.riverdir(face, adji, adjj) == 5 || world.riverdir(face, desti, destj) == 7)
                                candoit = 0;
                        }
                        else
                        {
                            adji = i + 1;
                            adjj = j;

                            newdir1 = 3;
                            newdir2 = 1;

                            if (world.riverdir(face, adji, adjj) == 7 || world.riverdir(face, desti, destj) == 5)
                                candoit = 0;
                        }

                        newheight = (world.nom(face, i, j) + world.nom(face, desti, destj)) / 2;

                        if (newheight > world.nom(face, adji, adjj))
                            candoit = 0;

                        if (candoit == 1)
                        {
                            world.setnom(face, adji, adjj, newheight);
                            world.setriverdir(face, i, j, newdir1);
                            world.setriverdir(face, adji, adjj, newdir2);
                        }
                    }

                    // Going northwest

                    if (world.riverdir(face, i, j) == 8)
                    {
                        desti = i - 1;
                        destj = j - 1;

                        if (random(0, 1) == 1)
                        {
                            adji = i;
                            adjj = j - 1;

                            newdir1 = 1;
                            newdir2 = 7;

                            if (world.riverdir(face, adji, adjj) == 5 || world.riverdir(face, desti, destj) == 3)
                                candoit = 0;
                        }
                        else
                        {
                            adji = i - 1;
                            adjj = j;

                            newdir1 = 7;
                            newdir2 = 1;

                            if (world.riverdir(face, adji, adjj) == 5 || world.riverdir(face, desti, destj) == 3)
                                candoit = 0;

                        }

                        newheight = (world.nom(face, i, j) + world.nom(face, desti, destj)) / 2;

                        if (newheight > world.nom(face, adji, adjj))
                            candoit = 0;

                        if (candoit == 1)
                        {
                            world.setnom(face, adji, adjj, newheight);
                            world.setriverdir(face, i, j, newdir1);
                            world.setriverdir(face, adji, adjj, newdir2);
                        }
                    }

                    // Going southeast

                    if (world.riverdir(face, i, j) == 4)
                    {
                        desti = i + 1;
                        destj = j + 1;

                        if (random(0, 1) == 1)
                        {
                            adji = i;
                            adjj = j + 1;

                            newdir1 = 5;
                            newdir2 = 3;

                            if (world.riverdir(face, adji, adjj) == 1 || world.riverdir(face, desti, destj) == 7)
                                candoit = 0;
                        }
                        else
                        {
                            adji = i + 1;
                            adjj = j;

                            newdir1 = 3;
                            newdir2 = 5;

                            if (world.riverdir(face, adji, adjj) == 7 || world.riverdir(face, desti, destj) == 1)
                                candoit = 0;

                        }

                        newheight = (world.nom(face, i, j) + world.nom(face, desti, destj)) / 2;

                        if (newheight > world.nom(face, adji, adjj))
                            candoit = 0;

                        if (candoit == 1)
                        {
                            world.setnom(face, adji, adjj, newheight);
                            world.setriverdir(face, i, j, newdir1);
                            world.setriverdir(face, adji, adjj, newdir2);
                        }
                    }

                    // Going southwest

                    if (world.riverdir(face, i, j) == 6)
                    {
                        desti = i - 1;
                        destj = j + 1;

                        if (random(0, 1) == 1)
                        {
                            adji = i;
                            adjj = j + 1;

                            newdir1 = 5;
                            newdir2 = 7;

                            if (world.riverdir(face, adji, adjj) == 1 || world.riverdir(face, desti, destj) == 3)
                                candoit = 0;
                        }
                        else
                        {
                            adji = i - 1;
                            adjj = j;

                            newdir1 = 7;
                            newdir2 = 5;

                            if (world.riverdir(face, adji, adjj) == 3 || world.riverdir(face, desti, destj) == 1)
                                candoit = 0;

                        }

                        newheight = (world.nom(face, i, j) + world.nom(face, desti, destj)) / 2;

                        if (newheight > world.nom(face, adji, adjj))
                            candoit = 0;

                        if (candoit == 1)
                        {
                            world.setnom(face, adji, adjj, newheight);
                            world.setriverdir(face, i, j, newdir1);
                            world.setriverdir(face, adji, adjj, newdir2);
                        }
                    }
                }
            }
        }
    }
}

// This function makes rivers meet each other at diagonals, which looks a little more realistic.

void adddiagonalriverjunctions(planet& world)
{
    int edge = world.edge();

    // First the N/S and E/W ones

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge -1; i++)
        {
            for (int j = 0; j < edge -1; j++)
            {
                // Going east

                if (world.riverdir(face, i, j) == 3 && world.riverdir(face, i, j + 1) == 1)
                    world.setriverdir(face, i, j + 1, 2);

                if (world.riverdir(face, i, j + 1) == 3 && world.riverdir(face, i, j) == 5)
                    world.setriverdir(face, i, j, 4);

                // Going west

                if (world.riverdir(face, i + 1, j) == 7 && world.riverdir(face, i + 1, j + 1) == 1)
                    world.setriverdir(face, i + 1, j + 1, 8);

                if (world.riverdir(face, i + 1, j + 1) == 7 && world.riverdir(face, i + 1, j) == 5)
                    world.setriverdir(face, i + 1, j, 6);

                // Going north

                if (world.riverdir(face, i, j + 1) == 1 && world.riverdir(face, i + 1, j + 1) == 7)
                    world.setriverdir(face, i + 1, j + 1, 8);

                if (world.riverdir(face, i + 1, j + 1) == 1 && world.riverdir(face, i, j + 1) == 3)
                    world.setriverdir(face, i, j + 1, 2);

                // Going south

                if (world.riverdir(face, i, j) == 5 && world.riverdir(face, i + 1, j) == 7)
                    world.setriverdir(face, i + 1, j, 6);

                if (world.riverdir(face, i + 1, j) == 5 && world.riverdir(face, i, j) == 3)
                    world.setriverdir(face, i, j, 4);
            }
        }
    }

    // Now the diagonal ones.

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge -1; i++)
        {
            for (int j = 0; j < edge -1; j++)
            {
                // Going northeast

                if (world.riverdir(face, i, j) == 4 && world.riverdir(face, i + 1, j + 1) == 2)
                {
                    if (world.nom(face, i + 1, j) > world.nom(face, i + 2, j))
                    {
                        world.setriverdir(face, i, j, 3);
                        world.setriverdir(face, i + 1, j, 3);

                        if (world.nom(face, i + 1, j) > world.nom(face, i, j))
                            world.setnom(face, i + 1, j, world.nom(face, i, j) - 1);
                    }
                }

                if (world.riverdir(face, i, j + 1) == 2 && world.riverdir(face, i + 1, j + 2) == 8)
                {
                    if (world.nom(face, i + 1, j + 1) > world.nom(face, i + 1, j))
                    {
                        world.setriverdir(face, i + 1, j + 2, 1);
                        world.setriverdir(face, i + 1, j + 1, 1);

                        if (world.nom(face, i + 1, j + 1) > world.nom(face, i + 1, j + 2))
                            world.setnom(face, i + 1, j + 1, world.nom(face, i + 1, j + 1) - 1);
                    }
                }

                // Going southwest

                if (world.riverdir(face, i, j) == 4 && world.riverdir(face, i + 1, j + 1) == 6)
                {
                    if (world.nom(face, i, j + 1) > world.nom(face, i, j + 1))
                    {
                        world.setriverdir(face, i, j, 5);
                        world.setriverdir(face, i, j + 1, 5);

                        if (world.nom(face, i, j + 1) > world.nom(face, i, j))
                            world.setnom(face, i, j + 1, world.nom(face, i, j) - 1);
                    }
                }

                if (world.riverdir(face, i + 1, j) == 6 && world.riverdir(face, i + 2, j + 1) == 8)
                {
                    if (world.nom(face, i + 1, j + 1) > world.nom(face, i, j + 1))
                    {
                        world.setriverdir(face, i + 2, j + 1, 7);
                        world.setriverdir(face, i + 1, j + 1, 7);

                        if (world.nom(face, i + 1, j + 1) > world.nom(face, i + 2, j + 1))
                            world.setnom(face, i + 1, j + 1, world.nom(face, i + 1, j + 1) - 1);
                    }
                }

                // Going southeast

                if (world.riverdir(face, i + 1, j) == 4 && world.riverdir(face, i, j + 1) == 2)
                {
                    if (world.nom(face, i + 1, j + 1) > world.nom(face, i + 2, j + 1))
                    {
                        world.setriverdir(face, i, j + 1, 3);
                        world.setriverdir(face, i + 1, j + 1, 3);

                        if (world.nom(face, i + 1, j + 1) > world.nom(face, i, j + 1))
                            world.setnom(face, i + 1, j + 1, world.nom(face, i, j + 1) - 1);
                    }
                }

                if (world.riverdir(face, i, j + 1) == 4 && world.riverdir(face, i + 1, j) == 6)
                {
                    if (world.nom(face, i + 1, j + 1) > world.nom(face, i + 1, j + 2))
                    {
                        world.setriverdir(face, i + 1, j, 5);
                        world.setriverdir(face, i + 1, j + 1, 5);

                        if (world.nom(face, i + 1, j + 1) > world.nom(face, i + 1, j))
                            world.setnom(face, i + 1, j + 1, world.nom(face, i + 1, j) - 1);
                    }
                }

                // Going northwest

                if (world.riverdir(face, i + 1, j + 1) == 8 && world.riverdir(face, i + 2, j) == 6)
                {
                    if (world.nom(face, i + 1, j) > world.nom(face, i, j))
                    {
                        world.setriverdir(face, i + 2, j, 7);
                        world.setriverdir(face, i + 1, j, 7);

                        if (world.nom(face, i + 1, j) > world.nom(face, i + 2, j))
                            world.setnom(face, i + 1, j, world.nom(face, i + 2, j) - 1);
                    }
                }

                if (world.riverdir(face, i + 1, j + 1) == 8 && world.riverdir(face, i, j + 2) == 2)
                {
                    if (world.nom(face, i, j + 1) > world.nom(face, i, j))
                    {
                        world.setriverdir(face, i, j + 2, 1);
                        world.setriverdir(face, i, j + 1, 1);

                        if (world.nom(face, i, j + 1) > world.nom(face, i, j + 2))
                            world.setnom(face, i, j + 1, world.nom(face, i, j + 2) - 1);
                    }
                }
            }
        }
    }
}

// This function makes parallel rivers run into each other.

void removeparallelrivers(planet& world)
{
    int edge = world.edge();

    int removechance = 1;

    vector<int> altered(6 * edge * edge, 0);

    // First, N/S and E/W

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge - 1; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge - 1; j++)
            {
                if (random(1, removechance) == 1)
                {
                    int index = vi + j;

                    int eindex = index + edge;
                    int sindex = index + 1;
                    int seindex = eindex + 1;

                    // Going east

                    if (world.riverdir(face, i, j) == 3 && world.riverdir(face, i + 1, j) == 3 && world.riverdir(face, i, j + 1) == 3 && world.riverdir(face, i + 1, j + 1) == 3)
                    {
                        if (world.nom(face, i, j) > world.nom(face, i + 1, j + 1) && altered[index] == 0)
                        {
                            world.setriverdir(face, i, j, 4);

                            altered[index] = 1;
                            altered[eindex] = 1;
                            altered[sindex] = 1;
                            altered[seindex] = 1;
                        }
                        else
                        {
                            if (world.nom(face, i, j + 1) > world.nom(face, i + 1, j) && altered[sindex] == 0)
                            {
                                world.setriverdir(face, i, j + 1, 2);

                                altered[index] = 1;
                                altered[eindex] = 1;
                                altered[sindex] = 1;
                                altered[seindex] = 1;
                            }
                        }
                    }

                    // Going west

                    if (world.riverdir(face, i, j) == 7 && world.riverdir(face, i + 1, j) == 7 && world.riverdir(face, i, j + 1) == 7 && world.riverdir(face, i + 1, j + 1) == 7)
                    {
                        if (world.nom(face, i + 1, j) > world.nom(face, i, j + 1) && altered[eindex] == 0)
                        {
                            world.setriverdir(face, i + 1, j, 6);

                            altered[index] = 1;
                            altered[eindex] = 1;
                            altered[sindex] = 1;
                            altered[seindex] = 1;
                        }
                        else
                        {
                            if (world.nom(face, i + 1, j + 1) > world.nom(face, i, j) && altered[seindex] == 0)
                            {
                                world.setriverdir(face, i + 1, j + 1, 8);

                                altered[index] = 1;
                                altered[eindex] = 1;
                                altered[sindex] = 1;
                                altered[seindex] = 1;
                            }
                        }
                    }

                    // Going north

                    if (world.riverdir(face, i, j) == 1 && world.riverdir(face, i, j + 1) == 1 && world.riverdir(face, i + 1, j) == 1 && world.riverdir(face, i + 1, j + 1) == 7)
                    {
                        if (world.nom(face, i, j + 1) > world.nom(face, i + 1, j) && altered[sindex] == 0)
                        {
                            world.setriverdir(face, i, j + 1, 2);

                            altered[index] = 1;
                            altered[eindex] = 1;
                            altered[sindex] = 1;
                            altered[seindex] = 1;
                        }
                        else
                        {
                            if (world.nom(face, i + 1, j + 1) > world.nom(face, i, j) && altered[seindex] == 0)
                            {
                                world.setriverdir(face, i + 1, j + 1, 8);

                                altered[index] = 1;
                                altered[eindex] = 1;
                                altered[sindex] = 1;
                                altered[seindex] = 1;
                            }
                        }
                    }

                    // Going south

                    if (world.riverdir(face, i, j) == 5 && world.riverdir(face, i, j + 1) == 5 && world.riverdir(face, i + 1, j) == 5 && world.riverdir(face, i + 1, j + 1) == 5)
                    {
                        if (world.nom(face, i, j) > world.nom(face, i + 1, j + 1) && altered[index] == 0)
                        {
                            world.setriverdir(face, i, j, 4);

                            altered[index] = 1;
                            altered[eindex] = 1;
                            altered[sindex] = 1;
                            altered[seindex] = 1;
                        }
                        else
                        {
                            if (world.nom(face, i + 1, j) > world.nom(face, i, j + 1) && altered[eindex] == 0)
                            {
                                world.setriverdir(face, i + 1, j, 6);

                                altered[index] = 1;
                                altered[eindex] = 1;
                                altered[sindex] = 1;
                                altered[seindex] = 1;
                            }
                        }
                    }
                }
            }
        }
    }

    // Now the diagonals.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge - 2; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge - 2; j++)
            {
                if (random(1, removechance) == 1)
                {
                    int index = vi + j;
                    int eindex = index + edge;
                    int seindex = eindex + 1;
                    int seeindex = seindex + edge;
                    int sindex = index + 1;
                    int sseindex = seindex + 1;

                    // Going northeast

                    if (world.riverdir(face, i, j + 1) == 2 && world.riverdir(face, i + 1, j) == 2 && world.riverdir(face, i + 1, j + 2) == 2 && world.riverdir(face, i + 2, j + 1) == 2)
                    {
                        if (world.nom(face, i, j + 1) > world.nom(face, i + 2, j + 1) && world.nom(face, i + 1, j + 1) > world.nom(face, i + 2, j + 1) && altered[sindex] == 0)
                        {
                            world.setriverdir(face, i, j + 1, 3);
                            world.setriverdir(face, i + 1, j + 1, 3);

                            altered[sindex] = 1;
                            altered[eindex] = 1;
                            altered[seeindex] = 1;
                            altered[seindex] = 1;

                            if (world.nom(face, i + 1, j + 1) > world.nom(face, i, j + 1))
                                world.setnom(face, i + 1, j + 1, world.nom(face, i, j + 1) - 1);

                        }
                        else
                        {
                            if (world.nom(face, i + 1, j + 2) > world.nom(face, i + 1, j) && world.nom(face, i + 1, j + 1) > world.nom(face, i + 1, j) && altered[sseindex] == 0)
                            {
                                world.setriverdir(face, i + 1, j + 2, 1);
                                world.setriverdir(face, i + 1, j + 1, 1);

                                altered[sindex] = 1;
                                altered[eindex] = 1;
                                altered[seeindex] = 1;
                                altered[seindex] = 1;

                                if (world.nom(face, i + 1, j + 1) > world.nom(face, i + 1, j + 2))
                                    world.setnom(face, i + 1, j + 1, world.nom(face, i + 1, j + 2) - 1);
                            }
                        }
                    }

                    // Going southwest

                    if (world.riverdir(face, i, j + 1) == 6 && world.riverdir(face, i + 1, j) == 6 && world.riverdir(face, i + 1, j + 2) == 6 && world.riverdir(face, i + 2, j + 1) == 6)
                    {
                        if (world.nom(face, i + 1, j) > world.nom(face, i + 1, j + 2) && world.nom(face, i + 1, j + 1) > world.nom(face, i + 1, j + 2) && altered[eindex] == 0)
                        {
                            world.setriverdir(face, i + 1, j, 5);
                            world.setriverdir(face, i + 1, j + 1, 5);

                            altered[sindex] = 1;
                            altered[eindex] = 1;
                            altered[seeindex] = 1;
                            altered[seindex] = 1;

                            if (world.nom(face, i + 1, j + 1) > world.nom(face, i + 1, j))
                                world.setnom(face, i + 1, j + 1, world.nom(face, i + 1, j) - 1);

                        }
                        else
                        {
                            if (world.nom(face, i + 2, j + 1) > world.nom(face, i, j + 1) && world.nom(face, i + 1, j + 1) > world.nom(face, i, j + 1) && altered[seeindex] == 0)
                            {
                                world.setriverdir(face, i + 2, j + 1, 7);
                                world.setriverdir(face, i + 1, j + 1, 7);

                                altered[sindex] = 1;
                                altered[eindex] = 1;
                                altered[seeindex] = 1;
                                altered[seindex] = 1;

                                if (world.nom(face, i + 1, j + 1) > world.nom(face, i + 2, j + 1))
                                    world.setnom(face, i + 1, j + 1, world.nom(face, i + 2, j + 1) - 1);
                            }
                        }
                    }

                    // Going southeast

                    if (world.riverdir(face, i, j + 1) == 4 && world.riverdir(face, i + 1, j) == 4 && world.riverdir(face, i + 1, j + 2) == 4 && world.riverdir(face, i + 2, j + 1) == 4)
                    {
                        if (world.nom(face, i + 1, j) > world.nom(face, i + 1, j + 2) && world.nom(face, i + 1, j + 1) > world.nom(face, i + 1, j + 2) && altered[eindex] == 0)
                        {
                            world.setriverdir(face, i + 1, j, 5);
                            world.setriverdir(face, i + 1, j + 1, 5);

                            altered[sindex] = 1;
                            altered[eindex] = 1;
                            altered[seeindex] = 1;
                            altered[seindex] = 1;

                            if (world.nom(face, i + 1, j + 1) > world.nom(face, i + 1, j))
                                world.setnom(face, i + 1, j + 1, world.nom(face, i + 1, j) - 1);

                        }
                        else
                        {
                            if (world.nom(face, i, j + 1) > world.nom(face, i + 2, j + 1) && world.nom(face, i + 1, j + 1) > world.nom(face, i + 2, j + 1) && altered[sindex] == 0)
                            {
                                world.setriverdir(face, i, j + 1, 3);
                                world.setriverdir(face, i + 1, j + 1, 3);

                                altered[sindex] = 1;
                                altered[eindex] = 1;
                                altered[seeindex] = 1;
                                altered[seindex] = 1;

                                if (world.nom(face, i + 1, j + 1) > world.nom(face, i, j + 1))
                                    world.setnom(face, i + 1, j + 1, world.nom(face, i, j + 1) - 1);
                            }
                        }
                    }

                    // Going northwest

                    if (world.riverdir(face, i, j + 1) == 8 && world.riverdir(face, i + 1, j) == 8 && world.riverdir(face, i + 1, j + 2) == 8 && world.riverdir(face, i + 2, j + 1) == 8)
                    {
                        if (world.nom(face, i + 2, j + 1) > world.nom(face, i, j + 1) && world.nom(face, i + 1, j + 1) > world.nom(face, i, j + 1) && altered[seeindex] == 0)
                        {
                            world.setriverdir(face, i + 2, j + 1, 7);
                            world.setriverdir(face, i + 1, j + 1, 7);

                            altered[sindex] = 1;
                            altered[eindex] = 1;
                            altered[seeindex] = 1;
                            altered[seindex] = 1;

                            if (world.nom(face, i + 1, j + 1) > world.nom(face, i + 2, j + 1))
                                world.setnom(face, i + 1, j + 1, world.nom(face, i + 2, j + 1) - 1);

                        }
                        else
                        {
                            if (world.nom(face, i + 1, j + 2) > world.nom(face, i + 1, j) && world.nom(face, i + 1, j + 1) > world.nom(face, i + 1, j) && altered[sseindex] == 0)
                            {
                                world.setriverdir(face, i + 1, j + 2, 1);
                                world.setriverdir(face, i + 1, j + 1, 1);

                                altered[sindex] = 1;
                                altered[eindex] = 1;
                                altered[seeindex] = 1;
                                altered[seindex] = 1;

                                if (world.nom(face, i + 1, j + 1) > world.nom(face, i + 1, j + 2))
                                    world.setnom(face, i + 1, j + 1, world.nom(face, i + 1, j + 2) - 1);
                            }
                        }
                    }
                }
            }
        }
    }
}

// This function makes rivers avoid volcanoes where possible.

void avoidvolcanoes(planet& world)
{
    int edge = world.edge();
    int maxelev = world.maxelevation();

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                int dir = world.riverdir(face, i, j);

                globepoint dest = getdestination(edge, face, i, j, dir);

                if (world.volcano(dest.face, dest.x, dest.y) != 0)
                {
                    int currentelev = world.nom(face, i, j);

                    int lowest = maxelev;

                    globepoint newpoint;
                    newpoint = dest;

                    for (int k = - 1; k <= 1; k++)
                    {
                        for (int l = -1; l <= 1; l++)
                        {
                            globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                            if (lpoint.face != -1)
                            {
                                if (lpoint.face != face || lpoint.x != i || lpoint.y != j)
                                {
                                    if (world.volcano(lpoint.face, lpoint.x, lpoint.y) == 0)
                                    {
                                        int nom = world.nom(lpoint.face, lpoint.x, lpoint.y);

                                        if (nom < currentelev && nom < lowest)
                                        {
                                            lowest = world.nom(lpoint.face, lpoint.x, lpoint.y);
                                            newpoint = lpoint;
                                        }
                                    }
                                }
                            }
                        }
                    }

                    int newdir = getdir(edge, face, i, j, newpoint.face, newpoint.x, newpoint.y);

                    world.setriverdir(face, i, j, newdir);
                }
            }
        }
    }
}

// This function removes any rivers that cross each other.

void removecrossingrivers(planet& world)
{
    int edge = world.edge();

    for (int face = 0; face < 6; face++)
    {
        for (int i = 1; i < edge-1; i++)
        {
            for (int j = 1; j < edge-1; j++)
            {
                // Going northeast

                if (world.riverdir(face, i, j) == 2)
                {
                    int ii = i;
                    int jj = j - 1;

                    if (jj >= 0 && jj < edge)
                    {
                        if (world.riverdir(face, ii, jj) == 4)
                        {
                            if (world.nom(face, ii, jj) <= world.nom(face, i, j))
                                world.setriverdir(face, i, j, 1);
                        }
                    }

                    ii = i + 1;
                    jj = j;

                    if (world.riverdir(face, ii, jj) == 8)
                    {
                        if (world.nom(face, ii, jj) <= world.nom(face, i, j))
                            world.setriverdir(face, i, j, 3);
                    }
                }

                // Going southeast

                if (world.riverdir(face, i, j) == 4)
                {
                    int ii = i;
                    int jj = j + 1;

                    if (world.riverdir(face, ii, jj) == 2)
                    {
                        if (world.nom(face, ii, jj) <= world.nom(face, i, j))
                            world.setriverdir(face, i, j, 5);
                    }

                    ii = i + 1;
                    jj = j;

                    if (world.riverdir(face, ii, jj) == 6)
                    {
                        if (world.nom(face, ii, jj) <= world.nom(face, i, j))
                            world.setriverdir(face, i, j, 3);
                    }
                }

                // Going southwest

                if (world.riverdir(face, i, j) == 6)
                {
                    int ii = i;
                    int jj = j + 1;

                    if (world.riverdir(face, ii, jj) == 8)
                    {
                        if (world.nom(face, ii, jj) <= world.nom(face, i, j))
                            world.setriverdir(face, i, j, 5);
                    }

                    ii = i - 1;
                    jj = j;

                    if (world.riverdir(face, ii, jj) == 4)
                    {
                        if (world.nom(face, ii, jj) <= world.nom(face, i, j))
                            world.setriverdir(face, i, j, 7);
                    }
                }

                // Going northwest

                if (world.riverdir(face, i, j) == 8)
                {
                    int ii = i;
                    int jj = j - 1;

                    if (world.riverdir(face, ii, jj) == 6)
                    {
                        if (world.nom(face, ii, jj) <= world.nom(face, i, j))
                            world.setriverdir(face, i, j, 1);
                    }

                    ii = i - 1;
                    jj = j;

                    if (world.riverdir(face, ii, jj) == 2)
                    {
                        if (world.nom(face, ii, jj) <= world.nom(face, i, j))
                            world.setriverdir(face, i, j, 7);
                    }
                }
            }
        }
    }
}

// This removes diagonal rivers crossing over awkward face boundaries.

void removefacediagonalrivers(planet& world)
{
    int edge = world.edge();

    for (int n = 0; n < edge; n++)
    {
        // face 1

        int dir = world.riverdir(1, n, 0);

        if (dir == 8)
        {
            world.setriverdir(1, n, 0, 1);
            world.setriverdir(4, edge - 1, edge - n - 1, 5);

            if (world.nom(4, edge - 1, edge - n - 1) > world.nom(1, n, 0))
                world.setnom(4, edge - 1, edge - n - 1, world.nom(1, n, 0));
        }

        if (dir == 2)
        {
            world.setriverdir(1, n, 0, 1);
            world.setriverdir(4, edge - 1, edge - n - 1, 1);

            if (world.nom(4, edge - 1, edge - n - 1) > world.nom(1, n, 0))
                world.setnom(4, edge - 1, edge - n - 1, world.nom(1, n, 0));
        }

        dir = world.riverdir(1, n, edge - 1);

        if (dir == 4)
        {
            world.setriverdir(1, n, edge - 1, 5);
            world.setriverdir(5, edge - 1, edge - n - 1, 5);

            if (world.nom(5, edge - 1, edge - n - 1) > world.nom(1, n, edge - 1))
                world.setnom(5, edge - 1, edge - n - 1, world.nom(1, n, edge - 1));
        }

        if (dir == 6)
        {
            world.setriverdir(1, n, edge - 1, 5);
            world.setriverdir(5, edge - 1, edge - n - 1, 1);

            if (world.nom(5, edge - 1, edge - n - 1) > world.nom(1, n, edge - 1))
                world.setnom(5, edge - 1, edge - n - 1, world.nom(1, n, edge - 1));
        }

        // face 2

        dir = world.riverdir(2, n, 0);

        if (dir == 8)
        {
            world.setriverdir(2, n, 0, 1);
            world.setriverdir(4, edge - n - 1, 0, 3);

            if (world.nom(4, edge - n - 1, 0) > world.nom(2, n, 0))
                world.setnom(4, edge - n - 1, 0, world.nom(2, n, 0));
        }

        if (dir == 2)
        {
            world.setriverdir(2, n, 0, 1);
            world.setriverdir(4, edge - n - 1, 0, 7);

            if (world.nom(4, edge - n - 1, 0) > world.nom(2, n, 0))
                world.setnom(4, edge - n - 1, 0, world.nom(2, n, 0));
        }

        dir = world.riverdir(2, n, edge - 1);

        if (dir == 4)
        {
            world.setriverdir(2, n, edge - 1, 5);
            world.setriverdir(5, edge - n - 1, edge - 1, 7);

            if (world.nom(5, edge - n - 1, edge - 1) > world.nom(2, n, edge - 1))
                world.setnom(5, edge - n - 1, edge - 1, world.nom(2, n, edge - 1));
        }

        if (dir == 6)
        {
            world.setriverdir(2, n, edge - 1, 5);
            world.setriverdir(5, edge - n - 1, edge - 1, 3);

            if (world.nom(5, edge - n - 1, edge - 1) > world.nom(2, n, edge - 1))
                world.setnom(5, edge - n - 1, edge - 1, world.nom(2, n, edge - 1));
        }

        // face 3

        dir = world.riverdir(3, n, 0);

        if (dir == 8)
        {
            world.setriverdir(3, n, 0, 1);
            world.setriverdir(4, 0, edge - n - 1, 1);

            if (world.nom(4, 0, edge - n - 1) > world.nom(3, n, 0))
                world.setnom(4, 0, edge - n - 1, world.nom(3, n, 0));
        }

        if (dir == 2)
        {
            world.setriverdir(3, n, 0, 1);
            world.setriverdir(4, 0, edge - n - 1, 5);

            if (world.nom(4, 0, edge - n - 1) > world.nom(3, n, 0))
                world.setnom(4, 0, edge - n - 1, world.nom(3, n, 0));
        }

        dir = world.riverdir(3, n, edge - 1);

        if (dir == 4)
        {
            world.setriverdir(3, n, edge - 1, 5);
            world.setriverdir(5, 0, edge - n - 1, 1);

            if (world.nom(5, 0, edge - n - 1) > world.nom(3, n, edge - 1))
                world.setnom(5, 0, edge - n - 1, world.nom(3, n, edge - 1));
        }

        if (dir == 6)
        {
            world.setriverdir(3, n, edge - 1, 5);
            world.setriverdir(5, 0, edge - n - 1, 5);

            if (world.nom(5, 0, edge - n - 1) > world.nom(3, n, edge - 1))
                world.setnom(5, 0, edge - n - 1, world.nom(3, n, edge - 1));
        }

        // face 4

        dir = world.riverdir(4, n, 0);

        if (dir == 8)
        {
            world.setriverdir(4, n, 0, 1);
            world.setriverdir(2, edge - n - 1, 0, 3);

            if (world.nom(2, edge - n - 1, 0) > world.nom(4, n, 0))
                world.setnom(2, edge - n - 1, 0, world.nom(4, n, 0));
        }

        if (dir == 2)
        {
            world.setriverdir(4, n, 0, 1);
            world.setriverdir(2, edge - n - 1, 0, 7);

            if (world.nom(2, edge - n - 1, 0) > world.nom(4, n, 0))
                world.setnom(2, edge - n - 1, 0, world.nom(4, n, 0));
        }

        dir = world.riverdir(4, 0, n);

        if (dir == 6)
        {
            world.setriverdir(4, 0, n, 7);
            world.setriverdir(3, n, 0, 3);

            if (world.nom(3, n, 0) > world.nom(4, 0, n))
                world.setnom(3, n, 0, world.nom(4, 0, n));
        }

        if (dir == 8)
        {
            world.setriverdir(4, 0, n, 7);
            world.setriverdir(3, n, 0, 7);

            if (world.nom(3, n, 0) > world.nom(4, 0, n))
                world.setnom(3, n, 0, world.nom(4, 0, n));
        }

        dir = world.riverdir(4, edge - 1, n);

        if (dir == 2)
        {
            world.setriverdir(4, edge - 1, n, 3);
            world.setriverdir(1, edge - n - 1, 0, 3);

            if (world.nom(1, edge - n - 1, 0) > world.nom(4, edge - 1, n))
                world.setnom(1, edge - n - 1, 0, world.nom(4, edge - 1, n));
        }

        if (dir == 4)
        {
            world.setriverdir(4, edge - 1, n, 3);
            world.setriverdir(1, edge - n - 1, 0, 7);

            if (world.nom(1, edge - n - 1, 0) > world.nom(4, edge - 1, n))
                world.setnom(1, edge - n - 1, 0, world.nom(4, edge - 1, n));
        }

        // face 5

        dir = world.riverdir(5, n, edge - 1);

        if (dir == 4)
        {
            world.setriverdir(5, n, edge - 1, 5);
            world.setriverdir(2, edge - n - 1, edge - 1, 7);

            if (world.nom(2, edge - n - 1, edge - 1) > world.nom(5, n, edge - 1))
                world.setnom(2, edge - n - 1, edge - 1, world.nom(5, n, edge - 1));
        }

        if (dir == 6)
        {
            world.setriverdir(5, n, edge - 1, 5);
            world.setriverdir(2, edge - n - 1, edge - 1, 3);

            if (world.nom(2, edge - n - 1, edge - 1) > world.nom(5, n, edge - 1))
                world.setnom(2, edge - n - 1, edge - 1, world.nom(5, n, edge - 1));
        }

        dir = world.riverdir(5, 0, n);

        if (dir == 6)
        {
            world.setriverdir(5, 0, n, 7);
            world.setriverdir(3, edge - n - 1, edge - 1, 7);

            if (world.nom(3, edge - n - 1, edge - 1) > world.nom(5, 0, n))
                world.setnom(3, edge - n - 1, edge - 1, world.nom(5, 0, n));
        }

        if (dir == 8)
        {
            world.setriverdir(5, 0, n, 7);
            world.setriverdir(3, edge - n - 1, edge - 1, 3);

            if (world.nom(3, edge - n - 1, edge - 1) > world.nom(5, 0, n))
                world.setnom(3, edge - n - 1, edge - 1, world.nom(5, 0, n));
        }

        dir = world.riverdir(5, edge - 1, n);

        if (dir == 2)
        {
            world.setriverdir(5, edge - 1, n, 3);
            world.setriverdir(1, n, edge - 1, 7);

            if (world.nom(1, n, edge - 1) > world.nom(5, edge - 1, n))
                world.setnom(1, n, edge - 1, world.nom(5, edge - 1, n));
        }

        if (dir == 4)
        {
            world.setriverdir(5, edge - 1, n, 3);
            world.setriverdir(1, n, edge - 1, 3);

            if (world.nom(1, n, edge - 1) > world.nom(5, edge - 1, n))
                world.setnom(1, n, edge - 1, world.nom(5, edge - 1, n));
        }
    }
}

// This function removes mountains that have rivers running over them.

void removerivermountains(planet& world)
{
    int edge = world.edge();

    int amount = 1; // Distance around mountain rivers to remove mountains.
    int mountainremovechance = 16; // The higher this is, the fewer mountains around rivers will be removed.

    int minremoveheight = 100; // Only mountains higher than this will be removed.

    vector<bool> toremove(6 * edge * edge, 0);

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.sea(face, i, j) == 0 && world.mountainheight(face, i, j) == 0)
                {
                    globepoint dest = getflowdestination(world, face, i, j, 0);

                    if (world.mountainheight(dest.face, dest.x, dest.y) > minremoveheight && world.sea(dest.face, dest.x, dest.y) == 0) // We've got flow from a non-mountain cell into a mountain cell!
                    {
                        int z = face;
                        int x = i;
                        int y = j;

                        for (int n = 0; n < edge; n++)
                        {
                            for (int k = -amount; k <= amount; k++)
                            {
                                for (int l = -amount; l <= amount; l++)
                                {
                                    globepoint lpoint = getglobepoint(edge, z, x, y, k, l);

                                    if (lpoint.face != -1 && random(1, mountainremovechance) == 1)
                                        toremove[lpoint.face * edge * edge + lpoint.x * edge + lpoint.y] = 1;
                                }
                            }

                            toremove[z * edge * edge + x * edge + y] = 1;

                            dest = getflowdestination(world, z, x, y, 0);

                            z = dest.face;
                            x = dest.x;
                            y = dest.y;

                            if (world.sea(z, x, y) == 1)
                                n = edge;

                            if (world.mountainheight(z, x, y) == 0)
                                n = edge;
                        }
                    }
                }
            }
        }
    }

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                if (toremove[vi + j] == 1 && world.sea(face, i, j) == 0)
                {
                    world.setmintemp(face, i, j, tempelevremove(world, world.mintemp(face, i, j), face, i, j));
                    world.setmaxtemp(face, i, j, tempelevremove(world, world.maxtemp(face, i, j), face, i, j));

                    int totalrain = world.winterrain(face, i, j) + world.summerrain(face, i, j);

                    short crount = 0;

                    int winterraintotal = 0;
                    int summerraintotal = 0;

                    for (int x = -1; x <= 1; x++)
                    {
                        globepoint xpoint = getglobepoint(edge, face, i, j, x, 0);

                        if (xpoint.face != -1)
                        {
                            for (int y = -1; y <= 1; y++)
                            {
                                globepoint ypoint = getglobepoint(edge, xpoint.face, xpoint.x, xpoint.y, 0, y);

                                if (ypoint.face != -1)
                                {
                                    if (ypoint.x != y || ypoint.y != j)
                                    {
                                        if (world.mountainheight(ypoint.face, ypoint.x, ypoint.y) == 0 && toremove[ypoint.face * edge * edge + ypoint.x * edge + ypoint.y] == 0)
                                        {
                                            crount++;

                                            winterraintotal = winterraintotal + world.winterrain(ypoint.face, ypoint.x, ypoint.y);
                                            summerraintotal = summerraintotal + world.summerrain(ypoint.face, ypoint.x, ypoint.y);
                                        }
                                    }
                                }
                            }
                        }
                    }

                    if (crount > 0)
                    {
                        world.setwinterrain(face, i, j, winterraintotal / crount);
                        world.setsummerrain(face, i, j, summerraintotal / crount);
                    }

                    world.setmountainridge(face, i, j, 0);
                    world.setmountainheight(face, i, j, 0);
                }
            }
        }
    }

    //cleanmountainridges(world);
}

// This function creates the major freshwater lakes.

void createlakemap(planet& world, vector<int>& nolake, boolshapetemplate smalllake[], boolshapetemplate largelake[], vector<fourglobepoints>& dirpoint)
{
    int edge = world.edge();

    int glacialtemp = world.glacialtemp();

    int minlake = 1000; // Flows of this size or higher might have lakes on them.
    int maxlake = 12000; // Flows larger than this can't have lakes.
    int lakechance = random(8, 16); //2; //10; //random(80,150); // The higher this is, the less likely lakes are.
    int minrain = 200;
    int maxtemp = 25; // Lakes won't appear where there is less rain *and* higher temperature than this.

    vector<int> thislake(6 * edge * edge, 0);
    vector<int> checked(6 * edge * edge, 0);
    vector<bool> lakeprobability(6 * edge * edge, 0);

    int clusternumber = 50; // Number of possible clusters of lakes on the map.
    int minrad = 10;
    int maxrad = 40; // Possible sizes of the clusters

    for (int n = 0; n < clusternumber; n++) // Put some circles on this array, to allow for lake clusters.
    {
        int centreface = random(0, 5);
        int centrex = random(maxrad + 1, edge - maxrad - 1);
        int centrey = random(maxrad + 1, edge - maxrad - 1);

        int vface = centreface * edge * edge;

        int radius = random(minrad, maxrad);

        for (int i = -radius; i <= radius; i++)
        {
            int ii = centrex + i;

            int vii = vface + ii * edge;

            for (int j = -radius; j <= radius; j++)
            {
                int jj = centrey + j;

                if (i * i + j * j < radius * radius + radius)
                    lakeprobability[vii + jj] = 1;
            }
        }
    }

    // Now make the lakes

    twointegers nearestsea;
    int lakeno = 0;

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int flow = world.riveraveflow(face, i, j);

                if (world.averain(face, i, j) >= minrain || world.avetemp(face, i, j) <= maxtemp)
                {
                    int index = vi + j;

                    if (flow >= minlake && flow <= maxlake && lakeprobability[index] == 1 && random(1, lakechance) == 1)
                    {
                        if (nolake[index] == 0)
                        {
                            lakeno++;

                            int templatesize = 1;
                            int shapenumber;

                            if (world.avetemp(face, i, j) <= glacialtemp)
                                shapenumber = random(0, 3);
                            else
                            {
                                shapenumber = random(2, 11);

                                if (random(1, 6) != 1)
                                    shapenumber = random(2, 11);
                                else
                                {
                                    shapenumber = random(0, 9);
                                    templatesize = 2;
                                }
                            }
                            drawlake(world, shapenumber, face, i, j, thislake, lakeno, checked, nolake, templatesize, minrain, maxtemp, smalllake, largelake);
                        }
                    }
                }
            }
        }
    }

    // Now clean up a little.

    cleanlakes(world);

    // Now we need to add precipitation from the lakes.

    vector<int> lakewinterrainmap(6 * edge * edge, 0);
    vector<int> lakesummerrainmap(6 * edge * edge, 0);

    lakerain(world, lakewinterrainmap, lakesummerrainmap, dirpoint);

    // Now we add new rivers from that extra precipitation.

    lakerivers(world, lakewinterrainmap, lakesummerrainmap);

    // Now we check that no rivers shrink inexplicably. (Turned off for now as it has a weird bug.)

    //checkglobalflows(world);
}

// Puts a lake template onto the lake map.

void drawlake(planet& world, int shapenumber, int centreface, int centrex, int centrey, vector<int>& thislake, int lakeno, vector<int>& checked, vector<int>& nolake, int templatesize, int minrain, int maxtemp, boolshapetemplate smalllake[], boolshapetemplate largelake[])
{
    int edge = world.edge();
    int width = edge - 1;
    int height = edge - 1;
    int riverlandreduce = world.riverlandreduce();

    if (centrey == 0 || centrey == height)
    {
        world.setlakesurface(centreface, centrex, centrey, 0);
        return;
    }

    twointegers nearestsea, flow;

    int minlakedistance = 10; // Minimum distance between lakes.
    int minmountaindistance = 2; // Minimum distance from mountains.
    int maxmountainlakeheight = 1000; // Ignore mountains smaller than this.
    float mindepth = 5; // Minimum depth.

    int imheight, imwidth;

    if (templatesize == 1)
    {
        imheight = smalllake[shapenumber].ysize() - 1;
        imwidth = smalllake[shapenumber].xsize() - 1;
    }
    else
    {
        imheight = largelake[shapenumber].ysize() - 1;
        imwidth = largelake[shapenumber].xsize() - 1;
    }

    int x = centrex - imwidth / 2; // Coordinates of the top left corner.
    int y = centrey - imheight / 2;

    if (x<0 || x>width || y<0 || y>height)
        return;

    int leftx = centrex;
    int lefty = centrey;
    int rightx = centrex;
    int righty = centrey; // These are the coordinates of the furthermost pixels of the lake.

    int leftr = random(1, 2); // If it's 1 then we reverse it left-right
    int downr = random(1, 2); // If it's 1 then we reverse it top-bottom

    int istart = 0, desti = imwidth + 1, istep = 1;
    int jstart = 0, destj = imheight + 1, jstep = 1;

    if (leftr == 1)
    {
        istart = imwidth;
        desti = -1;
        istep = -1;
    }

    if (downr == 1)
    {
        jstart = imwidth;
        destj = -1;
        jstep = -1;
    }

    // First, check to see if the area is clear.

    int vface = centreface * edge * edge;

    int mapi = -1;
    int mapj = -1; // These are the offsets on the actual map, as opposed to on the lake template.

    for (int i = istart; i != desti; i = i + istep)
    {
        mapi++;
        mapj = -1;

        for (int j = jstart; j != destj; j = j + jstep)
        {
            mapj++;

            bool point;

            if (templatesize == 1)
                point = smalllake[shapenumber].point(i, j);
            else
                point = largelake[shapenumber].point(i, j);

            if (point == 1)
            {
                int xx = x + mapi;
                int yy = y + mapj;

                if (yy >= 0 && yy <= height && xx >= 0 && xx <= width)
                {
                    if (world.sea(centreface, xx, yy) == 0)
                    {
                        if (nolake[vface + xx * edge + yy] == 1)
                            return;

                        if (world.volcano(centreface, xx, yy) != 0)
                            return;

                        for (int xxx = xx - minlakedistance; xxx <= xx + minlakedistance; xxx++)
                        {
                            int xxxx = xxx;

                            if (xxxx<0 || xxxx>width)
                                return;

                            for (int yyy = yy - minlakedistance; yyy <= yy + minlakedistance; yyy++)
                            {
                                if (yyy >= 0 && yyy <= height)
                                {
                                    if (thislake[vface + xxxx * edge + yyy] != 0 || world.volcano(centreface, xxxx, yyy) != 0) // There's another lake here already! Or a volcano.
                                        return;
                                }
                                else
                                    return;
                            }
                        }

                        for (int xxx = xx - minmountaindistance; xxx <= xx + minmountaindistance; xxx++)
                        {
                            int xxxx = xxx;

                            if (xxxx<0 || xxxx>width)
                                return;

                            for (int yyy = yy - minmountaindistance; yyy <= yy + minmountaindistance; yyy++)
                            {
                                if (yyy >= 0 && yyy <= height)
                                {
                                    if (world.mountainheight(centreface,xxxx, yyy) > maxmountainlakeheight) // There are mountains here.
                                        return;

                                }
                                else
                                    return;
                            }
                        }
                    }
                    else
                        return;
                }
                else
                    return;
            }
        }
    }

    // Now put down the actual lake.

    mapi = -1;
    mapj = -1;

    for (int i = istart; i != desti; i = i + istep)
    {
        mapi++;
        mapj = -1;

        for (int j = jstart; j != destj; j = j + jstep)
        {
            mapj++;

            bool point;

            if (templatesize == 1)
                point = smalllake[shapenumber].point(i, j);
            else
                point = largelake[shapenumber].point(i, j);

            if (point == 1)
            {
                int xx = x + mapi;
                int yy = y + mapj;

                thislake[vface + xx * edge + yy] = lakeno; // Mark it on the keeping-track array.

                if (xx < leftx)
                    leftx = xx;

                if (xx > rightx)
                    rightx = xx;

                if (yy < lefty)
                    lefty = yy;
                if (yy > righty)
                    righty = yy;
            }
        }
    }

    // Now we find out what the lowest point on the edge of the lake is. This will be the point from which the outflowing river emerges, and it will determine the surface level of the whole lake.

    int smallest = world.maxelevation();
    int outflowx = -1;
    int outflowy = -1;

    for (int i = leftx; i <= rightx; i++)
    {
        int vi = vface + i * edge;

        for (int j = lefty; j <= righty; j++)
        {
            if (thislake[vi + j] == lakeno)
            {
                float border = 0;

                for (int k = i - 1; k <= i + 1; k++)
                {
                    int vk = vface + k * edge;

                    for (int l = j - 1; l <= j + 1; l++)
                    {
                        if (thislake[vk + l] != lakeno)
                        {
                            border = 1;
                            k = i + 1;
                            l = j + 1;
                        }
                    }
                }

                if (border == 1)
                {
                    if (world.nom(centreface, i, j) < smallest)
                    {
                        smallest = world.nom(centreface, i, j);
                        outflowx = i;
                        outflowy = j;
                    }
                }
            }
        }
    }

    int surfaceheight = smallest - riverlandreduce;

    // Now make the lake itself that height.

    for (int i = leftx; i <= rightx; i++)
    {
        int vi = vface + i * edge;

        for (int j = lefty; j <= righty; j++)
        {
            if (thislake[vi + j] == lakeno)
                world.setlakesurface(centreface, i, j, surfaceheight);
        }
    }

    // Now we must alter the terrain height underneath the lake.

    int maxdepth = imwidth / 8;

    if (maxdepth < 10)
        maxdepth = 10;

    if (maxdepth > 40)
        maxdepth = 40;

    float depthmult = (float)random((int)mindepth, maxdepth); // The higher this is, the deeper the lake will be.

    float maxmaxdepth = (float)random(500, 1000); // Absolute maximum depth for lakes in this world.

    for (int i = leftx; i <= rightx; i++)
    {
        int vi = vface + i * edge;

        for (int j = lefty; j <= righty; j++)
        {
            if (thislake[vi + j] == lakeno)
            {
                float nom = (float)world.nom(centreface, i, j);

                float depth = nom - (float)surfaceheight;

                if (depth < 0.0f) // If the elevation here is higher than the lake surface
                    depth = (0.0f - depth) * depthmult;

                if (depth > maxmaxdepth)
                    depth = maxmaxdepth;

                if (depth < mindepth)
                    depth = mindepth;

                int newnom = surfaceheight - (int)depth;

                if (newnom < 1)
                    newnom = 1;

                world.setnom(centreface, i, j, newnom);
            }
        }
    }

    // Now do the rivers.

    sortlakerivers(world, centreface, leftx, lefty, rightx, righty, centrex, centrey, outflowx, outflowy, thislake, lakeno);

    // Now mark a "start point" somewhere on the edge of the lake, which we will use when it comes to the regional map.

    makelakestartpoint(world, thislake, lakeno, centreface, leftx, lefty, rightx, righty);
}

// Puts a lake template onto the lakemap, but as a "special", e.g. salt pans etc.

void drawspeciallake(planet& world, int shapenumber, int centreface, int centrex, int centrey, int lakeno, vector<int>& thislake, boolshapetemplate laketemplate[], bool canoverlap, int special)
{
    int edge = world.edge();    
    int width = edge - 1;
    int height = edge - 1;

    int vface = centreface * edge * edge;

    if (centrey == 0 || centrey == height)
    {
        world.setlakesurface(centreface, centrex, centrey, 0);
        return;
    }

    twointegers nearestsea, flow;

    int minlakedistance = 4; // Minimum distance between lakes.
    int maxmountainlakeheight = 1000; // Ignore mountains smaller than this.
    int mindepth = 5; // Minimum depth.

    int surfaceheight;

    int imheight = laketemplate[shapenumber].ysize() - 1;
    int imwidth = laketemplate[shapenumber].xsize() - 1;

    int x = centrex - imwidth / 2; // Coordinates of the top left corner.
    int y = centrey - imheight / 2;

    if (x<0 || x>width || y<0 || y>height)
        return;

    int leftx = centrex;
    int lefty = centrey;
    int rightx = centrex;
    int righty = centrey; // These are the coordinates of the furthermost pixels of the lake.
    bool wrapped = 0; // If this is 1, the lake wraps over the edge of the map.

    int leftr = random(0, 1); // If it's 1 then we reverse it left-right
    int downr = random(0, 1); // If it's 1 then we reverse it top-bottom

    int istart = 0, desti = imwidth + 1, istep = 1;
    int jstart = 0, destj = imheight + 1, jstep = 1;

    if (leftr == 1)
    {
        istart = imwidth;
        desti = -1;
        istep = -1;
    }

    if (downr == 1)
    {
        jstart = imwidth;
        destj = -1;
        jstep = -1;
    }

    // First, check that the location is clear. Also, get the height of the lowest point in it, and make that the surface height.

    int mapi = -1;
    int mapj = -1;

    surfaceheight = 1000000000;

    bool tooclose = 0;

    for (int i = istart; i != desti; i = i + istep)
    {
        mapi++;
        mapj = -1;

        for (int j = jstart; j != destj; j = j + jstep)
        {
            mapj++;

            if (laketemplate[shapenumber].point(i, j) == 1)
            {
                int xx = x + mapi;
                int yy = y + mapj;

                if (xx != 0 && xx <= width && world.nom(centreface, xx, yy) < surfaceheight)
                    surfaceheight = world.nom(centreface, xx, yy);

                if (xx>=0 && xx<=width && yy >= 0 && yy <= height)
                {
                    if (world.sea(centreface, xx, yy) != 0 || world.mountainheight(centreface, xx, yy) >= maxmountainlakeheight) // Don't try to put them on top of sea or mountains...
                        tooclose = 1;
                    else
                    {
                        int clim = 1; // This marks whether the climate is appropriate.

                        if (special == 110 || special == 120)
                        {
                            for (int k = xx - 1; k <= xx + 1; k++)
                            {
                                if (k >= 0 && k <= width)
                                {
                                    for (int l = yy - 1; l <= yy + 1; l++)
                                    {
                                        if (l >= 0 && l <= height)
                                        {
                                            if (world.climate(centreface, k, l) != 5 || world.riverjan(centreface, k, l) > 0 || world.riverjul(centreface, k, l) > 0)
                                            {
                                                clim = 0;

                                                k = xx + 1;
                                                l = yy + 1;
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        if (clim != 1) // Don't put lakes onto inappropriate climates.
                            tooclose = 1;
                        else
                        {
                            if (world.outline(centreface, xx, yy) == 1)
                                tooclose = 1;
                            else
                            {
                                for (int xxx = xx - minlakedistance; xxx <= xx + minlakedistance; xxx++)
                                {
                                    int xxxx = xxx;

                                    if (xxxx >= 0 && xxxx <= width)
                                    {
                                        int vx = vface + xxxx * edge;

                                        for (int yyy = yy - minlakedistance; yyy <= yy + minlakedistance; yyy++)
                                        {
                                            if (yyy >= 0 && yyy <= height)
                                            {
                                                int yindex = vx + y;

                                                if (thislake[yindex] != 0 && thislake[yindex] != lakeno && canoverlap == 0) // There's another lake here already!
                                                    tooclose = 1;

                                                if (world.lakesurface(centreface, xxxx, yyy) != 0 && thislake[yindex] != lakeno && (canoverlap == 0 || (special != 120 || world.special(centreface, xxxx, yyy) != 120))) // There's another kind of lake here already! (Doesn't apply when both are ergs.)
                                                    tooclose = 1;

                                                if (world.sea(centreface, xxxx, yyy) == 1) // && special != 120) // Sea here (doesn't apply to ergs).
                                                    tooclose = 1;
                                            }
                                            else
                                                tooclose = 1;
                                        }
                                    }
                                    else
                                        tooclose = 1;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (tooclose == 1)
        return;

    // Now, do the actual thing.

    mapi = -1;
    mapj = -1;

    for (int i = istart; i != desti; i = i + istep)
    {
        mapi++;
        mapj = -1;

        for (int j = jstart; j != destj; j = j + jstep)
        {
            mapj++;

            if (laketemplate[shapenumber].point(i, j) == 1)
            {
                int xx = x + mapi;
                int yy = y + mapj;

                if (xx>=0 && xx<=width && yy >= 0 && yy <= height)
                {
                    int index = vface + xx * edge + yy;

                    world.setlakesurface(centreface, xx, yy, surfaceheight); // Put the lake on this bit of the lake map.
                    world.setnom(centreface, xx, yy, surfaceheight);
                    world.setspecial(centreface, xx, yy, special);

                    thislake[index] = lakeno; // Mark it on the keeping-track array too.

                    if (world.mountainheight(centreface, xx, yy) != 0) // Remove any mountains that might be here.
                    {
                        for (int dir = 1; dir <= 8; dir++)
                            deleteridge(world, centreface, xx, yy, dir);
                    }

                    if (xx < leftx)
                        leftx = xx;

                    if (xx > rightx)
                        rightx = xx;

                    if (yy < lefty)
                        lefty = yy;
                    if (yy > righty)
                        righty = yy;
                }
            }
        }
    }

    if (wrapped == 1)
    {
        leftx = 0;
        lefty = 0;
        rightx = width;
        righty = height;
    }

    // Now mark a "start point" somewhere on the edge of the "lake", which we will use when it comes to the regional map.

    makelakestartpoint(world, thislake, lakeno, centreface, leftx, lefty, rightx, righty);
}

// This creates a "start point" for a lake, which will be used for drawing it at the regional level.

void makelakestartpoint(planet& world, vector<int>& thislake, int lakeno, int face, int leftx, int lefty, int rightx, int righty)
{
    int edge = world.edge();

    twointegers startpoint;
    startpoint.x = -1;
    startpoint.y = -1;

    int vface = face * edge * edge;

    int crount = 0;

    do
    {
        for (int i = leftx; i <= rightx; i++)
        {
            int vi = vface + i * edge;

            for (int j = lefty; j <= righty; j++)
            {
                if (thislake[vi + j] == lakeno)
                {
                    if (lakeoutline(world, thislake, lakeno, face, i, j) == 1) // If this is on the edge of the lake
                    {
                        if (random(1, 3) == 1)
                        {
                            startpoint.x = i;
                            startpoint.y = j;
                        }
                    }
                }
            }
        }

        crount++;

        if (crount > 1000) // Something's gone horribly wrong
            return;

    } while (startpoint.x == -1);

    world.setlakestart(face, startpoint.x, startpoint.y, 1);
}

// This checks whether a given tile is on the edge of a given lake.

int lakeoutline(planet& world, vector<int>& thislake, int lakeno, int face, int x, int y)
{
    int edge = world.edge();

    int vface = face * edge * edge;

    int xx = x - 1;
    int yy = y;

    if (xx >= 0 && xx < edge && yy >= 0 && yy < edge)
    {
        if (thislake[vface + xx * edge + yy] != lakeno)
            return(1);
    }

    xx = x + 1;
    yy = y;

    if (xx >= 0 && xx < edge && yy >= 0 && yy < edge)
    {
        if (thislake[vface + xx * edge + yy] != lakeno)
            return(1);
    }

    xx = x;
    yy = y - 1;

    if (xx >= 0 && xx < edge && yy >= 0 && yy < edge)
    {
        if (thislake[vface + xx * edge + yy] != lakeno)
            return(1);
    }

    xx = x;
    yy = y + 1;

    if (xx >= 0 && xx < edge && yy >= 0 && yy < edge)
    {
        if (thislake[vface + xx * edge + yy] != lakeno)
            return(1);
    }

    xx = x - 1;
    yy = y - 1;

    if (xx >= 0 && xx < edge && yy >= 0 && yy < edge)
    {
        if (thislake[vface + xx * edge + yy] != lakeno)
            return(1);
    }

    xx = x + 1;
    yy = y + 1;

    if (xx >= 0 && xx < edge && yy >= 0 && yy < edge)
    {
        if (thislake[vface + xx * edge + yy] != lakeno)
            return(1);
    }

    xx = x + 1;
    yy = y - 1;

    if (xx >= 0 && xx < edge && yy >= 0 && yy < edge)
    {
        if (thislake[vface + xx * edge + yy] != lakeno)
            return(1);
    }

    xx = x - 1;
    yy = y + 1;

    if (xx >= 0 && xx < edge && yy >= 0 && yy < edge)
    {
        if (thislake[vface + xx * edge + yy] != lakeno)
            return(1);
    }

    return (0);
}

// Deals with the rivers going into and out of a lake.

void sortlakerivers(planet& world, int face, int leftx, int lefty, int rightx, int righty, int centrex, int centrey, int outflowx, int outflowy, vector<int>& thislake, int lakeno)
{
    int edge = world.edge();

    // Mark the route of the outflowing river (if any) on this array. We will avoid messing with any of it.

    vector<int> outflow(6 * edge * edge, 0);

    markriver(world, face, outflowx, outflowy, outflow, 0);

    // Now go over the lake area. Every river tile that is under or next to the lake should be pointing to the centre of the lake.

    vector<int> checkedriverlaketiles(6 * edge * edge, 0);
    vector<int> removedrivers(6 * edge * edge, 0);

    int riverno = 0;

    int vface = face * edge * edge;

    for (int i = leftx; i <= rightx; i++)
    {
        for (int j = lefty; j <= righty; j++)
        {
            for (int k = i - 1; k <= i + 1; k++)
            {
                int vk = vface + k * edge;

                for (int l = j - 1; l <= j + 1; l++)
                {
                    // k and l are the points around the current one.

                    int lindex = vk + l;

                    if (outflow[lindex] == 1)
                        checkedriverlaketiles[lindex] = 1;

                    if (checkedriverlaketiles[lindex] == 0)
                    {
                        bool mustdivert = 0;

                        if (world.lakesurface(face, k, l) != 0)
                            mustdivert = 1;
                        else
                        {
                            if (nexttolake(world, face, k, l) != 0) // If it's next to our lake
                                mustdivert = 1;
                        }

                        if (mustdivert == 1)
                        {
                            riverno++;

                            riverno = divertlakeriver(world, face, k, l, face, centrex, centrey, removedrivers, riverno, face, outflowx, outflowy, outflow);
                        }

                        checkedriverlaketiles[lindex] = 1;
                    }
                }
            }
        }
    }

    // At this point, the outflowing river is reduced to nothing or almost nothing.
    // Now we need to work out how big it should actually be.

    for (int f = 0; f < 6; f++)
    {
        int vf = f * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vf + i * edge;

            for (int j = 0; j < edge; j++)
                checkedriverlaketiles[vi + j] = 0;
        }
    }

    leftx--;
    lefty--;
    rightx++;
    righty++;

    if (leftx < 0)
        leftx = 0;

    if (lefty < 0)
        lefty = 0;

    if (rightx > edge - 1)
        rightx = edge - 1;

    if (righty > edge - 1)
        righty = edge - 1;

    int janload = 0;
    int julload = 0;

    int origjanoutflow = 0;
    int origjuloutflow = 0;

    for (int i = leftx; i <= rightx; i++)
    {
        int vi = vface + i * edge;

        for (int j = lefty; j <= righty; j++)
        {
            if (thislake[vi + j] != lakeno)
            {
                float border = 0;

                for (int k = i - 1; k <= i + 1; k++)
                {
                    int vk = vface + k * edge;

                    for (int l = j - 1; l <= j + 1; l++)
                    {
                        if (thislake[vk + l] == lakeno)
                        {
                            border = 1;
                            k = i + 1;
                            l = j + 1;
                        }
                    }
                }

                if (border == 1)
                {
                    janload = janload + world.riverjan(face, i, j);
                    julload = julload + world.riverjul(face, i, j);
                }
            }
        }
    }

    if (janload < 0)
        janload = 0;

    if (julload < 0)
        julload = 0;

    janload = janload / 2;
    julload = julload / 2; // Because as it stands, it contains the original outflows as well as the inflows.

    if (janload == 0 && julload == 0) // If somehow we've ended up with zero flow for the outflowing river
    {
        janload = origjanoutflow;
        julload = origjuloutflow;
    }

    // Now we apply the new load to the outflowing river.

    vector<int> loadadded(6 * edge * edge, 0);

    int x = outflowx;
    int y = outflowy;
    bool keepgoing = 1;

    int yindex = vface + x * edge + y;

    do
    {
        if (loadadded[yindex] == 0) // Just to make sure it's not somehow going round in circles.
        {
            world.setriverjan(face, x, y, world.riverjan(face, x, y) + janload);
            world.setriverjul(face, x, y, world.riverjul(face, x, y) + julload);

            loadadded[yindex] = 1;

            int dir = world.riverdir(face, x, y);

            globepoint thispoint;
            thispoint.face = face;
            thispoint.x = x;
            thispoint.y = y;

            if (dir == 8 || dir == 1 || dir == 2)
                thispoint = getglobepoint(edge, thispoint.face, thispoint.x, thispoint.y, 0, -1);

            if (dir == 4 || dir == 5 || dir == 6)
                thispoint = getglobepoint(edge, thispoint.face, thispoint.x, thispoint.y, 0, 1);

            if (dir == 2 || dir == 3 || dir == 4)
            {
                if (thispoint.face != -1)
                    thispoint = getglobepoint(edge, thispoint.face, thispoint.x, thispoint.y, 1, 0);
            }

            if (dir == 6 || dir == 7 || dir == 8)
            {
                if (thispoint.face != -1)
                    thispoint = getglobepoint(edge, thispoint.face, thispoint.x, thispoint.y, -1, 0);
            }

            // Correct problems with some of the edges. (Going 3->5 is fine.)
            /*
            if (face == 3 && thispoint.face == 4 && thispoint.x == 1)
                thispoint.x = 0;

            if (face == 4 && thispoint.face == 3 && thispoint.y == 1)
                thispoint.y = 0;

            if (face == 2 && thispoint.face == 4 && thispoint.y == 1)
                thispoint.y = 0;
            */

            face = thispoint.face;
            x = thispoint.x;
            y = thispoint.y;

            yindex = face * edge * edge + x * edge + y;

            if (thislake[yindex] == lakeno && (x != outflowx || y != outflowy)) // If it's somehow flowed back into the lake!
                keepgoing = 0;

            if (world.sea(face, x, y) == 1)
                keepgoing = 0;

            if (world.truelake(face, x, y) != 0 && thislake[yindex] != lakeno) // If it's gone into another lake
                keepgoing = 0;
        }
        else
            keepgoing = 0;

    } while (keepgoing == 1);
}

// This marks the route of a river on an array, from the given point.

void markriver(planet& world, int face, int x, int y, vector<int>& markedarray, int extra)
{
    int edge = world.edge();

    if (y<0 || y>edge - 1 || x<0 || x>edge - 1)
        return;

    bool keepgoing = 1;

    int index = face * edge * edge + x * edge + y;

    do
    {
        if (markedarray[index] == 1) // If we're somehow going round in a loop)
            return;

        markedarray[index] = 1;

        if (extra == 1)
        {
            for (int i = - 1; i <= 1; i++)
            {
                globepoint ipoint = getglobepoint(edge, face, x, y, i, 0);

                if (ipoint.face != -1)
                {
                    for (int j = - 1; j <= 1; j++)
                    {
                        globepoint jpoint = getglobepoint(edge, ipoint.face, ipoint.x, ipoint.y, 0, j);

                        if (jpoint.face!=-1)
                        {
                            int jindex = jpoint.face * edge * edge + jpoint.x * edge + jpoint.y;

                            if (markedarray[jindex] == 0)
                                markedarray[jindex] = 2;
                        }
                    }
                }
            }
        }

        int dir = world.riverdir(face, x, y);

        if (dir == 0)
            return;

        globepoint thispoint;
        thispoint.face = face;
        thispoint.x = x;
        thispoint.y = y;

        if (dir == 8 || dir == 1 || dir == 2)
            thispoint = getglobepoint(edge, thispoint.face, thispoint.x, thispoint.y, 0, -1);

        if (dir == 4 || dir == 5 || dir == 6)
            thispoint = getglobepoint(edge, thispoint.face, thispoint.x, thispoint.y, 0, 1);

        if (dir == 2 || dir == 3 || dir == 4)
        {
            if (thispoint.face != -1)
                thispoint = getglobepoint(edge, thispoint.face, thispoint.x, thispoint.y, 1, 0);
        }

        if (dir == 6 || dir == 7 || dir == 8)
        {
            if (thispoint.face != -1)
                thispoint = getglobepoint(edge, thispoint.face, thispoint.x, thispoint.y, -1, 0);
        }

        // Correct problems with some of the edges. (Going 3->5 is fine.)
        /*
        if (face == 3 && thispoint.face == 4 && thispoint.x == 1)
            thispoint.x = 0;

        if (face == 4 && thispoint.face == 3 && thispoint.y == 1)
            thispoint.y = 0;

        if (face == 2 && thispoint.face == 4 && thispoint.y == 1)
            thispoint.y = 0;
        */
        if (thispoint.face == -1)
            keepgoing = 0;

        face = thispoint.face;
        x = thispoint.x;
        y = thispoint.y;

        index = face * edge * edge + x * edge + y;

        if (world.sea(face, x, y) == 1)
            keepgoing = 0;

    } while (keepgoing == 1);
}

// This function tells us whether a tile is next to a lake.

int nexttolake(planet& world, int face, int x, int y)
{
    int edge = world.edge();

    int dist = 1;

    for (int i = - dist; i <= dist; i++)
    {
        globepoint ipoint = getglobepoint(edge, face, x, y, i, 0);

        if (ipoint.face != -1)
        {
            for (int j = - dist; j <= dist; j++)
            {
                globepoint jpoint = getglobepoint(edge, ipoint.face, ipoint.x, ipoint.y, 0, j);
                
                if (jpoint.face != -1 && world.lakesurface(jpoint.face, jpoint.x, jpoint.y) != 0)
                    return (world.lakesurface(jpoint.face, jpoint.x, jpoint.y));
            }
        }
    }
    return (0);
}

// This diverts a river from the given point to the destination point.

int divertriver(planet& world, int face, int x, int y, int destface, int destx, int desty, vector<int>& removedrivers, int riverno)
{
    int edge = world.edge();
    
    globepoint destpoint;

    while (1 == 1)
    {
        if (face == destface && x == destx && y == desty)
            return (riverno);

        int dir = getdir(edge, face, x, y, destface, destx, desty);

        if (dir == 0)
            return (riverno);

        if (world.riverdir(face, x, y) == dir)
        {
            destpoint = getflowdestination(world, face, x, y, 0);

            face = destpoint.face;
            x = destpoint.x;
            y = destpoint.y;
        }
        else
        {
            // Delete the existing river

            riverno++;

            int janflow = world.riverjan(face, x, y);
            int julflow = world.riverjul(face, x, y);

            removeriver(world, removedrivers, riverno, face, x, y);

            // Now reinstate the river, pointing in the new direction

            world.setriverdir(face, x, y, dir);
            world.setriverjan(face, x, y, janflow);
            world.setriverjul(face, x, y, julflow);

            // Now move to the next point in the new river

            destpoint = getflowdestination(world, face, x, y, 0);

            face = destpoint.face;
            x = destpoint.x;
            y = destpoint.y;

            addtoriver(world, face, x, y, janflow, julflow);
        }
    }
    return (riverno);
}

// This diverts a river that's next to a lake, to go into it.

int divertlakeriver(planet& world, int face, int x, int y, int destface, int destx, int desty, vector<int>& removedrivers, int riverno, int outflowface, int outflowx, int outflowy, vector<int>& avoidarray)
{
    int edge = world.edge();
    
    globepoint destpoint;

    int index = face * edge * edge + x * edge + y;

    for (int n = 1; n < 1000; n++) // It very occasionally gets stuck in an endless loop, so limit the number of times it can go around.
    {
        if (face == destface && x == destx && y == desty)
            return (riverno);

        if (avoidarray[index] == 1)
            return (riverno);

        // First, get the direction to the destination.

        int dir = getdir(edge, face, x, y, destface, destx, desty);

        if (dir == 0)
            return (riverno);

        // Now we have to check to see whether that goes onto dry land, which we don't want.

        destpoint = getflowdestination(world, face, x, y, dir);

        int newdir = 0;

        if (world.lakesurface(destpoint.face, destpoint.x, destpoint.y) == 0)
        {
            // First, see if we can swerve right.

            newdir = dir + 1;

            if (newdir == 9)
                newdir = 1;

            destpoint = getflowdestination(world, face, x, y, newdir);

            if (world.lakesurface(destpoint.face, destpoint.x, destpoint.y) == 0)
            {
                // Now see if we can swerve left.

                newdir = dir - 1;

                if (newdir == 0)
                    newdir = 8;

                destpoint = getflowdestination(world, face, x, y, newdir);

                if (world.lakesurface(destpoint.face, destpoint.x, destpoint.y) == 0)
                {
                    // If we can't, then we will have to stop.

                    int janflow = world.riverjan(face, x, y);
                    int julflow = world.riverjul(face, x, y);

                    world.setriverjan(destface, destx, desty, world.riverjan(destface, destx, desty) + janflow);
                    world.setriverjul(destface, destx, desty, world.riverjul(destface, destx, desty) + julflow);

                    return (riverno);
                }
            }

            dir = newdir;
        }

        if (world.riverdir(face, x, y) == dir)
        {
            destpoint = getflowdestination(world, face, x, y, dir);

            face = destpoint.face;
            x = destpoint.x;
            y = destpoint.y;

            index = face * edge * edge + x * edge + y;
        }
        else
        {
            if (avoidarray[index] == 1)
                return (riverno);

            // Delete the existing river

            riverno++;

            int janflow = world.riverjan(face, x, y);
            int julflow = world.riverjul(face, x, y);

            removeriver(world, removedrivers, riverno, face, x, y);

            // Now reinstate the river, pointing in the new direction

            world.setriverdir(face, x, y, dir);
            world.setriverjan(face, x, y, janflow);
            world.setriverjul(face, x, y, julflow);

            // Now move to the next point in the new river

            destpoint = getflowdestination(world, face, x, y, 0);

            face = destpoint.face;
            x = destpoint.x;
            y = destpoint.y;

            if (avoidarray[index] != 1)
                addtoriver(world, face, x, y, janflow, julflow);

        }
    }
    return (riverno);
}

// This removes a river, starting from the given point.

void removeriver(planet& world, vector<int>& removedrivers, int riverno, int face, int x, int y)
{    
    int janload = world.riverjan(face, x, y);
    int julload = world.riverjul(face, x, y);

    if (janload == 0 && julload == 0) // No river here...
        return;

    int edge = world.edge();

    bool keepgoing = 1;

    do
    {
        int index = face * edge * edge + x * edge + y;

        int dir = world.riverdir(face, x, y);

        removedrivers[index] = riverno;

        if (world.riverjan(face, x, y) - janload >= 0)
            world.setriverjan(face, x, y, world.riverjan(face, x, y) - janload);
        else
            return;

        if (world.riverjul(face, x, y) - julload >= 0)
            world.setriverjul(face, x, y, world.riverjul(face, x, y) - julload);
        else
            return;

        globepoint thispoint;
        thispoint.face = face;
        thispoint.x = x;
        thispoint.y = y;

        if (dir == 8 || dir == 1 || dir == 2)
            thispoint = getglobepoint(edge, thispoint.face, thispoint.x, thispoint.y, 0, -1);

        if (dir == 4 || dir == 5 || dir == 6)
            thispoint = getglobepoint(edge, thispoint.face, thispoint.x, thispoint.y, 0, 1);

        if (dir == 2 || dir == 3 || dir == 4)
        {
            if (thispoint.face != -1)
                thispoint = getglobepoint(edge, thispoint.face, thispoint.x, thispoint.y, 1, 0);
        }

        if (dir == 6 || dir == 7 || dir == 8)
        {
            if (thispoint.face != -1)
                thispoint = getglobepoint(edge, thispoint.face, thispoint.x, thispoint.y, -1, 0);
        }

        // Correct problems with some of the edges. (Going 3->5 is fine.)
        /*
        if (face == 3 && thispoint.face == 4 && thispoint.x == 1)
            thispoint.x = 0;

        if (face == 4 && thispoint.face == 3 && thispoint.y == 1)
            thispoint.y = 0;

        if (face == 2 && thispoint.face == 4 && thispoint.y == 1)
            thispoint.y = 0;
        */
        face = thispoint.face;
        x = thispoint.x;
        y = thispoint.y;

        if (world.sea(face, x, y) == 1)
            keepgoing = 0;

        if (removedrivers[index] == riverno)
            keepgoing = 0;

    } while (keepgoing == 1);
}

// This lowers and reduces a river, starting from the given point.

void reduceriver(planet& world, int janreduce, int julreduce, vector<int>& removedrivers, int riverno, int face, int x, int y)
{
    int edge = world.edge();
    int sealevel = world.sealevel();

    bool keepgoing = 1;

    do
    {
        int index = face * edge * edge + x * edge + y;

        int dir = world.riverdir(face, x, y);

        removedrivers[index] = riverno;

        if (world.riverjan(face, x, y) - janreduce >= 0)
            world.setriverjan(face, x, y, world.riverjan(face, x, y) - janreduce);
        else
            return;

        if (world.riverjul(face, x, y) - julreduce >= 0)
            world.setriverjul(face, x, y, world.riverjul(face, x, y) - julreduce);
        else
            return;

        world.setnom(face, x, y, sealevel + 1);

        if (world.deltadir(face, x, y) == 0)
            world.setdeltadir(face, x, y, -1);

        globepoint thispoint;
        thispoint.face = face;
        thispoint.x = x;
        thispoint.y = y;

        if (dir == 8 || dir == 1 || dir == 2)
            thispoint = getglobepoint(edge, thispoint.face, thispoint.x, thispoint.y, 0, -1);

        if (dir == 4 || dir == 5 || dir == 6)
            thispoint = getglobepoint(edge, thispoint.face, thispoint.x, thispoint.y, 0, 1);

        if (dir == 2 || dir == 3 || dir == 4)
        {
            if (thispoint.face != -1)
                thispoint = getglobepoint(edge, thispoint.face, thispoint.x, thispoint.y, 1, 0);
        }

        if (dir == 6 || dir == 7 || dir == 8)
        {
            if (thispoint.face != -1)
                thispoint = getglobepoint(edge, thispoint.face, thispoint.x, thispoint.y, -1, 0);
        }

        // Correct problems with some of the edges. (Going 3->5 is fine.)
        /*
        if (face == 3 && thispoint.face == 4 && thispoint.x == 1)
            thispoint.x = 0;

        if (face == 4 && thispoint.face == 3 && thispoint.y == 1)
            thispoint.y = 0;

        if (face == 2 && thispoint.face == 4 && thispoint.y == 1)
            thispoint.y = 0;
        */
        face = thispoint.face;
        x = thispoint.x;
        y = thispoint.y;

        if (world.sea(face, x, y) == 1)
            keepgoing = 0;

        if (removedrivers[index] == riverno)
            keepgoing = 0;

    } while (keepgoing == 1);
}

// This adds water to an existing river, starting from the given point.

void addtoriver(planet& world, int face, int x, int y, int janload, int julload)
{
    int edge = world.edge();

    vector<int> thisdrop(6 * edge * edge, 0);

    bool keepgoing = 1;

    int index = face * edge * edge + x * edge + y;

    do
    {
        int dir = world.riverdir(face, x, y);

        thisdrop[index] = 1;

        world.setriverjan(face, x, y, world.riverjan(face, x, y) + janload);
        world.setriverjul(face, x, y, world.riverjul(face, x, y) + julload);

        globepoint thispoint;
        thispoint.face = face;
        thispoint.x = x;
        thispoint.y = y;

        if (dir == 8 || dir == 1 || dir == 2)
            thispoint = getglobepoint(edge, thispoint.face, thispoint.x, thispoint.y, 0, -1);

        if (dir == 4 || dir == 5 || dir == 6)
            thispoint = getglobepoint(edge, thispoint.face, thispoint.x, thispoint.y, 0, 1);

        if (dir == 2 || dir == 3 || dir == 4)
        {
            if (thispoint.face != -1)
                thispoint = getglobepoint(edge, thispoint.face, thispoint.x, thispoint.y, 1, 0);
        }

        if (dir == 6 || dir == 7 || dir == 8)
        {
            if (thispoint.face != -1)
                thispoint = getglobepoint(edge, thispoint.face, thispoint.x, thispoint.y, -1, 0);
        }

        // Correct problems with some of the edges. (Going 3->5 is fine.)
        /*
        if (face == 3 && thispoint.face == 4 && thispoint.x == 1)
            thispoint.x = 0;

        if (face == 4 && thispoint.face == 3 && thispoint.y == 1)
            thispoint.y = 0;

        if (face == 2 && thispoint.face == 4 && thispoint.y == 1)
            thispoint.y = 0;
        */
        face = thispoint.face;
        x = thispoint.x;
        y = thispoint.y;

        index = face * edge * edge + x * edge + y;

        if (world.sea(face, x, y) == 1)
            keepgoing = 0;

        if (thisdrop[index] == 1)
            keepgoing = 0;

    } while (keepgoing == 1);
}

// This function cleans up the lake map, removing any absurd values.

void cleanlakes(planet& world)
{
    int edge = world.edge();
    int maxelev = world.maxelevation();

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.lakesurface(face, i, j) > maxelev) // These are weird lake seeds that for some reason didn't get turned into lakes.
                    world.setlakesurface(face, i, j, 0);

                if (world.truelake(face, i, j) == 1) // No extra elevation where there are lakes.
                    world.setextraelev(face, i, j, 0);

            }
        }
    }
}

// This adds new precipitation from the lakes.

void lakerain(planet& world, vector<int>& lakewinterrainmap, vector<int>& lakesummerrainmap, vector<fourglobepoints>& dirpoint)
{
    int edge = world.edge();
    int sealevel = world.sealevel();
    int maxelev = world.maxelevation();

    int lakemult = 15; // This is the amount we multiply the wind factor by to get the number of lake tiles that will provide rain. The higher this is, the more rain will extend onto the land.
    float tempfactor = 20.0f; // The amount temperature affects moisture pickup over lakes. The higher it is, the more difference it makes.
    float mintemp = 0.15f; // The minimum fraction that low temperatures can reduce the pickup rate to.
    int dumprate = 5; // The higher this number, the less rain gets deposited, but the further across the land it is distributed.
    int pickuprate = 60; // The higher this number, the more rain gets picked up over lake tiles.
    int swervechance = 3; // The lower this number, the more variation in where the rain lands.
    int splashsize = 1; // The higher this number, the larger area gets splashed around each target tile.
    float slopefactor = 5.0f; // This determines how much gradient affects rainfall. The lower it is, the more it affects it.
    float elevationfactor = 0.002f; // This determines how much elevation affects the degree to which gradient affects rainfall.
    int slopemin = 200; // This is how high land has to be before the gradient affects rainfall.
    float seasonalvar = 0.02f; // Variation in rainfall between seasons.
    int maxrain = 800; // Any rainfall over this will be greatly reduced.
    float capfactor = 0.1f; // Amount to multiply excessive rain by.

    // First the westerly winds.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                int currentwind = 1; // world.wind(face, i, j);

                if (world.winddir(face, i, j)==1) //(currentwind > 0 && currentwind != 99)
                {
                    if (world.map(face, i, j) > sealevel && world.lakesurface(face, i, j) == 0 && world.lakesurface(dirpoint[index].west.face, dirpoint[index].west.x, dirpoint[index].west.y) != 0)// If this is next to a lake
                    {
                        int crount = lakemult; // currentwind* lakemult;
                        int waterlog = 0; // This will hold the amount of water being carried onto this land
                        bool lakeice = 0; // This will be 1 if any seasonal ice is found when picking up moisture.

                        // First we pick up water from the lake to the west.

                        globepoint thispoint;
                        thispoint.face = face;
                        thispoint.x = i;
                        thispoint.y = j;

                        while (crount > 0)
                        {
                            thispoint = dirpoint[thispoint.face * edge * edge + thispoint.x * edge + thispoint.y].west;

                            if (world.lakesurface(thispoint.face,thispoint.x,thispoint.y) != 0 && world.maxtemp(thispoint.face, thispoint.x, thispoint.y) > 0) // It picks up water from unfrozen lake
                            {
                                if (world.mintemp(thispoint.face, thispoint.x, thispoint.y) < 5)
                                    lakeice = 1;

                                float temp = world.avetemp(thispoint.face, thispoint.x, thispoint.y) / tempfactor; // Less water is picked up from colder lakes.

                                if (temp < mintemp)
                                    temp = mintemp;

                                temp = (temp / 100) + 1;

                                waterlog = waterlog + (int)((float)pickuprate * temp);
                            }

                            crount--;
                        }

                        if (lakeice == 1)
                            waterlog = waterlog / 2;

                        crount = 0;

                        // Now we deposit water onto the land to the east.

                        thispoint.face = face;
                        thispoint.x = i;
                        thispoint.y = j;

                        while (crount < currentwind * lakemult)
                        {
                            thispoint = dirpoint[thispoint.face * edge * edge + thispoint.x * edge + thispoint.y].east;

                            if (random(1, swervechance) == 1)
                            {
                                if (random(1, 2) == 1)
                                    thispoint = dirpoint[thispoint.face * edge * edge + thispoint.x * edge + thispoint.y].north;
                                else
                                    thispoint = dirpoint[thispoint.face * edge * edge + thispoint.x * edge + thispoint.y].south;
                            }

                            float waterdumped = (float)waterlog / (float)dumprate;

                            if (world.map(thispoint.face, thispoint.x, thispoint.y) - sealevel > slopemin)
                            {
                                globepoint upwind = dirpoint[thispoint.face * edge * edge + thispoint.x * edge + thispoint.y].west;
                                
                                float slope = (float)getslope(world, upwind.face, upwind.x, upwind.y, thispoint.face, thispoint.x, thispoint.y);

                                slope = slope / slopefactor;

                                if (slope > 1.0f) // If it's going uphill
                                {
                                    waterdumped = waterdumped * slope;

                                    float elevation = (float(world.map(thispoint.face, thispoint.x, thispoint.y) - sealevel));

                                    waterdumped = waterdumped * elevationfactor * elevation;

                                    if (waterdumped > (float)waterlog)
                                        waterdumped = (float)waterlog;
                                }
                            }

                            waterlog = waterlog - (int)waterdumped;

                            if (world.map(thispoint.face, thispoint.x, thispoint.y) > sealevel && world.wind(thispoint.face, thispoint.x, thispoint.y) < 50 && world.lakesurface(thispoint.face, thispoint.x, thispoint.y) == 0)
                            {
                                int thisindex = thispoint.face * edge * edge + thispoint.x * edge + thispoint.y;

                                for (int k = -splashsize; k <= splashsize; k++)
                                {
                                    for (int l = -splashsize; l <= splashsize; l++)
                                    {
                                        globepoint lpoint = getglobepoint(edge, thispoint.face, thispoint.x, thispoint.y, k, l);

                                        if (lpoint.face != -1)
                                        {
                                            int lindex = lpoint.face * edge * edge + lpoint.x * edge + lpoint.y;
                                            if (lakesummerrainmap[lindex] < (int)(waterdumped / 2.0f))
                                                lakesummerrainmap[lindex] = (int)(waterdumped / 2.0f);

                                            if (lakewinterrainmap[lindex] < (int)(waterdumped / 2.0f))
                                                lakewinterrainmap[lindex] = (int)(waterdumped / 2.0f);

                                        }
                                    }
                                }

                                if (lakesummerrainmap[thisindex] < (int)waterdumped)
                                    lakesummerrainmap[thisindex] = (int)waterdumped;

                                if (lakewinterrainmap[thisindex] < (int)waterdumped)
                                    lakesummerrainmap[thisindex] = (int)waterdumped;
                            }

                            crount++;

                            if (waterlog < 50)
                                crount = currentwind * lakemult;
                        }

                    }
                }
            }
        }
    }

    // Now the easterly winds.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int currentwind = 1; // world.wind(face, i, j);

                if (world.winddir(face,i,j) == -1) //(currentwind < 0 || currentwind == 99)
                {
                    int index = vi + j;

                    if (world.map(face, i, j) > sealevel && world.lakesurface(face, i, j) == 0 && world.lakesurface(dirpoint[index].west.face, dirpoint[index].west.x, dirpoint[index].west.y) != 0)// If this is next to a lake
                    {
                        int crount = lakemult; // currentwind* lakemult;
                        int waterlog = 0; // This will hold the amount of water being carried onto this land
                        bool lakeice = 0; // This will be 1 if any seasonal ice is found when picking up moisture.

                        // First we pick up water from the lake to the east.

                        globepoint thispoint;
                        thispoint.face = face;
                        thispoint.x = i;
                        thispoint.y = j;

                        while (crount > 0)
                        {
                            thispoint = dirpoint[thispoint.face * edge * edge + thispoint.x * edge + thispoint.y].east;

                            if (world.lakesurface(thispoint.face, thispoint.x, thispoint.y) != 0 && world.maxtemp(thispoint.face, thispoint.x, thispoint.y) > 0) // It picks up water from unfrozen lake
                            {
                                if (world.mintemp(thispoint.face, thispoint.x, thispoint.y) < 5)
                                    lakeice = 1;

                                float temp = world.avetemp(thispoint.face, thispoint.x, thispoint.y) / tempfactor; // Less water is picked up from colder lakes.

                                if (temp < mintemp)
                                    temp = mintemp;

                                temp = (temp / 100) + 1;

                                waterlog = waterlog + (int)((float)pickuprate * temp);
                            }

                            crount--;
                        }

                        if (lakeice == 1)
                            waterlog = waterlog / 2;

                        crount = 0;

                        // Now we deposit water onto the land to the west.

                        thispoint.face = face;
                        thispoint.x = i;
                        thispoint.y = j;

                        while (crount < currentwind * lakemult)
                        {
                            thispoint = dirpoint[thispoint.face * edge * edge + thispoint.x * edge + thispoint.y].west;

                            if (random(1, swervechance) == 1)
                            {
                                if (random(1, 2) == 1)
                                    thispoint = dirpoint[thispoint.face * edge * edge + thispoint.x * edge + thispoint.y].north;
                                else
                                    thispoint = dirpoint[thispoint.face * edge * edge + thispoint.x * edge + thispoint.y].south;
                            }

                            float waterdumped = (float)waterlog / (float)dumprate;

                            if (world.map(thispoint.face, thispoint.x, thispoint.y) - sealevel > slopemin)
                            {
                                globepoint upwind = dirpoint[thispoint.face * edge * edge + thispoint.x * edge + thispoint.y].east;

                                float slope = (float)getslope(world, upwind.face, upwind.x, upwind.y, thispoint.face, thispoint.x, thispoint.y);

                                slope = slope / slopefactor;

                                if (slope > 1.0f) // If it's going uphill
                                {
                                    waterdumped = waterdumped * slope;

                                    float elevation = (float(world.map(thispoint.face, thispoint.x, thispoint.y) - sealevel));

                                    waterdumped = waterdumped * elevationfactor * elevation;

                                    if (waterdumped > (float)waterlog)
                                        waterdumped = (float)waterlog;
                                }
                            }

                            waterlog = waterlog - (int)waterdumped;

                            if (world.map(thispoint.face, thispoint.x, thispoint.y) > sealevel && world.wind(thispoint.face, thispoint.x, thispoint.y) < 50 && world.lakesurface(thispoint.face, thispoint.x, thispoint.y) == 0)
                            {
                                int thisindex = thispoint.face * edge * edge + thispoint.x * edge + thispoint.y;

                                for (int k = -splashsize; k <= splashsize; k++)
                                {
                                    for (int l = -splashsize; l <= splashsize; l++)
                                    {
                                        globepoint lpoint = getglobepoint(edge, thispoint.face, thispoint.x, thispoint.y, k, l);

                                        if (lpoint.face != -1)
                                        {
                                            int lindex = lpoint.face * edge * edge + lpoint.x * edge + lpoint.y;

                                            if (lakesummerrainmap[lindex] < (int)(waterdumped / 2.0f))
                                                lakesummerrainmap[lindex] = (int)(waterdumped / 2.0f);

                                            if (lakewinterrainmap[lindex] < (int)(waterdumped / 2.0f))
                                                lakewinterrainmap[lindex] = (int)(waterdumped / 2.0f);

                                        }
                                    }
                                }

                                if (lakesummerrainmap[thisindex] < (int)waterdumped)
                                    lakesummerrainmap[thisindex] = (int)waterdumped;

                                if (lakewinterrainmap[thisindex] < (int)waterdumped)
                                    lakesummerrainmap[thisindex] = (int)waterdumped;
                            }

                            crount++;

                            if (waterlog < 50)
                                crount = currentwind * lakemult;
                        }

                    }
                }
            }
        }
    }

    // Now adjust for seasonal variation on land.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                if (world.map(face, i, j) > sealevel)
                {
                    int index = vi + j;

                    float tempdiff = (float)(world.maxtemp(face, i, j) - world.mintemp(face, i, j));

                    if (tempdiff < 0.0f)
                        tempdiff = 0.0f;

                    float winterrain = (float)lakewinterrainmap[index];

                    tempdiff = tempdiff * seasonalvar * winterrain;

                    lakewinterrainmap[index] = lakewinterrainmap[index] + (int)tempdiff;
                    lakesummerrainmap[index] = lakesummerrainmap[index] - (int)tempdiff;

                    if (lakesummerrainmap[index] < 0)
                        lakesummerrainmap[index] = 0;
                }
            }
        }
    }

    // Now cap excessive rainfall.

    float rain[2];

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                rain[0] = (float)lakewinterrainmap[index];
                rain[1] = (float)lakesummerrainmap[index];

                for (int n = 0; n <= 1; n++)
                {
                    if (rain[n] > (float)maxrain)
                        rain[n] = ((rain[n] - (float)maxrain) * capfactor) + (float)maxrain;

                    if (rain[n] < 0.0f)
                        rain[n] = 0.0f;
                }

                lakewinterrainmap[index] = (int)rain[0];
                lakesummerrainmap[index] = (int)rain[1];
            }
        }
    }

    // Now blur the lake rain.

    smooth(lakewinterrainmap, edge, maxelev, 1);
    smooth(lakesummerrainmap, edge, maxelev, 1);

    /*
    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (lakewinterrainmap[face][i][j] != 0 || lakesummerrainmap[face][i][j] != 0)
                    world.settest(face, i, j, 100);
            }
        }
    }
    */

    // Now add the lake rain to the existing rain maps.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                world.setwinterrain(face, i, j, world.winterrain(face, i, j) + lakewinterrainmap[index]);
                world.setsummerrain(face, i, j, world.summerrain(face, i, j) + lakesummerrainmap[index]);
            }
        }
    }
}

// This adds new rivers from the lake-related rainfall.

void lakerivers(planet& world, vector<int>& lakewinterrainmap, vector<int>& lakesummerrainmap)
{
    int edge = world.edge();
    int sealevel = world.sealevel();

    int minimum = 40; // We won't bother with tiles with lower average flow than this.
    int maxrepeat = 2; // If a river goes in the same direction for this long, it will try to change course.

    int neighbours[8][2];

    neighbours[0][0] = 0;
    neighbours[0][1] = -1;

    neighbours[1][0] = 1;
    neighbours[1][1] = -1;

    neighbours[2][0] = 1;
    neighbours[2][1] = 0;

    neighbours[3][0] = 1;
    neighbours[3][1] = 1;

    neighbours[4][0] = 0;
    neighbours[4][1] = 1;

    neighbours[5][0] = -1;
    neighbours[5][1] = 1;

    neighbours[6][0] = -1;
    neighbours[6][1] = -0;

    neighbours[7][0] = -1;
    neighbours[7][1] = -1;

    // Now, we go through the map tile by tile. Take the lake rainfall in each tile and add it to every downstream tile.

    vector<int> thisdrop(6 * edge * edge, 0);

    int dropno = 1;

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.map(face, i, j) > sealevel)
                {
                    tracelakedrop(world, face, i, j, minimum, dropno, thisdrop, maxrepeat, neighbours, lakesummerrainmap, lakewinterrainmap);
                    dropno++;
                }
            }
        }
    }
}

// This function traces a drop downhill to the sea, depositing water as it goes.

void tracelakedrop(planet& world, int face, int x, int y, int minimum, int dropno, vector<int>& thisdrop, int maxrepeat, int neighbours[8][2], vector<int>& lakewinterrainmap, vector<int>& lakesummerrainmap)
{
    globepoint newseatile;
    globepoint dest;

    int edge = world.edge();
    float riverfactor = world.riverfactor();

    int index = face * edge * edge + x * edge + y;

    float janload = (float)lakewinterrainmap[index];
    float julload = (float)lakesummerrainmap[index];

    if (face == 5 || (face != 4 && y > edge / 2))
    {
        janload = (float)lakesummerrainmap[index];
        julload = (float)lakewinterrainmap[index];
    }

    if ((janload + julload) / 2.0f < (float)minimum)
        return;

    janload = janload / riverfactor;
    julload = julload / riverfactor;

    bool keepgoing = 1;

    // This bit moves some water from the winter load to the summer. This is because winter precipitation feeds into the river more slowly than summer precipitation does.

    float amount = 10.0f;

    if (world.mintemp(face, x, y) < 0) // In cold areas, more winter water gets held over until summer because it's snow.
        amount = 3.0f;

    if (face == 4 || (face != 5 && y < edge / 2)) // Northern hemisphere
    {
        float diff = janload / amount;

        janload = janload - diff;
        julload = julload + diff;
    }
    else // Southern hemisphere
    {
        float diff = julload / amount;

        julload = julload - diff;
        janload = janload + diff;
    }

    if (janload < 0.0f)
        janload = 0.0f;

    if (julload < 0.0f)
        julload = 0.0f;

    // Now we just trace the drop through the map, increasing the water flow wherever it goes.

    int dir = -1;
    int repeated = 0;

    do
    {
        thisdrop[index] = dropno;

        world.setriverjan(face, x, y, world.riverjan(face, x, y) + (int)janload);
        world.setriverjul(face, x, y, world.riverjul(face, x, y) + (int)julload);

        dir = world.riverdir(face, x, y);

        globepoint thispoint;
        thispoint.face = face;
        thispoint.x = x;
        thispoint.y = y;

        if (dir == 8 || dir == 1 || dir == 2)
            thispoint = getglobepoint(edge, thispoint.face, thispoint.x, thispoint.y, 0, -1);

        if (dir == 4 || dir == 5 || dir == 6)
            thispoint = getglobepoint(edge, thispoint.face, thispoint.x, thispoint.y, 0, 1);

        if (dir == 2 || dir == 3 || dir == 4)
        {
            if (thispoint.face != -1)
                thispoint = getglobepoint(edge, thispoint.face, thispoint.x, thispoint.y, 1, 0);
        }

        if (dir == 6 || dir == 7 || dir == 8)
        {
            if (thispoint.face != -1)
                thispoint = getglobepoint(edge, thispoint.face, thispoint.x, thispoint.y, -1, 0);
        }

        // Correct problems with some of the edges. (Going 3->5 is fine.)
        /*
        if (face == 3 && thispoint.face == 4 && thispoint.x == 1)
            thispoint.x = 0;

        if (face == 4 && thispoint.face == 3 && thispoint.y == 1)
            thispoint.y = 0;

        if (face == 2 && thispoint.face == 4 && thispoint.y == 1)
            thispoint.y = 0;
        */

        face = thispoint.face;
        x = thispoint.x;
        y = thispoint.y;

        if (face == -1)
            keepgoing = 0;
        else
        {
            if (world.sea(face, x, y) == 1) // We'll continue the river into the sea for a couple of tiles, to help with the regional map generation later.
            {
                for (int n = 1; n <= 3; n++)
                {
                    world.setriverjan(face, x, y, world.riverjan(face, x, y) + (int)janload);
                    world.setriverjul(face, x, y, world.riverjul(face, x, y) + (int)julload);

                    newseatile = findseatile(world, face, x, y, dir);

                    dir = getdir(edge, face, x, y, newseatile.face, newseatile.x, newseatile.y);

                    world.setriverdir(face, x, y, dir);

                    face = newseatile.face;
                    x = newseatile.x;
                    y = newseatile.y;

                    index = face * edge * edge + x * edge + y;
                }

                return;
            }

            if (world.truelake(face, x, y) == 1) // These rivers will stop at lakes.
                keepgoing = 0;

            if (world.riftlakesurface(face, x, y) != 0)
                keepgoing = 0;

            if (thisdrop[index] == dropno)
                keepgoing = 0;
        }

    } while (keepgoing == 1);
}

// This function checks to make sure that river flows don't decrease.

void checkglobalflows(planet& world)
{
    int edge = world.edge();

    int found;

    do
    {
        found = 0;

        for (int face = 0; face < 6; face++)
        {
            for (int i = 0; i < edge; i++)
            {
                for (int j = 0; j < edge; j++)
                {
                    if (world.sea(face, i, j) == 0)
                    {
                        if (world.riverjan(face, i, j) != 0 || world.riverjul(face, i, j) != 0)
                            found = checkthisflow(world, face, i, j, found);
                    }
                }
            }
        }
    } while (found != 0);
}

// This checks an individual cell to check that the flow isn't decreasing.

int checkthisflow(planet& world, int face, int x, int y, int found)
{
    globepoint destpoint;

    destpoint = getflowdestination(world, face, x, y, 0);

    if (destpoint.face == -1)
        return (found);

    int def = destpoint.face;
    int dex = destpoint.x;
    int dey = destpoint.y;

    int jan = world.riverjan(face, x, y);
    int jul = world.riverjul(face, x, y);
    int jand = world.riverjan(def, dex, dey);
    int juld = world.riverjul(def, dex, dey);

    if (jand < jan || juld < jul)
    {
        found++;

        int janadd = jan - jand;
        int juladd = jul - juld;

        if (janadd > 0)
            world.setriverjan(def, dex, dey, jand + janadd);

        if (juladd > 0)
            world.setriverjul(def, dex, dey, juld + juladd);

        found = checkthisflow(world, def, dex, dey, found);
    }

    return (found);
}

// This creates the rift lake map.

void createriftlakemap(planet& world, vector<int>& nolake)
{
    int edge = world.edge();
    
    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();

    int riftlakechance = random(500, 4000); // The higher this is, the fewer rift lakes there will be.
    int minlake = 500; // Flows of this size or higher might have lakes on them.
    int maxlake = 2000; // Flows larger than this can't have lakes.
    int minlakelength = 4;
    int maxlakelength = 10;

    int leftmargin = edge / 8; // No rift lakes closer to the edge of each face than this, for simplicity's sake.
    int rightmargin = edge - leftmargin;

    twointegers nextpoint;

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = leftmargin; i <= rightmargin; i++)
        {
            int vi = vface + i * edge;

            for (int j = leftmargin; j <= rightmargin; j++)
            {
                if (world.nom(face, i, j) >= sealevel && world.lakesurface(face, i, j) == 0 && nolake[vi + j] == 0 && world.riftlakesurface(face, i, j) == 0)
                {
                    if (world.riveraveflow(face, i, j) > minlake && world.riveraveflow(face, i, j) < maxlake && random(1, riftlakechance) == 1)
                    {
                        // Check that no rift lakes currently exist downstream of here.

                        bool keepgoing = 1;
                        bool found = 0;

                        globepoint thispoint;

                        thispoint.face = face;
                        thispoint.x = i;
                        thispoint.y = j;

                        int crount = 0;

                        do
                        {
                            crount++;

                            int dir = world.riverdir(thispoint.face, thispoint.x, thispoint.y);

                            if (dir == 8 || dir == 1 || dir == 2)
                                thispoint = getglobepoint(edge, thispoint.face, thispoint.x, thispoint.y, 0, -1);

                            if (dir == 4 || dir == 5 || dir == 6)
                                thispoint = getglobepoint(edge, thispoint.face, thispoint.x, thispoint.y, 0, 1);

                            if (dir == 2 || dir == 3 || dir == 4)
                            {
                                if (thispoint.face != -1)
                                    thispoint = getglobepoint(edge, thispoint.face, thispoint.x, thispoint.y, 1, 0);
                            }

                            if (dir == 6 || dir == 7 || dir == 8)
                            {
                                if (thispoint.face != -1)
                                    thispoint = getglobepoint(edge, thispoint.face, thispoint.x, thispoint.y, -1, 0);
                            }

                            // Correct problems with some of the edges. (Going 3->5 is fine.)
                            /*
                            if (face == 3 && thispoint.face == 4 && thispoint.x == 1)
                                thispoint.x = 0;

                            if (face == 4 && thispoint.face == 3 && thispoint.y == 1)
                                thispoint.y = 0;

                            if (face == 2 && thispoint.face == 4 && thispoint.y == 1)
                                thispoint.y = 0;
                            */

                            if (dir == 0 || world.sea(thispoint.face, thispoint.x, thispoint.y) == 1)
                                keepgoing = 0;

                            if (world.riftlakesurface(thispoint.face, thispoint.x, thispoint.y) != 0 || world.lakesurface(thispoint.face, thispoint.x, thispoint.y) != 0)
                            {
                                found = 1;
                                keepgoing = 0;
                            }

                            if (crount > 100)
                                keepgoing = 0;

                        } while (keepgoing == 1);

                        if (found == 0)
                        {
                            int x = i;
                            int y = j;
                            keepgoing = 1;

                            do
                            {
                                int lakelength = random(minlakelength, maxlakelength);

                                nextpoint = createriftlake(world, face, x, y, lakelength, nolake); // Make a lake.

                                x = nextpoint.x;
                                y = nextpoint.y;

                                if (x == -1)
                                    keepgoing = 0;
                                else
                                {
                                    int gaplength = random(2, 10);

                                    for (int n = 1; n <= gaplength; n++) // Move down the river a bit.
                                    {
                                        if (x >= leftmargin && x <= rightmargin && y >= leftmargin && y <= rightmargin)
                                        {
                                            int dir = world.riverdir(face, x, y);

                                            if (dir == 8 || dir == 1 || dir == 2)
                                                y--;

                                            if (dir == 4 || dir == 5 || dir == 6)
                                                y++;

                                            if (dir == 2 || dir == 3 || dir == 4)
                                                x++;

                                            if (dir == 6 || dir == 7 || dir == 8)
                                                x--;

                                            if (x<leftmargin || x>rightmargin || y<leftmargin || y>rightmargin)
                                            {
                                                keepgoing = 0;
                                                n = gaplength;
                                            }
                                            else
                                            {
                                                if (world.lakesurface(face, x, y) != 0)
                                                {
                                                    keepgoing = 0;
                                                    n = gaplength;
                                                }

                                                if (world.nom(face, x, y) <= sealevel)
                                                {
                                                    keepgoing = 0;
                                                    n = gaplength;
                                                }
                                            }
                                        }
                                        else
                                        {
                                            keepgoing = 0;
                                            n = gaplength;
                                        }
                                    }

                                    if (random(1, 4) == 1)
                                        keepgoing = 0;
                                }

                            } while (keepgoing == 1);
                        }
                    }
                }
            }
        }
    }
}

// This function puts a particular rift lake onto the rift lake map.

twointegers createriftlake(planet& world, int face, int startx, int starty, int lakelength, vector<int>& nolake)
{
    int edge = world.edge();
    int maxelev = world.maxelevation();
    int riverlandreduce = world.riverlandreduce();

    int maxdepth = 1800; // Maximum depth
    int mindepth = 100; // Minimum depth

    int leftmargin = edge / 8; // No rift lakes closer to the edge of each face than this, for simplicity's sake.
    int rightmargin = edge - leftmargin;

    vector<int> removedrivers(6 * edge * edge, 0);
    vector<int> currentriver(6 * edge * edge, 0);

    // First we need to work out the surface level of this lake. It will be at the level of the river flowing out of it.

    int vface = face * edge * edge;

    int x = startx;
    int y = starty;

    for (int n = 1; n <= lakelength; n++)
    {
        int dir = world.riverdir(face, x, y);

        if (dir == 8 || dir == 1 || dir == 2)
            y--;

        if (dir == 4 || dir == 5 || dir == 6)
            y++;

        if (dir == 2 || dir == 3 || dir == 4)
            x++;

        if (dir == 6 || dir == 7 || dir == 8)
            x--;

        if (x<leftmargin || x>rightmargin || y<leftmargin || y>rightmargin)
            n = lakelength;
    }

    int surfacelevel = world.nom(face, x, y) - riverlandreduce;

    world.setlakestart(face, startx, starty, 1); // This marks this as the beginning of a rift lake, which we'll need to know when it comes to the regional map.

    markriver(world, face, startx, starty, currentriver, 0);

    twointegers nextpoint;

    int depth = random(mindepth, maxdepth);

    x = startx;
    y = starty;
    int done = 0;

    for (int n = 1; n <= lakelength; n++)
    {
        bool keepgoing = 1;

        if (nolake[vface + x * edge + y] != 0)
        {
            keepgoing = 0;
            n = lakelength;
        }

        for (int i = x - 2; i <= x + 2; i++)
        {
            if (i<leftmargin || i>rightmargin)
                keepgoing = 0;
            else
            {
                for (int j = y - 2; j <= y + 2; j++)
                {
                    if (j<leftmargin || j>rightmargin)
                        keepgoing = 0;
                    else
                    {
                        if (world.lakesurface(face, i, j) != 0)
                        {
                            keepgoing = 0;
                            i = x + 2;
                            j = y + 2;
                            n = lakelength;
                        }

                        if (world.riftlakesurface(face, i, j) != 0 && world.riftlakesurface(face, i, j) != surfacelevel)
                        {
                            keepgoing = 0;
                            i = x + 2;
                            j = y + 2;
                            n = lakelength;
                        }
                    }
                }
            }
        }

        if (keepgoing == 0)
        {
            if (done < 3) // If we weren't able to make this lake long enough, get rid of it again.
            {
                x = startx;
                y = starty;

                for (int n = 1; n <= done; n++)
                {
                    world.setriftlakesurface(face, x, y, 0);
                    world.setriftlakebed(face, x, y, 0);

                    int dir = world.riverdir(face, x, y);

                    if (dir == 8 || dir == 1 || dir == 2)
                        y--;

                    if (dir == 4 || dir == 5 || dir == 6)
                        y++;

                    if (dir == 2 || dir == 3 || dir == 4)
                        x++;

                    if (dir == 6 || dir == 7 || dir == 8)
                        x--;

                    if (x<leftmargin || x>rightmargin || y<leftmargin || y>rightmargin)
                        n = done;
                }
            }

            nextpoint.x = -1;
            nextpoint.y = -1;
            return (nextpoint);
        }

        world.setriftlakesurface(face, x, y, surfacelevel);

        done++;

        int thislevel = surfacelevel - depth + randomsign(random(0, 200));

        if (thislevel > surfacelevel - 50)
            thislevel = surfacelevel - 50;

        if (thislevel < 1)
            thislevel = 1;

        world.setriftlakebed(face, x, y, thislevel);

        int dir = world.riverdir(face, x, y);

        if (dir == 8 || dir == 1 || dir == 2)
            y--;

        if (dir == 4 || dir == 5 || dir == 6)
            y++;

        if (dir == 2 || dir == 3 || dir == 4)
            x++;

        if (dir == 6 || dir == 7 || dir == 8)
            x--;

        if (x<leftmargin || x>rightmargin || y<leftmargin || y>rightmargin)
            n = lakelength;
    }

    if (done < 3) // If we weren't able to make this lake long enough, get rid of it again.
    {
        x = startx;
        y = starty;

        for (int n = 1; n <= done; n++)
        {
            world.setriftlakesurface(face, x, y, 0);
            world.setriftlakebed(face, x, y, 0);

            int dir = world.riverdir(face, x, y);

            if (dir == 8 || dir == 1 || dir == 2)
                y--;

            if (dir == 4 || dir == 5 || dir == 6)
                y++;

            if (dir == 2 || dir == 3 || dir == 4)
                x++;

            if (dir == 6 || dir == 7 || dir == 8)
                x--;

            if (x<leftmargin || x>rightmargin || y<leftmargin || y>rightmargin)
                n = done;
        }
        nextpoint.x = x;
        nextpoint.y = y;

        return nextpoint;
    }

    // Now we need to make sure that all rivers bordering this rift lake flow into it.

    x = startx;
    y = starty;

    int riverno = 1;

    twointegers dest;

    for (int n = 1; n <= lakelength; n++)
    {
        for (int i = x - 1; i <= x + 1; i++)
        {
            int vi = vface + i * edge;

            for (int j = y - 1; j <= y + 1; j++)
            {
                if (world.riftlakesurface(face, i, j) == 0) // If this is not in the rift lake
                {
                    if (currentriver[vi + j] == 0)
                    {
                        int lowest = maxelev;

                        int newdestx = -1;
                        int newdesty = -1;

                        for (int k = i - 1; k <= i + 1; k++) // Find the deepest neighbouring rift lake tile
                        {
                            for (int l = j - 1; l <= j + 1; l++)
                            {
                                if (world.riftlakesurface(face, k, l) == surfacelevel)
                                {
                                    if (world.riftlakesurface(face, k, l) < lowest)
                                    {
                                        lowest = world.riftlakesurface(face, k, l);

                                        newdestx = k;
                                        newdesty = l;
                                    }
                                }
                            }
                        }

                        if (newdestx != -1) // Now divert the flow towards that tile
                        {
                            divertriver(world, face, i, j, face, newdestx, newdesty, removedrivers, riverno);

                            riverno++;
                        }
                    }
                }
            }
        }

        int dir = world.riverdir(face, x, y);

        if (dir == 8 || dir == 1 || dir == 2)
            y--;

        if (dir == 4 || dir == 5 || dir == 6)
            y++;

        if (dir == 2 || dir == 3 || dir == 4)
            x++;

        if (dir == 6 || dir == 7 || dir == 8)
            x--;

        if (x<leftmargin || x>rightmargin || y<leftmargin || y>rightmargin)
            n = lakelength;
    }

    nextpoint.x = x;
    nextpoint.y = y;

    return (nextpoint);
}

// This function puts ergs in deserts.

void createergs(planet& world, boolshapetemplate smalllake[], boolshapetemplate largelake[], boolshapetemplate shape[])
{
    int edge = world.edge();
    int worldsize = world.size();

    vector<short> ergprobability(6 * edge * edge, 0);
    vector<short> overlapprobability(6 * edge * edge, 0);

    int clusternumber = random(2, 50); // 50; Number of possible clusters of erg on the map.

    if (worldsize == 1)
        clusternumber = clusternumber * 4;

    if (worldsize == 2)
        clusternumber = clusternumber * 16;


    int minrad = 10;
    int maxrad = 40; // Possible sizes of the clusters

    for (int n = 0; n < clusternumber; n++) // Draw some shapes on this array, to allow for erg clusters.
    {
        int shapenumber = random(1, 11);

        int probability = random(20, 200);

        int overlap = random(1, 100);

        int imheight = shape[shapenumber].ysize() - 1;
        int imwidth = shape[shapenumber].xsize() - 1;

        int face = random(0, 5);
        int centrex = random(imwidth / 2 + 2, edge - imwidth / 2 + 2);
        int centrey = random(imwidth / 2 + 2, edge - imwidth / 2 + 2);

        int x = centrex - imwidth / 2; // Coordinates of the top left corner.
        int y = centrey - imheight / 2;

        int leftr = random(0, 1); // If it's 1 then we reverse it left-right
        int downr = random(0, 1); // If it's 1 then we reverse it top-bottom

        int istart = 0, desti = imwidth + 1, istep = 1;
        int jstart = 0, destj = imheight + 1, jstep = 1;

        if (leftr == 1)
        {
            istart = imwidth;
            desti = -1;
            istep = -1;
        }

        if (downr == 1)
        {
            jstart = imwidth;
            destj = -1;
            jstep = -1;
        }

        int imap = -1;
        int jmap = -1;

        for (int i = istart; i != desti; i = i + istep)
        {
            imap++;
            jmap = -1;

            for (int j = jstart; j != destj; j = j + jstep)
            {
                jmap++;

                if (shape[shapenumber].point(i, j) == 1)
                {
                    globepoint xpoint = getglobepoint(edge, face, x, y, imap, 0);

                    if (xpoint.face != -1)
                    {
                        globepoint ypoint = getglobepoint(edge, xpoint.face, xpoint.x, xpoint.y, 0, jmap);

                        if (ypoint.face != -1)
                        {
                            int yindex = ypoint.face * edge * edge + ypoint.x * edge + ypoint.y;

                            ergprobability[yindex] = probability;
                            overlapprobability[yindex] = overlap;
                        }
                    }
                }
            }
        }
    }

    int mindist = 2; // Minimum distance to existing lakes etc.

    int lakeno = 0;

    vector<int> thislake(6 * edge * edge, 0);

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                if (world.climate(face, i, j) == 5 && ergprobability[index] != 0) // If this is a hot desert
                {
                    if (random(1, ergprobability[index]) == 1)
                    {
                        bool goahead = 1;

                        for (int k = - mindist; k <= mindist; k++) // Check that there aren't any lakes too close.
                        {
                            for (int l = -mindist; l <= mindist; l++)
                            {
                                globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                                if (lpoint.face != -1)
                                {
                                    if (world.truelake(lpoint.face, lpoint.x, lpoint.y) != 0)
                                        goahead = 0;
                                }
                            }
                        }

                        if (goahead == 1)
                        {
                            bool canoverlap = 0;

                            if (random(1, 100) < overlapprobability[index])
                                canoverlap = 1;

                            int shapenumber;

                            if (random(1, 4) == 1)
                            {
                                shapenumber = random(0, 11);
                                drawspeciallake(world, shapenumber, face, i, j, lakeno, thislake, smalllake, canoverlap, 120); // 120 is the code for ergs.

                            }
                            else
                            {
                                if (random(1, 2) == 1)
                                    shapenumber = random(0, 2);
                                else
                                    shapenumber = random(0, 9);
                                drawspeciallake(world, shapenumber, face, i, j, lakeno, thislake, largelake, canoverlap, 120); // 120 is the code for ergs.
                            }

                            lakeno++;
                        }
                    }
                }
            }
        }
    }

    // Some ergs may have overlapped, so now we need to make sure their elevation remains constant.

    checkergelevation(world);
}

// This ensures that the elevation of each erg remains constant.

void checkergelevation(planet& world)
{
    int edge = world.edge();
    int sealevel = world.sealevel();

    vector<bool> checked(6 * edge * edge);

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                if (world.special(face, i, j) == 120 && checked[index] == 0)
                {
                    vector<bool> thiserg(6 * edge * edge, 0);

                    // First, find the lowest elevation within this erg.

                    int lowestelev = lowestergelevation(world, face, i, j, thiserg);

                    if (lowestelev < 2)
                        lowestelev = 2;

                    if (lowestelev <= sealevel)
                        lowestelev = sealevel + 1;

                    // Now apply that to the whole erg.

                    for (int fface = 0; fface < 6; fface++)
                    {
                        int vfface = fface * edge * edge;

                        for (int k = 0; k < edge; k++)
                        {
                            int vk = vfface + k * edge;

                            for (int l = 0; l < edge; l++)
                            {
                                int lindex = vk + l;

                                if (thiserg[lindex] == 1)
                                {
                                    checked[lindex] = 1;
                                    world.setlakesurface(fface, k, l, lowestelev);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// This finds the lowest elevation in an erg.

int lowestergelevation(planet& world, int face, int startx, int starty, vector<bool>& thiserg)
{
    int edge = world.edge();

    int lowest = 1000000;

    int row[] = { -1,0,0,1 };
    int col[] = { 0,-1,1,0 };

    globepoint node;

    node.face = face;
    node.x = startx;
    node.y = starty;

    queue<globepoint> q; // Create a queue
    q.push(node); // Put our starting node into the queue

    while (q.empty() != 1) // Keep going while there's anything left in the queue
    {
        node = q.front(); // Take the node at the front of the queue
        q.pop(); // And then pop it out of the queue

        if (node.face != -1 && node.x >= 0 && node.x < edge && node.y >= 0 && node.y < edge && world.special(node.face, node.x, node.y) == 120)
        {
            int thiselev = world.nom(node.face, node.x, node.y);

            if (thiselev < lowest)
                lowest = thiselev;

            thiserg[node.face * edge * edge + node.x * edge + node.y] = 1;

            for (int k = 0; k < 4; k++) // Look at the four neighbouring nodes in turn
            {
                globepoint nextnode = getglobepoint(edge, node.face, node.x, node.y, row[k], col[k]);

                if (nextnode.face != -1)
                {
                    int nindex = nextnode.face * edge * edge + nextnode.x * edge + nextnode.y;
                    if (world.special(nextnode.face, nextnode.x, nextnode.y) == 120 && thiserg[nindex] == 0) // If this node is erg
                    {
                        thiserg[nindex] = 1;
                        q.push(nextnode); // Put that node onto the queue
                    }
                }
            }
        }
    }

    return lowest;
}

// This function puts salt pans in deserts.

void createsaltpans(planet& world, boolshapetemplate smalllake[], boolshapetemplate largelake[])
{
    int edge = world.edge();

    int saltchance = 1000; //250; // Probability of salt pans in any given desert tile.
    int mindist = 2; // Minimum distance to existing lakes etc.

    int lakeno = 0;

    vector<int> thislake(6 * edge * edge, 0);

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.climate(face, i, j) == 5) // If this is a hot desert
                {
                    if (random(1, saltchance) == 1)
                    {
                        bool goahead = 1;

                        for (int k = i - mindist; k <= i + mindist; k++) // Check that there aren't any other lakes too close.
                        {
                            if (k >= 0 || k < edge)
                            {
                                for (int l = j - mindist; l <= j + mindist; l++)
                                {
                                    if (l >= 0 && l < edge)
                                    {
                                        if (world.lakesurface(face, k, l) != 0)
                                            goahead = 0;
                                    }
                                    else
                                        goahead = 0;
                                }
                            }
                            else
                                goahead = 0;
                        }

                        if (goahead == 1)
                        {
                            int shapenumber;

                            if (random(1, 4) != 1)
                            {
                                shapenumber = random(0, 3);
                                drawspeciallake(world, shapenumber, face, i, j, lakeno, thislake, smalllake, 0, 110); // 110 is the code for salt pans.
                            }
                            else
                            {
                                shapenumber = random(3, 11);
                                drawspeciallake(world, shapenumber, face, i, j, lakeno, thislake, smalllake, 0, 110); // 110 is the code for salt pans.
                            }

                            lakeno++;
                        }
                    }
                }
            }
        }
    }
}

// This function puts river deltas on the global map.

void createriverdeltas(planet& world)
{
    int edge = world.edge();
    
    // We're going to think of the world as a gigantic rectangle, with the six faces unfolded onto it.

    int wwidth = edge * 4;
    int hheight = edge * 3;

    int width = wwidth - 1;
    int height = hheight - 1;

    // The mask array shows which parts of the gigantic rectangle actually correspond to faces. 1 = actual face, 0 = blank part.

    vector<bool> mask(wwidth * hheight, 0);

    for (int i = 0; i < edge; i++)
    {
        int vi = i * hheight;

        for (int j = 0; j <= height; j++)
            mask[vi + j] = 1;
    }

    for (int i = edge; i <= width; i++)
    {
        int vi = i * hheight;

        for (int j = edge; j < edge * 2; j++)
            mask[vi + j] = 1;
    }

    vector<int> nom(wwidth * hheight, 0); // This one shows land.
    vector<int> tide(wwidth * hheight, 0);
    vector<int> riverjan(wwidth * hheight, 0);
    vector<int> riverjul(wwidth * hheight, 0);
    vector<int> riverdir(wwidth * hheight, 0);
    vector<int> avetemp(wwidth * hheight, 0);
    vector<bool> sea(wwidth * hheight, 0);

    for (int i = 0; i < edge; i++)
    {
        int vi = i * edge;

        for (int j = 0; j < edge; j++)
        {
            int index1 = vi + j;
            int index2 = vi + edge + j;
            int index3 = vi + edge * 2 + j;
            int index4 = (edge + i) * edge + edge + j;
            int index5 = (edge * 2 + i) * edge + edge + j;
            int index6 = (edge * 3 + i) * edge + edge + j;

            nom[index1] = world.nom(4, i, j);
            nom[index2] = world.nom(0, i, j);
            nom[index3] = world.nom(5, i, j);
            nom[index4] = world.nom(1, i, j);
            nom[index5] = world.nom(2, i, j);
            nom[index6] = world.nom(3, i, j);

            tide[index1] = world.tide(4, i, j);
            tide[index2] = world.tide(0, i, j);
            tide[index3] = world.tide(5, i, j);
            tide[index4] = world.tide(1, i, j);
            tide[index5] = world.tide(2, i, j);
            tide[index6] = world.tide(3, i, j);

            riverjan[index1] = world.riverjan(4, i, j);
            riverjan[index2] = world.riverjan(0, i, j);
            riverjan[index3] = world.riverjan(5, i, j);
            riverjan[index4] = world.riverjan(1, i, j);
            riverjan[index5] = world.riverjan(2, i, j);
            riverjan[index6] = world.riverjan(3, i, j);

            riverjul[index1] = world.riverjul(4, i, j);
            riverjul[index2] = world.riverjul(0, i, j);
            riverjul[index3] = world.riverjul(5, i, j);
            riverjul[index4] = world.riverjul(1, i, j);
            riverjul[index5] = world.riverjul(2, i, j);
            riverjul[index6] = world.riverjul(3, i, j);

            riverdir[index1] = world.riverdir(4, i, j);
            riverdir[index2] = world.riverdir(0, i, j);
            riverdir[index3] = world.riverdir(5, i, j);
            riverdir[index4] = world.riverdir(1, i, j);
            riverdir[index5] = world.riverdir(2, i, j);
            riverdir[index6] = world.riverdir(3, i, j);

            avetemp[index1] = world.avetemp(4, i, j);
            avetemp[index2] = world.avetemp(0, i, j);
            avetemp[index3] = world.avetemp(5, i, j);
            avetemp[index4] = world.avetemp(1, i, j);
            avetemp[index5] = world.avetemp(2, i, j);
            avetemp[index6] = world.avetemp(3, i, j);

            sea[index1] = world.sea(4, i, j);
            sea[index2] = world.sea(0, i, j);
            sea[index3] = world.sea(5, i, j);
            sea[index4] = world.sea(1, i, j);
            sea[index5] = world.sea(2, i, j);
            sea[index6] = world.sea(3, i, j);
        }
    }

    vector<int> deltajan(wwidth * hheight, 0);
    vector<int> deltajul(wwidth * hheight, 0);
    vector<int> deltadir(wwidth * hheight, 0);
    vector<int> reducejan(wwidth * hheight, 0);
    vector<int> reducejul(wwidth * hheight, 0);

    int sealevel = world.sealevel();
    int glaciertemp = world.glaciertemp();
    float gravity = world.gravity();

    vector<int> deltarivers(wwidth * hheight, 0); // Marks all the rivers that have been turned into deltas.

    twointegers frompoint, destpoint;

    int margin = 10; // Don't do any closer to the east/west edges of the map than this.
    int deltachance = 200000; //65000; // The lower this is, the more deltas there will be.

    if (gravity > 1.0) // Higher gravity means fewer deltas.
    {
        float fchance = (float)deltachance;
        fchance = fchance * gravity * gravity;

        deltachance = (int)fchance;
    }

    int tidefactor = 8; // The higher this is, the more deltas there will be.
    int range = 10; // Minimum gap between deltas.

    for (int i = margin; i <= width - margin; i++)
    {
        int vi = i * hheight;

        for (int j = 0; j <= height; j++)
        {
            int index = vi + j;

            if ((riverjan[index] != 0 || riverjul[index] != 0) && avetemp[index] > glaciertemp) // There's a river here and it's not too cold
            {
                int flowtotal = riverjan[index] + riverjul[index];

                if (random(1, deltachance) < flowtotal)
                {
                    if (sea[index] == 0)
                    {
                        destpoint.x = i;
                        destpoint.y = j;

                        if (riverdir[index] == 8 || riverdir[index] == 1 || riverdir[index] == 2)
                            destpoint.y--;

                        if (riverdir[index] == 4 || riverdir[index] == 5 || riverdir[index] == 6)
                            destpoint.y++;

                        if (riverdir[index] == 2 || riverdir[index] == 3 || riverdir[index] == 4)
                            destpoint.x++;

                        if (riverdir[index] == 6 || riverdir[index] == 7 || riverdir[index] == 8)
                            destpoint.x--;

                        int dindex = destpoint.x * hheight + destpoint.y;

                        if (destpoint.x >= 0 && destpoint.y <= width && destpoint.y >= 0 && destpoint.y <= height && mask[dindex] == 1)
                        {
                            if (sea[dindex] == 1)
                            {
                                bool found = 0;

                                if (random(1, tidefactor) > tide[index])
                                {
                                    for (int k = i - range; k <= i + range; k++)
                                    {
                                        int kk = k;

                                        if (kk<0 || kk>width)
                                            kk = wrap(kk, width);

                                        int vkk = kk * hheight;

                                        for (int l = j - range; l <= j + range; l++)
                                        {
                                            int vl = vkk + l;

                                            if (l >= 0 && l <= height && mask[vl] == 1)
                                            {
                                                if (deltadir[vl] != 0)
                                                {
                                                    found = 1;
                                                    k = i + range;
                                                    l = j + range;
                                                }
                                            }
                                        }
                                    }
                                    if (found == 0)
                                    {
                                        int x = destpoint.x;
                                        int y = destpoint.y;

                                        int xindex = x * hheight + y;

                                        int upriver = (((riverjan[xindex] + riverjul[xindex]) / 2) / 10000) + randomsign(random(0, 3));

                                        if (upriver < 2)
                                            upriver = 2;

                                        if (upriver > 10)
                                            upriver = 10;

                                        //upriver=upriver*2; // Just to make them bigger to check more easily!

                                        // Upriver is how far upstream from the coast we'll start the delta.

                                        placedelta(world, x, y, upriver, deltarivers, riverjan, riverjul, riverdir, deltajan, deltajul, deltadir, nom, reducejan, reducejul, sea, mask);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Now we flatten all land around the delta branches.

    for (int i = 0; i <= width; i++)
    {
        int vi = i * hheight;

        for (int j = 0; j <= height; j++)
        {
            int index = vi + j;

            if (deltadir[index] > 0)
            {
                for (int k = i - 1; k <= i + 1; k++)
                {
                    int kk = k;

                    if (kk<0 || kk>width)
                        kk = wrap(kk, width);

                    int vk = kk * hheight;

                    for (int l = j - 1; l <= j + 1; l++)
                    {
                        int lindex = vk + l;

                        if (l >= 0 && l <= height && mask[lindex] == 1)
                        {
                            if (sea[lindex] == 0)
                            {
                                nom[lindex] = sealevel + 1;

                                if (deltadir[lindex] == 0)
                                    deltadir[lindex] = -1;
                            }
                        }
                    }
                }
            }
        }
    }

    // Now we lower any land that borders flat land.

    vector<int> donethese(wwidth * hheight, 0);

    for (int i = 0; i <= width; i++)
    {
        int vi = i * hheight;

        for (int j = 0; j <= height; j++)
        {
            int index = vi + j;

            if (nom[index] == sealevel + 1)
            {
                for (int k = i - 1; k <= i + 1; k++)
                {
                    int kk = k;

                    if (kk<0 || kk>width)
                        kk = wrap(kk, width);

                    int vk = kk * hheight;

                    for (int l = j - 1; l <= j + 1; l++)
                    {
                        if (l >= 0 && l <= height)
                        {
                            int lindex = vk + l;

                            if (donethese[lindex] == 0 && nom[lindex] > sealevel + 1)
                            {
                                int elev = nom[lindex] - sealevel;
                                elev = elev / 2;

                                nom[lindex] = sealevel + elev;

                                donethese[lindex] = 1;
                            }
                        }
                    }
                }
            }
        }
    }

    // Now paste everything back onto the cubesphere.

    vector<int> cubereducejan(6 * edge * edge, 0);
    vector<int> cubereducejul(6 * edge * edge, 0);
    vector<int> cubedeltarivers(6 * edge * edge, 0);

    int f1 = edge * edge;
    int f2 = f1 * 2;
    int f3 = f1 * 3;
    int f4 = f1 * 4;
    int f5 = f1 * 5;

    for (int i = 0; i < edge; i++)
    {
        int vi = i * edge;

        for (int j = 0; j < edge; j++)
        {
            int index1 = vi + j;
            int index2 = vi + edge + j;
            int index3 = vi + edge * 2 + j;
            int index4 = (edge + i) * edge + edge + j;
            int index5 = (edge * 2 + i) * edge + edge + j;
            int index6 = (edge * 3 + i) * edge + edge + j;

            world.setnom(4, i, j, nom[index1]);
            world.setnom(0, i, j, nom[index2]);
            world.setnom(5, i, j, nom[index3]);
            world.setnom(1, i, j, nom[index4]);
            world.setnom(2, i, j, nom[index5]);
            world.setnom(3, i, j, nom[index6]);

            world.setriverjan(4, i, j, riverjan[index1]);
            world.setriverjan(0, i, j, riverjan[index2]);
            world.setriverjan(5, i, j, riverjan[index3]);
            world.setriverjan(1, i, j, riverjan[index4]);
            world.setriverjan(2, i, j, riverjan[index5]);
            world.setriverjan(3, i, j, riverjan[index6]);

            world.setriverjul(4, i, j, riverjul[index1]);
            world.setriverjul(0, i, j, riverjul[index2]);
            world.setriverjul(5, i, j, riverjul[index3]);
            world.setriverjul(1, i, j, riverjul[index4]);
            world.setriverjul(2, i, j, riverjul[index5]);
            world.setriverjul(3, i, j, riverjul[index6]);

            world.setriverdir(4, i, j, riverdir[index1]);
            world.setriverdir(0, i, j, riverdir[index2]);
            world.setriverdir(5, i, j, riverdir[index3]);
            world.setriverdir(1, i, j, riverdir[index4]);
            world.setriverdir(2, i, j, riverdir[index5]);
            world.setriverdir(3, i, j, riverdir[index6]);

            world.setdeltajan(4, i, j, deltajan[index1]);
            world.setdeltajan(0, i, j, deltajan[index2]);
            world.setdeltajan(5, i, j, deltajan[index3]);
            world.setdeltajan(1, i, j, deltajan[index4]);
            world.setdeltajan(2, i, j, deltajan[index5]);
            world.setdeltajan(3, i, j, deltajan[index6]);

            world.setdeltajul(4, i, j, deltajul[index1]);
            world.setdeltajul(0, i, j, deltajul[index2]);
            world.setdeltajul(5, i, j, deltajul[index3]);
            world.setdeltajul(1, i, j, deltajul[index4]);
            world.setdeltajul(2, i, j, deltajul[index5]);
            world.setdeltajul(3, i, j, deltajul[index6]);

            world.setdeltadir(4, i, j, deltadir[index1]);
            world.setdeltadir(0, i, j, deltadir[index2]);
            world.setdeltadir(5, i, j, deltadir[index3]);
            world.setdeltadir(1, i, j, deltadir[index4]);
            world.setdeltadir(2, i, j, deltadir[index5]);
            world.setdeltadir(3, i, j, deltadir[index6]);

            cubereducejan[f4 + index1] = reducejan[index1];
            cubereducejan[index1] = reducejan[index2];
            cubereducejan[f5 + index1] = reducejan[index3];
            cubereducejan[f1 + index1] = reducejan[index4];
            cubereducejan[f2 + index1] = reducejan[index5];
            cubereducejan[f3 + index1] = reducejan[index6];

            cubereducejul[f4 + index1] = reducejul[index1];
            cubereducejul[index1] = reducejul[index2];
            cubereducejul[f5 + index1] = reducejul[index3];
            cubereducejul[f1 + index1] = reducejul[index4];
            cubereducejul[f2 + index1] = reducejul[index5];
            cubereducejul[f3 + index1] = reducejul[index6];

            cubedeltarivers[f4 + index1] = deltarivers[index1];
            cubedeltarivers[index1] = deltarivers[index2];
            cubedeltarivers[f5 + index1] = deltarivers[index3];
            cubedeltarivers[f1 + index1] = deltarivers[index4];
            cubedeltarivers[f2 + index1] = deltarivers[index5];
            cubedeltarivers[f3 + index1] = deltarivers[index6];
        }
    }

    // Now we will reduce the rivers that had deltas added.

    vector<int> removedrivers(6 * edge * edge, 0);

    int riverno = 0;

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                if (cubereducejan[index] != 0 || cubereducejul[index] != 0)
                {
                    riverno++;
                    reduceriver(world, cubereducejan[index], cubereducejul[index], removedrivers, riverno, face, i, j);
                }
            }
        }
    }

    // Now we divert other rivers that flow through the delta areas, so they flow more naturally in the delta shape.

    divertdeltarivers(world, cubedeltarivers);
}

// This creates a river delta.

void placedelta(planet& world, int centrex, int centrey, int upriver, vector<int>& deltarivers, vector<int>& riverjan, vector<int>& riverjul, vector<int>& riverdir, vector<int>& deltajan, vector<int>& deltajul, vector<int>& deltadir, vector<int>& nom, vector<int>& reducejan, vector<int>& reducejul, vector<bool>& sea, vector<bool>& mask)
{
    int edge = world.edge();
    int width = edge * 4 - 1;
    int height = edge * 3 - 1;

    int sealevel = world.sealevel();

    int dir = riverdir[centrex * edge + centrey];

    if (dir == 0)
        return;

    const int maxbranches = 40;
    int branchestotal = 0;
    int branches = maxbranches; //20; //random(upriver*2,upriver*4); // Number of branches this delta will (hopefully) have.
    int size = upriver; // Possible distance from the centre that the branches can end.
    int addition = 2; // Amount to push branch ends out to sea.
    int x, y;

    if (branches > maxbranches)
        branches = maxbranches;

    twointegers destpoint;

    int branchends[maxbranches + 1][2];

    for (int i = 0; i < maxbranches + 1; i++)
    {
        for (int j = 0; j < 2; j++)
            branchends[i][j] = -1;
    }

    for (int n = 1; n <= branches; n++)
    {
        x = centrex;
        y = centrey;

        if (dir == 3 || dir == 7)
            y = y + randomsign(random(1, size));

        if (dir == 1 || dir == 5)
            x = x + randomsign(random(1, size));

        if (dir == 2 || dir == 6)
        {
            int amount = randomsign(random(1, size));
            x = x + amount;
            y = y + amount;
        }

        if (dir == 4 || dir == 8)
        {
            int amount = randomsign(random(1, size));
            x = x + amount;
            y = y - amount;
        }

        if (y < 0)
            y = 0;

        if (y > height)
            y = height;

        if (x<0 || x>width)
            x = wrap(x, width);

        int index = x * edge + y;

        if (mask[index] == 1 && sea[index] == 0)
        {
            destpoint = nearestpoint(sea, width, height, x, y, 10, 1);

            x = destpoint.x;
            y = destpoint.y;
        }

        if (x != -1 && y != -1)
        {
            bool goahead = 1;

            if (deltadir[index] != 0 && random(1, 2) == 1)
                goahead = 0;

            if (goahead == 1)
            {
                branchestotal++;

                branchends[branchestotal][0] = x;
                branchends[branchestotal][1] = y;
            }
        }
    }

    x = centrex;
    y = centrey;

    // Now move upstream the required amount.

    for (int n = 1; n <= upriver; n++)
    {
        deltarivers[x * edge + y] = 1; // Mark this bit of river to show that it's part of a river that's being turned into a delta.
        destpoint = getupstreamcell(world, x, y, riverjan, riverjul, riverdir);

        x = destpoint.x;
        y = destpoint.y;

        if (x == -1 || y == -1)
            return;
    }

    deltarivers[x * edge + y] = 1;

    int targetx = x;
    int targety = y; // These are the coordinates of the point where the delta branches are aiming for.

    // Now push those branch ends futher out to sea.

    for (int branch = 1; branch <= branchestotal; branch++)
    {
        x = branchends[branch][0];
        y = branchends[branch][1];

        if (x > targetx)
            x = x + addition;

        if (x < targetx)
            x = x - addition;

        if (y > targety)
            y = y + addition;

        if (y < targety)
            y = y + addition;

        if (x<0 || x>width)
            x = wrap(x, width);

        if (y < 0)
            y = 0;

        if (y > height)
            y = height;

        int xindex = x * edge + y;

        if (mask[xindex] == 1 && sea[xindex] == 0)
        {
            destpoint = nearestpoint(sea, width, height, x, y, 10, 1);

            x = destpoint.x;
            y = destpoint.y;
        }

        branchends[branch][0] = x;
        branchends[branch][1] = y;
    }

    int leftx = targetx;
    int rightx = targetx;
    int lefty = targety;
    int righty = targety; // These will mark the corners of the whole area containing the delta.

    // Now work out the flow for each branch.

    int targetindex = targetx * edge + targety;

    float janflow = (float)riverjan[targetindex];
    float julflow = (float)riverjul[targetindex];

    float branchjanflow = janflow / (float)(branchestotal + 1);
    float branchjulflow = julflow / (float)(branchestotal + 1);

    // Now do each branch in turn.

    for (int branch = 1; branch <= branchestotal; branch++)
    {
        x = branchends[branch][0];
        y = branchends[branch][1];

        int xindex = x * edge + y;

        deltajan[xindex] = (int)branchjanflow;
        deltajul[xindex] = (int)branchjulflow;

        for (int n = 1; n <= 100; n++)
        {
            if (deltadir[xindex] == 0) // If we're drawing a new branch route
            {
                int rshift = targetx - x;
                int dshift = targety - y;

                if (abs(rshift) < 2 && abs(dshift) < 2) // If we're next to the target go straight towards it
                {
                    if (targetx == x && targety == y - 1)
                        dir = 1;

                    if (targetx == x + 1 && targety == y - 1)
                        dir = 2;

                    if (targetx == x + 1 && targety == y)
                        dir = 3;

                    if (targetx == x + 1 && targety == y + 1)
                        dir = 5;

                    if (targetx == x - 1 && targety == y + 1)
                        dir = 6;

                    if (targetx == x - 1 && targety == y)
                        dir = 7;

                    if (targetx == x - 1 && targety == y - 1)
                        dir = 8;
                }
                else // Find a direction going roughly in the right direction
                {
                    if (rshift >= 0 && dshift >= 0) // Going roughly southeast
                    {
                        if (rshift > dshift)
                        {
                            if (random(1, 2) == 1)
                                dir = 3;
                            else
                                dir = 4;
                        }
                        else
                        {
                            if (random(1, 2) == 1)
                                dir = 4;
                            else
                                dir = 5;
                        }
                    }

                    if (rshift >= 0 && dshift < 0) // Going roughly northeast
                    {
                        if (rshift > 0 - dshift)
                        {
                            if (random(1, 2) == 1)
                                dir = 3;
                            else
                                dir = 2;
                        }
                        else
                        {
                            if (random(1, 2) == 1)
                                dir = 2;
                            else
                                dir = 1;
                        }
                    }

                    if (rshift < 0 && dshift >= 0) // Going roughly southwest
                    {
                        if (0 - rshift > dshift)
                        {
                            if (random(1, 2) == 1)
                                dir = 7;
                            else
                                dir = 6;
                        }
                        else
                        {
                            if (random(1, 2) == 1)
                                dir = 6;
                            else
                                dir = 5;
                        }
                    }

                    if (rshift < 0 && dshift < 0) // Going roughly northwest
                    {
                        if (rshift < dshift)
                        {
                            if (random(1, 2) == 1)
                                dir = 7;
                            else
                                dir = 8;
                        }
                        else
                        {
                            if (random(1, 2) == 1)
                                dir = 8;
                            else
                                dir = 1;
                        }
                    }
                    deltadir[xindex] = dir;
                }
            }
            else
                dir = deltadir[xindex];

            if (dir == 8 || dir == 1 || dir == 2)
                y--;

            if (dir == 4 || dir == 5 || dir == 6)
                y++;

            if (dir == 2 || dir == 3 || dir == 4)
                x++;

            if (dir == 6 || dir == 7 || dir == 8)
                x--;

            if (x < 0)
                x = width;

            if (x > width)
                x = 0;

            xindex = x * edge + y;

            if (y >= 0 && y <= height && mask[xindex] == 1)
            {
                deltajan[xindex] = deltajan[xindex] + (int)branchjanflow;
                deltajul[xindex] = deltajul[xindex] + (int)branchjulflow;

                if (nom[xindex] > sealevel)
                    nom[xindex] = sealevel + 1;

                if (x < leftx)
                    leftx = x;

                if (x > rightx)
                    rightx = x;

                if (y < lefty)
                    lefty = y;

                if (y > righty)
                    righty = y;

            }
            else
            {
                x = targetx;
                y = targety;
            }

            if (x == targetx && y == targety)
            {
                n = 100;
            }
        }
    }

    leftx--;
    rightx++;
    lefty--;
    righty++;

    if (leftx > rightx)
    {
        leftx = 0;
        rightx = width;
    }

    // Now fill in missing tiles.

    for (int i = targetx - 1; i <= targetx + 1; i++)
    {
        int ii = i;

        if (ii<0 || ii>width)
            ii = wrap(ii, width);

        int vii = ii * edge;

        for (int j = targety - 1; j <= targety + 1; j++)
        {
            int index = vii + j;

            if (j >= 0 && j <= height && mask[index] == 1)
            {
                // We're looking at each tile around the destination one.
                // For each of these, see whether any delta branch is pointing into it and add those amounts to its flow

                int janflow = 0;
                int julflow = 0;

                for (int k = ii - 1; k <= ii + 1; k++)
                {
                    int kk = k;

                    if (kk<0 || kk>width)
                        kk = wrap(kk, width);

                    int vk = kk * edge;

                    for (int l = j - 1; l <= j + 1; l++)
                    {
                        int lindex = vk + l;

                        if (l >= 0 && l <= height && mask[lindex] == 1)
                        {
                            if (kk != targetx || l != targety) // Don't do the actual destination tile
                            {
                                int dir = deltadir[lindex];

                                if (dir > 0) // If there's a delta branch here
                                {
                                    int dx = kk;
                                    int dy = l;
                                    
                                    if (dir == 8 || dir == 1 || dir == 2)
                                        dy--;

                                    if (dir == 4 || dir == 5 || dir == 6)
                                        dy++;

                                    if (dir == 2 || dir == 3 || dir == 4)
                                        dx++;

                                    if (dir == 6 || dir == 7 || dir == 8)
                                        dx--;

                                    if (dx == i && dy == j)
                                    {
                                        janflow = janflow + deltajan[lindex];
                                        julflow = julflow + deltajul[lindex];
                                    }
                                }
                            }
                        }
                    }
                }

                if (janflow > 0 || julflow > 0) // If there are delta branches flowing into this tile
                {
                    // First find the direction to the destination tile

                    int dir = getdir(ii, j, x, y);

                    // Now put a branch on our tile.

                    deltadir[index] = dir;
                    deltajan[index] = janflow;
                    deltajul[index] = julflow;
                }
            }
        }
    }

    // We will reduce the river below that point.

    targetindex = targetx * edge + targety;

    reducejan[targetindex] = riverjan[targetindex] - (int)(branchjanflow * 4.0f); // Shouldn't really be multiplied, but this is to make it more visible on the map.
    reducejul[targetindex] = riverjul[targetindex] - (int)(branchjulflow * 4.0f);

    //Now we simply make one final delta cell in the target cell, bringing the branches back together to meet the main river.

    int ux = -1;
    int uy = -1;

    x = targetx;
    y = targety;

    destpoint = getupstreamcell(world, x, y, riverjan, riverjul, riverdir);

    ux = destpoint.x;
    uy = destpoint.y;

    if (ux == x && uy == y - 1)
        dir = 1;

    if (ux == x + 1 && uy == y - 1)
        dir = 2;

    if (ux == x + 1 && uy == y)
        dir = 3;

    if (ux == x + 1 && uy == y + 1)
        dir = 4;

    if (ux == x && uy == y + 1)
        dir = 5;

    if (ux == x - 1 && uy == y + 1)
        dir = 6;

    if (ux == x - 1 && uy == y)
        dir = 7;

    if (ux == x - 1 && uy == y - 1)
        dir = 8;

    int xindex = x * edge + y;

    deltadir[xindex] = dir;
    deltajan[xindex] = 0 - (int)(branchjanflow * (float)branchestotal);
    deltajul[xindex] = 0 - (int)(branchjulflow * (float)branchestotal);

    deltarivers[xindex] = 1;
}

// This function diverts rivers that run through deltas, so that they follow the pattern of the delta branches more closely.

void divertdeltarivers(planet& world, vector<int>& deltarivers)
{
    int edge = world.edge();

    vector<int> removedrivers(6 * edge * edge, 0);
    vector<int> checked(6 * edge * edge, 0);

    int riverno = 0;

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                if (checked[index] == 0 && world.sea(face, i, j) == 0 && world.riverdir(face, i, j) != 0)
                {
                    int x = i;
                    int y = j;

                    bool keepgoing = 1;
                    bool found = 0;
                    bool alreadydeleted = 0;

                    do
                    {
                        int xindex = vface + x * edge + y;

                        checked[xindex] = 1; // Mark this as checked so we won't try to follow any rivers from this point again.

                        int dir = world.riverdir(face, x, y);

                        if (dir == 8 || dir == 1 || dir == 2)
                            y--;

                        if (dir == 4 || dir == 5 || dir == 6)
                            y++;

                        if (dir == 2 || dir == 3 || dir == 4)
                            x++;

                        if (dir == 6 || dir == 7 || dir == 8)
                            x--;

                        if (x < 3 || x > edge - 4 || y < 3 || y > edge - 4)
                            keepgoing = 0;
                        else
                        {

                            if (world.sea(face, x, y) == 1) // If it reaches the sea
                                keepgoing = 0;

                            if (checked[xindex] == 1) // If it reaches a river that's already been done
                            {
                                keepgoing = 0;
                            }

                            if (deltarivers[xindex] == 1) // If it reaches a river that's part of the delta
                            {
                                keepgoing = 0;
                            }

                            if (keepgoing == 1 && (world.deltajan(face, x, y) != 0 || world.deltajul(face, x, y) != 0)) // If it reaches an actual delta
                            {
                                keepgoing = 0;
                                found = 1;
                            }

                            // Now check whether it's about to cross over a delta diagonally. If it is, we will need to divert it into one of the neighbouring delta cells.

                            dir = world.riverdir(face, x, y);

                            if (dir == 2 || dir == 4 || dir == 6 || dir == 8)
                            {
                                int dx = -1;
                                int dy = -1; // Coordinates of the cell that this river is about to head into

                                int mx = -1;
                                int my = -1;
                                int nx = -1;
                                int ny = -1; // These are the coordinates of neighbouring cells that this river isn't actually heading into, to check for delta branches.

                                bool mfound = 0;
                                bool nfound = 0; // To tell whether we have found a delta branch in either of these cells.

                                if (dir == 2)
                                {
                                    dx = x + 1;
                                    dy = y - 1;

                                    if (world.deltajan(face, dx, dy) == 0 || world.deltajul(face, dx, dy) == 0) // If this river isn't about to run into a delta branch
                                    {
                                        // Check the ones to the sides to see if they contain a delta branch.

                                        mx = x;
                                        my = y - 1;

                                        nx = x + 1;
                                        ny = y;

                                        if (world.deltajan(face, mx, my) != 0 || world.deltajul(face, mx, my) != 0)
                                            mfound = 1;

                                        if (world.deltajan(face, nx, ny) != 0 || world.deltajul(face, nx, ny) != 0)
                                            nfound = 1;
                                    }
                                }

                                if (dir == 4)
                                {
                                    dx = x + 1;
                                    dy = y + 1;

                                    if (world.deltajan(face, dx, dy) == 0 || world.deltajul(face, dx, dy) == 0) // If this river isn't about to run into a delta branch
                                    {
                                        // Check the ones to the sides to see if they contain a delta branch.

                                        mx = x;
                                        my = y + 1;

                                        nx = x + 1;
                                        ny = y;

                                        if (world.deltajan(face, mx, my) != 0 || world.deltajul(face, mx, my) != 0)
                                            mfound = 1;

                                        if (world.deltajan(face, nx, ny) != 0 || world.deltajul(face, nx, ny) != 0)
                                            nfound = 1;
                                    }
                                }

                                if (dir == 6)
                                {
                                    dx = x - 1;
                                    dy = y + 1;

                                    if (world.deltajan(face, dx, dy) == 0 || world.deltajul(face, dx, dy) == 0) // If this river isn't about to run into a delta branch
                                    {
                                        // Check the ones to the sides to see if they contain a delta branch.

                                        mx = x - 1;
                                        my = y;

                                        nx = x;
                                        ny = y + 1;

                                        if (world.deltajan(face, mx, my) != 0 || world.deltajul(face, mx, my) != 0)
                                            mfound = 1;

                                        if (world.deltajan(face, nx, ny) != 0 || world.deltajul(face, nx, ny) != 0)
                                            nfound = 1;
                                    }
                                }

                                if (dir == 8)
                                {
                                    dx = x - 1;
                                    dy = y - 1;

                                    if (world.deltajan(face, dx, dy) == 0 || world.deltajul(face, dx, dy) == 0) // If this river isn't about to run into a delta branch
                                    {
                                        // Check the ones to the sides to see if they contain a delta branch.

                                        mx = x;
                                        my = y - 1;

                                        nx = x - 1;
                                        ny = y;

                                        if (world.deltajan(face, mx, my) != 0 || world.deltajul(face, mx, my) != 0)
                                            mfound = 1;

                                        if (world.deltajan(face, nx, ny) != 0 || world.deltajul(face, nx, ny) != 0)
                                            nfound = 1;
                                    }
                                }

                                if (mfound == 1 || nfound == 1) // We found one!
                                {
                                    keepgoing = 1;

                                    int tx = nx;
                                    int ty = ny; // The new target to divert our river into. By default it's n.

                                    if (mfound == 1 && nfound == 1) // If both possibilities contain a delta branch, we need the downstream one. (Remember it will show as the upstream one because delta branches are calculated in reverse.)
                                    {
                                        globepoint dest = getflowdestination(world, face, mx, my, world.deltadir(face, mx, my));

                                        if (dest.face==face && dest.x == nx && dest.y == ny) // m is flowing into n. That means we want to divert into m, as it's really downstream.
                                        {
                                            tx = mx;
                                            ty = my;
                                        }
                                    }

                                    if (nfound == 0)
                                    {
                                        tx = mx;
                                        ty = my;
                                    }
                                    // tx and ty now contains the coordinates of the tile we want to divert our river into.

                                    // Delete the river from this point onwards.

                                    int janload = world.riverjan(face, x, y);
                                    int julload = world.riverjul(face, x, y);

                                    riverno++;

                                    removeriver(world, removedrivers, riverno, face, x, y);

                                    // Now recreate it in this cell, pointing to the new destination cell.

                                    world.setriverjan(face, x, y, janload);
                                    world.setriverjul(face, x, y, julload);
                                    world.setriverdir(face, x, y, getdir(x, y, tx, ty));

                                    alreadydeleted = 1;

                                    // Now on the next pass of this loop, the program will move x and y in this new direction and discover that it's on a delta branch!
                                }
                            }
                        }
                    } while (keepgoing == 1);

                    if (found == 1) // If we've actually hit a delta, time to sort out this river!
                    {
                        int janload = world.riverjan(face, x, y);
                        int julload = world.riverjul(face, x, y);

                        // First, delete the river from this point onwards.

                        if (alreadydeleted == 0)
                        {
                            riverno++;

                            removeriver(world, removedrivers, riverno, face, x, y);
                        }

                        // Now, recreate the river, following the line of the delta branch.

                        keepgoing = 1;
                        globepoint dest, check;

                        int tally = 0;

                        do
                        {
                            tally++;

                            // We need to find a delta branch that's flowing into this tile.

                            int dir;
                            dest.x = -1;
                            dest.y = -1;

                            for (int k = x - 1; k <= x + 1; k++)
                            {
                                if (k >= 0 && k < edge)
                                {
                                    for (int l = y - 1; l <= y + 1; l++)
                                    {
                                        if (l >= 0 && l < edge)
                                        {
                                            if ((k != x || l != y) && world.deltadir(face, k, l) != 0)
                                            {
                                                check = getflowdestination(world, face, k, l, world.deltadir(face, k, l));

                                                if (check.face == face && check.x == x && check.y == y)
                                                {
                                                    dest.x = k;
                                                    dest.y = l;

                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            if (dest.x == -1) // We didn't find one
                                keepgoing = 0;
                            else
                            {
                                dir = getdir(x, y, dest.x, dest.y);

                                world.setriverdir(face, x, y, dir);
                                world.setriverjan(face, x, y, world.riverjan(face, x, y) + janload);
                                world.setriverjul(face, x, y, world.riverjul(face, x, y) + julload);

                                if (dir == 8 || dir == 1 || dir == 2)
                                    y--;

                                if (dir == 4 || dir == 5 || dir == 6)
                                    y++;

                                if (dir == 2 || dir == 3 || dir == 4)
                                    x++;

                                if (dir == 6 || dir == 7 || dir == 8)
                                    x--;

                                if (x < 0 || x >= edge || y < 0 || y >= edge)
                                    keepgoing = 0;
                            }

                            if (tally > 10000) // In case of infinite loops...
                                keepgoing = 0;

                        } while (keepgoing == 1);
                    }
                }
            }
        }
    }
}

// This goes over the map and ensures that rivers don't inexplicably grow too much within delta regions.

void checkrivers(planet& world)
{
    //highres_timer_t timer("Check Rivers"); // 666: 4156, 999: 6442 => 666: 2926, 999: 2907
    int edge = world.edge();

    int maxsource = 50; // Largest size a river source can be (jan and jul each)

    // First, go through and make sure there aren't any absurd river sources (these may be accidentally created in the delta regions)

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.deltadir(face, i, j) != 0 && checkwaterinflow(world, face, i, j) == 0) // If nothing is flowing into this cell, it's a river source
                {
                    if (world.riverjan(face, i, j) > maxsource)
                        world.setriverjan(face, i, j, maxsource);

                    if (world.riverjul(face, i, j) > maxsource)
                        world.setriverjul(face, i, j, maxsource);
                }
            }
        }
    }

    // Now go through and ensure that no river grows weirdly for no apparent reason.

    bool found = 0;

    int amount = 4;

    do
    {
        found = 0;

        for (int face = 0; face < 6; face++)
        {
            for (int i = amount * 2; i < edge - amount * 2; i++)
            {
                for (int j = amount * 2; j < edge - amount * 2; j++)
                {
                    bool neardelta = 0;

                    for (int k = i - amount; k <= i + amount; k++)
                    {
                        for (int l = j - amount; l <= j + amount; l++)
                        {
                            if (world.deltadir_no_bounds_check(face, k, l) != 0)
                            {
                                neardelta = 1;
                                k = i + amount;
                                l = j + amount;
                            }
                        }
                    }

                    if (neardelta == 1) // Only look at cells near river deltas.
                    {
                        twointegers inflow = gettotalinflow(world, face, i, j);

                        if (world.riverjan(face, i, j) > inflow.x)
                        {
                            world.setriverjan(face, i, j, inflow.x);
                            found = 1;
                        }

                        if (world.riverjul(face, i, j) > inflow.y)
                        {
                            world.setriverjul(face, i, j, inflow.y);
                            found = 1;
                        }
                    }
                }
            }
        }
    } while (found == 1);
}

// This lays down wetlands.

void createwetlands(planet& world, boolshapetemplate smalllake[])
{
    int edge = world.edge();
    int maxelev = world.maxelevation();

    int minflow = 100; // Average flow must be at least this to have wetlands.
    int maxflatness = 5; // Must be no less flat than this.
    float factor = (float)maxelev / 5.0f;

    // First, create a fractal to be the drainage map.

    int grain = 8; // This is the level of detail on this map.
    float valuemod = 0.2f;
    float valuemod2 = 3.0f;
    int shapenumber;

    vector<int> drainage(6 * edge * edge, 0);

    createfractal(drainage, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    // Now go through the map and place wetlands.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                if (world.sea(face, i, j) == 0 && world.special(face, i, j) == 0 && world.lakesurface(face, i, j) == 0 && world.climate(face, i, j) != 31 && world.climate(face, i, j) != 5)
                {
                    if (world.deltadir(face, i, j) != 0)
                    {
                        shapenumber = random(0, 5);
                        pastewetland(world, face, i, j, shapenumber, smalllake);
                    }
                    else
                    {
                        if (world.riveraveflow(face, i, j) >= minflow)
                        {
                            int flatness = getflatness(world, face, i, j);

                            if (flatness <= maxflatness)
                            {
                                float drain = (float)drainage[index];

                                drain = drain / factor;

                                int d = (int)drain; // This should be from 1 to 5.
                                d = d * 2;

                                int prob = flatness + d; // The flatter the land, the more likely wetlands are.

                                int flow = world.riveraveflow(face, i, j) / 1000;

                                prob = prob - flow; // The bigger the flow here, the more likely wetlands are.

                                for (int k = - 1; k <= 1; k++) // The more lakes/sea there are around here, the more likely wetlands are.
                                {
                                    for (int l = -1; l <= 1; l++)
                                    {
                                        globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                                        if (lpoint.face != -1)
                                        {
                                            if (world.lakesurface(lpoint.face, lpoint.x, lpoint.y) != 0)
                                                prob = prob - 3;

                                            if (world.sea(lpoint.face, lpoint.x, lpoint.y) == 1)
                                                prob = prob - 4;
                                        }
                                    }
                                }

                                prob = prob + (40 - (world.averain(face, i, j) / 10));

                                if (world.climate(face, i, j) == 30 && world.maxtemp(face, i, j) >= 5) // Much likelier in tundra with warmish summers
                                    prob = prob / 4;

                                if (prob < 1)
                                    prob = 1;

                                if (random(1, prob) == 1) // Put some wetlands here.
                                {
                                    shapenumber = random(0, 5);
                                    pastewetland(world, face, i, j, shapenumber, smalllake);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    int deltaswampchance = 6; // The BIGGER this is, the more probable wetlands will be around delta branches.

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++) // Add wetlands to the actual delta branches
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.deltadir(face, i, j) > 0 && world.sea(face, i, j) == 0 && random(1, deltaswampchance) != 1)
                    world.setspecial(face, i, j, 130);

            }
        }
    }

    // Now fill in extra wetlands if necessary to reach the sea or other wetlands.

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.sea(face, i, j) == 0 && world.special(face, i, j) == 0 && world.climate(face, i, j) != 31)
                {
                    globepoint northpoint = getglobepoint(edge, face, i, j, 0, -1);
                    globepoint southpoint = getglobepoint(edge, face, i, j, 0, 1);
                    globepoint eastpoint = getglobepoint(edge, face, i, j, -1, 0);
                    globepoint westpoint = getglobepoint(edge, face, i, j, 1, 0);

                    globepoint northeastpoint = getglobepoint(edge, northpoint.face, northpoint.x, northpoint.y, 1, 0);
                    globepoint northwestpoint = getglobepoint(edge, northpoint.face, northpoint.x, northpoint.y, -1, 0);
                    globepoint southeastpoint = getglobepoint(edge, southpoint.face, southpoint.x, southpoint.y, 1, 0);
                    globepoint southwestpoint = getglobepoint(edge, southpoint.face, southpoint.x, southpoint.y, -1, 0);

                    if (world.special(northpoint.face, northpoint.x, northpoint.y) == 130)
                    {
                        if (world.special(southpoint.face, southpoint.x, southpoint.y) == 130)
                            world.setspecial(face, i, j, 130);

                        if (world.sea(southpoint.face, southpoint.x, southpoint.y) == 1)
                            world.setspecial(face, i, j, 130);
                    }

                    if (world.special(southpoint.face, southpoint.x, southpoint.y) == 130)
                    {
                        if (world.special(northpoint.face, northpoint.x, northpoint.y) == 130)
                            world.setspecial(face, i, j, 130);

                        if (world.sea(northpoint.face, northpoint.x, northpoint.y) == 1)
                            world.setspecial(face, i, j, 130);
                    }

                    if (world.special(westpoint.face, westpoint.x, westpoint.y) == 130)
                    {
                        if (world.special(eastpoint.face, eastpoint.x, eastpoint.y) == 130)
                            world.setspecial(face, i, j, 130);

                        if (world.sea(eastpoint.face, eastpoint.x, eastpoint.y) == 1)
                            world.setspecial(face, i, j, 130);
                    }

                    if (world.special(eastpoint.face, eastpoint.x, eastpoint.y) == 130)
                    {
                        if (world.special(westpoint.face, westpoint.x, westpoint.y) == 130)
                            world.setspecial(face, i, j, 130);

                        if (world.sea(westpoint.face, westpoint.x, westpoint.y) == 1)
                            world.setspecial(face, i, j, 130);
                    }

                    if (world.special(northwestpoint.face, northwestpoint.x, northwestpoint.y) == 130)
                    {
                        if (world.special(southeastpoint.face, southeastpoint.x, southeastpoint.y) == 130)
                            world.setspecial(face, i, j, 130);

                        if (world.sea(southeastpoint.face, southeastpoint.x, southeastpoint.y) == 1)
                            world.setspecial(face, i, j, 130);
                    }

                    if (world.special(northeastpoint.face, northeastpoint.x, northeastpoint.y) == 130)
                    {
                        if (world.special(southwestpoint.face, southwestpoint.x, southwestpoint.y) == 130)
                            world.setspecial(face, i, j, 130);

                        if (world.sea(southwestpoint.face, southwestpoint.x, southwestpoint.y) == 1)
                            world.setspecial(face, i, j, 130);
                    }

                    if (world.special(southwestpoint.face, southwestpoint.x, southwestpoint.y) == 130)
                    {
                        if (world.special(northeastpoint.face, northeastpoint.x, northeastpoint.y) == 130)
                            world.setspecial(face, i, j, 130);

                        if (world.sea(northeastpoint.face, northeastpoint.x, northeastpoint.y) == 1)
                            world.setspecial(face, i, j, 130);
                    }

                    if (world.special(southeastpoint.face, southeastpoint.x, southeastpoint.y) == 130)
                    {
                        if (world.special(northwestpoint.face, northwestpoint.x, northwestpoint.y) == 130)
                            world.setspecial(face, i, j, 130);

                        if (world.sea(northwestpoint.face, northwestpoint.x, northwestpoint.y) == 1)
                            world.setspecial(face, i, j, 130);
                    }
                }
            }
        }
    }

    // For some reason it keeps putting wetlands all over the sea, so we remove them.

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.sea(face, i, j) == 1)
                    world.setspecial(face, i, j, 0);

            }
        }
    }

    // Now make the wetlands salty/brackish if necessary.

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.special(face, i, j) == 130)
                {
                    int salt = 0;

                    for (int k = - 1; k <= 1; k++)
                    {
                        for (int l = -1; l <= 1; l++)
                        {
                            globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                            if (lpoint.face != -1)
                            {
                                if (world.sea(lpoint.face, lpoint.x, lpoint.y) == 1)
                                    salt = 1;

                                if (world.special(lpoint.face, lpoint.x, lpoint.y) == 100 || world.special(lpoint.face, lpoint.x, lpoint.y) == 110)
                                    salt = 1;
                            }
                        }
                    }

                    if (salt == 1)
                        world.setspecial(face, i, j, 132);
                    else
                    {
                        for (int k = - 2; k <= 2; k++)
                        {
                            for (int l = -2; l <= 2; l++)
                            {
                                globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                                if (lpoint.face != -1)
                                {
                                    if (world.sea(lpoint.face, lpoint.x, lpoint.y) == 1)
                                        salt = 1;

                                    if (world.special(lpoint.face, lpoint.x, lpoint.y) == 100 || world.special(lpoint.face, lpoint.x, lpoint.y) == 110)
                                        salt = 1;
                                }
                            }
                        }

                        if (salt == 1)
                            world.setspecial(face, i, j, 131);
                    }
                }
            }
        }
    }
}

// This actually pastes a patch of wetlands onto the map.

void pastewetland(planet& world, int face, int centrex, int centrey, int shapenumber, boolshapetemplate smalllake[])
{
    int edge = world.edge();

    int imheight = smalllake[shapenumber].ysize() - 1;
    int imwidth = smalllake[shapenumber].xsize() - 1;

    globepoint startpoint = getglobepoint(edge, face, centrex, centrey, 0 - imwidth / 2, 0 - imheight / 2); // Coordinates of the top left corner.

    if (startpoint.face == -1)
        return;

    bool flipx = 0; // This is in case the origin point is on another face and we may need to change the direction of the shape.
    bool flipy = 0;

    if (face == 1 && startpoint.face == 4)
        flipy = 1;

    if (face == 2 && startpoint.face == 4)
    {
        flipx = 1;
        flipy = 1;
    }

    if (face == 3 && startpoint.face == 4)
        flipx = 1;

    if (face == 4 && startpoint.face == 2)
    {
        flipx = 1;
        flipy = 1;
    }

    if (face == 4 && startpoint.face == 3)
        flipy = 1;

    if (face == 5 && startpoint.face == 3)
        flipx = 1;

    int leftr = random(0, 1); // If it's 1 then we reverse it left-right
    int downr = random(0, 1); // If it's 1 then we reverse it top-bottom

    int istart = 0, desti = imwidth + 1, istep = 1;
    int jstart = 0, destj = imheight + 1, jstep = 1;

    if (leftr == 1)
    {
        istart = imwidth;
        desti = -1;
        istep = -1;
    }

    if (downr == 1)
    {
        jstart = imwidth;
        destj = -1;
        jstep = -1;
    }

    int imap = -1;
    int jmap = -1;

    for (int i = istart; i != desti; i = i + istep)
    {
        imap++;
        jmap = -1;

        for (int j = jstart; j != destj; j = j + jstep)
        {
            jmap++;

            if (smalllake[shapenumber].point(i, j) == 1)
            {
                int ii = imap;
                int jj = jmap;

                if (flipx)
                    ii = -ii;

                if (flipy)
                    jj = -jj;

                globepoint thispoint = getglobepoint(edge, startpoint.face, startpoint.x, startpoint.y, ii, jj);

                if (thispoint.face == -1)
                    return;
                else
                {
                    if (world.sea(thispoint.face, thispoint.x, thispoint.y) == 0 && world.mountainheight(thispoint.face, thispoint.x, thispoint.y) == 0) // Don't try to put wetlands on top of sea or mountains...
                        world.setspecial(thispoint.face, thispoint.x, thispoint.y, 130);
                }
            }
        }
    }
}

// This removes any wetlands tiles that border large lakes.

void removeexcesswetlands(planet& world)
{
    int edge = world.edge();

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.special(face, i, j) == 130)
                {
                    bool nearlake = 0;

                    for (int k = - 1; k <= 1; k++)
                    {
                        for (int l = -1; l <= 1; l++)
                        {
                            globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                            if (lpoint.face != -1)
                            {
                                if (world.truelake(lpoint.face, lpoint.x, lpoint.y) == 1)
                                {
                                    nearlake = 1;
                                    k = 1;
                                    l = 1;
                                }
                            }
                        }
                    }

                    if (nearlake == 1)
                    {
                        world.setlakesurface(face, i, j, 0);
                        world.setspecial(face, i, j, 0);
                    }
                }
            }
        }
    }
}

// This refines the roughness map so that it takes into account more factors, producing a roughness factor that will be used for the fractal terrain in the regional map.

void refineroughnessmap(planet& world)
{
    int edge = world.edge();
    int sealevel = world.sealevel();
    int maxelev = world.maxelevation();
    float maxvaluemod = 1.0f; // Maximum that a valuemod can be.
    float maxcoastalvaluemod = 0.1f; //0.08; // Maximum that it can at the coasts.

    // This array will hold the valuemod for every tile on the map. The valuemod is used in the regional map to determine how rough the diamond-square routine makes the terrain.

    vector<float> valuemod(6 * edge * edge, 0);

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                if (world.sea(face, i, j) == 0)
                {
                    globepoint northpoint = getglobepoint(edge, face, i, j, 0, -1);
                    globepoint southpoint = getglobepoint(edge, face, i, j, 0, 1);
                    globepoint eastpoint = getglobepoint(edge, face, i, j, -1, 0);
                    globepoint westpoint = getglobepoint(edge, face, i, j, 1, 0);

                    if (world.nom(face, i, j) <= sealevel && world.nom(northpoint.face, northpoint.x, northpoint.y) <= sealevel && world.nom(southpoint.face, southpoint.x, southpoint.y) <= sealevel && world.nom(eastpoint.face, eastpoint.x, eastpoint.y) <= sealevel && world.nom(westpoint.face, westpoint.x, westpoint.y) <= sealevel) // Sea beds will be very smooth.
                    {
                        float m = 0.08f / (float)maxelev;
                        float n = world.roughness(face, i, j);
                        float extrabit = 0.01f * (float)random(1, 7);

                        valuemod[index] = (m * n) + extrabit;
                    }
                    else // On land, the higher it is, the rougher it is.
                    {
                        valuemod[index] = world.roughness(face, i, j) / (float)maxelev;
                    }
                }
                else // Sea beds - very smooth.
                {

                    float m = 0.08f / (float)maxelev;
                    float n = world.roughness(face, i, j);
                    float extrabit = 0.01f * (float)random(1, 7);

                    valuemod[index] = (m * n) + extrabit;
                }
            }
        }
    }

    // Now we blur that, so that there aren't sharp transitions in roughness on the regional map.

    /*
    int amount = 2; // Amount to blur by.

    vector<vector<float>> valuemod2(ARRAYWIDTH, vector<float>(ARRAYHEIGHT, 0));

    for (int i = 0; i < edge; i++)
    {
        for (int j = 0; j < edge; j++)
        {
            if (world.nom(face, i, j) > sealevel && world.truelake(face, i, j) == 0)
            {
                bool goahead = 1;

                for (int k = i - 1; k <= i + 1; k++)
                {
                    int kk = k;

                    if (kk<0 || kk>width)
                        kk = wrap(kk, width);

                    for (int l = j - 1; l <= j + 1; l++)
                    {
                        if (l >= 0 && l < edge)
                        {
                            if (world.sea(kk, l) == 1)
                            {
                                goahead = 0;
                                k = i + 1;
                                l = j + 1;
                            }
                        }
                    }
                }

                if (goahead == 1)
                {
                    float total = 0.0f;
                    float crount = 0.0f;

                    for (int k = i - amount; k <= i + amount; k++)
                    {
                        int kk = k;

                        if (kk<0 || kk>width)
                            kk = wrap(kk, width);

                        for (int l = j - amount; l <= j + amount; l++)
                        {
                            if (l >= 0 && l < edge)
                            {
                                if (world.nom(kk, l) > sealevel && world.truelake(kk, l) == 0)
                                {
                                    total = total + valuemod[kk][l];
                                    crount++;
                                }
                            }
                        }
                    }
                    valuemod2[face][i][j] = total / crount;

                    if (valuemod2[face][i][j] > maxvaluemod)
                        valuemod2[face][i][j] = maxvaluemod;
                }
                else
                {
                    valuemod2[face][i][j] = valuemod[face][i][j];

                    if (valuemod2[face][i][j] > maxcoastalvaluemod)
                        valuemod2[face][i][j] = maxcoastalvaluemod;
                }
            }
            else
                valuemod2[face][i][j] = 0.0f;
        }
    }
    */

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vface + j;

                if (valuemod[index] < 0.0f)
                    world.setroughness(face, i, j, 0.0f);
                else
                    world.setroughness(face, i, j, valuemod[index]);
            }
        }
    }
}

// This removes any cells of sea that are next to lakes.

void removesealakes(planet& world)
{
    int edge = world.edge();
    int sealevel = world.sealevel();

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                int surface = world.lakesurface(face, i, j);

                if (surface != 0)
                {
                    for (int k = - 1; k <= 1; k++)
                    {
                        for (int l = -1; l <= 1; l++)
                        {
                            globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                            if (lpoint.face != -1)
                            {
                                if (world.nom(lpoint.face, lpoint.x, lpoint.y) <= sealevel && world.lakesurface(lpoint.face, lpoint.x, lpoint.y) == 0)
                                    world.setlakesurface(lpoint.face, lpoint.x, lpoint.y, surface);
                            }
                        }
                    }
                }
            }
        }
    }
}

// This ensures that lakes don't have fragments of themselves nearby.

void connectlakes(planet& world)
{
    int edge = world.edge();

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.lakesurface(face, i, j) != 0)
                {
                    globepoint northpoint = getglobepoint(edge, face, i, j, 0, -1);
                    globepoint southpoint = getglobepoint(edge, face, i, j, 0, 1);
                    globepoint eastpoint = getglobepoint(edge, face, i, j, -1, 0);
                    globepoint westpoint = getglobepoint(edge, face, i, j, 1, 0);

                    globepoint nnorthpoint = getglobepoint(edge, face, i, j, 0, -2);
                    globepoint ssouthpoint = getglobepoint(edge, face, i, j, 0, 2);
                    globepoint eeastpoint = getglobepoint(edge, face, i, j, -2, 0);
                    globepoint wwestpoint = getglobepoint(edge, face, i, j, 2, 0);

                    int surface = world.lakesurface(face, i, j);
                    int elev = world.nom(face, i, j);
                    int special = world.special(face, i, j);

                    if (world.lakesurface(eastpoint.face, eastpoint.x, eastpoint.y) == 0 && world.lakesurface(eeastpoint.face, eeastpoint.x, eeastpoint.y) == surface)
                    {
                        world.setnom(eastpoint.face, eastpoint.x, eastpoint.y, elev);
                        world.setspecial(eastpoint.face, eastpoint.x, eastpoint.y, special);
                        world.setlakesurface(eastpoint.face, eastpoint.x, eastpoint.y, surface);
                    }

                    if (world.lakesurface(westpoint.face, westpoint.x, westpoint.y) == 0 && world.lakesurface(wwestpoint.face, wwestpoint.x, wwestpoint.y) == surface)
                    {
                        world.setnom(westpoint.face, westpoint.x, westpoint.y, elev);
                        world.setspecial(westpoint.face, westpoint.x, westpoint.y, special);
                        world.setlakesurface(westpoint.face, westpoint.x, westpoint.y, surface);
                    }

                    if (world.lakesurface(northpoint.face, northpoint.x, northpoint.y) == 0 && world.lakesurface(nnorthpoint.face, nnorthpoint.x, nnorthpoint.y) == surface)
                    {
                        world.setnom(northpoint.face, northpoint.x, northpoint.y, elev);
                        world.setspecial(northpoint.face, northpoint.x, northpoint.y, special);
                        world.setlakesurface(nnorthpoint.face, nnorthpoint.x, nnorthpoint.y, surface);
                    }

                    if (world.lakesurface(southpoint.face, southpoint.x, southpoint.y) == 0 && world.lakesurface(ssouthpoint.face, ssouthpoint.x, ssouthpoint.y) == surface)
                    {
                        world.setnom(southpoint.face, southpoint.x, southpoint.y, elev);
                        world.setspecial(southpoint.face, southpoint.x, southpoint.y, special);
                        world.setlakesurface(ssouthpoint.face, ssouthpoint.x, ssouthpoint.y, surface);
                    }
                }
            }
        }
    }
}

// This removes odd areas where the seasonal rainfall is reversed. (Something about the river generation causes this, but I can't work out why...)

void correctseasonalrainfall(planet& world)
{
    int edge = world.edge();

    int range = 3;

    for (int n = 0; n < 4; n++)
    {
        for (int face = 0; face < 6; face++) // Compare points to those in the surrounding area
        {
            for (int i = range; i < edge - range; i++)
            {
                for (int j = range; j < edge - range; j++)
                {
                    if (world.sea(face, i, j) == 0)
                    {
                        int janrain = world.janrain(face, i, j);
                        int julrain = world.julrain(face, i, j);

                        int diff = janrain - julrain;

                        if (diff > 1 || diff < -1)
                        {
                            int janhigh = 0;
                            int julhigh = 0;

                            for (int k = i - range; k <= i + range; k++)
                            {
                                for (int l = j - range; l <= j + range; l++)
                                {
                                    int thisjanrain = world.janrain(face, k, l);
                                    int thisjulrain = world.julrain(face, k, l);

                                    if (thisjanrain > thisjulrain)
                                        janhigh++;

                                    if (thisjulrain > thisjanrain)
                                        julhigh++;
                                }
                            }

                            if (diff > 1 && julhigh > janhigh)
                            {
                                world.setjanrain(face, i, j, julrain);
                                world.setjulrain(face, i, j, janrain);
                            }

                            if (diff < 1 && janhigh > julhigh)
                            {
                                world.setjanrain(face, i, j, julrain);
                                world.setjulrain(face, i, j, janrain);
                            }
                        }
                    }
                }
            }
        }
    }
}

// This ensures that sea temperatures along the coasts are correct.

void correctcoastaltemperatures(planet& world)
{
    int edge = world.edge();

    vector<int>jantemp(6 * edge * edge, 0);
    vector<int>jultemp(6 * edge * edge, 0);

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                jantemp[index] = world.jantemp(face, i, j);
                jultemp[index] = world.jultemp(face, i, j);
            }
        }
    }

    int searchdist = 3;
    int correctdist = 3;
    int blurdist = 3;

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                if (world.sea(face, i, j))
                {
                    bool nearland = 0;

                    for (int k = -searchdist; k <= searchdist; k++)
                    {
                        for (int l = -searchdist; l <= searchdist; l++)
                        {
                            if (k != 0 || l != 0)
                            {
                                globepoint thispoint = getglobepoint(edge, face, i, j, k, l);

                                if (thispoint.face != -1 && world.sea(thispoint.face, thispoint.x, thispoint.y) != 1)
                                {
                                    nearland = 1;
                                    k = searchdist;
                                    l = searchdist;
                                }
                            }
                        }
                    }

                    if (nearland)
                    {
                        int maxjantemp = -10000;
                        int maxjultemp = -10000;

                        for (int k = -correctdist; k <= correctdist; k++)
                        {
                            for (int l = -correctdist; l <= correctdist; l++)
                            {
                                if (k != 0 || l != 0)
                                {
                                    globepoint thispoint = getglobepoint(edge, face, i, j, k, l);

                                    if (thispoint.face != -1)
                                    {
                                        if (world.sea(thispoint.face, thispoint.x, thispoint.y))
                                        {
                                            int thisjan = world.jantemp(thispoint.face, thispoint.x, thispoint.y);
                                            int thisjul = world.jultemp(thispoint.face, thispoint.x, thispoint.y);

                                            if (thisjan > maxjantemp)
                                                maxjantemp = thisjan;

                                            if (thisjul > maxjultemp)
                                                maxjultemp = thisjul;
                                        }
                                    }
                                }
                            }
                        }

                        jantemp[index] = maxjantemp;
                        jultemp[index] = maxjultemp;
                    }
                }
            }
        }

        for (int face = 0; face < 6; face++)
        {
            int vface = face * edge * edge;

            for (int i = 0; i < edge; i++)
            {
                int vi = vface + i * edge;

                for (int j = 0; j < edge; j++)
                {
                    int index = vi + j;

                    if (world.sea(face, i, j))
                    {
                        float jantotal = 0.0f;
                        float jultotal = 0.0f;

                        float crount = 0.0f;

                        for (int k = -blurdist; k <= blurdist; k++)
                        {
                            for (int l = -blurdist; l <= blurdist; l++)
                            {
                                globepoint thispoint = getglobepoint(edge, face, i, j, k, l);

                                if (thispoint.face != -1 && world.sea(thispoint.face, thispoint.x, thispoint.y))
                                {
                                    jantotal = jantotal + (float)jantemp[thispoint.face * edge * edge + thispoint.x * edge + thispoint.y];
                                    jultotal = jultotal + (float)jultemp[thispoint.face * edge * edge + thispoint.x * edge + thispoint.y];

                                    crount++;
                                }
                            }
                        }

                        if (crount > 0.0f)
                        {
                            world.setjantemp(face, i, j, (int)(jantotal / crount));
                            world.setjultemp(face, i, j, (int)(jultotal / crount));
                        }
                    }
                }
            }
        }
    }
}

// This draws the basic clouds. (note: doesn't take account of rainfall)

void makeclouds(planet& world, vector<fourglobepoints>& dirpoint)
{
    long seed = world.seed();
    fast_srand(seed);

    int edge = world.edge();
    int eedge = edge * edge;
    int maxelev = world.maxelevation();
    int halfmaxelev = maxelev / 2;

    // First, make some clouds in a blobby sort of style.

    vector<int> blobbycloud(6 * eedge, 0);

    int iterations = 4;

    vector<fourintegers> totals(6 * eedge);

    for (int n = 0; n < iterations; n++)
    {
        vector<int> fractal(6 * edge * edge);

        int grain; //= pow(2, 4 + n); //4 * (n + 1); 4, 8, 12, 16 // Level of detail on this fractal map.

        if (n == 0)
            grain = 8; // 4;

        if (n == 1)
            grain = 16; // 8;

        if (n == 2)
            grain = 32; // 16;

        if (n == 3)
            grain = 64; // 32;

        float valuemod = 0.2f;
        float valuemod2 = 3.0f;

        createfractal(fractal, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

        // Now make it more ridged. We convert it to the range (-1, 1) (kind of) and then take the absolute value.

        for (int face = 0; face < 6; face++)
        {
            int vface = face * eedge;

            for (int i = 0; i < edge; i++)
            {
                int vi = vface + i * edge;

                for (int j = 0; j < edge; j++)
                {
                    int index = vi + j;

                    int val = fractal[index] - maxelev / 2;

                    if (val < 0)
                        val = abs(val);

                    val = val * 2;

                    val = maxelev - val;

                    if (n == 0)
                        totals[index].w = val;

                    if (n == 1)
                        totals[index].x = val;

                    if (n == 2)
                        totals[index].y = val;

                    if (n == 3)
                        totals[index].z = val;
                }
            }
        }
    }

    for (int face = 0; face < 6; face++) // Now sum up all those iterations, and square the result to exaggerate the peakiness.
    {
        int vface = face * eedge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                int val = totals[index].w + totals[index].x + totals[index].y + totals[index].z;

                val = val / iterations;

                float fval = (float)val;

                fval = fval / (float)maxelev;

                fval = fval * fval;

                fval = fval * (float)maxelev;

                blobbycloud[index] = (int)fval;
            }
        }
    }

    // Now we raise the values by a bit. First, find out the highest existing value.

    int currentmax = 0;

    for (int face = 0; face < 6; face++) // Raise the values a bit.
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                if (blobbycloud[index] > currentmax)
                    currentmax = blobbycloud[index];
            }
        }
    }

    float mult = (float)maxelev / (float)currentmax; // Multiply everything by this to make the highest value the maximum.

    mult = mult * 2.4f; // 2.2 // Just to make the effect more pronounced.

    vector<int> cloudmult(6 * eedge, 0); // This will determine how much we multiply the cloud values by, to make it varied.

    int grain = 8; // Level of detail on this fractal map.
    float valuemod = 0.2f;
    float valuemod2 = 3.0f;

    createfractal(cloudmult, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    for (int face = 0; face < 6; face++) // Raise the values a bit.
    {
        int vface = face * eedge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                float thismult = mult;

                thismult = thismult * ((float)cloudmult[index] / (float)maxelev);

                int newval = (int)((float)blobbycloud[index] * thismult);

                if (newval > maxelev)
                    newval = maxelev;

                blobbycloud[index] = newval;
            }
        }
    }

    /*
    // Now do some curl warping to that.

    vector<int> mounds(6 * eedge, 0);

    makemounds(edge, mounds, 4000, (float)maxelev);

    vector<int> curlvary(6 * eedge);

    grain = 8;
    valuemod = 0.2f;
    valuemod2 = 3.0f;

    createfractal(curlvary, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    // Now use that to create curl noise to warp our clouds.

    vector<int> final(6 * eedge, 0);

    curlwarp(edge, (float)maxelev, blobbycloud, mounds, curlvary, dirpoint);

    */

    // Now make some clouds in a simpler puffy style.

    vector<int> puffycloud(6 * eedge, 0);

    grain = edge / 4; // 128;
    valuemod = 0.2f;
    valuemod2 = 3.0f;

    createfractal(puffycloud, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    vector<int> tinypuffycloud(6 * eedge, 0); // This will make the first one fluffier.

    grain = edge / 2; // 256;
    valuemod = 0.2f;
    valuemod2 = 3.0f;

    createfractal(tinypuffycloud, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    for (int face = 0; face < 6; face++)
    {
        int vface = face * eedge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                if (tinypuffycloud[index] < puffycloud[index])
                    puffycloud[index] = (puffycloud[index] + tinypuffycloud[index]) / 2;
            }
        }
    }

    vector<int> bigpuffycloud(6 * eedge, 0); // This will vary the strength of the base pattern across the globe.

    grain = 16;
    valuemod = 0.2f;
    valuemod2 = 3.0f;

    createfractal(bigpuffycloud, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    for (int face = 0; face < 6; face++)
    {
        int vface = face * eedge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                if ((int)((float)bigpuffycloud[index] * 1.8f) < puffycloud[index]) // 1.4
                    puffycloud[index] = bigpuffycloud[index];
            }
        }
    }

    for (int n = 0; n < 1; n++) // Now warp it.
    {
        int warpfactorright = 10;
        int warpfactordown = 5;

        int warpdetail = 16;

        warp(puffycloud, edge, maxelev, warpfactorright, warpfactordown, warpdetail, 0, dirpoint);
    }

    // Now the integrated cloud pattern. We make this by merging the ones we've just done.

    vector<int> mergedcloud(6 * eedge, 0);

    vector<int> mergefractal(6 * eedge, 0); // This will work out the merging.

    grain = 8;
    valuemod = 0.2f;
    valuemod2 = 3.0f;

    createfractal(mergefractal, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    for (int face = 0; face < 6; face++) // Merge the patterns and copy to the final cloud pattern.
    {
        int vface = face * eedge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                float blobby = (float)mergefractal[index] / (float)maxelev;
                float puffy = 1.0f - blobby;

                blobby = blobby * (float)blobbycloud[index];
                puffy = puffy * (float)puffycloud[index];

                mergedcloud[index] = (int)(blobby + puffy);
            }
        }
    }

    // Now to add swirliness!

    vector<int> swirlfractal(6 * eedge, 0); // This will determine the strength of the swirliness.

    grain = edge / 64; // 4;
    valuemod = 0.2f;
    valuemod2 = 3.0f;

    createfractal(swirlfractal, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);
    
    vector<int> lengthfractal(6 * eedge, 0);
    vector<int> lengthfractal2(6 * eedge, 0);
    vector<int> lengthfractal3(6 * eedge, 0);

    grain = 8;
    valuemod = 0.2f;
    valuemod2 = 3.0f;

    createfractal(lengthfractal, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);
    
    grain = 4;
    valuemod = 0.2f;
    valuemod2 = 3.0f;

    createfractal(lengthfractal2, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);      
    createfractal(lengthfractal3, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    for (int face = 0; face < 6; face++) // Normalise and square these fractals, to make longer wisps rarer.
    {
        int vface = face * eedge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                float val = (float)lengthfractal2[index];
                val = val / (float)maxelev;
                val = val * val;
                val = val * (float)maxelev;
                lengthfractal2[index] = (int)val;

                val = (float)lengthfractal3[index];
                val = val / (float)maxelev;
                val = val * val * val;
                val = val * (float)maxelev;
                lengthfractal3[index] = (int)val;
            }
        }
    }

    float pi = 3.1415926535f;
    int maxpoints = 100000000;
    float maxlength = (float)(random(20, 80)); //60.0f;
    int minamount = (int)((float)maxelev * 0.40f);

    vector<bool> done(6 * eedge, 0);

    for (int currentpoint = 0; currentpoint < maxpoints; currentpoint++)
    {
        int face = random(0, 5);
        int x = random(0, edge - 1);
        int y = random(0, edge - 1);

        if (done[face * eedge + x * edge + y] == 0 && mergedcloud[face * eedge + x * edge + y] > minamount)
        {
            globepoint thispoint;
            thispoint.face = face;
            thispoint.x = x;
            thispoint.y = y;

            int index = face * eedge + x * edge + y;

            int thislength = lengthfractal[index];

            if (lengthfractal2[index] < thislength)
                thislength = lengthfractal2[index];

            if (lengthfractal3[index] < thislength)
                thislength = lengthfractal3[index];

            thislength = (int)(((float)thislength / (float)maxelev) * maxlength);

            float fx = (float)x + 0.5f;
            float fy = (float)y + 0.5f;

            float n = (float)(swirlfractal[index] - halfmaxelev);
            n = n / (float)halfmaxelev;

            float moveeast = cos(n * 2.0f * pi);
            float movesouth = sin(n * 2.0f * pi);

            int stepsdone = 0;
            bool keepgoing = 1;

            do
            {
                fx = fx + moveeast;
                fy = fy + movesouth;

                if ((int)fx != x || (int)fy != y)
                {
                    int oldcloud = mergedcloud[thispoint.face * eedge + thispoint.x * edge + thispoint.y];

                    int thismoveeast = (int)fx - x;
                    int thismovesouth = (int)fy - y;

                    bool gowest = 0;

                    if (thismoveeast < 0)
                    {
                        thismoveeast = -thismoveeast;
                        gowest = 1;
                    }

                    bool gonorth = 0;

                    if (thismovesouth < 0)
                    {
                        thismovesouth = -thismovesouth;
                        gonorth = 1;
                    }

                    for (int n = 0; n < thismoveeast; n++)
                    {
                        if (gowest)
                            thispoint = dirpoint[thispoint.face * eedge + thispoint.x * edge + thispoint.y].west;
                        else
                            thispoint = dirpoint[thispoint.face * eedge + thispoint.x * edge + thispoint.y].east;
                    }

                    for (int n = 0; n < thismovesouth; n++)
                    {
                        if (gonorth)
                            thispoint = dirpoint[thispoint.face * eedge + thispoint.x * edge + thispoint.y].north;
                        else
                            thispoint = dirpoint[thispoint.face * eedge + thispoint.x * edge + thispoint.y].south;
                    }

                    int newindex = thispoint.face * eedge + thispoint.x * edge + thispoint.y;

                    if (done[newindex] == 0)
                    {
                        int newcloud = mergedcloud[newindex];

                        float newvalue = (float)oldcloud * 0.99f + (float)newcloud * 0.01f;

                        mergedcloud[newindex] = (int)newvalue;

                        done[newindex] = 1;

                        x = (int)fx;
                        y = (int)fy;

                        n = (float)(swirlfractal[newindex] - halfmaxelev);
                        n = n / (float)halfmaxelev;

                        moveeast = cos(n * 2.0f * pi);
                        movesouth = sin(n * 2.0f * pi);

                        stepsdone++;

                        if (stepsdone > thislength)
                            keepgoing = 0;
                    }
                    else
                        keepgoing = 0;
                }

            } while (keepgoing);
        }
    }

    // Now smooth it, just slightly.

    vector<int> cloud(6 * eedge, 0);

    globepoint neighbours[3][3];

    for (int face = 0; face < 6; face++)
    {
        int vface = face * eedge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                neighbours[1][1].face = face;
                neighbours[1][1].x = i;
                neighbours[1][1].y = j;

                neighbours[1][0] = dirpoint[index].north;
                neighbours[1][2] = dirpoint[index].south;
                neighbours[0][1] = dirpoint[index].west;
                neighbours[2][1] = dirpoint[index].east;

                neighbours[0][0] = dirpoint[neighbours[1][0].face * eedge + neighbours[1][0].x * edge + neighbours[1][0].y].west;
                neighbours[2][0] = dirpoint[neighbours[1][0].face * eedge + neighbours[1][0].x * edge + neighbours[1][0].y].east;
                neighbours[0][2] = dirpoint[neighbours[1][2].face * eedge + neighbours[1][2].x * edge + neighbours[1][2].y].west;
                neighbours[2][2] = dirpoint[neighbours[1][2].face * eedge + neighbours[1][2].x * edge + neighbours[1][2].y].east;

                float total = 0.0f;

                for (int k = 0; k < 3; k++)
                {
                    for (int l = 0; l < 3; l++)
                        total = total + (float)mergedcloud[neighbours[k][l].face * eedge + neighbours[k][l].x * edge + neighbours[k][l].y];
                }

                //cloud[index] = mergedcloud[index];

                cloud[index] = (int)(((float)mergedcloud[index] * 3.0f + total / 9.0f) / 4.0f);
            }
        }
    }

    // Now add some rotations to the cloud map!

    if (world.size() > 0) // Don't do this with small worlds, as it doesn't look good.
    {
        vector<bool> doneswirls(6 * edge * edge, 0); // This will record where we've done them, so they don't overlap.

        int stormtotal = random(8, 24);
        float stormsteps = 10.0f;

        for (int thistorm = 0; thistorm < stormtotal; thistorm++)
        {
            int stormface = random(0, 5);
            int stormx = random(0, edge - 1);
            int stormy = random(0, edge - 1);
            int radius = random(edge / 25, edge / 10);

            bool OK = 1; // Check that this won't overlap with another one.

            int polemargin = edge / 6; // Can't get closer than this to the poles.

            for (int i = -radius; i <= radius; i++)
            {
                for (int j = -radius; j <= radius; j++)
                {
                    int ii = i;
                    int jj = j;

                    bool goleft = 0;
                    bool goup = 0;

                    if (ii < 0)
                    {
                        ii = 0 - ii;
                        goleft = 1;
                    }

                    if (jj < 0)
                    {
                        jj = 0 - jj;
                        goup = 1;
                    }

                    globepoint thispoint;
                    thispoint.face = stormface;
                    thispoint.x = stormx;
                    thispoint.y = stormy;

                    for (int n = 0; n < ii; n++)
                    {
                        if (goleft)
                            thispoint = dirpoint[thispoint.face * edge * edge + thispoint.x * edge + thispoint.y].west;
                        else
                            thispoint = dirpoint[thispoint.face * edge * edge + thispoint.x * edge + thispoint.y].east;
                    }

                    for (int n = 0; n < jj; n++)
                    {
                        if (goup)
                            thispoint = dirpoint[thispoint.face * edge * edge + thispoint.x * edge + thispoint.y].north;
                        else
                            thispoint = dirpoint[thispoint.face * edge * edge + thispoint.x * edge + thispoint.y].south;
                    }

                    if (doneswirls[thispoint.face * edge * edge + thispoint.x * edge + thispoint.y])
                    {
                        i = radius;
                        j = radius;
                        OK = 0;
                    }

                    if (stormface < 4 && thispoint.face < 4 && thispoint.face != stormface) // Don't let it cross from one side face to another as it looks weird.
                    {
                        i = radius;
                        j = radius;
                        OK = 0;
                    }

                    if (stormface < 4 && thispoint.face < 4) // Don't let it cross the equator.
                    {
                        if (stormy < edge / 2 && thispoint.y > edge / 2)
                        {
                            i = radius;
                            j = radius;
                            OK = 0;
                        }

                        if (stormy > edge / 2 && thispoint.y < edge / 2)
                        {
                            i = radius;
                            j = radius;
                            OK = 0;
                        }
                    }

                    if (thispoint.face > 3) // Don't let it get too close to the poles.
                    {
                        if ((i > edge / 2 - polemargin && i < edge / 2 + polemargin) || (j > edge / 2 - polemargin && j < edge / 2 + polemargin))
                        {
                            i = radius;
                            j = radius;
                            OK = 0;
                        }
                    }
                }
            }

            if (OK)
            {
                float twists = (float)random(2, 8) * 0.1f;

                if (stormface == 5 || (stormface != 4 && stormy > edge / 2)) // Southern hemisphere
                    twists = -twists;

                if (world.rotation() == 0) // Reverse rotation
                    twists = -twists;

                for (float n = 1.0f; n < stormsteps; n++)
                    makeswirl(cloud, edge, stormface, stormx, stormy, (int)(((float)radius / stormsteps) * thistorm), twists / stormsteps, doneswirls, dirpoint);
            }
        }

        // Now remove any weird lines in the swirls.

        int maxdiff = 4; // maxelev / 15;

        int northindex, northnorthindex, southindex, southsouthindex, eastindex, westindex, westwestindex;

        for (int face = 0; face < 6; face++)
        {
            int vface = face * edge * edge;

            for (int i = 1; i < edge - 1; i++)
            {
                int vi = vface + i * edge;

                for (int j = 1; j < edge - 1; j++)
                {
                    int index = vi + j;

                    bool nearswirl = 0;

                    globepoint northpoint = dirpoint[index].north;

                    northindex = northpoint.face * eedge + northpoint.x * edge + northpoint.y;

                    if (doneswirls[northindex])
                        nearswirl = 1;
                    else
                    {
                        globepoint northnorthpoint = dirpoint[northindex].north;

                        northnorthindex = northnorthpoint.face * eedge + northnorthpoint.x * edge + northnorthpoint.y;

                        if (doneswirls[northnorthindex])
                            nearswirl = 1;
                        else
                        {
                            globepoint southpoint = dirpoint[index].south;

                            southindex = southpoint.face * eedge + southpoint.x * edge + southpoint.y;

                            if (doneswirls[southindex])
                                nearswirl = 1;
                            else
                            {
                                globepoint southsouthpoint = dirpoint[southindex].south;

                                southsouthindex = southsouthpoint.face * eedge + southsouthpoint.x * edge + southsouthpoint.y;

                                if (doneswirls[southsouthindex])
                                    nearswirl = 1;
                                else
                                {
                                    globepoint westpoint = dirpoint[index].west;

                                    westindex = westpoint.face * eedge + westpoint.x * edge + westpoint.y;

                                    if (doneswirls[westindex])
                                        nearswirl = 1;
                                    else
                                    {
                                        globepoint westwestpoint = dirpoint[westindex].west;

                                        westwestindex = westwestpoint.face * eedge + westwestpoint.x * edge + westwestpoint.y;

                                        if (doneswirls[westwestindex])
                                            nearswirl = 1;
                                        else
                                        {
                                            globepoint eastpoint = dirpoint[index].east;

                                            eastindex = eastpoint.face * eedge + eastpoint.x * edge + eastpoint.y;

                                            if (doneswirls[eastindex])
                                                nearswirl = 1;
                                            else
                                            {
                                                globepoint easteastpoint = dirpoint[eastindex].east;

                                                if (doneswirls[easteastpoint.face * eedge + easteastpoint.x * edge + easteastpoint.y])
                                                    nearswirl = 1;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    if (nearswirl)
                    {
                        if (cloud[index - edge] > cloud[index] + maxdiff && cloud[index + edge] > cloud[index] + maxdiff)
                            cloud[index] = (cloud[index - edge] + cloud[index + edge]) / 2;

                        if (cloud[index - 1] > cloud[index] + maxdiff && cloud[index + 1] > cloud[index] + maxdiff)
                            cloud[index] = (cloud[index - 1] + cloud[index + 1]) / 2;

                        if (cloud[index - edge] < cloud[index] + maxdiff && cloud[index + edge] < cloud[index] + maxdiff)
                            cloud[index] = (cloud[index - edge] + cloud[index + edge]) / 2;

                        if (cloud[index - 1] < cloud[index] + maxdiff && cloud[index + 1] < cloud[index] + maxdiff)
                            cloud[index] = (cloud[index - 1] + cloud[index + 1]) / 2;
                    }
                }
            }
        }
    }

    // Now just put that in the world object.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
                world.setclouds(face, i, j, cloud[vi + j]);
        }
    }
}

