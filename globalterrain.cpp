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
//#include <unistd.h>
#include <queue>
#include <SFML/Graphics.hpp>
#include <nanogui/nanogui.h>

#include "classes.hpp"
#include "planet.hpp"
#include "region.hpp"
#include "functions.hpp"

using namespace std;

void generateglobalterrain(planet &world, short terraintype, nanogui::Screen &screen, nanogui::Window &worldgenerationwindow, nanogui::Label &worldgenerationlabel, nanogui::ProgressBar &worldprogress, float progressstep, boolshapetemplate landshape[], boolshapetemplate chainland[], vector<vector<int>> &mountaindrainage, vector<vector<bool>> &shelves)
{
    switch (terraintype)
    {
        case 1:
            generateglobalterraintype1(world,screen,worldgenerationwindow,worldgenerationlabel,worldprogress,progressstep,landshape,mountaindrainage,shelves,chainland);
            break;
            
        case 2:
            generateglobalterraintype2(world,screen,worldgenerationwindow,worldgenerationlabel,worldprogress,progressstep,landshape,mountaindrainage,shelves,chainland);
            break;
    }
}

// This creates type 1 terrain. This type gives a fairly chaotic looking map with small continents and lots of islands.

void generateglobalterraintype1(planet &world, nanogui::Screen &screen, nanogui::Window &worldgenerationwindow, nanogui::Label &worldgenerationlabel, nanogui::ProgressBar &worldprogress, float progressstep, boolshapetemplate landshape[],vector<vector<int>> &mountaindrainage, vector<vector<bool>> &shelves, boolshapetemplate chainland[])
{
    // First get our key variables and clear the world.
    
    long seed=world.seed();
    fast_srand(seed);
    
    int width=world.width();
    int height=world.height();
    int maxelev=world.maxelevation();
    int sealevel=world.sealevel();
    int baseheight=sealevel-4500; //1250;
    if (baseheight<1)
        baseheight=1;
    int conheight=sealevel+50;
    
    vector<vector<int>> plateaumap(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
    vector<vector<int>> seafractal(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
    vector<vector<bool>> removedland(ARRAYWIDTH,vector<bool>(ARRAYWIDTH,ARRAYHEIGHT)); // This will show where land has been removed.
    vector<vector<int>> volcanodensity(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT)); // How many submarine volcanos.
    vector<vector<int>> volcanodirection(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT)); // Direction of extinct volcanos leading away from active ones.
    
    world.clear(); // Clears all of the maps in this world.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            world.setnom(i,j,0);
            plateaumap[i][j]=0;
            mountaindrainage[i][j]=0;
            shelves[i][j]=0;
            seafractal[i][j]=0;
            removedland[i][j]=0;
            volcanodensity[i][j]=0;
            volcanodirection[i][j]=0;
        }
    }
    
    // Now start the generating.
    
    worldgenerationlabel.set_caption("Creating fractal map");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    // First make a fractal for noise (used only at regional map level).
    
    int grain=8; // Level of detail on this fractal map.
    float valuemod=0.2;
    int v=random(3,6);
    float valuemod2=v;
    
    vector<vector<int>> fractal(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
    
    createfractal(fractal,width,height,grain,valuemod,valuemod2,1,maxelev,0,0);
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
            world.setnoisemap(i,j,fractal[i][j]);
    }
    
    // Now make a new one that we'll actually use for global terrain creation.
    
    createfractal(fractal,width,height,grain,valuemod,valuemod2,1,maxelev,0,0);
    
    worldgenerationlabel.set_caption("Creating continental map");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    smallcontinents(world,baseheight,conheight,fractal,plateaumap,landshape,chainland);

    flip(fractal,width,height,1,1);
    
    // Merge the maps.
    
    worldgenerationlabel.set_caption("Merging maps");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    fractalmerge(world,fractal);
    
    // Make continental shelves.
    
    worldgenerationlabel.set_caption("Making continental shelves");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    makecontinentalshelves(world,shelves,20); // Wider shelves than with the other terrain generator, because this one will produce lots of little bits of land

    // Now we add some island chains.
    
    sf::Vector2i focuspoints[4]; // These will be points where island chains will start close to.
    
    int focustotal=random(2,4); // The number of actual focus points.
    
    for (int n=0; n<focustotal; n++)
    {
        focuspoints[n].x=random(0,width);
        focuspoints[n].y=random(0,height);
    }
    
    int focaldistance=height/2; // Maximum distance a chain can start from the focuspoint.
    createchains(world,baseheight,conheight,fractal,plateaumap,landshape,chainland,focuspoints,focustotal,focaldistance,3);

    worldgenerationlabel.set_caption("Shifting fractal");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    shift(fractal,width,height,width/2); // Shift the fractal to make it different for the mountains - we will use the fractal to adjust peak height.
    
    worldgenerationlabel.set_caption("Smoothing map");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    world.smoothnom(1);
    
    // We also need to remove the trench down the left-hand side.
    
    removeseam(world,0);
    
    // Now remove inland seas.
    
    worldgenerationlabel.set_caption("Removing inland seas");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    removeseas(world,conheight);
    
    // Now widen any channels and lower the coasts.
    
    worldgenerationlabel.set_caption("Tidying up oceans");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    widenchannels(world);
    loweroceans(world);
    
    // Now try to remove straight coastlines.
    
    worldgenerationlabel.set_caption("Improving coastlines");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    removestraights(world);
    
    // Now make sure the southern edge is correct.
    
    worldgenerationlabel.set_caption("Checking poles");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    checkpoles(world);
    
    // Now sort out the sea depths.
    
    worldgenerationlabel.set_caption("Adjusting ocean depths");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    grain=8; // Level of detail on this fractal map.
    valuemod=0.2;
    v=random(3,6);
    valuemod2=v;
    
    createfractal(seafractal,width,height,grain,valuemod,valuemod2,1,maxelev,0,0);
    
    float coastalvarreduce=maxelev/500; //3000;
    float oceanvarreduce=maxelev/1000;
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (world.sea(i,j)==1)
            {
                if (shelves[i][j]==1)
                {
                    float var=seafractal[i][j]-maxelev/2;
                    var=var/coastalvarreduce;
                    
                    int newval=sealevel-200+var;
                    
                    if (newval>sealevel-10)
                        newval=sealevel-10;
                    
                    if (newval<1)
                        newval=1;
                    
                    world.setnom(i,j,newval);
                }
                else
                {
                    int ii=i+width/2;
                    
                    if (ii>width)
                        ii=ii-width;
                    
                    float var=seafractal[ii][j]-maxelev/2;
                    var=var/oceanvarreduce;
                    
                    int newval=sealevel-5000+var;
                    
                    if (newval>sealevel-3000)
                        newval=sealevel-3000;
                    
                    if (newval<1)
                        newval=1;
                    
                    world.setnom(i,j,newval);
                }
            }
        }
    }

    // Now we create mid-ocean ridges.
    
    worldgenerationlabel.set_caption("Generating mid-ocean ridges");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    createoceanridges(world,shelves);
    
    // Now we create deep-sea trenches.
    
    worldgenerationlabel.set_caption("Generating deep-sea trenches");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    createoceantrenches(world,shelves);
    
    // Now random volcanos.
    
    worldgenerationlabel.set_caption("Generating volcanos");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    grain=8; // Level of detail on this fractal map.
    valuemod=0.2;
    v=random(3,6);
    valuemod2=v;
    
    createfractal(volcanodensity,width,height,grain,valuemod,valuemod2,1,maxelev,0,0);
    
    grain=4; // Level of detail on this fractal map.
    valuemod=0.02;
    v=1; //random(3,6);
    valuemod2=0.2;
    
    createfractal(volcanodirection,width,height,grain,valuemod,valuemod2,1,maxelev,0,0);
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            bool goahead=0;
            
            if (world.sea(i,j)==1)
            {
                int frac=volcanodensity[i][j];
                
                int ii=i+width/2;
                if (ii>width)
                    ii=ii-width;
                
                int frac2=volcanodensity[ii][j];
                
                if (frac2<frac)
                    frac=frac2;
                
                int jj=j+height/2;
                if (jj>height)
                    jj=jj-height;
                
                int frac3=volcanodensity[i][jj];
                
                if (frac3<frac)
                    frac=frac3;
                
                int rand=5000;
                
                if (frac>maxelev/2)
                    rand=80;
                
                if (frac>(maxelev/4)*3)
                    rand=40;
                
                if (random(1,rand)==1)
                    goahead=1;
            }
            else
            {
                if (random(1,15000)==1)
                    goahead=1;
            }
            
            if (goahead==1)
            {
                bool strato=1;
                
                if (random(1,10)==1) // Shield volcanoes - much rarer.
                    strato=0;
                
                if (world.sea(i,j)==1)
                    strato=1;
                
                int peakheight;
                
                if (world.sea(i,j)==1)
                {
                    if (random(1,10)==1) // It could make a chain of volcanic islands.
                        peakheight=sealevel-world.nom(i,j)+random(500,3000);
                    else
                        peakheight=sealevel-world.nom(i,j)-random(100,200);
                    
                    if (peakheight<10)
                        peakheight=10;
                    
                    peakheight=random(peakheight/2,peakheight);
                }
                else
                {
                    if (strato==1)
                        peakheight=random(2000,6000);
                    else
                        peakheight=random(1000,2000);
                }
                
                //world.settest(i,j,1);
                
                createisolatedvolcano(world,i,j,shelves,volcanodirection,peakheight,strato);
            }
        }
    }
    
    // Now we shift the map so there is sea at the edges, if possible.
    
    worldgenerationlabel.set_caption("Shifting for best position");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    removeseam(world,0);
    
    adjustforsea(world);
    
    // Now we add smaller mountain chains that cannot form peninsulas.
    
    worldgenerationlabel.set_caption("Adding smaller mountain ranges");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    sf::Vector2i dummy[1];
    
    createchains(world,baseheight,conheight,fractal,plateaumap,landshape,chainland,dummy,0,0,2);
    
    // Now we alter the fractal again, and use it to add more height variation.
    
    worldgenerationlabel.set_caption("Merging fractal into land");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    flip(fractal,width,height,1,1);
    int offset=random(1,width);
    shift(fractal,width,height,offset);
    
    fractalmergeland(world,fractal,conheight);
    
    // Now remove any mountains that are over sea.
    
    worldgenerationlabel.set_caption("Removing floating mountains");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    removefloatingmountains(world);
    cleanmountainridges(world);
    
    // Now we raise the mountain bases.
    
    worldgenerationlabel.set_caption("Raising mountain bases");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    raisemountainbases(world,mountaindrainage);
    
    // Now we add plateaux.
    
    worldgenerationlabel.set_caption("Adding plateaux");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    erodeplateaux(world,plateaumap);
    smooth(plateaumap,width,height,maxelev,1,1);
    addplateaux(world,plateaumap,conheight);
    
    // Now we smooth again, without changing the coastlines.
    
    worldgenerationlabel.set_caption("Smoothing map, preserving coastlines");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    smoothland(world,2);
    
    // Now we create extra elevation over the land to create canyons.
    
    worldgenerationlabel.set_caption("Elevating land near canyons");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    createextraelev(world);
    
    // Now we remove depressions.
    
    worldgenerationlabel.set_caption("Filling depressions");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    depressionfill(world);
    
    addlandnoise(world); // Add a bit of noise, then do remove depressions again. This is to add variety to the river courses.
    
    depressionfill(world);
    
    // Now we adjust the land around coastlines.
    
    worldgenerationlabel.set_caption("Adjusting coastlines");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int n=1; n<=2; n++)    
        normalisecoasts(world,13,11,4);
    
    clamp(world);
    
    // Now we note down one-tile islands.
    
    worldgenerationlabel.set_caption("Checking islands");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    checkislands(world);
    
    // Now we create a roughness map.
    
    worldgenerationlabel.set_caption("Creating roughness map");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    vector<vector<int>> roughness(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
    
    grain=8; // Level of detail on this fractal map.
    valuemod=0.2;
    valuemod2=0.6;
    
    createfractal(roughness,width,height,grain,valuemod,valuemod2,1,maxelev,0,0);
    
    vector<vector<int>> roughness2(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
    
    createfractal(roughness2,width,height,grain,valuemod,valuemod2,1,maxelev,0,0);
    
    for (int i=0; i<=width; i++) // Do two, so that it's rare to get areas that are very smooth.
    {
        for (int j=0; j<=height; j++)
        {
            if (roughness[i][j]<roughness2[i][j])
                roughness[i][j]=roughness2[i][j];
        }
    }
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
            world.setroughness(i,j,roughness[i][j]);
    }
}

// This creates type 2 terrain. This type gives a more earthlike map with large continents.

void generateglobalterraintype2(planet &world, nanogui::Screen &screen, nanogui::Window &worldgenerationwindow, nanogui::Label &worldgenerationlabel, nanogui::ProgressBar &worldprogress, float progressstep, boolshapetemplate landshape[],vector<vector<int>> &mountaindrainage, vector<vector<bool>> &shelves, boolshapetemplate chainland[])
{
    // First get our key variables and clear the world.
    
    long seed=world.seed();
    fast_srand(seed);
    
    int width=world.width();
    int height=world.height();
    int maxelev=world.maxelevation();
    int sealevel=world.sealevel();
    int baseheight=sealevel-4500; //1250;
    if (baseheight<1)
        baseheight=1;
    int conheight=sealevel+50;
    
    int maxmountainheight=maxelev-sealevel;

    vector<vector<int>> plateaumap(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
    vector<vector<int>> seafractal(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
    vector<vector<bool>> removedland(ARRAYWIDTH,vector<bool>(ARRAYWIDTH,ARRAYHEIGHT)); // This will show where land has been removed.
    vector<vector<int>> volcanodensity(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT)); // How many submarine volcanos.
    vector<vector<int>> volcanodirection(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT)); // Direction of extinct volcanos leading away from active ones.
    
    world.clear(); // Clears all of the maps in this world.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            world.setnom(i,j,0);
            plateaumap[i][j]=0;
            mountaindrainage[i][j]=0;
            shelves[i][j]=0;
            seafractal[i][j]=0;
            removedland[i][j]=0;
            volcanodensity[i][j]=0;
            volcanodirection[i][j]=0;
        }
    }
    
    // Now start the generating.
    
    worldgenerationlabel.set_caption("Creating fractal map");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();

    // First make a fractal for noise (used only at regional map level).
    
    int grain=8; // Level of detail on this fractal map.
    float valuemod=0.2;
    int v=random(3,6);
    float valuemod2=v;
    
    vector<vector<int>> fractal(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
    
    createfractal(fractal,width,height,grain,valuemod,valuemod2,1,maxelev,0,0);
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
            world.setnoisemap(i,j,fractal[i][j]);
    }
    
    // Now make a new one that we'll actually use for global terrain creation.
    
    createfractal(fractal,width,height,grain,valuemod,valuemod2,1,12750,0,0);

    int fractaladd=sealevel-2500;
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
            fractal[i][j]=fractal[i][j]+fractaladd;
    }

    worldgenerationlabel.set_caption("Creating continental map");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    largecontinents(world,screen,worldgenerationwindow,worldgenerationlabel,worldprogress,progressstep,baseheight,conheight,fractal,plateaumap,shelves,landshape,chainland);

    flip(fractal,width,height,1,1);
    
    // Now merge the maps.
 
    worldgenerationlabel.set_caption("Merging maps");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
 
    fractalmergemodified(world,fractal,plateaumap,removedland);
 
    worldgenerationlabel.set_caption("Shifting fractal");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
 
    shift(fractal,width,height,width/2); // Shift the fractal to make it different for the mountains - we will use the fractal to adjust peak height.
 
    worldgenerationlabel.set_caption("Smoothing map");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
 
    world.smoothnom(1);

    // We also need to remove the trench down the left-hand side.
    
    removeseam(world,0);
    
    // Now remove inland seas.
    
    worldgenerationlabel.set_caption("Removing inland seas");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    removeseas(world,conheight);
    
    // Now make sure the southern edge is correct.
    
    worldgenerationlabel.set_caption("Checking poles");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    checkpoles(world);

    // Now add Aegean-style islands
    
    worldgenerationlabel.set_caption("Adding archipelagos");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    makearchipelagos(world,removedland,landshape);
    
    // Now widen any channels and lower the coasts.
    
    worldgenerationlabel.set_caption("Tidying up oceans");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    widenchannels(world);
    loweroceans(world);
    
    // Now try to remove straight coastlines.
    
    worldgenerationlabel.set_caption("Improving coastlines");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    removestraights(world);
    
    // Now sort out the sea depths.
    
    worldgenerationlabel.set_caption("Adjusting ocean depths");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    grain=8; // Level of detail on this fractal map.
    valuemod=0.2;
    v=random(3,6);
    valuemod2=v;
    
    createfractal(seafractal,width,height,grain,valuemod,valuemod2,1,maxelev,0,0);
    
    float coastalvarreduce=maxelev/500; //3000;
    float oceanvarreduce=maxelev/1000;
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (world.sea(i,j)==1)
            {
                if (shelves[i][j]==1)
                {
                    float var=seafractal[i][j]-maxelev/2;
                    var=var/coastalvarreduce;
                    
                    int newval=sealevel-200+var;
                    
                    if (newval>sealevel-10)
                        newval=sealevel-10;
                    
                    if (newval<1)
                        newval=1;
                    
                    world.setnom(i,j,newval);
                }
                else
                {
                    int ii=i+width/2;
                    
                    if (ii>width)
                        ii=ii-width;
                    
                    float var=seafractal[ii][j]-maxelev/2;
                    var=var/oceanvarreduce;
                    
                    int newval=sealevel-5000+var;
                    
                    if (newval>sealevel-3000)
                        newval=sealevel-3000;
                    
                    if (newval<1)
                        newval=1;
                    
                    world.setnom(i,j,newval);
                }
            }
        }
    }

    // Now we create mid-ocean ridges.
    
    worldgenerationlabel.set_caption("Generating mid-ocean ridges");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    createoceanridges(world,shelves);
    
    // Now we create deep-sea trenches.
    
    worldgenerationlabel.set_caption("Generating deep-sea trenches");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    createoceantrenches(world,shelves);
    
    // Now random volcanos.
    
    worldgenerationlabel.set_caption("Generating volcanos");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    grain=8; // Level of detail on this fractal map.
    valuemod=0.2;
    v=random(3,6);
    valuemod2=v;
    
    createfractal(volcanodensity,width,height,grain,valuemod,valuemod2,1,maxelev,0,0);
    
    grain=4; // Level of detail on this fractal map.
    valuemod=0.02;
    v=1; //random(3,6);
    valuemod2=0.2;
    
    createfractal(volcanodirection,width,height,grain,valuemod,valuemod2,1,maxelev,0,0);
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            bool goahead=0;
            
            if (world.sea(i,j)==1)
            {
                int frac=volcanodensity[i][j];
                
                int ii=i+width/2;
                if (ii>width)
                    ii=ii-width;
                
                int frac2=volcanodensity[ii][j];
                
                if (frac2<frac)
                    frac=frac2;
                
                int jj=j+height/2;
                if (jj>height)
                    jj=jj-height;
                
                int frac3=volcanodensity[i][jj];

                if (frac3<frac)
                    frac=frac3;

                int rand=5000;
                
                if (frac>maxelev/2)
                    rand=80;
                
                if (frac>(maxelev/4)*3)
                    rand=40;
                
                if (random(1,rand)==1)
                    goahead=1;
            }
            else
            {
                if (random(1,15000)==1)
                    goahead=1;
            }
            
            if (goahead==1)
            {
                bool strato=1;
                
                if (random(1,10)==1) // Shield volcanoes - much rarer.
                    strato=0;
                
                if (world.sea(i,j)==1)
                    strato=1;
                
                int peakheight;
                
                if (world.sea(i,j)==1)
                {
                    if (random(1,10)==1) // It could make a chain of volcanic islands.
                        peakheight=sealevel-world.nom(i,j)+random(500,3000);
                    else
                        peakheight=sealevel-world.nom(i,j)-random(100,200);
                    
                    if (peakheight<10)
                        peakheight=10;
                    
                    peakheight=random(peakheight/2,peakheight);
                }
                else
                {
                    if (strato==1)
                        peakheight=random(2000,6000);
                    else
                        peakheight=random(1000,2000);
                }
                
                //world.settest(i,j,1);
                
                createisolatedvolcano(world,i,j,shelves,volcanodirection,peakheight,strato);
            }
        }
    }

    // Now we shift the map so there is sea at the edges, if possible.
    
    worldgenerationlabel.set_caption("Shifting for best position");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    removeseam(world,0);
    
    adjustforsea(world);
    
    // Now we add smaller mountain chains that cannot form peninsulas.
    
    worldgenerationlabel.set_caption("Adding smaller mountain ranges");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    sf::Vector2i dummy[1];
    
    createchains(world,baseheight,conheight,fractal,plateaumap,landshape,chainland,dummy,0,0,2);
    
    // Now we alter the fractal again, and use it to add more height variation.
    
    worldgenerationlabel.set_caption("Merging fractal into land");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    flip(fractal,width,height,1,1);
    int offset=random(1,width);
    shift(fractal,width,height,offset);
    
    fractalmergeland(world,fractal,conheight);
    
    // Now remove any mountains that are over sea.
    
    worldgenerationlabel.set_caption("Removing floating mountains");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    removefloatingmountains(world);
    cleanmountainridges(world);
    
    // Now we raise the mountain bases.
    
    worldgenerationlabel.set_caption("Raising mountain bases");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    raisemountainbases(world,mountaindrainage);
    
    // Now we smooth again, without changing the coastlines.
    
    worldgenerationlabel.set_caption("Smoothing map, preserving coastlines");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    smoothland(world,2);
    
    vector<vector<int>> slopes(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
            slopes[i][j]=0;
    }
    
    getseaslopes(world,slopes); // Note down all the biggest slopes we currently have. This is so that we don't mark any non-shading areas over these slopes later.
    
    // Now we create extra elevation over the land to create canyons.
    
    worldgenerationlabel.set_caption("Elevating land near canyons");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    createextraelev(world);
    
    // Now we remove depressions.
    
    worldgenerationlabel.set_caption("Filling depressions");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    depressionfill(world);
    
    addlandnoise(world); // Add a bit of noise, then do remove depressions again. This is to add variety to the river courses.
    
    depressionfill(world);
    
    // Now we adjust the land around coastlines.
    
    worldgenerationlabel.set_caption("Adjusting coastlines");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    normalisecoasts(world,13,11,4);
    
    normalisecoasts(world,13,11,4);
    
    clamp(world);
    
    // Now we note down one-tile islands.
    
    worldgenerationlabel.set_caption("Checking islands");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    checkislands(world);
    
    extendnoshade(world);

    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (slopes[i][j]>30)
                world.setnoshade(i,j,0);
        }
    }
    
    // Now we remove odd undersea bumps.
    
    worldgenerationlabel.set_caption("Flattening seabeds");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    //removeunderseabumps(world);
    
    // Now we create a roughness map.
    
    worldgenerationlabel.set_caption("Creating roughness map");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    vector<vector<int>> roughness(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
    
    grain=8; // Level of detail on this fractal map.
    valuemod=0.2;
    valuemod2=0.6;
    
    createfractal(roughness,width,height,grain,valuemod,valuemod2,1,maxelev,0,0);
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
            world.setroughness(i,j,roughness[i][j]);
    }
}

// This function creates a new fractal map.

void createfractal(vector<vector<int>> &arr, int awidth, int aheight, int grain, float valuemod, float valuemod2, int min, int max, bool extreme, bool wrapped)
{
    newfractalinit(arr,awidth,aheight,grain,min,max,extreme);
    
    newfractal(arr,awidth,aheight,grain,valuemod,valuemod2,min,max,wrapped);
}

// Initialises the random values for the fractal map generator.

void newfractalinit(vector<vector<int>> &arr, int awidth, int aheight, int grain, int min, int max, bool extreme)
{
    int s=(aheight+1)/grain;
    
    int newval=0;
    
    for (int i=0; i<=awidth; i=i+s)
    {
        for (int j=0; j<=aheight; j=j+s)
        {
            if (extreme==1)
            {
                if (random(0,1)==1)
                    newval=min;
                
                else
                    newval=max;
            }
            else
                newval=random(min,max);
            
            arr[i][j]=newval;
        }
    }
}

// This is the main fractal generating routine.

void newfractal(vector<vector<int>> &arr, int awidth, int aheight, int grain, float valuemod, float valuemod2, int min, int max, bool wrapped)
{
    bool simple=0;
    
    if (valuemod==valuemod2)
        simple=1;
    
    int newheight=0;
    
    int s=(aheight+1)/grain;
    
    int focalx=awidth/2; // Coordinates for the focal point of the value variance.
    int focaly=random(0,aheight);
    
    //int maxdistance=(sqrt(pow(awidth,2)+pow(aheight,2)))/2; // This is a far away as it's possible to be from a point.
    
    float valuediff=abs(valuemod-valuemod2); // difference between the two valuemods.
    float increment=valuediff/100.0;
    
    valuemod=valuemod*100;
    valuemod2=valuemod2*100;
    
    while (s!=-1) // This loop will repeat, with s halving in size each time until it gets to 1.
    {
        int value=s*valuemod; // This is the amount we will vary each new pixel by. The smaller the tile, the smaller this value should be.
        
        // First we go over the whole map doing the square step.
        
        for (int z=0; z<=awidth+s; z=z+s)
        {
            int i=z;
            
            if (i>awidth)
                i=wrap(i,awidth);
            
            int ss=s/2;
            int ii=i+ss;
            
            if (ii>awidth)
                ii=wrap(ii,awidth);
            
            for (int j=0; j<=aheight; j=j+s)
            {
                int jj=j+ss;
                
                if (simple==0)
                {
                    int focaldistance=sqrt(pow((focalx-ii),2)+pow((focaly-jj),2));
                    int perc=increment*focaldistance;
                    
                    int newval=tilt(valuemod,valuemod2,perc);
                    
                    newval=newval/100.0;
                    value=s*newval;
                }
                
                newheight=square(arr,awidth,aheight,s,ii,jj,value,min,max,wrapped);
                
                if (jj<=aheight)
                    arr[ii][jj]=newheight;
            }
        }
        
        // Now we go over the whole map again, doing the diamond step.
        
        for (int z=0; z<=awidth+s; z=z+s)
        {
            int i=z;
            
            if (i>awidth)
                i=wrap(i,awidth);
            
            int ss=s/2;
            
            int ii=i+ss;
            
            if (ii>awidth)
                ii=wrap(ii,awidth);
            
            for (int j=0; j<=aheight; j=j+s)
            {
                int jj=j+ss;
                
                if (simple==0)
                {
                    int focaldistance=sqrt(pow((focalx-ii),2)+pow((focaly-jj),2));
                    int perc=increment*focaldistance;
                    
                    int newval=tilt(valuemod,valuemod2,perc);
                    
                    newval=newval/100.0;
                    value=s*newval;
                }
                
                for (int n=1; n<=2; n++)
                {
                    int ii=0;
                    int jj=0;
                    
                    if (n==1)
                    {
                        ii=i+ss;
                        jj=j;
                    }
                    
                    if (n==2)
                    {
                        ii=i;
                        jj=j+ss;
                    }
                    
                    if (jj>=0 && jj<=aheight)
                    {
                        //if (ii<0 || ii>awidth)
                        ii=wrap(ii,awidth);
                        
                        newheight=diamond(arr,awidth,aheight,s,ii,jj,value,min,max,wrapped);
                        
                        arr[ii][jj]=newheight;
                    }
                }
            }
        }
        
        if (s>2) // Now halve the size of the tiles.
            s=s/2;
        else
            s=-1;
    }
    
    if (simple==0)
    {
        int offset=random(1,awidth);
        shift(arr,awidth,aheight,offset);
    }
}

// This does the square part of the fractal.

int square(vector<vector<int>> &arr, int awidth, int aheight, int s, int x, int y, int value, int min, int max, bool wrapped)
{
    value=value*50;
    int dist=s/2;
    int total=0;
    int crount=0;
    
    int coords[4][2];
    
    coords[0][0]=x-dist;
    coords[0][1]=y-dist;
    
    coords[1][0]=x+dist;
    coords[1][1]=y-dist;
    
    coords[2][0]=x+dist;
    coords[2][1]=y+dist;
    
    coords[3][0]=x-dist;
    coords[3][1]=y+dist;
    
    if (wrapped==1)
    {
        int values[4];
        
        int nullval=0-max*10;
        
        for (int n=0; n<4; n++)
            values[n]=nullval;
        
        for (int n=0; n<=3; n++)
        {
            if (coords[n][1]>=0 && coords[n][1]<=aheight)
            {
                coords[n][0]=wrap(coords[n][0],awidth);
                
                int x=coords[n][0];
                int y=coords[n][1];
                
                values[n]=arr[x][y];
            }
        }
        
        int a=0;
        
        if (values[0]!=nullval && values[1]!=nullval)
            a=wrappedaverage(values[0],values[1],max);
        else
        {
            if (values[0]==nullval)
                a=values[1];
            else
                a=values[0];
        }
        
        int b=0;
        
        if (values[2]!=nullval && values[3]!=nullval)
            b=wrappedaverage(values[2],values[3],max);
        else
        {
            if (values[2]==nullval)
                b=values[3];
            else
                b=values[2];
        }
        
        if (a!=nullval && b!=nullval)
            total=wrappedaverage(a,b,max);
        else
        {
            if (a==nullval)
                total=b;
            else
                total=a;
        }
    }
    else
    {
        for (int n=0; n<=3; n++)
        {
            if (coords[n][1]>=0 && coords[n][1]<=aheight)
            {
                coords[n][0]=wrap(coords[n][0],awidth);
                
                int x=coords[n][0];
                int y=coords[n][1];
                
                total=total+arr[x][y];
                crount++;
            }
        }
        
        total=total/crount;
    }
    
    int difference=randomsign(random(0,value));
    
    total=total+difference;
    
    if (wrapped==1)
    {
        while (total>max)
            total=total-max;
        
        while (total<min)
            total=total+max;
        
    }
    else
    {
        if (total>max)
            total=max;
        
        if (total<min)
            total=min;
    }
    
    return (total);
}

// This does the diamond part of the fractal.

int diamond(vector<vector<int>> &arr, int awidth, int aheight, int s, int x, int y, int value, int min, int max, bool wrapped)
{
    value=value*50;
    int dist=s/2;
    int total=0;
    int crount=0;
    
    int coords[4][2];
    
    coords[0][0]=x;
    coords[0][1]=y-dist;
    
    coords[1][0]=x+dist;
    coords[1][1]=y;
    
    coords[2][0]=x;
    coords[2][1]=y+dist;
    
    coords[3][0]=x-dist;
    coords[3][1]=y;
    
    if (wrapped==1)
    {
        int values[4];
        
        int nullval=0-max*10;
        
        for (int n=0; n<4; n++)
            values[n]=nullval;
        
        for (int n=0; n<=3; n++)
        {
            if (coords[n][1]>=0 && coords[n][1]<=aheight)
            {
                coords[n][0]=wrap(coords[n][0],awidth);
                
                int x=coords[n][0];
                int y=coords[n][1];
                
                values[n]=arr[x][y];
            }
        }
        
        int a=0;
        
        if (values[0]!=nullval && values[1]!=nullval)
            a=wrappedaverage(values[0],values[1],max);
        else
        {
            if (values[0]==nullval)
                a=values[1];
            else
                a=values[0];
        }
        
        int b=0;
        
        if (values[2]!=nullval && values[3]!=nullval)
            b=wrappedaverage(values[2],values[3],max);
        else
        {
            if (values[2]==nullval)
                b=values[3];
            else
                b=values[2];
        }
        
        if (a!=nullval && b!=nullval)
            total=wrappedaverage(a,b,max);
        else
        {
            if (a==nullval)
                total=b;
            else
                total=a;
        }
    }
    else
    {
        for (int n=0; n<=3; n++)
        {
            if (coords[n][1]>=0 && coords[n][1]<=aheight)
            {
                coords[n][0]=wrap(coords[n][0],awidth);
                
                int x=coords[n][0];
                int y=coords[n][1];
                
                total=total+arr[x][y];
                crount++;
            }
        }
        
        total=total/crount;
    }
    
    int difference=randomsign(random(0,value));
    
    total=total+difference;
    
    if (wrapped==1)
    {
        while (total>max)
            total=total-max;
        
        while (total<min)
            total=total+max;
    }
    else
    {
        if (total>max)
            total=max;
        
        if (total<min)
            total=min;
    }
    
    return (total);
}

// This function creates a new fractal map for the modified map merge.

void createfractalformodifiedmerging(vector<vector<int>> &arr, int awidth, int aheight, int grain, float valuemod, float valuemod2, int min, int max, bool extreme)
{
    newfractalinitformodifiedmerging(arr,awidth,aheight,grain,min,max,extreme);
    
    newfractal(arr,awidth,aheight,grain,valuemod,valuemod2,min,max,0);
}

// Initialises the random values for the fractal map generator for the modified map merge.

void newfractalinitformodifiedmerging(vector<vector<int>> &arr, int awidth, int aheight, int grain, int min, int max, bool extreme)
{
    int maxchance=random(6,12); // The higher this number, the fewer maximum points there will be.
    
    int s=(aheight+1)/grain;
    
    int newval=0;
    
    for (int i=0; i<=awidth; i=i+s)
    {
        for (int j=0; j<=aheight; j=j+s)
        {
            if (random(1,maxchance)==1)
                newval=max;
            else
                newval=min;
            
            arr[i][j]=newval;
        }
    }
}

// This function makes the continents in the original, rather fragmented style.

void smallcontinents(planet &world, int baseheight, int conheight, vector<vector<int>> &fractal, vector<vector<int>> &plateaumap, boolshapetemplate landshape[], boolshapetemplate chainland[])
{
    int width=world.width();
    int height=world.height();
    
    sf::Vector2i focuspoints[4]; // These will be points where continents etc will start close to.
    
    int focustotal=random(2,4); // The number of actual focus points.
    
    for (int n=0; n<focustotal; n++)
    {
        focuspoints[n].x=random(0,width);
        focuspoints[n].y=random(0,height);
    }
    
    int focaldistance=height/2; // Maximum distance a continent can start from the focuspoint.
    
    int maxblobno=width/20; // Number of blobs per continent.
    int minblobno=5;
    int bloboffset=20; // Distance new blobs might be from the last one.
    
    int contno=random((width+1)/1024,(width+1)/64); // Number of continents
    
    if (random(1,10)==1) // Small chance it might have considerably more.
        contno=random((width+1)/1024,(width+1)/64);
    
    int cutno=width/250; // Number of cuts.
    
    // First, make the whole map our seabed baseheight.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
            world.setnom(i,j,baseheight);
    }
    
    // Now, make some mountain chains with associated land.
    
    createchains(world,baseheight,conheight,fractal,plateaumap,landshape,chainland,focuspoints,focustotal,focaldistance,0);
    
    // Now we draw the basic shapes of the continents.

     for (int cont=1; cont<=contno; cont++)
     {
         int thisfocus=random(0,focustotal);
         
         int bx=focuspoints[thisfocus].x;
         int by=focuspoints[thisfocus].y; // Coordinates of the current blob.
         
         float xdiff=random(1,100);
         float ydiff=random(1,100);
         
         xdiff=xdiff/100;
         ydiff=ydiff/100;
         
         xdiff=pow(xdiff,2);
         ydiff=pow(ydiff,2);
         
         xdiff=xdiff*focaldistance;
         ydiff=ydiff*focaldistance;
         
         bx=bx+randomsign(xdiff);
         by=by+randomsign(ydiff);
         
         if (by<0)
         by=0;
         
         if (by>height)
         by=height;
         
         if (bx<0 || bx>width)
         bx=wrap(bx,width);
         
         for (int i=1; i<=random(minblobno,maxblobno); i++) // Create a number of blobs to form this continent.
         {
             for (int j=1; j<=10; j++) // Move the location of the current blob.
             {
                 if (random(1,4)==1)
                 {
                     bx=bx+randomsign(random(1,bloboffset));
                     by=by+randomsign(random(1,bloboffset));
                 }
             }
         
             int shapenumber=random(1,11);
             
             drawshape(world,shapenumber,bx,by,1,baseheight,conheight,landshape);
         }
     }

    // Now for cuts!
    
    cuts(world,cutno,baseheight,conheight,landshape);
    
    // Now we add smaller mountain chains that might form peninsulas.
    
    createchains(world,baseheight,conheight,fractal,plateaumap,landshape,chainland,focuspoints,focustotal,focaldistance,1);
}

// This function makes the continents in a larger style.

