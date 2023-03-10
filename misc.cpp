//
//  misc.cpp
//  Undiscovered_Worlds
//
//  Created by Jonathan Hill on 23/04/2022.
//
#include <iostream>
#include <cmath>
#include <fstream>
#include <stdio.h>
#include <queue>

#include "classes.hpp"
#include "planet.hpp"
#include "region.hpp"
#include "functions.hpp"

// This toggles a bool.

void toggle(bool &val)
{
    if (val == 0)
    {
        val = 1;
        return;
    }

    val = 0;
    return;
}

// This converts a number to a string containing commas. Taken from here: https://9to5answer.com/c-format-number-with-commas

string formatnumber(int val)
{
    string s = to_string(val);

    if (val<1000 && val>-1000)
        return s;
    int n = s.length() - 3;
    while (n > 0)
    {
        s.insert(n, ",");
        n -= 3;
    }

    return s;
}

// This converts strings into bools.

bool stob(string const& instring)
{
    if (instring == "1")
        return 1;

    return 0;
}

// This converts strings into shorts. (Note: no error trapping.)

short stos(string const& instring)
{
    int val = stoi(instring);

    short s = (short)val;

    return s;
}

// This converts strings into unsigned shorts. (Note: no error trapping.)

unsigned short stous(string const& instring)
{
    int val = stoi(instring);

    unsigned short s = (unsigned short)val;

    return s;
}

// This converts strings into chars. (Note: no error trapping.)

char stoc(string const& instring)
{
    int val = stoi(instring);

    char s = (char)val;

    return s;
}

// This converts strings into unsigned chars. (Note: no error trapping.)

unsigned char stouc(string const& instring)
{
    int val = stoi(instring);

    unsigned char s = (unsigned char)val;

    return s;
}

// This function saves settings.

void savesettings(planet& world, string filename)
{
    ofstream outfile;
    outfile.open(filename, ios::out);

    outfile << world.settingssaveversion() << '\n';
    outfile << world.landshading() << '\n';
    outfile << world.lakeshading() << '\n';
    outfile << world.seashading() << '\n';
    outfile << world.shadingdir() << '\n';
    outfile << world.snowchange() << '\n';
    outfile << world.seaiceappearance() << '\n';
    outfile << world.landmarbling() << '\n';
    outfile << world.lakemarbling() << '\n';
    outfile << world.seamarbling() << '\n';
    outfile << world.minriverflowglobal() << '\n';
    outfile << world.minriverflowregional() << '\n';
    outfile << world.showmangroves() << '\n';
    outfile << world.colourcliffs() << '\n';
    outfile << world.seaice1() << '\n';
    outfile << world.seaice2() << '\n';
    outfile << world.seaice3() << '\n';
    outfile << world.ocean1() << '\n';
    outfile << world.ocean2() << '\n';
    outfile << world.ocean3() << '\n';
    outfile << world.deepocean1() << '\n';
    outfile << world.deepocean2() << '\n';
    outfile << world.deepocean3() << '\n';
    outfile << world.base1() << '\n';
    outfile << world.base2() << '\n';
    outfile << world.base3() << '\n';
    outfile << world.basetemp1() << '\n';
    outfile << world.basetemp2() << '\n';
    outfile << world.basetemp3() << '\n';
    outfile << world.highbase1() << '\n';
    outfile << world.highbase2() << '\n';
    outfile << world.highbase3() << '\n';
    outfile << world.desert1() << '\n';
    outfile << world.desert2() << '\n';
    outfile << world.desert3() << '\n';
    outfile << world.highdesert1() << '\n';
    outfile << world.highdesert2() << '\n';
    outfile << world.highdesert3() << '\n';
    outfile << world.colddesert1() << '\n';
    outfile << world.colddesert2() << '\n';
    outfile << world.colddesert3() << '\n';
    outfile << world.grass1() << '\n';
    outfile << world.grass2() << '\n';
    outfile << world.grass3() << '\n';
    outfile << world.cold1() << '\n';
    outfile << world.cold2() << '\n';
    outfile << world.cold3() << '\n';
    outfile << world.tundra1() << '\n';
    outfile << world.tundra2() << '\n';
    outfile << world.tundra3() << '\n';
    outfile << world.eqtundra1() << '\n';
    outfile << world.eqtundra2() << '\n';
    outfile << world.eqtundra3() << '\n';
    outfile << world.saltpan1() << '\n';
    outfile << world.saltpan2() << '\n';
    outfile << world.saltpan3() << '\n';
    outfile << world.erg1() << '\n';
    outfile << world.erg2() << '\n';
    outfile << world.erg3() << '\n';
    outfile << world.wetlands1() << '\n';
    outfile << world.wetlands2() << '\n';
    outfile << world.wetlands3() << '\n';
    outfile << world.lake1() << '\n';
    outfile << world.lake2() << '\n';
    outfile << world.lake3() << '\n';
    outfile << world.river1() << '\n';
    outfile << world.river2() << '\n';
    outfile << world.river3() << '\n';
    outfile << world.glacier1() << '\n';
    outfile << world.glacier2() << '\n';
    outfile << world.glacier3() << '\n';
    outfile << world.sand1() << '\n';
    outfile << world.sand2() << '\n';
    outfile << world.sand3() << '\n';
    outfile << world.mud1() << '\n';
    outfile << world.mud2() << '\n';
    outfile << world.mud3() << '\n';
    outfile << world.shingle1() << '\n';
    outfile << world.shingle2() << '\n';
    outfile << world.shingle3() << '\n';
    outfile << world.mangrove1() << '\n';
    outfile << world.mangrove2() << '\n';
    outfile << world.mangrove3() << '\n';
    outfile << world.highlight1() << '\n';
    outfile << world.highlight2() << '\n';
    outfile << world.highlight3() << '\n';

    outfile.close();
}

// This function loads settings.

bool loadsettings(planet& world, string filename)
{
    ifstream infile;
    infile.open(filename, ios::in);

    string line;
    int val;
    float fval;
    bool bval;

    getline(infile, line);
    val = stoi(line);

    if (val != world.settingssaveversion()) // Incompatible file format!
        return 0;

    getline(infile, line);
    fval = stof(line);
    world.setlandshading(fval);

    getline(infile, line);
    fval = stof(line);
    world.setlakeshading(fval);

    getline(infile, line);
    fval = stof(line);
    world.setseashading(fval);

    getline(infile, line);
    val = stoi(line);
    world.setshadingdir(val);

    getline(infile, line);
    val = stoi(line);
    world.setsnowchange(val);

    getline(infile, line);
    val = stoi(line);
    world.setseaiceappearance(val);

    getline(infile, line);
    fval = stof(line);
    world.setlandmarbling(fval);

    getline(infile, line);
    fval = stof(line);
    world.setlakemarbling(fval);

    getline(infile, line);
    fval = stof(line);
    world.setseamarbling(fval);

    getline(infile, line);
    val = stoi(line);
    world.setminriverflowglobal(val);

    getline(infile, line);
    val = stoi(line);
    world.setminriverflowregional(val);

    getline(infile, line);
    bval = stob(line);
    world.setshowmangroves(val);

    getline(infile, line);
    bval = stob(line);
    world.setcolourcliffs(val);

    getline(infile, line);
    val = stoi(line);
    world.setseaice1(val);

    getline(infile, line);
    val = stoi(line);
    world.setseaice2(val);

    getline(infile, line);
    val = stoi(line);
    world.setseaice3(val);

    getline(infile, line);
    val = stoi(line);
    world.setocean1(val);

    getline(infile, line);
    val = stoi(line);
    world.setocean2(val);

    getline(infile, line);
    val = stoi(line);
    world.setocean3(val);

    getline(infile, line);
    val = stoi(line);
    world.setdeepocean1(val);

    getline(infile, line);
    val = stoi(line);
    world.setdeepocean2(val);

    getline(infile, line);
    val = stoi(line);
    world.setdeepocean3(val);

    getline(infile, line);
    val = stoi(line);
    world.setbase1(val);

    getline(infile, line);
    val = stoi(line);
    world.setbase2(val);

    getline(infile, line);
    val = stoi(line);
    world.setbase3(val);

    getline(infile, line);
    val = stoi(line);
    world.setbasetemp1(val);

    getline(infile, line);
    val = stoi(line);
    world.setbasetemp2(val);

    getline(infile, line);
    val = stoi(line);
    world.setbasetemp3(val);

    getline(infile, line);
    val = stoi(line);
    world.sethighbase1(val);

    getline(infile, line);
    val = stoi(line);
    world.sethighbase2(val);

    getline(infile, line);
    val = stoi(line);
    world.sethighbase3(val);

    getline(infile, line);
    val = stoi(line);
    world.setdesert1(val);

    getline(infile, line);
    val = stoi(line);
    world.setdesert2(val);

    getline(infile, line);
    val = stoi(line);
    world.setdesert3(val);

    getline(infile, line);
    val = stoi(line);
    world.sethighdesert1(val);

    getline(infile, line);
    val = stoi(line);
    world.sethighdesert2(val);

    getline(infile, line);
    val = stoi(line);
    world.sethighdesert3(val);

    getline(infile, line);
    val = stoi(line);
    world.setcolddesert1(val);

    getline(infile, line);
    val = stoi(line);
    world.setcolddesert2(val);

    getline(infile, line);
    val = stoi(line);
    world.setcolddesert3(val);

    getline(infile, line);
    val = stoi(line);
    world.setgrass1(val);

    getline(infile, line);
    val = stoi(line);
    world.setgrass2(val);

    getline(infile, line);
    val = stoi(line);
    world.setgrass3(val);

    getline(infile, line);
    val = stoi(line);
    world.setcold1(val);

    getline(infile, line);
    val = stoi(line);
    world.setcold2(val);

    getline(infile, line);
    val = stoi(line);
    world.setcold3(val);

    getline(infile, line);
    val = stoi(line);
    world.settundra1(val);

    getline(infile, line);
    val = stoi(line);
    world.settundra2(val);

    getline(infile, line);
    val = stoi(line);
    world.settundra3(val);

    getline(infile, line);
    val = stoi(line);
    world.seteqtundra1(val);

    getline(infile, line);
    val = stoi(line);
    world.seteqtundra2(val);

    getline(infile, line);
    val = stoi(line);
    world.seteqtundra3(val);

    getline(infile, line);
    val = stoi(line);
    world.setsaltpan1(val);

    getline(infile, line);
    val = stoi(line);
    world.setsaltpan2(val);

    getline(infile, line);
    val = stoi(line);
    world.setsaltpan3(val);

    getline(infile, line);
    val = stoi(line);
    world.seterg1(val);

    getline(infile, line);
    val = stoi(line);
    world.seterg2(val);

    getline(infile, line);
    val = stoi(line);
    world.seterg3(val);

    getline(infile, line);
    val = stoi(line);
    world.setwetlands1(val);

    getline(infile, line);
    val = stoi(line);
    world.setwetlands2(val);

    getline(infile, line);
    val = stoi(line);
    world.setwetlands3(val);

    getline(infile, line);
    val = stoi(line);
    world.setlake1(val);

    getline(infile, line);
    val = stoi(line);
    world.setlake2(val);

    getline(infile, line);
    val = stoi(line);
    world.setlake3(val);

    getline(infile, line);
    val = stoi(line);
    world.setriver1(val);

    getline(infile, line);
    val = stoi(line);
    world.setriver2(val);

    getline(infile, line);
    val = stoi(line);
    world.setriver3(val);

    getline(infile, line);
    val = stoi(line);
    world.setglacier1(val);

    getline(infile, line);
    val = stoi(line);
    world.setglacier2(val);

    getline(infile, line);
    val = stoi(line);
    world.setglacier3(val);

    getline(infile, line);
    val = stoi(line);
    world.setsand1(val);

    getline(infile, line);
    val = stoi(line);
    world.setsand2(val);

    getline(infile, line);
    val = stoi(line);
    world.setsand3(val);

    getline(infile, line);
    val = stoi(line);
    world.setmud1(val);

    getline(infile, line);
    val = stoi(line);
    world.setmud2(val);

    getline(infile, line);
    val = stoi(line);
    world.setmud3(val);

    getline(infile, line);
    val = stoi(line);
    world.setshingle1(val);

    getline(infile, line);
    val = stoi(line);
    world.setshingle2(val);

    getline(infile, line);
    val = stoi(line);
    world.setshingle3(val);

    getline(infile, line);
    val = stoi(line);
    world.setmangrove1(val);

    getline(infile, line);
    val = stoi(line);
    world.setmangrove2(val);

    getline(infile, line);
    val = stoi(line);
    world.setmangrove3(val);

    getline(infile, line);
    val = stoi(line);
    world.sethighlight1(val);

    getline(infile, line);
    val = stoi(line);
    world.sethighlight2(val);

    getline(infile, line);
    val = stoi(line);
    world.sethighlight3(val);

    infile.close();

    return 1;
}

