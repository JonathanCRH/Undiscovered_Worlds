//
//  main.cpp
//  Undiscovered Worlds
//
//  Created by Jonathan Hill on 22/07/2019.
//
//  Please see functions.hpp for notes.
//
//  This is the only file (apart from functions.hpp) that requires stbi, NanoGUI, and SFML.

#include <iostream>
#include <cmath>
#include <fstream>
#include <stdio.h>
//#include <unistd.h>
#include <string>
#include <chrono>
#include <thread>
#include <queue>
#include <stdint.h>
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
#define GLOBALCLIMATECREATIONSTEPS 28

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
void fast_srand(long seed)
{
    g_seed = seed;
}

// Compute a pseudorandom integer.
// Output value in range [0, 32767]
int fast_rand(void)
{
    g_seed = (214013*g_seed+2531011);
    return (g_seed>>16)&0x7FFF;
}

int main(int /* argc */, char ** /* argv */)
{
    nanogui::init();
    
    Vector2i globaltexturesize;
    
    globaltexturesize.x()=2048;
    globaltexturesize.y()=1026;
    
    Vector2i regionaltexturesize;
    
    regionaltexturesize.x()=514;
    regionaltexturesize.y()=514;
    
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
    
    // Now we must create the images for the different global maps.
    // These are simple, one-dimensional arrays, with four elements per pixel (RGBA).
    
    int globalimagewidth=globaltexturesize.x();
    int globalimageheight=globaltexturesize.y();
    int globalimagechannels=4;
    
    int globalimagesize=globalimagewidth*globalimageheight*globalimagechannels;
    
    uint8_t *globalelevationimage=new uint8_t[globalimagesize];
    uint8_t *globaltemperatureimage=new uint8_t[globalimagesize];
    uint8_t *globalprecipitationimage=new uint8_t[globalimagesize];
    uint8_t *globalclimateimage=new uint8_t[globalimagesize];
    uint8_t *globalriversimage=new uint8_t[globalimagesize];
    uint8_t *globalreliefimage=new uint8_t[globalimagesize];
    
    for (int i=0; i<globalimagesize; i++)
    {
        globalelevationimage[i]=255;
        globaltemperatureimage[i]=255;
        globalprecipitationimage[i]=255;
        globalclimateimage[i]=255;
        globalriversimage[i]=255;
        globalreliefimage[i]=255;
    }
    
    // Now we must create the images for the different regional maps. Same thing again.
    
    int regionalimagewidth=regionaltexturesize.x();
    int regionalimageheight=regionaltexturesize.y();
    int regionalimagechannels=4;
    
    int regionalimagesize=regionalimagewidth*regionalimageheight*regionalimagechannels;
    
    uint8_t *regionalelevationimage=new uint8_t[regionalimagesize];
    uint8_t *regionaltemperatureimage=new uint8_t[regionalimagesize];
    uint8_t *regionalprecipitationimage=new uint8_t[regionalimagesize];
    uint8_t *regionalclimateimage=new uint8_t[regionalimagesize];
    uint8_t *regionalriversimage=new uint8_t[regionalimagesize];
    uint8_t *regionalreliefimage=new uint8_t[regionalimagesize];
    
    for (int i=0; i<regionalimagesize; i++)
    {
        regionalelevationimage[i]=255;
        regionaltemperatureimage[i]=255;
        regionalprecipitationimage[i]=255;
        regionalclimateimage[i]=255;
        regionalriversimage[i]=255;
        regionalreliefimage[i]=255;
    }
    
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
    
    vector<vector<float>> riftblob(riftblobsize+2,vector<float>(riftblobsize+2,0));
    
    createriftblob(riftblob,riftblobsize/2);
    
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
    
    Texture *globalmap = new Texture(
                               Texture::PixelFormat::RGBA,
                               Texture::ComponentFormat::UInt8,
                               globaltexturesize,
                               Texture::InterpolationMode::Trilinear,
                               Texture::InterpolationMode::Nearest);
    
    globalmap->upload(globalelevationimage);
    
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
    
    Texture *regionalmap = new Texture(
                                     Texture::PixelFormat::RGBA,
                                     Texture::ComponentFormat::UInt8,
                                     regionaltexturesize,
                                     Texture::InterpolationMode::Trilinear,
                                     Texture::InterpolationMode::Nearest);
    
    regionalmap->upload(regionalelevationimage);
    
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
    
    Texture *regionalminimap = new Texture(
                                     Texture::PixelFormat::RGBA,
                                     Texture::ComponentFormat::UInt8,
                                     globaltexturesize,
                                     Texture::InterpolationMode::Trilinear,
                                     Texture::InterpolationMode::Nearest);
    
    regionalmap->upload(globalelevationimage);
    
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
    
    // Set up the callbacks for the create world widgets.
    
    cancelbutton->set_callback([&world,&screen,&getseedwindow,&brandnew,&globalmapwindow,&focusinfo,&globalmapimagecreated,&mapview,&globalelevationimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&globalmapwidget,&region,&regionalminimap,&regionalminimapwidget,&regionalmapwindow,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&regionalmap,&regionalmapwidget,&oceanbutton,&deepoceanbutton,&basebutton,&grassbutton,&basetempbutton,&highbasebutton,&desertbutton,&highdesertbutton,&colddesertbutton,&eqtundrabutton,&tundrabutton,&coldbutton,&seaicebutton,&glacierbutton,&saltpanbutton,&ergbutton,&wetlandsbutton,&lakebutton,&riverbutton,&highlightbutton,&rfocused,&poix,&poiy,&focused,&colourpickerwindow,&colourbutton,&colourwheel,&redbox,&greenbox,&bluebox,&shadinglandslider,&shadinglakesslider,&shadingseaslider,&marblinglandslider,&marblinglakesslider,&marblingseaslider,&riversglobalbox,&riversregionalbox,&shadingdirectionchooser]
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
                                           drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                           
                                           
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
    
    importbutton->set_callback([&importwindow,&getseedwindow,&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&importmapwidget]
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
                                   drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                   
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
    
    OKbutton->set_callback([&seed,&seedinput,&screen,&getseedwindow,&world,&landshape,&chainland,&smalllake,&largelake,&mapview,&globalmapimagecreated,&globalelevationimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalmapwindow,&worldgenerationwindow,&generationlabel,&worldprogress,&globalmap,&globalmapwidget,&focusinfo,&globalimagewidth,&globalimageheight,&globalimagechannels,&reliefbutton,&elevationbutton,&temperaturebutton,&precipitationbutton,&climatebutton,&riversbutton]
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
                                   
                                   vector<vector<int>> mountaindrainage(ARRAYWIDTH,vector<int>(ARRAYHEIGHT,0));
                                   vector<vector<bool>> shelves(ARRAYWIDTH,vector<bool>(ARRAYHEIGHT,0));
                                   generateglobalterrain(*world,terraintype,*screen,*worldgenerationwindow,*generationlabel,*worldprogress,progressstep,landshape,chainland,mountaindrainage,shelves);
                                   
                                   generateglobalclimate(*world,*screen,*worldgenerationwindow,*generationlabel,*worldprogress,progressstep,smalllake,largelake,landshape,mountaindrainage,shelves);
                                   
                                   generationlabel->set_caption("Preparing map image");
                                   worldprogress->set_value(1.0);
                                   screen->redraw();
                                   screen->draw_all();
                                   
                                   // Now move this window away entirely and bring in the world map window, with the right map applied.
                                   
                                   mapview=relief;
                                   
                                   drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                   
                                   
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
    
    shadinglandslider->set_final_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&globalmapwidget,&region,&regionalminimap,&regionalminimapwidget,&regionalmapwindow,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&regionalmap,&regionalmapwidget,&focused,&poix,&poiy,&rfocused](float value)
                                          {
                                              world->setlandshading(value);
                                              
                                              // Redraw the global map.
                                              
                                              for (int n=0; n<GLOBALMAPTYPES; n++)
                                                  globalmapimagecreated[n]=0;
                                              drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                              
                                              
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
    
    shadinglakesslider->set_final_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&globalmapwidget,&region,&regionalminimap,&regionalminimapwidget,&regionalmapwindow,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&regionalmap,&regionalmapwidget,&focused,&poix,&poiy,&rfocused](float value)
                                           {
                                               world->setlakeshading(value);
                                               
                                               // Redraw the global map.
                                               
                                               for (int n=0; n<GLOBALMAPTYPES; n++)
                                                   globalmapimagecreated[n]=0;
                                               drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                               
                                               
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
    
    shadingseaslider->set_final_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&globalmapwidget,&region,&regionalminimap,&regionalminimapwidget,&regionalmapwindow,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&regionalmap,&regionalmapwidget,&focused,&poix,&poiy,&rfocused](float value)
                                         {
                                             world->setseashading(value);
                                             
                                             // Redraw the global map.
                                             
                                             for (int n=0; n<GLOBALMAPTYPES; n++)
                                                 globalmapimagecreated[n]=0;
                                             drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                             
                                             
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
    
    shadingdirectionchooser->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&globalmapwidget,&region,&regionalminimap,&regionalminimapwidget,&regionalmapwindow,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&regionalmap,&regionalmapwidget,&focused,&poix,&poiy,&rfocused](int value)
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
                                                  drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                                  
                                                  
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
    
    snowchangechooser->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&globalmapwidget,&region,&regionalminimap,&regionalminimapwidget,&regionalmapwindow,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&regionalmap,&regionalmapwidget,&focused,&poix,&poiy,&rfocused](int value)
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
                                            drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                            
                                            
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
    
    seaicechooser->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&globalmapwidget,&region,&regionalminimap,&regionalminimapwidget,&regionalmapwindow,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&regionalmap,&regionalmapwidget,&focused,&poix,&poiy,&rfocused](int value)
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
                                        drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                        
                                        
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
    
    marblinglandslider->set_final_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&globalmapwidget,&region,&regionalminimap,&regionalminimapwidget,&regionalmapwindow,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&regionalmap,&regionalmapwidget,&focused,&poix,&poiy,&rfocused](float value)
                                           {
                                               world->setlandmarbling(value);
                                               
                                               // Redraw the global map.
                                               
                                               for (int n=0; n<GLOBALMAPTYPES; n++)
                                                   globalmapimagecreated[n]=0;
                                               drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                               
                                               
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
    
    marblinglakesslider->set_final_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&globalmapwidget,&region,&regionalminimap,&regionalminimapwidget,&regionalmapwindow,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&regionalmap,&regionalmapwidget,&focused,&poix,&poiy,&rfocused](float value)
                                            {
                                                world->setlakemarbling(value);
                                                
                                                // Redraw the global map.
                                                
                                                for (int n=0; n<GLOBALMAPTYPES; n++)
                                                    globalmapimagecreated[n]=0;
                                                drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                                
                                                
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
    
    marblingseaslider->set_final_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&globalmapwidget,&region,&regionalminimap,&regionalminimapwidget,&regionalmapwindow,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&regionalmap,&regionalmapwidget,&focused,&poix,&poiy,&rfocused](float value)
                                          {
                                              world->setseamarbling(value);
                                              
                                              // Redraw the global map.
                                              
                                              for (int n=0; n<GLOBALMAPTYPES; n++)
                                                  globalmapimagecreated[n]=0;
                                              drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                              
                                              
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
    
    riversglobalbox->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&globalmapwidget,&region,&regionalminimap,&regionalminimapwidget,&regionalmapwindow,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&regionalmap,&regionalmapwidget,&focused,&poix,&poiy,&rfocused] (const int &v)
                                  {
                                      world->setminriverflowglobal(v);
                                      
                                      // Redraw the global map.
                                      
                                      for (int n=0; n<GLOBALMAPTYPES; n++)
                                          globalmapimagecreated[n]=0;
                                      drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                      
                                      
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
    
    riversregionalbox->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&globalmapwidget,&region,&regionalminimap,&regionalminimapwidget,&regionalmapwindow,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&regionalmap,&regionalmapwidget,&focused,&poix,&poiy,&rfocused] (const int &v)
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
    
    appearancedefaultbutton->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&globalmapwidget,&region,&regionalminimap,&regionalminimapwidget,&regionalmapwindow,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&regionalmap,&regionalmapwidget,&oceanbutton,&deepoceanbutton,&basebutton,&grassbutton,&basetempbutton,&highbasebutton,&desertbutton,&highdesertbutton,&colddesertbutton,&eqtundrabutton,&tundrabutton,&coldbutton,&seaicebutton,&glacierbutton,&saltpanbutton,&ergbutton,&wetlandsbutton,&lakebutton,&riverbutton,&highlightbutton,&rfocused,&poix,&poiy,&focused,&colourpickerwindow,&colourbutton,&colourwheel,&redbox,&greenbox,&bluebox,&shadinglandslider,&shadinglakesslider,&shadingseaslider,&marblinglandslider,&marblinglakesslider,&marblingseaslider,&riversglobalbox,&riversregionalbox,&shadingdirectionchooser,&snowchangechooser,&seaicechooser]
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
                                              drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                              
                                              
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
    
    appearanceloadbutton->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&globalmapwidget,&region,&regionalminimap,&regionalminimapwidget,&regionalmapwindow,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&regionalmap,&regionalmapwidget,&oceanbutton,&deepoceanbutton,&basebutton,&grassbutton,&basetempbutton,&highbasebutton,&desertbutton,&highdesertbutton,&colddesertbutton,&eqtundrabutton,&tundrabutton,&coldbutton,&seaicebutton,&glacierbutton,&saltpanbutton,&ergbutton,&wetlandsbutton,&lakebutton,&riverbutton,&highlightbutton,&rfocused,&poix,&poiy,&focused,&colourpickerwindow,&colourbutton,&colourwheel,&redbox,&greenbox,&bluebox,&shadinglandslider,&shadinglakesslider,&shadingseaslider,&marblinglandslider,&marblinglakesslider,&marblingseaslider,&riversglobalbox,&riversregionalbox,&shadingdirectionchooser]
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
                                               drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                               
                                               
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
    
    colourapplybutton->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&globalmapwidget,&region,&regionalminimap,&regionalminimapwidget,&regionalmapwindow,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&regionalmap,&regionalmapwidget,&focused,&poix,&poiy,&rfocused,&colourpickerwindow,&redbox,&greenbox,&bluebox,&oceanbutton,&deepoceanbutton,&basebutton,&grassbutton,&basetempbutton,&highbasebutton,&desertbutton,&highdesertbutton,&colddesertbutton,&eqtundrabutton,&tundrabutton,&coldbutton,&seaicebutton,&glacierbutton,&saltpanbutton,&ergbutton,&wetlandsbutton,&lakebutton,&riverbutton,&highlightbutton]
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
                                        drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                        
                                        
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
    
    loadworldbutton->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&globalmapwidget,&region,&regionalminimap,&regionalminimapwidget,&regionalmapwindow,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&regionalmap,&regionalmapwidget,&oceanbutton,&deepoceanbutton,&basebutton,&grassbutton,&basetempbutton,&highbasebutton,&desertbutton,&highdesertbutton,&colddesertbutton,&eqtundrabutton,&tundrabutton,&coldbutton,&seaicebutton,&glacierbutton,&saltpanbutton,&ergbutton,&wetlandsbutton,&lakebutton,&riverbutton,&highlightbutton,&rfocused,&poix,&poiy,&focused,&colourpickerwindow,&colourbutton,&colourwheel,&redbox,&greenbox,&bluebox,&shadinglandslider,&shadinglakesslider,&shadingseaslider,&marblinglandslider,&marblinglakesslider,&marblingseaslider,&riversglobalbox,&riversregionalbox,&shadingdirectionchooser,&focusinfo,&globalmapwindow]
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
                                          
                                          drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                          
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
    
    exportworldmapsbutton->set_callback([&world,&focused,&globalmapimagecreated,&mapview,&globalmap,&globalmapwidget,&focusinfo,&globalmapwindow,&globalelevationimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels]
                                        {
                                            // First, get the save directory.
                                            
                                            string filename=file_dialog({{"png", "Portable Network Graphics"}},true);
                                            
                                            if (filename!="")
                                            {
                                                // Now, draw all the maps.
                                                
                                                mapviewenum oldmapview=mapview;
                                                
                                                mapview=relief;
                                                
                                                drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                                
                                                mapview=elevation;
                                                
                                                drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                                
                                                
                                                mapview=temperature;
                                                
                                                drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                                
                                                mapview=precipitation;
                                                
                                                drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                                
                                                mapview=climate;
                                                
                                                drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                                
                                                mapview=rivers;
                                                
                                                drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                                
                                                // Now we need to convert these into actual images that we can save.
                                                
                                                int width=world->width()+1;
                                                int height=world->height()+1;
                                                
                                                filename.resize(filename.size()-4);
                                                
                                                saveimage(globalreliefimage,globalimagechannels,width,height,filename+" Relief.png");
                                                
                                                saveimage(globalelevationimage,globalimagechannels,width,height,filename+" Elevation.png");
                                                
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
    
    elevationbutton->set_callback([&globalmapwidget,&globalmap,&mapview,&world,&globalmapimagecreated,&globalelevationimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&focused,&poix,&poiy]
                                  {
                                      mapview=elevation;
                                      
                                      int r,g,b;
                                      
                                      drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                      
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
    
    temperaturebutton->set_callback([&globalmapwidget,&globalmap,&mapview,&world,&globalmapimagecreated,&globalelevationimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&focused,&poix,&poiy]
                                    {
                                        mapview=temperature;
                                        
                                        int r,g,b;
                                        
                                        drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                        
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
    
    precipitationbutton->set_callback([&globalmapwidget,&globalmap,&mapview,&world,&globalmapimagecreated,&globalelevationimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&focused,&poix,&poiy]
                                      {
                                          mapview=precipitation;
                                          
                                          int r,g,b;
                                          
                                          drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                          
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
    
    climatebutton->set_callback([&globalmapwidget,&globalmap,&mapview,&world,&globalmapimagecreated,&globalelevationimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&focused,&poix,&poiy]
                                {
                                    mapview=climate;
                                    
                                    int r,g,b;
                                    
                                    drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                    
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
    
    riversbutton->set_callback([&globalmapwidget,&globalmap,&mapview,&world,&globalmapimagecreated,&globalelevationimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&focused,&poix,&poiy]
                               {
                                   mapview=rivers;
                                   
                                   int r,g,b;
                                   
                                   drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                   
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
    
    focusbutton->set_callback([&world,&focused,&poix,&poiy,&screen,&globalmapwindow,&globalmapwindowmainbox,&mapandfocusbox,&globalmap,&globalmapwidget,&focusinfo,&globalelevationimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&mapview,&globalimagewidth,&globalimagechannels]
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
                                          
                                          // First, paint the focus point onto the currently viewed map.
                                          
                                          if (mapview==elevation)
                                          {
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
                                          }
                                          
                                          if (mapview==temperature)
                                          {
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
                                          }
                                          
                                          if (mapview==precipitation)
                                          {
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
                                          }
                                          
                                          if (mapview==climate)
                                          {
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
                                          }
                                          
                                          if (mapview==rivers)
                                          {
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
                                          }
                                          
                                          if (mapview==relief)
                                          {
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
                                          }
                                          
                                          globalmapwidget->set_image(globalmap);
                                          
                                          int sealevel=world->sealevel();
                                          
                                          string infotext; // Now do the information about this point.
                                          
                                          infotext="Location is "+to_string(poix)+", "+to_string(poiy)+". Latitude "+to_string(world->latitude(poix,poiy))+"??. ";
                                          
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
                                          
                                          infotext=infotext+"January temperature: "+to_string(world->jantemp(poix,poiy))+"??. July temperature: "+to_string(world->jultemp(poix,poiy))+"??. ";
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
                                              infotext=infotext+"River direction: "+direction+". January flow: "+to_string(world->riverjan(poix,poiy))+" m??/s. July flow: "+to_string(world->riverjul(poix,poiy))+" m??/s.\n";
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
    
    zoombutton->set_callback([&world,&region,&screen,&regionprogress,&regionalmap,&regionalmapimagecreated,&focused,&mapview,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&reliefbutton,&globalmapwindow,&regionalmapwindow,&regionalmapwidget,&regionalminimap,&regionalminimapwidget,&smalllake,&island,&peaks,&riftblob,&riftblobsize,&poix,&poiy,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&smudge,&smallsmudge,&regionalreliefbutton,&regionalelevationbutton,&regionaltemperaturebutton,&regionalprecipitationbutton,&regionalclimatebutton,&regionalriverbutton]
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
                                     
                                     generateregionalmap(*world,*region,*screen,*regionprogress,progressstep,smalllake,island,*peaks,riftblob,riftblobsize,0,smudge,smallsmudge);
                                     
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
    
    warningOKbutton->set_callback([&warningwindow,&globalmapwindow,&importwindow,&world,&mapview,&globalmapimagecreated,&globalelevationimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&importmapwidget]
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
                                      drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                      
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
                                                  
                                                  // First, paint the focus point onto the currently viewed map.
                                                  
                                                  if (mapview==elevation)
                                                  {
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
                                                  }
                                                  
                                                  if (mapview==temperature)
                                                  {
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
                                                  }
                                                  
                                                  if (mapview==precipitation)
                                                  {
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
                                                  }
                                                  
                                                  if (mapview==climate)
                                                  {
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
                                                  }
                                                  
                                                  if (mapview==rivers)
                                                  {
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
                                                  }
                                                  
                                                  if (mapview==relief)
                                                  {
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
                                                  
                                                  infotext2=infotext2+"January temperature: "+to_string(jantemp)+"??. July temperature: "+to_string(jultemp)+"??. ";
                                                  
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
                                                          infotext2=infotext2+"January flow: "+to_string(region->riverjan(poix,poiy))+" m??/s. July flow: "+to_string(region->riverjul(poix,poiy))+" m??/s. \n";
                                                      }
                                                      else
                                                      {
                                                          if (region->fakedir(poix,poiy)>0 && region->special(poix,poiy)!=140) // If there's a fake river here
                                                          {
                                                              infotext2=infotext2+"River direction: "+getdirstring(region->fakedir(poix,poiy))+". ";
                                                              infotext2=infotext2+"January flow: "+to_string(region->fakejan(poix,poiy))+" m??/s. July flow: "+to_string(region->fakejul(poix,poiy))+" m??/s. \n";
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
    
    regionalminimapfocusbutton->set_callback([&world,&region,&rfocused,&regionalmapwindow,&regionalmainbox,&regionalmapsbox,&regionalextrabitsbox,&regionalminimapwidget,&screen,&width,&height,&regionmargin,&regionalmapimagecreated,&regionprogress,&globalreliefimage,&regionalminimap,&regionalmap,&regionalmapwidget,&smalllake,&island,&peaks,&riftblob,&riftblobsize,&mapview,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalfocusinfo,&globalimagewidth,&globalimageheight,&globalimagechannels,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&poix,&poiy,&smudge,&smallsmudge]
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
                                                         
                                                         generateregionalmap(*world,*region,*screen,*regionprogress,progressstep,smalllake,island,*peaks,riftblob,riftblobsize,0,smudge,smallsmudge);
                                                         
                                                         drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                                         
                                                         int r,g,b;
                                                         
                                                         int index=((poix-region->regwidthbegin())+(poiy-region->regheightbegin())*regionalimagewidth)*regionalimagechannels;
                                                         
                                                         if (mapview==elevation)
                                                         {
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
                                                         }
                                                         
                                                         if (mapview==temperature)
                                                         {
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
                                                         }
                                                         
                                                         if (mapview==precipitation)
                                                         {
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
                                                         }
                                                         
                                                         if (mapview==climate)
                                                         {
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
                                                         }
                                                         
                                                         if (mapview==rivers)
                                                         {
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
                                                         }
                                                         
                                                         if (mapview==relief)
                                                         {
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
    
    regionalWbutton->set_callback([&world,&region,&rfocused,&regionalmapwindow,&regionalmainbox,&regionalmapsbox,&regionalextrabitsbox,&regionalminimapwidget,&screen,&width,&height,&regionmargin,&regionalmapimagecreated,&regionprogress,&globalreliefimage,&regionalminimap,&regionalmap,&regionalmapwidget,&smalllake,&island,&peaks,&riftblob,&riftblobsize,&mapview,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalfocusinfo,&globalimagewidth,&globalimageheight,&globalimagechannels,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&poix,&poiy,&smudge,&smallsmudge]
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
                                      
                                      generateregionalmap(*world,*region,*screen,*regionprogress,progressstep,smalllake,island,*peaks,riftblob,riftblobsize,0,smudge,smallsmudge);
                                      
                                      drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                      
                                      int r,g,b;
                                      
                                      int index=((poix-region->regwidthbegin())+(poiy-region->regheightbegin())*regionalimagewidth)*regionalimagechannels;
                                      
                                      if (mapview==elevation)
                                      {
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
                                      }
                                      
                                      if (mapview==temperature)
                                      {
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
                                      }
                                      
                                      if (mapview==precipitation)
                                      {
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
                                      }
                                      
                                      if (mapview==climate)
                                      {
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
                                      }
                                      
                                      if (mapview==rivers)
                                      {
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
                                      }
                                      
                                      if (mapview==relief)
                                      {
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
                                      }
                                      
                                      regionalmapwidget->set_image(regionalmap);
                                      regionalmapwidget->set_offset(Vector2f(0,0));
                                      regionalmapwidget->set_scale(1.0);
                                      
                                      regionprogress->set_value(0);
                                      
                                      if (rfocused==0)
                                          regionalfocusinfo->set_caption(" ");
                                  });
    
    regionalEbutton->set_callback([&world,&region,&rfocused,&regionalmapwindow,&regionalmainbox,&regionalmapsbox,&regionalextrabitsbox,&regionalminimapwidget,&screen,&width,&height,&regionmargin,&regionalmapimagecreated,&regionprogress,&globalreliefimage,&regionalminimap,&regionalmap,&regionalmapwidget,&smalllake,&island,&peaks,&riftblob,&riftblobsize,&mapview,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalfocusinfo,&globalimagewidth,&globalimageheight,&globalimagechannels,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&poix,&poiy,&smudge,&smallsmudge]
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
                                      
                                      generateregionalmap(*world,*region,*screen,*regionprogress,progressstep,smalllake,island,*peaks,riftblob,riftblobsize,0,smudge,smallsmudge);
                                      
                                      drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                      
                                      int r,g,b;
                                      
                                      int index=((poix-region->regwidthbegin())+(poiy-region->regheightbegin())*regionalimagewidth)*regionalimagechannels;
                                      
                                      if (mapview==elevation)
                                      {
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
                                      }
                                      
                                      if (mapview==temperature)
                                      {
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
                                      }
                                      
                                      if (mapview==precipitation)
                                      {
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
                                      }
                                      
                                      if (mapview==climate)
                                      {
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
                                      }
                                      
                                      if (mapview==rivers)
                                      {
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
                                      }
                                      
                                      if (mapview==relief)
                                      {
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
                                      }
                                      
                                      regionalmapwidget->set_image(regionalmap);
                                      regionalmapwidget->set_offset(Vector2f(0,0));
                                      regionalmapwidget->set_scale(1.0);
                                      
                                      regionprogress->set_value(0);
                                      
                                      if (rfocused==0)
                                          regionalfocusinfo->set_caption(" ");
                                  });
    
    regionalNbutton->set_callback([&world,&region,&rfocused,&regionalmapwindow,&regionalmainbox,&regionalmapsbox,&regionalextrabitsbox,&regionalminimapwidget,&screen,&width,&height,&regionmargin,&regionalmapimagecreated,&regionprogress,&globalreliefimage,&regionalminimap,&regionalmap,&regionalmapwidget,&smalllake,&island,&peaks,&riftblob,&riftblobsize,&mapview,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalfocusinfo,&globalimagewidth,&globalimageheight,&globalimagechannels,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&poix,&poiy,&smudge,&smallsmudge]
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
                                      
                                      generateregionalmap(*world,*region,*screen,*regionprogress,progressstep,smalllake,island,*peaks,riftblob,riftblobsize,0,smudge,smallsmudge);
                                      
                                      drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                      
                                      int r,g,b;
                                      
                                      int index=((poix-region->regwidthbegin())+(poiy-region->regheightbegin())*regionalimagewidth)*regionalimagechannels;
                                      
                                      if (mapview==elevation)
                                      {
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
                                      }
                                      
                                      if (mapview==temperature)
                                      {
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
                                      }
                                      
                                      if (mapview==precipitation)
                                      {
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
                                      }
                                      
                                      if (mapview==climate)
                                      {
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
                                      }
                                      
                                      if (mapview==rivers)
                                      {
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
                                      }
                                      
                                      if (mapview==relief)
                                      {
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
                                      }
                                      
                                      regionalmapwidget->set_image(regionalmap);
                                      regionalmapwidget->set_offset(Vector2f(0,0));
                                      regionalmapwidget->set_scale(1.0);
                                      
                                      regionprogress->set_value(0);
                                      
                                      if (rfocused==0)
                                          regionalfocusinfo->set_caption(" ");
                                  });
    
    regionalSbutton->set_callback([&world,&region,&rfocused,&regionalmapwindow,&regionalmainbox,&regionalmapsbox,&regionalextrabitsbox,&regionalminimapwidget,&screen,&width,&height,&regionmargin,&regionalmapimagecreated,&regionprogress,&globalreliefimage,&regionalminimap,&regionalmap,&regionalmapwidget,&smalllake,&island,&peaks,&riftblob,&riftblobsize,&mapview,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalfocusinfo,&globalimagewidth,&globalimageheight,&globalimagechannels,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&poix,&poiy,&smudge,&smallsmudge]
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
                                      
                                      generateregionalmap(*world,*region,*screen,*regionprogress,progressstep,smalllake,island,*peaks,riftblob,riftblobsize,0,smudge,smallsmudge);
                                      
                                      drawregionalmapimage(mapview,*world,*region,regionalmapimagecreated,regionalelevationimage,regionaltemperatureimage,regionalprecipitationimage,regionalclimateimage,regionalriversimage,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
                                      
                                      int r,g,b;
                                      
                                      int index=((poix-region->regwidthbegin())+(poiy-region->regheightbegin())*regionalimagewidth)*regionalimagechannels;
                                      
                                      if (mapview==elevation)
                                      {
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
                                      }
                                      
                                      if (mapview==temperature)
                                      {
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
                                      }
                                      
                                      if (mapview==precipitation)
                                      {
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
                                      }
                                      
                                      if (mapview==climate)
                                      {
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
                                      }
                                      
                                      if (mapview==rivers)
                                      {
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
                                      }
                                      
                                      if (mapview==relief)
                                      {
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
                                                     
                                                     int startpixelr=0, startpixelg=0, startpixelb=0;
                                                     
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
                                                     vector<uint8_t> northsider(areawidth);
                                                     vector<uint8_t> northsideg(areawidth);
                                                     vector<uint8_t> northsideb(areawidth);
                                                     
                                                     vector<uint8_t> southsider(areawidth);
                                                     vector<uint8_t> southsideg(areawidth);
                                                     vector<uint8_t> southsideb(areawidth);
                                                     
                                                     vector<uint8_t> eastsider(areawidth);
                                                     vector<uint8_t> eastsideg(areawidth);
                                                     vector<uint8_t> eastsideb(areawidth);
                                                     
                                                     vector<uint8_t> westsider(areawidth);
                                                     vector<uint8_t> westsideg(areawidth);
                                                     vector<uint8_t> westsideb(areawidth);
#else
                                                     uint8_t northsider[areawidth];
                                                     uint8_t northsideg[areawidth];
                                                     uint8_t northsideb[areawidth];
                                                     
                                                     uint8_t southsider[areawidth];
                                                     uint8_t southsideg[areawidth];
                                                     uint8_t southsideb[areawidth];
                                                     
                                                     uint8_t eastsider[areaheight];
                                                     uint8_t eastsideg[areaheight];
                                                     uint8_t eastsideb[areaheight];
                                                     
                                                     uint8_t westsider[areaheight];
                                                     uint8_t westsideg[areaheight];
                                                     uint8_t westsideb[areaheight];
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
    
    areaexportexportbutton->set_callback([&screen,&areaexportprogresswindow,&areaexportprogress,&globalmapwindow,&regionalmapwindow,&areaexportwindow,&world,&region,&mapview,&regionalmapimagecreated,&regionalelevationimage,&regionaltemperatureimage,&regionalprecipitationimage,&regionalclimateimage,&regionalriversimage,&regionalreliefimage,&regionalimagewidth,&regionalimageheight,&regionalimagechannels,&smalllake,&island,&peaks,&riftblob,&riftblobsize,&smudge,&smallsmudge,&areanwx,&areanwy,&areasex,&areasey,&areanex,&areaney,&areaswx,&areaswy,&globalmap,&globalreliefimage,&globalmapwidget,&areaexportmapwidget,&areafromregional,&regionalmap,&regionalmapwidget,&regionprogress,&regionalminimapwidget]
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
                                                     
                                                     generateregionalmap(*world,*region,*screen,*areaexportprogress,progressstep,smalllake,island,*peaks,riftblob,riftblobsize,0,smudge,smallsmudge);
                                                     
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
                                                 generateregionalmap(*world,*region,*screen,*areaexportprogress,progressstep,smalllake,island,*peaks,riftblob,riftblobsize,0,smudge,smallsmudge);
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
    
    importlandmapbutton->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&importmapwidget]
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
                                                  drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                                  
                                                  globalmap->upload(globalreliefimage);
                                                  
                                                  importmapwidget->set_image(globalmap);
                                              }
                                          }
                                      });
    
    importseamapbutton->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&importmapwidget]
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
                                                 drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                                 
                                                 globalmap->upload(globalreliefimage);
                                                 
                                                 importmapwidget->set_image(globalmap);
                                             }
                                         }
                                     });
    
    importmountainsbutton->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&importmapwidget]
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
                                                    
                                                    vector<vector<int>> rawmountains(ARRAYWIDTH,vector<int>(ARRAYHEIGHT,0));
                                                    
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
                                                    drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                                    
                                                    globalmap->upload(globalreliefimage);
                                                    
                                                    importmapwidget->set_image(globalmap);
                                                }
                                            }
                                        });
    
    importvolcanoesbutton->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&importmapwidget]
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
                                                    drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                                    
                                                    globalmap->upload(globalreliefimage);
                                                    
                                                    importmapwidget->set_image(globalmap);
                                                }
                                            }
                                        });
    
    importgenshelvesbutton->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&importmapwidget]
                                         {
                                             int width=world->width();
                                             int height=world->height();
                                             int maxelev=world->maxelevation();
                                             int sealevel=world->sealevel();
                                             
                                             vector<vector<bool>> shelves(ARRAYWIDTH,vector<bool>(ARRAYHEIGHT,0));
                                             
                                             makecontinentalshelves(*world,shelves,4);
                                             
                                             int grain=8; // Level of detail on this fractal map.
                                             float valuemod=0.2;
                                             int v=random(3,6);
                                             float valuemod2=v;
                                             
                                             vector<vector<int>> seafractal(ARRAYWIDTH,vector<int>(ARRAYHEIGHT,0));
                                             
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
                                             drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                             
                                             globalmap->upload(globalreliefimage);
                                             
                                             importmapwidget->set_image(globalmap);
                                         });
    
    importgenridgesbutton->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&importmapwidget]
                                        {
                                            int width=world->width();
                                            int height=world->height();
                                            int maxelev=world->maxelevation();
                                            int sealevel=world->sealevel();
                                            
                                            vector<vector<bool>> shelves(ARRAYWIDTH,vector<bool>(ARRAYHEIGHT,0));
                                            
                                            for (int i=0; i<=width; i++)
                                            {
                                                for (int j=0; j<=height; j++)
                                                {
                                                    if (world->nom(i,j)>sealevel-400)
                                                        shelves[i][j]=1;
                                                }
                                            }
                                            
                                            createoceanridges(*world,shelves);
                                            
                                            // Now just redo the map.
                                            
                                            for (int n=0; n<GLOBALMAPTYPES; n++)
                                                globalmapimagecreated[n]=0;
                                            drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                            
                                            globalmap->upload(globalreliefimage);
                                            
                                            importmapwidget->set_image(globalmap);
                                        });
    
    importgenlandbutton->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&importmapwidget]
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
                                          
                                          vector<vector<int>> fractal(ARRAYWIDTH,vector<int>(ARRAYHEIGHT,0));
                                          
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
                                          drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                          
                                          globalmap->upload(globalreliefimage);
                                          
                                          importmapwidget->set_image(globalmap);
                                      });
    
    importgenseabedbutton->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&importmapwidget]
                                        {
                                            int width=world->width();
                                            int height=world->height();
                                            int maxelev=world->maxelevation();
                                            int sealevel=world->sealevel();
                                            
                                            int grain=8; // Level of detail on this fractal map.
                                            float valuemod=0.2;
                                            int v=random(3,6);
                                            float valuemod2=v;
                                            
                                            vector<vector<int>> seafractal(ARRAYWIDTH,vector<int>(ARRAYHEIGHT,0));
                                            
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
                                            drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                            
                                            globalmap->upload(globalreliefimage);
                                            
                                            importmapwidget->set_image(globalmap);
                                        });
    
    importgenmountainsbutton->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&importmapwidget,&landshape,&chainland]
                                           {
                                               int width=world->width();
                                               int height=world->height();
                                               int maxelev=world->maxelevation();
                                               int sealevel=world->sealevel();
                                               
                                               int baseheight=sealevel-4500;
                                               if (baseheight<1)
                                                   baseheight=1;
                                               int conheight=sealevel+50;
                                               
                                               vector<vector<int>> plateaumap(ARRAYWIDTH,vector<int>(ARRAYHEIGHT,0));
                                               
                                               // First, make a fractal map.
                                               
                                               int grain=8; // Level of detail on this fractal map.
                                               float valuemod=0.2;
                                               int v=random(3,6);
                                               float valuemod2=v;
                                               
                                               vector<vector<int>> fractal(ARRAYWIDTH,vector<int>(ARRAYHEIGHT,0));
                                               
                                               createfractal(fractal,width,height,grain,valuemod,valuemod2,1,12750,0,0);
                                               
                                               int fractaladd=sealevel-2500;
                                               
                                               for (int i=0; i<=width; i++)
                                               {
                                                   for (int j=0; j<=height; j++)
                                                       fractal[i][j]=fractal[i][j]+fractaladd;
                                               }
                                               
                                               twointegers dummy[1];
                                               
                                               createchains(*world,baseheight,conheight,fractal,plateaumap,landshape,chainland,dummy,0,0,5);
                                               
                                               // Now just redo the map.
                                               
                                               for (int n=0; n<GLOBALMAPTYPES; n++)
                                                   globalmapimagecreated[n]=0;
                                               drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                               
                                               globalmap->upload(globalreliefimage);
                                               
                                               importmapwidget->set_image(globalmap);
                                           });
    
    importgenhillsbutton->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&importmapwidget,&landshape,&chainland]
                                       {
                                           int width=world->width();
                                           int height=world->height();
                                           int maxelev=world->maxelevation();
                                           int sealevel=world->sealevel();
                                           
                                           int baseheight=sealevel-4500;
                                           if (baseheight<1)
                                               baseheight=1;
                                           int conheight=sealevel+50;
                                           
                                           vector<vector<int>> plateaumap(ARRAYWIDTH,vector<int>(ARRAYHEIGHT,0));
                                           
                                           // First, make a fractal map.
                                           
                                           int grain=8; // Level of detail on this fractal map.
                                           float valuemod=0.2;
                                           int v=random(3,6);
                                           float valuemod2=v;
                                           
                                           vector<vector<int>> fractal(ARRAYWIDTH,vector<int>(ARRAYHEIGHT,0));
                                           
                                           createfractal(fractal,width,height,grain,valuemod,valuemod2,1,12750,0,0);
                                           
                                           int fractaladd=sealevel-2500;
                                           
                                           for (int i=0; i<=width; i++)
                                           {
                                               for (int j=0; j<=height; j++)
                                                   fractal[i][j]=fractal[i][j]+fractaladd;
                                           }
                                           
                                           twointegers dummy[1];
                                           
                                           createchains(*world,baseheight,conheight,fractal,plateaumap,landshape,chainland,dummy,0,0,5);
                                           
                                           // Now just redo the map.
                                           
                                           for (int n=0; n<GLOBALMAPTYPES; n++)
                                               globalmapimagecreated[n]=0;
                                           drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                           
                                           globalmap->upload(globalreliefimage);
                                           
                                           importmapwidget->set_image(globalmap);
                                       });
    
    importgencoastsbutton->set_callback([&world,&globalmapimagecreated,&mapview,&globalelevationimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels,&globalmap,&importmapwidget,&landshape]
                                        {
                                            removestraights(*world);
                                            
                                            // Now just redo the map.
                                            
                                            for (int n=0; n<GLOBALMAPTYPES; n++)
                                                globalmapimagecreated[n]=0;
                                            
                                            mapview=relief;
                                            drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                            
                                            globalmap->upload(globalreliefimage);
                                            
                                            importmapwidget->set_image(globalmap);
                                        });
    
    importgenclimatebutton->set_callback([&screen,&importwindow,&globalmapwindow,&globalmapwidget,&globalmap,&globalmapimagecreated,&worldgenerationwindow,&worldprogress,&generationlabel,&world,&smalllake,&largelake,&landshape,&mapview,&globalelevationimage,&globaltemperatureimage,&globalprecipitationimage,&globalclimateimage,&globalriversimage,&globalreliefimage,&globalimagewidth,&globalimageheight,&globalimagechannels]
                                         {
                                             globalmapwindow->set_position(importwindow->position());
                                             
                                             int width=world->width();
                                             int height=world->height();
                                             int maxelev=world->maxelevation();
                                             
                                             importwindow->set_visible(0);
                                             
                                             vector<vector<int>> mountaindrainage(ARRAYWIDTH,vector<int>(ARRAYHEIGHT,0));
                                             vector<vector<bool>> shelves(ARRAYWIDTH,vector<bool>(ARRAYHEIGHT,0));
                                             
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
                                             
                                             vector<vector<int>> roughness(ARRAYWIDTH,vector<int>(ARRAYHEIGHT,0));
                                             
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
                                             drawglobalmapimage(mapview,*world,globalmapimagecreated,globalelevationimage,globaltemperatureimage,globalprecipitationimage,globalclimateimage,globalriversimage,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
                                             
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

void setregionalminimap(planet &world, region &region, uint8_t globalreliefimage[], nanogui::Texture &regionalminimap, int globalimagewidth, int globalimageheight, int globalimagechannels)
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
    vector<uint8_t> northsider(patchwidth);
    vector<uint8_t> northsideg(patchwidth);
    vector<uint8_t> northsideb(patchwidth);
    
    vector<uint8_t> southsider(patchwidth);
    vector<uint8_t> southsideg(patchwidth);
    vector<uint8_t> southsideb(patchwidth);
    
    vector<uint8_t> eastsider(patchwidth);
    vector<uint8_t> eastsideg(patchwidth);
    vector<uint8_t> eastsideb(patchwidth);
    
    vector<uint8_t> westsider(patchwidth);
    vector<uint8_t> westsideg(patchwidth);
    vector<uint8_t> westsideb(patchwidth);
#else
    uint8_t northsider[patchwidth];
    uint8_t northsideg[patchwidth];
    uint8_t northsideb[patchwidth];
    
    uint8_t southsider[patchwidth];
    uint8_t southsideg[patchwidth];
    uint8_t southsideb[patchwidth];
    
    uint8_t eastsider[patchheight];
    uint8_t eastsideg[patchheight];
    uint8_t eastsideb[patchheight];
    
    uint8_t westsider[patchheight];
    uint8_t westsideg[patchheight];
    uint8_t westsideb[patchheight];
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

// This function saves an image.

void saveimage(uint8_t source[], int globalimagechannels, int width, int height, string filename)
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

void drawglobalmapimage(mapviewenum mapview, planet &world, bool globalmapimagecreated[], uint8_t globalelevationimage[], uint8_t globaltemperatureimage[], uint8_t globalprecipitationimage[], uint8_t globalclimateimage[], uint8_t globalriversimage[], uint8_t globalreliefimage[], int globalimagewidth, int globalimageheight, int globalimagechannels)
{
    // mapview tells us which of these we're actually drawing.
    
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    int globalimagesize=globalimagewidth*globalimageheight*globalimagechannels;
    int index=0;
    
    //int colour1, colour2, colour3;
    
    if (mapview==elevation)
    {
        if (globalmapimagecreated[0]==1)
            return;
        
        drawglobalelevationmapimage(world,globalelevationimage,globalimagewidth,globalimageheight,globalimagechannels);
        
        globalmapimagecreated[0]=1;
    }
    
    if (mapview==temperature)
    {
        if (globalmapimagecreated[2]==1)
            return;
        
        drawglobaltemperaturemapimage(world,globaltemperatureimage,globalimagewidth,globalimageheight,globalimagechannels);
        
        globalmapimagecreated[2]=1;
    }
    
    if (mapview==precipitation)
    {
        if (globalmapimagecreated[3]==1)
            return;
        
        drawglobalprecipitationmapimage(world,globalprecipitationimage,globalimagewidth,globalimageheight,globalimagechannels);
        
        globalmapimagecreated[3]=1;
    }
    
    if (mapview==climate)
    {
        if (globalmapimagecreated[4]==1)
            return;
        
        drawglobalclimatemapimage(world,globalclimateimage,globalimagewidth,globalimageheight,globalimagechannels);
        
        globalmapimagecreated[4]=1;
    }
    
    if (mapview==rivers)
    {
        if (globalmapimagecreated[5]==1)
            return;
        
        drawglobalriversmapimage(world,globalriversimage,globalimagewidth,globalimageheight,globalimagechannels);
        
        globalmapimagecreated[5]=1;
    }
    
    if (mapview==relief)
    {
        if (globalmapimagecreated[6]==1)
            return;

        drawglobalreliefmapimage(world,globalreliefimage,globalimagewidth,globalimageheight,globalimagechannels);
        
        globalmapimagecreated[6]=1;
    }
}

// This function draws a global elevation map image.

void drawglobalelevationmapimage(planet &world, uint8_t globalelevationimage[], int globalimagewidth, int globalimageheight, int globalimagechannels)
{
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    int globalimagesize=globalimagewidth*globalimageheight*globalimagechannels;
    int index=0;
    
    int colour1, colour2, colour3;
    
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
}

// This function draws a global temperature map image.

void drawglobaltemperaturemapimage(planet &world, uint8_t globaltemperatureimage[], int globalimagewidth, int globalimageheight, int globalimagechannels)
{
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    int globalimagesize=globalimagewidth*globalimageheight*globalimagechannels;
    int index=0;
    
    int colour1, colour2, colour3;
    
    int landdiv=((world.maxelevation()-sealevel)/2)/255;
    int seadiv=sealevel/255;
    
    int div=world.maxelevation()/255;
    int base=world.maxelevation()/4;
    
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
}

// This function draws a global precipitation map image.

void drawglobalprecipitationmapimage(planet &world, uint8_t globalprecipitationimage[], int globalimagewidth, int globalimageheight, int globalimagechannels)
{
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    int globalimagesize=globalimagewidth*globalimageheight*globalimagechannels;
    int index=0;
    
    int colour1, colour2, colour3;
    
    int landdiv=((world.maxelevation()-sealevel)/2)/255;
    int seadiv=sealevel/255;
    
    int div=world.maxelevation()/255;
    int base=world.maxelevation()/4;
    
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
}

// This function draws a global climate map image.

void drawglobalclimatemapimage(planet &world, uint8_t globalclimateimage[], int globalimagewidth, int globalimageheight, int globalimagechannels)
{
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    int globalimagesize=globalimagewidth*globalimageheight*globalimagechannels;
    int index=0;
    
    int colour1, colour2, colour3;
    
    int landdiv=((world.maxelevation()-sealevel)/2)/255;
    int seadiv=sealevel/255;
    
    int div=world.maxelevation()/255;
    int base=world.maxelevation()/4;
    
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
}

// This function draws a global rivers map image.

void drawglobalriversmapimage(planet &world, uint8_t globalriversimage[], int globalimagewidth, int globalimageheight, int globalimagechannels)
{
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    int globalimagesize=globalimagewidth*globalimageheight*globalimagechannels;
    int index=0;
    
    int colour1, colour2, colour3;
    
    int landdiv=((world.maxelevation()-sealevel)/2)/255;
    int seadiv=sealevel/255;
    
    int div=world.maxelevation()/255;
    int base=world.maxelevation()/4;
    
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
}

// This function draws a global relief map image.

void drawglobalreliefmapimage(planet &world, uint8_t globalreliefimage[], int globalimagewidth, int globalimageheight, int globalimagechannels)
{
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    int globalimagesize=globalimagewidth*globalimageheight*globalimagechannels;
    int index=0;
    
    int colour1, colour2, colour3;
    
    int landdiv=((world.maxelevation()-sealevel)/2)/255;
    int seadiv=sealevel/255;
    
    int div=world.maxelevation()/255;
    int base=world.maxelevation()/4;
    
    int mult=world.maxriverflow()/255;
    
    int minriverflow=world.minriverflowglobal(); // Rivers of this size or larger will be shown on the map.
    
    float landshading=world.landshading();
    float lakeshading=world.lakeshading();
    float seashading=world.seashading();
    
    int shadingdir=world.shadingdir();
    
    vector<vector<short>> reliefmap1(ARRAYWIDTH,vector<short>(ARRAYHEIGHT,0));
    vector<vector<short>> reliefmap2(ARRAYWIDTH,vector<short>(ARRAYHEIGHT,0));
    vector<vector<short>> reliefmap3(ARRAYWIDTH,vector<short>(ARRAYHEIGHT,0));
    
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
}

// This function draws a regional map image (ready to be applied to a texture).

void drawregionalmapimage(mapviewenum mapview, planet &world, region &region, bool regionalmapimagecreated[], uint8_t regionalelevationimage[], uint8_t regionaltemperatureimage[], uint8_t regionalprecipitationimage[],  uint8_t regionalclimateimage[], uint8_t regionalriversimage[], uint8_t regionalreliefimage[], int regionalimagewidth, int regionalimageheight, int regionalimagechannels)
{
    if (mapview==elevation)
    {
        if (regionalmapimagecreated[0]==1)
            return;
        
        drawregionalelevationmapimage(world,region,regionalelevationimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
        
        regionalmapimagecreated[0]=1;
    }
    
    if (mapview==temperature)
    {
        if (regionalmapimagecreated[2]==1)
            return;
        
        drawregionaltemperaturemapimage(world,region,regionaltemperatureimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
        
        regionalmapimagecreated[2]=1;
    }
    
    if (mapview==precipitation)
    {
        if (regionalmapimagecreated[3]==1)
            return;
        
        drawregionalprecipitationmapimage(world,region,regionalprecipitationimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
        
        regionalmapimagecreated[3]=1;
    }
    
    if (mapview==climate)
    {
        if (regionalmapimagecreated[4]==1)
            return;
        
        drawregionalclimatemapimage(world,region,regionalclimateimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
        
        regionalmapimagecreated[4]=1;
    }
    
    if (mapview==rivers)
    {
        if (regionalmapimagecreated[5]==1)
            return;
        
        drawregionalriversmapimage(world,region,regionalriversimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
        
        regionalmapimagecreated[5]=1;
    }
    
    if (mapview==relief)
    {
        if (regionalmapimagecreated[6]==1)
            return;
        
        drawregionalreliefmapimage(world,region,regionalreliefimage,regionalimagewidth,regionalimageheight,regionalimagechannels);
        
        regionalmapimagecreated[6]=1;
    }
}

// This function draws a regional elevation map image.

void drawregionalelevationmapimage(planet &world, region &region, uint8_t regionalelevationimage[], int regionalimagewidth, int regionalimageheight, int regionalimagechannels)
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
    
    int colour1, colour2, colour3, index;
    
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
}

void drawregionaltemperaturemapimage(planet &world, region &region, uint8_t regionaltemperatureimage[], int regionalimagewidth, int regionalimageheight, int regionalimagechannels)
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
    
    int colour1, colour2, colour3, index;
    
    int div=world.maxelevation()/255;
    int base=world.maxelevation()/4;
    
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
}

void drawregionalprecipitationmapimage(planet &world, region &region, uint8_t regionalprecipitationimage[], int regionalimagewidth, int regionalimageheight, int regionalimagechannels)
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
    
    int colour1, colour2, colour3, index;
    
    int div=world.maxelevation()/255;
    int base=world.maxelevation()/4;
    
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
            
            index=((i-origregwidthbegin)+(j-origregheightbegin)*regionalimagewidth)*regionalimagechannels;
            
            regionalprecipitationimage[index]=colour1;
            regionalprecipitationimage[index+1]=colour2;
            regionalprecipitationimage[index+2]=colour3;
        }
    }
}

void drawregionalclimatemapimage(planet &world, region &region, uint8_t regionalclimateimage[], int regionalimagewidth, int regionalimageheight, int regionalimagechannels)
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
    
    int colour1, colour2, colour3, index;
    
    int div=world.maxelevation()/255;
    int base=world.maxelevation()/4;
    
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
}

void drawregionalriversmapimage(planet &world, region &region, uint8_t regionalriversimage[], int regionalimagewidth, int regionalimageheight, int regionalimagechannels)
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
    
    int colour1, colour2, colour3, index;
    
    int div=world.maxelevation()/255;
    int base=world.maxelevation()/4;
    
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
}

void drawregionalreliefmapimage(planet &world, region &region, uint8_t regionalreliefimage[], int regionalimagewidth, int regionalimageheight, int regionalimagechannels)
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
    
    int colour1, colour2, colour3, index;
    
    int div=world.maxelevation()/255;
    int base=world.maxelevation()/4;
    
    int mult=world.maxriverflow()/400;
    
    //int regwidthbegin=region.regwidthbegin();
    
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
    
    vector<vector<short>> reliefmap1(RARRAYWIDTH,vector<short>(RARRAYHEIGHT,0));
    vector<vector<short>> reliefmap2(RARRAYWIDTH,vector<short>(RARRAYHEIGHT,0));
    vector<vector<short>> reliefmap3(RARRAYWIDTH,vector<short>(RARRAYHEIGHT,0));
    
    // Make a fractal based on rainfall, which will be used to add stripes to vary the colours.
    
    vector<vector<int>> source(ARRAYWIDTH,vector<int>(ARRAYHEIGHT,0));
    vector<vector<int>> stripefractal(RARRAYWIDTH,vector<int>(RARRAYHEIGHT,-5000));
    
    int coords[4][2];
    
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
                                twointegers nearest=findclosestriverquickly(region,i,j);
                                
                                if (nearest.x!=-1)
                                    biggestflow=region.riverjan(nearest.x,nearest.y)+region.riverjul(nearest.x,nearest.y);
                            }
                            
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
}

