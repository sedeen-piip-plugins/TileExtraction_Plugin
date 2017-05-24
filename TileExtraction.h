#ifndef SEDEEN_SRC_TILEEXTRACTION_TILEEXTRACTION_H
#define SEDEEN_SRC_TILEEXTRACTION_TILEEXTRACTION_H

// System headers
#include <memory>
#include <Windows.h>
#include <fstream>

// DPTK headers - a minimal set
#include "algorithm\AlgorithmBase.h"
#include "algorithm\Parameters.h"
#include "algorithm\Results.h"

namespace sedeen {

namespace image {

class RawImage;

namespace tile {

class ChannelSelect;
class Closing;
class FilterFactory;
class Opening;
class Threshold;

} // namespace tile

} // namespace image

namespace algorithm {


/// A uniform sampling utility based on principles of stereology 
//
/// Tile extraction plug-in is useful for sampling  a large image for training, auto-encoders applications etc..
/// Background tiles (i.e. fat tissue ) are eliminated using the existing tissue finder plug-in.
/// User can modify the results of tissue finder algorithm by tuning the “Window Size” and “Threshold” parameters.
/// Patches can be saved in different resolutions, and an “.xml” file will be created to keep the coordinate of each patches at the full digital resolution.
/// 
class TileExtraction : public AlgorithmBase {
 public:
  TileExtraction();

  virtual ~TileExtraction();

  //static const std::vector<std::string> resolutionList_;

 private:
  virtual void run();

  virtual void init(const image::ImageHandle& image);

  /// Determines the optimal threshold using Otsu's method
  //
  /// Updates the intermediate member \c channel_factory_
  int getOptimalThreshold();

  /// Check if the parameters have changed since the last invocation.
  //
  /// \return
  /// \c true if any of the parameters differ from their previously-cached
  /// counterparts, \c false otherwise.
  bool parametersChanged();


  /// Creates the foreground detection pipeline
  //
  /// Creates a Kernel for each of the steps and chains them together, storing
  /// the intermediate Factory object for each Kernel
  //
  /// \return 
  /// TRUE if the pipeline has changed since the call to this function, FALSE
  /// otherwise
  bool buildPipeline(int threshold);

  /// Updates the UI with the currently selection intermediate result
  void updateIntermediateResult();

  void drawTileBox();
  void SaveToXMLFile();

   bool contains(const PointF& topLeft, const PointF& bottomRight, Size& rect_size) const;

  std::string openFile(std::string path);

  /// Width and height, in pixels, of each box
  IntegerParameter box_width_;

  /// Number of pixels between boxes
  IntegerParameter box_spacing_;

  /// An x-offset to the starting location of ROIs
  IntegerParameter x_offset_;

  /// A y-offset to the starting location of ROIs
  IntegerParameter y_offset_;

  /// An optimal threshold determined by using Otsu's method
  int optimal_threshold_;

  /// Dimensions of the down-sampled image to use for processing
  Size downsample_size_;

  /// number of resolution level
  int numResolutionLevel_;

  /// Parameter for selecting the resolution to save the tiles
  OptionParameter ResolutionLevel_;
  std::vector<std::string> ResolutionList_;
  
  /// Scale factor to down-sampled image for processing
  double scale_;

  /// Parameter for selection of image component
  int channel_index_;

   /// Parameter for selecting threshold retainment
 // image::tile::Threshold::Behavior behavior_;

  /// Parameter for selection of morphology window size
  IntegerParameter window_size_;

  /// Parameter for selecting the tiles containg the tissue
  DoubleParameter threshold_;

  /// Parameter for selecting which of the intermediate result to display
  OptionParameter output_option_;

  /// Channel through which to report the generated overlay shapes
  OverlayResult results_;

  /// Image result reporter through which intermediate results are displayed
  ImageResult intermediate_result_;

   /// The display area parameter
  algorithm::DisplayAreaParameter display_area_;

  /// Parameter for selecting to save the tiles
  OptionParameter save_option_;

   /// The intermediate image factory after channel selection 
  std::shared_ptr<image::tile::Factory> channel_factory_;

  /// The intermediate image factory after thresholding
  std::shared_ptr<image::tile::Factory> threshold_factory_;

  /// The intermediate image factory after morphological processing
  std::shared_ptr<image::tile::Factory> morphology_factory_;

  std::string m_path_to_root;
  std::string m_path_to_image;
  std::string m_roi_file_name;

  struct GraphicInfo{
	  int red;
	  int green;
	  int blue;
	  std::string region;
	  std::string description;
	  std::string type;
	  std::vector<PointF> points;
	  GraphicStyle style;
  };

  std::vector<GraphicInfo> tiles_;
  double scaleResolution_;

};

//std::vector<std::string> fillVector()
//{
//	std::vector<std::string> list;
//	list.push_back("5X");
//	list.push_back("10X");
//	list.push_back("20X");
//	list.push_back("40X");
//	list.push_back("80X");
//
//	return list;
//}
//
//const std::vector<std::string> TileExtraction::resolutionList_ = fillVector();

} // namespace algorithm
} // namespace sedeen

#endif