void largecontinents(planet &world, nanogui::Screen &screen, nanogui::Window &worldgenerationwindow, nanogui::Label &worldgenerationlabel, nanogui::ProgressBar &worldprogress, float progressstep, int baseheight, int conheight, vector<vector<int>> &fractal, vector<vector<int>> &plateaumap, vector<vector<bool>> &shelves, boolshapetemplate landshape[], boolshapetemplate chainland[])
{
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    int maxelev=world.maxelevation();
    
    int movein=300; // Maximum amount to move neighbouring continents towards the central one.
    int origmountainschance=1; //2; // The higher this is, the less often central continents will have mountains down one side.
    int mountainschance=4; // The higher this is, the less often other continents will have mountains down one side.
    
    worldgenerationlabel.set_caption("Preparing Voronoi map");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    vector<vector<short>> voronoi(width+1,vector<short>(width+1,height+1));
    int points=200; // Number of points in the voronoi map
    
    makevoronoi(voronoi,width,height,points);
    
    vector<vector<bool>> continent(ARRAYWIDTH,vector<bool>(ARRAYWIDTH,ARRAYHEIGHT));
    vector<vector<short>> continentnos(ARRAYWIDTH,vector<short>(ARRAYWIDTH,ARRAYHEIGHT));
    vector<vector<short>> overlaps(ARRAYWIDTH,vector<short>(ARRAYWIDTH,ARRAYHEIGHT));
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            continent[i][j]=0;
            continentnos[i][j]=0;
            overlaps[i][j]=0;
        }
    }
    
    worldgenerationlabel.set_caption("Making continents");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    sf::Vector2i focuspoints[4]; // These will be points where continents etc will start close to.
    
    int focustotal;
    
    if (random(1,10)==1)
        focustotal=1;
    else
        focustotal=2;
    
    if (random(1,3)!=1)
        focustotal=3;

    focuspoints[0].x=width/4;
    focuspoints[0].y=random(height/6,height-height/6);
    
    focuspoints[1].x=width-width/4+randomsign(random(0,width/4));
    focuspoints[1].y=random(height/6,height-height/6);
    
    focuspoints[2].x=random(1,width);
    focuspoints[2].y=random(height/6,height-height/6);
    
    // First, make the whole map our seabed baseheight.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
            world.setnom(i,j,baseheight);
    }

    // Now we draw the basic shapes of the continents, and paste them onto the map.
    
    int leftx=0;
    int rightx=0;
    int lefty=0;
    int righty=0; // Define the size and shape of the current continent.
    
    short thiscontinent=0;
    
    for (int thispoint=0; thispoint<focustotal; thispoint++) // Go through the focus points one by one.
    {
        // First, put the central continent for this grouping onto the map.
        
        thiscontinent++;
        
        makecontinent(world,continent,voronoi,points,width,height,leftx,rightx,lefty,righty);
        
        int origcontwidth=rightx-leftx;
        int origcontheight=righty-lefty;
        
        int origstartpointx=focuspoints[thispoint].x-origcontwidth/2-(width-origcontwidth)/2;
        int origstartpointy=focuspoints[thispoint].y-origcontheight/2-(height-origcontheight)/2; // Coordinates of the top left-hand corner of this continent.
        
        int thisleft=-1;
        int thisright=-1;
        int thisup=-1;
        int thisdown=-1;
        
        bool wrapped=0;
        
        bool leftr=random(0,1); // If it's 1 then we reverse it left-right
        bool downr=random(0,1); // If it's 1 then we reverse it top-bottom
        
        int istart=leftx, desti=rightx, istep=1;
        int jstart=lefty, destj=righty, jstep=1;
        
        if (leftr==1)
        {
            istart=rightx;
            desti=leftx;
            istep=-1;
        }
        
        if (downr==1)
        {
            jstart=righty;
            destj=lefty;
            jstep=-1;
        }
        
        int imap=-1;
        int jmap=-1;
        
        for (int i=istart; i!=desti; i=i+istep)
        {
            imap++;
            jmap=-1;
            
            int ii=origstartpointx+i;
            
            if (ii<0 || ii>width)
            {
                ii=wrap(ii,width);
                wrapped=1;
            }
            
            for (int j=jstart; j!=destj; j=j+jstep)
            {
                jmap++;
                
                int jj=origstartpointy+j;
                
                if (jj>=0 && jj<=height)
                {
                    if (continent[imap+leftx][jmap+lefty]==1)
                    {
                        world.setnom(ii,jj,conheight);
                        //world.settest(ii,jj,1);
                        
                        if (continentnos[ii][jj]!=0)
                        {
                            short overlap=continentnos[ii][jj]*100+thiscontinent;
                            overlaps[ii][jj]=overlap;
                        }

                        continentnos[ii][jj]=thiscontinent;
                        
                        if (ii<thisleft || thisleft==-1)
                            thisleft=ii;
                        
                        if (ii>thisright || thisright==-1)
                            thisright=ii;
                        
                        if (jj<thisup || thisup==-1)
                            thisup=jj;
                        
                        if (jj>thisdown || thisdown==-1)
                            thisdown=jj;
                        
                    }
                }
            }
        }
        
        origstartpointx=thisleft;
        origstartpointy=thisup;
        
        origcontwidth=thisright-thisleft;
        origcontheight=thisdown-thisup;
        
        if (origcontwidth<2 || origcontheight<2)
            wrapped=1;
        
        bool origcontmountains=0;
        
        if (random(1,origmountainschance)==1) // Whether or not this continent has mountains along one edge.
            origcontmountains=1;
        
        short origcontdir=0;
        
        int startnearx=-1;
        int startneary=-1;
        
        int origmountainstartpointx=-1;
        int origmountainstartpointy=-1;
        
        short origstartpoint=0;

        if (wrapped==0)
        {
            origstartpoint=random(1,8);
            
            switch (origstartpoint)
            {
                case 1:
                    startnearx=thisleft+random(1,origcontwidth);
                    startneary=thisup;
                    origcontdir=random(4,6);
                    break;
                    
                case 2:
                    startnearx=thisright;
                    startneary=thisup;
                    origcontdir=random(5,7);
                    break;
                    
                case 3:
                    startnearx=thisright;
                    startneary=thisup+random(1,origcontheight);
                    origcontdir=random(6,8);
                    break;
                    
                case 4:
                    startnearx=thisright;
                    startneary=thisdown;
                    origcontdir=random(7,9);
                    if (origcontdir==9)
                        origcontdir=1;
                    break;
                    
                case 5:
                    startnearx=thisleft+random(1,origcontwidth);
                    startneary=thisdown;
                    origcontdir=random(1,3)-1;
                    if (origcontdir==0)
                        origcontdir=8;
                    break;
                    
                case 6:
                    startnearx=thisleft;
                    startneary=thisdown;
                    origcontdir=random(1,3);
                    break;
                    
                case 7:
                    startnearx=thisleft;
                    startneary=thisup+random(1,origcontheight);
                    origcontdir=random(2,4);
                    break;
                    
                case 8:
                    startnearx=thisleft;
                    startneary=thisup;
                    origcontdir=random(3,5);
                    break;
            }

            if (startnearx<0 || startnearx>width)
                startnearx=wrap(startnearx,width);
            
            if (startneary<0)
                startneary=0;
            
            if (startneary>height)
                startneary=height;
            
            if (origcontmountains==1)                makecontinentedgemountains(world,thiscontinent,continentnos,overlaps,baseheight,conheight,fractal,landshape,chainland,origstartpointx,origstartpointy,origcontwidth,origcontheight,startnearx,startneary,origcontdir,origmountainstartpointx,origmountainstartpointy);
            
        }
        
        // Now put other continents around it.

        bool fringeconts[3][3];
        
        for (int i=0; i<3; i++)
        {
            for (int j=0; j<3; j++)
                fringeconts[i][j]=0;
        }
        
        short extracont=random(1,9)-1; // Number of extra continents.
        
        for (int n=1; n<=extracont; n++)
        {
            thiscontinent++;
            
            makecontinent(world,continent,voronoi,points,width,height,leftx,rightx,lefty,righty);
            
            int thiscontwidth=rightx-leftx;
            int thiscontheight=righty-lefty;
            
            int dir=random(1,8); // Direction from the central continent.
            
            bool keepgoing=1;
            
            switch (dir)
            {
                case 1:
                    if (fringeconts[1][0]==1)
                        keepgoing=0;
                    break;
                    
                case 2:
                    if (fringeconts[2][0]==1)
                        keepgoing=0;
                    break;
                    
                case 3:
                    if (fringeconts[2][1]==1)
                        keepgoing=0;
                    break;
                    
                case 4:
                    if (fringeconts[2][2]==1)
                        keepgoing=0;
                    break;
                    
                case 5:
                    if (fringeconts[1][2]==1)
                        keepgoing=0;
                    break;
                    
                case 6:
                    if (fringeconts[0][2]==1)
                        keepgoing=0;
                    break;
                    
                case 7:
                    if (fringeconts[0][1]==1)
                        keepgoing=0;
                    break;
                    
                case 8:
                    if (fringeconts[0][0]==1)
                        keepgoing=0;
                    break;
            }
            
            if (keepgoing)
            {
                int thisstartpointx=-1;
                int thisstartpointy=-1; // Coordinates of the top left-hand corner of this continent.
                
                switch (dir)
                {
                    case 1: // north
                        thisstartpointx=origstartpointx+randomsign(random(0,origcontwidth/2));
                        thisstartpointy=focuspoints[thispoint].y-thiscontheight/2-(height-thiscontheight)/2-thiscontheight+random(0,movein);
                        fringeconts[1][0]=1;
                        break;
                        
                    case 2: // northeast
                        thisstartpointx=focuspoints[thispoint].x-thiscontwidth/2-(width-thiscontwidth)/2+origcontwidth-random(0,movein);
                        thisstartpointy=focuspoints[thispoint].y-thiscontheight/2-(height-thiscontheight)/2-thiscontheight+random(0,movein);
                        fringeconts[2][0]=1;
                        break;
                        
                    case 3: // east
                        thisstartpointx=focuspoints[thispoint].x-thiscontwidth/2-(width-thiscontwidth)/2+origcontwidth-random(0,movein);
                        thisstartpointy=origstartpointy+randomsign(random(0,origcontheight/2));
                        fringeconts[2][1]=1;
                        break;
                        
                    case 4: // southeast
                        thisstartpointx=focuspoints[thispoint].x-thiscontwidth/2-(width-thiscontwidth)/2+origcontwidth-random(0,movein);
                        thisstartpointy=focuspoints[thispoint].y-thiscontheight/2-(height-thiscontheight)/2+thiscontheight-random(0,movein);
                        fringeconts[2][2]=1;
                        break;
                        
                    case 5: // south
                        thisstartpointx=origstartpointx+randomsign(random(0,origcontwidth/2));
                        thisstartpointy=focuspoints[thispoint].y-thiscontheight/2-(height-thiscontheight)/2+thiscontheight-random(0,movein);
                        fringeconts[1][2]=1;
                        break;
                        
                    case 6: // southwest
                        thisstartpointx=focuspoints[thispoint].x-thiscontwidth/2-(width-thiscontwidth)/2-thiscontwidth+random(0,movein);
                        thisstartpointy=focuspoints[thispoint].y-thiscontheight/2-(height-thiscontheight)/2+thiscontheight-random(0,movein);
                        fringeconts[0][2]=1;
                        break;
                        
                    case 7: // west
                        thisstartpointx=focuspoints[thispoint].x-thiscontwidth/2-(width-thiscontwidth)/2-thiscontwidth+random(0,movein);
                        thisstartpointy=origstartpointy+randomsign(random(0,origcontheight/2));
                        fringeconts[0][1]=1;
                        break;
                        
                    case 8: // northwest
                        thisstartpointx=focuspoints[thispoint].x-thiscontwidth/2-(width-thiscontwidth)/2-thiscontwidth+random(0,movein);
                        thisstartpointy=focuspoints[thispoint].y-thiscontheight/2-(height-thiscontheight)/2-thiscontheight+random(0,movein);
                        fringeconts[0][0]=1;
                        break;
                }
                
                bool leftr=random(0,1); // If it's 1 then we reverse it left-right
                bool downr=random(0,1); // If it's 1 then we reverse it top-bottom
                
                int istart=leftx, desti=rightx, istep=1;
                int jstart=lefty, destj=righty, jstep=1;
                
                if (leftr==1)
                {
                    istart=rightx;
                    desti=leftx;
                    istep=-1;
                }
                
                if (downr==1)
                {
                    jstart=righty;
                    destj=lefty;
                    jstep=-1;
                }
                
                int imap=-1;
                int jmap=-1;
                
                for (int i=istart; i!=desti; i=i+istep)
                {
                    imap++;
                    jmap=-1;
                    
                    int ii=thisstartpointx+i;
                    
                    if (ii<0 || ii>width)
                        ii=wrap(ii,width);
                    
                    for (int j=jstart; j!=destj; j=j+jstep)
                    {
                        jmap++;
                        
                        int jj=thisstartpointy+j;
                        
                        if (jj>=0 && jj<=height)
                        {
                            if (continent[imap+leftx][jmap+lefty]==1)
                            {
                                world.setnom(ii,jj,conheight);
                                
                                if (continentnos[ii][jj]!=0)
                                {
                                    short overlap=continentnos[ii][jj]*100+thiscontinent;
                                    overlaps[ii][jj]=overlap;
                                }
                                
                                continentnos[ii][jj]=thiscontinent;
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Now remove inland seas.
    
    worldgenerationlabel.set_caption("Removing inland seas");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    removeseas(world,conheight);

    worldgenerationlabel.set_caption("Adding continental mountain ranges");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    // Now we add mountain ranges where continents overlap.
    
    int doneoverlaps[50]; // This will note down all the ones we've done.
    
    for (int n=0; n<50; n++)
        doneoverlaps[n]=0;
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (overlaps[i][j]!=0)
            {
                int thisoverlap=overlaps[i][j];
                
                bool donethisone=0;
                
                for (int n=0; n<50; n++)
                {
                    if (doneoverlaps[n]==thisoverlap)
                    {
                        donethisone=1;
                        n=50;
                    }
                }
                
                if (donethisone==0)
                {
                    if (world.outline(i,j)==1)
                    {
                        for (int n=0; n<50; n++)
                        {
                            if (doneoverlaps[n]==0)
                            {
                                doneoverlaps[n]=thisoverlap;
                                n=50;
                            }
                        }
                        
                        int furthestx=-1;
                        int furthesty=-1;
                        int dist=0;
                        
                        for (int k=0; k<=width; k++) // Find the furthest point that's in the same overlap.
                        {
                            for (int l=0; l<=height; l++)
                            {
                                if (overlaps[k][l]==thisoverlap)
                                {
                                    int xdist=i-k;
                                    int ydist=j-l;
                                    
                                    int thisdist=xdist*xdist+ydist+ydist;
                                    
                                    if (thisdist>dist)
                                    {
                                        dist=thisdist;
                                        furthestx=k;
                                        furthesty=l;
                                    }
                                }
                            }
                        }
                        
                        if (dist!=0)
                        {
                            vector <sf::Vector2i> dummy1(2);
                            
                            vector<vector<bool>> dummy2(2,vector<bool>(2,2));
                            
                            createdirectedchain(world,baseheight,conheight,1,continentnos,fractal,landshape,chainland,i,j,furthestx,furthesty,0,dummy1,dummy2,200);

                        }
                    }
                }
            }
        }
    }
    
    sf::Vector2i mountainfocuspoints[4]; // These will be points where mountains and islands will start close to.
    
    int mountainfocustotal=random(2,4); // The number of actual focus points.
    
    for (int n=0; n<mountainfocustotal; n++)
    {
        mountainfocuspoints[n].x=random(0,width);
        mountainfocuspoints[n].y=random(0,height);
    }
    
    int mountainfocaldistance=height/2;

    // Now we add smaller mountain chains that might form peninsulas.
    
    createchains(world,baseheight,conheight,fractal,plateaumap,landshape,chainland,mountainfocuspoints,mountainfocustotal,mountainfocaldistance,1);
    
    // Now we do the continental shelves.
    
    worldgenerationlabel.set_caption("Making continental shelves");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    makecontinentalshelves(world,shelves,4);
    
    // Now we add some island chains.
    
    int islandgroups=random(0,6); // Number of general groups of islands.
    
    for (int n=1; n<=islandgroups; n++)
    {
        mountainfocustotal=1; // The number of actual focus points.
        
        for (int n=0; n<mountainfocustotal; n++)
        {
            mountainfocuspoints[n].x=random(0,width);
            mountainfocuspoints[n].y=random(0,height);
        }
        
        mountainfocaldistance=random(height/8,height/4);
        createchains(world,baseheight,conheight,fractal,plateaumap,landshape,chainland,mountainfocuspoints,mountainfocustotal,mountainfocaldistance,3);
    }
}

// This function creates a chain of mountains along the edge of a continent.

void makecontinentedgemountains(planet &world, short thiscontinent, vector<vector<short>> &continentnos, vector<vector<short>> &overlaps, int baseheight, int conheight, vector<vector<int>> &fractal, boolshapetemplate landshape[], boolshapetemplate chainland[], int contstartpointx, int contstartpointy, int contwidth, int contheight, int startnearx, int startneary, short contdir, int &startpointx, int &startpointy)
{
    int width=world.width();
    int height=world.height();
    
    // First we need to work out the starting point of the chain. It will be the point of the continent that's closest to the startnearx and startneary point.
    
    startpointx=-1;
    startpointy=-1;
    
    int closestdist=width*width+height*height;
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (continentnos[i][j]==thiscontinent && world.sea(i,j)==0)
            {
                int widthdiff=i-startnearx;
                int heightdiff=j-startneary;
                
                int thisdist=widthdiff*widthdiff+heightdiff*heightdiff;
                
                if (thisdist<closestdist)
                {
                    closestdist=thisdist;
                    
                    startpointx=i;
                    startpointy=j;
                }
            }
        }
    }
    
    if (startpointx==-1)
        return;
    
    // Now we work out the end point of the chain.
    
    int endnearx=-1;
    int endneary=-1;
    
    if (contdir==6 || contdir==7 || contdir==8)
        endnearx=contstartpointx;
    
    if (contdir==1 || contdir==5)
        endnearx=contstartpointx+contwidth/2;
    
    if (contdir==2 || contdir==3 || contdir==4)
        endnearx=contstartpointx+contwidth;
    
    if (contdir==8 || contdir==1 || contdir==2)
        endneary=contstartpointy;
    
    if (contdir==3 || contdir==7)
        endneary=contstartpointy+contheight/2;
    
    if (contdir==4 || contdir==5 || contdir==6)
        endneary=contstartpointy+contheight;
    
    int endpointx=-1;
    int endpointy=-1;
    
    closestdist=width*width+height*height;
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (continentnos[i][j]==thiscontinent && world.sea(i,j)==0)
            {
                int widthdiff=i-endnearx;
                int heightdiff=j-endneary;
                
                int thisdist=widthdiff*widthdiff+heightdiff*heightdiff;
                
                if (thisdist<closestdist)
                {
                    closestdist=thisdist;
                    
                    endpointx=i;
                    endpointy=j;
                }
            }
        }
    }
    
    if (endpointx==-1)
        return;

    // Now create the ranges.

    vector <sf::Vector2i> rangepoints(1000); // This will hold the start points of each of the ranges in this chain.
    
    for (int n=0 ; n<1000; n++)
    {
        rangepoints[n].x=-1;
        rangepoints[n].y=-1;
    }
    
    vector<vector<bool>> currentland(width+1,vector<bool>(width+1,height+1)); // This will show where new land has been placed around the mountains.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
            currentland[i][j]=0;
    }
    
    int volcanochance=random(40,120);

    createdirectedchain(world,baseheight,conheight,thiscontinent,continentnos,fractal,landshape,chainland,startpointx,startpointy,endpointx,endpointy,1,rangepoints,currentland,volcanochance);
    
    // Now we draw a line between all the points where ranges started.
    
    rangepoints[0].x=startnearx;
    rangepoints[0].y=startneary;
    
    int lastpoint=0;
    
    for (int n=0; n<1000; n++)
    {
        if (rangepoints[n].x==-1)
        {
            lastpoint=n;
            n=1000;
        }
    }
    
    rangepoints[lastpoint].x=endnearx;
    rangepoints[lastpoint].y=endneary;
    

    for (int n=0; n<1000; n++)
    {
        if (rangepoints[n].x==-1)
            n=1000;
    }
    
    if (lastpoint<3)
        return;
    
    vector<vector<bool>> landmask(width+1,vector<bool>(width+1,height+1)); // This will show where the mountains divide the continent into two.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
            landmask[i][j]=0;
    }
    
    sf::Vector2f pt, mm1, mm2, mm3, mm4;

    mm1.x=rangepoints[0].x;
    mm1.y=rangepoints[0].y;
    
    mm2.x=rangepoints[0].x;
    mm2.y=rangepoints[0].y;
    
    mm3.x=rangepoints[1].x;
    mm3.y=rangepoints[1].y;
    
    mm4.x=rangepoints[2].x;
    mm4.y=rangepoints[2].y;
    
    for (float t=0.0; t<=1.0; t=t+0.01)
    {
        pt=curvepos(mm1,mm2,mm3,mm4,t);
        
        int x=pt.x;
        int y=pt.y;
        
        if (x<0 || x>width)
            x=wrap(x,width);
        
        if (y>=0 && y<=height)
            landmask[x][y]=1;
    }
    
    mm1.x=rangepoints[lastpoint-2].x;
    mm1.y=rangepoints[lastpoint-2].y;
    
    mm2.x=rangepoints[lastpoint-1].x;
    mm2.y=rangepoints[lastpoint-1].y;
    
    mm3.x=rangepoints[lastpoint].x;
    mm3.y=rangepoints[lastpoint].y;
    
    mm4.x=rangepoints[lastpoint].x;
    mm4.y=rangepoints[lastpoint].y;
    
    for (float t=0.0; t<=1.0; t=t+0.01)
    {
        pt=curvepos(mm1,mm2,mm3,mm4,t);
        
        int x=pt.x;
        int y=pt.y;
        
        if (x<0 || x>width)
            x=wrap(x,width);
        
        if (y>=0 && y<=height)
            landmask[x][y]=1;
    }

    for (int n=0; n<=lastpoint-3; n++)
    {
        mm1.x=rangepoints[n].x;
        mm1.y=rangepoints[n].y;
        
        mm2.x=rangepoints[n+1].x;
        mm2.y=rangepoints[n+1].y;
        
        mm3.x=rangepoints[n+2].x;
        mm3.y=rangepoints[n+2].y;
        
        mm4.x=rangepoints[n+3].x;
        mm4.y=rangepoints[n+3].y;

        /*
        
        if (mm2.x<mm1.x) // This dewraps them all, so some may extend beyond the eastern edge of the map.
            
            mm2.x=mm2.x+width;
        
        if (mm3.x<mm2.x)
            mm3.x=mm3.x+width;
        
        if (mm4.x<mm3.x)
            mm4.x=mm4.x+width;
        
        */
        
        for (float t=0.0; t<=1.0; t=t+0.01)
        {
            pt=curvepos(mm1,mm2,mm3,mm4,t);
            
            int x=pt.x;
            int y=pt.y;
            
            if (x<0 || x>width)
                x=wrap(x,width);

            if (y>=0 && y<=height)
                landmask[x][y]=1;
        }
    }
    
    // Now we need to fill one side of that line.
    
    int startfillpointx=contstartpointx+contwidth/2;
    int startfillpointy=contstartpointy+contheight/2;
    
    while (landmask[startfillpointx][startfillpointy]==1)
    {
        startfillpointx--;
        
        if (startfillpointx<0)
            startfillpointx=width;
    }
    
    fillcontinent (landmask,continentnos,thiscontinent,width,height,startfillpointx,startfillpointy,1);
    
    // Now work out which side of the line is smaller. That will be the side to delete.
    
    int filledside=0;
    int blankside=0;
    
    for (int i=contstartpointx; i<=contstartpointx+contwidth; i++)
    {
        for (int j=contstartpointy; j<=contstartpointy+contheight; j++)
        {
            if (j>=0 && j<=height)
            {
                if (continentnos[i][j]==thiscontinent)
                {
                    if (landmask[i][j]==1)
                        filledside++;
                    else
                        blankside++;
                    
                }
            }
        }
    }
    
    // Now delete all land (other than that immediately around mountains) from whichever side is smaller.
    
    for (int i=contstartpointx; i<=contstartpointx+contwidth; i++)
    {
        for (int j=contstartpointy; j<=contstartpointy+contheight; j++)
        {
            if (j>=0 && j<=height)
            {
                if (continentnos[i][j]==thiscontinent && currentland[i][j]==0)
                {
                    if (filledside<blankside)
                    {
                        if (landmask[i][j]==1)
                            world.setnom(i,j,baseheight);
                    }
                    else
                    {
                        if (landmask[i][j]==0)
                            world.setnom(i,j,baseheight);
                    }
                }
            }
        }
    }
}

// This function creates a voronoi map.

void makevoronoi(vector<vector<short>> &voronoi, int width, int height, int points)
{
    vector<vector<int>> pointslist(points,vector<int>(points,2));
    
    int margin=points/6;
    
    int widthmargin=width/10;
    int widthfarmargin=width-widthmargin;
    int heightmargin=height/10;
    int heightfarmargin=height-heightmargin;
    
    for (int n=0; n<=margin; n=n+3) // Force it to put some points near the edges.
    {
        if (random(1,2)==1)
        {
            pointslist[n][0]=random(0,widthmargin);
            pointslist[n][1]=random(0,height);
        }
        else
        {
            pointslist[n][0]=random(widthfarmargin,width);
            pointslist[n][1]=random(0,height);
        }
        
        pointslist[n+1][0]=random(0,width);
        pointslist[n+1][1]=random(0,heightmargin);
        
        pointslist[n+2][0]=random(0,width);
        pointslist[n+2][1]=random(heightfarmargin,height);
        
    }
    
    for (int n=margin+1; n<points; n++)
    {
        pointslist[n][0]=random(0,width);
        pointslist[n][1]=random(0,height);
    }
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            int closest=0;
            int dist=10000000;
            
            for (int point=0; point<points; point++)
            {
                int xdist=i-pointslist[point][0];
                int ydist=j-pointslist[point][1];
                int pointdist=xdist*xdist+ydist*ydist;
                
                if (pointdist<dist)
                {
                    closest=point;
                    dist=pointdist;
                }
            }
            
            voronoi[i][j]=closest;
        }
    }
}

// This does the same thing, for the continental shelves.

void makeshelvesvoronoi(planet &world, vector<vector<short>> &voronoi, vector<vector<bool>> &outline, int pointdist)
{
    int width=world.width();
    int height=world.height();
    int maxelev=world.maxelevation();
    int midelev=maxelev/2;
    
    int grain=16; // Level of detail on this fractal map.
    float valuemod=0.4;
    int v=random(3,6);
    float valuemod2=v;
    
    vector<vector<int>> fractal(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
    
    createfractal(fractal,width,height,grain,valuemod,valuemod2,1,maxelev,0,0);
    
    int warpdist=1;
    int warpdiv=midelev/warpdist;

    vector<vector<int>> pointsmap(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
            pointsmap[i][j]=0;
    }
    
    //int pointdist=4;
    int pointnumber=1;
    int movedist=(pointdist/4)*3; //3;
    
    for (int i=0; i<=width; i=i+pointdist)
    {
        for (int j=0; j<=height; j=j+pointdist)
        {
            int ii=i+randomsign(random(1,movedist));
            int jj=j+randomsign(random(1,movedist));
            
            if (ii<0 || ii>width)
                ii=wrap(ii,width);
            
            if (jj<0)
                jj=0;
            
            if (jj>height)
                jj=height;
            
            pointsmap[ii][jj]=pointnumber;
            pointnumber++;
        }
    }
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            int closest=0;
            int dist=10000000;
            
            int warpdistx=0;
            int warpdisty=0;
            
            int warpx=i;
            int warpy=j;
            
            if (fractal[warpx][warpy]>maxelev/2)
                warpdistx=(fractal[warpx][warpy]-midelev)/warpdiv;
            else
                warpdistx=0-(midelev-fractal[warpx][warpy])/warpdiv;

            int checkx=i+warpdistx;
            
            if (checkx<0 || checkx>width)
                wrap(checkx,width);
            
            warpx=warpx+width/2;
            
            if (warpx>width)
                warpx=warpx-width;
            
            if (fractal[warpx][warpy]>maxelev/2)
                warpdisty=(fractal[warpx][warpy]-midelev)/warpdiv;
            else
                warpdisty=0-(midelev-fractal[warpx][warpy])/warpdiv;
            
            int checky=j+warpdisty;
            
            if (checky<0)
                checky=0;
            
            if (checky>height)
                checky=height;
            
            checkx=i;
            checky=j;
            
            int distcheck=10;
            
            do
            {
                for (int x=checkx-distcheck; x<=checkx+distcheck; x++)
                {
                    int xx=x;
                    
                    if (xx<0 || xx>width)
                        xx=wrap(xx,width);
                    
                    for (int y=checky-distcheck; y<=checky+distcheck; y++)
                    {
                        if (y>=0 && y<=height)
                        {
                            if (pointsmap[xx][y]!=0)
                            {
                                int xdist=checkx-xx;
                                int ydist=checky-y;
                                int pointdist=xdist*xdist+ydist*ydist;
                                
                                if (pointdist<dist)
                                {
                                    closest=pointsmap[xx][y];
                                    dist=pointdist;
                                }
                            }
                        }
                    }
                }
                
                distcheck=distcheck+10;
                
            } while (closest==0);
            
            voronoi[i][j]=closest;
        }
    }
}

// This function draws a (large-style) continent onto an array.