// This function clears the regional relief map image (ready to be applied to a texture).

void blankregionalreliefimage(region &region, uint8_t regionalreliefimage[], int regionalimagewidth, int regionalimageheight, int regionalimagechannels)
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
    
    for (unsigned i=0; i<size.x; i=i+16)
    {
        for (unsigned j=0; j<size.y; j++)
            mapimage.setPixel(i,j,sf::Color(80,50,20));
    }
    
    for (unsigned j=0; j<size.y; j=j+16)
    {
        for (unsigned i=0; i<size.x; i++)
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

void displaytemplates(planet &world, uint8_t globalreliefimage[], int globalimagewidth, int globalimagechannels, boolshapetemplate shape[])
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

// Now come some functions to do with generating global terrain, global climate, and regional maps. These would be in globalterrain.cpp, globalclimate.cpp, and regionalmap.cpp, but they use NanoGUI elements, so they are in main.cpp.

// This function generates the global terrain.

void generateglobalterrain(planet &world, short terraintype, nanogui::Screen &screen, nanogui::Window &worldgenerationwindow, nanogui::Label &worldgenerationlabel, nanogui::ProgressBar &worldprogress, float progressstep, boolshapetemplate landshape[], boolshapetemplate chainland[], vector<vector<int>> &mountaindrainage, vector<vector<bool>> &shelves)
{
    switch (terraintype)
    {
        case 1:
            generateglobalterraintype1(world,screen,worldgenerationwindow,worldgenerationlabel,worldprogress,progressstep,landshape,mountaindrainage,shelves,chainland);
            break;
            
        case 2:
            generateglobalterraintype2(world,screen,worldgenerationwindow,worldgenerationlabel,worldprogress,progressstep,landshape,mountaindrainage,shelves,chainland);
            break;
    }
}

// This creates type 1 terrain. This type gives a fairly chaotic looking map with small continents and lots of islands.

void generateglobalterraintype1(planet &world, nanogui::Screen &screen, nanogui::Window &worldgenerationwindow, nanogui::Label &worldgenerationlabel, nanogui::ProgressBar &worldprogress, float progressstep, boolshapetemplate landshape[],vector<vector<int>> &mountaindrainage, vector<vector<bool>> &shelves, boolshapetemplate chainland[])
{
    // First get our key variables and clear the world.
    
    long seed=world.seed();
    fast_srand(seed);
    
    int width=world.width();
    int height=world.height();
    int maxelev=world.maxelevation();
    int sealevel=world.sealevel();
    int baseheight=sealevel-4500; //1250;
    if (baseheight<1)
        baseheight=1;
    int conheight=sealevel+50;
    
    vector<vector<int>> plateaumap(ARRAYWIDTH,vector<int>(ARRAYHEIGHT,0));
    vector<vector<int>> seafractal(ARRAYWIDTH,vector<int>(ARRAYHEIGHT,0));
    vector<vector<bool>> removedland(ARRAYWIDTH,vector<bool>(ARRAYHEIGHT,0)); // This will show where land has been removed.
    vector<vector<int>> volcanodensity(ARRAYWIDTH,vector<int>(ARRAYHEIGHT,0)); // How many submarine volcanoes.
    vector<vector<int>> volcanodirection(ARRAYWIDTH,vector<int>(ARRAYHEIGHT,0)); // Direction of extinct volcanoes leading away from active ones.
    
    world.clear(); // Clears all of the maps in this world.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            world.setnom(i,j,0);
            plateaumap[i][j]=0;
            mountaindrainage[i][j]=0;
            shelves[i][j]=0;
            seafractal[i][j]=0;
            removedland[i][j]=0;
            volcanodensity[i][j]=0;
            volcanodirection[i][j]=0;
        }
    }
    
    // Now start the generating.
    
    worldgenerationlabel.set_caption("Creating fractal map");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    // First make a fractal for noise (used only at regional map level).
    
    int grain=8; // Level of detail on this fractal map.
    float valuemod=0.2;
    int v=random(3,6);
    float valuemod2=v;
    
    vector<vector<int>> fractal(ARRAYWIDTH,vector<int>(ARRAYHEIGHT,0));
    
    createfractal(fractal,width,height,grain,valuemod,valuemod2,1,maxelev,0,0);
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
            world.setnoisemap(i,j,fractal[i][j]);
    }
    
    // Now make a new one that we'll actually use for global terrain creation.
    
    createfractal(fractal,width,height,grain,valuemod,valuemod2,1,maxelev,0,0);
    
    worldgenerationlabel.set_caption("Creating continental map");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    smallcontinents(world,baseheight,conheight,fractal,plateaumap,landshape,chainland);
    
    flip(fractal,width,height,1,1);
    
    // Merge the maps.
    
    worldgenerationlabel.set_caption("Merging maps");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    fractalmerge(world,fractal);
    
    // Make continental shelves.
    
    worldgenerationlabel.set_caption("Making continental shelves");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    makecontinentalshelves(world,shelves,20); // Wider shelves than with the other terrain generator, because this one will produce lots of little bits of land
    
    // Now we add some island chains.
    
    twointegers focuspoints[4]; // These will be points where island chains will start close to.
    
    int focustotal=random(2,4); // The number of actual focus points.
    
    for (int n=0; n<focustotal; n++)
    {
        focuspoints[n].x=random(0,width);
        focuspoints[n].y=random(0,height);
    }
    
    int focaldistance=height/2; // Maximum distance a chain can start from the focuspoint.
    createchains(world,baseheight,conheight,fractal,plateaumap,landshape,chainland,focuspoints,focustotal,focaldistance,3);
    
    worldgenerationlabel.set_caption("Shifting fractal");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    shift(fractal,width,height,width/2); // Shift the fractal to make it different for the mountains - we will use the fractal to adjust peak height.
    
    worldgenerationlabel.set_caption("Smoothing map");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    world.smoothnom(1);
    
    // We also need to remove the trench down the left-hand side.
    
    removeseam(world,0);
    
    // Now remove inland seas.
    
    worldgenerationlabel.set_caption("Removing inland seas");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    removeseas(world,conheight);
    
    // Now widen any channels and lower the coasts.
    
    worldgenerationlabel.set_caption("Tidying up oceans");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    widenchannels(world);
    loweroceans(world);
    
    // Now try to remove straight coastlines.
    
    worldgenerationlabel.set_caption("Improving coastlines");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    removestraights(world);
    
    // Now make sure the southern edge is correct.
    
    worldgenerationlabel.set_caption("Checking poles");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    checkpoles(world);
    
    // Now sort out the sea depths.
    
    worldgenerationlabel.set_caption("Adjusting ocean depths");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    grain=8; // Level of detail on this fractal map.
    valuemod=0.2;
    v=random(3,6);
    valuemod2=v;
    
    createfractal(seafractal,width,height,grain,valuemod,valuemod2,1,maxelev,0,0);
    
    float coastalvarreduce=maxelev/500; //3000;
    float oceanvarreduce=maxelev/1000;
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (world.sea(i,j)==1)
            {
                if (shelves[i][j]==1)
                {
                    float var=seafractal[i][j]-maxelev/2;
                    var=var/coastalvarreduce;
                    
                    int newval=sealevel-200+var;
                    
                    if (newval>sealevel-10)
                        newval=sealevel-10;
                    
                    if (newval<1)
                        newval=1;
                    
                    world.setnom(i,j,newval);
                }
                else
                {
                    int ii=i+width/2;
                    
                    if (ii>width)
                        ii=ii-width;
                    
                    float var=seafractal[ii][j]-maxelev/2;
                    var=var/oceanvarreduce;
                    
                    int newval=sealevel-5000+var;
                    
                    if (newval>sealevel-3000)
                        newval=sealevel-3000;
                    
                    if (newval<1)
                        newval=1;
                    
                    world.setnom(i,j,newval);
                }
            }
        }
    }
    
    // Now we create mid-ocean ridges.
    
    worldgenerationlabel.set_caption("Generating mid-ocean ridges");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    createoceanridges(world,shelves);
    
    // Now we create deep-sea trenches.
    
    worldgenerationlabel.set_caption("Generating deep-sea trenches");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    createoceantrenches(world,shelves);
    
    // Now random volcanoes.
    
    worldgenerationlabel.set_caption("Generating volcanoes");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    grain=8; // Level of detail on this fractal map.
    valuemod=0.2;
    v=random(3,6);
    valuemod2=v;
    
    createfractal(volcanodensity,width,height,grain,valuemod,valuemod2,1,maxelev,0,0);
    
    grain=4; // Level of detail on this fractal map.
    valuemod=0.02;
    v=1; //random(3,6);
    valuemod2=0.2;
    
    createfractal(volcanodirection,width,height,grain,valuemod,valuemod2,1,maxelev,0,0);
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            bool goahead=0;
            
            if (world.sea(i,j)==1)
            {
                int frac=volcanodensity[i][j];
                
                int ii=i+width/2;
                if (ii>width)
                    ii=ii-width;
                
                int frac2=volcanodensity[ii][j];
                
                if (frac2<frac)
                    frac=frac2;
                
                int jj=j+height/2;
                if (jj>height)
                    jj=jj-height;
                
                int frac3=volcanodensity[i][jj];
                
                if (frac3<frac)
                    frac=frac3;
                
                int rand=5000;
                
                if (frac>maxelev/2)
                    rand=80;
                
                if (frac>(maxelev/4)*3)
                    rand=40;
                
                if (random(1,rand)==1)
                    goahead=1;
            }
            else
            {
                if (random(1,15000)==1)
                    goahead=1;
            }
            
            if (goahead==1)
            {
                bool strato=1;
                
                if (random(1,10)==1) // Shield volcanoes - much rarer.
                    strato=0;
                
                if (world.sea(i,j)==1)
                    strato=1;
                
                int peakheight;
                
                if (world.sea(i,j)==1)
                {
                    if (random(1,10)==1) // It could make a chain of volcanic islands.
                        peakheight=sealevel-world.nom(i,j)+random(500,3000);
                    else
                        peakheight=sealevel-world.nom(i,j)-random(100,200);
                    
                    if (peakheight<10)
                        peakheight=10;
                    
                    peakheight=random(peakheight/2,peakheight);
                }
                else
                {
                    if (strato==1)
                        peakheight=random(2000,6000);
                    else
                        peakheight=random(1000,2000);
                }
                
                createisolatedvolcano(world,i,j,shelves,volcanodirection,peakheight,strato);
            }
        }
    }
    
    // Now we shift the map so there is sea at the edges, if possible.
    
    worldgenerationlabel.set_caption("Shifting for best position");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    removeseam(world,0);
    
    adjustforsea(world);
    
    // Now we add smaller mountain chains that cannot form peninsulas.
    
    worldgenerationlabel.set_caption("Adding smaller mountain ranges");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    twointegers dummy[1];
    
    createchains(world,baseheight,conheight,fractal,plateaumap,landshape,chainland,dummy,0,0,2);
    
    // Now we alter the fractal again, and use it to add more height variation.
    
    worldgenerationlabel.set_caption("Merging fractal into land");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    flip(fractal,width,height,1,1);
    int offset=random(1,width);
    shift(fractal,width,height,offset);
    
    fractalmergeland(world,fractal,conheight);
    
    // Now remove any mountains that are over sea.
    
    worldgenerationlabel.set_caption("Removing floating mountains");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    removefloatingmountains(world);
    cleanmountainridges(world);
    
    // Now we raise the mountain bases.
    
    worldgenerationlabel.set_caption("Raising mountain bases");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    raisemountainbases(world,mountaindrainage);
    
    // Now we add plateaux.
    
    worldgenerationlabel.set_caption("Adding plateaux");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    erodeplateaux(world,plateaumap);
    smooth(plateaumap,width,height,maxelev,1,1);
    addplateaux(world,plateaumap,conheight);
    
    // Now we smooth again, without changing the coastlines.
    
    worldgenerationlabel.set_caption("Smoothing map, preserving coastlines");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    smoothland(world,2);
    
    // Now we create extra elevation over the land to create canyons.
    
    worldgenerationlabel.set_caption("Elevating land near canyons");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    createextraelev(world);
    
    // Now we remove depressions.
    
    worldgenerationlabel.set_caption("Filling depressions");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    depressionfill(world);
    
    addlandnoise(world); // Add a bit of noise, then do remove depressions again. This is to add variety to the river courses.
    
    depressionfill(world);
    
    // Now we adjust the land around coastlines.
    
    worldgenerationlabel.set_caption("Adjusting coastlines");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int n=1; n<=2; n++)
        normalisecoasts(world,13,11,4);
    
    clamp(world);
    
    // Now we note down one-tile islands.
    
    worldgenerationlabel.set_caption("Checking islands");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    checkislands(world);
    
    // Now we create a roughness map.
    
    worldgenerationlabel.set_caption("Creating roughness map");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    vector<vector<int>> roughness(ARRAYWIDTH,vector<int>(ARRAYHEIGHT,0));
    
    grain=8; // Level of detail on this fractal map.
    valuemod=0.2;
    valuemod2=0.6;
    
    createfractal(roughness,width,height,grain,valuemod,valuemod2,1,maxelev,0,0);
    
    vector<vector<int>> roughness2(ARRAYWIDTH,vector<int>(ARRAYHEIGHT,0));
    
    createfractal(roughness2,width,height,grain,valuemod,valuemod2,1,maxelev,0,0);
    
    for (int i=0; i<=width; i++) // Do two, so that it's rare to get areas that are very smooth.
    {
        for (int j=0; j<=height; j++)
        {
            if (roughness[i][j]<roughness2[i][j])
                roughness[i][j]=roughness2[i][j];
        }
    }
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
            world.setroughness(i,j,roughness[i][j]);
    }
}

