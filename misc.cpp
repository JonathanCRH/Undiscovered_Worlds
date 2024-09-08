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
#include <iomanip>
#include <sstream>
#include <chrono>
#include <format>

#include "classes.hpp"
#include "planet.hpp"
#include "region.hpp"
#include "functions.hpp"

#define RADIANSTODEGREES 57.295779513082 

// This returns the coordinates of a point on the cubesphere in relation to a given point. (face, x, y identify the starting point. xshift and yshift are distances to the new point.)

globepoint getglobepoint(int edge, int face, int x, int y, int xshift, int yshift)
{
    globepoint newpoint;
    
    if (face == -1)
    {
        newpoint.face = -1;
        newpoint.x = -1;
        newpoint.y = -1;

        return newpoint;
    }

    newpoint.face = face;
    newpoint.x = x + xshift;
    newpoint.y = y + yshift;

    // If it's within the same face, it's easy!

    if (newpoint.x >= 0 && newpoint.x < edge && newpoint.y >= 0 && newpoint.y < edge)
        return newpoint;

        // First check the y coordinate.

    if (newpoint.y < 0)
    {
        switch (newpoint.face)
        {
        case 0:
            newpoint.face = 4;
            newpoint.y = edge + newpoint.y;
            break;

        case 1:
            newpoint.face = 4;
            newpoint.x = edge + newpoint.y;
            newpoint.y = edge - (x - xshift) - 1; // x - xshift
            break;

        case 2:
            newpoint.face = 4;
            newpoint.y = -newpoint.y - 1;
            newpoint.x = edge - newpoint.x - 1;
            break;

        case 3:
            newpoint.face = 4;
            newpoint.x = -newpoint.y - 1;
            newpoint.y = x + xshift;
            break;

        case 4:
            newpoint.face = 2;
            newpoint.y = -newpoint.y - 1;
            newpoint.x = edge - newpoint.x - 1;
            break;

        case 5:
            newpoint.face = 0;
            newpoint.y = edge + newpoint.y;
            break;
        }
    }
    else
    {
        if (newpoint.y >= edge)
        {
            switch (newpoint.face)
            {
            case 0:
                newpoint.face = 5;
                newpoint.y = newpoint.y - edge;
                break;

            case 1:
                newpoint.face = 5;
                newpoint.x = edge * 2 - newpoint.y - 1;
                newpoint.y = x + xshift;
                break;

            case 2:
                newpoint.face = 5;
                newpoint.x = edge - newpoint.x - 1;
                newpoint.y = edge * 2 - newpoint.y - 1;
                break;

            case 3:
                newpoint.face = 5;
                newpoint.x = newpoint.y - edge;
                newpoint.y = edge - (x + xshift) - 1;
                break;

            case 4:
                newpoint.face = 0;
                newpoint.y = newpoint.y - edge;
                break;

            case 5:
                newpoint.face = 2;
                newpoint.y = edge * 2 - newpoint.y - 1;
                newpoint.x = edge - newpoint.x - 1;
                break;
            }
        }
        else
        {
            // Now the x coordinate.

            if (newpoint.x < 0)
            {
                switch (newpoint.face)
                {
                case 0:
                    newpoint.face = 3;
                    newpoint.x = edge + newpoint.x;
                    break;

                case 1:
                    newpoint.face = 0;
                    newpoint.x = edge + newpoint.x;
                    break;

                case 2:
                    newpoint.face = 1;
                    newpoint.x = edge + newpoint.x;
                    break;

                case 3:
                    newpoint.face = 2;
                    newpoint.x = edge + newpoint.x;
                    break;

                case 4:
                    newpoint.face = 3;
                    newpoint.y = -newpoint.x - 1;
                    newpoint.x = y + yshift;
                    break;

                case 5:
                    newpoint.face = 3;
                    newpoint.y = edge + newpoint.x;
                    newpoint.x = edge - (y + yshift) - 0;
                    break;
                }
            }
            else
            {
                if (newpoint.x >= edge)
                {
                    switch (newpoint.face)
                    {
                    case 0:
                        newpoint.face = 1;
                        newpoint.x = newpoint.x - edge;
                        break;

                    case 1:
                        newpoint.face = 2;
                        newpoint.x = newpoint.x - edge;
                        break;

                    case 2:
                        newpoint.face = 3;
                        newpoint.x = newpoint.x - edge;
                        break;

                    case 3:
                        newpoint.face = 0;
                        newpoint.x = newpoint.x - edge;
                        break;

                    case 4:
                        newpoint.face = 1;
                        newpoint.y = newpoint.x - edge;
                        newpoint.x = edge - (y + yshift) - 1;
                        break;

                    case 5:
                        newpoint.face = 1;
                        newpoint.y = edge * 2 - newpoint.x - 1;
                        newpoint.x = y + yshift;
                        break;
                    }
                }
            }
        }
    }

    if (newpoint.x < 0 || newpoint.x >= edge || newpoint.y < 0 || newpoint.y >= edge)
    {
        newpoint.face = -1;
        newpoint.x = 0;
        newpoint.y = 0;
    }

    return newpoint;
}

// Same thing, but for use by the directional functions below.

globepoint getdirglobepoint(int edge, int face, int x, int y, int xshift, int yshift)
{
    globepoint newpoint;

    if (face == -1)
    {
        newpoint.face = -1;
        newpoint.x = -1;
        newpoint.y = -1;

        return newpoint;
    }

    newpoint.face = face;
    newpoint.x = x + xshift;
    newpoint.y = y + yshift;

    // If it's within the same face, it's easy!

    if (newpoint.x >= 0 && newpoint.x < edge && newpoint.y >= 0 && newpoint.y < edge)
        return newpoint;

    // First check the y coordinate.

    if (newpoint.y < 0)
    {
        switch (newpoint.face)
        {
        case 0:
            newpoint.face = 4;
            newpoint.y = edge + newpoint.y;
            break;

        case 1:
            newpoint.face = 4;
            newpoint.x = edge + newpoint.y;
            newpoint.y = edge - (x - xshift) - 1; // x - xshift
            break;

        case 2:
            newpoint.face = 4;
            newpoint.y = -newpoint.y - 1;
            newpoint.x = edge - newpoint.x - 1;
            break;

        case 3:
            newpoint.face = 4;
            newpoint.x = -newpoint.y - 1;
            newpoint.y = x + xshift;
            break;

        case 4:
            newpoint.face = 2;
            newpoint.y = -newpoint.y - 1;
            newpoint.x = edge - newpoint.x - 1;
            break;

        case 5:
            newpoint.face = 0;
            newpoint.y = edge + newpoint.y;
            break;
        }
    }
    else
    {
        if (newpoint.y >= edge)
        {
            switch (newpoint.face)
            {
            case 0:
                newpoint.face = 5;
                newpoint.y = newpoint.y - edge;
                break;

            case 1:
                newpoint.face = 5;
                newpoint.x = edge * 2 - newpoint.y - 1;
                newpoint.y = x + xshift;
                break;

            case 2:
                newpoint.face = 5;
                newpoint.x = edge - newpoint.x - 1;
                newpoint.y = edge * 2 - newpoint.y - 1;
                break;

            case 3:
                newpoint.face = 5;
                newpoint.x = newpoint.y - edge;
                newpoint.y = edge - (x + xshift) - 1;
                break;

            case 4:
                newpoint.face = 0;
                newpoint.y = newpoint.y - edge;
                break;

            case 5:
                newpoint.face = 2;
                newpoint.y = edge * 2 - newpoint.y - 1;
                newpoint.x = edge - newpoint.x - 1;
                break;
            }
        }
        else
        {
            // Now the x coordinate.

            if (newpoint.x < 0)
            {
                switch (newpoint.face)
                {
                case 0:
                    newpoint.face = 3;
                    newpoint.x = edge + newpoint.x;
                    break;

                case 1:
                    newpoint.face = 0;
                    newpoint.x = edge + newpoint.x;
                    break;

                case 2:
                    newpoint.face = 1;
                    newpoint.x = edge + newpoint.x;
                    break;

                case 3:
                    newpoint.face = 2;
                    newpoint.x = edge + newpoint.x;
                    break;

                case 4:
                    newpoint.face = 3;
                    newpoint.y = -newpoint.x - 1;
                    newpoint.x = y + yshift;
                    break;

                case 5:
                    newpoint.face = 3;
                    newpoint.y = edge + newpoint.x;
                    newpoint.x = edge - (y + yshift) - 0;
                    break;
                }
            }
            else
            {
                if (newpoint.x >= edge)
                {
                    switch (newpoint.face)
                    {
                    case 0:
                        newpoint.face = 1;
                        newpoint.x = newpoint.x - edge;
                        break;

                    case 1:
                        newpoint.face = 2;
                        newpoint.x = newpoint.x - edge;
                        break;

                    case 2:
                        newpoint.face = 3;
                        newpoint.x = newpoint.x - edge;
                        break;

                    case 3:
                        newpoint.face = 0;
                        newpoint.x = newpoint.x - edge;
                        break;

                    case 4:
                        newpoint.face = 1;
                        newpoint.y = newpoint.x - edge;
                        newpoint.x = edge - (y + yshift) - 1;
                        break;

                    case 5:
                        newpoint.face = 1;
                        newpoint.y = edge * 2 - newpoint.x - 1;
                        newpoint.x = y + yshift;
                        break;
                    }
                }
            }
        }
    }

    if (newpoint.x < 0 || newpoint.x >= edge || newpoint.y < 0 || newpoint.y >= edge)
    {
        newpoint.face = -1;
        newpoint.x = 0;
        newpoint.y = 0;
    }

    return newpoint;
}

// This converts a globepoint into a point on the cube in 3D coordinates.

threefloats getcubepoint(int edge, int face, int x, int y)
{
    threefloats cubepoint;

    float fedge = (float)edge;
    float fx = (float)x;
    float fy = (float)(y);

    switch (face)
    {
    case 0:
        cubepoint.x = (fx / fedge) * 2.0f - 1.0f;
        cubepoint.y = ((fy / fedge) * 2.0f - 1.0f) * -1.0f;
        cubepoint.z = 1.0f;
        break;

    case 1:
        cubepoint.x = 1.0f;
        cubepoint.y = ((fy / fedge) * 2.0f - 1.0f) * -1.0f;
        cubepoint.z = ((fx / fedge) * 2.0f - 1.0f) * -1.0f;
        break;

    case 2:
        cubepoint.x = ((fx / fedge) * 2.0f - 1.0f) * -1.0f;
        cubepoint.y = ((fy / fedge) * 2.0f - 1.0f) * -1.0f;
        cubepoint.z = -1.0f;
        break;

    case 3:
        cubepoint.x = -1.0f;
        cubepoint.y = ((fy / fedge) * 2.0f - 1.0f) * -1.0f;
        cubepoint.z = (fx / fedge) * 2.0f - 1.0f;
        break;

    case 4:
        cubepoint.x = (fx / fedge) * 2.0f - 1.0f;
        cubepoint.y = 1.0f;
        cubepoint.z = (fy / fedge) * 2.0f - 1.0f;
        break;

    case 5:
        cubepoint.x = (fx / fedge) * 2.0f - 1.0f;
        cubepoint.y = -1.0f;
        cubepoint.z = ((fy / fedge) * 2.0f - 1.0f) * -1.0f;
        break;
    }

    return cubepoint;
}

// This converts a globepoint into a cube point.

threefloats getcube(int edge, int face, int x, int y)
{
    threefloats cubepoint;

    float fedge = (float)edge;
    float fx = (float)x;
    float fy = (float)y;

    switch (face)
    {
    case 0:
        cubepoint.x = (fx / fedge) * 2.0f - 1.0f;
        cubepoint.y = ((fy / fedge) * 2.0f - 1.0f) * -1.0f;
        cubepoint.z = 1.0f;
        break;

    case 1:
        cubepoint.x = 1.0f;
        cubepoint.y = ((fy / fedge) * 2.0f - 1.0f) * -1.0f;
        cubepoint.z = ((fx / fedge) * 2.0f - 1.0f) * -1.0f;
        break;

    case 2:
        cubepoint.x = ((fx / fedge) * 2.0f - 1.0f) * -1.0f;
        cubepoint.y = ((fy / fedge) * 2.0f - 1.0f) * -1.0f;
        cubepoint.z = -1.0f;
        break;

    case 3:
        cubepoint.x = -1.0f;
        cubepoint.y = ((fy / fedge) * 2.0f - 1.0f) * -1.0f;
        cubepoint.z = (fx / fedge) * 2.0f - 1.0f;
        break;

    case 4:
        cubepoint.x = (fx / fedge) * 2.0f - 1.0f;
        cubepoint.y = 1.0f;
        cubepoint.z = (fy / fedge) * 2.0f - 1.0f;
        break;

    case 5:
        cubepoint.x = (fx / fedge) * 2.0f - 1.0f;
        cubepoint.y = -1.0f;
        cubepoint.z = ((fy / fedge) * 2.0f - 1.0f) * -1.0f;
        break;
    }

    return cubepoint;
}

// This converts a globepoint into longitude and latitude.

void getlonglat(int edge, int face, float x, float y, float& longitude, float& latitude)
{
    // First, convert the globepoint (face,x,y) into global coordinates (x,y,z) of the cube.

    threefloats cubepoint;

    float fedge = (float)edge;

    switch (face)
    {
    case 0:
        cubepoint.x = (x / fedge) * 2.0f - 1.0f;
        cubepoint.y = ((y / fedge) * 2.0f - 1.0f) * -1.0f;
        cubepoint.z = 1.0f;
        break;

    case 1:
        cubepoint.x = 1.0f;
        cubepoint.y = ((y / fedge) * 2.0f - 1.0f) * -1.0f;
        cubepoint.z = ((x / fedge) * 2.0f - 1.0f) * -1.0f;
        break;

    case 2:
        cubepoint.x = ((x / fedge) * 2.0f - 1.0f) * -1.0f;
        cubepoint.y = ((y / fedge) * 2.0f - 1.0f) * -1.0f;
        cubepoint.z = -1.0f;
        break;

    case 3:
        cubepoint.x = -1.0f;
        cubepoint.y = ((y / fedge) * 2.0f - 1.0f) * -1.0f;
        cubepoint.z = (x / fedge) * 2.0f - 1.0f;
        break;

    case 4:
        cubepoint.x = (x / fedge) * 2.0f - 1.0f;
        cubepoint.y = 1.0f;
        cubepoint.z = (y / fedge) * 2.0f - 1.0f;
        break;

    case 5:
        cubepoint.x = (x / fedge) * 2.0f - 1.0f;
        cubepoint.y = -1.0f;
        cubepoint.z = ((y / fedge) * 2.0f - 1.0f) * -1.0f;
        break;
    }

    // Now, we need to convert these into coordinates on the sphere.

    threefloats spherepoint;

    spherepoint.x = cubepoint.x * sqrt(1.0f - (cubepoint.y * cubepoint.y / 2.0f) - (cubepoint.z * cubepoint.z / 2.0f) + ((cubepoint.y * cubepoint.y) * (cubepoint.z * cubepoint.z) / 3.0f));
    spherepoint.y = cubepoint.y * sqrt(1.0f - (cubepoint.z * cubepoint.z / 2.0f) - (cubepoint.x * cubepoint.x / 2.0f) + ((cubepoint.z * cubepoint.z) * (cubepoint.x * cubepoint.x) / 3.0f));
    spherepoint.z = cubepoint.z * sqrt(1.0f - (cubepoint.x * cubepoint.x / 2.0f) - (cubepoint.y * cubepoint.y / 2.0f) + ((cubepoint.x * cubepoint.x) * (cubepoint.y * cubepoint.y) / 3.0f));

    // Now we can convert those coordinates into longitude and latitude.

    longitude = atan2(spherepoint.x, spherepoint.z);
    latitude = acos(spherepoint.y);

    longitude = longitude * (float)RADIANSTODEGREES;
    latitude = latitude * (float)RADIANSTODEGREES;

    latitude = latitude - 90.0f;
}

// This converts a longitude/latitude into degrees, minutes, and seconds.

threeintegers converttodms(float val)
{
    if (val < 0.0f)
        val = val * -1.0f;

    int degrees = (int)val;

    float dec = (val - (float)degrees) * 60.0f;

    int minutes = (int)dec;

    float dec2 = (dec - (float)minutes) * 60;

    int seconds = (int)dec2;

    threeintegers result;
    result.x = degrees;
    result.y = minutes;
    result.z = seconds;

    return result;
}

// These return the nearest globepoint in the given direction.

globepoint getnorthpoint(int edge, int face, int x, int y, vector<float>& longitude, vector<float>& latitude, float origlong)
{
    int index = face * edge * edge + x * edge + y;
    
    // First, get all the surrounding globepoints.

    globepoint neighbour[3][3];

    neighbour[1][1].face = face;
    neighbour[1][1].x = x;
    neighbour[1][1].y = y;

    neighbour[1][0] = getdirglobepoint(edge, face, x, y, 0, -1);
    neighbour[1][2] = getdirglobepoint(edge, face, x, y, 0, 1);
    neighbour[0][1] = getdirglobepoint(edge, face, x, y, -1, 0);
    neighbour[2][1] = getdirglobepoint(edge, face, x, y, 1, 0);

    if (neighbour[1][0].face == -1 || neighbour[1][2].face == -1 || neighbour[0][1].face == -1 || neighbour[2][1].face == -1)
        return neighbour[1][1];

    neighbour[0][0] = getdirglobepoint(edge, neighbour[1][0].face, neighbour[1][0].x, neighbour[1][0].y, -1, 0);
    neighbour[2][0] = getdirglobepoint(edge, neighbour[1][0].face, neighbour[1][0].x, neighbour[1][0].y, 1, 0);
    neighbour[0][2] = getdirglobepoint(edge, neighbour[1][2].face, neighbour[1][2].x, neighbour[1][2].y, -1, 0);
    neighbour[2][2] = getdirglobepoint(edge, neighbour[1][2].face, neighbour[1][2].x, neighbour[1][2].y, 1, 0);

    if (neighbour[0][0].face == -1 || neighbour[2][0].face == -1 || neighbour[0][2].face == -1 || neighbour[2][2].face == -1)
        return neighbour[1][1];

    // We want the point that's closest to the starting longitude, but in the correct direction.

    int bestx = -1;
    int besty = -1;
    float bestdiff = 1000.0f;

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (i != 1 || j != 1)
            {
                int thisindex = neighbour[i][j].face * edge * edge + neighbour[i][j].x * edge + neighbour[i][j].y;
                
                float diff = (float)(abs(origlong - longitude[thisindex]));

                if (diff < bestdiff && latitude[thisindex] < latitude[index])
                {
                    bestdiff = diff;
                    bestx = i;
                    besty = j;
                }
            }
        }
    }

    if (bestx != -1)
        return neighbour[bestx][besty];

    return neighbour[1][1];
}

globepoint getsouthpoint(int edge, int face, int x, int y, vector<float>& longitude, vector<float>& latitude, float origlong)
{
    int index = face * edge * edge + x * edge + y;

    // First, get all the surrounding globepoints.

    globepoint neighbour[3][3];

    neighbour[1][1].face = face;
    neighbour[1][1].x = x;
    neighbour[1][1].y = y;

    neighbour[1][0] = getdirglobepoint(edge, face, x, y, 0, -1);
    neighbour[1][2] = getdirglobepoint(edge, face, x, y, 0, 1);
    neighbour[0][1] = getdirglobepoint(edge, face, x, y, -1, 0);
    neighbour[2][1] = getdirglobepoint(edge, face, x, y, 1, 0);

    if (neighbour[1][0].face == -1 || neighbour[1][2].face == -1 || neighbour[0][1].face == -1 || neighbour[2][1].face == -1)
        return neighbour[1][1];

    neighbour[0][0] = getdirglobepoint(edge, neighbour[1][0].face, neighbour[1][0].x, neighbour[1][0].y, -1, 0);
    neighbour[2][0] = getdirglobepoint(edge, neighbour[1][0].face, neighbour[1][0].x, neighbour[1][0].y, 1, 0);
    neighbour[0][2] = getdirglobepoint(edge, neighbour[1][2].face, neighbour[1][2].x, neighbour[1][2].y, -1, 0);
    neighbour[2][2] = getdirglobepoint(edge, neighbour[1][2].face, neighbour[1][2].x, neighbour[1][2].y, 1, 0);

    if (neighbour[0][0].face == -1 || neighbour[2][0].face == -1 || neighbour[0][2].face == -1 || neighbour[2][2].face == -1)
        return neighbour[1][1];

    // We want the point that's closest to the starting longitude, but in the correct direction.

    int bestx = -1;
    int besty = -1;
    float bestdiff = 1000.0f;

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (i != 1 || j != 1)
            {
                int thisindex = neighbour[i][j].face * edge * edge + neighbour[i][j].x * edge + neighbour[i][j].y;

                float diff = (float)(abs(origlong - longitude[thisindex]));

                if (diff < bestdiff && latitude[thisindex] > latitude[index])
                {
                    bestdiff = diff;
                    bestx = i;
                    besty = j;
                }
            }
        }
    }

    if (bestx != -1)
        return neighbour[bestx][besty];

    return neighbour[1][1];
}

globepoint geteastpoint(int edge, int face, int x, int y, vector<float>& longitude, vector<float>& latitude, float origlat)
{
    // First, get all the surrounding globepoints.

    globepoint neighbour[3][3];

    neighbour[1][1].face = face;
    neighbour[1][1].x = x;
    neighbour[1][1].y = y;

    neighbour[1][0] = getdirglobepoint(edge, face, x, y, 0, -1);
    neighbour[1][2] = getdirglobepoint(edge, face, x, y, 0, 1);
    neighbour[0][1] = getdirglobepoint(edge, face, x, y, -1, 0);
    neighbour[2][1] = getdirglobepoint(edge, face, x, y, 1, 0);

    if (neighbour[1][0].face == -1 || neighbour[1][2].face == -1 || neighbour[0][1].face == -1 || neighbour[2][1].face == -1)
        return neighbour[1][1];

    neighbour[0][0] = getdirglobepoint(edge, neighbour[1][0].face, neighbour[1][0].x, neighbour[1][0].y, -1, 0);
    neighbour[2][0] = getdirglobepoint(edge, neighbour[1][0].face, neighbour[1][0].x, neighbour[1][0].y, 1, 0);
    neighbour[0][2] = getdirglobepoint(edge, neighbour[1][2].face, neighbour[1][2].x, neighbour[1][2].y, -1, 0);
    neighbour[2][2] = getdirglobepoint(edge, neighbour[1][2].face, neighbour[1][2].x, neighbour[1][2].y, 1, 0);

    if (neighbour[0][0].face == -1 || neighbour[2][0].face == -1 || neighbour[0][2].face == -1 || neighbour[2][2].face == -1)
        return neighbour[1][1];

    // Now, get their latitudes and longitudes and adjust where necessary (for the point at which negative longitudes meet positive ones).

    float thislatitude[3][3];
    float thislongitude[3][3];

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            int thisindex = neighbour[i][j].face * edge * edge + neighbour[i][j].x * edge + neighbour[i][j].y;

            thislongitude[i][j] = longitude[thisindex];
            thislatitude[i][j] = latitude[thisindex];
        }
    }

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (thislongitude[1][1] > 170.0f)
            {
                if (thislongitude[i][j] < 0.0f)
                    thislongitude[i][j] = thislongitude[i][j] + 360.0f;
            }

            if (thislongitude[1][1] < -170.0f)
            {
                if (thislongitude[i][j] > 0.0f)
                    thislongitude[i][j] = thislongitude[i][j] - 360.0f;
            }
        }
    }

    // We want the point that's closest to the starting latitude, but in the correct direction.

    int bestx = -1;
    int besty = -1;
    float bestdiff = 1000.0f;

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (i != 1 || j != 1)
            {
                float diff = (float)(abs(origlat - thislatitude[i][j]));

                if (diff<bestdiff && thislongitude[i][j] > thislongitude[1][1])
                {
                    bestdiff = diff;
                    bestx = i;
                    besty = j;
                }
            }
        }
    }

    if (bestx != -1)
        return neighbour[bestx][besty];

    return neighbour[1][1];
}

globepoint getwestpoint(int edge, int face, int x, int y, vector<float>& longitude, vector<float>& latitude, float origlat)
{
    // First, get all the surrounding globepoints.

    globepoint neighbour[3][3];

    neighbour[1][1].face = face;
    neighbour[1][1].x = x;
    neighbour[1][1].y = y;

    neighbour[1][0] = getdirglobepoint(edge, face, x, y, 0, -1);
    neighbour[1][2] = getdirglobepoint(edge, face, x, y, 0, 1);
    neighbour[0][1] = getdirglobepoint(edge, face, x, y, -1, 0);
    neighbour[2][1] = getdirglobepoint(edge, face, x, y, 1, 0);

    if (neighbour[1][0].face == -1 || neighbour[1][2].face == -1 || neighbour[0][1].face == -1 || neighbour[2][1].face == -1)
        return neighbour[1][1];

    neighbour[0][0] = getdirglobepoint(edge, neighbour[1][0].face, neighbour[1][0].x, neighbour[1][0].y, -1, 0);
    neighbour[2][0] = getdirglobepoint(edge, neighbour[1][0].face, neighbour[1][0].x, neighbour[1][0].y, 1, 0);
    neighbour[0][2] = getdirglobepoint(edge, neighbour[1][2].face, neighbour[1][2].x, neighbour[1][2].y, -1, 0);
    neighbour[2][2] = getdirglobepoint(edge, neighbour[1][2].face, neighbour[1][2].x, neighbour[1][2].y, 1, 0);

    if (neighbour[0][0].face == -1 || neighbour[2][0].face == -1 || neighbour[0][2].face == -1 || neighbour[2][2].face == -1)
        return neighbour[1][1];

    // Now, get their latitudes and longitudes and adjust where necessary (for the point at which negative longitudes meet positive ones).

    float thislatitude[3][3];
    float thislongitude[3][3];

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            int thisindex = neighbour[i][j].face * edge * edge + neighbour[i][j].x * edge + neighbour[i][j].y;

            thislongitude[i][j] = longitude[thisindex];
            thislatitude[i][j] = latitude[thisindex];
        }
    }

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (thislongitude[1][1] > 170.0f)
            {
                if (thislongitude[i][j] < 0.0f)
                    thislongitude[i][j] = thislongitude[i][j] + 360.0f;
            }

            if (thislongitude[1][1] < -170.0f)
            {
                if (thislongitude[i][j] > 0.0f)
                    thislongitude[i][j] = thislongitude[i][j] - 360.0f;
            }
        }
    }

    // We want the point that's closest to the starting latitude, but in the correct direction.

    int bestx = -1;
    int besty = -1;
    float bestdiff = 1000.0f;

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (i != 1 || j != 1)
            {
                float diff = (float)(abs(origlat - thislatitude[i][j]));

                if (diff<bestdiff && thislongitude[i][j] < thislongitude[1][1])
                {
                    bestdiff = diff;
                    bestx = i;
                    besty = j;
                }
            }
        }
    }

    if (bestx != -1)
        return neighbour[bestx][besty];

    return neighbour[1][1];
}

