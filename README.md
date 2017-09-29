<h1 align="center">TileExtraction Plugin</h1>
  
Tile extraction plugin retrieves uniformly spaced tile images based on principles of stereology. This plugin is useful for sampling a large image for training, auto-encoders applications, etc.
Background tiles (i.e. fat tissue ) are eliminated using the existing tissue finder plugin.
User can modify the results of tissue finder algorithm by tuning the “Window Size” and “Threshold” parameters.
Patches can be saved in different resolutions, and an “.xml” file will be created to keep the coordinate of each patches at the full digital resolution.

## User Manual
##### 1.	Open the WSI image in Sedeen Viewer. 
##### 2.	Load the “TileEXtraction” plugin from the pulldown list of Algorithms.

<div align="center">
  <img src="https://github.com/sedeen-piip-plugins/TileExtraction_Plugin/blob/master/Images/TileExtraction_1.png"/>
</div>
<div align="center">
  <h6> <strong>Fig1.</strong> The Analysis Manager view. The "Tile Extraction plugin" is appeared in the algorithm pop up      menu.</h6>
</div>


##### 3.  Clicking on the Run button will execute the algorithm with the default parameters. The extracted tiles are shown as an overlay rectangles over the image.
##### 4.  Use the "Intermediate result" option to see the results of tissue finder algorithm and modify the results using the “Window Size” and “Threshold” parameters. The window size is the kernel size used to perform morphological operation in the tissue finder algorithm. The Threshold value is in the range 0.0 to 1.0. It eliminate The tissue area with the size less than the threshold value.

![Analysis Manager view](https://github.com/sedeen-piip-plugins/TileExtraction_Plugin/blob/master/Images/TileExtraction_new_2.png)
<div align="center">
  <h6><strong>Fig2.</strong> Displaying the retrieved tile images.</h6>
</div>


##### 5.  "Save Tiles" option allows the user to modify the results before saving the patches. The patches will be saved only and only the “Save Tiles” option set to be “ON”. The user will select the directory and the tile base name by specifying the "Directory To Save Tiles" parameters. 
##### 6.  Also, the algorithm detects the hierarchical resolutions of the loaded image and presents them in “Resolution” combo box. The user can select the desired resolution to save the patches.

The extracted tiles will be saved with this naming format slideName_centreX_centreY_resolution.tif (for example: 99797_23090_18015_0.tif). (See Fig.3)

An “.xml” file will be created to keep the coordinates of each patch at the full digital resolution with this naming format slideName_ session.xml (for example: 99797_ session.xml).

![Analysis Manager view](https://github.com/sedeen-piip-plugins/TileExtraction_Plugin/blob/master/Images/TileExtraction_3.png)
<div align="center">
  <h6><strong>Fig3.</strong> Displaying the retrieved tile images saved to the hard drive and the associated “.xml” file.</h6>
</div>

##### 7.  Users can export the patches by selecting the "Export" button at the top of the "Overlay manager". This way the overlay rectangles will be saved as annotated regions in Sedeen format so then the users can apply another plugin on selected one. For example, users can apply "TileExtraction" plugin to extract the desirable regions and then apply the "Stain Analysis" plugin on the selected regions or even chain more than 2 pluging to provide a specific workflow. (See Fig.4)

![Analysis Manager view](https://github.com/sedeen-piip-plugins/TileExtraction_Plugin/blob/master/Images/TileExtraction_new_1.png)
<div align="center">
  <h6><strong>Fig4.</strong> Displaying the overlay manager and export option.</h6>
</div>

## Authors
TileExtraction plugin was developed by **Azadeh Yazanpanah**, Martel lab at Sunnybrook Research Institute (SRI), University of Toronto and was partially funded by [NIH grant.](https://itcr.nci.nih.gov/funded-project/pathology-image-informatics-platform-visualization-analysis-and-management)

## Copyright & License