// This function creates a rift blob template.

void createriftblob(vector<vector<float>>& riftblob, int size)
{
    for (int i = 0; i <= size * 2; i++)
    {
        for (int j = 0; j <= size * 2; j++)
            riftblob[i][j] = 0;
    }

    float startvalue = 1.0f; // The starting value for the centre of the circle.
    float endvalue = 0.15f; // The end value for the edge of the circle.
    float step = (startvalue - endvalue) / size; // Reduce the value by this much at each step of the circle drawing.

    float currentvalue = startvalue;

    int centrex = size + 1;
    int centrey = size + 1;

    for (int radius = 1; radius <= size; radius++)
    {
        for (int i = -radius; i <= radius; i++)
        {
            int ii = centrex + i;

            for (int j = -radius; j <= radius; j++)
            {
                int jj = centrey + j;

                if (riftblob[ii][jj] == 0 && i * i + j * j < radius * radius + radius)
                    riftblob[ii][jj] = currentvalue;
            }
        }
        currentvalue = currentvalue - step;
    }
}

// This function returns a random number from a to b inclusive.

int random(int a, int b)
{
    int range = (b - a) + 1; // This is the range of possible numbers.

    int r = a + (fast_rand() % range);

    return r;
}

// This function randomises the sign of an integer (positive or negative).

int randomsign(int a)
{
    if (random(0, 1) == 1)
        a = 0 - a;

    return a;
}

// This does the same thing, for floats.

float randomsign(float a)
{
    if (random(0, 1) == 1)
        a = 0.0f - a;

    return a;
}

// These do the same thing but with the inbuilt randomiser.

int altrandom(int a, int b)
{
    int range = (b - a) + 1; // This is the range of possible numbers.

    int r = a + (rand() % range);

    return r;
}

int altrandomsign(int a)
{
    if (altrandom(0, 1) == 1)
        a = 0 - a;

    return (a);
}

// This function wraps a value between 0 and a maximum.

int wrap(int value, int max)
{
    max++;

    value = value % max;

    if (value < 0)
        value = max + value;

    return value;
}

// This function finds the average of two values, wrapped around a maximum.

int wrappedaverage(int x, int y, int max)
{
    int result = -1;

    int diffa = abs(x - y);

    int diffb = abs((x - max) - y);

    int diffc = abs(x - (y - max));

    if (diffa <= diffb && diffa <= diffc)
        result = (x + y) / 2;
    else
    {
        if (diffb <= diffa && diffb <= diffc)
            result = ((x - max) + y) / 2;
        else
            result = (x + (y - max)) / 2;
    }

    if (result < 0)
        result = result + max;

    if (result > max)
        result = result - max;

    return result;
}

// This normalises one value to another, so they are as close as possible, given the wrapped maximum.

int normalise(int x, int y, int max)
{
    while (abs(x - y) > abs(x - (y - max)))
        y = y - max;

    while (abs(x - y) > abs(x - (y + max)))
        y = y + max;

    return y;
}

// This function takes two values (a and b) and creates a new value that is tilted towards a by percentage %.

int tilt(int a, int b, int percentage)
{
    int neg = 0;
    int tilt = 0;

    float diff = (float)(a - b);

    if (diff < 1.0f)
    {
        diff = 0.0f - diff;
        neg = 1;
    }

    if (diff != 0.0f)
        tilt = (int)((diff / 100.0f) * (float)percentage);

    if (neg == 1)
        tilt = 0 - tilt;

    int newval = b + tilt;

    return newval ;
}

// This does the same thing, but for floats.

float tilt(float a, float b, int percentage)
{
    int neg = 0;
    float tilt = 0.0f;

    float diff = a - b;

    if (diff < 1.0f)
    {
        diff = 0.0f - diff;
        neg = 1;
    }

    if (diff != 0.0f)
        tilt = (diff / 100.0f) * (float)percentage;

    if (neg == 1)
        tilt = 0.0f - tilt;

    float newval = b + tilt;

    return newval;
}

// This function warps a vector by the specified amount, creating a more natural look.

void warp(vector<vector<int>>& map, int width, int height, int maxelev, int warpfactor, bool vary)
{
    vector<vector<int>> dest(width+1, vector<int>(height+1, 0));
    
    int grain = 8; // Level of detail on this fractal map.
    float valuemod = 0.2f;
    int v = random(3, 6);
    float valuemod2 = (float)v;

    vector<vector<int>> warpright(width + 1, vector<int>(height + 1, 0));
    vector<vector<int>> warpdown(width + 1, vector<int>(height + 1, 0));

    createfractal(warpright, width, height, grain, valuemod, valuemod2, 1, maxelev, 0, 0);
    createfractal(warpdown, width, height, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    // Now create another one to vary the strength of this effect (if needed).

    vector<vector<int>> broad(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

    grain = 4;
    valuemod = 0.002f;
    valuemod2 = 0.002f;

    if (vary)
        createfractal(broad, width, height, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            int x = i;
            int y = j;

            float thiswarpright = (float)warpright[i][j] / (float)maxelev;
            float thiswarpdown = (float)warpdown[i][j] / (float)maxelev;

            thiswarpright = thiswarpright - 0.5f; // So they're in the range -0.5 - 0.5.
            thiswarpdown = thiswarpdown - 0.5f;

            thiswarpright = thiswarpright * warpfactor;
            thiswarpdown = thiswarpdown * warpfactor;

            if (vary)
            {
                float varyfactor = (float)broad[i][j] / (float)maxelev;

                thiswarpright = thiswarpright * varyfactor;
                thiswarpdown = thiswarpdown * varyfactor;
            }

            x = x + (int)thiswarpright;

            if (x<0 || x>width)
                x = wrap(x, width);

            y = y + (int)thiswarpdown;

            if (y < 0)
                y = 0;

            if (y > height)
                y = height;

            int val = map[x][y];

            dest[i][j] = (int)val;
        }
    }

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            map[i][j] = dest[i][j];
        }
    }
}

// This does the same thing, but a "monstrous" warp - http://thingonitsown.blogspot.com/2018/11/monstrous-terrain.html

void monstrouswarp(vector<vector<int>>& map, int width, int height, int maxelev, int warpfactor)
{
    vector<vector<int>> dest(width + 1, vector<int>(height + 1, 0));

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            float point = (float)map[i][j];

            point = point / (float)maxelev; // Makes it into a value from 0.0 to 1.0

            point = point * (float)warpfactor;

            int val = map[(int)point][(int)point];
 
            dest[i][j] = val;
        }
    }

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            map[i][j] = dest[i][j];
        }
    }
}

// This function shifts everything in a vector to the left by a given number of pixels.

void shift(vector<vector<int>>& map, int width, int height, int offset)
{
    vector<vector<int>> dummy(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
            dummy[i][j] = map[i][j];
    }

    for (int i = 0; i <= width; i++)
    {
        int ii = i + offset;

        if (ii<0 || ii>width)
            ii = wrap(ii, width);

        for (int j = 0; j <= height; j++)
            map[i][j] = dummy[ii][j];
    }

    // For some reason this leaves a vertical line of noise, so we remove that manually now.

    int i = width - offset; // This is the x-coordinate of the problematic line.
    int iminus = i - 1;
    int iplus = i + 1;

    if (iminus < 0)
        iminus = width;

    if (iplus > width)
        iplus = 0;

    for (int j = 0; j <= height; j++) // Make the line the average of its neighbours to left and right.
    {
        map[i][j] = (map[iminus][j] + map[iplus][j]) / 2;
    }
}

// This function flips an array, vertically/horizontally.

