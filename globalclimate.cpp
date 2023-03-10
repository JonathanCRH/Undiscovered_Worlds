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
#include "profiler.h"

using namespace std;

// This function creates the global climate.

void generateglobalclimate(planet& world, bool dorivers, bool dolakes,bool dodeltas, boolshapetemplate smalllake[], boolshapetemplate largelake[], boolshapetemplate landshape[], vector<vector<int>>& mountaindrainage, vector<vector<bool>>& shelves)
{
    //highres_timer_t timer("Generate Global Climate"); // 9.4s => 8.2s
    long seed = world.seed();
    fast_srand(seed);

    int width = world.width();
    int height = world.height();
    int maxelev = world.maxelevation();
    int sealevel = world.sealevel();
    int seatotal = world.seatotal();
    int landtotal = world.landtotal();

    // If there is no sea on a world, it may still have rain, provided we can find somewhere to put some salt lakes for the rivers to run into.

    int desertrainchance = 4; // On worlds with no sea, chance of trying to create rain anyway.
    int lakeattempts = random(1,8); // On worlds with no sea where there may be rain, make this many attempts to place a salt lake.

    int saltlakesplaced = 0;

    // If there's no sea, we will probably raise the sea level to be just below the lowest land.

    int raisesealevelchance = 6; // The higher this is, the *more* likely we are to try this.

    if (seatotal == 0 && random(1, raisesealevelchance) != 1)
    {
        int lowest = maxelev;

        for (int i = 0; i <= width; i++)
        {
            for (int j = 0; j <= height; j++)
            {
                int thiselev = world.nom(i, j);

                if (thiselev < lowest)
                    lowest = thiselev;
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

    vector<vector<vector<int>>> saltlakemap(ARRAYWIDTH, vector<vector<int>>(ARRAYHEIGHT, vector<int>(2)));
    vector<vector<int>> nolake(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));
    vector<vector<int>> basins(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0)); // This will store where endorheic basins have already been carved.

    if (dolakes)
    {
        for (int i = 0; i <= width; i++)
        {
            for (int j = 0; j <= height; j++)
            {
                saltlakemap[i][j][0] = 0;
                saltlakemap[i][j][1] = 0;
            }
        }

        for (int i = 0; i <= width; i++)
        {
            for (int j = 0; j <= height; j++)
            {
                if (world.outline(i, j) == 1)
                {
                    for (int k = i - minseadistance2; k <= i + minseadistance2; k++)
                    {
                        int kk = k;

                        if (kk<0 || kk>width)
                            kk = wrap(kk, width);

                        for (int l = j - minseadistance2; l <= j + minseadistance2; l++)
                        {
                            if (l >= 0 && l <= height)
                                nolake[kk][l] = 1;
                        }
                    }

                    for (int k = i - minseadistance; k <= i + minseadistance; k++)
                    {
                        int kk = k;

                        if (kk<0 || kk>width)
                            kk = wrap(kk, width);

                        for (int l = j - minseadistance; l <= j + minseadistance; l++)
                        {
                            if (l >= 0 && l <= height && nolake[kk][l] == 0)
                                nolake[kk][l] = 2;
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

        for (int i = 0; i <= width; i++)
        {
            for (int j = 0; j <= height; j++)
            {
                int thiselev = world.nom(i, j);

                if (thiselev < lowestelev)
                    lowestelev = thiselev;

                if (thiselev > highestelev)
                    highestelev = thiselev;
            }
        }

        int elevdiff = highestelev - lowestelev;

        int maxlakeelev = lowestelev + elevdiff / 4;

        vector<vector<int>> avoid(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0)); // This is for cells *not* to alter. This will remain empty (it's just here so we can use the depression-creating routine for normal lakes as well, where we don't want to mess with the path of the outflowing river.)

        for (int n = 0; n < lakeattempts; n++)
        {
            int x = random(1, width);
            int y = random(10, height - 10);

            for (int m = 0; m < 1000; m++)
            {
                if (world.nom(x, y) > maxlakeelev)
                {
                    x = random(1, width);
                    y = random(10, height - 10);
                }
                else
                    m = 1000;
            }

            placesaltlake(world, x, y, 1, 0, saltlakemap, basins, avoid, nolake, smalllake);
        }

        bool foundsea = 0; // Now look to see whether that worked. If it didn't, then there will be no rain on this world.

        for (int i = 0; i <= width; i++)
        {
            for (int j = 0; j <= height; j++)
            {
                if (world.nom(i, j) <= sealevel)
                {
                    desertworldrain = 1;
                    i = width;
                    j = height;
                }
            }
        }
    }

    if (desertworldrain)
    {
        // Now remove depressions.

        updatereport("Filling depressions");

        depressionfill(world);

        addlandnoise(world); // Add a bit of noise, then do remove depressions again. This is to add variety to the river courses.

        depressionfill(world);
    }

    // Now, set the river land reduce factor.

    float riverlandreduce = 20.0f * world.gravity() * world.gravity();
    world.setriverlandreduce((int)riverlandreduce);

    // Now, do the wind map.

    updatereport("Generating wind map");

    createwindmap(world);

    // Now create the temperature map.

    updatereport("Generating global temperature map");

    // Start by generating a new fractal map.

    int grain = 8; // Level of detail on this fractal map.
    float valuemod = 0.2f;
    int v = random(1, 4);
    float valuemod2 = float(v);

    vector<vector<int>> fractal(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

    createfractal(fractal, width, height, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    int warpfactor = random(40, 80);
    warp(fractal, width, height, maxelev, warpfactor, 0);

    createtemperaturemap(world, fractal);

    // Now do the sea ice.

    if (seatotal > 0)
    {
        updatereport("Generating sea ice map");

        for (int i = 0; i <= width; i++)
        {
            for (int j = 0; j <= height; j++)
                fractal[i][j] = 0;
        }

        createfractal(fractal, width, height, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

        warpfactor = random(40, 80);
        warp(fractal, width, height, maxelev, warpfactor, 0);

        createseaicemap(world, fractal);

        // Work out the tidal ranges.

        updatereport("Calculating tides");

        createtidalmap(world);
    }

    // Now do rainfall.

    if (seatotal > 0 || desertworldrain)
    {
        for (int i = 0; i <= width; i++)
        {
            for (int j = 0; j <= height; j++)
                fractal[i][j] = 0;
        }

        createfractal(fractal, width, height, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

        warpfactor = random(40, 80);
        warp(fractal, width, height, maxelev, warpfactor, 0);
        warp(fractal, width, height, maxelev, warpfactor, 0);

        createrainmap(world, fractal, landtotal, seatotal, smalllake, landshape);
    }

    // Now add fjord mountains.

    if (seatotal > 0)
    {
        updatereport("Carving fjords");

        addfjordmountains(world);
    }

    if (dorivers && (seatotal > 0 || desertworldrain))
    {
        // Now work out the rivers initially. We do this the first time so that after the first time we can place the salt lakes in appropriate places, and then we work out the rivers again.

        updatereport("Planning river courses");

        createrivermap(world, mountaindrainage);

        if (dolakes || seatotal==0) // If there's no sea we need at least one salt lake.
        {
            // Now create salt lakes.

            updatereport("Placing hydrological basins");

            createsaltlakes(world, saltlakesplaced, saltlakemap, nolake, basins, smalllake);

            addlandnoise(world);
            depressionfill(world);

            for (int i = 0; i <= width; i++)
            {
                for (int j = 0; j <= height; j++)
                {
                    world.setriverdir(i, j, 0);
                    world.setriverjan(i, j, 0);
                    world.setriverjul(i, j, 0);
                }
            }

            // Now work out the rivers again.

            updatereport("Generating rivers");

            createrivermap(world, mountaindrainage);
        }

        // Now check river valleys in mountains.

        updatereport("Checking mountain river valleys");

        removerivermountains(world);

        if (dolakes)
        {
            // Now create the lakes.

            updatereport("Generating lakes");

            convertsaltlakes(world, saltlakemap);

            createlakemap(world, nolake, smalllake, largelake);

            createriftlakemap(world, nolake);
        }
        else
        {
            if (seatotal==0)
                convertsaltlakes(world, saltlakemap);
        }      
    }

    world.setmaxriverflow();

    // Now create the climate map.

    updatereport("Calculating climates");

    createclimatemap(world);

    // Now specials.

    updatereport("Generating sand dunes");

    createergs(world, smalllake, largelake, landshape);

    updatereport("Generating salt pans");

    createsaltpans(world, smalllake, largelake);

    // Add river deltas.

    if (dodeltas)
    {
        updatereport("Generating river deltas");

        createriverdeltas(world);
        checkrivers(world);
    }

    // Now wetlands.

    updatereport("Generating wetlands");

    createwetlands(world, smalllake);

    removeexcesswetlands(world);

    // Now it's time to finesse the roughness map.

    updatereport("Refining roughness map");

    refineroughnessmap(world);

    // Check the rift lake map too.

    for (int i = 0; i < ARRAYWIDTH; i++)
    {
        for (int j = 0; j < ARRAYHEIGHT; j++)
        {
            if (world.lakestart(i, j) == 1 && world.riftlakesurface(i, j) == 0 && world.lakesurface(i, j) == 0)
                world.setlakestart(i, j, 0);
        }
    }

    // Check that the climates at the edges of the map are correct.

    updatereport("Checking poles");

    checkpoleclimates(world);

    if (dolakes == 0)
    {
        for (int i = 0; i <= width; i++)
        {
            for (int j = 0; j <= height; j++)
            {
                if (world.special(i, j) != 110)
                {
                    world.setlakesurface(i, j, 0);
                    world.setlakestart(i, j, 0);
                    world.setriftlakesurface(i, j, 0);
                    world.setriftlakebed(i, j, 0);
                }
            }
        }
    }

    removesealakes(world); // Also, make sure there are no weird bits of sea next to lakes.

    connectlakes(world); // Make sure lakes aren't fragmented.

    for (int i = 0; i <= width; i++) // Check erg/salt pans are the right depth.
    {
        for (int j = 0; j <= height; j++)
        {
            int special = world.special(i, j);
            
            if (special == 110 || special == 120)
            {
                int level = world.lakesurface(i, j);

                if (level <= sealevel)
                {
                    level = sealevel + 1;
                    world.setlakesurface(i, j, level);
                }

                world.setnom(i, j, level);
            }
        }
    }
}

// Rain map creator.

void createrainmap(planet& world, vector<vector<int>>& fractal,  int landtotal, int seatotal, boolshapetemplate smalllake[], boolshapetemplate shape[])
{
    int width = world.width();
    int height = world.height();
    
    int slopewaterreduce = 20; // The higher this is, the less extra rain falls on slopes.
    int maxmountainheight = 100;

    vector<vector<int>> inland(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

    // First, do rainfall over the oceans.

    if (seatotal > 0)
    {
        updatereport("Calculating ocean rainfall");

        createoceanrain(world, fractal);
    }

    if (landtotal > 0)
    {
        // Now we do the rainfall over land.

        if (seatotal > 0)
        {
            updatereport("Calculating rainfall from prevailing winds");
            
            createprevailinglandrain(world, inland, maxmountainheight, slopewaterreduce);

            // Now for monsoons!

            updatereport("Calculating monsoons");

            createmonsoons(world, maxmountainheight, slopewaterreduce);
        }
        else
        {
            updatereport("Calculating rainfall");

            createdesertworldrain(world);

            for (int i = 0; i <= width; i++)
            {
                for (int j = 0; j <= height; j++)
                    inland[i][j] = 10000;
            }
        }
    }

    // Now increase the seasonal variation in rainfall in certain areas, to encourage various climates.

    updatereport("Calculating seasonal rainfall");

    adjustseasonalrainfall(world, inland);

    // Now smooth the rainfall.

    updatereport("Smoothing rainfall");

    smoothrainfall(world, maxmountainheight);

    // Now cap excessive rainfall.

    updatereport("Capping rainfall");

    caprainfall(world);

    // Now we adjust temperatures in light of rainfall.

    updatereport("Adjusting temperatures");

    adjusttemperatures(world, inland);

    // Now make temperatures a little more extreme when further from the sea.

    updatereport("Adjusting continental temperatures");

    adjustcontinentaltemperatures(world, inland);

    // Now just smooth the temperatures a bit. Any temperatures that are lower than their neighbours to north and south get bumped up, to avoid the appearance of streaks.

    updatereport("Smoothing temperatures");

    smoothtemperatures(world);

    // Now just prevent subpolar climates from turning into other continental types when further from the sea.

    updatereport("Checking subpolar regions");

    removesubpolarstreaks(world);
    //extendsubpolar(world);
    //removesubpolarstreaks(world);

    // Now we just sort out the mountain precipitation arrays, which will be used at the regional level for ensuring that higher mountain precipitation isn't splashed too far.

    updatereport("Calculating mountain rainfall");

    createmountainprecipitation(world);
}

// Wind map creator.

void createwindmap(planet& world)
{
    int width = world.width();
    int height = world.height();
    bool rotation = world.rotation();

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

    int variation = random(10, 30); //random(8,10); // Maximum amount a node can vary from the base amount.
    float div = height / 180.0f; // This is the amount that corresponds to each degree latitude

    int linecol = 1000000; // Value for the lines between wind zones.
    int pets = 20; // This is the step width between each node of the icelines.

    int n = width / pets; // This is how many nodes there will be.

    vector<vector<int>> windlines(12, vector<int>(n + 1, 0)); // 0 holds the x coordinate of that node. 1 to 10 hold the y coordinates of the different lines.

    for (int i = 0; i <= n; i++)
        windlines[0][i] = i * pets;

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
            world.setwind(i, j, -1);
    }

    // First we fill in the basic y coordinate of each node.

    for (int i = 0; i <= n; i++)
    {
        for (int line = 1; line <= 10; line++)
        {
            float y = (float)borders[line] * div;

            windlines[line][i] = (int)y;
        }
    }

    for (int line = 1; line <= 10; line++)
    {
        float y = (float)borders[line] * div;

        windlines[line][0] = (int)y;
    }

    int maxoffset = 25;
    int maxoffsetchange = 3;
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
            int jj = windlines[line][i] + yoffset + yextraoffset;

            if (yoffset != 0)
            {
                int amount2 = yoffset / 2;

                if (line < 10) // Move all the lower ones accordingly.
                {
                    for (int line2 = line + 1; line2 <= 10; line2++)
                    {
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

                        int jjj = windlines[line2][i];
                        jjj = jjj + amount2;
                        windlines[line2][i] = jjj + randomsign(random(1, maxextraoffset));
                    }
                }

                if (line > 1) // Move all the higher ones accordingly.
                {
                    for (int line2 = 1; line2 <= line - 1; line2++)
                    {
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

                        int jjj = windlines[line2][i];
                        jjj = jjj + amount2;
                        windlines[line2][i] = jjj + randomsign(random(1, maxextraoffset));
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
            if (windlines[line][i] < windlines[line - 1][i] + 4)
                windlines[line][i] = windlines[line - 1][i] + 4;
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
            mm1.x = (float) windlines[0][m1];
            mm1.y = (float) windlines[line][m1];

            mm2.x = (float) windlines[0][m2];
            mm2.y = (float) windlines[line][m2];

            mm3.x = (float) windlines[0][m3];
            mm3.y = (float) windlines[line][m3];

            mm4.x = (float) windlines[0][m4];
            mm4.y = (float) windlines[line][m4];

            if (mm2.x < mm1.x) // This dewraps them all, so some may extend beyond the eastern edge of the map.
                mm2.x = (float) mm2.x + width;

            if (mm3.x < mm2.x)
                mm3.x = (float) mm3.x + width;

            if (mm4.x < mm3.x)
                mm4.x = (float) mm4.x + width;

            for (float t = 0.0f; t <= 1.0f; t = t + 0.01f)
            {
                pt = curvepos(mm1, mm2, mm3, mm4, t);

                int x = (int)pt.x;
                int y = (int)pt.y;

                if (x<0 || x>width)
                    x = wrap(x, width);

                int col = linecol + line;

                if (y >= 0 && y <= height)
                    world.setwind(x, y, col);

            }
        }
    }

    // Now we fill in the wind directions behind the lines.

    vector<vector<int>> winddir(11, vector<int>(3, 0));

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

    winddir[5][0] = 0;
    winddir[5][1] = 0;
    winddir[5][2] = 0;

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

    for (int i = 0; i <= width; i++)
    {
        int zone = 0;

        for (int j = 0; j <= height; j++)
        {
            if (world.wind(i, j) >= linecol)
            {
                zone = world.wind(i, j) - linecol;

                if (zone > 10)
                    zone = 10;

                if (zone == 3)
                    world.sethorse(i, 1, j);

                if (zone == 4)
                    world.sethorse(i, 2, j);

                if (zone == 7)
                    world.sethorse(i, 3, j);

                if (zone == 8)
                    world.sethorse(i, 4, j);
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

                world.setwind(i, j, wind);
            }
        }
    }

    // Now we clean up the edge.

    for (int j = 0; j <= height; j++)
        world.setwind(0, j, world.wind(width, j));

    // Now we remove the borders.

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (world.wind(i, j) >= linecol)
            {
                bool alldone = 0;
                int jj = j;

                do
                {
                    jj--;

                    if (world.wind(i, jj) < linecol)
                    {
                        world.setwind(i, j, world.wind(i, jj));
                        alldone = 1;
                    }

                } while (alldone == 0);
            }
        }
    }

    // Now we need to put provisional values in the zones without winds.

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (world.wind(i, j) == 0)
            {
                int up = 0;
                bool found = 0;
                int jj = j;
                bool northpositive = 0;

                do
                {
                    if (world.wind(i, jj) != 0 && world.wind(i, jj) < 50) // If there is a proper value here
                    {
                        found = 1;

                        if (world.wind(i, jj) > 0)
                            northpositive = 1;

                        else
                            northpositive = 0;
                    }

                    jj--;
                    up++;

                    if (up > height)
                        found = 1;

                } while (found == 0);

                int down = 0;
                found = 0;
                jj = j;
                bool southpositive = 0;

                do
                {
                    if (world.wind(i, jj) != 0 && world.wind(i, jj) < 50) // If there is a proper value here
                    {
                        found = 1;

                        if (world.wind(i, jj) > 0)
                            southpositive = 1;

                        else
                            southpositive = 0;
                    }

                    jj++;
                    down++;

                    if (down > height)
                        found = 1;

                } while (found == 0);

                if (down > up)
                {
                    if (northpositive)
                        world.setwind(i, j, 101);
                    else
                        world.setwind(i, j, 99);

                }
                else
                {
                    if (southpositive)
                        world.setwind(i, j, 101);
                    else
                        world.setwind(i, j, 99);
                }
            }
        }
    }
}

// Temperature map creator.

void createtemperaturemap(planet& world, vector<vector<int>>& fractal)
{
    int width = world.width();
    int height = world.height();
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

    float fracvar = (float)maxelev / fracrange; // FG: Note that integer division will always return an integer

    float equatorlat = (float)(height / 2); // FG: Note that integer division will always return an integer

    float northglobaldiff = equatorialtemp - northpolartemp; // This is the range in average temperatures from equator to pole, in the northern hemisphere.
    float southglobaldiff = equatorialtemp - southpolartemp; // This is the range in average temperatures from equator to pole, in the southern hemisphere.

    float northdiffperlat = northglobaldiff / equatorlat;
    float southdiffperlat = southglobaldiff / equatorlat;

    float variationperlat = polarvariation / equatorlat;

    // Now work out the actual temperatures. We do this in horizontal strips.

    float lattemp = northpolartemp;

    for (float j = 0.0f; j <= (float)height; j++)
    {
        // First get the base temperature for this latitude.

        float thisnorthdiffperlat = northdiffperlat;
        float thissouthdiffperlat = southdiffperlat;

        // Northern hemisphere

        if (j < equatorlat * 0.05f)
            thisnorthdiffperlat = thisnorthdiffperlat * 1.6f; // 0.6

        if (j > equatorlat * 0.05f && j < equatorlat * 0.1f)
            thisnorthdiffperlat = thisnorthdiffperlat * 2.5f; // 2.5

        if (j > equatorlat * 0.1f && j < equatorlat * 0.3f)
            thisnorthdiffperlat = thisnorthdiffperlat * 1.8f; // 1.8

        if (j > equatorlat * 0.3f && j < equatorlat * 0.45f)
            thisnorthdiffperlat = thisnorthdiffperlat * 0.4f; // 0.6

        if (j > equatorlat * 0.45f && j < equatorlat * 0.6f)
            thisnorthdiffperlat = thisnorthdiffperlat * 1.2f; // 1.2

        if (j > equatorlat * 0.6f && j < equatorlat)
            thisnorthdiffperlat = thisnorthdiffperlat * 0.7f; // 0.7

        // Southern hemisphere

        if (j > (float)height - equatorlat * 0.05f)
            thisnorthdiffperlat = thisnorthdiffperlat * 1.6f; // 0.6

        if (j<(float)height - equatorlat * 0.05f && j>(float)height - equatorlat * 0.1f)
            thisnorthdiffperlat = thisnorthdiffperlat * 2.5f; // 2.5

        if (j<(float)height - equatorlat * 0.1f && j>(float)height - equatorlat * 0.3f)
            thisnorthdiffperlat = thisnorthdiffperlat * 1.8f; // 1.8

        if (j<(float)height - equatorlat * 0.3f && j>(float)height - equatorlat * 0.45)
            thisnorthdiffperlat = thisnorthdiffperlat * 0.4f; // 0.6

        if (j<(float)height - equatorlat * 0.45f && j>(float)height - equatorlat * 0.6f)
            thisnorthdiffperlat = thisnorthdiffperlat * 1.2f; // 1.2

        if (j<(float)height - equatorlat * 0.6f && j>(float)height - equatorlat)
            thisnorthdiffperlat = thisnorthdiffperlat * 0.7f; // 0.7

        if (j < equatorlat)
            lattemp = lattemp + thisnorthdiffperlat;
        else
            lattemp = lattemp - thissouthdiffperlat;

        float lat;

        if (j < equatorlat)
            lat = equatorlat - j;

        else
            lat = j - equatorlat;

        // Now get the seasonal variation for this latitude.

        float latvariation = (lat * variationperlat) * 0.5f;

        // Now go through the different longitudes at this latitude...

        for (int i = 0; i <= width; i++)
        {
            float temperature = lattemp;

            // Now we adjust for sea temperatures.

            if (world.sea(i, (int)j))
                temperature = temperature + 2.0f;

            // Now add variation from the fractal.

            float var = (float)fractal[i][(int)j];

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

            if (j < equatorlat)
            {
                world.setjantemp(i, (int)j, newmintemp);
                world.setjultemp(i, (int)j, newmaxtemp);
            }
            else
            {
                world.setjultemp(i, (int)j, newmintemp);
                world.setjantemp(i, (int)j, newmaxtemp);
            }
        }
    }

    // Now blur it.

    for (int n = 0; n < 4; n++)
    {
        for (int i = 0; i <= width; i++)
        {
            for (int j = 0; j <= height; j++)
            {
                int crount = 0;
                int jantotal = 0;
                int jultotal = 0;

                for (int k = i - 2; k <= i + 2; k++)
                {
                    int kk = k;

                    if (kk<0 || kk>width)
                        kk = wrap(kk, width);

                    for (int l = j - 2; l <= j + 2; l++)
                    {
                        if (l >= 0 && l <= height)
                        {
                            jantotal = jantotal + world.jantemp(kk, l);
                            jultotal = jultotal + world.jultemp(kk, l);

                            crount++;
                        }
                    }
                }

                world.setjantemp(i, j, jantotal / crount);
                world.setjultemp(i, j, jultotal / crount);
            }
        }
    }


    // At high obliquity, equatorial regions have a two-summmer/two-winter seasonal cycle. 
    // Here, we adjust Jan/Jul temperatures accordingly.

    float fourseason = tilt * 0.294592f - 2.45428f; // This is the difference in temperature between the "summers" and the "winters".

    for (int j = 0; j <= height; j++)
    {
        float lat = (float)j;

        if (j > (int)equatorlat)
            lat = (float)(height - j);

        float fourseasonstrength = lat / equatorlat; // This measures the strength of the four-season cycle as determined by proximity to the equator.

        float thistempdiff = (fourseason * fourseasonstrength) / 2.0f;

        for (int i = 0; i <= width; i++)
        {
            int jantemp = world.jantemp(i, j);
            int jultemp = world.jultemp(i, j);

            world.setjantemp(i, j, jantemp - (int)thistempdiff); // Remove the temp difference from the solstices.
            world.setjultemp(i, j, jultemp - (int)thistempdiff);
        }
    }

    // Now adjust all the temperatures for eccentricity.
    // The more elliptical the orbit, the greater the adjustment. Also note: it is greater at the equator, not the poles.

    int perihelion = world.perihelion();
    float eccentricity = world.eccentricity();

    float betweenhot = 0.5f * (1.0f - eccentricity); // The higher the eccentricity, the shorter the "summer" will be.
    float betweencold = 1.0f - betweenhot;

    float eccendiff = (eccentricity * eccentricity) * 100.0f + eccentricity * 30.0f; // Temperature difference between "summer" and "winter".

    for (int j = 0; j <= height; j++)
    {
        float lat = (float)j;

        if (j > (int)equatorlat)
            lat = (float)(height - j);

        float eccenstrength = lat / equatorlat; // This measures the strength of the eccentricity "seasons". They're stronger at the equator.

        eccenstrength = (1.0f + eccenstrength) / 2.0f; // Half-strength at the poles.

        float thistempdiffhot = (eccendiff * eccenstrength) / 2.0f;
        float thistempdiffcold = 0.0f - thistempdiffhot;

        for (int i = 0; i <= width; i++)
        {
            float jantemp = (float)world.jantemp(i, j);
            float jultemp = (float)world.jultemp(i, j);

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


            world.setjantemp(i, j, (int)jantemp);
            world.setjultemp(i, j, (int)jultemp);

            world.settest(i, j, (int)thistempdiffhot);
        }
    }

    // Now adjust for altitude.

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            int jantemp = world.jantemp(i, j);
            int jultemp = world.jultemp(i, j);

            jantemp = tempelevadd(world, jantemp, i, j);
            jultemp = tempelevadd(world, jultemp, i, j);

            world.setjantemp(i, j, jantemp);
            world.setjultemp(i, j, jultemp);
        }
    }
}

// Work out the sea ice.

void createseaicemap(planet& world, vector<vector<int>>& fractal)
{
    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();
    int maxelev = world.maxelevation();

    int permice = -5; // If it's this temperature or lower all year, permanent sea ice.
    int seasice = -16; // If it's this temperature or lower for part of the year, seasonal sea ice.

    int maxadjust = 12; // Maximum amount temperatures can be adjusted by
    int adjustfactor = maxelev / maxadjust;

    int tempdist = 12; // Distance from the current point where we'll check temperatures
    float landfactor = 20.0f; // The higher this is, the less effect land will have on sea ice

    // First, check that the whole world isn't frozen

    bool foundnoperm = 0;

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (world.jantemp(i, j) <= permice || world.jultemp(i, j) <= permice)
            {
                foundnoperm = 1;
                i = width;
                j = height;
            }
        }
    }

    if (foundnoperm == 0)
    {
        for (int i = 0; i <= width; i++)
        {
            for (int j = 0; j <= height; j++)
                world.setseaice(i, j, 2);
        }

        return;
    }

    // Now work out temperature reductions

    vector<vector<int>> landreduce(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0)); // How much to reduce temperatures by, from nearby land

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            int thislanddist = landdistance(world, i, j);

            if (thislanddist > 0)
            {
                float flanddist = (float)thislanddist;

                flanddist = 200.0f - flanddist;

                if (flanddist < 0.0)
                    flanddist = 0.0f;

                if (flanddist > 200.0)
                    flanddist = 200.0f;

                flanddist = flanddist / landfactor;

                landreduce[i][j] = (int)flanddist;
            }
        }
    }

    // Now assign the ice

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            int maxtotal = 0;
            int mintotal = 0;
            int crount = 0;

            for (int k = -tempdist; k <= tempdist; k++)
            {
                int kk = i + k;

                if (kk<0 || kk>width)
                    kk = wrap(kk, width);

                for (int l = -tempdist; l <= tempdist; l++)
                {
                    if (k * k + l * l < tempdist * tempdist + tempdist)
                    {
                        int ll = j + l;

                        if (ll >= 0 && ll <= height)
                        {
                            int thismaxtemp = world.maxtemp(kk, ll) - landreduce[kk][ll];
                            int thismintemp = world.mintemp(kk, ll) - landreduce[kk][ll];

                            maxtotal = maxtotal + thismaxtemp;
                            mintotal = mintotal + thismintemp;
                            crount++;
                        }
                    }
                }
            }

            int maxtemp = maxtotal / crount + (fractal[i][j] / adjustfactor) - maxadjust / 2;
            int mintemp = mintotal / crount + (fractal[i][j] / adjustfactor) - maxadjust / 2;

            if (maxtemp <= permice && mintemp <= permice)
                world.setseaice(i, j, 2);
            else
            {
                if (mintemp <= seasice)
                    world.setseaice(i, j, 1);
            }
        }
    }

    // Remove odd bits

    for (int i = 0; i <= width; i++)
    {
        for (int j = 1; j < height; j++)
        {
            if (world.seaice(i, j) == 1)
            {
                if (world.seaice(i, j - 1) == 0 && world.seaice(i, j + 1) == 0)
                    world.setseaice(i, j, 0);
            }

            if (world.seaice(i, j) == 2)
            {
                if (world.seaice(i, j - 1) == 1 && world.seaice(i, j + 1) == 1)
                    world.setseaice(i, j, 2);
            }

        }
    }

    // Now we clean up the edge.

    for (int j = 0; j <= height; j++)
        world.setseaice(0, j, world.seaice(width, j));
}

// This creates rain over the oceans.

void createoceanrain(planet& world, vector<vector<int>>& fractal)
{
    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();
    int maxelev = world.maxelevation();

    int landmult = 5; // This is the amount we multiply the wind factor by to calculate land shadows.
    int fracrange = 60; // This is the range of variation given by the fractal.
    int maxoceanrain = 1500; // Maximum amount of rain per month over the ocean.
    int landshadowfactor = 4; // The amount that the land shadows affect ocean rainfall.

    for (int j = 0; j <= height; j++)
    {
        for (int i = 0; i <= width; i++)
        {
            if (world.nom(i, j) < sealevel)
            {
                int winterrain = world.mintemp(i, j) + 15;
                int summerrain = world.maxtemp(i, j) + 15;

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

                world.setwinterrain(i, j, winterrain);
                world.setsummerrain(i, j, summerrain);
            }
        }
    }

    // Now do the land shadows over oceans.
    // First the westerly ones.

    for (int j = 0; j <= height; j++)
    {
        for (int i = 1; i <= width; i++)
        {
            if (world.wind(i, j) > 0 && world.wind(i, j) < 50)
            {
                if (world.nom(i, j) <= sealevel && world.nom(i - 1, j) > sealevel) // If this is a coastal tile
                {
                    int crount = world.wind(i, j) * landmult;

                    int dryness = 0; // This will lot how much dryness we're dealing with here.

                    // First check to the west and see how much land there is.

                    while (crount > 0)
                    {
                        int ii = i - crount;

                        if (ii < 0)
                            ii = wrap(i, width);

                        if (world.nom(ii, j) > sealevel)
                            dryness++;

                        crount--;
                    }

                    crount = 0;

                    // Now we remove rainfall from the sea to the east.

                    if (dryness > 0)
                    {
                        for (int z = 1; z <= dryness; z++)
                        {
                            int ii = i + z;

                            if (ii > width)
                                ii = wrap(ii, width);

                            int amounttoremove = (dryness - z) * landshadowfactor;

                            world.setsummerrain(ii, j, world.summerrain(ii, j) - amounttoremove);
                            world.setwinterrain(ii, j, world.winterrain(ii, j) - amounttoremove);

                            if (world.summerrain(ii, j) < 0)
                                world.setsummerrain(ii, j, 0);

                            if (world.winterrain(ii, j) < 0)
                                world.setwinterrain(ii, j, 0);
                        }
                    }
                }
            }
        }
    }

    // Then the easterly ones.

    for (int j = 0; j <= height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            if (world.wind(i, j) < 0)
            {
                if (world.nom(i, j) <= sealevel && world.nom(i + 1, j) > sealevel) // If this is a coastal tile
                {
                    int crount = 0 - world.wind(i, j) * landmult;
                    int dryness = 0; // This will lot how much dryness we're dealing with here.

                    // First check to the east and see how much land there is.

                    while (crount > 0)
                    {
                        int ii = i + crount;

                        if (ii > width)
                            ii = wrap(i, width);

                        if (world.nom(ii, j) > sealevel)
                            dryness++;

                        crount--;
                    }

                    crount = 0;

                    // Now we remove rainfall from the sea to the west.

                    if (dryness > 0)
                    {
                        for (int z = 1; z <= dryness; z++)
                        {
                            int ii = i - z;

                            if (i < 0)
                                ii = wrap(ii, width);

                            int amounttoremove = (dryness - z) * landshadowfactor;

                            world.setsummerrain(ii, j, world.summerrain(ii, j) - amounttoremove);
                            world.setwinterrain(ii, j, world.winterrain(ii, j) - amounttoremove);

                            if (world.summerrain(ii, j) < 0)
                                world.setsummerrain(ii, j, 0);

                            if (world.winterrain(ii, j) < 0)
                                world.setwinterrain(ii, j, 0);
                        }
                    }
                }
            }
        }
    }

    // Now add variation from the fractal.

    float fracvar = (float)maxelev / (float)fracrange;

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            float var = (float)fractal[i][j];
            var = var / fracvar;
            var = var - ((float)fracrange / 2.0f);

            if (world.winterrain(i, j) > 0)
                world.setwinterrain(i, j, world.winterrain(i, j) + (int)var);
            else
                world.setwinterrain(i, j, (int)var);

            if (world.summerrain(i, j) > 0)
                world.setsummerrain(i, j, world.summerrain(i, j) + (int)var);
            else
                world.setsummerrain(i, j, (int)var);
        }
    }

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (world.winterrain(i, j) < 0)
                world.setwinterrain(i, j, 0);

            if (world.summerrain(i, j) < 0)
                world.setsummerrain(i, j, 0);
        }
    }
}

// This creates prevailing rain over the land.

