//
//  main.cpp
//  Undiscovered Worlds
//
//  Created by Jonathan Hill on 22/07/2019.
//
//  Please see functions.hpp for notes.

#include <iostream>
#include <cmath>
#include <fstream>
#include <stdio.h>
//#include <unistd.h>
#include <string>
#include <chrono>
#include <thread>
#include <queue>
#include <SFML/Graphics.hpp>
#include <GLFW/glfw3.h>
#include <nanogui/nanogui.h>

#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"

#include "classes.hpp"
#include "planet.hpp"
#include "region.hpp"
#include "functions.hpp"

#define REGIONALCREATIONSTEPS 84

#define GLOBALTERRAINCREATIONSTEPS1 26
#define GLOBALTERRAINCREATIONSTEPS2 31
#define GLOBALCLIMATECREATIONSTEPS 21

#define REGIONALTILEWIDTH 32
#define REGIONALTILEHEIGHT 32

using namespace std;
using namespace nanogui;
using ImageHolder = std::unique_ptr<uint8_t[], void(*)(void*)>;

typedef uint32_t uint;

// Random number generator. From https://stackoverflow.com/questions/26237419/faster-than-rand
// It uses a global variable. But this is the only one in the program, honest!

static long g_seed;

// Used to seed the generator.
inline void fast_srand(long seed)
{
    g_seed = seed;
}

// Compute a pseudorandom integer.
// Output value in range [0, 32767]
inline int fast_rand(void)
{
    g_seed = (214013*g_seed+2531011);
    return (g_seed>>16)&0x7FFF;
}

int main(int /* argc */, char ** /* argv */)
{
    nanogui::init();
    
    // Set up the window.
    
    int scwidth=1224; //1024;
    int scheight=768;

    nanogui::Screen *screen=nullptr;

    screen=new nanogui::Screen(Vector2i(scwidth,scheight),"Undiscovered Worlds");
    
    // Now create the main world object and initialise its basic variables.
    
    planet *world=new planet;
    
    initialiseworld(*world);
    initialisemapcolours(*world);
    
    int width=world->width();
    int height=world->height();
    
    // Now do the same thing for the region object.
    
    region *region=new class region;
    
    initialiseregion(*world,*region);
    
    int regionalmapimagewidth=region->regwidthend()-region->regwidthbegin()+1;
    int regionalmapimageheight=region->regheightend()-region->regheightbegin()+1;
    
    // Now create some empty template images for the global and regional maps, and save them to disk to load back in as stbi_uc objects. (I know this is an inefficient way of setting up the stbi_uc objects but it's the simplest I can think of!)
    
    sf::Image *globaltemplate=new sf::Image;
    sf::Image *regionaltemplate=new sf::Image;
    
    globaltemplate->create(2048,1026,sf::Color(0,0,0));
    regionaltemplate->create(514,514,sf::Color(0,0,0));
    
    bool const ret1(globaltemplate->saveToFile("Blankworld.tga"));
    if (!ret1) {cerr << "Error writing Blankworld.tga" << endl;}
    bool const ret2(regionaltemplate->saveToFile("Blankregion.tga"));
    if (!ret2) {cerr << "Error writing Blankregion.tga" << endl;}
    
    delete globaltemplate;
    delete regionaltemplate;
    
    // Now we must create the images for the different global maps.
    // These are simple, one-dimensional arrays, with four elements per pixel.

    int globalimagewidth, globalimageheight, globalimagechannels;
    
    stbi_uc *globalelevationimage=new stbi_uc;
    stbi_uc *globalwindsimage=new stbi_uc;
    stbi_uc *globaltemperatureimage=new stbi_uc;
    stbi_uc *globalprecipitationimage=new stbi_uc;
    stbi_uc *globalclimateimage=new stbi_uc;
    stbi_uc *globalriversimage=new stbi_uc;
    stbi_uc *globalreliefimage=new stbi_uc;
    
    globalelevationimage=stbi_load("Blankworld.tga",&globalimagewidth,&globalimageheight,&globalimagechannels,STBI_rgb_alpha);
    globalwindsimage=stbi_load("Blankworld.tga",&globalimagewidth,&globalimageheight,&globalimagechannels,STBI_rgb_alpha);
    globaltemperatureimage=stbi_load("Blankworld.tga",&globalimagewidth,&globalimageheight,&globalimagechannels,STBI_rgb_alpha);
    globalprecipitationimage=stbi_load("Blankworld.tga",&globalimagewidth,&globalimageheight,&globalimagechannels,STBI_rgb_alpha);
    globalclimateimage=stbi_load("Blankworld.tga",&globalimagewidth,&globalimageheight,&globalimagechannels,STBI_rgb_alpha);
    globalriversimage=stbi_load("Blankworld.tga",&globalimagewidth,&globalimageheight,&globalimagechannels,STBI_rgb_alpha);
    globalreliefimage=stbi_load("Blankworld.tga",&globalimagewidth,&globalimageheight,&globalimagechannels,STBI_rgb_alpha);
    
    int globalimagesize=globalimagewidth*globalimageheight*globalimagechannels;
    
    // Now we must create the images for the different regional maps. Same thing again.
    
    int regionalimagewidth, regionalimageheight, regionalimagechannels;
    
    stbi_uc *regionalelevationimage=new stbi_uc;
    stbi_uc *regionaltemperatureimage=new stbi_uc;
    stbi_uc *regionalprecipitationimage=new stbi_uc;
    stbi_uc *regionalclimateimage=new stbi_uc;
    stbi_uc *regionalriversimage=new stbi_uc;
    stbi_uc *regionalreliefimage=new stbi_uc;
    
    regionalelevationimage=stbi_load("Blankregion.tga",&regionalimagewidth,&regionalimageheight,&regionalimagechannels,STBI_rgb_alpha);
    regionaltemperatureimage=stbi_load("Blankregion.tga",&regionalimagewidth,&regionalimageheight,&regionalimagechannels,STBI_rgb_alpha);
    regionalprecipitationimage=stbi_load("Blankregion.tga",&regionalimagewidth,&regionalimageheight,&regionalimagechannels,STBI_rgb_alpha);
    regionalclimateimage=stbi_load("Blankregion.tga",&regionalimagewidth,&regionalimageheight,&regionalimagechannels,STBI_rgb_alpha);
    regionalriversimage=stbi_load("Blankregion.tga",&regionalimagewidth,&regionalimageheight,&regionalimagechannels,STBI_rgb_alpha);
    regionalreliefimage=stbi_load("Blankregion.tga",&regionalimagewidth,&regionalimageheight,&regionalimagechannels,STBI_rgb_alpha);
    
    int regionalimagesize=regionalimagewidth*regionalimageheight*regionalimagechannels;
    
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

    peaktemplate *peaks=new peaktemplate; // Peak shapes.
    
    loadpeaktemplates(*peaks);
    
    // Template for sections of rift lakes (I don't think these are used any more)
    
    int riftblobsize=40;
    
    vector<vector<float>> riftblob(riftblobsize+2,vector<float>(riftblobsize+2,riftblobsize+2));
    
    createriftblob(riftblob,riftblobsize/2);
    
    // Sort out circle templates.
    
    vector<vector<vector<int>>> circletemplates(61,vector<vector<int>>(61,vector<int>(61)));
    
    createcircletemplates(circletemplates);
    
    // Other bits

    fast_srand(time(0));
    
    screenmodeenum screenmode=createworld; // This is used to keep track of which screen mode we're in.
    mapviewenum mapview=relief; // This is used to keep track of which kind of information we want the map to show.
    
    int poix=0; // Coordinates of the point of interest, if there is one.
    int poiy=0;
    
    int areanwx=-1;
    int areanwy=-1;
    int areanex=-1;
    int areaney=-1;
    int areasex=-1;
    int areasey=-1;
    int areaswx=-1;
    int areaswy=-1; // Corners of the export area, if there is one.
    
    bool areafromregional=0; // If this is 1 then the area screen was opened from the regional map, not the global map.
    
    bool focused=0; // This will track whether we're focusing on one point or not.
    bool rfocused=0; // Same thing, for the regional map.
    
    short regionmargin=17; // The centre of the regional map can't be closer than this to the northern/southern edges of the global map.
    
    bool globalmapimagecreated[GLOBALMAPTYPES]; // This will keep track of which global map images have actually been created.
    
    bool regionalmapimagecreated[GLOBALMAPTYPES]; // This will keep track of which global map images have actually been created.
    
    for (int n=0; n<GLOBALMAPTYPES; n++)
        regionalmapimagecreated[n]=0;

    // Now set up the GUI.
    
    // First, create the widgets for creating the world.
    
    bool brandnew=1; // This means the program has just started and it's the first world to be generated.
    
    long seed=0;
    
    Window *getseedwindow=new Window(screen,"New world");
    
    getseedwindow->set_position(Vector2i(500,50));
    getseedwindow->set_layout(new GroupLayout());
    
    TextBox *seedinput=new TextBox(getseedwindow,"");
    seedinput->set_placeholder("Please enter a seed");
    seedinput->set_editable(1);
    seedinput->set_tooltip("This number will be used to calculate the new world. The same number will always yield the same world.");
    
    Widget *buttonbox=new Widget(getseedwindow);
    buttonbox->set_layout(new BoxLayout(Orientation::Horizontal,Alignment::Middle,0,6));
    
    Button *cancelbutton=new Button(buttonbox,"Load");
    Button *importbutton=new Button(buttonbox,"Import");
    Button *randombutton=new Button(buttonbox,"Random");
    Button *OKbutton=new Button(buttonbox,"OK");
    
    cancelbutton->set_tooltip("Load a world");
    importbutton->set_tooltip("Create a world from imported maps");
    randombutton->set_tooltip("Roll a random seed number");
    OKbutton->set_tooltip("Create a world from this seed number");
    
    Window *worldgenerationwindow=new Window(screen,"                                   New world                                    ");
    
    worldgenerationwindow->center(); //set_position(Vector2i(500,50));
    worldgenerationwindow->set_layout(new GroupLayout());
    
    Label *generationlabel=new Label(worldgenerationwindow,"Generating new world...");
    
    ProgressBar *worldprogress=new ProgressBar(worldgenerationwindow);

    worldgenerationwindow->set_visible(0);
    
    // Create the warning widgets.
    
    Window *warningwindow=new Window(screen,"Warning!");
    warningwindow->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Middle,10,6));
    Label *warninglabel=new Label(warningwindow,"This will delete the current world. Proceed?");
    Widget *warningwindowbuttonsbox=new Widget(warningwindow);
    warningwindowbuttonsbox->set_layout(new BoxLayout(Orientation::Horizontal,Alignment::Middle,0,6));
    Button *warningOKbutton=new Button(warningwindowbuttonsbox,"OK");
    Button *warningcancelbutton=new Button(warningwindowbuttonsbox,"Cancel");
    
    warningwindow->center();
    warningwindow->set_visible(0);
    
    // Create the map appearance widgets.
    
    Window *mapappearancewindow=new Window(screen,"Map appearance");
    
    mapappearancewindow->set_position(Vector2i(500,0));
    mapappearancewindow->set_layout(new GroupLayout());
    
    Widget *mapappearancewindowmainbox=new Widget(mapappearancewindow);
    mapappearancewindowmainbox->set_layout(new BoxLayout(Orientation::Horizontal,Alignment::Minimum,0,6));
    
    Widget *appearancebuttonsbox=new Widget(mapappearancewindowmainbox);
    appearancebuttonsbox->set_layout(new GroupLayout(10,6,14,20));
    
    Button *appearanceclosebutton=new Button(appearancebuttonsbox,"Close");
    Button *appearanceloadbutton=new Button(appearancebuttonsbox,"Load");
    Button *appearancesavebutton=new Button(appearancebuttonsbox,"Save");
    Button *appearancedefaultbutton=new Button(appearancebuttonsbox,"Default");
    
    Widget *appearanceeditingbox=new Widget(mapappearancewindowmainbox);
    appearanceeditingbox->set_layout(new GroupLayout(10,6,14,20));
    
    Widget *colourseditingbox=new Widget(appearanceeditingbox);
    GridLayout *cglayout=new GridLayout(Orientation::Vertical,4,Alignment::Middle,1,0);
    cglayout->set_spacing(10);
    colourseditingbox->set_layout(cglayout);
    
    Widget *oceaninfo=new Widget (colourseditingbox);
    oceaninfo->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Middle,0,2));
    Label *oceancolourwheellabel=new Label(oceaninfo,"Shallow ocean");
    Button *oceanbutton=new Button(oceaninfo,"                ");
    oceanbutton->set_background_color(Color (world->ocean1(),world->ocean2(),world->ocean3(),255));
    
    Widget *deepoceaninfo=new Widget (colourseditingbox);
    deepoceaninfo->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Middle,0,2));
    Label *deepoceancolourwheellabel=new Label(deepoceaninfo,"Deep ocean");
    Button *deepoceanbutton=new Button(deepoceaninfo,"                ");
    deepoceanbutton->set_background_color(Color (world->deepocean1(),world->deepocean2(),world->deepocean3(),255));
    
    Widget *baseinfo=new Widget (colourseditingbox);
    baseinfo->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Middle,0,2));
    Label *basecolourwheellabel=new Label(baseinfo,"Base land");
    Button *basebutton=new Button(baseinfo,"                ");
    basebutton->set_background_color(Color (world->base1(),world->base2(),world->base3(),255));
    
    Widget *grassinfo=new Widget (colourseditingbox);
    grassinfo->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Middle,0,2));
    Label *grasscolourwheellabel=new Label(grassinfo,"Grassland");
    Button *grassbutton=new Button(grassinfo,"                ");
    grassbutton->set_background_color(Color (world->grass1(),world->grass2(),world->grass3(),255));
    
    Widget *basetempinfo=new Widget (colourseditingbox);
    basetempinfo->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Middle,0,2));
    Label *basetempcolourwheellabel=new Label(basetempinfo,"Low temperate");
    Button *basetempbutton=new Button(basetempinfo,"                ");
    basetempbutton->set_background_color(Color (world->basetemp1(),world->basetemp2(),world->basetemp3(),255));
    
    Widget *highbaseinfo=new Widget (colourseditingbox);
    highbaseinfo->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Middle,0,2));
    Label *highbasecolourwheellabel=new Label(highbaseinfo,"High temperate");
    Button *highbasebutton=new Button(highbaseinfo,"                ");
    highbasebutton->set_background_color(Color (world->highbase1(),world->highbase2(),world->highbase3(),255));
    
    Widget *desertinfo=new Widget (colourseditingbox);
    desertinfo->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Middle,0,2));
    Label *desertcolourwheellabel=new Label(desertinfo,"Low desert");
    Button *desertbutton=new Button(desertinfo,"                ");
    desertbutton->set_background_color(Color (world->desert1(),world->desert2(),world->desert3(),255));
    
    Widget *highdesertinfo=new Widget (colourseditingbox);
    highdesertinfo->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Middle,0,2));
    Label *highdesertcolourwheellabel=new Label(highdesertinfo,"High desert");
    Button *highdesertbutton=new Button(highdesertinfo,"                ");
    highdesertbutton->set_background_color(Color (world->highdesert1(),world->highdesert2(),world->highdesert3(),255));
    
    Widget *colddesertinfo=new Widget (colourseditingbox);
    colddesertinfo->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Middle,0,2));
    Label *colddesertcolourwheellabel=new Label(colddesertinfo,"Cold desert");
    Button *colddesertbutton=new Button(colddesertinfo,"                ");
    colddesertbutton->set_background_color(Color (world->colddesert1(),world->colddesert2(),world->colddesert3(),255));
    
    Widget *eqtundrainfo=new Widget (colourseditingbox);
    eqtundrainfo->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Middle,0,2));
    Label *eqtundracolourwheellabel=new Label(eqtundrainfo,"Mild tundra");
    Button *eqtundrabutton=new Button(eqtundrainfo,"                ");
    eqtundrabutton->set_background_color(Color (world->eqtundra1(),world->eqtundra2(),world->eqtundra3(),255));
    
    Widget *tundrainfo=new Widget (colourseditingbox);
    tundrainfo->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Middle,0,2));
    Label *tundracolourwheellabel=new Label(tundrainfo,"Tundra");
    Button *tundrabutton=new Button(tundrainfo,"                ");
    tundrabutton->set_background_color(Color (world->tundra1(),world->tundra2(),world->tundra3(),255));
    
    Widget *coldinfo=new Widget (colourseditingbox);
    coldinfo->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Middle,0,2));
    Label *coldcolourwheellabel=new Label(coldinfo,"Arctic");
    Button *coldbutton=new Button(coldinfo,"                ");
    coldbutton->set_background_color(Color (world->cold1(),world->cold2(),world->cold3(),255));
    
    Widget *seaiceinfo=new Widget (colourseditingbox);
    seaiceinfo->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Middle,0,2));
    Label *seaicecolourwheellabel=new Label(seaiceinfo,"Sea ice");
    Button *seaicebutton=new Button(seaiceinfo,"                ");
    seaicebutton->set_background_color(Color (world->seaice1(),world->seaice2(),world->seaice3(),255));
    
    Widget *glacierinfo=new Widget (colourseditingbox);
    glacierinfo->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Middle,0,2));
    Label *glaciercolourwheellabel=new Label(glacierinfo,"Glaciers");
    Button *glacierbutton=new Button(glacierinfo,"                ");
    glacierbutton->set_background_color(Color (world->glacier1(),world->glacier2(),world->glacier3(),255));
    
    Widget *saltpaninfo=new Widget (colourseditingbox);
    saltpaninfo->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Middle,0,2));
    Label *saltpancolourwheellabel=new Label(saltpaninfo,"Salt pans");
    Button *saltpanbutton=new Button(saltpaninfo,"                ");
    saltpanbutton->set_background_color(Color (world->saltpan1(),world->saltpan2(),world->saltpan3(),255));
    
    Widget *erginfo=new Widget (colourseditingbox);
    erginfo->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Middle,0,2));
    Label *ergcolourwheellabel=new Label(erginfo,"Dunes");
    Button *ergbutton=new Button(erginfo,"                ");
    ergbutton->set_background_color(Color (world->erg1(),world->erg2(),world->erg3(),255));
    
    Widget *wetlandsinfo=new Widget (colourseditingbox);
    wetlandsinfo->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Middle,0,2));
    Label *wetlandscolourwheellabel=new Label(wetlandsinfo,"Wetlands");
    Button *wetlandsbutton=new Button(wetlandsinfo,"                ");
    wetlandsbutton->set_background_color(Color (world->wetlands1(),world->wetlands2(),world->wetlands3(),255));
    
    Widget *lakeinfo=new Widget (colourseditingbox);
    lakeinfo->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Middle,0,2));
    Label *lakecolourwheellabel=new Label(lakeinfo,"Lakes");
    Button *lakebutton=new Button(lakeinfo,"                ");
    lakebutton->set_background_color(Color (world->lake1(),world->lake2(),world->lake3(),255));
    
    Widget *riverinfo=new Widget (colourseditingbox);
    riverinfo->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Middle,0,2));
    Label *rivercolourwheellabel=new Label(riverinfo,"Rivers");
    Button *riverbutton=new Button(riverinfo,"                ");
    riverbutton->set_background_color(Color (world->river1(),world->river2(),world->river3(),255));
    
    Widget *highlightinfo=new Widget (colourseditingbox);
    highlightinfo->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Middle,0,2));
    Label *highlightcolourwheellabel=new Label(highlightinfo,"Highlights");
    Button *highlightbutton=new Button(highlightinfo,"                ");
    highlightbutton->set_background_color(Color (world->highlight1(),world->highlight2(),world->highlight3(),255));
    
    Widget *extraseditingbox=new Widget(appearanceeditingbox);
    extraseditingbox->set_layout(new GroupLayout(10,6,14,20));
    
    Label *shadinglabel=new Label(extraseditingbox,"Shading");
    
    Widget *shadingbox=new Widget(extraseditingbox);
    shadingbox->set_layout(new BoxLayout(Orientation::Horizontal,Alignment::Minimum,0,2));
    
    Widget *shadinglandbox=new Widget(shadingbox);
    shadinglandbox->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Middle,0,2));
    
    Label *shadinglandlabel=new Label(shadinglandbox,"On land");
    Slider *shadinglandslider = new Slider(shadinglandbox);
    shadinglandslider->set_value(world->landshading());
    shadinglandslider->set_fixed_width(120);
    
    Widget *shadinglakesbox=new Widget(shadingbox);
    shadinglakesbox->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Middle,0,2));
    
    Label *shadinglakeslabel=new Label(shadinglakesbox,"On lakes");
    Slider *shadinglakesslider = new Slider(shadinglakesbox);
    shadinglakesslider->set_value(world->lakeshading());
    shadinglakesslider->set_fixed_width(120);
    
    Widget *shadingseabox=new Widget(shadingbox);
    shadingseabox->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Middle,0,2));
    
    Label *shadingsealabel=new Label(shadingseabox,"On sea");
    Slider *shadingseaslider = new Slider(shadingseabox);
    shadingseaslider->set_value(world->seashading());
    shadingseaslider->set_fixed_width(120);
    
    Label *shadingdirectiongap=new Label(shadingbox,"        ");
    
    Widget *shadingdirectionbox=new Widget(shadingbox);
    shadingdirectionbox->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Middle,0,2));
    
    Label *shadingdirectionlabel=new Label(shadingdirectionbox,"Light source");
    ComboBox *shadingdirectionchooser=new ComboBox(shadingdirectionbox,{"Southeast","Southwest","Northeast","Northwest"});
    shadingdirectionchooser->set_font_size(16);
    
    if (world->shadingdir()==2)
        shadingdirectionchooser->set_selected_index(2);
    
    if (world->shadingdir()==4)
        shadingdirectionchooser->set_selected_index(0);
    
    if (world->shadingdir()==6)
        shadingdirectionchooser->set_selected_index(1);
    
    if (world->shadingdir()==8)
        shadingdirectionchooser->set_selected_index(3);
    
    Label *marblinglabel=new Label(extraseditingbox,"Marbling");
    
    Widget *marblingbox=new Widget(extraseditingbox);
    marblingbox->set_layout(new BoxLayout(Orientation::Horizontal,Alignment::Minimum,0,2));
    
    Widget *marblinglandbox=new Widget(marblingbox);
    marblinglandbox->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Middle,0,2));
    
    Label *marblinglandlabel=new Label(marblinglandbox,"On land");
    Slider *marblinglandslider=new Slider(marblinglandbox);
    marblinglandslider->set_value(0.5f);
    marblinglandslider->set_fixed_width(120);
    
    Widget *marblinglakesbox=new Widget(marblingbox);
    marblinglakesbox->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Middle,0,2));
    
    Label *marblinglakeslabel=new Label(marblinglakesbox,"On lakes");
    Slider *marblinglakesslider=new Slider(marblinglakesbox);
    marblinglakesslider->set_value(0.5f);
    marblinglakesslider->set_fixed_width(120);
    
    Widget *marblingseabox=new Widget(marblingbox);
    marblingseabox->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Middle,0,2));
    
    Label *marblingsealabel=new Label(marblingseabox,"On sea");
    Slider *marblingseaslider=new Slider(marblingseabox);
    marblingseaslider->set_value(0.5f);
    marblingseaslider->set_fixed_width(120);
    
    Label *snowchangegap=new Label(marblingbox,"        ");
    
    Widget *snowchangebox=new Widget(marblingbox);
    snowchangebox->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Middle,0,2));
    
    Label *snowchangelabel=new Label(snowchangebox,"Snow transition");
    ComboBox *snowchangechooser=new ComboBox(snowchangebox,{"Sudden","Speckled","Smooth"});
    snowchangechooser->set_font_size(16);
    
    Label *riverslabel=new Label(extraseditingbox,"Rivers");
    
    Widget *riverseditingbox=new Widget(extraseditingbox);
    riverseditingbox->set_layout(new BoxLayout(Orientation::Horizontal,Alignment::Minimum,0,2));
    
    Widget *riversglobaleditingbox=new Widget(riverseditingbox);
    riversglobaleditingbox->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Middle,0,2));
    
    Label *riversgloballabel=new Label(riversglobaleditingbox,"Global map");
    auto riversglobalbox=riversglobaleditingbox->add<IntBox<int>>();
    riversglobalbox->set_fixed_width(182);
    riversglobalbox->set_editable(true);
    riversglobalbox->set_value(world->minriverflowglobal());
    riversglobalbox->set_tooltip("Rivers larger than this will appear on the global relief map.");
    
    Widget *riversregionaleditingbox=new Widget(riverseditingbox);
    riversregionaleditingbox->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Middle,0,2));
    
    Label *riversregionallabel=new Label(riversregionaleditingbox,"Regional map");
    auto riversregionalbox=riversregionaleditingbox->add<IntBox<int>>();
    riversregionalbox->set_fixed_width(182);
    riversregionalbox->set_editable(true);
    riversregionalbox->set_value(world->minriverflowregional());
    riversregionalbox->set_tooltip("Rivers larger than this will appear on the regional relief map.");
    
    Label *seaicegap=new Label(riverseditingbox,"      ");
    
    Widget *seaicebox=new Widget(riverseditingbox);
    seaicebox->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Middle,0,2));
    
    Label *seaicelabel=new Label(seaicebox,"Sea ice");
    ComboBox *seaicechooser=new ComboBox(seaicebox,{"Permanent","None","All"});
    seaicechooser->set_font_size(16);
    

    mapappearancewindow->center();
    mapappearancewindow->set_visible(0); // This whole thing should be invisible to start with.

    // Create the colour picker widgets.
    
    Window *colourpickerwindow=new Window(screen,"Colour picker");
    colourpickerwindow->set_position(Vector2i(100,100));
    colourpickerwindow->set_layout(new GroupLayout());
    
    Widget *colourpickerbox=new Widget (colourpickerwindow);
    colourpickerbox->set_layout(new BoxLayout(Orientation::Horizontal,Alignment::Middle,0,20));
    
    Widget *colourpickerleftbox=new Widget (colourpickerbox);
    colourpickerleftbox->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Middle,0,10));
    
    Button *colourbutton=new Button(colourpickerleftbox,"");
    colourbutton->set_fixed_size({150,150});
    
    ColorWheel *colourwheel=new ColorWheel(colourpickerleftbox);
    colourwheel->set_fixed_size({185,185});
    
    Widget *colourpickerrightbox=new Widget(colourpickerbox);
    colourpickerrightbox->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Middle,0,100));
    
    Widget *colourpickerrgbbox=new Widget(colourpickerrightbox);
    colourpickerrgbbox->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Maximum,0,10));

    Widget *redinfo=new Widget(colourpickerrgbbox);
    redinfo->set_layout(new BoxLayout(Orientation::Horizontal,Alignment::Middle,0,2));
    Label *redlabel=new Label(redinfo,"Red:");
    auto redbox=redinfo->add<IntBox<int>>();
    redbox->set_fixed_width(50);
    redbox->set_editable(true);
    
    Widget *greeninfo=new Widget(colourpickerrgbbox);
    greeninfo->set_layout(new BoxLayout(Orientation::Horizontal,Alignment::Middle,0,2));
    Label *greenlabel=new Label(greeninfo,"Green:");
    auto greenbox=greeninfo->add<IntBox<int>>();
    greenbox->set_fixed_width(50);
    greenbox->set_editable(true);
    
    Widget *blueinfo=new Widget(colourpickerrgbbox);
    blueinfo->set_layout(new BoxLayout(Orientation::Horizontal,Alignment::Middle,0,2));
    Label *bluelabel=new Label(blueinfo,"Blue:");
    auto bluebox=blueinfo->add<IntBox<int>>();
    bluebox->set_fixed_width(50);
    bluebox->set_editable(true);
    
    Widget *colourpickerbuttonsbox=new Widget(colourpickerrightbox);
    colourpickerbuttonsbox->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Maximum,0,20));
    
    Widget *colourapplybox=new Widget(colourpickerbuttonsbox);
    colourapplybox->set_layout(new BoxLayout(Orientation::Horizontal,Alignment::Maximum,0,10));
    Label *colourapplylabel=new Label(colourapplybox,"   ");
    Button *colourapplybutton=new Button(colourapplybox,"Apply");
    Button *colourclosebutton=new Button(colourpickerbuttonsbox,"Close");
    colourapplybutton->set_font_size(24);
    colourclosebutton->set_font_size(24);
    

    colourpickerwindow->center();
    colourpickerwindow->set_visible(0); // This whole thing should be invisible to start with.
    
    // Create the world map widgets.
    
    Window *globalmapwindow=new Window(screen,"Global map window");
    
    globalmapwindow->set_position(Vector2i(0,0));
    globalmapwindow->set_layout(new GroupLayout());
    
    Widget *globalmapwindowmainbox=new Widget(globalmapwindow);
    globalmapwindowmainbox->set_layout(new BoxLayout(Orientation::Horizontal,Alignment::Middle,0,6));

    Widget *mapbuttonsbox=new Widget(globalmapwindowmainbox);
    mapbuttonsbox->set_layout(new GroupLayout(10,6,14,20));
    
    Label *worldcontrolslabel=new Label(mapbuttonsbox,"World controls:");
    Button *newworldbutton=new Button(mapbuttonsbox,"New world");
    Button *loadworldbutton=new Button(mapbuttonsbox,"Load world");
    Button *saveworldbutton=new Button(mapbuttonsbox,"Save world");
    Button *globalimportbutton=new Button(mapbuttonsbox,"Import world");
    
    Label *worldexportslabel=new Label(mapbuttonsbox,"Export options:");
    Button *exportworldmapsbutton=new Button(mapbuttonsbox,"World maps");
    Button *exportareamapsbutton=new Button(mapbuttonsbox,"Area maps");

    Label *maptypelabel=new Label(mapbuttonsbox,"Display map type:");
    Button *reliefbutton=new Button(mapbuttonsbox,"Relief");
    reliefbutton->set_flags(Button::RadioButton);
    Button *elevationbutton=new Button(mapbuttonsbox,"Elevation");
    elevationbutton->set_flags(Button::RadioButton);
    //Button *windbutton=new Button(mapbuttonsbox,"Winds");
    //windbutton->set_flags(Button::RadioButton);
    Button *temperaturebutton=new Button(mapbuttonsbox,"Temperature");
    temperaturebutton->set_flags(Button::RadioButton);
    Button *precipitationbutton=new Button(mapbuttonsbox,"Precipitation");
    precipitationbutton->set_flags(Button::RadioButton);
    Button *climatebutton=new Button(mapbuttonsbox,"Climate");
    climatebutton->set_flags(Button::RadioButton);
    Button *riversbutton=new Button(mapbuttonsbox,"Rivers");
    riversbutton->set_flags(Button::RadioButton);
    reliefbutton->set_pushed(1);

    Widget *globalmapiconbuttonsbox=new Widget(mapbuttonsbox);
    GridLayout *glayout=new GridLayout(Orientation::Horizontal,2,Alignment::Middle,1,0);
    glayout->set_spacing(4);
    globalmapiconbuttonsbox->set_layout(glayout);
    
    Button *recentrebutton=new Button(globalmapiconbuttonsbox,"",FA_RULER);
    recentrebutton->set_font_size(35);
    recentrebutton->set_fixed_width(45);
    recentrebutton->set_tooltip("Recentre the map.");
    
    Button *focusbutton=new Button(globalmapiconbuttonsbox,"",FA_MAP_MARKER_ALT); // FA_CROSSHAIRS FA_ANKH FA_EXPAND
    focusbutton->set_font_size(35);
    focusbutton->set_fixed_width(45);
    focusbutton->set_tooltip("Examine a point on the map.");
    
    Button *coloursbutton=new Button(globalmapiconbuttonsbox,"",FA_PALETTE);
    coloursbutton->set_font_size(35);
    coloursbutton->set_fixed_width(45);
    coloursbutton->set_tooltip("Change the map appearance.");
    
    Button *zoombutton=new Button(globalmapiconbuttonsbox,"", FA_SEARCH_LOCATION); //FA_EXPAND_ARROWS_ALT); // FA_EXPAND FA_BINOCULARS
    zoombutton->set_font_size(35);
    zoombutton->set_fixed_width(45);
    zoombutton->set_tooltip("Zoom into the current point.");
    
    Widget *mapandfocusbox=new Widget(globalmapwindowmainbox);
    mapandfocusbox->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Middle,0,2));

    Texture *globalmap=new Texture("Blankworld.tga",Texture::InterpolationMode::Trilinear,Texture::InterpolationMode::Nearest);
    
    ImageView *globalmapwidget=new ImageView(mapandfocusbox);
    
    Vector2i size;
    size.x()=width/2+1;
    size.y()=height/2+1;
    
    globalmapwidget->set_size(size);
    globalmapwidget->set_image(globalmap);
    globalmapwidget->set_scale(0.5);

    Label *focusinfo=new Label(mapandfocusbox,"Welcome to a new world!");
    focusinfo->set_font_size(24);
    focusinfo->set_fixed_width(1000);
    focusinfo->set_fixed_height(100);

    globalmapwindow->center();
    globalmapwindow->set_visible(0); // This whole thing should be invisible to start with.
    
    // Create the regional map widgets.
    
    Window *regionalmapwindow=new Window(screen,"Regional map");
    
    regionalmapwindow->set_position(Vector2i(0,0));
    regionalmapwindow->set_layout(new BoxLayout(Orientation::Horizontal,Alignment::Minimum,0,6));
    
    Widget *regionalmapbuttonsbox=new Widget(regionalmapwindow);
    regionalmapbuttonsbox->set_layout(new GroupLayout(10,6,14,20));

    Label *regionalcontrolslabel=new Label(regionalmapbuttonsbox,"World controls:");
    Button *regionalreturnbutton=new Button(regionalmapbuttonsbox,"World map");

    Label *regionalexportslabel=new Label(regionalmapbuttonsbox,"Export options:");
    Button *exportregionalmapsbutton=new Button(regionalmapbuttonsbox,"Regional maps");
    Button *regionalexportareamapsbutton=new Button(regionalmapbuttonsbox,"Area maps");
    
    Label *regionalmaptypelabel=new Label(regionalmapbuttonsbox,"Display map type:");
    Button *regionalreliefbutton=new Button(regionalmapbuttonsbox,"Relief");
    regionalreliefbutton->set_flags(Button::RadioButton);
    Button *regionalelevationbutton=new Button(regionalmapbuttonsbox,"Elevation");
    regionalelevationbutton->set_flags(Button::RadioButton);
    Button *regionaltemperaturebutton=new Button(regionalmapbuttonsbox,"Temperature");
    regionaltemperaturebutton->set_flags(Button::RadioButton);
    Button *regionalprecipitationbutton=new Button(regionalmapbuttonsbox,"Precipitation");
    regionalprecipitationbutton->set_flags(Button::RadioButton);
    Button *regionalclimatebutton=new Button(regionalmapbuttonsbox,"Climate");
    regionalclimatebutton->set_flags(Button::RadioButton);
    Button *regionalriverbutton=new Button(regionalmapbuttonsbox,"Rivers");
    regionalriverbutton->set_flags(Button::RadioButton);
    regionalreliefbutton->set_pushed(1);
    
    Label *regionaldummylabel1=new Label(regionalmapbuttonsbox," ");
    Label *regionaldummylabel2=new Label(regionalmapbuttonsbox," ");
    
    Widget *regionalmapiconbuttonsbox=new Widget(regionalmapbuttonsbox);
    GridLayout *rglayout=new GridLayout(Orientation::Horizontal,2,Alignment::Middle,1,0);
    rglayout->set_spacing(4);
    regionalmapiconbuttonsbox->set_layout(rglayout);
    
    Button *regionalrecentrebutton=new Button(regionalmapiconbuttonsbox,"",FA_RULER);
    regionalrecentrebutton->set_font_size(35);
    regionalrecentrebutton->set_fixed_width(45);
    regionalrecentrebutton->set_tooltip("Recentre the map.");
    
    Button *regionalfocusbutton=new Button(regionalmapiconbuttonsbox,"",FA_MAP_MARKER_ALT); // FA_CROSSHAIRS FA_ANKH FA_EXPAND
    regionalfocusbutton->set_font_size(35);
    regionalfocusbutton->set_fixed_width(45);
    regionalfocusbutton->set_tooltip("Examine a point on the map.");
    
    Button *regionalcoloursbutton=new Button(regionalmapiconbuttonsbox,"",FA_PALETTE);
    regionalcoloursbutton->set_font_size(35);
    regionalcoloursbutton->set_fixed_width(45);
    regionalcoloursbutton->set_tooltip("Change the map appearance.");
    
    Button *regionalzoombutton=new Button(regionalmapiconbuttonsbox," "); // FA_EXPAND FA_BINOCULARS
    regionalzoombutton->set_font_size(35);
    regionalzoombutton->set_fixed_width(45);
    regionalzoombutton->set_tooltip("This button intentionally left blank.");
    
    Widget *regionalmainbox=new Widget(regionalmapwindow); // All the stuff apart from the map buttons on the left.
    regionalmainbox->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Minimum,0,10));
    
    Widget *regionalmapsbox=new Widget(regionalmainbox); // All the stuff above the focus point info.
    regionalmapsbox->set_layout(new BoxLayout(Orientation::Horizontal,Alignment::Minimum,0,10));
    
    Widget *regionalmapandprogressbox=new Widget(regionalmapsbox); // The main regional map and progress bar.
    regionalmapandprogressbox->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Minimum,0,4));
    
    Texture *regionalmap=new Texture("Blankregion.tga",Texture::InterpolationMode::Trilinear,Texture::InterpolationMode::Nearest);
    
    ImageView *regionalmapwidget=new ImageView(regionalmapandprogressbox);

    Vector2i regionalsize;
    regionalsize.x()=regionalmapimagewidth;
    regionalsize.y()=regionalmapimageheight;
    
    regionalmapwidget->set_size(regionalsize);
    regionalmapwidget->set_image(regionalmap);
    
    ProgressBar *regionprogress=new ProgressBar(regionalmapandprogressbox);
    regionprogress->set_fixed_width(regionalmapimagewidth);
    
    Widget *regionalextrabitsbox=new Widget(regionalmapsbox); // Minimap and the buttons beneath it.
    regionalextrabitsbox->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Middle,0,6));
    
    ImageView *regionalminimapwidget=new ImageView(regionalextrabitsbox);

    Vector2i regionalminimapsize;
    regionalminimapsize.x()=width/4; //2;
    regionalminimapsize.y()=height/4; //2;
    
    Texture *regionalminimap=new Texture("Blankworld.tga",Texture::InterpolationMode::Trilinear,Texture::InterpolationMode::Nearest);
    
    regionalminimapwidget->set_size(regionalminimapsize);
    regionalminimapwidget->set_image(regionalminimap);
    regionalminimapwidget->set_scale(0.25); //(0.5);
    
    Widget *regionalminimapiconbuttonsbox=new Widget(regionalextrabitsbox);
    regionalminimapiconbuttonsbox->set_layout(new BoxLayout(Orientation::Horizontal,Alignment::Middle,0,6));
    
    Label *regionalminimapdummylabel=new Label(regionalminimapiconbuttonsbox,"                                   ");
    regionalminimapdummylabel->set_font_size(60);
    
    Button *regionalminimaprecentrebutton=new Button(regionalminimapiconbuttonsbox,"",FA_RULER);
    regionalminimaprecentrebutton->set_font_size(35);
    regionalminimaprecentrebutton->set_fixed_width(45);
    regionalminimaprecentrebutton->set_tooltip("Recentre the map.");
    
    Button *regionalminimapfocusbutton=new Button(regionalminimapiconbuttonsbox,"",FA_MAP_MARKER_ALT); // FA_CROSSHAIRS FA_ANKH FA_EXPAND
    regionalminimapfocusbutton->set_font_size(35);
    regionalminimapfocusbutton->set_fixed_width(45);
    regionalminimapfocusbutton->set_tooltip("Examine a point on the map.");
    
    Widget *regionalextraiconsbox=new Widget(regionalextrabitsbox);
    regionalextraiconsbox->set_layout(new BoxLayout(Orientation::Horizontal,Alignment::Middle,0,15));
    
    Widget *regionaldirectionbuttonsbox=new Widget(regionalextraiconsbox);
    regionaldirectionbuttonsbox->set_layout(new BoxLayout(Orientation::Horizontal,Alignment::Middle,0,4));
    
    Button *regionalWbutton=new Button(regionaldirectionbuttonsbox,"",FA_ARROW_LEFT);
    regionalWbutton->set_font_size(35);
    regionalWbutton->set_fixed_width(45);
    
    Widget *regionalNSbuttonsbox=new Widget(regionaldirectionbuttonsbox);
    regionalNSbuttonsbox->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Middle,0,4));
    
    Button *regionalNbutton=new Button(regionalNSbuttonsbox,"",FA_ARROW_UP);
    regionalNbutton->set_font_size(35);
    regionalNbutton->set_fixed_width(45);
    
    Button *regionalSbutton=new Button(regionalNSbuttonsbox,"",FA_ARROW_DOWN);
    regionalSbutton->set_font_size(35);
    regionalSbutton->set_fixed_width(45);

    Button *regionalEbutton=new Button(regionaldirectionbuttonsbox,"",FA_ARROW_RIGHT);
    regionalEbutton->set_font_size(35);
    regionalEbutton->set_fixed_width(45);

    Label *regionalminimapdummylabel2=new Label(regionalextraiconsbox,"                ");
    regionalminimapdummylabel2->set_font_size(80);

    Label *regionalfocusinfo=new Label(regionalmainbox," ");
    regionalfocusinfo->set_font_size(24); //25
    regionalfocusinfo->set_fixed_width(1000);
    regionalfocusinfo->set_fixed_height(100);

    regionalmapwindow->center();
    regionalmapwindow->set_visible(0); // This whole thing should be invisible to start with.
    
    // Create the area map export widgets.
    
    Window *areaexportwindow=new Window(screen,"Custom area export");
    
    areaexportwindow->set_position(Vector2i(0,0));
    areaexportwindow->set_layout(new GroupLayout());
    
    Widget *areaexportwindowmainbox=new Widget(areaexportwindow);
    areaexportwindowmainbox->set_layout(new BoxLayout(Orientation::Horizontal,Alignment::Minimum,0,6));
    
    Widget *areaexportbuttonsbox=new Widget(areaexportwindowmainbox);
    areaexportbuttonsbox->set_layout(new GroupLayout(10,6,14,20));
    
    Label *areaexportselectionlabel=new Label(areaexportbuttonsbox,"Selection controls:");
    Button *areaexportselectbutton=new Button(areaexportbuttonsbox,"Select point");
    Button *areaexportclearbutton=new Button(areaexportbuttonsbox,"Clear points");
    Label *areaexportcontrolslabel=new Label(areaexportbuttonsbox,"Export controls:");
    Button *areaexportexportbutton=new Button(areaexportbuttonsbox,"Export maps");
    Button *areaexportcancelbutton=new Button(areaexportbuttonsbox,"Cancel");
    
    areaexportselectbutton->set_tooltip("Define a corner of the area to export.");
    areaexportclearbutton->set_tooltip("Clear the currently defined area.");
    areaexportexportbutton->set_tooltip("Export maps of the currently defined area.");
    areaexportcancelbutton->set_tooltip("Go back to the previous screen.");
    
    Widget *areaexportmapbox=new Widget(areaexportwindowmainbox);
    areaexportmapbox->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Middle,0,2));
    
    ImageView *areaexportmapwidget=new ImageView(areaexportmapbox);
    
    areaexportmapwidget->set_size(size);
    areaexportmapwidget->set_image(globalmap);
    areaexportmapwidget->set_scale(0.5);
    
    Label *areaexportinfo=new Label(areaexportmapbox,"This screen allows you to export maps at the same scale as the regional map, but of larger areas. Use the 'select point' button to pick the corners of the area you want to export. You can re-select corners to fine-tune the area. When you are done, click on 'export maps'. The program will create the maps and then ask you to specify the filename under which to save them.");
    areaexportinfo->set_font_size(24);
    areaexportinfo->set_fixed_width(1000);
    areaexportinfo->set_fixed_height(100);
    
    areaexportwindow->center();
    areaexportwindow->set_visible(0); // This whole thing should be invisible to start with.
    
    Window *areaexportprogresswindow=new Window(screen,"                                                                                                         Exporting custom maps                                                                                                          ");
    
    
    areaexportprogresswindow->set_layout(new GroupLayout());
    
    Label *areaexportprogresslabel=new Label(areaexportprogresswindow,"Please wait - generating maps...");
    ProgressBar *areaexportprogress=new ProgressBar(areaexportprogresswindow);
    
    areaexportprogresswindow->center();
    areaexportprogresswindow->set_visible(0);
    
    remove("Blankworld.tga");
    remove("Blankregion.tga");

    // Set up the world import widgets.
    
    Window *importwindow=new Window(screen,"Import maps");
    
    importwindow->set_position(Vector2i(0,0));
    importwindow->set_layout(new GroupLayout());
    
    Widget *importwindowmainbox=new Widget(importwindow);
    importwindowmainbox->set_layout(new BoxLayout(Orientation::Horizontal,Alignment::Middle,0,6));

    
    Widget *importbuttonsbox=new Widget(importwindowmainbox);
    importbuttonsbox->set_layout(new GroupLayout(10,6,14,20));
    
    Label *importcontrolslabel=new Label(importbuttonsbox,"Controls:");
    Button *importcancelbutton=new Button(importbuttonsbox,"Cancel");
    
    Label *importimportlabel=new Label(importbuttonsbox,"Import:");
    Button *importlandmapbutton=new Button(importbuttonsbox,"Land map");
    Button *importseamapbutton=new Button(importbuttonsbox,"Sea map");
    Button *importmountainsbutton=new Button(importbuttonsbox,"Mountains");
    Button *importvolcanoesbutton=new Button(importbuttonsbox,"Volcanoes");
    
    Label *importgeneratelabel=new Label(importbuttonsbox,"Generate:");
    Button *importgencoastsbutton=new Button(importbuttonsbox,"Coastlines");
    Button *importgenshelvesbutton=new Button(importbuttonsbox,"Shelves");
    Button *importgenridgesbutton=new Button(importbuttonsbox,"Oceanic ridges");
    Button *importgenseabedbutton=new Button(importbuttonsbox,"Sea bed");
    Button *importgenlandbutton=new Button(importbuttonsbox,"Land elevation");
    Button *importgenmountainsbutton=new Button(importbuttonsbox,"Mountains");
    Button *importgenhillsbutton=new Button(importbuttonsbox,"Hills");
    //Button *importgenislandsbutton=new Button(importbuttonsbox,"Islands"); // Deactivated as it takes up space and would probably not be all that useful.
    
    Widget *importmapiconbuttonsbox=new Widget(importbuttonsbox);
    importmapiconbuttonsbox->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Middle,10,6));
    
    Button *importgenclimatebutton=new Button(importmapiconbuttonsbox,"",FA_CLOUD_SUN); // FA_ANKH FA_CLOUD_SUN
    importgenclimatebutton->set_font_size(80);
    importgenclimatebutton->set_fixed_width(90);
    
    importcancelbutton->set_tooltip("Return to the world creation screen.");
    importlandmapbutton->set_tooltip("Red value of 0 is sea. Higher values are elevation above sea level, in multiples of 10 metres. Blue and green values are ignored.");
    importseamapbutton->set_tooltip("Red value of 0 is land. Higher values are depths below sea level, in multiples of 50 metres. Blue and green values are ignored.");
    importmountainsbutton->set_tooltip("Red value is peak elevation above surrounding land, in multiples of 50 metres. Blue and green values are ignored.");
    importvolcanoesbutton->set_tooltip("As for mountains. Blue=0 for shield volcano, or higher for stratovolcano. Green=0 for extinct, or higher for active.");
    importgenshelvesbutton->set_tooltip("Generates continental shelves.");
    importgenridgesbutton->set_tooltip("Generates mid-ocean ridges.");
    importgenlandbutton->set_tooltip("Generates random elevation variation across the land.");
    importgenseabedbutton->set_tooltip("Creates random depth variation across the oceans.");
    //importgenislandsbutton->set_tooltip("Generates random chains of islands.");
    importgenmountainsbutton->set_tooltip("Generates random mountain ranges.");
    importgenhillsbutton->set_tooltip("Generates random ranges of hills.");
    importgencoastsbutton->set_tooltip("Disrupts straight edges on coastlines.");
    importgenclimatebutton->set_tooltip("Calculates climates, lakes, and rivers, and finishes the world.");

    Widget *importmapbox=new Widget(importwindowmainbox);
    importmapbox->set_layout(new BoxLayout(Orientation::Vertical,Alignment::Middle,0,2));
    
    ImageView *importmapwidget=new ImageView(importmapbox);
    
    importmapwidget->set_size(size);
    importmapwidget->set_image(globalmap);
    importmapwidget->set_scale(0.5);
    
    Label *importinfo=new Label(importmapbox,"Use the 'import' buttons to load in your own maps. These must be 2048x1025 pixels, in .png format. You need at least a land map, but the others are optional. Check the tooltips for each button for more details. After you have imported your maps, you can use the 'generate' buttons to tweak them or to add extra features. When you are done, click the big button at the bottom left to finish the world.");
    importinfo->set_font_size(24);
    importinfo->set_fixed_width(1000);
    importinfo->set_fixed_height(100);
    
    importwindow->center();
    importwindow->set_visible(0); // This whole thing should be invisible to start with.

    ////
    
    /*
    globalmapwindow->set_visible(1);
    
    displaytemplates(*world,globalreliefimage,globalimagewidth,globalimagechannels,island);
    
    
    globalmap->upload(globalreliefimage);
    
    globalmapwidget->set_image(globalmap);
    */
    
    ////
    
    
    
    // Set up the callbacks for the create world widgets.
    
    cancelbutton->set_callback([&world,&screen,&getseedwindow,&brandnew,&globalmapwindow,&focusinfo,&globalmapimagecreated,&mapview,&globalelevationimage,&globalwindsimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&globalmapwidget,&region,&regionalminimap,&regionalminimapwidget,&regionalmapwindow,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&regionalmap,&regionalmapwidget,&oceanbutton,&deepoceanbutton,&basebutton,&grassbutton,&basetempbutton,&highbasebutton,&desertbutton,&highdesertbutton,&colddesertbutton,&eqtundrabutton,&tundrabutton,&coldbutton,&seaicebutton,&glacierbutton,&saltpanbutton,&ergbutton,&wetlandsbutton,&lakebutton,&riverbutton,&highlightbutton,&rfocused,&poix,&poiy,&focused,&colourpickerwindow,&colourbutton,&colourwheel,&redbox,&greenbox,&bluebox,&shadinglandslider,&shadinglakesslider,&shadingseaslider,&marblinglandslider,&marblinglakesslider,&marblingseaslider,&riversglobalbox,&riversregionalbox,&shadingdirectionchooser]
                               {
                                   if (brandnew==1) // If this is the first time we've seen this window, this is a load button.
                                   {
                                       bool found=loadworld(*world);
                                       
                                       if (found==1)
                                       {
                                           // Put the correct values into the settings.
                                           
                                           shadinglandslider->set_value(world->landshading());
                                           shadinglakesslider->set_value(world->lakeshading());
                                           shadingseaslider->set_value(world->seashading());
                                           
                                           if (world->shadingdir()==2)
                                               shadingdirectionchooser->set_selected_index(2);
                                           
                                           if (world->shadingdir()==4)
                                               shadingdirectionchooser->set_selected_index(0);
                                           
                                           if (world->shadingdir()==6)
                                               shadingdirectionchooser->set_selected_index(1);
                                           
                                           if (world->shadingdir()==8)
                                               shadingdirectionchooser->set_selected_index(3);
                                           
                                           
                                           marblinglandslider->set_value(world->landmarbling());
                                           marblinglakesslider->set_value(world->lakemarbling());
                                           marblingseaslider->set_value(world->seamarbling());
                                           
                                           riversglobalbox->set_value(world->minriverflowglobal());
                                           riversregionalbox->set_value(world->minriverflowregional());
                                           
                                           // Put the correct colours into the colour pickers.
                                           
                                           seaicebutton->set_background_color(Color(world->seaice1(),world->seaice2(),world->seaice3(),255));
                                           oceanbutton->set_background_color(Color(world->ocean1(),world->ocean2(),world->ocean3(),255));
                                           deepoceanbutton->set_background_color(Color(world->deepocean1(),world->deepocean2(),world->deepocean3(),255));
                                           basebutton->set_background_color(Color(world->base1(),world->base2(),world->base3(),255));
                                           grassbutton->set_background_color(Color(world->grass1(),world->grass2(),world->grass3(),255));
                                           basetempbutton->set_background_color(Color(world->basetemp1(),world->basetemp2(),world->basetemp3(),255));
                                           highbasebutton->set_background_color(Color(world->highbase1(),world->highbase2(),world->highbase3(),255));
                                           desertbutton->set_background_color(Color(world->desert1(),world->desert2(),world->desert3(),255));
                                           highdesertbutton->set_background_color(Color(world->highdesert1(),world->highdesert2(),world->highdesert3(),255));
                                           colddesertbutton->set_background_color(Color(world->colddesert1(),world->colddesert2(),world->colddesert3(),255));
                                           tundrabutton->set_background_color(Color(world->tundra1(),world->tundra2(),world->tundra3(),255));
                                           eqtundrabutton->set_background_color(Color(world->eqtundra1(),world->eqtundra2(),world->eqtundra3(),255));
                                           coldbutton->set_background_color(Color(world->cold1(),world->cold2(),world->cold3(),255));
                                           saltpanbutton->set_background_color(Color(world->saltpan1(),world->saltpan2(),world->saltpan3(),255));
                                           ergbutton->set_background_color(Color(world->erg1(),world->erg2(),world->erg3(),255));
                                           wetlandsbutton->set_background_color(Color(world->wetlands1(),world->wetlands2(),world->wetlands3(),255));
                                           lakebutton->set_background_color(Color(world->lake1(),world->lake2(),world->lake3(),255));
                                           riverbutton->set_background_color(Color(world->river1(),world->river2(),world->river3(),255));
                                           glacierbutton->set_background_color(Color(world->glacier1(),world->glacier2(),world->glacier3(),255));
                                           highlightbutton->set_background_color(Color(world->highlight1(),world->highlight2(),world->highlight3(),255));
                                           
                                           // Now put the correct colour into the colourpicker window.
                                           
                                           string title=colourpickerwindow->title();
                                           int red=0;
                                           int green=0;
                                           int blue=0;
                                           
                                           if (title=="Shallow ocean")
                                           {
                                               red=world->ocean1();
                                               green=world->ocean2();
                                               blue=world->ocean3();
                                           }
                                           
                                           if (title=="Deep ocean")
                                           {
                                               red=world->deepocean1();
                                               green=world->deepocean2();
                                               blue=world->deepocean3();
                                           }
                                           
                                           if (title=="Base land")
                                           {
                                               red=world->base1();
                                               green=world->base2();
                                               blue=world->base3();
                                           }
                                           
                                           if (title=="Grassland")
                                           {
                                               red=world->grass1();
                                               green=world->grass2();
                                               blue=world->grass3();
                                           }
                                           
                                           if (title=="Low temperate")
                                           {
                                               red=world->basetemp1();
                                               green=world->basetemp2();
                                               blue=world->basetemp3();
                                           }
                                           
                                           if (title=="High temperate")
                                           {
                                               red=world->highbase1();
                                               green=world->highbase2();
                                               blue=world->highbase3();
                                           }
                                           
                                           if (title=="Low desert")
                                           {
                                               red=world->desert1();
                                               green=world->desert2();
                                               blue=world->desert3();
                                           }
                                           
                                           if (title=="High desert")
                                           {
                                               red=world->highdesert1();
                                               green=world->highdesert2();
                                               blue=world->highdesert3();
                                           }
                                           
                                           if (title=="Cold desert")
                                           {
                                               red=world->colddesert1();
                                               green=world->colddesert2();
                                               blue=world->colddesert3();
                                           }
                                           
                                           if (title=="Mild tundra")
                                           {
                                               red=world->eqtundra1();
                                               green=world->eqtundra2();
                                               blue=world->eqtundra3();
                                           }
                                           
                                           if (title=="Tundra")
                                           {
                                               red=world->tundra1();
                                               green=world->tundra2();
                                               blue=world->tundra3();
                                           }
                                           
                                           if (title=="Arctic")
                                           {
                                               red=world->cold1();
                                               green=world->cold2();
                                               blue=world->cold3();
                                           }
                                           
                                           if (title=="Sea ice")
                                           {
                                               red=world->seaice1();
                                               green=world->seaice2();
                                               blue=world->seaice3();
                                           }
                                           
                                           if (title=="Glaciers")
                                           {
                                               red=world->glacier1();
                                               green=world->glacier2();
                                               blue=world->glacier3();
                                           }
                                           
                                           if (title=="Salt pans")
                                           {
                                               red=world->saltpan1();
                                               green=world->saltpan2();
                                               blue=world->saltpan3();
                                           }
                                           
                                           if (title=="Dunes")
                                           {
                                               red=world->erg1();
                                               green=world->erg2();
                                               blue=world->erg3();
                                           }
                                           
                                           if (title=="Wetlands")
                                           {
                                               red=world->wetlands1();
                                               green=world->wetlands2();
                                               blue=world->wetlands3();
                                           }
                                           
                                           if (title=="Lakes")
                                           {
                                               red=world->lake1();
                                               green=world->lake2();
                                               blue=world->lake3();
                                           }
                                           
                                           if (title=="Rivers")
                                           {
                                               red=world->river1();
                                               green=world->river2();
                                               blue=world->river3();
                                           }
                                           
                                           if (title=="Highlights")
                                           {
                                               red=world->highlight1();
                                               green=world->highlight2();
                                               blue=world->highlight3();
                                           }
                                           
                                           Color colour(red,green,blue,255);
                                           
                                           colourwheel->set_color(colour);
                                           colourbutton->set_background_color(colour);
                                           
                                           redbox->set_value(red);
                                           greenbox->set_value(green);
                                           bluebox->set_value(blue);

                                           for (int n=0; n<GLOBALMAPTYPES; n++)
                                               globalmapimagecreated[n]=0;
                                           
                                           mapview=relief;
                                           drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                           
                                           
                                           globalmap->upload(globalreliefimage);
                                           globalmapwidget->set_image(globalmap);
                                           globalmapwidget->set_offset(Vector2f(0,0));
                                           globalmapwidget->set_scale(0.5);
                                           
                                           focusinfo->set_caption("Welcome to a new world!");
                                           
                                           string windowname="World seed: "+to_string(world->seed());
                                           globalmapwindow->set_title(windowname);
                                           
                                           getseedwindow->set_visible(0);
                                           globalmapwindow->set_visible(1);
                                       }
                                   }
                                   else // Otherwise, this is a cancel button.
                                   {
                                       getseedwindow->set_visible(0);
                                       globalmapwindow->set_visible(1);
                                   }
                               });
    
    importbutton->set_callback([&importwindow,&getseedwindow,&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globalwindsimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&importmapwidget]
                               {
                                   int width=world->width();
                                   int height=world->height();
                                   int val=world->sealevel()-5000;
                                   
                                   for (int i=0; i<=width; i++)
                                   {
                                       for (int j=0; j<=height; j++)
                                           world->setnom(i,j,val);
                                   }
                                   
                                   getseedwindow->set_visible(0);
                                   importwindow->set_visible(1);
                                   importwindow->request_focus();
                                   
                                   int seed=random(0,9);
                                   
                                   for (int n=1; n<=7; n++)
                                   {
                                       if (n==7)
                                           seed=seed+(random(1,9)*pow(10,n));
                                       else
                                           seed=seed+(random(0,9)*pow(10,n));
                                   }
                                   
                                   seed=0-seed;
                                   
                                   world->setseed(seed);
                                   
                                   // Now just redo the map.
                                   
                                   for (int n=0; n<GLOBALMAPTYPES; n++)
                                       globalmapimagecreated[n]=0;
                                   drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                   
                                   globalmap->upload(globalreliefimage);
                                   
                                   importmapwidget->set_image(globalmap);
                               });
    
    randombutton->set_callback([&seed,&seedinput]
                               {
                                   seed=random(0,9);
                                   
                                   for (int n=1; n<=7; n++)
                                   {
                                       if (n==7)
                                           seed=seed+(random(1,9)*pow(10,n));
                                       else
                                           seed=seed+(random(0,9)*pow(10,n));
                                   }
                                   
                                   seedinput->set_value(to_string(seed));
                               });
    
    OKbutton->set_callback([&seed,&seedinput,&screen,&getseedwindow,&world,&landshape,&chainland,&smalllake,&largelake,&mapview,&globalmapimagecreated,&globalelevationimage,&globalwindsimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalmapwindow,&worldgenerationwindow,&generationlabel,&worldprogress,&globalmap,&globalmapwidget,&focusinfo,&globalimagewidth,&globalimageheight,&globalimagechannels,&reliefbutton,&elevationbutton,&temperaturebutton,&precipitationbutton,&climatebutton,&riversbutton]
                           {
                               if (seedinput->value()!="")
                               {
                                   seed=stoi(seedinput->value());

                                   world->setseed(seed);
                                   
                                   // Now we need to actually create the world.
                                   
                                   for(int n=0; n<GLOBALMAPTYPES; n++)
                                       globalmapimagecreated[n]=0;
                                   
                                   // Bring up the window to show the progress of the world creation.
                                   
                                   seedinput->set_value("");
                                   
                                   getseedwindow->set_visible(0);
                                   worldgenerationwindow->set_visible(1);

                                   screen->perform_layout();
                                   screen->redraw();
                                   screen->draw_all();
                                   
                                   // Now do the actual world creation.

                                   short terraintype=2; // This terrain type gives large continents.
                                   
                                   if (random(1,10)==1) // Rarely, do the terrain type that gives fragmented land masses.
                                       terraintype=1;
                                   
                                   int creationsteps; // Number of steps in the generation process to mark.
                                   
                                   if (terraintype==1)
                                       creationsteps=GLOBALTERRAINCREATIONSTEPS1+GLOBALCLIMATECREATIONSTEPS;
                                   else
                                       creationsteps=GLOBALTERRAINCREATIONSTEPS2+GLOBALCLIMATECREATIONSTEPS;
                                   
                                   float progressstep=1.0/creationsteps;
                                   worldprogress->set_value(0);
                                   
                                   vector<vector<int>> mountaindrainage(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
                                   vector<vector<bool>> shelves(ARRAYWIDTH,vector<bool>(ARRAYWIDTH,ARRAYHEIGHT));
                                   generateglobalterrain(*world,terraintype,*screen,*worldgenerationwindow,*generationlabel,*worldprogress,progressstep,landshape,chainland,mountaindrainage,shelves);
                                   
                                   generateglobalclimate(*world,*screen,*worldgenerationwindow,*generationlabel,*worldprogress,progressstep,smalllake,largelake,landshape,mountaindrainage,shelves);
                                   
                                   generationlabel->set_caption("Preparing map image");
                                   worldprogress->set_value(1.0);
                                   screen->redraw();
                                   screen->draw_all();
                                   
                                   // Now move this window away entirely and bring in the world map window, with the right map applied.
                                   
                                   mapview=relief;
                                   
                                   drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                   

                                   globalmap->upload(globalreliefimage);
                                   
                                   globalmapwidget->set_image(globalmap);
                                   globalmapwidget->set_offset(Vector2f(0,0));
                                   globalmapwidget->set_scale(0.5);
                                   
                                   reliefbutton->set_pushed(1);
                                   elevationbutton->set_pushed(0);
                                   //windbutton->set_pushed(0);
                                   temperaturebutton->set_pushed(0);
                                   precipitationbutton->set_pushed(0);
                                   climatebutton->set_pushed(0);
                                   riversbutton->set_pushed(0);
                                   
                                   
                                   focusinfo->set_caption("Welcome to a new world!");
                                   
                                   worldgenerationwindow->set_visible(0);
                                   
                                   string windowname="World seed: "+to_string(world->seed());
                                   globalmapwindow->set_title(windowname);
                                   
                                   globalmapwindow->set_visible(1);
                                   globalmapwindow->request_focus();
                               }
                           });
    
    // Set up the callbacks for the map appearance widgets.
    
    oceanbutton->set_callback([&world,&colourbutton,&colourpickerwindow,&colourwheel,&redbox,&greenbox,&bluebox]
                              {
                                  colourpickerwindow->set_title("Shallow ocean");
                                  
                                  int red=world->ocean1();
                                  int green=world->ocean2();
                                  int blue=world->ocean3();
                                  
                                  Color colour(red,green,blue,255);
                                  
                                  colourbutton->set_background_color(colour);
                                  colourwheel->set_color(colour);
                                  
                                  redbox->set_value(red);
                                  greenbox->set_value(green);
                                  bluebox->set_value(blue);
                                  
                                  colourpickerwindow->set_visible(1);
                                  colourpickerwindow->request_focus();
                              });
    
    deepoceanbutton->set_callback([&world,&colourbutton,&colourpickerwindow,&colourwheel,&redbox,&greenbox,&bluebox]
                              {
                                  colourpickerwindow->set_title("Deep ocean");
                                  
                                  int red=world->deepocean1();
                                  int green=world->deepocean2();
                                  int blue=world->deepocean3();
                                  
                                  Color colour(red,green,blue,255);
                                  
                                  colourbutton->set_background_color(colour);
                                  colourwheel->set_color(colour);
                                  
                                  redbox->set_value(red);
                                  greenbox->set_value(green);
                                  bluebox->set_value(blue);
                                  
                                  colourpickerwindow->set_visible(1);
                                  colourpickerwindow->request_focus();
                              });
    
    basebutton->set_callback([&world,&colourbutton,&colourpickerwindow,&colourwheel,&redbox,&greenbox,&bluebox]
                              {
                                  colourpickerwindow->set_title("Base land");
                                  
                                  int red=world->base1();
                                  int green=world->base2();
                                  int blue=world->base3();
                                  
                                  Color colour(red,green,blue,255);
                                  
                                  colourbutton->set_background_color(colour);
                                  colourwheel->set_color(colour);
                                  
                                  redbox->set_value(red);
                                  greenbox->set_value(green);
                                  bluebox->set_value(blue);
                                  
                                  colourpickerwindow->set_visible(1);
                                  colourpickerwindow->request_focus();
                              });
    
    grassbutton->set_callback([&world,&colourbutton,&colourpickerwindow,&colourwheel,&redbox,&greenbox,&bluebox]
                              {
                                  colourpickerwindow->set_title("Grassland");
                                  
                                  int red=world->grass1();
                                  int green=world->grass2();
                                  int blue=world->grass3();
                                  
                                  Color colour(red,green,blue,255);
                                  
                                  colourbutton->set_background_color(colour);
                                  colourwheel->set_color(colour);
                                  
                                  redbox->set_value(red);
                                  greenbox->set_value(green);
                                  bluebox->set_value(blue);
                                  
                                  colourpickerwindow->set_visible(1);
                                  colourpickerwindow->request_focus();
                              });
    
    basetempbutton->set_callback([&world,&colourbutton,&colourpickerwindow,&colourwheel,&redbox,&greenbox,&bluebox]
                              {
                                  colourpickerwindow->set_title("Low temperate");
                                  
                                  int red=world->basetemp1();
                                  int green=world->basetemp2();
                                  int blue=world->basetemp3();
                                  
                                  Color colour(red,green,blue,255);
                                  
                                  colourbutton->set_background_color(colour);
                                  colourwheel->set_color(colour);
                                  
                                  redbox->set_value(red);
                                  greenbox->set_value(green);
                                  bluebox->set_value(blue);
                                  
                                  colourpickerwindow->set_visible(1);
                                  colourpickerwindow->request_focus();
                              });
    
    highbasebutton->set_callback([&world,&colourbutton,&colourpickerwindow,&colourwheel,&redbox,&greenbox,&bluebox]
                              {
                                  colourpickerwindow->set_title("High temperate");
                                  
                                  int red=world->highbase1();
                                  int green=world->highbase2();
                                  int blue=world->highbase3();
                                  
                                  Color colour(red,green,blue,255);
                                  
                                  colourbutton->set_background_color(colour);
                                  colourwheel->set_color(colour);
                                  
                                  redbox->set_value(red);
                                  greenbox->set_value(green);
                                  bluebox->set_value(blue);
                                  
                                  colourpickerwindow->set_visible(1);
                                  colourpickerwindow->request_focus();
                              });
    
    desertbutton->set_callback([&world,&colourbutton,&colourpickerwindow,&colourwheel,&redbox,&greenbox,&bluebox]
                              {
                                  colourpickerwindow->set_title("Low desert");
                                  
                                  int red=world->desert1();
                                  int green=world->desert2();
                                  int blue=world->desert3();
                                  
                                  Color colour(red,green,blue,255);
                                  
                                  colourbutton->set_background_color(colour);
                                  colourwheel->set_color(colour);
                                  
                                  redbox->set_value(red);
                                  greenbox->set_value(green);
                                  bluebox->set_value(blue);
                                  
                                  colourpickerwindow->set_visible(1);
                                  colourpickerwindow->request_focus();
                              });
    
    highdesertbutton->set_callback([&world,&colourbutton,&colourpickerwindow,&colourwheel,&redbox,&greenbox,&bluebox]
                              {
                                  colourpickerwindow->set_title("High desert");
                                  
                                  int red=world->highdesert1();
                                  int green=world->highdesert2();
                                  int blue=world->highdesert3();
                                  
                                  Color colour(red,green,blue,255);
                                  
                                  colourbutton->set_background_color(colour);
                                  colourwheel->set_color(colour);
                                  
                                  redbox->set_value(red);
                                  greenbox->set_value(green);
                                  bluebox->set_value(blue);
                                  
                                  colourpickerwindow->set_visible(1);
                                  colourpickerwindow->request_focus();
                              });
    
    colddesertbutton->set_callback([&world,&colourbutton,&colourpickerwindow,&colourwheel,&redbox,&greenbox,&bluebox]
                              {
                                  colourpickerwindow->set_title("Cold desert");
                                  
                                  int red=world->colddesert1();
                                  int green=world->colddesert2();
                                  int blue=world->colddesert3();
                                  
                                  Color colour(red,green,blue,255);
                                  
                                  colourbutton->set_background_color(colour);
                                  colourwheel->set_color(colour);
                                  
                                  redbox->set_value(red);
                                  greenbox->set_value(green);
                                  bluebox->set_value(blue);
                                  
                                  colourpickerwindow->set_visible(1);
                                  colourpickerwindow->request_focus();
                              });
    
    eqtundrabutton->set_callback([&world,&colourbutton,&colourpickerwindow,&colourwheel,&redbox,&greenbox,&bluebox]
                              {
                                  colourpickerwindow->set_title("Mild tundra");
                                  
                                  int red=world->eqtundra1();
                                  int green=world->eqtundra2();
                                  int blue=world->eqtundra3();
                                  
                                  Color colour(red,green,blue,255);
                                  
                                  colourbutton->set_background_color(colour);
                                  colourwheel->set_color(colour);
                                  
                                  redbox->set_value(red);
                                  greenbox->set_value(green);
                                  bluebox->set_value(blue);
                                  
                                  colourpickerwindow->set_visible(1);
                                  colourpickerwindow->request_focus();
                              });
    
    tundrabutton->set_callback([&world,&colourbutton,&colourpickerwindow,&colourwheel,&redbox,&greenbox,&bluebox]
                              {
                                  colourpickerwindow->set_title("Tundra");
                                  
                                  int red=world->tundra1();
                                  int green=world->tundra2();
                                  int blue=world->tundra3();
                                  
                                  Color colour(red,green,blue,255);
                                  
                                  colourbutton->set_background_color(colour);
                                  colourwheel->set_color(colour);
                                  
                                  redbox->set_value(red);
                                  greenbox->set_value(green);
                                  bluebox->set_value(blue);
                                  
                                  colourpickerwindow->set_visible(1);
                                  colourpickerwindow->request_focus();
                              });
    
    coldbutton->set_callback([&world,&colourbutton,&colourpickerwindow,&colourwheel,&redbox,&greenbox,&bluebox]
                              {
                                  colourpickerwindow->set_title("Arctic");
                                  
                                  int red=world->cold1();
                                  int green=world->cold2();
                                  int blue=world->cold3();
                                  
                                  Color colour(red,green,blue,255);
                                  
                                  colourbutton->set_background_color(colour);
                                  colourwheel->set_color(colour);
                                  
                                  redbox->set_value(red);
                                  greenbox->set_value(green);
                                  bluebox->set_value(blue);
                                  
                                  colourpickerwindow->set_visible(1);
                                  colourpickerwindow->request_focus();
                              });
    
    seaicebutton->set_callback([&world,&colourbutton,&colourpickerwindow,&colourwheel,&redbox,&greenbox,&bluebox]
                              {
                                  colourpickerwindow->set_title("Sea ice");
                                  
                                  int red=world->seaice1();
                                  int green=world->seaice2();
                                  int blue=world->seaice3();
                                  
                                  Color colour(red,green,blue,255);
                                  
                                  colourbutton->set_background_color(colour);
                                  colourwheel->set_color(colour);
                                  
                                  redbox->set_value(red);
                                  greenbox->set_value(green);
                                  bluebox->set_value(blue);
                                  
                                  colourpickerwindow->set_visible(1);
                                  colourpickerwindow->request_focus();
                              });
    
    glacierbutton->set_callback([&world,&colourbutton,&colourpickerwindow,&colourwheel,&redbox,&greenbox,&bluebox]
                              {
                                  colourpickerwindow->set_title("Glaciers");
                                  
                                  int red=world->glacier1();
                                  int green=world->glacier2();
                                  int blue=world->glacier3();
                                  
                                  Color colour(red,green,blue,255);
                                  
                                  colourbutton->set_background_color(colour);
                                  colourwheel->set_color(colour);
                                  
                                  redbox->set_value(red);
                                  greenbox->set_value(green);
                                  bluebox->set_value(blue);
                                  
                                  colourpickerwindow->set_visible(1);
                                  colourpickerwindow->request_focus();
                              });
    
    saltpanbutton->set_callback([&world,&colourbutton,&colourpickerwindow,&colourwheel,&redbox,&greenbox,&bluebox]
                              {
                                  colourpickerwindow->set_title("Salt pans");
                                  
                                  int red=world->saltpan1();
                                  int green=world->saltpan2();
                                  int blue=world->saltpan3();
                                  
                                  Color colour(red,green,blue,255);
                                  
                                  colourbutton->set_background_color(colour);
                                  colourwheel->set_color(colour);
                                  
                                  redbox->set_value(red);
                                  greenbox->set_value(green);
                                  bluebox->set_value(blue);
                                  
                                  colourpickerwindow->set_visible(1);
                                  colourpickerwindow->request_focus();
                              });
    
    ergbutton->set_callback([&world,&colourbutton,&colourpickerwindow,&colourwheel,&redbox,&greenbox,&bluebox]
                              {
                                  colourpickerwindow->set_title("Dunes");
                                  
                                  int red=world->erg1();
                                  int green=world->erg2();
                                  int blue=world->erg3();
                                  
                                  Color colour(red,green,blue,255);
                                  
                                  colourbutton->set_background_color(colour);
                                  colourwheel->set_color(colour);
                                  
                                  redbox->set_value(red);
                                  greenbox->set_value(green);
                                  bluebox->set_value(blue);
                                  
                                  colourpickerwindow->set_visible(1);
                                  colourpickerwindow->request_focus();
                              });
    
    wetlandsbutton->set_callback([&world,&colourbutton,&colourpickerwindow,&colourwheel,&redbox,&greenbox,&bluebox]
                              {
                                  colourpickerwindow->set_title("Wetlands");
                                  
                                  int red=world->wetlands1();
                                  int green=world->wetlands2();
                                  int blue=world->wetlands3();
                                  
                                  Color colour(red,green,blue,255);
                                  
                                  colourbutton->set_background_color(colour);
                                  colourwheel->set_color(colour);
                                  
                                  redbox->set_value(red);
                                  greenbox->set_value(green);
                                  bluebox->set_value(blue);
                                  
                                  colourpickerwindow->set_visible(1);
                                  colourpickerwindow->request_focus();
                              });
    
    lakebutton->set_callback([&world,&colourbutton,&colourpickerwindow,&colourwheel,&redbox,&greenbox,&bluebox]
                              {
                                  colourpickerwindow->set_title("Lakes");
                                  
                                  int red=world->lake1();
                                  int green=world->lake2();
                                  int blue=world->lake3();
                                  
                                  Color colour(red,green,blue,255);
                                  
                                  colourbutton->set_background_color(colour);
                                  colourwheel->set_color(colour);
                                  
                                  redbox->set_value(red);
                                  greenbox->set_value(green);
                                  bluebox->set_value(blue);
                                  
                                  colourpickerwindow->set_visible(1);
                                  colourpickerwindow->request_focus();
                              });
    
    riverbutton->set_callback([&world,&colourbutton,&colourpickerwindow,&colourwheel,&redbox,&greenbox,&bluebox]
                              {
                                  colourpickerwindow->set_title("Rivers");
                                  
                                  int red=world->river1();
                                  int green=world->river2();
                                  int blue=world->river3();
                                  
                                  Color colour(red,green,blue,255);
                                  
                                  colourbutton->set_background_color(colour);
                                  colourwheel->set_color(colour);
                                  
                                  redbox->set_value(red);
                                  greenbox->set_value(green);
                                  bluebox->set_value(blue);
                                  
                                  colourpickerwindow->set_visible(1);
                                  colourpickerwindow->request_focus();
                              });
    
    highlightbutton->set_callback([&world,&colourbutton,&colourpickerwindow,&colourwheel,&redbox,&greenbox,&bluebox]
                              {
                                  colourpickerwindow->set_title("Highlights");
                                  
                                  int red=world->highlight1();
                                  int green=world->highlight2();
                                  int blue=world->highlight3();
                                  
                                  Color colour(red,green,blue,255);
                                  
                                  colourbutton->set_background_color(colour);
                                  colourwheel->set_color(colour);
                                  
                                  redbox->set_value(red);
                                  greenbox->set_value(green);
                                  bluebox->set_value(blue);
                                  
                                  colourpickerwindow->set_visible(1);
                                  colourpickerwindow->request_focus();
                              });
    
    shadinglandslider->set_final_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globalwindsimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&globalmapwidget,&region,&regionalminimap,&regionalminimapwidget,&regionalmapwindow,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&regionalmap,&regionalmapwidget,&focused,&poix,&poiy,&rfocused](float value)
                                   {
                                       world->setlandshading(value);
                                       
                                       // Redraw the global map.
                                       
                                       for (int n=0; n<GLOBALMAPTYPES; n++)
                                           globalmapimagecreated[n]=0;
                                       drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                       
                                       
                                       globalmap->upload(globalreliefimage);
                                       globalmapwidget->set_image(globalmap);
                                       
                                       setregionalminimap(*world,*region,globalreliefimage,*regionalminimap,globalimagewidth,globalimageheight,globalimagechannels);
                                       
                                       regionalminimapwidget->set_image(regionalminimap);
                                       
                                       int r,g,b;
                                       
                                       if (focused==1) // If there's a focus, copy the colour at that point on the map.
                                       {
                                           int index=(poix+poiy*globalimagewidth)*globalimagechannels;
                                           
                                           r=globalreliefimage[index];
                                           g=globalreliefimage[index+1];
                                           b=globalreliefimage[index+2];
                                           
                                           globalreliefimage[index]=world->highlight1();
                                           globalreliefimage[index+1]=world->highlight2();
                                           globalreliefimage[index+2]=world->highlight3();
                                           
                                           globalmap->upload(globalreliefimage);
                                           globalmapwidget->set_image(globalmap);
                                           
                                           globalreliefimage[index]=r;
                                           globalreliefimage[index+1]=g;
                                           globalreliefimage[index+2]=b;
                                       }
                                       
                                       // Redraw the regional map, if need be.
                                       
                                       if (regionalmapwindow->visible()==1)
                                       {
                                           for (int n=0; n<GLOBALMAPTYPES; n++)
                                               regionalmapimagecreated[n]=0;
                                           drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                           
                                           regionalmap->upload(regionalreliefimage);
                                           regionalmapwidget->set_image(regionalmap);
                                           
                                           if (rfocused==1)
                                           {
                                               int r,g,b;
                                               
                                               int index=((poix-region->regwidthbegin())+(poiy-region->regheightbegin())*regionalimagewidth)*regionalimagechannels;
                                               
                                               r=regionalreliefimage[index];
                                               g=regionalreliefimage[index+1];
                                               b=regionalreliefimage[index+2];
                                               
                                               regionalreliefimage[index]=world->highlight1();
                                               regionalreliefimage[index+1]=world->highlight2();
                                               regionalreliefimage[index+2]=world->highlight3();
                                               
                                               regionalmap->upload(regionalreliefimage);
                                               regionalmapwidget->set_image(regionalmap);
                                               
                                               regionalreliefimage[index]=r;
                                               regionalreliefimage[index+1]=g;
                                               regionalreliefimage[index+2]=b;
                                           }
                                       }
                                       
                                   });
    
    shadinglakesslider->set_final_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globalwindsimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&globalmapwidget,&region,&regionalminimap,&regionalminimapwidget,&regionalmapwindow,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&regionalmap,&regionalmapwidget,&focused,&poix,&poiy,&rfocused](float value)
                                          {
                                              world->setlakeshading(value);
                                              
                                              // Redraw the global map.
                                              
                                              for (int n=0; n<GLOBALMAPTYPES; n++)
                                                  globalmapimagecreated[n]=0;
                                              drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                              
                                              
                                              globalmap->upload(globalreliefimage);
                                              globalmapwidget->set_image(globalmap);
                                              
                                              setregionalminimap(*world,*region,globalreliefimage,*regionalminimap,globalimagewidth,globalimageheight,globalimagechannels);
                                              
                                              regionalminimapwidget->set_image(regionalminimap);
                                              
                                              int r,g,b;
                                              
                                              if (focused==1) // If there's a focus, copy the colour at that point on the map.
                                              {
                                                  int index=(poix+poiy*globalimagewidth)*globalimagechannels;
                                                  
                                                  r=globalreliefimage[index];
                                                  g=globalreliefimage[index+1];
                                                  b=globalreliefimage[index+2];
                                                  
                                                  globalreliefimage[index]=world->highlight1();
                                                  globalreliefimage[index+1]=world->highlight2();
                                                  globalreliefimage[index+2]=world->highlight3();
                                                  
                                                  globalmap->upload(globalreliefimage);
                                                  globalmapwidget->set_image(globalmap);
                                                  
                                                  globalreliefimage[index]=r;
                                                  globalreliefimage[index+1]=g;
                                                  globalreliefimage[index+2]=b;
                                              }
                                              
                                              // Redraw the regional map, if need be.
                                              
                                              if (regionalmapwindow->visible()==1)
                                              {
                                                  for (int n=0; n<GLOBALMAPTYPES; n++)
                                                      regionalmapimagecreated[n]=0;
                                                  drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                                  
                                                  regionalmap->upload(regionalreliefimage);
                                                  regionalmapwidget->set_image(regionalmap);
                                                  
                                                  if (rfocused==1)
                                                  {
                                                      int r,g,b;
                                                      
                                                      int index=((poix-region->regwidthbegin())+(poiy-region->regheightbegin())*regionalimagewidth)*regionalimagechannels;
                                                      
                                                      r=regionalreliefimage[index];
                                                      g=regionalreliefimage[index+1];
                                                      b=regionalreliefimage[index+2];
                                                      
                                                      regionalreliefimage[index]=world->highlight1();
                                                      regionalreliefimage[index+1]=world->highlight2();
                                                      regionalreliefimage[index+2]=world->highlight3();
                                                      
                                                      regionalmap->upload(regionalreliefimage);
                                                      regionalmapwidget->set_image(regionalmap);
                                                      
                                                      regionalreliefimage[index]=r;
                                                      regionalreliefimage[index+1]=g;
                                                      regionalreliefimage[index+2]=b;
                                                  }
                                              }
                                              
                                          });
    
    shadingseaslider->set_final_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globalwindsimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&globalmapwidget,&region,&regionalminimap,&regionalminimapwidget,&regionalmapwindow,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&regionalmap,&regionalmapwidget,&focused,&poix,&poiy,&rfocused](float value)
                                          {
                                              world->setseashading(value);
                                              
                                              // Redraw the global map.
                                              
                                              for (int n=0; n<GLOBALMAPTYPES; n++)
                                                  globalmapimagecreated[n]=0;
                                              drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                              
                                              
                                              globalmap->upload(globalreliefimage);
                                              globalmapwidget->set_image(globalmap);
                                              
                                              setregionalminimap(*world,*region,globalreliefimage,*regionalminimap,globalimagewidth,globalimageheight,globalimagechannels);
                                              
                                              regionalminimapwidget->set_image(regionalminimap);
                                              
                                              int r,g,b;
                                              
                                              if (focused==1) // If there's a focus, copy the colour at that point on the map.
                                              {
                                                  int index=(poix+poiy*globalimagewidth)*globalimagechannels;
                                                  
                                                  r=globalreliefimage[index];
                                                  g=globalreliefimage[index+1];
                                                  b=globalreliefimage[index+2];
                                                  
                                                  globalreliefimage[index]=world->highlight1();
                                                  globalreliefimage[index+1]=world->highlight2();
                                                  globalreliefimage[index+2]=world->highlight3();
                                                  
                                                  globalmap->upload(globalreliefimage);
                                                  globalmapwidget->set_image(globalmap);
                                                  
                                                  globalreliefimage[index]=r;
                                                  globalreliefimage[index+1]=g;
                                                  globalreliefimage[index+2]=b;
                                              }
                                              
                                              // Redraw the regional map, if need be.
                                              
                                              if (regionalmapwindow->visible()==1)
                                              {
                                                  for (int n=0; n<GLOBALMAPTYPES; n++)
                                                      regionalmapimagecreated[n]=0;
                                                  drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                                  
                                                  regionalmap->upload(regionalreliefimage);
                                                  regionalmapwidget->set_image(regionalmap);
                                                  
                                                  if (rfocused==1)
                                                  {
                                                      int r,g,b;
                                                      
                                                      int index=((poix-region->regwidthbegin())+(poiy-region->regheightbegin())*regionalimagewidth)*regionalimagechannels;
                                                      
                                                      r=regionalreliefimage[index];
                                                      g=regionalreliefimage[index+1];
                                                      b=regionalreliefimage[index+2];
                                                      
                                                      regionalreliefimage[index]=world->highlight1();
                                                      regionalreliefimage[index+1]=world->highlight2();
                                                      regionalreliefimage[index+2]=world->highlight3();
                                                      
                                                      regionalmap->upload(regionalreliefimage);
                                                      regionalmapwidget->set_image(regionalmap);
                                                      
                                                      regionalreliefimage[index]=r;
                                                      regionalreliefimage[index+1]=g;
                                                      regionalreliefimage[index+2]=b;
                                                  }
                                              }
                                          });
    
    shadingdirectionchooser->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globalwindsimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&globalmapwidget,&region,&regionalminimap,&regionalminimapwidget,&regionalmapwindow,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&regionalmap,&regionalmapwidget,&focused,&poix,&poiy,&rfocused](int value)
                                          {
                                              int oldval=world->shadingdir();
                                              
                                              if (value==0)
                                                  world->setshadingdir(4);
                                              
                                              if (value==1)
                                                  world->setshadingdir(6);
                                              
                                              if (value==2)
                                                  world->setshadingdir(2);
                                              
                                              if (value==3)
                                                  world->setshadingdir(8);
                                              
                                              if (world->shadingdir()!=oldval)
                                              {
                                                  // Redraw the global map.
                                                  
                                                  for (int n=0; n<GLOBALMAPTYPES; n++)
                                                      globalmapimagecreated[n]=0;
                                                  drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                                  
                                                  
                                                  globalmap->upload(globalreliefimage);
                                                  globalmapwidget->set_image(globalmap);
                                                  
                                                  setregionalminimap(*world,*region,globalreliefimage,*regionalminimap,globalimagewidth,globalimageheight,globalimagechannels);
                                                  
                                                  regionalminimapwidget->set_image(regionalminimap);
                                                  
                                                  int r,g,b;
                                                  
                                                  if (focused==1) // If there's a focus, copy the colour at that point on the map.
                                                  {
                                                      int index=(poix+poiy*globalimagewidth)*globalimagechannels;
                                                      
                                                      r=globalreliefimage[index];
                                                      g=globalreliefimage[index+1];
                                                      b=globalreliefimage[index+2];
                                                      
                                                      globalreliefimage[index]=world->highlight1();
                                                      globalreliefimage[index+1]=world->highlight2();
                                                      globalreliefimage[index+2]=world->highlight3();
                                                      
                                                      globalmap->upload(globalreliefimage);
                                                      globalmapwidget->set_image(globalmap);
                                                      
                                                      globalreliefimage[index]=r;
                                                      globalreliefimage[index+1]=g;
                                                      globalreliefimage[index+2]=b;
                                                  }
                                                  
                                                  // Redraw the regional map, if need be.
                                                  
                                                  if (regionalmapwindow->visible()==1)
                                                  {
                                                      for (int n=0; n<GLOBALMAPTYPES; n++)
                                                          regionalmapimagecreated[n]=0;
                                                      drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                                      
                                                      regionalmap->upload(regionalreliefimage);
                                                      regionalmapwidget->set_image(regionalmap);
                                                      
                                                      if (rfocused==1)
                                                      {
                                                          int r,g,b;
                                                          
                                                          int index=((poix-region->regwidthbegin())+(poiy-region->regheightbegin())*regionalimagewidth)*regionalimagechannels;
                                                          
                                                          r=regionalreliefimage[index];
                                                          g=regionalreliefimage[index+1];
                                                          b=regionalreliefimage[index+2];
                                                          
                                                          regionalreliefimage[index]=world->highlight1();
                                                          regionalreliefimage[index+1]=world->highlight2();
                                                          regionalreliefimage[index+2]=world->highlight3();
                                                          
                                                          regionalmap->upload(regionalreliefimage);
                                                          regionalmapwidget->set_image(regionalmap);
                                                          
                                                          regionalreliefimage[index]=r;
                                                          regionalreliefimage[index+1]=g;
                                                          regionalreliefimage[index+2]=b;
                                                      }
                                                  }
                                              }
                                          });
    
    snowchangechooser->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globalwindsimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&globalmapwidget,&region,&regionalminimap,&regionalminimapwidget,&regionalmapwindow,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&regionalmap,&regionalmapwidget,&focused,&poix,&poiy,&rfocused](int value)
                                          {
                                              int oldval=world->snowchange();
                                              
                                              if (value==0)
                                                  world->setsnowchange(1);
                                              
                                              if (value==1)
                                                  world->setsnowchange(2);
                                              
                                              if (value==2)
                                                  world->setsnowchange(3);
                                              
                                              if (world->snowchange()!=oldval)
                                              {
                                                  // Redraw the global map.
                                                  
                                                  for (int n=0; n<GLOBALMAPTYPES; n++)
                                                      globalmapimagecreated[n]=0;
                                                  drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                                  
                                                  
                                                  globalmap->upload(globalreliefimage);
                                                  globalmapwidget->set_image(globalmap);
                                                  
                                                  setregionalminimap(*world,*region,globalreliefimage,*regionalminimap,globalimagewidth,globalimageheight,globalimagechannels);
                                                  
                                                  regionalminimapwidget->set_image(regionalminimap);
                                                  
                                                  int r,g,b;
                                                  
                                                  if (focused==1) // If there's a focus, copy the colour at that point on the map.
                                                  {
                                                      int index=(poix+poiy*globalimagewidth)*globalimagechannels;
                                                      
                                                      r=globalreliefimage[index];
                                                      g=globalreliefimage[index+1];
                                                      b=globalreliefimage[index+2];
                                                      
                                                      globalreliefimage[index]=world->highlight1();
                                                      globalreliefimage[index+1]=world->highlight2();
                                                      globalreliefimage[index+2]=world->highlight3();
                                                      
                                                      globalmap->upload(globalreliefimage);
                                                      globalmapwidget->set_image(globalmap);
                                                      
                                                      globalreliefimage[index]=r;
                                                      globalreliefimage[index+1]=g;
                                                      globalreliefimage[index+2]=b;
                                                  }
                                                  
                                                  // Redraw the regional map, if need be.
                                                  
                                                  if (regionalmapwindow->visible()==1)
                                                  {
                                                      for (int n=0; n<GLOBALMAPTYPES; n++)
                                                          regionalmapimagecreated[n]=0;
                                                      drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                                      
                                                      regionalmap->upload(regionalreliefimage);
                                                      regionalmapwidget->set_image(regionalmap);
                                                      
                                                      if (rfocused==1)
                                                      {
                                                          int r,g,b;
                                                          
                                                          int index=((poix-region->regwidthbegin())+(poiy-region->regheightbegin())*regionalimagewidth)*regionalimagechannels;
                                                          
                                                          r=regionalreliefimage[index];
                                                          g=regionalreliefimage[index+1];
                                                          b=regionalreliefimage[index+2];
                                                          
                                                          regionalreliefimage[index]=world->highlight1();
                                                          regionalreliefimage[index+1]=world->highlight2();
                                                          regionalreliefimage[index+2]=world->highlight3();
                                                          
                                                          regionalmap->upload(regionalreliefimage);
                                                          regionalmapwidget->set_image(regionalmap);
                                                          
                                                          regionalreliefimage[index]=r;
                                                          regionalreliefimage[index+1]=g;
                                                          regionalreliefimage[index+2]=b;
                                                      }
                                                  }
                                                  
                                              }
                                          });
    
    seaicechooser->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globalwindsimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&globalmapwidget,&region,&regionalminimap,&regionalminimapwidget,&regionalmapwindow,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&regionalmap,&regionalmapwidget,&focused,&poix,&poiy,&rfocused](int value)
                                    {
                                        int oldval=world->seaiceappearance();
                                        
                                        if (value==0)
                                            world->setseaiceappearance(1);
                                        
                                        if (value==1)
                                            world->setseaiceappearance(2);
                                        
                                        if (value==2)
                                            world->setseaiceappearance(3);
                                        
                                        if (world->seaiceappearance()!=oldval)
                                        {
                                            // Redraw the global map.
                                            
                                            for (int n=0; n<GLOBALMAPTYPES; n++)
                                                globalmapimagecreated[n]=0;
                                            drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                            
                                            
                                            globalmap->upload(globalreliefimage);
                                            globalmapwidget->set_image(globalmap);
                                            
                                            setregionalminimap(*world,*region,globalreliefimage,*regionalminimap,globalimagewidth,globalimageheight,globalimagechannels);
                                            
                                            regionalminimapwidget->set_image(regionalminimap);
                                            
                                            int r,g,b;
                                            
                                            if (focused==1) // If there's a focus, copy the colour at that point on the map.
                                            {
                                                int index=(poix+poiy*globalimagewidth)*globalimagechannels;
                                                
                                                r=globalreliefimage[index];
                                                g=globalreliefimage[index+1];
                                                b=globalreliefimage[index+2];
                                                
                                                globalreliefimage[index]=world->highlight1();
                                                globalreliefimage[index+1]=world->highlight2();
                                                globalreliefimage[index+2]=world->highlight3();
                                                
                                                globalmap->upload(globalreliefimage);
                                                globalmapwidget->set_image(globalmap);
                                                
                                                globalreliefimage[index]=r;
                                                globalreliefimage[index+1]=g;
                                                globalreliefimage[index+2]=b;
                                            }
                                            
                                            // Redraw the regional map, if need be.
                                            
                                            if (regionalmapwindow->visible()==1)
                                            {
                                                for (int n=0; n<GLOBALMAPTYPES; n++)
                                                    regionalmapimagecreated[n]=0;
                                                drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                                
                                                regionalmap->upload(regionalreliefimage);
                                                regionalmapwidget->set_image(regionalmap);
                                                
                                                if (rfocused==1)
                                                {
                                                    int r,g,b;
                                                    
                                                    int index=((poix-region->regwidthbegin())+(poiy-region->regheightbegin())*regionalimagewidth)*regionalimagechannels;
                                                    
                                                    r=regionalreliefimage[index];
                                                    g=regionalreliefimage[index+1];
                                                    b=regionalreliefimage[index+2];
                                                    
                                                    regionalreliefimage[index]=world->highlight1();
                                                    regionalreliefimage[index+1]=world->highlight2();
                                                    regionalreliefimage[index+2]=world->highlight3();
                                                    
                                                    regionalmap->upload(regionalreliefimage);
                                                    regionalmapwidget->set_image(regionalmap);
                                                    
                                                    regionalreliefimage[index]=r;
                                                    regionalreliefimage[index+1]=g;
                                                    regionalreliefimage[index+2]=b;
                                                }
                                            }
                                            
                                        }
                                    });
    
    marblinglandslider->set_final_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globalwindsimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&globalmapwidget,&region,&regionalminimap,&regionalminimapwidget,&regionalmapwindow,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&regionalmap,&regionalmapwidget,&focused,&poix,&poiy,&rfocused](float value)
                                          {
                                              world->setlandmarbling(value);
                                              
                                              // Redraw the global map.
                                              
                                              for (int n=0; n<GLOBALMAPTYPES; n++)
                                                  globalmapimagecreated[n]=0;
                                              drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                              
                                              
                                              globalmap->upload(globalreliefimage);
                                              globalmapwidget->set_image(globalmap);
                                              
                                              setregionalminimap(*world,*region,globalreliefimage,*regionalminimap,globalimagewidth,globalimageheight,globalimagechannels);
                                              
                                              regionalminimapwidget->set_image(regionalminimap);
                                              
                                              int r,g,b;
                                              
                                              if (focused==1) // If there's a focus, copy the colour at that point on the map.
                                              {
                                                  int index=(poix+poiy*globalimagewidth)*globalimagechannels;
                                                  
                                                  r=globalreliefimage[index];
                                                  g=globalreliefimage[index+1];
                                                  b=globalreliefimage[index+2];
                                                  
                                                  globalreliefimage[index]=world->highlight1();
                                                  globalreliefimage[index+1]=world->highlight2();
                                                  globalreliefimage[index+2]=world->highlight3();
                                                  
                                                  globalmap->upload(globalreliefimage);
                                                  globalmapwidget->set_image(globalmap);
                                                  
                                                  globalreliefimage[index]=r;
                                                  globalreliefimage[index+1]=g;
                                                  globalreliefimage[index+2]=b;
                                              }
                                              
                                              // Redraw the regional map, if need be.
                                              
                                              if (regionalmapwindow->visible()==1)
                                              {
                                                  for (int n=0; n<GLOBALMAPTYPES; n++)
                                                      regionalmapimagecreated[n]=0;
                                                  drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                                  
                                                  regionalmap->upload(regionalreliefimage);
                                                  regionalmapwidget->set_image(regionalmap);
                                                  
                                                  if (rfocused==1)
                                                  {
                                                      int r,g,b;
                                                      
                                                      int index=((poix-region->regwidthbegin())+(poiy-region->regheightbegin())*regionalimagewidth)*regionalimagechannels;
                                                      
                                                      r=regionalreliefimage[index];
                                                      g=regionalreliefimage[index+1];
                                                      b=regionalreliefimage[index+2];
                                                      
                                                      regionalreliefimage[index]=world->highlight1();
                                                      regionalreliefimage[index+1]=world->highlight2();
                                                      regionalreliefimage[index+2]=world->highlight3();
                                                      
                                                      regionalmap->upload(regionalreliefimage);
                                                      regionalmapwidget->set_image(regionalmap);
                                                      
                                                      regionalreliefimage[index]=r;
                                                      regionalreliefimage[index+1]=g;
                                                      regionalreliefimage[index+2]=b;
                                                  }
                                              }
                                              
                                          });
    
    marblinglakesslider->set_final_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globalwindsimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&globalmapwidget,&region,&regionalminimap,&regionalminimapwidget,&regionalmapwindow,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&regionalmap,&regionalmapwidget,&focused,&poix,&poiy,&rfocused](float value)
                                           {
                                               world->setlakemarbling(value);
                                               
                                               // Redraw the global map.
                                               
                                               for (int n=0; n<GLOBALMAPTYPES; n++)
                                                   globalmapimagecreated[n]=0;
                                               drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                               
                                               
                                               globalmap->upload(globalreliefimage);
                                               globalmapwidget->set_image(globalmap);
                                               
                                               setregionalminimap(*world,*region,globalreliefimage,*regionalminimap,globalimagewidth,globalimageheight,globalimagechannels);
                                               
                                               regionalminimapwidget->set_image(regionalminimap);
                                               
                                               int r,g,b;
                                               
                                               if (focused==1) // If there's a focus, copy the colour at that point on the map.
                                               {
                                                   int index=(poix+poiy*globalimagewidth)*globalimagechannels;
                                                   
                                                   r=globalreliefimage[index];
                                                   g=globalreliefimage[index+1];
                                                   b=globalreliefimage[index+2];
                                                   
                                                   globalreliefimage[index]=world->highlight1();
                                                   globalreliefimage[index+1]=world->highlight2();
                                                   globalreliefimage[index+2]=world->highlight3();
                                                   
                                                   globalmap->upload(globalreliefimage);
                                                   globalmapwidget->set_image(globalmap);
                                                   
                                                   globalreliefimage[index]=r;
                                                   globalreliefimage[index+1]=g;
                                                   globalreliefimage[index+2]=b;
                                               }
                                               
                                               // Redraw the regional map, if need be.
                                               
                                               if (regionalmapwindow->visible()==1)
                                               {
                                                   for (int n=0; n<GLOBALMAPTYPES; n++)
                                                       regionalmapimagecreated[n]=0;
                                                   drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                                   
                                                   regionalmap->upload(regionalreliefimage);
                                                   regionalmapwidget->set_image(regionalmap);
                                                   
                                                   if (rfocused==1)
                                                   {
                                                       int r,g,b;
                                                       
                                                       int index=((poix-region->regwidthbegin())+(poiy-region->regheightbegin())*regionalimagewidth)*regionalimagechannels;
                                                       
                                                       r=regionalreliefimage[index];
                                                       g=regionalreliefimage[index+1];
                                                       b=regionalreliefimage[index+2];
                                                       
                                                       regionalreliefimage[index]=world->highlight1();
                                                       regionalreliefimage[index+1]=world->highlight2();
                                                       regionalreliefimage[index+2]=world->highlight3();
                                                       
                                                       regionalmap->upload(regionalreliefimage);
                                                       regionalmapwidget->set_image(regionalmap);
                                                       
                                                       regionalreliefimage[index]=r;
                                                       regionalreliefimage[index+1]=g;
                                                       regionalreliefimage[index+2]=b;
                                                   }
                                               }
                                               
                                           });
    
    marblingseaslider->set_final_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globalwindsimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&globalmapwidget,&region,&regionalminimap,&regionalminimapwidget,&regionalmapwindow,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&regionalmap,&regionalmapwidget,&focused,&poix,&poiy,&rfocused](float value)
                                           {
                                               world->setseamarbling(value);
                                               
                                               // Redraw the global map.
                                               
                                               for (int n=0; n<GLOBALMAPTYPES; n++)
                                                   globalmapimagecreated[n]=0;
                                               drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                               
                                               
                                               globalmap->upload(globalreliefimage);
                                               globalmapwidget->set_image(globalmap);
                                               
                                               setregionalminimap(*world,*region,globalreliefimage,*regionalminimap,globalimagewidth,globalimageheight,globalimagechannels);
                                               
                                               regionalminimapwidget->set_image(regionalminimap);
                                               
                                               int r,g,b;
                                               
                                               if (focused==1) // If there's a focus, copy the colour at that point on the map.
                                               {
                                                   int index=(poix+poiy*globalimagewidth)*globalimagechannels;
                                                   
                                                   r=globalreliefimage[index];
                                                   g=globalreliefimage[index+1];
                                                   b=globalreliefimage[index+2];
                                                   
                                                   globalreliefimage[index]=world->highlight1();
                                                   globalreliefimage[index+1]=world->highlight2();
                                                   globalreliefimage[index+2]=world->highlight3();
                                                   
                                                   globalmap->upload(globalreliefimage);
                                                   globalmapwidget->set_image(globalmap);
                                                   
                                                   globalreliefimage[index]=r;
                                                   globalreliefimage[index+1]=g;
                                                   globalreliefimage[index+2]=b;
                                               }
                                               
                                               // Redraw the regional map, if need be.
                                               
                                               if (regionalmapwindow->visible()==1)
                                               {
                                                   for (int n=0; n<GLOBALMAPTYPES; n++)
                                                       regionalmapimagecreated[n]=0;
                                                   drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                                   
                                                   regionalmap->upload(regionalreliefimage);
                                                   regionalmapwidget->set_image(regionalmap);
                                                   
                                                   if (rfocused==1)
                                                   {
                                                       int r,g,b;
                                                       
                                                       int index=((poix-region->regwidthbegin())+(poiy-region->regheightbegin())*regionalimagewidth)*regionalimagechannels;
                                                       
                                                       r=regionalreliefimage[index];
                                                       g=regionalreliefimage[index+1];
                                                       b=regionalreliefimage[index+2];
                                                       
                                                       regionalreliefimage[index]=world->highlight1();
                                                       regionalreliefimage[index+1]=world->highlight2();
                                                       regionalreliefimage[index+2]=world->highlight3();
                                                       
                                                       regionalmap->upload(regionalreliefimage);
                                                       regionalmapwidget->set_image(regionalmap);
                                                       
                                                       regionalreliefimage[index]=r;
                                                       regionalreliefimage[index+1]=g;
                                                       regionalreliefimage[index+2]=b;
                                                   }
                                               }
                                           });

    riversglobalbox->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globalwindsimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&globalmapwidget,&region,&regionalminimap,&regionalminimapwidget,&regionalmapwindow,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&regionalmap,&regionalmapwidget,&focused,&poix,&poiy,&rfocused] (const int &v)
                         {
                             world->setminriverflowglobal(v);
                             
                             // Redraw the global map.
                             
                             for (int n=0; n<GLOBALMAPTYPES; n++)
                                 globalmapimagecreated[n]=0;
                             drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                             
                             
                             globalmap->upload(globalreliefimage);
                             globalmapwidget->set_image(globalmap);
                             
                             setregionalminimap(*world,*region,globalreliefimage,*regionalminimap,globalimagewidth,globalimageheight,globalimagechannels);
                             
                             regionalminimapwidget->set_image(regionalminimap);
                             
                             int r,g,b;
                             
                             if (focused==1) // If there's a focus, copy the colour at that point on the map.
                             {
                                 int index=(poix+poiy*globalimagewidth)*globalimagechannels;
                                 
                                 r=globalreliefimage[index];
                                 g=globalreliefimage[index+1];
                                 b=globalreliefimage[index+2];
                                 
                                 globalreliefimage[index]=world->highlight1();
                                 globalreliefimage[index+1]=world->highlight2();
                                 globalreliefimage[index+2]=world->highlight3();
                                 
                                 globalmap->upload(globalreliefimage);
                                 globalmapwidget->set_image(globalmap);
                                 
                                 globalreliefimage[index]=r;
                                 globalreliefimage[index+1]=g;
                                 globalreliefimage[index+2]=b;
                             }
                         });
    
    riversregionalbox->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globalwindsimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&globalmapwidget,&region,&regionalminimap,&regionalminimapwidget,&regionalmapwindow,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&regionalmap,&regionalmapwidget,&focused,&poix,&poiy,&rfocused] (const int &v)
                                  {
                                      world->setminriverflowregional(v);
                                      
                                      // Redraw the regional map, if need be.
                                      
                                      if (regionalmapwindow->visible()==1)
                                      {
                                          for (int n=0; n<GLOBALMAPTYPES; n++)
                                              regionalmapimagecreated[n]=0;
                                          drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                          
                                          regionalmap->upload(regionalreliefimage);
                                          regionalmapwidget->set_image(regionalmap);
                                          
                                          if (rfocused==1)
                                          {
                                              int r,g,b;
                                              
                                              int index=((poix-region->regwidthbegin())+(poiy-region->regheightbegin())*regionalimagewidth)*regionalimagechannels;
                                              
                                              r=regionalreliefimage[index];
                                              g=regionalreliefimage[index+1];
                                              b=regionalreliefimage[index+2];
                                              
                                              regionalreliefimage[index]=world->highlight1();
                                              regionalreliefimage[index+1]=world->highlight2();
                                              regionalreliefimage[index+2]=world->highlight3();
                                              
                                              regionalmap->upload(regionalreliefimage);
                                              regionalmapwidget->set_image(regionalmap);
                                              
                                              regionalreliefimage[index]=r;
                                              regionalreliefimage[index+1]=g;
                                              regionalreliefimage[index+2]=b;
                                          }
                                      }
                                  });
    
    appearanceclosebutton->set_callback([&mapappearancewindow]{mapappearancewindow->set_visible(0);});

    appearancedefaultbutton->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globalwindsimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&globalmapwidget,&region,&regionalminimap,&regionalminimapwidget,&regionalmapwindow,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&regionalmap,&regionalmapwidget,&oceanbutton,&deepoceanbutton,&basebutton,&grassbutton,&basetempbutton,&highbasebutton,&desertbutton,&highdesertbutton,&colddesertbutton,&eqtundrabutton,&tundrabutton,&coldbutton,&seaicebutton,&glacierbutton,&saltpanbutton,&ergbutton,&wetlandsbutton,&lakebutton,&riverbutton,&highlightbutton,&rfocused,&poix,&poiy,&focused,&colourpickerwindow,&colourbutton,&colourwheel,&redbox,&greenbox,&bluebox,&shadinglandslider,&shadinglakesslider,&shadingseaslider,&marblinglandslider,&marblinglakesslider,&marblingseaslider,&riversglobalbox,&riversregionalbox,&shadingdirectionchooser,&snowchangechooser,&seaicechooser]
                                          {
                                              initialisemapcolours(*world);
                                              
                                              // Put the correct values into the settings.
                                              
                                              shadinglandslider->set_value(world->landshading());
                                              shadinglakesslider->set_value(world->lakeshading());
                                              shadingseaslider->set_value(world->seashading());
                                              
                                              if (world->shadingdir()==2)
                                                  shadingdirectionchooser->set_selected_index(2);
                                              
                                              if (world->shadingdir()==4)
                                                  shadingdirectionchooser->set_selected_index(0);
                                              
                                              if (world->shadingdir()==6)
                                                  shadingdirectionchooser->set_selected_index(1);
                                              
                                              if (world->shadingdir()==8)
                                                  shadingdirectionchooser->set_selected_index(3);
                                              
                                              if (world->snowchange()==1)
                                                  snowchangechooser->set_selected_index(0);
                                              
                                              if (world->snowchange()==2)
                                                  snowchangechooser->set_selected_index(1);
                                              
                                              if (world->snowchange()==3)
                                                  snowchangechooser->set_selected_index(2);
                                              
                                              if (world->seaiceappearance()==1)
                                                  seaicechooser->set_selected_index(0);
                                              
                                              if (world->seaiceappearance()==2)
                                                  seaicechooser->set_selected_index(1);
                                              
                                              if (world->seaiceappearance()==3)
                                                  seaicechooser->set_selected_index(2);

                                              marblinglandslider->set_value(world->landmarbling());
                                              marblinglakesslider->set_value(world->lakemarbling());
                                              marblingseaslider->set_value(world->seamarbling());
                                              
                                              riversglobalbox->set_value(world->minriverflowglobal());
                                              riversregionalbox->set_value(world->minriverflowregional());
                                              
                                              // Put the correct colours into the colour pickers.
                                              
                                              seaicebutton->set_background_color(Color(world->seaice1(),world->seaice2(),world->seaice3(),255));
                                              oceanbutton->set_background_color(Color(world->ocean1(),world->ocean2(),world->ocean3(),255));
                                              deepoceanbutton->set_background_color(Color(world->deepocean1(),world->deepocean2(),world->deepocean3(),255));
                                              basebutton->set_background_color(Color(world->base1(),world->base2(),world->base3(),255));
                                              grassbutton->set_background_color(Color(world->grass1(),world->grass2(),world->grass3(),255));
                                              basetempbutton->set_background_color(Color(world->basetemp1(),world->basetemp2(),world->basetemp3(),255));
                                              highbasebutton->set_background_color(Color(world->highbase1(),world->highbase2(),world->highbase3(),255));
                                              desertbutton->set_background_color(Color(world->desert1(),world->desert2(),world->desert3(),255));
                                              highdesertbutton->set_background_color(Color(world->highdesert1(),world->highdesert2(),world->highdesert3(),255));
                                              colddesertbutton->set_background_color(Color(world->colddesert1(),world->colddesert2(),world->colddesert3(),255));
                                              tundrabutton->set_background_color(Color(world->tundra1(),world->tundra2(),world->tundra3(),255));
                                              eqtundrabutton->set_background_color(Color(world->eqtundra1(),world->eqtundra2(),world->eqtundra3(),255));
                                              coldbutton->set_background_color(Color(world->cold1(),world->cold2(),world->cold3(),255));
                                              saltpanbutton->set_background_color(Color(world->saltpan1(),world->saltpan2(),world->saltpan3(),255));
                                              ergbutton->set_background_color(Color(world->erg1(),world->erg2(),world->erg3(),255));
                                              wetlandsbutton->set_background_color(Color(world->wetlands1(),world->wetlands2(),world->wetlands3(),255));
                                              lakebutton->set_background_color(Color(world->lake1(),world->lake2(),world->lake3(),255));
                                              riverbutton->set_background_color(Color(world->river1(),world->river2(),world->river3(),255));
                                              glacierbutton->set_background_color(Color(world->glacier1(),world->glacier2(),world->glacier3(),255));
                                              highlightbutton->set_background_color(Color(world->highlight1(),world->highlight2(),world->highlight3(),255));
                                              
                                              // Now put the correct colour into the colourpicker window.
                                              
                                              string title=colourpickerwindow->title();
                                              int red=0;
                                              int green=0;
                                              int blue=0;
                                              
                                              if (title=="Shallow ocean")
                                              {
                                                  red=world->ocean1();
                                                  green=world->ocean2();
                                                  blue=world->ocean3();
                                              }
                                              
                                              if (title=="Deep ocean")
                                              {
                                                  red=world->deepocean1();
                                                  green=world->deepocean2();
                                                  blue=world->deepocean3();
                                              }
                                              
                                              if (title=="Base land")
                                              {
                                                  red=world->base1();
                                                  green=world->base2();
                                                  blue=world->base3();
                                              }
                                              
                                              if (title=="Grassland")
                                              {
                                                  red=world->grass1();
                                                  green=world->grass2();
                                                  blue=world->grass3();
                                              }
                                              
                                              if (title=="Low temperate")
                                              {
                                                  red=world->basetemp1();
                                                  green=world->basetemp2();
                                                  blue=world->basetemp3();
                                              }
                                              
                                              if (title=="High temperate")
                                              {
                                                  red=world->highbase1();
                                                  green=world->highbase2();
                                                  blue=world->highbase3();
                                              }
                                              
                                              if (title=="Low desert")
                                              {
                                                  red=world->desert1();
                                                  green=world->desert2();
                                                  blue=world->desert3();
                                              }
                                              
                                              if (title=="High desert")
                                              {
                                                  red=world->highdesert1();
                                                  green=world->highdesert2();
                                                  blue=world->highdesert3();
                                              }
                                              
                                              if (title=="Cold desert")
                                              {
                                                  red=world->colddesert1();
                                                  green=world->colddesert2();
                                                  blue=world->colddesert3();
                                              }
                                              
                                              if (title=="Mild tundra")
                                              {
                                                  red=world->eqtundra1();
                                                  green=world->eqtundra2();
                                                  blue=world->eqtundra3();
                                              }
                                              
                                              if (title=="Tundra")
                                              {
                                                  red=world->tundra1();
                                                  green=world->tundra2();
                                                  blue=world->tundra3();
                                              }
                                              
                                              if (title=="Arctic")
                                              {
                                                  red=world->cold1();
                                                  green=world->cold2();
                                                  blue=world->cold3();
                                              }
                                              
                                              if (title=="Sea ice")
                                              {
                                                  red=world->seaice1();
                                                  green=world->seaice2();
                                                  blue=world->seaice3();
                                              }
                                              
                                              if (title=="Glaciers")
                                              {
                                                  red=world->glacier1();
                                                  green=world->glacier2();
                                                  blue=world->glacier3();
                                              }
                                              
                                              if (title=="Salt pans")
                                              {
                                                  red=world->saltpan1();
                                                  green=world->saltpan2();
                                                  blue=world->saltpan3();
                                              }
                                              
                                              if (title=="Dunes")
                                              {
                                                  red=world->erg1();
                                                  green=world->erg2();
                                                  blue=world->erg3();
                                              }
                                              
                                              if (title=="Wetlands")
                                              {
                                                  red=world->wetlands1();
                                                  green=world->wetlands2();
                                                  blue=world->wetlands3();
                                              }
                                              
                                              if (title=="Lakes")
                                              {
                                                  red=world->lake1();
                                                  green=world->lake2();
                                                  blue=world->lake3();
                                              }
                                              
                                              if (title=="Rivers")
                                              {
                                                  red=world->river1();
                                                  green=world->river2();
                                                  blue=world->river3();
                                              }
                                              
                                              if (title=="Highlights")
                                              {
                                                  red=world->highlight1();
                                                  green=world->highlight2();
                                                  blue=world->highlight3();
                                              }
                                              
                                              Color colour(red,green,blue,255);
                                              
                                              colourwheel->set_color(colour);
                                              colourbutton->set_background_color(colour);
                                              
                                              redbox->set_value(red);
                                              greenbox->set_value(green);
                                              bluebox->set_value(blue);

                                              // Redraw the global map.
                                              
                                              for (int n=0; n<GLOBALMAPTYPES; n++)
                                                  globalmapimagecreated[n]=0;
                                              drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                              
                                              
                                              globalmap->upload(globalreliefimage);
                                              globalmapwidget->set_image(globalmap);
                                              
                                              setregionalminimap(*world,*region,globalreliefimage,*regionalminimap,globalimagewidth,globalimageheight,globalimagechannels);
                                              
                                              regionalminimapwidget->set_image(regionalminimap);
                                              
                                              int r,g,b;
                                              
                                              if (focused==1) // If there's a focus, copy the colour at that point on the map.
                                              {
                                                  int index=(poix+poiy*globalimagewidth)*globalimagechannels;
                                                  
                                                  r=globalreliefimage[index];
                                                  g=globalreliefimage[index+1];
                                                  b=globalreliefimage[index+2];
                                                  
                                                  globalreliefimage[index]=world->highlight1();
                                                  globalreliefimage[index+1]=world->highlight2();
                                                  globalreliefimage[index+2]=world->highlight3();
                                                  
                                                  globalmap->upload(globalreliefimage);
                                                  globalmapwidget->set_image(globalmap);
                                                  
                                                  globalreliefimage[index]=r;
                                                  globalreliefimage[index+1]=g;
                                                  globalreliefimage[index+2]=b;
                                              }
                                              
                                              // Redraw the regional map, if need be.
                                              
                                              if (regionalmapwindow->visible()==1)
                                              {
                                                  for (int n=0; n<GLOBALMAPTYPES; n++)
                                                      regionalmapimagecreated[n]=0;
                                                  drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                                  
                                                  regionalmap->upload(regionalreliefimage);
                                                  regionalmapwidget->set_image(regionalmap);
                                                  
                                                  if (rfocused==1)
                                                  {
                                                      int r,g,b;
                                                      
                                                      int index=((poix-region->regwidthbegin())+(poiy-region->regheightbegin())*regionalimagewidth)*regionalimagechannels;
                                                      
                                                      r=regionalreliefimage[index];
                                                      g=regionalreliefimage[index+1];
                                                      b=regionalreliefimage[index+2];
                                                      
                                                      regionalreliefimage[index]=world->highlight1();
                                                      regionalreliefimage[index+1]=world->highlight2();
                                                      regionalreliefimage[index+2]=world->highlight3();
                                                      
                                                      regionalmap->upload(regionalreliefimage);
                                                      regionalmapwidget->set_image(regionalmap);
                                                      
                                                      regionalreliefimage[index]=r;
                                                      regionalreliefimage[index+1]=g;
                                                      regionalreliefimage[index+2]=b;
                                                  }
                                              }
                                          });

    appearanceloadbutton->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globalwindsimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&globalmapwidget,&region,&regionalminimap,&regionalminimapwidget,&regionalmapwindow,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&regionalmap,&regionalmapwidget,&oceanbutton,&deepoceanbutton,&basebutton,&grassbutton,&basetempbutton,&highbasebutton,&desertbutton,&highdesertbutton,&colddesertbutton,&eqtundrabutton,&tundrabutton,&coldbutton,&seaicebutton,&glacierbutton,&saltpanbutton,&ergbutton,&wetlandsbutton,&lakebutton,&riverbutton,&highlightbutton,&rfocused,&poix,&poiy,&focused,&colourpickerwindow,&colourbutton,&colourwheel,&redbox,&greenbox,&bluebox,&shadinglandslider,&shadinglakesslider,&shadingseaslider,&marblinglandslider,&marblinglakesslider,&marblingseaslider,&riversglobalbox,&riversregionalbox,&shadingdirectionchooser]
                                       {
                                           bool found=loadsettings(*world);
                                           
                                           if (found==1)
                                           {                                               
                                               // Put the correct values into the settings.
                                               
                                               shadinglandslider->set_value(world->landshading());
                                               shadinglakesslider->set_value(world->lakeshading());
                                               shadingseaslider->set_value(world->seashading());
                                               
                                               if (world->shadingdir()==2)
                                                   shadingdirectionchooser->set_selected_index(2);
                                               
                                               if (world->shadingdir()==4)
                                                   shadingdirectionchooser->set_selected_index(0);
                                               
                                               if (world->shadingdir()==6)
                                                   shadingdirectionchooser->set_selected_index(1);
                                               
                                               if (world->shadingdir()==8)
                                                   shadingdirectionchooser->set_selected_index(3);
                                               
                                               
                                               marblinglandslider->set_value(world->landmarbling());
                                               marblinglakesslider->set_value(world->lakemarbling());
                                               marblingseaslider->set_value(world->seamarbling());
                                               
                                               riversglobalbox->set_value(world->minriverflowglobal());
                                               riversregionalbox->set_value(world->minriverflowregional());
                                               
                                               // Put the correct colours into the colour pickers.
                                               
                                               seaicebutton->set_background_color(Color(world->seaice1(),world->seaice2(),world->seaice3(),255));
                                               oceanbutton->set_background_color(Color(world->ocean1(),world->ocean2(),world->ocean3(),255));
                                               deepoceanbutton->set_background_color(Color(world->deepocean1(),world->deepocean2(),world->deepocean3(),255));
                                               basebutton->set_background_color(Color(world->base1(),world->base2(),world->base3(),255));
                                               grassbutton->set_background_color(Color(world->grass1(),world->grass2(),world->grass3(),255));
                                               basetempbutton->set_background_color(Color(world->basetemp1(),world->basetemp2(),world->basetemp3(),255));
                                               highbasebutton->set_background_color(Color(world->highbase1(),world->highbase2(),world->highbase3(),255));
                                               desertbutton->set_background_color(Color(world->desert1(),world->desert2(),world->desert3(),255));
                                               highdesertbutton->set_background_color(Color(world->highdesert1(),world->highdesert2(),world->highdesert3(),255));
                                               colddesertbutton->set_background_color(Color(world->colddesert1(),world->colddesert2(),world->colddesert3(),255));
                                               tundrabutton->set_background_color(Color(world->tundra1(),world->tundra2(),world->tundra3(),255));
                                               eqtundrabutton->set_background_color(Color(world->eqtundra1(),world->eqtundra2(),world->eqtundra3(),255));
                                               coldbutton->set_background_color(Color(world->cold1(),world->cold2(),world->cold3(),255));
                                               saltpanbutton->set_background_color(Color(world->saltpan1(),world->saltpan2(),world->saltpan3(),255));
                                               ergbutton->set_background_color(Color(world->erg1(),world->erg2(),world->erg3(),255));
                                               wetlandsbutton->set_background_color(Color(world->wetlands1(),world->wetlands2(),world->wetlands3(),255));
                                               lakebutton->set_background_color(Color(world->lake1(),world->lake2(),world->lake3(),255));
                                               riverbutton->set_background_color(Color(world->river1(),world->river2(),world->river3(),255));
                                               glacierbutton->set_background_color(Color(world->glacier1(),world->glacier2(),world->glacier3(),255));
                                               highlightbutton->set_background_color(Color(world->highlight1(),world->highlight2(),world->highlight3(),255));
                                               
                                               // Now put the correct colour into the colourpicker window.
                                               
                                               string title=colourpickerwindow->title();
                                               int red=0;
                                               int green=0;
                                               int blue=0;
                                               
                                               if (title=="Shallow ocean")
                                               {
                                                   red=world->ocean1();
                                                   green=world->ocean2();
                                                   blue=world->ocean3();
                                               }
                                               
                                               if (title=="Deep ocean")
                                               {
                                                   red=world->deepocean1();
                                                   green=world->deepocean2();
                                                   blue=world->deepocean3();
                                               }
                                               
                                               if (title=="Base land")
                                               {
                                                   red=world->base1();
                                                   green=world->base2();
                                                   blue=world->base3();
                                               }
                                               
                                               if (title=="Grassland")
                                               {
                                                   red=world->grass1();
                                                   green=world->grass2();
                                                   blue=world->grass3();
                                               }
                                               
                                               if (title=="Low temperate")
                                               {
                                                   red=world->basetemp1();
                                                   green=world->basetemp2();
                                                   blue=world->basetemp3();
                                               }
                                               
                                               if (title=="High temperate")
                                               {
                                                   red=world->highbase1();
                                                   green=world->highbase2();
                                                   blue=world->highbase3();
                                               }
                                               
                                               if (title=="Low desert")
                                               {
                                                   red=world->desert1();
                                                   green=world->desert2();
                                                   blue=world->desert3();
                                               }
                                               
                                               if (title=="High desert")
                                               {
                                                   red=world->highdesert1();
                                                   green=world->highdesert2();
                                                   blue=world->highdesert3();
                                               }
                                               
                                               if (title=="Cold desert")
                                               {
                                                   red=world->colddesert1();
                                                   green=world->colddesert2();
                                                   blue=world->colddesert3();
                                               }
                                               
                                               if (title=="Mild tundra")
                                               {
                                                   red=world->eqtundra1();
                                                   green=world->eqtundra2();
                                                   blue=world->eqtundra3();
                                               }
                                               
                                               if (title=="Tundra")
                                               {
                                                   red=world->tundra1();
                                                   green=world->tundra2();
                                                   blue=world->tundra3();
                                               }
                                               
                                               if (title=="Arctic")
                                               {
                                                   red=world->cold1();
                                                   green=world->cold2();
                                                   blue=world->cold3();
                                               }
                                               
                                               if (title=="Sea ice")
                                               {
                                                   red=world->seaice1();
                                                   green=world->seaice2();
                                                   blue=world->seaice3();
                                               }
                                               
                                               if (title=="Glaciers")
                                               {
                                                   red=world->glacier1();
                                                   green=world->glacier2();
                                                   blue=world->glacier3();
                                               }
                                               
                                               if (title=="Salt pans")
                                               {
                                                   red=world->saltpan1();
                                                   green=world->saltpan2();
                                                   blue=world->saltpan3();
                                               }
                                               
                                               if (title=="Dunes")
                                               {
                                                   red=world->erg1();
                                                   green=world->erg2();
                                                   blue=world->erg3();
                                               }
                                               
                                               if (title=="Wetlands")
                                               {
                                                   red=world->wetlands1();
                                                   green=world->wetlands2();
                                                   blue=world->wetlands3();
                                               }
                                               
                                               if (title=="Lakes")
                                               {
                                                   red=world->lake1();
                                                   green=world->lake2();
                                                   blue=world->lake3();
                                               }
                                               
                                               if (title=="Rivers")
                                               {
                                                   red=world->river1();
                                                   green=world->river2();
                                                   blue=world->river3();
                                               }
                                               
                                               if (title=="Highlights")
                                               {
                                                   red=world->highlight1();
                                                   green=world->highlight2();
                                                   blue=world->highlight3();
                                               }
                                               
                                               Color colour(red,green,blue,255);
                                               
                                               colourwheel->set_color(colour);
                                               colourbutton->set_background_color(colour);
                                               
                                               redbox->set_value(red);
                                               greenbox->set_value(green);
                                               bluebox->set_value(blue);
                                               
                                               // Redraw the global map.
                                               
                                               for (int n=0; n<GLOBALMAPTYPES; n++)
                                                   globalmapimagecreated[n]=0;
                                               drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                               
                                               
                                               globalmap->upload(globalreliefimage);
                                               globalmapwidget->set_image(globalmap);
                                               
                                               setregionalminimap(*world,*region,globalreliefimage,*regionalminimap,globalimagewidth,globalimageheight,globalimagechannels);
                                               
                                               regionalminimapwidget->set_image(regionalminimap);
                                               
                                               int r,g,b;
                                               
                                               if (focused==1) // If there's a focus, copy the colour at that point on the map.
                                               {
                                                   int index=(poix+poiy*globalimagewidth)*globalimagechannels;
                                                   
                                                   r=globalreliefimage[index];
                                                   g=globalreliefimage[index+1];
                                                   b=globalreliefimage[index+2];
                                                   
                                                   globalreliefimage[index]=world->highlight1();
                                                   globalreliefimage[index+1]=world->highlight2();
                                                   globalreliefimage[index+2]=world->highlight3();
                                                   
                                                   globalmap->upload(globalreliefimage);
                                                   globalmapwidget->set_image(globalmap);
                                                   
                                                   globalreliefimage[index]=r;
                                                   globalreliefimage[index+1]=g;
                                                   globalreliefimage[index+2]=b;
                                               }
                                               
                                               // Redraw the regional map, if need be.
                                               
                                               if (regionalmapwindow->visible()==1)
                                               {
                                                   for (int n=0; n<GLOBALMAPTYPES; n++)
                                                       regionalmapimagecreated[n]=0;
                                                   drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                                   
                                                   regionalmap->upload(regionalreliefimage);
                                                   regionalmapwidget->set_image(regionalmap);
                                                   
                                                   if (rfocused==1)
                                                   {
                                                       int r,g,b;
                                                       
                                                       int index=((poix-region->regwidthbegin())+(poiy-region->regheightbegin())*regionalimagewidth)*regionalimagechannels;
                                                       
                                                       r=regionalreliefimage[index];
                                                       g=regionalreliefimage[index+1];
                                                       b=regionalreliefimage[index+2];
                                                       
                                                       regionalreliefimage[index]=world->highlight1();
                                                       regionalreliefimage[index+1]=world->highlight2();
                                                       regionalreliefimage[index+2]=world->highlight3();
                                                       
                                                       regionalmap->upload(regionalreliefimage);
                                                       regionalmapwidget->set_image(regionalmap);
                                                       
                                                       regionalreliefimage[index]=r;
                                                       regionalreliefimage[index+1]=g;
                                                       regionalreliefimage[index+2]=b;
                                                   }
                                               }
                                           }
                                       });

    appearancesavebutton->set_callback([&world]{savesettings(*world);});
    
    // Set up the callbacks for the colour picker widgets.
    
    colourwheel->set_callback([&colourbutton,&redbox,&greenbox,&bluebox](const Color &c)
                              {
                                  colourbutton->set_background_color(c);
                                  
                                  int r=c.r()*255;
                                  int g=c.g()*255;
                                  int b=c.b()*255;
                                  
                                  redbox->set_value(r);
                                  greenbox->set_value(g);
                                  bluebox->set_value(b);
                              });
    
    redbox->set_callback([&redbox,&colourwheel,&colourbutton] (const int &v)
                         {
                             int value=v;
                             
                             if (value>255)
                             {
                                 value=255;
                                 redbox->set_value(value);
                             }
                             
                             if (value<0)
                             {
                                 value=0;
                                 redbox->set_value(value);
                             }

                             Color colour=colourwheel->color();
                             
                             int red=value;
                             int green=colour.g()*255;
                             int blue=colour.b()*255;

                             colourwheel->set_color(Color(red,green,blue,255));
                             
                             colourbutton->set_background_color(Color(red,green,blue,255));
                         });
    
    greenbox->set_callback([&greenbox,&colourwheel,&colourbutton] (const int &v)
                           {
                               int value=v;
                               
                               if (value>255)
                               {
                                   value=255;
                                   greenbox->set_value(value);
                               }
                               
                               if (value<0)
                               {
                                   value=0;
                                   greenbox->set_value(value);
                               }

                               Color colour=colourwheel->color();
                               
                               int red=colour.g()*255;
                               int green=value;
                               int blue=colour.b()*255;

                               colourwheel->set_color(Color(red,green,blue,255));
                               
                               colourbutton->set_background_color(Color(red,green,blue,255));
                           });
    
    bluebox->set_callback([&bluebox,&colourwheel,&colourbutton] (const int &v)
                          {
                              int value=v;
                              
                              if (value>255)
                              {
                                  value=255;
                                  bluebox->set_value(value);
                              }
                              
                              if (value<0)
                              {
                                  value=0;
                                  bluebox->set_value(value);
                              }

                              Color colour=colourwheel->color();
                              
                              int red=colour.r()*255;
                              int green=colour.g()*255;
                              int blue=value;

                              colourwheel->set_color(Color(red,green,blue,255));
                              
                              colourbutton->set_background_color(Color(red,green,blue,255));
                          });
    
    colourclosebutton->set_callback([&colourpickerwindow]
                                    {
                                        colourpickerwindow->set_visible(0);
                                    });
    
    colourapplybutton->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globalwindsimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&globalmapwidget,&region,&regionalminimap,&regionalminimapwidget,&regionalmapwindow,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&regionalmap,&regionalmapwidget,&focused,&poix,&poiy,&rfocused,&colourpickerwindow,&redbox,&greenbox,&bluebox,&oceanbutton,&deepoceanbutton,&basebutton,&grassbutton,&basetempbutton,&highbasebutton,&desertbutton,&highdesertbutton,&colddesertbutton,&eqtundrabutton,&tundrabutton,&coldbutton,&seaicebutton,&glacierbutton,&saltpanbutton,&ergbutton,&wetlandsbutton,&lakebutton,&riverbutton,&highlightbutton]
                                   {
                                       string title=colourpickerwindow->title();
                                       
                                       int red=redbox->value();
                                       int green=greenbox->value();
                                       int blue=bluebox->value();
                                       
                                       if (title=="Shallow ocean")
                                       {
                                           oceanbutton->set_background_color(Color(red,green,blue,255));
                                           world->setocean1(red);
                                           world->setocean2(green);
                                           world->setocean3(blue);
                                       }
                                       
                                       if (title=="Deep ocean")
                                       {
                                           deepoceanbutton->set_background_color(Color(red,green,blue,255));
                                           world->setdeepocean1(red);
                                           world->setdeepocean2(green);
                                           world->setdeepocean3(blue);
                                       }
                                       
                                       if (title=="Base land")
                                       {
                                           basebutton->set_background_color(Color(red,green,blue,255));
                                           world->setbase1(red);
                                           world->setbase2(green);
                                           world->setbase3(blue);
                                       }
                                       
                                       if (title=="Grassland")
                                       {
                                           grassbutton->set_background_color(Color(red,green,blue,255));
                                           world->setgrass1(red);
                                           world->setgrass2(green);
                                           world->setgrass3(blue);
                                       }
                                       
                                       if (title=="Low temperate")
                                       {
                                           basetempbutton->set_background_color(Color(red,green,blue,255));
                                           world->setbasetemp1(red);
                                           world->setbasetemp2(green);
                                           world->setbasetemp3(blue);
                                       }
                                       
                                       if (title=="High temperate")
                                       {
                                           highbasebutton->set_background_color(Color(red,green,blue,255));
                                           world->sethighbase1(red);
                                           world->sethighbase2(green);
                                           world->sethighbase3(blue);
                                       }
                                       
                                       if (title=="Low desert")
                                       {
                                           desertbutton->set_background_color(Color(red,green,blue,255));
                                           world->setdesert1(red);
                                           world->setdesert2(green);
                                           world->setdesert3(blue);
                                       }
                                       
                                       if (title=="High desert")
                                       {
                                           highdesertbutton->set_background_color(Color(red,green,blue,255));
                                           world->sethighdesert1(red);
                                           world->sethighdesert2(green);
                                           world->sethighdesert3(blue);
                                       }
                                       
                                       if (title=="Cold desert")
                                       {
                                           colddesertbutton->set_background_color(Color(red,green,blue,255));
                                           world->setcolddesert1(red);
                                           world->setcolddesert2(green);
                                           world->setcolddesert3(blue);
                                       }
                                       
                                       if (title=="Mild tundra")
                                       {
                                           eqtundrabutton->set_background_color(Color(red,green,blue,255));
                                           world->seteqtundra1(red);
                                           world->seteqtundra2(green);
                                           world->seteqtundra3(blue);
                                       }
                                       
                                       if (title=="Tundra")
                                       {
                                           tundrabutton->set_background_color(Color(red,green,blue,255));
                                           world->settundra1(red);
                                           world->settundra2(green);
                                           world->settundra3(blue);
                                       }
                                       
                                       if (title=="Arctic")
                                       {
                                           coldbutton->set_background_color(Color(red,green,blue,255));
                                           world->setcold1(red);
                                           world->setcold2(green);
                                           world->setcold3(blue);
                                       }
                                       
                                       if (title=="Sea ice")
                                       {
                                           seaicebutton->set_background_color(Color(red,green,blue,255));
                                           world->setseaice1(red);
                                           world->setseaice2(green);
                                           world->setseaice3(blue);
                                       }
                                       
                                       if (title=="Glaciers")
                                       {
                                           glacierbutton->set_background_color(Color(red,green,blue,255));
                                           world->setglacier1(red);
                                           world->setglacier2(green);
                                           world->setglacier3(blue);
                                       }
                                       
                                       if (title=="Salt pans")
                                       {
                                           saltpanbutton->set_background_color(Color(red,green,blue,255));
                                           world->setsaltpan1(red);
                                           world->setsaltpan2(green);
                                           world->setsaltpan3(blue);
                                       }
                                       
                                       if (title=="Dunes")
                                       {
                                           ergbutton->set_background_color(Color(red,green,blue,255));
                                           world->seterg1(red);
                                           world->seterg2(green);
                                           world->seterg3(blue);
                                       }
                                       
                                       if (title=="Wetlands")
                                       {
                                           wetlandsbutton->set_background_color(Color(red,green,blue,255));
                                           world->setwetlands1(red);
                                           world->setwetlands2(green);
                                           world->setwetlands3(blue);
                                       }
                                       
                                       if (title=="Lakes")
                                       {
                                           lakebutton->set_background_color(Color(red,green,blue,255));
                                           world->setlake1(red);
                                           world->setlake2(green);
                                           world->setlake3(blue);
                                       }
                                       
                                       if (title=="Rivers")
                                       {
                                           riverbutton->set_background_color(Color(red,green,blue,255));
                                           world->setriver1(red);
                                           world->setriver2(green);
                                           world->setriver3(blue);
                                       }
                                       
                                       if (title=="Highlights")
                                       {
                                           highlightbutton->set_background_color(Color(red,green,blue,255));
                                           world->sethighlight1(red);
                                           world->sethighlight2(green);
                                           world->sethighlight3(blue);
                                       }
                                       
                                       // Redraw the global map.
                                       
                                       for (int n=0; n<GLOBALMAPTYPES; n++)
                                           globalmapimagecreated[n]=0;
                                       drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                       
                                       
                                       globalmap->upload(globalreliefimage);
                                       globalmapwidget->set_image(globalmap);
                                       
                                       setregionalminimap(*world,*region,globalreliefimage,*regionalminimap,globalimagewidth,globalimageheight,globalimagechannels);
                                       
                                       regionalminimapwidget->set_image(regionalminimap);
                                       
                                       int r,g,b;
                                       
                                       if (focused==1) // If there's a focus, copy the colour at that point on the map.
                                       {
                                           int index=(poix+poiy*globalimagewidth)*globalimagechannels;
                                           
                                           r=globalreliefimage[index];
                                           g=globalreliefimage[index+1];
                                           b=globalreliefimage[index+2];
                                           
                                           globalreliefimage[index]=world->highlight1();
                                           globalreliefimage[index+1]=world->highlight2();
                                           globalreliefimage[index+2]=world->highlight3();
                                           
                                           globalmap->upload(globalreliefimage);
                                           globalmapwidget->set_image(globalmap);
                                           
                                           globalreliefimage[index]=r;
                                           globalreliefimage[index+1]=g;
                                           globalreliefimage[index+2]=b;
                                       }
                                       
                                       // Redraw the regional map, if need be.
                                       
                                       if (regionalmapwindow->visible()==1)
                                       {
                                           for (int n=0; n<GLOBALMAPTYPES; n++)
                                               regionalmapimagecreated[n]=0;
                                           drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                           
                                           regionalmap->upload(regionalreliefimage);
                                           regionalmapwidget->set_image(regionalmap);
                                           
                                           if (rfocused==1)
                                           {
                                               int r,g,b;
                                               
                                               int index=((poix-region->regwidthbegin())+(poiy-region->regheightbegin())*regionalimagewidth)*regionalimagechannels;
                                               
                                               r=regionalreliefimage[index];
                                               g=regionalreliefimage[index+1];
                                               b=regionalreliefimage[index+2];
                                               
                                               regionalreliefimage[index]=world->highlight1();
                                               regionalreliefimage[index+1]=world->highlight2();
                                               regionalreliefimage[index+2]=world->highlight3();
                                               
                                               regionalmap->upload(regionalreliefimage);
                                               regionalmapwidget->set_image(regionalmap);
                                               
                                               regionalreliefimage[index]=r;
                                               regionalreliefimage[index+1]=g;
                                               regionalreliefimage[index+2]=b;
                                           }
                                       }
                                   });
    

    // Set up the callbacks for the world map widgets.
    
    newworldbutton->set_callback([&globalmapwindow,&getseedwindow,&seedinput,&cancelbutton,&brandnew,&OKbutton]
                                 {
                                     brandnew=0;
                                     seedinput->set_value("");
                                     cancelbutton->set_caption("Cancel");
                                     //seedinput->set_size(Vector2i(185,30));
                                     //getseedwindow->set_size(Vector2i(215,115));
                                     OKbutton->set_pushed(0);
                                     getseedwindow->set_visible(1);
                                     globalmapwindow->set_visible(0);
                                 });
    
    loadworldbutton->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globalwindsimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&globalmapwidget,&region,&regionalminimap,&regionalminimapwidget,&regionalmapwindow,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&regionalmap,&regionalmapwidget,&oceanbutton,&deepoceanbutton,&basebutton,&grassbutton,&basetempbutton,&highbasebutton,&desertbutton,&highdesertbutton,&colddesertbutton,&eqtundrabutton,&tundrabutton,&coldbutton,&seaicebutton,&glacierbutton,&saltpanbutton,&ergbutton,&wetlandsbutton,&lakebutton,&riverbutton,&highlightbutton,&rfocused,&poix,&poiy,&focused,&colourpickerwindow,&colourbutton,&colourwheel,&redbox,&greenbox,&bluebox,&shadinglandslider,&shadinglakesslider,&shadingseaslider,&marblinglandslider,&marblinglakesslider,&marblingseaslider,&riversglobalbox,&riversregionalbox,&shadingdirectionchooser,&focusinfo,&globalmapwindow]
                                {
                                    bool found=loadworld(*world);
                                    
                                    if (found==1)
                                    {
                                        // Put the correct values into the settings.
                                        
                                        shadinglandslider->set_value(world->landshading());
                                        shadinglakesslider->set_value(world->lakeshading());
                                        shadingseaslider->set_value(world->seashading());
                                        
                                        if (world->shadingdir()==2)
                                            shadingdirectionchooser->set_selected_index(2);
                                        
                                        if (world->shadingdir()==4)
                                            shadingdirectionchooser->set_selected_index(0);
                                        
                                        if (world->shadingdir()==6)
                                            shadingdirectionchooser->set_selected_index(1);
                                        
                                        if (world->shadingdir()==8)
                                            shadingdirectionchooser->set_selected_index(3);
                                        
                                        
                                        marblinglandslider->set_value(world->landmarbling());
                                        marblinglakesslider->set_value(world->lakemarbling());
                                        marblingseaslider->set_value(world->seamarbling());
                                        
                                        riversglobalbox->set_value(world->minriverflowglobal());
                                        riversregionalbox->set_value(world->minriverflowregional());
                                        
                                        // Put the correct colours into the colour pickers.
                                        
                                        seaicebutton->set_background_color(Color(world->seaice1(),world->seaice2(),world->seaice3(),255));
                                        oceanbutton->set_background_color(Color(world->ocean1(),world->ocean2(),world->ocean3(),255));
                                        deepoceanbutton->set_background_color(Color(world->deepocean1(),world->deepocean2(),world->deepocean3(),255));
                                        basebutton->set_background_color(Color(world->base1(),world->base2(),world->base3(),255));
                                        grassbutton->set_background_color(Color(world->grass1(),world->grass2(),world->grass3(),255));
                                        basetempbutton->set_background_color(Color(world->basetemp1(),world->basetemp2(),world->basetemp3(),255));
                                        highbasebutton->set_background_color(Color(world->highbase1(),world->highbase2(),world->highbase3(),255));
                                        desertbutton->set_background_color(Color(world->desert1(),world->desert2(),world->desert3(),255));
                                        highdesertbutton->set_background_color(Color(world->highdesert1(),world->highdesert2(),world->highdesert3(),255));
                                        colddesertbutton->set_background_color(Color(world->colddesert1(),world->colddesert2(),world->colddesert3(),255));
                                        tundrabutton->set_background_color(Color(world->tundra1(),world->tundra2(),world->tundra3(),255));
                                        eqtundrabutton->set_background_color(Color(world->eqtundra1(),world->eqtundra2(),world->eqtundra3(),255));
                                        coldbutton->set_background_color(Color(world->cold1(),world->cold2(),world->cold3(),255));
                                        saltpanbutton->set_background_color(Color(world->saltpan1(),world->saltpan2(),world->saltpan3(),255));
                                        ergbutton->set_background_color(Color(world->erg1(),world->erg2(),world->erg3(),255));
                                        wetlandsbutton->set_background_color(Color(world->wetlands1(),world->wetlands2(),world->wetlands3(),255));
                                        lakebutton->set_background_color(Color(world->lake1(),world->lake2(),world->lake3(),255));
                                        riverbutton->set_background_color(Color(world->river1(),world->river2(),world->river3(),255));
                                        glacierbutton->set_background_color(Color(world->glacier1(),world->glacier2(),world->glacier3(),255));
                                        highlightbutton->set_background_color(Color(world->highlight1(),world->highlight2(),world->highlight3(),255));
                                        
                                        // Now put the correct colour into the colourpicker window.
                                        
                                        string title=colourpickerwindow->title();
                                        int red=0;
                                        int green=0;
                                        int blue=0;
                                        
                                        if (title=="Shallow ocean")
                                        {
                                            red=world->ocean1();
                                            green=world->ocean2();
                                            blue=world->ocean3();
                                        }
                                        
                                        if (title=="Deep ocean")
                                        {
                                            red=world->deepocean1();
                                            green=world->deepocean2();
                                            blue=world->deepocean3();
                                        }
                                        
                                        if (title=="Base land")
                                        {
                                            red=world->base1();
                                            green=world->base2();
                                            blue=world->base3();
                                        }
                                        
                                        if (title=="Grassland")
                                        {
                                            red=world->grass1();
                                            green=world->grass2();
                                            blue=world->grass3();
                                        }
                                        
                                        if (title=="Low temperate")
                                        {
                                            red=world->basetemp1();
                                            green=world->basetemp2();
                                            blue=world->basetemp3();
                                        }
                                        
                                        if (title=="High temperate")
                                        {
                                            red=world->highbase1();
                                            green=world->highbase2();
                                            blue=world->highbase3();
                                        }
                                        
                                        if (title=="Low desert")
                                        {
                                            red=world->desert1();
                                            green=world->desert2();
                                            blue=world->desert3();
                                        }
                                        
                                        if (title=="High desert")
                                        {
                                            red=world->highdesert1();
                                            green=world->highdesert2();
                                            blue=world->highdesert3();
                                        }
                                        
                                        if (title=="Cold desert")
                                        {
                                            red=world->colddesert1();
                                            green=world->colddesert2();
                                            blue=world->colddesert3();
                                        }
                                        
                                        if (title=="Mild tundra")
                                        {
                                            red=world->eqtundra1();
                                            green=world->eqtundra2();
                                            blue=world->eqtundra3();
                                        }
                                        
                                        if (title=="Tundra")
                                        {
                                            red=world->tundra1();
                                            green=world->tundra2();
                                            blue=world->tundra3();
                                        }
                                        
                                        if (title=="Arctic")
                                        {
                                            red=world->cold1();
                                            green=world->cold2();
                                            blue=world->cold3();
                                        }
                                        
                                        if (title=="Sea ice")
                                        {
                                            red=world->seaice1();
                                            green=world->seaice2();
                                            blue=world->seaice3();
                                        }
                                        
                                        if (title=="Glaciers")
                                        {
                                            red=world->glacier1();
                                            green=world->glacier2();
                                            blue=world->glacier3();
                                        }
                                        
                                        if (title=="Salt pans")
                                        {
                                            red=world->saltpan1();
                                            green=world->saltpan2();
                                            blue=world->saltpan3();
                                        }
                                        
                                        if (title=="Dunes")
                                        {
                                            red=world->erg1();
                                            green=world->erg2();
                                            blue=world->erg3();
                                        }
                                        
                                        if (title=="Wetlands")
                                        {
                                            red=world->wetlands1();
                                            green=world->wetlands2();
                                            blue=world->wetlands3();
                                        }
                                        
                                        if (title=="Lakes")
                                        {
                                            red=world->lake1();
                                            green=world->lake2();
                                            blue=world->lake3();
                                        }
                                        
                                        if (title=="Rivers")
                                        {
                                            red=world->river1();
                                            green=world->river2();
                                            blue=world->river3();
                                        }
                                        
                                        if (title=="Highlights")
                                        {
                                            red=world->highlight1();
                                            green=world->highlight2();
                                            blue=world->highlight3();
                                        }
                                        
                                        Color colour(red,green,blue,255);
                                        
                                        colourwheel->set_color(colour);
                                        colourbutton->set_background_color(colour);
                                        
                                        redbox->set_value(red);
                                        greenbox->set_value(green);
                                        bluebox->set_value(blue);

                                        for (int n=0; n<GLOBALMAPTYPES; n++)
                                            globalmapimagecreated[n]=0;
                                        
                                        mapview=relief;
                                        
                                        drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);

                                        globalmap->upload(globalreliefimage);
                                        globalmapwidget->set_image(globalmap);
                                        globalmapwidget->set_offset(Vector2f(0,0));
                                        globalmapwidget->set_scale(0.5);
                                        
                                        focused=0;
                                        
                                        focusinfo->set_caption("Welcome to a new world!");
                                        
                                        string windowname="World seed: "+to_string(world->seed());
                                        globalmapwindow->set_title(windowname);
                                    }
                                });
    
    saveworldbutton->set_callback([&world]{saveworld(*world);});
    
    globalimportbutton->set_callback([&warningwindow]
                                     {
                                         warningwindow->set_visible(1);
                                         warningwindow->request_focus();
                                     });
    
    exportworldmapsbutton->set_callback([&world,&focused,&globalmapimagecreated,&mapview,&globalmap,&globalmapwidget,&focusinfo,&globalmapwindow,&globalelevationimage,&globalwindsimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels]
                                        {
                                            // First, get the save directory.
                                            
                                            string filename=file_dialog({{"png", "Portable Network Graphics"}},true);
                                            
                                            if (filename!="")
                                            {
                                                // Now, draw all the maps.
                                                
                                                mapviewenum oldmapview=mapview;
                                                
                                                mapview=relief;
                                                
                                                drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                                
                                                mapview=elevation;
                                                
                                                drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                                
                                                mapview=winds;
                                                
                                                drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                                
                                                mapview=temperature;
                                                
                                                drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                                
                                                mapview=precipitation;
                                                
                                                drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                                
                                                mapview=climate;
                                                
                                                drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                                
                                                mapview=rivers;
                                                
                                                drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                                
                                                // Now we need to convert these into actual images that we can save.

                                                int width=world->width()+1;
                                                int height=world->height()+1;
                                                
                                                filename.resize(filename.size()-4);
                                                
                                                saveimage(globalreliefimage,globalimagechannels,width,height,filename+" Relief.png");
                                                
                                                saveimage(globalelevationimage,globalimagechannels,width,height,filename+" Elevation.png");
                                                
                                                saveimage(globalwindsimage,globalimagechannels,width,height,filename+"Winds.png");
                                                
                                                saveimage(globaltemperatureimage,globalimagechannels,width,height,filename+"Temperature.png");
                                                
                                                saveimage(globalprecipitationimage,globalimagechannels,width,height,filename+"Precipitation.png");
                                                
                                                saveimage(globalclimateimage,globalimagechannels,width,height,filename+"Climate.png");
                                                
                                                saveimage(globalriversimage,globalimagechannels,width,height,filename+"Rivers.png");
                                                
                                                mapview=oldmapview;
                                            }
                                        });
    
   exportareamapsbutton->set_callback([&areaexportwindow,&areaexportmapwidget,&globalmap,&globalmapwidget,&globalmapwindow,&globalreliefimage,&areafromregional]
                                      {
                                          areaexportwindow->set_position(globalmapwindow->position());
                                          
                                          globalmap->upload(globalreliefimage);
                                          
                                          areaexportmapwidget->set_image(globalmap);
                                          areaexportmapwidget->set_scale(globalmapwidget->scale());
                                          areaexportmapwidget->set_offset(globalmapwidget->offset());
                                          
                                          globalmapwindow->set_visible(0);
                                          areaexportwindow->set_visible(1);
                                          areaexportwindow->request_focus();
                                          
                                          areafromregional=0;
                                      });
    
    
    reliefbutton->set_callback([&world,&globalmapwidget,&globalmap,&mapview,&globalreliefimage,&focused,&globalimagewidth,&globalimagechannels,&poix,&poiy]
                               {
                                   mapview=relief;
                                   
                                   int r,g,b;
                                   
                                   if (focused==1) // If there's a focus, copy the colour at that point on the map.
                                   {
                                       int index=(poix+poiy*globalimagewidth)*globalimagechannels;
                                       
                                       r=globalreliefimage[index];
                                       g=globalreliefimage[index+1];
                                       b=globalreliefimage[index+2];
                                       
                                       globalreliefimage[index]=world->highlight1();
                                       globalreliefimage[index+1]=world->highlight2();
                                       globalreliefimage[index+2]=world->highlight3();
                                   }
                                   
                                   globalmap->upload(globalreliefimage);
                                   globalmapwidget->set_image(globalmap);
                                   
                                   if (focused==1) // Now just put the colour back again.
                                   {
                                       int index=(poix+poiy*globalimagewidth)*globalimagechannels;
                                       
                                       globalreliefimage[index]=r;
                                       globalreliefimage[index+1]=g;
                                       globalreliefimage[index+2]=b;
                                   }
                               });
    
    elevationbutton->set_callback([&globalmapwidget,&globalmap,&mapview,&world,&globalmapimagecreated,&globalelevationimage,&globalwindsimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&focused,&poix,&poiy]
                                  {
                                      mapview=elevation;
                                      
                                      int r,g,b;
                                      
                                      drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                      
                                      if (focused==1) // If there's a focus, copy the colour at that point on the map.
                                      {
                                          int index=(poix+poiy*globalimagewidth)*globalimagechannels;
                                          
                                          r=globalelevationimage[index];
                                          g=globalelevationimage[index+1];
                                          b=globalelevationimage[index+2];
                                          
                                          globalelevationimage[index]=world->highlight1();
                                          globalelevationimage[index+1]=world->highlight2();
                                          globalelevationimage[index+2]=world->highlight3();
                                      }
                                      
                                      globalmap->upload(globalelevationimage);
                                      globalmapwidget->set_image(globalmap);
                                      
                                      if (focused==1) // Now just put the colour back again.
                                      {
                                          int index=(poix+poiy*globalimagewidth)*globalimagechannels;
                                          
                                          globalelevationimage[index]=r;
                                          globalelevationimage[index+1]=g;
                                          globalelevationimage[index+2]=b;
                                      }
                                  });
    
    temperaturebutton->set_callback([&globalmapwidget,&globalmap,&mapview,&world,&globalmapimagecreated,&globalelevationimage,&globalwindsimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&focused,&poix,&poiy]
                                    {
                                        mapview=temperature;
                                        
                                        int r,g,b;
                                        
                                        drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                        
                                        if (focused==1) // If there's a focus, copy the colour at that point on the map.
                                        {
                                            int index=(poix+poiy*globalimagewidth)*globalimagechannels;
                                            
                                            r=globaltemperatureimage[index];
                                            g=globaltemperatureimage[index+1];
                                            b=globaltemperatureimage[index+2];
                                            
                                            globaltemperatureimage[index]=world->highlight1();
                                            globaltemperatureimage[index+1]=world->highlight2();
                                            globaltemperatureimage[index+2]=world->highlight3();
                                        }
                                        
                                        globalmap->upload(globaltemperatureimage);
                                        globalmapwidget->set_image(globalmap);
                                        
                                        if (focused==1) // Now just put the colour back again.
                                        {
                                            int index=(poix+poiy*globalimagewidth)*globalimagechannels;
                                            
                                            globaltemperatureimage[index]=r;
                                            globaltemperatureimage[index+1]=g;
                                            globaltemperatureimage[index+2]=b;
                                        }
                                    });
    
    precipitationbutton->set_callback([&globalmapwidget,&globalmap,&mapview,&world,&globalmapimagecreated,&globalelevationimage,&globalwindsimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&focused,&poix,&poiy]
                                      {
                                          mapview=precipitation;
                                          
                                          int r,g,b;
                                        
                                          drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                          
                                          if (focused==1) // If there's a focus, copy the colour at that point on the map.
                                          {
                                              int index=(poix+poiy*globalimagewidth)*globalimagechannels;
                                              
                                              r=globalprecipitationimage[index];
                                              g=globalprecipitationimage[index+1];
                                              b=globalprecipitationimage[index+2];
                                              
                                              globalprecipitationimage[index]=world->highlight1();
                                              globalprecipitationimage[index+1]=world->highlight2();
                                              globalprecipitationimage[index+2]=world->highlight3();
                                          }
                                          
                                          globalmap->upload(globalprecipitationimage);
                                          globalmapwidget->set_image(globalmap);
                                          
                                          if (focused==1) // Now just put the colour back again.
                                          {
                                              int index=(poix+poiy*globalimagewidth)*globalimagechannels;
                                              
                                              globalprecipitationimage[index]=r;
                                              globalprecipitationimage[index+1]=g;
                                              globalprecipitationimage[index+2]=b;
                                          }
                                      });
    
    climatebutton->set_callback([&globalmapwidget,&globalmap,&mapview,&world,&globalmapimagecreated,&globalelevationimage,&globalwindsimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&focused,&poix,&poiy]
                                {
                                    mapview=climate;
                                    
                                    int r,g,b;
                                    
                                    drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                    
                                    if (focused==1) // If there's a focus, copy the colour at that point on the map.
                                    {
                                        int index=(poix+poiy*globalimagewidth)*globalimagechannels;
                                        
                                        r=globalclimateimage[index];
                                        g=globalclimateimage[index+1];
                                        b=globalclimateimage[index+2];
                                        
                                        globalclimateimage[index]=world->highlight1();
                                        globalclimateimage[index+1]=world->highlight2();
                                        globalclimateimage[index+2]=world->highlight3();
                                    }
                                    
                                    globalmap->upload(globalclimateimage);
                                    globalmapwidget->set_image(globalmap);
                                    
                                    if (focused==1) // Now just put the colour back again.
                                    {
                                        int index=(poix+poiy*globalimagewidth)*globalimagechannels;
                                        
                                        globalclimateimage[index]=r;
                                        globalclimateimage[index+1]=g;
                                        globalclimateimage[index+2]=b;
                                        
                                    }
                                });
    
    riversbutton->set_callback([&globalmapwidget,&globalmap,&mapview,&world,&globalmapimagecreated,&globalelevationimage,&globalwindsimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&focused,&poix,&poiy]
                              {
                                  mapview=rivers;
                                  
                                  int r,g,b;
                                  
                                  drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                  
                                  if (focused==1) // If there's a focus, copy the colour at that point on the map.
                                  {
                                      int index=(poix+poiy*globalimagewidth)*globalimagechannels;
                                      
                                      r=globalriversimage[index];
                                      g=globalriversimage[index+1];
                                      b=globalriversimage[index+2];
                                      
                                      globalriversimage[index]=world->highlight1();
                                      globalriversimage[index+1]=world->highlight2();
                                      globalriversimage[index+2]=world->highlight3();
                                  }
                                  
                                  globalmap->upload(globalriversimage);
                                  globalmapwidget->set_image(globalmap);
                                  
                                  if (focused==1) // Now just put the colour back again.
                                  {
                                      int index=(poix+poiy*globalimagewidth)*globalimagechannels;
                                      
                                      globalriversimage[index]=r;
                                      globalriversimage[index+1]=g;
                                      globalriversimage[index+2]=b;
                                      
                                  }
                              });
    
    recentrebutton->set_callback([&globalmapwidget]
                                {
                                    globalmapwidget->set_offset(Vector2f(0,0));
                                    globalmapwidget->set_scale(0.5);
                                    
                                });
    
    focusbutton->set_callback([&world,&focused,&poix,&poiy,&screen,&globalmapwindow,&globalmapwindowmainbox,&mapandfocusbox,&globalmap,&globalmapwidget,&focusinfo,&globalelevationimage,&globalwindsimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&mapview,&globalimagewidth,&globalimagechannels]
                              {
                                  Vector2i mousepos=screen->mouse_pos();
                                  sf::Vector2i sfmousestartpos=sf::Mouse::getPosition(); // The nanogui mouse detection will only pick up where the mouse is when the button is initially clicked, so we need to use the sf mouse detection too to supplement it.
                                  
                                  this_thread::sleep_for(chrono::milliseconds{100});
                                  
                                  while (sf::Mouse::isButtonPressed(sf::Mouse::Left)==0){}
                                  
                                  sf::Vector2i sfmouseendpos=sf::Mouse::getPosition();
                                  
                                  int xdiff=sfmouseendpos.x-sfmousestartpos.x;
                                  int ydiff=sfmouseendpos.y-sfmousestartpos.y;
                                  
                                  mousepos.x()=mousepos.x()+xdiff;
                                  mousepos.y()=mousepos.y()+ydiff;
                                  
                                  bool onmap=1;
                                  
                                  if (globalmapwindow->find_widget(mousepos)==globalmapwidget)
                                  {
                                      focused=1;
                                    
                                      Vector2i mappos1=globalmapwindow->position();
                                      Vector2i mappos2=globalmapwindowmainbox->position();
                                      Vector2i mappos3=mapandfocusbox->position();
                                      Vector2i mappos4=globalmapwidget->position();
                                      
                                      Vector2f pointpos; // Coordinates of the mouse within the map widget.
                                      
                                      pointpos.x()=mousepos.x()-mappos1.x()-mappos2.x()-mappos3.x()-mappos4.x();
                                      pointpos.y()=mousepos.y()-mappos1.y()-mappos2.y()-mappos3.y()-mappos4.y();
                                      
                                      Vector2f poi=globalmapwidget->pos_to_pixel(pointpos); // Global coordinates of the point of interest.
                                      
                                      if (poi.x()<0 || poi.x()>world->width()+1 || poi.y()<0 || poi.y()>world->height()+1)
                                          onmap=0;
                                      else
                                      {
                                          poix=poi.x();
                                          poiy=poi.y();

                                          int index=(poix+poiy*globalimagewidth)*globalimagechannels;
                                          
                                          int r,g,b;
                                          
                                          switch (mapview) // First, paint the focus point onto the currently viewed map.
                                          {
                                              case elevation:
                                                  r=globalelevationimage[index];
                                                  g=globalelevationimage[index+1];
                                                  b=globalelevationimage[index+2];
                                                  
                                                  globalelevationimage[index]=world->highlight1();
                                                  globalelevationimage[index+1]=world->highlight2();
                                                  globalelevationimage[index+2]=world->highlight3();
                                                  
                                                  globalmap->upload(globalelevationimage);
                                                  
                                                  globalelevationimage[index]=r;
                                                  globalelevationimage[index+1]=g;
                                                  globalelevationimage[index+2]=b;
                                                  
                                                  break;
                                                  
                                              case winds:
                                                  r=globalwindsimage[index];
                                                  g=globalwindsimage[index+1];
                                                  b=globalwindsimage[index+2];
                                                  
                                                  globalwindsimage[index]=world->highlight1();
                                                  globalwindsimage[index+1]=world->highlight2();
                                                  globalwindsimage[index+2]=world->highlight3();
                                                  
                                                  globalmap->upload(globalwindsimage);
                                                  
                                                  globalwindsimage[index]=r;
                                                  globalwindsimage[index+1]=g;
                                                  globalwindsimage[index+2]=b;
                                                  
                                                  break;
                                                  
                                              case temperature:
                                                  r=globaltemperatureimage[index];
                                                  g=globaltemperatureimage[index+1];
                                                  b=globaltemperatureimage[index+2];
                                                  
                                                  globaltemperatureimage[index]=world->highlight1();
                                                  globaltemperatureimage[index+1]=world->highlight2();
                                                  globaltemperatureimage[index+2]=world->highlight3();
                                                  
                                                  globalmap->upload(globaltemperatureimage);
                                                  
                                                  globaltemperatureimage[index]=r;
                                                  globaltemperatureimage[index+1]=g;
                                                  globaltemperatureimage[index+2]=b;
                                                  
                                                  break;
                                                  
                                              case precipitation:
                                                  r=globalprecipitationimage[index];
                                                  g=globalprecipitationimage[index+1];
                                                  b=globalprecipitationimage[index+2];
                                                  
                                                  globalprecipitationimage[index]=world->highlight1();
                                                  globalprecipitationimage[index+1]=world->highlight2();
                                                  globalprecipitationimage[index+2]=world->highlight3();
                                                  
                                                  globalmap->upload(globalprecipitationimage);
                                                  
                                                  globalprecipitationimage[index]=r;
                                                  globalprecipitationimage[index+1]=g;
                                                  globalprecipitationimage[index+2]=b;
                                                  
                                                  break;
                                                  
                                              case climate:
                                                  r=globalclimateimage[index];
                                                  g=globalclimateimage[index+1];
                                                  b=globalclimateimage[index+2];
                                                  
                                                  globalclimateimage[index]=world->highlight1();
                                                  globalclimateimage[index+1]=world->highlight2();
                                                  globalclimateimage[index+2]=world->highlight3();
                                                  
                                                  globalmap->upload(globalclimateimage);
                                                  
                                                  globalclimateimage[index]=r;
                                                  globalclimateimage[index+1]=g;
                                                  globalclimateimage[index+2]=b;
                                                  
                                                  break;
                                                  
                                              case rivers:
                                                  r=globalriversimage[index];
                                                  g=globalriversimage[index+1];
                                                  b=globalriversimage[index+2];
                                                  
                                                  globalriversimage[index]=world->highlight1();
                                                  globalriversimage[index+1]=world->highlight2();
                                                  globalriversimage[index+2]=world->highlight3();
                                                  
                                                  globalmap->upload(globalriversimage);
                                                  
                                                  globalriversimage[index]=r;
                                                  globalriversimage[index+1]=g;
                                                  globalriversimage[index+2]=b;
                                                  
                                                  break;
                                                  
                                              case relief:
                                                  r=globalreliefimage[index];
                                                  g=globalreliefimage[index+1];
                                                  b=globalreliefimage[index+2];
                                                  
                                                  globalreliefimage[index]=world->highlight1();
                                                  globalreliefimage[index+1]=world->highlight2();
                                                  globalreliefimage[index+2]=world->highlight3();
                                                  
                                                  globalmap->upload(globalreliefimage);
                                                  
                                                  globalreliefimage[index]=r;
                                                  globalreliefimage[index+1]=g;
                                                  globalreliefimage[index+2]=b;
                                                  
                                                  break;
                                          }
                                          
                                          globalmapwidget->set_image(globalmap);

                                          int sealevel=world->sealevel();
                                          
                                          string infotext; // Now do the information about this point.
                                          
                                          infotext="Location is "+to_string(poix)+", "+to_string(poiy)+". Latitude "+to_string(world->latitude(poix,poiy))+"°. ";
                                          
                                          int wind=world->wind(poix,poiy);
                                          string winddir;
                                          
                                          if (wind>0)
                                              winddir=" westerly. ";
                                          
                                          if (wind<0)
                                              winddir=" easterly. ";
                                          
                                          if (wind==0 || wind>50)
                                              winddir=". ";
                                          
                                          if (wind<0)
                                              wind=-wind;
                                          
                                          if (wind>50)
                                              wind=0;
                                          
                                          infotext=infotext+"Wind: "+to_string(wind)+winddir;

                                          int pointelevation=world->map(poix,poiy);
                                          
                                          if (world->sea(poix,poiy)==0)
                                          {
                                              if (pointelevation>sealevel)
                                                  infotext=infotext+"Elevation: "+to_string(pointelevation-sealevel)+" metres above sea level.\n";
                                              else
                                              {
                                                  if (world->truelake(poix,poiy)==1)
                                                      infotext=infotext+"Elevation: "+to_string(sealevel-pointelevation)+" metres below sea level.\n";
                                                  else
                                                      infotext=infotext+"Elevation: "+to_string(world->lakesurface(poix,poiy)-sealevel)+" metres above sea level.\n";
                                              }
                                              
                                              string climatetype=world->climate(poix,poiy);
                                              string climate=getclimatename(climatetype)+" ("+climatetype+")";
                                              string glac="";
                                              
                                              if ((world->jantemp(poix,poiy)+world->jultemp(poix,poiy))/2<=world->glacialtemp())
                                                  glac="Glacial region. ";
                                              
                                              if (world->special(poix,poiy)==110)
                                                  climate=climate+". Salt pan";
                                              
                                              if (world->special(poix,poiy)==120)
                                                  climate=climate+". Dunes";
                                              
                                              if (world->special(poix,poiy)==130)
                                                  climate=climate+". Wetlands";
                                              
                                              if (world->special(poix,poiy)==131)
                                                  climate=climate+". Brackish wetlands";
                                              
                                              if (world->special(poix,poiy)==132)
                                                  climate=climate+". Salt wetlands";
                                              
                                              if (world->volcano(poix,poiy)>0)
                                                  climate=climate+". Volcano";
                                              
                                              infotext=infotext+"Climate: "+climate+". "+glac+"\n";
                                          }
                                          else
                                          {
                                              infotext=infotext+"Elevation: "+to_string(sealevel-pointelevation)+" metres below sea level. ";
                                              
                                              if (world->seaice(poix,poiy)==2)
                                                  infotext=infotext+"Permanent sea ice. ";
                                              else
                                              {
                                                  if (world->seaice(poix,poiy)==1)
                                                      infotext=infotext+"Seasonal sea ice. ";
                                                  
                                              }
                                              infotext=infotext+"\n";
                                              
                                              if (world->volcano(poix,poiy)>0)
                                                  infotext=infotext+"Submarine volcano.\n";
                                          }
                                          
                                          //infotext=infotext+"Test: "+to_string(world->test(poix,poiy))+". Mountain height: "+to_string(world->mountainheight(poix,poiy))+".\n";
                                          
                                          infotext=infotext+"January temperature: "+to_string(world->jantemp(poix,poiy))+"°. July temperature: "+to_string(world->jultemp(poix,poiy))+"°. ";
                                          infotext=infotext+"January rainfall: "+to_string(world->janrain(poix,poiy))+" mm/month. July rainfall: "+to_string(world->julrain(poix,poiy))+" mm/month.\n";
                                          
                                          if (world->sea(poix,poiy)==0 && world->riveraveflow(poix,poiy)>0 && world->lakesurface(poix,poiy)==0)
                                          {
                                              string direction;
                                              
                                              switch (world->riverdir(poix,poiy))
                                              {
                                                  case 1:
                                                      direction="north";
                                                      break;
                                                      
                                                  case 2:
                                                      direction="northeast";
                                                      break;
                                                      
                                                  case 3:
                                                      direction="east";
                                                      break;
                                                      
                                                  case 4:
                                                      direction="southeast";
                                                      break;
                                                      
                                                  case 5:
                                                      direction="south";
                                                      break;
                                                      
                                                  case 6:
                                                      direction="southwest";
                                                      break;
                                                      
                                                  case 7:
                                                      direction="west";
                                                      break;
                                                      
                                                  case 8:
                                                      direction="northwest";
                                                      break;
                                              }
                                              infotext=infotext+"River direction: "+direction+". January flow: "+to_string(world->riverjan(poix,poiy))+" m³/s. July flow: "+to_string(world->riverjul(poix,poiy))+" m³/s.\n";
                                          }
                                          
                                          if (world->truelake(poix,poiy)!=0)
                                          {
                                              infotext=infotext+"Lake elevation: "+to_string(world->lakesurface(poix,poiy)-sealevel)+" metres. ";
                                              int depth=world->lakesurface(poix,poiy)-world->nom(poix,poiy);
                                              
                                              string salt="";
                                              
                                              if (world->special(poix,poiy)==100)
                                                  salt="Salty. ";
                                              
                                              infotext=infotext+"Depth: "+to_string(depth)+" metres. "+salt;
                                              
                                          }
                                          
                                          focusinfo->set_caption(infotext);
                                          
                                          screen->redraw();
                                          screen->draw_all();
                                      }
                                  }
                                  else
                                      onmap=0;
                                  
                                  if (onmap==0)
                                  {
                                      focused=0;
                                      
                                      switch (mapview) // Make sure the map image is reset, to remove the focus point from it.
                                      {
                                          case elevation:
                                              globalmap->upload(globalelevationimage);
                                              break;
                                              
                                          case winds:
                                              globalmap->upload(globalwindsimage);
                                              break;
                                              
                                          case temperature:
                                              globalmap->upload(globaltemperatureimage);
                                              break;
                                              
                                          case precipitation:
                                              globalmap->upload(globalprecipitationimage);
                                              break;
                                              
                                          case climate:
                                              globalmap->upload(globalclimateimage);
                                              break;
                                              
                                          case rivers:
                                              globalmap->upload(globalriversimage);
                                              break;
                                              
                                          case relief:
                                              globalmap->upload(globalreliefimage);
                                              break;
                                      }
                                      
                                      globalmapwidget->set_image(globalmap);
                                      
                                      focusinfo->set_caption(" ");
                                      screen->redraw();
                                      screen->draw_all();
                                  }
                                  
                              });
    
    zoombutton->set_callback([&world,&region,&screen,&regionprogress,&regionalmap,&regionalmapimagecreated,&focused,&mapview,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&reliefbutton,&globalmapwindow,&regionalmapwindow,&regionalmapwidget,&regionalminimap,&regionalminimapwidget,&circletemplates,&smalllake,&island,&peaks,&riftblob,&riftblobsize,&poix,&poiy,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&smudge,&smallsmudge,&regionalreliefbutton,&regionalelevationbutton,&regionaltemperaturebutton,&regionalprecipitationbutton,&regionalclimatebutton,&regionalriverbutton]
                             {
                                 if (focused==1)
                                 {
                                     regionalmapwindow->set_position(globalmapwindow->position());
                                     
                                     int newx=poix;
                                     int newy=poiy;

                                     newx=newx/32;
                                     newy=newy/32;
                                     
                                     newx=newx*32;
                                     newy=newy*32;
                                     
                                     newx=newx+16;
                                     newy=newy+16;
                                     
                                     region->setcentrex(newx);
                                     region->setcentrey(newy);
                                     
                                     mapview=relief;
                                     reliefbutton->set_pushed(1);
                                     
                                     regionalreliefbutton->set_pushed(1);
                                     regionalelevationbutton->set_pushed(0);
                                     regionaltemperaturebutton->set_pushed(0);
                                     regionalprecipitationbutton->set_pushed(0);
                                     regionalclimatebutton->set_pushed(0);
                                     regionalriverbutton->set_pushed(0);
                                     
                                     for (int n=0; n<GLOBALMAPTYPES; n++)
                                         regionalmapimagecreated[n]=0;
                                     
                                     float progressstep=1.0/REGIONALCREATIONSTEPS;
                                     regionprogress->set_value(0);
                                     
                                     // Do the minimap
                                     
                                     setregionalminimap(*world,*region,globalreliefimage,*regionalminimap,globalimagewidth,globalimageheight,globalimagechannels);


                                     regionalminimapwidget->set_image(regionalminimap);
                                     regionalminimapwidget->set_scale(0.25);
                                     
                                     // Blank the regional map to start with
                                     
                                     blankregionalreliefimage(*region,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                     
                                     regionalmap->upload(regionalreliefimage);
                                     
                                     regionalmapwidget->set_image(regionalmap);
                                     
                                     regionalmapwidget->set_offset(Vector2f(0,0));
                                     regionalmapwidget->set_scale(1.0);
                                     
                                     // Now shift to the regional map screen and create the new map.
                                     
                                     globalmapwindow->set_visible(0);
                                     regionalmapwindow->set_visible(1);
                                     
                                     generateregionalmap(*world,*region,*screen,*regionprogress,progressstep,circletemplates,smalllake,island,*peaks,riftblob,riftblobsize,0,smudge,smallsmudge);
                                     
                                     drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                     
                                     regionalmap->upload(regionalreliefimage);

                                     
                                     regionalmapwidget->set_image(regionalmap);
                                     regionalmapwidget->set_offset(Vector2f(0,0));
                                     regionalmapwidget->set_scale(1.0);
                                     
                                     regionprogress->set_value(0);
                                 }
                             });
    
    coloursbutton->set_callback([&world,&mapappearancewindow,&mapview,&reliefbutton,&elevationbutton,&temperaturebutton,&precipitationbutton,&climatebutton,&riversbutton,&regionalreliefbutton,&regionalelevationbutton,&regionaltemperaturebutton,&regionalprecipitationbutton,&regionalriverbutton,&globalmap,&globalmapwidget,&globalimagewidth,&globalimagechannels,&globalreliefimage,&focused,&poix,&poiy]
                                {
                                    mapappearancewindow->set_visible(1);
                                    mapappearancewindow->request_focus();
                                    
                                    mapview=relief;
                                    
                                    reliefbutton->set_pushed(1);
                                    elevationbutton->set_pushed(0);
                                    //windbutton->set_pushed(0);
                                    temperaturebutton->set_pushed(0);
                                    precipitationbutton->set_pushed(0);
                                    riversbutton->set_pushed(0);
                                    
                                    regionalreliefbutton->set_pushed(1);
                                    regionalelevationbutton->set_pushed(0);
                                    regionaltemperaturebutton->set_pushed(0);
                                    regionalprecipitationbutton->set_pushed(0);
                                    regionalriverbutton->set_pushed(0);
                                    
                                    int r,g,b;
                                    
                                    if (focused==1) // If there's a focus, copy the colour at that point on the map.
                                    {
                                        int index=(poix+poiy*globalimagewidth)*globalimagechannels;
                                        
                                        r=globalreliefimage[index];
                                        g=globalreliefimage[index+1];
                                        b=globalreliefimage[index+2];
                                        
                                        globalreliefimage[index]=world->highlight1();
                                        globalreliefimage[index+1]=world->highlight2();
                                        globalreliefimage[index+2]=world->highlight3();
                                    }
                                    
                                    globalmap->upload(globalreliefimage);
                                    globalmapwidget->set_image(globalmap);
                                    
                                    if (focused==1) // Now just put the colour back again.
                                    {
                                        int index=(poix+poiy*globalimagewidth)*globalimagechannels;
                                        
                                        globalreliefimage[index]=r;
                                        globalreliefimage[index+1]=g;
                                        globalreliefimage[index+2]=b;
                                    }
                                });
    
    // Set up the warning window callbacks.
    
    warningOKbutton->set_callback([&warningwindow,&globalmapwindow,&importwindow,&world,&mapview,&globalmapimagecreated,&globalelevationimage,&globalwindsimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&importmapwidget]
                                  {
                                      warningwindow->set_visible(0);
                                      globalmapwindow->set_visible(0);
                                      
                                      initialiseworld(*world);
                                      world->clear();
                                      
                                      int width=world->width();
                                      int height=world->height();
                                      int val=world->sealevel()-5000;
                                      
                                      for (int i=0; i<=width; i++)
                                      {
                                          for (int j=0; j<=height; j++)
                                              world->setnom(i,j,val);
                                      }
                                      
                                      mapview=relief;
                                      
                                      for (int n=0; n<GLOBALMAPTYPES; n++)
                                          globalmapimagecreated[n]=0;
                                      drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                      
                                      globalmap->upload(globalreliefimage);
                                      
                                      importmapwidget->set_image(globalmap);
                                      
                                      int seed=random(0,9);
                                      
                                      for (int n=1; n<=7; n++)
                                      {
                                          if (n==7)
                                              seed=seed+(random(1,9)*pow(10,n));
                                          else
                                              seed=seed+(random(0,9)*pow(10,n));
                                      }
                                      
                                      seed=0-seed;
                                      
                                      world->setseed(seed);
                                      
                                      importwindow->set_visible(1);
                                      importwindow->request_focus();
                                  });
    
    warningcancelbutton->set_callback([&warningwindow,&globalmapwindow]
                                      {
                                          warningwindow->set_visible(0);
                                          globalmapwindow->request_focus();
                                      });
    

    // Set up the callbacks for the regional map widgets.
    
    regionalreturnbutton->set_callback([&focused,&rfocused,&focusinfo,&globalmapwindow,&regionalmapwindow,&regionalminimapwidget,&mapview,&globalmap,&globalreliefimage,&globalmapwidget,&regionalfocusinfo,&reliefbutton,&elevationbutton,&temperaturebutton,&precipitationbutton,&climatebutton,&riversbutton]
                                      {
                                          globalmapwindow->set_position(regionalmapwindow->position());
                                          
                                          focused=0;
                                          rfocused=0;
                                          focusinfo->set_caption(" ");
                                          regionalfocusinfo->set_caption(" ");
                                          
                                          reliefbutton->set_pushed(1);
                                          elevationbutton->set_pushed(0);
                                          temperaturebutton->set_pushed(0);
                                          precipitationbutton->set_pushed(0);
                                          climatebutton->set_pushed(0);
                                          riversbutton->set_pushed(0);
                                          
                                          mapview=relief;
                                          
                                          globalmap->upload(globalreliefimage);
                                          
                                          globalmapwidget->set_image(globalmap);
                                          
                                          regionalmapwindow->set_visible(0);
                                          
                                          regionalminimapwidget->set_offset(Vector2f(0,0));
                                          regionalminimapwidget->set_scale(0.25);
                                          
                                          globalmapwindow->set_visible(1);
                                      });
    
    exportregionalmapsbutton->set_callback([&mapview,&world,&region,&rfocused,&poix,&poiy,&regionalmap,&regionalmapwidget,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels]
                                           {
                                               // First, get the save directory.
                                               
                                               string filename=file_dialog({{"png", "Portable Network Graphics"}},true);
                                               
                                               // Now, draw all the maps.
                                               
                                               mapviewenum oldmapview=mapview;
                                               
                                               mapview=relief;
                                               
                                               drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                               
                                               mapview=elevation;
                                               
                                               drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                               
                                               mapview=temperature;
                                               
                                               drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                               
                                               mapview=precipitation;
                                               
                                               drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                               
                                               mapview=climate;
                                               
                                               drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                               
                                               mapview=rivers;
                                               
                                               drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                               
                                               // Now we need to convert these into actual images that we can save.
                                               
                                               filename.resize(filename.size()-4);
                                               
                                               saveimage(regionalreliefimage,regionalimagechannels,regionalimagewidth,regionalimageheight,filename+" Relief.png");
                                               
                                               saveimage(regionalelevationimage,regionalimagechannels,regionalimagewidth,regionalimageheight,filename+" Elevation.png");
                                               
                                               saveimage(regionaltemperatureimage,regionalimagechannels,regionalimagewidth,regionalimageheight,filename+" Temperature.png");
                                               
                                               saveimage(regionalprecipitationimage,regionalimagechannels,regionalimagewidth,regionalimageheight,filename+" Precipitation.png");
                                               
                                               saveimage(regionalclimateimage,regionalimagechannels,regionalimagewidth,regionalimageheight,filename+" Climate.png");
                                               
                                               saveimage(regionalriversimage,regionalimagechannels,regionalimagewidth,regionalimageheight,filename+" Rivers.png");
                                               
                                               mapview=oldmapview;
                                           });
    
    regionalexportareamapsbutton->set_callback([&areaexportwindow,&areaexportmapwidget,&globalmap,&globalmapwidget,&regionalmapwindow,&globalreliefimage,&areafromregional,&regionalminimapwidget]
                                       {
                                           areaexportwindow->set_position(regionalmapwindow->position());
                                           
                                           globalmap->upload(globalreliefimage);
                                           
                                           areaexportmapwidget->set_image(globalmap);

                                           regionalmapwindow->set_visible(0);
                                           areaexportwindow->set_visible(1);
                                           areaexportwindow->request_focus();
                                           
                                           areafromregional=1;
                                       });

    regionalreliefbutton->set_callback([&mapview,&world,&region,&rfocused,&poix,&poiy,&regionalmap,&regionalmapwidget,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels]
                                          {
                                              mapview=relief;
                                              
                                              int r,g,b;
                                              
                                              if (rfocused==1) // If there's a focus, copy the colour at that point on the map.
                                              {
                                                  int index=((poix-region->regwidthbegin())+(poiy-region->regheightbegin())*regionalimagewidth)*regionalimagechannels;
                                                  
                                                  r=regionalreliefimage[index];
                                                  g=regionalreliefimage[index+1];
                                                  b=regionalreliefimage[index+2];
                                                  
                                                  regionalreliefimage[index]=world->highlight1();
                                                  regionalreliefimage[index+1]=world->highlight2();
                                                  regionalreliefimage[index+2]=world->highlight3();
                                              }
                                              drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                              
                                              regionalmap->upload(regionalreliefimage);
                                              regionalmapwidget->set_image(regionalmap);
                                              
                                              if (rfocused==1) // Now just put the colour back again.
                                              {
                                                  int index=((poix-region->regwidthbegin())+(poiy-region->regheightbegin())*regionalimagewidth)*regionalimagechannels;
                                                  
                                                  regionalreliefimage[index]=r;
                                                  regionalreliefimage[index+1]=g;
                                                  regionalreliefimage[index+2]=b;
                                              }
                                          });
    
    regionalelevationbutton->set_callback([&mapview,&world,&region,&rfocused,&poix,&poiy,&regionalmap,&regionalmapwidget,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels]
                                  {
                                      mapview=elevation;
                                      
                                      int r,g,b;
                                      
                                      drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                      
                                      if (rfocused==1) // If there's a focus, copy the colour at that point on the map.
                                      {
                                          int index=((poix-region->regwidthbegin())+(poiy-region->regheightbegin())*regionalimagewidth)*regionalimagechannels;
                                          
                                          r=regionalelevationimage[index];
                                          g=regionalelevationimage[index+1];
                                          b=regionalelevationimage[index+2];
                                          
                                          regionalelevationimage[index]=world->highlight1();
                                          regionalelevationimage[index+1]=world->highlight2();
                                          regionalelevationimage[index+2]=world->highlight3();
                                      }
                                      
                                      regionalmap->upload(regionalelevationimage);
                                      regionalmapwidget->set_image(regionalmap);
                                      
                                      if (rfocused==1) // Now just put the colour back again.
                                      {
                                          int index=((poix-region->regwidthbegin())+(poiy-region->regheightbegin())*regionalimagewidth)*regionalimagechannels;
                                          
                                          regionalelevationimage[index]=r;
                                          regionalelevationimage[index+1]=g;
                                          regionalelevationimage[index+2]=b;
                                      }
                                  });
    
    regionaltemperaturebutton->set_callback([&mapview,&world,&region,&rfocused,&poix,&poiy,&regionalmap,&regionalmapwidget,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels]
                                          {
                                              mapview=temperature;
                                              
                                              int r,g,b;
                                            
                                              drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                              
                                              if (rfocused==1) // If there's a focus, copy the colour at that point on the map.
                                              {
                                                  int index=((poix-region->regwidthbegin())+(poiy-region->regheightbegin())*regionalimagewidth)*regionalimagechannels;
                                                  
                                                  r=regionaltemperatureimage[index];
                                                  g=regionaltemperatureimage[index+1];
                                                  b=regionaltemperatureimage[index+2];
                                                  
                                                  regionaltemperatureimage[index]=world->highlight1();
                                                  regionaltemperatureimage[index+1]=world->highlight2();
                                                  regionaltemperatureimage[index+2]=world->highlight3();
                                              }
                                              
                                              regionalmap->upload(regionaltemperatureimage);
                                              regionalmapwidget->set_image(regionalmap);
                                              
                                              if (rfocused==1) // Now just put the colour back again.
                                              {
                                                  int index=((poix-region->regwidthbegin())+(poiy-region->regheightbegin())*regionalimagewidth)*regionalimagechannels;
                                                  
                                                  regionaltemperatureimage[index]=r;
                                                  regionaltemperatureimage[index+1]=g;
                                                  regionaltemperatureimage[index+2]=b;
                                              }
                                          });
    
    regionalprecipitationbutton->set_callback([&mapview,&world,&region,&rfocused,&poix,&poiy,&regionalmap,&regionalmapwidget,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels]
                                          {
                                              mapview=precipitation;
                                              
                                              int r,g,b;
                                              
                                              drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                              
                                              if (rfocused==1) // If there's a focus, copy the colour at that point on the map.
                                              {
                                                  int index=((poix-region->regwidthbegin())+(poiy-region->regheightbegin())*regionalimagewidth)*regionalimagechannels;
                                                  
                                                  r=regionalprecipitationimage[index];
                                                  g=regionalprecipitationimage[index+1];
                                                  b=regionalprecipitationimage[index+2];
                                                  
                                                  regionalprecipitationimage[index]=world->highlight1();
                                                  regionalprecipitationimage[index+1]=world->highlight2();
                                                  regionalprecipitationimage[index+2]=world->highlight3();
                                              }
                                              
                                              regionalmap->upload(regionalprecipitationimage);
                                              regionalmapwidget->set_image(regionalmap);
                                              
                                              if (rfocused==1) // Now just put the colour back again.
                                              {
                                                  int index=((poix-region->regwidthbegin())+(poiy-region->regheightbegin())*regionalimagewidth)*regionalimagechannels;
                                                  
                                                  regionalprecipitationimage[index]=r;
                                                  regionalprecipitationimage[index+1]=g;
                                                  regionalprecipitationimage[index+2]=b;
                                              }
                                          });
    
    regionalclimatebutton->set_callback([&mapview,&world,&region,&rfocused,&poix,&poiy,&regionalmap,&regionalmapwidget,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels]
                                          {
                                              mapview=climate;
                                              
                                              int r,g,b;
                                              
                                              drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                              
                                              if (rfocused==1) // If there's a focus, copy the colour at that point on the map.
                                              {
                                                  int index=((poix-region->regwidthbegin())+(poiy-region->regheightbegin())*regionalimagewidth)*regionalimagechannels;
                                                  
                                                  r=regionalclimateimage[index];
                                                  g=regionalclimateimage[index+1];
                                                  b=regionalclimateimage[index+2];
                                                  
                                                  regionalclimateimage[index]=world->highlight1();
                                                  regionalclimateimage[index+1]=world->highlight2();
                                                  regionalclimateimage[index+2]=world->highlight3();
                                              }
                                              
                                              regionalmap->upload(regionalclimateimage);
                                              regionalmapwidget->set_image(regionalmap);
                                              
                                              if (rfocused==1) // Now just put the colour back again.
                                              {
                                                  int index=((poix-region->regwidthbegin())+(poiy-region->regheightbegin())*regionalimagewidth)*regionalimagechannels;
                                                  
                                                  regionalclimateimage[index]=r;
                                                  regionalclimateimage[index+1]=g;
                                                  regionalclimateimage[index+2]=b;
                                              }
                                          });
    
    regionalriverbutton->set_callback([&mapview,&world,&region,&rfocused,&poix,&poiy,&regionalmap,&regionalmapwidget,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels]
                                          {
                                              mapview=rivers;
                                              
                                              int r,g,b;
                                              
                                              drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                              
                                              if (rfocused==1) // If there's a focus, copy the colour at that point on the map.
                                              {
                                                  int index=((poix-region->regwidthbegin())+(poiy-region->regheightbegin())*regionalimagewidth)*regionalimagechannels;
                                                  
                                                  r=regionalriversimage[index];
                                                  g=regionalriversimage[index+1];
                                                  b=regionalriversimage[index+2];
                                                  
                                                  regionalriversimage[index]=world->highlight1();
                                                  regionalriversimage[index+1]=world->highlight2();
                                                  regionalriversimage[index+2]=world->highlight3();
                                              }
                                              
                                              regionalmap->upload(regionalriversimage);
                                              regionalmapwidget->set_image(regionalmap);
                                              
                                              if (rfocused==1) // Now just put the colour back again.
                                              {
                                                  int index=((poix-region->regwidthbegin())+(poiy-region->regheightbegin())*regionalimagewidth)*regionalimagechannels;
                                                  
                                                  regionalriversimage[index]=r;
                                                  regionalriversimage[index+1]=g;
                                                  regionalriversimage[index+2]=b;
                                              }
                                          });
    
    regionalrecentrebutton->set_callback([&regionalmapwidget]{regionalmapwidget->reset();});
    
    regionalfocusbutton->set_callback([&world,&region,&rfocused,&poix,&poiy,&regionalmapimagewidth,&regionalmapimageheight,&screen,&regionalmapwindow,&regionalmapandprogressbox,&regionalmapsbox,&regionalmainbox,&regionalmapwidget,&regionalfocusinfo,&regionalimagewidth,&regionalimagechannels,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&mapview,&regionalmap]
                              {
                                  Vector2i mousepos=screen->mouse_pos();
                                  sf::Vector2i sfmousestartpos=sf::Mouse::getPosition(); // The nanogui mouse detection will only pick up where the mouse is when the button is initially clicked, so we need to use the sf mouse detection too to supplement it.
                                  
                                  this_thread::sleep_for(chrono::milliseconds{100});
                                  
                                  while (sf::Mouse::isButtonPressed(sf::Mouse::Left)==0){}
                                  
                                  sf::Vector2i sfmouseendpos=sf::Mouse::getPosition();
                                  
                                  int xdiff=sfmouseendpos.x-sfmousestartpos.x;
                                  int ydiff=sfmouseendpos.y-sfmousestartpos.y;
                                  
                                  mousepos.x()=mousepos.x()+xdiff;
                                  mousepos.y()=mousepos.y()+ydiff;
                                  
                                  bool onmap=1;
                                  
                                  if (regionalmapwindow->find_widget(mousepos)==regionalmapwidget)
                                  {
                                      rfocused=1;
                                      
                                      Vector2i mappos1=regionalmapwindow->position();
                                      Vector2i mappos2=regionalmainbox->position();
                                      Vector2i mappos3=regionalmapsbox->position();
                                      Vector2i mappos4=regionalmapandprogressbox->position();
                                      Vector2i mappos5=regionalmapwidget->position();
                                      
                                      Vector2f pointpos; // Coordinates of the mouse within the map widget.
                                      
                                      pointpos.x()=mousepos.x()-mappos1.x()-mappos2.x()-mappos3.x()-mappos4.x()-mappos5.x();
                                      pointpos.y()=mousepos.y()-mappos1.y()-mappos2.y()-mappos3.y()-mappos4.y()-mappos5.y();
                                      
                                      Vector2f poi=regionalmapwidget->pos_to_pixel(pointpos); // Global coordinates of the point of interest.

                                      if (poi.x()<0 || poi.x()>regionalmapimagewidth || poi.y()<0 || poi.y()>regionalmapimageheight)
                                          onmap=0;
                                      else
                                      {
                                          int sealevel=world->sealevel();
                                          int height=world->height();
                                          
                                          poix=poi.x()+region->regwidthbegin();
                                          poiy=poi.y()+region->regheightbegin();
                                          
                                          int index=((poix-region->regwidthbegin())+(poiy-region->regheightbegin())*regionalimagewidth)*regionalimagechannels;
                                          
                                          int r,g,b;
                                          
                                          switch (mapview) // First, paint the focus point onto the currently viewed map.
                                          {
                                              case elevation:
                                                  r=regionalelevationimage[index];
                                                  g=regionalelevationimage[index+1];
                                                  b=regionalelevationimage[index+2];
                                                  
                                                  regionalelevationimage[index]=world->highlight1();
                                                  regionalelevationimage[index+1]=world->highlight2();
                                                  regionalelevationimage[index+2]=world->highlight3();
                                                  
                                                  regionalmap->upload(regionalelevationimage);
                                                  
                                                  regionalelevationimage[index]=r;
                                                  regionalelevationimage[index+1]=g;
                                                  regionalelevationimage[index+2]=b;
                                                  
                                                  break;
                                                  
                                              case winds:
                                                  break;
                                                  
                                              case temperature:
                                                  r=regionaltemperatureimage[index];
                                                  g=regionaltemperatureimage[index+1];
                                                  b=regionaltemperatureimage[index+2];
                                                  
                                                  regionaltemperatureimage[index]=world->highlight1();
                                                  regionaltemperatureimage[index+1]=world->highlight2();
                                                  regionaltemperatureimage[index+2]=world->highlight3();
                                                  
                                                  regionalmap->upload(regionaltemperatureimage);
                                                  
                                                  regionaltemperatureimage[index]=r;
                                                  regionaltemperatureimage[index+1]=g;
                                                  regionaltemperatureimage[index+2]=b;
                                                  
                                                  break;
                                                  
                                              case precipitation:
                                                  r=regionalprecipitationimage[index];
                                                  g=regionalprecipitationimage[index+1];
                                                  b=regionalprecipitationimage[index+2];
                                                  
                                                  regionalprecipitationimage[index]=world->highlight1();
                                                  regionalprecipitationimage[index+1]=world->highlight2();
                                                  regionalprecipitationimage[index+2]=world->highlight3();
                                                  
                                                  regionalmap->upload(regionalprecipitationimage);
                                                  
                                                  regionalprecipitationimage[index]=r;
                                                  regionalprecipitationimage[index+1]=g;
                                                  regionalprecipitationimage[index+2]=b;
                                                  
                                                  break;
                                                  
                                              case climate:
                                                  r=regionalclimateimage[index];
                                                  g=regionalclimateimage[index+1];
                                                  b=regionalclimateimage[index+2];
                                                  
                                                  regionalclimateimage[index]=world->highlight1();
                                                  regionalclimateimage[index+1]=world->highlight2();
                                                  regionalclimateimage[index+2]=world->highlight3();
                                                  
                                                  regionalmap->upload(regionalclimateimage);
                                                  
                                                  regionalclimateimage[index]=r;
                                                  regionalclimateimage[index+1]=g;
                                                  regionalclimateimage[index+2]=b;
                                                  
                                                  break;
                                                  
                                              case rivers:
                                                  r=regionalriversimage[index];
                                                  g=regionalriversimage[index+1];
                                                  b=regionalriversimage[index+2];
                                                  
                                                  regionalriversimage[index]=world->highlight1();
                                                  regionalriversimage[index+1]=world->highlight2();
                                                  regionalriversimage[index+2]=world->highlight3();
                                                  
                                                  regionalmap->upload(regionalriversimage);
                                                  
                                                  regionalriversimage[index]=r;
                                                  regionalriversimage[index+1]=g;
                                                  regionalriversimage[index+2]=b;
                                                  
                                                  break;
                                                  
                                              case relief:
                                                  r=regionalreliefimage[index];
                                                  g=regionalreliefimage[index+1];
                                                  b=regionalreliefimage[index+2];
                                                  
                                                  regionalreliefimage[index]=world->highlight1();
                                                  regionalreliefimage[index+1]=world->highlight2();
                                                  regionalreliefimage[index+2]=world->highlight3();
                                                  
                                                  regionalmap->upload(regionalreliefimage);
                                                  
                                                  regionalreliefimage[index]=r;
                                                  regionalreliefimage[index+1]=g;
                                                  regionalreliefimage[index+2]=b;
                                                  
                                                  break;
                                          }
                                          
                                          regionalmapwidget->set_image(regionalmap);

                                          string infotext2=""; // Now do the information about this point.
                                          
                                          int xx=region->leftx()+poix/16;
                                          int yy=region->lefty()+poiy/16; // Coordinates of the gobal cell we're in.
                                          
                                          if (region->surface(poix,poiy)!=0 && region->special(poix,poiy)>100) // If there's a "special" lake here
                                          {
                                              infotext2="Elevation: "+to_string(region->surface(poix,poiy)-sealevel)+" metres above sea level.\n";
                                          }
                                          else
                                          {
                                              if (region->map(poix,poiy)<=sealevel)
                                              {
                                                  infotext2="Elevation: "+to_string(sealevel-region->map(poix,poiy))+" metres below sea level.\n";
                                              }
                                              else
                                                  infotext2="Elevation: "+to_string(region->map(poix,poiy)-sealevel)+" metres above sea level.\n";
                                          }
                                          
                                          int janrain, julrain, jantemp, jultemp;
                                          
                                          if (yy<=height/2) // Northern hemisphere
                                          {
                                              janrain=region->winterrain(poix,poiy);
                                              julrain=region->summerrain(poix,poiy);
                                              
                                              jantemp=region->mintemp(poix,poiy);
                                              jultemp=region->maxtemp(poix,poiy);
                                              
                                          }
                                          else // Southern hemisphere
                                          {
                                              janrain=region->summerrain(poix,poiy);
                                              julrain=region->winterrain(poix,poiy);
                                              
                                              jantemp=region->maxtemp(poix,poiy);
                                              jultemp=region->mintemp(poix,poiy);
                                          }
                                          
                                          infotext2=infotext2+"January temperature: "+to_string(jantemp)+"°. July temperature: "+to_string(jultemp)+"°. ";
                                          
                                          infotext2=infotext2+"January rainfall: "+to_string(janrain)+" mm/month. July rainfall: "+to_string(julrain)+" mm/month.\n";
                                          
                                          if (region->truelake(poix,poiy)==1) // If there's a lake here
                                          {
                                              infotext2=infotext2+"Lake elevation: "+to_string(region->lakesurface(poix,poiy)-sealevel)+" metres. Depth: "+to_string(region->lakesurface(poix,poiy)-region->map(poix,poiy))+" metres. ";
                                              
                                              if (region->special(poix,poiy)==100)
                                                  infotext2=infotext2+"Salty. ";
                                              
                                              infotext2=infotext2+"\n";
                                          }
                                          else
                                          {
                                              if (region->riverdir(poix,poiy)!=0 && region->special(poix,poiy)!=140) // If there's a river here
                                              {
                                                  infotext2=infotext2+"River direction: "+getdirstring(region->riverdir(poix,poiy))+". ";
                                                  infotext2=infotext2+"January flow: "+to_string(region->riverjan(poix,poiy))+" m³/s. July flow: "+to_string(region->riverjul(poix,poiy))+" m³/s. \n";
                                              }
                                              else
                                              {
                                                  if (region->fakedir(poix,poiy)>0 && region->special(poix,poiy)!=140) // If there's a fake river here
                                                  {
                                                      infotext2=infotext2+"River direction: "+getdirstring(region->fakedir(poix,poiy))+". ";
                                                      infotext2=infotext2+"January flow: "+to_string(region->fakejan(poix,poiy))+" m³/s. July flow: "+to_string(region->fakejul(poix,poiy))+" m³/s. \n";
                                                  }
                                              }
                                          }
                                          
                                          if (region->sea(poix,poiy)==1)
                                          {
                                              if (region->volcano(poix,poiy))
                                                  infotext2=infotext2+"Submarine volcano. ";
                                              
                                              int seaice=region->seaice(poix,poiy);
                                              
                                              if (seaice==1)
                                                  infotext2=infotext2+"Seasonal sea ice.";
                                              
                                              if (seaice==2)
                                                  infotext2=infotext2+"Permanent sea ice.";
                                              
                                              infotext2=infotext2+"\n";
                                              
                                          }
                                          else
                                          {
                                              string climate=region->climate(poix,poiy);
                                              
                                              climate=getclimatename(climate)+" ("+climate+")";
                                              
                                              if (region->special(poix,poiy)==110)
                                                  climate=climate+". Salt pan";
                                              
                                              if (region->special(poix,poiy)==120)
                                                  climate=climate+". Dunes";
                                              
                                              if (region->special(poix,poiy)==130)
                                                  climate=climate+". Wetlands";
                                              
                                              if (region->special(poix,poiy)==131)
                                                  climate=climate+". Brackish wetlands";
                                              
                                              if (region->special(poix,poiy)==132)
                                                  climate=climate+". Salt wetlands";
                                              
                                              if (region->rivervalley(poix,poiy)==1)
                                                  climate=climate+". River valley";
                                              
                                              if (region->special(poix,poiy)==140)
                                                  climate=climate+". Glacier";
                                              
                                              if (region->volcano(poix,poiy))
                                                  climate=climate+". Volcano";
                                              
                                              infotext2=infotext2+"Climate: "+climate+".\n";
                                          }
                                          
                                          regionalfocusinfo->set_caption(infotext2);
                                          
                                          screen->redraw();
                                          screen->draw_all();
                                          
                                      }
                                  }
                                  else
                                      onmap=0;
                                  
                                  if (onmap==0)
                                  {
                                      rfocused=0;
                                      
                                      switch (mapview) // Make sure the map image is reset, to remove the focus point from it.
                                      {
                                          case elevation:
                                              regionalmap->upload(regionalelevationimage);
                                              break;
                                              
                                          case winds:
                                              break;
                                              
                                          case temperature:
                                              regionalmap->upload(regionaltemperatureimage);
                                              break;
                                              
                                          case precipitation:
                                              regionalmap->upload(regionalprecipitationimage);
                                              break;
                                              
                                          case climate:
                                              regionalmap->upload(regionalclimateimage);
                                              break;
                                              
                                          case rivers:
                                              regionalmap->upload(regionalriversimage);
                                              break;
                                              
                                          case relief:
                                              regionalmap->upload(regionalreliefimage);
                                              break;
                                      }
                                      
                                      regionalfocusinfo->set_caption(" ");
                                      screen->redraw();
                                      screen->draw_all();
                                  }
                                  
                              });
    
    regionalcoloursbutton->set_callback([&mapappearancewindow,&mapview,&reliefbutton,&elevationbutton,&temperaturebutton,&precipitationbutton,&climatebutton,&riversbutton,&regionalreliefbutton,&regionalelevationbutton,&regionaltemperaturebutton,&regionalprecipitationbutton,&regionalriverbutton,&world,&region,&rfocused,&poix,&poiy,&regionalmap,&regionalmapwidget,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels]
                                {
                                    mapappearancewindow->set_visible(1);
                                    mapappearancewindow->request_focus();
                                    
                                    mapview=relief;
                                    
                                    reliefbutton->set_pushed(1);
                                    elevationbutton->set_pushed(0);
                                    //windbutton->set_pushed(0);
                                    temperaturebutton->set_pushed(0);
                                    precipitationbutton->set_pushed(0);
                                    riversbutton->set_pushed(0);
                                    
                                    regionalreliefbutton->set_pushed(1);
                                    regionalelevationbutton->set_pushed(0);
                                    regionaltemperaturebutton->set_pushed(0);
                                    regionalprecipitationbutton->set_pushed(0);
                                    regionalriverbutton->set_pushed(0);
                                    
                                    int r,g,b;
                                    
                                    if (rfocused==1) // If there's a focus, copy the colour at that point on the map.
                                    {
                                        int index=((poix-region->regwidthbegin())+(poiy-region->regheightbegin())*regionalimagewidth)*regionalimagechannels;
                                        
                                        r=regionalreliefimage[index];
                                        g=regionalreliefimage[index+1];
                                        b=regionalreliefimage[index+2];
                                        
                                        regionalreliefimage[index]=world->highlight1();
                                        regionalreliefimage[index+1]=world->highlight2();
                                        regionalreliefimage[index+2]=world->highlight3();
                                    }
                                    drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                    
                                    regionalmap->upload(regionalreliefimage);
                                    regionalmapwidget->set_image(regionalmap);
                                    
                                    if (rfocused==1) // Now just put the colour back again.
                                    {
                                        int index=((poix-region->regwidthbegin())+(poiy-region->regheightbegin())*regionalimagewidth)*regionalimagechannels;
                                        
                                        regionalreliefimage[index]=r;
                                        regionalreliefimage[index+1]=g;
                                        regionalreliefimage[index+2]=b;
                                    }
                                });
    
    regionalminimaprecentrebutton->set_callback([&regionalminimapwidget]
                                            {
                                                regionalminimapwidget->set_offset(Vector2f(0,0));
                                                regionalminimapwidget->set_scale(0.25);
                                            });
    
    regionalminimapfocusbutton->set_callback([&world,&region,&rfocused,&regionalmapwindow,&regionalmainbox,&regionalmapsbox,&regionalextrabitsbox,&regionalminimapwidget,&screen,&width,&height,&regionmargin,&regionalmapimagecreated,&regionprogress,&globalreliefimage,&regionalminimap,&regionalmap,&regionalmapwidget,&circletemplates,&smalllake,&island,&peaks,&riftblob,&riftblobsize,&mapview,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalfocusinfo,&globalimagewidth,&globalimageheight,&globalimagechannels,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&poix,&poiy,&smudge,&smallsmudge]
                                      {
                                          Vector2i mousepos=screen->mouse_pos();
                                          sf::Vector2i sfmousestartpos=sf::Mouse::getPosition(); // The nanogui mouse detection will only pick up where the mouse is when the button is initially clicked, so we need to use the sf mouse detection too to supplement it.
                                          
                                          this_thread::sleep_for(chrono::milliseconds{100});
                                          
                                          while (sf::Mouse::isButtonPressed(sf::Mouse::Left)==0){}
                                          
                                          sf::Vector2i sfmouseendpos=sf::Mouse::getPosition();
                                          
                                          int xdiff=sfmouseendpos.x-sfmousestartpos.x;
                                          int ydiff=sfmouseendpos.y-sfmousestartpos.y;
                                          
                                          mousepos.x()=mousepos.x()+xdiff;
                                          mousepos.y()=mousepos.y()+ydiff;
                                          
                                          bool onmap=1;
                                          
                                          if (regionalmapwindow->find_widget(mousepos)==regionalminimapwidget)
                                          {
                                              Vector2i mappos1=regionalmapwindow->position();
                                              Vector2i mappos2=regionalmainbox->position();
                                              Vector2i mappos3=regionalmapsbox->position();
                                              Vector2i mappos4=regionalextrabitsbox->position();
                                              Vector2i mappos5=regionalminimapwidget->position();
                                              
                                              Vector2f pointpos; // Coordinates of the mouse within the map widget.
                                              
                                              pointpos.x()=mousepos.x()-mappos1.x()-mappos2.x()-mappos3.x()-mappos4.x()-mappos5.x();
                                              pointpos.y()=mousepos.y()-mappos1.y()-mappos2.y()-mappos3.y()-mappos4.y()-mappos5.y();
                                              
                                              Vector2f newloc=regionalminimapwidget->pos_to_pixel(pointpos); // Global coordinates of the new point.
                                              
                                              if (newloc.x()>=0 && newloc.x()<=width && newloc.y()>=0 && newloc.y()<=height)
                                              {
                                                  int newx=newloc.x();
                                                  int newy=newloc.y();
                                                  
                                                  if (newy<regionmargin)
                                                      newy=regionmargin;
                                                  
                                                  if (newy>height-regionmargin)
                                                      newy=height-regionmargin;
                                                  
                                                  if (rfocused==1)
                                                  {
                                                      int movex=newx-region->centrex();
                                                      int movey=newy-region->centrey();

                                                      poix=poix-movex*16;
                                                      poiy=poiy-movey*16;
                                                      
                                                      if (poix>region->regwidthend())
                                                          rfocused=0;
                                                      
                                                      if (poix<region->regwidthbegin())
                                                          rfocused=0;
                                                      
                                                      if (poiy>region->regheightend())
                                                          rfocused=0;
                                                      
                                                      if (poiy<region->regheightbegin())
                                                          rfocused=0;
                                                  }
                                                  
                                                  newx=newx/32;
                                                  newy=newy/32;
                                                  
                                                  newx=newx*32;
                                                  newy=newy*32;
                                                  
                                                  newx=newx+16;
                                                  newy=newy+16;

                                                  region->setcentrex(newx);
                                                  region->setcentrey(newy);
                                                  
                                                  for (int n=0; n<GLOBALMAPTYPES; n++)
                                                      regionalmapimagecreated[n]=0;
                                                  
                                                  float progressstep=1.0/REGIONALCREATIONSTEPS;
                                                  regionprogress->set_value(0);
                                                  
                                                  // Do the minimap
                                                  
                                                  setregionalminimap(*world,*region,globalreliefimage,*regionalminimap,globalimagewidth,globalimageheight,globalimagechannels);
                                                  
                                                  regionalminimapwidget->set_image(regionalminimap);
                                                  
                                                  // Now create the new map.
                                                  
                                                  for (int n=0; n<GLOBALMAPTYPES; n++)
                                                      regionalmapimagecreated[n]=0;
                                                  
                                                  generateregionalmap(*world,*region,*screen,*regionprogress,progressstep,circletemplates,smalllake,island,*peaks,riftblob,riftblobsize,0,smudge,smallsmudge);
                                                  
                                                  drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                                  
                                                  int r,g,b;
                                                  
                                                  int index=((poix-region->regwidthbegin())+(poiy-region->regheightbegin())*regionalimagewidth)*regionalimagechannels;
                                                  
                                                  switch (mapview)
                                                  {
                                                      case elevation:
                                                          if (rfocused==1)
                                                          {
                                                              r=regionalelevationimage[index];
                                                              g=regionalelevationimage[index+1];
                                                              b=regionalelevationimage[index+2];
                                                              
                                                              regionalelevationimage[index]=world->highlight1();
                                                              regionalelevationimage[index+1]=world->highlight2();
                                                              regionalelevationimage[index+2]=world->highlight3();
                                                          }
                                                          
                                                          regionalmap->upload(regionalelevationimage);
                                                          
                                                          if (rfocused==1)
                                                          {
                                                              regionalelevationimage[index]=r;
                                                              regionalelevationimage[index+1]=g;
                                                              regionalelevationimage[index+2]=b;
                                                          }
                                                          break;
                                                          
                                                      case winds:
                                                          break;
                                                          
                                                      case temperature:
                                                          if (rfocused==1)
                                                          {
                                                              r=regionaltemperatureimage[index];
                                                              g=regionaltemperatureimage[index+1];
                                                              b=regionaltemperatureimage[index+2];
                                                              
                                                              regionaltemperatureimage[index]=world->highlight1();
                                                              regionaltemperatureimage[index+1]=world->highlight2();
                                                              regionaltemperatureimage[index+2]=world->highlight3();
                                                          }
                                                          
                                                          regionalmap->upload(regionalelevationimage);
                                                          
                                                          if (rfocused==1)
                                                          {
                                                              regionaltemperatureimage[index]=r;
                                                              regionaltemperatureimage[index+1]=g;
                                                              regionaltemperatureimage[index+2]=b;
                                                          }
                                                          break;
                                                          
                                                      case precipitation:
                                                          if (rfocused==1)
                                                          {
                                                              r=regionalprecipitationimage[index];
                                                              g=regionalprecipitationimage[index+1];
                                                              b=regionalprecipitationimage[index+2];
                                                              
                                                              regionalprecipitationimage[index]=world->highlight1();
                                                              regionalprecipitationimage[index+1]=world->highlight2();
                                                              regionalprecipitationimage[index+2]=world->highlight3();
                                                          }
                                                          
                                                          regionalmap->upload(regionalprecipitationimage);
                                                          
                                                          if (rfocused==1)
                                                          {
                                                              regionalprecipitationimage[index]=r;
                                                              regionalprecipitationimage[index+1]=g;
                                                              regionalprecipitationimage[index+2]=b;
                                                          }
                                                          break;
                                                          
                                                      case climate:
                                                          if (rfocused==1)
                                                          {
                                                              r=regionalclimateimage[index];
                                                              g=regionalclimateimage[index+1];
                                                              b=regionalclimateimage[index+2];
                                                              
                                                              regionalclimateimage[index]=world->highlight1();
                                                              regionalclimateimage[index+1]=world->highlight2();
                                                              regionalclimateimage[index+2]=world->highlight3();
                                                          }
                                                          
                                                          regionalmap->upload(regionalelevationimage);
                                                          
                                                          if (rfocused==1)
                                                          {
                                                              regionalclimateimage[index]=r;
                                                              regionalclimateimage[index+1]=g;
                                                              regionalclimateimage[index+2]=b;
                                                          }
                                                          break;
                                                          
                                                      case rivers:
                                                          if (rfocused==1)
                                                          {
                                                              r=regionalriversimage[index];
                                                              g=regionalriversimage[index+1];
                                                              b=regionalriversimage[index+2];
                                                              
                                                              regionalriversimage[index]=world->highlight1();
                                                              regionalriversimage[index+1]=world->highlight2();
                                                              regionalriversimage[index+2]=world->highlight3();
                                                          }
                                                          
                                                          regionalmap->upload(regionalriversimage);
                                                          
                                                          if (rfocused==1)
                                                          {
                                                              regionalriversimage[index]=r;
                                                              regionalriversimage[index+1]=g;
                                                              regionalriversimage[index+2]=b;
                                                          }
                                                          break;
                                                          
                                                      case relief:
                                                          if (rfocused==1)
                                                          {
                                                              r=regionalreliefimage[index];
                                                              g=regionalreliefimage[index+1];
                                                              b=regionalreliefimage[index+2];
                                                              
                                                              regionalreliefimage[index]=world->highlight1();
                                                              regionalreliefimage[index+1]=world->highlight2();
                                                              regionalreliefimage[index+2]=world->highlight3();
                                                          }
                                                          
                                                          regionalmap->upload(regionalreliefimage);
                                                          
                                                          if (rfocused==1)
                                                          {
                                                              regionalreliefimage[index]=r;
                                                              regionalreliefimage[index+1]=g;
                                                              regionalreliefimage[index+2]=b;
                                                          }
                                                          break;
                                                          
                                                  }
                                                  
                                                  regionalmapwidget->set_image(regionalmap);
                                                  regionalmapwidget->set_offset(Vector2f(0,0));
                                                  regionalmapwidget->set_scale(1.0);
                                                  
                                                  regionprogress->set_value(0);
                                                  
                                                  if (rfocused==0)
                                                      regionalfocusinfo->set_caption(" ");
                                              }
                                          }
                                      });

    regionalWbutton->set_callback([&world,&region,&rfocused,&regionalmapwindow,&regionalmainbox,&regionalmapsbox,&regionalextrabitsbox,&regionalminimapwidget,&screen,&width,&height,&regionmargin,&regionalmapimagecreated,&regionprogress,&globalreliefimage,&regionalminimap,&regionalmap,&regionalmapwidget,&circletemplates,&smalllake,&island,&peaks,&riftblob,&riftblobsize,&mapview,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalfocusinfo,&globalimagewidth,&globalimageheight,&globalimagechannels,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&poix,&poiy,&smudge,&smallsmudge]
                                  {
                                      int regionalmapmove=REGIONALTILEWIDTH; //stoi(regionalmapmoveinput->value());

                                      int newx=region->centrex()-regionalmapmove;
                                      
                                      newx=newx/32;
                                      newx=newx*32;
                                      newx=newx+16;
                                      
                                      if (newx<0)
                                          newx=wrap(newx,world->width());

                                      region->setcentrex(newx);
                                      
                                      for (int n=0; n<GLOBALMAPTYPES; n++)
                                          regionalmapimagecreated[n]=0;
                                      
                                      if (rfocused==1)
                                      {
                                          poix=poix+(regionalmapmove*16);
                                          
                                          if (poix>region->regwidthend())
                                              rfocused=0;
                                      }
                                      
                                      float progressstep=1.0/REGIONALCREATIONSTEPS;
                                      regionprogress->set_value(0);
                                      
                                      // Do the minimap
                                      
                                      setregionalminimap(*world,*region,globalreliefimage,*regionalminimap,globalimagewidth,globalimageheight,globalimagechannels);
                                      
                                      regionalminimapwidget->set_image(regionalminimap);
                                      
                                      // Now create the new map.
                                      
                                      for (int n=0; n<GLOBALMAPTYPES; n++)
                                          regionalmapimagecreated[n]=0;
                                      
                                      generateregionalmap(*world,*region,*screen,*regionprogress,progressstep,circletemplates,smalllake,island,*peaks,riftblob,riftblobsize,0,smudge,smallsmudge);
                                      
                                      drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                      
                                      int r,g,b;
                                      
                                      int index=((poix-region->regwidthbegin())+(poiy-region->regheightbegin())*regionalimagewidth)*regionalimagechannels;
                                      
                                      switch (mapview)
                                      {
                                          case elevation:
                                              if (rfocused==1)
                                              {
                                                  r=regionalelevationimage[index];
                                                  g=regionalelevationimage[index+1];
                                                  b=regionalelevationimage[index+2];
                                                  
                                                  regionalelevationimage[index]=world->highlight1();
                                                  regionalelevationimage[index+1]=world->highlight2();
                                                  regionalelevationimage[index+2]=world->highlight3();
                                              }
                                              
                                              regionalmap->upload(regionalelevationimage);
                                              
                                              if (rfocused==1)
                                              {
                                                  regionalelevationimage[index]=r;
                                                  regionalelevationimage[index+1]=g;
                                                  regionalelevationimage[index+2]=b;
                                              }
                                              break;
                                              
                                          case winds:
                                              break;
                                              
                                          case temperature:
                                              if (rfocused==1)
                                              {
                                                  r=regionaltemperatureimage[index];
                                                  g=regionaltemperatureimage[index+1];
                                                  b=regionaltemperatureimage[index+2];
                                                  
                                                  regionaltemperatureimage[index]=world->highlight1();
                                                  regionaltemperatureimage[index+1]=world->highlight2();
                                                  regionaltemperatureimage[index+2]=world->highlight3();
                                              }
                                              
                                              regionalmap->upload(regionalelevationimage);
                                              
                                              if (rfocused==1)
                                              {
                                                  regionaltemperatureimage[index]=r;
                                                  regionaltemperatureimage[index+1]=g;
                                                  regionaltemperatureimage[index+2]=b;
                                              }
                                              break;
                                              
                                          case precipitation:
                                              if (rfocused==1)
                                              {
                                                  r=regionalprecipitationimage[index];
                                                  g=regionalprecipitationimage[index+1];
                                                  b=regionalprecipitationimage[index+2];
                                                  
                                                  regionalprecipitationimage[index]=world->highlight1();
                                                  regionalprecipitationimage[index+1]=world->highlight2();
                                                  regionalprecipitationimage[index+2]=world->highlight3();
                                              }
                                              
                                              regionalmap->upload(regionalprecipitationimage);
                                              
                                              if (rfocused==1)
                                              {
                                                  regionalprecipitationimage[index]=r;
                                                  regionalprecipitationimage[index+1]=g;
                                                  regionalprecipitationimage[index+2]=b;
                                              }
                                              break;
                                              
                                          case climate:
                                              if (rfocused==1)
                                              {
                                                  r=regionalclimateimage[index];
                                                  g=regionalclimateimage[index+1];
                                                  b=regionalclimateimage[index+2];
                                                  
                                                  regionalclimateimage[index]=world->highlight1();
                                                  regionalclimateimage[index+1]=world->highlight2();
                                                  regionalclimateimage[index+2]=world->highlight3();
                                              }
                                              
                                              regionalmap->upload(regionalelevationimage);
                                              
                                              if (rfocused==1)
                                              {
                                                  regionalclimateimage[index]=r;
                                                  regionalclimateimage[index+1]=g;
                                                  regionalclimateimage[index+2]=b;
                                              }
                                              break;
                                              
                                          case rivers:
                                              if (rfocused==1)
                                              {
                                                  r=regionalriversimage[index];
                                                  g=regionalriversimage[index+1];
                                                  b=regionalriversimage[index+2];
                                                  
                                                  regionalriversimage[index]=world->highlight1();
                                                  regionalriversimage[index+1]=world->highlight2();
                                                  regionalriversimage[index+2]=world->highlight3();
                                              }
                                              
                                              regionalmap->upload(regionalriversimage);
                                              
                                              if (rfocused==1)
                                              {
                                                  regionalriversimage[index]=r;
                                                  regionalriversimage[index+1]=g;
                                                  regionalriversimage[index+2]=b;
                                              }
                                              break;
                                              
                                          case relief:
                                              if (rfocused==1)
                                              {
                                                  r=regionalreliefimage[index];
                                                  g=regionalreliefimage[index+1];
                                                  b=regionalreliefimage[index+2];
                                                  
                                                  regionalreliefimage[index]=world->highlight1();
                                                  regionalreliefimage[index+1]=world->highlight2();
                                                  regionalreliefimage[index+2]=world->highlight3();
                                              }
                                              
                                              regionalmap->upload(regionalreliefimage);
                                              
                                              if (rfocused==1)
                                              {
                                                  regionalreliefimage[index]=r;
                                                  regionalreliefimage[index+1]=g;
                                                  regionalreliefimage[index+2]=b;
                                              }
                                              break;
                                              
                                      }
                                      
                                      regionalmapwidget->set_image(regionalmap);
                                      regionalmapwidget->set_offset(Vector2f(0,0));
                                      regionalmapwidget->set_scale(1.0);
                                      
                                      regionprogress->set_value(0);
                                      
                                      if (rfocused==0)
                                          regionalfocusinfo->set_caption(" ");
                                  });
    
    regionalEbutton->set_callback([&world,&region,&rfocused,&regionalmapwindow,&regionalmainbox,&regionalmapsbox,&regionalextrabitsbox,&regionalminimapwidget,&screen,&width,&height,&regionmargin,&regionalmapimagecreated,&regionprogress,&globalreliefimage,&regionalminimap,&regionalmap,&regionalmapwidget,&circletemplates,&smalllake,&island,&peaks,&riftblob,&riftblobsize,&mapview,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalfocusinfo,&globalimagewidth,&globalimageheight,&globalimagechannels,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&poix,&poiy,&smudge,&smallsmudge]
                                  {
                                      int regionalmapmove=REGIONALTILEWIDTH; //stoi(regionalmapmoveinput->value());
                                      
                                      int newx=region->centrex()+regionalmapmove;
                                      
                                      newx=newx/32;
                                      newx=newx*32;
                                      newx=newx+16;
                                      
                                      if (newx>world->width())
                                          newx=wrap(newx,world->width());
                                      
                                      region->setcentrex(newx);
                                      
                                      for (int n=0; n<GLOBALMAPTYPES; n++)
                                          regionalmapimagecreated[n]=0;
                                      
                                      if (rfocused==1)
                                      {
                                          poix=poix-(regionalmapmove*16);
                                          
                                          if (poix<region->regwidthbegin())
                                              rfocused=0;
                                      }
                                      
                                      float progressstep=1.0/REGIONALCREATIONSTEPS;
                                      regionprogress->set_value(0);
                                      
                                      // Do the minimap
                                      
                                      setregionalminimap(*world,*region,globalreliefimage,*regionalminimap,globalimagewidth,globalimageheight,globalimagechannels);
                                      
                                      regionalminimapwidget->set_image(regionalminimap);
                                      
                                      // Now create the new map.
                                      
                                      for (int n=0; n<GLOBALMAPTYPES; n++)
                                          regionalmapimagecreated[n]=0;
                                      
                                      generateregionalmap(*world,*region,*screen,*regionprogress,progressstep,circletemplates,smalllake,island,*peaks,riftblob,riftblobsize,0,smudge,smallsmudge);
                                      
                                      drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                      
                                      int r,g,b;
                                      
                                      int index=((poix-region->regwidthbegin())+(poiy-region->regheightbegin())*regionalimagewidth)*regionalimagechannels;
                                      
                                      switch (mapview)
                                      {
                                          case elevation:
                                              if (rfocused==1)
                                              {
                                                  r=regionalelevationimage[index];
                                                  g=regionalelevationimage[index+1];
                                                  b=regionalelevationimage[index+2];
                                                  
                                                  regionalelevationimage[index]=world->highlight1();
                                                  regionalelevationimage[index+1]=world->highlight2();
                                                  regionalelevationimage[index+2]=world->highlight3();
                                              }
                                              
                                              regionalmap->upload(regionalelevationimage);
                                              
                                              if (rfocused==1)
                                              {
                                                  regionalelevationimage[index]=r;
                                                  regionalelevationimage[index+1]=g;
                                                  regionalelevationimage[index+2]=b;
                                              }
                                              break;
                                              
                                          case winds:
                                              break;
                                              
                                          case temperature:
                                              if (rfocused==1)
                                              {
                                                  r=regionaltemperatureimage[index];
                                                  g=regionaltemperatureimage[index+1];
                                                  b=regionaltemperatureimage[index+2];
                                                  
                                                  regionaltemperatureimage[index]=world->highlight1();
                                                  regionaltemperatureimage[index+1]=world->highlight2();
                                                  regionaltemperatureimage[index+2]=world->highlight3();
                                              }
                                              
                                              regionalmap->upload(regionalelevationimage);
                                              
                                              if (rfocused==1)
                                              {
                                                  regionaltemperatureimage[index]=r;
                                                  regionaltemperatureimage[index+1]=g;
                                                  regionaltemperatureimage[index+2]=b;
                                              }
                                              break;
                                              
                                          case precipitation:
                                              if (rfocused==1)
                                              {
                                                  r=regionalprecipitationimage[index];
                                                  g=regionalprecipitationimage[index+1];
                                                  b=regionalprecipitationimage[index+2];
                                                  
                                                  regionalprecipitationimage[index]=world->highlight1();
                                                  regionalprecipitationimage[index+1]=world->highlight2();
                                                  regionalprecipitationimage[index+2]=world->highlight3();
                                              }
                                              
                                              regionalmap->upload(regionalprecipitationimage);
                                              
                                              if (rfocused==1)
                                              {
                                                  regionalprecipitationimage[index]=r;
                                                  regionalprecipitationimage[index+1]=g;
                                                  regionalprecipitationimage[index+2]=b;
                                              }
                                              break;
                                              
                                          case climate:
                                              if (rfocused==1)
                                              {
                                                  r=regionalclimateimage[index];
                                                  g=regionalclimateimage[index+1];
                                                  b=regionalclimateimage[index+2];
                                                  
                                                  regionalclimateimage[index]=world->highlight1();
                                                  regionalclimateimage[index+1]=world->highlight2();
                                                  regionalclimateimage[index+2]=world->highlight3();
                                              }
                                              
                                              regionalmap->upload(regionalelevationimage);
                                              
                                              if (rfocused==1)
                                              {
                                                  regionalclimateimage[index]=r;
                                                  regionalclimateimage[index+1]=g;
                                                  regionalclimateimage[index+2]=b;
                                              }
                                              break;
                                              
                                          case rivers:
                                              if (rfocused==1)
                                              {
                                                  r=regionalriversimage[index];
                                                  g=regionalriversimage[index+1];
                                                  b=regionalriversimage[index+2];
                                                  
                                                  regionalriversimage[index]=world->highlight1();
                                                  regionalriversimage[index+1]=world->highlight2();
                                                  regionalriversimage[index+2]=world->highlight3();
                                              }
                                              
                                              regionalmap->upload(regionalriversimage);
                                              
                                              if (rfocused==1)
                                              {
                                                  regionalriversimage[index]=r;
                                                  regionalriversimage[index+1]=g;
                                                  regionalriversimage[index+2]=b;
                                              }
                                              break;
                                              
                                          case relief:
                                              if (rfocused==1)
                                              {
                                                  r=regionalreliefimage[index];
                                                  g=regionalreliefimage[index+1];
                                                  b=regionalreliefimage[index+2];
                                                  
                                                  regionalreliefimage[index]=world->highlight1();
                                                  regionalreliefimage[index+1]=world->highlight2();
                                                  regionalreliefimage[index+2]=world->highlight3();
                                              }
                                              
                                              regionalmap->upload(regionalreliefimage);
                                              
                                              if (rfocused==1)
                                              {
                                                  regionalreliefimage[index]=r;
                                                  regionalreliefimage[index+1]=g;
                                                  regionalreliefimage[index+2]=b;
                                              }
                                              break;
                                              
                                      }
                                      
                                      regionalmapwidget->set_image(regionalmap);
                                      regionalmapwidget->set_offset(Vector2f(0,0));
                                      regionalmapwidget->set_scale(1.0);
                                      
                                      regionprogress->set_value(0);
                                      
                                      if (rfocused==0)
                                          regionalfocusinfo->set_caption(" ");
                                  });
    
    regionalNbutton->set_callback([&world,&region,&rfocused,&regionalmapwindow,&regionalmainbox,&regionalmapsbox,&regionalextrabitsbox,&regionalminimapwidget,&screen,&width,&height,&regionmargin,&regionalmapimagecreated,&regionprogress,&globalreliefimage,&regionalminimap,&regionalmap,&regionalmapwidget,&circletemplates,&smalllake,&island,&peaks,&riftblob,&riftblobsize,&mapview,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalfocusinfo,&globalimagewidth,&globalimageheight,&globalimagechannels,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&poix,&poiy,&smudge,&smallsmudge]
                                  {
                                      int regionalmapmove=REGIONALTILEHEIGHT; //stoi(regionalmapmoveinput->value());
                                      
                                      int newy=region->centrey()-regionalmapmove;
                                      
                                      newy=newy/32;
                                      newy=newy*32;
                                      newy=newy+16;
                                      
                                      //if (newy<regionmargin)
                                          //newy=regionmargin;
                                      
                                      region->setcentrey(newy);
                                      
                                      for (int n=0; n<GLOBALMAPTYPES; n++)
                                          regionalmapimagecreated[n]=0;
                                      
                                      if (rfocused==1)
                                      {
                                          poiy=poiy+(regionalmapmove*16);
                                          
                                          if (poiy>region->regheightend())
                                              rfocused=0;
                                      }
                                      
                                      float progressstep=1.0/REGIONALCREATIONSTEPS;
                                      regionprogress->set_value(0);
                                      
                                      // Do the minimap
                                      
                                      setregionalminimap(*world,*region,globalreliefimage,*regionalminimap,globalimagewidth,globalimageheight,globalimagechannels);
                                      
                                      regionalminimapwidget->set_image(regionalminimap);
                                      
                                      // Now create the new map.
                                      
                                      for (int n=0; n<GLOBALMAPTYPES; n++)
                                          regionalmapimagecreated[n]=0;
                                      
                                      generateregionalmap(*world,*region,*screen,*regionprogress,progressstep,circletemplates,smalllake,island,*peaks,riftblob,riftblobsize,0,smudge,smallsmudge);
                                      
                                      drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                      
                                      int r,g,b;
                                      
                                      int index=((poix-region->regwidthbegin())+(poiy-region->regheightbegin())*regionalimagewidth)*regionalimagechannels;
                                      
                                      switch (mapview)
                                      {
                                          case elevation:
                                              if (rfocused==1)
                                              {
                                                  r=regionalelevationimage[index];
                                                  g=regionalelevationimage[index+1];
                                                  b=regionalelevationimage[index+2];
                                                  
                                                  regionalelevationimage[index]=world->highlight1();
                                                  regionalelevationimage[index+1]=world->highlight2();
                                                  regionalelevationimage[index+2]=world->highlight3();
                                              }
                                              
                                              regionalmap->upload(regionalelevationimage);
                                              
                                              if (rfocused==1)
                                              {
                                                  regionalelevationimage[index]=r;
                                                  regionalelevationimage[index+1]=g;
                                                  regionalelevationimage[index+2]=b;
                                              }
                                              break;
                                              
                                          case winds:
                                              break;
                                              
                                          case temperature:
                                              if (rfocused==1)
                                              {
                                                  r=regionaltemperatureimage[index];
                                                  g=regionaltemperatureimage[index+1];
                                                  b=regionaltemperatureimage[index+2];
                                                  
                                                  regionaltemperatureimage[index]=world->highlight1();
                                                  regionaltemperatureimage[index+1]=world->highlight2();
                                                  regionaltemperatureimage[index+2]=world->highlight3();
                                              }
                                              
                                              regionalmap->upload(regionalelevationimage);
                                              
                                              if (rfocused==1)
                                              {
                                                  regionaltemperatureimage[index]=r;
                                                  regionaltemperatureimage[index+1]=g;
                                                  regionaltemperatureimage[index+2]=b;
                                              }
                                              break;
                                              
                                          case precipitation:
                                              if (rfocused==1)
                                              {
                                                  r=regionalprecipitationimage[index];
                                                  g=regionalprecipitationimage[index+1];
                                                  b=regionalprecipitationimage[index+2];
                                                  
                                                  regionalprecipitationimage[index]=world->highlight1();
                                                  regionalprecipitationimage[index+1]=world->highlight2();
                                                  regionalprecipitationimage[index+2]=world->highlight3();
                                              }
                                              
                                              regionalmap->upload(regionalprecipitationimage);
                                              
                                              if (rfocused==1)
                                              {
                                                  regionalprecipitationimage[index]=r;
                                                  regionalprecipitationimage[index+1]=g;
                                                  regionalprecipitationimage[index+2]=b;
                                              }
                                              break;
                                              
                                          case climate:
                                              if (rfocused==1)
                                              {
                                                  r=regionalclimateimage[index];
                                                  g=regionalclimateimage[index+1];
                                                  b=regionalclimateimage[index+2];
                                                  
                                                  regionalclimateimage[index]=world->highlight1();
                                                  regionalclimateimage[index+1]=world->highlight2();
                                                  regionalclimateimage[index+2]=world->highlight3();
                                              }
                                              
                                              regionalmap->upload(regionalelevationimage);
                                              
                                              if (rfocused==1)
                                              {
                                                  regionalclimateimage[index]=r;
                                                  regionalclimateimage[index+1]=g;
                                                  regionalclimateimage[index+2]=b;
                                              }
                                              break;
                                              
                                          case rivers:
                                              if (rfocused==1)
                                              {
                                                  r=regionalriversimage[index];
                                                  g=regionalriversimage[index+1];
                                                  b=regionalriversimage[index+2];
                                                  
                                                  regionalriversimage[index]=world->highlight1();
                                                  regionalriversimage[index+1]=world->highlight2();
                                                  regionalriversimage[index+2]=world->highlight3();
                                              }
                                              
                                              regionalmap->upload(regionalriversimage);
                                              
                                              if (rfocused==1)
                                              {
                                                  regionalriversimage[index]=r;
                                                  regionalriversimage[index+1]=g;
                                                  regionalriversimage[index+2]=b;
                                              }
                                              break;
                                              
                                          case relief:
                                              if (rfocused==1)
                                              {
                                                  r=regionalreliefimage[index];
                                                  g=regionalreliefimage[index+1];
                                                  b=regionalreliefimage[index+2];
                                                  
                                                  regionalreliefimage[index]=world->highlight1();
                                                  regionalreliefimage[index+1]=world->highlight2();
                                                  regionalreliefimage[index+2]=world->highlight3();
                                              }
                                              
                                              regionalmap->upload(regionalreliefimage);
                                              
                                              if (rfocused==1)
                                              {
                                                  regionalreliefimage[index]=r;
                                                  regionalreliefimage[index+1]=g;
                                                  regionalreliefimage[index+2]=b;
                                              }
                                              break;
                                              
                                      }
                                      
                                      regionalmapwidget->set_image(regionalmap);
                                      regionalmapwidget->set_offset(Vector2f(0,0));
                                      regionalmapwidget->set_scale(1.0);
                                      
                                      regionprogress->set_value(0);
                                      
                                      if (rfocused==0)
                                          regionalfocusinfo->set_caption(" ");
                                  });
    
    regionalSbutton->set_callback([&world,&region,&rfocused,&regionalmapwindow,&regionalmainbox,&regionalmapsbox,&regionalextrabitsbox,&regionalminimapwidget,&screen,&width,&height,&regionmargin,&regionalmapimagecreated,&regionprogress,&globalreliefimage,&regionalminimap,&regionalmap,&regionalmapwidget,&circletemplates,&smalllake,&island,&peaks,&riftblob,&riftblobsize,&mapview,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalfocusinfo,&globalimagewidth,&globalimageheight,&globalimagechannels,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&poix,&poiy,&smudge,&smallsmudge]
                                  {
                                      int regionalmapmove=REGIONALTILEHEIGHT; //stoi(regionalmapmoveinput->value());
                                      
                                      int newy=region->centrey()+regionalmapmove;
                                      
                                      newy=newy/32;
                                      newy=newy*32;
                                      newy=newy+16;
                                      
                                      //if (newy>world->height()-regionmargin)
                                          //newy=world->height()-regionmargin;
                                      
                                      region->setcentrey(newy);
                                      
                                      for (int n=0; n<GLOBALMAPTYPES; n++)
                                          regionalmapimagecreated[n]=0;
                                      
                                      if (rfocused==1)
                                      {
                                          poiy=poiy-(regionalmapmove*16);
                                          
                                          if (poiy<region->regheightbegin())
                                              rfocused=0;
                                      }
                                      
                                      float progressstep=1.0/REGIONALCREATIONSTEPS;
                                      regionprogress->set_value(0);
                                      
                                      // Do the minimap
                                      
                                      setregionalminimap(*world,*region,globalreliefimage,*regionalminimap,globalimagewidth,globalimageheight,globalimagechannels);
                                      
                                      regionalminimapwidget->set_image(regionalminimap);
                                      
                                      // Now create the new map.
                                      
                                      for (int n=0; n<GLOBALMAPTYPES; n++)
                                          regionalmapimagecreated[n]=0;
                                      
                                      generateregionalmap(*world,*region,*screen,*regionprogress,progressstep,circletemplates,smalllake,island,*peaks,riftblob,riftblobsize,0,smudge,smallsmudge);
                                      
                                      drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                      
                                      int r,g,b;
                                      
                                      int index=((poix-region->regwidthbegin())+(poiy-region->regheightbegin())*regionalimagewidth)*regionalimagechannels;
                                      
                                      switch (mapview)
                                      {
                                          case elevation:
                                              if (rfocused==1)
                                              {
                                                  r=regionalelevationimage[index];
                                                  g=regionalelevationimage[index+1];
                                                  b=regionalelevationimage[index+2];
                                                  
                                                  regionalelevationimage[index]=world->highlight1();
                                                  regionalelevationimage[index+1]=world->highlight2();
                                                  regionalelevationimage[index+2]=world->highlight3();
                                              }
                                              
                                              regionalmap->upload(regionalelevationimage);
                                              
                                              if (rfocused==1)
                                              {
                                                  regionalelevationimage[index]=r;
                                                  regionalelevationimage[index+1]=g;
                                                  regionalelevationimage[index+2]=b;
                                              }
                                              break;
                                              
                                          case winds:
                                              break;
                                              
                                          case temperature:
                                              if (rfocused==1)
                                              {
                                                  r=regionaltemperatureimage[index];
                                                  g=regionaltemperatureimage[index+1];
                                                  b=regionaltemperatureimage[index+2];
                                                  
                                                  regionaltemperatureimage[index]=world->highlight1();
                                                  regionaltemperatureimage[index+1]=world->highlight2();
                                                  regionaltemperatureimage[index+2]=world->highlight3();
                                              }
                                              
                                              regionalmap->upload(regionalelevationimage);
                                              
                                              if (rfocused==1)
                                              {
                                                  regionaltemperatureimage[index]=r;
                                                  regionaltemperatureimage[index+1]=g;
                                                  regionaltemperatureimage[index+2]=b;
                                              }
                                              break;
                                              
                                          case precipitation:
                                              if (rfocused==1)
                                              {
                                                  r=regionalprecipitationimage[index];
                                                  g=regionalprecipitationimage[index+1];
                                                  b=regionalprecipitationimage[index+2];
                                                  
                                                  regionalprecipitationimage[index]=world->highlight1();
                                                  regionalprecipitationimage[index+1]=world->highlight2();
                                                  regionalprecipitationimage[index+2]=world->highlight3();
                                              }
                                              
                                              regionalmap->upload(regionalprecipitationimage);
                                              
                                              if (rfocused==1)
                                              {
                                                  regionalprecipitationimage[index]=r;
                                                  regionalprecipitationimage[index+1]=g;
                                                  regionalprecipitationimage[index+2]=b;
                                              }
                                              break;
                                              
                                          case climate:
                                              if (rfocused==1)
                                              {
                                                  r=regionalclimateimage[index];
                                                  g=regionalclimateimage[index+1];
                                                  b=regionalclimateimage[index+2];
                                                  
                                                  regionalclimateimage[index]=world->highlight1();
                                                  regionalclimateimage[index+1]=world->highlight2();
                                                  regionalclimateimage[index+2]=world->highlight3();
                                              }
                                              
                                              regionalmap->upload(regionalelevationimage);
                                              
                                              if (rfocused==1)
                                              {
                                                  regionalclimateimage[index]=r;
                                                  regionalclimateimage[index+1]=g;
                                                  regionalclimateimage[index+2]=b;
                                              }
                                              break;
                                              
                                          case rivers:
                                              if (rfocused==1)
                                              {
                                                  r=regionalriversimage[index];
                                                  g=regionalriversimage[index+1];
                                                  b=regionalriversimage[index+2];
                                                  
                                                  regionalriversimage[index]=world->highlight1();
                                                  regionalriversimage[index+1]=world->highlight2();
                                                  regionalriversimage[index+2]=world->highlight3();
                                              }
                                              
                                              regionalmap->upload(regionalriversimage);
                                              
                                              if (rfocused==1)
                                              {
                                                  regionalriversimage[index]=r;
                                                  regionalriversimage[index+1]=g;
                                                  regionalriversimage[index+2]=b;
                                              }
                                              break;
                                              
                                          case relief:
                                              if (rfocused==1)
                                              {
                                                  r=regionalreliefimage[index];
                                                  g=regionalreliefimage[index+1];
                                                  b=regionalreliefimage[index+2];
                                                  
                                                  regionalreliefimage[index]=world->highlight1();
                                                  regionalreliefimage[index+1]=world->highlight2();
                                                  regionalreliefimage[index+2]=world->highlight3();
                                              }
                                              
                                              regionalmap->upload(regionalreliefimage);
                                              
                                              if (rfocused==1)
                                              {
                                                  regionalreliefimage[index]=r;
                                                  regionalreliefimage[index+1]=g;
                                                  regionalreliefimage[index+2]=b;
                                              }
                                              break;
                                              
                                      }
                                      
                                      regionalmapwidget->set_image(regionalmap);
                                      regionalmapwidget->set_offset(Vector2f(0,0));
                                      regionalmapwidget->set_scale(1.0);
                                      
                                      regionprogress->set_value(0);
                                      
                                      if (rfocused==0)
                                          regionalfocusinfo->set_caption(" ");
                                  });
    
    // Set up the callbacks for the custom area export widgets.
    
    areaexportcancelbutton->set_callback([&areaexportwindow,&globalmapwindow,&areaexportmapwidget,&areanwx,&areanwy,&areaswx,&areaswy,&areasex,&areasey,&areanex,&areaney,&globalreliefimage,&globalmap,&globalmapwidget,&regionalmapwindow,&areafromregional,&regionalminimapwidget]
                                         {
                                             globalmapwindow->set_position(areaexportwindow->position());
                                             
                                             globalmapwidget->set_scale(areaexportmapwidget->scale());
                                             globalmapwidget->set_offset(areaexportmapwidget->offset());
                                             
                                             areanex=-1;
                                             areaney=-1;
                                             areasex=-1;
                                             areasey=-1;
                                             areaswx=-1;
                                             areaswy=-1;
                                             areanwx=-1;
                                             areanwy=-1;
                                             
                                             globalmap->upload(globalreliefimage);
                                             globalmapwidget->set_image(globalmap);
                                             areaexportmapwidget->set_image(globalmap);
                                             
                                             areaexportwindow->set_visible(0);
                                             
                                             if (areafromregional==0)
                                             {
                                                 globalmapwindow->set_visible(1);
                                                 globalmapwindow->request_focus();
                                             }
                                             else
                                             {
                                                 regionalmapwindow->set_visible(1);
                                                 regionalmapwindow->request_focus();
                                             }
                                         });
    
    areaexportselectbutton->set_callback([&world,&region,&screen,&areaexportwindow,&areaexportwindowmainbox,&areaexportmapbox,&areaexportmapwidget,&areanwx,&areanwy,&areaswx,&areaswy,&areasex,&areasey,&areanex,&areaney,&globalimagewidth,&globalimagechannels,&globalreliefimage,&globalmap]
                                         {
                                             Vector2i mousepos=screen->mouse_pos();
                                             sf::Vector2i sfmousestartpos=sf::Mouse::getPosition(); // The nanogui mouse detection will only pick up where the mouse is when the button is initially clicked, so we need to use the sf mouse detection too to supplement it.
                                             
                                             this_thread::sleep_for(chrono::milliseconds{100});
                                             
                                             while (sf::Mouse::isButtonPressed(sf::Mouse::Left)==0){}
                                             
                                             sf::Vector2i sfmouseendpos=sf::Mouse::getPosition();
                                             
                                             int xdiff=sfmouseendpos.x-sfmousestartpos.x;
                                             int ydiff=sfmouseendpos.y-sfmousestartpos.y;
                                             
                                             mousepos.x()=mousepos.x()+xdiff;
                                             mousepos.y()=mousepos.y()+ydiff;
                                             
                                             bool onmap=1;
                                             
                                             if (areaexportwindow->find_widget(mousepos)==areaexportmapwidget)
                                             {

                                                 Vector2i mappos1=areaexportwindow->position();
                                                 Vector2i mappos2=areaexportwindowmainbox->position();
                                                 Vector2i mappos3=areaexportmapbox->position();
                                                 Vector2i mappos4=areaexportmapwidget->position();
                                                 
                                                 Vector2f pointpos; // Coordinates of the mouse within the map widget.
                                                 
                                                 pointpos.x()=mousepos.x()-mappos1.x()-mappos2.x()-mappos3.x()-mappos4.x();
                                                 pointpos.y()=mousepos.y()-mappos1.y()-mappos2.y()-mappos3.y()-mappos4.y();
                                                 
                                                 Vector2f poi=areaexportmapwidget->pos_to_pixel(pointpos); // Global coordinates of the point of interest.
                                                 
                                                 int startx=-1;
                                                 int starty=-1;
                                                 
                                                 if (poi.x()<0 || poi.x()>world->width()+1 || poi.y()<0 || poi.y()>world->height()+1)
                                                     onmap=0;
                                                 else
                                                 {
                                                     int highlight1=world->highlight1();
                                                     int highlight2=world->highlight2();
                                                     int highlight3=world->highlight3();
                                                     
                                                     int index;
                                                     
                                                     int startpixelr, startpixelg, startpixelb;
                                                     
                                                     if (areaswx==-1) // If we don't have any corners yet
                                                     {
                                                         areanex=poi.x();
                                                         areaney=poi.y();
                                                         areasex=poi.x();
                                                         areasey=poi.y();
                                                         areaswx=poi.x();
                                                         areaswy=poi.y();
                                                         areanwx=poi.x();
                                                         areanwy=poi.y();
                                                         
                                                         startx=poi.x(); // Get the details of this starting pixel so we can record its colour
                                                         starty=poi.y();
                                                         
                                                         index=(startx+starty*globalimagewidth)*globalimagechannels;
                                                         
                                                         startpixelr=globalreliefimage[index];
                                                         startpixelg=globalreliefimage[index+1];
                                                         startpixelb=globalreliefimage[index+2];
                                                         
                                                         globalreliefimage[index]=highlight1;
                                                         globalreliefimage[index+1]=highlight2;
                                                         globalreliefimage[index+2]=highlight3;
                                                     }
                                                     else // If we do have all corners
                                                     {
                                                         int nedistx=areanex-poi.x();
                                                         int nedisty=areaney-poi.y();
                                                         
                                                         int sedistx=areasex-poi.x();
                                                         int sedisty=areasey-poi.y();
                                                         
                                                         int swdistx=areaswx-poi.x();
                                                         int swdisty=areaswy-poi.y();
                                                         
                                                         int nwdistx=areanwx-poi.x();
                                                         int nwdisty=areanwy-poi.y();
                                                         
                                                         float nedist=sqrt(nedistx*nedistx+nedisty*nedisty);
                                                         float sedist=sqrt(sedistx*sedistx+sedisty*sedisty);
                                                         float swdist=sqrt(swdistx*swdistx+swdisty*swdisty);
                                                         float nwdist=sqrt(nwdistx*nwdistx+nwdisty*nwdisty);
                                                         
                                                         short id=2; // This will identify which corner this is going to be.
                                                         float mindist=nedist;
                                                         
                                                         if (sedist<mindist)
                                                         {
                                                             id=4;
                                                             mindist=sedist;
                                                         }
                                                         
                                                         if (swdist<mindist)
                                                         {
                                                             id=6;
                                                             mindist=swdist;
                                                         }
                                                         
                                                         if (nwdist<mindist)
                                                             id=8;
                                                         
                                                         if (id==2)
                                                         {
                                                             areanex=poi.x();
                                                             areaney=poi.y();
                                                             areasex=poi.x();
                                                             areanwy=poi.y();
                                                         }
                                                         
                                                         if (id==4)
                                                         {
                                                             areasex=poi.x();
                                                             areasey=poi.y();
                                                             areanex=poi.x();
                                                             areaswy=poi.y();
                                                         }
                                                         
                                                         if (id==6)
                                                         {
                                                             areaswx=poi.x();
                                                             areaswy=poi.y();
                                                             areanwx=poi.x();
                                                             areasey=poi.y();
                                                         }
                                                         
                                                         if (id==8)
                                                         {
                                                             areanwx=poi.x();
                                                             areanwy=poi.y();
                                                             areaswx=poi.x();
                                                             areaney=poi.y();
                                                         }
                                                         
                                                         if (areanwx>areanex)
                                                             swap(areanwx,areanex);
                                                         
                                                         if (areaswx>areasex)
                                                             swap(areaswx,areasex);
                                                         
                                                         if (areanwy>areaswy)
                                                             swap(areanwy,areaswy);
                                                         
                                                         if (areaney>areasey)
                                                             swap(areaney,areasey);

                                                     }
                                                     
                                                     // We should now have all four corners. So we just have to draw the box.
                                                     
                                                     int areawidth=(areanex-areanwx)+1;
                                                     int areaheight=(areasey-areaney)+1;
                                                     
                                                     // Create some dummy arrays to hold copies of the lines we'll be drawing over.
                                                     
#if 1
                                                     vector<stbi_uc> northsider(areawidth);
                                                     vector<stbi_uc> northsideg(areawidth);
                                                     vector<stbi_uc> northsideb(areawidth);

                                                     vector<stbi_uc> southsider(areawidth);
                                                     vector<stbi_uc> southsideg(areawidth);
                                                     vector<stbi_uc> southsideb(areawidth);

                                                     vector<stbi_uc> eastsider(areawidth);
                                                     vector<stbi_uc> eastsideg(areawidth);
                                                     vector<stbi_uc> eastsideb(areawidth);

                                                     vector<stbi_uc> westsider(areawidth);
                                                     vector<stbi_uc> westsideg(areawidth);
                                                     vector<stbi_uc> westsideb(areawidth);
#else
                                                     stbi_uc northsider[areawidth];
                                                     stbi_uc northsideg[areawidth];
                                                     stbi_uc northsideb[areawidth];
                                                     
                                                     stbi_uc southsider[areawidth];
                                                     stbi_uc southsideg[areawidth];
                                                     stbi_uc southsideb[areawidth];
                                                     
                                                     stbi_uc eastsider[areaheight];
                                                     stbi_uc eastsideg[areaheight];
                                                     stbi_uc eastsideb[areaheight];
                                                     
                                                     stbi_uc westsider[areaheight];
                                                     stbi_uc westsideg[areaheight];
                                                     stbi_uc westsideb[areaheight];
#endif
                                                     
                                                     // Now, draw our marker, copying all altered pixels onto the copy of the image.
                                                     
                                                     // Corners first.
                                                     
                                                     index=(areanwx+areanwy*globalimagewidth)*globalimagechannels;
                                                     
                                                     int nwpixelr=globalreliefimage[index];
                                                     int nwpixelg=globalreliefimage[index+1];
                                                     int nwpixelb=globalreliefimage[index+2];
                                                     
                                                     globalreliefimage[index]=highlight1;
                                                     globalreliefimage[index+1]=highlight2;
                                                     globalreliefimage[index+2]=highlight3;
                                                     
                                                     index=(areanex+areaney*globalimagewidth)*globalimagechannels;
                                                     
                                                     int nepixelr=globalreliefimage[index];
                                                     int nepixelg=globalreliefimage[index+1];
                                                     int nepixelb=globalreliefimage[index+2];
                                                     
                                                     globalreliefimage[index]=highlight1;
                                                     globalreliefimage[index+1]=highlight2;
                                                     globalreliefimage[index+2]=highlight3;
                                                     
                                                     index=(areasex+areasey*globalimagewidth)*globalimagechannels;
                                                     
                                                     int sepixelr=globalreliefimage[index];
                                                     int sepixelg=globalreliefimage[index+1];
                                                     int sepixelb=globalreliefimage[index+2];
                                                     
                                                     globalreliefimage[index]=highlight1;
                                                     globalreliefimage[index+1]=highlight2;
                                                     globalreliefimage[index+2]=highlight3;
                                                     
                                                     index=(areaswx+areaswy*globalimagewidth)*globalimagechannels;
                                                     
                                                     int swpixelr=globalreliefimage[index];
                                                     int swpixelg=globalreliefimage[index+1];
                                                     int swpixelb=globalreliefimage[index+2];
                                                     
                                                     globalreliefimage[index]=highlight1;
                                                     globalreliefimage[index+1]=highlight2;
                                                     globalreliefimage[index+2]=highlight3;
                                                     
                                                     for (int i=0; i<areawidth; i++)
                                                     {
                                                         int ii=areanwx+i;
                                                         
                                                         int j=areanwy;
                                                         
                                                         index=(ii+j*globalimagewidth)*globalimagechannels;
                                                         
                                                         northsider[i]=globalreliefimage[index];
                                                         northsideg[i]=globalreliefimage[index+1];
                                                         northsideb[i]=globalreliefimage[index+2];
                                                         
                                                         globalreliefimage[index]=highlight1;
                                                         globalreliefimage[index+1]=highlight2;
                                                         globalreliefimage[index+2]=highlight3;
                                                         
                                                         j=areaswy;
                                                         
                                                         index=(ii+j*globalimagewidth)*globalimagechannels;
                                                         
                                                         southsider[i]=globalreliefimage[index];
                                                         southsideg[i]=globalreliefimage[index+1];
                                                         southsideb[i]=globalreliefimage[index+2];
                                                         
                                                         globalreliefimage[index]=highlight1;
                                                         globalreliefimage[index+1]=highlight2;
                                                         globalreliefimage[index+2]=highlight3;
                                                     }
                                                     
                                                     for (int j=0; j<areaheight; j++)
                                                     {
                                                         int jj=areanwy+j;
                                                         
                                                         int i=areanwx;
                                                         
                                                         index=(i+jj*globalimagewidth)*globalimagechannels;
                                                         
                                                         westsider[j]=globalreliefimage[index];
                                                         westsideg[j]=globalreliefimage[index+1];
                                                         westsideb[j]=globalreliefimage[index+2];
                                                         
                                                         globalreliefimage[index]=highlight1;
                                                         globalreliefimage[index+1]=highlight2;
                                                         globalreliefimage[index+2]=highlight3;
                                                         
                                                         i=areanex;

                                                         index=(i+jj*globalimagewidth)*globalimagechannels;
                                                         
                                                         eastsider[j]=globalreliefimage[index];
                                                         eastsideg[j]=globalreliefimage[index+1];
                                                         eastsideb[j]=globalreliefimage[index+2];
                                                         
                                                         globalreliefimage[index]=highlight1;
                                                         globalreliefimage[index+1]=highlight2;
                                                         globalreliefimage[index+2]=highlight3;
                                                     }
                                                     
                                                     // Now apply the modified image to the map.
                                                     
                                                     globalmap->upload(globalreliefimage);
                                                     
                                                     // Now restore the changed pixels.
                                                     
                                                     for (int i=0; i<areawidth; i++)
                                                     {
                                                         int ii=areanwx+i;
                                                      
                                                         int j=areanwy;
                                                      
                                                         index=(ii+j*globalimagewidth)*globalimagechannels;
                                                      
                                                         globalreliefimage[index]=northsider[i];
                                                         globalreliefimage[index+1]=northsideg[i];
                                                         globalreliefimage[index+2]=northsideb[i];
                                                      
                                                         j=areaswy;
                                                      
                                                         index=(ii+j*globalimagewidth)*globalimagechannels;
                                                      
                                                         globalreliefimage[index]=southsider[i];
                                                         globalreliefimage[index+1]=southsideg[i];
                                                         globalreliefimage[index+2]=southsideb[i];
                                                     }
                                                     
                                                     for (int j=0; j<areaheight; j++)
                                                     {
                                                         int jj=areanwy+j;
                                                      
                                                         int i=areanwx;
                                                      
                                                         if (j>0)
                                                         {
                                                             index=(i+jj*globalimagewidth)*globalimagechannels;
                                                      
                                                             globalreliefimage[index]=westsider[j];
                                                             globalreliefimage[index+1]=westsideg[j];
                                                             globalreliefimage[index+2]=westsideb[j];
                                                         }
                                                      
                                                         i=areanex;

                                                         index=(i+jj*globalimagewidth)*globalimagechannels;
                                                      
                                                         globalreliefimage[index]=eastsider[j];
                                                         globalreliefimage[index+1]=eastsideg[j];
                                                         globalreliefimage[index+2]=eastsideb[j];
                                                     }
                                                     
                                                     index=(areanwx+areanwy*globalimagewidth)*globalimagechannels;
                                                     
                                                     globalreliefimage[index]=nwpixelr;
                                                     globalreliefimage[index+1]=nwpixelg;
                                                     globalreliefimage[index+2]=nwpixelb;
                                                     
                                                     index=(areanex+areaney*globalimagewidth)*globalimagechannels;
                                                     
                                                     globalreliefimage[index]=nepixelr;
                                                     globalreliefimage[index+1]=nepixelg;
                                                     globalreliefimage[index+2]=nepixelb;
                                                     
                                                     index=(areasex+areasey*globalimagewidth)*globalimagechannels;
                                                     
                                                     globalreliefimage[index]=sepixelr;
                                                     globalreliefimage[index+1]=sepixelg;
                                                     globalreliefimage[index+2]=sepixelb;
                                                     
                                                     index=(areaswx+areaswy*globalimagewidth)*globalimagechannels;
                                                     
                                                     globalreliefimage[index]=swpixelr;
                                                     globalreliefimage[index+1]=swpixelg;
                                                     globalreliefimage[index+2]=swpixelb;
                                                     
                                                     index=(startx+starty*globalimagewidth)*globalimagechannels;
                                                     
                                                     globalreliefimage[index]=startpixelr;
                                                     globalreliefimage[index+1]=startpixelg;
                                                     globalreliefimage[index+2]=startpixelb;
                                                     
                                                     areaexportmapwidget->set_image(globalmap);

                                                 }
                                             }
                                         });
    
    areaexportclearbutton->set_callback([&areaexportmapwidget,&areanwx,&areanwy,&areaswx,&areaswy,&areasex,&areasey,&areanex,&areaney,&globalreliefimage,&globalmap,&globalmapwidget]
                                        {
                                            areanex=-1;
                                            areaney=-1;
                                            areasex=-1;
                                            areasey=-1;
                                            areaswx=-1;
                                            areaswy=-1;
                                            areanwx=-1;
                                            areanwy=-1;
                                            
                                            globalmap->upload(globalreliefimage);
                                            globalmapwidget->set_image(globalmap);
                                            areaexportmapwidget->set_image(globalmap);
                                        });
    
    areaexportexportbutton->set_callback([&screen,&areaexportprogresswindow,&areaexportprogress,&globalmapwindow,&regionalmapwindow,&areaexportwindow,&world,&region,&mapview,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&circletemplates,&smalllake,&island,&peaks,&riftblob,&riftblobsize,&smudge,&smallsmudge,&areanwx,&areanwy,&areasex,&areasey,&areanex,&areaney,&areaswx,&areaswy,&globalmap,&globalreliefimage,&globalmapwidget,&areaexportmapwidget,&areafromregional,&regionalmap,&regionalmapwidget,&regionprogress,&regionalminimapwidget]
                                         {
                                             globalmapwindow->set_position(areaexportwindow->position());
                                             
                                             globalmapwindow->set_visible(0);
                                             regionalmapwindow->set_visible(0);
                                             areaexportwindow->set_visible(0);
                                             areaexportprogresswindow->set_visible(1);
                                             areaexportprogresswindow->request_focus();
                                             
                                             int oldregionalcentrex=region->centrex();
                                             int oldregionalcentrey=region->centrey();
                                             
                                             mapviewenum oldmapview=mapview;
                                             
                                             initialiseregion(*world,*region); // We'll do all this using the same region object as usual. We could create a new region object for it, but that seems to lead to inexplicable crashes, so we won't.
                                             
                                             float regiontilewidth=REGIONALTILEWIDTH; //30;
                                             float regiontileheight=REGIONALTILEHEIGHT; //30; // The width and height of the visible regional map, in tiles.
                                             
                                             int regionwidth=regiontilewidth*16;
                                             int regionheight=regiontileheight*16; // The width and height of the visible regional map, in pixels.
                                             
                                             int origareanwx=areanwx; // This is because the regions we'll be making will start to the north and west of the defined area.
                                             int origareanwy=areanwy;
                                             
                                             int origareanex=areanex;
                                             int origareaney=areaney;
                                             
                                             int origareaswx=areaswx;
                                             int origareaswy=areaswy;
                                             
                                             areanwx=areanwx/regiontilewidth;
                                             areanwx=areanwx*regiontilewidth;
                                             
                                             areanwy=areanwy/regiontileheight;
                                             areanwy=areanwy*regiontileheight;
                                             
                                             areaswx=areanwx;
                                             areaney=areanwy;
                                             
                                             int woffset=(origareanwx-areanwx)*16;
                                             int noffset=(origareanwy-areanwy)*16;

                                             float areatilewidth=areanex-areanwx;
                                             float areatileheight=areasey-areaney;
                                             
                                             int areawidth=areatilewidth*16;
                                             int areaheight=areatileheight*16;
                                             
                                             float imageareatilewidth=origareanex-origareanwx;
                                             float imageareatileheight=areasey-origareaney;
                                             
                                             int imageareawidth=imageareatilewidth*16;
                                             int imageareaheight=imageareatileheight*16;
                                             
                                             float fregionswide=areatilewidth/regiontilewidth;
                                             float fregionshigh=areatileheight/regiontileheight;
                                             
                                             int regionswide=fregionswide;
                                             int regionshigh=fregionshigh;
                                             
                                             if (regionswide!=fregionswide)
                                                 regionswide++;
                                             
                                             if (regionshigh!=fregionshigh)
                                                 regionshigh++;
                                             
                                             int totalregions=regionswide*regionshigh; // This is how many regional maps we're going to have to do.
                                             
                                             if (areafromregional==1)
                                                 totalregions++; // Because we'll have to redo the regional map we came from.
                                             
                                             float progressstep=1.0/(totalregions*REGIONALCREATIONSTEPS);
                                             areaexportprogress->set_value(0);
                                             
                                             // Now we need to prepare the images that we're going to copy the regional maps onto.
                                             
                                             sf::Image *areareliefimage=new sf::Image;
                                             sf::Image *areaelevationimage=new sf::Image;
                                             sf::Image *areatemperatureimage=new sf::Image;
                                             sf::Image *areaprecipitationimage=new sf::Image;
                                             sf::Image *areaclimateimage=new sf::Image;
                                             sf::Image *areariversimage=new sf::Image;
                                             
                                             areareliefimage->create(imageareawidth+1,imageareaheight+1,sf::Color::Black);
                                             areaelevationimage->create(imageareawidth+1,imageareaheight+1,sf::Color::Black);
                                             areatemperatureimage->create(imageareawidth+1,imageareaheight+1,sf::Color::Black);
                                             areaprecipitationimage->create(imageareawidth+1,imageareaheight+1,sf::Color::Black);
                                             areaclimateimage->create(imageareawidth+1,imageareaheight+1,sf::Color::Black);
                                             areariversimage->create(imageareawidth+1,imageareaheight+1,sf::Color::Black);
                                             
                                             // Now it's time to make the regions, one by one, generate their maps, and copy those maps over onto the area images.
                                             
                                             for (int i=0; i<regionswide; i++)
                                             {
                                                 int centrex=i*regiontilewidth+(regiontilewidth/2)+areanwx;
                                                 
                                                 for (int j=0; j<regionshigh; j++)
                                                 {
                                                     int centrey=j*regiontileheight+(regiontileheight/2)+areanwy;

                                                     // First, create the new region.
                                                     
                                                     region->setcentrex(centrex);
                                                     region->setcentrey(centrey);
                                                     
                                                     generateregionalmap(*world,*region,*screen,*areaexportprogress,progressstep,circletemplates,smalllake,island,*peaks,riftblob,riftblobsize,0,smudge,smallsmudge);
                                                     
                                                     // Now generate the maps.
                                                     
                                                     for (int n=0; n<GLOBALMAPTYPES; n++)
                                                         regionalmapimagecreated[n]=0;
                                                     
                                                     mapview=relief;
                                                     drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                                     
                                                     mapview=elevation;
                                                     drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                                     
                                                     mapview=temperature;
                                                     drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                                     
                                                     mapview=precipitation;
                                                     drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                                     
                                                     mapview=climate;
                                                     drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                                     
                                                     mapview=rivers;
                                                     drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);

                                                     // Now copy those maps into the images that will be exported.
                                                     
                                                     int index, r, g, b;
                                                     
                                                     for (int x=0; x<regionalimagewidth; x++)
                                                     {
                                                         for (int y=0; y<regionalimageheight; y++)
                                                         {
                                                             int thisx=x+i*regionwidth-woffset;
                                                             int thisy=y+j*regionheight-noffset; // Coordinates on the export image that correspond to this point on the regional map
                                                             
                                                             if (thisx>0 && thisx<imageareawidth && thisy>0 && thisy<imageareaheight)
                                                             {
                                                                 index=(x+y*regionalimagewidth)*regionalimagechannels;
                                                                 
                                                                 r=regionalreliefimage[index];
                                                                 g=regionalreliefimage[index+1];
                                                                 b=regionalreliefimage[index+2];
                                                                 
                                                                 areareliefimage->setPixel(thisx,thisy,sf::Color(r,g,b));
                                                                 
                                                                 r=regionalelevationimage[index];
                                                                 g=regionalelevationimage[index+1];
                                                                 b=regionalelevationimage[index+2];
                                                                 
                                                                 areaelevationimage->setPixel(thisx,thisy,sf::Color(r,g,b));
                                                                 
                                                                 r=regionaltemperatureimage[index];
                                                                 g=regionaltemperatureimage[index+1];
                                                                 b=regionaltemperatureimage[index+2];
                                                                 
                                                                 areatemperatureimage->setPixel(thisx,thisy,sf::Color(r,g,b));
                                                                 
                                                                 r=regionalprecipitationimage[index];
                                                                 g=regionalprecipitationimage[index+1];
                                                                 b=regionalprecipitationimage[index+2];
                                                                 
                                                                 areaprecipitationimage->setPixel(thisx,thisy,sf::Color(r,g,b));
                                                                 
                                                                 r=regionalclimateimage[index];
                                                                 g=regionalclimateimage[index+1];
                                                                 b=regionalclimateimage[index+2];
                                                                 
                                                                 areaclimateimage->setPixel(thisx,thisy,sf::Color(r,g,b));
                                                                 
                                                                 r=regionalriversimage[index];
                                                                 g=regionalriversimage[index+1];
                                                                 b=regionalriversimage[index+2];
                                                                 
                                                                 areariversimage->setPixel(thisx,thisy,sf::Color(r,g,b));
                                                             }
                                                         }
                                                     }
                                                 }
                                             }
                                             
                                             region->setcentrex(oldregionalcentrex); // Move the region back to where it started.
                                             region->setcentrey(oldregionalcentrey);
                                             
                                             if (areafromregional==1) // If we're going to go back to the regional map, we need to redo it.
                                             {
                                                 generateregionalmap(*world,*region,*screen,*areaexportprogress,progressstep,circletemplates,smalllake,island,*peaks,riftblob,riftblobsize,0,smudge,smallsmudge);
                                             }
                                             
                                             // Now just save the images.
                                             
                                             string filename=file_dialog({{"png", "Portable Network Graphics"}},true);
                                             
                                             if (filename!="")
                                             {
                                                 filename.resize(filename.size()-4);

                                                 areareliefimage->saveToFile(filename+" Relief.png");
                                                 areaelevationimage->saveToFile(filename+" Elevation.png");
                                                 areatemperatureimage->saveToFile(filename+" Temperature.png");
                                                 areaprecipitationimage->saveToFile(filename+" Precipitation.png");
                                                 areaclimateimage->saveToFile(filename+" Climate.png");
                                                 areariversimage->saveToFile(filename+" Rivers.png");
                                             }
                                             
                                             // Clean up.

                                             delete areareliefimage;
                                             delete areaelevationimage;
                                             delete areatemperatureimage;
                                             delete areaprecipitationimage;
                                             delete areaclimateimage;
                                             delete areariversimage;
                                             
                                             areanex=-1;
                                             areaney=-1;
                                             areasex=-1;
                                             areasey=-1;
                                             areaswx=-1;
                                             areaswy=-1;
                                             areanwx=-1;
                                             areanwy=-1;
                                             
                                             mapview=oldmapview;
                                             
                                             globalmap->upload(globalreliefimage);
                                             globalmapwidget->set_image(globalmap);
                                             areaexportmapwidget->set_image(globalmap);
                                             
                                             areaexportwindow->set_visible(0);
                                             areaexportprogresswindow->set_visible(0);
                                             
                                             globalmapwidget->set_scale(areaexportmapwidget->scale());
                                             globalmapwidget->set_offset(areaexportmapwidget->offset());
                                             
                                             for (int n=0; n<GLOBALMAPTYPES; n++)
                                                 regionalmapimagecreated[n]=0;
                                             
                                             if (areafromregional==0)
                                             {
                                                 globalmapwindow->set_visible(1);
                                                 globalmapwindow->request_focus();
                                             }
                                             else
                                             {
                                                 regionalmapwindow->set_visible(1);
                                                 regionalmapwindow->request_focus();
                                             }
                                         });
    
    // Set up the callbacks for the import widgets.
    
    importcancelbutton->set_callback([&importwindow,&getseedwindow,&seedinput,&cancelbutton,&brandnew,&OKbutton,&importbutton,&globalmapwindow]
                                 {
                                     globalmapwindow->set_position(importwindow->position());
                                     
                                     brandnew=1;
                                     seedinput->set_value("");
                                     OKbutton->set_pushed(0);
                                     importbutton->set_pushed(0);
                                     cancelbutton->set_pushed(0);
                                     importwindow->set_visible(0);
                                     
                                     getseedwindow->set_visible(1);
                                     getseedwindow->request_focus();
                                 });
    
    importlandmapbutton->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globalwindsimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&importmapwidget]
                                      {
                                          int width=world->width();
                                          int height=world->height();
                                          int sealevel=world->sealevel();
                                          
                                          string filename=nanogui::file_dialog({{"png", "Portable Network Graphics"}},0);
                                          
                                          if (filename!="")
                                          {
                                              sf::Image *importimage=new sf::Image;
                                              
                                              importimage->loadFromFile(filename);
                                              
                                              sf::Vector2u imagesize=importimage->getSize();
                                              
                                              if (imagesize.x==width+1 && imagesize.y==height+1)
                                              {
                                                  // First, load the elevations into the world.
                                                  
                                                  for (int i=0; i<=width; i++)
                                                  {
                                                      for (int j=0; j<=height; j++)
                                                      {
                                                          sf::Color colour=importimage->getPixel(i,j);
                                                          
                                                          if (colour.r!=0)
                                                          {
                                                              int elev=colour.r*10+sealevel;
                                                              
                                                              world->setnom(i,j,elev);
                                                          }
                                                      }
                                                  }
                                                  
                                                  // Now just redo the map.
                                                  
                                                  for (int n=0; n<GLOBALMAPTYPES; n++)
                                                      globalmapimagecreated[n]=0;
                                                  drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                                  
                                                  globalmap->upload(globalreliefimage);
                                                  
                                                  importmapwidget->set_image(globalmap);
                                              }
                                          }
                                      });
    
    importseamapbutton->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globalwindsimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&importmapwidget]
                                      {
                                          int width=world->width();
                                          int height=world->height();
                                          int sealevel=world->sealevel();
                                          
                                          string filename=nanogui::file_dialog({{"png", "Portable Network Graphics"}},0);
                                          
                                          if (filename!="")
                                          {
                                              sf::Image *importimage=new sf::Image;
                                              
                                              importimage->loadFromFile(filename);
                                              
                                              sf::Vector2u imagesize=importimage->getSize();
                                              
                                              if (imagesize.x==width+1 && imagesize.y==height+1)
                                              {
                                                  // First, load the elevations into the world.
                                                  
                                                  for (int i=0; i<=width; i++)
                                                  {
                                                      for (int j=0; j<=height; j++)
                                                      {
                                                          sf::Color colour=importimage->getPixel(i,j);
                                                          
                                                          if (colour.r!=0)
                                                          {
                                                              int elev=sealevel-colour.r*50;
                                                              
                                                              if (elev<1)
                                                                  elev=1;
                                                              
                                                              world->setnom(i,j,elev);
                                                          }
                                                      }
                                                  }
                                                  
                                                  // Now just redo the map.
                                                  
                                                  for (int n=0; n<GLOBALMAPTYPES; n++)
                                                      globalmapimagecreated[n]=0;
                                                  drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                                  
                                                  globalmap->upload(globalreliefimage);
                                                  
                                                  importmapwidget->set_image(globalmap);
                                              }
                                          }
                                      });
    
    importmountainsbutton->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globalwindsimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&importmapwidget]
                                     {
                                         int width=world->width();
                                         int height=world->height();
                                         int sealevel=world->sealevel();
                                         
                                         string filename=nanogui::file_dialog({{"png", "Portable Network Graphics"}},0);
                                         
                                         if (filename!="")
                                         {
                                             sf::Image *importimage=new sf::Image;
                                             
                                             importimage->loadFromFile(filename);
                                             
                                             sf::Vector2u imagesize=importimage->getSize();
                                             
                                             if (imagesize.x==width+1 && imagesize.y==height+1)
                                             {
                                                 // First, create an array for the raw mountain heights, and load them in.
                                                 
                                                 vector<vector<int>> rawmountains(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
                                                 
                                                 for (int i=0; i<=width; i++)
                                                 {
                                                     for (int j=0; j<=height; j++)
                                                     {
                                                         sf::Color colour=importimage->getPixel(i,j);
                                                         
                                                         if (colour.r!=0)
                                                         {
                                                             int elev=colour.r*65; // In theory *50, but this seems to result in roughly the right heights.
                                                             
                                                             rawmountains[i][j]=elev;
                                                         }
                                                         else
                                                             rawmountains[i][j]=0;
                                                     }
                                                 }
                                                 
                                                 // Now turn the raw mountains array into actual mountains.
                                                 
                                                 createmountainsfromraw(*world,rawmountains);
                                                 
                                                 
                                                 // Now just redo the map.
                                                 
                                                 for (int n=0; n<GLOBALMAPTYPES; n++)
                                                     globalmapimagecreated[n]=0;
                                                 drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                                 
                                                 globalmap->upload(globalreliefimage);
                                                 
                                                 importmapwidget->set_image(globalmap);
                                             }
                                         }
                                     });
    
    importvolcanoesbutton->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globalwindsimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&importmapwidget]
                                        {
                                            int width=world->width();
                                            int height=world->height();
                                            int sealevel=world->sealevel();
                                            
                                            string filename=nanogui::file_dialog({{"png", "Portable Network Graphics"}},0);
                                            
                                            if (filename!="")
                                            {
                                                sf::Image *importimage=new sf::Image;
                                                
                                                importimage->loadFromFile(filename);
                                                
                                                sf::Vector2u imagesize=importimage->getSize();
                                                
                                                if (imagesize.x==width+1 && imagesize.y==height+1)
                                                {
                                                    // First, load the volcanoes into the world.
                                                    
                                                    for (int i=0; i<=width; i++)
                                                    {
                                                        for (int j=0; j<=height; j++)
                                                        {
                                                            sf::Color colour=importimage->getPixel(i,j);
                                                            
                                                            if (colour.r!=0)
                                                            {
                                                                int elev=colour.r*45; // In theory *50, but this seems to give more accurate results.
                                                                
                                                                bool strato=0;
                                                                
                                                                if (colour.g>0)
                                                                {
                                                                    strato=1;
                                                                }
                                                                else
                                                                    elev=elev/2; // Because shield volcanoes have a lot of extra elevation anyway.
                                                                
                                                                if (colour.b==0)
                                                                    elev=0-elev;
                                                                
                                                                world->setvolcano(i,j,elev);
                                                                world->setstrato(i,j,strato);
                                                            }
                                                        }
                                                    }
                                                    
                                                    // Now just redo the map.
                                                    
                                                    for (int n=0; n<GLOBALMAPTYPES; n++)
                                                        globalmapimagecreated[n]=0;
                                                    drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                                    
                                                    globalmap->upload(globalreliefimage);
                                                    
                                                    importmapwidget->set_image(globalmap);
                                                }
                                            }
                                        });
    
    importgenshelvesbutton->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globalwindsimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&importmapwidget]
                                         {
                                             int width=world->width();
                                             int height=world->height();
                                             int maxelev=world->maxelevation();
                                             int sealevel=world->sealevel();
                                             
                                             vector<vector<bool>> shelves(ARRAYWIDTH,vector<bool>(ARRAYWIDTH,ARRAYHEIGHT));
                                             
                                             for (int i=0; i<=width; i++)
                                             {
                                                 for (int j=0; j<=height; j++)
                                                     shelves[i][j]=0;
                                             }
                                             
                                             makecontinentalshelves(*world,shelves,4);
                                             
                                             int grain=8; // Level of detail on this fractal map.
                                             float valuemod=0.2;
                                             int v=random(3,6);
                                             float valuemod2=v;
                                             
                                             vector<vector<int>> seafractal(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
                                             
                                             createfractal(seafractal,width,height,grain,valuemod,valuemod2,1,maxelev,0,0);
                                             
                                             float coastalvarreduce=maxelev/3000;
                                             float oceanvarreduce=maxelev/1000;
                                             
                                             for (int i=0; i<=width; i++)
                                             {
                                                 for (int j=0; j<=height; j++)
                                                 {
                                                     if (world->sea(i,j)==1 && shelves[i][j]==1)
                                                         world->setnom(i,j,sealevel-100);
                                                 }
                                             }
                                             
                                             // Now just redo the map.
                                             
                                             for (int n=0; n<GLOBALMAPTYPES; n++)
                                                 globalmapimagecreated[n]=0;
                                             drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                             
                                             globalmap->upload(globalreliefimage);
                                             
                                             importmapwidget->set_image(globalmap);
                                         });
    
    importgenridgesbutton->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globalwindsimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&importmapwidget]
                            {
                                int width=world->width();
                                int height=world->height();
                                int maxelev=world->maxelevation();
                                int sealevel=world->sealevel();
                                
                                vector<vector<bool>> shelves(ARRAYWIDTH,vector<bool>(ARRAYWIDTH,ARRAYHEIGHT));
                                
                                for (int i=0; i<=width; i++)
                                {
                                    for (int j=0; j<=height; j++)
                                    {
                                        if (world->nom(i,j)>sealevel-400)
                                                shelves[i][j]=1;
                                            else
                                                shelves[i][j]=0;
                                    }
                                }
                                
                                createoceanridges(*world,shelves);
                                
                                // Now just redo the map.
                                
                                for (int n=0; n<GLOBALMAPTYPES; n++)
                                    globalmapimagecreated[n]=0;
                                drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                
                                globalmap->upload(globalreliefimage);
                                
                                importmapwidget->set_image(globalmap);
                            });
    
    importgenlandbutton->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globalwindsimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&importmapwidget]
                                      {
                                          int width=world->width();
                                          int height=world->height();
                                          int maxelev=world->maxelevation();
                                          int sealevel=world->sealevel();
                                          
                                          // First, make a fractal map.
                                          
                                          int grain=8; // Level of detail on this fractal map.
                                          float valuemod=0.2;
                                          int v=random(3,6);
                                          float valuemod2=v;
                                          
                                          vector<vector<int>> fractal(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));

                                          createfractal(fractal,width,height,grain,valuemod,valuemod2,1,12750,0,0);
                                          
                                          int fractaladd=sealevel-2500;
                                          
                                          for (int i=0; i<=width; i++)
                                          {
                                              for (int j=0; j<=height; j++)
                                                  fractal[i][j]=fractal[i][j]+fractaladd;
                                          }
                                          
                                          // Now use it to change the land heights.
                                          
                                          fractaladdland(*world,fractal);
                                          
                                          // Smooth the land.
                                          
                                          smoothland(*world,2);
                                          
                                          // Also, create extra elevation.
                                          
                                          createextraelev(*world);
                                          
                                          // Now just redo the map.
                                          
                                          for (int n=0; n<GLOBALMAPTYPES; n++)
                                              globalmapimagecreated[n]=0;
                                          drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                          
                                          globalmap->upload(globalreliefimage);
                                          
                                          importmapwidget->set_image(globalmap);
                                      });
    
    importgenseabedbutton->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globalwindsimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&importmapwidget]
                                        {
                                            int width=world->width();
                                            int height=world->height();
                                            int maxelev=world->maxelevation();
                                            int sealevel=world->sealevel();
                                            
                                            int grain=8; // Level of detail on this fractal map.
                                            float valuemod=0.2;
                                            int v=random(3,6);
                                            float valuemod2=v;
                                            
                                            vector<vector<int>> seafractal(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
                                            
                                            createfractal(seafractal,width,height,grain,valuemod,valuemod2,1,maxelev,0,0);
                                            
                                            float coastalvarreduce=maxelev/3000;
                                            float oceanvarreduce=maxelev/1000;
                                            
                                            for (int i=0; i<=width; i++)
                                            {
                                                for (int j=0; j<=height; j++)
                                                {
                                                    if (world->sea(i,j)==1)
                                                    {
                                                        bool shelf=1;
                                                        
                                                        if (world->nom(i,j)<sealevel-300)
                                                            shelf=0;
                                                        
                                                        if (shelf==1)
                                                        {
                                                            float var=seafractal[i][j]-maxelev/2;
                                                            var=var/coastalvarreduce;
                                                            
                                                            int newval=world->nom(i,j)+var;
                                                            
                                                            if (newval>sealevel-10)
                                                                newval=sealevel-10;
                                                            
                                                            if (newval<1)
                                                                newval=1;
                                                            
                                                            world->setnom(i,j,newval);
                                                        }
                                                        else
                                                        {
                                                            int ii=i+width/2;
                                                            
                                                            if (ii>width)
                                                                ii=ii-width;
                                                            
                                                            float var=seafractal[ii][j]-maxelev/2;
                                                            var=var/oceanvarreduce;
                                                            
                                                            int newval=world->nom(i,j)+var;
                                                            
                                                            if (newval>sealevel-3000)
                                                                newval=sealevel-3000;
                                                            
                                                            if (newval<1)
                                                                newval=1;
                                                            
                                                            world->setnom(i,j,newval);
                                                        }
                                                    }
                                                }
                                            }
                                            
                                            // Now just redo the map.
                                            
                                            for (int n=0; n<GLOBALMAPTYPES; n++)
                                                globalmapimagecreated[n]=0;
                                            drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                            
                                            globalmap->upload(globalreliefimage);
                                            
                                            importmapwidget->set_image(globalmap);
                                        });

    /*
    importgenislandsbutton->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globalwindsimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&importmapwidget,&landshape]
                                           {
                                               int seed=abs(world->seed());
                                               
                                               int width=world->width();
                                               int height=world->height();
                                               int maxelev=world->maxelevation();
                                               int sealevel=world->sealevel();
                                               
                                               int baseheight=sealevel-4500;
                                               if (baseheight<1)
                                                   baseheight=1;
                                               int conheight=sealevel+50;
                                               
                                               vector<vector<int>> plateaumap(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
                                               
                                               for (int i=0; i<=width; i++)
                                               {
                                                   for (int j=0; j<=height; j++)
                                                       plateaumap[i][j]=0;
                                               }
                                               
                                               // First, make a fractal map.
                                               
                                               int grain=8; // Level of detail on this fractal map.
                                               float valuemod=0.2;
                                               int v=random(3,6);
                                               float valuemod2=v;
                                               
                                               vector<vector<int>> fractal(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
                                               
                                               createfractal(fractal,width,height,grain,valuemod,valuemod2,1,12750,0,0);
                                               
                                               int fractaladd=sealevel-2500;
                                               
                                               for (int i=0; i<=width; i++)
                                               {
                                                   for (int j=0; j<=height; j++)
                                                       fractal[i][j]=fractal[i][j]+fractaladd;
                                               }
                                               
                                               //fast_srand(seed);

                                               sf::Vector2i mountainfocuspoints[4]; // These will be points where mountains and islands will start close to.
                                               
                                               int mountainfocustotal=1; //random(2,4); // The number of actual focus points.
                                               
                                               for (int n=0; n<mountainfocustotal; n++)
                                               {
                                                   mountainfocuspoints[n].x=random(0,width);
                                                   mountainfocuspoints[n].y=random(0,height);
                                               }
                                               
                                               int mountainfocaldistance=random(height/8,height/4);
                                               
                                               //fast_srand(time(0));
                                               
                                               createchains(*world,baseheight,conheight,fractal,plateaumap,landshape,mountainfocuspoints,mountainfocustotal,mountainfocaldistance,3);
                                               
                                               // Now just redo the map.
                                               
                                               for (int n=0; n<GLOBALMAPTYPES; n++)
                                                   globalmapimagecreated[n]=0;
                                               
                                               mapview=relief;
                                               drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                               
                                               globalmap->upload(globalreliefimage);
                                               
                                               importmapwidget->set_image(globalmap);
                                           });
    */
    
    importgenmountainsbutton->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globalwindsimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&importmapwidget,&landshape,&chainland]
                                           {
                                               int width=world->width();
                                               int height=world->height();
                                               int maxelev=world->maxelevation();
                                               int sealevel=world->sealevel();
                                               
                                               int baseheight=sealevel-4500;
                                               if (baseheight<1)
                                                   baseheight=1;
                                               int conheight=sealevel+50;
                                               
                                               vector<vector<int>> plateaumap(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
                                               
                                               for (int i=0; i<=width; i++)
                                               {
                                                   for (int j=0; j<=height; j++)
                                                       plateaumap[i][j]=0;
                                               }
                                               
                                               // First, make a fractal map.
                                               
                                               int grain=8; // Level of detail on this fractal map.
                                               float valuemod=0.2;
                                               int v=random(3,6);
                                               float valuemod2=v;
                                               
                                               vector<vector<int>> fractal(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
                                               
                                               createfractal(fractal,width,height,grain,valuemod,valuemod2,1,12750,0,0);
                                               
                                               int fractaladd=sealevel-2500;
                                               
                                               for (int i=0; i<=width; i++)
                                               {
                                                   for (int j=0; j<=height; j++)
                                                       fractal[i][j]=fractal[i][j]+fractaladd;
                                               }
                                               
                                               sf::Vector2i dummy[1];
                                               
                                               createchains(*world,baseheight,conheight,fractal,plateaumap,landshape,chainland,dummy,0,0,5);
                                               
                                               // Now just redo the map.
                                               
                                               for (int n=0; n<GLOBALMAPTYPES; n++)
                                                   globalmapimagecreated[n]=0;
                                               drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                               
                                               globalmap->upload(globalreliefimage);
                                               
                                               importmapwidget->set_image(globalmap);
                                           });
    
    importgenhillsbutton->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globalwindsimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&importmapwidget,&landshape,&chainland]
                                           {
                                               int width=world->width();
                                               int height=world->height();
                                               int maxelev=world->maxelevation();
                                               int sealevel=world->sealevel();
                                               
                                               int baseheight=sealevel-4500;
                                               if (baseheight<1)
                                                   baseheight=1;
                                               int conheight=sealevel+50;
                                               
                                               vector<vector<int>> plateaumap(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
                                               
                                               for (int i=0; i<=width; i++)
                                               {
                                                   for (int j=0; j<=height; j++)
                                                       plateaumap[i][j]=0;
                                               }
                                               
                                               // First, make a fractal map.
                                               
                                               int grain=8; // Level of detail on this fractal map.
                                               float valuemod=0.2;
                                               int v=random(3,6);
                                               float valuemod2=v;
                                               
                                               vector<vector<int>> fractal(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
                                               
                                               createfractal(fractal,width,height,grain,valuemod,valuemod2,1,12750,0,0);
                                               
                                               int fractaladd=sealevel-2500;
                                               
                                               for (int i=0; i<=width; i++)
                                               {
                                                   for (int j=0; j<=height; j++)
                                                       fractal[i][j]=fractal[i][j]+fractaladd;
                                               }
                                               
                                               sf::Vector2i dummy[1];
                                               
                                               createchains(*world,baseheight,conheight,fractal,plateaumap,landshape,chainland,dummy,0,0,5);
                                               
                                               // Now just redo the map.
                                               
                                               for (int n=0; n<GLOBALMAPTYPES; n++)
                                                   globalmapimagecreated[n]=0;
                                               drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                               
                                               globalmap->upload(globalreliefimage);
                                               
                                               importmapwidget->set_image(globalmap);
                                           });
    
    importgencoastsbutton->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globalwindsimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&importmapwidget,&landshape]
                                        {
                                            removestraights(*world);
                                           
                                            // Now just redo the map.
                                            
                                            for (int n=0; n<GLOBALMAPTYPES; n++)
                                                globalmapimagecreated[n]=0;
                                            
                                            mapview=relief;
                                            drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                            
                                            globalmap->upload(globalreliefimage);
                                            
                                            importmapwidget->set_image(globalmap);
                                        });
    
    importgenclimatebutton->set_callback([&screen,&importwindow,&globalmapwindow,&globalmapwidget,&globalmap,&globalmapimagecreated,&worldgenerationwindow,&worldprogress,&generationlabel,&world,&smalllake,&largelake,&landshape,&mapview,&globalelevationimage,&globalwindsimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels]
                                      {
                                          globalmapwindow->set_position(importwindow->position());
                                          
                                          int width=world->width();
                                          int height=world->height();
                                          int maxelev=world->maxelevation();

                                          importwindow->set_visible(0);
                                          
                                          vector<vector<int>> mountaindrainage(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
                                          vector<vector<bool>> shelves(ARRAYWIDTH,vector<bool>(ARRAYWIDTH,ARRAYHEIGHT));
                                          
                                          for (int i=0; i<=width; i++)
                                          {
                                              for (int j=0; j<=height; j++)
                                              {
                                                  mountaindrainage[i][j]=0;
                                                  shelves[i][j]=0;
                                              }
                                          }
                                          
                                          // First, finish off the terrain generation.
                                          
                                          worldgenerationwindow->set_visible(1);
                                          
                                          screen->perform_layout();
                                          screen->redraw();
                                          screen->draw_all();
                                          
                                          int creationsteps=43; // Number of steps in the generation process to mark.
                                          float progressstep=1.0/creationsteps;
                                          worldprogress->set_value(0);
                                          
                                          generationlabel->set_caption("Raising mountain bases");
                                          worldprogress->set_value(worldprogress->value()+progressstep);
                                          screen->redraw();
                                          screen->draw_all();
                                          
                                          raisemountainbases(*world,mountaindrainage);
                                          
                                          generationlabel->set_caption("Filling depressions");
                                          worldprogress->set_value(worldprogress->value()+progressstep);
                                          screen->redraw();
                                          screen->draw_all();
                                          
                                          depressionfill(*world);
                                          
                                          addlandnoise(*world); // Add a bit of noise, then do remove depressions again. This is to add variety to the river courses.
                                          
                                          depressionfill(*world);
                                          
                                          generationlabel->set_caption("Adjusting coastlines");
                                          worldprogress->set_value(worldprogress->value()+progressstep);
                                          screen->redraw();
                                          screen->draw_all();
                                          
                                          for (int n=1; n<=2; n++)
                                              normalisecoasts(*world,13,11,4);
                                          
                                          clamp(*world);
                                          
                                          generationlabel->set_caption("Checking islands");
                                          worldprogress->set_value(worldprogress->value()+progressstep);
                                          screen->redraw();
                                          screen->draw_all();
                                          
                                          checkislands(*world);
                                          
                                          generationlabel->set_caption("Creating roughness map");
                                          worldprogress->set_value(worldprogress->value()+progressstep);
                                          screen->redraw();
                                          screen->draw_all();
                                          
                                          vector<vector<int>> roughness(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
                                          
                                          int grain=8; // Level of detail on this fractal map.
                                          float valuemod=0.2;
                                          float valuemod2=0.6;
                                          
                                          createfractal(roughness,width,height,grain,valuemod,valuemod2,1,maxelev,0,0);
                                          
                                          for (int i=0; i<=width; i++)
                                          {
                                              for (int j=0; j<=height; j++)
                                                  world->setroughness(i,j,roughness[i][j]);
                                          }
                                          
                                          // Now do the climates.
                                          
                                          generateglobalclimate(*world,*screen,*worldgenerationwindow,*generationlabel,*worldprogress,progressstep,smalllake,largelake,landshape,mountaindrainage,shelves);
                                          
                                          
                                          // Now just redo the map and go to the main world window.
                                          
                                          for (int n=0; n<GLOBALMAPTYPES; n++)
                                              globalmapimagecreated[n]=0;
                                          drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globalwindsimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                          
                                          globalmap->upload(globalreliefimage);
                                          
                                          globalmapwidget->set_image(globalmap);
                                          
                                          worldgenerationwindow->set_visible(0);
                                          
                                          globalmapwindow->set_title("Custom world");
                                          globalmapwindow->set_visible(1);
                                          globalmapwindow->request_focus();
                                          
                                      });
    
    // Now just set it all going.
    
    screen->set_visible(true);
    screen->perform_layout();
    
    nanogui::mainloop(-1);
    
    // Quit the app.
    
    delete world;
    
    nanogui::shutdown();
    return 0;
}

// This function prepares the minimap in the regional map screen.

void setregionalminimap(planet &world, region &region, stbi_uc globalreliefimage[], nanogui::Texture &regionalminimap, int globalimagewidth, int globalimageheight, int globalimagechannels)
{
    int width=world.width();
    
    int patchwidth=region.tilewidth()-2;
    int patchheight=region.tileheight()-2;
    
    int halfwidth=patchwidth/2;
    int halfheight=patchheight/2;
    
    int centrex=region.centrex();
    int centrey=region.centrey();
    
    int leftx=region.leftx();
    int lefty=region.lefty();
    
    int highlight1=world.highlight1();
    int highlight2=world.highlight2();
    int highlight3=world.highlight3();
    
    int index;
    
    // Create some dummy arrays to hold copies of the lines we'll be drawing over.
#if 1
    vector<stbi_uc> northsider(patchwidth);
    vector<stbi_uc> northsideg(patchwidth);
    vector<stbi_uc> northsideb(patchwidth);

    vector<stbi_uc> southsider(patchwidth);
    vector<stbi_uc> southsideg(patchwidth);
    vector<stbi_uc> southsideb(patchwidth);

    vector<stbi_uc> eastsider(patchwidth);
    vector<stbi_uc> eastsideg(patchwidth);
    vector<stbi_uc> eastsideb(patchwidth);

    vector<stbi_uc> westsider(patchwidth);
    vector<stbi_uc> westsideg(patchwidth);
    vector<stbi_uc> westsideb(patchwidth);
#else
    stbi_uc northsider[patchwidth];
    stbi_uc northsideg[patchwidth];
    stbi_uc northsideb[patchwidth];
    
    stbi_uc southsider[patchwidth];
    stbi_uc southsideg[patchwidth];
    stbi_uc southsideb[patchwidth];
    
    stbi_uc eastsider[patchheight];
    stbi_uc eastsideg[patchheight];
    stbi_uc eastsideb[patchheight];
    
    stbi_uc westsider[patchheight];
    stbi_uc westsideg[patchheight];
    stbi_uc westsideb[patchheight];
#endif
    
    // Now, draw our marker, copying all altered pixels onto the copy of the image.
    
    for (int i=0; i<patchwidth; i++)
    {
        int ii=centrex-halfwidth+i;
        
        if (ii<0 || ii>width)
            ii=wrap(ii,width);
        
        int j=centrey-halfheight;
        
        index=(ii+j*globalimagewidth)*globalimagechannels;
        
        northsider[i]=globalreliefimage[index];
        northsideg[i]=globalreliefimage[index+1];
        northsideb[i]=globalreliefimage[index+2];
        
        globalreliefimage[index]=highlight1;
        globalreliefimage[index+1]=highlight2;
        globalreliefimage[index+2]=highlight3;
        
        j=centrey+halfheight;
        
        index=(ii+j*globalimagewidth)*globalimagechannels;
        
        southsider[i]=globalreliefimage[index];
        southsideg[i]=globalreliefimage[index+1];
        southsideb[i]=globalreliefimage[index+2];
        
        globalreliefimage[index]=highlight1;
        globalreliefimage[index+1]=highlight2;
        globalreliefimage[index+2]=highlight3;
    }
    
    for (int j=0; j<patchheight; j++)
    {
        int jj=centrey-halfheight+j;
        
        int i=centrex-halfwidth;
        
        if (i<0 || i>width)
            i=wrap(i,width);
        
        index=(i+jj*globalimagewidth)*globalimagechannels;
        
        westsider[j]=globalreliefimage[index];
        westsideg[j]=globalreliefimage[index+1];
        westsideb[j]=globalreliefimage[index+2];
        
        globalreliefimage[index]=highlight1;
        globalreliefimage[index+1]=highlight2;
        globalreliefimage[index+2]=highlight3;
        
        i=centrex+halfwidth;
        
        if (i<0 || i>width)
            i=wrap(i,width);

        index=(i+jj*globalimagewidth)*globalimagechannels;
        
        eastsider[j]=globalreliefimage[index];
        eastsideg[j]=globalreliefimage[index+1];
        eastsideb[j]=globalreliefimage[index+2];
        
        globalreliefimage[index]=highlight1;
        globalreliefimage[index+1]=highlight2;
        globalreliefimage[index+2]=highlight3;
    }
    
    int extrax=centrex+halfwidth; // One extra corner to get!
    
    if (extrax>width)
        extrax=wrap(extrax,width);
    
    int extray=centrey+halfheight;
    
    index=(extrax+extray*globalimagewidth)*globalimagechannels;
    
    int extrapixelr=globalreliefimage[index];
    int extrapixelg=globalreliefimage[index+1];
    int extrapixelb=globalreliefimage[index+2];
    
    globalreliefimage[index]=highlight1;
    globalreliefimage[index+1]=highlight2;
    globalreliefimage[index+2]=highlight3;
    
    // Now apply the modified image to the minimap.
    
    regionalminimap.upload(globalreliefimage);

    // Now restore the changed pixels.

    for (int i=0; i<patchwidth; i++)
    {
        int ii=centrex-halfwidth+i;
        
        if (ii<0 || ii>width)
            ii=wrap(ii,width);
        
        int j=centrey-halfheight;
        
        index=(ii+j*globalimagewidth)*globalimagechannels;
        
        globalreliefimage[index]=northsider[i];
        globalreliefimage[index+1]=northsideg[i];
        globalreliefimage[index+2]=northsideb[i];
        
        j=centrey+halfheight;
        
        index=(ii+j*globalimagewidth)*globalimagechannels;
        
        globalreliefimage[index]=southsider[i];
        globalreliefimage[index+1]=southsideg[i];
        globalreliefimage[index+2]=southsideb[i];
    }
    
    for (int j=0; j<patchheight; j++)
    {
        int jj=centrey-halfheight+j;
        
        int i=centrex-halfwidth;
        
        if (i<0 || i>width)
            i=wrap(i,width);
        
        if (j>0)
        {
            index=(i+jj*globalimagewidth)*globalimagechannels;

            globalreliefimage[index]=westsider[j];
            globalreliefimage[index+1]=westsideg[j];
            globalreliefimage[index+2]=westsideb[j];
        }

        i=centrex+halfwidth;
        
        if (i<0 || i>width)
            i=wrap(i,width);
        
        index=(i+jj*globalimagewidth)*globalimagechannels;
        
        globalreliefimage[index]=eastsider[j];
        globalreliefimage[index+1]=eastsideg[j];
        globalreliefimage[index+2]=eastsideb[j];
    }
    
    index=(extrax+extray*globalimagewidth)*globalimagechannels;
    
    globalreliefimage[index]=extrapixelr;
    globalreliefimage[index+1]=extrapixelg;
    globalreliefimage[index+2]=extrapixelb;
}

// This function is for saving the world.

void saveworld(planet &world)
{
    string filename=nanogui::file_dialog({{"udw", "World file"}},1);
    
    if (filename=="")
        return;
    
    ofstream outfile;
    outfile.open(filename, ios::out);
    
    outfile.write((char*)&world,sizeof(world));
    
    outfile.close();
}

// This function is for loading the world.

bool loadworld(planet &world)
{
    string filename=nanogui::file_dialog({{"udw", "World file"}},0);
    
    if (filename=="")
        return 0;
    
    ifstream infile;
    infile.open(filename, ios::in);
    
    infile.read((char*)&world,sizeof(world));
    
    infile.close();
    
    return 1;
}

// This function saves settings.

void savesettings(planet &world)
{
    string filename=nanogui::file_dialog({{"uws", "Settings file"}},1);
    
    if (filename=="")
        return;
    
    ofstream outfile;
    outfile.open(filename, ios::out);
    
    outfile << world.landshading() << endl;
    outfile << world.lakeshading() << endl;
    outfile << world.seashading() << endl;
    outfile << world.shadingdir() << endl;
    outfile << world.snowchange() << endl;
    outfile << world.seaiceappearance() << endl;
    outfile << world.landmarbling() << endl;
    outfile << world.lakemarbling() << endl;
    outfile << world.seamarbling() << endl;
    outfile << world.minriverflowglobal() << endl;
    outfile << world.minriverflowregional() << endl;
    outfile << world.seaice1() << endl;
    outfile << world.seaice2() << endl;
    outfile << world.seaice3() << endl;
    outfile << world.ocean1() << endl;
    outfile << world.ocean2() << endl;
    outfile << world.ocean3() << endl;
    outfile << world.deepocean1() << endl;
    outfile << world.deepocean2() << endl;
    outfile << world.deepocean3() << endl;
    outfile << world.base1() << endl;
    outfile << world.base2() << endl;
    outfile << world.base3() << endl;
    outfile << world.basetemp1() << endl;
    outfile << world.basetemp2() << endl;
    outfile << world.basetemp3() << endl;
    outfile << world.highbase1() << endl;
    outfile << world.highbase2() << endl;
    outfile << world.highbase3() << endl;
    outfile << world.desert1() << endl;
    outfile << world.desert2() << endl;
    outfile << world.desert3() << endl;
    outfile << world.highdesert1() << endl;
    outfile << world.highdesert2() << endl;
    outfile << world.highdesert3() << endl;
    outfile << world.colddesert1() << endl;
    outfile << world.colddesert2() << endl;
    outfile << world.colddesert3() << endl;
    outfile << world.grass1() << endl;
    outfile << world.grass2() << endl;
    outfile << world.grass3() << endl;
    outfile << world.cold1() << endl;
    outfile << world.cold2() << endl;
    outfile << world.cold3() << endl;
    outfile << world.tundra1() << endl;
    outfile << world.tundra2() << endl;
    outfile << world.tundra3() << endl;
    outfile << world.eqtundra1() << endl;
    outfile << world.eqtundra2() << endl;
    outfile << world.eqtundra3() << endl;
    outfile << world.saltpan1() << endl;
    outfile << world.saltpan2() << endl;
    outfile << world.saltpan3() << endl;
    outfile << world.erg1() << endl;
    outfile << world.erg2() << endl;
    outfile << world.erg3() << endl;
    outfile << world.wetlands1() << endl;
    outfile << world.wetlands2() << endl;
    outfile << world.wetlands3() << endl;
    outfile << world.lake1() << endl;
    outfile << world.lake2() << endl;
    outfile << world.lake3() << endl;
    outfile << world.river1() << endl;
    outfile << world.river2() << endl;
    outfile << world.river3() << endl;
    outfile << world.glacier1() << endl;
    outfile << world.glacier2() << endl;
    outfile << world.glacier3() << endl;
    outfile << world.highlight1() << endl;
    outfile << world.highlight2() << endl;
    outfile << world.highlight3() << endl;
    
    outfile.close();
}

// This function loads settings.

bool loadsettings(planet &world)
{
    string filename=nanogui::file_dialog({{"uws", "Settings file"}},0);
    
    if (filename=="")
        return 0;
    
    ifstream infile;
    infile.open(filename, ios::in);
    
    string line;
    int val;
    float fval;
    
    getline(infile,line);
    fval=stof(line);
    world.setlandshading(fval);
    
    getline(infile,line);
    fval=stof(line);
    world.setlakeshading(fval);
    
    getline(infile,line);
    fval=stof(line);
    world.setseashading(fval);
    
    getline(infile,line);
    val=stoi(line);
    world.setshadingdir(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setsnowchange(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setseaiceappearance(val);
    
    getline(infile,line);
    fval=stof(line);
    world.setlandmarbling(fval);
    
    getline(infile,line);
    fval=stof(line);
    world.setlakemarbling(fval);
    
    getline(infile,line);
    fval=stof(line);
    world.setseamarbling(fval);
    
    getline(infile,line);
    val=stoi(line);
    world.setminriverflowglobal(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setminriverflowregional(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setseaice1(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setseaice2(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setseaice3(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setocean1(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setocean2(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setocean3(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setdeepocean1(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setdeepocean2(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setdeepocean3(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setbase1(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setbase2(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setbase3(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setbasetemp1(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setbasetemp2(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setbasetemp3(val);
    
    getline(infile,line);
    val=stoi(line);
    world.sethighbase1(val);
    
    getline(infile,line);
    val=stoi(line);
    world.sethighbase2(val);
    
    getline(infile,line);
    val=stoi(line);
    world.sethighbase3(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setdesert1(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setdesert2(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setdesert3(val);
    
    getline(infile,line);
    val=stoi(line);
    world.sethighdesert1(val);
    
    getline(infile,line);
    val=stoi(line);
    world.sethighdesert2(val);
    
    getline(infile,line);
    val=stoi(line);
    world.sethighdesert3(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setcolddesert1(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setcolddesert2(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setcolddesert3(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setgrass1(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setgrass2(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setgrass3(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setcold1(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setcold2(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setcold3(val);
    
    getline(infile,line);
    val=stoi(line);
    world.settundra1(val);
    
    getline(infile,line);
    val=stoi(line);
    world.settundra2(val);
    
    getline(infile,line);
    val=stoi(line);
    world.settundra3(val);
    
    getline(infile,line);
    val=stoi(line);
    world.seteqtundra1(val);
    
    getline(infile,line);
    val=stoi(line);
    world.seteqtundra2(val);
    
    getline(infile,line);
    val=stoi(line);
    world.seteqtundra3(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setsaltpan1(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setsaltpan2(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setsaltpan3(val);
    
    getline(infile,line);
    val=stoi(line);
    world.seterg1(val);
    
    getline(infile,line);
    val=stoi(line);
    world.seterg2(val);
    
    getline(infile,line);
    val=stoi(line);
    world.seterg3(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setwetlands1(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setwetlands2(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setwetlands3(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setlake1(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setlake2(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setlake3(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setriver1(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setriver2(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setriver3(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setglacier1(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setglacier2(val);
    
    getline(infile,line);
    val=stoi(line);
    world.setglacier3(val);
    
    getline(infile,line);
    val=stoi(line);
    world.sethighlight1(val);
    
    getline(infile,line);
    val=stoi(line);
    world.sethighlight2(val);
    
    getline(infile,line);
    val=stoi(line);
    world.sethighlight3(val);
    
    infile.close();
    
    return 1;
}

// This function saves an image.

void saveimage(stbi_uc source[], int globalimagechannels, int width, int height, string filename)
{
    sf::Image mapimage;
    
    mapimage.create(width,height,sf::Color::Black);
    
    int index, colour1, colour2, colour3;
    
    for (int i=0; i<width; i++)
    {
        for (int j=0; j<height; j++)
        {
            index=(i+j*width)*globalimagechannels;
            
            colour1=source[index];
            colour2=source[index+1];
            colour3=source[index+2];
            
            mapimage.setPixel(i,j,sf::Color(colour1,colour2,colour3));
        }
    }
    mapimage.saveToFile(filename);
}

// This function creates a rift blob template.

void createriftblob(vector<vector<float>> &riftblob, int size)
{
    for (int i=0; i<=size*2; i++)
    {
        for (int j=0; j<=size*2; j++)
            riftblob[i][j]=0;
    }
    
    float startvalue=1.0; // The starting value for the centre of the circle.
    float endvalue=0.15; // The end value for the edge of the circle.
    float step=(startvalue-endvalue)/size; // Reduce the value by this much at each step of the circle drawing.
    
    float currentvalue=startvalue;
    
    int centrex=size+1;
    int centrey=size+1;
    
    for (int radius=1; radius<=size; radius++)
    {
        for (int i=-radius; i<=radius; i++)
        {
            int ii=centrex+i;
            
            for (int j=-radius; j<=radius; j++)
            {
                int jj=centrey+j;
                
                if (riftblob[ii][jj]==0 && i*i+j*j<radius*radius+radius)
                    riftblob[ii][jj]=currentvalue;
            }
        }
        currentvalue=currentvalue-step;
    }
}

// This function returns a random number from a to b inclusive.

inline int random(int a, int b)
{
    int range=(b-a)+1; // This is the range of possible numbers.
    
    int r=a+(fast_rand()%range);
    
    return r;
}

// This function randomises the sign of an integer (positive or negative).

inline int randomsign(int a)
{
    if (random(0,1)==1)
        a=0-a;
    
    return (a);
}

// These do the same thing but with the inbuilt randomiser.

inline int altrandom(int a, int b)
{
    int range=(b-a)+1; // This is the range of possible numbers.
    
    int r=a+(rand()%range);
    
    return r;
}

inline int altrandomsign(int a)
{
    if (altrandom(0,1)==1)
        a=0-a;
    
    return (a);
}

// This function converts an integer into a binary string, 32 digits long. From here: https://stackoverflow.com/questions/8222127/changing-integer-to-binary-string-of-digits

string decimaltobinarystring(int a)
{
    uint b = (uint)a;
    string binary = "";
    uint mask = 0x80000000u;
    while (mask > 0)
    {
        binary += ((b & mask) == 0) ? '0' : '1';
        mask >>= 1;
    }
    return binary;
}

// This function wraps a value between 0 and a maximum.

int wrap(int value, int max)
{
    max++;
    
    value=value%max;
    
    if (value<0)
        value=max+value;
    
    return value ;
}

// This function finds the average of two values, wrapped around a maximum.

int wrappedaverage(int x, int y, int max)
{
    int result=-1;
    
    int diffa=abs(x-y);
    
    int diffb=abs((x-max)-y);
    
    int diffc=abs(x-(y-max));
    
    if (diffa<=diffb && diffa<=diffc)
        result=(x+y)/2;
    else
    {
        if (diffb<=diffa && diffb<=diffc)
            result=((x-max)+y)/2;
        else
            result=(x+(y-max))/2;
    }
    
    if (result<0)
        result=result+max;
    
    if (result>max)
        result=result-max;
        
    return result;
}

// This normalises one value to another, so they are as close as possible, given the wrapped maximum.

int normalise(int x, int y, int max)
{
    while (abs(x-y)>abs(x-(y-max)))
        y=y-max;
    
    while (abs(x-y)>abs(x-(y+max)))
        y=y+max;
    
    return y;
}

// This function takes two values (a and b) and creates a new value that is tilted towards a by percentage %.

int tilt(int a, int b, int percentage)
{
    int neg=0;
    int tilt=0;
    
    float diff=a-b;
    
    if (diff<1)
    {
        diff=0-diff;
        neg=1;
    }
    
    if (diff!=0)
        tilt=(diff/100)*percentage;
    
    if (neg==1)
        tilt=0-tilt;
    
    int newval=b+tilt;
    
    return(newval);
}

// This function shifts everything in a vector to the left by a given number of pixels.

void shift(vector<vector<int>> &map, int width, int height, int offset)
{
    vector<vector<int>> dummy(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
            dummy[i][j]=map[i][j];
    }
    
    for (int i=0; i<=width; i++)
    {
        int ii=i+offset;
        
        if (ii<0 || ii>width)
            ii=wrap(ii,width);
        
        for (int j=0; j<=height; j++)
            map[i][j]=dummy[ii][j];
    }
    
    // For some reason this leaves a vertical line of noise, so we remove that manually now.
    
    int i=width-offset; // This is the x-coordinate of the problematic line.
    int iminus=i-1;
    int iplus=i+1;
    
    if (iminus<0)
        iminus=width;
    
    if (iplus>width)
        iplus=0;
    
    for (int j=0; j<=height; j++) // Make the line the average of its neighbours to left and right.
    {
        map[i][j]=(map[iminus][j]+map[iplus][j])/2;
    }
}

// This function flips an array, vertically/horizontally.

void flip(vector<vector<int>> &arr, int awidth, int aheight, bool vert, bool horiz)
{
    vector<vector<int>> dummy(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
    
    for (int i=0; i<=awidth; i++)
    {
        for (int j=0; j<=aheight; j++)
            dummy[i][j]=arr[i][j];
    }
    
    for (int i=0; i<=awidth; i++)
    {
        for (int j=0; j<=aheight; j++)
        {
            int ii, jj;
            
            if (horiz==0)
                ii=1;
            else
                ii=awidth-i;
            
            if (vert==0)
                jj=j;
            else
                jj=aheight-j;
            
            arr[i][j]=dummy[ii][jj];
        }
    }
}

// This function smoothes a vector.

void smooth(vector<vector<int>> &arr, int width, int height, int maxelev, int amount, bool vary)
{
    int varyx=width/2;
    int varyy=random(1,height-1);
    
    for (int i=0; i<=width; i++)
    {
        for (int j=1; j<height; j++)
        {
            bool goaheadx=0;
            bool goaheady=0;
            
            if (vary==1) // This will vary the amount of smoothing over the map, to make it more interesting.
            {
                if (i<varyx)
                {
                    if (i>10 && random(0,varyx)<i*2)
                        goaheadx=1;
                }
                else
                {
                    if (i<width-10 && random(varyx,width)>i/2)
                        goaheadx=1;
                }
                
                if (j<varyy)
                {
                    if (random(0,varyy)<j*2)
                        goaheady=1;
                }
                else
                {
                    if (random(varyy,height)>j/2)
                        goaheady=1;
                }
                if (abs(varyx-i)<100) // Force it to smooth in the centre of the map.
                {
                    //goaheadx=1;
                    //goaheady=1;
                }
            }
            else
            {
                goaheadx=1;
                goaheady=1;
            }
            
            if (goaheadx==1 && goaheady==1)
            {
                int crount=0;
                int ave=0;
                
                for (int k=i-amount; k<=i+amount; k++)
                {
                    int kk=k;
                    
                    if (kk<0)
                        kk=width;
                    
                    if (kk>width)
                        kk=0;
                    
                    for (int l=j-amount; l<=j+amount; l++)
                    {
                        //ave=ave+nomwrap(k,l);
                        ave=ave+arr[kk][l];
                        crount++;
                    }
                }
                
                ave=ave/crount;
                
                if (ave>0 && ave<maxelev)
                    arr[i][j]=ave;
            }
        }
    }
}

// This function sees whether the given point on the given vector is on the edge of the filled area.
// Note, this wraps around the edges.

bool edge(vector<vector<bool>> &arr, int width, int height, int i, int j)
{
    if (arr[i][j]==0)
        return 0;
    
    for (int k=i-1; k<=i+1; k++)
    {
        int kk=k;
        
        if (kk<0 || kk>width)
            kk=wrap(kk,width);
        
        for (int l=j-1; l<=j+1; l++)
        {
            if (l>=0 && l<=height)
            {
                if (arr[kk][l]==0)
                    return 1;
            }
        }
    }
    
    return 0;
}

// This draws a straight line on the supplied vector.
// Uses Bresenham's line algorithm - http://rosettacode.org/wiki/Bitmap/Bresenham%27s_line_algorithm#C.2B.2B

void drawline(vector<vector<bool>> &arr, int x1, int y1, int x2, int y2)
{
    const bool steep = (fabs(y2 - y1) > fabs(x2 - x1));
    if(steep)
    {
        std::swap(x1, y1);
        std::swap(x2, y2);
    }
    
    if(x1 > x2)
    {
        std::swap(x1, x2);
        std::swap(y1, y2);
    }
    
    const float dx = x2 - x1;
    const float dy = fabs(y2 - y1);
    
    float error = dx / 2.0f;
    const int ystep = (y1 < y2) ? 1 : -1;
    int y = (int)y1;
    
    const int maxX = (int)x2;
    
    for(int x=(int)x1; x<=maxX; x++)
    {
        if(steep)
            arr[y][x]=1;
        else
            arr[x][y]=1;
        
        error -= dy;
        if(error < 0)
        {
            y += ystep;
            error += dx;
        }
    }
}

// This function draws a filled circle on the specified vector.
// Uses code by OBese87 and Libervurto - https://forum.thegamecreators.com/thread/197341

void drawcircle(vector<vector<int>> &arr, int x, int y, int col, int radius)
{
    float dots=radius*6.28;
    float t=360.0/dots;
    int quarter=dots/4;
    
    for (int i=1; i<=quarter; i++)
    {
        int u=sin(i*t)*radius;
        int v=cos(i*t)*radius;
        
        box(arr,x-u,y-v,x+u,y+v,col);
    }
}

// This function draws a filled box on the specified vector.

void box(vector<vector<int>> &arr, int x1, int y1, int x2, int y2, int col)
{
    for (int i=x1; i<=x2; i++)
    {
        int ii=i;
        
        if (ii<0 || i>ARRAYWIDTH-1)
            ii=wrap(ii,ARRAYWIDTH-1);
        
        for (int j=y1; j<=y2; j++)
        {
            if (j>=0 && j<ARRAYHEIGHT)
                arr[ii][j]=col;
        }
    }
}

// As above, but on a three-dimensional vector.

void drawcircle3d(vector<vector<vector<int>>> &arr, int x, int y, int col, int radius, int index)
{
    float dots=radius*6.28;
    float t=360.0/dots;
    int quarter=dots/4;
    
    for (int i=1; i<=quarter; i++)
    {
        int u=sin(i*t)*radius;
        int v=cos(i*t)*radius;
        
        box3d(arr,x-u,y-v,x+u,y+v,col,index);
    }
}

// Again, on a three-dimensional vector.

void box3d(vector<vector<vector<int>>> &arr, int x1, int y1, int x2, int y2, int col, int index)
{
    for (int i=x1; i<=x2; i++)
    {
        for (int j=y1; j<=y2; j++)
        {
            if (i>=0 && i<=20 && j>=0 && j<=20)
                arr[index][i][j]=col;
        }
    }
}

// This function draws splines. Based on functions by Markus - https://forum.thegamecreators.com/thread/202580
// (This is easily the most important bit of borrowed code in this project. Thank you Markus!)

sf::Vector2f curvepos(sf::Vector2f p0, sf::Vector2f p1, sf::Vector2f p2, sf::Vector2f p3, float t)
{
    sf::Vector2f pt;
    
    pt.x = 0.5 * ( (2.0 * p1.x) + (-p0.x + p2.x) * t + (2.0*p0.x - 5.0*p1.x + 4.0*p2.x - p3.x) * pow(t,2.0) + (-p0.x + 3.0*p1.x- 3.0*p2.x + p3.x) * pow(t,3.0) );
    pt.y = 0.5 * ( (2.0 * p1.y) + (-p0.y + p2.y) * t + (2.0*p0.y - 5.0*p1.y + 4.0*p2.y - p3.y) * pow(t,2.0) + (-p0.y + 3.0*p1.y- 3.0*p2.y + p3.y) * pow(t,3.0) );
    
    return (pt);
}

// This function does a flood fill on a bool vector.

void fill (vector<vector<bool>> &arr, int width, int height, int startx, int starty, int replacement)
{
    if (startx<0 || startx>=width || starty<0 || starty>=height)
        return;
    
    if (arr[startx][starty]==replacement)
        return;
    
    int row[]={-1,0,0,1};
    int col[]={0,-1,1,0};
    
    bool target=arr[startx][starty]; // This is the "colour" that we're going to change into the replacement "colour".
    
    sf::Vector2i node;
    
    node.x=startx;
    node.y=starty;
    
    queue<sf::Vector2i> q; // Create a queue
    q.push(node); // Put our starting node into the queue
    
    while (q.empty()!=1) // Keep going while there's anything left in the queue
    {
        node=q.front(); // Take the node at the front of the queue
        q.pop(); // And then pop it out of the queue
        
        if (node.x>=0 && node.x<width && node.y>=0 && node.y<height && arr[node.x][node.y]==target)
        {
            arr[node.x][node.y]=replacement;
            
            for (int k=0; k<4; k++) // Look at the four neighbouring nodes in turn
            {
                sf::Vector2i nextnode;
                
                nextnode.x=node.x+row[k];
                nextnode.y=node.y+col[k];
                
                if (nextnode.x>=0 && nextnode.x<width && nextnode.y>=0 && nextnode.y<height)
                {
                    if (arr[nextnode.x][nextnode.y]==target) // If this node is the right "colour"
                        q.push(nextnode); // Put that node onto the queue
                }
            }
        }
    }
}

// This function does the same thing, but with a continental mask to avoid going into the sea.

void fillcontinent (vector<vector<bool>> &arr, vector<vector<short>> &mask, short maskcheck, int width, int height, int startx, int starty, int replacement)
{
    if (startx<0 || startx>=width || starty<0 || starty>=height)
        return;
    
    if (arr[startx][starty]==replacement)
        return;
    
    if (mask[startx][starty]!=maskcheck)
        return;
    
    int row[]={-1,0,0,1};
    int col[]={0,-1,1,0};
    
    bool target=arr[startx][starty]; // This is the "colour" that we're going to change into the replacement "colour".
    
    sf::Vector2i node;
    
    node.x=startx;
    node.y=starty;
    
    queue<sf::Vector2i> q; // Create a queue
    q.push(node); // Put our starting node into the queue
    
    while (q.empty()!=1) // Keep going while there's anything left in the queue
    {
        node=q.front(); // Take the node at the front of the queue
        q.pop(); // And then pop it out of the queue
        
        if (node.x>=0 && node.x<width && node.y>=0 && node.y<height && arr[node.x][node.y]==target)
        {
            arr[node.x][node.y]=replacement;
            
            for (int k=0; k<4; k++) // Look at the four neighbouring nodes in turn
            {
                sf::Vector2i nextnode;
                
                nextnode.x=node.x+row[k];
                nextnode.y=node.y+col[k];
                
                if (nextnode.x>=0 && nextnode.x<width && nextnode.y>=0 && nextnode.y<height)
                {
                    if (arr[nextnode.x][nextnode.y]==target && mask[nextnode.x][nextnode.y]==maskcheck) // If this node is the right "colour"
                        q.push(nextnode); // Put that node onto the queue
                }
            }
        }
    }
}

// This function adds the elevation element to a temperature.

int tempelevadd(planet &world, int temp, int i, int j)
{
    int elevation=world.map(i,j)-world.sealevel();
    
    if (elevation<0)
        elevation=0;
    
    float elevationadjust=elevation;
    elevationadjust=elevationadjust/1000.0;
    elevationadjust=elevationadjust*6.5;
    
    temp=temp-elevationadjust;
    
    return (temp);
}

// Same thing, but at the regional level.

int tempelevadd(region &region, int temp, int i, int j)
{
    int elevation=region.map(i,j)-region.sealevel();
    
    if (elevation<0)
        elevation=0;
    
    float elevationadjust=elevation;
    elevationadjust=elevationadjust/1000.0;
    elevationadjust=elevationadjust*6.5;
    
    temp=temp-elevationadjust;
    
    return (temp);
}

// This function removes the elevation element of a temperature.

int tempelevremove(planet &world, int temp, int i, int j)
{
    int elevation=world.map(i,j)-world.sealevel();
    
    if (elevation<0)
        elevation=0;
    
    float elevationadjust=elevation;
    elevationadjust=elevationadjust/1000.0;
    elevationadjust=elevationadjust*6.5;
    
    temp=temp+elevationadjust;
    
    return (temp);
}

// Same thing, but at the regional level.

int tempelevremove(region &region, int temp, int i, int j)
{
    int elevation=region.map(i,j)-region.sealevel();
    
    if (elevation<0)
        elevation=0;
    
    float elevationadjust=elevation;
    elevationadjust=elevationadjust/1000.0;
    elevationadjust=elevationadjust*6.5;
    
    temp=temp+elevationadjust;
    
    return (temp);
}

// This function translates a direction from an integer into a text string.

string getdirstring(int dir)
{
    string direction;
    
    switch (dir)
    {
        case 1:
            direction="north";
            break;
            
        case 2:
            direction="northeast";
            break;
            
        case 3:
            direction="east";
            break;
            
        case 4:
            direction="southeast";
            break;
            
        case 5:
            direction="south";
            break;
            
        case 6:
            direction="southwest";
            break;
            
        case 7:
            direction="west";
            break;
            
        case 8:
            direction="northwest";
            break;
    }
    
    return (direction);
}

// This function returns the direction from one tile to another.

int getdir(int x, int y, int xx, int yy)
{
    if (xx==x && yy<y)
        return (1);
    
    if (xx>x && yy<y)
        return (2);
    
    if (xx>x && yy==y)
        return (3);
    
    if (xx>x && yy>y)
        return (4);
    
    if (xx==x && yy>y)
        return (5);
    
    if (xx<x && yy>y)
        return (6);
    
    if (xx<x && yy==y)
        return (7);
    
    if (xx<x && yy<y)
        return (8);
    
    return (0);
}

// This function gets the destination from a tile and a direction.

sf::Vector2i getdestination(int x, int y, int dir)
{
    if (dir==8 || dir==1 || dir==2)
        y--;
    
    if (dir==4 || dir==5 || dir==6)
        y++;
    
    if (dir==2 || dir==3 || dir==4)
        x++;
    
    if (dir==6 || dir==7 || dir==8)
        x--;
    
    sf::Vector2i dest;
    
    dest.x=x;
    dest.y=y;
    
    return dest;
}

// This function finds the direction of the lowest neighbouring tile.

int findlowestdir(planet &world, int neighbours[8][2], int x, int y)
{
    int width=world.width();
    int height=world.height();
    
    int lowest=world.maxelevation()*2;
    int nnn=-1;
    
    int start=random(0,7);
    
    for (int n=start; n<=start+7; n++)
    {
        int nn=wrap(n,7);
        
        int i=x+neighbours[nn][0];
        
        if (i<0 || i>width)
            i=wrap(i,width);
        
        int j=y+neighbours[nn][1];
        
        if (j>=0 && j<=height)
        {
            int pointelev=world.nom(i,j);
            
            if (pointelev<lowest)
            {
                lowest=pointelev;
                nnn=nn;
            }
        }
    }
    
    int dir=nnn+1;
    
    return (dir);
}

// Same thing, but for river directions.

int findlowestdirriver(planet &world, int neighbours[8][2], int x, int y, vector<vector<int>> &mountaindrainage)
{
    int width=world.width();
    int height=world.height();
    
    int lowest=world.maxelevation()*2;
    int nnn=-1;
    
    bool found=0;
    
    int start=random(0,7);

    if (1==0)//random(1,3)!=1)
    {
        for (int n=start; n<=start+7; n++)
        {
            int nn=wrap(n,7);
            
            int i=x+neighbours[nn][0];
            
            if (i<0 || i>width)
                i=wrap(i,width);
            
            int j=y+neighbours[nn][1];
            
            if (j>=0 && j<=height )
            {
                int pointelev=mountaindrainage[i][j];
                
                if (pointelev<lowest && world.nom(i,j)<world.nom(x,y))
                {
                    lowest=pointelev;
                    nnn=nn;
                    found=1;
                }
            }
        }
    }
    
    if (found==0) // Couldn't find one just from the mountain drainage.
    {
        lowest=world.maxelevation()*2;
        nnn=-1;
        
        start=random(0,7);
        
        for (int n=start; n<=start+7; n++)
        {
            int nn=wrap(n,7);
            
            int i=x+neighbours[nn][0];
            
            if (i<0 || i>width)
                i=wrap(i,width);
            
            int j=y+neighbours[nn][1];
            
            if (j>=0 && j<=height)
            {
                int pointelev=world.nom(i,j);
                
                if (pointelev<lowest)
                {
                    lowest=pointelev;
                    nnn=nn;
                }
            }
        }
    }
    
    int dir=nnn+1;
    
    return (dir);
}

// This gets the amount the land is sloping between two points.

int getslope(planet &world, int x, int y, int xx, int yy)
{
    int width=world.width();
    int height=world.height();
    
    if (x<0 || x>width)
        x=wrap(x,width);
    
    if (xx<0 || xx>width)
        xx=wrap(xx,width);
    
    if (y<0 || y>height)
        return (-1);
    
    if (yy<0 || yy>height)
        return (-1);
    
    int slope=world.map(xx,yy)-world.map(x,y);
    
    return (slope);
}

// Same thing on the regional map.

int getslope(region &region, int x, int y, int xx, int yy)
{
    int rwidth=region.rwidth();
    int rheight=region.rheight();
    
    if (x<0 || x>rwidth)
        return (-1);
    
    if (xx<0 || xx>rwidth)
        return (-1);
    
    if (y<0 || y>rheight)
        return (-1);
    
    if (yy<0 || yy>rheight)
        return (-1);
    
    int slope=region.map(xx,yy)-region.map(x,y);
    
    return (slope);
}

// This function checks how flat the land is at a given point.

int getflatness(planet &world, int x, int y)
{
    int width=world.width();
    int height=world.height();
    
    int xminus=x-1;
    
    if (xminus<0)
        xminus=width;
    
    int xplus=x+1;
    
    if (xplus>width)
        xplus=0;
    
    int thiselev=getflatelevation(world,x,y);
    
    float flatness=0;
    
    int thatelev=getflatelevation(world,xminus,y);
    flatness=flatness+abs(thiselev-thatelev);
    
    float crount=2;
    
    if (y>0)
    {
        thatelev=getflatelevation(world,xminus,y-1);
        flatness=flatness+abs(thiselev-thatelev);
        
        thatelev=getflatelevation(world,x,y-1);
        flatness=flatness+abs(thiselev-thatelev);
        
        thatelev=getflatelevation(world,xplus,y-1);
        flatness=flatness+abs(thiselev-thatelev);
        
        crount=crount+3;
    }
    
    if (y<height)
    {
        thatelev=getflatelevation(world,xminus,y+1);
        flatness=flatness+abs(thiselev-thatelev);
        
        thatelev=getflatelevation(world,x,y+1);
        flatness=flatness+abs(thiselev-thatelev);
        
        thatelev=getflatelevation(world,xplus,y+1);
        flatness=flatness+abs(thiselev-thatelev);
        
        crount=crount+3;
    }
    
    flatness=flatness/crount;
    
    int iflatness=flatness;
    
    return (iflatness);
}

// This function gets the elevation of a point, incorporating water level.

int getflatelevation(planet &world, int x, int y)
{
    int elev;
    
    int surface=world.lakesurface(x,y);
    
    if (surface>0)
        elev=surface;
    else
    {
        if (world.sea(x,y)==1)
            elev=world.sealevel();
        else
            elev=world.nom(x,y);
    }
    
    return (elev);
}

// This finds the distance to the nearest land in the direction of the equator.

int landdistance(planet &world, int x, int y)
{
    int width=world.width();
    int height=world.height();
    int equator=height/2;
    
    if (x<0 || x>width || y<0 || y>height)
        return (-1);
    
    int dist=0;
    int pets=-1;
    
    if (y<equator)
        pets=1;
    
    for (int j=y; j!=equator; j=j+pets)
    {
        if (world.sea(x,j)==0)
            return (dist);
        
        dist++;
    }
    
    dist=-1; // No land found at all.
    
    return(dist);
}

// This finds the coordinates of the nearest sea (or land, if land=1) to the point specified. Note that it does not wrap the x coordinate, i.e. it could return one lower than 0 or higher than width.

sf::Vector2i nearestsea(planet &world, int i, int j, bool land, int limit, int grain)
{
    int width=world.width();
    int height=world.height();
    
    sf::Vector2i seapoint(-1,-1);
    
    int distance=-1; // Distance to nearest sea.
    int seax=-1; // x coordinate of nearest sea.
    int seay=-1; // y coordinate of nearest sea.
    int checkdist=1; // Distance that we're checking to see if it contains sea.
    
    while (distance==-1)
    {
        checkdist=checkdist+grain; // Increase the size of circle that we're checking
        
        int rr=pow(checkdist,2);
        
        for (int x=i-checkdist; x<=i+checkdist; x++)
        {
            int xx=x;
            
            if (xx<0 || xx>width)
                xx=wrap(xx,width);
            
            for (int y=j-checkdist; y<=j+checkdist; y++)
            {
                if (y>=0 && y<=height)
                {
                    if (pow((x-i),2)+pow((y-j),2)==rr) // if this point is on the circle we're looking at
                    {
                        if (world.sea(xx,y)==1 && land==0) // And if this point is sea
                        {
                            distance=checkdist;
                            seax=x;
                            seay=y;
                        }
                        
                        if (world.sea(xx,y)==0 && land==1) // And if this point is land
                        {
                            distance=checkdist;
                            seax=x;
                            seay=y;
                        }
                    }
                }
            }
        }
        if (checkdist>limit) // We are surely not going to find any now...
        {
            distance=width*2;
            seax=-1;
            seay=-1;
        }
    }
    
    seapoint.x=seax;
    seapoint.y=seay;
    
    return (seapoint);
}

// This does the same thing, but for the regional map.

sf::Vector2i nearestsea(region &region, int leftx, int lefty, int rightx, int righty, int i, int j)
{
    sf::Vector2i seapoint(-1,-1);
    
    int distance=-1; // Distance to nearest sea.
    int seax=-1; // x coordinate of nearest sea.
    int seay=-1; // y coordinate of nearest sea.
    int checkdist=1; // Distance that we're checking to see if it contains sea.
    
    while (distance==-1)
    {
        checkdist++; // Increase the size of circle that we're checking
        
        int rr=pow(checkdist,2);
        
        for (int x=i-checkdist; x<=i+checkdist; x++)
        {
            for (int y=j-checkdist; y<=j+checkdist; y++)
            {
                if (pow((x-i),2)+pow((y-j),2)==rr) // if this point is on the circle we're looking at
                {
                    if (x>=leftx && x<=rightx && y>=lefty && y<=righty)
                    {
                        if (region.sea(x,y)==1) // And if this point is sea
                        {
                            distance=checkdist;
                            seax=x;
                            seay=y;
                        }
                        else
                        {
                            if (region.lakesurface(x,y)!=0) // Lake counts as sea for our purposes.
                            {
                                distance=checkdist;
                                seax=x;
                                seay=y;
                            }
                        }
                    }
                }
            }
        }
        if (checkdist>100) // We are surely not going to find any now...
        {
            distance=region.rwidth()*2;
            seax=-1;
            seay=-1;
        }
    }
    
    seapoint.x=seax;
    seapoint.y=seay;
    
    return seapoint;
}

// This function checks whether a global tile is either sea next to land or land next to sea.

bool vaguelycoastal(planet &world, int x, int y)
{
    int width=world.width();
    int height=world.height();
    
    bool sea=0;
    
    if (world.sea(x,y)==1)
        sea=1;
    
    bool found=0;
    
    for (int i=x-1; i<=x+1; i++)
    {
        int ii=i;
        
        if (ii<0 || ii>width)
            ii=wrap(ii,width);
        
        for (int j=y-1; j<=y+1; j++)
        {
            if (j>=0 && j<=height)
            {
                if (sea==0 && world.sea(ii,j)==1)
                {
                    found=1;
                    i=x+1;
                    j=y+1;
                }
                
                if (sea==1 && world.sea(ii,j)==0)
                {
                    found=1;
                    i=x+1;
                    j=y+1;
                }
            }
        }
    }
    
    return found;
}

// Same thing, for a regional cell.

bool vaguelycoastal(region &region, int x, int y)
{
    int rwidth=region.rwidth();
    int rheight=region.rheight();
    
    bool sea=0;
    
    if (region.sea(x,y)==1)
        sea=1;
    
    bool found=0;
    
    for (int i=x-1; i<=x+1; i++)
    {
        for (int j=y-1; j<=y+1; j++)
        {
            if (i>=0 && i<=rwidth && j>=0 && j<=rheight)
            {
                if (sea==0 && region.sea(i,j)==1)
                {
                    found=1;
                    i=x+1;
                    j=y+1;
                }
                
                if (sea==1 && region.sea(i,j)==0)
                {
                    found=1;
                    i=x+1;
                    j=y+1;
                }
            }
        }
    }
    
    return found;
}

// This function checks to see if a global tile is land, bordered by sea to the east, south, and southeast.

bool northwestlandonly(planet &world, int x, int y)
{
    int width=world.width();
    int height=world.height();
    
    if (world.sea(x,y)==1)
        return 0;
    
    int xx=x+1;
    
    if (xx>width)
        xx=0;
    
    int yy=y+1;
    
    if (yy>height)
        return 0;
    
    if (world.sea(xx,y)==0)
        return 0;
    
    if (world.sea(x,yy)==0)
        return 0;
    
    if (world.sea(xx,yy)==0)
        return 0;
    
    xx=x-1;
    
    if (xx<0)
        x=width;
    
    yy=y-1;
    
    if (yy<0)
        return 0;
    
    if (world.sea(xx,yy)==1)
        return 0;

    return 1;
}

// Same thing, at the regional level.

bool northwestlandonly(region &region, int x, int y)
{
    int rwidth=region.rwidth();
    int rheight=region.rheight();
    
    if (region.sea(x,y)==1)
        return 0;
    
    int xx=x+1;
    
    if (xx>rwidth)
        return 0;
    
    int yy=y+1;
    
    if (yy>rheight)
        return 0;
    
    if (region.sea(xx,y)==0)
        return 0;
    
    if (region.sea(x,yy)==0)
        return 0;
    
    if (region.sea(xx,yy)==0)
        return 0;
    
    xx=x-1;
    
    if (xx<0)
        return 0;
    
    yy=y-1;
    
    if (yy<0)
        return 0;
    
    if (region.sea(xx,yy)==1)
        return 0;
    
    return 1;
}

// Same again, but for lakes rather than sea.

bool lakenorthwestlandonly(region &region, int x, int y)
{
    int rwidth=region.rwidth();
    int rheight=region.rheight();
    
    if (region.lakesurface(x,y)!=0)
        return 0;
    
    int xx=x+1;
    
    if (xx>rwidth)
        return 0;
    
    int yy=y+1;
    
    if (yy>rheight)
        return 0;
    
    if (region.lakesurface(xx,y)==0)
        return 0;
    
    if (region.lakesurface(x,yy)==0)
        return 0;
    
    if (region.lakesurface(xx,yy)==0)
        return 0;
    
    xx=x-1;
    
    if (xx<0)
        return 0;
    
    yy=y-1;
    
    if (yy<0)
        return 0;
    
    if (region.lakesurface(xx,yy)!=0)
        return 0;
    
    return 1;
}

// This function checks to see if a global tile is land, bordered by sea to the west, south, and southwest.

bool northeastlandonly(planet &world, int x, int y)
{
    int width=world.width();
    int height=world.height();
    
    if (world.sea(x,y)==1)
        return 0;
    
    int xx=x-1;
    
    if (xx<0)
        xx=width;
    
    int yy=y+1;
    
    if (yy>height)
        return 0;
    
    if (world.sea(xx,y)==0)
        return 0;
    
    if (world.sea(x,yy)==0)
        return 0;
    
    if (world.sea(xx,yy)==0)
        return 0;
    
    xx=x+1;
    
    if (xx>width)
        xx=0;
    
    yy=y-1;
    
    if (yy<0)
        return 0;
    
    if (world.sea(xx,yy)==1)
        return 0;
    
    return 1;
}

// Same thing, at the regional level.

bool northeastlandonly(region &region, int x, int y)
{
    int rwidth=region.rwidth();
    int rheight=region.rheight();

    if (region.sea(x,y)==1)
        return 0;
    
    int xx=x-1;
    
    if (xx<0)
        return 0;
    
    int yy=y+1;
    
    if (yy>rheight)
        return 0;
    
    if (region.sea(xx,y)==0)
        return 0;
    
    if (region.sea(x,yy)==0)
        return 0;
    
    if (region.sea(xx,yy)==0)
        return 0;
    
    xx=x+1;
    
    if (xx>rwidth)
        return 0;
    
    yy=y-1;
    
    if (yy<0)
        return 0;
    
    if (region.sea(xx,yy)==1)
        return 0;
    
    return 1;
}

// Same again, but for lakes rather than sea.

bool lakenortheastlandonly(region &region, int x, int y)
{
    int rwidth=region.rwidth();
    int rheight=region.rheight();
    
    if (region.lakesurface(x,y)!=0)
        return 0;
    
    int xx=x-1;
    
    if (xx<0)
        return 0;
    
    int yy=y+1;
    
    if (yy>rheight)
        return 0;
    
    if (region.lakesurface(xx,y)==0)
        return 0;
    
    if (region.lakesurface(x,yy)==0)
        return 0;
    
    if (region.lakesurface(xx,yy)==0)
        return 0;
    
    xx=x+1;
    
    if (xx>rwidth)
        return 0;
    
    yy=y-1;
    
    if (yy<0)
        return 0;
    
    if (region.lakesurface(xx,yy)!=0)
        return 0;
    
    return 1;
}

// This function checks to see if a global tile is land, bordered by sea to the east, north, and northeast.

bool southwestlandonly(planet &world, int x, int y)
{
    int width=world.width();
    int height=world.height();
    
    if (world.sea(x,y)==1)
        return 0;
    
    int xx=x+1;
    
    if (xx>width)
        xx=0;
    
    int yy=y-1;
    
    if (yy<0)
        return 0;
    
    if (world.sea(xx,y)==0)
        return 0;
    
    if (world.sea(x,yy)==0)
        return 0;
    
    if (world.sea(xx,yy)==0)
        return 0;
    
    xx=x+1;
    
    if (xx>width)
        x=0;
    
    yy=y+1;
    
    if (yy>height)
        return 0;
    
    if (world.sea(xx,yy)==1)
        return 0;
    
    return 1;
}

// Same thing, at the regional level.

bool southwestlandonly(region &region, int x, int y)
{
    int rwidth=region.rwidth();
    int rheight=region.rheight();
    
    if (region.sea(x,y)==1)
        return 0;
    
    int xx=x+1;
    
    if (xx>rwidth)
        return 0;
    
    int yy=y-1;
    
    if (yy<0)
        return 0;
    
    if (region.sea(xx,y)==0)
        return 0;
    
    if (region.sea(x,yy)==0)
        return 0;
    
    if (region.sea(xx,yy)==0)
        return 0;
    
    xx=x+1;
    
    if (xx>rwidth)
        return 0;
    
    yy=y+1;
    
    if (yy>rheight)
        return 0;
    
    if (region.sea(xx,yy)==1)
        return 0;
    
    return 1;
}

// Same thing, but for lakes rather than sea.

bool lakesouthwestlandonly(region &region, int x, int y)
{
    int rwidth=region.rwidth();
    int rheight=region.rheight();
    
    if (region.lakesurface(x,y)!=0)
        return 0;
    
    int xx=x+1;
    
    if (xx>rwidth)
        return 0;
    
    int yy=y-1;
    
    if (yy<0)
        return 0;
    
    if (region.lakesurface(xx,y)==0)
        return 0;
    
    if (region.lakesurface(x,yy)==0)
        return 0;
    
    if (region.lakesurface(xx,yy)==0)
        return 0;
    
    xx=x+1;
    
    if (xx>rwidth)
        return 0;
    
    yy=y+1;
    
    if (yy>rheight)
        return 0;
    
    if (region.lakesurface(xx,yy)!=0)
        return 0;
    
    return 1;
}

// This function checks to see if a global tile is land, bordered by sea to the west, north, and northwest.

bool southeastlandonly(planet &world, int x, int y)
{
    int width=world.width();
    int height=world.height();
    
    if (world.sea(x,y)==1)
        return 0;
    
    int xx=x-1;
    
    if (xx<0)
        xx=width;
    
    int yy=y-1;
    
    if (yy<0)
        return 0;
    
    if (world.sea(xx,y)==0)
        return 0;
    
    if (world.sea(x,yy)==0)
        return 0;
    
    if (world.sea(xx,yy)==0)
        return 0;
    
    xx=x+1;
    
    if (xx>width)
        xx=0;
    
    yy=y+1;
    
    if (yy>height)
        return 0;
    
    if (world.sea(xx,yy)==1)
        return 0;
    
    return 1;
}

// Same thing, at the regional level.

bool southeastlandonly(region &region, int x, int y)
{
    int rwidth=region.rwidth();
    int rheight=region.rheight();
    
    if (region.sea(x,y)==1)
        return 0;
    
    int xx=x-1;
    
    if (xx<0)
        return 0;
    
    int yy=y-1;
    
    if (yy<0)
        return 0;
    
    if (region.sea(xx,y)==0)
        return 0;
    
    if (region.sea(x,yy)==0)
        return 0;
    
    if (region.sea(xx,yy)==0)
        return 0;
    
    xx=x+1;
    
    if (xx>rwidth)
        return 0;
    
    yy=y+1;
    
    if (yy>rheight)
        return 0;
    
    if (region.sea(xx,yy)==1)
        return 0;
    
    return 1;
}

// Same thing, but for lakes rather than sea..

bool lakesoutheastlandonly(region &region, int x, int y)
{
    int rwidth=region.rwidth();
    int rheight=region.rheight();
    
    if (region.lakesurface(x,y)!=0)
        return 0;
    
    int xx=x-1;
    
    if (xx<0)
        return 0;
    
    int yy=y-1;
    
    if (yy<0)
        return 0;
    
    if (region.lakesurface(xx,y)==0)
        return 0;
    
    if (region.lakesurface(x,yy)==0)
        return 0;
    
    if (region.lakesurface(xx,yy)==0)
        return 0;
    
    xx=x+1;
    
    if (xx>rwidth)
        return 0;
    
    yy=y+1;
    
    if (yy>rheight)
        return 0;
    
    if (region.lakesurface(xx,yy)!=0)
        return 0;
    
    return 1;
}

// This function finds a neighbouring sea tile that isn't the one we just came from.

sf::Vector2i findseatile(planet &world, int x, int y, int dir)
{
    int width=world.width();
    int height=world.height();
    
    sf::Vector2i newtile;
    
    newtile.x=-1;
    newtile.y=-1;
    
    int oldx=x;
    int oldy=y;
    
    if (dir==8 || dir==1 || dir==2)
        oldy=y+1;
    
    if (dir==4 || dir==5 || dir==6)
        oldy=y-1;
    
    if (dir==2 || dir==3 || dir==4)
        oldx=x-1;
    
    if (dir==6 || dir==7 || dir==8)
        oldx=x+1;
    
    if (oldx<0 || oldx>width)
        oldx=wrap(oldx,width);
    
    // oldx and oldy are the coordinates of where we are coming to this tile from. We want to avoid going back there!
    
    int lowest=world.map(x,y);
    
    for (int i=x-1; i<=x+1; i++)
    {
        int ii=i;
        
        if (ii<0 || ii>width)
            ii=wrap(ii,width);
        
        for (int j=y-1; j<=y+1; j++)
        {
            if (ii!=x && j!=y && ii!=oldx && j!=oldy && j>=0 && j<=height && world.sea(ii,j)==1 && world.map(ii,j)<lowest)
            {
                newtile.x=ii;
                newtile.y=j;
                lowest=world.map(ii,j);
            }
        }
    }
    
    if (newtile.x!=-1)
        return (newtile);
    
    // If it didn't find a neighbouring sea tile that's lower, just find one at all.
    
    for (int i=x-1; i<=x+1; i++)
    {
        int ii=i;
        
        if (ii<0 || ii>width)
            ii=wrap(ii,width);
        
        for (int j=y-1; j<=y+1; j++)
        {
            if (ii!=x && j!=y && ii!=oldx && j!=oldy && j>=0 && j<=height && world.sea(ii,j)==1)
            {
                newtile.x=ii;
                newtile.y=j;
            }
        }
    }
    return (newtile);
}

// This returns the coordinate that the river in the given tile is flowing into.

sf::Vector2i getflowdestination(planet &world, int x, int y, int dir)
{
    int width=world.width();
    int height=world.height();
    
    sf::Vector2i destpoint;
    
    if (dir==0)
        dir=world.riverdir(x,y);
    
    if (dir==8 || dir==1 || dir==2)
        y--;
    
    if (dir==4 || dir==5 || dir==6)
        y++;
    
    if (dir==2 || dir==3 || dir==4)
        x++;
    
    if (dir==6 || dir==7 || dir==8)
        x--;
    
    if (x<0 || x>width)
        x=wrap(x,width);
    
    if (y<0)
        y=0;
    
    if (y>height)
        y=height;
    
    destpoint.x=x;
    destpoint.y=y;
    
    return (destpoint);
}

// The same thing, but for the regional map.

sf::Vector2i getregionalflowdestination(region &region, int x, int y, int dir)
{
    int rwidth=region.rwidth();
    int rheight=region.rheight();
    
    sf::Vector2i destpoint;
    
    if (dir==0)
        dir=region.riverdir(x,y);
    
    if (dir==8 || dir==1 || dir==2)
        y--;
    
    if (dir==4 || dir==5 || dir==6)
        y++;
    
    if (dir==2 || dir==3 || dir==4)
        x++;
    
    if (dir==6 || dir==7 || dir==8)
        x--;
    
    if (x<0 || x>rwidth || y<0 || y>rheight)
    {
        destpoint.x=-1;
        destpoint.y=-1;
    }
    else
    {
        destpoint.x=x;
        destpoint.y=y;
    }
    
    return destpoint;
}

// This function finds the cell with the largest flow into the current one.

sf::Vector2i getupstreamcell(planet &world, int x, int y)
{
    int width=world.width();
    int height=world.height();
    
    sf::Vector2i upcell, destpoint;
    
    upcell.x=-1;
    upcell.y=-1;
    
    int largest=0;
    
    int riverjan, riverjul;
    
    for (int i=x-1; i<=x+1; i++)
    {
        int ii=i;
        
        if (ii<0 || ii>width)
            ii=wrap(ii,width);
        
        for (int j=y-1; j<=y+1; j++)
        {
            if (j>=0 && y<=height)
            {
                riverjan=world.riverjan(ii,j);
                riverjul=world.riverjul(ii,j);
                
                if (riverjan+riverjul>largest) // If this is larger than the current largest inflow
                {
                    destpoint=getflowdestination(world,ii,j,0);
                    
                    if (destpoint.x==x && destpoint.y==y) // If this is actually flowing into our cell
                    {
                        upcell.x=ii;
                        upcell.y=j;
                        
                        largest=riverjan+riverjul;
                    }
                    
                }
            }
        }
    }
    return (upcell);
}

// This function checks to see whether a tile on the global map has any water flowing into it.

int checkwaterinflow(planet &world, int x, int y)
{
    int i, j;
    
    int width=world.width();
    int height=world.height();
    
    // From the north
    
    i=x;
    j=y-1;
    
    if (j>=0 && world.riverdir(i,j)==5)
        return 1;
    
    // From the northeast
    
    i=x+1;
    if (i>width)
        i=0;
    j=y-1;
    
    if (j>=0 && world.riverdir(i,j)==6)
        return 1;
    
    // From the east
    
    i=x+1;
    if (i>width)
        i=0;
    j=y;
    
    if (j>=0 && world.riverdir(i,j)==7)
        return 1;
    
    // From the southeast
    
    i=x+1;
    if (i>width)
        i=0;
    j=y+1;
    
    if (j<=height && world.riverdir(i,j)==8)
        return 1;
    
    // From the south
    
    i=x;
    j=y+1;
    
    if (j<=height && world.riverdir(i,j)==1)
        return 1;
    
    // From the southwest
    
    i=x-1;
    if (i<0)
        i=width;
    j=y+1;
    
    if (j<=height && world.riverdir(i,j)==2)
        return 1;
    
    // From the west
    
    i=x-1;
    if (i<0)
        i=width;
    j=y;
    
    if (j<=height && world.riverdir(i,j)==3)
        return 1;
    
    // From the northwest
    
    i=x-1;
    if (i<0)
        i=width;
    j=y-1;
    
    if (j>=0 && world.riverdir(i,j)==4)
        return 1;
    
    return 0;
}

// This does the same thing for the regional map.

int checkregionalwaterinflow(region &region, int x, int y)
{
    int i, j;
    
    // From the north
    
    i=x;
    j=y-1;
    
    if (region.riverdir(i,j)==5)
        return 1;
    
    // From the northeast
    
    i=x+1;
    j=y-1;
    
    if (region.riverdir(i,j)==6)
        return 1;
    
    // From the east
    
    i=x+1;
    j=y;
    
    if (region.riverdir(i,j)==7)
        return 1;
    
    // From the southeast
    
    i=x+1;
    j=y+1;
    
    if (region.riverdir(i,j)==8)
        return 1;
    
    // From the south
    
    i=x;
    j=y+1;
    
    if (region.riverdir(i,j)==1)
        return 1;
    
    // From the southwest
    
    i=x-1;
    j=y+1;
    
    if (region.riverdir(i,j)==2)
        return 1;
    
    // From the west
    
    i=x-1;
    j=y;
    
    if (region.riverdir(i,j)==3)
        return 1;
    
    // From the northwest
    
    i=x-1;
    j=y-1;
    
    if (region.riverdir(i,j)==4)
        return 1;
    
    return 0;
}

// This function gets the total water inflow into a given tile on the global map.

sf::Vector2i gettotalinflow(planet &world, int x, int y)
{
    int width=world.width();
    int height=world.height();
    
    sf::Vector2i total;
    sf::Vector2i dest;
    
    total.x=0;
    total.y=0;
    
    for (int i=x-1; i<=x+1; i++)
    {
        int ii=i;
        
        if (ii<0 || ii>width)
            ii=wrap(ii,width);
        
        for (int j=y-1; j<=y+1; j++)
        {
            if (j>=0 && j<=height)
            {
                dest=getflowdestination(world,i,j,0);
                
                if (dest.x==x && dest.y==y)
                {
                    total.x=total.x+world.riverjan(i,j);
                    total.y=total.y+world.riverjul(i,j);
                }
            }
        }
    }
    return total;
}

// This does the same thing on the regional map.

sf::Vector2i gettotalinflow(region &region, int x, int y)
{
    int rwidth=region.rwidth();
    int rheight=region.rheight();
    
    sf::Vector2i total;
    sf::Vector2i dest;
    
    total.x=0;
    total.y=0;
    
    for (int i=x-1; i<=x+1; i++)
    {
        for (int j=y-1; j<=y+1; j++)
        {
            if (i>=0 && i<=rwidth && j>=0 && j<=rheight)
            {
                dest=getregionalflowdestination(region,i,j,0);
                
                if (dest.x==x && dest.y==y)
                {
                    total.x=total.x+region.riverjan(i,j);
                    total.y=total.y+region.riverjul(i,j);
                }
            }
        }
    }
    return total;
}

// This function finds the lowest neighbouring point that's still higher, on the regional map.

sf::Vector2i findlowesthigher(region &region, int dx, int dy, int x, int y, int janload, int julload, int crount, vector<vector<bool>> &mainriver)
{
    sf::Vector2i nextpoint;
    int origelev=region.map(x,y);
    
    nextpoint.x=-1;
    nextpoint.y=-1;
    
    int elev=100000000;
    
    for (int i=x-1; i<=x+1; i++)
    {
        for (int j=y-1; j<=y+1; j++)
        {
            if (region.map(i,j)<elev && region.map(i,j)>=origelev && region.sea(i,j)==0 && region.riverjan(i,j)==0 && region.riverjul(i,j)==0)
            {
                bool keepgoing=1;
                
                if (crount>1)
                {
                    int neighbouringrivers=0;
                    
                    for (int k=i-1; k<=i+1; k++)
                    {
                        for (int l=j-1; l<=j+1; l++)
                        {
                            if (region.riverjan(k,l)!=0 && region.riverjan(k,l)!=janload && region.riverjul(k,l)!=0 && region.riverjul(k,l)!=julload)
                                keepgoing=0; // After the first point, avoid going next to the main river in this tile.
                            
                            if (region.riverjan(k,l)!=0 || region.riverjul(k,l)!=0)
                                neighbouringrivers++;
                            
                            if (region.sea(k,l)==1) // Avoid going near coastlines after the first point, too.
                                keepgoing=0;
                        }
                    }
                    
                    if (neighbouringrivers>2)
                        keepgoing=0;
                }
                else
                {
                    for (int k=i-1; k<=i+1; k++)
                    {
                        for (int l=j-1; l<=j+1; l++)
                        {
                            if (region.riverdir(k,l)!=0) // && mainriver[dx-k][dy-l]==0)
                                keepgoing=0;
                        }
                    }
                }
                
                // Avoid crossing existing rivers diagonally.
                
                if (i==x+1 && j==y+1)
                {
                    if (region.riverdir(x+1,y)==6 || region.riverdir(x,y+1)==2)
                        keepgoing=0;
                }
                
                if (i==x-1 && j==y-1)
                {
                    if (region.riverdir(x,y-1)==6 || region.riverdir(x-1,y)==2)
                        keepgoing=0;
                }
                
                if (i==x+1 && j==y-1)
                {
                    if (region.riverdir(x,y-1)==4 || region.riverdir(x+1,y)==8)
                        keepgoing=0;
                }
                
                if (i==x-1 && j==y+1)
                {
                    if (region.riverdir(x-1,y)==4 || region.riverdir(x,y+1)==8)
                        keepgoing=0;
                }
                
                if (keepgoing==1)
                {
                    elev=region.map(i,j);
                    nextpoint.x=i;
                    nextpoint.y=j;
                }
            }
        }
    }
    return nextpoint;
}

// This function tells whether a given point on the world map is in or next to a lake.

int nearlake(planet &world, int x, int y, int dist, bool rift)
{
    int width=world.width();
    int height=world.height();
    
    for (int i=x-dist; i<=x+dist; i++)
    {
        int ii=i;
        
        if (ii<0 || ii>width)
            ii=wrap(ii,width);
        
        for (int j=y-dist; j<=y+dist; j++)
        {
            if (j>=0 && j<=height)
            {
                if (world.lakesurface(ii,j)!=0)
                    return world.lakesurface(ii,j);
                
                if (rift==1 && world.riftlakesurface(ii,j)!=0)
                    return world.riftlakesurface(ii,j);
            }
        }
    }
    return 0;
}

// This function tells us whether a tile is a lake tile on the edge.

int getlakeedge(planet &world, int x, int y)
{
    if (world.lakesurface(x,y)==0)
        return 0;
    
    int width=world.width();
    int height=world.height();
    
    for (int i=x-1; i<=x+1; i++)
    {
        int ii=i;
        
        if (ii<0 || ii>width)
            ii=wrap(ii,width);
        
        for (int j=y-1; j<=y+1; j++)
        {
            if (j>=0 && j<=height)
            {
                if (world.lakesurface(ii,j)==0)
                    return 1;
            }
        }
    }
    return 0;
}

// This function tells whether a regional cell is next to a lake.

int nexttolake(region &region, int x, int y)
{
    int rwidth=region.rwidth();
    int rheight=region.rheight();
    
    int dist=1;
    
    for (int i=x-dist; i<=x+dist; i++)
    {
        int ii=i;
        
        for (int j=y-dist; j<=y+dist; j++)
        {
            if (i>=0 && i<=rwidth && j>=0 && j<=rheight && region.lakesurface(ii,j)!=0)
                return (region.lakesurface(ii,j));
        }
    }
    return (0);
}

// This function finds the level of the nearest lake to the given point in the regional map.

int getnearestlakelevel(region &region, int x, int y)
{
    int rwidth=region.rwidth();
    int rheight=region.rheight();
    
    for (int n=1; n<=200; n++)
    {
        for (int i=x-n; i<=x+n; i++)
        {
            for (int j=y-n; j<=y+n; j++)
            {
                if (i>=0 && i<=rwidth && j>=0 && j<=rheight)
                {
                    if (region.lakesurface(i,j)!=0)
                        return region.lakesurface(i,j);
                }
            }
        }
    }
    return 0;
}

// This function finds the special value of the nearest lake to the given point in the regional map.

int getnearestlakespecial(region &region, int x, int y)
{
    int rwidth=region.rwidth();
    int rheight=region.rheight();
    
    for (int n=1; n<=200; n++)
    {
        for (int i=x-n; i<=x+n; i++)
        {
            for (int j=y-n; j<=y+n; j++)
            {
                if (i>=0 && i<=rwidth && j>=0 && j<=rheight)
                {
                    if (region.lakesurface(i,j)!=0)
                        return region.special(i,j);
                }
            }
        }
    }
    return 0;
}

// This function finds the nearest river on the regional map.

sf::Vector2i findclosestriver(region &region, int x, int y, vector<vector<vector<int>>> &circletemplates, bool delta)
{
    int rwidth=region.rwidth();
    int rheight=region.rheight();
    
    sf::Vector2i nearest;
    
    nearest.x=-1;;
    nearest.y=-1;
    
    for (int n=1; n<=14; n++)
    {
        int ii=0;
        
        for (int i=x-1; i<=x+n; i++)
        {
            int jj=0;
            
            for (int j=y-n; j<=y+n; j++)
            {
                if (circletemplates[n*2+1][ii][jj]==1)
                {
                    if (i>=0 && i<=rwidth && j>=0 && j<=rheight && region.waterdir(i,j,delta)!=0)
                    {
                        nearest.x=i;
                        nearest.y=j;
                        
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

// As above, but without using circletemplates.

sf::Vector2i findclosestriverquickly(region &region, int x, int y)
{
    sf::Vector2i nearest;
    
    if (region.riverdir(x,y)!=0)
    {
        nearest.x=x;
        nearest.y=y;
        return nearest;
    }
    
    nearest.x=-1;;
    nearest.y=-1;
    
    for (int n=1; n<=30; n++)
    {
        for (int i=x-n; i<=x+n; i++)
        {
            int j=y-n;
            
            if (region.riverdir(i,j)!=0)
            {
                nearest.x=i;
                nearest.y=j;
                return nearest;
            }
            
            j=y+n;
            
            if (region.riverdir(i,j)!=0)
            {
                nearest.x=i;
                nearest.y=j;
                return nearest;
            }
        }
        
        for (int j=y-n; j<=y+n; j++)
        {
            int i=x-n;
            
            if (region.riverdir(i,j)!=0)
            {
                nearest.x=i;
                nearest.y=j;
                return nearest;
            }
            
            i=x+n;
            
            if (region.riverdir(i,j)!=0)
            {
                nearest.x=i;
                nearest.y=j;
                return nearest;
            }
        }
    }
    
    return nearest;
}

// This function creates circle templates.

void createcircletemplates(vector<vector<vector<int>>> &circletemplates)
{
    int radius,x,y;
    
    for (int n=3; n<=59; n=n+2)
    {
        radius=n/2;
        x=radius;
        y=x;
        
        drawcircle3d(circletemplates,x,y,1,radius,n);
    }
    
    for (int n=2; n<=60; n=n+2)
    {
        radius=n/2-1;
        
        x=radius-1;
        y=radius-1;
        
        drawcircle3d(circletemplates,x,y,1,radius,n);
        
        x=radius;
        y=radius-1;
        
        drawcircle3d(circletemplates,x,y,1,radius,n);
        
        x=radius-1;
        y=radius;
        
        drawcircle3d(circletemplates,x,y,1,radius,n);
        
        x=radius;
        y=radius;
        
        drawcircle3d(circletemplates,x,y,1,radius,n);
    }
}

// This function finds how many inflows there are to a cell on the regional map.

int countinflows(region &region, int x, int y)
{
    int rwidth=region.rwidth();
    int rheight=region.rheight();
    
    int inflows=0;
    sf::Vector2i dest;
    
    for (int i=x-1; i<=x+1; i++)
    {
        for (int j=y-1; j<=y+1; j++)
        {
            if (i>=0 && i<=rwidth && j>=0 && j<=rheight)
            {
                dest=getregionalflowdestination(region,i,j,0);
                
                if (dest.x==x && dest.y==y)
                    inflows++;
            }
        }
    }
    
    return inflows;
}

// This function plugs the global variables into the world.

void initialiseworld(planet &world)
{
    int width=2047; //1023; //1024;
    int height=1024; //512;
    bool rotation=1;            // 1 to rotate like Earth, 0 for the other way
    float riverfactor=15.0;     // for calculating flow in cubic metres/second
    int riverlandreduce=20;     // how much rivers lower the land
    int estuarylimit=20;        // how big a river must be to have an estuary
    int glacialtemp=4;          // maximum temperature for glacial features
    int glaciertemp=4;          // maximum temperature for actual glaciers
    float mountainreduce=0.75;  // factor to reduce mountain size by
    int climatenumber=31;       // total number of climate types
    int maxelevation=24000; //12750;     // maximum elevation
    int sealevel=12000; //2500;          // sea level
    int noisewidth=1024;
    int noiseheight=512;
    
    
    world.setwidth(width);
    world.setheight(height);
    world.setrotation(rotation);
    world.setriverfactor(riverfactor);
    world.setriverlandreduce(riverlandreduce);
    world.setestuarylimit(estuarylimit);
    world.setglacialtemp(glacialtemp);
    world.setglaciertemp(glaciertemp);
    world.setmountainreduce(mountainreduce);
    world.setclimatenumber(climatenumber);
    world.setmaxelevation(maxelevation);
    world.setsealevel(sealevel);
    world.setnoisewidth(noisewidth);
    world.setnoiseheight(noiseheight);
}

// This function sets up the default colours (and other settings) for drawing relief maps.

void initialisemapcolours(planet &world)
{
    world.setlandshading(0.7);
    world.setlakeshading(0.3);
    world.setseashading(0.5);
    
    world.setlandmarbling(0.5);
    world.setlakemarbling(0.8);
    world.setseamarbling(0.8);
    
    world.setshadingdir(8);
    world.setsnowchange(1);
    world.setseaiceappearance(1);
    world.setminriverflowglobal(1000000);
    world.setminriverflowregional(150);
    
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
    
    world.sethighlight1(0);
    world.sethighlight2(255);
    world.sethighlight3(255); // Colours of highlights on the map.
}

// This function sets up the always-used variables in the region.

void initialiseregion(planet &world, region &region)
{
    int tilewidth=REGIONALTILEWIDTH+4;//34;
    int tileheight=REGIONALTILEHEIGHT+4; //34;
    
    int rstart=16;          // Amount to ignore on the edge of the regional map for most purposes.
    int pixelmetres=2500;   // number of metres each pixel represents
    
    region.settilewidth(tilewidth);
    region.settileheight(tileheight);
    region.setrstart(rstart);
    region.setsealevel(world.sealevel());
    region.setpixelmetres(pixelmetres);
}

// This function gets colours for drawing climate maps.

sf::Color getclimatecolours(string climate)
{
    int colour1=0;
    int colour2=0;
    int colour3=0;
    
    if (climate=="Af")
    {
        colour1=0;
        colour2=0;
        colour3=254;
    }
    
    if (climate=="Am")
    {
        colour1=1;
        colour2=119;
        colour3=255;
    }
    
    if (climate=="Aw")
    {
        colour1=70;
        colour2=169;
        colour3=250;
    }
    
    if (climate=="As")
    {
        colour1=70;
        colour2=169;
        colour3=250;
    }
    
    if (climate=="BWh")
    {
        colour1=249;
        colour2=15;
        colour3=0;
    }
    
    if (climate=="BWk")
    {
        colour1=251;
        colour2=150;
        colour3=149;
    }
    
    if (climate=="BSh")
    {
        colour1=245;
        colour2=163;
        colour3=1;
    }
    
    if (climate=="BSk")
    {
        colour1=254;
        colour2=219;
        colour3=99;
    }
    
    if (climate=="Csa")
    {
        colour1=255;
        colour2=255;
        colour3=0;
    }
    
    if (climate=="Csb")
    {
        colour1=198;
        colour2=199;
        colour3=1;
    }
    
    if (climate=="Csc")
    {
        colour1=184;
        colour2=184;
        colour3=114;
    }
    
    if (climate=="Cwa")
    {
        colour1=138;
        colour2=255;
        colour3=162;
    }
    
    if (climate=="Cwb")
    {
        colour1=86;
        colour2=199;
        colour3=112;
    }
    
    if (climate=="Cwc")
    {
        colour1=30;
        colour2=150;
        colour3=66;
    }
    
    if (climate=="Cfa")
    {
        colour1=192;
        colour2=254;
        colour3=109;
    }
    
    if (climate=="Cfb")
    {
        colour1=76;
        colour2=255;
        colour3=93;
    }
    
    if (climate=="Cfc")
    {
        colour1=19;
        colour2=203;
        colour3=74;
    }
    
    if (climate=="Dsa")
    {
        colour1=255;
        colour2=8;
        colour3=245;
    }
    
    if (climate=="Dsb")
    {
        colour1=204;
        colour2=3;
        colour3=192;
    }
    
    if (climate=="Dsc")
    {
        colour1=154;
        colour2=51;
        colour3=144;
    }
    
    if (climate=="Dsd")
    {
        colour1=153;
        colour2=100;
        colour3=146;
    }
    
    if (climate=="Dwa")
    {
        colour1=172;
        colour2=178;
        colour3=249;
    }
    
    if (climate=="Dwb")
    {
        colour1=91;
        colour2=121;
        colour3=213;
    }
    
    if (climate=="Dwc")
    {
        colour1=78;
        colour2=83;
        colour3=175;
    }
    
    if (climate=="Dwd")
    {
        colour1=54;
        colour2=3;
        colour3=130;
    }
    
    if (climate=="Dfa")
    {
        colour1=0;
        colour2=255;
        colour3=245;
    }
    
    if (climate=="Dfb")
    {
        colour1=32;
        colour2=200;
        colour3=250;
    }
    
    if (climate=="Dfc")
    {
        colour1=0;
        colour2=126;
        colour3=125;
    }
    
    if (climate=="Dfd")
    {
        colour1=0;
        colour2=69;
        colour3=92;
    }
    
    if (climate=="ET")
    {
        colour1=178;
        colour2=178;
        colour3=178;
    }
    
    if (climate=="EF")
    {
        colour1=104;
        colour2=104;
        colour3=104;
    }
    
    sf::Color colour(colour1,colour2,colour3);
    
    return (colour);
}

// This function draws a global map image (ready to be applied to a texture).

void drawglobalmapimage(mapviewenum mapview, planet &world, bool globalmapimagecreated[], stbi_uc globalelevationimage[], stbi_uc globalwindsimage[], stbi_uc globaltemperatureimage[], stbi_uc globalprecipitationimage[], stbi_uc globalclimateimage[], stbi_uc globalriversimage[], stbi_uc globalreliefimage[], int globalimagewidth, int globalimageheight, int globalimagechannels)
{
    // mapview tells us which of these we're actually drawing.
    
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    int globalimagesize=globalimagewidth*globalimageheight*globalimagechannels;
    int index=0;
    
    int colour1, colour2, colour3;
    
    switch (mapview)
    {
        case elevation:
        {
            if (globalmapimagecreated[0]==1)
                return;
            
            int landdiv=((world.maxelevation()-sealevel)/2)/255;
            int seadiv=sealevel/255;
            
            int div=world.maxelevation()/255;
            int base=world.maxelevation()/4;

            for (int i=0; i<=width; i++)
            {
                for (int j=0; j<=height; j++)
                {
                    int heightpoint=world.map(i,j);
                    
                    if (world.lakesurface(i,j)!=0 && world.special(i,j)==0)
                        heightpoint=world.lakesurface(i,j);
                    
                    colour1=heightpoint/div;
                    
                    if (colour1>255)
                        colour1=255;
                    
                    colour2=colour1;
                    colour3=colour2;
                    
                    index=(i+j*globalimagewidth)*globalimagechannels;
                    
                    globalelevationimage[index]=colour1;
                    globalelevationimage[index+1]=colour2;
                    globalelevationimage[index+2]=colour3;
                }
            }

            globalmapimagecreated[0]=1;
            
            break;
        }
            
        case winds: // Originally there was a winds map too, but I cut it as not very interesting.
        {
            if (globalmapimagecreated[1]==1)
                return;

            for (int i=0; i<=width; i++)
            {
                for (int j=0; j<=height; j++)
                {
                    if (world.outline(i,j))
                    {
                        colour1=0;
                        colour2=0;
                        colour3=0;
                    }
                    else
                    {
                        int wind=world.wind(i,j);
                        
                        if (wind==0 || wind>50)
                        {
                            colour1=250;
                            colour2=250;
                            colour3=250;
                        }
                        else
                        {
                            if (wind>0)
                            {
                                colour1=250;
                                colour2=250-(wind*10);
                                colour3=250;
                            }
                            else
                            {
                                wind=0-wind;
                                
                                colour1=250-(wind*10);
                                colour2=250;
                                colour3=250;
                            }
                        }
                    }
                    
                    if (colour1>255)
                        colour1=255;
                    
                    if (colour2>255)
                        colour2=255;
                    
                    if (colour2>255)
                        colour3=255;
                    
                    if (colour1<0)
                        colour1=0;
                    
                    if (colour2<0)
                        colour2=0;
                    
                    if (colour3<0)
                        colour3=0;
                    
                    index=(i+j*globalimagewidth)*globalimagechannels;
                    
                    globalwindsimage[index]=colour1;
                    globalwindsimage[index+1]=colour2;
                    globalwindsimage[index+2]=colour3;
                }
            }
            
            globalmapimagecreated[1]=1;
            
            break;
        }
            
        case temperature:
        {
            if (globalmapimagecreated[2]==1)
                return;
            
            for (int i=0; i<=width; i++)
            {
                for (int j=0; j<=height; j++)
                {
                    if (world.outline(i,j))
                    {
                        colour1=0;
                        colour2=0;
                        colour3=0;
                    }
                    else
                    {
                        int temperature=(world.mintemp(i,j)+world.maxtemp(i,j))/2;
                        
                        temperature=temperature+10;
                        
                        if (temperature>0)
                        {
                            colour1=250;
                            colour2=250-(temperature*3);
                            colour3=250-(temperature*7);
                        }
                        else
                        {
                            temperature=abs(temperature);
                            
                            colour1=250-(temperature*7);
                            colour2=250-(temperature*7);
                            colour3=250;
                        }
                    }
                    
                    if (colour1<0)
                        colour1=0;
                    
                    if (colour2<0)
                        colour2=0;
                    
                    if (colour3<0)
                        colour3=0;
                    
                    index=(i+j*globalimagewidth)*globalimagechannels;
                    
                    globaltemperatureimage[index]=colour1;
                    globaltemperatureimage[index+1]=colour2;
                    globaltemperatureimage[index+2]=colour3;
                }
            }
            
            globalmapimagecreated[2]=1;
            
            break;
        }
            
        case precipitation:
        {
            if (globalmapimagecreated[3]==1)
                return;
            
            for (int i=0; i<=width; i++)
            {
                for (int j=0; j<=height; j++)
                {
                    if (world.outline(i,j))
                    {
                        colour1=255;
                        colour2=0;
                        colour3=255;
                    }
                    else
                    {
                        int rainfall=(world.summerrain(i,j)+world.winterrain(i,j))/2;
                        
                        rainfall=rainfall/4;
                        
                        colour1=255-rainfall;
                        colour2=255-rainfall;
                        colour3=255;
                    }

                    if (colour1<0)
                        colour1=0;
                    
                    if (colour2<0)
                        colour2=0;
                    
                    if (colour3<0)
                        colour3=0;
                    
                    index=(i+j*globalimagewidth)*globalimagechannels;
                    
                    globalprecipitationimage[index]=colour1;
                    globalprecipitationimage[index+1]=colour2;
                    globalprecipitationimage[index+2]=colour3;
                }
            }
            
            globalmapimagecreated[3]=1;
            
            break;
        }
            
        case climate:
        {
            if (globalmapimagecreated[4]==1)
                return;
            
            for (int i=0; i<=width; i++)
            {
                for (int j=0; j<=height; j++)
                {
                    if (world.sea(i,j))
                    {
                        if (world.seaice(i,j)==2) // Permanent sea ice
                        {
                            colour1=243;
                            colour2=243;
                            colour3=255;
                        }
                        else
                        {
                            if (world.seaice(i,j)==1) // Seasonal sea ice
                            {
                                colour1=228;
                                colour2=228;
                                colour3=255;
                            }
                            else // Open sea
                            {
                                colour1=13;
                                colour2=49;
                                colour3=109;
                            }
                        }
                        
                        index=(i+j*globalimagewidth)*globalimagechannels;
                        
                        globalclimateimage[index]=colour1;
                        globalclimateimage[index+1]=colour2;
                        globalclimateimage[index+2]=colour3;
                    }
                    else
                    {
                        sf::Color landcolour;
                        
                        landcolour=getclimatecolours(world.climate(i,j));
                        
                        colour1=landcolour.r;
                        colour2=landcolour.g;
                        colour3=landcolour.b;
                        
                        index=(i+j*globalimagewidth)*globalimagechannels;
                        
                        globalclimateimage[index]=colour1;
                        globalclimateimage[index+1]=colour2;
                        globalclimateimage[index+2]=colour3;
                    }
                }
            }
            globalmapimagecreated[4]=1;
            
            break;
        }
            
        case rivers:
        {
            if (globalmapimagecreated[5]==1)
                return;
            
            int mult=world.maxriverflow()/255;
            
            if (mult<1)
                mult=1;
            
            for (int i=0; i<=width; i++)
            {
                for (int j=0; j<=height; j++)
                {
                    if (world.outline(i,j))
                    {
                        colour1=0;
                        colour2=0;
                        colour3=0;
                    }
                    else
                    {
                        int flow=world.riveraveflow(i,j);
                        
                        if (flow>0 && world.sea(i,j)==0)
                        {
                            flow=flow*10;
                            
                            colour1=255-(flow/mult);
                            if (colour1<0)
                                colour1=0;
                            colour2=colour1;
                        }
                        else
                        {
                            if (world.deltadir(i,j)!=0 && world.sea(i,j)==0)
                            {
                                flow=(world.deltajan(i,j)+world.deltajul(i,j))/2;
                                flow=flow*10;
                                
                                colour1=255-(flow/mult);
                                if (colour1<0)
                                    colour1=0;
                                colour2=colour1;
                            }
                            else
                            {
                                colour1=255;
                                colour2=255;
                            }
                        }
                        
                        colour3=255;
                        
                        if (world.truelake(i,j)!=0)
                        {
                            colour1=150;
                            colour2=150;
                            colour3=250;
                        }
                        
                        if (world.special(i,j)>100 && world.sea(i,j)==0 && world.riverjan(i,j)+world.riverjul(i,j)<600)
                        {
                            if (world.special(i,j)==110)
                            {
                                colour1=150;
                                colour2=150;
                                colour3=150;
                            }
                            
                            if (world.special(i,j)==120)
                            {
                                colour1=250;
                                colour2=250;
                                colour3=50;
                            }
                            
                            if (world.special(i,j)>=130)
                            {
                                colour1=50;
                                colour2=250;
                                colour3=100;
                            }
                        }
                    }
                    
                    if (world.outline(i,j))
                    {
                        colour1=0;
                        colour2=0;
                        colour3=0;
                    }
                    
                    if (world.volcano(i,j)>0)
                    {
                        colour1=240;
                        colour2=0;
                        colour3=0;
                    }
                    
                    index=(i+j*globalimagewidth)*globalimagechannels;
                    
                    globalriversimage[index]=colour1;
                    globalriversimage[index+1]=colour2;
                    globalriversimage[index+2]=colour3;
                }
            }
            
            globalmapimagecreated[5]=1;
            
            break;
        }
            
        case relief:
        {
            if (globalmapimagecreated[6]==1)
                return;
            
            int minriverflow=world.minriverflowglobal(); // Rivers of this size or larger will be shown on the map.
            
            float landshading=world.landshading();
            float lakeshading=world.lakeshading();
            float seashading=world.seashading();
            
            int shadingdir=world.shadingdir();
            
            vector<vector<short>> reliefmap1(ARRAYWIDTH,vector<short>(ARRAYWIDTH,ARRAYHEIGHT));
            vector<vector<short>> reliefmap2(ARRAYWIDTH,vector<short>(ARRAYWIDTH,ARRAYHEIGHT));
            vector<vector<short>> reliefmap3(ARRAYWIDTH,vector<short>(ARRAYWIDTH,ARRAYHEIGHT));
            
            for (int i=0; i<=width; i++)
            {
                for (int j=0; j<=height; j++)
                {
                    reliefmap1[i][j]=0;
                    reliefmap2[i][j]=0;
                    reliefmap3[i][j]=0;
                }
            }
            
            int var=0; // Amount colours may be varied to make the map seem more speckledy.

            for (int i=0; i<=width; i++)
            {
                for (int j=0; j<=height; j++)
                {
                    var=10;
                    
                    int sea=world.sea(i,j);
                    
                    if (sea==1)
                    {
                        if ((world.seaice(i,j)==2 && (world.seaiceappearance()==1 || world.seaiceappearance()==3)) || (world.seaice(i,j)==1 && world.seaiceappearance()==3)) // Sea ice
                        {
                            colour1=world.seaice1();
                            colour2=world.seaice2();
                            colour3=world.seaice3();
                            
                            var=0;
                        }
                        else
                        {
                            colour1=(world.ocean1()*world.map(i,j)+world.deepocean1()*(sealevel-world.map(i,j)))/sealevel;
                            colour2=(world.ocean2()*world.map(i,j)+world.deepocean2()*(sealevel-world.map(i,j)))/sealevel;
                            colour3=(world.ocean3()*world.map(i,j)+world.deepocean3()*(sealevel-world.map(i,j)))/sealevel;
                            
                            var=5;
                        }
                    }
                    else
                    {
                        if ((world.riverjan(i,j)+world.riverjul(i,j))/2>=minriverflow)
                        {
                            colour1=world.river1();
                            colour2=world.river2();
                            colour3=world.river3();
                        }
                        else
                        {
                            if (world.special(i,j)==110) // Salt pan
                            {
                                colour1=world.saltpan1();
                                colour2=world.saltpan2();
                                colour3=world.saltpan3();
                                
                                var=20;
                            }
                            else
                            {
                                int avetemp=world.avetemp(i,j)+10;
                                
                                // First, adjust the base colours depending on temperature.
                                
                                int thisbase1, thisbase2, thisbase3, newdesert1, newdesert2, newdesert3;
                                
                                if (avetemp>30)
                                {
                                    thisbase1=world.base1();
                                    thisbase2=world.base2();
                                    thisbase3=world.base3();
                                    
                                    newdesert1=world.desert1();
                                    newdesert2=world.desert2();
                                    newdesert3=world.desert3();
                                }
                                else
                                {
                                    int hotno=avetemp/3;
                                    int coldno=10-hotno;
                                    
                                    thisbase1=(hotno*world.base1()+coldno*world.basetemp1())/10;
                                    thisbase2=(hotno*world.base2()+coldno*world.basetemp2())/10;
                                    thisbase3=(hotno*world.base3()+coldno*world.basetemp3())/10;
                                }
                                
                                if (avetemp>30)
                                {
                                    newdesert1=world.desert1();
                                    newdesert2=world.desert2();
                                    newdesert3=world.desert3();
                                }
                                else
                                {
                                    if (avetemp<=10)
                                    {
                                        newdesert1=world.colddesert1();
                                        newdesert2=world.colddesert2();
                                        newdesert3=world.colddesert3();
                                    }
                                    else
                                    {
                                        int hotno=avetemp-10;
                                        int coldno=20-hotno;
                                        
                                        newdesert1=(hotno*world.desert1()+coldno*world.colddesert1())/20;
                                        newdesert2=(hotno*world.desert2()+coldno*world.colddesert2())/20;
                                        newdesert3=(hotno*world.desert3()+coldno*world.colddesert3())/20;
                                    }
                                }
                                
                                // Now, adjust for the presence of monsoon.
                                
                                float winterrain=world.winterrain(i,j);
                                float summerrain=world.summerrain(i,j);
                                
                                float totalrain=winterrain+summerrain;
                                
                                float monsoon=0;
                                
                                if (winterrain<1)
                                    winterrain=1;

                                if (winterrain<summerrain)
                                {
                                    monsoon=summerrain-winterrain;
                                    
                                    monsoon=monsoon/1000; // 410
                                    
                                    if (monsoon>0.99)
                                        monsoon=0.99;
                                }
                                
                                // The closer it is to tropical rainforest, the more we intensify the rain effect.
                                
                                float rainforestmult=world.mintemp(i,j)/18.0; //9.0;
                                
                                rainforestmult=rainforestmult*world.winterrain(i,j)/80.0;
                                
                                if (rainforestmult<1)
                                    rainforestmult=1;
                                
                                totalrain=totalrain*rainforestmult;
                                
                                // Now adjust the colours for height.
                                
                                int mapelev=world.map(i,j)-sealevel;
                                
                                int newbase1, newbase2, newbase3, newgrass1, newgrass2, newgrass3;
                                
                                if (mapelev>2000)
                                {
                                    newdesert1=world.highdesert1();
                                    newdesert2=world.highdesert2();
                                    newdesert3=world.highdesert3();
                                }
                                else
                                {
                                    int highno=mapelev/50;
                                    int lowno=40-highno;
                                    
                                    newdesert1=(highno*world.highdesert1()+lowno*newdesert1)/40;
                                    newdesert2=(highno*world.highdesert2()+lowno*newdesert2)/40;
                                    newdesert3=(highno*world.highdesert3()+lowno*newdesert3)/40;
                                }
                                
                                if (mapelev>3000)
                                {
                                    newbase1=world.highbase1();
                                    newbase2=world.highbase2();
                                    newbase3=world.highbase3();
                                    
                                    newgrass1=world.highbase1();
                                    newgrass2=world.highbase2();
                                    newgrass3=world.highbase3();
                                }
                                else
                                {
                                    int highno=mapelev/75;
                                    int lowno=40-highno;
                                    
                                    newbase1=(highno*world.highbase1()+lowno*thisbase1)/40;
                                    newbase2=(highno*world.highbase2()+lowno*thisbase2)/40;
                                    newbase3=(highno*world.highbase3()+lowno*thisbase3)/40;
                                    
                                    /*
                                    // Adjust it for monsoon colouring.
                                    
                                    monsoon=monsoon*0.3;
                                    
                                    float notmonsoon=1.0-monsoon;
                                    
                                    int monsooncol1=195;
                                    int monsooncol2=210;
                                    int monsooncol3=123;
                                    
                                    int thisgrass1=((monsoon*100*monsooncol1)+(notmonsoon*100*world.grass1()))/100;
                                    int thisgrass2=((monsoon*100*monsooncol1)+(notmonsoon*100*world.grass2()))/100;
                                    int thisgrass3=((monsoon*100*monsooncol1)+(notmonsoon*100*world.grass3()))/100;
                                    */
                                    
                                    newgrass1=(highno*world.highbase1()+lowno*world.grass1())/40;
                                    newgrass2=(highno*world.highbase2()+lowno*world.grass2())/40;
                                    newgrass3=(highno*world.highbase3()+lowno*world.grass3())/40;
                                }

                                // Now we need to mix these according to how dry the location is.
                                
                                if (totalrain>800) // 800
                                {
                                    colour1=newbase1;
                                    colour2=newbase2;
                                    colour3=newbase3;
                                }
                                else
                                { // That's it!
                                    if (totalrain>200) //400
                                    {
                                        int wetno=(totalrain-200)/40; //400 20
                                        if (wetno>20)
                                            wetno=20;
                                        int dryno=20-wetno;
                                        
                                        colour1=(wetno*newbase1+dryno*newgrass1)/20;
                                        colour2=(wetno*newbase2+dryno*newgrass2)/20;
                                        colour3=(wetno*newbase3+dryno*newgrass3)/20;
                                    }
                                    else
                                    {
                                        float ftotalrain=200-totalrain; // 400
                                        
                                        ftotalrain=ftotalrain/200.0; // 400
                                        
                                        int powamount=totalrain-150; // 350 This is to make a smoother transition.
                                        
                                        if (powamount<3)
                                            powamount=3;
                                        
                                        ftotalrain=pow(ftotalrain,powamount);
                                        
                                        ftotalrain=ftotalrain*200; // 400
                                        
                                        totalrain=200-ftotalrain; // 400
                                        
                                        int wetno=totalrain;
                                        int dryno=200-wetno;
                                        
                                        colour1=(wetno*newgrass1+dryno*newdesert1)/200;
                                        colour2=(wetno*newgrass2+dryno*newdesert2)/200;
                                        colour3=(wetno*newgrass3+dryno*newdesert3)/200;
                                    }
                                }

                                // Now we need to alter that according to how cold the location is.
                                
                                if (avetemp<=0)
                                {
                                    colour1=world.cold1();
                                    colour2=world.cold2();
                                    colour3=world.cold3();
                                }
                                else
                                {
                                    // Get the right tundra colour, depending on latitude.
                                    
                                    int lat=world.latitude(i,j);
                                    int lat2=90-lat;
                                    
                                    int thistundra1=(lat*world.tundra1()+lat2*world.eqtundra1())/90;
                                    int thistundra2=(lat*world.tundra2()+lat2*world.eqtundra2())/90;
                                    int thistundra3=(lat*world.tundra3()+lat2*world.eqtundra3())/90;
                                    
                                    if (world.snowchange()==1) // Abrupt transition
                                    {
                                        if (avetemp<20)
                                        {
                                            if (avetemp<6)
                                            {
                                                colour1=world.cold1();
                                                colour2=world.cold2();
                                                colour3=world.cold3();
                                            }
                                            else
                                            {
                                                if (avetemp<10)
                                                {
                                                    colour1=thistundra1;
                                                    colour2=thistundra2;
                                                    colour3=thistundra3;
                                                }
                                                else
                                                {
                                                    int hotno=avetemp-10;
                                                    int coldno=10-hotno;
                                                    
                                                    colour1=(hotno*colour1+coldno*thistundra1)/10;
                                                    colour2=(hotno*colour2+coldno*thistundra2)/10;
                                                    colour3=(hotno*colour3+coldno*thistundra3)/10;
                                                }
                                            }
                                        }
                                    }
                                    
                                    if (world.snowchange()==2) // Speckled transition
                                    {
                                        if (avetemp<20)
                                        {
                                            if (avetemp<6)
                                            {
                                                colour1=world.cold1();
                                                colour2=world.cold2();
                                                colour3=world.cold3();
                                            }
                                            else
                                            {
                                                if (avetemp<10)
                                                {
                                                    if (random(6,10)<avetemp)
                                                    {
                                                        colour1=thistundra1;
                                                        colour2=thistundra2;
                                                        colour3=thistundra3;
                                                    }
                                                    else
                                                    {
                                                        colour1=world.cold1();
                                                        colour2=world.cold2();
                                                        colour3=world.cold3();
                                                    }
                                                }
                                                else
                                                {
                                                    int hotno=avetemp-10;
                                                    int coldno=10-hotno;
                                                    
                                                    colour1=(hotno*colour1+coldno*thistundra1)/10;
                                                    colour2=(hotno*colour2+coldno*thistundra2)/10;
                                                    colour3=(hotno*colour3+coldno*thistundra3)/10;
                                                }
                                            }
                                        }
                                    }
                                    
                                    if (world.snowchange()==3) // Gradual transition
                                    {
                                        if (avetemp<20)
                                        {
                                            if (avetemp<10)
                                            {
                                                int hotno=avetemp;
                                                int coldno=10-hotno;
                                                
                                                colour1=(hotno*thistundra1+coldno*world.cold1())/10;
                                                colour2=(hotno*thistundra2+coldno*world.cold2())/10;
                                                colour3=(hotno*thistundra3+coldno*world.cold3())/10;
                                            }
                                            else
                                            {
                                                int hotno=avetemp-10;
                                                int coldno=10-hotno;
                                                
                                                colour1=(hotno*colour1+coldno*thistundra1)/10;
                                                colour2=(hotno*colour2+coldno*thistundra2)/10;
                                                colour3=(hotno*colour3+coldno*thistundra3)/10;
                                            }
                                        }
                                    }
                                }
                                
                                // Now add sand, if need be.
                                
                                int special=world.special(i,j);
                                
                                if (special==120)
                                {
                                    colour1=(colour1*2+world.erg1())/3;
                                    colour2=(colour2*2+world.erg2())/3;
                                    colour3=(colour3*2+world.erg3())/3;
                                }
                                
                                // Now wetlands.
                                
                                if (special>=130 && special<140)
                                {
                                    colour1=(colour1*2+world.wetlands1())/3;
                                    colour2=(colour2*2+world.wetlands2())/3;
                                    colour3=(colour3*2+world.wetlands3())/3;
                                }
                            }
                        }
                    }
                    
                    if (world.sea(i,j)==1)
                    {
                        int amount=randomsign(random(0,var));
                        
                        colour1=colour1+amount;
                        colour2=colour2+amount;
                        colour3=colour3+amount;
                    }
                    else
                    {
                        colour1=colour1+randomsign(random(0,var));
                        colour2=colour2+randomsign(random(0,var));
                        colour3=colour3+randomsign(random(0,var));
                        
                        if (world.truelake(i,j)!=0)
                        {
                            colour1=world.lake1();
                            colour2=world.lake2();
                            colour3=world.lake3();
                        }
                        else
                        {
                            if (world.riftlakesurface(i,j)!=0)
                            {
                                colour1=world.lake1();
                                colour2=world.lake2();
                                colour3=world.lake3();
                            }
                        }
                    }
                    
                    if (colour1>255)
                        colour1=255;
                    if (colour2>255)
                        colour2=255;
                    if (colour3>255)
                        colour3=255;
                    
                    if (colour1<0)
                        colour1=0;
                    if (colour2<0)
                        colour2=0;
                    if (colour3<0)
                        colour3=0;
                    
                    reliefmap1[i][j]=colour1;
                    reliefmap2[i][j]=colour2;
                    reliefmap3[i][j]=colour3;
                }
            }
            
            // Now apply that to the image, adding shading for slopes where appropriate.
            
            short r, g, b;
            
            for (int i=0; i<=width; i++)
            {
                for (int j=0; j<=height; j++)
                {
                    r=reliefmap1[i][j];
                    g=reliefmap2[i][j];
                    b=reliefmap3[i][j];
                    
                    if (world.noshade(i,j)==0)
                    {
                        bool goahead=1;
                        
                        if ((world.seaice(i,j)==2 && (world.seaiceappearance()==1 || world.seaiceappearance()==3)) || (world.seaice(i,j)==1 && world.seaiceappearance()==3)) // Sea ice
                            goahead=0;

                        if (goahead==1 || world.sea(i,j)==0)
                        {
                            int slope1, slope2;
                            
                            if (shadingdir==2)
                            {
                                slope1=getslope(world,i,j,i-1,j);
                                slope2=getslope(world,i,j,i,j+1);
                            }
                            
                            if (shadingdir==4)
                            {
                                slope1=getslope(world,i,j,i-1,j);
                                slope2=getslope(world,i,j,i,j-1);
                            }
                            
                            if (shadingdir==6)
                            {
                                slope1=getslope(world,i,j,i+1,j);
                                slope2=getslope(world,i,j,i,j-1);
                            }
                            
                            if (shadingdir==8)
                            {
                                slope1=getslope(world,i,j,i+1,j);
                                slope2=getslope(world,i,j,i,j+1);
                            }
                            
                            if (slope1!=-1 && slope2!=-1)
                            {
                                int totalslope=(slope1+slope2)/10;
                                
                                if (totalslope>40)
                                    totalslope=40;
                                
                                if (totalslope<-40)
                                    totalslope=-40;
                                
                                if (world.sea(i,j)==1)
                                    totalslope=totalslope*seashading*2;
                                else
                                {
                                    if (world.truelake(i,j)==1)
                                        totalslope=totalslope*lakeshading*2;
                                    else
                                        totalslope=totalslope*landshading*2;
                                    
                                }
                                
                                if (world.map(i,j)<=sealevel && world.oceanrifts(i,j)==0) // Reduce the shading effect around ocean ridges.
                                {
                                    int amount=1;
                                    int amount2=3;
                                    bool found=0;
                                    bool ignore=0;
                                    
                                    for (int k=i-amount2; k<=i+amount2; k++) // don't do this around the rift itself
                                    {
                                        int kk=k;
                                        
                                        if (kk<0 || kk>width)
                                            kk=wrap(kk,width);
                                        
                                        for (int l=j-amount2; l<=j+amount2; l++)
                                        {
                                            if (l>=0 && l<=height)
                                            {
                                                if (world.oceanrifts(kk,l)!=0)
                                                {
                                                    ignore=1;
                                                    k=i+amount2;
                                                    l=j+amount2;
                                                }
                                            }
                                        }
                                    }
                                    
                                    if (ignore==0)
                                    {
                                        for (int k=i-amount; k<=i+amount; k++)
                                        {
                                            int kk=k;
                                            
                                            if (kk<0 || kk>width)
                                                kk=wrap(kk,width);
                                            
                                            for (int l=j-amount; l<=j+amount; l++)
                                            {
                                                if (l>=0 && l<=height)
                                                {
                                                    if (world.oceanridges(kk,l)!=0)
                                                    {
                                                        found=1;
                                                        k=i+amount;
                                                        l=j+amount;
                                                    }
                                                }
                                            }
                                        }

                                        if (found==1)
                                            totalslope=totalslope/4;
                                    }
                                }
                                

                                r=r+totalslope;
                                g=g+totalslope;
                                b=b+totalslope;
                            }
                            
                            if (r<0)
                                r=0;
                            if (g<0)
                                g=0;
                            if (b<0)
                                b=0;
                            
                            if (r>255)
                                r=255;
                            if (g>255)
                                g=255;
                            if (b>255)
                                b=255;
                        }
                    }
                    
                    index=(i+j*globalimagewidth)*globalimagechannels;
                    
                    globalreliefimage[index]=r;
                    globalreliefimage[index+1]=g;
                    globalreliefimage[index+2]=b;
                    
                }
            }
            
            globalmapimagecreated[6]=1;
            
            break;
        }
    }
}

// This function draws a regional map image (ready to be applied to a texture).

void drawregionalmapimage(mapviewenum mapview, planet &world, region &region, bool regionalmapimagecreated[], stbi_uc regionalelevationimage[], stbi_uc regionaltemperatureimage[], stbi_uc regionalprecipitationimage[],  stbi_uc regionalclimateimage[], stbi_uc regionalriversimage[], stbi_uc regionalreliefimage[], int regionalimagewidth, int regionalimageheight, int regionalimagechannels)
{
    // mapview tells us which of these we're actually drawing.
    
    int origregwidthbegin=region.regwidthbegin();
    int origregwidthend=region.regwidthend();
    int origregheightbegin=region.regheightbegin();
    int origregheightend=region.regheightend();
    
    int regwidthbegin=origregwidthbegin;
    int regwidthend=origregwidthend;
    int regheightbegin=origregheightbegin;
    int regheightend=origregheightend;
    
    int regionalimagesize=regionalimagewidth*regionalimageheight*regionalimagechannels;

    int colour1, colour2, colour3, index;
    
    switch (mapview)
    {
        case elevation:
        {
            if (regionalmapimagecreated[0]==1)
                return;
            
            int div=world.maxelevation()/255;
            int base=world.maxelevation()/4;

            for (int i=regwidthbegin; i<=regwidthend; i++)
            {
                for (int j=regheightbegin; j<=regheightend; j++)
                {
                    int heightpoint=region.map(i,j);
                    
                    if (region.special(i,j)>100 && region.special(i,j)<130)
                        heightpoint=region.lakesurface(i,j);

                    colour1=heightpoint/div;
                    
                    if (colour1>255)
                        colour1=255;
                    
                    colour2=colour1;
                    colour3=colour2;
                    
                    index=((i-origregwidthbegin)+(j-origregheightbegin)*regionalimagewidth)*regionalimagechannels;
                    
                    regionalelevationimage[index]=colour1;
                    regionalelevationimage[index+1]=colour2;
                    regionalelevationimage[index+2]=colour3;
                }
            }
            
            regionalmapimagecreated[0]=1;
            
            break;
        }

        case winds:
            break;
            
        case temperature:
        {
            if (regionalmapimagecreated[2]==1)
                return;
            
            for (int i=regwidthbegin; i<=regwidthend; i++)
            {
                for (int j=regheightbegin; j<=regheightend; j++)
                {
                    if (region.outline(i,j))
                    {
                        colour1=0;
                        colour2=0;
                        colour3=0;
                    }
                    else
                    {
                        int temperature=(region.mintemp(i,j)+region.maxtemp(i,j))/2;
                        
                        temperature=temperature+10;
                        
                        if (temperature>0)
                        {
                            colour1=250;
                            colour2=250-(temperature*3);
                            colour3=250-(temperature*7);
                        }
                        else
                        {
                            temperature=abs(temperature);
                            
                            colour1=250-(temperature*7);
                            colour2=250-(temperature*7);
                            colour3=250;
                        }
                        
                        if (colour1<0)
                            colour1=0;
                        
                        if (colour2<0)
                            colour2=0;
                        
                        if (colour3<0)
                            colour3=0;
                    }
                    
                    index=((i-origregwidthbegin)+(j-origregheightbegin)*regionalimagewidth)*regionalimagechannels;
                    
                    regionaltemperatureimage[index]=colour1;
                    regionaltemperatureimage[index+1]=colour2;
                    regionaltemperatureimage[index+2]=colour3;
                }
            }

            regionalmapimagecreated[2]=1;
            
            break;
        }
            
        case precipitation:
        {
            if (regionalmapimagecreated[3]==1)
                return;
            
            for (int i=regwidthbegin; i<=regwidthend; i++)
            {
                for (int j=regheightbegin; j<=regheightend; j++)
                {
                    if (region.outline(i,j))
                    {
                        colour1=0;
                        colour2=0;
                        colour3=0;
                    }
                    else
                    {
                        int rainfall=(region.summerrain(i,j)+region.winterrain(i,j))/2;
                        
                        rainfall=rainfall/4;
                        
                        colour1=255-rainfall;
                        colour2=255-rainfall;
                        colour3=255;
                        
                        if (colour1<0)
                            colour1=0;
                        
                        if (colour2<0)
                            colour2=0;
                    }
                    
                    /*
                    float ii=i; // Add a grid.
                    
                    float jj=j;
                    
                    if (i/16==ii/16 || j/16==jj/16)
                    {
                        colour1=250;
                        colour2=100;
                        colour3=50;
                    }
                    
                    if (region.test(i,j)!=0)
                    {
                        colour1=255;
                        colour2=0;
                        colour3=255;
                    }
                    */

                    index=((i-origregwidthbegin)+(j-origregheightbegin)*regionalimagewidth)*regionalimagechannels;
                    
                    regionalprecipitationimage[index]=colour1;
                    regionalprecipitationimage[index+1]=colour2;
                    regionalprecipitationimage[index+2]=colour3;
                }
            }
            
            regionalmapimagecreated[3]=1;
            
            break;
        }
            
        case climate:
        {
            if (regionalmapimagecreated[4]==1)
                return;
            
            for (int i=regwidthbegin; i<=regwidthend; i++)
            {
                for (int j=regheightbegin; j<=regheightend; j++)
                {
                    if (region.sea(i,j))
                    {
                        if (region.seaice(i,j)==2) // Permanent sea ice
                        {
                            colour1=243;
                            colour2=243;
                            colour3=255;
                        }
                        else
                        {
                            if (region.seaice(i,j)==1) // Seasonal sea ice
                            {
                                colour1=228;
                                colour2=228;
                                colour3=255;
                            }
                            else // Open sea
                            {
                                colour1=13;
                                colour2=49;
                                colour3=109;
                            }
                        }
                        
                        index=((i-origregwidthbegin)+(j-origregheightbegin)*regionalimagewidth)*regionalimagechannels;
                        
                        regionalclimateimage[index]=colour1;
                        regionalclimateimage[index+1]=colour2;
                        regionalclimateimage[index+2]=colour3;
                    }
                    else
                    {
                        sf::Color landcolour=getclimatecolours(region.climate(i,j));
                        
                        colour1=landcolour.r;
                        colour2=landcolour.g;
                        colour3=landcolour.b;
                        
                        index=((i-origregwidthbegin)+(j-origregheightbegin)*regionalimagewidth)*regionalimagechannels;
                        
                        regionalclimateimage[index]=colour1;
                        regionalclimateimage[index+1]=colour2;
                        regionalclimateimage[index+2]=colour3;
                    }
                }
            }
            
            regionalmapimagecreated[4]=1;
            
            break;
        }
            
        case rivers:
        {
            if (regionalmapimagecreated[5]==1)
                return;
            
            int mult=world.maxriverflow()/400;
            
            if (mult<1)
                mult=1;
            
            for (int i=regwidthbegin; i<=regwidthend; i++)
            {
                for (int j=regheightbegin; j<=regheightend; j++)
                {
                    if (region.outline(i,j))
                    {
                        colour1=0;
                        colour2=0;
                        colour3=0;
                    }
                    else
                    {
                        int flow=region.riveraveflow(i,j);
                        
                        if (flow>0 && region.sea(i,j)==0)
                        {
                            
                            flow=flow*100;
                            
                            colour1=255-(flow/mult);
                            if (colour1<0)
                                colour1=0;
                            colour2=colour1;
                        }
                        else
                        {
                            colour1=255;
                            colour2=255;
                        }
                        
                        colour3=255;
                        
                        if (region.truelake(i,j)!=0)
                        {
                            colour1=150;
                            colour2=150;
                            colour3=250;
                        }
                        
                        if (region.volcano(i,j)==1)
                        {
                            colour1=240;
                            colour2=0;
                            colour3=0;
                        }

                         if (colour1==255 && colour2==255 && colour3==255)
                         {
                             int special=region.special(i,j);
                             
                             if (special>=130 && special<140)
                             {
                                 colour1=30;
                                 colour2=250;
                                 colour3=150;
                             }
                         }
                    }

                    index=((i-origregwidthbegin)+(j-origregheightbegin)*regionalimagewidth)*regionalimagechannels;
                    
                    regionalriversimage[index]=colour1;
                    regionalriversimage[index+1]=colour2;
                    regionalriversimage[index+2]=colour3;
                }
            }
            
            regionalmapimagecreated[5]=1;
            
            break;
        }
            
        case relief:
        {
            if (regionalmapimagecreated[6]==1)
                return;
            
            int regwidthbegin=region.regwidthbegin();
            
            int leftx=region.leftx();
            int lefty=region.lefty();
            
            int rwidth=region.rwidth();
            int rheight=region.rheight();
            
            int sealevel=world.sealevel();
            int minriverflow=world.minriverflowregional(); // Rivers of this size or larger will be shown on the map.
            int shadingdir=world.shadingdir();
            
            float landshading=world.landshading();
            float lakeshading=world.lakeshading();
            float seashading=world.seashading();
            
            float landmarbling=world.landmarbling()*2;
            float lakemarbling=world.lakemarbling()*2;
            float seamarbling=world.seamarbling()*2;
            
            int width=world.width();
            int height=world.height();
            int xleft=0;
            int xright=35;
            int ytop=0;
            int ybottom=35;
            
            vector<vector<short>> reliefmap1(RARRAYWIDTH,vector<short>(RARRAYWIDTH,RARRAYHEIGHT));
            vector<vector<short>> reliefmap2(RARRAYWIDTH,vector<short>(RARRAYWIDTH,RARRAYHEIGHT));
            vector<vector<short>> reliefmap3(RARRAYWIDTH,vector<short>(RARRAYWIDTH,RARRAYHEIGHT));

            // Make a fractal based on rainfall, which will be used to add stripes to vary the colours.
            
            vector<vector<int>> source(ARRAYWIDTH,vector<int>(ARRAYWIDTH,ARRAYHEIGHT));
            vector<vector<int>> stripefractal(RARRAYWIDTH,vector<int>(RARRAYWIDTH,RARRAYHEIGHT));
            
            int coords[4][2];
            
            for (int i=0; i<RARRAYWIDTH; i++)
            {
                for (int j=0; j<RARRAYHEIGHT; j++)
                    stripefractal[i][j]=-5000;
            }
            
            for (int i=0; i<=width; i++)
            {
                for (int j=0; j<=height; j++)
                {
                    if (world.wintermountainraindir(i,j)==0)
                        source[i][j]=world.winterrain(i,j);
                    else
                        source[i][j]=world.wintermountainrain(i,j);
                }
            }
            
            for (int x=xleft; x<=xright; x++)
            {
                int xx=leftx+x;
                
                if (xx<0 || xx>width)
                    xx=wrap(xx,width);
                
                for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
                {
                    int yy=lefty+y;
                    
                    float valuemod=0.04; //0.02;
                    
                    makegenerictile(world,x*16,y*16,xx,yy,valuemod,coords,source,stripefractal,100000,0,0);
                }
            }
            
            for (int i=0; i<RARRAYWIDTH; i++)
            {
                for (int j=0; j<=RARRAYHEIGHT; j++)
                {
                    reliefmap1[i][j]=0;
                    reliefmap2[i][j]=0;
                    reliefmap3[i][j]=0;
                }
            }
            
            int var=0; // Amount colours may be varied to make the map seem more speckledy.
            
            for (int i=regwidthbegin; i<=regwidthend; i++)
            {
                for (int j=regheightbegin; j<=regheightend; j++)
                {
                    bool stripe=1; // Indicates whether we're going to do a stripe here.
                    
                    int xx=leftx+(i/16);
                    int yy=lefty+(j/16); // Coordinates of the relevant global cell.
                    
                    int avetemp=(region.extramaxtemp(i,j)+region.extramintemp(i,j))/2;
                    
                    avetemp=avetemp+1000;
                    
                    int totalrain=region.summerrain(i,j)+region.winterrain(i,j);
                    
                    var=10;
                    
                    int sea=region.sea(i,j);
                    
                    if (sea==1)
                    {
                        if ((region.seaice(i,j)==2 && (world.seaiceappearance()==1 || world.seaiceappearance()==3)) || (region.seaice(i,j)==1 && world.seaiceappearance()==3)) // Sea ice
                        {
                            colour1=world.seaice1();
                            colour2=world.seaice2();
                            colour3=world.seaice3();
                            
                            stripe=0;
                            
                            var=0;
                        }
                        else
                        {
                            colour1=(world.ocean1()*region.map(i,j)+world.deepocean1()*(sealevel-region.map(i,j)))/sealevel;
                            colour2=(world.ocean2()*region.map(i,j)+world.deepocean2()*(sealevel-region.map(i,j)))/sealevel;
                            colour3=(world.ocean3()*region.map(i,j)+world.deepocean3()*(sealevel-region.map(i,j)))/sealevel;
                            
                            var=5;
                        }
                    }
                    else
                    {
                        if ((region.riverjan(i,j)+region.riverjul(i,j))/2>=minriverflow || (region.fakejan(i,j)+region.fakejul(i,j))/2 >=minriverflow)
                        {
                            colour1=world.river1();
                            colour2=world.river2();
                            colour3=world.river3();
                            
                            stripe=0;
                        }
                        else
                        {
                            if (region.special(i,j)==110) // Salt pan
                            {
                                colour1=world.saltpan1();
                                colour2=world.saltpan2();
                                colour3=world.saltpan3();
                                
                                var=20;
                                
                                stripe=0;
                            }
                            else
                            {
                                // First, adjust the base colours depending on temperature.
                                
                                int thisbase1, thisbase2, thisbase3, newdesert1, newdesert2, newdesert3;
                                
                                if (avetemp>3000)
                                {
                                    thisbase1=world.base1();
                                    thisbase2=world.base2();
                                    thisbase3=world.base3();
                                }
                                else
                                {
                                    int hotno=avetemp/3;
                                    int coldno=1000-hotno;
                                    
                                    thisbase1=(hotno*world.base1()+coldno*world.basetemp1())/1000;
                                    thisbase2=(hotno*world.base2()+coldno*world.basetemp2())/1000;
                                    thisbase3=(hotno*world.base3()+coldno*world.basetemp3())/1000;
                                }
                                
                                if (avetemp>30)
                                {
                                    newdesert1=world.desert1();
                                    newdesert2=world.desert2();
                                    newdesert3=world.desert3();
                                }
                                else
                                {
                                    if (avetemp<=10)
                                    {
                                        newdesert1=world.colddesert1();
                                        newdesert2=world.colddesert2();
                                        newdesert3=world.colddesert3();
                                    }
                                    else
                                    {
                                        int hotno=avetemp-10;
                                        int coldno=20-hotno;
                                        
                                        newdesert1=(hotno*world.desert1()+coldno*world.colddesert1())/20;
                                        newdesert2=(hotno*world.desert2()+coldno*world.colddesert2())/20;
                                        newdesert3=(hotno*world.desert3()+coldno*world.colddesert3())/20;
                                    }
                                }
                                
                                // The closer it is to tropical rainforest, the more we intensify the rain effect.
                                
                                float rainforestmult=region.mintemp(i,j)/18.0;
                                
                                rainforestmult=rainforestmult*region.winterrain(i,j)/80.0; //30.0;
                                
                                if (rainforestmult<1)
                                    rainforestmult=1;
                                
                                totalrain=totalrain*rainforestmult;
                                
                                // Now adjust the colours for height.
                                
                                int mapelev=region.map(i,j)-sealevel;
                                
                                if (region.special(i,j)==110 || region.special(i,j)==120) // If this is going to be an erg or salt pan
                                    mapelev=region.lakesurface(i,j)-sealevel; // Use the surface elevation of the erg or salt pan
                                
                                int newbase1, newbase2, newbase3, newgrass1, newgrass2, newgrass3;
                                
                                if (mapelev>2000)
                                {
                                    newdesert1=world.highdesert1();
                                    newdesert2=world.highdesert2();
                                    newdesert3=world.highdesert3();
                                }
                                else
                                {
                                    int highno=mapelev/50;
                                    int lowno=40-highno;
                                    
                                    newdesert1=(highno*world.highdesert1()+lowno*newdesert1)/40;
                                    newdesert2=(highno*world.highdesert2()+lowno*newdesert2)/40;
                                    newdesert3=(highno*world.highdesert3()+lowno*newdesert3)/40;
                                }
                                
                                if (mapelev>3000)
                                {
                                    newbase1=world.highbase1();
                                    newbase2=world.highbase2();
                                    newbase3=world.highbase3();
                                    
                                    newgrass1=world.highbase1();
                                    newgrass2=world.highbase2();
                                    newgrass3=world.highbase3();
                                }
                                else
                                {
                                    int highno=mapelev/75;
                                    int lowno=40-highno;
                                    
                                    newbase1=(highno*world.highbase1()+lowno*thisbase1)/40;
                                    newbase2=(highno*world.highbase2()+lowno*thisbase2)/40;
                                    newbase3=(highno*world.highbase3()+lowno*thisbase3)/40;
                                    
                                    newgrass1=(highno*world.highbase1()+lowno*world.grass1())/40;
                                    newgrass2=(highno*world.highbase2()+lowno*world.grass2())/40;
                                    newgrass3=(highno*world.highbase3()+lowno*world.grass3())/40;
                                }
                                
                                // Now we need to mix these according to how dry the location is.
                                
                                if (region.rivervalley(i,j)==1) // If this is a river valley, it's wetter than it would otherwise be.
                                {
                                    float biggestflow=0;
                                    
                                    for (int k=i-20; k<=i+20; k++)
                                    {
                                        for (int l=j-20; l<=j+20; l++)
                                        {
                                            if (k>=0 && k<=rwidth && l>=0 && l<=rheight)
                                            {
                                                if (region.riverjan(k,l)+region.riverjul(k,l)>biggestflow)
                                                    biggestflow=region.riverjan(k,l)+region.riverjul(k,l);
                                            }
                                        }
                                    }
                                    
                                    if (biggestflow==0)
                                    {
                                        sf::Vector2i nearest=findclosestriverquickly(region,i,j);
                                        
                                        if (nearest.x!=-1)
                                            biggestflow=region.riverjan(nearest.x,nearest.y)+region.riverjul(nearest.x,nearest.y);
                                    }
                                    
                                    /*
                                    if ((world.deltajan(xx,yy)+world.deltajul(xx,yy))>(world.riverjan(xx,yy)+world.riverjul(xx,yy)) && biggestflow>world.riverjan(xx,yy)+world.riverjul(xx,yy))
                                        biggestflow=world.riverjan(xx,yy)+world.riverjul(xx,yy);
                                    */
                                    
                                    if (biggestflow>12000)
                                        biggestflow=12000;
                                    
                                    float mult=totalrain;
                                    
                                    if (mult<1)
                                        mult=1;
                                    
                                    if (mult>1000)
                                        mult=1000;

                                    
                                    biggestflow=biggestflow/mult;
                                    totalrain=totalrain+biggestflow;
                                }
                                
                                if (totalrain>800) // 600
                                {
                                    colour1=newbase1;
                                    colour2=newbase2;
                                    colour3=newbase3;
                                }
                                else
                                {
                                    if (totalrain>200) //300
                                    {
                                        int wetno=(totalrain-200)/40; //20
                                        if (wetno>20) // 40
                                            wetno=20;
                                        int dryno=20-wetno;

                                        
                                        colour1=(wetno*newbase1+dryno*newgrass1)/20;
                                        colour2=(wetno*newbase2+dryno*newgrass2)/20;
                                        colour3=(wetno*newbase3+dryno*newgrass3)/20;
                                    }
                                    else
                                    {
                                        float ftotalrain=200-totalrain;
                                        
                                        ftotalrain=ftotalrain/200.0;
                                        
                                        int powamount=totalrain-150; // This is to make a smoother transition.
                                        
                                        if (powamount<3)
                                            powamount=3;
                                        
                                        ftotalrain=pow(ftotalrain,powamount);
                                        
                                        ftotalrain=ftotalrain*200;
                                        
                                        totalrain=200-ftotalrain;
                                        
                                        int wetno=totalrain;
                                        int dryno=200-wetno;
                                        
                                        colour1=(wetno*newgrass1+dryno*newdesert1)/200;
                                        colour2=(wetno*newgrass2+dryno*newdesert2)/200;
                                        colour3=(wetno*newgrass3+dryno*newdesert3)/200;
                                    }
                                }
                                
                                // Now we need to alter that according to how cold the location is.
                                
                                if (avetemp<=0 || yy>height-3) // This is because it has an odd tendency to show the very southernmost tiles in non-cold colours.
                                {
                                    colour1=world.cold1();
                                    colour2=world.cold2();
                                    colour3=world.cold3();
                                }
                                else
                                {
                                    // Get the right tundra colour, depending on latitude.
                                    
                                    int lat=world.latitude(region.centrex(),region.centrey());
                                    int lat2=90-lat;
                                    
                                    int thistundra1=(lat*world.tundra1()+lat2*world.eqtundra1())/90;
                                    int thistundra2=(lat*world.tundra2()+lat2*world.eqtundra2())/90;
                                    int thistundra3=(lat*world.tundra3()+lat2*world.eqtundra3())/90;
                                    
                                    if (world.snowchange()==1) // Abrupt transition
                                    {
                                        if (avetemp<2000)
                                        {
                                            if (avetemp<600)
                                            {
                                                colour1=world.cold1();
                                                colour2=world.cold2();
                                                colour3=world.cold3();
                                            }
                                            else
                                            {
                                                if (avetemp<1000)
                                                {
                                                    colour1=thistundra1;
                                                    colour2=thistundra2;
                                                    colour3=thistundra3;
                                                }
                                                else
                                                {
                                                    int hotno=avetemp-1000;
                                                    int coldno=1000-hotno;
                                                    
                                                    colour1=(hotno*colour1+coldno*thistundra1)/1000;
                                                    colour2=(hotno*colour2+coldno*thistundra2)/1000;
                                                    colour3=(hotno*colour3+coldno*thistundra3)/1000;
                                                }
                                            }
                                        }
                                    }
                                    
                                    if (world.snowchange()==2) // Speckled transition
                                    {
                                        if (avetemp<2000)
                                        {
                                            if (avetemp<600)
                                            {
                                                colour1=world.cold1();
                                                colour2=world.cold2();
                                                colour3=world.cold3();
                                            }
                                            else
                                            {
                                                if (avetemp<1000)
                                                {
                                                    if (random(600,1000)<avetemp)
                                                    {
                                                        colour1=thistundra1;
                                                        colour2=thistundra2;
                                                        colour3=thistundra3;
                                                    }
                                                    else
                                                    {
                                                        colour1=world.cold1();
                                                        colour2=world.cold2();
                                                        colour3=world.cold3();
                                                    }
                                                }
                                                else
                                                {
                                                    int hotno=avetemp-1000;
                                                    int coldno=1000-hotno;
                                                    
                                                    colour1=(hotno*colour1+coldno*thistundra1)/1000;
                                                    colour2=(hotno*colour2+coldno*thistundra2)/1000;
                                                    colour3=(hotno*colour3+coldno*thistundra3)/1000;
                                                }
                                            }
                                        }
                                    }
                                    
                                    if (world.snowchange()==3) // Gradual transition
                                    {
                                        if (avetemp<2000)
                                        {
                                            if (avetemp<1000)
                                            {
                                                int hotno=avetemp;
                                                int coldno=1000-hotno;
                                                
                                                colour1=(hotno*thistundra1+coldno*world.cold1())/1000;
                                                colour2=(hotno*thistundra2+coldno*world.cold2())/1000;
                                                colour3=(hotno*thistundra3+coldno*world.cold3())/1000;
                                            }
                                            else
                                            {
                                                int hotno=avetemp-1000;
                                                int coldno=1000-hotno;
                                                
                                                colour1=(hotno*colour1+coldno*thistundra1)/1000;
                                                colour2=(hotno*colour2+coldno*thistundra2)/1000;
                                                colour3=(hotno*colour3+coldno*thistundra3)/1000;
                                            }
                                        }
                                    }
                                }
                                
                                // Now add sand, if need be.
                                
                                int special=region.special(i,j);
                                
                                if (special==120)
                                {
                                    colour1=(colour1*6+world.erg1())/7;
                                    colour2=(colour2*6+world.erg2())/7;
                                    colour3=(colour3*6+world.erg3())/7;
                                    
                                    var=10; //4;
                                }
                                
                                // Now wetlands.
                                
                                if (special>=130 && special<140)
                                {
                                    colour1=(colour1*2+world.wetlands1())/3;
                                    colour2=(colour2*2+world.wetlands2())/3;
                                    colour3=(colour3*2+world.wetlands3())/3;
                                }
                            }
                        }
                    }
                    
                    if (region.sea(i,j)==1)
                    {
                        int amount=altrandomsign(altrandom(0,var));
                        
                        colour1=colour1+amount;
                        colour2=colour2+amount;
                        colour3=colour3+amount;
                    }
                    else
                    {
                        colour1=colour1+altrandomsign(altrandom(0,var));
                        colour2=colour2+altrandomsign(altrandom(0,var));
                        colour3=colour3+altrandomsign(altrandom(0,var));
                        
                        if (region.truelake(i,j)!=0)
                        {
                            colour1=world.lake1();
                            colour2=world.lake2();
                            colour3=world.lake3();
                            
                            for (int k=i-1; k<=i+1; k++) // If a river is flowing into/out of the lake, make it slightly river coloured.
                            {
                                for (int l=j-1; l<=j+1; l++)
                                {
                                    if (k>=0 && k<=rwidth && l>=0 && l<=rheight)
                                    {
                                        if (region.riverdir(k,l)!=0 || region.fakedir(k,l)>0)
                                        {
                                            if (region.lakesurface(k,l)==0)
                                            {
                                                colour1=(world.lake1()+world.river1())/2;
                                                colour2=(world.lake2()+world.river2())/2;
                                                colour3=(world.lake3()+world.river3())/2;
                                                
                                                k=i+1;
                                                l=j+1;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    
                    if (region.special(i,j)==140)
                    {
                        colour1=world.glacier1();
                        colour2=world.glacier2();
                        colour3=world.glacier3();
                        
                        stripe=0;
                    }
                    
                    if (stripe==1) // If we're doing a stripe here.
                    {
                        //fast_srand(region.maxtemp(i,j)+region.winterrain(i,j));
                        fast_srand(region.maxtemp(i,j)+stripefractal[i][j]);
                        
                        int stripevar=avetemp;
                        
                        if (stripevar>5)
                            stripevar=5;
                        
                        if (stripevar<0)
                            stripevar=0;
                        
                        if (region.special(i,j)>100)
                            stripevar=1;
                        
                        if (region.sea(i,j)==1)
                            stripevar=stripevar*seamarbling;
                        else
                        {
                            if (region.truelake(i,j)==1)
                                stripevar=stripevar*lakemarbling;
                            else
                                stripevar=stripevar*landmarbling;
                        }
                        
                        colour1=colour1+randomsign(random(0,stripevar));
                        colour2=colour2+randomsign(random(0,stripevar));
                        colour2=colour2+randomsign(random(0,stripevar));
                    }
                    
                    if (colour1>255)
                        colour1=255;
                    if (colour2>255)
                        colour2=255;
                    if (colour3>255)
                        colour3=255;
                    
                    if (colour1<0)
                        colour1=0;
                    if (colour2<0)
                        colour2=0;
                    if (colour3<0)
                        colour3=0;
                    
                    reliefmap1[i][j]=colour1;
                    reliefmap2[i][j]=colour2;
                    reliefmap3[i][j]=colour3;
                }
            }
            
            for (int i=regwidthbegin+1; i<=regwidthend-1; i++) // Blur ergs and wetlands, and also river valleys..
            {
                for (int j=regheightbegin+1; j<=regheightend-1; j++)
                {
                    int special=region.special(i,j);
                    
                    if (special==130 || special==131 || special==132) // special==120 ||
                    {
                        if ((region.riverjan(i,j)+region.riverjul(i,j))/2<minriverflow || (region.fakejan(i,j)+region.fakejul(i,j))/2 <minriverflow) // Don't do it to the rivers themselves.
                        {
                            float colred=0;
                            float colgreen=0;
                            float colblue=0;
                            
                            for (int k=i-1; k<=i+1; k++)
                            {
                                for (int l=j-1; l<=j+1; l++)
                                {
                                    colred=colred+reliefmap1[k][l];
                                    colgreen=colgreen+reliefmap2[k][l];
                                    colblue=colblue+reliefmap3[k][l];
                                }
                            }
                            
                            colred=colred/9.0;
                            colgreen=colgreen/9.0;
                            colblue=colblue/9.0;
                            
                            reliefmap1[i][j]=colred;
                            reliefmap2[i][j]=colgreen;
                            reliefmap3[i][j]=colblue;
                        }
                    }
                    
                    if (region.rivervalley(i,j)==1 && region.special(i,j)<130)
                    {
                        if (not((region.riverjan(i,j)+region.riverjul(i,j))/2>=minriverflow || (region.fakejan(i,j)+region.fakejul(i,j))/2 >=minriverflow))
                        {
                            float colred=0;
                            float colgreen=0;
                            float colblue=0;
                            
                            float crount=0;
                            
                            for (int k=i-1; k<=i+1; k++)
                            {
                                for (int l=j-1; l<=j+1; l++)
                                {
                                    if (region.riverjan(k,l)==0 && region.riverjul(k,l)==0 && region.fakejan(k,l)==0 && region.fakejul(k,l)==0 && region.deltajan(k,l)==0 && region.deltajul(k,l)==0)
                                    {
                                        colred=colred+reliefmap1[k][l];
                                        colgreen=colgreen+reliefmap2[k][l];
                                        colblue=colblue+reliefmap3[k][l];
                                        
                                        crount++;
                                    }
                                }
                            }
                            
                            colred=colred/crount;
                            colgreen=colgreen/crount;
                            colblue=colblue/crount;
                            
                            reliefmap1[i][j]=colred;
                            reliefmap2[i][j]=colgreen;
                            reliefmap3[i][j]=colblue;
                        }
                    }
                }
            }
            
            // Do the rivers again, as they might have got messed up by the blurring.
            
            for (int i=regwidthbegin; i<=regwidthend; i++)
            {
                for (int j=regheightbegin; j<=regheightend; j++)
                {
                    if (region.sea(i,j)==0 && region.special(i,j)<130 && region.truelake(i,j)==0)
                    {
                        if ((region.riverjan(i,j)+region.riverjul(i,j))/2>=minriverflow || (region.fakejan(i,j)+region.fakejul(i,j))/2 >=minriverflow)
                        {
                            colour1=world.river1();
                            colour2=world.river2();
                            colour3=world.river3();
                            
                            fast_srand(region.maxtemp(i,j)+region.winterrain(i,j));
                            
                            int stripevar=5;
                            
                            colour1=colour1+randomsign(random(0,stripevar));
                            colour2=colour2+randomsign(random(0,stripevar));
                            colour2=colour2+randomsign(random(0,stripevar));
                            
                            
                            reliefmap1[i][j]=colour1;
                            reliefmap2[i][j]=colour2;
                            reliefmap3[i][j]=colour3;
                        }
                    }
                }
            }
            
            // Now apply that to the image, adding shading for slopes where appropriate.
            
            short r, g, b;
            
            for (int i=regwidthbegin; i<=regwidthend; i++)
            {
                for (int j=regheightbegin; j<=regheightend; j++)
                {
                    r=reliefmap1[i][j];
                    g=reliefmap2[i][j];
                    b=reliefmap3[i][j];
                    
                    if (region.special(i,j)==0 && region.riverdir(i,j)==0 && region.fakedir(i,j)==0)
                    {
                        bool goahead=1;
                        
                        if ((region.seaice(i,j)==2 && (world.seaiceappearance()==1 || world.seaiceappearance()==3)) || (region.seaice(i,j)==1 && world.seaiceappearance()==3)) // Sea ice
                            goahead=0;
                        
                        if (goahead==1 || (region.sea(i,j)==0))
                        {
                            int slope1, slope2;
                            
                            if (shadingdir==2)
                            {
                                slope1=getslope(region,i,j,i-1,j);
                                slope2=getslope(region,i,j,i,j+1);
                            }
                            
                            if (shadingdir==4)
                            {
                                slope1=getslope(region,i,j,i-1,j);
                                slope2=getslope(region,i,j,i,j-1);
                            }
                            
                            if (shadingdir==6)
                            {
                                slope1=getslope(region,i,j,i+1,j);
                                slope2=getslope(region,i,j,i,j-1);
                            }
                            
                            if (shadingdir==8)
                            {
                                slope1=getslope(region,i,j,i+1,j);
                                slope2=getslope(region,i,j,i,j+1);
                            }
                            
                            if (slope1!=-1 && slope2!=-1)
                            {
                                int totalslope=(slope1+slope2)/10;
                                
                                if (totalslope>40)
                                    totalslope=40;
                                
                                if (totalslope<-40)
                                    totalslope=-40;
                                
                                float thisshading=landshading;
                                
                                if (region.truelake(i,j)==1)
                                    thisshading=lakeshading;
                                
                                if (region.sea(i,j)==1)
                                    thisshading=seashading;
                                
                                totalslope=totalslope*thisshading*2;
                                
                                r=r+totalslope;
                                g=g+totalslope;
                                b=b+totalslope;
                            }
           
                            if (r<0)
                                r=0;
                            if (g<0)
                                g=0;
                            if (b<0)
                                b=0;
                            
                            if (r>255)
                                r=255;
                            if (g>255)
                                g=255;
                            if (b>255)
                                b=255;
                        }
                    }
                    
                    colour1=r;
                    colour2=g;
                    colour3=b;
                    
                    index=((i-origregwidthbegin)+(j-origregheightbegin)*regionalimagewidth)*regionalimagechannels;
                    
                    regionalreliefimage[index]=colour1;
                    regionalreliefimage[index+1]=colour2;
                    regionalreliefimage[index+2]=colour3;
                }
            }
            
            regionalmapimagecreated[6]=1;
            
            break;
        }
    }
}

// This function clears the regional relief map image (ready to be applied to a texture).

void blankregionalreliefimage(region &region, stbi_uc regionalreliefimage[], int regionalimagewidth, int regionalimageheight, int regionalimagechannels)
{
    int origregwidthbegin=region.regwidthbegin();
    int origregwidthend=region.regwidthend();
    int origregheightbegin=region.regheightbegin();
    int origregheightend=region.regheightend();
    
    int regwidthbegin=origregwidthbegin;
    int regwidthend=origregwidthend;
    int regheightbegin=origregheightbegin;
    int regheightend=origregheightend;
    
    int regionalimagesize=regionalimagewidth*regionalimageheight*regionalimagechannels;
    
    int index;
    
    for (int i=regwidthbegin; i<=regwidthend; i++)
    {
        for (int j=regheightbegin; j<=regheightend; j++)
        {
            index=((i-origregwidthbegin)+(j-origregheightbegin)*regionalimagewidth)*regionalimagechannels;
            
            regionalreliefimage[index]=0;
            regionalreliefimage[index+1]=0;
            regionalreliefimage[index+2]=0;
        }
    }
}

void addgridlines(sf::Image &mapimage)
{
    sf::Vector2u size=mapimage.getSize();
    
    for (int i=0; i<size.x; i=i+16)
    {
        for (int j=0; j<size.y; j++)
            mapimage.setPixel(i,j,sf::Color(80,50,20));
    }
    
    for (int j=0; j<size.y; j=j+16)
    {
        for (int i=0; i<size.x; i++)
            mapimage.setPixel(i,j,sf::Color(80,50,20));
    }
}

// This function shifts the regional map image.

void shiftregionalmapimage(region &region, sf::Image &image, int shifting)
{
    sf::Vector2u size=image.getSize();
    
    int leftx=0;
    int lefty=0;
    int rightx=size.x-1;
    int righty=size.y-1;
    
    if (shifting==1) // shifting it downwards
    {
        lefty=lefty+16;
        
        for (int i=leftx; i<=rightx; i++)
        {
            for (int j=righty; j>=lefty; j--)
                image.setPixel(i,j,image.getPixel(i,j-16));
        }
    }
    
    if (shifting==3) // shifting it to the left
    {
        rightx=rightx-16;
        
        for (int i=leftx; i<=rightx; i++)
        {
            for (int j=lefty; j<=righty; j++)
                image.setPixel(i,j,image.getPixel(i+16,j));
        }
    }
    
    if (shifting==5) // shifting it upwards
    {
        righty=righty-16;
        
        for (int i=leftx; i<=rightx; i++)
        {
            for (int j=lefty; j<=righty; j++)
                image.setPixel(i,j,image.getPixel(i,j+16));
        }
    }
    
    if (shifting==7) // shifting it to the right
    {
        leftx=leftx+16;
        
        for (int i=rightx; i>=leftx; i--)
        {
            for (int j=lefty; j<=righty; j++)
                image.setPixel(i,j,image.getPixel(i-16,j));
        }
    }
}



// Utility function to display template images.

void displaytemplates(planet &world, stbi_uc globalreliefimage[], int globalimagewidth, int globalimagechannels, boolshapetemplate shape[])
{
    int width=world.width();
    int height=world.height();
    
    int index=0;;
    
    int xstart=0;
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            index=(i+j*globalimagewidth)*globalimagechannels;
            
            globalreliefimage[index]=0;
            globalreliefimage[index+1]=0;
            globalreliefimage[index+2]=0;
        }
    }

    for (int n=0; n<12; n++)
    {
        int xsize=shape[n].xsize();
        int ysize=shape[n].ysize();
        
        cout << "Doing shape number: " << n << " at x point: " << xstart << endl;
        
        for (int i=0; i<xsize; i++)
        {
            for (int j=0; j<ysize; j++)
            {
                if (shape[n].point(i,j)==1)
                {
                    index=(i+xstart+j*globalimagewidth)*globalimagechannels;
                    
                    globalreliefimage[index]=255;
                    globalreliefimage[index+1]=255;
                    globalreliefimage[index+2]=255;
                }
            }
        }
        
        xstart=xstart+xsize+5;
        
    }
    
}