// These are similar but rougher (more suitable for just moving one cell at a time.)

globepoint getnorthpointrough(int edge, int face, int x, int y, vector<float>& longitude, vector<float>& latitude)
{
    int index = face * edge * edge + x * edge + y;

    if (face >= 0 && face <= 3 && y > 1)
    {
        if (random(1, 3) != 1)
        {
            globepoint point;
            point.face = face;
            point.x = x;
            point.y = y - 1;

            return point;
        }
    }

    if (face == 4 && random(1, 6) != 1 && x >= 1 && x < edge - 1 && y >= 1 && y <= edge - 1)
    {
        int xdist = abs(x - edge / 2);
        int ydist = abs(y - edge / 2);

        if (xdist > 1 && ydist > 1)
        {
            globepoint point;
            point.face = face;
            point.x = x;
            point.y = y;

            if (random(1, xdist + ydist) <= xdist)
            {
                if (x > edge / 2)
                    point.x = x - 1;
                else
                    point.x = x + 1;
            }
            else
            {
                if (y > edge / 2)
                    point.y = y - 1;
                else
                    point.y = y + 1;
            }

            return point;
        }
    }

    if (face == 5 && random(1, 6) != 1 && x >= 1 && x < edge - 1 && y >= 1 && y <= edge - 1)
    {
        int xdist = abs(x - edge / 2);
        int ydist = abs(y - edge / 2);

        if (xdist > 1 && ydist > 1)
        {
            globepoint point;
            point.face = face;
            point.x = x;
            point.y = y;

            if (random(1, xdist + ydist) <= xdist)
            {
                if (x > edge / 2)
                    point.x = x + 1;
                else
                    point.x = x - 1;
            }
            else
            {
                if (y > edge / 2)
                    point.y = y + 1;
                else
                    point.y = y - 1;
            }

            return point;
        }
    }

    // First, get all the surrounding globepoints.

    globepoint neighbour[3][3];

    neighbour[1][1].face = face;
    neighbour[1][1].x = x;
    neighbour[1][1].y = y;

    neighbour[1][0] = getdirglobepoint(edge, face, x, y, 0, -1);
    neighbour[1][2] = getdirglobepoint(edge, face, x, y, 0, 1);
    neighbour[0][1] = getdirglobepoint(edge, face, x, y, -1, 0);
    neighbour[2][1] = getdirglobepoint(edge, face, x, y, 1, 0);

    neighbour[0][0] = getdirglobepoint(edge, neighbour[1][0].face, neighbour[1][0].x, neighbour[1][0].y, -1, 0);
    neighbour[2][0] = getdirglobepoint(edge, neighbour[1][0].face, neighbour[1][0].x, neighbour[1][0].y, 1, 0);
    neighbour[0][2] = getdirglobepoint(edge, neighbour[1][2].face, neighbour[1][2].x, neighbour[1][2].y, -1, 0);
    neighbour[2][2] = getdirglobepoint(edge, neighbour[1][2].face, neighbour[1][2].x, neighbour[1][2].y, 1, 0);

    for (int n = 1; n < 100; n++)
    {
        int x = random(1, 3) - 1;
        int y = random(1, 3) - 1;

        if (latitude[neighbour[x][y].face * edge * edge + neighbour[x][y].x * edge + neighbour[x][y].y] < latitude[index])
            return neighbour[x][y];
    }

    return neighbour[1][1];
}

globepoint getsouthpointrough(int edge, int face, int x, int y, vector<float>& longitude, vector<float>& latitude)
{
    int index = face * edge * edge + x * edge + y;

    if (face >= 0 && face <= 3 && y < edge - 1)
    {
        if (random(1, 3) != 1)
        {
            globepoint point;
            point.face = face;
            point.x = x;
            point.y = y + 1;

            return point;
        }
    }

    if (face == 4 && random(1, 6) != 1 && x >= 1 && x < edge - 1 && y >= 1 && y <= edge - 1)
    {
        int xdist = abs(x - edge / 2);
        int ydist = abs(y - edge / 2);

        if (xdist > 1 && ydist > 1)
        {
            globepoint point;
            point.face = face;
            point.x = x;
            point.y = y;

            if (random(1, xdist + ydist) <= xdist)
            {
                if (x > edge / 2)
                    point.x = x + 1;
                else
                    point.x = x - 1;
            }
            else
            {
                if (y > edge / 2)
                    point.y = y + 1;
                else
                    point.y = y - 1;
            }

            return point;
        }
    }

    if (face == 5 && random(1, 6) != 1 && x >= 1 && x < edge - 1 && y >= 1 && y <= edge - 1)
    {
        int xdist = abs(x - edge / 2);
        int ydist = abs(y - edge / 2);

        if (xdist > 1 && ydist > 1)
        {
            globepoint point;
            point.face = face;
            point.x = x;
            point.y = y;

            if (random(1, xdist + ydist) <= xdist)
            {
                if (x > edge / 2)
                    point.x = x - 1;
                else
                    point.x = x + 1;
            }
            else
            {
                if (y > edge / 2)
                    point.y = y - 1;
                else
                    point.y = y + 1;
            }

            return point;
        }
    }

    // First, get all the surrounding globepoints.

    globepoint neighbour[3][3];

    neighbour[1][1].face = face;
    neighbour[1][1].x = x;
    neighbour[1][1].y = y;

    neighbour[1][0] = getdirglobepoint(edge, face, x, y, 0, -1);
    neighbour[1][2] = getdirglobepoint(edge, face, x, y, 0, 1);
    neighbour[0][1] = getdirglobepoint(edge, face, x, y, -1, 0);
    neighbour[2][1] = getdirglobepoint(edge, face, x, y, 1, 0);

    neighbour[0][0] = getdirglobepoint(edge, neighbour[1][0].face, neighbour[1][0].x, neighbour[1][0].y, -1, 0);
    neighbour[2][0] = getdirglobepoint(edge, neighbour[1][0].face, neighbour[1][0].x, neighbour[1][0].y, 1, 0);
    neighbour[0][2] = getdirglobepoint(edge, neighbour[1][2].face, neighbour[1][2].x, neighbour[1][2].y, -1, 0);
    neighbour[2][2] = getdirglobepoint(edge, neighbour[1][2].face, neighbour[1][2].x, neighbour[1][2].y, 1, 0);

    for (int n = 1; n < 100; n++)
    {
        int x = random(1, 3) - 1;
        int y = random(1, 3) - 1;

        if (latitude[neighbour[x][y].face * edge * edge + neighbour[x][y].x * edge + neighbour[x][y].y] > latitude[index])
            return neighbour[x][y];
    }

    return neighbour[1][1];
}

// This prepares the directional lookup table.

void createdirectiontable(int edge, vector<fourglobepoints>& dirpoint, vector<float>& longitude, vector<float>& latitude)
{
    for (int face = 0; face < 6; face++)
    {
        int vface = face * 512 * 512;

        for (int i = 0; i < 512; i++)
        {
            int vi = vface + i * 512;

            for (int j = 0; j < 512; j++)
            {
                int index = vi + j;

                dirpoint[index].north.face = -1;
                dirpoint[index].east.face = -1;
                dirpoint[index].south.face = -1;
                dirpoint[index].west.face = -1;
            }
        }
    }

    // East directions.

    for (int face = 0; face < 4; face++) // Faces 0-3
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                float origlat = latitude[index];
                float origlong = longitude[index];

                globepoint thispoint;
                thispoint.face = face;
                thispoint.x = i;
                thispoint.y = j;

                bool keepgoing = 1;

                int crount = 0;

                do
                {
                    int thisindex = thispoint.face * edge * edge + thispoint.x * edge + thispoint.y;
                    
                    if (dirpoint[thisindex].east.face != -1)
                        keepgoing = 0;
                    else
                    {
                        globepoint newpoint = geteastpoint(edge, thispoint.face, thispoint.x, thispoint.y, longitude, latitude, origlat);

                        dirpoint[thisindex].east = newpoint;

                        thispoint = newpoint;

                        crount++;

                        if (crount > edge * 4)
                            keepgoing = 0;
                    }

                } while (keepgoing);
            }
        }
    }

    for (int n = 0; n < 4; n++) // Faces 4-5
    {
        for (int m = 0; m < edge; m++)
        {
            int i, j;

            switch (n)
            {
            case 0:
                i = m;
                j = edge - 1;
                break;

            case 1:
                i = edge - 1;
                j = edge - 1 - m;
                break;

            case 2:
                i = edge - 1 - m;
                j = 0;
                break;

            case 3:
                i = 0;
                j = m;
                break;
            }

            float righttotal = (float)(edge / 2 - i);
            float downtotal = (float)(edge / 2 - j);

            float totalsteps = (float)edge;

            float rightstep = righttotal / totalsteps;
            float downstep = downtotal / totalsteps;

            int x = i;
            int y = j;

            float fx = (float)x;
            float fy = (float)y;

            for (int z = 0; z < (int)totalsteps; z++)
            {
                fx = fx + rightstep;
                fy = fy + downstep;

                if (x != (int)fx || y != (int)fx)
                {
                    x = (int)fx;
                    y = (int)fy;

                    int vy = x * edge + y;

                    for (int face = 4; face < 6; face++)
                    {
                        int index = face * edge * edge + vy;

                        float origlat = latitude[index];
                        float origlong = longitude[index];

                        globepoint thispoint;
                        thispoint.face = face;
                        thispoint.x = x;
                        thispoint.y = y;

                        bool keepgoing = 1;

                        int crount = 0;

                        do
                        {
                            int thisindex = thispoint.face * edge * edge + thispoint.x * edge + thispoint.y;

                            if (dirpoint[thisindex].east.face != -1)
                                keepgoing = 0;
                            else
                            {
                                globepoint newpoint = geteastpoint(edge, thispoint.face, thispoint.x, thispoint.y, longitude, latitude, origlat);

                                dirpoint[thisindex].east = newpoint;

                                thispoint = newpoint;

                                crount++;

                                if (crount > edge * 3)
                                    keepgoing = 0;
                            }

                        } while (keepgoing);
                    }
                }
            }
        }
    }

    for (int face = 0; face < 6; face++) // Calculate directions for any leftovers.
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                if (dirpoint[index].east.face == -1)
                {
                    globepoint newpoint = geteastpoint(edge, face, i, j, longitude, latitude, latitude[index]);

                    dirpoint[index].east = newpoint;
                }
            }
        }
    }

    // West directions.

    for (int face = 0; face < 4; face++) // Faces 0-3
    {
        int vface = face * edge * edge;

        for (int i = edge - 1; i >= 0; i--)
        {
            int vi = vface + i * edge;

            for (int j = edge - 1; j >= 0; j--)
            {
                int index = vi + j;

                float origlat = latitude[index];
                float origlong = longitude[index];

                globepoint thispoint;
                thispoint.face = face;
                thispoint.x = i;
                thispoint.y = j;

                bool keepgoing = 1;

                int crount = 0;

                do
                {
                    int thisindex = thispoint.face * edge * edge + thispoint.x * edge + thispoint.y;

                    if (dirpoint[thisindex].west.face != -1)
                        keepgoing = 0;
                    else
                    {
                        globepoint newpoint = getwestpoint(edge, thispoint.face, thispoint.x, thispoint.y, longitude, latitude, origlat);

                        dirpoint[thisindex].west = newpoint;

                        thispoint = newpoint;

                        crount++;

                        if (crount > edge * 4)
                            keepgoing = 0;
                    }

                } while (keepgoing);
            }
        }
    }

    for (int n = 0; n < 4; n++) // Faces 4-5
    {
        for (int m = 0; m < edge; m++)
        {
            int i, j;

            switch (n)
            {
            case 0:
                i = m;
                j = edge - 1;
                break;

            case 1:
                i = edge - 1;
                j = edge - 1 - m;
                break;

            case 2:
                i = edge - 1 - m;
                j = 0;
                break;

            case 3:
                i = 0;
                j = m;
                break;
            }

            float righttotal = (float)(edge / 2 - i);
            float downtotal = (float)(edge / 2 - j);

            float totalsteps = (float)edge;

            float rightstep = righttotal / totalsteps;
            float downstep = downtotal / totalsteps;

            int x = i;
            int y = j;

            int vy = x * edge + y;

            float fx = (float)x;
            float fy = (float)y;

            for (int z = 0; z < (int)totalsteps; z++)
            {
                fx = fx + rightstep;
                fy = fy + downstep;

                if (x != (int)fx || y != (int)fx)
                {
                    x = (int)fx;
                    y = (int)fy;

                    for (int face = 4; face < 6; face++)
                    {
                        int thisindex = face * edge * edge + vy;

                        float origlat = latitude[thisindex];
                        float origlong = longitude[thisindex];

                        globepoint thispoint;
                        thispoint.face = face;
                        thispoint.x = x;
                        thispoint.y = y;

                        bool keepgoing = 1;

                        int crount = 0;

                        do
                        {
                            int index = thispoint.face * edge * edge + thispoint.x * edge + thispoint.y;

                            if (dirpoint[thisindex].west.face != -1)
                                keepgoing = 0;
                            else
                            {
                                globepoint newpoint = getwestpoint(edge, thispoint.face, thispoint.x, thispoint.y, longitude, latitude, origlat);

                                dirpoint[thisindex].west = newpoint;

                                thispoint = newpoint;

                                crount++;

                                if (crount > edge * 3)
                                    keepgoing = 0;
                            }

                        } while (keepgoing);
                    }
                }
            }
        }
    }

    for (int face = 0; face < 6; face++) // Calculate directions for any leftovers.
    {
        int vface = face * edge * edge;

        for (int i = edge - 1; i >= 0; i--)
        {
            int vi = vface + i * edge;

            for (int j = edge - 1; j >= 0; j--)
            {
                int index = vi + j;

                if (dirpoint[index].west.face == -1)
                {
                    globepoint newpoint = getwestpoint(edge, face, i, j, longitude, latitude, latitude[index]);

                    dirpoint[index].west = newpoint;
                }
            }
        }
    }

    // North and south directions.

    for (int face = 0; face < 4; face++) // Faces 0-3
    {
        int vface = face * edge * edge;

        for (int i = edge - 1; i >= 0; i--)
        {
            int vi = vface + i * edge;

            for (int j = edge - 1; j >= 0; j--)
            {
                int index = vi + j;

                float origlat = latitude[index];
                float origlong = longitude[index];

                globepoint thispoint;
                thispoint.face = face;
                thispoint.x = i;
                thispoint.y = j;

                bool keepgoing = 1;

                int crount = 0;

                do
                {
                    if (dirpoint[index].north.face != -1)
                        keepgoing = 0;
                    else
                    {
                        globepoint newpoint = getnorthpoint(edge, thispoint.face, thispoint.x, thispoint.y, longitude, latitude, origlong);

                        dirpoint[thispoint.face * edge * edge + thispoint.x * edge + thispoint.y].north = newpoint;

                        thispoint = newpoint;

                        crount++;

                        if (crount > edge * 4)
                            keepgoing = 0;
                    }

                } while (keepgoing);

                thispoint.face = face;
                thispoint.x = i;
                thispoint.y = j;

                keepgoing = 1;

                crount = 0;

                do
                {
                    int thisindex = thispoint.face * edge * edge + thispoint.x * edge + thispoint.y;

                    if (dirpoint[thisindex].south.face != -1)
                        keepgoing = 0;
                    else
                    {
                        globepoint newpoint = getsouthpoint(edge, thispoint.face, thispoint.x, thispoint.y, longitude, latitude, origlong);

                        dirpoint[thisindex].south = newpoint;

                        thispoint = newpoint;

                        crount++;

                        if (crount > edge * 4)
                            keepgoing = 0;
                    }

                } while (keepgoing);
            }
        }

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            globepoint thispoint = getglobepoint(edge, face, i, 0, 0, -1);

            if (thispoint.face != -1)
                dirpoint[vi].north = thispoint;

            thispoint = getglobepoint(edge, face, i, edge - 1, 0, 1);

            if (thispoint.face != -1)
                dirpoint[vi + edge - 1].south = thispoint;
        }
    }

    for (int n = 0; n < 4; n++) // Faces 4-5
    {
        for (int m = 0; m < edge; m++)
        {
            int i, j;

            switch (n)
            {
            case 0:
                i = m;
                j = edge - 1;
                break;

            case 1:
                i = edge - 1;
                j = edge - 1 - m;
                break;

            case 2:
                i = edge - 1 - m;
                j = 0;
                break;

            case 3:
                i = 0;
                j = m;
                break;
            }

            float righttotal = (float)(edge / 2 - i);
            float downtotal = (float)(edge / 2 - j);

            float totalsteps = (float)edge;

            float rightstep = righttotal / totalsteps;
            float downstep = downtotal / totalsteps;

            int x = i;
            int y = j;

            float fx = (float)x;
            float fy = (float)y;

            for (int z = 0; z < (int)totalsteps; z++)
            {
                fx = fx + rightstep;
                fy = fy + downstep;

                if (x != (int)fx || y != (int)fx)
                {
                    x = (int)fx;
                    y = (int)fy;

                    int vy = x * edge + y;

                    for (int face = 4; face < 6; face++)
                    {
                        int index = face * edge * edge + vy;

                        float origlat = latitude[index];
                        float origlong = longitude[index];

                        globepoint thispoint;
                        thispoint.face = face;
                        thispoint.x = x;
                        thispoint.y = y;

                        bool keepgoing = 1;

                        int crount = 0;

                        do
                        {
                            int thisindex = thispoint.face * edge * edge + thispoint.x * edge + thispoint.y;

                            if (dirpoint[thisindex].north.face != -1)
                                keepgoing = 0;
                            else
                            {
                                globepoint newpoint = getnorthpoint(edge, thispoint.face, thispoint.x, thispoint.y, longitude, latitude, origlong);

                                dirpoint[thisindex].north = newpoint;

                                thispoint = newpoint;

                                crount++;

                                if (crount > edge * 3)
                                    keepgoing = 0;
                            }

                        } while (keepgoing);

                        thispoint.face = face;
                        thispoint.x = x;
                        thispoint.y = y;

                        keepgoing = 1;

                        crount = 0;

                        do
                        {
                            int thisindex = thispoint.face * edge * edge + thispoint.x * edge + thispoint.y;

                            if (dirpoint[thisindex].south.face != -1)
                                keepgoing = 0;
                            else
                            {
                                globepoint newpoint = getsouthpoint(edge, thispoint.face, thispoint.x, thispoint.y, longitude, latitude, origlong);

                                dirpoint[thisindex].south = newpoint;

                                thispoint = newpoint;

                                crount++;

                                if (crount > edge * 3)
                                    keepgoing = 0;
                            }

                        } while (keepgoing);
                    }
                }
            }
        }
    }

    // Ensure roughly correct north/south directions at the edges of faces 4 and 5. (For some reason, some of them are weird otherwise.)

    for (int n = 0; n < edge; n++)
    {
        globepoint thispoint;
        thispoint.face = 4;
        int vface = 4 * edge * edge;

        thispoint.x = edge - 2;
        thispoint.y = n;
        dirpoint[vface + (edge - 1) * edge + n].north = thispoint;

        thispoint.x = 1;
        thispoint.y = n;
        dirpoint[vface + n].north = thispoint;

        thispoint.x = n;
        thispoint.y = 1;
        dirpoint[vface + n * edge].north = thispoint;

        thispoint.x = n;
        thispoint.y = edge - 2;
        dirpoint[vface + n * edge + edge - 1].north = thispoint;

        thispoint.face = 5;
        vface = 5 * edge * edge;

        thispoint.x = edge - 2;
        thispoint.y = n;
        dirpoint[vface + (edge - 1) * edge + n].south = thispoint;

        thispoint.x = 1;
        thispoint.y = n;
        dirpoint[vface + n].south = thispoint;

        thispoint.x = n;
        thispoint.y = 1;
        dirpoint[vface + n * edge].south = thispoint;

        thispoint.x = n;
        thispoint.y = edge - 2;
        dirpoint[vface + n * edge + edge - 1].south = thispoint;
    }

    for (int n = 0; n < edge; n++)
    {
        globepoint thispoint;

        thispoint.face = 0;
        thispoint.x = n;
        thispoint.y = 0;

        int vface = 4 * edge * edge;

        dirpoint[vface + n * edge + edge - 1].south = thispoint;

        thispoint.face = 1;
        thispoint.x = edge - n - 1;

        dirpoint[vface + (edge - 1) * edge + n].south = thispoint;

        thispoint.face = 2;
        thispoint.x = edge - n - 1;

        dirpoint[vface + n * edge].south = thispoint;

        thispoint.face = 3;
        thispoint.x = n;

        dirpoint[vface + n].south = thispoint;

        thispoint.face = 0;
        thispoint.x = n;
        thispoint.y = edge - 1;

        vface = 5 * edge * edge;

        dirpoint[vface + n * edge].north = thispoint;

        thispoint.face = 1;
        thispoint.x = n;

        dirpoint[vface + (edge - 1) * edge + n].north = thispoint;

        thispoint.face = 2;
        thispoint.x = edge - n - 1;

        dirpoint[vface + n * edge + edge - 1].north = thispoint;

        thispoint.face = 3;
        thispoint.x = edge - n - 1;

        dirpoint[vface + n].north = thispoint;
    }

    for (int face = 0; face < 6; face++) // Calculate directions for any leftovers.
    {
        int vface = face * edge * edge;

        for (int i = edge - 1; i >= 0; i--)
        {
            int vi = vface + i * edge;

            for (int j = edge - 1; j >= 0; j--)
            {
                int index = vi + j;

                if (dirpoint[index].north.face == -1)
                {
                    globepoint newpoint = getnorthpoint(edge, face, i, j, longitude, latitude, longitude[index]);

                    dirpoint[index].north = newpoint;
                }

                if (dirpoint[index].south.face == -1)
                {
                    globepoint newpoint = getsouthpoint(edge, face, i, j, longitude, latitude, longitude[index]);

                    dirpoint[index].south = newpoint;
                }
            }
        }
    }
}

// This creates the directional code table.

void createdircodetable(int edge, vector<fourglobepoints>& dirpoint, vector<fourdirs>& dircode)
{
    int eedge = edge * edge;

    // First do the middles of the faces.
   
    for (int face = 0; face < 6; face++)
    {
        int vface = face * eedge;

        for (int i = 1; i < edge - 1; i++)
        {
            int vi = vface + i * edge;

            for (int j = 1; j < edge - 1; j++)
            {
                int index = vi + j;

                globepoint thisnorthpoint = dirpoint[index].north;
                globepoint thissouthpoint = dirpoint[index].south;
                globepoint thiseastpoint = dirpoint[index].east;
                globepoint thiswestpoint = dirpoint[index].west;

                for (int k = - 1; k <= 1; k++)
                {
                    for (int l = - 1; l <=  1; l++)
                    {
                        if (k != 0 || l != 0)
                        {
                            unsigned int thisdircode = 0;

                            if (k == -1)
                            {
                                if (l == -1)
                                    thisdircode = 8;
                                else
                                {
                                    if (l == 0)
                                        thisdircode = 7;
                                    else
                                        thisdircode = 6;
                                }
                            }
                            else
                            {
                                if (k == 0)
                                {
                                    if (l == -1)
                                        thisdircode = 1;
                                    else
                                        thisdircode = 5;
                                }
                                else
                                {
                                    if (l == -1)
                                        thisdircode = 2;
                                    else
                                    {
                                        if (l == 0)
                                            thisdircode = 3;
                                        else
                                            thisdircode = 4;
                                    }
                                }
                            }

                            if (thisnorthpoint.face == face && thisnorthpoint.x == i + k && thisnorthpoint.y == j + l)
                                dircode[index].north = thisdircode;

                            if (thissouthpoint.face == face && thissouthpoint.x == i + k && thissouthpoint.y == j + l)
                                dircode[index].south = thisdircode;

                            if (thiseastpoint.face == face && thiseastpoint.x == i + k && thiseastpoint.y == j + l)
                                dircode[index].east = thisdircode;

                            if (thiswestpoint.face == face && thiswestpoint.x == i + k && thiswestpoint.y == j + l)
                                dircode[index].west = thisdircode;

                        }
                    }
                }
            }
        }
    }

    // Now do the edges.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * eedge;

        for (int n = 0; n < edge; n++)
        {
            for (int m = 0; m < 4; m++)
            {
                int i = 0;
                int j = 0;

                if (m == 0 || m == 1)
                    i = n;

                if (m == 1)
                    j = edge - 1;

                if (m == 2 || m == 3)
                    j = n;

                if (m == 3)
                    i = edge - 1;


                int index = vface + i * edge + j;

                globepoint thisnorthpoint = dirpoint[index].north;
                globepoint thissouthpoint = dirpoint[index].south;
                globepoint thiseastpoint = dirpoint[index].east;
                globepoint thiswestpoint = dirpoint[index].west;

                for (int k = -1; k <= 1; k++)
                {
                    for (int l = -1; l <= 1; l++)
                    {
                        if (k != 0 || l != 0)
                        {
                            globepoint thispoint = getglobepoint(edge, face, i, j, k, l);

                            if (thispoint.face != -1)
                            {
                                unsigned int thisdircode = 0;

                                if (k == -1)
                                {
                                    if (l == -1)
                                        thisdircode = 8;
                                    else
                                    {
                                        if (l == 0)
                                            thisdircode = 7;
                                        else
                                            thisdircode = 6;
                                    }
                                }
                                else
                                {
                                    if (k == 0)
                                    {
                                        if (l == -1)
                                            thisdircode = 1;
                                        else
                                            thisdircode = 5;
                                    }
                                    else
                                    {
                                        if (l == -1)
                                            thisdircode = 2;
                                        else
                                        {
                                            if (l == 0)
                                                thisdircode = 3;
                                            else
                                                thisdircode = 4;
                                        }
                                    }
                                }

                                if (thisnorthpoint.face == thispoint.face && thisnorthpoint.x == thispoint.x && thisnorthpoint.y == thispoint.y)
                                    dircode[index].north = thisdircode;

                                if (thissouthpoint.face == thispoint.face && thissouthpoint.x == thispoint.x && thissouthpoint.y == thispoint.y)
                                    dircode[index].south = thisdircode;

                                if (thiseastpoint.face == thispoint.face && thiseastpoint.x == thispoint.x && thiseastpoint.y == thispoint.y)
                                    dircode[index].east = thisdircode;

                                if (thiswestpoint.face == thispoint.face && thiswestpoint.x == thispoint.x && thiswestpoint.y == thispoint.y)
                                    dircode[index].west = thisdircode;

                            }
                        }
                    }
                }
            }
        }
    }
}