void createprevailinglandrain(planet& world, vector<vector<int>>& inland, int maxmountainheight, int slopewaterreduce)
{
    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();
    float tilt = world.tilt();
    float tempdecrease = world.tempdecrease() * 20.0f; // The default is 6.5 * 20.0f = 130.0.

    int seamult = 60; // This is the amount we multiply the wind factor by to get the number of sea tiles that will provide rain. The higher this is, the more rain will extend onto the land.
    float tempfactor = 80.0f; // The amount temperature affects moisture pickup over ocean. The higher it is, the more difference it makes.
    float mintemp = 0.15f; // The minimum fraction that low temperatures can reduce the pickup rate to.
    int dumprate = 80; // The higher this number, the less rain gets deposited, but the further across the land it is distributed.
    float fpickuprate=world.waterpickup() * 200.0f;   
    int pickuprate = (int)fpickuprate; // The higher this number, the more rain gets picked up over ocean tiles.
    int landpickuprate = 40; // This is the amount of rain that gets acquired while passing over land.
    int swervechance = 3; // The lower this number, the more variation in where the rain lands.
    int spreadchance = 2; // The lower this number, the more precipitation will spread to north and south.
    float newseedproportion = 0.95f; // When precipitation spreads, it's reduced to this proportion.
    float horseseedproportion = 0.4f; // If spreading into horse latitudes, it's reduced to this proportion.
    int splashsize = 1; // The higher this number, the larger area gets splashed around each target tile.    
    float elevationfactor = 0.002f; // This determines how much elevation affects the degree to which gradient affects rainfall.
    int slopemin = 300; //200; // This is how high land has to be before the gradient affects rainfall.
    //int fracrange=60; // This is the range of variation given by the fractal.
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

    vector<vector<int>> rainseed(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));
    vector<vector<int>> raintick(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));
    vector<vector<float>> rainheatseed(ARRAYWIDTH, vector<float>(ARRAYHEIGHT, 0));
    vector<vector<short>> raindir(ARRAYWIDTH, vector<short>(ARRAYHEIGHT, 0));

    // First, set the seeds for each bit of rain.

    // Westerly first.

    for (int j = 0; j <= height; j++)
    {
        for (int i = 1; i <= width; i++)
        {
            if (world.wind(i, j) > 0 && world.wind(i, j) != 99)
            {
                int currentwind = world.wind(i, j);

                if (currentwind == 101)
                    currentwind = 4;

                if (world.nom(i, j) > sealevel && world.nom(i - 1, j) <= sealevel) // If this is a coastal tile
                {
                    int crount = currentwind * seamult;
                    int waterlog = 0; // This is the amount of water being carried.
                    float waterheat = 0.0f; // This is the amount of extra heat being carried.
                    bool seaice = 0; // This will be 1 if any seasonal sea ice is found.

                    while (crount > 0)
                    {
                        int ii = i - crount;

                        if (ii < 0)
                            ii = wrap(ii, width);

                        if (world.nom(ii, j) <= sealevel && world.seaice(ii, j) != 2) // It picks up moisture from unfrozen sea
                        {
                            if (world.seaice(ii, j) == 1)
                                seaice = 1;

                            float temp = (float)((world.maxtemp(ii, j) + world.mintemp(ii, j)) / 2);
                            temp = temp / tempfactor; // Less water is picked up from colder oceans.

                            if (temp < mintemp)
                                temp = mintemp;

                            temp = (temp / 100.0f) + 1.0f;

                            waterlog = waterlog + (int)((float)pickuprate * temp);
                            waterheat = waterheat + heatpickuprate;

                            if (seaice == 1)
                                waterlog = waterlog / 2;

                            if (waterheat > maxwinterheatfactor)
                                waterheat = maxwinterheatfactor;

                            rainseed[i][j] = waterlog;
                            rainheatseed[i][j] = waterheat;
                            raindir[i][j] = 1;
                            raintick[i][j] = 1;
                        }
                        crount--;
                    }
                }
            }
        }
    }

    // Now easterly.

    for (int j = 0; j <= height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            if (world.wind(i, j) < 0 || world.wind(i, j) == 99)
            {
                int currentwind = world.wind(i, j);

                if (currentwind == 99)
                    currentwind = -4;

                if (world.nom(i, j) > sealevel && world.nom(i + 1, j) <= sealevel) // If this is a coastal tile
                {
                    int crount = 0 - (currentwind * seamult);
                    int waterlog = 0; // This is the amount of water being carried.
                    float waterheat = 0.0f; // This is the amount of extra heat being carried.
                    bool seaice = 0; // This will be 1 if any seasonal sea ice is found.

                    // First we pick up water from the sea to the east.

                    while (crount > 0)
                    {
                        int ii = i + crount;

                        if (ii > width)
                            ii = wrap(ii, width);

                        if (world.nom(ii, j) <= sealevel && world.seaice(ii, j) != 2) // It picks up moisture from unfrozen sea
                        {
                            if (world.seaice(ii, j) == 1)
                                seaice = 1;

                            float temp = (float)((world.maxtemp(ii, j) + world.mintemp(ii, j)) / 2);
                            temp = temp / tempfactor; // Less water is picked up from colder oceans.

                            if (temp < mintemp)
                                temp = mintemp;

                            temp = (temp / 100.0f) + 1.0f;

                            waterlog = waterlog + (int)((float)pickuprate * temp);
                            waterheat = waterheat + heatpickuprate;

                            if (seaice == 1)
                                waterlog = waterlog / 2;

                            if (waterheat > maxwinterheatfactor)
                                waterheat = maxwinterheatfactor;

                            rainseed[i][j] = waterlog;
                            rainheatseed[i][j] = waterheat;
                            raindir[i][j] = -1;
                            raintick[i][j] = 1;

                        }
                        crount--;
                    }
                }
            }
        }
    }

    // Now blur those seeds.

    int dist = 4;

    for (int i = 0; i <= width; i++)
    {
        int iminus = i - 1;

        if (iminus < 0)
            iminus = width;

        int iplus = i + 1;

        if (iplus > width)
            iplus = 0;

        for (int j = 0; j <= height; j++)
        {
            if (rainseed[i][j] != 0)
            {
                int crount = 0;
                short dir = 0;
                int raintotal = 0;
                float heattotal = 0;

                for (int k = i - dist; k <= i + dist; k++)
                {
                    int kk = k;

                    if (kk<0 || k>width)
                        kk = wrap(kk, width);

                    int kkminus = kk - 1;

                    if (kkminus < 0)
                        kkminus = width;

                    int kkplus = kk + 1;

                    if (kkplus > width)
                        kkplus = 0;

                    for (int l = j - dist; l <= j + dist; l++)
                    {
                        if (l >= 0 && l <= height)
                        {
                            if (world.nom(kk, j) > sealevel && (world.nom(kkminus, j) <= sealevel || world.nom(kkplus, j) <= sealevel))
                            {
                                crount++;
                                raintotal = raintotal + rainseed[kk][l];
                                heattotal = heattotal + rainheatseed[kk][l];

                                if (raindir[kk][l] != 0)
                                    dir = raindir[kk][l];

                            }
                        }
                    }
                }

                if (raintotal > 0)
                    rainseed[i][j] = raintotal / crount;

                if (heattotal > 0)
                    rainheatseed[i][j] = heattotal / crount;

                if (dir != 0)
                {
                    raindir[i][j] = dir;
                    raintick[i][j] = 1;
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

        for (int x = 0; x <= width; x++)
        {
            for (int y = 0; y <= height; y++)
            {                
                if (raintick[x][y] == thistick && world.sea(x, y) == 0)
                {
                    found = 1;

                    float waterlog = (float)rainseed[x][y];
                    float waterheat = rainheatseed[x][y];

                    short dir = raindir[x][y];

                    inland[x][y] = thistick; // This is how far we are from the sea!

                    // First, work out how much water to dump.

                    float waterdumped = waterlog / (float)dumprate;
                    float noslopewaterdumped = waterdumped;

                    float slope = 0.0f;

                    if (world.nom(x, y) - sealevel > slopemin) // If it's going uphill, increase the amount of water being dumped.
                    {
                        slope = (float)getslope(world, x - dir, y, x, y);
                        slope = slope / slopefactor;

                        if (slope > 1) // If it's going uphill
                        {
                            waterdumped = waterdumped * slope;
                            float elevation = (float)(world.nom(x, y) - sealevel);
                            waterdumped = waterdumped * elevationfactor * elevation;

                            if (waterdumped > waterlog)
                                waterdumped = waterlog;
                        }

                        // If this is a crater rim, reduce its height.

                        if (world.craterrim(x, y) > 0)
                        {
                            float rimelev = (float)world.craterrim(x, y);

                            rimelev = rimelev - waterlog * 10.0f; // 1.2f; // This will reduce crater rims quite drastically in areas of high rainfall, which means they'll only be clearly visible in deserts - which is what we want.

                            if (rimelev < 0.0f)
                                rimelev = 0.0f;

                            world.setcraterrim(x, y, (int)rimelev);
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

                    for (int i = x - splashsize; i <= x + splashsize; i++)
                    {
                        int ii = i;

                        if (ii<0 || ii>width)
                            ii = wrap(ii, width);

                        for (int j = y - splashsize; j <= y + splashsize; j++)
                        {
                            if (j >= 0 && j <= height)
                            {
                                bool splashok = 1;

                                if (world.map(ii, j) < world.map(x, y)) // Don't splash onto lower ground that's ahead.
                                {
                                    if (dir == 1 && i > x)
                                        splashok = 0;

                                    if (dir == -1 && i < x)
                                        splashok = 0;
                                }

                                if (slope > 1)
                                {
                                    float slopewater = waterdumped - noslopewaterdumped;
                                    waterdumped = noslopewaterdumped + slopewater / slopewaterreduce;
                                }

                                if (splashok == 1)
                                {
                                    if (world.summerrain(ii, j) < (int)(waterdumped / 2.0f))
                                        world.setsummerrain(ii, j, (int)(waterdumped / 2.0f));

                                    if (world.winterrain(ii, j) < (int)(waterdumped / 2.0f))
                                        world.setwinterrain(ii, j, (int)(waterdumped / 2.0f));

                                    if (j<world.horse(ii, 1) || j>world.horse(ii, 4))
                                    {
                                        world.setmintemp(ii, j, world.mintemp(ii, j) + (int)waterheat);
                                        world.setmaxtemp(ii, j, world.maxtemp(ii, j) + (int)(waterheat * summerfactor));
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

                    int xx = x + dir;

                    if (xx<0 || xx>width)
                        xx = wrap(xx, width);

                    int yy = y;

                    if (random(1, swervechance) == 1)
                        yy = yy + randomsign(1);

                    if (yy < 0)
                        yy = 0;

                    if (yy > height)
                        yy = height;

                    if (world.sea(xx, yy) == 0)
                    {
                        rainseed[xx][yy] = (int)waterlog;
                        rainheatseed[xx][yy] = waterheat;
                        raindir[xx][yy] = dir;
                        raintick[xx][yy] = thistick + 1;
                    }

                    // Now maybe create new seeds to the north or south.

                    for (int yy = y + 1; yy >= y - 1; yy = yy - 2)
                    {
                        if (yy >= 0 && yy <= height && world.sea(xx, yy) == 0)
                        {
                            int thisspreadchance = spreadchance;

                            bool horse = 0;

                            if (world.wind(xx, yy) == 99 || world.wind(xx, yy) == 101 || world.wind(xx, yy) == 0)
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

                                int newwaterlog = (int)fwaterlog; //fwaterlog;

                                if (rainseed[x][yy] < newwaterlog && rainseed[xx][yy] < newwaterlog)
                                {
                                    float waterdumped = (float)newwaterlog / (float)dumprate;
                                    float noslopewaterdumped = waterdumped;

                                    if (world.nom(xx, yy) - sealevel > slopemin)
                                    {
                                        float slope = (float)getslope(world, x, y, xx, yy);
                                        slope = slope / slopefactor;

                                        if (slope > 1.0f) // If it's going uphill
                                        {
                                            float waterdumped2 = waterdumped * slope;
                                            float elevation = (float)(world.nom(xx, yy) - sealevel);
                                            waterdumped2 = waterdumped2 * elevationfactor * elevation;
                                            waterdumped = waterdumped2;

                                            if (waterdumped > waterlog)
                                                waterdumped = waterlog;
                                        }
                                    }

                                    waterlog = waterlog - waterdumped;

                                    if (slope > 1)
                                    {
                                        float slopewater = waterdumped - noslopewaterdumped;
                                        waterdumped = noslopewaterdumped + slopewater / (float)slopewaterreduce;
                                    }

                                    if (world.summerrain(xx, yy) < (int)(waterdumped / 2.0f))
                                        world.setsummerrain(xx, yy, (int)(waterdumped / 2.0f));

                                    if (world.winterrain(xx, yy) < (int)(waterdumped / 2.0f))
                                        world.setwinterrain(xx, yy, (int)(waterdumped / 2.0f));

                                    if (yy<world.horse(xx, 1) || yy>world.horse(xx, 4))
                                    {
                                        world.setmintemp(xx, yy, world.mintemp(xx, yy) + (int)waterheat);
                                        world.setmaxtemp(xx, yy, world.maxtemp(xx, yy) + (int)(waterheat * summerfactor));
                                    }

                                    raindir[xx][yy] = dir;
                                    raintick[xx][yy] = thistick + 1;
                                    rainseed[xx][yy] = newwaterlog;
                                    rainheatseed[xx][yy] = waterheat;
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

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (world.nom(i, j) > sealevel)
            {
                float tempdiff = (float)(world.maxtemp(i, j) - world.mintemp(i, j));

                if (tempdiff < 0.0f)
                    tempdiff = 0.0f;

                float multfactor = 0.03f;
                int distance = inland[i][j];

                if (distance > maxseasonaldistance)
                    distance = maxseasonaldistance;

                float landdistvar = (float)distance * multfactor; // The further from land, the greater the seasonal variation

                if (landdistvar < 1.0f)
                    landdistvar = 1.0f;

                float winterrain = (float)world.winterrain(i, j);

                if (j<world.horse(i, 2) || j>world.horse(i, 3))
                    tempdiff = tempdiff * seasonalvar * winterrain;
                else
                    tempdiff = tempdiff * tropicalseasonalvar * winterrain;

                tempdiff = tempdiff * landdistvar;

                world.setwinterrain(i, j, world.winterrain(i, j) + (int)tempdiff);
                world.setsummerrain(i, j, world.summerrain(i, j) - (int)tempdiff);

                if (world.summerrain(i, j) < 0)
                    world.setsummerrain(i, j, 0);
            }
        }
    }

    // Now increase the amounts.

    vector<vector<int>> rainadd(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0)); // This is an amount of rain to add to areas with a roughly temperate/subtropical heat range.

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            float tempdiff = (float)abs(world.mintemp(i, j) + world.maxtemp(i, j) - 30); // 30

            tempdiff = 20.0f - tempdiff; // 15

            if (tempdiff < 0.0f)
                tempdiff = 0.0f;

            int thisrainadd = (int)(tempdiff * 4.0f);

            float coastal = (float)(250 - inland[i][j]);

            if (coastal < 1.0f)
                coastal = 1.0f;

            coastal = coastal / 250.0f;

            thisrainadd = (int)((float)thisrainadd * coastal);

            rainadd[i][j] = thisrainadd;
        }
    }

    short span = 2;

    for (int i = 0; i <= width; i++) // Blur this.
    {
        for (int j = 0; j <= height; j++)
        {
            if (world.sea(i, j) == 0)
            {
                int crount = 0;
                int total = 0;

                for (int k = i - span; k <= i + span; k++)
                {
                    int kk = k;

                    if (kk<0 || kk>width)
                        kk = wrap(kk, width);

                    for (int l = j - span; l <= j + span; l++)
                    {
                        if (l >= 0 && l <= height)
                        {
                            crount++;
                            total = total + rainadd[kk][l];
                        }

                    }
                }

                if (crount > 0)
                    rainadd[i][j] = total / crount;
            }
        }
    }

    for (int i = 0; i <= width; i++) // Now add it, together with a general rainfall increase.
    {
        for (int j = 0; j <= height; j++)
        {
            float summerrain = (float)world.summerrain(i, j);
            float winterrain = (float)world.winterrain(i, j);

            summerrain = summerrain * 1.20f + (float)rainadd[i][j] / 3.0f; // 1.3
            winterrain = winterrain * 1.20f + (float)rainadd[i][j];

            world.setsummerrain(i, j, (int)summerrain);
            world.setwinterrain(i, j, (int)winterrain);
        }
    }

    // Possible blurring here

    short blurdist = 1;

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            short winddir = 0; // This is to avoid blurring rain over rain shadows.

            if (world.wind(i, j) > 0 && world.wind(i, j) != 99)
                winddir = 1; // Westerly

            if (world.wind(i, j) < 0 || world.wind(i, j) == 99)
                winddir = -1; // Easterly

            short crount = 0;

            int summertotal = 0;
            int wintertotal = 0;

            for (int k = i - blurdist; k <= i + blurdist; k++)
            {
                int kk = k;

                if (kk<0 || kk>width)
                    kk = wrap(kk, width);

                for (int l = j - blurdist; l <= j + blurdist; l++)
                {
                    if (l > 0 && l < height)
                    {
                        bool goahead = 1;

                        if (world.mountainheight(kk, l) > maxmountainheight)
                            goahead = 0;

                        if (winddir == 1)
                        {
                            if (k<i && world.map(kk, l)>world.map(i, j))
                                goahead = 0;

                            if (k > i && world.map(kk, l) < world.map(i, j))
                                goahead = 0;
                        }

                        if (winddir == -1)
                        {
                            if (k > i && world.map(kk, l) > world.map(i, j))
                                goahead = 0;

                            if (k < i && world.map(kk, l) < world.map(i, j))
                                goahead = 0;
                        }

                        if (goahead == 1)
                        {
                            summertotal = summertotal + world.summerrain(kk, l);
                            wintertotal = wintertotal + world.winterrain(kk, l);
                            crount++;
                        }
                    }
                }
            }

            if (crount > 0)
            {
                world.setsummerrain(i, j, summertotal / crount);
                world.setwinterrain(i, j, wintertotal / crount);
            }
        }
    }
}

// This adds a bit of rainfall to worlds with no sea.

void createdesertworldrain(planet& world)
{
    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();
    float maxelev = (float)world.maxelevation();

    float slopefactor = 130.0f;
    float idealtemp = 20.0f; // The closer it is to this temperature, the more rain there is.
    float maxdiff = 40.0f; // If it's more than this hotter/colder than the ideal temperature, there will be no rain.

    float maxrain = (float)(random(20, 200));

    int grain = 8; // Level of detail on this fractal map.
    float valuemod = 0.2f;
    int v = random(1, 4);
    float valuemod2 = float(v);

    vector<vector<int>> fractal(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

    createfractal(fractal, width, height, grain, valuemod, valuemod2, 1, (int)maxelev, 0, 0);

    int warpfactor = 60;
    warp(fractal, width, height, (int)maxelev, warpfactor, 0);

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            int elev = world.map(i, j);

            int biggestdiff = 0;

            for (int k = i - 1; k <= i + 1; k++)
            {
                int kk = k;
                if (kk<0 || k>width)
                    kk = wrap(kk, width);

                for (int l = j - 1; l <= j + 1; l++)
                {
                    if (l >= 0 && l <= height)
                    {
                        int thisdiff = elev - world.map(kk, l);

                        if (thisdiff > biggestdiff)
                            biggestdiff = thisdiff;
                    }
                }
            }

            float thisslopemult = (float)biggestdiff / slopefactor;

            float thisfractalmult = (float)fractal[i][j] / maxelev;

            float thisrain = maxrain * thisslopemult * thisfractalmult;

            world.setjanrain(i, j, (int)thisrain);
            world.setjulrain(i, j, (int)thisrain);
        }
    }

    // Now adjust for seasonal variation.

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            float oldjanrain = (float)world.janrain(i, j);
            float oldjulrain = (float)world.julrain(i, j);
            
            float jandiff = (float)world.jantemp(i, j) - idealtemp;
            float juldiff = (float)world.jultemp(i, j) - idealtemp;

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
            
            world.setjanrain(i, j, (int)newjanrain);
            world.setjulrain(i, j, (int)newjulrain);
        }
    }

    // Now blur.

    int dist = 1;

    vector<vector<int>> finaljanrain(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));
    vector<vector<int>> finaljulrain(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            int crount = 0;
            int jantotal = 0;
            int jultotal = 0;

            for (int k = i - dist; k <= i + dist; k++)
            {
                int kk = k;
                if (kk<0 || k>width)
                    kk = wrap(kk, width);

                for (int l = j - dist; l <= j + dist; l++)
                {
                    if (l >= 0 && l <= height)
                    {
                        jantotal = jantotal + world.janrain(i, j);
                        jultotal = jultotal + world.julrain(i, j);

                        crount++;
                    }
                }
            }

            float newjanrain = (float)jantotal / (float)crount;
            float newjulrain = (float)jultotal / (float)crount;

            finaljanrain[i][j] = (int)newjanrain;
            finaljulrain[i][j] = (int)newjulrain;
        }
    }

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            world.setjanrain(i, j, finaljanrain[i][j]);
            world.setjulrain(i, j, finaljulrain[i][j]);
        }
    }
}

// This creates monsoons.

void createmonsoons(planet& world, int maxmountainheight, int slopewaterreduce)
{
    //highres_timer_t timer("Create Monsoons"); // 666: 7753 / 999: 6658 => 666: 6311, 999: 5189
    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();
    int maxelev = world.maxelevation();
    float tempdecrease = world.tempdecrease() * 10.0f; // The default is 6.5 * 10.0f = 65.0.

    float monstrength = 2.0f - (float)(abs(32.5f - world.tilt()) / 10.0f); // Overall strength of monsoons is affected by axial tilt. High with medium tilt, low with low or high tilt.

    if (monstrength > 0.0)
    {
        int minmonsoondiff = 2; // Winter/summer temperatures must be this far apart for monsoons to happen.
        int minmonsoontemp = 15; // Average annual temperature must be at least this for monsoons to happen.
        float monsoondifffactor = 1.0f; //5.0f; // For every degree of difference between summer and winter temperatures, add this to get the base monsoon strength.
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

        vector<vector<int>> raintick(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));
        vector<vector<short>> raindir(ARRAYWIDTH, vector<short>(ARRAYHEIGHT, 0));
        vector<vector<float>> monsoonstrength(ARRAYWIDTH, vector<float>(ARRAYHEIGHT, 0));
        vector<vector<int>> monsoonmap(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

        // precompute and cache the value of world.sea()
        vector<vector<uint8_t>> is_sea(ARRAYWIDTH, vector<uint8_t>(ARRAYHEIGHT));

        for (int i = 0; i <= width; i++)
        {
            for (int j = 0; j <= height; j++)
            {
                is_sea[i][j] = world.sea(i, j);
            }
        }

        // First, set the initial seeds along the coasts.

        for (int i = 0; i <= width; i++)
        {
            for (int j = 0; j <= height; j++)
            {
                if (j<height / 2 - monsoonminequatordist || j>height / 2 + monsoonminequatordist)
                {
                    if (is_sea[i][j] == 1 && j >= world.horse(i, 2) && j <= world.horse(i, 3))
                    {
                        bool found = 0;

                        for (int k = i - 1; k <= i + 1; k++)
                        {
                            int kk = k;

                            if (kk<0 || kk>width)
                                kk = wrap(kk, width);

                            for (int l = j - 1; l <= j + 1; l++)
                            {
                                if (l >= 0 && l <= height)
                                {
                                    if (is_sea[kk][l] == 0)
                                    {
                                        found = 1;
                                        k = i + 1;
                                        l = j + 1;
                                    }
                                }
                            }
                        }

                        if (found == 1)
                        {
                            int avetemp = (world.maxtemp(i, j) + world.mintemp(i, j)) / 2;
                            int tempdiff = world.maxtemp(i, j) - world.mintemp(i, j);

                            if (avetemp >= minmonsoontemp && tempdiff >= minmonsoondiff)
                            {
                                float monsoon = tempdiff * monsoondifffactor;
                                monsoon = monsoon + (avetemp * monsoontempfactor);

                                float tidefactor = (float)world.tide(i, j);
                                tidefactor = tidefactor / 10.0f;

                                if (tidefactor < mintidefactor)
                                    tidefactor = mintidefactor;

                                monsoon = monsoon * tidefactor;

                                monsoonstrength[i][j] = monsoon;
                                raintick[i][j] = 1;
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
            for (int a = 0; a <= width; a++)
            {
                for (int b = 0; b <= height; b++)
                {
                    int x = a;
                    int y = b; // This is so we can move them later without messing up the loop.

                    if (is_sea[x][y] == 0 && raintick[x][y] == 0) // && random(1,maxelev)<spreadfractal[x][y])
                    {
                        bool nearby = 0;

                        for (int i = x - 1; i <= x + 1; i++)
                        {
                            int ii = i;

                            if (ii<0 || ii>width)
                                ii = wrap(ii, width);

                            for (int j = y - 1; j <= y + 1; j++)
                            {
                                if (j >= 0 && j <= height)
                                {
                                    if (raintick[ii][j] == thistick)
                                    {
                                        nearby = 1;
                                        i = x + 1;
                                        j = y + 1;
                                    }
                                }
                            }
                        }

                        if (nearby == 1) // This is a cell next to a monsoon cell!
                        {
                            found = 1;

                            raintick[x][y] = thistick + 1; // This will be a seed for the next round.

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

                            int leftx = x - 1;
                            int rightx = x + 1;
                            int upy = y - 1;
                            int downy = y + 1;

                            if (leftx < 0)
                                leftx = width;

                            if (rightx > width)
                                rightx = 0;

                            if (upy < 0)
                                upy = 0;

                            if (downy > height)
                                downy = height;

                            if (raintick[x][upy] == thistick)
                            {
                                if (monsoonstrength[x][upy] > 0)
                                {
                                    northwest++;
                                    north = north + 2;
                                    northeast++;
                                }

                                crount++;
                                total = total + monsoonstrength[x][upy];
                            }

                            if (raintick[rightx][upy] == thistick)
                            {
                                if (monsoonstrength[rightx][upy] > 0)
                                {
                                    north++;
                                    northeast = northeast + 2;
                                    east++;
                                }

                                crount++;
                                total = total + monsoonstrength[rightx][upy];
                            }

                            if (raintick[rightx][y] == thistick)
                            {
                                if (monsoonstrength[rightx][y] > 0)
                                {
                                    northeast++;
                                    east = east + 2;
                                    southeast++;
                                }

                                crount++;
                                total = total + monsoonstrength[rightx][y];
                            }

                            if (raintick[rightx][downy] == thistick)
                            {
                                if (monsoonstrength[rightx][downy] > 0)
                                {
                                    east++;
                                    southeast = southeast + 2;
                                    south++;
                                }

                                crount++;
                                total = total + monsoonstrength[rightx][downy];
                            }

                            if (raintick[x][downy] == thistick)
                            {
                                if (monsoonstrength[x][downy] > 0)
                                {
                                    southeast++;
                                    south = south + 2;
                                    southwest++;
                                }

                                crount++;
                                total = total + monsoonstrength[x][downy];
                            }

                            if (raintick[leftx][downy] == thistick)
                            {
                                if (monsoonstrength[leftx][downy] > 0)
                                {
                                    south++;
                                    southwest = southwest + 2;
                                    west++;
                                }

                                crount++;
                                total = total + monsoonstrength[leftx][downy];
                            }

                            if (raintick[leftx][y] == thistick)
                            {
                                if (monsoonstrength[leftx][y] > 0)
                                {
                                    southwest++;
                                    west = west + 2;
                                    northwest++;
                                }

                                crount++;
                                total = total + monsoonstrength[leftx][y];
                            }

                            if (raintick[leftx][upy] == thistick)
                            {
                                if (monsoonstrength[leftx][upy] > 0)
                                {
                                    west++;
                                    northwest = northwest + 2;
                                    north++;
                                }

                                crount++;
                                total = total + monsoonstrength[leftx][upy];
                            }

                            waterlog = total / crount;
                            waterlog = waterlog * monsoonincrease;

                            float tempadjust = (float)world.avetemp(x, y);
                            tempadjust = tempadjust / monsoononinlandtempfactor;

                            if (tempadjust > 1.0f)
                                tempadjust = 1.0f;

                            waterlog = waterlog * tempadjust;

                            int equatordist = abs(height / 2 - y);

                            if (equatordist > monsoonminequatordist)
                            {
                                if (equatordist < monsoonequatordist)
                                {
                                    if (random(1, monsoonequatordist) > equatordist)
                                        waterlog = waterlog * monsoonequatorfactor;
                                }

                                tempadjust = (float)world.avetemp(x, y);

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

                                if (thistick > 2 && world.nom(x, y) - sealevel > monsoonslopemin)
                                {
                                    int upwindx = x;
                                    int upwindy = y;

                                    if (dir == 8 || dir == 1 || dir == 2)
                                        upwindy++;

                                    if (dir == 4 || dir == 5 || dir == 6)
                                        upwindy--;

                                    if (dir == 2 || dir == 3 || dir == 4)
                                        upwindx--;

                                    if (dir == 6 || dir == 7 || dir == 8)
                                        upwindx++;

                                    if (upwindx < 0)
                                        upwindx = width;

                                    if (upwindx > width)
                                        upwindx = 0;

                                    if (upwindy < 0)
                                        upwindy = 0;

                                    if (upwindy > height)
                                        upwindy = height;

                                    slope = (float)getslope(world, upwindx, upwindy, x, y);
                                    slope = slope / monsoonslopefactor;

                                    if (slope > 1.0f) // If it's going uphill
                                    {
                                        float waterdumped2 = waterdumped * slope;
                                        float elevation = (float)(world.nom(x, y) - sealevel);
                                        waterdumped2 = waterdumped2 * monsoonelevationfactor * elevation;
                                        waterdumped = (int)waterdumped2;

                                        if (waterdumped > (int)waterlog)
                                            waterdumped = (int)waterlog;
                                    }
                                }

                                waterlog = waterlog - (float)waterdumped;

                                if (random(1, monsoonswervechance) == 1)
                                {
                                    raintick[x][y] = 0;

                                    if (dir == 1 || dir == 5)
                                    {
                                        if (random(1, 2) == 1)
                                            x--;
                                        else
                                            x++;
                                    }

                                    if (dir == 2 || dir == 6)
                                    {
                                        if (random(1, 2) == 1)
                                        {
                                            x--;
                                            y--;
                                        }
                                        else
                                        {
                                            x++;
                                            y++;
                                        }
                                    }

                                    if (dir == 3 || dir == 7)
                                    {
                                        if (random(1, 2) == 1)
                                            y--;
                                        else
                                            y++;
                                    }

                                    if (dir == 4 || dir == 8)
                                    {
                                        if (random(1, 2) == 1)
                                        {
                                            x--;
                                            y++;
                                        }
                                        else
                                        {
                                            x++;
                                            y--;
                                        }
                                    }

                                    if (x < 0)
                                        x = width;

                                    if (x > width)
                                        x = 0;

                                    if (y < 0)
                                        y = 0;

                                    if (y > height)
                                        y = height;

                                    raintick[x][y] = thistick + 1;
                                }

                                if (slope > 0.0f)
                                {
                                    int slopewater = waterdumped - noslopewaterdumped;
                                    waterdumped = waterdumped + (int)((float)slopewater / ((float)slopewaterreduce * 5.0f));
                                }

                                if (waterdumped > monsoonmap[x][y])
                                    monsoonmap[x][y] = waterdumped;

                                monsoonstrength[x][y] = waterlog;
                            }
                            else
                                monsoonmap[x][y] = 0;
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
            for (int i = 0; i <= width; i++) // Normal blur.
            {
                int im = i - 1;
                int imm = i - 2;
                int ip = i + 1;
                int ipp = i + 2;

                if (im < 0)
                    im = width;

                if (imm < 0)
                    imm = wrap(imm, width);

                if (ip > width)
                    ip = 0;

                if (ipp > width)
                    ipp = wrap(ipp, width);

                for (int j = 0; j <= height; j++)
                {
                    if (is_sea[i][j] == 1)
                        monsoonmap[i][j] = 0;
                    else
                    {
                        if (world.mountainheight(i, j) < maxmountainheight)
                        {
                            float crount = 0;
                            float total = 0;

                            for (int k = i - 1; k <= i + 1; k++) // First, check the cells bordering the central one.
                            {
                                int kk = k;

                                if (kk<0 || kk>width)
                                    kk = wrap(kk, width);

                                for (int l = j - 1; l <= j + 1; l++)
                                {
                                    if (l >= 0 && l <= height)
                                    {
                                        if (world.mountainheight(kk, l) < maxmountainheight)
                                        {
                                            total = total + monsoonmap[kk][l];
                                            crount++;
                                        }

                                    }
                                }
                            }

                            int jm = j - 1;
                            int jmm = j - 2;
                            int jp = j + 1;
                            int jpp = j + 2;

                            if (jmm >= 0 && jpp <= height) // Now, check the cells bordering those ones.
                            {
                                // Cells to the north

                                if (world.mountainheight(im, jmm) < maxmountainheight && (world.mountainheight(im, jm) < maxmountainheight || world.mountainheight(i, jm) < maxmountainheight))
                                {
                                    total = total + monsoonmap[im][jmm];
                                    crount++;
                                }

                                if (world.mountainheight(i, jmm) < maxmountainheight && (world.mountainheight(im, jm) < maxmountainheight || world.mountainheight(i, jm) < maxmountainheight || world.mountainheight(ip, jm) < maxmountainheight))
                                {
                                    total = total + monsoonmap[i][jmm];
                                    crount++;
                                }

                                if (world.mountainheight(ip, jmm) < maxmountainheight && (world.mountainheight(i, jm) < maxmountainheight || world.mountainheight(ip, jm) < maxmountainheight))
                                {
                                    total = total + monsoonmap[ip][jmm];
                                    crount++;
                                }

                                // Cells to the south

                                if (world.mountainheight(im, jpp) < maxmountainheight && (world.mountainheight(im, jp) < maxmountainheight || world.mountainheight(i, jp) < maxmountainheight))
                                {
                                    total = total + monsoonmap[im][jpp];
                                    crount++;
                                }

                                if (world.mountainheight(i, jpp) < maxmountainheight && (world.mountainheight(im, jp) < maxmountainheight || world.mountainheight(i, jp) < maxmountainheight || world.mountainheight(ip, jp) < maxmountainheight))
                                {
                                    total = total + monsoonmap[i][jpp];
                                    crount++;
                                }

                                if (world.mountainheight(ip, jpp) < maxmountainheight && (world.mountainheight(i, jp) < maxmountainheight || world.mountainheight(ip, jp) < maxmountainheight))
                                {
                                    total = total + monsoonmap[ip][jpp];
                                    crount++;
                                }

                                // Cells to the west

                                if (world.mountainheight(imm, jm) < maxmountainheight && (world.mountainheight(im, jm) < maxmountainheight || world.mountainheight(im, j) < maxmountainheight))
                                {
                                    total = total + monsoonmap[imm][jm];
                                    crount++;
                                }

                                if (world.mountainheight(imm, j) < maxmountainheight && (world.mountainheight(im, jm) < maxmountainheight || world.mountainheight(im, j) < maxmountainheight || world.mountainheight(im, jp) < maxmountainheight))
                                {
                                    total = total + monsoonmap[imm][j];
                                    crount++;
                                }

                                if (world.mountainheight(imm, jp) < maxmountainheight && (world.mountainheight(im, j) < maxmountainheight || world.mountainheight(im, jp) < maxmountainheight))
                                {
                                    total = total + monsoonmap[imm][jp];
                                    crount++;
                                }

                                // Cells to the east

                                if (world.mountainheight(ipp, jm) < maxmountainheight && (world.mountainheight(ip, jm) < maxmountainheight || world.mountainheight(ip, j) < maxmountainheight))
                                {
                                    total = total + monsoonmap[ipp][jm];
                                    crount++;
                                }

                                if (world.mountainheight(ipp, j) < maxmountainheight && (world.mountainheight(ip, jm) < maxmountainheight || world.mountainheight(ip, j) < maxmountainheight || world.mountainheight(ip, jp) < maxmountainheight))
                                {
                                    total = total + monsoonmap[ipp][j];
                                    crount++;
                                }

                                if (world.mountainheight(ipp, jp) < maxmountainheight && (world.mountainheight(ip, j) < maxmountainheight || world.mountainheight(ip, jp) < maxmountainheight))
                                {
                                    total = total + monsoonmap[ipp][jp];
                                    crount++;
                                }
                            }

                            if (crount > 0)
                            {
                                total = total / crount;

                                monsoonmap[i][j] = (int)total;
                            }
                        }
                    }
                }
            }
        }

        // Now we need to use the monsoon map to tinker with the rainfall map.

        float monsoonelevfactor = 8.0f; // The higher this is, the more elevation will reduce the extra monsoon rain in summer.

        vector<vector<int>> summermonsoon(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));
        vector<vector<int>> wintermonsoon(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

        for (int i = 0; i <= width; i++)
        {
            for (int j = 0; j <= height; j++)
            {
                if (monsoonmap[i][j] > 0)
                {
                    float landelev = (float)(world.nom(i, j) - sealevel);

                    landelev = landelev * monsoonelevfactor;

                    monsoonmap[i][j] = monsoonmap[i][j] - (int)landelev;

                    if (monsoonmap[i][j] < 0)
                        monsoonmap[i][j] = 0;
                }
            }
        }

        for (int i = 0; i <= width; i++) // Make arrays of the new monsoon rainfall.
        {
            for (int j = 0; j <= height; j++)
            {
                float monsoonstrength = (float)monsoonmap[i][j];

                if (monsoonstrength != 0.0f && monsoonstrength != -1.0f)
                {
                    wintermonsoon[i][j] = (int)(monsoonstrength * wintermonsoonfactor);
                    summermonsoon[i][j] = (int)(monsoonstrength * summermonsoonfactor);

                    if (summermonsoon[i][j] > (int)maxsummerrain)
                        summermonsoon[i][j] = (int)maxsummerrain;
                }
            }
        }

        // Now just blur that monsoon rainfall too.

        for (int n = 0; n < 20; n++)
        {
            for (int i = 0; i <= width; i++) // Normal blur.
            {
                int im = i - 1;
                int imm = i - 2;
                int ip = i + 1;
                int ipp = i + 2;

                if (im < 0)
                    im = width;

                if (imm < 0)
                    imm = wrap(imm, width);

                if (ip > width)
                    ip = 0;

                if (ipp > width)
                    ipp = wrap(ipp, width);

                for (int j = 0; j <= height; j++)
                {
                    if (is_sea[i][j] == 0)
                    {
                        if (world.mountainheight(i, j) < maxmountainheight)
                        {
                            float crount = 0;
                            float summertotal = 0;

                            for (int k = i - 1; k <= i + 1; k++) // First, check the cells bordering the central one.
                            {
                                int kk = k;

                                if (kk<0 || kk>width)
                                    kk = wrap(kk, width);

                                for (int l = j - 1; l <= j + 1; l++)
                                {
                                    if (l >= 0 && l <= height)
                                    {
                                        if (is_sea[kk][l] == 0 && world.mountainheight(kk, l) < maxmountainheight)
                                        {
                                            summertotal = summertotal + summermonsoon[kk][l];
                                            crount++;
                                        }

                                    }
                                }
                            }

                            int jm = j - 1;
                            int jmm = j - 2;
                            int jp = j + 1;
                            int jpp = j + 2;

                            if (jmm >= 0 && jpp <= height) // Now, check the cells bordering those ones.
                            {
                                // Cells to the north

                                if (world.sea(im, jmm) == 0 && world.mountainheight(im, jmm) < maxmountainheight && (world.mountainheight(im, jm) < maxmountainheight || world.mountainheight(i, jm) < maxmountainheight))
                                {
                                    summertotal = summertotal + summermonsoon[im][jmm];
                                    crount++;
                                }

                                if (world.sea(i, jmm) == 0 && world.mountainheight(i, jmm) < maxmountainheight && (world.mountainheight(im, jm) < maxmountainheight || world.mountainheight(i, jm) < maxmountainheight || world.mountainheight(ip, jm) < maxmountainheight))
                                {
                                    summertotal = summertotal + summermonsoon[i][jmm];
                                    crount++;
                                }

                                if (world.sea(ip, jmm) == 0 && world.mountainheight(ip, jmm) < maxmountainheight && (world.mountainheight(i, jm) < maxmountainheight || world.mountainheight(ip, jm) < maxmountainheight))
                                {
                                    summertotal = summertotal + summermonsoon[ip][jmm];
                                    crount++;
                                }

                                // Cells to the south

                                if (world.sea(im, jpp) == 0 && world.mountainheight(im, jpp) < maxmountainheight && (world.mountainheight(im, jp) < maxmountainheight || world.mountainheight(i, jp) < maxmountainheight))
                                {
                                    summertotal = summertotal + summermonsoon[im][jpp];
                                    crount++;
                                }

                                if (world.sea(i, jpp) == 0 && world.mountainheight(i, jpp) < maxmountainheight && (world.mountainheight(im, jp) < maxmountainheight || world.mountainheight(i, jp) < maxmountainheight || world.mountainheight(ip, jp) < maxmountainheight))
                                {
                                    summertotal = summertotal + summermonsoon[i][jpp];
                                    crount++;
                                }

                                if (world.sea(ip, jpp) == 0 && world.mountainheight(ip, jpp) < maxmountainheight && (world.mountainheight(i, jp) < maxmountainheight || world.mountainheight(ip, jp) < maxmountainheight))
                                {
                                    summertotal = summertotal + summermonsoon[ip][jpp];
                                    crount++;
                                }

                                // Cells to the west

                                if (world.sea(imm, jm) == 0 && world.mountainheight(imm, jm) < maxmountainheight && (world.mountainheight(im, jm) < maxmountainheight || world.mountainheight(im, j) < maxmountainheight))
                                {
                                    summertotal = summertotal + summermonsoon[imm][jm];
                                    crount++;
                                }

                                if (world.sea(imm, j) == 0 && world.mountainheight(imm, j) < maxmountainheight && (world.mountainheight(im, jm) < maxmountainheight || world.mountainheight(im, j) < maxmountainheight || world.mountainheight(im, jp) < maxmountainheight))
                                {
                                    summertotal = summertotal + summermonsoon[imm][j];
                                    crount++;
                                }

                                if (world.sea(imm, jp) == 0 && world.mountainheight(imm, jp) < maxmountainheight && (world.mountainheight(im, j) < maxmountainheight || world.mountainheight(im, jp) < maxmountainheight))
                                {
                                    summertotal = summertotal + summermonsoon[imm][jp];
                                    crount++;
                                }

                                // Cells to the east

                                if (world.sea(ipp, jm) == 0 && world.mountainheight(ipp, jm) < maxmountainheight && (world.mountainheight(ip, jm) < maxmountainheight || world.mountainheight(ip, j) < maxmountainheight))
                                {
                                    summertotal = summertotal + summermonsoon[ipp][jm];
                                    crount++;
                                }

                                if (world.sea(ipp, j) == 0 && world.mountainheight(ipp, j) < maxmountainheight && (world.mountainheight(ip, jm) < maxmountainheight || world.mountainheight(ip, j) < maxmountainheight || world.mountainheight(ip, jp) < maxmountainheight))
                                {
                                    summertotal = summertotal + summermonsoon[ipp][j];
                                    crount++;
                                }

                                if (world.sea(ipp, jp) == 0 && world.mountainheight(ipp, jp) < maxmountainheight && (world.mountainheight(ip, j) < maxmountainheight || world.mountainheight(ip, jp) < maxmountainheight))
                                {
                                    summertotal = summertotal + summermonsoon[ipp][jp];
                                    crount++;
                                }
                            }

                            if (crount > 0)
                            {
                                summertotal = summertotal / crount;

                                summermonsoon[i][j] = (int)summertotal;
                            }
                        }
                    }
                }
            }
        }

        // Now use a fractal to add variation to the monsoon rainfall.

        int grain = 16;
        float valuemod = 0.2f;
        int v = random(3, 6);
        float valuemod2 = (float)v;

        vector<vector<int>> monsoonvary(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

        createfractal(monsoonvary, width, height, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

        int warpfactor = random(40, 80);
        warp(monsoonvary, width, height, maxelev, warpfactor, 0);

        float monsoondiv = 1.6f / (float)maxelev;

        for (int i = 0; i <= width; i++) // Now apply that fractal.
        {
            for (int j = 0; j <= height; j++)
            {
                float summerrain = (float)summermonsoon[i][j];
                float winterrain = (float)wintermonsoon[i][j];

                if (summerrain != 0.0f || winterrain != 0.0f)
                {
                    float amount = (float)monsoonvary[i][j] * monsoondiv;

                    amount = amount - 0.8f;

                    summerrain = summerrain + summerrain * amount;
                    winterrain = winterrain + winterrain * amount;

                    summermonsoon[i][j] = (int)summerrain;
                    wintermonsoon[i][j] = (int)winterrain;
                }
            }
        }

        for (int i = 0; i <= width; i++) // Now apply that rainfall.
        {
            for (int j = 0; j <= height; j++)
            {
                float wmonsoon = (float)wintermonsoon[i][j];
                float smonsoon = (float)summermonsoon[i][j];

                wmonsoon = wmonsoon * monstrength;
                smonsoon = smonsoon * monstrength;

                int winterrain = world.winterrain(i, j) - (int)wmonsoon;

                if (winterrain < 0)
                    winterrain = 0;

                int summerrain = world.summerrain(i, j) + (int)smonsoon;

                if (summerrain > (int)maxsummerrain)
                    summerrain = (int)maxsummerrain;

                world.setwinterrain(i, j, winterrain);
                world.setsummerrain(i, j, summerrain);
            }
        }
    }
}

// This adjusts the seasonal rainfall (to ensure certain climates)

void adjustseasonalrainfall(planet& world, vector<vector<int>>& inland)
{
    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();
    int maxelev = world.maxelevation();
    float tilt = world.tilt();

    // First, do some tinkering to encourage Mediterranean climates.

    vector<vector<float>> mediterranean(ARRAYWIDTH, vector<float>(ARRAYHEIGHT, 0));

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

        for (int i = 0; i <= width; i++)
        {
            for (int j = 0; j <= height; j++)
            {
                if (world.sea(i, j) == 0)
                {
                    float med = 1.0f;

                    float diff = (float)(abs(world.maxtemp(i, j) - avemedmaxtemp));

                    med = med - diff * medmaxtempdifffactor;

                    diff = minmedcoldtemp - world.mintemp(i, j);

                    if (diff > 0.0f)
                        med = med - diff * medmintempdifffactor;

                    diff = (float)(world.averain(i, j) - maxmedrain);

                    if (diff > 0.0f)
                        med = med - diff * medmaxraindifffactor;

                    diff = (float)(inland[i][j] - maxmedinland);

                    if (diff > 0.0f)
                        med = med - diff * medmaxinlanddifffactor;

                    if (j < height / 2)
                    {
                        diff = (float)abs(world.horse(i, 1) - j);

                        if (j > world.horse(i, 1))
                            diff = diff * 2.0f;
                    }
                    else
                    {
                        diff = (float)abs(world.horse(i, 4) - j);

                        if (j < world.horse(i, 4))
                            diff = diff * 2.0f;
                    }

                    med = med - diff * medhorsedifffactor;

                    if (med > 0.0f)
                        mediterranean[i][j] = med;

                }
            }
        }

        int dist = 3;

        for (int i = 0; i <= width; i++) // Smooth it
        {
            for (int j = 0; j <= height; j++)
            {
                float total = 0;
                short crount = 0;

                for (int k = i - dist; k <= i + dist; k++)
                {
                    int kk = k;

                    if (kk<0 || kk>width)
                        kk = wrap(kk, width);

                    for (int l = j - dist; l <= j + dist; l++)
                    {
                        if (l >= 0 && l <= height)
                        {
                            total = total + mediterranean[kk][l];
                            crount++;
                        }
                    }
                }

                if (crount > 0)
                    mediterranean[i][j] = total / crount;
            }
        }

        for (int i = 0; i <= width; i++) // Now use it to alter the seasonal variation in rainfall.
        {
            for (int j = 0; j <= height; j++)
            {
                if (mediterranean[i][j] != 0)
                {
                    float winterrain = (float)world.winterrain(i, j);
                    float summerrain = (float)world.summerrain(i, j);

                    float rainshift = summerrain * medstrength * mediterranean[i][j];

                    winterrain = winterrain + rainshift;
                    summerrain = summerrain - rainshift;

                    world.setwinterrain(i, j, (int)winterrain);
                    world.setsummerrain(i, j, (int)summerrain);
                }
            }
        }
    }

    // Now some tinkering to encourage rainforest near the equator.

    float rainstrength = 2.0f - (abs(32.5f - tilt) / 10.0f); // As for medstrength above.

    if (rainstrength > 0.0f)
    {
        float equator = (float)(height / 2);

        float winteradd = 0.3f * rainstrength; // 1.0f;
        float summeradd = 0.4f * rainstrength; // 1.6f; // 0.4f;

        for (int i = 0; i <= width; i++)
        {
            float tropicnorth = (float)world.horse(i, 2);
            float tropicnorthheight = equator - tropicnorth;
            float winterstep = winteradd / tropicnorthheight;
            float summerstep = summeradd / tropicnorthheight;

            float currentwinteradd = 0.0f;
            float currentsummeradd = 0.0f;

            for (int j = (int)tropicnorth; j <= (int)equator; j++)
            {
                currentwinteradd = currentwinteradd + winterstep;
                currentsummeradd = currentsummeradd + summerstep;

                float currentjan = float(world.janrain(i, j));
                float currentjul = float(world.julrain(i, j));

                currentjan = currentjan + currentjan * currentwinteradd;
                currentjul = currentjul + currentjul * currentsummeradd;

                world.setjanrain(i, j, (int)currentjan);
                world.setjulrain(i, j, (int)currentjul);
            }

            float tropicsouth = (float)world.horse(i, 3);
            float tropicsouthheight = tropicsouth - equator;
            winterstep = winteradd / tropicsouthheight;
            summerstep = summeradd / tropicsouthheight;

            currentwinteradd = 0.0f;
            currentsummeradd = 0.0f;

            for (int j = (int)tropicsouth; j > (int)equator; j--)
            {
                currentwinteradd = currentwinteradd + winterstep;
                currentsummeradd = currentsummeradd + summerstep;

                float currentjan = float(world.janrain(i, j));
                float currentjul = float(world.julrain(i, j));

                currentjan = currentjan + currentjan * currentsummeradd;
                currentjul = currentjul + currentjul * currentwinteradd;

                world.setjanrain(i, j, (int)currentjan);
                world.setjulrain(i, j, (int)currentjul);
            }
        }

        /*

        // And now some more tinkering, to encourage savannah nearer the edge of the tropics. (Turned off now as it isn't really needed.)

        float janadd = -0.8 * rainstrength; // 0.8f;
        float juladd = 0 * rainstrength; // 1.0f; // 0.6f;

        float bandwidth = 30; // The strip affected will extend by this much to the north and south of the boundary of the horse latitudes.

        float janstep = janadd / bandwidth;
        float julstep = juladd / bandwidth;

        for (int i = 0; i <= width; i++)
        {
            float tropicnorth = (world.horse(i, 2) + equator) / 2;
            float tropicsouth = (world.horse(i, 3) + equator) / 2;

            float currentjanadd = 0;
            float currentjuladd = 0;

            for (int j = tropicnorth - bandwidth; j <= tropicnorth; j++)
            {
                currentjanadd = currentjanadd + janstep;
                currentjuladd = currentjuladd + julstep;

                float currentjan = float(world.janrain(i, j));
                float currentjul = float(world.julrain(i, j));

                currentjan = currentjan + currentjan * currentjanadd;
                currentjul = currentjul + currentjul * currentjuladd;

                int newjan = (int)currentjan;
                int newjul = (int)currentjul;

                world.setjanrain(i, j, newjan);
                world.setjulrain(i, j, newjul);
            }

            currentjanadd = 0;
            currentjuladd = 0;

            for (int j = tropicnorth + bandwidth; j > tropicnorth; j--)
            {
                currentjanadd = currentjanadd + janstep;
                currentjuladd = currentjuladd + julstep;

                float currentjan = float(world.janrain(i, j));
                float currentjul = float(world.julrain(i, j));

                currentjan = currentjan + currentjan * currentjanadd;
                currentjul = currentjul + currentjul * currentjuladd;

                int newjan = (int)currentjan;
                int newjul = (int)currentjul;

                world.setjanrain(i, j, newjan);
                world.setjulrain(i, j, newjul);
            }

            currentjanadd = 0;
            currentjuladd = 0;

            for (int j = tropicsouth - bandwidth; j <= tropicsouth; j++)
            {
                currentjanadd = currentjanadd + janstep;
                currentjuladd = currentjuladd + julstep;

                float currentjan = float(world.janrain(i, j));
                float currentjul = float(world.julrain(i, j));

                currentjan = currentjan + currentjan * currentjuladd; // Other way round for southern hemisphere!
                currentjul = currentjul + currentjul * currentjanadd;

                int newjan = (int)currentjan;
                int newjul = (int)currentjul;

                world.setjanrain(i, j, newjan);
                world.setjulrain(i, j, newjul);
            }

            currentjanadd = 0;
            currentjuladd = 0;

            for (int j = tropicsouth + bandwidth; j > tropicsouth; j--)
            {
                currentjanadd = currentjanadd + janstep;
                currentjuladd = currentjuladd + julstep;

                float currentjan = float(world.janrain(i, j));
                float currentjul = float(world.julrain(i, j));

                currentjan = currentjan + currentjan * currentjuladd; // Other way round for southern hemisphere!
                currentjul = currentjul + currentjul * currentjanadd;

                int newjan = (int)currentjan;
                int newjul = (int)currentjul;

                world.setjanrain(i, j, newjan);
                world.setjulrain(i, j, newjul);
            }
        }
        */
    }
}

// This smooths the rainfall.

void smoothrainfall(planet& world, int maxmountainheight)
{
    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();

    vector<vector<int>> smoothedjanrain(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));
    vector<vector<int>> smoothedjulrain(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

    for (int n = 0; n < 5; n++) // Quick basic smooth, which will include smoothing from one mountain tile to the next.
    {
        for (int i = 0; i <= width; i++)
        {
            for (int j = 0; j <= height; j++)
            {
                int crount = 0;
                int jantotal = 0;
                int jultotal = 0;

                for (int k = i - 1; k <= i + 1; k++)
                {
                    int kk = k;

                    if (kk<0 || kk>width)
                        kk = wrap(kk, width);

                    for (int l = j - 1; l <= j + 1; l++)
                    {
                        if (l >= 0 && l <= height)
                        {
                            if (abs(world.map(i, j) - world.map(kk, l)) < 150)
                            {
                                jantotal = jantotal + world.janrain(kk, l);
                                jultotal = jultotal + world.julrain(kk, l);
                                crount++;
                            }
                        }
                    }
                }

                jantotal = jantotal / crount;
                jultotal = jultotal / crount;

                smoothedjanrain[i][j] = jantotal;
                smoothedjulrain[i][j] = jultotal;
            }
        }

        for (int i = 0; i <= width; i++)
        {
            for (int j = 0; j <= height; j++)
            {
                world.setjanrain(i, j, smoothedjanrain[i][j]);
                world.setjulrain(i, j, smoothedjulrain[i][j]);
            }
        }
    }

    for (int n = 0; n < 5; n++) // Smooth with wider scope, avoiding mountains altogether.
    {
        for (int i = 0; i <= width; i++) // Normal blur.
        {
            int im = i - 1;
            int imm = i - 2;
            int ip = i + 1;
            int ipp = i + 2;

            if (im < 0)
                im = width;

            if (imm < 0)
                imm = wrap(imm, width);

            if (ip > width)
                ip = 0;

            if (ipp > width)
                ipp = wrap(ipp, width);

            for (int j = 0; j <= height; j++)
            {
                if (world.sea(i, j) == 0)
                {
                    if (world.mountainheight(i, j) < maxmountainheight)
                    {
                        float crount = 0;
                        float jantotal = 0;
                        float jultotal = 0;

                        for (int k = i - 1; k <= i + 1; k++) // First, check the cells bordering the central one.
                        {
                            int kk = k;

                            if (kk<0 || kk>width)
                                kk = wrap(kk, width);

                            for (int l = j - 1; l <= j + 1; l++)
                            {
                                if (l >= 0 && l <= height)
                                {
                                    if (world.sea(kk, l) == 0 && world.mountainheight(kk, l) < maxmountainheight)
                                    {
                                        jantotal = jantotal + world.janrain(kk, l);
                                        jultotal = jultotal + world.julrain(kk, l);
                                        crount++;
                                    }
                                }
                            }
                        }

                        int jm = j - 1;
                        int jmm = j - 2;
                        int jp = j + 1;
                        int jpp = j + 2;

                        if (jmm >= 0 && jpp <= height) // Now, check the cells bordering those ones.
                        {
                            // Cells to the north

                            if (world.sea(im, jmm) == 0 && world.mountainheight(im, jmm) < maxmountainheight && (world.mountainheight(im, jm) < maxmountainheight || world.mountainheight(i, jm) < maxmountainheight))
                            {
                                jantotal = jantotal + world.janrain(im, jmm);
                                jultotal = jultotal + world.julrain(im, jmm);
                                crount++;
                            }

                            if (world.sea(i, jmm) == 0 && world.mountainheight(i, jmm) < maxmountainheight && (world.mountainheight(im, jm) < maxmountainheight || world.mountainheight(i, jm) < maxmountainheight || world.mountainheight(ip, jm) < maxmountainheight))
                            {
                                jantotal = jantotal + world.janrain(i, jmm);
                                jultotal = jultotal + world.julrain(i, jmm);
                                crount++;
                            }

                            if (world.sea(ip, jmm) == 0 && world.mountainheight(ip, jmm) < maxmountainheight && (world.mountainheight(i, jm) < maxmountainheight || world.mountainheight(ip, jm) < maxmountainheight))
                            {
                                jantotal = jantotal + world.janrain(ip, jmm);
                                jultotal = jultotal + world.julrain(ip, jmm);
                                crount++;
                            }

                            // Cells to the south

                            if (world.sea(im, jpp) == 0 && world.mountainheight(im, jpp) < maxmountainheight && (world.mountainheight(im, jp) < maxmountainheight || world.mountainheight(i, jp) < maxmountainheight))
                            {
                                jantotal = jantotal + world.janrain(im, jpp);
                                jultotal = jultotal + world.julrain(im, jpp);
                                crount++;
                            }

                            if (world.sea(i, jpp) == 0 && world.mountainheight(i, jpp) < maxmountainheight && (world.mountainheight(im, jp) < maxmountainheight || world.mountainheight(i, jp) < maxmountainheight || world.mountainheight(ip, jp) < maxmountainheight))
                            {
                                jantotal = jantotal + world.janrain(i, jpp);
                                jultotal = jultotal + world.julrain(i, jpp);
                                crount++;
                            }

                            if (world.sea(ip, jpp) == 0 && world.mountainheight(ip, jpp) < maxmountainheight && (world.mountainheight(i, jp) < maxmountainheight || world.mountainheight(ip, jp) < maxmountainheight))
                            {
                                jantotal = jantotal + world.janrain(ip, jpp);
                                jultotal = jultotal + world.julrain(ip, jpp);
                                crount++;
                            }

                            // Cells to the west

                            if (world.sea(imm, jm) == 0 && world.mountainheight(imm, jm) < maxmountainheight && (world.mountainheight(im, jm) < maxmountainheight || world.mountainheight(im, j) < maxmountainheight))
                            {
                                jantotal = jantotal + world.janrain(imm, jm);
                                jultotal = jultotal + world.julrain(imm, jm);
                                crount++;
                            }

                            if (world.sea(imm, j) == 0 && world.mountainheight(imm, j) < maxmountainheight && (world.mountainheight(im, jm) < maxmountainheight || world.mountainheight(im, j) < maxmountainheight || world.mountainheight(im, jp) < maxmountainheight))
                            {
                                jantotal = jantotal + world.janrain(imm, j);
                                jultotal = jultotal + world.julrain(imm, j);
                                crount++;
                            }

                            if (world.sea(imm, jp) == 0 && world.mountainheight(imm, jp) < maxmountainheight && (world.mountainheight(im, j) < maxmountainheight || world.mountainheight(im, jp) < maxmountainheight))
                            {
                                jantotal = jantotal + world.janrain(imm, jp);
                                jultotal = jultotal + world.julrain(imm, jp);
                                crount++;
                            }

                            // Cells to the east

                            if (world.sea(ipp, jm) == 0 && world.mountainheight(ipp, jm) < maxmountainheight && (world.mountainheight(ip, jm) < maxmountainheight || world.mountainheight(ip, j) < maxmountainheight))
                            {
                                jantotal = jantotal + world.janrain(ipp, jm);
                                jultotal = jultotal + world.julrain(ipp, jm);
                                crount++;
                            }

                            if (world.sea(ipp, j) == 0 && world.mountainheight(ipp, j) < maxmountainheight && (world.mountainheight(ip, jm) < maxmountainheight || world.mountainheight(ip, j) < maxmountainheight || world.mountainheight(ip, jp) < maxmountainheight))
                            {
                                jantotal = jantotal + world.janrain(ipp, j);
                                jultotal = jultotal + world.julrain(ipp, j);
                                crount++;
                            }

                            if (world.sea(ipp, jp) == 0 && world.mountainheight(ipp, jp) < maxmountainheight && (world.mountainheight(ip, j) < maxmountainheight || world.mountainheight(ip, jp) < maxmountainheight))
                            {
                                jantotal = jantotal + world.janrain(ipp, jp);
                                jultotal = jultotal + world.julrain(ipp, jp);
                                crount++;
                            }
                        }

                        if (crount > 0)
                        {
                            jantotal = jantotal / crount;
                            jultotal = jultotal / crount;

                            world.setjanrain(i, j, (int)jantotal);
                            world.setjulrain(i, j, (int)jultotal);
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
    int width = world.width();
    int height = world.height();
    float tilt = world.tilt();
    float eccentricity = world.eccentricity();

    float maxrain = 1000.0f; // Any rainfall over this will be greatly reduced.
    float capfactor = 0.1f; // Amount to multiply excessive rain by.

    float rain[2];

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            rain[0] = (float)world.janrain(i, j);
            rain[1] = (float)world.julrain(i, j);

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

            world.setjanrain(i, j, (int)rain[0]);
            world.setjulrain(i, j, (int)rain[1]);
        }
    }

    if (tilt < 10.f && eccentricity < 0.1f) // For worlds with low obliquity and low eccentricity, reduce any seasonal difference in rainfall.
    {
        float adjustfactor = tilt;

        float reducefactor = 0.5f + tilt / 20.0f;

        for (int i = 0; i <= width; i++)
        {
            for (int j = 0; j <= height; j++)
            {
                float averain = (float)((world.janrain(i, j) + world.julrain(i, j)) / 2);

                if (averain > 0.0f)
                {
                    float thisjanrain = (float)world.janrain(i, j) * adjustfactor + averain * (10.0f - adjustfactor);
                    float thisjulrain = (float)world.julrain(i, j) * adjustfactor + averain * (10.0f - adjustfactor);

                    thisjanrain = thisjanrain * reducefactor;
                    thisjulrain = thisjulrain * reducefactor;

                    world.setjanrain(i, j, (int)thisjanrain);
                    world.setjulrain(i, j, (int)thisjulrain);
                }
            }
        }
    }
}

// This adjusts temperatures in the light of rainfall.

void adjusttemperatures(planet& world, vector<vector<int>>& inland)
{
    int width = world.width();
    int height = world.height();
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

        for (int i = 0; i <= width; i++)
        {
            for (int j = 0; j <= height; j++)
            {
                if (world.nom(i, j) > sealevel)
                {
                    float mintemp = (float)world.mintemp(i, j);
                    float maxtemp = (float)world.maxtemp(i, j);

                    float minchangemult = abs(mintemp - midvar) * varstep; // Multiply the final adjustment by these so the effect tails off with colder or hotter temperatures.
                    float maxchangemult = abs(maxtemp - midvar) * varstep;

                    float minremainmult = 1.0f - minchangemult;
                    float maxremainmult = 1.0f - maxchangemult;
                   
                    float winterrain = (float)world.winterrain(i, j);
                    float summerrain = (float)world.summerrain(i, j);

                    float wintervar = winterrain * winterrainwarmth;
                    float summervar = summerrain * summerraincold;

                    float continental = (float)inland[i][j]; // Further inland, there is less warming effect from winter rain.

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

                    if (world.summerrain(i, j) == 0 && maxtemp > 0.0f) // If there's no rain at all, temperatures are more extreme.
                        newmaxtemp = maxtemp * norainheat;

                    if (world.winterrain(i, j) == 0)
                        newmintemp = mintemp * noraincold;

                    if (newmintemp - (float)world.mintemp(i, j) > maxwinterrainwarmth)
                        newmintemp = (float)world.mintemp(i, j) + maxwinterrainwarmth;

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

                    world.setmintemp(i, j, (int)newmintemp);
                    world.setmaxtemp(i, j, (int)newmaxtemp);

                    if (world.maxtemp(i, j) < world.mintemp(i, j))
                        world.setmaxtemp(i, j, world.mintemp(i, j));
                }
            }
        }
    }
}

// This makes temperatures a bit more extreme further from the sea.

void adjustcontinentaltemperatures(planet& world, vector<vector<int>>& inland)
{
    float tilt = world.tilt();
    
    if (tilt == 0.0)
        return;

    int width = world.width();
    int height = world.height();
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

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (world.sea(i, j) == 0)
            {
                float thisstrength = (float) inland[i][j]; // The further inland, the higher the strength of the effect.

                thisstrength = thisstrength / 100.0f;

                if (thisstrength > 1.5f)
                    thisstrength = 1.5f;

                thisstrength = thisstrength * contstrength; // The more global tilt there is, the higher the strength of the effect.

                float maxtemp = (float)world.maxtemp(i, j);
                float mintemp = (float)world.mintemp(i, j);

                float tempdiff = maxtemp - mintemp; // Take the existing difference between summer and winter.

                float mintempremove = tempdiff * winterremovefactor * thisstrength; // Multiply it by the factors to get the amount to lower in winter and raise in summer.
                float maxtempadd = tempdiff * summeraddfactor * thisstrength;

                if (mintempremove > 20)
                    mintempremove = 20;

                if (maxtempadd > 10)
                    maxtempadd = 10;

                float newmaxtemp = maxtemp + maxtempadd;
                float newmintemp = mintemp - mintempremove;

                world.setmintemp(i, j, (int)newmintemp);
                world.setmaxtemp(i, j, (int)newmaxtemp);
            }
        }
    }

    // Now just a slight tweak to make tundra areas a little warmer, to encourage more subpolar.

    if (world.northpolartemp() > world.eqtemp() || world.southpolartemp() > world.eqtemp()) // Don't do this if the poles are hotter than the equator
        return;

    float tweakstrength = (tilt-22.5f)/30.0f; // This is to lessen the effect when the tilt is higher.
    tweakstrength = 1.0f - tweakstrength;

    if (tweakstrength <= 0.0)
        return;

    if (tweakstrength > 1.0)
        tweakstrength = 1.0f;

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (world.sea(i, j) == 0)
            {
                int maxtemp = world.maxtemp(i, j);
                int mintemp = world.mintemp(i, j);

                if (maxtemp <= 12)
                {
                    float factor = 10.0f; // 3

                    float elev = (float)world.map(i, j)-(float)sealevel;

                    factor = factor - elev / 400.0f; // Reduce this effect at higher elevations.

                    factor = factor * contstrength; // Reduce this for lower axial tilt.

                    factor = factor * tweakstrength; // Reduce it for higher axial tilt, too.

                    if (factor > 0.0)
                    {
                        int newmaxtemp = maxtemp + (int)factor;

                        if (newmaxtemp > 13)
                            newmaxtemp = 13;

                        int newmintemp = mintemp + (int)factor;

                        world.setmaxtemp(i, j, newmaxtemp);
                        world.setmintemp(i, j, newmintemp);
                    }
                }
            }
        }
    }
}

// This smooths the temperatures in light of rainfall.

void smoothtemperatures(planet& world)
{
    int width = world.width();
    int height = world.height();

    vector<vector<int>> jantemp(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));
    vector<vector<int>> jultemp(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (world.sea(i, j) == 0)
            {
                jantemp[i][j] = tempelevremove(world, world.jantemp(i, j), i, j);
                jultemp[i][j] = tempelevremove(world, world.jultemp(i, j), i, j);
            }
        }
    }

    for (int i = 0; i <= width; i++)
    {
        for (int j = 1; j < height; j++)
        {
            if (world.sea(i, j) == 0)
            {
                if (world.sea(i, j - 1) == 0 && world.sea(i, j + 1) == 0)
                {
                    if (jantemp[i][j] < jantemp[i][j - 1] && jantemp[i][j] < jantemp[i][j + 1])
                    {
                        jantemp[i][j] = (jantemp[i][j - 1] + jantemp[i][j + 1]) / 2;
                    }

                    if (jultemp[i][j] < jultemp[i][j - 1] && jultemp[i][j] < jultemp[i][j + 1])
                    {
                        jultemp[i][j] = (jultemp[i][j - 1] + jultemp[i][j + 1]) / 2;
                    }
                }
            }
        }
    }

    for (int i = 0; i <= width; i++)
    {
        for (int j = 1; j < height; j++)
        {
            if (world.sea(i, j) == 0)
            {
                world.setjantemp(i, j, tempelevadd(world, jantemp[i][j], i, j));
                world.setjultemp(i, j, tempelevadd(world, jultemp[i][j], i, j));
            }
        }
    }
}

// This function removes weird lines of subpolar climate.

void removesubpolarstreaks(planet& world)
{
    int width = world.width();
    int height = world.height();

    vector<vector<int>> origmaxtemp(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));
    vector<vector<int>> origmintemp(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

    for (int times = 1; times <= 2; times++)
    {
        for (int n = 1; n <= 4; n++)
        {
            for (int i = 0; i <= width; i++)
            {
                for (int j = 0; j <= height; j++)
                {
                    origmaxtemp[i][j] = world.maxtemp(i, j);
                    origmintemp[i][j] = world.mintemp(i, j);
                }
            }

            for (int i = 0; i <= width; i++)
            {
                for (int j = n; j < height; j = j + 2)
                {
                    if (world.sea(i, j) == 0)
                    {
                        int climate = calculateclimate(world.map(i, j), world.sealevel(), (float)world.winterrain(i, j), (float)world.summerrain(i, j), (float)world.mintemp(i, j), (float)world.maxtemp(i, j));

                        if (world.mountainheight(i, j) < 200 && ((climate > 4 && climate < 9) || (climate > 17 && climate < 30))) //  climate=="D" || climate=="BW" || climate=="BS"))
                        {
                            int jminus = j - 1;
                            int jplus = j + 1;

                            if (world.mintemp(i, jminus) <= -3 && world.maxtemp(i, jminus) > 10 && world.mintemp(i, jplus) <= -3 && world.maxtemp(i, jplus) > 10)

                                if (world.mountainheight(i, jminus) < 200 && world.mountainheight(i, jplus) < 200)
                                {
                                    world.setmintemp(i, j, (origmintemp[i][j] + origmintemp[i][jminus] + origmintemp[i][jplus]) / 3);
                                    world.setmaxtemp(i, j, (origmaxtemp[i][j] + origmaxtemp[i][jminus] + origmaxtemp[i][jplus]) / 3);
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
        {
            origmaxtemp[i][j] = world.maxtemp(i, j);
            origmintemp[i][j] = world.mintemp(i, j);
        }
    }

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (world.sea(i, j) == 0)
            {
                int climate = calculateclimate(world.map(i, j), world.sealevel(), (float)world.winterrain(i, j), (float)world.summerrain(i, j), (float)world.mintemp(i, j), (float)world.maxtemp(i, j));

                if (world.mountainheight(i, j) < 200 && ((climate > 4 && climate < 9) || (climate > 17 && climate < 30))) // (climate=="D" || climate=="BW" || climate=="BS"))
                {
                    int iminus = i - 1;
                    int iplus = i + 1;

                    if (iplus > width)
                        iplus = 0;

                    if (iminus < 0)
                        iminus = width;

                    if (world.mintemp(iminus, j) <= -3 && world.maxtemp(iminus, j) > 10 && world.mintemp(iplus, j) <= -3 && world.maxtemp(iplus, j) > 10)

                        if (world.mountainheight(iminus, j) < 200)
                        {
                            world.setmintemp(i, j, (origmintemp[i][j] + origmintemp[iminus][j] + origmintemp[iplus][j]) / 3);
                            world.setmaxtemp(i, j, (origmaxtemp[i][j] + origmaxtemp[iminus][j] + origmaxtemp[iplus][j]) / 3);
                        }
                }
            }

        }
    }
}

// This function extends subpolar regions to the east and west. (Not currently used - it was used in an earlier version of the climate model.)

void extendsubpolar(planet& world)
{
    int width = world.width();
    int height = world.height();

    vector<vector<bool>> subpolar(ARRAYWIDTH, vector<bool>(ARRAYHEIGHT, 0));

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (world.sea(i, j) == 0 && subpolar[i][j] == 0)
            {
                int climate = calculateclimate(world.map(i, j), world.sealevel(), (float)world.winterrain(i, j), (float)world.summerrain(i, j), (float)world.mintemp(i, j), (float)world.maxtemp(i, j));

                if (climate == 28 || climate == 29) // If it's subpolar
                {
                    int x = i;
                    int y = j;

                    short winddir = 0;

                    if (world.wind(i, j) > 0 && world.wind(i, j) != 99)
                        winddir = 1; // Westerly

                    if (world.wind(i, j) < 0 || world.wind(i, j) == 99)
                        winddir = -1; // Easterly

                    bool keepgoing = 1;

                    do
                    {
                        x = x + winddir;

                        if (x < 0)
                            x = width;

                        if (x > width)
                            x = 0;

                        if (random(1, 8) == 1)
                        {
                            if (y < height / 2) // Northern hemisphere
                                y++;
                            else // Southern hemisphere
                                y--;
                        }
                        else
                        {
                            if (random(1, 20) == 1)
                            {
                                if (y < height / 2) // Northern hemisphere
                                    y--;
                                else // Southern hemisphere
                                    y++;
                            }
                        }

                        if (y >= 0 && y <= height)
                        {
                            if (world.maxtemp(x, y) >= 14)
                            {
                                world.setmaxtemp(x, y, random(12, 13));
                                subpolar[x][y] = 1;
                            }

                            if (world.mintemp(x, y) > -3)
                                keepgoing = 0;

                            if (world.sea(x, y) == 1)
                                keepgoing = 0;

                            if (x == i)
                                keepgoing = 0;

                            if (world.maxtemp(x, y) <= 10)
                                keepgoing = 0;
                        }
                        else
                            keepgoing = 0;

                    } while (keepgoing == 1);
                }
            }
        }
    }
}

// This creates mountain precipitation arrays (which are used at the regional level).

void createmountainprecipitation(planet& world)
{
    int width = world.width();
    int height = world.height();

    float totalmountaineffect = 1000; // Mountains higher than this will get the full effect.

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (world.mountainheight(i, j) == 0 && world.craterrim(i, j) == 0)
            {
                world.setwintermountainraindir(i, j, 0);
                world.setsummermountainraindir(i, j, 0);
            }
        }
    }

    for (int n = 0; n < 1; n++)
    {
        // Using average neighbouring rainfall

        for (int i = 0; i <= width; i++) // Winter precipitation
        {
            for (int j = 0; j <= height; j++)
            {
                if ((world.mountainheight(i, j) != 0 || world.craterrim(i, j) != 0) && world.wintermountainraindir(i, j) == 0)
                {
                    // First, find the cell that the wind's coming from. It's the non-mountain cell with the highest precipitation.

                    int windfromcellx = -1;
                    int windfromcelly = -1;
                    int highestamount = -1;

                    for (int k = i - 1; k <= i + 1; k++)
                    {
                        int kk = k;

                        if (kk<0 || kk>width)
                            kk = wrap(kk, width);

                        for (int l = j - 1; l <= j + 1; l++)
                        {
                            if (l >= 0 && l <= height)
                            {
                                if (world.sea(kk, l) == 0 && world.mountainheight(kk, l) == 0 && world.craterrim(kk, l) == 0)
                                {
                                    if (world.winterrain(kk, l) > highestamount)
                                    {
                                        highestamount = world.winterrain(kk, l);
                                        windfromcellx = kk;
                                        windfromcelly = l;
                                    }
                                }
                            }
                        }
                    }

                    if (windfromcellx == -1) // We didn't find one!
                    {
                        for (int k = i - 1; k <= i + 1; k++)
                        {
                            int kk = k;

                            if (kk<0 || kk>width)
                                kk = wrap(kk, width);

                            for (int l = j - 1; l <= j + 1; l++)
                            {
                                if (l >= 0 && l <= height)
                                {
                                    if (world.wintermountainraindir(kk, l) != 0)
                                    {
                                        if (world.winterrain(kk, l) > highestamount)
                                        {
                                            highestamount = world.winterrain(kk, l);
                                            windfromcellx = kk;
                                            windfromcelly = l;
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // Now find the average precipitation of neighbouring cells.

                    short crount = 0;
                    int total = 0;

                    for (int k = i - 1; k <= i + 1; k++)
                    {
                        int kk = k;

                        if (kk<0 || kk>width)
                            kk = wrap(kk, width);

                        for (int l = j - 1; l <= j + 1; l++)
                        {
                            if (l >= 0 && l <= height)
                            {
                                if (world.sea(kk, l) == 0 && world.mountainheight(kk, l) == 0 && world.craterrim(kk, l) == 0)
                                {
                                    crount++;
                                    total = total + world.winterrain(kk, l);
                                }
                            }
                        }
                    }

                    if (crount == 0) // We didn't find one!
                    {
                        for (int k = i - 1; k <= i + 1; k++)
                        {
                            int kk = k;

                            if (kk<0 || kk>width)
                                kk = wrap(kk, width);

                            for (int l = j - 1; l <= j + 1; l++)
                            {
                                if (l >= 0 && l <= height)
                                {
                                    if (world.wintermountainraindir(kk, l) != 0)
                                    {
                                        crount++;
                                        total = total + world.winterrain(kk, l);
                                    }
                                }
                            }
                        }
                    }

                    if (windfromcellx != -1 && crount != 0) // If we found one!
                    {
                        float thisheight = (float)world.mountainheight(i, j);
                        float craterheight = (float)world.craterrim(i, j);

                        if (craterheight > thisheight)
                            thisheight = craterheight;

                        float effect = 1.0f;

                        if (thisheight < totalmountaineffect) // Reduce the strength of this.
                            effect = thisheight / totalmountaineffect;

                        float rainamount = (float)total / (float)crount;
                        float rainamount2 = (float)world.winterrain(i, j);

                        float newrainamount = (rainamount * effect) + (rainamount2 * (1.0f - effect));

                        world.setwintermountainrain(i, j, (int)newrainamount);

                        int dir = getdir(windfromcellx, windfromcelly, i, j);
                        world.setwintermountainraindir(i, j, dir);
                    }
                }
            }
        }

        for (int i = 0; i <= width; i++) // Summer precipitation
        {
            for (int j = 0; j <= height; j++)
            {
                if ((world.mountainheight(i, j) != 0 || world.craterrim(i, j) != 0) && world.summermountainraindir(i, j) == 0)
                {
                    // First, find the cell that the wind's coming from. It's the non-mountain cell with the highest precipitation.

                    int windfromcellx = -1;
                    int windfromcelly = -1;
                    int highestamount = -1;

                    for (int k = i - 1; k <= i + 1; k++)
                    {
                        int kk = k;

                        if (kk<0 || kk>width)
                            kk = wrap(kk, width);

                        for (int l = j - 1; l <= j + 1; l++)
                        {
                            if (l >= 0 && l <= height)
                            {
                                if (world.sea(kk, l) == 0 && world.mountainheight(kk, l) == 0 && world.craterrim(kk, l) == 0)
                                {
                                    if (world.summerrain(kk, l) > highestamount)
                                    {
                                        highestamount = world.summerrain(kk, l);
                                        windfromcellx = kk;
                                        windfromcelly = l;
                                    }
                                }
                            }
                        }
                    }

                    if (windfromcellx == -1) // We didn't find one!
                    {
                        for (int k = i - 1; k <= i + 1; k++)
                        {
                            int kk = k;

                            if (kk<0 || kk>width)
                                kk = wrap(kk, width);

                            for (int l = j - 1; l <= j + 1; l++)
                            {
                                if (l >= 0 && l <= height)
                                {
                                    if (world.summermountainraindir(kk, l) != 0)
                                    {
                                        if (world.summerrain(kk, l) > highestamount)
                                        {
                                            highestamount = world.summerrain(kk, l);
                                            windfromcellx = kk;
                                            windfromcelly = l;
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // Now find the average precipitation of neighbouring cells.

                    short crount = 0;
                    int total = 0;

                    for (int k = i - 1; k <= i + 1; k++)
                    {
                        int kk = k;

                        if (kk<0 || kk>width)
                            kk = wrap(kk, width);

                        for (int l = j - 1; l <= j + 1; l++)
                        {
                            if (l >= 0 && l <= height)
                            {
                                if (world.sea(kk, l) == 0 && world.mountainheight(kk, l) == 0 && world.craterrim(kk, l) == 0)
                                {
                                    crount++;
                                    total = total + world.summerrain(kk, l);
                                }
                            }
                        }
                    }

                    if (crount == 0) // We didn't find one!
                    {
                        for (int k = i - 1; k <= i + 1; k++)
                        {
                            int kk = k;

                            if (kk<0 || kk>width)
                                kk = wrap(kk, width);

                            for (int l = j - 1; l <= j + 1; l++)
                            {
                                if (l >= 0 && l <= height)
                                {
                                    if (world.summermountainraindir(kk, l) != 0)
                                    {
                                        crount++;
                                        total = total + world.summerrain(kk, l);
                                    }
                                }
                            }
                        }
                    }

                    if (windfromcellx != -1 && crount != 0) // If we found one!
                    {
                        float thisheight = (float)world.mountainheight(i, j);
                        float craterheight = (float)world.craterrim(i, j);

                        if (craterheight > thisheight)
                            thisheight = craterheight;

                        float effect = 1.0f;

                        if (thisheight < totalmountaineffect) // Reduce the strength of this.
                            effect = thisheight / totalmountaineffect;

                        float rainamount = (float)total / (float)crount;
                        float rainamount2 = (float)world.summerrain(i, j);

                        float newrainamount = (rainamount * effect) + (rainamount2 * (1.0f - effect));

                        world.setsummermountainrain(i, j, (int)newrainamount);

                        int dir = getdir(windfromcellx, windfromcelly, i, j);
                        world.setsummermountainraindir(i, j, dir);
                    }
                }
            }
        }
    }
}

// This function draws a splash of monsoon on the monsoon map.

void drawmonsoonblob(planet& world, vector<vector<int>>& monsoonmap, int centrex, int centrey, int strength, int monsoonsplashradius, short dir, boolshapetemplate smalllake[])
{
    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();

    int shapenumber = random(3, 10); //random(6,10);

    int imheight = smalllake[shapenumber].ysize() - 1;
    int imwidth = smalllake[shapenumber].xsize() - 1;

    int x = centrex - imwidth / 2; // Coordinates of the top left corner.
    int y = centrey - imheight / 2;

    if (x<0 || x>width)
        x = wrap(x, width);

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
                int xx = x + imap;
                int yy = y + jmap;

                if (yy >= 0 && yy <= height)
                {
                    if (xx<0 || xx>width)
                        xx = wrap(xx, width);

                    if (world.nom(xx, yy) > sealevel)
                    {
                        // Make sure that we don't put down monsoon where there's a downslope.

                        int upwindx = 0;
                        int upwindy = 0; // Coordinates of the immediately upwind point.

                        bool downwind = 0; // This marks whether the current point is downwind of the centre point.

                        switch (dir)
                        {
                        case 1:
                            upwindx = xx;
                            upwindy = yy + 1;

                            if (yy < centrey)
                                downwind = 1;

                            break;

                        case 2:
                            upwindx = xx - 1;
                            upwindy = yy + 1;

                            if (xx > centrex && yy < centrey)
                                downwind = 1;

                            break;

                        case 3:
                            upwindx = xx - 1;
                            upwindy = yy;

                            if (xx > centrex)
                                downwind = 1;

                            break;

                        case 4:
                            upwindx = xx - 1;
                            upwindy = yy - 1;

                            if (xx > centrex && yy > centrey)
                                downwind = 1;

                            break;

                        case 5:
                            upwindx = xx;
                            upwindy = yy - 1;

                            if (yy > centrey)
                                downwind = 1;

                            break;

                        case 6:
                            upwindx = xx + 1;
                            upwindy = yy - 1;

                            if (xx<centrex && yy>centrey)
                                downwind = 1;

                            break;

                        case 7:
                            upwindx = xx + 1;
                            upwindy = yy;

                            if (xx < centrex)
                                downwind = 1;

                            break;

                        case 8:
                            upwindx = xx + 1;
                            upwindy = yy + 1;

                            if (xx < centrex && yy < centrey)
                                downwind = 1;

                            break;
                        }

                        if (downwind == 0 || world.map(xx, yy) >= world.map(upwindx, upwindy))
                            monsoonmap[xx][yy] = strength;
                    }
                }
            }
        }
    }
}

// This function creates areas of sea that will later become salt lakes.

void createsaltlakes(planet& world, int& lakesplaced, vector<vector<vector<int>>>& saltlakemap, vector<vector<int>>& nolake, vector<vector<int>>& basins, boolshapetemplate smalllake[])
{
    int width = world.width();
    int height = world.height();

    int maxsaltrain = 30; // Salt lakes may form if there is less rain than this.
    int minsalttemp = 15; // Salt lakes require more heat than this.
    int minflow = 200; // Salt lakes can only form on a river of this size or greater.

    int saltlakechance = 100; //1000;

    bool dodepressions = 0;

    if (random(1, 100) == 1) // Only do this very occasionally, as it has the odd side-effect of turning high ground in terrain 4-type worlds into plateaux. Which is good as an occasional thing but not all the time!
        dodepressions = 1;

    vector<vector<int>> avoid(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0)); // This is for cells *not* to alter. This will remain empty (it's just here so we can use the depression-creating routine for normal lakes as well, where we don't want to mess with the path of the outflowing river.)

    for (int i = 20; i <= width - 20; i++) // Don't want them too close to the edges of the map, because it won't be possible to make lake start points if they wrap over the edges.
    {
        for (int j = 0; j <= height; j++)
        {
            if (basins[i][j] == 0 && nolake[i][j] == 0 && world.sea(i, j) == 0 && world.riverjan(i, j) + world.riverjul(i, j) >= minflow)
            {
                if (random(1, saltlakechance) == 1)
                {
                    int averain = (world.winterrain(i, j) + world.summerrain(i, j)) / 2;
                    int avetemp = (world.maxtemp(i, j) + world.mintemp(i, j)) / 2;

                    if (averain<maxsaltrain && avetemp>minsalttemp)
                    {
                        lakesplaced++;
                        placesaltlake(world, i, j, 0, dodepressions, saltlakemap, basins, avoid, nolake, smalllake);
                    }
                }
            }
        }
    }
}

// This function puts a patch of sea on the map that will later be turned into a salt lake.

void placesaltlake(planet& world, int centrex, int centrey, bool large, bool dodepressions, vector<vector<vector<int>>>& saltlakemap, vector<vector<int>>& basins, vector<vector<int>>& avoid, vector<vector<int>>& nolake, boolshapetemplate smalllake[])
{
    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();
    int riverlandreduce = world.riverlandreduce();

    int shapenumber;

    if (large || random(1, 8) == 1)
        shapenumber = random(5, 11);
    else
        shapenumber = random(2, 5);

    int depth = random(5, 30);

    // The surfaceheight will be a lot lower than this point originally is.

    int origheight = world.nom(centrex, centrey);
    int surfaceheight = (sealevel + (origheight - sealevel) / 2) - riverlandreduce;

    if (surfaceheight <= sealevel)
        surfaceheight = sealevel + 1;

    int imheight = smalllake[shapenumber].ysize() - 1;
    int imwidth = smalllake[shapenumber].xsize() - 1;

    int x = centrex - imwidth / 2; // Coordinates of the top left corner.
    int y = centrey - imheight / 2;

    if (x<0 || x>width)
        x = wrap(x, width);

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

                if (yy >= 0 && yy <= height)
                {
                    if (xx<0 || xx>width)
                        xx = wrap(xx, width);

                    if (nolake[xx][yy] == 1 || world.nom(xx, yy) <= sealevel) // Don't try to put lakes on top of sea...
                        tooclose = 1;
                    else
                    {
                        if (surfaceheight > world.nom(xx, yy) - riverlandreduce) // Lakes can't extend onto areas where local rivers will be lower than the surface of the lake.
                            tooclose = 1;
                        else
                        {

                            for (int k = xx - 2; k <= xx + 2; k++) // Check nearby cells for other lakes.
                            {
                                int kk = k;

                                if (kk<0 || kk>width)
                                    kk = wrap(kk, width);

                                for (int l = yy - 2; l <= yy + 2; l++)
                                {
                                    if (l >= 0 && l <= height)
                                    {
                                        if (saltlakemap[kk][l][0] != 0 && saltlakemap[kk][l][0] != surfaceheight)
                                            tooclose = 1;

                                        if (world.nom(kk, l) <= sealevel && saltlakemap[kk][l][0] != surfaceheight)
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

                if (yy >= 0 && yy <= height)
                {
                    if (xx<0 || xx>width)
                    {
                        wrapped = 1;
                        xx = wrap(xx, width);
                    }

                    world.setnom(xx, yy, sealevel - 1);
                    saltlakemap[xx][yy][0] = surfaceheight;
                    saltlakemap[xx][yy][1] = surfaceheight - (depth + randomsign(random(0, 4)));

                    if (saltlakemap[xx][yy][1] < 1)
                        saltlakemap[xx][yy][1] = 1;

                    if (xx < leftx)
                        leftx = xx;

                    if (xx > rightx)
                        rightx = xx;

                    if (yy < lefty)
                        lefty = yy;
                    if (yy > righty)
                        righty = yy;

                    if (world.mountainheight(xx, yy) != 0) // Remove any mountains that might be here.
                    {
                        for (int dir = 1; dir <= 8; dir++)
                            deleteridge(world, xx, yy, dir);
                    }
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
        createlakedepression(world, centrex, centrey, surfaceheight, steepness, basins, limit, 0, avoid);

    // Now we need to create a "start point" on the sea/lake.

    twointegers startpoint;
    startpoint.x = -1;
    startpoint.y = -1;

    int crount = 0;

    do
    {
        for (int i = leftx; i <= rightx; i++)
        {
            for (int j = lefty; j <= righty; j++)
            {
                if (saltlakemap[i][j][0] == surfaceheight)
                {
                    if (vaguelycoastal(world, i, j) == 1) // If this is on the edge of the sea/lake
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

    world.setlakestart(startpoint.x, startpoint.y, 1);
}

// This function lowers land around a given point to put it at the centre of a large depression.

void createlakedepression(planet& world, int centrex, int centrey, int origlevel, int steepness, vector<vector<int>>& basins, int limit, bool up, vector<vector<int>>& avoid)
{
    int width = world.width();
    int height = world.height();

    if (limit == 0)
        limit = 1000;

    int maxmountains = 200; // Mountains larger than this will be left alone.

    for (int radius = 1; radius <= limit; radius++)
    {
        int numberchanged = 0;

        int currentlevel = origlevel + (steepness * (radius - 1)); // Cells checked on this round must be no higher than this.

        for (int i = centrex - radius; i <= centrex + radius; i++)
        {
            int ii = i;

            if (ii<0 || ii>width)
                ii = wrap(ii, width);

            for (int j = centrey - radius; j <= centrey + radius; j++)
            {
                if (j >= 0 && j <= height)
                {
                    if (up == 1 || world.nom(ii, j) > currentlevel) // If this cell is higher than the current limit (or if we're raising cells as well as lowering them)
                    {
                        if (basins[ii][j] == 0 && avoid[ii][j] == 0 && world.sea(ii, j) == 0 && world.truelake(ii, j) == 0 && world.mountainheight(ii, j) < maxmountains)
                        {
                            if ((ii - centrex) * (ii - centrex) + (j - centrey) * (j - centrey) <= radius * radius) // If this cell is within the current radius
                            {
                                int thislevel = currentlevel; // The level of this particular cell, which will be a variation on the current level of the whole radius.

                                if (radius > 1 && up == 0) // Add a bit of variation to the slope.
                                    thislevel = thislevel - random(0, steepness);

                                world.setnom(ii, j, thislevel);
                                numberchanged++;
                                basins[ii][j] = 1;
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

// This function creates the rivers.

void createrivermap(planet& world, vector<vector<int>>& mountaindrainage)
{
    int width = world.width();
    int height = world.height();
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

    for (int i = 0; i <= width; i++)
    {
        world.setriverdir(i, 0, 1); // Tiles on the northern edge point north.
        world.setriverdir(i, height, 5); // Tiles on the southern edge point south.

        for (int j = 1; j < height; j++)
        {
            if (world.sea(i, j) == 0)
            {
                int dir = findlowestdirriver(world, neighbours, i, j, mountaindrainage);

                if (dir == -1) // No lowest neighbour! (This never actually happens.)
                    dir = random(1, 8);

                world.setriverdir(i, j, dir);
            }
            else
                world.setriverdir(i, j, 0);

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

    // Now, we go through the map tile by tile. Take the rainfall in each tile and add it to every downstream tile.

    vector<vector<int>> thisdrop(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

    int dropno = 1;
    bool goahead = 1;

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (world.sea(i, j) == 0 && world.strato(i, j) == 0)
            {
                goahead = 1;

                if (world.mountainheight(i, j) > mountainheightlimit) // Don't trace rivers from one high mountainous point to another, as it erodes the mountains too much.
                {
                    int dir = world.riverdir(i, j);

                    int x = i;
                    int y = j;

                    if (dir == 8 || dir == 1 || dir == 2)
                        y--;

                    if (dir == 4 || dir == 5 || dir == 6)
                        y++;

                    if (dir == 2 || dir == 3 || dir == 4)
                        x++;

                    if (dir == 6 || dir == 7 || dir == 8)
                        x--;

                    if (x<0 || x>width)
                        x = wrap(x, width);

                    if (y < 0)
                        y = 0;

                    if (y > height)
                        y = height;

                    if (world.mountainheight(x, y) > mountainheightlimit)
                        goahead = 0;
                }

                if (goahead == 1)
                {
                    tracedrop(world, i, j, minimum, dropno, thisdrop, maxrepeat, neighbours);
                    dropno++;
                }
            }
        }
    }

    if (tilt < 10.f) // For worlds with low axial tilt, reduce any seasonal difference in river flow.
    {
        int adjustfactor = (int)tilt;

        for (int i = 0; i <= width; i++)
        {
            for (int j = 0; j <= height; j++)
            {
                int aveflow = (world.riverjan(i, j) + world.riverjul(i, j)) / 2;

                if (aveflow > 0)
                {
                    int thisjanflow = world.riverjan(i, j) * adjustfactor + aveflow * (10 - adjustfactor);
                    int thisjulflow = world.riverjul(i, j) * adjustfactor + aveflow * (10 - adjustfactor);

                    world.setriverjan(i, j, thisjanflow);
                    world.setriverjul(i, j, thisjulflow);
                }
            }
        }
    }
}

// This function forces rivers to avoid diagonals, as they don't look so good on the regional map.

void removediagonalrivers(planet& world)
{
    int width = world.width();
    int height = world.height();

    int adjustchance = 1; // Probability of adjusting these bits. The higher it is, the less likely.

    int desti, destj, adji, adjj, newdir1, newdir2, newheight;

    for (int i = 0; i <= width; i++)
    {
        for (int j = 2; j <= height - 2; j++)
        {
            if (random(1, adjustchance) == 1)
            {
                bool candoit = 1;

                // Going northeast

                if (world.riverdir(i, j) == 2)
                {
                    desti = i + 1;
                    destj = j - 1;

                    if (desti > width)
                        desti = 0;

                    if (random(0, 1) == 1)
                    {
                        adji = i;
                        adjj = j - 1;

                        newdir1 = 1;
                        newdir2 = 3;

                        if (world.riverdir(adji, adjj) == 5 || world.riverdir(desti, destj) == 7)
                            candoit = 0;
                    }
                    else
                    {
                        adji = i + 1;
                        adjj = j;

                        if (adji > width)
                            adji = 0;

                        newdir1 = 3;
                        newdir2 = 1;

                        if (world.riverdir(adji, adjj) == 7 || world.riverdir(desti, destj) == 5)
                            candoit = 0;

                    }

                    newheight = (world.nom(i, j) + world.nom(desti, destj)) / 2;

                    if (newheight > world.nom(adji, adjj))
                        candoit = 0;

                    if (candoit == 1)
                    {
                        world.setnom(adji, adjj, newheight);
                        world.setriverdir(i, j, newdir1);
                        world.setriverdir(adji, adjj, newdir2);
                    }
                }

                // Going northwest

                if (world.riverdir(i, j) == 8)
                {
                    desti = i - 1;
                    destj = j - 1;

                    if (desti < 0)
                        desti = width;

                    if (random(0, 1) == 1)
                    {
                        adji = i;
                        adjj = j - 1;

                        newdir1 = 1;
                        newdir2 = 7;

                        if (world.riverdir(adji, adjj) == 5 || world.riverdir(desti, destj) == 3)
                            candoit = 0;
                    }
                    else
                    {
                        adji = i - 1;
                        adjj = j;

                        if (adji < 0)
                            adji = width;

                        newdir1 = 7;
                        newdir2 = 1;

                        if (world.riverdir(adji, adjj) == 5 || world.riverdir(desti, destj) == 3)
                            candoit = 0;

                    }

                    newheight = (world.nom(i, j) + world.nom(desti, destj)) / 2;

                    if (newheight > world.nom(adji, adjj))
                        candoit = 0;

                    if (candoit == 1)
                    {
                        world.setnom(adji, adjj, newheight);
                        world.setriverdir(i, j, newdir1);
                        world.setriverdir(adji, adjj, newdir2);
                    }
                }

                // Going southeast

                if (world.riverdir(i, j) == 4)
                {
                    desti = i + 1;
                    destj = j + 1;

                    if (desti > width)
                        desti = 0;

                    if (random(0, 1) == 1)
                    {
                        adji = i;
                        adjj = j + 1;

                        newdir1 = 5;
                        newdir2 = 3;

                        if (world.riverdir(adji, adjj) == 1 || world.riverdir(desti, destj) == 7)
                            candoit = 0;
                    }
                    else
                    {
                        adji = i + 1;
                        adjj = j;

                        if (adji > width)
                            adji = 0;

                        newdir1 = 3;
                        newdir2 = 5;

                        if (world.riverdir(adji, adjj) == 7 || world.riverdir(desti, destj) == 1)
                            candoit = 0;

                    }

                    newheight = (world.nom(i, j) + world.nom(desti, destj)) / 2;

                    if (newheight > world.nom(adji, adjj))
                        candoit = 0;

                    if (candoit == 1)
                    {
                        world.setnom(adji, adjj, newheight);
                        world.setriverdir(i, j, newdir1);
                        world.setriverdir(adji, adjj, newdir2);
                    }
                }

                // Going southwest

                if (world.riverdir(i, j) == 6)
                {
                    desti = i - 1;
                    destj = j + 1;

                    if (desti < 0)
                        desti = width;

                    if (random(0, 1) == 1)
                    {
                        adji = i;
                        adjj = j + 1;

                        newdir1 = 5;
                        newdir2 = 7;

                        if (world.riverdir(adji, adjj) == 1 || world.riverdir(desti, destj) == 3)
                            candoit = 0;
                    }
                    else
                    {
                        adji = i - 1;
                        adjj = j;

                        if (adji < 0)
                            adji = width;

                        newdir1 = 7;
                        newdir2 = 5;

                        if (world.riverdir(adji, adjj) == 3 || world.riverdir(desti, destj) == 1)
                            candoit = 0;

                    }

                    newheight = (world.nom(i, j) + world.nom(desti, destj)) / 2;

                    if (newheight > world.nom(adji, adjj))
                        candoit = 0;

                    if (candoit == 1)
                    {
                        world.setnom(adji, adjj, newheight);
                        world.setriverdir(i, j, newdir1);
                        world.setriverdir(adji, adjj, newdir2);
                    }
                }
            }
        }
    }
}

// This function makes rivers meet each other at diagonals, which looks a little more realistic.

void adddiagonalriverjunctions(planet& world)
{
    int width = world.width();
    int height = world.height();

    // First the N/S and E/W ones

    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            // Going east

            if (world.riverdir(i, j) == 3 && world.riverdir(i, j + 1) == 1)
                world.setriverdir(i, j + 1, 2);

            if (world.riverdir(i, j + 1) == 3 && world.riverdir(i, j) == 5)
                world.setriverdir(i, j, 4);

            // Going west

            if (world.riverdir(i + 1, j) == 7 && world.riverdir(i + 1, j + 1) == 1)
                world.setriverdir(i + 1, j + 1, 8);

            if (world.riverdir(i + 1, j + 1) == 7 && world.riverdir(i + 1, j) == 5)
                world.setriverdir(i + 1, j, 6);

            // Going north

            if (world.riverdir(i, j + 1) == 1 && world.riverdir(i + 1, j + 1) == 7)
                world.setriverdir(i + 1, j + 1, 8);

            if (world.riverdir(i + 1, j + 1) == 1 && world.riverdir(i, j + 1) == 3)
                world.setriverdir(i, j + 1, 2);

            // Going south

            if (world.riverdir(i, j) == 5 && world.riverdir(i + 1, j) == 7)
                world.setriverdir(i + 1, j, 6);

            if (world.riverdir(i + 1, j) == 5 && world.riverdir(i, j) == 3)
                world.setriverdir(i, j, 4);
        }
    }

    // Now the diagonal ones.

    for (int i = 0; i < width - 1; i++)
    {
        for (int j = 0; j < height - 1; j++)
        {
            // Going northeast

            if (world.riverdir(i, j) == 4 && world.riverdir(i + 1, j + 1) == 2)
            {
                if (world.nom(i + 1, j) > world.nom(i + 2, j))
                {
                    world.setriverdir(i, j, 3);
                    world.setriverdir(i + 1, j, 3);

                    if (world.nom(i + 1, j) > world.nom(i, j))
                        world.setnom(i + 1, j, world.nom(i, j) - 1);
                }
            }

            if (world.riverdir(i, j + 1) == 2 && world.riverdir(i + 1, j + 2) == 8)
            {
                if (world.nom(i + 1, j + 1) > world.nom(i + 1, j))
                {
                    world.setriverdir(i + 1, j + 2, 1);
                    world.setriverdir(i + 1, j + 1, 1);

                    if (world.nom(i + 1, j + 1) > world.nom(i + 1, j + 2))
                        world.setnom(i + 1, j + 1, world.nom(i + 1, j + 1) - 1);
                }
            }

            // Going southwest

            if (world.riverdir(i, j) == 4 && world.riverdir(i + 1, j + 1) == 6)
            {
                if (world.nom(i, j + 1) > world.nom(i, j + 1))
                {
                    world.setriverdir(i, j, 5);
                    world.setriverdir(i, j + 1, 5);

                    if (world.nom(i, j + 1) > world.nom(i, j))
                        world.setnom(i, j + 1, world.nom(i, j) - 1);
                }
            }

            if (world.riverdir(i + 1, j) == 6 && world.riverdir(i + 2, j + 1) == 8)
            {
                if (world.nom(i + 1, j + 1) > world.nom(i, j + 1))
                {
                    world.setriverdir(i + 2, j + 1, 7);
                    world.setriverdir(i + 1, j + 1, 7);

                    if (world.nom(i + 1, j + 1) > world.nom(i + 2, j + 1))
                        world.setnom(i + 1, j + 1, world.nom(i + 1, j + 1) - 1);
                }
            }

            // Going southeast

            if (world.riverdir(i + 1, j) == 4 && world.riverdir(i, j + 1) == 2)
            {
                if (world.nom(i + 1, j + 1) > world.nom(i + 2, j + 1))
                {
                    world.setriverdir(i, j + 1, 3);
                    world.setriverdir(i + 1, j + 1, 3);

                    if (world.nom(i + 1, j + 1) > world.nom(i, j + 1))
                        world.setnom(i + 1, j + 1, world.nom(i, j + 1) - 1);
                }
            }

            if (world.riverdir(i, j + 1) == 4 && world.riverdir(i + 1, j) == 6)
            {
                if (world.nom(i + 1, j + 1) > world.nom(i + 1, j + 2))
                {
                    world.setriverdir(i + 1, j, 5);
                    world.setriverdir(i + 1, j + 1, 5);

                    if (world.nom(i + 1, j + 1) > world.nom(i + 1, j))
                        world.setnom(i + 1, j + 1, world.nom(i + 1, j) - 1);
                }
            }

            // Going northwest

            if (world.riverdir(i + 1, j + 1) == 8 && world.riverdir(i + 2, j) == 6)
            {
                if (world.nom(i + 1, j) > world.nom(i, j))
                {
                    world.setriverdir(i + 2, j, 7);
                    world.setriverdir(i + 1, j, 7);

                    if (world.nom(i + 1, j) > world.nom(i + 2, j))
                        world.setnom(i + 1, j, world.nom(i + 2, j) - 1);
                }
            }

            if (world.riverdir(i + 1, j + 1) == 8 && world.riverdir(i, j + 2) == 2)
            {
                if (world.nom(i, j + 1) > world.nom(i, j))
                {
                    world.setriverdir(i, j + 2, 1);
                    world.setriverdir(i, j + 1, 1);

                    if (world.nom(i, j + 1) > world.nom(i, j + 2))
                        world.setnom(i, j + 1, world.nom(i, j + 2) - 1);
                }
            }
        }
    }
}

// This function makes rivers avoid volcanoes where possible.

void avoidvolcanoes(planet& world)
{
    int width = world.width();
    int height = world.height();
    int maxelev = world.maxelevation();

    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            int dir = world.riverdir(i, j);

            twointegers dest = getdestination(i, j, dir);

            if (world.volcano(dest.x, dest.y) != 0)
            {
                int currentelev = world.nom(i, j);

                int lowest = maxelev;
                int newx = dest.x;
                int newy = dest.y;

                for (int k = i - 1; k <= i + 1; k++)
                {
                    int kk = k;

                    if (kk<0 || kk>width)
                        kk = wrap(kk, width);

                    for (int l = j - 1; l <= j + 1; l++)
                    {
                        if (l >= 0 && l <= height)
                        {
                            if (kk != i || l != j)
                            {
                                if (world.volcano(kk, l) == 0)
                                {
                                    int nom = world.nom(kk, l);

                                    if (nom < currentelev && nom < lowest)
                                    {
                                        lowest = world.nom(kk, l);
                                        newx = kk;
                                        newy = l;
                                    }
                                }
                            }
                        }
                    }
                }

                int newdir = getdir(i, j, newx, newy);

                world.setriverdir(i, j, newdir);
            }
        }
    }
}

// This function makes parallel rivers run into each other.

void removeparallelrivers(planet& world)
{
    int width = world.width();
    int height = world.height();

    int removechance = 1;

    vector<vector<int>> altered(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0)); // This will show any points that have been involved in a change

    // First, N/S and E/W

    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            if (random(1, removechance) == 1)
            {
                // Going east

                if (world.riverdir(i, j) == 3 && world.riverdir(i + 1, j) == 3 && world.riverdir(i, j + 1) == 3 && world.riverdir(i + 1, j + 1) == 3)
                {
                    if (world.nom(i, j) > world.nom(i + 1, j + 1) && altered[i][j] == 0)
                    {
                        world.setriverdir(i, j, 4);

                        altered[i][j] = 1;
                        altered[i + 1][j] = 1;
                        altered[i][j + 1] = 1;
                        altered[i + 1][j + 1] = 1;
                    }
                    else
                    {
                        if (world.nom(i, j + 1) > world.nom(i + 1, j) && altered[i][j + 1] == 0)
                        {
                            world.setriverdir(i, j + 1, 2);

                            altered[i][j] = 1;
                            altered[i + 1][j] = 1;
                            altered[i][j + 1] = 1;
                            altered[i + 1][j + 1] = 1;
                        }
                    }
                }

                // Going west

                if (world.riverdir(i, j) == 7 && world.riverdir(i + 1, j) == 7 && world.riverdir(i, j + 1) == 7 && world.riverdir(i + 1, j + 1) == 7)
                {
                    if (world.nom(i + 1, j) > world.nom(i, j + 1) && altered[i + 1][j] == 0)
                    {
                        world.setriverdir(i + 1, j, 6);

                        altered[i][j] = 1;
                        altered[i + 1][j] = 1;
                        altered[i][j + 1] = 1;
                        altered[i + 1][j + 1] = 1;
                    }
                    else
                    {
                        if (world.nom(i + 1, j + 1) > world.nom(i, j) && altered[i + 1][j + 1] == 0)
                        {
                            world.setriverdir(i + 1, j + 1, 8);

                            altered[i][j] = 1;
                            altered[i + 1][j] = 1;
                            altered[i][j + 1] = 1;
                            altered[i + 1][j + 1] = 1;
                        }
                    }
                }

                // Going north

                if (world.riverdir(i, j) == 1 && world.riverdir(i, j + 1) == 1 && world.riverdir(i + 1, j) == 1 && world.riverdir(i + 1, j + 1) == 7)
                {
                    if (world.nom(i, j + 1) > world.nom(i + 1, j) && altered[i][j + 1] == 0)
                    {
                        world.setriverdir(i, j + 1, 2);

                        altered[i][j] = 1;
                        altered[i + 1][j] = 1;
                        altered[i][j + 1] = 1;
                        altered[i + 1][j + 1] = 1;
                    }
                    else
                    {
                        if (world.nom(i + 1, j + 1) > world.nom(i, j) && altered[i + 1][j + 1] == 0)
                        {
                            world.setriverdir(i + 1, j + 1, 8);

                            altered[i][j] = 1;
                            altered[i + 1][j] = 1;
                            altered[i][j + 1] = 1;
                            altered[i + 1][j + 1] = 1;
                        }
                    }
                }

                // Going south

                if (world.riverdir(i, j) == 5 && world.riverdir(i, j + 1) == 5 && world.riverdir(i + 1, j) == 5 && world.riverdir(i + 1, j + 1) == 5)
                {
                    if (world.nom(i, j) > world.nom(i + 1, j + 1) && altered[i][j] == 0)
                    {
                        world.setriverdir(i, j, 4);

                        altered[i][j] = 1;
                        altered[i + 1][j] = 1;
                        altered[i][j + 1] = 1;
                        altered[i + 1][j + 1] = 1;
                    }
                    else
                    {
                        if (world.nom(i + 1, j) > world.nom(i, j + 1) && altered[i + 1][j] == 0)
                        {
                            world.setriverdir(i + 1, j, 6);

                            altered[i][j] = 1;
                            altered[i + 1][j] = 1;
                            altered[i][j + 1] = 1;
                            altered[i + 1][j + 1] = 1;
                        }
                    }
                }
            }
        }
    }

    // Now the diagonals.

    for (int i = 0; i <= width - 2; i++)
    {
        for (int j = 0; j <= height - 2; j++)
        {
            if (random(1, removechance) == 1)
            {
                // Going northeast

                if (world.riverdir(i, j + 1) == 2 && world.riverdir(i + 1, j) == 2 && world.riverdir(i + 1, j + 2) == 2 && world.riverdir(i + 2, j + 1) == 2)
                {
                    if (world.nom(i, j + 1) > world.nom(i + 2, j + 1) && world.nom(i + 1, j + 1) > world.nom(i + 2, j + 1) && altered[i][j + 1] == 0)
                    {
                        world.setriverdir(i, j + 1, 3);
                        world.setriverdir(i + 1, j + 1, 3);

                        altered[i][j + 1] = 1;
                        altered[i + 1][j] = 1;
                        altered[i + 2][j + 1] = 1;
                        altered[i + 1][j + 1] = 1;

                        if (world.nom(i + 1, j + 1) > world.nom(i, j + 1))
                            world.setnom(i + 1, j + 1, world.nom(i, j + 1) - 1);

                    }
                    else
                    {
                        if (world.nom(i + 1, j + 2) > world.nom(i + 1, j) && world.nom(i + 1, j + 1) > world.nom(i + 1, j) && altered[i + 1][j + 2] == 0)
                        {
                            world.setriverdir(i + 1, j + 2, 1);
                            world.setriverdir(i + 1, j + 1, 1);

                            altered[i][j + 1] = 1;
                            altered[i + 1][j] = 1;
                            altered[i + 2][j + 1] = 1;
                            altered[i + 1][j + 1] = 1;

                            if (world.nom(i + 1, j + 1) > world.nom(i + 1, j + 2))
                                world.setnom(i + 1, j + 1, world.nom(i + 1, j + 2) - 1);
                        }
                    }
                }

                // Going southwest

                if (world.riverdir(i, j + 1) == 6 && world.riverdir(i + 1, j) == 6 && world.riverdir(i + 1, j + 2) == 6 && world.riverdir(i + 2, j + 1) == 6)
                {
                    if (world.nom(i + 1, j) > world.nom(i + 1, j + 2) && world.nom(i + 1, j + 1) > world.nom(i + 1, j + 2) && altered[i + 1][j] == 0)
                    {
                        world.setriverdir(i + 1, j, 5);
                        world.setriverdir(i + 1, j + 1, 5);

                        altered[i][j + 1] = 1;
                        altered[i + 1][j] = 1;
                        altered[i + 2][j + 1] = 1;
                        altered[i + 1][j + 1] = 1;

                        if (world.nom(i + 1, j + 1) > world.nom(i + 1, j))
                            world.setnom(i + 1, j + 1, world.nom(i + 1, j) - 1);

                    }
                    else
                    {
                        if (world.nom(i + 2, j + 1) > world.nom(i, j + 1) && world.nom(i + 1, j + 1) > world.nom(i, j + 1) && altered[i + 2][j + 1] == 0)
                        {
                            world.setriverdir(i + 2, j + 1, 7);
                            world.setriverdir(i + 1, j + 1, 7);

                            altered[i][j + 1] = 1;
                            altered[i + 1][j] = 1;
                            altered[i + 2][j + 1] = 1;
                            altered[i + 1][j + 1] = 1;

                            if (world.nom(i + 1, j + 1) > world.nom(i + 2, j + 1))
                                world.setnom(i + 1, j + 1, world.nom(i + 2, j + 1) - 1);
                        }
                    }
                }

                // Going southeast

                if (world.riverdir(i, j + 1) == 4 && world.riverdir(i + 1, j) == 4 && world.riverdir(i + 1, j + 2) == 4 && world.riverdir(i + 2, j + 1) == 4)
                {
                    if (world.nom(i + 1, j) > world.nom(i + 1, j + 2) && world.nom(i + 1, j + 1) > world.nom(i + 1, j + 2) && altered[i + 1][j] == 0)
                    {
                        world.setriverdir(i + 1, j, 5);
                        world.setriverdir(i + 1, j + 1, 5);

                        altered[i][j + 1] = 1;
                        altered[i + 1][j] = 1;
                        altered[i + 2][j + 1] = 1;
                        altered[i + 1][j + 1] = 1;

                        if (world.nom(i + 1, j + 1) > world.nom(i + 1, j))
                            world.setnom(i + 1, j + 1, world.nom(i + 1, j) - 1);

                    }
                    else
                    {
                        if (world.nom(i, j + 1) > world.nom(i + 2, j + 1) && world.nom(i + 1, j + 1) > world.nom(i + 2, j + 1) && altered[i][j + 1] == 0)
                        {
                            world.setriverdir(i, j + 1, 3);
                            world.setriverdir(i + 1, j + 1, 3);

                            altered[i][j + 1] = 1;
                            altered[i + 1][j] = 1;
                            altered[i + 2][j + 1] = 1;
                            altered[i + 1][j + 1] = 1;

                            if (world.nom(i + 1, j + 1) > world.nom(i, j + 1))
                                world.setnom(i + 1, j + 1, world.nom(i, j + 1) - 1);
                        }
                    }
                }

                // Going northwest

                if (world.riverdir(i, j + 1) == 8 && world.riverdir(i + 1, j) == 8 && world.riverdir(i + 1, j + 2) == 8 && world.riverdir(i + 2, j + 1) == 8)
                {
                    if (world.nom(i + 2, j + 1) > world.nom(i, j + 1) && world.nom(i + 1, j + 1) > world.nom(i, j + 1) && altered[i + 2][j + 1] == 0)
                    {
                        world.setriverdir(i + 2, j + 1, 7);
                        world.setriverdir(i + 1, j + 1, 7);

                        altered[i][j + 1] = 1;
                        altered[i + 1][j] = 1;
                        altered[i + 2][j + 1] = 1;
                        altered[i + 1][j + 1] = 1;

                        if (world.nom(i + 1, j + 1) > world.nom(i + 2, j + 1))
                            world.setnom(i + 1, j + 1, world.nom(i + 2, j + 1) - 1);

                    }
                    else
                    {
                        if (world.nom(i + 1, j + 2) > world.nom(i + 1, j) && world.nom(i + 1, j + 1) > world.nom(i + 1, j) && altered[i + 1][j + 2] == 0)
                        {
                            world.setriverdir(i + 1, j + 2, 1);
                            world.setriverdir(i + 1, j + 1, 1);

                            altered[i][j + 1] = 1;
                            altered[i + 1][j] = 1;
                            altered[i + 2][j + 1] = 1;
                            altered[i + 1][j + 1] = 1;

                            if (world.nom(i + 1, j + 1) > world.nom(i + 1, j + 2))
                                world.setnom(i + 1, j + 1, world.nom(i + 1, j + 2) - 1);
                        }
                    }
                }
            }
        }
    }
}

// This function removes any rivers that cross each other.

void removecrossingrivers(planet& world)
{
    int width = world.width();
    int height = world.height();

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            // Going northeast

            if (world.riverdir(i, j) == 2)
            {
                int ii = i;
                int jj = j - 1;

                if (jj >= 0 && jj <= height)
                {
                    if (world.riverdir(ii, jj) == 4)
                    {
                        if (world.nom(ii, jj) <= world.nom(i, j))
                            world.setriverdir(i, j, 1);
                    }
                }

                ii = i + 1;
                jj = j;

                if (ii > width)
                    ii = 0;

                if (world.riverdir(ii, jj) == 8)
                {
                    if (world.nom(ii, jj) <= world.nom(i, j))
                        world.setriverdir(i, j, 3);
                }
            }

            // Going southeast

            if (world.riverdir(i, j) == 4)
            {
                int ii = i;
                int jj = j + 1;

                if (jj >= 0 && jj <= height)
                {
                    if (world.riverdir(ii, jj) == 2)
                    {
                        if (world.nom(ii, jj) <= world.nom(i, j))
                            world.setriverdir(i, j, 5);
                    }
                }

                ii = i + 1;
                jj = j;

                if (ii > width)
                    ii = 0;

                if (world.riverdir(ii, jj) == 6)
                {
                    if (world.nom(ii, jj) <= world.nom(i, j))
                        world.setriverdir(i, j, 3);
                }
            }

            // Going southwest

            if (world.riverdir(i, j) == 6)
            {
                int ii = i;
                int jj = j + 1;

                if (jj >= 0 && jj <= height)
                {
                    if (world.riverdir(ii, jj) == 8)
                    {
                        if (world.nom(ii, jj) <= world.nom(i, j))
                            world.setriverdir(i, j, 5);
                    }
                }

                ii = i - 1;
                jj = j;

                if (i < 0)
                    ii = width;

                if (world.riverdir(ii, jj) == 4)
                {
                    if (world.nom(ii, jj) <= world.nom(i, j))
                        world.setriverdir(i, j, 7);
                }
            }

            // Going northwest

            if (world.riverdir(i, j) == 8)
            {
                int ii = i;
                int jj = j - 1;

                if (jj >= 0 && jj <= height)
                {
                    if (world.riverdir(ii, jj) == 6)
                    {
                        if (world.nom(ii, jj) <= world.nom(i, j))
                            world.setriverdir(i, j, 1);
                    }
                }

                ii = i - 1;
                jj = j;

                if (i < 0)
                    ii = width;

                if (world.riverdir(ii, jj) == 2)
                {
                    if (world.nom(ii, jj) <= world.nom(i, j))
                        world.setriverdir(i, j, 7);
                }
            }
        }
    }
}

// This function removes mountains that have rivers running over them.

void removerivermountains(planet& world)
{
    int width = world.width();
    int height = world.height();

    int amount = 1; // Distance around mountain rivers to remove mountains.
    int mountainremovechance = 16; // The higher this is, the fewer mountains around rivers will be removed.

    int minremoveheight = 100; // Only mountains higher than this will be removed.

    vector<vector<bool>> toremove(ARRAYWIDTH, vector<bool>(ARRAYHEIGHT, 0));

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (world.sea(i, j) == 0 && world.mountainheight(i, j) == 0)
            {
                twointegers dest = getflowdestination(world, i, j, 0);

                if (world.mountainheight(dest.x, dest.y) > minremoveheight && world.sea(dest.x, dest.y) == 0) // We've got flow from a non-mountain cell into a mountain cell!
                {
                    bool keepgoing = 1;

                    int x = i;
                    int y = j;

                    do
                    {
                        for (int k = x - amount; k <= x + amount; k++)
                        {
                            int kk = k;

                            if (kk<0 || kk>width)
                                kk = wrap(kk, width);

                            for (int l = y - amount; l <= y + amount; l++)
                            {
                                if (l >= 0 && l <= height && random(1, mountainremovechance) == 1)
                                    toremove[kk][l] = 1;
                            }
                        }

                        toremove[x][y] = 1;

                        dest = getflowdestination(world, x, y, 0);

                        x = dest.x;
                        y = dest.y;

                        if (y == 0 || y == height)
                            keepgoing = 0;

                        if (world.sea(x, y) == 1)
                            keepgoing = 0;

                        if (world.mountainheight(x, y) == 0)
                            keepgoing = 0;

                    } while (keepgoing == 1);
                }
            }
        }
    }

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (toremove[i][j] == 1 && world.sea(i, j) == 0)
            {
                world.setmintemp(i, j, tempelevremove(world, world.mintemp(i, j), i, j));
                world.setmaxtemp(i, j, tempelevremove(world, world.maxtemp(i, j), i, j));

                int totalrain = world.winterrain(i, j) + world.summerrain(i, j);

                short crount = 0;

                int winterraintotal = 0;
                int summerraintotal = 0;

                for (int x = i - 1; x <= i + 1; x++)
                {
                    int xx = x;

                    if (xx<0 || xx>width)
                        xx = wrap(xx, width);

                    for (int y = j - 1; y <= j + 1; y++)
                    {
                        if (y >= 0 && y <= height)
                        {
                            if (xx != y || y != j)
                            {
                                if (world.mountainheight(xx, y) == 0 && toremove[xx][y] == 0)
                                {
                                    crount++;

                                    winterraintotal = winterraintotal + world.winterrain(xx, y);
                                    summerraintotal = summerraintotal + world.summerrain(xx, y);
                                }
                            }
                        }
                    }
                }

                if (crount > 0)
                {
                    world.setwinterrain(i, j, winterraintotal / crount);
                    world.setsummerrain(i, j, summerraintotal / crount);
                }

                world.setmountainridge(i, j, 0);
                world.setmountainheight(i, j, 0);
            }
        }
    }

    cleanmountainridges(world);
}

// This function traces a drop downhill to the sea, depositing water as it goes.

void tracedrop(planet& world, int x, int y, int minimum, int dropno, vector<vector<int>>& thisdrop, int maxrepeat, int neighbours[8][2])
{
    twointegers newseatile;
    twointegers dest;

    int width = world.width();
    int height = world.height();
    float riverfactor = world.riverfactor();

    float janload = (float)world.janrain(x, y);
    float julload = (float)world.julrain(x, y);

    if ((janload + julload) / 2.0f < (float)minimum)
        return;

    janload = janload / riverfactor;
    julload = julload / riverfactor;

    bool keepgoing = 1;

    // This bit moves some water from the winter load to the summer. This is because winter precipitation feeds into the river more slowly than summer precipitation does.

    float amount = 10.0f;

    if (world.mintemp(x, y) < 0) // In cold areas, more winter water gets held over until summer because it's snow.
        amount = 3.0f;

    if (y < height / 2) // Northern hemisphere
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

    do
    {
        thisdrop[x][y] = dropno;

        world.setriverjan(x, y, world.riverjan(x, y) + (int)janload);
        world.setriverjul(x, y, world.riverjul(x, y) + (int)julload);

        dir = world.riverdir(x, y);

        if (dir == 8 || dir == 1 || dir == 2)
            y--;

        if (dir == 4 || dir == 5 || dir == 6)
            y++;

        if (dir == 2 || dir == 3 || dir == 4)
            x++;

        if (dir == 6 || dir == 7 || dir == 8)
            x--;

        if (x<0 || x>width)
            x = wrap(x, width);

        if (y < 0)
            y = 0;

        if (y > height)
            y = height;

        if (world.sea(x, y) == 1) // We'll continue the river into the sea for a couple of tiles, to help with the regional map generation later.
        {
            for (int n = 1; n <= 3; n++)
            {

                world.setriverjan(x, y, world.riverjan(x, y) + (int)janload);
                world.setriverjul(x, y, world.riverjul(x, y) + (int)julload);

                newseatile = findseatile(world, x, y, dir);

                int newx = newseatile.x;
                int newy = newseatile.y;

                if (newx != -1)
                {
                    if (newx == x && newy < y)
                        dir = 1;

                    if (newx > x && newy < y)
                        dir = 2;

                    if (newx > x && newy == y)
                        dir = 3;

                    if (newx > x && newy > y)
                        dir = 4;

                    if (newx == x && newy > y)
                        dir = 5;

                    if (newx<x && newy>y)
                        dir = 6;

                    if (newx < x && newy == y)
                        dir = 7;

                    if (newx < x && newy < y)
                        dir = 8;

                    world.setriverdir(x, y, dir);

                    x = newx;
                    y = newy;
                }
            }

            return;
        }

        if (y == 0 || y == height)
            keepgoing = 0;

        if (thisdrop[x][y] == dropno)
        {
            keepgoing = 0;
            world.setlakesurface(x, y, world.maxelevation() * 2); // Put a lake seed at the problem point to hide it!
        }

    } while (keepgoing == 1);
}

// This function turns the inland seas into proper salt lakes.

void convertsaltlakes(planet& world, vector<vector<vector<int>>>& saltlakemap)
{
    int width = world.width();
    int height = world.height();

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (saltlakemap[i][j][0] != 0)
            {
                world.setlakesurface(i, j, saltlakemap[i][j][0]);
                world.setnom(i, j, saltlakemap[i][j][1]);

                world.setspecial(i, j, 100);
            }
        }
    }
}

// This function creates the major freshwater lakes.

void createlakemap(planet& world, vector<vector<int>>& nolake, boolshapetemplate smalllake[], boolshapetemplate largelake[])
{
    int width = world.width();
    int height = world.height();
    int glacialtemp = world.glacialtemp();


    int minlake = 1000; // Flows of this size or higher might have lakes on them.
    int maxlake = 12000; // Flows larger than this can't have lakes.
    int lakechance = random(8, 16); //2; //10; //random(80,150); // The higher this is, the less likely lakes are.
    int minrain = 200;
    int maxtemp = 25; // Lakes won't appear where there is less rain *and* higher temperature than this.

    vector<vector<int>> thislake(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));
    vector<vector<int>> checked(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            thislake[i][j] = 0; // Clear the thislake array, where we will keep track of the lakes as we do them.
        }
    }

    // This array is used to make the lakes cluster in particular parts of the world.

    vector<vector<bool>> lakeprobability(ARRAYWIDTH, vector<bool>(ARRAYHEIGHT, 0));

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
            lakeprobability[i][j] = 0;
    }

    int clusternumber = 50; // Number of possible clusters of lakes on the map.
    int minrad = 10;
    int maxrad = 40; // Possible sizes of the clusters

    for (int n = 0; n < clusternumber; n++) // Put some circles on this array, to allow for lake clusters.
    {
        int centrex = random(0, width);
        int centrey = random(0, height);

        int radius = random(minrad, maxrad);

        for (int i = -radius; i <= radius; i++)
        {
            int ii = centrex + i;

            if (ii<0 || ii>width)
                ii = wrap(ii, width);

            for (int j = -radius; j <= radius; j++)
            {
                int jj = centrey + j;

                if (jj >= 0 && jj <= height)
                {
                    if (i * i + j * j < radius * radius + radius)
                        lakeprobability[ii][jj] = 1;
                }
            }
        }
    }

    // Now make the lakes

    twointegers nearestsea;
    int lakeno = 0;

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            int flow = world.riveraveflow(i, j);

            if (world.averain(i, j) >= minrain || world.avetemp(i, j) <= maxtemp)
            {
                if (flow >= minlake && flow <= maxlake && lakeprobability[i][j] == 1 && random(1, lakechance) == 1)
                {
                    if (nolake[i][j] == 0)
                    {
                        lakeno++;

                        int templatesize = 1;
                        int shapenumber;

                        if (world.avetemp(i, j) <= glacialtemp)
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
                        drawlake(world, shapenumber, i, j, thislake, lakeno, checked, nolake, templatesize, minrain, maxtemp, smalllake, largelake);
                    }
                }
            }
        }
    }

    // Now clean up a little.

    cleanlakes(world);

    // Now we need to add precipitation from the lakes.

    vector<vector<int>> lakewinterrainmap(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));
    vector<vector<int>> lakesummerrainmap(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

    lakerain(world, lakewinterrainmap, lakesummerrainmap);

    // Now we add new rivers from that extra precipitation.

    lakerivers(world, lakewinterrainmap, lakesummerrainmap);

    // Now we check that no rivers shrink inexplicably.

    checkglobalflows(world);
}

// Puts a lake template onto the lake map.

void drawlake(planet& world, int shapenumber, int centrex, int centrey, vector<vector<int>>& thislake, int lakeno, vector<vector<int>>& checked, vector<vector<int>>& nolake, int templatesize, int minrain, int maxtemp, boolshapetemplate smalllake[], boolshapetemplate largelake[])
{
    int width = world.width();
    int height = world.height();
    int riverlandreduce = world.riverlandreduce();

    if (centrey == 0 || centrey == height)
    {
        world.setlakesurface(centrex, centrey, 0);
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

    if (x<0 || x>width)
        x = wrap(x, width);

    int leftx = centrex;
    int lefty = centrey;
    int rightx = centrex;
    int righty = centrey; // These are the coordinates of the furthermost pixels of the lake.
    bool wrapped = 0; // If this is 1, the lake wraps over the edge of the map.

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

    // First, check to see if another lake is already too close.

    bool tooclose = 0;

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

                if (yy >= 0 && yy <= height)
                {
                    if (xx<0 || xx>width)
                    {
                        xx = wrap(xx, width);
                        wrapped = 1;
                    }

                    if (world.sea(xx, yy) == 0)
                    {
                        if (nolake[xx][yy] == 1)
                            tooclose = 1;

                        if (world.volcano(xx, yy) != 0)
                            tooclose = 1;

                        if (tooclose == 0) // Only do this next check if we haven't already failed
                        {
                            for (int xxx = xx - minlakedistance; xxx <= xx + minlakedistance; xxx++)
                            {
                                int xxxx = xxx;

                                if (xxxx<0 || xxxx>width)
                                    xxxx = wrap(xxxx, width);

                                for (int yyy = yy - minlakedistance; yyy <= yy + minlakedistance; yyy++)
                                {
                                    if (yyy >= 0 && yyy <= height)
                                    {
                                        if (thislake[xxxx][yyy] != 0 || world.volcano(xxxx, yyy) != 0) // There's another lake here already! Or a volcano.
                                        {
                                            tooclose = 1;
                                            xxx = xx + minlakedistance;
                                            yyy = yy + minlakedistance;
                                        }
                                    }
                                }
                            }
                        }

                        if (tooclose == 0) // Only do this next check if we haven't already failed the last!
                        {
                            for (int xxx = xx - minmountaindistance; xxx <= xx + minmountaindistance; xxx++)
                            {
                                int xxxx = xxx;

                                if (xxxx<0 || xxxx>width)
                                    xxxx = wrap(xxxx, width);

                                for (int yyy = yy - minmountaindistance; yyy <= yy + minmountaindistance; yyy++)
                                {
                                    if (yyy >= 0 && yyy <= height)
                                    {
                                        if (world.mountainheight(xxxx, yyy) > maxmountainlakeheight) // There are mountains here.
                                        {
                                            tooclose = 1;
                                            xxx = xx + minmountaindistance;
                                            yyy = yy + minmountaindistance;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else
                        tooclose = 1;
                }
            }
        }
    }

    if (tooclose == 1)
        return;

    if (wrapped == 1)
    {
        leftx = 0;
        lefty = 0;
        rightx = width;
        righty = height;
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

                if (yy >= 0 && yy <= height)
                {
                    if (xx<0 || xx>width)
                    {
                        xx = wrap(xx, width);
                        wrapped = 1;
                    }

                    thislake[xx][yy] = lakeno; // Mark it on the keeping-track array.

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

    // Now we find out what the lowest point on the edge of the lake is. This will be the point from which the outflowing river emerges, and it will determine the surface level of the whole lake.

    int smallest = world.maxelevation();
    int outflowx = -1;
    int outflowy = -1;

    for (int i = leftx; i <= rightx; i++)
    {
        for (int j = lefty; j <= righty; j++)
        {
            if (thislake[i][j] == lakeno)
            {
                float border = 0;

                for (int k = i - 1; k <= i + 1; k++)
                {
                    int kk = k;

                    if (kk<0 || kk>width)
                        kk = wrap(kk, width);

                    for (int l = j - 1; l <= j + 1; l++)
                    {
                        if (l >= 0 && l <= height)
                        {
                            if (thislake[kk][l] != lakeno)
                            {
                                border = 1;
                                k = i + 1;
                                l = j + 1;
                            }
                        }
                    }
                }

                if (border == 1)
                {
                    if (world.nom(i, j) < smallest)
                    {
                        smallest = world.nom(i, j);
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
        for (int j = lefty; j <= righty; j++)
        {
            if (thislake[i][j] == lakeno)
                world.setlakesurface(i, j, surfaceheight);
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
        for (int j = lefty; j <= righty; j++)
        {
            if (thislake[i][j] == lakeno)
            {
                float nom = (float)world.nom(i, j);

                float depth = nom - (float)surfaceheight;

                if (depth<0.0f) // If the elevation here is higher than the lake surface
                    depth = (0.0f - depth) * depthmult;

                if (depth > maxmaxdepth)
                    depth = maxmaxdepth;

                if (depth < mindepth)
                    depth = mindepth;

                int newnom = surfaceheight - (int)depth;

                if (newnom < 1)
                    newnom = 1;

                world.setnom(i, j, newnom);
            }
        }
    }

    // Now do the rivers.

    sortlakerivers(world, leftx, lefty, rightx, righty, centrex, centrey, outflowx, outflowy, thislake, lakeno);

    // Now mark a "start point" somewhere on the edge of the lake, which we will use when it comes to the regional map.

    makelakestartpoint(world, thislake, lakeno, leftx, lefty, rightx, righty);
}

// This creates a "start point" for a lake, which will be used for drawing it at the regional level.

void makelakestartpoint(planet& world, vector<vector<int>>& thislake, int lakeno, int leftx, int lefty, int rightx, int righty)
{
    twointegers startpoint;
    startpoint.x = -1;
    startpoint.y = -1;

    int crount = 0;

    do
    {
        for (int i = leftx; i <= rightx; i++)
        {
            for (int j = lefty; j <= righty; j++)
            {
                if (thislake[i][j] == lakeno)
                {
                    if (lakeoutline(world, thislake, lakeno, i, j) == 1) // If this is on the edge of the lake
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

    world.setlakestart(startpoint.x, startpoint.y, 1);
}

// Checks to see whether any rivers are flowing into this cell from neighbouring cells that are too low.

int toolowinflow(planet& world, int x, int y, int elev, int surfacelevel)
{
    int width = world.width();
    int height = world.height();
    int riverlandreduce = world.riverlandreduce();

    int toolow = 0;
    twointegers destpoint;

    for (int i = x - 1; i <= x + 1; i++)
    {
        int ii = i;

        if (ii<0 || ii>width)
            ii = wrap(ii, width);

        for (int j = y - 1; j <= y + 1; j++)
        {
            if (j >= 0 && j <= height)
            {
                if (world.nom(ii, j) < elev || world.nom(ii, j) - riverlandreduce < surfacelevel)
                {
                    destpoint = getflowdestination(world, ii, j, 0);

                    if (destpoint.x == x && destpoint.y == y)
                        return (1);
                }
            }
        }
    }
    return (toolow);
}

// This function checks whether the given tile is an outflow from the given lake.

int checkoutflow(planet& world, vector<vector<int>>& thislake, int lakeno, int x, int y)
{
    // First check whether this point is in the lake.

    if (thislake[x][y] != lakeno)
        return (0);

    // Now check whether this point is on the edge of the lake.

    int outline = lakeoutline(world, thislake, lakeno, x, y);

    if (outline == 0) // If it's not on the edge, get out.
        return (0);

    int dir = world.riverdir(x, y); // This is the direction water is flowing.

    int newx = x;
    int newy = y;

    if (dir == 8 || dir == 1 || dir == 2)
        newy = y - 1;

    if (dir == 4 || dir == 5 || dir == 6)
        newy = y + 1;

    if (dir == 2 || dir == 3 || dir == 4)
        newx = x + 1;

    if (dir == 6 || dir == 7 || dir == 8)
        newx = x - 1;

    // newx and newy are now the coordinates of where the water is flowing to.

    int width = world.width();
    int height = world.height();

    if (newy<0 || newy>height) // Flowing off the map!
        return (1);

    if (newx<0 || newx>width)
        newx = wrap(newx, width);

    if (thislake[newx][newy] != lakeno) // If the water is flowing to somewhere outside the lake
        return (1);

    return (0);
}

// This checks whether a given tile is on the edge of a given lake.

int lakeoutline(planet& world, vector<vector<int>>& thislake, int lakeno, int x, int y)
{
    int width = world.width();
    int height = world.height();

    int xx = x - 1;
    int yy = y;

    if (xx<0 || xx>width)
        xx = wrap(xx, width);

    if (yy >= 0 && yy <= height)
    {
        if (thislake[xx][yy] != lakeno)
            return(1);
    }

    xx = x + 1;
    yy = y;

    if (xx<0 || xx>width)
        xx = wrap(xx, width);

    if (yy >= 0 && yy <= height)
    {
        if (thislake[xx][yy] != lakeno)
            return(1);
    }

    xx = x;
    yy = y - 1;

    if (xx<0 || xx>width)
        xx = wrap(xx, width);

    if (yy >= 0 && yy <= height)
    {
        if (thislake[xx][yy] != lakeno)
            return(1);
    }

    xx = x;
    yy = y + 1;

    if (xx<0 || xx>width)
        xx = wrap(xx, width);

    if (yy >= 0 && yy <= height)
    {
        if (thislake[xx][yy] != lakeno)
            return(1);
    }

    xx = x - 1;
    yy = y - 1;

    if (xx<0 || xx>width)
        xx = wrap(xx, width);

    if (yy >= 0 && yy <= height)
    {
        if (thislake[xx][yy] != lakeno)
            return(1);
    }

    xx = x + 1;
    yy = y + 1;

    if (xx<0 || xx>width)
        xx = wrap(xx, width);

    if (yy >= 0 && yy <= height)
    {
        if (thislake[xx][yy] != lakeno)
            return(1);
    }

    xx = x + 1;
    yy = y - 1;

    if (xx<0 || xx>width)
        xx = wrap(xx, width);

    if (yy >= 0 && yy <= height)
    {
        if (thislake[xx][yy] != lakeno)
            return(1);
    }

    xx = x - 1;
    yy = y + 1;

    if (xx<0 || xx>width)
        xx = wrap(xx, width);

    if (yy >= 0 && yy <= height)
    {
        if (thislake[xx][yy] != lakeno)
            return(1);
    }

    return (0);
}

// This function tells us whether a tile is next to a lake.

int nexttolake(planet& world, int x, int y)
{
    int width = world.width();
    int height = world.height();

    int dist = 1;

    for (int i = x - dist; i <= x + dist; i++)
    {
        int ii = i;

        if (ii<0 || ii>width)
            ii = wrap(ii, width);

        for (int j = y - dist; j <= y + dist; j++)
        {
            if (j >= 0 && j <= height && world.lakesurface(ii, j) != 0)
                return (world.lakesurface(ii, j));
        }
    }
    return (0);
}

// The same thing, but ignores diagonals.

int nexttolakenodiag(planet& world, int x, int y)
{
    int width = world.width();
    int height = world.height();

    if (y > 0) // North
    {
        if (world.lakesurface(x, y - 1) != 0)
            return world.lakesurface(x, y - 1);
    }

    if (y < height) // South
    {
        if (world.lakesurface(x, y + 1) != 0)
            return world.lakesurface(x, y + 1);
    }

    int xx = x + 1; // East

    if (xx > width)
        xx = 0;

    if (world.lakesurface(xx, y) != 0)
        return world.lakesurface(xx, y);

    xx = x - 1; // West

    if (xx < 0)
        xx = width;

    if (world.lakesurface(xx, y) != 0)
        return world.lakesurface(xx, y);

    return 0;
}

// Deals with the rivers going into and out of a lake.

void sortlakerivers(planet& world, int leftx, int lefty, int rightx, int righty, int centrex, int centrey, int outflowx, int outflowy, vector<vector<int>>& thislake, int lakeno)
{
    int width = world.width();
    int height = world.height();

    // Mark the route of the outflowing river (if any) on this array. We will avoid messing with any of it.

    vector<vector<int>> outflow(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

    markriver(world, outflowx, outflowy, outflow, 0);

    // Now go over the lake area. Every river tile that is under or next to the lake should be pointing to the centre of the lake.

    vector<vector<int>> checkedriverlaketiles(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));
    vector<vector<int>> removedrivers(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

    int riverno = 0;

    for (int i = leftx; i <= rightx; i++)
    {
        for (int j = lefty; j <= righty; j++)
        {
            for (int k = i - 1; k <= i + 1; k++)
            {
                int kk = k;

                if (kk<0 || k>width)
                    kk = wrap(kk, width);

                for (int l = j - 1; l <= j + 1; l++)
                {
                    if (l >= 0 && l <= height)
                    {
                        // kk and l are the points around the current one.

                        if (outflow[kk][l] == 1)
                            checkedriverlaketiles[kk][l] = 1;

                        if (checkedriverlaketiles[kk][l] == 0)
                        {
                            bool mustdivert = 0;

                            if (world.lakesurface(kk, l) != 0)
                                mustdivert = 1;
                            else
                            {
                                if (nexttolake(world, kk, l) != 0) // If it's next to our lake
                                    mustdivert = 1;
                            }

                            if (mustdivert == 1)
                            {
                                riverno++;

                                riverno = divertlakeriver(world, kk, l, centrex, centrey, removedrivers, riverno, outflowx, outflowy, outflow);
                            }

                            checkedriverlaketiles[kk][l] = 1;
                        }
                    }
                }
            }
        }
    }

    // At this point, the outflowing river is reduced to nothing or almost nothing.
    // Now we need to work out how big it should actually be.

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
            checkedriverlaketiles[i][j] = 0;
    }

    leftx--;
    lefty--;
    rightx++;
    righty++;

    if (leftx<0 || rightx>width)
    {
        leftx = 0;
        rightx = width;
    }

    if (lefty < 0)
        lefty = 0;

    if (righty > height)
        righty = height;

    int janload = 0;
    int julload = 0;

    int origjanoutflow = 0;
    int origjuloutflow = 0;

    for (int i = leftx; i <= rightx; i++)
    {
        for (int j = lefty; j <= righty; j++)
        {
            if (thislake[i][j] != lakeno)
            {
                float border = 0;

                for (int k = i - 1; k <= i + 1; k++)
                {
                    int kk = k;

                    if (kk<0 || kk>width)
                        kk = wrap(kk, width);

                    for (int l = j - 1; l <= j + 1; l++)
                    {
                        if (l >= 0 && l <= height)
                        {
                            if (thislake[kk][l] == lakeno)
                            {
                                border = 1;
                                k = i + 1;
                                l = j + 1;
                            }
                        }
                    }
                }

                if (border == 1)
                {
                    janload = janload + world.riverjan(i, j);
                    julload = julload + world.riverjul(i, j);
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

    vector<vector<bool>> loadadded(ARRAYWIDTH, vector<bool>(ARRAYHEIGHT, 0));

    int x = outflowx;
    int y = outflowy;
    bool keepgoing = 1;

    do
    {
        if (loadadded[x][y] == 0) // Just to make sure it's not somehow going round in circles.
        {
            world.setriverjan(x, y, world.riverjan(x, y) + janload);
            world.setriverjul(x, y, world.riverjul(x, y) + julload);

            loadadded[x][y] = 1;

            int dir = world.riverdir(x, y);

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

            if (thislake[x][y] == lakeno && (x != outflowx || y != outflowy)) // If it's somehow flowed back into the lake!
                keepgoing = 0;

            if (y<0 || y>height)
                keepgoing = 0;

            if (world.sea(x, y) == 1)
                keepgoing = 0;

            if (world.truelake(x, y) != 0 && thislake[x][y] != lakeno) // If it's gone into another lake
                keepgoing = 0;
        }
        else
            keepgoing = 0;

    } while (keepgoing == 1);
}

// This marks the route of a river on an array, from the given point.

void markriver(planet& world, int x, int y, vector<vector<int>>& markedarray, int extra)
{
    int width = world.width();
    int height = world.height();

    if (y<0 || y>height || x<0 || x>width)
        return;

    bool keepgoing = 1;

    do
    {
        if (markedarray[x][y] == 1) // If we're somehow going round in a loop)
            return;

        markedarray[x][y] = 1;

        if (extra == 1)
        {
            for (int i = x - 1; i <= x + 1; i++)
            {
                int ii = i;

                if (ii<0 || ii>width)
                    ii = wrap(ii, width);

                for (int j = y - 1; j <= y + 1; j++)
                {
                    if (j >= 0 && j <= height)
                    {
                        if (markedarray[ii][j] == 0)
                            markedarray[ii][j] = 2;
                    }
                }
            }
        }

        int dir = world.riverdir(x, y);

        if (dir == 0)
            return;

        if (dir == 8 || dir == 1 || dir == 2)
            y--;

        if (dir == 4 || dir == 5 || dir == 6)
            y++;

        if (dir == 2 || dir == 3 || dir == 4)
            x++;

        if (dir == 6 || dir == 7 || dir == 8)
            x--;

        if (x<0 || x>width)
            x = wrap(x, width);

        if (y < 0)
            y = 0;

        if (y > height)
            y = height;

        if (world.sea(x, y) == 1)
            keepgoing = 0;

        if (y == 0 || y == height)
            keepgoing = 0;

    } while (keepgoing == 1);
}

// This diverts a river from the given point to the destination point.

int divertriver(planet& world, int x, int y, int destx, int desty, vector<vector<int>>& removedrivers, int riverno)
{
    twointegers destpoint;

    while (1 == 1)
    {
        if (x == destx && y == desty)
            return (riverno);

        int dir = getdir(x, y, destx, desty);

        if (dir == 0)
            return (riverno);

        if (world.riverdir(x, y) == dir)
        {
            destpoint = getflowdestination(world, x, y, 0);
            x = destpoint.x;
            y = destpoint.y;
        }
        else
        {
            // Delete the existing river

            riverno++;

            int janflow = world.riverjan(x, y);
            int julflow = world.riverjul(x, y);

            removeriver(world, removedrivers, riverno, x, y);

            // Now reinstate the river, pointing in the new direction

            world.setriverdir(x, y, dir);
            world.setriverjan(x, y, janflow);
            world.setriverjul(x, y, julflow);

            // Now move to the next point in the new river

            destpoint = getflowdestination(world, x, y, 0);

            x = destpoint.x;
            y = destpoint.y;

            addtoriver(world, x, y, janflow, julflow);
        }
    }
    return (riverno);
}

// This diverts a river that's next to a lake, to go into it.

int divertlakeriver(planet& world, int x, int y, int destx, int desty, vector<vector<int>>& removedrivers, int riverno, int outflowx, int outflowy, vector<vector<int>>& avoidarray)
{
    twointegers destpoint;

    for (int n = 1; n < 1000; n++) // It very occasionally gets stuck in an endless loop, so limit the number of times it can go around.
    {
        if (x == destx && y == desty)
            return (riverno);

        if (avoidarray[x][y] == 1)
            return (riverno);

        // First, get the direction to the destination.

        int dir = getdir(x, y, destx, desty);

        if (dir == 0)
            return (riverno);

        // Now we have to check to see whether that goes onto dry land, which we don't want.

        destpoint = getflowdestination(world, x, y, dir);

        int newdir = 0;

        if (world.lakesurface(destpoint.x, destpoint.y) == 0)
        {
            // First, see if we can swerve right.

            newdir = dir + 1;

            if (newdir == 9)
                newdir = 1;

            destpoint = getflowdestination(world, x, y, newdir);

            if (world.lakesurface(destpoint.x, destpoint.y) == 0)
            {

                // now see if we can swerve left.

                newdir = dir - 1;

                if (newdir == 0)
                    newdir = 8;

                destpoint = getflowdestination(world, x, y, newdir);

                if (world.lakesurface(destpoint.x, destpoint.y) == 0)
                {

                    // If we can't, then we will have to stop.

                    int janflow = world.riverjan(x, y);
                    int julflow = world.riverjul(x, y);

                    world.setriverjan(destx, desty, world.riverjan(destx, desty) + janflow);
                    world.setriverjul(destx, desty, world.riverjul(destx, desty) + julflow);

                    return (riverno);
                }
            }

            dir = newdir;
        }

        if (world.riverdir(x, y) == dir)
        {
            destpoint = getflowdestination(world, x, y, dir);

            x = destpoint.x;
            y = destpoint.y;
        }
        else
        {
            if (avoidarray[x][y] == 1)
                return (riverno);

            // Delete the existing river

            riverno++;

            int janflow = world.riverjan(x, y);
            int julflow = world.riverjul(x, y);

            removeriver(world, removedrivers, riverno, x, y);

            // Now reinstate the river, pointing in the new direction

            world.setriverdir(x, y, dir);
            world.setriverjan(x, y, janflow);
            world.setriverjul(x, y, julflow);

            // Now move to the next point in the new river

            destpoint = getflowdestination(world, x, y, 0);

            x = destpoint.x;
            y = destpoint.y;

            if (avoidarray[x][y] != 1)
                addtoriver(world, x, y, janflow, julflow);

        }
    }
    return (riverno);
}

// This removes a river, starting from the given point.

void removeriver(planet& world, vector<vector<int>>& removedrivers, int riverno, int x, int y)
{
    int janload = world.riverjan(x, y);
    int julload = world.riverjul(x, y);

    if (janload == 0 && julload == 0) // No river here...
        return;

    int width = world.width();
    int height = world.height();

    bool keepgoing = 1;

    do
    {
        int dir = world.riverdir(x, y);

        removedrivers[x][y] = riverno;

        if (world.riverjan(x, y) - janload >= 0)
            world.setriverjan(x, y, world.riverjan(x, y) - janload);
        else
            return;

        if (world.riverjul(x, y) - julload >= 0)
            world.setriverjul(x, y, world.riverjul(x, y) - julload);
        else
            return;

        if (dir == 8 || dir == 1 || dir == 2)
            y--;

        if (dir == 4 || dir == 5 || dir == 6)
            y++;

        if (dir == 2 || dir == 3 || dir == 4)
            x++;

        if (dir == 6 || dir == 7 || dir == 8)
            x--;

        if (x<0 || x>width)
            x = wrap(x, width);

        if (y < 0)
            y = 0;

        if (y > height)
            y = height;

        if (world.sea(x, y) == 1)
            keepgoing = 0;

        if (y == 0 || y == height)
            keepgoing = 0;

        if (removedrivers[x][y] == riverno)
            keepgoing = 0;

    } while (keepgoing == 1);

}

// This lowers and reduces a river, starting from the given point.

void reduceriver(planet& world, int janreduce, int julreduce, vector<vector<int>>& removedrivers, int riverno, int x, int y)
{
    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();

    bool keepgoing = 1;

    do
    {
        int dir = world.riverdir(x, y);

        removedrivers[x][y] = riverno;

        if (world.riverjan(x, y) - janreduce >= 0)
            world.setriverjan(x, y, world.riverjan(x, y) - janreduce);
        else
            return;

        if (world.riverjul(x, y) - julreduce >= 0)
            world.setriverjul(x, y, world.riverjul(x, y) - julreduce);
        else
            return;

        world.setnom(x, y, sealevel + 1);

        if (world.deltadir(x, y) == 0)
            world.setdeltadir(x, y, -1);

        if (dir == 8 || dir == 1 || dir == 2)
            y--;

        if (dir == 4 || dir == 5 || dir == 6)
            y++;

        if (dir == 2 || dir == 3 || dir == 4)
            x++;

        if (dir == 6 || dir == 7 || dir == 8)
            x--;

        if (x<0 || x>width)
            x = wrap(x, width);

        if (y < 0)
            y = 0;

        if (y > height)
            y = height;

        if (world.sea(x, y) == 1)
            keepgoing = 0;

        if (y == 0 || y == height)
            keepgoing = 0;

        if (removedrivers[x][y] == riverno)
            keepgoing = 0;

    } while (keepgoing == 1);
}

// This adds water to an existing river, starting from the given point.

void addtoriver(planet& world, int x, int y, int janload, int julload)
{
    int width = world.width();
    int height = world.height();

    if (y<0 || y>height || x<0 || x>width)
        return;

    vector<vector<int>> thisdrop(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

    bool keepgoing = 1;

    do
    {
        int dir = world.riverdir(x, y);

        thisdrop[x][y] = 1;

        world.setriverjan(x, y, world.riverjan(x, y) + janload);
        world.setriverjul(x, y, world.riverjul(x, y) + julload);

        if (dir == 8 || dir == 1 || dir == 2)
            y--;

        if (dir == 4 || dir == 5 || dir == 6)
            y++;

        if (dir == 2 || dir == 3 || dir == 4)
            x++;

        if (dir == 6 || dir == 7 || dir == 8)
            x--;

        if (x<0 || x>width)
            x = wrap(x, width);

        if (y < 0)
            y = 0;

        if (y > height)
            y = height;

        if (world.sea(x, y) == 1)
            keepgoing = 0;

        if (thisdrop[x][y] == 1)
            keepgoing = 0;

    } while (keepgoing == 1);

}

// This function cleans up the lake map, removing any absurd values.

void cleanlakes(planet& world)
{
    int width = world.width();
    int height = world.height();
    int maxelev = world.maxelevation();

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (world.lakesurface(i, j) > maxelev) // These are weird lake seeds that for some reason didn't get turned into lakes.
                world.setlakesurface(i, j, 0);

            if (world.truelake(i, j) == 1) // No extra elevation where there are lakes.
                world.setextraelev(i, j, 0);

        }
    }
}

// This adds new precipitation from the lakes.

void lakerain(planet& world, vector<vector<int>>& lakewinterrainmap, vector<vector<int>>& lakesummerrainmap)
{
    int width = world.width();
    int height = world.height();
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

    for (int j = 0; j <= height; j++)
    {
        for (int i = 1; i <= width; i++)
        {
            int currentwind = world.wind(i, j);

            if (currentwind > 0 && currentwind != 99)
            {
                if (currentwind == 101)
                    currentwind = 4;

                if (world.map(i, j) > sealevel && world.lakesurface(i, j) == 0 && world.lakesurface(i - 1, j) != 0)// If this is next to a lake
                {
                    int crount = currentwind * lakemult;
                    int waterlog = 0; // This will hold the amount of water being carried onto this land
                    bool lakeice = 0; // This will be 1 if any seasonal ice is found when picking up moisture.

                    // First we pick up water from the lake to the west.

                    while (crount > 0)
                    {
                        int ii = i - crount;

                        if (ii < 0)
                            ii = wrap(ii, width);

                        if (world.lakesurface(ii, j) != 0 && world.maxtemp(ii, j) > 0) // It picks up water from unfrozen lake
                        {
                            if (world.mintemp(ii, j) < 5)
                                lakeice = 1;

                            float temp = world.avetemp(ii, j) / tempfactor; // Less water is picked up from colder lakes.

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
                    int jj = j;

                    // Now we deposit water onto the land to the east.

                    while (crount < currentwind * lakemult)
                    {
                        int ii = i + crount;

                        if (random(1, swervechance) == 1)
                        {
                            jj = jj + randomsign(1);

                            if (jj < 0)
                                jj = 0;

                            if (jj > height)
                                jj = height;
                        }

                        if (ii > width)
                            ii = wrap(ii, width);

                        float waterdumped = (float)waterlog / (float)dumprate;

                        if (world.map(ii, j) - sealevel > slopemin)
                        {
                            float slope = (float)getslope(world, ii - 1, j, ii, j);

                            slope = slope / slopefactor;

                            if (slope > 1.0f) // If it's going uphill
                            {
                                waterdumped = waterdumped * slope;

                                float elevation = (float(world.map(ii, j) - sealevel));

                                waterdumped = waterdumped * elevationfactor * elevation;

                                if (waterdumped > (float)waterlog)
                                    waterdumped = (float)waterlog;
                            }
                        }

                        waterlog = waterlog - (int)waterdumped;

                        if (world.map(ii, jj) > sealevel && world.wind(ii, jj) < 50 && world.lakesurface(ii, jj) == 0)
                        {
                            for (int iii = ii - splashsize; iii <= ii + splashsize; iii++)
                            {
                                int iiii = iii;

                                if (iiii<0 || iiii>width)
                                    iiii = wrap(iiii, width);

                                for (int jjj = jj - splashsize; jjj <= jj + splashsize; jjj++)
                                {
                                    int jjjj = jjj;

                                    if (jjjj < 0)
                                        jjjj = 0;

                                    if (jjjj > height)
                                        jjjj = height;

                                    if (lakesummerrainmap[iiii][jjjj] < (int)(waterdumped / 2.0f))
                                        lakesummerrainmap[iiii][jjjj] = (int)(waterdumped / 2.0f);

                                    if (lakewinterrainmap[iiii][jjjj] < (int)(waterdumped / 2.0f))
                                        lakewinterrainmap[iiii][jjjj] = (int)(waterdumped / 2.0f);

                                }
                            }

                            if (lakesummerrainmap[ii][jj] < (int)waterdumped)
                                lakesummerrainmap[ii][jj] = (int)waterdumped;

                            if (lakewinterrainmap[ii][jj] < (int)waterdumped)
                                lakesummerrainmap[ii][jj] = (int)waterdumped;
                        }

                        crount++;

                        if (waterlog < 50)
                            crount = currentwind * lakemult;
                    }

                }
            }
        }
    }

    // Now the easterly winds.

    for (int j = 0; j <= height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            int currentwind = world.wind(i, j);

            if (currentwind < 0 || currentwind == 99)
            {
                if (currentwind == 99)
                    currentwind = -4;

                if (world.map(i, j) > sealevel && world.lakesurface(i, j) == 0 && world.lakesurface(i + 1, j) != 0)// If this is next to a lake
                {
                    int crount = 0 - (currentwind * lakemult);
                    int waterlog = 0; // This will hold the amount of water being carried onto this land
                    bool lakeice = 0; // This will be 1 if any seasonal ice is found when picking up moisture.

                    // First we pick up water from the lake to the east.

                    while (crount > 0)
                    {
                        int ii = i + crount;

                        if (ii > width)
                            ii = wrap(ii, width);

                        if (world.lakesurface(ii, j) != 0 && world.maxtemp(ii, j) > 0) // It picks up water from unfrozen lake
                        {
                            if (world.mintemp(ii, j) < 5)
                                lakeice = 1;

                            float temp = world.avetemp(ii, j) / tempfactor; // Less water is picked up from colder lakes.

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
                    int jj = j;

                    // Now we deposit water onto the land to the west.

                    while (crount < 0 - (currentwind * lakemult))
                    {
                        int ii = i - crount;

                        if (random(1, swervechance) == 1)
                        {
                            jj = jj + randomsign(1);

                            if (jj < 0)
                                jj = 0;

                            if (jj > height)
                                jj = height;
                        }

                        if (ii < 0)
                            ii = wrap(ii, width);

                        float waterdumped = (float)waterlog / (float)dumprate;

                        if (world.map(ii, j) - sealevel > slopemin)
                        {
                            float slope = (float)getslope(world, ii - 1, j, ii, j);

                            slope = slope / slopefactor;

                            if (slope > 1.0f) // If it's going uphill
                            {
                                waterdumped = waterdumped * slope;

                                float elevation = (float(world.map(ii, j) - sealevel));

                                waterdumped = waterdumped * elevationfactor * elevation;

                                if (waterdumped > (float)waterlog)
                                    waterdumped = (float)waterlog;
                            }
                        }

                        waterlog = waterlog - (int)waterdumped;

                        if (world.map(ii, jj) > sealevel && world.wind(ii, jj) < 50 && world.lakesurface(ii, jj) == 0)
                        {
                            for (int iii = ii - splashsize; iii <= ii + splashsize; iii++)
                            {
                                int iiii = iii;

                                if (iiii<0 || iiii>width)
                                    iiii = wrap(iiii, width);

                                for (int jjj = jj - splashsize; jjj <= jj + splashsize; jjj++)
                                {
                                    int jjjj = jjj;

                                    if (jjjj < 0)
                                        jjjj = 0;

                                    if (jjjj > height)
                                        jjjj = height;

                                    if (lakesummerrainmap[iiii][jjjj] < (int)(waterdumped / 2.0f))
                                        lakesummerrainmap[iiii][jjjj] = (int)(waterdumped / 2.0f);

                                    if (lakewinterrainmap[iiii][jjjj] < (int)(waterdumped / 2.0f))
                                        lakewinterrainmap[iiii][jjjj] = (int)(waterdumped / 2.0f);
                                }
                            }

                            if (lakesummerrainmap[ii][jj] < (int)waterdumped)
                                lakesummerrainmap[ii][jj] = (int)waterdumped;

                            if (lakewinterrainmap[ii][jj] < (int)waterdumped)
                                lakesummerrainmap[ii][jj] = (int)waterdumped;
                        }

                        crount++;

                        if (waterlog < 50)
                            crount = 0 - (currentwind * lakemult);
                    }
                }
            }
        }
    }

    // Now adjust for seasonal variation on land.

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (world.map(i, j) > sealevel)
            {
                float tempdiff = (float)(world.maxtemp(i, j) - world.mintemp(i, j));

                if (tempdiff < 0.0f)
                    tempdiff = 0.0f;

                float winterrain = (float)lakewinterrainmap[i][j];

                tempdiff = tempdiff * seasonalvar * winterrain;

                lakewinterrainmap[i][j] = lakewinterrainmap[i][j] + (int)tempdiff;
                lakesummerrainmap[i][j] = lakesummerrainmap[i][j] - (int)tempdiff;

                if (lakesummerrainmap[i][j] < 0)
                    lakesummerrainmap[i][j] = 0;
            }
        }
    }

    // Now cap excessive rainfall.

    float rain[2];

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            rain[0] = (float)lakewinterrainmap[i][j];
            rain[1] = (float)lakesummerrainmap[i][j];

            for (int n = 0; n <= 1; n++)
            {
                if (rain[n] > (float)maxrain)
                    rain[n] = ((rain[n] - (float)maxrain) * capfactor) + (float)maxrain;

                if (rain[n] < 0.0f)
                    rain[n] = 0.0f;
            }

            lakewinterrainmap[i][j] = (int)rain[0];
            lakesummerrainmap[i][j] = (int)rain[1];
        }
    }

    // Now blur the lake rain.

    smooth(lakewinterrainmap, width, height, maxelev, 1, 0);
    smooth(lakesummerrainmap, width, height, maxelev, 1, 0);

    // Now add the lake rain to the existing rain maps.

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            world.setwinterrain(i, j, world.winterrain(i, j) + lakewinterrainmap[i][j]);
            world.setsummerrain(i, j, world.summerrain(i, j) + lakesummerrainmap[i][j]);
        }
    }
}

// This does the same thing, but for rift lakes.

void riftlakerain(planet& world, vector<vector<int>>& lakewinterrainmap, vector<vector<int>>& lakesummerrainmap)
{
    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();
    int maxelev = world.maxelevation();

    int lakemult = 15; // This is the amount we multiply the wind factor by to get the number of lake tiles that will provide rain. The higher this is, the more rain will extend onto the land.
    float tempfactor = 20.0f; // The amount temperature affects moisture pickup over lakes. The higher it is, the more difference it makes.
    float mintemp = 0.15f; // The minimum fraction that low temperatures can reduce the pickup rate to.
    int dumprate = 5; // The higher this number, the less rain gets deposited, but the further across the land it is distributed.
    int pickuprate = 300; // The higher this number, the more rain gets picked up over lake tiles.
    int swervechance = 3; // The lower this number, the more variation in where the rain lands.
    int splashsize = 1; // The higher this number, the larger area gets splashed around each target tile.
    int slopefactor = 50; // This determines how much gradient affects rainfall. The lower it is, the more it affects it.
    float elevationfactor = 0.002f; // This determines how much elevation affects the degree to which gradient affects rainfall.
    int slopemin = 200; // This is how high land has to be before the gradient affects rainfall.
    float seasonalvar = 0.02f; // Variation in rainfall between seasons.
    int maxrain = 800; // Any rainfall over this will be greatly reduced.
    float capfactor = 0.1f; // Amount to multiply excessive rain by.

    // First the westerly winds.

    for (int j = 0; j <= height; j++)
    {
        for (int i = 1; i <= width; i++)
        {
            int currentwind = world.wind(i, j);

            if (currentwind > 0 && currentwind != 99)
            {
                if (currentwind == 101)
                    currentwind = 4;

                if (world.map(i, j) > sealevel && world.riftlakesurface(i, j) == 0 && world.riftlakesurface(i - 1, j) != 0)// If this is next to a lake
                {
                    int crount = currentwind * lakemult;
                    int waterlog = 0; // This will hold the amount of water being carried onto this land
                    bool lakeice = 0; // This will be 1 if any seasonal ice is found when picking up moisture.

                    // First we pick up water from the lake to the west.

                    while (crount > 0)
                    {
                        int ii = i - crount;

                        if (ii < 0)
                            ii = wrap(ii, width);

                        if (world.riftlakesurface(ii, j) != 0 && world.maxtemp(ii, j) > 0) // It picks up water from unfrozen lake
                        {
                            if (world.mintemp(ii, j) < 5)
                                lakeice = 1;

                            float temp = world.avetemp(ii, j) / tempfactor; // Less water is picked up from colder lakes.

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
                    int jj = j;

                    // Now we deposit water onto the land to the east.

                    while (crount < currentwind * lakemult)
                    {
                        int ii = i + crount;

                        if (random(1, swervechance) == 1)
                        {
                            jj = jj + randomsign(1);

                            if (jj < 0)
                                jj = 0;

                            if (jj > height)
                                jj = height;
                        }

                        if (ii > width)
                            ii = wrap(ii, width);

                        float waterdumped = (float)waterlog / (float)dumprate;

                        if (world.map(ii, j) - sealevel > slopemin)
                        {
                            float slope = (float)getslope(world, ii - 1, j, ii, j);

                            slope = slope / slopefactor;

                            if (slope > 1.0f) // If it's going uphill
                            {
                                waterdumped = waterdumped * slope;

                                float elevation = (float)(world.map(ii, j) - sealevel);

                                waterdumped = waterdumped * elevationfactor * elevation;

                                if (waterdumped > (float)waterlog)
                                    waterdumped = (float)waterlog;
                            }
                        }

                        waterlog = waterlog - (int)waterdumped;

                        if (world.map(ii, jj) > sealevel && world.wind(ii, jj) < 50 && world.riftlakesurface(ii, jj) == 0)
                        {
                            for (int iii = ii - splashsize; iii <= ii + splashsize; iii++)
                            {

                                int iiii = iii;

                                if (iiii<0 || iiii>width)
                                    iiii = wrap(iiii, width);

                                for (int jjj = jj - splashsize; jjj <= jj + splashsize; jjj++)
                                {
                                    int jjjj = jjj;

                                    if (jjjj < 0)
                                        jjjj = 0;

                                    if (jjjj > height)
                                        jjjj = height;

                                    if (lakesummerrainmap[iiii][jjjj] < (int)(waterdumped / 2.0f))
                                        lakesummerrainmap[iiii][jjjj] = (int)(waterdumped / 2.0f);

                                    if (lakewinterrainmap[iiii][jjjj] < (int)(waterdumped / 2.0f))
                                        lakewinterrainmap[iiii][jjjj] = (int)(waterdumped / 2.0f);

                                }
                            }

                            if (lakesummerrainmap[ii][jj] < (int)waterdumped)
                                lakesummerrainmap[ii][jj] = (int)waterdumped;

                            if (lakewinterrainmap[ii][jj] < (int)waterdumped)
                                lakesummerrainmap[ii][jj] = (int)waterdumped;
                        }

                        crount++;

                        if (waterlog < 50)
                            crount = currentwind * lakemult;
                    }

                }
            }
        }
    }

    // Now the easterly winds.

    for (int j = 0; j <= height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            int currentwind = world.wind(i, j);

            if (currentwind < 0 || currentwind == 99)
            {
                if (currentwind == 99)
                    currentwind = -4;

                if (world.map(i, j) > sealevel && world.riftlakesurface(i, j) == 0 && world.riftlakesurface(i + 1, j) != 0)// If this is next to a lake
                {
                    int crount = 0 - (currentwind * lakemult);
                    int waterlog = 0; // This will hold the amount of water being carried onto this land
                    bool lakeice = 0; // This will be 1 if any seasonal ice is found when picking up moisture.

                    // First we pick up water from the lake to the east.

                    while (crount > 0)
                    {
                        int ii = i + crount;

                        if (ii > width)
                            ii = wrap(ii, width);

                        if (world.riftlakesurface(ii, j) != 0 && world.maxtemp(ii, j) > 0) // It picks up water from unfrozen lake
                        {
                            if (world.mintemp(ii, j) < 5)
                                lakeice = 1;

                            float temp = world.avetemp(ii, j) / tempfactor; // Less water is picked up from colder lakes.

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
                    int jj = j;

                    // Now we deposit water onto the land to the west.

                    while (crount < 0 - (currentwind * lakemult))
                    {
                        int ii = i - crount;

                        if (random(1, swervechance) == 1)
                        {
                            jj = jj + randomsign(1);

                            if (jj < 0)
                                jj = 0;

                            if (jj > height)
                                jj = height;
                        }

                        if (ii < 0)
                            ii = wrap(ii, width);

                        float waterdumped = (float)waterlog / (float)dumprate;

                        if (world.map(ii, j) - sealevel > slopemin)
                        {
                            float slope = (float)getslope(world, ii - 1, j, ii, j);

                            slope = slope / slopefactor;

                            if (slope > 1.0f) // If it's going uphill
                            {
                                waterdumped = waterdumped * slope;

                                float elevation = (float)(world.map(ii, j) - sealevel);

                                waterdumped = waterdumped * elevationfactor * elevation;

                                if (waterdumped > (float)waterlog)
                                    waterdumped = (float)waterlog;
                            }
                        }

                        waterlog = waterlog - (int)waterdumped;

                        if (world.map(ii, jj) > sealevel && world.wind(ii, jj) < 50 && world.riftlakesurface(ii, jj) == 0)
                        {
                            for (int iii = ii - splashsize; iii <= ii + splashsize; iii++)
                            {
                                int iiii = iii;

                                if (iiii<0 || iiii>width)
                                    iiii = wrap(iiii, width);

                                for (int jjj = jj - splashsize; jjj <= jj + splashsize; jjj++)
                                {
                                    int jjjj = jjj;

                                    if (jjjj < 0)
                                        jjjj = 0;

                                    if (jjjj > height)
                                        jjjj = height;

                                    if (lakesummerrainmap[iiii][jjjj] < (int)(waterdumped / 2.0f))
                                        lakesummerrainmap[iiii][jjjj] = (int)(waterdumped / 2.0f);

                                    if (lakewinterrainmap[iiii][jjjj] < (int)(waterdumped / 2.0f))
                                        lakewinterrainmap[iiii][jjjj] = (int)(waterdumped / 2.0f);
                                }
                            }

                            if (lakesummerrainmap[ii][jj] < (int)waterdumped)
                                lakesummerrainmap[ii][jj] = (int)waterdumped;

                            if (lakewinterrainmap[ii][jj] < (int)waterdumped)
                                lakesummerrainmap[ii][jj] = (int)waterdumped;
                        }

                        crount++;

                        if (waterlog < 50)
                            crount = 0 - (currentwind * lakemult);
                    }
                }
            }
        }
    }

    // Now adjust for seasonal variation on land.

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (world.map(i, j) > sealevel)
            {
                float tempdiff = (float)(world.maxtemp(i, j) - world.mintemp(i, j));

                if (tempdiff < 0.0f)
                    tempdiff = 0.0f;

                float winterrain = (float)lakewinterrainmap[i][j];

                tempdiff = tempdiff * seasonalvar * winterrain;

                lakewinterrainmap[i][j] = lakewinterrainmap[i][j] + (int)tempdiff;
                lakesummerrainmap[i][j] = lakesummerrainmap[i][j] - (int)tempdiff;

                if (lakesummerrainmap[i][j] < 0)
                    lakesummerrainmap[i][j] = 0;
            }
        }
    }

    // Now cap excessive rainfall.

    float rain[2];

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            rain[0] = (float)lakewinterrainmap[i][j];
            rain[1] = (float)lakesummerrainmap[i][j];

            for (int n = 0; n <= 1; n++)
            {
                if (rain[n] > (float)maxrain)
                    rain[n] = ((rain[n] - (float)maxrain) * capfactor) + (float)maxrain;

                if (rain[n] < 0.0f)
                    rain[n] = 0.0f;
            }

            lakewinterrainmap[i][j] = (int)rain[0];
            lakesummerrainmap[i][j] = (int)rain[1];
        }
    }

    // Now blur the lake rain.

    smooth(lakewinterrainmap, width, height, maxelev, 1, 0);
    smooth(lakesummerrainmap, width, height, maxelev, 1, 0);

    // Now add the lake rain to the existing rain maps.

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            world.setwinterrain(i, j, world.winterrain(i, j) + lakewinterrainmap[i][j]);
            world.setsummerrain(i, j, world.summerrain(i, j) + lakesummerrainmap[i][j]);
        }
    }
}

// This adds new rivers from the lake-related rainfall.

void lakerivers(planet& world, vector<vector<int>>& lakewinterrainmap, vector<vector<int>>& lakesummerrainmap)
{
    int width = world.width();
    int height = world.height();
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

    vector<vector<int>> thisdrop(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

    int dropno = 1;

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (world.map(i, j) > sealevel)
            {
                tracelakedrop(world, i, j, minimum, dropno, thisdrop, maxrepeat, neighbours, lakesummerrainmap, lakewinterrainmap);
                dropno++;
            }
        }
    }
}

// This function traces a drop from lake rainfall downhill to the sea (or a lake), depositing water as it goes.

void tracelakedrop(planet& world, int x, int y, int minimum, int dropno, vector<vector<int>>& thisdrop, int maxrepeat, int neighbours[8][2], vector<vector<int>>& lakesummerrainmap, vector<vector<int>>& lakewinterrainmap)
{
    twointegers newseatile;
    twointegers dest;

    int width = world.width();
    int height = world.height();
    float riverfactor = world.riverfactor();

    float janload = (float)lakewinterrainmap[x][y];
    float julload = (float)lakesummerrainmap[x][y];

    if (y >= height / 2)
    {
        janload = (float)lakesummerrainmap[x][y];
        julload = (float)lakewinterrainmap[x][y];
    }

    if ((janload + julload) / 2.0f < (float)minimum)
        return;

    janload = janload / riverfactor;
    julload = julload / riverfactor;

    bool keepgoing = 1;

    // This bit moves some water from the winter load to the summer. This is because winter precipitation feeds into the river more slowly than summer precipitation does.

    float amount = 10.0f;

    if (world.mintemp(x, y) < 0) // In cold areas, more winter water gets held over until summer because it's snow.
        amount = 3.0f;

    if (y < height / 2) // Northern hemisphere
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
        // First check to see if there's another river nearby, if we haven't been here before.

        if (world.riverjan(x, y) == 0 && world.riverjul(x, y) == 0)
        {
            int riverdir = findnearbyriver(world, x, y, dropno, thisdrop, neighbours);

            if (riverdir != 0) // If there is a suitable river nearby
                world.setriverdir(x, y, riverdir); // Just divert the flow of this tile into it.
        }

        int newdir = world.riverdir(x, y);

        if (newdir == dir)
            repeated++;

        // Now we have a check to see if we've been going in a straight line for too long.

        if (repeated > maxrepeat && world.riverjan(x, y) == 0 && world.riverjul(x, y) == 0 && y > 0 && y < height) // If we've been going in the same direction for too long and we're not at the northern/southern edge.
        {
            int leftdir = dir - 1;
            if (leftdir < 1)
                leftdir = 8;

            int rightdir = dir + 1;
            if (rightdir > 8)
                rightdir = 1;

            int lx = x;
            int ly = y;
            int rx = x;
            int ry = y;

            // leftdir and rightdir are now possible directions we might move in.

            if (leftdir == 8 || leftdir == 1 || leftdir == 2)
                ly--;

            if (leftdir == 4 || leftdir == 5 || leftdir == 6)
                ly++;

            if (leftdir == 2 || leftdir == 3 || leftdir == 4)
                lx++;

            if (leftdir == 6 || leftdir == 7 || leftdir == 8)
                lx--;

            if (rightdir == 8 || rightdir == 1 || rightdir == 2)
                ry--;

            if (rightdir == 4 || rightdir == 5 || rightdir == 6)
                ry++;

            if (rightdir == 2 || rightdir == 3 || rightdir == 4)
                rx++;

            if (rightdir == 6 || rightdir == 7 || rightdir == 8)
                rx--;

            if (rx<0 || rx>width)
                rx = wrap(rx, width);

            if (lx<0 || lx>width)
                lx = wrap(lx, width);

            // Now we have the coordinates of the two possible alternative points to go to.

            int lowered = world.nom(x, y) - 1; // This is what we would lower the alternative point to.

            bool leftposs = swervecheck(world, lx, ly, x, y, lowered, neighbours);
            bool rightposs = swervecheck(world, rx, ry, x, y, lowered, neighbours);

            // Now we just see whether we can do the swerve, and do it.

            bool swerved = 0;

            if (leftposs == 1 && world.nom(lx, ly) < world.nom(rx, ry))
            {
                world.setriverdir(x, y, leftdir);
                newdir = leftdir;
                world.setnom(lx, ly, lowered);
                swerved = 1;
            }

            if (rightposs == 1 && world.nom(rx, ry) < world.nom(lx, ly))
            {
                world.setriverdir(x, y, rightdir);
                newdir = rightdir;
                world.setnom(rx, ry, lowered);
                swerved = 1;
            }

            if (leftposs == 1 && swerved == 0)
            {
                world.setriverdir(x, y, leftdir);
                newdir = leftdir;
                world.setnom(lx, ly, lowered);
                swerved = 1;
            }

            if (rightposs == 1 && swerved == 0)
            {
                world.setriverdir(x, y, rightdir);
                newdir = rightdir;
                world.setnom(rx, ry, lowered);
                swerved = 1;
            }
        }

        if (newdir != dir)
            repeated = 0;

        thisdrop[x][y] = dropno;

        world.setriverjan(x, y, world.riverjan(x, y) + (int)janload);
        world.setriverjul(x, y, world.riverjul(x, y) + (int)julload);

        dir = newdir;

        if (dir == 8 || dir == 1 || dir == 2)
            y--;

        if (dir == 4 || dir == 5 || dir == 6)
            y++;

        if (dir == 2 || dir == 3 || dir == 4)
            x++;

        if (dir == 6 || dir == 7 || dir == 8)
            x--;

        if (x<0 || x>width)
            x = wrap(x, width);

        if (y < 0)
            y = 0;

        if (y > height)
            y = height;

        if (world.sea(x, y) == 1) // We'll continue the river into the sea for a couple of tiles, to help with the regional map generation later.
        {
            for (int n = 1; n <= 3; n++)
            {

                world.setriverjan(x, y, world.riverjan(x, y) + (int)janload);
                world.setriverjul(x, y, world.riverjul(x, y) + (int)julload);

                newseatile = findseatile(world, x, y, dir);

                int newx = newseatile.x;
                int newy = newseatile.y;

                if (newx != -1)
                {
                    if (newx == x && newy < y)
                        dir = 1;

                    if (newx > x && newy < y)
                        dir = 2;

                    if (newx > x && newy == y)
                        dir = 3;

                    if (newx > x && newy > y)
                        dir = 4;

                    if (newx == x && newy > y)
                        dir = 5;

                    if (newx<x && newy>y)
                        dir = 6;

                    if (newx < x && newy == y)
                        dir = 7;

                    if (newx < x && newy < y)
                        dir = 8;

                    world.setriverdir(x, y, dir);

                    x = newx;
                    y = newy;
                }
            }

            return;
        }

        if (world.truelake(x, y) == 1) // These rivers will stop at lakes.
            keepgoing = 0;

        if (world.riftlakesurface(x, y) != 0)
            keepgoing = 0;

        if (y == 0 || y == height)
            keepgoing = 0;

        if (thisdrop[x][y] == dropno)
        {
            keepgoing = 0;
        }

    } while (keepgoing == 1);
}

// This function sees if there's a river nearby and returns its direction.

int findnearbyriver(planet& world, int x, int y, int dropno, vector<vector<int>>& thisdrop, int neighbours[8][2])
{
    int width = world.width();
    int height = world.height();

    int start = random(0, 7);

    for (int n = start; n < start + 8; n++)
    {
        int nn = wrap(n, y);

        int i = x + neighbours[nn][0];

        if (i<0 || i>width)
            i = wrap(i, width);

        int j = y + neighbours[nn][1];

        if (j >= 0 && j <= height)
        {
            if (thisdrop[i][j] != 0 && thisdrop[i][j] != dropno) // If theres a river there, and it's not the current one
            {
                if (world.nom(i, j) < world.nom(x, y))
                    return (nn + 1);
            }
        }
    }
    return (0);
}

// This function works out whether a river could divert into the given square if its height were lowered.

bool swervecheck(planet& world, int x, int y, int origx, int origy, int lowered, int neighbours[8][2])
{
    if (world.riverjan(x, y) != 0 || world.riverjul(x, y) != 0)
        return (0);

    int width = world.width();
    int height = world.height();

    int dir = findlowestdir(world, neighbours, x, y);

    int newx = x;
    int newy = y;

    if (dir == 8 || dir == 1 || dir == 2)
        newy--;

    if (dir == 4 || dir == 5 || dir == 6)
        newy++;

    if (dir == 2 || dir == 3 || dir == 4)
        newx++;

    if (dir == 6 || dir == 7 || dir == 8)
        newx--;

    if (newx<0 || newx>width)
        newx = wrap(newx, width);

    if (y<0 || y>height)
        return (0);

    if (newx == origx && newy == origy) // If it's just pointing back to the original tile
        return (0);

    if (world.nom(newx, newy) >= lowered) // If it would be higher than this tile when lowered
        return (0);

    if (world.lakesurface(newx, newy) != 0) // If there's a lake here
        return (0);

    return (1);
}

// This function checks to make sure that river flows don't decrease.

void checkglobalflows(planet& world)
{
    int width = world.width();
    int height = world.height();

    int found;

    do
    {
        found = 0;

        for (int i = 0; i <= width; i++)
        {
            for (int j = 0; j <= height; j++)
            {
                if (world.sea(i, j) == 0)
                {
                    if (world.riverjan(i, j) != 0 || world.riverjul(i, j) != 0)
                        found = checkthisflow(world, i, j, found);
                }
            }
        }
    } while (found != 0);
}

// This checks an individual cell to check that the flow isn't decreasing.

int checkthisflow(planet& world, int x, int y, int found)
{
    twointegers destpoint;

    destpoint = getflowdestination(world, x, y, 0);

    if (destpoint.x == -1)
        return (found);

    int dex = destpoint.x;
    int dey = destpoint.y;

    int jan = world.riverjan(x, y);
    int jul = world.riverjul(x, y);
    int jand = world.riverjan(dex, dey);
    int juld = world.riverjul(dex, dey);

    if (jand < jan || juld < jul)
    {
        found++;

        int janadd = jan - jand;
        int juladd = jul - juld;

        if (janadd > 0)
            world.setriverjan(dex, dey, jand + janadd);

        if (juladd > 0)
            world.setriverjul(dex, dey, juld + juladd);

        found = checkthisflow(world, dex, dey, found);
    }

    return (found);
}

// This creates the rift lake map.

void createriftlakemap(planet& world, vector<vector<int>>& nolake)
{
    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();

    int riftlakechance = random(500, 4000); // The higher this is, the fewer rift lakes there will be.
    int minlake = 500; // Flows of this size or higher might have lakes on them.
    int maxlake = 2000; // Flows larger than this can't have lakes.
    int minlakelength = 4;
    int maxlakelength = 10;

    twointegers nextpoint;

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (world.nom(i, j) >= sealevel && world.lakesurface(i, j) == 0 && nolake[i][j] == 0 && world.riftlakesurface(i, j) == 0)
            {
                if (world.riveraveflow(i, j) > minlake && world.riveraveflow(i, j) < maxlake && random(1, riftlakechance) == 1)
                {
                    // Check that no rift lakes currently exist downstream of here.

                    bool keepgoing = 1;
                    bool found = 0;

                    int x = i;
                    int y = j;

                    int crount = 0;

                    do
                    {
                        crount++;

                        int dir = world.riverdir(x, y);

                        if (dir == 8 || dir == 1 || dir == 2)
                            y--;

                        if (dir == 4 || dir == 5 || dir == 6)
                            y++;

                        if (dir == 2 || dir == 3 || dir == 4)
                            x++;

                        if (dir == 6 || dir == 7 || dir == 8)
                            x--;

                        if (x<0 || x>width)
                            x = wrap(x, width);

                        if (y<0 || y>height)
                            keepgoing = 0;

                        if (dir == 0 || world.sea(x, y) == 1)
                            keepgoing = 0;

                        if (world.riftlakesurface(x, y) != 0 || world.lakesurface(x, y) != 0)
                        {
                            found = 1;
                            keepgoing = 0;
                        }

                        if (crount > 100)
                            keepgoing = 0;

                    } while (keepgoing == 1);

                    if (found == 0)
                    {
                        x = i;
                        y = j;
                        keepgoing = 1;

                        do
                        {
                            int lakelength = random(minlakelength, maxlakelength);

                            nextpoint = createriftlake(world, x, y, lakelength, nolake); // Make a lake.

                            x = nextpoint.x;
                            y = nextpoint.y;

                            if (x == -1)
                                keepgoing = 0;
                            else
                            {
                                int gaplength = random(2, 10);

                                for (int n = 1; n <= gaplength; n++) // Move down the river a bit.
                                {
                                    if (x<0 || x>width)
                                        x = wrap(x, width);

                                    if (y >= 0 && y <= height)
                                    {
                                        int dir = world.riverdir(x, y);

                                        if (dir == 8 || dir == 1 || dir == 2)
                                            y--;

                                        if (dir == 4 || dir == 5 || dir == 6)
                                            y++;

                                        if (dir == 2 || dir == 3 || dir == 4)
                                            x++;

                                        if (dir == 6 || dir == 7 || dir == 8)
                                            x--;

                                        if (x<0 || x>width)
                                            x = wrap(x, width);

                                        if (y == 0 || y == height)
                                        {
                                            keepgoing = 0;
                                            n = gaplength;
                                        }
                                        else
                                        {
                                            if (y >= 0 && y <= height)
                                            {
                                                if (world.lakesurface(x, y) != 0)
                                                {
                                                    keepgoing = 0;
                                                    n = gaplength;
                                                }

                                                if (world.nom(x, y) <= sealevel)
                                                {
                                                    keepgoing = 0;
                                                    n = gaplength;
                                                }
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

    // Now we need to add precipitation from the lakes.

    vector<vector<int>> lakewinterrainmap(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));
    vector<vector<int>> lakesummerrainmap(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

    riftlakerain(world, lakewinterrainmap, lakesummerrainmap);

    // Now we add new rivers from that extra precipitation.

    lakerivers(world, lakewinterrainmap, lakesummerrainmap); // We can use the same function as before, thankfully.
}

// This function puts a particular rift lake onto the rift lake map.

twointegers createriftlake(planet& world, int startx, int starty, int lakelength, vector<vector<int>>& nolake)
{
    int width = world.width();
    int height = world.height();
    int maxelev = world.maxelevation();
    int riverlandreduce = world.riverlandreduce();

    int maxdepth = 1800; // Maximum depth
    int mindepth = 100; // Minimum depth

    vector<vector<int>> removedrivers(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));
    vector<vector<int>> currentriver(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

    // First we need to work out the surface level of this lake. It will be at the level of the river flowing out of it.

    int x = startx;
    int y = starty;

    for (int n = 1; n <= lakelength; n++)
    {
        int dir = world.riverdir(x, y);

        if (dir == 8 || dir == 1 || dir == 2)
            y--;

        if (dir == 4 || dir == 5 || dir == 6)
            y++;

        if (dir == 2 || dir == 3 || dir == 4)
            x++;

        if (dir == 6 || dir == 7 || dir == 8)
            x--;

        if (x<0 || x>width)
            x = wrap(x, width);

        if (y == 0 || y == height)
            n = lakelength;
    }

    int surfacelevel = world.nom(x, y) - riverlandreduce;

    world.setlakestart(startx, starty, 1); // This marks this as the beginning of a rift lake, which we'll need to know when it comes to the regional map.

    markriver(world, startx, starty, currentriver, 0);

    twointegers nextpoint;

    int depth = random(mindepth, maxdepth);

    x = startx;
    y = starty;
    int done = 0;

    for (int n = 1; n <= lakelength; n++)
    {
        bool keepgoing = 1;

        if (nolake[x][y] != 0)
        {
            keepgoing = 0;
            n = lakelength;
        }

        for (int i = x - 2; i <= x + 2; i++)
        {
            int ii = i;

            if (ii<0 || ii>width)
                ii = wrap(ii, width);

            for (int j = y - 2; j <= y + 2; j++)
            {
                if (j >= 0 && j <= height && world.lakesurface(ii, j) != 0)
                {
                    keepgoing = 0;
                    i = x + 2;
                    j = y + 2;
                    n = lakelength;
                }
                if (j > 0 && j <= height)
                {
                    if (world.riftlakesurface(ii, j) != 0 && world.riftlakesurface(ii, j) != surfacelevel)
                    {
                        keepgoing = 0;
                        i = x + 2;
                        j = y + 2;
                        n = lakelength;
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
                    world.setriftlakesurface(x, y, 0);
                    world.setriftlakebed(x, y, 0);

                    int dir = world.riverdir(x, y);

                    if (dir == 8 || dir == 1 || dir == 2)
                        y--;

                    if (dir == 4 || dir == 5 || dir == 6)
                        y++;

                    if (dir == 2 || dir == 3 || dir == 4)
                        x++;

                    if (dir == 6 || dir == 7 || dir == 8)
                        x--;

                    if (x<0 || x>width)
                        x = wrap(x, width);

                    if (y == 0 || y == height)
                        n = done;
                }
            }

            nextpoint.x = -1;
            nextpoint.y = -1;
            return (nextpoint);
        }

        world.setriftlakesurface(x, y, surfacelevel);

        done++;

        int thislevel = surfacelevel - depth + randomsign(random(0, 200));

        if (thislevel > surfacelevel - 50)
            thislevel = surfacelevel - 50;

        if (thislevel < 1)
            thislevel = 1;

        world.setriftlakebed(x, y, thislevel);

        int dir = world.riverdir(x, y);

        if (dir == 8 || dir == 1 || dir == 2)
            y--;

        if (dir == 4 || dir == 5 || dir == 6)
            y++;

        if (dir == 2 || dir == 3 || dir == 4)
            x++;

        if (dir == 6 || dir == 7 || dir == 8)
            x--;

        if (x<0 || x>width)
            x = wrap(x, width);

        if (y == 0 || y == height)
            n = lakelength;
    }

    if (done < 3) // If we weren't able to make this lake long enough, get rid of it again.
    {
        x = startx;
        y = starty;

        for (int n = 1; n <= done; n++)
        {
            world.setriftlakesurface(x, y, 0);
            world.setriftlakebed(x, y, 0);

            int dir = world.riverdir(x, y);

            if (dir == 8 || dir == 1 || dir == 2)
                y--;

            if (dir == 4 || dir == 5 || dir == 6)
                y++;

            if (dir == 2 || dir == 3 || dir == 4)
                x++;

            if (dir == 6 || dir == 7 || dir == 8)
                x--;

            if (x<0 || x>width)
                x = wrap(x, width);

            if (y == 0 || y == height)
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
            int ii = i;

            if (ii<0 || ii>width)
                ii = wrap(ii, width);

            for (int j = y - 1; j <= y + 1; j++)
            {
                if (j >= 0 && j <= height)
                {
                    if (world.riftlakesurface(ii, j) == 0) // If this is not in the rift lake
                    {
                        if (currentriver[i][j] == 0)
                        {
                            int lowest = maxelev;

                            int newdestx = -1;
                            int newdesty = -1;

                            for (int k = i - 1; k <= i + 1; k++) // Find the deepest neighbouring rift lake tile
                            {
                                int kk = k;

                                if (kk<0 || kk>width)
                                    kk = wrap(kk, width);

                                for (int l = j - 1; l <= j + 1; l++)
                                {
                                    if (l >= 0 && l <= height)
                                    {
                                        if (world.riftlakesurface(kk, l) == surfacelevel)
                                        {
                                            if (world.riftlakesurface(kk, l) < lowest)
                                            {
                                                lowest = world.riftlakesurface(kk, l);

                                                newdestx = kk;
                                                newdesty = l;
                                            }
                                        }
                                    }
                                }
                            }

                            if (newdestx != -1) // Now divert the flow towards that tile
                            {
                                divertriver(world, ii, j, newdestx, newdesty, removedrivers, riverno);

                                riverno++;
                            }
                        }
                    }
                }
            }
        }

        int dir = world.riverdir(x, y);

        if (dir == 8 || dir == 1 || dir == 2)
            y--;

        if (dir == 4 || dir == 5 || dir == 6)
            y++;

        if (dir == 2 || dir == 3 || dir == 4)
            x++;

        if (dir == 6 || dir == 7 || dir == 8)
            x--;

        if (x<0 || x>width)
            x = wrap(x, width);

        if (y == 0 || y == height)
            n = lakelength;
    }

    nextpoint.x = x;
    nextpoint.y = y;

    return (nextpoint);
}

// This creates the climate map.

void createclimatemap(planet& world)
{
    int width = world.width();
    int height = world.height();

    short climate;

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            climate = getclimate(world, i, j);

            world.setclimate(i, j, climate);
        }
    }
}

// This returns the climate type of the given point.

short getclimate(planet& world, int x, int y)
{
    if (world.sea(x, y) == 1)
        return (0);

    int elev = world.map(x, y);
    int sealevel = world.sealevel();


    float wrain = (float)world.winterrain(x, y);
    float srain = (float)world.summerrain(x, y);
    float mintemp = (float)world.mintemp(x, y);
    float maxtemp = (float)world.maxtemp(x, y);

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

// This function puts ergs in deserts.

void createergs(planet& world, boolshapetemplate smalllake[], boolshapetemplate largelake[], boolshapetemplate shape[])
{
    int width = world.width();
    int height = world.height();
    int worldsize = world.size();

    vector<vector<short>> ergprobability(ARRAYWIDTH, vector<short>(ARRAYHEIGHT, 0)); // This is the probability of making an erg here.
    vector<vector<short>> overlapprobability(ARRAYWIDTH, vector<short>(ARRAYHEIGHT, 0)); // This is the probability of this erg being able to overlap its neighbours.

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
        
        int centrex = random(0, width);
        int centrey = random(0, height);

        int imheight = shape[shapenumber].ysize() - 1;
        int imwidth = shape[shapenumber].xsize() - 1;

        int x = centrex - imwidth / 2; // Coordinates of the top left corner.
        int y = centrey - imheight / 2;

        if (x<0 || x>width)
            x = wrap(x, width);

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
                    int xx = x + imap;
                    int yy = y + jmap;

                    if (yy >= 0 && yy <= height)
                    {
                        if (xx<0 || xx>width)
                            xx = wrap(xx, width);

                        ergprobability[xx][yy] = probability;
                        overlapprobability[xx][yy] = overlap;

                    }
                }
            }
        }

        /*
        int radius = random(minrad, maxrad);

        for (int i = -radius; i <= radius; i++)
        {
            int ii = centrex + i;

            if (ii<0 || ii>width)
                ii = wrap(ii, width);

            for (int j = -radius; j <= radius; j++)
            {
                int jj = centrey + j;

                if (jj >= 0 && jj <= height)
                {
                    if (i * i + j * j < radius * radius + radius)
                        ergprobability[ii][jj] = 2;
                }
            }
        }

        radius = (int)((float)radius * 1.5f);

        for (int i = -radius; i <= radius; i++)
        {
            int ii = centrex + i;

            if (ii<0 || ii>width)
                ii = wrap(ii, width);

            for (int j = -radius; j <= radius; j++)
            {
                int jj = centrey + j;

                if (jj >= 0 && jj <= height)
                {
                    if (ergprobability[ii][jj] == 0 && i * i + j * j < radius * radius + radius)
                        ergprobability[ii][jj] = 1;
                }
            }
        }
        */
    }

    int mindist = 2; // Minimum distance to existing lakes etc.

    int lakeno = 0;

    vector<vector<int>> thislake(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (world.climate(i, j) == 5 && ergprobability[i][j] != 0) // If this is a hot desert
            {
                if (random(1, ergprobability[i][j]) == 1)
                {
                    bool goahead = 1;

                    for (int k = i - mindist; k <= i + mindist; k++) // Check that there aren't any lakes too close.
                    {
                        int kk = k;

                        if (kk<0 || kk>width)
                            kk = wrap(kk, width);

                        for (int l = j - mindist; l <= j + mindist; l++)
                        {
                            if (l >= 0 && l <= height)
                            {
                                if (world.truelake(kk, l) != 0)
                                    goahead = 0;
                            }
                        }
                    }

                    if (goahead == 1)
                    {
                        bool canoverlap = 0;

                        if (random(1, 100) < overlapprobability[i][j])
                            canoverlap = 1;
                        
                        int shapenumber;

                        if (random(1, 4) == 1)
                        {
                            shapenumber = random(0, 11);
                            drawspeciallake(world, shapenumber, i, j, lakeno, thislake, smalllake, canoverlap, 120); // 120 is the code for ergs.

                        }
                        else
                        {
                            if (random(1, 2) == 1)
                                shapenumber = random(0, 2);
                            else
                                shapenumber = random(0, 9);
                            drawspeciallake(world, shapenumber, i, j, lakeno, thislake, largelake, canoverlap, 120); // 120 is the code for ergs.
                        }

                        lakeno++;
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
    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();

    vector<vector<bool>> checked(ARRAYWIDTH, vector<bool>(ARRAYHEIGHT, 0));

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (world.special(i, j) == 120 && checked[i][j] == 0)
            {
                vector<vector<bool>> thiserg(ARRAYWIDTH, vector<bool>(ARRAYHEIGHT, 0));
                
                // First, find the lowest elevation within this erg.

                int lowestelev = lowestergelevation(world, i, j, thiserg);

                if (lowestelev < 2)
                    lowestelev = 2;

                if (lowestelev <= sealevel)
                    lowestelev = sealevel + 1;

                // Now apply that to the whole erg.

                for (int k = 0; k <= width; k++)
                {
                    for (int l = 0; l <= height; l++)
                    {
                        if (thiserg[k][l] == 1)
                        {
                            checked[k][l] = 1;
                            world.setlakesurface(k, l, lowestelev);
                        }
                    }
                }
            }
        }
    }
}

// This finds the lowest elevation in an erg.

int lowestergelevation(planet& world, int startx, int starty, vector<vector<bool>>& thiserg)
{
    int width = world.width();
    int height = world.height();

    int lowest = 1000000;

    int row[] = { -1,0,0,1 };
    int col[] = { 0,-1,1,0 };

    twointegers node;

    node.x = startx;
    node.y = starty;

    queue<twointegers> q; // Create a queue
    q.push(node); // Put our starting node into the queue

    while (q.empty() != 1) // Keep going while there's anything left in the queue
    {
        node = q.front(); // Take the node at the front of the queue
        q.pop(); // And then pop it out of the queue

        if (node.x >= 0 && node.x <= width && node.y >= 0 && node.y <= height && world.special(node.x, node.y) == 120)
        {
            int thiselev = world.nom(node.x, node.y);

            if (thiselev < lowest)
                lowest = thiselev;

            thiserg[node.x][node.y] = 1;

            for (int k = 0; k < 4; k++) // Look at the four neighbouring nodes in turn
            {
                twointegers nextnode;

                nextnode.x = node.x + row[k];
                nextnode.y = node.y + col[k];

                if (nextnode.x > width)
                    nextnode.x = 0;

                if (nextnode.x < 0)
                    nextnode.x = width;

                if (nextnode.y >= 0 && nextnode.y < height)
                {
                    if (world.special(nextnode.x, nextnode.y) == 120 && thiserg[nextnode.x][nextnode.y] == 0) // If this node is erg
                    {
                        thiserg[nextnode.x][nextnode.y] = 1;
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
    int width = world.width();
    int height = world.height();

    int saltchance = 1000; //250; // Probability of salt pans in any given desert tile.
    int mindist = 2; // Minimum distance to existing lakes etc.

    int lakeno = 0;

    vector<vector<int>> thislake(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (world.climate(i, j) == 5) // If this is a hot desert
            {
                if (random(1, saltchance) == 1)
                {
                    bool goahead = 1;

                    for (int k = i - mindist; k <= i + mindist; k++) // Check that there aren't any other lakes too close.
                    {
                        int kk = k;

                        if (kk<0 || kk>width)
                            kk = wrap(kk, width);

                        for (int l = j - mindist; l <= j + mindist; l++)
                        {
                            if (l >= 0 && l <= height)
                            {
                                if (world.lakesurface(kk, l) != 0)
                                    goahead = 0;
                            }
                        }
                    }

                    if (goahead == 1)
                    {
                        int shapenumber;

                        if (random(1, 4) != 1)
                        {
                            shapenumber = random(0, 3);
                            drawspeciallake(world, shapenumber, i, j, lakeno, thislake, smalllake, 0, 110); // 110 is the code for salt pans.
                        }
                        else
                        {
                            shapenumber = random(3, 11);
                            drawspeciallake(world, shapenumber, i, j, lakeno, thislake, smalllake, 0, 110); // 110 is the code for salt pans.
                        }

                        lakeno++;
                    }
                }
            }
        }
    }
}

// Puts a lake template onto the lakemap, but as a "special", e.g. salt pans etc.

void drawspeciallake(planet& world, int shapenumber, int centrex, int centrey, int lakeno, vector<vector<int>>& thislake, boolshapetemplate laketemplate[], bool canoverlap, int special)
{
    int width = world.width();
    int height = world.height();

    if (centrey == 0 || centrey == height)
    {
        world.setlakesurface(centrex, centrey, 0);
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

    if (x<0 || x>width)
        x = wrap(x, width);

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

                if (xx != 0 && xx <= width && world.nom(xx, yy) < surfaceheight)
                    surfaceheight = world.nom(xx, yy);

                if (yy >= 0 && yy <= height)
                {
                    if (xx<0 || xx>width)
                        xx = wrap(xx, width);

                    if (world.sea(xx, yy) != 0 || world.mountainheight(xx, yy) >= maxmountainlakeheight) // Don't try to put them on top of sea or mountains...
                        tooclose = 1;
                    else
                    {
                        int clim = 1; // This marks whether the climate is appropriate.

                        if (special == 110 || special == 120)
                        {
                            for (int k = xx - 1; k <= xx + 1; k++)
                            {
                                int kk = k;

                                if (kk<0 || kk>width)
                                    kk = wrap(kk, width);

                                for (int l = yy - 1; l <= yy + 1; l++)
                                {
                                    if (l >= 0 && l <= height)
                                    {
                                        if (world.climate(kk, l) != 5 || world.riverjan(kk, l) > 0 || world.riverjul(kk, l) > 0)
                                        {
                                            clim = 0;

                                            k = xx + 1;
                                            l = yy + 1;
                                        }
                                    }
                                }
                            }
                        }

                        if (clim != 1) // Don't put lakes onto inappropriate climates.
                            tooclose = 1;
                        else
                        {
                            if (world.outline(xx, yy) == 1)
                                tooclose = 1;
                            else
                            {
                                for (int xxx = xx - minlakedistance; xxx <= xx + minlakedistance; xxx++)
                                {
                                    int xxxx = xxx;

                                    if (xxxx<0 || xxxx>width)
                                        xxxx = wrap(xxxx, width);

                                    for (int yyy = yy - minlakedistance; yyy <= yy + minlakedistance; yyy++)
                                    {
                                        if (yyy >= 0 && yyy <= height)
                                        {
                                            if (thislake[xxxx][yyy] != 0 && thislake[xxxx][yyy] != lakeno && canoverlap == 0) // There's another lake here already!
                                                tooclose = 1;

                                            if (world.lakesurface(xxxx, yyy) != 0 && thislake[xxxx][yyy] != lakeno && (canoverlap == 0 || (special != 120 || world.special(xxxx, yyy) != 120))) // There's another kind of lake here already! (Doesn't apply when both are ergs.)
                                                tooclose = 1;

                                            if (world.sea(xxxx, yyy) == 1) // && special != 120) // Sea here (doesn't apply to ergs).
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

                if (yy >= 0 && yy <= height)
                {
                    if (xx<0 || xx>width)
                    {
                        xx = wrap(xx, width);
                        wrapped = 1;
                    }

                    world.setlakesurface(xx, yy, surfaceheight); // Put the lake on this bit of the lake map.
                    world.setnom(xx, yy, surfaceheight);
                    world.setspecial(xx, yy, special);

                    thislake[xx][yy] = lakeno; // Mark it on the keeping-track array too.

                    if (world.mountainheight(xx, yy) != 0) // Remove any mountains that might be here.
                    {
                        for (int dir = 1; dir <= 8; dir++)
                            deleteridge(world, xx, yy, dir);
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

    makelakestartpoint(world, thislake, lakeno, leftx, lefty, rightx, righty);
}

// This function calculates the tides.

void createtidalmap(planet& world)
{
    int width = world.width();
    int height = world.height();
    int lunar = (int)world.lunar();

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
            world.settide(i, j, -1);
    }
    
    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (world.sea(i, j) == 1)
            {
                bool landfound = 0;

                for (int k = i - 1; k <= i + 1; k++)
                {
                    int kk = k;

                    if (kk<0 || kk>width)
                        kk = wrap(kk, width);

                    for (int l = j - 1; l <= j + 1; l++)
                    {
                        if (l >= 0 && l <= height)
                        {
                            if (world.sea(kk, l) == 0)
                            {
                                landfound = 1;
                                k = i + 1;
                                l = j + 1;
                            }
                        }
                    }
                }

                if (landfound == 1)
                {
                    int tide = gettidalrange(world, i, j);

                    for (int k = i - 1; k <= i + 1; k++)
                    {
                        int kk = k;

                        if (kk<0 || kk>width)
                            kk = wrap(kk, width);

                        for (int l = j - 1; l <= j + 2; l++)
                        {
                            if (l >= 0 && l <= height)
                            {
                                if (world.tide(kk, l) == -1)
                                    world.settide(kk, l, tide);
                            }
                        }
                    }

                    if (tide < lunar * 2)
                        tide = lunar * 2;

                    world.settide(i, j, tide);
                }
            }
        }
    }
}

// This function finds the tidal range of a given point.

int gettidalrange(planet& world, int startx, int starty)
{
    int width = world.width();
    int height = world.height();
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
    int x, y;

    // North

    hitland = 0;
    x = startx;
    y = starty;

    for (int n = 1; n <= 1000; n++)
    {
        y--;

        if (y < 0)
            hitland = 1;
        else
        {
            if (world.nom(x, y) > sealevel)
                hitland = 1;
            else
                north++;
        }

        if (hitland == 1)
            n = 1000;
    }

    // South

    hitland = 0;
    x = startx;
    y = starty;

    for (int n = 1; n <= 1000; n++)
    {
        y++;

        if (y > height)
            hitland = 1;
        else
        {
            if (world.nom(x, y) > sealevel)
                hitland = 1;
            else
                south++;
        }

        if (hitland == 1)
            n = 1000;
    }

    // East

    hitland = 0;
    x = startx;
    y = starty;

    for (int n = 1; n <= 1000; n++)
    {
        x++;

        if (x > width)
            x = 0;
        else
        {
            if (world.nom(x, y) > sealevel)
                hitland = 1;
            else
                east++;
        }

        if (hitland == 1)
            n = 1000;
    }

    // West

    hitland = 0;
    x = startx;
    y = starty;

    for (int n = 1; n <= 1000; n++)
    {
        x--;

        if (x < 0)
            x = width;
        else
        {
            if (world.nom(x, y) > sealevel)
                hitland = 1;
            else
                west++;
        }

        if (hitland == 1)
            n = 1000;
    }

    // Northeast

    hitland = 0;
    x = startx;
    y = starty;

    for (int n = 1; n <= 1000; n++)
    {
        x++;

        if (x > width)
            x = 0;

        y--;

        if (y < 0)
            hitland = 1;
        else
        {
            if (world.nom(x, y) > sealevel)
                hitland = 1;
            else
                northeast++;
        }

        if (hitland == 1)
            n = 1000;
    }

    // Southeast

    hitland = 0;
    x = startx;
    y = starty;

    for (int n = 1; n <= 1000; n++)
    {
        x++;

        if (x > width)
            x = 0;

        y++;

        if (y > height)
            hitland = 1;
        else
        {
            if (world.nom(x, y) > sealevel)
                hitland = 1;
            else
                southeast++;
        }

        if (hitland == 1)
            n = 1000;
    }

    // Southwest

    hitland = 0;
    x = startx;
    y = starty;

    for (int n = 1; n <= 1000; n++)
    {
        x--;

        if (x < 0)
            x = width;

        y++;

        if (y > height)
            hitland = 1;
        else
        {
            if (world.nom(x, y) > sealevel)
                hitland = 1;
            else
                southwest++;
        }

        if (hitland == 1)
            n = 1000;
    }

    // Northwest

    hitland = 0;
    x = startx;
    y = starty;

    for (int n = 1; n <= 1000; n++)
    {
        x--;

        if (x < 0)
            x = width;

        y--;

        if (y < 0)
            hitland = 1;
        else
        {
            if (world.nom(x, y) > sealevel)
                hitland = 1;
            else
                northwest++;
        }

        if (hitland == 1)
            n = 1000;
    }

    float total = (float)(north + south + east + west + southeast + southwest + northeast + northwest);

    total = total * mult1;

    total = total * world.lunar(); // Lunar pull multiplies tides.

    int tide = (int)total; // In theory this could be as high as 80, but never in practice assuming lunar = 1.0. The highest in the world is 16 metres, so divide this value by 4 to get it in metres.

    return (tide);
}

// This function puts river deltas on the global map.

void createriverdeltas(planet& world)
{
    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();
    int glaciertemp = world.glaciertemp();
    float gravity = world.gravity();

    vector<vector<int>> deltarivers(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0)); // Marks all the rivers that have been turned into deltas.

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
        for (int j = 0; j <= height; j++)
        {
            int riverjan = world.riverjan(i, j);
            int riverjul = world.riverjul(i, j);

            if ((riverjan != 0 || riverjul != 0) && world.avetemp(i, j) > glaciertemp) // There's a river here and it's not too cold
            {
                int flowtotal = riverjan + riverjul;

                if (random(1, deltachance) < flowtotal)
                {
                    if (world.sea(i, j) == 0)
                    {
                        destpoint = getflowdestination(world, i, j, 0);

                        if (destpoint.x != -1 && destpoint.y != -1)
                        {
                            if (world.sea(destpoint.x, destpoint.y) == 1)
                            {
                                int tide = world.tide(i, j);

                                bool found = 0;

                                if (random(1, tidefactor) > tide)
                                {
                                    for (int k = i - range; k <= i + range; k++)
                                    {
                                        int kk = k;

                                        if (kk<0 || kk>width)
                                            kk = wrap(kk, width);

                                        for (int l = j - range; l <= j + range; l++)
                                        {
                                            if (l >= 0 && l <= height)
                                            {
                                                if (world.deltadir(kk, l) != 0)
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

                                        int upriver = (world.riveraveflow(x, y) / 10000) + randomsign(random(0, 3));

                                        if (upriver < 2)
                                            upriver = 2;

                                        if (upriver > 10)
                                            upriver = 10;

                                        //upriver=upriver*2; // Just to make them bigger to check more easily!

                                        // Upriver is how far upstream from the coast we'll start the delta.

                                        placedelta(world, x, y, upriver, deltarivers);
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
        for (int j = 0; j <= height; j++)
        {
            if (world.deltadir(i, j) > 0)
            {
                for (int k = i - 1; k <= i + 1; k++)
                {
                    int kk = k;

                    if (kk<0 || kk>width)
                        kk = wrap(kk, width);

                    for (int l = j - 1; l <= j + 1; l++)
                    {
                        if (l >= 0 && l <= height)
                        {
                            if (world.sea(kk, l) == 0)
                            {
                                world.setnom(kk, l, sealevel + 1);

                                if (world.deltadir(kk, l) == 0)
                                    world.setdeltadir(kk, l, -1);
                            }
                        }
                    }
                }
            }
        }
    }

    // Now we lower any land that borders flat land.

    vector<vector<int>> donethese(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (world.nom(i, j) == sealevel + 1)
            {
                for (int k = i - 1; k <= i + 1; k++)
                {
                    int kk = k;

                    if (kk<0 || kk>width)
                        kk = wrap(kk, width);

                    for (int l = j - 1; l <= j + 1; l++)
                    {
                        if (l >= 0 && l <= height)
                        {
                            if (donethese[kk][l] == 0 && world.nom(kk, l) > sealevel + 1)
                            {
                                int elev = world.nom(kk, l) - sealevel;
                                elev = elev / 2;

                                world.setnom(kk, l, sealevel + elev);

                                donethese[kk][l] = 1;
                            }
                        }
                    }
                }
            }
        }
    }

    // Now we divert other rivers that flow through the delta areas, so they flow more naturally in the delta shape.

    divertdeltarivers(world, deltarivers);
}

// This creates a river delta.

void placedelta(planet& world, int centrex, int centrey, int upriver, vector<vector<int>>& deltarivers)
{
    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();

    int dir = world.riverdir(centrex, centrey);
    int maxbranches = 40;
    int branchestotal = 0;
    int branches = maxbranches; //20; //random(upriver*2,upriver*4); // Number of branches this delta will (hopefully) have.
    int size = upriver; // Possible distance from the centre that the branches can end.
    int addition = 2; // Amount to push branch ends out to sea.
    int x, y;

    if (branches > maxbranches)
        branches = maxbranches;

    twointegers destpoint;

    vector<vector<int>> branchends(maxbranches + 1, vector<int>(2));

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

        if (world.sea(x, y) == 0)
        {
            destpoint = nearestsea(world, x, y, 0, 10, 1);

            x = destpoint.x;
            y = destpoint.y;
        }

        if (x != -1 && y != -1)
        {
            bool goahead = 1;

            if (world.deltadir(x, y) != 0 && random(1, 2) == 1)
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
        deltarivers[x][y] = 1; // Mark this bit of river to show that it's part of a river that's being turned into a delta.
        destpoint = getupstreamcell(world, x, y);

        x = destpoint.x;
        y = destpoint.y;

        if (x == -1 || y == -1)
            return;
    }

    deltarivers[x][y] = 1;

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

        if (world.sea(x, y) == 0)
        {
            destpoint = nearestsea(world, x, y, 0, 10, 1);

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

    float janflow = (float)world.riverjan(targetx, targety);
    float julflow = (float)world.riverjul(targetx, targety);

    float branchjanflow = janflow / (float)(branchestotal + 1);
    float branchjulflow = julflow / (float)(branchestotal + 1);

    // Now do each branch in turn.

    for (int branch = 1; branch <= branchestotal; branch++)
    {
        x = branchends[branch][0];
        y = branchends[branch][1];

        world.setdeltajan(x, y, (int)branchjanflow);
        world.setdeltajul(x, y, (int)branchjulflow);

        for (int n = 1; n <= 100; n++)
        {
            if (world.deltadir(x, y) == 0) // If we're drawing a new branch route
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
                    world.setdeltadir(x, y, dir);
                }
            }
            else
                dir = world.deltadir(x, y);

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

            if (y >= 0 && y <= height)
            {
                world.setdeltajan(x, y, world.deltajan(x, y) + (int)branchjanflow);
                world.setdeltajul(x, y, world.deltajul(x, y) + (int)branchjulflow);

                if (world.nom(x, y) > sealevel)
                    world.setnom(x, y, sealevel + 1);

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

        for (int j = targety - 1; j <= targety + 1; j++)
        {
            if (j >= 0 && j <= height)
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

                    for (int l = j - 1; l <= j + 1; l++)
                    {
                        if (l >= 0 && l <= height)
                        {
                            if (kk != targetx || l != targety) // Don't do the actual destination tile
                            {
                                int dir = world.deltadir(kk, l);

                                if (dir > 0) // If there's a delta branch here
                                {
                                    twointegers destpoint;

                                    destpoint = getflowdestination(world, kk, l, dir);

                                    if (destpoint.x == i && destpoint.y == j)
                                    {
                                        janflow = janflow + world.deltajan(kk, l);
                                        julflow = julflow + world.deltajul(kk, l);
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

                    world.setdeltadir(ii, j, dir);
                    world.setdeltajan(ii, j, janflow);
                    world.setdeltajul(ii, j, julflow);
                }
            }
        }
    }

    // Now reduce the river below that point.

    vector<vector<int>> removedrivers(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

    int janreduce = world.riverjan(targetx, targety) - (int)(branchjanflow * 4.0f); // Shouldn't really be multiplied, but this is to make it more visible on the map.
    int julreduce = world.riverjul(targetx, targety) - (int)(branchjulflow * 4.0f);

    reduceriver(world, janreduce, julreduce, removedrivers, 1, targetx, targety);

    //Now we simply make one final delta cell in the target cell, bringing the branches back together to meet the main river.

    int ux = -1;
    int uy = -1;

    x = targetx;
    y = targety;

    destpoint = getupstreamcell(world, x, y);

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

    world.setdeltadir(x, y, dir);

    world.setdeltajan(x, y, 0 - (int)(branchjanflow * (float)branchestotal));
    world.setdeltajul(x, y, 0 - (int)(branchjulflow * (float)branchestotal));

    deltarivers[x][y] = 1;

}

// This function diverts rivers that run through deltas, so that they follow the pattern of the delta branches more closely.

void divertdeltarivers(planet& world, vector<vector<int>>& deltarivers)
{
    int width = world.width();
    int height = world.height();

    vector<vector<int>> removedrivers(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));
    vector<vector<int>> checked(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

    int riverno = 0;

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (checked[i][j] == 0 && world.sea(i, j) == 0 && world.riverdir(i, j) != 0)
            {
                int x = i;
                int y = j;

                bool keepgoing = 1;
                bool found = 0;
                bool alreadydeleted = 0;

                do
                {                    
                    checked[x][y] = 1; // Mark this as checked so we won't try to follow any rivers from this point again.

                    int dir = world.riverdir(x, y);

                    if (dir == 8 || dir == 1 || dir == 2)
                        y--;

                    if (dir == 4 || dir == 5 || dir == 6)
                        y++;

                    if (dir == 2 || dir == 3 || dir == 4)
                        x++;

                    if (dir == 6 || dir == 7 || dir == 8)
                        x--;

                    if (x<0 || x>width)
                        x = wrap(x, width);

                    if (y < 0)
                        y = 0;

                    if (y > height)
                        y = height;

                    if (world.sea(x, y) == 1) // If it reaches the sea
                        keepgoing = 0;

                    if (checked[x][y] == 1) // If it reaches a river that's already been done
                    {
                        keepgoing = 0;
                    }

                    if (deltarivers[x][y] == 1) // If it reaches a river that's part of the delta
                    {
                        keepgoing = 0;
                    }

                    if (keepgoing == 1 && (world.deltajan(x, y) != 0 || world.deltajul(x, y) != 0)) // If it reaches an actual delta
                    {
                        keepgoing = 0;
                        found = 1;
                    }

                    // Now check whether it's about to cross over a delta diagonally. If it is, we will need to divert it into one of the neighbouring delta cells.

                    dir = world.riverdir(x, y);

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

                            if (dx > width)
                                dx = 0;

                            if (dy >= 0 && (world.deltajan(dx, dy) == 0 || world.deltajul(dx, dy) == 0)) // If this river isn't about to run into a delta branch
                            {
                                // Check the ones to the sides to see if they contain a delta branch.

                                mx = x;
                                my = y - 1;

                                nx = x + 1;
                                ny = y;

                                if (nx > width)
                                    nx = 0;

                                if (my >= 0 && (world.deltajan(mx, my) != 0 || world.deltajul(mx, my) != 0))
                                    mfound = 1;

                                if (world.deltajan(nx, ny) != 0 || world.deltajul(nx, ny) != 0)
                                    nfound = 1;
                            }
                        }

                        if (dir == 4)
                        {
                            dx = x + 1;
                            dy = y + 1;

                            if (dx > width)
                                dx = 0;

                            if (dy <= height && (world.deltajan(dx, dy) == 0 || world.deltajul(dx, dy) == 0)) // If this river isn't about to run into a delta branch
                            {
                                // Check the ones to the sides to see if they contain a delta branch.

                                mx = x;
                                my = y + 1;

                                nx = x + 1;
                                ny = y;

                                if (nx > width)
                                    nx = 0;

                                if (my <= height && (world.deltajan(mx, my) != 0 || world.deltajul(mx, my) != 0))
                                    mfound = 1;

                                if (world.deltajan(nx, ny) != 0 || world.deltajul(nx, ny) != 0)
                                    nfound = 1;
                            }
                        }

                        if (dir == 6)
                        {
                            dx = x - 1;
                            dy = y + 1;

                            if (dx < 0)
                                dx = width;

                            if (dy <= height && (world.deltajan(dx, dy) == 0 || world.deltajul(dx, dy) == 0)) // If this river isn't about to run into a delta branch
                            {
                                // Check the ones to the sides to see if they contain a delta branch.

                                mx = x - 1;
                                my = y;

                                nx = x;
                                ny = y + 1;

                                if (mx < 0)
                                    mx = width;

                                if (world.deltajan(mx, my) != 0 || world.deltajul(mx, my) != 0)
                                    mfound = 1;

                                if (ny <= height && (world.deltajan(nx, ny) != 0 || world.deltajul(nx, ny) != 0))
                                    nfound = 1;
                            }
                        }

                        if (dir == 8)
                        {
                            dx = x - 1;
                            dy = y - 1;

                            if (dx < 0)
                                dx = width;

                            if (dy >= 0 && (world.deltajan(dx, dy) == 0 || world.deltajul(dx, dy) == 0)) // If this river isn't about to run into a delta branch
                            {
                                // Check the ones to the sides to see if they contain a delta branch.

                                mx = x;
                                my = y - 1;

                                nx = x - 1;
                                ny = y;

                                if (nx < 0)
                                    nx = width;

                                if (my >= 0 && (world.deltajan(mx, my) != 0 || world.deltajul(mx, my) != 0))
                                    mfound = 1;

                                if (world.deltajan(nx, ny) != 0 || world.deltajul(nx, ny) != 0)
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
                                twointegers dest = getflowdestination(world, mx, my, world.deltadir(mx, my));

                                if (dest.x == nx && dest.y == ny) // m is flowing into n. That means we want to divert into m, as it's really downstream.
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

                            int janload = world.riverjan(x, y);
                            int julload = world.riverjul(x, y);

                            riverno++;

                            removeriver(world, removedrivers, riverno, x, y);

                            // Now recreate it in this cell, pointing to the new destination cell.

                            world.setriverjan(x, y, janload);
                            world.setriverjul(x, y, julload);
                            world.setriverdir(x, y, getdir(x, y, tx, ty));

                            alreadydeleted = 1;

                            // Now on the next pass of this loop, the program will move x and y in this new direction and discover that it's on a delta branch!
                        }
                    }
                } while (keepgoing == 1);

                if (found == 1) // If we've actually hit a delta, time to sort out this river!
                {
                    int janload = world.riverjan(x, y);
                    int julload = world.riverjul(x, y);

                    // First, delete the river from this point onwards.

                    if (alreadydeleted == 0)
                    {
                        riverno++;

                        removeriver(world, removedrivers, riverno, x, y);
                    }

                    // Now, recreate the river, following the line of the delta branch.

                    keepgoing = 1;
                    twointegers dest, check;

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
                            int kk = k;

                            if (kk<0 || kk>width)
                                kk = wrap(kk, width);

                            for (int l = y - 1; l <= y + 1; l++)
                            {
                                if (l >= 0 && l <= height)
                                {
                                    if ((k != x || l != y) && world.deltadir(kk, l) != 0)
                                    {
                                        check = getflowdestination(world, kk, l, world.deltadir(kk, l));

                                        if (check.x == x && check.y == y)
                                        {
                                            dest.x = kk;
                                            dest.y = l;

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

                            world.setriverdir(x, y, dir);
                            world.setriverjan(x, y, world.riverjan(x, y) + janload);
                            world.setriverjul(x, y, world.riverjul(x, y) + julload);

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
                        }

                        if (tally > 10000) // In case of infinite loops...
                            keepgoing = 0;

                    } while (keepgoing == 1);
                }
            }
        }
    }
}

// This goes over the map and ensures that rivers don't inexplicably grow too much within delta regions.

void checkrivers(planet& world)
{
    //highres_timer_t timer("Check Rivers"); // 666: 4156, 999: 6442 => 666: 2926, 999: 2907
    int width = world.width();
    int height = world.height();

    int maxsource = 50; // Largest size a river source can be (jan and jul each)

    // First, go through and make sure there aren't any absurd river sources (these may be accidentally created in the delta regions)

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (world.deltadir(i, j) != 0 && checkwaterinflow(world, i, j) == 0) // If nothing is flowing into this cell, it's a river source
            {
                if (world.riverjan(i, j) > maxsource)
                    world.setriverjan(i, j, maxsource);

                if (world.riverjul(i, j) > maxsource)
                    world.setriverjul(i, j, maxsource);
            }
        }
    }

    // Now go through and ensure that no river grows weirdly for no apparent reason.

    bool found = 0;

    int amount = 4;

    do
    {
        found = 0;

        for (int i = 0; i <= width; i++)
        {
            for (int j = 0; j <= height; j++)
            {
                bool neardelta = 0;

                for (int k = i - amount; k <= i + amount; k++)
                {
                    int kk = k;

                    if (kk<0 || kk>width)
                        kk = wrap(kk, width);

                    for (int l = j - amount; l <= j + amount; l++)
                    {
                        if (l >= 0 && l <= height)
                        {
                            if (world.deltadir_no_bounds_check(kk, l) != 0)
                            {
                                neardelta = 1;
                                k = i + amount;
                                l = j + amount;
                            }
                        }
                    }
                }

                if (neardelta == 1) // Only look at cells near river deltas.
                {
                    twointegers inflow = gettotalinflow(world, i, j);

                    if (world.riverjan(i, j) > inflow.x)
                    {
                        world.setriverjan(i, j, inflow.x);
                        found = 1;
                    }

                    if (world.riverjul(i, j) > inflow.y)
                    {
                        world.setriverjul(i, j, inflow.y);
                        found = 1;
                    }
                }
            }
        }
    } while (found == 1);
}

// This lays down wetlands.

void createwetlands(planet& world, boolshapetemplate smalllake[])
{
    int width = world.width();
    int height = world.height();
    int maxelev = world.maxelevation();

    int minflow = 100; // Average flow must be at least this to have wetlands.
    int maxflatness = 5; // Must be no less flat than this.
    float factor = (float)maxelev / 5.0f;

    // First, create a fractal to be the drainage map.

    int grain = 8; // This is the level of detail on this map.
    float valuemod = 0.2f;

    int v = random(3, 6);
    float valuemod2 = (float)v;
    int shapenumber;

    vector<vector<int>> drainage(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

    createfractal(drainage, width, height, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    // Now go through the map and place wetlands.

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (world.sea(i, j) == 0 && world.special(i, j) == 0 && world.lakesurface(i, j) == 0 && world.climate(i, j) != 31 && world.climate(i, j) != 5)
            {
                if (world.deltadir(i, j) != 0)
                {
                    shapenumber = random(0, 5);
                    pastewetland(world, i, j, shapenumber, smalllake);
                }
                else
                {
                    if (world.riveraveflow(i, j) >= minflow)
                    {
                        int flatness = getflatness(world, i, j);

                        if (flatness <= maxflatness)
                        {
                            float drain = (float)drainage[i][j];

                            drain = drain / factor;

                            int d = (int)drain; // This should be from 1 to 5.
                            d = d * 2;

                            int prob = flatness + d; // The flatter the land, the more likely wetlands are.

                            int flow = world.riveraveflow(i, j) / 1000;

                            prob = prob - flow; // The bigger the flow here, the more likely wetlands are.

                            for (int k = i - 1; k <= i + 1; k++) // The more lakes/sea there are around here, the more likely wetlands are.
                            {
                                int kk = k;

                                if (kk<0 || kk>width)
                                    kk = wrap(kk, width);

                                for (int l = j - 1; l <= j + 1; l++)
                                {
                                    if (l >= 0 && l <= height)
                                    {
                                        if (world.lakesurface(kk, l) != 0)
                                            prob = prob - 3;

                                        if (world.sea(kk, l) == 1)
                                            prob = prob - 4;
                                    }
                                }
                            }

                            //prob=prob+(7-(world.averain(i,j)/60));
                            prob = prob + (40 - (world.averain(i, j) / 10));

                            if (world.climate(i, j) == 30 && world.maxtemp(i, j) >= 5) // Much likelier in tundra with warmish summers
                                prob = prob / 4;

                            if (prob < 1)
                                prob = 1;

                            if (random(1, prob) == 1) // Put some wetlands here.
                            {
                                shapenumber = random(0, 5);
                                pastewetland(world, i, j, shapenumber, smalllake);
                            }
                        }
                    }
                }
            }
        }
    }

    int deltaswampchance = 6; // The BIGGER this is, the more probable wetlands will be around delta branches.

    for (int i = 0; i <= width; i++) // Add wetlands to the actual delta branches
    {
        for (int j = 0; j <= height; j++)
        {
            if (world.deltadir(i, j) > 0 && world.sea(i, j) == 0 && random(1, deltaswampchance) != 1)
                world.setspecial(i, j, 130);

        }
    }

    // Now fill in extra wetlands if necessary to reach the sea or other wetlands.

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (world.sea(i, j) == 0 && world.special(i, j) == 0 && world.climate(i, j) != 31)
            {
                int westx = i - 1;
                if (westx < 0)
                    westx = width;

                int eastx = i + 1;
                if (eastx > width)
                    eastx = 0;

                int northy = j - 1;
                int southy = j + 1;

                if (world.special(i, northy) == 130)
                {
                    if (world.special(i, southy) == 130)
                        world.setspecial(i, j, 130);

                    if (world.sea(i, southy) == 1)
                        world.setspecial(i, j, 130);
                }

                if (world.special(i, southy) == 130)
                {
                    if (world.special(i, northy) == 130)
                        world.setspecial(i, j, 130);

                    if (world.sea(i, northy) == 1)
                        world.setspecial(i, j, 130);
                }

                if (world.special(westx, j) == 130)
                {
                    if (world.special(eastx, j) == 130)
                        world.setspecial(i, j, 130);

                    if (world.sea(eastx, j) == 1)
                        world.setspecial(i, j, 130);
                }

                if (world.special(eastx, j) == 130)
                {
                    if (world.special(westx, j) == 130)
                        world.setspecial(i, j, 130);

                    if (world.sea(westx, j) == 1)
                        world.setspecial(i, j, 130);
                }

                if (world.special(westx, northy) == 130)
                {
                    if (world.special(eastx, southy) == 130)
                        world.setspecial(i, j, 130);

                    if (world.sea(eastx, southy) == 1)
                        world.setspecial(i, j, 130);
                }

                if (world.special(eastx, northy) == 130)
                {
                    if (world.special(westx, southy) == 130)
                        world.setspecial(i, j, 130);

                    if (world.sea(westx, southy) == 1)
                        world.setspecial(i, j, 130);
                }

                if (world.special(westx, southy) == 130)
                {
                    if (world.special(eastx, northy) == 130)
                        world.setspecial(i, j, 130);

                    if (world.sea(eastx, northy) == 1)
                        world.setspecial(i, j, 130);
                }

                if (world.special(eastx, southy) == 130)
                {
                    if (world.special(westx, northy) == 130)
                        world.setspecial(i, j, 130);

                    if (world.sea(westx, northy) == 1)
                        world.setspecial(i, j, 130);
                }
            }
        }
    }

    for (int i = 0; i <= width; i++) // For some reason it keeps putting wetlands all over the sea, so we remove them.
    {
        for (int j = 0; j <= height; j++)
        {
            if (world.sea(i, j) == 1)
                world.setspecial(i, j, 0);

        }
    }

    // Now make the wetlands salty/brackish if necessary.

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (world.special(i, j) == 130)
            {
                int salt = 0;

                for (int k = i - 1; k <= i + 1; k++)
                {
                    int kk = k;

                    if (kk<0 || kk>width)
                        kk = wrap(kk, width);

                    for (int l = j - 1; l <= j + 1; l++)
                    {
                        if (l >= 0 && l <= height)
                        {
                            if (world.sea(kk, l) == 1)
                                salt = 1;

                            if (world.special(kk, l) == 100 || world.special(kk, l) == 110)
                                salt = 1;
                        }
                    }
                }

                if (salt == 1)
                    world.setspecial(i, j, 132);
                else
                {
                    for (int k = i - 2; k <= i + 2; k++)
                    {
                        int kk = k;

                        if (kk<0 || kk>width)
                            kk = wrap(kk, width);

                        for (int l = j - 2; l <= j + 2; l++)
                        {
                            if (l >= 0 && l <= height)
                            {
                                if (world.sea(kk, l) == 1)
                                    salt = 1;

                                if (world.special(kk, l) == 100 || world.special(kk, l) == 110)
                                    salt = 1;
                            }
                        }
                    }

                    if (salt == 1)
                        world.setspecial(i, j, 131);
                }
            }
        }
    }
}

// This actually pastes a patch of wetlands onto the map.

void pastewetland(planet& world, int centrex, int centrey, int shapenumber, boolshapetemplate smalllake[])
{
    int width = world.width();
    int height = world.height();

    if (centrey == 0 || centrey == height)
        return;

    int imheight = smalllake[shapenumber].ysize() - 1;
    int imwidth = smalllake[shapenumber].xsize() - 1;

    int x = centrex - imwidth / 2; // Coordinates of the top left corner.
    int y = centrey - imheight / 2;

    if (x<0 || x>width)
        x = wrap(x, width);

    int leftx = centrex;
    int lefty = centrey;
    int rightx = centrex;
    int righty = centrey; // These are the coordinates of the furthermost pixels of the swamp.
    bool wrapped = 0; // If this is 1, the swamp wraps over the edge of the map.

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

    for (int i = istart; i != desti; i = i + istep)
    {
        for (int j = jstart; j != destj; j = j + jstep)
        {
            if (smalllake[shapenumber].point(i, j) == 1)
            {
                int xx = x + i;
                int yy = y + j;

                if (yy >= 0 && yy <= height)
                {
                    if (xx<0 || xx>width)
                    {
                        xx = wrap(xx, width);
                        wrapped = 1;
                    }

                    if (world.sea(xx, yy) == 0 && world.mountainheight(xx, yy) == 0) // Don't try to put wetlands on top of sea or mountains...
                    {
                        world.setspecial(xx, yy, 130);

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
    }

    if (wrapped == 1)
    {
        leftx = 0;
        lefty = 0;
        rightx = width;
        righty = height;
    }
}

// This removes any wetlands tiles that border large lakes.

void removeexcesswetlands(planet& world)
{
    int width = world.width();
    int height = world.height();

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (world.special(i, j) == 130)
            {
                bool nearlake = 0;

                for (int k = i - 1; k <= i + 1; k++)
                {
                    int kk = k;

                    if (kk<0 || kk>width)
                        kk = wrap(kk, width);

                    for (int l = j - 1; l <= j + 1; l++)
                    {
                        if (l >= 0 && l <= height)
                        {
                            if (world.truelake(kk, l) == 1)
                            {
                                nearlake = 1;
                                k = i + 1;
                                l = j + 1;
                            }
                        }
                    }
                }

                if (nearlake == 1)
                {
                    world.setlakesurface(i, j, 0);
                    world.setspecial(i, j, 0);
                }
            }
        }
    }
}

// This refines the roughness map so that it takes into account more factors, producing a roughness factor that will be used for the fractal terrain in the regional map.

void refineroughnessmap(planet& world)
{
    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();
    int maxelev = world.maxelevation();
    float maxvaluemod = 1.0f; // Maximum that a valuemod can be.
    float maxcoastalvaluemod = 0.1f; //0.08; // Maximum that it can at the coasts.

    // This array will hold the valuemod for every tile on the map. The valuemod is used in the regional map to determine how rough the diamond-square routine makes the terrain.

    vector<vector<float>> valuemod(ARRAYWIDTH, vector<float>(ARRAYHEIGHT, 0));

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (world.sea(i, j) == 0)
            {
                int eax = i + 1;
                int eay = j;
                if (eax > width)
                    eax = 0;

                int wex = i - 1;
                int wey = j;
                if (wex < 0)
                    wex = width;

                int nox = i;
                int noy = j - 1;
                if (noy < 0)
                    noy = 0;

                int sox = i;
                int soy = j + 1;
                if (soy > height)
                    soy = height;

                int sex = eax;
                int sey = soy;
                /*
                int aveheight = (world.nom(i, j) + world.nom(eax, eay) + world.nom(sox, soy) + world.nom(sex, sey)) / 4;
                */
                if (world.nom(i, j) <= sealevel && world.nom(nox, noy) <= sealevel && world.nom(sox, soy) <= sealevel && world.nom(eax, eay) <= sealevel && world.nom(wex, wey) <= sealevel) // Sea beds will be very smooth.
                {
                    float m = 0.08f / (float)maxelev;
                    float n = world.roughness(i, j);
                    float extrabit = 0.01f * (float)random(1, 7);

                    valuemod[i][j] = (m * n) + extrabit;
                }
                else // On land, the higher it is, the rougher it is.
                {
                    valuemod[i][j] = world.roughness(i, j) / (float)maxelev;
                    /*
                    float mult = 3.0f / (float)(maxelev - (sealevel - 50)); // 6.0f
                    float rough = world.roughness(i, j);
                    rough = rough / (float)maxelev;

                    valuemod[i][j] = ((float)aveheight - ((float)sealevel - 50.0f)) * mult * rough;

                    // Alter it according to how flat this whole area is

                    float diff = 0.0f;

                    diff = diff + (float)abs(world.map(i, j) - world.map(eax, eay));
                    diff = diff + (float)abs(world.map(sox, soy) - world.map(sex, sey));
                    diff = diff + (float)abs(world.map(i, j) - world.map(sox, soy));
                    diff = diff + (float)abs(world.map(eax, eay) - world.map(sex, sey));

                    diff = diff / 20.0f;

                    // Alter it according to how mountainous the area is.

                    float mountain = (float)world.mountainheight(i, j);

                    if (mountain == 0.0f)
                    {
                        for (int k = i - 1; k <= i + 1; k++)
                        {
                            int kk = k;

                            if (kk<0 || kk>width)
                                kk = wrap(kk, width);

                            for (int l = j - 1; l <= j + 1; l++)
                            {
                                if (l >= 0 && l <= height)
                                {
                                    if (world.mountainheight(kk, l) > 0)
                                        mountain = (float)world.mountainheight(kk, l) * 0.7f;
                                }
                            }
                        }

                        if (mountain == 0.0f)
                        {
                            for (int k = i - 3; k <= i + 3; k++)
                            {
                                int kk = k;

                                if (kk<0 || kk>width)
                                    kk = wrap(kk, width);

                                for (int l = j - 3; l <= j + 3; l++)
                                {
                                    if (l >= 0 && l <= height)
                                    {
                                        if (world.mountainheight(kk, l) > 0)
                                            mountain = (float)world.mountainheight(kk, l) * 0.3f;
                                    }
                                }
                            }
                        }
                    }

                    mountain = mountain / 100.0f; // 50.0f;

                    diff = diff + mountain;

                    if (diff > 8.0f)
                        diff = 8.0f;

                    valuemod[i][j] = valuemod[i][j] * diff;
                    */
                }
            }
            else // Sea beds - very smooth.
            {

                float m = 0.08f / (float)maxelev;
                float n = world.roughness(i, j);
                float extrabit = 0.01f * (float)random(1, 7);

                valuemod[i][j] = (m * n) + extrabit;
            }
        }
    }

    // Now we blur that, so that there aren't sharp transitions in roughness on the regional map.

    /*
    int amount = 2; // Amount to blur by.

    vector<vector<float>> valuemod2(ARRAYWIDTH, vector<float>(ARRAYHEIGHT, 0));

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (world.nom(i, j) > sealevel && world.truelake(i, j) == 0)
            {
                bool goahead = 1;

                for (int k = i - 1; k <= i + 1; k++)
                {
                    int kk = k;

                    if (kk<0 || kk>width)
                        kk = wrap(kk, width);

                    for (int l = j - 1; l <= j + 1; l++)
                    {
                        if (l >= 0 && l <= height)
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
                            if (l >= 0 && l <= height)
                            {
                                if (world.nom(kk, l) > sealevel && world.truelake(kk, l) == 0)
                                {
                                    total = total + valuemod[kk][l];
                                    crount++;
                                }
                            }
                        }
                    }
                    valuemod2[i][j] = total / crount;

                    if (valuemod2[i][j] > maxvaluemod)
                        valuemod2[i][j] = maxvaluemod;
                }
                else
                {
                    valuemod2[i][j] = valuemod[i][j];

                    if (valuemod2[i][j] > maxcoastalvaluemod)
                        valuemod2[i][j] = maxcoastalvaluemod;
                }
            }
            else
                valuemod2[i][j] = 0.0f;
        }
    }
    */

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (valuemod[i][j] < 0.0f)
                world.setroughness(i, j, 0.0f);
            else
                world.setroughness(i, j, valuemod[i][j]);
        }
    }
}

// This ensures that the climates at the bottom of the map are correct.

void checkpoleclimates(planet& world)
{
    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();

    for (int i = 0; i <= width; i++)
    {
        world.setwinterrain(i, height, world.winterrain(i, height - 1));
        world.setsummerrain(i, height, world.summerrain(i, height - 1));
        world.setmaxtemp(i, height, world.maxtemp(i, height - 1));
        world.setmintemp(i, height, world.mintemp(i, height - 1));
    }
}

// This removes any cells of sea that are next to lakes.

void removesealakes(planet& world)
{
    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            int surface = world.lakesurface(i, j);

            if (surface != 0)
            {
                for (int k = i - 1; k <= i + 1; k++)
                {
                    for (int l = j - 1; l <= j + 1; l++)
                    {
                        if (world.nom(k, l) <= sealevel && world.lakesurface(k, l) == 0)
                            world.setlakesurface(k, l, surface);
                    }
                }
            }
        }
    }
}

// This ensures that lakes don't have fragments of themselves nearby.

void connectlakes(planet& world)
{
    int width = world.width();
    int height = world.height();

    for (int i = 0; i <= width; i++)
    {
        for (int j = 2; j < height - 1; j++)
        {
            if (world.lakesurface(i, j) != 0)
            {
                int left = i - 1;
                int right = i + 1;
                int up = j - 1;
                int down = j + 1;

                if (left < 0)
                    left = width;

                if (right > width)
                    right = 0;

                int lleft = left - 1;
                int rright = right + 1;
                int uup = up - 1;
                int ddown = down + 1;

                if (lleft < 0)
                    lleft = width;

                if (rright > width)
                    rright = 0;

                int surface = world.lakesurface(i, j);
                int elev = world.nom(i, j);
                int special = world.special(i, j);

                if (world.lakesurface(right, j) == 0 && world.lakesurface(rright, j) == surface)
                {
                    world.setnom(right, j, elev);
                    world.setspecial(right, j, special);
                    world.setlakesurface(right, j, surface);
                }

                if (world.lakesurface(left, j) == 0 && world.lakesurface(lleft, j) == surface)
                {
                    world.setnom(left, j, elev);
                    world.setspecial(left, j, special);
                    world.setlakesurface(left, j, surface);
                }

                if (world.lakesurface(i, up) == 0 && world.lakesurface(i, uup) == surface)
                {
                    world.setnom(i, up, elev);
                    world.setspecial(i, up, special);
                    world.setlakesurface(i, uup, surface);
                }

                if (world.lakesurface(i, down) == 0 && world.lakesurface(i, ddown) == surface)
                {
                    world.setnom(i, down, elev);
                    world.setspecial(i, up, special);
                    world.setlakesurface(i, ddown, surface);
                }
            }
        }
    }
}
