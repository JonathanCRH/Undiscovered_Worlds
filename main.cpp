
#include "imgui.h"
#include "imgui-SFML.h"
#include "ImGuiFileDialog.h"

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

static long g_seed;

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
    // Set up the window.

    int scwidth = 1224;
    int scheight = 768;

    sf::RenderWindow window(sf::VideoMode(scwidth, scheight), "Undiscovered Worlds");
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);

    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();

    // Setting up Dear ImGUI style
    // Based on Cinder-ImGui by Simon Geilfus - https://github.com/simongeilfus/Cinder-ImGui

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

    float highlight1 = 0.40;
    float highlight2 = 0.40;
    float highlight3 = 0.40;

    Style.Colors[ImGuiCol_Text] = ImVec4(0.86f, 0.93f, 0.89f, 0.78f);
    Style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.86f, 0.93f, 0.89f, 0.28f);
    Style.Colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.14f, 0.17f, 1.00f);
    Style.Colors[ImGuiCol_Border] = ImVec4(0.31f, 0.31f, 1.00f, 0.00f);
    Style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    Style.Colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
    Style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(highlight1, highlight2, highlight3, 0.78f);
    Style.Colors[ImGuiCol_FrameBgActive] = ImVec4(highlight1, highlight2, highlight3, 1.00f);
    Style.Colors[ImGuiCol_TitleBg] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
    Style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.20f, 0.22f, 0.27f, 0.75f);
    Style.Colors[ImGuiCol_TitleBgActive] = ImVec4(highlight1, highlight2, highlight3, 1.00f);
    Style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.20f, 0.22f, 0.27f, 0.47f);
    Style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
    Style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.09f, 0.15f, 0.16f, 1.00f);
    Style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.29f, 0.18f, 0.92f, 0.78f);
    Style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(highlight1, highlight2, highlight3, 1.00f);
    Style.Colors[ImGuiCol_CheckMark] = ImVec4(0.27f, 0.22f, 0.71f, 1.00f);
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
    globalmap->setPosition(sf::Vector2f(globalmapxpos,globalmapypos));

    // And also the minimap, which will use the same texture.

    sf::Sprite* minimap = new sf::Sprite;

    float minimapxpos = 190.f+globaltexturesize.x/4;
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
    highlight->setOrigin(highlightsize / 2, highlightsize / 2);

    sf::Texture* minihighlighttexture = new sf::Texture;
    minihighlighttexture->loadFromImage(*minihighlightimage);

    sf::Sprite* minihighlight = new sf::Sprite;
    minihighlight->setTexture(*minihighlighttexture);
    minihighlight->setOrigin(minihighlightsize / 2, minihighlightsize / 2);

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

    vector<vector<float>> riftblob(riftblobsize + 2, vector<float>(riftblobsize + 2, 0));

    createriftblob(riftblob, riftblobsize / 2);

    // Other bits

    fast_srand(time(0));

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

    screenmodeenum screenmode = createworldscreen; // This is used to keep track of which screen mode we're in.
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

    bool globalmapimagecreated[GLOBALMAPTYPES]; // This will keep track of which global map images have actually been created.

    bool regionalmapimagecreated[GLOBALMAPTYPES]; // This will keep track of which global map images have actually been created.

    short generatingnewregion = 0; // If this is 2 then we're about to generate a new region. If it's 1 then we will do so on the next frame.

    int newx = -1;
    int newy = -1; // These are used to locate the new region.

    bool showcolouroptions = 0; // If this is 1 then we display the appearance preferences options.

    bool showwarning = 0; // If this is 1 then a warning is shown (which allows the import screen to be opened).

    string infotext = "Welcome to a new world!"; // Info about the point selected in the global map screen
    string infotext2 = ""; // Info about the point selected in the regional map screen

    for (int n = 0; n < GLOBALMAPTYPES; n++)
        regionalmapimagecreated[n] = 0;

    bool colourschanged = 0; // If this is 1 then the colours have been changed and the maps need to be redrawn.

    float linespace = 8.0f; // Gap between groups of buttons.

    // Now we prepare map colours. We put them into ImVec4 objects, which can be directly manipulated by the colour picker objects.

    ImVec4 oceancolour;

    oceancolour.x = (float)world->ocean1()/255.f;
    oceancolour.y = (float)world->ocean2()/255.f;
    oceancolour.z = (float)world->ocean3()/255.f;
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

    int snowchange = world->snowchange()-1;
    int seaiceappearance = world->seaiceappearance()-1;

    sf::Clock deltaClock;
    while (window.isOpen())
    {
        if (generatingnewregion == 1)
            generatingnewregion = 2;

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
        {
            screenmode = globalmapscreen;
        }

        //ImGui::ShowDemoWindow();

        // First, draw the GUI.

        // Create world screen

        if (screenmode == createworldscreen)
        {
            showwarning = 0;

            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 507, main_viewport->WorkPos.y + 173), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(193, 71), ImGuiCond_FirstUseEver);
            ImGui::Begin("Create world");

            ImGui::SetNextItemWidth(-1.0f);
            ImGui::InputInt(" ", &seedentry);

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Please enter a seed number, which will be used to calculate the new world. The same number will always yield the same world.");

            if (seedentry < 0)
                seedentry = 0;

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
                if (ImGui::Button("Cancel"))
                    screenmode = globalmapscreen;
            }

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Load a world");

            ImGui::SameLine();

            if (ImGui::Button("Import"))
            {
                screenmode = importscreen;
            }

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Create a world from imported maps.");

            ImGui::SameLine();

            if (ImGui::Button("Random"))
            {
                int seed = random(0, 9);

                for (int n = 1; n <= 7; n++)
                {
                    if (n == 7)
                        seed = seed + (random(1, 9) * pow(10, n));
                    else
                        seed = seed + (random(0, 9) * pow(10, n));
                }

                seedentry = seed;
            }

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Roll a random seed number.");

            ImGui::SameLine();

            if (ImGui::Button("OK"))
            {
                // Accept seed and generate world

                //seedentry = 68384841; // Works!
                //seedentry = 55647486; // volcano

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
            for (int n = 0; n < 2; n++)
            {
                ImGui::SFML::Update(window, deltaClock.restart());
                ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 507, main_viewport->WorkPos.y + 173), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize(ImVec2(200, 50), ImGuiCond_FirstUseEver);
                ImGui::Begin("Please wait!");
                ImGui::Text("Generating world...");
                ImGui::End();
                window.clear();
                ImGui::SFML::Render(window);
                window.display();
            }       

            updatereport("Generating world from seed: "+to_string(world->seed())+":");
            updatereport("");
            
            for (int n = 0; n < GLOBALMAPTYPES; n++) // Set all map types as unviewed, to force them to be redrawn when called up
                globalmapimagecreated[n] = 0;

            short terraintype = 2; // This terrain type gives large continents.

            if (random(1, 10) == 1) // Rarely, do the terrain type that gives fragmented land masses.
                terraintype = 1;

            vector<vector<int>> mountaindrainage(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));
            vector<vector<bool>> shelves(ARRAYWIDTH, vector<bool>(ARRAYHEIGHT, 0));

            // Actually generate the world

            generateglobalterrain(*world, terraintype, landshape, chainland, mountaindrainage, shelves);
            generateglobalclimate(*world, smalllake, largelake, landshape, mountaindrainage, shelves);

            // Now draw a new map

            mapview = relief;

            drawglobalmapimage(mapview, *world, globalmapimagecreated, *globalelevationimage, *globaltemperatureimage, *globalprecipitationimage, *globalclimateimage, *globalriversimage, *globalreliefimage, *displayglobalelevationimage, *displayglobaltemperatureimage, *displayglobalprecipitationimage, *displayglobalclimateimage, *displayglobalriversimage, *displayglobalreliefimage);

            globalmaptexture->loadFromImage(*displayglobalreliefimage);
            globalmap->setTexture(*globalmaptexture);

            updatereport("");
            updatereport("World generation completed.");
            updatereport("");

            infotext = "Welcome to a new world!";

            focused = 0;

            screenmode = movingtoglobalmapscreen;
        }

        // Global map screen

        if (screenmode == globalmapscreen)
        {
            // Main controls.
            
            string title = "Seed: " + to_string(world->seed());
           
            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 10, main_viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(160, 431), ImGuiCond_FirstUseEver);

            ImGui::Begin(title.c_str());

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

            if (standardbutton("Import world"))
            {
                showwarning = 1;
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

            if (standardbutton("Appearance"))
            {
                showcolouroptions = 1;

            }

            if (standardbutton("Zoom"))
            {
                if (focused == 1)
                {
                    globalmaptexture->loadFromImage(*displayglobalreliefimage);
                    globalmap->setTexture(*globalmaptexture);
                    minimap->setTexture(*globalmaptexture);
                    
                    newx = poix;
                    newy = poiy;

                    generatingnewregion = 1;

                    infotext = "";
                    infotext2 = "";

                    screenmode = regionalmapscreen;
                }
            }

            ImGui::End();

            // Now the text box.

            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 180, main_viewport->WorkPos.y + 542), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(1023, 140), ImGuiCond_FirstUseEver);
            title = "            ";

            ImGui::Begin(title.c_str());
            ImGui::PushItemWidth(world->width() / 2);
            ImGui::Text(infotext.c_str(), world->width() / 2);

            ImGui::End();

            // Now check to see if the map has been clicked on.

            if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && io.WantCaptureMouse==0)
            {
                sf::Vector2i mousepos = sf::Mouse::getPosition(window);

                poix = (mousepos.x - globalmapxpos) * 2;
                poiy = (mousepos.y - globalmapypos) * 2;

                if (poix>=0 && poix<=world->width() && poiy>=0 && poiy<=world->height())
                {
                    focused = 1;

                    highlight->setPosition(sf::Vector2f(mousepos.x, mousepos.y));

                    int sealevel = world->sealevel();

                    infotext = "Location is " + to_string(poix) + ", " + to_string(poiy) + ". Latitude " + to_string(world->latitude(poix, poiy)) + "�. ";

                    int wind = world->wind(poix, poiy);
                    string winddir;

                    if (wind > 0)
                        winddir = " westerly. ";

                    if (wind < 0)
                        winddir = " easterly. ";

                    if (wind == 0 || wind > 50)
                        winddir = ". ";

                    if (wind < 0)
                        wind = -wind;

                    if (wind > 50)
                        wind = 0;

                    infotext = infotext + "Wind: " + to_string(wind) + winddir;

                    int pointelevation = world->map(poix, poiy);

                    if (world->sea(poix, poiy) == 0)
                    {
                        if (pointelevation > sealevel)
                            infotext = infotext + "Elevation: " + to_string(pointelevation - sealevel) + " metres above sea level.\n";
                        else
                        {
                            if (world->truelake(poix, poiy) == 1)
                                infotext = infotext + "Elevation: " + to_string(sealevel - pointelevation) + " metres below sea level.\n";
                            else
                                infotext = infotext + "Elevation: " + to_string(world->lakesurface(poix, poiy) - sealevel) + " metres above sea level.\n";
                        }

                        string climatetype = world->climate(poix, poiy);
                        string climate = getclimatename(climatetype) + " (" + climatetype + ")";
                        string glac = "";

                        if ((world->jantemp(poix, poiy) + world->jultemp(poix, poiy)) / 2 <= world->glacialtemp())
                            glac = "Glacial region. ";

                        if (world->special(poix, poiy) == 110)
                            climate = climate + ". Salt pan";

                        if (world->special(poix, poiy) == 120)
                            climate = climate + ". Dunes";

                        if (world->special(poix, poiy) == 130)
                            climate = climate + ". Wetlands";

                        if (world->special(poix, poiy) == 131)
                            climate = climate + ". Brackish wetlands";

                        if (world->special(poix, poiy) == 132)
                            climate = climate + ". Salt wetlands";

                        if (world->volcano(poix, poiy) > 0)
                            climate = climate + ". Volcano";

                        infotext = infotext + "Climate: " + climate + ". " + glac + "\n";
                    }
                    else
                    {
                        infotext = infotext + "Elevation: " + to_string(sealevel - pointelevation) + " metres below sea level. ";

                        if (world->seaice(poix, poiy) == 2)
                            infotext = infotext + "Permanent sea ice. ";
                        else
                        {
                            if (world->seaice(poix, poiy) == 1)
                                infotext = infotext + "Seasonal sea ice. ";

                        }
                        infotext = infotext + "\n";

                        if (world->volcano(poix, poiy) > 0)
                            infotext = infotext + "Submarine volcano.\n";
                    }

                    infotext = infotext + "January temperature: " + to_string(world->jantemp(poix, poiy)) + "C. July temperature: " + to_string(world->jultemp(poix, poiy)) + "C. ";
                    infotext = infotext + "January rainfall: " + to_string(world->janrain(poix, poiy)) + " mm/month. July rainfall: " + to_string(world->julrain(poix, poiy)) + " mm/month.\n";

                    if (world->sea(poix, poiy) == 0 && world->riveraveflow(poix, poiy) > 0 && world->lakesurface(poix, poiy) == 0)
                    {
                        string direction;

                        switch (world->riverdir(poix, poiy))
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
                        infotext = infotext + "River direction: " + direction + ". January flow: " + to_string(world->riverjan(poix, poiy)) + " m3/s. July flow: " + to_string(world->riverjul(poix, poiy)) + " m3/s.\n";
                    }

                    if (world->truelake(poix, poiy) != 0)
                    {
                        infotext = infotext + "Lake elevation: " + to_string(world->lakesurface(poix, poiy) - sealevel) + " metres. ";
                        int depth = world->lakesurface(poix, poiy) - world->nom(poix, poiy);

                        string salt = "";

                        if (world->special(poix, poiy) == 100)
                            salt = "Salty. ";

                        infotext = infotext + "Depth: " + to_string(depth) + " metres. " + salt;

                    }
                }
                else
                {
                    if (focused == 1)
                    {
                        focused = 0;
                        poix = -1;
                        poiy = -1;
                        infotext = "";
                    }
                }
            }
        }

        // Regional map screen

        if (screenmode == regionalmapscreen)
        {
            showwarning = 0;

            // Main controls.

            string title;
            
            if (world->seed() >= 0)
                title = "Seed: " + to_string(world->seed());
            else
                title = "Custom";

            title = title + "##regional";

            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 10, main_viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(160, 346), ImGuiCond_FirstUseEver);

            ImGui::Begin(title.c_str());

            ImGui::Text("World controls:");

            ImGui::PushItemWidth(100.0f);

            if (standardbutton("World map"))
            {
                focused = 0;
                poix = -1;
                poiy = -1;
                infotext = "";

                for (int i = 0; i < regionalimagewidth; i++)
                {
                    for (int j = 0; j < regionalimageheight; j++)
 
                        regionalreliefimage->setPixel(i, j, sf::Color::Black);
                }

                regionalmaptexture->loadFromImage(*regionalreliefimage);
                regionalmap->setTexture(*regionalmaptexture);

                screenmode = globalmapscreen;
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

            if (standardbutton("Appearance"))
            {
                showcolouroptions = 1;

            }

            ImGui::End();

            // Now the text box.

            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 180, main_viewport->WorkPos.y + 542), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(1023, 140), ImGuiCond_FirstUseEver);

            title = "               ";

            ImGui::Begin(title.c_str());
            ImGui::PushItemWidth(world->width() / 2);
            ImGui::Text(infotext2.c_str(), world->width() / 2);
            ImGui::End();
