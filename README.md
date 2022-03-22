# Undiscovered_Worlds
Procedural map creator

Undiscovered Worlds is a simple world creation and viewing tool. It can create maps at the global and regional scale.

For more information, and to complain about bugs, please visit the blog: https://undiscoveredworlds.blogspot.com/2019/01/what-is-undiscovered-worlds.html

----

* The code

Please note that this code requires the following libraries to work:
NanoGUI - https://github.com/mitsuba-renderer/nanogui
SFML - https://www.sfml-dev.org/
stb_image - https://github.com/nothings/stb

The code is offered under the GNU General Public Licence - https://choosealicense.com/licenses/gpl-3.0/. Feel free to make whatever use of it you like, but please credit me if you repurpose any of it!

----

* How to use Undiscovered Worlds

When you start the program, you are prompted to enter a seed number for the new world. You can choose a random number or enter your own, before clicking "OK" to begin the generation process.

Alternatively, you can load a previously created world, or import your own maps (see below).

When the world is ready, you will see the global map screen.

* The global map screen

This screen displays a map of your world, shown at a scale of approximately 1 pixel to 16km. You can use the mouse to move the map around or zoom in and out. The buttons to the left perform several functions:

"World controls" - these buttons let you create a new world, load in a different one, save the current one, or import your own maps (see below).

"Export options" - these allow you to export images showing maps of the world, or of a user-defined area (see below).

"Display map type" - these allow you to display different kinds of information on the map.

The four icons in the bottom left have the following functions:

Ruler (top left) - recentre and resize the map.
Palette (bottom left) - open the settings to change the map appearance (see below).
Pin (top right) - select a point on the map. Information about this point will appear below the map.
Zoom (bottom right) - open up the regional map screen for the selected point.

* The regional map screen

This screen displays a map of a small area of your world, shown at a scale of approximately 1 pixel to 1 km. As with the global map screen, you can use the mouse to move the map around or zoom in and out. The buttons to the left perform several functions:

"World controls" - the button here lets you return to the global map screen.

"Export options" - these allow you to export images showing maps of the currently viewed region, or of a user-defined area (see below).

"Display map type" - these allow you to display different kinds of information on the map.

The three icons in the bottom left have the same functions as on the global map (note that there is no zoom button on the regional map screen).

To the top right of the screen is a minimap showing the current region on a world map. You can scroll this map around and zoom in. Use the ruler icon underneath it to reset the size and position. You can also use the pin icon under it to select a new region.

Under the minimap are four arrow icons, which you can also use to move to a new region adjacent to the current one.

* The map appearance settings

This window allows you to change the appearance of the relief maps. Note that any changes here will be applied to both global and regional maps. These changes are purely aesthetic - nothing about the world itself is changed here, and none of these changes affects the other maps such as elevation, temperature, etc.

Also, note that if you type new values into the boxes that allow it, you must press Enter for them to take effect - don't just click out of the box.

You can click on the colour boxes to bring up a colour picker. The program mixes these colours to create the relief maps - try changing them to see what sort of effect it has. You can also try making some colours identical to produce simpler maps - e.g. if you want the sea to be a single colour throughout, set "shallow ocean" and "deep ocean" to the same colour and turn off both shading and marbling on sea.

Underneath the colour boxes are several sliders:

"Shading" - this controls the pseudo-3D shading effect. The sliders allow you to change its intensity on land, on lakes, and on sea. The "light source" box to the right allows you to change the apparent direction of the lighting.

"Marbling" - this controls the marbling effect, which adds variety to the appearance of the maps. You can, again, change its intensity on land, on lakes, and on sea. The "snow transition" box to the right allows you to change the way the map displays the transition between snowy and non-snowy regions on the map.

"Rivers" - this controls how many rivers are shown on the map. Only rivers with flow greater than the given number are shown, so the lower the number, the more rivers you will see. You can set different values for the global and regional maps. The "sea ice" button to the right allows you to set whether sea ice is shown.

The buttons on the left of the map appearance settings window allow you to close the window, load or save settings, and restore the defaults. Note that if you save the world from the global map screen, its appearance settings are saved with it and will be restored if you reload it. So you don't need to save the settings separately unless you plan to load them into other worlds.

* The custom area export screen

This window allows you to export maps from a custom-defined area of the world. These maps are at the same scale as the regional map - 1 pixel to 1km - but they can be of larger areas.

Click the "select point" button to the left, and then click a point on the map. Do this again to select a second point, defining a rectangle. You can continue to use the "select point" button to choose new points, to re-define the area. When you have the area you want, click on "export maps".

* The import maps screen

Please note that this feature is experimental! You may need trial and error to get good results.

This screen allows you to import your own maps - created with an image editor - and turn them into Undiscovered Worlds worlds. In this way, you can create your own terrain, and have Undiscovered Worlds calculate the climates, rivers, lakes, etc. You can then explore maps of your world just like any other.

The buttons to the left are in two main groups:

"Import" - these buttons are for importing your own maps. They must be 2048x1025 pixels, in .png format. The program will interpret them in the following way:

land map - only the red value is used. 0 indicates sea, and any higher value is elevation above sea level, in increments of 10.
sea map - only the red value is used. 0 indicates land, and any higher value is depth below sea level, in increnements of 50.
mountains map - only the red value is used. It shows the peak elevation above the surrounding land, in increments of 50.
volanoes map - the red value shows the peak elevation above the surrounding land, in increments of 50. A blue value of 0 indicates a shield volcano, or a higher value indicates a stratovolcano. A green value of 0 indicates an extinct volcano, or a higher value indicates an active volcano.

In theory you only need a land map - the others are optional. It's important to note that the land map shouldn't show mountain ranges. Undiscovered Worlds does not treat mountain ranges as normal elevation. If you want to define your own mountain ranges, you must import a mountains map, on which you have drawn the lines of the main mountain ranges as indicated above.

Also, the land map doesn't have to be very detailed. If you want, you could simply use the values of 0 to show sea and 1 to show land, without bothering about specifying elevation beyond that. You can use the "land elevation" button in the "generate" section to add random elevation to your map.

"Generate" - once you have imported your own maps, you can use these buttons to add features to your world.

When you have finished, click the large icon in the bottom left-hand corner. This finalises the terrain and then calculates rainfall, temperature, rivers, lakes, etc. When it is finished, the custom world will be displayed in the global map screen as usual, and you can view or save it like any other.