void makecontinent(planet &world, vector<vector<bool>> &continent, vector<vector<short>> &voronoi, int points, int width, int height, int &leftx, int &rightx, int &lefty, int &righty)
{
    vector<vector<bool>> circlemap(width+1,vector<bool>(width+1,height+1));
    vector<vector<bool>> outline(width+1,vector<bool>(width+1,height+1));
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            circlemap[i][j]=0;
            outline[i][j]=0;
        }
    }
    
    bool cells[points];
    
    for (int n=0; n<points; n++)
        cells[n]=0;
    
    int totalnodes=1;
    
    vector <sf::Vector2i> bordernodes; // Stores the location of the nodes.
    vector <sf::Vector2i> nodeshifts; // Stores how much each node has been shifted from its default location.
    
    int circleno;
    
    if (random(1,3)==1)
        circleno=random(20,100);
    else
        circleno=random(5,50);
    
    int circlesize;
    
    circlesize=random(height/30,height/12);
    
    // First, draw a load of blobs to get an extremely rough shape.
    
    int circleleftx=width/2;
    int circlerightx=width/2;
    int circlelefty=height/2;
    int circlerighty=height/2;

    makecontinentcircles(circlemap,width,height,circleno,circlesize,circleleftx,circlerightx,circlelefty,circlerighty);

    // Shift the circles, so that not all continents are made with the same Voronoi cells.
    
    int circleswidth=circlerightx-circleleftx;
    int circlesheight=circlerighty-circlelefty;
    
    int rightshift=0;
    int downshift=0;

    if (circleswidth<width-width/5 && circlesheight<height-height/5)
    {
        int leftcirclestart=random(width/10+circleswidth,width-width/10-circleswidth*2);
        
        rightshift=leftcirclestart-circleleftx;
    }

    // Now, take the Voronoi cells that overlap those blobs, to get the rough continent.
    
    for (int i=circleleftx; i<=circlerightx; i++)
    {
        for (int j=circlelefty; j<=circlerighty; j++)
        {
            if (circlemap[i][j]==1)
            {
                int ii=i+rightshift;
                int jj=j+downshift;
                
                if (ii>=0 && ii<=width && jj>=0 && jj<=height)
                    cells[voronoi[ii][jj]]=1;
            }
        }
    }
    
    for (int i=circleleftx; i<=circlerightx; i++)
    {
        for (int j=circlelefty; j<=circlerighty; j++)
        {
            int ii=i+rightshift;
            int jj=j+downshift;
            
            if (ii<0 || ii>width || jj<0 || jj>height)
                return;
            
            if (cells[voronoi[ii][jj]]==1)
                continent[i][j]=1;
        }
    }
    
    // Now we need to identify the outline.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (continent[i][j]==1)
            {
                bool found=0;
                
                for (int k=i-1; k<=i+1; k++)
                {
                    for (int l=j-1; l<=j+1; l++)
                    {
                        if (k>=0 && k<=width && l>=0 && l<=height && continent[k][l]==0)
                        {
                            found=1;
                            k=i+1;
                            l=j+1;
                        }
                    }
                }
                
                if (found)
                    outline[i][j]=1;

            }
        }
    }
    
    // Now we need to disrupt that outline to make it interesting.
    
    totalnodes=1;
    short minnodegap=5;
    short maxnodegap=10; //15;
    int nodegap=random(minnodegap,maxnodegap); //(6,8); // This is the number of positions to skip when setting up the initial nodes.
    short gapvarychance=2; // The distance between nodes may vary as it goes round!
    
    // First, find the first node.
    
    int x=width/2;
    int y=height/2;
    
    bool keepgoing=1;
    
    bool mainlyup=random(1,2)-1;
    
    short up=randomsign(1);
    short left=randomsign(1);
    
    short uprandom=random(1,5);
    short leftrandom=random(1,5);
    
    do
    {
        if (mainlyup)
        {
            y=y+up;
            
            if (random(1,leftrandom)==1)
                x=x+left;
        }
        else
        {
            x=x+left;
            
            if (random(1,uprandom)==1)
                y=y+up;
        }
        
        if (x<0 || x>width || y<0 || y>width)
            return;
        
        if (outline[x][y]==1)
            keepgoing=0;
        
        
    } while (keepgoing);

    // Now get the list of nodes ready.
    
    sf::Vector2i node;
    node.x=x;
    node.y=y;
    
    sf::Vector2i shift;
    shift.x=0;
    shift.y=0;
    
    bordernodes.push_back(node); // Put the first node onto our list.
    nodeshifts.push_back(shift);
    
    keepgoing=1;
    int crount=0;
    
    do
    {
        // First, remove the point of outline that we're on.
        
        outline[node.x][node.y]=0;
        
        // Next, we need to find the next one to move to.
        
        bool found=0;
        
        if (node.y>0 && outline[node.x][node.y-1]==1)
        {
            node.y--;
            found=1;
        }
        
        if (found==0 && node.x>0 && outline[node.x-1][node.y]==1)
        {
            node.x--;
            found=1;
        }
        
        if (found==0 && node.y<width && outline[node.x][node.y+1]==1)
        {
            node.y++;
            found=1;
        }
        
        if (found==0 && node.x<width && outline[node.x+1][node.y]==1)
        {
            node.x++;
            found=1;
        }
        
        if (found==0 && node.x>0 && node.y>0 && outline[node.x-1][node.y-1]==1)
        {
            node.x--;
            node.y--;
            found=1;
        }
        
        if (found==0 && node.x<width && node.y>0 && outline[node.x+1][node.y-1]==1)
        {
            node.x++;
            node.y--;
            found=1;
        }
        
        if (found==0 && node.x>0 && node.y<height && outline[node.x-1][node.y+1]==1)
        {
            node.x--;
            node.y++;
            found=1;
        }
        
        if (found==0 && node.x<width && node.y<height && outline[node.x+1][node.y+1]==1)
        {
            node.x++;
            node.y++;
            found=1;
        }
        
        if (found==0) // If we haven't found one, try looking a little further just in case.
        {
            for (int i=-2; i<=2; i++)
            {
                int ii=node.x+i;
                
                for (int j=-2; j<=2; j++)
                {
                    int jj=node.y+j;
                    
                    if (ii>=0 && ii<=width && jj>=0 && jj<=height)
                    {
                        if (outline[ii][jj]==1 && (ii!=bordernodes[0].x || jj!=bordernodes[0].y))
                        {
                            node.x=ii;
                            node.y=jj;
                            found=1;
                        }
                    }
                }
            }
        }
        
        if (found==0) // If we still haven't found one, try looking even further just in case.
        {
            for (int i=-4; i<=4; i++)
            {
                int ii=node.x+i;
                
                for (int j=-4; j<=4; j++)
                {
                    int jj=node.y+j;
                    
                    if (ii>=0 && ii<=width && jj>=0 && jj<=height)
                    {
                        if (outline[ii][jj]==1 && (ii!=bordernodes[0].x || jj!=bordernodes[0].y))
                        {
                            node.x=ii;
                            node.y=jj;
                            found=1;
                        }
                    }
                }
            }
        }

        if (found==0) // If we STILL didn't find one, we must have got to the end of the outline.
            keepgoing=0;
        else
        {
            crount++;
            
            if (crount==nodegap) // If this is one to turn into a node
            {
                bordernodes.push_back(node);
                nodeshifts.push_back(shift); // This is just blank for now. We'll use it when we apply offsets.
                totalnodes++;
                crount=0;

                if (random(1,gapvarychance)==1)
                {
                    nodegap=nodegap+randomsign(1);
                    
                    if (nodegap<minnodegap)
                        nodegap=minnodegap;
                    
                    if (nodegap>maxnodegap)
                        nodegap=maxnodegap;
                }
            }
        }
        
    } while (keepgoing==1);
    
    if (totalnodes==1) // Sometimes it just gets a rogue one for some reason
        return;
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
            continent[i][j]=0;
    }
    
    int maxdiff=120; //80;
    
    if (abs(bordernodes[totalnodes-1].x-bordernodes[0].x)>maxdiff)
        return;
    
    if (abs(bordernodes[totalnodes-1].y-bordernodes[0].y)>maxdiff)
        return;

    // Now apply offsets to our nodes.
    
    int maxoffset=40; //25; // No node can be offset further than this.
    
    int maxoffsetchange=random(5,10); // No offset can be changed by more than this.
    
    int xoffset=0;
    int yoffset=0; // These are the amounts to offset the node by.
    
    int xoffsetchange=randomsign(random(0,maxoffsetchange));
    int yoffsetchange=randomsign(random(0,maxoffsetchange)); // These are the amounts to change the offset by each turn.
    
    int minmaxaddoffset=2;
    int maxmaxaddoffset=6; //15;
    short maxaddoffsetchangechance=2; // This can change as it goes round.
    
    int maxaddoffset=random(minmaxaddoffset,maxmaxaddoffset); // Maximum additional offset a node can have.
    
    for (int node=0; node<totalnodes-1; node++)
    {
        if (random(1,maxaddoffsetchangechance)==1)
        {
            maxaddoffset=maxaddoffset+randomsign(1);
            
            if (maxaddoffset<minmaxaddoffset)
                maxaddoffset=minmaxaddoffset;
            
            if (maxaddoffset>maxmaxaddoffset)
                maxaddoffset=maxmaxaddoffset;
        }
        
        int x=bordernodes[node].x;
        int y=bordernodes[node].y;
        
        x=x+xoffset+randomsign(random(0,maxaddoffset));
        y=y+yoffset+randomsign(random(0,maxaddoffset));
        
        if (x<bordernodes[node].x-maxoffset)
            x=bordernodes[node].x-maxoffset;
        
        if (x>bordernodes[node].x+maxoffset)
            x=bordernodes[node].x+maxoffset;
        
        if (y<bordernodes[node].y-maxoffset)
            y=bordernodes[node].y-maxoffset;
        
        if (y>bordernodes[node].y+maxoffset)
            y=bordernodes[node].y+maxoffset;
        
        nodeshifts[node].x=x-bordernodes[node].x;
        nodeshifts[node].y=y-bordernodes[node].y;
        
        bordernodes[node].x=x;
        bordernodes[node].y=y;
        
        // Change the offsets. Make them tend away from the extremes to ensure that they keep varying.
        
        if (xoffset>0)
        {
            if (random(0,maxoffset)<=xoffset)
                xoffsetchange=xoffsetchange-random(0,maxoffsetchange);
            else
                xoffsetchange=xoffsetchange+random(0,maxoffsetchange);
        }
        else
        {
            if (random(0,maxoffset)<=abs(xoffset))
                xoffsetchange=xoffsetchange+random(0,maxoffsetchange);
            else
                xoffsetchange=xoffsetchange-random(0,maxoffsetchange);
        }
        
        if (yoffset>0)
        {
            if (random(0,maxoffset)<yoffset)
                yoffsetchange=yoffsetchange-random(0,maxoffsetchange);
            else
                yoffsetchange=yoffsetchange+random(0,maxoffsetchange);
        }
        else
        {
            if (random(0,maxoffset)<abs(yoffset))
                yoffsetchange=yoffsetchange+random(0,maxoffsetchange);
            else
                yoffsetchange=yoffsetchange-random(0,maxoffsetchange);
        }
        
        xoffsetchange=xoffsetchange+randomsign(random(0,maxoffsetchange));
        yoffsetchange=yoffsetchange+randomsign(random(0,maxoffsetchange));
        
        if (xoffsetchange>maxoffsetchange)
            xoffsetchange=maxoffsetchange;
        
        if (xoffsetchange<-maxoffsetchange)
            xoffsetchange=-maxoffsetchange;
        
        if (yoffsetchange>maxoffsetchange)
            yoffsetchange=maxoffsetchange;
        
        if (yoffsetchange<-maxoffsetchange)
            yoffsetchange=-maxoffsetchange;
        
        xoffset=xoffset+xoffsetchange;
        yoffset=yoffset+yoffsetchange;
        
        if (xoffset>maxoffset)
            xoffset=maxoffset;
        
        if (xoffset<-maxoffset)
            xoffset=-maxoffset;
        
        if (yoffset>maxoffset)
            yoffset=maxoffset;
        
        if (yoffset<-maxoffset)
            yoffset=-maxoffset;
    }
    
    // Make the last node midway between the penultimate and the first.
    
    bordernodes[totalnodes-1].x=(bordernodes[0].x+bordernodes[totalnodes-2].x)/2;
    bordernodes[totalnodes-1].y=(bordernodes[0].y+bordernodes[totalnodes-2].y)/2;
    
    nodeshifts[totalnodes-1].x=(nodeshifts[0].x+nodeshifts[totalnodes-2].x)/2;
    nodeshifts[totalnodes-1].y=(nodeshifts[0].y+nodeshifts[totalnodes-2].y)/2;

    // Now we've got all our nodes. We need to draw splines between them.
    
    int midvar=2; // Possible variation for the interpolated nodes. This is a fraction of the difference between the two nodes being interpolated, not an absolute value.
    
    sf::Vector2f pt, mm1, mm2, mm3;
    
    int nodeshiftedx=0;
    int nodeshiftedy=0; // This tells us how much the interpolated node is already shifted by. It's just the average of how much the other nodes are shifted by.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
            outline[i][j]=0;
    }
    
    leftx=width/2;
    rightx=leftx;
    lefty=height/2;
    righty=lefty;
    
    bool stopping=0;
    
    int minmaxswerve=1;
    int maxmaxswerve=5;
    short maxswervechangechance=2;
    
    int maxswerve=random(minmaxswerve,maxmaxswerve);
    
    for (int node=0; node<totalnodes; node++) // We'll draw this on the outline array.
    {
        if (random(1,maxswervechangechance)==1)
        {
            maxswerve=maxswerve+randomsign(1);
            
            if (maxswerve<minmaxswerve)
                maxswerve=minmaxswerve;
            
            if (maxswerve>maxmaxswerve)
                maxswerve=maxmaxswerve;
        }
        
        if (node==totalnodes-1) // If we've run out of nodes to do, stop after this one
        {
            nodeshiftedx=(nodeshifts[node].x+nodeshifts[0].x)/2;
            nodeshiftedy=(nodeshifts[node].y+nodeshifts[0].y)/2;
            
            mm1.x=bordernodes[node].x;
            mm1.y=bordernodes[node].y;
            
            mm3.x=bordernodes[0].x;
            mm3.y=bordernodes[0].y;
            
            stopping=1;
        }
        else
        {
            nodeshiftedx=(nodeshifts[node].x+nodeshifts[node+1].x)/2;
            nodeshiftedy=(nodeshifts[node].y+nodeshifts[node+1].y)/2;
            
            mm1.x=bordernodes[node].x;
            mm1.y=bordernodes[node].y;
            
            mm3.x=bordernodes[node+1].x;
            mm3.y=bordernodes[node+1].y;
        }
        
        int xswerve=random(1,maxswerve);
        int yswerve=random(1,maxswerve);
        
        // Add the swerve to the interpolated point
        
        mm2.x=(mm1.x+mm3.x)/2+xswerve;
        mm2.y=(mm1.y+mm3.y)/2+yswerve;
        
        // Do the actual line
        
        for (float t=0.0; t<=1.0; t=t+0.01)
        {
            pt=curvepos(mm1,mm1,mm2,mm3,t);
            
            int x=pt.x;
            int y=pt.y;
            
            if (x>=0 && x<=width && y>=0 && y<=height)
            {
                outline[x][y]=1;
                
                if (x<leftx)
                    leftx=x;
                
                if (x>rightx)
                    rightx=x;
                
                if (y<lefty)
                    lefty=y;
                
                if (y>righty)
                    righty=y;
            }
        }
        
        for (float t=0.0; t<=1.0; t=t+0.01)
        {
            pt=curvepos(mm1,mm2,mm3,mm3,t);
            
            int x=pt.x;
            int y=pt.y;
            
            if (x>=0 && x<=width && y>=0 && y<=height)
            {
                outline[x][y]=1;
                
                if (x<leftx)
                    leftx=x;
                
                if (x>rightx)
                    rightx=x;
                
                if (y<lefty)
                    lefty=y;
                
                if (y>righty)
                    righty=y;
            }
        }
        
        if (stopping==1)
            node=totalnodes;
    }
    
    leftx=leftx-2;
    rightx=rightx+2;
    lefty=lefty-2;
    righty=righty+2;
    
    if (leftx<0)
        leftx=0;

    if (rightx>width)
        rightx=width;
    
    if (lefty<0)
        lefty=0;
    
    if (righty>height)
        righty=height;
    
    // Now, draw a border around the continent.
    
    for (int i=leftx; i<=rightx; i++)
    {
        if (i>=0 && i<=width)
        {
            if (lefty>=0 && lefty<=height)
                outline[i][lefty]=1;
            
            if (righty>=0 && righty<=height)
                outline[i][righty]=1;
        }
    }
    
    for (int j=lefty; j<=righty; j++)
    {
        if (j>=0 && j<=height)
        {
            if (leftx>=0 && leftx<=width)
                outline[leftx][j]=1;
            
            if (rightx>=0 && rightx<=width)
                outline[rightx][j]=1;
        }
    }
    
    // Now fill within that border.

    fill(outline,width,height,leftx+1,lefty+1,1);
    fill(outline,width,height,rightx-1,lefty+1,1);
    fill(outline,width,height,rightx-1,righty-1,1);
    fill(outline,width,height,leftx+1,righty-1,1);
    
    for (int i=leftx; i<=rightx; i++)
    {
        for (int j=lefty; j<=righty; j++)
        {
            if (outline[i][j]==0)
                continent[i][j]=1;
            else
                continent[i][j]=0;
        }
    }
    
    // Now remove any odd bits of sea within the continent.
    
    for (int i=leftx+1; i<rightx; i++)
    {
        for (int j=lefty+1; j<righty; j++)
        {
            if (continent[i][j]==0)
            {
                short crount=0;
                
                for (int k=i-1; k<=i+1; k++)
                {
                    for (int l=j-1; l<=j+1; l++)
                    {
                        if (continent[k][l]==0)
                            crount++;
                    }
                }
                
                if (crount<5)
                    continent[i][j]=1;
            }
        }
    }
    
    // Now rotate! There's a tendency for some of these continents to come out a bit rectangular, so rotating them helps to conceal that.
    
    float distort;
    
    if (random(1,2)==1)
        distort=random(150,400);
    else
        distort=random(10,150);
    
    float distortvary;
    
    if (random(1,8)==1)
        distortvary=distort;
    else
    {
        if (random(1,4)==1)
            distortvary=distort*4;
        else
            distortvary=distort*2;
    }
    
    float distortstep=world.maxelevation()/distort;
    
    int grain=8; // Level of detail on this fractal map.
    float valuemod=0.2;
    
    vector<vector<int>> fractal(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
    
    createfractal(fractal,world.width(),world.height(),grain,valuemod,valuemod,1,world.maxelevation(),0,0);
    
    for (int i=leftx; i<=rightx; i++)
    {
        for (int j=lefty; j<=righty; j++)
        {
            outline[i][j]=continent[i][j];
            continent[i][j]=0;
        }
    }
    
    int originx=(leftx+rightx)/2;
    int originy=(lefty+righty)/2;
    
    short angle=random(25,65);
    leftx=width/2;
    rightx=width/2;
    lefty=height/2;
    righty=height/2;
    
    bool failed=0;
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            float thisdistort=fractal[i][j];
            
            thisdistort=(thisdistort/distortstep)-distort/2;
            
            if (thisdistort>0)
            {
                thisdistort=thisdistort/distortvary;
                
                thisdistort=thisdistort*thisdistort;
                
                thisdistort=thisdistort*distortvary;
            }
            else
                thisdistort=0;

            int ii=i-originx+thisdistort;
            int jj=j-originy+thisdistort;
            
            int newi=ii*cos(angle)-jj*sin(angle);
            int newj=jj*cos(angle)+ii*sin(angle);
            
            newi=newi+originx;
            newj=newj+originy;
            
            if (newi>=0 && newi<=width && newj>=0 && newj<=height)
            {
                continent[i][j]=outline[newi][newj];
                
                if (continent[i][j]==1)
                {
                    if (i<leftx)
                        leftx=i;
                    
                    if (i>rightx)
                        rightx=i;
                    
                    if (j<lefty)
                        lefty=j;
                    
                    if (j>righty)
                        righty=j;
                }
            }
            else
            {
                //failed=1;
                //i=width;
                //j=height;
            }
        }
    }
    
    if (failed)
    {
        leftx=width/2;
        rightx=width/2;
        lefty=height/2;
        righty=height/2;
        
        for (int i=0; i<=width; i++)
        {
            for (int j=0; j<=height; j++)
            {
                continent[i][j]=outline[i][j];
                
                if (continent[i][j]==1)
                {
                    if (i<leftx)
                        leftx=i;
                    
                    if (i>rightx)
                        rightx=i;
                    
                    if (j<lefty)
                        lefty=j;
                    
                    if (j>righty)
                        righty=j;
                }
            }
        }
    }
}

// This function creates a simple map of circles to be used for continents.

