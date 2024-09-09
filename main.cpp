
#include <cstdio>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <cmath>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <chrono>
#include <atomic>
#include <thread>
#include <queue>
#include <stdint.h>
#include "raylib.h"
#include "raymath.h"
#include "rcamera.h"

//#include <glm.hpp>
//#include <gtx\\string_cast.hpp>

//#define RLIGHTS_IMPLEMENTATION
//#include "rlights.h"

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION            100
#endif

#include "imgui.h"
#include "rlImGui.h"
#include "rlImGuiColors.h"

//#include "ImGuiFileDialog.h"

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

//MATERIAL_MAP_ROUGHNESS

using namespace std;

// Random number generator. From https://stackoverflow.com/questions/26237419/faster-than-rand
// It uses a global variable.

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
    SetTraceLogLevel(7);

    float currentversion = 1.0f;
    float latestversion = getlatestversion();

    string currentversionstring = "Current version: " + to_string(currentversion);
    string latestversionstring = "Latest version: " + to_string(latestversion);

    //updatereport(currentversionstring.c_str());
    //updatereport(latestversionstring.c_str());

    string degree = "\xC2\xB0"; // 0xC2 0xB0 (c2b0) °
    string cube = "\xC2\xB3"; // 0xC2 0xB3 (c2b3) ³

    // Set up the window.

    int scwidth = 1920; // 1224;
    int scheight = 1080; // 700; // 768;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI);
    //InitWindow(scwidth, scheight, "Undiscovered Worlds");
    InitWindow(0, 0, "Undiscovered Worlds"); // Uses maximum screen dimensions automatically.

    scwidth = GetScreenWidth();
    scheight = GetScreenHeight();

    ImVec2 focusedwindowpos = ImVec2(200,100); // The position of the focused window. (Because if the focus changes the window ID will change, but we want it to stay in the same position if the user has moved it.)

    // Set up ImGui.

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    // Set up Dear ImGUI style.
    // A rough imitation of Nanogui.

    ImGuiStyle& Style = ImGui::GetStyle();

    Style.WindowMinSize = ImVec2(160, 20);
    Style.WindowPadding = ImVec2(9, 2);
    Style.FramePadding = ImVec2(4, 2);
    Style.ItemSpacing = ImVec2(9, 12);
    Style.Alpha = 0.95f;
    Style.WindowRounding = 4.0f;
    Style.FrameRounding = 4.0f;
    Style.IndentSpacing = 6.0f;
    Style.ItemInnerSpacing = ImVec2(7, 4);
    Style.ColumnsMinSpacing = 50.0f;
    Style.GrabMinSize = 8.0f;
    Style.GrabRounding = 16.0f;
    Style.ScrollbarSize = 12.0f;
    Style.ScrollbarRounding = 16.0f;
    Style.AntiAliasedLinesUseTex = 0.0f;
    Style.TabBorderSize = 0.0f;
    Style.TabBarBorderSize = 0.0f;

    //Style.FancyThickness = 1.5f; // 2.0f;
    //Style.FancyTabIndent = 30;
    //Style.FancyTabBorder = 20;
    //Style.FancyButtonGradient = 0.15f; // 0.15f;
    //Style.FancyTitleGradient = 0.15f; // 0.1f;
    //Style.FancyTitlePadding = 10.0f;
    //Style.FancyProgressGradient = 0.2f; // 0.2f;
    //Style.FancyProgressBandWidth = 100.0f; // 60.0f;

    float highlight1 = 0.40f;
    float highlight2 = 0.40f;
    float highlight3 = 0.40f;

    Style.Colors[ImGuiCol_Text] = ImVec4(0.69f, 0.69f, 0.69f, 1.0f);
    Style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.86f, 0.93f, 0.89f, 0.28f);
    Style.Colors[ImGuiCol_WindowBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.0f); // 0.18
    Style.Colors[ImGuiCol_ChildBg] = ImVec4(highlight1, highlight2, highlight3, 1.00f); // new
    Style.Colors[ImGuiCol_PopupBg] = ImVec4(highlight1, highlight2, highlight3, 1.00f); // new
    Style.Colors[ImGuiCol_Border] = ImVec4(0.1f, 0.1f, 0.1f, 0.2f); // frame around each window
    Style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.2f);
    Style.Colors[ImGuiCol_FrameBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.0f);
    Style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.22f, 0.27f, 1.0f);
    Style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.22f, 0.27f, 1.0f);
    Style.Colors[ImGuiCol_TitleBg] = ImVec4(0.26f, 0.26f, 0.26f, 1.0f); // 25
    Style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.28f, 0.28f, 0.28f, 1.0f);
    Style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.28f, 0.28f, 0.28f, 1.0f);
    Style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.157f, 0.157f, 0.157f, 1.0f);
    Style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.157f, 0.157f, 0.157f, 1.0f);
    Style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.2f, 0.2f, 0.2f, 1.00f);
    Style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.29f, 0.18f, 0.92f, 0.78f);
    Style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(highlight1, highlight2, highlight3, 1.00f);
    Style.Colors[ImGuiCol_CheckMark] = ImVec4(0.86f, 0.93f, 0.89f, 0.78f);
    Style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
    Style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(highlight1, highlight2, highlight3, 1.00f);
    Style.Colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
    Style.Colors[ImGuiCol_ButtonHovered] = ImVec4(highlight1, highlight2, highlight3, 0.86f);
    Style.Colors[ImGuiCol_ButtonActive] = ImVec4(highlight1, highlight2, highlight3, 1.00f);
    Style.Colors[ImGuiCol_Header] = ImVec4(highlight1, highlight2, highlight3, 0.76f);
    Style.Colors[ImGuiCol_HeaderHovered] = ImVec4(highlight1, highlight2, highlight3, 0.86f);
    Style.Colors[ImGuiCol_HeaderActive] = ImVec4(highlight1, highlight2, highlight3, 1.00f);
    Style.Colors[ImGuiCol_Separator] = ImVec4(1.0f, 1.0f, 1.0f, 1.00f); //ImVec4(0.14f, 0.16f, 0.19f, 1.00f);
    Style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(highlight1, highlight2, highlight3, 0.78f);
    Style.Colors[ImGuiCol_SeparatorActive] = ImVec4(highlight1, highlight2, highlight3, 1.0f);
    Style.Colors[ImGuiCol_PlotLines] = ImVec4(0.86f, 0.93f, 0.89f, 0.63f);
    Style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(highlight1, highlight2, highlight3, 1.0f);
    Style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
    Style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(highlight1, highlight2, highlight3, 1.0f);
    Style.Colors[ImGuiCol_PopupBg] = ImVec4(0.157f, 0.157f, 0.157f, 1.0f);
    Style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.157f, 0.157f, 0.157f, 1.0f);

    Style.Colors[ImGuiCol_Tab] = ImVec4(0.11f, 0.11f, 0.11f, 1.0f);
    Style.Colors[ImGuiCol_TabHovered] = ImVec4(highlight1, highlight2, highlight3, 0.86f);
    Style.Colors[ImGuiCol_TabActive] = Style.Colors[ImGuiCol_WindowBg];
    Style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.11f, 0.11f, 0.11f, 1.0f);
    Style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(highlight1, highlight2, highlight3, 0.86f);

    // These ones are used for our custom highlighting/shading around buttons etc.

    Style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.377f, 0.377f, 0.377f, 1.0f); // Top highlight.
    Style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.259f, 0.259f, 0.259f, 1.0f); // Bottom highlight
    Style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.137f, 0.137f, 0.137f, 1.0f); // Shade

    ImFont* font1 = io.Fonts->AddFontFromFileTTF("fonts\\Roboto-Medium.ttf", 20.0f); // 18.0f
    io.Fonts->Build();
    IM_ASSERT(font1 != NULL);

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    //window_flags |= ImGuiWindowFlags_NoResize;

    ImGui_ImplRaylib_Init();

    float buttonheight = 30.0f; // 30
    float buttonwidth = 120.0f; // 140
    float buttongap = 40.0f; // To space buttons vertically.

    float topmargin = 30.0f; // Margin to add just to the top of windows.

    bool* closeflag = new bool(true); // For when we want a close button on windows.

    // Now create the main world object and initialise its basic variables.

    planet* world = new planet;

    initialiseworld(*world);
    initialisemapcolours(*world);
    initialisespacesettings(*world);

    int edge = world->edge();

    bool drawface[6];

    for (int n = 0; n < 6; n++)
        drawface[n] = 1;

    bool drawstarface[6];

    for (int n = 0; n < 6; n++)
        drawstarface[n] = 1;

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

    vector<int> squareroot((MAXCRATERRADIUS * MAXCRATERRADIUS + MAXCRATERRADIUS + 1) * 24);

    for (int n = 0; n <= ((MAXCRATERRADIUS * MAXCRATERRADIUS + MAXCRATERRADIUS) * 24); n++)
        squareroot[n] = (int)sqrt(n);

    screenmodeenum screenmode = createworldscreen; // This is used to keep track of which screen mode we're in.
    screenmodeenum oldscreenmode = createworldscreen;

    mapviewenum mapview = space; // This is used to keep track of which kind of information we want the map to show.

    int seedentry = 0; // The value currently entered into the seed box in the create world screen.

    bool *focused = new bool (false); // This will track whether we're focusing on one point or not.
    globepoint poi; // Coordinates of the point of interest.

    bool globalmapimagecreated[GLOBALMAPTYPES] = {}; // This will keep track of which global map images have actually been created.

    bool newworld = 0; // Whether to show the new world message.

    bool *showcolouroptions = new bool(false); // If this is 1 then we display the appearance preferences options.
    bool *showworldproperties = new bool (false); // If this is 1 then we display the properties of the current world.
    bool *showmovementoptions = new bool(false); // If this is 1 then we display the movement preferences options.
    bool *showglobaltemperaturechart = new bool(false); // If this is 1 then we show monthly temperatures for the selected point.
    bool *showglobalrainfallchart = new bool(false); // If this is 1 then we show monthly rainfall for the selected point.
    bool *showglobalriverchart = new bool(false); // If this is 1 then we show monthly river flow for the selected point.
    bool showregionaltemperaturechart = 0; // If this is 1 then we show monthly temperatures for the selected point.
    bool showregionalrainfallchart = 0; // If this is 1 then we show monthly rainfall for the selected point.
    bool showsetsize = 0; // If this is 1 then a window is show to set the size for the custom world.
    bool showtectonicchooser = 0; // If this is 1 then we show the panel for creating tectonic-based custom world terrain.
    bool shownontectonicchooser = 0; // If this is 1 then we show the panel for creating non-tectonic-based custom world terrain.
    bool showworldeditproperties = 0; // If this is 1 then we show the panel for editing custom world properties.
    bool showareawarning = 0; // If this is 1 then we show a warning about too-large areas.
    bool showabout = 0; // If this is 1 then we display information about the program.
    bool showui = 1; // If this is 1 then we show the UI.

    // And some for the movement controls.

    float camerazoomspeed = 0.0f;
    float camerarotatespeed = 0.0f;
    bool cameralock = 0;
    float planetrotatespeed = 0.0f;
    float monthspeed = 0.0f;
    bool monthrotate = 0;

    initialisemovementsettings(camerazoomspeed, camerarotatespeed, planetrotatespeed, monthspeed, cameralock, monthrotate);

    vector<string> keycodes(350, "");
    createkeycodelist(keycodes);

    int keybindingsno = 12;

    vector<int> keybindings(12, 0);
    loadcontrols(keybindings, keybindingsno, camerazoomspeed, camerarotatespeed, planetrotatespeed, monthspeed, cameralock, monthrotate, "bindings.txt");

    int gettingkeybinding = 0; // If this is > 0 then we are waiting for a key to be pressed to set a binding.

    for (int n = 0; n < GLOBALMAPTYPES; n++)
        globalmapimagecreated[n] = 0;

    bool colourschanged = 0; // If this is 1 then the colours have been changed and the maps need to be redrawn.
    bool othermonthsneedupdating = 0; // If this is 1 then we need to redo all the space images upon closing the colour changing window.

    float linespace = 8.0f; // Gap between groups of buttons.

    string filepathname = "";
    string filepath = "";

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

    // Prepare look-up tables for longitude and latitude. (As they take too long to calculate on the fly.)

    vector<float> longitude(6 * 512 * 512);
    vector<float> latitude(6 * 512 * 512);

    for (int face = 0; face < 6; face++)
    {
        int vface = face * 512 * 512;

        for (int i = 0; i < 512; i++)
        {
            int vi = vface + i * 512;

            for (int j = 0; j < 512; j++)
            {
                int index = vi + j;

                getlonglat(512, face, (float)i, (float)j, longitude[index], latitude[index]);
            }
        }
    }

    // Now prepare look-up tables for directions. This will store the coordinates of points to the north, east, south, and west of each point.

    vector<fourglobepoints> dirpoint(6 * 512 * 512);

    createdirectiontable(edge, dirpoint, longitude, latitude);

    vector<fourdirs> dircode(6 * 512 * 512);

    createdircodetable(edge, dirpoint, dircode);

    // Prepare 2D images to be used as textures. There are six images for each category, corresponding to the six faces of the cubesphere.

    const int globaltexturesize = 512;
    const int globallargetexturesize = 512 * 4;

    vector<Image> globalelevationimage(6);
    vector<Image> globaltemperatureimage(6);
    vector<Image> globalprecipitationimage(6);
    vector<Image> globalclimateimage(6);
    vector<Image> globalriversimage(6);
    vector<Image> globalreliefimage(6);
    vector<Image> globalpickingimage(6);
    vector<Image> globalnormalimage(6);
    vector<Image> globalspaceimage(6 * 12); // 12 of these, for the months
    vector<Image> globalspecularimage(6 * 12); // 12 of these, for the months

    for (int n = 0; n < 6; n++)
    {
        globalelevationimage[n] = GenImageColor(globaltexturesize, globaltexturesize, BLACK);
        globaltemperatureimage[n] = GenImageColor(globaltexturesize, globaltexturesize, BLACK);
        globalprecipitationimage[n] = GenImageColor(globaltexturesize, globaltexturesize, BLACK);
        globalclimateimage[n] = GenImageColor(globaltexturesize, globaltexturesize, BLACK);
        globalriversimage[n] = GenImageColor(globaltexturesize, globaltexturesize, BLACK);
        globalreliefimage[n] = GenImageColor(globaltexturesize, globaltexturesize, BLACK);
        globalpickingimage[n] = GenImageColor(globaltexturesize, globaltexturesize, BLACK);
        globalnormalimage[n] = GenImageColor(globallargetexturesize, globallargetexturesize, BLACK);
    }

    for (int n = 0; n < 6; n++)
    {
        for (int m = 0; m < 12; m++)
        {
            globalspaceimage[n * 12 + m] = GenImageColor(globallargetexturesize, globallargetexturesize, BLACK);
            globalspecularimage[n * 12 + m] = GenImageColor(globallargetexturesize, globallargetexturesize, BLACK);
        }
    }

    vector<Texture2D> globaltexture(6);
    vector<Texture2D> globalspeculartexture(6);
    vector<Texture2D> globalspacetextureold(6);
    vector<Texture2D> globalspeculartextureold(6);
    vector<Texture2D> globalnormaltexture(6);

    for (int n = 0; n < 6; n++)
    {
        globaltexture[n] = LoadTextureFromImage(globalnormalimage[n]);
        globalspeculartexture[n] = LoadTextureFromImage(globalnormalimage[n]);
        globalspacetextureold[n] = LoadTextureFromImage(globalnormalimage[n]);
        globalspeculartextureold[n] = LoadTextureFromImage(globalnormalimage[n]);
        globalnormaltexture[n] = LoadTextureFromImage(globalnormalimage[n]);
    }

    //Image testimage = LoadImage("resources\\parrots.png");

    drawglobalpickingimage(*world, globaltexturesize, globalpickingimage);

    // Now we prepare map colours. We put them into ImVec4 objects, which can be directly manipulated by the colour picker objects.

    ImVec4 seacolour;

    seacolour.x = (float)world->sea1() / 255.f;
    seacolour.y = (float)world->sea2() / 255.f;
    seacolour.z = (float)world->sea3() / 255.f;
    seacolour.w = 1.f;

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

    ImVec4 atmoscolour;

    atmoscolour.x = (float)world->atmos1() / 255.f;
    atmoscolour.y = (float)world->atmos2() / 255.f;
    atmoscolour.z = (float)world->atmos3() / 255.f;
    atmoscolour.w = 1.f;

    ImVec4 duskcolour;

    duskcolour.x = (float)world->dusk1() / 255.f;
    duskcolour.y = (float)world->dusk2() / 255.f;
    duskcolour.z = (float)world->dusk3() / 255.f;
    duskcolour.w = 1.f;

    ImVec4 suncolour;

    suncolour.x = (float)world->sun1() / 255.f;
    suncolour.y = (float)world->sun2() / 255.f;
    suncolour.z = (float)world->sun3() / 255.f;
    suncolour.w = 1.f;

    ImVec4 gal1hazecolour;

    gal1hazecolour.x = (float)world->gal1haze1() / 255.f;
    gal1hazecolour.y = (float)world->gal1haze2() / 255.f;
    gal1hazecolour.z = (float)world->gal1haze3() / 255.f;
    gal1hazecolour.w = 1.f;

    ImVec4 gal2hazecolour;

    gal2hazecolour.x = (float)world->gal2haze1() / 255.f;
    gal2hazecolour.y = (float)world->gal2haze2() / 255.f;
    gal2hazecolour.z = (float)world->gal2haze3() / 255.f;
    gal2hazecolour.w = 1.f;

    ImVec4 gal1nebulacolour;

    gal1nebulacolour.x = (float)world->gal1nebula1() / 255.f;
    gal1nebulacolour.y = (float)world->gal1nebula2() / 255.f;
    gal1nebulacolour.z = (float)world->gal1nebula3() / 255.f;
    gal1nebulacolour.w = 1.f;

    ImVec4 gal2nebulacolour;

    gal2nebulacolour.x = (float)world->gal2nebula1() / 255.f;
    gal2nebulacolour.y = (float)world->gal2nebula2() / 255.f;
    gal2nebulacolour.z = (float)world->gal2nebula3() / 255.f;
    gal2nebulacolour.w = 1.f;

    ImVec4 bloomcolour;

    bloomcolour.x = (float)world->bloom1() / 255.f;
    bloomcolour.y = (float)world->bloom2() / 255.f;
    bloomcolour.z = (float)world->bloom3() / 255.f;
    bloomcolour.w = 1.f;

    // Prepare some other variables for the appearance controls.

    int rainpoint = 80; // Areas with less rain than this will have reduced clouds.

    float marblingland = world->landmarbling();
    float marblinglake = world->lakemarbling();
    float marblingsea = world->seamarbling();

    int minriverflowglobal = world->minriverflowglobal();
    int minriverflowregional = world->minriverflowregional();

    int globalriversentry = world->minriverflowglobal();
    int regionalriversentry = world->minriverflowregional();

    float normal = world->normal();
    float ambient = world->ambient();
    float gamma = world->gamma();
    float haze = world->haze();
    float specular = world->specular();
    float cloudcover = world->cloudcover();

    float bloomdist = world->bloomdist();
    float sunglare = world->sunglare();
    float sunlens = world->sunlens();
    int sunrayno = world->sunrayno();
    float sunraystrength = world->sunraystrength();

    float starbright = world->starbright();
    float starcolour = world->starcolour();
    float starhaze = world->starhaze();
    float starnebula = world->starnebula();

    int snowchange = world->snowchange() - 1;
    int seaiceappearance = world->seaiceappearance() - 1;
    bool colourcliffs = world->colourcliffs();
    bool mangroves = world->showmangroves();

    bool mouseleftpressed = 0;

    // Set up the camera.

    float camdist = 4.0f; // Distance of the camera from the centre of the planet.
    float camlong = 0.0f; // "Longitude" of the camera position. Measured absolutely, not necessarily corresponding to the planet's long/lat, as that can rotate.
    float camlat = 0.0f; // "latitude" of the camera position.

    threefloats cameraposition = getcoordinatesfromlatlong(camlat, camlong, camdist);

    Vector3 campos = { cameraposition.x, cameraposition.y, cameraposition.z };

    Camera camera = { 0 };
    camera.position = campos;
    camera.target = Vector3{ 0.0f, 0.0f, 0.0f };
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    threefloats highlightposition;

    highlightposition.x = 0.0f;
    highlightposition.y = 0.0f;
    highlightposition.x = 0.0f;

    float sundistance = 400.0f;

    int currentmonth = 0;

    float currentsunlong = world->sunlong(currentmonth);
    float currentsunlat = world->sunlat(currentmonth);
    float currentsundist = world->sundist(currentmonth);

    int monthsteps = 100; // The number of frames it takes to shift from one month to another if it's done at maximum slowness.
    float thismonthsteps = 0.0f; // The number of frames it takes to shift at the moment.
    int currentstep = 0; // If this is > 1 then count it down to shift to the next month.
    float movesunlong = 0.0f; // The amount to move the sun (in degrees) every frame, if shifting.
    float movesunlat = 0.0f;
    float movesundist = 0.0f;

    float showcurrentmonth = 1.0f; // If this is 1, just show the image for the current month. If it's < 1.0, merge the current month image with the old month image.

    int totalprogress = 0; // How many steps it takes to create the world.
    int progressval = 0; // Current step.
    string progresstext = "";

    // Load shaders

    // Basic shader (for the planet when not in "space" view mode)

    Shader baseshader = LoadShader(TextFormat("shaders\\base.vs", GLSL_VERSION),
        TextFormat("shaders\\base.fs", GLSL_VERSION));
    baseshader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(baseshader, "viewPos");

    // Black shader (for the black version of the planet, when doing the glow for the sun)

    Shader blackshader = LoadShader(TextFormat("shaders\\black.vs", GLSL_VERSION),
        TextFormat("shaders\\black.fs", GLSL_VERSION));
    blackshader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(blackshader, "viewPos");

    // Shader for the starfield

    Shader starshader = LoadShader(TextFormat("shaders\\starfield.vs", GLSL_VERSION),
        TextFormat("shaders\\starfield.fs", GLSL_VERSION));
    starshader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(starshader, "viewPos");
    int starbrightLoc = GetShaderLocation(starshader, "starBrightness");
    int starcolourLoc = GetShaderLocation(starshader, "starColor");
    int starhazeLoc = GetShaderLocation(starshader, "hazeBrightness");
    int starnebulaLoc = GetShaderLocation(starshader, "nebulaBrightness");
    int galhazecol1Loc = GetShaderLocation(starshader, "hazeColor1");
    int galhazecol2Loc = GetShaderLocation(starshader, "hazeColor2");
    int galnebcol1Loc = GetShaderLocation(starshader, "nebulaColor1");
    int galnebcol2Loc = GetShaderLocation(starshader, "nebulaColor2");

    starshader.locs[SHADER_LOC_MAP_OCCLUSION] = GetShaderLocation(starshader, "haze");

    // Shader for the sun

    Shader sunshader = LoadShader(TextFormat("shaders\\sun.vs", GLSL_VERSION),
        TextFormat("shaders\\sun.fs", GLSL_VERSION));
    sunshader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(sunshader, "viewPos");

    // Shader for the bloom effect

    Shader bloomshader = LoadShader(TextFormat("shaders\\sunbloom.vs", GLSL_VERSION),
        TextFormat("shaders\\sunbloom.fs", GLSL_VERSION));
    bloomshader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(bloomshader, "viewPos");
    int suncolLoc = GetShaderLocation(bloomshader, "sunColor");
    int sunpos2dLoc = GetShaderLocation(bloomshader, "sunCentre");
    int bloomcolLoc = GetShaderLocation(bloomshader, "bloomColor");
    int bloomdistLoc = GetShaderLocation(bloomshader, "bDist");
    int sunglareLoc = GetShaderLocation(bloomshader, "glareFactor");
    int sunraystrengthLoc = GetShaderLocation(bloomshader, "rayValue");
    int sunraynoLoc = GetShaderLocation(bloomshader, "rayNo");

    // Shader for the lens flare effect

    Shader lensshader = LoadShader(TextFormat("shaders\\sunlens.vs", GLSL_VERSION),
        TextFormat("shaders\\sunlens.fs", GLSL_VERSION));
    lensshader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(bloomshader, "viewPos");
    int suncollensLoc = GetShaderLocation(lensshader, "sunColor");
    int sunpos2dlensLoc = GetShaderLocation(lensshader, "sunCentre");
    int sunlensLoc = GetShaderLocation(lensshader, "lensStrength");

    // Shader for the planet object (when in "space" view mode)

    Shader planetshader = LoadShader(TextFormat("shaders\\planet.vs", GLSL_VERSION),
        TextFormat("shaders\\planet.fs", GLSL_VERSION));
    planetshader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(planetshader, "viewPos");
    int normalLoc = GetShaderLocation(planetshader, "bumpiness");
    int ambientLoc = GetShaderLocation(planetshader, "ambient");
    int gammaLoc = GetShaderLocation(planetshader, "gamma");
    int hazeLoc = GetShaderLocation(planetshader, "haze");
    int specularLoc = GetShaderLocation(planetshader, "specularOverall");
    int cloudcoverLoc = GetShaderLocation(planetshader, "cloudCover");
    int showcurrentmonthLoc = GetShaderLocation(planetshader, "showCurrentMonth");
    int atmosLoc = GetShaderLocation(planetshader, "atmosCentreColor");
    int duskLoc = GetShaderLocation(planetshader, "duskColor");
    int lightcolLoc = GetShaderLocation(planetshader, "lightCol");
    int lightposLoc = GetShaderLocation(planetshader, "lightPos");
    int lighttargetLoc = GetShaderLocation(planetshader, "lightTarget");

    float ambientcol[4] = { ambient, ambient, ambient, 1.0f };
    SetShaderValue(planetshader, ambientLoc, ambientcol, SHADER_UNIFORM_VEC4);

    float lightpos[3];

    threefloats thissunpos = getcoordinatesfromlatlong(world->sunlat(currentmonth), world->sunlong(currentmonth), sundistance * world->sundist(currentmonth));

    lightpos[0] = thissunpos.x;
    lightpos[1] = thissunpos.y;
    lightpos[2] = thissunpos.z;

    SetShaderValue(planetshader, lightposLoc, lightpos, SHADER_UNIFORM_VEC3);

    float lighttarget[3] = { 0.0f, 0.0f, 0.0f };
    SetShaderValue(planetshader, lighttargetLoc, lighttarget, SHADER_UNIFORM_VEC3);

    planetshader.locs[SHADER_LOC_MAP_OCCLUSION] = GetShaderLocation(planetshader, "olddiffuse");
    planetshader.locs[SHADER_LOC_MAP_EMISSION] = GetShaderLocation(planetshader, "oldspecular");

    // Set up the cubesphere.

    Vector3 globepos = { 0.0f, 0.0f, 0.0f };
    float globerotate = 0.0f;
    float globetilt = 0.0f;
    float globepitch = 0.0f;

    float relativelong = 0.0f;
    float relativelat = 0.0f;

    int panels = 16; // Each face of the cubesphere consists of panels * panels panels. (Each panel consists of two flat triangles.)

    vector<vector<vector<Model>>> spherepanels(6, vector<vector<Model>>(panels, vector<Model>(panels)));

    setupspherepanels(spherepanels, panels, 1.0f);
    updateshader(spherepanels, planetshader, panels);

    // Now three more cubespheres for the regions (for the three planetary sizes). Where regions have been generated, we will show their panels instead of the global sphere panels.

    vector<vector<vector<Model>>> regionspherepanels0(6, vector<vector<Model>>(panels, vector<Model>(panels)));
    vector<vector<vector<Model>>> regionspherepanels1(6, vector<vector<Model>>(panels, vector<Model>(panels)));
    vector<vector<vector<Model>>> regionspherepanels2(6, vector<vector<Model>>(panels, vector<Model>(panels)));

    setupregionalspherepanels(regionspherepanels0, panels, 0, 1.0f);
    updateshader(regionspherepanels0, planetshader, panels);

    setupregionalspherepanels(regionspherepanels1, panels, 1, 1.0f);
    updateshader(regionspherepanels1, planetshader, panels);

    setupregionalspherepanels(regionspherepanels2, panels, 2, 1.0f);
    updateshader(regionspherepanels2, planetshader, panels);

    // Now create the skybox. It's a cubesphere as well.

    const int skytexturesize = 1012; // 512;

    vector<Image> starsimage(6);
    vector<Image> starhazeimage(6);

    for (int n = 0; n < 6; n++)
    {
        starsimage[n] = GenImageColor(skytexturesize, skytexturesize, BLACK);
        starhazeimage[n] = GenImageColor(skytexturesize, skytexturesize, BLACK);
    }

    vector<Texture2D> starstexture(6);
    vector<Texture2D> starhazetexture(6);

    for (int n = 0; n < 6; n++)
    {
        starstexture[n] = LoadTextureFromImage(starsimage[n]);
        starhazetexture[n] = LoadTextureFromImage(starhazeimage[n]);
    }

    float skyrotate = 0.0f;
    float skytilt = 0.0f;
    float skypitch = 0.0f;

    int spanels = 1; // Fewer panels for this one, as it doesn't need to be as smooth.

    vector<vector<vector<Model>>> skypanels(6, vector<vector<Model>>(spanels, vector<Model>(spanels)));

    setupspherepanels(skypanels, spanels, -1000.0f);

    updateshader(skypanels, starshader, spanels);

    float textureypos = -9.0f; // y position of the bloom texture, to make it match the rest of the scene.

    // Create a simple planet object. This will be used just for creating planetary textures.

    simpleplanet* simpleworld = new simpleplanet;

    // Vector to work out value proportions for enhanced detail on space images.

    vector<float> proportions(4 * 4 * 4, 0.0f);

    createproportionstable(proportions);

    // Now we need an enormous vector of region objects.

    int maxregions = 8 * 8; // This is the most regions we can hold in memory at once.

    vector<region> regions(maxregions); // The actual region objects themselves.
    vector<globepoint>regionID(maxregions); // This will specify where each of these regions actually is on the globe.
    vector<int>regionloc(16 * 16 * 6, -1); // The reverse of the above: it will specify which region object corresponds to this area of the globe.

    for (int n = 0; n < maxregions; n++)
    {
        regionID[n].face = -1;
        regionID[n].x = -1;
        regionID[n].y = -1;
    }

    vector<short> regioncreated(maxregions,0); // 0 == uncreated, 1 == created, 2 == being created.
    vector<short>regioncreatedloc(16 * 16 * 6, 0); // As above, but ordered by location.

    // We also need textures for the regions.

    const int regionaltexturesize = 512;

    vector<Image> regionalelevationimage(maxregions);
    vector<Image> regionaltemperatureimage(maxregions);
    vector<Image> regionalprecipitationimage(maxregions);
    vector<Image> regionalclimateimage(maxregions);
    vector<Image> regionalriversimage(maxregions);
    vector<Image> regionalreliefimage(maxregions);
    vector<Image> regionalpickingimage(maxregions);
    vector<Image> regionalnormalimage(maxregions);
    vector<Image> regionalspaceimage(maxregions * 12); // 12 of these, for the months
    vector<Image> regionalspecularimage(maxregions * 12); // 12 of these, for the months

    for (int n = 0; n < maxregions; n++)
    {
        regionalelevationimage[n] = GenImageColor(regionaltexturesize, regionaltexturesize, RED);
        regionaltemperatureimage[n] = GenImageColor(regionaltexturesize, regionaltexturesize, BLACK);
        regionalprecipitationimage[n] = GenImageColor(regionaltexturesize, regionaltexturesize, BLACK);
        regionalclimateimage[n] = GenImageColor(regionaltexturesize, regionaltexturesize, BLACK);
        regionalriversimage[n] = GenImageColor(regionaltexturesize, regionaltexturesize, BLACK);
        regionalreliefimage[n] = GenImageColor(regionaltexturesize, regionaltexturesize, BLACK);
        regionalpickingimage[n] = GenImageColor(regionaltexturesize, regionaltexturesize, BLACK);
        regionalnormalimage[n] = GenImageColor(regionaltexturesize, regionaltexturesize, BLACK);
    }

    for (int n = 0; n < maxregions; n++)
    {
        for (int m = 0; m < 12; m++)
        {
            regionalspaceimage[n * 12 + m] = GenImageColor(regionaltexturesize, regionaltexturesize, BLACK);
            regionalspecularimage[n * 12 + m] = GenImageColor(regionaltexturesize, regionaltexturesize, BLACK);
        }
    }

    vector<Texture2D> regionaltexture(maxregions);
    vector<Texture2D> regionalspeculartexture(maxregions);
    vector<Texture2D> regionalspacetextureold(maxregions);
    vector<Texture2D> regionalspeculartextureold(maxregions);
    vector<Texture2D> regionalnormaltexture(maxregions);

    for (int n = 0; n < maxregions; n++)
    {
        regionaltexture[n] = LoadTextureFromImage(regionalnormalimage[n]);
        regionalspeculartexture[n] = LoadTextureFromImage(regionalnormalimage[n]);
        regionalspacetextureold[n] = LoadTextureFromImage(regionalnormalimage[n]);
        regionalspeculartextureold[n] = LoadTextureFromImage(regionalnormalimage[n]);
        regionalnormaltexture[n] = LoadTextureFromImage(regionalnormalimage[n]);
    }

    // Set up test regions

    //for (int n = 0; n < maxregions; n++)
        //regionaltexture[n] = LoadTextureFromImage(testimage);


    // Set up lots of threads for creating regions.

    int maxregionthreads = 16;

    vector<thread> regionthreads(maxregionthreads);
    vector<bool> regionthreadready(maxregionthreads, true);

    for (int i = 0; i < maxregionthreads; i++)
    {
        regionthreads[i] = thread([&i, &regionthreadready] {threadinitialise(); regionthreadready[i] = true; });
        //regionthreads[i].join();
    }

    // Set up a thread object for creating the world.

    atomic<bool> createworldthreadready(true);

    thread createworldthread([&createworldthreadready] {threadinitialise(); createworldthreadready = true; });

    createworldthread.join();

    // Set up thread objects for drawing the textures.

    thread texturethread0([] {threadinitialise(); });
    thread texturethread1([] {threadinitialise(); });
    thread texturethread2([] {threadinitialise(); });
    thread texturethread3([] {threadinitialise(); });
    thread texturethread4([] {threadinitialise(); });
    thread texturethread5([] {threadinitialise(); });

    texturethread0.join();
    texturethread1.join();
    texturethread2.join();
    texturethread3.join();
    texturethread4.join();
    texturethread5.join();

    float frametime = 1000.0f;
    float targetframetime = 16000.0f; // These are to ensure consistent movement rates when the frame rate varies.

    globepoint facingpanel; // The panel on the globe that the camera is pointed to.
    facingpanel.face = -1;
    facingpanel.x = 0;
    facingpanel.y = 0;

    // Now the main loop.

    while (!WindowShouldClose())
    {
        chrono::steady_clock::time_point framestarttime = chrono::steady_clock::now();

        int oldmonth = currentmonth;

        if (screenmode == movingtoglobalmapscreen)
            screenmode = globalmapscreen;

        // GUI stuff now.

        ImGui_ImplRaylib_ProcessEvents();
        ImGui_ImplRaylib_NewFrame();
        ImGui::NewFrame();

        // Check whether a region-creating thread is ready to go. If it is, tell it to create a new region.
        // Turned off for now as the region-creating code has yet to be integrated into this version!

        if (1 == 0) //(screenmode == globalmapscreen && facingpanel.face != -1)
        {
            // Find a free thread.

            int thisthread = -1;

            for (int n = 0; n < maxregionthreads; n++)
            {
                if (regionthreadready[n])
                {
                    thisthread = n;
                    n = maxregionthreads;
                }
            }

            if (thisthread != -1) // If we found a free thread
            {
                // Work out which region area to make.

                globepoint facingregion = facingpanel;

                if (world->size() == 1)
                {
                    facingregion.x = facingregion.x / 2;
                    facingregion.y = facingregion.y / 2;
                }

                if (world->size() == 0)
                {
                    facingregion.x = facingregion.x / 4;
                    facingregion.y = facingregion.y / 4;
                }

                globepoint thisregionlocation = findregion(*world, panels, regioncreated, regioncreatedloc, regionID, regionloc, facingregion, 0); //facingregion;

                // Now we need to find a free region object.

                int thisregionobject = -1;

                for (int n = 0; n < maxregions; n++)
                {
                    if (regioncreated[n] == 0)
                    {
                        thisregionobject = n;
                        n = maxregions;
                    }
                }

                if (thisregionobject == -1) // If we didn't find one!
                {
                    globepoint oldregionlocation;
                    oldregionlocation.face = -1;
                    oldregionlocation.x = -1;
                    oldregionlocation.y = -1;

                    // First, try to find one from the opposite face.

                    int oppositeface;

                    if (facingregion.face == 0)
                        oppositeface = 2;

                    if (facingregion.face == 1)
                        oppositeface = 3;

                    if (facingregion.face == 2)
                        oppositeface = 0;

                    if (facingregion.face == 3)
                        oppositeface = 1;

                    if (facingregion.face == 4)
                        oppositeface = 5;

                    if (facingregion.face == 5)
                        oppositeface = 4;

                    for (int n = 0; n < maxregions; n++)
                    {
                        if (regionID[n].face == oppositeface)
                        {
                            thisregionobject = n;
                            n = maxregions;
                        }
                    }

                    if (thisregionobject == -1) // If that failed, try to find the most distant one
                    {
                        int largestdist = 0;

                        int edge = panels;

                        if (world->size() == 1)
                            edge = edge / 2;

                        if (world->size() == 0)
                            edge = edge / 4;

                        int thisdist = getpointdistsquared(edge, facingregion.face, facingregion.x, facingregion.y, thisregionlocation.face, thisregionlocation.x, thisregionlocation.y); // This is the distance of the region we're interested in from the facing region.

                        int furthest = -1;

                        for (int n = 0; n < maxregions; n++)
                        {
                            if (regioncreated[n])
                            {
                                int currentdist = getpointdistsquared(edge, facingregion.face, facingregion.x, facingregion.y, regionID[n].face, regionID[n].x, regionID[n].y);
                            
                                if (currentdist > largestdist && currentdist > thisdist)
                                {
                                    largestdist = currentdist;
                                    thisregionobject = n;
                                }
                            }
                        }
                    }                    

                    if (thisregionobject != -1)
                    {
                        globepoint oldregionlocation = regionID[thisregionobject];

                        thisregionobject = regionloc[oldregionlocation.face * panels * panels + oldregionlocation.x * panels + oldregionlocation.y];

                        // Delete the previous region that had this ID.

                        regionloc[oldregionlocation.face * panels * panels + oldregionlocation.x * panels + oldregionlocation.y] = -1;
                        regioncreatedloc[oldregionlocation.face * panels * panels + oldregionlocation.x * panels + oldregionlocation.y] = 0;
                        regioncreated[thisregionobject] = 0;

                        regionID[thisregionobject].face = -1;
                        regionID[thisregionobject].x = -1;
                        regionID[thisregionobject].y = -1;
                    }
                }

                if (thisregionobject != -1) // Set up the thread to create this new region!
                {
                    regionthreads[thisthread].join();

                    regionthreads[thisthread] = thread([&thisthread, &regionthreadready, &world, &panels, &maxregions, &thisregionobject, &thisregionlocation, &regionID, &regionloc, &regioncreated, &regioncreatedloc, &regionaltexturesize, &regionalelevationimage, &regionaltemperatureimage, &regionalprecipitationimage, &regionalclimateimage, &regionalriversimage, &regionalreliefimage, &regionalspaceimage, &regionalspecularimage, &regionaltexture, &regionalspeculartexture, &regionalnormaltexture] {createregion(*world, panels,maxregions, thisregionobject, thisregionlocation, regionID, regionloc, regioncreated, regioncreatedloc, regionaltexturesize, regionalelevationimage, regionaltemperatureimage, regionalprecipitationimage, regionalclimateimage, regionalriversimage, regionalreliefimage, regionalspaceimage, regionalspecularimage, regionaltexture, regionalspeculartexture, regionalnormaltexture); regionthreadready[thisthread] = true; });
                }
            }
        }

        //ImGui::ShowDemoWindow();

        // Create world screen

        if (screenmode == createworldscreen)
        {
            *showglobaltemperaturechart = false;
            *showglobalrainfallchart = false;
            *showglobalriverchart = false;
            showregionaltemperaturechart = 0;
            showregionalrainfallchart = 0;
            *showworldproperties = false;
            *showcolouroptions = false;
            *showmovementoptions = false;
            showworldeditproperties = 0;
            newworld = 0;

            // If there is an update available, show an alert. (Turned off for now as it conflicts with Raylib!)

            /*
            if (latestversion > currentversion)
            {
                ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize(ImVec2(224, 107), ImGuiCond_FirstUseEver);

                ImGui::Begin("Update available!", NULL, window_flags);
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + topmargin);

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
            */
            

            float thiswidth = 380.0f;
            float thisheight = 139.0f;
            
            ImGui::SetNextWindowSize(ImVec2(thiswidth, thisheight), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowPos(ImVec2((scwidth - thiswidth) / 2.0f, (scheight - thisheight) / 3.0f), ImGuiCond_FirstUseEver);

            ImGui::Begin("Create world", NULL, window_flags);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + topmargin);

            ImGui::SetNextItemWidth(-1.0f);
            ImGui::InputInt(" ", &seedentry, 1, 1, window_flags);

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Please enter a seed number, which will be used to calculate the new world. The same number will always yield the same world.");

            if (seedentry < 0)
                seedentry = 0;

            string loadtext = "Load a world.";

            if (brandnew == 1)
            {
                if (ImGui::Button("Load", ImVec2(85.0f, buttonheight))) // This opens the load world dialogue. We check its results later.
                {
                    loadingworld = 1;
                }
            }
            else
            {
                loadtext = "Return to the world map.";

                if (ImGui::Button("   Cancel   ", ImVec2(0.0f, buttonheight)))
                    screenmode = globalmapscreen;
            }

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip(loadtext.c_str());

            ImGui::SameLine();

            if (ImGui::Button("   Custom   ", ImVec2(0.0f, buttonheight)))
            {
                showsetsize = 1;
            }

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Create a custom world.");

            ImGui::SameLine();

            if (ImGui::Button("   Random   ", ImVec2(0.0f, buttonheight)))
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

            if (ImGui::Button("   OK   ", ImVec2(0.0f, buttonheight)) || IsKeyPressed(KEY_ENTER))
            {
                // Accept seed and generate world

                //seedentry = 45109170;

                world->setseed(seedentry);

                for (int n = 0; n < GLOBALMAPTYPES; n++) // Set all map types as unviewed, to force them to be redrawn when called up
                    globalmapimagecreated[n] = 0;

                globalmapimagecreated[6] = 1;

                mapview = space;
                createworldthreadready = 0;

                // Reset regions. Remember to reactivate this bit!

                /*
                for (int n = 0; n < maxregions; n++)
                {
                    regioncreatedloc[n] = 0;

                    regionID[n].face = -1;
                    regionID[n].x = -1;
                    regionID[n].y = -1;
                }

                for (int n = 0; n < 16 * 16 * 6; n++)
                    regionloc[n] = -1;
                */

                switch (world->size())
                {
                case 0:
                    createworldthread = thread([&createworldthreadready, &world, &simpleworld, &edge, &longitude, &latitude, &dirpoint, &squareroot, &globalspaceimage, &globalspecularimage, &starsimage, &starhazeimage, &globalnormalimage, &globaltexturesize, &globallargetexturesize, &skytexturesize, &skyrotate, &skytilt, &skypitch, &landshape, &chainland, &smalllake, &largelake, &proportions, &rainpoint, &panels, &regionspherepanels0, &planetshader, &dircode, &progressval, &progresstext] {createworld(*world, *simpleworld, edge, longitude, latitude, dirpoint, squareroot, globalspaceimage, globalspecularimage, starsimage, starhazeimage, globalnormalimage, globaltexturesize, globallargetexturesize, skytexturesize, skyrotate, skytilt, skypitch, landshape, chainland, smalllake, largelake, proportions, rainpoint, panels, regionspherepanels0, planetshader, dircode, progressval, progresstext); createworldthreadready = true; });
                    break;

                case 1:
                    createworldthread = thread([&createworldthreadready, &world, &simpleworld, &edge, &longitude, &latitude, &dirpoint, &squareroot, &globalspaceimage, &globalspecularimage, &starsimage, &starhazeimage, &globalnormalimage, &globaltexturesize, &globallargetexturesize, &skytexturesize, &skyrotate, &skytilt, &skypitch, &landshape, &chainland, &smalllake, &largelake, &proportions, &rainpoint, &panels, &regionspherepanels1, &planetshader, &dircode, &progressval, &progresstext] {createworld(*world, *simpleworld, edge, longitude, latitude, dirpoint, squareroot, globalspaceimage, globalspecularimage, starsimage, starhazeimage, globalnormalimage, globaltexturesize, globallargetexturesize, skytexturesize, skyrotate, skytilt, skypitch, landshape, chainland, smalllake, largelake, proportions, rainpoint, panels, regionspherepanels1, planetshader, dircode, progressval, progresstext); createworldthreadready = true; });
                    break;

                case 2:
                    createworldthread = thread([&createworldthreadready, &world, &simpleworld, &edge, &longitude, &latitude, &dirpoint, &squareroot, &globalspaceimage, &globalspecularimage, &starsimage, &starhazeimage, &globalnormalimage, &globaltexturesize, &globallargetexturesize, &skytexturesize, &skyrotate, &skytilt, &skypitch, &landshape, &chainland, &smalllake, &largelake, &proportions, &rainpoint, &panels, &regionspherepanels2, &planetshader, &dircode, &progressval, &progresstext] {createworld(*world, *simpleworld, edge, longitude, latitude, dirpoint, squareroot, globalspaceimage, globalspecularimage, starsimage, starhazeimage, globalnormalimage, globaltexturesize, globallargetexturesize, skytexturesize, skyrotate, skytilt, skypitch, landshape, chainland, smalllake, largelake, proportions, rainpoint, panels, regionspherepanels2, planetshader, dircode, progressval, progresstext); createworldthreadready = true; });
                    break;
                }

                screenmode = creatingworldscreen;

                progressval = 0;
                progresstext = "Generating world";
            }

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Create a world from this seed number.");

            ImGui::End();
        }

        // Creating world screen

        if (screenmode == creatingworldscreen)
        {
            float waitwidth = (float)scwidth / 2.0f;
            float waitheight = 120.0f;

            ImGui::SetNextWindowPos(ImVec2((scwidth - waitwidth) / 2.0f, (scheight - waitheight) / 2.0f), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(waitwidth, waitheight), ImGuiCond_FirstUseEver);
            ImGui::Begin("Please wait...", NULL, window_flags);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + topmargin);
            ImGui::Text(progresstext.c_str());

            waitwidth = ImGui::GetWindowWidth();

            if (world->type() == 1)
                totalprogress = 55;

            if (world->type() == 2)
                totalprogress = 58;

            if (world->type() == 3)
                totalprogress = 49;

            if (world->type() == 4)
                totalprogress = 50;

            float progress = (float)progressval/ (float)totalprogress;

            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
            ImGui::ProgressBar(progress, ImVec2(waitwidth - 20.0f, 10.0f), " ");
            ImGui::End();

            if (createworldthreadready)
            {
                createworldthread.join();

                updatetextures(starsimage, starstexture);
                updatetextures(starhazeimage, starhazetexture);

                applystarhazetextures(skypanels, starhazetexture, spanels);

                updateshader(skypanels, starshader, spanels);

                updatetextures(globalnormalimage, globalnormaltexture);
                applynormaltextures(spherepanels, globalnormaltexture, panels);

                updateshader(spherepanels, planetshader, panels);

                updatetextures(globalspaceimage, globaltexture, currentmonth);
                updatetextures(globalspecularimage, globalspeculartexture, currentmonth);
                applyspeculartextures(spherepanels, globalspeculartexture, panels);

                *focused = false;

                screenmode = movingtoglobalmapscreen;
                newworld = 1;
            }
        }

        // Global map screen

        if (showui && screenmode == globalmapscreen)
        {
            showworldeditproperties = 0;
            showregionaltemperaturechart = 0;
            showregionalrainfallchart = 0;

            brandnew = 0;

            // Main controls.

            string title;

            if (world->seed() >= 0)
                title = "Seed: " + to_string(world->seed());
            else
                title = "Custom";

            ImGui::SetNextWindowPos(ImVec2(10, 8), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(166, 728), ImGuiCond_FirstUseEver);

            ImGui::Begin(title.c_str(), NULL, window_flags);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + topmargin);

            ImVec2 pos = ImGui::GetWindowPos();

            ImGui::Text("World controls:");

            if (standardbutton("New world"))
            {
                brandnew = 0;
                seedentry = 0;

                screenmode = createworldscreen;
            }

            /*
            if (standardbutton("Load world"))
            {
                //ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".uww", ".");

                loadingworld = 1;
            }

            if (standardbutton("Save world"))
            {
                //ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".uww", ".");

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
                //ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".png", ".");

                exportingworldmaps = 1;
            }

            ImGui::Dummy(ImVec2(0.0f, linespace));
            */

            ImGui::SetNextItemWidth(0);

            ImGui::Text("Display map type:");

            ImGui::PushItemWidth(100.0f);

            if (standardbutton("Space"))
            {
                mapview = space;

                if (globalmapimagecreated[6] == 0)
                {
                    //drawglobalspacemapimage(*world, *simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, 0, 11, 0, 5);

                    texturethread0 = thread([&world, &simpleworld, &globaltexturesize, &globalspaceimage, &globalspecularimage, &latitude, &proportions, &rainpoint] {drawglobalspacemapimage(*world, *simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, 0, 11, 0, 0); });
                    texturethread1 = thread([&world, &simpleworld, &globaltexturesize, &globalspaceimage, &globalspecularimage, &latitude, &proportions, &rainpoint] {drawglobalspacemapimage(*world, *simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, 0, 11, 1, 1); });
                    texturethread2 = thread([&world, &simpleworld, &globaltexturesize, &globalspaceimage, &globalspecularimage, &latitude, &proportions, &rainpoint] {drawglobalspacemapimage(*world, *simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, 0, 11, 2, 2); });
                    texturethread3 = thread([&world, &simpleworld, &globaltexturesize, &globalspaceimage, &globalspecularimage, &latitude, &proportions, &rainpoint] {drawglobalspacemapimage(*world, *simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, 0, 11, 3, 3); });
                    texturethread4 = thread([&world, &simpleworld, &globaltexturesize, &globalspaceimage, &globalspecularimage, &latitude, &proportions, &rainpoint] {drawglobalspacemapimage(*world, *simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, 0, 11, 4, 4); });
                    texturethread5 = thread([&world, &simpleworld, &globaltexturesize, &globalspaceimage, &globalspecularimage, &latitude, &proportions, &rainpoint] {drawglobalspacemapimage(*world, *simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, 0, 11, 5, 5); });

                    texturethread0.join();
                    texturethread1.join();
                    texturethread2.join();
                    texturethread3.join();
                    texturethread4.join();
                    texturethread5.join();

                    globalmapimagecreated[6] = 1;
                }

                updatetextures(globalspaceimage, globaltexture, currentmonth);
                updatetextures(globalspecularimage, globalspeculartexture, currentmonth);
                applyspeculartextures(spherepanels, globalspeculartexture, panels);

                updateshader(spherepanels, planetshader, panels);
            }

            if (standardbutton("Relief"))
            {
                mapview = relief;

                if (globalmapimagecreated[5] == 0)
                {
                    drawglobalreliefmapimage(*world, globaltexturesize, globalreliefimage, latitude);
                    globalmapimagecreated[5] = 1;
                }

                updatetextures(globalreliefimage, globaltexture);
                updateshader(spherepanels, baseshader, panels);
            }

            if (standardbutton("Elevation"))
            {
                mapview = elevation;

                if (globalmapimagecreated[0] == 0)
                {
                    drawglobalelevationmapimage(*world, globaltexturesize, globalelevationimage);
                    globalmapimagecreated[0] = 1;
                }

                updatetextures(globalelevationimage, globaltexture);
                updateshader(spherepanels, baseshader, panels);
            }

            if (standardbutton("Temperature"))
            {
                mapview = temperature;

                if (globalmapimagecreated[1] == 0)
                {
                    drawglobaltemperaturemapimage(*world, globaltexturesize, globaltemperatureimage, latitude);
                    globalmapimagecreated[1] = 1;
                }

                updatetextures(globaltemperatureimage, globaltexture);
                updateshader(spherepanels, baseshader, panels);
            }

            if (standardbutton("Precipitation"))
            {
                mapview = precipitation;

                if (globalmapimagecreated[2] == 0)
                {
                    drawglobalprecipitationmapimage(*world, globaltexturesize, globalprecipitationimage);
                    globalmapimagecreated[2] = 1;
                }

                updatetextures(globalprecipitationimage, globaltexture);
                updateshader(spherepanels, baseshader, panels);
            }

            if (standardbutton("Climate"))
            {
                mapview = climate;

                if (globalmapimagecreated[3] == 0)
                {
                    drawglobalclimatemapimage(*world, globaltexturesize, globalclimateimage);
                    globalmapimagecreated[3] = 1;
                }

                updatetextures(globalclimateimage, globaltexture);
                updateshader(spherepanels, baseshader, panels);
            }

            if (standardbutton("Rivers"))
            {
                mapview = rivers;

                if (globalmapimagecreated[4] == 0)
                {
                    drawglobalriversmapimage(*world, globaltexturesize, globalriversimage);
                    globalmapimagecreated[4] = 1;
                }

                updatetextures(globalriversimage, globaltexture);
                updateshader(spherepanels, baseshader, panels);
            }

            ImGui::Dummy(ImVec2(0.0f, linespace));

            ImGui::SetNextItemWidth(0);

            ImGui::Text("Month:");

            ImGui::PushItemWidth(buttonwidth);

            ImGui::SetCursorPosX(20);

            const char* monthitems[] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
            ImGui::Combo(" ", &currentmonth, monthitems, 12, 12);

            ImGui::Dummy(ImVec2(0.0f, linespace));

            ImGui::SetNextItemWidth(0);

            ImGui::Text("Other controls:");

            ImGui::PushItemWidth(100.0f);

            if (standardbutton("Properties"))
                toggle(*showworldproperties);

            if (standardbutton("Appearance"))
            {
                toggle(*showcolouroptions);
            }

            if (standardbutton("Movement"))
            {
                toggle(*showmovementoptions);

                if (*showmovementoptions == false)
                    savecontrols(keybindings, keybindingsno, camerazoomspeed, camerarotatespeed, planetrotatespeed, monthspeed, cameralock, monthrotate, "bindings.txt");
            }

            //string fpsstring = "FPS: " + to_string((int)(1000000.0f / frametime));
            //ImGui::Text(fpsstring.c_str());

            ImGui::End();

            // Now the text box.

            if (*focused)
            {
                float thiswidth = 590.0f;
                float thisheight = 320.0f;

                ImGui::SetNextWindowSize(ImVec2(thiswidth, thisheight), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowPos(focusedwindowpos, ImGuiCond_FirstUseEver);

                float defaultalign = thiswidth - 136.0f;
                float defaulty = thisheight - 48.0f;

                threeintegers thislatitude = converttodms(latitude[poi.face * edge * edge + poi.x * edge + poi.y]);
                threeintegers thislongitude = converttodms(longitude[poi.face * edge * edge + poi.x * edge + poi.y]);

                title = "Lat: " + to_string(thislatitude.x) + degree + " " + to_string(thislatitude.y) + "' " + to_string(thislatitude.z) + "\" ";
                    
                if (latitude[poi.face * edge * edge + poi.x * edge + poi.y] > 0.0f)
                    title = title + "S";
                else
                {
                    if (latitude[poi.face * edge * edge + poi.x * edge + poi.y] < 0.0f)
                        title = title + "N";
                }

                title = title + "    Long: " + to_string(thislongitude.x) + degree + " " + to_string(thislongitude.y) + "' " + to_string(thislongitude.z) + "\" ";

                if (longitude[poi.face * edge * edge + poi.x * edge + poi.y] > 0.0f)
                    title = title + "E";
                else
                {
                    if (longitude[poi.face * edge * edge + poi.x * edge + poi.y] < 0.0f)
                        title = title + "W";
                }

                //string location1 = "Face: " + to_string(poi.face) + " ";
                //string location2 = "x: " + to_string(poi.x) + " y: " + to_string(poi.y);
                //title = title + "   " + location1 + location2;

                ImGui::Begin(title.c_str(), focused, window_flags);
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + topmargin);

                focusedwindowpos = ImGui::GetWindowPos();

                int sealevel = world->sealevel();

                float rightalign = thiswidth * 0.581f; // 290;

                float topleftfigures = thiswidth * 0.21f; //105;
                float bottomleftfigures = thiswidth * 0.451f; //225;
                float toprightfigures = thiswidth * 0.822f; //410;
                float bottomrightfigures = thiswidth * 0.922f; //460;

                float pos1 = 10.0f;
                float pos2 = thiswidth * 0.23f;
                float pos3 = thiswidth * 0.581f;
                float pos4 = thiswidth * 0.822f;
                float pos5 = 570.0f;
                float pos6 = 690.0f;

                float pos7 = 850.0f;

                if (*focused)

                    int sealevel = world->sealevel();

                bool lake = 0;
                if (world->truelake(poi.face, poi.x, poi.y) != 0 || world->riftlakesurface(poi.face, poi.x, poi.y) != 0)
                    lake = 1;

                bool sea = 0;
                if (world->sea(poi.face, poi.x, poi.y) == 1)
                    sea = 1;

                bool river = 0;
                if (sea == 0 && lake == 0 && (world->riveraveflow(poi.face, poi.x, poi.y) > 0 || world->deltadir(poi.face, poi.x, poi.y) != 0))
                    river = 1;

                bool delta = 0;
                if (sea == 0 && world->deltadir(poi.face, poi.x, poi.y) != 0 && lake == 0)
                    delta = 1;

                // Elevation

                string elevationtext = "";

                int pointelevation = world->map(poi.face, poi.x, poi.y);

                if (world->lakesurface(poi.face, poi.x, poi.y) != 0)
                    pointelevation = world->lakesurface(poi.face, poi.x, poi.y) - 1;

                if (sea == 0)
                {
                    if (pointelevation > sealevel)
                    {
                        elevationtext = formatnumber(pointelevation - sealevel) + " m";

                        if (world->seatotal() != 0)
                            elevationtext = elevationtext + " above sea level";
                    }
                    else
                    {
                        if (lake)
                            elevationtext = formatnumber(sealevel - pointelevation) + " m below sea level (lake)";
                        else
                        {
                            if (world->seatotal() != 0)
                                elevationtext = formatnumber(world->lakesurface(poi.face, poi.x, poi.y) - sealevel) + " m above sea level";
                            else
                                elevationtext = formatnumber(world->lakesurface(poi.face, poi.x, poi.y) - sealevel) + " m";
                        }
                    }
                }
                else
                    elevationtext = formatnumber(sealevel - pointelevation) + " m below sea level";

                // Climate

                string climatetext = "";

                if (!sea)
                {
                    short climatetype = world->climate(poi.face, poi.x, poi.y);
                    climatetext = getclimatename(climatetype) + " (" + getclimatecode(climatetype) + ")";
                }

                // Wind

                int wind = world->wind(poi.face, poi.x, poi.y);
                string windtext = "";

                if (wind > 0)
                    windtext = "westerly";

                if (wind < 0)
                    windtext = "easterly";

                if (wind == 0 || wind > 50)
                    windtext = "none";

                // Specials

                string specialtext = "";

                if (world->special(poi.face, poi.x, poi.y) == 110)
                    specialtext = "Salt pan";

                if (world->special(poi.face, poi.x, poi.y) == 120)
                    specialtext = "Dunes";

                if (world->special(poi.face, poi.x, poi.y) == 130)
                    specialtext = "Wetlands (fresh water)";

                if (world->special(poi.face, poi.x, poi.y) == 131)
                    specialtext = "Wetlands (brackish)";

                if (world->special(poi.face, poi.x, poi.y) == 132)
                    specialtext = "Wetlands (salty)";

                if (world->volcano(poi.face, poi.x, poi.y) != 0)
                    specialtext = "Shield volcano";

                if (world->strato(poi.face, poi.x, poi.y))
                    specialtext = "Stratovolcano";

                if (world->volcano(poi.face, poi.x, poi.y) < 0)
                    specialtext = specialtext + " (extinct)";

                // Sea ice

                string seaicetext = "";

                if (sea)
                {
                    seaicetext = "none";

                    if (world->seaice(poi.face, poi.x, poi.y) == 2)
                        seaicetext = "permanent";
                    else
                    {
                        if (world->seaice(poi.face, poi.x, poi.y) == 1)
                            seaicetext = "seasonal";
                    }
                }

                // Flow

                string janflowtext = "";
                string julflowtext = "";
                string flowdirtext = "no dir";

                if (river)
                {
                    switch (world->riverdir(poi.face, poi.x, poi.y))
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

                    janflowtext = formatnumber(world->riverjan(poi.face, poi.x, poi.y)) + " m" + cube + "/s";
                    julflowtext = formatnumber(world->riverjul(poi.face, poi.x, poi.y)) + " m" + cube + "/s";
                }

                // Delta

                string deltajantext = "";
                string deltajultext = "";
                string deltadirtext = "";

                if (delta)
                {
                    switch (world->deltadir(poi.face, poi.x, poi.y))
                    {
                    case 1:
                        deltadirtext = "north";
                        break;

                    case 2:
                        deltadirtext = "northeast";
                        break;

                    case 3:
                        deltadirtext = "east";
                        break;

                    case 4:
                        deltadirtext = "southeast";
                        break;

                    case 5:
                        deltadirtext = "south";
                        break;

                    case 6:
                        deltadirtext = "southwest";
                        break;

                    case 7:
                        deltadirtext = "west";
                        break;

                    case 8:
                        deltadirtext = "northwest";
                        break;
                    }

                    deltajantext = formatnumber(world->deltajan(poi.face, poi.x, poi.y)) + " m" + cube + "/s";
                    deltajultext = formatnumber(world->deltajul(poi.face, poi.x, poi.y)) + " m" + cube + "/s";
                }

                // Lake

                string lakedepthtext = "";

                if (lake)
                {
                    int surface = world->lakesurface(poi.face, poi.x, poi.y);

                    int depth = 0;

                    if (surface == 0)
                    {
                        surface = world->riftlakesurface(poi.face, poi.x, poi.y);
                        depth = surface - world->riftlakebed(poi.face, poi.x, poi.y);
                    }
                    else
                    {
                        depth = surface - world->nom(poi.face, poi.x, poi.y);
                    }

                    elevationtext = formatnumber(surface - sealevel) + " m";

                    if (world->seatotal() != 0)
                        elevationtext = elevationtext + " above sea level";

                    string salt = "";

                    if (world->special(poi.face, poi.x, poi.y) == 100)
                        salt = " (salty)";

                    lakedepthtext = formatnumber(depth) + " metres" + salt;
                }

                string jantemptext = to_string(world->jantemp(poi.face, poi.x, poi.y)) + degree + "C " + to_string(simpleworld->jantemp(poi.face, poi.x * 4, poi.y * 4));
                string jultemptext = to_string(world->jultemp(poi.face, poi.x, poi.y)) + degree + "C " + to_string(simpleworld->jultemp(poi.face, poi.x * 4, poi.y * 4));

                string janraintext = to_string(world->janrain(poi.face, poi.x, poi.y)) + " mm";
                string julraintext = to_string(world->julrain(poi.face, poi.x, poi.y)) + " mm";

                //ImGui::Text(location1.c_str());
                //ImGui::Text(location2.c_str());
                //ImGui::Text("");

                ImGui::SetCursorPosX(pos1);
                ImGui::Text("Elevation: ");
                ImGui::SameLine(pos2);
                ImGui::Text(elevationtext.c_str());
                ImGui::SameLine(pos3);
                ImGui::Text("Wind: ");
                ImGui::SameLine(pos4);
                ImGui::Text(windtext.c_str());

                ImGui::SetCursorPosX(pos1);
                ImGui::Text("Jan temp: ");
                ImGui::SameLine(pos2);
                ImGui::Text(jantemptext.c_str());
                ImGui::SameLine(pos3);
                ImGui::Text("Jan rainfall: ");
                ImGui::SameLine(pos4);
                ImGui::Text(janraintext.c_str());

                ImGui::SetCursorPosX(pos1);
                ImGui::Text("July temp: ");
                ImGui::SameLine(pos2);
                ImGui::Text(jultemptext.c_str());
                ImGui::SameLine(pos3);
                ImGui::Text("July rainfall: ");
                ImGui::SameLine(pos4);
                ImGui::Text(julraintext.c_str());

                if (sea)
                    ImGui::Text(" ");
                else
                {
                    ImGui::SetCursorPosX(pos1);
                    ImGui::Text("Climate: ");
                    ImGui::SameLine(pos2);
                    ImGui::Text(climatetext.c_str());
                }

                ImGui::SetCursorPosX(pos2);
                ImGui::Text(specialtext.c_str());

                if (sea)
                {
                    ImGui::SetCursorPosX(pos1);
                    ImGui::Text("Sea ice: ");
                    ImGui::SameLine(pos2);
                    ImGui::Text(seaicetext.c_str());
                }
                else
                {
                    if (river)
                    {
                        ImGui::SetCursorPosX(pos1);
                        ImGui::Text("Jan river flow: ");
                        ImGui::SameLine(pos2);
                        ImGui::Text(janflowtext.c_str());

                        ImGui::SetCursorPosX(pos1);
                        ImGui::Text("July river flow: ");
                        ImGui::SameLine(pos2);
                        ImGui::Text(julflowtext.c_str());

                        ImGui::SetCursorPosX(pos1);
                        ImGui::Text("Direction: ");
                        ImGui::SameLine(pos2);
                        ImGui::Text(flowdirtext.c_str());
                    }

                    if (lake)
                    {
                        ImGui::SetCursorPosX(pos1);
                        ImGui::Text("Lake depth: ");
                        ImGui::SameLine(pos2);
                        ImGui::Text(lakedepthtext.c_str());
                    }

                    if (delta)
                    {
                        ImGui::SetCursorPosX(pos1);
                        ImGui::Text("Jan delta flow: ");
                        ImGui::SameLine(pos2);
                        ImGui::Text(deltajantext.c_str());

                        ImGui::SetCursorPosX(pos1);
                        ImGui::Text("July dela flow: ");
                        ImGui::SameLine(pos2);
                        ImGui::Text(deltajultext.c_str());

                        ImGui::SetCursorPosX(pos1);
                        ImGui::Text("Direction: ");
                        ImGui::SameLine(pos2);
                        ImGui::Text(deltadirtext.c_str());
                    }
                }

                ImGui::SetCursorPosX(defaultalign);
                ImGui::SetCursorPosY(defaulty - buttongap * 2.0f);
                if (ImGui::Button("Temperature", ImVec2(buttonwidth, buttonheight)))
                    toggle(*showglobaltemperaturechart);

                ImGui::SetCursorPosX(defaultalign);
                ImGui::SetCursorPosY(defaulty - buttongap);
                if (ImGui::Button("Precipitation", ImVec2(buttonwidth, buttonheight)))
                    toggle(*showglobalrainfallchart);

                ImGui::SetCursorPosX(defaultalign);
                ImGui::SetCursorPosY(defaulty);
                if (ImGui::Button("River flow", ImVec2(buttonwidth, buttonheight)))
                    toggle(*showglobalriverchart);


                /*
                ImGui::SetCursorPosX(993.0);
                ImGui::SetCursorPosY(122.0f);

                if (ImGui::Button("?", ImVec2(0.0f, 0.0f)))
                    toggle(showabout);
                */

                ImGui::End();
            }
        }

        // Now check to see if the map has been clicked on.

        if (screenmode == globalmapscreen && IsWindowFocused() && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && io.WantCaptureMouse == 0)
        {
            int xpos = GetMouseX();
            int ypos = scheight - GetMouseY();

            // Draw the globe onto a texture, using the colour picking textures.

            RenderTexture2D checkingtexture = LoadRenderTexture(scwidth, scheight);
            BeginTextureMode(checkingtexture);

            updatetextures(globalpickingimage, globaltexture);

            if (mapview == space)
                updateshader(spherepanels, baseshader, panels);

            BeginDrawing();
            ClearBackground(Color{ (unsigned char)(0), (unsigned char)(0),(unsigned char)(0),(unsigned char)(255) });

            BeginMode3D(camera);
            
            switch (world->size())
            {
            case 0:
                drawglobe(spherepanels, regionspherepanels0, world->size(), regionloc, globaltexture, regionaltexture, panels, globepos, globerotate, globetilt, globepitch, drawface, regioncreated);
                break;

            case 1:
                drawglobe(spherepanels, regionspherepanels1, world->size(), regionloc, globaltexture, regionaltexture, panels, globepos, globerotate, globetilt, globepitch, drawface, regioncreated);
                break;

            case 2:
                drawglobe(spherepanels, regionspherepanels2, world->size(), regionloc, globaltexture, regionaltexture, panels, globepos, globerotate, globetilt, globepitch, drawface, regioncreated);
                break;
            }

            EndMode3D();
            EndDrawing();

            Image checkingimage = LoadImageFromTexture(checkingtexture.texture);

            EndTextureMode();

            // Now put everything back.

            UnloadRenderTexture(checkingtexture);

            switch (mapview)
            {
            case elevation:
                updatetextures(globalelevationimage, globaltexture);
                break;

            case temperature:
                updatetextures(globaltemperatureimage, globaltexture);
                break;

            case precipitation:
                updatetextures(globalprecipitationimage, globaltexture);
                break;

            case climate:
                updatetextures(globalclimateimage, globaltexture);
                break;

            case rivers:
                updatetextures(globalriversimage, globaltexture);
                break;

            case relief:
                updatetextures(globalreliefimage, globaltexture);
                break;

            case space:
                updatetextures(globalspaceimage, globaltexture, currentmonth);
                updatetextures(globalspecularimage, globalspeculartexture, currentmonth);
                applyspeculartextures(spherepanels, globalspeculartexture, panels);

                updateshader(spherepanels, planetshader, panels);
                break;
            }

            // Now examine that image to work out which point was at the mouse position.

            Color pointcolour = GetImageColor(checkingimage, xpos, ypos);

            if (pointcolour.r == 0 && pointcolour.g == 0 && pointcolour.b == 0) // Blackness, so not on the globe.
                *focused = false;
            else // It's on the globe! Identify the point using brute force. It's inelegant and slow but it works well enough for us.
            {
                *focused = true;
                newworld = 0;

                int r = 0;
                int g = 0;
                int b = 1;

                int divstep = globaltexturesize / world->edge();

                for (int face = 0; face < 6; face++)
                {
                    for (int i = 0; i < globaltexturesize; i++)
                    {
                        int ii = i / divstep;

                        for (int j = 0; j < globaltexturesize; j++)
                        {
                            int jj = j / divstep;

                            if (r == pointcolour.r && g == pointcolour.g && b == pointcolour.b) // It matches!
                            {
                                poi.face = face;
                                poi.x = ii;
                                poi.y = jj;

                                face = 6;
                                i = globaltexturesize;
                                j = globaltexturesize;
                            }
                            else
                            {
                                b++; // Now advance to the next colour value.

                                if (b > 255)
                                {
                                    b = 0;

                                    g++;

                                    if (g > 255)
                                    {
                                        g = 0;

                                        r++;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            /*
            globepoint checkpoint = poi;

            float origlong;
            float origlat;

            getlonglat(edge, poi.face, poi.x, poi.y, origlong, origlat);

            for (int n = 0; n < edge * 2; n++)
            {
                world->settest(checkpoint.face, checkpoint.x, checkpoint.y, 100);

                checkpoint = dirpoint[checkpoint.face][checkpoint.x][checkpoint.y][0];

                if (checkpoint.face == -1)
                    n = edge * 2;

                //checkpoint = getnorthpoint(edge, checkpoint.face, checkpoint.x, checkpoint.y, longitude, latitude, origlong);
            }

            drawglobalprecipitationmapimage(*world, globaltexturesize, globalimagesize, globalprecipitationimage0, globalprecipitationimage1, globalprecipitationimage2, globalprecipitationimage3, globalprecipitationimage4, globalprecipitationimage5);
            globalmapimagecreated[2] = 1;

            */
        }

        // Movement controls, if being shown

        if (showui && *showmovementoptions && screenmode != settingsloadfailure)
        {
            ImGui::SetNextWindowPos(ImVec2(387, 26), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(431, 661), ImGuiCond_FirstUseEver);

            ImGui::Begin("Movement controls", showmovementoptions, window_flags);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + topmargin);

            float movewidth = ImGui::GetWindowWidth();
            float moveheight = ImGui::GetWindowHeight();

            float otheralign = movewidth * 0.762f; // 330;

            float heading1align = 30.0f;
            float column1align = 50.0f;

            float settingalign1 = 20.0f;
            float settingalign2 = 40.0f;

            float defaultalign = movewidth - 156.0f; // 560.0f;
            float defaulty = moveheight - 65.0f;

            float controlalign1 = heading1align;
            float controlalign2 = movewidth * 0.461f; //200;
            float controlalign3 = defaultalign; // 280;

            ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
            if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
            {
                if (ImGui::BeginTabItem("Settings"))
                {
                    ImGui::Text("  ");

                    ImGui::PushItemWidth(200);

                    ImGui::SetCursorPosX(heading1align);
                    ImGui::Text("Camera movement");

                    ImGui::SetCursorPosX(column1align);
                    ImGui::SliderFloat("Zoom speed", &camerazoomspeed, 0.0f, 1.0f, "%.2f");
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Speed of zooming in and out.");

                    ImGui::SetCursorPosX(column1align);
                    ImGui::SliderFloat("Rotate speed##2", &camerarotatespeed, 0.0f, 1.0f, "%.2f");
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Speed of rotating around planet.");

                    ImGui::Text("  ");

                    ImGui::SetCursorPosX(heading1align);
                    ImGui::Text("Planetary movement");

                    ImGui::SetCursorPosX(column1align);
                    ImGui::SliderFloat("Rotate speed", &planetrotatespeed, 0.0f, 1.0f, "%.2f");
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Speed at which the planet rotates.");

                    ImGui::SetCursorPosX(column1align);
                    ImGui::SliderFloat("Month transitions", &monthspeed, 0.0f, 1.0f, "%.2f");
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Speed of moving between months.");

                    ImGui::Text("  ");

                    ImGui::SetCursorPosX(column1align);
                    ImGui::Checkbox("Lock on planet", &cameralock);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Moves the camera as the planet rotates, to maintain a fixed lock.");

                    ImGui::SetCursorPosX(column1align);
                    ImGui::Checkbox("Rotate with months", &monthrotate);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Rotates the planet when months change, to maintain time of day.");

                    ImGui::SetCursorPosX(defaultalign);
                    ImGui::SetCursorPosY(defaulty);

                    if (ImGui::Button("Default##1", ImVec2(buttonwidth, buttonheight)))
                    {
                        initialisemovementsettings(camerazoomspeed, camerarotatespeed, planetrotatespeed, monthspeed, cameralock, monthrotate);
                    }
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Restore the default movement settings.");

                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Controls"))
                {
                    ImGui::Text("  ");

                    ImGui::SetCursorPosX((float)controlalign1);
                    ImGui::Text("Rotate planet east");
                    ImGui::SameLine((float)controlalign2);
                    ImGui::Text(keycodes[keybindings[1]].c_str());
                    ImGui::SameLine((float)controlalign3);
                    if (ImGui::Button("Change##1", ImVec2(buttonwidth, buttonheight)) && gettingkeybinding == 0)
                        gettingkeybinding = 1;

                    ImGui::SetCursorPosX((float)controlalign1);
                    ImGui::Text("Rotate planet west");
                    ImGui::SameLine((float)controlalign2);
                    ImGui::Text(keycodes[keybindings[2]].c_str());
                    ImGui::SameLine((float)controlalign3);
                    if (ImGui::Button("Change##2", ImVec2(buttonwidth, buttonheight)) && gettingkeybinding == 0)
                        gettingkeybinding = 2;

                    ImGui::SetCursorPosX((float)controlalign1);
                    ImGui::Text("Move camera east");
                    ImGui::SameLine((float)controlalign2);
                    ImGui::Text(keycodes[keybindings[4]].c_str());
                    ImGui::SameLine((float)controlalign3);
                    if (ImGui::Button("Change##4", ImVec2(buttonwidth, buttonheight)) && gettingkeybinding == 0)
                        gettingkeybinding = 4;

                    ImGui::SetCursorPosX((float)controlalign1);
                    ImGui::Text("Move camera west");
                    ImGui::SameLine((float)controlalign2);
                    ImGui::Text(keycodes[keybindings[3]].c_str());
                    ImGui::SameLine((float)controlalign3);
                    if (ImGui::Button("Change##3", ImVec2(buttonwidth, buttonheight)) && gettingkeybinding == 0)
                        gettingkeybinding = 3;

                    ImGui::SetCursorPosX((float)controlalign1);
                    ImGui::Text("Move camera north");
                    ImGui::SameLine((float)controlalign2);
                    ImGui::Text(keycodes[keybindings[5]].c_str());
                    ImGui::SameLine((float)controlalign3);
                    if (ImGui::Button("Change##5", ImVec2(buttonwidth, buttonheight)) && gettingkeybinding == 0)
                        gettingkeybinding = 5;

                    ImGui::SetCursorPosX((float)controlalign1);
                    ImGui::Text("Move camera south");
                    ImGui::SameLine((float)controlalign2);
                    ImGui::Text(keycodes[keybindings[6]].c_str());
                    ImGui::SameLine((float)controlalign3);
                    if (ImGui::Button("Change##6", ImVec2(buttonwidth, buttonheight)) && gettingkeybinding == 0)
                        gettingkeybinding = 6;

                    ImGui::SetCursorPosX((float)controlalign1);
                    ImGui::Text("Zoom camera in");
                    ImGui::SameLine((float)controlalign2);
                    ImGui::Text(keycodes[keybindings[7]].c_str());
                    ImGui::SameLine((float)controlalign3);
                    if (ImGui::Button("Change##7", ImVec2(buttonwidth, buttonheight)) && gettingkeybinding == 0)
                        gettingkeybinding = 7;

                    ImGui::SetCursorPosX((float)controlalign1);
                    ImGui::Text("Zoom camera out");
                    ImGui::SameLine((float)controlalign2);
                    ImGui::Text(keycodes[keybindings[8]].c_str());
                    ImGui::SameLine((float)controlalign3);
                    if (ImGui::Button("Change##8", ImVec2(buttonwidth, buttonheight)) && gettingkeybinding == 0)
                        gettingkeybinding = 8;

                    ImGui::SetCursorPosX((float)controlalign1);
                    ImGui::Text("Previous month");
                    ImGui::SameLine((float)controlalign2);
                    ImGui::Text(keycodes[keybindings[10]].c_str());
                    ImGui::SameLine((float)controlalign3);
                    if (ImGui::Button("Change##10", ImVec2(buttonwidth, buttonheight)) && gettingkeybinding == 0)
                        gettingkeybinding = 10;

                    ImGui::SetCursorPosX((float)controlalign1);
                    ImGui::Text("Next month");
                    ImGui::SameLine((float)controlalign2);
                    ImGui::Text(keycodes[keybindings[11]].c_str());
                    ImGui::SameLine((float)controlalign3);
                    if (ImGui::Button("Change##11", ImVec2(buttonwidth, buttonheight)) && gettingkeybinding == 0)
                        gettingkeybinding = 11;

                    ImGui::SetCursorPosX((float)controlalign1);
                    ImGui::Text("Show/hide UI");
                    ImGui::SameLine((float)controlalign2);
                    ImGui::Text(keycodes[keybindings[9]].c_str());
                    ImGui::SameLine((float)controlalign3);
                    if (ImGui::Button("Change##9", ImVec2(buttonwidth, buttonheight)) && gettingkeybinding == 0)
                        gettingkeybinding = 9;

                    ImGui::SetCursorPosX(defaultalign);
                    ImGui::SetCursorPosY(defaulty);

                    if (ImGui::Button("Default##2", ImVec2(buttonwidth, buttonheight)))
                    {
                        initialisekeybindings(keybindings);
                    }
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Restore the default controls.");


                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }

            ImGui::SetCursorPos(ImVec2(controlalign3 + 20.0f, 430.0f));

            ImGui::End();
        }

        // Key binding change window, if being shown

        if (gettingkeybinding != 0)
        {
            ImGui::SetNextWindowFocus();

            ImGui::SetNextWindowSize(ImVec2(360, 95), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowPos(ImVec2(412, 214), ImGuiCond_FirstUseEver);

            ImGui::Begin("Select key binding", NULL, window_flags);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + topmargin);

            string select = "Please press a key to ";

            switch (gettingkeybinding)
            {
            case 1:
                select = select + "rotate planet east.";
                break;

            case 2:
                select = select + "rotate planet west.";
                break;

            case 3:
                select = select + "move camera west.";
                break;

            case 4:
                select = select + "move camera east.";
                break;

            case 5:
                select = select + "move camera north";
                break;

            case 6:
                select = select + "move camera south.";
                break;

            case 7:
                select = select + "zoom camera in.";
                break;

            case 8:
                select = select + "zoom camera out.";
                break;

            case 9:
                select = select + "show/hide UI.";
                break;

            case 10:
                select = select + "go to previous month.";
                break;

            case 11:
                select = select + "go to next month.";
                break;
            }

            ImGui::Text(select.c_str());

            ImGui::End();
        }

        // Colour options, if being shown

        if (showui && *showcolouroptions && screenmode != settingsloadfailure)
        {
            ImGui::SetNextWindowPos(ImVec2(316, 24), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(716, 716), ImGuiCond_FirstUseEver);

            ImGui::Begin("Appearance controls", showcolouroptions, window_flags);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + topmargin);

            float colourwidth = ImGui::GetWindowWidth();
            float colourheight = ImGui::GetWindowHeight();

            float heading1align = 30.0f;
            float heading2align = colourwidth / 1.99f; // 360.0f;

            float column1align = 50.0f;
            float column2align = heading2align + 20.0f;

            float colour1align = column1align;
            float colour2align = colourwidth / 2.47f; // 290.0f;
            float colour3align = colourwidth / 1.4f; // 510.0f;

            float defaultalign = colourwidth - 156.0f; // 560.0f;
            float defaulty = colourheight - 65.0f;

            ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
            if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
            {
                if (ImGui::BeginTabItem("Textures"))
                {
                    ImGui::Text(" ");

                    ImGui::PushItemWidth(200);

                    ImGui::SameLine(heading1align);
                    ImGui::Text("Colours");

                    ImGui::SetCursorPosX(colour1align);
                    ImGui::ColorEdit3("    Shallow sea", (float*)&seacolour, ImGuiColorEditFlags_NoInputs);
                    ImGui::SameLine(colour2align);
                    ImGui::ColorEdit3("    Ocean", (float*)&oceancolour, ImGuiColorEditFlags_NoInputs);
                    ImGui::SameLine(colour3align);
                    ImGui::ColorEdit3("    Deep ocean", (float*)&deepoceancolour, ImGuiColorEditFlags_NoInputs);

                    ImGui::SetCursorPosX(colour1align);
                    ImGui::ColorEdit3("    Base land", (float*)&basecolour, ImGuiColorEditFlags_NoInputs);
                    ImGui::SameLine(colour2align);
                    ImGui::ColorEdit3("    Grassland", (float*)&grasscolour, ImGuiColorEditFlags_NoInputs);
                    ImGui::SameLine(colour3align);
                    ImGui::ColorEdit3("    Low temperate", (float*)&basetempcolour, ImGuiColorEditFlags_NoInputs);

                    ImGui::SetCursorPosX(colour1align);
                    ImGui::ColorEdit3("    High temperate", (float*)&highbasecolour, ImGuiColorEditFlags_NoInputs);
                    ImGui::SameLine(colour2align);
                    ImGui::ColorEdit3("    Low desert", (float*)&desertcolour, ImGuiColorEditFlags_NoInputs);
                    ImGui::SameLine(colour3align);
                    ImGui::ColorEdit3("    High desert", (float*)&highdesertcolour, ImGuiColorEditFlags_NoInputs);

                    ImGui::SetCursorPosX(colour1align);
                    ImGui::ColorEdit3("    Cold desert", (float*)&colddesertcolour, ImGuiColorEditFlags_NoInputs);
                    ImGui::SameLine(colour2align);
                    ImGui::ColorEdit3("    Mild tundra", (float*)&eqtundracolour, ImGuiColorEditFlags_NoInputs);
                    ImGui::SameLine(colour3align);
                    ImGui::ColorEdit3("    Tundra", (float*)&tundracolour, ImGuiColorEditFlags_NoInputs);

                    ImGui::SetCursorPosX(colour1align);
                    ImGui::ColorEdit3("    Arctic", (float*)&coldcolour, ImGuiColorEditFlags_NoInputs);
                    ImGui::SameLine(colour2align);
                    ImGui::ColorEdit3("    Sea ice", (float*)&seaicecolour, ImGuiColorEditFlags_NoInputs);
                    ImGui::SameLine(colour3align);
                    ImGui::ColorEdit3("    Glaciers", (float*)&glaciercolour, ImGuiColorEditFlags_NoInputs);

                    ImGui::SetCursorPosX(colour1align);
                    ImGui::ColorEdit3("    Salt pans", (float*)&saltpancolour, ImGuiColorEditFlags_NoInputs);
                    ImGui::SameLine(colour2align);
                    ImGui::ColorEdit3("    Dunes", (float*)&ergcolour, ImGuiColorEditFlags_NoInputs);
                    ImGui::SameLine(colour3align);
                    ImGui::ColorEdit3("    Rivers", (float*)&rivercolour, ImGuiColorEditFlags_NoInputs);

                    ImGui::SetCursorPosX(colour1align);
                    ImGui::ColorEdit3("    Lakes", (float*)&lakecolour, ImGuiColorEditFlags_NoInputs);
                    ImGui::SameLine(colour2align);
                    ImGui::ColorEdit3("    Wetlands", (float*)&wetlandscolour, ImGuiColorEditFlags_NoInputs);
                    ImGui::SameLine(colour3align);
                    ImGui::ColorEdit3("    Mangroves", (float*)&mangrovecolour, ImGuiColorEditFlags_NoInputs);

                    ImGui::SetCursorPosX(colour1align);
                    ImGui::ColorEdit3("    Mud", (float*)&mudcolour, ImGuiColorEditFlags_NoInputs);
                    ImGui::SameLine(colour2align);
                    ImGui::ColorEdit3("    Sand", (float*)&sandcolour, ImGuiColorEditFlags_NoInputs);
                    ImGui::SameLine(colour3align);
                    ImGui::ColorEdit3("    Shingle", (float*)&shinglecolour, ImGuiColorEditFlags_NoInputs);

                    ImGui::Text(" ");

                    ImGui::SetCursorPosX(heading1align);
                    ImGui::Text("Other controls");

                    ImGui::SetCursorPosX(column1align);
                    ImGui::Checkbox("Only cliffs use high base colour", &colourcliffs);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Uses high base colour only on steep slopes. May look better on non-tectonic worlds with high plateaux.");

                    ImGui::SetCursorPosX(column1align);
                    ImGui::Checkbox("Show mangrove forests", &mangroves);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Shows mangrove forests on tropical mud flats and salt wetlands.");

                    ImGui::Text(" ");
                    string infotext1 = "If any of these settings are changed, all textures will be updated on closing the window.";
                    string infotext2 = "This will cause a short delay - please be patient.";
                    ImGui::SetCursorPosX(heading1align);
                    ImGui::Text(infotext1.c_str());
                    ImGui::SetCursorPosX(heading1align);
                    ImGui::Text(infotext2.c_str());

                    ImGui::SetCursorPosX(defaultalign);
                    ImGui::SetCursorPosY(defaulty);

                    if (ImGui::Button("Default", ImVec2(buttonwidth, buttonheight)))
                    {
                        initialisemapcolours(*world);

                        // Now put all the correct values into the settings.

                        seacolour.x = (float)world->sea1() / 255.f;
                        seacolour.y = (float)world->sea2() / 255.f;
                        seacolour.z = (float)world->sea3() / 255.f;
                        seacolour.w = 1.f;

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

                        marblingland = world->landmarbling();
                        marblinglake = world->lakemarbling();
                        marblingsea = world->seamarbling();

                        minriverflowglobal = world->minriverflowglobal();
                        minriverflowregional = world->minriverflowregional();

                        globalriversentry = world->minriverflowglobal();
                        regionalriversentry = world->minriverflowregional();

                        snowchange = world->snowchange() - 1;
                        seaiceappearance = world->seaiceappearance() - 1;
                        colourcliffs = world->colourcliffs();
                        mangroves = world->showmangroves();

                        colourschanged = 1;
                    }

                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Effects"))
                {
                    ImGui::Text(" ");

                    ImGui::PushItemWidth(200);

                    ImGui::SameLine(heading1align);
                    ImGui::Text("Additional colours");

                    ImGui::SetCursorPosX(colour1align);
                    ImGui::ColorEdit3("    Highlight", (float*)&highlightcolour, ImGuiColorEditFlags_NoInputs);
                    ImGui::SameLine(colour2align);
                    ImGui::ColorEdit3("    Sunlight", (float*)&suncolour, ImGuiColorEditFlags_NoInputs);
                    ImGui::SameLine(colour3align);
                    ImGui::ColorEdit3("    Bloom", (float*)&bloomcolour, ImGuiColorEditFlags_NoInputs);

                    ImGui::SetCursorPosX(colour1align);
                    ImGui::ColorEdit3("    Atmosphere", (float*)&atmoscolour, ImGuiColorEditFlags_NoInputs);
                    ImGui::SameLine(colour2align);
                    ImGui::ColorEdit3("    Galaxy haze 1", (float*)&gal1hazecolour, ImGuiColorEditFlags_NoInputs);
                    ImGui::SameLine(colour3align);
                    ImGui::ColorEdit3("    Galaxy haze 2", (float*)&gal2hazecolour, ImGuiColorEditFlags_NoInputs);

                    ImGui::SetCursorPosX(colour1align);
                    ImGui::ColorEdit3("    Dusk", (float*)&duskcolour, ImGuiColorEditFlags_NoInputs);
                    ImGui::SameLine(colour2align);
                    ImGui::ColorEdit3("    Nebulae 1", (float*)&gal1nebulacolour, ImGuiColorEditFlags_NoInputs);
                    ImGui::SameLine(colour3align);
                    ImGui::ColorEdit3("    Nebulae 2", (float*)&gal2nebulacolour, ImGuiColorEditFlags_NoInputs);

                    ImGui::Text(" ");

                    ImGui::SetCursorPosX(heading1align);
                    ImGui::Text("Sun effects");

                    ImGui::SameLine(heading2align);
                    ImGui::Text("Planet effects");

                    ImGui::SetCursorPosX(column1align);
                    ImGui::SliderFloat("Bloom", &bloomdist, 0.0f, 1.0f, "%.2f");
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Radius of bloom effect on the sun.");
                    ImGui::SameLine(column2align);
                    ImGui::SliderFloat("Roughness", &normal, 0.0f, 1.0f, " % .2f");
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("How bumpy the terrain appears.");

                    ImGui::SetCursorPosX(column1align);
                    ImGui::SliderFloat("Glare", &sunglare, 0.0f, 1.0f, "%.2f");
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Strength of horizontal glare effect.");
                    ImGui::SameLine(column2align);
                    ImGui::SliderFloat("Haziness", &haze, 0.0f, 1.0f, "%.2f");
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Changes the atmospheric density.");

                    ImGui::SetCursorPosX(column1align);
                    ImGui::SliderFloat("Lens flare", &sunlens, 0.0f, 1.0f, "%.2f");
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Strength of lens artefacts.");
                    ImGui::SameLine(column2align);
                    ImGui::SliderFloat("Ambient", &ambient, 0.0f, 1.0f, "%.2f");
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Changes brightness of dark areas.");

                    ImGui::SetCursorPosX(column1align);
                    ImGui::SliderFloat("Ray strength", &sunraystrength, 0.0f, 1.0f, "%.2f");
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Strength of rays effect from the sun.");
                    ImGui::SameLine(column2align);
                    ImGui::SliderFloat("Specular", &specular, 0.0f, 1.0f, "%.2f");
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Changes how shiny the planet is.");

                    ImGui::SetCursorPosX(column1align);
                    ImGui::SliderInt("Ray number", &sunrayno, 2, 50);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Radial symmetry of sun rays.");
                    ImGui::SameLine(column2align);
                    ImGui::SliderFloat("Gamma", &gamma, 0.0f, 1.0f, "%.2f");
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("How light the planet appears.");

                    ImGui::SetCursorPosX(column2align);
                    ImGui::SliderFloat("Clouds", &cloudcover, 0.0f, 1.0f, "%.2f");
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Changes extent of cloud cover.");

                    ImGui::SetCursorPosX(heading1align);
                    ImGui::Text("Background effects");

                    ImGui::SetCursorPosX(column1align);
                    ImGui::SliderFloat("Star density", &starbright, 0.0f, 1.0f, "%.2f");
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Whether to show dimmer stars.");

                    ImGui::SetCursorPosX(column1align);
                    ImGui::SliderFloat("Star colour", &starcolour, 0.0f, 1.0f, "%.2f");
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("How colourful stars should be.");

                    ImGui::SetCursorPosX(column1align);
                    ImGui::SliderFloat("Galaxy haze", &starhaze, 0.0f, 1.0f, "%.2f");
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Haziness in denser stellar regions.");

                    ImGui::SetCursorPosX(column1align);
                    ImGui::SliderFloat("Nebulae", &starnebula, 0.0f, 1.0f, "%.2f");
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Closer galactic nebulae.");

                    ImGui::SetCursorPosX(defaultalign);
                    ImGui::SetCursorPosY(defaulty);

                    if (ImGui::Button("Default", ImVec2(buttonwidth, buttonheight)))
                    {
                        initialisespacesettings(*world);

                        highlightcolour.x = (float)world->highlight1() / 255.f;
                        highlightcolour.y = (float)world->highlight2() / 255.f;
                        highlightcolour.z = (float)world->highlight3() / 255.f;
                        highlightcolour.w = 1.f;

                        atmoscolour.x = (float)world->atmos1() / 255.f;
                        atmoscolour.y = (float)world->atmos2() / 255.f;
                        atmoscolour.z = (float)world->atmos3() / 255.f;
                        atmoscolour.w = 1.f;

                        duskcolour.x = (float)world->dusk1() / 255.f;
                        duskcolour.y = (float)world->dusk2() / 255.f;
                        duskcolour.z = (float)world->dusk3() / 255.f;
                        duskcolour.w = 1.f;

                        suncolour.x = (float)world->sun1() / 255.f;
                        suncolour.y = (float)world->sun2() / 255.f;
                        suncolour.z = (float)world->sun3() / 255.f;
                        suncolour.w = 1.f;

                        gal1hazecolour.x = (float)world->gal1haze1() / 255.f;
                        gal1hazecolour.y = (float)world->gal1haze2() / 255.f;
                        gal1hazecolour.z = (float)world->gal1haze3() / 255.f;
                        gal1hazecolour.w = 1.f;

                        gal2hazecolour.x = (float)world->gal2haze1() / 255.f;
                        gal2hazecolour.y = (float)world->gal2haze2() / 255.f;
                        gal2hazecolour.z = (float)world->gal2haze3() / 255.f;
                        gal2hazecolour.w = 1.f;

                        gal1nebulacolour.x = (float)world->gal1nebula1() / 255.f;
                        gal1nebulacolour.y = (float)world->gal1nebula2() / 255.f;
                        gal1nebulacolour.z = (float)world->gal1nebula3() / 255.f;
                        gal1nebulacolour.w = 1.f;

                        gal2nebulacolour.x = (float)world->gal2nebula1() / 255.f;
                        gal2nebulacolour.y = (float)world->gal2nebula2() / 255.f;
                        gal2nebulacolour.z = (float)world->gal2nebula3() / 255.f;
                        gal2nebulacolour.w = 1.f;

                        bloomcolour.x = (float)world->bloom1() / 255.f;
                        bloomcolour.y = (float)world->bloom2() / 255.f;
                        bloomcolour.z = (float)world->bloom3() / 255.f;
                        bloomcolour.w = 1.f;

                        normal = world->normal();
                        ambient = world->ambient();
                        gamma = world->gamma();
                        haze = world->haze();
                        specular = world->specular();
                        cloudcover = world->cloudcover();
                        bloomdist = world->bloomdist();
                        sunglare = world->sunglare();
                        sunlens = world->sunlens();
                        sunrayno = world->sunrayno();
                        sunraystrength = world->sunraystrength();
                        starbright = world->starbright();
                        starcolour = world->starcolour();
                        starhaze = world->starhaze();
                        starnebula = world->starnebula();
                    }

                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }

            /*
            ImGui::SetCursorPos(ImVec2(75.0f, 525.0f));

            if (ImGui::Button("Save", ImVec2(buttonwidth, buttonheight)))
            {
                //ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".uws", ".");

                savingsettings = 1;
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Save appearance settings.");

            ImGui::SameLine();
            if (ImGui::Button("Load", ImVec2(buttonwidth, buttonheight)))
            {
                //ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".uws", ".");

                loadingsettings = 1;
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Load appearance settings.");

            ImGui::SameLine();
            if (ImGui::Button("Default", ImVec2(buttonwidth, buttonheight)))
            {
                initialisemapcolours(*world);

                // Now put all the correct values into the settings.

                seacolour.x = (float)world->sea1() / 255.f;
                seacolour.y = (float)world->sea2() / 255.f;
                seacolour.z = (float)world->sea3() / 255.f;
                seacolour.w = 1.f;

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

                marblingland = world->landmarbling();
                marblinglake = world->lakemarbling();
                marblingsea = world->seamarbling();

                minriverflowglobal = world->minriverflowglobal();
                minriverflowregional = world->minriverflowregional();

                globalriversentry = world->minriverflowglobal();
                regionalriversentry = world->minriverflowregional();

                snowchange = world->snowchange() - 1;
                seaiceappearance = world->seaiceappearance() - 1;
                colourcliffs = world->colourcliffs();
                mangroves = world->showmangroves();

                
                //drawhighlightobjects(*world, *highlightimage, highlightsize, *minihighlightimage, minihighlightsize);

                //highlighttexture->loadFromImage(*highlightimage);
                //highlight->setTexture(*highlighttexture);

                //minihighlighttexture->loadFromImage(*minihighlightimage);
                //minihighlight->setTexture(*minihighlighttexture);
                //minihighlight->setOrigin((float)minihighlightsize / 2.0f, (float)minihighlightsize / 2.0f);
                

                colourschanged = 1;
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Restore the default appearance settings.");
            */

            /* This bit needs sorting properly!
            ImGui::SameLine();
            if (ImGui::Button("Close", ImVec2(buttonwidth, buttonheight)))
            {
                if (othermonthsneedupdating)
                {
                    //drawglobalspacemapimage(*world, *simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, 0, 11, 0, 5);

                    texturethread0 = thread([&world, &simpleworld, &globaltexturesize, &globalspaceimage, &globalspecularimage, &latitude, &proportions, &rainpoint] {drawglobalspacemapimage(*world, *simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, 0, 11, 0, 0); });
                    texturethread1 = thread([&world, &simpleworld, &globaltexturesize, &globalspaceimage, &globalspecularimage, &latitude, &proportions, &rainpoint] {drawglobalspacemapimage(*world, *simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, 0, 11, 1, 1); });
                    texturethread2 = thread([&world, &simpleworld, &globaltexturesize, &globalspaceimage, &globalspecularimage, &latitude, &proportions, &rainpoint] {drawglobalspacemapimage(*world, *simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, 0, 11, 2, 2); });
                    texturethread3 = thread([&world, &simpleworld, &globaltexturesize, &globalspaceimage, &globalspecularimage, &latitude, &proportions, &rainpoint] {drawglobalspacemapimage(*world, *simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, 0, 11, 3, 3); });
                    texturethread4 = thread([&world, &simpleworld, &globaltexturesize, &globalspaceimage, &globalspecularimage, &latitude, &proportions, &rainpoint] {drawglobalspacemapimage(*world, *simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, 0, 11, 4, 4); });
                    texturethread5 = thread([&world, &simpleworld, &globaltexturesize, &globalspaceimage, &globalspecularimage, &latitude, &proportions, &rainpoint] {drawglobalspacemapimage(*world, *simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, 0, 11, 5, 5); });

                    texturethread0.join();
                    texturethread1.join();
                    texturethread2.join();
                    texturethread3.join();
                    texturethread4.join();
                    texturethread5.join();

                    othermonthsneedupdating = 0;
                }

                *showcolouroptions = false;
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Close the appearance panel.");
            */

            ImGui::End();
        }

        // If we need to update the textures because colours have changed

        if (othermonthsneedupdating && *showcolouroptions == false)
        {
            //drawglobalspacemapimage(*world, *simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, 0, 11, 0, 5);

            texturethread0 = thread([&world, &simpleworld, &globaltexturesize, &globalspaceimage, &globalspecularimage, &latitude, &proportions, &rainpoint] {drawglobalspacemapimage(*world, *simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, 0, 11, 0, 0); });
            texturethread1 = thread([&world, &simpleworld, &globaltexturesize, &globalspaceimage, &globalspecularimage, &latitude, &proportions, &rainpoint] {drawglobalspacemapimage(*world, *simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, 0, 11, 1, 1); });
            texturethread2 = thread([&world, &simpleworld, &globaltexturesize, &globalspaceimage, &globalspecularimage, &latitude, &proportions, &rainpoint] {drawglobalspacemapimage(*world, *simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, 0, 11, 2, 2); });
            texturethread3 = thread([&world, &simpleworld, &globaltexturesize, &globalspaceimage, &globalspecularimage, &latitude, &proportions, &rainpoint] {drawglobalspacemapimage(*world, *simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, 0, 11, 3, 3); });
            texturethread4 = thread([&world, &simpleworld, &globaltexturesize, &globalspaceimage, &globalspecularimage, &latitude, &proportions, &rainpoint] {drawglobalspacemapimage(*world, *simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, 0, 11, 4, 4); });
            texturethread5 = thread([&world, &simpleworld, &globaltexturesize, &globalspaceimage, &globalspecularimage, &latitude, &proportions, &rainpoint] {drawglobalspacemapimage(*world, *simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, 0, 11, 5, 5); });

            texturethread0.join();
            texturethread1.join();
            texturethread2.join();
            texturethread3.join();
            texturethread4.join();
            texturethread5.join();

            othermonthsneedupdating = 0;
        }

        // World properties window, if being shown

        if (showui && *showworldproperties)
        {
            ImGui::SetNextWindowPos(ImVec2(265, 60), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(567, 384), ImGuiCond_FirstUseEver);

            ImGui::Begin("World properties", showworldproperties, window_flags);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + topmargin);

            float propwidth = ImGui::GetWindowWidth();

            float rightalign = propwidth * 0.581f; // 290;

            float topleftfigures = propwidth * 0.21f; //105;
            float bottomleftfigures = propwidth * 0.451f; //225;
            float toprightfigures = propwidth * 0.822f; //410;
            float bottomrightfigures = propwidth * 0.922f; //460;

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
                perihelionvalue = "December";

            if (world->perihelion() == 1)
                perihelionvalue = "June";

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
            string glacialvalue = to_string(world->glacialtemp()) + degree;

            ImGui::Text(sizeinfo.c_str());

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Size of planet. (Earth: large; Mars: medium; Moon: small)");

            ImGui::SameLine(topleftfigures);
            ImGui::Text(sizevalue.c_str());

            ImGui::SameLine(rightalign);

            ImGui::Text(gravityinfo.c_str());

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Affects mountain and valley sizes. (Earth: 1.00g)");

            ImGui::SameLine(toprightfigures);
            ImGui::Text(gravityvalue.c_str());

            ImGui::Text(typeinfo.c_str());

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Terrain category. (Earth: large tectonic)");

            ImGui::SameLine(topleftfigures);
            ImGui::Text(typevalue.c_str());

            ImGui::SameLine(rightalign);

            ImGui::Text(lunarinfo.c_str());

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Affects tides and coastal regions. (Earth: 1.00)");

            ImGui::SameLine(toprightfigures);
            ImGui::Text(lunarvalue.c_str());

            ImGui::Text(rotationinfo.c_str());

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Affects weather patterns. (Earth: west to east)");

            ImGui::SameLine(topleftfigures);
            ImGui::Text(rotationvalue.c_str());

            ImGui::SameLine(rightalign);

            ImGui::Text(tiltinfo.c_str());

            string tilttip = "Affects seasonal variation in temperature. (Earth: 22.5" + degree + ")";

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip(tilttip.c_str());

            ImGui::SameLine(toprightfigures);
            ImGui::Text(tiltvalue.c_str());

            ImGui::Text(eccentricityinfo.c_str());

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("How elliptical the orbit is. (Earth: 0.0167)");

            ImGui::SameLine(topleftfigures);
            ImGui::Text(eccentricityvalue.c_str());

            ImGui::SameLine(rightalign);

            ImGui::Text(perihelioninfo.c_str());

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("When the planet is closest to the sun. (Earth: December)");

            ImGui::SameLine(toprightfigures);
            ImGui::Text(perihelionvalue.c_str());

            ImGui::Text("   ");

            ImGui::Text(tempdecreaseinfo.c_str());

            string tempdecreasetip = "Affects how much colder it gets higher up. (Earth: 6.5" + degree + ")";

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip(tempdecreasetip.c_str());

            ImGui::SameLine(bottomleftfigures);
            ImGui::Text(tempdecreasevalue.c_str());

            ImGui::SameLine(rightalign);

            ImGui::Text(moistureinfo.c_str());

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Affects how much moisture wind picks up from the ocean. (Earth: 1.0)");

            ImGui::SameLine(bottomrightfigures);
            ImGui::Text(moisturevalue.c_str());

            ImGui::Text("   ");

            ImGui::Text(averageinfo.c_str());

            string avetip = "Earth: 14" + degree;

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip(avetip.c_str());

            ImGui::SameLine(bottomleftfigures);
            ImGui::Text(averagevalue.c_str());

            ImGui::SameLine(rightalign);

            ImGui::Text(glacialinfo.c_str());

            string glacialtip = "Areas below this average temperature may show signs of past glaciation. (Earth: 4" + degree + ")";

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip(glacialtip.c_str());

            ImGui::SameLine(bottomrightfigures);
            ImGui::Text(glacialvalue.c_str());

            ImGui::Text(northpoleinfo.c_str());

            string northtip = "Adjustment to north pole temperature. (Earth: +3" + degree + ")";

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip(northtip.c_str());

            ImGui::SameLine(bottomleftfigures);
            ImGui::Text(northpolevalue.c_str());

            ImGui::Text(southpoleinfo.c_str());

            string southtip = "Adjustment to south pole temperature. (Earth: -3" + degree + ")";

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip(southtip.c_str());

            ImGui::SameLine(bottomleftfigures);
            ImGui::Text(southpolevalue.c_str());

            ImGui::SameLine(rightalign);

            ImGui::End();
        }

        // World edit properties screen, if being shown

        if (showui && showworldeditproperties)
        {
            int rightalign = 280;

            ImGui::SetNextWindowPos(ImVec2(420, 50), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(415, 427), ImGuiCond_FirstUseEver);

            ImGui::Begin("World properties##2", closeflag, window_flags);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + topmargin);

            ImGui::PushItemWidth(180);

            const char* rotationitems[] = { "East to west", "West to east" };
            static int item_current = 0;
            ImGui::Combo("Rotation", &currentrotation, rotationitems, 2);

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Affects weather patterns. (Earth: west to east)");

            const char* perihelionitems[] = { "December", "June" };
            static int pitem_current = 0;
            ImGui::Combo("Perihelion", &currentperihelion, perihelionitems, 2);

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("When the planet is closest to the sun. (Earth: December)");

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

            if (ImGui::Button("Close", ImVec2(buttonwidth, buttonheight)))
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

        // Global temperature/precipitation/river flow charts, if being shown

        if (showui && *showglobaltemperaturechart)
        {
            float temp[12];

            world->monthlytemps(poi.face, poi.x, poi.y, temp, latitude);

            float barwidth = 40.0f;

            ImGui::SetNextWindowPos(ImVec2(680, 385), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(518, 173), ImGuiCond_FirstUseEver);

            string panelname = "Temperature (" + degree + "C)";

            ImGui::Begin(panelname.c_str(), showglobaltemperaturechart, window_flags);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + topmargin);

            float lowest = temp[0];
            float highest = temp[0];

            for (int n = 1; n < 12; n++)
            {
                if (temp[n] < lowest)
                    lowest = temp[n];

                if (temp[n] > highest)
                    highest = temp[n];
            }

            float subzero = 0.0f;

            if (lowest < 0.0f) // Ensure that all values are at least 0
            {
                subzero = 0.0f - lowest;

                for (int n = 0; n < 12; n++)
                    temp[n] = temp[n] + subzero;

                lowest = temp[0];
                highest = temp[0];

                for (int n = 1; n < 12; n++)
                {
                    if (temp[n] < lowest)
                        lowest = temp[n];

                    if (temp[n] > highest)
                        highest = temp[n];
                }
            }

            float temprange = highest - lowest;

            ImGui::Text(" ");

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

                ImGui::SameLine((float)barwidth * (n + 1) - (ImGui::CalcTextSize(datetext.c_str()).x) / 2);

                ImGui::Text(datetext.c_str());
            }

            ImGui::End();
        }

        if (showui && *showglobalrainfallchart)
        {
            float temp[12];

            world->monthlytemps(poi.face, poi.x, poi.y, temp, latitude);

            float rain[12];

            world->monthlyrain(poi.face, poi.x, poi.y, temp, rain);

            float barwidth = 40.0f;

            ImGui::SetNextWindowPos(ImVec2(680, 246), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(518, 173), ImGuiCond_FirstUseEver);

            ImGui::Begin("Precipitation (mm)", showglobalrainfallchart, window_flags);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + topmargin);

            float lowest = rain[0];
            float highest = rain[0];

            for (int n = 1; n < 12; n++)
            {
                if (rain[n] < lowest)
                    lowest = rain[n];

                if (rain[n] > highest)
                    highest = rain[n];
            }

            float rainrange = highest - lowest;

            ImGui::Text(" ");

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
                string raininfotext;

                if (rain[n] < 1000)
                    raininfotext = formatnumber((int)rain[n]);
                else
                    raininfotext = numbertok((int)rain[n]);

                ImGui::SameLine((float)barwidth * (n + 1) - (ImGui::CalcTextSize(raininfotext.c_str()).x) / 2);

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

        if (showui && *showglobalriverchart)
        {
            float temp[12];

            world->monthlytemps(poi.face, poi.x, poi.y, temp, latitude);

            float rain[12];

            world->monthlyrain(poi.face, poi.x, poi.y, temp, rain);

            int flow[12];

            world->monthlyflow(poi.face, poi.x, poi.y, temp, rain, flow);

            float barwidth = 40.0f;

            ImGui::SetNextWindowPos(ImVec2(680, 246), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(518, 173), ImGuiCond_FirstUseEver);

            string panelname = "River flow (m" + cube + "/s)";

            ImGui::Begin(panelname.c_str(), showglobalriverchart, window_flags);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + topmargin);

            int lowest = flow[0];
            int highest = flow[0];

            for (int n = 1; n < 12; n++)
            {
                if (flow[n] < lowest)
                    lowest = flow[n];

                if (flow[n] > highest)
                    highest = flow[n];
            }

            int flowrange = highest - lowest;

            ImGui::Text(" ");

            for (int n = 0; n < 13; n++)
            {
                ImGui::SameLine((float)barwidth * (float)n + barwidth / 2.0f);

                float value;

                if (n < 12)
                    value = (float)flow[n];
                else
                    value = 0.0f;

                float flowarr[] = { value / highest };

                float colour1 = 0.0f;
                float colour2 = 0.0f;
                float colour3 = 0.0f;

                float flowfall = (float)flow[n];
                flowfall = flowfall / 2.0f;

                colour1 = 255.0f - flowfall;
                colour2 = 255.0f - flowfall;
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

                ImGui::PlotHistogram(" ", flowarr, IM_ARRAYSIZE(flowarr), 0, NULL, 0.0f, 1.0f, ImVec2(0.0f, barwidth));
            }

            ImGui::Text(" ");

            for (int n = 0; n < 12; n++)
            {
                string flowinfotext;

                if (flow[n] < 1000)
                    flowinfotext = formatnumber(flow[n]);
                else
                    flowinfotext = numbertok(flow[n]);

                ImGui::SameLine((float)barwidth * (n + 1) - (ImGui::CalcTextSize(flowinfotext.c_str()).x) / 2);

                ImGui::Text(flowinfotext.c_str());
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

        // Now update the colours if necessary.

        if (world->sea1() != seacolour.x * 255.f || world->sea2() != seacolour.y * 255.f || world->sea3() != seacolour.z * 255.f)
            colourschanged = 1;

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
            // This one's different as it doesn't involve redrawing the map.

            world->sethighlight1((int)(highlightcolour.x * 255.f));
            world->sethighlight2((int)(highlightcolour.y * 255.f));
            world->sethighlight3((int)(highlightcolour.z * 255.f));

            highlightcolour.x = (float)world->highlight1() / 255.f;
            highlightcolour.y = (float)world->highlight2() / 255.f;
            highlightcolour.z = (float)world->highlight3() / 255.f;
        }

        if (world->landmarbling() != marblingland || world->lakemarbling() != marblinglake || world->seamarbling() != marblingsea)
            colourschanged = 1;

        if (world->minriverflowglobal() != globalriversentry || world->minriverflowregional() != regionalriversentry)
            colourschanged = 1;

        if (world->colourcliffs() != colourcliffs)
            colourschanged = 1;

        if (world->showmangroves() != mangroves)
            colourschanged = 1;

        world->setnormal(normal);
        world->setambient(ambient);
        world->setgamma(gamma);
        world->sethaze(haze);
        world->setspecular(specular);
        world->setcloudcover(cloudcover);

        world->setbloomdist(bloomdist);
        world->setsunglare(sunglare);
        world->setsunlens(sunlens);
        world->setsunrayno(sunrayno);
        world->setsunraystrength(sunraystrength);
        world->setstarbright(starbright);
        world->setstarcolour(starcolour);
        world->setstarhaze(starhaze);
        world->setstarnebula(starnebula);

        world->setatmos1((int)(atmoscolour.x * 255.f));
        world->setatmos2((int)(atmoscolour.y * 255.f));
        world->setatmos3((int)(atmoscolour.z * 255.f));

        world->setdusk1((int)(duskcolour.x * 255.f));
        world->setdusk2((int)(duskcolour.y * 255.f));
        world->setdusk3((int)(duskcolour.z * 255.f));

        world->setsun1((int)(suncolour.x * 255.f));
        world->setsun2((int)(suncolour.y * 255.f));
        world->setsun3((int)(suncolour.z * 255.f));

        world->setgal1haze1((int)(gal1hazecolour.x * 255.f));
        world->setgal1haze2((int)(gal1hazecolour.y * 255.f));
        world->setgal1haze3((int)(gal1hazecolour.z * 255.f));

        world->setgal2haze1((int)(gal2hazecolour.x * 255.f));
        world->setgal2haze2((int)(gal2hazecolour.y * 255.f));
        world->setgal2haze3((int)(gal2hazecolour.z * 255.f));

        world->setgal1nebula1((int)(gal1nebulacolour.x * 255.f));
        world->setgal1nebula2((int)(gal1nebulacolour.y * 255.f));
        world->setgal1nebula3((int)(gal1nebulacolour.z * 255.f));

        world->setgal2nebula1((int)(gal2nebulacolour.x * 255.f));
        world->setgal2nebula2((int)(gal2nebulacolour.y * 255.f));
        world->setgal2nebula3((int)(gal2nebulacolour.z * 255.f));

        world->setbloom1((int)(bloomcolour.x * 255.f));
        world->setbloom2((int)(bloomcolour.y * 255.f));
        world->setbloom3((int)(bloomcolour.z * 255.f));

        if (*showcolouroptions && colourschanged == 1)
        {
            world->setsea1((int)(seacolour.x * 255.f));
            world->setsea2((int)(seacolour.y * 255.f));
            world->setsea3((int)(seacolour.z * 255.f));

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

            world->setlandmarbling(marblingland);
            world->setlakemarbling(marblinglake);
            world->setseamarbling(marblingsea);

            world->setminriverflowglobal(globalriversentry);
            world->setminriverflowregional(regionalriversentry);

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

            if (mapview == space) // We just redraw the current month, to save time. We'll redraw all the rest when the window is closed.
            {
                //drawglobalspacemapimage(*world, *simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, currentmonth, currentmonth, 0, 5);

                texturethread0 = thread([&world, &simpleworld, &globaltexturesize, &globalspaceimage, &globalspecularimage, &latitude, &proportions, &rainpoint, &currentmonth] {drawglobalspacemapimage(*world, *simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, currentmonth, currentmonth, 0, 0); });
                texturethread1 = thread([&world, &simpleworld, &globaltexturesize, &globalspaceimage, &globalspecularimage, &latitude, &proportions, &rainpoint, &currentmonth] {drawglobalspacemapimage(*world, *simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, currentmonth, currentmonth, 1, 1); });
                texturethread2 = thread([&world, &simpleworld, &globaltexturesize, &globalspaceimage, &globalspecularimage, &latitude, &proportions, &rainpoint, &currentmonth] {drawglobalspacemapimage(*world, *simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, currentmonth, currentmonth, 2, 2); });
                texturethread3 = thread([&world, &simpleworld, &globaltexturesize, &globalspaceimage, &globalspecularimage, &latitude, &proportions, &rainpoint, &currentmonth] {drawglobalspacemapimage(*world, *simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, currentmonth, currentmonth, 3, 3); });
                texturethread4 = thread([&world, &simpleworld, &globaltexturesize, &globalspaceimage, &globalspecularimage, &latitude, &proportions, &rainpoint, &currentmonth] {drawglobalspacemapimage(*world, *simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, currentmonth, currentmonth, 4, 4); });
                texturethread5 = thread([&world, &simpleworld, &globaltexturesize, &globalspaceimage, &globalspecularimage, &latitude, &proportions, &rainpoint, &currentmonth] {drawglobalspacemapimage(*world, *simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, currentmonth, currentmonth, 5, 5); });

                texturethread0.join();
                texturethread1.join();
                texturethread2.join();
                texturethread3.join();
                texturethread4.join();
                texturethread5.join();

                globalmapimagecreated[6] = 1;
                globalmapimagecreated[5] = 0;

                updatetextures(globalspaceimage, globaltexture, currentmonth);
                updatetextures(globalspecularimage, globalspeculartexture, currentmonth);
                updateshader(spherepanels, planetshader, panels);

                othermonthsneedupdating = 1;
            }

            if (mapview == relief)
            {
                drawglobalreliefmapimage(*world, globaltexturesize, globalreliefimage, latitude);
                globalmapimagecreated[5] = 1;
                globalmapimagecreated[6] = 0;

                updatetextures(globalreliefimage, globaltexture);
                updateshader(spherepanels, baseshader, panels);
            }

            if (mapview != space && mapview != relief)
            {
                mapview = space;

                //drawglobalspacemapimage(*world, *simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, 0, 11, 0, 5);

                texturethread0 = thread([&world, &simpleworld, &globaltexturesize, &globalspaceimage, &globalspecularimage, &latitude, &proportions, &rainpoint] {drawglobalspacemapimage(*world, *simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, 0, 11, 0, 0); });
                texturethread1 = thread([&world, &simpleworld, &globaltexturesize, &globalspaceimage, &globalspecularimage, &latitude, &proportions, &rainpoint] {drawglobalspacemapimage(*world, *simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, 0, 11, 1, 1); });
                texturethread2 = thread([&world, &simpleworld, &globaltexturesize, &globalspaceimage, &globalspecularimage, &latitude, &proportions, &rainpoint] {drawglobalspacemapimage(*world, *simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, 0, 11, 2, 2); });
                texturethread3 = thread([&world, &simpleworld, &globaltexturesize, &globalspaceimage, &globalspecularimage, &latitude, &proportions, &rainpoint] {drawglobalspacemapimage(*world, *simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, 0, 11, 3, 3); });
                texturethread4 = thread([&world, &simpleworld, &globaltexturesize, &globalspaceimage, &globalspecularimage, &latitude, &proportions, &rainpoint] {drawglobalspacemapimage(*world, *simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, 0, 11, 4, 4); });
                texturethread5 = thread([&world, &simpleworld, &globaltexturesize, &globalspaceimage, &globalspecularimage, &latitude, &proportions, &rainpoint] {drawglobalspacemapimage(*world, *simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, 0, 11, 5, 5); });

                texturethread0.join();
                texturethread1.join();
                texturethread2.join();
                texturethread3.join();
                texturethread4.join();
                texturethread5.join();

                globalmapimagecreated[6] = 1;
                globalmapimagecreated[5] = 0;

                updatetextures(globalspaceimage, globaltexture, currentmonth);
                updatetextures(globalspecularimage, globalspeculartexture, currentmonth);
                applyspeculartextures(spherepanels, globalspeculartexture, panels);

                updateshader(spherepanels, planetshader, panels);
            }

            colourschanged = 0;
        }

        // Update positions

        bool viewmoved = 0; // This will be 1 if we're looking at a new point on the globe.

        float speedfactor = frametime / targetframetime;

        // Move to a new month, if necessary

        if (*showmovementoptions == false && gettingkeybinding == 0 && IsKeyPressed(keybindings[10]))
        {
            currentmonth--;

            if (currentmonth < 0)
                currentmonth = 11;
        }

        if (*showmovementoptions == false && gettingkeybinding == 0 && IsKeyPressed(keybindings[11]))
        {
            currentmonth++;

            if (currentmonth > 11)
                currentmonth = 0;
        }

        if (*showcolouroptions || mapview != space) // Don't allow month changes if the colour options screen is up, because we haven't yet updated all the months, or if we're not in the space view, because then it messes up the textures for the other views.
            currentmonth = oldmonth;

        if (oldmonth != currentmonth) // If the month has changed.
        {
            if (currentstep == 0) // Only initiate a new change if we're not already doing one!
            {
                showcurrentmonth = 0.0f;

                // First, update the textures.

                updatetextures(globalspaceimage, globalspacetextureold, oldmonth);
                updatetextures(globalspecularimage, globalspeculartextureold, oldmonth);

                updatetextures(globalspaceimage, globaltexture, currentmonth);
                updatetextures(globalspecularimage, globalspeculartexture, currentmonth);

                applyoldspacetextures(spherepanels, globalspacetextureold, panels);
                applyoldspeculartextures(spherepanels, globalspeculartextureold, panels);

                applyspeculartextures(spherepanels, globalspeculartexture, panels);

                // Now work out where to move the sun to.

                float diff = world->sunlong(currentmonth) - world->sunlong(oldmonth);

                if (diff > 180.0f)
                    diff = diff - 360.0f;

                if (diff < -180.0f)
                    diff = diff + 360.0f;

                thismonthsteps = (float)monthsteps * (1.0f - monthspeed); // Number of steps we'll take for this.

                if (thismonthsteps < 1.0f)
                    thismonthsteps = 1.0f;

                movesunlong = diff / thismonthsteps;

                diff = world->sunlat(currentmonth) - world->sunlat(oldmonth);

                movesunlat = diff / thismonthsteps;

                diff = world->sundist(currentmonth) - world->sundist(oldmonth);

                movesundist = diff / thismonthsteps;

                currentstep = (int)thismonthsteps;

                oldmonth = currentmonth;
            }
            else
                currentmonth = oldmonth;
        }

        if (currentstep == 0)
        {
            currentsunlong = world->sunlong(currentmonth);
            currentsunlat = world->sunlat(currentmonth);
            currentsundist = world->sundist(currentmonth);
            movesunlong = 0.0f;
            movesunlat = 0.0f;
            showcurrentmonth = 1.0f;
            thismonthsteps = 0.0f;
        }
        else
        {
            currentsunlong = currentsunlong + movesunlong;
            currentsunlat = currentsunlat + movesunlat;
            currentsundist = currentsundist + movesundist;

            if (currentsunlong >= 360.0f)
                currentsunlong = currentsunlong - 360.0f;

            if (currentsunlong < 0.0f)
                currentsunlong = currentsunlong + 360.0f;

            if (monthrotate)
            {
                globerotate = globerotate - movesunlong; // Rotate the globe the other way, so the time of day doesn't change.

                if (globerotate >= 360.0f)
                    globerotate = globerotate - 360.0f;

                if (globerotate < 0.0f)
                    globerotate = globerotate + 360.0f;

                if (cameralock)
                {
                    camlong = camlong + movesunlong; // And the camera

                    if (camlong >= 360.0f)
                        camlong = camlong - 360.0f;

                    if (camlong < 0.0f)
                        camlong = camlong + 360.0f;
                }
            }

            currentstep--;

            showcurrentmonth = showcurrentmonth + 1.0f / thismonthsteps;
        }

        if (*showmovementoptions == false && gettingkeybinding == 0 && IsKeyDown(keybindings[1]))
        {
            viewmoved = 1;

            globerotate = globerotate + 1.0f * speedfactor;

            if (cameralock)
                camlong = camlong - 1.0f * speedfactor;
        }

        if (*showmovementoptions == false && gettingkeybinding == 0 && IsKeyDown(keybindings[2]))
        {
            viewmoved = 1;

            globerotate = globerotate - 1.0f * speedfactor;

            if (cameralock)
                camlong = camlong + 1.0f * speedfactor;
        }

        if (planetrotatespeed != 0.0f)
        {
            viewmoved = 1;

            if (world->rotation()) // Rotates the same way as Earth
            {
                globerotate = globerotate + planetrotatespeed * 0.5f * speedfactor;

                if (cameralock)
                    camlong = camlong - planetrotatespeed * 0.5f * speedfactor;
            }
            else // The other way
            {
                globerotate = globerotate - planetrotatespeed * 0.5f * speedfactor;

                if (cameralock)
                    camlong = camlong + planetrotatespeed * 0.5f * speedfactor;
            }

            if (globerotate < 0.0f)
                globerotate = globerotate + 360.0f;

            if (globerotate > 360.0f)
                globerotate = globerotate - 360.0f;
        }

        if (*showmovementoptions == false && gettingkeybinding == 0 && IsKeyDown(keybindings[3]))
        {
            viewmoved = 1;

            camlong = camlong + 2.0f * camerarotatespeed * speedfactor;
        }

        if (*showmovementoptions == false && gettingkeybinding == 0 && IsKeyDown(keybindings[4]))
        {
            viewmoved = 1;

            camlong = camlong - 2.0f * camerarotatespeed * speedfactor;
        }

        if (camlong < 0.0f)
            camlong = camlong + 360.0f;

        if (camlong >= 360.0f)
            camlong = camlong - 360.0f;

        if (*showmovementoptions == false && gettingkeybinding == 0 && IsKeyDown(keybindings[5]))
        {
            viewmoved = 1;

            camlat = camlat + 2.0f * camerarotatespeed * speedfactor;
        }

        if (*showmovementoptions == false && gettingkeybinding == 0 && IsKeyDown(keybindings[6]))
        {
            viewmoved = 1;

            camlat = camlat - 2.0f * camerarotatespeed * speedfactor;
        }

        if (camlat > 89.0f)
            camlat = 89.0f;

        if (camlat < -89.0f)
            camlat = -89.0f;

        if (*showmovementoptions == false && gettingkeybinding == 0 && IsKeyDown(keybindings[7]))
            camdist = camdist - camdist * 0.005f * camerazoomspeed * speedfactor;

        if (*showmovementoptions == false && gettingkeybinding == 0 && IsKeyDown(keybindings[8]))
            camdist = camdist + camdist * 0.005f * camerazoomspeed * speedfactor;

        if (gettingkeybinding == 0 && IsKeyPressed(keybindings[9]))
            toggle(showui);

        relativelong = getrelativelong(globerotate, camlong);
        relativelat = globepitch + camlat;

        drawfacecheck(drawface, relativelong, relativelat);

        if (gettingkeybinding > 0) // Checking for a new key binding
        {
            for (int n = 0; n <= 348; n++)
            {
                if (IsKeyPressed(n))
                {
                    if (keycodes[n] != "")
                    {
                        keybindings[gettingkeybinding] = n;
                        n = 350;
                        gettingkeybinding = 0;
                    }
                }
            }
        }

        cameraposition = getcoordinatesfromlatlong(camlat, camlong, camdist);

        Vector3 campos = { cameraposition.x, cameraposition.y, cameraposition.z };
        camera.position = campos;

        camera.target = Vector3{ 0.0f, 0.0f, 0.0f };

        float camerapos[3] = { camera.position.x, camera.position.y, camera.position.z };
        SetShaderValue(planetshader, planetshader.locs[SHADER_LOC_VECTOR_VIEW], camerapos, SHADER_UNIFORM_VEC3);
        SetShaderValue(bloomshader, bloomshader.locs[SHADER_LOC_VECTOR_VIEW], camerapos, SHADER_UNIFORM_VEC3);

        // Put the sun in the correct position.

        threefloats currentsunposition = getcoordinatesfromlatlong(currentsunlat, currentsunlong, sundistance * currentsundist);

        lightpos[0] = currentsunposition.x; // sunpositions[currentmonth].x;
        lightpos[1] = currentsunposition.y; // sunpositions[currentmonth].y;
        lightpos[2] = currentsunposition.z; // sunpositions[currentmonth].z;

        SetShaderValue(planetshader, lightposLoc, lightpos, SHADER_UNIFORM_VEC3);

        // Now adjust the position for the actual sun object, to keep it relative to the camera.

        Vector3 sunpos; // The sun is drawn relative to the camera, so the camera can't approach it.
        sunpos.x = lightpos[0] + camerapos[0];
        sunpos.y = lightpos[1] + camerapos[1];
        sunpos.z = lightpos[2] + camerapos[2];

        Vector2 sunpos2d = GetWorldToScreen(sunpos, camera);

        bool sunvisible = 1;

        if (sunpos2d.x < 0 || sunpos2d.x >= scwidth || sunpos2d.y < 0 || sunpos2d.y >= scheight)
            sunvisible = 0;

        float thissunpos2d[2]; // Convert this to coordinates that the shader will understand
        thissunpos2d[0] = sunpos2d.x / (float)scwidth;
        thissunpos2d[1] = 1.0f - sunpos2d.y / (float)scheight;

        SetShaderValue(bloomshader, sunpos2dLoc, thissunpos2d, SHADER_UNIFORM_VEC2);
        SetShaderValue(lensshader, sunpos2dlensLoc, thissunpos2d, SHADER_UNIFORM_VEC2);

        // Rendering

        // First, update uniforms.

        float ambientcol[4] = { ambient, ambient, ambient, 1.0f };
        SetShaderValue(planetshader, ambientLoc, ambientcol, SHADER_UNIFORM_VEC4);

        float atmoscol[4] = { atmoscolour.x, atmoscolour.y, atmoscolour.z, 1.0f };
        SetShaderValue(planetshader, atmosLoc, atmoscol, SHADER_UNIFORM_VEC4);

        float duskcol[4] = { duskcolour.x, duskcolour.y, duskcolour.z, 1.0f };
        SetShaderValue(planetshader, duskLoc, duskcol, SHADER_UNIFORM_VEC4);

        float lightcol[4] = { suncolour.x, suncolour.y, suncolour.z, 1.0f };
        SetShaderValue(planetshader, lightcolLoc, lightcol, SHADER_UNIFORM_VEC4);

        SetShaderValue(planetshader, normalLoc, &normal, SHADER_UNIFORM_FLOAT);
        SetShaderValue(planetshader, gammaLoc, &gamma, SHADER_UNIFORM_FLOAT);
        SetShaderValue(planetshader, hazeLoc, &haze, SHADER_UNIFORM_FLOAT);
        SetShaderValue(planetshader, specularLoc, &specular, SHADER_UNIFORM_FLOAT);
        SetShaderValue(planetshader, cloudcoverLoc, &cloudcover, SHADER_UNIFORM_FLOAT);
        SetShaderValue(planetshader, showcurrentmonthLoc, &showcurrentmonth, SHADER_UNIFORM_FLOAT);

        float gal1hazecol[4] = { gal1hazecolour.x, gal1hazecolour.y, gal1hazecolour.z, 1.0f };
        SetShaderValue(starshader, galhazecol1Loc, gal1hazecol, SHADER_UNIFORM_VEC4);

        float gal2hazecol[4] = { gal2hazecolour.x, gal2hazecolour.y, gal2hazecolour.z, 1.0f };
        SetShaderValue(starshader, galhazecol2Loc, gal2hazecol, SHADER_UNIFORM_VEC4);

        float gal1nebulacol[4] = { gal1nebulacolour.x, gal1nebulacolour.y, gal1nebulacolour.z, 1.0f };
        SetShaderValue(starshader, galnebcol1Loc, gal1nebulacol, SHADER_UNIFORM_VEC4);

        float gal2nebulacol[4] = { gal2nebulacolour.x, gal2nebulacolour.y, gal2nebulacolour.z, 1.0f };
        SetShaderValue(starshader, galnebcol2Loc, gal2nebulacol, SHADER_UNIFORM_VEC4);

        SetShaderValue(starshader, starbrightLoc, &starbright, SHADER_UNIFORM_FLOAT);
        SetShaderValue(starshader, starcolourLoc, &starcolour, SHADER_UNIFORM_FLOAT);
        SetShaderValue(starshader, starhazeLoc, &starhaze, SHADER_UNIFORM_FLOAT);
        SetShaderValue(starshader, starnebulaLoc, &starnebula, SHADER_UNIFORM_FLOAT);

        float bloomcol[4] = { bloomcolour.x, bloomcolour.y, bloomcolour.z, 1.0f };
        SetShaderValue(bloomshader, bloomcolLoc, bloomcol, SHADER_UNIFORM_VEC4);

        SetShaderValue(bloomshader, suncolLoc, lightcol, SHADER_UNIFORM_VEC4);
        SetShaderValue(bloomshader, bloomdistLoc, &bloomdist, SHADER_UNIFORM_FLOAT);
        SetShaderValue(bloomshader, sunglareLoc, &sunglare, SHADER_UNIFORM_FLOAT);
        SetShaderValue(bloomshader, sunraystrengthLoc, &sunraystrength, SHADER_UNIFORM_FLOAT);
        SetShaderValue(bloomshader, sunraynoLoc, &sunrayno, SHADER_UNIFORM_INT);

        SetShaderValue(lensshader, suncollensLoc, lightcol, SHADER_UNIFORM_VEC4);
        SetShaderValue(lensshader, sunlensLoc, &sunlens, SHADER_UNIFORM_FLOAT);

        // Next, we want to make the bloom texture.

        RenderTexture2D rendertexturebloom = LoadRenderTexture(scwidth, scheight);

        if (screenmode == globalmapscreen && mapview == space && sunvisible && camdist > 1.54f)
        {
            BeginTextureMode(rendertexturebloom);

            ClearBackground(Color{ (unsigned char)(0), (unsigned char)(0),(unsigned char)(0),(unsigned char)(255) });

            BeginMode3D(camera);

            // First, the planet in plain black.

            updateshader(spherepanels, blackshader, panels);

            drawbasicglobe(spherepanels, globaltexture, panels, globepos, globerotate, globetilt, globepitch, drawface);

            if (mapview == space)
                updateshader(spherepanels, planetshader, panels);
            else
                updateshader(spherepanels, baseshader, panels);

            // Now the sun.

            float sunradius = 12.0f; // 8.0f;

            BeginShaderMode(sunshader);

            DrawSphereEx(sunpos, sunradius, 32, 32, Color{ (unsigned char)(suncolour.x * 255.0), (unsigned char)(suncolour.y * 255.0), (unsigned char)(suncolour.z * 255.0), (unsigned char)255.0 });

            EndShaderMode();

            EndMode3D();
            EndTextureMode();

            // Now pass the sun's location to the bloom shader.

            float suncentre[2];
            suncentre[0] = sunpos2d.x / (float)scwidth;
            suncentre[1] = 1.0f - sunpos2d.y / (float)scheight;

            SetShaderValue(bloomshader, sunpos2dLoc, &suncentre, SHADER_UNIFORM_VEC2);
        }

        // Now draw everything else.

        ImGui::Render();
        BeginDrawing();
        ClearBackground(Color{ (unsigned char)(0), (unsigned char)(0),(unsigned char)(0),(unsigned char)(255) });

        if (screenmode == globalmapscreen) // Draw 3D objects here.
        {
            BeginMode3D(camera);

            if (mapview == space && camdist > 1.54f) // Draw the stars.
            {
                Vector3 skypos; // The stars are drawn centred on the camera.
                skypos.x = camerapos[0];
                skypos.y = camerapos[1];
                skypos.z = camerapos[2];

                drawbasicglobe(skypanels, starstexture, spanels, skypos, skyrotate, skytilt, skypitch, drawstarface);
            }

            // Draw the planet.

            switch (world->size())
            {
            case 0:
                drawglobe(spherepanels, regionspherepanels0, world->size(), regionloc, globaltexture, regionaltexture, panels, globepos, globerotate, globetilt, globepitch, drawface, regioncreated);
                break;

            case 1:
                drawglobe(spherepanels, regionspherepanels1, world->size(), regionloc, globaltexture, regionaltexture, panels, globepos, globerotate, globetilt, globepitch, drawface, regioncreated);
                break;

            case 2:
                drawglobe(spherepanels, regionspherepanels2, world->size(), regionloc, globaltexture, regionaltexture, panels, globepos, globerotate, globetilt, globepitch, drawface, regioncreated);
                break;
            }

            // Draw the highlight object.

            if (showui && *focused)
            {
                float origlong;
                float origlat;

                getlonglat(edge, poi.face, (float)poi.x + 0.5f, (float)poi.y + 0.5f, origlong, origlat);

                origlong = 180.0f - (origlong - 90.0f) - globerotate;

                highlightposition = getcoordinatesfromlatlong(-origlat, origlong, 1.0f);

                Vector3 highlightpos;
                highlightpos.x = highlightposition.x;
                highlightpos.y = highlightposition.y;
                highlightpos.z = highlightposition.z;

                float highlightradius = 0.006f;

                if (world->size() == 1)
                    highlightradius = highlightradius / 2.0f;

                if (world->size() == 2)
                    highlightradius = highlightradius / 4.0f;

                DrawSphereEx(highlightpos, highlightradius, 32, 32, Color{ (unsigned char)(highlightcolour.x * 255.0), (unsigned char)(highlightcolour.y * 255.0), (unsigned char)(highlightcolour.z * 255.0), (unsigned char)255.0 });
            }

            EndMode3D();

            // Now add the bloom texture over the top, doing the bloom effects as we do so.

            if (mapview == space && sunvisible && camdist > 1.54f)
            {
                Vector2 textureloc;
                textureloc.x = 0;
                textureloc.y = 9;

                Rectangle texturerec;

                texturerec.x = 0.0f;
                texturerec.y = textureypos; // 0.0f;
                texturerec.width = (float)rendertexturebloom.texture.width;
                texturerec.height = (float)-rendertexturebloom.texture.height;

                BeginShaderMode(bloomshader);
                DrawTextureRec(rendertexturebloom.texture, texturerec, textureloc, WHITE);
                EndShaderMode();

                // And the lens flare effects.

                if (sunlens > 0.0f)
                {
                    BeginShaderMode(lensshader);
                    DrawTextureRec(rendertexturebloom.texture, texturerec, textureloc, WHITE);
                    EndShaderMode();
                }
            }
        }

        // Now update where we are looking.

        if (screenmode == globalmapscreen) // && (viewmoved || facingpanel.face == -1))
        {
            for (int face = 0; face < 6; face++) // Make sure the globe position is up to date.
            {
                for (int i = 0; i < panels; i++)
                {
                    for (int j = 0; j < panels; j++)
                        spherepanels[face][i][j].transform = MatrixRotateXYZ(Vector3{ DEG2RAD * globetilt, DEG2RAD * globerotate, DEG2RAD * globepitch });
                }
            }
            
            Ray ray = GetMouseRay(Vector2((float)scwidth * 0.5f, (float)scheight * 0.5f), camera); // GetScreenToWorldRay(GetMousePosition(), camera);

            float paneldist = 0.0f; // Because there may be two panels in line with the camera, so we want to identify the closer.

            for (int face = 0; face < 6; face++)
            {
                for (int i = 0; i < panels; i++)
                {
                    for (int j = 0; j < panels; j++)
                    {
                        RayCollision collisioninfo = { 0 };

                        for (int m = 0; m < spherepanels[face][i][j].meshCount; m++)
                        {
                            collisioninfo = GetRayCollisionMesh(ray, spherepanels[face][i][j].meshes[m], spherepanels[face][i][j].transform);

                            if (collisioninfo.hit && (collisioninfo.distance < paneldist || paneldist == 0.0f))
                            {
                                facingpanel.face = face;
                                facingpanel.x = i;
                                facingpanel.y = j;
                                m = 500;

                                paneldist = collisioninfo.distance;
                            }
                        }
                    }
                }
            }
        }

        ImGui_ImplRaylib_RenderDrawData(ImGui::GetDrawData());
        EndDrawing();

        UnloadRenderTexture(rendertexturebloom);

        chrono::steady_clock::time_point frameendtime = chrono::steady_clock::now();

        frametime = (float)(chrono::duration_cast<std::chrono::microseconds>(frameendtime - framestarttime).count());
    }

    rlImGuiShutdown();

    CloseWindow();

    return 0;
}

// This function creates the world.

void createworld(planet &world, simpleplanet &simpleworld, int &edge, vector<float> &longitude, vector<float> &latitude, vector<fourglobepoints>& dirpoint, vector<int>& squareroot, vector<Image>& globalspaceimage, vector<Image>& globalspecularimage, vector<Image> &starsimage, vector <Image> &starhazeimage, vector<Image>& globalnormalimage, int globaltexturesize, int globallargetexturesize, int skytexturesize, float &skyrotate, float &skytilt, float &skypitch, boolshapetemplate landshape[], boolshapetemplate chainland[], boolshapetemplate smalllake[], boolshapetemplate largelake[], vector<float>& proportions, int rainpoint, int panels, vector<vector<vector<Model>>>& regionspherepanels, Shader planetshader, vector<fourdirs>& dircode, int &progressval, string &progresstext)
{
    long seed = world.seed();
    fast_srand(seed);

    int oldsize = world.size();

    initialiseworld(world);
    world.clear();

    changeworldproperties(world);

    //world->settilt(85.0f);
    //world->seteccentricity(0.8f);
    //world->setperihelion(1);

    //world.setsize(0);
    //world.settype(4);

    if (world.size() != oldsize)
    {
        updatereport(progressval, progresstext, "Adjusting for size");

        if (world.size() == 0) // Small
            edge = 128;

        if (world.size() == 1) // Medium
            edge = 256;

        if (world.size() == 2) // Large
            edge = 512;

        world.setedge(edge);

        for (int face = 0; face < 6; face++)
        {
            int vface = face * edge * edge;

            for (int i = 0; i < edge; i++)
            {
                int vi = vface + i * edge;

                for (int j = 0; j < edge; j++)
                {
                    int index = vi + j;

                    getlonglat(edge, face, (float)i, (float)j, longitude[index], latitude[index]);
                }
            }
        }

        createdirectiontable(edge, dirpoint, longitude, latitude);
    }

    int contno = random(1, 9);

    int thismergefactor = random(1, 15);

    if (random(1, 12) == 1) // Fairly rarely, have more fragmented continents
        thismergefactor = random(1, 25);

    int iterations = 4; // This is for worlds of terrain type 4.

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

    vector<vector<vector<int>>> mountaindrainage(6, vector<vector<int>>(edge, vector<int>(edge, 0)));
    vector<vector<vector<bool>>> shelves(6, vector<vector<bool>>(edge, vector<bool>(edge, 0)));

    // Save a record of this!

    string filename = "log.txt";

    saverecord(world, filename);

    // Actually generate the world

    //calculatesunpositions(world);

    // Do these concurrently, as they don't require any info about the world.

    thread cloudthread = thread([&world, &dirpoint] {makeclouds(world, dirpoint); });
    thread starthread = thread([&world, &starsimage, &starhazeimage, &skytexturesize, &skyrotate, &skytilt, &skypitch] { drawstarfield(world, starsimage, starhazeimage, skytexturesize, skyrotate, skytilt, skypitch); });

    // Main world generation.

    generateglobalterrain(world, 0, iterations, thismergefactor, -1, -1, landshape, chainland, mountaindrainage, shelves, longitude, latitude, dirpoint, squareroot, progressval, progresstext);
    generateglobalclimate(world, 1, 1, 1, smalllake, largelake, landshape, mountaindrainage, shelves, longitude, latitude, dirpoint, progressval, progresstext);

    cloudthread.join();
    starthread.join();

    createsimpleplanet(world, simpleworld, progressval, progresstext);

    saveendtime(world, filename);

    // Draw and apply the normal lighting map.

    updatereport(progressval, progresstext, "Preparing normal map");

    drawglobalnormalimage(world, simpleworld, globallargetexturesize, globalnormalimage, dirpoint, dircode);

    // Draw and apply the space image. Divide the six faces between six threads, for speed.

    updatereport(progressval, progresstext, "Drawing textures");

    thread texturethread0 = thread([&world, &simpleworld, &globaltexturesize, &globallargetexturesize, &globalspaceimage, &globalspecularimage, &latitude, &proportions, &rainpoint] {drawglobalspacemapimage(world, simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, 0, 11, 0, 0); });
    thread texturethread1 = thread([&world, &simpleworld, &globaltexturesize, &globallargetexturesize, &globalspaceimage, &globalspecularimage, &latitude, &proportions, &rainpoint] {drawglobalspacemapimage(world, simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, 0, 11, 1, 1); });
    thread texturethread2 = thread([&world, &simpleworld, &globaltexturesize, &globallargetexturesize, &globalspaceimage, &globalspecularimage, &latitude, &proportions, &rainpoint] {drawglobalspacemapimage(world, simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, 0, 11, 2, 2); });
    thread texturethread3 = thread([&world, &simpleworld, &globaltexturesize, &globallargetexturesize, &globalspaceimage, &globalspecularimage, &latitude, &proportions, &rainpoint] {drawglobalspacemapimage(world, simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, 0, 11, 3, 3); });
    thread texturethread4 = thread([&world, &simpleworld, &globaltexturesize, &globallargetexturesize, &globalspaceimage, &globalspecularimage, &latitude, &proportions, &rainpoint] {drawglobalspacemapimage(world, simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, 0, 11, 4, 4); });
    thread texturethread5 = thread([&world, &simpleworld, &globaltexturesize, &globallargetexturesize, &globalspaceimage, &globalspecularimage, &latitude, &proportions, &rainpoint] {drawglobalspacemapimage(world, simpleworld, globaltexturesize, globallargetexturesize, globalspaceimage, globalspecularimage, latitude, proportions, (float)rainpoint, 0, 11, 5, 5); });

    texturethread0.join();
    texturethread1.join();
    texturethread2.join();
    texturethread3.join();
    texturethread4.join();
    texturethread5.join();

    //updatereport("");
    updatereport(progressval, progresstext, "World generation completed.");
    //updatereport("");
}

// This function creates a region.
 
void createregion(planet& world, int panels, int maxregions, int thisregionobject, globepoint thisregionlocation, vector<globepoint> &regionID, vector<int> &regionloc, vector<short> &regioncreated, vector<short> &regioncreatedloc, int regionaltexturesize, vector<Image>& regionalelevationimage, vector<Image>& regionaltemperatureimage, vector<Image>& regionalprecipitationimage, vector<Image>& regionalclimateimage, vector<Image>& regionalriversimage, vector<Image>& regionalreliefimage, vector<Image>& regionalspaceimage, vector<Image>& regionalspecularimage, vector<Texture2D> &regionaltexture, vector<Texture2D>& regionalspeculartexture, vector<Texture2D>& regionalnormaltexture)
{
    // Actual region creation. (Just a delay for now, for testing purposes.)

    int delay = random(500000, 1000000);

    for (int n = 0; n < delay * 100; n++)
        int throwaway = 0;

    // Now create the images for the textures.

    vector<Image> regionalnormalimage(maxregions); // This one will never change, so we don't need it in the main thread.

    // Now apply the images to the textures.


    // Now clean up.

    int thisregionindex = thisregionlocation.face * panels * panels + thisregionlocation.x * panels + thisregionlocation.y;

    regionID[thisregionobject] = thisregionlocation;
    regionloc[thisregionindex] = thisregionobject;

    regioncreated[thisregionobject] = 1;
    regioncreatedloc[thisregionindex] = 1;
}

// This is a simple function to set up threads.

void threadinitialise()
{
    return;
}

// This is to test the threads functionality.

void threadtest(int &val, string &text)
{
    for (int m = 0; m < 100000; m++)
    {
        for (int n = 0; n < 10000; n++)
            int v = random(1, 100);

        val = random(1, 100);
        text = to_string(val);
    }

    cout << "Thread done!\n\n";
}

// This rotates a vector around another vector. (Taken from https://stackoverflow.com/questions/42421611/3d-vector-rotation-in-c )

/*
glm::dvec3 rotate(const glm::dvec3& v, const glm::dvec3& k, double theta)
{
    double cos_theta = cos(theta);
    double sin_theta = sin(theta);

    glm::dvec3 rotated = (v * cos_theta) + (glm::cross(k, v) * sin_theta) + (k * glm::dot(k, v)) * (1 - cos_theta);

    return rotated;
}
*/

// This generates a simple quad mesh (adapted from https://www.raylib.com/examples/models/loader.html?name=models_mesh_generation)

Mesh generatecustommesh(Vector3 p1, Vector3 p2, Vector3 p3, Vector3 p4, Vector3 n1, Vector3 n2, Vector3 n3, Vector3 n4, Vector2 t1, Vector2 t2, Vector2 t3, Vector2 t4)
{
    Mesh mesh = { 0 };
    mesh.triangleCount = 2;
    mesh.vertexCount = mesh.triangleCount * 3;
    mesh.vertices = (float*)MemAlloc(mesh.vertexCount * 3 * sizeof(float));    // 3 vertices, 3 coordinates each (x, y, z)
    mesh.texcoords = (float*)MemAlloc(mesh.vertexCount * 2 * sizeof(float));   // 3 vertices, 2 coordinates each (x, y)
    mesh.normals = (float*)MemAlloc(mesh.vertexCount * 3 * sizeof(float));     // 3 vertices, 3 coordinates each (x, y, z)

    // First triangle

    // Vertex 1
    mesh.vertices[0] = p1.x;
    mesh.vertices[1] = p1.y;
    mesh.vertices[2] = p1.z;
    mesh.normals[0] = n1.x;
    mesh.normals[1] = n1.y;
    mesh.normals[2] = n1.z;
    mesh.texcoords[0] = t1.x;
    mesh.texcoords[1] = t1.y;

    // Vertex 2
    mesh.vertices[3] = p3.x;
    mesh.vertices[4] = p3.y;
    mesh.vertices[5] = p3.z;
    mesh.normals[3] = n3.x;
    mesh.normals[4] = n3.y;
    mesh.normals[5] = n3.z;
    mesh.texcoords[2] = t3.x;
    mesh.texcoords[3] = t3.y;

    // Vertex 3
    mesh.vertices[6] = p2.x;
    mesh.vertices[7] = p2.y;
    mesh.vertices[8] = p2.z;
    mesh.normals[6] = n2.x;
    mesh.normals[7] = n2.y;
    mesh.normals[8] = n2.z;
    mesh.texcoords[4] = t2.x;
    mesh.texcoords[5] = t2.y;

    // Second triangle

    // Vertex 4
    mesh.vertices[9] = p1.x;
    mesh.vertices[10] = p1.y;
    mesh.vertices[11] = p1.z;
    mesh.normals[9] = n1.x;
    mesh.normals[10] = n1.y;
    mesh.normals[11] = n1.z;
    mesh.texcoords[6] = t1.x;
    mesh.texcoords[7] = t1.y;

    // Vertex 5
    mesh.vertices[12] = p4.x;
    mesh.vertices[13] = p4.y;
    mesh.vertices[14] = p4.z;
    mesh.normals[12] = n4.x;
    mesh.normals[13] = n4.y;
    mesh.normals[14] = n4.z;
    mesh.texcoords[8] = t4.x;
    mesh.texcoords[9] = t4.y;

    // Vertex 6
    mesh.vertices[15] = p3.x;
    mesh.vertices[16] = p3.y;
    mesh.vertices[17] = p3.z;
    mesh.normals[15] = n3.x;
    mesh.normals[16] = n3.y;
    mesh.normals[17] = n3.z;
    mesh.texcoords[10] = t3.x;
    mesh.texcoords[11] = t3.y;

    GenMeshTangents(&mesh);

    // Upload mesh data from CPU (RAM) to GPU (VRAM) memory
    UploadMesh(&mesh, false);

    //GenMeshTangents(&mesh);

    return mesh;
}

// This sets up the meshes for the cubesphere.

void setupspherepanels(vector<vector<vector<Model>>>& spherepanels, int panels, float size)
{
    // First, create six vectors of vertices to describe a cubesphere. Each vector holds the coordinates for one face.
    // The sphere transformations use formulae by Phil Nowell: http://mathproofs.blogspot.com/2005/07/mapping-cube-to-sphere.html

    double dpanels = (double)panels;
    double dsize = (double)size;

    // Face 0 (front)

    vector<vector<vector<double>>> face0vertices(panels + 1, vector<vector<double>>(panels + 1, vector<double>(3))); // This will hold the coordinates of the vertices for one side.

    for (double i = 0.0; i <= dpanels; i++) // Calculate the vertex coordinates
    {
        int ii = (int)i;

        for (double j = 0.0; j <= dpanels; j++)
        {
            int jj = (int)j;

            double x = (i / dpanels) * 2.0 - 1.0;
            double y = (j / dpanels) * 2.0 - 1.0;
            double z = -1.0;

            // Now transform those coordinates onto a portion of a sphere

            face0vertices[ii][jj][0] = x * sqrt(1.0 - (y * y / 2.0) - (z * z / 2.0) + ((y * y) * (z * z) / 3.0));
            face0vertices[ii][jj][1] = y * sqrt(1.0 - (z * z / 2.0) - (x * x / 2.0) + ((z * z) * (x * x) / 3.0));
            face0vertices[ii][jj][2] = z * sqrt(1.0 - (x * x / 2.0) - (y * y / 2.0) + ((x * x) * (y * y) / 3.0));
        }
    }

    // Face 1 (right)

    vector<vector<vector<double>>> face1vertices(panels + 1, vector<vector<double>>(panels + 1, vector<double>(3))); // This will hold the coordinates of the vertices for one side.

    for (double i = 0.0; i <= dpanels; i++) // Calculate the vertex coordinates
    {
        int ii = (int)i;

        for (double j = 0.0; j <= dpanels; j++)
        {
            int jj = (int)j;

            double x = 1.0;
            double y = (j / dpanels) * 2.0 - 1.0;
            double z = (i / dpanels) * 2.0 - 1.0;

            // Now transform those coordinates onto a portion of a sphere

            face1vertices[ii][jj][0] = x * sqrt(1.0 - (y * y / 2.0) - (z * z / 2.0) + ((y * y) * (z * z) / 3.0));
            face1vertices[ii][jj][1] = y * sqrt(1.0 - (z * z / 2.0) - (x * x / 2.0) + ((z * z) * (x * x) / 3.0));
            face1vertices[ii][jj][2] = z * sqrt(1.0 - (x * x / 2.0) - (y * y / 2.0) + ((x * x) * (y * y) / 3.0));
        }
    }

    // Face 2 (back)

    vector<vector<vector<double>>> face2vertices(panels + 1, vector<vector<double>>(panels + 1, vector<double>(3))); // This will hold the coordinates of the vertices for one side.

    for (double i = 0.0; i <= dpanels; i++) // Calculate the vertex coordinates
    {
        int ii = (int)i;

        for (double j = 0.0; j <= dpanels; j++)
        {
            int jj = (int)j;

            double x = (i / dpanels) * 2.0 - 1.0;
            double y = (j / dpanels) * 2.0 - 1.0;
            double z = 1.0;

            // Now transform those coordinates onto a portion of a sphere

            face2vertices[ii][jj][0] = x * sqrt(1.0 - (y * y / 2.0) - (z * z / 2.0) + ((y * y) * (z * z) / 3.0));
            face2vertices[ii][jj][1] = y * sqrt(1.0 - (z * z / 2.0) - (x * x / 2.0) + ((z * z) * (x * x) / 3.0));
            face2vertices[ii][jj][2] = z * sqrt(1.0 - (x * x / 2.0) - (y * y / 2.0) + ((x * x) * (y * y) / 3.0));
        }
    }

    // Face 3 (left)

    vector<vector<vector<double>>> face3vertices(panels + 1, vector<vector<double>>(panels + 1, vector<double>(3))); // This will hold the coordinates of the vertices for one side.

    for (double i = 0.0; i <= dpanels; i++) // Calculate the vertex coordinates
    {
        int ii = (int)i;

        for (double j = 0.0; j <= dpanels; j++)
        {
            int jj = (int)j;

            double x = -1.0;
            double y = (j / dpanels) * 2.0 - 1.0;
            double z = (i / dpanels) * 2.0 - 1.0;

            // Now transform those coordinates onto a portion of a sphere

            face3vertices[ii][jj][0] = x * sqrt(1.0 - (y * y / 2.0) - (z * z / 2.0) + ((y * y) * (z * z) / 3.0));
            face3vertices[ii][jj][1] = y * sqrt(1.0 - (z * z / 2.0) - (x * x / 2.0) + ((z * z) * (x * x) / 3.0));
            face3vertices[ii][jj][2] = z * sqrt(1.0 - (x * x / 2.0) - (y * y / 2.0) + ((x * x) * (y * y) / 3.0));
        }
    }

    // Face 4 (top)

    vector<vector<vector<double>>> face4vertices(panels + 1, vector<vector<double>>(panels + 1, vector<double>(3))); // This will hold the coordinates of the vertices for one side.

    for (double i = 0.0; i <= dpanels; i++) // Calculate the vertex coordinates
    {
        int ii = (int)i;

        for (double j = 0.0; j <= dpanels; j++)
        {
            int jj = (int)j;

            double x = (i / dpanels) * 2.0 - 1.0;
            double y = 1.0;
            double z = (j / dpanels) * 2.0 - 1.0;

            // Now transform those coordinates onto a portion of a sphere

            face4vertices[ii][jj][0] = x * sqrt(1.0 - (y * y / 2.0) - (z * z / 2.0) + ((y * y) * (z * z) / 3.0));
            face4vertices[ii][jj][1] = y * sqrt(1.0 - (z * z / 2.0) - (x * x / 2.0) + ((z * z) * (x * x) / 3.0));
            face4vertices[ii][jj][2] = z * sqrt(1.0 - (x * x / 2.0) - (y * y / 2.0) + ((x * x) * (y * y) / 3.0));
        }
    }

    // Face 5 (bottom)

    vector<vector<vector<double>>> face5vertices(panels + 1, vector<vector<double>>(panels + 1, vector<double>(3))); // This will hold the coordinates of the vertices for one side.

    for (double i = 0.0; i <= dpanels; i++) // Calculate the vertex coordinates
    {
        int ii = (int)i;

        for (double j = 0.0; j <= dpanels; j++)
        {
            int jj = (int)j;

            double x = (i / dpanels) * 2.0 - 1.0;
            double y = -1.0;
            double z = (j / dpanels) * 2.0 - 1.0;

            // Now transform those coordinates onto a portion of a sphere

            face5vertices[ii][jj][0] = x * sqrt(1.0 - (y * y / 2.0) - (z * z / 2.0) + ((y * y) * (z * z) / 3.0));
            face5vertices[ii][jj][1] = y * sqrt(1.0 - (z * z / 2.0) - (x * x / 2.0) + ((z * z) * (x * x) / 3.0));
            face5vertices[ii][jj][2] = z * sqrt(1.0 - (x * x / 2.0) - (y * y / 2.0) + ((x * x) * (y * y) / 3.0));
        }
    }

    if (dsize != 1.0)
    {
        for (int i = 0; i <= panels; i++) // Change the size, if need be.
        {
            for (int j = 0; j <= panels; j++)
            {
                face0vertices[i][j][0] = face0vertices[i][j][0] * dsize;
                face0vertices[i][j][1] = face0vertices[i][j][1] * dsize;
                face0vertices[i][j][2] = face0vertices[i][j][2] * dsize;

                face1vertices[i][j][0] = face1vertices[i][j][0] * dsize;
                face1vertices[i][j][1] = face1vertices[i][j][1] * dsize;
                face1vertices[i][j][2] = face1vertices[i][j][2] * dsize;

                face2vertices[i][j][0] = face2vertices[i][j][0] * dsize;
                face2vertices[i][j][1] = face2vertices[i][j][1] * dsize;
                face2vertices[i][j][2] = face2vertices[i][j][2] * dsize;

                face3vertices[i][j][0] = face3vertices[i][j][0] * dsize;
                face3vertices[i][j][1] = face3vertices[i][j][1] * dsize;
                face3vertices[i][j][2] = face3vertices[i][j][2] * dsize;

                face4vertices[i][j][0] = face4vertices[i][j][0] * dsize;
                face4vertices[i][j][1] = face4vertices[i][j][1] * dsize;
                face4vertices[i][j][2] = face4vertices[i][j][2] * dsize;

                face5vertices[i][j][0] = face5vertices[i][j][0] * dsize;
                face5vertices[i][j][1] = face5vertices[i][j][1] * dsize;
                face5vertices[i][j][2] = face5vertices[i][j][2] * dsize;
            }
        }
    }

    // Now use that information to set up the vertices, normals, and texture coordinates of each panel of the sphere. Note: since this is a sphere centred on 0,0,0, the normal for each vertex simply *is* that vertex, normalised!

    double fraction = (1.0 / dpanels);

    // Side 0 (front)

    for (int i = 0; i < panels; i++)
    {
        int ii = panels - 1 - i;

        for (int j = 0; j < panels; j++)
        {
            int jj = panels - 1 - j;

            double x1 = face0vertices[ii][jj][0];
            double y1 = face0vertices[ii][jj][1];
            double z1 = face0vertices[ii][jj][2];

            double x2 = face0vertices[ii + 1][jj][0];
            double y2 = face0vertices[ii + 1][jj][1];
            double z2 = face0vertices[ii + 1][jj][2];

            double x3 = face0vertices[ii + 1][jj + 1][0];
            double y3 = face0vertices[ii + 1][jj + 1][1];
            double z3 = face0vertices[ii + 1][jj + 1][2];

            double x4 = face0vertices[ii][jj + 1][0];
            double y4 = face0vertices[ii][jj + 1][1];
            double z4 = face0vertices[ii][jj + 1][2];

            double s1 = dpanels - (double)jj * fraction;
            double t1 = (double)ii * fraction;

            double s2 = s1 - fraction;
            double t2 = t1;

            double s3 = s2;
            double t3 = t1 + fraction;

            double s4 = s1;
            double t4 = t3;

            t1 = 1.0 - t1;
            t2 = 1.0 - t2;
            t3 = 1.0 - t3;
            t4 = 1.0 - t4;

            Vector3 vertex1 = { (float)x1,(float)y1,(float)z1 };
            Vector3 vertex2 = { (float)x2,(float)y2,(float)z2 };
            Vector3 vertex3 = { (float)x3,(float)y3,(float)z3 };
            Vector3 vertex4 = { (float)x4,(float)y4,(float)z4 };

            Vector2 tex1 = { (float)t1,(float)s1 };
            Vector2 tex2 = { (float)t4,(float)s4 };
            Vector2 tex3 = { (float)t3,(float)s3 };
            Vector2 tex4 = { (float)t2,(float)s2 };

            Vector3 normal1 = vertex1;
            Vector3 normal2 = vertex2;
            Vector3 normal3 = vertex3;
            Vector3 normal4 = vertex4;

            float length = (float)sqrt(normal1.x * normal1.x + normal1.y * normal1.y + normal1.z * normal1.z);

            normal1.x = normal1.x / length;
            normal1.y = normal1.y / length;
            normal1.z = normal1.z / length;

            length = (float)sqrt(normal2.x * normal2.x + normal2.y * normal2.y + normal2.z * normal2.z);

            normal2.x = normal2.x / length;
            normal2.y = normal2.y / length;
            normal2.z = normal2.z / length;

            length = (float)sqrt(normal3.x * normal3.x + normal3.y * normal3.y + normal3.z * normal3.z);

            normal3.x = normal3.x / length;
            normal3.y = normal3.y / length;
            normal3.z = normal3.z / length;

            length = (float)sqrt(normal4.x * normal4.x + normal4.y * normal4.y + normal4.z * normal4.z);

            normal4.x = normal4.x / length;
            normal4.y = normal4.y / length;
            normal4.z = normal4.z / length;

            spherepanels[0][i][j] = LoadModelFromMesh(generatecustommesh(vertex1, vertex2, vertex3, vertex4, normal1, normal2, normal3, normal4, tex1, tex2, tex3, tex4));
        }
    }

    // Side 1 (right)

    for (int i = 0; i < panels; i++)
    {
        for (int j = 0; j < panels; j++)
        {
            int jj = panels - 1 - j;

            double x1 = face3vertices[i][jj][0];
            double y1 = face3vertices[i][jj][1];
            double z1 = face3vertices[i][jj][2];

            double x2 = face3vertices[i + 1][jj][0];
            double y2 = face3vertices[i + 1][jj][1];
            double z2 = face3vertices[i + 1][jj][2];

            double x3 = face3vertices[i + 1][jj + 1][0];
            double y3 = face3vertices[i + 1][jj + 1][1];
            double z3 = face3vertices[i + 1][jj + 1][2];

            double x4 = face3vertices[i][jj + 1][0];
            double y4 = face3vertices[i][jj + 1][1];
            double z4 = face3vertices[i][jj + 1][2];

            double s1 = dpanels - (double)jj * fraction;
            double t1 = dpanels - (double)i * fraction;

            double s2 = s1 - fraction;
            double t2 = t1;

            double s3 = s2;
            double t3 = t1 - fraction;

            double s4 = s1;
            double t4 = t3;

            t1 = 1.0 - t1;
            t2 = 1.0 - t2;
            t3 = 1.0 - t3;
            t4 = 1.0 - t4;

            Vector3 vertex1 = { (float)x1,(float)y1,(float)z1 };
            Vector3 vertex2 = { (float)x4,(float)y4,(float)z4 };
            Vector3 vertex3 = { (float)x3,(float)y3,(float)z3 };
            Vector3 vertex4 = { (float)x2,(float)y2,(float)z2 };

            Vector2 tex1 = { (float)t1,(float)s1 };
            Vector2 tex2 = { (float)t2,(float)s2 };
            Vector2 tex3 = { (float)t3,(float)s3 };
            Vector2 tex4 = { (float)t4,(float)s4 };

            Vector3 normal1 = vertex1;
            Vector3 normal2 = vertex2;
            Vector3 normal3 = vertex3;
            Vector3 normal4 = vertex4;

            float length = (float)sqrt(normal1.x * normal1.x + normal1.y * normal1.y + normal1.z * normal1.z);

            normal1.x = normal1.x / length;
            normal1.y = normal1.y / length;
            normal1.z = normal1.z / length;

            length = (float)sqrt(normal2.x * normal2.x + normal2.y * normal2.y + normal2.z * normal2.z);

            normal2.x = normal2.x / length;
            normal2.y = normal2.y / length;
            normal2.z = normal2.z / length;

            length = (float)sqrt(normal3.x * normal3.x + normal3.y * normal3.y + normal3.z * normal3.z);

            normal3.x = normal3.x / length;
            normal3.y = normal3.y / length;
            normal3.z = normal3.z / length;

            length = (float)sqrt(normal4.x * normal4.x + normal4.y * normal4.y + normal4.z * normal4.z);

            normal4.x = normal4.x / length;
            normal4.y = normal4.y / length;
            normal4.z = normal4.z / length;

            spherepanels[1][i][j] = LoadModelFromMesh(generatecustommesh(vertex1, vertex2, vertex3, vertex4, normal1, normal2, normal3, normal4, tex1, tex2, tex3, tex4));
        }
    }

    // Side 2 (back)

    for (int i = 0; i < panels; i++)
    {
        for (int j = 0; j < panels; j++)
        {
            int jj = panels - 1 - j;

            double x1 = face2vertices[i][jj][0];
            double y1 = face2vertices[i][jj][1];
            double z1 = face2vertices[i][jj][2];

            double x2 = face2vertices[i + 1][jj][0];
            double y2 = face2vertices[i + 1][jj][1];
            double z2 = face2vertices[i + 1][jj][2];

            double x3 = face2vertices[i + 1][jj + 1][0];
            double y3 = face2vertices[i + 1][jj + 1][1];
            double z3 = face2vertices[i + 1][jj + 1][2];

            double x4 = face2vertices[i][jj + 1][0];
            double y4 = face2vertices[i][jj + 1][1];
            double z4 = face2vertices[i][jj + 1][2];

            double s1 = dpanels - (double)jj * fraction;
            double t1 = dpanels - (double)i * fraction;

            double s2 = s1 - fraction;
            double t2 = t1;

            double s3 = s2;
            double t3 = t1 - fraction;

            double s4 = s1;
            double t4 = t3;

            t1 = 1.0 - t1;
            t2 = 1.0 - t2;
            t3 = 1.0 - t3;
            t4 = 1.0 - t4;

            Vector3 vertex1 = { (float)x1,(float)y1,(float)z1 };
            Vector3 vertex2 = { (float)x4,(float)y4,(float)z4 };
            Vector3 vertex3 = { (float)x3,(float)y3,(float)z3 };
            Vector3 vertex4 = { (float)x2,(float)y2,(float)z2 };

            Vector2 tex1 = { (float)t1,(float)s1 };
            Vector2 tex2 = { (float)t2,(float)s2 };
            Vector2 tex3 = { (float)t3,(float)s3 };
            Vector2 tex4 = { (float)t4,(float)s4 };

            Vector3 normal1 = vertex1;
            Vector3 normal2 = vertex2;
            Vector3 normal3 = vertex3;
            Vector3 normal4 = vertex4;

            float length = (float)sqrt(normal1.x * normal1.x + normal1.y * normal1.y + normal1.z * normal1.z);

            normal1.x = normal1.x / length;
            normal1.y = normal1.y / length;
            normal1.z = normal1.z / length;

            length = (float)sqrt(normal2.x * normal2.x + normal2.y * normal2.y + normal2.z * normal2.z);

            normal2.x = normal2.x / length;
            normal2.y = normal2.y / length;
            normal2.z = normal2.z / length;

            length = (float)sqrt(normal3.x * normal3.x + normal3.y * normal3.y + normal3.z * normal3.z);

            normal3.x = normal3.x / length;
            normal3.y = normal3.y / length;
            normal3.z = normal3.z / length;

            length = (float)sqrt(normal4.x * normal4.x + normal4.y * normal4.y + normal4.z * normal4.z);

            normal4.x = normal4.x / length;
            normal4.y = normal4.y / length;
            normal4.z = normal4.z / length;

            spherepanels[2][i][j] = LoadModelFromMesh(generatecustommesh(vertex1, vertex2, vertex3, vertex4, normal1, normal2, normal3, normal4, tex1, tex2, tex3, tex4));
        }
    }

    // Side 3 (left)

    for (int i = 0; i < panels; i++)
    {
        int ii = panels - 1 - i;

        for (int j = 0; j < panels; j++)
        {
            int jj = panels - 1 - j;

            double x1 = face1vertices[ii][jj][0];
            double y1 = face1vertices[ii][jj][1];
            double z1 = face1vertices[ii][jj][2];

            double x2 = face1vertices[ii + 1][jj][0];
            double y2 = face1vertices[ii + 1][jj][1];
            double z2 = face1vertices[ii + 1][jj][2];

            double x3 = face1vertices[ii + 1][jj + 1][0];
            double y3 = face1vertices[ii + 1][jj + 1][1];
            double z3 = face1vertices[ii + 1][jj + 1][2];

            double x4 = face1vertices[ii][jj + 1][0];
            double y4 = face1vertices[ii][jj + 1][1];
            double z4 = face1vertices[ii][jj + 1][2];

            double s1 = dpanels - (double)jj * fraction;
            double t1 = (double)ii * fraction;

            double s2 = s1 - fraction;
            double t2 = t1;

            double s3 = s2;
            double t3 = t1 + fraction;

            double s4 = s1;
            double t4 = t3;

            t1 = 1.0 - t1;
            t2 = 1.0 - t2;
            t3 = 1.0 - t3;
            t4 = 1.0 - t4;

            Vector3 vertex1 = { (float)x1,(float)y1,(float)z1 };
            Vector3 vertex2 = { (float)x2,(float)y2,(float)z2 };
            Vector3 vertex3 = { (float)x3,(float)y3,(float)z3 };
            Vector3 vertex4 = { (float)x4,(float)y4,(float)z4 };

            Vector2 tex1 = { (float)t1,(float)s1 };
            Vector2 tex2 = { (float)t4,(float)s4 };
            Vector2 tex3 = { (float)t3,(float)s3 };
            Vector2 tex4 = { (float)t2,(float)s2 };

            Vector3 normal1 = vertex1;
            Vector3 normal2 = vertex2;
            Vector3 normal3 = vertex3;
            Vector3 normal4 = vertex4;

            float length = (float)sqrt(normal1.x * normal1.x + normal1.y * normal1.y + normal1.z * normal1.z);

            normal1.x = normal1.x / length;
            normal1.y = normal1.y / length;
            normal1.z = normal1.z / length;

            length = (float)sqrt(normal2.x * normal2.x + normal2.y * normal2.y + normal2.z * normal2.z);

            normal2.x = normal2.x / length;
            normal2.y = normal2.y / length;
            normal2.z = normal2.z / length;

            length = (float)sqrt(normal3.x * normal3.x + normal3.y * normal3.y + normal3.z * normal3.z);

            normal3.x = normal3.x / length;
            normal3.y = normal3.y / length;
            normal3.z = normal3.z / length;

            length = (float)sqrt(normal4.x * normal4.x + normal4.y * normal4.y + normal4.z * normal4.z);

            normal4.x = normal4.x / length;
            normal4.y = normal4.y / length;
            normal4.z = normal4.z / length;

            spherepanels[3][i][j] = LoadModelFromMesh(generatecustommesh(vertex1, vertex2, vertex3, vertex4, normal1, normal2, normal3, normal4, tex1, tex2, tex3, tex4));
        }
    }

    // Side 4 (top)

    for (int i = 0; i < panels; i++)
    {
        int ii = panels - 1 - i;

        for (int j = 0; j < panels; j++)
        {
            int jj = panels - 1 - j;

            double x1 = face4vertices[ii][jj][0];
            double y1 = face4vertices[ii][jj][1];
            double z1 = face4vertices[ii][jj][2];

            double x2 = face4vertices[ii + 1][jj][0];
            double y2 = face4vertices[ii + 1][jj][1];
            double z2 = face4vertices[ii + 1][jj][2];

            double x3 = face4vertices[ii + 1][jj + 1][0];
            double y3 = face4vertices[ii + 1][jj + 1][1];
            double z3 = face4vertices[ii + 1][jj + 1][2];

            double x4 = face4vertices[ii][jj + 1][0];
            double y4 = face4vertices[ii][jj + 1][1];
            double z4 = face4vertices[ii][jj + 1][2];

            double s1 = dpanels - (double)jj * fraction;
            double t1 = (double)ii * fraction;

            double s2 = s1 - fraction;
            double t2 = t1;

            double s3 = s2;
            double t3 = t1 + fraction;

            double s4 = s1;
            double t4 = t3;

            t1 = 1.0 - t1;
            t2 = 1.0 - t2;
            t3 = 1.0 - t3;
            t4 = 1.0 - t4;

            Vector3 vertex1 = { (float)x1,(float)y1,(float)z1 };
            Vector3 vertex2 = { (float)x2,(float)y2,(float)z2 };
            Vector3 vertex3 = { (float)x3,(float)y3,(float)z3 };
            Vector3 vertex4 = { (float)x4,(float)y4,(float)z4 };

            Vector2 tex1 = { (float)t1,(float)s1 };
            Vector2 tex2 = { (float)t4,(float)s4 };
            Vector2 tex3 = { (float)t3,(float)s3 };
            Vector2 tex4 = { (float)t2,(float)s2 };

            Vector3 normal1 = vertex1;
            Vector3 normal2 = vertex2;
            Vector3 normal3 = vertex3;
            Vector3 normal4 = vertex4;

            float length = (float)sqrt(normal1.x * normal1.x + normal1.y * normal1.y + normal1.z * normal1.z);

            normal1.x = normal1.x / length;
            normal1.y = normal1.y / length;
            normal1.z = normal1.z / length;

            length = (float)sqrt(normal2.x * normal2.x + normal2.y * normal2.y + normal2.z * normal2.z);

            normal2.x = normal2.x / length;
            normal2.y = normal2.y / length;
            normal2.z = normal2.z / length;

            length = (float)sqrt(normal3.x * normal3.x + normal3.y * normal3.y + normal3.z * normal3.z);

            normal3.x = normal3.x / length;
            normal3.y = normal3.y / length;
            normal3.z = normal3.z / length;

            length = (float)sqrt(normal4.x * normal4.x + normal4.y * normal4.y + normal4.z * normal4.z);

            normal4.x = normal4.x / length;
            normal4.y = normal4.y / length;
            normal4.z = normal4.z / length;

            spherepanels[4][i][j] = LoadModelFromMesh(generatecustommesh(vertex1, vertex2, vertex3, vertex4, normal1, normal2, normal3, normal4, tex1, tex2, tex3, tex4));
        }
    }

    // Side 5 (bottom)

    for (int i = 0; i < panels; i++)
    {
        int ii = panels - 1 - i;

        for (int j = 0; j < panels; j++)
        {
            double x1 = face5vertices[ii][j][0];
            double y1 = face5vertices[ii][j][1];
            double z1 = face5vertices[ii][j][2];

            double x2 = face5vertices[ii + 1][j][0];
            double y2 = face5vertices[ii + 1][j][1];
            double z2 = face5vertices[ii + 1][j][2];

            double x3 = face5vertices[ii + 1][j + 1][0];
            double y3 = face5vertices[ii + 1][j + 1][1];
            double z3 = face5vertices[ii + 1][j + 1][2];

            double x4 = face5vertices[ii][j + 1][0];
            double y4 = face5vertices[ii][j + 1][1];
            double z4 = face5vertices[ii][j + 1][2];

            double s1 = (double)j * fraction;
            double t1 = (double)ii * fraction;

            double s2 = s1 + fraction;
            double t2 = t1;

            double s3 = s2;
            double t3 = t1 + fraction;

            double s4 = s1;
            double t4 = t3;

            t1 = 1.0 - t1;
            t2 = 1.0 - t2;
            t3 = 1.0 - t3;
            t4 = 1.0 - t4;

            Vector3 vertex1 = { (float)x1,(float)y1,(float)z1 };
            Vector3 vertex2 = { (float)x4,(float)y4,(float)z4 };
            Vector3 vertex3 = { (float)x3,(float)y3,(float)z3 };
            Vector3 vertex4 = { (float)x2,(float)y2,(float)z2 };

            Vector2 tex1 = { (float)t1,(float)s1 };
            Vector2 tex2 = { (float)t2,(float)s2 };
            Vector2 tex3 = { (float)t3,(float)s3 };
            Vector2 tex4 = { (float)t4,(float)s4 };

            Vector3 normal1 = vertex1;
            Vector3 normal2 = vertex2;
            Vector3 normal3 = vertex3;
            Vector3 normal4 = vertex4;

            float length = (float)sqrt(normal1.x * normal1.x + normal1.y * normal1.y + normal1.z * normal1.z);

            normal1.x = normal1.x / length;
            normal1.y = normal1.y / length;
            normal1.z = normal1.z / length;

            length = (float)sqrt(normal2.x * normal2.x + normal2.y * normal2.y + normal2.z * normal2.z);

            normal2.x = normal2.x / length;
            normal2.y = normal2.y / length;
            normal2.z = normal2.z / length;

            length = (float)sqrt(normal3.x * normal3.x + normal3.y * normal3.y + normal3.z * normal3.z);

            normal3.x = normal3.x / length;
            normal3.y = normal3.y / length;
            normal3.z = normal3.z / length;

            length = (float)sqrt(normal4.x * normal4.x + normal4.y * normal4.y + normal4.z * normal4.z);

            normal4.x = normal4.x / length;
            normal4.y = normal4.y / length;
            normal4.z = normal4.z / length;

            spherepanels[5][i][j] = LoadModelFromMesh(generatecustommesh(vertex1, vertex2, vertex3, vertex4, normal1, normal2, normal3, normal4, tex1, tex2, tex3, tex4));
        }
    }
}

// This does the same thing, but for the regional cubesphere.

void setupregionalspherepanels(vector<vector<vector<Model>>>& spherepanels, int panels, int worldsize, float size)
{
    // These variables are used to ensure that the right sections of the region textures are applied to the panels.
    // If worldsize == 2 then each panel corresponds to one region, but if the world is smaller then each region is spread over multiple panels.

    int moddiv = 1;
    double modmult = 1.0;

    if (worldsize == 1)
    {
        moddiv = 2;
        modmult = 0.5;
    }

    if (worldsize == 0)
    {
        moddiv = 4;
        modmult = 0.25;
    }

    // First, create six vectors of vertices to describe a cubesphere. Each vector holds the coordinates for one face.
    // The sphere transformations use formulae by Phil Nowell: http://mathproofs.blogspot.com/2005/07/mapping-cube-to-sphere.html

    double dpanels = (double)panels;
    double dsize = (double)size;

    // Face 0 (front)

    vector<vector<vector<double>>> face0vertices(panels + 1, vector<vector<double>>(panels + 1, vector<double>(3))); // This will hold the coordinates of the vertices for one side.

    for (double i = 0.0; i <= dpanels; i++) // Calculate the vertex coordinates
    {
        int ii = (int)i;

        for (double j = 0.0; j <= dpanels; j++)
        {
            int jj = (int)j;

            double x = (i / dpanels) * 2.0 - 1.0;
            double y = (j / dpanels) * 2.0 - 1.0;
            double z = -1.0;

            // Now transform those coordinates onto a portion of a sphere

            face0vertices[ii][jj][0] = x * sqrt(1.0 - (y * y / 2.0) - (z * z / 2.0) + ((y * y) * (z * z) / 3.0));
            face0vertices[ii][jj][1] = y * sqrt(1.0 - (z * z / 2.0) - (x * x / 2.0) + ((z * z) * (x * x) / 3.0));
            face0vertices[ii][jj][2] = z * sqrt(1.0 - (x * x / 2.0) - (y * y / 2.0) + ((x * x) * (y * y) / 3.0));
        }
    }

    // Face 1 (right)

    vector<vector<vector<double>>> face1vertices(panels + 1, vector<vector<double>>(panels + 1, vector<double>(3))); // This will hold the coordinates of the vertices for one side.

    for (double i = 0.0; i <= dpanels; i++) // Calculate the vertex coordinates
    {
        int ii = (int)i;

        for (double j = 0.0; j <= dpanels; j++)
        {
            int jj = (int)j;

            double x = 1.0;
            double y = (j / dpanels) * 2.0 - 1.0;
            double z = (i / dpanels) * 2.0 - 1.0;

            // Now transform those coordinates onto a portion of a sphere

            face1vertices[ii][jj][0] = x * sqrt(1.0 - (y * y / 2.0) - (z * z / 2.0) + ((y * y) * (z * z) / 3.0));
            face1vertices[ii][jj][1] = y * sqrt(1.0 - (z * z / 2.0) - (x * x / 2.0) + ((z * z) * (x * x) / 3.0));
            face1vertices[ii][jj][2] = z * sqrt(1.0 - (x * x / 2.0) - (y * y / 2.0) + ((x * x) * (y * y) / 3.0));
        }
    }

    // Face 2 (back)

    vector<vector<vector<double>>> face2vertices(panels + 1, vector<vector<double>>(panels + 1, vector<double>(3))); // This will hold the coordinates of the vertices for one side.

    for (double i = 0.0; i <= dpanels; i++) // Calculate the vertex coordinates
    {
        int ii = (int)i;

        for (double j = 0.0; j <= dpanels; j++)
        {
            int jj = (int)j;

            double x = (i / dpanels) * 2.0 - 1.0;
            double y = (j / dpanels) * 2.0 - 1.0;
            double z = 1.0;

            // Now transform those coordinates onto a portion of a sphere

            face2vertices[ii][jj][0] = x * sqrt(1.0 - (y * y / 2.0) - (z * z / 2.0) + ((y * y) * (z * z) / 3.0));
            face2vertices[ii][jj][1] = y * sqrt(1.0 - (z * z / 2.0) - (x * x / 2.0) + ((z * z) * (x * x) / 3.0));
            face2vertices[ii][jj][2] = z * sqrt(1.0 - (x * x / 2.0) - (y * y / 2.0) + ((x * x) * (y * y) / 3.0));
        }
    }

    // Face 3 (left)

    vector<vector<vector<double>>> face3vertices(panels + 1, vector<vector<double>>(panels + 1, vector<double>(3))); // This will hold the coordinates of the vertices for one side.

    for (double i = 0.0; i <= dpanels; i++) // Calculate the vertex coordinates
    {
        int ii = (int)i;

        for (double j = 0.0; j <= dpanels; j++)
        {
            int jj = (int)j;

            double x = -1.0;
            double y = (j / dpanels) * 2.0 - 1.0;
            double z = (i / dpanels) * 2.0 - 1.0;

            // Now transform those coordinates onto a portion of a sphere

            face3vertices[ii][jj][0] = x * sqrt(1.0 - (y * y / 2.0) - (z * z / 2.0) + ((y * y) * (z * z) / 3.0));
            face3vertices[ii][jj][1] = y * sqrt(1.0 - (z * z / 2.0) - (x * x / 2.0) + ((z * z) * (x * x) / 3.0));
            face3vertices[ii][jj][2] = z * sqrt(1.0 - (x * x / 2.0) - (y * y / 2.0) + ((x * x) * (y * y) / 3.0));
        }
    }

    // Face 4 (top)

    vector<vector<vector<double>>> face4vertices(panels + 1, vector<vector<double>>(panels + 1, vector<double>(3))); // This will hold the coordinates of the vertices for one side.

    for (double i = 0.0; i <= dpanels; i++) // Calculate the vertex coordinates
    {
        int ii = (int)i;

        for (double j = 0.0; j <= dpanels; j++)
        {
            int jj = (int)j;

            double x = (i / dpanels) * 2.0 - 1.0;
            double y = 1.0;
            double z = (j / dpanels) * 2.0 - 1.0;

            // Now transform those coordinates onto a portion of a sphere

            face4vertices[ii][jj][0] = x * sqrt(1.0 - (y * y / 2.0) - (z * z / 2.0) + ((y * y) * (z * z) / 3.0));
            face4vertices[ii][jj][1] = y * sqrt(1.0 - (z * z / 2.0) - (x * x / 2.0) + ((z * z) * (x * x) / 3.0));
            face4vertices[ii][jj][2] = z * sqrt(1.0 - (x * x / 2.0) - (y * y / 2.0) + ((x * x) * (y * y) / 3.0));
        }
    }

    // Face 5 (bottom)

    vector<vector<vector<double>>> face5vertices(panels + 1, vector<vector<double>>(panels + 1, vector<double>(3))); // This will hold the coordinates of the vertices for one side.

    for (double i = 0.0; i <= dpanels; i++) // Calculate the vertex coordinates
    {
        int ii = (int)i;

        for (double j = 0.0; j <= dpanels; j++)
        {
            int jj = (int)j;

            double x = (i / dpanels) * 2.0 - 1.0;
            double y = -1.0;
            double z = (j / dpanels) * 2.0 - 1.0;

            // Now transform those coordinates onto a portion of a sphere

            face5vertices[ii][jj][0] = x * sqrt(1.0 - (y * y / 2.0) - (z * z / 2.0) + ((y * y) * (z * z) / 3.0));
            face5vertices[ii][jj][1] = y * sqrt(1.0 - (z * z / 2.0) - (x * x / 2.0) + ((z * z) * (x * x) / 3.0));
            face5vertices[ii][jj][2] = z * sqrt(1.0 - (x * x / 2.0) - (y * y / 2.0) + ((x * x) * (y * y) / 3.0));
        }
    }

    if (dsize != 1.0)
    {
        for (int i = 0; i <= panels; i++) // Change the size, if need be.
        {
            for (int j = 0; j <= panels; j++)
            {
                face0vertices[i][j][0] = face0vertices[i][j][0] * dsize;
                face0vertices[i][j][1] = face0vertices[i][j][1] * dsize;
                face0vertices[i][j][2] = face0vertices[i][j][2] * dsize;

                face1vertices[i][j][0] = face1vertices[i][j][0] * dsize;
                face1vertices[i][j][1] = face1vertices[i][j][1] * dsize;
                face1vertices[i][j][2] = face1vertices[i][j][2] * dsize;

                face2vertices[i][j][0] = face2vertices[i][j][0] * dsize;
                face2vertices[i][j][1] = face2vertices[i][j][1] * dsize;
                face2vertices[i][j][2] = face2vertices[i][j][2] * dsize;

                face3vertices[i][j][0] = face3vertices[i][j][0] * dsize;
                face3vertices[i][j][1] = face3vertices[i][j][1] * dsize;
                face3vertices[i][j][2] = face3vertices[i][j][2] * dsize;

                face4vertices[i][j][0] = face4vertices[i][j][0] * dsize;
                face4vertices[i][j][1] = face4vertices[i][j][1] * dsize;
                face4vertices[i][j][2] = face4vertices[i][j][2] * dsize;

                face5vertices[i][j][0] = face5vertices[i][j][0] * dsize;
                face5vertices[i][j][1] = face5vertices[i][j][1] * dsize;
                face5vertices[i][j][2] = face5vertices[i][j][2] * dsize;
            }
        }
    }

    // Now use that information to set up the vertices, normals, and texture coordinates of each panel of the sphere. Note: since this is a sphere centred on 0,0,0, the normal for each vertex simply *is* that vertex, normalised!

    double fraction = (1.0 / dpanels);

    // Side 0 (front)

    for (int i = 0; i < panels; i++)
    {
        int ii = panels - 1 - i;

        for (int j = 0; j < panels; j++)
        {
            int jj = panels - 1 - j;

            double x1 = face0vertices[ii][jj][0];
            double y1 = face0vertices[ii][jj][1];
            double z1 = face0vertices[ii][jj][2];

            double x2 = face0vertices[ii + 1][jj][0];
            double y2 = face0vertices[ii + 1][jj][1];
            double z2 = face0vertices[ii + 1][jj][2];

            double x3 = face0vertices[ii + 1][jj + 1][0];
            double y3 = face0vertices[ii + 1][jj + 1][1];
            double z3 = face0vertices[ii + 1][jj + 1][2];

            double x4 = face0vertices[ii][jj + 1][0];
            double y4 = face0vertices[ii][jj + 1][1];
            double z4 = face0vertices[ii][jj + 1][2];

            double imod = i % moddiv;
            double jmod = j % moddiv;

            double t3 = imod * modmult;
            double t4 = t3;
            double t1 = t3 + modmult;
            double t2 = t1;

            double s2 = jmod * modmult;
            double s3 = s2;
            double s4 = s2 + modmult;
            double s1 = s4;

            Vector3 vertex1 = { (float)x1,(float)y1,(float)z1 };
            Vector3 vertex2 = { (float)x2,(float)y2,(float)z2 };
            Vector3 vertex3 = { (float)x3,(float)y3,(float)z3 };
            Vector3 vertex4 = { (float)x4,(float)y4,(float)z4 };

            Vector2 tex1 = { (float)t1,(float)s1 };
            Vector2 tex2 = { (float)t4,(float)s4 };
            Vector2 tex3 = { (float)t3,(float)s3 };
            Vector2 tex4 = { (float)t2,(float)s2 };

            Vector3 normal1 = vertex1;
            Vector3 normal2 = vertex2;
            Vector3 normal3 = vertex3;
            Vector3 normal4 = vertex4;

            float length = (float)sqrt(normal1.x * normal1.x + normal1.y * normal1.y + normal1.z * normal1.z);

            normal1.x = normal1.x / length;
            normal1.y = normal1.y / length;
            normal1.z = normal1.z / length;

            length = (float)sqrt(normal2.x * normal2.x + normal2.y * normal2.y + normal2.z * normal2.z);

            normal2.x = normal2.x / length;
            normal2.y = normal2.y / length;
            normal2.z = normal2.z / length;

            length = (float)sqrt(normal3.x * normal3.x + normal3.y * normal3.y + normal3.z * normal3.z);

            normal3.x = normal3.x / length;
            normal3.y = normal3.y / length;
            normal3.z = normal3.z / length;

            length = (float)sqrt(normal4.x * normal4.x + normal4.y * normal4.y + normal4.z * normal4.z);

            normal4.x = normal4.x / length;
            normal4.y = normal4.y / length;
            normal4.z = normal4.z / length;

            spherepanels[0][i][j] = LoadModelFromMesh(generatecustommesh(vertex1, vertex2, vertex3, vertex4, normal1, normal2, normal3, normal4, tex1, tex2, tex3, tex4));
        }
    }

    // Side 1 (right)

    for (int i = 0; i < panels; i++)
    {
        for (int j = 0; j < panels; j++)
        {
            int jj = panels - 1 - j;

            double x1 = face3vertices[i][jj][0];
            double y1 = face3vertices[i][jj][1];
            double z1 = face3vertices[i][jj][2];

            double x2 = face3vertices[i + 1][jj][0];
            double y2 = face3vertices[i + 1][jj][1];
            double z2 = face3vertices[i + 1][jj][2];

            double x3 = face3vertices[i + 1][jj + 1][0];
            double y3 = face3vertices[i + 1][jj + 1][1];
            double z3 = face3vertices[i + 1][jj + 1][2];

            double x4 = face3vertices[i][jj + 1][0];
            double y4 = face3vertices[i][jj + 1][1];
            double z4 = face3vertices[i][jj + 1][2];

            double imod = i % moddiv;
            double jmod = j % moddiv;

            double t1 = imod * modmult;
            double t2 = t1;
            double t3 = t1 + modmult;
            double t4 = t3;

            double s2 = jmod * modmult;
            double s3 = s2;
            double s4 = s2 + modmult;
            double s1 = s4;

            Vector3 vertex1 = { (float)x1,(float)y1,(float)z1 };
            Vector3 vertex2 = { (float)x4,(float)y4,(float)z4 };
            Vector3 vertex3 = { (float)x3,(float)y3,(float)z3 };
            Vector3 vertex4 = { (float)x2,(float)y2,(float)z2 };

            Vector2 tex1 = { (float)t1,(float)s1 };
            Vector2 tex2 = { (float)t2,(float)s2 };
            Vector2 tex3 = { (float)t3,(float)s3 };
            Vector2 tex4 = { (float)t4,(float)s4 };

            Vector3 normal1 = vertex1;
            Vector3 normal2 = vertex2;
            Vector3 normal3 = vertex3;
            Vector3 normal4 = vertex4;

            float length = (float)sqrt(normal1.x * normal1.x + normal1.y * normal1.y + normal1.z * normal1.z);

            normal1.x = normal1.x / length;
            normal1.y = normal1.y / length;
            normal1.z = normal1.z / length;

            length = (float)sqrt(normal2.x * normal2.x + normal2.y * normal2.y + normal2.z * normal2.z);

            normal2.x = normal2.x / length;
            normal2.y = normal2.y / length;
            normal2.z = normal2.z / length;

            length = (float)sqrt(normal3.x * normal3.x + normal3.y * normal3.y + normal3.z * normal3.z);

            normal3.x = normal3.x / length;
            normal3.y = normal3.y / length;
            normal3.z = normal3.z / length;

            length = (float)sqrt(normal4.x * normal4.x + normal4.y * normal4.y + normal4.z * normal4.z);

            normal4.x = normal4.x / length;
            normal4.y = normal4.y / length;
            normal4.z = normal4.z / length;

            spherepanels[1][i][j] = LoadModelFromMesh(generatecustommesh(vertex1, vertex2, vertex3, vertex4, normal1, normal2, normal3, normal4, tex1, tex2, tex3, tex4));
        }
    }

    // Side 2 (back)

    for (int i = 0; i < panels; i++)
    {
        for (int j = 0; j < panels; j++)
        {
            int jj = panels - 1 - j;

            double x1 = face2vertices[i][jj][0];
            double y1 = face2vertices[i][jj][1];
            double z1 = face2vertices[i][jj][2];

            double x2 = face2vertices[i + 1][jj][0];
            double y2 = face2vertices[i + 1][jj][1];
            double z2 = face2vertices[i + 1][jj][2];

            double x3 = face2vertices[i + 1][jj + 1][0];
            double y3 = face2vertices[i + 1][jj + 1][1];
            double z3 = face2vertices[i + 1][jj + 1][2];

            double x4 = face2vertices[i][jj + 1][0];
            double y4 = face2vertices[i][jj + 1][1];
            double z4 = face2vertices[i][jj + 1][2];

            double imod = i % moddiv;
            double jmod = j % moddiv;

            double t1 = imod * modmult;
            double t2 = t1;
            double t3 = t1 + modmult;
            double t4 = t3;

            double s2 = jmod * modmult;
            double s3 = s2;
            double s4 = s2 + modmult;
            double s1 = s4;

            Vector3 vertex1 = { (float)x1,(float)y1,(float)z1 };
            Vector3 vertex2 = { (float)x4,(float)y4,(float)z4 };
            Vector3 vertex3 = { (float)x3,(float)y3,(float)z3 };
            Vector3 vertex4 = { (float)x2,(float)y2,(float)z2 };

            Vector2 tex1 = { (float)t1,(float)s1 };
            Vector2 tex2 = { (float)t2,(float)s2 };
            Vector2 tex3 = { (float)t3,(float)s3 };
            Vector2 tex4 = { (float)t4,(float)s4 };

            Vector3 normal1 = vertex1;
            Vector3 normal2 = vertex2;
            Vector3 normal3 = vertex3;
            Vector3 normal4 = vertex4;

            float length = (float)sqrt(normal1.x * normal1.x + normal1.y * normal1.y + normal1.z * normal1.z);

            normal1.x = normal1.x / length;
            normal1.y = normal1.y / length;
            normal1.z = normal1.z / length;

            length = (float)sqrt(normal2.x * normal2.x + normal2.y * normal2.y + normal2.z * normal2.z);

            normal2.x = normal2.x / length;
            normal2.y = normal2.y / length;
            normal2.z = normal2.z / length;

            length = (float)sqrt(normal3.x * normal3.x + normal3.y * normal3.y + normal3.z * normal3.z);

            normal3.x = normal3.x / length;
            normal3.y = normal3.y / length;
            normal3.z = normal3.z / length;

            length = (float)sqrt(normal4.x * normal4.x + normal4.y * normal4.y + normal4.z * normal4.z);

            normal4.x = normal4.x / length;
            normal4.y = normal4.y / length;
            normal4.z = normal4.z / length;

            spherepanels[2][i][j] = LoadModelFromMesh(generatecustommesh(vertex1, vertex2, vertex3, vertex4, normal1, normal2, normal3, normal4, tex1, tex2, tex3, tex4));
        }
    }

    // Side 3 (left)

    for (int i = 0; i < panels; i++)
    {
        int ii = panels - 1 - i;

        for (int j = 0; j < panels; j++)
        {
            int jj = panels - 1 - j;

            double x1 = face1vertices[ii][jj][0];
            double y1 = face1vertices[ii][jj][1];
            double z1 = face1vertices[ii][jj][2];

            double x2 = face1vertices[ii + 1][jj][0];
            double y2 = face1vertices[ii + 1][jj][1];
            double z2 = face1vertices[ii + 1][jj][2];

            double x3 = face1vertices[ii + 1][jj + 1][0];
            double y3 = face1vertices[ii + 1][jj + 1][1];
            double z3 = face1vertices[ii + 1][jj + 1][2];

            double x4 = face1vertices[ii][jj + 1][0];
            double y4 = face1vertices[ii][jj + 1][1];
            double z4 = face1vertices[ii][jj + 1][2];

            double imod = i % moddiv;
            double jmod = j % moddiv;

            double t3 = imod * modmult;
            double t4 = t3;
            double t1 = t3 + modmult;
            double t2 = t1;

            double s2 = jmod * modmult;
            double s3 = s2;
            double s4 = s2 + modmult;
            double s1 = s4;

            Vector3 vertex1 = { (float)x1,(float)y1,(float)z1 };
            Vector3 vertex2 = { (float)x2,(float)y2,(float)z2 };
            Vector3 vertex3 = { (float)x3,(float)y3,(float)z3 };
            Vector3 vertex4 = { (float)x4,(float)y4,(float)z4 };

            Vector2 tex1 = { (float)t1,(float)s1 };
            Vector2 tex2 = { (float)t4,(float)s4 };
            Vector2 tex3 = { (float)t3,(float)s3 };
            Vector2 tex4 = { (float)t2,(float)s2 };

            Vector3 normal1 = vertex1;
            Vector3 normal2 = vertex2;
            Vector3 normal3 = vertex3;
            Vector3 normal4 = vertex4;

            float length = (float)sqrt(normal1.x * normal1.x + normal1.y * normal1.y + normal1.z * normal1.z);

            normal1.x = normal1.x / length;
            normal1.y = normal1.y / length;
            normal1.z = normal1.z / length;

            length = (float)sqrt(normal2.x * normal2.x + normal2.y * normal2.y + normal2.z * normal2.z);

            normal2.x = normal2.x / length;
            normal2.y = normal2.y / length;
            normal2.z = normal2.z / length;

            length = (float)sqrt(normal3.x * normal3.x + normal3.y * normal3.y + normal3.z * normal3.z);

            normal3.x = normal3.x / length;
            normal3.y = normal3.y / length;
            normal3.z = normal3.z / length;

            length = (float)sqrt(normal4.x * normal4.x + normal4.y * normal4.y + normal4.z * normal4.z);

            normal4.x = normal4.x / length;
            normal4.y = normal4.y / length;
            normal4.z = normal4.z / length;

            spherepanels[3][i][j] = LoadModelFromMesh(generatecustommesh(vertex1, vertex2, vertex3, vertex4, normal1, normal2, normal3, normal4, tex1, tex2, tex3, tex4));
        }
    }

    // Side 4 (top)

    for (int i = 0; i < panels; i++)
    {
        int ii = panels - 1 - i;

        for (int j = 0; j < panels; j++)
        {
            int jj = panels - 1 - j;

            double x1 = face4vertices[ii][jj][0];
            double y1 = face4vertices[ii][jj][1];
            double z1 = face4vertices[ii][jj][2];

            double x2 = face4vertices[ii + 1][jj][0];
            double y2 = face4vertices[ii + 1][jj][1];
            double z2 = face4vertices[ii + 1][jj][2];

            double x3 = face4vertices[ii + 1][jj + 1][0];
            double y3 = face4vertices[ii + 1][jj + 1][1];
            double z3 = face4vertices[ii + 1][jj + 1][2];

            double x4 = face4vertices[ii][jj + 1][0];
            double y4 = face4vertices[ii][jj + 1][1];
            double z4 = face4vertices[ii][jj + 1][2];

            double imod = i % moddiv;
            double jmod = j % moddiv;

            double t3 = imod * modmult;
            double t4 = t3;
            double t1 = t3 + modmult;
            double t2 = t1;

            double s2 = jmod * modmult;
            double s3 = s2;
            double s4 = s2 + modmult;
            double s1 = s4;

            Vector3 vertex1 = { (float)x1,(float)y1,(float)z1 };
            Vector3 vertex2 = { (float)x2,(float)y2,(float)z2 };
            Vector3 vertex3 = { (float)x3,(float)y3,(float)z3 };
            Vector3 vertex4 = { (float)x4,(float)y4,(float)z4 };

            Vector2 tex1 = { (float)t1,(float)s1 };
            Vector2 tex2 = { (float)t4,(float)s4 };
            Vector2 tex3 = { (float)t3,(float)s3 };
            Vector2 tex4 = { (float)t2,(float)s2 };

            Vector3 normal1 = vertex1;
            Vector3 normal2 = vertex2;
            Vector3 normal3 = vertex3;
            Vector3 normal4 = vertex4;

            float length = (float)sqrt(normal1.x * normal1.x + normal1.y * normal1.y + normal1.z * normal1.z);

            normal1.x = normal1.x / length;
            normal1.y = normal1.y / length;
            normal1.z = normal1.z / length;

            length = (float)sqrt(normal2.x * normal2.x + normal2.y * normal2.y + normal2.z * normal2.z);

            normal2.x = normal2.x / length;
            normal2.y = normal2.y / length;
            normal2.z = normal2.z / length;

            length = (float)sqrt(normal3.x * normal3.x + normal3.y * normal3.y + normal3.z * normal3.z);

            normal3.x = normal3.x / length;
            normal3.y = normal3.y / length;
            normal3.z = normal3.z / length;

            length = (float)sqrt(normal4.x * normal4.x + normal4.y * normal4.y + normal4.z * normal4.z);

            normal4.x = normal4.x / length;
            normal4.y = normal4.y / length;
            normal4.z = normal4.z / length;

            spherepanels[4][i][j] = LoadModelFromMesh(generatecustommesh(vertex1, vertex2, vertex3, vertex4, normal1, normal2, normal3, normal4, tex1, tex2, tex3, tex4));
        }
    }

    // Side 5 (bottom)

    for (int i = 0; i < panels; i++)
    {
        int ii = panels - 1 - i;

        for (int j = 0; j < panels; j++)
        {
            double x1 = face5vertices[ii][j][0];
            double y1 = face5vertices[ii][j][1];
            double z1 = face5vertices[ii][j][2];

            double x2 = face5vertices[ii + 1][j][0];
            double y2 = face5vertices[ii + 1][j][1];
            double z2 = face5vertices[ii + 1][j][2];

            double x3 = face5vertices[ii + 1][j + 1][0];
            double y3 = face5vertices[ii + 1][j + 1][1];
            double z3 = face5vertices[ii + 1][j + 1][2];

            double x4 = face5vertices[ii][j + 1][0];
            double y4 = face5vertices[ii][j + 1][1];
            double z4 = face5vertices[ii][j + 1][2];

            double imod = i % moddiv;
            double jmod = j % moddiv;

            double t3 = imod * modmult;
            double t4 = t3;
            double t1 = t3 + modmult;
            double t2 = t1;

            double s4 = jmod * modmult;
            double s1 = s4;
            double s2 = s4 + modmult;
            double s3 = s2;

            Vector3 vertex1 = { (float)x1,(float)y1,(float)z1 };
            Vector3 vertex2 = { (float)x4,(float)y4,(float)z4 };
            Vector3 vertex3 = { (float)x3,(float)y3,(float)z3 };
            Vector3 vertex4 = { (float)x2,(float)y2,(float)z2 };

            Vector2 tex1 = { (float)t1,(float)s1 };
            Vector2 tex2 = { (float)t2,(float)s2 };
            Vector2 tex3 = { (float)t3,(float)s3 };
            Vector2 tex4 = { (float)t4,(float)s4 };

            Vector3 normal1 = vertex1;
            Vector3 normal2 = vertex2;
            Vector3 normal3 = vertex3;
            Vector3 normal4 = vertex4;

            float length = (float)sqrt(normal1.x * normal1.x + normal1.y * normal1.y + normal1.z * normal1.z);

            normal1.x = normal1.x / length;
            normal1.y = normal1.y / length;
            normal1.z = normal1.z / length;

            length = (float)sqrt(normal2.x * normal2.x + normal2.y * normal2.y + normal2.z * normal2.z);

            normal2.x = normal2.x / length;
            normal2.y = normal2.y / length;
            normal2.z = normal2.z / length;

            length = (float)sqrt(normal3.x * normal3.x + normal3.y * normal3.y + normal3.z * normal3.z);

            normal3.x = normal3.x / length;
            normal3.y = normal3.y / length;
            normal3.z = normal3.z / length;

            length = (float)sqrt(normal4.x * normal4.x + normal4.y * normal4.y + normal4.z * normal4.z);

            normal4.x = normal4.x / length;
            normal4.y = normal4.y / length;
            normal4.z = normal4.z / length;

            spherepanels[5][i][j] = LoadModelFromMesh(generatecustommesh(vertex1, vertex2, vertex3, vertex4, normal1, normal2, normal3, normal4, tex1, tex2, tex3, tex4));
        }
    }
}

// This looks up the latest version of the program. Based on code by Parveen: https://hoven.in/cpp-network/c-program-download-file-from-url.html

float getlatestversion()
{
    /*
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

    stream->Release();

    float val = stof(s);
    */

    return 0.0f; // val;
}

// This prints an update to the screen. (Only used when the console is shown.)

void updatereport(int& progressval, string& progresstext, string text)
{
    progressval++;
    progresstext = text;
    //cout << text << '\n';
}

// This makes a button in a standard size and indentation.

bool standardbutton(const char* label)
{
    ImGui::SetCursorPosX(20);

    return ImGui::Button(label, ImVec2(120.0f, 30.0f));
}

// This applies a set of images to a set of textures.

void updatetextures(vector<Image>& images, vector<Texture2D>& textures)
{
    for (int n = 0; n < 6; n++)
        textures[n] = LoadTextureFromImage(images[n]);
}

// This does the same thing, but for textures that have monthly versions.

void updatetextures(vector<Image>& images, vector<Texture2D>& textures, int month)
{
    for (int n = 0; n < 6; n++)
    {
        textures[n] = LoadTextureFromImage(images[n * 12 + month]);
    }
}

// This applies the correct shader to the globe.

void updateshader(vector<vector<vector<Model>>>& spherepanels, Shader shader, int panels)
{
    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < panels; i++)
        {
            for (int j = 0; j < panels; j++)
                spherepanels[face][i][j].materials[0].shader = shader;
        }
    }
}

// This applies specular textures to the globe.

void applyspeculartextures(vector<vector<vector<Model>>>& spherepanels, vector<Texture2D>& speculartexture, int panels)
{
    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < panels; i++)
        {
            for (int j = 0; j < panels; j++)
                spherepanels[face][i][j].materials[0].maps[MATERIAL_MAP_SPECULAR].texture = speculartexture[face];
        }
    }
}

