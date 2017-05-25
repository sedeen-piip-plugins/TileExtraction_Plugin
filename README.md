# TileExtraction_Plugin
Tile extraction plugin retrieves uniformly spaced tile images based on principles of stereology. This plugin is useful for sampling a large image for training, auto-encoders applications, etc.
Background tiles (i.e. fat tissue ) are eliminated using the existing tissue finder plugin.
User can modify the results of tissue finder algorithm by tuning the “Window Size” and “Threshold” parameters.
Patches can be saved in different resolutions, and an “.xml” file will be created to keep the coordinate of each patches at the full digital resolution.

## User Guild
1.	Open the WSI image. 
2.	Load the “TileEXtraction” plugin from the pulldown list of Algorithms.

![Analysis Manager view](https://github.com/sedeen-piip-plugins/TileExtraction_Plugin/blob/master/Images/TileExtraction_1.png)

3.  Clicking on the Run button will execute the algorithm with the default parameters. The extracted tiles are shown as an overlay over the image.
4.  Use the "Intermediate result" option to see the results of tissue finder algorithm and modify the results using the “Window Size” and “Threshold” parameters. The window size is the kernel size used to perform morphological operation withen the tissue finder algorithm. The Threshold value is in the range 0.0 to 1.0. It eliminate The tissue area with the size less than the threshold value.

![Analysis Manager view](https://github.com/sedeen-piip-plugins/TileExtraction_Plugin/blob/master/Images/TileExtraction_2.png)

5.  "Save Tiles" option allows the user to modify the results before saving the patches. The patches will be saved only and only the “Save Tiles” option set to be “ON”. The user will select the directory and the tile name in the pop up window if “Save Tile” option set to be “ON”.
6.  Also, the algorithm detects the hierarchical resolutions of the loaded image and presents them in “Resolution” combo box. The user can select the desired resolution to save the patches.

The extracted tiles will be saved with this naming format slideName_centreX_centreY_resolution.tif (for example: 99797_23090_18015_0.tif). (See Fig.3)

An “.xml” file will be created to keep the coordinates of each patch at the full digital resolution with this naming format slideName_ session.xml (for example: 99797_ session.xml).

![Analysis Manager view](https://github.com/sedeen-piip-plugins/TileExtraction_Plugin/blob/master/Images/TileExtraction_3.png)


## Authors
* Azadeh Yazanpanah

TileExtraction plugin has been developed by Martel lab at Sunnybrook Research Institute (SRI), University of Toronto.
[Funding provided by NIH.](https://itcr.nci.nih.gov/funded-project/pathology-image-informatics-platform-visualization-analysis-and-management)
