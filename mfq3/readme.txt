Military Forces Q3
==================


History:
========

+++v0.72+++ in progress...
-----------
- pressing throttle_decrease toggles waypoint display between "show all waypoints" and
  "show waypoints of selected vehicle"
- when in waypoint mode only the waypoints of the currently selected vehicle are shown
- make controls better: LEFT mouse button now drags, MIDDLE one (or SPACE) selects, SHIFT    while dragging rotates, CTRL while dragging lifts
- further increase of selection distance in IGME to 3000
- bugfixes

+++v0.71+++
-----------
- pressing the landing gear button toggles WaypointMode on/off (you need to have
  one single vehicle for it selected), when in waypoint mode vehicles cannot be 
  manipulated
- when in waypoint mode you can spawn new waypoints with the throttle_increase button
- pressing CTRL and throttle_increase inserts a waypaint after the selected waypoint
- waypoints can be selected the same way vehicles can
- waypoints can now be moved the same way vehicles are moved
- Backspace deletes selected waypoint(s)
- pressing Backspace while in menus/console no more deletes selection
- allow spawning boats and groundvehicles in IGME
- minor cleanup and fixes

+++v0.70+++ 
-----------
- in order to make IGME better I rewrote it to be client side only
- in order to allow selection of client side only stuff, I had to provide new 
  ray interesect function
- selecting objects is limited to within 2000 units of yourself
- holding the attack button and moving the mouse drags selected vehicles around
- holding the CTRL button while dragging moves selection vertically or rotates it
- hitting Backspace deletes the selection
- allow selection of multiple vehicles
- minor changes

+++v0.69+++
-----------
- added cvar g_spectSpeed to control spectator speed
- attack2 button selects/deselects vehicle
- vehicle selection menu spawns entities in mission editor now
- started preparational work for missioneditor (gametype 3)
- improve the code that displays numbers on the HUD
- make use of Abbo's "numberline" for displaying numbers
- update a couple of pk3s with newer artwork/models/etc
- Mark: add credits page
- minor fixes

+++v0.68+++
-----------
- added first cut of flares
- added first cut of missile-distraction code (flares), needs testing
- prevent camping on recharge spots by disabling shooting while on them (please test
  this on networks!)
- new ww2 loading pic from Abbo
- got updated F-15 for vwep compliance
- got updated F-18 for vwep compliance
- got updated F-5 for vwep compliance
- fixed throttle blurriness
- more minor changes

+++v0.67+++
-----------
- test: F-16 with various loadouts
- F-14, F-16 and Jas39 updated for vwep
- Mark: added gameset voting
- added vwep for ECM and droptanks
- added new weapons (no special code yet though): HARM, GBU-15, Durandal
- got new vwep models from Schez
- added a 30mm gun for use on the A-10
- make GVs survive longer underwater
- fix some amphib and boat bugs
- and other minor fixes

+++v0.66+++
-----------
- make FFARs work properly with new vwep code and even when there are more than 2 launchers
- allow draw char function to also draw numbers
- certain weapons now limited to categories so not every vehicle can carry for 
  example ironbombs
- weapon loadout now limited by pylons, ie if there are not enough pylons the
  loadout specified in the datasheets is adjusted accordingly (ie old vehicles 
  will be unarmed until fitted with properly named pylon tags)
- final (hopefully) naming convention for vwep: tag_P#&$$$$@ with
  # = number in hex of the station (increasing from outside towards the body)
  & = group of the weapon (for three mount pylons, set to 0 on single pylons)
  $$$$ = flags of what to carry (see dev forum)
  @ = L or R for the side it is on
  Note that # starts with 0 on both wings, so there will be two tag_P0..
- added small FFAR pod
- added medium bomb ('Mk83')
- Mark: event/prediction code updated
- Mark: minor death code cleanup
- revised and improved vwep system
- Mark: more work on shadows
- Mark: added proper spectator support to scoreboard
- Mark: added team counts to scoreboard
- Mark: fix bug with MFD zoom display not working on non 640x480 resolutions
- Mark: made gameset artwork appear if no map loading screen available
- more bugfixes

+++v0.65+++
-----------
- Mark: masked out all but MFQ3 servers in the UI server browser
- Mark: gameset now also shown in the 'Type' column (MDRN, WW2, WW1)
- bounding boxes for vehicles are now read out from the md3 file
- Mark: changed to fixed point zoom on MFD camera (x1...x8)
- gearheight is read out from md3 file now
- number of gear and bay frames now read out from md3 files
- vwep support for F-16
- Mark bugfix: smoke not stopped from being drawn anymore when client is close
- Mark bugfix: stopped smoke drawing when client paused
- Mark: reduced density of smoke
- made wingtip AAMs not drop before shooting off
- Mark: some fixes to smoketrails and shadows
- added heavy bomb ('Mk84'), and reduced light's damage ('Mk82')
- added Hawkboy's A-10
- allow proper reloading of vwep (trigger_recharge)
- no more possible to shoot missiles while landed
- made missiles drop from the pylon before their rocket motors go off
- got a better scaled B2 from Schez, with working gear
- use updated gear model for Spitfire, Mustang, Gripen, F-16, F-5, F-15, F-14, B-2
- fix some minor vwep problems

+++v0.64+++
-----------
- some more work on turret code (for boat turrets)
- some more work on boat code
- made weapons launch from their vwep positions
- first cut vwep (only supported by Harrier model atm)
- code to put weapon loadouts on pylons automatically (needed for vwep)
- Mark: more work on custom chat console
- allow reading out md3 files (esp. tags) from all three modules, which will be needed
  for vwep
- if no primary MG is available weapons can also be fired by other fire button
- fixing some problems in the weaponcode
- fixed throttle problem on HUD (for boats)
- various minor fixes from Mark and me