// This applies normal textures to the globe.

void applynormaltextures(vector<vector<vector<Model>>>& spherepanels, vector<Texture2D>& normaltexture, int panels)
{
    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < panels; i++)
        {
            for (int j = 0; j < panels; j++)
                spherepanels[face][i][j].materials[0].maps[MATERIAL_MAP_NORMAL].texture = normaltexture[face];
        }
    }
}

// These apply old versions of these maps, to create smooth transitions between maps.

void applyoldspacetextures(vector<vector<vector<Model>>>& spherepanels, vector<Texture2D>& spacetextureold, int panels)
{
    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < panels; i++)
        {
            for (int j = 0; j < panels; j++)
                spherepanels[face][i][j].materials[0].maps[MATERIAL_MAP_OCCLUSION].texture = spacetextureold[face];
        }
    }
}

void applyoldspeculartextures(vector<vector<vector<Model>>>& spherepanels, vector<Texture2D>& speculartextureold, int panels)
{
    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < panels; i++)
        {
            for (int j = 0; j < panels; j++)
                spherepanels[face][i][j].materials[0].maps[MATERIAL_MAP_EMISSION].texture = speculartextureold[face];
        }
    }
}

// This applies haze and nebulae maps to the starfield object.

void applystarhazetextures(vector<vector<vector<Model>>>& skypanels, vector<Texture2D>& starhazetexture, int spanels)
{
    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < spanels; i++)
        {
            for (int j = 0; j < spanels; j++)
                skypanels[face][i][j].materials[0].maps[MATERIAL_MAP_OCCLUSION].texture = starhazetexture[face];
        }
    }
}