void flip(vector<vector<int>>& arr, int awidth, int aheight, bool vert, bool horiz)
{
    vector<vector<int>> dummy(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

    for (int i = 0; i <= awidth; i++)
    {
        for (int j = 0; j <= aheight; j++)
            dummy[i][j] = arr[i][j];
    }

    for (int i = 0; i <= awidth; i++)
    {
        for (int j = 0; j <= aheight; j++)
        {
            int ii, jj;

            if (horiz == 0)
                ii = 1;
            else
                ii = awidth - i;

            if (vert == 0)
                jj = j;
            else
                jj = aheight - j;

            arr[i][j] = dummy[ii][jj];
        }
    }
}

// This function smoothes a vector.

void smooth(vector<vector<int>>& arr, int width, int height, int maxelev, int amount, bool vary)
{
    int varyx = width / 2;
    int varyy = random(1, height - 1);

    for (int i = 0; i <= width; i++)
    {
        for (int j = 1; j < height; j++)
        {
            bool goaheadx = 0;
            bool goaheady = 0;

            if (vary == 1) // This will vary the amount of smoothing over the map, to make it more interesting.
            {
                if (i < varyx)
                {
                    if (i > 10 && random(0, varyx) < i * 2)
                        goaheadx = 1;
                }
                else
                {
                    if (i<width - 10 && random(varyx, width)>i / 2)
                        goaheadx = 1;
                }

                if (j < varyy)
                {
                    if (random(0, varyy) < j * 2)
                        goaheady = 1;
                }
                else
                {
                    if (random(varyy, height) > j / 2)
                        goaheady = 1;
                }
                if (abs(varyx - i) < 100) // Force it to smooth in the centre of the map.
                {
                    //goaheadx=1;
                    //goaheady=1;
                }
            }
            else
            {
                goaheadx = 1;
                goaheady = 1;
            }

            if (goaheadx == 1 && goaheady == 1)
            {
                float crount = 0.0f;
                float ave = 0.0f;

                for (int k = i - amount; k <= i + amount; k++)
                {
                    int kk = k;

                    if (kk < 0)
                        kk = width;

                    if (kk > width)
                        kk = 0;

                    for (int l = j - amount; l <= j + amount; l++)
                    {
                        //ave=ave+nomwrap(k,l);
                        ave = ave + (float)arr[kk][l];
                        crount++;
                    }
                }

                ave = ave / crount;

                if (ave > 0.0f && ave < (float)maxelev)
                    arr[i][j] = (int)ave;
            }
        }
    }
}

// This function sees whether the given point on the given vector is on the edge of the filled area.
// Note, this wraps around the edges.

bool edge(vector<vector<bool>>& arr, int width, int height, int i, int j)
{
    if (arr[i][j] == 0)
        return 0;

    for (int k = i - 1; k <= i + 1; k++)
    {
        int kk = k;

        if (kk<0 || kk>width)
            kk = wrap(kk, width);

        for (int l = j - 1; l <= j + 1; l++)
        {
            if (l >= 0 && l <= height)
            {
                if (arr[kk][l] == 0)
                    return 1;
            }
        }
    }

    return 0;
}

// This draws a straight line on the supplied vector.
// Uses Bresenham's line algorithm - http://rosettacode.org/wiki/Bitmap/Bresenham%27s_line_algorithm#C.2B.2B

void drawline(vector<vector<bool>>& arr, int x1, int y1, int x2, int y2)
{
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

    const float dx = (float)(x2 - x1);
    const float dy = (float)fabs(y2 - y1);

    float error = dx / 2.0f;
    const int ystep = (y1 < y2) ? 1 : -1;
    int y = (int)y1;

    const int maxX = (int)x2;

    for (int x = (int)x1; x <= maxX; x++)
    {
        if (steep)
            arr[y][x] = 1;
        else
            arr[x][y] = 1;

        error -= dy;
        if (error < 0)
        {
            y += ystep;
            error += dx;
        }
    }
}

// This does the same thing, but allows lines to pass between the left and right sides.

void drawlinewrapped(vector<vector<bool>>& arr, int width, int height, int x1, int y1, int x2, int y2)
{
    if (x2 > x1)
    {
        int dist = x2 - x1;

        int altdist = (x1 + width) - x2;

        if (altdist < dist)
            x1 = x1 + width;
    }
    else
    {
        int dist = x1 - x2;

        int altdist = (x2 + width) - x1;

        if (altdist < dist)
            x2 = x2 + width;
    }
    
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

    const float dx = (float)(x2 - x1);
    const float dy = (float)fabs(y2 - y1);

    float error = dx / 2.0f;
    const int ystep = (y1 < y2) ? 1 : -1;
    int y = (int)y1;

    const int maxX = (int)x2;

    for (int x = (int)x1; x <= maxX; x++)
    {
        int a = x;
        int b = y;

        if (steep)
        {
            a = y;
            b = x;
        }

        if (a < 0)
            a = a + width;

        if (a > width)
            a = a - width;

        if (b < 0)
            b = b + width;

        if (b > width)
            b = b - width;
        
        arr[a][b] = 1;

        error -= dy;
        if (error < 0)
        {
            y += ystep;
            error += dx;
        }
    }
}

// This function draws a filled circle on the specified vector.
// Uses code by OBese87 and Libervurto - https://forum.thegamecreators.com/thread/197341

void drawcircle(vector<vector<int>>& arr, int x, int y, int col, int radius)
{
    float dots = radius * 6.28f;
    float t = 360.0f / dots;
    int quarter = (int)(dots / 4.0f);

    for (int i = 1; i <= quarter; i++)
    {
        int u = (int)((float)sin(i * t) * (float)radius);
        int v = (int)((float)cos(i * t) * (float)radius);

        box(arr, x - u, y - v, x + u, y + v, col);
    }
}

// This function draws a filled box on the specified vector.

void box(vector<vector<int>>& arr, int x1, int y1, int x2, int y2, int col)
{
    for (int i = x1; i <= x2; i++)
    {
        int ii = i;

        if (ii<0 || i>ARRAYWIDTH - 1)
            ii = wrap(ii, ARRAYWIDTH - 1);

        for (int j = y1; j <= y2; j++)
        {
            if (j >= 0 && j < ARRAYHEIGHT)
                arr[ii][j] = col;
        }
    }
}

// As above, but on a three-dimensional vector.

void drawcircle3d(vector<vector<vector<int>>>& arr, int x, int y, int col, int radius, int index)
{
    float dots = radius * 6.28f;
    float t = 360.0f / dots;
    int quarter = (int)(dots / 4.0f);

    for (int i = 1; i <= quarter; i++)
    {
        int u = (int)((float)sin(i * t) * (float)radius);
        int v = (int)((float)cos(i * t) * (float)radius);

        box3d(arr, x - u, y - v, x + u, y + v, col, index);
    }
}

// Again, on a three-dimensional vector.

void box3d(vector<vector<vector<int>>>& arr, int x1, int y1, int x2, int y2, int col, int index)
{
    for (int i = x1; i <= x2; i++)
    {
        for (int j = y1; j <= y2; j++)
        {
            if (i >= 0 && i <= 20 && j >= 0 && j <= 20)
                arr[index][i][j] = col;
        }
    }
}

// This function draws splines. Based on functions by Markus - https://forum.thegamecreators.com/thread/202580
// (This is easily the most important bit of borrowed code in this project. Thank you Markus!)

twofloats curvepos(twofloats p0, twofloats p1, twofloats p2, twofloats p3, float t)
{
    twofloats pt;

    pt.x = 0.5f * ((2.0f * p1.x) + (-p0.x + p2.x) * t + (2.0f * p0.x - 5.0f * p1.x + 4.0f * p2.x - p3.x) * (t * t) + (-p0.x + 3.0f * p1.x - 3.0f * p2.x + p3.x) * (t * t * t)); 
    pt.y = 0.5f * ((2.0f * p1.y) + (-p0.y + p2.y) * t + (2.0f * p0.y - 5.0f * p1.y + 4.0f * p2.y - p3.y) * (t * t) + (-p0.y + 3.0f * p1.y - 3.0f * p2.y + p3.y) * (t * t * t));

    return (pt);
}

// This function does a flood fill on a bool vector.

void fill(vector<vector<bool>>& arr, int width, int height, int startx, int starty, int replacement)
{
    if (startx < 0 || startx >= width || starty < 0 || starty >= height)
        return;

    if (arr[startx][starty] == replacement)
        return;

    int row[] = { -1,0,0,1 };
    int col[] = { 0,-1,1,0 };

    bool target = arr[startx][starty]; // This is the "colour" that we're going to change into the replacement "colour".

    twointegers node;

    node.x = startx;
    node.y = starty;

    queue<twointegers> q; // Create a queue
    q.push(node); // Put our starting node into the queue

    while (q.empty() != 1) // Keep going while there's anything left in the queue
    {
        node = q.front(); // Take the node at the front of the queue
        q.pop(); // And then pop it out of the queue

        if (node.x >= 0 && node.x < width && node.y >= 0 && node.y < height && arr[node.x][node.y] == target)
        {
            arr[node.x][node.y] = replacement;

            for (int k = 0; k < 4; k++) // Look at the four neighbouring nodes in turn
            {
                twointegers nextnode;

                nextnode.x = node.x + row[k];
                nextnode.y = node.y + col[k];

                if (nextnode.x >= 0 && nextnode.x < width && nextnode.y >= 0 && nextnode.y < height)
                {
                    if (arr[nextnode.x][nextnode.y] == target) // If this node is the right "colour"
                        q.push(nextnode); // Put that node onto the queue
                }
            }
        }
    }
}

// This function does the same thing, but with a continental mask to avoid going into the sea.

void fillcontinent(vector<vector<bool>>& arr, vector<vector<short>>& mask, short maskcheck, int width, int height, int startx, int starty, int replacement)
{
    if (startx < 0 || startx >= width || starty < 0 || starty >= height)
        return;

    if (arr[startx][starty] == replacement)
        return;

    if (mask[startx][starty] != maskcheck)
        return;

    int row[] = { -1,0,0,1 };
    int col[] = { 0,-1,1,0 };

    bool target = arr[startx][starty]; // This is the "colour" that we're going to change into the replacement "colour".

    twointegers node;

    node.x = startx;
    node.y = starty;

    queue<twointegers> q; // Create a queue
    q.push(node); // Put our starting node into the queue

    while (q.empty() != 1) // Keep going while there's anything left in the queue
    {
        node = q.front(); // Take the node at the front of the queue
        q.pop(); // And then pop it out of the queue

        if (node.x >= 0 && node.x < width && node.y >= 0 && node.y < height && arr[node.x][node.y] == target)
        {
            arr[node.x][node.y] = replacement;

            for (int k = 0; k < 4; k++) // Look at the four neighbouring nodes in turn
            {
                twointegers nextnode;

                nextnode.x = node.x + row[k];
                nextnode.y = node.y + col[k];

                if (nextnode.x >= 0 && nextnode.x < width && nextnode.y >= 0 && nextnode.y < height)
                {
                    if (arr[nextnode.x][nextnode.y] == target && mask[nextnode.x][nextnode.y] == maskcheck) // If this node is the right "colour"
                        q.push(nextnode); // Put that node onto the queue
                }
            }
        }
    }
}

// This function adds the elevation element to a temperature.

int tempelevadd(planet& world, int temp, int i, int j)
{
    int elevation = world.map(i, j) - world.sealevel();

    if (elevation < 0)
        elevation = 0;

    float elevationadjust = (float)elevation;
    elevationadjust = elevationadjust / 1000.0f;
    elevationadjust = elevationadjust * world.tempdecrease();

    temp = temp - (int)elevationadjust;

    return temp;
}

// Same thing, but at the regional level.

int tempelevadd(planet& world, region& region, int temp, int i, int j)
{
    int elevation = region.map(i, j) - region.sealevel();

    if (elevation < 0)
        elevation = 0;

    float elevationadjust = (float)elevation;
    elevationadjust = elevationadjust / 1000.0f;
    elevationadjust = elevationadjust * world.tempdecrease();

    temp = temp - (int)elevationadjust;

    return temp;
}

// This function removes the elevation element of a temperature.

int tempelevremove(planet& world, int temp, int i, int j)
{
    int elevation = world.map(i, j) - world.sealevel();

    if (elevation < 0)
        elevation = 0;

    float elevationadjust = (float)elevation;
    elevationadjust = elevationadjust / 1000.0f;
    elevationadjust = elevationadjust * world.tempdecrease();

    temp = temp + (int)elevationadjust;

    return temp;
}

// Same thing, but at the regional level.

int tempelevremove(planet& world, region& region, int temp, int i, int j)
{
    int elevation = region.map(i, j) - region.sealevel();

    if (elevation < 0)
        elevation = 0;

    float elevationadjust = (float)elevation;
    elevationadjust = elevationadjust / 1000.0f;
    elevationadjust = elevationadjust * world.tempdecrease();

    temp = temp + (int)elevationadjust;

    return temp;
}

// This function translates a direction from an integer into a text string.

string getdirstring(int dir)
{
    string direction;

    switch (dir)
    {
    case 1:
        direction = "north";
        break;

    case 2:
        direction = "northeast";
        break;

    case 3:
        direction = "east";
        break;

    case 4:
        direction = "southeast";
        break;

    case 5:
        direction = "south";
        break;

    case 6:
        direction = "southwest";
        break;

    case 7:
        direction = "west";
        break;

    case 8:
        direction = "northwest";
        break;
    }

    return (direction);
}

// This function returns the direction from one tile to another.

int getdir(int x, int y, int xx, int yy)
{
    if (xx == x && yy < y)
        return (1);

    if (xx > x && yy < y)
        return (2);

    if (xx > x && yy == y)
        return (3);

    if (xx > x && yy > y)
        return (4);

    if (xx == x && yy > y)
        return (5);

    if (xx<x && yy>y)
        return (6);

    if (xx < x && yy == y)
        return (7);

    if (xx < x && yy < y)
        return (8);

    return (0);
}

// This function gets the destination from a tile and a direction.

twointegers getdestination(int x, int y, int dir)
{
    if (dir == 8 || dir == 1 || dir == 2)
        y--;

    if (dir == 4 || dir == 5 || dir == 6)
        y++;

    if (dir == 2 || dir == 3 || dir == 4)
        x++;

    if (dir == 6 || dir == 7 || dir == 8)
        x--;

    twointegers dest;

    dest.x = x;
    dest.y = y;

    return dest;
}

// This function finds the direction of the lowest neighbouring tile.

int findlowestdir(planet& world, int neighbours[8][2], int x, int y)
{
    int width = world.width();
    int height = world.height();

    int lowest = world.maxelevation() * 2;
    int nnn = -1;

    int start = random(0, 7);

    for (int n = start; n <= start + 7; n++)
    {
        int nn = wrap(n, 7);

        int i = x + neighbours[nn][0];

        if (i<0 || i>width)
            i = wrap(i, width);

        int j = y + neighbours[nn][1];

        if (j >= 0 && j <= height)
        {
            int pointelev = world.nom(i, j);

            if (pointelev < lowest)
            {
                lowest = pointelev;
                nnn = nn;
            }
        }
    }

    int dir = nnn + 1;

    return (dir);
}

// Same thing, but for river directions.

int findlowestdirriver(planet& world, int neighbours[8][2], int x, int y, vector<vector<int>>& mountaindrainage)
{
    int width = world.width();
    int height = world.height();

    int lowest = world.maxelevation() * 2;
    int nnn = -1;

    bool found = 0;

    int start = random(0, 7);

    if (1 == 0)//random(1,3)!=1)
    {
        for (int n = start; n <= start + 7; n++)
        {
            int nn = wrap(n, 7);

            int i = x + neighbours[nn][0];

            if (i<0 || i>width)
                i = wrap(i, width);

            int j = y + neighbours[nn][1];

            if (j >= 0 && j <= height)
            {
                int pointelev = mountaindrainage[i][j];

                if (pointelev < lowest && world.nom(i, j) < world.nom(x, y))
                {
                    lowest = pointelev;
                    nnn = nn;
                    found = 1;
                }
            }
        }
    }

    if (found == 0) // Couldn't find one just from the mountain drainage.
    {
        lowest = world.maxelevation() * 2;
        nnn = -1;

        start = random(0, 7);

        for (int n = start; n <= start + 7; n++)
        {
            int nn = wrap(n, 7);

            int i = x + neighbours[nn][0];

            if (i<0 || i>width)
                i = wrap(i, width);

            int j = y + neighbours[nn][1];

            if (j >= 0 && j <= height)
            {
                int pointelev = world.nom(i, j);

                if (pointelev < lowest)
                {
                    lowest = pointelev;
                    nnn = nn;
                }
            }
        }
    }

    int dir = nnn + 1;

    return (dir);
}

// This gets the amount the land is sloping between two points.

int getslope(planet& world, int x, int y, int xx, int yy)
{
    int width = world.width();
    int height = world.height();

    if (x<0 || x>width)
        x = wrap(x, width);

    if (xx<0 || xx>width)
        xx = wrap(xx, width);

    if (y<0 || y>height)
        return (-1);

    if (yy<0 || yy>height)
        return (-1);

    int slope = world.map(xx, yy) - world.map(x, y);

    return (slope);
}

// Same thing on the regional map.

int getslope(region& region, int x, int y, int xx, int yy)
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    if (x<0 || x>rwidth)
        return (-1);

    if (xx<0 || xx>rwidth)
        return (-1);

    if (y<0 || y>rheight)
        return (-1);

    if (yy<0 || yy>rheight)
        return (-1);

    int slope = region.map(xx, yy) - region.map(x, y);

    return (slope);
}

