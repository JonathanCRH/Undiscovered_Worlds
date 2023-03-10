
#include "imgui.h"
#include "imgui-SFML.h"
#include "ImGuiFileDialog.h"

#pragma comment(lib, "urlmon.lib")

#include <urlmon.h>
#include <cstdio>

#include <windows.h>
#include <shellapi.h>

#include <iomanip>
#include <sstream>
#include <iostream>
#include <cmath>
#include <fstream>
#include <stdio.h>
#include <string>
#include <chrono>
#include <thread>
#include <queue>
#include <stdint.h>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include "classes.hpp"
#include "planet.hpp"
#include "region.hpp"
#include "functions.hpp"

#define getURL URLOpenBlockingStreamA

// nodiscard attribute
#pragma warning (disable: 4834)

#define REGIONALCREATIONSTEPS 84

#define GLOBALTERRAINCREATIONSTEPS1 26
#define GLOBALTERRAINCREATIONSTEPS2 31
#define GLOBALCLIMATECREATIONSTEPS 28

#define REGIONALTILEWIDTH 32
#define REGIONALTILEHEIGHT 32

#define DISPLAYMAPSIZEX 1024
#define DISPLAYMAPSIZEY 512

using namespace std;

// Random number generator. From https://stackoverflow.com/questions/26237419/faster-than-rand
// It uses a global variable. But this is the only one in the program, honest!

static long g_seed = 1;

// Used to seed the generator.
void fast_srand(long seed)
{
    g_seed = seed;
}

// Compute a pseudorandom integer.
// Output value in range [0, 32767]
int fast_rand(void)
{
    g_seed = (214013 * g_seed + 2531011);
    return (g_seed >> 16) & 0x7FFF;
}