// This draws the globe.

void drawglobe(vector<vector<vector<Model>>>& spherepanels, vector<vector<vector<Model>>>& regionspherepanels, int worldsize, vector<int>& regionloc, vector<Texture2D>& globaltexture, vector<Texture2D>& regionaltexture, int panels, Vector3 globepos, float globerotate, float globetilt, float globepitch, bool draw[6], vector<short>& regioncreated)
{
    int sizediv = 1;

    if (worldsize == 1)
        sizediv = 2;

    if (worldsize == 0)
        sizediv = 4;

    for (int face = 0; face < 6; face++)
    {
        if (1 == 1) //(draw[face])
        {
            for (int i = 0; i < panels; i++)
            {
                int ii = i / sizediv;

                for (int j = 0; j < panels; j++)
                {
                    int jj = j / sizediv;

                    int thisregion = regionloc[face * panels * panels + ii * panels + jj];

                    if (thisregion != -1 && regioncreated[thisregion] == 1)
                    {
                        regionspherepanels[face][i][j].transform = MatrixRotateXYZ(Vector3{ DEG2RAD * globetilt, DEG2RAD * globerotate, DEG2RAD * globepitch });

                        regionspherepanels[face][i][j].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = regionaltexture[thisregion];

                        DrawModel(regionspherepanels[face][i][j], globepos, 1.0f, WHITE);
                    }
                    else
                    {
                        spherepanels[face][i][j].transform = MatrixRotateXYZ(Vector3{ DEG2RAD * globetilt, DEG2RAD * globerotate, DEG2RAD * globepitch });

                        spherepanels[face][i][j].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = globaltexture[face];

                        DrawModel(spherepanels[face][i][j], globepos, 1.0f, WHITE);
                    }

                    /*
                    if (face == 0)
                        DrawModel(spherepanels[face][i][j], Vector3{ 0.0f,0.0f,0.0f }, 1.0f, DARKBLUE);

                    if (face == 1)
                        DrawModel(spherepanels[face][i][j], Vector3{ 0.0f,0.0f,0.0f }, 1.0f, RED);

                    if (face == 2)
                        DrawModel(spherepanels[face][i][j], Vector3{ 0.0f,0.0f,0.0f }, 1.0f, GREEN);

                    if (face == 3)
                        DrawModel(spherepanels[face][i][j], Vector3{ 0.0f,0.0f,0.0f }, 1.0f, YELLOW);

                    if (face == 4)
                        DrawModel(spherepanels[face][i][j], Vector3{ 0.0f,0.0f,0.0f }, 1.0f, PURPLE);

                    if (face == 5)
                        DrawModel(spherepanels[face][i][j], Vector3{ 0.0f,0.0f,0.0f }, 1.0f, MAGENTA);
                    */
                }
            }
        }
    }
}