// This gets a globepoint from a code direction (1-8).

globepoint getglobepointfromcode(int edge, int face, int x, int y, int dir)
{
    int rightshift = 0;
    int downshift = 0;

    if (dir == 8 || dir == 1 || dir == 2)
        downshift = -1;

    if (dir == 4 || dir == 5 || dir == 6)
        downshift = 1;

    if (dir == 1 || dir == 2 || dir == 3)
        rightshift = 1;

    if (dir == 6 || dir == 7 || dir == 8)
        rightshift = -1;

    globepoint newpoint = getglobepoint(edge, face, x, y, rightshift, downshift);

    return newpoint;
}

// This gets an intermediate direction.

int getinterdir(int dir1, int dir2)
{
    if (dir1 > 6 && dir2 < 3)
        dir2 = dir2 + 8;

    if (dir2 > 6 && dir1 < 3)
        dir1 = dir1 + 8;

    int newdir = (dir1 + dir2) / 2;

    if (dir2 > 8)
        dir2 = dir2 - 8;

    return dir2;
}

// This finds the approximate distance between two points on the surface. (Slow: uses square roots!)

int getpointdist(int edge, int face1, int x1, int y1, int face2, int x2, int y2)
{
    threefloats point1 = getcubepoint(edge, face1, x1, y1);
    threefloats point2 = getcubepoint(edge, face2, x2, y2);

    float mult = (float)edge / 2.0f;

    point1.x = point1.x * mult;
    point1.y = point1.y * mult;
    point1.z = point1.z * mult;

    point2.x = point2.x * mult;
    point2.y = point2.y * mult;
    point2.z = point2.z * mult;

    float xx = point1.x - point2.x; // This is a rough way of comparing distances, but it's close enough for our purposes and a lot faster than more accurate methods would be.
    float yy = point1.y - point2.y;
    float zz = point1.z - point2.z;

    float distance = xx * xx + yy * yy + zz * zz;

    distance = sqrt(distance);

    return (int)distance;
}

// The same thing, but without square roots.

int getpointdistsquared(int edge, int face1, int x1, int y1, int face2, int x2, int y2)
{
    threefloats point1 = getcubepoint(edge, face1, x1, y1);
    threefloats point2 = getcubepoint(edge, face2, x2, y2);

    float mult = (float)edge / 2.0f;

    point1.x = point1.x * mult;
    point1.y = point1.y * mult;
    point1.z = point1.z * mult;

    point2.x = point2.x * mult;
    point2.y = point2.y * mult;
    point2.z = point2.z * mult;

    float xx = point1.x - point2.x; // This is a rough way of comparing distances, but it's close enough for our purposes and a lot faster than more accurate methods would be.
    float yy = point1.y - point2.y;
    float zz = point1.z - point2.z;

    float distance = xx * xx + yy * yy + zz * zz;

    return (int)distance;
}

// This converts the coordinates of a globepoint into the coordinates of another face.

globepoint getfacepoint(int edge, globepoint startpoint, globepoint destpoint)
{
    if (startpoint.face = destpoint.face)
        return destpoint;

    globepoint convertedpoint = destpoint;

    convertedpoint.face = startpoint.face;

    if (startpoint.face == 0)
    {
        if (destpoint.face == 1)
        {
            convertedpoint.x = convertedpoint.x + edge;
            return convertedpoint;
        }

        if (destpoint.face == 3)
        {
            convertedpoint.x = convertedpoint.x - edge;
            return convertedpoint;
        }

        if (destpoint.face == 2)
        {
            if (destpoint.x < startpoint.x)
                convertedpoint.x = convertedpoint.x + edge * 2;
            else
                convertedpoint.x = convertedpoint.x - edge * 2;

            return convertedpoint;
        }

        if (destpoint.face == 4)
        {
            convertedpoint.y = convertedpoint.y - edge;
            return convertedpoint;
        }

        if (destpoint.face == 5)
        {
            convertedpoint.y = convertedpoint.y + edge;
            return convertedpoint;
        }
    }

    if (startpoint.face == 1)
    {
        if (destpoint.face == 0)
        {
            convertedpoint.x = convertedpoint.x - edge;
            return convertedpoint;
        }

        if (destpoint.face == 2)
        {
            convertedpoint.x = convertedpoint.x + edge;
            return convertedpoint;
        }

        if (destpoint.face == 3)
        {
            if (destpoint.x < startpoint.x)
                convertedpoint.x = convertedpoint.x + edge * 2;
            else
                convertedpoint.x = convertedpoint.x - edge * 2;

            return convertedpoint;
        }

        if (destpoint.face == 4)
        {
            convertedpoint.x = edge - 1 - destpoint.y;
            convertedpoint.y = destpoint.x - edge;

            return convertedpoint;
        }

        if (destpoint.face == 5)
        {
            convertedpoint.x = destpoint.y;
            convertedpoint.y = edge * 2 - destpoint.x;

            return convertedpoint;
        }
    }

    if (startpoint.face == 2)
    {
        if (destpoint.face == 1)
        {
            convertedpoint.x = convertedpoint.x - edge;
            return convertedpoint;
        }

        if (destpoint.face == 3)
        {
            convertedpoint.x = convertedpoint.x + edge;
            return convertedpoint;
        }

        if (destpoint.face == 0)
        {
            if (destpoint.x < startpoint.x)
                convertedpoint.x = convertedpoint.x + edge * 2;
            else
                convertedpoint.x = convertedpoint.x - edge * 2;

            return convertedpoint;
        }

        if (destpoint.face == 4)
        {
            convertedpoint.x = edge - 1 - destpoint.x;
            convertedpoint.y = -1 - destpoint.y;

            return convertedpoint;
        }

        if (destpoint.face == 5)
        {
            convertedpoint.x = edge - 1 - destpoint.x;
            convertedpoint.y = edge * 2 - destpoint.y;

            return convertedpoint;
        }
    }

    if (startpoint.face == 3)
    {
        if (destpoint.face == 2)
        {
            convertedpoint.x = convertedpoint.x - edge;
            return convertedpoint;
        }

        if (destpoint.face == 0)
        {
            convertedpoint.x = convertedpoint.x + edge;
            return convertedpoint;
        }

        if (destpoint.face == 1)
        {
            if (destpoint.x < startpoint.x)
                convertedpoint.x = convertedpoint.x + edge * 2;
            else
                convertedpoint.x = convertedpoint.x - edge * 2;

            return convertedpoint;
        }

        if (destpoint.face == 4)
        {
            convertedpoint.x = destpoint.y;
            convertedpoint.y = -1 - destpoint.x;

            return convertedpoint;
        }

        if (destpoint.face == 5)
        {
            convertedpoint.x = edge - 1 - destpoint.y;
            convertedpoint.y = edge + destpoint.x;

            return convertedpoint;
        }
    }

    if (startpoint.face == 4)
    {
        if (destpoint.face == 0)
        {
            convertedpoint.y = destpoint.y + edge;
            return convertedpoint;
        }

        if (destpoint.face == 1)
        {
            convertedpoint.x = edge + destpoint.y;
            convertedpoint.y = edge - 1 - destpoint.x;

            return convertedpoint;
        }

        if (destpoint.face == 2)
        {
            convertedpoint.x = edge - 1 - destpoint.x;
            convertedpoint.y = -1 - destpoint.y;

            return convertedpoint;
        }

        if (destpoint.face == 3)
        {
            convertedpoint.x = -1 - destpoint.y;
            convertedpoint.y = destpoint.x;

            return convertedpoint;

        }

        if (destpoint.face == 5)
        {
            if (destpoint.y < startpoint.y)
                convertedpoint.y = convertedpoint.y + edge * 2;
            else
                convertedpoint.y = convertedpoint.y - edge * 2;

            return convertedpoint;
        }
    }

    if (startpoint.face == 5)
    {
        if (destpoint.face == 0)
        {
            convertedpoint.y = convertedpoint.y - edge;
            return convertedpoint;
        }

        if (destpoint.face == 1)
        {
            convertedpoint.x = edge * 2 - destpoint.y;
            convertedpoint.y = destpoint.x;

            return convertedpoint;
        }

        if (destpoint.face == 2)
        {
            convertedpoint.x = edge - 1 - destpoint.x;
            convertedpoint.y = edge * 2 - destpoint.y;

            return convertedpoint;
        }

        if (destpoint.face == 3)
        {
            convertedpoint.x = -edge + destpoint.y;
            convertedpoint.y = edge - 1 - destpoint.x;

            return convertedpoint;
        }
    }

    return convertedpoint;
}

// This toggles a bool.

void toggle(bool& val)
{
    if (val == false)
    {
        val = true;
        return;
    }

    val = false;
    return;
}

// This converts a number to a string containing commas. Taken from here: https://9to5answer.com/c-format-number-with-commas

string formatnumber(int val)
{
    string s = to_string(val);

    if (val<1000 && val>-1000)
        return s;
    int n = (int)s.length() - 3;
    while (n > 0)
    {
        s.insert(n, ",");
        n -= 3;
    }

    return s;
}

// This converts a number to a string in the form 1.1k.

string numbertok(int val)
{
    int thousands = val / 1000;

    if (thousands > 9)
    {
        string s = to_string(thousands) + "k";

        return s;
    }

    int remainder = val - thousands * 1000;

    int hundreds = 0;

    if (remainder > 49)
        hundreds = 1;

    if (remainder > 149)
        hundreds = 2;

    if (remainder > 249)
        hundreds = 3;

    if (remainder > 349)
        hundreds = 4;

    if (remainder > 449)
        hundreds = 5;

    if (remainder > 549)
        hundreds = 6;

    if (remainder > 649)
        hundreds = 7;

    if (remainder > 749)
        hundreds = 8;

    if (remainder > 849)
        hundreds = 9;





    string s = to_string(thousands) + "." + to_string(hundreds) + "k";

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

// This function saves information about the world about to be generated.

void saverecord(planet& world, string filename)
{
    string degree = "\xC2\xB0";

    string sizeinfo = "Size: ";

    string sizevalue = "";

    if (world.size() == 0)
        sizevalue = "small";

    if (world.size() == 1)
        sizevalue = "medium";

    if (world.size() == 2)
        sizevalue = "large";

    stringstream ss3;
    ss3 << fixed << setprecision(5) << world.eccentricity();

    string eccentricityinfo = "Eccentricity: ";
    string eccentricityvalue = ss3.str();

    stringstream ss4;
    ss4 << fixed << setprecision(2) << world.gravity();

    string gravityinfo = "Surface gravity: ";
    string gravityvalue = ss4.str() + "g";

    string perihelioninfo = "Perihelion: ";
    string perihelionvalue = "";

    if (world.perihelion() == 0)
        perihelionvalue = "January";

    if (world.perihelion() == 1)
        perihelionvalue = "July";

    stringstream ss5;
    ss5 << fixed << setprecision(2) << world.lunar();

    string lunarinfo = "Lunar pull: ";
    string lunarvalue = ss5.str();

    string typeinfo = "Category: ";
    string typevalue = to_string(world.type());

    if (typevalue == "1")
        typevalue = "tectonic (small)";

    if (typevalue == "2")
        typevalue = "tectonic (large)";

    if (typevalue == "3")
        typevalue = "oceanic";

    if (typevalue == "4")
        typevalue = "non-tectonic";

    string rotationinfo = "Rotation: ";
    string rotationvalue = "";

    if (world.rotation())
        rotationvalue = "west to east";
    else
        rotationvalue = "east to west";

    stringstream ss;
    ss << fixed << setprecision(2) << world.tilt();

    string tiltinfo = "Obliquity: ";
    string tiltvalue = ss.str() + degree;

    stringstream ss2;
    ss2 << fixed << setprecision(2) << world.tempdecrease();

    string tempdecreaseinfo = "Heat decrease per vertical km: ";
    string tempdecreasevalue = ss2.str() + degree;

    string averageinfo = "Average global temperature: ";
    string averagevalue = to_string(world.averagetemp()) + degree;

    stringstream ss6;
    ss6 << fixed << setprecision(2) << world.waterpickup();

    string moistureinfo = "Moisture pickup rate: ";
    string moisturevalue = ss6.str();

    string glacialinfo = "Glaciation temperature: ";
    string glacialvalue = to_string(world.glacialtemp()) + degree;

    ofstream outfile;
    outfile.open(filename, ios_base::app);

    string timestring = format("{:%F %T %Z}", chrono::zoned_time{ chrono::current_zone(), chrono::time_point_cast<chrono::seconds>(chrono::system_clock::now()) });

    //string timestring; // = format("{:%T}", chrono::zoned_time{ chrono::current_zone(), chrono::time_point_cast<chrono::seconds>(chrono::system_clock::now()) });

    outfile << "\nTime started: " << timestring << '\n';
    outfile << "Seed: " << world.seed() << '\n';
    outfile << sizeinfo << sizevalue << '\n';
    outfile << typeinfo << typevalue << '\n';
    outfile << rotationinfo << rotationvalue << '\n';
    outfile << perihelioninfo << perihelionvalue << '\n';
    outfile << eccentricityinfo << eccentricityvalue << '\n';
    outfile << tiltinfo << tiltvalue << '\n';
    outfile << gravityinfo << gravityvalue << '\n';
    outfile << lunarinfo << lunarvalue << '\n';
    outfile << moistureinfo << moisturevalue << '\n';
    outfile << tempdecreaseinfo << tempdecreasevalue << '\n';
    outfile << averageinfo << averagevalue << '\n';
    outfile << glacialinfo << glacialvalue << '\n';

    outfile.close();
}

// This adds the time completed.

void saveendtime(planet& world, string filename)
{
    ofstream outfile;
    outfile.open(filename, ios_base::app);

    //string timestring = format("{:%T}", chrono::zoned_time{ chrono::current_zone(), chrono::time_point_cast<chrono::seconds>(chrono::system_clock::now()) });

    string timestring = format("{:%F %T %Z}", chrono::zoned_time{ chrono::current_zone(), chrono::time_point_cast<chrono::seconds>(chrono::system_clock::now()) });

    outfile << "Time completed: " << timestring << '\n';

    outfile.close();
}
// This function saves settings.

void savesettings(planet& world, string filename)
{
    ofstream outfile;
    outfile.open(filename, ios::out);

    outfile << world.settingssaveversion() << '\n';
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

// This saves control bindings.

void savecontrols(vector<int>& keybindings, int length, float& camerazoomspeed, float& camerarotatespeed, float& planetrotatespeed, float& monthspeed, bool& cameralock, bool& monthrotate, string filename)
{
    ofstream outfile;
    outfile.open(filename, ios::out, ios::trunc);

    for (int n = 0; n < length; n++)
        outfile << keybindings[n] << '\n';

    outfile << camerazoomspeed << '\n';
    outfile << camerarotatespeed << '\n';
    outfile << planetrotatespeed << '\n';
    outfile << monthspeed << '\n';
    outfile << cameralock << '\n';
    outfile << monthrotate << '\n';

    outfile.close();
}

// This loads control bindings.

void loadcontrols(vector<int>& keybindings, int length, float& camerazoomspeed, float& camerarotatespeed, float& planetrotatespeed, float& monthspeed, bool& cameralock, bool& monthrotate, string filename)
{
    ifstream infile;
    infile.open(filename, ios::in);

    if (infile.is_open())
    {
        string line;
        int val;
        float fval;
        bool bval;

        for (int n = 0; n < length; n++)
        {
            getline(infile, line);
            val = stoi(line);

            keybindings[n] = val;
        }

        getline(infile, line);
        fval = stof(line);

        camerazoomspeed = fval;

        getline(infile, line);
        fval = stof(line);

        camerarotatespeed = fval;

        getline(infile, line);
        fval = stof(line);

        planetrotatespeed = fval;

        getline(infile, line);
        fval = stof(line);

        monthspeed = fval;

        getline(infile, line);
        bval = stob(line);

        cameralock = bval;

        getline(infile, line);
        bval = stob(line);

        monthrotate = bval;
    }
    else // If there's no save file already, just use the default bindings.
    {
        initialisekeybindings(keybindings);
        initialisemovementsettings(camerazoomspeed, camerarotatespeed, planetrotatespeed, monthspeed, cameralock, monthrotate);
    }
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

    return newval;
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

// This function shifts everything in a vector to the left by a given number of pixels.

void shift(vector<vector<int>>& map, int width, int height, int offset)
{
    vector<vector<int>> dummy(ARRAYWIDTH, vector<int>(ARRAYWIDTH, 0));

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
    vector<vector<int>> dummy(ARRAYWIDTH, vector<int>(ARRAYWIDTH, 0));

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

// Same thing, for a cubesphere vector.

void smooth(vector<vector<vector<int>>>& arr, int edge, int maxelev, int amount)
{
    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                float crount = 0.0f;
                float ave = 0.0f;

                for (int k = -amount; k <= amount; k++)
                {
                    for (int l = -amount; l <= amount; l++)
                    {
                        globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                        if (lpoint.face != -1)
                        {
                            ave = ave + (float)arr[lpoint.face][lpoint.x][lpoint.y];
                            crount++;
                        }
                    }
                }

                ave = ave / crount;

                if (ave > 0.0f && ave < (float)maxelev)
                    arr[face][i][j] = (int)ave;
            }
        }
    }
}

void smooth(vector<int>& arr, int edge, int maxelev, int amount)
{
    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i;

            for (int j = 0; j < edge; j++)
            {
                float crount = 0.0f;
                float ave = 0.0f;

                for (int k = -amount; k <= amount; k++)
                {
                    for (int l = -amount; l <= amount; l++)
                    {
                        globepoint lpoint = getglobepoint(edge, face, i, j, k, l);

                        if (lpoint.face != -1)
                        {
                            ave = ave + (float)arr[lpoint.face * edge * edge + lpoint.x * edge + lpoint.y];
                            crount++;
                        }
                    }
                }

                ave = ave / crount;

                if (ave > 0.0f && ave < (float)maxelev)
                    arr[vface + j] = (int)ave;
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
            if (j >= 0 && j < ARRAYWIDTH)
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

// This function warps a vector by the specified amount, creating a more natural look.

void warp(vector<int>& map, int edge, int maxelev, int warpfactorright, int warpfactordown, int warpdetail, bool vary, vector<fourglobepoints>& dirpoint)
{
    vector<int> dest(6 * edge * edge, 0);

    int grain = warpdetail; // Level of detail on this fractal map.
    float valuemod = 0.2f;
    int v = random(3, 6);
    float valuemod2 = (float)v;

    vector<int> warpright(6 * edge * edge, 0);
    vector<int> warpdown(6 * edge * edge, 0);

    createfractal(warpright, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);
    createfractal(warpdown, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    // Now create another one to vary the strength of this effect (if needed).

    vector<int> broad(6 * edge * edge, 0);

    grain = 4;
    valuemod = 0.002f;
    valuemod2 = 0.002f;

    if (vary)
        createfractal(broad, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                int x = i;
                int y = j;

                float thiswarpright = (float)warpright[index] / (float)maxelev;
                float thiswarpdown = (float)warpdown[index] / (float)maxelev;

                thiswarpright = thiswarpright - 0.5f; // So they're in the range -0.5 - 0.5.
                thiswarpdown = thiswarpdown - 0.5f;

                thiswarpright = thiswarpright * (float)warpfactorright;
                thiswarpdown = thiswarpdown * (float)warpfactordown;

                if (vary)
                {
                    float varyfactor = (float)broad[index] / (float)maxelev;

                    thiswarpright = thiswarpright * varyfactor;
                    thiswarpdown = thiswarpdown * varyfactor;
                }

                globepoint warppoint;

                warppoint.face = face;
                warppoint.x = i;
                warppoint.y = j;

                bool goleft = 0;

                if (thiswarpright < 0.0f)
                {
                    goleft = 1;
                    thiswarpright = 0.0f - thiswarpright;
                }

                bool goup = 0;

                if (thiswarpdown < 0.0f)
                {
                    goup = 1;
                    thiswarpdown = 0.0f - thiswarpdown;
                }

                for (float n = 0.0f; n < thiswarpright; n++)
                {
                    if (goleft)
                        warppoint = dirpoint[warppoint.face * edge * edge + warppoint.x * edge + warppoint.y].west;
                    else
                        warppoint = dirpoint[warppoint.face * edge * edge + warppoint.x * edge + warppoint.y].east;
                }

                for (float n = 0.0f; n < thiswarpdown; n++)
                {
                    if (goup)
                        warppoint = dirpoint[warppoint.face * edge * edge + warppoint.x * edge + warppoint.y].north;
                    else
                        warppoint = dirpoint[warppoint.face * edge * edge + warppoint.x * edge + warppoint.y].south;
                }

                int val = map[warppoint.face * edge * edge + warppoint.x * edge + warppoint.y];

                dest[index] = val;
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

                map[index] = dest[index];
            }
        }
    }
}

// Deprecated version of the above, for 3D vectors.

void warpold(vector<vector<vector<int>>>& map, int edge, int maxelev, int warpfactorright, int warpfactordown, int warpdetail, bool vary, vector<fourglobepoints>& dirpoint)
{
    vector<vector<vector<int>>> dest(6, vector<vector<int>>(edge, vector<int>(edge, 0)));

    int grain = warpdetail; // Level of detail on this fractal map.
    float valuemod = 0.2f;
    int v = random(3, 6);
    float valuemod2 = (float)v;

    vector<vector<vector<int>>> warpright(6, vector<vector<int>>(edge, vector<int>(edge, 0)));
    vector<vector<vector<int>>> warpdown(6, vector<vector<int>>(edge, vector<int>(edge, 0)));

    createfractalold(warpright, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);
    createfractalold(warpdown, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    // Now create another one to vary the strength of this effect (if needed).

    vector<vector<vector<int>>> broad(6, vector<vector<int>>(edge, vector<int>(edge, 0)));

    grain = 4;
    valuemod = 0.002f;
    valuemod2 = 0.002f;

    if (vary)
        createfractalold(broad, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                int x = i;
                int y = j;

                float thiswarpright = (float)warpright[face][i][j] / (float)maxelev;
                float thiswarpdown = (float)warpdown[face][i][j] / (float)maxelev;

                thiswarpright = thiswarpright - 0.5f; // So they're in the range -0.5 - 0.5.
                thiswarpdown = thiswarpdown - 0.5f;

                thiswarpright = thiswarpright * (float)warpfactorright;
                thiswarpdown = thiswarpdown * (float)warpfactordown;

                if (vary)
                {
                    float varyfactor = (float)broad[face][i][j] / (float)maxelev;

                    thiswarpright = thiswarpright * varyfactor;
                    thiswarpdown = thiswarpdown * varyfactor;
                }

                globepoint warppoint;

                warppoint.face = face;
                warppoint.x = i;
                warppoint.y = j;

                bool goleft = 0;

                if (thiswarpright < 0.0f)
                {
                    goleft = 1;
                    thiswarpright = 0.0f - thiswarpright;
                }

                bool goup = 0;

                if (thiswarpdown < 0.0f)
                {
                    goup = 1;
                    thiswarpdown = 0.0f - thiswarpdown;
                }

                for (float n = 0.0f; n < thiswarpright; n++)
                {
                    if (goleft)
                        warppoint = dirpoint[warppoint.face * edge * edge + warppoint.x * edge + warppoint.y].west;
                    else
                        warppoint = dirpoint[warppoint.face * edge * edge + warppoint.x * edge + warppoint.y].east;
                }

                for (float n = 0.0f; n < thiswarpdown; n++)
                {
                    if (goup)
                        warppoint = dirpoint[warppoint.face * edge * edge + warppoint.x * edge + warppoint.y].north;
                    else
                        warppoint = dirpoint[warppoint.face * edge * edge + warppoint.x * edge + warppoint.y].south;
                }

                int val = map[warppoint.face][warppoint.x][warppoint.y];

                dest[face][i][j] = val;
            }
        }
    }

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                map[face][i][j] = dest[face][i][j];
            }
        }
    }
}

// This does the same thing, but a "monstrous" warp - http://thingonitsown.blogspot.com/2018/11/monstrous-terrain.html

void monstrouswarp(vector<int>& map, int edge, int maxelev, int warpfactor)
{
    vector<int> dest(6 * edge * edge);

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                float point = (float)map[index];

                point = point / (float)maxelev; // Makes it into a value from 0.0 to 1.0

                point = point * (float)warpfactor;

                if (point >= (float)edge)
                    point = (float)(edge - 1);

                int ppoint = (int)point;

                int val = map[ppoint * ppoint + ppoint];

                dest[index] = val;
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

                map[index] = dest[index];
            }
        }
    }
}

// Deprecated version of the above, for 3D vectors.

void monstrouswarpold(vector<vector<vector<int>>>& map, int edge, int maxelev, int warpfactor)
{
    vector<vector<vector<int>>> dest(6, vector<vector<int>>(edge, vector<int>(edge, 0)));

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                float point = (float)map[face][i][j];

                point = point / (float)maxelev; // Makes it into a value from 0.0 to 1.0

                point = point * (float)warpfactor;

                if (point >= (float)edge)
                    point = (float)(edge - 1);

                int val = map[0][(int)point][(int)point];

                dest[face][i][j] = val;
            }
        }
    }

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                map[face][i][j] = dest[face][i][j];
            }
        }
    }
}

// This function creates a voronoi map.

void makevoronoi(vector<vector<vector<int>>>& voronoi, int edge, int points)
{
    vector<threeintegers> pointslist(points);

    for (int n = 0; n < points; n++)
    {
        int face = random(0, 5);
        int x = random(0, edge - 1);
        int y = random(0, edge - 1);

        threefloats point = getcubepoint(edge, face, x, y);

        pointslist[n].x = (int)(point.x * 1000.0f);
        pointslist[n].y = (int)(point.y * 1000.0f);
        pointslist[n].z = (int)(point.z * 1000.0f);
    }

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                int closest = 0;
                int dist = 9999999;

                threefloats thispoint = getcubepoint(edge, face, i, j);

                thispoint.x = thispoint.x * 1000.0f;
                thispoint.y = thispoint.y * 1000.0f;
                thispoint.z = thispoint.z * 1000.0f;

                for (int point = 0; point < points; point++)
                {
                    int xx = (int)(thispoint.x - pointslist[point].x); // This is a rough way of comparing distances, but it's close enough for our purposes and a lot faster than more accurate methods would be.
                    int yy = (int)(thispoint.y - pointslist[point].y);
                    int zz = (int)(thispoint.z - pointslist[point].z);

                    int pointdist = xx * xx + yy * yy + zz * zz;

                    if (pointdist < dist)
                    {
                        closest = point;
                        dist = pointdist;
                    }
                }

                voronoi[face][i][j] = closest;
            }
        }
    }
}

// This does the same thing, but with a more even distribution of points (ideally close together - pointdist < 10).