int main()
{
    float currentversion = 1.0f;
    float latestversion = getlatestversion();

    string currentversionstring = "Current version: " + to_string(currentversion);
    string latestversionstring = "Latest version: " + to_string(latestversion);

    updatereport(currentversionstring.c_str());
    updatereport(latestversionstring.c_str());

    // Set up the window.

    int scwidth = 1224;
    int scheight = 768;

    sf::RenderWindow window(sf::VideoMode(scwidth, scheight), "Undiscovered Worlds");
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);

    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();

    // Setting up Dear ImGUI style
    // Adapted from Cinder-ImGui by Simon Geilfus - https://github.com/simongeilfus/Cinder-ImGui

    ImGuiStyle& Style = ImGui::GetStyle();

    Style.WindowMinSize = ImVec2(160, 20);
    Style.FramePadding = ImVec2(4, 2);
    Style.ItemSpacing = ImVec2(6, 4);
    Style.ItemInnerSpacing = ImVec2(6, 4);
    Style.Alpha = 0.95f;
    Style.WindowRounding = 4.0f;
    Style.FrameRounding = 2.0f;
    Style.IndentSpacing = 6.0f;
    Style.ItemInnerSpacing = ImVec2(2, 4);
    Style.ColumnsMinSpacing = 50.0f;
    Style.GrabMinSize = 14.0f;
    Style.GrabRounding = 16.0f;
    Style.ScrollbarSize = 12.0f;
    Style.ScrollbarRounding = 16.0f;

    float highlight1 = 0.40f;
    float highlight2 = 0.40f;
    float highlight3 = 0.40f;

    Style.Colors[ImGuiCol_Text] = ImVec4(0.86f, 0.93f, 0.89f, 0.78f);
    Style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.86f, 0.93f, 0.89f, 0.28f);
    Style.Colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.14f, 0.17f, 1.00f);
    Style.Colors[ImGuiCol_ChildBg] = ImVec4(highlight1, highlight2, highlight3, 1.00f); // new
    Style.Colors[ImGuiCol_PopupBg] = ImVec4(highlight1, highlight2, highlight3, 1.00f); // new
    Style.Colors[ImGuiCol_Border] = ImVec4(0.31f, 0.31f, 1.00f, 0.00f);
    Style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    Style.Colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
    Style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
    Style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
    Style.Colors[ImGuiCol_TitleBg] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
    Style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.20f, 0.22f, 0.27f, 0.75f);
    Style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
    Style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.20f, 0.22f, 0.27f, 0.47f);
    Style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
    Style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.09f, 0.15f, 0.16f, 1.00f);
    Style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.29f, 0.18f, 0.92f, 0.78f);
    Style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(highlight1, highlight2, highlight3, 1.00f);
    Style.Colors[ImGuiCol_CheckMark] = ImVec4(0.86f, 0.93f, 0.89f, 0.78f);
    Style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.47f, 0.77f, 0.83f, 0.14f);
    Style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.28f, 0.18f, 0.92f, 1.00f);
    Style.Colors[ImGuiCol_Button] = ImVec4(0.47f, 0.77f, 0.83f, 0.14f);
    Style.Colors[ImGuiCol_ButtonHovered] = ImVec4(highlight1, highlight2, highlight3, 0.86f);
    Style.Colors[ImGuiCol_ButtonActive] = ImVec4(highlight1, highlight2, highlight3, 1.00f);
    Style.Colors[ImGuiCol_Header] = ImVec4(highlight1, highlight2, highlight3, 0.76f);
    Style.Colors[ImGuiCol_HeaderHovered] = ImVec4(highlight1, highlight2, highlight3, 0.86f);
    Style.Colors[ImGuiCol_HeaderActive] = ImVec4(highlight1, highlight2, highlight3, 1.00f);
    Style.Colors[ImGuiCol_Separator] = ImVec4(0.14f, 0.16f, 0.19f, 1.00f);
    Style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(highlight1, highlight2, highlight3, 0.78f);
    Style.Colors[ImGuiCol_SeparatorActive] = ImVec4(highlight1, highlight2, highlight3, 1.00f);
    Style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.47f, 0.77f, 0.83f, 0.04f);
    Style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(highlight1, highlight2, highlight3, 0.78f);
    Style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(highlight1, highlight2, highlight3, 1.00f);
    Style.Colors[ImGuiCol_PlotLines] = ImVec4(0.86f, 0.93f, 0.89f, 0.63f);
    Style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(highlight1, highlight2, highlight3, 1.00f);
    Style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.86f, 0.93f, 0.89f, 0.63f);
    Style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(highlight1, highlight2, highlight3, 1.00f);
    Style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(highlight1, highlight2, highlight3, 0.43f);
    Style.Colors[ImGuiCol_PopupBg] = ImVec4(0.20f, 0.22f, 0.27f, 0.9f);
    //Style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.22f, 0.27f, 0.73f);

    Style.Colors[ImGuiCol_Tab] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
    Style.Colors[ImGuiCol_TabHovered] = ImVec4(highlight1, highlight2, highlight3, 0.86f);
    Style.Colors[ImGuiCol_TabActive] = ImVec4(highlight1, highlight2, highlight3, 0.86f);
    Style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
    Style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(highlight1, highlight2, highlight3, 0.86f);

    ImGuiIO& io = ImGui::GetIO();
    ImFont* font1 = io.Fonts->AddFontFromFileTTF("fonts\\Roboto-Medium.ttf", 18.0f);
    io.Fonts->Build();
    IM_ASSERT(font != NULL);
    ImGui::SFML::UpdateFontTexture();

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoResize;

    string degree = "\xC2\xB0"; // 0xC2 0xB0 (c2b0) °
    string cube = "\xC2\xB3"; // 0xC2 0xB3 (c2b3) ³

    // Now create the main world object and initialise its basic variables.

    planet* world = new planet;

    initialiseworld(*world);
    initialisemapcolours(*world);

    int width = world->width();
    int height = world->height();

    // Now do the same thing for the region object.

    region* region = new class region;

    initialiseregion(*world, *region);

    int regionalmapimagewidth = region->regwidthend() - region->regwidthbegin() + 1;
    int regionalmapimageheight = region->regheightend() - region->regheightbegin() + 1;

    // Now we must create the images for the different global maps.

    sf::Vector2i globaltexturesize;

    globaltexturesize.x = world->width() + 1; // 2048;
    globaltexturesize.y = world->height() + 2; // 1026;

    sf::Image* globalelevationimage = new sf::Image;
    sf::Image* globaltemperatureimage = new sf::Image;
    sf::Image* globalprecipitationimage = new sf::Image;
    sf::Image* globalclimateimage = new sf::Image;
    sf::Image* globalriversimage = new sf::Image;
    sf::Image* globalreliefimage = new sf::Image;

    globalelevationimage->create(globaltexturesize.x, globaltexturesize.y);
    globaltemperatureimage->create(globaltexturesize.x, globaltexturesize.y);
    globalprecipitationimage->create(globaltexturesize.x, globaltexturesize.y);
    globalclimateimage->create(globaltexturesize.x, globaltexturesize.y);
    globalriversimage->create(globaltexturesize.x, globaltexturesize.y);
    globalreliefimage->create(globaltexturesize.x, globaltexturesize.y);

    for (int i = 0; i < globaltexturesize.x; i++)
    {
        for (int j = 0; j < globaltexturesize.y; j++)
        {
            globalelevationimage->setPixel(i, j, sf::Color::Black);
            globaltemperatureimage->setPixel(i, j, sf::Color::Black);
            globalprecipitationimage->setPixel(i, j, sf::Color::Black);
            globalclimateimage->setPixel(i, j, sf::Color::Black);
            globalriversimage->setPixel(i, j, sf::Color::Black);
            globalreliefimage->setPixel(i, j, sf::Color::Black);
        }
    }

    // Create smaller ones that will actually be displayed on the global map screen. (This is because the full-size ones don't look good scaled down.)

    sf::Vector2i displayglobaltexturesize;

    displayglobaltexturesize.x = DISPLAYMAPSIZEX; // 1024
    displayglobaltexturesize.y = DISPLAYMAPSIZEY; // 512

    sf::Image* displayglobalelevationimage = new sf::Image;
    sf::Image* displayglobaltemperatureimage = new sf::Image;
    sf::Image* displayglobalprecipitationimage = new sf::Image;
    sf::Image* displayglobalclimateimage = new sf::Image;
    sf::Image* displayglobalriversimage = new sf::Image;
    sf::Image* displayglobalreliefimage = new sf::Image;

    displayglobalelevationimage->create(displayglobaltexturesize.x, displayglobaltexturesize.y);
    displayglobaltemperatureimage->create(displayglobaltexturesize.x, displayglobaltexturesize.y);
    displayglobalprecipitationimage->create(displayglobaltexturesize.x, displayglobaltexturesize.y);
    displayglobalclimateimage->create(displayglobaltexturesize.x, displayglobaltexturesize.y);
    displayglobalriversimage->create(displayglobaltexturesize.x, displayglobaltexturesize.y);
    displayglobalreliefimage->create(displayglobaltexturesize.x, displayglobaltexturesize.y);

    for (int i = 0; i < displayglobaltexturesize.x; i++)
    {
        for (int j = 0; j < displayglobaltexturesize.y; j++)
        {
            displayglobalelevationimage->setPixel(i, j, sf::Color::Black);
            displayglobaltemperatureimage->setPixel(i, j, sf::Color::Black);
            displayglobalprecipitationimage->setPixel(i, j, sf::Color::Black);
            displayglobalclimateimage->setPixel(i, j, sf::Color::Black);
            displayglobalriversimage->setPixel(i, j, sf::Color::Black);
            displayglobalreliefimage->setPixel(i, j, sf::Color::Black);
        }
    }

    // Create the texture that will apply these images to the global map.

    sf::Texture* globalmaptexture = new sf::Texture;
    globalmaptexture->loadFromImage(*displayglobalreliefimage);

    // Create the sprite that will display that texture.

    sf::Sprite* globalmap = new sf::Sprite;

    float globalmapxpos = 180.f;
    float globalmapypos = 20.f;

    globalmap->setTexture(*globalmaptexture);
    globalmap->setPosition(sf::Vector2f(globalmapxpos, globalmapypos));

    // And also the minimap, which will use the same texture.

    sf::Sprite* minimap = new sf::Sprite;

    float minimapxpos = 190.f + ARRAYWIDTH / 4;
    float minimapypos = 20.f;

    minimap->setTexture(*globalmaptexture);
    minimap->setScale(0.5, 0.5);
    minimap->setPosition(sf::Vector2f(minimapxpos, minimapypos));

    // Create an image, texture, and sprite for the highlight point and the minimap highlight

    int highlightsize = 3;

    sf::Image* highlightimage = new sf::Image;
    highlightimage->create(highlightsize, highlightsize);

    int minihighlightsize = 8;

    sf::Image* minihighlightimage = new sf::Image;
    minihighlightimage->create(minihighlightsize, minihighlightsize);

    drawhighlightobjects(*world, *highlightimage, highlightsize, *minihighlightimage, minihighlightsize);

    sf::Texture* highlighttexture = new sf::Texture;
    highlighttexture->loadFromImage(*highlightimage);

    sf::Sprite* highlight = new sf::Sprite;
    highlight->setTexture(*highlighttexture);
    highlight->setOrigin((float)(highlightsize / 2), (float)(highlightsize / 2));

    sf::Texture* minihighlighttexture = new sf::Texture;
    minihighlighttexture->loadFromImage(*minihighlightimage);

    sf::Sprite* minihighlight = new sf::Sprite;
    minihighlight->setTexture(*minihighlighttexture);
    minihighlight->setOrigin((float)(minihighlightsize / 2), (float)(minihighlightsize / 2));

    // Now we must create the images for the different regional maps. Same thing again.

    sf::Vector2i regionaltexturesize;

    regionaltexturesize.x = 514;
    regionaltexturesize.y = 514;

    int regionalimagewidth = regionaltexturesize.x;
    int regionalimageheight = regionaltexturesize.y;
    int regionalimagechannels = 4;

    int regionalimagesize = regionalimagewidth * regionalimageheight * regionalimagechannels;

    sf::Image* regionalelevationimage = new sf::Image;
    sf::Image* regionaltemperatureimage = new sf::Image;
    sf::Image* regionalprecipitationimage = new sf::Image;
    sf::Image* regionalclimateimage = new sf::Image;
    sf::Image* regionalriversimage = new sf::Image;
    sf::Image* regionalreliefimage = new sf::Image;

    regionalelevationimage->create(regionalimagewidth, regionalimageheight);
    regionaltemperatureimage->create(regionalimagewidth, regionalimageheight);
    regionalprecipitationimage->create(regionalimagewidth, regionalimageheight);
    regionalclimateimage->create(regionalimagewidth, regionalimageheight);
    regionalriversimage->create(regionalimagewidth, regionalimageheight);
    regionalreliefimage->create(regionalimagewidth, regionalimageheight);

    for (int i = 0; i < regionalimagewidth; i++)
    {
        for (int j = 0; j < regionalimageheight; j++)
        {
            regionalelevationimage->setPixel(i, j, sf::Color::Black);
            regionaltemperatureimage->setPixel(i, j, sf::Color::Black);
            regionalprecipitationimage->setPixel(i, j, sf::Color::Black);
            regionalclimateimage->setPixel(i, j, sf::Color::Black);
            regionalriversimage->setPixel(i, j, sf::Color::Black);
            regionalreliefimage->setPixel(i, j, sf::Color::Black);
        }
    }

    // Create the texture that will apply these images to the regional map.

    sf::Texture* regionalmaptexture = new sf::Texture;
    regionalmaptexture->loadFromImage(*regionalreliefimage);

    // Create the sprite that will display that texture.

    sf::Sprite* regionalmap = new sf::Sprite;

    float regionalmapxpos = 180.f;
    float regionalmapypos = 20.f;

    regionalmap->setTexture(*regionalmaptexture);
    regionalmap->setPosition(sf::Vector2f(regionalmapxpos, regionalmapypos));

    // A rectangle for the area select screen

    sf::RectangleShape* arearectangle = new sf::RectangleShape;
    arearectangle->setOutlineThickness(2);
    arearectangle->setFillColor(sf::Color::Transparent);

    // Now we need to load the template images for various kinds of terrain creation.

    boolshapetemplate landshape[12]; // Basic land shape
    createlandshapetemplates(landshape);

    boolshapetemplate chainland[2]; // Mountain chain land shapes.
    createchainlandtemplates(chainland);

    boolshapetemplate smalllake[12]; // Small lake shapes
    createsmalllaketemplates(smalllake);

    boolshapetemplate largelake[10]; // Large lake shapes
    createlargelaketemplates(largelake);

    boolshapetemplate island[12]; // Small island shapes
    createislandtemplates(island);

    byteshapetemplate smudge[6]; // Smudge shapes
    createsmudgetemplates(smudge);

    byteshapetemplate smallsmudge[6]; // Small smudge shapes
    createsmallsmudgetemplates(smallsmudge);

    peaktemplate* peaks = new peaktemplate; // Peak shapes.

    loadpeaktemplates(*peaks);

    // Template for sections of rift lakes (I don't think these are used any more, but still)

    int riftblobsize = 40;

    vector<vector<float>> riftblob(riftblobsize + 2, vector<float>(riftblobsize + 2, 0.0f));

    createriftblob(riftblob, riftblobsize / 2);

    // Other bits

    fast_srand((long)time(0));

    bool brandnew = 1; // This means the program has just started and it's the first world to be generated.
    bool loadingworld = 0; // This would mean that we're trying to load in a new world.
    bool savingworld = 0; // This would mean that we're trying to save a world.
    bool loadingsettings = 0; // This would mean that we're trying to load appearance settings.
    bool savingsettings = 0; // This would mean that we're trying to save appearance settings.
    bool exportingworldmaps = 0;
    bool exportingregionalmaps = 0;
    bool exportingareamaps = 0;
    bool importinglandmap = 0;
    bool importingseamap = 0;
    bool importingmountainsmap = 0;
    bool importingvolcanoesmap = 0;

    string month[12];

    month[0] = "Jan";
    month[1] = "Feb";
    month[2] = "Mar";
    month[3] = "Apr";
    month[4] = "May";
    month[5] = "Jun";
    month[6] = "Jul";
    month[7] = "Aug";
    month[8] = "Sep";
    month[9] = "Oct";
    month[10] = "Nov";
    month[11] = "Dec";

    vector<int> squareroot((MAXCRATERRADIUS* MAXCRATERRADIUS + MAXCRATERRADIUS + 1) * 24);

    for (int n = 0; n <= ((MAXCRATERRADIUS * MAXCRATERRADIUS + MAXCRATERRADIUS) * 24); n++)
        squareroot[n] = (int)sqrt(n);

    screenmodeenum screenmode = createworldscreen; // This is used to keep track of which screen mode we're in.
    screenmodeenum oldscreenmode = createworldscreen;

    mapviewenum mapview = relief; // This is used to keep track of which kind of information we want the map to show.

    int poix = 0; // Coordinates of the point of interest, if there is one.
    int poiy = 0;

    int areanwx = -1;
    int areanwy = -1;
    int areanex = -1;
    int areaney = -1;
    int areasex = -1;
    int areasey = -1;
    int areaswx = -1;
    int areaswy = -1; // Corners of the export area, if there is one.

    int seedentry = 0; // The value currently entered into the seed box in the create world screen.

    bool areafromregional = 0; // If this is 1 then the area screen was opened from the regional map, not the global map.

    bool focused = 0; // This will track whether we're focusing on one point or not.
    bool rfocused = 0; // Same thing, for the regional map.

    short regionmargin = 17; // The centre of the regional map can't be closer than this to the northern/southern edges of the global map.

    bool globalmapimagecreated[GLOBALMAPTYPES] = {}; // This will keep track of which global map images have actually been created.

    bool regionalmapimagecreated[GLOBALMAPTYPES] = {}; // This will keep track of which global map images have actually been created.

    int newx = -1;
    int newy = -1; // These are used to locate the new region.

    bool newworld = 0; // Whether to show the new world message.

    bool showcolouroptions = 0; // If this is 1 then we display the appearance preferences options.
    bool showworldproperties = 0; // If this is 1 then we display the properties of the current world.
    bool showglobaltemperaturechart = 0; // If thjis is 1 then we show monthly temperatures for the selected point.
    bool showglobalrainfallchart = 0; // If thjis is 1 then we show monthly rainfall for the selected point.
    bool showregionaltemperaturechart = 0; // If thjis is 1 then we show monthly temperatures for the selected point.
    bool showregionalrainfallchart = 0; // If thjis is 1 then we show monthly rainfall for the selected point.
    bool showsetsize = 0; // If this is 1 then a window is show to set the size for the custom world.
    bool showtectonicchooser = 0; // If this is 1 then we show the panel for creating tectonic-based custom world terrain.
    bool shownontectonicchooser = 0; // If this is 1 then we show the panel for creating non-tectonic-based custom world terrain.
    bool showworldeditproperties = 0; // If this is 1 then we show the panel for editing custom world properties.
    bool showareawarning = 0; // If this is 1 then we show a warning about too-large areas.
    bool showabout = 0; // If this is 1 then we display information about the program.

    for (int n = 0; n < GLOBALMAPTYPES; n++)
        regionalmapimagecreated[n] = 0;

    bool colourschanged = 0; // If this is 1 then the colours have been changed and the maps need to be redrawn.

    float linespace = 8.0f; // Gap between groups of buttons.

    string filepathname = "";
    string filepath = "";

    short creatingworldpass = 0; // Tracks which pass we're on for this section, to ensure that widgets are correctly displayed.
    short completingimportpass = 0; // Tracks which pass we're on for this section, to ensure that widgets are correctly displayed.
    short loadingworldpass = 0; // Tracks which pass we're on for this section, to ensure that widgets are correctly displayed.
    short savingworldpass = 0; // Tracks which pass we're on for this section, to ensure that widgets are correctly displayed.
    short exportingareapass = 0; // Tracks which pass we're on for this section, to ensure that widgets are correctly displayed.
    short generatingregionpass = 0; // Tracks which pass we're on for this section, to ensure that widgets are correctly displayed.
    short generatingtectonicpass = 0; // Tracks which pass we're on for this section, to ensure that widgets are correctly displayed.
    short generatingnontectonicpass = 0; // Tracks which pass we're on for this section, to ensure that widgets are correctly displayed.

    int landmass = 5; // Rough amount of land coverage, for custom worlds.
    int mergefactor = 15; // Amount continents will be removed by merging with the fractal map, for custom worlds.-----------------
    int iterations = 4; // Number of terrain iterations for worlds of terrain type 4.
    int sealeveleditable = 5; // Sea level (0-10) for worlds of terrain type 4.

    vector<vector<bool>> OKmountains(ARRAYWIDTH, vector<bool>(ARRAYHEIGHT, 0)); // This will track mountains that have been imported by the user and which therefore should not be altered to take account of gravity.

    // Now world property variables that can be directly manipulated in the world settings edit panel.

    int currentsize = world->size();
    float currentgravity = world->gravity();
    float currentlunar = world->lunar();
    float currenteccentricity = world->eccentricity();  
    int currentperihelion = world->perihelion();
    int currentrotation = world->rotation();
    float currenttilt = world->tilt();
    float currenttempdecrease = world->tempdecrease();
    int currentnorthpolaradjust = world->northpolaradjust();
    int currentsouthpolaradjust = world->southpolaradjust();
    int currentaveragetemp = world->averagetemp();
    float currentwaterpickup = world->waterpickup();
    int currentglacialtemp = world->glacialtemp();
    bool currentrivers = 1; // This one controls whether or not rivers will be calculated for the custom world.
    bool currentlakes = 1; // This one controls whether or not lakes will be calculated for the custom world.
    bool currentdeltas = 1; // This one controls whether or not river deltas will be calculated for the custom world.

    // Now we prepare map colours. We put them into ImVec4 objects, which can be directly manipulated by the colour picker objects.

    ImVec4 oceancolour;

    oceancolour.x = (float)world->ocean1() / 255.f;
    oceancolour.y = (float)world->ocean2() / 255.f;
    oceancolour.z = (float)world->ocean3() / 255.f;
    oceancolour.w = 1.f;

    ImVec4 deepoceancolour;

    deepoceancolour.x = (float)world->deepocean1() / 255.f;
    deepoceancolour.y = (float)world->deepocean2() / 255.f;
    deepoceancolour.z = (float)world->deepocean3() / 255.f;
    deepoceancolour.w = 1.f;

    ImVec4 basecolour;

    basecolour.x = (float)world->base1() / 255.f;
    basecolour.y = (float)world->base2() / 255.f;
    basecolour.z = (float)world->base3() / 255.f;
    basecolour.w = 1.f;

    ImVec4 grasscolour;

    grasscolour.x = (float)world->grass1() / 255.f;
    grasscolour.y = (float)world->grass2() / 255.f;
    grasscolour.z = (float)world->grass3() / 255.f;
    grasscolour.w = 1.f;

    ImVec4 basetempcolour;

    basetempcolour.x = (float)world->basetemp1() / 255.f;
    basetempcolour.y = (float)world->basetemp2() / 255.f;
    basetempcolour.z = (float)world->basetemp3() / 255.f;
    basetempcolour.w = 1.f;

    ImVec4 highbasecolour;

    highbasecolour.x = (float)world->highbase1() / 255.f;
    highbasecolour.y = (float)world->highbase2() / 255.f;
    highbasecolour.z = (float)world->highbase3() / 255.f;
    highbasecolour.w = 1.f;

    ImVec4 desertcolour;

    desertcolour.x = (float)world->desert1() / 255.f;
    desertcolour.y = (float)world->desert2() / 255.f;
    desertcolour.z = (float)world->desert3() / 255.f;
    desertcolour.w = 1.f;

    ImVec4 highdesertcolour;

    highdesertcolour.x = (float)world->highdesert1() / 255.f;
    highdesertcolour.y = (float)world->highdesert2() / 255.f;
    highdesertcolour.z = (float)world->highdesert3() / 255.f;
    highdesertcolour.w = 1.f;

    ImVec4 colddesertcolour;

    colddesertcolour.x = (float)world->colddesert1() / 255.f;
    colddesertcolour.y = (float)world->colddesert2() / 255.f;
    colddesertcolour.z = (float)world->colddesert3() / 255.f;
    colddesertcolour.w = 1.f;

    ImVec4 eqtundracolour;

    eqtundracolour.x = (float)world->eqtundra1() / 255.f;
    eqtundracolour.y = (float)world->eqtundra2() / 255.f;
    eqtundracolour.z = (float)world->eqtundra3() / 255.f;
    eqtundracolour.w = 1.f;

    ImVec4 tundracolour;

    tundracolour.x = (float)world->tundra1() / 255.f;
    tundracolour.y = (float)world->tundra2() / 255.f;
    tundracolour.z = (float)world->tundra3() / 255.f;
    tundracolour.w = 1.f;

    ImVec4 coldcolour;

    coldcolour.x = (float)world->cold1() / 255.f;
    coldcolour.y = (float)world->cold2() / 255.f;
    coldcolour.z = (float)world->cold3() / 255.f;
    coldcolour.w = 1.f;

    ImVec4 seaicecolour;

    seaicecolour.x = (float)world->seaice1() / 255.f;
    seaicecolour.y = (float)world->seaice2() / 255.f;
    seaicecolour.z = (float)world->seaice3() / 255.f;
    seaicecolour.w = 1.f;

    ImVec4 glaciercolour;

    glaciercolour.x = (float)world->glacier1() / 255.f;
    glaciercolour.y = (float)world->glacier2() / 255.f;
    glaciercolour.z = (float)world->glacier3() / 255.f;
    glaciercolour.w = 1.f;

    ImVec4 saltpancolour;

    saltpancolour.x = (float)world->saltpan1() / 255.f;
    saltpancolour.y = (float)world->saltpan2() / 255.f;
    saltpancolour.z = (float)world->saltpan3() / 255.f;
    saltpancolour.w = 1.f;

    ImVec4 ergcolour;

    ergcolour.x = (float)world->erg1() / 255.f;
    ergcolour.y = (float)world->erg2() / 255.f;
    ergcolour.z = (float)world->erg3() / 255.f;
    ergcolour.w = 1.f;

    ImVec4 wetlandscolour;

    wetlandscolour.x = (float)world->wetlands1() / 255.f;
    wetlandscolour.y = (float)world->wetlands2() / 255.f;
    wetlandscolour.z = (float)world->wetlands3() / 255.f;
    wetlandscolour.w = 1.f;

    ImVec4 lakecolour;

    lakecolour.x = (float)world->lake1() / 255.f;
    lakecolour.y = (float)world->lake2() / 255.f;
    lakecolour.z = (float)world->lake3() / 255.f;
    lakecolour.w = 1.f;

    ImVec4 rivercolour;

    rivercolour.x = (float)world->river1() / 255.f;
    rivercolour.y = (float)world->river2() / 255.f;
    rivercolour.z = (float)world->river3() / 255.f;
    rivercolour.w = 1.f;

    ImVec4 sandcolour;

    sandcolour.x = (float)world->sand1() / 255.f;
    sandcolour.y = (float)world->sand2() / 255.f;
    sandcolour.z = (float)world->sand3() / 255.f;
    sandcolour.w = 1.f;

    ImVec4 mudcolour;

    mudcolour.x = (float)world->mud1() / 255.f;
    mudcolour.y = (float)world->mud2() / 255.f;
    mudcolour.z = (float)world->mud3() / 255.f;
    mudcolour.w = 1.f;

    ImVec4 shinglecolour;

    shinglecolour.x = (float)world->shingle1() / 255.f;
    shinglecolour.y = (float)world->shingle2() / 255.f;
    shinglecolour.z = (float)world->shingle3() / 255.f;
    shinglecolour.w = 1.f;

    ImVec4 mangrovecolour;

    mangrovecolour.x = (float)world->mangrove1() / 255.f;
    mangrovecolour.y = (float)world->mangrove2() / 255.f;
    mangrovecolour.z = (float)world->mangrove3() / 255.f;
    mangrovecolour.w = 1.f;

    ImVec4 highlightcolour;

    highlightcolour.x = (float)world->highlight1() / 255.f;
    highlightcolour.y = (float)world->highlight2() / 255.f;
    highlightcolour.z = (float)world->highlight3() / 255.f;
    highlightcolour.w = 1.f;

    // Prepare some other variables for the appearance controls.

    float shadingland = world->landshading();
    float shadinglake = world->lakeshading();
    float shadingsea = world->seashading();
    float marblingland = world->landmarbling();
    float marblinglake = world->lakemarbling();
    float marblingsea = world->seamarbling();

    int minriverflowglobal = world->minriverflowglobal();
    int minriverflowregional = world->minriverflowregional();

    int globalriversentry = world->minriverflowglobal();
    int regionalriversentry = world->minriverflowregional();

    int shadingdir;

    if (world->shadingdir() == 4)
        shadingdir = 0;

    if (world->shadingdir() == 6)
        shadingdir = 1;

    if (world->shadingdir() == 2)
        shadingdir = 2;

    if (world->shadingdir() == 8)
        shadingdir = 3;

    int snowchange = world->snowchange() - 1;
    int seaiceappearance = world->seaiceappearance() - 1;
    bool colourcliffs = world->colourcliffs();
    bool mangroves = world->showmangroves();

    sf::Clock deltaClock;
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(window, event);

            if (event.type == sf::Event::Closed)
                window.close();
        }

        ImGuiIO& io = ImGui::GetIO();

        ImGui::SFML::Update(window, deltaClock.restart());

        if (screenmode == movingtoglobalmapscreen)
            screenmode = globalmapscreen;        

        ImGui::PushFont(font1);

        //ImGui::ShowDemoWindow();

        // First, draw the GUI.

        // Create world screen

        if (screenmode == createworldscreen)
        {
            showglobaltemperaturechart = 0;
            showglobalrainfallchart = 0;
            showregionaltemperaturechart = 0;
            showregionalrainfallchart = 0;
            showworldproperties = 0;
            showworldeditproperties = 0;
            newworld = 0;

            // If there is an update available, show an alert.

            if (latestversion > currentversion)
            {
                ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 10, main_viewport->WorkPos.y + 10), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize(ImVec2(224, 107), ImGuiCond_FirstUseEver);

                ImGui::Begin("Update available!", NULL, window_flags);
               
                ImGui::Text("Click here to visit the website.");
                ImGui::Text(" ");

                ImGui::Text(" ");

                ImGui::SameLine(95.0f);

                if (ImGui::Button("Go"))
                {
                    ShellExecute(0, 0, L"https://undiscoveredworlds.blogspot.com/2019/01/what-is-undiscovered-worlds.html", 0, 0, SW_SHOW);
                }

                ImGui::End();
            }
            
            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 483, main_viewport->WorkPos.y + 206), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(249, 90), ImGuiCond_FirstUseEver);
            
            ImGui::Begin("Create world",NULL, window_flags);

            ImGui::SetNextItemWidth(-1.0f);
            ImGui::InputInt(" ", &seedentry);

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Please enter a seed number, which will be used to calculate the new world. The same number will always yield the same world.");

            if (seedentry < 0)
                seedentry = 0;

            string loadtext = "Load a world.";

            if (brandnew == 1)
            {
                if (ImGui::Button("Load")) // This opens the load world dialogue. We check its results later.
                {
                    ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".uww", ".");

                    loadingworld = 1;
                }
            }
            else
            {
                loadtext = "Return to the world map.";
                
                if (ImGui::Button("Cancel"))
                    screenmode = globalmapscreen;
            }

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip(loadtext.c_str());

            ImGui::SameLine();

            if (ImGui::Button("Custom"))
            {
                    showsetsize = 1;
            }

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Create a custom world.");

            ImGui::SameLine();

            if (ImGui::Button("Random"))
            {
                int seed = random(0, 9);

                for (int n = 1; n <= 7; n++)
                {
                    if (n == 7)
                        seed = seed + (random(1, 9) * (int)pow(10, n));
                    else
                        seed = seed + (random(0, 9) * (int)pow(10, n));
                }

                seedentry = seed;
            }

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Roll a random seed number.");

            ImGui::SameLine();

            if (ImGui::Button("OK"))
            {
                // Accept seed and generate world

                world->setseed(seedentry);

                screenmode = creatingworldscreen;
            }

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Create a world from this seed number.");

            ImGui::End();
        }

        // Creating world screen

        if (screenmode == creatingworldscreen)
        {
            if (creatingworldpass<10)
            {
                ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 506, main_viewport->WorkPos.y + 171), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize(ImVec2(173, 68), ImGuiCond_FirstUseEver);
                ImGui::Begin("Please wait...##createworld ", NULL, window_flags);
                ImGui::Text("Generating world...");
                ImGui::End();

                creatingworldpass++;
            }
            else
            {
                updatereport("Generating world from seed: " + to_string(world->seed()) + ":");
                updatereport("");

                initialiseworld(*world);
                world->clear();

                changeworldproperties(*world);

                adjustforsize(*world, globaltexturesize, *globalelevationimage, *globaltemperatureimage, *globalprecipitationimage, *globalclimateimage, *globalriversimage, *globalreliefimage, *highlightimage, highlightsize, *minihighlightimage, minihighlightsize);
                
                minihighlighttexture->loadFromImage(*minihighlightimage);
                minihighlight->setTexture(*minihighlighttexture);
                minihighlight->setOrigin((float)minihighlightsize /2.0f, (float)minihighlightsize / 2.0f);

                for (int n = 0; n < GLOBALMAPTYPES; n++) // Set all map types as unviewed, to force them to be redrawn when called up
                    globalmapimagecreated[n] = 0;

                int contno = random(1, 9);

                int thismergefactor = random(1, 15);

                if (random(1, 12) == 1) // Fairly rarely, have more fragmented continents
                    thismergefactor = random(1, 25);

                iterations = 4; // This is for worlds of terrain type 4.

                for (int n = 0; n < 3; n++)
                {
                    if (random(1, 2) == 1)
                        iterations++;
                    else
                        iterations--;
                }

                if (iterations < 1)
                    iterations = 1;

                if (iterations > 7)
                    iterations = 7;

                vector<vector<int>> mountaindrainage(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));
                vector<vector<bool>> shelves(ARRAYWIDTH, vector<bool>(ARRAYHEIGHT, 0));

                // Actually generate the world

                generateglobalterrain(*world, 0, iterations, thismergefactor, -1, -1, landshape, chainland, mountaindrainage, shelves,squareroot);
                generateglobalclimate(*world, 1, 1, 1, smalllake, largelake, landshape, mountaindrainage, shelves);

                // Now draw a new map

                mapview = relief;

                drawglobalmapimage(mapview, *world, globalmapimagecreated, *globalelevationimage, *globaltemperatureimage, *globalprecipitationimage, *globalclimateimage, *globalriversimage, *globalreliefimage, *displayglobalelevationimage, *displayglobaltemperatureimage, *displayglobalprecipitationimage, *displayglobalclimateimage, *displayglobalriversimage, *displayglobalreliefimage);

                globalmaptexture->loadFromImage(*displayglobalreliefimage);
                globalmap->setTexture(*globalmaptexture);

                updatereport("");
                updatereport("World generation completed.");
                updatereport("");

                focused = 0;

                creatingworldpass = 0;
                screenmode = movingtoglobalmapscreen;
                newworld = 1;
            }
        }

        // Global map screen

        if (screenmode == globalmapscreen)
        {
            showworldeditproperties = 0;
            showregionaltemperaturechart = 0;
            showregionalrainfallchart = 0;
            
            areafromregional = 0;
            brandnew = 0;
            
            // Main controls.

            string title;

            if (world->seed() >= 0)
                title = "Seed: " + to_string(world->seed());
           else
                title = "Custom";

            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 10, main_viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(161, 549), ImGuiCond_FirstUseEver);

            ImGui::Begin(title.c_str(), NULL, window_flags);

            ImVec2 pos = ImGui::GetWindowPos();

            ImGui::Text("World controls:");

            if (standardbutton("New world"))
            {
                brandnew = 0;
                seedentry = 0;

                screenmode = createworldscreen;
            }

            if (standardbutton("Load world"))
            {
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".uww", ".");

                loadingworld = 1;
            }

            if (standardbutton("Save world"))
            {
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".uww", ".");

                savingworld = 1;
            }

            if (standardbutton("Custom world"))
            {
                showsetsize = 1;
            }

            ImGui::Dummy(ImVec2(0.0f, linespace));

            ImGui::SetNextItemWidth(0);

            ImGui::Text("Export options:");

            ImGui::PushItemWidth(100.0f);

            if (standardbutton("World maps"))
            {
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".png", ".");

                exportingworldmaps = 1;
            }

            if (standardbutton("Area maps"))
            {
                mapview = relief;

                drawglobalmapimage(mapview, *world, globalmapimagecreated, *globalelevationimage, *globaltemperatureimage, *globalprecipitationimage, *globalclimateimage, *globalriversimage, *globalreliefimage, *displayglobalelevationimage, *displayglobaltemperatureimage, *displayglobalprecipitationimage, *displayglobalclimateimage, *displayglobalriversimage, *displayglobalreliefimage);

                globalmaptexture->loadFromImage(*displayglobalreliefimage);
                globalmap->setTexture(*globalmaptexture);
                
                screenmode = exportareascreen;
                areafromregional = 0;
            }

            ImGui::Dummy(ImVec2(0.0f, linespace));

            ImGui::SetNextItemWidth(0);

            ImGui::Text("Display map type:");

            ImGui::PushItemWidth(100.0f);

            if (standardbutton("Relief"))
            {
                mapview = relief;

                drawglobalmapimage(mapview, *world, globalmapimagecreated, *globalelevationimage, *globaltemperatureimage, *globalprecipitationimage, *globalclimateimage, *globalriversimage, *globalreliefimage, *displayglobalelevationimage, *displayglobaltemperatureimage, *displayglobalprecipitationimage, *displayglobalclimateimage, *displayglobalriversimage, *displayglobalreliefimage);

                globalmaptexture->loadFromImage(*displayglobalreliefimage);
                globalmap->setTexture(*globalmaptexture);
            }

            if (standardbutton("Elevation"))
            {
                mapview = elevation;

                drawglobalmapimage(mapview, *world, globalmapimagecreated, *globalelevationimage, *globaltemperatureimage, *globalprecipitationimage, *globalclimateimage, *globalriversimage, *globalreliefimage, *displayglobalelevationimage, *displayglobaltemperatureimage, *displayglobalprecipitationimage, *displayglobalclimateimage, *displayglobalriversimage, *displayglobalreliefimage);

                globalmaptexture->loadFromImage(*displayglobalelevationimage);
                globalmap->setTexture(*globalmaptexture);
            }

            if (standardbutton("Temperature"))
            {
                mapview = temperature;

                drawglobalmapimage(mapview, *world, globalmapimagecreated, *globalelevationimage, *globaltemperatureimage, *globalprecipitationimage, *globalclimateimage, *globalriversimage, *globalreliefimage, *displayglobalelevationimage, *displayglobaltemperatureimage, *displayglobalprecipitationimage, *displayglobalclimateimage, *displayglobalriversimage, *displayglobalreliefimage);

                globalmaptexture->loadFromImage(*displayglobaltemperatureimage);
                globalmap->setTexture(*globalmaptexture);
            }

            if (standardbutton("Precipitation"))
            {
                mapview = precipitation;

                drawglobalmapimage(mapview, *world, globalmapimagecreated, *globalelevationimage, *globaltemperatureimage, *globalprecipitationimage, *globalclimateimage, *globalriversimage, *globalreliefimage, *displayglobalelevationimage, *displayglobaltemperatureimage, *displayglobalprecipitationimage, *displayglobalclimateimage, *displayglobalriversimage, *displayglobalreliefimage);

                globalmaptexture->loadFromImage(*displayglobalprecipitationimage);
                globalmap->setTexture(*globalmaptexture);
            }

            if (standardbutton("Climate"))
            {
                mapview = climate;

                drawglobalmapimage(mapview, *world, globalmapimagecreated, *globalelevationimage, *globaltemperatureimage, *globalprecipitationimage, *globalclimateimage, *globalriversimage, *globalreliefimage, *displayglobalelevationimage, *displayglobaltemperatureimage, *displayglobalprecipitationimage, *displayglobalclimateimage, *displayglobalriversimage, *displayglobalreliefimage);

                globalmaptexture->loadFromImage(*displayglobalclimateimage);
                globalmap->setTexture(*globalmaptexture);
            }

            if (standardbutton("Rivers"))
            {
                mapview = rivers;

                drawglobalmapimage(mapview, *world, globalmapimagecreated, *globalelevationimage, *globaltemperatureimage, *globalprecipitationimage, *globalclimateimage, *globalriversimage, *globalreliefimage, *displayglobalelevationimage, *displayglobaltemperatureimage, *displayglobalprecipitationimage, *displayglobalclimateimage, *displayglobalriversimage, *displayglobalreliefimage);

                globalmaptexture->loadFromImage(*displayglobalriversimage);
                globalmap->setTexture(*globalmaptexture);
            }

            ImGui::Dummy(ImVec2(0.0f, linespace));

            ImGui::SetNextItemWidth(0);

            ImGui::Text("Other controls:");

            ImGui::PushItemWidth(100.0f);

            if (standardbutton("Properties"))
                toggle(showworldproperties);

            if (standardbutton("Appearance"))
                toggle(showcolouroptions);

            if (standardbutton("Zoom"))
            {
                if (focused == 1)
                {
                    globalmaptexture->loadFromImage(*displayglobalreliefimage);
                    globalmap->setTexture(*globalmaptexture);
                    minimap->setTexture(*globalmaptexture);

                    newx = poix;
                    newy = poiy;

                    screenmode = generatingregionscreen;
                }
            }

            ImGui::End();

            // Now the text box.

            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 180, main_viewport->WorkPos.y + 539), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(1023, 155), ImGuiCond_FirstUseEver);
            title = "Information";

            ImGui::Begin(title.c_str(), NULL, window_flags);

            float pos1 = 10.0f;
            float pos2 = 100.0f;
            float pos3 = 220.0f;
            float pos4 = 350.0f;
            float pos5 = 570.0f;
            float pos6 = 690.0f;

            float pos7 = 850.0f;

            if (focused == 1)
            {                
                int sealevel = world->sealevel();

                bool lake = 0;             
                if (world->truelake(poix, poiy) != 0)
                    lake = 1;

                bool sea = 0;
                if (world->sea(poix, poiy) == 1)
                    sea = 1;

                bool river = 0;
                if (sea == 0 && world->riveraveflow(poix, poiy) > 0 && lake == 0)
                    river = 1;

                // Longitude
                
                int longdegrees = 0;
                int longminutes = 0;
                int longseconds = 0;
                bool longneg = 0;

                world->longitude(poix, longdegrees, longminutes, longseconds,longneg);

                string longtext = to_string(longdegrees) + degree + " " + to_string(longminutes) + "' " + to_string(longseconds);

                if (longneg)
                    longtext = "-" + longtext;

                // Latitude

                int latdegrees = 0;
                int latminutes = 0;
                int latseconds = 0;
                bool latneg = 0;

                world->latitude(poiy, latdegrees, latminutes, latseconds,latneg);

                string lattext = to_string(latdegrees) + degree + " " + to_string(latminutes) + "' " + to_string(latseconds);

                if (latneg)
                    lattext = "-" + lattext;

                // Wind

                int wind = world->wind(poix, poiy);
                string windtext = "";

                if (wind > 0)
                    windtext = "westerly";

                if (wind < 0)
                    windtext = "easterly";

                if (wind == 0 || wind > 50)
                    windtext = "none";

                // Elevation

                string elevationtext = "";

                int pointelevation = world->map(poix, poiy);

                if (world->lakesurface(poix, poiy) != 0)
                    pointelevation = world->lakesurface(poix, poiy) - 1;

                if (sea == 0)
                {
                    if (pointelevation > sealevel)
                    {
                        elevationtext = formatnumber(pointelevation - sealevel) + " metres";

                        if (world->seatotal() != 0)
                            elevationtext = elevationtext + " above sea level";
                    }
                    else
                    {
                        if (lake)
                            elevationtext = formatnumber(sealevel - pointelevation) + " metres below sea level";
                        else
                        {
                            if (world->seatotal() != 0)
                                elevationtext = formatnumber(world->lakesurface(poix, poiy) - sealevel) + " metres above sea level";
                            else
                                elevationtext = formatnumber(world->lakesurface(poix, poiy) - sealevel) + " metres";
                        }
                    }
                }
                else
                    elevationtext = formatnumber(sealevel - pointelevation) + " metres below sea level";

                // Climate
                
                short climatetype = world->climate(poix, poiy);
                string climatetext = getclimatename(climatetype) + " (" + getclimatecode(climatetype) + ")";


                // Specials

                string specialstext = "";

                int special = world->special(poix, poiy);

                if (special == 110)
                    specialstext = "Salt pan";

                if (special == 120)
                    specialstext = "Dunes";

                if (special == 130)
                    specialstext = "Wetlands";

                if (special == 131)
                    specialstext = "Brackish wetlands";

                if (special == 132)
                    specialstext = "Salt wetlands";

                if (world->volcano(poix, poiy) > 0)
                {
                    if (sea)
                        specialstext = "Submarine volcano";
                    else                    
                        specialstext = "Volcano";
                }

                // Glacial

                string glacialtext = "";

                if (!sea && (world->jantemp(poix, poiy) + world->jultemp(poix, poiy)) / 2 <= world->glacialtemp())
                    glacialtext = "Ancient glacial region";

                string jantemptext = "";
                string jultemptext = "";
                string janraintext = "";
                string julraintext = "";

                // Flow

                string janflowtext = "";
                string julflowtext = "";
                string flowdirtext = "";

                if (river)
                {
                    switch (world->riverdir(poix, poiy))
                    {
                    case 1:
                        flowdirtext = "north";
                        break;

                    case 2:
                        flowdirtext = "northeast";
                        break;

                    case 3:
                        flowdirtext = "east";
                        break;

                    case 4:
                        flowdirtext = "southeast";
                        break;

                    case 5:
                        flowdirtext = "south";
                        break;

                    case 6:
                        flowdirtext = "southwest";
                        break;

                    case 7:
                        flowdirtext = "west";
                        break;

                    case 8:
                        flowdirtext = "northwest";
                        break;
                    }

                    janflowtext = formatnumber(world->riverjan(poix, poiy)) + " m" + cube + "/s";
                    julflowtext = formatnumber(world->riverjul(poix, poiy)) + " m" + cube + "/s";
                }

                // Lake

                string lakedepthtext = "";

                if (lake)
                {
                    elevationtext = formatnumber(world->lakesurface(poix, poiy) - sealevel) + " metres";

                    if (world->seatotal() != 0)
                        elevationtext = elevationtext + " above sea level";

                    int depth = world->lakesurface(poix, poiy) - world->nom(poix, poiy);

                    string salt = "";

                    if (world->special(poix, poiy) == 100)
                        salt = " (salty)";

                    lakedepthtext = formatnumber(depth) + " metres" + salt;
                }

                // Sea ice

                string seaicetext = "none";

                if (world->seaice(poix, poiy) == 2)
                    seaicetext= "permanent";
                else
                {
                    if (world->seaice(poix, poiy) == 1)
                        seaicetext = "seasonal";
                }

                // Now display all that.

                // Line 1

                ImGui::Text("Longitude:");
                ImGui::SameLine((float)pos2);
                ImGui::Text(longtext.c_str());

                ImGui::SameLine((float)pos3);
                ImGui::Text("Elevation:");
                ImGui::SameLine((float)pos4);
                ImGui::Text(elevationtext.c_str());

                if (lake)
                {
                    ImGui::SameLine((float)pos5);

                    lakedepthtext = "Lake depth:  " + lakedepthtext;
                    ImGui::Text(lakedepthtext.c_str());
                }

                if (river)
                {
                    ImGui::SameLine((float)pos5);
                    ImGui::Text("River direction:");
                    ImGui::SameLine((float)pos6);
                    ImGui::Text(flowdirtext.c_str());
                }

                ImGui::SameLine((float)pos7);
                ImGui::Text("Charts:");

                // Line 2

                ImGui::Text("Latitude:");
                ImGui::SameLine((float)pos2);
                ImGui::Text(lattext.c_str());

                ImGui::SameLine((float)pos3);
                ImGui::Text("Prevailing wind:");
                ImGui::SameLine((float)pos4);
                ImGui::Text(windtext.c_str());

                if (river)
                {
                    ImGui::SameLine((float)pos5);
                    ImGui::Text("January flow:");
                    ImGui::SameLine((float)pos6);
                    ImGui::Text(janflowtext.c_str());
                }

                ImGui::SameLine((float)pos7+20);
                if (ImGui::Button("Temperature", ImVec2(120.0f, 0.0f)))
                    toggle(showglobaltemperaturechart);

                // Line 3

                ImGui::Text(" ");

                if (river)
                {                   
                    ImGui::SameLine((float)pos5);
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 4.0f); // Because the buttons to the right moves this line down a little
                    ImGui::Text("July flow:");
                    ImGui::SameLine((float)pos6);
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 4.0f); // Because the buttons to the right moves this line down a little
                    ImGui::Text(julflowtext.c_str());
                }

                ImGui::SameLine((float)pos7+20);
                if (ImGui::Button("Precipitation", ImVec2(120.0f, 0.0f)))
                    toggle(showglobalrainfallchart);

                // Line 4

                if (sea)
                {
                    ImGui::Text("Sea ice:");
                    ImGui::SameLine((float)pos2);
                    ImGui::Text(seaicetext.c_str());
                }
                else
                {
                    ImGui::Text("Climate:");
                    ImGui::SameLine((float)pos2);
                    ImGui::Text(climatetext.c_str());
                }

                // Line 5

                ImGui::Text(" ");

                ImGui::SameLine((float)pos2);

                if (specialstext != "")
                    ImGui::Text(specialstext.c_str());
                else
                    ImGui::Text(glacialtext.c_str());
            }

            if (newworld==1)
                ImGui::Text("Welcome to a new world!");

            ImGui::SetCursorPosX(993.0);
            ImGui::SetCursorPosY(122.0f);

            if (ImGui::Button("?"))
                toggle(showabout);

            ImGui::End();

            // Now check to see if the map has been clicked on.

            if (window.hasFocus() && sf::Mouse::isButtonPressed(sf::Mouse::Left) && io.WantCaptureMouse == 0)
            {
                sf::Vector2i mousepos = sf::Mouse::getPosition(window);

                float mult = ((float)world->width()+1.0f) / (float)DISPLAYMAPSIZEX;

                float fpoix = (float)(mousepos.x - globalmapxpos) * mult;
                float fpoiy = (float)(mousepos.y - globalmapypos) * mult;

                poix = (int)fpoix;
                poiy = (int)fpoiy;

                if (poix >= 0 && poix <= world->width() && poiy >= 0 && poiy <= world->height())
                {
                    focused = 1;
                    newworld = 0;

                    highlight->setPosition(sf::Vector2f((float)mousepos.x, (float)mousepos.y));
                }
                else
                {
                    if (focused == 1)
                    {
                        focused = 0;
                        poix = -1;
                        poiy = -1;
                    }
                }
            }
        }

        // Regional map screen

        if (screenmode == regionalmapscreen)
        {
            showglobaltemperaturechart = 0;
            showglobalrainfallchart = 0;
            showworldeditproperties = 0;
            
            areafromregional = 1;
            
            showsetsize = 0;
            newworld = 0;

            // Main controls.

            string title;

            if (world->seed() >= 0)
                title = "Seed: " + to_string(world->seed());
            else
                title = "Custom";

            title = title + "##regional";

            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 10, main_viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(161, 449), ImGuiCond_FirstUseEver);

            ImGui::Begin(title.c_str(), NULL, window_flags);

            ImGui::Text("World controls:");

            ImGui::PushItemWidth(100.0f);

            if (standardbutton("World map"))
            {
                focused = 0;
                poix = -1;
                poiy = -1;

                for (int i = 0; i < regionalimagewidth; i++)
                {
                    for (int j = 0; j < regionalimageheight; j++)

                        regionalreliefimage->setPixel(i, j, sf::Color::Black);
                }

                regionalmaptexture->loadFromImage(*regionalreliefimage);
                regionalmap->setTexture(*regionalmaptexture);

                screenmode = globalmapscreen;

                showcolouroptions = 0;
                showworldproperties = 0;
                showregionalrainfallchart = 0;
                showregionaltemperaturechart = 0;
            }

            ImGui::Dummy(ImVec2(0.0f, linespace));

            ImGui::SetNextItemWidth(0);

            ImGui::Text("Export options:");

            ImGui::PushItemWidth(100.0f);

            if (standardbutton("Regional maps"))
            {
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".png", ".");

                exportingregionalmaps = 1;
            }

            if (standardbutton("Area maps"))
            {
                screenmode = exportareascreen;
                areafromregional = 1;
            }

            ImGui::Dummy(ImVec2(0.0f, linespace));

            ImGui::SetNextItemWidth(0);

            ImGui::Text("Display map type:");

            ImGui::PushItemWidth(100.0f);

            if (standardbutton("Relief"))
            {
                mapview = relief;

                drawregionalmapimage(mapview, *world, *region, regionalmapimagecreated, *regionalelevationimage, *regionaltemperatureimage, *regionalprecipitationimage, *regionalclimateimage, *regionalriversimage, *regionalreliefimage);

                regionalmaptexture->loadFromImage(*regionalreliefimage);
                regionalmap->setTexture(*regionalmaptexture);

            }

            if (standardbutton("Elevation"))
            {
                mapview = elevation;

                drawregionalmapimage(mapview, *world, *region, regionalmapimagecreated, *regionalelevationimage, *regionaltemperatureimage, *regionalprecipitationimage, *regionalclimateimage, *regionalriversimage, *regionalreliefimage);

                regionalmaptexture->loadFromImage(*regionalelevationimage);
                regionalmap->setTexture(*regionalmaptexture);
            }

            if (standardbutton("Temperature"))
            {
                mapview = temperature;

                drawregionalmapimage(mapview, *world, *region, regionalmapimagecreated, *regionalelevationimage, *regionaltemperatureimage, *regionalprecipitationimage, *regionalclimateimage, *regionalriversimage, *regionalreliefimage);

                regionalmaptexture->loadFromImage(*regionaltemperatureimage);
                regionalmap->setTexture(*regionalmaptexture);
            }

            if (standardbutton("Precipitation"))
            {
                mapview = precipitation;

                drawregionalmapimage(mapview, *world, *region, regionalmapimagecreated, *regionalelevationimage, *regionaltemperatureimage, *regionalprecipitationimage, *regionalclimateimage, *regionalriversimage, *regionalreliefimage);

                regionalmaptexture->loadFromImage(*regionalprecipitationimage);
                regionalmap->setTexture(*regionalmaptexture);
            }

            if (standardbutton("Climate"))
            {
                mapview = climate;

                drawregionalmapimage(mapview, *world, *region, regionalmapimagecreated, *regionalelevationimage, *regionaltemperatureimage, *regionalprecipitationimage, *regionalclimateimage, *regionalriversimage, *regionalreliefimage);

                regionalmaptexture->loadFromImage(*regionalclimateimage);
                regionalmap->setTexture(*regionalmaptexture);
            }

            if (standardbutton("Rivers"))
            {
                mapview = rivers;

                drawregionalmapimage(mapview, *world, *region, regionalmapimagecreated, *regionalelevationimage, *regionaltemperatureimage, *regionalprecipitationimage, *regionalclimateimage, *regionalriversimage, *regionalreliefimage);

                regionalmaptexture->loadFromImage(*regionalriversimage);
                regionalmap->setTexture(*regionalmaptexture);
            }

            ImGui::Dummy(ImVec2(0.0f, linespace));

            ImGui::SetNextItemWidth(0);

            ImGui::Text("Other controls:");

            ImGui::PushItemWidth(100.0f);

            if (standardbutton("Properties"))
                toggle(showworldproperties);

            if (standardbutton("Appearance"))
                toggle(showcolouroptions);

            ImGui::End();

            // Now the text box.

            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 180, main_viewport->WorkPos.y + 539), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(1023, 155), ImGuiCond_FirstUseEver);

            title = "Information##regional ";

            ImGui::Begin(title.c_str(), NULL, window_flags);

            if (focused == 1)
            {
                int sealevel = world->sealevel();
                int width = world->width();
                int height = world->height();

                int xx = region->leftx() + poix / 16;
                int yy = region->lefty() + poiy / 16; // Coordinates of the gobal cell we're in.

                bool lake = 0;
                if (region->truelake(poix, poiy) != 0)
                    lake = 1;

                bool sea = 0;
                if (region->sea(poix, poiy) == 1)
                    sea = 1;

                bool river = 0;
                if (lake == 0 && region->map(poix,poiy)>sealevel && (region->riveraveflow(poix, poiy) > 0 || region->fakeaveflow(poix, poiy) > 0))
                    river = 1;

                bool beach = 0;

                if (region->sand(poix, poiy) || region->shingle(poix, poiy) || region->mud(poix, poiy))
                    beach = 1;

                float pos1 = 10.0f;
                float pos2 = 100.0f;
                float pos3 = 220.0f;
                float pos4 = 350.0f;
                float pos5 = 570.0f;
                float pos6 = 690.0f;
                float pos7 = 850.0f;

                // Longitude

                int longdegrees = 0;
                int longminutes = 0;
                int longseconds = 0;
                bool longneg = 0;

                region->longitude(poix, xx, poix / 16, width, longdegrees, longminutes, longseconds, longneg);

                string longtext = to_string(longdegrees) + degree + " " + to_string(longminutes) + "' " + to_string(longseconds);

                if (longneg)
                    longtext = "-" + longtext;

                // Latitude

                int latdegrees = 0;
                int latminutes = 0;
                int latseconds = 0;
                bool latneg = 0;

                region->latitude(poiy, yy, poiy / 16, height, latdegrees, latminutes, latseconds, latneg);

                string lattext = to_string(latdegrees) + degree + " " + to_string(latminutes) + "' " + to_string(latseconds);

                if (latneg)
                    lattext = "-" + lattext;

                // Wind

                int wind = world->wind(xx, yy);
                string windtext = "";

                if (wind > 0)
                    windtext = "westerly";

                if (wind < 0)
                    windtext = "easterly";

                if (wind == 0 || wind > 50)
                    windtext = "none";

                // Elevation

                string elevationtext = "";

                int pointelevation = region->map(poix, poiy);

                if (region->lakesurface(poix, poiy) != 0)
                    pointelevation = region->lakesurface(poix, poiy);


                if (sea == 0)
                {
                    if (pointelevation > sealevel)
                    {
                        elevationtext = formatnumber(pointelevation - sealevel) + " metres";

                        if (world->seatotal() != 0)
                        {
                            if (beach)
                                elevationtext = "Sea level";
                            else
                                elevationtext = elevationtext + " above sea level";
                        }
                    }
                    else
                    {
                        if (lake)
                            elevationtext = formatnumber(sealevel - pointelevation) + " metres below sea level";
                        else
                        {
                            if (world->seatotal() != 0)
                                elevationtext = formatnumber(world->lakesurface(poix, poiy) - sealevel) + " metres above sea level";
                            else
                                elevationtext = formatnumber(world->lakesurface(poix, poiy) - sealevel) + " metres";
                        }
                    }
                }
                else
                    elevationtext = formatnumber(sealevel - pointelevation) + " metres below sea level";

                // Climate

                short climatetype = region->climate(poix, poiy);
                string climatetext = getclimatename(climatetype) + " (" + getclimatecode(climatetype) + ")";

                // Specials

                string specialstext = "";

                int special = region->special(poix, poiy);

                if (region->barrierisland(poix, poiy) == 0 && (region->rivervalley(poix, poiy) == 1 || river))
                    specialstext = "River valley";
                
                if (special == 110)
                    specialstext = "Salt pan";

                if (special == 120)
                    specialstext = "Dunes";

                if (special == 130)
                    specialstext = "Wetlands";

                if (special == 131)
                    specialstext = "Brackish wetlands";

                if (special == 132)
                    specialstext = "Salt wetlands";

                if (region->mud(poix, poiy))
                    specialstext = "Mud flats";

                if (region->sand(poix, poiy))
                    specialstext = "Sandy beach";

                if (region->shingle(poix, poiy))
                    specialstext = "Shingle beach";

                if (region->sand(poix, poiy) && region->shingle(poix, poiy))
                    specialstext = "Sandy/shingle beach";

                if (region->mud(poix, poiy) && region->sand(poix, poiy))
                    specialstext = "Muddy/sandy beach";

                if (region->mud(poix, poiy) && region->shingle(poix, poiy))
                    specialstext = "Muddy/shingle beach";

                if (region->mud(poix, poiy) && region->sand(poix,poiy) && region->shingle(poix, poiy))
                    specialstext = "Muddy/sandy/shingle beach";

                if (region->volcano(poix, poiy))
                {
                    if (sea)
                        specialstext = "Submarine volcano";
                    else
                        specialstext = "Volcano";
                }

                if (region->barrierisland(poix, poiy))
                {
                    if (region->sand(poix, poiy))
                        specialstext = "Sandbar";
                    else
                    {
                        if (specialstext == "")
                            specialstext = "Barrier island";
                        else
                            specialstext = "Barrier island. " + specialstext;
                    }
                }

                if (region->mangrove(poix, poiy))
                    specialstext = specialstext + ". Mangrove";

                // Glacial

                string glacialtext = "";

                //if (!sea && (region->jantemp(poix, poiy) + region->jultemp(poix, poiy)) / 2 <= world->glacialtemp())
                    //glacialtext = "Ancient glacial region";

                string jantemptext = "";
                string jultemptext = "";
                string janraintext = "";
                string julraintext = "";

                // Flow

                string janflowtext = "";
                string julflowtext = "";
                string flowdirtext = "";

                if (river)
                {
                    switch (region->riverdir(poix, poiy))
                    {
                    case 1:
                        flowdirtext = "north";
                        break;

                    case 2:
                        flowdirtext = "northeast";
                        break;

                    case 3:
                        flowdirtext = "east";
                        break;

                    case 4:
                        flowdirtext = "southeast";
                        break;

                    case 5:
                        flowdirtext = "south";
                        break;

                    case 6:
                        flowdirtext = "southwest";
                        break;

                    case 7:
                        flowdirtext = "west";
                        break;

                    case 8:
                        flowdirtext = "northwest";
                        break;
                    }

                    int janflow = region->riverjan(poix, poiy);
                    int julflow = region->riverjul(poix, poiy);

                    if (flowdirtext == "") // It must be a fake river!
                    {
                        switch (region->fakedir(poix, poiy))
                        {
                        case 1:
                            flowdirtext = "north";
                            break;

                        case 2:
                            flowdirtext = "northeast";
                            break;

                        case 3:
                            flowdirtext = "east";
                            break;

                        case 4:
                            flowdirtext = "southeast";
                            break;

                        case 5:
                            flowdirtext = "south";
                            break;

                        case 6:
                            flowdirtext = "southwest";
                            break;

                        case 7:
                            flowdirtext = "west";
                            break;

                        case 8:
                            flowdirtext = "northwest";
                            break;
                        }

                        janflow = region->fakejan(poix, poiy);
                        julflow = region->fakejul(poix, poiy);
                    }

                    janflowtext = formatnumber(janflow) + " m" + cube + "/s";
                    julflowtext = formatnumber(julflow) + " m" + cube + "/s";
                }

                // Lake

                string lakedepthtext = "";

                if (lake)
                {
                    elevationtext = formatnumber(region->lakesurface(poix, poiy) - sealevel) + " metres";

                    if (world->seatotal() != 0)
                        elevationtext = elevationtext + " above sea level";

                    int depth = region->lakesurface(poix, poiy) - region->map(poix, poiy);

                    string salt = "";

                    if (region->special(poix, poiy) == 100)
                        salt = " (salty)";

                    lakedepthtext = formatnumber(depth) + " metres" + salt;
                }

                // Sea ice

                string seaicetext = "none";

                if (region->seaice(poix, poiy) == 2)
                    seaicetext = "permanent";
                else
                {
                    if (region->seaice(poix, poiy) == 1)
                        seaicetext = "seasonal";
                }

                // Now display all that.

                // Line 1

                ImGui::Text("Longitude:");
                ImGui::SameLine((float)pos2);
                ImGui::Text(longtext.c_str());

                ImGui::SameLine((float)pos3);
                ImGui::Text("Elevation:");
                ImGui::SameLine((float)pos4);
                ImGui::Text(elevationtext.c_str());

                if (lake)
                {
                    ImGui::SameLine((float)pos5);

                    lakedepthtext = "Lake depth:  " + lakedepthtext;
                    ImGui::Text(lakedepthtext.c_str());
                }

                if (river)
                {
                    ImGui::SameLine((float)pos5);
                    ImGui::Text("River direction:");
                    ImGui::SameLine((float)pos6);
                    ImGui::Text(flowdirtext.c_str());
                }

                ImGui::SameLine((float)pos7);
                ImGui::Text("Charts:");

                // Line 2

                ImGui::Text("Latitude:");
                ImGui::SameLine((float)pos2);
                ImGui::Text(lattext.c_str());

                ImGui::SameLine((float)pos3);
                ImGui::Text("Prevailing wind:");
                ImGui::SameLine((float)pos4);
                ImGui::Text(windtext.c_str());

                if (river)
                {
                    ImGui::SameLine((float)pos5);
                    ImGui::Text("January flow:");
                    ImGui::SameLine((float)pos6);
                    ImGui::Text(janflowtext.c_str());
                }

                ImGui::SameLine((float)pos7 + 20);
                if (ImGui::Button("Temperature", ImVec2(120.0f, 0.0f)))
                    toggle(showregionaltemperaturechart);

                // Line 3

                ImGui::Text(" ");

                if (river)
                {
                    ImGui::SameLine((float)pos5);
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 4.0f); // Because the buttons to the right move this line down a little
                    ImGui::Text("July flow:");
                    ImGui::SameLine((float)pos6);
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 4.0f); // Because the buttons to the right move this line down a little
                    ImGui::Text(julflowtext.c_str());
                }

                ImGui::SameLine((float)pos7 + 20);
                if (ImGui::Button("Precipitation", ImVec2(120.0f, 0.0f)))
                    toggle(showregionalrainfallchart);

                // Line 4

                if (sea)
                {
                    ImGui::Text("Sea ice:");
                    ImGui::SameLine((float)pos2);
                    ImGui::Text(seaicetext.c_str());
                }
                else
                {
                    ImGui::Text("Climate:");
                    ImGui::SameLine((float)pos2);
                    ImGui::Text(climatetext.c_str());
                }

                // Line 5

                ImGui::Text(" ");

                ImGui::SameLine((float)pos2);

                if (specialstext != "")
                    ImGui::Text(specialstext.c_str());
                else
                    ImGui::Text(glacialtext.c_str());
            }

            ImGui::SetCursorPosX(993.0);
            ImGui::SetCursorPosY(122.0f);

            if (ImGui::Button("?"))
                toggle(showabout);

            ImGui::End();

            if (window.hasFocus())
            {
                // Now check to see if the map has been clicked on.
                
                if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && io.WantCaptureMouse == 0)
                {
                    sf::Vector2i mousepos = sf::Mouse::getPosition(window);

                    poix = mousepos.x - (int)regionalmapxpos;
                    poiy = mousepos.y - (int)regionalmapypos;

                    if (poix >= 0 && poix < regionalmapimagewidth && poiy >= 0 && poiy < regionalmapimageheight)
                    {
                        poix = poix + region->regwidthbegin();
                        poiy = poiy + region->regheightbegin();

                        focused = 1;

                        highlight->setPosition(sf::Vector2f((float)mousepos.x, (float)mousepos.y));
                    }
                    else
                    {
                        if (focused == 1)
                        {
                            focused = 0;
                            poix = -1;
                            poiy = -1;
                        }
                    }

                    float mult = ((float)world->width() + 1.0f) / (float)DISPLAYMAPSIZEX;

                    float fpoix = (float)(mousepos.x - minimapxpos) * mult * 2.0f;
                    float fpoiy = (float)(mousepos.y - minimapypos) * mult * 2.0f;

                    int minipoix = (int)fpoix;
                    int minipoiy = (int)fpoiy;

                    if (minipoix >= 0 && minipoiy <= world->width() && minipoiy >= 0 && minipoiy <= world->height())// If the minimap has been clicked on.
                    {
                        screenmode = generatingregionscreen;
                        newx = minipoix;
                        newy = minipoiy;
                    }
                }

                // Check to see if the cursor keys have been pressed.

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
                {
                    int regionalmapmove = REGIONALTILEWIDTH;

                    newx = region->centrex() - regionalmapmove;

                    if (newx > 0)
                    {
                        newx = newx / 32;
                        newx = newx * 32;
                        newx = newx + 16;
                    }

                    if (newx < regionalmapmove * 2)
                        newx = wrap(newx, world->width());

                    newy = region->centrey();

                    screenmode = generatingregionscreen;
                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
                {
                    int regionalmapmove = REGIONALTILEWIDTH;

                    newx = region->centrex() + regionalmapmove;

                    newx = newx / 32;
                    newx = newx * 32;
                    newx = newx + 16;

                    if (newx > world->width())
                        newx = wrap(newx, world->width());

                    newy = region->centrey();

                    screenmode = generatingregionscreen;
                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
                {
                    int regionalmapmove = REGIONALTILEHEIGHT;

                    newy = region->centrey() - regionalmapmove;

                    newy = newy / 32;
                    newy = newy * 32;
                    newy = newy + 16;

                    if (newy > regionalmapmove)
                    {
                        newx = region->centrex();
                        screenmode = generatingregionscreen;
                    }
                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
                {
                    int regionalmapmove = REGIONALTILEHEIGHT;

                    newy = region->centrey() + regionalmapmove;

                    newy = newy / 32;
                    newy = newy * 32;
                    newy = newy + 16;

                    if (newy < world->height())
                    {
                        newx = region->centrex();
                        screenmode = generatingregionscreen;
                    }
                }
            }
        }

        // Area export screen (this is where the user selects the area to export)

        if (screenmode == exportareascreen)
        {
            showglobaltemperaturechart = 0;
            showglobalrainfallchart = 0;
            showregionaltemperaturechart = 0;
            showregionalrainfallchart = 0;
            showworldproperties = 0;
            showworldeditproperties = 0;
            showcolouroptions = 0;
            newworld = 0;
            
            showsetsize = 0;

            // Work out the size of the currently selected area.

            int totalregions = 0;
            int maxtotalregions = 100; // Query area maps larger than this.
            
            if (areanex != -1)
            {
                float regiontilewidth = (float)REGIONALTILEWIDTH; //30;
                float regiontileheight = (float)REGIONALTILEHEIGHT; //30; // The width and height of the visible regional map, in tiles.

                int regionwidth = (int)regiontilewidth * 16;
                int regionheight = (int)regiontileheight * 16; // The width and height of the visible regional map, in pixels.

                int newareanwx = areanwx; // This is because the regions we'll be making will start to the north and west of the defined area.
                int newareanwy = areanwy;

                int newareanex = areanex;
                int newareaney = areaney;

                int newareaswx = areaswx;
                int newareaswy = areaswy;

                int newareasey = areasey;

                newareanwx = newareanwx / (int)regiontilewidth;
                newareanwx = newareanwx * (int)regiontilewidth;

                newareanwy = newareanwy / (int)regiontileheight;
                newareanwy = newareanwy * (int)regiontileheight;

                newareaswx = newareanwx;
                newareaney = newareanwy;

                float areatilewidth = (float)(newareanex - newareanwx);
                float areatileheight = (float)(newareasey - newareaney);

                int areawidth = (int)(areatilewidth * 16.0f);
                int areaheight = (int)(areatileheight * 16.0f);

                float fregionswide = areatilewidth / regiontilewidth;
                float fregionshigh = areatileheight / regiontileheight;

                int regionswide = (int)fregionswide;
                int regionshigh = (int)fregionshigh;

                if (regionswide != fregionswide)
                    regionswide++;

                if (regionshigh != fregionshigh)
                    regionshigh++;

                totalregions = regionswide * regionshigh; // This is how many regional maps we're going to have to do.
            }

            // Main controls.

            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 10, main_viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(161, 138), ImGuiCond_FirstUseEver);

            ImGui::Begin("Export custom area", NULL, window_flags);

            ImGui::PushItemWidth(100.0f);

            if (standardbutton("Clear selection"))
            {
                areanex = -1;
                areaney = -1;
                areasex = -1;
                areasey = -1;
                areaswx = -1;
                areaswy = -1;
                areanwx = -1;
                areanwy = -1;
            }

            if (standardbutton("Export maps"))
            {
                if (areanex != -1)
                {
                    if (totalregions <= maxtotalregions)
                    {
                        ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".png", ".");

                        exportingareamaps = 1;
                    }
                    else
                        showareawarning = 1;
                }
            }

            ImGui::Text("  ");

            if (standardbutton("Cancel"))
            {
                areanex = -1;
                areaney = -1;
                areasex = -1;
                areasey = -1;
                areaswx = -1;
                areaswy = -1;
                areanwx = -1;
                areanwy = -1;

                if (areafromregional == 1)
                    screenmode = regionalmapscreen;
                else
                    screenmode = globalmapscreen;
            }

            ImGui::End();

            // Now the text box.

            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 180, main_viewport->WorkPos.y + 542), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(1023, 151), ImGuiCond_FirstUseEver);
            string title = "            ";
            string areatext = "Total regions: " + to_string(totalregions); // 117 OK; 143 not OK; 132 not OK; 121 not OK; 112 OK; 116 OK; 110

            ImGui::Begin(title.c_str(), NULL, window_flags);
            ImGui::PushItemWidth((float)world->width() / 2.0f);
            areatext="This screen allows you to export maps at the same scale as the regional map, but of larger areas.\n\nClick on the map to pick the corners of the area you want to export. You can re-select corners to fine-tune the area.\n\nWhen you are done, click on 'export maps'. The program will ask you to specify the filename under which to save the maps, and then create them.";
            
            ImGui::Text(areatext.c_str(), world->width() / 2);
            ImGui::End();

            // Now check to see if the map has been clicked on.

            if (window.hasFocus() && sf::Mouse::isButtonPressed(sf::Mouse::Left) && io.WantCaptureMouse == 0)
            {
                sf::Vector2i mousepos = sf::Mouse::getPosition(window);

                float mult = ((float)world->width() + 1.0f) / (float)DISPLAYMAPSIZEX;

                float xpos = ((float)mousepos.x - (float)globalmapxpos) * mult;
                float ypos = ((float)mousepos.y - (float)globalmapypos) * mult;

                poix = (int)xpos;
                poiy = (int)ypos;

                if (poix >= 0 && poix <= world->width() && poiy >= 0 && poiy <= world->height())
                {
                    if (areaswx == -1) // If we don't have any corners yet
                    {
                        areanex = poix + 2;
                        areaney = poiy;
                        areasex = poix + 2;
                        areasey = poiy + 2;
                        areaswx = poix;
                        areaswy = poiy + 2;
                        areanwx = poix;
                        areanwy = poiy;
                    }
                    else // If we do have all corners
                    {
                        int nedistx = areanex - poix;
                        int nedisty = areaney - poiy;

                        int sedistx = areasex - poix;
                        int sedisty = areasey - poiy;

                        int swdistx = areaswx - poix;
                        int swdisty = areaswy - poiy;

                        int nwdistx = areanwx - poix;
                        int nwdisty = areanwy - poiy;

                        float nedist = (float)sqrt(nedistx * nedistx + nedisty * nedisty);
                        float sedist = (float)sqrt(sedistx * sedistx + sedisty * sedisty);
                        float swdist = (float)sqrt(swdistx * swdistx + swdisty * swdisty);
                        float nwdist = (float)sqrt(nwdistx * nwdistx + nwdisty * nwdisty);

                        short id = 2; // This will identify which corner this is going to be.
                        float mindist = nedist;

                        if (sedist < mindist)
                        {
                            id = 4;
                            mindist = sedist;
                        }

                        if (swdist < mindist)
                        {
                            id = 6;
                            mindist = swdist;
                        }

                        if (nwdist < mindist)
                            id = 8;

                        if (id == 2)
                        {
                            areanex = poix;
                            areaney = poiy;
                            areasex = poix;
                            areanwy = poiy;
                        }

                        if (id == 4)
                        {
                            areasex = poix;
                            areasey = poiy;
                            areanex = poix;
                            areaswy = poiy;
                        }

                        if (id == 6)
                        {
                            areaswx = poix;
                            areaswy = poiy;
                            areanwx = poix;
                            areasey = poiy;
                        }

                        if (id == 8)
                        {
                            areanwx = poix;
                            areanwy = poiy;
                            areaswx = poix;
                            areaney = poiy;
                        }

                        if (areanwx > areanex)
                            swap(areanwx, areanex);

                        if (areaswx > areasex)
                            swap(areaswx, areasex);

                        if (areanwy > areaswy)
                            swap(areanwy, areaswy);

                        if (areaney > areasey)
                            swap(areaney, areasey);
                    }
                }
            }
        }

        // Custom world screen

        if (screenmode == importscreen)
        {
            showglobaltemperaturechart = 0;
            showglobalrainfallchart = 0;
            showregionaltemperaturechart = 0;
            showregionalrainfallchart = 0;
            showworldproperties = 0;
            
            showcolouroptions = 0;
            showworldproperties = 0;
            
            // Main controls.

            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 10, main_viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(161, 604), ImGuiCond_FirstUseEver);

            ImGui::Begin("Custom##custom", NULL, window_flags);

            ImGui::SetNextItemWidth(0);

            ImGui::Text("Import:");

            ImGui::PushItemWidth(100.0f);

            if (standardbutton("Land map"))
            {
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".png", ".");

                importinglandmap = 1;
            }

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Red value of 0 is sea. Higher values are elevation above sea level, in multiples of 10 metres. Blue and green values are ignored.");

            if (standardbutton("Sea map"))
            {
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".png", ".");

                importingseamap = 1;
            }

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Red value of 0 is land. Higher values are depths below sea level, in multiples of 50 metres. Blue and green values are ignored.");

            if (standardbutton("Mountains"))
            {
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".png", ".");

                importingmountainsmap = 1;
            }

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Red value is peak elevation above surrounding land, in multiples of 50 metres. Blue and green values are ignored.");

            if (standardbutton("Volcanoes"))
            {
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".png", ".");

                importingvolcanoesmap = 1;
            }

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("As for mountains. Green=0 for shield volcano, or higher for stratovolcano. Blue=0 for extinct, or higher for active.");

            ImGui::Dummy(ImVec2(0.0f, linespace));

            ImGui::SetNextItemWidth(0);

            ImGui::Text("Generate terrain:");

            ImGui::PushItemWidth(100.0f);

            if (standardbutton("Tectonic"))
                toggle(showtectonicchooser);

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Create tectonic-based global terrain, with continents, mountains, coastal shelves, and oceanic ridges.");

            if (standardbutton("Non-tectonic"))
                toggle(shownontectonicchooser);

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Create non-tectonic-based global terrain.");

            ImGui::Dummy(ImVec2(0.0f, linespace));

            ImGui::SetNextItemWidth(0);

            ImGui::Text("Generate elements:");

            ImGui::PushItemWidth(100.0f);

            if (standardbutton("Coastlines"))
            {
                removestraights(*world);

                // Now redraw the map.

                for (int n = 0; n < GLOBALMAPTYPES; n++)
                    globalmapimagecreated[n] = 0;

                mapview = relief;
                drawglobalmapimage(mapview, *world, globalmapimagecreated, *globalelevationimage, *globaltemperatureimage, *globalprecipitationimage, *globalclimateimage, *globalriversimage, *globalreliefimage, *displayglobalelevationimage, *displayglobaltemperatureimage, *displayglobalprecipitationimage, *displayglobalclimateimage, *displayglobalriversimage, *displayglobalreliefimage);

                globalmaptexture->loadFromImage(*displayglobalreliefimage);
                globalmap->setTexture(*globalmaptexture);
            }

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Disrupt straight edges on coastlines.");

            if (standardbutton("Shelves"))
            {
                int width = world->width();
                int height = world->height();
                int maxelev = world->maxelevation();
                int sealevel = world->sealevel();

                vector<vector<bool>> shelves(ARRAYWIDTH, vector<bool>(ARRAYHEIGHT, 0));

                makecontinentalshelves(*world, shelves, 4);
                createoceantrenches(*world, shelves);

                int grain = 8; // Level of detail on this fractal map.
                float valuemod = 0.2f;
                int v = random(3, 6);
                float valuemod2 = (float)v;

                vector<vector<int>> seafractal(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

                createfractal(seafractal, width, height, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

                float coastalvarreduce = (float)maxelev / 3000.0f;
                float oceanvarreduce = (float)maxelev / 100.0f;

                for (int i = 0; i <= width; i++)
                {
                    for (int j = 0; j <= height; j++)
                    {
                        if (world->sea(i, j) == 1 && shelves[i][j] == 1)
                            world->setnom(i, j, sealevel - 100);
                    }
                }

                // Now redraw the map.

                for (int n = 0; n < GLOBALMAPTYPES; n++)
                    globalmapimagecreated[n] = 0;

                mapview = relief;
                drawglobalmapimage(mapview, *world, globalmapimagecreated, *globalelevationimage, *globaltemperatureimage, *globalprecipitationimage, *globalclimateimage, *globalriversimage, *globalreliefimage, *displayglobalelevationimage, *displayglobaltemperatureimage, *displayglobalprecipitationimage, *displayglobalclimateimage, *displayglobalriversimage, *displayglobalreliefimage);

                globalmaptexture->loadFromImage(*displayglobalreliefimage);
                globalmap->setTexture(*globalmaptexture);
            }

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Generate continental shelves.");

            if (standardbutton("Oceanic ridges"))
            {
                int width = world->width();
                int height = world->height();
                int maxelev = world->maxelevation();
                int sealevel = world->sealevel();

                vector<vector<bool>> shelves(ARRAYWIDTH, vector<bool>(ARRAYHEIGHT, 0));

                for (int i = 0; i <= width; i++)
                {
                    for (int j = 0; j <= height; j++)
                    {
                        if (world->nom(i, j) > sealevel - 400)
                            shelves[i][j] = 1;
                    }
                }

                createoceanridges(*world, shelves);

                // Now redraw the map.

                for (int n = 0; n < GLOBALMAPTYPES; n++)
                    globalmapimagecreated[n] = 0;

                mapview = relief;
                drawglobalmapimage(mapview, *world, globalmapimagecreated, *globalelevationimage, *globaltemperatureimage, *globalprecipitationimage, *globalclimateimage, *globalriversimage, *globalreliefimage, *displayglobalelevationimage, *displayglobaltemperatureimage, *displayglobalprecipitationimage, *displayglobalclimateimage, *displayglobalriversimage, *displayglobalreliefimage);

                globalmaptexture->loadFromImage(*displayglobalreliefimage);
                globalmap->setTexture(*globalmaptexture);
            }

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Generate mid-ocean ridges.");

            if (standardbutton("Sea bed"))
            {
                int width = world->width();
                int height = world->height();
                int maxelev = world->maxelevation();
                int sealevel = world->sealevel();

                int grain = 8; // Level of detail on this fractal map.
                float valuemod = 0.2f;
                int v = random(3, 6);
                float valuemod2 = (float)v;

                vector<vector<int>> seafractal(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

                createfractal(seafractal, width, height, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

                int warpfactor = random(20, 80);
                warp(seafractal, width, height, maxelev, warpfactor, 1);

                float coastalvarreduce = (float)maxelev / 3000.0f;
                float oceanvarreduce = (float)maxelev / 1000.0f;

                for (int i = 0; i <= width; i++)
                {
                    for (int j = 0; j <= height; j++)
                    {
                        if (world->sea(i, j) == 1)
                        {
                            float var = (float)(seafractal[i][j] - maxelev / 2);
                            var = var / coastalvarreduce;
                            
                            int newval = world->nom(i, j) + (int)var;

                            if (newval > sealevel - 10)
                                newval = sealevel - 10;

                            if (newval < 1)
                                newval = 1;

                            world->setnom(i, j, newval);
                        }
                    }
                }

                // Smooth the seabed.

                smoothonlysea(*world, 2);

                // Now redraw the map.

                for (int n = 0; n < GLOBALMAPTYPES; n++)
                    globalmapimagecreated[n] = 0;

                mapview = relief;
                drawglobalmapimage(mapview, *world, globalmapimagecreated, *globalelevationimage, *globaltemperatureimage, *globalprecipitationimage, *globalclimateimage, *globalriversimage, *globalreliefimage, *displayglobalelevationimage, *displayglobaltemperatureimage, *displayglobalprecipitationimage, *displayglobalclimateimage, *displayglobalriversimage, *displayglobalreliefimage);

                globalmaptexture->loadFromImage(*displayglobalreliefimage);
                globalmap->setTexture(*globalmaptexture);
            }

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Create random depth variation across the oceans.");

            if (standardbutton("Land elevation"))
            {
                int width = world->width();
                int height = world->height();
                int maxelev = world->maxelevation();
                int sealevel = world->sealevel();

                // First, make a fractal map.

                int grain = 8; // Level of detail on this fractal map.
                float valuemod = 0.2f;
                int v = random(3, 6);
                float valuemod2 = (float)v;

                vector<vector<int>> fractal(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

                createfractal(fractal, width, height, grain, valuemod, valuemod2, 1, 12750, 0, 0);

                int warpfactor = random(20, 80);
                warp(fractal, width, height, maxelev, warpfactor, 1);

                int fractaladd = sealevel - 2500;

                for (int i = 0; i <= width; i++)
                {
                    for (int j = 0; j <= height; j++)
                        fractal[i][j] = fractal[i][j] + fractaladd;
                }

                // Now use it to change the land heights.

                fractaladdland(*world, fractal);

                // Smooth the land.

                //smoothonlyland(*world, 2);

                // Also, create extra elevation.

                createextraelev(*world);

                // Now redraw the map.

                for (int n = 0; n < GLOBALMAPTYPES; n++)
                    globalmapimagecreated[n] = 0;

                mapview = relief;
                drawglobalmapimage(mapview, *world, globalmapimagecreated, *globalelevationimage, *globaltemperatureimage, *globalprecipitationimage, *globalclimateimage, *globalriversimage, *globalreliefimage, *displayglobalelevationimage, *displayglobaltemperatureimage, *displayglobalprecipitationimage, *displayglobalclimateimage, *displayglobalriversimage, *displayglobalreliefimage);

                globalmaptexture->loadFromImage(*displayglobalreliefimage);
                globalmap->setTexture(*globalmaptexture);
            }

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Generate random elevation variation across the land.");

            if (standardbutton("Mountains##2"))
            {
                int width = world->width();
                int height = world->height();
                int maxelev = world->maxelevation();
                int sealevel = world->sealevel();

                int baseheight = sealevel - 4500;
                if (baseheight < 1)
                    baseheight = 1;
                int conheight = sealevel + 50;

                vector<vector<int>> plateaumap(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

                // First, make a fractal map.

                int grain = 8; // Level of detail on this fractal map.
                float valuemod = 0.2f;
                int v = random(3, 6);
                float valuemod2 = (float)v;

                vector<vector<int>> fractal(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

                createfractal(fractal, width, height, grain, valuemod, valuemod2, 1, 12750, 0, 0);

                int fractaladd = sealevel - 2500;

                for (int i = 0; i <= width; i++)
                {
                    for (int j = 0; j <= height; j++)
                        fractal[i][j] = fractal[i][j] + fractaladd;
                }

                twointegers dummy[1];

                createchains(*world, baseheight, conheight, fractal, plateaumap, landshape, chainland, dummy, 0, 0, 5);

                // Now redraw the map.

                for (int n = 0; n < GLOBALMAPTYPES; n++)
                    globalmapimagecreated[n] = 0;

                mapview = relief;
                drawglobalmapimage(mapview, *world, globalmapimagecreated, *globalelevationimage, *globaltemperatureimage, *globalprecipitationimage, *globalclimateimage, *globalriversimage, *globalreliefimage, *displayglobalelevationimage, *displayglobaltemperatureimage, *displayglobalprecipitationimage, *displayglobalclimateimage, *displayglobalriversimage, *displayglobalreliefimage);

                globalmaptexture->loadFromImage(*displayglobalreliefimage);
                globalmap->setTexture(*globalmaptexture);
            }

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Generate random mountain ranges.");

            if (standardbutton("Hills"))
            {
                int width = world->width();
                int height = world->height();
                int maxelev = world->maxelevation();
                int sealevel = world->sealevel();

                int baseheight = sealevel - 4500;
                if (baseheight < 1)
                    baseheight = 1;
                int conheight = sealevel + 50;

                vector<vector<int>> plateaumap(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

                // First, make a fractal map.

                int grain = 8; // Level of detail on this fractal map.
                float valuemod = 0.2f;
                int v = random(3, 6);
                float valuemod2 = (float)v;

                vector<vector<int>> fractal(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

                createfractal(fractal, width, height, grain, valuemod, valuemod2, 1, 12750, 0, 0);

                int fractaladd = sealevel - 2500;

                for (int i = 0; i <= width; i++)
                {
                    for (int j = 0; j <= height; j++)
                        fractal[i][j] = fractal[i][j] + fractaladd;
                }

                twointegers dummy[1];

                createchains(*world, baseheight, conheight, fractal, plateaumap, landshape, chainland, dummy, 0, 0, 5);

                // Now redraw the map.

                for (int n = 0; n < GLOBALMAPTYPES; n++)
                    globalmapimagecreated[n] = 0;

                mapview = relief;
                drawglobalmapimage(mapview, *world, globalmapimagecreated, *globalelevationimage, *globaltemperatureimage, *globalprecipitationimage, *globalclimateimage, *globalriversimage, *globalreliefimage, *displayglobalelevationimage, *displayglobaltemperatureimage, *displayglobalprecipitationimage, *displayglobalclimateimage, *displayglobalriversimage, *displayglobalreliefimage);

                globalmaptexture->loadFromImage(*displayglobalreliefimage);
                globalmap->setTexture(*globalmaptexture);
            }

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Generate random ranges of hills.");

            if (standardbutton("Craters"))
            {
                int width = world->width();
                int height = world->height();
                int maxelev = world->maxelevation();
                int sealevel = world->sealevel();

                vector<vector<int>> oldterrain(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0)); // Copy the terrain as it is before adding craters. This is so that we can add some variation to the craters afterwards, if there's sea on this world, so the depression filling doesn't fill them completely.

                for (int i = 0; i <= width; i++)
                {
                    for (int j = 0; j <= height; j++)
                        oldterrain[i][j] = world->nom(i, j);
                }

                int cratertotal = random(500, 10000);

                createcratermap(*world, cratertotal, squareroot, 1);

                float minseaproportion = (float)random(1, 500);
                minseaproportion = minseaproportion / 1000.0f;

                int minseasize = (int)(((float)width * (float)height) * minseaproportion);

                if (random(1, 4) != 1) // That may have produced seas inside craters, so probably remove those now.
                    removesmallseas(*world, minseasize, sealevel + 1);
                else // Definitely remove any really little bits of sea.
                    removesmallseas(*world, 20, sealevel + 1);

                int totalsea = 0;

                for (int i = 0; i <= -width; i++)
                {
                    for (int j = 0; j <= height; j++)
                    {
                        if (world->sea(i, j))
                            totalsea++;
                    }
                }

                if (totalsea > 40) // If there's sea, add some variation to how prominent the craters are. This will create some gaps in the craters so they don't get entirely filled up by the depression filling.
                {
                    vector<vector<int>> fractal(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

                    int grain = 8; // Level of detail on this fractal map.
                    float valuemod = 0.2f;
                    int v = random(3, 6);
                    float valuemod2 = (float)v;

                    createfractal(fractal, width, height, grain, valuemod, valuemod2, 1, maxelev, 1, 0);

                    for (int i = 0; i <= width; i++)
                    {
                        for (int j = 0; j <= height; j++)
                        {
                            float oldmult = (float)fractal[i][j] / (float)maxelev;
                            float newmult = 1.0f - oldmult;

                            float thiselev = (float)oldterrain[i][j] * oldmult + (float)world->nom(i, j) * newmult;

                            world->setnom(i, j, (int)thiselev);
                        }
                    }
                }

                // Now redraw the map.

                for (int n = 0; n < GLOBALMAPTYPES; n++)
                    globalmapimagecreated[n] = 0;

                mapview = relief;
                drawglobalmapimage(mapview, *world, globalmapimagecreated, *globalelevationimage, *globaltemperatureimage, *globalprecipitationimage, *globalclimateimage, *globalriversimage, *globalreliefimage, *displayglobalelevationimage, *displayglobaltemperatureimage, *displayglobalprecipitationimage, *displayglobalclimateimage, *displayglobalriversimage, *displayglobalreliefimage);

                globalmaptexture->loadFromImage(*displayglobalreliefimage);
                globalmap->setTexture(*globalmaptexture);
            }

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Generate random craters.");

            ImGui::Dummy(ImVec2(0.0f, linespace));

            ImGui::SetNextItemWidth(0);

            ImGui::Text("Other controls:");

            ImGui::PushItemWidth(100.0f);

            if (standardbutton("Properties"))
                toggle(showworldeditproperties);

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Adjust the world's properties.");

            if (standardbutton("Finish"))
            {
                screenmode = completingimportscreen;
            }

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Calculate climates, lakes, and rivers, and finish the world.");

            if (standardbutton("Cancel"))
            {
                brandnew = 1;
                seedentry = 0;

                screenmode = createworldscreen;
            }

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Return to the world creation screen.");

            ImGui::End();

            // Now the text box.

            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 180, main_viewport->WorkPos.y + 542), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(1023, 155), ImGuiCond_FirstUseEver);

            string title = "            ";

            string importtext = "You can use the 'import' buttons to load in your own maps. These must be " + formatnumber(world->width() + 1) + " x " + formatnumber(world->height() + 1) + " pixels, in .png format.\nAlternatively, you can use the 'World terrain' button to generate a map from scratch.\nAfter you have imported or generated the map, you can use the other 'generate' buttons to tweak it or to add extra features.\nYou can use the 'Properties' panel to change settings such as global temperatures or rainfall.\nWhen you are done, click 'Finish' to finish the world.";

            ImGui::Begin(title.c_str(), NULL, window_flags);
            ImGui::PushItemWidth((float)(world->width() / 2));
            ImGui::Text(importtext.c_str(), world->width() / 2);
            ImGui::End();
        }

        // These screens all display a "Please wait" message ten times (for some reason doing it once or twice doesn't actually display it) and then do something time-consuming.

        if (screenmode == generatingtectonicscreen)
        {
            if (generatingtectonicpass < 10)
            {
                ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 507, main_viewport->WorkPos.y + 173), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize(ImVec2(173, 68), ImGuiCond_FirstUseEver);
                ImGui::Begin("Please wait...##generatetectonic ", NULL, window_flags);
                ImGui::Text("Generating terrain...");
                ImGui::End();

                generatingtectonicpass++;
            }
            else
            {
                initialiseworld(*world);
                world->clear();

                world->setsize(currentsize);

                adjustforsize(*world, globaltexturesize, *globalelevationimage, *globaltemperatureimage, *globalprecipitationimage, *globalclimateimage, *globalriversimage, *globalreliefimage, *highlightimage, highlightsize, *minihighlightimage, minihighlightsize);

                minihighlighttexture->loadFromImage(*minihighlightimage);
                minihighlight->setTexture(*minihighlighttexture);
                minihighlight->setOrigin((float)minihighlightsize / 2.0f, (float)minihighlightsize / 2.0f);

                int seed = random(0, 9);

                for (int n = 1; n <= 7; n++)
                {
                    if (n == 7)
                        seed = seed + (random(1, 9) * (int)pow(10, n));
                    else
                        seed = seed + (random(0, 9) * (int)pow(10, n));
                }

                seed = 0 - seed;

                world->setseed(seed);

                updatereport("Generating custom world terrain from seed: " + to_string(world->seed()) + ":");
                updatereport("");

                for (int n = 0; n < GLOBALMAPTYPES; n++) // Set all map types as unviewed, to force them to be redrawn when called up
                    globalmapimagecreated[n] = 0;

                world->settype(2); // This terrain type gives large continents.

                fast_srand(world->seed());

                int clusterno = -1;
                int clustersize = -1;

                switch (landmass)
                {
                case 0:
                    world->settype(3);
                    break;

                case 1:
                    world->settype(1);
                    break;

                case 2:
                    clusterno = 1;
                    clustersize = 1;
                    break;

                case 3:
                    clusterno = 1;
                    clustersize = 6;
                    break;

                case 4:
                    clusterno = 2;
                    clustersize = 3;
                    break;

                case 5:
                    clusterno = 2;
                    clustersize = 7;
                    break;

                case 6:
                    clusterno = 3;
                    clustersize = 3;
                    break;

                case 7:
                    clusterno = 3;
                    clustersize = 8;
                    break;

                case 8:
                    clusterno = 3;
                    clustersize = 9;
                    break;

                case 9:
                    clusterno = 4;
                    clustersize = 5;
                    break;

                case 10:
                    clusterno = 4;
                    clustersize = 9;
                    break;
                }

                vector<vector<int>> mountaindrainage(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));
                vector<vector<bool>> shelves(ARRAYWIDTH, vector<bool>(ARRAYHEIGHT, 0));

                // Now generate the terrain.

                generateglobalterrain(*world, 1, iterations, mergefactor - 5, clusterno, clustersize, landshape, chainland, mountaindrainage, shelves, squareroot);

                // Now redraw the map.

                for (int n = 0; n < GLOBALMAPTYPES; n++)
                    globalmapimagecreated[n] = 0;

                mapview = relief;
                drawglobalmapimage(mapview, *world, globalmapimagecreated, *globalelevationimage, *globaltemperatureimage, *globalprecipitationimage, *globalclimateimage, *globalriversimage, *globalreliefimage, *displayglobalelevationimage, *displayglobaltemperatureimage, *displayglobalprecipitationimage, *displayglobalclimateimage, *displayglobalriversimage, *displayglobalreliefimage);

                globalmaptexture->loadFromImage(*displayglobalreliefimage);
                globalmap->setTexture(*globalmaptexture);

                generatingtectonicpass = 0;
                screenmode = importscreen;
            }
        }

        if (screenmode == generatingnontectonicscreen)
        {
            if (generatingnontectonicpass < 10)
            {
                ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 507, main_viewport->WorkPos.y + 173), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize(ImVec2(173, 68), ImGuiCond_FirstUseEver);
                ImGui::Begin("Please wait...##generatenontectonic ", NULL, window_flags);
                ImGui::Text("Generating terrain...");
                ImGui::End();

                generatingnontectonicpass++;
            }
            else
            {
                initialiseworld(*world);
                world->clear();

                world->setsize(currentsize);

                adjustforsize(*world, globaltexturesize, *globalelevationimage, *globaltemperatureimage, *globalprecipitationimage, *globalclimateimage, *globalriversimage, *globalreliefimage, *highlightimage, highlightsize, *minihighlightimage, minihighlightsize);

                minihighlighttexture->loadFromImage(*minihighlightimage);
                minihighlight->setTexture(*minihighlighttexture);
                minihighlight->setOrigin((float)minihighlightsize / 2.0f, (float)minihighlightsize / 2.0f);

                int seed = random(0, 9);

                for (int n = 1; n <= 7; n++)
                {
                    if (n == 7)
                        seed = seed + (random(1, 9) * (int)pow(10, n));
                    else
                        seed = seed + (random(0, 9) * (int)pow(10, n));
                }

                seed = 0 - seed;

                world->setseed(seed);

                updatereport("Generating custom world terrain from seed: " + to_string(world->seed()) + ":");
                updatereport("");

                for (int n = 0; n < GLOBALMAPTYPES; n++) // Set all map types as unviewed, to force them to be redrawn when called up
                    globalmapimagecreated[n] = 0;

                world->settype(4); // This terrain type gives alien-type terrain.

                fast_srand(world->seed());

                float sealevel = (float)sealeveleditable / 10.0f;

                sealevel = (float)world->maxelevation() * sealevel;

                if (sealevel < 1.0f)
                    sealevel = 1.0f;

                world->setsealevel((int)sealevel);

                vector<vector<int>> mountaindrainage(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));
                vector<vector<bool>> shelves(ARRAYWIDTH, vector<bool>(ARRAYHEIGHT, 0));

                // Now generate the terrain.

                generateglobalterrain(*world, 1, iterations, mergefactor - 5, 1, 1, landshape, chainland, mountaindrainage, shelves, squareroot);

                // Now redraw the map.

                for (int n = 0; n < GLOBALMAPTYPES; n++)
                    globalmapimagecreated[n] = 0;

                mapview = relief;
                drawglobalmapimage(mapview, *world, globalmapimagecreated, *globalelevationimage, *globaltemperatureimage, *globalprecipitationimage, *globalclimateimage, *globalriversimage, *globalreliefimage, *displayglobalelevationimage, *displayglobaltemperatureimage, *displayglobalprecipitationimage, *displayglobalclimateimage, *displayglobalriversimage, *displayglobalreliefimage);

                globalmaptexture->loadFromImage(*displayglobalreliefimage);
                globalmap->setTexture(*globalmaptexture);

                generatingtectonicpass = 0;
                screenmode = importscreen;
            }
        }

        if (screenmode == completingimportscreen)
        {
            if (completingimportpass < 10)
            {
                ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 507, main_viewport->WorkPos.y + 173), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize(ImVec2(173, 68), ImGuiCond_FirstUseEver);
                ImGui::Begin("Please wait...##completeimport ");
                ImGui::Text("Finishing world...");
                ImGui::End();

                completingimportpass++;
            }
            else
            {
                updatereport("Generating custom world:");
                updatereport("");

                // Plug in the world settings.

                world->setgravity(currentgravity);
                world->setlunar(currentlunar);
                world->seteccentricity(currenteccentricity);
                world->setperihelion(currentperihelion);
                world->setrotation((bool)currentrotation);
                world->settilt(currenttilt);
                world->settempdecrease(currenttempdecrease);
                world->setnorthpolaradjust(currentnorthpolaradjust);
                world->setsouthpolaradjust(currentsouthpolaradjust);
                world->setaveragetemp(currentaveragetemp);
                world->setwaterpickup(currentwaterpickup);
                world->setglacialtemp(currentglacialtemp);

                // First, finish off the terrain generation.

                vector<vector<int>> mountaindrainage(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));
                vector<vector<bool>> shelves(ARRAYWIDTH, vector<bool>(ARRAYHEIGHT, 0));

                world->setmaxelevation(100000);

                updatereport("Raising mountain bases");

                raisemountainbases(*world, mountaindrainage, OKmountains);

                getlandandseatotals(*world);

                bool seapresent = 0;

                if (world->seatotal() > 10)
                    seapresent = 1;

                if (seapresent)
                {
                    updatereport("Filling depressions");

                    depressionfill(*world);

                    addlandnoise(*world); // Add a bit of noise, then do remove depressions again. This is to add variety to the river courses.

                    depressionfill(*world);

                    updatereport("Adjusting coastlines");

                    for (int n = 1; n <= 2; n++)
                        normalisecoasts(*world, 13, 11, 4);

                    clamp(*world);

                    updatereport("Checking islands");

                    checkislands(*world);
                }

                updatereport("Creating roughness map");

                vector<vector<int>> roughness(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

                int grain = 8; // Level of detail on this fractal map.
                float valuemod = 0.2f;
                float valuemod2 = 0.6f;

                createfractal(roughness, width, height, grain, valuemod, valuemod2, 1, world->maxelevation(), 0, 0);

                for (int i = 0; i <= width; i++)
                {
                    for (int j = 0; j <= height; j++)
                        world->setroughness(i, j, (float)roughness[i][j]);
                }

                // Now do the climates.

                generateglobalclimate(*world, currentrivers, currentlakes, currentdeltas, smalllake, largelake, landshape, mountaindrainage, shelves);

                // Now draw a new map

                for (int n = 0; n < GLOBALMAPTYPES; n++) // Set all map types as unviewed, to force them to be redrawn when called up
                    globalmapimagecreated[n] = 0;

                mapview = relief;

                drawglobalmapimage(mapview, *world, globalmapimagecreated, *globalelevationimage, *globaltemperatureimage, *globalprecipitationimage, *globalclimateimage, *globalriversimage, *globalreliefimage, *displayglobalelevationimage, *displayglobaltemperatureimage, *displayglobalprecipitationimage, *displayglobalclimateimage, *displayglobalriversimage, *displayglobalreliefimage);

                globalmaptexture->loadFromImage(*displayglobalreliefimage);
                globalmap->setTexture(*globalmaptexture);

                updatereport("");
                updatereport("World generation completed.");
                updatereport("");

                focused = 0;

                completingimportpass = 0;
                screenmode = movingtoglobalmapscreen;
                newworld = 1;
            }
        }

        if (screenmode == loadingworldscreen)
        {
            if (loadingworldpass < 10)
            {
                ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 507, main_viewport->WorkPos.y + 173), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize(ImVec2(173, 68), ImGuiCond_FirstUseEver);
                ImGui::Begin("Please wait...##loadworld ", NULL, window_flags);
                ImGui::Text("Loading world...");
                ImGui::End();

                loadingworldpass++;
            }
            else
            {
                bool success=world->loadworld(filepathname);

                if (success == 0) // Failed to load
                {
                    screenmode = loadfailure;
                    loadingworld = 0;
                    loadingworldpass = 0;
                }
                else
                {
                    currentsize = world->size();
                    currentgravity = world->gravity();
                    currentlunar = world->lunar();
                    currenteccentricity = world->eccentricity();
                    currentperihelion = world->perihelion();
                    currentrotation = world->rotation();
                    currenttilt = world->tilt();
                    currenttempdecrease = world->tempdecrease();
                    currentnorthpolaradjust = world->northpolaradjust();
                    currentsouthpolaradjust = world->southpolaradjust();
                    currentaveragetemp = world->averagetemp();
                    currentwaterpickup = world->waterpickup();
                    currentglacialtemp = world->glacialtemp();
                    
                    //  Put all the correct values into the settings.

                    oceancolour.x = (float)world->ocean1() / 255.f;
                    oceancolour.y = (float)world->ocean2() / 255.f;
                    oceancolour.z = (float)world->ocean3() / 255.f;
                    oceancolour.w = 1.f;

                    deepoceancolour.x = (float)world->deepocean1() / 255.f;
                    deepoceancolour.y = (float)world->deepocean2() / 255.f;
                    deepoceancolour.z = (float)world->deepocean3() / 255.f;
                    deepoceancolour.w = 1.f;

                    basecolour.x = (float)world->base1() / 255.f;
                    basecolour.y = (float)world->base2() / 255.f;
                    basecolour.z = (float)world->base3() / 255.f;
                    basecolour.w = 1.f;

                    grasscolour.x = (float)world->grass1() / 255.f;
                    grasscolour.y = (float)world->grass2() / 255.f;
                    grasscolour.z = (float)world->grass3() / 255.f;
                    grasscolour.w = 1.f;

                    basetempcolour.x = (float)world->basetemp1() / 255.f;
                    basetempcolour.y = (float)world->basetemp2() / 255.f;
                    basetempcolour.z = (float)world->basetemp3() / 255.f;
                    basetempcolour.w = 1.f;

                    highbasecolour.x = (float)world->highbase1() / 255.f;
                    highbasecolour.y = (float)world->highbase2() / 255.f;
                    highbasecolour.z = (float)world->highbase3() / 255.f;
                    highbasecolour.w = 1.f;

                    desertcolour.x = (float)world->desert1() / 255.f;
                    desertcolour.y = (float)world->desert2() / 255.f;
                    desertcolour.z = (float)world->desert3() / 255.f;
                    desertcolour.w = 1.f;

                    highdesertcolour.x = (float)world->highdesert1() / 255.f;
                    highdesertcolour.y = (float)world->highdesert2() / 255.f;
                    highdesertcolour.z = (float)world->highdesert3() / 255.f;
                    highdesertcolour.w = 1.f;

                    colddesertcolour.x = (float)world->colddesert1() / 255.f;
                    colddesertcolour.y = (float)world->colddesert2() / 255.f;
                    colddesertcolour.z = (float)world->colddesert3() / 255.f;
                    colddesertcolour.w = 1.f;

                    eqtundracolour.x = (float)world->eqtundra1() / 255.f;
                    eqtundracolour.y = (float)world->eqtundra2() / 255.f;
                    eqtundracolour.z = (float)world->eqtundra3() / 255.f;
                    eqtundracolour.w = 1.f;

                    tundracolour.x = (float)world->tundra1() / 255.f;
                    tundracolour.y = (float)world->tundra2() / 255.f;
                    tundracolour.z = (float)world->tundra3() / 255.f;
                    tundracolour.w = 1.f;

                    coldcolour.x = (float)world->cold1() / 255.f;
                    coldcolour.y = (float)world->cold2() / 255.f;
                    coldcolour.z = (float)world->cold3() / 255.f;
                    coldcolour.w = 1.f;

                    seaicecolour.x = (float)world->seaice1() / 255.f;
                    seaicecolour.y = (float)world->seaice2() / 255.f;
                    seaicecolour.z = (float)world->seaice3() / 255.f;
                    seaicecolour.w = 1.f;

                    glaciercolour.x = (float)world->glacier1() / 255.f;
                    glaciercolour.y = (float)world->glacier2() / 255.f;
                    glaciercolour.z = (float)world->glacier3() / 255.f;
                    glaciercolour.w = 1.f;

                    saltpancolour.x = (float)world->saltpan1() / 255.f;
                    saltpancolour.y = (float)world->saltpan2() / 255.f;
                    saltpancolour.z = (float)world->saltpan3() / 255.f;
                    saltpancolour.w = 1.f;

                    ergcolour.x = (float)world->erg1() / 255.f;
                    ergcolour.y = (float)world->erg2() / 255.f;
                    ergcolour.z = (float)world->erg3() / 255.f;
                    ergcolour.w = 1.f;

                    wetlandscolour.x = (float)world->wetlands1() / 255.f;
                    wetlandscolour.y = (float)world->wetlands2() / 255.f;
                    wetlandscolour.z = (float)world->wetlands3() / 255.f;
                    wetlandscolour.w = 1.f;

                    lakecolour.x = (float)world->lake1() / 255.f;
                    lakecolour.y = (float)world->lake2() / 255.f;
                    lakecolour.z = (float)world->lake3() / 255.f;
                    lakecolour.w = 1.f;

                    rivercolour.x = (float)world->river1() / 255.f;
                    rivercolour.y = (float)world->river2() / 255.f;
                    rivercolour.z = (float)world->river3() / 255.f;
                    rivercolour.w = 1.f;

                    mudcolour.x = (float)world->mud1() / 255.f;
                    mudcolour.y = (float)world->mud2() / 255.f;
                    mudcolour.z = (float)world->mud3() / 255.f;
                    mudcolour.w = 1.f;

                    sandcolour.x = (float)world->sand1() / 255.f;
                    sandcolour.y = (float)world->sand2() / 255.f;
                    sandcolour.z = (float)world->sand3() / 255.f;
                    sandcolour.w = 1.f;

                    shinglecolour.x = (float)world->shingle1() / 255.f;
                    shinglecolour.y = (float)world->shingle2() / 255.f;
                    shinglecolour.z = (float)world->shingle3() / 255.f;
                    shinglecolour.w = 1.f;

                    mangrovecolour.x = (float)world->mangrove1() / 255.f;
                    mangrovecolour.y = (float)world->mangrove2() / 255.f;
                    mangrovecolour.z = (float)world->mangrove3() / 255.f;
                    mangrovecolour.w = 1.f;

                    highlightcolour.x = (float)world->highlight1() / 255.f;
                    highlightcolour.y = (float)world->highlight2() / 255.f;
                    highlightcolour.z = (float)world->highlight3() / 255.f;
                    highlightcolour.w = 1.f;

                    shadingland = world->landshading();
                    shadinglake = world->lakeshading();
                    shadingsea = world->seashading();
                    marblingland = world->landmarbling();
                    marblinglake = world->lakemarbling();
                    marblingsea = world->seamarbling();

                    minriverflowglobal = world->minriverflowglobal();
                    minriverflowregional = world->minriverflowregional();

                    globalriversentry = world->minriverflowglobal();
                    regionalriversentry = world->minriverflowregional();

                    if (world->shadingdir() == 4)
                        shadingdir = 0;

                    if (world->shadingdir() == 6)
                        shadingdir = 1;

                    if (world->shadingdir() == 2)
                        shadingdir = 2;

                    if (world->shadingdir() == 8)
                        shadingdir = 3;

                    snowchange = world->snowchange() - 1;
                    seaiceappearance = world->seaiceappearance() - 1;
                    colourcliffs = world->colourcliffs();
                    mangroves = world->showmangroves();
                    
                    // Now draw the new world.
                    
                    adjustforsize(*world, globaltexturesize, *globalelevationimage, *globaltemperatureimage, *globalprecipitationimage, *globalclimateimage, *globalriversimage, *globalreliefimage, *highlightimage, highlightsize, *minihighlightimage, minihighlightsize);

                    minihighlighttexture->loadFromImage(*minihighlightimage);
                    minihighlight->setTexture(*minihighlighttexture);
                    minihighlight->setOrigin((float)minihighlightsize / 2.0f, (float)minihighlightsize / 2.0f);

                    for (int n = 0; n < GLOBALMAPTYPES; n++)
                        globalmapimagecreated[n] = 0;

                    mapview = relief;
                    drawglobalmapimage(mapview, *world, globalmapimagecreated, *globalelevationimage, *globaltemperatureimage, *globalprecipitationimage, *globalclimateimage, *globalriversimage, *globalreliefimage, *displayglobalelevationimage, *displayglobaltemperatureimage, *displayglobalprecipitationimage, *displayglobalclimateimage, *displayglobalriversimage, *displayglobalreliefimage);

                    globalmaptexture->loadFromImage(*displayglobalreliefimage);
                    globalmap->setTexture(*globalmaptexture);

                    focused = 0;
                    newworld = 1;

                    filepathname = "";
                    filepath = "";

                    screenmode = globalmapscreen;
                    loadingworld = 0;
                    loadingworldpass = 0;
                }
            }
        }

        if (screenmode == savingworldscreen)
        {
            if (savingworldpass < 10)
            {
                ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 507, main_viewport->WorkPos.y + 173), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize(ImVec2(173, 68), ImGuiCond_FirstUseEver);
                ImGui::Begin("Please wait...##saveworld ", NULL, window_flags);
                ImGui::Text("Saving world...");
                ImGui::End();

                savingworldpass++;
            }
            else
            {
                world->saveworld(filepathname);

                filepathname = "";
                filepath = "";

                savingworld = 0;

                savingworldpass = 0;

                screenmode = movingtoglobalmapscreen;
            }
        }

        if (screenmode == exportingareascreen)
        {
            if (exportingareapass < 10)
            {
                ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 300, main_viewport->WorkPos.y + 200), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize(ImVec2(173, 68), ImGuiCond_FirstUseEver);
                ImGui::Begin("Please wait...##exportarea ", NULL, window_flags);
                ImGui::Text("Generating area maps...");
                ImGui::End();

                exportingareapass++;
            }
            else
            {
                int oldregionalcentrex = region->centrex();
                int oldregionalcentrey = region->centrey();

                mapviewenum oldmapview = mapview;

                float regiontilewidth = (float)REGIONALTILEWIDTH; //30;
                float regiontileheight = (float)REGIONALTILEHEIGHT; //30; // The width and height of the visible regional map, in tiles.

                int regionwidth = (int)(regiontilewidth * 16.0f);
                int regionheight = (int)(regiontileheight * 16.0f); // The width and height of the visible regional map, in pixels.

                int origareanwx = areanwx; // This is because the regions we'll be making will start to the north and west of the defined area.
                int origareanwy = areanwy;

                int origareanex = areanex;
                int origareaney = areaney;

                int origareaswx = areaswx;
                int origareaswy = areaswy;

                areanwx = areanwx / (int)regiontilewidth;
                areanwx = areanwx * (int)regiontilewidth;

                areanwy = areanwy / (int)regiontileheight;
                areanwy = areanwy * (int)regiontileheight;

                areaswx = areanwx;
                areaney = areanwy;

                int woffset = (origareanwx - areanwx) * 16;
                int noffset = (origareanwy - areanwy) * 16;

                float areatilewidth = (float)(areanex - areanwx);
                float areatileheight = (float)(areasey - areaney);

                int areawidth = (int)areatilewidth * 16;
                int areaheight = (int)areatileheight * 16;

                float imageareatilewidth = (float)(origareanex - origareanwx);
                float imageareatileheight = (float)(areasey - origareaney);

                int imageareawidth = (int)imageareatilewidth * 16;
                int imageareaheight = (int)imageareatileheight * 16;

                float fregionswide = areatilewidth / regiontilewidth;
                float fregionshigh = areatileheight / regiontileheight;

                int regionswide = (int)fregionswide;
                int regionshigh = (int)fregionshigh;

                if (regionswide != fregionswide)
                    regionswide++;

                if (regionshigh != fregionshigh)
                    regionshigh++;

                int totalregions = regionswide * regionshigh; // This is how many regional maps we're going to have to do.

                if (areafromregional == 1)
                    totalregions++; // Because we'll have to redo the regional map we came from.

                initialiseregion(*world, *region); // We'll do all this using the same region object as usual. We could create a new region object for it, but that seems to lead to inexplicable crashes, so we won't.

                // Now we need to prepare the images that we're going to copy the regional maps onto.

                sf::Image* areareliefimage = new sf::Image;
                sf::Image* areaelevationimage = new sf::Image;
                sf::Image* areatemperatureimage = new sf::Image;
                sf::Image* areaprecipitationimage = new sf::Image;
                sf::Image* areaclimateimage = new sf::Image;
                sf::Image* areariversimage = new sf::Image;

                areareliefimage->create(imageareawidth + 1, imageareaheight + 1, sf::Color::Black);
                areaelevationimage->create(imageareawidth + 1, imageareaheight + 1, sf::Color::Black);
                areatemperatureimage->create(imageareawidth + 1, imageareaheight + 1, sf::Color::Black);
                areaprecipitationimage->create(imageareawidth + 1, imageareaheight + 1, sf::Color::Black);
                areaclimateimage->create(imageareawidth + 1, imageareaheight + 1, sf::Color::Black);
                areariversimage->create(imageareawidth + 1, imageareaheight + 1, sf::Color::Black);

                // Now it's time to make the regions, one by one, generate their maps, and copy those maps over onto the area images.

                for (int i = 0; i < regionswide; i++)
                {
                    int centrex = i * (int)regiontilewidth + ((int)regiontilewidth / 2) + areanwx;

                    for (int j = 0; j < regionshigh; j++)
                    {
                        int centrey = j * (int)regiontileheight + ((int)regiontileheight / 2) + areanwy;

                        // First, create the new region.

                        region->setcentrex(centrex);
                        region->setcentrey(centrey);

                        generateregionalmap(*world, *region, smalllake, island, *peaks, riftblob, riftblobsize, 0, smudge, smallsmudge,squareroot);

                        // Now generate the maps.

                        for (int n = 0; n < GLOBALMAPTYPES; n++)
                            regionalmapimagecreated[n] = 0;

                        mapview = relief;
                        drawregionalmapimage(mapview, *world, *region, regionalmapimagecreated, *regionalelevationimage, *regionaltemperatureimage, *regionalprecipitationimage, *regionalclimateimage, *regionalriversimage, *regionalreliefimage);

                        mapview = elevation;
                        drawregionalmapimage(mapview, *world, *region, regionalmapimagecreated, *regionalelevationimage, *regionaltemperatureimage, *regionalprecipitationimage, *regionalclimateimage, *regionalriversimage, *regionalreliefimage);

                        mapview = temperature;
                        drawregionalmapimage(mapview, *world, *region, regionalmapimagecreated, *regionalelevationimage, *regionaltemperatureimage, *regionalprecipitationimage, *regionalclimateimage, *regionalriversimage, *regionalreliefimage);

                        mapview = precipitation;
                        drawregionalmapimage(mapview, *world, *region, regionalmapimagecreated, *regionalelevationimage, *regionaltemperatureimage, *regionalprecipitationimage, *regionalclimateimage, *regionalriversimage, *regionalreliefimage);

                        mapview = climate;
                        drawregionalmapimage(mapview, *world, *region, regionalmapimagecreated, *regionalelevationimage, *regionaltemperatureimage, *regionalprecipitationimage, *regionalclimateimage, *regionalriversimage, *regionalreliefimage);

                        mapview = rivers;
                        drawregionalmapimage(mapview, *world, *region, regionalmapimagecreated, *regionalelevationimage, *regionaltemperatureimage, *regionalprecipitationimage, *regionalclimateimage, *regionalriversimage, *regionalreliefimage);

                        // Now copy those maps into the images that will be exported.

                        for (int x = 0; x < regionalimagewidth; x++)
                        {
                            for (int y = 0; y < regionalimageheight; y++)
                            {
                                int thisx = x + i * regionwidth - woffset;
                                int thisy = y + j * regionheight - noffset; // Coordinates on the export image that correspond to this point on the regional map

                                if (thisx > 0 && thisx < imageareawidth && thisy>0 && thisy < imageareaheight)
                                {
                                    sf::Color pixelcolour = regionalreliefimage->getPixel(x, y);
                                    areareliefimage->setPixel(thisx, thisy, pixelcolour);

                                    pixelcolour = regionalelevationimage->getPixel(x, y);
                                    areaelevationimage->setPixel(thisx, thisy, pixelcolour);

                                    pixelcolour = regionaltemperatureimage->getPixel(x, y);
                                    areatemperatureimage->setPixel(thisx, thisy, pixelcolour);

                                    pixelcolour = regionalprecipitationimage->getPixel(x, y);
                                    areaprecipitationimage->setPixel(thisx, thisy, pixelcolour);

                                    pixelcolour = regionalclimateimage->getPixel(x, y);
                                    areaclimateimage->setPixel(thisx, thisy, pixelcolour);

                                    pixelcolour = regionalriversimage->getPixel(x, y);
                                    areariversimage->setPixel(thisx, thisy, pixelcolour);
                                }
                            }
                        }
                    }
                }

                region->setcentrex(oldregionalcentrex); // Move the region back to where it started.
                region->setcentrey(oldregionalcentrey);

                if (areafromregional == 1) // If we're going to go back to the regional map, we need to redo it.
                    generateregionalmap(*world, *region, smalllake, island, *peaks, riftblob, riftblobsize, 0, smudge, smallsmudge, squareroot);

                // Now just save the images.

                filepathname.resize(filepathname.size() - 4);

                areareliefimage->saveToFile(filepathname + " Relief.png");
                areaelevationimage->saveToFile(filepathname + " Elevation.png");
                areatemperatureimage->saveToFile(filepathname + " Temperature.png");
                areaprecipitationimage->saveToFile(filepathname + " Precipitation.png");
                areaclimateimage->saveToFile(filepathname + " Climate.png");
                areariversimage->saveToFile(filepathname + " Rivers.png");

                // Clean up.

                filepathname = "";
                filepath = "";

                delete areareliefimage;
                delete areaelevationimage;
                delete areatemperatureimage;
                delete areaprecipitationimage;
                delete areaclimateimage;
                delete areariversimage;

                areanex = -1;
                areaney = -1;
                areasex = -1;
                areasey = -1;
                areaswx = -1;
                areaswy = -1;
                areanwx = -1;
                areanwy = -1;

                mapview = oldmapview;

                for (int n = 0; n < GLOBALMAPTYPES; n++)
                    regionalmapimagecreated[n] = 0;

                exportingareapass = 0;

                if (areafromregional == 1)
                    screenmode = regionalmapscreen;
                else
                    screenmode = globalmapscreen;
            }
        }

        if (screenmode == generatingregionscreen)
        {
            showcolouroptions = 0;
            showworldproperties = 0;
            showglobalrainfallchart = 0;
            showglobaltemperaturechart = 0;
            showregionalrainfallchart = 0;
            showregionaltemperaturechart = 0;
            
            if (generatingregionpass < 10) // This one has a non-functioning copy of the regional map screen controls, plus the "Please wait" message.
            {
                string title;

                if (world->seed() >= 0)
                    title = "Seed: " + to_string(world->seed());
                else
                    title = "Custom";

                title = title + "##regional";

                ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 10, main_viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize(ImVec2(161, 449), ImGuiCond_FirstUseEver);

                ImGui::Begin(title.c_str(), NULL, window_flags);

                ImGui::Text("World controls:");

                ImGui::PushItemWidth(100.0f);

                standardbutton("World map");

                ImGui::Dummy(ImVec2(0.0f, linespace));

                ImGui::SetNextItemWidth(0);

                ImGui::Text("Export options:");

                ImGui::PushItemWidth(100.0f);

                standardbutton("Regional maps");

                standardbutton("Area maps");

                ImGui::Dummy(ImVec2(0.0f, linespace));

                ImGui::SetNextItemWidth(0);

                ImGui::Text("Display map type:");

                ImGui::PushItemWidth(100.0f);

                standardbutton("Relief");

                standardbutton("Elevation");

                standardbutton("Temperature");

                standardbutton("Precipitation");

                standardbutton("Climate");

                standardbutton("Rivers");

                ImGui::Dummy(ImVec2(0.0f, linespace));

                ImGui::SetNextItemWidth(0);

                ImGui::Text("Other controls:");

                ImGui::PushItemWidth(100.0f);

                standardbutton("Properties");

                standardbutton("Appearance");

                ImGui::End();

                // Now the text box.

                ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 180, main_viewport->WorkPos.y + 542), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize(ImVec2(1023, 140), ImGuiCond_FirstUseEver);

                title = "               ";

                ImGui::Begin(title.c_str(), NULL, window_flags);

                ImGui::End();

                // Now the additional element.

                ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 849, main_viewport->WorkPos.y + 364), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize(ImVec2(200, 60), ImGuiCond_FirstUseEver);
                ImGui::Begin("Please wait...##generateregion ", NULL, window_flags);
                ImGui::Text("Generating region");
                ImGui::End();

                generatingregionpass++;
            }
            else
            {
                newx = newx / 32;
                newy = newy / 32;

                newx = newx * 32;
                newy = newy * 32;

                newx = newx + 16;
                newy = newy + 16;

                region->setcentrex(newx);
                region->setcentrey(newy);

                mapview = relief;

                for (int n = 0; n < GLOBALMAPTYPES; n++)
                    regionalmapimagecreated[n] = 0;

                float progressstep = 1.0f / REGIONALCREATIONSTEPS;

                // Blank the regional map image first

                for (int i = 0; i < regionalimagewidth; i++)
                {
                    for (int j = 0; j < regionalimageheight; j++)
                        regionalreliefimage->setPixel(i, j, sf::Color::Black);

                }

                regionalmaptexture->loadFromImage(*regionalreliefimage);
                regionalmap->setTexture(*regionalmaptexture);

                // Now generate the regional map

                generateregionalmap(*world, *region, smalllake, island, *peaks, riftblob, riftblobsize, 0, smudge, smallsmudge, squareroot);

                // Now draw the regional map image

                drawregionalmapimage(mapview, *world, *region, regionalmapimagecreated, *regionalelevationimage, *regionaltemperatureimage, *regionalprecipitationimage, *regionalclimateimage, *regionalriversimage, *regionalreliefimage);

                regionalmaptexture->loadFromImage(*regionalreliefimage);
                regionalmap->setTexture(*regionalmaptexture);

                // Sort out the minimap

                globalmaptexture->loadFromImage(*displayglobalreliefimage);
                minimap->setTexture(*globalmaptexture);

                focused = 0;
                generatingregionpass = 0;

                screenmode = regionalmapscreen;
            }
        }

        if (screenmode == loadfailure)
        {
            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 446, main_viewport->WorkPos.y + 174), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(296, 129), ImGuiCond_FirstUseEver);

            ImGui::Begin("Loading unsuccessful!", NULL, window_flags);

            ImGui::Text("This world file is not compatible with the");
            ImGui::Text("current version of Undiscovered Worlds.");

            ImGui::Text(" ");
            ImGui::Text(" ");

            ImGui::SameLine((float)135);

            if (ImGui::Button("OK"))
            {
                if (brandnew)
                    screenmode = createworldscreen;
                else
                    screenmode = globalmapscreen;
            }

            ImGui::End();
        }

        if (screenmode == settingsloadfailure)
        {
            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 446, main_viewport->WorkPos.y + 174), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(310, 129), ImGuiCond_FirstUseEver);

            ImGui::Begin("Loading unsuccessful!##settings ", NULL, window_flags);

            ImGui::Text("This settings file is not compatible with the");
            ImGui::Text("current version of Undiscovered Worlds.");

            ImGui::Text(" ");
            ImGui::Text(" ");

            ImGui::SameLine((float)135);

            if (ImGui::Button("OK"))
                screenmode = oldscreenmode;

            ImGui::End();
        }

        window.clear();

        // Colour options, if being shown

        if (showcolouroptions && screenmode != settingsloadfailure)
        {
            int colouralign = 360;
            int otheralign = 330;

            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 316, main_viewport->WorkPos.y + 24), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(655, 440), ImGuiCond_FirstUseEver);

            ImGui::Begin("Map appearance", NULL, window_flags);

            ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
            if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
            {
                if (ImGui::BeginTabItem("Colours"))
                {
                    ImGui::Text("  ");

                    ImGui::PushItemWidth(200);

                    ImGui::ColorEdit3("Shallow ocean", (float*)&oceancolour);
                    ImGui::SameLine((float)colouralign);
                    ImGui::ColorEdit3("Sea ice", (float*)&seaicecolour);

                    ImGui::ColorEdit3("Deep ocean", (float*)&deepoceancolour);
                    ImGui::SameLine((float)colouralign);
                    ImGui::ColorEdit3("Glaciers", (float*)&glaciercolour);

                    ImGui::ColorEdit3("Base land", (float*)&basecolour);
                    ImGui::SameLine((float)colouralign);
                    ImGui::ColorEdit3("Salt pans", (float*)&saltpancolour);

                    ImGui::ColorEdit3("Grassland", (float*)&grasscolour);
                    ImGui::SameLine((float)colouralign);
                    ImGui::ColorEdit3("Dunes", (float*)&ergcolour);

                    ImGui::ColorEdit3("Low temperate", (float*)&basetempcolour);
                    ImGui::SameLine((float)colouralign);
                    ImGui::ColorEdit3("Rivers", (float*)&rivercolour);

                    ImGui::ColorEdit3("High temperate", (float*)&highbasecolour);
                    ImGui::SameLine((float)colouralign);
                    ImGui::ColorEdit3("Lakes", (float*)&lakecolour);

                    ImGui::ColorEdit3("Low desert", (float*)&desertcolour);
                    ImGui::SameLine((float)colouralign);
                    ImGui::ColorEdit3("Wetlands", (float*)&wetlandscolour);

                    ImGui::ColorEdit3("High desert", (float*)&highdesertcolour);
                    ImGui::SameLine((float)colouralign);
                    ImGui::ColorEdit3("Mangroves", (float*)&mangrovecolour);

                    ImGui::ColorEdit3("Cold desert", (float*)&colddesertcolour);
                    ImGui::SameLine((float)colouralign);
                    ImGui::ColorEdit3("Mud", (float*)&mudcolour);

                    ImGui::ColorEdit3("Mild tundra", (float*)&eqtundracolour);
                    ImGui::SameLine((float)colouralign);
                    ImGui::ColorEdit3("Sand", (float*)&sandcolour);

                    ImGui::ColorEdit3("Tundra", (float*)&tundracolour);
                    ImGui::SameLine((float)colouralign);
                    ImGui::ColorEdit3("Shingle", (float*)&shinglecolour);

                    ImGui::ColorEdit3("Arctic", (float*)&coldcolour);
                    ImGui::SameLine((float)colouralign);
                    ImGui::ColorEdit3("Highlights", (float*)&highlightcolour);

                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Effects"))
                {
                    ImGui::Text("  ");

                    ImGui::PushItemWidth(200);

                    ImGui::Text("Shading");

                    ImGui::SameLine((float)otheralign);
                    ImGui::Text("Rivers");

                    ImGui::SetCursorPosX(20);
                    ImGui::SliderFloat("On land", &shadingland, 0.0f, 1.0f, "%.2f");

                    ImGui::SameLine((float)otheralign + 20);
                    ImGui::InputInt("Global map", &globalriversentry);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Only rivers greater than this size will be displayed on the global relief map.");

                    ImGui::SetCursorPosX(20);
                    ImGui::SliderFloat("On lakes", &shadinglake, 0.0f, 1.0f, "%.2f");

                    ImGui::SameLine((float)otheralign + 20);
                    ImGui::InputInt("Regional map", &regionalriversentry);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Only rivers greater than this size will be displayed on the regional relief map.");

                    ImGui::SetCursorPosX(20);
                    ImGui::SliderFloat("On sea", &shadingsea, 0.0f, 1.0f, "%.2f");

                    ImGui::Text("Marbling");

                    ImGui::SameLine((float)otheralign);
                    ImGui::Text("Other effects");

                    ImGui::SetCursorPosX(20);
                    ImGui::SliderFloat("On land ", &marblingland, 0.0f, 1.0f, "%.2f");

                    ImGui::SameLine((float)otheralign + 20);
                    const char* lightdiritems[] = { "Southeast","Southwest","Northeast","Northwest" };
                    ImGui::Combo("Light", &shadingdir, lightdiritems, 4);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Controls the light source for the shading effect.");

                    ImGui::SetCursorPosX(20);
                    ImGui::SliderFloat("On lakes ", &marblinglake, 0.0f, 1.0f, "%.2f");


                    ImGui::SameLine((float)otheralign + 20);
                    const char* snowitems[] = { "Sudden","Speckled","Smooth" };
                    ImGui::Combo("Snow", &snowchange, snowitems, 3);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Changes the appearance of transitions from snowy regions to non-snowy ones.");

                    ImGui::SetCursorPosX(20);
                    ImGui::SliderFloat("On sea ", &marblingsea, 0.0f, 1.0f, " % .2f");

                    ImGui::SameLine((float)otheralign + 20);
                    const char* seaiceitems[] = { "Permanent","None","All" };
                    ImGui::Combo("Sea ice", &seaiceappearance, seaiceitems, 3);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Determines whether sea ice is shown.");

                    ImGui::Text("    ");

                    ImGui::SetCursorPosX(20);
                    ImGui::Checkbox("Show mangrove forests", &mangroves);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Shows mangrove forests on tropical mud flats and salt wetlands.");

                    ImGui::SameLine((float)otheralign + 20);
                    ImGui::Checkbox("Only cliffs use high base colour", &colourcliffs);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Uses high base colour only on steep slopes. May look better on non-tectonic worlds with high plateaux.");

                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }

            ImGui::SetCursorPos(ImVec2(75.0f, 405.0f));

            if (ImGui::Button("Save", ImVec2(120.0f, 0.0f)))
            {
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".uws", ".");

                savingsettings = 1;
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Save map appearance settings.");

            ImGui::SameLine();
            if (ImGui::Button("Load", ImVec2(120.0f, 0.0f)))
            {
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".uws", ".");

                loadingsettings = 1;
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Load map appearance settings.");

            ImGui::SameLine();
            if (ImGui::Button("Default", ImVec2(120.0f, 0.0f)))
            {
                initialisemapcolours(*world);

                // Now put all the correct values into the settings.

                oceancolour.x = (float)world->ocean1() / 255.f;
                oceancolour.y = (float)world->ocean2() / 255.f;
                oceancolour.z = (float)world->ocean3() / 255.f;
                oceancolour.w = 1.f;

                deepoceancolour.x = (float)world->deepocean1() / 255.f;
                deepoceancolour.y = (float)world->deepocean2() / 255.f;
                deepoceancolour.z = (float)world->deepocean3() / 255.f;
                deepoceancolour.w = 1.f;

                basecolour.x = (float)world->base1() / 255.f;
                basecolour.y = (float)world->base2() / 255.f;
                basecolour.z = (float)world->base3() / 255.f;
                basecolour.w = 1.f;

                grasscolour.x = (float)world->grass1() / 255.f;
                grasscolour.y = (float)world->grass2() / 255.f;
                grasscolour.z = (float)world->grass3() / 255.f;
                grasscolour.w = 1.f;

                basetempcolour.x = (float)world->basetemp1() / 255.f;
                basetempcolour.y = (float)world->basetemp2() / 255.f;
                basetempcolour.z = (float)world->basetemp3() / 255.f;
                basetempcolour.w = 1.f;

                highbasecolour.x = (float)world->highbase1() / 255.f;
                highbasecolour.y = (float)world->highbase2() / 255.f;
                highbasecolour.z = (float)world->highbase3() / 255.f;
                highbasecolour.w = 1.f;

                desertcolour.x = (float)world->desert1() / 255.f;
                desertcolour.y = (float)world->desert2() / 255.f;
                desertcolour.z = (float)world->desert3() / 255.f;
                desertcolour.w = 1.f;

                highdesertcolour.x = (float)world->highdesert1() / 255.f;
                highdesertcolour.y = (float)world->highdesert2() / 255.f;
                highdesertcolour.z = (float)world->highdesert3() / 255.f;
                highdesertcolour.w = 1.f;

                colddesertcolour.x = (float)world->colddesert1() / 255.f;
                colddesertcolour.y = (float)world->colddesert2() / 255.f;
                colddesertcolour.z = (float)world->colddesert3() / 255.f;
                colddesertcolour.w = 1.f;

                eqtundracolour.x = (float)world->eqtundra1() / 255.f;
                eqtundracolour.y = (float)world->eqtundra2() / 255.f;
                eqtundracolour.z = (float)world->eqtundra3() / 255.f;
                eqtundracolour.w = 1.f;

                tundracolour.x = (float)world->tundra1() / 255.f;
                tundracolour.y = (float)world->tundra2() / 255.f;
                tundracolour.z = (float)world->tundra3() / 255.f;
                tundracolour.w = 1.f;

                coldcolour.x = (float)world->cold1() / 255.f;
                coldcolour.y = (float)world->cold2() / 255.f;
                coldcolour.z = (float)world->cold3() / 255.f;
                coldcolour.w = 1.f;

                seaicecolour.x = (float)world->seaice1() / 255.f;
                seaicecolour.y = (float)world->seaice2() / 255.f;
                seaicecolour.z = (float)world->seaice3() / 255.f;
                seaicecolour.w = 1.f;

                glaciercolour.x = (float)world->glacier1() / 255.f;
                glaciercolour.y = (float)world->glacier2() / 255.f;
                glaciercolour.z = (float)world->glacier3() / 255.f;
                glaciercolour.w = 1.f;

                saltpancolour.x = (float)world->saltpan1() / 255.f;
                saltpancolour.y = (float)world->saltpan2() / 255.f;
                saltpancolour.z = (float)world->saltpan3() / 255.f;
                saltpancolour.w = 1.f;

                ergcolour.x = (float)world->erg1() / 255.f;
                ergcolour.y = (float)world->erg2() / 255.f;
                ergcolour.z = (float)world->erg3() / 255.f;
                ergcolour.w = 1.f;

                wetlandscolour.x = (float)world->wetlands1() / 255.f;
                wetlandscolour.y = (float)world->wetlands2() / 255.f;
                wetlandscolour.z = (float)world->wetlands3() / 255.f;
                wetlandscolour.w = 1.f;

                lakecolour.x = (float)world->lake1() / 255.f;
                lakecolour.y = (float)world->lake2() / 255.f;
                lakecolour.z = (float)world->lake3() / 255.f;
                lakecolour.w = 1.f;

                rivercolour.x = (float)world->river1() / 255.f;
                rivercolour.y = (float)world->river2() / 255.f;
                rivercolour.z = (float)world->river3() / 255.f;
                rivercolour.w = 1.f;

                mudcolour.x = (float)world->mud1() / 255.f;
                mudcolour.y = (float)world->mud2() / 255.f;
                mudcolour.z = (float)world->mud3() / 255.f;
                mudcolour.w = 1.f;

                sandcolour.x = (float)world->sand1() / 255.f;
                sandcolour.y = (float)world->sand2() / 255.f;
                sandcolour.z = (float)world->sand3() / 255.f;
                sandcolour.w = 1.f;

                shinglecolour.x = (float)world->shingle1() / 255.f;
                shinglecolour.y = (float)world->shingle2() / 255.f;
                shinglecolour.z = (float)world->shingle3() / 255.f;
                shinglecolour.w = 1.f;

                mangrovecolour.x = (float)world->mangrove1() / 255.f;
                mangrovecolour.y = (float)world->mangrove2() / 255.f;
                mangrovecolour.z = (float)world->mangrove3() / 255.f;
                mangrovecolour.w = 1.f;

                highlightcolour.x = (float)world->highlight1() / 255.f;
                highlightcolour.y = (float)world->highlight2() / 255.f;
                highlightcolour.z = (float)world->highlight3() / 255.f;
                highlightcolour.w = 1.f;

                shadingland = world->landshading();
                shadinglake = world->lakeshading();
                shadingsea = world->seashading();
                marblingland = world->landmarbling();
                marblinglake = world->lakemarbling();
                marblingsea = world->seamarbling();

                minriverflowglobal = world->minriverflowglobal();
                minriverflowregional = world->minriverflowregional();

                globalriversentry = world->minriverflowglobal();
                regionalriversentry = world->minriverflowregional();

                if (world->shadingdir() == 4)
                    shadingdir = 0;

                if (world->shadingdir() == 6)
                    shadingdir = 1;

                if (world->shadingdir() == 2)
                    shadingdir = 2;

                if (world->shadingdir() == 8)
                    shadingdir = 3;

                snowchange = world->snowchange() - 1;
                seaiceappearance = world->seaiceappearance() - 1;
                colourcliffs = world->colourcliffs();
                mangroves = world->showmangroves();

                drawhighlightobjects(*world, *highlightimage, highlightsize, *minihighlightimage, minihighlightsize);

                highlighttexture->loadFromImage(*highlightimage);
                highlight->setTexture(*highlighttexture);

                minihighlighttexture->loadFromImage(*minihighlightimage);
                minihighlight->setTexture(*minihighlighttexture);
                minihighlight->setOrigin((float)minihighlightsize / 2.0f, (float)minihighlightsize / 2.0f);

                colourschanged = 1;
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Restore the default map appearance settings.");

            ImGui::SameLine();
            if (ImGui::Button("Close", ImVec2(120.0f, 0.0f)))
            {
                showcolouroptions = 0;
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Close the map appearance panel.");

            ImGui::End();
        }

        // World properties window, if being shown

        if (showworldproperties)
        {
            int rightalign = 290;

            int topleftfigures = 105;
            int bottomleftfigures = 225;
            int toprightfigures = 410;
            int bottomrightfigures = 460;

            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 265, main_viewport->WorkPos.y + 60), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(499, 264), ImGuiCond_FirstUseEver);

            ImGui::Begin("World properties", NULL, window_flags);

            string sizeinfo = "Size:";

            string sizevalue = "";

            if (world->size() == 0)
                sizevalue = "small";

            if (world->size() == 1)
                sizevalue = "medium";

            if (world->size() == 2)
                sizevalue = "large";

            stringstream ss3;
            ss3 << fixed << setprecision(5) << world->eccentricity();

            string eccentricityinfo = "Eccentricity:";
            string eccentricityvalue = ss3.str();

            stringstream ss4;
            ss4 << fixed << setprecision(2) << world->gravity();

            string gravityinfo = "Surface gravity: ";
            string gravityvalue = ss4.str() + "g";

            string perihelioninfo = "Perihelion:";
            string perihelionvalue = "";

            if (world->perihelion() == 0)
                perihelionvalue = "January";

            if (world->perihelion() == 1)
                perihelionvalue = "July";

            stringstream ss5;
            ss5 << fixed << setprecision(2) << world->lunar();

            string lunarinfo = "Lunar pull:";
            string lunarvalue = ss5.str();

            string typeinfo = "Category: ";
            string typevalue = to_string(world->type());

            if (typevalue == "1")
                typevalue = "tectonic (small)";

            if (typevalue == "2")
                typevalue = "tectonic (large)";

            if (typevalue == "3")
                typevalue = "oceanic";

            if (typevalue == "4")
                typevalue = "non-tectonic";

            string rotationinfo = "Rotation:";
            string rotationvalue = "";

            if (world->rotation())
                rotationvalue = "west to east";
            else
                rotationvalue = "east to west";

            stringstream ss;
            ss << fixed << setprecision(2) << world->tilt();

            string tiltinfo = "Obliquity:";
            string tiltvalue = ss.str() + degree;

            stringstream ss2;
            ss2 << fixed << setprecision(2) << world->tempdecrease();

            string tempdecreaseinfo = "Heat decrease per vertical km:";
            string tempdecreasevalue = ss2.str() + degree;

            string northpoleinfo = "North pole adjustment:";
            string northpolevalue = to_string(world->northpolaradjust()) + degree;

            string southpoleinfo = "South pole adjustment:";
            string southpolevalue = to_string(world->southpolaradjust()) + degree;

            string averageinfo = "Average global temperature:";
            string averagevalue = to_string(world->averagetemp()) + degree;

            stringstream ss6;
            ss6 << fixed << setprecision(2) << world->waterpickup();

            string moistureinfo = "Moisture pickup rate:";
            string moisturevalue = ss6.str();

            string glacialinfo = "Glaciation temperature:";
            string glacialvalue=to_string(world->glacialtemp())+degree;

            ImGui::Text(sizeinfo.c_str());

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Size of planet. (Earth: large; Mars: medium; Moon: small)");

            ImGui::SameLine((float)topleftfigures);
            ImGui::Text(sizevalue.c_str());

            ImGui::SameLine((float)rightalign);

            ImGui::Text(gravityinfo.c_str());

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Affects mountain and valley sizes. (Earth: 1.00g)");

            ImGui::SameLine((float)toprightfigures);
            ImGui::Text(gravityvalue.c_str());

            ImGui::Text(typeinfo.c_str());

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Terrain category. (Earth: large tectonic)");

            ImGui::SameLine((float)topleftfigures);
            ImGui::Text(typevalue.c_str());

            ImGui::SameLine((float)rightalign);

            ImGui::Text(lunarinfo.c_str());

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Affects tides and coastal regions. (Earth: 1.00)");

            ImGui::SameLine((float)toprightfigures);
            ImGui::Text(lunarvalue.c_str());

            ImGui::Text(rotationinfo.c_str());

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Affects weather patterns. (Earth: west to east)");

            ImGui::SameLine((float)topleftfigures);
            ImGui::Text(rotationvalue.c_str());

            ImGui::SameLine((float)rightalign);

            ImGui::Text(tiltinfo.c_str());

            string tilttip = "Affects seasonal variation in temperature. (Earth: 22.5" + degree + ")";

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip(tilttip.c_str());

            ImGui::SameLine((float)toprightfigures);
            ImGui::Text(tiltvalue.c_str());

            ImGui::Text(eccentricityinfo.c_str());

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("How elliptical the orbit is. (Earth: 0.0167)");

            ImGui::SameLine((float)topleftfigures);
            ImGui::Text(eccentricityvalue.c_str());

            ImGui::SameLine((float)rightalign);

            ImGui::Text(perihelioninfo.c_str());

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("When the planet is closest to the sun. (Earth: January)");

            ImGui::SameLine((float)toprightfigures);
            ImGui::Text(perihelionvalue.c_str());

            ImGui::Text("   ");

            ImGui::Text(tempdecreaseinfo.c_str());

            string tempdecreasetip = "Affects how much colder it gets higher up. (Earth: 6.5" + degree + ")";

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip(tempdecreasetip.c_str());

            ImGui::SameLine((float)bottomleftfigures);
            ImGui::Text(tempdecreasevalue.c_str());

            ImGui::SameLine((float)rightalign);

            ImGui::Text(moistureinfo.c_str());

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Affects how much moisture wind picks up from the ocean. (Earth: 1.0)");

            ImGui::SameLine((float)bottomrightfigures);
            ImGui::Text(moisturevalue.c_str());

            ImGui::Text("   ");

            ImGui::Text(averageinfo.c_str());

            string avetip = "Earth: 14" + degree;

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip(avetip.c_str());

            ImGui::SameLine((float)bottomleftfigures);
            ImGui::Text(averagevalue.c_str());

            ImGui::SameLine((float)rightalign);

            ImGui::Text(glacialinfo.c_str());

            string glacialtip = "Areas below this average temperature may show signs of past glaciation. (Earth: 4" + degree + ")";

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip(glacialtip.c_str());

            ImGui::SameLine((float)bottomrightfigures);
            ImGui::Text(glacialvalue.c_str());

            ImGui::Text(northpoleinfo.c_str());

            string northtip = "Adjustment to north pole temperature. (Earth: +3" + degree + ")";

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip(northtip.c_str());

            ImGui::SameLine((float)bottomleftfigures);
            ImGui::Text(northpolevalue.c_str());

            ImGui::Text(southpoleinfo.c_str());

            string southtip = "Adjustment to south pole temperature. (Earth: -3" + degree + ")";

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip(southtip.c_str());

            ImGui::SameLine((float)bottomleftfigures);
            ImGui::Text(southpolevalue.c_str());

            ImGui::SameLine((float)rightalign);

            if (ImGui::Button("Close", ImVec2(120.0f, 0.0f)))
            {
                showworldproperties = 0;
            }

            ImGui::End();
        }

        // World edit properties screen, if being shown

        if (showworldeditproperties)
        {
            int rightalign = 280;

            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 420, main_viewport->WorkPos.y + 50), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(415, 427), ImGuiCond_FirstUseEver);

            ImGui::Begin("World properties##2", NULL, window_flags);

            ImGui::PushItemWidth(180);

            const char* rotationitems[] = { "East to west", "West to east" };
            static int item_current = 0;
            ImGui::Combo("Rotation", &currentrotation, rotationitems, 2);

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Affects weather patterns. (Earth: west to east)");

            const char* perihelionitems[] = { "January", "July" };
            static int pitem_current = 0;
            ImGui::Combo("Perihelion", &currentperihelion, perihelionitems, 2);

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("When the planet is closest to the sun. (Earth: January)");

            ImGui::InputFloat("Eccentricity", &currenteccentricity, 0.01f, 1.0f, "%.3f");

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("How elliptical the orbit is. (Earth: 0.0167)");

            ImGui::InputFloat("Obliquity", &currenttilt, 0.01f, 1.0f, "%.3f");

            string tilttip = "Affects seasonal variation in temperature. (Earth: 22.5" + degree + ")";

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip(tilttip.c_str());

            ImGui::InputFloat("Surface gravity", &currentgravity, 0.01f, 1.0f, "%.3f");

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Affects mountain and valley sizes. (Earth: 1.00g)");


            ImGui::InputFloat("Lunar pull", &currentlunar, 0.01f, 1.0f, "%.3f");

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Affects tides and coastal regions. (Earth: 1.00)");

            ImGui::InputFloat("Moisture pickup rate", &currentwaterpickup, 0.01f, 1.0f, "%.3f");

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Affects how much moisture wind picks up from the ocean. (Earth: 1.00)");

            ImGui::InputFloat("Heat decrease per vertical km", &currenttempdecrease, 0.01f, 1.0f, "%.3f");

            string tempdecreasetip = "Affects how much colder it gets higher up. (Earth: 6.5" + degree + ")";

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip(tempdecreasetip.c_str());

            ImGui::InputInt("Glaciation temperature", &currentglacialtemp);

            string glacialtip = "Areas below this average temperature may show signs of past glaciation. (Earth: 4" + degree + ")";

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip(glacialtip.c_str());

            ImGui::InputInt("Average global temperature", &currentaveragetemp);

            string avetip = "Earth: 14" + degree;

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip(avetip.c_str());

            ImGui::InputInt("North pole adjustment", &currentnorthpolaradjust);

            string northtip = "Adjustment to north pole temperature. (Earth: +3" + degree + ")";

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip(northtip.c_str());

            ImGui::InputInt("South pole adjustment", &currentsouthpolaradjust);

            string southtip = "Adjustment to south pole temperature. (Earth: -3" + degree + ")";

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip(southtip.c_str());

            ImGui::Text("   ");

            ImGui::Checkbox("Generate rivers", &currentrivers);

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Untick this if you don't want rivers to be generated.");

            ImGui::SameLine((float)rightalign);

            ImGui::Checkbox("Generate lakes", &currentlakes);

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Untick this if you don't want large lakes to be generated.");

            ImGui::Checkbox("Generate deltas", &currentdeltas);

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Untick this if you don't want river deltas to be generated.");

            ImGui::SameLine((float)rightalign);

            if (ImGui::Button("Close", ImVec2(120.0f, 0.0f)))
            {
                showworldeditproperties = 0;
            }

            ImGui::End();

            if (currentgravity < 0.05f)
                currentgravity = 0.05f;

            if (currentgravity > 10.0f)
                currentgravity = 10.0f;
            
            if (currentwaterpickup < 0.0f)
                currentwaterpickup = 0.0f;

            if (currenttilt > 90.0f)
                currenttilt = 90.0f;

            if (currenttilt < -90.0f)
                currenttilt = -90.0f;

            if (currenteccentricity < 0.0f)
                currenteccentricity = 0.0f;

            if (currenteccentricity > 0.999f)
                currenteccentricity = 0.999f;

            if (currentlunar < 0.0f)
                currentlunar = 0.0f;

            if (currentlunar > 10.0f)
                currentlunar = 10.0f;

            if (currentrivers == 0)
            {
                currentlakes = 0;
                currentdeltas = 0;
            }
        }

        // Global temperature chart, if being shown

        if (showglobaltemperaturechart)
        {           
            float barwidth = 40.0f;
            
            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 680, main_viewport->WorkPos.y + 385), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(518, 139), ImGuiCond_FirstUseEver);

            ImGui::Begin("Temperature", NULL, window_flags);

            float temp[12];

            temp[0] = (float)world->jantemp(poix, poiy);
            temp[3] = (float)world->aprtemp(poix, poiy);
            temp[6] = (float)world->jultemp(poix, poiy);
            temp[9] = (float)world->octtemp(poix, poiy);

            temp[1] = (temp[0] * 2.0f + temp[3]) / 3.0f;
            temp[2] = (temp[0] + temp[3] * 2.0f) / 3.0f;
            temp[4] = (temp[3] * 2.0f + temp[6]) / 3.0f;
            temp[5] = (temp[3] + temp[6] * 2.0f) / 3.0f;
            temp[7] = (temp[6] * 2.0f + temp[9]) / 3.0f;
            temp[8] = (temp[6] + temp[9] * 2.0f) / 3.0f;
            temp[10] = (temp[9] * 2.0f + temp[0]) / 3.0f;
            temp[11] = (temp[9] + temp[0] * 2.0f) / 3.0f;

            float lowest = temp[0];
            float highest = temp[0];

            if (temp[3] < lowest)
                lowest = temp[3];

            if (temp[3] > highest)
                highest = temp[3];

            if (temp[6] < lowest)
                lowest = temp[6];

            if (temp[6] > highest)
                highest = temp[6];

            if (temp[9] < lowest)
                lowest = temp[9];

            if (temp[9] > highest)
                highest = temp[9];

            float subzero = 0.0f;

            if (lowest < 0.0f) // Ensure that all values are at least 0
            {
                subzero = 0.0f - lowest;

                for (int n = 0; n < 12; n++)
                    temp[n] = temp[n] + subzero;

                lowest = temp[0];
                highest = temp[0];

                if (temp[3] < lowest)
                    lowest = temp[3];

                if (temp[3] > highest)
                    highest = temp[3];

                if (temp[6] < lowest)
                    lowest = temp[6];

                if (temp[6] > highest)
                    highest = temp[6];

                if (temp[9] < lowest)
                    lowest = temp[9];

                if (temp[9] > highest)
                    highest = temp[9];
            }

            float temprange = highest - lowest;

            for (int n = 0; n < 13; n++)
            {
                ImGui::SameLine((float)barwidth * (float)n + barwidth / 2.0f);

                float value;

                if (n < 12)
                    value = temp[n];
                else
                    value = 0.0f;

                float temparr[] = { value / highest };

                float colour1 = 0.0f;
                float colour2 = 0.0f;
                float colour3 = 0.0f;

                float temperature = value - subzero;

                if (temperature > 0.0f)
                {
                    colour1 = 250.0f;
                    colour2 = 250.0f - temperature * 3.0f;
                    colour3 = 250.0f - temperature * 7.0f;
                }
                else
                {
                    temperature = abs(temperature);

                    colour1 = 250.0f - temperature * 7.0f;
                    colour2 = 250.0f - temperature * 7.0f;
                    colour3 = 250.0f;
                }

                if (colour1 < 0.0f)
                    colour1 = 0.0f;

                if (colour2 < 0.0f)
                    colour2 = 0.0f;

                if (colour3 < 0.0f)
                    colour3 = 0.0f;

                colour1 = colour1 / 255.0f;
                colour2 = colour2 / 255.0f;
                colour3 = colour3 / 255.0f;

                Style.Colors[ImGuiCol_PlotHistogram] = ImVec4(colour1, colour2, colour3, 1.00f);
                Style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(colour1, colour2, colour3, 1.00f);

                ImGui::PlotHistogram(" ", temparr, IM_ARRAYSIZE(temparr), 0, NULL, 0.0f, 1.0f, ImVec2(0.0f, barwidth));
            }

            ImGui::Text(" ");

            for (int n = 0; n < 12; n++)
            {
                string tempinfotext = formatnumber((int)temp[n] - (int)subzero);

                ImGui::SameLine((float)barwidth * (n + 1) - (ImGui::CalcTextSize(tempinfotext.c_str()).x) / 2);

                ImGui::Text(tempinfotext.c_str());
            }

            ImGui::Text(" ");

            for (int n = 0; n < 12; n++)
            {
                string datetext = month[n];

                ImGui::SameLine((float)barwidth* (n + 1) - (ImGui::CalcTextSize(datetext.c_str()).x) / 2);

                ImGui::Text(datetext.c_str());
            }

            ImGui::End();
        }

        // Global precipitation chart, if being shown

        if (showglobalrainfallchart)
        {
            float barwidth = 40.0f;

            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 680, main_viewport->WorkPos.y + 246), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(518, 139), ImGuiCond_FirstUseEver);

            ImGui::Begin("Precipitation", NULL, window_flags);

            float rain[12];

            rain[0] = (float)world->janrain(poix, poiy);
            rain[3] = (float)world->aprrain(poix, poiy);
            rain[6] = (float)world->julrain(poix, poiy);
            rain[9] = (float)world->octrain(poix, poiy);

            rain[1] = (rain[0] * 2.0f + rain[3]) / 3.0f;
            rain[2] = (rain[0] + rain[3] * 2.0f) / 3.0f;
            rain[4] = (rain[3] * 2.0f + rain[6]) / 3.0f;
            rain[5] = (rain[3] + rain[6] * 2.0f) / 3.0f;
            rain[7] = (rain[6] * 2.0f + rain[9]) / 3.0f;
            rain[8] = (rain[6] + rain[9] * 2.0f) / 3.0f;
            rain[10] = (rain[9] * 2.0f + rain[0]) / 3.0f;
            rain[11] = (rain[9] + rain[0] * 2.0f) / 3.0f;

            float lowest = rain[0];
            float highest = rain[0];

            if (rain[3] < lowest)
                lowest = rain[3];

            if (rain[3] > highest)
                highest = rain[3];

            if (rain[6] < lowest)
                lowest = rain[6];

            if (rain[6] > highest)
                highest = rain[6];

            if (rain[9] < lowest)
                lowest = rain[9];

            if (rain[9] > highest)
                highest = rain[9];

            float rainrange = highest - lowest;

            for (int n = 0; n < 13; n++)
            {
                ImGui::SameLine((float)barwidth * (float)n + barwidth / 2.0f);

                float value;

                if (n < 12)
                    value = rain[n];
                else
                    value = 0.0f;

                float rainarr[] = { value / highest };

                float colour1 = 0.0f;
                float colour2 = 0.0f;
                float colour3 = 0.0f;

                float rainfall = rain[n];
                rainfall = rainfall / 2.0f;

                colour1 = 255.0f - rainfall;
                colour2 = 255.0f - rainfall;
                colour3 = 255.0f;

                if (colour1 < 0.0f)
                    colour1 = 0.0f;

                if (colour2 < 0.0f)
                    colour2 = 0.0f;

                colour1 = colour1 / 255.0f;
                colour2 = colour2 / 255.0f;
                colour3 = colour3 / 255.0f;

                Style.Colors[ImGuiCol_PlotHistogram] = ImVec4(colour1, colour2, colour3, 1.00f);
                Style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(colour1, colour2, colour3, 1.00f);

                ImGui::PlotHistogram(" ", rainarr, IM_ARRAYSIZE(rainarr), 0, NULL, 0.0f, 1.0f, ImVec2(0.0f, barwidth));
            }

            ImGui::Text(" ");

            for (int n = 0; n < 12; n++)
            {
                string raininfotext = formatnumber((int)rain[n]);

                ImGui::SameLine((float)barwidth* (n + 1) - (ImGui::CalcTextSize(raininfotext.c_str()).x) / 2);

                ImGui::Text(raininfotext.c_str());
            }

            ImGui::Text(" ");

            for (int n = 0; n < 12; n++)
            {
                string datetext = month[n];

                ImGui::SameLine((float)barwidth * (n + 1) - (ImGui::CalcTextSize(datetext.c_str()).x) / 2);

                ImGui::Text(datetext.c_str());
            }

            ImGui::End();
        }

        // Regional temperature chart, if being shown

        if (showregionaltemperaturechart)
        {
            float barwidth = 40.0f;

            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 680, main_viewport->WorkPos.y + 385), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(518, 139), ImGuiCond_FirstUseEver);

            ImGui::Begin("Temperature##regional", NULL, window_flags);

            int xx = region->leftx() + poix / 16;
            int yy = region->lefty() + poiy / 16; // Coordinates of the gobal cell we're in.

            float temp[12];

            temp[0] = (float)region->jantemp(poix, poiy);
            temp[3] = (float)region->aprtemp(poix, poiy, yy, world->height(), world->tilt(), world->eccentricity(), world->perihelion());
            temp[6] = (float)region->jultemp(poix, poiy);
            temp[9] = temp[3]; // October is the same as April.

            temp[1] = (temp[0] * 2.0f + temp[3]) / 3.0f;
            temp[2] = (temp[0] + temp[3] * 2.0f) / 3.0f;
            temp[4] = (temp[3] * 2.0f + temp[6]) / 3.0f;
            temp[5] = (temp[3] + temp[6] * 2.0f) / 3.0f;
            temp[7] = (temp[6] * 2.0f + temp[9]) / 3.0f;
            temp[8] = (temp[6] + temp[9] * 2.0f) / 3.0f;
            temp[10] = (temp[9] * 2.0f + temp[0]) / 3.0f;
            temp[11] = (temp[9] + temp[0] * 2.0f) / 3.0f;

            float lowest = temp[0];
            float highest = temp[0];

            if (temp[3] < lowest)
                lowest = temp[3];

            if (temp[3] > highest)
                highest = temp[3];

            if (temp[6] < lowest)
                lowest = temp[6];

            if (temp[6] > highest)
                highest = temp[6];

            if (temp[9] < lowest)
                lowest = temp[9];

            if (temp[9] > highest)
                highest = temp[9];

            float subzero = 0.0f;

            if (lowest < 0.0f) // Ensure that all values are at least 0
            {
                subzero = 0.0f - lowest;

                for (int n = 0; n < 12; n++)
                    temp[n] = temp[n] + subzero;

                lowest = temp[0];
                highest = temp[0];

                if (temp[3] < lowest)
                    lowest = temp[3];

                if (temp[3] > highest)
                    highest = temp[3];

                if (temp[6] < lowest)
                    lowest = temp[6];

                if (temp[6] > highest)
                    highest = temp[6];

                if (temp[9] < lowest)
                    lowest = temp[9];

                if (temp[9] > highest)
                    highest = temp[9];
            }

            float temprange = highest - lowest;

            for (int n = 0; n < 13; n++)
            {
                ImGui::SameLine((float)barwidth * (float)n + barwidth / 2.0f);

                float value;

                if (n < 12)
                    value = temp[n];
                else
                    value = 0.0f;

                float temparr[] = { value / highest };

                float colour1 = 0.0f;
                float colour2 = 0.0f;
                float colour3 = 0.0f;

                float temperature = value - subzero;

                if (temperature > 0.0f)
                {
                    colour1 = 250.0f;
                    colour2 = 250.0f - temperature * 3.0f;
                    colour3 = 250.0f - temperature * 7.0f;
                }
                else
                {
                    temperature = abs(temperature);

                    colour1 = 250.0f - temperature * 7.0f;
                    colour2 = 250.0f - temperature * 7.0f;
                    colour3 = 250.0f;
                }

                if (colour1 < 0.0f)
                    colour1 = 0.0f;

                if (colour2 < 0.0f)
                    colour2 = 0.0f;

                if (colour3 < 0.0f)
                    colour3 = 0.0f;

                colour1 = colour1 / 255.0f;
                colour2 = colour2 / 255.0f;
                colour3 = colour3 / 255.0f;

                Style.Colors[ImGuiCol_PlotHistogram] = ImVec4(colour1, colour2, colour3, 1.00f);
                Style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(colour1, colour2, colour3, 1.00f);

                ImGui::PlotHistogram(" ", temparr, IM_ARRAYSIZE(temparr), 0, NULL, 0.0f, 1.0f, ImVec2(0.0f, barwidth));
            }

            ImGui::Text(" ");

            for (int n = 0; n < 12; n++)
            {
                string tempinfotext = formatnumber((int)temp[n] - (int)subzero);

                ImGui::SameLine((float)barwidth * (n + 1) - (ImGui::CalcTextSize(tempinfotext.c_str()).x) / 2);

                ImGui::Text(tempinfotext.c_str());
            }

            ImGui::Text(" ");

            for (int n = 0; n < 12; n++)
            {
                string datetext = month[n];

                ImGui::SameLine((float)barwidth* (n + 1) - (ImGui::CalcTextSize(datetext.c_str()).x) / 2);

                ImGui::Text(datetext.c_str());
            }

            ImGui::End();
        }

        // Regional precipitation chart, if being shown

        if (showregionalrainfallchart)
        {
            float barwidth = 40.0f;

            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 680, main_viewport->WorkPos.y + 246), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(518, 139), ImGuiCond_FirstUseEver);

            ImGui::Begin("Precipitation##regional", NULL, window_flags);

            float rain[12];

            rain[0] = (float)region->janrain(poix, poiy);
            rain[3] = (float)region->aprrain(poix, poiy);
            rain[6] = (float)region->julrain(poix, poiy);
            rain[9] = (float)region->octrain(poix, poiy);

            rain[1] = (rain[0] * 2.0f + rain[3]) / 3.0f;
            rain[2] = (rain[0] + rain[3] * 2.0f) / 3.0f;
            rain[4] = (rain[3] * 2.0f + rain[6]) / 3.0f;
            rain[5] = (rain[3] + rain[6] * 2.0f) / 3.0f;
            rain[7] = (rain[6] * 2.0f + rain[9]) / 3.0f;
            rain[8] = (rain[6] + rain[9] * 2.0f) / 3.0f;
            rain[10] = (rain[9] * 2.0f + rain[0]) / 3.0f;
            rain[11] = (rain[9] + rain[0] * 2.0f) / 3.0f;

            float lowest = rain[0];
            float highest = rain[0];

            if (rain[3] < lowest)
                lowest = rain[3];

            if (rain[3] > highest)
                highest = rain[3];

            if (rain[6] < lowest)
                lowest = rain[6];

            if (rain[6] > highest)
                highest = rain[6];

            if (rain[9] < lowest)
                lowest = rain[9];

            if (rain[9] > highest)
                highest = rain[9];

            float rainrange = highest - lowest;

            for (int n = 0; n < 13; n++)
            {
                ImGui::SameLine((float)barwidth * (float)n + barwidth / 2.0f);

                float value;

                if (n < 12)
                    value = rain[n];
                else
                    value = 0.0f;

                float rainarr[] = { value / highest };

                float colour1 = 0.0f;
                float colour2 = 0.0f;
                float colour3 = 0.0f;

                float rainfall = rain[n];
                rainfall = rainfall / 2.0f;

                colour1 = 255.0f - rainfall;
                colour2 = 255.0f - rainfall;
                colour3 = 255.0f;

                if (colour1 < 0.0f)
                    colour1 = 0.0f;

                if (colour2 < 0.0f)
                    colour2 = 0.0f;

                colour1 = colour1 / 255.0f;
                colour2 = colour2 / 255.0f;
                colour3 = colour3 / 255.0f;

                Style.Colors[ImGuiCol_PlotHistogram] = ImVec4(colour1, colour2, colour3, 1.00f);
                Style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(colour1, colour2, colour3, 1.00f);

                ImGui::PlotHistogram(" ", rainarr, IM_ARRAYSIZE(rainarr), 0, NULL, 0.0f, 1.0f, ImVec2(0.0f, barwidth));
            }

            ImGui::Text(" ");

            for (int n = 0; n < 12; n++)
            {
                string raininfotext = formatnumber((int)rain[n]);

                ImGui::SameLine((float)barwidth * (n+1)- (ImGui::CalcTextSize(raininfotext.c_str()).x) / 2);

                ImGui::Text(raininfotext.c_str());
            }

            ImGui::Text(" ");

            for (int n = 0; n < 12; n++)
            {
                string datetext = month[n];

                ImGui::SameLine((float)barwidth * (n + 1) - (ImGui::CalcTextSize(datetext.c_str()).x) / 2);

                ImGui::Text(datetext.c_str());
            }

            ImGui::End();
        }

        // Now draw the graphical elements.

        if (screenmode == globalmapscreen || screenmode == exportareascreen || screenmode == importscreen)
            window.draw(*globalmap);

        if (screenmode == exportareascreen) // Area selection rectangle.
        {
            if (areaswx != -1)
            {
                float mult = ((float)world->width() + 1.0f) / (float)DISPLAYMAPSIZEX;
                
                float sizex = ((float)areanex - (float)areanwx) / mult;
                float sizey = ((float)areaswy - (float)areanwy) / mult;

                arearectangle->setSize(sf::Vector2f(sizex, sizey));
                arearectangle->setOutlineColor(sf::Color(world->highlight1(), world->highlight2(), world->highlight3()));

                sf::Vector2f position;

                position.x = (float)areanwx / mult + (float)globalmapxpos;
                position.y = (float)areanwy / mult + (float)globalmapypos;

                arearectangle->setPosition(position);

                window.draw(*arearectangle);
            }
        }

        if (screenmode == regionalmapscreen || screenmode == generatingregionscreen)
        {
            window.draw(*regionalmap);

            window.draw(*minimap);

            float mult = (float)DISPLAYMAPSIZEX / ((float)world->width() + 1.0f);
            mult = mult * 0.5f;

            float posx = (float)minimapxpos + (float)region->centrex() * mult;
            float posy = (float)minimapypos + (float)region->centrey() * mult;

            minihighlight->setPosition(sf::Vector2f(posx, posy));

            if (screenmode != generatingregionscreen)
            {
                window.draw(*minihighlight);
            }
        }

        if (screenmode == globalmapscreen || screenmode == regionalmapscreen)
        {
            if (focused == 1)
                window.draw(*highlight);
        }

        // Now draw file dialogues if needed

        if (loadingworld == 1 && ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) // If we're loading in a new world
        {
            filepathname = "";
            filepath = "";
            
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                filepathname = ImGuiFileDialog::Instance()->GetFilePathName();
                filepath = ImGuiFileDialog::Instance()->GetCurrentPath();

                if (filepathname != "")
                {
                    screenmode = loadingworldscreen;                  
                }

            }
            ImGuiFileDialog::Instance()->Close();
        }

        if (savingworld == 1 && ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) // If we're saving a world
        {
            filepathname = "";
            filepath = "";
            
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                filepathname = ImGuiFileDialog::Instance()->GetFilePathName();
                filepath = ImGuiFileDialog::Instance()->GetCurrentPath();

                if (filepathname != "")
                {
                    screenmode = savingworldscreen;
                }
            }
            ImGuiFileDialog::Instance()->Close();
        }

        if (loadingsettings == 1 && ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) // If we're loading in new settings
        {
            filepathname = "";
            filepath = "";
            
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string filepathname = ImGuiFileDialog::Instance()->GetFilePathName();
                std::string filepath = ImGuiFileDialog::Instance()->GetCurrentPath();

                if (filepathname != "")
                {
                    bool found = loadsettings(*world, filepathname);

                    if (found == 1)
                    {
                        //  Put all the correct values into the settings.

                        oceancolour.x = (float)world->ocean1() / 255.f;
                        oceancolour.y = (float)world->ocean2() / 255.f;
                        oceancolour.z = (float)world->ocean3() / 255.f;
                        oceancolour.w = 1.f;

                        deepoceancolour.x = (float)world->deepocean1() / 255.f;
                        deepoceancolour.y = (float)world->deepocean2() / 255.f;
                        deepoceancolour.z = (float)world->deepocean3() / 255.f;
                        deepoceancolour.w = 1.f;

                        basecolour.x = (float)world->base1() / 255.f;
                        basecolour.y = (float)world->base2() / 255.f;
                        basecolour.z = (float)world->base3() / 255.f;
                        basecolour.w = 1.f;

                        grasscolour.x = (float)world->grass1() / 255.f;
                        grasscolour.y = (float)world->grass2() / 255.f;
                        grasscolour.z = (float)world->grass3() / 255.f;
                        grasscolour.w = 1.f;

                        basetempcolour.x = (float)world->basetemp1() / 255.f;
                        basetempcolour.y = (float)world->basetemp2() / 255.f;
                        basetempcolour.z = (float)world->basetemp3() / 255.f;
                        basetempcolour.w = 1.f;

                        highbasecolour.x = (float)world->highbase1() / 255.f;
                        highbasecolour.y = (float)world->highbase2() / 255.f;
                        highbasecolour.z = (float)world->highbase3() / 255.f;
                        highbasecolour.w = 1.f;

                        desertcolour.x = (float)world->desert1() / 255.f;
                        desertcolour.y = (float)world->desert2() / 255.f;
                        desertcolour.z = (float)world->desert3() / 255.f;
                        desertcolour.w = 1.f;

                        highdesertcolour.x = (float)world->highdesert1() / 255.f;
                        highdesertcolour.y = (float)world->highdesert2() / 255.f;
                        highdesertcolour.z = (float)world->highdesert3() / 255.f;
                        highdesertcolour.w = 1.f;

                        colddesertcolour.x = (float)world->colddesert1() / 255.f;
                        colddesertcolour.y = (float)world->colddesert2() / 255.f;
                        colddesertcolour.z = (float)world->colddesert3() / 255.f;
                        colddesertcolour.w = 1.f;

                        eqtundracolour.x = (float)world->eqtundra1() / 255.f;
                        eqtundracolour.y = (float)world->eqtundra2() / 255.f;
                        eqtundracolour.z = (float)world->eqtundra3() / 255.f;
                        eqtundracolour.w = 1.f;

                        tundracolour.x = (float)world->tundra1() / 255.f;
                        tundracolour.y = (float)world->tundra2() / 255.f;
                        tundracolour.z = (float)world->tundra3() / 255.f;
                        tundracolour.w = 1.f;

                        coldcolour.x = (float)world->cold1() / 255.f;
                        coldcolour.y = (float)world->cold2() / 255.f;
                        coldcolour.z = (float)world->cold3() / 255.f;
                        coldcolour.w = 1.f;

                        seaicecolour.x = (float)world->seaice1() / 255.f;
                        seaicecolour.y = (float)world->seaice2() / 255.f;
                        seaicecolour.z = (float)world->seaice3() / 255.f;
                        seaicecolour.w = 1.f;

                        glaciercolour.x = (float)world->glacier1() / 255.f;
                        glaciercolour.y = (float)world->glacier2() / 255.f;
                        glaciercolour.z = (float)world->glacier3() / 255.f;
                        glaciercolour.w = 1.f;

                        saltpancolour.x = (float)world->saltpan1() / 255.f;
                        saltpancolour.y = (float)world->saltpan2() / 255.f;
                        saltpancolour.z = (float)world->saltpan3() / 255.f;
                        saltpancolour.w = 1.f;

                        ergcolour.x = (float)world->erg1() / 255.f;
                        ergcolour.y = (float)world->erg2() / 255.f;
                        ergcolour.z = (float)world->erg3() / 255.f;
                        ergcolour.w = 1.f;

                        wetlandscolour.x = (float)world->wetlands1() / 255.f;
                        wetlandscolour.y = (float)world->wetlands2() / 255.f;
                        wetlandscolour.z = (float)world->wetlands3() / 255.f;
                        wetlandscolour.w = 1.f;

                        lakecolour.x = (float)world->lake1() / 255.f;
                        lakecolour.y = (float)world->lake2() / 255.f;
                        lakecolour.z = (float)world->lake3() / 255.f;
                        lakecolour.w = 1.f;

                        rivercolour.x = (float)world->river1() / 255.f;
                        rivercolour.y = (float)world->river2() / 255.f;
                        rivercolour.z = (float)world->river3() / 255.f;
                        rivercolour.w = 1.f;

                        mudcolour.x = (float)world->mud1() / 255.f;
                        mudcolour.y = (float)world->mud2() / 255.f;
                        mudcolour.z = (float)world->mud3() / 255.f;
                        mudcolour.w = 1.f;

                        sandcolour.x = (float)world->sand1() / 255.f;
                        sandcolour.y = (float)world->sand2() / 255.f;
                        sandcolour.z = (float)world->sand3() / 255.f;
                        sandcolour.w = 1.f;

                        shinglecolour.x = (float)world->shingle1() / 255.f;
                        shinglecolour.y = (float)world->shingle2() / 255.f;
                        shinglecolour.z = (float)world->shingle3() / 255.f;
                        shinglecolour.w = 1.f;

                        mangrovecolour.x = (float)world->mangrove1() / 255.f;
                        mangrovecolour.y = (float)world->mangrove2() / 255.f;
                        mangrovecolour.z = (float)world->mangrove3() / 255.f;
                        mangrovecolour.w = 1.f;

                        highlightcolour.x = (float)world->highlight1() / 255.f;
                        highlightcolour.y = (float)world->highlight2() / 255.f;
                        highlightcolour.z = (float)world->highlight3() / 255.f;
                        highlightcolour.w = 1.f;

                        shadingland = world->landshading();
                        shadinglake = world->lakeshading();
                        shadingsea = world->seashading();
                        marblingland = world->landmarbling();
                        marblinglake = world->lakemarbling();
                        marblingsea = world->seamarbling();

                        minriverflowglobal = world->minriverflowglobal();
                        minriverflowregional = world->minriverflowregional();

                        globalriversentry = world->minriverflowglobal();
                        regionalriversentry = world->minriverflowregional();

                        if (world->shadingdir() == 4)
                            shadingdir = 0;

                        if (world->shadingdir() == 6)
                            shadingdir = 1;

                        if (world->shadingdir() == 2)
                            shadingdir = 2;

                        if (world->shadingdir() == 8)
                            shadingdir = 3;

                        snowchange = world->snowchange() - 1;
                        seaiceappearance = world->seaiceappearance() - 1;
                        colourcliffs = world->colourcliffs();
                        mangroves = world->showmangroves();

                        drawhighlightobjects(*world, *highlightimage, highlightsize, *minihighlightimage, minihighlightsize);

                        highlighttexture->loadFromImage(*highlightimage);
                        highlight->setTexture(*highlighttexture);

                        minihighlighttexture->loadFromImage(*minihighlightimage);
                        minihighlight->setTexture(*minihighlighttexture);
                        minihighlight->setOrigin((float)minihighlightsize / 2.0f, (float)minihighlightsize / 2.0f);

                        colourschanged = 1;
                        loadingsettings = 0;
                    }
                    else
                    {
                        oldscreenmode = screenmode;
                        screenmode = settingsloadfailure;
                    }
                }
            }
            ImGuiFileDialog::Instance()->Close();
        }

        if (savingsettings == 1 && ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) // If we're saving a world
        {
            filepathname = "";
            filepath = "";
            
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string filepathname = ImGuiFileDialog::Instance()->GetFilePathName();
                std::string filepath = ImGuiFileDialog::Instance()->GetCurrentPath();

                if (filepathname != "")
                {
                    savesettings(*world, filepathname);

                    savingworld = 0;
                }
            }
            ImGuiFileDialog::Instance()->Close();
        }

        if (exportingworldmaps == 1 && ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) // If we're exporting world maps
        {
            filepathname = "";
            filepath = "";
            
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string filepathname = ImGuiFileDialog::Instance()->GetFilePathName();
                std::string filepath = ImGuiFileDialog::Instance()->GetCurrentPath();

                if (filepathname != "")
                {
                    // Now, draw all the maps.

                    mapviewenum oldmapview = mapview;

                    mapview = relief;

                    drawglobalmapimage(mapview, *world, globalmapimagecreated, *globalelevationimage, *globaltemperatureimage, *globalprecipitationimage, *globalclimateimage, *globalriversimage, *globalreliefimage, *displayglobalelevationimage, *displayglobaltemperatureimage, *displayglobalprecipitationimage, *displayglobalclimateimage, *displayglobalriversimage, *displayglobalreliefimage);

                    mapview = elevation;

                    drawglobalmapimage(mapview, *world, globalmapimagecreated, *globalelevationimage, *globaltemperatureimage, *globalprecipitationimage, *globalclimateimage, *globalriversimage, *globalreliefimage, *displayglobalelevationimage, *displayglobaltemperatureimage, *displayglobalprecipitationimage, *displayglobalclimateimage, *displayglobalriversimage, *displayglobalreliefimage);

                    mapview = temperature;

                    drawglobalmapimage(mapview, *world, globalmapimagecreated, *globalelevationimage, *globaltemperatureimage, *globalprecipitationimage, *globalclimateimage, *globalriversimage, *globalreliefimage, *displayglobalelevationimage, *displayglobaltemperatureimage, *displayglobalprecipitationimage, *displayglobalclimateimage, *displayglobalriversimage, *displayglobalreliefimage);

                    mapview = precipitation;

                    drawglobalmapimage(mapview, *world, globalmapimagecreated, *globalelevationimage, *globaltemperatureimage, *globalprecipitationimage, *globalclimateimage, *globalriversimage, *globalreliefimage, *displayglobalelevationimage, *displayglobaltemperatureimage, *displayglobalprecipitationimage, *displayglobalclimateimage, *displayglobalriversimage, *displayglobalreliefimage);

                    mapview = climate;

                    drawglobalmapimage(mapview, *world, globalmapimagecreated, *globalelevationimage, *globaltemperatureimage, *globalprecipitationimage, *globalclimateimage, *globalriversimage, *globalreliefimage, *displayglobalelevationimage, *displayglobaltemperatureimage, *displayglobalprecipitationimage, *displayglobalclimateimage, *displayglobalriversimage, *displayglobalreliefimage);

                    mapview = rivers;

                    drawglobalmapimage(mapview, *world, globalmapimagecreated, *globalelevationimage, *globaltemperatureimage, *globalprecipitationimage, *globalclimateimage, *globalriversimage, *globalreliefimage, *displayglobalelevationimage, *displayglobaltemperatureimage, *displayglobalprecipitationimage, *displayglobalclimateimage, *displayglobalriversimage, *displayglobalreliefimage);

                    // Now save them.

                    filepathname.resize(filepathname.size() - 4);

                    globalreliefimage->saveToFile(filepathname + " Relief.png");
                    globalelevationimage->saveToFile(filepathname + " Elevation.png");
                    globaltemperatureimage->saveToFile(filepathname + " Temperature.png");
                    globalprecipitationimage->saveToFile(filepathname + " Precipitation.png");
                    globalclimateimage->saveToFile(filepathname + " Climate.png");
                    globalriversimage->saveToFile(filepathname + " Rivers.png");

                    mapview = oldmapview;

                    exportingworldmaps = 0;
                }
            }
            ImGuiFileDialog::Instance()->Close();
        }

        if (exportingregionalmaps == 1 && ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) // If we're exporting world maps
        {
            filepathname = "";
            filepath = "";
            
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string filepathname = ImGuiFileDialog::Instance()->GetFilePathName();
                std::string filepath = ImGuiFileDialog::Instance()->GetCurrentPath();

                if (filepathname != "")
                {
                    // Now, draw all the maps.

                    mapviewenum oldmapview = mapview;

                    mapview = relief;

                    drawregionalmapimage(mapview, *world, *region, regionalmapimagecreated, *regionalelevationimage, *regionaltemperatureimage, *regionalprecipitationimage, *regionalclimateimage, *regionalriversimage, *regionalreliefimage);

                    mapview = elevation;

                    drawregionalmapimage(mapview, *world, *region, regionalmapimagecreated, *regionalelevationimage, *regionaltemperatureimage, *regionalprecipitationimage, *regionalclimateimage, *regionalriversimage, *regionalreliefimage);

                    mapview = temperature;

                    drawregionalmapimage(mapview, *world, *region, regionalmapimagecreated, *regionalelevationimage, *regionaltemperatureimage, *regionalprecipitationimage, *regionalclimateimage, *regionalriversimage, *regionalreliefimage);

                    mapview = precipitation;

                    drawregionalmapimage(mapview, *world, *region, regionalmapimagecreated, *regionalelevationimage, *regionaltemperatureimage, *regionalprecipitationimage, *regionalclimateimage, *regionalriversimage, *regionalreliefimage);

                    mapview = climate;

                    drawregionalmapimage(mapview, *world, *region, regionalmapimagecreated, *regionalelevationimage, *regionaltemperatureimage, *regionalprecipitationimage, *regionalclimateimage, *regionalriversimage, *regionalreliefimage);

                    mapview = rivers;

                    drawregionalmapimage(mapview, *world, *region, regionalmapimagecreated, *regionalelevationimage, *regionaltemperatureimage, *regionalprecipitationimage, *regionalclimateimage, *regionalriversimage, *regionalreliefimage);

                    // Now save them.

                    filepathname.resize(filepathname.size() - 4);

                    regionalreliefimage->saveToFile(filepathname + " Relief.png");
                    regionalelevationimage->saveToFile(filepathname + " Elevation.png");
                    regionaltemperatureimage->saveToFile(filepathname + " Temperature.png");
                    regionalprecipitationimage->saveToFile(filepathname + " Precipitation.png");
                    regionalclimateimage->saveToFile(filepathname + " Climate.png");
                    regionalriversimage->saveToFile(filepathname + " Rivers.png");

                    mapview = oldmapview;

                    exportingregionalmaps = 0;
                }
            }
            ImGuiFileDialog::Instance()->Close();
        }

        if (exportingareamaps == 1 && ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) // If we're exporting area maps
        {
            filepathname = "";
            filepath = "";
            
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                filepathname = ImGuiFileDialog::Instance()->GetFilePathName();
                filepath = ImGuiFileDialog::Instance()->GetCurrentPath();

                if (filepathname != "")
                {
                    screenmode = exportingareascreen;
                }
            }
            ImGuiFileDialog::Instance()->Close();
        }

        // These sections are for loading in maps in the import screen.

        if (importinglandmap == 1 && ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) // If we're importing a land map
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string filepathname = ImGuiFileDialog::Instance()->GetFilePathName();
                std::string filepath = ImGuiFileDialog::Instance()->GetCurrentPath();

                if (filepathname != "")
                {
                    int width = world->width();
                    int height = world->height();
                    int sealevel = world->sealevel();

                    sf::Image* importimage = new sf::Image;

                    importimage->loadFromFile(filepathname);

                    sf::Vector2u imagesize = importimage->getSize();

                    if (imagesize.x == width + 1 && imagesize.y == height + 1)
                    {
                        // First, load the elevations into the world.

                        for (int i = 0; i <= width; i++)
                        {
                            for (int j = 0; j <= height; j++)
                            {
                                sf::Color colour = importimage->getPixel(i, j);

                                if (colour.r != 0)
                                {
                                    int elev = colour.r * 10 + sealevel;

                                    world->setnom(i, j, elev);
                                }
                                else
                                    world->setnom(i, j, sealevel - 5000);
                            }
                        }

                        // Now redraw the map.

                        for (int n = 0; n < GLOBALMAPTYPES; n++)
                            globalmapimagecreated[n] = 0;

                        mapview = relief;
                        drawglobalmapimage(mapview, *world, globalmapimagecreated, *globalelevationimage, *globaltemperatureimage, *globalprecipitationimage, *globalclimateimage, *globalriversimage, *globalreliefimage, *displayglobalelevationimage, *displayglobaltemperatureimage, *displayglobalprecipitationimage, *displayglobalclimateimage, *displayglobalriversimage, *displayglobalreliefimage);

                        globalmaptexture->loadFromImage(*displayglobalreliefimage);
                        globalmap->setTexture(*globalmaptexture);

                        importinglandmap = 0;
                    }
                }
            }
            ImGuiFileDialog::Instance()->Close();
        }

        if (importingseamap == 1 && ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) // If we're importing a sea map
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string filepathname = ImGuiFileDialog::Instance()->GetFilePathName();
                std::string filepath = ImGuiFileDialog::Instance()->GetCurrentPath();

                if (filepathname != "")
                {
                    int width = world->width();
                    int height = world->height();
                    int sealevel = world->sealevel();

                    sf::Image* importimage = new sf::Image;

                    importimage->loadFromFile(filepathname);

                    sf::Vector2u imagesize = importimage->getSize();

                    if (imagesize.x == width + 1 && imagesize.y == height + 1)
                    {
                        // First, load the elevations into the world.

                        for (int i = 0; i <= width; i++)
                        {
                            for (int j = 0; j <= height; j++)
                            {
                                sf::Color colour = importimage->getPixel(i, j);

                                if (colour.r != 0)
                                {
                                    int elev = sealevel - colour.r * 50;

                                    if (elev < 1)
                                        elev = 1;

                                    world->setnom(i, j, elev);
                                }
                            }
                        }

                        // Now redraw the map.

                        for (int n = 0; n < GLOBALMAPTYPES; n++)
                            globalmapimagecreated[n] = 0;

                        mapview = relief;
                        drawglobalmapimage(mapview, *world, globalmapimagecreated, *globalelevationimage, *globaltemperatureimage, *globalprecipitationimage, *globalclimateimage, *globalriversimage, *globalreliefimage, *displayglobalelevationimage, *displayglobaltemperatureimage, *displayglobalprecipitationimage, *displayglobalclimateimage, *displayglobalriversimage, *displayglobalreliefimage);

                        globalmaptexture->loadFromImage(*displayglobalreliefimage);
                        globalmap->setTexture(*globalmaptexture);

                        importingseamap = 0;
                    }
                }
            }
            ImGuiFileDialog::Instance()->Close();
        }

        if (importingmountainsmap == 1 && ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) // If we're importing a mountains map
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string filepathname = ImGuiFileDialog::Instance()->GetFilePathName();
                std::string filepath = ImGuiFileDialog::Instance()->GetCurrentPath();

                if (filepathname != "")
                {
                    int width = world->width();
                    int height = world->height();
                    int sealevel = world->sealevel();

                    sf::Image* importimage = new sf::Image;

                    importimage->loadFromFile(filepathname);

                    sf::Vector2u imagesize = importimage->getSize();

                    if (imagesize.x == width + 1 && imagesize.y == height + 1)
                    {
                        // First, create an array for the raw mountain heights, and load them in.

                        vector<vector<int>> rawmountains(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

                        for (int i = 0; i <= width; i++)
                        {
                            for (int j = 0; j <= height; j++)
                            {
                                sf::Color colour = importimage->getPixel(i, j);

                                if (colour.r != 0)
                                {
                                    int elev = colour.r * 65; // In theory *50, but this seems to result in roughly the right heights.

                                    rawmountains[i][j] = elev;
                                }
                                else
                                    rawmountains[i][j] = 0;
                            }
                        }

                        // Now turn the raw mountains array into actual mountains.

                        createmountainsfromraw(*world, rawmountains, OKmountains);

                        // Now redraw the map.

                        for (int n = 0; n < GLOBALMAPTYPES; n++)
                            globalmapimagecreated[n] = 0;

                        mapview = relief;
                        drawglobalmapimage(mapview, *world, globalmapimagecreated, *globalelevationimage, *globaltemperatureimage, *globalprecipitationimage, *globalclimateimage, *globalriversimage, *globalreliefimage, *displayglobalelevationimage, *displayglobaltemperatureimage, *displayglobalprecipitationimage, *displayglobalclimateimage, *displayglobalriversimage, *displayglobalreliefimage);

                        globalmaptexture->loadFromImage(*displayglobalreliefimage);
                        globalmap->setTexture(*globalmaptexture);

                        importingmountainsmap = 0;
                    }
                }
            }
            ImGuiFileDialog::Instance()->Close();
        }

        if (importingvolcanoesmap == 1 && ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) // If we're importing a volcanoes map
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string filepathname = ImGuiFileDialog::Instance()->GetFilePathName();
                std::string filepath = ImGuiFileDialog::Instance()->GetCurrentPath();

                if (filepathname != "")
                {
                    int width = world->width();
                    int height = world->height();
                    int sealevel = world->sealevel();

                    sf::Image* importimage = new sf::Image;

                    importimage->loadFromFile(filepathname);

                    sf::Vector2u imagesize = importimage->getSize();

                    if (imagesize.x == width + 1 && imagesize.y == height + 1)
                    {
                        // First, load the volcanoes into the world.

                        for (int i = 0; i <= width; i++)
                        {
                            for (int j = 0; j <= height; j++)
                            {
                                sf::Color colour = importimage->getPixel(i, j);

                                if (colour.r != 0)
                                {
                                    int elev = colour.r * 45; // In theory *50, but this seems to give more accurate results.

                                    bool strato = 0;

                                    if (colour.g > 0)
                                    {
                                        strato = 1;
                                    }
                                    else
                                        elev = elev / 2; // Because shield volcanoes have a lot of extra elevation anyway.

                                    if (colour.b == 0)
                                        elev = 0 - elev;

                                    world->setvolcano(i, j, elev);
                                    world->setstrato(i, j, strato);
                                }
                            }
                        }

                        // Now redraw the map.

                        for (int n = 0; n < GLOBALMAPTYPES; n++)
                            globalmapimagecreated[n] = 0;

                        mapview = relief;
                        drawglobalmapimage(mapview, *world, globalmapimagecreated, *globalelevationimage, *globaltemperatureimage, *globalprecipitationimage, *globalclimateimage, *globalriversimage, *globalreliefimage, *displayglobalelevationimage, *displayglobaltemperatureimage, *displayglobalprecipitationimage, *displayglobalclimateimage, *displayglobalriversimage, *displayglobalreliefimage);

                        globalmaptexture->loadFromImage(*displayglobalreliefimage);
                        globalmap->setTexture(*globalmaptexture);

                        importingvolcanoesmap = 0;
                    }
                }
            }
            ImGuiFileDialog::Instance()->Close();
        }

        // Set size window, for custom worlds.

        if (showsetsize == 1)
        {
            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 427, main_viewport->WorkPos.y + 174), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(323, 152), ImGuiCond_FirstUseEver);

            ImGui::Begin("Create custom world?", NULL, window_flags);

            string introtext = "This will delete the current world.";

            if (brandnew)
                introtext = "Please select a size for the new world.";

            ImGui::Text(introtext.c_str());
            ImGui::Text(" ");

            const char* sizeitems[] = { "Small", "Medium", "Large"};
            static int pitem_current = 2;
            ImGui::Combo("World size", &currentsize, sizeitems, 3);

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Earth: large; Mars: medium; Moon: small.");

            ImGui::Text(" ");
            ImGui::Text(" ");

            ImGui::SameLine((float)100);

            if (ImGui::Button("OK"))
            {
                initialiseworld(*world);
                world->clear();

                world->setsize(currentsize);

                adjustforsize(*world, globaltexturesize, *globalelevationimage, *globaltemperatureimage, *globalprecipitationimage, *globalclimateimage, *globalriversimage, *globalreliefimage, *highlightimage, highlightsize, *minihighlightimage, minihighlightsize);

                minihighlighttexture->loadFromImage(*minihighlightimage);
                minihighlight->setTexture(*minihighlighttexture);
                minihighlight->setOrigin((float)minihighlightsize / 2.0f, (float)minihighlightsize / 2.0f);

                world->setgravity(1.0f); // Set initial gravity that roughly matches the world size.

                if (currentsize==1)
                    world->setgravity(0.4f);

                if (currentsize == 0)
                    world->setgravity(0.15f);

                landmass = 5;
                mergefactor = 15;
                currentsize = world->size();
                currentgravity = world->gravity();
                currentrotation = world->rotation();
                currenttilt = world->tilt();
                currenteccentricity = world->eccentricity();
                currentperihelion = world->perihelion();
                currentlunar = world->lunar();
                currenttempdecrease = world->tempdecrease();
                currentnorthpolaradjust = world->northpolaradjust();
                currentsouthpolaradjust = world->southpolaradjust();
                currentaveragetemp = world->averagetemp();
                currentwaterpickup = world->waterpickup();
                currentglacialtemp = world->glacialtemp();
                currentrivers = 1;
                currentlakes = 1;
                currentdeltas = 1;
                int width = world->width();
                int height = world->height();
                int val = world->sealevel() - 5000;

                for (int i = 0; i <= width; i++)
                {
                    for (int j = 0; j <= height; j++)
                        world->setnom(i, j, val);
                }

                mapview = relief;

                for (int n = 0; n < GLOBALMAPTYPES; n++)
                    globalmapimagecreated[n] = 0;

                drawglobalmapimage(mapview, *world, globalmapimagecreated, *globalelevationimage, *globaltemperatureimage, *globalprecipitationimage, *globalclimateimage, *globalriversimage, *globalreliefimage, *displayglobalelevationimage, *displayglobaltemperatureimage, *displayglobalprecipitationimage, *displayglobalclimateimage, *displayglobalriversimage, *displayglobalreliefimage);

                globalmaptexture->loadFromImage(*displayglobalreliefimage);
                globalmap->setTexture(*globalmaptexture);

                int seed = random(0, 9);

                for (int n = 1; n <= 7; n++)
                {
                    if (n == 7)
                        seed = seed + (random(1, 9) * (int)pow(10, n));
                    else
                        seed = seed + (random(0, 9) * (int)pow(10, n));
                }

                seed = 0 - seed;

                world->setseed(seed);

                screenmode = importscreen;
                showsetsize = 0;
            }

            ImGui::SameLine((float)180);

            if (ImGui::Button("Cancel"))
            {
                showsetsize = 0;
            }

            ImGui::End();
        }

        // Window for creating custom tectonic-based worlds.

        if (showtectonicchooser == 1)
        {
            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 428, main_viewport->WorkPos.y + 167), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(405, 200), ImGuiCond_FirstUseEver);

            ImGui::Begin("Generate world terrain", NULL, window_flags);

            /*
            ImVec2 pos = ImGui::GetWindowPos();

            cout << "Position: " << pos.x << ", " << pos.y << endl;

            ImVec2 size = ImGui::GetWindowSize();

            cout << "Size: " << size.x << ", " << size.y << endl;
            */

            ImGui::Text("This will overwrite any existing terrain.");
            ImGui::Text(" ");

            ImGui::SliderInt("Continental mass", &landmass, 0, 10);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("The (very approximate) amount of land mass, compared to sea.");

            ImGui::Text(" ");

            ImGui::SliderInt("Marine flooding", &mergefactor, 1, 30);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("The degree to which sea covers the continents. The higher this is, the more inland seas and fragmented coastlines there will be.");

            ImGui::Text(" ");
            ImGui::Text(" ");

            ImGui::SameLine((float)100);

            if (ImGui::Button("OK"))
            {
                screenmode = generatingtectonicscreen;
                generatingtectonicpass = 0;

                showtectonicchooser = 0;
            }

            ImGui::SameLine((float)270);

            if (ImGui::Button("Cancel"))
            {
                showtectonicchooser = 0;
            }

            ImGui::End();
        }

        // Window for creating custom non-tectonic-based worlds.

        if (shownontectonicchooser == 1)
        {
            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 428, main_viewport->WorkPos.y + 167), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(405, 200), ImGuiCond_FirstUseEver);

            ImGui::Begin("Generate world terrain##nontectonic", NULL, window_flags);

            ImGui::Text("This will overwrite any existing terrain.");
            ImGui::Text(" ");

            ImGui::SliderInt("Sea level", &sealeveleditable, 0, 10);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("The approximate sea level. The higher it is, the more sea (very roughly) there is likely to be.");

            ImGui::Text(" ");

            ImGui::SliderInt("Variation", &iterations, 1, 10);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("The amount of variation in terrain type. The higher this is, the more chaotic the terrain may become.");

            ImGui::Text(" ");
            ImGui::Text(" ");

            ImGui::SameLine((float)100);

            if (ImGui::Button("OK"))
            {
                screenmode = generatingnontectonicscreen;
                generatingnontectonicpass = 0;

                shownontectonicchooser = 0;
            }

            ImGui::SameLine((float)270);

            if (ImGui::Button("Cancel"))
            {
                shownontectonicchooser = 0;
            }

            ImGui::End();
        }

        // Warning window for over-large area maps.

        if (showareawarning == 1)
        {
            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 492, main_viewport->WorkPos.y + 156), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(218, 174), ImGuiCond_FirstUseEver);

            ImGui::Begin("Warning!##areawarning", NULL, window_flags);

            ImGui::Text("This is a very large area,");
            ImGui::Text("and may crash the program.");
            ImGui::Text(" ");
            ImGui::Text("Proceed?");

            ImGui::Text(" ");
            ImGui::Text(" ");

            ImGui::SameLine((float)40);

            if (ImGui::Button("OK"))
            {
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".png", ".");

                exportingareamaps = 1;

                showareawarning = 0;
            }

            ImGui::SameLine((float)130);

            if (ImGui::Button("Cancel"))
            {
                showareawarning = 0;
            }

            ImGui::End();
        }

        // Window for displaying information about the program.

        if (showabout == 1)
        {
            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 501, main_viewport->WorkPos.y + 111), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(343, 373), ImGuiCond_FirstUseEver);

            stringstream ss;

            ss << fixed << setprecision(2) << currentversion;

            string title = "Undiscovered Worlds version " + ss.str();

            ImGui::Begin(title.c_str(), NULL, window_flags);

            ImGui::Text("Undiscovered Worlds is designed and written");
            ImGui::Text("in inelegant C++ by Jonathan Hill. The");
            ImGui::Text("interface uses Dear ImGUI and SFML.");
            ImGui::Text(" ");            
            ImGui::Text("Special thanks to Frank Gennari for testing,");
            ImGui::Text("debugging, and advice, and also to");
            ImGui::Text("u/Iron-Phoenix2307 for the application icon.");
            ImGui::Text(" ");
            ImGui::Text("For more information and instructions for use,");
            ImGui::Text("please visit the website.");
            ImGui::Text(" ");
            ImGui::Text("The source code for this application is available");
            ImGui::Text("under the GNU General Public License.");
            ImGui::Text(" ");

            if (ImGui::Button("Website"))
            {
                ShellExecute(0, 0, L"https://undiscoveredworlds.blogspot.com/2019/01/what-is-undiscovered-worlds.html", 0, 0, SW_SHOW);

            }

            ImGui::SameLine((float)140);

            if (ImGui::Button("Source"))
            {
                ShellExecute(0, 0, L"https://github.com/JonathanCRH/Undiscovered_Worlds", 0, 0, SW_SHOW);

            }

            ImGui::SameLine((float)280);

            if (ImGui::Button("Close"))
            {
                showabout = 0;
            }

            ImGui::End();
        }

        ImGui::SFML::Render(window);
        window.display();

        // Now update the colours if necessary.

        short shadingdircorrected = 0;

        if (shadingdir == 0)
            shadingdircorrected = 4;

        if (shadingdir == 1)
            shadingdircorrected = 6;

        if (shadingdir == 2)
            shadingdircorrected = 2;

        if (shadingdir == 3)
            shadingdircorrected = 8;

        if (world->ocean1() != oceancolour.x * 255.f || world->ocean2() != oceancolour.y * 255.f || world->ocean3() != oceancolour.z * 255.f)
            colourschanged = 1;

        if (world->deepocean1() != deepoceancolour.x * 255.f || world->deepocean2() != deepoceancolour.y * 255.f || world->deepocean3() != deepoceancolour.z * 255.f)
            colourschanged = 1;

        if (world->base1() != basecolour.x * 255.f || world->base2() != basecolour.y * 255.f || world->base3() != basecolour.z * 255.f)
            colourschanged = 1;

        if (world->grass1() != grasscolour.x * 255.f || world->grass2() != grasscolour.y * 255.f || world->grass3() != grasscolour.z * 255.f)
            colourschanged = 1;

        if (world->basetemp1() != basetempcolour.x * 255.f || world->basetemp2() != basetempcolour.y * 255.f || world->basetemp3() != basetempcolour.z * 255.f)
            colourschanged = 1;

        if (world->highbase1() != highbasecolour.x * 255.f || world->highbase2() != highbasecolour.y * 255.f || world->highbase3() != highbasecolour.z * 255.f)
            colourschanged = 1;

        if (world->desert1() != desertcolour.x * 255.f || world->desert2() != desertcolour.y * 255.f || world->desert3() != desertcolour.z * 255.f)
            colourschanged = 1;

        if (world->highdesert1() != highdesertcolour.x * 255.f || world->highdesert2() != highdesertcolour.y * 255.f || world->highdesert3() != highdesertcolour.z * 255.f)
            colourschanged = 1;

        if (world->colddesert1() != colddesertcolour.x * 255.f || world->colddesert2() != colddesertcolour.y * 255.f || world->colddesert3() != colddesertcolour.z * 255.f)
            colourschanged = 1;

        if (world->eqtundra1() != eqtundracolour.x * 255.f || world->eqtundra2() != eqtundracolour.y * 255.f || world->eqtundra3() != eqtundracolour.z * 255.f)
            colourschanged = 1;

        if (world->tundra1() != tundracolour.x * 255.f || world->tundra2() != tundracolour.y * 255.f || world->tundra3() != tundracolour.z * 255.f)
            colourschanged = 1;

        if (world->cold1() != coldcolour.x * 255.f || world->cold2() != coldcolour.y * 255.f || world->cold3() != coldcolour.z * 255.f)
            colourschanged = 1;

        if (world->seaice1() != seaicecolour.x * 255.f || world->seaice2() != seaicecolour.y * 255.f || world->seaice3() != seaicecolour.z * 255.f)
            colourschanged = 1;

        if (world->glacier1() != glaciercolour.x * 255.f || world->glacier2() != glaciercolour.y * 255.f || world->glacier3() != glaciercolour.z * 255.f)
            colourschanged = 1;

        if (world->saltpan1() != saltpancolour.x * 255.f || world->saltpan1() != saltpancolour.y * 255.f || world->saltpan1() != saltpancolour.z * 255.f)
            colourschanged = 1;

        if (world->erg1() != ergcolour.x * 255.f || world->erg2() != ergcolour.y * 255.f || world->erg3() != ergcolour.z * 255.f)
            colourschanged = 1;

        if (world->wetlands1() != wetlandscolour.x * 255.f || world->wetlands2() != wetlandscolour.y * 255.f || world->wetlands3() != wetlandscolour.z * 255.f)
            colourschanged = 1;

        if (world->lake1() != lakecolour.x * 255.f || world->lake2() != lakecolour.y * 255.f || world->lake3() != lakecolour.z * 255.f)
            colourschanged = 1;

        if (world->river1() != rivercolour.x * 255.f || world->river2() != rivercolour.y * 255.f || world->river3() != rivercolour.z * 255.f)
            colourschanged = 1;

        if (world->mud1() != mudcolour.x * 255.f || world->mud2() != mudcolour.y * 255.f || world->mud3() != mudcolour.z * 255.f)
            colourschanged = 1;

        if (world->sand1() != sandcolour.x * 255.f || world->sand2() != sandcolour.y * 255.f || world->sand3() != sandcolour.z * 255.f)
            colourschanged = 1;

        if (world->shingle1() != shinglecolour.x * 255.f || world->shingle2() != shinglecolour.y * 255.f || world->shingle3() != shinglecolour.z * 255.f)
            colourschanged = 1;

        if (world->mangrove1() != mangrovecolour.x * 255.f || world->mangrove2() != mangrovecolour.y * 255.f || world->mangrove3() != mangrovecolour.z * 255.f)
            colourschanged = 1;

        if (world->highlight1() != highlightcolour.x * 255.f || world->highlight2() != highlightcolour.y * 255.f || world->highlight3() != highlightcolour.z * 255.f)
        {
            // This one's different as it doesn't involve redrawing the map - just redrawing the highlight sprites.

            world->sethighlight1((int)(highlightcolour.x * 255.f));
            world->sethighlight2((int)(highlightcolour.y * 255.f));
            world->sethighlight3((int)(highlightcolour.z * 255.f));

            drawhighlightobjects(*world, *highlightimage, highlightsize, *minihighlightimage, minihighlightsize);

            highlighttexture->loadFromImage(*highlightimage);
            highlight->setTexture(*highlighttexture);

            minihighlighttexture->loadFromImage(*minihighlightimage);
            minihighlight->setTexture(*minihighlighttexture);
            minihighlight->setOrigin((float)minihighlightsize / 2.0f, (float)minihighlightsize / 2.0f);

            highlightcolour.x = (float)world->highlight1() / 255.f;
            highlightcolour.y = (float)world->highlight2() / 255.f;
            highlightcolour.z = (float)world->highlight3() / 255.f;
        }

        if (world->landshading() != shadingland || world->lakeshading() != shadinglake || world->seashading() != shadingsea)
            colourschanged = 1;

        if (world->landmarbling() != marblingland || world->lakemarbling() != marblinglake || world->seamarbling() != marblingsea)
            colourschanged = 1;

        if (world->minriverflowglobal() != globalriversentry || world->minriverflowregional() != regionalriversentry)
            colourschanged = 1;

        if (world->shadingdir() != shadingdircorrected || world->snowchange() != snowchange + 1 || world->seaiceappearance() != seaiceappearance + 1 || world->colourcliffs() != colourcliffs || world->showmangroves() != mangroves)
            colourschanged = 1;

        if (showcolouroptions == 1 && colourschanged == 1)
        {
            world->setocean1((int)(oceancolour.x * 255.f));
            world->setocean2((int)(oceancolour.y * 255.f));
            world->setocean3((int)(oceancolour.z * 255.f));

            world->setdeepocean1((int)(deepoceancolour.x * 255.f));
            world->setdeepocean2((int)(deepoceancolour.y * 255.f));
            world->setdeepocean3((int)(deepoceancolour.z * 255.f));

            world->setbase1((int)(basecolour.x * 255.f));
            world->setbase2((int)(basecolour.y * 255.f));
            world->setbase3((int)(basecolour.z * 255.f));

            world->setgrass1((int)(grasscolour.x * 255.f));
            world->setgrass2((int)(grasscolour.y * 255.f));
            world->setgrass3((int)(grasscolour.z * 255.f));

            world->setbasetemp1((int)(basetempcolour.x * 255.f));
            world->setbasetemp2((int)(basetempcolour.y * 255.f));
            world->setbasetemp3((int)(basetempcolour.z * 255.f));

            world->sethighbase1((int)(highbasecolour.x * 255.f));
            world->sethighbase2((int)(highbasecolour.y * 255.f));
            world->sethighbase3((int)(highbasecolour.z * 255.f));

            world->setdesert1((int)(desertcolour.x * 255.f));
            world->setdesert2((int)(desertcolour.y * 255.f));
            world->setdesert3((int)(desertcolour.z * 255.f));

            world->sethighdesert1((int)(highdesertcolour.x * 255.f));
            world->sethighdesert2((int)(highdesertcolour.y * 255.f));
            world->sethighdesert3((int)(highdesertcolour.z * 255.f));

            world->setcolddesert1((int)(colddesertcolour.x * 255.f));
            world->setcolddesert2((int)(colddesertcolour.y * 255.f));
            world->setcolddesert3((int)(colddesertcolour.z * 255.f));

            world->seteqtundra1((int)(eqtundracolour.x * 255.f));
            world->seteqtundra2((int)(eqtundracolour.y * 255.f));
            world->seteqtundra3((int)(eqtundracolour.z * 255.f));

            world->settundra1((int)(tundracolour.x * 255.f));
            world->settundra2((int)(tundracolour.y * 255.f));
            world->settundra3((int)(tundracolour.z * 255.f));

            world->setcold1((int)(coldcolour.x * 255.f));
            world->setcold2((int)(coldcolour.y * 255.f));
            world->setcold3((int)(coldcolour.z * 255.f));

            world->setseaice1((int)(seaicecolour.x * 255.f));
            world->setseaice2((int)(seaicecolour.y * 255.f));
            world->setseaice3((int)(seaicecolour.z * 255.f));

            world->setglacier1((int)(glaciercolour.x * 255.f));
            world->setglacier2((int)(glaciercolour.y * 255.f));
            world->setglacier3((int)(glaciercolour.z * 255.f));

            world->setsaltpan1((int)(saltpancolour.x * 255.f));
            world->setsaltpan2((int)(saltpancolour.y * 255.f));
            world->setsaltpan3((int)(saltpancolour.z * 255.f));

            world->seterg1((int)(ergcolour.x * 255.f));
            world->seterg2((int)(ergcolour.y * 255.f));
            world->seterg3((int)(ergcolour.z * 255.f));

            world->setwetlands1((int)(wetlandscolour.x * 255.f));
            world->setwetlands2((int)(wetlandscolour.y * 255.f));
            world->setwetlands3((int)(wetlandscolour.z * 255.f));

            world->setlake1((int)(lakecolour.x * 255.f));
            world->setlake2((int)(lakecolour.y * 255.f));
            world->setlake3((int)(lakecolour.z * 255.f));

            world->setriver1((int)(rivercolour.x * 255.f));
            world->setriver2((int)(rivercolour.y * 255.f));
            world->setriver3((int)(rivercolour.z * 255.f));

            world->setmud1((int)(mudcolour.x * 255.f));
            world->setmud2((int)(mudcolour.y * 255.f));
            world->setmud3((int)(mudcolour.z * 255.f));

            world->setsand1((int)(sandcolour.x * 255.f));
            world->setsand2((int)(sandcolour.y * 255.f));
            world->setsand3((int)(sandcolour.z * 255.f));

            world->setshingle1((int)(shinglecolour.x * 255.f));
            world->setshingle2((int)(shinglecolour.y * 255.f));
            world->setshingle3((int)(shinglecolour.z * 255.f));

            world->setmangrove1((int)(mangrovecolour.x * 255.f));
            world->setmangrove2((int)(mangrovecolour.y * 255.f));
            world->setmangrove3((int)(mangrovecolour.z * 255.f));

            world->setlandshading(shadingland);
            world->setlakeshading(shadinglake);
            world->setseashading(shadingsea);

            world->setlandmarbling(marblingland);
            world->setlakemarbling(marblinglake);
            world->setseamarbling(marblingsea);

            world->setminriverflowglobal(globalriversentry);
            world->setminriverflowregional(regionalriversentry);

            world->setshadingdir(shadingdircorrected);
            world->setsnowchange(snowchange + 1);
            world->setseaiceappearance(seaiceappearance + 1);
            world->setcolourcliffs(colourcliffs);
            world->setshowmangroves(mangroves);

            // Copy them back again to ensure that there aren't any stray floating points.

            oceancolour.x = (float)world->ocean1() / 255.f;
            oceancolour.y = (float)world->ocean2() / 255.f;
            oceancolour.z = (float)world->ocean3() / 255.f;

            deepoceancolour.x = (float)world->deepocean1() / 255.f;
            deepoceancolour.y = (float)world->deepocean2() / 255.f;
            deepoceancolour.z = (float)world->deepocean3() / 255.f;

            basecolour.x = (float)world->base1() / 255.f;
            basecolour.y = (float)world->base2() / 255.f;
            basecolour.z = (float)world->base3() / 255.f;

            grasscolour.x = (float)world->grass1() / 255.f;
            grasscolour.y = (float)world->grass2() / 255.f;
            grasscolour.z = (float)world->grass3() / 255.f;

            basetempcolour.x = (float)world->basetemp1() / 255.f;
            basetempcolour.y = (float)world->basetemp2() / 255.f;
            basetempcolour.z = (float)world->basetemp3() / 255.f;

            highbasecolour.x = (float)world->highbase1() / 255.f;
            highbasecolour.y = (float)world->highbase2() / 255.f;
            highbasecolour.z = (float)world->highbase3() / 255.f;

            desertcolour.x = (float)world->desert1() / 255.f;
            desertcolour.y = (float)world->desert2() / 255.f;
            desertcolour.z = (float)world->desert3() / 255.f;

            highdesertcolour.x = (float)world->highdesert1() / 255.f;
            highdesertcolour.y = (float)world->highdesert2() / 255.f;
            highdesertcolour.z = (float)world->highdesert3() / 255.f;

            colddesertcolour.x = (float)world->colddesert1() / 255.f;
            colddesertcolour.y = (float)world->colddesert2() / 255.f;
            colddesertcolour.z = (float)world->colddesert3() / 255.f;

            eqtundracolour.x = (float)world->eqtundra1() / 255.f;
            eqtundracolour.y = (float)world->eqtundra2() / 255.f;
            eqtundracolour.z = (float)world->eqtundra3() / 255.f;

            tundracolour.x = (float)world->tundra1() / 255.f;
            tundracolour.y = (float)world->tundra2() / 255.f;
            tundracolour.z = (float)world->tundra3() / 255.f;

            coldcolour.x = (float)world->cold1() / 255.f;
            coldcolour.y = (float)world->cold2() / 255.f;
            coldcolour.z = (float)world->cold3() / 255.f;

            seaicecolour.x = (float)world->seaice1() / 255.f;
            seaicecolour.y = (float)world->seaice2() / 255.f;
            seaicecolour.z = (float)world->seaice3() / 255.f;

            glaciercolour.x = (float)world->glacier1() / 255.f;
            glaciercolour.y = (float)world->glacier2() / 255.f;
            glaciercolour.z = (float)world->glacier3() / 255.f;

            saltpancolour.x = (float)world->saltpan1() / 255.f;
            saltpancolour.y = (float)world->saltpan2() / 255.f;
            saltpancolour.z = (float)world->saltpan3() / 255.f;

            ergcolour.x = (float)world->erg1() / 255.f;
            ergcolour.y = (float)world->erg2() / 255.f;
            ergcolour.z = (float)world->erg3() / 255.f;

            wetlandscolour.x = (float)world->wetlands1() / 255.f;
            wetlandscolour.y = (float)world->wetlands2() / 255.f;
            wetlandscolour.z = (float)world->wetlands3() / 255.f;

            lakecolour.x = (float)world->lake1() / 255.f;
            lakecolour.y = (float)world->lake2() / 255.f;
            lakecolour.z = (float)world->lake3() / 255.f;

            rivercolour.x = (float)world->river1() / 255.f;
            rivercolour.y = (float)world->river2() / 255.f;
            rivercolour.z = (float)world->river3() / 255.f;

            mudcolour.x = (float)world->mud1() / 255.f;
            mudcolour.y = (float)world->mud2() / 255.f;
            mudcolour.z = (float)world->mud3() / 255.f;

            sandcolour.x = (float)world->sand1() / 255.f;
            sandcolour.y = (float)world->sand2() / 255.f;
            sandcolour.z = (float)world->sand3() / 255.f;

            shinglecolour.x = (float)world->shingle1() / 255.f;
            shinglecolour.y = (float)world->shingle2() / 255.f;
            shinglecolour.z = (float)world->shingle3() / 255.f;

            mangrovecolour.x = (float)world->mangrove1() / 255.f;
            mangrovecolour.y = (float)world->mangrove2() / 255.f;
            mangrovecolour.z = (float)world->mangrove3() / 255.f;

            mapview = relief;
            globalmapimagecreated[6] = 0;

            drawglobalmapimage(mapview, *world, globalmapimagecreated, *globalelevationimage, *globaltemperatureimage, *globalprecipitationimage, *globalclimateimage, *globalriversimage, *globalreliefimage, *displayglobalelevationimage, *displayglobaltemperatureimage, *displayglobalprecipitationimage, *displayglobalclimateimage, *displayglobalriversimage, *displayglobalreliefimage);

            globalmaptexture->loadFromImage(*displayglobalreliefimage);
            globalmap->setTexture(*globalmaptexture);
            minimap->setTexture(*globalmaptexture);

            if (screenmode == regionalmapscreen)
            {
                mapview = relief;
                regionalmapimagecreated[6] = 0;

                drawregionalmapimage(mapview, *world, *region, regionalmapimagecreated, *regionalelevationimage, *regionaltemperatureimage, *regionalprecipitationimage, *regionalclimateimage, *regionalriversimage, *regionalreliefimage);

                regionalmaptexture->loadFromImage(*regionalreliefimage);
                regionalmap->setTexture(*regionalmaptexture);
            }

            colourschanged = 0;
        }
    }

    ImGui::SFML::Shutdown();

    return 0;
}