// This draws the globe without worrying about regions.

void drawbasicglobe(vector<vector<vector<Model>>>& spherepanels, vector<Texture2D>& globaltexture, int panels, Vector3 globepos, float globerotate, float globetilt, float globepitch, bool draw[6])
{
    for (int face = 0; face < 6; face++)
    {
        if (draw[face])
        {
            for (int i = 0; i < panels; i++)
            {
                for (int j = 0; j < panels; j++)
                {
                    spherepanels[face][i][j].transform = MatrixRotateXYZ(Vector3{ DEG2RAD * globetilt, DEG2RAD * globerotate, DEG2RAD * globepitch });

                    spherepanels[face][i][j].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = globaltexture[face];

                    DrawModel(spherepanels[face][i][j], globepos, 1.0f, WHITE);

                    /*
                    if (face == 0)
                        DrawModel(spherepanels[face][i][j], Vector3{ 0.0f,0.0f,0.0f }, 1.0f, DARKBLUE);

                    if (face == 1)
                        DrawModel(spherepanels[face][i][j], Vector3{ 0.0f,0.0f,0.0f }, 1.0f, RED);

                    if (face == 2)
                        DrawModel(spherepanels[face][i][j], Vector3{ 0.0f,0.0f,0.0f }, 1.0f, GREEN);

                    if (face == 3)
                        DrawModel(spherepanels[face][i][j], Vector3{ 0.0f,0.0f,0.0f }, 1.0f, YELLOW);

                    if (face == 4)
                        DrawModel(spherepanels[face][i][j], Vector3{ 0.0f,0.0f,0.0f }, 1.0f, PURPLE);

                    if (face == 5)
                        DrawModel(spherepanels[face][i][j], Vector3{ 0.0f,0.0f,0.0f }, 1.0f, MAGENTA);
                    */
                }
            }
        }
    }
}