void makeregularvoronoi(vector<int>& voronoi, int edge, int pointdist)
{
    int eedge = edge * edge;

    vector<int> pointsmap(6 * edge * edge, 0);

    int pointnumber = 1;
    int movedist = (int)(((float)pointdist / 4.0f) * 3.0f);

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i = i + pointdist)
        {
            for (int j = 0; j < edge; j = j + pointdist)
            {
                globepoint thispoint = getglobepoint(edge, face, i, j, randomsign(random(1, movedist)), randomsign(random(1, movedist)));

                if (thispoint.face == -1)
                {
                    thispoint.face = face;
                    thispoint.x = i;
                    thispoint.y = j;
                }

                pointsmap[thispoint.face * eedge + thispoint.x * edge + thispoint.y] = pointnumber;
                pointnumber++;
            }
        }
    }

    float fpointdist = (float)pointdist;

    for (int face = 0; face < 6; face++)
    {
        int vface = face * eedge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

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

                int closest = 0;
                int dist = 9999999;

                threefloats thispoint = getcubepoint(edge, face, i, j);

                thispoint.x = thispoint.x * 1000.0f;
                thispoint.y = thispoint.y * 1000.0f;
                thispoint.z = thispoint.z * 1000.0f;

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

                                if (pointsmap[tindex] != 0)
                                {
                                    threefloats newcubepoint = getcubepoint(edge, thisface, x, y);

                                    int xx = (int)(thispoint.x - newcubepoint.x * 1000.0f); // This is a rough way of comparing distances, but it's close enough for our purposes and a lot faster than more accurate methods would be.
                                    int yy = (int)(thispoint.y - newcubepoint.y * 1000.0f);
                                    int zz = (int)(thispoint.z - newcubepoint.z * 1000.0f);

                                    int pointdist = xx * xx + yy * yy + zz * zz;

                                    if (pointdist < dist)
                                    {
                                        closest = pointsmap[tindex];
                                        dist = pointdist;
                                    }
                                }
                            }
                        }
                    }
                }

                voronoi[index] = closest;
            }
        }
    }
}

// This function creates a voronoi map (on a 2D array, not a cube).

void make2Dvoronoi(vector<vector<int>>& voronoi, int width, int height, int points)
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

// This function makes Worley noise.

void makeworley(vector<int>& worley, int edge, int pointdist, int max, float dropoff)
{
    int eedge = edge * edge;

    int reduce = (int)((float)max * dropoff); // Amount to reduce for each step away from the points.

    vector<int> pointsmap(6 * edge * edge, 0);

    int pointnumber = 1;
    int movedist = (int)(((float)pointdist / 4.0f) * 3.0f);

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i = i + pointdist)
        {
            for (int j = 0; j < edge; j = j + pointdist)
            {
                globepoint thispoint = getglobepoint(edge, face, i, j, randomsign(random(1, movedist)), randomsign(random(1, movedist)));

                if (thispoint.face == -1)
                {
                    thispoint.face = face;
                    thispoint.x = i;
                    thispoint.y = j;
                }

                pointsmap[thispoint.face * eedge + thispoint.x * edge + thispoint.y] = pointnumber;
                pointnumber++;
            }
        }
    }

    float fpointdist = (float)pointdist;

    for (int face = 0; face < 6; face++)
    {
        int vface = face * eedge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

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

                int closest = 0;
                int dist = 9999999;

                threefloats thispoint = getcubepoint(edge, face, i, j);

                thispoint.x = thispoint.x * 1000.0f;
                thispoint.y = thispoint.y * 1000.0f;
                thispoint.z = thispoint.z * 1000.0f;

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

                                if (pointsmap[tindex] != 0)
                                {
                                    threefloats newcubepoint = getcubepoint(edge, thisface, x, y);

                                    int xx = (int)(thispoint.x - newcubepoint.x * 1000.0f); // This is a rough way of comparing distances, but it's close enough for our purposes and a lot faster than more accurate methods would be.
                                    int yy = (int)(thispoint.y - newcubepoint.y * 1000.0f);
                                    int zz = (int)(thispoint.z - newcubepoint.z * 1000.0f);

                                    int pointdist = xx * xx + yy * yy + zz * zz;

                                    if (pointdist < dist)
                                    {
                                        closest = pointsmap[tindex];
                                        dist = pointdist;
                                    }
                                }
                            }
                        }
                    }
                }

                int thisdist = (int)(sqrt((float)dist));

                int val = max - thisdist * reduce; 

                if (val < 0)
                    val = 0;

                worley[index] = val;
            }
        }
    }
}

// This covers an array with mounds. (Gives a pseudo-Worley effect, but much more quickly.)

void makemounds(int edge, vector<int>& arr, int totalmounds, float maxelev)
{
    int eedge = edge * edge;

    int maxblank = (int)(maxelev * 0.1f);

    for (int n = 0; n < totalmounds; n++)
    {
        int centreface = random(0, 5);

        int centrex = random(0, edge - 1);
        int centrey = random(0, edge - 1);
        
        if (arr[centreface * eedge + centrex * edge + centrey] < maxblank)
        {
            int radius = random(40, 100);

            float elevadd = maxelev / (float)radius;

            elevadd = elevadd * 2.0f;

            float thiselev = 0;

            for (int n = radius; n > 0; n--)
            {
                thiselev = thiselev + elevadd;

                if (thiselev > maxelev)
                    thiselev = maxelev;

                for (int i = -n; i <= n; i++)
                {
                    for (int j = -n; j <= n; j++)
                    {
                        if (i * i + j * j < n * n)
                        {
                            globepoint thispoint = getglobepoint(edge, centreface, centrex, centrey, i, j);

                            if (thispoint.face != -1)
                            {
                                int tindex = thispoint.face * eedge + thispoint.x * edge + thispoint.y;

                                if (arr[tindex] < (int)thiselev)
                                    arr[tindex] = (int)thiselev;
                            }
                        }
                    }
                }

                elevadd = elevadd * 0.95f;
            }
        }
    }
}

// This uses curl noise from one array to warp another. (Source is the original. It warps using warpvector, and writes to dest. Varyvector varies the strength of the effect.)

void curlwarp(int edge, float maxelev, vector<int>& source, vector<int>& warpvector, vector<int>& varyvector, vector<fourglobepoints>& dirpoint)
{
    int eedge = edge * edge;

    vector<int> dest(6 * edge * edge, 0);

    /*
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

                int biggest = 0;

                int xoffset = 0;
                int yoffset = 0;

                // First, check along the northern and southern borders of the square that our point is in the middle of.

                for (int k = -dist; k <= dist; k++)
                {
                    globepoint thispoint;
                    thispoint.face = face;
                    thispoint.x = i;
                    thispoint.y = j;

                    // First, get the value of this point on the warp vector.

                    int kk = k;

                    bool gowest = 0;

                    if (k < 0)
                    {
                        kk = -kk;
                        gowest = 1;
                    }

                    for (int n = 0; n < kk; n++)
                    {
                        if (gowest)
                            thispoint = dirpoint[thispoint.face * eedge + thispoint.x * edge + thispoint.y].west;
                        else
                            thispoint = dirpoint[thispoint.face * eedge + thispoint.x * edge + thispoint.y].east;
                    }

                    for (int n=0;n<dist;n++)
                        thispoint = dirpoint[thispoint.face * eedge + thispoint.x * edge + thispoint.y].north;

                    int point1 = warpvector[thispoint.face * eedge + thispoint.x * edge + thispoint.y];

                    // Now get the value at the opposite point.

                    globepoint thatpoint;

                    thatpoint.face = face;
                    thatpoint.x = i;
                    thatpoint.y = j;

                    for (int n = 0; n < kk; n++) // Opposite direction!
                    {
                        if (gowest)
                            thatpoint = dirpoint[thatpoint.face * eedge + thatpoint.x * edge + thatpoint.y].east;
                        else
                            thatpoint = dirpoint[thatpoint.face * eedge + thatpoint.x * edge + thatpoint.y].west;
                    }

                    for (int n = 0; n < dist; n++)
                        thatpoint = dirpoint[thatpoint.face * eedge + thatpoint.x * edge + thatpoint.y].south;

                    int point2 = warpvector[thatpoint.face * eedge + thatpoint.x * edge + thatpoint.y];

                    int diff = point1 - point2;

                    if (point2 > point1)
                        diff = point2 - point1;

                    if (diff > biggest)
                    {
                        biggest = diff;

                        if (point1 > point2)
                        {
                            xoffset = k;
                            yoffset = -dist;
                        }
                        else
                        {
                            xoffset = -k;
                            yoffset = dist;
                        }
                    }
                }

                // Now the western and eastern borders.

                for (int k = -dist; k <= dist; k++)
                {
                    globepoint thispoint;
                    thispoint.face = face;
                    thispoint.x = i;
                    thispoint.y = j;

                    // First, get the value of this point on the warp vector.

                    int kk = k;

                    bool gonorth = 0;

                    if (k < 0)
                    {
                        kk = -kk;
                        gonorth = 1;
                    }

                    for (int n = 0; n < kk; n++)
                    {
                        if (gonorth)
                            thispoint = dirpoint[thispoint.face * eedge + thispoint.x * edge + thispoint.y].north;
                        else
                            thispoint = dirpoint[thispoint.face * eedge + thispoint.x * edge + thispoint.y].south;
                    }

                    for (int n = 0; n < dist; n++)
                        thispoint = dirpoint[thispoint.face * eedge + thispoint.x * edge + thispoint.y].west;

                    int point1 = warpvector[thispoint.face * eedge + thispoint.x * edge + thispoint.y];

                    // Now get the value at the opposite point.

                    globepoint thatpoint;

                    thatpoint.face = face;
                    thatpoint.x = i;
                    thatpoint.y = j;

                    for (int n = 0; n < kk; n++) // Opposite direction!
                    {
                        if (gonorth)
                            thatpoint = dirpoint[thatpoint.face * eedge + thatpoint.x * edge + thatpoint.y].south;
                        else
                            thatpoint = dirpoint[thatpoint.face * eedge + thatpoint.x * edge + thatpoint.y].north;
                    }

                    for (int n = 0; n < dist; n++)
                        thatpoint = dirpoint[thatpoint.face * eedge + thatpoint.x * edge + thatpoint.y].east;

                    int point2 = warpvector[thatpoint.face * eedge + thatpoint.x * edge + thatpoint.y];

                    int diff = point1 - point2;

                    if (point2 > point1)
                        diff = point2 - point1;

                    if (diff > biggest)
                    {
                        biggest = diff;

                        if (point1 > point2)
                        {
                            xoffset = -dist;
                            yoffset = k;
                        }
                        else
                        {
                            xoffset = dist;
                            yoffset = -k;
                        }
                    }
                }

                // Now use that to work out where we need to look on the source array.

                float strength = (float)biggest / maxelev;

                strength = strength * 50.0f; // Just to make it more noticeable.

                xoffset = (int)((float)xoffset * strength);
                yoffset = (int)((float)yoffset * strength);

                int n = xoffset;
                xoffset = -yoffset;
                yoffset = n;

                globepoint thispoint;
                thispoint.face = face;
                thispoint.x = i;
                thispoint.y = j;

                bool gowest = 0;
                bool gonorth = 0;

                int k = xoffset;

                if (k < 0)
                {
                    k = -k;
                    gowest = 1;
                }

                int l = yoffset;

                if (l < 0)
                {
                    l = -l;
                    gonorth = 1;
                }

                for (int n = 0; n < k; n++)
                {
                    if (gowest)
                        thispoint = dirpoint[thispoint.face * eedge + thispoint.x * edge + thispoint.y].west;
                    else
                        thispoint = dirpoint[thispoint.face * eedge + thispoint.x * edge + thispoint.y].east;
                }

                for (int n = 0; n < l; n++)
                {
                    if (gonorth)
                        thispoint = dirpoint[thispoint.face * eedge + thispoint.x * edge + thispoint.y].north;
                    else
                        thispoint = dirpoint[thispoint.face * eedge + thispoint.x * edge + thispoint.y].south;
                }

                dest[index] = source[thispoint.face * eedge + thispoint.x * edge + thispoint.y];

            }
        }
    }
    */

    int delta = 4;

    for (int face = 0; face < 6; face++)
    {
        int vface = face * eedge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                globepoint thispoint;
                thispoint.face = face;
                thispoint.x = i;
                thispoint.y = j;

                for (int n = 0; n < delta; n++)
                    thispoint = dirpoint[thispoint.face * eedge + thispoint.x * edge + thispoint.y].west;

                float n1 = (float)warpvector[thispoint.face * eedge + thispoint.x * edge + thispoint.y];

                thispoint.face = face;
                thispoint.x = i;
                thispoint.y = j;

                for (int n = 0; n < delta; n++)
                    thispoint = dirpoint[thispoint.face * eedge + thispoint.x * edge + thispoint.y].east;

                float n2 = (float)warpvector[thispoint.face * eedge + thispoint.x * edge + thispoint.y];

                thispoint.face = face;
                thispoint.x = i;
                thispoint.y = j;

                for (int n = 0; n < delta; n++)
                    thispoint = dirpoint[thispoint.face * eedge + thispoint.x * edge + thispoint.y].north;

                float n3 = (float)source[thispoint.face * eedge + thispoint.x * edge + thispoint.y];

                thispoint.face = face;
                thispoint.x = i;
                thispoint.y = j;

                for (int n = 0; n < delta; n++)
                    thispoint = dirpoint[thispoint.face * eedge + thispoint.x * edge + thispoint.y].south;

                float n4 = (float)source[thispoint.face * eedge + thispoint.x * edge + thispoint.y];

                float strength = (float)varyvector[index] / maxelev;

                strength = strength * 60.f;

                int a = (int)((n2 - n1) / maxelev * strength);
                int b = (int)((n4 - n3) / maxelev * strength);

                int warpx = b;
                int warpy = -a;

                thispoint.face = face;
                thispoint.x = i;
                thispoint.y = j;

                if (warpx > 0)
                {
                    for (int n = 0; n < warpx; n++)
                        thispoint = dirpoint[thispoint.face * eedge + thispoint.x * edge + thispoint.y].east;
                }
                else
                {
                    for (int n = 0; n > warpx; n--)
                        thispoint = dirpoint[thispoint.face * eedge + thispoint.x * edge + thispoint.y].west;
                }

                if (warpy > 0)
                {
                    for (int n = 0; n < warpy; n++)
                        thispoint = dirpoint[thispoint.face * eedge + thispoint.x * edge + thispoint.y].south;
                }
                else
                {
                    for (int n = 0; n > warpy; n--)
                        thispoint = dirpoint[thispoint.face * eedge + thispoint.x * edge + thispoint.y].north;
                }

                dest[index] = source[thispoint.face * eedge + thispoint.x * edge + thispoint.y];
            }
        }
    }

    for (int face = 0; face < 6; face++)
    {
        int vface = face * eedge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                source[index] = dest[index];
            }
        }
    }
}

// Draws a shape from an image onto a vector.

void drawvectorshape(int edge, int shapenumber, int face, int centrex, int centrey, int val, vector<vector<vector<int>>>& drawmap, boolshapetemplate shape[])
{
    int imheight = shape[shapenumber].ysize() - 1;
    int imwidth = shape[shapenumber].xsize() - 1;

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
                    drawmap[thispoint.face][thispoint.x][thispoint.y] = val;
            }
        }
    }
}

// The same thing, but on a boolean vector.

void drawvectorshape(planet& world, int edge, int shapenumber, int face, int centrex, int centrey, bool val, vector<vector<vector<bool>>>& drawmap, boolshapetemplate shape[])
{
    int imheight = shape[shapenumber].ysize() - 1;
    int imwidth = shape[shapenumber].xsize() - 1;

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
                {
                    drawmap[thispoint.face][thispoint.x][thispoint.y] = val;
                    world.settest(thispoint.face, thispoint.x, thispoint.y, 100);
                }
            }
        }
    }
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

// This finds the coordinates of the nearest sea (or land, if land=1) to the point specified. It looks on only the current face.

twointegers nearestsea(planet& world, int face, int i, int j, bool land, int limit, int grain)
{
    int edge = world.edge();

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
            if (x >= 0 && x < edge)
            {
                for (int y = j - checkdist; y <= j + checkdist; y++)
                {
                    if (y >= 0 && y < edge)
                    {
                        if ((x - i) * (x - i) + (y - j) * (y - j) == rr) // if this point is on the circle we're looking at
                        {
                            if (world.sea(face, x, y) == 1 && land == 0) // And if this point is sea
                            {
                                distance = checkdist;
                                seax = x;
                                seay = y;
                            }

                            if (world.sea(face, x, y) == 0 && land == 1) // And if this point is land
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
        if (checkdist > limit) // We are surely not going to find any now...
        {
            distance = edge * 2;
            seax = -1;
            seay = -1;
        }
    }

    seapoint.x = seax;
    seapoint.y = seay;

    return (seapoint);
}

// This finds the coordinates of the nearest point==1 to the point specified, on the given array.

twointegers nearestpoint(vector<bool>& arr, int width, int height, int i, int j, int limit, int grain)
{
    twointegers point;

    point.x = -1;
    point.y = -1;

    if (grain < 1)
        grain = 1;

    int distance = -1; // Distance to nearest point.
    int pointx = -1; // x coordinate of nearest point.
    int pointy = -1; // y coordinate of nearest point.
    int checkdist = 1; // Distance that we're checking to see if it == 1.

    while (distance == -1)
    {
        checkdist = checkdist + grain; // Increase the size of circle that we're checking

        int rr = checkdist * checkdist;

        for (int x = i - checkdist; x <= i + checkdist; x++)
        {
            int xx = x;

            if (xx<0 || xx>width)
                xx = wrap(xx, width);

            int vx = xx * (height + 1);

            for (int y = j - checkdist; y <= j + checkdist; y++)
            {
                if (y >= 0 && y <= height)
                {
                    if ((x - i) * (x - i) + (y - j) * (y - j) == rr) // if this point is on the circle we're looking at
                    {
                        if (arr[vx + y]) // And if this point == 1
                        {
                            distance = checkdist;
                            pointx = xx;
                            pointy = y;
                        }
                    }
                }
            }
        }

        if (checkdist > limit) // We are surely not going to find any now...
        {
            point.x = -1;
            point.y = -1;

            return (point);
        }
    }

    point.x = pointx;
    point.y = pointy;

    return (point);
}

// Deprecated version.

twointegers nearestpointold(vector<vector<bool>>& arr, int width, int height, int i, int j, int limit, int grain)
{
    twointegers point;

    point.x = -1;
    point.y = -1;

    if (grain < 1)
        grain = 1;

    int distance = -1; // Distance to nearest point.
    int pointx = -1; // x coordinate of nearest point.
    int pointy = -1; // y coordinate of nearest point.
    int checkdist = 1; // Distance that we're checking to see if it == 1.

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
                    if ((x - i) * (x - i) + (y - j) * (y - j) == rr) // if this point is on the circle we're looking at
                    {
                        if (arr[xx][y]) // And if this point==1
                        {
                            distance = checkdist;
                            pointx = xx;
                            pointy = y;
                        }
                    }
                }
            }
        }

        if (checkdist > limit) // We are surely not going to find any now...
        {
            point.x = -1;
            point.y = -1;

            return (point);
        }
    }

    point.x = pointx;
    point.y = pointy;

    return (point);
}

// This tells whether the given point is coast (land next to sea).

bool coast(planet& world, int face, int x, int y)
{
    if (world.sea(face, x, y) == 1)
        return 0;

    int edge = world.edge();

    globepoint north = getglobepoint(edge, face, x, y, 0, -1);

    if (world.sea(north.face, north.x, north.y) == 1)
        return 1;

    globepoint south = getglobepoint(edge, face, x, y, 0, 1);

    if (world.sea(south.face, south.x, south.y) == 1)
        return 1;

    globepoint east = getglobepoint(edge, face, x, y, 1, 0);

    if (world.sea(east.face, east.x, east.y) == 1)
        return 1;

    globepoint west = getglobepoint(edge, face, x, y, -1, 0);

    if (world.sea(west.face, west.x, west.y) == 1)
        return 1;

    return 0;
}

// This function checks whether a global tile is either sea next to land or land next to sea.