+++v0.63+++ in progress
-----------
- more work on turret code (for boat-turrets)
- Mark: allow balanced teams (g_teamForceBalance)
- Mark: fixes to team and vehicle selection
- Mark: more work on HUD
- boats shouldn't be able to drive on land anymore
- more work on boat driving-code 
- Mark: more shadow work
- fix a bug that could cause the game to hang on spawning
- Mark: fixing vehicle selection
- Mark: scoreboard and crosshair fixes
- minor fixes

+++v0.62+++ 
-----------
- added Matt's Pbr31Mk2
- Mark: more work on custom chat console
- Mark: more work on shadow code
- added first cut boat code
- Mark: only vehicles with spawnpoints in the current map are selectable
- Mark: fixed shadow bug
- Mark: fixed crosshair positioning bug
- stage 1 code preparation for helos, lqms and boats
- added weaponbay code, they autoclose after the primary weapon hasn't been used
  for 5 seconds
- added weaponbay button: +button3
- minor changes

+++v0.61+++
-----------
- added first-cut Harrier model from Matt (no VTOL support yet!)
- got updated B17g from Matt
- internal cleanup (related to id/cat/class etc)
- Mark: experimental custom chat console - use cmessagemode/2/3/4 (bind a key to use)
- Mark: stopped drawing parts of the player's vehicle when rendering the MFD worlds
- Mark: added the new 2D crosshair system
- Mark: gameset in action is now shown on the scoreboard
- Mark: moved pre-cache of vehicle icons into the loading process to prevent 
  glitching in-game
- Mark: set example default 3rd person cameras for each vehicle
  (should have fixed the camera range/height bug)
- Mark: added min/max for both cam_dist and cam_height (per vehicle)
- minor fixes

+++v0.60+++
-----------
- Mark: fixes to allow right_justified owner drawn text (from UI scripts)
- Mark: added custom console printing (for chat text and game messages)
- Mark: new scoreboard message support (e.g. timelimit hit)
- Mark: hud/scoreboard bug fixes
- fixed a cockpit bug
- added cmds "toggle_mfd1" and "toggle_mfd2" to allow easily toggling the 
  MFDs on/off
- updated BRDM2 (for turning wheels)
- added new vehicle: Panzer III
- added a first cut of turning wheels
- make use of Abbo's new shadows
- added new vehicle: Humvee with 50 cal MG
- minor changes

+++v0.59+++ 
-----------
- fixed a crash bug that appeared when restarting the map
- added proper multi-engine support
- added a new plane: the B-17g
- added a new plane: the F-22
- HUD now shows different throttle pics for vehicles with reverse and afterburner
  and if they have more than one engine
- Mark: make turning more realistic for wheeled vehicles (e.g. BRDM2)
- Mark: improved explosions (wider smoke on buildings explosions)
- Mark: added fx quality selector - cg_fxQuality
- Mark: added new custom console text drawing
- Mark: added custom console text drawing style selector - cg_consoleTextStyle
- minor fixes

+++v0.58+++
-----------
- Mark: add more sfx for building explosions
- Mark: fixed "deaths" on the scoreboard
- Mark: added different size explosions and BIG explosions for buildings
- Mark: more work on shadow code
- added new command "+bombcam" to be able to see the spot where an iron-bomb
  dropped right now would approximately land, this is also a new mode in the
  list of cuyclable cameramodes, works up to a height of 20000 units
- removed the black color option from the HUD/MFD color palette
- it is now possible to individually set up the number of gear frames and 
  the time it takes to operate the gear
- more fixes on lock/launch code to make it work over the net
- stallwarning no more displayed when on ground
- made swingwings work properly over the net as well
- minor fixes

+++v0.57+++
-----------
- added lock and launch indicator to HUD
- added a random factor to the delivered damage of a weapon to reflect hits that
  might not be perfect, or on spots that are better protected
- added stallwarning to the HUD
- speed now colored on MFD page "flightdata", relativ to stallspeed
- got updated Fw190 model from Matt
- added new cvars to allow configuring the MFD cycles, ie you can say the left MFD
  only cycles through these pages and the right one through those. The cvars are
  starting with m1cp_page# and m2cp_page# and the min number is 0 and the max number
  is 5 (so for example to enable page 3 you type "m1cp_page3 1" or to disable page 4
  on MFD2 you type "m2cp_page4 0") - this will later be configurable via the 
  options menu as well
- added new cmds "+backcam" and "+downcam" to be able to bind keys so that you
  look back/down while holding down that key
- added two new cmds "cycle_camera_mode" (goes through all available camera modes) and 
  "set_camera_mode" (allows you to explicitely set a camera mode, 0 = targetcam (default),
  1 = backwards cam, 2 = downwards cam)
- added two new camera modes: 'back' (looking at your 6 o'clock position) and
  'down' (looking straight down, might be helpful on bombing)
- first cut shadow support from Mark 
- minor fixes


+++v0.56+++
-----------
- now remembering RADAR settings between deaths
- MFD modes are now saved between games
- added Matt's Fw190a8 (not yet set it's data properly though)
- allow properly right and left aligned numbers
- allow right aligned strings as well
- use new non-monospaced alphabet from Abbo
- implemented animations for landing gears
- got some new gear models for existing planes (which can use the new gear animation 
  code)
- got Matt's Bf109g (not yet set it's data properly though)
- minor changes

+++v0.55+++
-----------
- more UI fixes from Mark
- new HUD: added flightinfo page to MFDs, to have redundant info (in case HUD is 
  damaged)
- new HUD: for completeness' sake I added a cmd "cycle_mfd_color", to set the color of
  the MFDs as well
- new HUD: added cmds to explicitely switch to a certain MFD page, cmds are "mfd1_page"
  and "mfd2_page" and modenumbers start at 0 (example: "mfd1_page 4" switches
  to the camera page)