// This looks up the latest version of the program. Based on code by Parveen: https://hoven.in/cpp-network/c-program-download-file-from-url.html

float getlatestversion()
{
    IStream* stream;

    const char* URL = "https://raw.githubusercontent.com/JonathanCRH/Undiscovered_Worlds/main/version.txt";

    if (getURL(0, URL, &stream, 0, 0)) // Didn't work for some reason.
        return 0.0f;

    // this char array will be cyclically filled with bytes from URL
    char buff[100];

    // we shall keep appending the bytes to this string
    string s;

    unsigned long bytesRead;

    while (true)
    {
        // Reads a specified number of bytes from the stream object into char array and stores the actual bytes read to "bytesRead"
        stream->Read(buff, 100, &bytesRead);

        if (0U == bytesRead)
            break;

        // append and collect to the string
        s.append(buff, bytesRead);
    };

    // release the interface
    stream->Release();

    float val = stof(s);

    return val;
}

// This makes adjustments to accommodate different world sizes. (Basically resize the map images.)

void adjustforsize(planet& world, sf::Vector2i& globaltexturesize, sf::Image& globalelevationimage, sf::Image& globaltemperatureimage, sf::Image& globalprecipitationimage, sf::Image& globalclimateimage, sf::Image& globalriversimage, sf::Image& globalreliefimage, sf::Image& highlightimage, int highlightsize, sf::Image& minihighlightimage, int& minihighlightsize)
{
    int size = world.size();
    int width = 0;
    int height = 0;

    if (size == 0) // Small
    {
        width = 511;
        height = 256;
    }

    if (size == 1) // Medium
    {
        width = 1023;
        height = 512;
    }

    if (size == 2) // Large
    {
        width = 2047;
        height = 1024;
    }

    world.setwidth(width);
    world.setheight(height);

    globaltexturesize.x = world.width() + 1;
    globaltexturesize.y = world.height() + 2;

    globalelevationimage.create(globaltexturesize.x, globaltexturesize.y);
    globaltemperatureimage.create(globaltexturesize.x, globaltexturesize.y);
    globalprecipitationimage.create(globaltexturesize.x, globaltexturesize.y);
    globalclimateimage.create(globaltexturesize.x, globaltexturesize.y);
    globalriversimage.create(globaltexturesize.x, globaltexturesize.y);
    globalreliefimage.create(globaltexturesize.x, globaltexturesize.y);

    for (int i = 0; i < globaltexturesize.x; i++)
    {
        for (int j = 0; j < globaltexturesize.y; j++)
        {
            globalelevationimage.setPixel(i, j, sf::Color::Black);
            globaltemperatureimage.setPixel(i, j, sf::Color::Black);
            globalprecipitationimage.setPixel(i, j, sf::Color::Black);
            globalclimateimage.setPixel(i, j, sf::Color::Black);
            globalriversimage.setPixel(i, j, sf::Color::Black);
            globalreliefimage.setPixel(i, j, sf::Color::Black);
        }
    }

    drawhighlightobjects(world, highlightimage, highlightsize, minihighlightimage, minihighlightsize);
}