bool vaguelycoastal(planet& world, int face, int x, int y)
{
    int edge = world.edge();

    if (world.sea(face, x, y) == 0)
    {
        globepoint north = getglobepoint(edge, face, x, y, 0, -1);

        if (world.sea(north.face, north.x, north.y) == 1)
            return 1;

        globepoint south = getglobepoint(edge, face, x, y, 0, 1);

        if (world.sea(south.face, south.x, south.y) == 1)
            return 1;

        globepoint east = getglobepoint(edge, face, x, y, 1, 0);

        if (world.sea(east.face, east.x, east.y) == 1)
            return 1;

        globepoint west = getglobepoint(edge, face, x, y, -1, 0);

        if (world.sea(west.face, west.x, west.y) == 1)
            return 1;

        return 0;
    }
    else
    {
        globepoint north = getglobepoint(edge, face, x, y, 0, -1);

        if (world.sea(north.face, north.x, north.y) == 0)
            return 1;

        globepoint south = getglobepoint(edge, face, x, y, 0, 1);

        if (world.sea(south.face, south.x, south.y) == 0)
            return 1;

        globepoint east = getglobepoint(edge, face, x, y, 1, 0);

        if (world.sea(east.face, east.x, east.y) == 0)
            return 1;

        globepoint west = getglobepoint(edge, face, x, y, -1, 0);

        if (world.sea(west.face, west.x, west.y) == 0)
            return 1;

        return 0;
    }

    return 0;
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

// Same thing, but with faces.

int getdir(int edge, int face, int x, int y, int fface, int xx, int yy)
{
    if (face == fface)
    {
        int dir = getdir(x, y, xx, yy);

        return dir;
    }

    if (face == 0)
    {
        if (fface == 1)
            xx = xx + edge;

        if (fface == 2)
        {
            if (xx < edge / 2)
                xx = xx + edge * 2;
            else
                xx = xx - edge * 2;
        }

        if (fface == 3)
            xx = xx - edge;

        if (fface == 4)
            yy = yy - edge;

        if (fface == 5)
            yy = yy + edge;

        int dir = getdir(x, y, xx, yy);

        return dir;
    }

    if (face == 1)
    {
        if (fface == 2)
            xx = xx + edge;

        if (fface == 3)
        {
            if (xx < edge / 2)
                xx = xx + edge * 2;
            else
                xx = xx - edge * 2;
        }

        if (fface == 0)
            xx = xx - edge;

        if (fface == 4)
        {
            int newxx = edge - yy;
            int newyy = xx - edge;

            xx = newxx;
            yy = newyy;
        }

        if (fface == 5)
        {
            int newxx = yy;
            int newyy = edge * 2 - xx;

            xx = newxx;
            yy = newyy;
        }

        int dir = getdir(x, y, xx, yy);

        return dir;
    }

    if (face == 2)
    {
        if (fface == 3)
            xx = xx + edge;

        if (fface == 0)
        {
            if (xx < edge / 2)
                xx = xx + edge * 2;
            else
                xx = xx - edge * 2;
        }

        if (fface == 1)
            xx = xx - edge;

        if (fface == 4)
        {
            xx = edge - xx;
            yy = -yy;
        }

        if (fface == 5)
        {
            xx = edge - xx;
            yy = edge * 2 - yy;
        }

        int dir = getdir(x, y, xx, yy);

        return dir;
    }

    if (face == 3)
    {
        if (fface == 0)
            xx = xx + edge;

        if (fface == 1)
        {
            if (xx < edge / 2)
                xx = xx + edge * 2;
            else
                xx = xx - edge * 2;
        }

        if (fface == 2)
            xx = xx - edge;

        if (fface == 4)
        {
            int newxx = yy;
            int newyy = -xx;

            xx = newxx;
            yy = newyy;
        }

        if (fface == 5)
        {
            int newxx = edge - yy;
            int newyy = edge + xx;

            xx = newxx;
            yy = newyy;
        }

        int dir = getdir(x, y, xx, yy);

        return dir;
    }

    if (face == 4)
    {
        if (fface == 0)
            yy = yy + edge;

        if (fface == 1)
        {
            int newxx = edge + yy;
            int newyy = edge - xx;

            xx = newxx;
            yy = newyy;
        }

        if (fface == 2)
        {
            xx = edge - xx;
            yy = -yy;
        }

        if (fface == 3)
        {
            int newxx = -yy;
            int newyy = xx;

            xx = newxx;
            yy = newyy;
        }

        if (fface == 5)
        {
            if (yy < edge / 2)
                yy = yy + edge * 2;
            else
                yy = yy - edge * 2;

        }

        int dir = getdir(x, y, xx, yy);

        return dir;
    }

    if (face == 5)
    {
        if (fface == 0)
            yy = yy - edge;

        if (fface == 1)
        {
            int newxx = edge * 2 - yy;
            int newyy = xx;

            xx = newxx;
            yy = newyy;
        }

        if (fface == 2)
        {
            xx = edge - xx;
            yy = edge * 2 - yy;
        }

        if (fface == 3)
        {
            int newxx = edge * 2 - yy;
            int newyy = edge - xx;

            xx = newxx;
            yy = newyy;
        }

        if (fface == 5)
        {
            if (yy < edge / 2)
                yy = yy + edge * 2;
            else
                yy = yy - edge * 2;

        }

        int dir = getdir(x, y, xx, yy);

        return dir;
    }

    return (0);
}

// This function gets the destination from a tile and a direction, just on one face.

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

// Same thing, but with faces.

globepoint getdestination(int edge, int face, int x, int y, int dir)
{
    globepoint thispoint;
    thispoint.face = face;
    thispoint.x = x;
    thispoint.y = y;

    if (dir == 8 || dir == 1 || dir == 2)
        thispoint = getglobepoint(edge, face, x, y, 0, -1);

    if (dir == 4 || dir == 5 || dir == 6)
        thispoint = getglobepoint(edge, face, x, y, 0, 1);

    if (dir == 2 || dir == 3 || dir == 4)
    {
        if (thispoint.face != -1)
            thispoint = getglobepoint(edge, face, x, y, 1, 0);
    }

    if (dir == 6 || dir == 7 || dir == 8)
    {
        if (thispoint.face != -1)
            thispoint = getglobepoint(edge, face, x, y, -1, 0);
    }

    return thispoint;
}

// This returns the coordinate that the river in the given tile is flowing into.

globepoint getflowdestination(planet& world, int face, int x, int y, int dir)
{
    int edge = world.edge();

    globepoint destpoint;

    if (dir == 0)
        dir = world.riverdir(face, x, y);

    globepoint thispoint;
    thispoint.face = face;
    thispoint.x = x;
    thispoint.y = y;

    if (dir == 8 || dir == 1 || dir == 2)
        thispoint = getglobepoint(edge, face, x, y, 0, -1);

    if (dir == 4 || dir == 5 || dir == 6)
        thispoint = getglobepoint(edge, face, x, y, 0, 1);

    if (dir == 2 || dir == 3 || dir == 4)
    {
        if (thispoint.face != -1)
            thispoint = getglobepoint(edge, face, x, y, 1, 0);
    }

    if (dir == 6 || dir == 7 || dir == 8)
    {
        if (thispoint.face != -1)
            thispoint = getglobepoint(edge, face, x, y, -1, 0);
    }

    // Correct problems with some of the edges. (Going 3->5 is fine.)

    if (face == 3 && thispoint.face == 4 && thispoint.x == 1)
        thispoint.x = 0;

    if (face == 4 && thispoint.face == 3 && thispoint.y == 1)
        thispoint.y = 0;

    if (face == 2 && thispoint.face == 4 && thispoint.y == 1)
        thispoint.y = 0;

    return thispoint;
}

// This function tells whether a ridge goes from the specified tile in the specified direction.

int getridge(planet& world, int face, int x, int y, int dir)
{
    bool check = (world.mountainridge(face, x, y) & (1 << (dir - 1))) != 0;

    if (check == 1)
        return(1);

    return(0);
}

// Same thing, but on a specified 2D vector.

int getridge(vector<vector<int>>& arr, int x, int y, int dir)
{
    bool check = (arr[x][y] & (1 << (dir - 1))) != 0;

    if (check == 1)
        return(1);

    return(0);
}

// Same thing, but for an ocean ridge.

int getoceanridge(planet& world, int face, int x, int y, int dir)
{
    bool check = (world.mountainridge(face, x, y) & (1 << (dir - 1))) != 0;

    if (check == 1)
        return(1);

    return(0);
}

// This function deletes a ridge going from the specified tile in the specified direction.

void deleteridge(planet& world, int face, int x, int y, int dir)
{
    int edge = world.edge();

    if (x < 0 || x >= edge)
        return;

    if (y < 0 || y >= edge)
        return;

    if (getridge(world, face, x, y, dir) != 1) // If there isn't a ridge going this way
        return;

    int code = getcode(dir);

    int currentridge = world.mountainridge(face, x, y);

    currentridge = currentridge - code;

    world.setmountainridge(face, x, y, currentridge);

    if (currentridge == 0)
        world.setmountainheight(face, x, y, 0);

    // Now remove one going the other way.

    int xshift = 0;
    int yshift = 0;

    if (dir == 8 || dir == 1 || dir == 2)
        yshift--;

    if (dir == 4 || dir == 5 || dir == 6)
        yshift++;

    if (dir == 2 || dir == 3 || dir == 4)
        xshift++;

    if (dir == 6 || dir == 7 || dir == 8)
        xshift--;

    globepoint newpoint = getglobepoint(edge, face, x, y, xshift, yshift);

    if (newpoint.face != -1)
    {
        dir = dir + 4;

        if (dir > 8)
            dir = dir - 8;

        if (getridge(world, newpoint.face, newpoint.x, newpoint.y, dir) != 1) // If there isn't a ridge going in this direction
            return;

        code = getcode(dir);

        currentridge = world.mountainridge(newpoint.face, newpoint.x, newpoint.y);

        currentridge = currentridge - code;

        world.setmountainridge(newpoint.face, newpoint.x, newpoint.y, currentridge);

        if (currentridge == 0)
            world.setmountainheight(newpoint.face, newpoint.x, newpoint.y, 0);
    }
}

// Same thing, but on a specified 2D vector.

void deleteridge(planet& world, vector<vector<int>>& ridgesarr, vector<vector<int>>& heightsarr, int x, int y, int dir)
{
    int edge = world.edge();

    if (x < 0 || x >= edge)
        return;

    if (y < 0 || y >= edge)
        return;

    if (getridge(ridgesarr, x, y, dir) != 1) // If there isn't a ridge going this way
        return;

    int code = getcode(dir);

    int currentridge = ridgesarr[x][y];

    currentridge = currentridge - code;

    ridgesarr[x][y] = currentridge;

    if (currentridge == 0)
        heightsarr[x][y] = 0;

    // Now remove one going the other way.

    if (dir == 8 || dir == 1 || dir == 2)
        y--;

    if (dir == 4 || dir == 5 || dir == 6)
        y++;

    if (dir == 2 || dir == 3 || dir == 4)
        x++;

    if (dir == 6 || dir == 7 || dir == 8)
        x--;

    if (x < 0 || x >= edge)
        return;

    if (y < 0 || y >= edge)
        return;

    dir = dir + 4;

    if (dir > 8)
        dir = dir - 8;

    if (getridge(ridgesarr, x, y, dir) != 1) // If there isn't a ridge going in this direction
        return;

    code = getcode(dir);

    currentridge = ridgesarr[x][y];

    currentridge = currentridge - code;

    ridgesarr[x][y] = currentridge;

    if (currentridge == 0)
        heightsarr[x][y] = 0;
}

// Same thing, but for an ocean ridge.

void deleteoceanridge(planet& world, int face, int x, int y, int dir)
{
    int edge = world.edge();

    if (x < 0 || x >= edge)
        return;

    if (y < 0 || y >= edge)
        return;

    if (getoceanridge(world, face, x, y, dir) != 1) // If there isn't a ridge going this way
        return;

    int code = getcode(dir);

    int currentridge = world.oceanridges(face, x, y);

    currentridge = currentridge - code;

    world.setoceanridges(face, x, y, currentridge);

    // Now remove one going the other way.

    int xshift = 0;
    int yshift = 0;

    if (dir == 8 || dir == 1 || dir == 2)
        yshift--;

    if (dir == 4 || dir == 5 || dir == 6)
        yshift++;

    if (dir == 2 || dir == 3 || dir == 4)
        xshift++;

    if (dir == 6 || dir == 7 || dir == 8)
        xshift--;

    globepoint newpoint = getglobepoint(edge, face, x, y, xshift, yshift);

    if (newpoint.face != -1)
    {
        dir = dir + 4;

        if (dir > 8)
            dir = dir - 8;

        if (getoceanridge(world, newpoint.face, newpoint.x, newpoint.y, dir) != 1) // If there isn't a ridge going in this direction
            return;

        code = getcode(dir);

        currentridge = world.oceanridges(newpoint.face, newpoint.x, newpoint.y);

        currentridge = currentridge - code;

        world.setoceanridges(newpoint.face, newpoint.x, newpoint.y, currentridge);
    }
}

// This function translates a mountain direction into a binary code.

int getcode(int dir)
{
    if (dir == 1)
        return(1);

    if (dir == 2)
        return(2);

    if (dir == 3)
        return(4);

    if (dir == 4)
        return(8);

    if (dir == 5)
        return(16);

    if (dir == 6)
        return(32);

    if (dir == 7)
        return(64);

    if (dir == 8)
        return(128);

    return(0);
}

// This finds the direction from one point to another, for mountain ranges.

short getmountaindir(int startx, int starty, int endx, int endy)
{
    // First, work out whether we need to do any wrapping.

    short dir = 0;

    int xdist = abs(startx - endx);
    int ydist = abs(starty - endy);

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

// This function ensures that all mountain ridges connect up properly, removing rogue ones.

void cleanmountainridges(planet& world)
{
    int edge = world.edge();

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.mountainheight(face, i, j) != 0 && world.sea(face, i, j) == 0)
                {
                    for (int dir = 1; dir <= 8; dir++)
                    {
                        if (getridge(world, face, i, j, dir) == 1 && getdestinationland(world, face, i, j, dir) == 0) // If there is a ridge going in this direction from this point
                            deleteridge(world, face, i, j, dir);
                    }
                }
            }
        }
    }
}

// This function tells whether there is land in the specified direction from this point.

int getdestinationland(planet& world, int face, int x, int y, int dir)
{
    int edge = world.edge();

    int xshift = x;
    int yshift = y;

    if (dir == 8 || dir == 1 || dir == 2)
        yshift--;

    if (dir == 4 || dir == 5 || dir == 6)
        yshift++;

    if (dir == 2 || dir == 3 || dir == 4)
        xshift++;

    if (dir == 6 || dir == 7 || dir == 8)
        xshift--;

    globepoint newpoint = getglobepoint(edge, face, x, y, xshift, yshift);

    // newx and newy are the coordinates of where this ridge is pointing to.

    if (world.sea(newpoint.face, newpoint.x, newpoint.y) == 1)
        return(0);

    return(1);
}

// This works out the total areas of land and sea in the world.

void getlandandseatotals(planet& world)
{
    int edge = world.edge();
    int sealevel = world.sealevel();

    int seatotal = 0;
    int landtotal = 0;

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++) // Avoid the margins, to avoid false positives.
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.nom(face, i, j) > sealevel)
                    landtotal++;
                else
                    seatotal++;
            }
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

// This function adds the elevation element to a temperature.

int tempelevadd(planet& world, int temp, int face, int i, int j)
{
    int elevation = world.map(face, i, j) - world.sealevel();

    if (elevation < 0)
        elevation = 0;

    float elevationadjust = (float)elevation;
    elevationadjust = elevationadjust / 1000.0f;
    elevationadjust = elevationadjust * world.tempdecrease();

    temp = temp - (int)elevationadjust;

    return temp;
}

// Same thing, but for a simple world.

int tempelevadd(planet& world, simpleplanet& simpleworld, int temp, int face, int i, int j)
{
    int elevation = simpleworld.map(face, i, j) - world.sealevel();

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

int tempelevremove(planet& world, int temp, int face, int i, int j)
{
    int elevation = world.map(face, i, j) - world.sealevel();

    if (elevation < 0)
        elevation = 0;

    float elevationadjust = (float)elevation;
    elevationadjust = elevationadjust / 1000.0f;
    elevationadjust = elevationadjust * world.tempdecrease();

    temp = temp + (int)elevationadjust;

    return temp;
}

// Same thing, but for a simple world.

int tempelevremove(planet& world, simpleplanet& simpleworld, int temp, int face, int i, int j)
{
    int elevation = simpleworld.map(face, i, j) - world.sealevel();

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

// This gets the amount the land is sloping between two points.

int getslope(planet& world, int face, int x, int y, int fface, int xx, int yy)
{
    if (face == -1 || fface == -1)
        return -1;

    int slope = world.map(fface, xx, yy) - world.map(face, x, y);

    return (slope);
}

// This function checks how flat the land is at a given point.

int getflatness(planet& world, int face, int x, int y)
{
    int edge = world.edge();

    globepoint northpoint = getglobepoint(edge, face, x, y, 0, -1);
    globepoint southpoint = getglobepoint(edge, face, x, y, 0, 1);
    globepoint eastpoint = getglobepoint(edge, face, x, y, -1, 0);
    globepoint westpoint = getglobepoint(edge, face, x, y, 1, 0);

    int thiselev = getflatelevation(world, face, x, y);

    float flatness = 0.0f;

    int thatelev = getflatelevation(world, northpoint.face, northpoint.x, northpoint.y);
    flatness = flatness + (float)abs(thiselev - thatelev);

    thatelev = getflatelevation(world, southpoint.face, southpoint.x, southpoint.y);
    flatness = flatness + (float)abs(thiselev - thatelev);

    thatelev = getflatelevation(world, eastpoint.face, eastpoint.x, eastpoint.y);
    flatness = flatness + (float)abs(thiselev - thatelev);

    thatelev = getflatelevation(world, westpoint.face, westpoint.x, westpoint.y);
    flatness = flatness + (float)abs(thiselev - thatelev);

    flatness = flatness / 4.0f;

    return (int)flatness;
}

// This function gets the elevation of a point, incorporating water level.

int getflatelevation(planet& world, int face, int x, int y)
{
    int elev;

    int surface = world.lakesurface(face, x, y);

    if (surface > 0)
        elev = surface;
    else
    {
        if (world.sea(face, x, y) == 1)
            elev = world.sealevel();
        else
            elev = world.nom(face, x, y);
    }

    return (elev);
}

// This function finds the direction of the lowest neighbouring tile.

int findlowestdir(planet& world, int neighbours[8][2], int face, int x, int y)
{
    int edge = world.edge();

    int lowest = world.maxelevation() * 2;
    int nnn = -1;

    int start = random(0, 7);

    for (int n = start; n <= start + 7; n++)
    {
        int nn = wrap(n, 7);

        int i = neighbours[nn][0];

        int j = neighbours[nn][1];

        globepoint jpoint = getglobepoint(edge, face, x, y, i, j);

        if (jpoint.face != -1)
        {
            int pointelev = world.nom(jpoint.face, jpoint.x, jpoint.y);

            if (pointelev < lowest)
            {
                lowest = pointelev;
                nnn = nn;
            }
        }
    }

    /*
    if (nnn == -1)
    {
        start = random(0, 7);

        for (int n = start; n <= start + 7; n++)
        {
            int nn = wrap(n, 7);

            int i = neighbours[nn][0] * 2;

            globepoint ipoint = getglobepoint(edge, face, x, y, i, 0);

            if (ipoint.face != -1)
            {
                int j = neighbours[nn][1] * 2;

                globepoint jpoint = getglobepoint(edge, ipoint.face, ipoint.x, ipoint.y, 0, j);

                if (jpoint.face != -1)
                {
                    int pointelev = world.nom(jpoint.face, jpoint.x, jpoint.y);

                    if (pointelev < lowest)
                    {
                        lowest = pointelev;
                        nnn = nn;
                    }
                }
            }
        }
    }
    */

    int dir = nnn + 1;

    return (dir);
}

// The same thing, using cardinal directions.

int findlowestdir(planet& world, int face, int x, int y, vector<fourglobepoints>& dirpoint)
{
    int edge = world.edge();

    int index = face * edge * edge + x * edge + y;

    int lowest = world.maxelevation() * 2;

    globepoint neighbours[3][3];

    neighbours[1][0] = dirpoint[index].north;
    neighbours[2][1] = dirpoint[index].east;
    neighbours[1][2] = dirpoint[index].south;
    neighbours[0][1] = dirpoint[index].west;

    int northindex = neighbours[1][0].face * edge * edge + neighbours[1][0].x * edge + neighbours[1][0].y;
    int southindex = neighbours[1][2].face * edge * edge + neighbours[1][2].x * edge + neighbours[1][2].y;


    neighbours[0][0] = dirpoint[northindex].west;
    neighbours[2][0] = dirpoint[northindex].east;
    neighbours[0][2] = dirpoint[southindex].west;
    neighbours[2][2] = dirpoint[southindex].east;

    int istart = 0;
    int iend = 3;
    int istep = 1;

    int jstart = 0;
    int jend = 3;
    int jstep = 1;

    if (random(1, 2) == 1)
    {
        istart = 2;
        iend = -1;
        istep = -1;
    }

    if (random(1, 2) == 1)
    {
        jstart = 2;
        jend = -1;
        jend = -1;
    }

    int dirx = -1;
    int diry = -1;

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (i != 1 || j != 1)
            {
                int face = neighbours[i][j].face;
                int x = neighbours[i][j].x;
                int y = neighbours[i][j].y;

                int pointelev = world.nom(face, x, y);

                if (pointelev < lowest)
                {
                    lowest = pointelev;
                    dirx = i;
                    diry = j;
                }
            }
        }
    }

    if (dirx == 1 && diry == 0)
        return 1;

    if (dirx == 2 && diry == 0)
        return 2;

    if (dirx == 2 && diry == 1)
        return 3;

    if (dirx == 2 && diry == 2)
        return 4;

    if (dirx == 1 && diry == 2)
        return 5;

    if (dirx == 0 && diry == 2)
        return 6;

    if (dirx == 0 && diry == 1)
        return 7;

    if (dirx == 0 && diry == 0)
        return 8;

    return -1;
}

// This function finds a neighbouring sea tile that isn't the one we just came from.

globepoint findseatile(planet& world, int face, int x, int y, int dir)
{
    int edge = world.edge();

    globepoint newtile;

    newtile.face = -1;
    newtile.x = x;
    newtile.y = y;

    globepoint oldtile;

    oldtile.face = face;
    oldtile.x = x;
    oldtile.y = y;

    if (dir == 8 || dir == 1 || dir == 2)
        oldtile = getglobepoint(edge, oldtile.face, oldtile.x, oldtile.y, 0, 1);

    if (oldtile.face != -1)
    {
        if (dir == 4 || dir == 5 || dir == 6)
            oldtile = getglobepoint(edge, oldtile.face, oldtile.x, oldtile.y, 0, -1);

        if (oldtile.face != -1)
        {
            if (dir == 2 || dir == 3 || dir == 4)
                oldtile = getglobepoint(edge, oldtile.face, oldtile.x, oldtile.y, -1, 0);

            if (oldtile.face != -1)
            {
                if (dir == 6 || dir == 7 || dir == 8)
                    oldtile = getglobepoint(edge, oldtile.face, oldtile.x, oldtile.y, 1, 0);
            }
        }
    }

    if (oldtile.face != -1)
    {
        oldtile.face = face;
        oldtile.x = x;
        oldtile.y = y;
    }

    // oldx and oldy are the coordinates of where we are coming to this tile from. We want to avoid going back there!

    int lowest = world.map(face, x, y);

    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            globepoint jpoint = getglobepoint(edge, face, x, y, i, j);

            if ((jpoint.face != oldtile.face || jpoint.x != oldtile.x || jpoint.y != oldtile.y) && world.sea(jpoint.face, jpoint.x, jpoint.y) == 1 && world.map(jpoint.face, jpoint.x, jpoint.y) < lowest)
            {
                newtile = jpoint;
                lowest = world.map(jpoint.face, jpoint.x, jpoint.y);
            }
        }
    }

    if (newtile.face != -1)
        return (newtile);

    // If it didn't find a neighbouring sea tile that's lower, just find one at all.

    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            globepoint jpoint = getglobepoint(edge, face, x, y, i, j);

            if ((jpoint.face != oldtile.face || jpoint.x != oldtile.x || jpoint.y != oldtile.y) && world.sea(jpoint.face, jpoint.x, jpoint.y) == 1)
                newtile = jpoint;
        }
    }

    return (newtile);
}

// This function finds the cell with the largest flow into the current one. (This looks on only the current face.)

twointegers getupstreamcell(planet& world, int face, int x, int y)
{
    int edge = world.edge();

    twointegers upcell;
    globepoint destpoint;

    upcell.x = -1;
    upcell.y = -1;

    int largest = 0;

    int riverjan, riverjul;

    for (int i = x - 1; i <= x + 1; i++)
    {
        if (i < 0 || i >= edge)
            return upcell;

        for (int j = y - 1; j <= y + 1; j++)
        {
            if (j >= 0 && y < edge)
            {
                riverjan = world.riverjan(face, i, j);
                riverjul = world.riverjul(face, i, j);

                if (riverjan + riverjul > largest) // If this is larger than the current largest inflow
                {
                    destpoint = getflowdestination(world, face, i, j, 0);

                    if (destpoint.face == face && destpoint.x == x && destpoint.y == y) // If this is actually flowing into our cell
                    {
                        upcell.x = i;
                        upcell.y = j;

                        largest = riverjan + riverjul;
                    }
                }
            }
        }
    }
    return (upcell);
}

// Same thing, but using arrays.

twointegers getupstreamcell(planet& world, int x, int y, vector<int>& riverjan, vector<int>& riverjul, vector<int>& riverdir)
{
    int edge = world.edge();
    int width = edge * 4 - 1;
    int height = edge * 3 - 1;

    twointegers upcell;
    globepoint destpoint;

    upcell.x = -1;
    upcell.y = -1;

    int largest = 0;

    for (int i = x - 1; i <= x + 1; i++)
    {
        int ii = i;

        if (ii<0 || ii>width)
            ii = wrap(ii, width);

        int vi = ii * (height + 1);

        for (int j = y - 1; j <= y + 1; j++)
        {
            if (j >= 0 && y <= height)
            {
                int index = vi + j;

                if (riverjan[index] + riverjul[index] > largest) // If this is larger than the current largest inflow
                {
                    int dx = ii;
                    int dy = j;

                    if (riverdir[index] == 8 || riverdir[index] == 1 || riverdir[index] == 2)
                        dy--;

                    if (riverdir[index] == 4 || riverdir[index] == 5 || riverdir[index] == 6)
                        dy++;

                    if (riverdir[index] == 2 || riverdir[index] == 3 || riverdir[index] == 4)
                        dx++;

                    if (riverdir[index] == 6 || riverdir[index] == 7 || riverdir[index] == 8)
                        dx++;

                    if (dx<0 || dx>width)
                        dx = wrap(dx, width);

                    if (dx == x && dy == y) // If this is actually flowing into our cell
                    {
                        upcell.x = ii;
                        upcell.y = j;

                        largest = riverjan[index] + riverjul[index];
                    }
                }
            }
        }
    }
    return (upcell);
}

// Deprecated version.

twointegers getupstreamcellold(planet& world, int x, int y, vector<vector<int>>& riverjan, vector<vector<int>>& riverjul, vector<vector<int>>& riverdir)
{
    int edge = world.edge();
    int width = edge * 4 - 1;
    int height = edge * 3 - 1;

    twointegers upcell;
    globepoint destpoint;

    upcell.x = -1;
    upcell.y = -1;

    int largest = 0;

    for (int i = x - 1; i <= x + 1; i++)
    {
        int ii = i;

        if (ii<0 || ii>width)
            ii = wrap(ii, width);

        for (int j = y - 1; j <= y + 1; j++)
        {
            if (j >= 0 && y <= height)
            {
                if (riverjan[ii][j] + riverjul[ii][j] > largest) // If this is larger than the current largest inflow
                {
                    int dx = ii;
                    int dy = j;

                    if (riverdir[ii][j] == 8 || riverdir[ii][j] == 1 || riverdir[ii][j] == 2)
                        dy--;

                    if (riverdir[ii][j] == 4 || riverdir[ii][j] == 5 || riverdir[ii][j] == 6)
                        dy++;

                    if (riverdir[ii][j] == 2 || riverdir[ii][j] == 3 || riverdir[ii][j] == 4)
                        dx++;

                    if (riverdir[ii][j] == 6 || riverdir[ii][j] == 7 || riverdir[ii][j] == 8)
                        dx++;

                    if (dx<0 || dx>width)
                        dx = wrap(dx, width);

                    if (dx == x && dy == y) // If this is actually flowing into our cell
                    {
                        upcell.x = ii;
                        upcell.y = j;

                        largest = riverjan[ii][j] + riverjul[ii][j];
                    }
                }
            }
        }
    }
    return (upcell);
}

// This function checks to see whether a tile on the global map has any water flowing into it. (This looks on only the current face.)

int checkwaterinflow(planet& world, int face, int x, int y)
{
    int i, j;

    int edge = world.edge();

    // From the north

    i = x;
    j = y - 1;

    if (j >= 0 && world.riverdir(face, i, j) == 5)
        return 1;

    // From the northeast

    i = x + 1;
    j = y - 1;

    if (j >= 0 && i < edge && world.riverdir(face, i, j) == 6)
        return 1;

    // From the east

    i = x + 1;
    j = y;

    if (i < edge && world.riverdir(face, i, j) == 7)
        return 1;

    // From the southeast

    i = x + 1;
    j = y + 1;

    if (i < edge && j < edge && world.riverdir(face, i, j) == 8)
        return 1;

    // From the south

    i = x;
    j = y + 1;

    if (j < edge && world.riverdir(face, i, j) == 1)
        return 1;

    // From the southwest

    i = x - 1;
    j = y + 1;

    if (i >= 0 && j < edge && world.riverdir(face, i, j) == 2)
        return 1;

    // From the west

    i = x - 1;
    j = y;

    if (j < edge && world.riverdir(face, i, j) == 3)
        return 1;

    // From the northwest

    i = x - 1;
    j = y - 1;

    if (i >= 0 && j >= 0 && world.riverdir(face, i, j) == 4)
        return 1;

    return 0;
}

// This function gets the total water inflow into a given tile on the global map. (This looks on only the current face.)

twointegers gettotalinflow(planet& world, int face, int x, int y)
{
    int edge = world.edge();

    twointegers total;
    globepoint dest;

    total.x = 0;
    total.y = 0;

    for (int i = x - 1; i <= x + 1; i++)
    {
        if (i >= 0 && i < edge)
        {
            for (int j = y - 1; j <= y + 1; j++)
            {
                if (j >= 0 && j < edge)
                {
                    dest = getflowdestination(world, face, i, j, 0);

                    if (dest.face == face && dest.x == x && dest.y == y)
                    {
                        total.x = total.x + world.riverjan(face, i, j);
                        total.y = total.y + world.riverjul(face, i, j);
                    }
                }
            }
        }
    }
    return total;
}

// This rotates an area of the given global array. (twists: positive for anticlockwise, negative for clockwise.) (based on pseudocode here: https://stackoverflow.com/questions/30448045/how-do-you-add-a-swirl-to-an-image-image-distortion?)

void makeswirl(vector<int>& arr, int edge, int centreface, int centrex, int centrey, int maxradius, float twists, vector<bool>& done, vector<fourglobepoints>& dirpoint)
{
    int squaremax = maxradius * maxradius + maxradius;
    int smallsquaremax = (maxradius - 2) * (maxradius - 2) + maxradius - 2;

    // First, copy the array.

    vector<int> copiedarray(6 * edge * edge, 0);

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int index = vi + j;

                copiedarray[index] = arr[index];
            }
        }
    }

    for (int i = -maxradius; i <= maxradius; i++)
    {
        for (int j = -maxradius; j <= maxradius; j++)
        {
            globepoint thispoint;
            thispoint.face = centreface;
            thispoint.x = centrex;
            thispoint.y = centrey;

            globepoint thatpoint = thispoint;

            int ii = i;
            int jj = j;

            bool goup = 0;
            bool goleft = 0;

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

            float pixelX = (float)i;
            float pixelY = (float)j;
            float pixelDistance = sqrt((pixelX * pixelX) + (pixelY * pixelY));
            float pixelAngle = atan2(pixelY, pixelX);

            float swirlAmount = 1.0f - (pixelDistance / (float)maxradius);

            if (swirlAmount > 0.0f)
            {
                float twistAngle = twists * swirlAmount * PI * 2.0f;

                // adjust the pixel angle and compute the adjusted pixel co-ordinates:
                pixelAngle += twistAngle;
                pixelX = cos(pixelAngle) * pixelDistance;
                pixelY = sin(pixelAngle) * pixelDistance;
            }

            ii = (int)pixelX;
            jj = (int)pixelY;

            goup = 0;
            goleft = 0;

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

            for (int n = 0; n < ii; n++)
            {
                if (goleft)
                    thatpoint = dirpoint[thatpoint.face * edge * edge + thatpoint.x * edge + thatpoint.y].west;
                else
                    thatpoint = dirpoint[thatpoint.face * edge * edge + thatpoint.x * edge + thatpoint.y].east;
            }

            for (int n = 0; n < jj; n++)
            {
                if (goup)
                    thatpoint = dirpoint[thatpoint.face * edge * edge + thatpoint.x * edge + thatpoint.y].north;
                else
                    thatpoint = dirpoint[thatpoint.face * edge * edge + thatpoint.x * edge + thatpoint.y].south;
            }

            arr[thispoint.face * edge * edge + thispoint.x * edge + thispoint.y] = copiedarray[thatpoint.face * edge * edge + thatpoint.x * edge + thatpoint.y];

            if (thispoint.face != thatpoint.face || thispoint.x != thatpoint.x || thispoint.y != thatpoint.y)
                done[thispoint.face * edge * edge + thispoint.x * edge + thispoint.y] = 1;
        }
    }
}