// This creates type 2 terrain. This type gives a more earthlike map with large continents.

void generateglobalterraintype2(planet &world, nanogui::Screen &screen, nanogui::Window &worldgenerationwindow, nanogui::Label &worldgenerationlabel, nanogui::ProgressBar &worldprogress, float progressstep, boolshapetemplate landshape[],vector<vector<int>> &mountaindrainage, vector<vector<bool>> &shelves, boolshapetemplate chainland[])
{
    // First get our key variables and clear the world.
    
    long seed=world.seed();
    fast_srand(seed);
    
    int width=world.width();
    int height=world.height();
    int maxelev=world.maxelevation();
    int sealevel=world.sealevel();
    int baseheight=sealevel-4500; //1250;
    if (baseheight<1)
        baseheight=1;
    int conheight=sealevel+50;
    
    int maxmountainheight=maxelev-sealevel;
    
    vector<vector<int>> plateaumap(ARRAYWIDTH,vector<int>(ARRAYHEIGHT,0));
    vector<vector<int>> seafractal(ARRAYWIDTH,vector<int>(ARRAYHEIGHT,0));
    vector<vector<bool>> removedland(ARRAYWIDTH,vector<bool>(ARRAYHEIGHT,0)); // This will show where land has been removed.
    vector<vector<int>> volcanodensity(ARRAYWIDTH,vector<int>(ARRAYHEIGHT,0)); // How many submarine volcanoes.
    vector<vector<int>> volcanodirection(ARRAYWIDTH,vector<int>(ARRAYHEIGHT,0)); // Direction of extinct volcanoes leading away from active ones.
    
    world.clear(); // Clears all of the maps in this world.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            world.setnom(i,j,0);
            plateaumap[i][j]=0;
            mountaindrainage[i][j]=0;
            shelves[i][j]=0;
            seafractal[i][j]=0;
            removedland[i][j]=0;
            volcanodensity[i][j]=0;
            volcanodirection[i][j]=0;
        }
    }
    
    // Now start the generating.
    
    worldgenerationlabel.set_caption("Creating fractal map");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    // First make a fractal for noise (used only at regional map level).
    
    int grain=8; // Level of detail on this fractal map.
    float valuemod=0.2;
    int v=random(3,6);
    float valuemod2=v;
    
    vector<vector<int>> fractal(ARRAYWIDTH,vector<int>(ARRAYHEIGHT,0));
    
    createfractal(fractal,width,height,grain,valuemod,valuemod2,1,maxelev,0,0);
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
            world.setnoisemap(i,j,fractal[i][j]);
    }
    
    // Now make a new one that we'll actually use for global terrain creation.
    
    createfractal(fractal,width,height,grain,valuemod,valuemod2,1,12750,0,0);
    
    int fractaladd=sealevel-2500;
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
            fractal[i][j]=fractal[i][j]+fractaladd;
    }
    
    worldgenerationlabel.set_caption("Creating continental map");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    largecontinents(world,screen,worldgenerationwindow,worldgenerationlabel,worldprogress,progressstep,baseheight,conheight,fractal,plateaumap,shelves,landshape,chainland);
    
    flip(fractal,width,height,1,1);
    
    // Now merge the maps.
    
    worldgenerationlabel.set_caption("Merging maps");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    fractalmergemodified(world,fractal,plateaumap,removedland);
    
    worldgenerationlabel.set_caption("Shifting fractal");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    shift(fractal,width,height,width/2); // Shift the fractal to make it different for the mountains - we will use the fractal to adjust peak height.
    
    worldgenerationlabel.set_caption("Smoothing map");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    world.smoothnom(1);
    
    // We also need to remove the trench down the left-hand side.
    
    removeseam(world,0);
    
    // Now remove inland seas.
    
    worldgenerationlabel.set_caption("Removing inland seas");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    removeseas(world,conheight);
    
    // Now make sure the southern edge is correct.
    
    worldgenerationlabel.set_caption("Checking poles");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    checkpoles(world);
    
    // Now add Aegean-style islands
    
    worldgenerationlabel.set_caption("Adding archipelagos");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    makearchipelagos(world,removedland,landshape);
    
    // Now widen any channels and lower the coasts.
    
    worldgenerationlabel.set_caption("Tidying up oceans");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    widenchannels(world);
    loweroceans(world);
    
    // Now try to remove straight coastlines.
    
    worldgenerationlabel.set_caption("Improving coastlines");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    removestraights(world);
    
    // Now sort out the sea depths.
    
    worldgenerationlabel.set_caption("Adjusting ocean depths");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    grain=8; // Level of detail on this fractal map.
    valuemod=0.2;
    v=random(3,6);
    valuemod2=v;
    
    createfractal(seafractal,width,height,grain,valuemod,valuemod2,1,maxelev,0,0);
    
    float coastalvarreduce=maxelev/500; //3000;
    float oceanvarreduce=maxelev/1000;
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (world.sea(i,j)==1)
            {
                if (shelves[i][j]==1)
                {
                    float var=seafractal[i][j]-maxelev/2;
                    var=var/coastalvarreduce;
                    
                    int newval=sealevel-200+var;
                    
                    if (newval>sealevel-10)
                        newval=sealevel-10;
                    
                    if (newval<1)
                        newval=1;
                    
                    world.setnom(i,j,newval);
                }
                else
                {
                    int ii=i+width/2;
                    
                    if (ii>width)
                        ii=ii-width;
                    
                    float var=seafractal[ii][j]-maxelev/2;
                    var=var/oceanvarreduce;
                    
                    int newval=sealevel-5000+var;
                    
                    if (newval>sealevel-3000)
                        newval=sealevel-3000;
                    
                    if (newval<1)
                        newval=1;
                    
                    world.setnom(i,j,newval);
                }
            }
        }
    }
    
    // Now we create mid-ocean ridges.
    
    worldgenerationlabel.set_caption("Generating mid-ocean ridges");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    createoceanridges(world,shelves);
    
    // Now we create deep-sea trenches.
    
    worldgenerationlabel.set_caption("Generating deep-sea trenches");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    createoceantrenches(world,shelves);
    
    // Now random volcanoes.
    
    worldgenerationlabel.set_caption("Generating volcanoes");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    grain=8; // Level of detail on this fractal map.
    valuemod=0.2;
    v=random(3,6);
    valuemod2=v;
    
    createfractal(volcanodensity,width,height,grain,valuemod,valuemod2,1,maxelev,0,0);
    
    grain=4; // Level of detail on this fractal map.
    valuemod=0.02;
    v=1; //random(3,6);
    valuemod2=0.2;
    
    createfractal(volcanodirection,width,height,grain,valuemod,valuemod2,1,maxelev,0,0);
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            bool goahead=0;
            
            if (world.sea(i,j)==1)
            {
                int frac=volcanodensity[i][j];
                
                int ii=i+width/2;
                if (ii>width)
                    ii=ii-width;
                
                int frac2=volcanodensity[ii][j];
                
                if (frac2<frac)
                    frac=frac2;
                
                int jj=j+height/2;
                if (jj>height)
                    jj=jj-height;
                
                int frac3=volcanodensity[i][jj];
                
                if (frac3<frac)
                    frac=frac3;
                
                int rand=5000;
                
                if (frac>maxelev/2)
                    rand=80;
                
                if (frac>(maxelev/4)*3)
                    rand=40;
                
                if (random(1,rand)==1)
                    goahead=1;
            }
            else
            {
                if (random(1,15000)==1)
                    goahead=1;
            }
            
            if (goahead==1)
            {
                bool strato=1;
                
                if (random(1,10)==1) // Shield volcanoes - much rarer.
                    strato=0;
                
                if (world.sea(i,j)==1)
                    strato=1;
                
                int peakheight;
                
                if (world.sea(i,j)==1)
                {
                    if (random(1,10)==1) // It could make a chain of volcanic islands.
                        peakheight=sealevel-world.nom(i,j)+random(500,3000);
                    else
                        peakheight=sealevel-world.nom(i,j)-random(100,200);
                    
                    if (peakheight<10)
                        peakheight=10;
                    
                    peakheight=random(peakheight/2,peakheight);
                }
                else
                {
                    if (strato==1)
                        peakheight=random(2000,6000);
                    else
                        peakheight=random(1000,2000);
                }
                
                createisolatedvolcano(world,i,j,shelves,volcanodirection,peakheight,strato);
            }
        }
    }
    
    // Now we shift the map so there is sea at the edges, if possible.
    
    worldgenerationlabel.set_caption("Shifting for best position");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    removeseam(world,0);
    
    adjustforsea(world);
    
    // Now we add smaller mountain chains that cannot form peninsulas.
    
    worldgenerationlabel.set_caption("Adding smaller mountain ranges");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    twointegers dummy[1];
    
    createchains(world,baseheight,conheight,fractal,plateaumap,landshape,chainland,dummy,0,0,2);
    
    // Now we alter the fractal again, and use it to add more height variation.
    
    worldgenerationlabel.set_caption("Merging fractal into land");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    flip(fractal,width,height,1,1);
    int offset=random(1,width);
    shift(fractal,width,height,offset);
    
    fractalmergeland(world,fractal,conheight);
    
    // Now remove any mountains that are over sea.
    
    worldgenerationlabel.set_caption("Removing floating mountains");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    removefloatingmountains(world);
    cleanmountainridges(world);
    
    // Now we raise the mountain bases.
    
    worldgenerationlabel.set_caption("Raising mountain bases");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    raisemountainbases(world,mountaindrainage);
    
    // Now we smooth again, without changing the coastlines.
    
    worldgenerationlabel.set_caption("Smoothing map, preserving coastlines");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    smoothland(world,2);
    
    vector<vector<int>> slopes(ARRAYWIDTH,vector<int>(ARRAYHEIGHT,0));
    
    getseaslopes(world,slopes); // Note down all the biggest slopes we currently have. This is so that we don't mark any non-shading areas over these slopes later.
    
    // Now we create extra elevation over the land to create canyons.
    
    worldgenerationlabel.set_caption("Elevating land near canyons");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    createextraelev(world);
    
    // Now we remove depressions.
    
    worldgenerationlabel.set_caption("Filling depressions");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    depressionfill(world);
    
    addlandnoise(world); // Add a bit of noise, then do remove depressions again. This is to add variety to the river courses.
    
    depressionfill(world);
    
    // Now we adjust the land around coastlines.
    
    worldgenerationlabel.set_caption("Adjusting coastlines");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    normalisecoasts(world,13,11,4);
    
    normalisecoasts(world,13,11,4);
    
    clamp(world);
    
    // Now we note down one-tile islands.
    
    worldgenerationlabel.set_caption("Checking islands");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    checkislands(world);
    
    extendnoshade(world);
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (slopes[i][j]>30)
                world.setnoshade(i,j,0);
        }
    }
    
    // Now we remove odd undersea bumps.
    
    worldgenerationlabel.set_caption("Flattening seabeds");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    //removeunderseabumps(world);
    
    // Now we create a roughness map.
    
    worldgenerationlabel.set_caption("Creating roughness map");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    vector<vector<int>> roughness(ARRAYWIDTH,vector<int>(ARRAYHEIGHT,0));
    
    grain=8; // Level of detail on this fractal map.
    valuemod=0.2;
    valuemod2=0.6;
    
    createfractal(roughness,width,height,grain,valuemod,valuemod2,1,maxelev,0,0);
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
            world.setroughness(i,j,roughness[i][j]);
    }
}