// This draws the highlight objects.

void drawhighlightobjects(planet& world, sf::Image& highlightimage, int highlightsize, sf::Image& minihighlightimage, int& minihighlightsize)
{
    sf::Color currenthighlightcolour;

    currenthighlightcolour.r = world.highlight1();
    currenthighlightcolour.g = world.highlight2();
    currenthighlightcolour.b = world.highlight3();

    // Do the highlight point

    for (int i = 0; i < highlightsize; i++)
    {
        for (int j = 0; j < highlightsize; j++)
            highlightimage.setPixel(i, j, currenthighlightcolour);
    }

    // And the minimap highlight

    int size = world.size();

    if (size == 0)
        minihighlightsize = 32;

    if (size == 1)
        minihighlightsize = 16;

    if (size == 2)
        minihighlightsize = 8;

    minihighlightimage.create(32, 32);

    for (int i = 0; i < 32; i++)
    {
        for (int j = 0; j < 32; j++)
            minihighlightimage.setPixel(i, j, sf::Color::Transparent);
    }

    for (int i = 0; i < minihighlightsize; i++)
    {
        minihighlightimage.setPixel(i, 0, currenthighlightcolour);
        minihighlightimage.setPixel(i, minihighlightsize - 1, currenthighlightcolour);
    }

    for (int j = 0; j < minihighlightsize; j++)
    {
        minihighlightimage.setPixel(0, j, currenthighlightcolour);
        minihighlightimage.setPixel(minihighlightsize - 1, j, currenthighlightcolour);
    }
}