// This works out the position of the sun in Cartesian coordinates, given an angle from the planet.

threefloats getsunposition(float angle, float distance)
{
    threefloats pos;
    pos.y = 0.0f;

    angle = angle * 0.01745329f; // Convert to radians.

    if (angle == 0.0f)
    {
        pos.x = 0.0f;
        pos.z = -distance;

        return pos;
    }

    if (angle == 90.0f)
    {
        pos.x = distance;
        pos.z = 0.0f;

        return pos;
    }

    if (angle == 180.0f)
    {
        pos.x = 0.0f;
        pos.z = distance;

        return pos;
    }

    if (angle == 270.0f)
    {
        pos.x = -distance;
        pos.z = 0.0f;

        return pos;
    }

    if (angle > 0.0f && angle < 90.0f)
    {
        pos.x = sin(angle) * distance;
        pos.z = cos(angle) * -distance;

        return pos;
    }

    if (angle > 90.0f && angle < 180.0f)
    {
        angle = angle - 90.0f;

        pos.x = cos(angle) * distance;
        pos.z = sin(angle) * -distance;

        return pos;
    }

    if (angle > 180.0f && angle < 270.0f)
    {
        angle = angle - 180.0f;

        pos.x = sin(angle) * -distance;
        pos.z = cos(angle) * distance;

        return pos;
    }

    angle = angle - 270.0f;

    pos.x = cos(angle) * -distance;
    pos.z = sin(angle) * -distance;

    return pos;
}

// This works out the position of a point in Cartesian coordinates, given lat/long and radius from the planet's centre.

threefloats getcoordinatesfromlatlong(float lat, float lon, float radius)
{
    lat = lat * 0.01745329f; // Convert to radians.
    lon = lon * 0.01745329f; // Convert to radians.

    threefloats pos;

    pos.x = radius * cos(lat) * cos(lon);
    pos.y = radius * sin(lat);
    pos.z = radius * cos(lat) * sin(lon);

    return pos;
}

// This creates a list of key codes.

void createkeycodelist(vector<string>& keycodelist)
{
    keycodelist[32] = "SPACE";

    keycodelist[39] = "'";

    keycodelist[44] = ",";
    keycodelist[45] = "-";
    keycodelist[46] = ".";
    keycodelist[47] = "/";
    keycodelist[48] = "0";
    keycodelist[49] = "1";
    keycodelist[50] = "2";
    keycodelist[51] = "3";
    keycodelist[52] = "4";
    keycodelist[53] = "5";
    keycodelist[54] = "6";
    keycodelist[55] = "7";
    keycodelist[56] = "8";
    keycodelist[57] = "9";

    keycodelist[59] = ";";

    keycodelist[61] = "=";

    keycodelist[65] = "A";
    keycodelist[66] = "B";
    keycodelist[67] = "C";
    keycodelist[68] = "D";
    keycodelist[69] = "E";
    keycodelist[70] = "F";
    keycodelist[71] = "G";
    keycodelist[72] = "H";
    keycodelist[73] = "I";
    keycodelist[74] = "J";
    keycodelist[75] = "K";
    keycodelist[76] = "L";
    keycodelist[77] = "M";
    keycodelist[78] = "N";
    keycodelist[79] = "O";
    keycodelist[80] = "P";
    keycodelist[81] = "Q";
    keycodelist[82] = "R";
    keycodelist[83] = "S";
    keycodelist[84] = "T";
    keycodelist[85] = "U";
    keycodelist[86] = "V";
    keycodelist[87] = "W";
    keycodelist[88] = "X";
    keycodelist[89] = "Y";
    keycodelist[90] = "Z";
    keycodelist[91] = "[";
    keycodelist[92] = "\\";
    keycodelist[93] = "]";
    keycodelist[96] = "`";

    //keycodelist[256] = "ESC";
    keycodelist[257] = "ENTER";
    //keycodelist[258] = "TAB";
    keycodelist[259] = "BACKSPACE";
    keycodelist[260] = "INSERT";
    keycodelist[261] = "DEL";
    keycodelist[262] = "RIGHT";
    keycodelist[263] = "LEFT";
    keycodelist[264] = "DOWN";
    keycodelist[265] = "UP";
    keycodelist[266] = "PAGE UP";
    keycodelist[267] = "PAGE DOWN";
    keycodelist[268] = "HOME";
    keycodelist[269] = "END";

    keycodelist[280] = "CAPS LOCK";
    keycodelist[281] = "SCROLL LOCK";
    keycodelist[282] = "NUM LOCK";
    keycodelist[283] = "PRINT SCREEN";
    keycodelist[284] = "PAUSE";

    keycodelist[290] = "F1";
    keycodelist[291] = "F2";
    keycodelist[292] = "F3";
    keycodelist[293] = "F4";
    keycodelist[294] = "F5";
    keycodelist[295] = "F6";
    keycodelist[296] = "F7";
    keycodelist[297] = "F8";
    keycodelist[298] = "F9";
    keycodelist[299] = "F10";
    keycodelist[300] = "F11";
    keycodelist[301] = "F12";

    keycodelist[320] = "KEYPAD 0";
    keycodelist[321] = "KEYPAD 1";
    keycodelist[322] = "KEYPAD 2";
    keycodelist[323] = "KEYPAD 3";
    keycodelist[324] = "KEYPAD 4";
    keycodelist[325] = "KEYPAD 5";
    keycodelist[326] = "KEYPAD 6";
    keycodelist[327] = "KEYPAD 7";
    keycodelist[328] = "KEYPAD 8";
    keycodelist[329] = "KEYPAD 9";
    keycodelist[330] = "KEYPAD .";
    keycodelist[331] = "KEYPAD /";
    keycodelist[332] = "KEYPAD *";
    keycodelist[333] = "KEYPAD -";
    keycodelist[334] = "KEYPAD +";
    keycodelist[335] = "KEYPAD ENTER";
    keycodelist[336] = "KEYPAD =";

    keycodelist[340] = "LEFT SHIFT";
    keycodelist[341] = "LEFT CTRL";
    keycodelist[342] = "LEFT ALT";
    keycodelist[343] = "LEFT SUPER";
    keycodelist[344] = "RIGHT SHIFT";
    keycodelist[345] = "RIGHT CTRL";
    keycodelist[346] = "RIGHT ALT";
    keycodelist[347] = "RIGHT SUPER";
    keycodelist[348] = "KB MENU";
}

// This function sets up the default key bindings.

void initialisekeybindings(vector<int>& keybindings)
{
    keybindings[1] = 46; // Rotate planet east
    keybindings[2] = 44; // Rotate planet west
    keybindings[3] = 65; // Move camera west
    keybindings[4] = 68; // Move camera east
    keybindings[5] = 87; // Move camera north
    keybindings[6] = 83; // Move camera south
    keybindings[7] = 69; // Zoom camera in
    keybindings[8] = 81; // Zoom camera out
    keybindings[9] = 32; // Show/hide UI
    keybindings[10] = 91; // Previous month
    keybindings[11] = 93; // Next month
}

// This function sets up the default movement settings.

void initialisemovementsettings(float& camerazoomspeed, float& camerarotatespeed, float& planetrotatespeed, float& monthspeed, bool& cameralock, bool& monthrotate)
{
    camerazoomspeed = 0.5f;
    camerarotatespeed = 0.5f;
    planetrotatespeed = 0.2f;
    monthspeed = 0.5f;
    cameralock = 0;
    monthrotate = 1;
}


// This function plugs the global variables into the world.

void initialiseworld(planet& world)
{
    world.clear();

    int saveversion = 1;        // Only save files that start with this number can be loaded
    int settingssaveversion = 1; // As above, but for settings files.
    int size = 2;
    int type = 2;
    //int width = 2047; // 1280; // 2047;
    //int height = 1024; // 640; // 1024;

    int edge = 512;               // width/height of the faces of the cube

    bool rotation = 1;            // 1 to rotate like Earth, 0 for the other way
    float tilt = 22.5f;            // for calculating seasonal change
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
    int seaicetemp = -5;          // Temperature at (or below) which sea ice forms.


    world.setsaveversion(saveversion);
    world.setsettingssaveversion(settingssaveversion);
    world.setsize(size);
    //world.setwidth(width);
    //world.setheight(height);
    world.setedge(edge);
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
    world.setseaicetemp(seaicetemp);

    world.setseatotal(0);
    world.setlandtotal(0);
}

// This function sets up the default colours (and other settings) for drawing relief maps.

void initialisemapcolours(planet& world)
{
    world.setlandmarbling(0.5f);
    world.setlakemarbling(0.8f);
    world.setseamarbling(0.8f);

    world.setsnowchange(1);
    world.setseaiceappearance(1);
    world.setminriverflowglobal(1000000);
    world.setminriverflowregional(150);
    world.setshowmangroves(1);

    if (world.type() == 4)
        world.setcolourcliffs(1);
    else
        world.setcolourcliffs(0);

    world.setseaice1(243);
    world.setseaice2(243);
    world.setseaice3(243); // Colours for sea ice.

    world.setsea1(60); // 62
    world.setsea2(172); // 93
    world.setsea3(132); // 149 // Colours for the shallow sea.

    world.setocean1(62); // 62
    world.setocean2(84); // 93
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
}

// This sets up default settings used by the space view shaders.