void makecontinentcircles(vector<vector<bool>> &circlemap, int width, int height, int circleno, int circlesize, int &circleleftx, int &circlerightx, int &circlelefty, int &circlerighty)
{
    vector<vector<int>> circleinfo(circleno,vector<int>(circleno,2)); // This will hold information about each circle.
    
    circleinfo[0][0]=width/2; // centre x coordinate
    circleinfo[0][1]=height/2; // centre y coordinate
    circleinfo[0][2]=circlesize; // radius
    
    for (int circle=1; circle<circleno; circle++)
    {
        bool keepgoing=1;
        
        do
        {
            int prevcircle;
            
            //prevcircle=circle-1;
            
            prevcircle=random(0,circle-1); // Choose one of the previous ones to put this one near.
            
            short rand=random(1,4);
            
            switch (rand)
            {
                case 1:
                    circleinfo[circle][0]=random(circleinfo[prevcircle][0]-circleinfo[prevcircle][3],circleinfo[prevcircle][0]+circleinfo[prevcircle][3]);
                    circleinfo[circle][1]=random(circleinfo[prevcircle][1]-circleinfo[prevcircle][3],circleinfo[prevcircle][1]-circleinfo[prevcircle][3]/2);
                    break;
                    
                case 2:
                    circleinfo[circle][0]=random(circleinfo[prevcircle][0]-circleinfo[prevcircle][3],circleinfo[prevcircle][0]+circleinfo[prevcircle][3]);
                    circleinfo[circle][1]=random(circleinfo[prevcircle][1]+circleinfo[prevcircle][3]/2,circleinfo[prevcircle][1]+circleinfo[prevcircle][3]);
                    break;
                    
                case 3:
                    circleinfo[circle][0]=random(circleinfo[prevcircle][0]-circleinfo[prevcircle][3],circleinfo[prevcircle][0]-circleinfo[prevcircle][3]/2);
                    circleinfo[circle][1]=random(circleinfo[prevcircle][1]-circleinfo[prevcircle][3],circleinfo[prevcircle][1]+circleinfo[prevcircle][3]);
                    break;
                    
                case 4:
                    circleinfo[circle][0]=random(circleinfo[prevcircle][0]+circleinfo[prevcircle][3]/2,circleinfo[prevcircle][0]+circleinfo[prevcircle][3]);
                    circleinfo[circle][1]=random(circleinfo[prevcircle][1]-circleinfo[prevcircle][3],circleinfo[prevcircle][1]+circleinfo[prevcircle][3]);
                    break;
            }
            
            if (circleinfo[circle][0]>circlesize*3 && circleinfo[circle][0]<width-circlesize*3 && circleinfo[circle][1]>circlesize*3 && circleinfo[circle][1]<height-circlesize*3)
                keepgoing=0;
            
        } while (keepgoing==1);

        circleinfo[circle][3]=random((circlesize/3)*2,circlesize);
    }
    
    for (int circle=0; circle<circleno; circle++)
    {
        int centrex=0;
        int centrey=0;
        int radius=circleinfo[circle][3];
        
        for (int i=-radius; i<=radius; i++)
        {
            int ii=circleinfo[circle][0]+i;
            
            for (int j=-radius; j<=radius; j++)
            {
                int jj=circleinfo[circle][1]+j;
                
                if (ii>=0 && ii<=width && jj>=0 && jj<=height)
                {
                    if (i*i+j*j<radius*radius+radius)
                    {
                        circlemap[ii][jj]=1;
                        
                        if (ii<circleleftx)
                            circleleftx=ii;
                        
                        if (ii>circlerightx)
                            circlerightx=ii;
                        
                        if (jj<circlelefty)
                            circlelefty=jj;
                        
                        if (jj>circlerighty)
                            circlerighty=jj;
                    }
                }
            }
        }
    }
    
    // Possibly cut away half of it, diagonally.
    
    int cutchance=2;
    
    if (random(1,cutchance)==1)
    {
        int circlewidth=circlerightx-circleleftx;
        int circleheight=circlerighty-circlelefty;
        
        for (int i=0; i<circlewidth; i++)
        {
            int ii=circleleftx+i;
            
            for (int j=0; j<=i; j++)
            {
                int jj=circlelefty+j;
                
                if (jj>=0 && jj<=height && ii>=0 && ii<=width)
                    circlemap[ii][jj]=0;
            }
        }
    }

    // Now just stick a few on around the edges, and also remove a few.
    
    vector<vector<bool>> circlecopy(width+1,vector<bool>(width+1,height+1));
    
    for (int n=0; n<2; n++)
    {
        for (int i=0; i<=width; i++)
        {
            for (int j=0; j<=height; j++)
                circlecopy[i][j]=circlemap[i][j];
        }
        
        int extracirclechance=100;
        
        for (int i=0; i<=width; i++)
        {
            for (int j=0; j<=height; j++)
            {
                if (circlecopy[i][j]==1 && random(1,extracirclechance)==1)
                {
                    bool outline=0;
                    
                    for (int ii=i-1; ii<=i+1; ii++)
                    {
                        for (int jj=j-1; jj<=j+1; jj++)
                        {
                            if (ii>=0 && ii<=width && jj>=0 && jj<=height)
                            {
                                if (circlecopy[ii][jj]==0)
                                {
                                    outline=1;
                                    ii=i+1;
                                    jj=j+1;
                                }
                            }
                            
                        }
                    }
                    
                    if (outline==1)
                    {
                        int centrex=i;
                        int centrey=j;
                        
                        int radius=random(circlesize/4,circlesize/2);
                        
                        for (int x=-radius; x<=radius; x++)
                        {
                            int xx=centrex+x;
                            
                            for (int y=-radius; y<=radius; y++)
                            {
                                int yy=centrey+y;
                                
                                if (xx>0 && xx<width && yy>0 && yy<height)
                                {
                                    if (x*x+y*y<radius*radius+radius)
                                    {
                                        circlemap[xx][yy]=1;
                                        
                                        if (xx<circleleftx)
                                            circleleftx=xx;
                                        
                                        if (xx>circlerightx)
                                            circlerightx=xx;
                                        
                                        if (yy<circlelefty)
                                            circlelefty=yy;
                                        
                                        if (yy>circlerighty)
                                            circlerighty=yy;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        
        for (int i=0; i<=width; i++)
        {
            for (int j=0; j<=height; j++)
                circlecopy[i][j]=circlemap[i][j];
        }
        
        for (int i=0; i<=width; i++)
        {
            for (int j=0; j<=height; j++)
            {
                if (circlecopy[i][j]==1 && random(1,extracirclechance)==1)
                {
                    bool outline=0;
                    
                    for (int ii=i-1; ii<=i+1; ii++)
                    {
                        for (int jj=j-1; jj<=j+1; jj++)
                        {
                            if (ii>=0 && ii<=width && jj>=0 && jj<=height)
                            {
                                if (circlecopy[ii][jj]==0)
                                {
                                    outline=1;
                                    ii=i+1;
                                    jj=j+1;
                                }
                            }
                            
                        }
                    }
                    
                    if (outline==1)
                    {
                        int centrex=i;
                        int centrey=j;
                        
                        int radius=random(circlesize/4,circlesize/2);
                        
                        for (int x=-radius; x<=radius; x++)
                        {
                            int xx=centrex+x;
                            
                            for (int y=-radius; y<=radius; y++)
                            {
                                int yy=centrey+y;
                                
                                if (xx>0 && xx<width && yy>0 && yy<height)
                                {
                                    if (x*x+y*y<radius*radius+radius)
                                    {
                                        circlemap[xx][yy]=0;
                                        
                                        if (xx<circleleftx)
                                            circleleftx=xx;
                                        
                                        if (xx>circlerightx)
                                            circlerightx=xx;
                                        
                                        if (yy<circlelefty)
                                            circlelefty=yy;
                                        
                                        if (yy>circlerighty)
                                            circlerighty=yy;
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

// This function creates chains of mountains, with associated land where appropriate.

void createchains(planet &world, int baseheight, int conheight, vector<vector<int>> &fractal, vector<vector<int>> &plateaumap, boolshapetemplate landshape[], boolshapetemplate chainland[], sf::Vector2i focuspoints[], int focustotal, int focaldistance, int mode)
{
    // mode=0: continental chains (with associated land).
    // mode=1: smaller mountains (no associated land, can form peninsulas).
    // mode=2: smaller mountains (no associated land, can't form peninsulas).
    // mode=3: island chains.
    // mode=5: like mode 2, but fewer.
    // mode=6: like 3, but fewer.
    // mode=7: hills.
    
    bool lower=0;
    
    if (mode==7)
    {
        mode=2;
        lower=1;
    }
    
    bool fewer=0;
    
    if (mode==5)
    {
        mode=2;
        fewer=1;
    }
    
    if (mode==6)
    {
        mode=3;
        fewer=1;
    }

    //int temptotal=24; // Total number of mountain range templates available.
    
    int rangestartvar=2; //6; // Possible distance between previous range and next one in a chain
    int changedirchance=3; // Chance of changing direction (the lower it is, the more likely)
    int landchance=10; // Chance of any given mountain pixel generating a nearby lump of land (the lower it is, the more likely)
    int minlanddist=5;
    int maxlanddist=10; // Range for how far lumps of land can be from the previous one (or the originating mountain)
    int stringmax=20; //10; // Maximum number of lumps in a string of land
    int landvar=4; //8; // Maximum distance a lump of land can be displaced from its proper position
    int platchance=4; // Chance of any range generating a plateau
    int platmaxheight=30; // Percentage of the range height for the plateau maximum height
    int platminheight=5; // Percentage of the range height for the plateau minimum height
    int minplatreduce=80;
    int maxplatreduce=95; // Range of percentages to reduce the height of the plateau with each step
    int mode1landmin=30; // Minimum distance to land from the starting point of a mode 1 chain.
    
    int width=world.width();
    int height=world.height();
    int maxelev=world.maxelevation();
    int sealevel=world.sealevel();
    
    vector<vector<unsigned char>> rangeheighttemplate(height+1,vector<unsigned char>(height+1,height+1));
    vector<vector<unsigned char>> rangeridgetemplate(height+1,vector<unsigned char>(height+1,height+1)); // These will hold the range we're currently putting on.

    int chainno=0;
    int minchainlength=0;
    int maxchainlength=0;
    
    switch (mode)
    {
        case 0:
            
            chainno=random(10,40); //(5,20);
            maxchainlength=15;
            minchainlength=4;
            break;
            
        case 1:
            
            chainno=random(8,60); //(2,15);
            maxchainlength=8;
            minchainlength=1;
            break;
            
        case 2:
            
            chainno=random(4000,32000); //(1000,8000);
            
            if (fewer==1)
                chainno=1000;
            
            maxchainlength=8;
            minchainlength=1;
            break;
            
        case 3:
            
            chainno=random(20,200); //(80,800);
            
            if (fewer==1)
                chainno=10;
            
            maxchainlength=6;
            minchainlength=1;
            break;
    }
    
    // Now do the chains.
    
    for (int chain=1; chain<=chainno; chain++)
    {
        int oldx=random(0,width);
        int oldy=random(0,height);
        
        if (mode==0 || mode==3) // Continental ranges should begin near the continental focus points, if possible. Islands should begin near their focus points too.
        {
            int thisfocus=random(0,focustotal);
            
            oldx=focuspoints[thisfocus].x;
            oldy=focuspoints[thisfocus].y; // Coordinates of the current blob.
            
            float xdiff=random(1,100);
            float ydiff=random(1,100);
            
            xdiff=xdiff/100;
            ydiff=ydiff/100;
            
            xdiff=pow(xdiff,2);
            ydiff=pow(ydiff,2);
            
            xdiff=xdiff*focaldistance;
            ydiff=ydiff*focaldistance;
            
            oldx=oldx+randomsign(xdiff);
            oldy=oldy+randomsign(ydiff);
            
            if (oldy<0)
                oldy=0;
            
            if (oldy>height)
                oldy=height;
            
            if (oldx<0 || oldx>width)
                oldx=wrap(oldx,width);
        }
        
        int chaindir=random(1,8); // Chaindir is the direction of the chain, from 1-8 (sort of on the diagonals!).
        int maxranges=random(minchainlength,maxchainlength); // The chain won't have more ranges in it than this.
        int rangeno=1;
        bool goahead=1;
        
        int tempno=-1;
        
        if (!(mode==2 && world.seawrap(oldx,oldy)==1)) // If we're placing land-only mountains and we're in the sea, just skip the whole thing.
        {
            do
            {
                int lasttemp=tempno;
                
                do // Get a new template, making sure it's not the same as the last one.
                {
                    tempno=random(1,MOUNTAINTEMPLATESTOTAL);
                } while (tempno==lasttemp);
                
                int dampner=1800; //maxelev/7; // This is to reduce the maximum height a bit.
                int rangeheight=0;
                
                switch (mode)
                {
                    case 0:
                        
                        rangeheight=random((maxelev-sealevel)/2,maxelev-sealevel); // Approximate height of the range (above sea level)
                        rangeheight=rangeheight-dampner;
                        
                        break;
                        
                    case 1:
                        
                        rangeheight=random(200,(maxelev-sealevel)/2);
                        rangeheight=rangeheight-dampner;
                        
                        break;
                        
                    case 2:
                        
                        rangeheight=random(100,(maxelev-sealevel)/3);
                        rangeheight=rangeheight-dampner;
                        
                        if (lower==1)
                            rangeheight=rangeheight/4;
                        
                        break;
                        
                    case 3:
                        
                        rangeheight=random(sealevel/2,sealevel+3000);
                        
                        break;
                }
                
                if (rangeheight<200)
                    rangeheight=200;
                
                bool plateau=0;
                int platheight=0;
                
                if (mode==0 && random(1,platchance)==1) // See if we might do a plateau.
                {
                    plateau=1;
                    
                    float platheightfactor=random(platminheight,platmaxheight);
                    platheightfactor=platheightfactor/100;
                    
                    platheight=rangeheight*platheightfactor;
                }
                
                int rangeinc=rangeheight/10; // Difference between the different template heights
                int rangelittleinc=rangeinc/100; // Amount to vary the template heights by
                
                // Get the template.
                
                int thisdir=chaindir;
                
                if (random(1,2)==1)
                    thisdir=thisdir+4;
                
                if (thisdir>8)
                    thisdir=thisdir-8;
                
                int span=createmountainrangetemplate(rangeridgetemplate,rangeheighttemplate,tempno,thisdir);
                
                // Now we need to find the start of the actual range within the template.
                
                int startx=0;
                int starty=0;
                
                if (chaindir==1 || chaindir==2)
                {
                    for (int i=0; i<=span; i++)
                    {
                        for (int j=0; j<=span; j++)
                        {
                            if (rangeridgetemplate[i][j]!=0)
                            {
                                startx=i;
                                starty=j;
                                
                                i=span;
                                j=span;
                            }
                        }
                    }
                }
                
                if (chaindir==3 || chaindir==4)
                {
                    for (int j=0; j<=span; j++)
                    {
                        for (int i=0; i<=span; i++)
                        {
                            if (rangeridgetemplate[i][j]!=0)
                            {
                                startx=i;
                                starty=j;
                                
                                i=span;
                                j=span;
                            }
                        }
                    }
                }
                
                if (chaindir==5 || chaindir==6)
                {
                    for (int i=span; i>=0; i--)
                    {
                        for (int j=0; j<=span; j++)
                        {
                            if (i>=0 && i<=height && j>=0 && j<=height && rangeridgetemplate[i][j]!=0)
                            {
                                startx=i;
                                starty=j;
                                
                                i=0;
                                j=span;
                            }
                        }
                    }
                }
                
                if (chaindir==7 || chaindir==8)
                {
                    for (int j=span; j>=0; j--)
                    {
                        for (int i=0; i<=span; i++)
                        {
                            if (rangeridgetemplate[i][j]!=0)
                            {
                                startx=i;
                                starty=j;
                                
                                i=span;
                                j=0;
                            }
                        }
                    }
                }
                
                if (chaindir==1 || chaindir==2)
                {
                    oldx=oldx+random(0,rangestartvar);
                    oldy=oldy+randomsign(random(0,rangestartvar));
                }
                
                if (chaindir==3 || chaindir==4)
                {
                    oldx=oldx+randomsign(random(0,rangestartvar));
                    oldy=oldy+random(0,rangestartvar);
                }
                
                if (chaindir==5 || chaindir==6)
                {
                    oldx=oldx-random(0,rangestartvar);
                    oldy=oldy+randomsign(random(0,rangestartvar));
                }
                
                if (chaindir==7 || chaindir==8)
                {
                    oldx=oldx+randomsign(random(0,rangestartvar));
                    oldy=oldy-random(0,rangestartvar);
                }
                
                // oldx and oldy are now the point where we want the new range to start.
                
                int x=oldx-startx;
                int y=oldy-starty;
                
                if (x<=0 || x>width)
                    x=wrap(x,width);
                
                if (mode==1 && oldx>=0 && oldx<=width && oldy>=0 && oldy<=height) // Peninsular mountains have to start within a certain distance of land.
                {
                    if (world.sea(oldx,oldy)==1)
                    {
                        bool foundone=0;
                        
                        for (int i=oldx-mode1landmin; i<=oldx+mode1landmin; i++)
                        {
                            for (int j=oldy-mode1landmin; j<=oldy+mode1landmin; j++)
                            {
                                if (world.seawrap(i,j)==0)
                                {
                                    foundone=1;
                                    
                                    i=oldx+mode1landmin;
                                    j=oldy+mode1landmin;
                                }
                            }
                        }
                        
                        if (foundone==0)
                            goahead=0;
                    }
                }
                
                if (mode==2) // Non-continental, non-peninsular mountains can't start in the sea.
                {
                    if (world.seawrap(oldx,oldy)==1)
                        goahead=0;
                }
                
                if (mode==3) // Island chains can't start on the land.
                {
                    if (oldx<0 || oldx>width || oldy<0 || oldy>height)
                        goahead=0;
                    else
                    {
                        if (world.sea(oldx,oldy)==0)
                            goahead=0;
                    }
                }
                
                // x and y are now the coordinates of the top left of the new range template.
                // Now we need to see whether there is a clear area to paste it.
                
                if (mode!=2)
                {
                    for (int i=x; i<=x+span; i++)
                    {
                        for (int j=y; j<=y+span; j++)
                        {
                            if (world.mountainheightwrap(i,j)!=0)
                                goahead=0;
                        }
                    }
                }
                
                int hislandadd=random(1000,5000);
                
                if (goahead==1) // If the area is clear
                {
                    for (int i=0; i<=span; i++)
                    {
                        int ii=x+i;
                        if (ii<0 || ii>width)
                            ii=wrap(ii,width);
                        
                        for (int j=0; j<=span; j++)
                        {
                            int jj=y+j;
                            
                            if (jj>=0 && jj<=height)
                            {
                                if (world.mountainheight(ii,jj)==0)
                                {
                                    if (i>=0 && i<=height && j>=0 && j<=height && rangeheighttemplate[i][j]!=0)
                                    {
                                        int h=rangeheighttemplate[i][j]*rangeinc;
                                        h=h+randomsign(rangelittleinc*random(0,100));
                                        
                                        if (mode==3)
                                            h=h+hislandadd;
                                        
                                        float frac=fractal[ii][jj];
                                        frac=frac/maxelev;
                                        h=h*frac;
                                        
                                        if (mode==0 || mode==1)
                                        {
                                            world.setmountainheight(ii,jj,h);
                                            world.setmountainridge(ii,jj,rangeridgetemplate[i][j]);
                                            world.setnom(ii,jj,conheight);
                                        }
                                        
                                        if (mode==2) // Mountains of this kind cannot create their own land, so must stop at the sea.
                                        {
                                            if (world.map(ii,jj)>=conheight)
                                            {
                                                world.setmountainheight(ii,jj,h);
                                                world.setmountainridge(ii,jj,rangeridgetemplate[i][j]);
                                                
                                                if (fewer==0)
                                                    world.setnom(ii,jj,conheight);
                                            }
                                        }
                                        
                                        if (mode==3) // Islands are just strange and different.
                                        {
                                            if (h>sealevel)
                                            {
                                                world.setmountainheight(ii,jj,h-sealevel);
                                                world.setmountainridge(ii,jj,rangeridgetemplate[i][j]);
                                                world.setnom(ii,jj,conheight);
                                            }
                                        }
                                        
                                        if (mode==0) // Only create substantial land if these are continental mountains.
                                        {
                                            if (random(1,landchance)==1) // See if we'll paste a chunk of land nearby
                                            {
                                                int dist=random(minlanddist,maxlanddist);
                                                
                                                int movex=0;
                                                int movey=0;
                                                
                                                switch (chaindir)
                                                {
                                                    case 1:
                                                        movex=0-dist/2;
                                                        movey=0-dist;
                                                        break;
                                                        
                                                    case 2:
                                                        movex=dist/2;
                                                        movey=0-dist;
                                                        break;
                                                        
                                                    case 3:
                                                        movex=dist;
                                                        movey=0-dist/2;
                                                        break;
                                                        
                                                    case 4:
                                                        movex=dist;
                                                        movey=dist/2;
                                                        break;
                                                        
                                                    case 5:
                                                        movex=dist/2;
                                                        movey=dist;
                                                        break;
                                                        
                                                    case 6:
                                                        movex=0-dist/2;
                                                        movey=dist;
                                                        break;
                                                        
                                                    case 7:
                                                        movex=0-dist;
                                                        movey=dist/2;
                                                        break;
                                                        
                                                    case 8:
                                                        movex=0-dist;
                                                        movey=0-dist/2;
                                                        break;
                                                }
                                                
                                                int landx=ii;
                                                int landy=jj;
                                                
                                                int max=rangeno;
                                                int max2=maxranges-rangeno;
                                                
                                                if (max2<max)
                                                    max=max2;
                                                
                                                int mult=4;
                                                
                                                max=max*mult;
                                                
                                                if (max>stringmax)
                                                    max=stringmax;
                                                
                                                int totallumps=random((max/4)*3,max);
                                                
                                                for (int lump=1; lump<=totallumps; lump++) // Now we will paste a string of land lumps leading away from the mountain.
                                                {
                                                    landx=landx+movex;
                                                    landy=landy+movey;
                                                    
                                                    landx=landx+randomsign(random(0,landvar));
                                                    landy=landy+randomsign(random(0,landvar));
                                                    
                                                    if (landx<0 || landx>width)
                                                        landx=wrap(landx,width);
                                                    
                                                    int shapenumber=random(0,1);
                                                    drawshape(world,shapenumber,landx,landy,1,baseheight,conheight,chainland);
                                                    
                                                    if (plateau==1) // Draw a bit of plateau, and reduce the height for the next pass.
                                                    {
                                                        drawplateaushape(world,shapenumber,landx,landy,baseheight,platheight,plateaumap,chainland);
                                                        
                                                        float platreduce=random(minplatreduce,maxplatreduce);
                                                        platreduce=platreduce/100.0;
                                                        platheight=platheight*platreduce;
                                                    }
                                                }
                                            }
                                        }
                                        
                                        if (mode==1) // Mountains of this kind can create land just immediately around themselves.
                                        {
                                            if (world.nom(ii,jj)<conheight)
                                                world.setnom(ii,jj,conheight);
                                        }
                                        
                                        if (mode==3) // Island mountains can create land just immediately around themselves.
                                        {
                                            if (h>sealevel)
                                            {
                                                if (world.nom(ii,jj)<conheight)
                                                    world.setnom(ii,jj,conheight);
                                            }
                                        }
                                    }
                                }
                                else // If this range goes off the map, and the chain.
                                    goahead=0;
                                
                            }
                        }
                    }
                    
                    // Now we need to find the new oldx and oldy.
                    
                    if (chaindir==1 || chaindir==2)
                    {
                        for (int i=span; i>=0; i--)
                        {
                            for (int j=0; j<=span; j++)
                            {
                                if (i>=0 && i<=height && j>=0 && j<=height & rangeheighttemplate[i][j]!=0)
                                {
                                    oldx=x+i;
                                    oldy=y+j;
                                    
                                    if (oldx<0 || oldx>width)
                                        oldx=wrap(oldx,width);
                                    
                                    i=0;
                                    j=span;
                                }
                            }
                        }
                    }
                    
                    if (chaindir==3 || chaindir==4)
                    {
                        for (int j=span; j>=0; j--)
                        {
                            for (int i=0; i<=span; i++)
                            {
                                if (rangeheighttemplate[i][j]!=0)
                                {
                                    oldx=x+i;
                                    oldy=y+j;
                                    
                                    if (oldx<0 || oldx>width)
                                        oldx=wrap(oldx,width);
                                    
                                    j=0;
                                    i=span;
                                }
                            }
                        }
                    }
                    
                    if (chaindir==5 || chaindir==6)
                    {
                        for (int i=0; i<=span; i++)
                        {
                            for (int j=0; j<=span; j++)
                            {
                                if (rangeheighttemplate[i][j]!=0)
                                {
                                    oldx=x+i;
                                    oldy=y+j;
                                    
                                    if (oldx<0 || oldx>width)
                                        oldx=wrap(oldx,width);
                                    
                                    i=span;
                                    j=span;
                                }
                            }
                        }
                    }
                    
                    if (chaindir==7 || chaindir==8)
                    {
                        for (int j=0; j<=span; j++)
                        {
                            for (int i=0; i<=span; i++)
                            {
                                if (rangeheighttemplate[i][j]!=0)
                                {
                                    oldx=x+i;
                                    oldy=y+j;
                                    
                                    if (oldx<0 || oldx>width)
                                        oldx=wrap(oldx,width);
                                    
                                    j=span;
                                    i=span;
                                }
                            }
                        }
                    }
                    
                    // Now change direction, possibly.
                    
                    if (random(1,changedirchance)==1)
                    {
                        chaindir=chaindir+randomsign(1);
                        
                        if (chaindir>8)
                            chaindir=1;
                        
                        if (chaindir<1)
                            chaindir=8;
                    }
                }
                
                rangeno++;
                
                if (rangeno>maxranges)
                    goahead=0;
                
            } while (goahead==1);
        }
    }
}

// This function creates a chain of mountains from one point to another.

void createdirectedchain(planet &world, int baseheight, int conheight, short thiscontinent, vector<vector<short>> &continentnos, vector<vector<int>> &fractal, boolshapetemplate landshape[], boolshapetemplate chainland[], int chainstartx, int chainstarty, int chainendx, int chainendy, int mode, vector<sf::Vector2i> &rangepoints, vector<vector<bool>> &currentland, int volcanochance)
{

    // mode=0: overlapping continents (over land, no associated land is created).
    // mode=1: along the edge of a continent (creates land if necessary, removes land to one side).
    
    //int temptotal=24; // Total number of mountain range templates available.
    
    int rangestartvar=1; //2; // Possible distance between previous range and next one in a chain
    int changedirchance=3; // Chance of changing direction (the lower it is, the more likely)
    int landchance=10; // Chance of any given mountain pixel generating a nearby lump of land (the lower it is, the more likely)
    int minlanddist=random(2,5);
    int maxlanddist=random(6,12); // Range for how far lumps of land can be from the originating mountain
    int landvar=4; // Maximum distance a lump of land can be displaced from its proper position
    int swervechance=4; // Probability of the chain swerving from its destination.
    if (mode==1)
        swervechance=12;
    
    int totaldist=sqrt((chainstartx-chainendx)*(chainstartx-chainendx)+(chainstarty-chainendy)*(chainstarty-chainendy));
    
    int waypointvary=totaldist/4;
    int waypointminvary=waypointvary/4;
    
    if (waypointvary<2)
        waypointvary=2;
    
    int width=world.width();
    int height=world.height();
    int maxelev=world.maxelevation();
    int sealevel=world.sealevel();
    
    vector<vector<bool>> thischain(width+1,vector<bool>(width+1,height+1));
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
            thischain[i][j]=0;
    }
    
    vector<vector<unsigned char>> rangeheighttemplate(height+1,vector<unsigned char>(height+1,height+1));
    vector<vector<unsigned char>> rangeridgetemplate(height+1,vector<unsigned char>(height+1,height+1)); // These will hold the range we're currently putting on.

    int halfwayx=(chainstartx+chainendx)/2;
    int halfwayy=(chainstarty+chainendy)/2;
    
    int waypoint1x=(chainstartx+halfwayx)/2;
    int waypoint1y=(chainstarty+halfwayy)/2;
    
    int waypoint2x=(halfwayx+chainendx)/2;
    int waypoint2y=(halfwayy+chainendy)/2;
    
    waypoint1x=waypoint1x+randomsign(random(waypointminvary,waypointvary));
    waypoint1y=waypoint1y+randomsign(random(waypointminvary,waypointvary));
    
    waypoint2x=waypoint2x+randomsign(random(waypointminvary,waypointvary));
    waypoint2y=waypoint2y+randomsign(random(waypointminvary,waypointvary));
    
    if (waypoint1x<0)
        waypoint1x=0;
    
    if (waypoint1x>width)
        waypoint1x=width;
    
    if (waypoint1y<0)
        waypoint1y=0;
    
    if (waypoint1y>height)
        waypoint1y=height;
    
    if (waypoint2x<0)
        waypoint2x=0;
    
    if (waypoint2x>width)
        waypoint2x=width;
    
    if (waypoint2y<0)
        waypoint2y=0;
    
    if (waypoint2y>height)
        waypoint2y=height;

    short goingto=1;
    
    // Now do the chain.

    int oldx=chainstartx;
    int oldy=chainstarty;

    short chaindir=0; // Chaindir is the direction of the chain, from 1-8 (sort of on the diagonals!).

    int rangeno=1;
    bool goahead=1;
    
    bool goingtowardsend=1;
    bool justswerved=0;
    
    int tempno=-1;
    
    do
    {
        int lasttemp=tempno;
        
        if (mode==1)
        {
            rangepoints[rangeno].x=oldx;
            rangepoints[rangeno].y=oldy;
        }
        
        //short olddir=chaindir;
        
        if (mode==0)
        {
            if (goingtowardsend==1)
                chaindir=getmountaindir(world,oldx,oldy,chainendx,chainendy);
            else
            {
                sf::Vector2i sea=nearestsea(world,oldx,oldy,0,height,10);
                
                chainendx=sea.x;
                chainendy=sea.y;
                
                if (chainendx<0 || chainendx>width)
                    chainendx=wrap(chainendx,width);
                
                chaindir=getmountaindir(world,oldx,oldy,chainendx,chainendy);
            }
        }
        else
        {
            if (goingto==1)
                chaindir=getmountaindir(world,oldx,oldy,waypoint1x,waypoint1y);
            
            if (goingto==2)
                chaindir=getmountaindir(world,oldx,oldy,waypoint2x,waypoint2y);
            
            if (goingto==3)
                chaindir=getmountaindir(world,oldx,oldy,chainendx,chainendy);
        }
        
        if (justswerved==0 && random(1,swervechance)==1)
        {
            chaindir=chaindir+randomsign(1);
            justswerved=1;
        }
        else
            justswerved=0;
        
        if (chaindir==0)
            chaindir=8;
        
        if (chaindir==9)
            chaindir=1;
        
        goahead=1;
        
        do // Get a new template, making sure it's not the same as the last one.
        {
            tempno=random(1,MOUNTAINTEMPLATESTOTAL);
        } while (tempno==lasttemp);
        
        int dampner=maxelev/12; // This is to reduce the maximum height a bit.
        int rangeheight=random((maxelev-sealevel)/2,maxelev-sealevel); // Approximate height of the range (above sea level)
        rangeheight=rangeheight-dampner;

        if (rangeheight<200)
            rangeheight=200;
        
        int rangeinc=rangeheight/10; // Difference between the different template heights
        int rangelittleinc=rangeinc/100; // Amount to vary the template heights by
        
        // Get the template.
        
        int thisdir=chaindir;
        
        if (random(1,2)==1)
            thisdir=thisdir+4;
        
        if (thisdir>8)
            thisdir=thisdir-8;
        
        int span=createmountainrangetemplate(rangeridgetemplate,rangeheighttemplate,tempno,thisdir);
        
        // Now we need to find the start of the actual range within the template.
        
        int startx=0;
        int starty=0;
        
        if (chaindir==1 || chaindir==2)
        {
            for (int i=0; i<=span; i++)
            {
                for (int j=0; j<=span; j++)
                {
                    if (rangeridgetemplate[i][j]!=0)
                    {
                        startx=i;
                        starty=j;
                        
                        i=span;
                        j=span;
                    }
                }
            }
        }
        
        if (chaindir==3 || chaindir==4)
        {
            for (int j=0; j<=span; j++)
            {
                for (int i=0; i<=span; i++)
                {
                    if (rangeridgetemplate[i][j]!=0)
                    {
                        startx=i;
                        starty=j;
                        
                        i=span;
                        j=span;
                    }
                }
            }
        }
        
        if (chaindir==5 || chaindir==6)
        {
            for (int i=span; i>=0; i--)
            {
                for (int j=0; j<=span; j++)
                {
                    if (i>=0 && i<=height && j>=0 && j<=height && rangeridgetemplate[i][j]!=0)
                    {
                        startx=i;
                        starty=j;
                        
                        i=0;
                        j=span;
                    }
                }
            }
        }
        
        if (chaindir==7 || chaindir==8)
        {
            for (int j=span; j>=0; j--)
            {
                for (int i=0; i<=span; i++)
                {
                    if (rangeridgetemplate[i][j]!=0)
                    {
                        startx=i;
                        starty=j;
                        
                        i=span;
                        j=0;
                    }
                }
            }
        }
        
        if (chaindir==1 || chaindir==2)
        {
            oldx=oldx+random(0,rangestartvar);
            oldy=oldy+randomsign(random(0,rangestartvar));
        }
        
        if (chaindir==3 || chaindir==4)
        {
            oldx=oldx+randomsign(random(0,rangestartvar));
            oldy=oldy+random(0,rangestartvar);
        }
        
        if (chaindir==5 || chaindir==6)
        {
            oldx=oldx-random(0,rangestartvar);
            oldy=oldy+randomsign(random(0,rangestartvar));
        }
        
        if (chaindir==7 || chaindir==8)
        {
            oldx=oldx+randomsign(random(0,rangestartvar));
            oldy=oldy-random(0,rangestartvar);
        }
        
        // oldx and oldy are now the point where we want the new range to start.
        
        int x=oldx-startx;
        int y=oldy-starty;
        
        if (x<=0 || x>width)
            x=wrap(x,width);

        if (mode==0) // Mountains on overlapping continents can't start in the sea.
        {
            if (world.seawrap(oldx,oldy)==1)
                goahead=0;
        }
        
        // x and y are now the coordinates of the top left of the new range template.
        // Now we need to see whether there is a clear area to paste it.

        for (int i=x; i<=x+span; i++)
        {
            int ii=i;
            
            if (ii<0 || ii>width)
                ii=wrap(ii,width);
            
            for (int j=y; j<=y+span; j++)
            {
                if (j>=0 && j<=height && world.mountainheight(ii,j)!=0 && thischain[ii][j]==0)
                {
                    goahead=0;
                    i=x+span;
                    j=y+span;
                }
            }
        }

        if (goahead==1) // If the area is clear
        {
            for (int i=0; i<=span; i++)
            {
                int ii=x+i;
                if (ii<0 || ii>width)
                    ii=wrap(ii,width);
                
                for (int j=0; j<=span; j++)
                {
                    int jj=y+j;
                    
                    if (jj>=0 && jj<=height)
                    {
                        if (ii==chainendx && jj==chainendy) // If we've reached the vicinity of the end point
                        {
                            if (mode==0)
                                goingtowardsend=0;
                            else
                                return;
                        }
                        
                        if (mode==1)
                        {
                            if (goingto==1 && ii==waypoint1x && jj==waypoint1y)
                                goingto=2;
                            
                            if (goingto==2 && ii==waypoint2x && jj==waypoint2y)
                                goingto=3;
                        }
                        
                        if (world.mountainheight(ii,jj)==0)
                        {
                            if (rangeheighttemplate[i][j]!=0)
                            {
                                int h=rangeheighttemplate[i][j]*rangeinc;
                                h=h+randomsign(rangelittleinc*random(0,100));
                                
                                float frac=fractal[ii][jj];
                                frac=frac/maxelev;
                                h=h*frac;
                                
                                if (mode==0) // Mountains of this kind cannot create their own land, so must stop at the sea.
                                {
                                    if (world.map(ii,jj)>=conheight)
                                    {
                                        thischain[ii][jj]=1;
                                        
                                        world.setmountainheight(ii,jj,h);
                                        world.setmountainridge(ii,jj,rangeridgetemplate[i][j]);
                                        world.setnom(ii,jj,conheight);
                                    }
                                    else
                                        goahead=0;
                                }

                                if (mode==1) // Mountains of this kind can create their own land.
                                {
                                    thischain[ii][jj]=1;
                                    
                                    world.setmountainheight(ii,jj,h);
                                    world.setmountainridge(ii,jj,rangeridgetemplate[i][j]);
                                    world.setnom(ii,jj,conheight);
                                    
                                    if (rangeheighttemplate[i][j]>=5 && random(1,volcanochance)==1) // Put a volcano here!
                                    {
                                        world.setvolcano(ii,jj,h);
                                        world.setstrato(ii,jj,h);
                                    }
                                    
                                    if (random(1,landchance)==1) // See if we'll paste a chunk of land nearby
                                    {
                                        int dist=randomsign(random(minlanddist,maxlanddist));
                                        
                                        int movex=0;
                                        int movey=0;
                                        
                                        switch (chaindir)
                                        {
                                            case 1:
                                                movex=0-dist/2;
                                                movey=0-dist;
                                                break;
                                                
                                            case 2:
                                                movex=dist/2;
                                                movey=0-dist;
                                                break;
                                                
                                            case 3:
                                                movex=dist;
                                                movey=0-dist/2;
                                                break;
                                                
                                            case 4:
                                                movex=dist;
                                                movey=dist/2;
                                                break;
                                                
                                            case 5:
                                                movex=dist/2;
                                                movey=dist;
                                                break;
                                                
                                            case 6:
                                                movex=0-dist/2;
                                                movey=dist;
                                                break;
                                                
                                            case 7:
                                                movex=0-dist;
                                                movey=dist/2;
                                                break;
                                                
                                            case 8:
                                                movex=0-dist;
                                                movey=0-dist/2;
                                                break;
                                        }
                                        
                                        int landx=ii;
                                        int landy=jj;
                                        
                                        // Now paste a lump under the mountain.

                                        landx=landx+movex;
                                        landy=landy+movey;
                                        
                                        landx=landx+randomsign(random(0,landvar));
                                        landy=landy+randomsign(random(0,landvar));
                                        
                                        if (landx<0 || landx>width)
                                            landx=wrap(landx,width);
                                        
                                        int shapenumber=random(0,1);
                                        drawmarkedshape(world,shapenumber,landx,landy,1,baseheight,conheight,currentland,chainland);
                                    }
                                }
                            }
                        }
                    }
                    else // If this range goes off the map, end the chain.
                        goahead=0;
                }
            }
            
            // Now we need to find the new oldx and oldy.
            
            if (chaindir==1 || chaindir==2)
            {
                for (int i=span; i>=0; i--)
                {
                    for (int j=0; j<=span; j++)
                    {
                        if (i>=0 && i<=height && j>=0 && j<=height & rangeheighttemplate[i][j]!=0)
                        {
                            oldx=x+i;
                            oldy=y+j;
                            
                            if (oldx<0 || oldx>width)
                                oldx=wrap(oldx,width);
                            
                            i=0;
                            j=span;
                        }
                    }
                }
            }
            
            if (chaindir==3 || chaindir==4)
            {
                for (int j=span; j>=0; j--)
                {
                    for (int i=0; i<=span; i++)
                    {
                        if (rangeheighttemplate[i][j]!=0)
                        {
                            oldx=x+i;
                            oldy=y+j;
                            
                            if (oldx<0 || oldx>width)
                                oldx=wrap(oldx,width);
                            
                            j=0;
                            i=span;
                        }
                    }
                }
            }
            
            if (chaindir==5 || chaindir==6)
            {
                for (int i=0; i<=span; i++)
                {
                    for (int j=0; j<=span; j++)
                    {
                        if (rangeheighttemplate[i][j]!=0)
                        {
                            oldx=x+i;
                            oldy=y+j;
                            
                            if (oldx<0 || oldx>width)
                                oldx=wrap(oldx,width);
                            
                            i=span;
                            j=span;
                        }
                    }
                }
            }
            
            if (chaindir==7 || chaindir==8)
            {
                for (int j=0; j<=span; j++)
                {
                    for (int i=0; i<=span; i++)
                    {
                        if (rangeheighttemplate[i][j]!=0)
                        {
                            oldx=x+i;
                            oldy=y+j;
                            
                            if (oldx<0 || oldx>width)
                                oldx=wrap(oldx,width);
                            
                            j=span;
                            i=span;
                        }
                    }
                }
            }
        }
        
        rangeno++;
        
    } while (goahead==1);
}

// This finds the direction from one point to another, for mountain ranges.

short getmountaindir(planet &world, int startx, int starty, int endx, int endy)
{
    int width=world.width();
    
    // First, work out whether we need to do any wrapping.

    int xdist=startx-endx;
    int ydist=starty-endy;
    
    int dist=xdist*xdist+ydist*ydist;
    
    int leftxdist=(startx+width)-endx;
    
    int leftdist=leftxdist*leftxdist+ydist*ydist;
    
    int rightxdist=startx-(endx+width);
    
    int rightdist=rightxdist*rightxdist+ydist*ydist;
    
    if (leftdist<dist)
    {
        dist=leftdist;
        startx=startx+width;
    }
    
    if (rightdist<dist)
    {
        dist=rightdist;
        endx=endx+width;
    }
    
    short dir=0;
    
    xdist=abs(startx-endx);
    ydist=abs(starty-endy);
    
    if (endx>startx && endy>starty) // Roughly southeast
    {
        if (xdist>ydist)
            dir=2;
        
        else
            dir=3;
    }
    
    if (endx>startx && endy<=starty) // Roughly northeast
    {
        if (xdist>ydist)
            dir=1;
        
        else
            dir=8;
    }
    
    if (endx<=startx && endy>starty) // Roughly southwest
    {
        if (xdist>ydist)
            dir=5;
        
        else
            dir=4;
    }
    
    if (endx<=startx && endy<=starty) // Roughly northwest
    {
        if (xdist>ydist)
            dir=6;
        
        else
            dir=7;
    }

    return dir;
}

// Draws a shape from an image onto the map.

void drawshape(planet &world, int shapenumber, int centrex, int centrey, bool land, int baseheight, int conheight, boolshapetemplate shape[])
{
    int width=world.width();
    int height=world.height();
    
    int imheight=shape[shapenumber].ysize()-1;
    int imwidth=shape[shapenumber].xsize()-1;
    
    int x=centrex-imwidth/2; // Coordinates of the top left corner.
    int y=centrey-imheight/2;
    
    if (x<0 || x>width)
        x=wrap(x,width);
    
    int leftr=random(0,1); // If it's 1 then we reverse it left-right
    int downr=random(0,1); // If it's 1 then we reverse it top-bottom
    
    int istart=0, desti=imwidth+1, istep=1;
    int jstart=0, destj=imheight+1, jstep=1;
    
    if (leftr==1)
    {
        istart=imwidth;
        desti=-1;
        istep=-1;
    }
    
    if (downr==1)
    {
        jstart=imwidth;
        destj=-1;
        jstep=-1;
    }
    
    int imap=-1;
    int jmap=-1;
    
    for (int i=istart; i!=desti; i=i+istep)
    {
        imap++;
        jmap=-1;
        
        for (int j=jstart; j!=destj; j=j+jstep)
        {
            jmap++;
            
            if (shape[shapenumber].point(i,j)==1)
            {
                int xx=x+imap;
                int yy=y+jmap;
                
                if (yy>=0 && yy<=height)
                {
                    if (xx<0 || xx>width)
                        xx=wrap(xx,width);
                    
                    if (land==1)
                        world.setnom(xx,yy,conheight);
                    else
                    {
                        world.setnom(xx,yy,baseheight);
                        world.setmountainheight(xx,yy,0);
                        world.setmountainridge(xx,yy,0);
                    }
                }
            }
        }
    }
}

// Draws a shape from an image onto the map and marks it on another array.

void drawmarkedshape(planet &world, int shapenumber, int centrex, int centrey, bool land, int baseheight, int conheight, vector<vector<bool>> &markedmap, boolshapetemplate shape[])
{
    int width=world.width();
    int height=world.height();
    
    int imheight=shape[shapenumber].ysize()-1;
    int imwidth=shape[shapenumber].xsize()-1;
    
    int x=centrex-imwidth/2; // Coordinates of the top left corner.
    int y=centrey-imheight/2;
    
    if (x<0 || x>width)
        x=wrap(x,width);
    
    int leftr=random(0,1); // If it's 1 then we reverse it left-right
    int downr=random(0,1); // If it's 1 then we reverse it top-bottom
    
    int istart=0, desti=imwidth+1, istep=1;
    int jstart=0, destj=imheight+1, jstep=1;
    
    if (leftr==1)
    {
        istart=imwidth;
        desti=-1;
        istep=-1;
    }
    
    if (downr==1)
    {
        jstart=imwidth;
        destj=-1;
        jstep=-1;
    }
    
    int imap=-1;
    int jmap=-1;
    
    for (int i=istart; i!=desti; i=i+istep)
    {
        imap++;
        jmap=-1;
        
        for (int j=jstart; j!=destj; j=j+jstep)
        {
            jmap++;
            
            if (shape[shapenumber].point(i,j)==1)
            {
                int xx=x+imap;
                int yy=y+jmap;
                
                if (yy>=0 && yy<=height)
                {
                    if (xx<0 || xx>width)
                        xx=wrap(xx,width);
                    
                    if (land==1)
                    {
                        world.setnom(xx,yy,conheight);
                        markedmap[xx][yy]=1;
                    }
                    else
                    {
                        world.setnom(xx,yy,baseheight);
                        world.setmountainheight(xx,yy,0);
                        world.setmountainridge(xx,yy,0);
                    }
                }
            }
        }
    }
}

// Draws a shape from an image onto the plateau map.

void drawplateaushape(planet &world, int shapenumber, int centrex, int centrey, int baseheight, int platheight, vector<vector<int>> &plateaumap, boolshapetemplate chainland[])
{
    //int platheightvary=platheight/10; // Maximum amount that the height can vary by.
    //float maxplatheight=100; // platheight*1.5; // Maximum amount that the height can be up to.
    
    int width=world.width();
    int height=world.height();
    
    int imheight=chainland[shapenumber].ysize()-1;
    int imwidth=chainland[shapenumber].xsize()-1;
    
    int x=centrex-imwidth/2; // Coordinates of the top left corner.
    int y=centrey-imheight/2;
    
    if (x<0 || x>width)
        x=wrap(x,width);
    
    int leftr=random(0,1); // If it's 1 then we reverse it left-right
    int downr=random(0,1); // If it's 1 then we reverse it top-bottom
    
    int istart=0, desti=imwidth+1, istep=1;
    int jstart=0, destj=imheight+1, jstep=1;
    
    if (leftr==1)
    {
        istart=imwidth;
        desti=-1;
        istep=-1;
    }
    
    if (downr==1)
    {
        jstart=imwidth;
        destj=-1;
        jstep=-1;
    }
    
    for (int i=istart; i!=desti; i=i+istep)
    {
        for (int j=jstart; j!=destj; j=j+jstep)
        {
            if (chainland[shapenumber].point(i,j)==1)
            {
                int xx=x+1;
                int yy=y+1;
                
                if (yy>=0 && yy<=height)
                {
                    if (xx<0 || xx>width)
                        xx=wrap(xx,width);
                    
                    if (world.sea(xx,yy)==0)
                        plateaumap[xx][yy]=platheight;
                }
            }
        }
    }
}

// This function adds "cuts" to the map, i.e. adds or takes away shapes from the land.

void cuts(planet &world, int cuttotal, int baseheight, int conheight, boolshapetemplate shape[])
{
    int removechance=random(2,3); // The higher this is, the fewer cuts will be taken away.
    
    for (int cutno=1; cutno<=cuttotal; cutno++)
    {
        int shapenumber=random(1,11);
        
        int centrex=random(0,world.width());
        int centrey=random(0,world.height());
        
        if (world.sea(centrex,centrey)==0) // Check to see whether this is going to be in the land or the sea.
            drawshape(world,shapenumber,centrex,centrey,1,baseheight,conheight,shape);
        
        else
        {
            if (random(1,removechance)==1)
                drawshape(world,shapenumber,centrex,centrey,0,baseheight,conheight,shape);
            
        }
    }
}

// This function merges the fractal map into the main map to create more interesting terrain.

void fractalmerge(planet &world, vector<vector<int>> &fractal)
{
    vector<vector<int>> temp(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT)); // This will be used to merge the fractal onto the main map in a varied way.
    
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    
    int grain=2;
    float valuemod=0.5;
    float valuemod2=0.5;
    
    int min=random(30,90);
    int max=100;
    bool extreme=1;
    
    createfractal(temp,width,height,grain,valuemod,valuemod2,min,max,extreme,0);
    
    int addbit=random(0,15);
    addbit=addbit/10.0;
    
    float div1=1.5+addbit; // This is the amount we divide the fractal height by over the sea. The higher this number, the more the fractal will eat away at the land masses.
    
    float div2=div1-1.2; // This is the amount we divide the fractal height by over land. The higher this number, the flatter the land will be.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            int perc;
            
            if (world.sea(i,j)==0)
                perc=65;
            else
                perc=85;
            
            perc=(perc+temp[i][j])/2;
            
            int oldcol=world.map(i,j);
            int newcol=tilt(oldcol,fractal[i][j]/div1,perc);
            
            if (newcol>sealevel)
                newcol=tilt(oldcol,fractal[i][j]/div2,perc);
            
            if (newcol<=sealevel)
                world.setnom(i,j,newcol);
        }
    }
}

// Same thing, but for the larger continental style.

void fractalmergemodified(planet &world, vector<vector<int>> &fractal, vector<vector<int>> &plateaumap, vector<vector<bool>> &removedland)
{
    vector<vector<int>> temp(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT)); // This will be used to merge the fractal onto the main map in a varied way.
    
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    int maxelev=world.maxelevation();

    int grain=4;
    float valuemod=0.003;
    float valuemod2=0.003;

    bool extreme=0;
    
    createfractalformodifiedmerging(temp,width,height,grain,valuemod,valuemod2,1,maxelev,extreme);
    
    int tippingpoint=maxelev/4;

    for (int j=0; j<=height; j++)
    {
        for (int i=0; i<=width; i++)
        {
            if (temp[i][j]>tippingpoint)
            {
                if (fractal[i][j]<=sealevel && world.nom(i,j)>sealevel)
                {
                    world.setnom(i,j,fractal[i][j]);
                    removedland[i][j]=1;
                }
            }
        }
    }
}

// This function merges the fractal map into the main map, but only on land.

void fractalmergeland(planet &world, vector<vector<int>> &fractal, int conheight)
{
    int div=150; // The higher this number, the more highlands there will be.
    int maxamount=25; // The higher this is, the flatter the flat areas will be.
    int minamount=10; // The lower this is, the higher the high areas will be.
    
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    int maxelev=world.maxelevation();
    
    // First we create a template map, which we will use to vary divamount.
    
    vector<vector<int>> templ(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
            templ[i][j]=fractal[i][j];
    }
    
    flip(templ,width,height,1,1);
    int offset=random(1,width);
    shift(templ,width,height,offset);
    
    // Now we use that template to apply the fractal to the map, to create highlands and lowlands.

    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (world.nom(i,j)>sealevel)
            {
                int valtoadd=fractal[i][j]-sealevel;
                
                int divamount=templ[i][j];
                divamount=divamount/div;
                if (divamount>maxamount)
                    divamount=maxamount;
                if (divamount<minamount)
                    divamount=minamount;
                
                valtoadd=valtoadd/divamount;
                
                int newval=conheight+valtoadd;
                
                if (newval>maxelev)
                    newval=maxelev;
                
                if (newval>world.nom(i,j))
                    world.setnom(i,j,newval);
            }
        }
    }
}

// This function does something similar, but adds the fractal map onto the land map.

void fractaladdland(planet &world, vector<vector<int>> &fractal)
{
    int div=300; //150; // The higher this number, the more highlands there will be.
    int maxamount=25; // The higher this is, the flatter the flat areas will be.
    int minamount=10; // The lower this is, the higher the high areas will be.
    
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    int maxelev=world.maxelevation();
    
    // First we create a template map, which we will use to vary divamount.
    
    vector<vector<int>> templ(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
            templ[i][j]=fractal[i][j];
    }
    
    flip(templ,width,height,1,1);
    int offset=random(1,width);
    shift(templ,width,height,offset);
    
    // Now we use that template to apply the fractal to the map, to create highlands and lowlands.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (world.nom(i,j)>sealevel)
            {
                int valtoadd=fractal[i][j]-sealevel;
                
                int divamount=templ[i][j];
                divamount=divamount/div;
                if (divamount>maxamount)
                    divamount=maxamount;
                if (divamount<minamount)
                    divamount=minamount;
                
                valtoadd=valtoadd/divamount;
                
                int newval=world.nom(i,j)+valtoadd;
                
                if (newval>maxelev)
                    newval=maxelev;
                
                if (newval>world.nom(i,j))
                    world.setnom(i,j,newval);
            }
        }
    }
}

// This function removes any inland seas from the map. level=level to fill them in to.

void removeseas(planet &world, int level)
{
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    
    int minseasize=(width*height)/4;
    
    for (int j=0; j<=height; j++)
    {
        int amount=(world.nom(width-1,j)+world.nom(0,j))/2;
        world.setnom(width,j,amount);
    }
    
    vector<vector<bool>> checked(ARRAYWIDTH,vector<bool>(ARRAYWIDTH,ARRAYHEIGHT)); // This array marks which sea areas have already been scanned.
    
    vector<vector<int>> area(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT)); // This array marks the area of each bit of sea.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            checked[i][j]=0;
            area[i][j]=0;
        }
    }
    
    int seax=-1;
    int seay=-1;

    // First, we find the area of each section of sea and mark them on the area array.
    
    for (int i=width-1; i>0; i--)
    {
        for (int j=height-1; j>0; j--)
        {
            if (world.nom(i,j)<=sealevel && checked[i][j]==0 && area[i][j]==0)
            {
                int size=areacheck(world,checked,i,j);
                
                for (int k=0; k<=width; k++) // Clear the checked array and mark all adjoining points with this area.
                {
                    for (int l=0; l<=height; l++)
                    {
                        if (checked[k][l]==1)
                        {
                            area[k][l]=size;
                            checked[k][l]=0;
                        }
                    }
                }
            }
        }
    }
    
    // Now we just find a point that's in the largest area of sea.
    
    int largest=0;
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (area[i][j]>largest)
            {
                largest=area[i][j];
                seax=i;
                seay=j;
            }
        }
    }
    
    // Now we simply remove any sea tiles that aren't part of the largest area of sea.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (area[i][j]!=largest)
                world.setnom(i,j,level);
            
            if (world.nom(i,j)>sealevel && world.nom(i,j)<level) // Remove any odd low-lying bits around the areas that have just been filled in.
                world.setnom(i,j,level);
        }
    }
}

// This tells how large a given area of sea is.

int areacheck (planet &world, vector<vector<bool>> &checked, int startx, int starty)
{
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    
    int total=0;
    
    int row[]={-1,0,0,1};
    int col[]={0,-1,1,0};
    
    sf::Vector2i node;
    
    node.x=startx;
    node.y=starty;
    
    queue<sf::Vector2i> q; // Create a queue
    q.push(node); // Put our starting node into the queue
    
    while (q.empty()!=1) // Keep going while there's anything left in the queue
    {
        node=q.front(); // Take the node at the front of the queue
        q.pop(); // And then pop it out of the queue
        
        if (node.x>=0 && node.x<=width && node.y>=0 && node.y<=height && world.nom(node.x,node.y)<=sealevel)
        {
            total++;
            checked[node.x][node.y]=1;
            
            for (int k=0; k<4; k++) // Look at the four neighbouring nodes in turn
            {
                sf::Vector2i nextnode;
                
                nextnode.x=node.x+row[k];
                nextnode.y=node.y+col[k];
                
                if (nextnode.x>width)
                    nextnode.x=0;
                
                if (nextnode.x<0)
                    nextnode.x=width;
                
                if (nextnode.y>=0 && nextnode.y<height)
                {
                    if (world.nom(nextnode.x,nextnode.y)<=sealevel && checked[nextnode.x][nextnode.y]==0) // If this node is sea
                    {
                        checked[nextnode.x][nextnode.y]=1;
                        q.push(nextnode); // Put that node onto the queue
                    }
                }
            }
        }
    }
    
    return total;
}

// This marks an area of sea with the size of that area.

int markseasize (planet &world, vector<vector<bool>> &checked, vector<vector<int>> &area, int size, int startx, int starty)
{
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    
    int row[]={-1,0,0,1};
    int col[]={0,-1,1,0};
    
    sf::Vector2i node;
    
    node.x=startx;
    node.y=starty;
    
    queue<sf::Vector2i> q; // Create a queue
    q.push(node); // Put our starting node into the queue
    
    while (q.empty()!=1) // Keep going while there's anything left in the queue
    {
        node=q.front(); // Take the node at the front of the queue
        q.pop(); // And then pop it out of the queue
        
        if (node.x>=0 && node.x<=width && node.y>=0 && node.y<=height && world.nom(node.x,node.y)<=sealevel)
        {
            area[node.x][node.y]=size;
            checked[node.x][node.y]=1;
            
            for (int k=0; k<4; k++) // Look at the four neighbouring nodes in turn
            {
                sf::Vector2i nextnode;
                
                nextnode.x=node.x+row[k];
                nextnode.y=node.y+col[k];
                
                if (nextnode.x>width)
                    nextnode.x=0;
                
                if (nextnode.x<0)
                    nextnode.x=width;
                
                if (nextnode.y>=0 && nextnode.y<height)
                {
                    if (world.nom(nextnode.x,nextnode.y)<=sealevel && checked[nextnode.x][nextnode.y]==0) // If this node is sea
                    {
                        checked[nextnode.x][nextnode.y]=1;
                        q.push(nextnode); // Put that node onto the queue
                    }
                }
            }
        }
    }
    
    return 0;
}

// This function removes straight edges on the coastlines.

void removestraights(planet &world)
{
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    
    int maxlength=4; // Any straights longer than this will be disrupted.
    
    int minsize=2;
    int maxsize=4; // Range of sizes for the chunks to be cut out of the coasts.
    
    for (int i=0; i<=width; i++) // First do vertical/horizontal lines.
    {
        for (int j=0; j<=height; j++)
        {
            if (world.map(i,j)<=sealevel)
            {
                if (j>0)
                {
                    if (world.map(i,j-1)>sealevel) // Sea to the south, land to the north
                    {
                        bool keepgoing=1;
                        int x=i;
                        int xx=x;
                        int y=j;
                        
                        do
                        {
                            if (world.map(x,y)>sealevel)
                                keepgoing=0;
                            
                            if (world.map(x,y-1)<=sealevel)
                                keepgoing=0;
                            
                            x++;
                            
                            xx=x;
                            
                            if (xx>width)
                                xx=wrap(xx,width);
                            
                        } while (keepgoing==1);
                        
                        if (x>i+maxlength) // If this line is too long
                        {
                            x=i+(x-i)/2; // Move to the middle of the line
                            
                            int size=random(minsize,maxsize);
                            disruptseacoastline(world,x,y,world.map(i,j),0,size);
                        }
                    }
                }
                
                if (j<height)
                {
                    if (world.map(i,j+1)>sealevel) // Sea to the north, land to the south
                    {
                        bool keepgoing=1;
                        int x=i;
                        int xx=x;
                        int y=j;
                        
                        do
                        {
                            if (world.map(x,y)>sealevel)
                                keepgoing=0;
                            
                            if (world.map(x,y+1)<=sealevel)
                                keepgoing=0;
                            
                            x++;
                            
                            xx=x;
                            
                            if (xx>width)
                                xx=wrap(xx,width);
                            
                        } while (keepgoing==1);
                        
                        if (x>i+maxlength) // If this line is too long
                        {
                            if (x>i+maxlength) // If this line is too long
                            {
                                x=i+(x-i)/2; // Move to the middle of the line
                                
                                int size=random(minsize,maxsize);
                                disruptseacoastline(world,x,y,world.map(i,j),0,size);
                            }
                        }
                    }
                }
                
                if (world.map(i+1,j)>sealevel) // Sea to the west, land to the east
                {
                    bool keepgoing=1;
                    int x=i;
                    int y=j;
                    int xx=x+1;
                    
                    if (xx>width)
                        xx=0;
                    
                    do
                    {
                        if (world.map(x,y)>sealevel)
                            keepgoing=0;
                        
                        if (world.map(xx,y)<=sealevel)
                            keepgoing=0;
                        
                        y++;
                        
                        if (y>height-1)
                            keepgoing=0;
                        
                    } while (keepgoing==1);
                    
                    if (y>j+maxlength) // If this line is too long
                    {
                        y=j+(y-j)/2; // Move to the middle of the line
                        
                        int size=random(minsize,maxsize);
                        disruptseacoastline(world,x,y,world.map(i,j),0,size);
                    }
                }
                
                if (world.map(i-1,j)>sealevel) // Sea to the east, land to the west
                {
                    bool keepgoing=1;
                    int x=i;
                    int y=j;
                    int xx=i-1;
                    
                    if (xx<0)
                        xx=width;
                    
                    do
                    {
                        if (world.map(x,y)>sealevel)
                            keepgoing=0;
                        
                        if (world.map(x-1,y)<=sealevel)
                            keepgoing=0;
                        
                        y++;
                        
                        if (y>height-1)
                            keepgoing=0;
                        
                    } while (keepgoing==1);
                    
                    if (y>j+maxlength) // If this line is too long
                    {
                        y=j+(y-j)/2; // Move to the middle of the line
                        
                        int size=random(minsize,maxsize);
                        disruptseacoastline(world,x,y,world.map(i,j),0,size);
                    }
                }
            }
        }
    }
    
    for (int i=0; i<=width; i++) // Now do diagonals.
    {
        for (int j=1; j<height; j++)
        {
            if (world.map(i,j)>sealevel)
            {
                if (northwestlandonly(world,i,j)==1)
                {
                    int ii=i+1;
                    
                    if (ii>width)
                        ii=0;
                    
                    int jj=j-1;

                    if (northwestlandonly(world,ii,jj)==1)
                    {
                        int iii=i-1;
                        
                        if (iii<0)
                            iii=width;
                        
                        int jjj=j+1;
                        
                        if (northwestlandonly(world,iii,jjj)==1)
                        {
                            int choose=random(1,3);
                            
                            if (choose==1)
                                world.setnom(i,j,world.nom(ii,jjj));
                            
                            if (choose==2)
                                world.setnom(ii,j,world.nom(i,j));
                            
                            if (choose==3)
                                world.setnom(i,jjj,world.nom(i,j));
                        }
                    }
                }
                
                if (northeastlandonly(world,i,j)==1)
                {
                    int ii=i+1;
                    
                    if (ii>width)
                        ii=0;
                    
                    int jj=j+1;
                    
                    if (northeastlandonly(world,ii,jj)==1)
                    {
                        int iii=i-1;
                        
                        if (iii<0)
                            iii=width;
                        
                        int jjj=j-1;
                        
                        if (northeastlandonly(world,iii,jjj)==1)
                        {
                            int choose=random(1,3);
                            
                            if (choose==1)
                                world.setnom(i,j,world.nom(iii,jj));
                            
                            if (choose==2)
                                world.setnom(iii,j,world.nom(i,j));
                            
                            if (choose==3)
                                world.setnom(i,jj,world.nom(i,j));
                        }
                    }
                }
                
                if (southwestlandonly(world,i,j)==1)
                {
                    int ii=i+1;
                    
                    if (ii>width)
                        ii=0;
                    
                    int jj=j+1;
                    
                    if (southwestlandonly(world,ii,jj)==1)
                    {
                        int iii=i-1;
                        
                        if (iii<0)
                            iii=width;
                        
                        int jjj=j-1;
                        
                        if (southwestlandonly(world,iii,jjj)==1)
                        {
                            int choose=random(1,3);
                            
                            if (choose==1)
                                world.setnom(i,j,world.nom(ii,jjj));
                            
                            if (choose==2)
                                world.setnom(ii,j,world.nom(i,j));
                            
                            if (choose==3)
                                world.setnom(i,jjj,world.nom(i,j));
                        }
                    }
                    
                    
                    
                }
                
                if (southeastlandonly(world,i,j)==1)
                {
                    int ii=i+1;
                    
                    if (ii>width)
                        ii=0;
                    
                    int jj=j-1;
                    
                    if (southeastlandonly(world,ii,jj)==1)
                    {
                        int iii=i-1;
                        
                        if (iii<0)
                            iii=width;
                        
                        int jjj=j+1;
                        
                        if (southeastlandonly(world,iii,jjj)==1)
                        {
                            int choose=random(1,3);
                            
                            if (choose==1)
                                world.setnom(i,j,world.nom(iii,jj));
                            
                            if (choose==2)
                                world.setnom(iii,j,world.nom(i,j));
                            
                            if (choose==3)
                                world.setnom(i,jj,world.nom(i,j));
                        }
                    }
                }
            }
        }
    }
}

// This function cuts a slice out of the coastline, to try to make it more interesting.

void disruptseacoastline(planet &world, int centrex, int centrey, int avedepth, bool raise, int size)
{
    int width=world.width();
    int height=world.height();
    
    int half=size/2;
    
    for (int i=-half; i<=half; i++)
    {
        for (int j=-half; j<=half; j++)
        {
            int ii=centrex+i;
            int jj=centrey+j;
            
            if (ii<0 || ii>width)
                ii=wrap(ii,width);
            
            if (jj>=0 && jj<=height)
            {
                if (world.nom(ii,jj)>avedepth)
                {
                    if (i*i+j*j<half*half+half)
                        world.setnom(ii,jj,avedepth);
                }
            }
        }
    }
}

// This function ensures that channels of sea are at least two pixels wide.

void widenchannels(planet &world)
{
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    
    for (int i=1; i<width; i++) // First remove any one-pixel channels.
    {
        for (int j=1; j<height; j++)
        {
            if (world.sea(i,j))
            {
                if (world.sea(i,j-1)==0 && world.sea(i,j+1)==0)
                    world.setnom(i,j-1,sealevel-15);
                
                if (world.sea(i-1,j)==0 && world.sea(i+1,j)==0)
                    world.setnom(i-1,j,sealevel-15);
                
                if (world.sea(i-1,j-1)==0 && world.sea(i+1,j+1)==0)
                    world.setnom(i-1,j-1,sealevel-15);
                
                if (world.sea(i-1,j+1)==0 & world.sea(i+1,j-1)==0)
                    world.setnom(i-1,j+1,sealevel-15);
            }
        }
    }
    
    for (int i=2; i<width-1; i++) // Now remove any two-pixel channels.
    {
        for (int j=2; j<height-1; j++)
        {
            if (world.sea(i,j))
            {
                if (world.sea(i,j-1)==0 && world.sea(i,j+1)==1 && world.sea(i,j+2)==0)
                    world.setnom(i,j,sealevel-15);
                
                if (world.sea(i-1,j)==0 && world.sea(i+1,j)==1 && world.sea(i+2,j)==0)
                    world.setnom(i+1,j,sealevel-15);
            }
        }
    }
}

void loweroceans(planet &world)
{
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (world.sea(i,j) && world.nom(i,j)>sealevel-170)
            {
                bool nearland=0;
                
                for (int ii=i-2; ii<=i+2; ii++)
                {
                    int iii=ii;
                    
                    if (iii<0 || iii>width)
                        iii=wrap(iii,width);
                    
                    for (int jj=j-2; jj<=j+2; jj++)
                    {
                        if (jj>=0 && jj<=height)
                        {
                            if (world.sea(iii,jj)==0)
                            {
                                nearland=1;
                                
                                ii=i+2;
                                jj=j+2;
                            }
                        }
                    }
                }
                
                if (nearland==0)
                    world.setnom(i,j,sealevel-170+randomsign(random(0,10)));
                
            }
        }
    }
}

