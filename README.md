# TileExtraction_Plugin
Tile extraction plug-in retrieves uniformly spaced tile images based on principles of stereology. This plug-in is useful for sampling a large image for training, auto-encoders applications, etc.
Background tiles (i.e. fat tissue ) are eliminated using the existing tissue finder plug-in.
User can modify the results of tissue finder algorithm by tuning the “Window Size” and “Threshold” parameters.
Patches can be saved in different resolutions, and an “.xml” file will be created to keep the coordinate of each patches at the full digital resolution.

This Plug-in has been implemented by Azadeh Yazanpanah in Visual Studio C++ 2012 and tested with “Sedeen Viewer SDK version V5.2.1.427”. Funding from NIH and Feddev.