// This function checks how flat the land is at a given point.

int getflatness(planet& world, int x, int y)
{
    int width = world.width();
    int height = world.height();

    int xminus = x - 1;

    if (xminus < 0)
        xminus = width;

    int xplus = x + 1;

    if (xplus > width)
        xplus = 0;

    int thiselev = getflatelevation(world, x, y);

    float flatness = 0.0f;

    int thatelev = getflatelevation(world, xminus, y);
    flatness = flatness + (float)abs(thiselev - thatelev);

    float crount = 2.0f;

    if (y > 0)
    {
        thatelev = getflatelevation(world, xminus, y - 1);
        flatness = flatness + (float)abs(thiselev - thatelev);

        thatelev = getflatelevation(world, x, y - 1);
        flatness = flatness + (float)abs(thiselev - thatelev);

        thatelev = getflatelevation(world, xplus, y - 1);
        flatness = flatness + (float)abs(thiselev - thatelev);

        crount = crount + 3.0f;
    }

    if (y < height)
    {
        thatelev = getflatelevation(world, xminus, y + 1);
        flatness = flatness + (float)abs(thiselev - thatelev);

        thatelev = getflatelevation(world, x, y + 1);
        flatness = flatness + (float)abs(thiselev - thatelev);

        thatelev = getflatelevation(world, xplus, y + 1);
        flatness = flatness + (float)abs(thiselev - thatelev);

        crount = crount + 3.0f;
    }

    flatness = flatness / crount;

    return (int)flatness;
}

// This function gets the elevation of a point, incorporating water level.

int getflatelevation(planet& world, int x, int y)
{
    int elev;

    int surface = world.lakesurface(x, y);

    if (surface > 0)
        elev = surface;
    else
    {
        if (world.sea(x, y) == 1)
            elev = world.sealevel();
        else
            elev = world.nom(x, y);
    }

    return (elev);
}

// This finds the distance to the nearest land in the direction of the equator.

int landdistance(planet& world, int x, int y)
{
    int width = world.width();
    int height = world.height();
    int equator = height / 2;

    if (x<0 || x>width || y<0 || y>height)
        return (-1);

    int dist = 0;
    int pets = -1;

    int target = equator;

    if (y < equator)
        pets = 1;

    // Reverse the direction of search if the equator is colder than the poles!

    if (y < equator) // Northern hemisphere
    {
        if (world.northpolartemp()>world.eqtemp())
        {
            pets = -1;
            target = 0;
        }
    }
    else // Southern hemisphere
    {
        if (world.southpolartemp() > world.eqtemp())
        {
            pets = 1;
            target = height;
        }
    }

    for (int j = y; j != target; j = j + pets)
    {
        if (world.sea(x, j) == 0)
        {
            if (world.sea(x, j + pets) == 0 && world.sea(x, j + pets * 2) == 0) // To avoid little islands showing up
            {
                int xleft = x - 1;

                if (xleft == -1)
                    xleft = width;

                int xright = x + 1;
                if (xright == width + 1)
                    xright = 0;

                if (world.sea(xleft, j + pets) == 0 && world.sea(xright, j + pets) == 0)
                {
                    return (dist);
                }
            }
        }

        dist++;
    }

    dist = -1; // No land found at all.

    return(dist);
}

// This finds the coordinates of the nearest sea (or land, if land=1) to the point specified. Note that it does not wrap the x coordinate, i.e. it could return one lower than 0 or higher than width.

twointegers nearestsea(planet& world, int i, int j, bool land, int limit, int grain)
{
    int width = world.width();
    int height = world.height();

    twointegers seapoint;

    seapoint.x = -1;
    seapoint.y = -1;

    int distance = -1; // Distance to nearest sea.
    int seax = -1; // x coordinate of nearest sea.
    int seay = -1; // y coordinate of nearest sea.
    int checkdist = 1; // Distance that we're checking to see if it contains sea.

    while (distance == -1)
    {
        checkdist = checkdist + grain; // Increase the size of circle that we're checking

        int rr = checkdist * checkdist;

        for (int x = i - checkdist; x <= i + checkdist; x++)
        {
            int xx = x;

            if (xx<0 || xx>width)
                xx = wrap(xx, width);

            for (int y = j - checkdist; y <= j + checkdist; y++)
            {
                if (y >= 0 && y <= height)
                {
                    if ((x-i)*(x-i)+(y-j)*(y-j)==rr) // if this point is on the circle we're looking at
                    {
                        if (world.sea(xx, y) == 1 && land == 0) // And if this point is sea
                        {
                            distance = checkdist;
                            seax = x;
                            seay = y;
                        }

                        if (world.sea(xx, y) == 0 && land == 1) // And if this point is land
                        {
                            distance = checkdist;
                            seax = x;
                            seay = y;
                        }
                    }
                }
            }
        }
        if (checkdist > limit) // We are surely not going to find any now...
        {
            distance = width * 2;
            seax = -1;
            seay = -1;
        }
    }

    seapoint.x = seax;
    seapoint.y = seay;

    return (seapoint);
}

// This does the same thing, but for the regional map.

