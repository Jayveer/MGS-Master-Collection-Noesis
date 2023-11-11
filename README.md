# Metal Gear Solid Master Collection Noesis Plugin

This is a plugin for [Noesis](https://richwhitehouse.com/index.php?content=inc_projects.php&showproject=91) which allows the user to view textured 3D Models and animations from the PC Master Collection version of the games Metal Gear Solid 2 and 3.

It currently supports kms, evm and mdl files for models, tri files for textures and mtar or mar files for animation. I am not sure why the game still contains these files as I can't imagine it uses them other than as a lookup.

The winding order in mgs3 mdl files issue is still prevelant in this version and thus those models are rendered two sided.

### Latest Changes
 - Added support for viewing MGS2 KMS, EVM models and MAR motions
 - Added preliminary support for viewing Master Collection CMDL models 

##  Usage.

Drag the dll file into the plugins folder of your Noesis folder, and find and locate the MDL file you wish to view. This program assumes models are in a mdl folder in assets, and textures are in a tri file in assets. If textures are available they will be applied automatically, this may take a while.

If you also have my mgs3 or mgs2 noesis plugins you may have to swap it out to use this. It's a rather inelgant solution than what I'd usually make, but at this point I don't care, the leechers can do that much at least.

There is only one option which when checked allows you to load Mtar or Mar animation files for the model if it has bones.

##### Prompt for Motion Archive
This option will allow you to choose an Mtar or Mar file after the model has loaded. This allows you to view animations provided the bones match.