#
            // Now check to see if the map has been clicked on.

            if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && io.WantCaptureMouse == 0)
            {
                sf::Vector2i mousepos = sf::Mouse::getPosition(window);

                poix = mousepos.x - regionalmapxpos;
                poiy = mousepos.y - regionalmapypos;

                if (poix >= 0 && poix < regionalmapimagewidth && poiy >= 0 && poiy < regionalmapimageheight)
                {
                    poix=poix + region->regwidthbegin();
                    poiy=poiy + region->regheightbegin();

                    focused = 1;

                    highlight->setPosition(sf::Vector2f(mousepos.x, mousepos.y));

                    int sealevel = world->sealevel();

                    int xx = region->leftx() + poix / 16;
                    int yy = region->lefty() + poiy / 16; // Coordinates of the gobal cell we're in.

                    if (region->surface(poix, poiy) != 0 && region->special(poix, poiy) > 100) // If there's a "special" lake here
                    {
                        infotext2 = "Elevation: " + to_string(region->surface(poix, poiy) - sealevel) + " metres above sea level.\n";
                    }
                    else
                    {
                        if (region->map(poix, poiy) <= sealevel)
                        {
                            infotext2 = "Elevation: " + to_string(sealevel - region->map(poix, poiy)) + " metres below sea level.\n";
                        }
                        else
                            infotext2 = "Elevation: " + to_string(region->map(poix, poiy) - sealevel) + " metres above sea level.\n";
                    }

                    int janrain, julrain, jantemp, jultemp;

                    if (yy <= height / 2) // Northern hemisphere
                    {
                        janrain = region->winterrain(poix, poiy);
                        julrain = region->summerrain(poix, poiy);

                        jantemp = region->mintemp(poix, poiy);
                        jultemp = region->maxtemp(poix, poiy);

                    }
                    else // Southern hemisphere
                    {
                        janrain = region->summerrain(poix, poiy);
                        julrain = region->winterrain(poix, poiy);

                        jantemp = region->maxtemp(poix, poiy);
                        jultemp = region->mintemp(poix, poiy);
                    }

                    infotext2 = infotext2 + "January temperature: " + to_string(jantemp) + "C. July temperature: " + to_string(jultemp) + "C. ";

                    infotext2 = infotext2 + "January rainfall: " + to_string(janrain) + " mm/month. July rainfall: " + to_string(julrain) + " mm/month.\n";

                    if (region->truelake(poix, poiy) == 1) // If there's a lake here
                    {
                        infotext2 = infotext2 + "Lake elevation: " + to_string(region->lakesurface(poix, poiy) - sealevel) + " metres. Depth: " + to_string(region->lakesurface(poix, poiy) - region->map(poix, poiy)) + " metres. ";

                        if (region->special(poix, poiy) == 100)
                            infotext2 = infotext2 + "Salty. ";

                        infotext2 = infotext2 + "\n";
                    }
                    else
                    {
                        if (region->riverdir(poix, poiy) != 0 && region->special(poix, poiy) != 140) // If there's a river here
                        {
                            infotext2 = infotext2 + "River direction: " + getdirstring(region->riverdir(poix, poiy)) + ". ";
                            infotext2 = infotext2 + "January flow: " + to_string(region->riverjan(poix, poiy)) + " m3/s. July flow: " + to_string(region->riverjul(poix, poiy)) + " m3/s. \n";
                        }
                        else
                        {
                            if (region->fakedir(poix, poiy) > 0 && region->special(poix, poiy) != 140) // If there's a fake river here
                            {
                                infotext2 = infotext2 + "River direction: " + getdirstring(region->fakedir(poix, poiy)) + ". ";
                                infotext2 = infotext2 + "January flow: " + to_string(region->fakejan(poix, poiy)) + " m3/s. July flow: " + to_string(region->fakejul(poix, poiy)) + " m3/s. \n";
                            }
                        }
                    }

                    if (region->sea(poix, poiy) == 1)
                    {
                        if (region->volcano(poix, poiy))
                            infotext2 = infotext2 + "Submarine volcano. ";

                        int seaice = region->seaice(poix, poiy);

                        if (seaice == 1)
                            infotext2 = infotext2 + "Seasonal sea ice.";

                        if (seaice == 2)
                            infotext2 = infotext2 + "Permanent sea ice.";

                        infotext2 = infotext2 + "\n";

                    }
                    else
                    {
                        string climate = region->climate(poix, poiy);

                        climate = getclimatename(climate) + " (" + climate + ")";

                        if (region->special(poix, poiy) == 110)
                            climate = climate + ". Salt pan";

                        if (region->special(poix, poiy) == 120)
                            climate = climate + ". Dunes";

                        if (region->special(poix, poiy) == 130)
                            climate = climate + ". Wetlands";

                        if (region->special(poix, poiy) == 131)
                            climate = climate + ". Brackish wetlands";

                        if (region->special(poix, poiy) == 132)
                            climate = climate + ". Salt wetlands";

                        if (region->rivervalley(poix, poiy) == 1)
                            climate = climate + ". River valley";

                        if (region->special(poix, poiy) == 140)
                            climate = climate + ". Glacier";

                        if (region->volcano(poix, poiy))
                            climate = climate + ". Volcano";

                        infotext2 = infotext2 + "Climate: " + climate + ".\n";
                    }
                }
                else
                {
                    if (focused == 1)
                    {
                        focused = 0;
                        poix = -1;
                        poiy = -1;
                        infotext2 = "";
                    }
                }

                int minipoix = (mousepos.x - minimapxpos) * 4;
                int minipoiy = (mousepos.y - minimapypos) * 4;

                if (minipoix>=0 && minipoiy<=world->width() && minipoiy>=0 && minipoiy<=world->height())// If the minimap has been clicked on.
                {
                    generatingnewregion = 2;
                    newx = minipoix;
                    newy = minipoiy;
                }
            }
        }

        // Area export screen

        if (screenmode == exportareascreen)
        {
            showwarning = 0;
            
            // Main controls.

            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 10, main_viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(160, 431), ImGuiCond_FirstUseEver);

            ImGui::Begin("Export custom area");

            ImGui::PushItemWidth(100.0f);

            if (standardbutton("Export maps"))
            {
                if (areanex != -1)
                {
                    ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".png", ".");

                    exportingareamaps = 1;
                }
            }

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
            ImGui::SetNextWindowSize(ImVec2(1023, 140), ImGuiCond_FirstUseEver);
            string title = "            ";

            ImGui::Begin(title.c_str());
            ImGui::PushItemWidth(world->width() / 2);
            ImGui::Text("This screen allows you to export maps at the same scale as the regional map, but of larger areas. Click on the map to pick the corners of the area you want to export. You can re-select corners to fine-tune the area. When you are done, click on 'export maps'. The program will create the maps and then ask you to specify the filename under which to save them.", world->width() / 2);
            ImGui::End();