twointegers nearestsea(region& region, int leftx, int lefty, int rightx, int righty, int i, int j)
{
    twointegers seapoint;

    seapoint.x = -1;
    seapoint.y = -1;

    int distance = -1; // Distance to nearest sea.
    int seax = -1; // x coordinate of nearest sea.
    int seay = -1; // y coordinate of nearest sea.
    int checkdist = 1; // Distance that we're checking to see if it contains sea.

    while (distance == -1)
    {
        checkdist++; // Increase the size of circle that we're checking

        int rr = checkdist * checkdist;

        for (int x = i - checkdist; x <= i + checkdist; x++)
        {
            for (int y = j - checkdist; y <= j + checkdist; y++)
            {
                if ((x - i) * (x - i) + (y - j) * (y - j) == rr) // if this point is on the circle we're looking at
                {
                    if (x >= leftx && x <= rightx && y >= lefty && y <= righty)
                    {
                        if (region.sea(x, y) == 1) // And if this point is sea
                        {
                            distance = checkdist;
                            seax = x;
                            seay = y;
                        }
                        else
                        {
                            if (region.lakesurface(x, y) != 0) // Lake counts as sea for our purposes.
                            {
                                distance = checkdist;
                                seax = x;
                                seay = y;
                            }
                        }
                    }
                }
            }
        }
        if (checkdist > 100) // We are surely not going to find any now...
        {
            distance = region.rwidth() * 2;
            seax = -1;
            seay = -1;
        }
    }

    seapoint.x = seax;
    seapoint.y = seay;

    return seapoint;
}

// This function checks whether a global tile is either sea next to land or land next to sea.

bool vaguelycoastal(planet& world, int x, int y)
{
    int width = world.width();
    int height = world.height();

    bool sea = 0;

    if (world.sea(x, y) == 1)
        sea = 1;

    bool found = 0;

    for (int i = x - 1; i <= x + 1; i++)
    {
        int ii = i;

        if (ii<0 || ii>width)
            ii = wrap(ii, width);

        for (int j = y - 1; j <= y + 1; j++)
        {
            if (j >= 0 && j <= height)
            {
                if (sea == 0 && world.sea(ii, j) == 1)
                {
                    found = 1;
                    i = x + 1;
                    j = y + 1;
                }

                if (sea == 1 && world.sea(ii, j) == 0)
                {
                    found = 1;
                    i = x + 1;
                    j = y + 1;
                }
            }
        }
    }

    return found;
}

// Same thing, for a regional cell.

bool vaguelycoastal(region& region, int x, int y)
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    bool sea = 0;

    if (region.sea(x, y) == 1)
        sea = 1;

    bool found = 0;

    for (int i = x - 1; i <= x + 1; i++)
    {
        for (int j = y - 1; j <= y + 1; j++)
        {
            if (i >= 0 && i <= rwidth && j >= 0 && j <= rheight)
            {
                if (sea == 0 && region.sea(i, j) == 1)
                {
                    found = 1;
                    i = x + 1;
                    j = y + 1;
                }

                if (sea == 1 && region.sea(i, j) == 0)
                {
                    found = 1;
                    i = x + 1;
                    j = y + 1;
                }
            }
        }
    }

    return found;
}

// This function checks to see if a global tile is land, bordered by sea to the east, south, and southeast.

bool northwestlandonly(planet& world, int x, int y)
{
    int width = world.width();
    int height = world.height();

    if (world.sea(x, y) == 1)
        return 0;

    int xx = x + 1;

    if (xx > width)
        xx = 0;

    int yy = y + 1;

    if (yy > height)
        return 0;

    if (world.sea(xx, y) == 0)
        return 0;

    if (world.sea(x, yy) == 0)
        return 0;

    if (world.sea(xx, yy) == 0)
        return 0;

    xx = x - 1;

    if (xx < 0)
        x = width;

    yy = y - 1;

    if (yy < 0)
        return 0;

    if (world.sea(xx, yy) == 1)
        return 0;

    return 1;
}

// Same thing, at the regional level.

bool northwestlandonly(region& region, int x, int y)
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    if (region.sea(x, y) == 1)
        return 0;

    int xx = x + 1;

    if (xx > rwidth)
        return 0;

    int yy = y + 1;

    if (yy > rheight)
        return 0;

    if (region.sea(xx, y) == 0)
        return 0;

    if (region.sea(x, yy) == 0)
        return 0;

    if (region.sea(xx, yy) == 0)
        return 0;

    xx = x - 1;

    if (xx < 0)
        return 0;

    yy = y - 1;

    if (yy < 0)
        return 0;

    if (region.sea(xx, yy) == 1)
        return 0;

    return 1;
}

// Same again, but for lakes rather than sea.

bool lakenorthwestlandonly(region& region, int x, int y)
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    if (region.lakesurface(x, y) != 0)
        return 0;

    int xx = x + 1;

    if (xx > rwidth)
        return 0;

    int yy = y + 1;

    if (yy > rheight)
        return 0;

    if (region.lakesurface(xx, y) == 0)
        return 0;

    if (region.lakesurface(x, yy) == 0)
        return 0;

    if (region.lakesurface(xx, yy) == 0)
        return 0;

    xx = x - 1;

    if (xx < 0)
        return 0;

    yy = y - 1;

    if (yy < 0)
        return 0;

    if (region.lakesurface(xx, yy) != 0)
        return 0;

    return 1;
}

// This function checks to see if a global tile is land, bordered by sea to the west, south, and southwest.

bool northeastlandonly(planet& world, int x, int y)
{
    int width = world.width();
    int height = world.height();

    if (world.sea(x, y) == 1)
        return 0;

    int xx = x - 1;

    if (xx < 0)
        xx = width;

    int yy = y + 1;

    if (yy > height)
        return 0;

    if (world.sea(xx, y) == 0)
        return 0;

    if (world.sea(x, yy) == 0)
        return 0;

    if (world.sea(xx, yy) == 0)
        return 0;

    xx = x + 1;

    if (xx > width)
        xx = 0;

    yy = y - 1;

    if (yy < 0)
        return 0;

    if (world.sea(xx, yy) == 1)
        return 0;

    return 1;
}

// Same thing, at the regional level.

bool northeastlandonly(region& region, int x, int y)
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    if (region.sea(x, y) == 1)
        return 0;

    int xx = x - 1;

    if (xx < 0)
        return 0;

    int yy = y + 1;

    if (yy > rheight)
        return 0;

    if (region.sea(xx, y) == 0)
        return 0;

    if (region.sea(x, yy) == 0)
        return 0;

    if (region.sea(xx, yy) == 0)
        return 0;

    xx = x + 1;

    if (xx > rwidth)
        return 0;

    yy = y - 1;

    if (yy < 0)
        return 0;

    if (region.sea(xx, yy) == 1)
        return 0;

    return 1;
}

// Same again, but for lakes rather than sea.

bool lakenortheastlandonly(region& region, int x, int y)
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    if (region.lakesurface(x, y) != 0)
        return 0;

    int xx = x - 1;

    if (xx < 0)
        return 0;

    int yy = y + 1;

    if (yy > rheight)
        return 0;

    if (region.lakesurface(xx, y) == 0)
        return 0;

    if (region.lakesurface(x, yy) == 0)
        return 0;

    if (region.lakesurface(xx, yy) == 0)
        return 0;

    xx = x + 1;

    if (xx > rwidth)
        return 0;

    yy = y - 1;

    if (yy < 0)
        return 0;

    if (region.lakesurface(xx, yy) != 0)
        return 0;

    return 1;
}

// This function checks to see if a global tile is land, bordered by sea to the east, north, and northeast.

bool southwestlandonly(planet& world, int x, int y)
{
    int width = world.width();
    int height = world.height();

    if (world.sea(x, y) == 1)
        return 0;

    int xx = x + 1;

    if (xx > width)
        xx = 0;

    int yy = y - 1;

    if (yy < 0)
        return 0;

    if (world.sea(xx, y) == 0)
        return 0;

    if (world.sea(x, yy) == 0)
        return 0;

    if (world.sea(xx, yy) == 0)
        return 0;

    xx = x + 1;

    if (xx > width)
        x = 0;

    yy = y + 1;

    if (yy > height)
        return 0;

    if (world.sea(xx, yy) == 1)
        return 0;

    return 1;
}

// Same thing, at the regional level.

bool southwestlandonly(region& region, int x, int y)
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    if (region.sea(x, y) == 1)
        return 0;

    int xx = x + 1;

    if (xx > rwidth)
        return 0;

    int yy = y - 1;

    if (yy < 0)
        return 0;

    if (region.sea(xx, y) == 0)
        return 0;

    if (region.sea(x, yy) == 0)
        return 0;

    if (region.sea(xx, yy) == 0)
        return 0;

    xx = x + 1;

    if (xx > rwidth)
        return 0;

    yy = y + 1;

    if (yy > rheight)
        return 0;

    if (region.sea(xx, yy) == 1)
        return 0;

    return 1;
}

// Same thing, but for lakes rather than sea.

bool lakesouthwestlandonly(region& region, int x, int y)
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    if (region.lakesurface(x, y) != 0)
        return 0;

    int xx = x + 1;

    if (xx > rwidth)
        return 0;

    int yy = y - 1;

    if (yy < 0)
        return 0;

    if (region.lakesurface(xx, y) == 0)
        return 0;

    if (region.lakesurface(x, yy) == 0)
        return 0;

    if (region.lakesurface(xx, yy) == 0)
        return 0;

    xx = x + 1;

    if (xx > rwidth)
        return 0;

    yy = y + 1;

    if (yy > rheight)
        return 0;

    if (region.lakesurface(xx, yy) != 0)
        return 0;

    return 1;
}

// This function checks to see if a global tile is land, bordered by sea to the west, north, and northwest.

bool southeastlandonly(planet& world, int x, int y)
{
    int width = world.width();
    int height = world.height();

    if (world.sea(x, y) == 1)
        return 0;

    int xx = x - 1;

    if (xx < 0)
        xx = width;

    int yy = y - 1;

    if (yy < 0)
        return 0;

    if (world.sea(xx, y) == 0)
        return 0;

    if (world.sea(x, yy) == 0)
        return 0;

    if (world.sea(xx, yy) == 0)
        return 0;

    xx = x + 1;

    if (xx > width)
        xx = 0;

    yy = y + 1;

    if (yy > height)
        return 0;

    if (world.sea(xx, yy) == 1)
        return 0;

    return 1;
}

// Same thing, at the regional level.

bool southeastlandonly(region& region, int x, int y)
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    if (region.sea(x, y) == 1)
        return 0;

    int xx = x - 1;

    if (xx < 0)
        return 0;

    int yy = y - 1;

    if (yy < 0)
        return 0;

    if (region.sea(xx, y) == 0)
        return 0;

    if (region.sea(x, yy) == 0)
        return 0;

    if (region.sea(xx, yy) == 0)
        return 0;

    xx = x + 1;

    if (xx > rwidth)
        return 0;

    yy = y + 1;

    if (yy > rheight)
        return 0;

    if (region.sea(xx, yy) == 1)
        return 0;

    return 1;
}