// This creates a set of textures for the globe where every texel is a unique colour. We'll use these to work out which point has been clicked on by the user.

void drawglobalpickingimage(planet& world, int globaltexturesize, vector<Image>& globalpickingimage)
{
    int edge = world.edge();

    int r = 0;
    int g = 0;
    int b = 1; // Don't start with 0,0,0 as that will be the background!

    int id = 0; // ID of the current texel.

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < globaltexturesize; i++)
        {
            for (int j = 0; j < globaltexturesize; j++)
            {
                Color colour{ (unsigned char)r, (unsigned char)g, (unsigned char)b, 255 };

                ImageDrawPixel(&globalpickingimage[face], i, j, colour);

                // Now advance to the next colour value.

                b++;

                if (b > 255)
                {
                    b = 0;

                    g++;

                    if (g > 255)
                    {
                        g = 0;

                        r++;
                    }
                }
            }
        }
    }
}

// This function gets colours for drawing climate maps.

Color getclimatecolours(short climate)
{
    unsigned char colour1 = 0;
    unsigned char colour2 = 0;
    unsigned char colour3 = 0;

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

    Color colour{ colour1, colour2, colour3, 255 };

    return (colour);
}

// This function creates the starfield.

void drawstarfield(planet& world, vector<Image>& starsimage, vector<Image>& starhazeimage, int skytexturesize, float& skyrotate, float& skytilt, float& skypitch)
{
    long seed = world.seed();
    fast_srand(seed);

    int maxelev = world.maxelevation();

    int halfmaxelev = maxelev / 2;

    skyrotate = (float)(random(0, 360));
    skytilt = (float)(random(0, 360));
    skypitch = (float)(random(0, 360));

    float density = (float)random(8, 25) * 0.001f; // 0.005f; // 0.02f; // The higher this is, the more stars there will be.
    float minstarchance = 0.01f;

    float fmaxelev = (float)maxelev;
    int edge = 512;
    int eedge = edge * edge;

    float div = (float)edge / (float)skytexturesize;

    // First, make a fractal to determine star density.

    vector<int>fractal(6 * eedge, 0);

    int grain;
    float valuemod, valuemod2;

    bool hazepresent = 1;

    if (random(1, 4) == 1)
        hazepresent = 0;

    bool nebulaepresent = 0;

    if (random(1, 3) == 1)
        nebulaepresent = 1;

    if (hazepresent)
    {
        if (random(1, 4) != 1)
        {
            // Milky Way style

            int grain = (int)pow(2, random(3, 4)); // Level of detail on this fractal map.
            float valuemod = 0.2f;
            float valuemod2 = 3.0f;

            int min = 1;
            int max = maxelev / random(5, 20);
            int milkywaymin = maxelev / 2;
            int milkywaymax = maxelev;

            createmilkywayfractal(fractal, edge, grain, valuemod, valuemod2, min, max, milkywaymin, milkywaymax, 0, 0); // (maxelev / 8) * 7
        }
        else
        {
            // All over style

            grain = (int)pow(2, random(1, 4)); // Level of detail on this fractal map.
            valuemod = 0.2f;
            valuemod2 = 3.0f;

            createfractal(fractal, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

            if (random(1, 4) != 1) // Alter it with some additional fractals.
            {
                int iterations = random(1, 4);

                for (int n = 0; n < iterations; n++)
                {
                    vector<int>newfractal(6 * eedge, 0);

                    grain = (int)pow(2, random(2, 5)); // Level of detail on this fractal map.
                    valuemod = 0.2f;
                    valuemod2 = 3.0f;

                    createfractal(newfractal, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

                    for (int face = 0; face < 6; face++)
                    {
                        int vface = face * eedge;

                        for (int i = 0; i < edge; i++)
                        {
                            int vi = vface + i * edge;

                            for (int j = 0; j < edge; j++)
                            {
                                int index = vi + j;

                                if (fractal[index] > newfractal[index])
                                    fractal[index] = newfractal[index];
                            }
                        }
                    }
                }
            }
        }

        int remove = maxelev / 20; // Remove this amount from the whole fractal array, to ensure large areas of true darkness.

        for (int face = 0; face < 6; face++)
        {
            int vface = face * eedge;

            for (int i = 0; i < edge; i++)
            {
                int vi = vface + i * edge;

                for (int j = 0; j < edge; j++)
                {
                    int index = vi + j;

                    fractal[index] = fractal[index] - remove;

                    if (fractal[index] < 0)
                        fractal[index] = 0;
                }
            }
        }
    }

    // Now make a fractal to vary the colour of the haze.

    vector<int>hazecolourfractal(6 * eedge);

    if (hazepresent)
    {
        grain = 8; // Level of detail on this fractal map.
        valuemod = 0.2f;
        valuemod2 = 3.0f;

        createfractal(hazecolourfractal, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);
    }

    // Now we do nebulae.

    vector<int>nebulafractal(6 * eedge, 0);

    if (nebulaepresent)
    {
        grain = (int)pow(2, random(3, 4)); // Level of detail on this fractal map.
        valuemod = 0.2f;
        valuemod2 = 3.0f;

        createfractal(nebulafractal, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

        if (random(1, 4) != 1)
        {
            vector<int>bubbles(6 * eedge, 0);

            int moundno = random(40000, 120000);

            makemounds(edge, bubbles, moundno, fmaxelev);

            vector<int>bubbles2(6 * eedge, 0);

            moundno = random(40000, 120000);

            if (random(1, 2) == 1)
                makemounds(edge, bubbles2, moundno, fmaxelev);

            for (int face = 0; face < 6; face++) // Combine with the bubbles.
            {
                int vface = face * eedge;

                for (int i = 0; i < edge; i++)
                {
                    int vi = vface + i * edge;

                    for (int j = 0; j < edge; j++)
                    {
                        int index = vi + j;

                        int thisbubble = maxelev - bubbles[index];
                        int thisbubble2 = maxelev - bubbles2[index];


                        if (thisbubble < thisbubble2)
                            thisbubble = thisbubble2;

                        //float bub = (float)thisbubble / fmaxelev;

                        //bub = bub * bub;

                        //thisbubble = (int)(bub * fmaxelev);

                        if (nebulafractal[index] > thisbubble)
                            nebulafractal[index] = thisbubble;

                    }
                }
            }
        }
    }

    vector<float>nebulamask(6 * eedge, 0.0f); // Create a mask to ensure that only parts of this fractal are used (we don't want any nebulae crossing from one face to another, as that looks bad).

    if (nebulaepresent)
    {
        for (int face = 0; face < 6; face++)
        {
            if (random(1, 3) == 1)
            {
                int vface = face * eedge;

                int radius = random(edge / 10, edge / 3); // Radius of this nebula.

                int centrex = random(radius, edge - radius);
                int centrey = random(radius, edge - radius);

                for (int i = -radius; i <= radius; i++)
                {
                    int vi = vface + (centrex + i) * edge;

                    for (int j = -radius; j <= radius; j++)
                    {
                        int index = vi + centrey + j;

                        float thisradius = (float)(sqrt(i * i + j * j));

                        nebulamask[index] = 1.0f - (thisradius / (float)radius);

                        if (nebulamask[index] < 0.0f)
                            nebulamask[index] = 0.0f;
                    }
                }
            }
        }
    }

    // Now make a fractal to vary the colour of the nebulae.

    vector<int>nebulacolourfractal(6 * eedge);

    if (nebulaepresent)
    {
        grain = (int)pow(2, random(2, 4)); // 8; // Level of detail on this fractal map.
        valuemod = 0.2f;
        valuemod2 = 3.0f;

        createfractal(nebulacolourfractal, edge, grain, valuemod, valuemod2, 1, maxelev, 0, 0);
    }

    // Now use the fractal to draw the stars, haze, and nebulae.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * eedge;

        for (int i = 0; i < skytexturesize; i++)
        {
            int ii = (int)((float)i * div);

            int vi = vface + ii * edge;

            for (int j = 0; j < skytexturesize; j++)
            {
                int jj = (int)((float)j * div);

                int index = vi + jj;

                float basechance = (float)fractal[index] / fmaxelev;

                float starchance = 0.2f;

                if (hazepresent)
                    starchance = basechance;

                if (starchance < minstarchance)
                    starchance = minstarchance;

                starchance = starchance * density;

                int prob = (int)(starchance * 10000.0f);

                if (prob > 0 && random(0, 10000) < prob) // Drawing a star!
                {
                    float brightness = (float)(random(10, 100)) * 0.01f;

                    brightness = brightness * brightness;

                    if (random(1, 3) == 1)
                        brightness = brightness * brightness;

                    Color starcolour{ 255,255,255,255 };

                    float r = 1.0f;
                    float g = 1.0f;
                    float b = 1.0f;

                    if (random(1, 20) == 1)
                    {
                        if (random(1, 10) == 1) // Blue star
                        {
                            r = (float)(random(1, 40)) * 0.01f;
                            g = r;
                        }
                        else
                        {
                            if (random(1, 3) == 1) // Red star
                            {
                                g = (float)(random(30, 70)) * 0.01f;
                                b = 0.0f;
                            }
                            else // Yellow star
                            {
                                b = (float)(random(1, 70)) * 0.01f;
                            }
                        }
                    }

                    r = r * brightness;
                    g = g * brightness;
                    b = b * brightness;

                    starcolour.r = (int)(r * 255.0);
                    starcolour.g = (int)(g * 255.0);
                    starcolour.b = (int)(b * 255.0);

                    ImageDrawPixel(&starsimage[face], i, j, starcolour);
                }

                // Now the haze.

                float r = basechance;
                float g = (float)hazecolourfractal[index] / fmaxelev;

                // Now the nebulae.

                float b = ((float)nebulafractal[index] / fmaxelev) * nebulamask[index];
                float a = (float)nebulacolourfractal[index] / fmaxelev;

                // Now encode both of those onto a single image.

                Color thishazecolour{ 0,0,0,0 };

                thishazecolour.r = (int)(r * 255.0);
                thishazecolour.g = (int)(g * 255.0);
                thishazecolour.b = (int)(b * 255.0);
                thishazecolour.a = (int)(a * 255.0);

                ImageDrawPixel(&starhazeimage[face], i, j, thishazecolour);
            }
        }
    }
}

// This creates the normal maps for the globe.

void drawglobalnormalimage(planet& world, simpleplanet& simpleworld, int globaltexturesize, vector<Image>& globalnormalimage, vector<fourglobepoints>& dirpoint, vector<fourdirs>& dircode)
{
    int edge = world.edge();
    int simpleedge = simpleworld.edge();
    int sealevel = world.sealevel();

    double pi = 3.14159265358979323846;

    vector<int>red(6 * simpleedge * simpleedge);
    vector<int>green(6 * simpleedge * simpleedge);
    vector<int>blue(6 * simpleedge * simpleedge);

    int colour1, colour2, colour3;

    float max = (float)(world.maxelevation() - sealevel);

    int div = simpleedge / edge;

    max = max * 0.2f; // 1.0f; // The smaller max is, the more strongly shaded the mountains will appear.

    for (int face = 0; face < 6; face++)
    {
        int vface = face * simpleedge * simpleedge;
        int vsmallface = face * edge * edge;

        for (int i = 0; i < simpleedge; i++)
        {
            int vi = vface + i * simpleedge;

            int vsmalli = vsmallface + ((i / div) * edge);

            for (int j = 0; j < simpleedge; j++)
            {
                int index = vi + j;
                int smallindex = vsmalli + j / div;

                bool sea = 0;

                if (simpleworld.map(face, i, j) <= sealevel && simpleworld.lake(face, i, j) == 0)
                    sea = 1;

                if (sea || simpleworld.lake(face, i, j)) // If this is water, no slope!
                {
                    colour1 = 128;
                    colour2 = 128;
                    colour3 = 255;
                }
                else // Work out the slope at this point, and use that to calculate the normal.
                {
                    Vector3 n;

                    if (1 == 0) //(face == 4 || face == 5)
                    {
                        // First, work out which sector of the face we're in.

                        int sector = 0;
                        float sectorwidth = 0;
                        float cornerdist = 0;

                        if (j < i)
                        {
                            if (i + j < simpleedge)
                            {
                                sectorwidth = (float)(j - simpleedge / 2);

                                if (i < simpleedge / 2)
                                {
                                    cornerdist = (float)i;
                                    sector = 4;
                                }
                                else
                                {
                                    cornerdist = (float)(simpleedge - i);
                                    sector = 3;
                                }

                            }
                            else
                            {
                                sectorwidth = (float)(i - simpleedge / 2);

                                if (j < simpleedge / 2)
                                {
                                    cornerdist = (float)j;
                                    sector = 1;
                                }
                                else
                                {
                                    cornerdist = (float)(simpleedge - j);
                                    sector = 2;
                                }

                            }

                        }
                        else
                        {
                            if (i + j < simpleedge)
                            {
                                sectorwidth = (float)(simpleedge / 2 - i);

                                if (j < simpleedge / 2)
                                {
                                    cornerdist = (float)j;
                                    sector = 6;
                                }
                                else
                                {
                                    cornerdist = (float)(simpleedge - j);
                                    sector = 5;
                                }
                            }
                            else
                            {
                                sectorwidth = (float)(simpleedge / 2 - j);

                                if (i < simpleedge / 2)
                                {
                                    cornerdist = (float)i;
                                    sector = 7;
                                }
                                else
                                {
                                    cornerdist = (float)(simpleedge - i);
                                    sector = 0;
                                }
                            }
                        }

                        if (sector == 7 || sector == 0 || sector == 3 || sector == 4) // These sectors can be calculated relatively simply.
                        {
                            getlidnormals(world, simpleworld, face, i, j, smallindex, sector, sectorwidth, cornerdist, max, dircode, n);

                        }
                        else // These have to be worked out by calculating what the normals would be if they were on neighbouring sectors, and then extrapolating.
                        {
                            Vector3 n1, n2;

                            if (sector == 1 || sector == 2)
                            {
                                getlidnormals(world, simpleworld, face, i, j, smallindex, 0, sectorwidth, 0.0f, max, dircode, n1);
                                getlidnormals(world, simpleworld, face, i, j, smallindex, 3, sectorwidth, 0.0f, max, dircode, n2);
                            }
                            else
                            {
                                getlidnormals(world, simpleworld, face, i, j, smallindex, 4, sectorwidth, 0.0f, max, dircode, n1);
                                getlidnormals(world, simpleworld, face, i, j, smallindex, 7, sectorwidth, 0.0f, max, dircode, n2);
                            }

                            float centredist = sectorwidth - cornerdist;

                            if (sector == 1)
                            {
                                float factor0 = (centredist / sectorwidth) * 0.5f + 0.5f;
                                float factor3 = 1.0f - factor0;

                                n.x = n1.x * factor0 + n2.x * factor3;
                                n.y = n1.y * factor0 + n2.y * factor3;
                                n.z = n1.z * factor0 + n2.z * factor3;
                            }

                            if (sector == 2)
                            {
                                float factor3 = (centredist / sectorwidth) * 0.5f + 0.5f;
                                float factor0 = 1.0f - factor3;

                                n.x = n1.x * factor0 + n2.x * factor3;
                                n.y = n1.y * factor0 + n2.y * factor3;
                                n.z = n1.z * factor0 + n2.z * factor3;
                            }

                            if (sector == 5)
                            {
                                float factor4 = (centredist / sectorwidth) * 0.5f + 0.5f;
                                float factor7 = 1.0f - factor4;

                                n.x = n1.x * factor4 + n2.x * factor7;
                                n.y = n1.y * factor4 + n2.y * factor7;
                                n.z = n1.z * factor4 + n2.z * factor7;
                            }

                            if (sector == 6)
                            {
                                float factor7 = (centredist / sectorwidth) * 0.5f + 0.5f;
                                float factor4 = 1.0f - factor7;

                                n.x = n1.x * factor4 + n2.x * factor7;
                                n.y = n1.y * factor4 + n2.y * factor7;
                                n.z = n1.z * factor4 + n2.z * factor7;
                            }

                            // Swap the horizontal/vertical factors as it gets closer to the centre.

                            float centredistx = (float)(i - simpleedge / 2);
                            float centredisty = (float)(j - simpleedge / 2);

                            centredist = sqrt(centredistx * centredistx + centredisty + centredisty);

                            if (centredist < simpleedge / 2)
                            {
                                float edgefactor = centredist / (simpleedge / 2);
                                float centrefactor = 1.0f - edgefactor;

                                Vector3 m;

                                m.x = n.x * edgefactor + n.y * centrefactor;
                                m.y = n.y * edgefactor + n.x * centrefactor;

                                n.x = m.x;
                                n.y = m.y;

                            }
                        }
                    }
                    else
                    {
                        // First, get the heights of the points around this one.

                        globepoint neighbourpoints[3][3];
                        float heights[3][3];

                        neighbourpoints[1][1].face = face;
                        neighbourpoints[1][1].x = i;
                        neighbourpoints[1][1].y = j;

                        for (int x = -1; x <= 1; x++)
                        {
                            for (int y = -1; y <= 1; y++)
                            {
                                if (x != 0 || y != 0)
                                    neighbourpoints[x + 1][y + 1] = getglobepoint(simpleedge, face, i, j, x, y);
                            }
                        }


                        /*
                        neighbourpoints[1][0] = getglobepointfromcode(simpleedge, face, i, j, dircode[smallindex].north);
                        neighbourpoints[2][1] = getglobepointfromcode(simpleedge, face, i, j, dircode[smallindex].east);
                        neighbourpoints[1][2] = getglobepointfromcode(simpleedge, face, i, j, dircode[smallindex].south);
                        neighbourpoints[0][1] = getglobepointfromcode(simpleedge, face, i, j, dircode[smallindex].west);

                        if (neighbourpoints[1][0].face != -1)
                        {
                            neighbourpoints[0][0] = getglobepointfromcode(simpleedge, neighbourpoints[1][0].face, neighbourpoints[1][0].x, neighbourpoints[1][0].y, dircode[smallindex].west);
                            neighbourpoints[2][0] = getglobepointfromcode(simpleedge, neighbourpoints[1][0].face, neighbourpoints[1][0].x, neighbourpoints[1][0].y, dircode[smallindex].east);
                        }

                        if (neighbourpoints[1][2].face != -1)
                        {
                            neighbourpoints[0][2] = getglobepointfromcode(simpleedge, neighbourpoints[1][2].face, neighbourpoints[1][2].x, neighbourpoints[1][2].y, dircode[smallindex].west);
                            neighbourpoints[2][2] = getglobepointfromcode(simpleedge, neighbourpoints[1][2].face, neighbourpoints[1][2].x, neighbourpoints[1][2].y, dircode[smallindex].east);
                        }
                        */

                        for (int k = 0; k < 3; k++)
                        {
                            for (int l = 0; l < 3; l++)
                            {
                                if (neighbourpoints[k][l].face != -1)
                                {
                                    heights[k][l] = (float)(simpleworld.map(neighbourpoints[k][l].face, neighbourpoints[k][l].x, neighbourpoints[k][l].y));

                                    if (heights[k][l] < (float)sealevel)
                                        heights[k][l] = (float)sealevel;

                                    heights[k][l] = heights[k][l] - (float)sealevel;

                                    heights[k][l] = heights[k][l] / max;
                                }
                            }
                        }

                        // Calculate a normal vector based upon those heights.

                        n.x = -(heights[2][2] - heights[0][2] + 2 * (heights[2][1] - heights[0][1]) + heights[2][0] - heights[0][0]);
                        n.y = -(heights[0][0] - heights[0][2] + 2 * (heights[1][0] - heights[1][2]) + heights[2][0] - heights[2][2]);
                        n.z = 1.0f;

                        // Now normalise it.

                        float len = sqrt(n.x * n.x + n.y * n.y + n.z * n.z);

                        n.x = n.x / len;
                        n.y = n.y / len;
                        n.z = n.z / len;
                    }

                    n.x = n.x * 0.5f + 0.5f;
                    n.y = n.y * 0.5f + 0.5f;
                    n.z = n.z * 0.5f + 0.5f;

                    colour1 = int(n.x * 255.0);
                    colour2 = int(n.y * 255.0);
                    colour3 = int(n.z * 255.0);

                    if (colour1 < 0)
                        colour1 = 0;

                    if (colour2 < 0)
                        colour2 = 0;

                    if (colour2 < 0)
                        colour3 = 0;

                    if (colour1 > 255)
                        colour1 = 255;
                    if (colour2 > 255)
                        colour2 = 255;
                    if (colour3 > 255)
                        colour3 = 255;
                }

                red[index] = colour1;
                green[index] = colour2;
                blue[index] = colour3;
            }
        }
    }

    if (simpleedge == globaltexturesize)
    {
        for (int face = 0; face < 6; face++)
        {
            int vface = face * globaltexturesize * globaltexturesize;

            for (int i = 0; i < globaltexturesize; i++)
            {
                int vi = vface + i * globaltexturesize;

                for (int j = 0; j < globaltexturesize; j++)
                {
                    int index = vi + j;

                    Color colour{ (unsigned char)red[index],(unsigned char)green[index],(unsigned char)blue[index],255 };

                    ImageDrawPixel(&globalnormalimage[face], i, j, colour);
                }
            }
        }
    }
    else // For smaller worlds we must expand it!
    {
        vector<int>redlarge(6 * globaltexturesize * globaltexturesize);
        vector<int>greenlarge(6 * globaltexturesize * globaltexturesize);
        vector<int>bluelarge(6 * globaltexturesize * globaltexturesize);

        float valuemod = 0.008f; // 0.002f;

        expand(red, redlarge, simpleedge, globaltexturesize, 0, 255, valuemod);
        expand(green, greenlarge, simpleedge, globaltexturesize, 0, 255, valuemod);
        expand(blue, bluelarge, simpleedge, globaltexturesize, 0, 255, valuemod);

        for (int face = 0; face < 6; face++)
        {
            int vface = face * globaltexturesize * globaltexturesize;

            for (int i = 0; i < globaltexturesize; i++)
            {
                int vi = vface + i * globaltexturesize;

                for (int j = 0; j < globaltexturesize; j++)
                {
                    int index = vi + j;

                    Color colour{ (unsigned char)redlarge[index],(unsigned char)greenlarge[index],(unsigned char)bluelarge[index],255 };

                    ImageDrawPixel(&globalnormalimage[face], i, j, colour);
                }
            }
        }
    }
}

// This gets the normals for a point on face 4 or face 5 of the globe.

void getlidnormals(planet& world, simpleplanet& simpleworld, int face, int i, int j, int smallindex, int sector, float sectorwidth, float cornerdist, float max, vector<fourdirs>& dircode, Vector3& n)
{
    int edge = world.edge();
    int simpleedge = simpleworld.edge();
    int sealevel = world.sealevel();

    // Now get the correct heights for the points around this one.
    // We do it twice: once assuming that the point is in the centre of the face edge, and once assuming it's at the corner. Then we'll combine them later.

    globepoint baseneighbourpoints[3][3]; // This is what would be the neighbouring points if everything were normal!

    globepoint centralneighbourpoints[3][3];
    float centralheights[3][3];

    globepoint cornerneighbourpoints[3][3];
    float cornerheights[3][3];

    baseneighbourpoints[1][1].face = face;
    baseneighbourpoints[1][1].x = i;
    baseneighbourpoints[1][1].y = j;

    centralneighbourpoints[1][1] = baseneighbourpoints[1][1];
    cornerneighbourpoints[1][1] = baseneighbourpoints[1][1];

    baseneighbourpoints[1][0] = getglobepointfromcode(simpleedge, face, i, j, dircode[smallindex].north);
    baseneighbourpoints[2][1] = getglobepointfromcode(simpleedge, face, i, j, dircode[smallindex].east);
    baseneighbourpoints[1][2] = getglobepointfromcode(simpleedge, face, i, j, dircode[smallindex].south);
    baseneighbourpoints[0][1] = getglobepointfromcode(simpleedge, face, i, j, dircode[smallindex].west);

    if (baseneighbourpoints[1][0].face != -1)
    {
        baseneighbourpoints[0][0] = getglobepointfromcode(simpleedge, baseneighbourpoints[1][0].face, baseneighbourpoints[1][0].x, baseneighbourpoints[1][0].y, dircode[smallindex].west);
        baseneighbourpoints[2][0] = getglobepointfromcode(simpleedge, baseneighbourpoints[1][0].face, baseneighbourpoints[1][0].x, baseneighbourpoints[1][0].y, dircode[smallindex].east);
    }

    if (baseneighbourpoints[1][2].face != -1)
    {
        baseneighbourpoints[0][2] = getglobepointfromcode(simpleedge, baseneighbourpoints[1][2].face, baseneighbourpoints[1][2].x, baseneighbourpoints[1][2].y, dircode[smallindex].west);
        baseneighbourpoints[2][2] = getglobepointfromcode(simpleedge, baseneighbourpoints[1][2].face, baseneighbourpoints[1][2].x, baseneighbourpoints[1][2].y, dircode[smallindex].east);
    }

    if ((face == 4 && (sector == 7 || sector == 0)) || (face == 5 && (sector == 3 || sector == 4))) // This is just the same as the base version.
    {
        for (int k = 0; k < 3; k++)
        {
            for (int l = 0; l < 3; l++)
                centralneighbourpoints[k][l] = baseneighbourpoints[k][l];
        }
    }

    if ((face == 4 && (sector == 3 || sector == 4)) || (face == 5 && (sector == 7 || sector == 0))) // Rotate 180 degrees.
    {
        centralneighbourpoints[0][0] = baseneighbourpoints[2][2];
        centralneighbourpoints[1][0] = baseneighbourpoints[1][2];
        centralneighbourpoints[2][0] = baseneighbourpoints[0][2];
        centralneighbourpoints[2][1] = baseneighbourpoints[0][1];
        centralneighbourpoints[2][2] = baseneighbourpoints[0][0];
        centralneighbourpoints[1][2] = baseneighbourpoints[1][0];
        centralneighbourpoints[0][2] = baseneighbourpoints[2][0];
        centralneighbourpoints[0][1] = baseneighbourpoints[2][1];
    }

    // Now rotate those to get what the points would be if we're at the corner of the face.

    if (sector == 3 || sector == 7) // Rotate clockwise.
    {
        cornerneighbourpoints[0][0] = centralneighbourpoints[0][1];
        cornerneighbourpoints[1][0] = centralneighbourpoints[0][0];
        cornerneighbourpoints[2][0] = centralneighbourpoints[1][0];
        cornerneighbourpoints[2][1] = centralneighbourpoints[2][0];
        cornerneighbourpoints[2][2] = centralneighbourpoints[2][1];
        cornerneighbourpoints[1][2] = centralneighbourpoints[2][2];
        cornerneighbourpoints[0][2] = centralneighbourpoints[1][2];
        cornerneighbourpoints[0][1] = centralneighbourpoints[0][2];
    }
    else // Rotate anticlockwise.
    {
        cornerneighbourpoints[0][0] = centralneighbourpoints[1][0];
        cornerneighbourpoints[1][0] = centralneighbourpoints[2][0];
        cornerneighbourpoints[2][0] = centralneighbourpoints[2][1];
        cornerneighbourpoints[2][1] = centralneighbourpoints[2][2];
        cornerneighbourpoints[2][2] = centralneighbourpoints[1][2];
        cornerneighbourpoints[1][2] = centralneighbourpoints[0][2];
        cornerneighbourpoints[0][2] = centralneighbourpoints[0][2];
        cornerneighbourpoints[0][1] = centralneighbourpoints[0][0];
    }

    // Now we get the heights for both of these.

    for (int k = 0; k < 3; k++)
    {
        for (int l = 0; l < 3; l++)
        {
            centralheights[k][l] = (float)(simpleworld.map(centralneighbourpoints[k][l].face, centralneighbourpoints[k][l].x, centralneighbourpoints[k][l].y));

            if (centralheights[k][l] < (float)sealevel)
                centralheights[k][l] = (float)sealevel;

            centralheights[k][l] = centralheights[k][l] - (float)sealevel;

            centralheights[k][l] = centralheights[k][l] / max;


            cornerheights[k][l] = (float)(simpleworld.map(cornerneighbourpoints[k][l].face, cornerneighbourpoints[k][l].x, cornerneighbourpoints[k][l].y));

            if (cornerheights[k][l] < (float)sealevel)
                cornerheights[k][l] = (float)sealevel;

            cornerheights[k][l] = cornerheights[k][l] - (float)sealevel;

            cornerheights[k][l] = cornerheights[k][l] / max;
        }
    }

    // Calculate normal vectors based upon those heights.

    Vector3 ncentral, ncorner;

    ncentral.x = -(centralheights[2][2] - centralheights[0][2] + 2 * (centralheights[2][1] - centralheights[0][1]) + centralheights[2][0] - centralheights[0][0]);
    ncentral.y = -(centralheights[0][0] - centralheights[0][2] + 2 * (centralheights[1][0] - centralheights[1][2]) + centralheights[2][0] - centralheights[2][2]);
    ncentral.z = 1.0f;

    ncorner.x = -(cornerheights[2][2] - cornerheights[0][2] + 2 * (cornerheights[2][1] - cornerheights[0][1]) + cornerheights[2][0] - cornerheights[0][0]);
    ncorner.y = -(cornerheights[0][0] - cornerheights[0][2] + 2 * (cornerheights[1][0] - cornerheights[1][2]) + cornerheights[2][0] - cornerheights[2][2]);
    ncorner.z = 1.0f;

    // Now normalise them.

    float len1 = sqrt(ncentral.x * ncentral.x + ncentral.y * ncentral.y + ncentral.z * ncentral.z);
    float len2 = sqrt(ncorner.x * ncorner.x + ncorner.y * ncorner.y + ncorner.z * ncorner.z);

    ncentral.x = ncentral.x / len1;
    ncentral.y = ncentral.y / len1;
    ncentral.z = ncentral.z / len1;

    ncorner.x = ncorner.x / len2;
    ncorner.y = ncorner.y / len2;
    ncorner.z = ncorner.z / len2;

    // Now we need to combine them depending on how close to the corner we actually are.

    if (sectorwidth < 1.0f)
        n = ncentral;
    else
    {
        float centredist = sectorwidth - cornerdist;

        n.x = (ncentral.x * cornerdist + ncorner.x * centredist) / sectorwidth;
        n.y = (ncentral.y * cornerdist + ncorner.y * centredist) / sectorwidth;
        n.z = (ncentral.z * cornerdist + ncorner.z * centredist) / sectorwidth;
    }
}

// This creates the elevation images for the globe.

void drawglobalelevationmapimage(planet& world, int globaltexturesize, vector<Image>& globalelevationimage)
{
    int edge = world.edge();
    int sealevel = world.sealevel();

    int colour1, colour2, colour3;

    int div = world.maxelevation() / 255;
    int base = world.maxelevation() / 4;

    int divstep = globaltexturesize / edge;

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < globaltexturesize; i++)
        {
            int ii = i / divstep;

            for (int j = 0; j < globaltexturesize; j++)
            {
                int jj = j / divstep;

                int heightpoint = world.map(face, ii, jj);

                colour1 = heightpoint / div;

                if (colour1 > 255)
                    colour1 = 255;

                colour2 = colour1;
                colour3 = colour2;

                Color colour{ (unsigned char)colour1, (unsigned char)colour2, (unsigned char)colour3,255 };

                ImageDrawPixel(&globalelevationimage[face], i, j, colour);
            }
        }
    }
}