- added support for dual-prop planes (quick solution, will need proper
  implementation later)
- guided missiles now also explode in proximity when it is unlikely that 
  they can hit
- minor fixes

+++v0.54+++
-----------
- new HUD: tracking camera now shows reticle and range
- switching weapons now only breaks lock, but keeps tracking the target,
  whereas changing radarmode stops tracking as well
- new HUD: heading caret now shows direction to targets that are tracked
- tracking vehicles with radar no more depends on active weapon but only 
  on vehicle radar system
- vehicles (not all, only those for which it is set) can now also track targets
  behind them
- added a camera page to the MFDs (hopefully not too screwy) - shows view from the
  plane, when it is locked it shows the target
- cg_fov and cg_zoomfov now work for the MFD camera screen only
- disabled setting of fov and zoom fov for the main view
- minor fixes

+++v0.53+++
-----------
- new HUD: added GPS coords to HUD (can still be toggled by old GPS command)
- new HUD: added weaponinfo to HUD and cvar "hud_weapons" to turn it on/off
- new HUD: added Inventory page to MFDs
- new HUD: color can now be set by user, cmd to cycle through colors is "cycle_hud_color",
- new HUD color is saved to a cvar, so it doesn't have to be set every time
- new sounds from Mark
- more UI enhancements from Mark
- using different weapons models now (also done by Mark)
- and lots of other changes by Mark to make it all look and feel better
- new HUD: added Status page to MFDs
- new HUD: displaying radarmode on MFD using new "font"
- new HUD: displayig flare info on RWR screen
- minor changes

+++v0.52+++
-----------
- fixed a problem when displaying '%' on the HUD
- added cvar cg_oldHUD, if set to 1 it displays the old instead of the new HUD
- new HUD: added RWR info
- new HUD: added HUD throttle info (and cvar hud_throttle)
- new HUD: added HUD health info (and cvar hud_health)
- new HUD: added center bottom section and cvar hud_center to turn it on/off
- more UI code from Mark
- new HUD: added altitude tape and cvar hud_altitude to turn it on/off
- minor fixes

+++v0.51+++
-----------
- added cmds "mfd1_mode" and "mfd2_mode" to cycle through MFD modes
- new HUD: added MFD2 and cvar hud_mfd2 to turn it on/off
- new HUD: added MFD and cvar hud_mfd to turn it on/off
- new HUD: implemented speed tape and cvar hud_speed to turn it on/off
  NOTE: red caret indicates stall speed, green caret indicates gearspeed
- new HUD: implemented heading tape and cvar hud_heading to turn it on/off
  NOTE: caret indicates heading towards waypoint (only pointing north atm though)
- fixed guided weapons bug allowing to lock with MG after spawning
- fixed F-14 swingwing bug
- minor fixes

+++v0.50+++
-----------
- fix up new ui code and make it compile to qvms
- added cmd "tesetGVCmd" to test certain stuff on a test GV (not yet used though)
- added cmd "testPlaneCmd" to test certain stuff on a test plane, current options are:
  "cockpit" - toggle cockpit opened/closed
  "speedbrakes" - toggle speedbrakes
  "gear" - toggle gear
  "controls" - cycles through the availabe control frames
  "throttle" - cycles through the body anims for the exhaust nozzle
  "burnermodel" - cycles through the available burner models
  "burnernum" - switches between 1 and 2 burners
  "vapormodel" - cycles through available vapor models
  "vapor" - cycles between vapor states
  NOTE: The test commands are used for example like this: "\testPlaneCmd cockpit"
- added cmd "testGV" to add a new test model for a ground vehicle
- added cmd "testPlane" to add a new test model for a plane
  NOTE: If you want to add a GV or Plane you just have to specify the modelname
  without the extension and without the directory, for example: "\testPlane f-22"
  This looks for a plane in models/vehicles/planes/f-22/f-22.md3 
- fix smoke bug
- minor changes

+++v0.49+++
-----------
- Mark: wrote TA compliant NEW main menu system
- Mark: wrote TA compliant NEW ingame menu system
- made BRDM2 amphibious
- added support for amphibian vehicles, still a bit buggy
- added basic ground vehicle water-movement code
- minor fixes

+++v0.48+++
-----------
- changed vehicle loadouts using the new weapons (note however
  that these are not the final loadouts because I haven't implemented
  all the weapons yet)
- added Maverick missile
- changed missile data for better gameplay (please test!)
- added two new MGs, the 0.303 and 0.312, both WW1
- added Sopwith Camel
- corrected F-18 data
- corrected F-14 data
- minor changes

+++v0.47+++
-----------
- fixed a bug that retracted the gear on fixed-gear planes
- added Sopwith Camel (at the moment using the Dr.1 model)
- added Fokker Dr.1
- added WW1 gameset
- introduced Bomber class
- using updated F16 canopy
- minor fixes

+++v0.46+++
-----------
- added B-2 to the game
- added updated Spitfire
- added F-18 Hornet to the game
- made swingwings actually work
- added F-14 Tomcat to the game
- fixed a bug that allowed targetting invincible objects
- minor changes

+++v0.45+++
-----------
- each weapon now has its own time that it needs to lock on
- each weapon can have its own individual tracking, locking and 
  follow cones
- each weapon can now have individual crosshairs for no-lock, tracking,
  locking, locking on friendly
- made guided missiles work with func_explosives
- made missile reticles work with func_explosives
- minor fixes

+++v0.44+++
-----------
- equipped F16 with Hellfires as a test (will be removed soon)
- equipped M1 with Stingers as a test (will be removed soon)
- added Hellfire as first test-anti-ground missile
- made missiles work with ground vehicles
- make tracking reticles work with ground vehicles 
- unlock targets on weapon change
- unlock added to controls/setup
- added cmd "unlock" to break lock on current target
- minor bugfixes