// Same thing, but for lakes rather than sea..

bool lakesoutheastlandonly(region& region, int x, int y)
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    if (region.lakesurface(x, y) != 0)
        return 0;

    int xx = x - 1;

    if (xx < 0)
        return 0;

    int yy = y - 1;

    if (yy < 0)
        return 0;

    if (region.lakesurface(xx, y) == 0)
        return 0;

    if (region.lakesurface(x, yy) == 0)
        return 0;

    if (region.lakesurface(xx, yy) == 0)
        return 0;

    xx = x + 1;

    if (xx > rwidth)
        return 0;

    yy = y + 1;

    if (yy > rheight)
        return 0;

    if (region.lakesurface(xx, yy) != 0)
        return 0;

    return 1;
}

// This function finds a neighbouring sea tile that isn't the one we just came from.

twointegers findseatile(planet& world, int x, int y, int dir)
{
    int width = world.width();
    int height = world.height();

    twointegers newtile;

    newtile.x = -1;
    newtile.y = -1;

    int oldx = x;
    int oldy = y;

    if (dir == 8 || dir == 1 || dir == 2)
        oldy = y + 1;

    if (dir == 4 || dir == 5 || dir == 6)
        oldy = y - 1;

    if (dir == 2 || dir == 3 || dir == 4)
        oldx = x - 1;

    if (dir == 6 || dir == 7 || dir == 8)
        oldx = x + 1;

    if (oldx<0 || oldx>width)
        oldx = wrap(oldx, width);

    // oldx and oldy are the coordinates of where we are coming to this tile from. We want to avoid going back there!

    int lowest = world.map(x, y);

    for (int i = x - 1; i <= x + 1; i++)
    {
        int ii = i;

        if (ii<0 || ii>width)
            ii = wrap(ii, width);

        for (int j = y - 1; j <= y + 1; j++)
        {
            if (ii != x && j != y && ii != oldx && j != oldy && j >= 0 && j <= height && world.sea(ii, j) == 1 && world.map(ii, j) < lowest)
            {
                newtile.x = ii;
                newtile.y = j;
                lowest = world.map(ii, j);
            }
        }
    }

    if (newtile.x != -1)
        return (newtile);

    // If it didn't find a neighbouring sea tile that's lower, just find one at all.

    for (int i = x - 1; i <= x + 1; i++)
    {
        int ii = i;

        if (ii<0 || ii>width)
            ii = wrap(ii, width);

        for (int j = y - 1; j <= y + 1; j++)
        {
            if (ii != x && j != y && ii != oldx && j != oldy && j >= 0 && j <= height && world.sea(ii, j) == 1)
            {
                newtile.x = ii;
                newtile.y = j;
            }
        }
    }
    return (newtile);
}

// This returns the coordinate that the river in the given tile is flowing into.

twointegers getflowdestination(planet& world, int x, int y, int dir)
{
    int width = world.width();
    int height = world.height();

    twointegers destpoint;

    if (dir == 0)
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

    destpoint.x = x;
    destpoint.y = y;

    return (destpoint);
}

// The same thing, but for the regional map.

twointegers getregionalflowdestination(region& region, int x, int y, int dir)
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    twointegers destpoint;

    if (dir == 0)
        dir = region.riverdir(x, y);

    if (dir == 8 || dir == 1 || dir == 2)
        y--;

    if (dir == 4 || dir == 5 || dir == 6)
        y++;

    if (dir == 2 || dir == 3 || dir == 4)
        x++;

    if (dir == 6 || dir == 7 || dir == 8)
        x--;

    if (x<0 || x>rwidth || y<0 || y>rheight)
    {
        destpoint.x = -1;
        destpoint.y = -1;
    }
    else
    {
        destpoint.x = x;
        destpoint.y = y;
    }

    return destpoint;
}

// This function finds the cell with the largest flow into the current one.

twointegers getupstreamcell(planet& world, int x, int y)
{
    int width = world.width();
    int height = world.height();

    twointegers upcell, destpoint;

    upcell.x = -1;
    upcell.y = -1;

    int largest = 0;

    int riverjan, riverjul;

    for (int i = x - 1; i <= x + 1; i++)
    {
        int ii = i;

        if (ii<0 || ii>width)
            ii = wrap(ii, width);

        for (int j = y - 1; j <= y + 1; j++)
        {
            if (j >= 0 && y <= height)
            {
                riverjan = world.riverjan(ii, j);
                riverjul = world.riverjul(ii, j);

                if (riverjan + riverjul > largest) // If this is larger than the current largest inflow
                {
                    destpoint = getflowdestination(world, ii, j, 0);

                    if (destpoint.x == x && destpoint.y == y) // If this is actually flowing into our cell
                    {
                        upcell.x = ii;
                        upcell.y = j;

                        largest = riverjan + riverjul;
                    }

                }
            }
        }
    }
    return (upcell);
}

// This function checks to see whether a tile on the global map has any water flowing into it.

int checkwaterinflow(planet& world, int x, int y)
{
    int i, j;

    int width = world.width();
    int height = world.height();

    // From the north

    i = x;
    j = y - 1;

    if (j >= 0 && world.riverdir(i, j) == 5)
        return 1;

    // From the northeast

    i = x + 1;
    if (i > width)
        i = 0;
    j = y - 1;

    if (j >= 0 && world.riverdir(i, j) == 6)
        return 1;

    // From the east

    i = x + 1;
    if (i > width)
        i = 0;
    j = y;

    if (j >= 0 && world.riverdir(i, j) == 7)
        return 1;

    // From the southeast

    i = x + 1;
    if (i > width)
        i = 0;
    j = y + 1;

    if (j <= height && world.riverdir(i, j) == 8)
        return 1;

    // From the south

    i = x;
    j = y + 1;

    if (j <= height && world.riverdir(i, j) == 1)
        return 1;

    // From the southwest

    i = x - 1;
    if (i < 0)
        i = width;
    j = y + 1;

    if (j <= height && world.riverdir(i, j) == 2)
        return 1;

    // From the west

    i = x - 1;
    if (i < 0)
        i = width;
    j = y;

    if (j <= height && world.riverdir(i, j) == 3)
        return 1;

    // From the northwest

    i = x - 1;
    if (i < 0)
        i = width;
    j = y - 1;

    if (j >= 0 && world.riverdir(i, j) == 4)
        return 1;

    return 0;
}

// This does the same thing for the regional map.

int checkregionalwaterinflow(region& region, int x, int y)
{
    int i, j;

    // From the north

    i = x;
    j = y - 1;

    if (region.riverdir(i, j) == 5)
        return 1;

    // From the northeast

    i = x + 1;
    j = y - 1;

    if (region.riverdir(i, j) == 6)
        return 1;

    // From the east

    i = x + 1;
    j = y;

    if (region.riverdir(i, j) == 7)
        return 1;

    // From the southeast

    i = x + 1;
    j = y + 1;

    if (region.riverdir(i, j) == 8)
        return 1;

    // From the south

    i = x;
    j = y + 1;

    if (region.riverdir(i, j) == 1)
        return 1;

    // From the southwest

    i = x - 1;
    j = y + 1;

    if (region.riverdir(i, j) == 2)
        return 1;

    // From the west

    i = x - 1;
    j = y;

    if (region.riverdir(i, j) == 3)
        return 1;

    // From the northwest

    i = x - 1;
    j = y - 1;

    if (region.riverdir(i, j) == 4)
        return 1;

    return 0;
}

// This function gets the total water inflow into a given tile on the global map.

twointegers gettotalinflow(planet& world, int x, int y)
{
    int width = world.width();
    int height = world.height();

    twointegers total;
    twointegers dest;

    total.x = 0;
    total.y = 0;

    for (int i = x - 1; i <= x + 1; i++)
    {
        int ii = i;

        if (ii<0 || ii>width)
            ii = wrap(ii, width);

        for (int j = y - 1; j <= y + 1; j++)
        {
            if (j >= 0 && j <= height)
            {
                dest = getflowdestination(world, i, j, 0);

                if (dest.x == x && dest.y == y)
                {
                    total.x = total.x + world.riverjan(i, j);
                    total.y = total.y + world.riverjul(i, j);
                }
            }
        }
    }
    return total;
}

// This does the same thing on the regional map.

twointegers gettotalinflow(region& region, int x, int y)
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    twointegers total;
    twointegers dest;

    total.x = 0;
    total.y = 0;

    for (int i = x - 1; i <= x + 1; i++)
    {
        for (int j = y - 1; j <= y + 1; j++)
        {
            if (i >= 0 && i <= rwidth && j >= 0 && j <= rheight)
            {
                dest = getregionalflowdestination(region, i, j, 0);

                if (dest.x == x && dest.y == y)
                {
                    total.x = total.x + region.riverjan(i, j);
                    total.y = total.y + region.riverjul(i, j);
                }
            }
        }
    }
    return total;
}

// This function finds the lowest neighbouring point that's still higher, on the regional map.

twointegers findlowesthigher(region& region, int dx, int dy, int x, int y, int janload, int julload, int crount, vector<vector<bool>>& mainriver)
{
    twointegers nextpoint;
    int origelev = region.map(x, y);

    nextpoint.x = -1;
    nextpoint.y = -1;

    int elev = 100000000;

    for (int i = x - 1; i <= x + 1; i++)
    {
        for (int j = y - 1; j <= y + 1; j++)
        {
            if (region.map(i, j) < elev && region.map(i, j) >= origelev && region.sea(i, j) == 0 && region.riverjan(i, j) == 0 && region.riverjul(i, j) == 0)
            {
                bool keepgoing = 1;

                if (crount > 1)
                {
                    int neighbouringrivers = 0;

                    for (int k = i - 1; k <= i + 1; k++)
                    {
                        for (int l = j - 1; l <= j + 1; l++)
                        {
                            if (region.riverjan(k, l) != 0 && region.riverjan(k, l) != janload && region.riverjul(k, l) != 0 && region.riverjul(k, l) != julload)
                                keepgoing = 0; // After the first point, avoid going next to the main river in this tile.

                            if (region.riverjan(k, l) != 0 || region.riverjul(k, l) != 0)
                                neighbouringrivers++;

                            if (region.sea(k, l) == 1) // Avoid going near coastlines after the first point, too.
                                keepgoing = 0;
                        }
                    }

                    if (neighbouringrivers > 2)
                        keepgoing = 0;
                }
                else
                {
                    for (int k = i - 1; k <= i + 1; k++)
                    {
                        for (int l = j - 1; l <= j + 1; l++)
                        {
                            if (region.riverdir(k, l) != 0) // && mainriver[dx-k][dy-l]==0)
                                keepgoing = 0;
                        }
                    }
                }

                // Avoid crossing existing rivers diagonally.

                if (i == x + 1 && j == y + 1)
                {
                    if (region.riverdir(x + 1, y) == 6 || region.riverdir(x, y + 1) == 2)
                        keepgoing = 0;
                }

                if (i == x - 1 && j == y - 1)
                {
                    if (region.riverdir(x, y - 1) == 6 || region.riverdir(x - 1, y) == 2)
                        keepgoing = 0;
                }

                if (i == x + 1 && j == y - 1)
                {
                    if (region.riverdir(x, y - 1) == 4 || region.riverdir(x + 1, y) == 8)
                        keepgoing = 0;
                }

                if (i == x - 1 && j == y + 1)
                {
                    if (region.riverdir(x - 1, y) == 4 || region.riverdir(x, y + 1) == 8)
                        keepgoing = 0;
                }

                if (keepgoing == 1)
                {
                    elev = region.map(i, j);
                    nextpoint.x = i;
                    nextpoint.y = j;
                }
            }
        }
    }
    return nextpoint;
}

