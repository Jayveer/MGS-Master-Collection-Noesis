# Metal Gear Solid Master Collection Noesis Plugin

This is a plugin for [Noesis](https://richwhitehouse.com/index.php?content=inc_projects.php&showproject=91) which allows the user to view textured 3D Models and animations from the PC Master Collection version of the game Metal Gear Solid 3.

It currently only supports mdl files for models, tri files for textures and mtar files for mtar files for animation. I am not sure why the game still contains these files as I can't imagine it uses them.

The winding order issue is still prevelant in this version and thus the models are rendered two sided.

##  Usage.

Drag the dll file into the plugins folder of your Noesis folder, and find and locate the MDL file you wish to view. This program assumes models are in a mdl folder in assets, and textures are in a tri file in assets. If textures are available they will be applied automatically, this may take a while.

If you also have my mgs3 noesis plugin you may have to swap it out to use this. It's a rather inelgant solution than what I'd usually make, but at this point I don't care, the leechers can do that much at least.

There is only one option which when checked allows you to load Mtar animation files for the model if it has bones.

##### Prompt for Motion Archive
This option will allow you to choose an Mtar file after the model has loaded. This allows you to view animations provided the bones match.
