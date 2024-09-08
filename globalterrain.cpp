//
//
//  globalterrain.cpp
//  Undiscovered Worlds
//
//  Created by Jonathan Hill on 24/07/2019.
//
//  Please see functions.hpp for notes.

#include <iostream>
#include <cmath>
#include <fstream>
#include <stdio.h>
#include <queue>

#include "classes.hpp"
#include "planet.hpp"
#include "region.hpp"
#include "functions.hpp"
//#include "profiler.h"

using namespace std;

void generateglobalterrain(planet& world, bool customgenerate, int iterations, int mergefactor, int clusterno, int clustersize, boolshapetemplate landshape[], boolshapetemplate chainland[], vector<vector<vector<int>>>& mountaindrainage, vector<vector<vector<bool>>>& shelves, vector<float>& longitude, vector<float>& latitude, vector<fourglobepoints>& dirpoint, vector<int>& squareroot, int& progressval, string& progresstext)
{
    //highres_timer_t timer("Generate Global Terrain"); // 22.1s => 17.6s

    //generateglobalterraintype1(world, customgenerate, mergefactor, landshape, mountaindrainage, shelves, longitude, latitude, dirpoint, chainland, squareroot);

    //generateglobalterraintype2(world, customgenerate, mergefactor, clusterno, clustersize, landshape, mountaindrainage, shelves, longitude, latitude, dirpoint, chainland, squareroot);

    //generateglobalterraintype3(world, customgenerate, mergefactor, landshape, mountaindrainage, shelves, chainland);

    //generateglobalterraintype4(world, customgenerate, iterations, landshape, mountaindrainage, shelves, longitude, latitude, dirpoint, chainland, squareroot);

    // Work out positions of the sun for each month.

    calculatesunpositions(world);

    switch (world.type())
    {
    case 1:
        generateglobalterraintype1(world, customgenerate, mergefactor, landshape, mountaindrainage, shelves, longitude, latitude, dirpoint, chainland, squareroot, progressval, progresstext);
        break;

    case 2:
        generateglobalterraintype2(world, customgenerate, mergefactor, clusterno, clustersize, landshape, mountaindrainage, shelves, longitude, latitude, dirpoint, chainland, squareroot, progressval, progresstext);
        break;

    case 3:
        generateglobalterraintype3(world, customgenerate, mergefactor, landshape, mountaindrainage, shelves, chainland, progressval, progresstext);
        break;

    case 4:
        generateglobalterraintype4(world, customgenerate, iterations, landshape, mountaindrainage, shelves, longitude, latitude, dirpoint, chainland, squareroot, progressval, progresstext);
        break;
    }
}

// This creates type 1 terrain. This type gives a fairly chaotic looking map with small continents and lots of islands.

void generateglobalterraintype1(planet& world, bool customgenerate, int mergefactor, boolshapetemplate landshape[], vector<vector<vector<int>>>& mountaindrainage, vector<vector<vector<bool>>>& shelves, vector<float>& longitude, vector<float>& latitude, vector<fourglobepoints>& dirpoint, boolshapetemplate chainland[], vector<int>& squareroot, int& progressval, string& progresstext)
{
    // First get our key variables and clear the world.

    long seed = world.seed();
    fast_srand(seed);

    int edge = world.edge();
    int maxelev = world.maxelevation();
    int sealevel = world.sealevel();
    int baseheight = sealevel - 4500; //1250;
    if (baseheight < 1)
        baseheight = 1;
    int conheight = sealevel + 50;

    vector<vector<vector<int>>> plateaumap(6, vector<vector<int>>(edge, vector<int>(edge, 0)));
    vector<vector<vector<int>>> seafractal(6, vector<vector<int>>(edge, vector<int>(edge, 0)));
    vector<vector<vector<bool>>> removedland(6, vector<vector<bool>>(edge, vector<bool>(edge, 0)));
    vector<vector<vector<int>>> volcanodensity(6, vector<vector<int>>(edge, vector<int>(edge, 0)));
    vector<vector<vector<int>>> volcanodirection(6, vector<vector<int>>(edge, vector<int>(edge, 0)));

    world.clear(); // Clears all of the maps in this world.

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                world.setnom(face, i, j, 0);
                plateaumap[face][i][j] = 0;
                mountaindrainage[face][i][j] = 0;
                shelves[face][i][j] = 0;
                seafractal[face][i][j] = 0;
                removedland[face][i][j] = 0;
                volcanodensity[face][i][j] = 0;
                volcanodirection[face][i][j] = 0;
            }
        }
    }

    // Now start the generating.

    updatereport(progressval, progresstext, "Creating fractal map");

    // First make a fractal for noise (used only at regional map level).

    vector<vector<vector<int>>> fractal(6, vector<vector<int>>(edge, vector<int>(edge, 0)));

    int grain = 8; // Level of detail on this fractal map.
    float valuemod = 0.2f;
    float valuemod2 = 3.0f;

    createfractalold(fractal, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++) // Add a bit more variation to it.
        {
            for (int j = 0; j < edge; j++)
                fractal[face][i][j] = fractal[face][i][j] / 2;

        }
    }

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++) // Add a bit more variation to it.
        {
            for (int j = 0; j < edge; j++)
            {
                int adjust = randomsign(random(1, 8000));

                fractal[face][i][j] = fractal[face][i][j] + adjust; // The central tile gets it added twice.

                for (int k = - 1; k <= 1; k++) // Neighbouring tiles get it added once.
                {
                    for (int l = -1; l <= 1; l++)
                    {
                        globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                        if (lpoint.face != -1)
                            fractal[lpoint.face][lpoint.x][lpoint.y] = fractal[lpoint.face][lpoint.x][lpoint.y] + adjust;
                    }
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
                if (fractal[face][i][j] < 0)
                    fractal[face][i][j] = 0;

                if (fractal[face][i][j] > maxelev)
                    fractal[face][i][j] = maxelev;
            }
        }
    }

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
                world.setnoise(face, i, j, fractal[face][i][j]);
        }
    }

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
                fractal[face][i][j] = 0;
        }
    }

    // Now make a new one that we'll actually use for global terrain creation.

    createfractalold(fractal, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    updatereport(progressval, progresstext, "Creating continental map");

    smallcontinents(world, baseheight, conheight, fractal, plateaumap, landshape, chainland);

    createfractalold(fractal, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    // Merge the maps.

    updatereport(progressval, progresstext, "Merging maps");

    createfractalold(fractal, edge, grain, valuemod, valuemod2, 1, 12750, 0, 0);

    fractalmergemodified(world, mergefactor, fractal, removedland);

    createfractalold(fractal, edge, grain, valuemod, valuemod2, 1, 12750, 0, 0); // New fractal for the mountains.

    // Make continental shelves.

    updatereport(progressval, progresstext, "Making continental shelves");

    makecontinentalshelves(world, shelves, dirpoint, edge / 25, landshape); // Wider shelves than with the other terrain generator, because this one will produce lots of little bits of land

    // Now we add some island chains.

    twointegers focuspoints[4]; // These will be points where island chains will start close to.

    int focustotal = random(2, 4); // The number of actual focus points.

    for (int n = 0; n < focustotal; n++)
    {
        focuspoints[n].x = random(0, edge * 4 - 1);
        focuspoints[n].y = random(0, edge * 3 - 1);
    }

    int focaldistance = edge / 2; // Maximum distance a chain can start from the focuspoint.
    
    createchains(world, baseheight, conheight, fractal, landshape, chainland, focuspoints, focustotal, focaldistance, 3);

    updatereport(progressval, progresstext, "Smoothing map");

    createfractalold(fractal, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    world.smoothnom(1);

    // Now remove inland seas.

    updatereport(progressval, progresstext, "Removing inland seas");

    removeinlandseas(world, conheight);

    // Now widen any channels and lower the coasts.

    updatereport(progressval, progresstext, "Tidying up oceans");

    widenchannels(world);
    //loweroceans(world);

    // Now try to remove straight coastlines.

    updatereport(progressval, progresstext, "Improving coastlines");

   // removestraights(world);

    // Now sort out the sea depths.

    updatereport(progressval, progresstext, "Adjusting ocean depths");

    // We want to reduce shelves near the boundaries with face 5, where there are problems.
    // We use a fractal to make the reduction seem natural.

    vector<vector<vector<int>>> shelffractal(6, vector<vector<int>>(edge, vector<int>(edge, 0)));

    grain = 8; // Level of detail on this fractal map.
    valuemod = 0.2f;
    valuemod2 = 3.0f;

    createfractalold(shelffractal, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    for (int i = 0; i < edge; i++)
    {
        for (int j = 0; j < edge; j++)
            shelffractal[4][i][j] = maxelev;
    }

    float radius = ((float)edge / 4.0f) * 3.0f;

    for (int i = 0; i < edge; i++)
    {
        int ii = i - (edge / 2);

        for (int j = 0; j < edge; j++)
        {
            int jj = j - (edge / 2);

            float thisradius = (float)((ii * ii) + (jj * jj));

            if (thisradius <= (float)(MAXCRATERRADIUS * MAXCRATERRADIUS + MAXCRATERRADIUS) * 24)
                thisradius = (float)squareroot[(int)thisradius];
            else
                thisradius = (float)sqrt(thisradius);

            if (thisradius > radius)
                shelffractal[5][i][j] = 0;
            else
            {
                float factor = 1.0f - thisradius / radius;

                float add = (float)maxelev * 2.0f * factor;
                add = add - (float)maxelev;

                shelffractal[5][i][j] = shelffractal[5][i][j] + (int)add;

                if (shelffractal[5][i][j] < 0)
                    shelffractal[5][i][j] = 0;

                if (shelffractal[5][i][j] > maxelev)
                    shelffractal[5][i][j] = maxelev;
            }
        }
    }

    int topboundary = edge / 2;
    int middleboundary = edge - edge / 4;
    int bottomboundary = edge;

    for (int face = 0; face < 4; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < topboundary; j++)
                shelffractal[face][i][j] = maxelev;
        }
    }

    for (int face = 0; face < 4; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = topboundary; j < middleboundary; j++)
            {
                float dist1 = (float)(j - topboundary);
                float dist2 = (float)(middleboundary - j);

                float topfactor = dist2 / (dist1 + dist2);
                float middlefactor = dist1 / (dist1 + dist2);

                float val = (float)maxelev * topfactor + (float)shelffractal[face][i][j] * middlefactor;

                shelffractal[face][i][j] = (int)val;
            }
        }
    }

    for (int face = 0; face < 4; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = middleboundary; j < bottomboundary; j++)
            {
                float dist1 = (float)(j - middleboundary);
                float dist2 = (float)(bottomboundary - j);

                float bottomfactor = dist2 / (dist1 + dist2);

                float val = (float)shelffractal[face][i][j] * bottomfactor;

                shelffractal[face][i][j] = (int)val;
            }
        }
    }

    int mid = maxelev / 2;

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                int searchdist = random(1, 3);

                if (shelffractal[face][i][j] < mid && world.sea(face, i, j))
                {
                    bool found = 0;

                    for (int k = -searchdist; k <= searchdist; k++)
                    {
                        for (int l = -searchdist; l <= searchdist; l++)
                        {
                            globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                            if (lpoint.face != -1)
                            {
                                if (world.sea(lpoint.face, lpoint.x, lpoint.y) == 0)
                                {
                                    found = 1;
                                    k = searchdist;
                                    l = searchdist;
                                }
                            }
                        }
                    }

                    if (found == 0)
                        shelves[face][i][j] = 0;
                }
            }
        }
    }

    grain = 8; // Level of detail on this fractal map.
    valuemod = 0.2f;
    valuemod2 = 3.0f;

    createfractalold(fractal, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);
    createfractalold(seafractal, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    float coastalvarreduce = (float)maxelev / 500.0f; //3000;
    float oceanvarreduce = (float)maxelev / 1000.0f;

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.sea(face, i, j) == 1)
                {
                    if (shelves[face][i][j] == 1)
                    {
                        float var = (float)(seafractal[face][i][j] - maxelev / 2);
                        var = var / coastalvarreduce;

                        int newval = sealevel - 200 + (int)var;

                        if (newval > sealevel - 10)
                            newval = sealevel - 10;

                        if (newval < 1)
                            newval = 1;

                        world.setnom(face, i, j, newval);
                    }
                    else
                    {
                        float var = (float)(fractal[face][i][j] - maxelev / 2);
                        var = var / oceanvarreduce;

                        int newval = sealevel - 5000 + (int)var;

                        if (newval > sealevel - 3000)
                            newval = sealevel - 3000;

                        if (newval < 1)
                            newval = 1;

                        world.setnom(face, i, j, newval);
                    }
                }
            }
        }
    }

    if (random(1, 30) == 1)
    {
        // Now we create mid-ocean ridges.

        updatereport(progressval, progresstext, "Generating mid-ocean ridges");

        createoceanridges(world, shelves, longitude, latitude);

        ridgestomountains(world);

        // Now remove inland seas again.

        updatereport(progressval, progresstext, "Removing additional inland seas");

        removeinlandseas(world, conheight);

        // Now we create deep-sea trenches.

        updatereport(progressval, progresstext, "Generating deep-sea trenches");

        createoceantrenches(world, shelves);

        removebigstraights(world, baseheight, conheight, landshape);
    }

    // Now check for edge artefacts.

    removeedgeartefacts(world);

    // Now random volcanoes.

    updatereport(progressval, progresstext, "Generating volcanoes");

    grain = 8; // Level of detail on this fractal map.
    valuemod = 0.2f;
    valuemod2 = 3.0f;

    createfractalold(volcanodensity, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    grain = 4; // Level of detail on this fractal map.
    valuemod = 0.02f;
    valuemod2 = 3.0f;

    createfractalold(volcanodirection, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                bool goahead = 0;

                if (world.sea(face, i, j) == 1)
                {
                    int frac = volcanodensity[face][i][j];

                    int ii = i + edge / 2;

                    if (ii > edge)
                        ii = ii - edge;

                    int jj = j + edge / 2;

                    if (jj > edge)
                        jj = jj - edge;

                    int frac2 = volcanodensity[face][ii][j];
                    int frac3 = volcanodensity[face][i][jj];

                    int rand = 5000;

                    if (frac > (maxelev / 4) * 3)
                        rand = 80;

                    if (frac > (maxelev / 6) * 5)
                        rand = 40;

                    if (random(1, rand) == 1)
                        goahead = 1;
                }
                else
                {
                    if (random(1, 15000) == 1)
                        goahead = 1;
                }

                if (goahead == 1)
                {
                    bool strato = 1;

                    if (random(1, 10) == 1) // Shield volcanoes - much rarer.
                        strato = 0;

                    if (world.sea(face, i, j) == 1)
                        strato = 1;

                    int peakheight;

                    if (world.sea(face, i, j) == 1)
                    {
                        if (random(1, 10) == 1) // It could make a chain of volcanic islands.
                            peakheight = sealevel - world.nom(face, i, j) + random(500, 3000);
                        else
                            peakheight = sealevel - world.nom(face, i, j) - random(100, 200);

                        if (peakheight < 10)
                            peakheight = 10;

                        peakheight = random(peakheight / 2, peakheight);
                    }
                    else
                    {
                        if (strato == 1)
                            peakheight = random(2000, 6000);
                        else
                            peakheight = random(1000, 2000);
                    }

                    createisolatedvolcano(world, face, i, j, shelves, volcanodirection, peakheight, strato);
                }
            }
        }
    }

    // Now we add smaller mountain chains that cannot form peninsulas.

    updatereport(progressval, progresstext, "Adding smaller mountain ranges");

    twointegers dummy[1];

    createchains(world, baseheight, conheight, fractal, landshape, chainland, dummy, 0, 0, 2);

    // Now we add ranges of hills.

    updatereport(progressval, progresstext, "Adding hills");

    createchains(world, baseheight, conheight, fractal, landshape, chainland, dummy, 0, 0, 7);

    // Now we alter the fractal again, and use it to add more height variation.

    updatereport(progressval, progresstext, "Merging fractal into land");

    grain = 8; // Level of detail on this fractal map.
    valuemod = 0.2f;
    valuemod2 = 3.0f;

    createfractalold(fractal, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    fractalmergeland(world, fractal, conheight);

    getlandandseatotals(world);

    // Now remove any mountains that are over sea.

    updatereport(progressval, progresstext, "Removing floating mountains");

    removefloatingmountains(world);
    //cleanmountainridges(world);

    // Now we raise the mountain bases.

    if (customgenerate == 0) // Don't do this if this is a custom generation, as that will have the mountain bases raised later. (This is because the user might have changed the gravity after generating the terrain.)
    {
        updatereport(progressval, progresstext, "Raising mountain bases");

        vector<vector<vector<bool>>> mountainsOK(6, vector<vector<bool>>(edge, vector<bool>(edge, 0))); // This will track mountains that have been imported by the user and which therefore should not be altered to take account of gravity.

        raisemountainbases(world, mountaindrainage, mountainsOK);
    }

    // Now we smooth again, without changing the coastlines.

    updatereport(progressval, progresstext, "Smoothing map, preserving coastlines");

    smoothland(world, 2);

    // Now we create extra elevation over the land to create canyons.

    updatereport(progressval, progresstext, "Elevating land near canyons");

    createextraelev(world);

    // Now we remove depressions.

    updatereport(progressval, progresstext, "Filling depressions");

    depressionfill(world);

    addlandnoise(world); // Add a bit of noise, then do remove depressions again. This is to add variety to the river courses.

    depressionfill(world);

    // Now we adjust the land around coastlines.

    updatereport(progressval, progresstext, "Adjusting coastlines");

    normalisecoasts(world, 13, 11, 4);

    normalisecoasts(world, 13, 11, 4);

    clamp(world);

    // Now we note down one-tile islands.

    updatereport(progressval, progresstext, "Checking islands");

    checkislands(world);

    // Now we remove odd undersea bumps.

    updatereport(progressval, progresstext, "Flattening sea beds");

    removeunderseabumps(world);

    // Now we create a roughness map.

    updatereport(progressval, progresstext, "Creating roughness map");

    vector<vector<vector<int>>> roughness(6, vector<vector<int>>(edge, vector<int>(edge, 0)));

    grain = 8; // Level of detail on this fractal map.
    valuemod = 0.2f;
    valuemod2 = 3.0f;

    createfractalold(roughness, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
                world.setroughness(face, i, j, (float)roughness[face][i][j]);
        }
    }
}

// This creates type 2 terrain. This type gives a more earthlike map with large continents.

void generateglobalterraintype2(planet& world, bool customgenerate, int mergefactor, int clusterno, int clustersize, boolshapetemplate landshape[], vector<vector<vector<int>>>& mountaindrainage, vector<vector<vector<bool>>>& shelves, vector<float>& longitude, vector<float>& latitude, vector<fourglobepoints>& dirpoint, boolshapetemplate chainland[], vector<int>& squareroot, int& progressval, string& progresstext)
{
    // First get our key variables and clear the world.

    long seed = world.seed();
    fast_srand(seed);

    int edge = world.edge();
    int maxelev = world.maxelevation();
    int sealevel = world.sealevel();
    int baseheight = sealevel - 4500;
    if (baseheight < 1)
        baseheight = 1;
    int conheight = sealevel + 50;

    int maxmountainheight = maxelev - sealevel;

    vector<vector<vector<int>>> plateaumap(6, vector<vector<int>>(edge, vector<int>(edge, 0)));
    vector<vector<vector<int>>> seafractal(6, vector<vector<int>>(edge, vector<int>(edge, 0)));
    vector<vector<vector<bool>>> removedland(6, vector<vector<bool>>(edge, vector<bool>(edge, 0)));
    vector<vector<vector<int>>> volcanodensity(6, vector<vector<int>>(edge, vector<int>(edge, 0)));
    vector<vector<vector<int>>> volcanodirection(6, vector<vector<int>>(edge, vector<int>(edge, 0)));

    world.clear(); // Clears all of the maps in this world.

    /*
    // Try out a mounds map.

    vector<int> mounds(6 * edge * edge, 0);

    makemounds(edge, mounds, 4000, (float)maxelev);

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
                world.setnom(face, i, j, mounds[vi + j]);
        }
    }

    return;
    */

    /*
    // Make a fractal.

    vector<int> ffractal(6 * edge * edge, 0);

    //int grain = 4;
    //float valuemod = 0.003f;
    //float valuemod2 = 0.003f;

    int xgrain = 4; // Level of detail on this fractal map.
    float xvaluemod = 0.02f; // 0.03f;
    float xvaluemod2 = 0.02f; // 0.03f;

    createfractal(ffractal, edge, xgrain, xvaluemod, xvaluemod2, 1, maxelev, 0, 0);

    // And another one.

    //vector<int> warpfractal(6 * edge * edge, 0);

    //createfractal(warpfractal, edge, xgrain, xvaluemod, xvaluemod2, 1, maxelev, 0, 0);


    // Now make some mounds.

    vector<int> mounds(6 * edge * edge, 0);

    makemounds(edge, mounds, 4000, (float)maxelev);

    // Now use that to create curl noise to warp our fractal.

    vector<int> final(6 * edge * edge, 0);

    curlwarp(edge, (float)maxelev, ffractal, final, mounds, dirpoint);

    // Now apply that fractal to the world map.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
                world.setnom(face, i, j, final[vi + j]);
        }
    }

    return;
    */

    /*
    // Try out a voronoi map.

    vector<vector<vector<int>>> voronoi(6, vector<vector<int>>(edge, vector<int>(edge, 0)));

    int points = 10000;

    //makevoronoi(voronoi, edge, points);
    makeregularvoronoi(voronoi, edge, 4, 4, landshape);

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                fast_srand(voronoi[face][i][j]);
                int elev = random(1, maxelev);

                world.setnom(face, i, j, elev);
            }
        }
    }

    return;
    */

    // Now start the generating.

    /*
    for (int face = 0; face < 6; face++) // Make everything grey to start with.
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
                world.setnom(face, i, j, maxelev / 4);
        }
    }

    // Mark out the edges of the faces.

    for (int face = 0; face < 6; face++)
    {
        for (int n = 0; n < edge; n++)
        {
            world.setnom(face, n, 0, maxelev);
            world.setnom(face, n, edge - 1, maxelev);
            world.setnom(face, 0, n, maxelev);
            world.setnom(face, edge - 1, n, maxelev);
        }
    }

    int shapenumber = random(1, 11);

    drawshape(world, shapenumber, 1, edge / 2, edge - 1, 1, baseheight, conheight, landshape);

    return;
    */
    
    // First make a fractal for noise (used only at regional map level).

    vector<vector<vector<int>>> fractal(6, vector<vector<int>>(edge, vector<int>(edge, 0)));

    int grain = 8; // Level of detail on this fractal map.
    float valuemod = 0.2f;
    float valuemod2 = 3.0f;

    createfractalold(fractal, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
                world.setnoise(face, i, j, fractal[face][i][j]);
        }
    }

    // Now make a new one that we'll actually use for global terrain creation.

    createfractalold(fractal, edge, grain, valuemod, valuemod2, 1, 12750, 0, 0);

    int warpfactor = random(20, 80);
    warpold(fractal, edge, maxelev, warpfactor, warpfactor, 8, 1, dirpoint);

    int fractaladd = sealevel - 2500;

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
                fractal[face][i][j] = fractal[face][i][j] + fractaladd;
        }
    }

    updatereport(progressval, progresstext, "Creating continental map");

    largecontinents(world, baseheight, conheight, clusterno, clustersize, fractal, plateaumap, shelves, longitude, latitude, dirpoint, landshape, chainland, progressval, progresstext);

    // Now merge the maps.

    updatereport(progressval, progresstext, "Merging maps");

    createfractalold(fractal, edge, grain, valuemod, valuemod2, 1, 12750, 0, 0);

    fractalmergemodified(world, mergefactor, fractal, removedland);

    createfractalold(fractal, edge, grain, valuemod, valuemod2, 1, 12750, 0, 0); // New fractal for the mountains.

    // Now remove inland seas.

    updatereport(progressval, progresstext, "Removing inland seas");

    removeinlandseas(world, conheight);

    // Now widen any channels and lower the coasts.

    updatereport(progressval, progresstext, "Tidying up oceans");

    widenchannels(world);
    //loweroceans(world);

    // Now sort out the sea depths.

    updatereport(progressval, progresstext, "Adjusting ocean depths");

    // We want to reduce shelves near the boundaries with face 5, where there are problems.
    // We use a fractal to make the reduction seem natural.
        
    vector<vector<vector<int>>> shelffractal(6, vector<vector<int>>(edge, vector<int>(edge, 0)));

    grain = 8; // Level of detail on this fractal map.
    valuemod = 0.2f;
    valuemod2 = 3.0f;

    createfractalold(shelffractal, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    for (int i = 0; i < edge; i++)
    {
        for (int j = 0; j < edge; j++)
            shelffractal[4][i][j] = maxelev;
    }

    float radius = ((float)edge / 4.0f) * 3.0f;

    for (int i = 0; i < edge; i++)
    {
        int ii = i - (edge / 2);

        for (int j = 0; j < edge; j++)
        {
            int jj = j - (edge / 2);

            float thisradius = (float)((ii * ii) + (jj * jj));

            if (thisradius <= (float)(MAXCRATERRADIUS * MAXCRATERRADIUS + MAXCRATERRADIUS) * 24)
                thisradius = (float)squareroot[(int)thisradius];
            else
                thisradius = (float)sqrt(thisradius);

            if (thisradius > radius)
                shelffractal[5][i][j] = 0;
            else
            {
                float factor = 1.0f - thisradius / radius;

                float add = (float)maxelev * 2.0f * factor;
                add = add - (float)maxelev;

                shelffractal[5][i][j] = shelffractal[5][i][j] + (int)add;

                if (shelffractal[5][i][j] < 0)
                    shelffractal[5][i][j] = 0;

                if (shelffractal[5][i][j] > maxelev)
                    shelffractal[5][i][j] = maxelev;
            }
        }
    }

    int topboundary = edge / 2;
    int middleboundary = edge - edge / 4;
    int bottomboundary = edge;

    for (int face = 0; face < 4; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < topboundary; j++)
                shelffractal[face][i][j] = maxelev;
        }
    }

    for (int face = 0; face < 4; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = topboundary; j < middleboundary; j++)
            {
                float dist1 = (float)(j - topboundary);
                float dist2 = (float)(middleboundary - j);

                float topfactor = dist2 / (dist1 + dist2);
                float middlefactor = dist1 / (dist1 + dist2);

                float val = (float)maxelev * topfactor + (float)shelffractal[face][i][j] * middlefactor;

                shelffractal[face][i][j] = (int)val;
            }
        }
    }

    for (int face = 0; face < 4; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = middleboundary; j < bottomboundary; j++)
            {
                float dist1 = (float)(j - middleboundary);
                float dist2 = (float)(bottomboundary - j);

                float bottomfactor = dist2 / (dist1 + dist2);

                float val = (float)shelffractal[face][i][j] * bottomfactor;

                shelffractal[face][i][j] = (int)val;
            }
        }
    }

    int mid = maxelev / 2;

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                int searchdist = random(1, 3);
                    
                if (shelffractal[face][i][j] < mid && world.sea(face, i, j))
                {
                    bool found = 0;
                        
                    for (int k = -searchdist; k <= searchdist; k++)
                    {
                        for (int l = -searchdist; l <= searchdist; l++)
                        {
                            globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                            if (lpoint.face != -1)
                            {
                                if (world.sea(lpoint.face, lpoint.x, lpoint.y) == 0)
                                {
                                    found = 1;
                                    k = searchdist;
                                    l = searchdist;
                                }
                            }
                        }
                    }

                    if (found == 0)
                        shelves[face][i][j] = 0;
                }
            }
        }
    }

    grain = 8; // Level of detail on this fractal map.
    valuemod = 0.2f;
    valuemod2 = 3.0f;

    createfractalold(fractal, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);
    createfractalold(seafractal, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    float coastalvarreduce = (float)maxelev / 500.0f; //3000;
    float oceanvarreduce = (float)maxelev / 1000.0f;

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.sea(face,i, j) == 1)
                {
                    if (shelves[face][i][j] == 1)
                    {
                        float var = (float)(seafractal[face][i][j] - maxelev / 2);
                        var = var / coastalvarreduce;

                        int newval = sealevel - 200 + (int)var;

                        if (newval > sealevel - 10)
                            newval = sealevel - 10;

                        if (newval < 1)
                            newval = 1;

                        world.setnom(face,i, j, newval);
                    }
                    else
                    {
                        float var = (float)(fractal[face][i][j] - maxelev / 2);
                        var = var / oceanvarreduce;

                        int newval = sealevel - 5000 + (int)var;

                        if (newval > sealevel - 3000)
                            newval = sealevel - 3000;

                        if (newval < 1)
                            newval = 1;

                        world.setnom(face,i, j, newval);
                    }
                }
            }
        }
    }

    // Now we create mid-ocean ridges.

    updatereport(progressval, progresstext, "Generating mid-ocean ridges");

    createoceanridges(world, shelves, longitude, latitude);

    ridgestomountains(world);

    // Now we create deep-sea trenches.

    updatereport(progressval, progresstext, "Generating deep-sea trenches");

    createoceantrenches(world, shelves);

    // Now random volcanoes.

    updatereport(progressval, progresstext, "Generating volcanoes");

    grain = 8; // Level of detail on this fractal map.
    valuemod = 0.2f;
    valuemod2 = 3.0f;

    createfractalold(volcanodensity, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    grain = 4; // Level of detail on this fractal map.
    valuemod = 0.02f;
    valuemod2 = 3.0f;

    createfractalold(volcanodirection, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                bool goahead = 0;

                if (world.sea(face,i, j) == 1)
                {
                    int frac = volcanodensity[face][i][j];

                    int ii = i + edge / 2;

                    if (ii > edge)
                        ii = ii - edge;

                    int jj = j + edge / 2;

                    if (jj > edge)
                        jj = jj - edge;

                    int frac2 = volcanodensity[face][ii][j];
                    int frac3 = volcanodensity[face][i][jj];

                    int rand = 5000;

                    if (frac > (maxelev / 4) * 3)
                        rand = 80;

                    if (frac > (maxelev / 6) * 5)
                        rand = 40;

                    if (random(1, rand) == 1)
                        goahead = 1;
                }
                else
                {
                    if (random(1, 15000) == 1)
                        goahead = 1;
                }

                if (goahead == 1)
                {
                    bool strato = 1;

                    if (random(1, 10) == 1) // Shield volcanoes - much rarer.
                        strato = 0;

                    if (world.sea(face,i, j) == 1)
                        strato = 1;

                    int peakheight;

                    if (world.sea(face,i, j) == 1)
                    {
                        if (random(1, 10) == 1) // It could make a chain of volcanic islands.
                            peakheight = sealevel - world.nom(face, i, j) + random(500, 3000);
                        else
                            peakheight = sealevel - world.nom(face, i, j) - random(100, 200);

                        if (peakheight < 10)
                            peakheight = 10;

                        peakheight = random(peakheight / 2, peakheight);
                    }
                    else
                    {
                        if (strato == 1)
                            peakheight = random(2000, 6000);
                        else
                            peakheight = random(1000, 2000);
                    }

                    createisolatedvolcano(world, face, i, j, shelves, volcanodirection, peakheight, strato);
                }
            }
        }
    }
   
    // Now we add smaller mountain chains that cannot form peninsulas.

    updatereport(progressval, progresstext, "Adding smaller mountain ranges");

    twointegers dummy[1];

    createchains(world, baseheight, conheight, fractal, landshape, chainland, dummy, 0,0, 2);

    // Now we add ranges of hills.

    updatereport(progressval, progresstext, "Adding hills");

    createchains(world, baseheight, conheight, fractal, landshape, chainland, dummy, 0, 0, 7);

    // Now craters, rarely.

    if (random(1, 20) == 1)
    {
        updatereport(progressval, progresstext, "Bombarding world");

        vector<vector<vector<int>>> oldterrain(6, vector<vector<int>>(edge, vector<int>(edge, 0))); // Copy the terrain as it is before adding craters. This is so that we can add some variation to the craters afterwards, if there's sea on this world, so the depression filling doesn't fill them completely.

        for (int face = 0; face < 6; face++)
        {
            for (int i = 0; i < edge; i++)
            {
                for (int j = 0; j < edge; j++)
                    oldterrain[face][i][j] = world.nom(face, i, j);
            }
        }

        int cratertotal = random(5000, 10000);

        createcratermap(world, cratertotal, squareroot, 0);

        // Add some variation to how prominent the craters are. This will create some gaps in the craters so they don't get entirely filled up by the depression filling.

        vector<vector<vector<int>>> fractal(6, vector<vector<int>>(edge, vector<int>(edge, 0)));

        int grain = 8; // Level of detail on this fractal map.
        float valuemod = 0.2f;
        float valuemod2 = 3.0f;

        createfractalold(fractal, edge, grain, valuemod, valuemod2, 1, maxelev, 1, 0);

        for (int face = 0; face < 6; face++)
        {
            for (int i = 0; i < edge; i++)
            {
                for (int j = 0; j < edge; j++)
                {
                    float oldmult = (float)fractal[face][i][j] / (float)maxelev;
                    float newmult = 1.0f - oldmult;

                    float thiselev = (float)oldterrain[face][i][j] * oldmult + (float)world.nom(face, i, j) * newmult;

                    world.setnom(face, i, j, (int)thiselev);
                }
            }
        }

        // And get rid of any little crater seas.

        removeinlandseas(world, conheight);
    }

    getlandandseatotals(world);

    int landtotal = world.landtotal();
    int seatotal = world.seatotal();

    // Now we alter the fractal again, and use it to add more height variation.

    updatereport(progressval, progresstext, "Merging fractal into land");

    grain = 4; // Level of detail on this fractal map.
    valuemod = 0.02f;
    valuemod2 = 3.0f;

    createfractalold(fractal, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    fractalmergeland(world, fractal, conheight);

    // Now remove any mountains that are over sea.

    updatereport(progressval, progresstext, "Removing floating mountains");

    removefloatingmountains(world);
    //cleanmountainridges(world);

    // Now we raise the mountain bases.

    if (customgenerate == 0) // Don't do this if this is a custom generation, as that will have the mountain bases raised later. (This is because the user might have changed the gravity after generating the terrain.)
    {
        updatereport(progressval, progresstext, "Raising mountain bases");

        vector<vector<vector<bool>>> mountainsOK(6, vector<vector<bool>>(edge, vector<bool>(edge, 0))); // This will track mountains that have been imported by the user and which therefore should not be altered to take account of gravity.

        raisemountainbases(world, mountaindrainage, mountainsOK);
    }

    // Now we smooth again, without changing the coastlines.

    updatereport(progressval, progresstext, "Smoothing map, preserving coastlines");

    smoothland(world, 2);

    // Now we create extra elevation over the land to create canyons.

    updatereport(progressval, progresstext, "Elevating land near canyons");

    createextraelev(world);

    // Now check for edge artefacts.

    removeedgeartefacts(world);

    // Now we remove depressions.

    updatereport(progressval, progresstext, "Filling depressions");

    depressionfill(world);

    addlandnoise(world); // Add a bit of noise, then do remove depressions again. This is to add variety to the river courses.

    depressionfill(world);

    // Now we adjust the land around coastlines.

    updatereport(progressval, progresstext, "Adjusting coastlines");

    normalisecoasts(world, 13, 11, 4);

    normalisecoasts(world, 13, 11, 4);

    clamp(world);

    // Now we note down one-tile islands.

    updatereport(progressval, progresstext, "Checking islands");

    checkislands(world);

    // Now we remove odd undersea bumps.

    updatereport(progressval, progresstext, "Flattening sea beds");

    removeunderseabumps(world);

    // Now we create a roughness map.

    updatereport(progressval, progresstext, "Creating roughness map");

    vector<vector<vector<int>>> roughness(6, vector<vector<int>>(edge, vector<int>(edge, 0)));

    grain = 8; // Level of detail on this fractal map.
    valuemod = 0.2f;
    valuemod2 = 3.0f;

    createfractalold(roughness, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
                world.setroughness(face, i, j, (float)roughness[face][i][j]);
        }
    }
}

// This creates type 3 terrain. This is basically just ocean, so very simple.

void generateglobalterraintype3(planet& world, bool customgenerate, int mergefactor, boolshapetemplate landshape[], vector<vector<vector<int>>>& mountaindrainage, vector<vector<vector<bool>>>& shelves, boolshapetemplate chainland[], int& progressval, string& progresstext)
{
    // First get our key variables and clear the world.

    long seed = world.seed();
    fast_srand(seed);

    int size = world.size();
    int edge = world.edge();
    int maxelev = world.maxelevation();
    int sealevel = world.sealevel();
    int baseheight = sealevel - 4500; //1250;
    if (baseheight < 1)
        baseheight = 1;
    int conheight = sealevel + 50;

    int islandchance = 2; // The bigger the planet, the more likely islands are.

    if (size == 1)
        islandchance = 4;

    if (size == 0)
        islandchance = 10;

    vector<vector<vector<int>>> plateaumap(6, vector<vector<int>>(edge, vector<int>(edge, 0)));
    vector<vector<vector<int>>> seafractal(6, vector<vector<int>>(edge, vector<int>(edge, 0)));
    vector<vector<vector<bool>>> removedland(6, vector<vector<bool>>(edge, vector<bool>(edge, 0)));
    vector<vector<vector<int>>> volcanodensity(6, vector<vector<int>>(edge, vector<int>(edge, 0)));
    vector<vector<vector<int>>> volcanodirection(6, vector<vector<int>>(edge, vector<int>(edge, 0)));

    world.clear(); // Clears all of the maps in this world.

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                world.setnom(face, i, j, 0);
                plateaumap[face][i][j] = 0;
                mountaindrainage[face][i][j] = 0;
                shelves[face][i][j] = 0;
                seafractal[face][i][j] = 0;
                removedland[face][i][j] = 0;
                volcanodensity[face][i][j] = 0;
                volcanodirection[face][i][j] = 0;
            }
        }
    }

    // Now start the generating.

    updatereport(progressval, progresstext, "Creating fractal map");

    vector<vector<vector<int>>> fractal(6, vector<vector<int>>(edge, vector<int>(edge, 0)));

    int grain = 8; // Level of detail on this fractal map.
    float valuemod = 0.2f;
    float valuemod2 = 3.0f;

    if (random(1, 100) == 1) // This has weird results, so use it very sparingly!
    {
        createfractalold(fractal, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

        for (int face = 0; face < 6; face++)
        {
            for (int i = 0; i < edge; i++)
            {
                for (int j = 0; j < edge; j++)
                    world.setnoise(face, i, j, fractal[face][i][j]);
            }
        }

        // Now make a new one that we'll actually use for global terrain creation.

        createfractalold(fractal, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

        // Merge the maps.

        updatereport(progressval, progresstext, "Merging maps");

        fractalmerge(world, mergefactor, fractal);
    }

    // Now we add some island chains, perhaps.

    if (random(1, islandchance) == 1)
    {
        twointegers focuspoints[4]; // These will be points where island chains will start close to.

        int focustotal = random(2, 4); // The number of actual focus points.

        for (int n = 0; n < focustotal; n++)
        {
            focuspoints[n].x = random(0, edge * 4 - 1);
            focuspoints[n].y = random(0, edge * 3 - 1);
        }

        int focaldistance = edge / 2; // Maximum distance a chain can start from the focuspoint.

        createchains(world, baseheight, conheight, fractal, landshape, chainland, focuspoints, focustotal, focaldistance, 3);
    }

    updatereport(progressval, progresstext, "Shifting fractal");

    createfractalold(fractal, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    updatereport(progressval, progresstext, "Smoothing map");

    world.smoothnom(1);

    // Now remove inland seas.

    updatereport(progressval, progresstext, "Removing inland seas");

    removeinlandseas(world, conheight);

    // Now widen any channels and lower the coasts.

    updatereport(progressval, progresstext, "Tidying up oceans");

    widenchannels(world);
    //loweroceans(world);

    // Now try to remove straight coastlines.

    updatereport(progressval, progresstext, "Improving coastlines");

    //removestraights(world);

    // Now sort out the sea depths.

    updatereport(progressval, progresstext, "Adjusting ocean depths");

    grain = 8; // Level of detail on this fractal map.
    valuemod = 0.2f;
    valuemod2 = 3.0f;

    createfractalold(fractal, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);
    createfractalold(seafractal, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    float coastalvarreduce = (float)maxelev / 500.0f; //3000;
    float oceanvarreduce = (float)maxelev / 1000.0f;

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.sea(face, i, j) == 1)
                {
                    if (shelves[face][i][j] == 1)
                    {
                        float var = (float)(seafractal[face][i][j] - maxelev / 2);
                        var = var / coastalvarreduce;

                        int newval = sealevel - 200 + (int)var;

                        if (newval > sealevel - 10)
                            newval = sealevel - 10;

                        if (newval < 1)
                            newval = 1;

                        world.setnom(face, i, j, newval);
                    }
                    else
                    {
                        float var = (float)(fractal[face][i][j] - maxelev / 2);
                        var = var / oceanvarreduce;

                        int newval = sealevel - 5000 + (int)var;

                        if (newval > sealevel - 3000)
                            newval = sealevel - 3000;

                        if (newval < 1)
                            newval = 1;

                        world.setnom(face, i, j, newval);
                    }
                }
            }
        }
    }

    getlandandseatotals(world);

    // Now we alter the fractal again, and use it to add more height variation.

    updatereport(progressval, progresstext, "Merging fractal into land");

    createfractalold(fractal, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    fractalmergeland(world, fractal, conheight);

    // Now check for edge artefacts.

    removeedgeartefacts(world);

    // Now random volcanoes.

    updatereport(progressval, progresstext, "Generating volcanoes");

    grain = 8; // Level of detail on this fractal map.
    valuemod = 0.2f;
    valuemod2 = 3.0f;

    createfractalold(volcanodensity, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    grain = 4; // Level of detail on this fractal map.
    valuemod = 0.02f;
    valuemod2 = 3.0f;

    createfractalold(volcanodirection, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                bool goahead = 0;

                if (world.sea(face, i, j) == 1)
                {
                    int frac = volcanodensity[face][i][j];

                    int ii = i + edge / 2;

                    if (ii > edge)
                        ii = ii - edge;

                    int jj = j + edge / 2;

                    if (jj > edge)
                        jj = jj - edge;

                    int frac2 = volcanodensity[face][ii][j];
                    int frac3 = volcanodensity[face][i][jj];

                    int rand = 5000;

                    if (frac > (maxelev / 4) * 3)
                        rand = 80;

                    if (frac > (maxelev / 6) * 5)
                        rand = 40;

                    if (random(1, rand) == 1)
                        goahead = 1;
                }
                else
                {
                    if (random(1, 15000) == 1)
                        goahead = 1;
                }

                if (goahead == 1)
                {
                    bool strato = 1;

                    if (random(1, 10) == 1) // Shield volcanoes - much rarer.
                        strato = 0;

                    if (world.sea(face, i, j) == 1)
                        strato = 1;

                    int peakheight;

                    if (world.sea(face, i, j) == 1)
                    {
                        if (random(1, 10) == 1) // It could make a chain of volcanic islands.
                            peakheight = sealevel - world.nom(face, i, j) + random(500, 3000);
                        else
                            peakheight = sealevel - world.nom(face, i, j) - random(100, 200);

                        if (peakheight < 10)
                            peakheight = 10;

                        peakheight = random(peakheight / 2, peakheight);
                    }
                    else
                    {
                        if (strato == 1)
                            peakheight = random(2000, 6000);
                        else
                            peakheight = random(1000, 2000);
                    }

                    createisolatedvolcano(world, face, i, j, shelves, volcanodirection, peakheight, strato);
                }
            }
        }
    }

    getlandandseatotals(world);

    // Now remove any mountains that are over sea.

    updatereport(progressval, progresstext, "Removing floating mountains");

    removefloatingmountains(world);
    //cleanmountainridges(world);

    // Now we raise the mountain bases.

    if (customgenerate == 0) // Don't do this if this is a custom generation, as that will have the mountain bases raised later. (This is because the user might have changed the gravity after generating the terrain.)
    {
        updatereport(progressval, progresstext, "Raising mountain bases");

        vector<vector<vector<bool>>> mountainsOK(6, vector<vector<bool>>(edge, vector<bool>(edge, 0))); // This will track mountains that have been imported by the user and which therefore should not be altered to take account of gravity.

        raisemountainbases(world, mountaindrainage, mountainsOK);
    }

    // Now we smooth again, without changing the coastlines.

    updatereport(progressval, progresstext, "Smoothing map, preserving coastlines");

    smoothland(world, 2);

    // Now we create extra elevation over the land to create canyons.

    updatereport(progressval, progresstext, "Elevating land near canyons");

    createextraelev(world);

    // Now we remove depressions.

    updatereport(progressval, progresstext, "Filling depressions");

    depressionfill(world);

    addlandnoise(world); // Add a bit of noise, then do remove depressions again. This is to add variety to the river courses.

    depressionfill(world);

    // Now we adjust the land around coastlines.

    updatereport(progressval, progresstext, "Adjusting coastlines");

    normalisecoasts(world, 13, 11, 4);

    normalisecoasts(world, 13, 11, 4);

    clamp(world);

    // Now we note down one-tile islands.

    updatereport(progressval, progresstext, "Checking islands");

    checkislands(world);

    // Now we remove odd undersea bumps.

    updatereport(progressval, progresstext, "Flattening sea beds");

    removeunderseabumps(world);

    // Now we create a roughness map.

    updatereport(progressval, progresstext, "Creating roughness map");

    vector<vector<vector<int>>> roughness(6, vector<vector<int>>(edge, vector<int>(edge, 0)));

    grain = 8; // Level of detail on this fractal map.
    valuemod = 0.2f;
    valuemod2 = 3.0f;

    createfractalold(roughness, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    vector<vector<vector<int>>> roughness2(6, vector<vector<int>>(edge, vector<int>(edge, 0)));

    createfractalold(roughness2, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
                world.setroughness(face, i, j, (float)roughness[face][i][j]);
        }
    }

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++) // Do two, so that it's rare to get areas that are very smooth.
        {
            for (int j = 0; j < edge; j++)
            {
                if (roughness[face][i][j] < roughness2[face][i][j])
                    roughness[face][i][j] = roughness2[face][i][j];
            }
        }
    }
}

// This creates type 4 terrain. This is a fractal-based terrain for alien-looking worlds.

void generateglobalterraintype4(planet& world, bool customgenerate, int iterations, boolshapetemplate landshape[], vector<vector<vector<int>>>& mountaindrainage, vector<vector<vector<bool>>>& shelves, vector<float>& longitude, vector<float>& latitude, vector<fourglobepoints>& dirpoint, boolshapetemplate chainland[], vector<int>& squareroot, int& progressval, string& progresstext)
{
    // First get our key variables and clear the world.

    long seed = world.seed();
    fast_srand(seed);

    int edge = world.edge();
    int maxelev = world.maxelevation();
    int sealevel = world.sealevel();
    float gravity = world.gravity();

    int maxelevabovesea = maxelev - sealevel;

    vector<vector<vector<int>>> terrain(6, vector<vector<int>>(edge, vector<int>(edge, 0)));
    vector<vector<vector<int>>> broad(6, vector<vector<int>>(edge, vector<int>(edge, 0)));
    vector<vector<vector<int>>> plateaumap(6, vector<vector<int>>(edge, vector<int>(edge, 0)));
    vector<vector<vector<int>>> seafractal(6, vector<vector<int>>(edge, vector<int>(edge, 0)));
    vector<vector<vector<bool>>> removedland(6, vector<vector<bool>>(edge, vector<bool>(edge, 0)));
    vector<vector<vector<int>>> volcanodensity(6, vector<vector<int>>(edge, vector<int>(edge, 0)));
    vector<vector<vector<int>>> volcanodirection(6, vector<vector<int>>(edge, vector<int>(edge, 0)));

    world.clear(); // Clears all of the maps in this world.

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                world.setnom(face, i, j, 0);
                plateaumap[face][i][j] = 0;
                mountaindrainage[face][i][j] = 0;
                shelves[face][i][j] = 0;
                seafractal[face][i][j] = 0;
                removedland[face][i][j] = 0;
                volcanodensity[face][i][j] = 0;
                volcanodirection[face][i][j] = 0;
            }
        }
    }

    // Now start the generating.

    updatereport(progressval, progresstext, "Creating fractal map");

    // First make a fractal for noise (used only at regional map level).

    vector<vector<vector<int>>> fractal(6, vector<vector<int>>(edge, vector<int>(edge, 0)));

    int grain = 8; // Level of detail on this fractal map.
    float valuemod = 0.2f;
    float valuemod2 = 3.0f;

    createfractalold(fractal, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
                world.setnoise(face, i, j, fractal[face][i][j]);
        }
    }

    // Now create the basic terrain.

    updatereport(progressval, progresstext, "Creating basic terrain");

    int v = random(1, 100);

    if (random(1, 3) == 1)
        v = random(70, 100);

    float landscale = (float)v / 100.0f;

    makebasicterrain(world, landscale, terrain, dirpoint, landshape, 1);

    sealevel = world.sealevel();

    // Now apply the terrain to the world.

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
                world.setnom(face, i, j, terrain[face][i][j]);
        }
    }

    updatereport(progressval, progresstext, "Smoothing map");

    world.smoothnom(1);

    // Now do further iterations of the land terrain.

    if (iterations > 1)
    {
        updatereport(progressval, progresstext, "Complicating terrain");

        for (int n = 0; n < iterations - 1; n++)
        {
            // First create the broad array.

            int grain = 8; // Level of detail on this fractal map.
            float valuemod = 0.2f;
            float valuemod2 = 3.0f;

            grain = 4;
            valuemod = 0.01f;
            valuemod2 = 0.01f;
            createfractalold(broad, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

            int change = random(25, 200); // Now multiply that whole fractal by a random amount, so effectively the whole effect will be multiplied by this amount.
            float fchange = (float)change / 100.0f;

            for (int face = 0; face < 6; face++)
            {
                for (int i = 0; i < edge; i++)
                {
                    for (int j = 0; j < edge; j++)
                    {
                        float val = (float)broad[face][i][j];
                        val = val * fchange;

                        if (val > (float)maxelev)
                            val = (float)maxelev;

                        broad[face][i][j] = (int)val;
                    }
                }
            }

            // Now make new terrain and blend it into the existing terrain.
            // We only blend it into areas above sea level, so we retain the existing land/sea areas. Otherwise the map would become too jumbled.

            vector<vector<vector<int>>> newterrain(6, vector<vector<int>>(edge, vector<int>(edge, 0)));

            v = random(1, 100);

            if (random(1, 3) != 1)
                v = random(70, 100);

            landscale = (float)v / 100.0f;

            //landscale = (float)(random(1, 40)); // This *exaggerates* land heights!

            makebasicterrain(world, landscale, newterrain, dirpoint, landshape, 0);

            for (int face = 0; face < 6; face++)
            {
                for (int i = 0; i < edge; i++)
                {
                    for (int j = 0; j < edge; j++)
                    {
                        if (world.nom(face, i, j) > sealevel)
                        {
                            float oldval = (float)world.nom(face, i, j);

                            float newval = (float)newterrain[face][i][j];

                            float newmult = (float)broad[face][i][j] / (float)maxelev;
                            float oldmult = 1.0f - newmult;

                            newval = oldval * oldmult + newval * newmult;

                            if ((int)newval > sealevel)
                                world.setnom(face, i, j, (int)newval);
                        }
                    }
                }
            }
        }
    }

    // Now blur in another terrain map to smooth over the awkward face boundaries.

    updatereport(progressval, progresstext, "Improving terrain");

    blurinterrain(world, dirpoint, landshape);

    // Now change the sea level, possibly.

    //sealevel = 1;
    //world.setsealevel(sealevel);

    if (random(1, 10) == 1)
    {
        updatereport(progressval, progresstext, "Improving sea levels");

        adjustsealevel(world);

        sealevel = world.sealevel();
        world.setsealevel(sealevel);
    }

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                int thisnom = world.nom(face, i, j);

                if (thisnom == sealevel)
                    world.setnom(face, i, j, sealevel + 1);             
            }
        }
    }

    // Now remove small seas.

    updatereport(progressval, progresstext, "Removing small seas");

    float minseaproportion = (float)random(1, 500);
    minseaproportion = minseaproportion / 1000.0f;

    int minseasize = (int)(((float)edge * (float)edge) * minseaproportion);

    if (random(1, 3) == 1)
        removeinlandseas(world, sealevel + 1);
    else
        removesmallseas(world, minseasize, sealevel + 1);

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.nom(face, i, j) == sealevel || world.nom(face, i, j) == sealevel - 1)
                    world.setnom(face, i, j, sealevel + 1);
            }
        }
    }

    // Now widen any channels and lower the coasts.

    updatereport(progressval, progresstext, "Tidying up oceans");

    widenchannels(world);
    //loweroceans(world);

    // Now try to remove straight coastlines.

    updatereport(progressval, progresstext, "Improving coastlines");

    //removestraights(world);

    // Now do some smoothing, probably. (Can add artefacts at face edges...)

    if (random(1, 4) != 1)
    {
        updatereport(progressval, progresstext, "Smoothing terrain");

        int smoothamount = random(1, 4);

        //smoothonlylandvariable(world, smoothamount);
    }

    // Now check for edge artefacts.

    removeedgeartefacts(world);

    getlandandseatotals(world);

    // Now perhaps add some ranges of hills.

    if (random(1, 20) == 1)
    {
        updatereport(progressval, progresstext, "Adding hills");

        twointegers dummy[1];

        int baseheight = sealevel - 4500;

        if (baseheight < 1)
            baseheight = 1;

        int conheight = sealevel + 50;

        if (conheight > maxelev)
            conheight = maxelev;

        for (int n = 0; n < random(1, 4); n++)
        {
            int mode = 7;

            if (random(1, 8) == 1)
                mode = 2;

            createchains(world, baseheight, conheight, fractal, landshape, chainland, dummy, 0, 0, mode);
        }

        updatereport(progressval, progresstext, "Removing floating mountains");

        removefloatingmountains(world);
        //cleanmountainridges(world);
    }

    // Now perhaps add some mysterious channels.

    if (random(1, 8) == 1)
    {
        updatereport(progressval, progresstext, "Inscribing channels");

        //createchannels(world);
    }

    // Now random volcanoes.

    updatereport(progressval, progresstext, "Generating volcanoes");

    grain = 8; // Level of detail on this fractal map.
    valuemod = 0.2f;
    valuemod2 = 3.0f;

    createfractalold(volcanodensity, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    grain = 4; // Level of detail on this fractal map.
    valuemod = 0.02f;
    valuemod2 = 3.0f;

    createfractalold(volcanodirection, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                bool goahead = 0;

                if (world.sea(face, i, j) == 1)
                {
                    int frac = volcanodensity[face][i][j];

                    int ii = i + edge / 2;

                    if (ii > edge)
                        ii = ii - edge;

                    int jj = j + edge / 2;

                    if (jj > edge)
                        jj = jj - edge;

                    int frac2 = volcanodensity[face][ii][j];
                    int frac3 = volcanodensity[face][i][jj];

                    int rand = 5000;

                    if (frac > (maxelev / 4) * 3)
                        rand = 80;

                    if (frac > (maxelev / 6) * 5)
                        rand = 40;

                    if (random(1, rand) == 1)
                        goahead = 1;
                }
                else
                {
                    if (random(1, 15000) == 1)
                        goahead = 1;
                }

                if (goahead == 1)
                {
                    bool strato = 1;

                    if (random(1, 10) == 1) // Shield volcanoes - much rarer.
                        strato = 0;

                    if (world.sea(face, i, j) == 1)
                        strato = 1;

                    int peakheight;

                    if (world.sea(face, i, j) == 1)
                    {
                        if (random(1, 10) == 1) // It could make a chain of volcanic islands.
                            peakheight = sealevel - world.nom(face, i, j) + random(500, 3000);
                        else
                            peakheight = sealevel - world.nom(face, i, j) - random(100, 200);

                        if (peakheight < 10)
                            peakheight = 10;

                        peakheight = random(peakheight / 2, peakheight);
                    }
                    else
                    {
                        if (strato == 1)
                            peakheight = random(2000, 6000);
                        else
                            peakheight = random(1000, 2000);
                    }

                    createisolatedvolcano(world, face, i, j, shelves, volcanodirection, peakheight, strato);
                }
            }
        }
    }

    // Now craters.

    if (random(1, 2) == 1)
    {
        updatereport(progressval, progresstext, "Bombarding world");

        vector<vector<vector<int>>> oldterrain(6, vector<vector<int>>(edge, vector<int>(edge, 0))); // Copy the terrain as it is before adding craters. This is so that we can add some variation to the craters afterwards, if there's sea on this world, so the depression filling doesn't fill them completely.

        for (int face = 0; face < 6; face++)
        {
            for (int i = 0; i < edge; i++)
            {
                for (int j = 0; j < edge; j++)
                    oldterrain[face][i][j] = world.nom(face, i, j);
            }
        }

        int cratertotal = random(05000, 800000);

        createcratermap(world, cratertotal, squareroot, 0);

        if (random(1, 4) != 1) // That may have produced seas inside craters, so probably remove those now.
            removesmallseas(world, minseasize, sealevel + 1);
        else // Definitely remove any really little bits of sea.
            removesmallseas(world, 20, sealevel + 1);

        int totalsea = 0;

        for (int face = 0; face < 6; face++)
        {
            for (int i = 0; i < edge; i++)
            {
                for (int j = 0; j < edge; j++)
                {
                    if (world.sea(face, i, j))
                        totalsea++;
                }
            }
        }

        if (totalsea > 40) // If there's sea, we must fill depressions.
        {
            // First, add some variation to how prominent the craters are. This will create some gaps in the craters so they don't get entirely filled up by the depression filling.

            vector<vector<vector<int>>> fractal(6, vector<vector<int>>(edge, vector<int>(edge, 0)));

            int grain = 8; // Level of detail on this fractal map.
            float valuemod = 0.2f;
            float valuemod2 = 3.0f;

            createfractalold(fractal, edge, grain, valuemod, valuemod2, 1, maxelev, 1, 0);

            for (int face = 0; face < 6; face++)
            {
                for (int i = 0; i < edge; i++)
                {
                    for (int j = 0; j < edge; j++)
                    {
                        float oldmult = (float)fractal[face][i][j] / (float)maxelev;
                        float newmult = 1.0f - oldmult;

                        float thiselev = (float)oldterrain[face][i][j] * oldmult + (float)world.nom(face, i, j) * newmult;

                        world.setnom(face, i, j, (int)thiselev);
                    }
                }
            }

            // Now fill the depressions.

            depressionfill(world);

            addlandnoise(world); // Add a bit of noise, then do remove depressions again. This is to add variety to the river courses.

            depressionfill(world);
        }
    }

    // If there's almost no sea, get rid of what sea there is.

    int totalsea = 0;

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.sea(face, i, j))
                    totalsea++;
            }
        }
    }

    if (totalsea > 0 && totalsea < 100)
    {
        for (int face = 0; face < 6; face++)
        {
            for (int i = 0; i < edge; i++)
            {
                for (int j = 0; j < edge; j++)
                {
                    if (world.sea(face, i, j))
                        world.setnom(face, i, j, sealevel + random(2, 10));
                }
            }
        }
    }

    // Now we smooth again, without changing the coastlines.

    if (random(1, 2) == 1)
    {
        updatereport(progressval, progresstext, "Smoothing map, preserving coastlines");

        smoothland(world, 2);
    }

    // Now we raise the mountain bases.

    if (customgenerate == 0) // Don't do this if this is a custom generation, as that will have the mountain bases raised later. (This is because the user might have changed the gravity after generating the terrain.)
    {
        updatereport(progressval, progresstext, "Raising mountain bases");

        vector<vector<vector<bool>>> mountainsOK(6, vector<vector<bool>>(edge, vector<bool>(edge, 0))); // This will track mountains that have been imported by the user and which therefore should not be altered to take account of gravity.

        raisemountainbases(world, mountaindrainage, mountainsOK);
    }

    // Now we create extra elevation over the land to create canyons.

    updatereport(progressval, progresstext, "Elevating land near canyons");

    createextraelev(world);

    // Now we remove depressions (unless there is no sea, in which case we'll do it in the climate section).

    getlandandseatotals(world);

    int seatotal = world.seatotal();

    if (seatotal > 0)
    {
        updatereport(progressval, progresstext, "Filling depressions");

        depressionfill(world);

        for (int n = 0; n < random(4, 8); n++)
            addlandnoise(world); // Add a bit of noise, then do remove depressions again. This is to add variety to the river courses.

        depressionfill(world);
    }

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.nom(face, i, j) == sealevel)
                    world.setnom(face, i, j, sealevel + 1);
            }
        }
    }

    // Now we adjust the land around coastlines.

    updatereport(progressval, progresstext, "Adjusting coastlines");

    for (int n = 1; n <= 2; n++)
        normalisecoasts(world, 13, 11, 4);

    clamp(world);

    // Now we note down one-tile islands.

    updatereport(progressval, progresstext, "Checking islands");

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                int thisnom = world.nom(face, i, j);

                if (thisnom == sealevel)
                    world.setnom(face, i, j, sealevel + 1);
            }
        }
    }

    checkislands(world);

    // Now we create a roughness map.

    updatereport(progressval, progresstext, "Creating roughness map");

    vector<vector<vector<int>>> roughness(6, vector<vector<int>>(edge, vector<int>(edge, 0)));

    grain = 8; // Level of detail on this fractal map.
    valuemod = 0.2f;
    valuemod2 = 3.0f;

    createfractalold(roughness, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    vector<vector<vector<int>>> roughness2(6, vector<vector<int>>(edge, vector<int>(edge, 0)));

    createfractalold(roughness2, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
                world.setroughness(face, i, j, (float)roughness[face][i][j]);
        }
    }

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++) // Do two, so that it's rare to get areas that are very smooth.
        {
            for (int j = 0; j < edge; j++)
            {
                if (roughness[face][i][j] < roughness2[face][i][j])
                    roughness[face][i][j] = roughness2[face][i][j];
            }
        }
    }
}

// This function creates a fractal map on a vector that could be applied to the cubesphere.

void createfractal(vector<int>& arr, int edge, int grain, float valuemod, float valuemod2, int min, int max, bool extreme, bool wrapped)
{
    // First, we need to make one long fractal for faces 0-3, all in one go.

    int longwidth = edge * 4;
    int longheight = edge;

    vector<int> longfractal(longwidth * longheight, 0);

    newfractalinit(longfractal, edge, longwidth - 1, longheight - 1, grain, min, max, extreme);    
    newfractal(longfractal, edge, longwidth - 1, longheight - 1, grain, valuemod, valuemod2, min, max, wrapped);

    // Apply that to faces 0-3.

    int f1 = edge * edge;
    int f2 = edge * edge * 2;
    int f3 = edge * edge * 3;
    int f4 = edge * edge * 4;
    int f5 = edge * edge * 5;

    for (int i = 0; i < edge; i++)
    {
        int vi = i * edge;

        for (int j = 0; j < edge; j++)
        {
            int vj = vi + j;

            arr[vj] = longfractal[i * edge + j];
            arr[f1 + vj] = longfractal[(i + edge) * edge + j];
            arr[f2 + vj] = longfractal[(i + edge * 2) * edge + j];
            arr[f3 + vj] = longfractal[(i + edge * 3) * edge + j];
        }
    }

    // Now we need to make a square fractal for face 4.

    vector<int> squarefractal((edge + 1) * (edge + 1), 0);

    newfractalinit(squarefractal, edge, edge - 1, edge - 1, grain, min, max, extreme);

    // Copy information from the edges of the adjacent faces, so it matches up.

    for (int i = 0; i < edge; i++)
    {
        int vi = i * edge;

        squarefractal[vi + edge - 1] = arr[vi];
        squarefractal[(edge - 1) * edge + i] = arr[f1 + (edge - 1 - i) * edge];
        squarefractal[vi] = arr[f2 + (edge - 1 - i) * edge];
        squarefractal[i] = arr[f3 + vi];
    }

    newlidfractal(squarefractal, arr, edge, edge - 1, edge - 1, grain, valuemod, valuemod2, min, max, wrapped, 1);

    // Apply that to face 4.

    for (int i = 0; i < edge; i++)
    {
        int vi = i * edge;

        for (int j = 0; j < edge; j++)
        {
            int index = vi + j;
            arr[f4 + index] = squarefractal[index];
        }
    }

    // Now do all that again for face 5.

    for (int i = 0; i <= edge; i++)
    {
        int vi = i * edge;

        for (int j = 0; j <= edge; j++)
            squarefractal[vi + j] = 0;
    }

    newfractalinit(squarefractal, edge, edge - 1, edge - 1, grain, min, max, extreme);

    // Copy information from the edges of the adjacent faces, so it matches up.

    for (int i = 0; i < edge; i++)
    {
        int vi = i * edge;

        squarefractal[vi] = arr[vi + edge - 1];
        squarefractal[(edge - 1) * edge + i] = arr[f1 + vi + edge - 1];
        squarefractal[vi + edge - 1] = arr[f2 + (edge - 1 - i) * edge + (edge - 1)];
        squarefractal[i] = arr[f3 + (edge - 1 - i) * edge + (edge - 1)];
    }

    newlidfractal(squarefractal, arr, edge, edge - 1, edge - 1, grain, valuemod, valuemod2, min, max, wrapped, 0);

    // Apply that to face 5.

    for (int i = 0; i < edge; i++)
    {
        int vi = i * edge;

        for (int j = 0; j < edge; j++)
        {
            int index = vi + j;
            arr[f5 + index] = squarefractal[index];
        }
    }
}

// Deprecated version of the above, for 3D vectors.

void createfractalold(vector<vector<vector<int>>>& arr, int edge, int grain, float valuemod, float valuemod2, int min, int max, bool extreme, bool wrapped)
{
    // First, we need to make one long fractal for faces 0-3, all in one go.

    int longwidth = edge * 4;
    int longheight = edge;

    vector<vector<int>> longfractal(longwidth, vector<int>(longheight, 0));

    newfractalinitold(longfractal, longwidth - 1, longheight - 1, grain, min, max, extreme);
    newfractalold(longfractal, longwidth - 1, longheight - 1, grain, valuemod, valuemod2, min, max, wrapped);

    // Apply that to faces 0-3.

    for (int i = 0; i < edge; i++)
    {
        for (int j = 0; j < edge; j++)
        {
            arr[0][i][j] = longfractal[i][j];
            arr[1][i][j] = longfractal[i + edge][j];
            arr[2][i][j] = longfractal[i + edge * 2][j];
            arr[3][i][j] = longfractal[i + edge * 3][j];
        }
    }

    // Now we need to make a square fractal for face 4.

    vector<vector<int>> squarefractal(edge + 1, vector<int>(edge + 1, 0));

    newfractalinitold(squarefractal, edge - 1, edge - 1, grain, min, max, extreme);

    // Copy information from the edges of the adjacent faces, so it matches up.

    for (int i = 0; i < edge; i++)
    {
        squarefractal[i][edge - 1] = arr[0][i][0];
        squarefractal[edge - 1][i] = arr[1][edge - 1 - i][0];
        squarefractal[i][0] = arr[2][edge - 1 - i][0];
        squarefractal[0][i] = arr[3][i][0];
    }

    newlidfractalold(squarefractal, arr, edge - 1, edge - 1, grain, valuemod, valuemod2, min, max, wrapped, 1);

    // Apply that to face 4.

    for (int i = 0; i < edge; i++)
    {
        for (int j = 0; j < edge; j++)
        {
            arr[4][i][j] = squarefractal[i][j];
        }
    }

    // Now do all that again for face 5.

    for (int i = 0; i <= edge; i++)
    {
        for (int j = 0; j <= edge; j++)
            squarefractal[i][j] = 0;
    }

    newfractalinitold(squarefractal, edge - 1, edge - 1, grain, min, max, extreme);

    // Copy information from the edges of the adjacent faces, so it matches up.

    for (int i = 0; i < edge; i++)
    {
        squarefractal[i][0] = arr[0][i][edge - 1];
        squarefractal[edge - 1][i] = arr[1][i][edge - 1];
        squarefractal[i][edge - 1] = arr[2][edge - 1 - i][edge - 1];
        squarefractal[0][i] = arr[3][edge - 1 - i][edge - 1];
    }

    newlidfractalold(squarefractal, arr, edge - 1, edge - 1, grain, valuemod, valuemod2, min, max, wrapped, 0);

    // Apply that to face 5.

    for (int i = 0; i < edge; i++)
    {
        for (int j = 0; j < edge; j++)
        {
            arr[5][i][j] = squarefractal[i][j];
        }
    }
}

// This function creates a new fractal map for the modified map merge.

void createfractalformodifiedmerging(vector<int>& arr, int edge, int grain, float valuemod, float valuemod2, int min, int max, bool extreme)
{
    // First, we need to make one long fractal for faces 0-3, all in one go.

    int longwidth = edge * 4;
    int longheight = edge;

    vector<int> longfractal(longwidth * longheight, 0);

    newfractalinitformodifiedmerging(longfractal, longwidth - 1, longheight - 1, grain, min, max, extreme);
    newfractal(longfractal, edge, longwidth - 1, longheight - 1, grain, valuemod, valuemod2, min, max, 0);

    // Apply that to faces 0-3.

    int f1 = edge * edge;
    int f2 = edge * edge * 2;
    int f3 = edge * edge * 3;
    int f4 = edge * edge * 4;
    int f5 = edge * edge * 5;

    for (int i = 0; i < edge; i++)
    {
        int vi = i * edge;

        for (int j = 0; j < edge; j++)
        {
            int vj = vi + j;

            arr[vj] = longfractal[i * edge + j];
            arr[f1 + vj] = longfractal[(i + edge) * edge + j];
            arr[f2 + vj] = longfractal[(i + edge * 2) * edge + j];
            arr[f3 + vj] = longfractal[(i + edge * 3) * edge + j];
        }
    }

    // Now we need to make a square fractal for face 4.

    vector<int> squarefractal((edge + 1) * (edge + 1), 0);

    newfractalinitformodifiedmerging(squarefractal, edge - 1, edge - 1, grain, min, max, extreme);

    // Copy information from the edges of the adjacent faces, so it matches up.

    for (int i = 0; i < edge; i++)
    {
        int vi = i * edge;

        squarefractal[vi + edge - 1] = arr[vi];
        squarefractal[(edge - 1) * edge + i] = arr[f1 + (edge - 1 - i) * edge];
        squarefractal[vi] = arr[f2 + (edge - 1 - i) * edge];
        squarefractal[i] = arr[f3 + vi];
    }

    newlidfractal(squarefractal, arr, edge, edge - 1, edge - 1, grain, valuemod, valuemod2, min, max, 0, 1);

    // Apply that to face 4.

    for (int i = 0; i < edge; i++)
    {
        int vi = i * edge;

        for (int j = 0; j < edge; j++)
        {
            int index = vi + j;
            arr[f4 + index] = squarefractal[index];
        }
    }

    // Now do all that again for face 5.

    for (int i = 0; i <= edge; i++)
    {
        int vi = i * edge;

        for (int j = 0; j <= edge; j++)
            squarefractal[vi + j] = 0;
    }

    newfractalinitformodifiedmerging(squarefractal, edge - 1, edge - 1, grain, min, max, extreme);

    // Copy information from the edges of the adjacent faces, so it matches up.

    for (int i = 0; i < edge; i++)
    {
        int vi = i * edge;

        squarefractal[vi] = arr[vi + edge - 1];
        squarefractal[(edge - 1) * edge + i] = arr[f1 + vi + edge - 1];
        squarefractal[vi + edge - 1] = arr[f2 + (edge - 1 - i) * edge + (edge - 1)];
        squarefractal[i] = arr[f3 + (edge - 1 - i) * edge + (edge - 1)];
    }

    newlidfractal(squarefractal, arr, edge, edge - 1, edge - 1, grain, valuemod, valuemod2, min, max, 0, 0);

    // Apply that to face 5.

    for (int i = 0; i < edge; i++)
    {
        int vi = i * edge;

        for (int j = 0; j < edge; j++)
        {
            int index = vi + j;
            arr[f5 + index] = squarefractal[index];
        }
    }
}

// This function creates a fractal map to serve as a mask around problematic edges.

void createfractalforedgemask(vector<vector<vector<int>>>& arr, int edge, int grain, float valuemod, float valuemod2, int min, int max, bool extreme, bool wrapped)
{
    // We'll do this on a temporary array because we will do some transformations to it at the end to make it smoother.

    vector<vector<vector<int>>> temp(6, vector<vector<int>>(edge, vector<int>(edge, 0)));
   
    // First, we need to make one long fractal for faces 0-3, all in one go.

    int longwidth = edge * 4;
    int longheight = edge;

    vector<vector<int>> longfractal(longwidth, vector<int>(longheight, 0));
    vector<vector<int>> longfractal2(longwidth, vector<int>(longheight, 0));

    int s = (longheight + 1) / grain;

    for (int n = edge; n < edge * 3; n++)
    {
        for (int m = 0; m <= s; m++)
        {
            longfractal[n][m] = max;
            longfractal2[n][m] = max;
        }
    }
    
    newfractalold(longfractal, longwidth - 1, longheight - 1, grain, valuemod, valuemod2, min, max, wrapped);
    newfractalold(longfractal2, longwidth - 1, longheight - 1, grain, valuemod, valuemod2, min, max, wrapped);

    float pets = (float)max * 1.5f / (float)longheight;

    float amount = 0.0f - (float)max * 0.5f;

    for (int j = longheight - 1; j >= 0; j--)
    {
        amount = amount + pets;

        int thisamount = (int)amount;

        if (thisamount < 0)
            thisamount = 0;

        for (int i = 0; i < longwidth; i++)
        {
            if (longfractal[i][j] > thisamount)
                longfractal[i][j] = thisamount;

            if (longfractal2[i][j] > thisamount)
                longfractal2[i][j] = thisamount;
        }
    }

    /*
    pets = (float)max * 1.5f / (float)edge;

    amount = 0.0f - (float)max * 0.5f;

    for (int i = 0; i < edge; i++)
    {
        amount = amount + pets;

        int thisamount = (int)amount;

        if (thisamount < 0)
            thisamount = 0;

        for (int j = 0; j < edge; j++)
        {
            if (longfractal[i][j] > thisamount)
                longfractal[i][j] = thisamount;

            if (longfractal[longwidth - i - 1][j] > thisamount)
                longfractal[longwidth - i - 1][j] = thisamount;
        }
    }
    */

    // Apply that to faces 0-3.

    for (int i = 0; i < edge; i++)
    {
        for (int j = 0; j < edge; j++)
        {
            temp[0][i][j] = longfractal[i][j];
            temp[1][i][j] = longfractal[i + edge][j];
            temp[2][i][j] = longfractal[i + edge * 2][j];
            temp[3][i][j] = longfractal[i + edge * 3][j];

            /*
            if (longfractal2[i][j] < temp[0][i][j])
                temp[0][i][j] = longfractal2[i][j];

            if (longfractal2[i + edge][j] < temp[1][i][j])
                temp[1][i][j] = longfractal2[i + edge][j];

            if (longfractal2[i + edge * 2][j] < temp[2][i][j])
                temp[2][i][j] = longfractal2[i + edge * 2][j];

            if (longfractal2[i + edge * 3][j] < temp[3][i][j])
                temp[3][i][j] = longfractal2[i + edge * 3][j];
            */
        }
    }

    // Now face 4.

    vector<vector<int>> squarefractal(edge + 1, vector<int>(edge + 1, 0));
    vector<vector<int>> squarefractal2(edge + 1, vector<int>(edge + 1, 0));

    /*
    s = (edge) / grain;

    for (int i = 0; i <= edge; i = i + s)
    {
        for (int j = 0; j <= i; j = j + s)
            squarefractal[i][j] = max;
    }
    */


    /*
    int radius = edge / 2;

    for (int i = 0; i < radius; i++)
    {
        for (int j = 0; j < radius; j++)
        {
            if (i * i + j * j > radius * radius)
            {
                int ii = edge / 2 + i;
                int jj = edge / 2 + j;

                squarefractal[ii][jj] = max;
            }
        }
    }
    */

    /*
    for (int n = 0; n < edge; n++)
    {
        for (int m = edge - s; m < edge - 1; m++)
        {
            squarefractal[n][m] = max;
            squarefractal[m][n] = max;
        }
    }
    */

    /*
    for (int i = 0; i < edge; i++)
    {
        float upval = (float)temp[2][edge - 1 - i][0];
        float downval = (float)temp[0][i][0];

        float rightprop = (float)i / (float)edge;
        float leftprop = 1.0f - rightprop;

        for (int j = 0; j < edge; j++)
        {
            float leftval = (float)temp[3][j][0];
            float rightval = (float)temp[1][edge - 1 - j][0];

            float downprop = (float)j / (float)edge;
            float upprop = 1.0f - downprop;

            float hoz = leftval * leftprop + rightval * rightprop;
            float vert = upval * upprop + downval * downprop;

            float totalval = (hoz + vert) / 2.0f;

            squarefractal[i][j] = (int)totalval;
        }
    }
    */

    // Copy information from the edges of the adjacent faces, so it matches up.

    for (int i = 0; i < edge; i++)
    {
        squarefractal[i][edge] = temp[0][i][0];
        squarefractal[edge][i] = temp[1][edge - 1 - i][0];
        squarefractal[i][0] = temp[2][edge - 1 - i][0];
        squarefractal[0][i] = temp[3][i][0];

        squarefractal2[i][edge] = temp[0][i][0];
        squarefractal2[edge][i] = temp[1][edge - 1 - i][0];
        squarefractal2[i][0] = temp[2][edge - 1 - i][0];
        squarefractal2[0][i] = temp[3][i][0];
    }

    for (int i = edge - 2; i <= edge; i++)
    {
        for (int j = edge - 2; j <= edge; j++)
        {
            squarefractal[i][j] = max;
            squarefractal2[i][j] = max;
        }
    }

    newlidfractalold(squarefractal, temp, edge - 1, edge - 1, grain, valuemod, valuemod2, min, max, wrapped, 1);
    newlidfractalold(squarefractal2, temp, edge - 1, edge - 1, grain, valuemod, valuemod2, min, max, wrapped, 1);

    // Apply that to face 4.

    for (int i = 0; i < edge; i++)
    {
        for (int j = 0; j < edge; j++)
        {
            temp[4][i][j] = squarefractal[i][j];

            //if (squarefractal2[i][j] < temp[4][i][j])
                //temp[4][i][j] = squarefractal2[i][j];
        }
    }

    // Now copy all that over to the array we're returning, with some transformations.

    int mergemargin = edge / 12;

    for (int i = 0; i < edge; i++)
    {
        for (int j = 0; j < edge; j++)
        {
            arr[0][i][j] = temp[0][i][edge - 1 - j];
            arr[1][i][j] = temp[1][i][edge - 1 - j];
            arr[2][i][j] = temp[2][i][edge - 1 - j];
            arr[3][i][j] = temp[3][i][edge - 1 - j];
            
            arr[4][i][j] = 0;
            arr[5][i][j] = temp[4][i][edge - 1 - j];

            if (i > edge - mergemargin)
            {
                float dist = (float)(edge - i);
                float perc = dist / (float)mergemargin;
                float pperc = 1.0f - perc;

                float val = (float)temp[4][i][edge - 1 - j] * perc + (float)temp[4][i][j] * pperc;

                float strength = (float)j / (float)edge;
                strength = 1.0f - strength;

                strength = strength * 0.5f;

                val = val * strength;

                arr[5][i][j] = (int)val + (int)((float)arr[5][i][j] * (1.0f - strength));
            }
        }
    }
}

// Initialises the random values for the fractal map generator.

void newfractalinit(vector<int>& arr, int edge, int awidth, int aheight, int grain, int min, int max, bool extreme)
{
    int s = (aheight + 1) / grain;

    int newval = 0;

    for (int i = 0; i <= awidth; i = i + s)
    {
        int vi = i * edge;

        for (int j = 0; j <= aheight; j = j + s)
        {
            int index = vi + j;

            if (extreme == 1)
            {
                if (random(0, 1) == 1)
                    newval = min;

                else
                    newval = max;
            }
            else
                newval = random(min, max);

            if (arr[index] == 0)
                arr[index] = newval;
        }
    }
}

// Deprecated version of the above, for 3D vectors.

void newfractalinitold(vector<vector<int>>& arr, int awidth, int aheight, int grain, int min, int max, bool extreme)
{
    int s = (aheight + 1) / grain;

    int newval = 0;

    for (int i = 0; i <= awidth; i = i + s)
    {
        for (int j = 0; j <= aheight; j = j + s)
        {
            if (extreme == 1)
            {
                if (random(0, 1) == 1)
                    newval = min;

                else
                    newval = max;
            }
            else
                newval = random(min, max);

            if (arr[i][j] == 0)
                arr[i][j] = newval;
        }
    }
}

// Initialises the random values for the fractal map generator for the modified map merge.

void newfractalinitformodifiedmerging(vector<int>& arr, int awidth, int aheight, int grain, int min, int max, bool extreme)
{
    int maxchance = random(6, 12); // The higher this number, the fewer maximum points there will be.

    int s = (aheight + 1) / grain;

    int newval = 0;

    for (int i = 0; i <= awidth; i = i + s)
    {
        int vi = i * (aheight + 1);

        for (int j = 0; j <= aheight; j = j + s)
        {
            if (random(1, maxchance) == 1)
                newval = max;
            else
                newval = min;

            arr[vi + j] = newval;
        }
    }
}

// This is the main fractal generating routine.

void newfractal(vector<int>& arr, int edge, int awidth, int aheight, int grain, float valuemod, float valuemod2, int min, int max, bool wrapped)
{
    bool simple = 0;

    if (valuemod == valuemod2)
        simple = 1;

    int newheight = 0;

    int s = (aheight + 1) / grain;

    int focalx = awidth / 2; // Coordinates for the focal point of the value variance.
    int focaly = random(0, aheight);

    float valuediff = abs(valuemod - valuemod2); // difference between the two valuemods.
    float increment = valuediff / 100.0f;

    valuemod = valuemod * 100.0f;
    valuemod2 = valuemod2 * 100.0f;

    while (s != -1) // This loop will repeat, with s halving in size each time until it gets to 1.
    {
        int value = s * (int)valuemod; // This is the amount we will vary each new pixel by. The smaller the tile, the smaller this value should be.

        // First we go over the whole map doing the square step.

        for (int z = 0; z <= awidth + s; z = z + s)
        {
            int i = z;

            if (i > awidth)
                i = wrap(i, awidth);

            int ss = s / 2;
            int ii = i + ss;

            if (ii > awidth)
                ii = wrap(ii, awidth);

            int vii = ii * edge;

            for (int j = 0; j <= aheight; j = j + s)
            {
                int jj = j + ss;

                if (simple == 0)
                {
                    float focaldistance = (float)sqrt((focalx - ii) * (focalx - ii) + (focaly - jj) * (focaly - jj));
                    int perc = (int)(increment * focaldistance);

                    float newval = tilt(valuemod, valuemod2, perc);

                    newval = newval / 100.0f;
                    value = s * (int)newval;
                }

                newheight = square(arr, edge, awidth, aheight, s, ii, jj, value, min, max, wrapped);

                if (jj <= aheight)
                    arr[vii + jj] = newheight;
            }
        }

        // Now we go over the whole map again, doing the diamond step.

        for (int z = 0; z <= awidth + s; z = z + s)
        {
            int i = z;

            if (i > awidth)
                i = wrap(i, awidth);

            int ss = s / 2;

            int ii = i + ss;

            if (ii > awidth)
                ii = wrap(ii, awidth);

            for (int j = 0; j <= aheight; j = j + s)
            {
                int jj = j + ss;

                if (simple == 0)
                {
                    float focaldistance = (float)sqrt((focalx - ii) * (focalx - ii) + (focaly - jj) * (focaly - jj));

                    int perc = (int)(increment * focaldistance);

                    float newval = tilt(valuemod, valuemod2, perc);

                    newval = newval / 100.0f;
                    value = s * (int)newval;
                }

                for (int n = 1; n <= 2; n++)
                {
                    int ii = 0;
                    int jj = 0;

                    if (n == 1)
                    {
                        ii = i + ss;
                        jj = j;
                    }

                    if (n == 2)
                    {
                        ii = i;
                        jj = j + ss;
                    }

                    if (jj >= 0 && jj <= aheight)
                    {
                        ii = wrap(ii, awidth);

                        newheight = diamond(arr, edge, awidth, aheight, s, ii, jj, value, min, max, wrapped);

                        arr[ii * edge + jj] = newheight;
                    }
                }
            }
        }

        if (s > 2) // Now halve the size of the tiles.
            s = s / 2;
        else
            s = -1;
    }
}

// Deprecated version of the above, for 3D vectors.

void newfractalold(vector<vector<int>>& arr, int awidth, int aheight, int grain, float valuemod, float valuemod2, int min, int max, bool wrapped)
{
    bool simple = 0;

    if (valuemod == valuemod2)
        simple = 1;

    int newheight = 0;

    int s = (aheight + 1) / grain;

    int focalx = awidth / 2; // Coordinates for the focal point of the value variance.
    int focaly = random(0, aheight);

    float valuediff = abs(valuemod - valuemod2); // difference between the two valuemods.
    float increment = valuediff / 100.0f;

    valuemod = valuemod * 100.0f;
    valuemod2 = valuemod2 * 100.0f;

    while (s != -1) // This loop will repeat, with s halving in size each time until it gets to 1.
    {
        int value = s * (int)valuemod; // This is the amount we will vary each new pixel by. The smaller the tile, the smaller this value should be.

        // First we go over the whole map doing the square step.

        for (int z = 0; z <= awidth + s; z = z + s)
        {
            int i = z;

            if (i > awidth)
                i = wrap(i, awidth);

            int ss = s / 2;
            int ii = i + ss;

            if (ii > awidth)
                ii = wrap(ii, awidth);

            for (int j = 0; j <= aheight; j = j + s)
            {
                int jj = j + ss;

                if (simple == 0)
                {
                    float focaldistance = (float)sqrt((focalx - ii) * (focalx - ii) + (focaly - jj) * (focaly - jj));
                    int perc = (int)(increment * focaldistance);

                    float newval = tilt(valuemod, valuemod2, perc);

                    newval = newval / 100.0f;
                    value = s * (int)newval;
                }

                newheight = squareold(arr, awidth, aheight, s, ii, jj, value, min, max, wrapped);

                if (jj <= aheight)
                    arr[ii][jj] = newheight;
            }
        }

        // Now we go over the whole map again, doing the diamond step.

        for (int z = 0; z <= awidth + s; z = z + s)
        {
            int i = z;

            if (i > awidth)
                i = wrap(i, awidth);

            int ss = s / 2;

            int ii = i + ss;

            if (ii > awidth)
                ii = wrap(ii, awidth);

            for (int j = 0; j <= aheight; j = j + s)
            {
                int jj = j + ss;

                if (simple == 0)
                {
                    float focaldistance = (float)sqrt((focalx - ii) * (focalx - ii) + (focaly - jj) * (focaly - jj));

                    int perc = (int)(increment * focaldistance);

                    float newval = tilt(valuemod, valuemod2, perc);

                    newval = newval / 100.0f;
                    value = s * (int)newval;
                }

                for (int n = 1; n <= 2; n++)
                {
                    int ii = 0;
                    int jj = 0;

                    if (n == 1)
                    {
                        ii = i + ss;
                        jj = j;
                    }

                    if (n == 2)
                    {
                        ii = i;
                        jj = j + ss;
                    }

                    if (jj >= 0 && jj <= aheight)
                    {
                        ii = wrap(ii, awidth);

                        newheight = diamondold(arr, awidth, aheight, s, ii, jj, value, min, max, wrapped);

                        arr[ii][jj] = newheight;
                    }
                }
            }
        }

        if (s > 2) // Now halve the size of the tiles.
            s = s / 2;
        else
            s = -1;
    }
}

// This creates a fractal for a starfield, with a milky way effect.

void createmilkywayfractal(vector<int>& arr, int edge, int grain, float valuemod, float valuemod2, int min, int max, int milkywaymin, int milkywaymax, bool extreme, bool wrapped)
{
    // First, we need to make one long fractal for faces 0-3, all in one go.

    int longwidth = edge * 4;
    int longheight = edge;

    vector<int> longfractal(longwidth * longheight, 0);

    newmilkywayfractalinit(longfractal, edge, longwidth - 1, longheight - 1, grain, min, max, milkywaymin, milkywaymax, extreme);    
    newfractal(longfractal, edge, longwidth - 1, longheight - 1, grain, valuemod, valuemod2, min, milkywaymax, wrapped);

    // Apply that to faces 0-3.

    int f1 = edge * edge;
    int f2 = edge * edge * 2;
    int f3 = edge * edge * 3;
    int f4 = edge * edge * 4;
    int f5 = edge * edge * 5;

    for (int i = 0; i < edge; i++)
    {
        int vi = i * edge;

        for (int j = 0; j < edge; j++)
        {
            int vj = vi + j;

            arr[vj] = longfractal[i * edge + j];
            arr[f1 + vj] = longfractal[(i + edge) * edge + j];
            arr[f2 + vj] = longfractal[(i + edge * 2) * edge + j];
            arr[f3 + vj] = longfractal[(i + edge * 3) * edge + j];
        }
    }

    // Now we need to make a square fractal for face 4.

    vector<int> squarefractal((edge + 1) * (edge + 1), 0);

    newfractalinit(squarefractal, edge, edge - 1, edge - 1, grain, min, max, extreme);

    // Copy information from the edges of the adjacent faces, so it matches up.

    for (int i = 0; i < edge; i++)
    {
        int vi = i * edge;

        squarefractal[vi + edge - 1] = arr[vi];
        squarefractal[(edge - 1) * edge + i] = arr[f1 + (edge - 1 - i) * edge];
        squarefractal[vi] = arr[f2 + (edge - 1 - i) * edge];
        squarefractal[i] = arr[f3 + vi];
    }

    newlidfractal(squarefractal, arr, edge, edge - 1, edge - 1, grain, valuemod, valuemod2, min, milkywaymax, wrapped, 1);

    // Apply that to face 4.

    for (int i = 0; i < edge; i++)
    {
        int vi = i * edge;

        for (int j = 0; j < edge; j++)
        {
            int index = vi + j;
            arr[f4 + index] = squarefractal[index];
        }
    }

    // Now do all that again for face 5.

    for (int i = 0; i <= edge; i++)
    {
        int vi = i * edge;

        for (int j = 0; j <= edge; j++)
            squarefractal[vi + j] = 0;
    }

    newfractalinit(squarefractal, edge, edge - 1, edge - 1, grain, min, max, extreme);

    // Copy information from the edges of the adjacent faces, so it matches up.

    for (int i = 0; i < edge; i++)
    {
        int vi = i * edge;

        squarefractal[vi] = arr[vi + edge - 1];
        squarefractal[(edge - 1) * edge + i] = arr[f1 + vi + edge - 1];
        squarefractal[vi + edge - 1] = arr[f2 + (edge - 1 - i) * edge + (edge - 1)];
        squarefractal[i] = arr[f3 + (edge - 1 - i) * edge + (edge - 1)];
    }

    newlidfractal(squarefractal, arr, edge, edge - 1, edge - 1, grain, valuemod, valuemod2, min, milkywaymax, wrapped, 0);

    // Apply that to face 5.

    for (int i = 0; i < edge; i++)
    {
        int vi = i * edge;

        for (int j = 0; j < edge; j++)
        {
            int index = vi + j;
            arr[f5 + index] = squarefractal[index];
        }
    }
}

// This initialises the milky way fractal on faces 0-3.

void newmilkywayfractalinit(vector<int>& arr, int edge, int awidth, int aheight, int grain, int min, int max, int milkywaymin, int milkywaymax, bool extreme)
{
    int s = (aheight + 1) / grain;

    // First, we will nudge the y location and the width of the milky way at different points along its length.

    int nodestotal = (awidth + 1) / s;

    int baseheight = aheight / 2;

    int a = random(10, 60); 

    if (grain >= 16)
        a = random(40, 60); // Ensure a broad width for finer-grained fractals.

    int basewidth = aheight / a; // 100;

    vector<int> heightnodes(nodestotal, baseheight); // This will hold the y value of the centre of the milky way at this point.
    vector<int> widthnodes(nodestotal, basewidth);

    int maxheightnudge = aheight / 120; // 100
    int maxwidthnudge = aheight / 180; //40; // 80;

    int totalnudges = 20;

    int bulgewidth = nodestotal / 30;

    for (int n = 0; n < nodestotal; n++)
    {
        heightnodes[n] = baseheight;
        widthnodes[n] = basewidth;
    }

    for (int thisnode = 0; thisnode < nodestotal; thisnode++)
    {
        int thisheightnudge = random(0, maxheightnudge);
        int thiswidthnudge = random(0, maxwidthnudge);

        bool negheight = 0;

        if (random(1, 2) == 1)
            negheight = 0;
        else
            negheight = 1;

        bool negwidth = 0;

        if (random(1, 2) == 1)
            negwidth = 0;
        else
            negwidth = 1;

        if (negheight)
            thisheightnudge = -thisheightnudge;

        if (negwidth)
            thiswidthnudge = -thiswidthnudge;

        heightnodes[thisnode] = heightnodes[thisnode] + thisheightnudge;
        widthnodes[thisnode] = widthnodes[thisnode] + thiswidthnudge;

        int prevnode = thisnode;
        int nextnode = thisnode;

        float heightdiv = (float)thisheightnudge / (float)bulgewidth;
        float widthdiv = (float)thiswidthnudge / (float)bulgewidth;

        for (int p = bulgewidth; p > 0; p--)
        {
            int thatheightnudge = (int)((float)thisheightnudge * (heightdiv * (float)p));
            int thatwidthnudge = (int)((float)thiswidthnudge * (widthdiv * (float)p));

            prevnode--;

            if (prevnode < 0)
                prevnode = nodestotal - 1;

            nextnode++;

            if (nextnode = nodestotal)
                nextnode = 0;

            heightnodes[prevnode] = heightnodes[prevnode] + thatheightnudge;
            heightnodes[nextnode] = heightnodes[nextnode] + thatheightnudge;
            widthnodes[prevnode] = widthnodes[prevnode] + thatwidthnudge;
            widthnodes[nextnode] = widthnodes[nextnode] + thatwidthnudge;
        }
    }

    for (int n = 0; n < nodestotal; n++)
    {
        if (heightnodes[n] < aheight / 4)
            heightnodes[n] = aheight / 4 + random(1, maxheightnudge); // 4

        if (heightnodes[n] > aheight - aheight / 4)
            heightnodes[n] = aheight - aheight / 4 - random(1, maxheightnudge); // 4

        if (widthnodes[n] < 0)
            widthnodes[n] = 0;

        if (widthnodes[n] > basewidth * 3)
            widthnodes[n] = basewidth * 3;
    }

    heightnodes[0] = (heightnodes[nodestotal - 1] + heightnodes[1]) / 2;
    widthnodes[0] = (widthnodes[nodestotal - 1] + widthnodes[1]) / 2;

    // Now populate the array.

    int newval = 0;

    float centre = 0.6f; // The bigger this is, the wider the central denser band.

    for (int i = 0; i <= awidth; i = i + s)
    {
        int vi = i * edge;

        int node = i / s;

        for (int j = 0; j <= aheight; j = j + s)
        {
            int index = vi + j;

            int thismin = min;
            int thismax = max;

            int thiscentre = (int)((float)widthnodes[node] * centre);

            if ((j > heightnodes[node] - widthnodes[node] && j < heightnodes[node] + widthnodes[node]))
            {
                if (j < heightnodes[node] - thiscentre || j > heightnodes[node] + thiscentre)
                {
                    thismin = (min + milkywaymin) / 2;
                    thismax = (max + milkywaymax) / 2;
                }
                else
                {
                    thismin = milkywaymin;
                    thismax = milkywaymax;
                }
            }
            else
            {
                if (random(1, 50) == 1)
                    thismax = (int)((float)milkywaymax * 0.7);
            }

            if (extreme == 1)
            {
                if (random(0, 1) == 1)
                    newval = thismin;

                else
                    newval = thismax;
            }
            else
                newval = random(thismin, thismax);

            if (arr[index] == 0)
                arr[index] = newval;
        }
    }
}


// This does the square part of the fractal.

int square(vector<int>& arr, int edge, int awidth, int aheight, int s, int x, int y, int value, int min, int max, bool wrapped)
{
    value = value * 50;
    int dist = s / 2;
    int total = 0;
    int crount = 0;

    int coords[4][2];

    coords[0][0] = x - dist;
    coords[0][1] = y - dist;

    coords[1][0] = x + dist;
    coords[1][1] = y - dist;

    coords[2][0] = x + dist;
    coords[2][1] = y + dist;

    coords[3][0] = x - dist;
    coords[3][1] = y + dist;

    if (wrapped == 1)
    {
        int values[4];

        int nullval = 0 - max * 10;

        for (int n = 0; n < 4; n++)
            values[n] = nullval;

        for (int n = 0; n <= 3; n++)
        {
            if (coords[n][1] >= 0 && coords[n][1] <= aheight)
            {
                coords[n][0] = wrap(coords[n][0], awidth);

                int x = coords[n][0];
                int y = coords[n][1];

                values[n] = arr[x * edge + y];
            }
        }

        int a = 0;

        if (values[0] != nullval && values[1] != nullval)
            a = wrappedaverage(values[0], values[1], max);
        else
        {
            if (values[0] == nullval)
                a = values[1];
            else
                a = values[0];
        }

        int b = 0;

        if (values[2] != nullval && values[3] != nullval)
            b = wrappedaverage(values[2], values[3], max);
        else
        {
            if (values[2] == nullval)
                b = values[3];
            else
                b = values[2];
        }

        if (a != nullval && b != nullval)
            total = wrappedaverage(a, b, max);
        else
        {
            if (a == nullval)
                total = b;
            else
                total = a;
        }
    }
    else
    {
        for (int n = 0; n <= 3; n++)
        {
            if (coords[n][1] >= 0 && coords[n][1] <= aheight)
            {
                coords[n][0] = wrap(coords[n][0], awidth);

                int x = coords[n][0];
                int y = coords[n][1];

                total = total + arr[x * edge + y];
                crount++;
            }
        }

        total = total / crount;
    }

    int difference = randomsign(random(0, value));

    total = total + difference;

    if (wrapped == 1)
    {
        while (total > max)
            total = total - max;

        while (total < min)
            total = total + max;

    }
    else
    {
        if (total > max)
            total = max;

        if (total < min)
            total = min;
    }

    return (total);
}

// Deprecated version of the above, for 3D vectors.

int squareold(vector<vector<int>>& arr, int awidth, int aheight, int s, int x, int y, int value, int min, int max, bool wrapped)
{
    value = value * 50;
    int dist = s / 2;
    int total = 0;
    int crount = 0;

    int coords[4][2];

    coords[0][0] = x - dist;
    coords[0][1] = y - dist;

    coords[1][0] = x + dist;
    coords[1][1] = y - dist;

    coords[2][0] = x + dist;
    coords[2][1] = y + dist;

    coords[3][0] = x - dist;
    coords[3][1] = y + dist;

    if (wrapped == 1)
    {
        int values[4];

        int nullval = 0 - max * 10;

        for (int n = 0; n < 4; n++)
            values[n] = nullval;

        for (int n = 0; n <= 3; n++)
        {
            if (coords[n][1] >= 0 && coords[n][1] <= aheight)
            {
                coords[n][0] = wrap(coords[n][0], awidth);

                int x = coords[n][0];
                int y = coords[n][1];

                values[n] = arr[x][y];
            }
        }

        int a = 0;

        if (values[0] != nullval && values[1] != nullval)
            a = wrappedaverage(values[0], values[1], max);
        else
        {
            if (values[0] == nullval)
                a = values[1];
            else
                a = values[0];
        }

        int b = 0;

        if (values[2] != nullval && values[3] != nullval)
            b = wrappedaverage(values[2], values[3], max);
        else
        {
            if (values[2] == nullval)
                b = values[3];
            else
                b = values[2];
        }

        if (a != nullval && b != nullval)
            total = wrappedaverage(a, b, max);
        else
        {
            if (a == nullval)
                total = b;
            else
                total = a;
        }
    }
    else
    {
        for (int n = 0; n <= 3; n++)
        {
            if (coords[n][1] >= 0 && coords[n][1] <= aheight)
            {
                coords[n][0] = wrap(coords[n][0], awidth);

                int x = coords[n][0];
                int y = coords[n][1];

                total = total + arr[x][y];
                crount++;
            }
        }

        total = total / crount;
    }

    int difference = randomsign(random(0, value));

    total = total + difference;

    if (wrapped == 1)
    {
        while (total > max)
            total = total - max;

        while (total < min)
            total = total + max;

    }
    else
    {
        if (total > max)
            total = max;

        if (total < min)
            total = min;
    }

    return (total);
}

// This does the diamond part of the fractal.

int diamond(vector<int>& arr, int edge, int awidth, int aheight, int s, int x, int y, int value, int min, int max, bool wrapped)
{
    value = value * 50;
    int dist = s / 2;
    int total = 0;
    int crount = 0;

    int coords[4][2];

    coords[0][0] = x;
    coords[0][1] = y - dist;

    coords[1][0] = x + dist;
    coords[1][1] = y;

    coords[2][0] = x;
    coords[2][1] = y + dist;

    coords[3][0] = x - dist;
    coords[3][1] = y;

    if (wrapped == 1)
    {
        int values[4];

        int nullval = 0 - max * 10;

        for (int n = 0; n < 4; n++)
            values[n] = nullval;

        for (int n = 0; n <= 3; n++)
        {
            if (coords[n][1] >= 0 && coords[n][1] <= aheight)
            {
                coords[n][0] = wrap(coords[n][0], awidth);

                int x = coords[n][0];
                int y = coords[n][1];

                values[n] = arr[x * edge + y];
            }
        }

        int a = 0;

        if (values[0] != nullval && values[1] != nullval)
            a = wrappedaverage(values[0], values[1], max);
        else
        {
            if (values[0] == nullval)
                a = values[1];
            else
                a = values[0];
        }

        int b = 0;

        if (values[2] != nullval && values[3] != nullval)
            b = wrappedaverage(values[2], values[3], max);
        else
        {
            if (values[2] == nullval)
                b = values[3];
            else
                b = values[2];
        }

        if (a != nullval && b != nullval)
            total = wrappedaverage(a, b, max);
        else
        {
            if (a == nullval)
                total = b;
            else
                total = a;
        }
    }
    else
    {
        for (int n = 0; n <= 3; n++)
        {
            if (coords[n][1] >= 0 && coords[n][1] <= aheight)
            {
                coords[n][0] = wrap(coords[n][0], awidth);

                int x = coords[n][0];
                int y = coords[n][1];

                total = total + arr[x * edge + y];
                crount++;
            }
        }

        total = total / crount;
    }

    int difference = randomsign(random(0, value));

    total = total + difference;

    if (wrapped == 1)
    {
        while (total > max)
            total = total - max;

        while (total < min)
            total = total + max;
    }
    else
    {
        if (total > max)
            total = max;

        if (total < min)
            total = min;
    }

    return (total);
}

// Deprecated version of the above, for 3D vectors.

int diamondold(vector<vector<int>>& arr, int awidth, int aheight, int s, int x, int y, int value, int min, int max, bool wrapped)
{
    value = value * 50;
    int dist = s / 2;
    int total = 0;
    int crount = 0;

    int coords[4][2];

    coords[0][0] = x;
    coords[0][1] = y - dist;

    coords[1][0] = x + dist;
    coords[1][1] = y;

    coords[2][0] = x;
    coords[2][1] = y + dist;

    coords[3][0] = x - dist;
    coords[3][1] = y;

    if (wrapped == 1)
    {
        int values[4];

        int nullval = 0 - max * 10;

        for (int n = 0; n < 4; n++)
            values[n] = nullval;

        for (int n = 0; n <= 3; n++)
        {
            if (coords[n][1] >= 0 && coords[n][1] <= aheight)
            {
                coords[n][0] = wrap(coords[n][0], awidth);

                int x = coords[n][0];
                int y = coords[n][1];

                values[n] = arr[x][y];
            }
        }

        int a = 0;

        if (values[0] != nullval && values[1] != nullval)
            a = wrappedaverage(values[0], values[1], max);
        else
        {
            if (values[0] == nullval)
                a = values[1];
            else
                a = values[0];
        }

        int b = 0;

        if (values[2] != nullval && values[3] != nullval)
            b = wrappedaverage(values[2], values[3], max);
        else
        {
            if (values[2] == nullval)
                b = values[3];
            else
                b = values[2];
        }

        if (a != nullval && b != nullval)
            total = wrappedaverage(a, b, max);
        else
        {
            if (a == nullval)
                total = b;
            else
                total = a;
        }
    }
    else
    {
        for (int n = 0; n <= 3; n++)
        {
            if (coords[n][1] >= 0 && coords[n][1] <= aheight)
            {
                coords[n][0] = wrap(coords[n][0], awidth);

                int x = coords[n][0];
                int y = coords[n][1];

                total = total + arr[x][y];
                crount++;
            }
        }

        total = total / crount;
    }

    int difference = randomsign(random(0, value));

    total = total + difference;

    if (wrapped == 1)
    {
        while (total > max)
            total = total - max;

        while (total < min)
            total = total + max;
    }
    else
    {
        if (total > max)
            total = max;

        if (total < min)
            total = min;
    }

    return (total);
}

// This does a fractal for the top or bottom face of the cube.

void newlidfractal(vector<int>& arr, vector<int>& cubearr, int edge, int awidth, int aheight, int grain, float valuemod, float valuemod2, int min, int max, bool wrapped, bool top)
{
    bool simple = 0;

    if (valuemod == valuemod2)
        simple = 1;

    int newheight = 0;

    int s = (aheight + 1) / grain;

    int focalx = awidth / 2; // Coordinates for the focal point of the value variance.
    int focaly = random(0, aheight);

    float valuediff = abs(valuemod - valuemod2); // difference between the two valuemods.
    float increment = valuediff / 100.0f;

    valuemod = valuemod * 100.0f;
    valuemod2 = valuemod2 * 100.0f;

    while (s != -1) // This loop will repeat, with s halving in size each time until it gets to 1.
    {
        int value = s * (int)valuemod; // This is the amount we will vary each new pixel by. The smaller the tile, the smaller this value should be.

        // First we go over the whole map doing the square step.

        for (int z = 0; z <= awidth + s; z = z + s)
        {
            int i = z;

            int ss = s / 2;
            int ii = i + ss;

            if (ii >= 0 && ii <= awidth)
            {
                int vii = ii * edge;

                for (int j = 0; j <= aheight; j = j + s)
                {
                    int jj = j + ss;

                    int index = vii + jj;

                    if (simple == 0)
                    {
                        float focaldistance = (float)sqrt((focalx - ii) * (focalx - ii) + (focaly - jj) * (focaly - jj));
                        int perc = (int)(increment * focaldistance);

                        float newval = tilt(valuemod, valuemod2, perc);

                        newval = newval / 100.0f;
                        value = s * (int)newval;
                    }

                    if (arr[index] == 0)
                    {
                        newheight = lidsquare(arr, cubearr, edge, awidth, aheight, s, ii, jj, value, min, max, wrapped, top);

                        if (jj <= aheight)
                            arr[index] = newheight;
                    }
                }
            }
        }

        // Now we go over the whole map again, doing the diamond step.

        for (int z = 0; z <= awidth + s; z = z + s)
        {
            int i = z;

            int ss = s / 2;

            int ii = i + ss;

            if (ii >= 0 && ii <= aheight)
            {
                for (int j = 0; j <= aheight; j = j + s)
                {
                    int jj = j + ss;

                    if (simple == 0)
                    {
                        float focaldistance = (float)sqrt((focalx - ii) * (focalx - ii) + (focaly - jj) * (focaly - jj));

                        int perc = (int)(increment * focaldistance);

                        float newval = tilt(valuemod, valuemod2, perc);

                        newval = newval / 100.0f;
                        value = s * (int)newval;
                    }

                    for (int n = 1; n <= 2; n++)
                    {
                        int ii = 0;
                        int jj = 0;

                        if (n == 1)
                        {
                            ii = i + ss;
                            jj = j;
                        }

                        if (n == 2)
                        {
                            ii = i;
                            jj = j + ss;
                        }

                        if (jj >= 0 && jj <= aheight && ii >= 0 && ii <= awidth)
                        {
                            int index = ii * edge + jj;

                            if (arr[index] == 0)
                            {
                                newheight = liddiamond(arr, cubearr, edge, awidth, aheight, s, ii, jj, value, min, max, wrapped, top);

                                arr[index] = newheight;
                            }
                        }
                    }
                }
            }
        }

        if (s > 2) // Now halve the size of the tiles.
            s = s / 2;
        else
            s = -1;
    }

    // Now just smooth the edges a bit.

    for (int i = 0; i <= awidth; i++)
    {
        int vi = i * edge;

        int thisval = arr[vi];
        int thatval = arr[vi + 1];

        arr[vi] = (thisval + thatval) / 2;

        thisval = arr[vi + aheight];
        thatval = arr[vi + aheight - 1];

        arr[vi + aheight] = (thisval + thatval) / 2;
    }

    for (int j = 0; j <= aheight; j++)
    {
        int thisval = arr[j];
        int thatval = arr[edge + j];

        arr[j] = (thisval + thatval) / 2;

        thisval = arr[awidth * edge + j];
        thatval = arr[(awidth - 1) * edge + j];

        arr[awidth * edge + j] = (thisval + thatval) / 2;
    }
}

// Deprecated version of the above, for 3D vectors.

void newlidfractalold(vector<vector<int>>& arr, vector<vector<vector<int>>>& cubearr, int awidth, int aheight, int grain, float valuemod, float valuemod2, int min, int max, bool wrapped, bool top)
{
    bool simple = 0;

    if (valuemod == valuemod2)
        simple = 1;

    int newheight = 0;

    int s = (aheight + 1) / grain;

    int focalx = awidth / 2; // Coordinates for the focal point of the value variance.
    int focaly = random(0, aheight);

    float valuediff = abs(valuemod - valuemod2); // difference between the two valuemods.
    float increment = valuediff / 100.0f;

    valuemod = valuemod * 100.0f;
    valuemod2 = valuemod2 * 100.0f;

    while (s != -1) // This loop will repeat, with s halving in size each time until it gets to 1.
    {
        int value = s * (int)valuemod; // This is the amount we will vary each new pixel by. The smaller the tile, the smaller this value should be.

        // First we go over the whole map doing the square step.

        for (int z = 0; z <= awidth + s; z = z + s)
        {
            int i = z;

            int ss = s / 2;
            int ii = i + ss;

            if (ii >= 0 && ii <= awidth)
            {
                for (int j = 0; j <= aheight; j = j + s)
                {
                    int jj = j + ss;

                    if (simple == 0)
                    {
                        float focaldistance = (float)sqrt((focalx - ii) * (focalx - ii) + (focaly - jj) * (focaly - jj));
                        int perc = (int)(increment * focaldistance);

                        float newval = tilt(valuemod, valuemod2, perc);

                        newval = newval / 100.0f;
                        value = s * (int)newval;
                    }

                    if (arr[ii][jj] == 0)
                    {
                        newheight = lidsquareold(arr, cubearr, awidth, aheight, s, ii, jj, value, min, max, wrapped, top);

                        if (jj <= aheight)
                            arr[ii][jj] = newheight;
                    }
                }
            }
        }

        // Now we go over the whole map again, doing the diamond step.

        for (int z = 0; z <= awidth + s; z = z + s)
        {
            int i = z;

            int ss = s / 2;

            int ii = i + ss;

            if (ii >= 0 && ii <= aheight)
            {
                for (int j = 0; j <= aheight; j = j + s)
                {
                    int jj = j + ss;

                    if (simple == 0)
                    {
                        float focaldistance = (float)sqrt((focalx - ii) * (focalx - ii) + (focaly - jj) * (focaly - jj));

                        int perc = (int)(increment * focaldistance);

                        float newval = tilt(valuemod, valuemod2, perc);

                        newval = newval / 100.0f;
                        value = s * (int)newval;
                    }

                    for (int n = 1; n <= 2; n++)
                    {
                        int ii = 0;
                        int jj = 0;

                        if (n == 1)
                        {
                            ii = i + ss;
                            jj = j;
                        }

                        if (n == 2)
                        {
                            ii = i;
                            jj = j + ss;
                        }

                        if (jj >= 0 && jj <= aheight && ii >= 0 && ii <= awidth)
                        {
                            if (arr[ii][jj] == 0)
                            {
                                newheight = liddiamondold(arr, cubearr, awidth, aheight, s, ii, jj, value, min, max, wrapped, top);

                                arr[ii][jj] = newheight;
                            }
                        }
                    }
                }
            }
        }

        if (s > 2) // Now halve the size of the tiles.
            s = s / 2;
        else
            s = -1;
    }

    // Now just smooth the edges a bit.

    for (int i = 0; i <= awidth; i++)
    {
        int thisval = arr[i][0];
        int thatval = arr[i][1];

        arr[i][0] = (thisval + thatval) / 2;

        thisval = arr[i][aheight];
        thatval = arr[i][aheight - 1];

        arr[i][aheight] = (thisval + thatval) / 2;
    }

    for (int j = 0; j <= aheight; j++)
    {
        int thisval = arr[0][j];
        int thatval = arr[1][j];

        arr[0][j] = (thisval + thatval) / 2;

        thisval = arr[awidth][j];
        thatval = arr[awidth - 1][j];

        arr[awidth][j] = (thisval + thatval) / 2;
    }
}

// This does the square part of the lid fractal.

int lidsquare(vector<int>& arr, vector<int>& cubearr, int edge, int awidth, int aheight, int s, int x, int y, int value, int min, int max, bool wrapped, bool top)
{
    value = value * 50;
    int dist = s / 2;
    int total = 0;
    int crount = 0;

    int coords[4][2];

    coords[0][0] = x - dist;
    coords[0][1] = y - dist;

    coords[1][0] = x + dist;
    coords[1][1] = y - dist;

    coords[2][0] = x + dist;
    coords[2][1] = y + dist;

    coords[3][0] = x - dist;
    coords[3][1] = y + dist;

    int f1 = edge * edge;
    int f2 = edge * edge * 2;
    int f3 = edge * edge * 3;
    int f4 = edge * edge * 4;
    int f5 = edge * edge * 5;

    if (wrapped == 1)
    {
        int values[4];

        int nullval = 0 - max * 10;

        for (int n = 0; n < 4; n++)
            values[n] = nullval;

        for (int n = 0; n <= 3; n++)
        {
            int x = coords[n][0];
            int y = coords[n][1];

            if (x >= 0 && x <= awidth && y >= 0 && y <= aheight)
                values[n] = arr[x * edge + y];
            else
            {
                if (x > awidth && y >= 0 && y <= aheight)
                {
                    if (top)
                        values[n] = cubearr[f1 + (aheight - y) * edge + x - awidth];
                    else
                        values[n] = cubearr[f1 + y * edge + (aheight - (x - awidth))];
                }

                if (x < awidth && y >= 0 && y <= aheight)
                {
                    if (top)
                        values[n] = cubearr[f3 + y * edge - x];
                    else
                        values[n] = cubearr[f3 + (aheight - y) * edge + (aheight - (x - awidth))];
                }

                if (y > aheight && x >= 0 && x <= awidth)
                {
                    if (top)
                        values[n] = cubearr[x * edge + (y - aheight)];
                    else
                    {
                        values[n] = cubearr[f2 + (awidth - x) * edge + (aheight - (y - aheight))];
                    }
                }

                if (y < 0 && x >= 0 && x <= awidth)
                {
                    if (top)
                        values[n] = cubearr[f2 + (awidth - x) * edge - y];
                    else
                        values[n] = cubearr[x * edge + (aheight + y)];
                }
            }
        }

        int a = 0;

        if (values[0] != nullval && values[1] != nullval)
            a = wrappedaverage(values[0], values[1], max);
        else
        {
            if (values[0] == nullval)
                a = values[1];
            else
                a = values[0];
        }

        int b = 0;

        if (values[2] != nullval && values[3] != nullval)
            b = wrappedaverage(values[2], values[3], max);
        else
        {
            if (values[2] == nullval)
                b = values[3];
            else
                b = values[2];
        }

        if (a != nullval && b != nullval)
            total = wrappedaverage(a, b, max);
        else
        {
            if (a == nullval)
                total = b;
            else
                total = a;
        }
    }
    else
    {
        for (int n = 0; n <= 3; n++)
        {
            int x = coords[n][0];
            int y = coords[n][1];

            int val = 0;

            if (x >= 0 && x <= awidth && y >= 0 && y <= aheight)
                val = arr[x * edge + y];
            else
            {
                if (x > awidth && y >= 0 && y <= aheight)
                {
                    if (top)
                        val = cubearr[f1 + (aheight - y) * edge + (x - awidth)];
                    else
                        val = cubearr[f1 + y * edge + (aheight - (x - awidth))];
                }

                if (x < awidth && y >= 0 && y <= aheight)
                {
                    if (top)
                        val = cubearr[f3 + y * edge - x];
                    else
                        val = cubearr[f3 + (aheight - y) * edge + (aheight - (x - awidth))];
                }

                if (y > aheight && x >= 0 && x <= awidth)
                {
                    if (top)
                        val = cubearr[x * edge + (y - aheight)];
                    else
                    {
                        val = cubearr[f2 + (awidth - x) * edge + (aheight - (y - aheight))];
                    }
                }

                if (y < 0 && x >= 0 && x <= awidth)
                {
                    if (top)
                        val = cubearr[f2 + (awidth - x) * edge - y];
                    else
                        val = cubearr[x * edge + (aheight + y)];
                }
            }

            total = total + val;
            crount++;
        }

        total = total / crount;
    }

    int difference = randomsign(random(0, value));

    total = total + difference;

    if (wrapped == 1)
    {
        while (total > max)
            total = total - max;

        while (total < min)
            total = total + max;

    }
    else
    {
        if (total > max)
            total = max;

        if (total < min)
            total = min;
    }

    return (total);
}

// Deprecated version of the above, for 3D vectors.

int lidsquareold(vector<vector<int>>& arr, vector<vector<vector<int>>>& cubearr, int awidth, int aheight, int s, int x, int y, int value, int min, int max, bool wrapped, bool top)
{
    value = value * 50;
    int dist = s / 2;
    int total = 0;
    int crount = 0;

    int coords[4][2];

    coords[0][0] = x - dist;
    coords[0][1] = y - dist;

    coords[1][0] = x + dist;
    coords[1][1] = y - dist;

    coords[2][0] = x + dist;
    coords[2][1] = y + dist;

    coords[3][0] = x - dist;
    coords[3][1] = y + dist;

    if (wrapped == 1)
    {
        int values[4];

        int nullval = 0 - max * 10;

        for (int n = 0; n < 4; n++)
            values[n] = nullval;

        for (int n = 0; n <= 3; n++)
        {
            int x = coords[n][0];
            int y = coords[n][1];

            if (x >= 0 && x <= awidth && y >= 0 && y <= aheight)
                values[n] = arr[x][y];
            else
            {
                if (x > awidth && y >= 0 && y <= aheight)
                {
                    if (top)
                        values[n] = cubearr[1][aheight - y][x - awidth];
                    else
                        values[n] = cubearr[1][y][aheight - (x - awidth)];
                }

                if (x < awidth && y >= 0 && y <= aheight)
                {
                    if (top)
                        values[n] = cubearr[3][y][-x];
                    else
                        values[n] = cubearr[3][aheight - y][aheight - (x - awidth)];
                }

                if (y > aheight && x >= 0 && x <= awidth)
                {
                    if (top)
                        values[n] = cubearr[0][x][y - aheight];
                    else
                    {
                        values[n] = cubearr[2][awidth - x][aheight - (y - aheight)];
                    }
                }

                if (y < 0 && x >= 0 && x <= awidth)
                {
                    if (top)
                        values[n] = cubearr[2][awidth - x][-y];
                    else
                        values[n] = cubearr[0][x][aheight + y];
                }
            }
        }

        int a = 0;

        if (values[0] != nullval && values[1] != nullval)
            a = wrappedaverage(values[0], values[1], max);
        else
        {
            if (values[0] == nullval)
                a = values[1];
            else
                a = values[0];
        }

        int b = 0;

        if (values[2] != nullval && values[3] != nullval)
            b = wrappedaverage(values[2], values[3], max);
        else
        {
            if (values[2] == nullval)
                b = values[3];
            else
                b = values[2];
        }

        if (a != nullval && b != nullval)
            total = wrappedaverage(a, b, max);
        else
        {
            if (a == nullval)
                total = b;
            else
                total = a;
        }
    }
    else
    {
        for (int n = 0; n <= 3; n++)
        {
            int x = coords[n][0];
            int y = coords[n][1];

            int val = 0;

            if (x >= 0 && x <= awidth && y >= 0 && y <= aheight)
                val = arr[x][y];
            else
            {
                if (x > awidth && y >= 0 && y <= aheight)
                {
                    if (top)
                        val = cubearr[1][aheight - y][x - awidth];
                    else
                        val = cubearr[1][y][aheight - (x - awidth)];
                }

                if (x < awidth && y >= 0 && y <= aheight)
                {
                    if (top)
                        val = cubearr[3][y][-x];
                    else
                        val = cubearr[3][aheight - y][aheight - (x - awidth)];
                }

                if (y > aheight && x >= 0 && x <= awidth)
                {
                    if (top)
                        val = cubearr[0][x][y - aheight];
                    else
                    {
                        val = cubearr[2][awidth - x][aheight - (y - aheight)];
                    }
                }

                if (y < 0 && x >= 0 && x <= awidth)
                {
                    if (top)
                        val = cubearr[2][awidth - x][-y];
                    else
                        val = cubearr[0][x][aheight + y];
                }
            }

            total = total + val;
            crount++;
        }

        total = total / crount;
    }

    int difference = randomsign(random(0, value));

    total = total + difference;

    if (wrapped == 1)
    {
        while (total > max)
            total = total - max;

        while (total < min)
            total = total + max;

    }
    else
    {
        if (total > max)
            total = max;

        if (total < min)
            total = min;
    }

    return (total);
}

// This does the diamond part of the lid fractal.

int liddiamond(vector<int>& arr, vector<int>& cubearr, int edge, int awidth, int aheight, int s, int x, int y, int value, int min, int max, bool wrapped, bool top)
{
    value = value * 50;
    int dist = s / 2;
    int total = 0;
    int crount = 0;

    int coords[4][2];

    coords[0][0] = x;
    coords[0][1] = y - dist;

    coords[1][0] = x + dist;
    coords[1][1] = y;

    coords[2][0] = x;
    coords[2][1] = y + dist;

    coords[3][0] = x - dist;
    coords[3][1] = y;

    int f1 = edge * edge;
    int f2 = edge * edge * 2;
    int f3 = edge * edge * 3;
    int f4 = edge * edge * 4;
    int f5 = edge * edge * 5;

    if (wrapped == 1)
    {
        int values[4];

        int nullval = 0 - max * 10;

        for (int n = 0; n < 4; n++)
            values[n] = nullval;

        for (int n = 0; n <= 3; n++)
        {
            int x = coords[n][0];
            int y = coords[n][1];

            if (x >= 0 && x <= awidth && y >= 0 && y <= aheight)
                values[n] = arr[x * edge + y];
            else
            {
                if (x > awidth && y >= 0 && y <= aheight)
                {
                    if (top)
                        values[n] = cubearr[f1 + (aheight - y) * edge + (x - awidth)];
                    else
                        values[n] = cubearr[f1 + y * edge + (aheight - (x - awidth))];
                }

                if (x < awidth && y >= 0 && y <= aheight)
                {
                    if (top)
                        values[n] = cubearr[f3 + y * edge - x];
                    else
                        values[n] = cubearr[f3 + (aheight - y) * edge + (aheight - (x - awidth))];
                }

                if (y > aheight && x >= 0 && x <= awidth)
                {
                    if (top)
                        values[n] = cubearr[x * edge + (y - aheight)];
                    else
                    {
                        values[n] = cubearr[f2 + (awidth - x) * edge + (aheight - (y - aheight))];
                    }
                }

                if (y < 0 && x >= 0 && x <= awidth)
                {
                    if (top)
                        values[n] = cubearr[f2 + (awidth - x) * edge - y];
                    else
                        values[n] = cubearr[x * edge + (aheight + y)];
                }
            }
        }

        int a = 0;

        if (values[0] != nullval && values[1] != nullval)
            a = wrappedaverage(values[0], values[1], max);
        else
        {
            if (values[0] == nullval)
                a = values[1];
            else
                a = values[0];
        }

        int b = 0;

        if (values[2] != nullval && values[3] != nullval)
            b = wrappedaverage(values[2], values[3], max);
        else
        {
            if (values[2] == nullval)
                b = values[3];
            else
                b = values[2];
        }

        if (a != nullval && b != nullval)
            total = wrappedaverage(a, b, max);
        else
        {
            if (a == nullval)
                total = b;
            else
                total = a;
        }
    }
    else
    {
        for (int n = 0; n <= 3; n++)
        {
            int x = coords[n][0];
            int y = coords[n][1];

            int val;

            if (x >= 0 && x <= awidth && y >= 0 && y <= aheight)
                val = arr[x * edge + y];
            else
            {
                if (x > awidth && y >= 0 && y <= aheight)
                {
                    if (top)
                        val = cubearr[f1 + (aheight - y) * edge + (x - awidth)];
                    else
                        val = cubearr[f1 + y * edge + (aheight - (x - awidth))];
                }

                if (x < awidth && y >= 0 && y <= aheight)
                {
                    if (top)
                        val = cubearr[f3 + y * edge - x];
                    else
                        val = cubearr[f3 + (aheight - y) * edge + (aheight - (x - awidth))];
                }

                if (y > aheight && x >= 0 && x <= awidth)
                {
                    if (top)
                        val = cubearr[x * edge + (y - aheight)];
                    else
                    {
                        val = cubearr[f2 + (awidth - x) * edge + (aheight - (y - aheight))];
                    }
                }

                if (y < 0 && x >= 0 && x <= awidth)
                {
                    if (top)
                        val = cubearr[f2 + (awidth - x) * edge - y];
                    else
                        val = cubearr[x * edge + (aheight + y)];
                }
            }

            total = total + val;
            crount++;
        }

        total = total / crount;
    }

    int difference = randomsign(random(0, value));

    total = total + difference;

    if (wrapped == 1)
    {
        while (total > max)
            total = total - max;

        while (total < min)
            total = total + max;
    }
    else
    {
        if (total > max)
            total = max;

        if (total < min)
            total = min;
    }

    return (total);
}

// Deprecated version of the above, for 3D vectors.

int liddiamondold(vector<vector<int>>& arr, vector<vector<vector<int>>>& cubearr, int awidth, int aheight, int s, int x, int y, int value, int min, int max, bool wrapped, bool top)
{
    value = value * 50;
    int dist = s / 2;
    int total = 0;
    int crount = 0;

    int coords[4][2];

    coords[0][0] = x;
    coords[0][1] = y - dist;

    coords[1][0] = x + dist;
    coords[1][1] = y;

    coords[2][0] = x;
    coords[2][1] = y + dist;

    coords[3][0] = x - dist;
    coords[3][1] = y;

    if (wrapped == 1)
    {
        int values[4];

        int nullval = 0 - max * 10;

        for (int n = 0; n < 4; n++)
            values[n] = nullval;

        for (int n = 0; n <= 3; n++)
        {
            int x = coords[n][0];
            int y = coords[n][1];

            if (x >= 0 && x <= awidth && y >= 0 && y <= aheight)
                values[n] = arr[x][y];
            else
            {
                if (x > awidth && y >= 0 && y <= aheight)
                {
                    if (top)
                        values[n] = cubearr[1][aheight - y][x - awidth];
                    else
                        values[n] = cubearr[1][y][aheight - (x - awidth)];
                }

                if (x < awidth && y >= 0 && y <= aheight)
                {
                    if (top)
                        values[n] = cubearr[3][y][-x];
                    else
                        values[n] = cubearr[3][aheight - y][aheight - (x - awidth)];
                }

                if (y > aheight && x >= 0 && x <= awidth)
                {
                    if (top)
                        values[n] = cubearr[0][x][y - aheight];
                    else
                    {
                        values[n] = cubearr[2][awidth - x][aheight - (y - aheight)];
                    }
                }

                if (y < 0 && x >= 0 && x <= awidth)
                {
                    if (top)
                        values[n] = cubearr[2][awidth - x][-y];
                    else
                        values[n] = cubearr[0][x][aheight + y];
                }
            }
        }

        int a = 0;

        if (values[0] != nullval && values[1] != nullval)
            a = wrappedaverage(values[0], values[1], max);
        else
        {
            if (values[0] == nullval)
                a = values[1];
            else
                a = values[0];
        }

        int b = 0;

        if (values[2] != nullval && values[3] != nullval)
            b = wrappedaverage(values[2], values[3], max);
        else
        {
            if (values[2] == nullval)
                b = values[3];
            else
                b = values[2];
        }

        if (a != nullval && b != nullval)
            total = wrappedaverage(a, b, max);
        else
        {
            if (a == nullval)
                total = b;
            else
                total = a;
        }
    }
    else
    {
        for (int n = 0; n <= 3; n++)
        {
            int x = coords[n][0];
            int y = coords[n][1];

            int val;

            if (x >= 0 && x <= awidth && y >= 0 && y <= aheight)
                val = arr[x][y];
            else
            {
                if (x > awidth && y >= 0 && y <= aheight)
                {
                    if (top)
                        val = cubearr[1][aheight - y][x - awidth];
                    else
                        val = cubearr[1][y][aheight - (x - awidth)];
                }

                if (x < awidth && y >= 0 && y <= aheight)
                {
                    if (top)
                        val = cubearr[3][y][-x];
                    else
                        val = cubearr[3][aheight - y][aheight - (x - awidth)];
                }

                if (y > aheight && x >= 0 && x <= awidth)
                {
                    if (top)
                        val = cubearr[0][x][y - aheight];
                    else
                    {
                        val = cubearr[2][awidth - x][aheight - (y - aheight)];
                    }
                }

                if (y < 0 && x >= 0 && x <= awidth)
                {
                    if (top)
                        val = cubearr[2][awidth - x][-y];
                    else
                        val = cubearr[0][x][aheight + y];
                }
            }

            total = total + val;
            crount++;
        }

        total = total / crount;
    }

    int difference = randomsign(random(0, value));

    total = total + difference;

    if (wrapped == 1)
    {
        while (total > max)
            total = total - max;

        while (total < min)
            total = total + max;
    }
    else
    {
        if (total > max)
            total = max;

        if (total < min)
            total = min;
    }

    return (total);
}

// This function creates a new 2D fractal map.

void create2dfractal(vector<int>& arr, int edge, int awidth, int aheight, int grain, float valuemod, float valuemod2, int min, int max, bool extreme, bool wrapped)
{
    newfractalinit(arr, edge, awidth, aheight, grain, min, max, extreme);

    newfractal(arr, edge, awidth, aheight, grain, valuemod, valuemod2, min, max, wrapped);
}

// Deprecated version.

void create2dfractalold(vector<vector<int>>& arr, int awidth, int aheight, int grain, float valuemod, float valuemod2, int min, int max, bool extreme, bool wrapped)
{
    newfractalinitold(arr, awidth, aheight, grain, min, max, extreme);

    newfractalold(arr, awidth, aheight, grain, valuemod, valuemod2, min, max, wrapped);
}

// Draws a shape from an image onto the map.

void drawshape(planet& world, int shapenumber, int face, int centrex, int centrey, bool land, int baseheight, int conheight, boolshapetemplate shape[])
{
    int edge = world.edge();

    int imheight = shape[shapenumber].ysize() - 1;
    int imwidth = shape[shapenumber].xsize() - 1;

    vector<vector<vector<bool>>> drawmap(6, vector<vector<bool>>(edge, vector<bool>(edge, 0))); // First draw the shape on this. If there are no problems we'll copy it onto the world map, otherwise we'll just abandon it.

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

            if (shape[shapenumber].point(i, j) == 1)
            {
                int ii = imap;
                int jj = jmap;

                if (flipx)
                    ii = -ii;

                if (flipy)
                    jj = -jj;
                
                globepoint thispoint = getglobepoint(edge, startpoint.face, startpoint.x, startpoint.y, ii, jj);
                
                if (thispoint.face!=-1)
                    drawmap[thispoint.face][thispoint.x][thispoint.y] = 1;

                /*
                if (thispoint.face == -1)
                    return;
                else
                    drawmap[thispoint.face][thispoint.x][thispoint.y] = 1;
                */
            }
        }
    }

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (drawmap[face][i][j] == 1)
                {
                    if (land == 1)
                    {
                        world.setnom(face, i, j, conheight);
                        world.settest(face, i, j, 100);
                    }
                    else
                    {
                        world.setnom(face, i, j, baseheight);
                        world.setmountainheight(face, i, j, 0);
                        world.setmountainridge(face, i, j, 0);
                    }
                }
            }
        }
    }
}

// Draws a shape from an image onto the map and marks it on another array.

void drawmarkedshape(planet& world, int shapenumber, int face, int centrex, int centrey, bool land, int baseheight, int conheight, vector<vector<vector<bool>>>& markedmap, boolshapetemplate shape[])
{
    int edge = world.edge();

    int imheight = shape[shapenumber].ysize() - 1;
    int imwidth = shape[shapenumber].xsize() - 1;

    vector<vector<vector<bool>>> drawmap(6, vector<vector<bool>>(edge, vector<bool>(edge, 0))); // First draw the shape on this. If there are no problems we'll copy it onto the world map, otherwise we'll just abandon it.

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

            if (shape[shapenumber].point(i, j) == 1)
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
                    drawmap[thispoint.face][thispoint.x][thispoint.y] = 1;
            }
        }
    }

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (drawmap[face][i][j] == 1)
                {
                    if (land == 1)
                    {
                        world.setnom(face, i, j, conheight);
                        markedmap[face][i][j] = 1;
                    }
                    else
                    {
                        world.setnom(face, i, j, baseheight);
                        world.setmountainheight(face, i, j, 0);
                        world.setmountainridge(face, i, j, 0);
                    }
                }
            }
        }
    }
}

// This function makes the continents in the original, rather fragmented style.

void smallcontinents(planet& world, int baseheight, int conheight, vector<vector<vector<int>>>& fractal, vector<vector<vector<int>>>& plateaumap, boolshapetemplate landshape[], boolshapetemplate chainland[])
{
    int edge = world.edge();

    twointegers focuspoints[4]; // These will be points where continents etc will start close to.

    int focustotal = random(2, 4); // The number of actual focus points.

    for (int n = 0; n < focustotal; n++)
    {
        focuspoints[n].x = random(0, edge * 4 - 1);
        focuspoints[n].y = random(0, edge * 3 - 1);
    }

    float focaldistance = (float)(edge / 2); // Maximum distance a continent can start from the focuspoint.

    int maxblobno = edge / 10; // Number of blobs per continent.
    int minblobno = 5;
    int bloboffset = 20; // Distance new blobs might be from the last one.

    int contno = random((edge * 2 + 1) / 1024, (edge * 2 + 1) / 64); // Number of continents

    if (random(1, 10) == 1) // Small chance it might have considerably more.
        contno = random((edge * 2 + 1) / 1024, (edge * 2 + 1) / 64);

    int cutno = edge / 125; // Number of cuts.

    // First, make the whole map our seabed baseheight.

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
                world.setnom(face, i, j, baseheight);
        }
    }

    // Now, make some mountain chains with associated land.

    createchains(world, baseheight, conheight, fractal, landshape, chainland, focuspoints, focustotal, (int)focaldistance, 0);

    // Now we draw the basic shapes of the continents.

    for (int cont = 1; cont <= contno; cont++)
    {
        int thisfocus = random(0, focustotal);

        int bx = focuspoints[thisfocus].x;
        int by = focuspoints[thisfocus].y; // Coordinates of the current blob.
        int bface = bx / edge;
        bx = bx - (bx / edge);

        float xdiff = (float)random(1, 100);
        float ydiff = (float)random(1, 100);

        xdiff = xdiff / 100.0f;
        ydiff = ydiff / 100.0f;

        xdiff = xdiff * xdiff;
        ydiff = ydiff * ydiff;

        xdiff = xdiff * focaldistance;
        ydiff = ydiff * focaldistance;

        globepoint bpoint = getglobepoint(edge, bface, bx, by, (int)randomsign(xdiff), (int)randomsign(ydiff));

        if (bpoint.face != -1)
        {
            for (int i = 1; i <= random(minblobno, maxblobno); i++) // Create a number of blobs to form this continent.
            {
                for (int j = 1; j <= 10; j++) // Move the location of the current blob.
                {
                    if (random(1, 4) == 1)
                        bpoint = getglobepoint(edge, bpoint.face, bpoint.x, bpoint.y, randomsign(random(1, bloboffset)), randomsign(random(1, bloboffset)));
                }

                int shapenumber = random(1, 11);

                drawshape(world, shapenumber, bpoint.face, bpoint.x, bpoint.y, 1, baseheight, conheight, landshape);
            }
        }
    }

    // Now for cuts!

    cuts(world, cutno, baseheight, conheight, landshape);

    // Now remove any lines of sea/land that appear at the edges of faces.

    removeedgelines(world, baseheight, conheight);

    // Now remove other straight lines.

    removebigstraights(world, baseheight, conheight, landshape);

    // Now we add smaller mountain chains that might form peninsulas.

    createchains(world, baseheight, conheight, fractal, landshape, chainland, focuspoints, focustotal, (int)focaldistance, 1);
}

// This function makes the continents in a larger style.

void largecontinents(planet& world, int baseheight, int conheight, int clusterno, int clustersize, vector<vector<vector<int>>>& fractal, vector<vector<vector<int>>>& plateaumap, vector<vector<vector<bool>>>& shelves, vector<float>& longitude, vector<float>& latitude, vector<fourglobepoints>& dirpoint, boolshapetemplate landshape[], boolshapetemplate chainland[], int& progressval, string& progresstext)
{
    int edge = world.edge();
    int sealevel = world.sealevel();
    int maxelev = world.maxelevation();

    int movein = 300; // Maximum amount to move neighbouring continents towards the central one.
    int origmountainschance = 1; //2; // The higher this is, the less often central continents will have mountains down one side.
    int mountainschance = 2; // 4; // The higher this is, the less often other continents will have mountains down one side.

    int contwidth = edge * 4; // Size of the array that we'll draw continents on.
    int contheight = edge * 2;

    updatereport(progressval, progresstext, "Preparing Voronoi map");

    vector<vector<int>> voronoi(contwidth + 1, vector<int>(contheight + 1, 0));
    int points = 200; // Number of points in the voronoi map

    make2Dvoronoi(voronoi, edge * 4 - 1, edge * 2 - 1, points);

    vector<vector<bool>> continent(contwidth + 1, vector<bool>(contheight + 1, 0));
    vector<vector<vector<short>>> continentnos(6, vector<vector<short>>(edge, vector<short>(edge, 0)));
    vector<vector<vector<short>>> overlaps(6, vector<vector<short>>(edge, vector<short>(edge, 0)));

    updatereport(progressval, progresstext, "Making continents");

    const int maxfocuspoints = 55;

    globepoint focuspoints[maxfocuspoints]; // These will be points where continents etc will start close to.

    int focustotal;

    if (clusterno == -1)
    {
        if (random(1, 2) == 1)
            focustotal = maxfocuspoints / 2 + 1;
        else
        {
            if (random(1, 2) == 1)
                focustotal = random(maxfocuspoints / 3, maxfocuspoints - maxfocuspoints / 3);
            else
                focustotal = random(1, maxfocuspoints);
        }
    }
    else
        focustotal = clusterno;

    //focustotal = maxfocuspoints;

    for (int n = 0; n < maxfocuspoints; n++) // It's possible to have different clusters on the same face!
    {
        focuspoints[n].face = random(0, 5);
        focuspoints[n].x = random(edge / 6, edge - edge / 6);
        focuspoints[n].y = random(edge / 6, edge - edge / 6);
    }

    // First, make the whole map our seabed baseheight.
    
    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
                world.setnom(face, i, j, baseheight);
        }
    }

    // Now we draw the basic shapes of the continents, and paste them onto the map.

    int leftx = 0;
    int rightx = 0;
    int lefty = 0;
    int righty = 0; // Define the size and shape of the current continent.

    short thiscontinent = 0;

    for (int thispoint = 0; thispoint < focustotal; thispoint++) // Go through the focus points one by one.
    {
        vector<vector<vector<bool>>> drawmap(6, vector<vector<bool>>(edge, vector<bool>(edge, 0))); // First draw the continent on this. If there are no problems we'll copy it onto the world map, otherwise we'll just abandon it.

        // First, put the central continent for this grouping onto the map.

        thiscontinent++;

        makecontinent(world, continent, voronoi, points, contwidth, contheight, leftx, rightx, lefty, righty);

        int origcontwidth = rightx - leftx;
        int origcontheight = righty - lefty;

        globepoint origstartpoint = getglobepoint(edge, focuspoints[thispoint].face, focuspoints[thispoint].x, focuspoints[thispoint].y, 0 - origcontwidth / 2, 0 - origcontheight / 2);

        bool gonewrong = 0;

        if (origstartpoint.face != -1)
        {
            int thisleft = -1;
            int thisright = -1;
            int thisup = -1;
            int thisdown = -1;

            bool flipx = 0; // This is in case the origin point is on another face and we may need to change the direction of the shape.
            bool flipy = 0;

            if (focuspoints[thispoint].face == 1 && origstartpoint.face == 4)
                flipy = 1;

            if (focuspoints[thispoint].face == 2 && origstartpoint.face == 4)
            {
                flipx = 1;
                flipy = 1;
            }

            if (focuspoints[thispoint].face == 3 && origstartpoint.face == 4)
                flipx = 1;

            if (focuspoints[thispoint].face == 4 && origstartpoint.face == 2)
            {
                flipx = 1;
                flipy = 1;
            }

            if (focuspoints[thispoint].face == 4 && origstartpoint.face == 3)
                flipy = 1;

            if (focuspoints[thispoint].face == 5 && origstartpoint.face == 3)
                flipx = 1;

            int startface = focuspoints[thispoint].face;

            bool leftr = random(0, 1); // If it's 1 then we reverse it left-right
            bool downr = random(0, 1); // If it's 1 then we reverse it top-bottom

            int istart = leftx, desti = rightx, istep = 1;
            int jstart = lefty, destj = righty, jstep = 1;

            if (leftr == 1)
            {
                istart = rightx;
                desti = leftx;
                istep = -1;
            }

            if (downr == 1)
            {
                jstart = righty;
                destj = lefty;
                jstep = -1;
            }

            int imap = -1;
            int jmap = -1;

            float size = 1.0f; // ((float)random(50, 100)) / 100.0f;

            for (int i = istart; i != desti; i = i + istep)
            {
                imap++;
                jmap = -1;

                for (int j = jstart; j != destj; j = j + jstep)
                {
                    jmap++;

                    if (continent[imap + leftx][jmap + lefty] == 1)
                    {
                        float ii = (float)imap;
                        float jj = (float)jmap;

                        if (flipx)
                            ii = -ii;

                        if (flipy)
                            jj = -jj;

                        ii = ii * size;
                        jj = jj * size;

                        globepoint thispoint = getglobepoint(edge, origstartpoint.face, origstartpoint.x, origstartpoint.y, (int)ii, (int)jj);

                        if (thispoint.face == -1)
                        {
                            gonewrong = 1;
                        }
                        else
                            drawmap[thispoint.face][thispoint.x][thispoint.y] = 1;
                    }
                }
            }

            int origstartpointx = thisleft;
            int origstartpointy = thisup;

            origcontwidth = thisright - thisleft;
            origcontheight = thisdown - thisup;
        }
        else
            gonewrong = 1;

        if (gonewrong == 0)
        {
            for (int face = 0; face < 6; face++)
            {
                for (int i = 0; i < edge; i++)
                {
                    for (int j = 0; j < edge; j++)
                    {
                        if (drawmap[face][i][j] == 1)
                        {
                            world.setnom(face, i, j, conheight);

                            if (continentnos[face][i][j] != 0)
                            {
                                short overlap = continentnos[face][i][j] * 100 + thiscontinent;
                                overlaps[face][i][j] = overlap;
                            }

                            continentnos[face][i][j] = thiscontinent;
                        }
                    }
                }
            }
        } 

        // Now put other continents around it.

        bool fringeconts[3][3];

        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
                fringeconts[i][j] = 0;
        }

        short extracont = random(1, 9) - 1; // Number of extra continents.

        if (clustersize != -1)
        {
            extracont = clustersize - 1;

            thiscontinent++;

            makecontinent(world, continent, voronoi, points, contwidth, contheight, leftx, rightx, lefty, righty);

            int thiscontwidth = rightx - leftx;
            int thiscontheight = righty - lefty;

            int dir = random(1, 8); // Direction from the central continent.

            bool keepgoing = 1;

            switch (dir)
            {
            case 1:
                if (fringeconts[1][0] == 1)
                    keepgoing = 0;
                break;

            case 2:
                if (fringeconts[2][0] == 1)
                    keepgoing = 0;
                break;

            case 3:
                if (fringeconts[2][1] == 1)
                    keepgoing = 0;
                break;

            case 4:
                if (fringeconts[2][2] == 1)
                    keepgoing = 0;
                break;

            case 5:
                if (fringeconts[1][2] == 1)
                    keepgoing = 0;
                break;

            case 6:
                if (fringeconts[0][2] == 1)
                    keepgoing = 0;
                break;

            case 7:
                if (fringeconts[0][1] == 1)
                    keepgoing = 0;
                break;

            case 8:
                if (fringeconts[0][0] == 1)
                    keepgoing = 0;
                break;
            }

            if (keepgoing)
            {
                int shiftx = 0; // How far the centre of this continent should be from the centre of the central one.
                int shifty = 0;

                int maxvar = edge / 6;

                switch (dir)
                {
                case 1: // north
                    shiftx = randomsign(random(1, maxvar));
                    shifty = randomsign(random(1, maxvar)) - origcontheight / 2;
                    fringeconts[1][0] = 1;
                    break;

                case 2: // northeast
                    shiftx = randomsign(random(1, maxvar)) + origcontwidth / 2;
                    shifty = randomsign(random(1, maxvar)) - origcontheight / 2;
                    fringeconts[2][0] = 1;
                    break;

                case 3: // east
                    shiftx = randomsign(random(1, maxvar)) + origcontwidth / 2;
                    shifty = randomsign(random(1, maxvar));
                    fringeconts[2][1] = 1;
                    break;

                case 4: // southeast
                    shiftx = randomsign(random(1, maxvar)) + origcontwidth / 2;
                    shifty = randomsign(random(1, maxvar)) + origcontheight / 2;
                    fringeconts[2][2] = 1;
                    break;

                case 5: // south
                    shiftx = randomsign(random(1, maxvar));
                    shifty = randomsign(random(1, maxvar)) + origcontheight / 2;
                    fringeconts[1][2] = 1;
                    break;

                case 6: // southwest
                    shiftx = randomsign(random(1, maxvar)) - origcontwidth / 2;
                    shifty = randomsign(random(1, maxvar)) + origcontheight / 2;
                    fringeconts[0][2] = 1;
                    break;

                case 7: // west
                    shiftx = randomsign(random(1, maxvar)) - origcontwidth / 2;
                    shifty = randomsign(random(1, maxvar));
                    fringeconts[0][1] = 1;
                    break;

                case 8: // northwest
                    shiftx = randomsign(random(1, maxvar)) - origcontwidth / 2;
                    shifty = randomsign(random(1, maxvar)) - origcontheight / 2;
                    fringeconts[0][0] = 1;
                    break;
                }

                for (int n = 1; n <= extracont; n++)
                {
                    for (int face = 0; face < 6; face++)
                    {
                        for (int i = 0; i < edge; i++)
                        {
                            for (int j = 0; j < edge; j++)
                                drawmap[face][i][j] = 0;
                        }
                    }
                }

                globepoint thisstartpoint = getglobepoint(edge, focuspoints[thispoint].face, focuspoints[thispoint].x, focuspoints[thispoint].y, shiftx - thiscontwidth / 2, shifty - thiscontheight / 2);

                bool flipx = 0; // This is in case the origin point is on another face and we may need to change the direction of the shape.
                bool flipy = 0;

                if (focuspoints[thispoint].face == 1 && thisstartpoint.face == 4)
                    flipy = 1;

                if (focuspoints[thispoint].face == 2 && thisstartpoint.face == 4)
                {
                    flipx = 1;
                    flipy = 1;
                }

                if (focuspoints[thispoint].face == 3 && thisstartpoint.face == 4)
                    flipx = 1;

                if (focuspoints[thispoint].face == 4 && thisstartpoint.face == 2)
                {
                    flipx = 1;
                    flipy = 1;
                }

                if (focuspoints[thispoint].face == 4 && thisstartpoint.face == 3)
                    flipy = 1;

                if (focuspoints[thispoint].face == 5 && thisstartpoint.face == 3)
                    flipx = 1;

                bool leftr = random(0, 1); // If it's 1 then we reverse it left-right
                bool downr = random(0, 1); // If it's 1 then we reverse it top-bottom

                int istart = leftx, desti = rightx, istep = 1;
                int jstart = lefty, destj = righty, jstep = 1;

                if (leftr == 1)
                {
                    istart = rightx;
                    desti = leftx;
                    istep = -1;
                }

                if (downr == 1)
                {
                    jstart = righty;
                    destj = lefty;
                    jstep = -1;
                }

                int imap = -1;
                int jmap = -1;

                gonewrong = 0;

                float size = ((float)random(50, 100)) / 100.0f;

                for (int i = istart; i != desti; i = i + istep)
                {
                    imap++;
                    jmap = -1;

                    for (int j = jstart; j != destj; j = j + jstep)
                    {
                        jmap++;

                        if (continent[imap + leftx][jmap + lefty] == 1)
                        {
                            float ii = (float)imap;
                            float jj = (float)jmap;

                            if (flipx)
                                ii = -ii;

                            if (flipy)
                                jj = -jj;

                            ii = ii * size;
                            jj = jj * size;

                            globepoint thispoint = getglobepoint(edge, thisstartpoint.face, thisstartpoint.x, thisstartpoint.y, (int)ii, (int)jj);

                            if (thispoint.face == -1)
                            {
                                gonewrong = 1;
                                //i = desti;
                                //j = destj;
                            }
                            else
                                drawmap[thispoint.face][thispoint.x][thispoint.y] = 1;
                        }
                    }
                }

                if (gonewrong == 0)
                {
                    for (int face = 0; face < 6; face++)
                    {
                        for (int i = 0; i < edge; i++)
                        {
                            for (int j = 0; j < edge; j++)
                            {
                                if (drawmap[face][i][j] == 1)
                                {
                                    world.setnom(face, i, j, conheight);

                                    if (continentnos[face][i][j] != 0)
                                    {
                                        short overlap = continentnos[face][i][j] * 100 + thiscontinent;
                                        overlaps[face][i][j] = overlap;
                                    }

                                    continentnos[face][i][j] = thiscontinent;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Now remove any lines of sea/land that appear at the edges of faces.

    removeedgelines(world, baseheight, conheight);

    // Now remove other straight lines.

    removebigstraights(world, baseheight, conheight, landshape);

    // Now remove inland seas.

    updatereport(progressval, progresstext, "Removing inland seas");

    removeinlandseas(world, conheight);

    // Now we add mountain ranges along the edges of some continents.

    updatereport(progressval, progresstext, "Adding continental mountain ranges");

    vector<vector<vector<int>>> mountainsraw(6, vector<vector<int>>(edge, vector<int>(edge, 0)));

    markedgemountains(world, mountainsraw);

    int mountainlandchance = 4; // Probability of putting a chunk of land at this mountain point.
    int maxmountainlandmove = 4; // Maximum amount the centre of the chunk of land may be moved.

    for (int face = 0; face < 6; face++) // Add blobs of land around where these mountains will go.
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (mountainsraw[face][i][j] != 0 && random(1, mountainlandchance) == 1)
                {
                    int shiftx = randomsign(random(1, maxmountainlandmove));
                    int shifty = randomsign(random(1, maxmountainlandmove));

                    globepoint landcentre = getglobepoint(edge, face, i, j, shiftx, shifty);

                    if (landcentre.face != -1)
                    {
                        int templateno = random(0, 1);

                        drawshape(world, templateno, landcentre.face, landcentre.x, landcentre.y, 1, baseheight, conheight, chainland);
                    }
                }
            }
        }
    }

    removeedgelines(world, baseheight, conheight);

    // Now turn all those into actual mountains.

    for (int face = 0; face < 6; face++)
        createmountainsfromraw(world, face, mountainsraw);

    removefloatingmountains(world);
    //cleanmountainridges(world);

    // Now we add mountain ranges across continents.

    createcentralmountains(world, baseheight, conheight, landshape, chainland);

    // Now we add smaller mountain chains that might form peninsulas.
  
    twointegers mountainfocuspoints[8]; // These will be points where mountains and islands will start close to.

    int mountainfocustotal = random(2, 8); // The number of actual focus points.

    for (int n = 0; n < mountainfocustotal; n++)
    {
        mountainfocuspoints[n].x = random(0, edge * 4 - 1);
        mountainfocuspoints[n].y = random(0, edge * 3 - 1);

        while (mountainfocuspoints[n].x > edge && (mountainfocuspoints[n].y<edge || mountainfocuspoints[n].y>edge * 2))
        {
            mountainfocuspoints[n].x = random(0, edge * 4 - 1);
            mountainfocuspoints[n].y = random(0, edge * 3 - 1);
        }
    }

    int mountainfocaldistance = edge / 2;
    
    createchains(world, baseheight, conheight, fractal, landshape, chainland, mountainfocuspoints, mountainfocustotal, mountainfocaldistance, 1);

    // Now we do the continental shelves.

    updatereport(progressval, progresstext, "Making continental shelves");

    makecontinentalshelves(world, shelves, dirpoint, 4, landshape);

    // Now we add some island chains.

    int islandgroups = random(0, 6); // Number of general groups of islands.

    for (int n = 1; n <= islandgroups; n++)
    {
        mountainfocustotal = 1; // The number of actual focus points.

        for (int n = 0; n < mountainfocustotal; n++)
        {
            mountainfocuspoints[n].x = random(0, edge * 4 - 1);
            mountainfocuspoints[n].y = random(0, edge * 3 - 1);

            while (mountainfocuspoints[n].x > edge && (mountainfocuspoints[n].y<edge || mountainfocuspoints[n].y>edge * 2))
            {
                mountainfocuspoints[n].x = random(0, edge * 4 - 1);
                mountainfocuspoints[n].y = random(0, edge * 3 - 1);
            }
        }

        mountainfocaldistance = edge / 2;

        createchains(world, baseheight, conheight, fractal, landshape, chainland, mountainfocuspoints, mountainfocustotal, mountainfocaldistance, 3);
    }
    
    // Now remove inland seas again!

    removeinlandseas(world, conheight);
}

// This function draws a (large-style) continent onto an array.

void makecontinent(planet& world, vector<vector<bool>>& continent, vector<vector<int>>& voronoi, int points, int width, int height, int& leftx, int& rightx, int& lefty, int& righty)
{
    vector<vector<bool>> circlemap(width + 1, vector<bool>(height + 1, 0));
    vector<vector<bool>> outline(width + 1, vector<bool>(height + 1, 0));

    vector<bool> cells(points); // vector<uint8_t>?

    for (int n = 0; n < points; n++)
        cells[n] = 0;

    int totalnodes = 1;

    vector <twointegers> bordernodes; // Stores the location of the nodes.
    vector <twointegers> nodeshifts; // Stores how much each node has been shifted from its default location.

    int circleno;

    if (random(1, 3) == 1)
        circleno = random(20, 100);
    else
        circleno = random(5, 50);

    int circlesize;

    circlesize = random(height / 30, height / 12);

    // First, draw a load of blobs to get an extremely rough shape.

    int circleleftx = width / 2;
    int circlerightx = width / 2;
    int circlelefty = height / 2;
    int circlerighty = height / 2;

    makecontinentcircles(circlemap, width, height, circleno, circlesize, circleleftx, circlerightx, circlelefty, circlerighty);

    if (circleleftx >= circlerightx || circlelefty >= circlerighty)
        return;

    if (circleleftx<0 || circlerightx>width || circlelefty<0 || circlerighty>height)
        return;

    // Shift the circles, so that not all continents are made with the same Voronoi cells.

    int circleswidth = circlerightx - circleleftx;
    int circlesheight = circlerighty - circlelefty;

    int rightshift = 0;
    int downshift = 0;

    if (circleswidth < width - width / 5 && circlesheight < height - height / 5)
    {
        int leftcirclestart = random(width / 10 + circleswidth, width - width / 10 - circleswidth * 2);

        rightshift = leftcirclestart - circleleftx;
    }

    // Now, take the Voronoi cells that overlap those blobs, to get the rough continent.

    for (int i = circleleftx; i <= circlerightx; i++)
    {
        for (int j = circlelefty; j <= circlerighty; j++)
        {
            if (circlemap[i][j] == 1)
            {
                int ii = i + rightshift;
                int jj = j + downshift;

                if (ii >= 0 && ii <= width && jj >= 0 && jj <= height)
                    cells[voronoi[ii][jj]] = 1;
            }
        }
    }

    for (int i = circleleftx; i <= circlerightx; i++)
    {
        for (int j = circlelefty; j <= circlerighty; j++)
        {
            int ii = i + rightshift;
            int jj = j + downshift;

            if (ii<0 || ii>width || jj<0 || jj>height)
                return;

            if (cells[voronoi[ii][jj]] == 1)
                continent[i][j] = 1;
        }
    }

    // Now we need to identify the outline.

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (continent[i][j] == 1)
            {
                bool found = 0;

                for (int k = i - 1; k <= i + 1; k++)
                {
                    for (int l = j - 1; l <= j + 1; l++)
                    {
                        if (k >= 0 && k <= width && l >= 0 && l <= height && continent[k][l] == 0)
                        {
                            found = 1;
                            k = i + 1;
                            l = j + 1;
                        }
                    }
                }

                if (found)
                    outline[i][j] = 1;

            }
        }
    }

    // Now we need to disrupt that outline to make it interesting.

    totalnodes = 1;
    short minnodegap = 5;
    short maxnodegap = 10; //15;
    int nodegap = random(minnodegap, maxnodegap); //(6,8); // This is the number of positions to skip when setting up the initial nodes.
    short gapvarychance = 2; // The distance between nodes may vary as it goes round!

    // First, find the first node.

    int x = width / 2;
    int y = height / 2;

    bool keepgoing = 1;

    bool mainlyup = random(1, 2) - 1;

    short up = randomsign(1);
    short left = randomsign(1);

    short uprandom = random(1, 5);
    short leftrandom = random(1, 5);

    do
    {
        if (mainlyup)
        {
            y = y + up;

            if (random(1, leftrandom) == 1)
                x = x + left;
        }
        else
        {
            x = x + left;

            if (random(1, uprandom) == 1)
                y = y + up;
        }

        if (x<0 || x>width || y<0 || y>width)
            return;

        if (outline[x][y] == 1)
            keepgoing = 0;


    } while (keepgoing);

    // Now get the list of nodes ready.

    twointegers node;
    node.x = x;
    node.y = y;

    twointegers shift;
    shift.x = 0;
    shift.y = 0;

    bordernodes.push_back(node); // Put the first node onto our list.
    nodeshifts.push_back(shift);

    keepgoing = 1;
    int crount = 0;

    do
    {
        // First, remove the point of outline that we're on.

        outline[node.x][node.y] = 0;

        // Next, we need to find the next one to move to.

        bool found = 0;

        if (node.y > 0 && outline[node.x][node.y - 1] == 1)
        {
            node.y--;
            found = 1;
        }

        if (found == 0 && node.x > 0 && outline[node.x - 1][node.y] == 1)
        {
            node.x--;
            found = 1;
        }

        if (found == 0 && node.y < width && outline[node.x][node.y + 1] == 1)
        {
            node.y++;
            found = 1;
        }

        if (found == 0 && node.x < width && outline[node.x + 1][node.y] == 1)
        {
            node.x++;
            found = 1;
        }

        if (found == 0 && node.x > 0 && node.y > 0 && outline[node.x - 1][node.y - 1] == 1)
        {
            node.x--;
            node.y--;
            found = 1;
        }

        if (found == 0 && node.x < width && node.y>0 && outline[node.x + 1][node.y - 1] == 1)
        {
            node.x++;
            node.y--;
            found = 1;
        }

        if (found == 0 && node.x > 0 && node.y < height && outline[node.x - 1][node.y + 1] == 1)
        {
            node.x--;
            node.y++;
            found = 1;
        }

        if (found == 0 && node.x < width && node.y < height && outline[node.x + 1][node.y + 1] == 1)
        {
            node.x++;
            node.y++;
            found = 1;
        }

        if (found == 0) // If we haven't found one, try looking a little further just in case.
        {
            for (int i = -2; i <= 2; i++)
            {
                int ii = node.x + i;

                for (int j = -2; j <= 2; j++)
                {
                    int jj = node.y + j;

                    if (ii >= 0 && ii <= width && jj >= 0 && jj <= height)
                    {
                        if (outline[ii][jj] == 1 && (ii != bordernodes[0].x || jj != bordernodes[0].y))
                        {
                            node.x = ii;
                            node.y = jj;
                            found = 1;
                        }
                    }
                }
            }
        }

        if (found == 0) // If we still haven't found one, try looking even further just in case.
        {
            for (int i = -4; i <= 4; i++)
            {
                int ii = node.x + i;

                for (int j = -4; j <= 4; j++)
                {
                    int jj = node.y + j;

                    if (ii >= 0 && ii <= width && jj >= 0 && jj <= height)
                    {
                        if (outline[ii][jj] == 1 && (ii != bordernodes[0].x || jj != bordernodes[0].y))
                        {
                            node.x = ii;
                            node.y = jj;
                            found = 1;
                        }
                    }
                }
            }
        }

        if (found == 0) // If we STILL didn't find one, we must have got to the end of the outline.
            keepgoing = 0;
        else
        {
            crount++;

            if (crount == nodegap) // If this is one to turn into a node
            {
                bordernodes.push_back(node);
                nodeshifts.push_back(shift); // This is just blank for now. We'll use it when we apply offsets.
                totalnodes++;
                crount = 0;

                if (random(1, gapvarychance) == 1)
                {
                    nodegap = nodegap + randomsign(1);

                    if (nodegap < minnodegap)
                        nodegap = minnodegap;

                    if (nodegap > maxnodegap)
                        nodegap = maxnodegap;
                }
            }
        }

    } while (keepgoing == 1);

    if (totalnodes == 1) // Sometimes it just gets a rogue one for some reason
        return;

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
            continent[i][j] = 0;
    }

    int maxdiff = 120; //80;

    if (abs(bordernodes[totalnodes - 1].x - bordernodes[0].x) > maxdiff)
        return;

    if (abs(bordernodes[totalnodes - 1].y - bordernodes[0].y) > maxdiff)
        return;

    // Now apply offsets to our nodes.

    int maxoffset = 40; //25; // No node can be offset further than this.

    int maxoffsetchange = random(5, 10); // No offset can be changed by more than this.

    int xoffset = 0;
    int yoffset = 0; // These are the amounts to offset the node by.

    int xoffsetchange = randomsign(random(0, maxoffsetchange));
    int yoffsetchange = randomsign(random(0, maxoffsetchange)); // These are the amounts to change the offset by each turn.

    int minmaxaddoffset = 2;
    int maxmaxaddoffset = 6; //15;
    short maxaddoffsetchangechance = 2; // This can change as it goes round.

    int maxaddoffset = random(minmaxaddoffset, maxmaxaddoffset); // Maximum additional offset a node can have.

    for (int node = 0; node < totalnodes - 1; node++)
    {
        if (random(1, maxaddoffsetchangechance) == 1)
        {
            maxaddoffset = maxaddoffset + randomsign(1);

            if (maxaddoffset < minmaxaddoffset)
                maxaddoffset = minmaxaddoffset;

            if (maxaddoffset > maxmaxaddoffset)
                maxaddoffset = maxmaxaddoffset;
        }

        int x = bordernodes[node].x;
        int y = bordernodes[node].y;

        x = x + xoffset + randomsign(random(0, maxaddoffset));
        y = y + yoffset + randomsign(random(0, maxaddoffset));

        if (x < bordernodes[node].x - maxoffset)
            x = bordernodes[node].x - maxoffset;

        if (x > bordernodes[node].x + maxoffset)
            x = bordernodes[node].x + maxoffset;

        if (y < bordernodes[node].y - maxoffset)
            y = bordernodes[node].y - maxoffset;

        if (y > bordernodes[node].y + maxoffset)
            y = bordernodes[node].y + maxoffset;

        nodeshifts[node].x = x - bordernodes[node].x;
        nodeshifts[node].y = y - bordernodes[node].y;

        bordernodes[node].x = x;
        bordernodes[node].y = y;

        // Change the offsets. Make them tend away from the extremes to ensure that they keep varying.

        if (xoffset > 0)
        {
            if (random(0, maxoffset) <= xoffset)
                xoffsetchange = xoffsetchange - random(0, maxoffsetchange);
            else
                xoffsetchange = xoffsetchange + random(0, maxoffsetchange);
        }
        else
        {
            if (random(0, maxoffset) <= abs(xoffset))
                xoffsetchange = xoffsetchange + random(0, maxoffsetchange);
            else
                xoffsetchange = xoffsetchange - random(0, maxoffsetchange);
        }

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

        xoffsetchange = xoffsetchange + randomsign(random(0, maxoffsetchange));
        yoffsetchange = yoffsetchange + randomsign(random(0, maxoffsetchange));

        if (xoffsetchange > maxoffsetchange)
            xoffsetchange = maxoffsetchange;

        if (xoffsetchange < -maxoffsetchange)
            xoffsetchange = -maxoffsetchange;

        if (yoffsetchange > maxoffsetchange)
            yoffsetchange = maxoffsetchange;

        if (yoffsetchange < -maxoffsetchange)
            yoffsetchange = -maxoffsetchange;

        xoffset = xoffset + xoffsetchange;
        yoffset = yoffset + yoffsetchange;

        if (xoffset > maxoffset)
            xoffset = maxoffset;

        if (xoffset < -maxoffset)
            xoffset = -maxoffset;

        if (yoffset > maxoffset)
            yoffset = maxoffset;

        if (yoffset < -maxoffset)
            yoffset = -maxoffset;
    }

    // Make the last node midway between the penultimate and the first.

    bordernodes[totalnodes - 1].x = (bordernodes[0].x + bordernodes[totalnodes - 2].x) / 2;
    bordernodes[totalnodes - 1].y = (bordernodes[0].y + bordernodes[totalnodes - 2].y) / 2;

    nodeshifts[totalnodes - 1].x = (nodeshifts[0].x + nodeshifts[totalnodes - 2].x) / 2;
    nodeshifts[totalnodes - 1].y = (nodeshifts[0].y + nodeshifts[totalnodes - 2].y) / 2;

    // Now we've got all our nodes. We need to draw splines between them.

    int midvar = 2; // Possible variation for the interpolated nodes. This is a fraction of the difference between the two nodes being interpolated, not an absolute value.

    twofloats pt, mm1, mm2, mm3;

    int nodeshiftedx = 0;
    int nodeshiftedy = 0; // This tells us how much the interpolated node is already shifted by. It's just the average of how much the other nodes are shifted by.

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
            outline[i][j] = 0;
    }

    leftx = width / 2;
    rightx = leftx;
    lefty = height / 2;
    righty = lefty;

    bool stopping = 0;

    int minmaxswerve = 1;
    int maxmaxswerve = 5;
    short maxswervechangechance = 2;

    int maxswerve = random(minmaxswerve, maxmaxswerve);

    for (int node = 0; node < totalnodes; node++) // We'll draw this on the outline array.
    {
        if (random(1, maxswervechangechance) == 1)
        {
            maxswerve = maxswerve + randomsign(1);

            if (maxswerve < minmaxswerve)
                maxswerve = minmaxswerve;

            if (maxswerve > maxmaxswerve)
                maxswerve = maxmaxswerve;
        }

        if (node == totalnodes - 1) // If we've run out of nodes to do, stop after this one
        {
            nodeshiftedx = (nodeshifts[node].x + nodeshifts[0].x) / 2;
            nodeshiftedy = (nodeshifts[node].y + nodeshifts[0].y) / 2;

            mm1.x = (float)bordernodes[node].x;
            mm1.y = (float)bordernodes[node].y;

            mm3.x = (float)bordernodes[0].x;
            mm3.y = (float)bordernodes[0].y;

            stopping = 1;
        }
        else
        {
            nodeshiftedx = (nodeshifts[node].x + nodeshifts[node + 1].x) / 2;
            nodeshiftedy = (nodeshifts[node].y + nodeshifts[node + 1].y) / 2;

            mm1.x = (float)bordernodes[node].x;
            mm1.y = (float)bordernodes[node].y;

            mm3.x = (float)bordernodes[node + 1].x;
            mm3.y = (float)bordernodes[node + 1].y;
        }

        int xswerve = random(1, maxswerve);
        int yswerve = random(1, maxswerve);

        // Add the swerve to the interpolated point

        mm2.x = (float)(mm1.x + mm3.x) / 2 + xswerve;
        mm2.y = (float)(mm1.y + mm3.y) / 2 + yswerve;

        // Do the actual line

        for (float t = 0.0f; t <= 1.0f; t = t + 0.01f)
        {
            pt = curvepos(mm1, mm1, mm2, mm3, t);

            int x = (int)pt.x;
            int y = (int)pt.y;

            if (x >= 0 && x <= width && y >= 0 && y <= height)
            {
                outline[x][y] = 1;

                if (x < leftx)
                    leftx = x;

                if (x > rightx)
                    rightx = x;

                if (y < lefty)
                    lefty = y;

                if (y > righty)
                    righty = y;
            }
        }

        for (float t = 0.0f; t <= 1.0f; t = t + 0.01f)
        {
            pt = curvepos(mm1, mm2, mm3, mm3, t);

            int x = (int)pt.x;
            int y = (int)pt.y;

            if (x >= 0 && x <= width && y >= 0 && y <= height)
            {
                outline[x][y] = 1;

                if (x < leftx)
                    leftx = x;

                if (x > rightx)
                    rightx = x;

                if (y < lefty)
                    lefty = y;

                if (y > righty)
                    righty = y;
            }
        }

        if (stopping == 1)
            node = totalnodes;
    }

    leftx = leftx - 2;
    rightx = rightx + 2;
    lefty = lefty - 2;
    righty = righty + 2;

    if (leftx < 0)
        leftx = 0;

    if (rightx > width)
        rightx = width;

    if (lefty < 0)
        lefty = 0;

    if (righty > height)
        righty = height;

    // Now, draw a border around the continent.

    for (int i = leftx; i <= rightx; i++)
    {
        if (i >= 0 && i <= width)
        {
            if (lefty >= 0 && lefty <= height)
                outline[i][lefty] = 1;

            if (righty >= 0 && righty <= height)
                outline[i][righty] = 1;
        }
    }

    for (int j = lefty; j <= righty; j++)
    {
        if (j >= 0 && j <= height)
        {
            if (leftx >= 0 && leftx <= width)
                outline[leftx][j] = 1;

            if (rightx >= 0 && rightx <= width)
                outline[rightx][j] = 1;
        }
    }

    // Now fill within that border.

    fill(outline, width, height, leftx + 1, lefty + 1, 1);
    fill(outline, width, height, rightx - 1, lefty + 1, 1);
    fill(outline, width, height, rightx - 1, righty - 1, 1);
    fill(outline, width, height, leftx + 1, righty - 1, 1);

    for (int i = leftx; i <= rightx; i++)
    {
        for (int j = lefty; j <= righty; j++)
        {
            if (outline[i][j] == 0)
                continent[i][j] = 1;
            else
                continent[i][j] = 0;
        }
    }

    // Now remove any odd bits of sea within the continent.

    for (int i = leftx + 1; i < rightx; i++)
    {
        for (int j = lefty + 1; j < righty; j++)
        {
            if (continent[i][j] == 0)
            {
                short crount = 0;

                for (int k = i - 1; k <= i + 1; k++)
                {
                    for (int l = j - 1; l <= j + 1; l++)
                    {
                        if (continent[k][l] == 0)
                            crount++;
                    }
                }

                if (crount < 5)
                    continent[i][j] = 1;
            }
        }
    }

    // Now rotate! There's a tendency for some of these continents to come out a bit rectangular, so rotating them helps to conceal that.

    float distort;

    if (random(1, 2) == 1)
        distort = (float)random(150, 400);
    else
        distort = (float)random(10, 150);

    float distortvary;

    if (random(1, 8) == 1)
        distortvary = distort;
    else
    {
        if (random(1, 4) == 1)
            distortvary = distort * 4.0f;
        else
            distortvary = distort * 2.0f;
    }

    float distortstep = (float)world.maxelevation() / distort;

    int grain = 8; // Level of detail on this fractal map.
    float valuemod = 0.2f;

    vector<vector<int>> fractal(width + 1, vector<int>(height + 1, 0));

    create2dfractalold(fractal, width, height, grain, valuemod, valuemod, 1, world.maxelevation(), 0, 0);

    for (int i = leftx; i <= rightx; i++)
    {
        for (int j = lefty; j <= righty; j++)
        {
            outline[i][j] = continent[i][j];
            continent[i][j] = 0;
        }
    }

    int originx = (leftx + rightx) / 2;
    int originy = (lefty + righty) / 2;

    int angle = random(25, 65);
    leftx = width / 2;
    rightx = width / 2;
    lefty = height / 2;
    righty = height / 2;

    bool failed = 0;

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            float thisdistort = (float)fractal[i][j];

            thisdistort = (thisdistort / distortstep) - distort / 2.0f;

            if (thisdistort > 0.0f)
            {
                thisdistort = thisdistort / distortvary;

                thisdistort = thisdistort * thisdistort;

                thisdistort = thisdistort * distortvary;
            }
            else
                thisdistort = 0.0f;

            float ii = (float)(i - originx) + thisdistort;
            float jj = (float)(j - originy) + thisdistort;

            int newi = (int)(ii * cos(angle) - jj * sin(angle));
            int newj = (int)(jj * cos(angle) + ii * sin(angle));

            newi = newi + originx;
            newj = newj + originy;

            if (newi >= 0 && newi <= width && newj >= 0 && newj <= height)
            {
                continent[i][j] = outline[newi][newj];

                if (continent[i][j] == 1)
                {
                    if (i < leftx)
                        leftx = i;

                    if (i > rightx)
                        rightx = i;

                    if (j < lefty)
                        lefty = j;

                    if (j > righty)
                        righty = j;
                }
            }
        }
    }

    if (failed)
    {
        leftx = width / 2;
        rightx = width / 2;
        lefty = height / 2;
        righty = height / 2;

        for (int i = 0; i <= width; i++)
        {
            for (int j = 0; j <= height; j++)
            {
                continent[i][j] = outline[i][j];

                if (continent[i][j] == 1)
                {
                    if (i < leftx)
                        leftx = i;

                    if (i > rightx)
                        rightx = i;

                    if (j < lefty)
                        lefty = j;

                    if (j > righty)
                        righty = j;
                }
            }
        }
    }
}

// This function creates a simple map of circles to be used for continents.

void makecontinentcircles(vector<vector<bool>>& circlemap, int width, int height, int circleno, int circlesize, int& circleleftx, int& circlerightx, int& circlelefty, int& circlerighty)
{
    vector<vector<int>> circleinfo(circleno, vector<int>(circleno, 2)); // This will hold information about each circle.

    circleinfo[0][0] = width / 2; // centre x coordinate
    circleinfo[0][1] = height / 2; // centre y coordinate
    circleinfo[0][2] = circlesize; // radius

    for (int circle = 1; circle < circleno; circle++)
    {
        bool keepgoing = 1;

        do
        {
            int prevcircle;

            prevcircle = random(0, circle - 1); // Choose one of the previous ones to put this one near.

            short rand = random(1, 4);

            switch (rand)
            {
            case 1:
                circleinfo[circle][0] = random(circleinfo[prevcircle][0] - circleinfo[prevcircle][3], circleinfo[prevcircle][0] + circleinfo[prevcircle][3]);
                circleinfo[circle][1] = random(circleinfo[prevcircle][1] - circleinfo[prevcircle][3], circleinfo[prevcircle][1] - circleinfo[prevcircle][3] / 2);
                break;

            case 2:
                circleinfo[circle][0] = random(circleinfo[prevcircle][0] - circleinfo[prevcircle][3], circleinfo[prevcircle][0] + circleinfo[prevcircle][3]);
                circleinfo[circle][1] = random(circleinfo[prevcircle][1] + circleinfo[prevcircle][3] / 2, circleinfo[prevcircle][1] + circleinfo[prevcircle][3]);
                break;

            case 3:
                circleinfo[circle][0] = random(circleinfo[prevcircle][0] - circleinfo[prevcircle][3], circleinfo[prevcircle][0] - circleinfo[prevcircle][3] / 2);
                circleinfo[circle][1] = random(circleinfo[prevcircle][1] - circleinfo[prevcircle][3], circleinfo[prevcircle][1] + circleinfo[prevcircle][3]);
                break;

            case 4:
                circleinfo[circle][0] = random(circleinfo[prevcircle][0] + circleinfo[prevcircle][3] / 2, circleinfo[prevcircle][0] + circleinfo[prevcircle][3]);
                circleinfo[circle][1] = random(circleinfo[prevcircle][1] - circleinfo[prevcircle][3], circleinfo[prevcircle][1] + circleinfo[prevcircle][3]);
                break;
            }

            if (circleinfo[circle][0] > circlesize * 3 && circleinfo[circle][0]<width - circlesize * 3 && circleinfo[circle][1]>circlesize * 3 && circleinfo[circle][1] < height - circlesize * 3)
                keepgoing = 0;

        } while (keepgoing == 1);

        circleinfo[circle][3] = random((circlesize / 3) * 2, circlesize);
    }

    for (int circle = 0; circle < circleno; circle++)
    {
        int centrex = 0;
        int centrey = 0;
        int radius = circleinfo[circle][3];

        for (int i = -radius; i <= radius; i++)
        {
            int ii = circleinfo[circle][0] + i;

            for (int j = -radius; j <= radius; j++)
            {
                int jj = circleinfo[circle][1] + j;

                if (ii >= 0 && ii <= width && jj >= 0 && jj <= height)
                {
                    if (i * i + j * j < radius * radius + radius)
                    {
                        circlemap[ii][jj] = 1;

                        if (ii < circleleftx)
                            circleleftx = ii;

                        if (ii > circlerightx)
                            circlerightx = ii;

                        if (jj < circlelefty)
                            circlelefty = jj;

                        if (jj > circlerighty)
                            circlerighty = jj;
                    }
                }
            }
        }
    }

    // Possibly cut away half of it, diagonally.

    int cutchance = 2;

    if (random(1, cutchance) == 1)
    {
        int circlewidth = circlerightx - circleleftx;
        int circleheight = circlerighty - circlelefty;

        for (int i = 0; i < circlewidth; i++)
        {
            int ii = circleleftx + i;

            for (int j = 0; j <= i; j++)
            {
                int jj = circlelefty + j;

                if (jj >= 0 && jj <= height && ii >= 0 && ii <= width)
                    circlemap[ii][jj] = 0;
            }
        }
    }

    // Now just stick a few on around the edges, and also remove a few.

    vector<vector<bool>> circlecopy(width + 1, vector<bool>(height + 1, 0));

    for (int n = 0; n < 2; n++)
    {
        for (int i = 0; i <= width; i++)
        {
            for (int j = 0; j <= height; j++)
                circlecopy[i][j] = circlemap[i][j];
        }

        int extracirclechance = 100;

        for (int i = 0; i <= width; i++)
        {
            for (int j = 0; j <= height; j++)
            {
                if (circlecopy[i][j] == 1 && random(1, extracirclechance) == 1)
                {
                    bool outline = 0;

                    for (int ii = i - 1; ii <= i + 1; ii++)
                    {
                        for (int jj = j - 1; jj <= j + 1; jj++)
                        {
                            if (ii >= 0 && ii <= width && jj >= 0 && jj <= height)
                            {
                                if (circlecopy[ii][jj] == 0)
                                {
                                    outline = 1;
                                    ii = i + 1;
                                    jj = j + 1;
                                }
                            }

                        }
                    }

                    if (outline == 1)
                    {
                        int centrex = i;
                        int centrey = j;

                        int radius = random(circlesize / 4, circlesize / 2);

                        for (int x = -radius; x <= radius; x++)
                        {
                            int xx = centrex + x;

                            for (int y = -radius; y <= radius; y++)
                            {
                                int yy = centrey + y;

                                if (xx > 0 && xx < width && yy>0 && yy < height)
                                {
                                    if (x * x + y * y < radius * radius + radius)
                                    {
                                        circlemap[xx][yy] = 1;

                                        if (xx < circleleftx)
                                            circleleftx = xx;

                                        if (xx > circlerightx)
                                            circlerightx = xx;

                                        if (yy < circlelefty)
                                            circlelefty = yy;

                                        if (yy > circlerighty)
                                            circlerighty = yy;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        for (int i = 0; i <= width; i++)
        {
            for (int j = 0; j <= height; j++)
                circlecopy[i][j] = circlemap[i][j];
        }

        for (int i = 0; i <= width; i++)
        {
            for (int j = 0; j <= height; j++)
            {
                if (circlecopy[i][j] == 1 && random(1, extracirclechance) == 1)
                {
                    bool outline = 0;

                    for (int ii = i - 1; ii <= i + 1; ii++)
                    {
                        for (int jj = j - 1; jj <= j + 1; jj++)
                        {
                            if (ii >= 0 && ii <= width && jj >= 0 && jj <= height)
                            {
                                if (circlecopy[ii][jj] == 0)
                                {
                                    outline = 1;
                                    ii = i + 1;
                                    jj = j + 1;
                                }
                            }

                        }
                    }

                    if (outline == 1)
                    {
                        int centrex = i;
                        int centrey = j;

                        int radius = random(circlesize / 4, circlesize / 2);

                        for (int x = -radius; x <= radius; x++)
                        {
                            int xx = centrex + x;

                            for (int y = -radius; y <= radius; y++)
                            {
                                int yy = centrey + y;

                                if (xx > 0 && xx < width && yy>0 && yy < height)
                                {
                                    if (x * x + y * y < radius * radius + radius)
                                    {
                                        circlemap[xx][yy] = 0;

                                        if (xx < circleleftx)
                                            circleleftx = xx;

                                        if (xx > circlerightx)
                                            circlerightx = xx;

                                        if (yy < circlelefty)
                                            circlelefty = yy;

                                        if (yy > circlerighty)
                                            circlerighty = yy;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// This function removes lines of sea/land that appear at face edges.

void removeedgelines(planet& world, int baseheight, int conheight)
{
    int edge = world.edge();
    
    for (int face = 0; face < 6; face++)
    {
        for (int n = 0; n < edge; n++)
        {
            if (world.nom(face, 0, n) == baseheight && world.nom(face, 1, n) == conheight)
            {
                globepoint newpoint = getglobepoint(edge, face, 0, n, -1, 0);

                if (world.nom(newpoint.face, newpoint.x, newpoint.y) == conheight)
                    world.setnom(face, 0, n, conheight);
            }

            if (world.nom(face, n, 0) == baseheight && world.nom(face, n, 1) == conheight)
            {
                globepoint newpoint = getglobepoint(edge, face, n, 0, 0, -1);

                if (world.nom(newpoint.face, newpoint.x, newpoint.y) == conheight)
                    world.setnom(face, n, 0, conheight);
            }

            if (world.nom(face, 0, n) == conheight && world.nom(face, 1, n) == baseheight)
            {
                globepoint newpoint = getglobepoint(edge, face, 0, n, -1, 0);

                if (world.nom(newpoint.face, newpoint.x, newpoint.y) == baseheight)
                    world.setnom(face, 0, n, baseheight);
            }

            if (world.nom(face, n, 0) == conheight && world.nom(face, n, 1) == baseheight)
            {
                globepoint newpoint = getglobepoint(edge, face, n, 0, 0, -1);

                if (world.nom(newpoint.face, newpoint.x, newpoint.y) == baseheight)
                    world.setnom(face, n, 0, baseheight);
            }
        }
    }
}

// This function removes straight lines at problematic edges between faces.

void removeweirdstraights(planet& world, int baseheight, int conheight, boolshapetemplate landshape[])
{
    int edge = world.edge();
    
    for (int n = 0; n < edge; n++)
    {
        if (world.nom(2, 0, n) == conheight) // face 2 to face 1
        {
            int shapenumber = random(0, 6);
            drawshape(world, shapenumber, 2, 0, n, 0, baseheight, conheight, landshape);
        }

        if (world.nom(1, n, edge - 1) == conheight) // face 1 to face 5
        {
            int shapenumber = random(0, 6);
            drawshape(world, shapenumber, 1, n, edge - 1, 0, baseheight, conheight, landshape);
        }

        if (world.nom(2, n, edge - 1) == conheight) // face 2 to face 5
        {
            int shapenumber = random(0, 6);
            drawshape(world, shapenumber, 2, n, edge - 1, 0, baseheight, conheight, landshape);
        }

        if (world.nom(2, edge - 1, n) == conheight) // face 2 to face 3
        {
            int shapenumber = random(0, 6);
            drawshape(world, shapenumber, 2, edge - 1, n, 0, baseheight, conheight, landshape);
        }

        if (world.nom(5, n, edge - 1) == conheight) // face 5 to face 2
        {
            int shapenumber = random(0, 6);
            drawshape(world, shapenumber, 5, n, edge - 1, 0, baseheight, conheight, landshape);
        }

        if (world.nom(0, n, edge - 1) == conheight) // face 0 to face 5
        {
            int shapenumber = random(0, 6);
            drawshape(world, shapenumber, 0, n, edge - 1, 0, baseheight, conheight, landshape);
        }

        if (world.nom(0, 0, n) == conheight) // face 0 to face 3
        {
            int shapenumber = random(0, 6);
            drawshape(world, shapenumber, 0, 0, n, 0, baseheight, conheight, landshape);
        }

        if (world.nom(4, 0, n) == conheight) // face 4 to face 3
        {
            int shapenumber = random(0, 6);
            drawshape(world, shapenumber, 4, 0, n, 0, baseheight, conheight, landshape);
        }
    }
}

// This function removes straight edges on the coastlines.

void removebigstraights(planet& world, int baseheight, int conheight, boolshapetemplate landshape[])
{
    int edge = world.edge();
    int sealevel = world.sealevel();

    int maxlength = 4; // Any straights longer than this will be disrupted.

    int minsize = 2;
    int maxsize = 4; // Range of sizes for the chunks to be cut out of the coasts.

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++) // First do vertical/horizontal lines.
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.map(face, i, j) <= sealevel)
                {
                    globepoint upone = getglobepoint(edge, face, i, j, 0, -1);
                    globepoint downone = getglobepoint(edge, face, i, j, 0, 1);
                    globepoint leftone = getglobepoint(edge, face, i, j, -1, 0);
                    globepoint rightone = getglobepoint(edge, face, i, j, 1, 0);

                    if (world.map(upone.face, upone.x, upone.y) > sealevel) // Sea to the south, land to the north
                    {
                        bool keepgoing = 1;
                        int rightshift = 1;

                        do
                        {
                            globepoint shifted = getglobepoint(edge, face, i, j, rightshift, 0);

                            if (world.map(shifted.face, shifted.x, shifted.y) > sealevel)
                                keepgoing = 0;
                            else
                            {
                                globepoint doubleshifted = getglobepoint(edge, face, i, j, rightshift, -1);

                                if (world.map(shifted.face, shifted.x, shifted.y) <= sealevel)
                                    keepgoing = 0;
                            }

                            rightshift++;

                        } while (keepgoing == 1);

                        if (rightshift > maxlength) // If this line is too long
                        {
                            int size = random(minsize, maxsize);

                            globepoint midpoint = getglobepoint(edge, face, i, j, rightshift / 2, 0);

                            int shapenumber = random(0, 6);
                            bool land = random(0, 1);

                            drawshape(world, shapenumber, midpoint.face, midpoint.x, midpoint.y, land, baseheight, conheight, landshape);
                        }
                    }

                    if (world.map(downone.face, downone.x, downone.y) > sealevel) // Sea to the north, land to the south
                    {
                        bool keepgoing = 1;
                        int rightshift = 1;

                        do
                        {
                            globepoint shifted = getglobepoint(edge, face, i, j, rightshift, 0);

                            if (world.map(shifted.face, shifted.x, shifted.y) > sealevel)
                                keepgoing = 0;
                            else
                            {
                                globepoint doubleshifted = getglobepoint(edge, face, i, j, rightshift, 1);

                                if (world.map(shifted.face, shifted.x, shifted.y) <= sealevel)
                                    keepgoing = 0;
                            }

                            rightshift++;

                        } while (keepgoing == 1);

                        if (rightshift > maxlength) // If this line is too long
                        {
                            int size = random(minsize, maxsize);

                            globepoint midpoint = getglobepoint(edge, face, i, j, rightshift / 2, 0);

                            int shapenumber = random(0, 6);
                            bool land = random(0, 1);

                            drawshape(world, shapenumber, midpoint.face, midpoint.x, midpoint.y, land, baseheight, conheight, landshape);
                        }
                    }

                    if (world.map(rightone.face, rightone.x, rightone.y) > sealevel) // Sea to the west, land to the east
                    {
                        bool keepgoing = 1;
                        int downshift = 1;

                        do
                        {
                            globepoint shifted = getglobepoint(edge, face, i, j, 0, downshift);

                            if (world.map(shifted.face, shifted.x, shifted.y) > sealevel)
                                keepgoing = 0;
                            else
                            {
                                globepoint doubleshifted = getglobepoint(edge, face, i, j, 1, downshift);

                                if (world.map(shifted.face, shifted.x, shifted.y) <= sealevel)
                                    keepgoing = 0;
                            }

                            downshift++;

                        } while (keepgoing == 1);

                        if (downshift > maxlength) // If this line is too long
                        {
                            int size = random(minsize, maxsize);

                            globepoint midpoint = getglobepoint(edge, face, i, j, 0, downshift / 2);

                            int shapenumber = random(0, 6);
                            bool land = random(0, 1);

                            drawshape(world, shapenumber, midpoint.face, midpoint.x, midpoint.y, land, baseheight, conheight, landshape);
                        }
                    }

                    if (world.map(leftone.face, rightone.x, rightone.y) > sealevel) // Sea to the east, land to the west
                    {
                        bool keepgoing = 1;
                        int downshift = 1;

                        do
                        {
                            globepoint shifted = getglobepoint(edge, face, i, j, 0, downshift);

                            if (world.map(shifted.face, shifted.x, shifted.y) > sealevel)
                                keepgoing = 0;
                            else
                            {
                                globepoint doubleshifted = getglobepoint(edge, face, i, j, -1, downshift);

                                if (world.map(shifted.face, shifted.x, shifted.y) <= sealevel)
                                    keepgoing = 0;
                            }

                            downshift++;

                        } while (keepgoing == 1);

                        if (downshift > maxlength) // If this line is too long
                        {
                            int size = random(minsize, maxsize);

                            globepoint midpoint = getglobepoint(edge, face, i, j, 0, downshift / 2);

                            int shapenumber = random(0, 6);
                            bool land = random(0, 1);

                            drawshape(world, shapenumber, midpoint.face, midpoint.x, midpoint.y, land, baseheight, conheight, landshape);
                        }
                    }
                }
            }
        }
    }

    /*
    for (int i = 0; i <= width; i++) // Now do diagonals.
    {
        for (int j = 1; j < height; j++)
        {
            if (world.map(i, j) > sealevel)
            {
                if (northwestlandonly(world, i, j) == 1)
                {
                    int ii = i + 1;

                    if (ii > width)
                        ii = 0;

                    int jj = j - 1;

                    if (northwestlandonly(world, ii, jj) == 1)
                    {
                        int iii = i - 1;

                        if (iii < 0)
                            iii = width;

                        int jjj = j + 1;

                        if (northwestlandonly(world, iii, jjj) == 1)
                        {
                            int choose = random(1, 3);

                            if (choose == 1)
                                world.setnom(i, j, world.nom(ii, jjj));

                            if (choose == 2)
                                world.setnom(ii, j, world.nom(i, j));

                            if (choose == 3)
                                world.setnom(i, jjj, world.nom(i, j));
                        }
                    }
                }

                if (northeastlandonly(world, i, j) == 1)
                {
                    int ii = i + 1;

                    if (ii > width)
                        ii = 0;

                    int jj = j + 1;

                    if (northeastlandonly(world, ii, jj) == 1)
                    {
                        int iii = i - 1;

                        if (iii < 0)
                            iii = width;

                        int jjj = j - 1;

                        if (northeastlandonly(world, iii, jjj) == 1)
                        {
                            int choose = random(1, 3);

                            if (choose == 1)
                                world.setnom(i, j, world.nom(iii, jj));

                            if (choose == 2)
                                world.setnom(iii, j, world.nom(i, j));

                            if (choose == 3)
                                world.setnom(i, jj, world.nom(i, j));
                        }
                    }
                }

                if (southwestlandonly(world, i, j) == 1)
                {
                    int ii = i + 1;

                    if (ii > width)
                        ii = 0;

                    int jj = j + 1;

                    if (southwestlandonly(world, ii, jj) == 1)
                    {
                        int iii = i - 1;

                        if (iii < 0)
                            iii = width;

                        int jjj = j - 1;

                        if (southwestlandonly(world, iii, jjj) == 1)
                        {
                            int choose = random(1, 3);

                            if (choose == 1)
                                world.setnom(i, j, world.nom(ii, jjj));

                            if (choose == 2)
                                world.setnom(ii, j, world.nom(i, j));

                            if (choose == 3)
                                world.setnom(i, jjj, world.nom(i, j));
                        }
                    }



                }

                if (southeastlandonly(world, i, j) == 1)
                {
                    int ii = i + 1;

                    if (ii > width)
                        ii = 0;

                    int jj = j - 1;

                    if (southeastlandonly(world, ii, jj) == 1)
                    {
                        int iii = i - 1;

                        if (iii < 0)
                            iii = width;

                        int jjj = j + 1;

                        if (southeastlandonly(world, iii, jjj) == 1)
                        {
                            int choose = random(1, 3);

                            if (choose == 1)
                                world.setnom(i, j, world.nom(iii, jj));

                            if (choose == 2)
                                world.setnom(iii, j, world.nom(i, j));

                            if (choose == 3)
                                world.setnom(i, jj, world.nom(i, j));
                        }
                    }
                }
            }
        }
    }
    */
}

// This function identifies edges of continents to put down mountain ranges.

void markedgemountains(planet& world, vector<vector<vector<int>>>& mountainsraw)
{
    int edge = world.edge();
    int sealevel = world.sealevel();
    int maxelev = world.maxelevation();
    int abovesea = maxelev - sealevel;
    int minadjust = edge / 6; // 10;
    int maxadjust = edge / 3; // 4;

    int minmountainheight = abovesea / 2;
    int maxmountainheight = abovesea - abovesea / 10;

    float minheightmult = 0.6f;

    for (int face = 0; face < 6; face++)
    {
        bool doneupdown = 0;
        bool doneleftright = 0;

        // First, check the top edge.

        bool found = 0;

        for (int i = 0; i < edge; i++)
        {
            if (world.map(face, i, 0) > sealevel)
            {
                found = 1;
                i = edge;
            }
        }

        if (found == 0) // If it's all sea along this edge
        {
            // First, find the nearest bit of land, if any.

            int centrex = -1;
            int centrey = -1;

            for (int j = 0; j < edge; j++)
            {
                for (int i = 0; i < edge; i++)
                {
                    if (world.nom(face, i, j) > sealevel)
                    {
                        centrex = i;
                        centrey = j;
                        j = edge;
                        i = edge;
                    }
                }
            }
            
            if (centrex != -1) // If we did find a point
            {
                int adjust = random(minadjust, maxadjust);
                
                centrey = centrey + adjust; // Move the centre of our search oval a bit inland.

                if (centrey >= edge)
                    centrey = edge - 1;

                if (world.nom(face, centrex, centrey) > sealevel)
                {
                    // We need to see how far the land extends to the left and right.

                    int leftx = centrex;

                    for (int i = centrex; i >= 0; i--)
                    {
                        if (world.nom(face, i, centrey) > sealevel)
                            leftx = i;
                        else
                            i = 0;
                    }

                    int rightx = centrex;

                    for (int i = centrex; i < edge; i++)
                    {
                        if (world.nom(face, i, centrey) > sealevel)
                            rightx = i;
                        else
                            i = edge;
                    }
                    
                    if (leftx > 0 && rightx < edge - 1)
                    {
                        centrex = (leftx + rightx) / 2; // Move the centre to the middle of this chunk of coastline.

                        float radius1 = (float)((rightx - leftx) / 2) * 1.5f;
                        float radius2 = (float)adjust * 1.5f; // The search area will be an oval.

                        float mountainheight = (float)(random(minmountainheight, maxmountainheight));

                        int ifrom = centrex - (int)radius1;

                        if (ifrom < 1)
                            ifrom = 1;

                        int ito = centrex + (int)radius1;

                        if (ito >= edge - 1)
                            ito = edge - 2;

                        int jfrom = centrey - (int)radius2;

                        if (jfrom < 1)
                            jfrom = 1;

                        int jto = centrey + (int)radius2;

                        if (jto >= edge - 1)
                            jto = edge - 2;

                        for (int i = ifrom; i <= ito; i++)
                        {
                            float ii = (float)(i - centrex);

                            for (int j = jfrom; j <= jto; j++)
                            {
                                float jj = (float)(j - centrey);

                                float calc = (float)(ii * ii) / (radius1 * radius1) + (float)(jj * jj) / (radius2 * radius2);

                                if (calc <= 1.0f) // Inside the oval!
                                {
                                    if (world.nom(face, i, j) > sealevel)
                                    {
                                        if (coast(world, face, i, j) && world.nom(face, i, j + 1) > sealevel)
                                        {
                                            float centredist = (float)(i - centrex);

                                            if (i < centrex)
                                                centredist = (float)(centrex - i);

                                            float heightmult = 1.0f - (centredist / radius1);

                                            if (heightmult < minheightmult)
                                                heightmult = minheightmult;

                                            float thismountainheight = mountainheight * heightmult;

                                            mountainsraw[face][i][j] = (int)thismountainheight;

                                            doneupdown = 1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // Now, the bottom edge.

        if (doneupdown == 0)
        {
            for (int i = 0; i < edge; i++)
            {
                if (world.map(face, i, edge - 1) > sealevel)
                {
                    found = 1;
                    i = edge;
                }
            }

            if (found == 0) // If it's all sea along this edge
            {
                // First, find the nearest bit of land, if any.

                int centrex = -1;
                int centrey = -1;

                for (int j = edge - 1; j > 0; j--)
                {
                    for (int i = 0; i < edge; i++)
                    {
                        if (world.nom(face, i, j) > sealevel)
                        {
                            centrex = i;
                            centrey = j;
                            j = 0;
                            i = edge;
                        }
                    }
                }

                if (centrex != -1) // If we did find a point
                {
                    int adjust = random(minadjust, maxadjust);

                    centrey = centrey - adjust; // Move the centre of our search oval a bit inland.

                    if (centrey < 0)
                        centrey = 0;

                    if (world.nom(face, centrex, centrey) > sealevel)
                    {
                        // We need to see how far the land extends to the left and right.

                        int leftx = centrex;

                        for (int i = centrex; i >= 0; i--)
                        {
                            if (world.nom(face, i, centrey) > sealevel)
                                leftx = i;
                            else
                                i = 0;
                        }

                        int rightx = centrex;

                        for (int i = centrex; i < edge; i++)
                        {
                            if (world.nom(face, i, centrey) > sealevel)
                                rightx = i;
                            else
                                i = edge;
                        }

                        if (leftx > 0 && rightx < edge - 1)
                        {
                            centrex = (leftx + rightx) / 2; // Move the centre to the middle of this chunk of coastline.

                            float radius1 = (float)((rightx - leftx) / 2) * 1.5f;
                            float radius2 = (float)adjust * 1.5f; // The search area will be an oval.

                            float mountainheight = (float)(random(minmountainheight, maxmountainheight));

                            int ifrom = centrex - (int)radius1;

                            if (ifrom < 1)
                                ifrom = 1;

                            int ito = centrex + (int)radius1;

                            if (ito >= edge - 1)
                                ito = edge - 2;

                            int jfrom = centrey - (int)radius2;

                            if (jfrom < 1)
                                jfrom = 1;

                            int jto = centrey + (int)radius2;

                            if (jto >= edge - 1)
                                jto = edge - 2;

                            for (int i = ifrom; i <= ito; i++)
                            {
                                float ii = (float)(i - centrex);

                                for (int j = jfrom; j <= jto; j++)
                                {
                                    float jj = (float)(j - centrey);

                                    float calc = (float)(ii * ii) / (radius1 * radius1) + (float)(jj * jj) / (radius2 * radius2);

                                    if (calc <= 1.0f) // Inside the oval!
                                    {
                                        if (world.nom(face, i, j) > sealevel)
                                        {
                                            if (coast(world, face, i, j) && world.nom(face, i, j - 1) > sealevel)
                                            {
                                                float centredist = (float)(i - centrex);

                                                if (i < centrex)
                                                    centredist = (float)(centrex - i);

                                                float heightmult = 1.0f - (centredist / radius1);

                                                if (heightmult < minheightmult)
                                                    heightmult = minheightmult;

                                                float thismountainheight = mountainheight * heightmult;

                                                mountainsraw[face][i][j] = (int)thismountainheight;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // Now, the left edge.

        found = 0;

        for (int j = 0; j < edge; j++)
        {
            if (world.map(face, 0, j) > sealevel)
            {
                found = 1;
                j = edge;
            }
        }

        if (found == 0) // If it's all sea along this edge
        {
            // First, find the nearest bit of land, if any.

            int centrex = -1;
            int centrey = -1;

            for (int i = 0; i < edge; i++)
            {
                for (int j = 0; j < edge; j++)
                {
                    if (world.nom(face, i, j) > sealevel)
                    {
                        centrex = i;
                        centrey = j;
                        j = edge;
                        i = edge;
                    }
                }
            }

            if (centrex != -1) // If we did find a point
            {
                int adjust = random(minadjust, maxadjust);

                centrex = centrex + adjust; // Move the centre of our search oval a bit inland.

                if (centrex >= edge)
                    centrex = edge - 1;

                if (world.nom(face, centrex, centrey) > sealevel)
                {
                    // We need to see how far the land extends up and down.

                    int upy = centrey;

                    for (int j = centrey; j >= 0; j--)
                    {
                        if (world.nom(face, centrex, j) > sealevel)
                            upy = j;
                        else
                            j = 0;
                    }

                    int downy = centrey;

                    for (int j = centrey; j < edge; j++)
                    {
                        if (world.nom(face, centrex,j) > sealevel)
                            downy = j;
                        else
                            j = edge;
                    }

                    if (upy > 0 && downy < edge - 1)
                    {
                        centrey = (upy + downy) / 2; // Move the centre to the middle of this chunk of coastline.
                           
                        float radius1 = (float)adjust * 1.5f;
                        float radius2 = (float)((downy - upy) / 2) * 1.5f; // The search area will be an oval.

                        float mountainheight = (float)(random(minmountainheight, maxmountainheight));

                        int ifrom = centrex - (int)radius1;

                        if (ifrom < 1)
                            ifrom = 1;

                        int ito = centrex + (int)radius1;

                        if (ito >= edge - 1)
                            ito = edge - 2;

                        int jfrom = centrey - (int)radius2;

                        if (jfrom < 1)
                            jfrom = 1;

                        int jto = centrey + (int)radius2;

                        if (jto >= edge - 1)
                            jto = edge - 2;

                        for (int i = ifrom; i <= ito; i++)
                        {
                            float ii = (float)(i - centrex);

                            for (int j = jfrom; j <= jto; j++)
                            {
                                float jj = (float)(j - centrey);

                                float calc = (float)(ii * ii) / (radius1 * radius1) + (float)(jj * jj) / (radius2 * radius2);

                                if (calc <= 1.0f) // Inside the oval!
                                {
                                    if (world.nom(face, i, j) > sealevel)
                                    {
                                        if (coast(world, face, i, j) && world.nom(face, i + 1, j) > sealevel)
                                        {
                                            float centredist = (float)(j - centrey);

                                            if (j < centrey)
                                                centredist = (float)(centrey - j);

                                            float heightmult = 1.0f - (centredist / radius1);

                                            if (heightmult < minheightmult)
                                                heightmult = minheightmult;

                                            float thismountainheight = mountainheight * heightmult;

                                            mountainsraw[face][i][j] = (int)thismountainheight;

                                            doneleftright = 1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // Now, the right edge.

        if (doneleftright == 0)
        {
            found = 0;

            for (int j = 0; j < edge; j++)
            {
                if (world.map(face, edge - 1, j) > sealevel)
                {
                    found = 1;
                    j = edge;
                }
            }

            if (found == 0) // If it's all sea along this edge
            {
                // First, find the nearest bit of land, if any.

                int centrex = -1;
                int centrey = -1;

                for (int i = edge - 1; i >= 0; i--)
                {
                    for (int j = 0; j < edge; j++)
                    {
                        if (world.nom(face, i, j) > sealevel)
                        {
                            centrex = i;
                            centrey = j;
                            j = edge;
                            i = 0;
                        }
                    }
                }

                if (centrex != -1) // If we did find a point
                {
                    int adjust = random(minadjust, maxadjust);

                    centrex = centrex - adjust; // Move the centre of our search oval a bit inland.

                    if (centrex < 0)
                        centrex = 0;

                    if (world.nom(face, centrex, centrey) > sealevel)
                    {
                        // We need to see how far the land extends up and down.

                        int upy = centrey;

                        for (int j = centrey; j >= 0; j--)
                        {
                            if (world.nom(face, centrex, j) > sealevel)
                                upy = j;
                            else
                                j = 0;
                        }

                        int downy = centrey;

                        for (int j = centrey; j < edge; j++)
                        {
                            if (world.nom(face, centrex, j) > sealevel)
                                downy = j;
                            else
                                j = edge;
                        }

                        if (upy > 0 && downy < edge - 1)
                        {
                            centrey = (upy + downy) / 2; // Move the centre to the middle of this chunk of coastline.

                            float radius1 = (float)adjust * 1.5f;
                            float radius2 = (float)((downy - upy) / 2) * 1.5f; // The search area will be an oval.

                            float mountainheight = (float)(random(minmountainheight, maxmountainheight));

                            int ifrom = centrex - (int)radius1;

                            if (ifrom < 1)
                                ifrom = 1;

                            int ito = centrex + (int)radius1;

                            if (ito >= edge - 1)
                                ito = edge - 2;

                            int jfrom = centrey - (int)radius2;

                            if (jfrom < 1)
                                jfrom = 1;

                            int jto = centrey + (int)radius2;

                            if (jto >= edge - 1)
                                jto = edge - 2;

                            for (int i = ifrom; i <= ito; i++)
                            {
                                float ii = (float)(i - centrex);

                                for (int j = jfrom; j <= jto; j++)
                                {
                                    float jj = (float)(j - centrey);

                                    float calc = (float)(ii * ii) / (radius1 * radius1) + (float)(jj * jj) / (radius2 * radius2);

                                    if (calc <= 1.0f) // Inside the oval!
                                    {
                                        if (world.nom(face, i, j) > sealevel)
                                        {
                                            if (coast(world, face, i, j) && world.nom(face, i - 1, j) > sealevel)
                                            {
                                                float centredist = (float)(j - centrey);

                                                if (j < centrey)
                                                    centredist = (float)(centrey - j);

                                                float heightmult = 1.0f - (centredist / radius1);

                                                if (heightmult < minheightmult)
                                                    heightmult = minheightmult;

                                                float thismountainheight = mountainheight * heightmult;

                                                mountainsraw[face][i][j] = (int)thismountainheight;

                                                doneleftright = 1;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// This creates mountains from a raw map (showing just the central peak lines). (Note: it does just one face of the cubesphere, so these mountains can't go onto another face.)

void createmountainsfromraw(planet& world, int face, vector<vector<vector<int>>>& rawmountains)
{
    int edge = world.edge();
    
    int width = edge - 1;
    int height = edge - 1;
    int maxelev = world.maxelevation();

    vector<vector<int>> extraraw(edge, vector<int>(edge, 0)); // This will hold extra raw ridges that we add near the start.
    vector<vector<int>> mountaindist(edge, vector<int>(edge, 0)); // This holds the proximity to the central peaks (the higher the value, the closer it is).
    vector<vector<int>> mountainbaseheight(edge, vector<int>(edge, 0)); // This holds the height that this peak will be (to start with, it's just recorded as the same as the central peak that it's measured from).
    vector<vector<bool>> timesten(edge, vector<bool>(edge, 0)); // This records whether the ridge distance has been multiplied by ten (this is done so that the ridges on either side of the main ridge don't join up with each other).
    vector<vector<int>> mountainridges(edge, vector<int>(edge, 0));
    vector<vector<int>> mountainheights(edge, vector<int>(edge, 0)); // These two are the same as the world mountain ridges/heights arrays, but we'll do everything on these first and then copy them over.

    int grain = edge / 2;
    float valuemod = 8;
    int v = random(3, 6);
    float valuemod2 = (float)v;

    vector<vector<int>> heightsfractal(edge, vector<int>(edge, 0)); // This will vary the heights of the peaks in the subsidiary ridges.
    create2dfractalold(heightsfractal, width, height, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    grain = edge / 32;
    valuemod = 0.2f;
    v = random(3, 6);
    valuemod2 = (float)v;

    vector<vector<int>> radiusfractal(edge, vector<int>(edge, 0)); // This will vary the effective radius of each mountain point.
    create2dfractalold(radiusfractal, width, height, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    float maxradius = 6; // 10; // The bigger this is, the wider the mountain ranges will be.

    // First, ensure that the raw mountains array has ridges that are only one cell wide.

    for (int i = 1; i < width; i++)
    {
        for (int j = 1; j < height; j++)
        {
            if (rawmountains[face][i][j] != 0)
            {
                int ii = i + 1;

                if (rawmountains[face][i][j - 1] != 0 && rawmountains[face][i][j + 1] != 0)
                    rawmountains[face][ii][j] = 0;

                ii = i - 1;

                if (rawmountains[face][i][j - 1] != 0 && rawmountains[face][i][j + 1] != 0)
                    rawmountains[face][ii][j] = 0;
            }
        }
    }

    for (int i = 1; i < width; i++)
    {
        for (int j = 1; j < height; j++)
        {
            if (rawmountains[face][i][j] != 0)
            {
                int jj = j + 1;

                int ileft = i - 1;

                int iright = i + 1;

                if (rawmountains[face][iright][j] != 0 && rawmountains[face][ileft][j] != 0)
                    rawmountains[face][i][jj] = 0;

                jj = j - 1;

                if (rawmountains[face][iright][j] != 0 && rawmountains[face][ileft][j] != 0)
                    rawmountains[face][i][jj] = 0;
            }
        }
    }

    for (int i = 1; i < width; i++)
    {
        int ii = i + 1;

        for (int j = 1; j < height; j++)
        {
            if (rawmountains[face][i][j] != 0 && rawmountains[face][ii][j + 1] != 0)
            {
                rawmountains[face][ii][j] = 0;
                rawmountains[face][i][j + 1] = 0;
            }

            if (rawmountains[face][ii][j] != 0 && rawmountains[face][i][j + 1] != 0)
            {
                rawmountains[face][i][j] = 0;
                rawmountains[face][ii][j + 1] = 0;
            }
        }
    }

    // Now add some extra ridges! We do this on another array to start with so we don't add them onto each other.

    int extrachance = 10; // The higher this is, the fewer extra ridges there will be.
    int swervechance = 4; // The higher this is, the straighter they will be.
    int minlength = 2;
    int maxlength = 4; // Possible lengths of these extra ridges.

    for (int t = 1; t <= 2; t++) // Do the whole thing twice!
    {
        for (int i = 1; i < width; i++)
        {
            for (int j = 1; j < height; j++)
            {
                if (rawmountains[face][i][j] != 0 && random(1, extrachance) == 1)
                {
                    int lefti = i - 1;
                    int righti = i + 1;

                    int dir = 0;
                    int x = 0;
                    int y = 0;

                    if (rawmountains[face][i][j - 1] != 0)
                    {
                        if (random(1, 2) == 1)
                            x = i - 2;
                        else
                            x = i + 2;

                        y = j;

                        if (random(1, 2) == 1)
                            dir = 1;
                        else
                            dir = 5;
                    }

                    if (rawmountains[face][righti][j - 1] != 0)
                    {
                        if (random(1, 2) == 1)
                        {
                            x = i - 1;
                            y = j - 1;
                        }
                        else
                        {
                            x = i + 1;
                            y = j + 1;
                        }

                        if (random(1, 2) == 1)
                            dir = 2;
                        else
                            dir = 6;
                    }

                    if (rawmountains[face][righti][j] != 0)
                    {
                        if (random(1, 2) == 1)
                            y = j - 2;
                        else
                            y = j + 2;

                        x = i;

                        if (random(1, 2) == 1)
                            dir = 3;
                        else
                            dir = 7;
                    }

                    if (rawmountains[face][righti][j + 1] != 0)
                    {
                        if (random(1, 2) == 1)
                        {
                            x = i + 1;
                            y = j - 1;
                        }
                        else
                        {
                            x = i - 1;
                            y = j + 1;
                        }

                        if (random(1, 2) == 1)
                            dir = 4;
                        else
                            dir = 8;
                    }

                    if (rawmountains[face][i][j + 1] != 0)
                    {
                        if (random(1, 2) == 1)
                            x = i - 2;
                        else
                            x = i + 2;

                        y = j;

                        if (random(1, 2) == 1)
                            dir = 5;
                        else
                            dir = 1;
                    }

                    if (rawmountains[face][lefti][j + 1] != 0)
                    {
                        if (random(1, 2) == 1)
                        {
                            x = i - 1;
                            y = j - 1;
                        }
                        else
                        {
                            x = i + 1;
                            y = j + 1;
                        }

                        if (random(1, 2) == 1)
                            dir = 6;
                        else
                            dir = 2;
                    }

                    if (rawmountains[face][lefti][j] != 0)
                    {
                        if (random(1, 2) == 1)
                            y = j - 2;
                        else
                            y = j + 2;

                        x = i;

                        if (random(1, 2) == 1)
                            dir = 7;
                        else
                            dir = 3;
                    }

                    if (rawmountains[face][lefti][j - 1] != 0)
                    {
                        if (random(1, 2) == 1)
                        {
                            x = i + 1;
                            y = j - 1;
                        }
                        else
                        {
                            x = i - 1;
                            y = j + 1;
                        }

                        if (random(1, 2) == 1)
                            dir = 8;
                        else
                            dir = 4;
                    }

                    if (dir != 0)
                    {
                        int peakheight = rawmountains[face][i][j];
                        int origpeakheight = peakheight;

                        int length = random(minlength, maxlength);

                        for (int n = 1; n <= length; n++)
                        {
                            if (dir == 8 || dir == 1 || dir == 2)
                                y--;

                            if (dir == 4 || dir == 5 || dir == 6)
                                y++;

                            if (dir == 2 || dir == 3 || dir == 4)
                                x++;

                            if (dir == 6 || dir == 7 || dir == 8)
                                x--;

                            if (x >= 0 && x <= height && y >= 0 && y <= height)
                            {
                                extraraw[x][y] = peakheight;

                                if (random(1, swervechance) == 1)
                                {
                                    if (random(1, 2) == 1)
                                        dir--;
                                    else
                                        dir++;

                                    if (dir == 0)
                                        dir = 8;

                                    if (dir == 9)
                                        dir = 1;
                                }

                                peakheight = peakheight + randomsign(random(1, 5));

                                if (peakheight < origpeakheight / 2)
                                    peakheight = origpeakheight / 2;

                                if (peakheight > origpeakheight)
                                    peakheight = origpeakheight;

                            }
                            else
                                n = length;
                        }
                    }
                }
            }
        }

        // Now just add those extra raw ridges to the main raw array.

        for (int i = 0; i <= width; i++)
        {
            for (int j = 0; j <= height; j++)
            {
                if (rawmountains[face][i][j] < extraraw[i][j])
                    rawmountains[face][i][j] = extraraw[i][j];

                extraraw[i][j] = 0;
            }
        }
    }

    // Now get rid of straight sections, where possible.

    for (int n = 1; n <= 2; n++) // Do this whole thing twice, to be sure.
    {
        for (int i = 1; i < width; i++)
        {
            for (int j = 1; j < height; j++)
            {
                if (rawmountains[face][i][j] != 0)
                {
                    int lefti = i - 1;
                    int righti = i + 1;

                    // First, east and west.

                    if (rawmountains[face][lefti][j] != 0 && rawmountains[face][righti][j] != 0)
                    {
                        bool up = 0;

                        if (random(1, 2) == 1)
                            up = 1;

                        if (up == 1)
                        {
                            rawmountains[face][i][j - 1] = rawmountains[face][i][j];
                            rawmountains[face][i][j] = 0;
                        }
                        else
                        {
                            rawmountains[face][i][j + 1] = rawmountains[face][i][j];
                            rawmountains[face][i][j] = 0;
                        }

                        if (random(1, 2) == 1) // Maybe move one of the neighbouring ones too
                        {
                            int ii = lefti;

                            if (random(1, 2) == 1)
                                ii = righti;

                            if (up == 1)
                            {
                                rawmountains[face][ii][j - 1] = rawmountains[face][ii][j];
                                rawmountains[face][ii][j] = 0;
                            }
                            else
                            {
                                rawmountains[face][ii][j + 1] = rawmountains[face][ii][j];
                                rawmountains[face][ii][j] = 0;
                            }
                        }

                    }

                    // Now, north and south.

                    if (rawmountains[face][i][j - 1] != 0 && rawmountains[face][i][j + 1] != 0)
                    {
                        bool left = 0;

                        if (random(1, 2) == 1)
                            left = 1;

                        if (left == 1)
                        {
                            rawmountains[face][lefti][j] = rawmountains[face][i][j];
                            rawmountains[face][i][j] = 0;
                        }
                        else
                        {
                            rawmountains[face][righti][j] = rawmountains[face][i][j];
                            rawmountains[face][i][j] = 0;
                        }

                        if (random(1, 2) == 1) // Maybe move one of the neighbouring ones too
                        {
                            int jj = j - 1;

                            if (random(1, 2) == 1)
                                jj = j + 1;

                            if (left == 1)
                            {
                                rawmountains[face][lefti][jj] = rawmountains[face][i][jj];
                                rawmountains[face][i][jj] = 0;
                            }
                            else
                            {
                                rawmountains[face][righti][jj] = rawmountains[face][i][jj];
                                rawmountains[face][i][jj] = 0;
                            }
                        }
                    }

                    // Now, diagonals from NW to SE.

                    if (rawmountains[face][i - 1][j - 1] != 0 && rawmountains[face][i + 1][j + 1] != 0)
                    {
                        bool left = 0;

                        if (random(1, 2) == 1)
                            left = 1;

                        if (left == 1)
                        {
                            rawmountains[face][lefti][j] = rawmountains[face][i][j];
                            rawmountains[face][i][j + 1] = rawmountains[face][i][j];
                            rawmountains[face][i][j] = 0;
                        }
                        else
                        {
                            rawmountains[face][righti][j] = rawmountains[face][i][j];
                            rawmountains[face][i][j - 1] = rawmountains[face][i][j];
                            rawmountains[face][i][j] = 0;
                        }
                    }
                }
            }
        }
    }

    // Now, work out the distances from the central peaks.

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (rawmountains[face][i][j] != 0)
            {
                float peakheight = (float)rawmountains[face][i][j];

                int mult = 0;

                for (int radius = (int)maxradius; radius >= 1; radius--)
                {
                    mult++;

                    for (int k = -radius; k <= radius; k++)
                    {
                        for (int l = -radius; l <= radius; l++)
                        {
                            int dist = k * k + l * l;

                            if (dist < radius * radius + radius)
                            {
                                int kk = i + k;
                                int ll = j + l;

                                if (kk>=0 && kk<=width && ll >= 0 && ll <= height)
                                {
                                    dist = (int)maxradius - dist;

                                    if (mountaindist[kk][ll] < dist)
                                    {
                                        mountaindist[kk][ll] = dist;
                                        mountainbaseheight[kk][ll] = (int)peakheight;

                                        if (k < 0 || l < 0)
                                        {
                                            mountaindist[kk][ll] = dist * 10;
                                            timesten[kk][ll] = 1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Now sort out the heights

    float heightmult = 1.0f / maxradius;
    float heightmult2 = 1.0f / maxelev;

    float finalmult = 0.6f; // Reduce it all! Because we'll have raised ground underneath.

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            float thisheight = (float)mountainbaseheight[i][j];

            float thismountaindist = (float)mountaindist[i][j];

            if (timesten[i][j] == 1)
                thismountaindist = thismountaindist / 10.0f;

            if (thismountaindist < maxradius - 1.0f)
                thismountaindist = thismountaindist * 0.8f;

            if (thismountaindist < maxradius - 2.0f)
                thismountaindist = thismountaindist * 0.8f;

            float distmult = (float)radiusfractal[i][j];
            distmult = distmult * heightmult2;

            if (distmult < 0.95f)
                distmult = 0.95f;

            if (thismountaindist > 2)
                thismountaindist = thismountaindist * distmult;

            thisheight = thisheight * thismountaindist * heightmult;

            float thisheightmult = (float)heightsfractal[i][j]; // This is to vary it using the fractal
            thisheightmult = thisheightmult * heightmult2;

            if (thisheightmult < 0.95f)
                thisheightmult = 0.95f;

            thisheight = thisheight * thisheightmult;

            thisheight = thisheight * finalmult;

            if (thisheight > (float)mountainbaseheight[i][j])
                thisheight = (float)mountainbaseheight[i][j];

            mountainheights[i][j] = (int)thisheight;
        }
    }

    // Now we draw out the ridges of the mountains, based on similar distances from the central peaks.

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (mountaindist[i][j] != 0)
            {
                int ii = i; // Looking north
                int jj = j - 1;
                int dir = 1;

                if (jj >= 0)
                {
                    if (mountaindist[ii][jj] == mountaindist[i][j])
                    {
                        if (getridge(mountainridges, i, j, dir) == 0)
                        {
                            int code = getcode(dir);
                            mountainridges[i][j] = mountainridges[i][j] + code;
                        }
                    }
                }

                ii = i + 1; // Looking northeast
                jj = j - 1;
                dir = 2;

                if (ii <=width && jj >= 0)
                {
                    if (mountaindist[ii][jj] == mountaindist[i][j])
                    {
                        if (getridge(mountainridges, i, j, dir) == 0)
                        {
                            int code = getcode(dir);
                            mountainridges[i][j] = mountainridges[i][j] + code;
                        }
                    }
                }

                ii = i + 1; // Looking east
                jj = j;
                dir = 3;

                if (ii <= width)
                {
                    if (mountaindist[ii][jj] == mountaindist[i][j])
                    {
                        if (getridge(mountainridges, i, j, dir) == 0)
                        {
                            int code = getcode(dir);
                            mountainridges[i][j] = mountainridges[i][j] + code;
                        }
                    }
                }

                ii = i + 1; // Looking southeast
                jj = j + 1;
                dir = 4;

                if (ii <= width && jj <= height)
                {
                    if (mountaindist[ii][jj] == mountaindist[i][j])
                    {
                        if (getridge(mountainridges, i, j, dir) == 0)
                        {
                            int code = getcode(dir);
                            mountainridges[i][j] = mountainridges[i][j] + code;
                        }
                    }
                }

                ii = i; // Looking south
                jj = j + 1;
                dir = 5;

                if (jj <= height)
                {
                    if (mountaindist[ii][jj] == mountaindist[i][j])
                    {
                        if (getridge(mountainridges, i, j, dir) == 0)
                        {
                            int code = getcode(dir);
                            mountainridges[i][j] = mountainridges[i][j] + code;
                        }
                    }
                }

                ii = i - 1; // Looking southwest
                jj = j + 1;
                dir = 6;

                if (ii >= 0 && jj <= height)
                {
                    if (mountaindist[ii][jj] == mountaindist[i][j])
                    {
                        if (getridge(mountainridges, i, j, dir) == 0)
                        {
                            int code = getcode(dir);
                            mountainridges[i][j] = mountainridges[i][j] + code;
                        }
                    }
                }

                ii = i - 1; // Looking west
                jj = j;
                dir = 7;

                if (ii >= 0)
                {
                    if (mountaindist[ii][jj] == mountaindist[i][j])
                    {
                        if (getridge(mountainridges, i, j, dir) == 0)
                        {
                            int code = getcode(dir);
                            mountainridges[i][j] = mountainridges[i][j] + code;
                        }
                    }
                }

                ii = i - 1; // Looking northwest
                jj = j - 1;
                dir = 8;

                if (ii >= 0 && jj >= 0)
                {
                    if (mountaindist[ii][jj] == mountaindist[i][j])
                    {
                        if (getridge(mountainridges, i, j, dir) == 0)
                        {
                            int code = getcode(dir);
                            mountainridges[i][j] = mountainridges[i][j] + code;
                        }
                    }
                }
            }
        }
    }

    // Now we want to add some variation to the ridge directions.

    int varchance = 3; // The lower this is, the more variation there will be.
    int deletechance = 15; // The higher this is, the more extra ridges will be deleted.

    for (int i = 1; i < width; i++)
    {
        for (int j = 1; j < height; j++)
        {
            if (mountainridges[i][j] != 0)
            {
                int dir, dir2, ii, jj;

                if (getridge(mountainridges, i, j, 1) == 1 && random(1, varchance) == 1) // Looking north
                {
                    dir = 8;
                    dir2 = 4;
                    ii = i - 1;
                    jj = j - 1;

                    if (random(1, 2) == 1)
                    {
                        dir = 2;
                        dir2 = 6;
                        ii = i + 1;
                        jj = j - 1;
                    }

                    if (getridge(mountainridges, i, j, dir) == 0) // If there isn't one in this direction
                    {
                        int code = getcode(dir);
                        mountainridges[i][j] = mountainridges[i][j] + code;

                        code = getcode(dir2);
                        mountainridges[ii][jj] = mountainridges[ii][jj] + code;

                        if (mountainheights[ii][jj] == 0)
                            mountainheights[ii][jj] = mountainheights[i][j];

                        if (random(1, deletechance) > 10)
                            deleteridge(world, mountainridges, mountainheights, i, j, 1);
                    }
                }
                else
                {
                    if (getridge(mountainridges, i, j, 2) == 1 && random(1, varchance) == 1) // Looking northeast
                    {
                        dir = 1;
                        dir2 = 5;
                        ii = i;
                        jj = j - 1;

                        if (random(1, 2) == 1)
                        {
                            dir = 3;
                            dir2 = 7;
                            ii = i + 1;
                            jj = j;
                        }

                        if (ii<0 || ii>width)
                            ii = wrap(ii, width);

                        if (getridge(mountainridges, i, j, dir) == 0) // If there isn't one in this direction
                        {
                            int code = getcode(dir);
                            mountainridges[i][j] = mountainridges[i][j] + code;

                            code = getcode(dir2);
                            mountainridges[ii][jj] = mountainridges[ii][jj] + code;

                            if (mountainheights[ii][jj] == 0)
                                mountainheights[ii][jj] = mountainheights[i][j];

                            if (random(1, deletechance) > 10)
                                deleteridge(world, mountainridges, mountainheights, i, j, 2);
                        }
                    }
                    else
                    {
                        if (getridge(mountainridges, i, j, 3) == 1 && random(1, varchance) == 1) // Looking east
                        {
                            dir = 2;
                            dir2 = 6;
                            ii = i + 1;
                            jj = j - 1;

                            if (random(1, 2) == 1)
                            {
                                dir = 4;
                                dir2 = 8;
                                ii = i + 1;
                                jj = j + 1;
                            }

                            if (ii<0 || ii>width)
                                ii = wrap(ii, width);

                            if (getridge(mountainridges, i, j, dir) == 0) // If there isn't one in this direction
                            {
                                int code = getcode(dir);
                                mountainridges[i][j] = mountainridges[i][j] + code;

                                code = getcode(dir2);
                                mountainridges[ii][jj] = mountainridges[ii][jj] + code;

                                if (mountainheights[ii][jj] == 0)
                                    mountainheights[ii][jj] = mountainheights[i][j];

                                if (random(1, deletechance) > 10)
                                    deleteridge(world, mountainridges, mountainheights, i, j, 3);
                            }
                        }
                        else
                        {
                            if (getridge(mountainridges, i, j, 4) == 1 && random(1, varchance) == 1) // Looking southeast
                            {
                                dir = 3;
                                dir2 = 7;
                                ii = i + 1;
                                jj = j;

                                if (random(1, 2) == 1)
                                {
                                    dir = 5;
                                    dir2 = 1;
                                    ii = i;
                                    jj = j + 1;
                                }

                                if (ii<0 || ii>width)
                                    ii = wrap(ii, width);

                                if (getridge(mountainridges, i, j, dir) == 0) // If there isn't one in this direction
                                {
                                    int code = getcode(dir);
                                    mountainridges[i][j] = mountainridges[i][j] + code;

                                    code = getcode(dir2);
                                    mountainridges[ii][jj] = mountainridges[ii][jj] + code;

                                    if (mountainheights[ii][jj] == 0)
                                        mountainheights[ii][jj] = mountainheights[i][j];

                                    if (random(1, deletechance) > 10)
                                        deleteridge(world, mountainridges, mountainheights, i, j, 4);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Now apply the new mountain heights and ridges to the world.

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (mountainridges[i][j] != 0)
            {
                world.setmountainridge(face, i, j, mountainridges[i][j]);
                world.setmountainheight(face, i, j, mountainheights[i][j]);

                //OKmountains[i][j] = 1; // Mark them as user-added, so we won't tinker with their heights later.
            }
        }
    }
}

// This function makes mountain chains across continents.

void createcentralmountains(planet& world, int baseheight, int conheight, boolshapetemplate landshape[], boolshapetemplate chainland[])
{
    int edge = world.edge();
    int sealevel = world.sealevel();
    int maxelev = world.maxelevation();

    int minthickness = edge / 8; // Continents must be this thick to get mountains.
    int maxthickness = edge - edge / 8; // Continents must be no thicker than this.

    for (int face = 0; face < 6; face++)
    {
        int startx = -1;
        int starty = -1;
        int endx = -1;
        int endy = -1;

        for (int n = 0; n < 100; n++)
        {
            int i = random(1, edge - 2);

            if (world.nom(face, i, 1) < sealevel)
            {
                startx = i;
                n = 100;

                for (int m = 1; m < edge; m++)
                {
                    if (world.nom(face, startx, m) > sealevel)
                    {
                        starty = m;
                        m = edge;
                    }
                }
            }
        }

        if (startx != -1 && starty != -1)
        {
            for (int n = 0; n < 100; n++)
            {
                int i = random(1, edge - 2);

                if (world.nom(face, i, edge - 2) < sealevel)
                {
                    endx = i;
                    n = 100;

                    for (int m = edge - 2; m > 0; m--)
                    {
                        if (world.nom(face, endx, m) > sealevel)
                        {
                            endy = m;
                            m = 0;
                        }
                    }
                }
            }
        }

        if (startx != -1 && starty != -1 && endx != -1 && endy != -1) // Check the continent width here.
        {
            int xdist = startx - endx;
            int ydist = starty - endy;

            int dist = (int)sqrt(xdist * xdist + ydist * ydist);

            if (dist<minthickness || dist>maxthickness)
                startx = -1;
        }

        if (startx != -1 && starty != -1 && endx != -1 && endy != -1)
        {
            int grain = edge / 2;
            float valuemod = 8;
            int v = random(3, 6);
            float valuemod2 = (float)v;

            vector<vector<int>> fractal(edge, vector<int>(edge, 0));
            create2dfractalold(fractal, edge - 1, edge - 1, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

            createdirectedchain(world, face, baseheight, conheight, fractal, landshape, chainland, startx, starty, endx, endy, 1, 1, 0, 3.0f, 200, 0);
        }
        else // Try making one in the opposite direction.
        {
            for (int n = 0; n < 100; n++)
            {
                int j = random(1, edge - 2);

                if (world.nom(face, 1, j) < sealevel)
                {
                    starty = j;
                    n = 100;

                    for (int m = 1; m < edge; m++)
                    {
                        if (world.nom(face, m, starty) > sealevel)
                        {
                            startx = m;
                            m = edge;
                        }
                    }
                }
            }

            if (startx != -1 && starty != -1)
            {
                for (int n = 0; n < 100; n++)
                {
                    int j = random(1, edge - 2);

                    if (world.nom(face, edge - 2, j) < sealevel)
                    {
                        endy = j;
                        n = 100;

                        for (int m = edge - 2; m > 0; m--)
                        {
                            if (world.nom(face, m, endy) > sealevel)
                            {
                                endx = m;
                                m = 0;
                            }
                        }
                    }
                }
            }

            if (startx != -1 && starty != -1 && endx != -1 && endy != -1) // Check the continent width here.
            {
                int xdist = startx - endx;
                int ydist = starty - endy;

                int dist = (int)sqrt(xdist * xdist + ydist * ydist);

                if (dist<minthickness || dist>maxthickness)
                    startx = -1;
            }

            if (startx != -1 && starty != -1 && endx != -1 && endy != -1)
            {
                int grain = edge / 2;
                float valuemod = 8;
                int v = random(3, 6);
                float valuemod2 = (float)v;

                vector<vector<int>> fractal(edge, vector<int>(edge, 0));
                create2dfractalold(fractal, edge - 1, edge - 1, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

                //vector <twointegers> dummy1(2);

                //createdirectedchain(world, face, baseheight, conheight, fractal, landshape, chainland, startx, starty, endx, endy, 0, dummy1, 200);

                createdirectedchain(world, face, baseheight, conheight, fractal, landshape, chainland, startx, starty, endx, endy, 1, 1, 0, 3.0f, 200, 0);
            }
        }
    }
}

// This function creates a chain of mountains from one point to another.

void createdirectedchain(planet& world, int face, int baseheight, int conheight, vector<vector<int>>& fractal, boolshapetemplate landshape[], boolshapetemplate chainland[], int chainstartx, int chainstarty, int chainendx, int chainendy, bool crossingcontinent, bool muststayonland, bool muststayonsea, float heightmult, int volcanochance, int landdelete)
{
    int rangestartvar = 1; //2; // Possible distance between previous range and next one in a chain
    int changedirchance = 3; // Chance of changing direction (the lower it is, the more likely)
    int landchance = 10; // Chance of any given mountain pixel generating a nearby lump of land (the lower it is, the more likely)
    int minlanddist = random(2, 5);
    int maxlanddist = random(6, 12); // Range for how far lumps of land can be from the originating mountain
    int landvar = 4; // Maximum distance a lump of land can be displaced from its proper position
    int swervechance = 4; // Probability of the chain swerving from its destination.

    int totaldist = (int)sqrt((chainstartx - chainendx) * (chainstartx - chainendx) + (chainstarty - chainendy) * (chainstarty - chainendy));

    int waypointvary = totaldist / 4;
    int waypointminvary = waypointvary / 4;

    if (waypointvary < 2)
        waypointvary = 2;

    int edge = world.edge();
    int width = edge - 1;
    int height = edge - 1;
    int maxelev = world.maxelevation();
    int sealevel = world.sealevel();

    vector<vector<bool>> thischain(width + 1, vector<bool>(height + 1, 0));

    vector<vector<unsigned char>> rangeheighttemplate(height + 1, vector<unsigned char>(height + 1, 0));
    vector<vector<unsigned char>> rangeridgetemplate(height + 1, vector<unsigned char>(height + 1, 0)); // These will hold the range we're currently putting on.

    int halfwayx = (chainstartx + chainendx) / 2;
    int halfwayy = (chainstarty + chainendy) / 2;

    int waypoint1x = (chainstartx + halfwayx) / 2;
    int waypoint1y = (chainstarty + halfwayy) / 2;

    int waypoint2x = (halfwayx + chainendx) / 2;
    int waypoint2y = (halfwayy + chainendy) / 2;

    waypoint1x = waypoint1x + randomsign(random(waypointminvary, waypointvary));
    waypoint1y = waypoint1y + randomsign(random(waypointminvary, waypointvary));

    waypoint2x = waypoint2x + randomsign(random(waypointminvary, waypointvary));
    waypoint2y = waypoint2y + randomsign(random(waypointminvary, waypointvary));

    if (waypoint1x < 0)
        waypoint1x = 0;

    if (waypoint1x > width)
        waypoint1x = width;

    if (waypoint1y < 0)
        waypoint1y = 0;

    if (waypoint1y > height)
        waypoint1y = height;

    if (waypoint2x < 0)
        waypoint2x = 0;

    if (waypoint2x > width)
        waypoint2x = width;

    if (waypoint2y < 0)
        waypoint2y = 0;

    if (waypoint2y > height)
        waypoint2y = height;

    short goingto = 1;

    // Now do the chain.

    int oldx = chainstartx;
    int oldy = chainstarty;

    short chaindir = 0; // Chaindir is the direction of the chain, from 1-8 (sort of on the diagonals!).

    int rangeno = 1;
    bool goahead = 1;

    bool goingtowardsend = 1;
    bool justswerved = 0;

    int tempno = -1;

    do
    {
        int lasttemp = tempno;

        if (goingtowardsend == 1 || muststayonsea == 1)
            chaindir = getmountaindir(oldx, oldy, chainendx, chainendy);
        else
        {
            twointegers sea = nearestsea(world, face, oldx, oldy, 0, height, 10);

            chainendx = sea.x;
            chainendy = sea.y;

            chaindir = getmountaindir(oldx, oldy, chainendx, chainendy);
        }

        if (justswerved == 0 && random(1, swervechance) == 1)
        {
            chaindir = chaindir + randomsign(1);
            justswerved = 1;
        }
        else
            justswerved = 0;

        if (chaindir == 0)
            chaindir = 8;

        if (chaindir == 9)
            chaindir = 1;

        goahead = 1;

        do // Get a new template, making sure it's not the same as the last one.
        {
            tempno = random(1, MOUNTAINTEMPLATESTOTAL);
        } while (tempno == lasttemp);

        int dampner = maxelev / 12; // This is to reduce the maximum height a bit.
        int rangeheight = random((maxelev - sealevel) / 2, maxelev - sealevel); // Approximate height of the range (above sea level)
        rangeheight = rangeheight - dampner;

        if (rangeheight < 200)
            rangeheight = 200;

        rangeheight = (int)((float)rangeheight * heightmult);

        int rangeinc = rangeheight / 10; // Difference between the different template heights
        int rangelittleinc = rangeinc / 100; // Amount to vary the template heights by

        // Get the template.

        int thisdir = chaindir;

        if (random(1, 2) == 1)
            thisdir = thisdir + 4;

        if (thisdir > 8)
            thisdir = thisdir - 8;

        int span = createmountainrangetemplate(rangeridgetemplate, rangeheighttemplate, tempno, thisdir);

        // Now we need to find the start of the actual range within the template.

        int startx = 0;
        int starty = 0;

        if (chaindir == 1 || chaindir == 2)
        {
            for (int i = 0; i <= span; i++)
            {
                for (int j = 0; j <= span; j++)
                {
                    if (rangeridgetemplate[i][j] != 0)
                    {
                        startx = i;
                        starty = j;

                        i = span;
                        j = span;
                    }
                }
            }
        }

        if (chaindir == 3 || chaindir == 4)
        {
            for (int j = 0; j <= span; j++)
            {
                for (int i = 0; i <= span; i++)
                {
                    if (rangeridgetemplate[i][j] != 0)
                    {
                        startx = i;
                        starty = j;

                        i = span;
                        j = span;
                    }
                }
            }
        }

        if (chaindir == 5 || chaindir == 6)
        {
            for (int i = span; i >= 0; i--)
            {
                for (int j = 0; j <= span; j++)
                {
                    if (i >= 0 && i <= height && j >= 0 && j <= height && rangeridgetemplate[i][j] != 0)
                    {
                        startx = i;
                        starty = j;

                        i = 0;
                        j = span;
                    }
                }
            }
        }

        if (chaindir == 7 || chaindir == 8)
        {
            for (int j = span; j >= 0; j--)
            {
                for (int i = 0; i <= span; i++)
                {
                    if (rangeridgetemplate[i][j] != 0)
                    {
                        startx = i;
                        starty = j;

                        i = span;
                        j = 0;
                    }
                }
            }
        }

        if (chaindir == 1 || chaindir == 2)
        {
            oldx = oldx + random(0, rangestartvar);
            oldy = oldy + randomsign(random(0, rangestartvar));
        }

        if (chaindir == 3 || chaindir == 4)
        {
            oldx = oldx + randomsign(random(0, rangestartvar));
            oldy = oldy + random(0, rangestartvar);
        }

        if (chaindir == 5 || chaindir == 6)
        {
            oldx = oldx - random(0, rangestartvar);
            oldy = oldy + randomsign(random(0, rangestartvar));
        }

        if (chaindir == 7 || chaindir == 8)
        {
            oldx = oldx + randomsign(random(0, rangestartvar));
            oldy = oldy - random(0, rangestartvar);
        }

        if (oldx < 0)
            return;

        if (oldx > edge - 1)
            return;

        if (oldy < 0)
            return;

        if (oldy > edge - 1)
            return;

        // oldx and oldy are now the point where we want the new range to start.

        int x = oldx - startx;
        int y = oldy - starty;

        if (muststayonland) // These mountains can't start in the sea.
        {
            if (world.sea(face, oldx, oldy) == 1)
                goahead = 0;
        }

        if (muststayonsea) // These ones can't start on the land.
        {
            if (world.sea(face, oldx, oldy) == 0)
                goahead = 0;
        }

        // x and y are now the coordinates of the top left of the new range template.
        // Now we need to see whether there is a clear area to paste it.

        for (int i = x; i <= x + span; i++)
        {
            if (i >= 0 && i <= width)
            {
                for (int j = y; j <= y + span; j++)
                {
                    if (j >= 0 && j <= height && world.mountainheight(face, i, j) != 0 && thischain[i][j] == 0)
                    {
                        goahead = 0;
                        i = x + span;
                        j = y + span;
                    }
                }
            }
        }

        if (goahead == 1) // If the area is clear
        {
            for (int i = 0; i <= span; i++)
            {
                int ii = x + i;

                if (ii >= 0 && ii < edge)
                {
                    for (int j = 0; j <= span; j++)
                    {
                        int jj = y + j;

                        if (jj >= 0 && jj <= height)
                        {
                            if (ii == chainendx && jj == chainendy) // If we've reached the vicinity of the end point
                            {
                                if (crossingcontinent)
                                    goingtowardsend = 0;
                                else
                                    return;
                            }

                            if (world.mountainheight(face, ii, jj) == 0)
                            {
                                if (rangeheighttemplate[i][j] != 0)
                                {
                                    int h = rangeheighttemplate[i][j] * rangeinc;
                                    h = h + randomsign(rangelittleinc * random(0, 100));

                                    float frac = (float)fractal[ii][jj];
                                    frac = frac / (float)maxelev;
                                    h = (int)((float)h * frac);

                                    if (muststayonland)
                                    {
                                        if (world.map(face, ii, jj) >= conheight)
                                        {
                                            thischain[ii][jj] = 1;

                                            world.setmountainheight(face, ii, jj, h);
                                            world.setmountainridge(face, ii, jj, rangeridgetemplate[i][j]);
                                            world.setnom(face, ii, jj, conheight);
                                        }
                                        else
                                            goahead = 0;
                                    }

                                    if (muststayonsea) // Islands are just strange and different.
                                    {
                                        if (h > sealevel)
                                        {
                                            world.setmountainheight(face, ii, jj, h - sealevel);
                                            world.setmountainridge(face, ii, jj, rangeridgetemplate[i][j]);
                                            world.setnom(face, ii, jj, conheight);
                                        }
                                    }

                                    if (muststayonland == 0 && muststayonsea == 0) // Can form peninsulas.
                                    {
                                        if (world.nom(face, ii, jj) < sealevel) // If this is in the sea, we will end after this range.
                                            goahead = 0;

                                        thischain[ii][jj] = 1;

                                        world.setmountainheight(face, ii, jj, h);
                                        world.setmountainridge(face, ii, jj, rangeridgetemplate[i][j]);
                                        world.setnom(face, ii, jj, conheight);
                                    }
                                }
                            }
                        }
                        else // If this range goes off the map, end the chain.
                            goahead = 0;
                    }
                }
            }

            // Now we need to find the new oldx and oldy.

            if (chaindir == 1 || chaindir == 2)
            {
                for (int i = span; i >= 0; i--)
                {
                    for (int j = 0; j <= span; j++)
                    {
                        if (i >= 0 && i <= height && j >= 0 && j <= height && rangeheighttemplate[i][j] != 0)
                        {
                            oldx = x + i;
                            oldy = y + j;

                            i = 0;
                            j = span;
                        }
                    }
                }
            }

            if (chaindir == 3 || chaindir == 4)
            {
                for (int j = span; j >= 0; j--)
                {
                    for (int i = 0; i <= span; i++)
                    {
                        if (rangeheighttemplate[i][j] != 0)
                        {
                            oldx = x + i;
                            oldy = y + j;

                            j = 0;
                            i = span;
                        }
                    }
                }
            }

            if (chaindir == 5 || chaindir == 6)
            {
                for (int i = 0; i <= span; i++)
                {
                    for (int j = 0; j <= span; j++)
                    {
                        if (rangeheighttemplate[i][j] != 0)
                        {
                            oldx = x + i;
                            oldy = y + j;

                            i = span;
                            j = span;
                        }
                    }
                }
            }

            if (chaindir == 7 || chaindir == 8)
            {
                for (int j = 0; j <= span; j++)
                {
                    for (int i = 0; i <= span; i++)
                    {
                        if (rangeheighttemplate[i][j] != 0)
                        {
                            oldx = x + i;
                            oldy = y + j;

                            j = span;
                            i = span;
                        }
                    }
                }
            }
        }

        rangeno++;

    } while (goahead == 1);

    if (landdelete == 0)
        return;

    // Now for continent edge mountains we'll delete all the land on one side.

    if (landdelete == 1) // Delete to the north
    {
        for (int i = 0; i <= width; i++)
        {
            for (int j = 0; j <= height; j++)
            {
                if (thischain[i][j])
                {
                    for (int n = i; n >= 0; n--)
                        world.setnom(face, i, n, baseheight);
                }
            }
        }
    }

    if (landdelete == 2) // Delete to the east
    {
        for (int i = 0; i <= width; i++)
        {
            for (int j = 0; j <= height; j++)
            {
                if (thischain[i][j])
                {
                    for (int n = i; n <= width; n++)
                        world.setnom(face, n, j, baseheight);
                }
            }
        }
    }

    if (landdelete == 3) // Delete to the south
    {
        for (int i = 0; i <= width; i++)
        {
            for (int j = 0; j <= height; j++)
            {
                if (thischain[i][j])
                {
                    for (int n = i; n <= height; n++)
                        world.setnom(face, i, n, baseheight);
                }
            }
        }
    }

    if (landdelete == 4) // Delete to the west
    {
        for (int i = 0; i <= width; i++)
        {
            for (int j = 0; j <= height; j++)
            {
                if (thischain[i][j])
                {
                    for (int n = i; n >= 0; n--)
                        world.setnom(face, n, j, baseheight);
                }
            }
        }
    }

    // Now add more land around the actual mountains.

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (thischain[i][j] && random(1, landchance) == 1)
            {
                int landx = i + randomsign(random(minlanddist, maxlanddist));
                int landy = j + randomsign(random(minlanddist, maxlanddist));

                if (landx >= 0 && landx <= width && landy >= 0 && landy <= height)
                {
                    int shapenumber = random(0, 1);
                    drawshape(world, shapenumber, face, landx, landy, 1, baseheight, conheight, chainland);
                }
            }
        }
    }
}

// This function creates chains of mountains, with associated land where appropriate.

void createchains(planet& world, int baseheight, int conheight, vector<vector<vector<int>>>& fractal, boolshapetemplate landshape[], boolshapetemplate chainland[], twointegers focuspoints[], int focustotal, int focaldistance, int mode)
{
    // mode=0: continental chains (with associated land).
    // mode=1: smaller mountains (no associated land, can form peninsulas).
    // mode=2: smaller mountains (no associated land, can't form peninsulas).
    // mode=3: island chains.
    // mode=5: like mode 2, but fewer.
    // mode=6: like 3, but fewer.
    // mode=7: hills.

    bool lower = 0;

    int origmode = mode;

    if (mode == 7)
    {
        mode = 2;
        lower = 1;
    }

    bool fewer = 0;

    if (mode == 5)
    {
        mode = 2;
        fewer = 1;
    }

    if (mode == 6)
    {
        mode = 3;
        fewer = 1;
    }

    //int temptotal=24; // Total number of mountain range templates available.

    int rangestartvar = 2; //6; // Possible distance between previous range and next one in a chain
    int changedirchance = 3; // Chance of changing direction (the lower it is, the more likely)
    int landchance = 10; // Chance of any given mountain pixel generating a nearby lump of land (the lower it is, the more likely)
    int minlanddist = 5;
    int maxlanddist = 10; // Range for how far lumps of land can be from the previous one (or the originating mountain)
    int stringmax = 20; //10; // Maximum number of lumps in a string of land
    int landvar = 4; //8; // Maximum distance a lump of land can be displaced from its proper position
    int platchance = 4; // Chance of any range generating a plateau
    int platmaxheight = 30; // Percentage of the range height for the plateau maximum height
    int platminheight = 5; // Percentage of the range height for the plateau minimum height
    int minplatreduce = 80;
    int maxplatreduce = 95; // Range of percentages to reduce the height of the plateau with each step
    int mode1landmin = 30; // Minimum distance to land from the starting point of a mode 1 chain.

    int edge = world.edge();
    int maxelev = world.maxelevation();
    int sealevel = world.sealevel();

    float overallheightmult = (float)(random(50, 100)) / 100.0f;

    vector<vector<vector<int>>> ffractal(6, vector<vector<int>>(edge, vector<int>(edge, 0))); // This will be for ensuring that mountains nearby each other will have roughly the same height.

    int grain = 2; // Level of detail on this fractal map.
    float valuemod = 0.01f; // 0.03f;
    float valuemod2 = 0.01f; // 0.03f;

    if (mode != 3)
        createfractalold(ffractal, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    // We're going to think of the world as a gigantic rectangle, with the six faces unfolded onto it.

    int width = edge * 4 - 1;
    int height = edge * 3 - 1;

    // The mask array shows which parts of the gigantic rectangle actually correspond to faces. 1 = actual face, 0 = blank part.

    vector<vector<bool>> mask(width + 1, vector<bool>(height + 1, 0));

    for (int i = 0; i < edge; i++)
    {
        for (int j = 0; j <= height; j++)
            mask[i][j] = 1;
    }

    for (int i = edge; i <= width; i++)
    {
        for (int j = edge; j < edge * 2; j++)
            mask[i][j] = 1;
    }

    vector<vector<int>> nom(width + 1, vector<int>(height + 1, 0)); // This one shows land.
    vector<vector<int>> flatfractal(width + 1, vector<int>(height + 1, 0));
    vector<vector<int>> flatffractal(width + 1, vector<int>(height + 1, 0));
    vector<vector<int>> flatmountainheights(width + 1, vector<int>(height + 1, 0));
    vector<vector<int>> flatmountainridges(width + 1, vector<int>(height + 1, 0)); // We'll put the mountains onto these arrays, then copy them over onto the cubesphere at the end.

    for (int i = 0; i < edge; i++)
    {
        for (int j = 0; j < edge; j++)
        {
            nom[i][j] = world.nom(4, i, j);
            nom[i][edge + j] = world.nom(0, i, j);
            nom[i][edge * 2 + j] = world.nom(5, i, j);
            nom[edge + i][edge + j] = world.nom(1, i, j);
            nom[edge * 2 + i][edge + j] = world.nom(2, i, j);
            nom[edge * 3 + i][edge + j] = world.nom(3, i, j);

            flatfractal[i][j] = fractal[4][i][j];
            flatfractal[i][edge + j] = fractal[0][i][j];
            flatfractal[i][edge * 2 + j] = fractal[5][i][j];
            flatfractal[edge + i][edge + j] = fractal[1][i][j];
            flatfractal[edge * 2 + i][edge + j] = fractal[2][i][j];
            flatfractal[edge * 3 + i][edge + j] = fractal[3][i][j];

            flatffractal[i][j] = ffractal[4][i][j];
            flatffractal[i][edge + j] = ffractal[0][i][j];
            flatffractal[i][edge * 2 + j] = ffractal[5][i][j];
            flatffractal[edge + i][edge + j] = ffractal[1][i][j];
            flatffractal[edge * 2 + i][edge + j] = ffractal[2][i][j];
            flatffractal[edge * 3 + i][edge + j] = ffractal[3][i][j];

            flatmountainheights[i][j] = world.mountainheight(4, i, j);
            flatmountainridges[i][j] = world.mountainridge(4, i, j);
            flatmountainheights[i][edge + j] = world.mountainheight(0, i, j);
            flatmountainridges[i][edge + j] = world.mountainridge(0, i, j);
            flatmountainheights[i][edge * 2 + j] = world.mountainheight(5, i, j);
            flatmountainridges[i][edge * 2 + j] = world.mountainridge(5, i, j);
            flatmountainheights[edge + i][edge + j] = world.mountainheight(1, i, j);
            flatmountainridges[edge + i][edge + j] = world.mountainridge(1, i, j);
            flatmountainheights[edge * 2 + i][edge + j] = world.mountainheight(2, i, j);
            flatmountainridges[edge * 2 + i][edge + j] = world.mountainridge(2, i, j);
            flatmountainheights[edge * 3 + i][edge + j] = world.mountainheight(3, i, j);
            flatmountainridges[edge * 3 + i][edge + j] = world.mountainridge(3, i, j);
        }
    }

    vector<vector<unsigned char>> rangeheighttemplate(height + 1, vector<unsigned char>(height + 1, 0));
    vector<vector<unsigned char>> rangeridgetemplate(height + 1, vector<unsigned char>(height + 1, 0)); // These will hold the range we're currently putting on.

    int chainno = 0;
    int minchainlength = 0;
    int maxchainlength = 0;

    switch (mode)
    {
    case 0:

        chainno = random(5, 20); //(5,20);
        maxchainlength = 15;
        minchainlength = 4;
        break;

    case 1:

        chainno = random(20, 60); //(8,60);
        maxchainlength = 8;
        minchainlength = 1;
        break;

    case 2:

        chainno = random(20, 600); //(4000, 32000);

        if (fewer == 1)
            chainno = 1000;

        maxchainlength = 8;
        minchainlength = 1;
        break;

    case 3:

        chainno = random(20, 200); //(80,800);

        if (fewer == 1)
            chainno = 10;

        maxchainlength = 6;
        minchainlength = 1;
        break;
    }

    // Now do the chains.

    for (int chain = 1; chain <= chainno; chain++)
    {
        int oldx = random(0, width);
        int oldy = random(0, height);

        while (mask[oldx][oldy] == 0)
        {
            oldx = random(0, width);
            oldy = random(0, height);
        }

        if (mode == 0 || mode == 3) // Continental ranges should begin near the continental focus points, if possible. Islands should begin near their focus points too.
        {
            int thisfocus = random(0, focustotal);

            oldx = focuspoints[thisfocus].x;
            oldy = focuspoints[thisfocus].y; // Coordinates of the current blob.

            float xdiff = (float)random(1, 100);
            float ydiff = (float)random(1, 100);

            xdiff = xdiff / 100.0f;
            ydiff = ydiff / 100.0f;

            xdiff = xdiff * xdiff;
            ydiff = ydiff * ydiff;

            xdiff = xdiff * (float)focaldistance;
            ydiff = ydiff * (float)focaldistance;

            oldx = oldx + (int)randomsign(xdiff);
            oldy = oldy + (int)randomsign(ydiff);

            if (oldy < 0)
                oldy = 0;

            if (oldy > height)
                oldy = height;

            if (oldx<0 || oldx>width)
                oldx = wrap(oldx, width);
        }

        int chaindir = random(1, 8); // Chaindir is the direction of the chain, from 1-8 (sort of on the diagonals!).
        int maxranges = random(minchainlength, maxchainlength); // The chain won't have more ranges in it than this.
        int rangeno = 1;
        bool goahead = 1;

        int tempno = -1;

        if (!(mode == 2 && nom[oldx][oldy] <= sealevel)) // If we're placing land-only mountains and we're in the sea, just skip the whole thing.
        {
            do
            {
                int lasttemp = tempno;

                do // Get a new template, making sure it's not the same as the last one.
                {
                    tempno = random(1, MOUNTAINTEMPLATESTOTAL);
                } while (tempno == lasttemp);

                int dampner = 0; // 1800; //maxelev/7; // This is to reduce the maximum height a bit.
                int rangeheight = 0;

                /*
                switch (mode)
                {
                case 0:

                    rangeheight = random((maxelev - sealevel) / 2, maxelev - sealevel); // Approximate height of the range (above sea level)
                    rangeheight = rangeheight - dampner;

                    break;

                case 1:

                    rangeheight = random(200, (maxelev - sealevel) / 2);
                    rangeheight = rangeheight - dampner;

                    break;

                case 2:

                    rangeheight = random(100, (maxelev - sealevel) / 3);
                    rangeheight = rangeheight - dampner;

                    if (lower == 1)
                        rangeheight = rangeheight / 4;

                    break;

                case 3:

                    rangeheight = (maxelev - maxelev / 10) - sealevel; // random(sealevel / 2, maxelev - maxelev / 10);

                    break;
                }
                */

                rangeheight = (maxelev - maxelev / 10) - sealevel;

                float heightmult = flatffractal[oldx][oldy] / float(maxelev);

                rangeheight = (int)((float)rangeheight * heightmult * overallheightmult);

                if (lower == 1)
                    rangeheight = rangeheight / 4;

                if (rangeheight < 200)
                    rangeheight = 200;

                if (rangeheight > maxelev - sealevel)
                    rangeheight = maxelev - sealevel;

                bool plateau = 0;
                int platheight = 0;

                int rangeinc = rangeheight / 10; // Difference between the different template heights
                int rangelittleinc = rangeinc / 100; // Amount to vary the template heights by

                // Get the template.

                int thisdir = chaindir;

                if (random(1, 2) == 1)
                    thisdir = thisdir + 4;

                if (thisdir > 8)
                    thisdir = thisdir - 8;

                int span = createmountainrangetemplate(rangeridgetemplate, rangeheighttemplate, tempno, thisdir);

                // Now we need to find the start of the actual range within the template.

                int startx = 0;
                int starty = 0;

                if (chaindir == 1 || chaindir == 2)
                {
                    for (int i = 0; i <= span; i++)
                    {
                        for (int j = 0; j <= span; j++)
                        {
                            if (rangeridgetemplate[i][j] != 0)
                            {
                                startx = i;
                                starty = j;

                                i = span;
                                j = span;
                            }
                        }
                    }
                }

                if (chaindir == 3 || chaindir == 4)
                {
                    for (int j = 0; j <= span; j++)
                    {
                        for (int i = 0; i <= span; i++)
                        {
                            if (rangeridgetemplate[i][j] != 0)
                            {
                                startx = i;
                                starty = j;

                                i = span;
                                j = span;
                            }
                        }
                    }
                }

                if (chaindir == 5 || chaindir == 6)
                {
                    for (int i = span; i >= 0; i--)
                    {
                        for (int j = 0; j <= span; j++)
                        {
                            if (i >= 0 && i <= height && j >= 0 && j <= height && rangeridgetemplate[i][j] != 0)
                            {
                                startx = i;
                                starty = j;

                                i = 0;
                                j = span;
                            }
                        }
                    }
                }

                if (chaindir == 7 || chaindir == 8)
                {
                    for (int j = span; j >= 0; j--)
                    {
                        for (int i = 0; i <= span; i++)
                        {
                            if (rangeridgetemplate[i][j] != 0)
                            {
                                startx = i;
                                starty = j;

                                i = span;
                                j = 0;
                            }
                        }
                    }
                }

                if (chaindir == 1 || chaindir == 2)
                {
                    oldx = oldx + random(0, rangestartvar);
                    oldy = oldy + randomsign(random(0, rangestartvar));
                }

                if (chaindir == 3 || chaindir == 4)
                {
                    oldx = oldx + randomsign(random(0, rangestartvar));
                    oldy = oldy + random(0, rangestartvar);
                }

                if (chaindir == 5 || chaindir == 6)
                {
                    oldx = oldx - random(0, rangestartvar);
                    oldy = oldy + randomsign(random(0, rangestartvar));
                }

                if (chaindir == 7 || chaindir == 8)
                {
                    oldx = oldx + randomsign(random(0, rangestartvar));
                    oldy = oldy - random(0, rangestartvar);
                }

                if (oldx<0 || oldx>width)
                    oldx = wrap(oldx, width);

                if (oldy < 0)
                    oldy = 0;

                if (oldy > height)
                    oldy = height;

                // oldx and oldy are now the point where we want the new range to start.

                int x = oldx - startx;
                int y = oldy - starty;

                if (x < 0 || x > width)
                    x = wrap(x, width);

                if (mode == 1 && oldx >= 0 && oldx <= width && oldy >= 0 && oldy <= height && mask[oldx][oldy] == 1) // Peninsular mountains have to start within a certain distance of land.
                {
                    if (nom[oldx][oldy] <= sealevel)
                    {
                        bool foundone = 0;

                        for (int i = oldx - mode1landmin; i <= oldx + mode1landmin; i++)
                        {
                            int ii = i;

                            if (ii<0 || ii>width)
                                ii = wrap(ii, width);

                            for (int j = oldy - mode1landmin; j <= oldy + mode1landmin; j++)
                            {
                                if (j >= 0 && j <= height && mask[ii][j] == 1 && nom[ii][j] >= sealevel)
                                {
                                    foundone = 1;

                                    i = oldx + mode1landmin;
                                    j = oldy + mode1landmin;
                                }
                            }
                        }

                        if (foundone == 0)
                            goahead = 0;
                    }
                }

                if (mode == 2) // Non-continental, non-peninsular mountains can't start in the sea.
                {
                    if (nom[oldx][oldy] <= sealevel)
                        goahead = 0;
                }

                if (mode == 3) // Island chains can't start on the land.
                {
                    if (oldx<0 || oldx>width || oldy<0 || oldy>height)
                        goahead = 0;
                    else
                    {
                        if (nom[oldx][oldy] > sealevel || mask[oldx][oldy] == 0)
                            goahead = 0;
                    }
                }

                if (origmode == 7) // Hills need to avoid faces 4 and 5.
                {
                    if (oldy<edge || oldy>edge * 2)
                        goahead = 0;
                }

                // x and y are now the coordinates of the top left of the new range template.
                // Now we need to see whether there is a clear area to paste it.

                if (mode != 2)
                {
                    for (int i = x; i <= x + span; i++)
                    {
                        int ii = i;

                        if (ii<0 || ii>width)
                            ii = wrap(ii, width);

                        if (goahead)
                        {
                            for (int j = y; j <= y + span; j++)
                            {
                                if (j<0 || j>height)
                                    goahead = 0;
                                else
                                {
                                    if (mask[ii][j] == 0)
                                        goahead = 0;

                                    if (flatmountainheights[ii][j] != 0)
                                        goahead = 0;
                                }

                                if (origmode == 7)
                                {
                                    if (j<edge || j>edge * 2)
                                        goahead = 0;
                                }
                            }
                        }
                    }
                }

                int hislandadd = random(1000, 5000);

                if (goahead == 1) // If the area is clear
                {
                    vector<vector<int>> thisflatmountainheights(width + 1, vector<int>(height + 1, 0));
                    vector<vector<int>> thisflatmountainridges(width + 1, vector<int>(height + 1, 0)); // We'll put the mountains onto these arrays, then copy them over onto the cubesphere at the end.

                    for (int i = 0; i <= span; i++)
                    {
                        int ii = x + i;
                        if (ii<0 || ii>width)
                            ii = wrap(ii, width);

                        for (int j = 0; j <= span; j++)
                        {
                            int jj = y + j;

                            if (jj >= 0 && jj <= height)
                            {
                                if (mask[ii][jj] == 1) // && flatmountainheights[ii][jj] == 0)
                                {
                                    if (i >= 0 && i <= width && j >= 0 && j <= height && rangeheighttemplate[i][j] != 0)
                                    {
                                        int h = rangeheighttemplate[i][j] * rangeinc;
                                        h = h + randomsign(rangelittleinc * random(0, 100));

                                        if (mode == 3)
                                            h = h + hislandadd;

                                        float frac = (float)flatfractal[ii][jj];
                                        frac = frac / (float)maxelev;
                                        h = (int)((float)h * frac);

                                        if (mode == 0 || mode == 1)
                                        {
                                            thisflatmountainheights[ii][jj] = h;
                                            thisflatmountainridges[ii][jj] = rangeridgetemplate[i][j];
                                        }

                                        if (mode == 2) // Mountains of this kind cannot create their own land, so must stop at the sea.
                                        {
                                            if (nom[ii][jj] >= conheight)
                                            {
                                                thisflatmountainheights[ii][jj] = h;
                                                thisflatmountainridges[ii][jj] = rangeridgetemplate[i][j];
                                            }
                                        }

                                        if (mode == 3) // Islands are just strange and different.
                                        {
                                            if (h > sealevel)
                                            {
                                                thisflatmountainheights[ii][jj] = h - sealevel;
                                                thisflatmountainridges[ii][jj] = rangeridgetemplate[i][j];
                                                nom[ii][jj] = conheight;
                                            }
                                        }

                                        /*
                                        if (mode == 0) // Only create substantial land if these are continental mountains.
                                        {
                                            if (random(1, landchance) == 1) // See if we'll paste a chunk of land nearby
                                            {
                                                int dist = random(minlanddist, maxlanddist);

                                                int movex = 0;
                                                int movey = 0;

                                                switch (chaindir)
                                                {
                                                case 1:
                                                    movex = 0 - dist / 2;
                                                    movey = 0 - dist;
                                                    break;

                                                case 2:
                                                    movex = dist / 2;
                                                    movey = 0 - dist;
                                                    break;

                                                case 3:
                                                    movex = dist;
                                                    movey = 0 - dist / 2;
                                                    break;

                                                case 4:
                                                    movex = dist;
                                                    movey = dist / 2;
                                                    break;

                                                case 5:
                                                    movex = dist / 2;
                                                    movey = dist;
                                                    break;

                                                case 6:
                                                    movex = 0 - dist / 2;
                                                    movey = dist;
                                                    break;

                                                case 7:
                                                    movex = 0 - dist;
                                                    movey = dist / 2;
                                                    break;

                                                case 8:
                                                    movex = 0 - dist;
                                                    movey = 0 - dist / 2;
                                                    break;
                                                }

                                                int landx = ii;
                                                int landy = jj;

                                                int max = rangeno;
                                                int max2 = maxranges - rangeno;

                                                if (max2 < max)
                                                    max = max2;

                                                int mult = 4;

                                                max = max * mult;

                                                if (max > stringmax)
                                                    max = stringmax;

                                                int totallumps = random((max / 4) * 3, max);

                                                for (int lump = 1; lump <= totallumps; lump++) // Now we will paste a string of land lumps leading away from the mountain.
                                                {
                                                    landx = landx + movex;
                                                    landy = landy + movey;

                                                    landx = landx + randomsign(random(0, landvar));
                                                    landy = landy + randomsign(random(0, landvar));

                                                    if (landx<0 || landx>width)
                                                        landx = wrap(landx, width);

                                                    int shapenumber = random(0, 1);
                                                    drawshape(world, shapenumber, landx, landy, 1, baseheight, conheight, chainland);

                                                }
                                            }
                                        }
                                        */

                                        if (mode == 1) // Mountains of this kind can create land just immediately around themselves.
                                        {
                                            if (nom[ii][jj] < conheight)
                                                nom[ii][jj] = conheight;
                                        }

                                        if (mode == 3) // Island mountains can create land just immediately around themselves.
                                        {
                                            if (h > sealevel)
                                            {
                                                if (nom[ii][jj] < conheight)
                                                    nom[ii][jj] = conheight;
                                            }
                                        }
                                    }
                                }
                                else // If this range goes off the map, and the chain.
                                    goahead = 0;
                            }
                        }
                    }

                    if (goahead)
                    {
                        for (int i = 0; i <= width; i++)
                        {
                            for (int j = 0; j <= height; j++)
                            {
                                if (thisflatmountainheights[i][j] != 0)
                                {
                                    flatmountainheights[i][j] = thisflatmountainheights[i][j];
                                    flatmountainridges[i][j] = thisflatmountainridges[i][j];

                                    if (mode == 0 || mode == 1)
                                        nom[i][j] = conheight;

                                    if (mode == 2)
                                    {
                                        if (fewer == 0)
                                            nom[i][j] = conheight;
                                    }
                                }
                            }
                        }
                    }

                    // Now we need to find the new oldx and oldy.

                    if (chaindir == 1 || chaindir == 2)
                    {
                        for (int i = span; i >= 0; i--)
                        {
                            for (int j = 0; j <= span; j++)
                            {
                                if (i >= 0 && i <= height && j >= 0 && j <= height && rangeheighttemplate[i][j] != 0)
                                {
                                    oldx = x + i;
                                    oldy = y + j;

                                    if (oldx<0 || oldx>width)
                                        oldx = wrap(oldx, width);

                                    i = 0;
                                    j = span;
                                }
                            }
                        }
                    }

                    if (chaindir == 3 || chaindir == 4)
                    {
                        for (int j = span; j >= 0; j--)
                        {
                            for (int i = 0; i <= span; i++)
                            {
                                if (rangeheighttemplate[i][j] != 0)
                                {
                                    oldx = x + i;
                                    oldy = y + j;

                                    if (oldx<0 || oldx>width)
                                        oldx = wrap(oldx, width);

                                    j = 0;
                                    i = span;
                                }
                            }
                        }
                    }

                    if (chaindir == 5 || chaindir == 6)
                    {
                        for (int i = 0; i <= span; i++)
                        {
                            for (int j = 0; j <= span; j++)
                            {
                                if (rangeheighttemplate[i][j] != 0)
                                {
                                    oldx = x + i;
                                    oldy = y + j;

                                    if (oldx<0 || oldx>width)
                                        oldx = wrap(oldx, width);

                                    i = span;
                                    j = span;
                                }
                            }
                        }
                    }

                    if (chaindir == 7 || chaindir == 8)
                    {
                        for (int j = 0; j <= span; j++)
                        {
                            for (int i = 0; i <= span; i++)
                            {
                                if (rangeheighttemplate[i][j] != 0)
                                {
                                    oldx = x + i;
                                    oldy = y + j;

                                    if (oldx<0 || oldx>width)
                                        oldx = wrap(oldx, width);

                                    j = span;
                                    i = span;
                                }
                            }
                        }
                    }

                    // Now change direction, possibly.

                    if (random(1, changedirchance) == 1)
                    {
                        chaindir = chaindir + randomsign(1);

                        if (chaindir > 8)
                            chaindir = 1;

                        if (chaindir < 1)
                            chaindir = 8;
                    }
                }

                rangeno++;

                if (rangeno > maxranges)
                    goahead = 0;

            } while (goahead == 1);
        }
    }

    /*
    // Adjust heights.

    if (1==1) //(mode == 2 || mode == 3)
    {
        for (int i = 0; i <= width; i++)
        {
            for (int j = 0; j <= height; j++)
            {
                if (flatmountainheights[i][j] != 0)
                    flatmountainheights[i][j] = flatmountainheights[i][j] + flatfractal[i][j] / 4;
            }
        }
    }
    */

    // Now paste it all back onto the cubesphere.

    int fracdiv = maxelev; // 4;
    int addamount = 0; // 1000;

    if (mode < 2)
    {
        fracdiv = maxelev;
        addamount = 0;
    }

    for (int i = 0; i < edge; i++)
    {
        for (int j = 0; j < edge; j++)
        {
            world.setnom(4, i, j, nom[i][j]);
            world.setnom(0, i, j, nom[i][edge + j]);
            world.setnom(5, i, j, nom[i][edge * 2 + j]);
            world.setnom(1, i, j, nom[edge + i][edge + j]);
            world.setnom(2, i, j, nom[edge * 2 + i][edge + j]);
            world.setnom(3, i, j, nom[edge * 3 + i][edge + j]);

            if (flatmountainheights[i][j] > 0)
            {
                world.setmountainheight(4, i, j, flatmountainheights[i][j] + fractal[4][i][j] / fracdiv + addamount);
                world.setmountainridge(4, i, j, flatmountainridges[i][j]);
            }

            if (flatmountainheights[i][edge + j] > 0)
            {
                world.setmountainheight(0, i, j, flatmountainheights[i][edge + j] + fractal[0][i][j] / fracdiv + addamount);
                world.setmountainridge(0, i, j, flatmountainridges[i][edge + j]);
            }

            if (flatmountainheights[i][edge * 2 + j] > 0)
            {
                world.setmountainheight(5, i, j, flatmountainheights[i][edge * 2 + j] + fractal[5][i][j] / fracdiv + addamount);
                world.setmountainridge(5, i, j, flatmountainridges[i][edge * 2 + j]);
            }

            if (flatmountainheights[edge + i][edge + j] > 0)
            {
                world.setmountainheight(1, i, j, flatmountainheights[edge + i][edge + j] + fractal[1][i][j] / fracdiv + addamount);
                world.setmountainridge(1, i, j, flatmountainridges[edge + i][edge + j]);
            }

            if (flatmountainheights[edge * 2 + i][edge + j] > 0)
            {
                world.setmountainheight(2, i, j, flatmountainheights[edge * 2 + i][edge + j] + fractal[2][i][j] / fracdiv + addamount);
                world.setmountainridge(2, i, j, flatmountainridges[edge * 2 + i][edge + j]);
            }

            if (flatmountainheights[edge * 3 + i][edge + j] > 0)
            {
                world.setmountainheight(3, i, j, flatmountainheights[edge * 3 + i][edge + j] + fractal[3][i][j] / fracdiv + addamount);
                world.setmountainridge(3, i, j, flatmountainridges[edge * 3 + i][edge + j]);
            }
        }
    }

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.mountainheight(face, i, j) < 0)
                {
                    for (int dir = 1; dir <= 8; dir++)
                        deleteridge(world, face, i, j, dir);
                }
            }
        }
    }
}

// This function removes any mountains that aren't over land.

void removefloatingmountains(planet& world)
{
    return;
    
    int edge = world.edge();

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.sea(face,i, j) == 1)
                {
                    if (world.mountainridge(face, i, j) != 0)
                    {
                        for (int dir = 1; dir <= 8; dir++)
                            deleteridge(world, face,i, j, dir);
                    }

                    world.setcraterrim(face, i, j, 0);
                }
            }
        }
    }
}

// This function removes any inland seas from the map. level is level to fill them in to.

void removeinlandseas(planet& world, int level)
{
    int edge = world.edge();
    int sealevel = world.sealevel();

    int minseasize = ((edge * edge) * 6) / 4;

    vector<bool> checked(6 * edge * edge, 0); // This array marks which sea areas have already been scanned.
    vector<int> area(6 * edge * edge, 0); // This array marks the area of each bit of sea.

    int seax = -1;
    int seay = -1;

    // First, we find the area of each section of sea and mark them on the area array.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = edge - 2; i > 0; i--)
        {
            int vi = vface + i * edge;

            for (int j = edge - 2; j > 0; j--)
            {
                int index = vi + j;

                if (world.nom(face,i, j) <= sealevel && checked[index] == 0 && area[index] == 0)
                {
                    int size = areacheck(world, checked, face, i, j);

                    for (int thisface = 0; thisface < 6; thisface++)  // Clear the checked array and mark all adjoining points with this area.
                    {
                        int vtface = thisface * edge * edge;

                        for (int k = 0; k < edge; k++)
                        {
                            int vk = vtface + k * edge;

                            for (int l = 0; l < edge; l++)
                            {
                                int lindex = vk + l;

                                if (checked[lindex] == 1)
                                {
                                    area[lindex] = size;
                                    checked[lindex] = 0;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Now we just find a point that's in the largest area of sea.

    int largest = 0;

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                if (area[index] > largest)
                {
                    largest = area[index];
                    seax = i;
                    seay = j;
                }
            }
        }
    }

    // Now we simply remove any sea tiles that aren't part of the largest area of sea.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                if (world.nom(face, i, j) <= sealevel && area[vi + j] != largest)
                    world.setnom(face, i, j, level);

                if (world.nom(face, i, j) > sealevel && world.nom(face, i, j) < level) // Remove any odd low-lying bits around the areas that have just been filled in.
                    world.setnom(face, i, j, level);
            }
        }
    }
}

// This function removes any small seas from the map. level is level to fill them in to.

void removesmallseas(planet& world, int minseasize, int level)
{
    int edge = world.edge();
    int sealevel = world.sealevel();

    vector<bool> checked(6 * edge * edge, 0); // This array marks which sea areas have already been scanned.
    vector<int> area(6 * edge * edge, 0); // This array marks the area of each bit of sea.

    int seax = -1;
    int seay = -1;

    // First, we find the area of each section of sea and mark them on the area array.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = edge - 2; i > 0; i--)
        {
            int vi = vface + i * edge;

            for (int j = edge - 2; j > 0; j--)
            {
                int index = vi + j;

                if (world.nom(face, i, j) <= sealevel && checked[index] == 0 && area[index] == 0)
                {
                    int size = areacheck(world, checked, face, i, j);

                    for (int thisface = 0; thisface < 6; thisface++)
                    {
                        int vtface = thisface * edge * edge;

                        for (int k = 0; k < edge; k++) // Clear the checked array and mark all adjoining points with this area.
                        {
                            int vk = vtface + k * edge;

                            for (int l = 0; l < edge; l++)
                            {
                                int tindex = vk + l;

                                if (checked[tindex] == 1)
                                {
                                    area[tindex] = size;
                                    checked[tindex] = 0;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Now turn all the points whose area is too small into land.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                if (world.nom(face, i, j) <= sealevel && area[vi + j] < minseasize)
                    world.setnom(face, i, j, level);
            }
        }
    }
}

// This tells how large a given area of sea is.

int areacheck(planet& world, vector<bool>& checked, int startface, int startx, int starty)
{
    int edge = world.edge();
    int sealevel = world.sealevel();

    int total = 0;

    int row[] = { -1,0,0,1 };
    int col[] = { 0,-1,1,0 };

    globepoint node;

    node.face = startface;
    node.x = startx;
    node.y = starty;

    queue<globepoint> q; // Create a queue
    q.push(node); // Put our starting node into the queue

    while (q.empty() != 1) // Keep going while there's anything left in the queue
    {
        node = q.front(); // Take the node at the front of the queue
        q.pop(); // And then pop it out of the queue

        if (node.face != -1 && node.x >= 0 && node.x < edge && node.y >= 0 && node.y < edge && world.nom(node.face, node.x, node.y) <= sealevel)
        {
            total++;
            checked[node.face * edge * edge + node.x * edge + node.y] = 1;

            for (int k = 0; k < 4; k++) // Look at the four neighbouring nodes in turn
            {
                globepoint nextnode = getglobepoint(edge, node.face, node.x, node.y, row[k], col[k]);

                if (nextnode.face != -1)
                {
                    int nindex = nextnode.face * edge * edge + nextnode.x * edge + nextnode.y;
                    if (world.nom(nextnode.face, nextnode.x, nextnode.y) <= sealevel && checked[nindex] == 0) // If this node is sea
                    {
                        checked[nindex] = 1;
                        q.push(nextnode); // Put that node onto the queue
                    }
                }
            }
        }
    }

    return total;
}

// This function removes any gaps in the continental shelves

void removeshelfgaps(planet& world, vector<vector<vector<bool>>>& shelves)
{
    int edge = world.edge();
    int eedge = edge * edge;

    int minseasize = ((edge * edge) * 6) / 4;

    vector<bool> checked(6 * edge * edge, 0); // This array marks which sea areas have already been scanned.
    vector<int> area(6 * edge * edge, 0); // This array marks the area of each bit of sea.

    int seax = -1;
    int seay = -1;

    // First, we find the area of each section of non-shelf and mark them on the area array.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * eedge;

        for (int i = edge - 2; i > 0; i--)
        {
            int vi = vface + i * edge;

            for (int j = edge - 2; j > 0; j--)
            {
                int index = vi + j;

                if (shelves[face][i][j] == 0 && checked[index] == 0 && area[index] == 0)
                {
                    int size = nonshelfareacheck(world, shelves, checked, face, i, j);

                    for (int thisface = 0; thisface < 6; thisface++)  // Clear the checked array and mark all adjoining points with this area.
                    {
                        int vtface = thisface * eedge;

                        for (int k = 0; k < edge; k++)
                        {
                            int vk = vtface + k * edge;

                            for (int l = 0; l < edge; l++)
                            {
                                int lindex = vk + l;

                                if (checked[lindex] == 1)
                                {
                                    area[lindex] = size;
                                    checked[lindex] = 0;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Now we just find a point that's in the largest area of non-shelf.

    int largest = 0;

    for (int face = 0; face < 6; face++)
    {
        int vface = face * eedge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                if (area[index] > largest)
                {
                    largest = area[index];
                    seax = i;
                    seay = j;
                }
            }
        }
    }


    // Now we simply remove any non-shelf tiles that aren't part of the largest area of non-shelf.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * eedge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                if (area[index] != largest)
                    shelves[face][i][j] = 1;
            }
        }
    }
}

// This tells how large a given area of non-shelf sea is.

int nonshelfareacheck(planet& world, vector<vector<vector<bool>>>& shelves, vector<bool>& checked, int startface, int startx, int starty)
{
    int edge = world.edge();
    int sealevel = world.sealevel();

    int eedge = edge * edge;

    int total = 0;

    int row[] = { -1,0,0,1 };
    int col[] = { 0,-1,1,0 };

    globepoint node;

    node.face = startface;
    node.x = startx;
    node.y = starty;

    queue<globepoint> q; // Create a queue
    q.push(node); // Put our starting node into the queue

    while (q.empty() != 1) // Keep going while there's anything left in the queue
    {
        node = q.front(); // Take the node at the front of the queue
        q.pop(); // And then pop it out of the queue

        if (node.face!=-1 && node.x >= 0 && node.x < edge && node.y >= 0 && node.y < edge && shelves[node.face][node.x][node.y] == 0)
        {
            total++;
            checked[node.face * eedge + node.x * edge + node.y] = 1;

            for (int k = 0; k < 4; k++) // Look at the four neighbouring nodes in turn
            {
                globepoint nextnode = getglobepoint(edge, node.face, node.x, node.y, row[k], col[k]);

                if (nextnode.face != -1)
                {
                    int nindex = nextnode.face * eedge + nextnode.x * edge + nextnode.y;

                    if (shelves[nextnode.face][nextnode.x][nextnode.y] == 0 && checked[nindex] == 0) // If this node is sea
                    {
                        checked[nindex] = 1;
                        q.push(nextnode); // Put that node onto the queue
                    }
                }
            }
        }
    }
    return total;
}

// This makes the continental shelves.

void makecontinentalshelves(planet& world, vector<vector<vector<bool>>>& shelves, vector<fourglobepoints>& dirpoint, int pointdist, boolshapetemplate landshape[])
{
    int edge = world.edge();
    int width = edge - 1;
    int height = edge - 1;
    int maxelev = world.maxelevation();
    int midelev = maxelev / 2;

    int eedge = edge * edge;

    // First, sort out the outline.

    vector<bool> outline(6 * edge * edge, 0);
    vector<int> fractal(6 * edge * edge, 0);
    vector<int> southshiftfractal(6 * edge * edge, 0);
    vector<int> eastshiftfractal(6 * edge * edge, 0);

    int grain = 8; // Level of detail on this fractal map.
    float valuemod = 0.2f;
    float valuemod2 = 3.0f;

    createfractal(fractal, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);
    
    createfractal(southshiftfractal, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);
    createfractal(eastshiftfractal, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    int maxwarp = edge / 8; // 60;
    int warpdiv = maxelev / maxwarp;
    int maxradius = edge / 25; // 20;
    int raddiv = maxelev / maxradius;

    twofloats pt, mm1, mm2, mm3;

    for (int face = 0; face < 6; face++)
    {
        int vface = face * eedge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                if (world.outline(face,i, j) == 1)
                {
                    int index = vi + j;

                    // First, work out where the centre of this circle will be. It's getting offset to make the shelves more interesting.

                    int warpdistx = 0;
                    int warpdisty = 0;

                    if (eastshiftfractal[index] > maxelev / 2)
                        warpdistx = (eastshiftfractal[index] - midelev) / warpdiv;
                    else
                        warpdistx = 0 - (midelev - eastshiftfractal[index]) / warpdiv;

                    if (southshiftfractal[index] > maxelev / 2)
                        warpdisty = (southshiftfractal[index] - midelev) / warpdiv;
                    else
                        warpdisty = 0 - (midelev - southshiftfractal[index]) / warpdiv;

                    globepoint centrepoint;

                    centrepoint.face = face;
                    centrepoint.x = i;
                    centrepoint.y = j;

                    if (warpdistx > 0)
                    {                       
                        for (int n = 0; n < warpdistx; n++)
                            centrepoint = dirpoint[centrepoint.face * eedge + centrepoint.x * edge + centrepoint.y].east;
                    }
                    else
                    {
                        for (int n = 0; n > warpdistx; n--)
                            centrepoint = dirpoint[centrepoint.face * eedge + centrepoint.x * edge + centrepoint.y].west;
                    }

                    if (warpdisty > 0)
                    {
                        for (int n = 0; n < warpdisty; n++)
                            centrepoint = dirpoint[centrepoint.face * eedge + centrepoint.x * edge + centrepoint.y].south;
                    }
                    else
                    {
                        for (int n = 0; n > warpdisty; n--)
                            centrepoint = dirpoint[centrepoint.face * eedge + centrepoint.x * edge + centrepoint.y].north;
                    }

                    // Now we can draw a circle.

                    int radius = fractal[index] / raddiv;

                    for (int x = -radius; x <= radius; x++)
                    {
                        for (int y = -radius; y <= radius; y++)
                        {
                            globepoint thispoint = getglobepoint(edge, centrepoint.face, centrepoint.x, centrepoint.y, x, y);
                            
                            if (thispoint.face != -1 && x * x + y * y < radius * radius + radius)
                                outline[thispoint.face * eedge + thispoint.x * edge + thispoint.y] = 1;
                        }
                    }
                }
            }
        }
    }
    
    // And anywhere that's land is continental shelf too, technically.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * eedge;

        for (int i = 0; i <= width; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < height; j++)
            {
                if (world.sea(face, i, j) == 0)
                    outline[vi + j] = 1;
            }
        }
    }

    // Now, we need a voronoi map.

    vector<int> voronoi(6 * edge * edge, 0);

    makeregularvoronoi(voronoi, edge, pointdist);

    // Now, every panel of the voronoi map that has any outline in it is continental shelf.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * eedge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                if (outline[index] == 1 && shelves[face][i][j] == 0)
                {
                    int thiscell = voronoi[index];

                    // Identify just a small area of the globe to search in.

                    bool searchface[6];

                    for (int n = 0; n < 6; n++)
                        searchface[n] = 0;

                    int startx[6];
                    int starty[6];
                    int endx[6];
                    int endy[6];

                    int margin = pointdist * 3;

                    searchface[face] = 1;
                    startx[face] = i - margin;
                    starty[face] = j - margin;
                    endx[face] = i + margin;
                    endy[face] = j + margin;

                    if (face == 0)
                    {
                        if (j < margin)
                        {
                            searchface[4] = 1;
                            startx[4] = i - margin;
                            starty[4] = edge - margin;
                            endx[4] = i + margin;
                            endy[4] = edge - 1;
                        }

                        if (j > edge - margin)
                        {
                            searchface[5] = 1;
                            startx[5] = i - margin;
                            starty[5] = 0;
                            endx[5] = i + margin;
                            endy[5] = margin;
                        }

                        if (i < margin)
                        {
                            searchface[3] = 1;
                            startx[3] = edge - margin;
                            starty[3] = j - margin;
                            endx[3] = edge - 1;
                            endy[3] = j + margin;
                        }

                        if (i > edge - margin)
                        {
                            searchface[1] = 1;
                            startx[1] = 0;
                            starty[1] = j - margin;
                            endx[1] = margin;
                            endy[1] = j + margin;
                        }
                    }

                    if (face == 1)
                    {
                        if (j < margin)
                        {
                            searchface[4] = 1;
                            startx[4] = edge - margin;
                            starty[4] = edge - (i + margin);
                            endx[4] = edge - 1;
                            endy[4] = edge - (i - margin);
                        }

                        if (j > edge - margin)
                        {
                            searchface[5] = 1;
                            startx[5] = edge - margin;
                            starty[5] = i - margin;
                            endx[5] = edge - 1;
                            endy[5] = i + margin;
                        }

                        if (i < margin)
                        {
                            searchface[0] = 1;
                            startx[0] = edge - margin;
                            starty[0] = j - margin;
                            endx[0] = edge - 1;
                            endy[0] = j + margin;
                        }

                        if (i > edge - margin)
                        {
                            searchface[2] = 1;
                            startx[2] = 0;
                            starty[2] = j - margin;
                            endx[2] = margin;
                            endy[2] = j + margin;
                        }
                    }

                    if (face == 2)
                    {
                        if (j < margin)
                        {
                            searchface[4] = 1;
                            startx[4] = edge - (i - margin);
                            starty[4] = 0;
                            endx[4] = edge - (i + margin);
                            endy[4] = margin;
                        }

                        if (j > edge - margin)
                        {
                            searchface[5] = 1;
                            startx[5] = edge - (i - margin);
                            starty[5] = edge - margin;
                            endx[5] = edge - (i + margin);
                            endy[5] = edge - 1;
                        }

                        if (i < margin)
                        {
                            searchface[1] = 1;
                            startx[1] = edge - margin;
                            starty[1] = j - margin;
                            endx[1] = edge - 1;
                            endy[1] = j + margin;
                        }

                        if (i > edge - margin)
                        {
                            searchface[3] = 1;
                            startx[3] = 0;
                            starty[3] = j - margin;
                            endx[3] = margin;
                            endy[3] = j + margin;
                        }
                    }

                    if (face == 3)
                    {
                        if (j < margin)
                        {
                            searchface[4] = 1;
                            startx[4] = 0;
                            starty[4] = i - margin;
                            endx[4] = margin;
                            endy[4] = i + margin;
                        }

                        if (j > edge - margin)
                        {
                            searchface[5] = 1;
                            startx[5] = 0;
                            starty[5] = edge - (i + margin);
                            endx[5] = margin;
                            endy[5] = edge - (i - margin);
                        }

                        if (i < margin)
                        {
                            searchface[2] = 1;
                            startx[2] = edge - margin;
                            starty[2] = j - margin;
                            endx[2] = edge - 1;
                            endy[2] = j + margin;
                        }

                        if (i > edge - margin)
                        {
                            searchface[0] = 1;
                            startx[0] = 0;
                            starty[0] = j - margin;
                            endx[0] = margin;
                            endy[0] = j + margin;
                        }
                    }

                    if (face == 4)
                    {
                        if (j < margin)
                        {
                            searchface[2] = 1;
                            startx[2] = edge - (i + margin);
                            starty[2] = 0;
                            endx[2] = edge - (i - margin);
                            endy[2] = margin;
                        }

                        if (j > edge - margin)
                        {
                            searchface[0] = 1;
                            startx[0] = i - margin;
                            starty[0] = 0;
                            endx[0] = i + margin;
                            endy[0] = margin;
                        }

                        if (i < margin)
                        {
                            searchface[3] = 1;
                            startx[3] = j - margin;
                            starty[3] = 0;
                            endx[3] = j + margin;
                            endy[3] = margin;
                        }

                        if (i > edge - margin)
                        {
                            searchface[1] = 1;
                            startx[1] = edge - (i + margin);
                            starty[1] = 0;
                            endx[1] = edge - (i - margin);
                            endy[1] = margin;
                        }
                    }

                    if (face == 5)
                    {
                        if (j < margin)
                        {
                            searchface[0] = 1;
                            startx[0] = i - margin;
                            starty[0] = edge - margin;
                            endx[0] = i + margin;
                            endy[0] = edge - 1;
                        }

                        if (j > margin)
                        {
                            searchface[2] = 1;
                            startx[2] = edge - (i + margin);
                            starty[2] = edge - margin;
                            endx[2] = edge - (i - margin);
                            endy[2] = edge - 1;
                        }

                        if (i < margin)
                        {
                            searchface[3] = 1;
                            startx[3] = edge - (j + margin);
                            starty[3] = edge - margin;
                            endx[3] = edge - (j - margin);
                            endy[3] = edge - 1;
                        }

                        if (i > margin)
                        {
                            searchface[1] = 1;
                            startx[1] = j - margin;
                            starty[1] = edge - margin;
                            endx[1] = j + margin;
                            endy[1] = edge - 1;
                        }

                    }

                    for (int n = 0; n < 6; n++)
                    {
                        if (startx[n] < 0)
                            startx[n] = 0;

                        if (starty[n] < 0)
                            starty[n] = 0;

                        if (endx[n] >= edge)
                            endx[n] = edge - 1;

                        if (endy[n] >= edge)
                            endy[n] = edge - 1;
                    }

                    // Now search just that area of the globe.

                    for (int thisface = 0; thisface < 6; thisface++)
                    {
                        if (searchface[thisface])
                        {
                            int vtface = thisface * eedge;

                            for (int x = startx[thisface]; x <= endx[thisface]; x++)
                            {
                                int vx = vtface + x * edge;

                                for (int y = starty[thisface]; y <= endy[thisface]; y++)
                                {
                                    int tindex = vx + y;

                                    if (voronoi[tindex] == thiscell)
                                        shelves[thisface][x][y] = 1;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Now we need to remove any holes in the continental shelves!

    removeshelfgaps(world, shelves);
}

// This function adds "cuts" to the map, i.e. adds or takes away shapes from the land.

void cuts(planet& world, int cuttotal, int baseheight, int conheight, boolshapetemplate shape[])
{
    int edge = world.edge();
    
    int removechance = random(2, 3); // The higher this is, the fewer cuts will be taken away.

    for (int cutno = 1; cutno <= cuttotal; cutno++)
    {
        int shapenumber = random(1, 11);

        int centreface = random(0, 5);
        int centrex = random(0, edge - 1);
        int centrey = random(0, edge - 1);

        if (world.sea(centreface, centrex, centrey) == 0) // Check to see whether this is going to be in the land or the sea.
            drawshape(world, shapenumber, centreface, centrex, centrey, 1, baseheight, conheight, shape);

        else
        {
            if (random(1, removechance) == 1)
                drawshape(world, shapenumber, centreface, centrex, centrey, 0, baseheight, conheight, shape);

        }
    }
}

// This function merges the fractal map into the main map to create more interesting terrain.

void fractalmerge(planet& world, int adjust, vector<vector<vector<int>>>& fractal)
{
    int edge = world.edge();
    int sealevel = world.sealevel();
    int maxelev = world.maxelevation();

    int eedge = edge * edge;

    vector<int> temp(6 * edge * edge, 0);

    int grain = 2;
    float valuemod = 0.5f;
    float valuemod2 = 3.0f;

    int min = random(30, 90);
    int max = 100;
    bool extreme = 1;

    createfractal(temp, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    float addbit = (float)adjust; // adjust is -5 to 25. 1 for very little land taken away, 25 for lots.

    addbit = addbit / 100.0f;

    float div1 = 1.5f + addbit; // This is the amount we divide the fractal height by over the sea. The higher this number, the more the fractal will eat away at the land masses.

    float div2 = div1 - 1.2f; // This is the amount we divide the fractal height by over land. The higher this number, the flatter the land will be.

    for (int face = 0; face < 6; face++)
    {
        int vface = face + eedge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                int perc;

                if (world.sea(face, i, j) == 0)
                    perc = 65;
                else
                    perc = 85;

                perc = (perc + temp[index]) / 2;

                int oldcol = world.map(face, i, j);
                int newcol = tilt(oldcol, (int)((float)fractal[face][i][j] / div1), perc);

                if (newcol > sealevel)
                    newcol = tilt(oldcol, (int)((float)fractal[face][i][j] / div2), perc);

                if (newcol <= sealevel)
                    world.setnom(face, i, j, newcol);
            }
        }
    }
}

// This function merges the fractal map into the main map, but only on land.

void fractalmergeland(planet& world, vector<vector<vector<int>>>& fractal, int conheight)
{
    int div = 150; // The higher this number, the more highlands there will be.
    int maxamount = 25; // The higher this is, the flatter the flat areas will be.
    int minamount = 10; // The lower this is, the higher the high areas will be.

    int edge = world.edge();
    int sealevel = world.sealevel();
    int maxelev = world.maxelevation();

    int eedge = edge * edge;

    // First we create a template map, which we will use to vary divamount.

    vector<int> templ(6 * edge * edge, 0);

    int grain = 4; // Level of detail on this fractal map.
    float valuemod = 0.02f;
    float valuemod2 = 3.0f;

    createfractal(templ, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    // Now we use that template to apply the fractal to the map, to create highlands and lowlands.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * eedge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                if (world.nom(face,i, j) > sealevel)
                {
                    int index = vi + j;

                    int valtoadd = fractal[face][i][j] - sealevel;

                    int divamount = templ[index];
                    divamount = divamount / div;
                    if (divamount > maxamount)
                        divamount = maxamount;
                    if (divamount < minamount)
                        divamount = minamount;

                    valtoadd = valtoadd / divamount;

                    int newval = conheight + valtoadd;

                    if (newval > maxelev)
                        newval = maxelev;

                    if (newval > world.nom(face,i, j))
                        world.setnom(face,i, j, newval);
                }
            }
        }
    }
}

// Merges maps together for the larger continental style.

void fractalmergemodified(planet& world, int adjust, vector<vector<vector<int>>>& fractal, vector<vector<vector<bool>>>& removedland)
{
    int edge = world.edge();
    int sealevel = world.sealevel();
    int maxelev = world.maxelevation();

    int eedge = edge * edge;


    vector<int> temp(6 * edge * edge, 0);
    vector<int> tipping(6 * edge * edge, 0);

    int grain = 4; // Level of detail on this fractal map.
    float valuemod = 0.01f;
    float valuemod2 = 0.01f;

    bool extreme = 0;

    createfractalformodifiedmerging(temp, edge, grain, valuemod, valuemod2, 1, maxelev, extreme);

    grain = 8;
    valuemod = 0.2f;
    valuemod2 = 3.0f;

    createfractal(tipping, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    int tippingadjust = random(1, maxelev / 2);

    for (int face = 0; face < 6; face++)
    {
        int vface = face * eedge;

        for (int j = 0; j < edge; j++)
        {
            for (int i = 0; i < edge; i++)
            {
                int index = vface + i * edge + j;

                tipping[index] = tipping[index] + tippingadjust;

                if (tipping[index] < 1)
                    tipping[index] = 1;

                if (tipping[index] > maxelev)
                    tipping[index] = maxelev;

            }
        }
    }

    adjust = adjust * 200; // adjust is -5 to 25. 1 for very little land taken away, 25 for lots.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * eedge;

        for (int j = 0; j < edge; j++)
        {
            for (int i = 0; i < edge; i++)
            {
                int index = vface + i * edge + j;

                if (temp[index] > tipping[index])
                {
                    if (fractal[face][i][j] - adjust <= sealevel && world.nom(face,i, j) > sealevel)
                    {
                        int newval = fractal[face][i][j] - adjust;

                        if (newval < 1)
                            newval = 1;

                        world.setnom(face, i, j, newval);
                        removedland[face][i][j] = 1;
                    }
                }
            }
        }
    }
}

// This function ensures that channels of sea are at least two pixels wide.

void widenchannels(planet& world)
{
    int edge = world.edge();
    int sealevel = world.sealevel();

    for (int face = 0; face < 6; face++)
    {
        for (int i = 1; i < edge - 1; i++) // First remove any one-pixel channels.
        {
            for (int j = 1; j < edge - 1; j++)
            {
                if (world.sea(face,i, j))
                {
                    if (world.sea(face,i, j - 1) == 0 && world.sea(face,i, j + 1) == 0)
                        world.setnom(face,i, j - 1, sealevel - 15);

                    if (world.sea(face,i - 1, j) == 0 && world.sea(face,i + 1, j) == 0)
                        world.setnom(face,i - 1, j, sealevel - 15);

                    if (world.sea(face,i - 1, j - 1) == 0 && world.sea(face,i + 1, j + 1) == 0)
                        world.setnom(face,i - 1, j - 1, sealevel - 15);

                    if (world.sea(face,i - 1, j + 1) == 0 && world.sea(face,i + 1, j - 1) == 0)
                        world.setnom(face,i - 1, j + 1, sealevel - 15);
                }
            }
        }
    }

    for (int face = 0; face < 6; face++)
    {
        for (int i = 2; i < edge - 2; i++) // Now remove any two-pixel channels.
        {
            for (int j = 2; j < edge - 2; j++)
            {
                if (world.sea(face,i, j))
                {
                    if (world.sea(face,i, j - 1) == 0 && world.sea(face,i, j + 1) == 1 && world.sea(face,i, j + 2) == 0)
                        world.setnom(face,i, j, sealevel - 15);

                    if (world.sea(face,i - 1, j) == 0 && world.sea(face,i + 1, j) == 1 && world.sea(face,i + 2, j) == 0)
                        world.setnom(face,i + 1, j, sealevel - 15);
                }
            }
        }
    }
}

// This creates mid-ocean ridges.

void createoceanridges(planet& world, vector<vector<vector<bool>>>& shelves, vector<float>& longitude, vector<float>& latitude)
{
    int edge = world.edge();
    int maxelev = world.maxelevation();

    int eedge = edge * edge;

    int f1 = eedge;
    int f2 = f1 * 2;
    int f3 = f1 * 3;
    int f4 = f1 * 4;
    int f5 = f1 * 5;

    vector<int> nearestshelfdist(6 * edge * edge, 0);
    vector<globepoint> nearestshelf(6 * edge * edge);
    vector<int> grid(6 * edge * edge, 0);
    vector<int> gridnumbers(6 * edge * edge, 0);
    vector<int> ridges(6 * edge * edge, 0);
    vector<int> ridgesmap(6 * edge * edge, 0);
    vector<int> ridgedistances(6 * edge * edge, 0);
    vector<bool> edgepoints(6 * edge * edge, 0);
    vector<bool> boundaries(6 * edge * edge, 0);

    // We need a grid of edge points - points where the coastal shelves meet ocean.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * eedge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                if (shelves[face][i][j] == 1 && shelfedge(world, shelves, face, i, j) == 1)
                    edgepoints[index] = 1;
            }
        }
    }

    // Now we need to go over all the ocean points and work out their closest edge points.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * eedge;

        for (int i = 0; i < edge; i++) // Every cell that *is* an edge point is closest to itself.
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                if (edgepoints[index] == 1)
                {
                    nearestshelfdist[index] = 1;
                    nearestshelf[index].face = face;
                    nearestshelf[index].x = i;
                    nearestshelf[index].y = j;
                }
            }
        }
    }

    for (int n = 0; n < edge; n++) // There are problems going over these borders between faces, so put some artifical starting points there too.
    {
        int vn = n * edge;

        nearestshelfdist[f1 + vn + edge - 1] = 1;
        nearestshelf[f1 + vn + edge - 1].face = 5;
        nearestshelf[f1 + vn + edge - 1].x = edge - 1;
        nearestshelf[f1 + vn + edge - 1].y = n;

        nearestshelfdist[f2 + vn + edge - 1] = 1;
        nearestshelf[f2 + vn + edge - 1].face = 5;
        nearestshelf[f2 + vn + edge - 1].x = edge - 1;
        nearestshelf[f2 + vn + edge - 1].y = n;

        nearestshelfdist[f5 + (edge - 1) * edge + n] = 1;
        nearestshelf[f5 + (edge - 1) * edge + n].face = 5;
        nearestshelf[f5 + (edge - 1) * edge + n].x = edge - 1;
        nearestshelf[f5 + (edge - 1) * edge + n].y = n;

        nearestshelfdist[f5 + vn + edge - 1] = 1;
        nearestshelf[f5 + vn + edge - 1].face = 5;
        nearestshelf[f5 + vn + edge - 1].x = edge - 1;
        nearestshelf[f5 + vn + edge - 1].y = n;
    }

    bool keepgoing = 1;

    do // Now spread those out from the coasts.
    {
        keepgoing = 0;

        vector<bool> justdid(6 * edge * edge, 0);
        
        for (int face = 0; face < 6; face++)
        {
            int vface = face * eedge;

            int ai = 0;
            int bi = edge - 1;
            int ci = 1;

            if (random(1, 2) == 1)
            {
                ai = edge - 1;
                bi = 0;
                ci = -1;
            }

            int aj = 0;
            int bj = edge - 1;
            int cj = 1;

            if (random(1, 2) == 1)
            {
                aj = edge - 1;
                bj = 0;
                cj = -1;
            }
                       
            for (int i = ai; i != bi; i = i + ci)
            {
                int vi = vface + i * edge;

                for (int j = aj; j != bj; j = j + cj)
                {
                    int index = vi + j;

                    if (shelves[face][i][j] == 0)
                    {
                        if (nearestshelfdist[index] == 0)
                        {
                            for (int k = -1; k <= 1; k++)
                            {
                                for (int l = -1; l <= 1; l++)
                                {
                                    globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                                    if (lpoint.face != -1)
                                    {
                                        int lindex = lpoint.face * eedge + lpoint.x * edge + lpoint.y;

                                        if (nearestshelfdist[lindex] != 0 && justdid[lindex] == 0)
                                        {
                                            nearestshelfdist[index] = nearestshelfdist[lindex] + 1;
                                            nearestshelf[index] = nearestshelf[lindex];

                                            if (random(1, 4) != 1)
                                                justdid[index] = 1;

                                            k = 1;
                                            l = 1;
                                            keepgoing = 1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    } while (keepgoing);

    // Increase the distances for smaller worlds, so they still produce ridges.

    if (world.size() != 2)
    {
        int factor = 4;

        if (world.size() == 0)
            factor = 8;

        for (int face = 0; face < 6; face++)
        {
            int vface = face * eedge;

            for (int i = 0; i < edge; i++)
            {
                int vi = vface + i * edge;

                for (int j = 0; j < edge; j++)
                {
                    int index = vi + j;

                    nearestshelfdist[index] = nearestshelfdist[index] * factor;
                }
            }
        }
    }

    // Now we need to find where the zones meet.

    int maxdiff = 100; //400; // If neighbouring cells have closest edgepoints that are further away than this, it means they are on the boundary between different zones.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * eedge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                if (nearestshelfdist[index] > 0)
                {
                    bool found = 0;

                    for (int k = - 1; k <= 1; k++)
                    {
                        for (int l = -1; l <= 1; l++)
                        {
                            globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                            if (lpoint.face != -1)
                            {
                                int lindex = lpoint.face * eedge + lpoint.x * edge + lpoint.y;

                                if (shelves[face][i][j] == 0 && shelves[lpoint.face][lpoint.x][lpoint.y] == 0)
                                {
                                    if (boundaries[lindex] == 0 && getpointdist(edge, nearestshelf[index].face, nearestshelf[index].x, nearestshelf[index].y, nearestshelf[lindex].face, nearestshelf[lindex].x, nearestshelf[lindex].y) > maxdiff)
                                    {
                                        boundaries[index] = 1;
                                        found = 1;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Now mark that on the ridges array.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * eedge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                if (boundaries[index] == 1)
                    ridges[index] = nearestshelfdist[index];

            }
        }
    }

    // Now get rid of excess points around the diagonals.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * eedge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                if (ridges[index] != 0)
                {
                    globepoint rightpoint = getglobepoint(edge, face, i, j, 1, 0);
                    globepoint downpoint = getglobepoint(edge, face, i, j, 0, 1);
                    globepoint uppoint = getglobepoint(edge, face, i, j, 0, -1);
                    
                    if (rightpoint.face != -1 && downpoint.face != -1 && uppoint.face != -1)
                    {
                        globepoint diagdownpoint = getglobepoint(edge, rightpoint.face, rightpoint.x, rightpoint.y, 0, 1);
                        globepoint diaguppoint = getglobepoint(edge, rightpoint.face, rightpoint.x, rightpoint.y, 0, -1);

                        if (diagdownpoint.face != -1 && ridges[diagdownpoint.face * eedge + diagdownpoint.x * edge + diagdownpoint.y] != 0)
                        {
                            ridges[downpoint.face * eedge + downpoint.x * edge + downpoint.y] = 0;
                            ridges[rightpoint.face * eedge + rightpoint.x * edge + rightpoint.y] = 0;
                        }

                        if (diaguppoint.face != -1 && ridges[diaguppoint.face * eedge + diaguppoint.x * edge + diaguppoint.y] != 0)
                        {
                            ridges[uppoint.face * eedge + uppoint.x * edge + uppoint.y] = 0;
                            ridges[rightpoint.face * eedge + rightpoint.x * edge + rightpoint.y] = 0;
                        }
                    }
                }
            }
        }
    }

    // Now make sure each ridge point is adjacent to no more than one other ridge point.

    bool found = 0;

    do
    {
        found = 0;

        for (int face = 0; face < 6; face++)
        {
            int vface = face * eedge;

            for (int i = 0; i < edge; i++)
            {
                int vi = vface + i * edge;

                for (int j = 0; j < edge; j++)
                {
                    int index = vi + j;

                    if (ridges[index] != 0)
                    {
                        int neighbours = -1; // Because we'll add one for itself

                        for (int k = - 1; k <= 1; k++)
                        {
                            for (int l = -1; l <= 1; l++)
                            {
                                globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                                if (lpoint.face != -1)
                                {
                                    if (ridges[lpoint.face * eedge + lpoint.x * edge + lpoint.y] != 0)
                                        neighbours++;

                                }
                            }
                        }

                        if (neighbours > 2)
                        {
                            found = 1;

                            for (int k = - 1; k <= 1; k++)
                            {
                                for (int l = -1; l <= 1; l++)
                                {
                                    globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                                    if (lpoint.face != -1)
                                    {
                                        int lindex = lpoint.face * eedge + lpoint.x * edge + lpoint.y;

                                        if (ridges[lindex] != 0)
                                        {
                                            ridges[lindex] = 0;
                                            k = 1;
                                            l = 1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

    } while (found == 1);

    // Now raise the land around the ridges.

    const int firstmaxradius = 70;//50; // The bigger this is, the wider the ridge areas will be.
    int heightmult = 6; // The bigger this is, the higher the ridge areas will be.
    int maxvolcanoradius = 6; // Volcanoes can't be further than this from the central ridge.

    float thisheightperthousand[firstmaxradius + 1]; // Create a lookup table for these values, for speed.

    thisheightperthousand[1] = 1.1037f;

    for (int n = 2; n <= firstmaxradius; n++)
        thisheightperthousand[n] = thisheightperthousand[n - 1] * thisheightperthousand[1];

    for (int face = 0; face < 6; face++)
    {
        int vface = face * eedge;

        for (int i = 0; i < edge; i = i + 1) // First, the ridges.
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j = j + 1)
            {
                int index = vi + j;

                if (ridges[index] != 0 && random(1,4) == 1)
                {
                    float riftheight = (float)(ridges[index] * heightmult);
                    float riftheightdiv = riftheight / 1000.0f;

                    int mult = 0;

                    for (int radius = firstmaxradius; radius >= 1; radius--)
                    {
                        mult++;

                        float thisheight = riftheightdiv * thisheightperthousand[mult];

                        for (int k = -radius; k <= radius; k++)
                        {
                            for (int l = -radius; l <= radius; l++)
                            {
                                if (k * k + l * l < radius * radius + radius)
                                {
                                    globepoint thispoint = getglobepoint(edge, face, i, j, k, l);

                                    if (thispoint.face != -1)
                                    {
                                        int tindex = thispoint.face * eedge + thispoint.x * edge + thispoint.y;

                                        if (ridgesmap[tindex] < (int)thisheight)
                                        {
                                            ridgesmap[tindex] = (int)thisheight;
                                            ridgedistances[tindex] = mult;
                                            world.setoceanridgeheights(thispoint.face, thispoint.x, thispoint.y, (int)thisheight);

                                            if (radius <= maxvolcanoradius && random(1, 4000000) < thisheight / 10)
                                                world.setvolcano(thispoint.face, thispoint.x, thispoint.y, random((int)(thisheight / 4.0f), (int)((thisheight / 4.0f) * 3.0f)));
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    int maxradius = 50; // The bigger this is, the wider the ridge areas will be.
    heightmult = 6; // The bigger this is, the higher the ridge areas will be.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * eedge;

        for (int i = 0; i < edge; i++) // Now add some more for just the land.
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                if (ridges[index] != 0)
                {
                    int heightdivs = ((ridges[index] * heightmult) / maxradius);
                    int mult = 0;

                    for (int radius = maxradius; radius >= 1; radius--)
                    {
                        mult++;

                        int thisheight = heightdivs * mult;

                        thisheight = (thisheight * 3 + (ridges[index] * heightmult) / radius) / 4; // 3, 4

                        for (int k = -radius; k <= radius; k++)
                        {
                            for (int l = -radius; l <= radius; l++)
                            {
                                if (k * k + l * l < radius * radius + radius)
                                {
                                    globepoint thispoint = getglobepoint(edge, face, i, j, k, l);

                                    if (thispoint.face!=-1)
                                    {
                                        int tindex = thispoint.face * eedge + thispoint.x * edge + thispoint.y;

                                        if (ridgesmap[tindex] < thisheight)
                                            ridgesmap[tindex] = thisheight;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Add the rift in the middle

    for (int face = 0; face < 6; face++)
    {
        int vface = face * eedge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                if (ridges[index] != 0)
                {
                    ridgesmap[index] = ridgesmap[index] / 2;
                    world.setoceanrifts(face, i, j, ridgesmap[index]);
                }
            }
        }
    }

    // Mark out the ridge mountains, following the contours of the raised land

    for (int face = 0; face < 6; face++)
    {
        int vface = face * eedge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                if (ridgedistances[index] != 0)
                {
                    globepoint thispoint = getglobepoint(edge, face, i, j, 0, -1);
                    int dir = 1;

                    if (thispoint.face != -1)
                    {
                        if (ridgedistances[thispoint.face * eedge + thispoint.x * edge + thispoint.y] == ridgedistances[index])
                        {
                            if (getoceanridge(world, face, i, j, dir) == 0)
                            {
                                int code = getcode(dir);
                                world.setoceanridges(face, i, j, world.oceanridges(face, i, j) + code);
                            }
                        }
                    }

                    thispoint = getglobepoint(edge, face, i, j, 1, -1);
                    dir = 2;

                    if (thispoint.face != -1)
                    {
                        if (ridgedistances[thispoint.face * eedge + thispoint.x * edge + thispoint.y] == ridgedistances[index])
                        {
                            if (getoceanridge(world, face, i, j, dir) == 0)
                            {
                                int code = getcode(dir);
                                world.setoceanridges(face, i, j, world.oceanridges(face, i, j) + code);
                            }
                        }
                    }

                    thispoint = getglobepoint(edge, face, i, j, 1, 0);
                    dir = 3;

                    if (thispoint.face != -1)
                    {
                        if (ridgedistances[thispoint.face * eedge + thispoint.x * edge + thispoint.y] == ridgedistances[index])
                        {
                            if (getoceanridge(world, face, i, j, dir) == 0)
                            {
                                int code = getcode(dir);
                                world.setoceanridges(face, i, j, world.oceanridges(face, i, j) + code);
                            }
                        }
                    }

                    thispoint = getglobepoint(edge, face, i, j, 1, 1);
                    dir = 4;

                    if (thispoint.face != -1)
                    {
                        if (ridgedistances[thispoint.face * eedge + thispoint.x * edge + thispoint.y] == ridgedistances[index])
                        {
                            if (getoceanridge(world, face, i, j, dir) == 0)
                            {
                                int code = getcode(dir);
                                world.setoceanridges(face, i, j, world.oceanridges(face, i, j) + code);
                            }
                        }
                    }

                    thispoint = getglobepoint(edge, face, i, j, 0, 1);
                    dir = 5;

                    if (thispoint.face != -1)
                    {
                        if (ridgedistances[thispoint.face * eedge + thispoint.x * edge + thispoint.y] == ridgedistances[index])
                        {
                            if (getoceanridge(world, face, i, j, dir) == 0)
                            {
                                int code = getcode(dir);
                                world.setoceanridges(face, i, j, world.oceanridges(face, i, j) + code);
                            }
                        }
                    }

                    thispoint = getglobepoint(edge, face, i, j, -1, 1);
                    dir = 6;

                    if (thispoint.face != -1)
                    {
                        if (ridgedistances[thispoint.face * eedge + thispoint.x * edge + thispoint.y] == ridgedistances[index])
                        {
                            if (getoceanridge(world, face, i, j, dir) == 0)
                            {
                                int code = getcode(dir);
                                world.setoceanridges(face, i, j, world.oceanridges(face, i, j) + code);
                            }
                        }
                    }

                    thispoint = getglobepoint(edge, face, i, j, -1, 0);
                    dir = 7;

                    if (thispoint.face != -1)
                    {
                        if (ridgedistances[thispoint.face * eedge + thispoint.x * edge + thispoint.y] == ridgedistances[index])
                        {
                            if (getoceanridge(world, face, i, j, dir) == 0)
                            {
                                int code = getcode(dir);
                                world.setoceanridges(face, i, j, world.oceanridges(face, i, j) + code);
                            }
                        }
                    }

                    thispoint = getglobepoint(edge, face, i, j, -1, -1);
                    dir = 8;

                    if (thispoint.face != -1)
                    {
                        if (ridgedistances[thispoint.face * eedge + thispoint.x * edge + thispoint.y] == ridgedistances[index])
                        {
                            if (getoceanridge(world, face, i, j, dir) == 0)
                            {
                                int code = getcode(dir);
                                world.setoceanridges(face, i, j, world.oceanridges(face, i, j) + code);
                            }
                        }
                    }
                }
            }
        }
    }

    // Now remove extraneous ridges

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.oceanridges(face, i, j) != 0)
                {
                    short total = 0;

                    for (int dir = 1; dir <= 8; dir++)
                    {
                        if (getoceanridge(world, face, i, j, dir) == 1)
                            total++;
                    }

                    if (total > 2)
                    {
                        int extra = total - 2;

                        for (int n = 1; n <= extra; n++)
                        {
                            int a = 1;
                            int b = 8;
                            int c = 1;

                            if (random(1, 2) == 1)
                            {
                                a = 8;
                                b = 1;
                                c = -1;
                            }

                            for (int dir = a; dir != b; dir = dir + c)
                            {
                                if (getoceanridge(world, face, i, j, dir) == 1)
                                    deleteoceanridge(world, face, i, j, dir);
                            }
                        }
                    }
                }
            }
        }
    }

    // Now we'll make a fractal. This will be used to displace the ridges in the regional map.

    int maxdisplace = 16; // Maximum displacement of the ridges in the regional map.

    int grain = 128; // Level of detail on this fractal map.
    float valuemod = 4.0f;
    float valuemod2 = 8.0f;

    vector<int> ridgefractal(6 * edge * edge, 0);

    createfractal(ridgefractal, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    float div = (float)maxelev / ((float)maxdisplace * 2.0f);

    for (int face = 0; face < 6; face++)
    {
        int vface = face * eedge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                float amount = (float)ridgefractal[vi + j];

                amount = amount / div;
                amount = amount - (float)maxdisplace;

                if (amount < 0.0f - (float)maxdisplace)
                    amount = 0.0f - (float)maxdisplace;

                if (amount > (float)maxdisplace)
                    amount = (float)maxdisplace;

                world.setoceanridgeoffset(face, i, j, (int)amount);
            }
        }
    }

    // Remove any ridge-related stuff that's on continental plates.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * eedge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                if (shelves[face][i][j] == 1 || world.sea(face, i, j) == 0)
                {
                    ridgesmap[index] = 0;
                    world.setoceanrifts(face, i, j, 0);
                    world.setoceanridges(face, i, j, 0);
                    world.setoceanridgeheights(face, i, j, 0);
                    world.setoceanridgeangle(face, i, j, 0);
                    world.setoceanridgeoffset(face, i, j, 0);
                }
            }
        }
    }

    // Draw the raised land onto the actual sea bed.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * eedge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
                world.setnom(face, i, j, world.nom(face, i, j) + ridgesmap[vi + j]);
        }
    }
}

// This checks to see whether the given point is on the edge of a continental shelf.

bool shelfedge(planet& world, vector<vector<vector<bool>>>& shelves, int face, int x, int y)
{
    if (shelves[face][x][y] == 0)
        return 0;

    int edge = world.edge();

    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            globepoint jpoint = getglobepoint(edge, face, x, y, i, j);

            if (jpoint.face != -1)
            {
                if (shelves[jpoint.face][jpoint.x][jpoint.y] == 0)
                    return 1;
            }
            else
                return 0;
        }
    }
    return 0;
}

// This creates the ocean trenches.

void createoceantrenches(planet& world, vector<vector<vector<bool>>>& shelves)
{
    int edge = world.edge();
    int maxelev = world.maxelevation();

    int trenchmin = maxelev / 2; // Values in the fractal higher than this will spawn trenches.

    int maxradius = 20;

    int div = (maxelev - trenchmin) / maxradius;

    vector<vector<vector<bool>>> trenchmap(6, vector<vector<bool>>(edge, vector<bool>(edge, 0)));
    vector<vector<vector<int>>> fractal(6, vector<vector<int>>(edge, vector<int>(edge, 0)));

    int grain = 4; // Level of detail on this fractal map.
    float valuemod = 0.01f;
    int v = 1; //random(3,6);
    float valuemod2 = 0.1f;

    createfractalold(fractal, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (shelves[face][i][j] == 1 && fractal[face][i][j] > trenchmin)
                {
                    if (shelfedge(world, shelves, face, i, j) == 1)
                    {
                        int radius = (fractal[face][i][j] - trenchmin) / div;

                        for (int k = -radius; k <= radius; k++)
                        {
                            for (int l = -radius; l <= radius; l++)
                            {
                                if (k * k + l * l < radius * radius + radius)
                                {
                                    globepoint kpoint = getglobepoint(edge, face, i, j, k, l);

                                    if (kpoint.face != -1)
                                        trenchmap[kpoint.face][kpoint.x][kpoint.y] = 1;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Now just add the trenches to the world map.

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (trenchmap[face][i][j] == 1)
                {
                    if (world.sea(face, i, j) == 1 && shelves[face][i][j] == 0 && world.oceanridges(face, i, j) == 0)
                    {
                        int newval = world.nom(face,i, j) - random(4900, 5100);

                        if (newval < 1)
                            newval = 1;

                        world.setnom(face, i, j, newval);
                    }
                }
            }
        }
    }
}

// This makes an isolated volcano, together with a line of extinct neighbours.

void createisolatedvolcano(planet& world, int face, int x, int y, vector<vector<vector<bool>>>& shelves, vector<vector<vector<int>>>& volcanodirection, int peakheight, bool strato)
{
    int edge = world.edge();
    int sealevel = world.sealevel();
    int maxelev = world.maxelevation();

    float maxshift = 6;
    float shiftdiv = maxshift / (maxelev / 2);

    int shelfcheck = 5;

    if (world.sea(face, x, y) == 1) // Don't do any submarine volcanoes near continental shelves.
    {
        for (int i = -shelfcheck; i <= shelfcheck; i++)
        {
            for (int j = -shelfcheck; j <= -shelfcheck; j++)
            {
                globepoint jpoint = getglobepoint(edge, face, x, y, i, j);

                if (jpoint.face != -1)
                {
                    if (shelves[jpoint.face][jpoint.x][jpoint.y] == 1)
                        return;
                }
            }
        }
    }

    bool active = 1;

    int total = random(1, 6);

    for (int n = 1; n != total; n++)
    {
        /*
        if (world.sea(face, x, y) == 1)
        {
            for (int i = x - 1; i <= x + 1; i++) // No shading around undersea volcanoes.
            {
                int ii = i;

                if (ii<0 || ii>width)
                    ii = wrap(ii, width);

                for (int j = y - 1; j <= y + 1; j++)
                {
                    if (j >= 0 && j <= height)
                        world.setnoshade(face, i, j, 1);
                }
            }
        }
        */

        float thispeakheight = (float)peakheight;

        thispeakheight = thispeakheight / 100.0f;

        thispeakheight = thispeakheight * (float)random(80, 120);

        if (active == 0)
            thispeakheight = (float)(0 - peakheight);

        for (int i = -1; i <= 1; i++)
        {
            for (int j = -1; j <= -1; j++)
            {
                globepoint jpoint = getglobepoint(edge, face, x, y, i, j);

                if (jpoint.face != -1)
                    world.setvolcano(jpoint.face, jpoint.x, jpoint.y, 0);

            }
        }

        world.setvolcano(face, x, y, (int)thispeakheight);
        world.setstrato(face, x, y, strato);

        active = 0;

        int xx = x + edge / 2;

        if (xx >= edge)
            xx = xx - edge;

        float xshift = (float)(volcanodirection[face][x][y] - maxelev / 2);
        float yshift = (float)(volcanodirection[face][xx][y] - maxelev / 2);

        xshift = xshift * shiftdiv + (float)randomsign(random(0, 2));
        yshift = yshift * shiftdiv + (float)randomsign(random(0, 2));

        globepoint xshifted = getglobepoint(edge, face, x, y, (int)xshift, 0);

        if (xshifted.face != -1)
        {
            globepoint shifted = getglobepoint(edge, xshifted.face, xshifted.x, xshifted.y, 0, (int)yshift);

            if (shifted.face != -1)
            {
                face = shifted.face;
                x = shifted.x;
                y = shifted.y;

                if (world.sea(face, x, y) == 1) // Don't do any submarine volcanoes near continental shelves.
                {
                    for (int i = -shelfcheck; i <= shelfcheck; i++)
                    {
                        for (int j = -shelfcheck; j <= -shelfcheck; j++)
                        {
                            globepoint jpoint = getglobepoint(edge, face, x, y, i, j);

                            if (jpoint.face != -1)
                            {
                                if (shelves[jpoint.face][jpoint.x][jpoint.y] == 1)
                                    return;
                            }
                        }
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

// This function raises the land beneath mountains.

void raisemountainbases(planet& world, vector<vector<vector<int>>>& mountaindrainage, vector<vector<vector<bool>>>& OKmountains)
{
    int edge = world.edge();
    int sealevel = world.sealevel();
    int maxelev = world.maxelevation();
    float gravity = world.gravity();

    int maxradius = 6;

    float heightreduce = 0.5f; // Multiply the extra height by this each time.
    float extraheightreduce = 0.045f; // Each time, reduce the heightreduce by this.

    if (gravity < 1.0f) // Low gravity means wider areas of higher ground.
    {
        float diff = 1.0f - gravity;
        diff = diff * 10.0f;

        maxradius = maxradius + (int)diff;

        if (maxradius > 12)
            maxradius = 12;

        heightreduce = heightreduce + diff / 10.0f;

        if (heightreduce > 0.8f)
            heightreduce = 0.8f;
    }

    if (gravity > 1.0f) // High gravity means smaller areas of higher ground.
    {
        float diff = gravity - 1.0f;

        maxradius = maxradius - (int)(diff * 10.0f);

        if (maxradius < 2)
            maxradius = 2;

        heightreduce = heightreduce - diff;

        if (heightreduce < 0.2f)
            heightreduce = 0.2f;
    }

    int volcanomaxradius = maxradius / 3;

    // First, adjust the heights of the mountains themselves to take account of gravity.

    float mountainheightfactor = 1.0f;

    if (gravity < 1.0f) // Low gravity means higher mountains.
    {
        float diff = 1.0f - gravity;

        mountainheightfactor = mountainheightfactor + diff * 2.0f;
    }

    if (gravity > 1.0f) // High gravity means lower mountains.
    {
        float diff = gravity - 1.0f;

        mountainheightfactor = mountainheightfactor - diff / 2.0f;

        if (mountainheightfactor < 0.2f)
            mountainheightfactor = 0.2f;
    }

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (OKmountains[face][i][j] == 0) // Don't change the heights of any mountains that the user has imported.
                {
                    float thismountain = (float)world.mountainheight(face, i, j);

                    thismountain = thismountain * mountainheightfactor;

                    world.setmountainheight(face, i, j, (int)thismountain);
                }
            }
        }
    }

    // Adjust the maximum elevation for the world.

    float abovesea = (float)maxelev - (float)sealevel;

    abovesea = abovesea * mountainheightfactor;

    if (abovesea > 20000.0f) // Hard maximum, as weirdness happens above this height.
        abovesea = 20000.0f;

    world.setmaxelevation(sealevel + (int)abovesea);

    // Now do the mountain bases themselves.

    bool volcano = 0;

    float lowrandom = 20.0f;
    float highrandom = 50.0f;

    if (gravity > 1.0f)
    {
        lowrandom = lowrandom - gravity * 2.0f;

        if (lowrandom < 2.0f)
            lowrandom = 2.0f;

        highrandom = lowrandom * 2.0f;
    }

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                volcano = 0;

                float mheights = (float)world.mountainheight(face, i, j);

                if (mheights == 0 && world.strato(face, i, j) == 0)
                {
                    mheights = (float)world.volcano(face, i, j);
                    volcano = 1;
                }

                if (mheights < 0.0f)
                    mheights = 0.0f - mheights;

                if (mheights != 0.0f)
                {
                    heightreduce = 0.7f; //0.5f;

                    float baseheightfraction = (float)random((int)lowrandom, (int)highrandom);

                    if (volcano == 1)
                        baseheightfraction = (float)random((int)lowrandom * 5, (int)highrandom * 4);

                    baseheightfraction = baseheightfraction / 100.0f;
                    float baseheight = mheights * baseheightfraction;

                    int thismaxradius = maxradius;

                    if (volcano == 1)
                        thismaxradius = volcanomaxradius;

                    for (int radius = 1; radius <= thismaxradius; radius++) // Go through ever-increasing circles
                    {
                        for (int k = -radius; k <= radius; k++)
                        {
                            for (int l = -radius; l <= radius; l++)
                            {
                                if (k * k + l * l < radius * radius + radius) // If we're within the current circle
                                {
                                    globepoint kpoint = getglobepoint(edge, face, i, j, k, l);

                                    if (kpoint.face != -1)
                                    {
                                        if (world.sea(kpoint.face, kpoint.x, kpoint.y) == 0 && mountaindrainage[kpoint.face][kpoint.x][kpoint.y] < baseheight)
                                            mountaindrainage[kpoint.face][kpoint.x][kpoint.y] = (int)baseheight;
                                    }
                                }
                            }
                        }

                        baseheight = baseheight * heightreduce;
                    }
                }
            }
        }
    }

    // Now add the extra heights onto the actual map.

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.sea(face, i, j) == 0)
                    world.setnom(face, i, j, world.nom(face, i, j) + mountaindrainage[face][i][j]);
            }
        }
    }
}

// This function smoothes, but without turning any land to sea or vice versa.

void smoothland(planet& world, int amount)
{
    int edge = world.edge();
    int sealevel = world.sealevel();
    float maxelev = (float)world.maxelevation();
    float seaamount = (float)(amount * 3);
    float div = maxelev / seaamount;

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
            int vi = i * edge;

            for (int j = 0; j < edge; j++)
            {
                float crount = 0.0f;
                float ave = 0.0f;
                int origland = 0;
                int thisamount = amount;

                if (world.sea(face,i, j) == 0) // Check to see whether this point is originally land or sea
                    origland = 1;
                else
                    thisamount = (int)((float)fractal[vi + j] / div);

                if (thisamount < 1)
                    thisamount = 1;

                bool goahead = 1;
                int seacheck = 5;

                if (goahead == 1)
                {
                    for (int k = -thisamount; k <= thisamount; k++)
                    {
                        for (int l = -thisamount; l <= thisamount; l++)
                        {
                            globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                            if (lpoint.face != -1)
                            {
                                ave = ave + (float)world.nom(lpoint.face, lpoint.x, lpoint.y);
                                crount++;
                            }
                        }
                    }

                    if (crount > 0.0f)
                    {
                        ave = ave / crount;

                        if (ave > 0 && ave < maxelev)
                        {
                            int newland = 0;

                            if (ave > (float)sealevel) // Now check to see whether the new value is land or sea
                                newland = 1;

                            if (origland == 0 && newland == 0)
                                world.setnom(face,i, j, (int)ave);
                        }
                    }
                }
            }
        }
    }
}

// This function smoothes only the land, varying throughout the map, taking into account gravity.

void smoothonlylandvariable(planet& world, int amount)
{
    int edge = world.edge();
    int sealevel = world.sealevel();
    float maxelev = (float)world.maxelevation();
    float gravity = world.gravity();

    // First create the broad array, so the smoothing will be varied.

    int grain = 4;
    float valuemod = 0.02f;
    float valuemod2 = 0.02f;

    vector<int> broad(6 * edge * edge, 0);

    createfractal(broad, edge, grain, valuemod, valuemod2, 1, (int)maxelev, 0, 0);

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                float crount = 0.0f;
                float ave = 0.0f;
                int origland = 0;
                int thisamount = amount;

                if (world.nom(face, i, j) > sealevel)
                {
                    for (int k = - thisamount; k <= thisamount; k++)
                    {
                        for (int l = -thisamount; l <= thisamount; l++)
                        {
                            globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                            if (lpoint.face != -1 && world.nom(lpoint.face, lpoint.x, lpoint.y) > sealevel)
                            {
                                ave = ave + (float)world.nom(lpoint.face, lpoint.x, lpoint.y);
                                crount++;
                            }
                        }
                    }

                    if (crount > 0.0f)
                    {
                        ave = ave / crount;

                        float newmult = (float)broad[vi + j] / maxelev;
                        newmult = newmult * gravity;

                        if (newmult > 1.0f)
                            newmult = 1.0f;

                        float oldmult = 1.0f - newmult;

                        float newvalue = (float)world.nom(face, i, j) * oldmult + ave * newmult;

                        if (newmult > 0.0f && newmult < maxelev)
                            world.setnom(face, i, j, (int)ave);
                    }
                }
            }
        }
    }
}

// This function creates areas of raised elevation around where canyons might form later.

void createextraelev(planet& world)
{
    int edge = world.edge();
    float gravity = world.gravity();

    vector<int> tempelev(6 * edge * edge, 0);

    float maxextra = 1000.0f; // Maximum amount that the extraelev array can have.

    float maxelev = (float)world.maxelevation();

    float div = maxelev / (maxextra * 2.0f);

    // First create a fractal for this map.

    int grain = 16; // Level of detail on this fractal map.
    float valuemod = 0.2f;
    float valuemod2 = 0.2f;

    createfractal(tempelev, edge, grain, valuemod, valuemod2, 1, (int)maxelev, 0, 0);

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                world.setextraelev(face, i, j, tempelev[index]);
                tempelev[index] = 0;
            }
        }
    }

    createfractal(tempelev, edge, grain, valuemod, valuemod2, 1, (int)maxelev, 0, 0);

    // Now alter it a bit.

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.sea(face,i, j) == 1)
                    world.setextraelev(face,i, j, 0);
                else
                {
                    float e = (float)world.extraelev(face,i, j);
                    e = e / div;
                    e = e - maxextra;

                    if (e < 0.0f)
                        e = 0.0f;

                    if (e > maxextra)
                        e = maxextra;

                    e = e * gravity;

                    world.setextraelev(face,i, j, (int)e);
                }
            }
        }
    }

    // That gives us a *lot* of extra elevation across the map. We want to apply it a bit more judiciously.
    // So we use a second fractal to mask the first, so that fewer areas get extra elevation.

    float multfactor = (float)random(1, 10);
    multfactor = multfactor / 10.0f + 1.0f; // Different worlds will have different amounts of extra elevation.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                if (world.sea(face,i, j) == 0)
                {
                    int index = vi + j;

                    float e = (float)tempelev[index];
                    e = e / div;
                    e = e - maxextra;

                    e = e * multfactor; // The higher this is, the more widespread the extra elevation will be.

                    tempelev[index] = (int)e;

                    if (tempelev[index] < 0)
                        tempelev[index] = 0;

                    if (tempelev[index] < world.extraelev(face,i, j))
                        world.setextraelev(face,i, j, tempelev[index]);
                }
            }
        }
    }

    // Now smooth it

    world.smoothextraelev(2);
    world.smoothextraelev(5);
}

// This function fills in depressions, using my implementation of the Planchon-Darboux algorithm. (Faster algorithms exist, e.g. Wang-Liu and Su-Wang et al., but they're beyond my ability to implement.)

void depressionfill(planet& world)
{
    int edge = world.edge();
    int sealevel = world.sealevel();
    int maxelev = world.maxelevation();

    int e = 2; // This is the extra amount added to ensure that everywhere slopes.

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

    vector<int> noise(6 * edge * edge, 0);

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
                noise[vi + j] = random(0, 5);
        }
    }
    vector<int> filledmap(6 * edge * edge, 0); // This will be the new version of the map.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++) // First, fill the new map to a huge height.
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                if (world.nom(face, i, j) <= sealevel)
                    filledmap[index] = world.nom(face, i, j); // Sea tiles start with the normal heights.
                else
                {
                    if (world.outline(face, i, j) == 1) // If this is a coastal tile
                        filledmap[index] = world.nom(face, i, j); // Coastal tiles start with the normal heights.
                    else
                        filledmap[index] = maxelev * 2; // Other tiles start very high

                }
            }
        }
    }

    // Now we're ready to start!

    globepoint lowest;

    lowest.face = 0;
    lowest.x = 0;
    lowest.y = 0;

    bool somethingdone = 0;

    do
    {
        for (int face = 0; face < 6; face++)
        {
            int vface = face * edge * edge;

            for (int i = 0; i < edge; i++) // Eight mini-loops, because there are eight possible ways to scan the map and we must rotate between them.
            {
                int vi = vface + i * edge;

                for (int j = 0; j < edge; j++)
                {
                    if (filledmap[vi + j] > world.nom(face, i, j))
                        somethingdone = checkdepressiontile(world, filledmap, face, i, j, e, neighbours, somethingdone, noise);
                }
            }
        }

        if (somethingdone == 1)
        {
            somethingdone = 0;

            for (int face = 0; face < 6; face++)
            {
                int vface = face * edge * edge;

                for (int j = edge - 1; j >= 0; j--)
                {
                    for (int i = edge - 1; i >= 0; i--)
                    {
                        if (filledmap[vface + i * edge + j] > world.nom(face, i, j))
                            somethingdone = checkdepressiontile(world, filledmap, face, i, j, e, neighbours, somethingdone, noise);

                    }
                }
            }
        }

        if (somethingdone == 1)
        {
            somethingdone = 0;

            for (int face = 0; face < 6; face++)
            {
                int vface = face * edge * edge;

                for (int i = edge - 1; i >= 0; i--)
                {
                    int vi = vface + i * edge;

                    for (int j = 0; j < edge; j++)
                    {
                        if (filledmap[vi + j] > world.nom(face, i, j))
                            somethingdone = checkdepressiontile(world, filledmap, face, i, j, e, neighbours, somethingdone, noise);

                    }
                }
            }
        }

        if (somethingdone == 1)
        {
            somethingdone = 0;

            for (int face = 0; face < 6; face++)
            {
                int vface = face * edge * edge;

                for (int j = 0; j < edge; j++)
                {
                    for (int i = edge - 1; i >= 0; i--)
                    {
                        if (filledmap[vface + i * edge + j] > world.nom(face, i, j))
                            somethingdone = checkdepressiontile(world, filledmap, face, i, j, e, neighbours, somethingdone, noise);

                    }
                }
            }
        }

        if (somethingdone == 1)
        {
            somethingdone = 0;

            for (int face = 0; face < 6; face++)
            {
                int vface = face * edge * edge;

                for (int i = 0; i < edge; i++)
                {
                    int vi = vface + i * edge;

                    for (int j = edge - 1; j >= 0; j--)
                    {
                        if (filledmap[vi + j] > world.nom(face, i, j))
                            somethingdone = checkdepressiontile(world, filledmap, face, i, j, e, neighbours, somethingdone, noise);

                    }
                }
            }
        }

        if (somethingdone == 1)
        {
            somethingdone = 0;

            for (int face = 0; face < 6; face++)
            {
                int vface = face * edge * edge;

                for (int j = edge - 1; j >= 0; j--)
                {
                    for (int i = 0; i < edge; i++)
                    {
                        if (filledmap[vface + i * edge + j] > world.nom(face, i, j))
                            somethingdone = checkdepressiontile(world, filledmap, face, i, j, e, neighbours, somethingdone, noise);

                    }
                }
            }
        }

        if (somethingdone == 1)
        {
            somethingdone = 0;

            for (int face = 0; face < 6; face++)
            {
                int vface = face * edge * edge;

                for (int i = edge - 1; i > 0; i--)
                {
                    int vi = vface + i * edge;

                    for (int j = edge - 1; j >= 0; j--)
                    {
                        if (filledmap[vi + j] > world.nom(face, i, j))
                            somethingdone = checkdepressiontile(world, filledmap, face, i, j, e, neighbours, somethingdone, noise);

                    }
                }
            }
        }

        if (somethingdone == 1)
        {
            somethingdone = 0;

            for (int face = 0; face < 6; face++)
            {
                int vface = face * edge * edge;

                for (int j = 0; j < edge; j++)
                {
                    for (int i = 0; i < edge; i++)
                    {
                        if (filledmap[vface + i * edge + j] > world.nom(face, i, j))
                            somethingdone = checkdepressiontile(world, filledmap, face, i, j, e, neighbours, somethingdone, noise);

                    }
                }
            }
        }
    } while (somethingdone == 1);

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
                world.setnom(face, i, j, filledmap[vi + j]);

        }
    }

    checkdepressionedges(world);
}

// This does the actual checking/filling of each tile.

bool checkdepressiontile(planet& world, vector<int>& filledmap, int face, int i, int j, int e, int neighbours[8][2], bool somethingdone, vector<int>& noise)
{
    int edge = world.edge();

    int start = random(0, 7);

    int index = face * edge * edge + i * edge + j;

    for (int n = start; n <= start + 7; n++)
    {
        int nn = wrap(n, 7);

        globepoint lpoint = getglobepoint(edge, face, i, j, neighbours[nn][0], neighbours[nn][1]);

        if (lpoint.face != -1)
        {
            int lindex = lpoint.face * edge * edge + lpoint.x * edge + lpoint.y;

            int ee = 0;

            if (lpoint.x == i || lpoint.y == j)
                ee = e + 1 + noise[index];

            else
                ee = e + noise[index];

            if (world.nom(face, i, j) >= filledmap[lindex] + ee)
            {
                filledmap[index] = world.nom(face, i, j);
                somethingdone = 1;
            }
            else
            {
                if (filledmap[index] > filledmap[lindex] + ee)
                {
                    filledmap[index] = filledmap[lindex] + ee;
                    somethingdone = 1;
                }
            }
        }
    }

    /*
    if (i == 0 || i == edge - 1 || j == 0 || j == edge - 1) // To ensure that it wraps over face edges properly.
    {
        start = random(0, 7);

        for (int n = start; n <= start + 7; n++)
        {
            int nn = wrap(n, 7);

            globepoint kpoint = getglobepoint(edge, face, i, j, neighbours[nn][0] * 2, 0);

            if (kpoint.face != -1)
            {
                globepoint lpoint = getglobepoint(edge, kpoint.face, kpoint.x, kpoint.y, 0, neighbours[nn][1] * 2);

                if (lpoint.face != -1)
                {
                    int ee = 0;

                    if (lpoint.x == i || lpoint.y == j)
                        ee = e + 1 + noise[face][i][j];

                    else
                        ee = e + noise[face][i][j];

                    if (world.nom(face, i, j) >= filledmap[lpoint.face][lpoint.x][lpoint.y] + ee)
                    {
                        filledmap[face][i][j] = world.nom(face, i, j);
                        somethingdone = 1;
                    }
                    else
                    {
                        if (filledmap[face][i][j] > filledmap[lpoint.face][lpoint.x][lpoint.y] + ee)
                        {
                            filledmap[face][i][j] = filledmap[lpoint.face][lpoint.x][lpoint.y] + ee;
                            somethingdone = 1;
                        }
                    }
                }
            }
        }
    }
    */

    return (somethingdone);
}

// This ensures that the edges of faces have properly low elevation.

void checkdepressionedges(planet& world)
{
    int edge = world.edge();

    for (int face = 0; face < 4; face++)
    {
        for (int n = 0; n < edge; n++)
        {
            int thisnom = world.nom(face, n, 0);
            int thatnom = world.nom(face, n, 1);

            if (thisnom > thatnom)
                world.setnom(face, n, 0, thatnom);

            thisnom = world.nom(face, n, edge - 1);
            thatnom = world.nom(face, n, edge - 2);

            if (thisnom > thatnom)
                world.setnom(face, n, edge - 1, thatnom);
        }
    }

    for (int face = 4; face < 6; face++)
    {
        for (int n = 0; n < edge; n++)
        {
            int thisnom = world.nom(face, n, 0);
            int thatnom = world.nom(face, n, 1);

            if (thisnom > thatnom)
                world.setnom(face, n, 0, thatnom);

            thisnom = world.nom(face, n, edge - 1);
            thatnom = world.nom(face, n, edge - 2);

            if (thisnom > thatnom)
                world.setnom(face, n, edge - 1, thatnom);

            thisnom = world.nom(face, 0, n);
            thatnom = world.nom(face, 1, n);

            if (thisnom > thatnom)
                world.setnom(face, 0, n, thatnom);

            thisnom = world.nom(face, edge - 1, n);
            thatnom = world.nom(face, edge - 2, n);

            if (thisnom > thatnom)
                world.setnom(face, edge - 1, n, thatnom);
        }
    }
}

// This function adds a bit of noise to the land.

void addlandnoise(planet& world)
{
    int edge = world.edge();
    int sealevel = world.sealevel();
    int maxelev = world.maxelevation();

    int maxadjust = 10;
    int adjustchance = 5; // The lower this is, the more there will be.

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.nom(face, i, j) > sealevel && random(1, adjustchance) == 1)
                {
                    int adjust = randomsign(random(1, maxadjust));

                    int newamount = world.nom(face, i, j) + adjust;

                    if (newamount <= sealevel)
                        newamount = sealevel;

                    if (newamount > maxelev)
                        newamount = maxelev;

                    world.setnom(face, i, j, newamount);
                }
            }
        }
    }
}

// This function forces land and sea at coastlines to normalise towards certain values, to improve the appearance of the coastline. Note that landheight and seadepth are relative to the sea level, not absolute.

void normalisecoasts(planet& world, int landheight, int seadepth, int severity)
{
    int edge = world.edge();
    int sealevel = world.sealevel();

    int depth, elev;

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.volcano(face, i, j) == 0 && world.mountainisland(face, i, j) == 0)
                {
                    if (world.coast(face, i, j))
                    {
                        depth = sealevel - world.nom(face, i, j);
                        int newdepth = (depth + seadepth * severity) / (severity + 1);

                        world.setnom(face, i, j, sealevel - newdepth);
                    }

                    if (world.outline(face, i, j))
                    {
                        elev = world.nom(face, i, j) - sealevel;
                        int newelev = (elev + landheight * severity) / (severity + 1);

                        world.setnom(face, i, j, sealevel + newelev);
                    }
                }
            }
        }
    }

    // Now we need to smooth the seabeds around them.

    vector<bool> done(6 * edge * edge, 0);

    int amount = 2;
    int amount2 = 2;

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.coast(face, i, j) == 1)
                {
                    for (int k = -amount; k <= amount; k++)
                    {
                        for (int l = -amount; l <= -amount; l++)
                        {
                            globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                            if (lpoint.face != -1)
                            {
                                int lindex = lpoint.face * edge * edge + lpoint.x * edge + lpoint.y;

                                if (done[lindex] == 0 && world.sea(lpoint.face, lpoint.x, lpoint.y) == 1 && world.coast(lpoint.face, lpoint.x, lpoint.y) == 0)
                                {
                                    int total = 0;
                                    int crount = 0;

                                    for (int m = -amount2; m <= -amount2; m++)
                                    {
                                        globepoint mpoint = getglobepoint(edge, lpoint.face, lpoint.x, lpoint.y, m, 0);

                                        if (mpoint.face != -1)
                                        {
                                            for (int n = -amount2; n <= amount2; n++)
                                            {
                                                globepoint npoint = getglobepoint(edge, mpoint.face, mpoint.x, mpoint.y, 0, n);

                                                if (npoint.face != -1 && world.sea(npoint.face, npoint.x, npoint.y) == 1)
                                                {
                                                    crount++;
                                                    total = total + world.nom(npoint.face, npoint.x, npoint.y);
                                                }
                                            }
                                        }
                                    }

                                    if (total != 0)
                                    {
                                        int newnom = total / crount;
                                        world.setnom(lpoint.face, lpoint.x, lpoint.y, newnom);
                                        done[lindex] = 1;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Do it again, going the other way.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
                done[vi + j] = 0;
        }
    }

    for (int face = 0; face < 6; face++)
    {
        for (int i = edge - 1; i >= 0; i++)
        {
            for (int j = edge - 1; j >= 0; j++)
            {
                if (world.coast(face, i, j) == 1)
                {
                    for (int k = -amount; k <= amount; k++)
                    {
                        for (int l = -amount; l <= -amount; l++)
                        {
                            globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                            if (lpoint.face != -1)
                            {
                                int lindex = lpoint.face * edge * edge + lpoint.x * edge + lpoint.y;

                                if (done[lindex] == 0 && world.sea(lpoint.face, lpoint.x, lpoint.y) == 1 && world.coast(lpoint.face, lpoint.x, lpoint.y) == 0)
                                {
                                    int total = 0;
                                    int crount = 0;

                                    for (int m = -amount2; m <= -amount2; m++)
                                    {
                                        globepoint mpoint = getglobepoint(edge, lpoint.face, lpoint.x, lpoint.y, m, 0);

                                        if (mpoint.face != -1)
                                        {
                                            for (int n = -amount2; n <= amount2; n++)
                                            {
                                                globepoint npoint = getglobepoint(edge, mpoint.face, mpoint.x, mpoint.y, 0, n);

                                                if (npoint.face != -1 && world.sea(npoint.face, npoint.x, npoint.y) == 1)
                                                {
                                                    crount++;
                                                    total = total + world.nom(npoint.face, npoint.x, npoint.y);
                                                }
                                            }
                                        }
                                    }

                                    if (total != 0)
                                    {
                                        int newnom = total / crount;
                                        world.setnom(lpoint.face, lpoint.x, lpoint.y, newnom);
                                        done[lindex] = 1;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// This function ensures that all values in the map are in the correct range.

void clamp(planet& world)
{
    int edge = world.edge();
    int maxelev = world.maxelevation();
    int sealevel = world.sealevel();

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.nom(face,i, j) > maxelev)
                    world.setnom(face,i, j, maxelev);

                if (world.nom(face,i, j) < 1)
                    world.setnom(face,i, j, 1);

                if (world.mountainheight(face,i, j) > 0)
                {
                    int map = world.map(face,i, j);

                    if (map > maxelev)
                    {
                        int newheight = maxelev - map - world.extraelev(face,i, j);

                        world.setmountainheight(face,i, j, newheight);
                    }
                }
            }
        }
    }
}

// This notes down all the very small islands on the map.

void checkislands(planet& world)
{
    int edge = world.edge();
    int sealevel = world.sealevel();

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.sea(face,i, j) == 0)
                {
                    int crount = -1; // Because there will definitely be one positive, i.e. the current cell.

                    for (int k = - 1; k <= 1; k++)
                    {
                        for (int l = -1; l <= 1; l++)
                        {
                            globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                            if (lpoint.face != -1)
                            {
                                if (world.sea(lpoint.face, lpoint.x, lpoint.y) == 0)
                                    crount++;
                            }
                        }
                    }

                    if (crount == 0) // This is a one-tile island!
                        world.setisland(face,i, j, 1);
                }
                else
                {
                    int volcano = world.volcano(face,i, j);

                    if (volcano != 0)
                    {
                        bool extinct = 0;

                        if (volcano < 0)
                        {
                            extinct = 1;
                            volcano = 0 - volcano;
                        }

                        int totalheight = world.nom(face,i, j) + volcano;

                        if (totalheight > sealevel)
                        {
                            volcano = totalheight - sealevel;

                            makemountainisland(world, face, i, j, volcano);

                            if (extinct == 1)
                                volcano = 0 - volcano;

                            world.setvolcano(face,i, j, volcano);
                        }
                    }
                }
            }
        }
    }
}

// This makes a small, mountainous island.

void makemountainisland(planet& world, int face, int x, int y, int peakheight)
{
    int edge = world.edge();
    int sealevel = world.sealevel();

    int iterations = random(1, 3);

    for (int n = 1; n <= iterations; n++)
    {
        int landheight = sealevel + random(5, 50);

        int xx = x;
        int yy = y;

        do
        {
            xx = x - 2 + random(1, 3);
            yy = y - 2 + random(1, 3);

        } while (x == xx && y == yy);

        if (xx < 0 || xx >= edge)
            return;

        if (yy < 0 || yy >= edge)
            return;

        if (world.mountainheight(face,x, y) < peakheight)
            world.setmountainheight(face,x, y, peakheight);

        if (world.mountainheight(face,xx, yy) < peakheight)
            world.setmountainheight(face,xx, yy, peakheight);

        for (int i = x - 1; i <= x + 1; i++) // Mark it all out as mountain islands!
        {
            if (i >= 0 && i < edge)
            {
                for (int j = y - 1; j <= y + 1; j++)
                {
                    if (j >= 0 && j < edge)
                        world.setmountainisland(face,i, j, 1);
                }
            }
        }

        for (int i = xx - 1; i <= xx + 1; i++)
        {
            if (i >= 0 && i < edge)
            {
                for (int j = yy - 1; j <= yy + 1; j++)
                {
                    if (j >= 0 && j < edge)
                        world.setmountainisland(face,i, j, 1);
                }
            }
        }

        if (world.nom(face, x, y) < landheight)
            world.setnom(face, x, y, landheight);

        if (world.nom(face, xx, yy) < landheight)
            world.setnom(face, xx, yy, landheight);

        // If there isn't a ridge between them, make one.

        int dir = getdir(x, y, xx, yy);

        if (getridge(world, face, x, y, dir) == 0)
        {
            int code = getcode(dir);

            world.setmountainridge(face,x, y, world.mountainridge(face,x, y) + code);

            dir = dir + 8;

            if (dir > 8)
                dir = dir - 8;

            code = getcode(dir);

            world.setmountainridge(face,xx, yy, world.mountainridge(face,xx, yy) + code);
        }

        x = xx;
        y = yy;

        peakheight = peakheight + randomsign(random(peakheight / 4, peakheight / 2));
    }
}

// This removes odd bumps on the sea floor.

void removeunderseabumps(planet& world)
{
    int edge = world.edge();
    int sealevel = world.sealevel();

    int maxdiff = 100; // If the cell being looked at is more than the average of its neighbours plus this, it will be lowered.

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.sea(face, i, j) == 1 && world.oceanridges(face, i, j) == 0)
                {
                    float total = 0.0f;
                    float crount = 0.0f;

                    for (int k = - 1; k <= 1; k++)
                    {
                        for (int l = -1; l <= 1; l++)
                        {
                            globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                            if (lpoint.face != -1 && (lpoint.x != i || lpoint.y != j) && world.sea(lpoint.face, lpoint.x, lpoint.y) == 1)
                            {
                                total = total + (float)world.nom(lpoint.face, lpoint.x, lpoint.y);
                                crount++;
                            }
                        }
                    }

                    if (crount > 0)
                    {
                        int ave = (int)(total / crount);

                        if (world.nom(face, i, j) > ave + maxdiff)
                            world.setnom(face, i, j, ave);
                    }
                }
            }
        }
    }

    for (int n = 0; n < 6; n++)
    {
        for (int face = 0; face < 6; face++)
        {
            for (int n = 0; n < edge; n++)
            {
                if (world.sea(face, n, 0) && world.sea(face, n, 1))
                    world.setnom(face, n, 0, world.nom(face, n, 1));

                if (world.sea(face, 0, n) && world.sea(face, 1, n))
                        world.setnom(face, 0, n, world.nom(face, 1, n));
            }
        }
    }
}

// This function adds mountain ridges over the sea next to coastal mountains in glaciated regions (to make fjords) and elsewhere (to make rocky peninsulas). It's actually called in globalclimate.cpp but it's here because it's terrain really.

void addfjordmountains(planet& world)
{
    int edge = world.edge();
    int maxelev = world.maxelevation();
    int glacialtemp = world.glacialtemp();
    int glacialmountainheight = 20; //200; // In glacial regions, mountains this height or more are guaranteed to spawn fjords.

    int minmountheight = 0; // Ignore mountains lower than this.
    int noglacchance = 8; //3; //15; // Chance of making sea mountain ridges in non-glaciated areas (to add occasional rocky peninsulas).

    vector<int> justadded(6 * edge * edge, 0);
    vector<int> previouslyadded(6 * edge * edge, 0);

    for (int n = 1; n <= 2; n++)
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

                    if (world.sea(face, i, j)) // && random(1,maxelev)<world.roughness(face, i,j))
                    {
                        int avetemp = (world.mintemp(face, i, j) + world.maxtemp(face, i, j)) / 2;

                        bool fjordreason1 = 0;
                        bool fjordreason2 = 0;

                        if (avetemp < glacialtemp)
                            fjordreason1 = 1; // Because it's cold enough. But this will depend upon the mountains being high enough.

                        if (random(1, noglacchance) == 1 && random(1, maxelev) < (int)world.roughness(face, i, j))
                            fjordreason2 = 1; // Because it's just one of those ones that appear throughout the map.

                        if (fjordreason1 == 1 || fjordreason2 == 1)
                        {
                            int nexttoland = 0;

                            int roughquotient = random(1, maxelev);

                            for (int k = - 1; k <= 1; k++)
                            {
                                for (int l = -1; l <= 1; l++)
                                {
                                    globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                                    if (lpoint.face != -1)
                                    {
                                        if (n == 1 && world.sea(lpoint.face, lpoint.x, lpoint.y) == 0 && world.mountainisland(lpoint.face, lpoint.x, lpoint.y) == 0) // Don't allow these from mountain islands, as that causes weirdness.
                                        {
                                            nexttoland = 1;
                                            k = 1;
                                            l = 1;
                                        }

                                        if (n != 1)
                                        {
                                            if (previouslyadded[lpoint.face * edge * edge + lpoint.x * edge + lpoint.y] == 1)
                                            {
                                                int roughness = (int)world.roughness(face, i, j);

                                                if (avetemp < glacialtemp)
                                                    roughness = roughness * 5; // This is more likely in glacial areas.

                                                if (roughquotient < roughness) // The rougher the area, the more likely this is
                                                {
                                                    nexttoland = 1;
                                                    k = 1;
                                                    l = 1;
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            if (nexttoland == 1)
                            {
                                int highest = 0;

                                globepoint land;
                                land.face = -1;

                                for (int k = 1; k <= 1; k++) // Find the highest mountain nearby, if there is one
                                {
                                    for (int l = -1; l <= 1; l++)
                                    {
                                        globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                                        if (lpoint.face != -1)
                                        {
                                            if (world.mountainheight(lpoint.face, lpoint.x, lpoint.y) > highest && justadded[lpoint.face * edge * edge + lpoint.x * edge + lpoint.y] == 0)
                                            {
                                                highest = world.mountainheight(lpoint.face, lpoint.x, lpoint.y);

                                                land.face = lpoint.face;
                                                land.x = lpoint.x;
                                                land.y = lpoint.y;
                                            }
                                        }
                                    }
                                }

                                if (fjordreason1 == 1 && fjordreason2 == 0 && highest < glacialmountainheight) // If these are glacial mountain fjords, the mountains have to be high enough.
                                    land.face = -1;

                                if (land.face != -1 && highest >= minmountheight) // If we found a nearby mountain, add a ridge to it.
                                {
                                    int dir1 = getdir(edge, face, i, j, land.face, land.x, land.y);
                                    int dir2 = getdir(edge, land.face, land.x, land.y, face, i, j);

                                    int code1 = getcode(dir1);
                                    int code2 = getcode(dir2);

                                    world.setmountainridge(face, i, j, world.mountainridge(face, i, j) + code1);
                                    world.setmountainridge(land.face, land.x, land.y, world.mountainridge(land.face, land.x, land.y) + code2);

                                    world.setmountainheight(face, i, j, world.mountainheight(land.face, land.x, land.y));

                                    world.setsummerrain(face, i, j, world.summerrain(land.face, land.x, land.y));
                                    world.setwinterrain(face, i, j, world.winterrain(land.face, land.x, land.y));

                                    justadded[index] = 1;

                                    //if (world.mountainheight(face, i,j)!=0)
                                    //world.settest(face, i,j,world.mountainheight(face, i,j));
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

                    if (previouslyadded[index] == 0)
                        previouslyadded[index] = justadded[index];

                    justadded[index] = 0;
                }
            }
        }
    }
}

// This finds the direction from one point to another, for mountain ranges.

short getmountaindir(planet& world, int startx, int starty, int endx, int endy)
{
    int edge = world.edge();

    // First, work out whether we need to do any wrapping.

    int xdist = startx - endx;
    int ydist = starty - endy;

    int dist = xdist * xdist + ydist * ydist;

    int leftxdist = (startx + edge) - endx;

    int leftdist = leftxdist * leftxdist + ydist * ydist;

    int rightxdist = startx - (endx + edge);

    int rightdist = rightxdist * rightxdist + ydist * ydist;

    if (leftdist < dist)
    {
        dist = leftdist;
        startx = startx + edge;
    }

    if (rightdist < dist)
    {
        dist = rightdist;
        endx = endx + edge;
    }

    short dir = 0;

    xdist = abs(startx - endx);
    ydist = abs(starty - endy);

    if (endx > startx && endy > starty) // Roughly southeast
    {
        if (xdist > ydist)
            dir = 2;

        else
            dir = 3;
    }

    if (endx > startx && endy <= starty) // Roughly northeast
    {
        if (xdist > ydist)
            dir = 1;

        else
            dir = 8;
    }

    if (endx <= startx && endy > starty) // Roughly southwest
    {
        if (xdist > ydist)
            dir = 5;

        else
            dir = 4;
    }

    if (endx <= startx && endy <= starty) // Roughly northwest
    {
        if (xdist > ydist)
            dir = 6;

        else
            dir = 7;
    }

    return dir;
}

// This converts any oceanic ridges sticking out of the sea into mountains.

void ridgestomountains(planet& world)
{
    int edge = world.edge();
    int sealevel = world.sealevel();

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                int ridgeheight = world.oceanridgeheights(face, i, j);
                
                if (ridgeheight != 0)
                {
                    int nom = world.nom(face, i, j);

                    if (nom + ridgeheight > sealevel)
                    {
                        int totalheight = nom + ridgeheight;

                        int newnom = sealevel + 1;
                        int newheight = totalheight - newnom;

                        world.setnom(face, i, j, newnom);
                        world.setmountainheight(face, i, j, newheight);
                        world.setmountainridge(face, i, j, world.oceanridges(face, i, j));

                        world.setoceanridgeheights(face, i, j, 0);
                        world.setoceanridges(face, i, j, 0);
                    }
                }
            }
        }
    }

}

// This makes a basic fractal terrain with warping and peak exaggeration (for terrain type 4).

void makebasicterrain(planet& world, float landscale, vector<vector<vector<int>>>& terrain, vector<fourglobepoints>& dirpoint, boolshapetemplate landshape[], bool first)
{
    int edge = world.edge();
    int size = world.size();
    int maxelev = world.maxelevation();
    int sealevel = world.sealevel();

    int monstrouschance = 6; // Chance of using monstrous terrain. Lower = more likely.
    int voronoichance = 20; // Chance of using a voronoi map instead of a fractal.
    int secondwarpchance = 2; // Chance of doing a second warp.
    int peakschance = 2; // Chance of exaggerating peaks/troughs.

    int warpfactor = random(60, 120); // Amount to warp the terrain.

    if (random(1, voronoichance) == 1)
    {
        vector<vector<vector<int>>> voronoi(6, vector<vector<int>>(edge, vector<int>(edge, 0)));

        int points = random(10, 30);

        if (size == 1)
            points = points * 4;

        if (size == 2)
            points = points * 16;

        makevoronoi(voronoi, edge, points);

        int aveelev = random(1, maxelev);

        int lowestelev = random(1, aveelev);
        int highestelev = random(aveelev, maxelev);

        int span = highestelev - lowestelev;

        int step = span / points;

        for (int face = 0; face < 6; face++)
        {
            for (int i = 0; i < edge; i++)
            {
                for (int j = 0; j < edge; j++)
                {
                    int elev = lowestelev + (int)voronoi[face][i][j] * step;

                    if (elev > maxelev)
                        elev = maxelev;

                    terrain[face][i][j] = elev;
                }
            }
        }
    }
    else
    {
        int grain = 8; // Level of detail on this fractal map.
        float valuemod = 0.2f;
        float valuemod2 = 3.0f;

        createfractalold(terrain, edge, grain, valuemod, valuemod2, 1, maxelev, 1, 0);

        warpfactor = warpfactor * 2;
    }

    // Create a broad template fractal, to force large areas of low/high terrain.

    int grain = 2; // Level of detail on this fractal map.
    float valuemod = 0.01f; // 0.002f;
    float valuemod2 = 0.01f; // 0.002f;

    vector<vector<vector<int>>> broad(6, vector<vector<int>>(edge, vector<int>(edge, 0)));

    createfractalold(broad, edge, grain, valuemod, valuemod2, 1, maxelev, 1, 0);

    float broadmin = 0.2f; // To ensure that it doesn't create swathes of zero elevation!

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                float val1 = (float)terrain[face][i][j] / (float)maxelev;

                float val2 = (float)broad[face][i][j] / (float)maxelev;

                if (val2 < broadmin)
                    val2 = broadmin;

                float newval = val1 * val2;
                newval = newval * (float)maxelev;

                terrain[face][i][j] = (int)newval;
            }
        }
    }

    // Warp it.

    if (random(1, monstrouschance) == 1)
        monstrouswarpold(terrain, edge, maxelev, random(20, edge - 1));
    else
        warpold(terrain, edge, maxelev, warpfactor, warpfactor, 8, 1, dirpoint);

    // And again.

    if (random(1, secondwarpchance) == 1)
        warpold(terrain, edge, maxelev, warpfactor / 2, warpfactor / 2, 8, 1, dirpoint);

    // Now exaggerate peaks/troughs.

    if (random(1, 2) == peakschance)
    {
        for (int face = 0; face < 6; face++)
        {
            for (int i = 0; i < edge; i++) // Create a new fractal to vary the effect throughout the map.
            {
                for (int j = 0; j < edge; j++)
                    broad[face][i][j] = 0;
            }
        }

        grain = 4;
        valuemod = 0.01f;
        valuemod2 = 0.01f;
        createfractalold(broad, edge, grain, valuemod, valuemod2, 1, maxelev, 1, 0);

        int change = random(25, 200); // Now multiply that whole fractal by a random amount, so effectively the whole effect will be multiplied by this amount.
        float fchange = (float)change / 100.0f;

        for (int face = 0; face < 6; face++)
        {
            for (int i = 0; i < edge; i++)
            {
                for (int j = 0; j < edge; j++)
                {
                    float val = (float)broad[face][i][j];
                    val = val * fchange;

                    if (val > (float)maxelev)
                        val = (float)maxelev;

                    broad[face][i][j] = (int)val;
                }
            }
        }

        int highest = 0;

        for (int face = 0; face < 6; face++)
        {
            for (int i = 0; i < edge; i++) // Find the highest value in our terrain.
            {
                for (int j = 0; j < edge; j++)
                {
                    if (terrain[face][i][j] > highest)
                        highest = terrain[face][i][j];
                }
            }
        }

        if (highest < sealevel)
            return;

        highest = highest - sealevel;

        for (int face = 0; face < 6; face++)
        {
            for (int i = 0; i < edge; i++) // Now do the actual peaks and troughs.
            {
                for (int j = 0; j < edge; j++)
                {
                    if (terrain[face][i][j] > sealevel)
                    {
                        float proportionchange = (float)broad[face][i][j]; // This determines how much of the effect we will have here.
                        float proportionremain = (float)maxelev - proportionchange;

                        float oldelev = (float)terrain[face][i][j];

                        float newelev = oldelev - (float)sealevel;

                        newelev = newelev / (float)highest;

                        newelev = newelev * newelev;

                        newelev = newelev * (float)highest;

                        newelev = newelev + (float)sealevel;

                        float finalelev = ((float)terrain[face][i][j] * proportionremain + newelev * proportionchange) / (float)maxelev;

                        terrain[face][i][j] = (int)finalelev;
                    }
                }
            }
        }
    }

    // Now scale down everything above sea level.

    scaledownland((float)sealevel, (float)maxelev, landscale, edge, terrain);
}

// This function scales down (or up!) everything on an array above a certain height.

void scaledownland(float scaleelev, float maxelev, float landscale, int edge, vector<vector<vector<int>>>& terrain)
{
    float maxscaleelev = maxelev - scaleelev; // This is the maximum amount of elevation above the cutoff point.

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (terrain[face][i][j] > (int)scaleelev)
                {                    
                    float thiselev = (float)terrain[face][i][j] - scaleelev;

                    thiselev = thiselev  * landscale;

                    thiselev = scaleelev + thiselev;

                    if (thiselev > maxelev)
                        thiselev = maxelev;

                    terrain[face][i][j] = (int)thiselev;
                }
            }
        }
    }
}

// Remove line artefacts at face boundaries.

void removelinesatedges(planet& world)
{
    int edge = world.edge();

    int maxvariation = 50;

    for (int face = 0; face < 6; face++)
    {
        for (int n = 0; n < edge; n++)
        {
            int thiselev = world.nom(face, n, 0);

            globepoint ypoint = getglobepoint(edge, face, n, 0, 0, -1);

            int thatelev = world.nom(ypoint.face, ypoint.x, ypoint.y);

            if (thiselev - thatelev > maxvariation || thatelev - thiselev > maxvariation)
                world.setnom(face, n, 0, thatelev);

            thiselev = world.nom(face, 0, n);

            globepoint xpoint = getglobepoint(edge, face, 0, n, -1, 0);

            thatelev = world.nom(xpoint.face, xpoint.x, xpoint.y);

            if (thiselev - thatelev > maxvariation || thatelev - thiselev > maxvariation)
                world.setnom(face, 0, n, thatelev);


            thiselev = world.nom(face, n, edge - 1);

            ypoint = getglobepoint(edge, face, n, edge - 1, 0, 2);

            thatelev = world.nom(ypoint.face, ypoint.x, ypoint.y);

            if (thiselev - thatelev > maxvariation || thatelev - thiselev > maxvariation)
                world.setnom(face, n, edge - 1, thatelev);

            thiselev = world.nom(face, edge - 1, n);

            xpoint = getglobepoint(edge, face, edge - 1, n, 2, 0);

            thatelev = world.nom(xpoint.face, xpoint.x, xpoint.y);

            if (thiselev - thatelev > maxvariation || thatelev - thiselev > maxvariation)
                world.setnom(face, edge - 1, n, thatelev);
        }
    }
}

// This function creates craters.

void createcratermap(planet& world, int cratertotal, vector<int>& squareroot, bool custom)
{
    int edge = world.edge();
    int maxelev = world.maxelevation();

    int margin = edge / 4; // Craters can't be closer to face edges than this, because they would be too distorted.

    if (world.size() == 0)
        cratertotal = cratertotal / 4;

    if (world.size() == 2)
        cratertotal = cratertotal * 4;

    int oldcraterno = world.craterno(); // Because this may not be the first time we've done this for this world (e.g. the "craters" button is clicked multiple times in the custom world screen).

    if (oldcraterno == MAXCRATERS)
        return;

    cratertotal = cratertotal + oldcraterno;

    if (cratertotal > MAXCRATERS)
        cratertotal = MAXCRATERS;

    world.setcraterno(cratertotal);

    int checktype = 0; // What additional kinds of checks there might be on crater placement.

    if (custom == 0) // If we haven't come here from the custom world page, we might restrict where craters can appear.
    {
        if (random(1, 10) == 1) // Based on slopes.
            checktype = 1;

        if (random(1, 4) != 1) // Based on elevation (high).
            checktype = 2;

        if (random(1, 8) == 1) // Based on elevation (low).
            checktype = 3;
    }

    int totalsea = 0;

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.sea(face, i, j))
                    totalsea++;
            }
        }
    }

    int lowestelev = maxelev;
    int highestelev = 0;

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

    int boundary = random(lowestelev, highestelev);

    // Create a broad fractal, to vary the density of craters.

    int grain = 2; // Level of detail on this fractal map.
    float valuemod = 0.002f;
    float valuemod2 = 0.002f;

    vector<int> broad(6 * edge * edge, 0);

    createfractal(broad, edge, grain, valuemod, valuemod2, 1, maxelev, 1, 0);

    for (int craterno = oldcraterno; craterno < cratertotal; craterno++)
    {
        int thisface = random(0, 5);
        int thisx = random(margin, edge - margin);
        int thisy = random(margin, edge - margin);

        if (random(1, 3) == 1)
        {
            thisx = random(margin / 2, edge - margin / 2);
            thisy = random(margin / 2, edge - margin / 2);

            if (random(1, 3) == 1)
            {
                thisx = random(margin / 4, edge - margin / 4);
                thisy = random(margin / 4, edge - margin / 4);
            }
        }

        if (random(1, maxelev) < broad[thisface * edge * edge + thisx * edge + thisy])
        {
            bool makethiscrater = 0;

            int thiselev = world.nom(thisface, thisx, thisy);

            if (checktype == 0)
                makethiscrater = 1;

            if (checktype == 1)
            {
                int slope = 0;

                for (int i = -1; i <= 1; i++)
                {
                    for (int j = -1; j <= 1; j++)
                    {
                        globepoint jpoint = getglobepoint(edge, thisface, thisx, thisy, i, j);

                        if (jpoint.face != -1)
                        {
                            int thisslope = thiselev - world.nom(jpoint.face, jpoint.x, jpoint.y);

                            if (thisslope > slope)
                                slope = thisslope;
                        }
                    }
                }

                if (random(1, 60) < slope) // The steeper it is here, the more likely there are to be craters.
                    makethiscrater = 1;
            }

            if (checktype == 2)
            {
                if (thiselev > boundary)
                    makethiscrater = 1;
            }

            if (checktype == 3)
            {
                if (thiselev < boundary)
                    makethiscrater = 1;
            }

            for (int n = 0; n <= craterno; n++) // Don't allow two craters with precisely the same centre.
            {
                if (world.craterx(n) == thisx && world.cratery(n) == thisy && world.craterface(n) == thisface)
                    makethiscrater = 0;
            }

            if (makethiscrater)
            {
                int range = 10; // To ensure that most craters are small.

                if (random(1, 6) == 1)
                {
                    range = 25;

                    if (random(1, 8) == 1)
                    {
                        range = 50;

                        if (random(1, 20) == 1)
                        {
                            range = 70;

                            if (random(1, 50) == 1)
                                range = 100;
                        }
                    }
                }

                float thissize = (float)random(1, range);
                thissize = thissize / 100.0f;
                thissize = thissize * 15.0f; // In practice, can't draw craters of radius>15 on the regional map.

                if (thissize < 1.0f)
                    thissize = 1.0f;

                makecrater(world, squareroot, craterno, thisface, thisx, thisy, (int)thissize);
            }
        }
    }

    // If doing this has created sea when before there was none, probably lower the sea level.

    if (totalsea < 100)
    {
        int newtotalsea = 0;

        for (int face = 0; face < 6; face++)
        {
            for (int i = 0; i < edge; i++)
            {
                for (int j = 0; j < edge; j++)
                {
                    if (world.sea(face, i, j))
                        newtotalsea++;
                }
            }
        }

        if (newtotalsea > 100)
        {
            if (random(1, 10) != 1)
            {
                int lowest = maxelev;

                for (int face = 0; face < 6; face++)
                {
                    for (int i = 0; i < edge; i++)
                    {
                        for (int j = 0; j < edge; j++)
                        {
                            if (world.nom(face, i, j) < lowest)
                                lowest = world.nom(face, i, j);
                        }
                    }
                }

                int sealevel = lowest - random(10, 500);

                if (sealevel < 1)
                    sealevel = 1;

                world.setsealevel(sealevel);
            }
        }
    }
}

// This function creates a crater on the world map.

void makecrater(planet& world, vector <int>& squareroot, int thiscraterno, int centreface, int centrex, int centrey, int size)
{
    int edge = world.edge();
    int maxelev = world.maxelevation();

    int centreelev = size * 20;
    int rimelev = size * 250;

    float var = ((float)random(80, 120)) / 100.0f;

    centreelev = (int)((float)centreelev * var);

    var = ((float)random(80, 120)) / 100.0f;

    rimelev = (int)((float)rimelev * var);

    world.setcraterface(thiscraterno, centreface);
    world.setcraterx(thiscraterno, centrex);
    world.setcratery(thiscraterno, centrey);
    world.setcraterelev(thiscraterno, centreelev);
    world.setcraterradius(thiscraterno, size);

    int sizecheck = size * size + size;
    int sizechecksmall = (size - 1) * (size - 1) + size - 1;

    vector<vector<vector<bool>>> placed(6, vector<vector<bool>>(edge, vector<bool>(edge, 0))); // This notes where the crater has been placed.

    // First, find the base elevation for this crater.

    int baseelev = world.nom(centreface, centrex, centrey);

    // Now make a circular depression, with a rim.

    int depthmult = random(30, 70); // The higher this is, the deeper and more sloping the crater will be.

    int deepestelev = baseelev - (size * depthmult);

    if (deepestelev < 1)
        deepestelev = 1;

    int aveelev = deepestelev + (baseelev - deepestelev) / 2; // Roughly average elevation of the crater floor.

    for (int i = 0 - size; i <= size; i++)
    {
        for (int j = 0 - size; j <= size; j++)
        {
            globepoint jpoint = getglobepoint(edge, centreface, centrex, centrey, i, j);

            if (jpoint.face != -1)
            {
                int thischeck = i * i + j * j;

                if (thischeck <= sizecheck)
                {
                    // Work out how deep the crater should be at this point.

                    int depth = squareroot[sizecheck - thischeck];

                    depth = depth * depthmult;

                    int thiselev = baseelev - depth;

                    if (thiselev < 1)
                        thiselev = 1;

                    int oldelev = world.nom(jpoint.face, jpoint.x, jpoint.y);

                    if (oldelev >= deepestelev)
                    {
                        placed[jpoint.face][jpoint.x][jpoint.y] = 1;

                        world.setnom(jpoint.face, jpoint.x, jpoint.y, thiselev); // This is the flat crater floor.

                        world.setmountainheight(jpoint.face, jpoint.x, jpoint.y, 0);
                        world.setmountainridge(jpoint.face, jpoint.x, jpoint.y, 0);
                        world.setvolcano(jpoint.face, jpoint.x, jpoint.y, 0);
                        world.setcraterrim(jpoint.face, jpoint.x, jpoint.y, 0);
                        world.setcratercentre(jpoint.face, jpoint.x, jpoint.y, 0);

                        if (thischeck <= sizecheck && thischeck >= sizechecksmall)
                        {
                            world.setcraterrim(jpoint.face, jpoint.x, jpoint.y, rimelev); // This is the ring of mountains around the rim.
                        }
                    }
                }
            }
        }
    }

    world.setcratercentre(centreface, centrex, centrey, centreelev); // Do this only now, as we wiped any that the crater covers when making the crater floor just now.

    // Now raise the middle a bit around the central peak.

    int peakrad = (int)((float)size / 4.0f);

    if (peakrad < 1)
        peakrad = 1;

    int peakradcheck = peakrad * peakrad + peakrad;
    float peakbaseelev = (float)centreelev / 10.0f;

    for (int i = 0 - peakrad; i <= peakrad; i++)
    {
        for (int j = 0 - peakrad; j <= peakrad; j++)
        {
            globepoint jpoint = getglobepoint(edge, centreface, centrex, centrey, i, j);

            if (jpoint.face != -1)
            {
                int thischeck = i * i + j * j;

                if (thischeck <= peakradcheck)
                {
                    // Work out how much elevation to add at this point.

                    float dist = 1.0f;

                    if (thischeck > 1)
                        dist = (float)squareroot[thischeck];

                    dist = (float)peakrad - dist;

                    if (dist < 0.0f)
                        dist = 0.0f;

                    float mult = dist / (float)peakrad;

                    int newelev = (int)(peakbaseelev * mult);

                    int thiselev = world.nom(jpoint.face, jpoint.x, jpoint.y) + newelev;

                    world.setnom(jpoint.face, jpoint.x, jpoint.y, thiselev);
                }
            }
        }
    }

    // Now blur around the edges a little.

    int largesize = size + 2;

    for (int i = 0 - largesize; i <= largesize; i++)
    {
        for (int j = 0 - largesize; j <= largesize; j++)
        {
            globepoint jpoint = getglobepoint(edge, centreface, centrex, centrey, i, j);

            if (jpoint.face != -1)
            {
                if (placed[jpoint.face][jpoint.x][jpoint.y] == 0)
                {
                    bool nexttocrater = 0;

                    for (int k = -1; k <= 1; k++)
                    {
                        for (int l = -1; l <= 1; l++)
                        {
                            globepoint lpoint = getglobepoint(edge, jpoint.face, jpoint.x, jpoint.y, k, l);

                            if (lpoint.face != -1)
                            {
                                if (placed[lpoint.face][lpoint.x][lpoint.y] == 1)
                                {
                                    nexttocrater = 1;
                                    k = 1;
                                    l = 1;
                                }
                            }
                        }
                    }

                    if (nexttocrater)
                    {
                        int highestcrater = 0;

                        for (int k = -2; k <= 2; k++)
                        {
                            for (int l = -2; l <= 2; l++)
                            {
                                globepoint lpoint = getglobepoint(edge, jpoint.face, jpoint.x, jpoint.y, 0, l);

                                if (lpoint.face != -1)
                                {
                                    if (placed[lpoint.face][lpoint.x][lpoint.y] == 1 && world.nom(lpoint.face, lpoint.x, lpoint.y) > highestcrater)
                                        highestcrater = world.nom(lpoint.face, lpoint.x, lpoint.y);
                                }
                            }
                        }

                        int newelev = (world.nom(jpoint.face, jpoint.x, jpoint.y) + highestcrater) / 2;

                        world.setnom(jpoint.face, jpoint.x, jpoint.y, newelev);
                    }
                }
            }
        }
    }
}

// This function creates a new terrain map and blurs it into the map to smooth over awkward face boundaries.

void blurinterrain(planet& world, vector<fourglobepoints>& dirpoint, boolshapetemplate landshape[])
{
    int edge = world.edge();
    int maxelev = world.maxelevation();

    // This whole function is a dreadful fudge. The problem is that type-4 worlds have seams in the terrain between faces 5 and 1, and 5 and 2, and I can't identify what causes it.
    // So we create a mask covering the problematic seams and surrounding areas, suitably blurred.
    // Then we create a new terrain map.
    // Then we merge the new map into the old one, using the mask, but rotating the new terrain map so the seamless portion of the new one merges into the problematic seams of the original and obscures them.

    // First create the mask.

    vector<vector<vector<int>>> mask(6, vector<vector<int>>(edge, vector<int>(edge, 0)));

    int grain = 4; // Level of detail on this fractal map.
    float valuemod = 0.02f; // 0.03f;
    float valuemod2 = 0.02f; // 0.03f;

    createfractalforedgemask(mask, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    // Now make new terrain.

    vector<vector<vector<int>>> newterrain(6, vector<vector<int>>(edge, vector<int>(edge, 0)));

    int v = random(1, 100);

    if (random(1, 3) == 1)
        v = random(70, 100);

    float landscale = (float)v / 100.0f;

    makebasicterrain(world, landscale, newterrain, dirpoint, landshape, 0);

    // Blend in faces 0-3.

    for (int face = 0; face < 4; face++)
    {
        int fface = face + 2;

        if (fface > 3)
            fface = fface - 4;

        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                    float oldval = (float)world.nom(face, i, j);

                    float newval = (float)newterrain[fface][i][j];

                    float newmult = (float)mask[face][i][j] / (float)maxelev;
                    float oldmult = 1.0f - newmult;

                    newval = oldval * oldmult + newval * newmult;

                    world.setnom(face, i, j, (int)newval);
            }
        }
    } 

    // Blend in face 5. (No need to do 4, as the mask is blank in that portion - no problems with face 4.)

    for (int i = 0; i < edge; i++)
    {
        int ii = edge - 1 - i;

        for (int j = 0; j < edge; j++)
        {
            int jj = edge - 1 - j;

            float oldval = (float)world.nom(5, i, j);

            float newval = (float)newterrain[5][ii][jj];

            float newmult = (float)mask[5][i][j] / (float)maxelev;
            float oldmult = 1.0f - newmult;

            newval = oldval * oldmult + newval * newmult;

            world.setnom(5, i, j, (int)newval);
        }
    }
}

// This function adjusts the sea level.

void adjustsealevel(planet& world)
{
    int edge = world.edge();
    int maxelev = world.maxelevation();

    int lowest = maxelev;

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                int thisnom = world.nom(face, i, j);

                if (thisnom < lowest && thisnom>100)
                    lowest = thisnom;
            }
        }
    }

    int sealevel = lowest + randomsign(random(1, 4000));

    if (sealevel < 1)
        sealevel = 1;

    if (sealevel > maxelev - 100)
        sealevel = maxelev - 100;

    world.setsealevel(sealevel);
}

// This removes odd elevations at the edges of faces.

void removeedgeartefacts(planet& world)
{
    int edge = world.edge();

    for (int n = 0; n < edge; n++)
    {
        for (int m = 0; m < 6; m++)
        {
            globepoint thispoint, uppoint, downpoint;

            switch (m)
            {
            case 0:
                thispoint.face = 4;
                thispoint.x = 0;
                thispoint.y = n;

                uppoint.face = 4;
                uppoint.x = 1;
                uppoint.y = n;

                downpoint.face = 3;
                downpoint.x = n;
                downpoint.y = 0;
                break;

            case 1:
                thispoint.face = 4;
                thispoint.x = n;
                thispoint.y = 0;

                uppoint.face = 4;
                uppoint.x = n;
                uppoint.y = 1;

                downpoint.face = 2;
                downpoint.x = edge - 1 - n;
                downpoint.y = 0;
                break;

            case 2:
                thispoint.face = 4;
                thispoint.x = edge - 1;
                thispoint.y = n;

                uppoint.face = 4;
                uppoint.x = edge - 2;
                uppoint.y = n;

                downpoint.face = 1;
                downpoint.x = edge - 1 - n;
                downpoint.y = 0;
                break;

            case 3:
                thispoint.face = 5;
                thispoint.x = 0;
                thispoint.y = n;

                uppoint.face = 5;
                uppoint.x = 1;
                uppoint.y = n;

                downpoint.face = 3;
                downpoint.x = edge - 1 - n;
                downpoint.y = edge - 1;
                break;

            case 4:
                thispoint.face = 5;
                thispoint.x = n;
                thispoint.y = edge - 1;

                uppoint.face = 5;
                uppoint.x = n;
                uppoint.y = edge - 2;

                downpoint.face = 2;
                downpoint.x = edge - 1 - n;
                downpoint.y = edge - 1;
                break;

            case 5:
                thispoint.face = 5;
                thispoint.x = edge - 1;
                thispoint.y = n;

                uppoint.face = 5;
                uppoint.x = edge - 2;
                uppoint.y = n;

                downpoint.face = 1;
                downpoint.x = n;
                downpoint.y = edge - 1;
                break;
            }

            int thiselev = world.nom(thispoint.face, thispoint.x, thispoint.y);
            int upelev = world.nom(uppoint.face, uppoint.x, uppoint.y);
            int downelev = world.nom(downpoint.face, downpoint.x, downpoint.y);

            if ((thiselev > upelev && thiselev > downelev) || (thiselev < upelev && thiselev < downelev))
            {
                int newelev = (upelev + downelev) / 2;
                world.setnom(thispoint.face, thispoint.x, thispoint.y, newelev);
            }
        }
    }
}