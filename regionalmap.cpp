//
//  regionalmap.cpp
//  Undiscovered Worlds
//
//  Created by Jonathan Hill on 01/11/2019.
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

#define REGIONALTILEWIDTH 32 // Height and width of the regional map measured in tiles
#define REGIONALTILEHEIGHT 32

// This function creates the region.

void generateregionalmap(planet& world, region& region, boolshapetemplate smalllake[], boolshapetemplate island[], peaktemplate& peaks, vector<vector<float>>& riftblob, int riftblobsize, int partial, byteshapetemplate smudge[], byteshapetemplate smallsmudge[], vector<int>& squareroot)
{
    int xleft = 0;
    int xright = 35;
    int ytop = 0;
    int ybottom = 35;

    if (partial == 7)
        xright = 2 + STRIPWIDTH;

    int xstart = xleft * 16;
    int xend = xright * 16 + 16;
    int ystart = ytop * 16;
    int yend = ybottom * 16 + 16; // These define the actual area of the regional map that we're creating (with some margin).

    int leftx = region.leftx();
    int lefty = region.lefty(); // These mark the upper left corner of the part of the global map we're expanding on.

    int regwidthbegin = region.regwidthbegin();
    int regwidthend = region.regwidthend();
    int regheightbegin = region.regheightbegin();
    int regheightend = region.regheightend();

    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();
    int maxelev = world.maxelevation();
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    float warpfactor = 400.0f;

    if (leftx<0 || leftx>width)
    {
        leftx = wrap(leftx, width);
        region.setleftx(leftx);
    }

    initialiseregion(world, region);

    if (partial == 0)
        region.clear();

    vector<vector<bool>> globalestuaries(ARRAYWIDTH, vector<bool>(ARRAYHEIGHT, 0));
    vector<vector<bool>> safesaltlakes(RARRAYWIDTH, vector<bool>(RARRAYHEIGHT, 0));
    vector<vector<bool>> disruptpoints(RARRAYWIDTH, vector<bool>(RARRAYHEIGHT, 0));
    vector<vector<int>> rriverscarved(RARRAYWIDTH, vector<int>(RARRAYHEIGHT, 0));
    vector<vector<int>> fakesourcex(RARRAYWIDTH, vector<int>(RARRAYHEIGHT, 0));
    vector<vector<int>> fakesourcey(RARRAYWIDTH, vector<int>(RARRAYHEIGHT, 0));
    vector<vector<bool>> riverinlets(RARRAYWIDTH, vector<bool>(RARRAYHEIGHT, 0));
    vector<vector<int>> warptox(RARRAYWIDTH, vector<int>(RARRAYHEIGHT, 0));
    vector<vector<int>> warptoy(RARRAYWIDTH, vector<int>(RARRAYHEIGHT, 0));

    vector<vector<int>> source(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

    int coords[4][2];

    // First, work out whether there will be sea visible on this map.

    bool seapresent = 0;

    for (int i = region.centrex() - 17; i <= region.centrex() + 17; i++)
    {
        int ii = i;

        if (ii<0 || ii>width)
            ii = wrap(ii, width);

        for (int j = region.centrey() - 17; j <= region.centrey() + 17; j++)
        {
            if (j >= 0 && j <= height)
            {
                if (world.sea(ii, j) == 1)
                {
                    seapresent = 1;
                    i = region.centrex() + 17;
                    j = region.centrey() + 17;
                }
            }
        }
    }

    // Now, prepare the warp arrays. (Currently only used for submarine terrain and also for warping the coastlines a little.)

    vector<vector<int>> warpright(RARRAYWIDTH, vector<int>(RARRAYHEIGHT, -5000));
    vector<vector<int>> warpdown(RARRAYWIDTH, vector<int>(RARRAYHEIGHT, -5000));

    for (int i = 0; i < RARRAYWIDTH; i++)
    {
        for (int j = 0; j < RARRAYHEIGHT; j++)
        {
            warpright[i][j] = -5000;
            warpdown[i][j] = -5000;
        }
    }

    for (int i = 0; i <= width; i++)
    {
        int ii = i + width / 3;

        if (ii > width)
            ii = ii - width;

        for (int j = 0; j <= height; j++)
            source[i][j] = world.noise(ii, j);
    }

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            float valuemod = 0.04f; //0.02;

            makegenerictile(world, x * 16, y * 16, xx, yy, valuemod, coords, source, warpright, maxelev, 0, 1);
        }
    }

    for (int i = 0; i <= width; i++)
    {
        int ii = i - width / 3;

        if (ii < 0)
            ii = ii + width;

        for (int j = 0; j <= height; j++)
            source[i][j] = world.noise(ii, j);
    }

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            float valuemod = 0.04f; //0.02;

            makegenerictile(world, x * 16, y * 16, xx, yy, valuemod, coords, source, warpdown, maxelev, 0, 1);
        }
    }

    float margin = 120.0f; // 100 // Closer to the edge than this, and the warp effect will be reduced.

    vector<float>marginfactor(RARRAYWIDTH, 0.0f); // Precalculate these, to save time. This is to reduce the warp effect near the edges of the (visible) regional map, to avoid warping in values from off the edge.

    for (int n = 0; n < RARRAYWIDTH; n++)
    {
        float thismarginfactor = 1.0f;

        if (n < regwidthbegin || n > regwidthend)
            thismarginfactor = 0.0f;
        else
        {
            float thismargin = (float)(n - regwidthbegin);

            if (regwidthend - n < (int)thismargin)
                thismargin = (float)(regwidthend - n);

            if (thismargin < margin)
                thismarginfactor = thismargin / margin;
        }

        marginfactor[n] = thismarginfactor;
    }

    for (int i = 0; i < RARRAYWIDTH; i++)
    {
        for (int j = 0; j < RARRAYHEIGHT; j++)
        {
            int x = i;
            int y = j;

            float thiswarpright = (float)warpright[i][j] / maxelev;
            float thiswarpdown = (float)warpdown[i][j] / maxelev;

            thiswarpright = thiswarpright - 0.5f; // So they're in the range -0.5 - 0.5.
            thiswarpdown = thiswarpdown - 0.5f;

            thiswarpright = thiswarpright * warpfactor;
            thiswarpdown = thiswarpdown * warpfactor;

            float thismarginfactor = marginfactor[i];

            if (marginfactor[j] < thismarginfactor)
                thismarginfactor = marginfactor[j];

            thiswarpright = thiswarpright * thismarginfactor;
            thiswarpdown = thiswarpdown * thismarginfactor;

            x = x + (int)thiswarpright;

            if (x < 0)
                x = 0;

            if (x > rwidth)
                x = rwidth;

            y = y + (int)thiswarpdown;

            if (y < 0)
                y = 0;

            if (y > rheight)
                y = rheight;

            warptox[i][j] = x;
            warptoy[i][j] = y;
        }
    } 

    // Counter-intuitively, we do the lakes and rivers first. Rivers are drawn at a somewhat lower elevation than the rest of the tile will be. After that, we draw in the rest of the elevation around them. This creates the effect of rivers carving out valleys in the landscape, when in fact the landscape is built around the rivers.

    makeregionalwater(world, region, safesaltlakes, disruptpoints, rriverscarved, fakesourcex, fakesourcey, smalllake, island, riftblob, riftblobsize, xleft, xright, ytop, ybottom);

    // Only now, surprisingly, do we do basic elevation.

    makeregionalterrain(world, region, disruptpoints, riverinlets, globalestuaries, rriverscarved, fakesourcex, fakesourcey, smalllake, peaks, smallsmudge, xleft, xright, ytop, ybottom, squareroot, warptox, warptoy);

    // Now do the undersea terrain.

    if (seapresent)
        makeregionalunderseaterrain(world, region, peaks, smudge, smallsmudge, xleft, xright, ytop, ybottom, warptox, warptoy);

    // Now do various miscellaneous bits.

    makeregionalmiscellanies(world, region, safesaltlakes, riverinlets, globalestuaries, smalllake, smallsmudge, xleft, xright, ytop, ybottom);

    // Now we do the climates.

    makeregionalclimates(world, region, safesaltlakes, smalllake, xleft, xright, ytop, ybottom);

    // Quick bit of clearing up.

    for (int i = 0; i <= rwidth; i++)
    {
        for (int j = 0; j <= rheight; j++)
        {
            if (region.map(i, j) <= sealevel)
            {
                region.setriverdir(i, j, 0);
                region.setriverjan(i, j, 0);
                region.setriverjul(i, j, 0);
                region.setfakedir(i, j, 0);
                region.setfakejan(i, j, 0);
                region.setriverjul(i, j, 0);
            }
        }
    }

}

// This does the regional rivers and lakes.

void makeregionalwater(planet& world, region& region, vector<vector<bool>>& safesaltlakes, vector<vector<bool>>& disruptpoints, vector<vector<int>>& rriverscarved, vector<vector<int>>& fakesourcex, vector<vector<int>>& fakesourcey, boolshapetemplate smalllake[], boolshapetemplate island[], vector<vector<float>>& riftblob, int riftblobsize, int xleft, int xright, int ytop, int ybottom)
{
    int xstart = xleft * 16;
    int xend = xright * 16 + 16;
    int ystart = ytop * 16;
    int yend = ybottom * 16 + 16; // These define the actual area of the regional map that we're creating (with some margin).

    int leftx = region.leftx();
    int lefty = region.lefty(); // These mark the upper left corner of the part of the global map we're expanding on.

    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();
    int maxelev = world.maxelevation();

    int rwidth = region.rwidth();
    int rheight = region.rheight();

    if (leftx<0 || leftx>width)
    {
        leftx = wrap(leftx, width);
        region.setleftx(leftx);
    }

    vector<vector<int>> source(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));
    vector<vector<int>> destination(RARRAYWIDTH, vector<int>(ARRAYHEIGHT, -5000));
    vector<vector<bool>> lakemap(RARRAYWIDTH, vector<bool>(RARRAYHEIGHT, 0));
    vector<vector<bool>> lakeislands(RARRAYWIDTH, vector<bool>(RARRAYHEIGHT, 0));
    vector<vector<bool>> rivercurves(RARRAYWIDTH, vector<bool>(RARRAYHEIGHT, 0));
    vector<vector<bool>> riftlakemap(RARRAYWIDTH * 2, vector<bool>(RARRAYHEIGHT * 2, 0));

    int coords[4][2];
    
    // First, sort out the roughness.
  
    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop + 1; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            float thisroughness = world.roughness(xx, yy) * 2.0f; // 1.75;

            if (thisroughness < 0.4f)
                thisroughness = 0.4f;

            for (int i = x * 16; i <= x * 16 + 16; i++)
            {
                for (int j = y * 16; j <= y * 16 + 16; j++)
                    region.setroughness(i, j, thisroughness);
            }
        }
    }

    // Now large lakes.

    // First, make templates for the lakes. Start by making a sort of terrain map where all areas below sea level are lake.

    fast_srand(world.seed());

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {            
            if (world.lakesurface(i, j) != 0)
                source[i][j] = sealevel - (100 + random(1, 800));
            else
                source[i][j] = sealevel + (100 + random(1, 800));
        }
    }

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            float valuemod = 0.04f; //0.02;

            makegenerictile(world, x * 16, y * 16, xx, yy, valuemod, coords, source, destination, maxelev, 0, 1);
        }
    }

    // Now we just create a lake template map based on that.

    for (int i = 0; i < RARRAYWIDTH; i++)
    {
        for (int j = 0; j < RARRAYHEIGHT; j++)
        {
            if (destination[i][j] < sealevel)
                lakemap[i][j] = 1;
            else
                lakemap[i][j] = 0;
        }
    }

    for (int i = 0; i < RARRAYWIDTH; i++)
    {
        for (int j = 0; j < RARRAYHEIGHT; j++)
            destination[i][j] = -5000;
    }


    // Now, actually create the lakes using that lake template.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            //int surfacelevel = nearlake(world, xx, yy, 1, 0);

            int surfacelevel = world.lakesurface(xx, yy);

            if (surfacelevel == 0)
            {
                for (int radius = 1; radius <= 5; radius++)
                {
                    for (int i = xx - radius; i <= xx + radius; i++)
                    {
                        int ii = i;

                        if (ii<0 || ii>width)
                            ii = wrap(ii, width);

                        for (int j = yy - radius; j <= yy + radius; j++)
                        {
                            if (j >= 0 && j <= height)
                            {
                                int thissurface = world.lakesurface(ii, j);

                                if (thissurface != 0)
                                {
                                    surfacelevel = thissurface;
                                    i = xx + radius;
                                    j = yy + radius;
                                    radius = 5;
                                }
                            }
                        }
                    }
                }
            }

            if (surfacelevel != 0)
                makelaketile(world, region, x * 16, y * 16, xx, yy, lakemap, surfacelevel, coords, source, destination, safesaltlakes, smalllake);
        }
    }

    // Check for any diagonal sections in the lake coastlines.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            findlakecoastdiagonals(world, region, x * 16, y * 16, xx, yy, disruptpoints);
        }
    }

    // Now remove them.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            removelakecoastdiagonals(world, region, x * 16, y * 16, xx, yy, disruptpoints, smalllake);
        }
    }

    for (int i = 0; i < RARRAYWIDTH; i++)
    {
        for (int j = 0; j < RARRAYHEIGHT; j++)
            disruptpoints[i][j] = 0;
    }

    // Now put islands in those lakes.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            int surfacelevel = nearlake(world, xx, yy, 1, 0);

            if (surfacelevel != 0)
                makelakeislands(world, region, x * 16, y * 16, xx, yy, surfacelevel, island, lakeislands);
        }
    }

    // Now, rivers.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            makerivertile(world, region, x * 16, y * 16, xx, yy, rriverscarved, smalllake, rivercurves);
        }
    }

    // Now, fill in any missing sections of river.

    fixrivers(world, region, xstart, ystart, xend, yend);

    // Now rift lakes.

    int extra = 10; // For these, we will create a *bigger* map than the actual regional area. This is so that we can work out the whole coastline of lakes even if only a part of them appears on the regional map. extra is the amount of additional margin to put around the regional map for this.
   
    for (int x = xleft - extra; x <= xright + extra; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop - extra; y <= ybottom + extra; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            if (world.lakestart(xx, yy) != 0 && world.riftlakesurface(xx, yy) != 0)
                makeriftlaketemplates(world, region, x * 16, y * 16, xx, yy, extra, riftlakemap);
        }
    }

    // We've got the templates for the rift lakes. Now go through the map again and create the lakes from the templates.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            makeriftlaketile(world, region, x * 16, y * 16, xx, yy, extra, riftlakemap, riftblob, riftblobsize);
        }
    }

    // Get rid of rivers that are in mostly sea tiles (otherwise they may reappear on small islands)

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            removeseatilerivers(world, region, x * 16, y * 16, xx, yy);
        }
    }

    // If any rivers inexplicably stop, extend them until they hit sea, lake, or another river.

    finishrivers(world, region, xstart, ystart, xend, yend);

    // Remove any sections of rivers that flow out of lakes and then back into the same lakes.

    for (int x = xleft + 1; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop + 1; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            removeriverlakeloops(world, region, x * 16, y * 16, xx, yy, smalllake);
        }
    }

    // Now, expand those rivers to their proper widths.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            expandrivers(world, region, x * 16, y * 16, xx, yy, 0, fakesourcex, fakesourcey);
        }
    }

    // Remove weird river elevations.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            removeweirdelevations(world, region, x * 16, y * 16, xx, yy);
        }
    }
}

// This does the regional physical terrain.

void makeregionalterrain(planet& world, region& region, vector<vector<bool>>& disruptpoints, vector<vector<bool>>& riverinlets, vector<vector<bool>>& globalestuaries, vector<vector<int>>& rriverscarved, vector<vector<int>>& fakesourcex, vector<vector<int>>& fakesourcey, boolshapetemplate smalllake[], peaktemplate& peaks, byteshapetemplate smallsmudge[], int xleft, int xright, int ytop, int ybottom, vector<int>& squareroot, vector<vector<int>>& warptox, vector<vector<int>>& warptoy)
{
    int xstart = xleft * 16;
    int xend = xright * 16 + 16;
    int ystart = ytop * 16;
    int yend = ybottom * 16 + 16; // These define the actual area of the regional map that we're creating (with some margin).

    int leftx = region.leftx();
    int lefty = region.lefty(); // These mark the upper left corner of the part of the global map we're expanding on.

    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();
    int maxelev = world.maxelevation();

    int rwidth = region.rwidth();
    int rheight = region.rheight();

    int regwidthbegin = region.regwidthbegin();
    int regwidthend = region.regwidthend();
    int regheightbegin = region.regheightbegin();
    int regheightend = region.regheightend();

    if (leftx<0 || leftx>width)
    {
        leftx = wrap(leftx, width);
        region.setleftx(leftx);
    }

    vector<vector<int>> rotatearray(RARRAYWIDTH, vector<int>(RARRAYHEIGHT, 0));
    vector<vector<int>> rmountainmap(RARRAYWIDTH, vector<int>(RARRAYHEIGHT, 0));
    vector<vector<int>> ridgeids(RARRAYWIDTH, vector<int>(RARRAYHEIGHT, 0));
    vector<vector<int>> nearestridgepointdist(RARRAYWIDTH, vector<int>(RARRAYHEIGHT, 0));
    vector<vector<int>> nearestridgepointx(RARRAYWIDTH, vector<int>(RARRAYHEIGHT, 0));
    vector<vector<int>> nearestridgepointy(RARRAYWIDTH, vector<int>(RARRAYHEIGHT, 0));
    vector<vector<bool>> mountainedges(RARRAYWIDTH, vector<bool>(RARRAYHEIGHT, 0));
    vector<vector<bool>> buttresspoints(RARRAYWIDTH, vector<bool>(RARRAYHEIGHT, 0));
    vector<vector<int>> pathchecked(RARRAYWIDTH, vector<int>(RARRAYHEIGHT, 0));
    vector<vector<int>> rcratermap(RARRAYWIDTH, vector<int>(RARRAYHEIGHT, 0));
    vector<vector<int>> warpedterrain(RARRAYWIDTH, vector<int>(RARRAYHEIGHT, 0));

    vector<vector<int>> source(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

    int coords[4][2];

    // Do basic elevation.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            makeelevationtile(world, region, x * 16, y * 16, xx, yy, coords, smalllake);
        }
    }

    // Now warp that. This only affects coastlines, and it only removes land, doesn't add it.

    warpcoasts(world, region, xstart, ystart, xend, yend, warptox, warptoy);

    // Make coastlines on single-tile islands more interesting.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            if (world.island(xx, yy) == 1)
                complicatecoastlines(world, region, x * 16, y * 16, xx, yy, smalllake, 8);
        }
    }

    // Check for any diagonal sections in the coastlines.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            findcoastdiagonals(world, region, x * 16, y * 16, xx, yy, disruptpoints);
        }
    }

    // Now remove them.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            removecoastdiagonals(world, region, x * 16, y * 16, xx, yy, disruptpoints, smalllake);
        }
    }

    // Ensure that delta regions are above sea level.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            removedeltasea(world, region, x * 16, y * 16, xx, yy);
        }
    }

    // Perform rotations on the edges of the tiles, to break up any grid-like artefacts. (This creates the nicely granular texture to many maps, especially in wetter climates.)

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            rotatetileedges(world, region, x * 16, y * 16, xx, yy, rotatearray, 0);
        }
    }

    // Remove awkward straight lines on the elevation map. (Note: This is the function that adds the pretty but probably unrealistic terrace-like texturing to many maps, especially around mountains and deserts.)

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            addterraces(world, region, x * 16, y * 16, xx, yy, smalllake, smallsmudge);
        }
    }

    // Now ensure that there are no lakes bordering the sea.

    removesealakes(world, region, xstart, ystart, xend, yend);

    // Now, delta branches.

    for (int x = xleft + 1; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop + 1; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            makedeltatile(world, region, x * 16, y * 16, xx, yy, rriverscarved);
        }
    }

    // Now, expand those branches to their proper widths.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            expandrivers(world, region, x * 16, y * 16, xx, yy, 1, fakesourcex, fakesourcey);
        }
    }

    // Now add the delta branches to the normal river map.

    adddeltamap(world, region, xstart, ystart, xend, yend);

    // Now remove any sinks from the land. (Currently turned off because it had strange effects.)

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            //removesinks(world,region,x*16,y*16,xx,yy);
        }
    }

    // Now we make the main mountain ridges.

    int maxridgedist = 10;
    int maxmaxridgedist = maxridgedist * maxridgedist + maxridgedist * maxridgedist;
    int buttressspacing = 14;
    short markgap = 6;

    for (int x = xleft; x <= xright; x++) // Wider
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            makemountaintile(world, region, x * 16, y * 16, xx, yy, peaks, rmountainmap, ridgeids, markgap, warptox, warptoy);
        }
    }

    // Add in shield volcanoes and central crater peaks.

    for (int x = xleft; x <= xright; x++) // Wider
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            if (world.sea(xx, yy) == 0)
            {
                if (world.volcano(xx, yy) != 0 && world.strato(xx, yy) == 0)
                {
                    int templateno = random(1, 2);

                    makevolcano(world, region, x * 16, y * 16, xx, yy, peaks, rmountainmap, ridgeids, templateno);
                }

                if (world.cratercentre(xx, yy) != 0) // Treat the central peaks of craters as if they're volcanoes.
                {
                    int templateno = random(1, 2);

                    makevolcano(world, region, x * 16, y * 16, xx, yy, peaks, rmountainmap, ridgeids, templateno);
                }
            }
        }
    }

    // Now we find cells that are close to those ridges.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            assignridgeregions(world, region, x * 16, y * 16, xx, yy, rmountainmap, ridgeids, nearestridgepointdist, nearestridgepointx, nearestridgepointy, maxridgedist, maxmaxridgedist);
        }
    }

    // Now we find the edges of the mountain ranges.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            findmountainedges(world, region, x * 16, y * 16, xx, yy, nearestridgepointdist, nearestridgepointx, nearestridgepointy, mountainedges);
        }
    }

    // Now we identify the points where buttresses will end.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            int thisbuttressspacing = buttressspacing;

            if (world.sea(xx, yy) == 1)
                thisbuttressspacing = thisbuttressspacing * 2;

            findbuttresspoints(world, region, x * 16, y * 16, xx, yy, ridgeids, nearestridgepointdist, nearestridgepointx, nearestridgepointy, mountainedges, buttresspoints, maxmaxridgedist, thisbuttressspacing);
        }
    }

    // Now we actually do the buttresses.

    for (int i = 0; i <= rwidth; i++)
    {
        for (int j = 0; j <= rheight; j++)
            ridgeids[i][j] = 0;
    }

    markgap = 4; // This is for marking where we want the mini-buttresses to go to.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            makebuttresses(world, region, x * 16, y * 16, xx, yy, rmountainmap, peaks, nearestridgepointx, nearestridgepointy, nearestridgepointdist, maxridgedist, buttresspoints, ridgeids, markgap, 0);
        }
    }

    // Now do all that again for mini-buttresses!

    for (int i = 0; i <= rwidth; i++)
    {
        for (int j = 0; j <= rheight; j++)
        {
            nearestridgepointx[i][j] = 0;
            nearestridgepointy[i][j] = 0;
            nearestridgepointdist[i][j] = 0;
            mountainedges[i][j] = 0;
            buttresspoints[i][j] = 0;
        }
    }

    maxridgedist = 5;
    maxmaxridgedist = maxridgedist * maxridgedist + maxridgedist * maxridgedist;
    buttressspacing = 3;

    // Find cells that are close to those buttresses.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            assignridgeregions(world, region, x * 16, y * 16, xx, yy, rmountainmap, ridgeids, nearestridgepointdist, nearestridgepointx, nearestridgepointy, maxridgedist, maxmaxridgedist);
        }
    }

    // Now we find the edges of the mountain ranges.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            findmountainedges(world, region, x * 16, y * 16, xx, yy, nearestridgepointdist, nearestridgepointx, nearestridgepointy, mountainedges);
        }
    }

    // Now we identify the points where mini-buttresses will end.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            findbuttresspoints(world, region, x * 16, y * 16, xx, yy, ridgeids, nearestridgepointdist, nearestridgepointx, nearestridgepointy, mountainedges, buttresspoints, maxmaxridgedist, buttressspacing);
        }
    }

    // Now we actually do the mini-buttresses.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            if (world.sea(xx, yy) == 0)
                makebuttresses(world, region, x * 16, y * 16, xx, yy, rmountainmap, peaks, nearestridgepointx, nearestridgepointy, nearestridgepointdist, maxridgedist, buttresspoints, ridgeids, markgap, 1);
        }
    }

    // Now put those mountains onto the regional map.

    for (int i = xstart; i <= xend; i++)
    {
        for (int j = ystart; j <= yend; j++)
        {
            if (region.map(i, j) < rmountainmap[i][j] && region.lakesurface(i, j) == 0 && region.riverdir(i, j) == 0)
            {
                region.setmap(i, j, rmountainmap[i][j]);
            }
        }
    }

    // Now do stratovolcanoes.

    for (int i = xstart; i <= xend; i++)
    {
        for (int j = ystart; j <= yend; j++)
            rmountainmap[i][j] = 0;
    }

    for (int x = xleft; x <= xright; x++) // Wider
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            if (world.sea(xx, yy) == 0 && world.strato(xx, yy) == 1)
            {
                int templateno = 1;

                makevolcano(world, region, x * 16, y * 16, xx, yy, peaks, rmountainmap, ridgeids, templateno);
            }
        }
    }

    // Now put those onto the regional map.

    for (int i = xstart; i <= xend; i++)
    {
        for (int j = ystart; j <= yend; j++)
        {
            if (region.map(i, j) < rmountainmap[i][j] && region.lakesurface(i, j) == 0 && region.riverdir(i, j) == 0)
            {
                region.setmap(i, j, rmountainmap[i][j]);
            }
        }
    }

    // Now we do crater rims.

    int regioncentrex = region.centrex();
    int regioncentrey = region.centrey();
    
    for (int n = 0; n < world.craterno(); n++)
    {
        int xx = world.craterx(n);
        int yy = world.cratery(n);

        int radius = (int)((float)world.craterradius(n) * 1.5f);

        if (yy + radius > regioncentrey - 16 && yy - radius < regioncentrey + 16) // If this crater is close enough on the Y axis to perhaps show on the regional map
        {
            // Now work out whether it's close enough on the X axis too, which is more complicated because of wrapping.

            bool goahead = 0;

            if (xx + radius > regioncentrex - 16 && xx - radius < regioncentrex + 16)
                goahead = 1;
            else
            {
                if (xx + radius - width > regioncentrex - 16 && xx - radius < regioncentrex + 16)
                    goahead = 1;
                else
                    if (xx + radius > regioncentrex - 16 && xx - radius + width < regioncentrex + 16)
                        goahead = 1;
            }

            if (goahead)
            {
                int x = xx - leftx;
                int y = yy - lefty;

                makeregionalcraterrim(world, region, x * 16, y * 16, xx, yy, xstart, ystart, xend, yend, n, rcratermap, peaks, squareroot);
            }
        }
    }

    // Now put those onto the regional map.

    for (int i = xstart; i <= xend; i++)
    {
        for (int j = ystart; j <= yend; j++)
        {
            if (region.map(i, j) < rcratermap[i][j] && region.sea(i, j) == 0 && region.lakesurface(i, j) == 0 && region.riverdir(i, j) == 0 && region.rivervalley(i, j) == 0)
            {
                region.setmap(i, j, rcratermap[i][j]);
            }
        }
    }

    // Now, remove extra land around mountain islands.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            if (world.mountainisland(xx, yy) == 1)
                trimmountainislands(world, region, x * 16, y * 16, xx, yy, rmountainmap, riverinlets, globalestuaries, smalllake);
        }
    }

    // Now we widen any diagonal waterways.

    removediagonalwater(region, xstart, ystart, xend, yend, sealevel);

    /*
     // Now remove pools. (Turned off for now as it creates weird artefacts, and the turnpoolstolakes routine pretty much catches them all anyway.)

     regionprogress.set_value(regionprogress.value()+progressstep);
     screen.redraw();
     screen.draw_all();

     int checkno=0;

     for (int x=xleft+1; x<xright; x++) // Note that we don't do the ones on the edges of the regional map.
     {
     int xx=leftx+x;

     if (xx<0 || xx>width)
     xx=wrap(xx,width);

     for (int y=ytop+1; y<ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
     {
     int yy=lefty+y;

     removepools(world,region,x*16,y*16,xx,yy,pathchecked,checkno);
     }
     }
     */

     // Remove straight edges on the coastlines again.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            removestraights(world, region, x * 16, y * 16, xx, yy, smalllake);
        }
    }

    // Now turn any pools that may be left into lakes.

    int checkno = 0;

    vector<vector<bool>> regionsea(RARRAYWIDTH, vector<bool>(RARRAYHEIGHT, 0)); // This will store info about which cells are sea.

    for (int i = 0; i < RARRAYWIDTH; i++)
    {
        for (int j = 0; j < RARRAYHEIGHT; j++)
        {
            if (region.sea(i, j))
                regionsea[i][j] = 1;
        }
    }

    //highres_timer_t timer("turnpoolstolakes");
    for (int x = xleft + 1; x < xright; x++) // Note that we don't do the ones on the edges of the regional map.
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop + 1; y < ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            turnpoolstolakes(world, region, x * 16, y * 16, xx, yy, regionsea, pathchecked, checkno);
        }
    }
    //timer.end();

    // Remove bits of lakes that touch the sea. (Turned off as it sometimes had the unfortunate effect of turning the entire sea into lake.)

    // //removelakesbysea(region,rstart,rstart,rwidth,rheight,sealevel);

    // Now, remove weird bits of rivers from inside seas.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            removeextrasearivers(world, region, x * 16, y * 16, xx, yy, regionsea);
        }
    }

    // Now remove rivers that emerge from the sea onto land.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            removeriverscomingfromsea(world, region, x * 16, y * 16, xx, yy, fakesourcex, fakesourcey, regionsea);
        }
    }

    // Now, add inlets to rivers.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            addinlets(world, region, x * 16, y * 16, xx, yy, riverinlets, globalestuaries);
        }
    }

    // Now, remove extraneous islands from the sea.

    for (int x = xleft; x <= xright; x = x + 2)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y = y + 2) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            removesmallislands(world, region, x * 16, y * 16, xx, yy);
        }
    }
}

// This does the regional undersea terrain.

void makeregionalunderseaterrain(planet& world, region& region, peaktemplate& peaks, byteshapetemplate smudge[], byteshapetemplate smallsmudge[], int xleft, int xright, int ytop, int ybottom, vector<vector<int>>& warptox, vector<vector<int>>& warptoy)
{
    int xstart = xleft * 16;
    int xend = xright * 16 + 16;
    int ystart = ytop * 16;
    int yend = ybottom * 16 + 16; // These define the actual area of the regional map that we're creating (with some margin).

    int leftx = region.leftx();
    int lefty = region.lefty(); // These mark the upper left corner of the part of the global map we're expanding on.

    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();
    int maxelev = world.maxelevation();

    int rwidth = region.rwidth();
    int rheight = region.rheight();

    float warpfactor = 100.0f;

    if (leftx<0 || leftx>width)
    {
        leftx = wrap(leftx, width);
        region.setleftx(leftx);
    }

    vector<vector<int>> underseamap(RARRAYWIDTH * 4, vector<int>(RARRAYHEIGHT * 4, 0));
    vector<vector<bool>> undersearidgelines(RARRAYWIDTH, vector<bool>(RARRAYHEIGHT, 0));
    vector<vector<bool>> volcanomap(RARRAYWIDTH, vector<bool>(RARRAYHEIGHT, 0));
    vector<vector<int>> undersearidges(RARRAYWIDTH, vector<int>(RARRAYHEIGHT, 0));
    vector<vector<int>> underseaspikes(RARRAYWIDTH * 4, vector<int>(RARRAYHEIGHT * 4, 0));
    vector<vector<int>> warpedterrain(RARRAYWIDTH, vector<int>(RARRAYHEIGHT, 0));
    vector<vector<bool>> warpedvolcanoes(RARRAYWIDTH, vector<bool>(RARRAYHEIGHT, 0));
    vector<vector<int>> source(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

    int coords[4][2];

    // Do submarine elevation.

    int extra = 20; // For these again, we will create a *bigger* map than the actual regional area. This is because the templates that we're going to use to disrupt this terrain are quite big, so we need to have a big margin around the visible area to ensure that every tile looks the same no matter where the regional map is centred.

    for (int x = xleft - extra; x <= xright + extra; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop - extra; y <= ybottom + extra; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            makesubmarineelevationtile(world, region, x * 16, y * 16, xx, yy, underseamap, coords, extra);
        }
    }

    // Now disrupt that elevation a bit.

    // The sweep over the whole region needs to be staggered to keep the disruption evenly distributed.

    for (int x = xleft - extra; x <= xright + extra; x = x + 2)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop - extra; y <= ybottom + extra; y = y + 2) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            if (world.sea(xx, yy) == 1)
                disruptsubmarineelevationtile(world, region, x * 16, y * 16, xx, yy, underseamap, smudge, extra);
        }
    }

    for (int x = xright + extra - 1; x >= xleft - extra; x = x - 2)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ybottom + extra; y >= ytop - extra; y = y - 2) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            if (world.sea(xx, yy) == 1)
                disruptsubmarineelevationtile(world, region, x * 16, y * 16, xx, yy, underseamap, smudge, extra);
        }
    }

    for (int x = xleft - extra; x <= xright + extra; x = x + 2)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop - extra + 1; y <= ybottom + extra; y = y + 2) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            if (world.sea(xx, yy) == 1)
                disruptsubmarineelevationtile(world, region, x * 16, y * 16, xx, yy, underseamap, smudge, extra);
        }
    }

    for (int x = xright + extra - 1; x >= xleft - extra; x = x - 2)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ybottom + extra - 1; y >= ytop - extra; y = y - 2) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            if (world.sea(xx, yy) == 1)
                disruptsubmarineelevationtile(world, region, x * 16, y * 16, xx, yy, underseamap, smudge, extra);
        }
    }

    // Now we find the paths of the submarine ridges.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            if (world.oceanridges(xx, yy) != 0)
                makesubmarineridgelines(world, region, x * 16, y * 16, xx, yy, undersearidgelines, warptox, warptoy);
        }
    }

    // Now we draw the ridges onto the ridge map.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            if (world.sea(xx, yy) == 1)
                drawsubmarineridges(world, region, x * 16, y * 16, xx, yy, undersearidgelines, peaks, undersearidges);
        }
    }

    // Now we add the central mountains to the ridges.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            if (world.oceanrifts(xx, yy) != 0)
                makesubmarineriftmountains(world, region, x * 16, y * 16, xx, yy, undersearidges, peaks, warptox, warptoy);
        }
    }

    // Now we carve out the central rift valley.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            if (world.oceanrifts(xx, yy) != 0)
                makesubmarinerift(world, region, x * 16, y * 16, xx, yy, undersearidges, smallsmudge, warptox, warptoy);
        }
    }

    // Now we add the spikes radiating away from the rifts.

    extra = 20; // This one has to be done with a wide margin around the visible map, as there could be spikes coming in from outside.

    for (int x = xleft - extra; x <= xright + extra; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop - extra; y <= ybottom + extra; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            if (world.oceanrifts(xx, yy) != 0)
                makesubmarineriftradiations(world, region, x * 16, y * 16, xx, yy, underseaspikes, peaks, extra);
        }
    }

    // Apply the spikes map to the ridges map.

    int dextra = extra * 16;

    for (int i = 0; i <= rwidth; i++)
    {
        for (int j = 0; j <= rheight; j++)
        {
            if (region.sea(i, j))
            {
                if (underseaspikes[i + dextra][j + dextra] < 0)
                {
                    undersearidges[i][j] = undersearidges[i][j] + underseaspikes[i + dextra][j + dextra];

                    if (undersearidges[i][j] < 0)
                        undersearidges[i][j] = 0;
                }
                else
                {
                    if (undersearidges[i][j] < underseaspikes[i + dextra][j + dextra])
                        undersearidges[i][j] = underseaspikes[i + dextra][j + dextra];
                }
            }
        }
    }

    // Now, add submarine volcanoes (seamounts).

    for (int x = xleft - extra; x <= xright + extra; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop - extra; y <= ybottom + extra; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            if (world.sea(xx, yy) == 1 && world.volcano(xx, yy) != 0)
                makesubmarinevolcano(world, region, x * 16, y * 16, xx, yy, peaks, undersearidges, volcanomap);
        }
    }

    // Now warp the ridges. This is to make them look a little more natural.

    // Apply the ridges to the undersea terrain.

    for (int i = 0; i <= rwidth; i++)
    {
        for (int j = 0; j <= rheight; j++)
            underseamap[i + extra * 16][j + extra * 16] = underseamap[i + extra * 16][j + extra * 16] + undersearidges[i][j];
    }

    // Check that all of this only rarely breaks the surface.

    for (int i = 0; i <= rwidth; i++)
    {
        int ii = i + extra * 16;

        for (int j = 0; j <= rheight; j++)
        {
            int jj = j + extra * 16;

            if (underseamap[ii][jj] > sealevel)
            {
                if (random(1, 100) != 1)
                {
                    underseamap[ii][jj] = sealevel - random(50, 100);

                    if (underseamap[ii][jj] < 1)
                        underseamap[ii][jj] = 1;
                }
            }
        }
    }

    // Put the undersea terrain onto the actual map.

    for (int i = 0; i <= rwidth; i++)
    {
        for (int j = 0; j <= rheight; j++)
        {
            if (region.sea(i, j))
            {
                region.setmap(i, j, underseamap[i + extra * 16][j + extra * 16]);

                if (warpedvolcanoes[i][j] == 1)
                    region.setvolcano(i, j, 1);
            }
        }
    }
}

// This does some miscellaneous stuff to the regional map.

void makeregionalmiscellanies(planet& world, region& region, vector<vector<bool>>& safesaltlakes, vector<vector<bool>>& riverinlets, vector<vector<bool>>& globalestuaries, boolshapetemplate smalllake[], byteshapetemplate smallsmudge[], int xleft, int xright, int ytop, int ybottom)
{
    int xstart = xleft * 16;
    int xend = xright * 16 + 16;
    int ystart = ytop * 16;
    int yend = ybottom * 16 + 16; // These define the actual area of the regional map that we're creating (with some margin).

    int leftx = region.leftx();
    int lefty = region.lefty(); // These mark the upper left corner of the part of the global map we're expanding on.

    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();
    int maxelev = world.maxelevation();

    int rwidth = region.rwidth();
    int rheight = region.rheight();

    if (leftx<0 || leftx>width)
    {
        leftx = wrap(leftx, width);
        region.setleftx(leftx);
    }

    vector<vector<int>> source(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));
    vector<vector<int>> destination(RARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));
    vector<vector<int>> rotatearray(RARRAYWIDTH, vector<int>(RARRAYHEIGHT, 0));
    vector<vector<float>> siltstrength(RARRAYWIDTH, vector<float>(RARRAYHEIGHT, 0.0f));
    vector<vector<float>> sandstrength(RARRAYWIDTH, vector<float>(RARRAYHEIGHT, 0.0f));
    vector<vector<float>> shinglestrength(RARRAYWIDTH, vector<float>(RARRAYHEIGHT, 0.0f));
    vector<vector<bool>> checked(RARRAYWIDTH, vector<bool>(RARRAYHEIGHT, 0.0f));

     for (int i = xstart; i <= xend; i++)
    {
        for (int j = ystart; j <= yend; j++)
            region.settide(i, j, destination[i][j]);
    }

    // Make wetlands.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            makewetlandtile(world, region, x * 16, y * 16, xx, yy, smalllake);
        }
    }

    // Make sure special "lakes" are correctly identified. (Turned off for now as it had problems.)
    /*
    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            convertlakestospecials(world, region, x * 16, y * 16, xx, yy, safesaltlakes);
        }
    }
    */

    // Check for weird lake surfaces.

    checklakesurfaces(world, region);

    // Now salt pans around salt lakes.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            makesaltpantile(world, region, x * 16, y * 16, xx, yy, smalllake);
        }
    }

    // Now we do barrier islands.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            addbarrierislands(world, region, x * 16, y * 16, xx, yy, riverinlets);
        }
    }

    // Now, remove odd little parallel lines of islands that sometimes get created by the barrier islands routine.

    removeparallelislands(region, xstart, ystart, xend, yend, sealevel);

    // Check for any impossibly low areas of elevation.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            removetoolow(world, region, x * 16, y * 16, xx, yy);
        }
    }

    // Now remove any bits of sea that are next to lakes.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            removelakeseas(world, region, x * 16, y * 16, xx, yy);
        }
    }

    // Now smooth lake beds.

    smoothlakebeds(region);

    // Now do rotations on the lake beds to make the terrain look more natural.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            rotatetileedges(world, region, x * 16, y * 16, xx, yy, rotatearray, 1);
        }
    }

    // Now remove any weirdly high elevation.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            removetoohighelevations(world, region, x * 16, y * 16, xx, yy);
        }
    }

    // Now disrupt lake beds again.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            addlaketerraces(world, region, x * 16, y * 16, xx, yy, smalllake, smallsmudge);
        }
    }

    // Now we do glaciers.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            addregionalglaciers(world, region, x * 16, y * 16, xx, yy);
        }
    }

    // Now prepare to make mud flats.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            bool goahead = 0;

            if (world.deltadir(xx, yy) != 0)
                goahead = 1;

            if (yy >= 0 && yy <= height && globalestuaries[xx][yy])
                goahead = 1;

            if (goahead)
                makemudflatseeds(world, region, x * 16, y * 16, xx, yy, riverinlets, siltstrength);
        }
    }

    // And make the mud flats.

    spreadsilt(world, region, xstart, ystart, xend, yend, siltstrength, smalllake);

    for (int x = xleft+4; x <= xright - 2; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        int xxx = xx + width / 10;

        if (xxx > width)
            xxx = xxx - width;

        for (int y = ytop+4; y <= ybottom - 2; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            if (world.noise(xxx, yy) < maxelev / 3)
                makemudbarriers(world, region, x * 16, y * 16, xx, yy);
        }
    }

    // Now prepare to make sandy/shingle beaches.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            makesandseeds(world, region, x * 16, y * 16, xx, yy, riverinlets, sandstrength, shinglestrength);
        }
    }

    // And make the beaches.

    float reduce = (128.0f - world.lunar() * 8.0f) * 2.0f; // * 1.0f;
    float diagreduce = reduce * 1.41421f;

    float landreduce = reduce * 0.3f; // 0.6f
    float landdiagreduce = landreduce * 1.41421f;

    float coastreduce = reduce * 0.05f; // 0.1f
    float coastdiagreduce = landreduce * 1.41421f;

    int crount = 1;

    for (int n = 0; n <= 100000; n++)
    {
        bool doneone = 0;

        for (int x = xleft; x <= xright; x++)
        {
            int xx = leftx + x;

            if (xx<0 || xx>width)
                xx = wrap(xx, width);

            for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
            {
                int yy = lefty + y;

                bool donethis = spreadsand(world, region, x * 16, y * 16, xx, yy, reduce, diagreduce, landreduce, landdiagreduce, coastreduce, coastdiagreduce, crount, 0, checked, sandstrength);

                if (donethis)
                    doneone = 1;
            }
        }

        if (doneone == 0)
            n = 100000;
        else
        {
            crount++;

            if (crount > 4)
                crount = 1;
        }
    }

    for (int i = 0; i < RARRAYWIDTH; i++)
    {
        for (int j = 0; j < RARRAYHEIGHT; j++)
            checked[i][j] = 0;
    }

    crount = 1;

    for (int n = 0; n <= 100000; n++)
    {
        bool doneone = 0;

        for (int x = xleft; x <= xright; x++)
        {
            int xx = leftx + x;

            if (xx<0 || xx>width)
                xx = wrap(xx, width);

            for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
            {
                int yy = lefty + y;

                bool donethis = spreadsand(world, region, x * 16, y * 16, xx, yy, reduce, diagreduce, landreduce, landdiagreduce, coastreduce, coastdiagreduce, crount, 1, checked, shinglestrength);

                if (donethis)
                    doneone = 1;
            }
        }

        if (doneone == 0)
            n = 100000;
        else
        {
            crount++;

            if (crount > 4)
                crount = 1;
        }
    }

    createbeaches(world, region, xstart, ystart, xend, yend,0,sandstrength);
    createbeaches(world, region, xstart, ystart, xend, yend, 1, shinglestrength);

    // Spread mud onto sand where needed.

    putmudonsand(world, region, xstart, ystart, xend, yend);

    // Make sure the sea isn't too deep next to beaches.

    checkbeachcoasts(world, region, xstart, ystart, xend, yend);
}

// This does the regional climates.

void makeregionalclimates(planet& world, region& region, vector<vector<bool>>& safesaltlakes, boolshapetemplate smalllake[], int xleft, int xright, int ytop, int ybottom)
{
    int xstart = xleft * 16;
    int xend = xright * 16 + 16;
    int ystart = ytop * 16;
    int yend = ybottom * 16 + 16; // These define the actual area of the regional map that we're creating (with some margin).

    int leftx = region.leftx();
    int lefty = region.lefty(); // These mark the upper left corner of the part of the global map we're expanding on.

    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();
    int maxelev = world.maxelevation();

    int rwidth = region.rwidth();
    int rheight = region.rheight();

    if (leftx<0 || leftx>width)
    {
        leftx = wrap(leftx, width);
        region.setleftx(leftx);
    }

    vector<vector<int>> source(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));
    vector<vector<int>> destination(RARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));
    vector<vector<int>> rotatearray(RARRAYWIDTH, vector<int>(RARRAYHEIGHT, 0));

    int coords[4][2];

    // First we do the precipitation maps.

    // First, jan precipitation.

    for (int i = 0; i < RARRAYWIDTH; i++)
    {
        for (int j = 0; j < RARRAYHEIGHT; j++)
        {
            destination[i][j] = -5000;
            rotatearray[i][j] = 0;
        }
    }

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (world.janmountainraindir(i, j) == 0)
                source[i][j] = world.janrain(i, j);
            else
                source[i][j] = world.janmountainrain(i, j);
        }
    }

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            float valuemod = 0.04f; //0.02;

            makegenerictile(world, x * 16, y * 16, xx, yy, valuemod, coords, source, destination, 100000, 0, 1);

            if (yy == height) // If this is the very bottom of the global map, ensure there's no weirdness.
            {
                int maxval = world.janrain(xx, yy);

                for (int i = x * 16; i <= x * 16 + 16; i++)
                {
                    for (int j = y * 16; j <= y * 16 + 16; j++)
                    {
                        if (destination[i][j] > maxval)
                            destination[i][j] = maxval;
                    }
                }
            }
        }
    }

    // Now add rotations to that precipitation map, to improve the look.

    for (int x = xleft + 2; x < xright - 1; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop + 2; y < ybottom - 1; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            rotatetileedgesarray(world, region, x * 16, y * 16, xx, yy, destination, rotatearray, -5000);
        }
    }

    for (int i = xstart; i <= xend; i++)
    {
        for (int j = ystart; j <= yend; j++)
            region.setjanrain(i, j, destination[i][j]);
    }

    // Now, jul precipitation.

    for (int i = 0; i < RARRAYWIDTH; i++)
    {
        for (int j = 0; j < RARRAYHEIGHT; j++)
        {
            destination[i][j] = -5000;
            rotatearray[i][j] = 0;
        }
    }

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (world.julmountainraindir(i, j) == 0)
                source[i][j] = world.julrain(i, j);
            else
                source[i][j] = world.julmountainrain(i, j);
        }
    }

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            float valuemod = 0.04f; //0.02;

            makegenerictile(world, x * 16, y * 16, xx, yy, valuemod, coords, source, destination, 100000, 0, 1);

            if (yy == height) // If this is the very bottom of the global map, ensure there's no weirdness.
            {
                int maxval = world.julrain(xx, yy);

                for (int i = x * 16; i <= x * 16 + 16; i++)
                {
                    for (int j = y * 16; j <= y * 16 + 16; j++)
                    {
                        if (destination[i][j] > maxval)
                            destination[i][j] = maxval;
                    }
                }
            }
        }
    }

    // Now add rotations to that precipitation map, to improve the look.

    for (int x = xleft + 2; x < xright - 1; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop + 2; y < ybottom - 1; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            rotatetileedgesarray(world, region, x * 16, y * 16, xx, yy, destination, rotatearray, -5000);
        }
    }

    for (int i = xstart; i <= xend; i++)
    {
        for (int j = ystart; j <= yend; j++)
            region.setjulrain(i, j, destination[i][j]);
    }

    smoothprecipitation(region, xstart, ystart, xend, yend, 2);

    // Now add extra precipitation to mountains, where appropriate.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            if (world.wintermountainraindir(xx, yy) != 0)
                addregionalmountainprecipitation(world, region, x * 16, y * 16, xx, yy, 0);

            if (world.summermountainraindir(xx, yy) != 0)
                addregionalmountainprecipitation(world, region, x * 16, y * 16, xx, yy, 1);
        }
    }

    // Now remove any salt pans from areas that have too much precipitation.

    removewetsaltpans(region, xstart, ystart, xend, yend);

    // Now we do the temperature maps.

    // First, jul temperature.

    for (int i = 0; i < RARRAYWIDTH; i++)
    {
        for (int j = 0; j < RARRAYHEIGHT; j++)
        {
            destination[i][j] = -5000;
            rotatearray[i][j] = 0;
        }
    }

    // Note that we remove the elevation element of temperatures (because that is determined by the elevation of the regional map, not of the global map). Also we multiply temperatures by 100 temporarily, to ensure smoother fractals.

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
            source[i][j] = tempelevremove(world, world.jultemp(i, j), i, j) * 100;
    }

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            float valuemod = 0.01f;

            makegenerictile(world, x * 16, y * 16, xx, yy, valuemod, coords, source, destination, 5000, -5000, 0);
        }
    }

    // Now add rotations to that temperature map, to improve the look.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            rotatetileedgesarray(world, region, x * 16, y * 16, xx, yy, destination, rotatearray, -5000);
        }
    }

    // Remove any anomalous values.

    checkanomalies(world, region, destination, xstart, ystart, xend, yend);

    // Now add the elevation element back in.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            for (int i = x * 16; i <= x * 16 + 15; i++) // Divide by 100 and add the elevation element back in.
            {
                for (int j = y * 16; j <= y * 16 + 15; j++)
                {
                    int amount = tempelevadd(world, region, destination[i][j], i, j) - destination[i][j];

                    region.setextrajultemp(i, j, destination[i][j] + amount * 100);

                    region.setjultemp(i, j, tempelevadd(world, region, destination[i][j] / 100, i, j));
                }
            }
        }
    }

    // Now, jan temperature.

    for (int i = 0; i < RARRAYWIDTH; i++)
    {
        for (int j = 0; j < RARRAYHEIGHT; j++)
        {
            destination[i][j] = -5000;
            rotatearray[i][j] = 0;
        }
    }

    // Note that we remove the elevation element of temperatures (because that is determined by the elevation of the regional map, not of the global map). Also we multiply temperatures by 100 temporarily, to ensure smoother fractals.

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
            source[i][j] = tempelevremove(world, world.jantemp(i, j), i, j) * 100;
    }

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            float valuemod = 0.01f;

            makegenerictile(world, x * 16, y * 16, xx, yy, valuemod, coords, source, destination, 5000, -5000, 0);

            if (yy == height) // If this is the very bottom of the global map, ensure there's no weirdness.
            {
                int maxval = world.jantemp(xx, yy);

                for (int i = x * 16; i <= x * 16 + 16; i++)
                {
                    for (int j = y * 16; j <= y * 16 + 16; j++)
                    {
                        if (destination[i][j] > maxval)
                            destination[i][j] = maxval;
                    }
                }
            }
        }
    }

    // Now add rotations to that temperature map, to improve the look.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            rotatetileedgesarray(world, region, x * 16, y * 16, xx, yy, destination, rotatearray, -5000);
        }
    }

    // Remove any anomalous values.

    checkanomalies(world, region, destination, xstart, ystart, xend, yend);

    // Now add the elevation element back in.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            for (int i = x * 16; i <= x * 16 + 15; i++) // Divide by 100 and add the elevation element back in.
            {
                for (int j = y * 16; j <= y * 16 + 15; j++)
                {
                    int amount = tempelevadd(world, region, destination[i][j], i, j) - destination[i][j];

                    region.setextrajantemp(i, j, destination[i][j] + amount * 100);

                    region.setjantemp(i, j, tempelevadd(world, region, destination[i][j] / 100, i, j));
                }
            }
        }
    }

    // Now sort out any problems with the temperatures at the very bottom of the map.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ybottom - 2; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            if (yy > height - 10)
            {
                int maxmaxtemp = world.maxtemp(xx, yy);
                int maxmintemp = world.mintemp(xx, yy);

                for (int i = x * 16; i <= x * 16 + 15; i++)
                {
                    for (int j = y * 16; j <= y * 16 + 15; j++)
                    {
                        if (region.maxtemp(i, j) > maxmaxtemp)
                            region.setmaxtemp(i, j, maxmaxtemp);

                        if (region.mintemp(i, j) > maxmintemp)
                            region.setmintemp(i, j, maxmintemp);
                    }
                }
            }
        }
    }

    // Now the sea ice map.
    // Here again we have to fiddle with the values, as it only has three values and that's not much use for the diamond-square function to get to work on.

    int icediv = 20;

    for (int i = 0; i < RARRAYWIDTH; i++)
    {
        for (int j = 0; j < RARRAYHEIGHT; j++)
            destination[i][j] = -5000;
    }

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            switch (world.seaice(i, j))
            {
            case 0:
                source[i][j] = icediv / 2;
                break;

            case 1:
                source[i][j] = icediv / 2 + icediv;
                break;

            case 2:
                source[i][j] = icediv / 2 + icediv * 2;
                break;
            }
        }
    }

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            int surroundingice = getsurroundingice(world, xx, yy);

            if (surroundingice != -1) // If this isn't a mixed tile
            {
                for (int i = x * 16; i <= x * 16 + 15; i++)
                {
                    for (int j = y * 16; j <= y * 16 + 15; j++)
                        region.setseaice(i, j, world.seaice(xx, yy));
                }
            }
            else
            {
                float valuemod = 0.015f;

                makegenerictile(world, x * 16, y * 16, xx, yy, valuemod, coords, source, destination, icediv * 3, 0, 0);

                for (int i = x * 16; i <= x * 16 + 15; i++) // Translate the values back to sea ice values.
                {
                    for (int j = y * 16; j <= y * 16 + 15; j++)
                    {
                        if (destination[i][j] <= icediv)
                        {
                            bool openseaok = 1;

                            for (int k = xx - 1; k <= xx + 1; k++)
                            {
                                int kk = k;

                                if (kk<0 || kk>width)
                                    kk = wrap(kk, width);

                                for (int l = yy - 1; l <= yy + 1; l++)
                                {
                                    if (l >= 0 && l <= height)
                                    {
                                        if (world.seaice(kk, l) == 2)
                                        {
                                            openseaok = 0;
                                            k = xx + 1;
                                            l = yy + 1;
                                        }
                                    }
                                }
                            }

                            if (openseaok == 1)
                                region.setseaice(i, j, 0);
                            else
                                region.setseaice(i, j, 1);

                        }
                        else
                        {
                            if (destination[i][j] > icediv && destination[i][j] <= icediv * 2)
                                region.setseaice(i, j, 1);
                            else
                            {
                                bool permiceok = 1;

                                for (int k = xx - 1; k <= xx + 1; k++)
                                {
                                    int kk = k;

                                    if (kk<0 || kk>width)
                                        kk = wrap(kk, width);

                                    for (int l = yy - 1; l <= yy + 1; l++)
                                    {
                                        if (l >= 0 && l <= height)
                                        {
                                            if (world.seaice(kk, l) == 0)
                                            {
                                                permiceok = 0;
                                                k = xx + 1;
                                                l = yy + 1;
                                            }
                                        }
                                    }
                                }

                                if (permiceok == 1)
                                    region.setseaice(i, j, 2);
                                else
                                    region.setseaice(i, j, 1);

                            }
                        }
                    }
                }
            }
        }
    }

    // Now remove glitches in the sea ice map.

    removeseaiceglitches(region);

    // Now we do the climate map.

    for (int i = xstart; i <= xend; i++)
    {
        for (int j = ystart; j <= yend; j++)
        {
            short climate = getclimate(region, i, j);
            region.setclimate(i, j, climate);
        }
    }

    // Now create small salt pans.

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            makesmallsaltpans(world, region, x * 16, y * 16, xx, yy, safesaltlakes, smalllake);
        }
    }

    // Now just check that all the lake beds make sense.

    checklakebeds(region, xstart, ystart, xend, yend);
}

// This returns the global coordinate of the tile that a regional coordinate is in.

twointegers convertregionaltoglobal(planet& world, region& region, int x, int y)
{
    twointegers global;

    int width = world.width();

    x = x / 16;
    y = y / 16;

    x = x + region.leftx();
    y = y + region.lefty();

    if (x<0 || x>width)
        x = wrap(x, width);

    global.x = x;
    global.y = y;

    return global;
}

// This removes any lake cells that border sea cells.

void removesealakes(planet& world, region& region, int leftx, int lefty, int rightx, int righty)
{
    int sealevel = world.sealevel();

    for (int i = leftx + 1; i <= rightx - 1; i++)
    {
        for (int j = lefty + 1; j <= righty - 1; j++)
        {
            if (region.truelake(i, j) != 0)
            {
                for (int k = i - 1; k <= i + 1; k++)
                {
                    for (int l = j - 1; l <= j + 1; l++)
                    {
                        if (region.sea(k, l) == 1)
                        {
                            region.setmap(i, j, sealevel + random(1, 5));
                            region.setlakesurface(i, j, 0);

                            k = i + 1;
                            l = j + 1;
                        }
                    }
                }
            }

        }
    }
}

// This adds rivers to tiles on the regional map.

void makerivertile(planet& world, region& region, int dx, int dy, int sx, int sy, vector<vector<int>>& rriverscarved, boolshapetemplate smalllake[], vector<vector<bool>>& rivercurves)
{
    int riverdir = world.riverdir(sx, sy);

    if (riverdir == 0)
        return;

    int riverjan = world.riverjan(sx, sy);
    int riverjul = world.riverjul(sx, sy);

    if (riverjan == 0 && riverjul == 0)
        return;

    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();

    int rwidth = region.rwidth();
    int rheight = region.rheight();

    int riverlandreduce = world.riverlandreduce();

    for (int i = 0; i <= rwidth; i++)
    {
        for (int j = 0; j <= rheight; j++)
            rriverscarved[i][j] = -1;
    }

    vector<vector<int>> mainriver(17, vector<int>(17, 0));

    // First, get coordinates for the tiles around this one.

    int eax = sx + 1;
    int eay = sy;
    if (eax > width)
        eax = 0;

    int wex = sx - 1;
    int wey = sy;
    if (wex < 0)
        wex = width;

    int sox = sx;
    int soy = sy + 1;

    int nox = sx;
    int noy = sy - 1;

    int sex = eax;
    int sey = soy;

    int nwx = wex;
    int nwy = noy;

    int nex = eax;
    int ney = noy;

    int swx = wex;
    int swy = soy;

    bool lakepresent = 0;
    int surfacelevel = 0;

    if (world.truelake(sx, sy) != 0)
    {
        int found = 0;

        if (world.truelake(nox, noy) != 0)
            found++;

        if (world.truelake(nex, ney) != 0)
            found++;

        if (world.truelake(eax, eay) != 0)
            found++;

        if (world.truelake(sex, sey) != 0)
            found++;

        if (world.truelake(sox, soy) != 0)
            found++;

        if (world.truelake(swx, swy) != 0)
            found++;

        if (world.truelake(wex, wey) != 0)
            found++;

        if (world.truelake(nwx, nwy) != 0)
            found++;

        if (found == 8) // Don't bother doing rivers if we're in the middle of a lake.
            return;

        lakepresent = 1;
        surfacelevel = world.lakesurface(sx, sy);
    }

    int aveflow = (riverjan + riverjul) / 2;

    if (aveflow == 0 && world.deltadir(sx, sy) < 1)
        return;

    twofloats pt, mm1, mm2, mm3, mm4;
    twointegers riverpoint;

    threeintegers rinfo;

    int margin = 2; // This is to ensure that the midpoints aren't too close to the edges.
    int mmargin = 16 - margin;
    bool goingtolake = 0;

    int minmidvar = 2; // Minimum amount the midpoints between the junction and the sides can be offset by.
    int maxmidvar = 3; // Maximum amount the midpoints between the junction and the sides can be offset by.
    int minpoint = 3; // Minimum distance the points on the sides (where rivers go off the tile) can be from the corners.

    int maxmidvardiag = 4; // Maximum amount the midpoints can be offset by on a diagonal tile.
    int minmidvardiag = 2; // Minimum amount the midpoints can be offset by on a diagonal tile.

    int north = 0;
    int south = 0;
    int east = 0;
    int west = 0;
    int northeast = 0;
    int northwest = 0;
    int southeast = 0;
    int southwest = 0; // These will hold the average flow of rivers coming from these directions.

    // Now check to see which directions have incoming rivers.

    if (world.riverdir(nox, noy) == 5) // North
        north = (world.riverjan(nox, noy) + world.riverjul(nox, noy)) / 2;

    if (world.riverdir(nex, ney) == 6) // Northeast
        northeast = (world.riverjan(nex, ney) + world.riverjul(nex, ney)) / 2;

    if (world.riverdir(eax, eay) == 7) // East
        east = (world.riverjan(eax, eay) + world.riverjul(eax, eay)) / 2;

    if (world.riverdir(sex, sey) == 8) // Southeast
        southeast = (world.riverjan(sex, sey) + world.riverjul(sex, sey)) / 2;

    if (world.riverdir(sox, soy) == 1) // South
        south = (world.riverjan(sox, soy) + world.riverjul(sox, soy)) / 2;

    if (world.riverdir(swx, swy) == 2) // Southwest
        southwest = (world.riverjan(swx, swy) + world.riverjul(swx, swy)) / 2;

    if (world.riverdir(wex, wey) == 3) // West
        west = (world.riverjan(wex, wey) + world.riverjul(wex, wey)) / 2;

    if (world.riverdir(nwx, nwy) == 4) // Northwest
        northwest = (world.riverjan(nwx, nwy) + world.riverjul(nwx, nwy)) / 2;

    // Now work out which of these is the largest.

    int biggestflowin = north;
    int maininflow = 1;

    if (northeast > biggestflowin)
    {
        biggestflowin = northeast;
        maininflow = 2;
    }

    if (east > biggestflowin)
    {
        biggestflowin = east;
        maininflow = 3;
    }

    if (southeast > biggestflowin)
    {
        biggestflowin = southeast;
        maininflow = 4;
    }

    if (south > biggestflowin)
    {
        biggestflowin = south;
        maininflow = 5;
    }

    if (southwest > biggestflowin)
    {
        biggestflowin = southwest;
        maininflow = 6;
    }

    if (west > biggestflowin)
    {
        biggestflowin = west;
        maininflow = 7;
    }

    if (northwest > biggestflowin)
    {
        biggestflowin = northwest;
        maininflow = 8;
    }

    // maininflow now holds the direction of the largest inflowing river.

    bool diag = 0; // This will show whether the main river is flowing diagonally right across the tile, from corner to corner.

    if ((maininflow == 2 && riverdir == 6) || (maininflow == 4 && riverdir == 8) || (maininflow == 6 && riverdir == 2) || (maininflow == 8 && riverdir == 4))
        diag = 1;

    // Now we need to work out where the *outflowing* river is going.
    // This will be shown by a *negative* value for the relevant direction.

    bool foundone = 0;

    switch (riverdir)
    {
    case 1:
        north = 0 - aveflow;
        foundone = 1;
        break;

    case 2:
        northeast = 0 - aveflow;
        foundone = 1;
        break;

    case 3:
        east = 0 - aveflow;
        foundone = 1;
        break;

    case 4:
        southeast = 0 - aveflow;
        foundone = 1;
        break;

    case 5:
        south = 0 - aveflow;
        foundone = 1;
        break;

    case 6:
        southwest = 0 - aveflow;
        foundone = 1;
        break;

    case 7:
        west = 0 - aveflow;
        foundone = 1;
        break;

    case 8:
        northwest = 0 - aveflow;
        foundone = 1;
        break;
    }

    if (foundone == 0)
        north = 0 - aveflow;

    int nseed = (sy * width + sx) + world.nom(sx, sy) + world.julrain(sx, sy) + riverjan; // Add the flow here so it isn't exactly the same as the western edge seed (which would make for weirdly symmetrical river courses).
    int sseed = (soy * width + sox) + world.nom(sox, soy) + world.julrain(sox, soy) + world.riverjan(sox, soy);
    int wseed = (sy * width + sx) + world.nom(sx, sy) + world.julrain(sx, sy);
    int eseed = (eay * width + eax) + world.nom(eax, eay) + world.julrain(eax, eay);

    // Now work out how many additional rivers there are.

    int additionalrivers = -1; // Start with -1 because one of the ones we'll be adding is the main river itself.

    if (north != 0)
        additionalrivers++;

    if (south != 0)
        additionalrivers++;

    if (east != 0)
        additionalrivers++;

    if (west != 0)
        additionalrivers++;

    if (northeast != 0)
        additionalrivers++;

    if (southeast != 0)
        additionalrivers++;

    if (northwest != 0)
        additionalrivers++;

    if (southwest != 0)
        additionalrivers++;

    // Now get coordinates for the points where rivers will appear into or disappear out of the tile.

    fast_srand(nseed);

    int northpointx = random(dx + minpoint, dx + 16 - minpoint);
    int northpointy = dy;

    fast_srand(sseed);

    int southpointx = random(dx + minpoint, dx + 16 - minpoint);
    int southpointy = dy + 16;

    fast_srand(wseed);

    int westpointx = dx;
    int westpointy = random(dy + minpoint, dy + 16 - minpoint);

    fast_srand(eseed);

    int eastpointx = dx + 16;
    int eastpointy = random(dy + minpoint, dy + 16 - minpoint);

    int northeastpointx = dx + 16;
    int northeastpointy = dy;

    int northwestpointx = dx;
    int northwestpointy = dy;

    int southeastpointx = dx + 16;
    int southeastpointy = dy + 16;

    int southwestpointx = dx;
    int southwestpointy = dy + 16;

    fast_srand((sy * width + sx) + world.nom(sx, sy) + world.julrain(sx, sy));

    // Now we get the coordinates of the main inflow and outflow.

    int inx = 0;
    int iny = 0;

    switch (maininflow)
    {
    case 1:
        inx = northpointx;
        iny = northpointy;
        break;

    case 2:
        inx = northeastpointx;
        iny = northeastpointy;
        break;

    case 3:
        inx = eastpointx;
        iny = eastpointy;
        break;

    case 4:
        inx = southeastpointx;
        iny = southeastpointy;
        break;

    case 5:
        inx = southpointx;
        iny = southpointy;
        break;

    case 6:
        inx = southwestpointx;
        iny = southwestpointy;
        break;

    case 7:
        inx = westpointx;
        iny = westpointy;
        break;

    case 8:
        inx = northwestpointx;
        iny = northwestpointy;
        break;
    }

    int outx = 0;
    int outy = 0;

    if (north < 0)
    {
        outx = northpointx;
        outy = northpointy;
    }

    if (south < 0)
    {
        outx = southpointx;
        outy = southpointy;
    }

    if (east < 0)
    {
        outx = eastpointx;
        outy = eastpointy;
    }

    if (west < 0)
    {
        outx = westpointx;
        outy = westpointy;
    }

    if (northeast < 0)
    {
        outx = northeastpointx;
        outy = northeastpointy;
    }

    if (northwest < 0)
    {
        outx = northwestpointx;
        outy = northwestpointy;
    }

    if (southeast < 0)
    {
        outx = southeastpointx;
        outy = southeastpointy;
    }

    if (southwest < 0)
    {
        outx = southwestpointx;
        outy = southwestpointy;
    }

    // Now we can use all of that to get the central junction location.

    twointegers jp = getjunctionpoint(world, region, dx, dy, sx, sy, lakepresent, goingtolake, diag, maininflow, inx, iny, outx, outy);

    int junctionpointx = jp.x;
    int junctionpointy = jp.y;

    fast_srand((sy * width + sx) + world.nom(sx, sy) + world.julrain(sx, sy));

    // By default, the river begins at the junction point (for carving purposes).
    // If there is inflow, this will be changed in a moment.

    int riverstartx = junctionpointx;
    int riverstarty = junctionpointy;
    int riverlength = 0;

    int startlandlevel = world.nom(sx, sy) - riverlandreduce;

    // Now we draw the first river, from the main inflow to the junction point and then to the outflow.

    int janinflow = 0, julinflow = 0;
    int fromtilelevel = 0;
    int totilelevel = 0;

    if (maininflow == 1)
    {
        mm1.x = (float) northpointx;
        mm1.y = (float) northpointy;

        janinflow = world.riverjan(nox, noy);
        julinflow = world.riverjul(nox, noy);

        fromtilelevel = nexttolake(world, nox, noy);

        if (fromtilelevel == 0)
            fromtilelevel = world.riftlakesurface(nox, noy);

        if (fromtilelevel == 0)
            fromtilelevel = world.nom(nox, noy) - riverlandreduce;
    }

    if (maininflow == 2)
    {
        mm1.x = (float) northeastpointx;
        mm1.y = (float) northeastpointy;

        janinflow = world.riverjan(nex, ney);
        julinflow = world.riverjul(nex, ney);

        fromtilelevel = nexttolake(world, nex, ney);

        if (fromtilelevel == 0)
            fromtilelevel = world.riftlakesurface(nex, ney);

        if (fromtilelevel == 0)
            fromtilelevel = world.nom(nex, ney) - riverlandreduce;
    }

    if (maininflow == 3)
    {
        mm1.x = (float) eastpointx;
        mm1.y = (float) eastpointy;

        janinflow = world.riverjan(eax, eay);
        julinflow = world.riverjul(eax, eay);

        fromtilelevel = nexttolake(world, eax, eay);

        if (fromtilelevel == 0)
            fromtilelevel = world.riftlakesurface(eax, eay);

        if (fromtilelevel == 0)
            fromtilelevel = world.nom(eax, eay) - riverlandreduce;
    }

    if (maininflow == 4)
    {
        mm1.x = (float) southeastpointx;
        mm1.y = (float) southeastpointy;

        janinflow = world.riverjan(sex, sey);
        julinflow = world.riverjul(sex, sey);

        fromtilelevel = nexttolake(world, sex, sey);

        if (fromtilelevel == 0)
            fromtilelevel = world.riftlakesurface(sex, sey);

        if (fromtilelevel == 0)
            fromtilelevel = world.nom(sex, sey) - riverlandreduce;
    }

    if (maininflow == 5)
    {
        mm1.x = (float) southpointx;
        mm1.y = (float) southpointy;

        janinflow = world.riverjan(sox, soy);
        julinflow = world.riverjul(sox, soy);

        fromtilelevel = nexttolake(world, sox, soy);

        if (fromtilelevel == 0)
            fromtilelevel = world.riftlakesurface(sox, soy);

        if (fromtilelevel == 0)
            fromtilelevel = world.nom(sox, soy) - riverlandreduce;
    }

    if (maininflow == 6)
    {
        mm1.x = (float) southwestpointx;
        mm1.y = (float) southwestpointy;

        janinflow = world.riverjan(swx, swy);
        julinflow = world.riverjul(swx, swy);

        fromtilelevel = nexttolake(world, swx, swy);

        if (fromtilelevel == 0)
            fromtilelevel = world.riftlakesurface(swx, swy);

        if (fromtilelevel == 0)
            fromtilelevel = world.nom(swx, swy) - riverlandreduce;
    }

    if (maininflow == 7)
    {
        mm1.x = (float) westpointx;
        mm1.y = (float) westpointy;

        janinflow = world.riverjan(wex, wey);
        julinflow = world.riverjul(wex, wey);

        fromtilelevel = nexttolake(world, wex, wey);

        if (fromtilelevel == 0)
            fromtilelevel = world.riftlakesurface(wex, wey);

        if (fromtilelevel == 0)
            fromtilelevel = world.nom(wex, wey) - riverlandreduce;
    }

    if (maininflow == 8)
    {
        mm1.x = (float) northwestpointx;
        mm1.y = (float) northwestpointy;

        janinflow = world.riverjan(nwx, nwy);
        julinflow = world.riverjul(nwx, nwy);

        fromtilelevel = nexttolake(world, nwx, nwy);

        if (fromtilelevel == 0)
            fromtilelevel = world.riftlakesurface(nwx, nwy);

        if (fromtilelevel == 0)
            fromtilelevel = world.nom(nwx, nwy) - riverlandreduce;
    }

    if (world.truelake(sx, sy) == 1)
    {
        janinflow = world.riverjan(sx, sy);
        julinflow = world.riverjul(sx, sy);
    }
    else
    {
        twointegers upstream = getupstreamcell(world, sx, sy);

        if (world.truelake(upstream.x, upstream.y) == 1)
        {
            janinflow = world.riverjan(sx, sy);
            julinflow = world.riverjul(sx, sy);
        }
    }

    if (biggestflowin == 0) // If this is the start of the river, the flow should be the flow for this tile.
    {
        janinflow = world.riverjan(sx, sy);
        julinflow = world.riverjul(sx, sy);
    }

    // First we draw the first half, inflow to central junction.

    if (north > 0 || northeast > 0 || east > 0 || southeast > 0 || south > 0 || southwest > 0 || west > 0 || northwest > 0) // Only do this if there actually is an inflow.
    {
        int thistilelevel = world.nom(sx, sy) - riverlandreduce;

        if (lakepresent != 0)
            thistilelevel = surfacelevel;

        startlandlevel = (fromtilelevel + thistilelevel) / 2;

        mm3.x = (float) junctionpointx;
        mm3.y = (float) junctionpointy;

        mm2.x = (float) (mm3.x + mm1.x) / 2;
        mm2.y = (float) (mm3.y + mm1.y) / 2;

        if (diag == 1)
        {
            mm2.x = (float) mm2.x + randomsign(random(minmidvardiag, maxmidvardiag));
            mm2.y = (float) mm2.y + randomsign(random(minmidvardiag, maxmidvardiag));
        }
        else
        {
            mm2.x = (float) mm2.x + randomsign(random(minmidvar, maxmidvar));
            mm2.y = (float) mm2.y + randomsign(random(minmidvar, maxmidvar));
        }

        if (mm2.x < dx + margin)
            mm2.x = (float) dx + margin;

        if (mm2.x > dx + mmargin)
            mm2.x = (float) dx + mmargin;

        if (mm2.y < dy + margin)
            mm2.y = (float) dy + margin;

        if (mm2.y > dy + mmargin)
            mm2.y = (float) dy + mmargin;

        if (lakepresent == 1)
        {
            if (north < 0 && world.truelake(nox, noy) == 1)
                goingtolake = 1;

            if (south < 0 && world.truelake(sox, soy) == 1)
                goingtolake = 1;

            if (east < 0 && world.truelake(eax, eay) == 1)
                goingtolake = 1;

            if (west < 0 && world.truelake(wex, wey) == 1)
                goingtolake = 1;

            if (northeast < 0 && world.truelake(nex, ney) == 1)
                goingtolake = 1;

            if (northwest < 0 && world.truelake(nwx, nwy) == 1)
                goingtolake = 1;

            if (southeast < 0 && world.truelake(sex, sey) == 1)
                goingtolake = 1;

            if (southwest < 0 && world.truelake(swx, swy) == 1)
                goingtolake = 1;
        }

        riverstartx = (int)mm1.x;
        riverstarty = (int)mm1.y;

        riverlength = 1;

        rinfo = calculateregionalriver(world, region, dx, dy, sx, sy, pt, mm1, mm2, mm3, janinflow, julinflow, riverlength, 0, goingtolake, 0);

        riverlength = rinfo.z;

        if (region.riverdir(junctionpointx, junctionpointy) == 0) // If for some reason it hasn't reached the junction point
        {
            for (int i = junctionpointx - 1; i <= junctionpointx + 1; i++)
            {
                for (int j = junctionpointy - 1; j <= junctionpointy + 1; j++)
                {
                    if (region.riverdir(i, j) != 0)
                    {
                        int dir = region.riverdir(i, j);

                        twointegers dest = getregionalflowdestination(region, i, j, dir);

                        if (dest.x == junctionpointx && dest.y == junctionpointy)
                        {
                            region.setriverdir(junctionpointx, junctionpointy, dir);
                            region.setriverjan(junctionpointx, junctionpointy, janinflow);
                            region.setriverjul(junctionpointx, junctionpointy, julinflow);
                        }
                    }
                }
            }
        }
    }

    // Now we draw the second half, from the central junction to the outflow.

    mm1.x = (float) junctionpointx;
    mm1.y = (float) junctionpointy;

    int thistilelevel = world.nom(sx, sy) - riverlandreduce;

    if (lakepresent == 1)
        thistilelevel = surfacelevel;

    if (north < 0)
    {
        mm3.x = (float) northpointx;
        mm3.y = (float) northpointy;

        totilelevel = world.lakesurface(nox, noy);

        if (totilelevel == 0)
            totilelevel = world.nom(nox, noy) - riverlandreduce;
        else
            goingtolake = 1;
    }

    if (south < 0)
    {
        mm3.x = (float) southpointx;
        mm3.y = (float) southpointy;

        totilelevel = world.lakesurface(sox, soy);

        if (totilelevel == 0)
            totilelevel = world.nom(sox, soy) - riverlandreduce;
        else
            goingtolake = 1;
    }

    if (east < 0)
    {
        mm3.x = (float) eastpointx;
        mm3.y = (float) eastpointy;

        totilelevel = world.lakesurface(eax, eay);

        if (totilelevel == 0)
            totilelevel = world.nom(eax, eay) - riverlandreduce;
        else
            goingtolake = 1;
    }

    if (west < 0)
    {
        mm3.x = (float) westpointx;
        mm3.y = (float) westpointy;

        totilelevel = world.lakesurface(wex, wey);

        if (totilelevel == 0)
            totilelevel = world.nom(wex, wey) - riverlandreduce;
        else
            goingtolake = 1;
    }

    if (northeast < 0)
    {
        mm3.x = (float) northeastpointx;
        mm3.y = (float) northeastpointy;

        totilelevel = world.lakesurface(nex, ney);

        if (totilelevel == 0)
            totilelevel = world.nom(nex, ney) - riverlandreduce;
        else
            goingtolake = 1;
    }

    if (northwest < 0)
    {
        mm3.x = (float) northwestpointx;
        mm3.y = (float) northwestpointy;

        totilelevel = world.lakesurface(nwx, nwy);

        if (totilelevel == 0)
            totilelevel = world.nom(nwx, nwy) - riverlandreduce;
        else
            goingtolake = 1;
    }

    if (southeast < 0)
    {
        mm3.x = (float) southeastpointx;
        mm3.y = (float) southeastpointy;

        totilelevel = world.lakesurface(sex, sey);

        if (totilelevel == 0)
            totilelevel = world.nom(sex, sey) - riverlandreduce;
        else
            goingtolake = 1;
    }

    if (southwest < 0)
    {
        mm3.x = (float) southwestpointx;
        mm3.y = (float) southwestpointy;

        totilelevel = world.lakesurface(swx, swy);

        if (totilelevel == 0)
            totilelevel = world.nom(swx, swy) - riverlandreduce;
        else
            goingtolake = 1;
    }

    int endlandlevel = (thistilelevel + totilelevel) / 2;

    mm2.x = (float) (mm3.x + mm1.x) / 2;
    mm2.y = (float) (mm3.y + mm1.y) / 2;

    if (diag == 1)
    {
        mm2.x = (float) mm2.x + randomsign(random(minmidvardiag, maxmidvardiag));
        mm2.y = (float) mm2.y + randomsign(random(minmidvardiag, maxmidvardiag));
    }
    else
    {
        mm2.x = (float) mm2.x + randomsign(random(minmidvar, maxmidvar));
        mm2.y = (float) mm2.y + randomsign(random(minmidvar, maxmidvar));
    }

    if (mm2.x < dx + margin)
        mm2.x = (float) dx + margin;

    if (mm2.x > dx + mmargin)
        mm2.x = (float) dx + mmargin;

    if (mm2.y < dy + margin)
        mm2.y = (float) dy + margin;

    if (mm2.y > dy + mmargin)
        mm2.y = (float) dy + mmargin;

    int riverendx = (int)mm3.x;
    int riverendy = (int)mm3.y;

    rinfo = calculateregionalriver(world, region, dx, dy, sx, sy, pt, mm1, mm2, mm3, janinflow, julinflow, riverlength, 0, goingtolake, 0);

    riverlength = rinfo.z;

    // Now remove any orphans there might be.

    if (lakepresent == 0)
        riverlength = removeriverorphans(world, region, sx, sy, dx, dy, riverlength);

    // Now get rid of any annoying straight bits.

    removeregionalstraightrivers(world, region, dx, dy, sx, sy, rivercurves);

    riverlength = findriverlength(region, riverstartx, riverstarty, riverendx, riverendy, 0);

    // Now mark that river on our array that holds just the main river.

    for (int i = 0; i <= 16; i++)
    {
        for (int j = 0; j <= 16; j++)
        {
            if (region.riverdir(dx + i, dy + j) != 0)
                mainriver[i][j] = 1;
        }
    }

    // Now we need to erode the land under the river to ensure it runs downhill all the way.

    fast_srand((sy * width + sx) + world.nom(sx, sy) + world.julrain(sx, sy));

    carveriver(world, region, dx, dy, sx, sy, riverlength, riverstartx, riverstarty, riverendx, riverendy, startlandlevel, endlandlevel, goingtolake, rriverscarved);

    // Now we need to calculate where other rivers will join this one.
    // Each one needs a different junction because it would look weird if they all joined the main river at the same junction point. additionalrivers is the number we need.

    if (additionalrivers > 0)
    {
        twointegers junction;

        bool extrarivers[9]; // This will tell us which incoming rivers still need doing.

        for (int n = 0; n <= 8; n++)
            extrarivers[n] = 0;

        if (north > 0 && maininflow != 1)
            extrarivers[1] = 1;

        if (northeast > 0 && maininflow != 2)
            extrarivers[2] = 1;

        if (east > 0 && maininflow != 3)
            extrarivers[3] = 1;

        if (southeast > 0 && maininflow != 4)
            extrarivers[4] = 1;

        if (south > 0 && maininflow != 5)
            extrarivers[5] = 1;

        if (southwest > 0 && maininflow != 6)
            extrarivers[6] = 1;

        if (west > 0 && maininflow != 7)
            extrarivers[7] = 1;

        if (northwest > 0 && maininflow != 8)
            extrarivers[8] = 1;

        bool keepgoing = 0;

        do
        {
            keepgoing = 0;

            int rivernumber = 0;

            for (int nn = 1; nn <= 8; nn++)
            {
                if (extrarivers[nn] == 1)
                {
                    rivernumber = nn;
                    keepgoing = 1;
                }
            }

            if (keepgoing == 1)
            {
                // rivernumber is now the direction of the current inflowing one being done.

                extrarivers[rivernumber] = 0; // Clear in that array so we don't do it again.

                // Now we draw that river, from its point of origin to the junction.

                // First, find its origin point.

                int thistilelevel = world.nom(sx, sy) - riverlandreduce;

                if (lakepresent != 0)
                    thistilelevel = surfacelevel;

                int janinflow = 0, julinflow = 0, fromtilelevel = 0;

                if (rivernumber == 1)
                {
                    mm1.x = (float) northpointx;
                    mm1.y = (float) northpointy;

                    janinflow = world.riverjan(nox, noy);
                    julinflow = world.riverjul(nox, noy);

                    if (world.lakesurface(nox, noy) == 0)
                        fromtilelevel = world.nom(nox, noy) - riverlandreduce;
                    else
                        fromtilelevel = nexttolake(world, nox, noy);
                }

                if (rivernumber == 2)
                {
                    mm1.x = (float) northeastpointx;
                    mm1.y = (float) northeastpointy;

                    janinflow = world.riverjan(nex, ney);
                    julinflow = world.riverjul(nex, ney);

                    if (world.lakesurface(nex, ney) == 0)
                        fromtilelevel = world.nom(nex, ney) - riverlandreduce;
                    else
                        fromtilelevel = nexttolake(world, nex, ney);
                }

                if (rivernumber == 3)
                {
                    mm1.x = (float) eastpointx;
                    mm1.y = (float) eastpointy;

                    janinflow = world.riverjan(eax, eay);
                    julinflow = world.riverjul(eax, eay);

                    if (world.lakesurface(eax, eay) == 0)
                        fromtilelevel = world.nom(eax, eay) - riverlandreduce;
                    else
                        fromtilelevel = nexttolake(world, eax, eay);
                }

                if (rivernumber == 4)
                {
                    mm1.x = (float) southeastpointx;
                    mm1.y = (float) southeastpointy;

                    janinflow = world.riverjan(sex, sey);
                    julinflow = world.riverjul(sex, sey);

                    if (world.lakesurface(sex, sey) == 0)
                        fromtilelevel = world.nom(sex, sey) - riverlandreduce;
                    else
                        fromtilelevel = nexttolake(world, sex, sey);
                }

                if (rivernumber == 5)
                {
                    mm1.x = (float) southpointx;
                    mm1.y = (float) southpointy;

                    janinflow = world.riverjan(sox, soy);
                    julinflow = world.riverjul(sox, soy);

                    if (world.lakesurface(sox, soy) == 0)
                        fromtilelevel = world.nom(sox, soy) - riverlandreduce;
                    else
                        fromtilelevel = nexttolake(world, sox, soy);
                }

                if (rivernumber == 6)
                {
                    mm1.x = (float) southwestpointx;
                    mm1.y = (float) southwestpointy;

                    janinflow = world.riverjan(swx, swy);
                    julinflow = world.riverjul(swx, swy);

                    if (world.lakesurface(swx, swy) == 0)
                        fromtilelevel = world.nom(swx, swy) - riverlandreduce;
                    else
                        fromtilelevel = nexttolake(world, swx, swy);
                }

                if (rivernumber == 7)
                {
                    mm1.x = (float) westpointx;
                    mm1.y = (float) westpointy;

                    janinflow = world.riverjan(wex, wey);
                    julinflow = world.riverjul(wex, wey);

                    if (world.lakesurface(wex, wey) == 0)
                        fromtilelevel = world.nom(wex, wey) - riverlandreduce;
                    else
                        fromtilelevel = nexttolake(world, wex, wey);
                }

                if (rivernumber == 8)
                {
                    mm1.x = (float) northwestpointx;
                    mm1.y = (float) northwestpointy;

                    janinflow = world.riverjan(nwx, nwy);
                    julinflow = world.riverjul(nwx, nwy);

                    if (world.lakesurface(nwx, nwy) == 0)
                        fromtilelevel = world.nom(nwx, nwy) - riverlandreduce;
                    else
                        fromtilelevel = nexttolake(world, nwx, nwy);
                }

                if (totilelevel <= sealevel)
                    totilelevel = sealevel + 1;

                if (fromtilelevel <= sealevel)
                    fromtilelevel = sealevel + 1;

                int thisstartlandlevel = (fromtilelevel + thistilelevel) / 2;

                int thisriverstartx = (int)mm1.x;
                int thisriverstarty = (int)mm1.y;

                // Now we find a random river point within this tile to be the junction (destination) of this river.

                mm3.x = (float) -1;
                mm3.y = (float) -1;

                int a, b, c, d, e, f;

                if (random(0, 1) == 1)
                {
                    a = dx + 5;
                    b = dx + 10;
                    c = 1;
                }
                else
                {
                    a = dx + 10;
                    b = dx + 5;
                    c = -1;
                }

                if (random(0, 1) == 1)
                {
                    d = dy + 5;
                    e = dy + 10;
                    f = 1;
                }
                else
                {
                    d = dy + 10;
                    e = dy + 5;
                    f = -1;
                }

                for (int i = a; i != b; i = i + c)
                {
                    for (int j = d; j != e; j = j + f)
                    {
                        if (region.riverjan(i, j) != 0 || region.riverjul(i, j) != 0)
                        {
                            mm3.x = (float) i;
                            mm3.y = (float) j;
                        }
                    }
                }

                if (mm3.x == -1) // If we didn't find a suitable point, try alternatives
                {
                    if (region.riverjan(junctionpointx, junctionpointy) != 0 || region.riverjul(junctionpointx, junctionpointy) != 0)
                    {
                        mm3.x = (float) junctionpointx;
                        mm3.y = (float) junctionpointy;
                    }
                    else
                    {
                        mm3.x = (float) riverstartx;
                        mm3.y = (float) riverstarty;
                    }
                }

                mm2.x = (float) (mm3.x + mm1.x) / 2;
                mm2.y = (float) (mm3.y + mm1.y) / 2;

                mm2.x = (float) mm2.x + randomsign(random(minmidvar, maxmidvar));
                mm2.y = (float) mm2.y + randomsign(random(minmidvar, maxmidvar));

                if (mm2.x > dx + 14)
                    mm2.x = (float) dx + 14;

                if (mm2.x < dx + 2)
                    mm2.x = (float) dx + 2;

                if (mm2.y > dy + 14)
                    mm2.y = (float) dy + 14;

                if (mm2.y < dy + 2)
                    mm2.y = (float) dy + 2;

                rinfo = calculateregionalriver(world, region, dx, dy, sx, sy, pt, mm1, mm2, mm3, janinflow, julinflow, 1, 1, goingtolake, 0);

                // Now get rid of any annoying straight bits, again.

                removeregionalstraightrivers(world, region, dx, dy, sx, sy, rivercurves);

                int thisriverendx = rinfo.x;
                int thisriverendy = rinfo.y;
                int thisriverlength = rinfo.z;

                int thisendlandlevel = region.map(thisriverendx, thisriverendy);

                if (lakepresent == 0)
                    removeriverorphans(world, region, sx, sy, dx, dy, 1);

                carverivertributary(world, region, dx, dy, sx, sy, thisriverlength, thisriverstartx, thisriverstarty, thisriverendx, thisriverendy, thisstartlandlevel, thisendlandlevel, riverendx, riverendy, endlandlevel, rriverscarved);
            }
        } while (keepgoing == 1);
    }

    // Now for springs! These are small rivers that arise within the tile and flow into existing rivers. They deliver the rainfall from this tile itself.

    int maxmountains = 500; // The mountains must be lower than this...
    int minriver = 100; // ...unless the river is bigger than this.

    bool dosprings = 1;

    if (lakepresent != 0)
        dosprings = 0;

    if (world.mountainheight(sx, sy) > maxmountains && aveflow < minriver)
        dosprings = 0;

    if (dosprings == 1)
    {
        for (int i = sx - 1; i <= sx + 1; i++)
        {
            int ii = i;

            if (ii<0 || ii>width)
                ii = wrap(ii, width);

            for (int j = sy - 1; j <= sy + 1; j++)
            {
                if (j >= 0 && j <= height)
                {
                    if (world.riftlakesurface(i, j) != 0)
                    {
                        dosprings = 0;
                        i = sx + 1;
                        j = sy + 1;
                    }
                }
            }
        }
    }

    if (dosprings == 1)
    {
        bool goahead = 1; // Don't do this if we already have any sea or lakes in the tile. It messes it up.

        for (int i = dx; i <= dx + 16; i++)
        {
            for (int j = dy; j <= dy + 16; j++)
            {
                if (1 == 0) //(region.map(i,j)>0 && region.sea(i,j)==1)
                {
                    goahead = 0;
                    i = dx + 16;
                    j = dy + 16;
                }

                if (1 == 0) //(region.truelake(i,j)==1)
                {
                    goahead = 0;
                    i = dx + 16;
                    j = dy + 16;
                }
            }
        }

        if (goahead == 1)
        {
            addsprings(world, region, sx, sy, dx, dy, junctionpointx, junctionpointy, riverendx, riverendy, maxmidvar, rriverscarved);
        }
    }

    // Now remove any over-long straight sections.

    removeregionalriverstraights(region, dx, dy, 0, rriverscarved);

    // Now remove any odd sections where the flow is too low.

    removelowflow(region, dx, dy);

    // Remove any negative flows.

    removenegativeflow(region, dx, dy);

    // Now possibly add a small lake!

    if (world.sea(sx, sy) || world.volcano(sx, sy) != 0)
        return;

    for (int i = sx - 2; i <= sx + 2; i++)
    {
        int ii = i;

        if (ii<0 || ii>width)
            ii = wrap(ii, width);

        for (int j = sy - 2; j <= sy + 2; j++)
        {
            if (j >= 0 && j <= height)
            {
                if (world.sea(ii, j))
                    return;
            }
        }
    }

    int averain = world.averain(sx, sy);
    int avetemp = world.avetemp(sx, sy);

    int lakechance = 25;
    int minlake = 40; // Flows of this size or higher might have lakes.
    int maxlake = 800; // Flows larger than this can't have lakes.

    if (avetemp < world.glacialtemp()) // Lakes are common where there were once glaciers.
    {
        int sxx = sx + width / 2;

        if (sxx > width)
            sxx = sxx - width;

        if (world.noise(sxx, sy) < world.maxelevation() / 2) // Only in some areas
        {
            lakechance = 6;
            minlake = 5;
            maxlake = 800;
        }
    }

    lakechance = lakechance - (averain / 300); // The more rain there is, the more likely lakes are.

    int div = world.maxelevation() / 20;

    lakechance = lakechance - (int)(world.roughness(sx, sy) / (float)div); // Rougher areas of the map have more lakes.

    if (lakechance < 2)
        lakechance = 2;

    if (random(1, lakechance) == 1)
    {
        if (aveflow >= minlake && aveflow <= maxlake)
        {
            if (region.riverdir(junctionpointx, junctionpointy) != 0)
            {
                short climate = world.climate(sx, sy);
                {
                    if (climate != 5 && climate != 7) // Don't put lakes onto hot/mild deserts/semi-arid climates.
                    {
                        if (nearlake(world, sx, sy, 1, 1) == 0) // Don't do this near big lakes
                        {
                            bool seapresent = 0;

                            for (int i = dx; i <= dx + 16; i++)
                            {
                                for (int j = dy; j <= dy + 16; j++)
                                {
                                    if (region.map(i, j) <= sealevel && region.map(i, j) != 0)
                                    {
                                        seapresent = 1;
                                        i = dx + 16;
                                        j = dy + 16;
                                    }
                                }
                            }

                            if (seapresent == 0)
                            {
                                if (avetemp <= world.glacialtemp())
                                    createsmallglaciallake(world, region, dx, dy, sx, sy, junctionpointx, junctionpointy, mainriver);

                                else
                                    createsmalllake(world, region, dx, dy, sx, sy, junctionpointx, junctionpointy, mainriver, smalllake);
                            }
                        }
                    }
                }
            }
        }
    }
}

// This finds a junction point for the river tile.

twointegers getjunctionpoint(planet& world, region& region, int dx, int dy, int sx, int sy, bool lakepresent, bool goingtolake, bool diag, int maininflow, int inx, int iny, int outx, int outy)
{
    int width = world.width();
    int height = world.height();

    int riverdir = world.riverdir(sx, sy);

    int maxvar = 4; // Maximum amount the central junction can be offset by.
    int maxvardiag = 6; // Maximum amount the central junction can be offset by on diagonal tiles.
    int minvardiag = 3; // Minimum amount the central junction can be offset by on diagonal tiles.

    int junctionpointx = -1;
    int junctionpointy = -1;

    fast_srand((sy * width + sx) + world.nom(sx, sy) + world.julrain(sx, sy));

    if (lakepresent == 1 && goingtolake == 0) //  If this is a river flowing out of a lake, make sure the junction point is in the lake.
    {
        int a, b, c, d, e, f;

        if (random(0, 1) == 1)
        {
            a = dx;
            b = dx + 16;
            c = 1;
        }
        else
        {
            a = dx + 16;
            b = dx;
            c = -1;
        }

        if (random(0, 1) == 1)
        {
            d = dy;
            e = dy + 16;
            f = 1;
        }
        else
        {
            d = dy + 16;
            e = dy;
            f = -1;
        }

        for (int i = a; i != b; i = i + c)
        {
            for (int j = d; j != e; j = j + f)
            {
                if (region.truelake(i, j) == 1)
                {
                    junctionpointx = i;
                    junctionpointy = j;
                }
            }
        }
    }
    else // Just find the junction point normally.
    {
        junctionpointx = (inx + outx) / 2;
        junctionpointx = (junctionpointx + dx + 8) / 2;

        junctionpointy = (iny + outy) / 2;
        junctionpointy = (junctionpointy + dy + 8) / 2;

        if (diag == 1) // If the main river is going diagonally across the tile, we need to make sure the junction point is well off-centre
        {
            if ((maininflow == 8 && riverdir == 4) || (maininflow == 4 && riverdir == 8)) // Top left to bottom right or vice versa
            {
                if (random(1, 2) == 1)
                {
                    junctionpointx = junctionpointx + random(minvardiag, maxvardiag);
                    junctionpointy = junctionpointy - random(minvardiag, maxvardiag);
                }
                else
                {
                    junctionpointx = junctionpointx - random(minvardiag, maxvardiag);
                    junctionpointy = junctionpointy + random(minvardiag, maxvardiag);
                }
            }
            else
            {
                if (random(1, 2) == 1)
                {
                    junctionpointx = junctionpointx + random(minvardiag, maxvardiag);
                    junctionpointy = junctionpointy + random(minvardiag, maxvardiag);
                }
                else
                {
                    junctionpointx = junctionpointx - random(minvardiag, maxvardiag);
                    junctionpointy = junctionpointy - random(minvardiag, maxvardiag);
                }
            }
        }
        else
        {
            junctionpointx = junctionpointx + randomsign(random(0, maxvar));
            junctionpointy = junctionpointy + randomsign(random(0, maxvar));
        }

        // If this isn't a lake tile and there's lake along the edges, move the junction point away from it.

        int amounttomove = 6;

        if (lakepresent == 0)
        {
            for (int n = dx; n <= dx + 16; n++)
            {
                if (region.truelake(n, dy) != 0)
                {
                    junctionpointy = junctionpointy + amounttomove;
                    n = dx + 16;
                }
            }

            for (int n = dx; n <= dx + 16; n++)
            {
                if (region.truelake(n, dy + 16) != 0)
                {
                    junctionpointy = junctionpointy - amounttomove;
                    n = dx + 16;
                }
            }

            for (int n = dy; n <= dy + 16; n++)
            {
                if (region.truelake(dx, n) != 0)
                {
                    junctionpointx = junctionpointx + amounttomove;
                    n = dy + 16;
                }
            }

            for (int n = dy; n <= dy + 16; n++)
            {
                if (region.truelake(dx + 16, n) != 0)
                {
                    junctionpointx = junctionpointx - amounttomove;
                    n = dy + 16;
                }
            }

            if (junctionpointx < dx + 4)
                junctionpointx = dx + 4;

            if (junctionpointx > dx + 12)
                junctionpointx = dx + 12;

            if (junctionpointy < dy + 4)
                junctionpointy = dy + 4;

            if (junctionpointy > dy + 12)
                junctionpointy = dy + 12;
        }
    }

    twointegers jp;

    jp.x = junctionpointx;
    jp.y = junctionpointy;

    return jp;
}

// This adds delta branches to tiles on the regional map.

void makedeltatile(planet& world, region& region, int dx, int dy, int sx, int sy, vector<vector<int>>& rriverscarved)
{
    int deltajan = world.deltajan(sx, sy);
    int deltajul = world.deltajul(sx, sy);

    if (deltajan == 0 && deltajul == 0)
        return;

    int deltadir = world.deltadir(sx, sy);

    int width = world.width();

    int rwidth = region.rwidth();
    int rheight = region.rheight();

    for (int i = 0; i <= rwidth; i++)
    {
        for (int j = 0; j <= rheight; j++)
            rriverscarved[i][j] = -1;
    }

    vector<vector<int>> mainriver(17, vector<int>(17, 0));

    // First, get coordinates for the tiles around this one.

    int eax = sx + 1;
    int eay = sy;
    if (eax > width)
        eax = 0;

    int wex = sx - 1;
    int wey = sy;
    if (wex < 0)
        wex = width;

    int sox = sx;
    int soy = sy + 1;

    int nox = sx;
    int noy = sy - 1;

    int sex = eax;
    int sey = soy;

    int nwx = wex;
    int nwy = noy;

    int nex = eax;
    int ney = noy;

    int swx = wex;
    int swy = soy;

    int aveflow = (deltajan + deltajul) / 2;

    int joiningriver = -1;

    if (aveflow < 0)
    {
        aveflow = 0 - aveflow;
        joiningriver = 1; // This means that the branches will join the main river when they exit this tile.
    }

    if (world.deltadir(sx, sy) < 1)
        return;

    twofloats pt, mm1, mm2, mm3, mm4;
    twointegers riverpoint;

    threeintegers rinfo;

    int margin = 2; // This is to ensure that the midpoints aren't too close to the edges.
    int mmargin = 16 - margin;

    int maxvar = 4; // Maximum amount the central junction can be offset by.
    int maxmidvar = 3; //2; // Maximum amount the midpoints between the junction and the sides can be offset by.
    int minpoint = 3; // Minimum distance the points on the sides (where rivers go off the tile) can be from the corners.

    int maxvardiag = 6; // Maximum amount the central junction can be offset by on diagonal tiles.
    int minvardiag = 3; // Minimum amount the central junction can be offset by on diagonal tiles.
    int maxmidvardiag = 4; // Maximum amount the midpoints can be offset by on a diagonal tile.
    int minmidvardiag = 2; // Minimum amount the midpoints can be offset by on a diagonal tile.

    int north = 0;
    int south = 0;
    int east = 0;
    int west = 0;
    int northeast = 0;
    int northwest = 0;
    int southeast = 0;
    int southwest = 0; // These will hold the average flow of rivers coming from these directions.

    // Now check to see which directions have incoming rivers.

    if (world.deltadir(nox, noy) == 5) // North
        north = (world.deltajan(nox, noy) + world.deltajul(nox, noy)) / 2;

    if (world.deltadir(nex, ney) == 6) // Northeast
        northeast = (world.deltajan(nex, ney) + world.deltajul(nex, ney)) / 2;

    if (world.deltadir(eax, eay) == 7) // East
        east = (world.deltajan(eax, eay) + world.deltajul(eax, eay)) / 2;

    if (world.deltadir(sex, sey) == 8) // Southeast
        southeast = (world.deltajan(sex, sey) + world.deltajul(sex, sey)) / 2;

    if (world.deltadir(sox, soy) == 1) // South
        south = (world.deltajan(sox, soy) + world.deltajul(sox, soy)) / 2;

    if (world.deltadir(swx, swy) == 2) // Southwest
        southwest = (world.deltajan(swx, swy) + world.deltajul(swx, swy)) / 2;

    if (world.deltadir(wex, wey) == 3) // West
        west = (world.deltajan(wex, wey) + world.deltajul(wex, wey)) / 2;

    if (world.deltadir(nwx, nwy) == 4) // Northwest
        northwest = (world.deltajan(nwx, nwy) + world.deltajul(nwx, nwy)) / 2;

    // Now work out which of these is the largest.

    int biggestflowin = north;
    int maininflow = 1;

    if (northeast > biggestflowin)
    {
        biggestflowin = northeast;
        maininflow = 2;
    }

    if (east > biggestflowin)
    {
        biggestflowin = east;
        maininflow = 3;
    }

    if (southeast > biggestflowin)
    {
        biggestflowin = southeast;
        maininflow = 4;
    }

    if (south > biggestflowin)
    {
        biggestflowin = south;
        maininflow = 5;
    }

    if (southwest > biggestflowin)
    {
        biggestflowin = 6;
    }

    if (west > biggestflowin)
    {
        biggestflowin = west;
        maininflow = 7;
    }

    if (northwest > biggestflowin)
    {
        biggestflowin = northwest;
        maininflow = 8;
    }

    // maininflow now holds the direction of the largest inflowing river.

    bool diag = 0; // This will show whether the main river is flowing diagonally right across the tile, from corner to corner.

    if ((maininflow == 2 && deltadir == 6) || (maininflow == 4 && deltadir == 8) || (maininflow == 6 && deltadir == 2) || (maininflow == 8 && deltadir == 4))
        diag = 1;

    int nseed = (sy * width + sx) + world.nom(sx, sy) + world.janrain(sx, sy) + deltajan; // Add the flow here so it isn't exactly the same as the western edge seed (which would make for weirdly symmetrical river courses).
    int sseed = (soy * width + sox) + world.nom(sox, soy) + world.janrain(sox, soy) + world.deltajan(sox, soy);
    int wseed = (sy * width + sx) + world.nom(sx, sy) + world.janrain(sx, sy);
    int eseed = (eay * width + eax) + world.nom(eax, eay) + world.janrain(eax, eay);

    // Now we need to work out where the *outflowing* river is going.
    // This will be shown by a *negative* value for the relevant direction.

    switch (deltadir)
    {
    case 1:
        north = 0 - aveflow;
        if (joiningriver == 1)
            nseed = (sy * width + sx) + world.nom(sx, sy) + world.julrain(sx, sy) + world.riverjan(sx, sy);
        break;

    case 2:
        northeast = 0 - aveflow;
        break;

    case 3:
        east = 0 - aveflow;
        if (joiningriver == 1)
            eseed = (eay * width + eax) + world.nom(eax, eay) + world.julrain(eax, eay);
        break;

    case 4:
        southeast = 0 - aveflow;
        break;

    case 5:
        south = 0 - aveflow;
        if (joiningriver == 1)
            sseed = (soy * width + sox) + world.nom(sox, soy) + world.julrain(sox, soy) + world.riverjan(sox, soy);
        break;

    case 6:
        southwest = 0 - aveflow;
        break;

    case 7:
        west = 0 - aveflow;
        if (joiningriver == 1)
            wseed = (sy * width + sx) + world.nom(sx, sy) + world.julrain(sx, sy);
        break;

    case 8:
        northwest = 0 - aveflow;
        break;
    }

    // Now work out how many additional rivers there are.

    int additionalrivers = -1; // Start with -1 because one of the ones we'll be adding is the main river itself.

    if (north != 0)
        additionalrivers++;

    if (south != 0)
        additionalrivers++;

    if (east != 0)
        additionalrivers++;

    if (west != 0)
        additionalrivers++;

    if (northeast != 0)
        additionalrivers++;

    if (southeast != 0)
        additionalrivers++;

    if (northwest != 0)
        additionalrivers++;

    if (southwest != 0)
        additionalrivers++;

    // Now get coordinates for the points where rivers will appear into or disappear out of the tile.

    fast_srand(nseed);

    int northpointx = random(dx + minpoint, dx + 16 - minpoint);
    int northpointy = dy;

    fast_srand(sseed);

    int southpointx = random(dx + minpoint, dx + 16 - minpoint);
    int southpointy = dy + 16;

    fast_srand(wseed);

    int westpointx = dx;
    int westpointy = random(dy + minpoint, dy + 16 - minpoint);

    fast_srand(eseed);

    int eastpointx = dx + 16;
    int eastpointy = random(dy + minpoint, dy + 16 - minpoint);

    int northeastpointx = dx + 16;
    int northeastpointy = dy;

    int northwestpointx = dx;
    int northwestpointy = dy;

    int southeastpointx = dx + 16;
    int southeastpointy = dy + 16;

    int southwestpointx = dx;
    int southwestpointy = dy + 16;

    fast_srand((sy * width + sx) + world.nom(sx, sy) + world.janrain(sx, sy));

    // Now we get the coordinates of the main inflow and outflow.

    int inx, iny;

    switch (maininflow)
    {
    case 1:
        inx = northpointx;
        iny = northpointy;
        break;

    case 2:
        inx = northeastpointx;
        iny = northeastpointy;
        break;

    case 3:
        inx = eastpointx;
        iny = eastpointy;
        break;

    case 4:
        inx = southeastpointx;
        iny = southeastpointy;
        break;

    case 5:
        inx = southpointx;
        iny = southpointy;
        break;

    case 6:
        inx = southwestpointx;
        iny = southwestpointy;
        break;

    case 7:
        inx = westpointx;
        iny = westpointy;
        break;

    case 8:
        inx = northwestpointx;
        iny = northwestpointy;
        break;
    }

    int outx = 0, outy = 0;

    if (north < 0)
    {
        outx = northpointx;
        outy = northpointy;
    }

    if (south < 0)
    {
        outx = southpointx;
        outy = southpointy;
    }

    if (east < 0)
    {
        outx = eastpointx;
        outy = eastpointy;
    }

    if (west < 0)
    {
        outx = westpointx;
        outy = westpointy;
    }

    if (northeast < 0)
    {
        outx = northeastpointx;
        outy = northeastpointy;
    }

    if (northwest < 0)
    {
        outx = northwestpointx;
        outy = northwestpointy;
    }

    if (southeast < 0)
    {
        outx = southeastpointx;
        outy = southeastpointy;
    }

    if (southwest < 0)
    {
        outx = southwestpointx;
        outy = southwestpointy;
    }

    // Now we can use all of that to get the central junction location.

    int junctionpointx, junctionpointy;

    fast_srand((sy * width + sx) + world.nom(sx, sy) + world.julrain(sx, sy));

    junctionpointx = (inx + outx) / 2;
    junctionpointx = (junctionpointx + dx + 8) / 2;

    junctionpointy = (iny + outy) / 2;
    junctionpointy = (junctionpointy + dy + 8) / 2;

    if (diag == 1) // If the main river is going diagonally across the tile, we need to make sure the junction point is well off-centre
    {
        if ((maininflow == 8 && deltadir == 4) || (maininflow == 4 && deltadir == 8)) // Top left to bottom right or vice versa
        {
            if (random(1, 2) == 1)
            {
                junctionpointx = junctionpointx + random(minvardiag, maxvardiag);
                junctionpointy = junctionpointy - random(minvardiag, maxvardiag);
            }
            else
            {
                junctionpointx = junctionpointx - random(minvardiag, maxvardiag);
                junctionpointy = junctionpointy + random(minvardiag, maxvardiag);
            }
        }
        else
        {
            if (random(1, 2) == 1)
            {
                junctionpointx = junctionpointx + random(minvardiag, maxvardiag);
                junctionpointy = junctionpointy + random(minvardiag, maxvardiag);
            }
            else
            {
                junctionpointx = junctionpointx - random(minvardiag, maxvardiag);
                junctionpointy = junctionpointy - random(minvardiag, maxvardiag);
            }
        }
    }
    else
    {
        junctionpointx = junctionpointx + randomsign(random(0, maxvar));
        junctionpointy = junctionpointy + randomsign(random(0, maxvar));
    }

    fast_srand((sy * width + sx) + world.nom(sx, sy) + world.julrain(sx, sy));

    // By default, the river begins at the junction point (for carving purposes).
    // If there is inflow, this will be changed in a moment.

    int riverstartx = junctionpointx;
    int riverstarty = junctionpointy;
    int riverlength = 0;

    // Now we draw the first river, from the main inflow to the outflow.

    int janinflow = 0, julinflow = 0;

    switch (maininflow)
    {
    case 1:
        mm1.x = (float) northpointx;
        mm1.y = (float) northpointy;

        janinflow = world.deltajan(nox, noy);
        julinflow = world.deltajul(nox, noy);
        break;

    case 2:
        mm1.x = (float) northeastpointx;
        mm1.y = (float) northeastpointy;

        janinflow = world.deltajan(nex, ney);
        julinflow = world.deltajul(nex, ney);

        break;

    case 3:
        mm1.x = (float) eastpointx;
        mm1.y = (float) eastpointy;

        janinflow = world.deltajan(eax, eay);
        julinflow = world.deltajul(eax, eay);

        break;

    case 4:
        mm1.x = (float) southeastpointx;
        mm1.y = (float) southeastpointy;

        janinflow = world.deltajan(sex, sey);
        julinflow = world.deltajul(sex, sey);

        break;

    case 5:
        mm1.x = (float) southpointx;
        mm1.y = (float) southpointy;

        janinflow = world.deltajan(sox, soy);
        julinflow = world.deltajul(sox, soy);

        break;

    case 6:
        mm1.x = (float) southwestpointx;
        mm1.y = (float) southwestpointy;

        janinflow = world.deltajan(swx, swy);
        julinflow = world.deltajul(swx, swy);

        break;

    case 7:
        mm1.x = (float) westpointx;
        mm1.y = (float) westpointy;

        janinflow = world.deltajan(wex, wey);
        julinflow = world.deltajul(wex, wey);

        break;

    case 8:
        mm1.x = (float) northwestpointx;
        mm1.y = (float) northwestpointy;

        janinflow = world.deltajan(nwx, nwy);
        julinflow = world.deltajul(nwx, nwy);

        break;
    }

    // First we draw the first half, inflow to central junction.

    if (biggestflowin != 0) // Only do this if there actually is an inflow.
    {
        mm3.x = (float) junctionpointx;
        mm3.y = (float) junctionpointy;

        mm2.x = (float) (mm3.x + mm1.x) / 2;
        mm2.y = (float) (mm3.y + mm1.y) / 2;

        if (diag == 1)
        {
            mm2.x = (float) mm2.x + randomsign(random(minmidvardiag, maxmidvardiag));
            mm2.y = (float) mm2.y + randomsign(random(minmidvardiag, maxmidvardiag));
        }
        else
        {
            mm2.x = (float) mm2.x + randomsign(random(0, maxmidvar));
            mm2.y = (float) mm2.y + randomsign(random(0, maxmidvar));
        }

        if (mm2.x < dx + margin)
            mm2.x = (float) dx + margin;

        if (mm2.x > dx + mmargin)
            mm2.x = (float) dx + mmargin;

        if (mm2.y < dy + margin)
            mm2.y = (float) dy + margin;

        if (mm2.y > dy + mmargin)
            mm2.y = (float) dy + mmargin;

        riverstartx = (int)mm1.x;
        riverstarty = (int)mm1.y;

        riverlength = 1;

        rinfo = calculateregionalriver(world, region, dx, dy, sx, sy, pt, mm1, mm2, mm3, janinflow, julinflow, riverlength, 0, 0, 1);

        riverlength = rinfo.z;
    }
    else
    {
        janinflow = 0;
        julinflow = 0;
    }

    // Now we draw the second half, from the central junction to the outflow.

    mm1.x = (float) junctionpointx;
    mm1.y = (float) junctionpointy;

    if (north < 0)
    {
        mm3.x = (float) northpointx;
        mm3.y = (float) northpointy;
    }

    if (south < 0)
    {
        mm3.x = (float) southpointx;
        mm3.y = (float) southpointy;
    }

    if (east < 0)
    {
        mm3.x = (float) eastpointx;
        mm3.y = (float) eastpointy;
    }

    if (west < 0)
    {
        mm3.x = (float) westpointx;
        mm3.y = (float) westpointy;
    }

    if (northeast < 0)
    {
        mm3.x = (float) northeastpointx;
        mm3.y = (float) northeastpointy;
    }

    if (northwest < 0)
    {
        mm3.x = (float) northwestpointx;
        mm3.y = (float) northwestpointy;
    }

    if (southeast < 0)
    {
        mm3.x = (float) southeastpointx;
        mm3.y = (float) southeastpointy;
    }

    if (southwest < 0)
    {
        mm3.x = (float) southwestpointx;
        mm3.y = (float) southwestpointy;
    }

    mm2.x = (float) (mm3.x + mm1.x) / 2;
    mm2.y = (float) (mm3.y + mm1.y) / 2;

    if (diag == 1)
    {
        mm2.x = (float) mm2.x + randomsign(random(minmidvardiag, maxmidvardiag));
        mm2.y = (float) mm2.y + randomsign(random(minmidvardiag, maxmidvardiag));
    }
    else
    {
        mm2.x = (float) mm2.x + randomsign(random(0, maxmidvar));
        mm2.y = (float) mm2.y + randomsign(random(0, maxmidvar));
    }

    if (mm2.x < dx + margin)
        mm2.x = (float) dx + margin;

    if (mm2.x > dx + mmargin)
        mm2.x = (float) dx + mmargin;

    if (mm2.y < dy + margin)
        mm2.y = (float) dy + margin;

    if (mm2.y > dy + mmargin)
        mm2.y = (float) dy + mmargin;

    int riverendx = (int)mm3.x;
    int riverendy = (int)mm3.y;

    rinfo = calculateregionalriver(world, region, dx, dy, sx, sy, pt, mm1, mm2, mm3, janinflow, julinflow, riverlength, 0, 0, 1);

    riverlength = findriverlength(region, riverstartx, riverstarty, riverendx, riverendy, 1);

    // Now we need to calculate where other rivers will join this one.
    // Each one needs a junction. additionalrivers is the number we need.

    if (additionalrivers > 0)
    {
        twointegers junction;

        bool extrarivers[9]; // This will tell us which incoming rivers still need doing.

        for (int n = 0; n <= 8; n++)
            extrarivers[n] = 0;

        if (north > 0 && maininflow != 1)
            extrarivers[1] = 1;

        if (northeast > 0 && maininflow != 2)
            extrarivers[2] = 1;

        if (east > 0 && maininflow != 3)
            extrarivers[3] = 1;

        if (southeast > 0 && maininflow != 4)
            extrarivers[4] = 1;

        if (south > 0 && maininflow != 5)
            extrarivers[5] = 1;

        if (southwest > 0 && maininflow != 6)
            extrarivers[6] = 1;

        if (west > 0 && maininflow != 7)
            extrarivers[7] = 1;

        if (northwest > 0 && maininflow != 8)
            extrarivers[8] = 1;

        bool keepgoing = 0;

        do
        {
            keepgoing = 0;

            int rivernumber = 0;

            for (int nn = 1; nn <= 8; nn++)
            {
                if (extrarivers[nn] == 1)
                {
                    rivernumber = nn;
                    keepgoing = 1;
                }
            }

            if (keepgoing == 1)
            {
                // rivernumber is now the direction of the current inflowing one being done.

                extrarivers[rivernumber] = 0; // Clear in that array so we don't do it again.

                // Now we draw that river, from its point of origin to the junction.

                // First, find its origin point.

                int janinflow = 0, julinflow = 0;

                switch (rivernumber)
                {
                case 1:
                    mm1.x = (float) northpointx;
                    mm1.y = (float) northpointy;

                    janinflow = world.deltajan(nox, noy);
                    julinflow = world.deltajul(nox, noy);

                    break;

                case 2:
                    mm1.x = (float) northeastpointx;
                    mm1.y = (float) northeastpointy;

                    janinflow = world.deltajan(nex, ney);
                    julinflow = world.deltajul(nex, ney);

                    break;

                case 3:
                    mm1.x = (float) eastpointx;
                    mm1.y = (float) eastpointy;

                    janinflow = world.deltajan(eax, eay);
                    julinflow = world.deltajul(eax, eay);

                    break;

                case 4:
                    mm1.x = (float) southeastpointx;
                    mm1.y = (float) southeastpointy;

                    janinflow = world.deltajan(sex, sey);
                    julinflow = world.deltajul(sex, sey);

                    break;

                case 5:
                    mm1.x = (float) southpointx;
                    mm1.y = (float) southpointy;

                    janinflow = world.deltajan(sox, soy);
                    julinflow = world.deltajul(sox, soy);

                    break;

                case 6:
                    mm1.x = (float) southwestpointx;
                    mm1.y = (float) southwestpointy;

                    janinflow = world.deltajan(swx, swy);
                    julinflow = world.deltajul(swx, swy);

                    break;

                case 7:
                    mm1.x = (float) westpointx;
                    mm1.y = (float) westpointy;

                    janinflow = world.deltajan(wex, wey);
                    julinflow = world.deltajul(wex, wey);

                    break;

                case 8:
                    mm1.x = (float) northwestpointx;
                    mm1.y = (float) northwestpointy;

                    janinflow = world.deltajan(nwx, nwy);
                    julinflow = world.deltajul(nwx, nwy);

                    break;
                }

                // Now we find a random river point within this tile to be the junction (destination) of this river.

                mm3.x = (float) -1;
                mm3.y = (float) -1;

                int a, b, c, d, e, f;

                if (random(0, 1) == 1)
                {
                    a = dx + 5;
                    b = dx + 10;
                    c = 1;
                }
                else
                {
                    a = dx + 10;
                    b = dx + 5;
                    c = -1;
                }

                if (random(0, 1) == 1)
                {
                    d = dy + 5;
                    e = dy + 10;
                    f = 1;
                }
                else
                {
                    d = dy + 10;
                    e = dy + 5;
                    f = -1;
                }

                for (int i = a; i != b; i = i + c)
                {
                    for (int j = d; j != e; j = j + f)
                    {
                        if (region.deltajan(i, j) != 0 || region.deltajul(i, j) != 0)
                        {
                            mm3.x = (float) i;
                            mm3.y = (float) j;
                        }
                    }
                }

                if (mm3.x == -1) // If we didn't find a suitable point, try alternatives
                {
                    if (region.deltajan(junctionpointx, junctionpointy) != 0 || region.deltajul(junctionpointx, junctionpointy) != 0)
                    {
                        mm3.x = (float) junctionpointx;
                        mm3.y = (float) junctionpointy;
                    }
                    else
                    {
                        mm3.x = (float) riverstartx;
                        mm3.y = (float) riverstarty;
                    }
                }

                mm2.x = (float) (mm3.x + mm1.x) / 2;
                mm2.y = (float) (mm3.y + mm1.y) / 2;

                mm2.x = (float) mm2.x + randomsign(random(0, maxmidvar));
                mm2.y = (float) mm2.y + randomsign(random(0, maxmidvar));

                if (mm2.x > dx + 14)
                    mm2.x = (float) dx + 14;

                if (mm2.x < dx + 2)
                    mm2.x = (float) dx + 2;

                if (mm2.y > dy + 14)
                    mm2.y = (float) dy + 14;

                if (mm2.y < dy + 2)
                    mm2.y = (float) dy + 2;

                rinfo = calculateregionalriver(world, region, dx, dy, sx, sy, pt, mm1, mm2, mm3, janinflow, julinflow, 1, 1, 0, 1);
            }
        } while (keepgoing == 1);
    }
}

// This draws a section of river on a tile of the regional map.

threeintegers calculateregionalriver(planet& world, region& region, int dx, int dy, int sx, int sy, twofloats pt, twofloats mm1, twofloats mm2, twofloats mm3, int janinflow, int julinflow, int riverlength, int isittributary, int goingtolake, bool delta)
{
    threeintegers rinfo;

    int rwidth = region.rwidth();
    int rheight = region.rheight();

    int x = (int)mm1.x;
    int y = (int)mm1.y;

    if (goingtolake == 1 && region.truelake(x, y) == 1)
    {
        rinfo.x = x;
        rinfo.y = y;
        rinfo.z = riverlength;

        return rinfo;
    }

    int lastx = (int)mm1.x;
    int lasty = (int)mm1.y; // Coordinates of last point done.

    if (isittributary == 1) // For tributaries, check first to see if there's another river of the same size.
    {
        bool found = 0;

        do // Keep going until this river is unique in size.
        {
            found = 0;

            for (int i = dx; i <= dx + 16; i++)
            {
                for (int j = dy; j <= dy + 16; j++)
                {
                    if (region.waterjan(i, j, delta) == janinflow && region.waterjul(i, j, delta) == julinflow)
                    {
                        found = 1;

                        if (janinflow > julinflow)
                            janinflow++;
                        else
                            julinflow++;
                    }
                }
            }

        } while (found != 0);
    }

    region.setwaterjan(lastx, lasty, delta, janinflow);
    region.setwaterjul(lastx, lasty, delta, julinflow);

    bool finishing = 0;

    for (int n = 1; n <= 2; n++) // Two halves of the curve.
    {
        for (float t = 0.0f; t <= 1.10f; t = t + 0.01f)
        {
            bool goahead = 0;

            if (t <= 1.00f)
            {
                if (n == 1)
                    pt = curvepos(mm1, mm1, mm2, mm3, t);
                else
                    pt = curvepos(mm1, mm2, mm3, mm3, t);

                x = (int)pt.x;
                y = (int)pt.y;

                goahead = 1;
            }

            if (n == 2 && t > 1.00f && finishing == 0 && (lastx != mm3.x || lasty != mm3.y)) // If for some reason we're at the end of the curve but it hasn't reached the destination, add an extra one at the destination.
            {
                x = (int)mm3.x;
                y = (int)mm3.y;

                goahead = 1;
                finishing = 1;
            }

            if (goahead == 1 && x >= dx && x <= dx + 16 && y >= dy && y <= dy + 16)
            {
                bool newpoint = 0;

                if (x != lastx || y != lasty)
                    newpoint = 1;

                if (newpoint == 1) // If this is a new point
                {
                    bool cangoonedge = 0;

                    if (x == mm1.x && y == mm1.y && n == 1)
                        cangoonedge = 1;

                    if (x == mm3.x && y == mm3.y && n == 2)
                        cangoonedge = 1;

                    if (cangoonedge == 0) // If it's not the beginning or end, it shouldn't be on the edge of the tile.
                    {
                        if (x <= dx)
                            x = dx + 1;

                        if (x >= dx + 16)
                            x = dx + 15;

                        if (y <= dy)
                            y = dy + 1;

                        if (y >= dy + 16)
                            y = dy + 15;
                    }

                    // First, we need to mark on the last point the direction to the new point.

                    int dir = 0;

                    if (x == lastx && y < lasty)
                        dir = 1;

                    if (x > lastx && y < lasty)
                        dir = 2;

                    if (x > lastx && y == lasty)
                        dir = 3;

                    if (x > lastx && y > lasty)
                        dir = 4;

                    if (x == lastx && y > lasty)
                        dir = 5;

                    if (x<lastx && y>lasty)
                        dir = 6;

                    if (x < lastx && y == lasty)
                        dir = 7;

                    if (x < lastx && y < lasty)
                        dir = 8;

                    region.setwaterdir(lastx, lasty, delta, dir);

                    // Now we need to see whether we are joining an existing river.

                    bool newriver = 0;

                    if (isittributary == 1)
                    {
                        if (region.waterjan(x, y, delta) != janinflow || region.waterjul(x, y, delta) != julinflow)
                            newriver = 1;
                    }

                    if (region.waterdir(x, y, delta) != 0 && newriver == 1)
                    {
                        addtoexistingregionalriver(world, region, dx, dy, sx, sy, x, y, janinflow, julinflow, delta);

                        rinfo.x = x;
                        rinfo.y = y;
                        rinfo.z = riverlength;

                        return rinfo;
                    }

                    // If we're flowing into a lake, and this is a lake, we need to stop.

                    if (goingtolake == 1 && region.truelake(x, y) != 0)
                    {
                        rinfo.x = x;
                        rinfo.y = y;
                        rinfo.z = riverlength;

                        return rinfo;
                    }

                    // If there is no river here, we need to place the flow onto the map.

                    region.setwaterjan(x, y, delta, janinflow);
                    region.setwaterjul(x, y, delta, julinflow);

                    riverlength++;

                    // Now update the old x and y.

                    lastx = x;
                    lasty = y;

                    // Now if this is a tributary, do a check to see if there's another river next door. If there is, we will join it.

                    if (isittributary == 1)
                    {
                        for (int i = x - 1; i <= x + 1; i++)
                        {
                            for (int j = y - 1; j <= y + 1; j++)
                            {
                                if (i >= 0 && i <= rwidth && j >= 0 && j <= rheight)
                                {
                                    if (region.waterdir(i, j, delta) != 0 && region.waterjan(i, j, delta) != janinflow && region.waterjul(i, j, delta) != julinflow)
                                    {
                                        int dir = 0;

                                        if (i == x && j < y)
                                            dir = 1;

                                        if (i > x && j < y)
                                            dir = 2;

                                        if (i > x && j < y)
                                            dir = 3;

                                        if (i > x && j > y)
                                            dir = 4;

                                        if (i == x && j > y)
                                            dir = 5;

                                        if (i<x && j>y)
                                            dir = 6;

                                        if (i < x && j == y)
                                            dir = 7;

                                        if (i < x && j < y)
                                            dir = 8;

                                        region.setwaterdir(x, y, delta, dir);

                                        addtoexistingregionalriver(world, region, dx, dy, sx, sy, i, j, janinflow, julinflow, delta);

                                        rinfo.x = i;
                                        rinfo.y = j;
                                        rinfo.z = riverlength;

                                        return (rinfo);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    rinfo.x = x;
    rinfo.y = y;
    rinfo.z = riverlength;

    return rinfo;
}

// This checks for weird elevations. If it finds any it gives all rivers in the tile the basic tile elevation.

void removeweirdelevations(planet& world, region& region, int dx, int dy, int sx, int sy)
{
    int sealevel = world.sealevel();
    int maxelev = world.maxelevation();
    int safetyelev = world.nom(sx, sy) - world.riverlandreduce();

    if (safetyelev <= sealevel)
        safetyelev = sealevel + 1;

    bool found = 0;

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (region.map(i, j) < 0 || region.map(i, j) > maxelev)
            {
                found = 1;
                i = dx + 16;
                j = dy + 16;
            }
        }
    }

    if (found == 0) // Check for rivers that go up in elevation. We don't like those.
    {
        for (int i = dx; i <= dx + 16; i++)
        {
            for (int j = dy; j <= dy + 16; j++)
            {
                int dir = region.riverdir(i, j);

                if (dir != 0)
                {
                    int fromelev = region.map(i, j);

                    twointegers dest = getdestination(i, j, dir);

                    int toelev = region.map(dest.x, dest.y);

                    if (toelev > fromelev)
                    {
                        found = 1;
                        i = dx + 16;
                        j = dy + 16;
                    }
                }
            }
        }
    }

    if (found == 1)
    {
        for (int i = dx; i <= dx + 16; i++)
        {
            for (int j = dy; j <= dy + 16; j++)
            {
                if (region.riverdir(i, j) != 0)
                {
                    region.setmap(i, j, safetyelev);
                }
            }
        }
    }
}

// This function goes over the regional river tile and removes any odd bits where the flow goes suddenly low.

void removelowflow(region& region, int dx, int dy)
{
    for (int i = dx + 1; i <= dx + 15; i++)
    {
        for (int j = dy + 1; j <= dy + 15; j++)
        {
            if (region.riverjan(i, j) != 0 || region.riverjul(i, j) != 0)
            {
                int x = i;
                int y = j;

                int janflow = 0;
                int julflow = 0;

                bool keepgoing = 1;
                int crount = 0;

                do
                {
                    crount++;
                    if (crount > 100)
                        keepgoing = 0;

                    if (region.riverjan(x, y) > janflow)
                        janflow = region.riverjan(x, y);

                    if (region.riverjul(x, y) > julflow)
                        julflow = region.riverjul(x, y);

                    int dir = region.riverdir(x, y);

                    if (dir == 0)
                        keepgoing = 0;

                    if (dir == 8 || dir == 1 || dir == 2)
                        y--;

                    if (dir == 4 || dir == 5 || dir == 6)
                        y++;

                    if (dir == 2 || dir == 3 || dir == 4)
                        x++;

                    if (dir == 6 || dir == 7 || dir == 8)
                        x--;

                    if (x == dx || x == dx + 16 || y == dy || y == dy + 16)
                        keepgoing = 0;

                    if (region.riverjan(x, y) < janflow)
                        region.setriverjan(x, y, janflow);

                    if (region.riverjul(x, y) < julflow)
                        region.setriverjul(x, y, julflow);

                } while (keepgoing == 1);
            }
        }
    }
}

// This function adds the flow from one river to another that it's just joined on the regional map.

void addtoexistingregionalriver(planet& world, region& region, int dx, int dy, int sx, int sy, int x, int y, int janinflow, int julinflow, bool delta)
{
    int finaloutflow;

    if (delta == 0)
        finaloutflow = world.riverjan(sx, sy) + world.riverjul(sx, sy); // This is what should be flowing out of this tile.
    else
        finaloutflow = world.deltajan(sx, sy) + world.deltajul(sx, sy);

    while (1 == 1)
    {
        if (region.waterjan(x, y, delta) + region.waterjul(x, y, delta) + janinflow + julinflow <= finaloutflow)
        {
            region.setwaterjan(x, y, delta, region.waterjan(x, y, delta) + janinflow);
            region.setwaterjul(x, y, delta, region.waterjul(x, y, delta) + julinflow);
        }
        else
            return;

        int dir = region.waterdir(x, y, delta);

        if (dir == 8 || dir == 1 || dir == 2)
            y--;

        if (dir == 4 || dir == 5 || dir == 6)
            y++;

        if (dir == 2 || dir == 3 || dir == 4)
            x++;

        if (dir == 6 || dir == 7 || dir == 8)
            x--;

        if (x <= dx || x >= dx + 16 || y <= dy || y >= dy + 16)
            return;
    }
}

// This function lowers the land underneath a river to ensure that it flows downhill all the way.

void carveriver(planet& world, region& region, int dx, int dy, int sx, int sy, int riverlength, int riverstartx, int riverstarty, int riverendx, int riverendy, int startlandlevel, int endlandlevel, bool goingtolake, vector<vector<int>>& rriverscarved)
{
    int sealevel = world.sealevel();

    if (startlandlevel < endlandlevel)
        startlandlevel = endlandlevel;

    vector<vector<int>> carvedcheck(17, vector<int>(17, 0)); // This will ensure that we don't double back on ourselves.

    float totaldrop = (float)startlandlevel - (float)endlandlevel;
    float friverlength = (float)riverlength;
    float dropstep = (float)totaldrop / (float)(friverlength + 1); // Amount to lower the land each time.

    float dropcount = 0.0f; // If the dropstep < 1.0, add it to this each round and drop when it's > 1.0.

    int currentlandlevel = startlandlevel;
    int x = riverstartx;
    int y = riverstarty;

    int riftsurface = world.riftlakesurface(sx, sy);

    for (int n = 0; n <= 1000; n++)
    {
        int xx = x - dx;
        int yy = y - dy;

        if (xx < 0)
            xx = 0;

        if (yy < 0)
            y = 0;

        if (xx > 16)
            xx = 16;

        if (yy > 16)
            yy = 16;

        if (carvedcheck[xx][yy] == 1)
            return;

        if (rriverscarved[x][y] == -1)
        {
            if (currentlandlevel <= sealevel)
                currentlandlevel = sealevel + 1;

            if (world.deltadir(sx, sy) != 0)
                currentlandlevel = sealevel + 1;

            if (region.riverjan(x, y) != 0 || region.riverjul(x, y) != 0)
            {
                if (region.lakesurface(x, y) == 0 && riftsurface == 0)
                {
                    rriverscarved[x][y] = region.map(x, y);
                    region.setmap(x, y, currentlandlevel);

                    if (x - dx >= 0 && x - dx <= 16 && y - dy >= 0 && y - dy <= 16)
                        carvedcheck[x - dx][y - dy] = 1;
                }

                if (riftsurface != 0)
                    region.setmap(x, y, riftsurface);
            }
        }

        if (x == riverendx && y == riverendy)
            return;

        if (region.truelake(x, y) != 0 && goingtolake == 1)
            return;

        int dir = region.riverdir(x, y);

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

        if (x == riverendx && y == riverendy)
            currentlandlevel = endlandlevel;
        else
        {
            if (dropstep >= 1.0f)
            {
                float fcurrentlandlevel = (float)currentlandlevel;
                fcurrentlandlevel = fcurrentlandlevel - (float)dropstep;
                currentlandlevel = (int)fcurrentlandlevel;
            }
            else
            {
                dropcount = dropcount + dropstep;
                if (dropcount >= 1.0f)
                {
                    currentlandlevel--;
                    dropcount = 0;
                }
            }
        }
    }
}

// This does the same thing, but for a tributary.

void carverivertributary(planet& world, region& region, int dx, int dy, int sx, int sy, int thisriverlength, int thisriverstartx, int thisriverstarty, int thisriverendx, int thisriverendy, int thisstartlandlevel, int thisendlandlevel, int riverendx, int riverendy, int endlandlevel, vector<vector<int>>& rriverscarved)
{
    int sealevel = world.sealevel();

    twointegers endpoint;

    if (thisendlandlevel == 0) // If there's something wrong with the destination, recalculate it.
    {
        endpoint = gettributaryendpoint(region, thisriverstartx, thisriverstarty);

        if (endpoint.x == -1) // If that still didn't work, give up.
            return;

        thisriverendx = endpoint.x;
        thisriverendy = endpoint.y;

        thisendlandlevel = region.map(thisriverendx, thisriverendy);
    }

    float totaldrop = (float)thisstartlandlevel - (float)thisendlandlevel;

    if (totaldrop >= 0.0f) // If it's straightforwardly going downhill.
    {
        float fthisriverlength = (float)thisriverlength;
        float dropstep = totaldrop / (fthisriverlength + 1.0f); // Amount to lower the land each time.

        float dropcount = 0.0f; // If the dropstep < 1.0, add it to this each round and drop when it's > 1.0.

        int currentlandlevel = thisstartlandlevel;
        int x = thisriverstartx;
        int y = thisriverstarty;

        for (int n = 0; n < 1000; n++) // Just in case it gets stuck in a loop.
        {
            if (rriverscarved[x][y] == -1)
            {
                if (currentlandlevel <= sealevel)
                    currentlandlevel = sealevel + 1;

                if (world.deltadir(sx, sy) != 0)
                    currentlandlevel = sealevel + 1;

                if (region.riverjan(x, y) != 0 || region.riverjul(x, y) != 0)
                {
                    if (region.lakesurface(x, y) == 0)
                    {
                        rriverscarved[x][y] = region.map(x, y);
                        region.setmap(x, y, currentlandlevel);
                    }
                }
            }
            else
            {
                if (x != thisriverstartx || y != thisriverstarty)
                    return;
            }

            if (x == thisriverendx && y == thisriverendy)
                return;

            int dir = region.riverdir(x, y);

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

            if (x == thisriverendx && y == thisriverendy)
                currentlandlevel = thisendlandlevel;
            else
            {
                if (dropstep >= 1.0f)
                    currentlandlevel = currentlandlevel - (int)dropstep;
                else
                {
                    dropcount = dropcount + dropstep;
                    if (dropcount >= 1.0f)
                    {
                        currentlandlevel--;
                        dropcount = 0;
                    }
                }
            }
        }
    }

    // If it's not straightforwardly downhill, that means we have to redo part of the main river.

    // First we must find the length from our entry point to the exit point of the whole river.

    thisriverlength = findriverlength(region, thisriverstartx, thisriverstarty, thisriverendx, thisriverendy, 0);

    float fthisriverlength = (float)thisriverlength;

    if (thisriverlength == -1.0f)
        return;

    // Now we carve the river all the way to the exit point of the whole river.

    thisendlandlevel = endlandlevel;
    thisriverendx = riverendx;
    thisriverendy = riverendy;

    totaldrop = (float)(thisstartlandlevel - thisendlandlevel);

    if (totaldrop < 0.0f)
        return;

    if (thisendlandlevel == 0)
        return;

    fthisriverlength = (float)thisriverlength;
    float dropstep = totaldrop / (fthisriverlength + 1.0f); // Amount to lower the land each time.

    float dropcount = 0.0f; // If the dropstep < 1.0, add it to this each round and drop when it's > 1.0.

    int currentlandlevel = thisstartlandlevel;
    int x = thisriverstartx;
    int y = thisriverstarty;

    if (x<0 || y<0 || x>region.rwidth() || y>region.rheight())
        return;

    for (int n = 0; n <= 100; n++)
    {
        if (currentlandlevel <= sealevel) // && (region.riverjan(x,y)+region.riverjul(x,y))/2<estuarylimit)
            currentlandlevel = sealevel + 1;

        rriverscarved[x][y] = region.map(x, y);
        region.setmap(x, y, currentlandlevel);

        if (x == thisriverendx && y == thisriverendy)
            return;

        int dir = region.riverdir(x, y);

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

        if (x == thisriverendx && y == thisriverendy)
            currentlandlevel = thisendlandlevel;
    }
}

// This function finds the point at which a tributary enters another river.

twointegers gettributaryendpoint(region& region, int startx, int starty)
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    twointegers endpoint;

    endpoint.x = -1;
    endpoint.y = -1;

    int x = startx;
    int y = starty;

    for (int n = 1; n <= 100; n++)
    {
        if (region.map(x, y) != 0 && x != startx && y != starty)
        {
            endpoint.x = x;
            endpoint.y = y;
            return endpoint;
        }

        int dir = region.riverdir(x, y);

        if (dir == 8 || dir == 1 || dir == 2)
            y--;

        if (dir == 4 || dir == 5 || dir == 6)
            y++;

        if (dir == 2 || dir == 3 || dir == 4)
            x++;

        if (dir == 6 || dir == 7 || dir == 8)
            x--;

        if (x<0 || x>rwidth || y<0 || y>rheight)
            return endpoint;
    }
    return endpoint;
}

// This function adds springs to a tile.

void addsprings(planet& world, region& region, int sx, int sy, int dx, int dy, int junctionpointx, int junctionpointy, int riverendx, int riverendy, int maxmidvar, vector<vector<int>>& rriverscarved)
{
    float janload = (float)world.janrain(sx, sy);
    float julload = (float)world.julrain(sx, sy);

    if (janload == 0.0f && julload == 0.0f)
        return;

    int height = world.height();
    int riverlandreduce = world.riverlandreduce();

    float riverfactor = world.riverfactor();

    twofloats pt, mm1, mm2, mm3;
    threeintegers rinfo;

    int endlandlevel = region.map(riverendx, riverendy);

    int springtotal = random(1, 6); // Number of springs for this tile.

    // First we need to find out how much water to add.

    janload = janload / riverfactor;
    julload = julload / riverfactor;

    // This bit moves some water from the winter load to the summer. This is because winter precipitation feeds into the river more slowly than summer precipitation does.

    float amount;

    if (world.mintemp(sx, sy) < 0) // In cold areas, more winter water gets held over until summer because it's snow.
        amount = 3.0f;
    else
        amount = 10.0f;

    if (sy < height / 2) // Northern hemisphere
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

    int jantotal = (int)janload;
    int jultotal = (int)julload;

    int janinitialload = jantotal / springtotal - springtotal;
    int julinitialload = jultotal / springtotal - springtotal;

    twointegers destpoint;

    int springstartlandlevel = world.nom(sx, sy) - riverlandreduce;

    for (int springno = 1; springno <= springtotal; springno++) // Go through them one by one.
    {
        // First we need to find a starting point for the spring. It shouldn't be next to an existing river.

        int timeslooped = 0;
        int nearbyrivers;
        int springstartx = -1;
        int springstarty = -1;

        do
        {
            if (timeslooped > 200)
                return;

            nearbyrivers = 0;

            springstartx = random(dx + 2, dx + 14);
            springstarty = random(dy + 2, dy + 14);

            for (int i = springstartx - 1; i <= springstartx + 1; i++)
            {
                for (int j = springstarty - 1; j <= springstarty + 1; j++)
                {
                    if (region.riverdir(i, j) != 0)
                        nearbyrivers = 1;
                }
            }

            timeslooped++;

        } while (nearbyrivers != 0);

        mm1.x = (float) springstartx;
        mm1.y = (float) springstarty;

        // Now we need to find an ending point for it. Look for the closest river that's lower than this spring's starting point.

        destpoint = nearestlowerriver(region, dx, dy, springstartx, springstarty, springstartlandlevel);

        int springendx = destpoint.x;
        int springendy = destpoint.y;

        if (springendx == -1) // If for some reason this didn't work, use an alternative.
        {
            if (region.riverdir(junctionpointx, junctionpointy) != 0)
            {
                springendx = junctionpointx;
                springendy = junctionpointy;
            }
            else
            {
                springendx = riverendx;
                springendy = riverendy;
            }
        }

        mm3.x = (float) springendx;
        mm3.y = (float) springendy;

        // Now the midpoint.

        mm2.x = (float) (mm3.x + mm1.x) / 2;
        mm2.y = (float) (mm3.y + mm1.y) / 2;

        mm2.x = (float) mm2.x + randomsign(random(0, maxmidvar));
        mm2.y = (float) mm2.y + randomsign(random(0, maxmidvar));

        if (mm2.x > dx + 14)
            mm2.x = (float) dx + 14;

        if (mm2.x < dx + 2)
            mm2.x = (float) dx + 2;

        if (mm2.y > dy + 14)
            mm2.y = (float) dy + 14;

        if (mm2.y < dy + 2)
            mm2.y = (float) dy + 2;

        // Now work out its flow.

        if (springno < springtotal)
        {
            janload = (float)(janinitialload + springno);
            julload = (float)(julinitialload + springno);
        }
        else
        {
            int biggestjanriver = 0;
            int biggestjulriver = 0; // This is the current total outflow.

            for (int i = dx + 1; i <= dx + 15; i++)
            {
                for (int j = dy + 1; j <= dy + 15; j++)
                {
                    if (region.riverjan(i, j) > biggestjanriver)
                        biggestjanriver = region.riverjan(i, j);

                    if (region.riverjul(i, j) > biggestjulriver)
                        biggestjulriver = region.riverjul(i, j);
                }
            }
            janload = (float)(world.riverjan(sx, sy) - biggestjanriver); // Just make up the shortfall in this last spring.
            julload = (float)(world.riverjul(sx, sy) - biggestjulriver);
        }

        if (janload < 0.0f)
            janload = 0.0f;

        if (julload < 0.0f)
            julload = 0.0f;

        if (janload == 0.0f && julload == 0.0f)
            return;

        // Now make the spring.

        rinfo = calculateregionalriver(world, region, dx, dy, sx, sy, pt, mm1, mm2, mm3, (int)janload, (int)julload, 1, 1, 0, 0);

        int springriverlength = rinfo.x;
        int springriverendx = rinfo.x;
        int springriverendy = rinfo.y;

        int springendlandlevel = region.map(springriverendx, springriverendy);

        if (springendlandlevel != 0)
            carverivertributary(world, region, dx, dy, sx, sy, springriverlength, springstartx, springstarty, springendx, springendy, springstartlandlevel, springendlandlevel, riverendx, riverendy, endlandlevel, rriverscarved);
        else // Sometimes it thinks it's drilling down to a level of 0. I don't know why. If this happens then just delete the spring in question.
        {
            for (int i = dx; i <= dx + 16; i++)
            {
                for (int j = dy; j <= dy + 16; j++)
                {
                    if (region.riverjan(i, j) == janload && region.riverjul(i, j) == julload)
                    {
                        region.setriverdir(i, j, 0);
                        region.setriverjan(i, j, 0);
                        region.setriverjul(i, j, 0);
                    }
                }
            }
        }
    }
}

// This finds the coordinates (on the regional map) of the nearest river point (within the current tile) that's lower than the point specified.

twointegers nearestlowerriver(region& region, int dx, int dy, int i, int j, int landlevel)
{
    twointegers riverpoint;

    int distance = -1; // Distance to nearest lower river.
    int riverx = -1; // x coordinate of nearest lower river.
    int rivery = -1; // y coordinate of nearest lower river.
    int checkdist = 1; // Distance that we're checking to see if it contains a lower river.

    while (distance == -1)
    {
        checkdist++; // Increase the size of circle that we're checking
        int rr = checkdist * checkdist;

        for (int x = i - checkdist; x <= i + checkdist; x++)
        {
            for (int y = j - checkdist; y <= j + checkdist; y++)
            {
                if (x >= dx + 1 && x <= dx + 15 && y >= dy + 1 && y <= dy + 15)
                {
                    if ((x - i) * (x - i) + (y - j) * (y - j) == rr) // If this point is on the circle we're checking
                    {
                        if (region.riverdir(x, y) != 0 && region.map(x, y) < landlevel) // If this is a river and it's lower than our starting point
                        {
                            distance = checkdist;
                            riverx = x;
                            rivery = y;
                        }
                    }
                }
            }
        }

        if (checkdist > 15) // We are surely not going to find any now...
        {
            distance = 100;
            riverx = -1;
            rivery = -1;
        }
    }

    riverpoint.x = riverx;
    riverpoint.y = rivery;

    return riverpoint;
}

// This function determines whether the river cell specified has any inflows other than the other one specified.

bool riverdircheck(region& region, int x, int y, int x2, int y2, int delta)
{
    bool check = 0;

    if (region.waterdir(x, y - 1, delta) == 5)
    {
        if (x != x2 || y - 1 != y2)
            check = 1;
    }

    if (region.waterdir(x + 1, y - 1, delta) == 6)
    {
        if (x + 1 != x2 || y - 1 != y2)
            check = 1;
    }

    if (region.waterdir(x + 1, y, delta) == 7)
    {
        if (x + 1 != x2 || y != y2)
            check = 1;
    }

    if (region.waterdir(x + 1, y + 1, delta) == 8)
    {
        if (x + 1 != x2 || y + 1 != y2)
            check = 1;
    }

    if (region.waterdir(x, y + 1, delta) == 1)
    {
        if (x != x2 || y + 1 != y2)
            check = 1;
    }

    if (region.waterdir(x - 1, y + 1, delta) == 2)
    {
        if (x - 1 != x2 || y + 1 != y2)
            check = 1;
    }

    if (region.waterdir(x - 1, y, delta) == 3)
    {
        if (x - 1 != x2 || y != y2)
            check = 1;
    }

    if (region.waterdir(x - 1, y - 1, delta) == 4)
    {
        if (x - 1 != x2 || y - 1 != y2)
            check = 1;
    }

    return (check);
}

// This function goes through a river tile and removes any orphans (odd tiles that flow into rivers out of nowhere).

int removeriverorphans(planet& world, region& region, int sx, int sy, int dx, int dy, int riverlength)
{
    if (checkwaterinflow(world, sx, sy) == 1) // Don't do this check if there really is a river actually starting in this tile.
    {
        bool found = 0;

        do
        {
            found = 0;

            for (int i = dx + 1; i <= dx + 15; i++)
            {
                for (int j = dy + 1; j <= dy + 15; j++)
                {
                    if (region.riverjan(i, j) != 0 || region.riverjul(i, j) != 0)
                    {
                        if (checkregionalwaterinflow(region, i, j) == 0)
                        {
                            region.setriverdir(i, j, 0);
                            region.setriverjan(i, j, 0);
                            region.setriverjul(i, j, 0);

                            riverlength--;
                            found = 1;
                        }
                    }
                }
            }

        } while (found == 1);
    }
    return riverlength;
}

// This function goes through a river tile and removes any widows (odd tiles where rivers just stop flowing).

void removeriverwidows(planet& world, region& region, int sx, int sy, int dx, int dy, int junctionpointx, int junctionpointy)
{
    twofloats pt, mm1, mm2, mm3;

    for (int i = dx + 1; i <= dx + 15; i++)
    {
        for (int j = dy + 1; j <= dy + 15; j++)
        {
            int dir = region.riverdir(i, j);

            int janinflow = region.riverjan(i, j);
            int julinflow = region.riverjul(i, j);

            if (dir != 0 || janinflow != 0 || julinflow != 0)
            {
                int newi = i;
                int newj = j;

                if (dir == 8 || dir == 1 || dir == 2)
                    newj = j - 1;

                if (dir == 4 || dir == 5 || dir == 6)
                    newj = j + 1;

                if (dir == 2 || dir == 3 || dir == 4)
                    newi = i + 1;

                if (dir == 6 || dir == 7 || dir == 8)
                    newi = i - 1;

                if (region.riverjan(newi, newj) == 0 && region.riverjul(newi, newj) == 0) // If the river disappears!
                {
                    // Create a new river from this point to the junction point.

                    mm1.x = (float) newi;
                    mm1.y = (float) newj;

                    mm3.x = (float) junctionpointx;
                    mm3.y = (float) junctionpointy;

                    mm2.x = (float) (mm1.x + mm3.x) / 2;
                    mm2.y = (float) (mm1.y + mm3.y) / 2;

                    calculateregionalriver(world, region, dx, dy, sx, sy, pt, mm1, mm2, mm3, janinflow, julinflow, 1, 1, 0, 0);
                }
            }
        }
    }
}

// This function removes over-long straight sections of rivers on the regional map.

void removeregionalriverstraights(region& region, int dx, int dy, bool delta, vector<vector<int>>& rriverscarved)
{
    int margin = 2;
    int mmargin = 16 - margin;

    int adjustchance = 3; // Probability of adjusting these lines. The higher it is, the less likely.

    for (int i = dx + margin; i <= dx + mmargin; i++)
    {
        for (int j = dy + margin; j <= dy + mmargin; j++)
        {
            if (random(1, adjustchance) == 1)
            {
                // Lines going south

                if (region.riverdir(i, j) == 5 && region.riverdir(i, j + 1) == 5)
                {
                    if (riverdircheck(region, i, j + 1, i, j, delta) == 0)
                    {
                        int adj = randomsign(1);
                        int newdir;

                        if (adj == -1)
                            newdir = 4;
                        else
                            newdir = 6;

                        if (region.riverdir(i + adj, j + 1) == 0)
                        {
                            region.setriverdir(i + adj, j + 1, newdir);
                            region.setriverjan(i + adj, j + 1, region.riverjan(i, j + 1));
                            region.setriverjul(i + adj, j + 1, region.riverjul(i, j + 1));

                            if (rriverscarved[i][j + 1] != -1)
                                rriverscarved[i + adj][j + 1] = region.map(i + adj, j + 1);

                            region.setmap(i + adj, j + 1, region.map(i, j + 1));

                            if (rriverscarved[i][j + 1] != -1)
                                region.setmap(i, j + 1, rriverscarved[i][j + 1]);

                            rriverscarved[i][j + 1] = -1;

                            region.setriverdir(i, j + 1, 0);
                            region.setriverjan(i, j + 1, 0);
                            region.setriverjul(i, j + 1, 0);

                            if (adj == -1)
                                newdir = 6;
                            else
                                newdir = 4;

                            region.setriverdir(i, j, newdir);
                        }
                    }
                }

                // Lines going north

                if (region.riverdir(i, j) == 1 && region.riverdir(i, j - 1) == 1)
                {
                    if (riverdircheck(region, i, j - 1, i, j, delta) == 0)
                    {
                        int adj = randomsign(1);
                        int newdir;

                        if (adj == -1)
                            newdir = 2;
                        else
                            newdir = 8;

                        if (region.riverdir(i + adj, j - 1) == 0)
                        {
                            region.setriverdir(i + adj, j - 1, newdir);
                            region.setriverjan(i + adj, j - 1, region.riverjan(i, j - 1));
                            region.setriverjul(i + adj, j - 1, region.riverjul(i, j - 1));

                            if (rriverscarved[i][j - 1] != -1)
                                rriverscarved[i + adj][j - 1] = region.map(i + adj, j - 1);

                            region.setmap(i + adj, j - 1, region.map(i, j - 1));

                            if (rriverscarved[i][j - 1] != -1)
                                region.setmap(i, j - 1, rriverscarved[i][j - 1]);

                            rriverscarved[i][j - 1] = -1;

                            region.setriverdir(i, j - 1, 0);
                            region.setriverjan(i, j - 1, 0);
                            region.setriverjul(i, j - 1, 0);

                            if (adj == -1)
                                newdir = 8;
                            else
                                newdir = 2;

                            region.setriverdir(i, j, newdir);
                        }
                    }
                }

                // Lines going east

                if (region.riverdir(i, j) == 3 && region.riverdir(i + 1, j) == 3)
                {
                    if (riverdircheck(region, i + 1, j, i, j, delta) == 0)
                    {
                        int adj = randomsign(1);
                        int newdir;

                        if (adj == -1)
                            newdir = 4;
                        else
                            newdir = 2;

                        if (region.riverdir(i + 1, j + adj) == 0)
                        {
                            region.setriverdir(i + 1, j + adj, newdir);
                            region.setriverjan(i + 1, j + adj, region.riverjan(i + 1, j));
                            region.setriverjul(i + 1, j + adj, region.riverjul(i + 1, j));

                            if (rriverscarved[i + 1][j] != -1)
                                rriverscarved[i + 1][j + adj] = region.map(i + 1, j + adj);

                            region.setmap(i + 1, j + adj, region.map(i + 1, j));

                            if (rriverscarved[i + 1][j] != -1)
                                region.setmap(i + 1, j, rriverscarved[i + 1][j]);

                            rriverscarved[i + 1][j] = -1;

                            region.setriverdir(i + 1, j, 0);
                            region.setriverjan(i + 1, j, 0);
                            region.setriverjul(i + 1, j, 0);

                            if (adj == -1)
                                newdir = 2;
                            else
                                newdir = 4;

                            region.setriverdir(i, j, newdir);
                        }
                    }
                }

                // Lines going west

                if (region.riverdir(i, j) == 7 && region.riverdir(i - 1, j) == 7)
                {
                    if (riverdircheck(region, i - 1, j, i, j, delta) == 0)
                    {
                        int adj = randomsign(1);
                        int newdir;

                        if (adj == -1)
                            newdir = 6;
                        else
                            newdir = 8;

                        if (region.riverdir(i - 1, j + adj) == 0)
                        {
                            region.setriverdir(i - 1, j + adj, newdir);
                            region.setriverjan(i - 1, j + adj, region.riverjan(i - 1, j));
                            region.setriverjul(i - 1, j + adj, region.riverjul(i - 1, j));

                            if (rriverscarved[i - 1][j] != -1)
                                rriverscarved[i - 1][j + adj] = region.map(i - 1, j + adj);

                            region.setmap(i - 1, j + adj, region.map(i - 1, j));

                            if (rriverscarved[i - 1][j] != -1)
                                region.setmap(i - 1, j, rriverscarved[i - 1][j]);

                            rriverscarved[i - 1][j] = -1;

                            region.setriverdir(i - 1, j, 0);
                            region.setriverjan(i - 1, j, 0);
                            region.setriverjul(i - 1, j, 0);

                            if (adj == -1)
                                newdir = 8;
                            else
                                newdir = 6;

                            region.setriverdir(i, j, newdir);
                        }
                    }
                }
            }
        }
    }
}

// This function finds the length of a stretch of regional river between two points.

int findriverlength(region& region, int startx, int starty, int endx, int endy, bool delta)
{
    int riverlength = 0;
    int x = startx;
    int y = starty;

    int timesrepeated = 0;

    while (1 == 1)
    {
        timesrepeated++;

        if (timesrepeated > 200)
            return -1;

        riverlength++;

        if (x == endx && y == endy)
            return riverlength;

        int dir = region.waterdir(x, y, delta);

        if (dir == 0)
            return riverlength;

        if (dir == 8 || dir == 1 || dir == 2)
            y--;

        if (dir == 4 || dir == 5 || dir == 6)
            y++;

        if (dir == 2 || dir == 3 || dir == 4)
            x++;

        if (dir == 6 || dir == 7 || dir == 8)
            x--;
    }

    return riverlength;
}

// This function checks to see if there are any gaps in the rivers, and joins them up.

void joinuprivers(region& region, int dx, int dy, int sx, int sy)
{
    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (region.sea(i, j != 0))
            {
                int dir = region.riverdir(i, j);

                if (dir != 0)
                {
                    int x = i;
                    int y = j;

                    if (dir == 8 || dir == 1 || dir == 2)
                        y--;

                    if (dir == 4 || dir == 5 || dir == 6)
                        y++;

                    if (dir == 1 || dir == 2 || dir == 3)
                        x++;

                    if (dir == 6 || dir == 7 || dir == 8)
                        x--;

                    if (region.riverdir(x, y) == 0 && region.sea(x, y) == 0) // If there's no river where there ought to be river
                    {
                        region.setriverdir(x, y, dir);
                        region.setriverjan(x, y, region.riverjan(i, j));
                        region.setriverjul(x, y, region.riverjul(i, j));

                        region.setmap(x, y, region.map(i, j));
                    }
                }
            }
        }
    }
}

// This function removes any negative flow values from a river tile.

void removenegativeflow(region& region, int dx, int dy)
{
    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            bool done = 0;

            if (region.riverjan(i, j) < 0)
            {
                region.setriverjan(i, j, 0);
                done = 1;
            }

            if (region.riverjul(i, j) < 0)
            {
                region.setriverjul(i, j, 0);
                done = 1;
            }

            if (done == 1 && region.riverjan(i, j) == 0 && region.riverjul(i, j) == 0)
                region.setriverdir(i, j, 0);
        }
    }
}

// This function creates a small lake within a given tile of the regional map.

void createsmalllake(planet& world, region& region, int dx, int dy, int sx, int sy, int centrex, int centrey, vector<vector<int>>& mainriver, boolshapetemplate smalllake[])
{
    int width = world.width();
    int sealevel = world.sealevel();
    int maxelev = world.maxelevation();
    int size;
    int surfaceheight = region.map(centrex, centrey);

    int diffx = centrex - dx;
    if (diffx > 8)
        diffx = 16 - diffx;

    int diffy = centrey - dy;
    if (diffy > 8)
        diffy = 16 - dy;

    if (diffx < diffy)
        size = diffx;
    else
        size = diffy;

    size = (size * 2) - 3;

    if (size < 0)
        size = 0;

    if (size > 11)
        size = 11;

    // size is now the maximum width/height of this template, to avoid going over the borders of the tile.

    int shapenumber = random(0, size);

    vector<vector<int>> thislake(17, vector<int>(17, 0));

    int imheight = smalllake[shapenumber].ysize() - 1;
    int imwidth = smalllake[shapenumber].ysize() - 1;

    int x = centrex - imwidth / 2; // Coordinates of the top left corner.
    int y = centrey - imheight / 2;

    if (x<0 || x>width)
        x = wrap(x, width);

    bool leftr = random(0, 1); // If it's 1 then we reverse it left-right
    bool downr = random(0, 1); // If it's 1 then we reverse it top-bottom

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
        jstart = imheight;
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

                bool goodtogo = 1;

                if (xx - dx < 0 || xx - dx>16 || yy - dy < 0 || yy - dy>16)
                    goodtogo = 0;
                else
                {
                    if (mainriver[xx - dx][yy - dy] == 0 && region.riverdir(xx, yy) != 0 && region.map(xx, yy) < surfaceheight) // Don't spread the lake over existing tributaries that are lower than it.
                        goodtogo = 0;
                }

                if (goodtogo == 1)
                {
                    region.setlakesurface(xx, yy, 1); // Put the lake on this bit of the lake map.
                    thislake[xx - dx][yy - dy] = 1; // Mark it on the keeping-track array too.
                }
            }
        }
    }

    // Now we must remove any lake that borders a river that's not going into the lake.

    twointegers destpoint;

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (region.riverdir(i, j) != 0)
            {
                if (region.lakesurface(i, j) == 0)
                {
                    destpoint = getregionalflowdestination(region, i, j, 0);

                    if (region.lakesurface(destpoint.x, destpoint.y) == 0)
                    {
                        for (int k = i - 1; k <= i + 1; k++)
                        {
                            for (int l = j - 1; l <= j + 1; l++)
                            {
                                if (k >= dx && k <= dx + 16 && l >= dy && l <= dy + 16)
                                {
                                    if (region.lakesurface(k, l) == 1 && region.riverdir(k, l) == 0)
                                    {
                                        region.setlakesurface(k, l, 0);
                                        thislake[k - dx][l - dy] = 0;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Now get the right surface height for the lake.

    surfaceheight = maxelev;

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (region.riverdir(i, j) != 0 && region.lakesurface(i, j) == 1 && region.map(i, j) < surfaceheight && region.map(i, j) > 0)
                surfaceheight = region.map(i, j);
        }
    }

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (region.lakesurface(i, j) == 1)
            {
                if (surfaceheight > sealevel + 4)
                    region.setlakesurface(i, j, surfaceheight);
                else
                    region.setlakesurface(i, j, 0);
            }
        }
    }

    if (surfaceheight == maxelev) // Something went terribly wrong.
    {
        for (int i = dx; i <= dx + 16; i++)
        {
            for (int j = dy; j <= dy + 16; j++)
                region.setlakesurface(i, j, 0);
        }
        return;
    }

    // Now we must alter the terrain height underneath the lake.

    int maxdepth = imwidth * 2; // Maximum depth.
    int mindepth = 4; // Minimum depth.

    int depth = random(mindepth, maxdepth);

    for (int i = 0; i <= 16; i++)
    {
        for (int j = 0; j <= 16; j++)
        {
            if (thislake[i][j] == 1)
            {
                int thisdepth = depth + randomsign(random(0, 2));

                if (thisdepth < mindepth)
                    thisdepth = mindepth;

                region.setmap(dx + i, dy + j, surfaceheight - thisdepth);
            }
        }
    }
}

// This does the same thing, but a salt lake.

void createsmallsaltlake(planet& world, region& region, int dx, int dy, int sx, int sy, int centrex, int centrey, int surfacelevel, vector<vector<bool>>& safesaltlakes, boolshapetemplate smalllake[])
{
    int width = world.width();
    int rwidth = region.rwidth();
    int rheight = region.rheight();
    int size;
    int surfaceheight = region.map(centrex, centrey);

    int diffx = centrex - dx;
    if (diffx > 8)
        diffx = 16 - diffx;

    int diffy = centrey - dy;
    if (diffy > 8)
        diffy = 16 - dy;

    if (diffx < diffy)
        size = diffx;
    else
        size = diffy;

    size = (size * 2) - 3;

    if (size < 0)
        size = 0;

    if (size > 11)
        size = 11;

    // size is now the maximum width/height of this template, to avoid going over the borders of the tile.

    int shapenumber = random(0, size);

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
        jstart = imheight;
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

                if (xx >= 0 && xx <= rwidth && yy >= 0 && yy <= rheight)
                {
                    region.setlakesurface(xx, yy, surfaceheight); // Put the lake on this bit of the lake map.
                    region.setspecial(xx, yy, 100); // Mark it as a salt lake.

                    safesaltlakes[xx][yy] = 1;
                }
            }
        }
    }

    // Now we must alter the terrain height underneath the lake.

    int maxdepth = imwidth; // Maximum depth.
    int mindepth = 1; // Minimum depth.

    if (maxdepth <= 1)
        maxdepth = 1;

    int depth = random(mindepth, maxdepth);

    for (int i = 0; i <= 16; i++)
    {
        for (int j = 0; j <= 16; j++)
        {
            if (region.special(i, j) == 100)
            {
                int thisdepth = depth + randomsign(random(0, 2));

                if (thisdepth < mindepth)
                    thisdepth = mindepth;

                region.setmap(dx + i, dy + j, surfaceheight - thisdepth);
            }
        }
    }
}

// This does the same thing, but a glacial-style lake.

void createsmallglaciallake(planet& world, region& region, int dx, int dy, int sx, int sy, int centrex, int centrey, vector<vector<int>>& mainriver)
{
    int sealevel = world.sealevel();
    int maxelev = world.maxelevation();
    int surfaceheight = region.map(centrex, centrey);

    vector<vector<int>> thislake(17, vector<int>(17, 0));

    /*
    for (int i=0; i<=16; i++)
    {
        for (int j=0; j<=16; j++)
            thislake[i][j]=0;
    }
    */

    int llimit = dx + random(1, 3);
    int rlimit = dx + random(12, 15);
    int ulimit = dy + random(1, 3);
    int dlimit = dy + random(12, 15);

    for (int i = llimit; i <= rlimit; i++)
    {
        for (int j = ulimit; j <= dlimit; j++)
        {
            if (region.riverdir(i, j) != 0)
            {
                if (random(0, 1) == 1)
                    thislake[i - dx][j - dy] = 1;
                else
                {
                    for (int k = i; k <= i + 1; k++)
                    {
                        for (int l = j; l <= j + 1; l++)
                        {
                            if (k >= llimit && k <= rlimit && l >= ulimit && l <= dlimit)
                                thislake[k - dx][l - dy] = 1;
                        }
                    }
                }
            }
        }
    }

    // Now get the right surface height for the lake.

    surfaceheight = maxelev;

    for (int i = llimit; i <= rlimit; i++)
    {
        for (int j = ulimit; j <= dlimit; j++)
        {
            if (region.riverdir(i, j) != 0 && thislake[i - dx][j - dy] == 1 && region.map(i, j) < surfaceheight && region.map(i, j) > 0)
                surfaceheight = region.map(i, j);
        }
    }

    for (int i = llimit; i <= rlimit; i++)
    {
        for (int j = ulimit; j <= dlimit; j++)
        {
            if (thislake[i - dx][j - dy] == 1)
            {
                if (surfaceheight > sealevel + 4)
                    region.setlakesurface(i, j, surfaceheight);
                else
                    region.setlakesurface(i, j, 0);
            }
        }
    }

    if (surfaceheight == maxelev) // Something went terribly wrong.
    {
        for (int i = dx; i <= dx + 16; i++)
        {
            for (int j = dy; j <= dy + 16; j++)
                region.setlakesurface(i, j, 0);
        }
        return;
    }

    // Now we must alter the terrain height underneath the lake.

    int maxdepth = 300; // Maximum depth.
    int mindepth = 30; // Minimum depth.

    int depth = random(mindepth, maxdepth);

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (region.lakesurface(i, j) != 0)
                region.setmap(i, j, surfaceheight - (depth + randomsign(random(0, 10))));
        }
    }
}

// This function goes through the tile and expands the rivers to their correct widths.

void expandrivers(planet& world, region& region, int dx, int dy, int sx, int sy, bool delta, vector<vector<int>>& fakesourcex, vector<vector<int>>& fakesourcey)
{
    float pixelmetres = (float)region.pixelmetres();
    float gravity = world.gravity();

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (region.waterdir(i, j, delta) != 0 && region.lakesurface(i, j) == 0)
            {
                float wide = (float)getriverwidth(region, i, j, delta, 0);

                float valleyfactor = 15;

                if (gravity < 1.0f)  // Lower gravity means wider (but shallower) river valleys.
                {
                    float diff = 1.0f - gravity;
                    diff = diff * 10;
                    valleyfactor = valleyfactor + diff;
                }

                if (gravity > 1.0f) // Higher gravity means narrower (but deeper) river valleys.
                {
                    float diff = gravity-1.0f;
                    diff = diff * 7;
                    valleyfactor = valleyfactor - diff;

                    if (valleyfactor < 1.0f)
                        valleyfactor = 1.0f;
                }

                float riverpixels = wide / pixelmetres + 1.0f;
                float valleypixels = (wide * valleyfactor) / pixelmetres + 1.0f;

                int dir = region.waterdir(i, j, delta);
                int janload = region.waterjan(i, j, delta);
                int julload = region.waterjul(i, j, delta);

                if (riverpixels > 30.0f)
                    riverpixels = 30.0f;

                if (valleypixels > 40.0f)
                    valleypixels = 40.0f;

                if (riverpixels > 1.0f)
                    pasterivercircle(region, i, j, (int)riverpixels, 1, dir, janload, julload, delta, fakesourcex, fakesourcey);

                if (world.sea(sx, sy) == 0 && world.outline(sx, sy) == 0)
                {
                    if (valleypixels > riverpixels)
                        pasterivercircle(region, i, j, (int)valleypixels, 0, dir, janload, julload, delta, fakesourcex, fakesourcey);
                }
            }
        }
    }
}

// This function works out the width (in m) of a river at the given point on the regional map.

int getriverwidth(region& region, int x, int y, bool delta, int season)
{
    int wide = 0;

    switch (season)
    {
    case 0: // Average over the year
        wide = (region.waterjan(x, y, delta) + region.waterjul(x, y, delta)) / 2;
        break;

    case 1: // January flow
        wide = region.waterjan(x, y, delta);
        break;

    case 2: // July flow
        wide = region.waterjul(x, y, delta);
        break;

    case 3: // Whichever is widest
        wide = region.waterjan(x, y, delta);
        if (region.waterjul(x, y, delta) > wide)
            wide = region.waterjul(x, y, delta);
    }

    return wide;
}

// This function pastes a river circle onto the regional map.

void pasterivercircle(region& region, int centrex, int centrey, int pixels, bool river, int dir, int janload, int julload, bool delta, vector<vector<int>>& fakesourcex, vector<vector<int>>& fakesourcey)
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    twointegers nearest;

    int x = centrex - pixels / 2 + 1;
    int y = centrey - pixels / 2 + 1; // Coordinates of the top left corner.

    x = x + pixels / 5;
    y = y + pixels / 5;

    for (int i = x; i <= x + pixels / 2; i++)
    {
        for (int j = y; j <= y + pixels / 2; j++)
        {
            if (i >= 0 && i <= rwidth && j >= 0 && j <= rheight)
            {
                bool goahead = 1;

                if (river == 1) // Expanding the river itself
                {
                    if (region.fakedir(i, j) > 0)
                        goahead = 0;

                    if (region.lakesurface(i, j) != 0)
                        goahead = 0;

                    if (goahead == 1 && pixels > 2) // Check that it's within a circle.
                    {
                        if ((i - x) * (j - y) > (pixels * pixels) + pixels)
                            goahead = 0;
                    }

                    if (janload + julload < region.fakejan(i, j) + region.fakejul(i, j)) // Don't overwrite existing fake rivers here unless they're smaller
                        goahead = 0;

                    if (goahead == 1)
                    {
                        region.setfakedir(i, j, dir);
                        region.setfakejan(i, j, janload);
                        region.setfakejul(i, j, julload);

                        fakesourcex[i][j] = centrex;
                        fakesourcey[i][j] = centrey; // This records the location of the actual river that's providing the source for this point of fake river.
                    }
                }
                else // Expanding the river valley
                {

                    if (region.fakedir(i, j) > 0)
                        goahead = 0;

                    if (region.fakedir(i, j) == -1 && region.map(i, j) > region.map(centrex, centrey))
                        goahead = 0;

                    if (region.lakesurface(i, j) != 0)
                        goahead = 0;

                    if (goahead == 1 && pixels > 2) // Check that it's within a circle.
                    {
                        if ((i - x) * (j - y) > (pixels * pixels) + pixels)
                            goahead = 0;
                    }

                    if (goahead == 1)
                    {
                        nearest = findclosestriver(region, i, j, delta);

                        if (nearest.x == -1)
                        {
                            nearest.x = centrex;
                            nearest.y = centrey;
                        }

                        region.setmap(i, j, region.map(nearest.x, nearest.y));
                        region.setfakedir(i, j, -1);
                    }
                }
            }
        }
    }
}

// This function turns all river cells that are near lakes into lakes, and widens them.

void turnriverstolakes(planet& world, region& region, int dx, int dy, int sx, int sy)
{
    int surfacelevel = world.lakesurface(sx, sy);

    if (surfacelevel == 0)
        return;

    int rwidth = region.rwidth();
    int rheight = region.rheight();

    int varamount = 20; // Amount lake beds might be raised/lowered by
    int bedlevel = world.nom(sx, sy);

    fast_srand((sy * world.width() + sx) + world.map(sx, sy) + (int)world.roughness(sx, sy));

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (region.riverdir(i, j) != 0 || region.fakedir(i, j) > 0)
            {
                // Do a minimal blob here, to ensure that the lake is continuous.

                for (int k = i - 1; k <= i + 1; k++)
                {
                    for (int l = j - 1; l <= j + 1; l++)
                    {
                        if (k >= 0 && k <= rwidth && l >= 0 && l <= rheight)
                        {
                            if (region.lakesurface(k, l) == 0)
                            {
                                region.setlakesurface(k, l, surfacelevel);

                                int thiselev = bedlevel + randomsign(random(0, varamount));

                                if (thiselev < 1)
                                    thiselev = 1;

                                if (thiselev >= surfacelevel - 1)
                                    thiselev = surfacelevel - 2;

                                region.setmap(k, l, thiselev);
                            }
                        }
                    }
                }
            }
        }
    }
}

// This function checks for rivers that don't quite meet the sea, and completes them

void finishrivers(planet& world, region& region, int leftx, int lefty, int rightx, int righty)
{
    twointegers seapoint;

    // First, clean up the river map.

    for (int i = leftx; i <= rightx; i++)
    {
        for (int j = lefty; j <= righty; j++)
        {
            if (region.riverjan(i, j) != 0 || region.riverjul(i, j) != 0)
            {
                if (region.riverdir(i, j) == 0)
                {
                    region.setriverjan(i, j, 0);
                    region.setriverjul(i, j, 0);
                }
            }
        }
    }

    // Now search for any unfinished rivers.

    for (int i = leftx; i <= rightx; i++)
    {
        for (int j = lefty; j <= righty; j++)
        {
            if (region.riverdir(i, j) != 0) // If there's a river here
            {
                int x = i;
                int y = j;
                int dir = region.riverdir(i, j);

                if (dir == 8 || dir == 1 || dir == 2)
                    y--;

                if (dir == 4 || dir == 5 || dir == 6)
                    y++;

                if (dir == 2 || dir == 3 || dir == 4)
                    x++;

                if (dir == 6 || dir == 7 || dir == 8)
                    x--;

                if (x >= leftx && x <= rightx && y >= lefty && y <= righty)
                {
                    if (region.riverdir(x, y) == 0) // If it flows into a cell with no river!
                    {
                        if (region.sea(x, y) == 0 && region.lakesurface(x, y) == 0) // If it's also not sea or lake
                        {
                            // Get the load of this river.

                            int janload = region.riverjan(i, j);
                            int julload = region.riverjul(i, j);

                            int elev = region.map(i, j);

                            // Find the nearest sea.

                            seapoint = nearestsea(region, leftx, lefty, rightx, righty, i, j);
                            drawriverline(region, leftx, lefty, rightx, righty, i, j, seapoint.x, seapoint.y, janload, julload, elev);
                        }
                    }
                }
            }
        }
    }
}

// This function draws a line of river from one point to another on the regional map at a given elevation.

void drawriverline(region& region, int leftx, int lefty, int rightx, int righty, int startx, int starty, int endx, int endy, int janload, int julload, int elev)
{
    int x = startx;
    int y = starty;

    int dir = region.riverdir(x, y);

    while (1 == 1)
    {
        region.setriverdir(x, y, dir);
        region.setriverjan(x, y, janload);
        region.setriverjul(x, y, julload);
        region.setmap(x, y, elev);

        if (dir == 8 || dir == 1 || dir == 2)
            y--;

        if (dir == 4 || dir == 5 || dir == 6)
            y++;

        if (dir == 2 || dir == 3 || dir == 4)
            x++;

        if (dir == 6 || dir == 7 || dir == 8)
            x--;

        if (x<leftx || x>rightx || y<lefty || y>righty)
            return;

        if (region.sea(x, y) == 1)
            return;

        if (region.lakesurface(x, y) != 0)
            return;

        if (region.riverjan(x, y) != 0 || region.riverjul(x, y) != 0)
            return;
    }
}

// This function creates a tile of the elevation regional map.

void makeelevationtile(planet& world, region& region, int dx, int dy, int sx, int sy, int coords[4][2], boolshapetemplate smalllake[])
{
    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();
    int maxelev = world.maxelevation();

    int min = 1;
    int max = world.maxelevation();

    if (sx<0 || sx>width)
        sx = wrap(sx, width);

    if (sx == 0)
    {
        int level = world.nom(sx, sy);

        if (level <= sealevel)
        {
            for (int i = dx; i <= dx + 16; i++)
            {
                for (int j = dy; j <= dy + 16; j++)
                {
                    region.setmap(i, j, level);
                }
            }
            return;
        }
    }

    float roughness = 0.0f;

    for (int i = sx - 2; i <= sx + 2; i++)
    {
        int ii = i;

        if (ii<0 || ii>width)
            ii = wrap(ii, width);

        for (int j = sy - 2; j <= sy + 2; j++)
        {
            if (j >= 0 && j <= height && world.roughness(i, j) > roughness)
                roughness = world.roughness(i, j);
        }
    }

    bool adjustsea = 1;

    int seaadjust = 0;

    if (world.nom(sx, sy) > sealevel)
        adjustsea = 0;
    else
    {
        seaadjust = sealevel - (2400 - (int)(2000.0f * roughness));

        if (world.mintemp(sx, sy) <= world.glacialtemp())
            seaadjust = sealevel - (2400 - (int)(2000.0f * roughness)) / 12;
    }

    // First, get coordinates for the tiles around this one.

    int eax = sx + 1;
    int eay = sy;
    if (eax > width)
        eax = 0;

    int sox = sx;
    int soy = sy + 1;

    int sex = eax;
    int sey = soy;

    // Now, initialise the four corners. We use mapnom (the world map without mountains) because we'll be adding mountains later. We also add the extra elevation to make the land in general higher than the rivers. If there are major lakes nearby, use the surface level of those instead.

    if (dx != 0 && dy != 0 && region.map(dx, dy) == 0)
    {
        if (world.riftlakesurface(sx, sy) != 0)
            region.setmap(dx, dy, world.riftlakesurface(sx, sy));
        else
        {
            if (world.lakesurface(sx, sy) != 0)
                region.setmap(dx, dy, world.lakesurface(sx, sy));

            else
                region.setmap(dx, dy, world.nom(sx, sy) + world.extraelev(sx, sy));
        }
    }

    if (dx != 0 && region.map(dx + 16, dy) == 0)
    {
        if (world.riftlakesurface(eax, eay) != 0)
            region.setmap(dx + 16, dy, world.riftlakesurface(eax, eay));
        else
        {
            if (world.lakesurface(eax, eay) != 0)
                region.setmap(dx + 16, dy, world.lakesurface(eax, eay));

            else
                region.setmap(dx + 16, dy, world.nom(eax, eay) + world.extraelev(eax, eay));
        }
    }

    if (dx != 0 && region.map(dx, dy + 16) == 0)
    {
        if (world.riftlakesurface(sox, soy) != 0)
            region.setmap(dx, dy + 16, world.riftlakesurface(sox, soy));
        else
        {
            if (world.lakesurface(sox, soy) != 0)
                region.setmap(dx, dy + 16, world.lakesurface(sox, soy));

            else
                region.setmap(dx, dy + 16, world.nom(sox, soy) + world.extraelev(sox, soy));
        }
    }

    if (region.map(dx + 16, dy + 16) == 0)
    {
        if (world.riftlakesurface(sex, sey) != 0)
            region.setmap(dx + 16, dy + 16, world.riftlakesurface(sex, sey));
        else
        {
            if (world.lakesurface(sex, sey) != 0)
                region.setmap(dx + 16, dy + 16, world.lakesurface(sex, sey));

            else
                region.setmap(dx + 16, dy + 16, world.nom(sex, sey) + world.extraelev(sex, sey));
        }
    }

    if (adjustsea)
    {
        if (region.map(dx, dy) <= sealevel)
            region.setmap(dx, dy, seaadjust);

        if (region.map(dx + 16, dy) <= sealevel)
            region.setmap(dx + 16, dy, seaadjust);

        if (region.map(dx, dy + 16) <= sealevel)
            region.setmap(dx, dy + 16, seaadjust);

        if (region.map(dx + 16, dy + 16) <= sealevel)
            region.setmap(dx + 16, dy + 16, seaadjust);
    }

    if (region.map(dx, dy) < 1 || region.map(dx, dy) > maxelev)
        region.setmap(dx, dy, world.nom(sx, sy));

    if (region.map(dx + 16, dy) < 1 || region.map(dx + 16, dy) > maxelev)
        region.setmap(dx + 16, dy, world.nom(sx, sy));

    if (region.map(dx, dy + 16) < 1 || region.map(dx, dy + 16) > maxelev)
        region.setmap(dx, dy + 16, world.nom(sx, sy));

    if (region.map(dx + 16, dy + 16) < 1 || region.map(dx + 16, dy + 16) > maxelev)
        region.setmap(dx + 16, dy + 16, world.nom(sx, sy));

    // Now we need to fill in the rest.

    bool riverspresent = 0;

    if (world.riverjan(sx, sy) > 0 || world.riverjul(sx, sy) > 0)
        riverspresent = 1;

    bool onlyup = 0; // If it's 1, height values will only be raised, not lowered.

    if (riverspresent == 1 && region.map(dx, dy) > sealevel && region.map(dx + 16, dy) > sealevel && region.map(dx + 16, dy + 16) > sealevel && region.map(dx, dy + 16) > sealevel)
        onlyup = 1;

    bool nosea = 0; // If it's 1, we won't allow height values lower than sea level. If we're by a major lake, this will be the case.

    for (int i = sx - 2; i <= sx + 2; i++)
    {
        int ii = i;

        if (ii<0 || ii>width)
            ii = wrap(ii, width);

        for (int j = sy - 2; j <= sy + 2; j++)
        {
            if (j >= 0 && j <= height)
            {
                if (world.truelake(i, j) == 1)
                    nosea = 1;
            }
        }
    }

    // Now we do the edges.

    int nseed = (sy * width + sx) + world.map(sx, sy) + world.julrain(sx, sy);
    int sseed = (soy * width + sox) + world.map(sox, soy) + world.julrain(sox, soy);
    int wseed = (sy * width + sx) + world.map(sx, sy) + world.julrain(sx, sy);
    int eseed = (eay * width + eax) + world.map(eax, eay) + world.julrain(eax, eay);

    float valuemult = 30.0f; // Amount to multiply the valuemod by for the edges.

    if (dy != 0 && dx != 0)
    {
        if (region.map(dx + 8, dy) == 0) // Northern edge
        {
            fast_srand(nseed);

            float s = 16.0f; // Segment size.

            while (s != -1.0f)
            {
                //int value=s*valuemod*valuemult;
                int ss = (int)(s / 2.0f); // Size of half the segment.

                for (int n = 0; n <= (int)(16.0f / s) - 1; n++) // Go through all the segments of this size.
                {
                    int nn = (int)((float)n * s);
                    int ourpoint = nn + ss;

                    int value = (int)(s * region.roughness(dx + ourpoint, dy) * valuemult);

                    if (region.map(dx + ourpoint, dy) == 0)
                    {
                        int newvalue = (region.surface(dx + nn, dy) + region.surface(dx + nn + (int)s, dy)) / 2;
                        int difference = randomsign(random(0, value));

                        if (onlyup == 1 && random(0, 1) == 1)
                            difference = abs(difference);

                        newvalue = newvalue + difference;

                        if (newvalue <= sealevel)
                        {
                            if (region.surface(dx, dy) > sealevel && region.surface(dx + 16, dy) > sealevel)
                            {
                                if (checknearbyriver(region, dx + ourpoint, dy) == 0)
                                    newvalue = sealevel + 1;
                            }

                            if (nosea == 1)
                                newvalue = sealevel + 1;
                        }
                        else
                        {
                            if (region.surface(dx, dy) <= sealevel && region.surface(dx + 16, dy) <= sealevel)
                                newvalue = sealevel;
                        }

                        if (newvalue <= sealevel && adjustsea)
                            newvalue = seaadjust;

                        region.setmap(dx + ourpoint, dy, newvalue);
                    }
                }

                if (s > 2.0f)
                    s = s / 2.0f;
                else
                    s = -1.0f;
            }
        }
    }

    if (dx != 0)
    {
        if (region.map(dx + 8, dy + 16) == 0) // Southern edge
        {
            fast_srand(sseed);

            float s = 16.0f; // Segment seize.

            while (s != -1.0f)
            {
                int ss = (int)(s / 2.0f); // Size of half the segment.

                for (int n = 0; n <= (int)(16.0f / s) - 1; n++) // Go through all the segments of this size.
                {
                    int nn = (int)((float)n * s);
                    int ourpoint = nn + ss;

                    int value = (int)(s * region.roughness(dx + ourpoint, dy + 16) * valuemult);

                    if (region.map(dx + ourpoint, dy + 16) == 0)
                    {
                        int newvalue = (region.surface(dx + nn, dy + 16) + region.surface(dx + nn + (int)s, dy + 16)) / 2;
                        int difference = randomsign(random(0, value));

                        if (onlyup == 1 && random(0, 1) == 1)
                            difference = abs(difference);

                        newvalue = newvalue + difference;

                        if (newvalue <= sealevel)
                        {
                            if (region.surface(dx, dy + 16) > sealevel && region.surface(dx + 16, dy + 16) > sealevel)
                            {
                                if (checknearbyriver(region, dx + ourpoint, dy + 16) == 0)
                                    newvalue = sealevel + 1;
                            }
                            if (nosea == 1)
                                newvalue = sealevel + 1;
                        }
                        else
                        {
                            if (region.surface(dx, dy + 16) <= sealevel && region.surface(dx + 16, dy + 16) <= sealevel)
                                newvalue = sealevel;
                        }

                        if (newvalue <= sealevel && adjustsea)
                            newvalue = seaadjust;

                        region.setmap(dx + ourpoint, dy + 16, newvalue);
                    }
                }

                if (s > 2.0f)
                    s = s / 2.0f;
                else
                    s = -1.0f;
            }
        }
    }

    if (dx != 0 && dy != 0)
    {
        if (region.map(dx, dy + 8) == 0) // Western edge
        {
            fast_srand(wseed);

            float s = 16.0f; // Segment seize.

            while (s != -1.0f)
            {
                //int value=s*valuemod*valuemult;
                int ss = (int)(s / 2.0f); // Size of half the segment.

                for (int n = 0; n <= (int)(16.0f / s) - 1; n++) // Go through all the segments of this size.
                {
                    int nn = (int)((float)n * s);
                    int ourpoint = nn + ss;

                    int value = (int)(s * region.roughness(dx, dy + ourpoint) * valuemult);

                    if (region.map(dx, dy + ourpoint) == 0)
                    {
                        int newvalue = (region.surface(dx, dy + nn) + region.surface(dx, dy + nn + (int)s)) / 2;
                        int difference = randomsign(random(0, value));

                        if (onlyup == 1 && random(0, 1) == 1)
                            difference = abs(difference);

                        newvalue = newvalue + difference;

                        if (newvalue <= sealevel)
                        {
                            if (region.surface(dx, dy) > sealevel && region.surface(dx, dy + 16) > sealevel)
                            {
                                if (checknearbyriver(region, dx, dy + ourpoint) == 0)
                                    newvalue = sealevel + 1;
                            }
                            if (nosea == 1)
                                newvalue = sealevel + 1;
                        }
                        else
                        {
                            if (region.surface(dx, dy) <= sealevel && region.surface(dx, dy + 16) <= sealevel)
                                newvalue = sealevel;
                        }

                        if (newvalue <= sealevel && adjustsea)
                            newvalue = seaadjust;

                        region.setmap(dx, dy + ourpoint, newvalue);
                    }
                }

                if (s > 2.0f)
                    s = s / 2.0f;
                else
                    s = -1.0f;
            }
        }
    }

    if (dy != 0)
    {
        if (region.map(dx + 16, dy + 8) == 0) // Eastern edge
        {
            fast_srand(eseed);

            float s = 16.0f; // Segment seize.

            while (s != -1.0f)
            {
                //int value=s*valuemod*valuemult;
                int ss = (int)(s / 2.0f); // Size of half the segment.

                for (int n = 0; n <= (int)(16.0f / s) - 1; n++) // Go through all the segments of this size.
                {
                    int nn = (int)((float)n * s);
                    int ourpoint = nn + ss;

                    int value = (int)(s * region.roughness(dx + 16, dy + ourpoint) * valuemult);

                    if (region.map(dx + 16, dy + ourpoint) == 0)
                    {
                        int newvalue = (region.surface(dx + 16, dy + nn) + region.surface(dx + 16, dy + nn + (int)s)) / 2;
                        int difference = randomsign(random(0, value));

                        if (onlyup == 1 && random(0, 1) == 1)
                            difference = abs(difference);

                        newvalue = newvalue + difference;

                        if (newvalue <= sealevel)
                        {
                            if (region.surface(dx + 16, dy) > sealevel && region.surface(dx + 16, dy + 16) > sealevel)
                            {
                                if (checknearbyriver(region, dx + 16, dy + ourpoint) == 0)
                                    newvalue = sealevel + 1;
                            }
                            if (nosea == 1 && newvalue <= sealevel)
                                newvalue = sealevel + 1;
                        }
                        else
                        {
                            if (region.surface(dx + 16, dy) <= sealevel && region.surface(dx + 16, dy + 16) <= sealevel)
                                newvalue = sealevel;
                        }

                        if (newvalue <= sealevel && adjustsea)
                            newvalue = seaadjust;

                        region.setmap(dx + 16, dy + ourpoint, newvalue);
                    }
                }

                if (s > 2.0f)
                    s = s / 2.0f;
                else
                    s = -1.0f;
            }
        }
    }

    bool fullyland = 1;

    if (vaguelycoastal(world, sx, sy) == 1 || world.sea(sx, sy) == 1)
        fullyland = 0;

    // Now we fill in the middle of the tile. We do this with diamond-square.

    if (dx != 0 && dy != 0)
    {
        fast_srand((sy * width + sx) + world.map(sx, sy) + world.julrain(sx, sy));

        float s = 16.0f; // Size of the section. This will halve each time.

        while (s != -1)
        {
            //int value=s*valuemod; // This is the amount we will vary each new pixel by.
            int ss = (int)(s / 2.0f);

            // First, go over the tile doing the square step.

            for (int i = 0; i <= 16; i = i + (int)s)
            {
                int ii = i + ss;

                for (int j = 0; j <= 16; j = j + (int)s)
                {
                    int jj = j + ss;

                    int value = (int)(s * region.roughness(dx + ii, dy + jj));

                    if (jj >= 0 && jj <= 16 && ii >= 0 && ii <= 16)
                    {
                        if (region.map(dx + ii, dy + jj) == 0)
                        {
                            int newheight;

                            if (fullyland == 1)
                            {
                                newheight = rsquare(region, dx, dy, (int)s, ii, jj, value, sealevel + 1, max, coords, onlyup, 10000, sealevel, nosea);

                                if (newheight <= sealevel)
                                {
                                    if (checknearbyriver(region, dx + ii, dy + jj) == 0)
                                        newheight = sealevel + 1;
                                }
                            }
                            else
                                newheight = rcoastsquare(region, dx, dy, (int)s, ii, jj, value, min, max, coords, sealevel);

                            if (nosea == 1 && newheight <= sealevel)
                                newheight = sealevel + 1;

                            region.setmap(dx + ii, dy + jj, newheight);
                        }
                    }
                }
            }

            // Now we go over the whole tile again, doing the diamond step.

            for (int i = 0; i <= 16; i = i + (int)s)
            {
                int ii = i + ss;

                for (int j = 0; j <= 16; j = j + (int)s)
                {
                    int jj = j + ss;

                    for (int n = 1; n <= 2; n++) // We have to do the diamond step twice for each tile.
                    {
                        if (n == 1)
                        {
                            ii = i + ss;
                            jj = j;
                        }
                        else
                        {
                            ii = i;
                            jj = j + ss;
                        }

                        int value = (int)(s * region.roughness(dx + ii, dy + jj));

                        if (jj >= 0 && jj <= 16 && ii >= 0 && ii <= 16)
                        {
                            if (region.map(dx + ii, dy + jj) == 0)
                            {
                                int newheight;

                                if (fullyland == 1)
                                {
                                    newheight = rdiamond(region, dx, dy, (int)s, ii, jj, value, sealevel + 1, max, coords, onlyup, 10000, sealevel, nosea);

                                    if (newheight <= sealevel)
                                    {
                                        if (checknearbyriver(region, dx + ii, dy + jj) == 0)
                                            newheight = sealevel + 1;
                                    }
                                }
                                else
                                    newheight = rcoastdiamond(region, dx, dy, (int)s, ii, jj, value, min, max, coords, sealevel);

                                if (nosea == 1 && newheight <= sealevel)
                                    newheight = sealevel + 1;

                                region.setmap(dx + ii, dy + jj, newheight);
                            }
                        }
                    }
                }
            }

            if (s > 2.0f) // Now halve the size of the tiles.
                s = s / 2.0f;
            else
                s = -1.0f;
        }
    }

    // Now remove weird bits of river sticking out of the sea.

    removesearivers(world, region, dx, dy);
}

// This does the square part of the fractal for the regional terrain.

int rsquare(region& region, int dx, int dy, int s, int x, int y, int value, int min, int max, int coords[4][2], bool onlyup, int negchance, int sealevel, bool nosea)
{
    value = value * 50;

    int dist = s / 2;
    int total = 0;
    int crount = 0;

    coords[0][0] = x - dist;
    coords[0][1] = y - dist;

    coords[1][0] = x + dist;
    coords[1][1] = y - dist;

    coords[2][0] = x + dist;
    coords[2][1] = y + dist;

    coords[3][0] = x - dist;
    coords[3][1] = y + dist;

    for (int n = 0; n <= 3; n++)
    {
        if (coords[n][1] >= 0 && coords[n][1] <= 16 && coords[n][0] >= 0 && coords[n][0] <= 16)
        {
            total = total + region.surface(dx + coords[n][0], dy + coords[n][1]);
            crount++;
        }
    }

    total = total / crount;

    int difference = randomsign(random(0, value));

    //if (onlyup==1)
    //difference=abs(difference);

    if (difference > 0)
    {
        if (random(1, negchance) == 1)
            difference = 0 - difference;
    }

    total = total + difference;

    if (total > max)
        total = max;

    if (total < min)
        total = min;

    if (total <= sealevel && nosea == 1)
        total = sealevel + 1;

    return total;
}

// This is the same, but for coastlines.

int rcoastsquare(region& region, int dx, int dy, int s, int x, int y, int value, int min, int max, int coords[4][2], int sealevel)
{
    value = value * 50; //50;

    int dist = s / 2;
    int total = 0;
    int crount = 0;

    coords[0][0] = x - dist;
    coords[0][1] = y - dist;

    coords[1][0] = x + dist;
    coords[1][1] = y - dist;

    coords[2][0] = x + dist;
    coords[2][1] = y + dist;

    coords[3][0] = x - dist;
    coords[3][1] = y + dist;

    for (int n = 0; n <= 3; n++)
    {
        if (coords[n][1] >= 0 && coords[n][1] <= 16 && coords[n][0] >= 0 && coords[n][0] <= 16)
        {
            total = total + region.surface(dx + coords[n][0], dy + coords[n][1]);
            crount++;
        }
    }

    total = total / crount;

    int difference = randomsign(random(0, value));

    if (total <= sealevel)
    {
        if (total + difference <= sealevel)
            total = total + difference;
    }
    else
        total = total + difference;

    if (total > max)
        total = max;

    if (total < min)
        total = min;

    return total;
}

// This is the same, but for lakes.

int rlakesquare(region& region, int dx, int dy, int s, int x, int y, int value, int min, int max, int coords[4][2], int lakesurface)
{
    value = value * 500; //50;

    int dist = s / 2;
    int total = 0;
    int crount = 0;

    coords[0][0] = x - dist;
    coords[0][1] = y - dist;

    coords[1][0] = x + dist;
    coords[1][1] = y - dist;

    coords[2][0] = x + dist;
    coords[2][1] = y + dist;

    coords[3][0] = x - dist;
    coords[3][1] = y + dist;

    for (int n = 0; n <= 3; n++)
    {
        if (coords[n][1] >= 0 && coords[n][1] <= 16 && coords[n][0] >= 0 && coords[n][0] <= 16)
        {
            total = total + region.map(dx + coords[n][0], dy + coords[n][1]);
            crount++;
        }
    }

    total = total / crount;

    int difference = randomsign(random(0, value));

    if (total <= lakesurface)
    {
        if (total + difference <= lakesurface)
            total = total + difference;
    }
    else
        total = total + difference;

    if (total > max)
        total = max;

    if (total < min)
        total = min;

    return total;
}

// This does the diamond part of the fractal for the regional terrain.

int rdiamond(region& region, int dx, int dy, int s, int x, int y, int value, int min, int max, int coords[4][2], bool onlyup, int negchance, int sealevel, bool nosea)
{
    value = value * 50;

    int dist = s / 2;
    int total = 0;
    int crount = 0;

    coords[0][0] = x;
    coords[0][1] = y - dist;

    coords[1][0] = x + dist;
    coords[1][1] = y;

    coords[2][0] = x;
    coords[2][1] = y + dist;

    coords[3][0] = x - dist;
    coords[3][1] = y;

    for (int n = 0; n <= 3; n++)
    {
        if (coords[n][1] >= 0 && coords[n][1] <= 16 && coords[n][0] >= 0 && coords[n][0] <= 16)
        {
            total = total + region.surface(dx + coords[n][0], dy + coords[n][1]);
            crount++;
        }
    }

    total = total / crount;

    int difference = randomsign(random(0, value));

    if (onlyup == 1 && random(0, 1) == 1)
        difference = abs(difference);

    if (difference > 0)
    {
        if (random(1, negchance) == 1)
            difference = 0 - difference;
    }

    total = total + difference;

    if (total > max)
        total = max;

    if (total < min)
        total = min;

    if (total <= sealevel && nosea == 1)
        total = sealevel + 1;

    return total;
}

// This is the same, but for coastlines.

int rcoastdiamond(region& region, int dx, int dy, int s, int x, int y, int value, int min, int max, int coords[4][2], int sealevel)
{
    value = value * 50; //50;

    int dist = s / 2;
    int total = 0;
    int crount = 0;

    coords[0][0] = x;
    coords[0][1] = y - dist;

    coords[1][0] = x + dist;
    coords[1][1] = y;

    coords[2][0] = x;
    coords[2][1] = y + dist;

    coords[3][0] = x - dist;
    coords[3][1] = y;

    for (int n = 0; n <= 3; n++)
    {
        if (coords[n][1] >= 0 && coords[n][1] <= 16 && coords[n][0] >= 0 && coords[n][0] <= 16)
        {
            total = total + region.surface(dx + coords[n][0], dy + coords[n][1]);
            crount++;
        }
    }

    total = total / crount;

    int difference = randomsign(random(0, value));

    if (total <= sealevel)
    {
        if (total + difference <= sealevel)
            total = total + difference;
    }
    else
        total = total + difference;

    if (total > max)
        total = max;

    if (total < min)
        total = min;

    return total;
}

// This is the same, but for lakes.

int rlakediamond(region& region, int dx, int dy, int s, int x, int y, int value, int min, int max, int coords[4][2], int lakesurface)
{
    value = value * 500; //50;

    int dist = s / 2;
    int total = 0;
    int crount = 0;

    coords[0][0] = x;
    coords[0][1] = y - dist;

    coords[1][0] = x + dist;
    coords[1][1] = y;

    coords[2][0] = x;
    coords[2][1] = y + dist;

    coords[3][0] = x - dist;
    coords[3][1] = y;

    for (int n = 0; n <= 3; n++)
    {
        if (coords[n][1] >= 0 && coords[n][1] <= 16 && coords[n][0] >= 0 && coords[n][0] <= 16)
        {
            total = total + region.map(dx + coords[n][0], dy + coords[n][1]);
            crount++;
        }
    }

    total = total / crount;

    int difference = randomsign(random(0, value));

    if (total <= lakesurface)
    {
        if (total + difference <= lakesurface)
            total = total + difference;
    }
    else
        total = total + difference;

    if (total > max)
        total = max;

    if (total < min)
        total = min;

    return total;
}

// This warps coastlines.

void warpcoasts(planet& world, region& region, int leftx, int lefty, int rightx, int righty, vector<vector<int>>& warptox, vector<vector<int>>& warptoy)
{
    int sealevel = world.sealevel();

    vector<vector<bool>> warpedland(RARRAYWIDTH, vector<bool>(RARRAYHEIGHT, 0)); // This just shows land and sea.

    for (int i = leftx; i <= rightx; i++)
    {
        for (int j = lefty; j <= righty; j++)
        {
            int x = (warptox[i][j] + i * 3) / 4;
            int y = (warptoy[i][j] + j * 3) / 4;

            bool land = 1;

            if (region.map(x, y) <= sealevel && region.lakesurface(x, y) == 0)
                land = 0;

            warpedland[i][j] = land;
        }
    }

    for (int i = leftx; i <= rightx; i++)
    {
        for (int j = lefty; j <= righty; j++)
        {
            if (warpedland[i][j] == 0)
            {
                if (region.map(i, j) > sealevel && region.lakesurface(i, j) == 0)
                    region.setmap(i, j, sealevel - 10);
            }
        }
    }
}

// This goes over the current tile on the regional map and removes straight lines on the coasts.

void removestraights(planet& world, region& region, int dx, int dy, int sx, int sy, boolshapetemplate smalllake[])
{
    int width = world.width();
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    int maxlength = 4; // Any straights longer than this will be disrupted.

    int minsize = 2;
    int maxsize = 7; //4; // Range of sizes for the chunks to be cut out of the coasts.

    bool raise = 0; // Chunks might be land or sea.

    if (world.sea(sx, sy) == 0) // If we're further inland, they should only be land.
        raise = 1;

    fast_srand((sy * width + sx) + world.map(sx, sy) + world.julrain(sx, sy));

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (region.sea(i, j) == 1)
            {
                if (j > 0)
                {
                    if (region.sea(i, j - 1) == 0) // Sea to the south, land to the north
                    {
                        bool keepgoing = 1;
                        int x = i;
                        int y = j;

                        do
                        {
                            if (region.sea(x, y) == 0)
                                keepgoing = 0;

                            if (region.sea(x, y - 1) == 1)
                                keepgoing = 0;

                            x++;

                            if (x > dx + 16)
                                keepgoing = 0;

                        } while (keepgoing == 1);

                        if (x > i + maxlength) // If this line is too long
                        {
                            for (int k = i; k <= x; k++)
                            {
                                int size = random(minsize, maxsize);
                                disruptseacoastline(world, region, dx, dy, k, j, region.map(i, j), raise, size, 0, smalllake);
                            }
                        }
                    }
                }

                if (j < rheight)
                {
                    if (region.sea(i, j + 1) == 0) // Sea to the north, land to the south
                    {
                        bool keepgoing = 1;
                        int x = i;
                        int y = j;

                        do
                        {
                            if (region.sea(x, y) == 0)
                                keepgoing = 0;

                            if (region.sea(x, y + 1) == 1)
                                keepgoing = 0;

                            x++;

                            if (x > dx + 16)
                                keepgoing = 0;

                        } while (keepgoing == 1);

                        if (x > i + maxlength) // If this line is too long
                        {
                            for (int k = i; k <= x; k++)
                            {
                                int size = random(minsize, maxsize);
                                disruptseacoastline(world, region, dx, dy, k, j, region.map(i, j), raise, size, 0, smalllake);
                            }
                        }
                    }
                }

                if (i < rwidth)
                {
                    if (region.sea(i + 1, j) == 0) // Sea to the west, land to the east
                    {
                        bool keepgoing = 1;
                        int x = i;
                        int y = j;

                        do
                        {
                            if (region.sea(x, y) == 0)
                                keepgoing = 0;

                            if (region.sea(x + 1, y) == 1)
                                keepgoing = 0;

                            y++;

                            if (y > dy + 16)
                                keepgoing = 0;

                        } while (keepgoing == 1);

                        if (y > j + maxlength) // If this line is too long
                        {
                            for (int k = j; k <= y; k++)
                            {
                                int size = random(minsize, maxsize);
                                disruptseacoastline(world, region, dx, dy, i, k, region.map(i, j), raise, size, 0, smalllake);
                            }
                        }
                    }
                }

                if (i > 0)
                {
                    if (region.sea(i - 1, j) == 0) // Sea to the east, land to the west
                    {
                        bool keepgoing = 1;
                        int x = i;
                        int y = j;

                        do
                        {
                            if (region.sea(x, y) == 0)
                                keepgoing = 0;

                            if (region.sea(x - 1, y) == 1)
                                keepgoing = 0;

                            y++;

                            if (y > dy + 16)
                                keepgoing = 0;

                        } while (keepgoing == 1);

                        if (y > j + maxlength) // If this line is too long
                        {
                            for (int k = j; k <= y; k++)
                            {
                                int size = random(minsize, maxsize);
                                disruptseacoastline(world, region, dx, dy, i, k, region.map(i, j), raise, size, 0, smalllake);
                            }
                        }
                    }
                }
            }
        }
    }

    return;

    // Now do diagonals, which are tricker.

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (region.sea(i, j) == 1 && region.sea(i + 1, j + 1) == 1 && region.sea(i + 2, j + 2) == 1 && region.sea(i + 3, j + 3 == 1))
            {
                if (region.sea(i - 1, j) == 0 && region.sea(i, j + 1) == 0 && region.sea(i + 1, j + 2) == 0 && region.sea(i + 2, j + 3) == 0)
                {
                    for (int k = i; k <= i + 3; k++)
                    {
                        for (int l = j; l <= j + 3; l++)
                        {
                            int size = random(minsize, maxsize);
                            disruptseacoastline(world, region, dx, dy, k, l, region.map(i + 1, j + 1), raise, size, 0, smalllake);
                        }
                    }
                }
            }

            if (region.sea(i - 1, j) == 1 && region.sea(i, j + 1) == 1 && region.sea(i + 1, j + 2) == 1 && region.sea(i + 2, j + 3 == 1))
            {
                if (region.sea(i, j) == 0 && region.sea(i + 1, j + 1) == 0 && region.sea(i + 2, j + 2) == 0 && region.sea(i + 3, j + 3) == 0)
                {
                    for (int k = i - 1; k <= i + 2; k++)
                    {
                        for (int l = j; l <= j + 3; l++)
                        {
                            int size = random(minsize, maxsize);
                            disruptseacoastline(world, region, dx, dy, k, l, region.map(i + 1, j + 1), raise, size, 0, smalllake);
                        }
                    }
                }
            }
        }
    }

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (region.sea(i, j) == 1 && region.sea(i + 1, j - 1) == 1 && region.sea(i + 2, j - 2) == 1 && region.sea(i + 3, j - 3) == 1)
            {
                if (region.sea(i - 1, j) == 0 && region.sea(i, j - 1) == 0 && region.sea(i + 1, j - 2) == 0 && region.sea(i + 2, j - 3) == 0)
                {
                    for (int k = i; k <= i + 3; k++)
                    {
                        for (int l = j - 3; l >= j; l--)
                        {
                            int size = random(minsize, maxsize);
                            disruptseacoastline(world, region, dx, dy, k, l, region.map(i + 1, j + 1), raise, size, 0, smalllake);
                        }
                    }
                }
            }

            if (region.sea(i, j) == 0 && region.sea(i + 1, j - 1) == 0 && region.sea(i + 2, j - 2) == 0 && region.sea(i + 3, j - 3) == 0)
            {
                if (region.sea(i - 1, j) == 1 && region.sea(i, j - 1) == 1 && region.sea(i + 1, j - 2) == 1 && region.sea(i + 2, j - 3) == 1)
                {
                    for (int k = i - 1; k <= i + 2; k++)
                    {
                        for (int l = j - 3; l >= j; l--)
                        {
                            int size = random(minsize, maxsize);
                            disruptseacoastline(world, region, dx, dy, k, l, region.map(i + 1, j + 1), raise, size, 0, smalllake);
                        }
                    }
                }
            }
        }
    }
}

// This function pastes a small lake template over the coastline, to try to make it more interesting.

void disruptseacoastline(planet& world, region& region, int dx, int dy, int centrex, int centrey, int avedepth, bool raise, int maxsize, bool stayintile, boolshapetemplate smalllake[])
{
    int width = world.width();
    int sealevel = world.sealevel();

    if (raise == 0)
    {
        raise = random(0, 1);

        if (raise == 1)
        {
            if (random(1, world.maxelevation()) > world.roughness(centrex, centrey) * 1.5f)
                raise = 0;
        }
    }

    if (maxsize == 0) // If no size is specified
    {
        int diffx = centrex - dx;
        if (diffx > 8)
            diffx = 16 - diffx;

        int diffy = centrey - dy;
        if (diffy > 8)
            diffy = 16 - dy;

        if (diffx < diffy)
            maxsize = diffx;
        else
            maxsize = diffy;

        maxsize = (maxsize * 2) - 3;
    }

    // maxsize is now the maximum width/height of this template, to avoid going over the borders of the tile.

    int minsize = maxsize - 2;

    if (raise == 0)
    {
        maxsize++;
        minsize++;
    }

    if (maxsize > 11)
        maxsize = 11;

    if (minsize < 0)
        minsize = 0;

    if (minsize > 9)
        minsize = 9;

    int shapenumber = random(minsize, maxsize);

    if (raise == 0)
        shapenumber++;

    if (shapenumber > 11)
        shapenumber = 11;

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
        jstart = imheight;
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

                bool goodtogo = 1;

                if (stayintile == 1)
                {
                    if (xx >= dx && xx <= dx + 16 && yy >= dy && yy <= dy + 16)
                        goodtogo = 1;
                    else
                        goodtogo = 0;
                }

                if (goodtogo == 1)
                {
                    if (raise == 0) // If we're lowering this chunk.
                    {
                        if (region.map(xx, yy) > avedepth)
                            region.setmap(xx, yy, avedepth);
                    }
                    else // If we're raising it.
                    {
                        if (region.map(xx, yy) <= sealevel && region.map(xx, yy) != 0)
                            region.setmap(xx, yy, sealevel + 1);
                    }
                }
            }
        }
    }
}

// This function pastes a small lake template over a large lake coastline, to try to make it more interesting.

void disruptlakecoastline(planet& world, region& region, int dx, int dy, int centrex, int centrey, int surfacelevel, int avedepth, bool raise, int maxsize, bool stayintile, int special, boolshapetemplate smalllake[])
{
    int width = world.width();
    int sealevel = world.sealevel();

    if (maxsize == 0) // If no size is specified
    {
        int diffx = centrex - dx;
        if (diffx > 8)
            diffx = 16 - diffx;

        int diffy = centrey - dy;
        if (diffy > 8)
            diffy = 16 - dy;

        if (diffx < diffy)
            maxsize = diffx;
        else
            maxsize = diffy;

        maxsize = (maxsize * 2) - 3;

        if (maxsize > 10)
            maxsize = 10;

        if (maxsize < 2)
            maxsize = 2;
    }

    // maxsize is now the maximum width/height of this template, to avoid going over the borders of the tile.

    int minsize = maxsize - 2;

    if (minsize < 2)
        minsize = 2;

    int shapenumber = random(minsize, maxsize);

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
        jstart = imheight;
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

                bool goodtogo = 1;

                if (stayintile == 1)
                {
                    if (xx >= dx && xx <= dx + 16 && yy >= dy && yy <= dy + 16)
                        goodtogo = 1;
                    else
                        goodtogo = 0;
                }

                if (goodtogo == 1)
                {
                    if (raise == 0) // If we're lowering this chunk.
                    {
                        if (region.map(xx, yy) > avedepth)
                            region.setmap(xx, yy, avedepth);

                        if (surfacelevel != sealevel)
                        {
                            region.setlakesurface(xx, yy, surfacelevel);

                            if (special != 0)
                                region.setspecial(xx, yy, special); // Just in case this is a weird lake, i.e. salt pan or similar.
                        }
                    }
                    else // If we're raising it.
                    {
                        if (region.map(xx, yy) <= surfacelevel && region.map(xx, yy) != 0)
                        {
                            region.setmap(xx, yy, surfacelevel + 1);
                            region.setmap(xx, yy, 0);

                            region.setlakesurface(xx, yy, 0);
                            region.setspecial(xx, yy, 0);
                        }
                    }
                }
            }
        }
    }
}

// This function removes rivers that are in tiles that are meant to be mainly sea *and* that are mostly surrounding by tiles that are mainly sea.

void removeseatilerivers(planet& world, region& region, int dx, int dy, int sx, int sy)
{
    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();

    if (world.map(sx, sy) > sealevel)
        return;

    int eax = sx - 1;
    if (eax < 0)
        eax = width;

    if (world.map(eax, sy) > sealevel)
        return;

    int wex = sx + 1;
    if (wex > width)
        wex = 0;

    if (world.map(wex, sy) > sealevel)
        return;

    if (sy > 0 && world.map(sx, sy - 1) > sealevel)
        return;

    if (sy<height && world.map(sx, sy + 1)>sealevel)
        return;

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            region.setriverdir(i, j, 0);
            region.setriverjan(i, j, 0);
            region.setriverjul(i, j, 0);
        }
    }
}

// This function removes odd bits of river sticking up out of the sea.

void removesearivers(planet& world, region& region, int dx, int dy)
{
    int maxelev = world.maxelevation();
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (region.riverjan(i, j) != 0 || region.riverjul(i, j) != 0)
            {
                if (region.sea(i, j) == 0)
                {
                    if (findsurroundingsea(region, i, j) > 3)
                    {
                        int lowest = maxelev;

                        for (int k = i - 1; k <= i + 1; k++)
                        {
                            for (int l = j - 1; l <= j + 1; l++)
                            {
                                if (k >= 0 && k <= rwidth && l >= 0 and l <= rheight)
                                {
                                    if (region.map(k, l) < lowest)
                                        lowest = region.map(k, l);
                                }
                            }
                        }

                        region.setmap(i, j, lowest);
                    }
                }
            }
        }
    }
}

// This one gets rid of odd stringy spits of land with rivers on them.

void removeextrasearivers(planet& world, region& region, int dx, int dy, int sx, int sy, vector<vector<bool>>& regionsea)
{
    if (world.sea(sx, sy) == 0 && world.outline(sx, sy) == 0)
        return;

    int rwidth = region.rwidth();
    int rheight = region.rheight();
    int sealevel = world.sealevel();

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (i > 0 && i < rwidth && j>0 && j<rheight && region.map(i, j)>sealevel)
            {
                if (region.riverdir(i, j) != 0 || region.fakedir(i, j) > 0)
                {
                    int depth = 0;

                    if (region.sea(i - 1, j) == 1 && regionsea[i + 1][j] == 1)
                        depth = region.map(i - 1, j);

                    if (region.sea(i, j - 1) == 1 && regionsea[i][j + 1] == 1)
                        depth = region.map(i, j - 1);

                    if (region.sea(i - 1, j - 1) == 1 && regionsea[i + 1][j + 1] == 1)
                        depth = region.map(i - 1, j - 1);

                    if (region.sea(i + 1, j - 1) == 1 && regionsea[i - 1][j + 1] == 1)
                        depth = region.map(i + 1, j - 1);

                    if (depth != 0)
                    {
                        region.setriverdir(i, j, 0);
                        region.setriverjan(i, j, 0);
                        region.setriverjul(i, j, 0);

                        region.setfakedir(i, j, 0);
                        region.setfakejan(i, j, 0);
                        region.setfakejul(i, j, 0);

                        region.setmap(i, j, depth);
                    }
                }
            }
        }
    }
}

// This function removes rivers that come out of the sea (for where a river goes into the sea and then comes out of it again a little further on).

void removeriverscomingfromsea(planet& world, region& region, int dx, int dy, int sx, int sy, vector<vector<int>>& fakesourcex, vector<vector<int>>& fakesourcey, vector<vector<bool>>& regionsea)
{
    int width=world.width();
    int height = world.height();
    
    if (world.sea(sx, sy) == 0 && world.outline(sx, sy) == 0)
        return;

    int rwidth = region.rwidth();
    int rheight = region.rheight();

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (regionsea[i][j] == 0 && region.riverdir(i, j) != 0)
            {
                bool nexttosea = 0;

                for (int k = i - 1; k <= i + 1; k++)
                {
                    for (int l = j - 1; l <= j + 1; l++)
                    {
                        if (k >= 0 && k <= rwidth && l >= 0 && l <= rheight)
                        {
                            if (regionsea[k][l] == 1)
                            {
                                nexttosea = 1;
                                k = i + 1;
                                l = j + 1;
                            }
                        }
                    }
                }

                if (nexttosea == 1)
                {
                    bool found = findinflowinglandriver(region, i, j);

                    if (found == 0)
                        removeregionalriver(region, dx, dy, sx, sy, i, j, fakesourcex, fakesourcey);

                }
            }
        }
    }
}

// This function removes a river from the regional map, from the given point downstream.

void removeregionalriver(region& region, int dx, int dy, int sx, int sy, int startx, int starty, vector<vector<int>>& fakesourcex, vector<vector<int>>& fakesourcey)
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    int janflow = region.riverjan(startx, starty);
    int julflow = region.riverjul(startx, starty);

    int x = startx;
    int y = starty;

    for (int n = 1; n <= 200; n++)
    {
        int dir = region.riverdir(x, y);

        int newjan = region.riverjan(x, y) - janflow;
        int newjul = region.riverjul(x, y) - julflow;

        if (newjan < 0)
            newjan = 0;

        if (newjul < 0)
            newjul = 0;

        region.setriverjan(x, y, newjan);
        region.setriverjul(x, y, newjul);

        if (newjan == 0 && newjul == 0)
            region.setriverdir(x, y, 0);

        for (int i = x - 15; i <= x + 15; i++)
        {
            for (int j = y - 15; j <= y + 15; j++)
            {
                if (i >= 0 && i <= rwidth && j >= 0 && j <= rheight)
                {
                    if (fakesourcex[i][j] == x && fakesourcey[i][j] == y) // This is a bit of fake river that is based on the river point we're deleting!
                    {
                        int newfakejan = region.fakejan(i, j) - janflow;
                        int newfakejul = region.fakejul(i, j) - julflow;

                        if (newfakejan < 0)
                            newfakejan = 0;

                        if (newfakejul < 0)
                            newfakejul = 0;

                        region.setfakejan(i, j, newfakejan);
                        region.setfakejul(i, j, newfakejul);

                        if (newfakejan == 0 && newfakejul == 0)
                            region.setfakedir(i, j, 0);
                    }
                }
            }
        }

        if (dir == 8 || dir == 1 || dir == 2)
            y--;

        if (dir == 4 || dir == 5 || dir == 6)
            y++;

        if (dir == 2 || dir == 3 || dir == 4)
            x++;

        if (dir == 6 || dir == 7 || dir == 8)
            x--;

        if (x<0 || x>rwidth || y<0 || y>rheight)
            return;

        if (region.sea(x, y) == 1)
            return;

        if (region.truelake(x, y) == 1)
            return;
    }
}

// This function finds how many surrounding sea squares there are (on the regional map).

int findsurroundingsea(region& region, int x, int y)
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    int found = 0;

    for (int i = x - 1; i <= x + 1; i++)
    {
        for (int j = y - 1; j <= y + 1; j++)
        {
            if (i >= 0 && i <= rwidth && j >= 0 && j <= rheight)
            {
                if (region.sea(i, j) == 1)
                    found++;
            }
        }
    }
    return found;
}

// This function tells us if there are any rivers flowing into the square in question from the land.

int findinflowinglandriver(region& region, int x, int y)
{
    if (x < 1 || x >= region.rwidth() || y < 1 || y >= region.rheight())
        return 0;

    if (region.riverdir(x, y - 1) == 5 && region.sea(x, y - 1) == 0)
        return 1;

    if (region.riverdir(x + 1, y - 1) == 6 && region.sea(x + 1, y - 1) == 0)
        return 1;

    if (region.riverdir(x + 1, y) == 7 && region.sea(x + 1, y) == 0)
        return 1;

    if (region.riverdir(x + 1, y + 1) == 8 && region.sea(x + 1, y + 1) == 0)
        return 1;

    if (region.riverdir(x, y + 1) == 1 && region.sea(x, y + 1) == 0)
        return 1;

    if (region.riverdir(x - 1, y + 1) == 2 && region.sea(x - 1, y + 1) == 0)
        return 1;

    if (region.riverdir(x - 1, y) == 3 && region.sea(x - 1, y) == 0)
        return 1;

    if (region.riverdir(x - 1, y - 1) == 4 && region.sea(x - 1, y - 1) == 0)
        return 1;

    return 0;
}

// This checks to see whether there is a regional river (going under sea level) on the current pixel or those next to it.

int checknearbyriver(region& region, int x, int y)
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    for (int i = x - 1; i <= x + 1; i++)
    {
        for (int j = y - 1; j <= y + 1; j++)
        {
            if (i >= 0 && i <= rwidth && j >= 0 && j <= rheight)
            {
                if (region.riverjan(i, j) != 0 || region.riverjul(i, j) != 0)
                {
                    if (region.sea(i, j) == 1)
                        return 1;
                }
            }
        }
    }
    return 0;
}

// This function removes rivers on the regional map that come out of lakes and then return to the same lakes.

void removeriverlakeloops(planet& world, region& region, int dx, int dy, int sx, int sy, boolshapetemplate smalllake[])
{
    int width = world.width();
    int height = world.height();

    int rstart = region.rstart();
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    int maxtilemoves = 1; // Can't more from one tile to another more often than this.

    twointegers destpoint;

    fast_srand((sy * width + sx) + world.nom(sx, sy) + world.julrain(sx, sy));

    for (int i = dx; i <= dx + 16; i++)
    {
        int origx = i / 16;

        for (int j = dy; j <= dy + 16; j++)
        {
            if (region.riverdir(i, j) != 0)
            {
                if (region.truelake(i, j) == 1)
                {
                    destpoint = getregionalflowdestination(region, i, j, 0);

                    if (region.truelake(destpoint.x, destpoint.y) == 0) // So if this is a river on a lake cell that's flowing into a non-lake cell
                    {
                        int origy = j / 16; // Tile we start in (doesn't correspond to coordinates in global map)

                        int surfacelevel = region.lakesurface(i, j);

                        int movedtile = 0;
                        bool movedoutoflake = 0;

                        int startx = i;
                        int starty = j;
                        int endx = -1;
                        int endy = -1;

                        int x = i;
                        int y = j;

                        int currentx = origx;
                        int currenty = origy;

                        bool keepgoing = 1;
                        bool deleteriver = 0;

                        int crount = 0;

                        do
                        {
                            crount++;

                            int dir = region.riverdir(x, y);

                            if (dir == 0)
                                keepgoing = 0;
                            else
                            {
                                if (dir == 8 || dir == 1 || dir == 2)
                                    y--;

                                if (dir == 4 || dir == 5 || dir == 6)
                                    y++;

                                if (dir == 2 || dir == 3 || dir == 4)
                                    x++;

                                if (dir == 6 || dir == 7 || dir == 8)
                                    x--;

                                if (x / 16 != currentx || y / 16 != currenty)
                                {
                                    currentx = x / 16;
                                    currenty = y / 16;

                                    movedtile++;

                                    if (movedtile > maxtilemoves)
                                        keepgoing = 0;
                                }

                                if (x >= rstart && x <= rwidth && y >= rstart && y <= rheight)
                                {
                                    if (region.truelake(x, y) == 0)
                                        movedoutoflake = 1;

                                    if (region.lakesurface(x, y) == surfacelevel && movedoutoflake == 1)
                                    {
                                        deleteriver = 1;
                                        endx = x;
                                        endy = y;
                                        keepgoing = 0;
                                    }

                                    if (region.sea(x, y) == 1)
                                        keepgoing = 0;

                                }
                                else
                                    keepgoing = 0;

                                if (x == startx && y == starty)
                                    keepgoing = 0;
                            }
                        } while (keepgoing == 1 && crount < 200);

                        if (deleteriver == 1) // This river came out of a lake and then went back into the same lake!
                        {
                            x = startx;
                            y = starty;

                            int janload = region.riverjan(x, y);
                            int julload = region.riverjul(x, y);

                            int biglakesurface = 0; // If this isn't 0 then we're dealing with a big lake, and we will want to turn this river into lake rather than delete it.
                            int special = 0;
                            int depth = 0;

                            for (int k = sx - 1; k <= sx + 1; k++)
                            {
                                int kk = k;

                                if (kk<0 || kk>width)
                                    kk = wrap(kk, width);

                                for (int l = sy - 1; l <= sy + 1; l++)
                                {
                                    if (l >= 0 && l <= height)
                                    {
                                        if (world.lakesurface(k, l) != 0)
                                        {
                                            biglakesurface = world.lakesurface(k, l);
                                            special = world.special(k, l);
                                            depth = world.nom(k, l);

                                            k = sx + 1;
                                            l = sy + 1;
                                        }
                                    }
                                }
                            }

                            do
                            {
                                int dir = region.riverdir(x, y);

                                if (biglakesurface == 0) // If it's looping in a small lake, delete the river altogether
                                {
                                    region.setriverjan(x, y, region.riverjan(x, y) - janload);
                                    region.setriverjul(x, y, region.riverjul(x, y) - julload);

                                    if (region.riverjan(x, y) == 0 && region.riverjul(x, y) == 0)
                                        region.setriverdir(x, y, 0);

                                }
                                else // If it's looping in a large lake, turn the river into new lake!
                                {

                                    disruptlakecoastline(world, region, dx, dy, x, y, biglakesurface, depth, 0, 0, 0, special, smalllake);

                                }

                                if (dir == 0)
                                {
                                    x = endx;
                                    y = endy;
                                }

                                if (dir == 8 || dir == 1 || dir == 2)
                                    y--;

                                if (dir == 4 || dir == 5 || dir == 6)
                                    y++;

                                if (dir == 2 || dir == 3 || dir == 4)
                                    x++;

                                if (dir == 6 || dir == 7 || dir == 8)
                                    x--;

                            } while (x != endx || y != endy);
                        }
                    }
                }
            }
        }
    }
}

// This adds mountains to tiles on the regional map.

void makemountaintile(planet& world, region& region, int dx, int dy, int sx, int sy, peaktemplate& peaks, vector<vector<int>>& rmountainmap, vector<vector<int>>& ridgeids, short markgap, vector<vector<int>>& warptox, vector<vector<int>>& warptoy)
{
    if (world.mountainheight(sx, sy) == 0)
        return;

    int width = world.width();
    int sealevel = world.sealevel();

    int mountainheight = world.mountainheight(sx, sy);

    twofloats pt, mm1, mm2, mm3;

    int maxvar = 4; // Maximum amount the central junction can be offset by.
    int maxmidvar = 2; // Maximum amount the midpoints between the junction and the sides can be offset by.
    int minpoint = 4; // Minimum distance the points on the sides (where ranges go off the tile) can be from the corners. (This didn't work, so this isn't actually implemented. They are always on the corners.)

    // First, get coordinates for the tiles around this one.

    int eax = sx + 1;
    int eay = sy;
    if (eax > width)
        eax = 0;

    int wex = sx - 1;
    int wey = sy;
    if (wex < 0)
        wex = width;

    int sox = sx;
    int soy = sy + 1;

    int nox = sx;
    int noy = sy - 1;

    int sex = eax;
    int sey = soy;

    int nwx = wex;
    int nwy = noy;

    int nex = eax;
    int ney = noy;

    int swx = wex;
    int swy = soy;

    int north = 0;
    int south = 0;
    int east = 0;
    int west = 0;
    int northeast = 0;
    int northwest = 0;
    int southeast = 0;
    int southwest = 0;

    // Now check to see which directions the ridges go in.

    if (getridge(world, sx, sy, 1) == 1) // North
        north = world.mountainheight(nox, noy) + sealevel;

    if (getridge(world, sx, sy, 2) == 1) // Northeast
        northeast = world.mountainheight(nex, ney) + sealevel;

    if (getridge(world, sx, sy, 3) == 1) // East
        east = world.mountainheight(eax, eay) + sealevel;

    if (getridge(world, sx, sy, 4) == 1) // Southeast
        southeast = world.mountainheight(sex, sey) + sealevel;

    if (getridge(world, sx, sy, 5) == 1) // South
        south = world.mountainheight(sox, soy) + sealevel;

    if (getridge(world, sx, sy, 6) == 1) // Southwest
        southwest = world.mountainheight(swx, swy) + sealevel;

    if (getridge(world, sx, sy, 7) == 1) // West
        west = world.mountainheight(wex, wey) + sealevel;

    if (getridge(world, sx, sy, 8) == 1) // Northwest
        northwest = world.mountainheight(nwx, nwy) + sealevel;

    fast_srand((sy * width + sx) + world.nom(sx, sy) + world.julrain(sx, sy));

    int junctionpointx = dx + 8 + randomsign(random(0, maxvar));
    int junctionpointy = dy + 8 + randomsign(random(0, maxvar));

    int nseed = (sy * width + sx) + world.nom(sx, sy) + world.julrain(sx, sy) + mountainheight; // Add the mountain height here so it isn't exactly the same as the western edge seed (which would make for weirdly symmetrical mountain ranges).
    int sseed = (soy * width + sox) + world.nom(sox, soy) + world.julrain(sox, soy) + world.mountainheight(sox, soy);
    int wseed = (sy * width + sx) + world.nom(sx, sy) + world.julrain(sx, sy);
    int eseed = (eay * width + eax) + world.nom(eax, eay) + world.julrain(eax, eay);

    // Now get coordinates for the points where ridges will appear into or disappear out of the tile.
    // We also get coordinates for the midpoints in between those and the central junction point.

    fast_srand(nseed);

    int northpointx = random(dx + minpoint, dx + 16 - minpoint);
    int northpointy = dy;

    int northmidpointx = (junctionpointx + northpointx) / 2 + randomsign(random(0, maxmidvar));
    int northmidpointy = (junctionpointy + northpointy) / 2 + randomsign(random(0, maxmidvar));

    fast_srand(sseed);

    int southpointx = random(dx + minpoint, dx + 16 - minpoint);
    int southpointy = dy + 16;

    int southmidpointx = (junctionpointx + southpointx) / 2 + randomsign(random(0, maxmidvar));
    int southmidpointy = (junctionpointy + southpointy) / 2 + randomsign(random(0, maxmidvar));

    fast_srand(wseed);

    int westpointx = dx;
    int westpointy = random(dy + minpoint, dy + 16 - minpoint);

    int westmidpointx = (junctionpointx + westpointx) / 2 + randomsign(random(0, maxmidvar));
    int westmidpointy = (junctionpointy + westpointy) / 2 + randomsign(random(0, maxmidvar));

    fast_srand(eseed);

    int eastpointx = dx + 16;
    int eastpointy = random(dy + minpoint, dy + 16 - minpoint);

    int eastmidpointx = (junctionpointx + eastpointx) / 2 + randomsign(random(0, maxmidvar));
    int eastmidpointy = (junctionpointy + eastpointy) / 2 + randomsign(random(0, maxmidvar));

    int northeastpointx = dx + 16;
    int northeastpointy = dy;

    int northeastmidpointx = (junctionpointx + northeastpointx) / 2 + randomsign(random(0, maxmidvar));
    int northeastmidpointy = (junctionpointy + northeastpointy) / 2 + randomsign(random(0, maxmidvar));

    int northwestpointx = dx;
    int northwestpointy = dy;

    int northwestmidpointx = (junctionpointx + northwestpointx) / 2 + randomsign(random(0, maxmidvar));
    int northwestmidpointy = (junctionpointy + northwestpointy) / 2 + randomsign(random(0, maxmidvar));

    int southeastpointx = dx + 16;
    int southeastpointy = dy + 16;

    int southeastmidpointx = (junctionpointx + southeastpointx) / 2 + randomsign(random(0, maxmidvar));
    int southeastmidpointy = (junctionpointy + southeastpointy) / 2 + randomsign(random(0, maxmidvar));

    int southwestpointx = dx;
    int southwestpointy = dy + 16;

    int southwestmidpointx = (junctionpointx + southwestpointx) / 2 + randomsign(random(0, maxmidvar));
    int southwestmidpointy = (junctionpointy + southwestpointy) / 2 + randomsign(random(0, maxmidvar));

    fast_srand((sy * width + sx) + world.nom(sx, sy) + world.julrain(sx, sy));

    // Now draw lines from the central junction to the points on the edge where the ridges go.

    mm1.x = (float) junctionpointx;
    mm1.y = (float) junctionpointy;

    char ridgedir = 0;

    if (north != 0)
    {
        ridgedir = 1;

        int destheight = world.mountainheight(nox, noy);
        int midheight = (mountainheight + destheight) / 2;

        mm2.x = (float) northmidpointx;
        mm2.y = (float) northmidpointy;

        mm3.x = (float) northpointx;
        mm3.y = (float) northpointy;

        calculateridges(world, region, sx, sy, dx, dy, pt, mm1, mm2, mm3, mountainheight, midheight, destheight, peaks, rmountainmap, ridgedir, ridgeids, markgap, warptox, warptoy);
    }

    if (south != 0)
    {
        ridgedir = 5;

        int destheight = world.mountainheight(sox, soy);
        int midheight = (mountainheight + destheight) / 2;

        mm2.x = (float) southmidpointx;
        mm2.y = (float) southmidpointy;

        mm3.x = (float) southpointx;
        mm3.y = (float) southpointy;

        calculateridges(world, region, sx, sy, dx, dy, pt, mm1, mm2, mm3, mountainheight, midheight, destheight, peaks, rmountainmap, ridgedir, ridgeids, markgap, warptox, warptoy);
    }

    if (west != 0)
    {
        ridgedir = 7;

        int destheight = world.mountainheight(wex, wey);
        int midheight = (mountainheight + destheight) / 2;

        mm2.x = (float) westmidpointx;
        mm2.y = (float) westmidpointy;

        mm3.x = (float) westpointx;
        mm3.y = (float) westpointy;

        calculateridges(world, region, sx, sy, dx, dy, pt, mm1, mm2, mm3, mountainheight, midheight, destheight, peaks, rmountainmap, ridgedir, ridgeids, markgap, warptox, warptoy);
    }

    if (east != 0)
    {
        ridgedir = 3;

        int destheight = world.mountainheight(eax, eay);
        int midheight = (mountainheight + destheight) / 2;

        mm2.x = (float) eastmidpointx;
        mm2.y = (float) eastmidpointy;

        mm3.x = (float) eastpointx;
        mm3.y = (float) eastpointy;

        calculateridges(world, region, sx, sy, dx, dy, pt, mm1, mm2, mm3, mountainheight, midheight, destheight, peaks, rmountainmap, ridgedir, ridgeids, markgap, warptox, warptoy);
    }

    if (northeast != 0)
    {
        ridgedir = 2;

        int destheight = world.mountainheight(nex, ney);
        int midheight = (mountainheight + destheight) / 2;

        mm2.x = (float) northeastmidpointx;
        mm2.y = (float) northeastmidpointy;

        mm3.x = (float) northeastpointx;
        mm3.y = (float) northeastpointy;

        calculateridges(world, region, sx, sy, dx, dy, pt, mm1, mm2, mm3, mountainheight, midheight, destheight, peaks, rmountainmap, ridgedir, ridgeids, markgap, warptox, warptoy);
    }

    if (northwest != 0)
    {
        ridgedir = 8;

        int destheight = world.mountainheight(nwx, nwy);
        int midheight = (mountainheight + destheight) / 2;

        mm2.x = (float) northwestmidpointx;
        mm2.y = (float) northwestmidpointy;

        mm3.x = (float) northwestpointx;
        mm3.y = (float) northwestpointy;

        calculateridges(world, region, sx, sy, dx, dy, pt, mm1, mm2, mm3, mountainheight, midheight, destheight, peaks, rmountainmap, ridgedir, ridgeids, markgap, warptox, warptoy);
    }

    if (southeast != 0)
    {
        ridgedir = 4;

        int destheight = world.mountainheight(sex, sey);
        int midheight = (mountainheight + destheight) / 2;

        mm2.x = (float) southeastmidpointx;
        mm2.y = (float) southeastmidpointy;

        mm3.x = (float) southeastpointx;
        mm3.y = (float) southeastpointy;

        calculateridges(world, region, sx, sy, dx, dy, pt, mm1, mm2, mm3, mountainheight, midheight, destheight, peaks, rmountainmap, ridgedir, ridgeids, markgap, warptox, warptoy);
    }

    if (southwest != 0)
    {
        ridgedir = 6;

        int destheight = world.mountainheight(swx, swy);
        int midheight = (mountainheight + destheight) / 2;

        mm2.x = (float) southwestmidpointx;
        mm2.y = (float) southwestmidpointy;

        mm3.x = (float) southwestpointx;
        mm3.y = (float) southwestpointy;

        calculateridges(world, region, sx, sy, dx, dy, pt, mm1, mm2, mm3, mountainheight, midheight, destheight, peaks, rmountainmap, ridgedir, ridgeids, markgap, warptox, warptoy);
    }
}

// This function calculates and draws the mountain ridges on the regional map.

void calculateridges(planet& world, region& region, int dx, int dy, int sx, int sy, twofloats pt, twofloats mm1, twofloats mm2, twofloats mm3, int newheight, int midheight, int destheight, peaktemplate& peaks, vector<vector<int>>& rmountainmap, int ridgedir, vector<vector<int>>& ridgeids, short markgap, vector<vector<int>>& warptox, vector<vector<int>>& warptoy)
{
    // First, adjust the start and end points using the warp arrays, so that the mountain range looks a little more natural.
    // (Turned off for now as it led to problems.)

    /*
    if (world.island(sx, sy) == 0 && world.mountainisland(sx, sy) == 0) // Don't do this on small islands, as the mountains there would look too weird shifted away.
    {
        int x1 = (int)mm1.x;
        int y1 = (int)mm1.y;

        int x2 = (int)mm2.x;
        int y2 = (int)mm2.y;

        int x3 = (int)mm3.x;
        int y3 = (int)mm3.y;

        int newx1 = (x1*3 + warptox[x1][y1]) / 4; // Reduce the amount of warp, because otherwise the mountains won't match where the rivers and high ground are.
        int newy1 = (y1*3 + warptoy[x1][y1]) / 4;

        int newx2 = (x2*3 + warptox[x2][y2]) / 4;
        int newy2 = (y2*3 + warptoy[x2][y2]) / 4;

        int newx3 = (x3*3 + warptox[x3][y3]) / 4;
        int newy3 = (y3*3 + warptoy[x3][y3]) / 4;

        mm1.x = (float)newx1;
        mm1.y = (float)newy1;

        mm2.x = (float)newx2;
        mm2.y = (float)newy2;

        mm3.x = (float)newx3;
        mm3.y = (float)newy3;
    }
    */
    
    short markcount = markgap / (random(2, 6));

    int peakheight = 0;

    int lastx = (int)mm1.x;
    int lasty = (int)mm1.y; // Coordinates of last point done.

    for (int n = 1; n <= 2; n++) // Two halves of the curve.
    {
        for (float t = 0.0f; t <= 1.0f; t = t + 0.01f)
        {
            if (n == 1)
                pt = curvepos(mm1, mm1, mm2, mm3, t);
            else
                pt = curvepos(mm1, mm2, mm3, mm3, t);

            int x = (int)pt.x;
            int y = (int)pt.y;

            if (x != lastx || y != lasty)
            {
                markcount++;

                if (markcount == markgap)
                {
                    ridgeids[x][y] = ridgedir;
                    markcount = 0;
                }

                lastx = x;
                lasty = y;
            }

            if (newheight < midheight)
                peakheight = random(newheight, midheight);
            else
                peakheight = random(midheight, newheight);

            drawpeak(world, region, sx, sy, dx, dy, x, y, peakheight, peaks, rmountainmap, 0);
        }
    }
}

// This function works out which points on the regional map are closest to which mountain ridges.

void assignridgeregions(planet& world, region& region, int dx, int dy, int sx, int sy, vector<vector<int>>& rmountainmap, vector<vector<int>>& ridgeids, vector<vector<int>>& nearestridgepointdist, vector<vector<int>>& nearestridgepointx, vector<vector<int>>& nearestridgepointy, int smallermaxdist, int maxdist)
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();
    float gravity = world.gravity();

    float fmaxdist = (float)maxdist;

    float gravityfactor = 1.0f;

    if (gravity < 1.0f) // The lower the gravity, the further out the regions can extend.
    {
        float diff = 1.0f - gravity;
        
        gravityfactor = gravityfactor + diff * 3;
    }

    if (gravity > 1.0f) // The higher the gravity, the less far they can extend.
    {
        float diff = gravity - 1.0f;

        gravityfactor = gravityfactor - diff / 3.0f;

        if (gravityfactor < 0.2f)
            gravityfactor = 0.2f;
    }

    fmaxdist = fmaxdist * gravityfactor;

    maxdist = (int)fmaxdist;

    if (maxdist < 1)
        maxdist = 1;

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (rmountainmap[i][j] == 0) // Only check cells that don't already have peaks drawn on them.
            {
                nearestridgepointdist[i][j] = maxdist + 1;

                for (int k = dx - smallermaxdist - 1; k <= dx + 16 + smallermaxdist + 1; k++)
                {
                    for (int l = dy - smallermaxdist - 1; l <= dy + 16 + smallermaxdist + 1; l++)
                    {
                        if (k > 0 && k < rwidth && l>0 && l < rheight && ridgeids[k][l] != 0) // There's a point on a ridge here.
                        {
                            int xdiff = i - k;
                            int ydiff = j - l;

                            int thisdist = xdiff * xdiff + ydiff * ydiff;

                            if (thisdist < nearestridgepointdist[i][j])
                            {
                                nearestridgepointdist[i][j] = thisdist;
                                nearestridgepointx[i][j] = k;
                                nearestridgepointy[i][j] = l;
                            }
                        }
                    }
                }

                if (nearestridgepointdist[i][j] == maxdist + 1)
                    nearestridgepointdist[i][j] = 0;
            }
            else
                nearestridgepointdist[i][j] = -1; // If it's under a mountain already, the distance is -1.
        }
    }
}

// This function identifies the edges of mountainous regions, where buttresses can form.

void findmountainedges(planet& world, region& region, int dx, int dy, int sx, int sy, vector<vector<int>>& nearestridgepointdist, vector<vector<int>>& nearestridgepointx, vector<vector<int>>& nearestridgepointy, vector<vector<bool>>& mountainedges)
{
    int width = world.width();
    int height = world.height();

    bool nearmountains = 0;

    for (int i = sx - 1; i <= sx + 1; i++)
    {
        int ii = i;

        if (ii<0 || ii>width)
            ii = wrap(ii, width);

        for (int j = sy - 1; j <= sy + 1; j++)
        {
            if (j >= 0 && j <= sy)
            {
                if (world.mountainheight(i, j) != 0)
                {
                    nearmountains = 1;
                    i = sx + 1;
                    j = sy + 1;
                }
            }
        }
    }

    if (nearmountains == 0)
        return;

    int rwidth = region.rwidth();
    int rheight = region.rheight();

    for (int i = dx; i <= dy; i++)
    {
        for (int j = dx; j <= dy; j++)
        {
            if (nearestridgepointdist[i][j] > 0)
            {
                bool outeredge = 0; // If it's on the edge of the whole mountainous area
                bool inneredge = 0; // If it's at the point where one ridge's area meets another's
                bool tooclose = 0;

                for (int k = i - 1; k <= i + 1; k++)
                {
                    for (int l = j - 1; l <= j + 1; l++)
                    {
                        if (k >= 0 && k <= rwidth && l >= 0 && l <= rheight)
                        {
                            if (nearestridgepointdist[k][l] == 0)
                            {
                                outeredge = 1;
                                k = i + 1;
                                l = j + 1;
                            }
                            if (nearestridgepointdist[k][l] == -1)
                            {
                                tooclose = 1;
                                k = i + 1;
                                l = j + 1;
                            }
                        }
                    }
                }

                int furtherx = 0;
                int furthery = 0; // Coordinates of the point that's *further* from the nearest ridge point than our point is.

                if (outeredge == 0 && tooclose == 0) // Not found an edge yet. But perhaps this point is on the boundary between one ridge's area and another.
                {
                    if (nearestridgepointx[i][j] == i && nearestridgepointy[i][j] < j)
                    {
                        furtherx = i;
                        furthery = j + 1;
                    }

                    if (nearestridgepointx[i][j] > i && nearestridgepointy[i][j] < j)
                    {
                        furtherx = i - 1;
                        furthery = j + 1;
                    }

                    if (nearestridgepointx[i][j] > i && nearestridgepointy[i][j] == j)
                    {
                        furtherx = i - 1;
                        furthery = j;
                    }

                    if (nearestridgepointx[i][j] > i && nearestridgepointy[i][j] > j)
                    {
                        furtherx = i - 1;
                        furthery = j - 1;
                    }

                    if (nearestridgepointx[i][j] == i && nearestridgepointy[i][j] > j)
                    {
                        furtherx = i;
                        furthery = j - 1;
                    }

                    if (nearestridgepointx[i][j]<i && nearestridgepointy[i][j]>j)
                    {
                        furtherx = i + 1;
                        furthery = j - 1;
                    }

                    if (nearestridgepointx[i][j] < i && nearestridgepointy[i][j] == j)
                    {
                        furtherx = i + 1;
                        furthery = j;
                    }

                    if (nearestridgepointx[i][j] < i && nearestridgepointy[i][j] < j)
                    {
                        furtherx = i + 1;
                        furthery = j + 1;
                    }

                    if (furtherx >= 0 && furtherx <= rwidth && furthery >= 0 && furthery <= rheight && nearestridgepointdist[furtherx][furthery] < nearestridgepointdist[i][j])
                        inneredge = 1;
                }

                if ((outeredge == 1 || inneredge == 1) && tooclose == 0)
                {
                    mountainedges[i][j] = 1;

                    if (inneredge == 1)
                        mountainedges[furtherx][furthery] = 1;
                }
            }
        }
    }
}

// This function identifies the points where buttresses will end.

void findbuttresspoints(planet& world, region& region, int dx, int dy, int sx, int sy, vector<vector<int>>& ridgeids, vector<vector<int>>& nearestridgepointdist, vector<vector<int>>& nearestridgepointx, vector<vector<int>>& nearestridgepointy, vector<vector<bool>>& mountainedges, vector<vector<bool>>& buttresspoints, int maxdist, int spacing)
{
    int width = world.width();
    int height = world.height();

    int rwidth = region.rwidth();
    int rheight = region.rheight();

    fast_srand((sy * width + sx) + world.map(sx, sy) + world.julrain(sx, sy));

    // We go through the tile and identify the points on the main ridges that will have buttresses going from them.

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (ridgeids[i][j] != 0)
            {
                int a = i - maxdist;
                int b = i + maxdist;
                int c = 1;

                if (random(1, 2) == 1)
                {
                    a = i + maxdist;
                    b = i - maxdist;
                    c = -1;
                }

                int d = j - maxdist;
                int e = j + maxdist;
                int f = 1;

                if (random(1, 2) == 1)
                {
                    d = j + maxdist;
                    e = j - maxdist;
                    f = -1;
                }

                for (int k = a; k != b; k = k + c)
                {
                    for (int l = d; l != e; l = l + f)
                    {
                        if (k >= 0 && k <= rwidth && l >= 0 && l <= rheight)
                        {
                            if (nearestridgepointx[k][l] == i && nearestridgepointy[k][l] == j && k != i && l != j)
                            {
                                bool tooclose = 0;

                                int thisspacing = random(spacing, spacing * 2);

                                for (int m = k - thisspacing; m <= k + thisspacing; m++)
                                {
                                    for (int n = l - thisspacing; n <= l + thisspacing; n++)
                                    {
                                        if (m >= 0 && m <= rwidth && n >= 0 && n <= rheight)
                                        {
                                            if (buttresspoints[m][n] == 1 && nearestridgepointx[m][n] == i && nearestridgepointy[m][n] == j)
                                            {
                                                tooclose = 1;
                                                m = k + thisspacing;
                                                n = l + thisspacing;
                                            }
                                        }
                                    }
                                }

                                if (tooclose == 0)
                                    buttresspoints[k][l] = 1;

                            }
                        }
                    }
                }
            }
        }
    }
}

// This function draws the buttresses onto the mountain map.

void makebuttresses(planet& world, region& region, int dx, int dy, int sx, int sy, vector<vector<int>>& rmountainmap, peaktemplate& peaks, vector<vector<int>>& nearestridgepointx, vector<vector<int>>& nearestridgepointy, vector<vector<int>>& nearestridgepointdist, int maxdist, vector<vector<bool>>& buttresspoints, vector<vector<int>>& ridgeids, short markgap, bool minibuttresses)
{
    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();

    int rwidth = region.rwidth();
    int rheight = region.rheight();

    short markcount = markgap / 2;

    short swervelimit = 3;

    if (minibuttresses == 1)
        swervelimit = 1;

    fast_srand((sy * world.width() + sx) + world.map(sx, sy) + world.julrain(sx, sy));

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (buttresspoints[i][j] == 1 && nearestridgepointx[i][j] > 0 && nearestridgepointy[i][j] > 0)
            {
                float peakheight = (float)(rmountainmap[nearestridgepointx[i][j]][nearestridgepointy[i][j]] - sealevel) * 0.5f + (float)sealevel;

                twofloats pt, mm1, mm2, mm3;

                mm1.x = (float) i;
                mm1.y = (float) j;

                mm3.x = (float) nearestridgepointx[i][j];
                mm3.y = (float) nearestridgepointy[i][j];

                mm2.x = ((mm1.x + mm3.x) / 2.0f) + (float)randomsign(random(1, 3));
                mm2.y = ((mm1.y + mm3.y) / 2.0f) + (float)randomsign(random(1, 3));

                int lastx = (int)mm1.x;
                int lasty = (int)mm1.y; // Coordinates of last point done.

                for (int n = 1; n <= 2; n++) // Two halves of the curve.
                {
                    for (float t = 0.0f; t <= 1.0f; t = t + 0.01f)
                    {
                        if (n == 1)
                            pt = curvepos(mm1, mm1, mm2, mm3, t);
                        else
                            pt = curvepos(mm1, mm2, mm3, mm3, t);

                        int x = (int)pt.x;
                        int y = (int)pt.y;

                        if (x != lastx || y != lasty)
                        {
                            markcount++;

                            if (x >= 0 && x <= rwidth && y >= 0 && y <= rheight && markcount == markgap)
                            {
                                ridgeids[x][y] = 1;
                                markcount = 0;
                            }

                            lastx = x;
                            lasty = y;

                            if (minibuttresses == 0)
                                peakheight = (peakheight - (float)sealevel) * 1.1f + (float)sealevel;
                            else
                                peakheight = (peakheight - (float)sealevel) * 1.3f + (float)sealevel;
                        }

                        if (peakheight > (float)rmountainmap[nearestridgepointx[i][j]][nearestridgepointy[i][j]])
                            peakheight = (float)rmountainmap[nearestridgepointx[i][j]][nearestridgepointy[i][j]];


                        if (x >= 0 && x <= rwidth && y >= 0 && y <= rheight && rmountainmap[x][y] < peakheight)
                            rmountainmap[x][y] = (int)peakheight;

                        for (int k = x - 1; k <= x + 1; k++)
                        {
                            for (int l = y - 1; l <= y + 1; l++)
                            {
                                if (random(1, 10) == 1 && k >= 0 && k <= rwidth && l >= 0 && l <= rheight)
                                {
                                    float div;

                                    if (minibuttresses == 0)
                                        div = (float)random(40, 80);
                                    else
                                        div = (float)random(30, 60);

                                    div = div / 100.0f;

                                    int thisheight = (int)((peakheight - (float)sealevel) * div) + sealevel;

                                    if (rmountainmap[k][l] < thisheight)
                                        rmountainmap[k][l] = thisheight;
                                }
                            }
                        }

                        if (minibuttresses == 0)
                        {
                            for (int k = x - 2; k <= x + 2; k++)
                            {
                                for (int l = y - 2; l <= y + 2; l++)
                                {
                                    if (random(1, 10) == 1 && k >= 0 && k <= rwidth && l >= 0 && l <= rheight)
                                    {
                                        float div = (float)random(20, 40);
                                        div = div / 100.0f;

                                        int thisheight = (int)((peakheight - (float)sealevel) * div) + sealevel;

                                        if (rmountainmap[k][l] < thisheight)
                                            rmountainmap[k][l] = thisheight;
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

// This function draws a mountain peak onto the regional map.

void drawpeak(planet& world, region& region, int sx, int sy, int dx, int dy, int x, int y, int peakheight, peaktemplate& peaks, vector<vector<int>>& rmountainmap, bool buttress)
{
    if (buttress == 0 && random(1, 10) != 1) // Only draw peaks for some of the points in the line.
        return;

    int rwidth = region.rwidth();
    int rheight = region.rheight();
    float gravity = world.gravity();

    float thinheight = (-491.66666666667f * gravity * gravity) - (122535.0f * gravity) + 124526.66666667f; // The higher the gravity, the more likely we are to use thin peaks.

    if (thinheight < 2.0f)
        thinheight = 2.0f;

    float buttressthinheight = thinheight * 0.66f;

    int templateno = 1; // 1 is fat. 2 is thin. 3 is minute/speckledy. 4 is invisible!

    if (random(1, (int)thinheight) < peakheight)
        templateno = 2;

    if (buttress == 1)
    {
        if (random(1, (int)buttressthinheight) < peakheight)
            templateno = 2;
    }

    if (templateno == 2 && gravity < 1.0f) // Just to ensure wider mountains at lower gravities.
    {
        int val = (int)(gravity * 100.0f);

        if (random(1, 100) > val)
            templateno = 1;

    }

    bool leftr = random(0, 1); // If it's 1 then we reverse it left-right.
    bool downr = random(0, 1); // If it's 1 then we reverse it top-bottom.

    pastepeak(world, region, x, y, (float)peakheight, templateno, leftr, downr, peaks, rmountainmap);
}

// This function actually pastes the peak onto the regional map.

void pastepeak(planet& world, region& region, int x, int y, float peakheight, int templateno, bool leftr, bool downr, peaktemplate& peaks, vector<vector<int>>& rmountainmap)
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    int sealevel = world.sealevel();
    int maxelev = world.maxelevation();

    int span = peaks.span(templateno);
    int centrex = peaks.centrex(templateno);
    int centrey = peaks.centrey(templateno);

    span--;

    int a, b, c, d, e, f;

    if (leftr == 0)
    {
        a = 0;
        b = span;
        c = 1;
    }
    else
    {
        a = span;
        b = 0;
        c = -1;

        centrex = span - centrex;
    }

    if (downr == 0)
    {
        d = 0;
        e = span;
        f = 1;
    }
    else
    {
        d = span;
        e = 0;
        f = -1;

        centrey = span - centrey;
    }

    x = x - centrex;
    y = y - centrey; // Coordinates of the top left corner.

    // First, check that there's no river or lake in the way.

    for (int i = a; i != b; i = i + c)
    {
        for (int j = d; j != e; j = j + f)
        {
            if (peaks.peakmap(templateno, i, j) != 0)
            {
                int xx = x + i;
                int yy = y + j;

                if (xx >= 0 && xx <= rwidth && yy >= 0 && yy <= rheight)
                {
                    if (region.riverdir(xx, yy) != 0 || region.fakedir(xx, yy) != 0 || region.lakesurface(xx, yy) != 0)
                        return;
                }
            }
        }
    }

    // Now actually paste the peak on.

    int imap = -1;
    int jmap = -1;

    for (int i = a; i != b; i = i + c)
    {
        imap++;
        jmap = -1;

        for (int j = d; j != e; j = j + f)
        {
            jmap++;

            if (peaks.peakmap(templateno, i, j) != 0)
            {
                int xx = x + imap;
                int yy = y + jmap;

                if (xx >= 0 && xx <= rwidth && yy >= 0 && yy <= rheight)
                {
                    float val = (float)peaks.peakmap(templateno, i, j);

                    val = val * 0.1f;

                    float newheight = peakheight * val;

                    if (region.sea(xx, yy) == 0)
                        newheight = newheight + (float)region.map(xx, yy);
                    else
                        newheight = newheight + (float)sealevel;

                    if (newheight > (float)maxelev)
                        newheight = (float)maxelev;

                    if (rmountainmap[xx][yy] < (int)newheight)
                    {
                        rmountainmap[xx][yy] = (int)newheight; // We put it on the rmountainmap rather than direct onto region.map to ensure that bits of different peaks don't add onto each other.

                        if (region.special(xx, yy) == 110) // If there's salt pan here, remove it.
                        {
                            if (region.lakesurface(xx, yy) != 0)
                                region.setmap(xx, yy, region.lakesurface(xx, yy));
                        }
                        region.setlakesurface(xx, yy, 0);
                    }
                }
            }
        }
    }
}

// This looks at each tile of the regional map and removes any bits of sea that are not connected to the ocean.

void removepools(planet& world, region& region, int dx, int dy, int sx, int sy, vector<vector<int>>& pathchecked, int& checkno)
{
    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();

    // First check whether we're dealing with a fjord situation.

    bool fjords = 0;

    for (int i = sx - 1; i <= sx + 1; i++)
    {
        int ii = i;

        if (ii<0 || ii>width)
            ii = wrap(ii, width);

        for (int j = sy - 1; j <= sy + 1; j++)
        {
            if (j >= 0 && j <= height)
            {
                if (world.mountainheight(ii, j) != 0 && world.sea(ii, j) == 1) // This is a fjord tile
                {
                    fjords = 1;
                    i = sx + 1;
                    j = sy + 1;
                }
            }
        }
    }

    if (fjords == 0 && vaguelycoastal(world, sx, sy) == 0)
        return;

    // Now check that there's any land on here at all.

    bool landfound = 0;

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (region.sea(i, j) == 0)
            {
                landfound = 1;
                i = dx + 16;
                j = dy + 16;
            }
        }
    }

    if (landfound == 0)
        return;

    // If we're still here then it's a normal vaguely coastal tile, and we will need to deal with it using pathfinding.

    int rstart = region.rstart();
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    bool sea = 0;

    if (world.sea(sx, sy) == 1 && world.mountainheight(sx, sy) == 0)
        sea = 1;

    bool neighbours[3][3]; // This will hold which neighbouring tiles we're hoping sea cells will connect to for this tile.

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (i != 1 || j != 1)
            {
                int x = sx + i - 1;
                int y = sy + j - 1;

                int xx = x;

                if (xx<0 || xx>width)
                    xx = wrap(xx, width);

                if (sea == 0)
                {
                    if (y >= 0 && y < height)
                    {
                        if (world.sea(xx, y) == 1 && world.mountainheight(xx, y) == 0)
                            neighbours[i][j] = 1;
                        else
                            neighbours[i][j] = 0;
                    }
                    else
                        neighbours[i][j] = 0;
                }
                else // If we're on a sea tile, we want one that's entirely surrounded by sea.
                {
                    int leftx = x - 1;
                    int rightx = x + 1;
                    int upy = y - 1;
                    int downy = y + 1;

                    int lleftx = leftx;

                    if (lleftx<0 || lleftx>width)
                        lleftx = wrap(lleftx, width);

                    if (y >= 0 && y <= height)
                    {
                        bool entirelysurrounded = 1;

                        for (int k = leftx; k <= rightx; k++)
                        {
                            int kk = k;

                            if (kk<0 || kk>width)
                                kk = wrap(kk, width);

                            for (int l = upy; l <= downy; l++)
                            {
                                if (l >= 0 && l <= height)
                                {
                                    if (world.sea(kk, l) == 0 || world.mountainheight(kk, l) != 0)
                                    {
                                        entirelysurrounded = 0;
                                        k = rightx;
                                        l = downy;
                                    }
                                }
                            }
                        }

                        if (entirelysurrounded == 1)
                            neighbours[i][j] = 1;
                        else
                            neighbours[i][j] = 0;
                    }
                }
            }
        }
    }

    if (sea == 1)
    {
        int crount = 0;

        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                if (neighbours[i][j] == 1)
                    crount++;
            }
        }

        if (crount == 0) // We didn't find any!
        {
            int leftx = sx - 1;
            int rightx = sx + 1;
            int upy = sy - 1;
            int downy = sy + 1;

            if (leftx < 0)
                leftx = width;

            if (leftx > width)
                leftx = 0;

            if (upy >= 0 && downy <= height) // Make do with sea tiles (even coastal ones) if land tiles are on the other side of our tile
            {
                if (world.sea(sx, upy) == 1 && world.sea(sx, downy) == 0)
                    neighbours[1][0] = 1;

                if (world.sea(rightx, upy) == 1 && world.sea(leftx, downy) == 0)
                    neighbours[2][0] = 1;

                if (world.sea(rightx, sy) == 1 && world.sea(leftx, sy) == 0)
                    neighbours[2][1] = 1;

                if (world.sea(rightx, downy) == 1 && world.sea(leftx, upy) == 0)
                    neighbours[2][2] = 1;

                if (world.sea(sx, downy) == 1 && world.sea(sx, upy) == 0)
                    neighbours[1][2] = 1;

                if (world.sea(leftx, downy) == 1 && world.sea(rightx, upy) == 0)
                    neighbours[0][2] = 1;

                if (world.sea(leftx, sy) == 1 && world.sea(rightx, sy) == 0)
                    neighbours[0][1] = 1;

                if (world.sea(leftx, upy) == 1 && world.sea(rightx, downy) == 0)
                    neighbours[0][0] = 1;
            }
        }
    }

    // The neighbours array now tells us which neighbouring tiles we want sea cells in our tile to connect to. For each sea cell in our tile, it must connect by sea to a sea cell in at least one of those neighbouring tiles (doesn't have to with all of them, just at least one). Ideally the target sea cell in the neighbouring tile should be as far as possible to the other side of that tile.

    int recursion = 0;

    for (int fromx = dx; fromx <= dx + 16; fromx++)
    {
        for (int fromy = dy; fromy <= dy + 16; fromy++)
        {
            if (region.sea(fromx, fromy) == 1) // Bit of sea in the source tile
            {
                bool foundpath = 0;

                int destx = -1;
                int desty = -1;

                if (neighbours[1][0] == 1 && foundpath == 0) // North
                {
                    // First, get the destination cell in the neighbouring tile.

                    for (int j = dy - 15; j <= dy - 2; j++)
                    {
                        for (int i = dx + 1; i <= dx + 15; i++)
                        {
                            if (region.sea(i, j) == 1 && region.sea(i - 1, j) == 1 && region.sea(i + 1, j) == 1 && region.sea(i, j - 1) == 1 && region.sea(i, j + 1) == 1)
                            {
                                destx = i;
                                desty = j;

                                j = dy - 2;
                                i = dx + 15;
                            }
                        }
                    }

                    if (destx != -1) // If we actually found one
                    {
                        // Now we need to see whether there is a sea path from fromx,fromy to destx,desty

                        // Define the area within which we're searching.

                        int leftx = dx - 32;
                        int lefty = dy - 32;
                        int rightx = dx + 48;
                        int righty = dy + 32;

                        if (leftx < rstart)
                            leftx = rstart;

                        if (lefty < rstart)
                            lefty = rstart;

                        if (rightx > rwidth)
                            rightx = rwidth;

                        if (righty > rheight)
                            righty = rheight;

                        checkno++;

                        bool found = findpath(region, leftx, lefty, rightx, righty, fromx, fromy, destx, desty, checkno, pathchecked, recursion);

                        if (found == 1)
                        {
                            for (int i = leftx; i <= rightx; i++)
                            {
                                for (int j = lefty; j <= righty; j++)
                                    pathchecked[i][j] = checkno;
                            }

                            foundpath = 1;
                        }
                    }
                }

                if (neighbours[1][2] == 1 && foundpath == 0) // South
                {
                    // First, get the destination cell in the neighbouring tile.

                    for (int j = dy + 31; j >= dy + 17; j--)
                    {
                        for (int i = dx + 1; i <= dx + 15; i++)
                        {
                            if (region.sea(i, j) == 1 && region.sea(i - 1, j) == 1 && region.sea(i + 1, j) == 1 && region.sea(i, j - 1) == 1 && region.sea(i, j + 1) == 1)
                            {
                                destx = i;
                                desty = j;

                                j = dy + 17;
                                i = dx + 15;
                            }
                        }
                    }

                    if (destx != -1) // If we actually found one
                    {
                        // Now we need to see whether there is a sea path from fromx,fromy to destx,desty

                        // Define the area within which we're searching.

                        int leftx = dx - 32;
                        int lefty = dy - 16;
                        int rightx = dx + 48;
                        int righty = dy + 48;

                        if (leftx < rstart)
                            leftx = rstart;

                        if (lefty < rstart)
                            lefty = rstart;

                        if (rightx > rwidth)
                            rightx = rwidth;

                        if (righty > rheight)
                            righty = rheight;

                        checkno++;

                        bool found = findpath(region, leftx, lefty, rightx, righty, fromx, fromy, destx, desty, checkno, pathchecked, recursion);

                        if (found == 1)
                        {
                            for (int i = leftx; i <= rightx; i++)
                            {
                                for (int j = lefty; j <= righty; j++)
                                    pathchecked[i][j] = checkno;
                            }

                            foundpath = 1;
                        }
                    }
                }

                if (neighbours[2][1] == 1 && foundpath == 0) // East
                {
                    // First, get the destination cell in the neighbouring tile.

                    for (int i = dx + 31; i >= dx + 17; i--)
                    {
                        for (int j = dy + 1; j <= dy + 15; j++)
                        {
                            if (region.sea(i, j) == 1 && region.sea(i - 1, j) == 1 && region.sea(i + 1, j) == 1 && region.sea(i, j - 1) == 1 && region.sea(i, j + 1) == 1)
                            {
                                destx = i;
                                desty = j;

                                i = dx + 17;
                                j = dy + 15;
                            }
                        }
                    }

                    if (destx != -1) // If we actually found one
                    {
                        // Now we need to see whether there is a sea path from fromx,fromy to destx,desty

                        // Define the area within which we're searching.

                        int leftx = dx - 16;
                        int lefty = dy - 32;
                        int rightx = dx + 48;
                        int righty = dy + 48;

                        if (leftx < rstart)
                            leftx = rstart;

                        if (lefty < rstart)
                            lefty = rstart;

                        if (rightx > rwidth)
                            rightx = rwidth;

                        if (righty > rheight)
                            righty = rheight;

                        checkno++;

                        bool found = findpath(region, leftx, lefty, rightx, righty, fromx, fromy, destx, desty, checkno, pathchecked, recursion);

                        if (found == 1)
                        {
                            for (int i = leftx; i <= rightx; i++)
                            {
                                for (int j = lefty; j <= righty; j++)
                                    pathchecked[i][j] = checkno;
                            }

                            foundpath = 1;
                        }
                    }
                }

                if (neighbours[0][1] == 1 && foundpath == 0) // West
                {
                    // First, get the destination cell in the neighbouring tile.

                    for (int i = dx - 15; i <= dx - 2; i++)
                    {
                        for (int j = dy + 1; j <= dy + 15; j++)
                        {
                            if (region.sea(i, j) == 1 && region.sea(i - 1, j) == 1 && region.sea(i + 1, j) == 1 && region.sea(i, j - 1) == 1 && region.sea(i, j + 1) == 1)
                            {
                                destx = i;
                                desty = j;

                                i = dx - 2;
                                j = dy + 15;
                            }
                        }
                    }

                    if (destx != -1) // If we actually found one
                    {
                        // Now we need to see whether there is a sea path from fromx,fromy to destx,desty

                        // Define the area within which we're searching.

                        int leftx = dx - 32;
                        int lefty = dy - 32;
                        int rightx = dx + 32;
                        int righty = dy + 48;

                        if (leftx < rstart)
                            leftx = rstart;

                        if (lefty < rstart)
                            lefty = rstart;

                        if (rightx > rwidth)
                            rightx = rwidth;

                        if (righty > rheight)
                            righty = rheight;

                        checkno++;

                        bool found = findpath(region, leftx, lefty, rightx, righty, fromx, fromy, destx, desty, checkno, pathchecked, recursion);

                        if (found == 1)
                        {
                            for (int i = leftx; i <= rightx; i++)
                            {
                                for (int j = lefty; j <= righty; j++)
                                    pathchecked[i][j] = checkno;
                            }

                            foundpath = 1;
                        }
                    }
                }

                if (neighbours[0][0] == 1 && foundpath == 0) // Northwest
                {
                    // First, get the destination cell in the neighbouring tile.

                    for (int j = dy - 15; j <= dy - 2; j++)
                    {
                        for (int i = dx - 15; i <= dx - 2; i++)
                        {
                            if (region.sea(i, j) == 1 && region.sea(i - 1, j) == 1 && region.sea(i + 1, j) == 1 && region.sea(i, j - 1) == 1 && region.sea(i, j + 1) == 1)
                            {
                                destx = i;
                                desty = j;

                                j = dy - 2;
                                i = dx - 2;
                            }
                        }
                    }

                    if (destx != -1) // If we actually found one
                    {
                        // Now we need to see whether there is a sea path from fromx,fromy to destx,desty

                        // Define the area within which we're searching.

                        int leftx = dx - 32;
                        int lefty = dy - 32;
                        int rightx = dx + 32;
                        int righty = dy + 32;

                        if (leftx < rstart)
                            leftx = rstart;

                        if (lefty < rstart)
                            lefty = rstart;

                        if (rightx > rwidth)
                            rightx = rwidth;

                        if (righty > rheight)
                            righty = rheight;

                        checkno++;

                        bool found = findpath(region, leftx, lefty, rightx, righty, fromx, fromy, destx, desty, checkno, pathchecked, recursion);

                        if (found == 1)
                        {
                            for (int i = leftx; i <= rightx; i++)
                            {
                                for (int j = lefty; j <= righty; j++)
                                    pathchecked[i][j] = checkno;
                            }

                            foundpath = 1;
                        }
                    }
                }

                if (neighbours[2][0] == 1 && foundpath == 0) // Northeast
                {
                    // First, get the destination cell in the neighbouring tile.

                    for (int j = dy - 15; j <= dy - 2; j++)
                    {
                        for (int i = dx + 31; i >= dx + 17; i--)
                        {
                            if (region.sea(i, j) == 1 && region.sea(i - 1, j) == 1 && region.sea(i + 1, j) == 1 && region.sea(i, j - 1) == 1 && region.sea(i, j + 1) == 1)
                            {
                                destx = i;
                                desty = j;

                                j = dy - 2;
                                i = dx + 17;
                            }
                        }
                    }

                    if (destx != -1) // If we actually found one
                    {
                        // Now we need to see whether there is a sea path from fromx,fromy to destx,desty

                        // Define the area within which we're searching.

                        int leftx = dx - 16;
                        int lefty = dy - 32;
                        int rightx = dx + 48;
                        int righty = dy + 32;

                        if (leftx < rstart)
                            leftx = rstart;

                        if (lefty < rstart)
                            lefty = rstart;

                        if (rightx > rwidth)
                            rightx = rwidth;

                        if (righty > rheight)
                            righty = rheight;

                        checkno++;

                        bool found = findpath(region, leftx, lefty, rightx, righty, fromx, fromy, destx, desty, checkno, pathchecked, recursion);

                        if (found == 1)
                        {
                            for (int i = leftx; i <= rightx; i++)
                            {
                                for (int j = lefty; j <= righty; j++)
                                    pathchecked[i][j] = checkno;
                            }

                            foundpath = 1;
                        }
                    }
                }

                if (neighbours[0][2] == 1 && foundpath == 0) // Southwest
                {
                    // First, get the destination cell in the neighbouring tile.

                    for (int j = dy + 31; j >= dy + 17; j--)
                    {
                        for (int i = dx - 15; i <= dx - 2; i++)
                        {
                            if (region.sea(i, j) == 1 && region.sea(i - 1, j) == 1 && region.sea(i + 1, j) == 1 && region.sea(i, j - 1) == 1 && region.sea(i, j + 1) == 1)
                            {
                                destx = i;
                                desty = j;

                                j = dy + 17;
                                i = dx - 2;
                            }
                        }
                    }

                    if (destx != -1) // If we actually found one
                    {
                        // Now we need to see whether there is a sea path from fromx,fromy to destx,desty

                        // Define the area within which we're searching.

                        int leftx = dx - 32;
                        int lefty = dy - 16;
                        int rightx = dx + 32;
                        int righty = dy + 48;

                        if (leftx < rstart)
                            leftx = rstart;

                        if (lefty < rstart)
                            lefty = rstart;

                        if (rightx > rwidth)
                            rightx = rwidth;

                        if (righty > rheight)
                            righty = rheight;

                        checkno++;

                        bool found = findpath(region, leftx, lefty, rightx, righty, fromx, fromy, destx, desty, checkno, pathchecked, recursion);

                        if (found == 1)
                        {
                            for (int i = leftx; i <= rightx; i++)
                            {
                                for (int j = lefty; j <= righty; j++)
                                    pathchecked[i][j] = checkno;
                            }

                            foundpath = 1;
                        }
                    }
                }

                if (neighbours[2][2] == 1 && foundpath == 0) // Southeast
                {
                    // First, get the destination cell in the neighbouring tile.

                    for (int j = dy + 31; j >= dy + 17; j--)
                    {
                        for (int i = dx + 17; i <= dx + 31; i++)
                        {
                            if (region.sea(i, j) == 1 && region.sea(i - 1, j) == 1 && region.sea(i + 1, j) == 1 && region.sea(i, j - 1) == 1 && region.sea(i, j + 1) == 1)
                            {
                                destx = i;
                                desty = j;

                                j = dy + 17;
                                i = dx + 31;
                            }
                        }
                    }

                    if (destx != -1) // If we actually found one
                    {
                        // Now we need to see whether there is a sea path from fromx,fromy to destx,desty

                        // Define the area within which we're searching.

                        int leftx = dx - 16;
                        int lefty = dy - 16;
                        int rightx = dx + 48;
                        int righty = dy + 48;

                        if (leftx < rstart)
                            leftx = rstart;

                        if (lefty < rstart)
                            lefty = rstart;

                        if (rightx > rwidth)
                            rightx = rwidth;

                        if (righty > rheight)
                            righty = rheight;

                        checkno++;

                        int recursion = 0;

                        bool found = findpath(region, leftx, lefty, rightx, righty, fromx, fromy, destx, desty, checkno, pathchecked, recursion);

                        if (found == 1)
                        {
                            for (int i = leftx; i <= rightx; i++)
                            {
                                for (int j = lefty; j <= righty; j++)
                                    pathchecked[i][j] = checkno;
                            }

                            foundpath = 1;
                        }
                    }
                }

                if (destx != -1)
                {
                    if (foundpath == 0) // We couldn't find any path from the sea in this tile to the sea in the neighbouring tiles, so get rid of it.
                    {
                        int newelev = world.maxelevation();

                        for (int i = dx; i <= dx + 16; i++) // Get the lowest existing land height in this cell to be the new height for these ones.
                        {
                            for (int j = dy; j <= dy + 16; j++)
                            {
                                if (region.map(i, j) > sealevel && region.map(i, j) < newelev)
                                    newelev = region.map(i, j);
                            }
                        }

                        for (int i = dx - 16; i <= dx + 16; i++) // All cells that were examined in this pass must be unconnected to the target, so they can all be turned into land.
                        {
                            for (int j = dy - 16; j <= dy + 16; j++)
                            {
                                if (pathchecked[i][j] == checkno)
                                {
                                    region.setmap(i, j, newelev);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// This finds whether one point of sea is connected to another, by sea.

bool findpath(region& region, int& leftx, int& lefty, int& rightx, int& righty, int& currentx, int& currenty, int& destx, int& desty, int& checkno, vector<vector<int>>& pathchecked, int& recursion)
{
    if (currentx<leftx || currenty<lefty || currentx>rightx || currenty>righty) // We've gone out of the search zone
        return 0;

    recursion++;

    if (recursion > 3000)
        return 0;

    int rstart = region.rstart();
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    if (currentx<rstart + 1 || currentx>rwidth - 1 || currenty<rstart + 1 || currenty>rheight - 1) // If we're too close to the edge of the map
    {
        recursion--;
        return 0;
    }

    if (currentx == destx && currenty == desty) // If we've reached the destination
    {
        recursion--;
        return 1;
    }

    if (region.sea(currentx, currenty) == 0)
    {
        recursion--;
        return 0;
    }

    if (pathchecked[currentx][currenty] == checkno)
    {
        recursion--;
        return 0;
    }

    pathchecked[currentx][currenty] = checkno;

    bool n = 0;
    bool nn = 0;

    // First, look to see roughly which direction the target is.

    short searchdir = 0;

    int northdist = currenty - desty;
    int eastdist = destx - currentx;

    if (northdist > 0 && eastdist > 0) // Going roughly northeast
    {
        if (northdist > eastdist)
            searchdir = 2;
        else
            searchdir = 4;
    }

    if (northdist <= 0 && eastdist > 0) // Going roughly southeast
    {
        if (abs(northdist) > eastdist)
            searchdir = 8;
        else
            searchdir = 6;
    }

    if (northdist <= 0 && eastdist <= 0) // Going roughly southwest
    {
        if (abs(northdist) > abs(eastdist))
            searchdir = 10;
        else
            searchdir = 12;
    }

    if (northdist > 0 && eastdist <= 0) // Going roughly northwest
    {
        if (northdist > abs(eastdist))
            searchdir = 16;
        else
            searchdir = 14;
    }

    // Now move closer towards the target, hopefully.

    if (searchdir == 2) // If we're looking roughly to the north-northeast
    {
        // Look to the north.

        if (n == 0)
        {
            int newx = currentx;
            int newy = currenty - 1;

            if (pathchecked[newx][newy] != checkno && region.sea(newx, newy) == 1)
            {
                nn = findpath(region, leftx, lefty, rightx, righty, newx, newy, destx, desty, checkno, pathchecked, recursion);
                pathchecked[newx][newy] = checkno;

                if (nn == 0)
                    n = 0;
                else
                    n = 1;
            }
        }

        // Look to the east.

        if (n == 0)
        {
            int newx = currentx + 1;
            int newy = currenty;

            if (pathchecked[newx][newy] != checkno && region.sea(newx, newy) == 1)
            {
                nn = findpath(region, leftx, lefty, rightx, righty, newx, newy, destx, desty, checkno, pathchecked, recursion);
                pathchecked[newx][newy] = checkno;

                if (nn == 0)
                    n = 0;
                else
                    n = 1;
            }
        }

        // Look to the west.

        if (n == 0)
        {
            int newx = currentx - 1;
            int newy = currenty;

            if (pathchecked[newx][newy] != checkno && region.sea(newx, newy) == 1)
            {
                nn = findpath(region, leftx, lefty, rightx, righty, newx, newy, destx, desty, checkno, pathchecked, recursion);
                pathchecked[newx][newy] = checkno;

                if (nn == 0)
                    n = 0;
                else
                    n = 1;
            }
        }

        // Look to the south.

        if (n == 0)
        {
            int newx = currentx;
            int newy = currenty + 1;

            if (pathchecked[newx][newy] != checkno && region.sea(newx, newy) == 1)
            {
                nn = findpath(region, leftx, lefty, rightx, righty, newx, newy, destx, desty, checkno, pathchecked, recursion);
                pathchecked[newx][newy] = checkno;

                if (nn == 0)
                    n = 0;
                else
                    n = 1;
            }
        }
    }

    if (searchdir == 4) // If we're looking roughly to the east-northeast
    {
        // Look to the east.

        if (n == 0)
        {
            int newx = currentx + 1;
            int newy = currenty;

            if (pathchecked[newx][newy] != checkno && region.sea(newx, newy) == 1)
            {
                nn = findpath(region, leftx, lefty, rightx, righty, newx, newy, destx, desty, checkno, pathchecked, recursion);
                pathchecked[newx][newy] = checkno;

                if (nn == 0)
                    n = 0;
                else
                    n = 1;
            }
        }

        // Look to the north.

        if (n == 0)
        {
            int newx = currentx;
            int newy = currenty - 1;

            if (pathchecked[newx][newy] != checkno && region.sea(newx, newy) == 1)
            {
                nn = findpath(region, leftx, lefty, rightx, righty, newx, newy, destx, desty, checkno, pathchecked, recursion);
                pathchecked[newx][newy] = checkno;

                if (nn == 0)
                    n = 0;
                else
                    n = 1;
            }
        }

        // Look to the south.

        if (n == 0)
        {
            int newx = currentx;
            int newy = currenty + 1;

            if (pathchecked[newx][newy] != checkno && region.sea(newx, newy) == 1)
            {
                nn = findpath(region, leftx, lefty, rightx, righty, newx, newy, destx, desty, checkno, pathchecked, recursion);
                pathchecked[newx][newy] = checkno;

                if (nn == 0)
                    n = 0;
                else
                    n = 1;
            }
        }

        // Look to the west.

        if (n == 0)
        {
            int newx = currentx - 1;
            int newy = currenty;

            if (pathchecked[newx][newy] != checkno && region.sea(newx, newy) == 1)
            {
                nn = findpath(region, leftx, lefty, rightx, righty, newx, newy, destx, desty, checkno, pathchecked, recursion);
                pathchecked[newx][newy] = checkno;

                if (nn == 0)
                    n = 0;
                else
                    n = 1;
            }
        }
    }

    if (searchdir == 6) // If we're looking roughly to the east-southeast
    {
        // Look to the east.

        if (n == 0)
        {
            int newx = currentx + 1;
            int newy = currenty;

            if (pathchecked[newx][newy] != checkno && region.sea(newx, newy) == 1)
            {
                nn = findpath(region, leftx, lefty, rightx, righty, newx, newy, destx, desty, checkno, pathchecked, recursion);
                pathchecked[newx][newy] = checkno;

                if (nn == 0)
                    n = 0;
                else
                    n = 1;
            }
        }

        // Look to the south.

        if (n == 0)
        {
            int newx = currentx;
            int newy = currenty + 1;

            if (pathchecked[newx][newy] != checkno && region.sea(newx, newy) == 1)
            {
                nn = findpath(region, leftx, lefty, rightx, righty, newx, newy, destx, desty, checkno, pathchecked, recursion);
                pathchecked[newx][newy] = checkno;

                if (nn == 0)
                    n = 0;
                else
                    n = 1;
            }
        }

        // Look to the north.

        if (n == 0)
        {
            int newx = currentx;
            int newy = currenty - 1;

            if (pathchecked[newx][newy] != checkno && region.sea(newx, newy) == 1)
            {
                nn = findpath(region, leftx, lefty, rightx, righty, newx, newy, destx, desty, checkno, pathchecked, recursion);
                pathchecked[newx][newy] = checkno;

                if (nn == 0)
                    n = 0;
                else
                    n = 1;
            }
        }

        // Look to the west.

        if (n == 0)
        {
            int newx = currentx - 1;
            int newy = currenty;

            if (pathchecked[newx][newy] != checkno && region.sea(newx, newy) == 1)
            {
                nn = findpath(region, leftx, lefty, rightx, righty, newx, newy, destx, desty, checkno, pathchecked, recursion);
                pathchecked[newx][newy] = checkno;

                if (nn == 0)
                    n = 0;
                else
                    n = 1;
            }
        }
    }

    if (searchdir == 8) // If we're looking roughly to the south-southeast
    {
        // Look to the south.

        if (n == 0)
        {
            int newx = currentx;
            int newy = currenty + 1;

            if (pathchecked[newx][newy] != checkno && region.sea(newx, newy) == 1)
            {
                nn = findpath(region, leftx, lefty, rightx, righty, newx, newy, destx, desty, checkno, pathchecked, recursion);
                pathchecked[newx][newy] = checkno;

                if (nn == 0)
                    n = 0;
                else
                    n = 1;
            }
        }

        // Look to the east.

        if (n == 0)
        {
            int newx = currentx + 1;
            int newy = currenty;

            if (pathchecked[newx][newy] != checkno && region.sea(newx, newy) == 1)
            {
                nn = findpath(region, leftx, lefty, rightx, righty, newx, newy, destx, desty, checkno, pathchecked, recursion);
                pathchecked[newx][newy] = checkno;

                if (nn == 0)
                    n = 0;
                else
                    n = 1;
            }
        }

        // Look to the west.

        if (n == 0)
        {
            int newx = currentx - 1;
            int newy = currenty;

            if (pathchecked[newx][newy] != checkno && region.sea(newx, newy) == 1)
            {
                nn = findpath(region, leftx, lefty, rightx, righty, newx, newy, destx, desty, checkno, pathchecked, recursion);
                pathchecked[newx][newy] = checkno;

                if (nn == 0)
                    n = 0;
                else
                    n = 1;
            }
        }

        // Look to the north.

        if (n == 0)
        {
            int newx = currentx;
            int newy = currenty - 1;

            if (pathchecked[newx][newy] != checkno && region.sea(newx, newy) == 1)
            {
                nn = findpath(region, leftx, lefty, rightx, righty, newx, newy, destx, desty, checkno, pathchecked, recursion);
                pathchecked[newx][newy] = checkno;

                if (nn == 0)
                    n = 0;
                else
                    n = 1;
            }
        }
    }

    if (searchdir == 10) // If we're looking roughly to the south-southwest
    {
        // Look to the south.

        if (n == 0)
        {
            int newx = currentx;
            int newy = currenty + 1;

            if (pathchecked[newx][newy] != checkno && region.sea(newx, newy) == 1)
            {
                nn = findpath(region, leftx, lefty, rightx, righty, newx, newy, destx, desty, checkno, pathchecked, recursion);
                pathchecked[newx][newy] = checkno;

                if (nn == 0)
                    n = 0;
                else
                    n = 1;
            }
        }

        // Look to the west.

        if (n == 0)
        {
            int newx = currentx - 1;
            int newy = currenty;

            if (pathchecked[newx][newy] != checkno && region.sea(newx, newy) == 1)
            {
                nn = findpath(region, leftx, lefty, rightx, righty, newx, newy, destx, desty, checkno, pathchecked, recursion);
                pathchecked[newx][newy] = checkno;

                if (nn == 0)
                    n = 0;
                else
                    n = 1;
            }
        }

        // Look to the east.

        if (n == 0)
        {
            int newx = currentx + 1;
            int newy = currenty;

            if (pathchecked[newx][newy] != checkno && region.sea(newx, newy) == 1)
            {
                nn = findpath(region, leftx, lefty, rightx, righty, newx, newy, destx, desty, checkno, pathchecked, recursion);
                pathchecked[newx][newy] = checkno;

                if (nn == 0)
                    n = 0;
                else
                    n = 1;
            }
        }

        // Look to the north.

        if (n == 0)
        {
            int newx = currentx;
            int newy = currenty - 1;

            if (pathchecked[newx][newy] != checkno && region.sea(newx, newy) == 1)
            {
                nn = findpath(region, leftx, lefty, rightx, righty, newx, newy, destx, desty, checkno, pathchecked, recursion);
                pathchecked[newx][newy] = checkno;

                if (nn == 0)
                    n = 0;
                else
                    n = 1;
            }
        }
    }

    if (searchdir == 12) // If we're looking roughly to the west-southwest
    {
        // Look to the west.

        if (n == 0)
        {
            int newx = currentx - 1;
            int newy = currenty;

            if (pathchecked[newx][newy] != checkno && region.sea(newx, newy) == 1)
            {
                nn = findpath(region, leftx, lefty, rightx, righty, newx, newy, destx, desty, checkno, pathchecked, recursion);
                pathchecked[newx][newy] = checkno;

                if (nn == 0)
                    n = 0;
                else
                    n = 1;
            }
        }

        // Look to the south.

        if (n == 0)
        {
            int newx = currentx;
            int newy = currenty + 1;

            if (pathchecked[newx][newy] != checkno && region.sea(newx, newy) == 1)
            {
                nn = findpath(region, leftx, lefty, rightx, righty, newx, newy, destx, desty, checkno, pathchecked, recursion);
                pathchecked[newx][newy] = checkno;

                if (nn == 0)
                    n = 0;
                else
                    n = 1;
            }
        }

        // Look to the north.

        if (n == 0)
        {
            int newx = currentx;
            int newy = currenty - 1;

            if (pathchecked[newx][newy] != checkno && region.sea(newx, newy) == 1)
            {
                nn = findpath(region, leftx, lefty, rightx, righty, newx, newy, destx, desty, checkno, pathchecked, recursion);
                pathchecked[newx][newy] = checkno;

                if (nn == 0)
                    n = 0;
                else
                    n = 1;
            }
        }

        // Look to the east.

        if (n == 0)
        {
            int newx = currentx + 1;
            int newy = currenty;

            if (pathchecked[newx][newy] != checkno && region.sea(newx, newy) == 1)
            {
                nn = findpath(region, leftx, lefty, rightx, righty, newx, newy, destx, desty, checkno, pathchecked, recursion);
                pathchecked[newx][newy] = checkno;

                if (nn == 0)
                    n = 0;
                else
                    n = 1;
            }
        }
    }

    if (searchdir == 14) // If we're looking roughly to the west-northwest
    {
        // Look to the west.

        if (n == 0)
        {
            int newx = currentx - 1;
            int newy = currenty;

            if (pathchecked[newx][newy] != checkno && region.sea(newx, newy) == 1)
            {
                nn = findpath(region, leftx, lefty, rightx, righty, newx, newy, destx, desty, checkno, pathchecked, recursion);
                pathchecked[newx][newy] = checkno;

                if (nn == 0)
                    n = 0;
                else
                    n = 1;
            }
        }

        // Look to the north.

        if (n == 0)
        {
            int newx = currentx;
            int newy = currenty - 1;

            if (pathchecked[newx][newy] != checkno && region.sea(newx, newy) == 1)
            {
                nn = findpath(region, leftx, lefty, rightx, righty, newx, newy, destx, desty, checkno, pathchecked, recursion);
                pathchecked[newx][newy] = checkno;

                if (nn == 0)
                    n = 0;
                else
                    n = 1;
            }
        }

        // Look to the south.

        if (n == 0)
        {
            int newx = currentx;
            int newy = currenty + 1;

            if (pathchecked[newx][newy] != checkno && region.sea(newx, newy) == 1)
            {
                nn = findpath(region, leftx, lefty, rightx, righty, newx, newy, destx, desty, checkno, pathchecked, recursion);
                pathchecked[newx][newy] = checkno;

                if (nn == 0)
                    n = 0;
                else
                    n = 1;
            }
        }

        // Look to the east.

        if (n == 0)
        {
            int newx = currentx + 1;
            int newy = currenty;

            if (pathchecked[newx][newy] != checkno && region.sea(newx, newy) == 1)
            {
                nn = findpath(region, leftx, lefty, rightx, righty, newx, newy, destx, desty, checkno, pathchecked, recursion);
                pathchecked[newx][newy] = checkno;

                if (nn == 0)
                    n = 0;
                else
                    n = 1;
            }
        }
    }

    if (searchdir == 16) // If we're looking roughly to the north-northwest
    {
        // Look to the north.

        if (n == 0)
        {
            int newx = currentx;
            int newy = currenty - 1;

            if (pathchecked[newx][newy] != checkno && region.sea(newx, newy) == 1)
            {
                nn = findpath(region, leftx, lefty, rightx, righty, newx, newy, destx, desty, checkno, pathchecked, recursion);
                pathchecked[newx][newy] = checkno;

                if (nn == 0)
                    n = 0;
                else
                    n = 1;
            }
        }

        // Look to the west.

        if (n == 0)
        {
            int newx = currentx - 1;
            int newy = currenty;

            if (pathchecked[newx][newy] != checkno && region.sea(newx, newy) == 1)
            {
                nn = findpath(region, leftx, lefty, rightx, righty, newx, newy, destx, desty, checkno, pathchecked, recursion);
                pathchecked[newx][newy] = checkno;

                if (nn == 0)
                    n = 0;
                else
                    n = 1;
            }
        }

        // Look to the east.

        if (n == 0)
        {
            int newx = currentx + 1;
            int newy = currenty;

            if (pathchecked[newx][newy] != checkno && region.sea(newx, newy) == 1)
            {
                nn = findpath(region, leftx, lefty, rightx, righty, newx, newy, destx, desty, checkno, pathchecked, recursion);
                pathchecked[newx][newy] = checkno;

                if (nn == 0)
                    n = 0;
                else
                    n = 1;
            }
        }

        // Look to the south.

        if (n == 0)
        {
            int newx = currentx;
            int newy = currenty + 1;

            if (pathchecked[newx][newy] != checkno && region.sea(newx, newy) == 1)
            {
                nn = findpath(region, leftx, lefty, rightx, righty, newx, newy, destx, desty, checkno, pathchecked, recursion);
                pathchecked[newx][newy] = checkno;

                if (nn == 0)
                    n = 0;
                else
                    n = 1;
            }
        }
    }


    return n;
}

// This identifies any more pools that are on the map and turns them into land (originally it changed them into lakes but I changed it!). (This is because the removepools routine checks only near coastlines, and may miss some that are further inland, especially where fjords are nearby.)

void turnpoolstolakes(planet& world, region& region, int dx, int dy, int sx, int sy, vector<vector<bool>>& regionsea, vector<vector<int>>& pathchecked, int& checkno)
{
    int width = world.width();
    int height = world.height();

    // First see if this tile contains both sea and land.

    bool seapresent = 0;
    bool landpresent = 0;

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (regionsea[i][j] == 1)
                seapresent = 1;
            else
                landpresent = 1;

            if (seapresent && landpresent)
            {
                i = dx + 16;
                j = dy + 16;
            }
        }
    }

    if (seapresent == 0 || landpresent == 0)
        return;

    // Now we need to check each sea cell. If it is part of a body of water that is smaller than a certain amount, we will consider it a pool.

    int sealevel = world.sealevel();
    int maxtally = 800; // Any bit of sea with a smaller tally than this is a pool.

    for (int startx = dx; startx <= dx + 16; startx++)
    {
        for (int starty = dy; starty <= dy + 16; starty++)
        {
            if (regionsea[startx][starty] == 1) // Bit of sea in the source tile
            {
                checkno++;
                int tally = 0;

                poolcheckrecursive(region, startx, starty, tally, maxtally, checkno, regionsea, pathchecked);

#if 0 // example debugging code for comparing the results of two different poolcheck() functions
                static unsigned tot_count(0), diff_count(0);
                ++tot_count;

                if (diff_count < 100) { // show at most 100 diffs
                    checkno++;
                    int tally2 = 0;
                    poolcheckrecursive(region, startx, starty, tally2, maxtally, checkno, regionsea, pathchecked);

                    if ((tally < maxtally) != (tally2 < maxtally)) {
                        cout << "tally: " << tally << ", tally2: " << tally2 << ", maxtally: " << maxtally << ", tot_count: " << tot_count << ", diff_count: " << diff_count << endl;
                        ++diff_count;
                    }
                }
#endif
                if (tally < maxtally) // If it's small enough to be a pool
                {
                    for (int i = dx - 16; i <= dx + 16; i++) // Turn all the cells that got marked in this pass into land.
                    {
                        for (int j = dy - 16; j <= dy + 16; j++)
                        {
                            if (pathchecked[i][j] == checkno)
                            {
                                region.setmap(i, j, sealevel + 1);
                                regionsea[i][j] = 0;
                            }
                        }
                    }
                }
            }
        }
    }
}

// This finds whether the area of a given bit of sea is smaller than a certain amount. Tally records the number of cells counted.
// (Originally by me, improved and corrected by FG)

// Recursive version:

void poolcheckrecursive(region const& region, int const currentx, int const currenty, int& tally, int maxtally, int const checkno, vector<vector<bool>> const& regionsea, vector<vector<int>>& pathchecked)
{
    if (pathchecked[currentx][currenty] == checkno)
        return;
    pathchecked[currentx][currenty] = checkno;

    if (regionsea[currentx][currenty] == 0)
        return;

    if (currentx<1 || currenty<1 || currentx>region.rwidth() - 1 || currenty>region.rheight() - 1) // We've gone out of the search zone, so this might be a pool
    {
        tally = maxtally + 1;
        return;
    }
    tally++;

    if (tally > maxtally) // We've counted more cells than a pool can be
        return;

    int const x_deltas[4] = { 0, 1, -1, 0 }; // north, east, west, south
    int const y_deltas[4] = { -1, 0, 0, 1 }; // north, east, west, south

    for (int dir = 0; dir < 4; ++dir) {
        poolcheckrecursive(region, (currentx + x_deltas[dir]), (currenty + y_deltas[dir]), tally, maxtally, checkno, regionsea, pathchecked);
        if (tally > maxtally) // We've counted more cells than a pool can be (early exit optimization)
            return;
    }
}

// Non-recursive version (not used, as it's slower):

void poolcheck(region const& region, int const currentx, int const currenty, int& tally, int maxtally, int const checkno, vector<vector<bool>> const& regionsea, vector<vector<int>>& pathchecked)
{
    if (regionsea[currentx][currenty] == 0)
        return;

    if (pathchecked[currentx][currenty] == checkno)
        return;

    queue<twointegers> q;

    twointegers point;
    point.x = currentx;
    point.y = currenty;

    q.push(point);
    tally++; // first point counts

    while (!q.empty())
    {
        point = q.front();

        if (point.x<1 || point.y<1 || point.x>region.rwidth() - 1 || point.y>region.rheight() - 1) // We've gone out of the search zone, so this might be ocean
        {
            tally = maxtally + 1;
            return;
        }

        q.pop();

        int const x_deltas[4] = { 0, 1, -1, 0 }; // north, east, west, south
        int const y_deltas[4] = { -1, 0, 0, 1 }; // north, east, west, south

        for (int dir = 0; dir < 4; ++dir) {
            twointegers newpoint = point;
            newpoint.x += x_deltas[dir];
            newpoint.y += y_deltas[dir];

            if (pathchecked[newpoint.x][newpoint.y] != checkno && regionsea[newpoint.x][newpoint.y] == 1)
            {
                pathchecked[newpoint.x][newpoint.y] = checkno;
                tally++;

                if (tally > maxtally) // We've counted more cells than a pool can be
                    return;
                q.push(newpoint);
            }
        }
    }
}

// This function turns a river on the regional map into sea from this point downstream.

void turntosea(region& region, int leftx, int rightx, int lefty, int righty, int x, int y, int newheight, int sealevel)
{
    for (int n = 1; n <= 100; n++)
    {
        if (x<leftx || x>rightx || y<lefty || y>righty)
            return;

        int dir = region.riverdir(x, y);

        if (dir < 1 || dir>8)
            return;

        for (int i = x - 1; i <= x + 1; i++)
        {
            for (int j = y - 1; j <= y + 1; j++)
            {
                if (i >= leftx && i <= rightx && j >= lefty && j <= righty)
                {
                    if (region.map(i, j) > sealevel)
                        region.setmap(i, j, newheight);
                }
            }
        }

        if (dir == 8 || dir == 1 || dir == 2)
            y--;

        if (dir == 4 || dir == 5 || dir == 6)
            y++;

        if (dir == 2 || dir == 3 || dir == 4)
            x++;

        if (dir == 6 || dir == 7 || dir == 8)
            x--;
    }
}

// This function removes diagonal connections in seas and lakes.

void removediagonalwater(region& region, int leftx, int lefty, int rightx, int righty, int sealevel)
{
    for (int i = leftx; i < rightx; i++)
    {
        for (int j = lefty; j < righty; j++)
        {
            // First do seas.

            if (region.map(i, j) <= sealevel && region.map(i + 1, j) > sealevel && region.map(i, j + 1) > sealevel && region.map(i + 1, j + 1) <= sealevel)
                region.setmap(i + 1, j, region.map(i, j));

            if (region.map(i, j) > sealevel && region.map(i + 1, j) <= sealevel && region.map(i, j + 1) <= sealevel && region.map(i + 1, j + 1) > sealevel)
                region.setmap(i, j, region.map(i + 1, j));

            // Now do lakes.

            if (region.truelake(i, j) == 0 && region.truelake(i + 1, j) != 0 && region.truelake(i, j + 1) != 0 && region.truelake(i + 1, j + 1) == 0)
            {
                region.setlakesurface(i + 1, j, region.lakesurface(i, j));
                region.setmap(i + 1, j, region.map(i, j));
            }

            if (region.truelake(i, j) == 0 && region.truelake(i + 1, j) != 0 && region.truelake(i, j + 1) != 0 && region.truelake(i + 1, j + 1) == 0)
            {
                region.setlakesurface(i, j, region.lakesurface(i + 1, j));
                region.setmap(i, j, region.map(i + 1, j));
            }
        }
    }
}

// This function turns any lake tiles next to sea tiles into land.

void removelakesbysea(region& region, int leftx, int lefty, int rightx, int righty, int sealevel)
{
    for (int i = leftx; i <= rightx; i++)
    {
        for (int j = lefty; j <= righty; j++)
        {
            if (region.lakesurface(i, j) != 0)
            {
                bool foundsea = 0;

                for (int k = i - 1; k <= i + 1; k++)
                {
                    for (int l = j - 1; l <= j + 1; l++)
                    {
                        if (k >= leftx && k <= rightx && l >= lefty && l <= righty)
                        {
                            if (region.sea(k, l) == 1)
                            {
                                foundsea = 1;
                                k = i + 1;
                                l = j + 1;
                            }
                        }
                    }
                }

                if (foundsea == 1) // If we've found some sea next to this lake
                {
                    region.setmap(i, j, region.lakesurface(i, j));
                    region.setlakesurface(i, j, 0);
                }
            }
        }
    }
}

// This function removes sinks - areas of very low elevation - from the regional map.

void removesinks(planet& world, region& region, int dx, int dy, int sx, int sy)
{
    int sealevel = world.sealevel();

    int rstart = region.rstart();
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    int maxdiff = 200; // Maximum difference in elevation between a point and both its neighbours on opposite sides.

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            // First remove low areas near lakes.

            if (region.map(i, j) == sealevel + 1 && region.lakesurface(i, j) == 0 && region.riverdir(i, j) == 0 && region.deltadir(i, j) == 0)
            {
                int elev = world.nom(sx, sy);

                if (world.lakesurface(sx, sy) > elev)
                    elev = world.lakesurface(sx, sy);

                //region.setmap(i,j,elev);

                // We need to fill this depression up to the new height.

                fillregionaldepression(region, dx, dy, i, j, elev);
            }

            if (i > rstart && i<rwidth && j>rstart && j < rheight && region.sea(i, j) == 0 && region.lakesurface(i, j) == 0 && region.riverdir(i, j) == 0 && region.deltadir(i, j) == 0)
            {
                int elev = region.map(i, j);

                // Now remove low points with higher points on either side.

                int leftelev = region.map(i - 1, j);
                int rightelev = region.map(i + 1, j);
                int upelev = region.map(i, j - 1);
                int downelev = region.map(i, j + 1);

                if (elev < leftelev - maxdiff && elev < rightelev - maxdiff)
                    elev = (leftelev + rightelev) / 2;

                if (elev < upelev - maxdiff && elev < downelev - maxdiff)
                    elev = (upelev + downelev) / 2;

                // Now remove any points lower than neighbouring lakes.

                int leftlake = region.lakesurface(i - 1, j);
                int rightlake = region.lakesurface(i + 1, j);
                int uplake = region.lakesurface(i, j - 1);
                int downlake = region.lakesurface(i, j + 1);

                if (leftlake > elev)
                    elev = leftlake;

                if (rightlake > elev)
                    elev = rightlake;

                if (uplake > elev)
                    elev = uplake;

                if (downlake > elev)
                    elev = downlake;

                region.setmap(i, j, elev);
            }
        }
    }
}

// This fills a depression on the regional map, within a tile, up to the given height.

void fillregionaldepression(region& region, int dx, int dy, int x, int y, int elev)
{
    vector<vector<bool>> checked(RARRAYWIDTH * 2, vector<bool>(RARRAYHEIGHT * 2, 0));

    int elevadded = elev - region.sealevel();
    int reduceamount = elevadded / 4;
    bool overrun = 0;

    while (1 == 1)
    {
        elev = region.sealevel() + elevadded;

        for (int i = dx; i <= dx + 16; i++)
        {
            for (int j = dy; j <= dy + 16; j++)
                checked[i][j] = 0;
        }

        overrun = 0;

        checkdepression(region, dx, dy, x, y, elev, overrun, checked);

        if (overrun == 0)
        {
            filldepression(region, dx, dy, x, y, elev);
            return;
        }

        elevadded = elevadded - reduceamount;

        if (elevadded < 1)
            return;
    }
}

// This looks to see whether the depression can be filled with the current elevation without spilling over the edge of the tile.

void checkdepression(region& region, int dx, int dy, int x, int y, int elev, bool& overrun, vector<vector<bool>>& checked)
{
    if (overrun == 1)
        return;

    checked[x][y] = 1;

    int ny = y - 1;
    int sy = y + 1;
    int wx = x - 1;
    int ex = x + 1;

    if (ny >= dy)
    {
        if (region.map(x, ny) < elev && region.sea(x, ny) == 0 && region.riverdir(x, ny) == 0 && region.deltadir(x, ny) == 0 && checked[x][ny] == 0)
            checkdepression(region, dx, dy, x, ny, elev, overrun, checked);
    }
    else
    {
        if (region.map(x, ny) < elev && region.sea(x, ny) == 0 && region.riverdir(x, ny) == 0 && region.deltadir(x, ny) == 0)
        {
            overrun = 1;
            return;
        }
    }

    if (sy <= dy + 16)
    {
        if (region.map(x, sy) < elev && region.sea(x, sy) == 0 && region.riverdir(x, sy) == 0 && region.deltadir(x, sy) == 0 && checked[x][sy] == 0)
            checkdepression(region, dx, dy, x, sy, elev, overrun, checked);
    }
    else
    {
        if (region.map(x, sy) < elev && region.sea(x, sy) == 0 && region.riverdir(x, sy) == 0 && region.deltadir(x, sy) == 0)
        {
            overrun = 1;
            return;
        }
    }

    if (wx >= dx)
    {
        if (region.map(wx, y) < elev && region.sea(wx, y) == 0 && region.riverdir(wx, y) == 0 && region.deltadir(wx, y) == 0 && checked[wx][y] == 0)
            checkdepression(region, dx, dy, wx, y, elev, overrun, checked);
    }
    else
    {
        if (region.map(wx, y) < elev && region.sea(wx, y) == 0 && region.riverdir(wx, y) == 0 && region.deltadir(wx, y) == 0)
        {
            overrun = 1;
            return;
        }
    }

    if (ex <= dx + 16)
    {
        if (region.map(ex, y) < elev && region.sea(ex, y) == 0 && region.riverdir(ex, y) == 0 && region.deltadir(ex, y) == 0 && checked[ex][y] == 0)
            checkdepression(region, dx, dy, ex, y, elev, overrun, checked);
    }
    else
    {
        if (region.map(ex, y) < elev && region.sea(ex, y) == 0 && region.riverdir(ex, y) == 0 && region.deltadir(ex, y) == 0)
        {
            overrun = 1;
            return;
        }
    }
}

// This does the actual filling.

void filldepression(region& region, int dx, int dy, int x, int y, int elev)
{
    region.setmap(x, y, elev);

    int ny = y - 1;
    int sy = y + 1;
    int wx = x - 1;
    int ex = x + 1;

    if (ny >= dy)
    {
        if (region.map(x, ny) < elev && region.sea(x, ny) == 0 && region.riverdir(x, ny) == 0 && region.deltadir(x, ny) == 0)
            fillregionaldepression(region, dx, dy, x, ny, elev);
    }

    if (sy <= dy + 16)
    {
        if (region.map(x, sy) < elev && region.sea(x, sy) == 0 && region.riverdir(x, sy) == 0 && region.deltadir(x, sy) == 0)
            fillregionaldepression(region, dx, dy, x, sy, elev);
    }

    if (wx >= dx)
    {
        if (region.map(wx, y) < elev && region.sea(wx, y) == 0 && region.riverdir(wx, y) == 0 && region.deltadir(wx, y) == 0)
            fillregionaldepression(region, dx, dy, wx, y, elev);
    }

    if (ex <= dx + 16)
    {
        if (region.map(ex, y) < elev && region.sea(ex, y) == 0 && region.riverdir(ex, y) == 0 && region.deltadir(ex, y) == 0)
            fillregionaldepression(region, dx, dy, ex, y, elev);
    }
}

// This function adds inlets to the mouths of rivers.

void addinlets(planet& world, region& region, int dx, int dy, int sx, int sy, vector<vector<bool>>& riverinlets, vector<vector<bool>>& globalestuaries)
{
    if (world.riverdir(sx, sy) == 0)
        return;

    if (vaguelycoastal(world, sx, sy) == 0)
        return;

    if (world.deltadir(sx, sy) != 0)
        return;

    int width = world.width();
    int height = world.height();

    for (int i = sx - 1; i <= sx + 1; i++)
    {
        int ii = i;

        if (ii<0 || ii>width)
            ii = wrap(ii, width);

        for (int j = sy - 1; j <= sy + 1; j++)
        {
            if (j >= 0 && j <= height)
            {
                if (world.deltadir(ii, j) != 0)
                    return;
            }
        }
    }

    fast_srand((sy * world.width() + sx) + world.map(sx, sy) + world.julrain(sx, sy));

    int inletchance = 5; // Base probability of an inlet in low-tide areas.

    inletchance = inletchance - world.tide(sx, sy); // Higher tides mean more inlets.

    int aveflow = (world.riverjan(sx, sy) + world.riverjul(sx, sy)) / 2;

    inletchance = inletchance - (aveflow / 200); // 300 The bigger the river, the more likely it is to have an inlet.

    if (inletchance < 1)
        inletchance = 1;

    if (random(1, inletchance) != 1)
        return;

    for (int i = sx - 1; i <= sx + 1; i++) // Mark on a global map array where we're doing this, so we can add mud flats to these quickly later on.
    {
        int ii = i;

        if (ii<0 || ii>width)
            ii = wrap(ii, width);

        for (int j = sy - 1; j <= sy + 1; j++)
        {
            if (j >= 0 && j <= height)
            {
                globalestuaries[ii][j] = 1;
            }
        }
    }

    int rwidth = region.rwidth();
    int rheight = region.rheight();
    int widthfactor = random(1, 5); // The mouth of the inlet will be the width of its inland end, times this.
    int lengthfactor = random(1, 6); // The length of the inlet will be the width of its mouth, times this.

    lengthfactor = lengthfactor * (world.tide(sx, sy) / 4);

    if (lengthfactor > 10)
        lengthfactor = 10;

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (region.sea(i, j) == 0 && region.riverdir(i, j) != 0)
            {
                twointegers dest = getregionalflowdestination(region, i, j, 0);

                if (region.sea(dest.x, dest.y) == 1) // It's a river on land, flowing into the sea
                {
                    int depth = region.map(dest.x, dest.y);

                    int wide = getriverwidth(region, i, j, 0, 3);

                    int mininletwidth = (wide) / region.pixelmetres() + 1;

                    if (mininletwidth > 10)
                        return;

                    int maxinletwidth = mininletwidth * widthfactor;

                    if (maxinletwidth > 10)
                        maxinletwidth = 10;

                    float currentinletwidth = (float)maxinletwidth;

                    float inletlength = (float)(maxinletwidth * lengthfactor);

                    if (inletlength > 20.0f)
                        inletlength = 20.0f;

                    float widthreduce = currentinletwidth / inletlength;

                    int x = i;
                    int y = j;

                    int currentlength = 0;

                    while (1 == 1)
                    {
                        pasteinletcircle(region, x, y, depth, (int)currentinletwidth, riverinlets);

                        int nextx = -1;
                        int nexty = -1;

                        twointegers dest;

                        for (int ii = x - 1; ii <= x + 1; ii++)
                        {
                            for (int jj = y - 1; jj <= y + 1; jj++)
                            {
                                if (x >= 0 && x <= rwidth && y >= 0 && y <= rheight)
                                {
                                    dest = getregionalflowdestination(region, ii, jj, 0);

                                    if (dest.x == x && dest.y == y)
                                    {
                                        nextx = ii;
                                        nexty = jj;

                                        ii = x + 1;
                                        jj = y + 1;
                                    }
                                }
                            }
                        }

                        if (nextx == -1)
                            return;

                        currentlength++;

                        if (currentlength > inletlength)
                            return;

                        currentinletwidth = currentinletwidth - widthreduce;

                        if (currentinletwidth < (float)mininletwidth)
                            currentinletwidth = (float)mininletwidth;

                        x = nextx;
                        y = nexty;
                    }
                    return;
                }
            }
        }
    }
}

// This function pastes an inlet circle onto the regional map.

void pasteinletcircle(region& region, int centrex, int centrey, int depth, int pixels, vector<vector<bool>>& riverinlets)
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    int half = pixels / 2;

    for (int i = -half; i <= half; i++)
    {
        for (int j = -half; j <= half; j++)
        {
            int ii = centrex + i;
            int jj = centrey + j;

            if (ii >= 0 && ii <= rwidth && jj >= 0 && jj <= rheight)
            {
                if (region.map(ii, jj) > depth)
                {
                    if (i * i + j * j < half * half + half)
                    {
                        region.setmap(ii, jj, depth);

                        riverinlets[ii][jj] = 1; // Mark it here so we can later make sure that barrier islands don't appear in inlets. Also we will put beaches and mud flats in inlets.
                    }
                }
            }
        }
    }
}

// This function puts the wetlands in a tile.

void makewetlandtile(planet& world, region& region, int dx, int dy, int sx, int sy, boolshapetemplate smalllake[])
{
    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();

    fast_srand((sy * width + sx) + world.map(sx, sy) + world.julrain(sx, sy) + world.riverjan(sx, sy));

    int wetlandchance = 14; // The higher this is, the fewer splodges of wetlands there will be.
    int special = world.special(sx, sy);

    if (special < 130 || special>139)
    {
        for (int i = sx - 1; i <= sx + 1; i++) // See if there are wetlands in neighbouring tiles. If so, we'll put just a few in this tile.
        {
            int ii = i;

            if (ii<0 || i>width)
                ii = wrap(ii, width);

            for (int j = sy - 1; j <= sy + 1; j++)
            {
                if (j >= 0 && j <= height)
                {
                    if (world.special(ii, j) >= 130 && world.special(ii, j) <= 139)
                    {
                        special = world.special(ii, j);
                        wetlandchance = 30;

                        i = sx + 1;
                        j = sy + 1;
                    }
                }
            }
        }
    }

    if (special < 130 || special>139)
        return;

    fast_srand((sy * width + sx) + world.map(sx, sy) + (int)world.roughness(sx, sy));

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            bool wetlandspresent = 0;
            int elev = 0;

            if (region.riverdir(i, j) != 0 || region.fakedir(i, j) != 0)
                wetlandspresent = 1;
            else
            {
                if (region.lakesurface(i, j) != 0 || region.sea(i, j) == 1)
                    wetlandspresent = 1;
            }

            if (wetlandspresent == 1)
            {
                if (region.sea(i, j) == 1)
                    elev = sealevel + 1;
                else
                {
                    if (region.lakesurface(i, j) != 0)
                        elev = region.lakesurface(i, j);
                    else
                        elev = region.map(i, j);
                }

                if (elev < sealevel + 2)
                    elev = sealevel + 2;

                if (random(1, wetlandchance) == 1)
                {
                    int shapenumber = random(0, 11);
                    pasteregionalwetlands(region, i, j, special, elev, shapenumber, smalllake);
                }
            }
        }
    }
}

// This function pastes some wetlands onto the regional map.

void pasteregionalwetlands(region& region, int centrex, int centrey, int special, int elev, int shapenumber, boolshapetemplate smalllake[])
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    int imheight = smalllake[shapenumber].ysize() - 1;
    int imwidth = smalllake[shapenumber].xsize() - 1;

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
        jstart = imheight;
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

                if (xx >= 0 && xx <= rwidth && yy >= 0 && yy <= rheight)
                {
                    if (region.lakesurface(xx, yy) == 0 && region.sea(xx, yy) == 0 && region.mud(xx, yy) == 0 && region.sand(xx, yy) == 0 && region.shingle(xx, yy) == 0)
                    {
                        region.setspecial(xx, yy, special);

                        if (region.map(xx, yy) > elev || region.map(xx, yy) == 0)
                            region.setmap(xx, yy, elev);
                    }
                }
            }
        }
    }
}

// This ensures that any lakes in the tile are special lakes, if they should be.

void convertlakestospecials(planet& world, region& region, int dx, int dy, int sx, int sy, vector<vector<bool>>& safesaltlakes)
{
    int width = world.width();
    int height = world.height();

    int amount = 1; // Range to search

    int special = 0;
    int elev = 0;
    bool found = 0;

    // 100: salt lake. 110: salt pan. 120: dunes. 130: fresh wetlands. 131: brackish wetlands. 132: salt wetlands.

    if (world.special(sx, sy) >= 100 && world.special(sx, sy) < 130)
    {
        special = world.special(sx, sy);
        elev = world.lakesurface(sx, sy);
        found = 1;
    }

    if (found == 0)
    {
        for (int i = sx - amount; i <= sx + amount; i++)
        {
            int ii = i;

            if (ii<0 || ii>width)
                ii = wrap(ii, width);

            for (int j = sy - amount; j <= sy + amount; j++)
            {
                if (j >= 0 && j <= height)
                {
                    if (world.special(ii, j) >= 100 && world.special(ii, j) < 130)
                    {
                        special = world.special(ii, j);
                        elev = world.lakesurface(ii, j);

                        i = sx + amount;
                        j = sy + amount;
                    }
                }
            }
        }
    }

    if (special == 0)
        return;

    // Get coordinates for the tiles around this one.

    int eax = sx + 1;
    int eay = sy;
    if (eax > width)
        eax = 0;

    int sox = sx;
    int soy = sy + 1;

    int sex = eax;
    int sey = soy;

    int nwelev = world.nom(sx, sy);
    int neelev = world.nom(eax, eay);
    int swelev = world.nom(sox, soy);
    int seelev = world.nom(sex, sey);

    if (special == 100)
    {
        if (nwelev >= elev)
            nwelev = elev - 1;

        if (neelev >= elev)
            neelev = elev - 1;

        if (swelev >= elev)
            swelev = elev - 1;

        if (seelev >= elev)
            seelev = elev - 1;
    }

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            int thisspecial = region.special(i, j);
            
            if ((region.lakesurface(i, j) != 0 || thisspecial != 0) && safesaltlakes[i][j] == 0)
            {
                region.setspecial(i, j, special);
                region.setlakesurface(i, j, elev);

                if (thisspecial != 100)
                    region.setmap(i, j, elev);

                else // Salt lakes get 0 elevation on their beds for some reason, so here we sort them out.
                {
                    // We work out how close our point is to the different corners of the tile, and calculate an approximate elevation for it on that basis.

                    float nwdist = (float)(abs(i - dx) + abs(j - dy));
                    float nedist = (float)(abs(i - (dx + 16)) + abs(j - dy));
                    float swdist = (float)(abs(i - dx) + abs(j - (dy + 16)));
                    float sedist = (float)(abs(i - (dx + 16)) + abs(j - (dy + 16)));

                    float total = nwdist + nedist + swdist + sedist;

                    float elevf = ((float)nwelev * sedist + (float)neelev * swdist + (float)swelev * nedist + (float)seelev * nwdist) / total;

                    if (elevf >= (float)elev)
                        elevf = (float)elev - 1.0f;

                    region.setmap(i, j, (int)elevf);
                }
            }
        }
    }
}

// This function removes sea from river delta tiles.

void removedeltasea(planet& world, region& region, int dx, int dy, int sx, int sy)
{
    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();

    bool deltafound = 0;

    for (int i = sx - 1; i <= sx + 1; i++)
    {
        int ii = i;

        if (ii<0 || ii>width)
            ii = wrap(ii, width);

        for (int j = sy - 1; j <= sy + 1; j++)
        {
            if (j >= 0 && j <= height)
            {
                if (world.deltadir(ii, j) != 0)
                {
                    deltafound = 1;

                    i = sx + 1;
                    j = sy + 1;
                }
            }
        }
    }

    if (deltafound == 0)
        return;

    // Now check that we're not next to the sea.

    for (int i = sx - 1; i <= sx + 1; i++)
    {
        int ii = i;

        if (ii<0 || ii>width)
            ii = wrap(ii, width);

        for (int j = sy - 1; j <= sy + 1; j++)
        {
            if (j >= 0 && j <= height)
            {
                if (world.sea(ii, j) == 1)
                    return;
            }
        }
    }

    // Now get rid of any sea that's here.

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (region.sea(i, j) == 1)
                region.setmap(i, j, sealevel + 1);
        }
    }
}

// This function puts the delta branches onto the normal river map.

void adddeltamap(planet& world, region& region, int leftx, int lefty, int rightx, int righty)
{
    int sealevel = world.sealevel();

    int dir = 0;

    for (int i = leftx + 1; i <= rightx - 1; i++)
    {
        for (int j = lefty + 1; j <= righty - 1; j++)
        {
            if (region.deltadir(i, j) != 0 || region.deltajan(i, j) != 0 || region.deltajul(i, j) != 0)
            {
                if (region.deltadir(i, j - 1) == 5)
                    dir = 1;

                if (region.deltadir(i + 1, j - 1) == 6)
                    dir = 2;

                if (region.deltadir(i + 1, j) == 7)
                    dir = 3;

                if (region.deltadir(i + 1, j + 1) == 8)
                    dir = 4;

                if (region.deltadir(i, j + 1) == 1)
                    dir = 5;

                if (region.deltadir(i - 1, j + 1) == 2)
                    dir = 6;

                if (region.deltadir(i - 1, j) == 3)
                    dir = 7;

                if (region.deltadir(i - 1, j - 1) == 4)
                    dir = 8;

                region.setriverdir(i, j, dir);
                region.setriverjan(i, j, region.riverjan(i, j) + abs(region.deltajan(i, j)));
                region.setriverjul(i, j, region.riverjul(i, j) + abs(region.deltajul(i, j)));

                region.setmap(i, j, sealevel + 1);
            }
        }
    }
}

// This function creates a tile of the various regional maps.

void makegenerictile(planet& world, int dx, int dy, int sx, int sy, float valuemod, int coords[4][2], vector<vector<int>>& source, vector<vector<int>>& dest, int max, int min, bool interpolate)
{
    int width = world.width();
    int height = world.height();

    if (sx<0 || sx>width)
        sx = wrap(sx, width);

    // First, get coordinates for the tiles around this one.

    int eax = sx + 1;
    int eay = sy;
    if (eax > width)
        eax = 0;

    int eeax = eax + 1;
    int eeay = eay;
    if (eeax > width)
        eeax = 0;

    int wex = sx - 1;
    int wey = sy;
    if (wex < 0)
        wex = width;

    int sox = sx;
    int soy = sy + 1;

    int nox = sx;
    int noy = sy - 1;

    int sex = eax;
    int sey = soy;

    int seex = eeax;
    int seey = soy;

    int nwx = wex;
    int nwy = noy;

    int nex = eax;
    int ney = noy;

    int swx = wex;
    int swy = soy;

    // Now, initialise the four corners.

    if (dx != 0 && dy != 0)// && dest[dx][dy] == -5000)
        dest[dx][dy] = source[sx][sy];

    if (dx != 0)// && dest[dx + 16][dy] == -5000)
        dest[dx + 16][dy] = source[eax][eay];

    if (dx != 0)// && dest[dx][dy + 16] == -5000)
        dest[dx][dy + 16] = source[sox][soy];

    //if (dest[dx + 16][dy + 16] == -5000)
    dest[dx + 16][dy + 16] = source[sex][sey];

    if (interpolate == 1) // Add some values in the middle of the edges, to add variety.
    {
        if (random(1, 2) == 1) //(dest[dx][dy]<dest[dx+16][dy])
            dest[dx + 8][dy] = dest[dx][dy] + randomsign(random(1, 4));
        else
            dest[dx + 8][dy] = dest[dx + 16][dy] + randomsign(random(1, 4));

        if (random(1, 2) == 1) //(dest[dx][dy+16]<dest[dx+16][dy+16])
            dest[dx + 8][dy + 16] = dest[dx][dy + 16] + randomsign(random(1, 4));
        else
            dest[dx + 8][dy + 16] = dest[dx + 16][dy + 16] + randomsign(random(1, 4));

        if (random(1, 2) == 1) //(dest[dx][dy]<dest[dx][dy+16])
            dest[dx][dy + 8] = dest[dx][dy] + randomsign(random(1, 4));
        else
            dest[dx][dy + 8] = dest[dx][dy + 16] + randomsign(random(1, 4));

        if (random(1, 2) == 1) //(dest[dx+16][dy]<dest[dx+16][dy+16])
            dest[dx + 16][dy + 8] = dest[dx + 16][dy] + randomsign(random(1, 4));
        else
            dest[dx + 16][dy + 8] = dest[dx + 16][dy + 16] + randomsign(random(1, 4));
    }

    // Now we do the edges.

    int nseed = (sy * width + sx) + world.map(sx, sy) + world.julrain(sx, sy);
    int sseed = (soy * width + sox) + world.map(sox, soy) + world.julrain(sox, soy);
    int wseed = (sy * width + sx) + world.map(sx, sy) + world.julrain(sx, sy);
    int eseed = (eay * width + eax) + world.map(eax, eay) + world.julrain(eax, eay);

    if (dy != 0 && dx != 0)
    {
        if (dest[dx + 8][dy] == -5000) // Northern edge
        {
            fast_srand(nseed);

            float s = 16.0f; // Segment size.

            while (s != -1.0f)
            {
                int value = (int)(s * valuemod * 50.0f);
                int ss = (int)(s / 2.0f); // Size of half the segment.

                for (int n = 0; n <= (int)(16.0f / s) - 1; n++) // Go through all the segments of this size.
                {
                    int nn = (int)((float)n * s);
                    int ourpoint = nn + ss;

                    if (dest[dx + ourpoint][dy] == -5000)
                    {
                        int newvalue = (dest[dx + nn][dy] + dest[dx + nn + (int)s][dy]) / 2;
                        newvalue = newvalue + randomsign(random(0, value));

                        if (newvalue < min)
                            newvalue = min;

                        if (newvalue > max)
                            newvalue = max;

                        dest[dx + ourpoint][dy] = newvalue;
                    }
                }

                if (s > 2.0f)
                    s = s / 2.0f;
                else
                    s = -1.0f;
            }
        }
    }

    if (dx != 0)
    {
        if (dest[dx + 8][dy + 16] == -5000) // Southern edge
        {
            fast_srand(sseed);

            float s = 16.0f; // Segment size.

            while (s != -1.0f)
            {
                int value = (int)(s * valuemod * 50.0f);
                int ss = (int)(s / 2.0f); // Size of half the segment.

                for (int n = 0; n <= (int)(16.0f / s) - 1; n++) // Go through all the segments of this size.
                {
                    int nn = (int)((float)n * s);
                    int ourpoint = nn + ss;

                    if (dest[dx + ourpoint][dy + 16] == -5000)
                    {
                        int newvalue = (dest[dx + nn][dy + 16] + dest[dx + nn + (int)s][dy + 16]) / 2;
                        newvalue = newvalue + randomsign(random(0, value));

                        if (newvalue < min)
                            newvalue = min;

                        if (newvalue > max)
                            newvalue = max;

                        dest[dx + ourpoint][dy + 16] = newvalue;
                    }
                }

                if (s > 2.0f)
                    s = s / 2.0f;
                else
                    s = -1.0f;
            }
        }
    }

    if (dy != 0 && dx != 0)
    {
        if (dest[dx][dy + 8] == -5000) // Western edge
        {
            fast_srand(wseed);

            float s = 16.0f; // Segment size.

            while (s != -1.0f)
            {
                int value = (int)(s * valuemod * 50.0f);
                int ss = (int)(s / 2.0f); // Size of half the segment.

                for (int n = 0; n <= (int)(16.0f / s) - 1; n++) // Go through all the segments of this size.
                {
                    int nn = (int)((float)n * s);
                    int ourpoint = nn + ss;

                    if (dest[dx][dy + ourpoint] == -5000)
                    {
                        int newvalue = (dest[dx][dy + nn] + dest[dx][dy + nn + (int)s]) / 2;
                        newvalue = newvalue + randomsign(random(0, value));

                        if (newvalue < min)
                            newvalue = min;

                        if (newvalue > max)
                            newvalue = max;

                        dest[dx][dy + ourpoint] = newvalue;
                    }
                }

                if (s > 2.0f)
                    s = s / 2.0f;
                else
                    s = -1.0f;
            }
        }
    }

    if (dy != 0)
    {
        if (dest[dx + 16][dy + 8] == -5000) // Eastern edge
        {
            fast_srand(eseed);

            float s = 16.0f; // Segment size.

            while (s != -1.0f)
            {
                int value = (int)(s * valuemod * 50.0f);
                int ss = (int)(s / 2.0f); // Size of half the segment.

                for (int n = 0; n <= (int)(16.0f / s) - 1; n++) // Go through all the segments of this size.
                {
                    int nn = (int)((float)n * s);
                    int ourpoint = nn + ss;

                    if (dest[dx + 16][dy + ourpoint] == -5000)
                    {
                        int newvalue = (dest[dx + 16][dy + nn] + dest[dx + 16][dy + nn + (int)s]) / 2;
                        newvalue = newvalue + randomsign(random(0, value));

                        if (newvalue < min)
                            newvalue = min;

                        if (newvalue > max)
                            newvalue = max;

                        dest[dx + 16][dy + ourpoint] = newvalue;
                    }
                }

                if (s > 2.0f)
                    s = s / 2.0f;
                else
                    s = -1.0f;
            }
        }
    }

    // Now we fill in the middle of the tile. We do this with diamond-square.

    if (dx != 0 && dy != 0)
    {
        fast_srand(nseed);

        float s = 16.0f; // Size of the section. This will halve each time.

        while (s != -1)
        {
            int value = (int)(s * valuemod); // This is the amount we will vary each new pixel by.
            int ss = (int)(s / 2.0f);

            // First, go over the tile doing the square step.

            for (int i = 0; i <= 16; i = i + (int)s)
            {
                int ii = i + ss;

                for (int j = 0; j <= 16; j = j + (int)s)
                {
                    int jj = j + ss;

                    if (jj >= 0 && jj <= 16 && ii >= 0 && ii <= 16)
                    {
                        if (dest[dx + ii][dy + jj] == -5000)
                        {
                            int newheight;

                            newheight = genericsquare(dx, dy, (int)s, ii, jj, value, min, max, coords, dest);

                            dest[dx + ii][dy + jj] = newheight;
                        }
                    }
                }
            }

            // Now we go over the whole tile again, doing the diamond step.

            for (int i = 0; i <= 16; i = i + (int)s)
            {
                int ii = i + ss;

                for (int j = 0; j <= 16; j = j + (int)s)
                {
                    int jj = j + ss;

                    for (int n = 1; n <= 2; n++) // We have to do the diamond step twice for each tile.
                    {
                        if (n == 1)
                        {
                            ii = i + ss;
                            jj = j;
                        }
                        else
                        {
                            ii = i;
                            jj = j + ss;
                        }

                        if (jj >= 0 && jj <= 16 && ii >= 0 && ii <= 16)
                        {
                            if (dest[dx + ii][dy + jj] == -5000)
                            {
                                int newheight;

                                newheight = genericdiamond(dx, dy, (int)s, ii, jj, value, min, max, coords, dest);

                                dest[dx + ii][dy + jj] = newheight;
                            }
                        }
                    }
                }
            }

            if (s > 2.0f) // Now halve the size of the tiles.
                s = s / 2.0f;
            else
                s = -1.0f;
        }
    }
}

// This does the square part of the fractal for generic regional maps.

int genericsquare(int dx, int dy, int s, int x, int y, int value, int min, int max, int coords[4][2], vector<vector<int>>& dest)
{
    value = value * 50;

    int dist = s / 2;
    int total = 0;
    int crount = 0;

    coords[0][0] = x - dist;
    coords[0][1] = y - dist;

    coords[1][0] = x + dist;
    coords[1][1] = y - dist;

    coords[2][0] = x + dist;
    coords[2][1] = y + dist;

    coords[3][0] = x - dist;
    coords[3][1] = y + dist;

    for (int n = 0; n <= 3; n++)
    {
        if (coords[n][1] >= 0 && coords[n][1] <= 16 && coords[n][0] >= 0 && coords[n][0] <= 16)
        {
            total = total + dest[dx + coords[n][0]][dy + coords[n][1]];
            crount++;
        }
    }

    total = total / crount;

    int difference = randomsign(random(0, value));

    total = total + difference;

    if (total > max)
        total = max;

    if (total < min)
        total = min;

    return total;
}

// This does the diamond part of the fractal for generic regional maps.

int genericdiamond(int dx, int dy, int s, int x, int y, int value, int min, int max, int coords[4][2], vector<vector<int>>& dest)
{
    value = value * 50;

    int dist = s / 2;
    int total = 0;
    int crount = 0;

    coords[0][0] = x;
    coords[0][1] = y - dist;

    coords[1][0] = x + dist;
    coords[1][1] = y;

    coords[2][0] = x;
    coords[2][1] = y + dist;

    coords[3][0] = x - dist;
    coords[3][1] = y;

    for (int n = 0; n <= 3; n++)
    {
        if (coords[n][1] >= 0 && coords[n][1] <= 16 && coords[n][0] >= 0 && coords[n][0] <= 16)
        {
            total = total + dest[dx + coords[n][0]][dy + coords[n][1]];
            crount++;
        }
    }

    total = total / crount;

    int difference = randomsign(random(0, value));

    total = total + difference;

    if (total > max)
        total = max;

    if (total < min)
        total = min;

    return total;
}

// This smoothes the precipitation maps.

void smoothprecipitation(region& region, int leftx, int lefty, int rightx, int righty, int amount)
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    for (int i = leftx; i <= rightx; i++)
    {
        for (int j = lefty; j <= righty; j++)
        {
            int jantotal = 0;
            int jultotal = 0;
            int crount = 0;

            for (int k = i - amount; k <= i + amount; k++)
            {
                for (int l = j - amount; l <= j + amount; l++)
                {
                    if (k >= 0 && k <= rwidth && l >= 0 && l <= rheight)
                    {
                        jantotal = jantotal + region.janrain(k, l);
                        jultotal = jultotal + region.julrain(k, l);
                        crount++;
                    }
                }
            }
            region.setjanrain(i, j, jantotal / crount);
            region.setjulrain(i, j, jultotal / crount);
        }
    }
}

// This function returns the value of the ice at this point and surrounding points, or -1 if it's mixed.

int getsurroundingice(planet& world, int x, int y)
{
    int width = world.width();
    int height = world.height();

    int permno = 0;
    int tempno = 0;
    int noneno = 0;

    for (int i = x - 1; i <= x + 1; i++)
    {
        int ii = i;

        if (ii<0 || ii>width)
            ii = wrap(ii, width);

        for (int j = y - 1; j <= y + 1; j++)
        {
            if (j >= 0 && j <= height)
            {
                switch (world.seaice(ii, j))
                {
                case 0:
                    noneno++;
                    if (tempno != 0 || permno != 0)
                        return -1;
                    break;

                case 1:
                    tempno++;
                    if (noneno != 0 || permno != 0)
                        return -1;
                    break;

                case 2:
                    permno++;
                    if (noneno != 0 || tempno != 0)
                        return -1;
                }
            }
        }
    }

    if (noneno == 9)
        return 0;

    if (tempno == 9)
        return 1;

    if (permno == 9)
        return 2;

    return -1;
}

// This makes small salt pans, sometimes, on desert tiles.

void makesmallsaltpans(planet& world, region& region, int dx, int dy, int sx, int sy, vector<vector<bool>>& safesaltlakes, boolshapetemplate smalllake[])
{
    if (world.climate(sx, sy) != 5)
        return;

    if (world.riverjan(sx, sy) != 0 || world.riverjul(sx, sy) != 0)
        return;

    if (world.lakesurface(sx, sy) != 0)
        return;

    int width = world.width();

    fast_srand((sy * width + sx) + world.nom(sx, sy) + world.julrain(sx, sy));

    int saltpanchance = 100;

    if (random(1, saltpanchance) != 1)
        return;

    int centrex = dx + random(2, 14);
    int centrey = dy + random(2, 14);

    int diffx = centrex - dx;

    if (diffx > 8)
        diffx = 16 - diffx;

    int diffy = centrey - dy;

    if (diffy > 8)
        diffy = 16 - diffy;

    int size;

    if (diffx < diffy)
        size = diffx;
    else
        size = diffy;

    size = (size * 2) - 3;

    if (size > 12)
        size = 12;

    // size is now the maximum width/height of this salt pan, to avoid going over the borders of the tile.

    int shapenumber = random(0, size - 1);

    vector<vector<int>> thislake(17, vector<int>(17, 0));

    /*
    for (int i=0; i<=16; i++)
    {
        for (int j=0; j<=16; j++)
            thislake[i][j]=0;
    }
    */

    int imheight = smalllake[shapenumber].ysize() - 1;
    int imwidth = smalllake[shapenumber].xsize() - 1;

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
        jstart = imheight;
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

                bool goahead = 1;

                if (region.climate(xx, yy) != 5)
                    goahead = 0;

                if (xx - dx < 0 || xx - dx>16 || yy - dy < 0 || yy - dy>16)
                    goahead = 0;

                if (goahead == 1)
                {
                    region.setspecial(xx, yy, 110);
                    thislake[xx - dx][yy - dy] = 1; // Mark it on the keeping-track array too.
                }
            }
        }
    }

    // Now get the right surface height for the salt pan.

    int surfaceheight = world.nom(sx, sy) - world.riverlandreduce();

    // Now we must alter the terrain height for the salt pan.

    for (int i = 0; i <= 16; i++)
    {
        for (int j = 0; j <= 16; j++)
        {
            if (thislake[i][j] == 1)
            {
                region.setmap(dx + i, dy + j, surfaceheight);
                region.setlakesurface(dx + i, dy + j, surfaceheight);
            }
        }
    }
}

// This function adds salt pans around any salt lakes that are on the tile.

void makesaltpantile(planet& world, region& region, int dx, int dy, int sx, int sy, boolshapetemplate smalllake[])
{
    int width = world.width();
    int height = world.height();
   
    int surfacelevel = world.lakesurface(sx, sy);

    if (surfacelevel == 0)
        surfacelevel = nexttolake(world, sx, sy);

    if (surfacelevel == 0)
        return;

    bool salt = 0;

    for (int i = sx - 1; i <= sx + 1; i++)
    {
        int ii = i;

        if (ii<0 || ii>width)
            ii = wrap(ii, width);

        for (int j = sy - 1; j <= sy + 1; j++)
        {
            if (j >= 0 && j <= height)
            {
                if (world.special(ii, j) == 100) // If there's a salt lake present
                {
                    salt = 1;
                    i = sx + 1;
                    j = sy + 1;
                }
            }
        }
    }

    if (salt == 0)
        return;

    // So there is a salty lake here. Now go over the tile and make salt pans around the lake cells.

    int rwidth = region.rwidth();
    int rheight = region.rheight();

    int saltchance = 8;

    fast_srand((sy * width + sx) + world.nom(sx, sy) + world.julrain(sx, sy));

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (region.truelake(i, j) == 1 && random(1, saltchance) == 1)
                pastesaltpan(region, i, j, surfacelevel, smalllake);
        }
    }
}

// This pastes salt pan around the given point on the regional map.

void pastesaltpan(region& region, int centrex, int centrey, int surfacelevel, boolshapetemplate smalllake[])
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    int depth = surfacelevel - region.map(centrex, centrey);

    int size = 13 - depth;

    if (size > 11)
        size = 11;

    if (size < 0)
        size = 0;

    int shapenumber = random(0, size);

    int imheight = smalllake[shapenumber].ysize() - 1;
    int imwidth = smalllake[shapenumber].xsize() - 1;

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
        jstart = imheight;
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

                if (xx >= 0 && xx <= rwidth && yy >= 0 && yy <= rheight && region.lakesurface(xx, yy) == 0)
                {
                    region.setspecial(xx, yy, 110);
                    region.setmap(xx, yy, surfacelevel);
                    region.setlakesurface(xx, yy, surfacelevel);
                }
            }
        }
    }
}

//This removes any salt pan where there is too much precipitation.

void removewetsaltpans(region& region, int leftx, int lefty, int rightx, int righty)
{
    int maxsaltrain = 30; // Salt lakes may form if there is less rain than this.
    //int minsalttemp=15; // Salt lakes require more heat than this.

    for (int i = leftx; i <= rightx; i++)
    {
        for (int j = lefty; j <= righty; j++)
        {
            if (region.special(i, j) == 110)
            {
                int averain = region.averain(i, j);

                if (averain > maxsaltrain)
                {
                    region.setspecial(i, j, 0);

                    if (region.lakesurface(i, j) != 0)
                    {
                        region.setmap(i, j, region.lakesurface(i, j));
                        region.setlakesurface(i, j, 0);
                    }
                }
            }
        }
    }
}

// This function does the barrier islands for a regional map tile.

void addbarrierislands(planet& world, region& region, int dx, int dy, int sx, int sy, vector<vector<bool>>& riverinlets)
{
    if (world.avetemp(sx, sy) < world.glaciertemp()) // No barrier islands in glacial areas.
        return;

    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();

    fast_srand((sy * width + sx) + world.map(sx, sy) + world.julrain(sx, sy) + world.riverjul(sx, sy));

    int prob = world.tide(sx, sy) * world.tide(sx, sy) + 1; // Probability of barrier islands is based on tides. Make it *much* more unlikely if there is a high tidal range.

    if (random(1, prob) != 1)
        return;

    if (world.mountainheight(sx, sy) > 500) // Don't do this near mountains.
        return;

    if (world.sea(sx, sy) == 1 && world.mountainheight(sx, sy) != 0) // Don't do it near rocky peninsulas either
        return;

    // First, check that there's a sea tile within range.

    bool seatile = 0;

    if (world.sea(sx, sy))
        seatile = 1;
    else
    {
        for (int i = sx - 1; i <= sx + 1; i++)
        {
            int ii = i;

            if (i<0 || ii>width)
                wrap(ii, width);

            for (int j = sy - 1; j < -sy + 1; j++)
            {
                if (j >= 0 && j <= height)
                {
                    if (world.sea(ii, j))
                    {
                        seatile = 1;
                        i = sx + 1;
                        j = sy + 1;
                    }
                }
            }
        }
    }

    if (seatile == 0)
        return;

    //Now, check that we have both land and sea present on this tile.

    bool landfound = 0;
    bool seafound = 0;

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (region.sea(i, j) == 1)
                seafound = 1;
            else
                landfound = 1;

            if (seafound == 1 && landfound == 1)
            {
                i = dx + 16;
                j = dy + 16;
            }
        }
    }

    if (landfound == 0 || seafound == 0)
        return;

    int northcount = 0;
    int southcount = 0;
    int eastcount = 0;
    int westcount = 0;

    for (int n = 1; n <= 14; n++)
    {
        if (region.map(dx + n, dy) > sealevel)
            northcount++;

        if (region.map(dx + n, dy + 14) > sealevel)
            southcount++;

        if (region.map(dx, dy + n) > sealevel)
            westcount++;

        if (region.map(dx + 14, dy + n) > sealevel)
            eastcount++;
    }

    int maxcount = 8;

    if (northcount > maxcount && southcount > maxcount)
        return;

    if (eastcount > maxcount && westcount > maxcount)
        return;

    bool barrier[17][17]; // This will hold the barrier islands. We'll paste them onto the elevation map at the end.
    bool spit[17][17]; // This will hold the spits (stretches of sand linking the islands to the coast).

    for (int i = 0; i <= 16; i++)
    {
        for (int j = 0; j <= 16; j++)
        {
            barrier[i][j] = 0;
            spit[i][j] = 0;
        }
    }

    int rwidth = region.rwidth();
    int rheight = region.rheight();

    int spitchance = 5; // Chance of sending a spit towards the mainland. Higher=less likely.
    int breakchance = 5; // Chance of having no island at this point. Higher=less likely.

    // Land to the north.

    bool opensea = 0; // If this is 0 it means the lagoon might be closed off to the sea (i.e. we've had a spit). If it's 1 it means we've had a break so we know it's open to the sea.

    for (int i = 0; i <= 15; i++)
    {
        int ii = dx + i;
        int jj = -1;
        int j = -1;

        if (region.sea(ii, dy) == 0)
        {
            bool foundsea = 0;
            bool noisland = 0;

            do // Go down from the land edge of the tile until we hit some sea
            {
                j++;
                jj = dy + j;

                if (j > 13)
                {
                    noisland = 1;
                    foundsea = 1;
                }

                if (region.sea(ii, jj) == 1)
                {
                    foundsea = 1;

                    // Now keep going if putting an island here would make awkward diagonals with the land

                    if (ii > 0)
                    {
                        if (region.sea(ii - 1, jj) == 0 && region.sea(ii - 1, jj + 1) == 1)
                            foundsea = 0;

                        if (region.sea(ii + 1, jj) == 0 && region.sea(ii + 1, jj + 1) == 1)
                            foundsea = 0;
                    }
                }
            } while (foundsea == 0);

            if (noisland == 0)
            {
                if (region.sea(ii, jj + 1) == 1 && region.sea(ii, jj + 2) == 1) // If there's room to do an island here
                {
                    if (random(1, breakchance) == 1) // If in fact we're going to have a gap
                        opensea = 1;
                    else
                    {
                        int islandx = i;
                        int islandy = j + 1;

                        if (region.riverdir(dx + islandx, dy + islandy) == 0)
                        {
                            bool goodtogo = 1;

                            for (int n = islandy; n <= 15; n++) // Check that there's no further land beyond this island
                            {
                                if (region.sea(dx + islandx, dy + n) == 0)
                                {
                                    goodtogo = 0;
                                    n = 15;
                                }
                            }

                            if (goodtogo == 1)
                            {
                                if (islandx > 0 && dx + islandx < rwidth)
                                {
                                    if (region.sea(dx + islandx - 1, dy + islandy) == 0 && region.sea(dx + islandx + 1, dy + islandy) == 0)
                                        goodtogo = 0;
                                }
                            }

                            if (goodtogo == 1)
                            {
                                barrier[islandx][islandy] = 1;

                                // Add extra bits of island to join up with previous ones if there are diagonals.

                                if (islandx > 0 && islandy > 0 && islandy < 16)
                                {
                                    if (region.sea(dx + islandx - 1, dy + islandy - 1) == 0 || region.sea(dx + islandx - 1, dy + islandy) == 0 || region.sea(dx + islandx - 1, dy + islandy + 1) == 0)
                                        opensea = 0;

                                    if (random(1, 2) == 1) // Don't always do this
                                    {
                                        if (barrier[islandx - 1][islandy + 1] == 1 && barrier[islandx - 1][islandy] == 0)
                                            barrier[islandx][islandy + 1] = 1;

                                        if (barrier[islandx - 1][islandy - 1] == 1)
                                            barrier[islandx - 1][islandy] = 1;
                                    }
                                }

                                if (opensea == 1 && random(1, spitchance) == 1 && region.riverdir(dx + islandx, dy + islandy - 1) == 0) // If the sea is currently open we could put a spit here.
                                {
                                    if (dx + islandx > 0 && dx + islandx < rwidth && dy + islandy - 2 >= 0)
                                    {
                                        if (region.sea(dx + islandx - 1, dy + islandy - 2) == 1 || region.sea(dx + islandx + 1, dy + islandy - 2) == 1) // Be careful not to enclose some sea with the spit
                                        {
                                            spit[islandx][islandy - 1] = 1;
                                            opensea = 0;
                                        }
                                    }
                                }
                                else
                                {
                                    if (region.map(ii, jj) < sealevel - 3) // Make sure the sea between the island and the mainland is shallow
                                        region.setmap(ii, jj, sealevel - random(1, 3));
                                }
                            }
                            else
                                opensea = 1;
                        }
                        else
                            opensea = 1;
                    }
                }
            }
        }
    }

    // Land to the south.

    opensea = 0; // If this is 0 it means the lagoon might be closed off to the sea (i.e. we've had a spit). If it's 1 it means we've had a break so we know it's open to the sea.

    for (int i = 0; i <= 15; i++)
    {
        int ii = dx + i;
        int jj = -1;
        int j = 16;

        if (region.sea(ii, dy + 16) == 0)
        {
            bool foundsea = 0;
            bool noisland = 0;

            do // Go up from the land edge of the tile until we hit some sea
            {
                j--;
                jj = dy + j;

                if (j < 2)
                {
                    noisland = 1;
                    foundsea = 1;
                }

                if (region.sea(ii, jj) == 1)
                {
                    foundsea = 1;

                    // Now keep going if putting an island here would make awkward diagonals with the land

                    if (ii > 0)
                    {
                        if (region.sea(ii - 1, jj) == 0 && region.sea(ii - 1, jj - 1) == 1)
                            foundsea = 0;

                        if (region.sea(ii + 1, jj) == 0 && region.sea(ii + 1, jj - 1) == 1)
                            foundsea = 0;
                    }
                }
            } while (foundsea == 0);

            /*

             if (opensea==0 || region.sea(ii+1,jj+1)==0) // Avoid closing off lagoons by going into the land
             noisland=1;
             */

            if (noisland == 0)
            {
                if (region.sea(ii, jj - 1) == 1 && region.sea(ii, jj - 2) == 1) // If there's room to do an island here
                {
                    if (random(1, breakchance) == 1) // If in fact we're going to have a gap
                        opensea = 1;
                    else
                    {
                        int islandx = i;
                        int islandy = j - 1;

                        if (region.riverdir(dx + islandx, dy + islandy) == 0)
                        {
                            bool goodtogo = 1;

                            for (int n = islandy; n >= 0; n--) // Check that there's no further land beyond this island
                            {
                                if (region.sea(dx + islandx, dy + n) == 0)
                                {
                                    goodtogo = 0;
                                    n = 0;
                                }
                            }

                            if (goodtogo == 1)
                            {
                                if (islandx > 0 && dx + islandx < rwidth)
                                {
                                    if (region.sea(dx + islandx - 1, dy + islandy) == 0 && region.sea(dx + islandx + 1, dy + islandy) == 0)
                                        goodtogo = 0;
                                }
                            }

                            if (goodtogo == 1)
                            {
                                barrier[islandx][islandy] = 1;

                                // Add extra bits of island to join up with previous ones if there are diagonals.

                                if (islandx > 0 && islandy > 0 && islandy < 16)
                                {
                                    if (region.sea(dx + islandx - 1, dy + islandy - 1) == 0 || region.sea(dx + islandx - 1, dy + islandy) == 0 || region.sea(dx + islandx - 1, dy + islandy + 1) == 0)
                                        opensea = 0;

                                    if (random(1, 2) == 1) // Don't always do this
                                    {
                                        if (barrier[islandx - 1][islandy - 1] == 1 && barrier[islandx - 1][islandy] == 0)
                                            barrier[islandx][islandy - 1] = 1;

                                        if (barrier[islandx - 1][islandy + 1] == 1)
                                            barrier[islandx - 1][islandy] = 1;
                                    }
                                }

                                if (opensea == 1 && random(1, spitchance) == 1 && region.riverdir(dx + islandx, dy + islandy + 1) == 0) // If the sea is currently open we could put a spit here.
                                {
                                    if (dx + islandx > 0 && dx + islandx < rwidth && dy + islandy + 2 <= rheight)
                                    {
                                        if (region.sea(dx + islandx - 1, dy + islandy + 2) == 1 || region.sea(dx + islandx + 1, dy + islandy + 2) == 1) // Be careful not to enclose some sea with the spit
                                        {
                                            spit[islandx][islandy + 1] = 1;
                                            opensea = 0;
                                        }
                                    }
                                }
                                else
                                {
                                    if (region.map(ii, jj) < sealevel - 3) // Make sure the sea between the island and the mainland is shallow
                                        region.setmap(ii, jj, sealevel - random(1, 3));
                                }
                            }
                            else
                                opensea = 1;
                        }
                        else
                            opensea = 1;
                    }
                }
            }
        }
    }

    // Now put the new islands onto the elevation map and clear the island map.

    for (short i = 0; i <= 16; i++)
    {
        int ii = dx + i;

        for (short j = 0; j <= 16; j++)
        {
            int jj = dy + j;

            if (barrier[i][j] == 1)
            {
                if (riverinlets[ii][jj] == 0)
                {
                    region.setmap(ii, jj, sealevel + 1);
                    region.setbarrierisland(ii, jj, 1);

                    region.setriverdir(ii, jj, 0);
                    region.setriverjan(ii, jj, 0);
                    region.setriverjul(ii, jj, 0);
                }

                barrier[i][j] = 0;
            }

            if (spit[i][j] == 1)
            {
                if (riverinlets[ii][jj] == 0)
                {
                    region.setmap(ii, jj, sealevel + 1);
                    region.setbarrierisland(ii, jj, 1);
                    region.setsand(ii, jj, 1);

                    region.setriverdir(ii, jj, 0);
                    region.setriverjan(ii, jj, 0);
                    region.setriverjul(ii, jj, 0);
                }

                spit[i][j] = 0;
            }
        }
    }

    // Land to the west.

    opensea = 0; // If this is 0 it means the lagoon might be closed off to the sea (i.e. we've had a spit). If it's 1 it means we've had a break so we know it's open to the sea.

    for (int j = 0; j <= 15; j++)
    {
        int jj = dy + j;
        int ii = -1;
        int i = -1;

        if (region.sea(dx, jj) == 0)
        {
            bool foundsea = 0;
            bool noisland = 0;

            do // Go down from the land edge of the tile until we hit some sea
            {
                i++;
                ii = dx + i;

                if (i > 13)
                {
                    noisland = 1;
                    foundsea = 1;
                }

                if (region.sea(ii, jj) == 1)
                {
                    foundsea = 1;

                    // Now keep going if putting an island here would make awkward diagonals with the land

                    if (jj > 0)
                    {
                        if (region.sea(ii, jj - 1) == 0 && region.sea(ii + 1, jj - 1) == 1)
                            foundsea = 0;

                        if (region.sea(ii, jj + 1) == 0 && region.sea(ii + 1, jj + 1) == 1)
                            foundsea = 0;
                    }
                }
            } while (foundsea == 0);

            /*

             if (opensea==0 || region.sea(ii+1,jj+1)==0) // Avoid closing off lagoons by going into the land
             noisland=1;
             */

            if (noisland == 0)
            {
                if (region.sea(ii + 1, jj) == 1 && region.sea(ii + 2, jj) == 1) // If there's room to do an island here
                {
                    if (random(1, breakchance) == 1) // If in fact we're going to have a gap
                        opensea = 1;
                    else
                    {
                        int islandx = i + 1;
                        int islandy = j;

                        if (region.riverdir(dx + islandx, dy + islandy) == 0)
                        {
                            bool goodtogo = 1;

                            for (int n = islandx; n <= 15; n++) // Check that there's no further land beyond this island
                            {
                                if (region.sea(dx + n, dy + islandy) == 0)
                                {
                                    goodtogo = 0;
                                    n = 15;
                                }
                            }

                            if (goodtogo == 1)
                            {
                                if (islandy > 0 && dy + islandy < rheight)
                                {
                                    if (region.sea(dx + islandx, dy + islandy - 1) == 0 && region.sea(dx + islandx, dy + islandy + 1) == 0)
                                        goodtogo = 0;
                                }
                            }

                            if (goodtogo == 1)
                            {
                                barrier[islandx][islandy] = 1;

                                // Add extra bits of island to join up with previous ones if there are diagonals.

                                if (islandx > 0 && islandx < 16 && islandy>0)
                                {
                                    if (region.sea(dx + islandx - 1, dy + islandy - 1) == 0 || region.sea(dx + islandx, dy + islandy - 1) == 0 || region.sea(dx + islandx + 1, dy + islandy - 1) == 0)
                                        opensea = 0;

                                    if (random(1, 2) == 1) // Don't always do this
                                    {
                                        if (barrier[islandx + 1][islandy - 1] == 1 && barrier[islandx][islandy - 1] == 0)
                                            barrier[islandx + 1][islandy] = 1;

                                        if (barrier[islandx - 1][islandy - 1] == 1)
                                            barrier[islandx][islandy - 1] = 1;
                                    }
                                }

                                if (opensea == 1 && random(1, spitchance) == 1 && region.riverdir(dx + islandx - 1, dy + islandy) == 0) // If the sea is currently open we could put a spit here.
                                {
                                    if (dx + islandx - 2 > 0 && dy + islandy > 0 && dy + islandy < rheight)
                                    {
                                        if (region.sea(dx + islandx - 2, dy + islandy - 1) == 1 || region.sea(dx + islandx - 2, dy + islandy + 1) == 1) // Be careful not to enclose some sea with the spit
                                        {
                                            spit[islandx - 1][islandy] = 1;
                                            opensea = 0;
                                        }

                                    }
                                }
                                else
                                {
                                    if (region.map(ii, jj) < sealevel - 3) // Make sure the sea between the island and the mainland is shallow
                                        region.setmap(ii, jj, sealevel - random(1, 3));
                                }
                            }
                            else
                                opensea = 1;
                        }
                        else
                            opensea = 1;
                    }
                }
            }
        }
    }

    // Land to the east.

    opensea = 0; // If this is 0 it means the lagoon might be closed off to the sea (i.e. we've had a spit). If it's 1 it means we've had a break so we know it's open to the sea.

    for (int j = 0; j <= 15; j++)
    {
        int jj = dy + j;
        int ii = -1;
        int i = 16;

        if (region.sea(dx + 16, jj) == 0)
        {
            bool foundsea = 0;
            bool noisland = 0;

            do // Go up from the land edge of the tile until we hit some sea
            {
                i--;
                ii = dx + i;

                if (i < 2)
                {
                    noisland = 1;
                    foundsea = 1;
                }

                if (region.sea(ii, jj) == 1)
                {
                    foundsea = 1;

                    // Now keep going if putting an island here would make awkward diagonals with the land

                    if (jj > 0)
                    {
                        if (region.sea(ii, jj - 1) == 0 && region.sea(ii - 1, jj - 1) == 1)
                            foundsea = 0;

                        if (region.sea(ii, jj + 1) == 0 && region.sea(ii - 1, jj + 1) == 1)
                            foundsea = 0;
                    }
                }
            } while (foundsea == 0);

            /*

             if (opensea==0 || region.sea(ii+1,jj+1)==0) // Avoid closing off lagoons by going into the land
             noisland=1;
             */

            if (noisland == 0)
            {
                if (region.sea(ii - 1, jj) == 1 && region.sea(ii - 2, jj) == 1) // If there's room to do an island here
                {
                    if (random(1, breakchance) == 1) // If in fact we're going to have a gap
                        opensea = 1;
                    else
                    {
                        int islandx = i - 1;
                        int islandy = j;

                        if (region.riverdir(dx + islandx, dy + islandy) == 0)
                        {
                            bool goodtogo = 1;

                            for (int n = islandx; n >= 0; n--) // Check that there's no further land beyond this island
                            {
                                if (region.sea(dx + n, dy + islandy) == 0)
                                {
                                    goodtogo = 0;
                                    n = 0;
                                }
                            }

                            if (goodtogo == 1)
                            {
                                if (islandy > 0 && dy + islandy < rheight)
                                {
                                    if (region.sea(dx + islandx, dy + islandy - 1) == 0 && region.sea(dx + islandx, dy + islandy + 1) == 0)
                                        goodtogo = 0;
                                }
                            }

                            if (goodtogo == 1)
                            {
                                barrier[islandx][islandy] = 1;

                                // Add extra bits of island to join up with previous ones if there are diagonals.

                                if (islandx > 0 && islandx < 16 && islandy>0)
                                {
                                    if (region.sea(dx + islandx - 1, dy + islandy - 1) == 0 || region.sea(dx + islandx, dy + islandy - 1) == 0 || region.sea(dx + islandx + 1, dy + islandy - 1) == 0)
                                        opensea = 0;

                                    if (random(1, 2) == 1) // Don't always do this
                                    {
                                        if (barrier[islandx - 1][islandy - 1] == 1 && barrier[islandx][islandy - 1] == 0)
                                            barrier[islandx - 1][islandy] = 1;

                                        if (barrier[islandx + 1][islandy - 1] == 1)
                                            barrier[islandx][islandy - 1] = 1;
                                    }
                                }

                                if (opensea == 1 && random(1, spitchance) == 1 && region.riverdir(dx + islandx + 1, dy + islandy) == 0) // If the sea is currently open we could put a spit here.
                                {
                                    if (dy + islandy > 0 && dy + islandy < rheight && dx + islandx + 2 < rwidth)
                                    {
                                        if (region.sea(dx + islandx + 2, dy + islandy - 1) == 1 || region.sea(dx + islandx + 2, dy + islandy + 1) == 1) // Be careful not to enclose some sea with the spit
                                        {
                                            spit[islandx + 1][islandy] = 1;
                                            opensea = 0;
                                        }

                                    }
                                }
                                else
                                {
                                    if (region.map(ii, jj) < sealevel - 3) // Make sure the sea between the island and the mainland is shallow
                                        region.setmap(ii, jj, sealevel - random(1, 3));
                                }
                            }
                            else
                                opensea = 1;
                        }
                        else
                            opensea = 1;
                    }
                }
            }
        }
    }

    // Now put the new islands onto the elevation map.

    for (short i = 0; i <= 16; i++)
    {
        int ii = dx + i;

        for (short j = 0; j <= 16; j++)
        {
            int jj = dy + j;

            if (barrier[i][j] == 1)
            {
                if (riverinlets[ii][jj] == 0)
                {
                    region.setmap(ii, jj, sealevel + 1);
                    region.setbarrierisland(ii, jj, 1);

                    region.setriverdir(ii, jj, 0);
                    region.setriverjan(ii, jj, 0);
                    region.setriverjul(ii, jj, 0);
                }
            }

            if (spit[i][j] == 1)
            {
                if (riverinlets[ii][jj] == 0)
                {
                    region.setmap(ii, jj, sealevel + 1);
                    region.setbarrierisland(ii, jj, 1);
                    region.setsand(ii, jj, 1);

                    region.setriverdir(ii, jj, 0);
                    region.setriverjan(ii, jj, 0);
                    region.setriverjul(ii, jj, 0);
                }
            }
        }
    }
}

// This function adds glaciers to the regional specials map.

void addregionalglaciers(planet& world, region& region, int dx, int dy, int sx, int sy)
{
    int width = world.width();
    int height = world.height();
    int glaciertemp = world.glaciertemp();

    // First, lake ice. Do this at the level of individual regional cells.

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (region.truelake(i, j) != 0 && region.maxtemp(dx, dy) <= glaciertemp)
                region.setspecial(i, j, 140);
        }
    }

    // Now rivers. Do this at the level of tiles.

    for (int i = sx - 1; i <= sx + 1; i++)
    {
        int ii = i;

        if (ii<0 || ii>width)
            ii = wrap(ii, width);

        for (int j = sy - 1; j <= sy + 1; j++)
        {
            if (j >= 0 && j <= height)
            {
                if (world.maxtemp(ii, j) > glaciertemp)
                    return;
            }
        }
    }

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (region.riverdir(i, j) != 0 || region.fakedir(i, j) != 0)
            {
                if (region.sea(i, j) == 0)
                    region.setspecial(i, j, 140);
            }
        }
    }
}

// This function creates templates for the rift lakes.

void makeriftlaketemplates(planet& world, region& region, int dx, int dy, int sx, int sy, int extra, vector<vector<bool>>& riftlakemap)
{
    fast_srand(sy * world.width() + sx + world.map(sx, sy) + world.julrain(sx, sy));

    int dextra = extra * 16; // Amount to add to ddx and ddy to position everything towards the middle of the rift lake array.

    dx = dx + dextra;
    dy = dy + dextra;

    // First, we need to work out how many tiles this rift lake extends over and where it ends.

    int width = world.width();
    int totalpossnodes = 1000;

    vector<vector<int>> bordernodes(totalpossnodes, vector<int>(2, -1)); // This will hold the coordinates of the key nodes of the lake border.

    int totalnodes = 0; // Number of nodes this lake coast will have.

    int endx = 0;
    int endy = 0; // Global coordinates of the last tile containing lake.

    int ddx = 0;
    int ddy = 0; // Equivalent of dx,dy for tiles other than the starting one.

    int lakelength = 1;
    bool keepgoing = 1;

    int x = sx;
    int y = sy;

    do
    {
        int dir = world.riverdir(x, y);

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

        if (world.riftlakesurface(newx, newy) == 0)
        {
            keepgoing = 0;
            endx = x;
            endy = y;
        }
        else
        {
            lakelength++;
            x = newx;
            y = newy;
        }

    } while (keepgoing == 1);

    // Every tile will have four nodes, two for each side of the lake. The start and end tiles will each have an additional node for the end of the lake.

    totalnodes = lakelength * 4 + 2;

    // First, set the nodes at the ends of the lake.

    // The starting one first.

    twointegers dest = getupstreamcell(world, sx, sy);

    int dir = getdir(sx, sy, dest.x, dest.y); // dir is the direction to the upstream cell.

    if (dir == 2)
    {
        bordernodes[0][0] = dx + 16;
        bordernodes[0][1] = dy;
    }

    if (dir == 4)
    {
        bordernodes[0][0] = dx + 16;
        bordernodes[0][1] = dy + 16;
    }

    if (dir == 6)
    {
        bordernodes[0][0] = dx;
        bordernodes[0][1] = dy + 16;
    }

    if (dir == 8)
    {
        bordernodes[0][0] = dx;
        bordernodes[0][1] = dy;
    }

    if (bordernodes[0][0] == -1) // If we didn't get one
    {
        if (dir == 1)
        {
            bordernodes[0][0] = dx + random(1, 15);
            bordernodes[0][1] = dy - 1;

            for (int i = dx + 1; i <= dx + 15; i++)
            {
                if (region.riverdir(i, dy - 1) != 0)
                {
                    bordernodes[0][0] = i;
                    bordernodes[0][1] = dy - 1;
                    i = dx + 15;
                }
            }
        }

        if (dir == 3)
        {
            bordernodes[0][0] = dx + 17;
            bordernodes[0][1] = dy + random(1, 15);

            for (int j = dy + 1; j <= dy + 15; j++)
            {
                if (region.riverdir(dx + 17, j) != 0)
                {
                    bordernodes[0][0] = dx + 17;
                    bordernodes[0][1] = j;
                    j = dy + 15;
                }
            }
        }

        if (dir == 5)
        {
            bordernodes[0][0] = dx + random(1, 15);
            bordernodes[0][1] = dy + 17;

            for (int i = dx + 1; i <= dx + 15; i++)
            {
                if (region.riverdir(i, dy + 17) != 0)
                {
                    bordernodes[0][0] = i;
                    bordernodes[0][1] = dy + 17;
                    i = dx + 15;
                }
            }
        }

        if (dir == 7)
        {
            bordernodes[0][0] = dx - 1;
            bordernodes[0][1] = dy + random(1, 15);

            for (int j = dy + 1; j <= dy + 15; j++)
            {
                if (region.riverdir(dx - 1, j) != 0)
                {
                    bordernodes[0][0] = dx - 1;
                    bordernodes[0][1] = j;
                    j = dy + 15;
                }
            }
        }
    }

    // Now for the end one.

    int node = lakelength * 2 + 1; // This is the number of the node at the end of the lake.

    dir = world.riverdir(endx, endy);

    ddx = dx - (sx - endx) * 16;
    ddy = dy - (sy - endy) * 16;

    if (dir == 2)
    {
        bordernodes[node][0] = ddx + 16;
        bordernodes[node][1] = ddy;
    }

    if (dir == 4)
    {
        bordernodes[node][0] = ddx + 16;
        bordernodes[node][1] = ddy + 16;
    }

    if (dir == 6)
    {
        bordernodes[node][0] = ddx;
        bordernodes[node][1] = ddy + 16;
    }

    if (dir == 8)
    {
        bordernodes[node][0] = ddx;
        bordernodes[node][1] = ddy;
    }

    if (bordernodes[node][0] == -1) // If we didn't get one
    {
        if (dir == 1)
        {
            bordernodes[node][0] = ddx + random(1, 15);
            bordernodes[node][1] = ddy - 1;

            for (int i = ddx + 1; i <= ddx + 15; i++)
            {
                if (region.riverdir(i, ddy - 1) != 0)
                {
                    bordernodes[node][0] = i;
                    bordernodes[node][1] = ddy - 1;
                    i = ddx + 15;
                }
            }
        }

        if (dir == 3)
        {
            bordernodes[node][0] = ddx + 17;
            bordernodes[node][1] = ddy + random(1, 15);

            for (int j = ddy + 1; j <= ddy + 15; j++)
            {
                if (region.riverdir(ddx + 17, j) != 0)
                {
                    bordernodes[node][0] = ddx + 17;
                    bordernodes[node][1] = j;
                    j = ddy + 15;
                }
            }
        }

        if (dir == 5)
        {
            bordernodes[node][0] = ddx + random(1, 15);
            bordernodes[node][1] = ddy + 17;

            for (int i = ddx + 1; i <= ddx + 15; i++)
            {
                if (region.riverdir(i, ddy + 17) != 0)
                {
                    bordernodes[node][0] = i;
                    bordernodes[node][1] = ddy + 17;
                    i = ddx + 15;
                }
            }
        }

        if (dir == 7)
        {
            bordernodes[node][0] = ddx - 1;
            bordernodes[node][1] = ddy + random(1, 15);

            for (int j = ddy + 1; j <= ddy + 15; j++)
            {
                if (region.riverdir(ddx - 1, j) != 0)
                {
                    bordernodes[node][0] = ddx - 1;
                    bordernodes[node][1] = j;
                    j = ddy + 15;
                }
            }
        }
    }

    // Now we do the nodes along the sides of the lake. We'll imagine the outline to be going anti-clockwise. So for each tile, there will be two nodes that the outline passes through first (outgoing) on its way from the start, and another two nodes that it passes through again later (incoming) on its way back to the start.

    x = sx;
    y = sy;

    int minvar = 0; // Minimum variation on each node's position.
    int maxvar = 3; // Maximum variation on each node's position.

    int thisnode = 1; // The number of the first node on the outgoing side of this tile.
    int laternode = totalnodes - 2; // The number of the first node on the incoming side of this tile.
    int margin = 3; // Nodes must be no closer to the middle of the tile than this.

    int extrawidth = 0; //random(0,maxextrawidth); // This will be added to the incoming nodes' positions.

    keepgoing = 1;

    do
    {
        ddx = dx - (sx - x) * 16;
        ddy = dy - (sy - y) * 16;

        int dir = world.riverdir(x, y);

        if (dir == 1)
        {
            bordernodes[thisnode][0] = ddx + 15;
            bordernodes[thisnode][1] = ddy + 12;

            bordernodes[thisnode + 1][0] = ddx + 15;
            bordernodes[thisnode + 1][1] = ddy + 4;

            bordernodes[laternode][0] = ddx + 1 - extrawidth;
            bordernodes[laternode][1] = ddy + 4;

            bordernodes[laternode + 1][0] = ddx + 1 - extrawidth;
            bordernodes[laternode + 1][1] = ddy + 8;
        }

        if (dir == 2)
        {
            bordernodes[thisnode][0] = ddx + 10;
            bordernodes[thisnode][1] = ddy + 15;

            bordernodes[thisnode + 1][0] = ddx + 15;
            bordernodes[thisnode + 1][1] = ddy + 10;

            bordernodes[laternode][0] = ddx + 5 - (extrawidth / 2);
            bordernodes[laternode][1] = ddy + 1 - (extrawidth / 2);

            bordernodes[laternode + 1][0] = ddx + 1 - (extrawidth / 2);
            bordernodes[laternode + 1][1] = ddy + 5 - (extrawidth / 2);
        }

        if (dir == 3)
        {
            bordernodes[thisnode][0] = ddx + 4;
            bordernodes[thisnode][1] = ddy + 15;

            bordernodes[thisnode + 1][0] = ddx + 12;
            bordernodes[thisnode + 1][1] = ddy + 15;

            bordernodes[laternode][0] = ddx + 12;
            bordernodes[laternode][1] = ddy + 1 - extrawidth;

            bordernodes[laternode + 1][0] = ddx + 4;
            bordernodes[laternode + 1][1] = ddy + 1 - extrawidth;
        }

        if (dir == 4)
        {
            bordernodes[thisnode][0] = ddx + 1;
            bordernodes[thisnode][1] = ddy + 10;

            bordernodes[thisnode + 1][0] = ddx + 5;
            bordernodes[thisnode + 1][1] = ddy + 15;

            bordernodes[laternode][0] = ddx + 15 + (extrawidth / 2);
            bordernodes[laternode][1] = ddy + 5 - (extrawidth / 2);

            bordernodes[laternode + 1][0] = ddx + 10 + (extrawidth / 2);
            bordernodes[laternode + 1][1] = ddy + 1 - (extrawidth / 2);
        }

        if (dir == 5)
        {
            bordernodes[thisnode][0] = ddx + 1;
            bordernodes[thisnode][1] = ddy + 4;

            bordernodes[thisnode + 1][0] = ddx + 1;
            bordernodes[thisnode + 1][1] = ddy + 12;

            bordernodes[laternode][0] = ddx + 15 + extrawidth;
            bordernodes[laternode][1] = ddy + 12;

            bordernodes[laternode + 1][0] = ddx + 15 + extrawidth;
            bordernodes[laternode + 1][1] = ddy + 4;
        }

        if (dir == 6)
        {
            bordernodes[thisnode][0] = ddx + 5;
            bordernodes[thisnode][1] = ddy + 1;

            bordernodes[thisnode + 1][0] = ddx + 1;
            bordernodes[thisnode + 1][1] = ddy + 5;

            bordernodes[laternode][0] = ddx + 10 + (extrawidth / 2);
            bordernodes[laternode][1] = ddy + 15 + (extrawidth / 2);

            bordernodes[laternode + 1][0] = ddx + 15 + (extrawidth / 2);
            bordernodes[laternode + 1][1] = ddy + 10 + (extrawidth / 2);
        }

        if (dir == 7)
        {
            bordernodes[thisnode][0] = ddx + 12;
            bordernodes[thisnode][1] = ddy + 1;

            bordernodes[thisnode + 1][0] = ddx + 4;
            bordernodes[thisnode + 1][1] = ddy + 1;

            bordernodes[laternode][0] = ddx + 4;
            bordernodes[laternode][1] = ddy + 15 + extrawidth;

            bordernodes[laternode + 1][0] = ddx + 12;
            bordernodes[laternode + 1][1] = ddy + 15 + extrawidth;
        }

        if (dir == 8)
        {
            bordernodes[thisnode][0] = ddx + 15;
            bordernodes[thisnode][1] = ddy + 5;

            bordernodes[thisnode + 1][0] = ddx + 10;
            bordernodes[thisnode + 1][1] = ddy + 1;

            bordernodes[laternode][0] = ddx + 1 - (extrawidth / 2);
            bordernodes[laternode][1] = ddy + 10 + (extrawidth / 2);

            bordernodes[laternode + 1][0] = ddx + 5 - (extrawidth / 2);
            bordernodes[laternode + 1][1] = ddy + 15 + (extrawidth / 2);
        }

        // Apply some random shifts to these nodes.

        int xshift1 = randomsign(random(minvar, maxvar));
        int yshift1 = randomsign(random(minvar, maxvar));

        int xshift2 = randomsign(random(minvar, maxvar));
        int yshift2 = randomsign(random(minvar, maxvar));

        int new1x = bordernodes[thisnode][0] + xshift1;
        int new1y = bordernodes[thisnode][1] + yshift1;

        int new2x = bordernodes[thisnode + 1][0] + xshift2;
        int new2y = bordernodes[thisnode + 1][1] + yshift2;

        int new3x = bordernodes[laternode][0] + xshift2;
        int new3y = bordernodes[laternode][1] + yshift2;

        int new4x = bordernodes[laternode + 1][0] + xshift1;
        int new4y = bordernodes[laternode + 1][1] + yshift1;

        // Now make sure that these don't shift them too much - keep the middle of the tile clear, so the flood fill will work properly later on.

        if (bordernodes[thisnode][0] <= ddx + 8)
        {
            if (new1x > ddx + 8 - margin)
                new1x = ddx + 8 - margin;
        }
        else
        {
            if (new1x < ddx + 8 + margin)
                new1x = ddx + 8 + margin;
        }

        if (bordernodes[thisnode][1] <= ddy + 8)
        {
            if (new1y > ddy + 8 - margin)
                new1y = ddy + 8 - margin;
        }
        else
        {
            if (new1y < ddy + 8 + margin)
                new1y = ddy + 8 + margin;
        }

        if (bordernodes[thisnode + 1][0] <= ddx + 8)
        {
            if (new2x > ddx + 8 - margin)
                new2x = ddx + 8 - margin;
        }
        else
        {
            if (new2x < ddx + 8 + margin)
                new2x = ddx + 8 + margin;
        }

        if (bordernodes[thisnode + 1][1] <= ddy + 8)
        {
            if (new2y > ddy + 8 - margin)
                new2y = ddy + 8 - margin;
        }
        else
        {
            if (new2y < ddy + 8 + margin)
                new2y = ddy + 8 + margin;
        }

        if (bordernodes[laternode][0] <= ddx + 8)
        {
            if (new3x > ddx + 8 - margin)
                new3x = ddx + 8 - margin;
        }
        else
        {
            if (new3x < ddx + 8 + margin)
                new3x = ddx + 8 + margin;
        }

        if (bordernodes[laternode][1] <= ddy + 8)
        {
            if (new3y > ddy + 8 - margin)
                new3y = ddy + 8 - margin;
        }
        else
        {
            if (new3y < ddy + 8 + margin)
                new3y = ddy + 8 + margin;
        }

        if (bordernodes[laternode + 1][0] <= ddx + 8)
        {
            if (new4x > ddx + 8 - margin)
                new4x = ddx + 8 - margin;
        }
        else
        {
            if (new4x < ddx + 8 + margin)
                new4x = ddx + 8 + margin;
        }

        if (bordernodes[laternode + 1][1] <= ddy + 8)
        {
            if (new4y > ddy + 8 - margin)
                new4y = ddy + 8 - margin;
        }
        else
        {
            if (new4y < ddy + 8 + margin)
                new4y = ddy + 8 + margin;
        }

        bordernodes[thisnode][0] = new1x;
        bordernodes[thisnode][1] = new1y;

        bordernodes[thisnode + 1][0] = new2x;
        bordernodes[thisnode + 1][1] = new2y;

        bordernodes[laternode][0] = new3x;
        bordernodes[laternode][1] = new3y;

        bordernodes[laternode + 1][0] = new4x;
        bordernodes[laternode + 1][1] = new4y;

        if (x == endx && y == endy)
            keepgoing = 0;
        else
        {
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

            thisnode = thisnode + 2;
            laternode = laternode - 2;

            /*

             if (random(1,widthchangechance)==1)
             {
             extrawidth=extrawidth+randomsign(1);

             if (extrawidth<0)
             extrawidth=0;

             if (extrawidth>maxextrawidth)
             extrawidth=maxextrawidth;
             }

             */
        }

    } while (keepgoing == 1);

    // Now copy the first node onto the end of our node list.

    for (int n = 0; n < totalpossnodes; n++)
    {
        if (bordernodes[n][0] == -1)
        {
            bordernodes[n][0] = bordernodes[0][0];
            bordernodes[n][1] = bordernodes[0][1];
            n = totalpossnodes;
        }
    }

    // Now adjust the ones near the ends to make a more tapered shape.

    int xshift1 = randomsign(random(minvar, maxvar));
    int yshift1 = randomsign(random(minvar, maxvar));

    int xshift2 = randomsign(random(minvar, maxvar));
    int yshift2 = randomsign(random(minvar, maxvar));

    int mintaper = 2; //3;
    int maxtaper = 4; //5; // The lower it is, the thinner the taper.

    int taper = random(mintaper, maxtaper);
    bordernodes[2][0] = (bordernodes[2][0] * taper + bordernodes[1][0] + bordernodes[0][0]) / (taper + 2) + xshift1;
    bordernodes[2][1] = (bordernodes[2][1] * taper + bordernodes[1][1] + bordernodes[0][1]) / (taper + 2) + yshift1;

    taper = random(mintaper, maxtaper);
    bordernodes[1][0] = (bordernodes[1][0] * taper + bordernodes[2][0] + bordernodes[0][0]) / (taper + 2) + xshift2;
    bordernodes[1][1] = (bordernodes[1][1] * taper + bordernodes[2][1] + bordernodes[0][1]) / (taper + 2) + yshift2;

    taper = random(mintaper, maxtaper);
    bordernodes[totalnodes - 1][0] = (bordernodes[totalnodes - 1][0] * taper + bordernodes[totalnodes - 2][0] + bordernodes[totalnodes][0]) / (taper + 2) + xshift2;
    bordernodes[totalnodes - 1][1] = (bordernodes[totalnodes - 1][1] * taper + bordernodes[totalnodes - 2][1] + bordernodes[totalnodes][1]) / (taper + 2) + yshift2;

    taper = random(mintaper, maxtaper);
    bordernodes[totalnodes - 2][0] = (bordernodes[totalnodes - 2][0] * taper + bordernodes[totalnodes - 1][0] + bordernodes[totalnodes][0]) / (taper + 2) + xshift1;
    bordernodes[totalnodes - 2][1] = (bordernodes[totalnodes - 2][1] * taper + bordernodes[totalnodes - 1][1] + bordernodes[totalnodes][1]) / (taper + 2) + yshift1;

    xshift1 = randomsign(random(minvar, maxvar));
    yshift1 = randomsign(random(minvar, maxvar));

    xshift2 = randomsign(random(minvar, maxvar));
    yshift2 = randomsign(random(minvar, maxvar));

    taper = random(mintaper, maxtaper);
    bordernodes[lakelength * 2 - 1][0] = (bordernodes[lakelength * 2 - 1][0] * taper + bordernodes[lakelength * 2][0] + bordernodes[lakelength * 2 + 1][0]) / (taper + 2) + xshift1;
    bordernodes[lakelength * 2 - 1][1] = (bordernodes[lakelength * 2 - 1][1] * taper + bordernodes[lakelength * 2][1] + bordernodes[lakelength * 2 + 1][1]) / (taper + 2) + yshift1;

    taper = random(mintaper, maxtaper);
    bordernodes[lakelength * 2][0] = (bordernodes[lakelength * 2][0] * taper + bordernodes[lakelength * 2 - 1][0] + bordernodes[lakelength * 2 + 1][0]) / (taper + 2) + xshift2;
    bordernodes[lakelength * 2][1] = (bordernodes[lakelength * 2][1] * taper + bordernodes[lakelength * 2 - 1][1] + bordernodes[lakelength * 2 + 1][1]) / (taper + 2) + yshift2;

    taper = random(mintaper, maxtaper);
    bordernodes[lakelength * 2 + 3][0] = (bordernodes[lakelength * 2 + 3][0] * taper + bordernodes[lakelength * 2 + 2][0] + bordernodes[lakelength * 2 + 1][0]) / (taper + 2) + xshift2;
    bordernodes[lakelength * 2 + 3][1] = (bordernodes[lakelength * 2 + 3][1] * taper + bordernodes[lakelength * 2 + 2][1] + bordernodes[lakelength * 2 + 1][1]) / (taper + 2) + yshift2;

    taper = random(mintaper, maxtaper);
    bordernodes[lakelength * 2 + 2][0] = (bordernodes[lakelength * 2 + 2][0] * taper + bordernodes[lakelength * 2 + 3][0] + bordernodes[lakelength * 2 + 1][0]) / (taper + 2) + xshift1;
    bordernodes[lakelength * 2 + 2][1] = (bordernodes[lakelength * 2 + 2][1] * taper + bordernodes[lakelength * 2 + 3][1] + bordernodes[lakelength * 2 + 1][1]) / (taper + 2) + yshift1;

    // Now remove totally straight lines.

    int minadjust = 2;
    int maxadjust = 4;

    for (node = 1; node < totalnodes; node++)
    {
        if (bordernodes[node][0] == bordernodes[node + 1][0])
        {
            int amount = randomsign(random(minadjust, maxadjust));

            bordernodes[node][0] = bordernodes[node][0] + amount;
            bordernodes[node + 1][0] = bordernodes[node + 1][0] - amount;
        }

        if (bordernodes[node][1] == bordernodes[node + 1][1])
        {
            int amount = randomsign(random(minadjust, maxadjust));

            bordernodes[node][1] = bordernodes[node][1] + amount;
            bordernodes[node + 1][1] = bordernodes[node + 1][1] - amount;
        }
    }

    // Now we've got all our nodes. We need to draw splines between them.

    int midvar = 2; //1; // Standard variation for the interpolated nodes.

    twofloats pt, mm1, mm2, mm3;

    bool stopping = 0;

    for (int node = 0; node < totalpossnodes - 1; node++)
    {
        if (bordernodes[node + 1][0] == -1) // If we've run out of nodes to do, stop after this one
        {
            mm1.x = (float) bordernodes[node][0];
            mm1.y = (float) bordernodes[node][1];

            mm3.x = (float) bordernodes[0][0];
            mm3.y = (float) bordernodes[0][1];

            stopping = 1;
        }
        else
        {
            mm1.x = (float) bordernodes[node][0];
            mm1.y = (float) bordernodes[node][1];

            mm3.x = (float) bordernodes[node + 1][0];
            mm3.y = (float) bordernodes[node + 1][1];
        }

        int amount = random(1, midvar);

        int diffx = abs(bordernodes[node][0] - bordernodes[node + 1][0]);
        int diffy = abs(bordernodes[node][1] - bordernodes[node + 1][1]);

        if (diffx + diffy < 6) // If the nodes are so close together, don't wobble at all.
            amount = 0;

        mm2.x = (float) (mm1.x + mm3.x) / 2 + randomsign(amount);
        mm2.y = (float) (mm1.y + mm3.y) / 2 + randomsign(amount);

        for (float t = 0.0f; t <= 1.0f; t = t + 0.01f)
        {
            pt = curvepos(mm1, mm1, mm2, mm3, t);

            int x = (int)pt.x;
            int y = (int)pt.y;

            if (x >= 0 && x < ARRAYWIDTH * 2 && y >= 0 && y <= ARRAYHEIGHT * 2)
                riftlakemap[x][y] = 1;
        }

        for (float t = 0.0f; t <= 1.0f; t = t + 0.01f)
        {
            pt = curvepos(mm1, mm2, mm3, mm3, t);

            int x = (int)pt.x;
            int y = (int)pt.y;

            if (x >= 0 && x < ARRAYWIDTH * 2 && y >= 0 && y <= ARRAYHEIGHT * 2)
                riftlakemap[x][y] = 1;
        }

        if (stopping == 1)
            node = totalpossnodes - 1;
    }

    // Now we fill in the middle. To be on the safe side we will do a flood fill from the middle of each tile (other than the first and last), just in case the lake disappears off the edge of the map and then curves back onto it.

    x = sx;
    y = sy;

    thisnode = 1; // The number of the first node on the outgoing side of this tile.
    laternode = totalnodes - 2; // The number of the first node on the incoming side of this tile.

    keepgoing = 1;

    int col = 1;

    do
    {
        ddx = dx - (sx - x) * 16;
        ddy = dy - (sy - y) * 16;

        dir = world.riverdir(x, y);

        bool goahead = 1;

        if (x == sx && y == sy)
            goahead = 0;

        if (x == endx && y == endy)
            goahead = 0;

        if (goahead == 1)
        {
            int fillstartx = ddx + 8;
            int fillstarty = ddy + 8;

            fill(riftlakemap, RARRAYWIDTH * 2, RARRAYHEIGHT * 2, fillstartx, fillstarty, col);
        }

        if (x == endx && y == endy)
            keepgoing = 0;
        else
        {
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

            thisnode++;
            laternode--;
        }
    } while (keepgoing == 1);
}

// This function puts the rift lakes on the regional map tiles.

void makeriftlaketile(planet& world, region& region, int dx, int dy, int sx, int sy, int extra, vector<vector<bool>>& riftlakemap, vector<vector<float>>& riftblob, int riftblobsize)
{
    int surfacelevel = world.riftlakesurface(sx, sy);

    if (surfacelevel == 0)
        return;

    int width = world.width();
    int rwidth = region.rwidth();
    int rheight = region.rheight();
    int minpoint = 3; // Minimum distance the points on the sides (where rivers go off the tile) can be from the corners.
    int dextra = extra * 16;

    twointegers upstream = getupstreamcell(world, sx, sy);

    int upstreamdir = getdir(sx, sy, upstream.x, upstream.y);
    int downstreamdir = world.riverdir(sx, sy);

    // Get coordinates for the tiles around this one.

    int eax = sx + 1;
    int eay = sy;
    if (eax > width)
        eax = 0;

    int sox = sx;
    int soy = sy + 1;

    int nseed = (sy * width + sx) + world.nom(sx, sy) + world.julrain(sx, sy) + world.riverjan(sx, sy); // Add the flow here so it isn't exactly the same as the western edge seed (which would make for weirdly symmetrical river courses).
    int sseed = (soy * width + sox) + world.nom(sox, soy) + world.julrain(sox, soy) + world.riverjan(sox, soy);
    int wseed = (sy * width + sx) + world.nom(sx, sy) + world.julrain(sx, sy);
    int eseed = (eay * width + eax) + world.nom(eax, eay) + world.julrain(eax, eay);

    fast_srand(nseed);

    int northpointx = random(dx + minpoint, dx + 16 - minpoint);
    int northpointy = dy;

    fast_srand(sseed);

    int southpointx = random(dx + minpoint, dx + 16 - minpoint);
    int southpointy = dy + 16;

    fast_srand(wseed);

    int westpointx = dx;
    int westpointy = random(dy + minpoint, dy + 16 - minpoint);

    fast_srand(eseed);

    int eastpointx = dx + 16;
    int eastpointy = random(dy + minpoint, dy + 16 - minpoint);


    int northeastpointx = dx + 16;
    int northeastpointy = dy;

    int northwestpointx = dx;
    int northwestpointy = dy;

    int southeastpointx = dx + 16;
    int southeastpointy = dy + 16;

    int southwestpointx = dx;
    int southwestpointy = dy + 16;

    int inx = -1;
    int iny = -1;
    int outx = -1;
    int outy = -1;

    switch (upstreamdir)
    {
    case 1:
        inx = northpointx;
        iny = northpointy;
        break;

    case 2:
        inx = northeastpointx;
        iny = northeastpointy;
        break;

    case 3:
        inx = eastpointx;
        iny = eastpointy;
        break;

    case 4:
        inx = southeastpointx;
        iny = southeastpointy;
        break;

    case 5:
        inx = southpointx;
        iny = southpointy;
        break;

    case 6:
        inx = southwestpointx;
        iny = southwestpointy;
        break;

    case 7:
        inx = westpointx;
        iny = westpointy;
        break;

    case 8:
        inx = northwestpointx;
        iny = northwestpointy;
        break;
    }

    switch (downstreamdir)
    {
    case 1:
        outx = northpointx;
        outy = northpointy;
        break;

    case 2:
        outx = northeastpointx;
        outy = northeastpointy;
        break;

    case 3:
        outx = eastpointx;
        outy = eastpointy;
        break;

    case 4:
        outx = southeastpointx;
        outy = southeastpointy;
        break;

    case 5:
        outx = southpointx;
        outy = southpointy;
        break;

    case 6:
        outx = southwestpointx;
        outy = southwestpointy;
        break;

    case 7:
        outx = westpointx;
        outy = westpointy;
        break;

    case 8:
        outx = northwestpointx;
        outy = northwestpointy;
        break;
    }

    // Now we know the points where the main river enters and leaves the tile. Next we need to find out how long it is.

    bool keepgoing = 1;
    float length = 0;

    int x = inx;
    int y = iny;

    do
    {
        length++;

        int dir = region.riverdir(x, y);

        if (dir == 0)
            keepgoing = 0;

        if (dir == 8 || dir == 1 || dir == 2)
            y--;

        if (dir == 4 || dir == 5 || dir == 6)
            y++;

        if (dir == 2 || dir == 3 || dir == 4)
            x++;

        if (dir == 6 || dir == 7 || dir == 8)
            x--;

        if (x == outx && y == outy)
            keepgoing = 0;

        if (length > 20)
            keepgoing = 0;

    } while (keepgoing == 1);

    // Now we know how long the river is as well as its entry and exit points.

    twointegers dest = getflowdestination(world, sx, sy, 0);

    float startdepth = (float)(surfacelevel - world.riftlakebed(sx, sy));
    float enddepth = (float)(surfacelevel - world.riftlakebed(dest.x, dest.y));

    if (enddepth == (float)surfacelevel)
        enddepth = startdepth;

    // We want to change from startdepth to enddepth gradually over the course of the tile.

    int radius = riftblobsize / 2;
    float diff = enddepth - startdepth;

    float step = diff / length; // Add this to the depth each step.

    x = inx;
    y = iny;

    keepgoing = 1;
    float currentdepth = startdepth;
    int crount = 0;

    do
    {
        if (x == outx && y == outy)
            keepgoing = 0;

        if (crount > length)
            keepgoing = 0;

        for (int i = -radius; i <= radius; i++)
        {
            int ii = x + i;

            for (int j = -radius; j <= radius; j++)
            {
                int jj = y + j;

                float amount = currentdepth * riftblob[i + radius][j + radius];

                if (amount > 0 && ii >= 0 && ii <= rwidth && jj >= 0 && jj <= rheight && (region.map(ii, jj) > surfacelevel - amount || region.map(ii, jj) == 0))
                {
                    if (riftlakemap[ii + dextra][jj + dextra] == 1)
                    {
                        region.setlakesurface(ii, jj, surfacelevel);
                        region.setmap(ii, jj, surfacelevel - (int)amount); //+randomsign(random(0,var)));
                    }

                }
            }
        }

        int dir = region.riverdir(x, y);

        if (dir == 8 || dir == 1 || dir == 2)
            y--;

        if (dir == 4 || dir == 5 || dir == 6)
            y++;

        if (dir == 2 || dir == 3 || dir == 4)
            x++;

        if (dir == 6 || dir == 7 || dir == 8)
            x--;

        currentdepth = currentdepth + step;
        crount++;

    } while (keepgoing == 1);

    // Now maybe put an island on it.

    int islandchance = 6; // The lower this, the more islands there will be.

    int dir = world.riverdir(sx, sy);

    if (dir != 1 && dir != 3 && dir != 5 && dir != 7)
        return;

    if (random(1, islandchance) != 1)
        return;

    x = -1;
    y = -1;

    if (dir == 1 || dir == 5) // North/south
    {
        x = random(dx + 3, dx + 13);

        if (random(1, 2) == 1)
        {
            for (int yy = dy - 16; yy <= dy + 16; yy++)
            {
                if (region.lakesurface(x, yy) == surfacelevel)
                {
                    y = yy + random(1, 2);
                    yy = dy + 16;
                }
            }
        }
        else
        {
            for (int yy = dy + 16; yy >= dy - 16; yy--)
            {
                if (region.lakesurface(x, yy) == surfacelevel)
                {
                    y = yy - random(1, 2);
                    yy = dy - 16;
                }
            }
        }
    }
    else // East/west
    {
        y = random(dy + 3, dy + 13);

        if (random(1, 2) == 1)
        {
            for (int xx = dx - 16; xx <= dx + 16; xx++)
            {
                if (region.lakesurface(xx, y) == surfacelevel)
                {
                    x = xx + random(1, 2);
                    xx = dx + 16;
                }
            }
        }
        else
        {
            for (int xx = dx + 16; xx >= dx - 16; xx--)
            {
                if (region.lakesurface(xx, y) == surfacelevel)
                {
                    x = xx - random(1, 2);
                    xx = dx - 16;
                }
            }
        }
    }

    if (x == -1 || y == -1)
        return;

    int islandlength = random(2, 4);
    int swervechance = 4; // The higher this is, the straighter islands will tend to be

    for (int n = 1; n <= islandlength; n++)
    {
        if (region.lakesurface(x, y) == 0)
            return;

        int depth = surfacelevel - region.map(x, y);

        region.setlakesurface(x, y, 0);
        region.setmap(x, y, surfacelevel + depth / 2);

        if (dir == 1 || dir == 5)
        {
            if (random(1, swervechance) == 1)
            {
                if (random(1, 2) == 1)
                    x--;
                else
                    x++;
            }
            else
                y++;
        }
        else
        {
            if (random(1, swervechance) == 1)
            {
                if (random(1, 2) == 1)
                    y--;
                else
                    y++;
            }
            else
                x++;
        }
    }
}

// This function puts the large lakes on the regional map tiles.

void makelaketile(planet& world, region& region, int dx, int dy, int sx, int sy, vector<vector<bool>>& lakemap, int surfacelevel, int coords[4][2], vector<vector<int>>& source, vector<vector<int>>& dest, vector<vector<bool>>& safesaltlakes, boolshapetemplate smalllake[])
{
    int width = world.width();
    int height = world.height();

    int rwidth = region.rwidth();
    int rheight = region.rheight();

    // Get coordinates for the tiles around this one.

    int eax = sx + 1;
    int eay = sy;
    if (eax > width)
        eax = 0;

    int sox = sx;
    int soy = sy + 1;

    int sex = eax;
    int sey = soy;

    int nwelev = world.nom(sx, sy);
    int neelev = world.nom(eax, eay);
    int swelev = world.nom(sox, soy);
    int seelev = world.nom(sex, sey);

    if (nwelev >= surfacelevel)
        nwelev = surfacelevel - 1;

    if (neelev >= surfacelevel)
        neelev = surfacelevel - 1;

    if (swelev >= surfacelevel)
        swelev = surfacelevel - 1;

    if (seelev >= surfacelevel)
        seelev = surfacelevel - 1;

    fast_srand((sy * width + sx) + world.nom(sx, sy) + world.julrain(sx, sy) + world.riverjan(sx, sy));

    int special = 0;
    int amount = 1;

    // 100: salt lake. 110: salt pan. 120: dunes. 130: fresh wetlands. 131: brackish wetlands. 132: salt wetlands.

    if (world.special(sx, sy) != 0)
        special = world.special(sx, sy);
    else
    {
        for (int i = sx - amount; i <= sx + amount; i++)
        {
            int ii = i;

            if (ii<0 || ii>width)
                ii = wrap(ii, width);

            for (int j = sy - amount; j <= sy + amount; j++)
            {
                if (j >= 0 && j <= height)
                {
                    if (world.special(ii, j) >= 100 && world.special(ii, j) < 130)
                    {
                        special = world.special(ii, j);

                        i = sx + amount;
                        j = sy + amount;
                    }
                }
            }
        }
    }

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (i >= 0 && i <= rwidth && j >= 0 && j <= rheight && lakemap[i][j] == 1)
            {
                region.setlakesurface(i, j, surfacelevel);
                region.setspecial(i, j, special);

                if (special > 100)
                    region.setmap(i, j, surfacelevel);

                // We work out how close our point is to the different corners of the tile, and calculate an approximate elevation for it on that basis.

                float nwdist = (float)(abs(i - dx) + abs(j - dy));
                float nedist = (float)(abs(i - (dx + 16)) + abs(j - dy));
                float swdist = (float)(abs(i - dx) + abs(j - (dy + 16)));
                float sedist = (float)(abs(i - (dx + 16)) + abs(j - (dy + 16)));

                float total = nwdist + nedist + swdist + sedist;

                float elev = ((float)nwelev * sedist + (float)neelev * swdist + (float)swelev * nedist + (float)seelev * nwdist) / total;

                if (elev >= (float)surfacelevel)
                    elev = (float)(surfacelevel - 1);

                region.setmap(i, j, (int)elev);
            }
        }
    }

    if (special == 110) // If this is a salt pan, there's a chance of putting a small salt lake in the middle of it.
    {
        int smallsaltlakechance = 3;

        if (random(1, smallsaltlakechance) == 1)
        {
            int centrex = dx + random(3, 13);
            int centrey = dy + random(3, 13);

            createsmallsaltlake(world, region, dx, dy, sx, sy, centrex, centrey, surfacelevel, safesaltlakes, smalllake);
        }
    }
}

// This function puts islands in the large lakes.

void makelakeislands(planet& world, region& region, int dx, int dy, int sx, int sy, int surfacelevel, boolshapetemplate island[], vector<vector<bool>>& lakeislands)
{
    fast_srand((sy * world.width() + sx) + world.nom(sx, sy) + world.julrain(sx, sy) + world.riverjul(sx, sy));

    int width = world.width();
    int height = world.height();

    int islandchance = 6; // The higher this is, the fewer islands there will be.
    int maxislands = 6; // The maximum number of islands on any given tile.

    if (world.lakesurface(sx, sy) != 0) // The further from the edge of the lake we are, the fewer islands.
    {
        islandchance = islandchance * 2;

        if (getlakeedge(world, sx, sy) == 0)
            islandchance = islandchance * 2;
    }

    if (random(1, islandchance) != 1)
        return;

    // Check that this is the sort of lake that can have islands.
    // 100: salt lake. 110: salt pan. 120: dunes. 130: fresh wetlands. 131: brackish wetlands. 132: salt wetlands.

    int special = 0;
    bool found = 0;
    int amount = 1;

    if (world.special(sx, sy) >= 100)
    {
        special = world.special(sx, sy);
        found = 1;
    }

    if (found == 0)
    {
        for (int i = sx - amount; i <= sx + amount; i++)
        {
            int ii = i;

            if (ii<0 || ii>width)
                ii = wrap(ii, width);

            for (int j = sy - amount; j <= sy + amount; j++)
            {
                if (j >= 0 && j <= height)
                {
                    if (world.special(ii, j) >= 100)
                    {
                        special = world.special(ii, j);

                        i = sx + amount;
                        j = sy + amount;
                    }
                }
            }
        }
    }

    if (special == 120) // Ergs can't have islands.
        return;

    bool nooverlap = 1;

    if (special == 110) // Salt pans can have overlapping islands.
        nooverlap = 0;

    int totalislands = random(1, maxislands);

    for (int islandnumber = 1; islandnumber <= totalislands; islandnumber++)
    {
        int centrex = -1;
        int centrey = -1;

        bool found = 0;

        for (int n = 1; n <= 20; n++) // Try to find a starting point that isn't currently land.
        {
            centrex = random(dx, dx + 16);
            centrey = random(dy, dy + 16);

            if (region.lakesurface(centrex, centrey) != 0)
            {
                n = 20;
                found = 1;
            }
        }

        if (found == 0)
            return;

        // Now put an island there.

        createlakeisland(world, region, centrex, centrey, surfacelevel, island, lakeislands, nooverlap, special);
    }
}

// This function creates an island in a large lake.

void createlakeisland(planet& world, region& region, int centrex, int centrey, int surfacelevel, boolshapetemplate island[], vector<vector<bool>>& lakeislands, bool nooverlap, int special)
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    int shapenumber = random(0, 11);

    int imheight = island[shapenumber].ysize() - 1;
    int imwidth = island[shapenumber].xsize() - 1;

    int x = centrex - imwidth / 2; // Coordinates of the top left corner.
    int y = centrey - imheight / 2;

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
        jstart = imheight;
        destj = -1;
        jstep = -1;
    }

    int imap = -1;
    int jmap = -1;

    // First, check that this island will not border any land.

    if (nooverlap == 1)
    {
        bool byland = 0;

        for (int i = istart; i != desti; i = i + istep)
        {
            imap++;
            jmap = -1;

            for (int j = jstart; j != destj; j = j + jstep)
            {
                jmap++;

                if (island[shapenumber].point(i, j) == 1)
                {
                    int xx = x + imap;
                    int yy = y + jmap;

                    for (int k = xx - 1; k <= xx + 1; k++)
                    {
                        for (int l = yy - 1; l <= yy + 1; l++)
                        {
                            if (k >= 0 && k <= rwidth && l >= 0 && l <= rheight)
                            {
                                if (region.lakesurface(k, l) == 0)
                                {
                                    byland = 1;
                                    k = xx + 1;
                                    l = yy + 1;
                                }
                            }
                        }
                    }
                }
            }
        }

        if (byland == 1)
            return;
    }

    // Now actually put down the island.

    int elevfactor = random(2, 4); // Divide the existing depth by this to get the elevation of the island at this point.

    imap = -1;
    jmap = -1;

    for (int i = istart; i != desti; i = i + istep)
    {
        imap++;
        jmap = -1;

        for (int j = jstart; j != destj; j = j + jstep)
        {
            jmap++;

            if (island[shapenumber].point(i, j) == 1)
            {
                int xx = x + imap;
                int yy = y + jmap;

                if (xx >= 0 && xx <= rwidth && yy >= 0 && yy <= rheight)
                {
                    lakeislands[xx][yy] = 1;
                    region.setlakesurface(xx, yy, 0);
                    region.setspecial(xx, yy, 0);

                    if (special == 110) // Salt pan
                        region.setmap(xx, yy, surfacelevel);
                    else
                    {
                        int depth = surfacelevel - region.map(xx, yy);

                        depth = depth / elevfactor;

                        if (depth < 1)
                            depth = 1;

                        region.setmap(xx, yy, surfacelevel + depth);
                    }
                }
            }
        }
    }
}

// This function makes the coastlines in the given tile more interesting. (Used for single-tile islands.)

void complicatecoastlines(planet& world, region& region, int dx, int dy, int sx, int sy, boolshapetemplate smalllake[], int chance)
{
    int sealevel = world.sealevel();
    //int chance=8; // Probability of this happening on any given cell.

    int avedepth = 0;
    int crount = 0;

    if (region.map(dx, dy) <= sealevel)
    {
        avedepth = avedepth + region.map(dx, dy);
        crount++;
    }

    if (region.map(dx + 16, dy) <= sealevel)
    {
        avedepth = avedepth + region.map(dx + 16, dy);
        crount++;
    }

    if (region.map(dx, dy + 16) <= sealevel)
    {
        avedepth = avedepth + region.map(dx, dy + 16);
        crount++;
    }

    if (region.map(dx + 16, dy + 16) <= sealevel)
    {
        avedepth = avedepth + region.map(dx + 16, dy + 16);
        crount++;
    }

    if (avedepth == 0)
        return;

    avedepth = avedepth / crount;

    fast_srand((sy * world.width() + sx) + world.nom(sx, sy) + world.janrain(sx, sy));

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (random(1, chance) == 1)
            {
                if (region.sea(i, j) == 0 && vaguelycoastal(region, i, j) == 1)
                    disruptseacoastline(world, region, dx, dy, i, j, avedepth, 0, 11, 0, smalllake);
            }
        }
    }
}

// This function adds extra precipitation to mountains, to match what's assigned to them in the global map.

void addregionalmountainprecipitation(planet& world, region& region, int dx, int dy, int sx, int sy, bool summer)
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    float extraprep = 0; // This is the amount of extra precipitation to add.
    int dir = 0; // Direction that the wind is blowing.
    float slopefactor = 400; // If the slope is this steep, total extra precipitation will be added.

    int lowestelev = world.maxelevation(); // Lowest elevation in this tile.
    int highestelev = 0; // Highest elevation in this tile.

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (region.map(i, j) < lowestelev)
                lowestelev = region.map(i, j);

            if (region.map(i, j) > highestelev)
                highestelev = region.map(i, j);
        }
    }

    float elevdiff = (float)(highestelev - lowestelev);

    if (summer == 0)
    {
        extraprep = (float)(world.winterrain(sx, sy) - world.wintermountainrain(sx, sy));
        dir = world.wintermountainraindir(sx, sy);
    }
    else
    {
        extraprep = (float)(world.summerrain(sx, sy) - world.summermountainrain(sx, sy));
        dir = world.summermountainraindir(sx, sy);
    }

    if (extraprep <= 0.0f)
        return;

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            int k = i;
            int l = j; // Coordinates of the cell where the wind is coming from.

            if (dir == 8 || dir == 1 || dir == 2)
                l--;

            if (dir == 4 || dir == 5 || dir == 6)
                l++;

            if (dir == 2 || dir == 3 || dir == 4)
                k++;

            if (dir == 6 || dir == 7 || dir == 8)
                k--;

            if (k >= 0 && k <= rwidth && l >= 0 && l <= rheight)
            {
                float slope = (float)getslope(region, k, l, i, j);

                if (slope > 0)
                {
                    float thisextraprep = extraprep;

                    if (slope < slopefactor)
                        thisextraprep = thisextraprep * (slope / slopefactor);

                    if (region.map(i, j) < highestelev)
                        thisextraprep = thisextraprep * ((float)(region.map(i, j) - lowestelev) / elevdiff);

                    if (summer == 0)
                        region.setwinterrain(i, j, region.winterrain(i, j) + (int)thisextraprep);
                    else
                        region.setsummerrain(i, j, region.summerrain(i, j) + (int)thisextraprep);
                }
            }
        }
    }
}

// This function removes any remaining straight lines of rivers.

void removeregionalstraightrivers(planet& world, region& region, int dx, int dy, int sx, int sy, vector<vector<bool>>& rivercurves)
{
    fast_srand((sy * world.width() + sx) + world.nom(sx, sy) + world.julrain(sx, sy));

    twofloats mm1, mm2, mm3, pt;

    int maxline = 3; // Any lines longer than this will be dealt with.
    int minadd = 1;
    int maxadd = 3; // Min/max amounts to add to the midpoint of the curve.

    int changechance = 3; // The higher this is, the more diagonal cells will be changed.
    int removechance = 2; // The lower this is, the more cells will be changed in straight diagonals.

    int diagmaxline = 4; // Any diagonal lines longer than this will be dealt with.

    // First, sort out diagonals. We go through and look for rivers that are heading diagonally (including those that are doing it in two steps). We randomly change some of them to take slightly different routes.

    // North then east.

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (random(1, changechance) != 1)
            {
                if (rivercurves[i][j] == 0 && region.riverdir(i, j) == 1 && region.riverdir(i, j - 1) == 3)
                {
                    rivercurves[i][j] = 1;

                    int riverjan = region.riverjan(i, j);
                    int riverjul = region.riverjul(i, j);

                    if (region.riverjan(i, j - 1 == riverjan && region.riverjul(i, j - 1) == riverjul))
                    {
                        region.setriverjan(i, j - 1, 0);
                        region.setriverjul(i, j - 1, 0);
                        region.setriverdir(i, j - 1, 0);

                        if (random(1, 2) == 1 && region.riverdir(i - 1, j + 1) != 2)
                            region.setriverdir(i, j, 2);
                        else
                        {
                            if (region.riverdir(i + 1, j) == 0 && region.riverdir(i, j + 1) != 1)
                            {
                                region.setriverdir(i, j, 3);

                                region.setriverjan(i + 1, j, riverjan);
                                region.setriverjul(i + 1, j, riverjul);
                                region.setriverdir(i + 1, j, 1);
                                rivercurves[i + 1][j] = 1;
                            }
                            else
                                region.setriverdir(i, j, 2);
                        }
                    }
                }
            }
        }
    }

    // Northeast.

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (random(1, changechance) != 1)
            {
                if (rivercurves[i][j] == 0 && region.riverdir(i, j) == 2)
                {
                    rivercurves[i][j] = 1;

                    int riverjan = region.riverjan(i, j);
                    int riverjul = region.riverjul(i, j);

                    if (region.riverjan(i + 1, j + 1 == riverjan && region.riverjul(i + 1, j + 1) == riverjul))
                    {
                        if (random(1, 2) == 1)
                        {
                            if (region.riverdir(i + 1, j) == 0 && region.riverdir(i, j + 1) != 1)
                            {
                                region.setriverdir(i, j, 3);

                                region.setriverjan(i + 1, j, riverjan);
                                region.setriverjul(i + 1, j, riverjul);
                                region.setriverdir(i + 1, j, 1);
                                rivercurves[i + 1][j] = 1;
                            }
                        }
                        else
                        {
                            if (region.riverdir(i, j - 1) == 0 && region.riverdir(i - 1, j) != 3)
                            {
                                region.setriverdir(i, j, 1);

                                region.setriverjan(i, j - 1, riverjan);
                                region.setriverjul(i, j - 1, riverjul);
                                region.setriverdir(i, j - 1, 3);
                                rivercurves[i][j - 1] = 1;
                            }

                        }
                    }
                }
            }
        }
    }

    // East then north.

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (random(1, changechance) != 1)
            {
                if (rivercurves[i][j] == 0 && region.riverdir(i, j) == 3 && region.riverdir(i + 1, j) == 1)
                {
                    rivercurves[i][j] = 1;

                    int riverjan = region.riverjan(i, j);
                    int riverjul = region.riverjul(i, j);

                    if (region.riverjan(i + 1, j == riverjan && region.riverjul(i + 1, j) == riverjul))
                    {
                        region.setriverjan(i + 1, j, 0);
                        region.setriverjul(i + 1, j, 0);
                        region.setriverdir(i + 1, j, 0);

                        if (random(1, 2) == 1 && region.riverdir(i - 1, j + 1) != 2)
                            region.setriverdir(i, j, 2);
                        else
                        {
                            if (region.riverdir(i, j - 1) == 0 && region.riverdir(i - 1, j) != 3)
                            {
                                region.setriverdir(i, j, 1);

                                region.setriverjan(i, j - 1, riverjan);
                                region.setriverjul(i, j - 1, riverjul);
                                region.setriverdir(i, j - 1, 3);
                                rivercurves[i][j - 1] = 1;
                            }
                            else
                                region.setriverdir(i, j, 2);
                        }
                    }
                }
            }
        }
    }

    // East then south.

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (random(1, changechance) != 1)
            {
                if (rivercurves[i][j] == 0 && region.riverdir(i, j) == 3 && region.riverdir(i + 1, j) == 5)
                {
                    rivercurves[i][j] = 1;

                    int riverjan = region.riverjan(i, j);
                    int riverjul = region.riverjul(i, j);

                    if (region.riverjan(i + 1, j == riverjan && region.riverjul(i + 1, j) == riverjul))
                    {
                        region.setriverjan(i + 1, j, 0);
                        region.setriverjul(i + 1, j, 0);
                        region.setriverdir(i + 1, j, 0);

                        if (random(1, 2) == 1 && region.riverdir(i - 1, j - 1) != 4)
                            region.setriverdir(i, j, 4);
                        else
                        {
                            if (region.riverdir(i, j + 1) == 0 && region.riverdir(i - 1, j) != 3)
                            {
                                region.setriverdir(i, j, 5);

                                region.setriverjan(i, j + 1, riverjan);
                                region.setriverjul(i, j + 1, riverjul);
                                region.setriverdir(i, j + 1, 3);
                                rivercurves[i][j + 1] = 1;
                            }
                            else
                                region.setriverdir(i, j, 4);
                        }
                    }
                }
            }
        }
    }

    // Southeast.

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (random(1, changechance) != 1)
            {
                if (rivercurves[i][j] == 0 && region.riverdir(i, j) == 4)
                {
                    rivercurves[i][j] = 1;

                    int riverjan = region.riverjan(i, j);
                    int riverjul = region.riverjul(i, j);

                    if (region.riverjan(i + 1, j - 1 == riverjan && region.riverjul(i + 1, j - 1) == riverjul))
                    {
                        if (random(1, 2) == 1)
                        {
                            if (region.riverdir(i + 1, j) == 0 && region.riverdir(i, j - 1) != 5)
                            {
                                region.setriverdir(i, j, 3);

                                region.setriverjan(i + 1, j, riverjan);
                                region.setriverjul(i + 1, j, riverjul);
                                region.setriverdir(i + 1, j, 5);
                                rivercurves[i + 1][j] = 1;
                            }
                        }
                        else
                        {
                            if (region.riverdir(i, j + 1) == 0 && region.riverdir(i - 1, j) != 3)
                            {
                                region.setriverdir(i, j, 5);

                                region.setriverjan(i, j + 1, riverjan);
                                region.setriverjul(i, j + 1, riverjul);
                                region.setriverdir(i, j + 1, 3);
                                rivercurves[i][j + 1] = 1;
                            }
                        }
                    }
                }
            }
        }
    }

    // South then east.

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (random(1, changechance) != 1)
            {
                if (rivercurves[i][j] == 0 && region.riverdir(i, j) == 5 && region.riverdir(i, j + 1) == 3)
                {
                    rivercurves[i][j] = 1;

                    int riverjan = region.riverjan(i, j);
                    int riverjul = region.riverjul(i, j);

                    if (region.riverjan(i, j + 1 == riverjan && region.riverjul(i, j + 1) == riverjul))
                    {
                        region.setriverjan(i, j + 1, 0);
                        region.setriverjul(i, j + 1, 0);
                        region.setriverdir(i, j + 1, 0);

                        if (random(1, 2) == 1 && region.riverdir(i - 1, j - 1) != 4)
                            region.setriverdir(i, j, 4);
                        else
                        {
                            if (region.riverdir(i + 1, j) == 0 && region.riverdir(i, j - 1) != 5)
                            {
                                region.setriverdir(i, j, 3);

                                region.setriverjan(i + 1, j, riverjan);
                                region.setriverjul(i + 1, j, riverjul);
                                region.setriverdir(i + 1, j, 5);
                                rivercurves[i + 1][j] = 1;
                            }
                            else
                                region.setriverdir(i, j, 4);
                        }
                    }
                }
            }
        }
    }

    // South then west.

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (random(1, changechance) != 1)
            {
                if (rivercurves[i][j] == 0 && region.riverdir(i, j) == 5 && region.riverdir(i, j + 1) == 7)
                {
                    rivercurves[i][j] = 1;

                    int riverjan = region.riverjan(i, j);
                    int riverjul = region.riverjul(i, j);

                    if (region.riverjan(i, j + 1 == riverjan && region.riverjul(i, j + 1) == riverjul))
                    {
                        region.setriverjan(i, j + 1, 0);
                        region.setriverjul(i, j + 1, 0);
                        region.setriverdir(i, j + 1, 0);

                        if (random(1, 2) == 1 && region.riverdir(i - 1, j - 1) != 6)
                            region.setriverdir(i, j, 6);
                        else
                        {
                            if (region.riverdir(i - 1, j) == 0 && region.riverdir(i, j - 1) != 5)
                            {
                                region.setriverdir(i, j, 7);

                                region.setriverjan(i - 1, j, riverjan);
                                region.setriverjul(i - 1, j, riverjul);
                                region.setriverdir(i - 1, j, 5);
                                rivercurves[i - 1][j] = 1;
                            }
                            else
                                region.setriverdir(i, j, 6);
                        }
                    }
                }
            }
        }
    }

    // Southwest.

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (random(1, changechance) != 1)
            {
                if (rivercurves[i][j] == 0 && region.riverdir(i, j) == 6)
                {
                    rivercurves[i][j] = 1;

                    int riverjan = region.riverjan(i, j);
                    int riverjul = region.riverjul(i, j);

                    if (region.riverjan(i - 1, j - 1 == riverjan && region.riverjul(i - 1, j - 1) == riverjul))
                    {
                        if (random(1, 2) == 1)
                        {
                            if (region.riverdir(i - 1, j) == 0 && region.riverdir(i, j - 1) != 5)
                            {
                                region.setriverdir(i, j, 7);

                                region.setriverjan(i - 1, j, riverjan);
                                region.setriverjul(i - 1, j, riverjul);
                                region.setriverdir(i - 1, j, 5);
                                rivercurves[i - 1][j] = 1;
                            }
                        }
                        else
                        {
                            if (region.riverdir(i, j + 1) == 0 && region.riverdir(i + 1, j) != 7)
                            {
                                region.setriverdir(i, j, 5);

                                region.setriverjan(i, j + 1, riverjan);
                                region.setriverjul(i, j + 1, riverjul);
                                region.setriverdir(i, j + 1, 7);
                                rivercurves[i][j + 1] = 1;
                            }
                        }
                    }
                }
            }
        }
    }

    // West then south.

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (random(1, changechance) != 1)
            {
                if (rivercurves[i][j] == 0 && region.riverdir(i, j) == 7 && region.riverdir(i - 1, j) == 5)
                {
                    rivercurves[i][j] = 1;

                    int riverjan = region.riverjan(i, j);
                    int riverjul = region.riverjul(i, j);

                    if (region.riverjan(i - 1, j == riverjan && region.riverjul(i - 1, j) == riverjul))
                    {
                        region.setriverjan(i - 1, j, 0);
                        region.setriverjul(i - 1, j, 0);
                        region.setriverdir(i - 1, j, 0);

                        if (random(1, 2) == 1 && region.riverdir(i + 1, j - 1) != 6)
                            region.setriverdir(i, j, 6);
                        else
                        {
                            if (region.riverdir(i, j + 1) == 0 && region.riverdir(i + 1, j) != 7)
                            {
                                region.setriverdir(i, j, 5);

                                region.setriverjan(i, j + 1, riverjan);
                                region.setriverjul(i, j + 1, riverjul);
                                region.setriverdir(i, j + 1, 7);
                                rivercurves[i][j + 1] = 1;
                            }
                            else
                                region.setriverdir(i, j, 6);
                        }
                    }
                }
            }
        }
    }

    // West then north.

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (random(1, changechance) != 1)
            {
                if (rivercurves[i][j] == 0 && region.riverdir(i, j) == 7 && region.riverdir(i - 1, j) == 1)
                {
                    rivercurves[i][j] = 1;

                    int riverjan = region.riverjan(i, j);
                    int riverjul = region.riverjul(i, j);

                    if (region.riverjan(i - 1, j == riverjan && region.riverjul(i - 1, j) == riverjul))
                    {
                        region.setriverjan(i - 1, j, 0);
                        region.setriverjul(i - 1, j, 0);
                        region.setriverdir(i - 1, j, 0);

                        if (random(1, 2) == 1 && region.riverdir(i + 1, j - 1) != 8)
                            region.setriverdir(i, j, 8);
                        else
                        {
                            if (region.riverdir(i, j - 1) == 0 && region.riverdir(i + 1, j) != 7)
                            {
                                region.setriverdir(i, j, 1);

                                region.setriverjan(i, j - 1, riverjan);
                                region.setriverjul(i, j - 1, riverjul);
                                region.setriverdir(i, j - 1, 7);
                                rivercurves[i][j - 1] = 1;
                            }
                            else
                                region.setriverdir(i, j, 8);
                        }
                    }
                }
            }
        }
    }

    // Northwest.

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (random(1, changechance) != 1)
            {
                if (rivercurves[i][j] == 0 && region.riverdir(i, j) == 8)
                {
                    rivercurves[i][j] = 1;

                    int riverjan = region.riverjan(i, j);
                    int riverjul = region.riverjul(i, j);

                    if (region.riverjan(i - 1, j + 1 == riverjan && region.riverjul(i - 1, j + 1) == riverjul))
                    {
                        if (random(1, 2) == 1)
                        {
                            if (region.riverdir(i - 1, j) == 0 && region.riverdir(i, j + 1) != 1)
                            {
                                region.setriverdir(i, j, 7);

                                region.setriverjan(i - 1, j, riverjan);
                                region.setriverjul(i - 1, j, riverjul);
                                region.setriverdir(i - 1, j, 1);
                                rivercurves[i - 1][j] = 1;
                            }
                        }
                        else
                        {
                            if (region.riverdir(i, j - 1) == 0 && region.riverdir(i + 1, j) != 7)
                            {
                                region.setriverdir(i, j, 1);

                                region.setriverjan(i, j - 1, riverjan);
                                region.setriverjul(i, j - 1, riverjul);
                                region.setriverdir(i, j - 1, 7);
                                rivercurves[i][j - 1] = 1;
                            }
                        }
                    }
                }
            }
        }
    }

    // North then west.

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (random(1, changechance) != 1)
            {
                if (rivercurves[i][j] == 0 && region.riverdir(i, j) == 1 && region.riverdir(i, j - 1) == 7)
                {
                    rivercurves[i][j] = 1;

                    int riverjan = region.riverjan(i, j);
                    int riverjul = region.riverjul(i, j);

                    if (region.riverjan(i, j - 1 == riverjan && region.riverjul(i, j - 1) == riverjul))
                    {
                        region.setriverjan(i, j - 1, 0);
                        region.setriverjul(i, j - 1, 0);
                        region.setriverdir(i, j - 1, 0);

                        if (random(1, 2) == 1 && region.riverdir(i - 1, j - 1) != 8)
                            region.setriverdir(i, j, 8);
                        else
                        {
                            if (region.riverdir(i - 1, j) == 0 && region.riverdir(i, j + 1) != 1)
                            {
                                region.setriverdir(i, j, 7);

                                region.setriverjan(i - 1, j, riverjan);
                                region.setriverjul(i - 1, j, riverjul);
                                region.setriverdir(i - 1, j, 1);
                                rivercurves[i - 1][j] = 1;
                            }
                            else
                                region.setriverdir(i, j, 8);
                        }
                    }
                }
            }
        }
    }

    // Now, we do any diagonal lines that are still there.

    // First, lines going southeast.

    for (int i = dx; i <= dx + 16 - diagmaxline / 2; i++)
    {
        for (int j = dy; j <= dy + 16 - diagmaxline / 2; j++)
        {
            if (region.riverdir(i, j) == 3 && region.riverdir(i + 1, j - 1) == 0 && region.riverdir(i - 1, j) == 0 && region.riverdir(i - 1, j + 1) == 0 && region.riverdir(i, j + 1 == 0))
            {
                int x = i;
                int y = j;

                bool keepgoing = 1;

                bool step = 1;

                int linelength = 0;

                do
                {
                    linelength++;

                    if (step == 1)
                    {
                        x++;

                        if (x > dx + 16)
                        {
                            x--;
                            keepgoing = 0;
                        }
                        else
                        {
                            if (region.riverdir(x, y) != 5 || region.riverdir(x, y - 1) != 0 || region.riverdir(x + 1, y - 1) != 0 || region.riverdir(x + 1, y) != 0 || region.riverdir(x - 1, y + 1) != 0)
                            {
                                x--;
                                keepgoing = 0;
                            }
                        }

                        step = 0;
                    }
                    else
                    {
                        y++;

                        if (y > dy + 16)
                        {
                            y--;
                            keepgoing = 0;
                        }
                        else
                        {
                            if (region.riverdir(x, y) != 3 || region.riverdir(x + 1, y - 1) != 0 || region.riverdir(x - 1, y) != 0 || region.riverdir(x - 1, y + 1) != 0 || region.riverdir(x, y + 1) != 0)
                            {
                                y--;
                                keepgoing = 0;
                            }
                        }

                        step = 1;
                    }
                } while (keepgoing == 1);

                if (linelength > diagmaxline) // Change some cells so the line looks more varied.
                {
                    int riverjan = region.riverjan(i, j);
                    int riverjul = region.riverjul(i, j);

                    int enddir = region.riverdir(x, y);

                    int xx = i;
                    int yy = j;

                    bool keepgoing = 1;
                    bool didlast = 0;

                    do
                    {
                        bool stymied = 0;

                        int dir = region.riverdir(xx, yy);

                        if (didlast == 0 && random(1, removechance) == 1 && (xx != i || yy != j) && (xx != x || yy != y)) // && countinflows(region,xx,yy)==1)
                        {
                            if (dir == 3)
                            {
                                if (random(1, 2) == 1)
                                {
                                    region.setriverdir(xx, yy, 0);
                                    region.setriverjan(xx, yy, 0);
                                    region.setriverjul(xx, yy, 0);

                                    region.setriverdir(xx, yy - 1, 4);
                                }
                                else
                                {
                                    if (region.riverdir(xx + 1, yy - 1) == 0)
                                    {
                                        region.setriverdir(xx, yy, 0);
                                        region.setriverjan(xx, yy, 0);
                                        region.setriverjul(xx, yy, 0);

                                        region.setriverdir(xx, yy - 1, 3);

                                        region.setriverdir(xx + 1, yy - 1, 5);
                                        region.setriverjan(xx + 1, yy - 1, riverjan);
                                        region.setriverjul(xx + 1, yy - 1, riverjul);
                                    }
                                    else
                                        stymied = 1;
                                }
                            }
                            else
                            {
                                if (random(1, 2) == 1)
                                {
                                    region.setriverdir(xx, yy, 0);
                                    region.setriverjan(xx, yy, 0);
                                    region.setriverjul(xx, yy, 0);

                                    region.setriverdir(xx - 1, yy, 4);
                                }
                                else
                                {
                                    if (region.riverdir(xx - 1, yy + 1) == 0)
                                    {
                                        region.setriverdir(xx, yy, 0);
                                        region.setriverjan(xx, yy, 0);
                                        region.setriverjul(xx, yy, 0);

                                        region.setriverdir(xx - 1, yy, 5);

                                        region.setriverdir(xx - 1, yy + 1, 3);
                                        region.setriverjan(xx - 1, yy + 1, riverjan);
                                        region.setriverjul(xx - 1, yy + 1, riverjul);
                                    }
                                    else
                                        stymied = 1;
                                }
                            }

                            if (stymied == 0)
                                didlast = 1;
                        }
                        else
                            didlast = 0;

                        if (dir == 3)
                            xx++;
                        else
                            yy++;

                        if (xx == x && yy == y)
                            keepgoing = 0;

                    } while (keepgoing == 1);
                }
            }
        }
    }

    // Next, lines going northwest.

    for (int i = dx + 16; i >= dx + diagmaxline / 2; i--)
    {
        for (int j = dy + 16; j >= dy + diagmaxline / 2; j--)
        {
            if (region.riverdir(i, j) == 7 && region.riverdir(i - 1, j + 1) == 0 && region.riverdir(i + 1, j) == 0 && region.riverdir(i + 1, j - 1) == 0 && region.riverdir(i, j - 1) == 0)
            {
                int x = i;
                int y = j;

                bool keepgoing = 1;

                bool step = 1;

                int linelength = 0;

                do
                {
                    linelength++;

                    if (step == 1)
                    {
                        x--;

                        if (x < dx)
                        {
                            x++;
                            keepgoing = 0;
                        }
                        else
                        {
                            if (region.riverdir(x, y) != 1 || region.riverdir(x, y + 1) != 0 || region.riverdir(x - 1, y + 1) != 0 || region.riverdir(x - 1, y) != 0 || region.riverdir(x + 1, y - 1) != 0)
                            {
                                x++;
                                keepgoing = 0;
                            }
                        }

                        step = 0;
                    }
                    else
                    {
                        y--;

                        if (y < dy)
                        {
                            y++;
                            keepgoing = 0;
                        }
                        else
                        {
                            if (region.riverdir(x, y) != 7 || region.riverdir(x - 1, y + 1) != 0 || region.riverdir(x + 1, y) != 0 || region.riverdir(x + 1, y - 1) != 0 || region.riverdir(x, y - 1) != 0)
                            {
                                y++;
                                keepgoing = 0;
                            }
                        }

                        step = 1;
                    }
                } while (keepgoing == 1);

                if (linelength > diagmaxline) // Change some cells so the line looks more varied.
                {
                    int riverjan = region.riverjan(i, j);
                    int riverjul = region.riverjul(i, j);

                    int enddir = region.riverdir(x, y);

                    int xx = i;
                    int yy = j;

                    bool keepgoing = 1;
                    bool didlast = 0;

                    do
                    {
                        bool stymied = 0;

                        int dir = region.riverdir(xx, yy);

                        if (didlast == 0 && random(1, removechance) == 1 && (xx != i || yy != j) && (xx != x || yy != y)) // && countinflows(region,xx,yy)==1)
                        {
                            if (dir == 7)
                            {
                                if (random(1, 2) == 1)
                                {
                                    region.setriverdir(xx, yy, 0);
                                    region.setriverjan(xx, yy, 0);
                                    region.setriverjul(xx, yy, 0);

                                    region.setriverdir(xx, yy + 1, 8);
                                }
                                else
                                {
                                    if (region.riverdir(xx - 1, yy + 1) == 0)
                                    {
                                        region.setriverdir(xx, yy, 0);
                                        region.setriverjan(xx, yy, 0);
                                        region.setriverjul(xx, yy, 0);

                                        region.setriverdir(xx, yy + 1, 7);

                                        region.setriverdir(xx - 1, yy + 1, 1);
                                        region.setriverjan(xx - 1, yy + 1, riverjan);
                                        region.setriverjul(xx - 1, yy + 1, riverjul);
                                    }
                                    else
                                        stymied = 1;
                                }
                            }
                            else
                            {
                                if (random(1, 2) == 1)
                                {
                                    region.setriverdir(xx, yy, 0);
                                    region.setriverjan(xx, yy, 0);
                                    region.setriverjul(xx, yy, 0);

                                    region.setriverdir(xx + 1, yy, 8);
                                }
                                else
                                {
                                    if (region.riverdir(xx + 1, yy - 1) == 0)
                                    {
                                        region.setriverdir(xx, yy, 0);
                                        region.setriverjan(xx, yy, 0);
                                        region.setriverjul(xx, yy, 0);

                                        region.setriverdir(xx + 1, yy, 1);

                                        region.setriverdir(xx + 1, yy - 1, 7);
                                        region.setriverjan(xx + 1, yy - 1, riverjan);
                                        region.setriverjul(xx + 1, yy - 1, riverjul);
                                    }
                                    else
                                        stymied = 1;
                                }
                            }

                            if (stymied == 0)
                                didlast = 1;
                        }
                        else
                            didlast = 0;

                        if (dir == 7)
                            xx--;
                        else
                            yy--;

                        if (xx == x && yy == y)
                            keepgoing = 0;

                    } while (keepgoing == 1);
                }
            }
        }
    }

    // Next, lines going northeast.

    for (int i = dx; i <= dx + 16 - diagmaxline / 2; i++)
    {
        for (int j = dy + 16; j >= dy + diagmaxline / 2; j--)
        {
            if (region.riverdir(i, j) == 3 && region.riverdir(i + 1, j + 1) == 0 && region.riverdir(i - 1, j) == 0 && region.riverdir(i - 1, j - 1) == 0 && region.riverdir(i, j - 1) == 0)
            {
                int x = i;
                int y = j;

                bool keepgoing = 1;

                bool step = 1;

                int linelength = 0;

                do
                {
                    linelength++;

                    if (step == 1)
                    {
                        x++;

                        if (x > dx + 16)
                        {
                            x--;
                            keepgoing = 0;
                        }
                        else
                        {
                            if (region.riverdir(x, y) != 1 || region.riverdir(x, y + 1) != 0 || region.riverdir(x + 1, y + 1) != 0 || region.riverdir(x + 1, y) != 0 || region.riverdir(x - 1, y - 1) != 0)
                            {
                                x--;
                                keepgoing = 0;
                            }
                        }

                        step = 0;
                    }
                    else
                    {
                        y--;

                        if (y < dy)
                        {
                            y++;
                            keepgoing = 0;
                        }
                        else
                        {
                            if (region.riverdir(x, y) != 3 || region.riverdir(x + 1, y + 1) != 0 || region.riverdir(x - 1, y) != 0 || region.riverdir(x - 1, y - 1) != 0 || region.riverdir(x, y - 1) != 0)
                            {
                                y++;
                                keepgoing = 0;
                            }
                        }

                        step = 1;
                    }
                } while (keepgoing == 1);

                if (linelength > diagmaxline) // Change some cells so the line looks more varied.
                {
                    int riverjan = region.riverjan(i, j);
                    int riverjul = region.riverjul(i, j);

                    int enddir = region.riverdir(x, y);

                    int xx = i;
                    int yy = j;

                    bool keepgoing = 1;
                    bool didlast = 0;

                    do
                    {
                        bool stymied = 0;

                        int dir = region.riverdir(xx, yy);

                        if (didlast == 0 && random(1, removechance) == 1 && (xx != i || yy != j) && (xx != x || yy != y)) // && countinflows(region,xx,yy)==1)
                        {
                            if (dir == 3)
                            {
                                if (random(1, 2) == 1)
                                {
                                    region.setriverdir(xx, yy, 0);
                                    region.setriverjan(xx, yy, 0);
                                    region.setriverjul(xx, yy, 0);

                                    region.setriverdir(xx, yy + 1, 2);
                                }
                                else
                                {
                                    if (region.riverdir(xx + 1, yy + 1) == 0)
                                    {
                                        region.setriverdir(xx, yy, 0);
                                        region.setriverjan(xx, yy, 0);
                                        region.setriverjul(xx, yy, 0);

                                        region.setriverdir(xx, yy + 1, 3);

                                        region.setriverdir(xx + 1, yy + 1, 1);
                                        region.setriverjan(xx + 1, yy + 1, riverjan);
                                        region.setriverjul(xx + 1, yy + 1, riverjul);
                                    }
                                    else
                                        stymied = 1;
                                }
                            }
                            else
                            {
                                if (random(1, 2) == 1)
                                {
                                    region.setriverdir(xx, yy, 0);
                                    region.setriverjan(xx, yy, 0);
                                    region.setriverjul(xx, yy, 0);

                                    region.setriverdir(xx - 1, yy, 2);
                                }
                                else
                                {
                                    if (region.riverdir(xx - 1, yy - 1) == 0)
                                    {
                                        region.setriverdir(xx, yy, 0);
                                        region.setriverjan(xx, yy, 0);
                                        region.setriverjul(xx, yy, 0);

                                        region.setriverdir(xx - 1, yy, 1);

                                        region.setriverdir(xx - 1, yy - 1, 3);
                                        region.setriverjan(xx - 1, yy - 1, riverjan);
                                        region.setriverjul(xx - 1, yy - 1, riverjul);
                                    }
                                    else
                                        stymied = 1;
                                }
                            }

                            if (stymied == 0)
                                didlast = 1;
                        }
                        else
                            didlast = 0;

                        if (dir == 3)
                            xx++;
                        else
                            yy--;

                        if (xx == x && yy == y)
                            keepgoing = 0;

                    } while (keepgoing == 1);
                }
            }
        }
    }

    // Next, lines going southwest.

    for (int i = dx + 16; i >= dx + diagmaxline / 2; i--)
    {
        for (int j = dy; j <= dy + 16 - diagmaxline / 2; j++)
        {
            if (region.riverdir(i, j) == 7 && region.riverdir(i - 1, j - 1) == 0 && region.riverdir(i + 1, j) == 0 && region.riverdir(i + 1, j + 1) == 0 && region.riverdir(i, j + 1) == 0)
            {
                int x = i;
                int y = j;

                bool keepgoing = 1;

                bool step = 1;

                int linelength = 0;

                do
                {
                    linelength++;

                    if (step == 1)
                    {
                        x--;

                        if (x < dx)
                        {
                            x++;
                            keepgoing = 0;
                        }
                        else
                        {
                            if (region.riverdir(x, y) != 5 || region.riverdir(x, y - 1) != 0 || region.riverdir(x - 1, y - 1) != 0 || region.riverdir(x - 1, y) != 0 || region.riverdir(x + 1, y + 1) != 0)
                            {
                                x++;
                                keepgoing = 0;
                            }
                        }

                        step = 0;
                    }
                    else
                    {
                        y++;

                        if (y > dy + 16)
                        {
                            y--;
                            keepgoing = 0;
                        }
                        else
                        {
                            if (region.riverdir(x, y) != 7 || region.riverdir(x - 1, y - 1) != 0 || region.riverdir(x + 1, y) != 0 || region.riverdir(x + 1, y + 1) != 0 || region.riverdir(x, y + 1) != 0)
                            {
                                y--;
                                keepgoing = 0;
                            }
                        }

                        step = 1;
                    }
                } while (keepgoing == 1);

                if (linelength > diagmaxline) // Change some cells so the line looks more varied.
                {
                    int riverjan = region.riverjan(i, j);
                    int riverjul = region.riverjul(i, j);

                    int enddir = region.riverdir(x, y);

                    int xx = i;
                    int yy = j;

                    bool keepgoing = 1;
                    bool didlast = 0;

                    do
                    {
                        bool stymied = 0;

                        int dir = region.riverdir(xx, yy);

                        if (didlast == 0 && random(1, removechance) == 1 && (xx != i || yy != j) && (xx != x || yy != y)) // && countinflows(region,xx,yy)==1)
                        {
                            if (dir == 7)
                            {
                                if (random(1, 2) == 1)
                                {
                                    region.setriverdir(xx, yy, 0);
                                    region.setriverjan(xx, yy, 0);
                                    region.setriverjul(xx, yy, 0);

                                    region.setriverdir(xx, yy - 1, 6);
                                }
                                else
                                {
                                    if (region.riverdir(xx - 1, yy - 1) == 0)
                                    {
                                        region.setriverdir(xx, yy, 0);
                                        region.setriverjan(xx, yy, 0);
                                        region.setriverjul(xx, yy, 0);

                                        region.setriverdir(xx, yy - 1, 7);

                                        region.setriverdir(xx - 1, yy - 1, 5);
                                        region.setriverjan(xx - 1, yy - 1, riverjan);
                                        region.setriverjul(xx - 1, yy - 1, riverjul);
                                    }
                                    else
                                        stymied = 1;
                                }
                            }
                            else
                            {
                                if (random(1, 2) == 1)
                                {
                                    region.setriverdir(xx, yy, 0);
                                    region.setriverjan(xx, yy, 0);
                                    region.setriverjul(xx, yy, 0);

                                    region.setriverdir(xx + 1, yy, 6);
                                }
                                else
                                {
                                    if (region.riverdir(xx + 1, yy + 1) == 0)
                                    {
                                        region.setriverdir(xx, yy, 0);
                                        region.setriverjan(xx, yy, 0);
                                        region.setriverjul(xx, yy, 0);

                                        region.setriverdir(xx + 1, yy, 5);

                                        region.setriverdir(xx + 1, yy + 1, 7);
                                        region.setriverjan(xx + 1, yy + 1, riverjan);
                                        region.setriverjul(xx + 1, yy + 1, riverjul);
                                    }
                                    else
                                        stymied = 1;
                                }
                            }

                            if (stymied == 0)
                                didlast = 1;
                        }
                        else
                            didlast = 0;

                        if (dir == 7)
                            xx--;
                        else
                            yy++;

                        if (xx == x && yy == y)
                            keepgoing = 0;

                    } while (keepgoing == 1);
                }
            }
        }
    }

    // Now, the straight vertical/horizontal lines.

    // First, lines going south.

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16 - maxline; j++)
        {
            if (region.riverdir(i, j) == 5 && region.riverdir(i - 1, j) == 0 && region.riverdir(i + 1, j) == 0)
            {
                int x = i;
                int y = j;

                bool keepgoing = 1;

                do
                {
                    y++;

                    if (y > dy + 16)
                        keepgoing = 0;
                    else
                    {
                        if (region.riverdir(x, y) != 5 || region.riverdir(x - 1, y) != 0 || region.riverdir(x + 1, y) != 0)
                            keepgoing = 0;
                    }

                } while (keepgoing == 1);

                if (y > j + maxline) // Replace this section of river with a curvier one.
                {
                    int riverjan = region.riverjan(i, j);
                    int riverjul = region.riverjul(i, j);

                    int enddir = region.riverdir(x, y);

                    for (int yy = j; yy <= y; yy++) // Get rid of the existing section.
                    {
                        region.setriverdir(i, yy, 0);
                        region.setriverjan(i, yy, 0);
                        region.setriverjul(i, yy, 0);
                    }

                    mm1.x = (float) i;
                    mm1.y = (float) j;
                    mm2.x = (float) i;
                    mm2.y = (float) (j + y) / 2;
                    mm3.x = (float) x;
                    mm3.y = (float) y;

                    bool found = 0;
                    int addedamount = 0;

                    for (int attempt = 1; attempt < 10; attempt++) // Now try to find a clear path for a new curve.
                    {
                        addedamount = randomsign(random(minadd, maxadd));
                        mm2.x = (float) i + addedamount;

                        if (checkrivercurve(region, dx, dy, pt, mm1, mm2, mm3) == 1)
                        {
                            found = 1;
                            attempt = 10;
                        }
                    }

                    if (found == 0)
                        mm2.x = (float) i;
                    else
                        mm2.x = (float) i + addedamount;

                    makenewrivercurve(region, pt, mm1, mm2, mm3, riverjan, riverjul);

                    region.setriverdir(x, y, enddir);
                }
            }
        }
    }

    // Now, lines going north.

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy + 16; j >= dy + maxline; j--)
        {
            if (region.riverdir(i, j) == 1 && region.riverdir(i - 1, j) == 0 && region.riverdir(i + 1, j) == 0)
            {
                int x = i;
                int y = j;

                bool keepgoing = 1;

                do
                {
                    y--;

                    if (y < dy)
                        keepgoing = 0;
                    else
                    {
                        if (region.riverdir(x, y) != 1 || region.riverdir(x - 1, y) != 0 || region.riverdir(x + 1, y) != 0)
                            keepgoing = 0;
                    }

                } while (keepgoing == 1);

                if (y < j - maxline) // Replace this section of river with a curvier one.
                {
                    int riverjan = region.riverjan(i, j);
                    int riverjul = region.riverjul(i, j);

                    int enddir = region.riverdir(x, y);

                    for (int yy = j; yy >= y; yy--) // Get rid of the existing section.
                    {
                        region.setriverdir(i, yy, 0);
                        region.setriverjan(i, yy, 0);
                        region.setriverjul(i, yy, 0);
                    }

                    mm1.x = (float) i;
                    mm1.y = (float) j;
                    mm2.x = (float) i;
                    mm2.y = (float) (j + y) / 2;
                    mm3.x = (float) x;
                    mm3.y = (float) y;

                    bool found = 0;
                    int addedamount = 0;

                    for (int attempt = 1; attempt < 10; attempt++) // Now try to find a clear path for a new curve.
                    {
                        addedamount = randomsign(random(minadd, maxadd));
                        mm2.x = (float) i + addedamount;

                        if (checkrivercurve(region, dx, dy, pt, mm1, mm2, mm3) == 1)
                        {
                            found = 1;
                            attempt = 10;
                        }
                    }

                    if (found == 0)
                        mm2.x = (float) i;
                    else
                        mm2.x = (float) i + addedamount;

                    makenewrivercurve(region, pt, mm1, mm2, mm3, riverjan, riverjul);

                    region.setriverdir(x, y, enddir);
                }
            }
        }
    }

    // Now, lines going east.

    for (int i = dx; i <= dx + 16 - maxline; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (region.riverdir(i, j) == 3 && region.riverdir(i, j - 1) == 0 && region.riverdir(i, j + 1) == 0)
            {
                int x = i;
                int y = j;

                bool keepgoing = 1;

                do
                {
                    x++;

                    if (x > dx + 16)
                        keepgoing = 0;
                    else
                    {
                        if (region.riverdir(x, y) != 3 || region.riverdir(x, y - 1) != 0 || region.riverdir(x, y + 1) != 0)
                            keepgoing = 0;
                    }

                } while (keepgoing == 1);

                if (x > i + maxline) // Replace this section of river with a curvier one.
                {
                    int riverjan = region.riverjan(i, j);
                    int riverjul = region.riverjul(i, j);

                    int enddir = region.riverdir(x, y);

                    for (int xx = i; xx <= x; xx++) // Get rid of the existing section.
                    {
                        region.setriverdir(xx, j, 0);
                        region.setriverjan(xx, j, 0);
                        region.setriverjul(xx, j, 0);
                    }

                    mm1.x = (float) i;
                    mm1.y = (float) j;
                    mm2.x = (float) (i + x) / 2;
                    mm2.y = (float) j;
                    mm3.x = (float) x;
                    mm3.y = (float) y;

                    bool found = 0;
                    int addedamount = 0;

                    for (int attempt = 1; attempt < 10; attempt++) // Now try to find a clear path for a new curve.
                    {
                        addedamount = randomsign(random(minadd, maxadd));
                        mm2.y = (float) j + addedamount;

                        if (checkrivercurve(region, dx, dy, pt, mm1, mm2, mm3) == 1)
                        {
                            found = 1;
                            attempt = 10;
                        }
                    }

                    if (found == 0)
                        mm2.y = (float) j;
                    else
                        mm2.y = (float) j + addedamount;

                    makenewrivercurve(region, pt, mm1, mm2, mm3, riverjan, riverjul);

                    region.setriverdir(x, y, enddir);
                }
            }
        }
    }

    // Now, lines going west.

    for (int i = dx + 16; i >= dx + maxline; i--)
    {
        for (int j = 16; j <= dy + 16; j++)
        {
            if (region.riverdir(i, j) == 7 && region.riverdir(i, j - 1) == 0 && region.riverdir(i, j + 1) == 0)
            {
                int x = i;
                int y = j;

                bool keepgoing = 1;

                do
                {
                    x--;

                    if (x < dx)
                        keepgoing = 0;
                    else
                    {
                        if (region.riverdir(x, y) != 7 || region.riverdir(x, y - 1) != 0 || region.riverdir(x, y + 1) != 0)
                            keepgoing = 0;
                    }

                } while (keepgoing == 1);

                if (x < i - maxline) // Replace this section of river with a curvier one.
                {
                    int riverjan = region.riverjan(i, j);
                    int riverjul = region.riverjul(i, j);

                    int enddir = region.riverdir(x, y);

                    for (int xx = i; xx >= x; xx--) // Get rid of the existing section.
                    {
                        region.setriverdir(xx, j, 0);
                        region.setriverjan(xx, j, 0);
                        region.setriverjul(xx, j, 0);
                    }

                    mm1.x = (float) i;
                    mm1.y = (float) j;
                    mm2.x = (float) (i + x) / 2;
                    mm2.y = (float) j;
                    mm3.x = (float) x;
                    mm3.y = (float) y;

                    bool found = 0;
                    int addedamount = 0;

                    for (int attempt = 1; attempt < 10; attempt++) // Now try to find a clear path for a new curve.
                    {
                        addedamount = randomsign(random(minadd, maxadd));
                        mm2.y = (float) j + addedamount;

                        if (checkrivercurve(region, dx, dy, pt, mm1, mm2, mm3) == 1)
                        {
                            found = 1;
                            attempt = 10;
                        }
                    }

                    if (found == 0)
                        mm2.y = (float) j;
                    else
                        mm2.y = (float) j + addedamount;

                    makenewrivercurve(region, pt, mm1, mm2, mm3, riverjan, riverjul);

                    region.setriverdir(x, y, enddir);
                }
            }
        }
    }
}

// This checks whether a proposed route for a new curve of river is clear.

bool checkrivercurve(region& region, int dx, int dy, twofloats pt, twofloats mm1, twofloats mm2, twofloats mm3)
{
    for (int n = 1; n <= 2; n++) // Two halves of the curve.
    {
        for (float t = 0; t <= 1.10; t = t + 0.01f)
        {
            if (t <= 1.00)
            {
                if (n == 1)
                    pt = curvepos(mm1, mm1, mm2, mm3, t);
                else
                    pt = curvepos(mm1, mm2, mm3, mm3, t);

                int x = (int)pt.x;
                int y = (int)pt.y;

                if (x<dx || x>dx + 16 || y<dy || y>dy + 16)
                    return 0;

                if (region.riverdir(x, y) != 0)
                    return 0;
            }
        }
    }

    return 1;
}

// This adds a new curve to a river.

void makenewrivercurve(region& region, twofloats pt, twofloats mm1, twofloats mm2, twofloats mm3, int riverjan, int riverjul)
{
    int x = (int)mm1.x;
    int y = (int)mm1.y;

    int lastx = (int)mm1.x;
    int lasty = (int)mm1.y; // Coordinates of last point done.

    region.setriverjan(lastx, lasty, riverjan);
    region.setriverjul(lastx, lasty, riverjul);

    bool finishing = 0;

    for (int n = 1; n <= 2; n++) // Two halves of the curve.
    {
        for (float t = 0; t <= 1.10; t = t + 0.01f)
        {
            bool goahead = 0;

            if (t <= 1.00)
            {
                if (n == 1)
                    pt = curvepos(mm1, mm1, mm2, mm3, t);
                else
                    pt = curvepos(mm1, mm2, mm3, mm3, t);

                x = (int)pt.x;
                y = (int)pt.y;

                goahead = 1;
            }

            if (n == 2 && t > 1.00 && finishing == 0 && (lastx != mm3.x || lasty != mm3.y)) // If for some reason we're at the end of the curve but it hasn't reached the destination, add an extra one at the destination.
            {
                x = (int)mm3.x;
                y = (int)mm3.y;

                goahead = 1;
                finishing = 1;
            }

            if (goahead == 1)
            {
                bool newpoint = 0;

                if (x != lastx || y != lasty)
                    newpoint = 1;

                if (newpoint == 1) // If this is a new point
                {
                    // First, we need to mark on the last point the direction to the new point.

                    int dir = 0;

                    if (x == lastx && y < lasty)
                        dir = 1;

                    if (x > lastx && y < lasty)
                        dir = 2;

                    if (x > lastx && y == lasty)
                        dir = 3;

                    if (x > lastx && y > lasty)
                        dir = 4;

                    if (x == lastx && y > lasty)
                        dir = 5;

                    if (x<lastx && y>lasty)
                        dir = 6;

                    if (x < lastx && y == lasty)
                        dir = 7;

                    if (x < lastx && y < lasty)
                        dir = 8;

                    region.setriverdir(lastx, lasty, dir);

                    // If there is no river here, we need to place the flow onto the map.

                    region.setriverjan(x, y, riverjan);
                    region.setriverjul(x, y, riverjul);

                    // Now update the old x and y.

                    lastx = x;
                    lasty = y;

                }
            }
        }
    }
}

// This tries to remove straight lines from the elevation by adding sort of terrace-like patterns to it.

void addterraces(planet& world, region& region, int dx, int dy, int sx, int sy, boolshapetemplate smalllake[], byteshapetemplate smudge[])
{
    fast_srand((sy * world.width() + sx) + world.nom(sx, sy) + world.julrain(sx, sy));

    int threshold = 100;
    int maxlength = 3;
    int maxelev = world.maxelevation();

    // North-south, looking east.

    for (int i = dx + 1; i <= dx + 15; i++)
    {
        for (int j = dy + 1; j <= dy + 15; j++)
        {
            if (region.riverdir(i + 1, j) == 0 && region.fakedir(i + 1, j) == 0 && region.map(i, j) > region.map(i + 1, j) + threshold)
            {
                int length = 1;
                int jj = j;
                bool keepgoing = 1;

                do
                {
                    jj++;

                    if (region.map(i, jj) > region.map(i + 1, jj) + threshold)
                        length++;
                    else
                        keepgoing = 0;

                    if (jj >= dy + 15)
                        keepgoing = 0;

                } while (keepgoing == 1);

                if (length > maxlength)
                {
                    disruptcliff(world, region, dx, dy, sx, sy, i, j, i, jj, smalllake, smudge);
                }
            }
        }
    }

    // North-south, looking west.

    for (int i = dx + 1; i <= dx + 15; i++)
    {
        for (int j = dy + 1; j <= dy + 15; j++)
        {
            if (region.riverdir(i - 1, j) == 0 && region.fakedir(i - 1, j) == 0 && region.map(i, j) > region.map(i - 1, j) + threshold)
            {
                int length = 1;
                int jj = j;
                bool keepgoing = 1;

                do
                {
                    jj++;

                    if (region.map(i, jj) > region.map(i - 1, jj) + threshold)
                        length++;
                    else
                        keepgoing = 0;

                    if (jj >= dy + 15)
                        keepgoing = 0;

                } while (keepgoing == 1);

                if (length > maxlength)
                {
                    disruptcliff(world, region, dx, dy, sx, sy, i, j, i, jj, smalllake, smudge);
                }
            }
        }
    }

    // East-west, looking south.

    for (int i = dx + 1; i <= dx + 15; i++)
    {
        for (int j = dy + 1; j <= dy + 15; j++)
        {
            if (region.riverdir(i, j + 1) == 0 && region.fakedir(i, j + 1) == 0 && region.map(i, j) > region.map(i, j + 1) + threshold)
            {
                int length = 1;
                int ii = i;
                bool keepgoing = 1;

                do
                {
                    ii++;

                    if (region.map(ii, j) > region.map(ii, j + 1) + threshold)
                        length++;
                    else
                        keepgoing = 0;

                    if (ii >= dx + 15)
                        keepgoing = 0;

                } while (keepgoing == 1);

                if (length > maxlength)
                {
                    disruptcliff(world, region, dx, dy, sx, sy, i, j, ii, j, smalllake, smudge);
                }
            }
        }
    }

    // East-west, looking north.

    for (int i = dx + 1; i <= dx + 15; i++)
    {
        for (int j = dy + 1; j <= dy + 15; j++)
        {
            if (region.riverdir(i, j - 1) == 0 && region.fakedir(i, j - 1) == 0 && region.map(i, j) > region.map(i, j - 1) + threshold)
            {
                int length = 1;
                int ii = i;
                bool keepgoing = 1;

                do
                {
                    ii++;

                    if (region.map(ii, j) > region.map(ii, j - 1) + threshold)
                        length++;
                    else
                        keepgoing = 0;

                    if (ii >= dx + 15)
                        keepgoing = 0;

                } while (keepgoing == 1);

                if (length > maxlength)
                {
                    disruptcliff(world, region, dx, dy, sx, sy, i, j, ii, j, smalllake, smudge);
                }
            }
        }
    }
}

// Same thing, but for lake beds.

void addlaketerraces(planet& world, region& region, int dx, int dy, int sx, int sy, boolshapetemplate smalllake[], byteshapetemplate smudge[])
{
    fast_srand((sy * world.width() + sx) + world.nom(sx, sy) + world.julrain(sx, sy));

    int threshold = 100;
    int maxlength = 3;

    int width = world.width();
    int height = world.height();

    bool foundlake = 0;

    for (int i = sx - 1; i <= sx + 1; i++)
    {
        int ii = i;

        if (ii<0 || i>width)
            ii = wrap(ii, width);

        for (int j = sy - 1; j <= sy + 1; j++)
        {
            if (j >= 0 && j <= height)
            {
                if (world.truelake(i, j) == 1)
                {
                    foundlake = 1;
                    i = sx + 1;
                    j = sy + 1;
                }
            }
        }
    }

    if (foundlake == 0)
        return;

    // North-south, looking east.

    for (int i = dx + 1; i <= dx + 15; i++)
    {
        for (int j = dy + 1; j <= dy + 15; j++)
        {
            if (region.truelake(i + 1, j) == 1 && region.map(i, j) > region.map(i + 1, j) + threshold)
            {
                int length = 1;
                int jj = j;
                bool keepgoing = 1;

                do
                {
                    jj++;

                    if (region.map(i, jj) > region.map(i + 1, jj) + threshold)
                        length++;
                    else
                        keepgoing = 0;

                    if (jj >= dy + 15)
                        keepgoing = 0;

                } while (keepgoing == 1);

                if (length > maxlength)
                {
                    disruptlakecliff(world, region, dx, dy, sx, sy, i, j, i, jj, smalllake, smudge);
                }
            }
        }
    }

    // North-south, looking west.

    for (int i = dx + 1; i <= dx + 15; i++)
    {
        for (int j = dy + 1; j <= dy + 15; j++)
        {
            if (region.truelake(i - 1, j) == 1 && region.map(i, j) > region.map(i - 1, j) + threshold)
            {
                int length = 1;
                int jj = j;
                bool keepgoing = 1;

                do
                {
                    jj++;

                    if (region.map(i, jj) > region.map(i - 1, jj) + threshold)
                        length++;
                    else
                        keepgoing = 0;

                    if (jj >= dy + 15)
                        keepgoing = 0;

                } while (keepgoing == 1);

                if (length > maxlength)
                {
                    disruptlakecliff(world, region, dx, dy, sx, sy, i, j, i, jj, smalllake, smudge);
                }
            }
        }
    }

    // East-west, looking south.

    for (int i = dx + 1; i <= dx + 15; i++)
    {
        for (int j = dy + 1; j <= dy + 15; j++)
        {
            if (region.truelake(i, j + 1) == 1 && region.map(i, j) > region.map(i, j + 1) + threshold)
            {
                int length = 1;
                int ii = i;
                bool keepgoing = 1;

                do
                {
                    ii++;

                    if (region.map(ii, j) > region.map(ii, j + 1) + threshold)
                        length++;
                    else
                        keepgoing = 0;

                    if (ii >= dx + 15)
                        keepgoing = 0;

                } while (keepgoing == 1);

                if (length > maxlength)
                {
                    disruptlakecliff(world, region, dx, dy, sx, sy, i, j, ii, j, smalllake, smudge);
                }
            }
        }
    }

    // East-west, looking north.

    for (int i = dx + 1; i <= dx + 15; i++)
    {
        for (int j = dy + 1; j <= dy + 15; j++)
        {
            if (region.truelake(i, j - 1) == 1 && region.map(i, j) > region.map(i, j - 1) + threshold)
            {
                int length = 1;
                int ii = i;
                bool keepgoing = 1;

                do
                {
                    ii++;

                    if (region.map(ii, j) > region.map(ii, j - 1) + threshold)
                        length++;
                    else
                        keepgoing = 0;

                    if (ii >= dx + 15)
                        keepgoing = 0;

                } while (keepgoing == 1);

                if (length > maxlength)
                {
                    disruptlakecliff(world, region, dx, dy, sx, sy, i, j, ii, j, smalllake, smudge);
                }
            }
        }
    }
}

// This disrupts a long straight cliff on the regional map by making a sort of terrace on it.

void disruptcliff(planet& world, region& region, int dx, int dy, int sx, int sy, int startx, int starty, int endx, int endy, boolshapetemplate smalllake[], byteshapetemplate smudge[])
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    int maxelev = world.maxelevation();

    int disruptchance = 4; // The higher this is, the fewer points will be disrupted.

    int x = startx;
    int y = starty;

    bool islake = 0;

    if (region.truelake(x, y) == 1)
        islake = 1;

    if (islake == 0)
    {
        int riversearch = 2; // Don't do it if there's a river this close.

        for (int i = x - riversearch; i <= x + riversearch; i++)
        {
            if (i >= 0 && i <= rwidth)
            {
                for (int j = y - riversearch; j <= y + riversearch; j++)
                {
                    if (j >= 0 && j <= rheight)
                    {
                        if (region.riverdir(i, j) != 0 || region.fakedir(i, j) != 0)
                            return;
                    }
                }
            }
        }
    }

    do
    {
        if (random(1, disruptchance) == 1)
        {
            int lowest = 1000000;
            int highest = 0;
            bool closeriver = 0;

            if (islake == 0)
            {
                for (int i = x - 1; i <= x + 1; i++)
                {
                    for (int j = y - 1; j <= y + 1; j++)
                    {
                        if (region.map(i, j) < lowest && region.truelake(i, j) == 0 && region.map(i, j) > 0)
                            lowest = region.map(i, j);

                        if (region.map(i, j) > highest)
                            highest = region.map(i, j);

                        if (region.riverdir(i, j) != 0 || region.fakedir(i, j) != 0)
                            closeriver = 1;
                    }
                }
            }
            else
            {
                for (int i = x - 1; i <= x + 1; i++)
                {
                    for (int j = y - 1; j <= y + 1; j++)
                    {
                        if (region.map(i, j) < lowest && region.truelake(i, j) == 1 && region.map(i, j) > 0)
                            lowest = region.map(i, j);

                        if (region.map(i, j) > highest)
                            highest = region.map(i, j);
                    }
                }
            }

            int newheight = lowest;

            if (closeriver == 0)
            {
                if (random(1, 2) == 1)
                    newheight = lowest;
                else
                    newheight = highest;
            }

            if (islake == 1)
                disruptland(region, x, y, newheight, smalllake);
            else
            {
                int janrain = world.janrain(sx, sy);
                int julrain = world.julrain(sx, sy);

                int maxrain = janrain;

                if (julrain > maxrain)
                    maxrain = julrain;

                if (maxrain < 20 || maxrain>120) // Wetter regions use the softer effect
                {
                    if (maxrain < 20)
                        disruptland(region, x, y, newheight, smalllake);
                    else
                        smudgeterrain(world, region, x, y, 2, smudge);
                }
                else
                {
                    if (random(20, 120) > maxrain && random(1, maxelev) < world.noise(sx, sy))
                        disruptland(region, x, y, newheight, smalllake);
                    else
                        smudgeterrain(world, region, x, y, 2, smudge);
                }
            }
        }

        if (x < endx)
            x++;

        if (x > endx)
            x--;

        if (y < endy)
            y++;

        if (y > endy)
            y--;

    } while (x != endx || y != endy);
}

// Same thing, but for lake beds.

void disruptlakecliff(planet& world, region& region, int dx, int dy, int sx, int sy, int startx, int starty, int endx, int endy, boolshapetemplate smalllake[], byteshapetemplate smudge[])
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    int tileelev = world.nom(sx, sy);

    int disruptchance = 4; // The higher this is, the fewer points will be disrupted.

    int x = startx;
    int y = starty;

    do
    {
        if (random(1, disruptchance) == 1)
        {
            int lowest = 1000000;
            int highest = 0;

            for (int i = x - 1; i <= x + 1; i++)
            {
                for (int j = y - 1; j <= y + 1; j++)
                {
                    if (region.map(i, j) < lowest && region.truelake(i, j) == 1 && region.map(i, j) > 0)
                        lowest = region.map(i, j);

                    if (region.map(i, j) > highest)
                        highest = region.map(i, j);
                }
            }

            int newheight = lowest;

            if (newheight > tileelev)
                newheight = tileelev;

            disruptlakebed(region, x, y, newheight, smalllake);
        }

        if (x < endx)
            x++;

        if (x > endx)
            x--;

        if (y < endy)
            y++;

        if (y > endy)
            y--;

    } while (x != endx || y != endy);
}

// This creates an area of lake-shaped land to a certain height.

void disruptland(region& region, int centrex, int centrey, int newheight, boolshapetemplate smalllake[])
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    int shapenumber = random(2, 10);

    int imheight = smalllake[shapenumber].ysize() - 1;
    int imwidth = smalllake[shapenumber].xsize() - 1;

    int x = centrex - imwidth / 2; // Coordinates of the top left corner.
    int y = centrey - imheight / 2;

    bool islake = 0;

    if (region.truelake(centrex, centrey) == 1)
        islake = 1;

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
        jstart = imheight;
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

                if (xx > 0 && xx < rwidth && yy>0 && yy < rheight)
                {
                    if (region.truelake(xx, yy) == int(islake) && region.riverdir(xx, yy) == 0 && region.fakedir(xx, yy) == 0 && region.sea(xx, yy) == 0)
                        region.setmap(xx, yy, newheight);
                }
            }
        }
    }
}

// Same thing, but on lake beds.

void disruptlakebed(region& region, int centrex, int centrey, int newheight, boolshapetemplate smalllake[])
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    int shapenumber = random(2, 10);

    int imheight = smalllake[shapenumber].ysize() - 1;
    int imwidth = smalllake[shapenumber].xsize() - 1;

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
        jstart = imheight;
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

                if (xx > 0 && xx < rwidth && yy>0 && yy < rheight)
                {
                    if (region.truelake(xx, yy) == 1)
                        region.setmap(xx, yy, newheight);
                }
            }
        }
    }
}

// This function makes a tile of undersea elevation.

void makesubmarineelevationtile(planet& world, region& region, int dx, int dy, int sx, int sy, vector<vector<int>>& underseamap, int coords[4][2], int extra)
{
    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();
    int maxelev = world.maxelevation();

    int dextra = extra * 16; // Amount to add to dx and dy to position everything towards the middle of the seabed array.

    dx = dx + dextra;
    dy = dy + dextra;

    float valuemod = 0.4f; //0.1f;

    int min = 1;
    int max = world.maxelevation();

    if (sx<0 || sx>width)
        sx = wrap(sx, width);

    // First, get coordinates for the tiles around this one.

    int eax = sx + 1;
    int eay = sy;
    if (eax > width)
        eax = 0;

    int sox = sx;
    int soy = sy + 1;

    int sex = eax;
    int sey = soy;

    // Now, initialise the four corners.

    int baseelev = sealevel - 5; // Amount to make the corners if they're actually on land. This is so that this whole tile comes out as sea. We'll only apply it to the actual elevation where the latter shows that it should be sea.

    int amount = 2;

    if (dx != 0 && dy != 0 && underseamap[dx][dy] == 0)
    {
        int newamount = world.nom(sx, sy);

        if (newamount > baseelev)
            newamount = baseelev;

        underseamap[dx][dy] = newamount;
    }

    if (dx != 0 && underseamap[dx + 16][dy] == 0)
    {
        int newamount = world.nom(eax, eay);

        if (newamount > baseelev)
            newamount = baseelev;

        underseamap[dx + 16][dy] = newamount;
    }

    if (dx != 0 && underseamap[dx][dy + 16] == 0)
    {
        int newamount = world.nom(sox, soy);

        if (newamount > baseelev)
            newamount = baseelev;

        underseamap[dx][dy + 16] = newamount;
    }

    if (underseamap[dx + 16][dy + 16] == 0)
    {
        int newamount = world.nom(sex, sey);

        if (newamount > baseelev)
            newamount = baseelev;

        underseamap[dx + 16][dy + 16] = newamount;
    }

    // Now we need to fill in the rest.

    bool onlyup = 0; // If it's 1, height values will mostly be raised, not lowered.

    // Now we do the edges.

    int nseed = (sy * width + sx) + world.map(sx, sy) + world.julrain(sx, sy);
    int sseed = (soy * width + sox) + world.map(sox, soy) + world.julrain(sox, soy);
    int wseed = (sy * width + sx) + world.map(sx, sy) + world.julrain(sx, sy);
    int eseed = (eay * width + eax) + world.map(eax, eay) + world.julrain(eax, eay);

    int valuemult = 30; // Amount to multiply the valuemod by for the edges.

    valuemod = 0;

    if (dy != 0 && dx != 0)
    {
        if (underseamap[dx + 8][dy] == 0) // Northern edge
        {
            fast_srand(nseed);

            float s = 16.0f; // Segment size.

            while (s != -1.0f)
            {
                int value = (int)(s * valuemod * valuemult);
                int ss = (int)(s / 2.0f); // Size of half the segment.

                for (int n = 0; n <= (int)(16.0f / s) - 1; n++) // Go through all the segments of this size.
                {
                    int nn = (int)((float)n * s);
                    int ourpoint = nn + ss;

                    if (underseamap[dx + ourpoint][dy] == 0)
                    {
                        int newvalue = (underseamap[dx + nn][dy] + underseamap[dx + nn + (int)s][dy]) / 2;
                        int difference = randomsign(random(0, value));

                        if (onlyup == 1 && random(0, 1) == 1)
                            difference = abs(difference);

                        newvalue = newvalue + difference;

                        if (newvalue > baseelev)
                            newvalue = baseelev;

                        underseamap[dx + ourpoint][dy] = newvalue;
                    }
                }

                if (s > 2.0f)
                    s = s / 2.0f;
                else
                    s = -1.0f;
            }
        }
    }

    if (dx != 0)
    {
        if (underseamap[dx + 8][dy + 16] == 0) // Southern edge
        {
            fast_srand(sseed);

            float s = 16.0f; // Segment seize.

            while (s != -1.0f)
            {
                int value = (int)(s * valuemod * valuemult);
                int ss = (int)(s / 2.0f); // Size of half the segment.

                for (int n = 0; n <= (int)(16.0f / s) - 1; n++) // Go through all the segments of this size.
                {
                    int nn = (int)((float)n * s);
                    int ourpoint = nn + ss;

                    if (underseamap[dx + ourpoint][dy + 16] == 0)
                    {
                        int newvalue = (underseamap[dx + nn][dy + 16] + underseamap[dx + nn + (int)s][dy + 16]) / 2;
                        int difference = randomsign(random(0, value));

                        if (onlyup == 1 && random(0, 1) == 1)
                            difference = abs(difference);

                        newvalue = newvalue + difference;

                        if (newvalue > baseelev)
                            newvalue = baseelev;

                        underseamap[dx + ourpoint][dy + 16] = newvalue;
                    }
                }

                if (s > 2.0f)
                    s = s / 2.0f;
                else
                    s = -1.0f;
            }
        }
    }

    if (dx != 0 && dy != 0)
    {
        if (underseamap[dx][dy + 8] == 0) // Western edge
        {
            fast_srand(wseed);

            float s = 16.0f; // Segment seize.

            while (s != -1.0f)
            {
                int value = (int)(s * valuemod * valuemult);
                int ss = (int)(s / 2.0f); // Size of half the segment.

                for (int n = 0; n <= (int)(16.0f / s) - 1; n++) // Go through all the segments of this size.
                {
                    int nn = (int)((float)n * s);
                    int ourpoint = nn + ss;

                    if (underseamap[dx][dy + ourpoint] == 0)
                    {
                        int newvalue = (underseamap[dx][dy + nn] + underseamap[dx][dy + nn + (int)s]) / 2;
                        int difference = randomsign(random(0, value));

                        if (onlyup == 1 && random(0, 1) == 1)
                            difference = abs(difference);

                        newvalue = newvalue + difference;

                        if (newvalue > baseelev)
                            newvalue = baseelev;

                        underseamap[dx][dy + ourpoint] = newvalue;
                    }
                }

                if (s > 2.0f)
                    s = s / 2.0f;
                else
                    s = -1.0f;
            }
        }
    }

    if (dy != 0)
    {
        if (underseamap[dx + 16][dy + 8] == 0) // Eastern edge
        {
            fast_srand(eseed);

            float s = 16.0f; // Segment seize.

            while (s != -1.0f)
            {
                int value = (int)(s * valuemod * valuemult);
                int ss = (int)(s / 2.0f); // Size of half the segment.

                for (int n = 0; n <= (int)(16.0f / s) - 1; n++) // Go through all the segments of this size.
                {
                    int nn = (int)((float)n * s);
                    int ourpoint = nn + ss;

                    if (underseamap[dx + 16][dy + ourpoint] == 0)
                    {
                        int newvalue = (underseamap[dx + 16][dy + nn] + underseamap[dx + 16][dy + nn + (int)s]) / 2;
                        int difference = randomsign(random(0, value));

                        if (onlyup == 1 && random(0, 1) == 1)
                            difference = abs(difference);

                        newvalue = newvalue + difference;

                        if (newvalue > baseelev)
                            newvalue = baseelev;

                        underseamap[dx + 16][dy + ourpoint] = newvalue;
                    }
                }

                if (s > 2.0f)
                    s = s / 2.0f;
                else
                    s = -1.0f;
            }
        }
    }

    // Now we fill in the middle of the tile. We do this with diamond-square.

    if (dx != 0 && dy != 0)
    {
        fast_srand((sy * width + sx) + world.map(sx, sy) + world.julrain(sx, sy));

        float s = 16.0f; // Size of the section. This will halve each time.

        while (s != -1)
        {
            int value = (int)(s * valuemod); // This is the amount we will vary each new pixel by.
            int ss = (int)(s / 2.0f);

            // First, go over the tile doing the square step.

            for (int i = 0; i <= 16; i = i + (int)s)
            {
                int ii = i + ss;

                for (int j = 0; j <= 16; j = j + (int)s)
                {
                    int jj = j + ss;

                    if (jj >= 0 && jj <= 16 && ii >= 0 && ii <= 16)
                    {
                        if (underseamap[dx + ii][dy + jj] == 0)
                        {
                            int newheight;

                            newheight = submarinesquare(underseamap, dx, dy, (int)s, ii, jj, value, 1, baseelev, coords, onlyup);

                            underseamap[dx + ii][dy + jj] = newheight;
                        }
                    }
                }
            }

            // Now we go over the whole tile again, doing the diamond step.

            for (int i = 0; i <= 16; i = i + (int)s)
            {
                int ii = i + ss;

                for (int j = 0; j <= 16; j = j + (int)s)
                {
                    int jj = j + ss;

                    for (int n = 1; n <= 2; n++) // We have to do the diamond step twice for each tile.
                    {
                        if (n == 1)
                        {
                            ii = i + ss;
                            jj = j;
                        }
                        else
                        {
                            ii = i;
                            jj = j + ss;
                        }

                        if (jj >= 0 && jj <= 16 && ii >= 0 && ii <= 16)
                        {
                            if (underseamap[dx + ii][dy + jj] == 0)
                            {
                                int newheight;

                                newheight = submarinediamond(underseamap, dx, dy, (int)s, ii, jj, value, 1, baseelev, coords, onlyup);

                                underseamap[dx + ii][dy + jj] = newheight;
                            }
                        }
                    }
                }
            }

            if (s > 2.0f) // Now halve the size of the tiles.
                s = s / 2.0f;
            else
                s = -1.0f;
        }
    }
}

// This does the square part of the fractal for the submarine terrain.

int submarinesquare(vector<vector<int>>& underseamap, int dx, int dy, int s, int x, int y, int value, int min, int max, int coords[4][2], bool onlyup)
{
    value = value * 50;

    int dist = s / 2;
    int total = 0;
    int crount = 0;

    coords[0][0] = x - dist;
    coords[0][1] = y - dist;

    coords[1][0] = x + dist;
    coords[1][1] = y - dist;

    coords[2][0] = x + dist;
    coords[2][1] = y + dist;

    coords[3][0] = x - dist;
    coords[3][1] = y + dist;

    for (int n = 0; n <= 3; n++)
    {
        if (coords[n][1] >= 0 && coords[n][1] <= 16 && coords[n][0] >= 0 && coords[n][0] <= 16)
        {
            int thisamount = underseamap[dx + coords[n][0]][dy + coords[n][1]];

            if (thisamount > max)
                thisamount = max;

            total = total + thisamount;
            crount++;
        }
    }

    total = total / crount;

    int difference = randomsign(random(0, value));

    //if (onlyup==1)
    //difference=abs(difference);

    total = total + difference;

    if (total > max)
        total = max;

    if (total < min)
        total = min;

    return total;
}

// This does the diamond part of the fractal for the submarine terrain.

int submarinediamond(vector<vector<int>>& underseamap, int dx, int dy, int s, int x, int y, int value, int min, int max, int coords[4][2], bool onlyup)
{
    value = value * 50;

    int dist = s / 2;
    int total = 0;
    int crount = 0;

    coords[0][0] = x;
    coords[0][1] = y - dist;

    coords[1][0] = x + dist;
    coords[1][1] = y;

    coords[2][0] = x;
    coords[2][1] = y + dist;

    coords[3][0] = x - dist;
    coords[3][1] = y;

    for (int n = 0; n <= 3; n++)
    {
        if (coords[n][1] >= 0 && coords[n][1] <= 16 && coords[n][0] >= 0 && coords[n][0] <= 16)
        {
            int thisamount = underseamap[dx + coords[n][0]][dy + coords[n][1]];

            if (thisamount > max)
                thisamount = max;

            total = total + thisamount;
            crount++;
        }
    }

    total = total / crount;

    int difference = randomsign(random(0, value));

    if (onlyup == 1 && random(0, 1) == 1)
        difference = abs(difference);

    total = total + difference;

    if (total > max)
        total = max;

    if (total < min)
        total = min;

    return total;
}

// This disrupts the underwater terrain.

int disruptsubmarineelevationtile(planet& world, region& region, int dx, int dy, int sx, int sy, vector<vector<int>>& underseamap, byteshapetemplate smudge[], int extra)
{
    int width = world.width();
    int height = world.height();

    int sealevel = world.sealevel();
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    int dextra = extra * 16; // Amount to add to dx and dy to position everything towards the middle of the seabed array.

    dx = dx + dextra;
    dy = dy + dextra;

    fast_srand((sy * width + sx) + world.map(sx, sy) + world.julrain(sx, sy));

    int reps = random(2, 6); // Number of times to do it on each edge.
    int extrareps = random(4, 16); // Number of extra ones to do just anywhere.
    int extravar = 20; // Distance extra smudges might be from the centre.
    int mindiff = random(5, 15); // Only disrupt where the tiles' elevation differs by at least this much.
    int searchdist = 1; // Look this far away from the centre for the lowest elevation.

    int maxbordervar = 12; // Maximum distance from the tile border that each smudge can be.

    // First, get coordinates for the tiles around this one.

    int nox = sx;
    int noy = sy - 1;

    int wex = sx - 1;
    int wey = sy;
    if (wex < 0)
        wex = width;

    if (abs(world.nom(sx, sy) - world.nom(nox, noy)) > mindiff) // Border with the northern tile
    {
        for (int n = 0; n < reps; n++)
        {
            int centrex = dx + random(1, 15);
            int centrey = dy + randomsign(random(1, maxbordervar));

            smudgesubmarineterrain(world, region, centrex, centrey, searchdist, underseamap, smudge);
        }
    }

    if (abs(world.nom(sx, sy) - world.nom(wex, wey)) > mindiff) // Border with the western tile
    {
        for (int n = 0; n < reps; n++)
        {
            int centrex = dx + randomsign(random(1, maxbordervar));
            int centrey = dy + random(1, 15);

            smudgesubmarineterrain(world, region, centrex, centrey, searchdist, underseamap, smudge);
        }
    }

    // Now a few random ones just anywhere.

    for (int n = 0; n < extrareps; n++)
    {
        int centrex = dx + 8 + randomsign(random(1, extravar));
        int centrey = dy + 8 + randomsign(random(1, extravar));

        smudgesubmarineterrain(world, region, centrex, centrey, searchdist, underseamap, smudge);
    }
    return 0;
}

// This smudges an area of underwater terrain.

void smudgesubmarineterrain(planet& world, region& region, int centrex, int centrey, int searchdist, vector<vector<int>>& underseamap, byteshapetemplate smudge[])
{
    int sealevel = world.sealevel();

    int rwidth = RARRAYWIDTH * 4 - 1;
    int rheight = RARRAYHEIGHT * 4 - 1;

    int lowestelev = world.sealevel();

    for (int i = centrex - searchdist; i <= centrex + searchdist; i++)
    {
        for (int j = centrey - searchdist; j <= centrey + searchdist; j++)
        {
            if (i >= 0 && i < RARRAYWIDTH * 4 && j >= 0 && j < RARRAYHEIGHT * 4)
            {
                if (underseamap[i][j] < lowestelev && underseamap[i][j]>0)
                    lowestelev = underseamap[i][j];
            }
        }
    }

    int shapenumber = random(1, 5);

    int imheight = smudge[shapenumber].ysize() - 1;
    int imwidth = smudge[shapenumber].xsize() - 1;

    int x = centrex - imwidth / 2; // Coordinates of the top left corner.
    int y = centrey - imheight / 2;

    bool leftr = random(0, 1); // If it's 1 then we reverse it left-right
    bool downr = random(0, 1); // If it's 1 then we reverse it top-bottom

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
        jstart = imheight;
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

            int xx = x + imap;
            int yy = y + jmap;

            if (xx >= 0 && xx <= rwidth && yy >= 0 && yy <= rheight && underseamap[xx][yy] <= sealevel && underseamap[xx][yy] > lowestelev)
            {
                int point = smudge[shapenumber].point(i, j);

                int oldelev = underseamap[xx][yy];

                int newelev = (oldelev * point + lowestelev * (255 - point)) / 255;

                underseamap[xx][yy] = newelev;
            }
        }
    }
}

// The same thing, but on land.

void smudgeterrain(planet& world, region& region, int centrex, int centrey, int searchdist, byteshapetemplate smudge[])
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();
    int sealevel = world.sealevel();

    float lowestelev = (float)world.maxelevation();

    for (int i = centrex - searchdist; i <= centrex + searchdist; i++)
    {
        for (int j = centrey - searchdist; j <= centrey + searchdist; j++)
        {
            if (i >= 0 && i <= rwidth && j >= 0 && j <= rheight)
            {
                float thiselev = (float)region.map(i, j);

                if (thiselev < lowestelev && thiselev > 0.0f)
                    lowestelev = thiselev;
            }
        }
    }

    int shapenumber = random(1, 4);

    int imheight = smudge[shapenumber].ysize() - 1;
    int imwidth = smudge[shapenumber].xsize() - 1;

    int x = centrex - imwidth / 2; // Coordinates of the top left corner.
    int y = centrey - imheight / 2;

    bool leftr = random(0, 1); // If it's 1 then we reverse it left-right
    bool downr = random(0, 1); // If it's 1 then we reverse it top-bottom

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
        jstart = imheight;
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

            int xx = x + imap;
            int yy = y + jmap;

            if (xx >= 0 && xx <= rwidth && yy >= 0 && yy <= rheight && region.riverdir(xx, yy) == 0)
            {
                float oldelev = (float)region.map(xx, yy);

                if (oldelev > (float)sealevel && oldelev > lowestelev)
                {
                    float point = smudge[shapenumber].point(i, j);

                    float newelev = (oldelev * point + lowestelev * (255.0f - point)) / 255.0f;

                    region.setmap(xx, yy, (int)newelev);
                }
            }
        }
    }
}

// This draws the submarine ridges

void makesubmarineridgelines(planet& world, region& region, int dx, int dy, int sx, int sy, vector<vector<bool>>& undersearidgelines, vector<vector<int>>& warptox, vector<vector<int>>& warptoy)
{
    int width = world.width();
    int height = world.height();

    int maxvar = 3; // Maximum amount the central junction can be offset by

    // First, get coordinates for the tiles around this one.

    int eax = sx + 1;
    int eay = sy;
    if (eax > width)
        eax = 0;

    int nox = sx;
    int noy = sy - 1;

    int nex = eax;
    int ney = noy;

    int sex = eax;
    int sey = sy + 1;

    // These are for getting the offset amounts.

    int sxx = sx + width / 2;

    if (sxx > width)
        sxx = width - sxx;

    int eaxx = eax + width / 2;

    if (eaxx > width)
        eaxx = width - eaxx;

    int noxx = sxx;
    int nexx = eaxx;
    int sexx = eaxx;

    int thisseed = (sy * width + sx) + world.map(sx, sy) + world.julrain(sx, sy);

    int nseed = (noy * width + nox) + world.map(nox, noy) + world.julrain(nox, noy);
    int neseed = (ney * width + nex) + world.map(nex, ney) + world.julrain(nex, ney);
    int eseed = (eay * width + eax) + world.map(eax, eay) + world.julrain(eax, eay);
    int seseed = (sey * width + sex) + world.map(sex, sey) + world.julrain(sex, sey);

    fast_srand(thisseed);

    int fromx = dx + 8 + randomsign(random(1, maxvar)) + world.oceanridgeoffset(sx, sy);
    int fromy = dy + 8 + randomsign(random(1, maxvar)) + world.oceanridgeoffset(sxx, sy);

    int tox, toy;

    // Now mark out each ridge in turn.

    if (getoceanridge(world, sx, sy, 1) == 1) // North
    {
        fast_srand(nseed);

        tox = dx + 8 + randomsign(random(1, maxvar)) + world.oceanridgeoffset(nox, noy);
        toy = dy - 8 + randomsign(random(1, maxvar)) + world.oceanridgeoffset(noxx, noy);

        marksubmarineridgeline(region, undersearidgelines, fromx, fromy, tox, toy, warptox, warptoy);
    }

    if (getoceanridge(world, sx, sy, 2) == 1) // Northeast
    {
        fast_srand(neseed);

        tox = dx + 24 + randomsign(random(1, maxvar)) + world.oceanridgeoffset(nex, ney);
        toy = dy - 8 + randomsign(random(1, maxvar)) + world.oceanridgeoffset(nexx, ney);

        marksubmarineridgeline(region, undersearidgelines, fromx, fromy, tox, toy, warptox, warptoy);
    }

    if (getoceanridge(world, sx, sy, 3) == 1) // East
    {
        fast_srand(eseed);

        tox = dx + 24 + randomsign(random(1, maxvar)) + world.oceanridgeoffset(eax, eay);
        toy = dy + 8 + randomsign(random(1, maxvar)) + world.oceanridgeoffset(eaxx, eay);

        marksubmarineridgeline(region, undersearidgelines, fromx, fromy, tox, toy, warptox, warptoy);

    }

    if (getoceanridge(world, sx, sy, 4) == 1) // Southeast
    {
        fast_srand(seseed);

        tox = dx + 24 + randomsign(random(1, maxvar)) + world.oceanridgeoffset(nex, ney);
        toy = dy + 24 + randomsign(random(1, maxvar)) + world.oceanridgeoffset(nexx, ney);

        marksubmarineridgeline(region, undersearidgelines, fromx, fromy, tox, toy, warptox, warptoy);
    }
}

// This marks an undersea ridge line.

void marksubmarineridgeline(region& region, vector<vector<bool>>& undersearidgelines, int fromx, int fromy, int tox, int toy, vector<vector<int>>& warptox, vector<vector<int>>& warptoy)
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    int regwidthbegin = region.regwidthbegin();
    int regwidthend = region.regwidthend();
    int regheightbegin = region.regheightbegin();
    int regheightend = region.regheightend();

    int maxmidvar = 2; // Maximum amount the midpoint between one junction and the next can be offset by.

    int midx = (fromx + tox) / 2 + randomsign(random(1, maxmidvar));
    int midy = (fromy + toy) / 2 + randomsign(random(1, maxmidvar));

    int newfromx = fromx;
    int newfromy = fromy;
    int newmidx = midx;
    int newmidy = midy;
    int newtox = tox;
    int newtoy = toy;

    if (fromx >= regwidthbegin && fromx <= regwidthend && fromy >= regheightbegin && fromy <= regheightend)
    {
        newfromx = warptox[fromx][fromy];
        newfromy = warptoy[fromx][fromy];
    }

    if (midx >= regwidthbegin && midx <= regwidthend && midy >= regheightbegin && midy <= regheightend)
    {
        newmidx = warptox[midx][midy];
        newmidy = warptoy[midx][midy];
    }

    if (tox >= regwidthbegin && tox <= regwidthend && toy >= regheightbegin && toy <= regheightend)
    {
        newtox = warptox[tox][toy];
        newtoy = warptoy[tox][toy];
    }

    twofloats pt, mm1, mm2, mm3;

    mm1.x = (float)newfromx;
    mm1.y = (float)newfromy;
    mm2.x = (float)newmidx;
    mm2.y = (float)newmidy;
    mm3.x = (float)newtox;
    mm3.y = (float)newtoy;

    for (int n = 1; n <= 2; n++) // Two halves of the curve.
    {
        for (float t = 0.0f; t <= 1.0f; t = t + 0.01f)
        {
            if (n == 1)
                pt = curvepos(mm1, mm1, mm2, mm3, t);
            else
                pt = curvepos(mm1, mm2, mm3, mm3, t);

            int x = (int)pt.x;
            int y = (int)pt.y;

            if (x >= 0 && x <= rwidth && y >= 0 && y <= rheight)
                undersearidgelines[x][y] = 1;
        }
    }
}

// This draws the actual undersea ridges themselves.

void drawsubmarineridges(planet& world, region& region, int dx, int dy, int sx, int sy, vector<vector<bool>>& undersearidgelines, peaktemplate& peaks, vector<vector<int>>& undersearidges)
{
    int width = world.width();
    int height = world.height();

    int rwidth = region.rwidth();
    int rheight = region.rheight();

    int peakchance = 2; // The higher this is, the fewer peaks will be pasted.

    int tileheight = world.nom(sx, sy);

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (undersearidgelines[i][j] == 1 && random(1, peakchance) == 1)
            {
                int peakheight = world.oceanridgeheights(sx, sy);

                peakheight = random(peakheight / 2, peakheight * 2);

                int templateno = random(1, 2);

                pastesubmarinepeak(world, region, i, j, (float)peakheight, templateno, peaks, undersearidges, rwidth, rheight);
            }
        }
    }

    // Now add some extra peaks.

    int maxvar = 3; // Maximum amount the central junction can be offset by
    int sxx = sx + width / 2;

    if (sxx > width)
        sxx = width - sxx;

    fast_srand((sy * width + sx) + world.map(sx, sy) + world.julrain(sx, sy));

    int fromx = dx + 8 + randomsign(random(1, maxvar)) + world.oceanridgeoffset(sx, sy);
    int fromy = dy + 8 + randomsign(random(1, maxvar)) + world.oceanridgeoffset(sxx, sy);

    int newx = sx;
    int newy = sy + height / 2;

    if (newy > height)
        newy = newy - height;

    int extrapeaks = world.oceanridgeoffset(newx, newy);

    extrapeaks = extrapeaks + 16;

    if (extrapeaks < 1)
        extrapeaks = 1;

    if (extrapeaks > 32)
        extrapeaks = 32;

    int limit = 16 * 16 + 16;

    for (int n = 1; n <= extrapeaks; n++)
    {
        int thisx = randomsign(random(1, 16));
        int thisy = randomsign(random(1, 16));

        if (thisx * thisx + thisy * thisy < limit)
        {
            int peakheight = world.oceanridgeheights(sx, sy);

            peakheight = random(peakheight / 4, peakheight);

            int templateno = random(1, 2);

            pastesubmarinepeak(world, region, fromx + thisx, fromy + thisy, (float)peakheight, templateno, peaks, undersearidges, rwidth, rheight);
        }
    }
}

// This pastes a peak onto the underseamap.

void pastesubmarinepeak(planet& world, region& region, int x, int y, float peakheight, int templateno, peaktemplate& peaks, vector<vector<int>>& undersearidges, int rwidth, int rheight)
{
    int sealevel = world.sealevel();
    float maxelev = (float)world.maxelevation();

    int span = peaks.span(templateno);
    int centrex = peaks.centrex(templateno);
    int centrey = peaks.centrey(templateno);

    span--;

    int leftr = random(1, 2); // If it's 1 then we reverse it left-right
    int downr = random(1, 2); // If it's 1 then we reverse it top-bottom

    int a, b, c, d, e, f;

    if (leftr == 1)
    {
        a = 0;
        b = span;
        c = 1;
    }
    else
    {
        a = span;
        b = 0;
        c = -1;

        centrex = span - centrex;
    }

    if (downr == 1)
    {
        d = 0;
        e = span;
        f = 1;
    }
    else
    {
        d = span;
        e = 0;
        f = -1;

        centrey = span - centrey;
    }

    int imap = -1;
    int jmap = -1;

    x = x - centrex;
    y = y - centrey; // Coordinates of the top left corner.

    // Now actually paste the peak on.

    for (int i = a; i != b; i = i + c)
    {
        imap++;
        jmap = -1;

        for (int j = d; j != e; j = j + f)
        {
            jmap++;

            if (peaks.peakmap(templateno, i, j) != 0)
            {
                int xx = x + imap;
                int yy = y + jmap;

                if (xx >= 0 && xx <= rwidth && yy >= 0 && yy <= rheight)
                {
                    float val = (float)peaks.peakmap(templateno, i, j);

                    val = val * 0.1f;

                    float newheight = peakheight * val;

                    if (newheight > maxelev)
                        newheight = maxelev;

                    if (undersearidges[xx][yy] < (int)newheight)
                        undersearidges[xx][yy] = (int)newheight;
                }
            }
        }
    }
}

// This draws the radiating spines away from the central rift.

void makesubmarineriftradiations(planet& world, region& region, int dx, int dy, int sx, int sy, vector<vector<int>>& underseaspikes, peaktemplate& peaks, int extra)
{
    int width = world.width();
    int height = world.height();

    if (sx<0 || sx>width)
        sx = wrap(sx, width);

    int dextra = extra * 16;

    dx = dx + dextra;
    dy = dy + dextra;

    int minlength = 5 * 16;
    int maxlength = 15 * 16; // The max/min length of each radiating spine.

    fast_srand((sy * width + sx) + world.map(sx, sy) + world.janrain(sx, sy));

    float angle = 0.0f - (float)world.oceanridgeangle(sx, sy);

    while (angle < 0.0f)
        angle = angle + 360.0f;

    int ssx = sx + width / 2;

    if (ssx > width)
        ssx = ssx - width;

    int ssy = sy + height / 2;

    if (ssy > height)
        ssy = ssy - height;

    int maxspines = abs(world.oceanridgeoffset(ssx, ssy)); // Number of spines depends on this global fractal.

    maxspines = maxspines / 2;

    if (maxspines == 0)
        return;

    for (int spine = 1; spine <= maxspines; spine++)
    {
        int startx = dx + random(1, 15);
        int starty = dy + random(1, 15);

        float length = (float)random(minlength, maxlength);

        float thisangle = angle + (float)(randomsign(random(1, 5)));

        float peakheight = (float)world.oceanridgeheights(sx, sy);

        peakheight = (float)random((int)(peakheight / 4.0f), (int)(peakheight * 1.5f));

        float heightstep = peakheight / length; // Amount to reduce the height by, each step.

        bool lower = 0;

        if (random(1, 4) == 1)
            lower = 1;

        for (int n = 1; n <= 2; n++) // Two halves - one on each side of the rift.
        {
            float fangle;

            if (n == 1)
                fangle = angle * 0.01745329f;
            else
            {
                fangle = angle + 180.0f;

                if (fangle > 360.0f)
                    fangle = fangle - 360.0f;

                fangle = fangle * 0.01745329f;
            }

            int endx = startx + (int)(length * (float)sin(fangle));
            int endy = starty + (int)(length * (float)cos(fangle));

            drawriftspine(world, region, dx, dy, sx, sy, startx, starty, endx, endy, peakheight, heightstep, underseaspikes, peaks, lower);
        }
    }
}

// This draws a line radiating across the sea bed.
// Uses Bresenham's line algorithm - http://rosettacode.org/wiki/Bitmap/Bresenham%27s_line_algorithm#C.2B.2B

void drawriftspine(planet& world, region& region, int dx, int dy, int sx, int sy, int x1, int y1, int x2, int y2, float peakheight, float heightstep, vector<vector<int>>& underseaspikes, peaktemplate& peaks, bool lower)
{
    int width = world.width();
    int height = world.height();

    int rwidth = region.rwidth();
    int rheight = region.rheight();

    float thispeakheight = peakheight;

    int peakchance = 5; // The higher this is, the fewer peaks this will make.

    bool begun = 0;
    bool reversed = 0;

    const bool steep = (fabs(y2 - y1) > fabs(x2 - x1));

    if (steep)
    {
        std::swap(x1, y1);
        std::swap(x2, y2);
    }

    if (x1 > x2)
    {
        std::swap(x1, x2);
        std::swap(y1, y2);
    }

    const float px = (float)(x2 - x1);
    const float py = (float)fabs(y2 - y1);

    float error = px / 2.0f;
    const int ystep = (y1 < y2) ? 1 : -1;
    int y = (int)y1;

    const int maxX = (int)x2;

    for (int x = (int)x1; x <= maxX; x++)
    {
        int plotx, ploty;

        if (steep)
        {
            plotx = y;
            ploty = x;
        }
        else
        {
            plotx = x;
            ploty = y;
        }

        if (begun == 0)
        {
            begun = 1;

            if (abs(plotx - x1) < abs(plotx - x2) || abs(ploty - y1) < abs(ploty - y2))
                reversed = 0;
            else
            {
                reversed = 1;
                thispeakheight = 0;
            }
        }

        if (lower == 1 || random(1, peakchance) == 1)
        {
            int xdiff = plotx - dx;
            int ydiff = ploty - dy;

            int sxdiff = xdiff / 16;
            int sydiff = ydiff / 16;

            int thissx = sx + sxdiff; // The global cell that we're currently in.
            int thissy = sy + sydiff;

            if (thissx<0 || thissx>width)
                thissx = wrap(thissx, width);

            if (reversed == 0)
                thispeakheight = thispeakheight - heightstep;
            else
                thispeakheight = thispeakheight + heightstep;

            if (plotx >= 0 && plotx < RARRAYWIDTH * 4 && ploty >= 0 && ploty < RARRAYHEIGHT * 4)
            {
                if (lower == 1)
                    underseaspikes[plotx][ploty] = underseaspikes[plotx][ploty] - (int)thispeakheight;
                else
                {
                    int templateno = 2; //random(1,2);
                    pastesubmarinepeak(world, region, plotx, ploty, peakheight, templateno, peaks, underseaspikes, RARRAYWIDTH * 4 - 1, RARRAYHEIGHT * 4 - 1);
                }
            }
        }

        error -= py;
        if (error < 0)
        {
            y += ystep;
            error += px;
        }
    }
}

// This draws the central submarine rift mountains.

void makesubmarineriftmountains(planet& world, region& region, int dx, int dy, int sx, int sy, vector<vector<int>>& undersearidges, peaktemplate& peaks, vector<vector<int>>& warptox, vector<vector<int>>& warptoy)
{
    int width = world.width();
    int height = world.height();

    int maxvar = 3; // Maximum amount the central junction can be offset by

    // First, get coordinates for the tiles around this one.

    int eax = sx + 1;
    int eay = sy;
    if (eax > width)
        eax = 0;

    int nox = sx;
    int noy = sy - 1;

    int nex = eax;
    int ney = noy;

    int sex = eax;
    int sey = sy + 1;

    // These are for getting the offset amounts.

    int sxx = sx + width / 2;

    if (sxx > width)
        sxx = width - sxx;

    int eaxx = eax + width / 2;

    if (eaxx > width)
        eaxx = width - eaxx;

    int noxx = sxx;
    int nexx = eaxx;
    int sexx = eaxx;

    int thisseed = (sy * width + sx) + world.map(sx, sy) + world.julrain(sx, sy);

    int nseed = (noy * width + nox) + world.map(nox, noy) + world.julrain(nox, noy);
    int neseed = (ney * width + nex) + world.map(nex, ney) + world.julrain(nex, ney);
    int eseed = (eay * width + eax) + world.map(eax, eay) + world.julrain(eax, eay);
    int seseed = (sey * width + sex) + world.map(sex, sey) + world.julrain(sex, sey);

    fast_srand(thisseed);

    int fromx = dx + 8 + randomsign(random(1, maxvar));//+world.oceanridgeoffset(sx,sy);
    int fromy = dy + 8 + randomsign(random(1, maxvar));//+world.oceanridgeoffset(sxx,sy);

    int tox, toy;

    // Now make the mountains around the rifts.

    if (world.oceanrifts(nox, noy) != 0) // North
    {
        fast_srand(nseed);

        tox = dx + 8 + randomsign(random(1, maxvar));//+world.oceanridgeoffset(nox,noy);
        toy = dy - 8 + randomsign(random(1, maxvar));//+world.oceanridgeoffset(noxx,noy);

        makeoceanicriftmountains(world, region, sx, sy, fromx, fromy, tox, toy, undersearidges, peaks, warptox, warptoy);
    }

    if (world.oceanrifts(nex, ney) != 0) // Northeast
    {
        fast_srand(neseed);

        tox = dx + 24 + randomsign(random(1, maxvar));//+world.oceanridgeoffset(nex,ney);
        toy = dy - 8 + randomsign(random(1, maxvar));//+world.oceanridgeoffset(nexx,ney);

        makeoceanicriftmountains(world, region, sx, sy, fromx, fromy, tox, toy, undersearidges, peaks, warptox, warptoy);
    }

    if (world.oceanrifts(eax, eay) != 0) // East
    {
        fast_srand(eseed);

        tox = dx + 24 + randomsign(random(1, maxvar));//+world.oceanridgeoffset(eax,eay);
        toy = dy + 8 + randomsign(random(1, maxvar));//+world.oceanridgeoffset(eaxx,eay);

        makeoceanicriftmountains(world, region, sx, sy, fromx, fromy, tox, toy, undersearidges, peaks, warptox, warptoy);
    }

    if (world.oceanrifts(sex, sey) != 0) // Southeast
    {
        fast_srand(seseed);

        tox = dx + 24 + randomsign(random(1, maxvar));//+world.oceanridgeoffset(sex,sey);
        toy = dy + 24 + randomsign(random(1, maxvar));//+world.oceanridgeoffset(sexx,sey);

        makeoceanicriftmountains(world, region, sx, sy, fromx, fromy, tox, toy, undersearidges, peaks, warptox, warptoy);
    }
}

// This draws the mountains around the central oceanic rifts.

void makeoceanicriftmountains(planet& world, region& region, int sx, int sy, int fromx, int fromy, int tox, int toy, vector<vector<int>>& undersearidges, peaktemplate& peaks, vector<vector<int>>& warptox, vector<vector<int>>& warptoy)
{
    int width = world.width();
    int height = world.height();

    int rwidth = region.rwidth();
    int rheight = region.rheight();

    int regwidthbegin = region.regwidthbegin();
    int regwidthend = region.regwidthend();
    int regheightbegin = region.regheightbegin();
    int regheightend = region.regheightend();

    // First, adjust the start and end points using the warp arrays, so that the range looks a little more natural.

    if (fromx >= regwidthbegin && fromx <= regwidthend && fromy >= regheightbegin && fromy <= regheightend)
    {
        int x1 = fromx;
        int y1 = fromy;

        fromx = warptox[x1][y1];
        fromy = warptoy[x1][y1];
    }

    if (tox >= regwidthbegin && tox <= regwidthend && toy >= regheightbegin && toy <= regheightend)
    {
        int x1 = tox;
        int y1 = toy;

        tox = warptox[x1][y1];
        toy = warptoy[x1][y1];
    }

    int tilepeakheight = 0;

    for (int i = sx - 1; i <= sx + 1; i++)
    {
        int ii = i;
        if (ii<0 || ii>width)
            ii = wrap(ii, width);

        for (int j = sy - 1; j <= sy + 1; j++)
        {
            if (j >= 0 && j <= height)
            {
                if (world.oceanridgeheights(ii, j) > tilepeakheight)
                    tilepeakheight = world.oceanridgeheights(ii, j);
            }
        }
    }

    int maxmidvar = 2; // Maximum amount the midpoint between one junction and the next can be offset by.

    int midx = (fromx + tox) / 2 + randomsign(random(1, maxmidvar));
    int midy = (fromy + toy) / 2 + randomsign(random(1, maxmidvar));

    twofloats pt, mm1, mm2, mm3;

    mm1.x = (float) fromx;
    mm1.y = (float) fromy;
    mm2.x = (float) midx;
    mm2.y = (float) midy;
    mm3.x = (float) tox;
    mm3.y = (float) toy;

    int lastx = -100;
    int lasty = -100;

    for (int n = 1; n <= 2; n++) // Two halves of the curve.
    {
        for (float t = 0.0f; t <= 1.0f; t = t + 0.01f)
        {
            if (n == 1)
                pt = curvepos(mm1, mm1, mm2, mm3, t);
            else
                pt = curvepos(mm1, mm2, mm3, mm3, t);

            int x = (int)pt.x;
            int y = (int)pt.y;

            if (x != lastx || y != lasty)
            {
                lastx = x;
                lasty = y;

                for (int radius = 32; radius > 4; radius = radius / 2)
                {
                    for (int thispeak = 1; thispeak <= 4; thispeak++)
                    {
                        int thisx = randomsign(random(1, radius));
                        int thisy = randomsign(random(1, radius));

                        if (thisx * thisx + thisy * thisy < radius * radius + radius)
                        {
                            float peakheight = (float)tilepeakheight;

                            peakheight = (float)random((int)peakheight, (int)(peakheight * 1.5f));

                            int templateno = random(1, 2);
                            pastesubmarinepeak(world, region, x + thisx, y + thisy, peakheight, templateno, peaks, undersearidges, rwidth, rheight);
                        }
                    }
                }
            }
        }
    }
}

// This puts the rift itself into the oceanic rifts.

void makesubmarinerift(planet& world, region& region, int dx, int dy, int sx, int sy, vector<vector<int>>& undersearidges, byteshapetemplate smudge[], vector<vector<int>>& warptox, vector<vector<int>>& warptoy)
{
    int width = world.width();
    int height = world.height();

    int tileriftheight = world.nom(sx, sy) - world.oceanrifts(sx, sy);

    if (tileriftheight < 1)
        tileriftheight = 1;

    int maxvar = 3; // Maximum amount the central junction can be offset by

    // First, get coordinates for the tiles around this one.

    int eax = sx + 1;
    int eay = sy;
    if (eax > width)
        eax = 0;

    int nox = sx;
    int noy = sy - 1;

    int nex = eax;
    int ney = noy;

    int sex = eax;
    int sey = sy + 1;

    // These are for getting the offset amounts.

    int sxx = sx + width / 2;

    if (sxx > width)
        sxx = width - sxx;

    int eaxx = eax + width / 2;

    if (eaxx > width)
        eaxx = width - eaxx;

    int noxx = sxx;
    int nexx = eaxx;
    int sexx = eaxx;

    int thisseed = (sy * width + sx) + world.map(sx, sy) + world.julrain(sx, sy);

    int nseed = (noy * width + nox) + world.map(nox, noy) + world.julrain(nox, noy);
    int neseed = (ney * width + nex) + world.map(nex, ney) + world.julrain(nex, ney);
    int eseed = (eay * width + eax) + world.map(eax, eay) + world.julrain(eax, eay);
    int seseed = (sey * width + sex) + world.map(sex, sey) + world.julrain(sex, sey);

    fast_srand(thisseed);

    int fromx = dx + 8 + randomsign(random(1, maxvar));//+world.oceanridgeoffset(sx,sy);
    int fromy = dy + 8 + randomsign(random(1, maxvar));//+world.oceanridgeoffset(sxx,sy);

    int tox, toy;

    // Now make the rifts.

    if (world.oceanrifts(nox, noy) != 0) // North
    {
        fast_srand(nseed);

        tox = dx + 8 + randomsign(random(1, maxvar));//+world.oceanridgeoffset(nox,noy);
        toy = dy - 8 + randomsign(random(1, maxvar));//+world.oceanridgeoffset(noxx,noy);

        makesubmarineriftvalley(world, region, sx, sy, fromx, fromy, tox, toy, tileriftheight, undersearidges, smudge, warptox, warptoy);
    }

    if (world.oceanrifts(nex, ney) != 0) // Northeast
    {
        fast_srand(neseed);

        tox = dx + 24 + randomsign(random(1, maxvar));//+world.oceanridgeoffset(nex,ney);
        toy = dy - 8 + randomsign(random(1, maxvar));//+world.oceanridgeoffset(nexx,ney);

        makesubmarineriftvalley(world, region, sx, sy, fromx, fromy, tox, toy, tileriftheight, undersearidges, smudge, warptox, warptoy);
    }

    if (world.oceanrifts(eax, eay) != 0) // East
    {
        fast_srand(eseed);

        tox = dx + 24 + randomsign(random(1, maxvar));//+world.oceanridgeoffset(eax,eay);
        toy = dy + 8 + randomsign(random(1, maxvar));//+world.oceanridgeoffset(eaxx,eay);

        makesubmarineriftvalley(world, region, sx, sy, fromx, fromy, tox, toy, tileriftheight, undersearidges, smudge, warptox, warptoy);
    }

    if (world.oceanrifts(sex, sey) != 0) // Southeast
    {
        fast_srand(seseed);

        tox = dx + 24 + randomsign(random(1, maxvar));//+world.oceanridgeoffset(sex,sey);
        toy = dy + 24 + randomsign(random(1, maxvar));//+world.oceanridgeoffset(sexx,sey);

        makesubmarineriftvalley(world, region, sx, sy, fromx, fromy, tox, toy, tileriftheight, undersearidges, smudge, warptox, warptoy);
    }
}

// This draws the oceanic rifts.

void makesubmarineriftvalley(planet& world, region& region, int sx, int sy, int fromx, int fromy, int tox, int toy, int tileriftheight, vector<vector<int>>& undersearidges, byteshapetemplate smudge[], vector<vector<int>>& warptox, vector<vector<int>>& warptoy)
{
    int width = world.width();
    int height = world.height();

    int rwidth = region.rwidth();
    int rheight = region.rheight();

    int regwidthbegin = region.regwidthbegin();
    int regwidthend = region.regwidthend();
    int regheightbegin = region.regheightbegin();
    int regheightend = region.regheightend();

    int amount = 8;

    int smudgechance = 2; // The higher this is, the fewer smudges there will be
    int nudge = 3; // The higher this is, the more irregular the line will be

    int maxmidvar = 2; // Maximum amount the midpoint between one junction and the next can be offset by.

    // First, adjust the start and end points using the warp arrays, so that the range looks a little more natural.

    if (fromx >= regwidthbegin && fromx <= regwidthend && fromy >= regheightbegin && fromy <= regheightend)
    {
        int x1 = fromx;
        int y1 = fromy;

        fromx = warptox[x1][y1];
        fromy = warptoy[x1][y1];
    }

    if (tox >= regwidthbegin && tox <= regwidthend && toy >= regheightbegin && toy <= regheightend)
    {
        int x1 = tox;
        int y1 = toy;

        tox = warptox[x1][y1];
        toy = warptoy[x1][y1];
    }

    int midx = (fromx + tox) / 2 + randomsign(random(1, maxmidvar));
    int midy = (fromy + toy) / 2 + randomsign(random(1, maxmidvar));

    twofloats pt, mm1, mm2, mm3;

    mm1.x = (float) fromx;
    mm1.y = (float) fromy;
    mm2.x = (float) midx;
    mm2.y = (float) midy;
    mm3.x = (float) tox;
    mm3.y = (float) toy;

    int lastx = -1000000;
    int lasty = -1000000;

    for (int n = 1; n <= 2; n++) // Two halves of the curve.
    {
        for (float t = 0.0f; t <= 1.0f; t = t + 0.01f)
        {
            if (n == 1)
                pt = curvepos(mm1, mm1, mm2, mm3, t);
            else
                pt = curvepos(mm1, mm2, mm3, mm3, t);

            int centrex = (int)pt.x;
            int centrey = (int)pt.y;

            if (centrex != lastx || centrey != lasty)
            {
                lastx = centrex;
                lasty = centrey;

                if (random(1, smudgechance) == 1)
                {
                    int shapenumber = random(1, 4);

                    int imheight = smudge[shapenumber].ysize() - 1;
                    int imwidth = smudge[shapenumber].xsize() - 1;

                    int x = centrex - imwidth / 2;//+randomsign(random(1,nudge)); // Coordinates of the top left corner.
                    int y = centrey - imheight / 2;//+randomsign(random(1,nudge));

                    bool leftr = random(0, 1); // If it's 1 then we reverse it left-right
                    bool downr = random(0, 1); // If it's 1 then we reverse it top-bottom

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
                        jstart = imheight;
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

                            int xx = x + imap;
                            int yy = y + jmap;

                            if (xx >= 0 && xx <= rwidth && yy >= 0 && yy <= rheight && world.nom(sx, sy) > tileriftheight)
                            {
                                int point = smudge[shapenumber].point(i, j);

                                int oldelev = region.map(xx, yy);

                                int newelev = (oldelev * point + tileriftheight * (255 - point)) / 255;

                                region.setmap(xx, yy, newelev);

                                if (newelev != oldelev)
                                    undersearidges[xx][yy] = 0;
                            }
                        }
                    }
                }
            }
        }
    }
}

// This removes extra land around Aegean-style islands.

void trimmountainislands(planet& world, region& region, int dx, int dy, int sx, int sy, vector<vector<int>>& rmountainmap, vector<vector<bool>>& riverinlets, vector<vector<bool>>& globalestuaries, boolshapetemplate smalllake[])
{
    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    bool removepoints[17][17]; // This will hold the points where we will carve away some land.

    for (int i = 0; i < 17; i++)
    {
        for (int j = 0; j < 17; j++)
            removepoints[i][j] = 0;
    }

    // Now mark some points on that array.

    for (int i = 0; i < 17; i++)
    {
        int ii = dx + i;
        
        for (int j = 0; j < 17; j++)
        {
            int jj = dy + j;

            if (region.map(ii, jj) <= sealevel && random(1, 4) == 1)
            {
                bool foundland = 0;
                
                for (int k = ii - 1; k <= ii + 1; k++)
                {
                    if (k >= 0 && k <= rwidth)
                    {
                        for (int l = jj - 1; l <= jj + 1; l++)
                        {
                            if (l >= 0 && l <= rheight)
                            {
                                if (region.map(k, l) > sealevel)
                                {
                                    foundland = 1;
                                    k = ii + 1;
                                    l = jj + 1;
                                }
                            }
                        }
                    }
                }

                if (foundland)
                    removepoints[i][j] = 1;
            }
        }
    }

    // Now remove land around those points.

    int maxsize = 11;
    int minsize = 3;

    for (int k = 0; k <= 16; k++)
    {
        for (int l = 0; l <= 16; l++)
        {
            if (removepoints[k][l])
            {
                globalestuaries[sx][sy] = 1;
                
                int centrex = dx + k;
                int centrey = dy + l;

                int shapenumber = random(minsize, maxsize);

                int imheight = smalllake[shapenumber].ysize() - 1;
                int imwidth = smalllake[shapenumber].xsize() - 1;

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
                    jstart = imheight;
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

                            if (region.map(xx, yy) > sealevel && rmountainmap[xx][yy] == 0)
                            {
                                region.setmap(xx, yy, sealevel);
                                riverinlets[xx][yy] = 1; // Mark this as an estuary (even though it isn't) so it could get a beach later.
                            }
                        }
                    }
                }
            }
        }
    }

    // Older version of this function, now not used.

    /*
    int amount = 4; // Distance to look for mountains.

    // First, find the depth of the nearest sea.

    int minelev = world.maxelevation();

    for (int i = sx - 1; i <= sx + 1; i++)
    {
        int ii = i;

        if (ii<0 || ii>width)
            ii = wrap(ii, width);

        for (int j = sy - 1; j <= sy + 1; j++)
        {
            if (j >= 0 && j <= height)
            {
                if (world.nom(ii, j) < minelev)
                    minelev = world.nom(ii, j);
            }
        }
    }

    if (minelev > sealevel)
        return;

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (region.sea(i, j) == 0)
            {
                bool nearmountain = 0;

                for (int k = i - amount; k <= i + amount; k++)
                {
                    for (int l = j - amount; l <= j + amount; l++)
                    {
                        if (k >= 0 && k <= rwidth && l >= 0 && l <= rheight)
                        {
                            if (rmountainmap[k][l] != 0)
                            {
                                nearmountain = 1;
                                k = i + amount;
                                l = j + amount;
                            }
                        }
                    }
                }

                if (nearmountain == 0)
                {
                    region.setmap(i, j, minelev);
                }
            }
        }
    }
    */
}

// This removes any weirdly high elevation.

void removetoohighelevations(planet& world, region& region, int dx, int dy, int sx, int sy)
{
    int maxelev = world.maxelevation();

    int tileelev = world.map(sx, sy);

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (region.map(i, j) > maxelev)
                region.setmap(i, j, tileelev);
        }
    }

    if (world.lakesurface(sx, sy) == 0)
        return;

    if (world.riftlakesurface(sx, sy) != 0)
        return;

    // Now check for weird lake beds. (Again.)

    int surface = world.lakesurface(sx, sy);

    float depth = (float)(surface - tileelev);

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (region.lakesurface(i, j) != 0)
            {
                if (region.map(i, j) >= surface)
                    region.setmap(i, j, tileelev);
                else
                {
                    float thisdepth = (float)(surface - region.map(i, j));

                    if (thisdepth > depth * 1.2f)
                        region.setmap(i, j, tileelev);

                    if (region.map(i, j) < 10)
                        region.setmap(i, j, tileelev);
                }
            }
        }
    }
}

// This puts a single isolated peak onto the map.

void makevolcano(planet& world, region& region, int dx, int dy, int sx, int sy, peaktemplate& peaks, vector<vector<int>>& rmountainmap, vector<vector<int>>& ridgeids, int templateno)
{
    int width = world.width();
    int rheight = region.rheight();
    int rwidth = region.rwidth();

    bool strato = 0;

    if (world.strato(sx, sy) == 1)
        strato = 1;

    fast_srand((sy * width + sx) + world.map(sx, sy) + world.julrain(sx, sy));

    float peakheight = (float)world.volcano(sx, sy);

    bool extinct = 0;

    if (peakheight < 0.0f)
    {
        peakheight = 1.0f - peakheight;
        extinct = 1;
    }

    int x = dx + random(2, 15);
    int y = dy + random(2, 15);

    if (peakheight == 0.0f) // The central peaks of craters are treated like extinct volcanoes.
    {
        peakheight = (float)world.cratercentre(sx, sy);

        if (peakheight < 1.0f)
            return;

        if (random(1, (int)peakheight) > 4000) // The lower it is, the more likely it is to be like a stratovolcano.
            strato = 1;

        extinct = 1;

        x = dx + 8;
        y = dy + 8;
    }

    bool leftr = 0;
    bool downr = 0;

    if (random(1, 2) == 1)
        leftr = 1;

    if (random(1, 2) == 1)
        downr = 1;

    pastepeak(world, region, x, y, peakheight, templateno, leftr, downr, peaks, rmountainmap);

    if (strato == 0)
        ridgeids[x][y] = random(1, 8);

    if (extinct == 0)
        region.setvolcano(x, y, 1);

    if (strato == 0)
        return;

    // Stratovolcanoes may have subsidiary peaks.

    int maxsubdist = 7; // Max distance the subsidiary peaks may be.

    if (random(1, 2) != 1)
    {
        int extrapeaks = random(1, 4);

        for (int n = 1; n <= extrapeaks; n++)
        {
            int xx = x + randomsign(random(1, maxsubdist));
            int yy = y + randomsign(random(1, maxsubdist));

            if (xx >= 0 && xx <= rwidth && yy >= 0 && yy <= rheight)
            {
                float thispeakheight = peakheight;

                thispeakheight = thispeakheight / 100.0f;
                thispeakheight = thispeakheight * (float)random(50, 120);

                int thistemplateno = random(1, 2);

                bool thisleftr = 0;
                bool thisdownr = 0;

                if (random(1, 2) == 1)
                    thisleftr = 1;

                if (random(1, 2) == 1)
                    thisdownr = 1;

                pastepeak(world, region, x, y, thispeakheight, thistemplateno, thisleftr, thisdownr, peaks, rmountainmap);

                if (extinct == 0)
                    region.setvolcano(xx, yy, 1);
            }
        }
    }
}

// Same thing, but on the sea bed.

void makesubmarinevolcano(planet& world, region& region, int dx, int dy, int sx, int sy, peaktemplate& peaks, vector<vector<int>>& undersearidges, vector<vector<bool>>& volcanomap)
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    float peakheight = (float)world.volcano(sx, sy);

    int extrapeaks = random(2, 10); // Number of extra peaks to scatter nearby.
    int maxextradistance = 24;

    bool extinct = 0;

    if (peakheight < 0.0f)
    {
        peakheight = peakheight * -1.0f;
        extinct = 1;
    }

    int templateno = random(1, 2);

    int x = dx + random(2, 15);
    int y = dy + random(2, 15);

    bool volcano = 0;

    pastesubmarinepeak(world, region, x, y, peakheight, templateno, peaks, undersearidges, rwidth, rheight);

    if (extinct == 0 && x >= 0 && x <= rwidth && y >= 0 && y <= rheight)
        volcanomap[x][y] = 1;

    for (int n = 1; n <= extrapeaks; n++)
    {
        int thisx = x + randomsign(random(1, maxextradistance));
        int thisy = y + randomsign(random(1, maxextradistance));

        float thispeakheight = (float)random((int)(peakheight / 4.0f), (int)((peakheight / 4.0f) * 3.0f));

        templateno = random(1, 2);

        if (thisx >= 0 && thisx <= rwidth && thisy >= 0 && thisy <= rheight)
            pastesubmarinepeak(world, region, thisx, thisy, thispeakheight, templateno, peaks, undersearidges, rwidth, rheight);
    }
}

// This goes through the regional map and tries to fill in any missing river sections.

void fixrivers(planet& world, region& region, int leftx, int lefty, int rightx, int righty)
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    int minload = 80; // minimum size of river to worry about

    for (int i = leftx; i <= rightx; i++)
    {
        for (int j = lefty; j <= righty; j++)
        {
            if (region.riverdir(i, j) != 0)
            {
                int dir = region.riverdir(i, j);
                int janload = region.riverjan(i, j);
                int julload = region.riverjul(i, j);

                if (janload > minload || julload > minload)
                {
                    int minjanload = janload / 2;
                    int minjulload = julload / 2; // If the flow drops by more than this, it's a problem.

                    twointegers dest = getdestination(i, j, dir);

                    if (dest.x >= leftx && dest.x <= rightx && dest.y >= lefty && dest.y <= righty)
                    {
                        if (region.sea(dest.x, dest.y) == 0)
                        {
                            if (region.riverjan(dest.x, dest.y) < minjanload || region.riverjul(dest.x, dest.y) < minjulload) // If the destination cell has less flow than the current one!
                            {
                                vector<vector<bool>> checked(rwidth + 1, vector<bool>(rheight + 1, 0));

                                /*
                                for (int k=0; k<=rwidth; k++)
                                {
                                    for (int l=0; l<=rwidth; l++)
                                        checked[k][l]=0;
                                }
                                */

                                int newdestx = -1;
                                int newdesty = -1;

                                int maxradius = 20;

                                for (int radius = 2; radius <= maxradius; radius++)
                                {
                                    // This bit is to restrict our search to roughly the direction in which the river is pointing.

                                    int kstart = 0 - radius;
                                    int kend = radius;
                                    int lstart = 0 - radius;
                                    int lend = radius;

                                    if (dir == 2 || dir == 3 || dir == 4)
                                        kstart = 0;

                                    if (dir == 6 || dir == 7 || dir == 8)
                                        kend = 0;

                                    if (dir == 8 || dir == 1 || dir == 2)
                                        lend = 0;

                                    if (dir == 4 || dir == 5 || dir == 6)
                                        lstart = 0;

                                    for (int k = kstart; k <= kend; k++)
                                    {
                                        int kk = i + k;

                                        for (int l = lstart; l <= lend; l++)
                                        {
                                            int ll = j + l;

                                            if ((k != 0 || l != 0) && k * k + l * l <= radius * radius + radius && kk >= leftx && kk <= rightx && ll >= lefty && ll <= righty && checked[kk][ll] == 0)
                                            {
                                                checked[kk][ll] = 1;

                                                int destjan = region.riverjan(kk, ll);
                                                int destjul = region.riverjul(kk, ll);

                                                if (destjan >= janload && destjul >= julload) // This cell has a big enough river going through it
                                                {
                                                    twointegers thisload = gettotalinflow(region, kk, ll);

                                                    if (thisload.x < destjan || thisload.y < destjul) // This cell isn't getting all the flow it should do.
                                                    {
                                                        newdestx = kk;
                                                        newdesty = ll;

                                                        k = kend;
                                                        l = lend;
                                                        radius = maxradius;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }

                                if (newdestx == -1) // If that didn't work, widen the search.
                                {
                                    for (int radius = 2; radius <= maxradius; radius++)
                                    {
                                        int kstart = 0 - radius;
                                        int kend = radius;

                                        if (dir == 2 || dir == 3 || dir == 4)
                                            kstart = 0;

                                        if (dir == 6 || dir == 7 || dir == 8)
                                            kend = 0;

                                        for (int k = kstart; k <= kend; k++)
                                        {
                                            int kk = i + k;

                                            for (int l = 0 - radius; l <= radius; l++)
                                            {
                                                int ll = j + l;

                                                if ((k != 0 || l != 0) && k * k + l * l <= radius * radius + radius && kk >= leftx && kk <= rightx && ll >= lefty && ll <= righty && checked[kk][ll] == 0)
                                                {
                                                    checked[kk][ll] = 1;

                                                    int destjan = region.riverjan(kk, ll);
                                                    int destjul = region.riverjul(kk, ll);

                                                    if (destjan >= janload && destjul >= julload) // This cell has a big enough river going through it
                                                    {
                                                        twointegers thisload = gettotalinflow(region, kk, ll);

                                                        if (thisload.x < destjan || thisload.y < destjul) // This cell isn't getting all the flow it should do.
                                                        {
                                                            newdestx = kk;
                                                            newdesty = ll;

                                                            k = kend;
                                                            l = radius;
                                                            radius = maxradius;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }

                                if (newdestx == -1) // If that didn't work, widen the search the other way.
                                {
                                    for (int radius = 2; radius <= maxradius; radius++)
                                    {
                                        int lstart = 0 - radius;
                                        int lend = radius;

                                        if (dir == 8 || dir == 1 || dir == 2)
                                            lend = 0;

                                        if (dir == 4 || dir == 5 || dir == 6)
                                            lstart = 0;

                                        for (int k = 0 - radius; k <= radius; k++)
                                        {
                                            int kk = i + k;

                                            for (int l = lstart; l <= lend; l++)
                                            {
                                                int ll = j + l;

                                                if ((k != 0 || l != 0) && k * k + l * l <= radius * radius + radius && kk >= leftx && kk <= rightx && ll >= lefty && ll <= righty && checked[kk][ll] == 0)
                                                {
                                                    checked[kk][ll] = 1;

                                                    int destjan = region.riverjan(kk, ll);
                                                    int destjul = region.riverjul(kk, ll);

                                                    if (destjan >= janload && destjul >= julload) // This cell has a big enough river going through it
                                                    {
                                                        twointegers thisload = gettotalinflow(region, kk, ll);

                                                        if (thisload.x < destjan || thisload.y < destjul) // This cell isn't getting all the flow it should do.
                                                        {
                                                            newdestx = kk;
                                                            newdesty = ll;

                                                            k = radius;
                                                            l = lend;
                                                            radius = maxradius;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }

                                if (newdestx == -1) // If that didn't work, look in all directions.
                                {
                                    for (int radius = 2; radius <= maxradius; radius++)
                                    {
                                        for (int k = 0 - radius; k <= radius; k++)
                                        {
                                            int kk = i + k;

                                            for (int l = 0 - radius; l <= radius; l++)
                                            {
                                                int ll = j + l;

                                                if ((k != 0 || l != 0) && k * k + l * l <= radius * radius + radius && kk >= leftx && kk <= rightx && ll >= lefty && ll <= righty && checked[kk][ll] == 0)
                                                {
                                                    checked[kk][ll] = 1;

                                                    int destjan = region.riverjan(kk, ll);
                                                    int destjul = region.riverjul(kk, ll);

                                                    if (destjan >= janload && destjul >= julload) // This cell has a big enough river going through it
                                                    {
                                                        twointegers thisload = gettotalinflow(region, kk, ll);

                                                        if (thisload.x < destjan || thisload.y < destjul) // This cell isn't getting all the flow it should do.
                                                        {
                                                            newdestx = kk;
                                                            newdesty = ll;

                                                            k = radius;
                                                            l = radius;
                                                            radius = maxradius;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }

                                if (newdestx != -1) // We found a destination to go to! Now we just draw river from the starting point to this one.
                                {
                                    int newelev = region.map(newdestx, newdesty);

                                    twofloats mm1, mm2, mm3, pt;

                                    mm1.x = (float) i;
                                    mm1.y = (float) j;

                                    mm3.x = (float) newdestx;
                                    mm3.y = (float) newdesty;

                                    float rightshift = mm3.x - mm1.x;
                                    float downshift = mm3.y - mm1.y;

                                    if (rightshift < 0.0f)
                                        rightshift = 0.0f - rightshift;

                                    if (downshift < 0.0f)
                                        downshift = 0.0f - downshift;

                                    rightshift = rightshift / 4.0f;
                                    if (rightshift < 2.0f)
                                        rightshift = 2.0f;

                                    downshift = downshift / 4.0f;
                                    if (downshift < 2.0f)
                                        downshift = 2.0f;

                                    mm2.x = (mm1.x + mm3.x) / 2.0f + (float)randomsign(random(1, (int)rightshift));
                                    mm2.y = (mm1.y + mm3.y) / 2.0f + (float)randomsign(random(1, (int)downshift));

                                    int lastx = (int)mm1.x;
                                    int lasty = (int)mm1.y; // Coordinates of last point done.

                                    for (int n = 1; n <= 2; n++) // Two halves of the curve.
                                    {
                                        for (float t = 0.0f; t <= 1.0f; t = t + 0.01f)
                                        {
                                            if (n == 1)
                                                pt = curvepos(mm1, mm1, mm2, mm3, t);
                                            else
                                                pt = curvepos(mm1, mm2, mm3, mm3, t);

                                            int x = (int)pt.x;
                                            int y = (int)pt.y;

                                            if ((x != lastx || y != lasty) && (x != mm1.x || y != mm2.y) && x >= leftx && x <= rightx && y >= lefty && y <= righty)
                                            {
                                                int dir = getdir(lastx, lasty, x, y);

                                                region.setriverdir(lastx, lasty, dir);

                                                if (x != mm3.x || y != mm3.y)
                                                {
                                                    if (region.riverjan(x, y) < janload && region.riverjul(x, y) < julload)
                                                    {
                                                        region.setriverjan(x, y, region.riverjan(x, y) + janload);
                                                        region.setriverjul(x, y, region.riverjul(x, y) + julload);
                                                    }
                                                    region.setmap(x, y, newelev);
                                                }
                                                lastx = x;
                                                lasty = y;
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

// This notes down too-long diagonals on the coastlines.

void findcoastdiagonals(planet& world, region& region, int dx, int dy, int sx, int sy, vector<vector<bool>>& disruptpoints)
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    for (int i = dx; i <= dx + 16; i++)
    {
        int iright = i + 1;
        int ileft = i - 1;

        int iright2 = i + 2;

        if (iright2 <= rwidth && ileft >= 0)
        {
            for (int j = dy; j <= dy + 16; j++)
            {
                int jup = j - 1;
                int jdown = j + 1;

                int jup2 = j - 2;
                int jdown2 = j + 2;

                if (jup2 <= rheight && jdown2 >= 0)
                {
                    if (region.sea(i, j) == 0)
                    {
                        if (northwestlandonly(region, i, j) == 1)
                        {
                            if (northwestlandonly(region, iright, jup) == 1 && northwestlandonly(region, ileft, jdown) == 1 && northwestlandonly(region, iright2, jup2) == 1)
                            {
                                if (disruptpoints[iright][jup] == 0 && disruptpoints[ileft][jdown] == 0)
                                    disruptpoints[i][j] = 1;
                            }
                        }

                        if (northeastlandonly(region, i, j) == 1)
                        {
                            if (northeastlandonly(region, ileft, jup) == 1 && northeastlandonly(region, iright, jdown) == 1 && northeastlandonly(region, iright2, jdown2) == 1)
                            {
                                if (disruptpoints[ileft][jup] == 0 && disruptpoints[iright][jdown] == 0)
                                    disruptpoints[i][j] = 1;
                            }
                        }

                        if (southwestlandonly(region, i, j) == 1)
                        {
                            if (southwestlandonly(region, ileft, jup) == 1 && southwestlandonly(region, iright, jdown) == 1 && southwestlandonly(region, iright2, jdown2) == 1)
                            {
                                if (disruptpoints[ileft][jup] == 0 && disruptpoints[iright][jdown] == 0)
                                    disruptpoints[i][j] = 1;
                            }
                        }

                        if (southeastlandonly(region, i, j) == 1)
                        {
                            if (southeastlandonly(region, iright, jup) == 1 && southeastlandonly(region, ileft, jdown) == 1 && southeastlandonly(region, iright2, jup2) == 1)
                            {
                                if (disruptpoints[iright][jup] == 0 && disruptpoints[ileft][jdown] == 0)
                                    disruptpoints[i][j] = 1;
                            }
                        }
                    }
                }
            }
        }
    }
}

// This removes the diagonals.

void removecoastdiagonals(planet& world, region& region, int dx, int dy, int sx, int sy, vector<vector<bool>>& disruptpoints, boolshapetemplate smalllake[])
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    int maxsize = 3; // Biggest template to use.

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (disruptpoints[i][j] == 1)
            {
                int crount = 0;
                int avedepth = 0;

                for (int k = i - 1; k <= i + 1; k++)
                {
                    for (int l = j - 1; l <= j + 1; l++)
                    {
                        if (k >= 0 && k <= rwidth && l >= 0 && l <= rheight)
                        {
                            if (region.sea(k, l) == 1)
                            {
                                avedepth = avedepth + region.map(k, l);
                                crount++;
                            }
                        }
                    }
                }

                if (crount > 0)
                    avedepth = avedepth / crount;

                disruptseacoastline(world, region, dx, dy, i, j, avedepth, 0, maxsize, 0, smalllake);
            }
        }
    }
}

// This notes down too-long diagonals on the lake coastlines.

void findlakecoastdiagonals(planet& world, region& region, int dx, int dy, int sx, int sy, vector<vector<bool>>& disruptpoints)
{
    int width = world.width();
    int height = world.height();

    int rwidth = region.rwidth();
    int rheight = region.rheight();

    bool nearlake = 0;

    for (int i = sx - 1; i <= sx + 1; i++)
    {
        int ii = i;

        if (ii<0 || ii>width)
            ii = wrap(ii, width);

        for (int j = sy - 1; j <= sy + 1; j++)
        {
            if (j >= 0 && j <= height)
            {
                if (world.lakesurface(i, j) != 0)
                {
                    nearlake = 1;
                    i = sx + 1;
                    j = sy + 1;
                }
            }
        }
    }

    if (nearlake == 0)
        return;

    for (int i = dx; i <= dx + 16; i++)
    {
        int iright = i + 1;
        int ileft = i - 1;

        int iright2 = i + 2;

        if (iright2 <= rwidth && ileft >= 0)
        {
            for (int j = dy; j <= dy + 16; j++)
            {
                int jup = j - 1;
                int jdown = j + 1;

                int jup2 = j - 2;
                int jdown2 = j + 2;

                if (jup2 <= rheight && jdown2 >= 0)
                {
                    if (region.lakesurface(i, j) == 0)
                    {
                        if (lakenorthwestlandonly(region, i, j) == 1)
                        {
                            if (lakenorthwestlandonly(region, iright, jup) == 1 && lakenorthwestlandonly(region, ileft, jdown) == 1 && lakenorthwestlandonly(region, iright2, jup2) == 1)
                            {
                                if (disruptpoints[iright][jup] == 0 && disruptpoints[ileft][jdown] == 0)
                                    disruptpoints[i][j] = 1;
                            }
                        }

                        if (lakenortheastlandonly(region, i, j) == 1)
                        {
                            if (lakenortheastlandonly(region, ileft, jup) == 1 && lakenortheastlandonly(region, iright, jdown) == 1 && lakenortheastlandonly(region, iright2, jdown2) == 1)
                            {
                                if (disruptpoints[ileft][jup] == 0 && disruptpoints[iright][jdown] == 0)
                                    disruptpoints[i][j] = 1;
                            }
                        }

                        if (lakesouthwestlandonly(region, i, j) == 1)
                        {
                            if (lakesouthwestlandonly(region, ileft, jup) == 1 && lakesouthwestlandonly(region, iright, jdown) == 1 && lakesouthwestlandonly(region, iright2, jdown2) == 1)
                            {
                                if (disruptpoints[ileft][jup] == 0 && disruptpoints[iright][jdown] == 0)
                                    disruptpoints[i][j] = 1;
                            }
                        }

                        if (lakesoutheastlandonly(region, i, j) == 1)
                        {
                            if (lakesoutheastlandonly(region, iright, jup) == 1 && lakesoutheastlandonly(region, ileft, jdown) == 1 && lakesoutheastlandonly(region, iright2, jup2) == 1)
                            {
                                if (disruptpoints[iright][jup] == 0 && disruptpoints[ileft][jdown] == 0)
                                    disruptpoints[i][j] = 1;
                            }
                        }
                    }
                }
            }
        }
    }
}

// This removes the diagonals.

void removelakecoastdiagonals(planet& world, region& region, int dx, int dy, int sx, int sy, vector<vector<bool>>& disruptpoints, boolshapetemplate smalllake[])
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    int maxsize = 6; // Biggest template to use.

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (disruptpoints[i][j] == 1)
            {
                int crount = 0;
                int avedepth = 0;

                int surfacelevel = 0;
                int special = 0;

                for (int k = i - 1; k <= i + 1; k++)
                {
                    for (int l = j - 1; l <= j + 1; l++)
                    {
                        if (k >= 0 && k <= rwidth && l >= 0 && l <= rheight)
                        {
                            if (region.lakesurface(k, l) != 0)
                            {
                                surfacelevel = region.lakesurface(k, l);
                                special = region.special(k, l);
                                avedepth = avedepth + region.map(k, l);
                                crount++;
                            }
                        }
                    }
                }

                if (crount > 0)
                    avedepth = avedepth / crount;

                bool raise = 0;

                if (random(1, 2) == 1)
                    raise = 1;

                disruptlakecoastline(world, region, dx, dy, i, j, surfacelevel, avedepth, raise, maxsize, 0, special, smalllake);
            }
        }
    }
}

// This adds rotations to the terrain at the northern and western edges of each tile.

void rotatetileedges(planet& world, region& region, int dx, int dy, int sx, int sy, vector<vector<int>>& rotatearray, bool lakes)
{
    int width = world.width();
    int height = world.height();
    int maxelev = world.maxelevation();

    int rwidth = region.rwidth();
    int rheight = region.rheight();

    float radius = 16.0f; // The radius of the circle that will be rotated.

    fast_srand((sy * width + sx) + world.map(sx, sy) + world.julrain(sx, sy));

    int x, y;

    int nox = sx;
    int noy = sy - 1;

    if (noy < 0)
        noy = 0;

    int wex = sx - 1;

    if (wex < 0)
        wex = width;

    int wey = sy;

    for (int n = 0; n < 1; n++)
    {
        bool goahead = 1;

        if (n == 0)
        {
            if (world.sea(wex, wey) == 1)
                goahead = 0;
            else
            {
                x = dx + randomsign(random(1, 4));
                y = dy + random(4, 12);
            }
        }
        else
        {
            if (world.sea(nox, noy) == 1)
                goahead = 0;
            else
            {
                x = dx + random(4, 12);
                y = dy + randomsign(random(1, 4));
            }
        }

        if (goahead == 1)
        {
            int angle;

            if (random(1, 2) == 1)
                angle = random(10, 80);
            else
                angle = random(280, 350);

            if (lakes == 1)
                rotatelakes(world, region, x, y, (int)radius, (float)angle, rotatearray);
            else
                rotateland(world, region, x, y, (int)radius, (float)angle, rotatearray);
        }
    }
}

// This rotates an area of land elevation and blends it in, to remove grid artefacts.

void rotateland(planet& world, region& region, int centrex, int centrey, int maxradius, float angle, vector<vector<int>>& rotatearray)
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    int squaremax = maxradius * maxradius + maxradius;
    int smallsquaremax = (maxradius - 2) * (maxradius - 2) + maxradius - 2;

    // First, clear this section of the newelevation array. We'll put the new elevation values in this array and then copy them onto the elevation map.

    for (int x = centrex - maxradius; x <= centrex + maxradius; x++)
    {
        if (x >= 0 && x <= rwidth)
        {
            for (int y = centrey - maxradius; y <= centrey + maxradius; y++)
            {
                if (y >= 0 && y <= rheight)
                    rotatearray[x][y] = 0;
            }
        }
    }

    // Now do the rotation, and put the new values onto that array.

    for (float x = (float) - maxradius; x <= (float)maxradius; x++)
    {
        int xx = centrex + (int)x;

        if (xx >= 0 && xx <= rwidth)
        {
            for (float y = (float) - maxradius; y <= (float)maxradius; y++)
            {
                int yy = centrey + (int)y;

                if (yy >= 0 && yy <= rheight)
                {
                    bool goahead = 0;

                    if (region.sea(xx, yy) == 0 && region.riverdir(xx, yy) == 0 && region.fakedir(xx, yy) == 0 && region.lakesurface(xx, yy) == 0 && x * x + y * y < squaremax)
                    {
                        bool nearbyrivers = 0;

                        for (int i = xx - 1; i <= xx + 1; i++)
                        {
                            for (int j = yy - 1; j <= yy + 1; j++)
                            {
                                if (i >= 0 && i <= rwidth && j >= 0 && j <= rheight)
                                {
                                    if (region.riverdir(i, j) != 0 || region.fakedir(i, j) != 0 || region.lakesurface(i, j) != 0)
                                    {
                                        nearbyrivers = 1;
                                        i = xx + 1;
                                        j = yy + 1;
                                    }
                                }
                            }
                        }

                        if (nearbyrivers == 0)
                        {
                            float oldelev = (float)region.map(xx, yy);

                            int newx = (int)((float)x * (float)cos(angle) - (float)y * (float)sin(angle));
                            int newy = (int)((float)y * (float)cos(angle) + (float)x * (float)sin(angle));

                            newx = newx + centrex;
                            newy = newy + centrey;

                            if (newx >= 0 && newx <= rwidth && newy >= 0 && newy <= rheight && region.sea(newx, newy) == 0 && region.riverdir(newx, newy) == 0 && region.fakedir(newx, newy) == 0 && region.lakesurface(newx, newy) == 0)
                            {
                                float rotelev = (float)region.map(newx, newy);

                                float radius = (float)sqrt(x * x + y * y); // This is the current radius from the centre point.

                                float rotproportion = radius / (float)maxradius;
                                float oldproportion = ((float)maxradius - radius) / (float)maxradius;

                                float newelev = oldelev * oldproportion + rotelev * rotproportion;

                                rotatearray[xx][yy] = (int)newelev;
                            }
                        }
                    }
                }
            }
        }
    }

    // Now copy the new values from the array onto the elevation map.

    for (int x = centrex - maxradius; x <= centrex + maxradius; x++)
    {
        if (x >= 0 && x <= rwidth)
        {
            for (int y = centrey - maxradius; y <= centrey + maxradius; y++)
            {
                if (y >= 0 && y <= rheight && rotatearray[x][y] != 0)
                    region.setmap(x, y, rotatearray[x][y]);
            }
        }
    }

    // Now smooth the edges of the circle.

    int amount = 1; // Size of the blurring.

    for (int x = -maxradius; x <= maxradius; x++)
    {
        int xx = centrex + x;

        if (xx >= 0 && xx <= rwidth)
        {
            for (int y = -maxradius; y <= maxradius; y++)
            {
                int yy = centrey + y;

                if (yy >= 0 && yy <= rheight)
                {
                    bool goahead = 0;

                    if (region.sea(xx, yy) == 0 && region.riverdir(xx, yy) == 0 && region.fakedir(xx, yy) == 0 && region.lakesurface(xx, yy) == 0)
                    {
                        int thissquare = x * x + y * y;

                        if (thissquare<squaremax && thissquare>smallsquaremax)
                        {
                            float crount = 0;
                            float total = 0;

                            for (int i = xx - amount; i <= xx + amount; i++)
                            {
                                if (i >= 0 && i <= rwidth)
                                {
                                    for (int j = yy - amount; j <= yy + amount; j++)
                                    {
                                        if (j >= 0 && j <= rheight)
                                        {
                                            if (region.sea(i, j) == 0 && region.riverdir(i, j) == 0 && region.fakedir(i, j) == 0 && region.lakesurface(i, j) == 0)
                                            {
                                                total = total + region.map(i, j);
                                                crount++;
                                            }
                                        }
                                    }
                                }
                            }

                            total = total / crount;

                            region.setmap(xx, yy, (int)total);
                        }
                    }
                }
            }
        }
    }
}

// The same, but lake beds.

void rotatelakes(planet& world, region& region, int centrex, int centrey, int maxradius, float angle, vector<vector<int>>& rotatearray)
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    // First, clear this section of the newelevation array. We'll put the new elevation values in this array and then copy them onto the elevation map.

    for (int x = centrex - maxradius; x <= centrex + maxradius; x++)
    {
        if (x >= 0 && x <= rwidth)
        {
            for (int y = centrey - maxradius; y <= centrey + maxradius; y++)
            {
                if (y >= 0 && y <= rheight)
                    rotatearray[x][y] = 0;
            }
        }
    }

    // Now do the rotation, and put the new values onto that array.

    for (float x = 0.0f-(float)maxradius; x <= (float)maxradius; x++)
    {
        int xx = centrex + (int)x;

        if (xx >= 0 && xx <= rwidth)
        {
            for (float y = 0.0f-(float)maxradius; y <= (float)maxradius; y++)
            {
                int yy = centrey + (int)y;

                if (yy >= 0 && yy <= rheight)
                {
                    bool goahead = 0;

                    if (region.sea(xx, yy) == 0 && region.lakesurface(xx, yy) != 0 && (int)(x * x + y * y) < maxradius * maxradius + maxradius)
                    {
                        float oldelev = (float)region.map(xx, yy);

                        int newx = (int)(x * (float)cos(angle) - y * (float)sin(angle));
                        int newy = (int)(y * (float)cos(angle) + x * (float)sin(angle));

                        newx = newx + centrex;
                        newy = newy + centrey;

                        if (newx >= 0 && newx <= rwidth && newy >= 0 && newy <= rheight && region.sea(newx, newy) == 0 && region.lakesurface(newx, newy) != 0)
                        {
                            float rotelev = (float)region.map(newx, newy);

                            float radius = (float)sqrt(x * x + y * y); // This is the current radius from the centre point.

                            float rotproportion = radius / (float)maxradius;
                            float oldproportion = ((float)maxradius - radius) / (float)maxradius;

                            float newelev = oldelev * oldproportion + rotelev * rotproportion;

                            rotatearray[xx][yy] = (int)newelev;
                        }
                    }
                }
            }
        }
    }

    // Now copy the new values from the array onto the elevation map.

    for (int x = centrex - maxradius; x <= centrex + maxradius; x++)
    {
        if (x >= 0 && x <= rwidth)
        {
            for (int y = centrey - maxradius; y <= centrey + maxradius; y++)
            {
                if (y >= 0 && y <= rheight && rotatearray[x][y] != 0)
                    region.setmap(x, y, rotatearray[x][y]);
            }
        }
    }
}

// This adds rotations to a generic array at the northern and western edges of each tile.

void rotatetileedgesarray(planet& world, region& region, int dx, int dy, int sx, int sy, vector<vector<int>>& destarray, vector<vector<int>>& rotatearray, int min)
{
    int width = world.width();
    int height = world.height();
    int maxelev = world.maxelevation();

    int rwidth = region.rwidth();
    int rheight = region.rheight();

    int radius = 16; // The radius of the circle that will be rotated.

    fast_srand((sy * width + sx) + world.map(sx, sy) + world.julrain(sx, sy));

    int x, y;

    int nox = sx;
    int noy = sy - 1;

    if (noy < 0)
        noy = 0;

    int wex = sx - 1;

    if (wex < 0)
        wex = width;

    int wey = sy;

    for (int n = 0; n < 1; n++)
    {
        if (n == 0)
        {
            x = dx + randomsign(random(1, 4));
            y = dy + random(4, 12);
        }
        else
        {
            x = dx + random(4, 12);
            y = dy + randomsign(random(1, 4));
        }

        int angle;

        if (random(1, 2) == 1)
            angle = random(10, 80);
        else
            angle = random(280, 350);

        rotatelandarray(world, region, x, y, radius, (float)angle, destarray, rotatearray, min);
    }
}

void rotatelandarray(planet& world, region& region, int centrex, int centrey, int maxradius, float angle, vector<vector<int>>& destarray, vector<vector<int>>& rotatearray, int min)
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    // First, clear this section of the newelevation array. We'll put the new elevation values in this array and then copy them onto the elevation map.

    for (int x = centrex - maxradius; x <= centrex + maxradius; x++)
    {
        if (x >= 0 && x <= rwidth)
        {
            for (int y = centrey - maxradius; y <= centrey + maxradius; y++)
            {
                if (y >= 0 && y <= rheight)
                    rotatearray[x][y] = 0;
            }
        }
    }

    // Now do the rotation, and put the new values onto that array.

    for (float x = 0.0f-(float)maxradius; x <= (float)maxradius; x++)
    {
        int xx = centrex + (int)x;

        if (xx >= 0 && xx <= rwidth)
        {
            for (float y = 0.0f-(float)maxradius; y <= (float)maxradius; y++)
            {
                int yy = centrey + (int)y;

                if (yy >= 0 && yy <= rheight && destarray[xx][yy] != min)
                {
                    if (x * x + y * y < (float)(maxradius * maxradius + maxradius))
                    {
                        float oldval = (float)destarray[xx][yy];

                        int newx = (int)(x * (float)cos(angle) - y * (float)sin(angle));
                        int newy = (int)(y * (float)cos(angle) + x * (float)sin(angle));

                        newx = newx + centrex;
                        newy = newy + centrey;

                        if (newx >= 0 && newx <= rwidth && newy >= 0 && newy <= rheight && destarray[newx][newy] != min)
                        {
                            float rotval = (float)destarray[newx][newy];

                            float radius = (float)sqrt(x * x + y * y); // This is the current radius from the centre point.

                            float rotproportion = radius / (float)maxradius;
                            float oldproportion = ((float)maxradius - radius) / (float)maxradius;

                            float newval = oldval * oldproportion + rotval * rotproportion;

                            rotatearray[xx][yy] = (int)newval;
                        }
                    }
                }
            }
        }
    }

    // Now copy the new values from the rotate array onto the destination array.

    for (int x = centrex - maxradius; x <= centrex + maxradius; x++)
    {
        if (x >= 0 && x <= rwidth)
        {
            for (int y = centrey - maxradius; y <= centrey + maxradius; y++)
            {
                if (y >= 0 && y <= rheight && rotatearray[x][y] != 0)
                    destarray[x][y] = rotatearray[x][y];
            }
        }
    }
}

// This removes small islands from a sea tile.

void removesmallislands(planet& world, region& region, int dx, int dy, int sx, int sy)
{
    if (world.sea(sx, sy) == 0)
        return;

    int width = world.width();
    int height = world.height();
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    for (int i = sx - 1; i <= sx + 1; i++)
    {
        int ii = i;

        if (ii<0 || ii>width)
            ii = wrap(ii, width);

        for (int j = sy - 1; j <= sy + 1; j++)
        {
            if (j >= 0 && j <= height)
            {
                if (world.island(ii, j) == 1 || world.mountainisland(ii, j) == 1)
                    return;
            }
        }
    }

    int sealevel = world.sealevel();
    int margin = 8; // The margin around the current tile to check

    for (int i = dx - margin; i <= dx + 16 + margin; i++)
    {
        if (i >= 0 && i <= rwidth && region.map(i, dy - margin) > sealevel)
            return;
    }

    for (int i = dx - margin; i <= dx + 16 + margin; i++)
    {
        if (i >= 0 && i <= rwidth && region.map(i, dy + 16 + margin) > sealevel)
            return;
    }

    for (int j = dy - margin; j <= dy + 16 + margin; j++)
    {
        if (j >= 0 && j <= rheight && region.map(dx - margin, j) > sealevel)
            return;
    }

    for (int j = dy - margin; j <= dy + 16 + margin; j++)
    {
        if (j >= 0 && j <= rheight && region.map(dx + 16 + margin, j) > sealevel)
            return;
    }

    for (int i = dx - margin + 1; i < dx + 16 + margin; i++)
    {
        for (int j = dy - margin + 1; j < dy + 16 + margin; j++)
        {
            if (i >= 0 && i <= rwidth && j >= 0 && j <= rheight && region.map(i, j) > sealevel)
                region.setmap(i, j, sealevel - 100);
        }
    }
}

// This removes odd-looking little parallel lines of islands that sometimes appear.

void removeparallelislands(region& region, int leftx, int lefty, int rightx, int righty, int sealevel)
{
    for (int i = leftx; i <= rightx; i++)
    {
        for (int j = lefty; j <= righty - 5; j++)
        {
            if (region.sea(i, j) == 1 && region.sea(i, j + 1) == 0 && region.sea(i, j + 2) == 1 && region.sea(i, j + 3) == 0 && region.sea(i, j + 4) == 1)
            {
                int newval = region.map(i, j);

                if (region.map(i, j + 2) < newval)
                    newval = region.map(i, j + 2);

                if (region.map(i, j + 4) < newval)
                    newval = region.map(i, j + 4);

                for (int n = 0; n <= 4; n++)
                    region.setmap(i, j + n, newval);

                if (region.sea(i, j + 5) == 0)
                    region.setmap(i, j + 5, newval);
            }
        }
    }

    for (int i = leftx; i <= rightx - 5; i++)
    {
        for (int j = lefty; j <= righty; j++)
        {
            if (region.sea(i, j) == 1 && region.sea(i + 1, j) == 0 && region.sea(i + 2, j) == 1 && region.sea(i + 3, j) == 0 && region.sea(i + 4, j) == 1)
            {
                int newval = region.map(i, j);

                if (region.map(i + 2, j) < newval)
                    newval = region.map(i + 2, j);

                if (region.map(i + 4, j) < newval)
                    newval = region.map(i + 2, j);

                for (int n = 0; n <= 4; n++)
                    region.setmap(i + n, j, newval);

                if (region.sea(i + 5, j) == 0)
                    region.setmap(i + 5, j, newval);
            }
        }
    }
}

// This removes glitches from the sea ice.

void removeseaiceglitches(region& region)
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    for (int n = 0; n < 2; n++)
    {
        for (int i = 1; i < rwidth; i++)
        {
            for (int j = 1; j < rheight; j++)
            {
                int thisval = region.seaice(i, j);

                int leftval = region.seaice(i - 1, j);
                int rightval = region.seaice(i + 1, j);
                int upval = region.seaice(i, j - 1);
                int downval = region.seaice(i, j + 1);

                if (leftval == rightval && thisval != leftval)
                    region.setseaice(i, j, leftval);

                if (upval == downval && thisval != upval)
                    region.setseaice(i, j, upval);
            }
        }
    }
}

// This smoothes the lake beds.

void smoothlakebeds(region& region)
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    for (int n = 0; n < 2; n++)
    {
        for (int i = 1; i < rwidth; i++)
        {
            for (int j = 1; j < rheight; j++)
            {
                if (region.truelake(i, j) == 1)
                {
                    int thisval = region.map(i, j);

                    int leftval = region.map(i - 1, j);
                    int rightval = region.map(i + 1, j);
                    int upval = region.map(i, j - 1);
                    int downval = region.map(i, j + 1);

                    if (region.truelake(i - 1, j) == 1 && region.truelake(i + 1, j) == 1 && leftval < thisval && rightval < thisval)
                        region.setmap(i, j, leftval);

                    if (region.truelake(i - 1, j) == 1 && region.truelake(i + 1, j) == 1 && leftval > thisval && rightval > thisval)
                        region.setmap(i, j, leftval);

                    if (region.truelake(i, j - 1) == 1 && region.truelake(i, j + 1) == 1 && upval < thisval && downval < thisval)
                        region.setmap(i, j, upval);

                    if (region.truelake(i, j - 1) == 1 && region.truelake(i, j + 1) == 1 && upval > thisval && downval > thisval)
                        region.setmap(i, j, upval);
                }
            }
        }
    }
}

// This removes too-low areas of elevation (occasionally can occur in lakes).

void removetoolow(planet& world, region& region, int dx, int dy, int sx, int sy)
{
    int maxelev = world.maxelevation();
    int minelev = 3;

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (region.map(i, j) < minelev || region.map(i, j) > maxelev)
            {
                region.setmap(i, j, world.nom(sx, sy));
            }
        }
    }
}

// This removes any sea cells that may appear in lakes.

void removelakeseas(planet& world, region& region, int dx, int dy, int sx, int sy)
{
    if (world.sea(sx, sy) == 1)
        return;

    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();

    int surface = 0;

    int checkdist = 1;

    for (int i = sx - checkdist; i <= sx + checkdist; i++)
    {
        int ii = i;

        if (ii<0 || ii>width)
            wrap(ii, width);

        for (int j = sy - checkdist; j <= sy + checkdist; j++)
        {
            if (j >= 0 && j <= height)
            {
                if (world.sea(i, j) == 1)
                    return;
            }
        }
    }

    for (int i = sx - checkdist; i <= sx + checkdist; i++)
    {
        int ii = i;

        if (ii<0 || ii>width)
            wrap(ii, width);

        for (int j = sy - checkdist; j <= sy + checkdist; j++)
        {
            if (j >= 0 && j <= height && world.lakesurface(i, j) != 0)
            {
                surface = world.lakesurface(i, j);
                i = sx + checkdist;
                j = sy + checkdist;
            }
        }
    }

    if (surface == 0)
        return;

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (region.map(i, j) <= sealevel && region.lakesurface(i, j) != surface)
                region.setlakesurface(i, j, surface);
        }
    }
}

// This makes sure that all lake beds are below the surface of the lake.

void checklakebeds(region& region, int leftx, int lefty, int rightx, int righty)
{
    for (int i = leftx; i <= rightx; i++)
    {
        for (int j = lefty; j <= righty; j++)
        {
            int surface = region.lakesurface(i, j);

            if (surface != 0)
            {
                int elev = region.map(i, j);

                if (elev >= surface)
                    region.setmap(i, j, surface - 1);
            }

            int special = region.special(i, j);

            if ((special == 110 || special == 120) && surface == 0)
                region.setlakesurface(i, j, region.map(i, j));
        }
    }
}

// This removes any anomalous values. (These are very high/low temperatures that appear on some points in worlds with exotic axial tilt.)

void checkanomalies(planet& world, region& region, vector<vector<int>>& dest, int leftx, int lefty, int rightx, int righty)
{
    int sealevel = world.sealevel();

    int checkamount = 500;

    for (int i = leftx + 1; i < rightx; i++)
    {
        for (int j = lefty + 1; j < righty; j++)
        {
            int thisval = dest[i][j];

            int northval = dest[i][j - 1];
            int southval = dest[i][j + 1];
            int eastval = dest[i + 1][j];
            int westval = dest[i - 1][j];

            if (region.map(i, j) > sealevel && region.map(i, j - 1) > sealevel && region.map(i, j + 1) > sealevel)
            {
                if (thisval < northval - checkamount && thisval < southval - checkamount)
                    dest[i][j] = (northval + southval + eastval + westval) / 4;
                else
                {
                    if (thisval > northval + checkamount && thisval > southval + checkamount)
                        dest[i][j] = (northval + southval + eastval + westval) / 4;
                    else
                    {
                        if (region.map(i + 1, j) > sealevel && region.map(i - 1, j) > sealevel)
                        {
                            if (thisval < eastval - checkamount && thisval < westval - checkamount)
                                dest[i][j] = (northval + southval + eastval + westval) / 4;
                            else
                            {
                                if (thisval > eastval + checkamount && thisval > westval + checkamount)
                                    dest[i][j] = (northval + southval + eastval + westval) / 4;
                            }
                        }
                    }
                }
            }
        }
    }
}

// This creates crater rims.

void makeregionalcraterrim(planet& world, region& region, int dx, int dy, int sx, int sy, int leftx, int lefty, int rightx, int righty, int craterno, vector<vector<int>>& rcratermap, peaktemplate& peaks, vector<int>squareroot)
{
    int width = world.width();
    int height = world.height();

    int rwidth = region.rwidth();
    int rheight = region.rheight();

    int regioncentrex = region.centrex();
    int regioncentrey = region.centrey();

    fast_srand((sy * width + sx) + world.nom(sx, sy) + world.julrain(sx, sy) + world.riverjan(sx,sy));

    float thickness = 2.0f;

    int mcraterradius = (MAXCRATERRADIUS * MAXCRATERRADIUS + MAXCRATERRADIUS) * 24;

    int centrex = dx + 8;
    int centrey = dy + 8;

    int radius = world.craterradius(craterno) * 16;

    if (radius == 16) // For craters of the minimum size, globally speaking, we might make them appear even smaller here.
    {
        radius = random(4, 10);

        if (random(1, 4) == 1)
            radius = random(4, 20);
    }

    int radiuscheck = radius * radius + radius;

    float buttressmult = (float)(random(2, 6)) / 10.0f; // Buttress length will be the radius, multiplied by this.
    int buttresschance = 2;

    int buttresslength = (int)((float)radius * buttressmult);

    int longleftx = leftx - buttresslength; // Because of buttresses, we will have to do rims that are out of view, since their buttresses may be in view.
    int longrightx = rightx + buttresslength;
    int longlefty = lefty - buttresslength;
    int longrighty = righty + buttresslength;

    vector<vector<int>> thiscratermap(RARRAYWIDTH, vector<int>(RARRAYHEIGHT, 0)); // This will have the current crater rim on it.

    int lastgx = -1;
    int lastgy = -1;

    float peakheight = 0.0f;

    bool leftr = 0;
    bool downr = 0;

    int templateno = 0;

    for (int i = 0 - radius; i <= radius; i++)
    {
        int ii = centrex + i;

        if (ii >= longleftx && ii <= longrightx)
        {
            for (int j = 0 - radius; j <= radius; j++)
            {
                int jj = centrey + j;
                
                if (jj >= longlefty && jj <= longrighty && i * i + j * j < radiuscheck) // We're within the radius of this crater.
                {
                    // First, wipe out any rims from earlier craters here.

                    if (ii >= leftx && ii <= rightx && jj >= lefty && jj <= righty)
                        rcratermap[ii][jj] = 0;

                    // Now, work out what global cell we're in.

                    int gx = sx + i / 16;

                    if (gx<0 || gx>width)
                        gx = wrap(gx, width);

                    int gy = sy + j / 16;

                    if (gx != lastgx || gy != lastgy) // If we've moved into another cell, recheck the rim height.
                    {
                        // Check to see whether there is rim here.

                        peakheight = 0.0f;
                        float crount = 0.0f;

                        for (int x = gx - 2; x <= gx + 2; x++)
                        {
                            int xx = x;

                            if (xx<0 || xx>width)
                                xx = wrap(xx, width);

                            for (int y = gy - 2; y <= gy + 2; y++)
                            {
                                if (y >= 0 && y <= height)
                                {
                                    if (world.craterrim(xx, y) != 0)
                                    {
                                        peakheight = peakheight + (float)world.craterrim(xx, y);
                                        crount++;
                                    }
                                }
                            }
                        }

                        if (crount > 1.0f)
                            peakheight = peakheight / crount;
                    }

                    int thisradius = i * i + j * j;

                    if (peakheight > 0.0f && thisradius < mcraterradius)
                    {
                        // Now see if we're on the edge, in which case, paste some rim down.

                        thisradius = squareroot[thisradius];

                        if (thisradius >= radius - (int)thickness)
                        {
                            if (ii >= leftx && ii <= rightx && jj >= lefty && jj <= righty)
                            {
                                leftr = 0;
                                downr = 0;

                                if (random(1, 2) == 1)
                                    leftr = 1;

                                if (random(1, 2) == 1)
                                    downr = 1;

                                templateno = random(1, 2);

                                pastepeak(world, region, ii, jj, peakheight, templateno, leftr, downr, peaks, thiscratermap);
                            }

                            lastgx = gx;
                            lastgy = gy;

                            // Now maybe do a buttress here.

                            if (random(1, buttresschance) == 1)
                            {
                                float rightshift = (float)i * buttressmult;
                                float downshift = (float)j * buttressmult;

                                int bx = ii + (int)rightshift; // Coordinates of the end of the buttress.
                                int by = jj + (int)downshift;

                                float stepno = 200.0f;
                                float rightstep = rightshift / stepno;
                                float downstep = downshift / stepno;
                                float heightstep = peakheight / stepno;

                                int thisx = ii;
                                int thisy = jj;

                                float fx = (float)thisx;
                                float fy = (float)thisy;

                                for (int n = 0; n < (int)stepno; n++)
                                {
                                    fx = fx + rightstep;
                                    fy = fy + downstep;
                                    peakheight = peakheight - heightstep;

                                    if (thisx != (int)fx || thisy != (int)fy) // We've moved to the next cell
                                    {
                                        thisx = (int)fx;
                                        thisy = (int)fy;

                                        if (thisx >= leftx && thisx <= rightx && thisy >= lefty && thisy <= righty)
                                        {
                                            leftr = 0;
                                            downr = 0;

                                            if (random(1, 2) == 1)
                                                leftr = 1;

                                            if (random(1, 2) == 1)
                                                downr = 1;
                                            
                                            templateno = random(1, 2);
                                            
                                            pastepeak(world, region, thisx, thisy, peakheight, templateno, leftr, downr, peaks, thiscratermap);
                                        
                                            if (random(1, 2) == 1) // 4
                                            {
                                                int newx = thisx + random(1, 3) - 2;
                                                int newy = thisy + random(1, 3) - 2;
                                                
                                                if (random(1, 4) == 1)
                                                {
                                                    newx = thisx + random(1, 7) - 4;
                                                    newy = thisy + random(1, 7) - 4;
                                                }

                                                if (newx >= leftx && newx <= rightx && newy >= lefty && newy <= righty)
                                                {
                                                    leftr = 0;
                                                    downr = 0;

                                                    if (random(1, 2) == 1)
                                                        leftr = 1;

                                                    if (random(1, 2) == 1)
                                                        downr = 1;

                                                    templateno = random(1, 2);

                                                    pastepeak(world, region, newx, newy, peakheight, templateno, leftr, downr, peaks, thiscratermap);
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

    // Now copy this crater rim onto the existing crater rim map.

    for (int i = 0; i <= rwidth; i++)
    {
        for (int j = 0; j <= rheight; j++)
        {
            if (rcratermap[i][j] < thiscratermap[i][j])
                rcratermap[i][j] = thiscratermap[i][j];
        }
    }
}

// This lays down the seeds for mud flats.

void makemudflatseeds(planet& world, region& region, int dx, int dy, int sx, int sy, vector<vector<bool>>& riverinlets, vector<vector<float>>& siltstrength)
{
    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();
    
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    int deltasize = 0;

    for (int i = sx - 1; i <= sx + 1; i++)
    {
        int ii = i;

        if (ii<0 || ii>width)
            ii = wrap(ii, width);

        for (int j = sy - 1; j <= sy + 1; j++)
        {          
            if (j >= 0 && j <= height)
            {
                if (world.deltadir(ii, j) != 0)
                {
                    deltasize = (world.deltajan(ii, j) + world.deltajul(ii, j)) / 2;
                    i = sx + 1;
                    j = sy + 1;
                }
            }
        }
    }

    if (deltasize != 0) // With deltas, put masses of silt on all river mouths.
    {
        for (int i = dx; i <= dx + 16; i++)
        {
            for (int j = dy; j <= dy + 16; j++)
            {
                if (region.map(i, j) <= sealevel)
                {
                    bool riverfound = 0;

                    for (int k = i - 1; k <= i + 1; k++)
                    {
                        if (k >= 0 && k <= rwidth)
                        {
                            for (int l = j - 1; l <= j + 1; l++)
                            {
                                if (l >= 0 && l <= rheight)
                                {
                                    if (region.map(k, l) >= sealevel && (region.riverdir(k, l) != 0 || region.fakedir(k, l) != 0))
                                    {
                                        riverfound = 1;
                                        k = i + 1;
                                        l = j + 1;
                                    }
                                }
                            }
                        }
                    }

                    if (riverfound)
                    {
                        siltstrength[i][j] = (float)deltasize / 40.0f;

                        if (siltstrength[i][j] > 100.0f)
                            siltstrength[i][j] = 100.0f;
                    }
                }
            }
        }

        return;
    }

    float thissiltstrength = 0;

    int tide = world.tide(sx, sy); // The higher the tidal range, the more mud flats there will be.

    if (tide > 15) // Cap this, otherwise worlds with high lunar pull will have mud flats literally everywhere.
        tide = 15;

    float riversize = 0.0f; // The bigger the river, the more mud flats there will be.

    for (int i = sx - 1; i <= sx + 1; i++)
    {
        int ii = i;

        if (ii<0 || ii>width)
            ii = wrap(ii, width);

        for (int j = sy - 1; j <= sy + 1; j++)
        {
            if (j >= 0 && j <= height)
            {
                float thissize = (float)world.riveraveflow(i, j);

                if (thissize > riversize)
                    riversize = thissize;
            }
        }
    }

    float riverfactor = riversize / 60.0f; // 80

    if (riverfactor < 0.1f)
        riverfactor = 0.1f;

    if (riverfactor > 8.0f)
        riverfactor = 8.0f;

    thissiltstrength = (float)tide;
    thissiltstrength = thissiltstrength * riverfactor;

    if (thissiltstrength > 100.0f)
        thissiltstrength = 100.0f;

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (region.map(i, j)<=sealevel)
            {
                if (riverinlets[i][j])
                {
                    // Check to see if this is next to land.

                    bool nexttoland = 0;

                    for (int k = i - 1; k <= i + 1; k++)
                    {
                        if (k >= 0 && k <= rwidth)
                        {
                            for (int l = j - 1; l <= j + 1; l++)
                            {
                                if (l >= 0 && l <= rheight)
                                {
                                    if (region.map(k, l) > sealevel)
                                    {
                                        nexttoland = 1;
                                        k = i + 1;
                                        l = j + 1;
                                    }
                                }
                            }
                        }
                    }

                    if (nexttoland) // If it is, put a silt seed there.
                        siltstrength[i][j] = thissiltstrength;
                }
            }
        }
    }
}

// This function spreads out silt from estuaries in order to lay down mud flats.

void spreadsilt(planet& world, region& region, int leftx, int lefty, int rightx, int righty, vector<vector<float>>& siltstrength, boolshapetemplate smalllake[])
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();
    int sealevel = world.sealevel();
    int maxelev = world.maxelevation();

    int regwidthbegin = region.regwidthbegin();
    int regwidthend = region.regwidthend();
    int regheightbegin = region.regheightbegin();
    int regheightend = region.regheightend();

    // First, remove any silt that's on small islands.

    int span = 6;

    for (int i = leftx + span; i <= rightx - span; i++)
    {
        for (int j = lefty + span; j <= righty - span; j++)
        {
            if (siltstrength[i][j] > 0.0f && region.map(i, j) > sealevel)
            {
                bool landfound = 0;
                
                for (int k = i - span; k <= i + span; k++)
                {
                    if (region.map(k, j - span) > sealevel || region.map(k, j + span) > sealevel)
                    {
                        landfound = 1;
                        k = i + span;
                    }
                }

                if (landfound == 0)
                {
                    for (int l = j - span; l <= j + span; l++)
                    {
                        if (region.map(i - span, l) > sealevel || region.map(i + span, l) > sealevel)
                        {
                            landfound = 1;
                            l = j + span;
                        }
                    }
                }

                if (landfound == 0) // This small square is entirely bounded by sea, so get rid of any silt seeds in it.
                {
                    for (int k = i - span; k <= i + span; k++)
                    {
                        for (int l = j - span; l <= j + span; l++)
                            siltstrength[k][l] = 0;
                    }

                }
            }
        }
    }

    vector<vector<bool>> checked(RARRAYWIDTH, vector<bool>(RARRAYHEIGHT, 0));

    float reduce = 50.0f; // 5.0f
    float diagreduce = reduce * 1.41421f;

    float landreduce = reduce * 0.5f; // 0.25f
    float landdiagreduce = landreduce * 1.41421f;

    float riverreduce = landreduce * 0.3f; // 0.5f
    float riverdiagreduce = riverreduce * 1.41421f;

    bool done = 0;

    // Now spread the silt around.

    int crount = 1;

    for (int n = 0; n < 100000; n++) // Make it a finite loop, just in case it gets  stuck...
    {
        done = 0;
        
        int starti = leftx;
        int endi = rightx;
        int istep = 1;

        int startj = lefty;
        int endj = righty;
        int jstep = 1;

        if (crount == 3 || crount == 4)
        {
            starti = rightx;
            endi = leftx;
            istep = -1;
        }

        if (crount == 2 || crount == 4)
        {
            startj = righty;
            endj = lefty;
            jstep = -1;
        }

        for (int i = starti; i <= endi; i = i + istep)
        {
            for (int j = startj; j <= endj; j = j + jstep)
            {
                if (checked[i][j] == 0 && siltstrength[i][j]>1)
                {
                    float newstrength = siltstrength[i][j] - reduce;
                    float newdiagonalstrength = siltstrength[i][j] - diagreduce;

                    float newlandstrength= siltstrength[i][j] - landreduce;
                    float newlanddiagonalstrength = siltstrength[i][j] -landdiagreduce;

                    float newriverstrength = siltstrength[i][j] - riverreduce;
                    float newriverdiagonalstrength = siltstrength[i][j] - riverdiagreduce;

                    bool onriver = 0;

                    if ((region.riverdir(i, j) != 0 || region.fakedir(i, j) != 0))
                        onriver = 1;

                    for (int k = i - 1; k <= i + 1; k++)
                    {
                        if (k >= 0 && k <= rwidth)
                        {
                            for (int l = j - 1; l <= j + 1; l++)
                            {
                                if (l >= 0 && l <= rheight)
                                {
                                    float thisnewstrength = 0.0f;

                                    bool toriver = 0;

                                    if (region.map(k, l) > sealevel && (region.riverdir(k, l) != 0 || region.fakedir(k, l) != 0))
                                        toriver = 1;

                                    if (region.map(k, l) <= sealevel || toriver)
                                    {
                                        if (k == i || l == j)
                                            thisnewstrength = newstrength;
                                        else
                                            thisnewstrength = newdiagonalstrength;
                                    }
                                    else
                                    {
                                        if (k == i || l == j)
                                            thisnewstrength = newlandstrength;
                                        else
                                            thisnewstrength = newlanddiagonalstrength;
                                    }

                                    if (onriver && toriver)
                                    {
                                        if (k == i || l == j)
                                            thisnewstrength = newriverstrength;
                                        else
                                            thisnewstrength = newriverdiagonalstrength;
                                    }

                                    // Now reduce it if we're near the edge of the map, because we can't carry beaches over from one region to the next.

                                    int edgedist = k - regwidthbegin; // Distance to the nearest edge of the visible map.

                                    if (regwidthend - k < edgedist)
                                        edgedist = regwidthend - k;

                                    if (l - regheightbegin < edgedist)
                                        edgedist = l - regheightbegin;

                                    if (regheightend - l < edgedist)
                                        edgedist = regheightend - l;

                                    if (edgedist < 40)
                                        thisnewstrength = thisnewstrength * ((float)edgedist / 40.0f);

                                    if (siltstrength[k][l] < thisnewstrength)
                                    {
                                        siltstrength[k][l] = thisnewstrength;
                                        done = 1;
                                        checked[k][l] = 0;
                                    }
                                }
                            }
                        }
                    }

                    checked[i][j] = 1;
                }
            }
        }

        crount++;

        if (crount > 4)
            crount = 4;

        if (done == 0)
            n = 100000;
    }

    // Now turn that into mud flats where there's enough silt.

    float minsilt;

    if (world.lunar() <= 1.0f)
        minsilt = 100.0f - world.lunar() * 60.0f;
    else
        minsilt = 44.0f - world.lunar() * 4.0f;

    if (minsilt > 100.0f)
        minsilt = 100.0f;

    if (minsilt < 10.0f)
        minsilt = 10.0f;

    for (int i = leftx; i <= rightx; i++)
    {
        for (int j = lefty; j <= righty; j++)
        {
            if (siltstrength[i][j] >= minsilt)
            {
                region.setmap(i, j, sealevel + 1);
                region.setmud(i, j, 1);

                // Possibly make some salt marshes here too!

                if ((float)random(1, 100) / 100.0f > region.roughness(i, j)) // The lower the roughness, the more likely salt wetlands are.
                {
                    int shapenumber = random(0, 11);
                    pasteregionalwetlands(region, i, j, 132, sealevel + 1, shapenumber, smalllake);
                }
            }
        }
    }
}

// This lays down the seeds for beaches.

void makesandseeds(planet& world, region& region, int dx, int dy, int sx, int sy, vector<vector<bool>>& riverinlets, vector<vector<float>>& sandstrength, vector<vector<float>>& shinglestrength)
{
    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();
    int maxelev = world.maxelevation();
    int lunar = (int)((world.lunar() * world.lunar()) / 2.0f);

    int rwidth = region.rwidth();
    int rheight = region.rheight();

    int centrex = region.centrex();
    int centrey = region.centrey();

    bool seafound = 0;
    bool landfound = 0;

    fast_srand((sy * width + sx) + world.nom(sx, sy) + world.julrain(sx, sy));

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (region.sea(i, j))
                seafound = 1;

            if (region.sea(i, j) == 0)
                landfound = 1;

            if (seafound && landfound)
            {
                i = dx + 16;
                j = dy + 16;
            }
        }
    }

    if (seafound == 0 || landfound == 0)
        return;

    bool shingle = 0;

    if (random(1, maxelev) < world.noise(sx, sy))
        shingle = 1;

    float thissandstrength = 0;

    int tide = world.tide(sx, sy); // The higher the tidal range, the more sand there will be.

    if (tide < lunar)
        tide = lunar;

    float riversize = 0.0f; // The smaller the river, the more sand there will be.

    for (int i = sx - 1; i <= sx + 1; i++)
    {
        int ii = i;

        if (ii<0 || ii>width)
            ii = wrap(ii, width);

        for (int j = sy - 1; j <= sy + 1; j++)
        {
            if (j >= 0 && j <= height)
            {
                float thissize = (float)world.riveraveflow(i, j);

                if (thissize > riversize)
                    riversize = thissize;
            }
        }
    }

    float riverfactor = riversize / 60.0f; // 80

    if (riverfactor < 0.1f)
        riverfactor = 0.1f;

    if (riverfactor > 8.0f)
        riverfactor = 8.0f;

    riverfactor = 8.0f - riverfactor;

    int extrachance = 0; // The lower this is, the more likely there are to be beaches away from inlets.

    if (world.lunar() > 2.0f)
    {
        extrachance = 45 - ((int)(world.lunar() * 4.0f));

        if (riverfactor < world.lunar())
            riverfactor = world.lunar();
    }

    if (random(1, 100) == 1 && sx > centrex - 10 && sx<centrex + 10 && sy>centrey - 10 && sy < centrey + 10) // Occasionally, have a huge beach!
        thissandstrength = (float)tide * 4.0f;
    else
        thissandstrength = (float)tide * 0.5f; // *2.0f;

    thissandstrength = thissandstrength * riverfactor;   

    if (thissandstrength > 100.0f)
        thissandstrength = 100.0f;

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (region.map(i, j) <= sealevel)
            {
                bool extrabeach = 0;

                if (extrachance >= 1 && random(1, extrachance) == 1)
                    extrabeach = 1;
                
                if (riverinlets[i][j] || extrabeach)
                {
                    // Check to see if this is next to land where there's a river.

                    bool nexttoland = 0;

                    for (int k = i - 1; k <= i + 1; k++)
                    {
                        if (k >= 0 && k <= rwidth)
                        {
                            for (int l = j - 1; l <= j + 1; l++)
                            {
                                if (l >= 0 && l <= rheight)
                                {
                                    if (region.map(k, l) > sealevel && (extrabeach || region.riverdir(k, l) != 0))
                                    {
                                        nexttoland = 1;
                                        k = i + 1;
                                        l = j + 1;
                                    }
                                }
                            }
                        }
                    }

                    if (nexttoland) // If it is, put a sand seed there.
                    {
                        if (shingle)
                            shinglestrength[i][j] = thissandstrength;
                        else
                            sandstrength[i][j] = thissandstrength;
                    }
                }
            }
        }
    }
}

// This function spreads out sand in order to lay down sandy beaches.

bool spreadsand(planet& world, region& region, int dx, int dy, int sx, int sy, float reduce, float diagreduce, float landreduce, float landdiagreduce, float coastreduce, float coastdiagreduce, int crount, bool shingle, vector<vector<bool>>& checked, vector<vector<float>>& sandstrength)
{
    int width = world.width();
    
    int rwidth = region.rwidth();
    int rheight = region.rheight();
    int sealevel = world.sealevel();
    int maxelev = world.maxelevation();

    int minvar = 40;
    int maxvar = 140;

    if (shingle == 1)
    {
        minvar = 60;
        maxvar = 140;
    }

    bool done = 0;

    fast_srand((sy * width + sx) + world.nom(sx, sy) + world.julrain(sx, sy));

    // Now spread the sand around.

    int starti = dx;
    int endi = dx+16;
    int istep = 1;

    int startj = dy;
    int endj = dy+16;
    int jstep = 1;

    if (crount == 3 || crount == 4)
    {
        starti = dx+16;
        endi = dx;
        istep = -1;
    }

    if (crount == 2 || crount == 4)
    {
        startj = dy+16;
        endj = dy;
        jstep = -1;
    }

    for (int i = starti; i <= endi; i = i + istep)
    {
        for (int j = startj; j <= endj; j = j + jstep)
        {
            if (checked[i][j] == 0 && sandstrength[i][j] > 1)
            {
                float newstrength = sandstrength[i][j] - reduce;
                float newdiagonalstrength = sandstrength[i][j] - diagreduce;

                float newlandstrength = sandstrength[i][j] - landreduce;
                float newlanddiagonalstrength = sandstrength[i][j] - landdiagreduce;

                float newcoaststrength = sandstrength[i][j] - coastreduce;
                float newcoastdiagonalstrength = sandstrength[i][j] - coastdiagreduce;

                for (int k = i - 1; k <= i + 1; k++)
                {
                    if (k >= 0 && k <= rwidth)
                    {
                        for (int l = j - 1; l <= j + 1; l++)
                        {
                            if (l >= 0 && l <= rheight)
                            {
                                float thisnewstrength = 0.0f;

                                if (region.map(k, l) <= sealevel)
                                {
                                    bool landfound = 0;

                                    for (int m = k - 1; m <= k + 1; m++)
                                    {
                                        if (m >= 0 && m <= rwidth)
                                        {
                                            for (int n = l - 1; n <= l + 1; n++)
                                            {
                                                if (n >= 0 && n <= rheight)
                                                {
                                                    if (region.map(m, n) > sealevel)
                                                    {
                                                        landfound = 1;
                                                        m = k + 1;
                                                        n = l + 1;
                                                    }
                                                }
                                            }
                                        }
                                    }

                                    if (landfound)
                                    {
                                        if (k == i || l == j)
                                            thisnewstrength = newcoaststrength;
                                        else
                                            thisnewstrength = newcoastdiagonalstrength;
                                    }
                                    else
                                    {
                                        if (k == i || l == j)
                                            thisnewstrength = newstrength;
                                        else
                                            thisnewstrength = newdiagonalstrength;
                                    }
                                }
                                else
                                {
                                    if (k == i || l == j)
                                        thisnewstrength = newlandstrength;
                                    else
                                        thisnewstrength = newlanddiagonalstrength;
                                }

                                float var = (float)random(minvar, maxvar) / 100.0f;

                                thisnewstrength = thisnewstrength * var;

                                if (thisnewstrength > sandstrength[i][j])
                                    thisnewstrength = sandstrength[i][j];

                                if (sandstrength[k][l] < thisnewstrength)
                                {
                                    sandstrength[k][l] = thisnewstrength;
                                    done = 1;
                                    checked[k][l] = 0;
                                }
                            }
                        }
                    }
                }

                checked[i][j] = 1;
            }
        }
    }

    return done;
}

// This function turns those arrays into actual beaches.

void createbeaches(planet& world, region& region, int leftx, int lefty, int rightx, int righty, bool shingle, vector<vector<float>>& sandstrength)
{
    int sealevel = world.sealevel();
    
    float minsand;

    if (world.lunar() <= 1.0f)
        minsand = 100.0f - world.lunar() * 60.0f;
    else
        minsand = 46.0f - world.lunar() * 6.0f;

    if (minsand > 100.0f)
        minsand = 100.0f;

    if (minsand < 0.1f)
        minsand = 0.1f;

    for (int i = leftx; i <= rightx; i++)
    {
        for (int j = lefty; j <= righty; j++)
        {
            if (sandstrength[i][j] >= minsand)
            {
                region.setmap(i, j, sealevel + 1);

                if (shingle == 0)
                    region.setsand(i, j, 1);
                else
                    region.setshingle(i, j, 1);
            }
        }
    }
}

// This function makes any sandy beaches that are next to mud flats muddy.

void putmudonsand(planet& world, region& region, int leftx, int lefty, int rightx, int righty)
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    vector<vector<bool>> checked(RARRAYWIDTH, vector<bool>(RARRAYHEIGHT, 0));

    for (int n = 0; n < 100000; n++)
    {
        bool doneone = 0;
        
        for (int i = leftx; i <= rightx; i++)
        {
            for (int j = lefty; j <= righty; j++)
            {
                if (checked[i][j] == 0 && region.mud(i, j))
                {
                    checked[i][j] = 1;

                    if (random(1, 4) == 1) // Don't always do this.
                    {
                        for (int k = i - 1; k <= i + 1; k++)
                        {
                            if (k >= 0 && k <= rwidth)
                            {
                                for (int l = j - 1; l <= j + 1; l++)
                                {
                                    if (l >= 0 && l <= rheight)
                                    {
                                        if (region.sand(k, l) && region.mud(k, l) == 0)
                                        {
                                            region.setmud(k, l, 1);
                                            doneone = 1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        if (doneone == 0)
            n = 100000;
    }
}

// This function makes sure that the sea isn't too deep next to mud flats, beaches, and wetlands.

void checkbeachcoasts(planet& world, region& region, int leftx, int lefty, int rightx, int righty)
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();
    int sealevel = world.sealevel();

    int maxdepth = 50;

    for (int i = leftx; i <= rightx; i++)
    {
        for (int j = lefty; j <= righty; j++)
        {
            if (region.map(i, j) <= sealevel)
            {
                region.setsand(i, j, 0);
                region.setshingle(i, j, 0);
                region.setbarrierisland(i, j, 0);
            }
            
            if (region.map(i, j) < sealevel - maxdepth)
            {
                bool found = 0;

                for (int k = i - 1; k <= i + 1; k++)
                {
                    if (k >= 0 && k <= rwidth)
                    {
                        for (int l = j - 1; l <= j + 1; l++)
                        {
                            if (l >= 0 && l <= rheight)
                            {
                                if ((region.sand(k, l) || region.shingle(k, l) || region.mud(k, l)) || region.special(k, l) >= 130)
                                {
                                    found = 1;
                                    k = i + 1;
                                    l = j + 1;
                                }
                            }
                        }
                    }
                }

                if (found)
                    region.setmap(i, j, sealevel - maxdepth + randomsign(random(1, 10)));
            }
        }
    }
}

// This function adds barrier islands to the edges of some mud flats.

void makemudbarriers(planet& world, region& region, int dx, int dy, int sx, int sy)
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();
    int sealevel = world.sealevel();

    for (int i = dx; i <= dx + 16; i++)
    {
        for (int j = dy; j <= dy + 16; j++)
        {
            if (region.mud(i, j) && random(1, 2) == 1)
            {
                bool landfound = 0;
                bool seafound = 0;

                for (int k = i - 1; k <= i + 1; k++)
                {
                    for (int l = j - 1; l <= j + 1; l++)
                    {
                        if (region.map(k, l) <= sealevel)
                            seafound = 1;

                        if (region.mud(k, l) == 0 && region.map(k, l) > sealevel)
                        {
                            landfound = 1;
                            k = i + 1;
                            l = j + 1;
                        }
                    }
                }

                if (seafound && landfound == 0)
                {
                    region.setmud(i, j, 0);
                    region.setmap(i, j, sealevel + 1);
                    region.setbarrierisland(i, j, 1);
                }
            }
        }
    }
}

// This function checks that lake surfaces are consistent.

void checklakesurfaces(planet& world, region& region)
{
    int width = world.width();
    int height = world.height();

    int rwidth = region.rwidth();
    int rheight = region.rheight();

    int leftx = region.leftx();
    int lefty = region.lefty();

    int startx = region.regwidthbegin();
    int endx = region.regwidthend();
    int starty = region.regheightbegin();
    int endy = region.regheightend();

    bool doneone = 0;

    for (int n = 0; n < 10000; n++)
    {
        doneone = 0;
        
        for (int x = startx; x <= endx; x++)
        {
            for (int y = starty; y <= endy; y++)
            {
                if (region.lakesurface(x, y) != 0)
                {
                    int nsurface = region.lakesurface(x, y - 1);
                    int ssurface = region.lakesurface(x, y + 1);
                    int esurface = region.lakesurface(x + 1, y);
                    int wsurface = region.lakesurface(x - 1, y);

                    if (nsurface > 0 && ssurface > 0 && esurface > 0 && wsurface > 0)
                    {
                        if (nsurface != ssurface || (nsurface != esurface || nsurface != wsurface)) // Discrepancy!
                        {
                            // Find the most common lake surface in the surrounding global tiles.

                            int sx = leftx + (x / 16);
                            int sy = lefty + (y / 16); // Coordinates of the relevant global cell.

                            int level[9];
                            int crount[9];

                            for (int n = 0; n < 9; n++)
                            {
                                level[n] = 0;
                                crount[n] = 0;
                            }

                            for (int i = sx - 1; i <= sx + 1; i++)
                            {
                                int ii = i;

                                if (ii<0 || ii>width)
                                    ii = wrap(ii, width);

                                for (int j = sy - 1; j <= sy + 1; j++)
                                {
                                    if (j >= 0 && j <= height)
                                    {
                                        int thissurface = world.lakesurface(ii, j);

                                        if (thissurface != 0)
                                        {
                                            for (int n = 0; n < 9; n++)
                                            {
                                                if (level[n] = thissurface)
                                                {
                                                    crount[n]++;
                                                    n = 9;
                                                }
                                                else
                                                {
                                                    if (level[n] == 0)
                                                    {
                                                        level[n] = thissurface;
                                                        crount[n] = 1;
                                                        n = 9;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            
                            int highest = 0;
                            int mostpopular = -1;

                            for (int n = 0; n < 9; n++)
                            {
                                if (crount[n] > highest)
                                {
                                    highest = crount[n];
                                    mostpopular = n;
                                }
                            }

                            int correctsurface = 0;
                            
                            if (mostpopular != -1)
                                correctsurface = level[mostpopular];

                            doneone = 1;

                            /*
                            // Find the lowest of all the surrounding lake surfaces, and use that.

                            int correctsurface = nsurface;

                            if (ssurface < correctsurface)
                                correctsurface = ssurface;

                            if (esurface < correctsurface)
                                correctsurface = esurface;

                            if (wsurface < correctsurface)
                                correctsurface = wsurface;                            
                            */

                            if (correctsurface != 0)
                            {
                                for (int i = x - 16; i <= x + 16; i++)
                                {
                                    for (int j = y - 16; j <= y + 16; j++)
                                    {
                                        if (i >= 0 && i <= rwidth && j >= 0 && j <= rheight && region.lakesurface(i, j) > 0)
                                            region.setlakesurface(i, j, correctsurface);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        if (doneone == 0)
            n = 10000;
    }
}