// This function tells whether a given point on the world map is in or next to a lake.

int nearlake(planet& world, int x, int y, int dist, bool rift)
{
    int width = world.width();
    int height = world.height();

    for (int i = x - dist; i <= x + dist; i++)
    {
        int ii = i;

        if (ii<0 || ii>width)
            ii = wrap(ii, width);

        for (int j = y - dist; j <= y + dist; j++)
        {
            if (j >= 0 && j <= height)
            {
                if (world.lakesurface(ii, j) != 0)
                    return world.lakesurface(ii, j);

                if (rift == 1 && world.riftlakesurface(ii, j) != 0)
                    return world.riftlakesurface(ii, j);
            }
        }
    }
    return 0;
}

// This function tells us whether a tile is a lake tile on the edge.

int getlakeedge(planet& world, int x, int y)
{
    if (world.lakesurface(x, y) == 0)
        return 0;

    int width = world.width();
    int height = world.height();

    for (int i = x - 1; i <= x + 1; i++)
    {
        int ii = i;

        if (ii<0 || ii>width)
            ii = wrap(ii, width);

        for (int j = y - 1; j <= y + 1; j++)
        {
            if (j >= 0 && j <= height)
            {
                if (world.lakesurface(ii, j) == 0)
                    return 1;
            }
        }
    }
    return 0;
}

// This function tells whether a regional cell is next to a lake.

int nexttolake(region& region, int x, int y)
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    int dist = 1;

    for (int i = x - dist; i <= x + dist; i++)
    {
        int ii = i;

        for (int j = y - dist; j <= y + dist; j++)
        {
            if (i >= 0 && i <= rwidth && j >= 0 && j <= rheight && region.lakesurface(ii, j) != 0)
                return (region.lakesurface(ii, j));
        }
    }
    return (0);
}

// This function finds the level of the nearest lake to the given point in the regional map.

int getnearestlakelevel(region& region, int x, int y)
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    for (int n = 1; n <= 200; n++)
    {
        for (int i = x - n; i <= x + n; i++)
        {
            for (int j = y - n; j <= y + n; j++)
            {
                if (i >= 0 && i <= rwidth && j >= 0 && j <= rheight)
                {
                    if (region.lakesurface(i, j) != 0)
                        return region.lakesurface(i, j);
                }
            }
        }
    }
    return 0;
}

// This function finds the special value of the nearest lake to the given point in the regional map.

int getnearestlakespecial(region& region, int x, int y)
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    for (int n = 1; n <= 200; n++)
    {
        for (int i = x - n; i <= x + n; i++)
        {
            for (int j = y - n; j <= y + n; j++)
            {
                if (i >= 0 && i <= rwidth && j >= 0 && j <= rheight)
                {
                    if (region.lakesurface(i, j) != 0)
                        return region.special(i, j);
                }
            }
        }
    }
    return 0;
}

// This function finds the nearest river on the regional map.

twointegers findclosestriver(region& region, int x, int y, bool delta)
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    twointegers nearest;

    nearest.x = -1;;
    nearest.y = -1;

    for (int n = 1; n <= 14; n++)
    {
        int ii = 0;

        for (int i = x - 1; i <= x + n; i++)
        {
            int jj = 0;

            for (int j = y - n; j <= y + n; j++)
            {
                if (ii * jj < (n * n) + n)
                {
                    if (i >= 0 && i <= rwidth && j >= 0 && j <= rheight && region.waterdir(i, j, delta) != 0)
                    {
                        nearest.x = i;
                        nearest.y = j;

                        return nearest;
                    }
                }

                jj++;
            }

            ii++;
        }
    }

    return nearest;
}

// As above, but more rough and ready.

twointegers findclosestriverquickly(region& region, int x, int y)
{
    twointegers nearest;

    if (region.riverdir(x, y) != 0)
    {
        nearest.x = x;
        nearest.y = y;
        return nearest;
    }

    nearest.x = -1;;
    nearest.y = -1;

    for (int n = 1; n <= 30; n++)
    {
        for (int i = x - n; i <= x + n; i++)
        {
            int j = y - n;

            if (region.riverdir(i, j) != 0)
            {
                nearest.x = i;
                nearest.y = j;
                return nearest;
            }

            j = y + n;

            if (region.riverdir(i, j) != 0)
            {
                nearest.x = i;
                nearest.y = j;
                return nearest;
            }
        }

        for (int j = y - n; j <= y + n; j++)
        {
            int i = x - n;

            if (region.riverdir(i, j) != 0)
            {
                nearest.x = i;
                nearest.y = j;
                return nearest;
            }

            i = x + n;

            if (region.riverdir(i, j) != 0)
            {
                nearest.x = i;
                nearest.y = j;
                return nearest;
            }
        }
    }

    return nearest;
}

// This function finds how many inflows there are to a cell on the regional map.

int countinflows(region& region, int x, int y)
{
    int rwidth = region.rwidth();
    int rheight = region.rheight();

    int inflows = 0;
    twointegers dest;

    for (int i = x - 1; i <= x + 1; i++)
    {
        for (int j = y - 1; j <= y + 1; j++)
        {
            if (i >= 0 && i <= rwidth && j >= 0 && j <= rheight)
            {
                dest = getregionalflowdestination(region, i, j, 0);

                if (dest.x == x && dest.y == y)
                    inflows++;
            }
        }
    }

    return inflows;
}

// This function plugs the global variables into the world.

void initialiseworld(planet& world)
{
    world.clear();

    int saveversion = 1;        // Only save files that start with this number can be loaded
    int settingssaveversion = 1; // As above, but for settings files.
    int size = 2;
    int type = 2;
    int width = 2047; // 1280; // 2047;
    int height = 1024; // 640; // 1024;
    bool rotation = 1;            // 1 to rotate like Earth, 0 for the other way
    float tilt = 22.5;            // for calculating seasonal change
    float eccentricity = 0.0167f;  // the greater this is, the more climate difference there will be between hemispheres
    short perihelion = 0;         // point at which it's closest to the sun (0=Jan, 1=Jul)
    float gravity = 1.0f;          // affects mountain size and valley depth
    float lunar = 1.0f;            // affects tides
    float tempdecrease = 6.5f;     // for reducing temperature with elevation
    int northpolaradjust = 0; // -36;     // adjustment to temperature at the north pole
    int southpolaradjust = 0; // -36     // adjustment to temperature at the south pole
    int averagetemp = 14;              // average global temperature
    float waterpickup = 1.0f;        // How much water to pick up over oceans
    float riverfactor = 15.0f;     // for calculating flow in cubic metres/second
    int riverlandreduce = 20;     // how much rivers lower the land
    int estuarylimit = 20;        // how big a river must be to have an estuary
    int glacialtemp = 4;          // maximum temperature for glacial features
    int glaciertemp = -1;          // maximum temperature for actual glaciers
    float mountainreduce = 0.75f;  // factor to reduce mountain size by
    int climatenumber = 31;       // total number of climate types
    int maxelevation = 24000;     // maximum elevation
    int sealevel = 12000;         // sea level
    int craterno = 0;             // number of craters


    world.setsaveversion(saveversion);
    world.setsettingssaveversion(settingssaveversion);
    world.setsize(size);
    world.setwidth(width);
    world.setheight(height);
    world.settype(type);
    world.setrotation(rotation);
    world.settilt(tilt);
    world.seteccentricity(eccentricity);
    world.setperihelion(perihelion);
    world.setgravity(gravity);
    world.setlunar(lunar);
    world.settempdecrease(tempdecrease);
    world.setnorthpolaradjust(northpolaradjust);
    world.setsouthpolaradjust(southpolaradjust);
    world.setaveragetemp(averagetemp);
    world.setwaterpickup(waterpickup);
    world.setriverfactor(riverfactor);
    world.setriverlandreduce(riverlandreduce);
    world.setestuarylimit(estuarylimit);
    world.setglacialtemp(glacialtemp);
    world.setglaciertemp(glaciertemp);
    world.setmountainreduce(mountainreduce);
    world.setclimatenumber(climatenumber);
    world.setmaxelevation(maxelevation);
    world.setsealevel(sealevel);
    world.setlandtotal(0);
    world.setseatotal(0);
    world.setcraterno(craterno);

    world.setseatotal(0);
    world.setlandtotal(0);
}

// This function sets up the default colours (and other settings) for drawing relief maps.

void initialisemapcolours(planet& world)
{
    world.setlandshading(0.7f);
    world.setlakeshading(0.3f);
    world.setseashading(0.5f);

    world.setlandmarbling(0.5f);
    world.setlakemarbling(0.8f);
    world.setseamarbling(0.8f);

    world.setshadingdir(8);
    world.setsnowchange(1);
    world.setseaiceappearance(1);
    world.setminriverflowglobal(1000000);
    world.setminriverflowregional(150);
    world.setshowmangroves(1);
    world.setcolourcliffs(0);

    world.setseaice1(243);
    world.setseaice2(243);
    world.setseaice3(243); // Colours for sea ice.

    world.setocean1(62); // 62
    world.setocean2(93); // 93
    world.setocean3(149); // 149 // Colours for the shallow ocean.

    world.setdeepocean1(3); // 13
    world.setdeepocean2(24); // 34
    world.setdeepocean3(33); // 63 // Colours for the deep ocean.

    world.setbase1(48);
    world.setbase2(69);
    world.setbase3(13); // These are the basic colours for most land.

    world.setbasetemp1(108);
    world.setbasetemp2(111);
    world.setbasetemp3(51); // Basic colours for temperate regions.

    world.sethighbase1(171);
    world.sethighbase2(153);
    world.sethighbase3(135); // High base colours.

    world.setdesert1(225);
    world.setdesert2(175);
    world.setdesert3(118); // Desert colours.

    world.sethighdesert1(132);
    world.sethighdesert2(102);
    world.sethighdesert3(66); // High desert colours.

    world.setcolddesert1(111);
    world.setcolddesert2(93);
    world.setcolddesert3(72); // Cold desert colours.

    world.setgrass1(62);
    world.setgrass2(81);
    world.setgrass3(41); // Grass colours (in between base and desert).

    world.setcold1(207);
    world.setcold2(219);
    world.setcold3(231); // Cold colours.

    world.settundra1(117);
    world.settundra2(120);
    world.settundra3(62); // Tundra colours (in between base and cold).

    world.seteqtundra1(72);
    world.seteqtundra2(63);
    world.seteqtundra3(32); // Tundra at equatorial latitudes.

    world.setsaltpan1(200);
    world.setsaltpan2(200);
    world.setsaltpan3(200); // Salt pan colours.

    world.seterg1(244);
    world.seterg2(231);
    world.seterg3(90); // Erg colours.

    world.setwetlands1(54);
    world.setwetlands2(73);
    world.setwetlands3(75); // Wetlands colours.

    world.setlake1(93); //107;
    world.setlake2(118); //106;
    world.setlake3(178); //146; // Lake colours.

    world.setriver1(75);
    world.setriver2(104);
    world.setriver3(169); // River colours.

    world.setglacier1(209);
    world.setglacier2(222);
    world.setglacier3(251); // Glacier colours.

    world.setsand1(174);
    world.setsand2(168);
    world.setsand3(84); // Sand colours.

    world.setmud1(94);
    world.setmud2(85);
    world.setmud3(55); // Mud colours.

    world.setshingle1(142);
    world.setshingle2(113);
    world.setshingle3(58); // Shingle colours.

    world.setmangrove1(37);
    world.setmangrove2(70);
    world.setmangrove3(17); // Mangrove colours.

    world.sethighlight1(0);
    world.sethighlight2(255);
    world.sethighlight3(255); // Colours of highlights on the map.
}