// This prints out an update text.

void updatereport(string text)
{
    cout << text << endl;
}

// This makes a button in a standard size and indentation.

bool standardbutton(const char* label)
{
    ImGui::SetCursorPosX(20);

    return ImGui::Button(label, ImVec2(120.0f, 0.0f));
}

// These functions draw a global map image (ready to be applied to a texture).

void drawglobalmapimage(mapviewenum mapview, planet& world, bool globalmapimagecreated[], sf::Image& globalelevationimage, sf::Image& globaltemperatureimage, sf::Image& globalprecipitationimage, sf::Image& globalclimateimage, sf::Image& globalriversimage, sf::Image& globalreliefimage, sf::Image& displayglobalelevationimage, sf::Image& displayglobaltemperatureimage, sf::Image& displayglobalprecipitationimage, sf::Image& displayglobalclimateimage, sf::Image& displayglobalriversimage, sf::Image& displayglobalreliefimage)
{
    if (mapview == elevation)
    {
        if (globalmapimagecreated[0] == 1)
            return;

        drawglobalelevationmapimage(world, globalelevationimage, displayglobalelevationimage);

        globalmapimagecreated[0] = 1;
    }

    if (mapview == temperature)
    {
        if (globalmapimagecreated[2] == 1)
            return;

        drawglobaltemperaturemapimage(world, globaltemperatureimage, displayglobaltemperatureimage);

        globalmapimagecreated[2] = 1;
    }

    if (mapview == precipitation)
    {
        if (globalmapimagecreated[3] == 1)
            return;

        drawglobalprecipitationmapimage(world, globalprecipitationimage, displayglobalprecipitationimage);

        globalmapimagecreated[3] = 1;
    }

    if (mapview == climate)
    {
        if (globalmapimagecreated[4] == 1)
            return;

        drawglobalclimatemapimage(world, globalclimateimage, displayglobalclimateimage);

        globalmapimagecreated[4] = 1;
    }

    if (mapview == rivers)
    {
        if (globalmapimagecreated[5] == 1)
            return;

        drawglobalriversmapimage(world, globalriversimage, displayglobalriversimage);

        globalmapimagecreated[5] = 1;
    }

    if (mapview == relief)
    {
        if (globalmapimagecreated[6] == 1)
            return;

        drawglobalreliefmapimage(world, globalreliefimage, displayglobalreliefimage);

        globalmapimagecreated[6] = 1;
    }
}

