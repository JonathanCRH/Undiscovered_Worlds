//
//  classes.hpp
//  Undiscovered Worlds
//
//  Created by Jonathan Hill on 17/04/2020.
//  
//  Please see functions.hpp for notes.

#ifndef classes_hpp
#define classes_hpp

#include <iostream>
#include <cmath>
#include <fstream>
#include <stdio.h>
#include <unistd.h>
#include <queue>
#include <SFML/Graphics.hpp>
#include <nanogui/nanogui.h>

#define PEAKWIDTH 20
#define PEAKHEIGHT 20
#define PEAKTOTAL 5

using namespace std;

// This class holds the templates for the mountain peaks. All the templates are held in a single object and accessed by index.

class peaktemplate
{
public:
    
    peaktemplate(); // constructor
    ~peaktemplate();  // destructor
    
    // Accessor functions
    
    int span(int index) const;   // Width/height
    void setspan(int index, int amount);
    
    int centrex(int index) const; // Centre x coordinate
    void setcentrex(int index, int amount);

    int centrey(int index) const; // Centre y coordinate
    void setcentrey(int index, int amount);
    
    int peakmap(int index, int x, int y) const; // Peak height map
    void setpeakmap(int index, int x, int y, int amount);

private:
    
    int itsspan[PEAKTOTAL]; // Width/height of the peak
    int itscentrex[PEAKTOTAL];
    int itscentrey[PEAKTOTAL]; // Coordinates of the centre of the peak
    
    int itspeakmap[PEAKTOTAL][PEAKWIDTH][PEAKHEIGHT]; // Map of the peak heights
};

// This is basically a wrapper for a 2D vector, to store a template of bools. (We make the vector bigger than we let on, just for safety's sake.)

class boolshapetemplate
{
public:
    
    boolshapetemplate(); // constructor
    ~boolshapetemplate();  // destructor
    
    // Accessor functions

    int xsize() const {return itsvector.size()-1;};
    void setxsize(int amount){itsvector.resize(amount+1);};
    
    int ysize() const {return itsvector[0].size()-1;};
    void setysize(int amount)
    {
        for (int n=0; n<itsvector.size(); n++)
            itsvector[n].resize(amount+1);
    };
    
    bool point(int x, int y) const {return itsvector[x][y];}
    void setpoint(int x, int y, bool amount) {itsvector[x][y]=amount;}
    
    // Other functions
    
    void clear();
    
private:
    
    std::vector<std::vector<bool>> itsvector;
};

// Same thing, but for unsigned chars.

class byteshapetemplate
{
public:
    
    byteshapetemplate(); // constructor
    ~byteshapetemplate();  // destructor
    
    // Accessor functions
    
    int xsize() const {return itsvector.size()-1;};
    void setxsize(int amount){itsvector.resize(amount+1);};
    
    int ysize() const {return itsvector[0].size()-1;};
    void setysize(int amount)
    {
        for (int n=0; n<itsvector.size(); n++)
            itsvector[n].resize(amount+1);
    };
    
    unsigned char point(int x, int y) const {return itsvector[x][y];}
    void setpoint(int x, int y, unsigned char amount) {itsvector[x][y]=amount;}
    
    // Other functions
    
    void clear();
    
private:
    
    std::vector<std::vector<unsigned char>> itsvector;
};



#endif /* classes_hpp */
