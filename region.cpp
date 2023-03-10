//
//  region.cpp
//  Undiscovered Worlds
//
//  Created by Jonathan Hill on 27/10/2019.
//
//  Please see functions.hpp for notes.

#include "region.hpp"
#include "functions.hpp"

region::region() //constructor
{
}

region::~region()
{
}

bool region::outline(int x, int y) const
{
    if (y<1 || y>itsrheight - 1)
        return 0;

    if (sea(x, y) == 0)
    {
        if (x > 0 && sea(x - 1, y) == 1)
            return 1;

        if (sea(x, y - 1) == 1)
            return 1;

        if (x < itsrwidth && sea(x + 1, y) == 1)
            return 1;

        if (sea(x, y + 1) == 1)
            return 1;
    }

    return 0;
}

// Other public functions

void region::clear()
{
    for (int i = 0; i <= itsrwidth; i++) // Set all the maps to 0.
    {
        for (int j = 0; j <= itsrheight; j++)
        {
            rmap[i][j] = 0;
            rjantempmap[i][j] = 0;
            rjultempmap[i][j] = 0;
            rjanrainmap[i][j] = 0;
            rjulrainmap[i][j] = 0;
            rclimatemap[i][j] = 0;
            rlakemap[i][j] = 0;
            rrivermapdir[i][j] = 0;
            rrivermapjan[i][j] = 0;
            rrivermapjul[i][j] = 0;
            rseaicemap[i][j] = 0;
            rfakeriversdir[i][j] = 0;
            rfakeriversjan[i][j] = 0;
            rfakeriversjul[i][j] = 0;
            rspecials[i][j] = 0;
            rdeltamapdir[i][j] = 0;
            rdeltamapjan[i][j] = 0;
            rdeltamapjul[i][j] = 0;
            rmountainsdone[i][j] = 0;
            rvolcanomap[i][j] = 0;
            rtidalmap[i][j] = 0;
            rmudmap[i][j] = 0;
            rsandmap[i][j] = 0;
            rshinglemap[i][j] = 0;
            rbarrierislandmap[i][j] = 0;
            testmap[i][j] = 0;
            testmap2[i][j] = 0;
            testmapfloat[i][j] = 0;
        }
    }
}



