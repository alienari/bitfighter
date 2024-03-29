//------------------------------------------------------------------------------
// Copyright Chris Eykamp
// See LICENSE.txt for full copyright information
//------------------------------------------------------------------------------

#include "loadoutHelper.h"

#include "UIGame.h"
#include "UIManager.h"
#include "UIInstructions.h"
#include "Colors.h"
#include "LoadoutTracker.h"
#include "gameConnection.h"
#include "ClientGame.h"


namespace Zap
{

// For clarity and consistency
#define UNSEL_COLOR &Colors::overlayMenuUnselectedItemColor
#define HELP_COLOR  &Colors::overlayMenuHelpColor

static const OverlayMenuItem loadoutModuleMenuItems[] = {
   { KEY_1, BUTTON_1, true, ModuleBoost,    "Turbo Boost",           UNSEL_COLOR, "",               NULL,       NULL },
   { KEY_2, BUTTON_2, true, ModuleShield,   "Shield Generator",      UNSEL_COLOR, "",               NULL,       NULL },
   { KEY_3, BUTTON_3, true, ModuleRepair,   "Repair Module",         UNSEL_COLOR, "",               NULL,       NULL },
   { KEY_4, BUTTON_4, true, ModuleSensor,   "Enhanced Sensor",       UNSEL_COLOR, "",               NULL,       NULL },
   { KEY_5, BUTTON_5, true, ModuleCloak,    "Cloak Field Modulator", UNSEL_COLOR, "",               NULL,       NULL },
   { KEY_6, BUTTON_6, true, ModuleArmor,    "Armor",                 UNSEL_COLOR, "(adds mass)",    HELP_COLOR, NULL },
   { KEY_7, BUTTON_7, true, ModuleEngineer, "Engineer",              UNSEL_COLOR, "",               NULL,       NULL },
};

static const S32 moduleEngineerIndex = 6;

static const OverlayMenuItem loadoutWeaponMenuItems[] = {
   { KEY_1, BUTTON_1, true, WeaponPhaser, "Phaser",     UNSEL_COLOR, "", NULL, NULL },
   { KEY_2, BUTTON_2, true, WeaponBounce, "Bouncer",    UNSEL_COLOR, "", NULL, NULL },
   { KEY_3, BUTTON_3, true, WeaponTriple, "Triple",     UNSEL_COLOR, "", NULL, NULL },
   { KEY_4, BUTTON_4, true, WeaponBurst,  "Burster",    UNSEL_COLOR, "", NULL, NULL },
   { KEY_5, BUTTON_5, true, WeaponMine,   "Mine Layer", UNSEL_COLOR, "", NULL, NULL },
   { KEY_6, BUTTON_6, true, WeaponSeeker, "Seeker",     UNSEL_COLOR, "", NULL, NULL },
};      


static string preset1, preset2, preset3;     // Static so that the c_str() pointers we use will stick around


////////////////////////////////////////
////////////////////////////////////////

// Constructor
LoadoutHelper::LoadoutHelper()
{
   S32 modItemWidth  =  getMaxItemWidth(loadoutModuleMenuItems, ARRAYSIZE(loadoutModuleMenuItems));
   S32 weapItemWidth =  getMaxItemWidth(loadoutWeaponMenuItems, ARRAYSIZE(loadoutWeaponMenuItems));
   mLoadoutItemsDisplayWidth = max(modItemWidth, weapItemWidth);

   mPresetItemsDisplayWidth = 0;    // Will be set in onActivated()
   mPresetButtonsWidth = 0;

   reset();

   mTitle = "";
   mLegend = NULL;
}


// Destructor
LoadoutHelper::~LoadoutHelper()
{
   // Do nothing
}


void LoadoutHelper::reset()
{
   mCurrentIndex = 0;
   setShowingPresets(false);
}


HelperMenu::HelperMenuType LoadoutHelper::getType() { return LoadoutHelperType; }


// Gets called at the beginning of every game; available options may change based on level
void LoadoutHelper::pregameSetup(bool engineerEnabled)
{
   mEngineerEnabled = engineerEnabled;
}


void LoadoutHelper::rebuildPresetItems()
{
   preset1 = "Preset 1 - " + getGame()->getSettings()->getLoadoutPreset(0).toString(false);
   preset2 = "Preset 2 - " + getGame()->getSettings()->getLoadoutPreset(1).toString(false);
   preset3 = "Preset 3 - " + getGame()->getSettings()->getLoadoutPreset(2).toString(false);

#define GET_COLOR(p) getGame()->getSettings()->getLoadoutPreset(p).isValid() ? UNSEL_COLOR : &Colors::DisabledGray

   // Need to rebuild these on every activation in case the user has changed their presets
   const OverlayMenuItem presetItems[] = {
      { KEY_1, BUTTON_1, true, 0, preset1.c_str(), GET_COLOR(0), "", NULL, NULL },
      { KEY_2, BUTTON_2, true, 1, preset2.c_str(), GET_COLOR(1), "", NULL, NULL },
      { KEY_3, BUTTON_3, true, 2, preset3.c_str(), GET_COLOR(2), "", NULL, NULL },
   };
   TNLAssert(ARRAYSIZE(presetItems) == GameSettings::LoadoutPresetCount, "presetItems[] has the wrong number of elements!");

   mPresetItems = Vector<OverlayMenuItem>(presetItems, ARRAYSIZE(presetItems));
   mPresetItemsDisplayWidth = getMaxItemWidth(presetItems, ARRAYSIZE(presetItems));

   mPresetButtonsWidth = getButtonWidth(presetItems, ARRAYSIZE(presetItems));
}

#undef UNSEL_COLOR
#undef HELP_COLOR
#undef GET_COLOR


void LoadoutHelper::onActivated()
{
   // Need to do this here because user may have toggled joystick and keyboard modes
   S32 modButtWidth  = getButtonWidth(loadoutModuleMenuItems, ARRAYSIZE(loadoutModuleMenuItems));
   S32 weapButtWidth = getButtonWidth(loadoutWeaponMenuItems, ARRAYSIZE(loadoutWeaponMenuItems));

   mLoadoutButtonsWidth = max(modButtWidth, weapButtWidth);

   // Before we activate the helper, we need to tell it what its width will be
   setExpectedWidth(getTotalDisplayWidth(mLoadoutButtonsWidth, mLoadoutItemsDisplayWidth));
   Parent::onActivated();

   // Player has proven they know how to change loadouts, so no need to show a help message on how to do it
   getGame()->getUIManager()->removeInlineHelpItem(ChangeConfigItem, true);

   // Define these here because they get modified as we go through the menus; we want a clean set to start with
   mModuleMenuItems = Vector<OverlayMenuItem>(loadoutModuleMenuItems, ARRAYSIZE(loadoutModuleMenuItems));
   mWeaponMenuItems = Vector<OverlayMenuItem>(loadoutWeaponMenuItems, ARRAYSIZE(loadoutWeaponMenuItems));

   reset();

   mModuleMenuItems[moduleEngineerIndex].showOnMenu = mEngineerEnabled;    // Can't delete this or other arrays will become unaligned

   mLoadoutChanged = false;
   setShowingPresets(false);     // Start in regular mode -- press activation key again to enter preset mode

   // Rebuild the text of the preset items we'll show -- user may have defined new presets since last visit
   rebuildPresetItems();         
}


// Returns true if we are currently displaying modules, false if weapons
bool LoadoutHelper::showingModules() const
{
   return mCurrentIndex < ShipModuleCount;
}


void LoadoutHelper::render() const
{
   drawItemMenu(mLoadoutButtonsWidth, mLoadoutItemsDisplayWidth);
}


// Return true if key did something, false if key had no effect
// Runs on client
bool LoadoutHelper::processInputCode(InputCode inputCode)
{
   if(Parent::processInputCode(inputCode))    // Check for cancel keys
      return true;

   Vector<OverlayMenuItem> &menuItems = mShowingPresets ? mPresetItems : 
                                                   (mCurrentIndex < ShipModuleCount) ? mModuleMenuItems : 
                                                                                       mWeaponMenuItems;
   S32 index;

   for(index = 0; index < menuItems.size(); index++)
      if(inputCode == menuItems[index].key || inputCode == menuItems[index].button)
         break;

   // Does key correspond to a menu item being displayed?  No?
   if(index == menuItems.size() || !menuItems[index].showOnMenu)     
   {
      // Since this isn't a menu option, maybe it is a request to avance loadout menu to the presets display.
      // Put this inside here so if joystick users remap the loadout button, maybe they can use the preset menu.
      if(inputCode == getActivationKey())       // Pressing loadout button again advances to preset mode
      {
         TNLAssert(!mShowingPresets, "Should only get here when mShowingPresets is false -- when it is true, menu should close!");
         activateTransitionFromLoadoutMenuToPresetMenu();

         return true;
      }

      return false;     // Key not handled
   }

   if(mShowingPresets)
   {
      LoadoutTracker loadout = getGame()->getSettings()->getLoadoutPreset(index);

      // Ignore presets that have not been defined (these are displayed but cannot be chosen)
      if(!loadout.isValid())
         return true;

      getGame()->requestLoadoutPreset(index);
      exitHelper();
      return true;
   }

   /////
   // Showing normal loadout menu

   // Make sure user doesn't select the same loadout item twice
   bool alreadyUsed = false;
	
   if(mCurrentIndex < ShipModuleCount)    // We're working with modules
   {                                      // (note... braces required here!)
      for(S32 i = 0; i < mCurrentIndex && !alreadyUsed; i++)
         if(mModule[i] == index)
            alreadyUsed = true;
   }
   else                                   // We're working with weapons
      for(S32 i = ShipModuleCount; i < mCurrentIndex && !alreadyUsed; i++)
         if(mWeapon[i - ShipModuleCount] == index)
            alreadyUsed = true;

   if(!alreadyUsed)
   {
      menuItems[index].itemColor           = &Colors::overlayMenuSelectedItemColor;
      menuItems[index].helpColor           = &Colors::overlayMenuSelectedItemColor;
      menuItems[index].buttonOverrideColor = &Colors::overlayMenuSelectedItemColor;

      mModule[mCurrentIndex] = ShipModule(index);
      mCurrentIndex++;

      setShowingPresets(false);     // Triggers update of menu title and such

      bool showingMods = showingModules();
      
      // Check if we need to switch over to weapons
      if(mCurrentIndex == ShipModuleCount)
         resetScrollTimer();
   }

   if(mCurrentIndex == ShipModuleCount + ShipWeaponCount)   // All loadout options selected, process complete
   {
      LoadoutTracker loadout;

      for(S32 i = 0; i < ShipModuleCount; i++)
         loadout.setModule(i, ShipModule(loadoutModuleMenuItems[mModule[i]].itemIndex));

      for(S32 i = 0; i < ShipWeaponCount; i++)
         loadout.setWeapon(i, WeaponType(loadoutWeaponMenuItems[mWeapon[i]].itemIndex));

      Ship *ship = getGame()->getLocalPlayerShip();         // Can be NULL if game has ended while we're here
      mLoadoutChanged = !ship || !getGame()->getLocalPlayerShip()->isLoadoutSameAsCurrent(loadout);

      GameConnection *conn = getGame()->getConnectionToServer();

      if(conn)
      {
         getGame()->displayShipDesignChangedMessage(loadout, "Selected loadout: ", 
                                                               "Modifications canceled -- new ship design same as the current");

         // Request loadout even if it was the same -- if I have loadout A, with on-deck loadout B, and I enter a new loadout
         // that matches A, it would be better to have loadout remain unchanged if I entered a loadout zone.
         // Tell server loadout has changed.  Server will activate it when we enter a loadout zone.
         conn->c2sRequestLoadout(loadout.toU8Vector());     
      }
      exitHelper();     
   }

   return true;
}


// Handle phase transitions when menu changes from presets to modules to weapons
void LoadoutHelper::setShowingPresets(bool showingPresets)
{
   mShowingPresets = showingPresets;

   if(mShowingPresets)
   {
      mTitle = "Choose loadout preset:";
      mCurrentRenderItems = mPresetItems.address();
      mCurrentRenderCount = mPresetItems.size();
   }

   else if(showingModules())
   {
      static char title[32];  // static, so mTitle pointer stays valid
      dSprintf(title, sizeof(title), "Pick %d modules:", ShipModuleCount);
      mTitle = title;

      mPrevRenderItems = NULL; //(showingMods ? mModuleMenuItems : mWeaponMenuItems).address();
      mPrevRenderCount = 0; //(showingMods ? mModuleMenuItems : mWeaponMenuItems).size();

      mCurrentRenderItems = mModuleMenuItems.address();
      mCurrentRenderCount = mModuleMenuItems.size();
   }

   else     
   {
      static char title[32];
      dSprintf(title, sizeof(title), "Pick %d weapons:", ShipWeaponCount);
      mTitle = title;

      mPrevRenderItems = mModuleMenuItems.address();
      mPrevRenderCount = mModuleMenuItems.size();

      mCurrentRenderItems = mWeaponMenuItems.address();
      mCurrentRenderCount = mWeaponMenuItems.size();
   }
}


void LoadoutHelper::activateTransitionFromLoadoutMenuToPresetMenu()
{
   setShowingPresets(true);

   // The menu will be getting larger to accomodate the presets, which are wider than the standard 
   // loadout menu.  Here we'll calculate the width of the loadout menu and the preset menu.
   S32 currDisplayWidth = getCurrentDisplayWidth(mLoadoutButtonsWidth, mLoadoutItemsDisplayWidth);
   S32 futureDisplayWidth = getTotalDisplayWidth(mPresetButtonsWidth, mPresetItemsDisplayWidth);

   resetScrollTimer();              // Activate the transition between old items and new ones
   SlideOutWidget::onActivated();   // Activiate the slide animation to extend the menu a bit

   // Now we're transitioning between the already visible loadout menu and the preset menu; we need to tell the
   // render function how much of the display is already visible
   setStartingOffset(currDisplayWidth);
   setExpectedWidth_MidTransition(futureDisplayWidth);
}


InputCode LoadoutHelper::getActivationKey() 
{ 
   GameSettings *settings = getGame()->getSettings();
   return settings->getInputCodeManager()->getBinding(BINDING_LOADOUT);
}


// When we are in preset mode, the next press of the activation key will close the menu
bool LoadoutHelper::getActivationKeyClosesHelper()
{
   return mShowingPresets;
}


const char *LoadoutHelper::getCancelMessage() const
{
   return "Modifications canceled -- ship design unchanged.";
}


void LoadoutHelper::activateHelp(UIManager *uiManager)
{
   // Go to module help page
   if(mCurrentIndex < ShipModuleCount)
      uiManager->getUI<InstructionsUserInterface>()->activatePage(InstructionsUserInterface::InstructionModules);
   // Go to weapons help page
   else
      uiManager->getUI<InstructionsUserInterface>()->activatePage(InstructionsUserInterface::InstructionWeaponProjectiles);
}


void LoadoutHelper::onWidgetClosed()
{
   Parent::onWidgetClosed();

   // We only want to display this help item if the loadout actually changed
   if(mLoadoutChanged)
   {
      bool hasLoadout = getGame()->levelHasLoadoutZone();
      getGame()->getUIManager()->addInlineHelpItem(hasLoadout ?  LoadoutChangedZoneItem : LoadoutChangedNoZoneItem);
   }
}


// Static method, for testing
InputCode LoadoutHelper::getInputCodeForWeaponOption(WeaponType index, bool keyBut)
{
   InputCode code = Parent::getInputCodeForOption(&loadoutWeaponMenuItems[0], ARRAYSIZE(loadoutWeaponMenuItems), index, keyBut);

   TNLAssert(code != KEY_UNKNOWN, "InputCode not found!");

   return code;
}


// Static method, for testing
InputCode LoadoutHelper::getInputCodeForModuleOption(ShipModule index, bool keyBut)
{
   InputCode code = Parent::getInputCodeForOption(&loadoutModuleMenuItems[0], ARRAYSIZE(loadoutModuleMenuItems), index, keyBut);

   TNLAssert(code != KEY_UNKNOWN, "InputCode not found!");

   return code;
}


};