// This function makes the continents in a larger style.

void largecontinents(planet &world, nanogui::Screen &screen, nanogui::Window &worldgenerationwindow, nanogui::Label &worldgenerationlabel, nanogui::ProgressBar &worldprogress, float progressstep, int baseheight, int conheight, vector<vector<int>> &fractal, vector<vector<int>> &plateaumap, vector<vector<bool>> &shelves, boolshapetemplate landshape[], boolshapetemplate chainland[])
{
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    int maxelev=world.maxelevation();
    
    int movein=300; // Maximum amount to move neighbouring continents towards the central one.
    int origmountainschance=1; //2; // The higher this is, the less often central continents will have mountains down one side.
    int mountainschance=4; // The higher this is, the less often other continents will have mountains down one side.
    
    worldgenerationlabel.set_caption("Preparing Voronoi map");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    vector<vector<short>> voronoi(width+1,vector<short>(height+1,0));
    int points=200; // Number of points in the voronoi map
    
    makevoronoi(voronoi,width,height,points);
    
    vector<vector<bool>> continent(ARRAYWIDTH,vector<bool>(ARRAYHEIGHT,0));
    vector<vector<short>> continentnos(ARRAYWIDTH,vector<short>(ARRAYHEIGHT,0));
    vector<vector<short>> overlaps(ARRAYWIDTH,vector<short>(ARRAYHEIGHT,0));
    
    worldgenerationlabel.set_caption("Making continents");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    twointegers focuspoints[4]; // These will be points where continents etc will start close to.
    
    int focustotal;
    
    if (random(1,10)==1)
        focustotal=1;
    else
        focustotal=2;
    
    if (random(1,3)!=1)
        focustotal=3;
    
    focuspoints[0].x=width/4;
    focuspoints[0].y=random(height/6,height-height/6);
    
    focuspoints[1].x=width-width/4+randomsign(random(0,width/4));
    focuspoints[1].y=random(height/6,height-height/6);
    
    focuspoints[2].x=random(1,width);
    focuspoints[2].y=random(height/6,height-height/6);
    
    // First, make the whole map our seabed baseheight.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
            world.setnom(i,j,baseheight);
    }
    
    // Now we draw the basic shapes of the continents, and paste them onto the map.
    
    int leftx=0;
    int rightx=0;
    int lefty=0;
    int righty=0; // Define the size and shape of the current continent.
    
    short thiscontinent=0;
    
    for (int thispoint=0; thispoint<focustotal; thispoint++) // Go through the focus points one by one.
    {
        // First, put the central continent for this grouping onto the map.
        
        thiscontinent++;
        
        makecontinent(world,continent,voronoi,points,width,height,leftx,rightx,lefty,righty);
        
        int origcontwidth=rightx-leftx;
        int origcontheight=righty-lefty;
        
        int origstartpointx=focuspoints[thispoint].x-origcontwidth/2-(width-origcontwidth)/2;
        int origstartpointy=focuspoints[thispoint].y-origcontheight/2-(height-origcontheight)/2; // Coordinates of the top left-hand corner of this continent.
        
        int thisleft=-1;
        int thisright=-1;
        int thisup=-1;
        int thisdown=-1;
        
        bool wrapped=0;
        
        bool leftr=random(0,1); // If it's 1 then we reverse it left-right
        bool downr=random(0,1); // If it's 1 then we reverse it top-bottom
        
        int istart=leftx, desti=rightx, istep=1;
        int jstart=lefty, destj=righty, jstep=1;
        
        if (leftr==1)
        {
            istart=rightx;
            desti=leftx;
            istep=-1;
        }
        
        if (downr==1)
        {
            jstart=righty;
            destj=lefty;
            jstep=-1;
        }
        
        int imap=-1;
        int jmap=-1;
        
        for (int i=istart; i!=desti; i=i+istep)
        {
            imap++;
            jmap=-1;
            
            int ii=origstartpointx+i;
            
            if (ii<0 || ii>width)
            {
                ii=wrap(ii,width);
                wrapped=1;
            }
            
            for (int j=jstart; j!=destj; j=j+jstep)
            {
                jmap++;
                
                int jj=origstartpointy+j;
                
                if (jj>=0 && jj<=height)
                {
                    if (continent[imap+leftx][jmap+lefty]==1)
                    {
                        world.setnom(ii,jj,conheight);
                        
                        if (continentnos[ii][jj]!=0)
                        {
                            short overlap=continentnos[ii][jj]*100+thiscontinent;
                            overlaps[ii][jj]=overlap;
                        }
                        
                        continentnos[ii][jj]=thiscontinent;
                        
                        if (ii<thisleft || thisleft==-1)
                            thisleft=ii;
                        
                        if (ii>thisright || thisright==-1)
                            thisright=ii;
                        
                        if (jj<thisup || thisup==-1)
                            thisup=jj;
                        
                        if (jj>thisdown || thisdown==-1)
                            thisdown=jj;
                        
                    }
                }
            }
        }
        
        origstartpointx=thisleft;
        origstartpointy=thisup;
        
        origcontwidth=thisright-thisleft;
        origcontheight=thisdown-thisup;
        
        if (origcontwidth<2 || origcontheight<2)
            wrapped=1;
        
        bool origcontmountains=0;
        
        if (random(1,origmountainschance)==1) // Whether or not this continent has mountains along one edge.
            origcontmountains=1;
        
        short origcontdir=0;
        
        int startnearx=-1;
        int startneary=-1;
        
        int origmountainstartpointx=-1;
        int origmountainstartpointy=-1;
        
        short origstartpoint=0;
        
        if (wrapped==0)
        {
            origstartpoint=random(1,8);
            
            switch (origstartpoint)
            {
                case 1:
                    startnearx=thisleft+random(1,origcontwidth);
                    startneary=thisup;
                    origcontdir=random(4,6);
                    break;
                    
                case 2:
                    startnearx=thisright;
                    startneary=thisup;
                    origcontdir=random(5,7);
                    break;
                    
                case 3:
                    startnearx=thisright;
                    startneary=thisup+random(1,origcontheight);
                    origcontdir=random(6,8);
                    break;
                    
                case 4:
                    startnearx=thisright;
                    startneary=thisdown;
                    origcontdir=random(7,9);
                    if (origcontdir==9)
                        origcontdir=1;
                    break;
                    
                case 5:
                    startnearx=thisleft+random(1,origcontwidth);
                    startneary=thisdown;
                    origcontdir=random(1,3)-1;
                    if (origcontdir==0)
                        origcontdir=8;
                    break;
                    
                case 6:
                    startnearx=thisleft;
                    startneary=thisdown;
                    origcontdir=random(1,3);
                    break;
                    
                case 7:
                    startnearx=thisleft;
                    startneary=thisup+random(1,origcontheight);
                    origcontdir=random(2,4);
                    break;
                    
                case 8:
                    startnearx=thisleft;
                    startneary=thisup;
                    origcontdir=random(3,5);
                    break;
            }
            
            if (startnearx<0 || startnearx>width)
                startnearx=wrap(startnearx,width);
            
            if (startneary<0)
                startneary=0;
            
            if (startneary>height)
                startneary=height;
            
            if (origcontmountains==1)                makecontinentedgemountains(world,thiscontinent,continentnos,overlaps,baseheight,conheight,fractal,landshape,chainland,origstartpointx,origstartpointy,origcontwidth,origcontheight,startnearx,startneary,origcontdir,origmountainstartpointx,origmountainstartpointy);
            
        }
        
        // Now put other continents around it.
        
        bool fringeconts[3][3];
        
        for (int i=0; i<3; i++)
        {
            for (int j=0; j<3; j++)
                fringeconts[i][j]=0;
        }
        
        short extracont=random(1,9)-1; // Number of extra continents.
        
        for (int n=1; n<=extracont; n++)
        {
            thiscontinent++;
            
            makecontinent(world,continent,voronoi,points,width,height,leftx,rightx,lefty,righty);
            
            int thiscontwidth=rightx-leftx;
            int thiscontheight=righty-lefty;
            
            int dir=random(1,8); // Direction from the central continent.
            
            bool keepgoing=1;
            
            switch (dir)
            {
                case 1:
                    if (fringeconts[1][0]==1)
                        keepgoing=0;
                    break;
                    
                case 2:
                    if (fringeconts[2][0]==1)
                        keepgoing=0;
                    break;
                    
                case 3:
                    if (fringeconts[2][1]==1)
                        keepgoing=0;
                    break;
                    
                case 4:
                    if (fringeconts[2][2]==1)
                        keepgoing=0;
                    break;
                    
                case 5:
                    if (fringeconts[1][2]==1)
                        keepgoing=0;
                    break;
                    
                case 6:
                    if (fringeconts[0][2]==1)
                        keepgoing=0;
                    break;
                    
                case 7:
                    if (fringeconts[0][1]==1)
                        keepgoing=0;
                    break;
                    
                case 8:
                    if (fringeconts[0][0]==1)
                        keepgoing=0;
                    break;
            }
            
            if (keepgoing)
            {
                int thisstartpointx=-1;
                int thisstartpointy=-1; // Coordinates of the top left-hand corner of this continent.
                
                switch (dir)
                {
                    case 1: // north
                        thisstartpointx=origstartpointx+randomsign(random(0,origcontwidth/2));
                        thisstartpointy=focuspoints[thispoint].y-thiscontheight/2-(height-thiscontheight)/2-thiscontheight+random(0,movein);
                        fringeconts[1][0]=1;
                        break;
                        
                    case 2: // northeast
                        thisstartpointx=focuspoints[thispoint].x-thiscontwidth/2-(width-thiscontwidth)/2+origcontwidth-random(0,movein);
                        thisstartpointy=focuspoints[thispoint].y-thiscontheight/2-(height-thiscontheight)/2-thiscontheight+random(0,movein);
                        fringeconts[2][0]=1;
                        break;
                        
                    case 3: // east
                        thisstartpointx=focuspoints[thispoint].x-thiscontwidth/2-(width-thiscontwidth)/2+origcontwidth-random(0,movein);
                        thisstartpointy=origstartpointy+randomsign(random(0,origcontheight/2));
                        fringeconts[2][1]=1;
                        break;
                        
                    case 4: // southeast
                        thisstartpointx=focuspoints[thispoint].x-thiscontwidth/2-(width-thiscontwidth)/2+origcontwidth-random(0,movein);
                        thisstartpointy=focuspoints[thispoint].y-thiscontheight/2-(height-thiscontheight)/2+thiscontheight-random(0,movein);
                        fringeconts[2][2]=1;
                        break;
                        
                    case 5: // south
                        thisstartpointx=origstartpointx+randomsign(random(0,origcontwidth/2));
                        thisstartpointy=focuspoints[thispoint].y-thiscontheight/2-(height-thiscontheight)/2+thiscontheight-random(0,movein);
                        fringeconts[1][2]=1;
                        break;
                        
                    case 6: // southwest
                        thisstartpointx=focuspoints[thispoint].x-thiscontwidth/2-(width-thiscontwidth)/2-thiscontwidth+random(0,movein);
                        thisstartpointy=focuspoints[thispoint].y-thiscontheight/2-(height-thiscontheight)/2+thiscontheight-random(0,movein);
                        fringeconts[0][2]=1;
                        break;
                        
                    case 7: // west
                        thisstartpointx=focuspoints[thispoint].x-thiscontwidth/2-(width-thiscontwidth)/2-thiscontwidth+random(0,movein);
                        thisstartpointy=origstartpointy+randomsign(random(0,origcontheight/2));
                        fringeconts[0][1]=1;
                        break;
                        
                    case 8: // northwest
                        thisstartpointx=focuspoints[thispoint].x-thiscontwidth/2-(width-thiscontwidth)/2-thiscontwidth+random(0,movein);
                        thisstartpointy=focuspoints[thispoint].y-thiscontheight/2-(height-thiscontheight)/2-thiscontheight+random(0,movein);
                        fringeconts[0][0]=1;
                        break;
                }
                
                bool leftr=random(0,1); // If it's 1 then we reverse it left-right
                bool downr=random(0,1); // If it's 1 then we reverse it top-bottom
                
                int istart=leftx, desti=rightx, istep=1;
                int jstart=lefty, destj=righty, jstep=1;
                
                if (leftr==1)
                {
                    istart=rightx;
                    desti=leftx;
                    istep=-1;
                }
                
                if (downr==1)
                {
                    jstart=righty;
                    destj=lefty;
                    jstep=-1;
                }
                
                int imap=-1;
                int jmap=-1;
                
                for (int i=istart; i!=desti; i=i+istep)
                {
                    imap++;
                    jmap=-1;
                    
                    int ii=thisstartpointx+i;
                    
                    if (ii<0 || ii>width)
                        ii=wrap(ii,width);
                    
                    for (int j=jstart; j!=destj; j=j+jstep)
                    {
                        jmap++;
                        
                        int jj=thisstartpointy+j;
                        
                        if (jj>=0 && jj<=height)
                        {
                            if (continent[imap+leftx][jmap+lefty]==1)
                            {
                                world.setnom(ii,jj,conheight);
                                
                                if (continentnos[ii][jj]!=0)
                                {
                                    short overlap=continentnos[ii][jj]*100+thiscontinent;
                                    overlaps[ii][jj]=overlap;
                                }
                                
                                continentnos[ii][jj]=thiscontinent;
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Now remove inland seas.
    
    worldgenerationlabel.set_caption("Removing inland seas");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    removeseas(world,conheight);
    
    worldgenerationlabel.set_caption("Adding continental mountain ranges");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    // Now we add mountain ranges where continents overlap.
    
    int doneoverlaps[50]; // This will note down all the ones we've done.
    
    for (int n=0; n<50; n++)
        doneoverlaps[n]=0;
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (overlaps[i][j]!=0)
            {
                int thisoverlap=overlaps[i][j];
                
                bool donethisone=0;
                
                for (int n=0; n<50; n++)
                {
                    if (doneoverlaps[n]==thisoverlap)
                    {
                        donethisone=1;
                        n=50;
                    }
                }
                
                if (donethisone==0)
                {
                    if (world.outline(i,j)==1)
                    {
                        for (int n=0; n<50; n++)
                        {
                            if (doneoverlaps[n]==0)
                            {
                                doneoverlaps[n]=thisoverlap;
                                n=50;
                            }
                        }
                        
                        int furthestx=-1;
                        int furthesty=-1;
                        int dist=0;
                        
                        for (int k=0; k<=width; k++) // Find the furthest point that's in the same overlap.
                        {
                            for (int l=0; l<=height; l++)
                            {
                                if (overlaps[k][l]==thisoverlap)
                                {
                                    int xdist=i-k;
                                    int ydist=j-l;
                                    
                                    int thisdist=xdist*xdist+ydist+ydist;
                                    
                                    if (thisdist>dist)
                                    {
                                        dist=thisdist;
                                        furthestx=k;
                                        furthesty=l;
                                    }
                                }
                            }
                        }
                        
                        if (dist!=0)
                        {
                            vector <twointegers> dummy1(2);
                            
                            vector<vector<bool>> dummy2(2,vector<bool>(2,0));
                            
                            createdirectedchain(world,baseheight,conheight,1,continentnos,fractal,landshape,chainland,i,j,furthestx,furthesty,0,dummy1,dummy2,200);
                            
                        }
                    }
                }
            }
        }
    }
    
    twointegers mountainfocuspoints[4]; // These will be points where mountains and islands will start close to.
    
    int mountainfocustotal=random(2,4); // The number of actual focus points.
    
    for (int n=0; n<mountainfocustotal; n++)
    {
        mountainfocuspoints[n].x=random(0,width);
        mountainfocuspoints[n].y=random(0,height);
    }
    
    int mountainfocaldistance=height/2;
    
    // Now we add smaller mountain chains that might form peninsulas.
    
    createchains(world,baseheight,conheight,fractal,plateaumap,landshape,chainland,mountainfocuspoints,mountainfocustotal,mountainfocaldistance,1);
    
    // Now we do the continental shelves.
    
    worldgenerationlabel.set_caption("Making continental shelves");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    makecontinentalshelves(world,shelves,4);
    
    // Now we add some island chains.
    
    int islandgroups=random(0,6); // Number of general groups of islands.
    
    for (int n=1; n<=islandgroups; n++)
    {
        mountainfocustotal=1; // The number of actual focus points.
        
        for (int n=0; n<mountainfocustotal; n++)
        {
            mountainfocuspoints[n].x=random(0,width);
            mountainfocuspoints[n].y=random(0,height);
        }
        
        mountainfocaldistance=random(height/8,height/4);
        createchains(world,baseheight,conheight,fractal,plateaumap,landshape,chainland,mountainfocuspoints,mountainfocustotal,mountainfocaldistance,3);
    }
}

// This function creates the global climate.

void generateglobalclimate(planet &world, nanogui::Screen &screen, nanogui::Window &worldgenerationwindow, nanogui::Label &worldgenerationlabel, nanogui::ProgressBar &worldprogress, float progressstep, boolshapetemplate smalllake[], boolshapetemplate largelake[], boolshapetemplate landshape[], vector<vector<int>> &mountaindrainage, vector<vector<bool>> &shelves)
{
    long seed=world.seed();
    fast_srand(seed);
    
    int width=world.width();
    int height=world.height();
    int maxelev=world.maxelevation();
    
    // First, do the wind map.
    
    worldgenerationlabel.set_caption("Generating wind map");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    createwindmap(world);
    
    // Now create the temperature map.
    
    worldgenerationlabel.set_caption("Generating global temperature map");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    // Start by generating a new fractal map.
    
    int grain=8; // Level of detail on this fractal map.
    float valuemod=0.2f;
    int v=random(1,4);
    float valuemod2=float(v);
    
    vector<vector<int>> fractal(ARRAYWIDTH,vector<int>(ARRAYHEIGHT,0));
    
    createfractal(fractal,width,height,grain,valuemod,valuemod2,1,maxelev,0,0);
    
    createtemperaturemap(world,fractal);
    
    // Now do the sea ice.
    
    worldgenerationlabel.set_caption("Generating sea ice map");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    flip(fractal,width,height,1,1);
    int offset=random(1,width);
    shift(fractal,width,height,offset);
    
    createseaicemap(world,fractal);
    
    // Work out the tidal ranges.
    
    worldgenerationlabel.set_caption("Calculating tides");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    createtidalmap(world);
    
    // Now do rainfall.
    
    flip(fractal,width,height,1,1);
    offset=random(1,width);
    shift(fractal,width,height,offset);
    
    createrainmap(world,screen,worldgenerationlabel,worldprogress,progressstep,fractal,smalllake,landshape);
    
    // Now add fjord mountains.
    
    worldgenerationlabel.set_caption("Carving fjords");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    addfjordmountains(world);
    
    // Now work out the rivers initially. We do this the first time so that after the first time we can place the salt lakes in appropriate places, and then we work out the rivers again.
    
    worldgenerationlabel.set_caption("Planning river courses");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    createrivermap(world,mountaindrainage);
    
    // Now create salt lakes.
    
    worldgenerationlabel.set_caption("Placing hydrological basins");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    vector<vector<vector<int>>> saltlakemap(ARRAYWIDTH,vector<vector<int>>(ARRAYHEIGHT, vector<int>(2)));
    vector<vector<int>> nolake(ARRAYWIDTH,vector<int>(ARRAYHEIGHT,0));
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            saltlakemap[i][j][0]=0;
            saltlakemap[i][j][1]=0;
            nolake[i][j]=0;
        }
    }
    
    // First we need to prepare a no-lake template, marking out areas too close to the coasts, where lakes can't go.
    
    int minseadistance=15; // Points closer to the shore than this can't be the centre of lakes, normally.
    int minseadistance2=8; // This is for any lake tile, not just the centre.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (world.outline(i,j)==1)
            {
                for (int k=i-minseadistance2; k<=i+minseadistance2; k++)
                {
                    int kk=k;
                    
                    if (kk<0 || kk>width)
                        kk=wrap(kk,width);
                    
                    for (int l=j-minseadistance2; l<=j+minseadistance2; l++)
                    {
                        if (l>=0 && l<=height)
                            nolake[kk][l]=1;
                    }
                }
                
                for (int k=i-minseadistance; k<=i+minseadistance; k++)
                {
                    int kk=k;
                    
                    if (kk<0 || kk>width)
                        kk=wrap(kk,width);
                    
                    for (int l=j-minseadistance; l<=j+minseadistance; l++)
                    {
                        if (l>=0 && l<=height && nolake[kk][l]==0)
                            nolake[kk][l]=2;
                    }
                }
            }
        }
    }
    
    createsaltlakes(world,saltlakemap,nolake,smalllake);
    
    addlandnoise(world);
    depressionfill(world);
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            world.setriverdir(i,j,0);
            world.setriverjan(i,j,0);
            world.setriverjul(i,j,0);
        }
    }
    
    // Now work out the rivers again.
    
    worldgenerationlabel.set_caption("Generating rivers");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    createrivermap(world,mountaindrainage);
    
    // Now check river valleys in mountains.
    
    worldgenerationlabel.set_caption("Checking mountain river valleys");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    removerivermountains(world);
    
    // Now create the lakes.
    
    worldgenerationlabel.set_caption("Generating lakes");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    convertsaltlakes(world,saltlakemap);
    
    createlakemap(world,nolake,smalllake,largelake);
    
    createriftlakemap(world,nolake);
    
    world.setmaxriverflow();
    
    // Now create the climate map.
    
    worldgenerationlabel.set_caption("Calculating climates");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    createclimatemap(world);
    
    // Now specials.
    
    worldgenerationlabel.set_caption("Generating sand dunes");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    createergs(world,smalllake,largelake);
    
    worldgenerationlabel.set_caption("Generating salt pans");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    createsaltpans(world,smalllake,largelake);
    
    // Add river deltas.
    
    worldgenerationlabel.set_caption("Generating river deltas");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    createriverdeltas(world);
    checkrivers(world);
    
    // Now wetlands.
    
    worldgenerationlabel.set_caption("Generating wetlands");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    createwetlands(world,smalllake);
    
    removeexcesswetlands(world);
    
    // Now it's time to finesse the roughness map.
    
    worldgenerationlabel.set_caption("Refining roughness map");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    refineroughnessmap(world);
    
    // Check the rift lake map too.
    
    for (int i=0; i<ARRAYWIDTH; i++)
    {
        for (int j=0; j<ARRAYHEIGHT; j++)
        {
            if (world.lakestart(i,j)==1 && world.riftlakesurface(i,j)==0 && world.lakesurface(i,j)==0)
                world.setlakestart(i,j,0);
        }
    }
    
    // Finally, check that the climates at the edges of the map are correct.
    
    worldgenerationlabel.set_caption("Checking poles");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    checkpoleclimates(world);
    
    removesealakes(world); // Also, make sure there are no weird bits of sea next to lakes.
    
    connectlakes(world); // Make sure lakes aren't fragmented.
}