// This creates the temperature images for the globe.

void drawglobaltemperaturemapimage(planet& world, int globaltexturesize, vector<Image>& globaltemperatureimage, vector<float>& latitude)
{
    int edge = world.edge();
    int sealevel = world.sealevel();

    int colour1, colour2, colour3;

    int landdiv = ((world.maxelevation() - sealevel) / 2) / 255;
    int seadiv = sealevel / 255;

    int div = world.maxelevation() / 255;
    int base = world.maxelevation() / 4;

    int divstep = globaltexturesize / edge;

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < globaltexturesize; i++)
        {
            int ii = i / divstep;

            for (int j = 0; j < globaltexturesize; j++)
            {
                int jj = j / divstep;

                if (world.outline(face, ii, jj))
                {
                    colour1 = 0;
                    colour2 = 0;
                    colour3 = 0;
                }
                else
                {
                    int jjantemp = world.jantemp(face, ii, jj);
                    int jjultemp = world.jultemp(face, ii, jj);
                    int eqtemp = world.equinoxtemp(face, ii, jj, latitude);

                    int temperature = (jjantemp + eqtemp + jjultemp + eqtemp) / 4;

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

                Color colour{ (unsigned char)colour1, (unsigned char)colour2, (unsigned char)colour3,255 };

                ImageDrawPixel(&globaltemperatureimage[face], i, j, colour);
            }
        }
    }
}