// This function sets up the always-used variables in the region.

void initialiseregion(planet& world, region& region)
{
    int tilewidth = REGIONALTILEWIDTH + 4;//34;
    int tileheight = REGIONALTILEHEIGHT + 4; //34;

    int rstart = 16;          // Amount to ignore on the edge of the regional map for most purposes.
    int pixelmetres = 2500;   // number of metres each pixel represents

    region.settilewidth(tilewidth);
    region.settileheight(tileheight);
    region.setrstart(rstart);
    region.setsealevel(world.sealevel());
    region.setpixelmetres(pixelmetres);
}

// This randomises some of the world's properties. (Note: the possible changes are fairly modest.)

void changeworldproperties(planet& world)
{
    long seed = world.seed();
    fast_srand(seed);

    // Size and type

    world.setsize(2); // Large world
    world.setgravity(1.0f);

    world.settype(2);

    if (random(1, 30) == 1)
        world.settype(4);

    if (random(1, 12) == 1) // Fairly rarely, have more fragmented continents
        world.settype(1);

    if (random(1, 16) == 1) // Medium-sized world
    {
        world.setsize(1);
        world.setgravity(0.4f);

        if (random(1,4)!=1)
            world.settype(4);
    }

    if (random(1, 25) == 1) // Small world
    {
        world.setsize(0);
        world.setgravity(0.15f);

        if (random(1, 8) != 1)
            world.settype(4);
    }

    if (random(1, 30) == 1) // Occasionally, have ocean worlds
        world.settype(3);

    // Rotation

    if (random(1, 6) == 1)
        world.setrotation(0);
    else
        world.setrotation(1);

    // Gravity

    float oldgravity = world.gravity();

    int gravityvar = random(75, 125);

    world.setgravity(oldgravity * ((float)gravityvar / 100.0f));

    // Perihelion

    if (random(1, 6) == 1)
        world.setperihelion(1);

    // Obliquity

    float newtilt = 25.0f - (float)random(1, 50);
    newtilt = newtilt / 10.0f;

    newtilt = newtilt + 22.5f;

    world.settilt(newtilt);

    // Eccentricity

    float neweccentricity = (float)random(1, 200);
    neweccentricity = neweccentricity / 10000.0f + 0.005f;

    world.seteccentricity(neweccentricity);

    // Lunar pull

    float newlunar = (float)random(1, 40);

    newlunar = newlunar / 100.0f;
    newlunar = newlunar + 0.8f;

    world.setlunar(newlunar);

    // Average temperature

    int averagetemp  = random(1, 30);
    averagetemp = averagetemp - 1;

    world.setaveragetemp(averagetemp);

    // Polar temperature adjustments

    int npoleadjust = randomsign(random(0, 4));
    int spoleadjust = randomsign(random(0, 4));

    world.setnorthpolaradjust(npoleadjust);
    world.setsouthpolaradjust(spoleadjust);

    // Temperature decrease rate

    float newtempdecrease = (float)random(1, 100);
    newtempdecrease = newtempdecrease / 100.0f;

    newtempdecrease = newtempdecrease + 6.0f;

    world.settempdecrease(newtempdecrease);

    // Water pickup rate

    float newwater = 80.0f + (float)random(1, 40);

    newwater = newwater / 100.0f;

    world.setwaterpickup(newwater);

    // Glacial temperature

    int newglacial = -10 + random(1, 20);

    world.setglacialtemp(newglacial);

    // Sea level

    if (world.type() == 4)
    {
        int sealevel = world.sealevel();
        
        float sealevelmult = (float)random(15, 100);
        sealevelmult = sealevelmult / 100.0f;

        if (random(1, 5) == 1)
            sealevelmult = sealevelmult * sealevelmult;
        else
            sealevelmult = ((sealevelmult * sealevelmult) + sealevelmult) / 2.0f; // Make lower values more likely.

        float newsealevel = (float)sealevel * sealevelmult;
        sealevel = (int)newsealevel;

        world.setsealevel(sealevel);
    }
}

// This works out the total areas of land and sea in the world.

void getlandandseatotals(planet& world)
{
    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();
    
    int seatotal = 0;
    int landtotal = 0;

    for (int i = 1; i < width; i++) // Avoid the margins, to avoid false positives.
    {
        for (int j = 1; j < height; j++)
        {
            if (world.nom(i, j) > sealevel)
                landtotal++;
            else
                seatotal++;
        }
    }

    if (landtotal > 0)
    {
        if ((float)seatotal / (float)landtotal < 0.05) // If it's this close to 0, assume it is 0.
            seatotal = 0;
    }

    world.setlandtotal(landtotal);
    world.setseatotal(seatotal);
}

// This function creates a voronoi map.

void makevoronoi(vector<vector<short>>& voronoi, int width, int height, int points)
{
    vector<vector<int>> pointslist(points, vector<int>(2, 0));

    int margin = points / 6;

    int widthmargin = width / 10;
    int widthfarmargin = width - widthmargin;
    int heightmargin = height / 10;
    int heightfarmargin = height - heightmargin;

    for (int n = 0; n <= margin; n = n + 3) // Force it to put some points near the edges.
    {
        if (random(1, 2) == 1)
        {
            pointslist[n][0] = random(0, widthmargin);
            pointslist[n][1] = random(0, height);
        }
        else
        {
            pointslist[n][0] = random(widthfarmargin, width);
            pointslist[n][1] = random(0, height);
        }

        pointslist[n + 1][0] = random(0, width);
        pointslist[n + 1][1] = random(0, heightmargin);

        pointslist[n + 2][0] = random(0, width);
        pointslist[n + 2][1] = random(heightfarmargin, height);

    }

    for (int n = margin + 1; n < points; n++)
    {
        pointslist[n][0] = random(0, width);
        pointslist[n][1] = random(0, height);
    }

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            int closest = 0;
            int dist = 10000000;

            for (int point = 0; point < points; point++)
            {
                int xdist = i - pointslist[point][0];
                int ydist = j - pointslist[point][1];
                int pointdist = xdist * xdist + ydist * ydist;

                if (pointdist < dist)
                {
                    closest = point;
                    dist = pointdist;
                }
            }

            voronoi[i][j] = closest;
        }
    }
}

// This does the same thing, for the continental shelves.

void makeshelvesvoronoi(planet& world, vector<vector<short>>& voronoi, vector<vector<bool>>& outline, int pointdist)
{
    int width = world.width();
    int height = world.height();
    int maxelev = world.maxelevation();
    int midelev = maxelev / 2;

    int grain = 16; // Level of detail on this fractal map.
    float valuemod = 0.4f;
    int v = random(3, 6);
    float valuemod2 = (float)v;

    vector<vector<int>> fractal(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

    createfractal(fractal, width, height, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    int warpdist = 1;
    int warpdiv = midelev / warpdist;

    vector<vector<int>> pointsmap(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

    int pointnumber = 1;
    int movedist = (pointdist / 4) * 3;

    for (int i = 0; i <= width; i = i + pointdist)
    {
        for (int j = 0; j <= height; j = j + pointdist)
        {
            int ii = i + randomsign(random(1, movedist));
            int jj = j + randomsign(random(1, movedist));

            if (ii<0 || ii>width)
                ii = wrap(ii, width);

            if (jj < 0)
                jj = 0;

            if (jj > height)
                jj = height;

            pointsmap[ii][jj] = pointnumber;
            pointnumber++;
        }
    }

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            int closest = 0;
            int dist = 10000000;

            int warpdistx = 0;
            int warpdisty = 0;

            int warpx = i;
            int warpy = j;

            if (fractal[warpx][warpy] > maxelev / 2)
                warpdistx = (fractal[warpx][warpy] - midelev) / warpdiv;
            else
                warpdistx = 0 - (midelev - fractal[warpx][warpy]) / warpdiv;

            int checkx = i + warpdistx;

            if (checkx<0 || checkx>width)
                wrap(checkx, width);

            warpx = warpx + width / 2;

            if (warpx > width)
                warpx = warpx - width;

            if (fractal[warpx][warpy] > maxelev / 2)
                warpdisty = (fractal[warpx][warpy] - midelev) / warpdiv;
            else
                warpdisty = 0 - (midelev - fractal[warpx][warpy]) / warpdiv;

            int checky = j + warpdisty;

            if (checky < 0)
                checky = 0;

            if (checky > height)
                checky = height;

            checkx = i;
            checky = j;

            int distcheck = 10;

            do
            {
                for (int x = checkx - distcheck; x <= checkx + distcheck; x++)
                {
                    int xx = x;

                    if (xx<0 || xx>width)
                        xx = wrap(xx, width);

                    for (int y = checky - distcheck; y <= checky + distcheck; y++)
                    {
                        if (y >= 0 && y <= height)
                        {
                            if (pointsmap[xx][y] != 0)
                            {
                                int xdist = checkx - xx;
                                int ydist = checky - y;
                                int pointdist = xdist * xdist + ydist * ydist;

                                if (pointdist < dist)
                                {
                                    closest = pointsmap[xx][y];
                                    dist = pointdist;
                                }
                            }
                        }
                    }
                }

                distcheck = distcheck + 10;

            } while (closest == 0);

            voronoi[i][j] = closest;
        }
    }
}