+++v0.43+++
-----------
- first cut of missile target tracking code
- weapons can now be set to work against certain categories only and have 
  limited effect on others
- added a first set of anti air missiles: Sidewinder, Stinger, Sparrow,
  AMRAAM, Phoenix, Archer, Atoll, Alamo
- added new weapon types: anti aircraft missile, anti ground missile, anti
  radar missile and guided bombs
- added tracking and locking mechanism for use with guided weapons, in order
  to make it not too easy weapon platform has to use radar 
- Mark: changed loading screen
- Mark: enhanced encyclopedia/vehicle information
- fix a drone bug where it always turned left instead of the shortest distance
- planes can now land on other platform-like entities as well (func_train and
  func_door)
- minor fixes

+++v0.42+++
-----------
- func_explosives have target and targetname, to allow chain reactions
  of explosions
- added a cvar cg_smoke to allow turning on(1) or off(0) the smoke on
  damaged vehicles
- planes can land on func_plat now
- killing misc_vehicles changes score
- added 'set_State' action to scripts with possible values 'landed' and 
  'airborne'
- misc_vehicles now do radius damage no explosion
- drones now facing first waypoint upon spawning
- minor changes

+++v0.41+++
-----------
- added a scriptparser to read in drone scripts (at the moment only parses
  route information for event 'on_Waypoint' and action 'next_Waypoint')
- misc_vehicles which have a targetname now become drones (drones have pilots
  in the vehicles and cannot be used by players, whereas 'normal' misc_vehicles 
  are empty and can be used by players)
- made radar-altimeter work on top of water as well
- misc_vehicle can also spawn random vehicles when 'model' key is set to
  'randomplane' or 'randomground' 
