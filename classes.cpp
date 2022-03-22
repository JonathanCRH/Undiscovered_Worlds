//
//  classes.cpp
//  Undiscovered Worlds
//
//  Created by Jonathan Hill on 17/04/2020.
//  
//  Please see functions.hpp for notes.

#include "classes.hpp"
#include "functions.hpp"
#include <list>

// Peaktemplate class

peaktemplate::peaktemplate() // constructor
{
}
peaktemplate::~peaktemplate()  // destructor
{
}

int peaktemplate::span(int index) const {return itsspan[index];}
void peaktemplate::setspan(int index, int amount) {itsspan[index]=amount;}

int peaktemplate::centrex(int index) const {return itscentrex[index];}
void peaktemplate::setcentrex(int index, int amount) {itscentrex[index]=amount;}

int peaktemplate::centrey(int index) const {return itscentrey[index];}
void peaktemplate::setcentrey(int index, int amount) {itscentrey[index]=amount;}

int peaktemplate::peakmap(int index, int x, int y) const
{
    if (x<0 || x>PEAKWIDTH || y<0 || y>PEAKHEIGHT)
        return 0;
    
    return itspeakmap[index][x][y];
    
}

void peaktemplate::setpeakmap(int index, int x, int y, int amount) {itspeakmap[index][x][y]=amount;}

// boolshapetemplate class

boolshapetemplate::boolshapetemplate()
{
}
boolshapetemplate::~boolshapetemplate()
{
}

void boolshapetemplate::clear()
{
    for (int i=0; i<itsvector.size(); i++)
    {
        for (int j=0; j<itsvector[i].size(); j++)
            itsvector[i][j]=0;
    }
}

// byteshapetemplate class

byteshapetemplate::byteshapetemplate()
{
}
byteshapetemplate::~byteshapetemplate()
{
}

void byteshapetemplate::clear()
{
    for (int i=0; i<itsvector.size(); i++)
    {
        for (int j=0; j<itsvector[i].size(); j++)
            itsvector[i][j]=0;
    }
}