void drawglobalelevationmapimage(planet& world, sf::Image& globalelevationimage, sf::Image& displayglobalelevationimage)
{
    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();

    int colour1, colour2, colour3;

    int div = world.maxelevation() / 255;
    int base = world.maxelevation() / 4;

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            int heightpoint = world.map(i, j);

            colour1 = heightpoint / div;

            if (colour1 > 255)
                colour1 = 255;

            colour2 = colour1;
            colour3 = colour2;

            globalelevationimage.setPixel(i, j, sf::Color(colour1, colour2, colour3));
        }
    }

    float displayx = DISPLAYMAPSIZEX;

    float mapdiv = ((float)width + 1.0f) / displayx;

    for (float i = 0.0f; i < DISPLAYMAPSIZEX; i++)
    {
        float pixelx = i * mapdiv;
        
        for (float j = 0.0f; j < DISPLAYMAPSIZEY; j++)
        {
            float pixely = j * mapdiv;
            
            sf::Color thispixel = globalelevationimage.getPixel((int)pixelx, (int)pixely);

            displayglobalelevationimage.setPixel((int)i, (int)j, thispixel);
        }
    }
}

void drawglobaltemperaturemapimage(planet& world, sf::Image& globaltemperatureimage, sf::Image& displayglobaltemperatureimage)
{
    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();

    int colour1, colour2, colour3;

    int landdiv = ((world.maxelevation() - sealevel) / 2) / 255;
    int seadiv = sealevel / 255;

    int div = world.maxelevation() / 255;
    int base = world.maxelevation() / 4;

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (world.outline(i, j))
            {
                colour1 = 0;
                colour2 = 0;
                colour3 = 0;
            }
            else
            {
                int jantemp = world.jantemp(i, j);
                int jultemp = world.jultemp(i, j);
                int aprtemp = world.aprtemp(i, j);
                
                int temperature = (jantemp + aprtemp + jultemp + aprtemp) / 4; // April appears twice, as it's the same as October

                temperature = temperature + 10;

                if (temperature > 0)
                {
                    colour1 = 250;
                    colour2 = 250 - (temperature * 3);
                    colour3 = 250 - (temperature * 7);
                }
                else
                {
                    temperature = abs(temperature);

                    colour1 = 250 - (temperature * 7);
                    colour2 = 250 - (temperature * 7);
                    colour3 = 250;
                }
            }

            if (colour1 < 0)
                colour1 = 0;

            if (colour2 < 0)
                colour2 = 0;

            if (colour3 < 0)
                colour3 = 0;

            globaltemperatureimage.setPixel(i, j, sf::Color(colour1, colour2, colour3));
        }
    }

    float displayx = DISPLAYMAPSIZEX;

    float mapdiv = ((float)width + 1.0f) / displayx;

    for (float i = 0.0f; i < DISPLAYMAPSIZEX; i++)
    {
        float pixelx = i * mapdiv;
        int sourcei = (int)pixelx;

        for (float j = 0.0f; j < DISPLAYMAPSIZEY; j++)
        {
            float pixely = j * mapdiv;
            int sourcej = (int)pixely;

            sf::Color thispixel = globaltemperatureimage.getPixel(sourcei, sourcej);

            displayglobaltemperatureimage.setPixel((int)i, (int)j, thispixel);

            // If there's an outline pixel nearby, use that instead.

            if (width > DISPLAYMAPSIZEX && sourcei > 0 && sourcej > 0)
            {
                for (int k = sourcei - 1; k <= sourcei; k++)
                {
                    for (int l = sourcej - 1; l <= sourcej; l++)
                    {
                        sf::Color newpixel = globaltemperatureimage.getPixel(k, l);

                        if (newpixel.r == 0 && newpixel.g == 0 && newpixel.b == 0)
                            displayglobaltemperatureimage.setPixel((int)i, (int)j, newpixel);
                    }
                }
            }
        }
    }
}