- added an explosion to func_explosive on death
- made radar work with mi`sc_vehicles
- added misc_vehicle entity
- added misc_waypoint
- minor fixes

+++v0.40+++
-----------
- guns now have a spread (individual for each MG)
- temporarily disable new plane collision code until it works properly
- trigger_recharge and trigger_radio should now work properly in teamgames
- score fix for collisions
- radar mode names now "AIR" and "GND"
- adjusted gun elevation on BMP-3
- after all the whining I added the crosshair id again (for now)
- possible fix for vehicle selection bug (hard for me to test though as I hardly
  ever got it on my PC)
- fixed obituary bug (which btw only happened when sv_privateClients was set >0)
- EIB now turned on by default
- minor fixes

+++v0.39+++
-----------
- new explosion (thanks to ... umm sorry dont know your name...css.ts@talknet.de)
- added a 100mm gun for use on the BMP-3
- some adjustments to encyclopedia
- ground radar only shows targets within a 90 degree cone
- vehicles now have air and ground radar modes (radar toggle key now toggles
  through all available modes) and the different modes have different ranges 
  as well
- removed g_tracer cvar and added cg_tracer which defaults to 3 meaning
  that every third bullet is a tracer (for every bullet to be a tracer set to 1)
- memory footprint of MFQ3 should be a bit reduced (_might_ make a minor 
  difference on slow machines)
- clean up other useless stuff
- cleaning up some old code to remove warning messages
- new model BMP-3
- new model T-90
- some changes to (hopefully) balance the game a bit better (esp. in CTF)
- updated gun on M1
- minor fixes

+++v0.38+++
-----------
- CTF flags return to base after 3 mins now (instead of 30 sec) after
  being dropped by a dying player
- in non-team games recharging is now a lot faster
- flags should only spawn in CTF now
- trigger_recharge now has a "team" key as well (can be "red" or "blue")
- trigger_radio now has a "team" key as well (can be "red" or "blue")
- started working on plane and ground vehicle movement code-overhaul
- throttle can't be changed anymore after death
- default minimum damage on func_explosives is now 30
- some data tweaks
- added first-cut code to make plane-collisions -when speed difference is
  low- less dangerous
- only show MFQ3 games in start game/multiplayer menu
- now possible to vote for mf_gameset
- minor fixes

+++v0.37+++
-----------
- WW2 planes now also have pilots
- updated banner for main menu
- new pilot models and ww2 plane adjustments for them
- some optimizations in radar code to be less heavy on CPU
- added a simple credits menu
- minor changes

+++v0.36+++
-----------
- radar altimeter shows "XXXX" when being higher than 2000
- new selection pic for F-5
- desert skin for F-5 (thx Hedhunta) 
- crosshair now depends on selected primary weapon
- new selection icon for BRDM2
- removed Q3 crosshair as it got obsolete now
- crosshair fixes (mindist, triggerbox problems)
- slight changes to vehicledata
- minor fixes

+++v0.35+++
-----------
- changed tag names of M1
- only ground vehicles can pick up flags (CTF)
- updated F-5 skin
- added dual 14.5mm MG
- added BRDM2 vehicle (thx Minkis!)
- more work on tank code
- not using night-afterburner on Gripen any more
- first cut crosshair to help you aiming
- data sheet modifications
- minor changes

+++v0.34+++
-----------
- func_explosives can now be "on teams"
- added a new map key for func_explosives "score" to specify how many
  points func_explosives are worth in team games
- removed sarge icon and maploading
- fixed lockup bug in vehicle selection
- added simple ground vehicle falling
- changed teamnames and vehicles on teams
- tanks shouldn't be floating anymore (they need a bounding box adjustment though)
- fixed radar to show horizontal distance
- increased radar icon size
- some more HUD changes from oq
- in vehicle selection clicking on pic also switches to next vehicle now
- minor model update on F-15
- minor fixes

+++v0.33+++
-----------
- fixed stupid radar bug (not using nomip shaders - argh!)
- further enhanced encyclopedia (pics)
- changed vehicleselection menu a bit
- added F-15 skin (thx Hedhunta!)
- number of flares now shown on right bottom of HUD
- HUD changes (thx oq!) 
- encyclopedia now shows more information on vehicles (like weapons)
- fixed a radar bug (not showing vehicles with radar on behind you)
- encyclopedia now shows all gamesets when in the main menu and the
  current gameset when in game
- removed triangle floating above vehicles in teamplay
- added F-5 skin (thanks Hedhunta!)
- fixed camera-jerky-with-tank-driving-on-runways bug
- fixed a weapon selection bug
- fixed a throttle bug when running out of fuel
- fixed a bug that didnt allow you to select the vehicle you had before
  when changing to spectator and then back to the game
- in teamgames only vehicles from your team can be selected
- spectators can no longer select vehicles
- fixed dual-pilot bug (no more little men in the engines)
- minor fixes


+++v0.32+++
-----------
- added new model F-15 Eagle
- targets are now only shown on the radar if they have their radar turned on
  or if there is a line of sight between the two vehicles and if they are
  in front of you
- radar now shows buildings as well (squares; ground vehicles are circles
  from now on)
- func_explosives can now be on teams as well, by using the key "team"
  (for example: team "red")
- added a new, bigger vapor model for bigger planes (thx Schez!)
- teamplay spawn points should now properly use categories
- changing teams doesn't instantly spawn you any more
- added support for planes with two pilots 
- planes now show appropriate number of muzzle flashes
- increased fuel consumption on afterburners
- fixed a radar bug (drawing outside radar screen)
- added support for dual-engine planes (esp. afterburner)
- added new model F-5 Tiger
- added turn left/right (for tanks) to controls setup again after accidentally
  removing them
- minor changes and fixes

+++v0.31+++
-----------
- added RADAR to display your enemies, squares are ground vehicles and 
  triangles are aircraft, blue is friend, red is enemy (yellow is unidentified,
  which is not yet used), brighter shade of the color means enemy is higher
  than you (1000+ units higher), darker shade means lower (1000+ units lower)
- added new command "radarrange" to toggle between high, medium and low
  radar range (actual range depends on vehicle, this just halfs or quarters
  the maximum range of the selected vehicle)
- added cvar "cg_radarTargets" to specify the maximum number of targets
  that can appear on the radar screen
- some more value tweaking
- cleaned up ingame menu a bit
- split the generic MG into several types with different characteristics
- muzzleflash of M1 now on proper position
- fixed "soul floating away" bug after being killed on a runway
- spawnpoint fix for team games
- GPS altitude now color coded
- minor fixes

+++v0.30+++
-----------
- added support for flares, use "+button10" (can't be launched yet though)
- fixed a ground vehicle brake bug
- added oq's cool new HUD (first cut though, haven't done everything 100%
  properly yet)
- added two new commands: "radar" to toggle radar on/off (not yet
  functional though) and "extinfo" to toggle "extended info board" (EIB)
  on/off (also available in controls setup)
- fixed zoomin/zoomout mixup...oops 
- only maps with the following key-value in the .arena file will be 
  shown in the map selection menu from now on: 
  game	"mfq3"  
- some fixes in several parts of UI
- cleaned up controls menu and added more MFQ3 controls
- cleaned up gameoptions menu
- minor changes

+++v0.29+++
-----------
- fixed a scoreboard problem
- fixed afterburner frames and using new models
- added commands to move external camera up/down: "cameraup" "cameradown"
- added commands to zoom in/out external camera: "zoomin" "zoomout"
- added command "toggleview" to toggle between external and cockpit view
- tweaking of some vehicle values 
- increased damage radius and damage of tank main gun
- increased muzzle velocity of tank main gun
- minor things

+++v0.28+++
-----------
- changed directory/qvm structure - please remove all old files and 
  use new ones only!
- afterburner code no more references shaders, only model
- only vehicles for the current gameset are loaded to reduce memory
  usage (after changing gameset they will be reloaded)
- fixed camera bug after death
- added new model (by LSMR, skin by Hedhunta, final touches by Schez)
- speed on runway restricted
- fixed weapon bug that allowed shooting the weapons at higher speed
- fixed dual rocket explosion bug
- other minor changes

+++v0.27+++
-----------
- MG is now fired with "+attack" and primary weapons with "+button2"
- completely rewrote weapons system
- fixed a bug that caused planes to be unable to move in certain
  situations
- minor fixes

+++v0.26+++
-----------
- another huge code cleanup, removed most of what is not of use for
  MFQ3 (code reduction: cgame -27%, qagame -20%, ui -11%), will still
  need some more polishing though
- code preparations for the all-new weapons code
- fixed a bug in the cameracode that caused some camera-flickering
  (especially noticable in groundvehicles)
- fixed some spectator mode bugs
- tanks now show smaller numbers for fuel remaining
- planes can now also use forward/backward keys to accelerate (they
  also override the current throttle setting)
- fixed a fuelflow bug (cheat) for tanks when using the forward/back
  keys for movement instead of throttle
- changed the way throttle is handled (codewise)
- cleaned up the whole workspace and build settings
- fixed a CTF bug
- minor changes

+++v0.25+++
-----------
- first part of big code-cleanup, and removal of "normal Q3" code,
  so far mainly removal of bots and armor and minor things
  (code reduction: cgame -3%, qagame -41%, ui -1%)
  Testers: Please make sure existing stuff still works as before!
- removed player model from key-setup menu
- fixed a target_radio bug
- fixed a sound bug that appeared when vehicles reversed
- using Jeremy's jetsounds in mfq3_5.pk3, and changed the afterburner
  sound from rocket-sound to a dedicated afterburner sound
- minor fixes

+++v0.24+++
-----------
- speedbrakes now act as parking brakes on ground vehicles (you
  can engage them on speeds lower than 10, but they wont work
  on too steep terrain)
- terrain angle now affects speed (first cut code)
- tank can now also be moved with forward/backward keys (they
  override the actual throttle setting)
- added reverse for ground vehicles (reduce throttle below 0)
- vehicle should now spawn on their proper spawnpoints (ie with the
  correct category)
- vehicle-selection only allows selection of vehicle for which there
  is at least one spawnpoint in the map
- added a "category" key for use in info_player_deathmatch and in
  trigger_radio to specify the categories of vehicles that may use
  them (1 = plane, 2 = ground vehicles, more will follow, to have
  more categories use them just add them together)
- other minor changes

+++v0.23+++ 
-----------
- changed M1 so that it references skins in vehicles/ground
- made weapons work properly with ground vehicles (need to get a proper
  offset though -> will be done later)
- first cut of tank code (Matt) + some optimization (Bjoern)
- M1 tank added
- weapon_maingun now uses shotgun ammo
- added new weapon to key-setup-menu
- added weapon_maingun (ballistic tank shells, weapon 13 for now)
- minor fixes

+++v0.22+++
----------- 
- selecting tanks from encyclopdia or vehicleselection menu now possible
  (although we dont have a tank model yet, just spawns a test model for
  now, which doesn't do anything)
- reworked parts of the code to be more generic (so they can easily
  be used for other vehicle categories as well)
- expanded code framework to allow for ground vehicles (some rather
  big changes, therefore it might have broken existing stuff, please
  test if the old functionality is still ok)
- added command "contact_tower" to trigger targets of trigger_radio
  (as in AirQ2 you have to set a trigger box for the trigger_radio, 
  and link all targets to it, and when a cmd contact_tower is 
  received and a player is inside a trigger_radio box the target is
  activated) -> please test!
- added trigger_radio (for tower controlled things like doors,
  elevators, catapults etc), use the message key for the radio msg
  that will be displayed, and use the wait key to delay the action
  to make it look more realistic
- minor bugfixes

+++v0.21+++
----------- 
- weapons can be changed now much faster
- bombs cannot be dropped on the ground anymore
- added trigger_recharge, which reloads (weapons), refuels and 
  recharges (health) of the vehicles when inside it (and later 
  will also repair damage)
- added GPS to controls setup menu
- you can specify the minimum damage needed to damage func_runways 
  and func_explosives by setting the "count" key (if not specified
  it defaults to 50)
  NOTE: Later only anti-runway bombs will be able to damage runways
- added a GPS, to turn it on/off use the cmd "gps", it displays the
  following information on top of the screen (from left to right):
  heading, x coordinate, y coordinate and altitude 
- vehicle loadouts now depend on the gameset (later they will of
  course be individual)
- new skin from Hedhunta (well done!)
- vehicles without speedbrakes can use the speedbrake functionality
  on the ground (ie as wheelbrakes, to have the means to stop)
- slight modifications of datasheets
- added a first cut of takeoff and landing (will be refined later!
  and probably still buggy as hell)
- added gearheight and tailangle (angle the nose is pointing while 
  on ground with low speed - only on planes with no nosewheel) 
- added plane ground movement
- func_explosive and func_runway can be blown up by setting their
  "health" key to a value greater than 0 (check for bugs!)
- implemented "func_explosive" entity
- implemented "func_runway" entity
- minor fixes and changes

+++v0.20+++
-----------
- planes can no more fly through water
- fixed a collision detection bug
- changed bomb icon on the HUD to the railgun icon (because railgun 
  ammo is used as bomb-pickup) - later there will of course be a 
  proper HUD with all-new icons
- vehicles max number of certain ammo types changed (32 rockets, 8
  bombs, 250 rounds for the autocannon), ie no more possible to pick
  up more than this
- planes get more bullets on startup
- bullets do more damage now (and tracer bullets do full damage; they 
  only did half the damage before, and when g_tracer was set to 0 -ie
  every bullet a tracer- it took forever to kill someone)
- bot picking up flag in CTF should no more stop the game - ie CTF
  should now work properly with bots!
- flag model now visible (very visible indeed!) on vehicles
- flags now dropped properly when a vehicle dies in CTF
- added a (small) smoke trail to FFARs
- removed "swooosh" sound when changing weapons
- unavailable options in start-game menu now grayed
- encyclopedia and vehicleselect can now also be bound from controls
  setup menu
- fixed the crash-into-void-when-dead-doesn't-finish-you-off bug (oops)
- minor fixes

+++v0.19+++
-----------
- crashing into func_bobbing now should always kill you (func_bobbings
  are those moving platforms in q3dm19 for example)
- hitting the sky should no more kill the planes
- slightly changed vehicle datasheets
- increased fuel consumption on afterburner
- new skin for Gripen
- added pilot model
- added new server-cvar g_tracer (default 2) for the number of bullet 
  tracers (a value of two means every third bullet is a tracer)
- vehicles do not get weapons from target_give anymore
- vehicles now have a default camera distance (so that the camera is not
  inside big vehicles, and not too far away from smaller ones), and the
  cvars cg_thirdPersonRange and cg_thirdPersonHeight (now defaulting to 0)
  are now added to that default distance
- health pickups now give you a PERCENTAGE of health (ie 5, 25, 50, 100)
- killing another plane now gives you 3 points instead of one (bots
  still get only one) - check for bugs!
- removed all the temp airq2 stuff (ie airq2 vehicle can no more be
  used in mfq3)
- added MFQ3 subsection to controls menu (only a first cut)
- added new model from oq: JAS 39 Gripen
- other minor changes

+++v0.18+++
-----------
- dualguns now firing two projectiles at a time
- muzzleflashes positioned correctly
- removed most of the AirQ2 vehicles
- implemented fuel and fuel display on HUD (ughh UGLY! we really could
  need a new HUD soon...)
- bullets now start from correct position
- vehicleinformation now contains info about gun offset (to have
  the correct startpos for projectiles), and maxfuel
- increased bullet speed and number of tracers
- changed remaining Q3-style menus to MFQ3-style
- added first cut of encyclopedia in MFQ3-style (not finished)
- minor fixes

+++v0.17+++
-----------
- added new weapon: Machinegun (weapon 12 for now; different to Q3 MG!)
- using Schez's new weapon models
- exhaust nozzle frames for afterburner, and new F-16 from Schez
- vehicles do no more bleed when hit (game still stable?)
- vehicles now explode in fireball (and no more as bloodcloud)
- added smoketrail to damaged planes
- added new afterburner effect
- added oq's new main menu banner model (cool!)
- removed unnecessary debug information
- prevent too early respawning
- added more menu artwork from oq
- suiciding does not instantly gib you anymore
- dying planes now spin towards the ground
- exploding planes now do radius damage (no effect played yet though)
- bombs can only be dropped with a bankangle less than 60 degrees
- bugfix: vapor effects cleared when freelook cam is used
- minor adjustments to datasheets 
- vehicles now using maxhealth from their datasheet
- slightly increased fire-rate of rockets
- railgun ammo used for iron bombs for now
- added new weapon: Iron Bomb (weapon 11 for now)
- and tons of other minor fixes

+++v0.16+++ 
-----------
- disabled picking up and using "normal Q3" weapons and items (except 
  ammo_rockets which is used for the FFAR)
- added new weapon: FFAR (weapon 10 for now)
- code framework changes for new weapons (needs to be reworked when bots
  are kicked out)
- further work on ingame menu system (not yet finished)
- further work on menu system (using oq's cool new stuff), not yet 
  completely finished
- other minor changes

+++v0.15+++
-----------
- code preparations for new weapons
- dead planes don't drop weapons anymore
- changed afterburner effect (Jeremy, Schez is it ok now ?)
- added engine sounds (BUT: I need better sound files!!)
- first part of oq's new menu implemented
- split up the effects.shader into several shader files to make it work
  on all machines
- fixed a problem where gameset information was lost, when changed in
  UI and then a game was started
- fixed a bug where vehicle information was lost after a vid_restart
- added some new capabilities flags to vehicles (see vehicleinfo doc)
- minor fixes

+++v0.14+++
-----------
- fixed some client-server discrepancies (lost data)
- added a new model, and a new body part for it and animation
- improved throttle handling
- added effects for afterburner and vapor
- more work on flight physics
- added control surface movement
- implemented gear and speedbrakes, and their animations as well as
  the physical effects of these
- code for new MFQ3 modelling conventions, no more hardcoded F-16!
- adjusted data naming conventions (hope all UI stuff still works)
- fixed a bug that didnt allow you to select the vehicle you just had after
  changing map
- speed indicator coloring now according to actual vehicledata, and can now
  also accomodate speeds up to 9999 
- tweaking the speeds of F-16 to find optimal values (are they 
  better now ?)
- removed the last remainders of frame-rate dependency
- minor fixes

+++v0.13+++ 
-----------
- fixed throttle overreaction
- preparations for gear and speedbrake code
- F-16 frames now displayed for in-flight configuration
- armor, holdable items and powerups can no more be picked up by 
  MFQ3 vehicles
- first few changes to HUD: Instead of armor the HUD now displays
  speed and throttle (in percent, where >100% means afterburner),
  Speed color codes are: Red = below stallspeed, Yellow = slightly
  above stall speed, Green = speed where gear can be used (gear not
  yet coded though) and White = highspeed
  NOTE: These early HUD changes are just so that the player can get the
  necessary information, the "real MFQ3 HUD" will of course be 
  completely different and much prettier (so don't tell me you don't
  like it!!)
- fixed a bug where the plane occasionally turned the wrong way (ie 
  instead of +90 it turned -270)
- fixed a bug where you occasionally could see a vehicle at the 
  intermission point after selecting a vehicle
- fixed a bug that didn't allow you to select the vehicle you had before
  after a map_restart
- fixed a (nasty!) bug that caused models to flicker and disappear 
  (reminds me of the tank-turrets disappearing in AirQ2 hehe)
- added error-checking to catch Null Src in Q_strncpyz (will cause the
  game to pop out to the main mene - will be removed later), please test
  whether the game pops out in bot- or multiplayer games when persons
  are killed (in this case it should display 'MFQ3 Error (#)' which you
  should report to me; thx!)
- minor stuff


+++v0.12+++ 
-----------
- TEST: added first MFQ3 vehicle (replacing old F-16)
  NOTE: This is hardcoded teststuff in order to find modelling 
  conventions, and it is also still VERY BUGGY. So if you want to play
  then use the other vehicles!
- fixed a vehicleselection bug (gameset not taken into consideration,
  it always allowed every vehicle)
- added freelook to camerasystem, to use it bind a button to "+button9"
  and while you hold this button you can look whereever you want and
  the plane keeps its current heading
- added new (repeatable) commands, "+button7" (gear), "+button8" (brakes)
  and "+button9" (freecamera) - functionality for those will come later
- fixed a bug where vehicles spawned on intermission/spectator points
- fixed a bug in spectator mode (unable to turn when no vehicle was
  selected)
- fixed a bug where initial spawning left a body
- added a 'flags' field to the vehicle data, currently with two options 
  (that can of course be ORed together) VF_AIRQ2VEHICLE (using a single
  piece AirQ2 model) and VF_NOTSELECTABLE (vehicle cannot be selected,
  like for example fixed AA turrets etc, but it will be displayed in
  the encyclopedia)
- minor fixes

+++v0.11+++
-----------
- finished vehicle selection code
- fixed missing vehicle information (client now displays all vehicles
  properly)
- fixed bots spawning
- fixed bots not moving
- nose dropped less far on stalls, and recovery made easier
- increase the shadow height (and experimented with nice plane shadows,
  worked out well, but need better artwork)
- fix some bugs from the merge

+++v0.10+++ 
-----------
- final merge to the official v.1.27
- added ingame vehicle selection menu
- removed cdkey protection
- minor fixes and improvements

+++v0.09+++
-----------
- restructuring parts of the code to add more flexibility, and make
  future addons easier
- added completely new vehicle selection code for improved flexibility,
  which also makes it much easier to add new vehicles
- added first cut of encyclopedia menu (using the new vehicle selection
  code)
- fix a couple of menu bugs, some improvements of existing types
- minor stuff

+++v0.08+++ in progress...
-----------
- completely new menu system
- lots of new artwork (don't forget I am a coder not an artist!)
- new cvar mf_gameset
- minor fixes

+++v0.07+++ 
-----------
- porting from 1.17 to 1.27

+++v0.06+++ 
-----------
- first cut of climb/descend dependent acceleration (all the values
  need tweaking of course)
- added stalls
- implemented simple stall recovery procedure: if you are in a stall
  then look downwards and in the direction of flight, and apply thrust
  (this should simulate releasing the stick/yoke and pointing it down -
  the procedure you have to do in real life) - when you are fast enough
  you will gain control again
- planes now spawn with speed higher than stallspeed
- minor fixes and cleanup

+++v0.05+++ 
-----------
- reduced frame-rate dependency of acceleration (but it still is, but this
  problem should be solved in the 1.27 source release)
- acceleration now depends on thrust
- fixed server overflow: commands increase and decrease removed now, instead
  use "+button5" to increase thrust and "+button6" to decrease thrust, ie
  throttle code completely rewritten
- slightly increase speed of planes
- minor fixes


+++v0.04+++ 
-----------
- vehicles are now cached on startup
- implemented (simple) encyclopedia to review vehicle data, to see it
  either go there from the main menu or with the command "encyclopedia"
  (NOTE: This is just a mockup, when we have artwork it will look MUCH better)
- implemented plane acceleration/deceleration (no stalling yet)
- implemented throttle, use commands "increase" and "decrease" to
  change throttle in 10% steps
- implemented dynamic bounding box (untested yet), when you bank the width of
  the bounding box decreases and the height increases
- some more code generalisation (for later use with different vehicle types)

+++v0.03a+++ quick fixes
------------
- after dying camera stops moving
- fixed obituary bug (causing the game to exit to main menu)
- some more cleanup

+++v0.03+++
-----------
- added basic obituary for planes (so far the only new message is when you 
  crash into the landscape)
- vehicles can now crash into landscape
- made spectator mode work properly
- cvar cg_thirdperson now usable and by default set to 1
  NOTE: You _can_ use first person view ("cockpit view", cockpit artwork will
        be added later), but I strictly recommend you _do not_ use it for now.
        The current vehicle-follows-camera control method can give you no
        indication of your flight direction. My recommendation: Wait for the
        camera-follows-vehicle control method (ie the sim-like controls) to
        use cockpit view (and you wont get motion sick and wont crash too often)
- viewweapon removed from first person view
- vehicle no more visible from first person view
- vehices do no more teleport

+++v0.02+++
-----------
- added cvar cg_thirdPersonHeight to set the camera height properly
  (to set the distance you can use cg_thirdPersonRange (was already in Q3))
- slightly improved camera for the new movement code
- made very basic plane movement (not sim like, sim like will come later)
- added a couple more of the old AirQ2 planes (for testing)
- improved vehicle selection (to utilize the new planes)
- some code cleanup

+++v0.01+++
-----------
- spawns an F16
- added vehicle selection menu to ingame menu
- main menu changed
- vehicle information now available to all three modules
- third person view now default view
- first adjustments to code

+++BEGIN OF PROJECT+++
----------------------
- Start Date: Tuesday Oct. 10th
- New team built







Testing Notes:
==============
Unless otherwise stated test versions that you receive are for your local use
only and not to be distributed. You can of course play it with your siblings
and friends to test it, but keep it private, I don't want to get emails from
people -who are not supposed to have it- saying something like "this doesn't work
properly" etc.
The following just states what you have to look at when testing the mod. I want
to be able to find and eliminate bugs as early as possible to save a lot of time
later. Therefore you need to make sure you properly test all the new stuff and 
also do some regression tests, ie see if older features still work. The first 
thing you should do is read through the history section to find out what was 
changed lately and read through the to-do section to see known problems and not
yet implemented features. When you send feedback of your testing don't send an
email "this or that doesn't work.", but send a report as complete as possible,
I will probably provide you with a template document to fill out in the process
of testing, use this form ("testing.txt") to keep my workload, in trying
to find useful information from the reports, minimal. The following is a rough
overview of what you should especially take care of when testing:

- if possible do multiplayer testing, make sure all clients run smoothly and the
  prediction looks ok; jerky or jumpy movement means I might have a problem with
  the prediction code which needs to be addressed ASAP. Make sure the content is
  consistent on all machines in the network. I can not overstate the importance
  of multiplayer testing, it is essential to get support and to keep the community
  alive and make sure MFQ3 gets plenty of servers on the net for everyone to play.
- check all the cvars the have been changed or newly implemented or removed by
  me, make sure they work as expected and dont cause sideeffects on other clients
- when encountering bugs make sure you can reproduce them and know (and can tell
  me) how to reproduce them
- if somehow possible make sure the game works on different operating systems, as
  I only have access to Win98 and Win2k I cannot test other platforms, if you have
  the chance please make sure it works properly and as expected on non Win-platforms.
  The .qvm files are supposed to work equally fine on different platforms.
- test all new and old features according to the test document which comes with
  the test versions
- report results to bjoern.drabeck@planetquake.com 