void initialisespacesettings(planet& world)
{
    world.setnormal(0.25f);
    world.setambient(0.0f);
    world.setgamma(0.6f);
    world.sethaze(0.3f);
    world.setspecular(0.5f);
    world.setcloudcover(0.5f);

    world.setbloomdist(0.6f);
    world.setsunglare(0.4f);
    world.setsunlens(0.35f);
    world.setsunrayno(12);
    world.setsunraystrength(0.25f);
    world.setstarbright(0.5f);
    world.setstarcolour(0.75f);
    world.setstarhaze(0.75f);
    world.setstarnebula(1.0f);

    world.sethighlight1(0);
    world.sethighlight2(255);
    world.sethighlight3(255); // Colours of highlights on the map.

    world.setatmos1(131);
    world.setatmos2(150);
    world.setatmos3(203); // Colours of the atmosphere.

    world.setdusk1(204);
    world.setdusk2(76);
    world.setdusk3(0); // Colours of dawn/dusk light.

    world.setsun1(255);
    world.setsun2(255);
    world.setsun3(255); // Colours of sunlight.

    world.setgal1haze1(255);
    world.setgal1haze2(180);
    world.setgal1haze3(180); // Galactic haze colour 1.

    world.setgal2haze1(180);
    world.setgal2haze2(180);
    world.setgal2haze3(255); // Galactic haze colour 2.

    world.setgal1nebula1(209);
    world.setgal1nebula2(73);
    world.setgal1nebula3(0); // Galactic nebula colour 1.

    world.setgal2nebula1(2);
    world.setgal2nebula2(192);
    world.setgal2nebula3(52); // Galactic nebula colour 2.

    world.setbloom1(102);
    world.setbloom2(128);
    world.setbloom3(255); // Colour of the tint on the bloom effect.
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

        if (random(1, 4) != 1)
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

    int averagetemp = random(1, 30);
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

// This calculates the position of the sun for each month. We assume a year of 12 months of equal duration.

void calculatesunpositions(planet& world)
{
    float pi = 3.1415926535f;
    float degtorad = 0.01745329f;
    float radtodeg = 57.2957795f;

    float e = world.eccentricity();

    float decdist = 0.0f;
    float jundist = 0.0f;
    
    float thisday = -10.0f;

    // First, longitude and distance.

    for (int thismonth = 0; thismonth < 12; thismonth++)
    {
        thisday = thisday + 30.0f;

        float t = thisday + 10.0f; // Time since perihelion.

        if (world.perihelion() == 1) // Perihelion in June
        {
            t = thisday - 170.0f;
        }

        if (t < 0.0f)
            t = t + 360.0f;

        if (t > 360.0f)
            t = 360.0f - t;

        float E = 0.0f; // This will be the eccentric anomaly.

        float M = t; // Mean anomaly equals time since perihelion, in our simplified system.

        float oldE = M + (180.0f / pi) * e * sin(M * degtorad) * (1.0f + e * cos(M * degtorad));

        for (int n = 0; n < 10; n++) // We have to reiterate a few times to try to get this accurate.
        {
            float newE = oldE - (oldE - (180.0f / pi) * e * sin(oldE * degtorad) - M) / (1.0f - e * cos(oldE * degtorad));

            oldE = newE;
            E = newE;
        }

        // Now work out the position in x,y coordinates.

        float x = cos(E * degtorad) - e;
        float y = sqrt(1.0f - e * e) * sin(E * degtorad);

        // Now convert that to longitude and distance.

        float dist = sqrt(x * x + y * y);
        float longitude = atan2(y, x) * radtodeg;

        if (world.perihelion() == 1)
        {
            longitude = longitude + 180.0f;

            if (longitude > 360.0f)
                longitude = longitude - 360.0f;
        }

        world.setsunlong(thismonth, longitude);
        world.setsundist(thismonth, dist);

        if (thismonth == 5)
            jundist = dist;

        if (thismonth == 11)
            decdist = dist;
    }

    // Now latitude. We just work this out from the distance of the sun at each month, as it varies with it.

    float maxdist = jundist;
    float mindist = decdist;

    if (maxdist < mindist)
    {
        float a = maxdist;
        maxdist = mindist;
        mindist = a;
    }

    float maxdistdiff = maxdist - mindist;

    float tilt = world.tilt();

    float maxtiltadd = tilt * 2.0f;

    for (int thismonth = 0; thismonth < 12; thismonth++)
    {
        float thisdistdiff = (maxdist - world.sundist(thismonth)) / maxdistdiff;

        float tiltadd = maxtiltadd * thisdistdiff;

        float thistilt = tiltadd - tilt;

        if (world.perihelion() == 0)
            thistilt = -thistilt;

        world.setsunlat(thismonth, thistilt);
    }

}

// This creates a simple planet from the main planet object.

void createsimpleplanet(planet& world, simpleplanet& simpleworld, int& progressval, string& progresstext)
{
    updatereport(progressval, progresstext, "Adding details");

    long seed = world.seed();
    fast_srand(seed);

    int edge = world.edge();

    int simpleedge = edge * 4;
    simpleworld.setedge(simpleedge);

    int maxelev = world.maxelevation();
    int sealevel = world.sealevel();

    vector<int>from(6 * edge * edge, 0);
    vector<int>dest(6 * simpleedge * simpleedge, 0);

    vector<int>mountains(6 * simpleedge * simpleedge, 0);
    vector<int>janaddedrain(6 * simpleedge * simpleedge, 0);
    vector<int>juladdedrain(6 * simpleedge * simpleedge, 0);

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < simpleedge; i++)
        {
            for (int j = 0; j < simpleedge; j++)
            {
                simpleworld.setriverjan(face, i, j, 0);
                simpleworld.setriverjul(face, i, j, 0);
            }
        }
    }

    // Clouds.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                from[vi + j] = world.clouds(face, i, j);
            }
        }
    }

    expand(from, dest, edge, simpleedge, 1, maxelev, 0.1f);

    for (int face = 0; face < 6; face++)
    {
        int vface = face * simpleedge * simpleedge;

        for (int i = 0; i < simpleedge; i++)
        {
            int vi = vface + i * simpleedge;

            for (int j = 0; j < simpleedge; j++)
            {
                simpleworld.setclouds(face, i, j, dest[vi + j]);
            }
        }
    }

    // Terrain.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                from[vi + j] = world.nom(face, i, j);
            }
        }
    }

    expand(from, dest, edge, simpleedge, 1, maxelev, 0.002f);

    for (int face = 0; face < 6; face++)
    {
        int vface = face * simpleedge * simpleedge;

        for (int i = 0; i < simpleedge; i++)
        {
            int vi = vface + i * simpleedge;

            for (int j = 0; j < simpleedge; j++)
            {
                simpleworld.setmap(face, i, j, dest[vi + j]);
            }
        }
    }

    // Craters.

    for (int n = 0; n < world.craterno(); n++)
    {
        int face = world.craterface(n);
        int x = world.craterx(n);
        int y = world.cratery(n);

        int elev = world.craterelev(n);
        int radius = world.craterradius(n) * 4;
        int rimelev = elev * 2;

        int simplex = x * 4;
        int simpley = y * 4;

        addsimplemountainpoint(simpleedge, face, simplex, simpley, mountains, elev);

        int sizecheck = radius * radius;// +radius;
        int sizechecksmall = (radius - 1) * (radius - 1);// +radius - 1;

        for (int i = -radius; i <= radius; i++)
        {
            for (int j = -radius; j <= radius; j++)
            {
                int thischeck = i * i + j * j;

                if (thischeck <= sizecheck)
                {
                    globepoint thispoint = getglobepoint(simpleedge, face, simplex, simpley, i, j);

                    if (thispoint.face != -1)
                    {
                        int index = thispoint.face * simpleedge * simpleedge + thispoint.x * simpleedge + thispoint.y;

                        if (thischeck >= sizechecksmall)
                            addsimplemountainpoint(simpleedge, thispoint.face, thispoint.x, thispoint.y, mountains, rimelev);
                        else
                            mountains[index] = 0;
                    }
                }
            }
        }
    }

    // Mountains.

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                int mountainheight = world.mountainheight(face, i, j);

                if (mountainheight != 0)
                {
                    int thisheight = world.mountainheight(face, i, j);
                    int simplei = i * 4;
                    int simplej = j * 4;

                    addsimplemountainpoint(simpleedge, face, simplei + 2, simplej + 2, mountains, thisheight);

                    if (getridge(world, face, i, j, 1) == 1) // North
                    {
                        addsimplemountainpoint(simpleedge, face, simplei + 2, simplej, mountains, thisheight);

                        if (random(1,2)==1)
                            addsimplemountainpoint(simpleedge, face, simplei + 1, simplej + 1, mountains, thisheight);
                        else
                            addsimplemountainpoint(simpleedge, face, simplei + 3, simplej + 1, mountains, thisheight);
                    }

                    if (getridge(world, face, i, j, 2) == 1) // Northeast
                    {
                        addsimplemountainpoint(simpleedge, face, simplei + 4, simplej, mountains, thisheight);

                        int r = random(1, 3);

                        if (r == 1)
                        {
                            addsimplemountainpoint(simpleedge, face, simplei + 3, simplej + 0, mountains, thisheight);
                            addsimplemountainpoint(simpleedge, face, simplei + 2, simplej + 1, mountains, thisheight);
                        }

                        if (r == 2)
                        {
                            addsimplemountainpoint(simpleedge, face, simplei + 3, simplej + 1, mountains, thisheight);

                            int s = random(1, 5);

                            if (s == 1)
                                addsimplemountainpoint(simpleedge, face, simplei + 3, simplej, mountains, thisheight);

                            if (s == 2)
                                addsimplemountainpoint(simpleedge, face, simplei + 4, simplej + 1, mountains, thisheight);

                            if (s == 3)
                                addsimplemountainpoint(simpleedge, face, simplei + 2, simplej + 1, mountains, thisheight);

                            if (s == 4)
                                addsimplemountainpoint(simpleedge, face, simplei + 3, simplej + 2, mountains, thisheight);
                        }

                        if (r == 3)
                        {
                            addsimplemountainpoint(simpleedge, face, simplei + 4, simplej + 1, mountains, thisheight);
                            addsimplemountainpoint(simpleedge, face, simplei + 3, simplej + 2, mountains, thisheight);
                        }
                    }

                    if (getridge(world, face, i, j, 3) == 1) // East
                    {
                        int r = random(1, 3);

                        if (r == 1)
                            addsimplemountainpoint(simpleedge, face, simplei + 3, simplej + 1, mountains, thisheight);

                        if (r == 2)
                            addsimplemountainpoint(simpleedge, face, simplei + 3, simplej + 2, mountains, thisheight);

                        if (r == 3)
                            addsimplemountainpoint(simpleedge, face, simplei + 3, simplej + 3, mountains, thisheight);
                    }

                    if (getridge(world, face, i, j, 4) == 1) // Southeast
                    {
                        addsimplemountainpoint(simpleedge, face, simplei + 4, simplej + 4, mountains, thisheight);

                        int r = random(1, 3);

                        if (r == 1)
                        {
                            addsimplemountainpoint(simpleedge, face, simplei + 3, simplej + 4, mountains, thisheight);
                            addsimplemountainpoint(simpleedge, face, simplei + 2, simplej + 3, mountains, thisheight);
                        }

                        if (r == 2)
                        {
                            addsimplemountainpoint(simpleedge, face, simplei + 3, simplej + 3, mountains, thisheight);

                            int s = random(1, 5);

                            if (s == 1)
                                addsimplemountainpoint(simpleedge, face, simplei + 3, simplej + 4, mountains, thisheight);

                            if (s == 2)
                                addsimplemountainpoint(simpleedge, face, simplei + 4, simplej + 3, mountains, thisheight);

                            if (s == 3)
                                addsimplemountainpoint(simpleedge, face, simplei + 2, simplej + 3, mountains, thisheight);

                            if (s == 4)
                                addsimplemountainpoint(simpleedge, face, simplei + 3, simplej + 2, mountains, thisheight);
                        }

                        if (r == 3)
                        {
                            addsimplemountainpoint(simpleedge, face, simplei + 4, simplej + 3, mountains, thisheight);
                            addsimplemountainpoint(simpleedge, face, simplei + 3, simplej + 2, mountains, thisheight);
                        }
                    }

                    if (getridge(world, face, i, j, 5) == 1) // South
                    {
                        int r = random(1, 3);

                        if (r == 1)
                            addsimplemountainpoint(simpleedge, face, simplei + 1, simplej + 3, mountains, thisheight);

                        if (r == 2)
                            addsimplemountainpoint(simpleedge, face, simplei + 2, simplej + 3, mountains, thisheight);

                        if (r == 3)
                            addsimplemountainpoint(simpleedge, face, simplei + 3, simplej + 3, mountains, thisheight);
                    }

                    if (getridge(world, face, i, j, 6) == 1) // Southwest
                    {
                        addsimplemountainpoint(simpleedge, face, simplei, simplej + 4, mountains, thisheight);

                        int r = random(1, 3);

                        if (r == 1)
                        {
                            addsimplemountainpoint(simpleedge, face, simplei + 1, simplej + 4, mountains, thisheight);
                            addsimplemountainpoint(simpleedge, face, simplei + 2, simplej + 3, mountains, thisheight);
                        }

                        if (r == 2)
                        {
                            addsimplemountainpoint(simpleedge, face, simplei + 3, simplej + 3, mountains, thisheight);

                            int s = random(1, 5);

                            if (s == 1)
                                addsimplemountainpoint(simpleedge, face, simplei + 1, simplej + 4, mountains, thisheight);

                            if (s == 2)
                                addsimplemountainpoint(simpleedge, face, simplei + 0, simplej + 3, mountains, thisheight);

                            if (s == 3)
                                addsimplemountainpoint(simpleedge, face, simplei + 2, simplej + 3, mountains, thisheight);

                            if (s == 4)
                                addsimplemountainpoint(simpleedge, face, simplei + 1, simplej + 2, mountains, thisheight);
                        }

                        if (r == 3)
                        {
                            addsimplemountainpoint(simpleedge, face, simplei, simplej + 3, mountains, thisheight);
                            addsimplemountainpoint(simpleedge, face, simplei + 1, simplej + 2, mountains, thisheight);
                        }
                    }

                    if (getridge(world, face, i, j, 7) == 1) // West
                    {
                        addsimplemountainpoint(simpleedge, face, simplei, simplej + 2, mountains, thisheight);

                        if (random(1, 2) == 1)
                            addsimplemountainpoint(simpleedge, face, simplei + 1, simplej + 1, mountains, thisheight);
                        else
                            addsimplemountainpoint(simpleedge, face, simplei + 1, simplej + 3, mountains, thisheight);
                    }

                    if (getridge(world, face, i, j, 8) == 1) // Northwest
                    {
                        addsimplemountainpoint(simpleedge, face, simplei, simplej, mountains, thisheight);

                        int r = random(1, 3);

                        if (r == 1)
                        {
                            addsimplemountainpoint(simpleedge, face, simplei + 1, simplej + 0, mountains, thisheight);
                            addsimplemountainpoint(simpleedge, face, simplei + 2, simplej + 1, mountains, thisheight);
                        }

                        if (r == 2)
                        {
                            addsimplemountainpoint(simpleedge, face, simplei + 1, simplej + 1, mountains, thisheight);

                            int s = random(1, 5);

                            if (s == 1)
                                addsimplemountainpoint(simpleedge, face, simplei + 1, simplej, mountains, thisheight);

                            if (s == 2)
                                addsimplemountainpoint(simpleedge, face, simplei, simplej + 1, mountains, thisheight);

                            if (s == 3)
                                addsimplemountainpoint(simpleedge, face, simplei + 2, simplej + 1, mountains, thisheight);

                            if (s == 4)
                                addsimplemountainpoint(simpleedge, face, simplei + 1, simplej + 2, mountains, thisheight);
                        }

                        if (r == 3)
                        {
                            addsimplemountainpoint(simpleedge, face, simplei, simplej + 1, mountains, thisheight);
                            addsimplemountainpoint(simpleedge, face, simplei + 1, simplej + 2, mountains, thisheight);
                        }
                    }
                }

                int volcano = abs(world.volcano(face, i, j));

                if (volcano > mountainheight)
                    addsimplemountainpoint(simpleedge, face, i * 4, j * 4, mountains, volcano);
            }
        }
    }

    for (int face = 0; face < 6; face++)
    {
        int vface = face * simpleedge * simpleedge;

        for (int i = 0; i < simpleedge; i++)
        {
            int vi = vface + i * simpleedge;

            for (int j = 0; j < simpleedge; j++)
            {
                if (simpleworld.map(face, i, j) > sealevel)
                {
                    int val = simpleworld.map(face, i, j) + mountains[vi + j];

                    if (val > maxelev)
                        val = maxelev;

                    simpleworld.setmap(face, i, j, val);
                }
            }
        }
    }

    // Jan temperature.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int val = world.jantemp(face, i, j);

                val = tempelevremove(world, val, face, i, j);

                from[vi + j] = val * 50 + 5000;
            }
        }
    }

    expand(from, dest, edge, simpleedge, 1, maxelev, 0.001f); // 0.001f

    for (int face = 0; face < 6; face++)
    {
        int vface = face * simpleedge * simpleedge;

        for (int i = 0; i < simpleedge; i++)
        {
            int vi = vface + i * simpleedge;

            for (int j = 0; j < simpleedge; j++)
            {
                int val = (dest[vi + j] - 5000) / 50;

                val = tempelevadd(world, simpleworld, val, face, i, j);

                //val = world.jantemp(face, i / 4, j / 4);

                simpleworld.setjantemp(face, i, j, val);
            }
        }
    }

    // July temperature.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                int val = world.jultemp(face, i, j);

                val = tempelevremove(world, val, face, i, j);

                from[vi + j] = val * 50 + 5000;
            }
        }
    }

    expand(from, dest, edge, simpleedge, 1, maxelev, 0.001f); // 0.001f

    for (int face = 0; face < 6; face++)
    {
        int vface = face * simpleedge * simpleedge;

        for (int i = 0; i < simpleedge; i++)
        {
            int vi = vface + i * simpleedge;

            for (int j = 0; j < simpleedge; j++)
            {
                int val = (dest[vi + j] - 5000) / 50;

                val = tempelevadd(world, simpleworld, val, face, i, j);

                //val = world.jultemp(face, i / 4, j / 4);

                simpleworld.setjultemp(face, i, j, val);
            }
        }
    }

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                int minjanseatemp = 100000;
                int maxjanseatemp = -100000;
                int minjanlandtemp = 100000;
                int maxjanlandtemp = -100000;

                int minjulseatemp = 100000;
                int maxjulseatemp = -100000;
                int minjullandtemp = 100000;
                int maxjullandtemp = -100000;

                for (int k = -1; k <= 1; k++)
                {
                    for (int l = -1; l <= 1; l++)
                    {
                        globepoint thispoint = getglobepoint(edge, face, i, j, k, l);

                        if (thispoint.face != -1)
                        {
                            int thisjantemp = world.jantemp(thispoint.face, thispoint.x, thispoint.y);
                            int thisjultemp = world.jultemp(thispoint.face, thispoint.x, thispoint.y);

                            if (world.sea(thispoint.face, thispoint.x, thispoint.y))
                            {
                                if (thisjantemp < minjanseatemp)
                                    minjanseatemp = thisjantemp;

                                if (thisjantemp > maxjanseatemp)
                                    maxjanseatemp = thisjantemp;

                                if (thisjultemp < minjulseatemp)
                                    minjulseatemp = thisjultemp;

                                if (thisjultemp > maxjulseatemp)
                                    maxjulseatemp = thisjultemp;
                            }
                            else
                            {
                                if (thisjantemp < minjanlandtemp)
                                    minjanlandtemp = thisjantemp;

                                if (thisjantemp > maxjanlandtemp)
                                    maxjanlandtemp = thisjantemp;

                                if (thisjultemp < minjullandtemp)
                                    minjullandtemp = thisjultemp;

                                if (thisjultemp > maxjullandtemp)
                                    maxjullandtemp = thisjultemp;
                            }
                        }
                    }
                }

                int ii = i * 4;
                int jj = j * 4;

                for (int k = 0; k < 4; k++)
                {
                    for (int l = 0; l < 4; l++)
                    {
                        if (simpleworld.map(face, ii + k, jj + l) <= sealevel)
                        {
                            if (simpleworld.jantemp(face, ii + k, jj + l) < minjanseatemp && minjanseatemp != 100000)
                                simpleworld.setjantemp(face, ii + k, jj + l, minjanseatemp);

                            if (simpleworld.jantemp(face, ii + k, jj + l) > maxjanseatemp && maxjanseatemp != -100000)
                                simpleworld.setjantemp(face, ii + k, jj + l, maxjanseatemp);

                            if (simpleworld.jultemp(face, ii + k, jj + l) < minjulseatemp && minjulseatemp != 100000)
                                simpleworld.setjultemp(face, ii + k, jj + l, minjulseatemp);

                            if (simpleworld.jultemp(face, ii + k, jj + l) > maxjulseatemp && maxjulseatemp != -100000)
                                simpleworld.setjultemp(face, ii + k, jj + l, maxjulseatemp);
                        }
                        else
                        {
                            if (simpleworld.jantemp(face, ii + k, jj + l) < minjanlandtemp && minjanlandtemp != 100000)
                                simpleworld.setjantemp(face, ii + k, jj + l, minjanlandtemp);

                            if (simpleworld.jantemp(face, ii + k, jj + l) > maxjanlandtemp && maxjanlandtemp != -100000)
                                simpleworld.setjantemp(face, ii + k, jj + l, maxjanlandtemp);

                            if (simpleworld.jultemp(face, ii + k, jj + l) < minjullandtemp && minjullandtemp != 100000)
                                simpleworld.setjultemp(face, ii + k, jj + l, minjullandtemp);

                            if (simpleworld.jultemp(face, ii + k, jj + l) > maxjullandtemp && maxjullandtemp != -100000)
                                simpleworld.setjultemp(face, ii + k, jj + l, maxjullandtemp);
                        }
                    }
                }
            }
        }
    }

    // Sea temperature reduction.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                from[vi + j] = world.seatempreduce(face, i, j) + 5000;
            }
        }
    }

    expand(from, dest, edge, simpleedge, 1, maxelev, 0.006f); // 0.002f

    for (int face = 0; face < 6; face++)
    {
        int vface = face * simpleedge * simpleedge;

        for (int i = 0; i < simpleedge; i++)
        {
            int vi = vface + i * simpleedge;

            for (int j = 0; j < simpleedge; j++)
            {
                int val = dest[vi + j] - 5000;

                if (val < 0)
                    val = 0;

                simpleworld.setseatempreduce(face, i, j, val);
            }
        }
    }

    // Jan rain.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                from[vi + j] = world.janrain(face, i, j);
            }
        }
    }

    expand(from, dest, edge, simpleedge, 0, maxelev, 0.007f); //0.007f

    for (int face = 0; face < 6; face++)
    {
        int vface = face * simpleedge * simpleedge;

        for (int i = 0; i < simpleedge; i++)
        {
            int vi = vface + i * simpleedge;

            for (int j = 0; j < simpleedge; j++)
            {
                simpleworld.setjanrain(face, i, j, dest[vi + j]);
            }
        }
    }

    // Jul rain.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                from[vi + j] = world.julrain(face, i, j);
            }
        }
    }

    expand(from, dest, edge, simpleedge, 0, maxelev, 0.007f); // 0.007f

    for (int face = 0; face < 6; face++)
    {
        int vface = face * simpleedge * simpleedge;

        for (int i = 0; i < simpleedge; i++)
        {
            int vi = vface + i * simpleedge;

            for (int j = 0; j < simpleedge; j++)
            {
                simpleworld.setjulrain(face, i, j, dest[vi + j]);
            }
        }
    }

    // Rivers.

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < edge; i++)
        {
            for (int j = 0; j < edge; j++)
            {
                if (world.sea(face, i, j) == 0 && world.riverjan(face, i, j) != 0 || world.riverjul(face, i, j) != 0)
                {
                    int simplei = i * 4;
                    int simplej = j * 4;

                    // Work out how much flow is coming in/out in all directions

                    int flow[9][2];

                    for (int n = 1; n <= 8; n++)
                    {
                        flow[n][0] = 0;
                        flow[n][1] = 0;
                    }

                    int outdir = world.riverdir(face, i, j);

                    if (outdir > 0 && outdir < 9)
                    {
                        flow[outdir][0] = world.riverjan(face, i, j);
                        flow[outdir][1] = world.riverjul(face, i, j);

                        if (i > 0 && i < edge - 1 && j > 0 && j < edge - 1)
                        {
                            if (world.riverdir(face, i, j - 1) == 5)
                            {
                                flow[1][0] = world.riverjan(face, i, j - 1);
                                flow[1][1] = world.riverjul(face, i, j - 1);
                            }

                            if (world.riverdir(face, i + 1, j - 1) == 6)
                            {
                                flow[2][0] = world.riverjan(face, i + 1, j - 1);
                                flow[2][1] = world.riverjul(face, i + 1, j - 1);
                            }

                            if (world.riverdir(face, i + 1, j) == 7)
                            {
                                flow[3][0] = world.riverjan(face, i + 1, j);
                                flow[3][1] = world.riverjul(face, i + 1, j);
                            }

                            if (world.riverdir(face, i + 1, j + 1) == 8)
                            {
                                flow[4][0] = world.riverjan(face, i + 1, j + 1);
                                flow[4][1] = world.riverjul(face, i + 1, j + 1);
                            }

                            if (world.riverdir(face, i, j + 1) == 1)
                            {
                                flow[5][0] = world.riverjan(face, i, j + 1);
                                flow[5][1] = world.riverjul(face, i, j + 1);
                            }

                            if (world.riverdir(face, i - 1, j + 1) == 2)
                            {
                                flow[6][0] = world.riverjan(face, i - 1, j + 1);
                                flow[6][1] = world.riverjul(face, i - 1, j + 1);
                            }

                            if (world.riverdir(face, i - 1, j) == 3)
                            {
                                flow[7][0] = world.riverjan(face, i - 1, j);
                                flow[7][1] = world.riverjul(face, i - 1, j);
                            }

                            if (world.riverdir(face, i - 1, j - 1) == 4)
                            {
                                flow[8][0] = world.riverjan(face, i - 1, j - 1);
                                flow[8][1] = world.riverjul(face, i - 1, j - 1);
                            }
                        }
                        else
                        {
                            globepoint northpoint = getglobepoint(edge, face, i, j, 0, -1);
                            globepoint northeastpoint = getglobepoint(edge, face, i, j, 1, -1);
                            globepoint eastpoint = getglobepoint(edge, face, i, j, 1, 0);
                            globepoint southeastpoint = getglobepoint(edge, face, i, j, 1, 1);
                            globepoint southpoint = getglobepoint(edge, face, i, j, 0, 1);
                            globepoint southwestpoint = getglobepoint(edge, face, i, j, -1, 1);
                            globepoint westpoint = getglobepoint(edge, face, i, j, -1, 0);
                            globepoint northwestpoint = getglobepoint(edge, face, i, j, -1, -1);

                            if (world.riverdir(northpoint.face, northpoint.x, northpoint.y) == 5)
                            {
                                flow[1][0] = world.riverjan(northpoint.face, northpoint.x, northpoint.y);
                                flow[1][1] = world.riverjul(northpoint.face, northpoint.x, northpoint.y);
                            }

                            if (world.riverdir(northeastpoint.face, northeastpoint.x, northeastpoint.y) == 6)
                            {
                                flow[2][0] = world.riverjan(northeastpoint.face, northeastpoint.x, northeastpoint.y);
                                flow[2][1] = world.riverjul(northeastpoint.face, northeastpoint.x, northeastpoint.y);
                            }

                            if (world.riverdir(eastpoint.face, eastpoint.x, eastpoint.y) == 7)
                            {
                                flow[3][0] = world.riverjan(eastpoint.face, eastpoint.x, eastpoint.y);
                                flow[3][1] = world.riverjul(eastpoint.face, eastpoint.x, eastpoint.y);
                            }

                            if (world.riverdir(southeastpoint.face, southeastpoint.x, southeastpoint.y) == 8)
                            {
                                flow[4][0] = world.riverjan(southeastpoint.face, southeastpoint.x, southeastpoint.y);
                                flow[4][1] = world.riverjul(southeastpoint.face, southeastpoint.x, southeastpoint.y);
                            }

                            if (world.riverdir(southpoint.face, southpoint.x, southpoint.y) == 1)
                            {
                                flow[5][0] = world.riverjan(southpoint.face, southpoint.x, southpoint.y);
                                flow[5][1] = world.riverjul(southpoint.face, southpoint.x, southpoint.y);
                            }

                            if (world.riverdir(southwestpoint.face, southwestpoint.x, southwestpoint.y) == 2)
                            {
                                flow[6][0] = world.riverjan(southwestpoint.face, southwestpoint.x, southwestpoint.y);
                                flow[6][1] = world.riverjul(southwestpoint.face, southwestpoint.x, southwestpoint.y);
                            }

                            if (world.riverdir(westpoint.face, westpoint.x, westpoint.y) == 3)
                            {
                                flow[7][0] = world.riverjan(westpoint.face, westpoint.x, westpoint.y);
                                flow[7][1] = world.riverjul(westpoint.face, westpoint.x, westpoint.y);
                            }

                            if (world.riverdir(northwestpoint.face, northwestpoint.x, northwestpoint.y) == 4)
                            {
                                flow[8][0] = world.riverjan(northwestpoint.face, northwestpoint.x, northwestpoint.y);
                                flow[8][1] = world.riverjul(northwestpoint.face, northwestpoint.x, northwestpoint.y);
                            }
                        }

                        // Now draw the rivers in those directions.
                        
                        addsimpleriverpoint(simpleworld, face, simplei + 2, simplej +2, flow[outdir][0], flow[outdir][1], janaddedrain, juladdedrain);
                       
                        if (flow[1][0] > 0 || flow[1][1] > 0) // North
                        {
                            addsimpleriverpoint(simpleworld, face, simplei + 2, simplej, flow[1][0], flow[1][1], janaddedrain, juladdedrain);

                            if (random(1, 2) == 1)
                                addsimpleriverpoint(simpleworld, face, simplei + 1, simplej + 1, flow[1][0], flow[1][1], janaddedrain, juladdedrain);
                            else
                                addsimpleriverpoint(simpleworld, face, simplei + 3, simplej + 1, flow[1][0], flow[1][1], janaddedrain, juladdedrain);
                        }

                        if (flow[2][0] > 0 || flow[2][1] > 0) // Northeast
                        {
                            addsimpleriverpoint(simpleworld, face, simplei + 4, simplej, flow[2][0], flow[2][1], janaddedrain, juladdedrain);

                            int r = random(1, 3);

                            if (r == 1)
                            {
                                addsimpleriverpoint(simpleworld, face, simplei + 3, simplej + 0, flow[2][0], flow[2][1], janaddedrain, juladdedrain);
                                addsimpleriverpoint(simpleworld, face, simplei + 2, simplej + 1, flow[2][0], flow[2][1], janaddedrain, juladdedrain);
                            }

                            if (r == 2)
                            {
                                addsimpleriverpoint(simpleworld, face, simplei + 3, simplej + 1, flow[2][0], flow[2][1], janaddedrain, juladdedrain);

                                int s = random(1, 5);

                                if (s == 1)
                                    addsimpleriverpoint(simpleworld, face, simplei + 3, simplej, flow[2][0], flow[2][1], janaddedrain, juladdedrain);

                                if (s == 2)
                                    addsimpleriverpoint(simpleworld, face, simplei + 4, simplej + 1, flow[2][0], flow[2][1], janaddedrain, juladdedrain);

                                if (s == 3)
                                    addsimpleriverpoint(simpleworld, face, simplei + 2, simplej + 1, flow[2][0], flow[2][1], janaddedrain, juladdedrain);

                                if (s == 4)
                                    addsimpleriverpoint(simpleworld, face, simplei + 3, simplej + 2, flow[2][0], flow[2][1], janaddedrain, juladdedrain);
                            }

                            if (r == 3)
                            {
                                addsimpleriverpoint(simpleworld, face, simplei + 4, simplej + 1, flow[2][0], flow[2][1], janaddedrain, juladdedrain);
                                addsimpleriverpoint(simpleworld, face, simplei + 3, simplej + 2, flow[2][0], flow[2][1], janaddedrain, juladdedrain);
                            }
                        }

                        if (flow[3][0] > 0 || flow[3][1] > 0) // East
                        {
                            int r = random(1, 3);

                            if (r == 1)
                                addsimpleriverpoint(simpleworld, face, simplei + 3, simplej + 1, flow[3][0], flow[3][1], janaddedrain, juladdedrain);

                            if (r == 2)
                                addsimpleriverpoint(simpleworld, face, simplei + 3, simplej + 2, flow[3][0], flow[3][1], janaddedrain, juladdedrain);

                            if (r == 3)
                                addsimpleriverpoint(simpleworld, face, simplei + 3, simplej + 3, flow[3][0], flow[3][1], janaddedrain, juladdedrain);
                        }

                        if (flow[4][0] > 0 || flow[4][1] > 0) // Southeast
                        {
                            addsimpleriverpoint(simpleworld, face, simplei + 4, simplej + 4, flow[4][0], flow[4][1], janaddedrain, juladdedrain);

                            int r = random(1, 3);

                            if (r == 1)
                            {
                                addsimpleriverpoint(simpleworld, face, simplei + 3, simplej + 4, flow[4][0], flow[4][1], janaddedrain, juladdedrain);
                                addsimpleriverpoint(simpleworld, face, simplei + 2, simplej + 3, flow[4][0], flow[4][1], janaddedrain, juladdedrain);
                            }

                            if (r == 2)
                            {
                                addsimpleriverpoint(simpleworld, face, simplei + 3, simplej + 3, flow[4][0], flow[4][1], janaddedrain, juladdedrain);

                                int s = random(1, 5);

                                if (s == 1)
                                    addsimpleriverpoint(simpleworld, face, simplei + 3, simplej + 4, flow[4][0], flow[4][1], janaddedrain, juladdedrain);

                                if (s == 2)
                                    addsimpleriverpoint(simpleworld, face, simplei + 4, simplej + 3, flow[4][0], flow[4][1], janaddedrain, juladdedrain);

                                if (s == 3)
                                    addsimpleriverpoint(simpleworld, face, simplei + 2, simplej + 3, flow[4][0], flow[4][1], janaddedrain, juladdedrain);

                                if (s == 4)
                                    addsimpleriverpoint(simpleworld, face, simplei + 3, simplej + 2, flow[4][0], flow[4][1], janaddedrain, juladdedrain);
                            }

                            if (r == 3)
                            {
                                addsimpleriverpoint(simpleworld, face, simplei + 4, simplej + 3, flow[4][0], flow[4][1], janaddedrain, juladdedrain);
                                addsimpleriverpoint(simpleworld, face, simplei + 3, simplej + 2, flow[4][0], flow[4][1], janaddedrain, juladdedrain);
                            }
                        }

                        if (flow[5][0] > 0 || flow[5][1] > 0) // South
                        {
                            int r = random(1, 3);

                            if (r == 1)
                                addsimpleriverpoint(simpleworld, face, simplei + 1, simplej + 3, flow[5][0], flow[5][1], janaddedrain, juladdedrain);

                            if (r == 2)
                                addsimpleriverpoint(simpleworld, face, simplei + 2, simplej + 3, flow[5][0], flow[5][1], janaddedrain, juladdedrain);

                            if (r == 3)
                                addsimpleriverpoint(simpleworld, face, simplei + 3, simplej + 3, flow[5][0], flow[5][1], janaddedrain, juladdedrain);
                        }

                        if (flow[6][0] > 0 || flow[6][1] > 0) // Southwest
                        {
                            addsimpleriverpoint(simpleworld, face, simplei, simplej + 4, flow[6][0], flow[6][1], janaddedrain, juladdedrain);

                            int r = random(1, 3);

                            if (r == 1)
                            {
                                addsimpleriverpoint(simpleworld, face, simplei + 1, simplej + 4, flow[6][0], flow[6][1], janaddedrain, juladdedrain);
                                addsimpleriverpoint(simpleworld, face, simplei + 2, simplej + 3, flow[6][0], flow[6][1], janaddedrain, juladdedrain);
                            }

                            if (r == 2)
                            {
                                addsimpleriverpoint(simpleworld, face, simplei + 3, simplej + 3, flow[6][0], flow[6][1], janaddedrain, juladdedrain);

                                int s = random(1, 5);

                                if (s == 1)
                                    addsimpleriverpoint(simpleworld, face, simplei + 1, simplej + 4, flow[6][0], flow[6][1], janaddedrain, juladdedrain);

                                if (s == 2)
                                    addsimpleriverpoint(simpleworld, face, simplei + 0, simplej + 3, flow[6][0], flow[6][1], janaddedrain, juladdedrain);

                                if (s == 3)
                                    addsimpleriverpoint(simpleworld, face, simplei + 2, simplej + 3, flow[6][0], flow[6][1], janaddedrain, juladdedrain);

                                if (s == 4)
                                    addsimpleriverpoint(simpleworld, face, simplei + 1, simplej + 2, flow[6][0], flow[6][1], janaddedrain, juladdedrain);
                            }

                            if (r == 3)
                            {
                                addsimpleriverpoint(simpleworld, face, simplei, simplej + 3, flow[6][0], flow[6][1], janaddedrain, juladdedrain);
                                addsimpleriverpoint(simpleworld, face, simplei + 1, simplej + 2, flow[6][0], flow[6][1], janaddedrain, juladdedrain);
                            }
                        }

                        if (flow[7][0] > 0 || flow[7][1] > 0) // West
                        {
                            addsimpleriverpoint(simpleworld, face, simplei, simplej + 2, flow[7][0], flow[7][1], janaddedrain, juladdedrain);

                            if (random(1, 2) == 1)
                                addsimpleriverpoint(simpleworld, face, simplei + 1, simplej + 1, flow[7][0], flow[7][1], janaddedrain, juladdedrain);
                            else
                                addsimpleriverpoint(simpleworld, face, simplei + 1, simplej + 3, flow[7][0], flow[7][1], janaddedrain, juladdedrain);
                        }

                        if (flow[8][0] > 0 || flow[8][1] > 0) // Northwest
                        {
                            addsimpleriverpoint(simpleworld, face, simplei, simplej, flow[8][0], flow[8][1], janaddedrain, juladdedrain);

                            int r = random(1, 3);

                            if (r == 1)
                            {
                                addsimpleriverpoint(simpleworld, face, simplei + 1, simplej + 0, flow[8][0], flow[8][1], janaddedrain, juladdedrain);
                                addsimpleriverpoint(simpleworld, face, simplei + 2, simplej + 1, flow[8][0], flow[8][1], janaddedrain, juladdedrain);
                            }

                            if (r == 2)
                            {
                                addsimpleriverpoint(simpleworld, face, simplei + 1, simplej + 1, flow[8][0], flow[8][1], janaddedrain, juladdedrain);

                                int s = random(1, 5);

                                if (s == 1)
                                    addsimpleriverpoint(simpleworld, face, simplei + 1, simplej, flow[8][0], flow[8][1], janaddedrain, juladdedrain);

                                if (s == 2)
                                    addsimpleriverpoint(simpleworld, face, simplei, simplej + 1, flow[8][0], flow[8][1], janaddedrain, juladdedrain);

                                if (s == 3)
                                    addsimpleriverpoint(simpleworld, face, simplei + 2, simplej + 1, flow[8][0], flow[8][1], janaddedrain, juladdedrain);

                                if (s == 4)
                                    addsimpleriverpoint(simpleworld, face, simplei + 1, simplej + 2, flow[8][0], flow[8][1], janaddedrain, juladdedrain);
                            }

                            if (r == 3)
                            {
                                addsimpleriverpoint(simpleworld, face, simplei, simplej + 1, flow[8][0], flow[8][1], janaddedrain, juladdedrain);
                                addsimpleriverpoint(simpleworld, face, simplei + 1, simplej + 2, flow[8][0], flow[8][1], janaddedrain, juladdedrain);
                            }
                        }
                    }
                }
            }
        }
    }

    // Lakes.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * edge * edge;

        for (int i = 0; i < edge; i++)
        {
            int vi = vface + i * edge;

            for (int j = 0; j < edge; j++)
            {
                if (world.special(face, i, j) != 0 || world.lakesurface(face, i, j) != 0 || world.riftlakesurface(face, i, j) != 0)
                    from[vi + j] = sealevel + 400;
                else
                    from[vi + j] = sealevel - 400;

            }
        }
    }

    expand(from, dest, edge, simpleedge, 1, maxelev, 0.01f);

    for (int face = 0; face < 6; face++)
    {
        int vface = face * simpleedge * simpleedge;

        for (int i = 0; i < simpleedge; i++)
        {
            int vi = vface + i * simpleedge;

            for (int j = 0; j < simpleedge; j++)
            {
                if (dest[vi + j] >= sealevel)
                    simpleworld.setlake(face, i, j, 1);
                else
                    simpleworld.setlake(face, i, j, 0);
            }
        }
    }

    for (int face = 0; face < 6; face++) // Remove odd isolated bits
    {
        int vface = face * simpleedge * simpleedge;

        for (int i = 1; i < simpleedge - 1; i++)
        {
            int vi = vface + i * simpleedge;

            for (int j = 1; j < simpleedge - 1; j++)
            {
                if (simpleworld.lake(face, i, j))
                {
                    if (simpleworld.lake(face, i - 1, j) == 0 && simpleworld.lake(face, i + 1, j) == 0 && simpleworld.lake(face, i, j - 1) == 0 && simpleworld.lake(face, i, j + 1) == 0)
                        simpleworld.setlake(face, i, j, 0);
                }
            }
        }
    }
}

// This puts a point of mountain onto the simple world.

void addsimplemountainpoint(int edge, int face, int x, int y, vector<int>& mountains, int height)
{
    if (x >= edge || y >=edge)
    {
        int xshift = x + 1 - edge;
        int yshift = y + 1 - edge;

        globepoint newpoint = getglobepoint(edge, face, x, y, xshift, yshift);

        face = newpoint.face;

        if (face == -1)
            return;

        x = newpoint.x;
        y = newpoint.y;
    }

    int eedge = edge * edge;

    int index = face * eedge + x * edge + y;

    int heightfrac = (int)((float)height / 20.0f);

    if (heightfrac < 1)
        heightfrac = 1;

    if (mountains[index] < height)
        mountains[index] = height;

    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            if (i != 0 || j != 0)
            {
                globepoint newpoint = getglobepoint(edge, face, x, y, i, j);

                if (newpoint.face != -1)
                {
                    int newindex = newpoint.face * eedge + newpoint.x * edge + newpoint.y;

                    int newheight;
                    
                    if (i == 0 || j == 0)
                        newheight = height / 2 + randomsign(random(1, heightfrac));
                    else
                        newheight = height / 6 + randomsign(random(1, heightfrac));

                    float mult = (float)(100 + randomsign(random(1, 50)));
                    mult = mult / 100.0f;

                    newheight = (int)((float)newheight * mult);

                    if (mountains[newindex] < newheight)
                        mountains[newindex] = newheight;
                }
            }
        }
    }
}

// This puts a point of river onto the simple world.