// This creates the precipitation images for the globe.

void drawglobalprecipitationmapimage(planet& world, int globaltexturesize, vector<Image>& globalprecipitationimage)
{
    int edge = world.edge();
    int sealevel = world.sealevel();

    int colour1, colour2, colour3;

    int landdiv = ((world.maxelevation() - sealevel) / 2) / 255;
    int seadiv = sealevel / 255;

    int div = world.maxelevation() / 255;
    int base = world.maxelevation() / 4;

    int divstep = globaltexturesize / edge;

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < globaltexturesize; i++)
        {
            int ii = i / divstep;

            for (int j = 0; j < globaltexturesize; j++)
            {
                int jj = j / divstep;

                if (world.outline(face, ii, jj))
                {
                    colour1 = 0;
                    colour2 = 0;
                    colour3 = 0;
                }
                else
                {
                    int rainfall = (world.summerrain(face, ii, jj) + world.winterrain(face, ii, jj)) / 2;

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

                /*
                switch (face)
                {
                case 0:
                    colour1 = 255;
                    colour2 = 0;
                    colour3 = 0;
                    break;

                case 1:
                    colour1 = 0;
                    colour2 = 255;
                    colour3 = 0;
                    break;

                case 2:
                    colour1 = 0;
                    colour2 = 0;
                    colour3 = 255;
                    break;

                case 3:
                    colour1 = 255;
                    colour2 = 255;
                    colour3 = 0;
                    break;

                case 4:
                    colour1 = 255;
                    colour2 = 0;
                    colour3 = 255;
                    break;

                case 5:
                    colour1 = 0;
                    colour2 = 255;
                    colour3 = 255;
                    break;
                }
                */

                Color colour{ (unsigned char)colour1, (unsigned char)colour2, (unsigned char)colour3,255 };

                ImageDrawPixel(&globalprecipitationimage[face], i, j, colour);
            }
        }
    }
}