// This function removes any mountains that aren't over land.

void removefloatingmountains(planet &world)
{
    int width=world.width();
    int height=world.height();
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (world.mountainridge(i,j)!=0 && world.sea(i,j)==1)
            {
                for (int dir=1; dir<=8; dir++)
                    deleteridge(world,i,j,dir);
            }
        }
    }
}

// This function ensures that all mountain ridges connect up properly, removing rogue ones.

void cleanmountainridges(planet &world)
{
    int width=world.width();
    int height=world.height();
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (world.mountainheight(i,j)!=0 && world.sea(i,j)==0)
            {
                for (int dir=1; dir<=8; dir++)
                {
                    if (getridge(world,i,j,dir)==1 && getdestinationland(world,i,j,dir)==0) // If there is a ridge going in this direction from this point
                        deleteridge(world,i,j,dir);
                }
            }
        }
    }
}

// This function translates a mountain direction into a binary code.

int getcode(int dir)
{
    if (dir==1)
        return(1);
    
    return(pow(2,dir-1));
}

// This function tells whether a ridge goes from the specified tile in the specified direction.

int getridge(planet &world, int x, int y, int dir)
{
    string n=decimaltobinarystring(world.mountainridge(x,y));
    
    string nn=n.substr(n.length()-dir,1);
    
    if (nn=="1")
        return(1);
    
    return(0);
}

// Same thing, but on a specified vector.

int getridge(vector<vector<int>> &arr, int x, int y, int dir)
{
    string n=decimaltobinarystring(arr[x][y]);
    
    string nn=n.substr(n.length()-dir,1);
    
    if (nn=="1")
        return(1);
    
    return(0);
}

// Same thing, but for an ocean ridge.

int getoceanridge(planet &world, int x, int y, int dir)
{
    string n=decimaltobinarystring(world.oceanridges(x,y));
    
    string nn=n.substr(n.length()-dir,1);
    
    if (nn=="1")
        return(1);
    
    return(0);
}

// This function deletes a ridge going from the specified tile in the specified direction.

void deleteridge(planet &world, int x, int y, int dir)
{
    int width=world.width();
    int height=world.height();
    
    if (x<0 || x>width)
        x=wrap(x,width);
    
    if (y<0 || y>height)
        return;
    
    if (getridge(world,x,y,dir)!=1) // If there isn't a ridge going this way
        return;
    
    int code=getcode(dir);
    
    int currentridge=world.mountainridge(x,y);
    
    currentridge=currentridge-code;
    
    world.setmountainridge(x,y,currentridge);
    
    if (currentridge==0)
        world.setmountainheight(x,y,0);
    
    // Now remove one going the other way.
    
    if (dir==8 || dir==1 || dir==2)
        y--;
    
    if (dir==4 || dir==5 || dir==6)
        y++;
    
    if (dir==2 || dir==3 || dir==4)
        x++;
    
    if (dir==6 || dir==7 || dir==8)
        x--;
    
    if (x<0 || x>width)
        x=wrap(x,width);
    
    if (y<0 || y>height)
        return;
    
    dir=dir+4;
    
    if (dir>8)
        dir=dir-8;
    
    if (getridge(world,x,y,dir)!=1) // If there isn't a ridge going in this direction
        return;
    
    code=getcode(dir);
    
    currentridge=world.mountainridge(x,y);
    
    currentridge=currentridge-code;
    
    world.setmountainridge(x,y,currentridge);
    
    if (currentridge==0)
        world.setmountainheight(x,y,0);
}

// Same thing, but on a specified vector.

void deleteridge(planet &world, vector<vector<int>> &ridgesarr, vector<vector<int>> &heightsarr, int x, int y, int dir)
{
    int width=world.width();
    int height=world.height();
    
    if (x<0 || x>width)
        x=wrap(x,width);
    
    if (y<0 || y>height)
        return;
    
    if (getridge(ridgesarr,x,y,dir)!=1) // If there isn't a ridge going this way
        return;
    
    int code=getcode(dir);
    
    int currentridge=ridgesarr[x][y];
    
    currentridge=currentridge-code;
    
    ridgesarr[x][y]=currentridge;
    
    if (currentridge==0)
        heightsarr[x][y]=0;
    
    // Now remove one going the other way.
    
    if (dir==8 || dir==1 || dir==2)
        y--;
    
    if (dir==4 || dir==5 || dir==6)
        y++;
    
    if (dir==2 || dir==3 || dir==4)
        x++;
    
    if (dir==6 || dir==7 || dir==8)
        x--;
    
    if (x<0 || x>width)
        x=wrap(x,width);
    
    if (y<0 || y>height)
        return;
    
    dir=dir+4;
    
    if (dir>8)
        dir=dir-8;
    
    if (getridge(ridgesarr,x,y,dir)!=1) // If there isn't a ridge going in this direction
        return;
    
    code=getcode(dir);
    
    currentridge=ridgesarr[x][y];
    
    currentridge=currentridge-code;
    
    ridgesarr[x][y]=currentridge;
    
    if (currentridge==0)
        heightsarr[x][y]=0;
}

// Same thing, but for an ocean ridge.

void deleteoceanridge(planet &world, int x, int y, int dir)
{
    int width=world.width();
    int height=world.height();
    
    if (x<0 || x>width)
        x=wrap(x,width);
    
    if (y<0 || y>height)
        return;
    
    if (getoceanridge(world,x,y,dir)!=1) // If there isn't a ridge going this way
        return;
    
    int code=getcode(dir);
    
    int currentridge=world.oceanridges(x,y);
    
    currentridge=currentridge-code;
    
    world.setoceanridges(x,y,currentridge);
    
    // Now remove one going the other way.
    
    if (dir==8 || dir==1 || dir==2)
        y--;
    
    if (dir==4 || dir==5 || dir==6)
        y++;
    
    if (dir==2 || dir==3 || dir==4)
        x++;
    
    if (dir==6 || dir==7 || dir==8)
        x--;
    
    if (x<0 || x>width)
        x=wrap(x,width);
    
    if (y<0 || y>height)
        return;
    
    dir=dir+4;
    
    if (dir>8)
        dir=dir-8;
    
    if (getoceanridge(world,x,y,dir)!=1) // If there isn't a ridge going in this direction
        return;
    
    code=getcode(dir);
    
    currentridge=world.oceanridges(x,y);
    
    currentridge=currentridge-code;
    
    world.setoceanridges(x,y,currentridge);
}

// This function tells whether there is land in the specified direction from this point.

int getdestinationland(planet &world, int x, int y, int dir)
{
    int newx=x;
    int newy=y;
    
    if (dir==8 || dir==1 || dir==2)
        newy--;
    
    if (dir==4 || dir==5 || dir==6)
        newy++;
    
    if (dir==2 || dir==3 || dir==4)
        newx++;
    
    if (dir==6 || dir==7 || dir==8)
        newx--;
    
    if (newy<0 || newy>world.height())
        return(0);
    
    if (newx<0 || newx>world.width())
        newx=wrap(newx,world.width());
    
    // newx and newy are the coordinates of where this ridge is pointing to.
    
    if (world.sea(newx,newy)==1)
        return(0);
    
    return(1);
}

// This function raises the land beneath mountains.

void raisemountainbases(planet &world, vector<vector<int>> &mountaindrainage)
{
    int width=world.width();
    int height=world.height();

    int maxradius=6; //4;
    int volcanomaxradius=2;
    float heightreduce=0.5; //0.5; // Multiply the extra height by this each time.
    float extraheightreduce=0.045; // Each time, reduce the heightreduce by this.
    
    bool volcano=0;
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            volcano=0;
            
            int mheights=world.mountainheight(i,j);
            
            if (mheights==0 && world.strato(i,j)==0)
            {
                mheights=world.volcano(i,j);
                volcano=1;
            }
            
            if (mheights<0)
                mheights=0-mheights;
            
            if (mheights!=0)
            {
                heightreduce=0.7; //0.5;
                
                float baseheightfraction=random(20,50); //(40,60);
                
                if (volcano==1)
                    baseheightfraction=random(100,200);
                
                baseheightfraction=baseheightfraction/100.0;
                float baseheight=mheights*baseheightfraction;
                
                int thismaxradius=maxradius;
                
                if (volcano==1)
                    thismaxradius=volcanomaxradius;
                
                for (int radius=1; radius<=thismaxradius; radius++) // Go through ever-increasing circles
                {
                    for (int k=-radius; k<=radius; k++)
                    {
                        for (int l=-radius; l<=radius; l++)
                        {
                            if (k*k+l*l<radius*radius+radius) // If we're within the current circle
                            {
                                int ll=j+l;
                                
                                if (ll>=0 && ll<=height)
                                {
                                    int kk=i+k;
                                    
                                    if (kk<0 || kk>width)
                                        kk=wrap(kk,width);
                                    
                                    if (world.sea(kk,ll)==0 && mountaindrainage[kk][ll]<baseheight)
                                        mountaindrainage[kk][ll]=baseheight;
                                }
                            }
                        }
                    }
                    
                    baseheight=baseheight*heightreduce;
                    //heightreduce=heightreduce-extraheightreduce;

                }
            }
        }
    }
    
    // Now add the extra heights onto the actual map.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (world.sea(i,j)==0)
                world.setnom(i,j,world.nom(i,j)+mountaindrainage[i][j]); //*0.7);
        }
    }
}

// This function removes plateaux that are too close to the coast.

void erodeplateaux(planet &world, vector<vector<int>> &plateaumap)
{
    int width=world.width();
    int height=world.height();
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (plateaumap[i][j]!=0)
            {
                int nearsea=0;
                
                for (int ii=i-6; ii<=i+6; ii++)
                {
                    int iii=ii;
                    if (iii<0 || iii>width)
                        iii=wrap(iii,width);
                    
                    for (int jj=j-6; jj<=j+6; jj++)
                    {
                        if (jj>=0 && jj<=height)
                        {
                            if (world.sea(iii,jj)==1)
                            {
                                nearsea=1;
                                
                                ii=i+6;
                                jj=j+6;
                            }
                        }
                    }
                }
                
                if (nearsea==1)
                    plateaumap[i][j]=0;
                
            }
        }
    }
}

// This function adds the plateaux to the main map.

void addplateaux(planet &world, vector<vector<int>> &plateaumap, int conheight)
{
    int width=world.width();
    int height=world.height();
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (plateaumap[i][j]!=0 && world.sea(i,j)==0 && conheight+plateaumap[i][j]>world.map(i,j))
                world.setnom(i,j,conheight+plateaumap[i][j]);
        }
    }
}

// This function smoothes, but without turning any land to sea or vice versa.

void smoothland(planet &world, int amount)
{
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    float maxelev=world.maxelevation();
    float seaamount=amount*3;
    float div=maxelev/seaamount;
    
    int grain=8; // Level of detail on this fractal map.
    float valuemod=0.2;
    int v=random(3,6);
    float valuemod2=v;
    
    vector<vector<int>> fractal(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
            fractal[i][j]=0;
    }
    
    createfractal(fractal,width,height,grain,valuemod,valuemod2,1,maxelev,0,0);
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            int crount=0;
            int ave=0;
            int origland=0;
            int thisamount=amount;
            
            if (world.sea(i,j)==0) // Check to see whether this point is originally land or sea
                origland=1;
            else
                thisamount=fractal[i][j]/div;
            
            if (thisamount<1)
                thisamount=1;
            
            bool goahead=1;
            int seacheck=5;
            
            if (goahead==1)
            {
                for (int k=i-thisamount; k<=i+thisamount; k++)
                {
                    int kk=k;
                    
                    if (kk<0 || kk>width)
                        kk=wrap(kk,width);
                    
                    for (int l=j-thisamount; l<=j+thisamount; l++)
                    {
                        if (l>0 && l<height)
                        {
                            ave=ave+world.nom(kk,l);
                            crount++;
                        }
                    }
                }
                
                if (crount>0)
                {
                    ave=ave/crount;
                    
                    if (ave>0 && ave<maxelev)
                    {
                        int newland=0;
                        
                        if (ave>sealevel) // Now check to see whether the new value is land or sea
                            newland=1;
                        
                        if (origland==0 && newland==0)
                            world.setnom(i,j,ave);
                    }
                }
            }
        }
    }
}

/*
void smoothland(planet &world, int amount)
{
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    float maxelev=world.maxelevation();
    float seaamount=amount*3;
    float div=maxelev/seaamount;
    
    int grain=8; // Level of detail on this fractal map.
    float valuemod=0.2;
    int v=random(3,6);
    float valuemod2=v;
    
    vector<vector<int>> fractal(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
            fractal[i][j]=0;
    }
    
    createfractal(fractal,width,height,grain,valuemod,valuemod2,1,maxelev,0,0);

    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            int crount=0;
            int ave=0;
            int origland=0;
            int thisamount=amount;
            
            if (world.sea(i,j)==0) // Check to see whether this point is originally land or sea
                origland=1;
            else
                thisamount=fractal[i][j]/div;
            
            if (thisamount<1)
                thisamount=1;
            
            bool goahead=1;
            
            if (world.coast(i,j)==1)
                goahead=0;

            if (goahead==1)
            {
                if (world.sea(i,j)==0)
                {
                    for (int k=i-thisamount; k<=i+thisamount; k++)
                    {
                        int kk=k;
                        
                        if (kk<0 || kk>width)
                            kk=wrap(kk,width);
                        
                        for (int l=j-thisamount; l<=j+thisamount; l++)
                        {
                            if (l>0 && l<height && world.sea(kk,l)==0)
                            {
                                ave=ave+world.nom(kk,l);
                                crount++;
                            }
                        }
                    }
                    
                    if (crount>0)
                    {
                        ave=ave/crount;
                        world.setnom(i,j,ave);
                    }
                }
                else
                {
                    for (int k=i-thisamount; k<=i+thisamount; k++)
                    {
                        int kk=k;
                        
                        if (kk<0 || kk>width)
                            kk=wrap(kk,width);
                        
                        for (int l=j-thisamount; l<=j+thisamount; l++)
                        {
                            if (l>0 && l<height && world.sea(kk,l)==1 && world.coast(kk,l)==0)
                            {
                                ave=ave+world.nom(kk,l);
                                crount++;
                            }
                        }
                    }
                    
                    if (crount>0)
                    {
                        ave=ave/crount;
                        world.setnom(i,j,ave);
                    }
                }
            }
        }
    }
}
*/
 
// This function creates areas of raised elevation around where canyons might form later.

void createextraelev(planet &world)
{
    int width=world.width();
    int height=world.height();
    
    vector<vector<int>> tempelev(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
            tempelev[i][j]=0;
    }
    
    float maxextra=1000; // Maximum amount that the extraelev array can have.
    float maxelev=world.maxelevation();
    
    float div=maxelev/(maxextra*2.0);
    
    // First create a fractal for this map.
    
    int grain=16; // Level of detail on this fractal map.
    float valuemod=0.2;
    float valuemod2=0.2;
    
    createfractal(tempelev,width,height,grain,valuemod,valuemod2,1,maxelev,0,0);
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            world.setextraelev(i,j,tempelev[i][j]);
            tempelev[i][j]=0;
        }
    }
    
    int v=random(3,6);
    valuemod2=v;
    
    createfractal(tempelev,width,height,grain,valuemod,valuemod2,1,maxelev,0,0);
    
    // Now alter it a bit.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (world.sea(i,j)==1)
                world.setextraelev(i,j,0);
            else
            {
                float e=world.extraelev(i,j);
                e=e/div;
                e=e-maxextra;
                
                if (e<0)
                    e=0;
                
                if (e>maxextra)
                    e=maxextra;
                
                world.setextraelev(i,j,e);
            }
        }
    }
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (world.sea(i,j)==0)
            {
                float e=tempelev[i][j];
                e=e/div;
                e=e-maxextra;
                
                tempelev[i][j]=e;
                
                if (tempelev[i][j]<0)
                    tempelev[i][j]=0;
                
                if (tempelev[i][j]<world.extraelev(i,j))
                    world.setextraelev(i,j,tempelev[i][j]);
            }
        }
    }
    
    // Now smooth it
    
    world.smoothextraelev(2);
    world.smoothextraelev(5);
}

// This function moves the map so that the edge is mostly sea.

void adjustforsea(planet &world)
{
    int width=world.width();
    int height=world.height();
    
    vector<int> arr(ARRAYWIDTH);
    
    for (int n=0; n<=width; n++)
        arr[n]=0;
    
    for (int n=0; n<=width; n++)
    {
        int land=landcheck(world,n);
        
        if (land==0)
        {
            bool keepgoing=1;
            int m=n+1;
            int total=1;
            
            do
            {
                if (landcheck(world,m)==0)
                {
                    total++;
                    m++;
                    
                    if (m>width)
                        m=0;
                }
                else
                    keepgoing=0;
                
            } while (keepgoing==1);
            
            arr[n]=total;
        }
    }
    
    int max=0;
    int theone=0;
    
    for (int n=1; n<=width; n++)
    {
        if (arr[n]>max)
        {
            theone=n;
            max=arr[n];
        }
    }
    
    if (max>1)
        theone=theone+max/2;
    
    else // If we haven't found any clear ocean from north to south
    {
        for (int n=0; n<=width; n++)
            arr[n]=landcheck(world,n);
        
        int min=height;
        theone=0;
        
        for (int n=0; n<=width; n++)
        {
            if (arr[n]<min && arr[n]!=-1)
            {
                theone=n;
                min=arr[n];
            }
        }
    }
    
    world.shiftterrain(theone);
    
    // Now we remove the "seam" caused by the above.
    
    removeseam(world,width-theone);
    
    return;
}

// This function removes a vertical seam in the world from the given x coordinate.

void removeseam(planet &world, int i)
{
    int width=world.width();
    int height=world.height();
    
    int mountainaddchance=3; // Chance of adding a ridge connecting to existing ones to the side. Higher=less likely.
    
    int iminus=i-2;
    int iplus=i+2;
    
    if (iminus<0)
        iminus=width-1;
    
    if (iminus>width)
        iminus=1;
    
    for (int j=0; j<=height; j++)
    {
        world.setnom(i,j,(world.nom(iminus,j)+world.nom(iplus,j))/2);
        world.setextraelev(i,j,(world.extraelev(iminus,j)+world.extraelev(iplus,j))/2);
        
        bool mountainsadded=0;
        int dir=0;
        
        if (world.mountainheight(iminus,j)!=0)
        {
            if (random(1,mountainaddchance)==1)
            {
                dir=getcode(3);
                world.setmountainridge(iminus,j,world.mountainridge(iminus,j)+dir);
                
                dir=getcode(7);
                world.setmountainridge(i,j,world.mountainridge(i,j)+dir);
                
                mountainsadded=1;
            }
        }
        
        if (world.mountainheight(iplus,j)!=0)
        {
            if (random(1,mountainaddchance)==1)
            {
                dir=getcode(7);
                world.setmountainridge(iplus,j,world.mountainridge(iplus,j)+dir);
                
                dir=getcode(3);
                world.setmountainridge(i,j,world.mountainridge(i,j)+dir);
                
                mountainsadded=1;
            }
        }
        
        if (j>0)
        {
            if (world.mountainheight(iminus,j-1)!=0)
            {
                if (random(1,mountainaddchance)==1)
                {
                    dir=getcode(4);
                    world.setmountainridge(iminus,j-1,world.mountainridge(iminus,j-1)+dir);
                    
                    dir=getcode(8);
                    world.setmountainridge(i,j,world.mountainridge(i,j)+dir);
                    
                    mountainsadded=1;
                }
            }
            
            if (world.mountainheight(iplus,j-1)!=0)
            {
                if (random(1,mountainaddchance)==1)
                {
                    dir=getcode(6);
                    world.setmountainridge(iplus,j-1,world.mountainridge(iplus,j-1)+dir);
                    
                    dir=getcode(2);
                    world.setmountainridge(i,j,world.mountainridge(i,j)+dir);
                    
                    mountainsadded=1;
                }
            }
        }
        
        if (j<height)
        {
            if (world.mountainheight(iminus,j+1)!=0)
            {
                if (random(1,mountainaddchance)==1)
                {
                    dir=getcode(2);
                    world.setmountainridge(iminus,j+1,world.mountainridge(iminus,j+1)+dir);
                    
                    dir=getcode(6);
                    world.setmountainridge(i,j,world.mountainridge(i,j)+dir);
                    
                    mountainsadded=1;
                }
            }
            if (world.mountainheight(iplus,j+1)!=0)
            {
                if (random(1,mountainaddchance)==1)
                {
                    dir=getcode(8);
                    world.setmountainridge(iplus,j+1,world.mountainridge(iplus,j+1)+dir);
                    
                    dir=getcode(4);
                    world.setmountainridge(i,j,world.mountainridge(i,j)+dir);
                    
                    mountainsadded=1;
                }
            }
        }
        
        if (mountainsadded==1)
        {
            int total=0;
            int crount=0;
            
            for (int k=i-1; k<=i+1; k++)
            {
                int kk=k;
                
                if (kk<0 || kk>width)
                    kk=wrap(kk,width);
                
                for (int l=j-1; l<j+1; l++)
                {
                    if (l>=0 && l<=height && world.mountainheight(kk,l)!=0)
                    {
                        total=total+world.mountainheight(kk,l);
                        crount++;
                    }
                }
            }
            
            if (crount>0)
                world.setmountainheight(i,j,total/crount);
        }
    }
    
}

// This function checks to see how much land there is at this x position in the global map.

int landcheck(planet &world, int n)
{
    if (n<0 || n>world.width())
        return(-1);
    
    int land=0;
    
    for (int i=0; i<=world.height(); i++)
    {
        if (world.sea(n,i)==0)
            land++;
    }
    
    return (land);
}

// This function fills in depressions, using my implementation of the Planchon-Darboux algorithm. (Faster algorithms exist, e.g. Wang-Liu and Su-Wang et al., but they're beyond my ability to implement.)

void depressionfill(planet &world)
{
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    int maxelev=world.maxelevation();
    
    int e=2; // This is the extra amount added to ensure that everywhere slopes.
    //int depth=2;
    //int maxdepth=2000;
    
    int neighbours[8][2];
    
    neighbours[0][0]=0;
    neighbours[0][1]=-1;
    
    neighbours[1][0]=1;
    neighbours[1][1]=-1;
    
    neighbours[2][0]=1;
    neighbours[2][1]=0;
    
    neighbours[3][0]=1;
    neighbours[3][1]=1;
    
    neighbours[4][0]=0;
    neighbours[4][1]=1;
    
    neighbours[5][0]=-1;
    neighbours[5][1]=1;
    
    neighbours[6][0]=-1;
    neighbours[6][1]=-0;
    
    neighbours[7][0]=-1;
    neighbours[7][1]=-1;
    
    vector<vector<int>> noise(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT)); // This will contain noise that allows us to vary the value of e from tile to tile.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=width; j++)
            noise[i][j]=random(0,5);
    }
    
    vector<vector<int>> filledmap(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT)); // This will be the new version of the map.
    
    for (int i=0; i<=width; i++) // First, fill the new map to a huge height.
    {
        for (int j=0; j<=height; j++)
        {
            if (world.nom(i,j)<=sealevel)
                filledmap[i][j]=world.nom(i,j); // Sea tiles start with the normal heights.
            
            else
            {
                if (world.outline(i,j)==1) // If this is a coastal tile
                {
                    filledmap[i][j]=world.nom(i,j); // Coastal tiles start with the normal heights.
                    //dryupwardcell(world,filledmap,i,j,e,neighbours,noise);
                }
                
                else
                    filledmap[i][j]=maxelev*2; // Other tiles start very high
                
            }
        }
    }
    
    for (int i=0; i<=width; i++) // Now make the edges have the normal heights too.
    {
        filledmap[i][0]=world.nom(i,0);
        filledmap[i][height]=world.nom(i,height);
        
        //dryupwardcell(world,filledmap,i,0,e,neighbours,noise);
        //dryupwardcell(filledmap,i,height,e,neighbours,noise);
        
    }
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if ((j==0 || j==height) && world.nom(i,j)>sealevel)
                dryupwardcell(world,filledmap,i,j,e,neighbours,noise);
        }
    }
    
    // Now we're ready to start!
    
    sf::Vector2i lowest(0,0);
    bool somethingdone=0;
    
    do
    {
        for (int i=0; i<=width; i++) // Eight mini-loops, because there are eight possible ways to scan the map and we must rotate between them.
        {
            for (int j=0; j<=height; j++)
            {
                if (filledmap[i][j]>world.nom(i,j))
                    somethingdone=checkdepressiontile(world,filledmap,i,j,e,neighbours,somethingdone,noise);
            }
        }
        
        if (somethingdone==1)
        {
            somethingdone=0;
            
            for (int j=height; j>=0; j--)
            {
                for (int i=width; i>=0; i--)
                {
                    if (filledmap[i][j]>world.nom(i,j))
                        somethingdone=checkdepressiontile(world,filledmap,i,j,e,neighbours,somethingdone,noise);
                    
                }
            }
        }
        
        if (somethingdone==1)
        {
            somethingdone=0;
            
            for (int i=width; i>=0; i--)
            {
                for (int j=0; j<=height; j++)
                {
                    if (filledmap[i][j]>world.nom(i,j))
                        somethingdone=checkdepressiontile(world,filledmap,i,j,e,neighbours,somethingdone,noise);
                    
                }
            }
        }
        
        if (somethingdone==1)
        {
            somethingdone=0;
            
            for (int j=0; j<=height; j++)
            {
                for (int i=width; i>=0; i--)
                {
                    if (filledmap[i][j]>world.nom(i,j))
                        somethingdone=checkdepressiontile(world,filledmap,i,j,e,neighbours,somethingdone,noise);
                    
                }
            }
        }
        
        if (somethingdone==1)
        {
            somethingdone=0;
            
            for (int i=0; i<=width; i++)
            {
                for (int j=height; j>=0; j--)
                {
                    if (filledmap[i][j]>world.nom(i,j))
                        somethingdone=checkdepressiontile(world,filledmap,i,j,e,neighbours,somethingdone,noise);
                    
                }
            }
        }
        
        if (somethingdone==1)
        {
            somethingdone=0;
            
            for (int j=height; j>=0; j--)
            {
                for (int i=0; i<=width; i++)
                {
                    if (filledmap[i][j]>world.nom(i,j))
                        somethingdone=checkdepressiontile(world,filledmap,i,j,e,neighbours,somethingdone,noise);
                    
                }
            }
        }
        
        if (somethingdone==1)
        {
            somethingdone=0;
            
            for (int i=width; i>0; i--)
            {
                for (int j=height; j>=0; j--)
                {
                    if (filledmap[i][j]>world.nom(i,j))
                        somethingdone=checkdepressiontile(world,filledmap,i,j,e,neighbours,somethingdone,noise);
                    
                }
            }
        }
        
        if (somethingdone==1)
        {
            somethingdone=0;
            
            for (int j=0; j<=height; j++)
            {
                for (int i=0; i<=width; i++)
                {
                    if (filledmap[i][j]>world.nom(i,j))
                        somethingdone=checkdepressiontile(world,filledmap,i,j,e,neighbours,somethingdone,noise);
                    
                }
            }
        }
    } while (somethingdone==1);
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
            world.setnom(i,j,filledmap[i][j]);
        
    }
}

// This does the same thing, but for continental shelves.

void seadepressionfill(planet &world, vector<vector<bool>> &shelves)
{
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    int maxelev=world.maxelevation();
    
    int e=2; // This is the extra amount added to ensure that everywhere slopes.
    //int depth=2;
    //int maxdepth=2000;
    
    int neighbours[8][2];
    
    neighbours[0][0]=0;
    neighbours[0][1]=-1;
    
    neighbours[1][0]=1;
    neighbours[1][1]=-1;
    
    neighbours[2][0]=1;
    neighbours[2][1]=0;
    
    neighbours[3][0]=1;
    neighbours[3][1]=1;
    
    neighbours[4][0]=0;
    neighbours[4][1]=1;
    
    neighbours[5][0]=-1;
    neighbours[5][1]=1;
    
    neighbours[6][0]=-1;
    neighbours[6][1]=-0;
    
    neighbours[7][0]=-1;
    neighbours[7][1]=-1;
    
    vector<vector<int>> noise(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT)); // This will contain noise that allows us to vary the value of e from tile to tile.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=width; j++)
            noise[i][j]=random(0,5);
    }
    
    vector<vector<int>> filledmap(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT)); // This will be the new version of the map.
    
    for (int i=0; i<=width; i++) // First, fill the new map to a huge height.
    {
        for (int j=0; j<=height; j++)
        {
            if (shelves[i][j]==0)
                filledmap[i][j]=world.nom(i,j); // Non-shelf tiles start with the normal heights.
            
            else // Shelf tiles
            {
                bool edge=0;
                
                for (int k=i-1; k<=i+1; k++)
                {
                    int kk=k;
                    
                    if (kk<0 || kk>width)
                        kk=wrap(kk,width);
                    
                    for (int l=j-1; l<=j+1; l++)
                    {
                        if (l>=0 && l<=height)
                        {
                            if (shelves[kk][l]==0)
                            {
                                edge=1;
                                k=i+1;
                                l=j+1;
                            }
                        }
                    }
                }
                
                if (edge==1) // If this is on the edge of the shelf
                    filledmap[i][j]=world.nom(i,j); // Edge tiles start with the normal heights.
                else
                    filledmap[i][j]=maxelev*2; // Other tiles start very high
                
            }
        }
    }
    
    for (int i=0; i<=width; i++) // Now make the edges have the normal heights too.
    {
        filledmap[i][0]=world.nom(i,0);
        filledmap[i][height]=world.nom(i,height);
    }
    
    sf::Vector2i lowest(0,0);
    bool somethingdone=0;
    
    do
    {
        for (int i=0; i<=width; i++) // Eight mini-loops, because there are eight possible ways to scan the map and we must rotate between them.
        {
            for (int j=0; j<=height; j++)
            {
                if (filledmap[i][j]>world.nom(i,j))
                    somethingdone=checkdepressiontile(world,filledmap,i,j,e,neighbours,somethingdone,noise);
            }
        }
        
        if (somethingdone==1)
        {
            somethingdone=0;
            
            for (int j=height; j>=0; j--)
            {
                for (int i=width; i>=0; i--)
                {
                    if (filledmap[i][j]>world.nom(i,j))
                        somethingdone=checkdepressiontile(world,filledmap,i,j,e,neighbours,somethingdone,noise);
                    
                }
            }
        }
        
        if (somethingdone==1)
        {
            somethingdone=0;
            
            for (int i=width; i>=0; i--)
            {
                for (int j=0; j<=height; j++)
                {
                    if (filledmap[i][j]>world.nom(i,j))
                        somethingdone=checkdepressiontile(world,filledmap,i,j,e,neighbours,somethingdone,noise);
                    
                }
            }
        }
        
        if (somethingdone==1)
        {
            somethingdone=0;
            
            for (int j=0; j<=height; j++)
            {
                for (int i=width; i>=0; i--)
                {
                    if (filledmap[i][j]>world.nom(i,j))
                        somethingdone=checkdepressiontile(world,filledmap,i,j,e,neighbours,somethingdone,noise);
                    
                }
            }
        }
        
        if (somethingdone==1)
        {
            somethingdone=0;
            
            for (int i=0; i<=width; i++)
            {
                for (int j=height; j>=0; j--)
                {
                    if (filledmap[i][j]>world.nom(i,j))
                        somethingdone=checkdepressiontile(world,filledmap,i,j,e,neighbours,somethingdone,noise);
                    
                }
            }
        }
        
        if (somethingdone==1)
        {
            somethingdone=0;
            
            for (int j=height; j>=0; j--)
            {
                for (int i=0; i<=width; i++)
                {
                    if (filledmap[i][j]>world.nom(i,j))
                        somethingdone=checkdepressiontile(world,filledmap,i,j,e,neighbours,somethingdone,noise);
                    
                }
            }
        }
        
        if (somethingdone==1)
        {
            somethingdone=0;
            
            for (int i=width; i>0; i--)
            {
                for (int j=height; j>=0; j--)
                {
                    if (filledmap[i][j]>world.nom(i,j))
                        somethingdone=checkdepressiontile(world,filledmap,i,j,e,neighbours,somethingdone,noise);
                    
                }
            }
        }
        
        if (somethingdone==1)
        {
            somethingdone=0;
            
            for (int j=0; j<=height; j++)
            {
                for (int i=0; i<=width; i++)
                {
                    if (filledmap[i][j]>world.nom(i,j))
                        somethingdone=checkdepressiontile(world,filledmap,i,j,e,neighbours,somethingdone,noise);
                    
                }
            }
        }
    } while (somethingdone==1);
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
            world.setnom(i,j,filledmap[i][j]);
        
    }
}