// Rain map creator.

void createrainmap(planet &world, nanogui::Screen &screen, nanogui::Label &worldgenerationlabel, nanogui::ProgressBar &worldprogress, float progressstep, vector<vector<int>> &fractal, boolshapetemplate smalllake[], boolshapetemplate shape[])
{
    int slopewaterreduce=20; // The higher this is, the less extra rain falls on slopes.
    int maxmountainheight=100;
    
    vector<vector<int>> inland(ARRAYWIDTH,vector<int>(ARRAYHEIGHT,0));
    
    // First, do rainfall over the oceans.
    
    worldgenerationlabel.set_caption("Calculating ocean rainfall");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    createoceanrain(world,fractal);
    
    // Now we do the rainfall over land.
    
    worldgenerationlabel.set_caption("Calculating rainfall from prevailing winds");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    createprevailinglandrain(world,inland,maxmountainheight,slopewaterreduce);
    
    // Now for monsoons!
    
    worldgenerationlabel.set_caption("Calculating monsoons");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    createmonsoons(world,maxmountainheight,slopewaterreduce);
    
    // Now increase the seasonal variation in rainfall in certain areas, to encourage Mediterranean climates.
    
    worldgenerationlabel.set_caption("Calculating seasonal rainfall");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    adjustseasonalrainfall(world,inland);
    
    // Now smooth the rainfall.
    
    worldgenerationlabel.set_caption("Smoothing rainfall");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    smoothrainfall(world,maxmountainheight);
    
    // Now cap excessive rainfall.
    
    worldgenerationlabel.set_caption("Capping rainfall");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    caprainfall(world);
    
    // Now we adjust temperatures in light of rainfall.
    
    worldgenerationlabel.set_caption("Adjusting temperatures");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    adjusttemperatures(world,inland);
    
    // Now make temperatures a little more extreme when further from the sea.
    
    worldgenerationlabel.set_caption("Adjusting continental temperatures");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    adjustcontinentaltemperatures(world,inland);
    
    // Now just smooth the temperatures a bit. Any temperatures that are lower than their neighbours to north and south get bumped up, to avoid the appearance of streaks.
    
    worldgenerationlabel.set_caption("Smoothing temperatures");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    smoothtemperatures(world);
    
    // Now just prevent subpolar climates from turning into other continental types when further from the sea.
    
    worldgenerationlabel.set_caption("Checking subpolar regions");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    removesubpolarstreaks(world);
    extendsubpolar(world);
    removesubpolarstreaks(world);
    
    // Now we just sort out the mountain precipitation arrays, which will be used at the regional level for ensuring that higher mountain precipitation isn't splashed too far.
    
    worldgenerationlabel.set_caption("Calculating mountain rainfall");
    worldprogress.set_value(worldprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    createmountainprecipitation(world);
}

// This function creates the region.

void generateregionalmap(planet &world, region &region, nanogui::Screen &screen, nanogui::ProgressBar &regionprogress, float progressstep, boolshapetemplate smalllake[], boolshapetemplate island[], peaktemplate &peaks, vector<vector<float>> &riftblob, int riftblobsize, int partial, byteshapetemplate smudge[], byteshapetemplate smallsmudge[])
{
    int xleft=0;
    int xright=35;
    int ytop=0;
    int ybottom=35;
    
    if (partial==7)
        xright=2+STRIPWIDTH;
    
    int xstart=xleft*16;
    int xend=xright*16+16;
    int ystart=ytop*16;
    int yend=ybottom*16+16; // These define the actual area of the regional map that we're creating (with some margin).
    
    int leftx=region.leftx();
    int lefty=region.lefty(); // These mark the upper left corner of the part of the global map we're expanding on.
    
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    int maxelev=world.maxelevation();
    
    int rwidth=region.rwidth();
    int rheight=region.rheight();
    
    if (leftx<0 || leftx>width)
    {
        leftx=wrap(leftx,width);
        region.setleftx(leftx);
    }
    
    if (partial==0)
        region.clear();
    
    vector<vector<bool>> safesaltlakes(RARRAYWIDTH,vector<bool>(RARRAYHEIGHT,0));
    vector<vector<bool>> disruptpoints(RARRAYWIDTH,vector<bool>(RARRAYHEIGHT,0));
    vector<vector<int>> rriverscarved(RARRAYWIDTH,vector<int>(RARRAYHEIGHT,0));
    vector<vector<int>> fakesourcex(RARRAYWIDTH,vector<int>(RARRAYHEIGHT,0));
    vector<vector<int>> fakesourcey(RARRAYWIDTH,vector<int>(RARRAYHEIGHT,0));
    vector<vector<bool>> riverinlets(RARRAYWIDTH,vector<bool>(RARRAYHEIGHT,0));
    
    // Counter-intuitively, we do the lakes and rivers first. Rivers are drawn at a somewhat lower elevation than the rest of the tile will be. After that, we draw in the rest of the elevation around them. This creates the effect of rivers carving out valleys in the landscape, when in fact the landscape is built around the rivers.
    
    makeregionalwater(world,region,screen,regionprogress,progressstep,safesaltlakes,disruptpoints,rriverscarved,fakesourcex,fakesourcey,smalllake,island,riftblob,riftblobsize,xleft,xright,ytop,ybottom);
    
    // Only now, surprisingly, do we do basic elevation.
    
    makeregionalterrain(world,region,screen,regionprogress,progressstep,disruptpoints,riverinlets,rriverscarved,fakesourcex,fakesourcey,smalllake,peaks,smallsmudge,xleft,xright,ytop,ybottom);
    
    // Now do the undersea terrain.
    
    makeregionalunderseaterrain(world,region,screen,regionprogress,progressstep,peaks,smudge,smallsmudge,xleft,xright,ytop,ybottom);
    
    // Now do various miscellaneous bits.
    
    makeregionalmiscellanies(world,region,screen,regionprogress,progressstep,safesaltlakes,riverinlets,smalllake,smallsmudge,xleft,xright,ytop,ybottom);
    
    // Now we do the climates.
    
    makeregionalclimates(world,region,screen,regionprogress,progressstep,safesaltlakes,smalllake,xleft,xright,ytop,ybottom);
}

// This does the regional rivers and lakes.

void makeregionalwater(planet &world, region &region, nanogui::Screen &screen, nanogui::ProgressBar &regionprogress, float progressstep, vector<vector<bool>> &safesaltlakes, vector<vector<bool>> &disruptpoints, vector<vector<int>> &rriverscarved, vector<vector<int>> &fakesourcex, vector<vector<int>> &fakesourcey, boolshapetemplate smalllake[], boolshapetemplate island[], vector<vector<float>> &riftblob, int riftblobsize, int xleft, int xright, int ytop, int ybottom)
{
    int xstart=xleft*16;
    int xend=xright*16+16;
    int ystart=ytop*16;
    int yend=ybottom*16+16; // These define the actual area of the regional map that we're creating (with some margin).
    
    int leftx=region.leftx();
    int lefty=region.lefty(); // These mark the upper left corner of the part of the global map we're expanding on.
    
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    int maxelev=world.maxelevation();
    
    int rwidth=region.rwidth();
    int rheight=region.rheight();
    
    if (leftx<0 || leftx>width)
    {
        leftx=wrap(leftx,width);
        region.setleftx(leftx);
    }
    
    vector<vector<int>> source(ARRAYWIDTH,vector<int>(ARRAYHEIGHT,0));
    vector<vector<int>> destination(RARRAYWIDTH,vector<int>(ARRAYHEIGHT,0));
    vector<vector<bool>> lakemap(RARRAYWIDTH*4,vector<bool>(RARRAYHEIGHT*4,0));
    vector<vector<bool>> lakeislands(RARRAYWIDTH,vector<bool>(RARRAYHEIGHT,0));
    vector<vector<bool>> rivercurves(RARRAYWIDTH,vector<bool>(RARRAYHEIGHT,0));
    vector<vector<bool>> riftlakemap(RARRAYWIDTH*2,vector<bool>(RARRAYHEIGHT*2,0));
    
    int coords[4][2];
    
    // First, sort out the roughness.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
            source[i][j]=world.roughness(i,j)*12000;
    }
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop+1; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            float valuemod=0.1;
            
            makegenerictile(world,x*16,y*16,xx,yy,valuemod,coords,source,destination,100000,0,0);
        }
    }
    
    for (int i=xstart; i<=xend; i++)
    {
        for (int j=ystart; j<=yend; j++)
        {
            float roughness=destination[i][j];
            roughness=roughness/15000; // 10000 originally. I changed it because it was sometimes just too rough.
            
            if (roughness<0.2) // Because perfect smoothness looks a bit unnatural.
                roughness=0.2;
            
            region.setroughness(i,j,roughness);
        }
    }
    
    // Now large lakes.
    
    int extra=25; // For these, we will create a *bigger* map than the actual regional area. This is so that we can work out the whole coastline of lakes even if only a part of them appears on the regional map. extra is the amount of additional margin to put around the regional map for this, measured in tiles.
    
    // Start by marking the edges of the lake tiles on this map.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft-extra; x<=xright+extra; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop-extra; y<=ybottom+extra; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            if (world.lakesurface(xx,yy)!=0)
                marklakeedges(world,region,x*16,y*16,xx,yy,extra,lakemap);
        }
    }
    
    // Now, turn those edges into borders.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft-extra; x<=xright+extra; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop-extra; y<=ybottom+extra; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            if (world.lakestart(xx,yy)!=0 && world.riftlakesurface(xx,yy)==0)
                makelaketemplates(world,region,x*16,y*16,xx,yy,extra,lakemap);
        }
    }
    
    // Now, actually create the lakes.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft-extra; x<=xright+extra; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop-extra; y<=ybottom+extra; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            int surfacelevel=nearlake(world,xx,yy,1,0);
            
            if (surfacelevel!=0)
                makelaketile(world,region,x*16,y*16,xx,yy,extra,lakemap,surfacelevel,coords,source,destination,safesaltlakes,smalllake);
        }
    }
    
    // Check for any diagonal sections in the lake coastlines.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            findlakecoastdiagonals(world,region,x*16,y*16,xx,yy,disruptpoints);
        }
    }
    
    // Now remove them.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            removelakecoastdiagonals(world,region,x*16,y*16,xx,yy,disruptpoints,smalllake);
        }
    }
    
    for (int i=0; i<RARRAYWIDTH; i++)
    {
        for (int j=0; j<RARRAYHEIGHT; j++)
            disruptpoints[i][j]=0;
    }
    
    // Now put islands in those lakes.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft-extra; x<=xright+extra; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop-extra; y<=ybottom+extra; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            int surfacelevel=nearlake(world,xx,yy,1,0);
            
            if (surfacelevel!=0)
                makelakeislands(world,region,x*16,y*16,xx,yy,surfacelevel,island,lakeislands);
        }
    }
    
    // Now salt pans around salt lakes.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            makesaltpantile(world,region,x*16,y*16,xx,yy,smalllake);
        }
    }
    
    // Now, rivers.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            makerivertile(world,region,x*16,y*16,xx,yy,rriverscarved,smalllake,rivercurves);
        }
    }
    
    // Now, fill in any missing sections of river.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    fixrivers(world,region,xstart,ystart,xend,yend);
    
    // Now rift lakes.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    extra=10; // For these, we will create a *bigger* map than the actual regional area. This is so that we can work out the whole coastline of lakes even if only a part of them appears on the regional map. extra is the amount of additional margin to put around the regional map for this.
    
    for (int x=xleft-extra; x<=xright+extra; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop-extra; y<=ybottom+extra; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            if (world.lakestart(xx,yy)!=0 && world.riftlakesurface(xx,yy)!=0)
                makeriftlaketemplates(world,region,x*16,y*16,xx,yy,extra,riftlakemap);
        }
    }
    
    // We've got the templates for the rift lakes. Now go through the map again and create the lakes from the templates.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            makeriftlaketile(world,region,x*16,y*16,xx,yy,extra,riftlakemap,riftblob,riftblobsize);
        }
    }
    
    // If any rivers inexplicably stop, extend them until they hit sea, lake, or another river.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    finishrivers(world,region,xstart,ystart,xend,yend);
    
    // Remove any sections of rivers that flow out of lakes and then back into the same lakes.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft+1; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop+1; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            removeriverlakeloops(world,region,x*16,y*16,xx,yy,smalllake);
        }
    }
    
    // Now, expand those rivers to their proper widths.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            expandrivers(world,region,x*16,y*16,xx,yy,0,fakesourcex,fakesourcey);
        }
    }
    
    // Remove weird river elevations.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            removeweirdelevations(world,region,x*16,y*16,xx,yy);
        }
    }
}