// This creates the climate images for the globe.

void drawglobalclimatemapimage(planet& world, int globaltexturesize, vector<Image>& globalclimateimage)
{
    int edge = world.edge();
    int sealevel = world.sealevel();

    int colour1, colour2, colour3;

    int landdiv = ((world.maxelevation() - sealevel) / 2) / 255;
    int seadiv = sealevel / 255;

    int div = world.maxelevation() / 255;
    int base = world.maxelevation() / 4;

    int divstep = globaltexturesize / edge;

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < globaltexturesize; i++)
        {
            int ii = i / divstep;

            for (int j = 0; j < globaltexturesize; j++)
            {
                int jj = j / divstep;

                if (world.sea(face, ii, jj))
                {
                    if (world.seaice(face, ii, jj) == 2) // Permanent sea ice
                    {
                        colour1 = 243;
                        colour2 = 243;
                        colour3 = 255;
                    }
                    else
                    {
                        if (world.seaice(face, ii, jj) == 1) // Seasonal sea ice
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
                }
                else
                {
                    Color landcolour = getclimatecolours(world.climate(face, ii, jj));

                    colour1 = landcolour.r;
                    colour2 = landcolour.g;
                    colour3 = landcolour.b;
                }

                Color colour{ (unsigned char)colour1, (unsigned char)colour2, (unsigned char)colour3,255 };

                ImageDrawPixel(&globalclimateimage[face], i, j, colour);
            }
        }
    }
}

// This creates the rivers images for the globe.

void drawglobalriversmapimage(planet& world, int globaltexturesize, vector<Image>& globalriversimage)
{
    int edge = world.edge();
    int sealevel = world.sealevel();

    int colour1, colour2, colour3;

    int landdiv = ((world.maxelevation() - sealevel) / 2) / 255;
    int seadiv = sealevel / 255;

    int div = world.maxelevation() / 255;
    int base = world.maxelevation() / 4;

    int divstep = globaltexturesize / edge;

    int mult = world.maxriverflow() / 255;
    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < globaltexturesize; i++)
        {
            int ii = i / divstep;

            for (int j = 0; j < globaltexturesize; j++)
            {
                int jj = j / divstep;

                if (world.outline(face, ii, jj))
                {
                    colour1 = 0;
                    colour2 = 0;
                    colour3 = 0;
                }
                else
                {
                    int flow = world.riveraveflow(face, ii, jj);
                    int dflow = (world.deltajan(face, ii, jj) + world.deltajul(face, ii, jj)) / 2;

                    if (dflow > flow)
                        flow = dflow;

                    if (flow > 0 && world.sea(face, ii, jj) == 0)
                    {
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

                    colour3 = 255;

                    if (world.truelake(face, ii, jj) != 0 || world.riftlakesurface(face, ii, jj) != 0)
                    {
                        colour1 = 150;
                        colour2 = 150;
                        colour3 = 250;
                    }

                    if (world.special(face, ii, jj) > 100 && world.sea(face, ii, jj) == 0 && world.riverjan(face, ii, jj) + world.riverjul(face, ii, jj) < 600)
                    {
                        if (world.special(face, ii, jj) == 110)
                        {
                            colour1 = 150;
                            colour2 = 150;
                            colour3 = 150;
                        }

                        if (world.special(face, ii, jj) == 120)
                        {
                            colour1 = 250;
                            colour2 = 250;
                            colour3 = 50;
                        }

                        if (world.special(face, ii, jj) >= 130)
                        {
                            colour1 = 50;
                            colour2 = 250;
                            colour3 = 100;
                        }
                    }
                }

                if (world.outline(face, ii, jj))
                {
                    colour1 = 0;
                    colour2 = 0;
                    colour3 = 0;
                }

                if (world.volcano(face, ii, jj) > 0)
                {
                    colour1 = 240;
                    colour2 = 0;
                    colour3 = 0;
                }

                /*
                if (world.test(face, ii, jj) > 0)
                {
                    colour1 = 255;
                    colour2 = 0;
                    colour3 = 255;
                }
                */

                Color colour{ (unsigned char)colour1, (unsigned char)colour2, (unsigned char)colour3,255 };

                ImageDrawPixel(&globalriversimage[face], i, j, colour);
            }
        }
    }
}

// This creates the relief images for the globe.

void drawglobalreliefmapimage(planet& world, int globaltexturesize, vector<Image>& globalreliefimage, vector<float>& latitude)
{
    int edge = world.edge();
    int sealevel = world.sealevel();
    int type = world.type();

    int colour1, colour2, colour3;

    int landdiv = ((world.maxelevation() - sealevel) / 2) / 255;
    int seadiv = sealevel / 255;

    int div = world.maxelevation() / 255;
    int base = world.maxelevation() / 4;

    int divstep = globaltexturesize / edge;

    int mult = world.maxriverflow() / 255;

    int minriverflow = world.minriverflowglobal(); // Rivers of this size or larger will be shown on the map.

    bool colourcliffs = world.colourcliffs();

    int var = 0; // Amount colours may be varied to make the map seem more speckledy.

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < globaltexturesize; i++)
        {
            int ii = i / divstep;

            for (int j = 0; j < globaltexturesize; j++)
            {
                int jj = j / divstep;

                var = 10;

                int sea = world.sea(face, ii, jj);

                if (sea == 1)
                {
                    if ((world.seaice(face, ii, jj) == 2 && (world.seaiceappearance() == 1 || world.seaiceappearance() == 3)) || (world.seaice(face, ii, jj) == 1 && world.seaiceappearance() == 3)) // Sea iice
                    {
                        colour1 = world.seaice1();
                        colour2 = world.seaice2();
                        colour3 = world.seaice3();

                        var = 0;
                    }
                    else
                    {
                        colour1 = (world.ocean1() * world.map(face, ii, jj) + world.deepocean1() * (sealevel - world.map(face, ii, jj))) / sealevel;
                        colour2 = (world.ocean2() * world.map(face, ii, jj) + world.deepocean2() * (sealevel - world.map(face, ii, jj))) / sealevel;
                        colour3 = (world.ocean3() * world.map(face, ii, jj) + world.deepocean3() * (sealevel - world.map(face, ii, jj))) / sealevel;

                        var = 5;
                    }
                }
                else
                {
                    if ((world.riverjan(face, ii, jj) + world.riverjul(face, ii, jj)) / 2 >= minriverflow)
                    {
                        colour1 = world.river1();
                        colour2 = world.river2();
                        colour3 = world.river3();
                    }
                    else
                    {
                        if (world.special(face, ii, jj) == 110) // Salt pan
                        {
                            colour1 = world.saltpan1();
                            colour2 = world.saltpan2();
                            colour3 = world.saltpan3();

                            var = 20;
                        }
                        else
                        {
                            int avetemp = world.avetemp(face, ii, jj) + 10;

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

                            float winterrain = (float)world.winterrain(face, ii, jj);
                            float summerrain = (float)world.summerrain(face, ii, jj);

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

                            float rainforestmult = (float)world.mintemp(face, ii, jj) / 18.0f; //9.0f;

                            rainforestmult = rainforestmult * (float)world.winterrain(face, ii, jj) / 80.0f;

                            if (rainforestmult < 1.0f)
                                rainforestmult = 1.0f;

                            totalrain = totalrain * rainforestmult;

                            // Now adjust the colours for height.

                            int mapelev = world.map(face, ii, jj) - sealevel;
                            int desertmapelev = mapelev; // We won't mess about with this one.

                            // if this setting is chosen, pretend that the elevation is much lower for flat areas.

                            if (colourcliffs == 1)
                            {
                                int biggestslope = 0;

                                for (int k = ii - 1; k <= ii + 1; k++)
                                {
                                    for (int l = jj - 1; l <= jj + 1; l++)
                                    {
                                        if (k >= 0 && k < edge && l >= 0 && l < edge)
                                        {
                                            int thisslope = mapelev + sealevel - world.map(face, k, l);

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

                            // Now we need to mix these according to how dry the location iis.

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

                                    int powamount = (int)totalrain - 150; // 350 This iis to make a smoother transition.

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

                            // Now we need to alter that according to how cold the location iis.

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
                                /*
                                int latminutes = 0;
                                int latseconds = 0;
                                bool latneg = 0;

                                world.latitude(j, lat, latminutes, latseconds, latneg);
                                */

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

                            int special = world.special(face, ii, jj);

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

                if (world.sea(face, ii, jj) == 1)
                {
                    int amount = randomsign(random(0, var));

                    colour1 = colour1 + amount;
                    colour2 = colour2 + amount;
                    colour3 = colour3 + amount;

                    amount = randomsign(random(0, var));
                }
                else
                {
                    colour1 = colour1 + randomsign(random(0, var));
                    colour2 = colour2 + randomsign(random(0, var));
                    colour3 = colour3 + randomsign(random(0, var));

                    if (world.truelake(face, ii, jj) != 0)
                    {
                        colour1 = world.lake1();
                        colour2 = world.lake2();
                        colour3 = world.lake3();
                    }
                    else
                    {
                        if (world.riftlakesurface(face, ii, jj) != 0)
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

                Color colour{ (unsigned char)colour1, (unsigned char)colour2, (unsigned char)colour3,255 };

                ImageDrawPixel(&globalreliefimage[face], i, j, colour);
            }
        }
    }
}

// This creates the space images for the globe, and also the specular images (including specular, roughness, and cloud cover information).

void drawglobalspacemapimage(planet& world, simpleplanet& simpleworld, int globaltexturesize, int globallargetexturesize, vector<Image>& globalspaceimage, vector<Image>& globalspecularimage, vector<float>& latitude, vector<float>& proportions, float rainpoint, int frommonth, int tomonth, int fromface, int toface)
{
    int edge = world.edge();
    int simpleedge = simpleworld.edge();

    int sealevel = world.sealevel();
    int type = world.type();

    int seaicetemp = world.seaicetemp();

    int rivermin = 3000; // 10000; // Rivers larger than this will be shown partially.
    int rivermax = 8000; // 50000; // Rivers larger than this will be shown fully.

    int waterspec = 200; // Specular value for these elements.
    int icespec = 120;
    int saltspec = 80;
    int sandspec = 40;
    int wetlandspec = 60;
    int basespec = 30;

    int waterrough = 220; // 10; // Roughness value for these elements. Note that it's the wrong way round - higher values are smoother.
    int icerough = 180; // 180;
    int saltrough = 100;
    int sandrough = 80;
    int wetlandrough = 200;
    int baserough = 150;

    int colour1[12];
    int colour2[12];
    int colour3[12];

    vector<float>eqtemp(6 * edge * edge, -5000.0f);

    int landdiv = ((world.maxelevation() - sealevel) / 2) / 255;
    int seadiv = sealevel / 255;

    int div = world.maxelevation() / 255;
    int base = world.maxelevation() / 4;

    int divstep = globallargetexturesize / edge;

    int simpledivstep = globallargetexturesize / simpleedge;

    int convertdivstep = simpleedge / edge;

    int mult = world.maxriverflow() / 255;

    int minriverflow = world.minriverflowglobal(); // Rivers of this size or larger will be shown on the map.

    bool colourcliffs = world.colourcliffs();

    int var = 0; // Amount colours may be varied to make the map seem more speckledy.

    for (int face = fromface; face <= toface; face++)
    {
        int vbaseface = face * edge * edge;

        for (int simplei = 0; simplei < simpleedge; simplei++)
        {
            int ii = simplei / convertdivstep;

            int vii = vbaseface + ii * edge;

            for (int simplej = 0; simplej < simpleedge; simplej++)
            {
                int jj = simplej / convertdivstep;

                int baseindex = vii + jj;

                var = 10;

                int spec[12];
                int rough[12];
                int cloudcolour[12];

                int basecloudcolour = (int)((float)simpleworld.clouds(face, simplei, simplej) / div);

                for (int n = frommonth; n <= tomonth; n++)
                {
                    spec[n] = basespec;
                    rough[n] = baserough;
                    cloudcolour[n] = basecloudcolour;
                }

                // Get the temperature for every month.

                float temp[12];

                if (eqtemp[baseindex] == -5000.0f) // We haven't done this one yet!
                    eqtemp[baseindex] = (float)(world.equinoxtemp(face, ii, jj, latitude));

                int tempdiverge = (world.jantemp(face, ii, jj) - simpleworld.jantemp(face, simplei, simplej)) + (world.jultemp(face, ii, jj) - simpleworld.jultemp(face, simplei, simplej)) / 2;

                float thiseqtemp = eqtemp[baseindex] - (float)tempdiverge;

                monthlytemps(world, face, ii, jj, temp, latitude, thiseqtemp, (float)simpleworld.jantemp(face, simplei, simplej), (float)simpleworld.jultemp(face, simplei, simplej));
                
                float lowesttemp = temp[0];

                for (int n = 1; n < 12; n++)
                {
                    if (temp[n] < lowesttemp)
                        lowesttemp = temp[n];
                }

                int sea = 0; // world.sea(face, ii, jj);

                if (simpleworld.map(face, simplei, simplej) <= sealevel)
                    sea = 1;

                int seatempreduce = simpleworld.seatempreduce(face, simplei, simplej);

                bool lake = 0;

                if (simpleworld.lake(face, simplei, simplej))
                    lake = 1;

                int special = 0;

                for (int x = ii - 1; x <= ii + 1; x++)
                {
                    for (int y = jj - 1; y <= jj + 1; y++)
                    {
                        if (x >= 0 && x < edge && y >= 0 && y < edge)
                        {
                            if (world.special(face, x, y) != 0)
                            {
                                special = world.special(face, x, y);
                                x = ii + 2;
                                y = jj + 2;
                            }
                        }
                    }
                }

                if (sea)
                {
                    if (seatempreduce == 0)
                    {
                        for (int x = ii - 1; x <= ii + 1; x++)
                        {
                            for (int y = jj - 1; y <= jj + 1; y++)
                            {
                                if (x >= 0 && x < edge && y >= 0 && y < edge)
                                {
                                    if (world.seatempreduce(face, x, y) != 0)
                                    {
                                        seatempreduce = world.seatempreduce(face, x, y);
                                        x = ii + 2;
                                        y = jj + 2;
                                    }
                                }
                            }
                        }
                    }

                    for (int n = frommonth; n <= tomonth; n++)
                    {
                        if ((int)temp[n] - seatempreduce <= seaicetemp) // If it's cold enough for sea ice
                        {
                            colour1[n] = world.seaice1();
                            colour2[n] = world.seaice2();
                            colour3[n] = world.seaice3();

                            spec[n] = icespec;
                            rough[n] = icerough;

                            var = 0;
                        }
                        else // No sea ice
                        {
                            colour1[n] = (world.ocean1() + world.deepocean1()) / 2;
                            colour2[n] = (world.ocean2() + world.deepocean2()) / 2;
                            colour3[n] = (world.ocean3() + world.deepocean3()) / 2;

                            spec[n] = waterspec;
                            rough[n] = waterrough;

                            var = 5;
                        }
                    }
                }
                else
                {
                    // Get the rainfall for every month.

                    float rain[12];

                    monthlyrain(world, face, ii, jj, temp, rain, (float)simpleworld.janrain(face, simplei, simplej), (float)simpleworld.julrain(face, simplei, simplej));

                    float lowestrain = rain[0];

                    for (int n = 1; n < 12; n++)
                    {
                        if (rain[n] < lowestrain)
                            lowestrain = rain[n];
                    }

                    // And the river flow.

                    int flow[12];

                    monthlyflow(world, temp, rain, flow, (float)simpleworld.riverjan(face, simplei, simplej), (float)simpleworld.riverjul(face, simplei, simplej));

                    float winterrain = rain[0];
                    float summerrain = rain[6];

                    if (temp[6] < temp[0])
                    {
                        winterrain = rain[6];
                        summerrain = rain[0];
                    }

                    for (int n = frommonth; n <= tomonth; n++)
                    {
                        if (rain[n] < rainpoint) // If there isn't much rain here, reduce the clouds.
                        {
                            float mult = rain[n] / rainpoint;

                            cloudcolour[n] = (int)((float)cloudcolour[n] * mult);
                        }
                    }

                    // Now, check for the presence of monsoon.

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

                    // Also tropical rainforest.

                    float rainforestmult = lowesttemp / 18.0f; // (float)world.mintemp(face, ii, jj) / 18.0f; //9.0f;

                    rainforestmult = rainforestmult * lowestrain / 80.0f; // (float)world.winterrain(face, ii, jj) / 80.0f;

                    if (rainforestmult < 1.0f)
                        rainforestmult = 1.0f;

                    if (lake && special == 110) // Salt pan
                    {
                        for (int n = frommonth; n <= tomonth; n++)
                        {
                            colour1[n] = world.saltpan1();
                            colour2[n] = world.saltpan2();
                            colour3[n] = world.saltpan3();

                            spec[n] = saltspec;
                            rough[n] = saltrough;
                        }
                        var = 20;
                    }
                    else
                    {
                        int mapelev = simpleworld.map(face, simplei, simplej) - sealevel;
                        int desertmapelev = mapelev; // We won't mess about with this one.

                        // if this setting is chosen, pretend that the elevation is much lower for flat areas.

                        if (colourcliffs == 1)
                        {
                            int biggestslope = 0;

                            for (int k = simplei - 1; k <= simplei + 1; k++)
                            {
                                for (int l = simplej - 1; l <= simplej + 1; l++)
                                {
                                    if (k >= 0 && k < simpleedge && l >= 0 && l < simpleedge)
                                    {
                                        int thisslope = mapelev + sealevel - simpleworld.map(face, k, l);

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

                        // Alter the rainfall if we're near a river.

                        float biggestjanflow = 0.0f;
                        float biggestjulflow = 0.0f;

                        if (simplei > 0 && simplei < simpleedge - 1 && simplej > 0 && simplej < simpleedge - 1)
                        {
                            for (int k = simplei - 1; k <= simplei + 1; k++)
                            {
                                for (int l = simplej - 1; l <= simplej + 1; l++)
                                {
                                    float thisjanflow = (float)simpleworld.riverjan(face, k, l);
                                    float thisjulflow = (float)simpleworld.riverjul(face, k, l);

                                    if (k != simplei || l != simplej)
                                    {
                                        thisjanflow = thisjanflow * 0.5f;
                                        thisjulflow = thisjulflow * 0.5f;
                                    }

                                    if (thisjanflow > biggestjanflow)
                                        biggestjanflow = thisjanflow;

                                    if (thisjulflow > biggestjulflow)
                                        biggestjulflow = thisjulflow;
                                }
                            }
                        }
                        else
                        {
                            for (int k = -1; k <= 1; k++)
                            {
                                for (int l = -1; l <= 1; l++)
                                {
                                    globepoint thispoint = getglobepoint(simpleedge, face, simplei, simplej, k, l);

                                    if (thispoint.face != -1)
                                    {
                                        float thisjanflow = (float)simpleworld.riverjan(thispoint.face, thispoint.x, thispoint.y);
                                        float thisjulflow = (float)simpleworld.riverjul(thispoint.face, thispoint.x, thispoint.y);

                                        if (k != 0 || l != 0)
                                        {
                                            thisjanflow = thisjanflow * 0.5f;
                                            thisjulflow = thisjulflow * 0.5f;
                                        }

                                        if (thisjanflow > biggestjanflow)
                                            biggestjanflow = thisjanflow;

                                        if (thisjulflow > biggestjulflow)
                                            biggestjulflow = thisjulflow;
                                    }
                                }
                            }
                        }

                        if (biggestjanflow > 0.0f || biggestjulflow > 0.0f)
                        {
                            for (int n = 0; n < 12; n++)
                                rain[n] = 0;

                            float janrain = (float)simpleworld.janrain(face, simplei, simplej);
                            float julrain = (float)simpleworld.julrain(face, simplei, simplej);

                            /*
                            float mult = janrain;

                            if (mult < 1.0f)
                                mult = 1.0f;

                            if (mult > 1000.0f)
                                mult = 1000.0f;

                            biggestjanflow = biggestjanflow / mult;
                            */
                            janrain = janrain + (int)(biggestjanflow * 0.0025f);

                            /*
                            mult = julrain;

                            if (mult < 1.0f)
                                mult = 1.0f;

                            if (mult > 1000.0f)
                                mult = 1000.0f;


                            biggestjulflow = biggestjulflow / mult;
                            */
                            julrain = julrain + (int)(biggestjanflow * 0.0025f);

                            monthlyrain(world, face, ii, jj, temp, rain, janrain, julrain);

                            lowestrain = rain[0];

                            for (int n = 1; n < 12; n++)
                            {
                                if (rain[n] < lowestrain)
                                    lowestrain = rain[n];
                            }
                        }



                        for (int n = frommonth; n <= tomonth; n++)
                        {
                            float totalrain = rain[n] * 2.0f;
                            int avetemp = (int)temp[n] + 10;

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

                            totalrain = totalrain * rainforestmult;

                            if (totalrain > 800.0f) // 800
                            {
                                colour1[n] = newbase1;
                                colour2[n] = newbase2;
                                colour3[n] = newbase3;
                            }
                            else
                            {
                                if (totalrain > 200.0f) //400
                                {
                                    int wetno = ((int)totalrain - 200) / 40; //400 20
                                    if (wetno > 20)
                                        wetno = 20;
                                    int dryno = 20 - wetno;

                                    colour1[n] = (wetno * newbase1 + dryno * newgrass1) / 20;
                                    colour2[n] = (wetno * newbase2 + dryno * newgrass2) / 20;
                                    colour3[n] = (wetno * newbase3 + dryno * newgrass3) / 20;
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

                                    colour1[n] = (wetno * newgrass1 + dryno * newdesert1) / 200;
                                    colour2[n] = (wetno * newgrass2 + dryno * newdesert2) / 200;
                                    colour3[n] = (wetno * newgrass3 + dryno * newdesert3) / 200;
                                }
                            }

                            // Now we need to alter that according to how cold the location is.

                            if (avetemp <= 0)
                            {
                                colour1[n] = world.cold1();
                                colour2[n] = world.cold2();
                                colour3[n] = world.cold3();

                                spec[n] = icespec;
                                rough[n] = icerough;
                            }
                            else
                            {
                                // Get the right tundra colour, depending on latitude.

                                int lat = 0;
                                /*
                                int latminutes = 0;
                                int latseconds = 0;
                                bool latneg = 0;

                                world.latitude(j, lat, latminutes, latseconds, latneg);
                                */

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
                                            colour1[n] = world.cold1();
                                            colour2[n] = world.cold2();
                                            colour3[n] = world.cold3();

                                            spec[n] = icespec;
                                            rough[n] = icerough;
                                        }
                                        else
                                        {
                                            if (avetemp < 10)
                                            {
                                                colour1[n] = thistundra1;
                                                colour2[n] = thistundra2;
                                                colour3[n] = thistundra3;
                                            }
                                            else
                                            {
                                                int hotno = avetemp - 10;
                                                int coldno = 10 - hotno;

                                                colour1[n] = (hotno * colour1[n] + coldno * thistundra1) / 10;
                                                colour2[n] = (hotno * colour2[n] + coldno * thistundra2) / 10;
                                                colour3[n] = (hotno * colour3[n] + coldno * thistundra3) / 10;
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
                                            colour1[n] = world.cold1();
                                            colour2[n] = world.cold2();
                                            colour3[n] = world.cold3();
                                        }
                                        else
                                        {
                                            if (avetemp < 10)
                                            {
                                                if (random(6, 10) < avetemp)
                                                {
                                                    colour1[n] = thistundra1;
                                                    colour2[n] = thistundra2;
                                                    colour3[n] = thistundra3;
                                                }
                                                else
                                                {
                                                    colour1[n] = world.cold1();
                                                    colour2[n] = world.cold2();
                                                    colour3[n] = world.cold3();
                                                }
                                            }
                                            else
                                            {
                                                int hotno = avetemp - 10;
                                                int coldno = 10 - hotno;

                                                colour1[n] = (hotno * colour1[n] + coldno * thistundra1) / 10;
                                                colour2[n] = (hotno * colour2[n] + coldno * thistundra2) / 10;
                                                colour3[n] = (hotno * colour3[n] + coldno * thistundra3) / 10;
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

                                            colour1[n] = (hotno * thistundra1 + coldno * world.cold1()) / 10;
                                            colour2[n] = (hotno * thistundra2 + coldno * world.cold2()) / 10;
                                            colour3[n] = (hotno * thistundra3 + coldno * world.cold3()) / 10;
                                        }
                                        else
                                        {
                                            int hotno = avetemp - 10;
                                            int coldno = 10 - hotno;

                                            colour1[n] = (hotno * colour1[n] + coldno * thistundra1) / 10;
                                            colour2[n] = (hotno * colour2[n] + coldno * thistundra2) / 10;
                                            colour3[n] = (hotno * colour3[n] + coldno * thistundra3) / 10;
                                        }
                                    }
                                }
                            }

                            // Now add sand, if need be.

                            if (lake && special == 120)
                            {
                                colour1[n] = (colour1[n] * 2 + world.erg1()) / 3;
                                colour2[n] = (colour2[n] * 2 + world.erg2()) / 3;
                                colour3[n] = (colour3[n] * 2 + world.erg3()) / 3;

                                spec[n] = sandspec;
                                rough[n] = sandrough;
                            }

                            // Now wetlands.

                            if (lake && special >= 130 && special < 140)
                            {
                                colour1[n] = (colour1[n] * 2 + world.wetlands1()) / 3;
                                colour2[n] = (colour2[n] * 2 + world.wetlands2()) / 3;
                                colour3[n] = (colour3[n] * 2 + world.wetlands3()) / 3;

                                spec[n] = wetlandspec; // random(wetlandspec / 8, wetlandspec);
                                rough[n] = wetlandrough;
                            }
                        }
                    }

                    // Now maybe show rivers.

                    for (int n = frommonth; n <= tomonth; n++)
                    {
                        if (flow[n] > rivermin)
                        {
                            if (flow[n] > rivermax)
                            {
                                colour1[n] = (colour1[n] + world.river1()) / 2;
                                colour2[n] = (colour2[n] + world.river2()) / 2;
                                colour3[n] = (colour3[n] + world.river3()) / 2;

                                spec[n] = waterspec;
                                rough[n] = waterrough;
                            }
                            else
                            {
                                float fflow = (float)(flow[n] - rivermin);

                                float riverdiff = (float)(rivermax - rivermin);

                                float rivermult = fflow / riverdiff;

                                float normmult = 1.0f - rivermult;

                                colour1[n] = (colour1[n] + (int)(rivermult * (float)world.river1()) + (int)(normmult * (float)colour1[n])) / 2;
                                colour2[n] = (colour2[n] + (int)(rivermult * (float)world.river2()) + (int)(normmult * (float)colour2[n])) / 2;
                                colour3[n] = (colour3[n] + (int)(rivermult * (float)world.river3()) + (int)(normmult * (float)colour3[n])) / 2;

                                spec[n] = (int)((float)waterspec * rivermult + (float)basespec * normmult);
                                rough[n] = (int)((float)waterrough * rivermult + (float)baserough * normmult); //waterrough; // 255 - waterrough; // Keep it with the standard roughness for water, otherwise smaller rivers can be shinier.
                            }
                        }
                    }
                }

                for (int n = frommonth; n <= tomonth; n++)
                {
                    if (sea)
                    {
                        int amount = randomsign(random(0, var));

                        colour1[n] = colour1[n] + amount;
                        colour2[n] = colour2[n] + amount;
                        colour3[n] = colour3[n] + amount;
                    }
                    else
                    {
                        colour1[n] = colour1[n] + randomsign(random(0, var));
                        colour2[n] = colour2[n] + randomsign(random(0, var));
                        colour3[n] = colour3[n] + randomsign(random(0, var));

                        bool truelake = 0;

                        for (int x = ii - 1; x <= ii + 1; x++)
                        {
                            for (int y = jj - 1; y <= jj + 1; y++)
                            {
                                if (x >= 0 && x < edge && y >= 0 && y < edge)
                                {
                                    if (world.truelake(face, x, y) != 0 || world.riftlakesurface(face, x, y) != 0)
                                    {
                                        truelake = 1;
                                        x = ii + 2;
                                        y = jj + 2;
                                    }
                                }
                            }
                        }

                        if (lake && truelake)
                        {
                            colour1[n] = world.lake1();
                            colour2[n] = world.lake2();
                            colour3[n] = world.lake3();

                            spec[n] = waterspec;
                            rough[n] = waterrough;
                        }
                    }
                }

                for (int n = frommonth; n <= tomonth; n++)
                {
                    if (colour1[n] > 255)
                        colour1[n] = 255;
                    if (colour2[n] > 255)
                        colour2[n] = 255;
                    if (colour3[n] > 255)
                        colour3[n] = 255;

                    if (colour1[n] < 0)
                        colour1[n] = 0;
                    if (colour2[n] < 0)
                        colour2[n] = 0;
                    if (colour3[n] < 0)
                        colour3[n] = 0;
                }

                for (int i = simplei * simpledivstep; i < (simplei + 1) * simpledivstep; i++)
                {
                    for (int j = simplej * simpledivstep; j < (simplej + 1) * simpledivstep; j++)
                    {
                        for (int n = frommonth; n <= tomonth; n++)
                        {
                            Color colour{ (unsigned char)colour1[n], (unsigned char)colour2[n], (unsigned char)colour3[n], 255 };

                            ImageDrawPixel(&globalspaceimage[face * 12 + n], i, j, colour);
                        }

                        for (int n = frommonth; n <= tomonth; n++)
                        {
                            Color speccolour{ (unsigned char)spec[n], (unsigned char)rough[n], (unsigned char)cloudcolour[n], 255 };

                            ImageDrawPixel(&globalspecularimage[face * 12 + n], i, j, speccolour);
                        }
                    }
                }
            }
        }
    }
}