void drawglobalprecipitationmapimage(planet& world, sf::Image& globalprecipitationimage, sf::Image& displayglobalprecipitationimage)
{
    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();

    int colour1, colour2, colour3;

    int landdiv = ((world.maxelevation() - sealevel) / 2) / 255;
    int seadiv = sealevel / 255;

    int div = world.maxelevation() / 255;
    int base = world.maxelevation() / 4;

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (world.outline(i, j))
            {
                colour1 = 0;
                colour2 = 0;
                colour3 = 0;
            }
            else
            {
                int rainfall = (world.summerrain(i, j) + world.winterrain(i, j)) / 2;

                rainfall = rainfall / 4;

                colour1 = 255 - rainfall;
                colour2 = 255 - rainfall;
                colour3 = 255;
            }

            if (colour1 < 0)
                colour1 = 0;

            if (colour2 < 0)
                colour2 = 0;

            if (colour3 < 0)
                colour3 = 0;

            globalprecipitationimage.setPixel(i, j, sf::Color(colour1, colour2, colour3));
        }
    }

    float displayx = DISPLAYMAPSIZEX;

    float mapdiv = ((float)width + 1.0f) / displayx;

    for (float i = 0.0f; i < DISPLAYMAPSIZEX; i++)
    {
        float pixelx = i * mapdiv;
        int sourcei = (int)pixelx;

        for (float j = 0.0f; j < DISPLAYMAPSIZEY; j++)
        {
            float pixely = j * mapdiv;
            int sourcej = (int)pixely;

            sf::Color thispixel = globalprecipitationimage.getPixel(sourcei, sourcej);

            displayglobalprecipitationimage.setPixel((int)i, (int)j, thispixel);

            // If there's an outline pixel nearby, use that instead.

            if (width > DISPLAYMAPSIZEX && sourcei > 0 && sourcej > 0)
            {
                for (int k = sourcei - 1; k <= sourcei; k++)
                {
                    for (int l = sourcej - 1; l <= sourcej; l++)
                    {
                        sf::Color newpixel = globalprecipitationimage.getPixel(k, l);

                        if (newpixel.r == 0 && newpixel.g == 0 && newpixel.b == 0)
                            displayglobalprecipitationimage.setPixel((int)i, (int)j, newpixel);
                    }
                }
            }
        }
    }
}

void drawglobalclimatemapimage(planet& world, sf::Image& globalclimateimage, sf::Image& displayglobalclimateimage)
{
    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();

    int colour1, colour2, colour3;

    int landdiv = ((world.maxelevation() - sealevel) / 2) / 255;
    int seadiv = sealevel / 255;

    int div = world.maxelevation() / 255;
    int base = world.maxelevation() / 4;

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (world.sea(i, j))
            {
                if (world.seaice(i, j) == 2) // Permanent sea ice
                {
                    colour1 = 243;
                    colour2 = 243;
                    colour3 = 255;
                }
                else
                {
                    if (world.seaice(i, j) == 1) // Seasonal sea ice
                    {
                        colour1 = 228;
                        colour2 = 228;
                        colour3 = 255;
                    }
                    else // Open sea
                    {
                        colour1 = 13;
                        colour2 = 49;
                        colour3 = 109;
                    }
                }

                globalclimateimage.setPixel(i, j, sf::Color(colour1, colour2, colour3));
            }
            else
            {
                sf::Color landcolour;

                landcolour = getclimatecolours(world.climate(i, j));
                globalclimateimage.setPixel(i, j, landcolour);
            }
        }
    }

    float displayx = DISPLAYMAPSIZEX;

    float mapdiv = ((float)width + 1.0f) / displayx;

    for (float i = 0.0f; i < DISPLAYMAPSIZEX; i++)
    {
        float pixelx = i * mapdiv;
        int sourcei = (int)pixelx;

        for (float j = 0.0f; j < DISPLAYMAPSIZEY; j++)
        {
            float pixely = j * mapdiv;
            int sourcej = (int)pixely;

            sf::Color thispixel = globalclimateimage.getPixel(sourcei, sourcej);

            displayglobalclimateimage.setPixel((int)i, (int)j, thispixel);

            // If there's an outline pixel nearby, use that instead.

            if (width > DISPLAYMAPSIZEX && sourcei > 0 && sourcej > 0)
            {
                for (int k = sourcei - 1; k <= sourcei; k++)
                {
                    for (int l = sourcej - 1; l <= sourcej; l++)
                    {
                        sf::Color newpixel = globalclimateimage.getPixel(k, l);

                        if (newpixel.r == 0 && newpixel.g == 0 && newpixel.b == 0)
                            displayglobalclimateimage.setPixel((int)i, (int)j, newpixel);
                    }
                }
            }
        }
    }
}

void drawglobalriversmapimage(planet& world, sf::Image& globalriversimage, sf::Image& displayglobalriversimage)
{
    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();

    int colour1, colour2, colour3;

    int landdiv = ((world.maxelevation() - sealevel) / 2) / 255;
    int seadiv = sealevel / 255;

    int div = world.maxelevation() / 255;
    int base = world.maxelevation() / 4;

    int mult = world.maxriverflow() / 255;

    if (mult < 1)
        mult = 1;

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (world.outline(i, j))
            {
                colour1 = 0;
                colour2 = 0;
                colour3 = 0;
            }
            else
            {
                int flow = world.riveraveflow(i, j);

                if (flow > 0 && world.sea(i, j) == 0)
                {
                    flow = flow * 10;

                    colour1 = 255 - (flow / mult);
                    if (colour1 < 0)
                        colour1 = 0;
                    colour2 = colour1;
                }
                else
                {
                    if (world.deltadir(i, j) != 0 && world.sea(i, j) == 0)
                    {
                        flow = (world.deltajan(i, j) + world.deltajul(i, j)) / 2;
                        flow = flow * 10;

                        colour1 = 255 - (flow / mult);
                        if (colour1 < 0)
                            colour1 = 0;
                        colour2 = colour1;
                    }
                    else
                    {
                        colour1 = 255;
                        colour2 = 255;
                    }
                }

                colour3 = 255;

                if (world.truelake(i, j) != 0)
                {
                    colour1 = 150;
                    colour2 = 150;
                    colour3 = 250;
                }

                if (world.special(i, j) > 100 && world.sea(i, j) == 0 && world.riverjan(i, j) + world.riverjul(i, j) < 600)
                {
                    if (world.special(i, j) == 110)
                    {
                        colour1 = 150;
                        colour2 = 150;
                        colour3 = 150;
                    }

                    if (world.special(i, j) == 120)
                    {
                        colour1 = 250;
                        colour2 = 250;
                        colour3 = 50;
                    }

                    if (world.special(i, j) >= 130)
                    {
                        colour1 = 50;
                        colour2 = 250;
                        colour3 = 100;
                    }
                }
            }

            if (world.outline(i, j))
            {
                colour1 = 0;
                colour2 = 0;
                colour3 = 0;
            }

            if (world.volcano(i, j) > 0)
            {
                colour1 = 240;
                colour2 = 0;
                colour3 = 0;
            }

            globalriversimage.setPixel(i, j, sf::Color(colour1, colour2, colour3));
        }
    }

    if (world.size() == 0)
    {
        for (int i = 0; i < DISPLAYMAPSIZEX; i++)
        {
            for (int j = 0; j < DISPLAYMAPSIZEY; j++)
            {
                sf::Color thispixel = globalriversimage.getPixel(i/2, j/2);
                displayglobalriversimage.setPixel(i, j, thispixel);
            }
        }
    }
    else
    {
        if (world.size() == 1)
        {
            for (int i = 0; i < DISPLAYMAPSIZEX; i++)
            {
                for (int j = 0; j < DISPLAYMAPSIZEY; j++)
                {
                    sf::Color thispixel = globalriversimage.getPixel(i, j);
                    displayglobalriversimage.setPixel(i, j, thispixel);
                }
            }
        }
        else
        {
            float displayx = DISPLAYMAPSIZEX;

            float mapdiv = ((float)width + 1.0f) / displayx;

            for (float i = 0.0f; i < DISPLAYMAPSIZEX; i++)
            {
                float pixelx = i * mapdiv;
                int sourcei = (int)pixelx;

                for (float j = 0.0f; j < DISPLAYMAPSIZEY; j++)
                {
                    float pixely = j * mapdiv;
                    int sourcej = (int)pixely;

                    sf::Color thispixel = globalriversimage.getPixel(sourcei, sourcej);

                    displayglobalriversimage.setPixel((int)i, (int)j, thispixel);

                    // Check for the largest river pixel nearby.

                    if (sourcei > 0 && sourcej > 0)
                    {
                        int lowestred = 255;
                        int lowestgreen = 255;
                        int lowestblue = 255;

                        for (int k = sourcei - 1; k <= sourcei; k++)
                        {
                            for (int l = sourcej - 1; l <= sourcej; l++)
                            {
                                sf::Color newpixel = globalriversimage.getPixel(k, l);

                                if (newpixel.r < lowestred && newpixel.r == newpixel.g)
                                {
                                    lowestred = newpixel.r;
                                    lowestgreen = newpixel.g;
                                    lowestblue = newpixel.b;
                                }
                            }
                        }

                        if (lowestred < 255)
                            displayglobalriversimage.setPixel((int)i, (int)j, sf::Color(lowestred, lowestgreen, lowestblue));
                    }

                    // If there's an outline pixel nearby, use that instead.

                    if (width > DISPLAYMAPSIZEX && sourcei > 0 && sourcej > 0)
                    {
                        for (int k = sourcei - 1; k <= sourcei; k++)
                        {
                            for (int l = sourcej - 1; l <= sourcej; l++)
                            {
                                sf::Color newpixel = globalriversimage.getPixel(k, l);

                                if (newpixel.r == 0 && newpixel.g == 0 && newpixel.b == 0)
                                    displayglobalriversimage.setPixel((int)i, (int)j, newpixel);
                            }
                        }
                    }
                }
            }
        }
    }
}

void drawglobalreliefmapimage(planet& world, sf::Image& globalreliefimage, sf::Image& displayglobalreliefimage)
{
    int width = world.width();
    int height = world.height();
    int sealevel = world.sealevel();
    int type = world.type();

    int colour1, colour2, colour3;

    int landdiv = ((world.maxelevation() - sealevel) / 2) / 255;
    int seadiv = sealevel / 255;

    int div = world.maxelevation() / 255;
    int base = world.maxelevation() / 4;

    int mult = world.maxriverflow() / 255;

    int minriverflow = world.minriverflowglobal(); // Rivers of this size or larger will be shown on the map.

    float landshading = world.landshading();
    float lakeshading = world.lakeshading();
    float seashading = world.seashading();

    int shadingdir = world.shadingdir();
    bool colourcliffs = world.colourcliffs();

    vector<vector<short>> reliefmap1(ARRAYWIDTH, vector<short>(ARRAYHEIGHT, 0));
    vector<vector<short>> reliefmap2(ARRAYWIDTH, vector<short>(ARRAYHEIGHT, 0));
    vector<vector<short>> reliefmap3(ARRAYWIDTH, vector<short>(ARRAYHEIGHT, 0));

    int var = 0; // Amount colours may be varied to make the map seem more speckledy.

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            var = 10;

            int sea = world.sea(i, j);

            if (sea == 1)
            {
                if ((world.seaice(i, j) == 2 && (world.seaiceappearance() == 1 || world.seaiceappearance() == 3)) || (world.seaice(i, j) == 1 && world.seaiceappearance() == 3)) // Sea ice
                {
                    colour1 = world.seaice1();
                    colour2 = world.seaice2();
                    colour3 = world.seaice3();

                    var = 0;
                }
                else
                {
                    colour1 = (world.ocean1() * world.map(i, j) + world.deepocean1() * (sealevel - world.map(i, j))) / sealevel;
                    colour2 = (world.ocean2() * world.map(i, j) + world.deepocean2() * (sealevel - world.map(i, j))) / sealevel;
                    colour3 = (world.ocean3() * world.map(i, j) + world.deepocean3() * (sealevel - world.map(i, j))) / sealevel;

                    var = 5;
                }
            }
            else
            {
                if ((world.riverjan(i, j) + world.riverjul(i, j)) / 2 >= minriverflow)
                {
                    colour1 = world.river1();
                    colour2 = world.river2();
                    colour3 = world.river3();
                }
                else
                {
                    if (world.special(i, j) == 110) // Salt pan
                    {
                        colour1 = world.saltpan1();
                        colour2 = world.saltpan2();
                        colour3 = world.saltpan3();

                        var = 20;
                    }
                    else
                    {
                        int avetemp = world.avetemp(i, j) + 10;

                        // First, adjust the base colours depending on temperature.

                        int thisbase1, thisbase2, thisbase3, newdesert1, newdesert2, newdesert3;

                        if (avetemp > 30)
                        {
                            thisbase1 = world.base1();
                            thisbase2 = world.base2();
                            thisbase3 = world.base3();

                            newdesert1 = world.desert1();
                            newdesert2 = world.desert2();
                            newdesert3 = world.desert3();
                        }
                        else
                        {
                            int hotno = avetemp / 3;
                            int coldno = 10 - hotno;

                            thisbase1 = (hotno * world.base1() + coldno * world.basetemp1()) / 10;
                            thisbase2 = (hotno * world.base2() + coldno * world.basetemp2()) / 10;
                            thisbase3 = (hotno * world.base3() + coldno * world.basetemp3()) / 10;
                        }

                        if (avetemp > 30)
                        {
                            newdesert1 = world.desert1();
                            newdesert2 = world.desert2();
                            newdesert3 = world.desert3();
                        }
                        else
                        {
                            if (avetemp <= 10)
                            {
                                newdesert1 = world.colddesert1();
                                newdesert2 = world.colddesert2();
                                newdesert3 = world.colddesert3();
                            }
                            else
                            {
                                int hotno = avetemp - 10;
                                int coldno = 20 - hotno;

                                newdesert1 = (hotno * world.desert1() + coldno * world.colddesert1()) / 20;
                                newdesert2 = (hotno * world.desert2() + coldno * world.colddesert2()) / 20;
                                newdesert3 = (hotno * world.desert3() + coldno * world.colddesert3()) / 20;
                            }
                        }

                        // Now, adjust for the presence of monsoon.

                        float winterrain = (float)world.winterrain(i, j);
                        float summerrain = (float)world.summerrain(i, j);

                        float totalrain = winterrain + summerrain;

                        float monsoon = 0.0f;

                        if (winterrain < 1.0f)
                            winterrain = 1.0f;

                        if (winterrain < summerrain)
                        {
                            monsoon = summerrain - winterrain;

                            monsoon = monsoon / 1000.0f; // 410

                            if (monsoon > 0.99f)
                                monsoon = 0.99f;
                        }

                        // The closer it is to tropical rainforest, the more we intensify the rain effect.

                        float rainforestmult = (float)world.mintemp(i, j) / 18.0f; //9.0f;

                        rainforestmult = rainforestmult * (float)world.winterrain(i, j) / 80.0f;

                        if (rainforestmult < 1.0f)
                            rainforestmult = 1.0f;

                        totalrain = totalrain * rainforestmult;

                        // Now adjust the colours for height.

                        int mapelev = world.map(i, j) - sealevel;
                        int desertmapelev = mapelev; // We won't mess about with this one.

                        // If this setting is chosen, pretend that the elevation is much lower for flat areas.

                        if (colourcliffs == 1)
                        {
                            int biggestslope = 0;

                            for (int k = i - 1; k <= i + 1; k++)
                            {
                                int kk = k;

                                if (kk<0 || kk>width)
                                    kk = wrap(kk, width);

                                for (int l = j - 1; l <= j + 1; l++)
                                {
                                    if (l >= 0 && l <= height)
                                    {
                                        int thisslope = mapelev + sealevel - world.map(kk, l);

                                        if (thisslope > biggestslope)
                                            biggestslope = thisslope;
                                    }
                                }
                            }

                            biggestslope = biggestslope - 240; // 180

                            if (biggestslope < 0)
                                biggestslope = 0;

                            float adjustedelev = (float)mapelev;

                            adjustedelev = adjustedelev * (biggestslope / 240.0f);

                            mapelev = (int)adjustedelev;
                        }

                        int newbase1, newbase2, newbase3, newgrass1, newgrass2, newgrass3;

                        if (desertmapelev > 2000)
                        {
                            newdesert1 = world.highdesert1();
                            newdesert2 = world.highdesert2();
                            newdesert3 = world.highdesert3();
                        }
                        else
                        {
                            int highno = desertmapelev / 50;
                            int lowno = 40 - highno;

                            newdesert1 = (highno * world.highdesert1() + lowno * newdesert1) / 40;
                            newdesert2 = (highno * world.highdesert2() + lowno * newdesert2) / 40;
                            newdesert3 = (highno * world.highdesert3() + lowno * newdesert3) / 40;
                        }

                        if (mapelev > 3000)
                        {
                            newbase1 = world.highbase1();
                            newbase2 = world.highbase2();
                            newbase3 = world.highbase3();

                            newgrass1 = world.highbase1();
                            newgrass2 = world.highbase2();
                            newgrass3 = world.highbase3();
                        }
                        else
                        {
                            int highno = mapelev / 75;
                            int lowno = 40 - highno;

                            newbase1 = (highno * world.highbase1() + lowno * thisbase1) / 40;
                            newbase2 = (highno * world.highbase2() + lowno * thisbase2) / 40;
                            newbase3 = (highno * world.highbase3() + lowno * thisbase3) / 40;

                            newgrass1 = (highno * world.highbase1() + lowno * world.grass1()) / 40;
                            newgrass2 = (highno * world.highbase2() + lowno * world.grass2()) / 40;
                            newgrass3 = (highno * world.highbase3() + lowno * world.grass3()) / 40;
                        }

                        // Now we need to mix these according to how dry the location is.

                        if (totalrain > 800.0f) // 800
                        {
                            colour1 = newbase1;
                            colour2 = newbase2;
                            colour3 = newbase3;
                        }
                        else
                        {
                            if (totalrain > 200.0f) //400
                            {
                                int wetno = ((int)totalrain - 200) / 40; //400 20
                                if (wetno > 20)
                                    wetno = 20;
                                int dryno = 20 - wetno;

                                colour1 = (wetno * newbase1 + dryno * newgrass1) / 20;
                                colour2 = (wetno * newbase2 + dryno * newgrass2) / 20;
                                colour3 = (wetno * newbase3 + dryno * newgrass3) / 20;
                            }
                            else
                            {
                                float ftotalrain = 200.0f - totalrain; // 400

                                ftotalrain = ftotalrain / 200.0f; // 400

                                int powamount = (int)totalrain - 150; // 350 This is to make a smoother transition.

                                if (powamount < 3)
                                    powamount = 3;

                                ftotalrain = (float)pow(ftotalrain, powamount);

                                ftotalrain = ftotalrain * 200.0f; // 400

                                totalrain = 200.0f - ftotalrain; // 400

                                int wetno = (int)totalrain;
                                int dryno = 200 - wetno;

                                colour1 = (wetno * newgrass1 + dryno * newdesert1) / 200;
                                colour2 = (wetno * newgrass2 + dryno * newdesert2) / 200;
                                colour3 = (wetno * newgrass3 + dryno * newdesert3) / 200;
                            }
                        }

                        // Now we need to alter that according to how cold the location is.

                        if (avetemp <= 0)
                        {
                            colour1 = world.cold1();
                            colour2 = world.cold2();
                            colour3 = world.cold3();
                        }
                        else
                        {
                            // Get the right tundra colour, depending on latitude.

                            int lat = 0;
                            int latminutes = 0;
                            int latseconds = 0;
                            bool latneg = 0;

                            world.latitude(j, lat, latminutes, latseconds,latneg);

                            int lat2 = 90 - lat;

                            int thistundra1 = (lat * world.tundra1() + lat2 * world.eqtundra1()) / 90;
                            int thistundra2 = (lat * world.tundra2() + lat2 * world.eqtundra2()) / 90;
                            int thistundra3 = (lat * world.tundra3() + lat2 * world.eqtundra3()) / 90;

                            if (world.snowchange() == 1) // Abrupt transition
                            {
                                if (avetemp < 20)
                                {
                                    if (avetemp < 6)
                                    {
                                        colour1 = world.cold1();
                                        colour2 = world.cold2();
                                        colour3 = world.cold3();
                                    }
                                    else
                                    {
                                        if (avetemp < 10)
                                        {
                                            colour1 = thistundra1;
                                            colour2 = thistundra2;
                                            colour3 = thistundra3;
                                        }
                                        else
                                        {
                                            int hotno = avetemp - 10;
                                            int coldno = 10 - hotno;

                                            colour1 = (hotno * colour1 + coldno * thistundra1) / 10;
                                            colour2 = (hotno * colour2 + coldno * thistundra2) / 10;
                                            colour3 = (hotno * colour3 + coldno * thistundra3) / 10;
                                        }
                                    }
                                }
                            }

                            if (world.snowchange() == 2) // Speckled transition
                            {
                                if (avetemp < 20)
                                {
                                    if (avetemp < 6)
                                    {
                                        colour1 = world.cold1();
                                        colour2 = world.cold2();
                                        colour3 = world.cold3();
                                    }
                                    else
                                    {
                                        if (avetemp < 10)
                                        {
                                            if (random(6, 10) < avetemp)
                                            {
                                                colour1 = thistundra1;
                                                colour2 = thistundra2;
                                                colour3 = thistundra3;
                                            }
                                            else
                                            {
                                                colour1 = world.cold1();
                                                colour2 = world.cold2();
                                                colour3 = world.cold3();
                                            }
                                        }
                                        else
                                        {
                                            int hotno = avetemp - 10;
                                            int coldno = 10 - hotno;

                                            colour1 = (hotno * colour1 + coldno * thistundra1) / 10;
                                            colour2 = (hotno * colour2 + coldno * thistundra2) / 10;
                                            colour3 = (hotno * colour3 + coldno * thistundra3) / 10;
                                        }
                                    }
                                }
                            }

                            if (world.snowchange() == 3) // Gradual transition
                            {
                                if (avetemp < 20)
                                {
                                    if (avetemp < 10)
                                    {
                                        int hotno = avetemp;
                                        int coldno = 10 - hotno;

                                        colour1 = (hotno * thistundra1 + coldno * world.cold1()) / 10;
                                        colour2 = (hotno * thistundra2 + coldno * world.cold2()) / 10;
                                        colour3 = (hotno * thistundra3 + coldno * world.cold3()) / 10;
                                    }
                                    else
                                    {
                                        int hotno = avetemp - 10;
                                        int coldno = 10 - hotno;

                                        colour1 = (hotno * colour1 + coldno * thistundra1) / 10;
                                        colour2 = (hotno * colour2 + coldno * thistundra2) / 10;
                                        colour3 = (hotno * colour3 + coldno * thistundra3) / 10;
                                    }
                                }
                            }
                        }

                        // Now add sand, if need be.

                        int special = world.special(i, j);

                        if (special == 120)
                        {
                            colour1 = (colour1 * 2 + world.erg1()) / 3;
                            colour2 = (colour2 * 2 + world.erg2()) / 3;
                            colour3 = (colour3 * 2 + world.erg3()) / 3;
                        }

                        // Now wetlands.

                        if (special >= 130 && special < 140)
                        {
                            colour1 = (colour1 * 2 + world.wetlands1()) / 3;
                            colour2 = (colour2 * 2 + world.wetlands2()) / 3;
                            colour3 = (colour3 * 2 + world.wetlands3()) / 3;
                        }
                    }
                }
            }

            if (world.sea(i, j) == 1)
            {
                int amount = randomsign(random(0, var));

                colour1 = colour1 + amount;
                colour2 = colour2 + amount;
                colour3 = colour3 + amount;
            }
            else
            {
                colour1 = colour1 + randomsign(random(0, var));
                colour2 = colour2 + randomsign(random(0, var));
                colour3 = colour3 + randomsign(random(0, var));

                if (world.truelake(i, j) != 0)
                {
                    colour1 = world.lake1();
                    colour2 = world.lake2();
                    colour3 = world.lake3();
                }
                else
                {
                    if (world.riftlakesurface(i, j) != 0)
                    {
                        colour1 = world.lake1();
                        colour2 = world.lake2();
                        colour3 = world.lake3();
                    }
                }
            }

            if (colour1 > 255)
                colour1 = 255;
            if (colour2 > 255)
                colour2 = 255;
            if (colour3 > 255)
                colour3 = 255;

            if (colour1 < 0)
                colour1 = 0;
            if (colour2 < 0)
                colour2 = 0;
            if (colour3 < 0)
                colour3 = 0;

            reliefmap1[i][j] = colour1;
            reliefmap2[i][j] = colour2;
            reliefmap3[i][j] = colour3;
        }
    }

    // Now apply that to the image, adding shading for slopes where appropriate.

    short r, g, b;

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            r = reliefmap1[i][j];
            g = reliefmap2[i][j];
            b = reliefmap3[i][j];

            if (world.noshade(i, j) == 0)
            {
                bool goahead = 1;

                if ((world.seaice(i, j) == 2 && (world.seaiceappearance() == 1 || world.seaiceappearance() == 3)) || (world.seaice(i, j) == 1 && world.seaiceappearance() == 3)) // Sea ice
                    goahead = 0;

                if (goahead == 1 || world.sea(i, j) == 0)
                {
                    int slope1 = 0;
                    int slope2 = 0;

                    if (shadingdir == 2)
                    {
                        slope1 = getslope(world, i, j, i - 1, j);
                        slope2 = getslope(world, i, j, i, j + 1);
                    }

                    if (shadingdir == 4)
                    {
                        slope1 = getslope(world, i, j, i - 1, j);
                        slope2 = getslope(world, i, j, i, j - 1);
                    }

                    if (shadingdir == 6)
                    {
                        slope1 = getslope(world, i, j, i + 1, j);
                        slope2 = getslope(world, i, j, i, j - 1);
                    }

                    if (shadingdir == 8)
                    {
                        slope1 = getslope(world, i, j, i + 1, j);
                        slope2 = getslope(world, i, j, i, j + 1);
                    }

                    if (slope1 != -1 && slope2 != -1)
                    {
                        int totalslope = (slope1 + slope2) / 10;

                        if (totalslope > 40)
                            totalslope = 40;

                        if (totalslope < -40)
                            totalslope = -40;

                        if (world.sea(i, j) == 1)
                            totalslope = (int)((float)totalslope * (seashading * 2.0f));
                        else
                        {
                            if (world.truelake(i, j) == 1)
                                totalslope = (int)((float)totalslope * (lakeshading * 2.0f));
                            else
                                totalslope = (int)((float)totalslope * (landshading * 2.0f));
                        }

                        if (world.map(i, j) <= sealevel && world.oceanrifts(i, j) == 0) // Reduce the shading effect around ocean ridges.
                        {
                            int amount = 1;
                            int amount2 = 3;
                            bool found = 0;
                            bool ignore = 0;

                            for (int k = i - amount2; k <= i + amount2; k++) // don't do this around the rift itself
                            {
                                int kk = k;

                                if (kk<0 || kk>width)
                                    kk = wrap(kk, width);

                                for (int l = j - amount2; l <= j + amount2; l++)
                                {
                                    if (l >= 0 && l <= height)
                                    {
                                        if (world.oceanrifts(kk, l) != 0)
                                        {
                                            ignore = 1;
                                            k = i + amount2;
                                            l = j + amount2;
                                        }
                                    }
                                }
                            }

                            if (ignore == 0)
                            {
                                for (int k = i - amount; k <= i + amount; k++)
                                {
                                    int kk = k;

                                    if (kk<0 || kk>width)
                                        kk = wrap(kk, width);

                                    for (int l = j - amount; l <= j + amount; l++)
                                    {
                                        if (l >= 0 && l <= height)
                                        {
                                            if (world.oceanridges(kk, l) != 0)
                                            {
                                                found = 1;
                                                k = i + amount;
                                                l = j + amount;
                                            }
                                        }
                                    }
                                }

                                if (found == 1)
                                    totalslope = totalslope / 4;
                            }
                        }


                        r = r + totalslope;
                        g = g + totalslope;
                        b = b + totalslope;
                    }

                    if (r < 0)
                        r = 0;
                    if (g < 0)
                        g = 0;
                    if (b < 0)
                        b = 0;

                    if (r > 255)
                        r = 255;
                    if (g > 255)
                        g = 255;
                    if (b > 255)
                        b = 255;
                }
            }

            globalreliefimage.setPixel(i, j, sf::Color((sf::Uint8)r, (sf::Uint8)g, (sf::Uint8)b));
        }
    }

    float displayx = DISPLAYMAPSIZEX;

    float mapdiv = ((float)width + 1.0f) / displayx;

    for (float i = 0.0f; i < DISPLAYMAPSIZEX; i++)
    {
        float pixelx = i * mapdiv;

        for (float j = 0.0f; j < DISPLAYMAPSIZEY; j++)
        {
            float pixely = j * mapdiv;

            sf::Color thispixel = globalreliefimage.getPixel((int)pixelx, (int)pixely);

            displayglobalreliefimage.setPixel((int)i, (int)j, thispixel);
        }
    }
}

// This function gets colours for drawing climate maps.

sf::Color getclimatecolours(short climate)
{
    int colour1 = 0;
    int colour2 = 0;
    int colour3 = 0;

    if (climate == 1)
    {
        colour1 = 0;
        colour2 = 0;
        colour3 = 254;
    }

    if (climate == 2)
    {
        colour1 = 1;
        colour2 = 119;
        colour3 = 255;
    }

    if (climate == 3)
    {
        colour1 = 70;
        colour2 = 169;
        colour3 = 250;
    }

    if (climate == 4)
    {
        colour1 = 70;
        colour2 = 169;
        colour3 = 250;
    }

    if (climate == 5)
    {
        colour1 = 249;
        colour2 = 15;
        colour3 = 0;
    }

    if (climate == 6)
    {
        colour1 = 251;
        colour2 = 150;
        colour3 = 149;
    }

    if (climate == 7)
    {
        colour1 = 245;
        colour2 = 163;
        colour3 = 1;
    }

    if (climate == 8)
    {
        colour1 = 254;
        colour2 = 219;
        colour3 = 99;
    }

    if (climate == 9)
    {
        colour1 = 255;
        colour2 = 255;
        colour3 = 0;
    }

    if (climate == 10)
    {
        colour1 = 198;
        colour2 = 199;
        colour3 = 1;
    }

    if (climate == 11)
    {
        colour1 = 184;
        colour2 = 184;
        colour3 = 114;
    }

    if (climate == 12)
    {
        colour1 = 138;
        colour2 = 255;
        colour3 = 162;
    }

    if (climate == 13)
    {
        colour1 = 86;
        colour2 = 199;
        colour3 = 112;
    }

    if (climate == 14)
    {
        colour1 = 30;
        colour2 = 150;
        colour3 = 66;
    }

    if (climate == 15)
    {
        colour1 = 192;
        colour2 = 254;
        colour3 = 109;
    }

    if (climate == 16)
    {
        colour1 = 76;
        colour2 = 255;
        colour3 = 93;
    }

    if (climate == 17)
    {
        colour1 = 19;
        colour2 = 203;
        colour3 = 74;
    }

    if (climate == 18)
    {
        colour1 = 255;
        colour2 = 8;
        colour3 = 245;
    }

    if (climate == 19)
    {
        colour1 = 204;
        colour2 = 3;
        colour3 = 192;
    }

    if (climate == 20)
    {
        colour1 = 154;
        colour2 = 51;
        colour3 = 144;
    }

    if (climate == 21)
    {
        colour1 = 153;
        colour2 = 100;
        colour3 = 146;
    }

    if (climate == 22)
    {
        colour1 = 172;
        colour2 = 178;
        colour3 = 249;
    }

    if (climate == 23)
    {
        colour1 = 91;
        colour2 = 121;
        colour3 = 213;
    }

    if (climate == 24)
    {
        colour1 = 78;
        colour2 = 83;
        colour3 = 175;
    }

    if (climate == 25)
    {
        colour1 = 54;
        colour2 = 3;
        colour3 = 130;
    }

    if (climate == 26)
    {
        colour1 = 0;
        colour2 = 255;
        colour3 = 245;
    }

    if (climate == 27)
    {
        colour1 = 32;
        colour2 = 200;
        colour3 = 250;
    }

    if (climate == 28)
    {
        colour1 = 0;
        colour2 = 126;
        colour3 = 125;
    }

    if (climate == 29)
    {
        colour1 = 0;
        colour2 = 69;
        colour3 = 92;
    }

    if (climate == 30)
    {
        colour1 = 178;
        colour2 = 178;
        colour3 = 178;
    }

    if (climate == 31)
    {
        colour1 = 104;
        colour2 = 104;
        colour3 = 104;
    }

    sf::Color colour(colour1, colour2, colour3);

    return (colour);
}