// This does the regional physical terrain.

void makeregionalterrain(planet &world, region &region, nanogui::Screen &screen, nanogui::ProgressBar &regionprogress, float progressstep, vector<vector<bool>> &disruptpoints, vector<vector<bool>> &riverinlets,vector<vector<int>> &rriverscarved, vector<vector<int>> &fakesourcex, vector<vector<int>> &fakesourcey, boolshapetemplate smalllake[], peaktemplate &peaks, byteshapetemplate smallsmudge[], int xleft, int xright, int ytop, int ybottom)
{
    int xstart=xleft*16;
    int xend=xright*16+16;
    int ystart=ytop*16;
    int yend=ybottom*16+16; // These define the actual area of the regional map that we're creating (with some margin).
    
    int leftx=region.leftx();
    int lefty=region.lefty(); // These mark the upper left corner of the part of the global map we're expanding on.
    
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    int maxelev=world.maxelevation();
    
    int rwidth=region.rwidth();
    int rheight=region.rheight();
    
    if (leftx<0 || leftx>width)
    {
        leftx=wrap(leftx,width);
        region.setleftx(leftx);
    }
    
    vector<vector<int>> rotatearray(RARRAYWIDTH,vector<int>(RARRAYHEIGHT,0));
    vector<vector<int>> rmountainmap(RARRAYWIDTH,vector<int>(RARRAYHEIGHT,0));
    vector<vector<int>> ridgeids(RARRAYWIDTH,vector<int>(RARRAYHEIGHT,0));
    vector<vector<int>> nearestridgepointdist(RARRAYWIDTH,vector<int>(RARRAYHEIGHT,0));
    vector<vector<int>> nearestridgepointx(RARRAYWIDTH,vector<int>(RARRAYHEIGHT,0));
    vector<vector<int>> nearestridgepointy(RARRAYWIDTH,vector<int>(RARRAYHEIGHT,0));
    vector<vector<bool>> mountainedges(RARRAYWIDTH,vector<bool>(RARRAYHEIGHT,0));
    vector<vector<bool>> buttresspoints(RARRAYWIDTH,vector<bool>(RARRAYHEIGHT,0));
    vector<vector<int>> pathchecked(RARRAYWIDTH,vector<int>(RARRAYHEIGHT,0));
    
    int coords[4][2];
    
    // Do basic elevation.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            makeelevationtile(world,region,x*16,y*16,xx,yy,coords,smalllake);
        }
    }
    
    // Make coastlines on single-tile islands more interesting.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            if (world.island(xx,yy)==1)
                
                complicatecoastlines(world,region,x*16,y*16,xx,yy,smalllake,8);
        }
    }
    
    // Check for any diagonal sections in the coastlines.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            findcoastdiagonals(world,region,x*16,y*16,xx,yy,disruptpoints);
        }
    }
    
    // Now remove them.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            removecoastdiagonals(world,region,x*16,y*16,xx,yy,disruptpoints,smalllake);
        }
    }
    
    // Ensure that delta regions are above sea level.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            removedeltasea(world,region,x*16,y*16,xx,yy);
        }
    }
    
    // Perform rotations on the edges of the tiles, to break up any grid-like artefacts. (This creates the nicely granular texture to many maps, especially in wetter climates.)
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            rotatetileedges(world,region,x*16,y*16,xx,yy,rotatearray,0);
        }
    }
    
    // Remove awkward straight lines on the elevation map. (Note: This is the function that adds the pretty but probably unrealistic terrace-like texturing to many maps, especially around mountains and deserts.)
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            addterraces(world,region,x*16,y*16,xx,yy,smalllake,smallsmudge);
        }
    }
    
    // Now ensure that there are no lakes bordering the sea.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    removesealakes(world,region,xstart,ystart,xend,yend);
    
    // Now, delta branches.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft+1; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop+1; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            makedeltatile(world,region,x*16,y*16,xx,yy,rriverscarved);
        }
    }
    
    // Now, expand those branches to their proper widths.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            expandrivers(world,region,x*16,y*16,xx,yy,1,fakesourcex,fakesourcey);
        }
    }
    
    // Now add the delta branches to the normal river map.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    adddeltamap(world,region,xstart,ystart,xend,yend);
    
    // Now remove any sinks from the land.
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            //removesinks(world,region,x*16,y*16,xx,yy);
        }
    }
    
    // Now we make the main mountain ridges.
    
    int maxridgedist=10;
    int maxmaxridgedist=maxridgedist*maxridgedist+maxridgedist*maxridgedist;
    int buttressspacing=14;
    short markgap=6;
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++) // Wider
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            makemountaintile(world,region,x*16,y*16,xx,yy,peaks,rmountainmap,ridgeids,markgap);
        }
    }
    
    // Add in shield volcanoes.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++) // Wider
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            if (world.sea(xx,yy)==0 && world.volcano(xx,yy)!=0 && world.strato(xx,yy)==0)
            {
                int templateno=random(1,2);
                
                makevolcano(world,region,x*16,y*16,xx,yy,peaks,rmountainmap,ridgeids,templateno);
            }
        }
    }
    
    // Now we find cells that are close to those ridges.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            assignridgeregions(world,region,x*16,y*16,xx,yy,rmountainmap,ridgeids,nearestridgepointdist,nearestridgepointx,nearestridgepointy,maxridgedist,maxmaxridgedist);
        }
    }
    
    // Now we find the edges of the mountain ranges.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            findmountainedges(world,region,x*16,y*16,xx,yy,nearestridgepointdist,nearestridgepointx,nearestridgepointy,mountainedges);
        }
    }
    
    // Now we identify the points where buttresses will end.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            int thisbuttressspacing=buttressspacing;
            
            if (world.sea(xx,yy)==1)
                thisbuttressspacing=thisbuttressspacing*2;
            
            findbuttresspoints(world,region,x*16,y*16,xx,yy,ridgeids,nearestridgepointdist,nearestridgepointx,nearestridgepointy,mountainedges,buttresspoints,maxmaxridgedist,thisbuttressspacing);
        }
    }
    
    // Now we actually do the buttresses.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int i=0; i<=rwidth; i++)
    {
        for (int j=0; j<=rheight; j++)
            ridgeids[i][j]=0;
    }
    
    markgap=4; // This is for marking where we want the mini-buttresses to go to.
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            //if (world.sea(xx,yy)==0)
            makebuttresses(world,region,x*16,y*16,xx,yy,rmountainmap,peaks,nearestridgepointx,nearestridgepointy,nearestridgepointdist,maxridgedist,buttresspoints,ridgeids,markgap,0);
        }
    }
    
    // Now do all that again for mini-buttresses!
    
    for (int i=0; i<=rwidth; i++)
    {
        for (int j=0; j<=rheight; j++)
        {
            nearestridgepointx[i][j]=0;
            nearestridgepointy[i][j]=0;
            nearestridgepointdist[i][j]=0;
            mountainedges[i][j]=0;
            buttresspoints[i][j]=0;
        }
    }
    
    maxridgedist=5;
    maxmaxridgedist=maxridgedist*maxridgedist+maxridgedist*maxridgedist;
    buttressspacing=3;
    
    // Find cells that are close to those buttresses.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            assignridgeregions(world,region,x*16,y*16,xx,yy,rmountainmap,ridgeids,nearestridgepointdist,nearestridgepointx,nearestridgepointy,maxridgedist,maxmaxridgedist);
        }
    }
    
    // Now we find the edges of the mountain ranges.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            findmountainedges(world,region,x*16,y*16,xx,yy,nearestridgepointdist,nearestridgepointx,nearestridgepointy,mountainedges);
        }
    }
    
    // Now we identify the points where mini-buttresses will end.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            findbuttresspoints(world,region,x*16,y*16,xx,yy,ridgeids,nearestridgepointdist,nearestridgepointx,nearestridgepointy,mountainedges,buttresspoints,maxmaxridgedist,buttressspacing);
        }
    }
    
    // Now we actually do the mini-buttresses.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            if (world.sea(xx,yy)==0)
                makebuttresses(world,region,x*16,y*16,xx,yy,rmountainmap,peaks,nearestridgepointx,nearestridgepointy,nearestridgepointdist,maxridgedist,buttresspoints,ridgeids,markgap,1);
        }
    }
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int i=xstart; i<=xend; i++) // Now put those mountains onto the regional map.
    {
        for (int j=ystart; j<=yend; j++)
        {
            if (region.map(i,j)<rmountainmap[i][j] && region.lakesurface(i,j)==0 && region.riverdir(i,j)==0)
            {
                region.setmap(i,j,rmountainmap[i][j]);
            }
        }
    }
    
    // Now do stratovolcanoes.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int i=xstart; i<=xend; i++) // Now put those mountains onto the regional map.
    {
        for (int j=ystart; j<=yend; j++)
            rmountainmap[i][j]=0;
    }
    
    for (int x=xleft; x<=xright; x++) // Wider
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            if (world.sea(xx,yy)==0 && world.strato(xx,yy)==1)
            {
                int templateno=1;
                
                makevolcano(world,region,x*16,y*16,xx,yy,peaks,rmountainmap,ridgeids,templateno);
            }
        }
    }
    
    for (int i=xstart; i<=xend; i++) // Now put those onto the regional map.
    {
        for (int j=ystart; j<=yend; j++)
        {
            if (region.map(i,j)<rmountainmap[i][j] && region.lakesurface(i,j)==0 && region.riverdir(i,j)==0)
            {
                region.setmap(i,j,rmountainmap[i][j]);
            }
        }
    }
    
    // Now, remove extra land around mountain islands.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            if (world.mountainisland(xx,yy)==1)
                trimmountainislands(world,region,x*16,y*16,xx,yy,rmountainmap);
        }
    }
    
    // Now we widen any diagonal waterways.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    removediagonalwater(region,xstart,ystart,xend,yend,sealevel);
    
    /*
     // Now remove pools. (Turned off for now as it creates weird artefacts, and the turnpoolstolakes routine pretty much catches them all anyway.)
     
     regionprogress.set_value(regionprogress.value()+progressstep);
     screen.redraw();
     screen.draw_all();
     
     int checkno=0;
     
     for (int x=xleft+1; x<xright; x++) // Note that we don't do the ones on the edges of the regional map.
     {
     int xx=leftx+x;
     
     if (xx<0 || xx>width)
     xx=wrap(xx,width);
     
     for (int y=ytop+1; y<ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
     {
     int yy=lefty+y;
     
     removepools(world,region,x*16,y*16,xx,yy,pathchecked,checkno);
     }
     }
     */
    
    // Remove straight edges on the coastlines again.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            removestraights(world,region,x*16,y*16,xx,yy,smalllake);
        }
    }
    
    // Now turn any pools that may be left into lakes.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    int checkno=0;
    
    for (int i=0; i<RARRAYWIDTH; i++)
    {
        for (int j=0; j<RARRAYHEIGHT; j++)
            
            pathchecked[i][j]=0;
    }
    
    for (int x=xleft+1; x<xright; x++) // Note that we don't do the ones on the edges of the regional map.
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop+1; y<ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            turnpoolstolakes(world,region,x*16,y*16,xx,yy,pathchecked,checkno);
        }
    }
    
    // Remove bits of lakes that touch the sea. (Turned off as it sometimes had the unfortunate effect of turning the entire sea into lake.)
    
    // //removelakesbysea(region,rstart,rstart,rwidth,rheight,sealevel);
    
    // Now, remove weird bits of rivers from inside seas.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            removeextrasearivers(world,region,x*16,y*16,xx,yy);
        }
    }
    
    // Now remove rivers that emerge from the sea onto land.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            removeriverscomingfromsea(world,region,x*16,y*16,xx,yy,fakesourcex,fakesourcey);
        }
    }
    
    // Now, add inlets to rivers.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            addinlets(world,region,x*16,y*16,xx,yy,riverinlets);
        }
    }
    
    // Now, remove extraneous islands from the sea.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x=x+2)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y=y+2) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            removesmallislands(world,region,x*16,y*16,xx,yy);
        }
    }
    
}