#
            // Now check to see if the map has been clicked on.

            if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && io.WantCaptureMouse == 0)
            {
                sf::Vector2i mousepos = sf::Mouse::getPosition(window);

                poix = (mousepos.x - globalmapxpos) * 2;
                poiy = (mousepos.y - globalmapypos) * 2;

                if (poix >= 0 && poix <= world->width() && poiy >= 0 && poiy <= world->height())
                {
                    if (areaswx == -1) // If we don't have any corners yet
                    {
                        areanex = poix+2;
                        areaney = poiy;
                        areasex = poix+2;
                        areasey = poiy+2;
                        areaswx = poix;
                        areaswy = poiy+2;
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

                        float nedist = sqrt(nedistx * nedistx + nedisty * nedisty);
                        float sedist = sqrt(sedistx * sedistx + sedisty * sedisty);
                        float swdist = sqrt(swdistx * swdistx + swdisty * swdisty);
                        float nwdist = sqrt(nwdistx * nwdistx + nwdisty * nwdisty);

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

        // Import world screen

        if (screenmode == importscreen)
        {
            // Main controls.

            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 10, main_viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(160, 431), ImGuiCond_FirstUseEver);

            ImGui::Begin("Import");

            ImGui::Text("Controls:");

            ImGui::PushItemWidth(100.0f);

            if (standardbutton("Cancel"))
            {
                brandnew = 1;
                seedentry = 0;

                screenmode = createworldscreen;
            }

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Return to the world creation screen.");

            ImGui::Dummy(ImVec2(0.0f, linespace));;

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
                ImGui::SetTooltip("As for mountains. Blue=0 for shield volcano, or higher for stratovolcano. Green=0 for extinct, or higher for active.");

            ImGui::Dummy(ImVec2(0.0f, linespace));

            ImGui::SetNextItemWidth(0);

            ImGui::Text("Generate:");

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

                int grain = 8; // Level of detail on this fractal map.
                float valuemod = 0.2;
                int v = random(3, 6);
                float valuemod2 = v;

                vector<vector<int>> seafractal(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

                createfractal(seafractal, width, height, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

                float coastalvarreduce = maxelev / 3000;
                float oceanvarreduce = maxelev / 1000;

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
                float valuemod = 0.2;
                int v = random(3, 6);
                float valuemod2 = v;

                vector<vector<int>> seafractal(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

                createfractal(seafractal, width, height, grain, valuemod, valuemod2, 1, maxelev, 0, 0);

                float coastalvarreduce = maxelev / 3000;
                float oceanvarreduce = maxelev / 1000;

                for (int i = 0; i <= width; i++)
                {
                    for (int j = 0; j <= height; j++)
                    {
                        if (world->sea(i, j) == 1)
                        {
                            bool shelf = 1;

                            if (world->nom(i, j) < sealevel - 300)
                                shelf = 0;

                            if (shelf == 1)
                            {
                                float var = seafractal[i][j] - maxelev / 2;
                                var = var / coastalvarreduce;

                                int newval = world->nom(i, j) + var;

                                if (newval > sealevel - 10)
                                    newval = sealevel - 10;

                                if (newval < 1)
                                    newval = 1;

                                world->setnom(i, j, newval);
                            }
                            else
                            {
                                int ii = i + width / 2;

                                if (ii > width)
                                    ii = ii - width;

                                float var = seafractal[ii][j] - maxelev / 2;
                                var = var / oceanvarreduce;

                                int newval = world->nom(i, j) + var;

                                if (newval > sealevel - 3000)
                                    newval = sealevel - 3000;

                                if (newval < 1)
                                    newval = 1;

                                world->setnom(i, j, newval);
                            }
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
                ImGui::SetTooltip("Create random depth variation across the oceans.");

            if (standardbutton("Land elevation"))
            {
                int width = world->width();
                int height = world->height();
                int maxelev = world->maxelevation();
                int sealevel = world->sealevel();

                // First, make a fractal map.

                int grain = 8; // Level of detail on this fractal map.
                float valuemod = 0.2;
                int v = random(3, 6);
                float valuemod2 = v;

                vector<vector<int>> fractal(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

                createfractal(fractal, width, height, grain, valuemod, valuemod2, 1, 12750, 0, 0);

                int fractaladd = sealevel - 2500;

                for (int i = 0; i <= width; i++)
                {
                    for (int j = 0; j <= height; j++)
                        fractal[i][j] = fractal[i][j] + fractaladd;
                }

                // Now use it to change the land heights.

                fractaladdland(*world, fractal);

                // Smooth the land.

                smoothland(*world, 2);

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
                float valuemod = 0.2;
                int v = random(3, 6);
                float valuemod2 = v;

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
                ImGui::SetTooltip("Generates random mountain ranges.");

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
                float valuemod = 0.2;
                int v = random(3, 6);
                float valuemod2 = v;

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
                ImGui::SetTooltip("Generates random ranges of hills.");

            ImGui::Dummy(ImVec2(0.0f, linespace));

            if (standardbutton("Done"))
            {
                updatereport("Generating world from imported maps:");
                updatereport("");

                // First, finish off the terrain generation.

                vector<vector<int>> mountaindrainage(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));
                vector<vector<bool>> shelves(ARRAYWIDTH, vector<bool>(ARRAYHEIGHT, 0));

                updatereport("Raising mountain bases");

                raisemountainbases(*world, mountaindrainage);

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

                updatereport("Creating roughness map");

                vector<vector<int>> roughness(ARRAYWIDTH, vector<int>(ARRAYHEIGHT, 0));

                int grain = 8; // Level of detail on this fractal map.
                float valuemod = 0.2;
                float valuemod2 = 0.6;

                createfractal(roughness, width, height, grain, valuemod, valuemod2, 1, world->maxelevation(), 0, 0);

                for (int i = 0; i <= width; i++)
                {
                    for (int j = 0; j <= height; j++)
                        world->setroughness(i, j, roughness[i][j]);
                }

                // Now do the climates.

                generateglobalclimate(*world, smalllake, largelake, landshape, mountaindrainage, shelves);
                
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

                infotext = "Welcome to a new world!";

                focused = 0;

                screenmode = movingtoglobalmapscreen;
            }

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Calculate climates, lakes, and rivers, and finish the world.");

            ImGui::End();

            // Now the text box.

            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 180, main_viewport->WorkPos.y + 542), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(1023, 140), ImGuiCond_FirstUseEver);

            string title = "            ";

            string importtext = "Use the 'import' buttons to load in your own maps. These must be 2048x1025 pixels, in .png format.\nYou need at least a land map, but the others are optional.\nCheck the tooltips for each button for more details.\n\nAfter you have imported your maps, you can use the 'generate' buttons to tweak them or to add extra features.\n\nWhen you are done, click 'Done' to finish the world.";

            ImGui::Begin(title.c_str());
            ImGui::PushItemWidth(world->width() / 2);
            ImGui::Text(importtext.c_str(), world->width() / 2);
            ImGui::End();
        }

        window.clear();

        // Colour options, if being shown

        if (showcolouroptions)
        {
            int colouralign = 360;
            
            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 306, main_viewport->WorkPos.y + 33), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(646, 484), ImGuiCond_FirstUseEver);

            ImGui::Begin("Map appearance");

            ImGui::PushItemWidth(200);

            ImGui::ColorEdit3("Shallow ocean", (float*)&oceancolour);
            ImGui::SameLine(colouralign);
            ImGui::ColorEdit3("Tundra", (float*)&tundracolour);

            ImGui::ColorEdit3("Deep ocean", (float*)&deepoceancolour);
            ImGui::SameLine(colouralign);
            ImGui::ColorEdit3("Arctic", (float*)&coldcolour);

            ImGui::ColorEdit3("Base land", (float*)&basecolour);
            ImGui::SameLine(colouralign);
            ImGui::ColorEdit3("Sea ice", (float*)&seaicecolour);

            ImGui::ColorEdit3("Grassland", (float*)&grasscolour);
            ImGui::SameLine(colouralign);
            ImGui::ColorEdit3("Glaciers", (float*)&glaciercolour);

            ImGui::ColorEdit3("Low temperate", (float*)&basetempcolour);
            ImGui::SameLine(colouralign);
            ImGui::ColorEdit3("Salt pans", (float*)&saltpancolour);

            ImGui::ColorEdit3("High temperate", (float*)&highbasecolour);
            ImGui::SameLine(colouralign);
            ImGui::ColorEdit3("Dunes", (float*)&ergcolour);

            ImGui::ColorEdit3("Low desert", (float*)&desertcolour);
            ImGui::SameLine(colouralign);
            ImGui::ColorEdit3("Wetlands", (float*)&wetlandscolour);

            ImGui::ColorEdit3("High desert", (float*)&highdesertcolour);
            ImGui::SameLine(colouralign);
            ImGui::ColorEdit3("Lakes", (float*)&lakecolour);

            ImGui::ColorEdit3("Cold desert", (float*)&colddesertcolour);
            ImGui::SameLine(colouralign);
            ImGui::ColorEdit3("Rivers", (float*)&rivercolour);

            ImGui::ColorEdit3("Mild tundra", (float*)&eqtundracolour);
            ImGui::SameLine(colouralign);
            ImGui::ColorEdit3("Highlights", (float*)&highlightcolour);

            ImGui::Text("   ");

            ImGui::PushItemWidth(200);

            ImGui::Text("Shading");
            ImGui::SameLine(colouralign);
            ImGui::Text("Other effects");

            ImGui::SetCursorPosX(20);
            ImGui::SliderFloat("On land", &shadingland, 0.0f, 1.0f, "%.2f");

            ImGui::SameLine(colouralign+20);

            const char* lightdiritems[] = { "Southeast","Southwest","Northeast","Northwest" };
            ImGui::Combo("Light", &shadingdir, lightdiritems, 4);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Controls the light source for the shading effect.");

            ImGui::SetCursorPosX(20);
            ImGui::SliderFloat("On lakes", &shadinglake, 0.0f, 1.0f, "%.2f");

            ImGui::SameLine(colouralign+20);

            const char* snowitems[] = { "Sudden","Speckled","Smooth" };
            ImGui::Combo("Snow", &snowchange, snowitems, 3);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Changes the appearance of transitions from snowy regions to non-snowy ones.");

            ImGui::SetCursorPosX(20);
            ImGui::SliderFloat("On sea", &shadingsea, 0.0f, 1.0f, "%.2f");

            ImGui::SameLine(colouralign+20);

            const char* seaiceitems[] = { "Permanent","None","All" };
            ImGui::Combo("Sea ice", &seaiceappearance, seaiceitems, 3);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Determines whether sea ice is shown.");

            ImGui::Text("Marbling");

            ImGui::SetCursorPosX(20);
            ImGui::SliderFloat("On land ", &marblingland, 0.0f, 1.0f, "%.2f");

            ImGui::SetCursorPosX(20);
            ImGui::SliderFloat("On lakes ", &marblinglake, 0.0f, 1.0f, "%.2f");

            ImGui::SameLine(colouralign);

            ImGui::Text("Controls");

            ImGui::SetCursorPosX(20);
            ImGui::SliderFloat("On sea ", &marblingsea, 0.0f, 1.0f, " % .2f");

            ImGui::SameLine(colouralign+20);

            if (ImGui::Button("Save",ImVec2(120.0f,0.0f)))
            {
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".uws", ".");

                savingsettings = 1;
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Save map appearance settings.");

            ImGui::Text("Rivers");

            ImGui::SameLine(colouralign+20);

            if (ImGui::Button("Load", ImVec2(120.0f, 0.0f)))
            {
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".uws", ".");

                loadingsettings = 1;
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Load map appearance settings.");

            ImGui::PushItemWidth(200);

            ImGui::SetCursorPosX(20);

            ImGui::InputInt("Global map", &globalriversentry);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Only rivers greater than this size will be displayed on the global relief map.");

            ImGui::SameLine(colouralign+20);

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

                drawhighlightobjects(*world, *highlightimage, highlightsize, *minihighlightimage, minihighlightsize);

                highlighttexture->loadFromImage(*highlightimage);
                highlight->setTexture(*highlighttexture);

                minihighlighttexture->loadFromImage(*minihighlightimage);
                minihighlight->setTexture(*minihighlighttexture);

                colourschanged = 1;
            }
            
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Restore the default map appearance settings.");

            ImGui::SetCursorPosX(20);

            ImGui::InputInt("Regional map", &regionalriversentry);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Only rivers greater than this size will be displayed on the regional relief map.");

            ImGui::SameLine(colouralign+20);

            if (ImGui::Button("Close", ImVec2(120.0f, 0.0f)))
            {
                showcolouroptions = 0;
            }

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Close the map appearance panel.");

            ImGui::End();
        }

        // Now draw the graphical elements.

        if (screenmode == globalmapscreen || screenmode==exportareascreen || screenmode==importscreen)
            window.draw(*globalmap);

        if (screenmode == exportareascreen) // Area selection rectangle.
        {
            if (areaswx != -1)
            {
                int sizex = (areanex - areanwx)/2;
                int sizey = (areaswy - areanwy)/2;

                arearectangle->setSize(sf::Vector2f(sizex, sizey));
                arearectangle->setOutlineColor(sf::Color(world->highlight1(), world->highlight2(), world->highlight3()));

                sf::Vector2f position;

                position.x = areanwx / 2 + globalmapxpos;
                position.y = areanwy / 2 + globalmapypos;

                arearectangle->setPosition(position);

                window.draw(*arearectangle);
            }
        }

        if (screenmode == regionalmapscreen)
        {
            window.draw(*regionalmap);

            window.draw(*minimap);

            minihighlight->setPosition(sf::Vector2f(minimapxpos + region->centrex() / 4, minimapypos + region->centrey() / 4));

            if (generatingnewregion != 1)
                window.draw(*minihighlight);
        }

        if (screenmode == globalmapscreen || screenmode == regionalmapscreen)
        {
            if (focused == 1 && generatingnewregion == 0)
                window.draw(*highlight);
        }

        // Now draw file dialogues if needed

        if (loadingworld == 1 && ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) // If we're loading in a new world
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string filepathname = ImGuiFileDialog::Instance()->GetFilePathName();
                std::string filepath = ImGuiFileDialog::Instance()->GetCurrentPath();

                if (filepathname != "")
                {
                    cout << "Loading world object." << endl;

                    loadworld(*world, filepathname);

                    for (int n = 0; n < GLOBALMAPTYPES; n++)
                        globalmapimagecreated[n] = 0;

                    mapview = relief;
                    drawglobalmapimage(mapview, *world, globalmapimagecreated, *globalelevationimage, *globaltemperatureimage, *globalprecipitationimage, *globalclimateimage, *globalriversimage, *globalreliefimage, *displayglobalelevationimage, *displayglobaltemperatureimage, *displayglobalprecipitationimage, *displayglobalclimateimage, *displayglobalriversimage, *displayglobalreliefimage);

                    globalmaptexture->loadFromImage(*displayglobalreliefimage);
                    globalmap->setTexture(*globalmaptexture);

                    focused = 0;
                    infotext = "Welcome to a new world!";

                    screenmode = globalmapscreen;
                    loadingworld = 0;
                }

            }
            ImGuiFileDialog::Instance()->Close();
        }

        if (savingworld == 1 && ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) // If we're saving a world
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string filepathname = ImGuiFileDialog::Instance()->GetFilePathName();
                std::string filepath = ImGuiFileDialog::Instance()->GetCurrentPath();

                if (filepathname != "")
                {
                    saveworld(*world, filepathname);

                    savingworld = 0;
                }
            }
            ImGuiFileDialog::Instance()->Close();
        }

        if (loadingsettings == 1 && ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) // If we're loading in new settings
        {
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

                        drawhighlightobjects(*world, *highlightimage, highlightsize, *minihighlightimage, minihighlightsize);

                        highlighttexture->loadFromImage(*highlightimage);
                        highlight->setTexture(*highlighttexture);

                        minihighlighttexture->loadFromImage(*minihighlightimage);
                        minihighlight->setTexture(*minihighlighttexture);

                        colourschanged = 1;
                        loadingsettings = 0;
                    }
                }
            }
            ImGuiFileDialog::Instance()->Close();
        }

        if (savingsettings == 1 && ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) // If we're saving a world
        {
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
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string filepathname = ImGuiFileDialog::Instance()->GetFilePathName();
                std::string filepath = ImGuiFileDialog::Instance()->GetCurrentPath();

                if (filepathname != "")
                {
                    for (int n = 0; n < 2; n++)
                    {
                        ImGui::SFML::Update(window, deltaClock.restart());
                        ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 300, main_viewport->WorkPos.y + 200), ImGuiCond_FirstUseEver);
                        ImGui::SetNextWindowSize(ImVec2(200, 50), ImGuiCond_FirstUseEver);
                        ImGui::Begin("Please wait!");
                        ImGui::Text("Generating area maps...");
                        ImGui::End();
                        window.clear();
                        ImGui::SFML::Render(window);
                        window.display();
                    }

                    int oldregionalcentrex = region->centrex();
                    int oldregionalcentrey = region->centrey();

                    mapviewenum oldmapview = mapview;

                    initialiseregion(*world, *region); // We'll do all this using the same region object as usual. We could create a new region object for it, but that seems to lead to inexplicable crashes, so we won't.

                    float regiontilewidth = REGIONALTILEWIDTH; //30;
                    float regiontileheight = REGIONALTILEHEIGHT; //30; // The width and height of the visible regional map, in tiles.

                    int regionwidth = regiontilewidth * 16;
                    int regionheight = regiontileheight * 16; // The width and height of the visible regional map, in pixels.

                    int origareanwx = areanwx; // This is because the regions we'll be making will start to the north and west of the defined area.
                    int origareanwy = areanwy;

                    int origareanex = areanex;
                    int origareaney = areaney;

                    int origareaswx = areaswx;
                    int origareaswy = areaswy;

                    areanwx = areanwx / regiontilewidth;
                    areanwx = areanwx * regiontilewidth;

                    areanwy = areanwy / regiontileheight;
                    areanwy = areanwy * regiontileheight;

                    areaswx = areanwx;
                    areaney = areanwy;

                    int woffset = (origareanwx - areanwx) * 16;
                    int noffset = (origareanwy - areanwy) * 16;

                    float areatilewidth = areanex - areanwx;
                    float areatileheight = areasey - areaney;

                    int areawidth = areatilewidth * 16;
                    int areaheight = areatileheight * 16;

                    float imageareatilewidth = origareanex - origareanwx;
                    float imageareatileheight = areasey - origareaney;

                    int imageareawidth = imageareatilewidth * 16;
                    int imageareaheight = imageareatileheight * 16;

                    float fregionswide = areatilewidth / regiontilewidth;
                    float fregionshigh = areatileheight / regiontileheight;

                    int regionswide = fregionswide;
                    int regionshigh = fregionshigh;

                    if (regionswide != fregionswide)
                        regionswide++;

                    if (regionshigh != fregionshigh)
                        regionshigh++;

                    int totalregions = regionswide * regionshigh; // This is how many regional maps we're going to have to do.

                    if (areafromregional == 1)
                        totalregions++; // Because we'll have to redo the regional map we came from.

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
                        int centrex = i * regiontilewidth + (regiontilewidth / 2) + areanwx;

                        for (int j = 0; j < regionshigh; j++)
                        {
                            int centrey = j * regiontileheight + (regiontileheight / 2) + areanwy;

                            // First, create the new region.

                            region->setcentrex(centrex);
                            region->setcentrey(centrey);

                            generateregionalmap(*world, *region, smalllake, island, *peaks, riftblob, riftblobsize, 0, smudge, smallsmudge);

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
                        generateregionalmap(*world, *region, smalllake, island, *peaks, riftblob, riftblobsize, 0, smudge, smallsmudge);

                    // Now just save the images.

                    filepathname.resize(filepathname.size() - 4);

                    areareliefimage->saveToFile(filepathname + " Relief.png");
                    areaelevationimage->saveToFile(filepathname + " Elevation.png");
                    areatemperatureimage->saveToFile(filepathname + " Temperature.png");
                    areaprecipitationimage->saveToFile(filepathname + " Precipitation.png");
                    areaclimateimage->saveToFile(filepathname + " Climate.png");
                    areariversimage->saveToFile(filepathname + " Rivers.png");

                    // Clean up.

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

                    if (areafromregional == 1)
                        screenmode = regionalmapscreen;
                    else
                        screenmode = globalmapscreen;
                }
            }
            ImGuiFileDialog::Instance()->Close();
        }

        if (importinglandmap == 1 && ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) // If we're loading in a new world
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

        if (importingseamap == 1 && ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) // If we're loading in a new world
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

        if (importingmountainsmap == 1 && ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) // If we're loading in a new world
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

                        createmountainsfromraw(*world, rawmountains);

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

        if (importingvolcanoesmap == 1 && ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) // If we're loading in a new world
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

        // Warning window

        if (showwarning == 1)
        {
            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 469, main_viewport->WorkPos.y + 202), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(330, 85), ImGuiCond_FirstUseEver);

            ImGui::Begin("Import maps?");

            ImGui::Text("This will delete the current world. Proceed?");
            ImGui::Text(" ");
            ImGui::Text(" ");

            ImGui::SameLine(100);

            if (ImGui::Button("OK"))
            {
                initialiseworld(*world);
                world->clear();

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
                        seed = seed + (random(1, 9) * pow(10, n));
                    else
                        seed = seed + (random(0, 9) * pow(10, n));
                }

                seed = 0 - seed;

                world->setseed(seed);

                screenmode = importscreen;
                showwarning = 0;
            }

            ImGui::SameLine(180);

            if (ImGui::Button("Cancel"))
            {
                showwarning = 0;
            }

            ImGui::End();
        }

        ImGui::SFML::Render(window);
        window.display();

        if (generatingnewregion==2) // This is where we generate a new region.
        {
            ImGui::SFML::Update(window, deltaClock.restart());
            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 849, main_viewport->WorkPos.y + 364), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(200, 50), ImGuiCond_FirstUseEver);
            ImGui::Begin("Please wait...");
            ImGui::Text("Generating region");
            ImGui::End();
            ImGui::SFML::Render(window);
            window.display();

            infotext2 = "";

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

            float progressstep = 1.0 / REGIONALCREATIONSTEPS;

            // Blank the regional map image first

            for (int i = 0; i < regionalimagewidth; i++)
            {
                for (int j = 0; j < regionalimageheight; j++)
                    regionalreliefimage->setPixel(i, j, sf::Color::Black);

            }

            regionalmaptexture->loadFromImage(*regionalreliefimage);
            regionalmap->setTexture(*regionalmaptexture);

            // Now generate the regional map

            generateregionalmap(*world, *region, smalllake, island, *peaks, riftblob, riftblobsize, 0, smudge, smallsmudge);

            // Now draw the regional map image

            drawregionalmapimage(mapview, *world, *region, regionalmapimagecreated, *regionalelevationimage, *regionaltemperatureimage, *regionalprecipitationimage, *regionalclimateimage, *regionalriversimage, *regionalreliefimage);

            regionalmaptexture->loadFromImage(*regionalreliefimage);
            regionalmap->setTexture(*regionalmaptexture);

            // Sort out the minimap

            globalmaptexture->loadFromImage(*displayglobalreliefimage);
            minimap->setTexture(*globalmaptexture);

            focused = 0;
            generatingnewregion = 0;
        }

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

        if (world->ocean1() != oceancolour.x*255.f || world->ocean2() != oceancolour.y*255.f || world->ocean3() != oceancolour.z*255.f)
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

        if (world->highlight1() != highlightcolour.x * 255.f || world->highlight2() != highlightcolour.y * 255.f || world->highlight3() != highlightcolour.z * 255.f)
        {
            // This one's different as it doesn't involve redrawing the map - just redrawing the highlight sprites.

            world->sethighlight1(highlightcolour.x * 255.f);
            world->sethighlight2(highlightcolour.y * 255.f);
            world->sethighlight3(highlightcolour.z * 255.f);
            
            drawhighlightobjects(*world, *highlightimage, highlightsize, *minihighlightimage, minihighlightsize);

            highlighttexture->loadFromImage(*highlightimage);
            highlight->setTexture(*highlighttexture);

            minihighlighttexture->loadFromImage(*minihighlightimage);
            minihighlight->setTexture(*minihighlighttexture);

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

        if (world->shadingdir() != shadingdircorrected || world->snowchange() != snowchange+1 || world->seaiceappearance() != seaiceappearance+1)
            colourschanged = 1;

        if (showcolouroptions==1 && colourschanged == 1)
        {
            world->setocean1(oceancolour.x*255.f);
            world->setocean2(oceancolour.y*255.f);
            world->setocean3(oceancolour.z*255.f);

            world->setdeepocean1(deepoceancolour.x * 255.f);
            world->setdeepocean2(deepoceancolour.y * 255.f);
            world->setdeepocean3(deepoceancolour.z * 255.f);

            world->setbase1(basecolour.x * 255.f);
            world->setbase2(basecolour.y * 255.f);
            world->setbase3(basecolour.z * 255.f);

            world->setgrass1(grasscolour.x * 255.f);
            world->setgrass2(grasscolour.y * 255.f);
            world->setgrass3(grasscolour.z * 255.f);

            world->setbasetemp1(basetempcolour.x * 255.f);
            world->setbasetemp2(basetempcolour.y * 255.f);
            world->setbasetemp3(basetempcolour.z * 255.f);

            world->sethighbase1(highbasecolour.x * 255.f);
            world->sethighbase2(highbasecolour.y * 255.f);
            world->sethighbase3(highbasecolour.z * 255.f);

            world->setdesert1(desertcolour.x * 255.f);
            world->setdesert2(desertcolour.y * 255.f);
            world->setdesert3(desertcolour.z * 255.f);

            world->sethighdesert1(highdesertcolour.x * 255.f);
            world->sethighdesert2(highdesertcolour.y * 255.f);
            world->sethighdesert3(highdesertcolour.z * 255.f);

            world->setcolddesert1(colddesertcolour.x * 255.f);
            world->setcolddesert2(colddesertcolour.y * 255.f);
            world->setcolddesert3(colddesertcolour.z * 255.f);

            world->seteqtundra1(eqtundracolour.x * 255.f);
            world->seteqtundra2(eqtundracolour.y * 255.f);
            world->seteqtundra3(eqtundracolour.z * 255.f);

            world->settundra1(tundracolour.x * 255.f);
            world->settundra2(tundracolour.y * 255.f);
            world->settundra3(tundracolour.z * 255.f);

            world->setcold1(coldcolour.x * 255.f);
            world->setcold2(coldcolour.y * 255.f);
            world->setcold3(coldcolour.z * 255.f);

            world->setseaice1(seaicecolour.x * 255.f);
            world->setseaice2(seaicecolour.y * 255.f);
            world->setseaice3(seaicecolour.z * 255.f);

            world->setglacier1(glaciercolour.x * 255.f);
            world->setglacier2(glaciercolour.y * 255.f);
            world->setglacier3(glaciercolour.z * 255.f);

            world->setsaltpan1(saltpancolour.x * 255.f);
            world->setsaltpan2(saltpancolour.y * 255.f);
            world->setsaltpan3(saltpancolour.z * 255.f);

            world->seterg1(ergcolour.x * 255.f);
            world->seterg2(ergcolour.y * 255.f);
            world->seterg3(ergcolour.z * 255.f);

            world->setwetlands1(wetlandscolour.x * 255.f);
            world->setwetlands2(wetlandscolour.y * 255.f);
            world->setwetlands3(wetlandscolour.z * 255.f);

            world->setlake1(lakecolour.x * 255.f);
            world->setlake2(lakecolour.y * 255.f);
            world->setlake3(lakecolour.z * 255.f);

            world->setriver1(rivercolour.x * 255.f);
            world->setriver2(rivercolour.y * 255.f);
            world->setriver3(rivercolour.z * 255.f);

            world->setlandshading(shadingland);
            world->setlakeshading(shadinglake);
            world->setseashading(shadingsea);

            world->setlandmarbling(marblingland);
            world->setlakemarbling(marblinglake);
            world->setseamarbling(marblingsea);

            world->setminriverflowglobal(globalriversentry);
            world->setminriverflowregional(regionalriversentry);

            world->setshadingdir(shadingdircorrected);
            world->setsnowchange(snowchange+1);
            world->setseaiceappearance(seaiceappearance+1);

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

// This draws the highlight objects.

void drawhighlightobjects(planet &world, sf::Image &highlightimage, int highlightsize, sf::Image &minihighlightimage, int minihighlightsize)
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

    for (int i = 1; i < minihighlightsize - 1; i++)
    {
        for (int j = 1; j < minihighlightsize - 1; j++)
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

            if (world.lakesurface(i, j) != 0 && world.special(i, j) == 0)
                heightpoint = world.lakesurface(i, j);

            colour1 = heightpoint / div;

            if (colour1 > 255)
                colour1 = 255;

            colour2 = colour1;
            colour3 = colour2;

            globalelevationimage.setPixel(i, j, sf::Color(colour1, colour2, colour3));
        }
    }

    int mapdiv = (width + 1) / DISPLAYMAPSIZEX;

    for (int i = 0; i < DISPLAYMAPSIZEX; i++)
    {
        for (int j = 0; j < DISPLAYMAPSIZEY; j++)
        {
            sf::Color thispixel = globalelevationimage.getPixel(i * mapdiv, j * mapdiv);

            displayglobalelevationimage.setPixel(i, j, thispixel);
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
                int temperature = (world.mintemp(i, j) + world.maxtemp(i, j)) / 2;

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

    int mapdiv = (width + 1) / DISPLAYMAPSIZEX;

    for (int i = 0; i < DISPLAYMAPSIZEX; i++)
    {
        for (int j = 0; j < DISPLAYMAPSIZEY; j++)
        {
            int sourcei = i * mapdiv;
            int sourcej = j * mapdiv;
            
            sf::Color thispixel = globaltemperatureimage.getPixel(sourcei, sourcej);

            displayglobaltemperatureimage.setPixel(i, j, thispixel);

            // If there's an outline pixel nearby, use that instead.

            if (sourcei > 0 && sourcej > 0)
            {
                for (int k = sourcei - 1; k <= sourcei; k++)
                {
                    for (int l = sourcej - 1; l <= sourcej; l++)
                    {
                        sf::Color newpixel = globaltemperatureimage.getPixel(k, l);

                        if (newpixel.r == 0 && newpixel.g == 0 && newpixel.b == 0)
                            displayglobaltemperatureimage.setPixel(i, j, newpixel);
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

    int mapdiv = (width + 1) / DISPLAYMAPSIZEX;

    for (int i = 0; i < DISPLAYMAPSIZEX; i++)
    {
        for (int j = 0; j < DISPLAYMAPSIZEY; j++)
        {
            int sourcei = i * mapdiv;
            int sourcej = j * mapdiv;

            sf::Color thispixel = globalprecipitationimage.getPixel(sourcei, sourcej);

            displayglobalprecipitationimage.setPixel(i, j, thispixel);

            // If there's an outline pixel nearby, use that instead.

            if (sourcei > 0 && sourcej > 0)
            {
                for (int k = sourcei - 1; k <= sourcei; k++)
                {
                    for (int l = sourcej - 1; l <= sourcej; l++)
                    {
                        sf::Color newpixel = globalprecipitationimage.getPixel(k, l);

                        if (newpixel.r == 0 && newpixel.g == 0 && newpixel.b == 0)
                            displayglobalprecipitationimage.setPixel(i, j, newpixel);
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

    int mapdiv = (width + 1) / DISPLAYMAPSIZEX;

    for (int i = 0; i < DISPLAYMAPSIZEX; i++)
    {
        for (int j = 0; j < DISPLAYMAPSIZEY; j++)
        {
            int sourcei = i * mapdiv;
            int sourcej = j * mapdiv;

            sf::Color thispixel = globalclimateimage.getPixel(sourcei, sourcej);

            displayglobalclimateimage.setPixel(i, j, thispixel);
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

    int mapdiv = (width + 1) / DISPLAYMAPSIZEX;

    for (int i = 0; i < DISPLAYMAPSIZEX; i++)
    {
        for (int j = 0; j < DISPLAYMAPSIZEY; j++)
        {
            int sourcei = i * mapdiv;
            int sourcej = j * mapdiv;

            sf::Color thispixel = globalriversimage.getPixel(sourcei, sourcej);

            displayglobalriversimage.setPixel(i, j, thispixel);

            // Check for the largest river pixel nearby.

            if (sourcei > 0 && sourcej > 0)
            {
                int lowestred = 255;
                
                for (int k = sourcei - 1; k <= sourcei; k++)
                {
                    for (int l = sourcej - 1; l <= sourcej; l++)
                    {
                        sf::Color newpixel = globalriversimage.getPixel(k, l);

                        if (newpixel.r < lowestred && newpixel.r==newpixel.g)
                            lowestred = newpixel.r;
                    }
                }

                if (lowestred < 255)
                    displayglobalriversimage.setPixel(i, j, sf::Color(lowestred, lowestred, 255));
            }

            // If there's an outline pixel nearby, use that instead.

            if (sourcei > 0 && sourcej > 0)
            {
                for (int k = sourcei - 1; k <= sourcei; k++)
                {
                    for (int l = sourcej - 1; l <= sourcej; l++)
                    {
                        sf::Color newpixel = globalriversimage.getPixel(k, l);

                        if (newpixel.r == 0 && newpixel.g == 0 && newpixel.b == 0)
                            displayglobalriversimage.setPixel(i, j, newpixel);
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

                        float winterrain = world.winterrain(i, j);
                        float summerrain = world.summerrain(i, j);

                        float totalrain = winterrain + summerrain;

                        float monsoon = 0;

                        if (winterrain < 1)
                            winterrain = 1;

                        if (winterrain < summerrain)
                        {
                            monsoon = summerrain - winterrain;

                            monsoon = monsoon / 1000; // 410

                            if (monsoon > 0.99)
                                monsoon = 0.99;
                        }

                        // The closer it is to tropical rainforest, the more we intensify the rain effect.

                        float rainforestmult = world.mintemp(i, j) / 18.0; //9.0;

                        rainforestmult = rainforestmult * world.winterrain(i, j) / 80.0;

                        if (rainforestmult < 1)
                            rainforestmult = 1;

                        totalrain = totalrain * rainforestmult;

                        // Now adjust the colours for height.

                        int mapelev = world.map(i, j) - sealevel;

                        int newbase1, newbase2, newbase3, newgrass1, newgrass2, newgrass3;

                        if (mapelev > 2000)
                        {
                            newdesert1 = world.highdesert1();
                            newdesert2 = world.highdesert2();
                            newdesert3 = world.highdesert3();
                        }
                        else
                        {
                            int highno = mapelev / 50;
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

                        if (totalrain > 800) // 800
                        {
                            colour1 = newbase1;
                            colour2 = newbase2;
                            colour3 = newbase3;
                        }
                        else
                        { // That's it!
                            if (totalrain > 200) //400
                            {
                                int wetno = (totalrain - 200) / 40; //400 20
                                if (wetno > 20)
                                    wetno = 20;
                                int dryno = 20 - wetno;

                                colour1 = (wetno * newbase1 + dryno * newgrass1) / 20;
                                colour2 = (wetno * newbase2 + dryno * newgrass2) / 20;
                                colour3 = (wetno * newbase3 + dryno * newgrass3) / 20;
                            }
                            else
                            {
                                float ftotalrain = 200 - totalrain; // 400

                                ftotalrain = ftotalrain / 200.0; // 400

                                int powamount = totalrain - 150; // 350 This is to make a smoother transition.

                                if (powamount < 3)
                                    powamount = 3;

                                ftotalrain = pow(ftotalrain, powamount);

                                ftotalrain = ftotalrain * 200; // 400

                                totalrain = 200 - ftotalrain; // 400

                                int wetno = totalrain;
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

                            int lat = world.latitude(i, j);
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
                    int slope1, slope2;

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
                            totalslope = totalslope * seashading * 2;
                        else
                        {
                            if (world.truelake(i, j) == 1)
                                totalslope = totalslope * lakeshading * 2;
                            else
                                totalslope = totalslope * landshading * 2;

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

            globalreliefimage.setPixel(i, j, sf::Color(r, g, b));
        }
    }

    int mapdiv = (width + 1) / DISPLAYMAPSIZEX;

    for (int i = 0; i < DISPLAYMAPSIZEX; i++)
    {
        for (int j = 0; j < DISPLAYMAPSIZEY; j++)
        {
            int sourcei = i * mapdiv;
            int sourcej = j * mapdiv;

            sf::Color thispixel = globalreliefimage.getPixel(sourcei, sourcej);

            displayglobalreliefimage.setPixel(i, j, thispixel);
        }
    }
}

// This function gets colours for drawing climate maps.

sf::Color getclimatecolours(string climate)
{
    int colour1 = 0;
    int colour2 = 0;
    int colour3 = 0;

    if (climate == "Af")
    {
        colour1 = 0;
        colour2 = 0;
        colour3 = 254;
    }

    if (climate == "Am")
    {
        colour1 = 1;
        colour2 = 119;
        colour3 = 255;
    }

    if (climate == "Aw")
    {
        colour1 = 70;
        colour2 = 169;
        colour3 = 250;
    }

    if (climate == "As")
    {
        colour1 = 70;
        colour2 = 169;
        colour3 = 250;
    }

    if (climate == "BWh")
    {
        colour1 = 249;
        colour2 = 15;
        colour3 = 0;
    }

    if (climate == "BWk")
    {
        colour1 = 251;
        colour2 = 150;
        colour3 = 149;
    }

    if (climate == "BSh")
    {
        colour1 = 245;
        colour2 = 163;
        colour3 = 1;
    }

    if (climate == "BSk")
    {
        colour1 = 254;
        colour2 = 219;
        colour3 = 99;
    }

    if (climate == "Csa")
    {
        colour1 = 255;
        colour2 = 255;
        colour3 = 0;
    }

    if (climate == "Csb")
    {
        colour1 = 198;
        colour2 = 199;
        colour3 = 1;
    }

    if (climate == "Csc")
    {
        colour1 = 184;
        colour2 = 184;
        colour3 = 114;
    }

    if (climate == "Cwa")
    {
        colour1 = 138;
        colour2 = 255;
        colour3 = 162;
    }

    if (climate == "Cwb")
    {
        colour1 = 86;
        colour2 = 199;
        colour3 = 112;
    }

    if (climate == "Cwc")
    {
        colour1 = 30;
        colour2 = 150;
        colour3 = 66;
    }

    if (climate == "Cfa")
    {
        colour1 = 192;
        colour2 = 254;
        colour3 = 109;
    }

    if (climate == "Cfb")
    {
        colour1 = 76;
        colour2 = 255;
        colour3 = 93;
    }

    if (climate == "Cfc")
    {
        colour1 = 19;
        colour2 = 203;
        colour3 = 74;
    }

    if (climate == "Dsa")
    {
        colour1 = 255;
        colour2 = 8;
        colour3 = 245;
    }

    if (climate == "Dsb")
    {
        colour1 = 204;
        colour2 = 3;
        colour3 = 192;
    }

    if (climate == "Dsc")
    {
        colour1 = 154;
        colour2 = 51;
        colour3 = 144;
    }

    if (climate == "Dsd")
    {
        colour1 = 153;
        colour2 = 100;
        colour3 = 146;
    }

    if (climate == "Dwa")
    {
        colour1 = 172;
        colour2 = 178;
        colour3 = 249;
    }

    if (climate == "Dwb")
    {
        colour1 = 91;
        colour2 = 121;
        colour3 = 213;
    }

    if (climate == "Dwc")
    {
        colour1 = 78;
        colour2 = 83;
        colour3 = 175;
    }

    if (climate == "Dwd")
    {
        colour1 = 54;
        colour2 = 3;
        colour3 = 130;
    }

    if (climate == "Dfa")
    {
        colour1 = 0;
        colour2 = 255;
        colour3 = 245;
    }

    if (climate == "Dfb")
    {
        colour1 = 32;
        colour2 = 200;
        colour3 = 250;
    }

    if (climate == "Dfc")
    {
        colour1 = 0;
        colour2 = 126;
        colour3 = 125;
    }

    if (climate == "Dfd")
    {
        colour1 = 0;
        colour2 = 69;
        colour3 = 92;
    }

    if (climate == "ET")
    {
        colour1 = 178;
        colour2 = 178;
        colour3 = 178;
    }

    if (climate == "EF")
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

    //cout << regwidthbegin << ", " << regheightbegin << endl;
    //cout << regwidthend << ", " << regheightend << endl;

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

                if (flow > 0 && region.sea(i, j) == 0)
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

    //int regwidthbegin=region.regwidthbegin();

    int leftx = region.leftx();
    int lefty = region.lefty();

    int rwidth = region.rwidth();
    int rheight = region.rheight();

    int sealevel = world.sealevel();
    int minriverflow = world.minriverflowregional(); // Rivers of this size or larger will be shown on the map.
    int shadingdir = world.shadingdir();

    float landshading = world.landshading();
    float lakeshading = world.lakeshading();
    float seashading = world.seashading();

    float landmarbling = world.landmarbling() * 2;
    float lakemarbling = world.lakemarbling() * 2;
    float seamarbling = world.seamarbling() * 2;

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

    int coords[4][2];

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

    for (int x = xleft; x <= xright; x++)
    {
        int xx = leftx + x;

        if (xx<0 || xx>width)
            xx = wrap(xx, width);

        for (int y = ytop; y <= ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy = lefty + y;

            float valuemod = 0.04; //0.02;

            makegenerictile(world, x * 16, y * 16, xx, yy, valuemod, coords, source, stripefractal, 100000, 0, 0);
        }
    }

    for (int i = 0; i < RARRAYWIDTH; i++)
    {
        for (int j = 0; j < RARRAYHEIGHT; j++)
        {
            reliefmap1[i][j] = 0;
            reliefmap2[i][j] = 0;
            reliefmap3[i][j] = 0;
        }
    }

    int var = 0; // Amount colours may be varied to make the map seem more speckledy.

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

                        float rainforestmult = region.mintemp(i, j) / 18.0;

                        rainforestmult = rainforestmult * region.winterrain(i, j) / 80.0; //30.0;

                        if (rainforestmult < 1)
                            rainforestmult = 1;

                        totalrain = totalrain * rainforestmult;

                        // Now adjust the colours for height.

                        int mapelev = region.map(i, j) - sealevel;

                        if (region.special(i, j) == 110 || region.special(i, j) == 120) // If this is going to be an erg or salt pan
                            mapelev = region.lakesurface(i, j) - sealevel; // Use the surface elevation of the erg or salt pan

                        int newbase1, newbase2, newbase3, newgrass1, newgrass2, newgrass3;

                        if (mapelev > 2000)
                        {
                            newdesert1 = world.highdesert1();
                            newdesert2 = world.highdesert2();
                            newdesert3 = world.highdesert3();
                        }
                        else
                        {
                            int highno = mapelev / 50;
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

                        if (region.rivervalley(i, j) == 1) // If this is a river valley, it's wetter than it would otherwise be.
                        {
                            float biggestflow = 0;

                            for (int k = i - 20; k <= i + 20; k++)
                            {
                                for (int l = j - 20; l <= j + 20; l++)
                                {
                                    if (k >= 0 && k <= rwidth && l >= 0 && l <= rheight)
                                    {
                                        if (region.riverjan(k, l) + region.riverjul(k, l) > biggestflow)
                                            biggestflow = region.riverjan(k, l) + region.riverjul(k, l);
                                    }
                                }
                            }

                            if (biggestflow == 0)
                            {
                                twointegers nearest = findclosestriverquickly(region, i, j);

                                if (nearest.x != -1)
                                    biggestflow = region.riverjan(nearest.x, nearest.y) + region.riverjul(nearest.x, nearest.y);
                            }

                            if (biggestflow > 12000)
                                biggestflow = 12000;

                            float mult = totalrain;

                            if (mult < 1)
                                mult = 1;

                            if (mult > 1000)
                                mult = 1000;


                            biggestflow = biggestflow / mult;
                            totalrain = totalrain + biggestflow;
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
                                float ftotalrain = 200 - totalrain;

                                ftotalrain = ftotalrain / 200.0;

                                int powamount = totalrain - 150; // This is to make a smoother transition.

                                if (powamount < 3)
                                    powamount = 3;

                                ftotalrain = pow(ftotalrain, powamount);

                                ftotalrain = ftotalrain * 200;

                                totalrain = 200 - ftotalrain;

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

                            int lat = world.latitude(region.centrex(), region.centrey());
                            int lat2 = 90 - lat;

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

                        // Now add sand, if need be.

                        int special = region.special(i, j);

                        if (special == 120)
                        {
                            colour1 = (colour1 * 6 + world.erg1()) / 7;
                            colour2 = (colour2 * 6 + world.erg2()) / 7;
                            colour3 = (colour3 * 6 + world.erg3()) / 7;

                            var = 10; //4;
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

                int stripevar = avetemp;

                if (stripevar > 5)
                    stripevar = 5;

                if (stripevar < 0)
                    stripevar = 0;

                if (region.special(i, j) > 100)
                    stripevar = 1;

                if (region.sea(i, j) == 1)
                    stripevar = stripevar * seamarbling;
                else
                {
                    if (region.truelake(i, j) == 1)
                        stripevar = stripevar * lakemarbling;
                    else
                        stripevar = stripevar * landmarbling;
                }

                colour1 = colour1 + randomsign(random(0, stripevar));
                colour2 = colour2 + randomsign(random(0, stripevar));
                colour2 = colour2 + randomsign(random(0, stripevar));
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

    for (int i = regwidthbegin + 1; i <= regwidthend - 1; i++) // Blur ergs and wetlands, and also river valleys..
    {
        for (int j = regheightbegin + 1; j <= regheightend - 1; j++)
        {
            int special = region.special(i, j);

            if (special == 130 || special == 131 || special == 132) // special==120 ||
            {
                if ((region.riverjan(i, j) + region.riverjul(i, j)) / 2 < minriverflow || (region.fakejan(i, j) + region.fakejul(i, j)) / 2 < minriverflow) // Don't do it to the rivers themselves.
                {
                    float colred = 0;
                    float colgreen = 0;
                    float colblue = 0;

                    for (int k = i - 1; k <= i + 1; k++)
                    {
                        for (int l = j - 1; l <= j + 1; l++)
                        {
                            colred = colred + reliefmap1[k][l];
                            colgreen = colgreen + reliefmap2[k][l];
                            colblue = colblue + reliefmap3[k][l];
                        }
                    }

                    colred = colred / 9.0;
                    colgreen = colgreen / 9.0;
                    colblue = colblue / 9.0;

                    reliefmap1[i][j] = colred;
                    reliefmap2[i][j] = colgreen;
                    reliefmap3[i][j] = colblue;
                }
            }

            if (region.rivervalley(i, j) == 1 && region.special(i, j) < 130)
            {
                if (not((region.riverjan(i, j) + region.riverjul(i, j)) / 2 >= minriverflow || (region.fakejan(i, j) + region.fakejul(i, j)) / 2 >= minriverflow))
                {
                    float colred = 0;
                    float colgreen = 0;
                    float colblue = 0;

                    float crount = 0;

                    for (int k = i - 1; k <= i + 1; k++)
                    {
                        for (int l = j - 1; l <= j + 1; l++)
                        {
                            if (region.riverjan(k, l) == 0 && region.riverjul(k, l) == 0 && region.fakejan(k, l) == 0 && region.fakejul(k, l) == 0 && region.deltajan(k, l) == 0 && region.deltajul(k, l) == 0)
                            {
                                colred = colred + reliefmap1[k][l];
                                colgreen = colgreen + reliefmap2[k][l];
                                colblue = colblue + reliefmap3[k][l];

                                crount++;
                            }
                        }
                    }

                    colred = colred / crount;
                    colgreen = colgreen / crount;
                    colblue = colblue / crount;

                    reliefmap1[i][j] = colred;
                    reliefmap2[i][j] = colgreen;
                    reliefmap3[i][j] = colblue;
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
                    int slope1, slope2;

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

                        totalslope = totalslope * thisshading * 2;

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