// This does the actual checking/filling of each tile.

bool checkdepressiontile(planet &world, vector<vector<int>> &filledmap, int i, int j, int e, int neighbours[8][2], bool somethingdone, vector<vector<int>> &noise)
{
    int width=world.width();
    int height=world.height();
    
    int start=random(0,7);
    
    for (int n=start; n<=start+7; n++)
    {
        int nn=wrap(n,7);
        
        int k=i+neighbours[nn][0];
        
        if (k<0 || k>width)
            k=wrap(k,width);
        
        int l=j+neighbours[nn][1];
        
        if (l>=0 && l<=height)
        {
            int ee=0;
            
            if (k==i || l==j)
                ee=e+1+noise[i][j];
            
            else
                ee=e+noise[i][j];
            
            if (world.nom(i,j)>=filledmap[k][l]+ee)
            {
                filledmap[i][j]=world.nom(i,j);
                somethingdone=1;
                
                //dryupwardcell(world,filledmap,i,j,e,neighbours,noise)
            }
            else
            {
                if (filledmap[i][j]>filledmap[k][l]+ee)
                {
                    filledmap[i][j]=filledmap[k][l]+ee;
                    somethingdone=1;
                }
            }
        }
    }
    
    return (somethingdone);
}

// This function finds the lowest neighbouring point on the filledmap.

sf::Vector2i lowestfill(planet &world, vector<vector<int>> &filledmap, int x, int y, int neighbours[8][2])
{
    int width=world.width();
    int height=world.height();
    
    sf::Vector2i lowest(-1,-1);
    
    int lowestheight=world.maxelevation()*3;
    
    int start=random(0,7);
    
    for (int n=start; n<=start+7; n++)
    {
        int nn=wrap(n,7);
        
        int i=x+neighbours[nn][0];
        
        if (i<0 || i>width)
            i=wrap(i,width);
        
        int j=y+neighbours[nn][1];
        
        if (j>=0 && j<=height && filledmap[i][j]<lowestheight)
        {
            lowest.x=i;
            lowest.y=j;
            lowestheight=filledmap[i][j];
        }
    }
    
    return (lowest);
}

// This function does recursive checking from points we've already done.

void dryupwardcell(planet &world, vector<vector<int>> &filledmap, int x, int y, int e, int neighbours[8][2], vector<vector<int>> &noise)
{
    int width=world.width();
    int height=world.height();
    int maxelev=world.maxelevation();
    
    sf::Vector2i lowest(-1,-1);
    
    int start=random(0,7);
    
    for (int n=start; n<=start+7; n++)
    {
        int nn=wrap(n,7);
        
        int i=x+neighbours[nn][0];
        
        if (i<0 || i>width)
            i=wrap(i,width);
        
        int j=y+neighbours[nn][1];
        
        if (j>=0 && j<=height)
        {
            int ee=0;
            
            if (i==x || j==y)
                ee=e+1+noise[i][j];
            else
                ee=e+noise[i][j];
            
            if (filledmap[i][j]==maxelev*2)
            {
                //int currentelev=filledmap[i][j];
                
                lowest=lowestfill(world,filledmap,i,j,neighbours);
                
                if (world.nom(i,j)>=filledmap[x][y]+ee)
                {
                    filledmap[i][j]=world.nom(i,j);
                    
                    dryupwardcell(world,filledmap,i,j,e,neighbours,noise);
                }
            }
        }
    }
}

// This function adds a bit of noise to the land.

void addlandnoise(planet &world)
{
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    int maxelev=world.maxelevation();
    
    int maxadjust=10;
    int adjustchance=5; // The lower this is, the more there will be.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (world.nom(i,j)>sealevel && random(1,adjustchance)==1)
            {
                int adjust=randomsign(random(1,maxadjust));
                
                int newamount=world.nom(i,j)+adjust;
                
                if (newamount<=sealevel)
                    newamount=sealevel;
                
                if (newamount>maxelev)
                    newamount=maxelev;
                
                world.setnom(i,j,newamount);
            }
        }
    }
}

// This function lowers the heights of all coastal tiles, which makes for more natural-looking coastlines in the regional map.

void lowercoasts(planet &world)
{
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    
    vector<vector<int>> loweredland(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (world.outline(i,j))
            {
                //world.setnom(i,j,sealevel+1);
                
                int amount=(world.nom(i,j)-sealevel)/4*3;
                
                world.setnom(i,j,world.nom(i,j)-amount);
                
                for (int ii=i-1; ii<=i+1; ii++) // Now lower the neighbouring tiles by a bit too.
                {
                    int iii=ii;
                    
                    if (iii<0 || iii>width)
                        iii=wrap(iii,width);
                    
                    for (int jj=j-1; jj<=j+1; jj++)
                    {
                        if (jj>=0 && jj<=height && iii!=i & jj!=j && loweredland[iii][jj]==0)
                        {
                            amount=(world.nom(iii,jj)-sealevel)/4;
                            world.setnom(iii,jj,world.nom(iii,jj)-amount);
                            
                            loweredland[iii][jj]=1;
                        }
                    }
                }
            }
        }
    }
}

// This function ensures that all values in the map are in the correct range.

void clamp(planet &world)
{
    int width=world.width();
    int height=world.height();
    int maxelev=world.maxelevation();
    int sealevel=world.sealevel();
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0;j<=height; j++)
        {
            if (world.nom(i,j)>maxelev)
                world.setnom(i,j,maxelev);
            
            if (world.nom(i,j)<1)
                world.setnom(i,j,1);
            
            if (world.mountainheight(i,j)>0)
            {
                int map=world.map(i,j);
                
                if (map>maxelev)
                {
                    int newheight=maxelev-map-world.extraelev(i,j);
                    
                    world.setmountainheight(i,j,newheight);
                }
            }
        }
    }
}

// This function makes coasts shallower.

void shallowcoasts(planet &world)
{
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    
    int depth;
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (world.coast(i,j))
            {
                depth=sealevel-world.nom(i,j);
                world.setnom(i,j,sealevel-depth/2);
                world.setnom(i,j,sealevel-1);
            }
            
            //if (world.sea(i,j))
            //world.setnom(i,j,sealevel-1);
            
        }
    }
}

// This function makes coasts deeper!

void deepcoasts(planet &world)
{
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    
    int depth;
    int elev;
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (world.coast(i,j))
            {
                depth=sealevel-world.nom(i,j);
                world.setnom(i,j,sealevel-depth*2);
            }
        }
    }
}

// This function forces land and sea at coastlines to normalise towards certain values, to improve the appearance of the coastline. Note that landheight and seadepth are relative to the sea level, not absolute.

void normalisecoasts(planet &world, int landheight, int seadepth, int severity)
{
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    
    int depth, elev;
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (world.volcano(i,j)==0 && world.mountainisland(i,j)==0)
            {
                if (world.coast(i,j))
                {
                    depth=sealevel-world.nom(i,j);
                    int newdepth=(depth+seadepth*severity)/(severity+1);
                    
                    world.setnom(i,j,sealevel-newdepth);
                }
                
                if (world.outline(i,j))
                {
                    elev=world.nom(i,j)-sealevel;
                    int newelev=(elev+landheight*severity)/(severity+1);
                    
                    world.setnom(i,j,sealevel+newelev);
                }
            }
        }
    }
    
    // Now we need to smooth the seabeds around them.
    
    vector<vector<bool>> done(ARRAYWIDTH,vector<bool>(ARRAYWIDTH,ARRAYHEIGHT));

    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
            done[i][j]=0;
    }
    
    int amount=2;
    int amount2=2;
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (world.coast(i,j)==1)
            {
                for (int k=i-amount; k<=i+amount; k++)
                {
                    int kk=k;
                    
                    if (kk<0 || kk>width)
                        kk=wrap(kk,width);
                    
                    for (int l=j-amount; l<=j+amount; l++)
                    {
                        if (l>=0 && l<=height)
                        {
                            if (done[kk][l]==0 && world.sea(kk,l)==1 && world.coast(kk,l)==0)
                            {
                                int total=0;
                                int crount=0;
                                
                                for (int m=kk-amount2; m<=kk+amount2; m++)
                                {
                                    int mm=m;
                                    
                                    if (mm<0 || mm>width)
                                        mm=wrap(mm,width);
                                    
                                    for (int n=l-amount2; n<=l+amount2; n++)
                                    {
                                        if (n>=0 && n<=height && world.sea(mm,n)==1)
                                        {
                                            crount++;
                                            total=total+world.nom(mm,n);
                                        }
                                    }
                                }
                                
                                if (total!=0)
                                {
                                    int newnom=total/crount;
                                    world.setnom(kk,l,newnom);
                                    done[kk][l]=1;
                                    
                                    world.setnoshade(kk,l,1);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Do it again, going the other way.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
            done[i][j]=0;
    }
    
    for (int i=width; i>=0; i--)
    {
        for (int j=height; j>=0; j--)
        {
            if (world.coast(i,j)==1)
            {
                for (int k=i-amount; k<=i+amount; k++)
                {
                    int kk=k;
                    
                    if (kk<0 || kk>width)
                        kk=wrap(kk,width);
                    
                    for (int l=j-amount; l<=j+amount; l++)
                    {
                        if (l>=0 && l<=height)
                        {
                            if (done[kk][l]==0 && world.sea(kk,l)==1 && world.coast(kk,l)==0)
                            {
                                int total=0;
                                int crount=0;
                                
                                for (int m=kk-amount2; m<=kk+amount2; m++)
                                {
                                    int mm=m;
                                    
                                    if (mm<0 || mm>width)
                                        mm=wrap(mm,width);
                                    
                                    for (int n=l-amount2; n<=l+amount2; n++)
                                    {
                                        if (n>=0 && n<=height && world.sea(mm,n)==1)
                                        {
                                            crount++;
                                            total=total+world.nom(mm,n);
                                        }
                                    }
                                }
                                
                                if (total!=0)
                                {
                                    int newnom=total/crount;
                                    world.setnom(kk,l,newnom);
                                    done[kk][l]=1;
                                    world.setnoshade(kk,l,1);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// This notes down all the very small islands on the map.

void checkislands(planet &world)
{
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    
    for (int i=0; i<=width; i++)
    {
        for (int j=1; j<height; j++)
        {
            if (world.sea(i,j)==0)
            {
                int crount=-1; // Because there will definitely be one positive, i.e. the current cell.
                
                for (int k=i-1; k<=i+1; k++)
                {
                    int kk=k;
                    
                    if (kk<0 || kk>width)
                        kk=wrap(kk, width);
                    
                    for (int l=j-1; l<=j+1; l++)
                    {
                        if (world.sea(kk,l)==0)
                            crount++;
                    }
                }
                
                if (crount==0) // This is a one-tile island!
                {
                    world.setisland(i,j,1);

                    for (int k=i-1; k<=i+1; k++)
                    {
                        int kk=k;
                        
                        if (kk<0 || kk>width)
                            kk=wrap(kk, width);
                        
                        for (int l=j-1; l<=j+1; l++)
                        {
                            if (world.sea(kk,l)==1)
                                world.setnoshade(kk,l,1);
                        }
                    }
                }
            }
            else
            {
                int volcano=world.volcano(i,j);
                
                if (volcano!=0)
                {
                    bool extinct=0;
                    
                    if (volcano<0)
                    {
                        extinct=1;
                        volcano=0-volcano;
                    }
                    
                    int totalheight=world.nom(i,j)+volcano;
                    
                    if (totalheight>sealevel)
                    {
                        volcano=totalheight-sealevel;
                        
                        makemountainisland(world,i,j,volcano);
                        
                        if (extinct==1)
                            volcano=0-volcano;
                        
                        world.setvolcano(i,j,volcano);
                        
                        /*
                        world.setvolcano(i,j,volcano);
                        world.setnom(i,j,sealevel+random(5,20));
                        
                        if (random(1,3)==1) // Maybe make a bigger island here.
                        {
                            for (int k=i-1; k<=i+1; k++)
                            {
                                int kk=k;
                                
                                if (kk<0 || kk>width)
                                    kk=wrap(kk,width);
                                
                                for (int l=j-1; l<=j+1; l++)
                                {
                                    if (l>=0 && l<=height)
                                    {
                                        if (random(1,2)==1)
                                        {
                                            world.setnom(kk,l,sealevel+random(5,20));
                                            
                                            if (world.volcano(kk,l)==0 && random(1,2)==1) // Maybe put an extinct volcano here!
                                            {
                                                int newvolcano=world.volcano(i,j)+randomsign(random(1,100));
                                                
                                                if (newvolcano>0)
                                                    newvolcano=0-newvolcano;
                                                
                                                world.setvolcano(kk,l,newvolcano);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            world.setisland(i,j,1);
                            world.setmountainisland(i,j,1);
                        }
                        */
                    }
                }
            }
        }
    }
}

// This extends the noshade areas by one cell in each direction.

void extendnoshade(planet &world)
{
    int width=world.width();
    int height=world.height();
    
    vector<vector<bool>> extra(ARRAYWIDTH,vector<bool>(ARRAYWIDTH,ARRAYHEIGHT)); // This is to mark all cells touching the no-shade ones as themselves no-shade.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=width; j++)
            extra[i][j]=0;
    }
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (world.noshade(i,j)==1)
            {
                for (int k=i-1; k<=i+1; k++)
                {
                    int kk=k;
                    
                    if (kk<0 || kk>width)
                        kk=wrap(kk,width);
                    
                    for (int l=j-1; l<=j+1; l++)
                    {
                        if (l>=0 && l<=height && world.sea(kk,l)==1)
                            extra[kk][l]=1;
                    }
                }
            }
        }
    }
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=width; j++)
        {
            if (extra[i][j]==1)
                world.setnoshade(i,j,1);
        }
    }
}

// This function adds mountain ridges over the sea next to coastal mountains in glaciated regions (to make fjords) and elsewhere (to make rocky peninsulas). It's actually called in globalclimate.cpp but it's here because it's terrain really.

void addfjordmountains(planet &world)
{
    int width=world.width();
    int height=world.height();
    int maxelev=world.maxelevation();
    int glacialtemp=world.glacialtemp();
    int glacialmountainheight=20; //200; // In glacial regions, mountains this height or more are guaranteed to spawn fjords.
    
    int minmountheight=0; // Ignore mountains lower than this.
    int noglacchance=8; //3; //15; // Chance of making sea mountain ridges in non-glaciated areas (to add occasional rocky peninsulas).
    
    vector<vector<int>> justadded(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
    vector<vector<int>> previouslyadded(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            justadded[i][j]=0;
            previouslyadded[i][j]=0;
        }
    }
    
    for (int n=1; n<=2; n++)
    {
        for (int i=0; i<=width; i++)
        {
            for (int j=0; j<=height; j++)
            {
                if (world.sea(i,j)) // && random(1,maxelev)<world.roughness(i,j))
                {
                    int avetemp=(world.mintemp(i,j)+world.maxtemp(i,j))/2;
                    
                    bool fjordreason1=0;
                    bool fjordreason2=0;
                    
                    if (avetemp<glacialtemp)
                        fjordreason1=1; // Because it's cold enough. But this will depend upon the mountains being high enough.
                    
                    if (random(1,noglacchance)==1 && random(1,maxelev)<world.roughness(i,j))
                        fjordreason2=1; // Because it's just one of those ones that appear throughout the map.
                    
                    if (fjordreason1==1 || fjordreason2==1)
                    {
                        int nexttoland=0;
                        
                        int roughquotient=random(1,maxelev);
                        
                        for (int k=i-1; k<=i+1; k++)
                        {
                            int kk=k;
                            
                            if (kk<0 || kk>width)
                                kk=wrap(kk,width);
                            
                            for (int l=j-1; l<=j+1; l++)
                            {
                                if (l>=0 && l<=height)
                                {
                                    if (n==1 && world.sea(kk,l)==0 && world.mountainisland(kk,l)==0) // Don't allow these from mountain islands, as that causes weirdness.
                                    {
                                        nexttoland=1;
                                        k=i+1;
                                        l=j+1;
                                    }
                                    
                                    if (n!=1 && l>=0 && l<=height)
                                    {
                                        if (previouslyadded[kk][l]==1)
                                        {
                                            int roughness=world.roughness(i,j);
                                            
                                            if (avetemp<glacialtemp)
                                                roughness=roughness*5; // This is more likely in glacial areas.
                                            
                                            if (roughquotient<roughness) // The rougher the area, the more likely this is
                                            {
                                                nexttoland=1;
                                                k=i+1;
                                                l=j+1;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        
                        if (nexttoland==1)
                        {
                            int highest=0;
                            
                            int landx=-1;
                            int landy=-1;
                            
                            for (int k=i-1; k<=i+1; k++) // Find the highest mountain nearby, if there is one
                            {
                                int kk=k;
                                
                                if (kk<0 || kk>width)
                                    kk=wrap(kk,width);
                                
                                for (int l=j-1; l<=j+1; l++)
                                {
                                    if (l>=0 && l<=height)
                                    {
                                        if (world.mountainheight(kk,l)>highest && justadded[kk][l]==0)
                                        {
                                            highest=world.mountainheight(kk,l);
                                            
                                            landx=kk;
                                            landy=l;
                                        }
                                    }
                                }
                            }
                            
                            if (fjordreason1==1 && fjordreason2==0 && highest<glacialmountainheight) // If these are glacial mountain fjords, the mountains have to be high enough.
                                landx=-1;
                            
                            if (landx!=-1 && highest>=minmountheight) // If we found a nearby mountain, add a ridge to it.
                            {
                                int dir1=getdir(i,j,landx,landy);
                                int dir2=getdir(landx,landy,i,j);
                                
                                int code1=getcode(dir1);
                                int code2=getcode(dir2);
                                
                                world.setmountainridge(i,j,world.mountainridge(i,j)+code1);
                                world.setmountainridge(landx,landy,world.mountainridge(landx,landy)+code2);
                                
                                world.setmountainheight(i,j,world.mountainheight(landx,landy));
                                
                                world.setsummerrain(i,j,world.summerrain(landx,landy));
                                world.setwinterrain(i,j,world.winterrain(landx,landy));
                                
                                justadded[i][j]=1;
                                
                                //if (world.mountainheight(i,j)!=0)
                                    //world.settest(i,j,world.mountainheight(i,j));
                            }
                        }
                    }
                }
            }
        }
        
        for (int i=0; i<=width; i++)
        {
            for (int j=0; j<=height; j++)
            {
                if (previouslyadded[i][j]==0)
                    previouslyadded[i][j]=justadded[i][j];
                
                justadded[i][j]=0;
            }
        }
    }
}

// This function makes continental shelves.

void makecontinentalshelves(planet &world, vector<vector<bool>> &shelves, int pointdist)
{
    int width=world.width();
    int height=world.height();
    int maxelev=world.maxelevation();
    int midelev=maxelev/2;
    
    // First, sort out the outline.
    
    vector<vector<bool>> outline(ARRAYWIDTH,vector<bool>(ARRAYWIDTH,ARRAYHEIGHT));
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=width; j++)
            outline[i][j]=0;
    }
    
    int grain=8; // Level of detail on this fractal map.
    float valuemod=0.2; //0.4;
    int v=random(3,6);
    float valuemod2=v;
    
    vector<vector<int>> fractal(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
    
    createfractal(fractal,width,height,grain,valuemod,valuemod2,1,maxelev,0,0);
    
    int maxwarp=60;
    int warpdiv=maxelev/maxwarp;
    int maxradius=20;
    int raddiv=maxelev/maxradius;
    
    sf::Vector2f pt, mm1, mm2, mm3;
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=width; j++)
        {
            if (world.outline(i,j)==1)
            {
                // First, work out where the centre of this circle will be. It's getting offset to make the shelves more interesting.
                
                int warpdistx=0;
                int warpdisty=0;
                
                int warpx=i+width/4;
                
                if (warpx>width)
                    warpx=warpx-width;
                
                int warpy=j;
                
                if (fractal[warpx][warpy]>maxelev/2)
                    warpdistx=(fractal[warpx][warpy]-midelev)/warpdiv;
                else
                    warpdistx=0-(midelev-fractal[warpx][warpy])/warpdiv;
                
                int ii=i+warpdistx;
                
                if (ii<0 || ii>width)
                    wrap(ii,width);
                
                warpx=warpx+width/2;
                
                if (warpx>width)
                    warpx=warpx-width;
                
                if (fractal[warpx][warpy]>maxelev/2)
                    warpdisty=(fractal[warpx][warpy]-midelev)/warpdiv;
                else
                    warpdisty=0-(midelev-fractal[warpx][warpy])/warpdiv;
                
                int jj=j+warpdisty;

                // ii and jj are the centre of the circle we're drawing. But we want to draw a line from the actual outline to that centre, to ensure that there isn't a gap in the shelf.
                
                mm1.x=i;
                mm1.y=j;
                
                mm2.x=(i+ii)/2;
                mm2.y=(j+jj)/2;
                
                mm3.x=ii;
                mm3.y=jj;
                
                for (int n=1; n<=2; n++) // Two halves of the curve.
                {
                    for (float t=0.0; t<=1.0; t=t+0.01)
                    {
                        if (n==1)
                            pt=curvepos(mm1,mm1,mm2,mm3,t);
                        else
                            pt=curvepos(mm1,mm2,mm3,mm3,t);
                        
                        int x=pt.x;
                        int y=pt.y;
                        
                        if (x<0 || x>width)
                            x=wrap(x,width);
                        
                        if (y<0)
                            y=0;
                        
                        if (y>height)
                            y=height;
                        
                        outline[x][y]=1;
                    }
                }

                // Now we can draw a circle.
                
                if (ii<0 || ii>width)
                    wrap(ii,width);
                
                if (jj<0)
                    jj=0;
                
                if (jj>height)
                    jj=height;

                int radius=fractal[i][j]/raddiv;
                
                for (int x=-radius; x<=radius; x++)
                {
                    int xx=ii+x;
                    
                    if (xx<0 || xx>width)
                        xx=wrap(xx,width);
                    
                    for (int y=-radius; y<=radius; y++)
                    {
                        int yy=jj+y;
                        
                        if (yy>=0 && yy<=height)
                        {
                            if (x*x+y*y<radius*radius+radius)
                                outline[xx][yy]=1;
                        }
                    }
                }
            }
        }
    }

    // Now, we need a voronoi map.
    
    vector<vector<short>> voronoi(width+1,vector<short>(width+1,height+1));

    
    makeshelvesvoronoi(world,voronoi,outline,pointdist);
    
    // Now, every panel of the voronoi map that has any outline in it is continental shelf.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<height; j++)
        {
            if (outline[i][j]==1 && shelves[i][j]==0)
            {
                int thiscell=voronoi[i][j];
                
                for (int k=i-50; k<=i+50; k++)
                {
                    int kk=k;
                    
                    if (kk<0 || kk>width)
                        kk=wrap(kk,width);
                    
                    for (int l=j-50; l<=j+50; l++)
                    {
                        if (l>=0 && l<height)
                        {
                            if (voronoi[kk][l]==thiscell)
                                shelves[kk][l]=1;
                        }
                    }
                }
            }
        }
    }
    
    // And anywhere that's land is continental shelf too, technically.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<height; j++)
        {
            if (world.sea(i,j)==0)
                shelves[i][j]=1;
        }
    }
    
    // Now add a circles of shelf to the shelves, to make the edges a bit more varied.
    
    vector<vector<bool>> circles(ARRAYWIDTH,vector<bool>(ARRAYWIDTH,ARRAYHEIGHT));
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=width; j++)
            circles[i][j]=0;
    }
    
    int circlechance=40;
    int mindist=5; // Minimum distance to non-shelf sea.
    int radius=8;
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<height; j++)
        {
            if (shelves[i][j]==1 && random(1,circlechance)==1)
            {
                bool nearsea=0;
                
                for (int k=-mindist; k<=mindist; k++)
                {
                    for (int l=-mindist; l<=mindist; l++)
                    {
                        int ii=i+k;
                        int jj=j+l;
                        
                        if (ii<0 || ii>width)
                            ii=wrap(ii,width);
                        
                        if (jj>=0 && jj<=height)
                        {
                            if (shelves[ii][jj]==0)
                            {
                                nearsea=1;
                                k=mindist;
                                l=mindist;
                            }
                        }
                    }
                }
                
                if (nearsea==0)
                {

                    for (int x=-radius; x<=radius; x++)
                    {
                        int xx=i+x;
                        
                        if (xx<0 || xx>width)
                            xx=wrap(xx,width);
                        
                        for (int y=-radius; y<=radius; y++)
                        {
                            int yy=j+y;
                            
                            if (yy>=0 && yy<=height)
                            {
                                if (x*x+y*y<radius*radius+radius)
                                    circles[xx][yy]=1;
                            }
                        }
                    }
                }
            }
        }
    }
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<height; j++)
        {
            if (circles[i][j]==1)
                shelves[i][j]=1;
        }
    }
    
    // Now we need to remove any holes in the continental shelves!
    
    removeshelfgaps(world,shelves);
}

// This function removes any gaps in the continental shelves

void removeshelfgaps(planet &world, vector<vector<bool>> &shelves)
{
    int width=world.width();
    int height=world.height();

    int minseasize=(width*height)/4;
    
    vector<vector<bool>> checked(ARRAYWIDTH,vector<bool>(ARRAYWIDTH,ARRAYHEIGHT)); // This array marks which sea areas have already been scanned.
    
    vector<vector<int>> area(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT)); // This array marks the area of each bit of sea.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            checked[i][j]=0;
            area[i][j]=0;
        }
    }
    
    int seax=-1;
    int seay=-1;
    
    // First, we find the area of each section of non-shelf and mark them on the area array.
    
    for (int i=width-1; i>0; i--)
    {
        for (int j=height-1; j>0; j--)
        {
            if (shelves[i][j]==0 && checked[i][j]==0 && area[i][j]==0)
            {
                int size=nonshelfareacheck(world,shelves,checked,i,j);
                
                for (int k=0; k<=width; k++) // Clear the checked array and mark all adjoining points with this area.
                {
                    for (int l=0; l<=height; l++)
                    {
                        if (checked[k][l]==1)
                        {
                            area[k][l]=size;
                            checked[k][l]=0;
                        }
                    }
                }
            }
        }
    }
    
    // Now we just find a point that's in the largest area of non-shelf.
    
    int largest=0;
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (area[i][j]>largest)
            {
                largest=area[i][j];
                seax=i;
                seay=j;
            }
        }
    }
    
    // Now we simply remove any sea tiles that aren't part of the largest area of sea.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (area[i][j]!=largest)
                shelves[i][j]=1;
        }
    }
}

// This tells how large a given area of non-shelf sea is.

int nonshelfareacheck (planet &world, vector<vector<bool>> &shelves, vector<vector<bool>> &checked, int startx, int starty)
{
    int width=world.width();
    int height=world.height();
    
    int total=0;
    
    int row[]={-1,0,0,1};
    int col[]={0,-1,1,0};
    
    sf::Vector2i node;
    
    node.x=startx;
    node.y=starty;
    
    queue<sf::Vector2i> q; // Create a queue
    q.push(node); // Put our starting node into the queue
    
    while (q.empty()!=1) // Keep going while there's anything left in the queue
    {
        node=q.front(); // Take the node at the front of the queue
        q.pop(); // And then pop it out of the queue
        
        if (node.x>=0 && node.x<=width && node.y>=0 && node.y<=height && shelves[node.x][node.y]==0)
        {
            total++;
            checked[node.x][node.y]=1;
            
            for (int k=0; k<4; k++) // Look at the four neighbouring nodes in turn
            {
                sf::Vector2i nextnode;
                
                nextnode.x=node.x+row[k];
                nextnode.y=node.y+col[k];
                
                if (nextnode.x>width)
                    nextnode.x=0;
                
                if (nextnode.x<0)
                    nextnode.x=width;
                
                if (nextnode.y>=0 && nextnode.y<height)
                {
                    if (shelves[nextnode.x][nextnode.y]==0 && checked[nextnode.x][nextnode.y]==0) // If this node is sea
                    {
                        checked[nextnode.x][nextnode.y]=1;
                        q.push(nextnode); // Put that node onto the queue
                    }
                }
            }
        }
    }
    return total;
}

// This creates mid-ocean ridges.

