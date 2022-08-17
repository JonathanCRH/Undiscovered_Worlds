# Undiscovered_Worlds
Procedural map creator

Undiscovered Worlds is a simple world creation and viewing tool. It can create maps at the global and regional scale.

For more information, and to complain about bugs, please visit the blog: https://undiscoveredworlds.blogspot.com/2019/01/what-is-undiscovered-worlds.html

----

* Credits

Undiscovered Worlds is written by Jonathan Hill, with additional contributions and corrections by Frank Gennari. Sections of code that are taken or adapted from other sources are noted in the comments.

----

* The code

Please note that this code requires the following libraries to work:

SFML - https://www.sfml-dev.org/

Dear ImGui - https://github.com/ocornut/imgui

ImGui-SFML - https://github.com/eliasdaler/imgui-sfml

ImGuiFileDialog - https://github.com/aiekick/ImGuiFileDialog


The code is offered under the GNU General Public Licence - https://choosealicense.com/licenses/gpl-3.0/. Feel free to make whatever use of it you like, but please credit me if you repurpose any of it!

----

* How to use Undiscovered Worlds

When you start the program, you are prompted to enter a seed number for the new world. You can choose a random number or enter your own, before clicking "OK" to begin the generation process.

Alternatively, you can load a previously created world, or import your own maps (see below).

When the world is ready, you will see the global map screen.

* The global map screen

This screen displays a map of your world. You can click on any point to see information about it. The buttons to the left perform several functions:

"World controls" - these buttons let you create a new world, load in a different one, save the current one, or import your own maps (see below).

"Export options" - these allow you to export images showing maps of the world, or of a user-defined area (see below).

"Display map type" - these allow you to display different kinds of information on the map.

"Appearance" - open the settings to change the map appearance (see below).

"Zoom" - open up the regional map screen for the selected point.

* The regional map screen

This screen displays a map of a small area of your world, shown at a scale of approximately 1 pixel to 1 km. As with the global map screen, you can click on the map to get information about that point. The buttons to the left perform several functions:

"World controls" - the button here lets you return to the global map screen.

"Export options" - these allow you to export images showing maps of the currently viewed region, or of a user-defined area (see below).

"Display map type" - these allow you to display different kinds of information on the map.

"Appearance" - open the settings to change the map appearance (see below).

To the top right of the screen is a minimap showing the current region on a world map. You can click on this map to go directly to another region. You can also use the cursor keys to move to neighbouring regions.

* The map appearance settings

This window allows you to change the appearance of the relief maps. Note that any changes here will be applied to both global and regional maps. These changes are purely aesthetic - nothing about the world itself is changed here, and none of these changes affects the other maps such as elevation, temperature, etc.

You can click on the colour boxes to bring up a colour picker. The program mixes these colours to create the relief maps - try changing them to see what sort of effect it has. You can also try making some colours identical to produce simpler maps - e.g. if you want the sea to be a single colour throughout, set "shallow ocean" and "deep ocean" to the same colour and turn off both shading and marbling on sea. Note that you can type new values directly into the boxes by clicking on them while holding down the control key.

Underneath the colour boxes are several sliders:

"Shading" - this controls the pseudo-3D shading effect. The sliders allow you to change its intensity on land, on lakes, and on sea.

"Marbling" - this controls the marbling effect, which adds variety to the appearance of the maps. You can, again, change its intensity on land, on lakes, and on sea.

"Rivers" - this controls how many rivers are shown on the map. Only rivers with flow greater than the given number are shown, so the lower the number, the more rivers you will see. You can set different values for the global and regional maps.

There are also some other controls to the right. The "light" box allows you to change the apparent direction of the lighting. The "snow" box under it allows you to change the way the map displays the transition between snowy and non-snowy regions on the map. Finally, the "sea ice" button allows you to set whether sea ice is shown.

The buttons at the bottom right allow you to save or load settings, restore the defaults, and close the panel. Note that if you save the world from the global map screen, its appearance settings are saved with it and will be restored if you reload it. So you don't need to save the settings separately unless you plan to load them into other worlds.

* The custom area export screen

This window allows you to export maps from a custom-defined area of the world. These maps are at the same scale as the regional map - 1 pixel to 1km - but they can be of larger areas.

Click a point on the map to select a point. Do this again to select a second point, defining a rectangle. You can continue to click or drag the points to re-define the area. When you have the area you want, click on "export maps". There are also buttons to clear your selected area and to return to the global map screen.

* The import maps screen

Please note that this feature is experimental! You may need trial and error to get good results.

This screen allows you to import your own maps - created with an image editor - and turn them into Undiscovered Worlds worlds. In this way, you can create your own terrain, and have Undiscovered Worlds calculate the climates, rivers, lakes, etc. You can then explore maps of your world just like any other. The zip file "Example import files" contains some maps that you can import to recreate Tolkien's Arda, which should give you a good idea of the format they need to be in to create your own.

The buttons to the left are in two main groups:

"Import" - these buttons are for importing your own maps. They must be 2048x1025 pixels, in .png format. The program will interpret them in the following way:

land map - only the red value is used. 0 indicates sea, and any higher value is elevation above sea level, in increments of 10.

sea map - only the red value is used. 0 indicates land, and any higher value is depth below sea level, in increnements of 50.

mountains map - only the red value is used. It shows the peak elevation above the surrounding land, in increments of 50.

volanoes map - the red value shows the peak elevation above the surrounding land, in increments of 50. A blue value of 0 indicates a shield volcano, or a higher value indicates a stratovolcano. A green value of 0 indicates an extinct volcano, or a higher value indicates an active volcano.

In theory you only need a land map - the others are optional. It's important to note that the land map shouldn't show mountain ranges. Undiscovered Worlds does not treat mountain ranges as normal elevation. If you want to define your own mountain ranges, you must import a mountains map, on which you have drawn the lines of the main mountain ranges as indicated above.

Also, the land map doesn't have to be very detailed. If you want, you could simply use the values of 0 to show sea and 1 to show land, without bothering about specifying elevation beyond that. You can use the "land elevation" button in the "generate" section to add random elevation to your map.

"Generate" - once you have imported your own maps, you can use these buttons to add features to your world.

When you have finished, click the "done" button. This finalises the terrain and then calculates rainfall, temperature, rivers, lakes, etc. When it is finished, the custom world will be displayed in the global map screen as usual, and you can view or save it like any other.

----

* Known issues

The ability to resize the window is turned off, because it messes up the mouse tracking. (You can minimise it, of course.)

Saving and loading worlds is slow, but it works.

It occasionally crashes when exporting area maps. The cause is as yet unknown. Be sure to save worlds/settings before using this.

Occasionally, exporting maps doesn't work. Save the world, restart UW, load the world back in, and try again. I don't know why this happens or why restarting UW usually solves the issue.

Continents occasionally appear with straight sides. The cause of this is unknown too, but it is rare.

Gridlike artefacts sometimes appear on the ocean floor of custom worlds created from imported maps. I'm looking into this.

The climate simulation is imperfect. Climate regions are more jumbled together than they should be. There is too much monsoon (Am) and not enough savannah (Aw/As). There is less warm-summer humid continental (Dfb) than there should be. However, a perfect climate simulation would require a lot more processing power and time than I have available!

Lakes occasionally go haywire. Known cause: lakes are the work of the devil - https://undiscoveredworlds.blogspot.com/2019/02/grappling-with-lakes.html