// This does the regional undersea terrain.

void makeregionalunderseaterrain(planet &world, region &region, nanogui::Screen &screen, nanogui::ProgressBar &regionprogress, float progressstep, peaktemplate &peaks, byteshapetemplate smudge[], byteshapetemplate smallsmudge[], int xleft, int xright, int ytop, int ybottom)
{
    int xstart=xleft*16;
    int xend=xright*16+16;
    int ystart=ytop*16;
    int yend=ybottom*16+16; // These define the actual area of the regional map that we're creating (with some margin).
    
    int leftx=region.leftx();
    int lefty=region.lefty(); // These mark the upper left corner of the part of the global map we're expanding on.
    
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    int maxelev=world.maxelevation();
    
    int rwidth=region.rwidth();
    int rheight=region.rheight();
    
    if (leftx<0 || leftx>width)
    {
        leftx=wrap(leftx,width);
        region.setleftx(leftx);
    }
    
    vector<vector<int>> underseamap(RARRAYWIDTH*4,vector<int>(RARRAYHEIGHT*4,0));
    vector<vector<bool>> undersearidgelines(RARRAYWIDTH,vector<bool>(RARRAYHEIGHT,0));
    vector<vector<int>> undersearidges(RARRAYWIDTH,vector<int>(RARRAYHEIGHT,0));
    vector<vector<int>> underseaspikes(RARRAYWIDTH*4,vector<int>(RARRAYHEIGHT*4,0));
    
    int coords[4][2];
    
    // Do submarine elevation.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    int extra=20; // For these again, we will create a *bigger* map than the actual regional area. This is because the templates that we're going to use to disrupt this terrain are quite big, so we need to have a big margin around the visible area to ensure that every tile looks the same no matter where the regional map is centred.
    
    for (int x=xleft-extra; x<=xright+extra; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop-extra; y<=ybottom+extra; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            makesubmarineelevationtile(world,region,x*16,y*16,xx,yy,underseamap,coords,extra);
        }
    }
    
    // Now disrupt that elevation a bit.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    // The sweep over the whole region needs to be staggered to keep the disruption evenly distributed.
    
    for (int x=xleft-extra; x<=xright+extra; x=x+2)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop-extra; y<=ybottom+extra; y=y+2) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            if (world.sea(xx,yy)==1)
                disruptsubmarineelevationtile(world,region,x*16,y*16,xx,yy,underseamap,smudge,extra);
        }
    }
    
    for (int x=xright+extra-1; x>=xleft-extra; x=x-2)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ybottom+extra; y>=ytop-extra; y=y-2) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            if (world.sea(xx,yy)==1)
                disruptsubmarineelevationtile(world,region,x*16,y*16,xx,yy,underseamap,smudge,extra);
        }
    }
    
    for (int x=xleft-extra; x<=xright+extra; x=x+2)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop-extra+1; y<=ybottom+extra; y=y+2) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            if (world.sea(xx,yy)==1)
                disruptsubmarineelevationtile(world,region,x*16,y*16,xx,yy,underseamap,smudge,extra);
        }
    }
    
    for (int x=xright+extra-1; x>=xleft-extra; x=x-2)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ybottom+extra-1; y>=ytop-extra; y=y-2) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            if (world.sea(xx,yy)==1)
                disruptsubmarineelevationtile(world,region,x*16,y*16,xx,yy,underseamap,smudge,extra);
        }
    }
    
    for (int i=0; i<=rwidth; i++)
    {
        for (int j=0; j<=rheight; j++)
        {
            if (region.sea(i,j))
                region.setmap(i,j,underseamap[i+extra*16][j+extra*16]);
        }
    }
    
    
    // Now we find the paths of the submarine ridges.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            if (world.oceanridges(xx,yy)!=0)
                makesubmarineridgelines(world,region,x*16,y*16,xx,yy,undersearidgelines);
        }
    }
    
    // Now we draw the ridges onto the ridge map.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            if (world.sea(xx,yy)==1)
                drawsubmarineridges(world,region,x*16,y*16,xx,yy,undersearidgelines,peaks,undersearidges);
        }
    }
    
    // Now we add the central mountains to the ridges.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            if (world.oceanrifts(xx,yy)!=0)
                makesubmarineriftmountains(world,region,x*16,y*16,xx,yy,undersearidges,peaks);
        }
    }
    
    // Now we carve out the central rift valley.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            if (world.oceanrifts(xx,yy)!=0)
                makesubmarinerift(world,region,x*16,y*16,xx,yy,undersearidges,smallsmudge);
        }
    }
    
    // Now we add the spikes radiating away from the rifts.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    extra=20; // This one has to be done with a wide margin around the visible map, as there could be spikes coming in from outside.
    
    for (int x=xleft-extra; x<=xright+extra; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop-extra; y<=ybottom+extra; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            if (world.oceanrifts(xx,yy)!=0)
                makesubmarineriftradiations(world,region,x*16,y*16,xx,yy,underseaspikes,peaks,extra);
        }
    }
    
    // Apply the spikes map to the ridges map.
    
    int dextra=extra*16;
    
    for (int i=0; i<=rwidth; i++)
    {
        for (int j=0; j<=rheight; j++)
        {
            if (region.sea(i,j))
            {
                if (underseaspikes[i+dextra][j+dextra]<0)
                {
                    undersearidges[i][j]=undersearidges[i][j]+underseaspikes[i+dextra][j+dextra];
                    
                    if (undersearidges[i][j]<0)
                        undersearidges[i][j]=0;
                }
                else
                {
                    if (undersearidges[i][j]<underseaspikes[i+dextra][j+dextra])
                        undersearidges[i][j]=underseaspikes[i+dextra][j+dextra];
                }
            }
        }
    }
    
    // Now, add submarine volcanoes (seamounts).
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft-extra; x<=xright+extra; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop-extra; y<=ybottom+extra; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            if (world.sea(xx,yy)==1 && world.volcano(xx,yy)!=0)
                makesubmarinevolcano(world,region,x*16,y*16,xx,yy,peaks,undersearidges);
        }
    }
    
    // Apply the ridges to the map.
    
    for (int i=0; i<=rwidth; i++)
    {
        for (int j=0; j<=rheight; j++)
            region.setmap(i,j,region.map(i,j)+undersearidges[i][j]);
    }
}