void createoceanridges(planet &world, vector<vector<bool>> &shelves)
{
    int width=world.width();
    int height=world.height();
    int maxelev=world.maxelevation();
    
    vector<vector<int>> nearestshelfdist(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
    vector<vector<int>> nearestshelfx(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
    vector<vector<int>> nearestshelfy(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
    vector<vector<bool>> edgepoints(ARRAYWIDTH,vector<bool>(ARRAYWIDTH,ARRAYHEIGHT));
    vector<vector<bool>> boundaries(ARRAYWIDTH,vector<bool>(ARRAYWIDTH,ARRAYHEIGHT));
    vector<vector<int>> grid(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
    vector<vector<int>> gridnumbers(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
    vector<vector<int>> ridges(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
    vector<vector<int>> ridgesmap(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
    vector<vector<int>> ridgedistances(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=width; j++)
        {
            nearestshelfdist[i][j]=0;
            nearestshelfx[i][j]=-1;
            nearestshelfy[i][j]=-1;
            edgepoints[i][j]=0;
            boundaries[i][j]=0;
            grid[i][j]=0;
            gridnumbers[i][j]=0;
            ridges[i][j]=0;
            ridgesmap[i][j]=0;
            ridgedistances[i][j]=0;
        }
    }
    
    // We need a grid of edge points - points where the coastal shelves meet ocean.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (shelves[i][j]==1 && shelfedge(world,shelves,i,j)==1)
                edgepoints[i][j]=1;
        }
    }

    // Now we need to go over all the ocean points and work out their closest edge points.
    
    for (int i=0; i<=width; i++) // Every cell that *is* an edge point is closest to itself.
    {
        for (int j=0; j<=height; j++)
        {
            if (edgepoints[i][j]==1)
            {
                nearestshelfdist[i][j]=1;
                nearestshelfx[i][j]=i;
                nearestshelfy[i][j]=j;
            }
        }
    }
    
    for (int i=0; i<=width; i++) // The northern and southern edges of the map count as shelf edges for this purpose.
    {
        if (nearestshelfdist[i][0]==0)
        {
            nearestshelfdist[i][0]=1;
            nearestshelfx[i][0]=-1;
            nearestshelfy[i][0]=-1;
        }
        
        if (nearestshelfdist[i][1]==0)
        {
            nearestshelfdist[i][1]=1;
            nearestshelfx[i][1]=-1;
            nearestshelfy[i][1]=-1;
        }
        
        if (nearestshelfdist[i][height]==0)
        {
            nearestshelfdist[i][height]=1;
            nearestshelfx[i][height]=-1;
            nearestshelfy[i][height]=-1;
        }
        
        if (nearestshelfdist[i][height-1]==0)
        {
            nearestshelfdist[i][height-1]=1;
            nearestshelfx[i][height-1]=-1;
            nearestshelfy[i][height-1]=-1;
        }
    }
    
    bool done=0;
    int sweep=2; // Because the first sweep was just setting up the edge points themselves
    
    while (done==0) // Now go over the map, over and over again, filling out the zones
    {
        bool found=0;
        
        if (random(1,2)==1) // This is to make sure that we scan across the map in different directions equally, to avoid weird artefacts
        {
            int a=0;
            int b=width;
            int c=1;
            
            int d=0;
            int e=height;
            int f=1;
     
            if (random(1,2)==1)
            {
                a=width;
                b=0;
                c=-1;
            }
            
            if (random(1,2)==1)
            {
                d=height;
                e=0;
                f=-1;
            }

            for (int i=a; i!=b; i=i+c)
            {
                for (int j=d; j!=e; j=j+f)
                {
                    if (nearestshelfdist[i][j]==sweep-1) // If this is one we did on the *last* sweep
                    {
                        for (int k=i-1; k<=i+1; k++)
                        {
                            int kk=k;
                            
                            if (kk<0 || kk>width)
                                kk=wrap(kk,width);
                            
                            for (int l=j-1; l<=j+1; l++)
                            {
                                if (l>=0 && l<=height)
                                {
                                    if (k==i || l==j || random(1,2)==1) // Only sometimes do diagonals, otherwise the end result looks too angular.
                                    {
                                        if (shelves[kk][l]==0 && nearestshelfdist[kk][l]==0)
                                        {
                                            found=1;
                                            nearestshelfdist[kk][l]=sweep;
                                            nearestshelfx[kk][l]=nearestshelfx[i][j];
                                            nearestshelfy[kk][l]=nearestshelfy[i][j];
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else
        {
            int a=0;
            int b=width;
            int c=1;
            
            int d=0;
            int e=height;
            int f=1;
            
            if (random(1,2)==1)
            {
                a=width;
                b=0;
                c=-1;
            }
            
            if (random(1,2)==1)
            {
                d=height;
                e=0;
                f=-1;
            }
            
            for (int j=d; j!=e; j=j+f)
            {
                for (int i=a; i!=b; i=i+c)
                {
                    if (nearestshelfdist[i][j]==sweep-1) // If this is one we did on the *last* sweep
                    {
                        for (int k=i-1; k<=i+1; k++)
                        {
                            int kk=k;
                            
                            if (kk<0 || kk>width)
                                kk=wrap(kk,width);
                            
                            for (int l=j-1; l<=j+1; l++)
                            {
                                if (l>=0 && l<=height)
                                {
                                    if (k==i || l==j || random(1,2)==1) // Only sometimes do diagonals, otherwise the end result looks too angular.
                                    {
                                        if (shelves[kk][l]==0 && nearestshelfdist[kk][l]==0)
                                        {
                                            found=1;
                                            nearestshelfdist[kk][l]=sweep;
                                            nearestshelfx[kk][l]=nearestshelfx[i][j];
                                            nearestshelfy[kk][l]=nearestshelfy[i][j];
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        
        if (found==0)
            done=1;
        
        sweep++;
    }

    // Now we need to find where the zones meet.
    
    int maxdiff=400; //100; // If neighbouring cells have closest edgepoints that are further away than this, it means they are on the boundary between different zones.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (nearestshelfdist[i][j]>0)
            {
                bool found=0;
                
                for (int k=i-1; k<=i+1; k++)
                {
                    int kk=k;
                    
                    if (kk<0 || kk>width)
                        kk=wrap(kk,width);
                    
                    for (int l=j-1; l<=j+1; l++)
                    {
                        if (l>=0 && l<=height)
                        {
                            if (shelves[i][j]==0 && shelves[kk][l]==0)
                            {
                                if (boundaries[kk][l]==0 && (nearestshelfx[i][j]-nearestshelfx[kk][l]>maxdiff || nearestshelfx[kk][l]-nearestshelfx[i][j]>maxdiff || nearestshelfy[i][j]-nearestshelfy[kk][l]>maxdiff || nearestshelfy[kk][l]-nearestshelfy[i][j]>maxdiff))
                                {
                                    boundaries[i][j]=1;
                                    found=1;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    // We've got the boundaries marked out. Now we need to make a grid.
    
    int gridsize=16; // The bigger this is, the fewer points we'll identify.
    int halfgrid=gridsize/2-1;
    
    for (int i=0; i<=width; i=i+gridsize)
    {
        for (int j=0; j<=height; j=j+gridsize)
        {
            for (int k=i-halfgrid; k<=i+halfgrid; k++)
            {
                int kk=k;
                
                if (kk<0 || kk>width)
                    kk=wrap(kk,width);
                
                for (int l=j-halfgrid; l<=j+halfgrid; l++)
                {
                    if (l>=0 && l<=height)
                    {
                        if (boundaries[kk][l]==1)
                        {
                            grid[i][j]=nearestshelfdist[kk][l];
                            k=i+halfgrid;
                            l=j+halfgrid;
                        }
                    }
                }
            }
        }
    }
    
    for (int i=0; i<=width; i=i+gridsize) // This one is to get rid of extra points around the diagonals.
    {
        int righti=i+gridsize;
        
        if (righti>width)
            righti=righti-width;
        
        for (int j=0; j<=height; j=j+gridsize)
        {
            if (grid[i][j]!=0)
            {
                int upj=j-gridsize;
                int downj=j+gridsize;
                
                if (downj<=height)
                {
                    if (grid[righti][j]!=0 && grid[i][downj]!=0)
                        grid[i][j]=0;
                }
                
                if (upj>=0)
                {
                    if (grid[righti][j]!=0 && grid[i][upj]!=0)
                        grid[i][j]=0;
                }
            }
        }
    }
    
    // Now we have a grid of points. We need to assign each one a unique number.
    
    int gridno=1;
    
    for (int i=0; i<=width; i=i+gridsize)
    {
        for (int j=0; j<=height; j=j+gridsize)
        {
            if (grid[i][j]!=0)
            {
                gridnumbers[i][j]=gridno;
                gridno++;
            }
        }
    }
    
    int totalpoints=gridno;
    
    // Now we store which points neighbour each one.
    
    vector<int> tonorth(totalpoints+1);
    vector<int> tonortheast(totalpoints+1);
    vector<int> toeast(totalpoints+1);
    vector<int> tosoutheast(totalpoints+1);
    vector<int> distance(totalpoints+1);
    vector<int> pointx(totalpoints+1);
    vector<int> pointy(totalpoints+1);
    
    for (int n=0; n<=totalpoints; n++)
    {
        tonorth[n]=0;
        tonortheast[n]=0;
        toeast[n]=0;
        tosoutheast[n]=0;
        distance[n]=0;
    }
    
    int checkdist=1;
    
    for (int i=0; i<=width; i=i+gridsize)
    {
        int righti=i+gridsize;
        
        if (righti>width)
            righti=righti-width;
        
        for (int j=0; j<=height; j=j+gridsize)
        {
            if (grid[i][j]!=0)
            {
                int thispoint=gridnumbers[i][j];
                
                distance[thispoint]=nearestshelfdist[i][j];
                pointx[thispoint]=i;
                pointy[thispoint]=j;
                
                int upj=j-gridsize;
                int downj=j+gridsize;
                
                if (upj>=0)
                {
                    for (int k=i-halfgrid; k<=i+halfgrid; k++)
                    {
                        int kk=k;
                        
                        if (kk<-0 || kk>=width)
                            kk=wrap(kk,width);
                        
                        if (grid[kk][upj]!=0)
                        {
                            tonorth[thispoint]=gridnumbers[kk][upj];
                            k=i+halfgrid;
                        }
                    }
                    
                    for (int k=righti-halfgrid; k<=righti+halfgrid; k++)
                    {
                        int kk=k;
                        
                        if (kk<-0 || kk>=width)
                            kk=wrap(kk,width);
                        
                        if (grid[kk][upj]!=0)
                        {
                            tonortheast[thispoint]=gridnumbers[kk][upj];
                            k=righti+halfgrid;
                        }
                    }
                }
                
                for (int k=righti-halfgrid; k<=righti+halfgrid; k++)
                {
                    int kk=k;
                    
                    if (kk<-0 || kk>=width)
                        kk=wrap(kk,width);
                    
                    if (grid[kk][j]!=0)
                    {
                        toeast[thispoint]=gridnumbers[kk][j];
                        k=righti+halfgrid;
                    }
                }

                
                if (downj<=height)
                {
                    for (int k=righti-halfgrid; k<=righti+halfgrid; k++)
                    {
                        int kk=k;
                        
                        if (kk<-0 || kk>=width)
                            kk=wrap(kk,width);
                        
                        if (grid[kk][downj]!=0)
                        {
                            tosoutheast[thispoint]=gridnumbers[kk][downj];
                            k=righti+halfgrid;
                        }
                    }
                }
            }
        }
    }

    // Now for each point, we know its location, the distance to the closest shelf, and also which points (if any) border it to the N, NE, E, and SE. Now we just offset it a bit.
    
    // First, we want a fractal to offset all points.
    
    int grain=8; // Level of detail on this fractal map.
    float valuemod=0.2;
    int v=random(3,6);
    float valuemod2=v;
    
    vector<vector<int>> fractal(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
    
    createfractal(fractal,width,height,grain,valuemod,valuemod2,1,maxelev,0,0);
    
    int maxshift=40; //8; // Amount that points can move as a result of the fractal.
    
    int maxadditionalshift=6; //4; // Amount that points can be moved in addition.
    int minadditionalshift=1;
    
    float div=maxelev/(maxshift*2);
    
    for (int thispoint=1; thispoint<=totalpoints; thispoint++)
    {
        float xshift=fractal[pointx[thispoint]][pointy[thispoint]];
        
        xshift=xshift/div;
        xshift=xshift-maxshift;
        
        int yy=pointx[thispoint]+width;
        
        if (yy>width)
            yy=yy-width;
        
        float yshift=fractal[yy][pointy[thispoint]];
        
        yshift=yshift/div;
        yshift=yshift-maxshift;
        
        if (xshift<-maxshift)
            xshift=-maxshift;
        
        if (xshift>maxshift)
            xshift=maxshift;
        
        if (yshift<-maxshift)
            yshift=-maxshift;
        
        if (yshift>maxshift)
            yshift=maxshift;
        
        int xextrashift=randomsign(random(minadditionalshift,maxadditionalshift));
        int yextrashift=randomsign(random(minadditionalshift,maxadditionalshift));
        
        int newx=pointx[thispoint]+xshift+xextrashift;
        
        if (newx<0 || newx>width)
            newx=wrap(newx,width);
        
        int newy=pointy[thispoint]+yshift+yextrashift;
        
        if (newy<0)
            newy=0;
        
        if (newy>height)
            newy=height;
        
        pointx[thispoint]=newx;
        pointy[thispoint]=newy;
    }
    
    // Now join up the points.
    
    for (int thispoint=1; thispoint<=totalpoints; thispoint++)
    {
        if (tonorth[thispoint]!=0)
            drawoceanridgeline(world,pointx[thispoint],pointy[thispoint],pointx[tonorth[thispoint]],pointy[tonorth[thispoint]],ridges,distance[thispoint]);
        
        if (tonortheast[thispoint]!=0)
            drawoceanridgeline(world,pointx[thispoint],pointy[thispoint],pointx[tonortheast[thispoint]],pointy[tonortheast[thispoint]],ridges,distance[thispoint]);
        
        if (toeast[thispoint]!=0)
            drawoceanridgeline(world,pointx[thispoint],pointy[thispoint],pointx[toeast[thispoint]],pointy[toeast[thispoint]],ridges,distance[thispoint]);
        
        if (tosoutheast[thispoint]!=0)
            drawoceanridgeline(world,pointx[thispoint],pointy[thispoint],pointx[tosoutheast[thispoint]],pointy[tosoutheast[thispoint]],ridges,distance[thispoint]);

    }
    
    // Now get rid of excess points around the diagonals.
 
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (ridges[i][j]!=0)
            {
                int ii=i+1;
                
                if (ii>width)
                    ii=0;
                
                if (j<height)
                {
                    if (ridges[ii][j+1]!=0)
                    {
                        ridges[i][j+1]=0;
                        ridges[ii][j]=0;
                    }
                }
                
                if (j>0)
                {
                    if (ridges[ii][j-1]!=0)
                    {
                        ridges[i][j-1]=0;
                        ridges[ii][j]=0;
                    }
                }
            }
        }
    }
    
    // Now make sure each ridge point is adjacent to no more than one other ridge point.
    
    bool found=0;
    
    do
    {
        found=0;
        
        for (int i=0; i<=width; i++)
        {
            for (int j=0; j<=height; j++)
            {
                if (ridges[i][j]!=0)
                {
                    int neighbours=-1; // Because we'll add one for itself
                    
                    for (int k=i-1; k<=i+1; k++)
                    {
                        int kk=k;
                        
                        if (kk<0 || kk>width)
                            kk=wrap(kk,width);
                        
                        for (int l=j-1; l<=j+1; l++)
                        {
                            if (l>=0 && l<=height)
                            {
                                if (ridges[kk][l]!=0)
                                    neighbours++;
                                
                            }
                        }
                    }
                    
                    if (neighbours>2)
                    {
                        found=1;
                        
                        for (int k=i-1; k<=i+1; k++)
                        {
                            int kk=k;
                            
                            if (kk<0 || kk>width)
                                kk=wrap(kk,width);
                            
                            for (int l=j-1; l<=j+1; l++)
                            {
                                if (l>=0 && l<=height)
                                {
                                    if (ridges[kk][l]!=0)
                                    {
                                        ridges[kk][l]=0;
                                        k=i+1;
                                        l=j+1;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

    } while (found==1);
    
    // Now raise the land around the ridges.
    
    int maxradius=70;//50; // The bigger this is, the wider the ridge areas will be.
    int heightmult=6; // The bigger this is, the higher the ridge areas will be.
    int maxvolcanoradius=6; // Volcanos can't be further than this from the central ridge.

    for (int i=0; i<=width; i++) // First, the ridges.
    {
        for (int j=0; j<=height; j++)
        {
            if (ridges[i][j]!=0)
            {
                float riftheight=ridges[i][j]*heightmult;
                float riftheightdiv=riftheight/1000.0;
                
                int mult=0;
                
                for (int radius=maxradius; radius>=1; radius--)
                {
                    mult++;
                    
                    float thisheightperthousand=pow(1.1037,mult);
                    
                    float thisheight=riftheightdiv*thisheightperthousand;
                    
                    for (int k=-radius; k<=radius; k++)
                    {
                        for (int l=-radius; l<=radius; l++)
                        {
                            if (k*k+l*l<radius*radius+radius)
                            {
                                int kk=i+k;
                                
                                if (kk<0 || kk>width)
                                    kk=wrap(kk,width);
                                
                                int ll=j+l;
                                
                                if (ll>=0 && ll<=height)
                                {
                                    if (ridgesmap[kk][ll]<thisheight)
                                    {
                                        ridgesmap[kk][ll]=thisheight;
                                        ridgedistances[kk][ll]=mult;
                                        world.setoceanridgeheights(kk,ll,thisheight);
                                        
                                        if (radius<=maxvolcanoradius && random(1,4000000)<thisheight/10)
                                            world.setvolcano(kk,ll,random(thisheight/4,(thisheight/4)*3));
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    maxradius=50; // The bigger this is, the wider the ridge areas will be.
    heightmult=6; // The bigger this is, the higher the ridge areas will be.

    for (int i=0; i<=width; i++) // Now add some more for just the land.
    {
        for (int j=0; j<=height; j++)
        {
            if (ridges[i][j]!=0)
            {
                int heightdivs=((ridges[i][j]*heightmult)/maxradius);
                int mult=0;
                
                for (int radius=maxradius; radius>=1; radius--)
                {
                    mult++;
                    
                    int thisheight=heightdivs*mult;
                    
                    thisheight=(thisheight*3+(ridges[i][j]*heightmult)/radius)/4; // 3, 4
                    
                    for (int k=-radius; k<=radius; k++)
                    {
                        for (int l=-radius; l<=radius; l++)
                        {
                            if (k*k+l*l<radius*radius+radius)
                            {
                                int kk=i+k;
                                
                                if (kk<0 || kk>width)
                                    kk=wrap(kk,width);
                                
                                int ll=j+l;
                                
                                if (ll>=0 && ll<=height)
                                {
                                    if (ridgesmap[kk][ll]<thisheight)
                                        ridgesmap[kk][ll]=thisheight;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Add the rift in the middle
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (ridges[i][j]!=0)
            {
                ridgesmap[i][j]=ridgesmap[i][j]/2;
                world.setoceanrifts(i,j,ridgesmap[i][j]);
            }
        }
    }
    
    // Now we have to work out the angles of lines crossing the rifts.
    
    // First, make another fractal.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
            fractal[i][j]=0;
    }
    
    grain=4; // Level of detail on this fractal map.
    valuemod=0.01;
    v=1; //random(3,6);
    valuemod2=0.1;
    
    createfractal(fractal,width,height,grain,valuemod,valuemod2,1,360,0,1); // The values in this fractal wrap!
    
    // Now we need to go through every rift cell, and work out the angle of the line that would cross it at right angles.
    
    vector<vector<bool>> checked(ARRAYWIDTH,vector<bool>(ARRAYWIDTH,ARRAYHEIGHT));
    
    short dist=3; // Go this far away from the point in question in each direction.
    
    int x1, y1, x2, y2;
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (world.oceanrifts(i,j)!=0)
            {
                // Clear this area in the checked array.
                
                for (int k=i-dist-1; k<=i+dist+1; k++)
                {
                    int kk=k;
                    
                    if (kk<0 || kk>width)
                        kk=wrap(kk,width);
                    
                    for (int l=j-dist-1; l<=j+dist+1; l++)
                    {
                        if (l>=0 && l<=height)
                            checked[kk][l]=0;
                    }
                }
                
                // Now go along the line twice - once in each direction.

                for (int dir=1; dir<=2; dir++)
                {
                    int x=i;
                    int y=j;
                    
                    for (int n=1; n<=dist; n++)
                    {
                        checked[x][y]=1;
                        
                        for (int k=x-1; k<=x+1; k++)
                        {
                            int kk=k;
                            if (kk<0 || kk>width)
                                kk=wrap(kk,width);
                            
                            for (int l=y-1; l<=y+1; l++)
                            {
                                if (l>=0 && l<=height)
                                {
                                    if (checked[kk][l]==0 && world.oceanrifts(kk,l)!=0)
                                    {
                                        x=kk;
                                        y=l;
                                        
                                        k=x+20;
                                        l=y+20;
                                    }
                                }
                            }
                        }
                    }
                    
                    if (dir==1)
                    {
                        x1=x;
                        y1=y;
                    }
                    else
                    {
                        x2=x;
                        y2=y;
                    }
                }
                
                // Now we have the two nearby points, we just find the angle between them.
                
                float angle=atan2(y2-y1,x2-x1)*180/3.14159265358979323846; // This gives us the angle of the ridge
                
                angle=angle+180; // Because we want the line that crosses the ridge
                
                while (angle>360)
                    angle=angle-360;
                
                // Now we just have to blend that with the fractal angle for this point.
                
                int angle2=wrappedaverage(angle,fractal[i][j],360);
                
                while (angle2>180)
                    angle2=angle2-180;
                
                world.setoceanridgeangle(i,j,angle2);
            }
        }
    }

    // Mark out the ridge mountains, following the contours of the raised land
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (ridgedistances[i][j]!=0)
            {
                int ii=i; // Looking north
                int jj=j-1;
                int dir=1;
                
                if (jj>=0)
                {
                    if (ridgedistances[ii][jj]==ridgedistances[i][j])
                    {
                        if (getoceanridge(world,i,j,dir)==0)
                        {
                            int code=getcode(dir);
                            world.setoceanridges(i,j,world.oceanridges(i,j)+code);
                        }
                    }
                }
                
                ii=i+1; // Looking northeast
                jj=j-1;
                dir=2;
                
                if (ii>width)
                    ii=0;
                
                if (jj>=0)
                {
                    if (ridgedistances[ii][jj]==ridgedistances[i][j])
                    {
                        if (getoceanridge(world,i,j,dir)==0)
                        {
                            int code=getcode(dir);
                            world.setoceanridges(i,j,world.oceanridges(i,j)+code);
                        }
                    }
                }
                
                ii=i+1; // Looking east
                jj=j;
                dir=3;
                
                if (ii>width)
                    ii=0;

                if (ridgedistances[ii][jj]==ridgedistances[i][j])
                {
                    if (getoceanridge(world,i,j,dir)==0)
                    {
                        int code=getcode(dir);
                        world.setoceanridges(i,j,world.oceanridges(i,j)+code);
                    }
                }
                
                ii=i+1; // Looking southeast
                jj=j+1;
                dir=4;
                
                if (ii>width)
                    ii=0;
                
                if (jj<=height)
                {
                    if (ridgedistances[ii][jj]==ridgedistances[i][j])
                    {
                        if (getoceanridge(world,i,j,dir)==0)
                        {
                            int code=getcode(dir);
                            world.setoceanridges(i,j,world.oceanridges(i,j)+code);
                        }
                    }
                }
                
                ii=i; // Looking south
                jj=j+1;
                dir=5;
                
                if (jj<=height)
                {
                    if (ridgedistances[ii][jj]==ridgedistances[i][j])
                    {
                        if (getoceanridge(world,i,j,dir)==0)
                        {
                            int code=getcode(dir);
                            world.setoceanridges(i,j,world.oceanridges(i,j)+code);
                        }
                    }
                }
                
                ii=i-1; // Looking southwest
                jj=j+1;
                dir=6;
                
                if (ii<0)
                    ii=width;
                
                if (jj<=height)
                {
                    if (ridgedistances[ii][jj]==ridgedistances[i][j])
                    {
                        if (getoceanridge(world,i,j,dir)==0)
                        {
                            int code=getcode(dir);
                            world.setoceanridges(i,j,world.oceanridges(i,j)+code);
                        }
                    }
                }
                
                ii=i-1; // Looking west
                jj=j;
                dir=7;
                
                if (ii<0)
                    ii=width;
                
                if (ridgedistances[ii][jj]==ridgedistances[i][j])
                {
                    if (getoceanridge(world,i,j,dir)==0)
                    {
                        int code=getcode(dir);
                        world.setoceanridges(i,j,world.oceanridges(i,j)+code);
                    }
                }
                
                ii=i-1; // Looking northwest
                jj=j-1;
                dir=8;
                
                if (ii<0)
                    ii=width;
                
                if (jj>=0)
                {
                    if (ridgedistances[ii][jj]==ridgedistances[i][j])
                    {
                        if (getoceanridge(world,i,j,dir)==0)
                        {
                            int code=getcode(dir);
                            world.setoceanridges(i,j,world.oceanridges(i,j)+code);
                        }
                    }
                }
            }
        }
    }

    // Now remove extraneous ridges
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (world.oceanridges(i,j)!=0)
            {
                short total=0;
                
                for (int dir=1; dir<=8; dir++)
                {
                    if (getoceanridge(world,i,j,dir)==1)
                        total++;
                }
                
                if (total>2)
                {
                    int extra=total-2;
                    
                    for (int n=1; n<=extra; n++)
                    {
                        int a=1;
                        int b=8;
                        int c=1;
                        
                        if (random(1,2)==1)
                        {
                            a=8;
                            b=1;
                            c=-1;
                        }
                        
                        for (int dir=a; dir!=b; dir=dir+c)
                        {
                            if (getoceanridge(world,i,j,dir)==1)
                                deleteoceanridge(world,i,j,dir);
                        }
                    }
                }
            }
        }
    }

    // Now we'll make a fractal. This will be used to displace the ridges in the regional map.
    
    int maxdisplace=16; // Maximum displacement of the ridges in the regional map.
    
    grain=128; // Level of detail on this fractal map.
    valuemod=4.0;;
    valuemod2=8;
    
    vector<vector<int>> ridgefractal(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
    
    createfractal(ridgefractal,width,height,grain,valuemod,valuemod2,1,maxelev,0,0);
    
    div=maxelev/(maxdisplace*2);
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            int amount=ridgefractal[i][j];
            
            amount=amount/div;
            amount=amount-maxdisplace;
            
            if (amount<0-maxdisplace)
                amount=0-maxdisplace;
            
            if (amount>maxdisplace)
                amount=maxdisplace;
            
            world.setoceanridgeoffset(i,j,amount);
        }
    }
    
    

    // Now we need to disrupt the rifts by adding some faults.
    
    for (int n=1; n<=4; n++)
    {
        int faultstep=8; // The higher this is, the fewer faults there will be
        int faultvar=3; // The higher this is, the more random the spaces between faults
        int lookdist=4; // Distance to look for faults from the focal points
        
        int faultstotal=0;
        
        for (int i=0; i<ARRAYWIDTH; i=i+faultstep)
        {
            for (int j=0; j<ARRAYHEIGHT; j=j+faultstep)
            {
                int ii=i+randomsign(random(1,faultvar));
                int jj=j+randomsign(random(1,faultvar));
                
                if (ii<0 || ii>width)
                    ii=wrap(ii,width);
                
                if (jj<0)
                    jj=0;
                
                if (jj>height)
                    jj=height;
                
                if (world.sea(ii,jj)==1)
                {
                    for (int x=ii-lookdist; x<=ii+lookdist; x++)
                    {
                        int xx=x;
                        
                        if (xx<0 || xx>width)
                            xx=wrap(xx,width);
                        
                        for (int y=jj-lookdist; y<=jj+lookdist; y++)
                        {
                            if (y>=0 && y<=height)
                            {
                                if (world.oceanrifts(xx,y)!=0)
                                {
                                    int maxwidth=30;
                                    int minwidth=6;
                                    
                                    int masksize=200+maxwidth;
                                    
                                    createoceanfault(world,xx,y,minwidth,maxwidth,ridgesmap,checked,masksize);
                                    
                                    x=ii+lookdist;
                                    y=jj+lookdist;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    

    
    // Remove any ridge-related stuff that's on continental plates
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (shelves[i][j]==1 || world.sea(i,j)==0)
            {
                ridgesmap[i][j]=0;
                world.setoceanrifts(i,j,0);
                world.setoceanridges(i,j,0);
                world.setoceanridgeheights(i,j,0);
                world.setoceanridgeangle(i,j,0);
                world.setoceanridgeoffset(i,j,0);
            }
        }
    }

    // Draw the raised land onto the actual sea bed
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
            world.setnom(i,j,world.nom(i,j)+ridgesmap[i][j]);
    }
}

// This checks to see whether the given point is on the edge of a continental shelf.

bool shelfedge(planet &world, vector<vector<bool>> &shelves, int x, int y)
{
    if (shelves[x][y]==0)
        return 0;
    
    int width=world.width();
    int height=world.height();
    
    for (int i=x-1; i<=x+1; i++)
    {
        int ii=i;
        
        if (ii<0)
            ii=width;
        
        if (ii>width)
            ii=0;
        
        for (int j=y-1; j<=y+1; j++)
        {
            if (j>=0 && j<=height)
            {
                if (shelves[ii][j]==0)
                    return 1;
            }
        }
    }
    return 0;
}

// This draws a curvy line between two points on the ocean ridge map.

void drawoceanridgeline(planet &world, int fromx, int fromy, int tox, int toy, vector<vector<int>> &array, int value)
{
    int width=world.width();
    int height=world.height();
    
    if (tox<fromx-width/2)
        tox=tox+width;
    
    if (fromx<tox-width/2)
        fromx=fromx+width;
    
    int distance=abs(fromx-tox)+abs(fromy-toy);
    
    int minvar=2;
    int maxvar=distance/8;

    if (maxvar<3)
        maxvar=3;
    
    sf::Vector2f pt, mm1, mm2, mm3;
    
    mm1.x=fromx;
    mm1.y=fromy;
    
    mm2.x=(fromx+tox)/2+randomsign(random(minvar,maxvar));
    mm2.y=(fromy+toy)/2+randomsign(random(minvar,maxvar));
    
    mm3.x=tox;
    mm3.y=toy;
    
    for (int n=1; n<=2; n++) // Two halves of the curve.
    {
        for (float t=0.0; t<=1.0; t=t+0.01)
        {
            if (n==1)
                pt=curvepos(mm1,mm1,mm2,mm3,t);
            else
                pt=curvepos(mm1,mm2,mm3,mm3,t);
            
            int x=pt.x;
            int y=pt.y;
            
            if (x<0 || x>width)
                x=wrap(x,width);
            
            if (y<0)
                y=0;
            
            if (y>height)
                y=height;
            
            array[x][y]=value;
        }
    }
    
}

// This creates a fault in an oceanic rift.

void createoceanfault(planet &world, int midx, int midy, int mindist, int maxdist, vector<vector<int>> &ridgesmap, vector<vector<bool>> &checked, int masksize)
{
    int width=world.width();
    int height=world.height();
    int maxelev=world.maxelevation();
    
    int minshiftdist=2;
    int maxshiftdist=6;
    
    int dist=random(mindist,maxdist); // Distance from the midpoint that the edge points should be.
    
    int x1,y1,x2,y2; // Coordinates of the rift points at the ends of the area to be moved.
    
    // Clear this area in the checked array.
    
    for (int i=midx-20; i<=midx+20; i++)
    {
        int ii=i;
        
        if (ii<0 || ii>width)
            ii=wrap(ii,width);
        
        for (int j=midy-20; j<=midy+20; j++)
        {
            if (j>=0 && j<=height)
                checked[ii][j]=0;
        }
    }
    
    // Now go along the line twice - once in each direction.
    
    for (int dir=1; dir<=2; dir++)
    {
        int x=midx;
        int y=midy;
        
        for (int n=1; n<=dist; n++)
        {
            checked[x][y]=1;
            
            bool found=0;
            
            for (int k=x-1; k<=x+1; k++)
            {
                int kk=k;
                if (kk<0 || kk>width)
                    kk=wrap(kk,width);
                
                for (int l=y-1; l<=y+1; l++)
                {
                    if (l>=0 && l<=height)
                    {
                        if (checked[kk][l]==0 && world.oceanrifts(kk,l)!=0)
                        {
                            x=kk;
                            y=l;
                            
                            found=1;
                            k=x+20;
                            l=y+20;
                        }
                    }
                }
            }
            
            if (found==0) // This means we reached the end of the rift line, so we can't do it.
                return;
        }
        
        if (dir==1)
        {
            x1=x;
            y1=y;
        }
        else
        {
            x2=x;
            y2=y;
        }
    }

    // Now we need to find the coordinates of the four corners of the area to be moved.
    
    int length=70; //100; // Distance to go out from the central rift.
    
    int cornerx1,cornery1,cornerx2,cornery2,cornerx3,cornery3,cornerx4,cornery4;
    
    for (int n=1; n<=2; n++) // Do it for each of the two end points
    {
        int x,y;
        
        if (n==1)
        {
            x=x1;
            y=y1;
        }
        else
        {
            x=x2;
            y=y2;
        }
        
        int angle=0-world.oceanridgeangle(x,y);
        
        while (angle<0)
            angle=angle+360;
        
        for (int nn=1; nn<=2; nn++) // Two halves - one on each side of the rift.
        {
            float fangle;
            
            if (nn==1)
                fangle=angle*0.01745329;
            else
            {
                fangle=angle+180;
                
                if (fangle>360)
                    fangle=fangle-360;
                
                fangle=fangle*0.01745329;
            }
            
            int endx=x+(length*sin(fangle));
            int endy=y+(length*cos(fangle));
            
            if (n==1)
            {
                if (nn==1)
                {
                    cornerx1=endx;
                    cornery1=endy;
                }
                else
                {
                    cornerx2=endx;
                    cornery2=endy;
                }
            }
            else
            {
                if (nn==1)
                {
                    cornerx4=endx;
                    cornery4=endy;
                }
                else
                {
                    cornerx3=endx;
                    cornery3=endy;
                }
            }
        }
    }
    
    if (cornery1<0 || cornery1>height || cornery2<0 || cornery2>height || cornery3<0 || cornery3>height || cornery4<0 || cornery4>height)
        return;
    
    // Now make sure that all of our points are equally wrapped/unwrapped.
    
    normalise(midx,cornerx1,width);
    normalise(midx,cornerx2,width);
    normalise(midx,cornerx3,width);
    normalise(midx,cornerx4,width);
    
    // We now have the four corners of the whole area to be shifted.
    
    // Check that they don't cross over each other:
    
    if (x1<x2)
    {
        if (cornerx1>cornerx4)
            return;
        
        if (cornerx2>cornerx3)
            return;
    }
    else
    {
        if (cornerx1<cornerx4)
            return;
        
        if (cornerx2<cornerx3)
            return;
    }
    
    if (y1<y2)
    {
        if (cornery1>cornery4)
            return;
        
        if (cornery2>cornery3)
            return;
    }
    else
    {
        if (cornery1<cornery4)
            return;
        
        if (cornery2<cornery3)
            return;
    }
    
    // Now we need to create a mask of this area.
    
    vector<vector<bool>> mask(masksize+1,vector<bool>(masksize+1,masksize+1));
    
    for (int i=0; i<=masksize; i++)
    {
        for (int j=0; j<=masksize; j++)
            mask[i][j]=0;
    }
    
    // To convert coordinates from the map to the mask, add the offsets.
    // To convert them from the mask to the map, subtract the offsets.
    
    int offsetx=masksize/2-midx;
    int offsety=masksize/2-midy;
    
    cornerx1=cornerx1+offsetx;
    cornerx2=cornerx2+offsetx;
    cornerx3=cornerx3+offsetx;
    cornerx4=cornerx4+offsetx;
    
    cornery1=cornery1+offsety;
    cornery2=cornery2+offsety;
    cornery3=cornery3+offsety;
    cornery4=cornery4+offsety;
    
    if (cornerx1<0 || cornerx1>width)
        cornerx1=wrap(cornerx1,width);
    
    if (cornerx2<0 || cornerx2>width)
        cornerx2=wrap(cornerx2,width);
    
    if (cornerx3<0 || cornerx3>width)
        cornerx3=wrap(cornerx3,width);
    
    if (cornerx4<0 || cornerx4>width)
        cornerx4=wrap(cornerx4,width);
    
    // Draw the outline of the mask.
    
    drawline(mask,cornerx1,cornery1,cornerx2,cornery2);
    drawline(mask,cornerx2,cornery2,cornerx3,cornery3);
    drawline(mask,cornerx3,cornery3,cornerx4,cornery4);
    drawline(mask,cornerx4,cornery4,cornerx1,cornery1);
    
    // Fill in the mask.
    
    fill(mask,masksize,masksize,masksize/2,masksize/2,1);
    
    if (mask[0][0]==1) // If somehow we've filled the outside of the shape rather than the inside - too weird to deal with!
        return;
    
    // Now we need to remove any links to ridges outside this area.
    
    for (int i=0; i<=masksize; i++)
    {
        int ii=i-offsetx;
        
        if (ii<0 || ii>width)
            ii=wrap(ii,width);
        
        for (int j=0; j<=masksize; j++)
        {
            int jj=j-offsety;
            
            if (jj>=0 && jj<=height)
            {
                if (mask[i][j]==1)
                {
                    // Looking north
                    
                    int iii=i;
                    int jjj=j-1;
                    
                    bool docheck=0;
                    
                    if (jjj<0)
                        docheck=1;
                    
                    if (mask[iii][jjj]==0)
                        docheck=1;
                    
                    if (docheck==1)
                        deleteoceanridge(world,ii,jj,1);
                    
                    // Looking northeast
                    
                    iii=i+1;
                    jjj=j-1;
                    
                    docheck=0;
                    
                    if (jjj<0 || iii>masksize)
                        docheck=1;
                    
                    if (mask[iii][jjj]==0)
                        docheck=1;
                    
                    if (docheck==1)
                        deleteoceanridge(world,ii,jj,2);
                    
                    // Looking east
                    
                    iii=i+1;
                    jjj=j;
                    
                    docheck=0;
                    
                    if (iii>masksize)
                        docheck=1;
                    
                    if (mask[iii][jjj]==0)
                        docheck=1;
                    
                    if (docheck==1)
                        deleteoceanridge(world,ii,jj,3);
                    
                    // Looking southeast
                    
                    iii=i+1;
                    jjj=j+1;
                    
                    docheck=0;
                    
                    if (iii>masksize || jjj>masksize)
                        docheck=1;
                    
                    if (mask[iii][jjj]==0)
                        docheck=1;
                    
                    if (docheck==1)
                        deleteoceanridge(world,ii,jj,4);
                    
                    // Looking south
                    
                    iii=i;
                    jjj=j+1;
                    
                    docheck=0;
                    
                    if (jjj>masksize)
                        docheck=1;
                    
                    if (mask[iii][jjj]==0)
                        docheck=1;
                    
                    if (docheck==1)
                        deleteoceanridge(world,ii,jj,5);
                    
                    // Looking southwest
                    
                    iii=i-1;
                    jjj=j+1;
                    
                    docheck=0;
                    
                    if (iii<0 || jjj>masksize)
                        docheck=1;
                    
                    if (mask[iii][jjj]==0)
                        docheck=1;
                    
                    if (docheck==1)
                        deleteoceanridge(world,ii,jj,6);
                    
                    // Looking west
                    
                    iii=i-1;
                    jjj=j;
                    
                    docheck=0;
                    
                    if (iii<0)
                        docheck=1;
                    
                    if (mask[iii][jjj]==0)
                        docheck=1;
                    
                    if (docheck==1)
                        deleteoceanridge(world,ii,jj,7);
                    
                    // Looking northwest
                    
                    iii=i-1;
                    jjj=j-1;
                    
                    docheck=0;
                    
                    if (iii<0 || jjj<0)
                        docheck=1;
                    
                    if (mask[iii][jjj]==0)
                        docheck=1;
                    
                    if (docheck==1)
                        deleteoceanridge(world,ii,jj,8);
                }
            }
        }
    }
    
    // Now we need to record all of the ridge information for this area
    
    vector<vector<int>> oldridgesmap(masksize+1,vector<int>(masksize+1,masksize+1));
    vector<vector<int>> oldoceanrifts(masksize+1,vector<int>(masksize+1,masksize+1));
    vector<vector<int>> oldoceanridges(masksize+1,vector<int>(masksize+1,masksize+1));
    vector<vector<int>> oldoceanridgeheights(masksize+1,vector<int>(masksize+1,masksize+1));
    vector<vector<int>> oldoceanridgeangle(masksize+1,vector<int>(masksize+1,masksize+1));
    vector<vector<int>> oldoceanridgeoffset(masksize+1,vector<int>(masksize+1,masksize+1));
    
    for (int i=0; i<=masksize; i++)
    {
        int ii=i-offsetx;
        
        if (ii<0 || ii>width)
            ii=wrap(ii,width);
        
        for (int j=0; j<=masksize; j++)
        {
            int jj=j-offsety;
            
            if (jj>=0 && jj<=height)
            {
                if (mask[i][j]==1)
                {
                    oldridgesmap[i][j]=ridgesmap[ii][jj];
                    oldoceanrifts[i][j]=world.oceanrifts(ii,jj);
                    oldoceanridges[i][j]=world.oceanridges(ii,jj);
                    oldoceanridgeheights[i][j]=world.oceanridgeheights(ii,jj);
                    oldoceanridgeangle[i][j]=world.oceanridgeangle(ii,jj);
                    oldoceanridgeoffset[i][j]=world.oceanridgeoffset(ii,jj);
                }
            }
        }
    }
    
    // Now we need to work out the direction and distance that all of this is going to be shifted.
    
    int angle=0-(world.oceanridgeangle(x1,y1)+world.oceanridgeangle(x2,y1))/2;
    
    //int angle=0-world.oceanridgeangle(x1,y1);
    
    while (angle<0)
        angle=angle+360;
    
    if (random(1,2)==1)
    {
        angle=angle+180;
        
        while (angle>360)
            angle=angle-360;
    }
    
    float fangle=angle*0.01745329;
    
    int shiftdist=random(minshiftdist,maxshiftdist);
    
    int xshift=(shiftdist*sin(fangle));
    int yshift=(shiftdist*cos(fangle));
    
    // Now we rewrite all that information, offset by the new shift values.

    for (int i=0; i<=masksize; i++)
    {
        int iii=i-offsetx+xshift;
        
        if (iii<0 || iii>width)
            iii=wrap(iii,width);
        
        for (int j=0; j<=masksize; j++)
        {
            int jjj=j-offsety+yshift;
            
            if (jjj>=0 && jjj<=height)
            {
                if (mask[i][j]==1)
                {
                    ridgesmap[iii][jjj]=oldridgesmap[i][j];
                    world.setoceanrifts(iii,jjj,oldoceanrifts[i][j]);
                    world.setoceanridges(iii,jjj,oldoceanridges[i][j]);
                    world.setoceanridgeheights(iii,jjj,oldoceanridgeheights[i][j]);
                    world.setoceanridgeangle(iii,jjj,oldoceanridgeangle[i][j]);
                    world.setoceanridgeoffset(iii,jjj,oldoceanridgeoffset[i][j]);
                }
            }
        }
    }
}

// This creates the ocean trenches.

void createoceantrenches(planet &world, vector<vector<bool>> &shelves)
{
    int width=world.width();
    int height=world.height();
    int maxelev=world.maxelevation();
    
    int trenchmin=maxelev/2; // Values in the fractal higher than this will spawn trenches.
    
    int maxradius=20;
    
    int div=(maxelev-trenchmin)/maxradius;
    
    vector<vector<bool>> trenchmap(ARRAYWIDTH,vector<bool>(ARRAYWIDTH,ARRAYHEIGHT));
    vector<vector<int>> fractal(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            trenchmap[i][j]=0;
            fractal[i][j]=0;
        }
    }

    int grain=4; // Level of detail on this fractal map.
    float valuemod=0.01;
    int v=1; //random(3,6);
    float valuemod2=0.1;
    
    createfractal(fractal,width,height,grain,valuemod,valuemod2,1,maxelev,0,0);
    
    for (int i=0; i<=width; i++)
    {
        for (int j=2; j<height-1; j++)
        {
            if (shelves[i][j]==1 && fractal[i][j]>trenchmin)
            {
                if (shelfedge(world,shelves,i,j)==1)
                {
                    int radius=(fractal[i][j]-trenchmin)/div;
                    
                    for (int k=-radius; k<=radius; k++)
                    {
                        for (int l=-radius; l<=radius; l++)
                        {
                            if (k*k+l*l<radius*radius+radius)
                            {
                                int ll=j+l;
                                
                                if (ll>=0 && ll<=height)
                                {
                                    int kk=i+k;
                                    
                                    if (kk<0 || kk>width)
                                        kk=wrap(kk,width);
                                    
                                    trenchmap[kk][ll]=1;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Now just add the trenches to the world map.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (trenchmap[i][j]==1)
            {
                if (world.sea(i,j)==1 && shelves[i][j]==0 && world.oceanridges(i,j)==0)
                {
                    int newval=world.nom(i,j)-random(4900,5100);
                    
                    if (newval<1)
                        newval=1;
                    
                    world.setnom(i,j,newval);
                }
            }
        }
    }
}

// This creates mountains from an imported raw mountain map.

void createmountainsfromraw(planet &world, vector<vector<int>> &rawmountains)
{
    int width=world.width();
    int height=world.height();
    int maxelev=world.maxelevation();
    
    vector<vector<int>> extraraw(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT)); // This will hold extra raw ridges that we add near the start.
    vector<vector<int>> mountaindist(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT)); // This holds the proximity to the central peaks (the higher the value, the closer it is).
    vector<vector<int>> mountainbaseheight(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT)); // This holds the height that this peak will be (to start with, it's just recorded as the same as the central peak that it's measured from).
    vector<vector<bool>> timesten(ARRAYWIDTH,vector<bool>(ARRAYWIDTH,ARRAYHEIGHT)); // This records whether the ridge distance has been multiplied by ten (this is done so that the ridges on either side of the main ridge don't join up with each other).
    vector<vector<int>> mountainridges(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
    vector<vector<int>> mountainheights(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT)); // These two are the same as the world mountain ridges/heights arrays, but we'll do everything on these first and then copy them over.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            extraraw[i][j]=0;
            mountaindist[i][j]=0;
            mountainbaseheight[i][j]=0;
            timesten[i][j]=0;
            mountainridges[i][j]=0;
            mountainheights[i][j]=0;
        }
    }
    
    int grain=256;
    float valuemod=8;
    int v=random(3,6);
    float valuemod2=v;
    
    vector<vector<int>> heightsfractal(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT)); // This will vary the heights of the peaks in the subsidiary ridges.
    createfractal(heightsfractal,width,height,grain,valuemod,valuemod2,1,maxelev,0,0);
    
    grain=16;
    valuemod=0.2;
    v=random(3,6);
    valuemod2=v;
    
    vector<vector<int>> radiusfractal(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT)); // This will vary the effective radius of each mountain point.
    createfractal(radiusfractal,width,height,grain,valuemod,valuemod2,1,maxelev,0,0);
    
    float maxradius=6; // 10; // The bigger this is, the wider the mountain ranges will be.
    
    // First, ensure that the raw mountains array has ridges that are only one cell wide.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=1; j<height; j++)
        {
            if (rawmountains[i][j]!=0)
            {
                int ii=i+1;
                if (ii>width)
                    ii=0;
                
                if (rawmountains[i][j-1]!=0 && rawmountains[i][j+1]!=0)
                    rawmountains[ii][j]=0;
                
                ii=i-1;
                if (ii<0)
                    ii=width;
                
                if (rawmountains[i][j-1]!=0 && rawmountains[i][j+1]!=0)
                    rawmountains[ii][j]=0;
            }
        }
    }
    
    for (int i=0; i<=width; i++)
    {
        for (int j=1; j<height; j++)
        {
            if (rawmountains[i][j]!=0)
            {
                int jj=j+1;
                
                int ileft=i-1;
                if (ileft<0)
                    ileft=width;
                
                int iright=i+1;
                if (iright>width)
                    iright=0;
                
                if (rawmountains[iright][j]!=0 && rawmountains[ileft][j]!=0)
                    rawmountains[i][jj]=0;
                
                jj=j-1;

                if (rawmountains[iright][j]!=0 && rawmountains[ileft][j]!=0)
                    rawmountains[i][jj]=0;
            }
        }
    }
    
    for (int i=0; i<=width; i++)
    {
        int ii=i+1;
        
        if (ii>width)
            ii=0;
        
        for (int j=1; j<height; j++)
        {
            if (rawmountains[i][j]!=0 && rawmountains[ii][j+1]!=0)
            {
                rawmountains[ii][j]=0;
                rawmountains[i][j+1]=0;
            }
            
            if (rawmountains[ii][j]!=0 && rawmountains[i][j+1]!=0)
            {
                rawmountains[i][j]=0;
                rawmountains[ii][j+1]=0;
            }
        }
    }
    
    // Now add some extra ridges! We do this on another array to start with so we don't add them onto each other.
    
    int extrachance=10; // The higher this is, the fewer extra ridges there will be.
    int swervechance=4; // The higher this is, the straighter they will be.
    int minlength=2;
    int maxlength=4; // Possible lengths of these extra ridges.
    
    for (int t=1; t<=2; t++) // Do the whole thing twice!
    {
        for (int i=0; i<=width; i++)
        {
            for (int j=1; j<height; j++)
            {
                if (rawmountains[i][j]!=0 && random(1,extrachance)==1)
                {
                    int lefti=i-1;
                    
                    if (lefti<0)
                        lefti=width;
                    
                    int righti=i+1;
                    if (righti>width)
                        righti=0;
                    
                    int dir=0;
                    int x=0;
                    int y=0;
                    
                    if (rawmountains[i][j-1]!=0)
                    {
                        if (random(1,2)==1)
                            x=i-2;
                        else
                            x=i+2;
                        
                        y=j;
                        
                        if (random(1,2)==1)
                            dir=1;
                        else
                            dir=5;
                    }
                    
                    if (rawmountains[righti][j-1]!=0)
                    {
                        if (random(1,2)==1)
                        {
                            x=i-1;
                            y=j-1;
                        }
                        else
                        {
                            x=i+1;
                            y=j+1;
                        }
                        
                        if (random(1,2)==1)
                            dir=2;
                        else
                            dir=6;
                    }
                    
                    if (rawmountains[righti][j]!=0)
                    {
                        if (random(1,2)==1)
                            y=j-2;
                        else
                            y=j+2;
                        
                        x=i;
                        
                        if (random(1,2)==1)
                            dir=3;
                        else
                            dir=7;
                    }
                    
                    if (rawmountains[righti][j+1]!=0)
                    {
                        if (random(1,2)==1)
                        {
                            x=i+1;
                            y=j-1;
                        }
                        else
                        {
                            x=i-1;
                            y=j+1;
                        }
                        
                        if (random(1,2)==1)
                            dir=4;
                        else
                            dir=8;
                    }
                    
                    if (rawmountains[i][j+1]!=0)
                    {
                        if (random(1,2)==1)
                            x=i-2;
                        else
                            x=i+2;
                        
                        y=j;
                        
                        if (random(1,2)==1)
                            dir=5;
                        else
                            dir=1;
                    }
                    
                    if (rawmountains[lefti][j+1]!=0)
                    {
                        if (random(1,2)==1)
                        {
                            x=i-1;
                            y=j-1;
                        }
                        else
                        {
                            x=i+1;
                            y=j+1;
                        }
                        
                        if (random(1,2)==1)
                            dir=6;
                        else
                            dir=2;
                    }
                    
                    if (rawmountains[lefti][j]!=0)
                    {
                        if (random(1,2)==1)
                            y=j-2;
                        else
                            y=j+2;
                        
                        x=i;
                        
                        if (random(1,2)==1)
                            dir=7;
                        else
                            dir=3;
                    }
                    
                    if (rawmountains[lefti][j-1]!=0)
                    {
                        if (random(1,2)==1)
                        {
                            x=i+1;
                            y=j-1;
                        }
                        else
                        {
                            x=i-1;
                            y=j+1;
                        }
                        
                        if (random(1,2)==1)
                            dir=8;
                        else
                            dir=4;
                    }
                    
                    if (dir!=0)
                    {
                        int peakheight=rawmountains[i][j];
                        int origpeakheight=peakheight;
                        
                        int length=random(minlength,maxlength);
                        
                        for (int n=1; n<=length; n++)
                        {
                            if (dir==8 || dir==1 || dir==2)
                                y--;
                            
                            if (dir==4 || dir==5 || dir==6)
                                y++;
                            
                            if (dir==2 || dir==3 || dir==4)
                                x++;
                            
                            if (dir==6 || dir==7 || dir==8)
                                x--;
                            
                            if (x<0)
                                x=width;
                            
                            if (x>width)
                                x=0;
                            
                            if (y>=0 && y<=height)
                            {
                                extraraw[x][y]=peakheight;
                                
                                if (random(1,swervechance)==1)
                                {
                                    if (random(1,2)==1)
                                        dir--;
                                    else
                                        dir++;
                                    
                                    if (dir==0)
                                        dir=8;
                                    
                                    if (dir==9)
                                        dir=1;
                                }
                                
                                peakheight=peakheight+randomsign(random(1,5));
                                
                                if (peakheight<origpeakheight/2)
                                    peakheight=origpeakheight/2;
                                
                                if (peakheight>origpeakheight)
                                    peakheight=origpeakheight;
                                
                            }
                            else
                                n=length;
                        }
                    }
                }
            }
        }
        
        // Now just add those extra raw ridges to the main raw array.
        
        for (int i=0; i<=width; i++)
        {
            for (int j=0; j<=height; j++)
            {
                if (rawmountains[i][j]<extraraw[i][j])
                    rawmountains[i][j]=extraraw[i][j];
                
                extraraw[i][j]=0;
            }
        }
    }
    
    // Now get rid of straight sections, where possible.
    
    for (int n=1; n<=2; n++) // Do this whole thing twice, to be sure.
    {
        for (int i=0; i<=width; i++)
        {
            for (int j=1; j<height; j++)
            {
                if (rawmountains[i][j]!=0)
                {
                    int lefti=i-1;
                    
                    if (lefti<0)
                        lefti=width;
                    
                    int righti=i+1;
                    
                    if (righti>width)
                        righti=0;
                    
                    // First, east and west.
                    
                    if (rawmountains[lefti][j]!=0 && rawmountains[righti][j]!=0)
                    {
                        bool up=0;
                        
                        if (random(1,2)==1)
                            up=1;
                        
                        if (up==1)
                        {
                            rawmountains[i][j-1]=rawmountains[i][j];
                            rawmountains[i][j]=0;
                        }
                        else
                        {
                            rawmountains[i][j+1]=rawmountains[i][j];
                            rawmountains[i][j]=0;
                        }
                        
                        if (random(1,2)==1) // Maybe move one of the neighbouring ones too
                        {
                            int ii=lefti;
                            
                            if (random(1,2)==1)
                                ii=righti;
                            
                            if (up==1)
                            {
                                rawmountains[ii][j-1]=rawmountains[ii][j];
                                rawmountains[ii][j]=0;
                            }
                            else
                            {
                                rawmountains[ii][j+1]=rawmountains[ii][j];
                                rawmountains[ii][j]=0;
                            }
                        }
                        
                    }
                    
                    // Now, north and south.
                    
                    if (rawmountains[i][j-1]!=0 && rawmountains[i][j+1]!=0)
                    {
                        bool left=0;
                        
                        if (random(1,2)==1)
                            left=1;
                        
                        if (left==1)
                        {
                            rawmountains[lefti][j]=rawmountains[i][j];
                            rawmountains[i][j]=0;
                        }
                        else
                        {
                            rawmountains[righti][j]=rawmountains[i][j];
                            rawmountains[i][j]=0;
                        }
                        
                        if (random(1,2)==1) // Maybe move one of the neighbouring ones too
                        {
                            int jj=j-1;
                            
                            if (random(1,2)==1)
                                jj=j+1;
                            
                            if (left==1)
                            {
                                rawmountains[lefti][jj]=rawmountains[i][jj];
                                rawmountains[i][jj]=0;
                            }
                            else
                            {
                                rawmountains[righti][jj]=rawmountains[i][jj];
                                rawmountains[i][jj]=0;
                            }
                        }
                    }
                    
                    // Now, diagonals from NW to SE.
                    
                    if (rawmountains[i-1][j-1]!=0 && rawmountains[i+1][j+1]!=0)
                    {
                        bool left=0;
                        
                        if (random(1,2)==1)
                            left=1;
                        
                        if (left==1)
                        {
                            rawmountains[lefti][j]=rawmountains[i][j];
                            rawmountains[i][j+1]=rawmountains[i][j];
                            rawmountains[i][j]=0;
                        }
                        else
                        {
                            rawmountains[righti][j]=rawmountains[i][j];
                            rawmountains[i][j-1]=rawmountains[i][j];
                            rawmountains[i][j]=0;
                        }
                    }
                }
            }
        }
    }
    
    // Now, work out the distances from the central peaks.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (rawmountains[i][j]!=0)
            {
                float peakheight=rawmountains[i][j];
                
                int mult=0;
                
                for (int radius=maxradius; radius>=1; radius--)
                {
                    mult++;
                    
                    for (int k=-radius; k<=radius; k++)
                    {
                        for (int l=-radius; l<=radius; l++)
                        {
                            int dist=k*k+l*l;
                            
                            if (dist<radius*radius+radius)
                            {
                                int kk=i+k;
                                
                                if (kk<0 || kk>width)
                                    kk=wrap(kk,width);
                                
                                int ll=j+l;
                                
                                if (ll>=0 && ll<=height)
                                {
                                    dist=maxradius-dist;
                                    
                                    if (mountaindist[kk][ll]<dist)
                                    {
                                        mountaindist[kk][ll]=dist;
                                        mountainbaseheight[kk][ll]=peakheight;
                                        
                                        if (k<0 || l<0)
                                        {
                                            mountaindist[kk][ll]=dist*10;
                                            timesten[kk][ll]=1;
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
    
    float heightmult=1.0/maxradius;
    float heightmult2=1.0/maxelev;
    
    float finalmult=0.6; // Reduce it all! Because we'll have raised ground underneath.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            float thisheight=mountainbaseheight[i][j];
            
            float thismountaindist=mountaindist[i][j];
            
            if (timesten[i][j]==1)
                thismountaindist=thismountaindist/10;
            
            if (thismountaindist<maxradius-1)
                thismountaindist=thismountaindist*0.8;
            
            if (thismountaindist<maxradius-2)
                thismountaindist=thismountaindist*0.8;

            float distmult=radiusfractal[i][j];
            distmult=distmult*heightmult2;
            
            if (distmult<0.95)
                distmult=0.95;
            
            if (thismountaindist>2)
                thismountaindist=thismountaindist*distmult;

            thisheight=thisheight*thismountaindist*heightmult;
            
            float thisheightmult=heightsfractal[i][j]; // This is to vary it using the fractal
            thisheightmult=thisheightmult*heightmult2;
            
            if (thisheightmult<0.95)
                thisheightmult=0.95;
            
            thisheight=thisheight*thisheightmult;
            
            thisheight=thisheight*finalmult;
            
            if (thisheight>mountainbaseheight[i][j])
                thisheight=mountainbaseheight[i][j];
            
            mountainheights[i][j]=thisheight;
        }
    }
    
    // Now we draw out the ridges of the mountains, based on similar distances from the central peaks.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (mountaindist[i][j]!=0)
            {
                int ii=i; // Looking north
                int jj=j-1;
                int dir=1;
                
                if (jj>=0)
                {
                    if (mountaindist[ii][jj]==mountaindist[i][j])
                    {
                        if (getridge(mountainridges,i,j,dir)==0)
                        {
                            int code=getcode(dir);
                            mountainridges[i][j]=mountainridges[i][j]+code;
                        }
                    }
                }
                
                ii=i+1; // Looking northeast
                jj=j-1;
                dir=2;
                
                if (ii>width)
                    ii=0;
                
                if (jj>=0)
                {
                    if (mountaindist[ii][jj]==mountaindist[i][j])
                    {
                        if (getridge(mountainridges,i,j,dir)==0)
                        {
                            int code=getcode(dir);
                            mountainridges[i][j]=mountainridges[i][j]+code;
                        }
                    }
                }
                
                ii=i+1; // Looking east
                jj=j;
                dir=3;
                
                if (ii>width)
                    ii=0;
                
                if (mountaindist[ii][jj]==mountaindist[i][j])
                {
                    if (getridge(mountainridges,i,j,dir)==0)
                    {
                        int code=getcode(dir);
                        mountainridges[i][j]=mountainridges[i][j]+code;
                    }
                }
                
                ii=i+1; // Looking southeast
                jj=j+1;
                dir=4;
                
                if (ii>width)
                    ii=0;
                
                if (jj<=height)
                {
                    if (mountaindist[ii][jj]==mountaindist[i][j])
                    {
                        if (getridge(mountainridges,i,j,dir)==0)
                        {
                            int code=getcode(dir);
                            mountainridges[i][j]=mountainridges[i][j]+code;
                        }
                    }
                }
                
                ii=i; // Looking south
                jj=j+1;
                dir=5;
                
                if (jj<=height)
                {
                    if (mountaindist[ii][jj]==mountaindist[i][j])
                    {
                        if (getridge(mountainridges,i,j,dir)==0)
                        {
                            int code=getcode(dir);
                            mountainridges[i][j]=mountainridges[i][j]+code;
                        }
                    }
                }
                
                ii=i-1; // Looking southwest
                jj=j+1;
                dir=6;
                
                if (ii<0)
                    ii=width;
                
                if (jj<=height)
                {
                    if (mountaindist[ii][jj]==mountaindist[i][j])
                    {
                        if (getridge(mountainridges,i,j,dir)==0)
                        {
                            int code=getcode(dir);
                            mountainridges[i][j]=mountainridges[i][j]+code;
                        }
                    }
                }
                
                ii=i-1; // Looking west
                jj=j;
                dir=7;
                
                if (ii<0)
                    ii=width;
                
                if (mountaindist[ii][jj]==mountaindist[i][j])
                {
                    if (getridge(mountainridges,i,j,dir)==0)
                    {
                        int code=getcode(dir);
                        mountainridges[i][j]=mountainridges[i][j]+code;
                    }
                }
                
                ii=i-1; // Looking northwest
                jj=j-1;
                dir=8;
                
                if (ii<0)
                    ii=width;
                
                if (jj>=0)
                {
                    if (mountaindist[ii][jj]==mountaindist[i][j])
                    {
                        if (getridge(mountainridges,i,j,dir)==0)
                        {
                            int code=getcode(dir);
                            mountainridges[i][j]=mountainridges[i][j]+code;
                        }
                    }
                }
            }
        }
    }
    
    {
    /*
    // Now add in the main, central ridge.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=1; j<height; j++)
        {
            if (rawmountains[i][j]!=0)
            {
                mountainheights[i][j]=rawmountains[i][j];
                
                int dir, ii, jj;
                
                // Looking north
                
                dir=1;
                ii=i;
                jj=j-1;
                
                if (rawmountains[ii][jj]!=0)
                {
                    if (getridge(mountainridges,i,j,dir)==0)
                    {
                        int code=getcode(dir);
                        mountainridges[i][j]=mountainridges[i][j]+code;
                    }
                    
                }
                
                // Looking northeast
                
                dir=2;
                ii=i+1;
                jj=j-1;
                
                if (ii<0 || ii>width)
                    ii=wrap(ii,width);
                
                if (rawmountains[ii][jj]!=0)
                {
                    if (getridge(mountainridges,i,j,dir)==0)
                    {
                        int code=getcode(dir);
                        mountainridges[i][j]=mountainridges[i][j]+code;
                    }
                    
                }
                
                // Looking east
                
                dir=3;
                ii=i+1;
                jj=j;
                
                if (ii<0 || ii>width)
                    ii=wrap(ii,width);
                
                if (rawmountains[ii][jj]!=0)
                {
                    if (getridge(mountainridges,i,j,dir)==0)
                    {
                        int code=getcode(dir);
                        mountainridges[i][j]=mountainridges[i][j]+code;
                    }
                    
                }
                
                // Looking southeast
                
                dir=4;
                ii=i+1;
                jj=j+1;
                
                if (ii<0 || ii>width)
                    ii=wrap(ii,width);
                
                if (rawmountains[ii][jj]!=0)
                {
                    if (getridge(mountainridges,i,j,dir)==0)
                    {
                        int code=getcode(dir);
                        mountainridges[i][j]=mountainridges[i][j]+code;
                    }
                    
                }
                
                // Looking south
                
                dir=5;
                ii=i;
                jj=j+1;
                
                if (rawmountains[ii][jj]!=0)
                {
                    if (getridge(mountainridges,i,j,dir)==0)
                    {
                        int code=getcode(dir);
                        mountainridges[i][j]=mountainridges[i][j]+code;
                    }
                    
                }
                
                // Looking southwest
                
                dir=6;
                ii=i-1;
                jj=j+1;
                
                if (ii<0 || ii>width)
                    ii=wrap(ii,width);
                
                if (rawmountains[ii][jj]!=0)
                {
                    if (getridge(mountainridges,i,j,dir)==0)
                    {
                        int code=getcode(dir);
                        mountainridges[i][j]=mountainridges[i][j]+code;
                    }
                    
                }
                
                // Looking west
                
                dir=7;
                ii=i-1;
                jj=j;
                
                if (ii<0 || ii>width)
                    ii=wrap(ii,width);
                
                if (rawmountains[ii][jj]!=0)
                {
                    if (getridge(mountainridges,i,j,dir)==0)
                    {
                        int code=getcode(dir);
                        mountainridges[i][j]=mountainridges[i][j]+code;
                    }
                    
                }
                
                // Looking northwest
                
                dir=8;
                ii=i-1;
                jj=j-1;
                
                if (ii<0 || ii>width)
                    ii=wrap(ii,width);
                
                if (rawmountains[ii][jj]!=0)
                {
                    if (getridge(mountainridges,i,j,dir)==0)
                    {
                        int code=getcode(dir);
                        mountainridges[i][j]=mountainridges[i][j]+code;
                    }
                    
                }
            }
        }
    }
    */
    }
    
    // Now we want to add some variation to the ridge directions.
    
    int varchance=3; // The lower this is, the more variation there will be.
    int deletechance=2; // The higher this is, the more extra ridges will be deleted.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=1; j<height; j++)
        {
            if (mountainridges[i][j]!=0)
            {
                int dir, dir2, ii, jj;
                
                if (getridge(mountainridges,i,j,1)==1 && random(1,varchance)==1) // Looking north
                {
                    dir=8;
                    dir2=4;
                    ii=i-1;
                    jj=j-1;
                    
                    if (random(1,2)==1)
                    {
                        dir=2;
                        dir2=6;
                        ii=i+1;
                        jj=j-1;
                    }
                    
                    if (ii<0 || ii>width)
                        ii=wrap(ii,width);
                    
                    if (getridge(mountainridges,i,j,dir)==0) // If there isn't one in this direction
                    {
                        int code=getcode(dir);
                        mountainridges[i][j]=mountainridges[i][j]+code;
                        
                        code=getcode(dir2);
                        mountainridges[ii][jj]=mountainridges[ii][jj]+code;
                        
                        if (mountainheights[ii][jj]==0)
                            mountainheights[ii][jj]=mountainheights[i][j];
                        
                        if (random(1,deletechance)!=1)
                            deleteridge(world,mountainridges,mountainheights,i,j,1);
                    }
                }
                else
                {
                    if (getridge(mountainridges,i,j,2)==1 && random(1,varchance)==1) // Looking northeast
                    {
                        dir=1;
                        dir2=5;
                        ii=i;
                        jj=j-1;
                        
                        if (random(1,2)==1)
                        {
                            dir=3;
                            dir2=7;
                            ii=i+1;
                            jj=j;
                        }
                        
                        if (ii<0 || ii>width)
                            ii=wrap(ii,width);
                        
                        if (getridge(mountainridges,i,j,dir)==0) // If there isn't one in this direction
                        {
                            int code=getcode(dir);
                            mountainridges[i][j]=mountainridges[i][j]+code;
                            
                            code=getcode(dir2);
                            mountainridges[ii][jj]=mountainridges[ii][jj]+code;
                            
                            if (mountainheights[ii][jj]==0)
                                mountainheights[ii][jj]=mountainheights[i][j];
                            
                            if (random(1,deletechance)!=1)
                                deleteridge(world,mountainridges,mountainheights,i,j,2);
                        }
                    }
                    else
                    {
                        if (getridge(mountainridges,i,j,3)==1 && random(1,varchance)==1) // Looking east
                        {
                            dir=2;
                            dir2=6;
                            ii=i+1;
                            jj=j-1;
                            
                            if (random(1,2)==1)
                            {
                                dir=4;
                                dir2=8;
                                ii=i+1;
                                jj=j+1;
                            }
                            
                            if (ii<0 || ii>width)
                                ii=wrap(ii,width);
                            
                            if (getridge(mountainridges,i,j,dir)==0) // If there isn't one in this direction
                            {
                                int code=getcode(dir);
                                mountainridges[i][j]=mountainridges[i][j]+code;
                                
                                code=getcode(dir2);
                                mountainridges[ii][jj]=mountainridges[ii][jj]+code;
                                
                                if (mountainheights[ii][jj]==0)
                                    mountainheights[ii][jj]=mountainheights[i][j];
                                
                                if (random(1,deletechance)!=1)
                                    deleteridge(world,mountainridges,mountainheights,i,j,3);
                            }
                        }
                        else
                            if (getridge(mountainridges,i,j,4)==1 && random(1,varchance)==1) // Looking southeast
                            {
                                dir=3;
                                dir2=7;
                                ii=i+1;
                                jj=j;
                                
                                if (random(1,2)==1)
                                {
                                    dir=5;
                                    dir2=1;
                                    ii=i;
                                    jj=j+1;
                                }
                                
                                if (ii<0 || ii>width)
                                    ii=wrap(ii,width);
                                
                                if (getridge(mountainridges,i,j,dir)==0) // If there isn't one in this direction
                                {
                                    int code=getcode(dir);
                                    mountainridges[i][j]=mountainridges[i][j]+code;
                                    
                                    code=getcode(dir2);
                                    mountainridges[ii][jj]=mountainridges[ii][jj]+code;
                                    
                                    if (mountainheights[ii][jj]==0)
                                        mountainheights[ii][jj]=mountainheights[i][j];
                                    
                                    if (random(1,deletechance)!=1)
                                        deleteridge(world,mountainridges,mountainheights,i,j,4);
                                }
                            }
                    }
                }
            }
        }
    }

    /*
    // Now remove extraneous ridges.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (mountainridges[i][j]!=0)
            {
                short total=0;
                
                for (int dir=1; dir<=8; dir++)
                {
                    if (getridge(mountainridges,i,j,dir)==1)
                        total++;
                }
                
                if (total>3)
                {
                    int extra=total-3;
                    
                    for (int n=1; n<=extra; n++)
                    {
                        int a=1;
                        int b=8;
                        int c=1;
                        
                        if (random(1,2)==1)
                        {
                            a=8;
                            b=1;
                            c=-1;
                        }
                        
                        for (int dir=a; dir!=b; dir=dir+c)
                        {
                            if (getridge(mountainridges,i,j,dir)==1)
                                deleteridge(world,mountainridges,mountainheights,i,j,dir);
                        }
                    }
                }
            }
        }
    }
    */

    
    // Now apply the new mountain heights and ridges to the world.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (mountainridges[i][j]!=0)
            {
                world.setmountainridge(i,j,mountainridges[i][j]);
                world.setmountainheight(i,j,mountainheights[i][j]);
            }
        }
    }

    removefloatingmountains(world);
    cleanmountainridges(world);
}

// This scatters islands across areas where land has been removed.

void makearchipelagos(planet &world, vector<vector<bool>> &removedland, boolshapetemplate landshape[])
{
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    int maxelev=world.maxelevation();
    int conheight=sealevel+50;
    
    int baseheight=sealevel-4500;
    if (baseheight<1)
        baseheight=1;
    
    int grain=8; // This is a fractal, for the islands
    float valuemod=0.05;
    float valuemod2=valuemod;
    
    vector<vector<int>> islandfractal(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
    
    createfractal(islandfractal,width,height,grain,valuemod,valuemod2,1,maxelev,0,0);
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            int thisrandom=random(1,maxelev*100);
            
            int islandchance=0;
            
            if (islandfractal[i][j]<maxelev/4)
                islandchance=70;
            
            if (islandfractal[i][j]<maxelev/5)
                islandchance=40;
            
            if (islandfractal[i][j]<maxelev/6)
                islandchance=30;
            
            int ii=i+width/2;
            
            if (ii>width)
                ii=ii-width;
            
            if (islandchance!=0 && islandfractal[ii][j]>maxelev/3 && removedland[i][j]==1 && world.sea(i,j)==1 && random(1,islandchance)==1) // Put an island here.
            {
                int peakheight=random(500,3000);
                makemountainisland(world,i,j,peakheight);
            }
        }
    }
    
    int amount=2; // No mountain islands this close to existing land (at least not marked as such).
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (world.mountainisland(i,j)==1)
            {
                bool nearland=0;
                
                for (int k=i-amount; k<=i+amount; k++)
                {
                    int kk=k;
                    
                    if (kk<0 || kk>width)
                        kk=wrap(kk,width);
                    
                    for (int l=j-amount; l<=j+amount; l++)
                    {
                        if (l>=0 && l<=height)
                        {
                            if (world.sea(kk,l)==0 && world.mountainisland(kk,l)==0)
                            {
                                nearland=1;
                                k=i+amount;
                                l=j+amount;
                            }
                        }
                    }
                }
                
                if (nearland==1)
                    world.setmountainisland(i,j,0);
            }
        }
    }
}

// This makes a small, mountainous island.

void makemountainisland(planet &world, int x, int y, int peakheight)
{
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    
    int iterations=random(1,3);
    
    for (int n=1; n<=iterations; n++)
    {
        int landheight=sealevel+random(5,50);
        
        int xx=x;
        int yy=y;
        
        do
        {
            xx=x-2+random(1,3);
            yy=y-2+random(1,3);
            
        } while (x==xx && y==yy);
        
        if (yy<0 || yy>height)
            return;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);

        if (world.mountainheight(x,y)<peakheight)
            world.setmountainheight(x,y,peakheight);
        
        if (world.mountainheight(xx,yy)<peakheight)
            world.setmountainheight(xx,yy,peakheight);
        
        for (int i=x-1; i<=x+1; i++) // Mark it all out as mountain islands!
        {
            int ii=i;
            
            if (ii<0 || ii>width)
                ii=wrap(ii,width);
            
            for (int j=y-1; j<=y+1; j++)
            {
                if (j>=0 && j<=height)
                    world.setmountainisland(i,j,1);
            }
        }
        
        for (int i=xx-1; i<=xx+1; i++)
        {
            int ii=i;
            
            if (ii<0 || ii>width)
                ii=wrap(ii,width);
            
            for (int j=yy-1; j<=yy+1; j++)
            {
                if (j>=0 && j<=height)
                    world.setmountainisland(i,j,1);
            }
        }

        if (world.nom(x,y)<landheight)
            world.setnom(x,y,landheight);
        
        if (world.nom(xx,yy)<landheight)
            world.setnom(xx,yy,landheight);
        
        // If there isn't a ridge between them, make one.
        
        int dir=getdir(x,y,xx,yy);
        
        if (getridge(world,x,y,dir)==0)
        {
            int code=getcode(dir);
            
            world.setmountainridge(x,y,world.mountainridge(x,y)+code);
            
            dir=dir+8;
            
            if (dir>8)
                dir=dir-8;
            
            code=getcode(dir);
            
            world.setmountainridge(xx,yy,world.mountainridge(xx,yy)+code);
        }
        
        //world.settest(x,y,1);
        
        x=xx;
        y=yy;
        
        peakheight=peakheight+randomsign(random(peakheight/4,peakheight/2));
    }
}

// This makes an isolated volcano, together with a line of extinct neighbours.

void createisolatedvolcano(planet &world, int x, int y, vector<vector<bool>> &shelves, vector<vector<int>> &volcanodirection, int peakheight, bool strato)
{
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    int maxelev=world.maxelevation();
    
    float maxshift=6;
    float shiftdiv=maxshift/(maxelev/2);
    
    int shelfcheck=5;
    
    if (world.sea(x,y)==1) // Don't do any submarine volcanos near continental shelves.
    {
        for (int i=x-shelfcheck; i<=x+shelfcheck; i++)
        {
            int ii=i;
            
            if (ii<0 || ii>width)
                ii=wrap(ii,width);
            
            for (int j=y-shelfcheck; j<=y+shelfcheck; j++)
            {
                if (j>=0 && j<=height)
                {
                    if (shelves[ii][j]==1)
                        return;
                }
            }
        }
    }
    
    bool active=1;
    
    int total=random(1,6);
    
    for (int n=1; n!=total; n++)
    {
        if (world.sea(x,y)==1)
        {
            for (int i=x-1; i<=x+1; i++) // No shading around undersea volcanos.
            {
                int ii=i;
                
                if (ii<0 || ii>width)
                    ii=wrap(ii,width);
                
                for (int j=y-1; j<=y+1; j++)
                {
                    if (j>=0 && j<=height)
                        world.setnoshade(i,j,1);
                }
            }
        }
        
        float thispeakheight=peakheight;
        
        thispeakheight=thispeakheight/100;
        
        thispeakheight=thispeakheight*random(80,120);
        
        if (active==0)
            thispeakheight=0-peakheight;
        
        for (int i=x-1; i<=x+1; i++) // Remove any volcanos immediately around
        {
            int ii=i;
            
            if (ii<0 || ii>width)
                ii=wrap(ii,width);
            
            for (int j=y-1; j<=y+1; j++)
            {
                if (j>=0 && j<=height)
                {
                    world.setvolcano(ii,j,0);
                    //world.settest(ii,j,0);
                }
            }
        }
        
        world.setvolcano(x,y,thispeakheight);
        //world.settest(x,y,1);
        world.setstrato(x,y,strato);
        
        active=0;
        
        int xx=x+width/2;
        
        if (xx>width)
            xx=xx-width;
        
        float xshift=volcanodirection[x][y]-maxelev/2;
        float yshift=volcanodirection[xx][y]-maxelev/2;
        
        xshift=xshift*shiftdiv+randomsign(random(0,2));
        yshift=yshift*shiftdiv+randomsign(random(0,2));
        
        x=x+xshift;
        
        if (x<0 || x>width)
            x=wrap(x,width);
        
        y=y+yshift;
        
        if (y<0 || y>height)
            return;
        
        if (world.sea(x,y)==1) // Don't do any submarine volcanos near continental shelves.
        {
            for (int i=x-shelfcheck; i<=x+shelfcheck; i++)
            {
                int ii=i;
                
                if (ii<0 || ii>width)
                    ii=wrap(ii,width);
                
                for (int j=y-shelfcheck; j<=y+shelfcheck; j++)
                {
                    if (j>=0 && j<=height)
                    {
                        if (shelves[ii][j]==1)
                            return;
                    }
                }
            }
        }
    }
}

// This notes down slopes on the seabed.

void getseaslopes(planet &world, vector<vector<int>> &slopes)
{
    int width=world.width();
    int height=world.height();
    
    int slope1, slope2, slope3, slope4, slope5, slope6, slope7, slope8, biggestslope;
    
    for (int i=0; i<=width; i++)
    {
        int iminus=i-1;
        if (iminus<0)
            iminus=width;
        
        int iplus=i+1;
        if (iplus>width)
            iplus=0;
        
        for (int j=1; j<height; j++)
        {
            int jminus=j-1;
            int jplus=j+1;
            
            slope1=getslope(world,i,j,i,jminus);
            slope2=getslope(world,i,j,iplus,jminus);
            slope3=getslope(world,i,j,iplus,j);
            slope4=getslope(world,i,j,iplus,jplus);
            slope5=getslope(world,i,j,iminus,jplus);
            slope6=getslope(world,i,j,iminus,jplus);
            slope7=getslope(world,i,j,iminus,j);
            slope8=getslope(world,i,j,iminus,jminus);
            
            biggestslope=slope1;
            
            if (slope2>biggestslope)
                biggestslope=slope2;
            
            if (slope3>biggestslope)
                biggestslope=slope3;
            
            if (slope4>biggestslope)
                biggestslope=slope4;
            
            if (slope5>biggestslope)
                biggestslope=slope5;
            
            if (slope6>biggestslope)
                biggestslope=slope6;
            
            if (slope7>biggestslope)
                biggestslope=slope7;
            
            if (slope8>biggestslope)
                biggestslope=slope8;
            
            slopes[i][j]=biggestslope;
        }
    }
}

// This removes odd bumps on the sea floor.

void removeunderseabumps(planet &world)
{
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    
    int maxdiff=100; // If the cell being looked at is more than the average of its neighbours plus this, it will be lowered.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=1; j<height; j++)
        {
            if (world.sea(i,j)==1 && world.oceanridges(i,j)==0)
            {
                int total=0;
                int crount=0;
                
                for (int k=i-1; k<=i+1; k++)
                {
                    int kk=k;
                    
                    if (kk<0 || kk>width)
                        kk=wrap(kk,width);
                    
                    for (int l=j-1; l<=j+1; l++)
                    {
                        if ((k!=i || l!=j) && world.sea(kk,l)==1)
                        {
                            total=total+world.nom(kk,l);
                            crount++;
                        }
                    }
                }
                
                if (crount>0)
                {
                    int ave=total/crount;
                    
                    if (world.nom(i,j)>ave+maxdiff)
                    {
                        world.setnom(i,j,ave);
                    }
                }
            }
        }
    }
}

// This removes odd bits of land at the southern edge of the map.

void checkpoles(planet &world)
{
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    
    for (int i=0; i<=width; i++)
    {
        int bottomelev=world.map(i,height);
        int upelev=world.map(i,height-1);
        
        if (bottomelev>sealevel && upelev<=sealevel)
            world.setnom(i,height,upelev);
    }
}