// These functions draw a regional map image (ready to be applied to a texture).

void drawregionalmapimage(mapviewenum mapview, planet& world, region& region, bool regionalmapimagecreated[], sf::Image& regionalelevationimage, sf::Image& regionaltemperatureimage, sf::Image& regionalprecipitationimage, sf::Image& regionalclimateimage, sf::Image& regionalriversimage, sf::Image& regionalreliefimage)
{
    if (mapview == elevation)
    {
        if (regionalmapimagecreated[0] == 1)
            return;

        drawregionalelevationmapimage(world, region, regionalelevationimage);

        regionalmapimagecreated[0] = 1;
    }

    if (mapview == temperature)
    {
        if (regionalmapimagecreated[2] == 1)
            return;

        drawregionaltemperaturemapimage(world, region, regionaltemperatureimage);

        regionalmapimagecreated[2] = 1;
    }

    if (mapview == precipitation)
    {
        if (regionalmapimagecreated[3] == 1)
            return;

        drawregionalprecipitationmapimage(world, region, regionalprecipitationimage);

        regionalmapimagecreated[3] = 1;
    }

    if (mapview == climate)
    {
        if (regionalmapimagecreated[4] == 1)
            return;

        drawregionalclimatemapimage(world, region, regionalclimateimage);

        regionalmapimagecreated[4] = 1;
    }

    if (mapview == rivers)
    {
        if (regionalmapimagecreated[5] == 1)
            return;

        drawregionalriversmapimage(world, region, regionalriversimage);

        regionalmapimagecreated[5] = 1;
    }

    if (mapview == relief)
    {
        if (regionalmapimagecreated[6] == 1)
            return;

        drawregionalreliefmapimage(world, region, regionalreliefimage);

        regionalmapimagecreated[6] = 1;
    }
}

void drawregionalelevationmapimage(planet& world, region& region, sf::Image& regionalelevationimage)
{
    int origregwidthbegin = region.regwidthbegin();
    int origregwidthend = region.regwidthend();
    int origregheightbegin = region.regheightbegin();
    int origregheightend = region.regheightend();

    int regwidthbegin = origregwidthbegin;
    int regwidthend = origregwidthend;
    int regheightbegin = origregheightbegin;
    int regheightend = origregheightend;

    int colour1, colour2, colour3;

    int div = world.maxelevation() / 255;
    int base = world.maxelevation() / 4;

    for (int i = regwidthbegin; i <= regwidthend; i++)
    {
        for (int j = regheightbegin; j <= regheightend; j++)
        {
            int heightpoint = region.map(i, j);

            if (region.special(i, j) > 100 && region.special(i, j) < 130)
                heightpoint = region.lakesurface(i, j);

            colour1 = heightpoint / div;

            if (colour1 > 255)
                colour1 = 255;

            colour2 = colour1;
            colour3 = colour2;

            regionalelevationimage.setPixel(i - origregwidthbegin, j - origregheightbegin, sf::Color(colour1, colour2, colour3));
        }
    }
}

void drawregionaltemperaturemapimage(planet& world, region& region, sf::Image& regionaltemperatureimage)
{
    int origregwidthbegin = region.regwidthbegin();
    int origregwidthend = region.regwidthend();
    int origregheightbegin = region.regheightbegin();
    int origregheightend = region.regheightend();

    int regwidthbegin = origregwidthbegin;
    int regwidthend = origregwidthend;
    int regheightbegin = origregheightbegin;
    int regheightend = origregheightend;

    int colour1, colour2, colour3;

    int div = world.maxelevation() / 255;
    int base = world.maxelevation() / 4;

    for (int i = regwidthbegin; i <= regwidthend; i++)
    {
        for (int j = regheightbegin; j <= regheightend; j++)
        {
            if (region.outline(i, j))
            {
                colour1 = 0;
                colour2 = 0;
                colour3 = 0;
            }
            else
            {
                int temperature = (region.mintemp(i, j) + region.maxtemp(i, j)) / 2;

                temperature = temperature + 10;

                if (temperature > 0)
                {
                    colour1 = 250;
                    colour2 = 250 - (temperature * 3);
                    colour3 = 250 - (temperature * 7);
                }
                else
                {
                    temperature = abs(temperature);

                    colour1 = 250 - (temperature * 7);
                    colour2 = 250 - (temperature * 7);
                    colour3 = 250;
                }

                if (colour1 < 0)
                    colour1 = 0;

                if (colour2 < 0)
                    colour2 = 0;

                if (colour3 < 0)
                    colour3 = 0;
            }

            regionaltemperatureimage.setPixel(i - origregwidthbegin, j - origregheightbegin, sf::Color(colour1, colour2, colour3));
        }
    }
}

void drawregionalprecipitationmapimage(planet& world, region& region, sf::Image& regionalprecipitationimage)
{
    int origregwidthbegin = region.regwidthbegin();
    int origregwidthend = region.regwidthend();
    int origregheightbegin = region.regheightbegin();
    int origregheightend = region.regheightend();

    int regwidthbegin = origregwidthbegin;
    int regwidthend = origregwidthend;
    int regheightbegin = origregheightbegin;
    int regheightend = origregheightend;

    int colour1, colour2, colour3;

    int div = world.maxelevation() / 255;
    int base = world.maxelevation() / 4;

    for (int i = regwidthbegin; i <= regwidthend; i++)
    {
        for (int j = regheightbegin; j <= regheightend; j++)
        {
            if (region.outline(i, j))
            {
                colour1 = 0;
                colour2 = 0;
                colour3 = 0;
            }
            else
            {
                int rainfall = (region.summerrain(i, j) + region.winterrain(i, j)) / 2;

                rainfall = rainfall / 4;

                colour1 = 255 - rainfall;
                colour2 = 255 - rainfall;
                colour3 = 255;

                if (colour1 < 0)
                    colour1 = 0;

                if (colour2 < 0)
                    colour2 = 0;
            }

            if (region.test(i, j) != 0)
            {
                colour1 = 255;
                colour2 = 0;
                colour3 = 255;
            }

            regionalprecipitationimage.setPixel(i - origregwidthbegin, j - origregheightbegin, sf::Color(colour1, colour2, colour3));
        }
    }
}

void drawregionalclimatemapimage(planet& world, region& region, sf::Image& regionalclimateimage)
{
    int origregwidthbegin = region.regwidthbegin();
    int origregwidthend = region.regwidthend();
    int origregheightbegin = region.regheightbegin();
    int origregheightend = region.regheightend();

    int regwidthbegin = origregwidthbegin;
    int regwidthend = origregwidthend;
    int regheightbegin = origregheightbegin;
    int regheightend = origregheightend;

    int colour1, colour2, colour3;

    int div = world.maxelevation() / 255;
    int base = world.maxelevation() / 4;

    for (int i = regwidthbegin; i <= regwidthend; i++)
    {
        for (int j = regheightbegin; j <= regheightend; j++)
        {
            if (region.sea(i, j))
            {
                if (region.seaice(i, j) == 2) // Permanent sea ice
                {
                    colour1 = 243;
                    colour2 = 243;
                    colour3 = 255;
                }
                else
                {
                    if (region.seaice(i, j) == 1) // Seasonal sea ice
                    {
                        colour1 = 228;
                        colour2 = 228;
                        colour3 = 255;
                    }
                    else // Open sea
                    {
                        colour1 = 13;
                        colour2 = 49;
                        colour3 = 109;
                    }
                }

                regionalclimateimage.setPixel(i - origregwidthbegin, j - origregheightbegin, sf::Color(colour1, colour2, colour3));
            }
            else
            {
                sf::Color landcolour = getclimatecolours(region.climate(i, j));

                regionalclimateimage.setPixel(i - origregwidthbegin, j - origregheightbegin, landcolour);
            }
        }
    }
}

void drawregionalriversmapimage(planet& world, region& region, sf::Image& regionalriversimage)
{
    int origregwidthbegin = region.regwidthbegin();
    int origregwidthend = region.regwidthend();
    int origregheightbegin = region.regheightbegin();
    int origregheightend = region.regheightend();

    int regwidthbegin = origregwidthbegin;
    int regwidthend = origregwidthend;
    int regheightbegin = origregheightbegin;
    int regheightend = origregheightend;

    int colour1, colour2, colour3;

    int div = world.maxelevation() / 255;
    int base = world.maxelevation() / 4;

    int mult = world.maxriverflow() / 400;

    if (mult < 1)
        mult = 1;

    for (int i = regwidthbegin; i <= regwidthend; i++)
    {
        for (int j = regheightbegin; j <= regheightend; j++)
        {
            if (region.outline(i, j))
            {
                colour1 = 0;
                colour2 = 0;
                colour3 = 0;
            }
            else
            {
                int flow = region.riveraveflow(i, j);

                if (flow > 0) // && region.sea(i, j) == 0)
                {
                    flow = flow * 100;

                    colour1 = 255 - (flow / mult);
                    if (colour1 < 0)
                        colour1 = 0;
                    colour2 = colour1;
                }
                else
                {
                    colour1 = 255;
                    colour2 = 255;
                }

                colour3 = 255;

                if (region.truelake(i, j) != 0)
                {
                    colour1 = 150;
                    colour2 = 150;
                    colour3 = 250;
                }

                if (region.volcano(i, j) == 1)
                {
                    colour1 = 240;
                    colour2 = 0;
                    colour3 = 0;
                    colour3 = 0;
                }

                if (colour1 == 255 && colour2 == 255 && colour3 == 255)
                {
                    int special = region.special(i, j);

                    if (special >= 130 && special < 140)
                    {
                        colour1 = 30;
                        colour2 = 250;
                        colour3 = 150;
                    }

                    if (region.mud(i, j))
                    {
                        colour1 = 131;
                        colour2 = 98;
                        colour3 = 75;
                    }

                    if (region.sand(i, j) || region.shingle(i, j))
                    {
                        colour1 = 255;
                        colour2 = 255;
                        colour3 = 50;
                    }
                }
            }
            regionalriversimage.setPixel(i - origregwidthbegin, j - origregheightbegin, sf::Color(colour1, colour2, colour3));
        }
    }
}

void drawregionalreliefmapimage(planet& world, region& region, sf::Image& regionalreliefimage)
{
    int origregwidthbegin = region.regwidthbegin();
    int origregwidthend = region.regwidthend();
    int origregheightbegin = region.regheightbegin();
    int origregheightend = region.regheightend();

    int regwidthbegin = origregwidthbegin;
    int regwidthend = origregwidthend;
    int regheightbegin = origregheightbegin;
    int regheightend = origregheightend;

    int colour1, colour2, colour3;

    int div = world.maxelevation() / 255;
    int base = world.maxelevation() / 4;

    int mult = world.maxriverflow() / 400;

    int leftx = region.leftx();
    int lefty = region.lefty();

    int rwidth = region.rwidth();
    int rheight = region.rheight();

    int type = world.type();
    int sealevel = world.sealevel();
    int minriverflow = world.minriverflowregional(); // Rivers of this size or larger will be shown on the map.
    int shadingdir = world.shadingdir();

    float landshading = world.landshading();
    float lakeshading = world.lakeshading();
    float seashading = world.seashading();

    float landmarbling = world.landmarbling() * 2;
    float lakemarbling = world.lakemarbling() * 2;
    float seamarbling = world.seamarbling() * 2;

    bool colourcliffs = world.colourcliffs();

    int width = world.width();
    int height = world.height();
    int xleft = 0;
    int xright = 35;
    int ytop = 0;
    int ybottom = 35;

    vector<vector<short>> reliefmap1(RARRAYWIDTH, vector<short>(RARRAYHEIGHT, 0));
    vector<vector<short>> reliefmap2(RARRAYWIDTH, vector<short>(RARRAYHEIGHT, 0));
    vector<vector<short>> reliefmap3(RARRAYWIDTH, vector<short>(RARRAYHEIGHT, 0));

    // Make a fractal based on rainfall, which will be used to add stripes to vary the colours.

    vector<vector<int>> source(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));
    vector<vector<int>> stripefractal(RARRAYWIDTH, vector<int>(RARRAYHEIGHT, -5000));

    for (int i = 0; i <= width; i++)
    {
        for (int j = 0; j <= height; j++)
        {
            if (world.wintermountainraindir(i, j) == 0)
                source[i][j] = world.winterrain(i, j);
            else
                source[i][j] = world.wintermountainrain(i, j);
        }
    }

    int var = 0; // Amount colours may be varied to make the map seem more speckledy.

    int lat = 0;
    int lat2 = 0;
    int latminutes = 0;
    int latseconds = 0;
    bool latneg = 0;

    for (int i = regwidthbegin; i <= regwidthend; i++)
    {
        for (int j = regheightbegin; j <= regheightend; j++)
        {
            bool stripe = 1; // Indicates whether we're going to do a stripe here.

            int xx = leftx + (i / 16);
            int yy = lefty + (j / 16); // Coordinates of the relevant global cell.

            int avetemp = (region.extramaxtemp(i, j) + region.extramintemp(i, j)) / 2;

            avetemp = avetemp + 1000;

            int totalrain = region.summerrain(i, j) + region.winterrain(i, j);

            if (region.special(i, j) == 120)
                totalrain = totalrain / 4;

            var = 10;

            int sea = region.sea(i, j);

            if (sea == 1)
            {
                if ((region.seaice(i, j) == 2 && (world.seaiceappearance() == 1 || world.seaiceappearance() == 3)) || (region.seaice(i, j) == 1 && world.seaiceappearance() == 3)) // Sea ice
                {
                    colour1 = world.seaice1();
                    colour2 = world.seaice2();
                    colour3 = world.seaice3();

                    stripe = 0;

                    var = 0;
                }
                else
                {
                    colour1 = (world.ocean1() * region.map(i, j) + world.deepocean1() * (sealevel - region.map(i, j))) / sealevel;
                    colour2 = (world.ocean2() * region.map(i, j) + world.deepocean2() * (sealevel - region.map(i, j))) / sealevel;
                    colour3 = (world.ocean3() * region.map(i, j) + world.deepocean3() * (sealevel - region.map(i, j))) / sealevel;

                    var = 5;
                }
            }
            else
            {
                if ((region.riverjan(i, j) + region.riverjul(i, j)) / 2 >= minriverflow || (region.fakejan(i, j) + region.fakejul(i, j)) / 2 >= minriverflow)
                {
                    colour1 = world.river1();
                    colour2 = world.river2();
                    colour3 = world.river3();

                    stripe = 0;
                }
                else
                {
                    if (region.special(i, j) == 110) // Salt pan
                    {
                        colour1 = world.saltpan1();
                        colour2 = world.saltpan2();
                        colour3 = world.saltpan3();

                        var = 20;

                        stripe = 0;
                    }
                    else
                    {
                        if (region.sand(i, j) || region.shingle(i,j))
                        {
                            if (region.sand(i, j) && region.shingle(i, j) == 0)
                            {
                                colour1 = world.sand1();
                                colour2 = world.sand2();
                                colour3 = world.sand3();

                                var = 5;
                            }

                            if (region.shingle(i, j) && region.sand(i, j) == 0)
                            {
                                colour1 = world.shingle1();
                                colour2 = world.shingle2();
                                colour3 = world.shingle3();

                                var = 40;
                            }

                            if (region.sand(i, j) && region.shingle(i, j))
                            {
                                colour1 = (world.sand1() + world.shingle1()) / 2;
                                colour2 = (world.sand2() + world.shingle2()) / 2;
                                colour3 = (world.sand3() + world.shingle3()) / 2;

                                var = 20;
                            }

                            if (region.mud(i, j))
                            {
                                colour1 = (colour1 + world.mud1()) / 2;
                                colour2 = (colour2 + world.mud2()) / 2;
                                colour3 = (colour3 + world.mud3()) / 2;
                            }
                        }
                        else
                        {
                            if (region.mangrove(i, j) && world.showmangroves())
                            {
                                colour1 = world.mangrove1();
                                colour2 = world.mangrove2();
                                colour3 = world.mangrove3();

                                var = 20;
                            }
                            else
                            {
                                // First, adjust the base colours depending on temperature.

                                int thisbase1, thisbase2, thisbase3, newdesert1, newdesert2, newdesert3;

                                if (avetemp > 3000)
                                {
                                    thisbase1 = world.base1();
                                    thisbase2 = world.base2();
                                    thisbase3 = world.base3();
                                }
                                else
                                {
                                    int hotno = avetemp / 3;
                                    int coldno = 1000 - hotno;

                                    thisbase1 = (hotno * world.base1() + coldno * world.basetemp1()) / 1000;
                                    thisbase2 = (hotno * world.base2() + coldno * world.basetemp2()) / 1000;
                                    thisbase3 = (hotno * world.base3() + coldno * world.basetemp3()) / 1000;
                                }

                                if (avetemp > 30)
                                {
                                    newdesert1 = world.desert1();
                                    newdesert2 = world.desert2();
                                    newdesert3 = world.desert3();
                                }
                                else
                                {
                                    if (avetemp <= 10)
                                    {
                                        newdesert1 = world.colddesert1();
                                        newdesert2 = world.colddesert2();
                                        newdesert3 = world.colddesert3();
                                    }
                                    else
                                    {
                                        int hotno = avetemp - 10;
                                        int coldno = 20 - hotno;

                                        newdesert1 = (hotno * world.desert1() + coldno * world.colddesert1()) / 20;
                                        newdesert2 = (hotno * world.desert2() + coldno * world.colddesert2()) / 20;
                                        newdesert3 = (hotno * world.desert3() + coldno * world.colddesert3()) / 20;
                                    }
                                }

                                // The closer it is to tropical rainforest, the more we intensify the rain effect.

                                float rainforestmult = (float)region.mintemp(i, j) / 18.0f;

                                rainforestmult = rainforestmult * (float)region.winterrain(i, j) / 80.0f; //30.0f;

                                if (rainforestmult < 1.0f)
                                    rainforestmult = 1.0f;

                                totalrain = totalrain * (int)rainforestmult;

                                // Now adjust the colours for height.

                                int mapelev = region.map(i, j) - sealevel;

                                if (region.special(i, j) == 110 || region.special(i, j) == 120) // If this is going to be an erg or salt pan
                                    mapelev = region.lakesurface(i, j) - sealevel; // Use the surface elevation of the erg or salt pan

                                int desertmapelev = mapelev; // We won't mess about with this one.

                                // If it's flat here and this setting is enabled, pretend the elevation is lower than it really is.

                                if (colourcliffs==1)
                                {
                                    int biggestslope = 0;

                                    for (int k = i - 1; k <= i + 1; k++)
                                    {
                                        if (k >= 0 && k <= rwidth)
                                        {
                                            for (int l = j - 1; l <= j + 1; l++)
                                            {
                                                if (l >= 0 && l <= rheight)
                                                {
                                                    int thisslope = mapelev + sealevel - region.map(k, l);

                                                    if (thisslope > biggestslope)
                                                        biggestslope = thisslope;
                                                }
                                            }
                                        }
                                    }

                                    biggestslope = biggestslope - 240; //180

                                    if (biggestslope < 0)
                                        biggestslope = 0;

                                    float adjustedelev = (float)mapelev;

                                    adjustedelev = adjustedelev * (biggestslope / 240.0f);

                                    if (adjustedelev > (float)mapelev)
                                        adjustedelev = (float)mapelev;

                                    mapelev = (int)adjustedelev;
                                }

                                int newbase1, newbase2, newbase3, newgrass1, newgrass2, newgrass3;

                                if (desertmapelev > 2000)
                                {
                                    newdesert1 = world.highdesert1();
                                    newdesert2 = world.highdesert2();
                                    newdesert3 = world.highdesert3();
                                }
                                else
                                {
                                    int highno = desertmapelev / 50;
                                    int lowno = 40 - highno;

                                    newdesert1 = (highno * world.highdesert1() + lowno * newdesert1) / 40;
                                    newdesert2 = (highno * world.highdesert2() + lowno * newdesert2) / 40;
                                    newdesert3 = (highno * world.highdesert3() + lowno * newdesert3) / 40;
                                }

                                if (mapelev > 3000)
                                {
                                    newbase1 = world.highbase1();
                                    newbase2 = world.highbase2();
                                    newbase3 = world.highbase3();

                                    newgrass1 = world.highbase1();
                                    newgrass2 = world.highbase2();
                                    newgrass3 = world.highbase3();
                                }
                                else
                                {
                                    int highno = mapelev / 75;
                                    int lowno = 40 - highno;

                                    newbase1 = (highno * world.highbase1() + lowno * thisbase1) / 40;
                                    newbase2 = (highno * world.highbase2() + lowno * thisbase2) / 40;
                                    newbase3 = (highno * world.highbase3() + lowno * thisbase3) / 40;

                                    newgrass1 = (highno * world.highbase1() + lowno * world.grass1()) / 40;
                                    newgrass2 = (highno * world.highbase2() + lowno * world.grass2()) / 40;
                                    newgrass3 = (highno * world.highbase3() + lowno * world.grass3()) / 40;
                                }

                                // Now we need to mix these according to how dry the location is.

                                if (region.rivervalley(i, j) == 1 || region.riverdir(i,j)!=0 || region.fakedir(i,j)!=0) // If this is a river valley, it's wetter than it would otherwise be.
                                {
                                    float biggestflow = 0.0f;

                                    for (int k = i - 20; k <= i + 20; k++)
                                    {
                                        for (int l = j - 20; l <= j + 20; l++)
                                        {
                                            if (k >= 0 && k <= rwidth && l >= 0 && l <= rheight)
                                            {
                                                if (region.riverjan(k, l) + region.riverjul(k, l) > biggestflow)
                                                    biggestflow = (float)(region.riverjan(k, l) + region.riverjul(k, l));
                                            }
                                        }
                                    }

                                    if (biggestflow == 0.0f)
                                    {
                                        twointegers nearest = findclosestriverquickly(region, i, j);

                                        if (nearest.x != -1)
                                            biggestflow = (float)(region.riverjan(nearest.x, nearest.y) + region.riverjul(nearest.x, nearest.y));
                                    }

                                    if (biggestflow > 12000.0f)
                                        biggestflow = 1200.0f;

                                    float mult = (float)totalrain;

                                    if (mult < 1.0f)
                                        mult = 1.0f;

                                    if (mult > 1000.0f)
                                        mult = 1000.0f;


                                    biggestflow = biggestflow / mult;
                                    totalrain = totalrain + (int)biggestflow;
                                }

                                if (totalrain > 800) // 600
                                {
                                    colour1 = newbase1;
                                    colour2 = newbase2;
                                    colour3 = newbase3;
                                }
                                else
                                {
                                    if (totalrain > 200) //300
                                    {
                                        int wetno = (totalrain - 200) / 40; //20
                                        if (wetno > 20) // 40
                                            wetno = 20;
                                        int dryno = 20 - wetno;


                                        colour1 = (wetno * newbase1 + dryno * newgrass1) / 20;
                                        colour2 = (wetno * newbase2 + dryno * newgrass2) / 20;
                                        colour3 = (wetno * newbase3 + dryno * newgrass3) / 20;
                                    }
                                    else
                                    {
                                        float ftotalrain = 200.0f - (float)totalrain;

                                        ftotalrain = ftotalrain / 200.0f;

                                        int powamount = totalrain - 150; // This is to make a smoother transition.

                                        if (powamount < 3)
                                            powamount = 3;

                                        ftotalrain = (float)pow((double)ftotalrain, (double)powamount);

                                        ftotalrain = ftotalrain * 200.0f;

                                        totalrain = 200 - (int)ftotalrain;

                                        int wetno = totalrain;
                                        int dryno = 200 - wetno;

                                        colour1 = (wetno * newgrass1 + dryno * newdesert1) / 200;
                                        colour2 = (wetno * newgrass2 + dryno * newdesert2) / 200;
                                        colour3 = (wetno * newgrass3 + dryno * newdesert3) / 200;
                                    }
                                }

                                // Now we need to alter that according to how cold the location is.

                                if (avetemp <= 0 || yy > height - 3) // This is because it has an odd tendency to show the very southernmost tiles in non-cold colours.
                                {
                                    colour1 = world.cold1();
                                    colour2 = world.cold2();
                                    colour3 = world.cold3();
                                }
                                else
                                {
                                    // Get the right tundra colour, depending on latitude.

                                    world.latitude(yy, lat, latminutes, latseconds, latneg);

                                    lat2 = 90 - lat;

                                    int thistundra1 = (lat * world.tundra1() + lat2 * world.eqtundra1()) / 90;
                                    int thistundra2 = (lat * world.tundra2() + lat2 * world.eqtundra2()) / 90;
                                    int thistundra3 = (lat * world.tundra3() + lat2 * world.eqtundra3()) / 90;

                                    if (world.snowchange() == 1) // Abrupt transition
                                    {
                                        if (avetemp < 2000)
                                        {
                                            if (avetemp < 600)
                                            {
                                                colour1 = world.cold1();
                                                colour2 = world.cold2();
                                                colour3 = world.cold3();
                                            }
                                            else
                                            {
                                                if (avetemp < 1000)
                                                {
                                                    colour1 = thistundra1;
                                                    colour2 = thistundra2;
                                                    colour3 = thistundra3;
                                                }
                                                else
                                                {
                                                    int hotno = avetemp - 1000;
                                                    int coldno = 1000 - hotno;

                                                    colour1 = (hotno * colour1 + coldno * thistundra1) / 1000;
                                                    colour2 = (hotno * colour2 + coldno * thistundra2) / 1000;
                                                    colour3 = (hotno * colour3 + coldno * thistundra3) / 1000;
                                                }
                                            }
                                        }
                                    }

                                    if (world.snowchange() == 2) // Speckled transition
                                    {
                                        if (avetemp < 2000)
                                        {
                                            if (avetemp < 600)
                                            {
                                                colour1 = world.cold1();
                                                colour2 = world.cold2();
                                                colour3 = world.cold3();
                                            }
                                            else
                                            {
                                                if (avetemp < 1000)
                                                {
                                                    if (random(600, 1000) < avetemp)
                                                    {
                                                        colour1 = thistundra1;
                                                        colour2 = thistundra2;
                                                        colour3 = thistundra3;
                                                    }
                                                    else
                                                    {
                                                        colour1 = world.cold1();
                                                        colour2 = world.cold2();
                                                        colour3 = world.cold3();
                                                    }
                                                }
                                                else
                                                {
                                                    int hotno = avetemp - 1000;
                                                    int coldno = 1000 - hotno;

                                                    colour1 = (hotno * colour1 + coldno * thistundra1) / 1000;
                                                    colour2 = (hotno * colour2 + coldno * thistundra2) / 1000;
                                                    colour3 = (hotno * colour3 + coldno * thistundra3) / 1000;
                                                }
                                            }
                                        }
                                    }

                                    if (world.snowchange() == 3) // Gradual transition
                                    {
                                        if (avetemp < 2000)
                                        {
                                            if (avetemp < 1000)
                                            {
                                                int hotno = avetemp;
                                                int coldno = 1000 - hotno;

                                                colour1 = (hotno * thistundra1 + coldno * world.cold1()) / 1000;
                                                colour2 = (hotno * thistundra2 + coldno * world.cold2()) / 1000;
                                                colour3 = (hotno * thistundra3 + coldno * world.cold3()) / 1000;
                                            }
                                            else
                                            {
                                                int hotno = avetemp - 1000;
                                                int coldno = 1000 - hotno;

                                                colour1 = (hotno * colour1 + coldno * thistundra1) / 1000;
                                                colour2 = (hotno * colour2 + coldno * thistundra2) / 1000;
                                                colour3 = (hotno * colour3 + coldno * thistundra3) / 1000;
                                            }
                                        }
                                    }
                                }

                                // Now add dunes, if need be.

                                int special = region.special(i, j);

                                if (special == 120)
                                {                                    
                                    colour1 = (colour1 * 6 + world.erg1()) / 7;
                                    colour2 = (colour2 * 6 + world.erg2()) / 7;
                                    colour3 = (colour3 * 6 + world.erg3()) / 7;

                                    var = 10; //4;
                                }

                                // Same thing for mud flats.

                                if (region.mud(i, j))
                                {
                                    colour1 = (colour1 + world.mud1() * 2) / 3;
                                    colour2 = (colour2 + world.mud2() * 2) / 3;
                                    colour3 = (colour3 + world.mud3() * 2) / 3;

                                    var = 10;

                                }

                                // Now wetlands.

                                if (special >= 130 && special < 140)
                                {
                                    colour1 = (colour1 * 2 + world.wetlands1()) / 3;
                                    colour2 = (colour2 * 2 + world.wetlands2()) / 3;
                                    colour3 = (colour3 * 2 + world.wetlands3()) / 3;
                                }
                            }
                        }
                    }
                }
            }

            if (region.sea(i, j) == 1)
            {
                int amount = altrandomsign(altrandom(0, var));

                colour1 = colour1 + amount;
                colour2 = colour2 + amount;
                colour3 = colour3 + amount;
            }
            else
            {
                colour1 = colour1 + altrandomsign(altrandom(0, var));
                colour2 = colour2 + altrandomsign(altrandom(0, var));
                colour3 = colour3 + altrandomsign(altrandom(0, var));

                if (region.truelake(i, j) != 0)
                {
                    colour1 = world.lake1();
                    colour2 = world.lake2();
                    colour3 = world.lake3();

                    for (int k = i - 1; k <= i + 1; k++) // If a river is flowing into/out of the lake, make it slightly river coloured.
                    {
                        for (int l = j - 1; l <= j + 1; l++)
                        {
                            if (k >= 0 && k <= rwidth && l >= 0 && l <= rheight)
                            {
                                if (region.riverdir(k, l) != 0 || region.fakedir(k, l) > 0)
                                {
                                    if (region.lakesurface(k, l) == 0)
                                    {
                                        colour1 = (world.lake1() + world.river1()) / 2;
                                        colour2 = (world.lake2() + world.river2()) / 2;
                                        colour3 = (world.lake3() + world.river3()) / 2;

                                        k = i + 1;
                                        l = j + 1;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (region.special(i, j) == 140)
            {
                colour1 = world.glacier1();
                colour2 = world.glacier2();
                colour3 = world.glacier3();

                stripe = 0;
            }

            if (stripe == 1) // If we're doing a stripe here.
            {
                //fast_srand(region.maxtemp(i,j)+region.winterrain(i,j));
                fast_srand(region.maxtemp(i, j) + stripefractal[i][j]);

                float stripevar = (float)avetemp;

                if (stripevar > 5.0f)
                    stripevar = 5.0f;

                if (stripevar < 0.0f)
                    stripevar = 0.0f;

                if (region.special(i, j) > 100)
                    stripevar = 1.0f;

                if (region.sea(i, j) == 1)
                    stripevar = stripevar * seamarbling;
                else
                {
                    if (region.truelake(i, j) == 1)
                        stripevar = stripevar * lakemarbling;
                    else
                        stripevar = stripevar * landmarbling;
                }

                colour1 = colour1 + randomsign(random(0, (int)stripevar));
                colour2 = colour2 + randomsign(random(0, (int)stripevar));
                colour2 = colour2 + randomsign(random(0, (int)stripevar));
            }

            if (colour1 > 255)
                colour1 = 255;
            if (colour2 > 255)
                colour2 = 255;
            if (colour3 > 255)
                colour3 = 255;

            if (colour1 < 0)
                colour1 = 0;
            if (colour2 < 0)
                colour2 = 0;
            if (colour3 < 0)
                colour3 = 0;

            reliefmap1[i][j] = colour1;
            reliefmap2[i][j] = colour2;
            reliefmap3[i][j] = colour3;
        }
    }

    for (int i = regwidthbegin + 1; i <= regwidthend - 1; i++) // Blur ergs, mud flats, and wetlands, and also river valleys..
    {
        for (int j = regheightbegin + 1; j <= regheightend - 1; j++)
        {
            int special = region.special(i, j);

            if ((special == 130 || special == 131 || special == 132 || region.mud(i, j)) && (region.mangrove(i, j) == 0 || world.showmangroves() == 0))
            {
                if ((region.riverjan(i, j) + region.riverjul(i, j)) / 2 < minriverflow || (region.fakejan(i, j) + region.fakejul(i, j)) / 2 < minriverflow) // Don't do it to the rivers themselves.
                {
                    float colred = 0.0f;
                    float colgreen = 0.0f;
                    float colblue = 0.0f;

                    for (int k = i - 1; k <= i + 1; k++)
                    {
                        for (int l = j - 1; l <= j + 1; l++)
                        {
                            colred = colred + (float)reliefmap1[k][l];
                            colgreen = colgreen + (float)reliefmap2[k][l];
                            colblue = colblue + (float)reliefmap3[k][l];
                        }
                    }

                    colred = colred / 9.0f;
                    colgreen = colgreen / 9.0f;
                    colblue = colblue / 9.0f;

                    reliefmap1[i][j] = (short)colred;
                    reliefmap2[i][j] = (short)colgreen;
                    reliefmap3[i][j] = (short)colblue;
                }
            }

            if (region.rivervalley(i, j) == 1 && region.special(i, j) < 130)
            {
                if (not((region.riverjan(i, j) + region.riverjul(i, j)) / 2 >= minriverflow || (region.fakejan(i, j) + region.fakejul(i, j)) / 2 >= minriverflow))
                {
                    float colred = 0.0f;
                    float colgreen = 0.0f;
                    float colblue = 0.0f;

                    float crount = .0f;

                    for (int k = i - 1; k <= i + 1; k++)
                    {
                        for (int l = j - 1; l <= j + 1; l++)
                        {
                            if (region.riverjan(k, l) == 0 && region.riverjul(k, l) == 0 && region.fakejan(k, l) == 0 && region.fakejul(k, l) == 0 && region.deltajan(k, l) == 0 && region.deltajul(k, l) == 0)
                            {
                                colred = colred + (float)reliefmap1[k][l];
                                colgreen = colgreen + (float)reliefmap2[k][l];
                                colblue = colblue +(float)reliefmap3[k][l];

                                crount++;
                            }
                        }
                    }

                    colred = colred / crount;
                    colgreen = colgreen / crount;
                    colblue = colblue / crount;

                    reliefmap1[i][j] = (short)colred;
                    reliefmap2[i][j] = (short)colgreen;
                    reliefmap3[i][j] = (short)colblue;
                }
            }
        }
    }

    // Do the rivers again, as they might have got messed up by the blurring.

    for (int i = regwidthbegin; i <= regwidthend; i++)
    {
        for (int j = regheightbegin; j <= regheightend; j++)
        {
            if (region.sea(i, j) == 0 && region.special(i, j) < 130 && region.truelake(i, j) == 0)
            {
                if ((region.riverjan(i, j) + region.riverjul(i, j)) / 2 >= minriverflow || (region.fakejan(i, j) + region.fakejul(i, j)) / 2 >= minriverflow)
                {
                    colour1 = world.river1();
                    colour2 = world.river2();
                    colour3 = world.river3();

                    fast_srand(region.maxtemp(i, j) + region.winterrain(i, j));

                    int stripevar = 5;

                    colour1 = colour1 + randomsign(random(0, stripevar));
                    colour2 = colour2 + randomsign(random(0, stripevar));
                    colour2 = colour2 + randomsign(random(0, stripevar));

                    reliefmap1[i][j] = colour1;
                    reliefmap2[i][j] = colour2;
                    reliefmap3[i][j] = colour3;
                }
            }
        }
    }

    // Now apply that to the image, adding shading for slopes where appropriate.

    short r, g, b;

    for (int i = regwidthbegin; i <= regwidthend; i++)
    {
        for (int j = regheightbegin; j <= regheightend; j++)
        {
            r = reliefmap1[i][j];
            g = reliefmap2[i][j];
            b = reliefmap3[i][j];

            if (region.special(i, j) == 0 && region.riverdir(i, j) == 0 && region.fakedir(i, j) == 0)
            {
                bool goahead = 1;

                if ((region.seaice(i, j) == 2 && (world.seaiceappearance() == 1 || world.seaiceappearance() == 3)) || (region.seaice(i, j) == 1 && world.seaiceappearance() == 3)) // Sea ice
                    goahead = 0;

                if (goahead == 1 || (region.sea(i, j) == 0))
                {
                    int slope1 = 0, slope2 = 0;

                    if (shadingdir == 2)
                    {
                        slope1 = getslope(region, i, j, i - 1, j);
                        slope2 = getslope(region, i, j, i, j + 1);
                    }

                    if (shadingdir == 4)
                    {
                        slope1 = getslope(region, i, j, i - 1, j);
                        slope2 = getslope(region, i, j, i, j - 1);
                    }

                    if (shadingdir == 6)
                    {
                        slope1 = getslope(region, i, j, i + 1, j);
                        slope2 = getslope(region, i, j, i, j - 1);
                    }

                    if (shadingdir == 8)
                    {
                        slope1 = getslope(region, i, j, i + 1, j);
                        slope2 = getslope(region, i, j, i, j + 1);
                    }

                    if (slope1 != -1 && slope2 != -1)
                    {
                        int totalslope = (slope1 + slope2) / 10;

                        if (totalslope > 40)
                            totalslope = 40;

                        if (totalslope < -40)
                            totalslope = -40;

                        float thisshading = landshading;

                        if (region.truelake(i, j) == 1)
                            thisshading = lakeshading;

                        if (region.sea(i, j) == 1)
                            thisshading = seashading;

                        totalslope = (int)((float)totalslope * (thisshading * 2.0f));

                        r = r + totalslope;
                        g = g + totalslope;
                        b = b + totalslope;
                    }

                    if (r < 0)
                        r = 0;
                    if (g < 0)
                        g = 0;
                    if (b < 0)
                        b = 0;

                    if (r > 255)
                        r = 255;
                    if (g > 255)
                        g = 255;
                    if (b > 255)
                        b = 255;
                }
            }

            colour1 = r;
            colour2 = g;
            colour3 = b;

            regionalreliefimage.setPixel(i - origregwidthbegin, j - origregheightbegin, sf::Color(colour1, colour2, colour3));
        }
    }
}