// This does some miscellaneous stuff to the regional map.

void makeregionalmiscellanies(planet &world, region &region, nanogui::Screen &screen, nanogui::ProgressBar &regionprogress, float progressstep, vector<vector<bool>> &safesaltlakes, vector<vector<bool>> &riverinlets, boolshapetemplate smalllake[], byteshapetemplate smallsmudge[], int xleft, int xright, int ytop, int ybottom)
{
    int xstart=xleft*16;
    int xend=xright*16+16;
    int ystart=ytop*16;
    int yend=ybottom*16+16; // These define the actual area of the regional map that we're creating (with some margin).
    
    int leftx=region.leftx();
    int lefty=region.lefty(); // These mark the upper left corner of the part of the global map we're expanding on.
    
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    int maxelev=world.maxelevation();
    
    int rwidth=region.rwidth();
    int rheight=region.rheight();
    
    if (leftx<0 || leftx>width)
    {
        leftx=wrap(leftx,width);
        region.setleftx(leftx);
    }
    
    vector<vector<int>> rotatearray(RARRAYWIDTH,vector<int>(RARRAYHEIGHT,0));
    
    //int coords[4][2];
    
    // Now, wetlands.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            makewetlandtile(world,region,x*16,y*16,xx,yy,smalllake);
        }
    }
    
    // Make sure special "lakes" are correctly identified.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            convertlakestospecials(world,region,x*16,y*16,xx,yy,safesaltlakes);
        }
    }
    
    // Now we do barrier islands.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            addbarrierislands(world,region,x*16,y*16,xx,yy,riverinlets);
        }
    }
    
    // Now, remove odd little parallel lines of islands that sometimes get created by the barrier islands routine.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    removeparallelislands(region,xstart,ystart,xend,yend,sealevel);
    
    // Check for any impossibly low areas of elevation.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            removetoolow(world,region,x*16,y*16,xx,yy);
        }
    }
    
    // Now remove any bits of sea that are next to lakes.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            removelakeseas(world,region,x*16,y*16,xx,yy);
        }
    }
    
    // Now smooth lake beds.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    smoothlakebeds(region);
    
    // Now do rotations on the lake beds.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            rotatetileedges(world,region,x*16,y*16,xx,yy,rotatearray,1);
        }
    }
    
    // Now remove any weirdly high elevation.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            removetoohighelevations(world,region,x*16,y*16,xx,yy);
        }
    }
    
    // Now disrupt lake beds again.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            addlaketerraces(world,region,x*16,y*16,xx,yy,smalllake,smallsmudge);
        }
    }
    
    // Now we do glaciers.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            addregionalglaciers(world,region,x*16,y*16,xx,yy);
        }
    }
}

// This does the regional climates.

void makeregionalclimates(planet &world, region &region, nanogui::Screen &screen, nanogui::ProgressBar &regionprogress, float progressstep, vector<vector<bool>> &safesaltlakes, boolshapetemplate smalllake[], int xleft, int xright, int ytop, int ybottom)
{
    int xstart=xleft*16;
    int xend=xright*16+16;
    int ystart=ytop*16;
    int yend=ybottom*16+16; // These define the actual area of the regional map that we're creating (with some margin).
    
    int leftx=region.leftx();
    int lefty=region.lefty(); // These mark the upper left corner of the part of the global map we're expanding on.
    
    int width=world.width();
    int height=world.height();
    int sealevel=world.sealevel();
    int maxelev=world.maxelevation();
    
    int rwidth=region.rwidth();
    int rheight=region.rheight();
    
    if (leftx<0 || leftx>width)
    {
        leftx=wrap(leftx,width);
        region.setleftx(leftx);
    }
    
    vector<vector<int>> source(ARRAYWIDTH,vector<int>(ARRAYHEIGHT,0));
    vector<vector<int>> destination(RARRAYWIDTH,vector<int>(ARRAYHEIGHT,0));
    vector<vector<int>> rotatearray(RARRAYWIDTH,vector<int>(RARRAYHEIGHT,0));
    
    int coords[4][2];
    
    // First we do the precipitation maps.
    
    // First, winter precipitation.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int i=0; i<RARRAYWIDTH; i++)
    {
        for (int j=0; j<RARRAYHEIGHT; j++)
        {
            destination[i][j]=-5000;
            rotatearray[i][j]=0;
        }
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
            
            makegenerictile(world,x*16,y*16,xx,yy,valuemod,coords,source,destination,100000,0,1);
            
            if (yy==height) // If this is the very bottom of the global map, ensure there's no weirdness.
            {
                int maxval=world.winterrain(xx,yy);
                
                for (int i=x*16; i<=x*16+16; i++)
                {
                    for (int j=y*16; j<=y*16+16; j++)
                    {
                        if (destination[i][j]>maxval)
                            destination[i][j]=maxval;
                    }
                }
            }
        }
    }
    
    // Now add rotations to that precipitation map, to improve the look.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft+2; x<xright-1; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop+2; y<ybottom-1; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            rotatetileedgesarray(world,region,x*16,y*16,xx,yy,destination,rotatearray,-5000);
        }
    }
    
    for (int i=xstart; i<=xend; i++)
    {
        for (int j=ystart; j<=yend; j++)
            region.setwinterrain(i,j,destination[i][j]);
    }
    
    // Now, summer precipitation.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int i=0; i<RARRAYWIDTH; i++)
    {
        for (int j=0; j<RARRAYHEIGHT; j++)
        {
            destination[i][j]=-5000;
            rotatearray[i][j]=0;
        }
    }
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            if (world.summermountainraindir(i,j)==0)
                source[i][j]=world.summerrain(i,j);
            else
                source[i][j]=world.summermountainrain(i,j);
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
            
            makegenerictile(world,x*16,y*16,xx,yy,valuemod,coords,source,destination,100000,0,1);
            
            if (yy==height) // If this is the very bottom of the global map, ensure there's no weirdness.
            {
                int maxval=world.summerrain(xx,yy);
                
                for (int i=x*16; i<=x*16+16; i++)
                {
                    for (int j=y*16; j<=y*16+16; j++)
                    {
                        if (destination[i][j]>maxval)
                            destination[i][j]=maxval;
                    }
                }
            }
        }
    }
    
    // Now add rotations to that precipitation map, to improve the look.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft+2; x<xright-1; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop+2; y<ybottom-1; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            rotatetileedgesarray(world,region,x*16,y*16,xx,yy,destination,rotatearray,-5000);
        }
    }
    
    for (int i=xstart; i<=xend; i++)
    {
        for (int j=ystart; j<=yend; j++)
            region.setsummerrain(i,j,destination[i][j]);
    }
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    smoothprecipitation(region,xstart,ystart,xend,yend,2);
    
    // Now add extra precipitation to mountains, where appropriate.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            if (world.wintermountainraindir(xx,yy)!=0)
                addregionalmountainprecipitation(world,region,x*16,y*16,xx,yy,0);
            
            if (world.summermountainraindir(xx,yy)!=0)
                addregionalmountainprecipitation(world,region,x*16,y*16,xx,yy,1);
        }
    }
    
    // Now remove any salt pans from areas that have too much precipitation.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    removewetsaltpans(region,xstart,ystart,xend,yend);
    
    // Now we do the temperature maps.
    
    // First, maximum temperature.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int i=0; i<RARRAYWIDTH; i++)
    {
        for (int j=0; j<RARRAYHEIGHT; j++)
        {
            destination[i][j]=-5000;
            rotatearray[i][j]=0;
        }
    }
    
    // Note that we remove the elevation element of temperatures (because that is determined by the elevation of the regional map, not of the global map). Also we multiply temperatures by 100 temporarily, to ensure smoother fractals.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
            source[i][j]=tempelevremove(world,world.maxtemp(i,j),i,j)*100;
    }
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            float valuemod=0.01;
            
            makegenerictile(world,x*16,y*16,xx,yy,valuemod,coords,source,destination,5000,-5000,0);
        }
    }
    
    // Now add rotations to that temperature map, to improve the look.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            rotatetileedgesarray(world,region,x*16,y*16,xx,yy,destination,rotatearray,-5000);
        }
    }
    
    // Now add the elevation element back in.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            for (int i=x*16; i<=x*16+15; i++) // Divide by 100 and add the elevation element back in.
            {
                for (int j=y*16; j<=y*16+15; j++)
                {
                    int amount=tempelevadd(region,destination[i][j],i,j)-destination[i][j];
                    
                    region.setextramaxtemp(i,j,destination[i][j]+amount*100);
                    
                    region.setmaxtemp(i,j,tempelevadd(region,destination[i][j]/100,i,j));
                }
            }
        }
    }
    
    // Now, minimum temperature.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int i=0; i<RARRAYWIDTH; i++)
    {
        for (int j=0; j<RARRAYHEIGHT; j++)
        {
            destination[i][j]=-5000;
            rotatearray[i][j]=0;
        }
    }
    
    // Note that we remove the elevation element of temperatures (because that is determined by the elevation of the regional map, not of the global map). Also we multiply temperatures by 100 temporarily, to ensure smoother fractals.
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
            source[i][j]=tempelevremove(world,world.mintemp(i,j),i,j)*100;
    }
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            float valuemod=0.01;
            
            makegenerictile(world,x*16,y*16,xx,yy,valuemod,coords,source,destination,5000,-5000,0);
            
            if (yy==height) // If this is the very bottom of the global map, ensure there's no weirdness.
            {
                int maxval=world.mintemp(xx,yy);
                
                for (int i=x*16; i<=x*16+16; i++)
                {
                    for (int j=y*16; j<=y*16+16; j++)
                    {
                        if (destination[i][j]>maxval)
                            destination[i][j]=maxval;
                    }
                }
            }
        }
    }
    
    // Now add rotations to that temperature map, to improve the look.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            rotatetileedgesarray(world,region,x*16,y*16,xx,yy,destination,rotatearray,-5000);
        }
    }
    
    // Now add the elevation element back in.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            for (int i=x*16; i<=x*16+15; i++) // Divide by 100 and add the elevation element back in.
            {
                for (int j=y*16; j<=y*16+15; j++)
                {
                    int amount=tempelevadd(region,destination[i][j],i,j)-destination[i][j];
                    
                    region.setextramintemp(i,j,destination[i][j]+amount*100);
                    
                    region.setmintemp(i,j,tempelevadd(region,destination[i][j]/100,i,j));
                }
            }
        }
    }
    
    // Now sort out any problems with the temperatures at the very bottom of the map.
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ybottom-2; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            if (yy>height-10)
            {
                int maxmaxtemp=world.maxtemp(xx,yy);
                int maxmintemp=world.mintemp(xx,yy);
                
                for (int i=x*16; i<=x*16+15; i++)
                {
                    for (int j=y*16; j<=y*16+15; j++)
                    {
                        if (region.maxtemp(i,j)>maxmaxtemp)
                            region.setmaxtemp(i,j,maxmaxtemp);
                        
                        if (region.mintemp(i,j)>maxmintemp)
                            region.setmintemp(i,j,maxmintemp);
                    }
                }
            }
        }
    }
    
    // Now the sea ice map.
    // Here again we have to fiddle with the values, as it only has three values and that's not much use for the diamond-square function to get to work on.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    int icediv=20;
    
    for (int i=0; i<RARRAYWIDTH; i++)
    {
        for (int j=0; j<RARRAYHEIGHT; j++)
            destination[i][j]=-5000;
    }
    
    for (int i=0; i<=width; i++)
    {
        for (int j=0; j<=height; j++)
        {
            switch (world.seaice(i,j))
            {
                case 0:
                    source[i][j]=icediv/2;
                    break;
                    
                case 1:
                    source[i][j]=icediv/2+icediv;
                    break;
                    
                case 2:
                    source[i][j]=icediv/2+icediv*2;
                    break;
            }
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
            
            int surroundingice=getsurroundingice(world,xx,yy);
            
            if (surroundingice!=-1) // If this isn't a mixed tile
            {
                for (int i=x*16; i<=x*16+15; i++)
                {
                    for (int j=y*16; j<=y*16+15; j++)
                        region.setseaice(i,j,world.seaice(xx,yy));
                }
            }
            else
            {
                float valuemod=0.015;
                
                makegenerictile(world,x*16,y*16,xx,yy,valuemod,coords,source,destination,icediv*3,0,0);
                
                for (int i=x*16; i<=x*16+15; i++) // Translate the values back to sea ice values.
                {
                    for (int j=y*16; j<=y*16+15; j++)
                    {
                        if (destination[i][j]<=icediv)
                        {
                            bool openseaok=1;
                            
                            for (int k=xx-1; k<=xx+1; k++)
                            {
                                int kk=k;
                                
                                if (kk<0 || kk>width)
                                    kk=wrap(kk,width);
                                
                                for (int l=yy-1; l<=yy+1; l++)
                                {
                                    if (l>=0 && l<=height)
                                    {
                                        if (world.seaice(kk,l)==2)
                                        {
                                            openseaok=0;
                                            k=xx+1;
                                            l=yy+1;
                                        }
                                    }
                                }
                            }
                            
                            if (openseaok==1)
                                region.setseaice(i,j,0);
                            else
                                region.setseaice(i,j,1);
                            
                        }
                        else
                        {
                            if (destination[i][j]>icediv && destination[i][j]<=icediv*2)
                                region.setseaice(i,j,1);
                            else
                            {
                                bool permiceok=1;
                                
                                for (int k=xx-1; k<=xx+1; k++)
                                {
                                    int kk=k;
                                    
                                    if (kk<0 || kk>width)
                                        kk=wrap(kk,width);
                                    
                                    for (int l=yy-1; l<=yy+1; l++)
                                    {
                                        if (l>=0 && l<=height)
                                        {
                                            if (world.seaice(kk,l)==0)
                                            {
                                                permiceok=0;
                                                k=xx+1;
                                                l=yy+1;
                                            }
                                        }
                                    }
                                }
                                
                                if (permiceok==1)
                                    region.setseaice(i,j,2);
                                else
                                    region.setseaice(i,j,1);
                                
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Now remove glitches in the sea ice map.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    removeseaiceglitches(region);
    
    // Now we do the climate map.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int i=xstart; i<=xend; i++)
    {
        for (int j=ystart; j<=yend; j++)
        {
            string climate=getclimate(region,i,j);
            region.setclimate(i,j,climate);
        }
    }
    
    // Now create small salt pans.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    for (int x=xleft; x<=xright; x++)
    {
        int xx=leftx+x;
        
        if (xx<0 || xx>width)
            xx=wrap(xx,width);
        
        for (int y=ytop; y<=ybottom; y++) // xx and yy are the coordinates of the current pixel being expanded.
        {
            int yy=lefty+y;
            
            makesmallsaltpans(world,region,x*16,y*16,xx,yy,safesaltlakes,smalllake);
        }
    }
    
    // Now just check that all the lake beds make sense.
    
    regionprogress.set_value(regionprogress.value()+progressstep);
    screen.redraw();
    screen.draw_all();
    
    checklakebeds(region,xstart,ystart,xend,yend);
}