void addsimpleriverpoint(simpleplanet& simpleworld, int face, int x, int y, int janflow, int julflow, vector<int>& addedjanrain, vector<int>& addedjulrain)
{
    if (janflow > 25000)
        janflow = 25000;

    if (julflow > 25000)
        julflow = 25000;

    bool added = 0;

    if (simpleworld.riverjan(face, x, y) < janflow)
    {
        simpleworld.setriverjan(face, x, y, janflow);
        added = 1;
    }

    if (simpleworld.riverjul(face, x, y) < julflow)
    {
        simpleworld.setriverjul(face, x, y, julflow);
        added = 1;
    }

    //if (added == 0)
        return;

    // Now increase the rainfall around this point, so it will look greener. (Turned off for now.)

    float janmult = (float)simpleworld.janrain(face, x, y);
    float julmult = (float)simpleworld.julrain(face, x, y);

    if (janmult < 1.0f)
        janmult = 1.0f;

    if (janmult > 1000.0f)
        janmult = 1000.0f;

    if (julmult < 1.0f)
        julmult = 1.0f;

    if (julmult > 1000.0f)
        julmult = 1000.0f;

    int janadd = (int)(((float)janflow / janmult) * 0.01f);
    int juladd = (int)(((float)julflow / julmult) * 0.01f);

    int edge = simpleworld.edge();
    int eedge = edge * edge;

    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            globepoint newpoint = getglobepoint(edge, face, x, y, i, j);

            if (newpoint.face != -1)
            {
                int newindex = newpoint.face * eedge + newpoint.x * edge + newpoint.y;

                int newjanadd = janadd;
                int newjuladd = juladd;

                if ((i == -1 && j == -1) || (i == 1 && j == -1) || (i == 1 && j == 1) || (i == -1 && j == 1))
                {
                    newjanadd = newjanadd / 2;
                    newjuladd = newjuladd / 2;
                }

                if (addedjanrain[newindex] < newjanadd)
                    addedjanrain[newindex] = newjanadd;

                if (addedjulrain[newindex] < newjuladd)
                    addedjulrain[newindex] = newjuladd;
            }
        }
    }
}

// This creates a table of proportions for expanding details on the space map.

void createproportionstable(vector<float>& proportions)
{
    // 0,0

    proportions[0] = 1.0f;

    // 1,0

    proportions[1 * 4 * 4 + 0 + 0] = 0.75f;
    proportions[1 * 4 * 4 + 0 + 1] = 0.25;

    // 2,0

    proportions[2 * 4 * 4 + 0 + 0] = 0.5f;
    proportions[2 * 4 * 4 + 0 + 1] = 0.5f;

    // 3,0

    proportions[3 * 4 * 4 + 0 + 0] = 0.25f;
    proportions[3 * 4 * 4 + 0 + 1] = 0.75f;

    // 0,1

    proportions[0 + 1 * 4 + 0] = 0.75f;
    proportions[0 + 1 * 4 + 3] = 0.25f;

    // 1,1

    proportions[1 * 4 * 4 + 1 * 4 + 0] = 0.5625f;
    proportions[1 * 4 * 4 + 1 * 4 + 1] = 0.1875f;
    proportions[1 * 4 * 4 + 1 * 4 + 2] = 0.0625f;
    proportions[1 * 4 * 4 + 1 * 4 + 3] = 0.1875f;

    // 2,1

    proportions[2 * 4 * 4 + 1 * 4 + 0] = 0.375f;
    proportions[2 * 4 * 4 + 1 * 4 + 1] = 0.375f;
    proportions[2 * 4 * 4 + 1 * 4 + 2] = 0.125;
    proportions[2 * 4 * 4 + 1 * 4 + 3] = 0.125;

    // 3,1

    proportions[3 * 4 * 4 + 1 * 4 + 0] = 0.1875f;
    proportions[3 * 4 * 4 + 1 * 4 + 1] = 0.5625f;
    proportions[3 * 4 * 4 + 1 * 4 + 2] = 0.1875f;
    proportions[3 * 4 * 4 + 1 * 4 + 3] = 0.0625f;

    // 0,2

    proportions[0 + 2 * 4 + 0] = 0.5f;
    proportions[0 + 2 * 4 + 3] = 0.5f;

    // 1,2

    proportions[1 * 4 * 4 + 2 * 4 + 0] = 0.375f;
    proportions[1 * 4 * 4 + 2 * 4 + 1] = 0.125;
    proportions[1 * 4 * 4 + 2 * 4 + 2] = 0.125;
    proportions[1 * 4 * 4 + 2 * 4 + 3] = 0.375f;

    // 2,2

    proportions[2 * 4 * 4 + 2 * 4 + 0] = 0.25f;
    proportions[2 * 4 * 4 + 2 * 4 + 1] = 0.25f;
    proportions[2 * 4 * 4 + 2 * 4 + 2] = 0.25f;
    proportions[2 * 4 * 4 + 2 * 4 + 3] = 0.25f;

    // 3,2

    proportions[3 * 4 * 4 + 2 * 4 + 0] = 0.125;
    proportions[3 * 4 * 4 + 2 * 4 + 1] = 0.375f;
    proportions[3 * 4 * 4 + 2 * 4 + 2] = 0.375f;
    proportions[3 * 4 * 4 + 2 * 4 + 3] = 0.125;

    // 0,3

    proportions[0 + 3 * 4 + 0] = 0.25f;
    proportions[0 + 3 * 4 + 3] = 0.75f;

    // 1,3

    proportions[1 * 4 * 4 + 3 * 4 + 0] = 0.1875f;
    proportions[1 * 4 * 4 + 3 * 4 + 1] = 0.0625f;
    proportions[1 * 4 * 4 + 3 * 4 + 2] = 0.1875f;
    proportions[1 * 4 * 4 + 3 * 4 + 3] = 0.5625f;

    // 2,3

    proportions[2 * 4 * 4 + 3 * 4 + 0] = 0.125;
    proportions[2 * 4 * 4 + 3 * 4 + 1] = 0.125;
    proportions[2 * 4 * 4 + 3 * 4 + 2] = 0.375f;
    proportions[2 * 4 * 4 + 3 * 4 + 3] = 0.375f;

    // 3,3

    proportions[3 * 4 * 4 + 3 * 4 + 0] = 0.0625f;
    proportions[3 * 4 * 4 + 3 * 4 + 1] = 0.1875f;
    proportions[3 * 4 * 4 + 3 * 4 + 2] = 0.5625f;
    proportions[3 * 4 * 4 + 3 * 4 + 3] = 0.1875f;
}

// Gets monthly temperatures for a point.

void monthlytemps(planet& world, int face, int x, int y, float arr[12], vector<float>& latitude, float eqtemp, float jantemp, float jultemp)
{
    float maxdist = world.sundist(5);
    float mindist = world.sundist(11);

    if (maxdist < mindist)
    {
        float a = maxdist;
        maxdist = mindist;
        mindist = a;
    }

    float eqdist = (maxdist + mindist) / 2.0f;
    //float eqtemp = (float)equinoxtemp(face, x, y, latitude);

    arr[0] = jantemp;
    arr[6] = jultemp;

    for (int n = 1; n < 12; n++)
    {
        if (n != 6)
        {
            int thismonth = n - 1; // Because temperature lags behind the sun's position.

            float thisdist = world.sundist(thismonth);

            if (thisdist == eqdist) // If we're exactly at the equinox!
                arr[n] = eqtemp;
            else
            {
                float maxdistdiff = maxdist - eqdist;

                float neartemp = jantemp;
                float fartemp = jultemp;

                if (world.perihelion() == (int)1)
                {
                    neartemp = jultemp;
                    fartemp = jantemp;
                }

                if (thisdist < eqdist) // If the planet is closer than average to the sun
                {
                    float maxtempdiff = eqtemp - neartemp;

                    float maxdiff = eqdist - mindist;

                    float thisdistdiff = (thisdist - mindist) / maxdiff;

                    float thistemp = neartemp + maxtempdiff * thisdistdiff;

                    arr[n] = thistemp;
                }
                else // If the planet is further than average from the sun
                {
                    float maxtempdiff = fartemp - eqtemp;

                    float maxdiff = maxdist - eqdist;

                    float thisdistdiff = (thisdist - eqdist) / maxdiff;

                    float thistemp = eqtemp + maxtempdiff * thisdistdiff;

                    arr[n] = thistemp;
                }
            }
        }
    }
}

// Gets monthly rain for a point.

void monthlyrain(planet& world, int face, int x, int y, float temp[12], float rain[12], float janrain, float julrain)
{
    float jantemp = temp[0];
    float jultemp = temp[6];

    float maxtempdiff = jultemp - jantemp;

    float maxrain = julrain;
    float minrain = janrain;

    if (janrain > julrain)
    {
        maxrain = janrain;
        minrain = julrain;
    }

    float maxraindiff = julrain - janrain;

    rain[0] = janrain;
    rain[6] = julrain;

    float maxlatdiff = world.sunlat(6) - world.sunlat(0);

    // First, calculate a simple rain variation based on the months. We will later blend this with the more complex calculation.

    float simplerain[12];
    simplerain[0] = rain[0];
    simplerain[6] = rain[6];

    /*
    float simplefactor = maxraindiff / 300.0f;
    simplefactor = simplefactor * simplefactor;
    simplefactor = 1.0f - simplefactor;

    if (simplefactor < 0.0f)
        simplefactor = 0.0f;

    //simplefactor = simplefactor * 0.3f;

    //simplefactor = 1.0f;

    if (simplefactor > 0.0f)
    {
        */
    for (int n = 1; n < 12; n++)
    {
        if (n != 6)
        {
            if (world.sunlat(0) != world.sunlat(6)) // Do this by sun latitude.
            {
                float thislatdiff = world.sunlat(6) - world.sunlat(n);

                float factor = thislatdiff / maxlatdiff;
                float raintoremove = maxraindiff * factor;

                simplerain[n] = julrain - raintoremove;

            }
            else // Just do it by time of year.
            {
                float janstrength = 1.0f;
                float julstrength = 1.0f;

                if (n == 1 || n == 11)
                {
                    janstrength = 5.0f;
                    julstrength = 1.0f;
                }

                if (n == 2 || n == 10)
                {
                    janstrength = 4.0f;
                    julstrength = 2.0f;
                }

                if (n == 3 || n == 9)
                {
                    janstrength = 3.0f;
                    julstrength = 3.0f;
                }

                if (n == 4 || n == 8)
                {
                    janstrength = 2.0f;
                    julstrength = 4.0f;
                }

                if (n == 5 || n == 7)
                {
                    janstrength = 1.0f;
                    julstrength = 5.0f;
                }

                rain[n] = (janrain * janstrength + julrain * julstrength) / 6.0f;
            }
        }
    }
    //}

    for (int n = 1; n < 12; n++)
    {
        if (n != 6)
        {
            /*
            if (maxtempdiff != 0.0f) // Do this by temperature if possible.
            {
                float thistempdiff = jultemp - temp[n];

                float factor = thistempdiff / maxtempdiff;
                float raintoremove = maxraindiff * factor;

                rain[n] = julrain - raintoremove;

                if (simplefactor > 0.0f)
                    rain[n] = rain[n] * (1.0f - simplefactor) + simplerain[n] * simplefactor;
            }
            else
                */
            rain[n] = simplerain[n];

        }
    }

    if ((jantemp > jultemp && janrain > julrain) || (jultemp > jantemp && julrain > janrain)) // Monsoon!
    {
        float monstrength = 1.0f; // The less seasonal variation in rainfall, the less pronounced the monsoon effect.

        if (janrain > julrain)

            monstrength = monstrength - julrain / janrain;
        else

            monstrength = monstrength - janrain / julrain;

        if (monstrength > 0.0f)
        {
            float cutoff = maxrain * 0.85f; // Any rainfall below this will be reduced.

            float origrain[12];

            for (int n = 0; n < 12; n++)
                origrain[n] = rain[n];

            for (int n = 1; n < 12; n++)
            {
                if (n != 6)
                {
                    int prevmonth = n - 1;

                    if (origrain[n] < cutoff)
                    {
                        if (origrain[prevmonth] < origrain[n]) // If this month's rainfall is greater than last month's, reduce it more.
                            rain[n] = (origrain[n] + minrain * 2.0f) / 3.0f;
                        else // If it's after the main monsoon month, the rain reduces more slowly.
                            rain[n] = (origrain[n] * 2.0f + minrain) / 3.0f;
                    }
                    else
                    {
                        if (origrain[prevmonth] < origrain[n]) // If this month's rainfall is greater than last month's, reduce it more.
                            rain[n] = (origrain[n] * 3.0f + minrain) / 4.0f;
                        else // If it's after the main monsoon month, the rain reduces more slowly.
                            rain[n] = (origrain[n] * 5.0f + minrain) / 6.0f;
                    }
                }
            }
        }
    }

    /*
    float diffthreshold = 3.0f;
    float absmaxraindiff = abs(maxraindiff);

    if (absmaxraindiff < diffthreshold) // Do it again, as if the summer and winter temps are the same.
    {
        float altrain[12];
    }
    */



    for (int n = 0; n < 12; n++)
    {
        if (rain[n] < minrain)
            rain[n] = minrain;
    }
}

// Gets monthly flow for a point.

void monthlyflow(planet& world, float temp[12], float rain[12], int flow[12], float janflow, float julflow) 
{
    float janrain = (float)rain[0];
    float julrain = (float)rain[6];

    float maxraindiff = julrain - janrain;

    float maxflow = julflow;
    float minflow = janflow;

    if (janflow > julflow)
    {
        maxflow = janflow;
        minflow = julflow;
    }

    float maxflowdiff = julflow - janflow;

    float freezefactor = 1.2f; // Multiply the flow by this for every preceding month of freezing temperatures.

    flow[0] = (int)janflow;
    flow[6] = (int)julflow;

    float maxlatdiff = world.sunlat(6) - world.sunlat(0);

    // First, calculate a simple flow variation based on the months. We will later blend this with the more complex calculation.

    float simpleflow[12];
    simpleflow[0] = (float)flow[0];
    simpleflow[6] = (float)flow[6];

    float simplefactor = 1.0f - (maxflowdiff / 200.0f);

    if (simplefactor < 0.0f)
        simplefactor = 0.0f;

    simplefactor = simplefactor * 0.3f;

    if (simplefactor > 0.0f)
    {
        for (int n = 1; n < 12; n++)
        {
            if (n != 6)
            {
                if (world.sunlat(0) != world.sunlat(6)) // Do this by sun latitude.
                {
                    float thislatdiff = world.sunlat(6) - world.sunlat(n);

                    float factor = thislatdiff / maxlatdiff;
                    float flowtoremove = maxflowdiff * factor;

                    simpleflow[n] = julflow - flowtoremove;

                }
                else // Just do it by time of year.
                {
                    float janstrength = 1.0f;
                    float julstrength = 1.0f;

                    if (n == 1 || n == 11)
                    {
                        janstrength = 5.0f;
                        julstrength = 1.0f;
                    }

                    if (n == 2 || n == 10)
                    {
                        janstrength = 4.0f;
                        julstrength = 2.0f;
                    }

                    if (n == 3 || n == 9)
                    {
                        janstrength = 3.0f;
                        julstrength = 3.0f;
                    }

                    if (n == 4 || n == 8)
                    {
                        janstrength = 2.0f;
                        julstrength = 4.0f;
                    }

                    if (n == 5 || n == 7)
                    {
                        janstrength = 1.0f;
                        julstrength = 5.0f;
                    }

                    flow[n] = (int)((janflow * janstrength + julflow * julstrength) / 6.0f);
                }
            }
        }
    }

    for (int n = 1; n < 12; n++)
    {
        if (n != 6)
        {
            if (temp[n] > 0.0f)
            {
                if (maxflowdiff != 0.0f) // Do this by rainfall if possible.
                {
                    float thisraindiff = julrain - rain[n];

                    float factor = thisraindiff / maxraindiff;
                    float flowtoremove = maxflowdiff * factor;

                    flow[n] = (int)(julflow - flowtoremove);

                    if (simplefactor > 0.0f)
                        flow[n] = (int)((float)flow[n] * (1.0f - simplefactor) + simpleflow[n] * simplefactor);
                }
                else
                    flow[n] = (int)simpleflow[n];
            }
            else
                flow[n] = (int)minflow;
        }
    }

    for (int n = 0; n < 12; n++) // For each non-freezing month after a freezing month, increase the flow to simulate floods during the thaw
    {
        if (temp[n] > 0.0f)
        {
            for (int m = n - 1; m > n - 5; m--)
            {
                int mm = m;

                if (mm < 0)
                    mm = mm + 12;

                if (temp[mm] > 0.0f)
                    m = n - 5;
                else
                    flow[n] = (int)((float)flow[n] * freezefactor);

            }
        }
    }

    for (int n = 0; n < 12; n++)
    {
        if (flow[n] < (int)minflow)
            flow[n] = (int)minflow;

        if (flow[n] > (int)maxflow)
            flow[n] = (int)maxflow;
    }
}

// This function expands a vector, creating fractal detail as it goes.

void expand(vector<int>& from, vector<int>& dest, int fromedge, int destedge, int min, int max, float valuemod)
{
    int div = destedge / fromedge;

    int grain = destedge / div;

    // First, we need to make one long vector for faces 0-3, all in one go.

    int longwidth = destedge * 4;
    int longheight = destedge;

    vector<int> longdest(longwidth * longheight, 0);

    // Initialise it.

    for (int face = 0; face < 4; face++)
    {
        int vface = face * fromedge * fromedge;

        int vvface = face * destedge;

        for (int i = 0; i < fromedge; i++)
        {
            int vi = vface + i * fromedge;

            int ii = vvface + i * div;

            int vii = ii * destedge;

            for (int j = 0; j < fromedge; j++)
            {
                int jj = j * div;

                int val = from[vi + j];

                longdest[vii + jj] = val;
            }
        }
    }

    for (int n = 0; n < fromedge; n++) // Bottom edge
    {
        longdest[n * div * destedge + (destedge - 1)] = from[5 * fromedge * fromedge + n * fromedge]; // Face 0
        longdest[(destedge + n * div) * destedge + (destedge - 1)] = from[5 * fromedge * fromedge + (fromedge - 1) * fromedge + n]; // Face 1
        longdest[(2 * destedge) + n * div * destedge + (destedge - 1)] = from[5 * fromedge * fromedge + ((fromedge - 1) - n) * fromedge + (fromedge - 1)]; // Face 2
        longdest[(3 * destedge + n * div) * destedge + (destedge - 1)] = from[5 * fromedge * fromedge + ((fromedge - 1) - n)]; // Face 3
    }

    newfractal(longdest, destedge, longwidth - 1, longheight - 1, grain, valuemod, valuemod, min, max, 0);

    // Apply that to faces 0-3.

    int f1 = destedge * destedge;
    int f2 = destedge * destedge * 2;
    int f3 = destedge * destedge * 3;
    int f4 = destedge * destedge * 4;
    int f5 = destedge * destedge * 5;

    for (int i = 0; i < destedge; i++)
    {
        int vi = i * destedge;

        for (int j = 0; j < destedge; j++)
        {
            int vj = vi + j;

            dest[vj] = longdest[i * destedge + j];
            dest[f1 + vj] = longdest[(i + destedge) * destedge + j];
            dest[f2 + vj] = longdest[(i + destedge * 2) * destedge + j];
            dest[f3 + vj] = longdest[(i + destedge * 3) * destedge + j];
        }
    }

    // Now we need to make a square for face 4.

    vector<int> squaredest((destedge + 1) * (destedge + 1), 0);

    for (int i = 0; i < fromedge; i++)
    {
        int vi = 4 * fromedge * fromedge + i * fromedge;

        int ii = i * div;

        int vii = ii * destedge;

        for (int j = 0; j < fromedge; j++)
        {
            int jj = j * div;

            int val = from[vi + j];

            squaredest[vii + jj] = val;
        }
    }

    // Copy information from the edges of the adjacent faces, so it matches up.

    for (int i = 0; i < destedge; i++)
    {
        int vi = i * destedge;

        squaredest[vi + destedge - 1] = dest[vi];
        squaredest[(destedge - 1) * destedge + i] = dest[f1 + (destedge - 1 - i) * destedge];
        squaredest[vi] = dest[f2 + (destedge - 1 - i) * destedge];
        squaredest[i] = dest[f3 + vi];
    }

    newlidfractal(squaredest, dest, destedge, destedge - 1, destedge - 1, grain, valuemod, valuemod, min, max, 0, 1);

    // Apply that to face 4.

    for (int i = 0; i < destedge; i++)
    {
        int vi = i * destedge;

        for (int j = 0; j < destedge; j++)
        {
            int index = vi + j;
            dest[f4 + index] = squaredest[index];
        }
    }

    // Now do all that again for face 5.

    for (int i = 0; i <= destedge; i++)
    {
        int vi = i * destedge;

        for (int j = 0; j <= destedge; j++)
            squaredest[vi + j] = 0;
    }

    for (int i = 0; i < fromedge; i++)
    {
        int vi = 5 * fromedge * fromedge + i * fromedge;

        int ii = i * div;

        int vii = ii * destedge;

        for (int j = 0; j < fromedge; j++)
        {
            int jj = j * div;

            int val = from[vi + j];

            squaredest[vii + jj] = val;
        }
    }

    // Copy information from the edges of the adjacent faces, so it matches up.

    for (int i = 0; i < destedge; i++)
    {
        int vi = i * destedge;

        squaredest[vi] = dest[vi + destedge - 1];
        squaredest[(destedge - 1) * destedge + i] = dest[f1 + vi + destedge - 1];
        squaredest[vi + destedge - 1] = dest[f2 + (destedge - 1 - i) * destedge + (destedge - 1)];
        squaredest[i] = dest[f3 + (destedge - 1 - i) * destedge + (destedge - 1)];
    }

    newlidfractal(squaredest, dest, destedge, destedge - 1, destedge - 1, grain, valuemod, valuemod, min, max, 0, 1);

    // Apply that to face 5.

    for (int i = 0; i < destedge; i++)
    {
        int vi = i * destedge;

        for (int j = 0; j < destedge; j++)
        {
            int index = vi + j;
            dest[f5 + index] = squaredest[index];
        }
    }

    // Finish the edges of face 5.

    int margin = div + 2;

    for (int j = 0; j < destedge; j++) // First the border with face 1.
    {
        float fromval = (float)dest[f5 + (destedge - 2 - margin) * destedge + j];
        float toval = (float)dest[f1 + j * destedge + destedge - 1];

        float increment = (toval - fromval) / (float)margin;

        float thisval = fromval;

        for (int n = 0; n < margin; n++)
        {
            thisval = thisval + increment;

            dest[f5 + (destedge - margin + n) * destedge + j] = (int)thisval;
        }
    }

    for (int i = 0; i < destedge; i++) // Now the border with face 2.
    {
        float fromval = (float)dest[f5 + i * destedge + destedge - 2 - margin];
        float toval = (float)dest[f2 + (destedge - 1 - i) * destedge + destedge - 1];

        float increment = (toval - fromval) / (float)margin;

        float thisval = fromval;

        for (int n = 0; n < margin; n++)
        {
            thisval = thisval + increment;

            dest[f5 + i * destedge + destedge - margin + n] = (int)thisval;
        }
    }
}

float getrelativelong(float globerotate, float camlong)
{
    float val = globerotate + camlong;

    while (val >= 360.0f)
        val = val - 360.0f;

    while (val < 0.0f)
        val = val + 360.0f;

    return val;
}

// This specifies faces of the globe not to be drawn if they wouldn't be visible.

void drawfacecheck(bool drawface[6], float relativelong, float relativelat)
{
    for (int n = 0; n < 6; n++)
        drawface[n] = 1;

    if (relativelat > 40.0f)
        drawface[5] = 0;
    else
    {
        if (relativelat < -40.0f)
            drawface[4] = 0;
        else
        {
            if (relativelong > 45.0f && relativelong <= 135.0f)
                drawface[0] = 0;

            if (relativelong > 135.0f && relativelong <= 225.0f)
                drawface[3] = 0;

            if (relativelong > 225.0f && relativelong <= 315.0f)
                drawface[2] = 0;

            if (relativelong > 315.0f || relativelong <= 45.0f)
                drawface[1] = 0;
        }
    }
}

// This finds the nearest created/uncreated region to a specified region. (if created == 1 we're looking for a created one, otherwise an uncreated one)

globepoint findregion(planet& world, int panels, vector<short>& regioncreated, vector<short>& regioncreatedloc, vector<globepoint>& regionID, vector<int>& regionloc, globepoint targetregion, bool created)
{
    int targetID= regionloc[targetregion.face * panels * panels + targetregion.x * panels + targetregion.y];

    if (created && regioncreated[targetID] == 1)
        return targetregion;

    if (created == 0 && regioncreated[targetID] == 0)
        return targetregion;

    int edge = panels;

    if (world.size() == 1)
        edge = panels / 2;

    if (world.size() == 0)
        edge = panels / 4;

    int maxradius = edge * 2;

    // First, do a progressive sweep around the target.

    for (int radius = 1; radius <= maxradius; radius++)
    {
        int rradius = radius * radius;

        for (int i = -radius; i <= radius; i++)
        {
            for (int j = -radius; j <= radius; j++)
            {
                if (i * i + j * j <= rradius)
                {
                    globepoint thispoint = getglobepoint(edge, targetregion.face, targetregion.x, targetregion.y, i, j);

                    if (thispoint.face != -1)
                    {
                        if (created)
                        {
                            if (regioncreatedloc[thispoint.face * panels * panels + thispoint.x * panels + thispoint.y] == 1)
                                return thispoint;
                        }
                        else
                        {
                            if (regioncreatedloc[thispoint.face * panels * panels + thispoint.x * panels + thispoint.y] == 0)
                                return thispoint;
                        }
                    }
                }
            }
        }
    }

    // If that didn't work, do a regressive sweep around the region opposite the target.

    int oldface = targetregion.face;

    switch (oldface)
    {
    case 0:
        targetregion.face = 2;
        break;

    case 1:
        targetregion.face = 3;
        break;

    case 2:
        targetregion.face = 0;
        break;

    case 3:
        targetregion.face = 1;
        break;

    case 4:
        targetregion.face = 5;
        break;

    case 5:
        targetregion.face = 4;
        break;
    }

    targetregion.x = edge - 1 - targetregion.x;
    targetregion.y = edge - 1 - targetregion.y;

    for (int radius = maxradius; radius >= 1; radius--)
    {
        int rradius = radius - 2;

        if (rradius < 0)
            rradius = 0;

        rradius = rradius * rradius;

        for (int i = -radius; i <= radius; i++)
        {
            for (int j = -radius; j <= radius; j++)
            {
                if (i * i + j * j > rradius)
                {
                    globepoint thispoint = getglobepoint(edge, targetregion.face, targetregion.x, targetregion.y, i, j);

                    if (thispoint.face != -1)
                    {
                        if (created)
                        {
                            if (regioncreatedloc[thispoint.face * 16 * 16 + thispoint.x * 16 + thispoint.y] == 1)
                                return thispoint;
                        }
                        else
                        {
                            if (regioncreatedloc[thispoint.face * 16 * 16 + thispoint.x * 16 + thispoint.y] == 0)
                                return thispoint;
                        }
                    }
                }
            }
        }
    }

    targetregion.face = -1;
    targetregion.x = -1;
    targetregion.y = -1;

    return targetregion;
}




