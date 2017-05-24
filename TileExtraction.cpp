#include "plugins\TileExtraction\TileExtraction.h"

// DPTK headers
#include "Algorithm.h"
#include "Geometry.h"
#include "Global.h"
#include "Image.h"
#include "archive\Session.h"

// Poco header needed for the macros below 
#include <Poco/ClassLibrary.h>

// Declare that this object has AlgorithmBase subclasses
//  and declare each of those sub-classes
POCO_BEGIN_MANIFEST(sedeen::algorithm::AlgorithmBase)
POCO_EXPORT_CLASS(sedeen::algorithm::TileExtraction)
POCO_END_MANIFEST

namespace sedeen {
namespace algorithm {

TileExtraction::TileExtraction()
    : box_width_(),
      box_spacing_(),
      x_offset_(),
      y_offset_(),
	  optimal_threshold_(-1),
	  channel_index_(1),
	  downsample_size_(),
      scale_(1.0),
	  scaleResolution_(4.0),
	  display_area_(),
	  //behavior_(image::tile::Threshold::RETAIN_BRIGHTER),
	  window_size_(),
	  threshold_(),
	  numResolutionLevel_(),
	  ResolutionLevel_(),
	  save_option_(),
	  output_option_(),
	  channel_factory_(),
      threshold_factory_(),
      morphology_factory_(),
	  intermediate_result_(),
      results_() {
}

TileExtraction::~TileExtraction() {
}

void TileExtraction::run() {

	// On the first call to this method, determine optimal threshold value
	if (-1 == optimal_threshold_) {
		optimal_threshold_ = getOptimalThreshold();
	}

	// Build pipeline by chaining together all of the kernels
	auto pipeline_changed = buildPipeline(optimal_threshold_);

	auto numResolutions = getNumResolutionLevels(image());

	if ( pipeline_changed ) 
	{

		drawTileBox( );
		if((int)save_option_)
		{
			SaveToXMLFile();
		}
	}

	if (output_option_.isChanged()) {
		updateIntermediateResult();
	}

}

void TileExtraction::init(const image::ImageHandle& input_image) {

	results_ = createOverlayResult(*this);

	// Bind intermediate result image to UI
	intermediate_result_ = createImageResult(*this, "Final Image");

	// Default number of regions to place along the narrowest dimension.
	const auto DEFAULT_NUM_REGIONS = 8;
	// Default fraction of the spacing occupied by the ROI
	const auto DEFAULT_FRACTION = 0.1;

	auto image_size = getDimensions(input_image, 0);
	auto narrowest_dim = std::min(image_size.width(), image_size.height());
	const auto DEFAULT_SPACING = narrowest_dim / DEFAULT_NUM_REGIONS;
	const auto DEFAULT_SIZE = int(DEFAULT_SPACING * DEFAULT_FRACTION);

	const auto maxTileSize = 4096;

	box_width_ = createIntegerParameter(
		*this,
		"Size",
		"Value assigned as both the width and height of each box",
		DEFAULT_SIZE,
		1,
		std::min(narrowest_dim, maxTileSize),
		false);

	box_spacing_ = createIntegerParameter(
		*this,
		"Spacing",
		"Number of pixels between centres of each box",
		DEFAULT_SPACING,
		1,
		narrowest_dim,
		false);

	x_offset_ = createIntegerParameter(
		*this,
		"X-offset",
		"Y-Offset from the origin of the first ROI",
		DEFAULT_SPACING / 2,
		0,
		image_size.width(),
		false);

	y_offset_ = createIntegerParameter(
		*this,
		"Y-offset",
		"Y-Offset from the origin of the first ROI",
		DEFAULT_SPACING / 2,
		0,
		image_size.height(),
		false);

	// Create morphology window size selector and bind member to UI
	window_size_ = createIntegerParameter(*this,
		"Window Size",
		"Window size use for morphology.",
		5,   // initial value
		1,   // minimum value
		10,  // maximum value
		false);

	threshold_ = createDoubleParameter(*this,
		"Threshold",
		"Threshold to identify tissue regions",
		0.2, // initial value 20%
		0.0, // minimum value
		1,   // maximum value
		false);

	//Create resolution option list and bind member to UI
	scaleResolution_ = std::min( image_size.width() /getDimensions(input_image, 1).width(),
		 image_size.height() /getDimensions(input_image, 1).height());
	numResolutionLevel_ = getNumResolutionLevels(input_image)-1;
	double maxMagnificationm = getMaximumMagnification(input_image);

	std::vector<std::string> ResolutionList;
	ResolutionList.push_back("5.0X");
	ResolutionList.push_back("10.0X");
	ResolutionList.push_back("20.0X");
	ResolutionList.push_back("40.0X");
	ResolutionList.push_back("80.0X");

	int i=0;
	while( (int)maxMagnificationm % 5 == 0 && i < 5 )
	{
		if(!ResolutionList.empty()){
			ResolutionList_.push_back(ResolutionList.at(i));
			i++;
		}

		maxMagnificationm /=2;
	}
	std::reverse(ResolutionList_.begin(), ResolutionList_.end());
	ResolutionLevel_ = createOptionParameter(
		*this,
		"Resolution",
		"Tiles will be saved at selected resolution",
		0,
		ResolutionList_,
		false);

	//Create save option list and bind member to UI
	std::vector<std::string> save_options;
	save_options.push_back("OFF");
	save_options.push_back("ON");
	save_option_ = createOptionParameter(
		*this,
		"Save Tiles",
		"Tiles will be saved, if the save option is ON",
		0,                  // initial selection
		save_options,
		false);   // option list

	// Create output option list and bind member to UI
	std::vector<std::string> compute_options;
	compute_options.push_back("None");
	compute_options.push_back("Binary Image");
	output_option_ = createOptionParameter(
		*this,
		"Intermediate result",
		"Optionally show the selected intermediate result.",
		0,                  // initial selection
		compute_options,
		false);   // option list

	// Calculate a suitable size for processing that is not larger that 512x512
	double scale = std::min(512.0 / image_size.width(),
		512.0 / image_size.height());
	downsample_size_ = image_size * scale;
	scale_ = scale;

	// Create system parameter - provide information about current view in UI
	display_area_ = createDisplayAreaParameter(*this);
	
}

bool TileExtraction::parametersChanged() {
	return box_width_.isChanged() ||
		box_spacing_.isChanged() ||
		x_offset_.isChanged() ||
		y_offset_.isChanged() ||
		window_size_.isChanged() ||
		(ResolutionLevel_.isChanged() && save_option_) ||
		save_option_.isChanged() ||
		threshold_.isChanged();
}

bool TileExtraction::buildPipeline(int threshold) {
	// For Cache, FilterFactory, ChannelSelect, Threshold, etc...
	using namespace image::tile;
	bool pipeline_changed = false;
	auto cache_policy = RecentCachePolicy(10);

	//
	// Create the channel selection stage applied to the source image
	//
	if (nullptr == channel_factory_) {
		auto source_factory = image()->getFactory();
		auto source_color = source_factory->getColor();

		// Create a channel select kernel
		auto kernel = std::make_shared<ChannelSelect>(channel_index_, source_color);

		// Apply kernel to source factory - creating a modified factory
		auto factory = std::make_shared<FilterFactory>(source_factory, kernel);

		// cache resulting factory for speedy results
		channel_factory_ = std::make_shared<Cache>(factory, cache_policy);
		pipeline_changed = true;
	}

	//
	// Append the thresholding stage after channel selection
	//
	if ( (nullptr == threshold_factory_) || pipeline_changed) {
		/*auto type = 0 == behavior_ ? Threshold::RETAIN_DARKER : 
			Threshold::RETAIN_BRIGHTER;*/
		auto type = Threshold::RETAIN_DARKER;
		// Create a channel threshold kernel
		auto kernel = std::make_shared<Threshold>(threshold, type);

		// Apply kernel to source factory - creating a modified factory
		auto factory = std::make_shared<FilterFactory>(channel_factory_, kernel);

		// cache resulting factory for speedy results
		threshold_factory_ = std::make_shared<Cache>(factory, cache_policy);
		pipeline_changed = true;
	}

	//
	// Append morphological stage after thresholding
	//
	if ((window_size_.isChanged()) || (nullptr == morphology_factory_) || pipeline_changed) {
		// Create a opening morphology kernel
		auto closing_kernel = std::make_shared<Closing>(window_size_);
		// Apply opening kernel to source factory - creating a modified factory
		auto close_factory = 
			std::make_shared<FilterFactory>(threshold_factory_, closing_kernel);

		// Create a closing morphology kernel
		auto opening_kernel = std::make_shared<Opening>(window_size_);

		// Apply closing kernel to opening factory - creating a modified factory
		auto open_factory = 
			std::make_shared<FilterFactory>(close_factory, closing_kernel);

		// cache resulting factory for speedy results
		morphology_factory_ = 
			std::make_shared<Cache>(open_factory, cache_policy);

		pipeline_changed = true;
	}

	if ( parametersChanged() && (int)save_option_ ) 
	{
		std::string path_to_image = 
			image()->getMetaData()->get(image::StringTags::SOURCE_DESCRIPTION, 0);
		auto found = path_to_image.find_last_of(".");
		m_path_to_root = path_to_image.substr(0, found);
		m_roi_file_name = openFile(m_path_to_root);

		pipeline_changed = true;
	}

	if( parametersChanged())
	{
		pipeline_changed = true;
	}

	return pipeline_changed;
}

int TileExtraction::getOptimalThreshold() {
	using namespace image::tile;

	// Get image source
	auto source_factory = image()->getFactory();
	auto source_color = source_factory->getColor();

	// build kernel
	auto kernel = 
		std::make_shared<image::tile::ChannelSelect>(channel_index_, source_color);

	// Build the channel select Kernel
	auto factory = 
		std::make_shared<image::tile::FilterFactory>(source_factory, kernel);

	return image::OtsuThresholdValue(factory, downsample_size_);
}

void TileExtraction::updateIntermediateResult() {
  using namespace image::tile;

   // Update UI with the results of the given factory 
  auto update_result = [&](const std::shared_ptr<Factory> &factory) {
    // Create a compositor
    auto compositor = std::unique_ptr<Compositor>(new Compositor(factory));

    // Extract image from it
    /*auto source_region = image()->getFactory()->getLevelRegion(0);
    auto image = compositor->getImage(source_region, downsample_size_);*/

    // Update UI
	DisplayRegion region = display_area_;
	auto image = compositor->getImage(region.source_region, 
                                           region.output_size);
    intermediate_result_.update(image, region.source_region);
  };

  auto source_factory = image()->getFactory();
  switch ((int)output_option_) {
  case 0:
	  update_result(source_factory);
	  break;
  case 1:
	  update_result(morphology_factory_);
	  break;
  case 2:
	  update_result(threshold_factory_);
	  break;
  case 3:
	  update_result(channel_factory_);
	  break;
  default:
	  break;
  }
   
}

void TileExtraction::SaveToXMLFile()
{
	using namespace image::tile;

	auto image_size = getDimensions(image(),0);
	/*Session s(path_to_image);
	s.saveToFile();*/
	if(tiles_.size() >0 )
		std::reverse(tiles_.begin(),tiles_.end());

	std::ofstream txtfile;
	std::string path_to_image = 
			image()->getMetaData()->get(image::StringTags::SOURCE_DESCRIPTION, 0);
	txtfile.open(m_roi_file_name + "_session.xml");
	txtfile<<"<?xml version=\"1.0\"?>\n";
	txtfile<<"<session software=\"PathCore Session Printer\" version=\"0.1.0\">\n";
	txtfile<<"    <image identifier=\""<<path_to_image+"\">\n";
	//txtfile<<image()->getMetaData()->get(image::StringTags::SOURCE_DESCRIPTION,0)+"\n";
	txtfile<<"        <overlays>\n";
	while(!tiles_.empty())
	{
		auto region_info = tiles_.back();
		txtfile<<"            <graphic type=\""
			<<region_info.type<<"\" name=\"";
		txtfile<<region_info.region<<"\" description=\""
			<<region_info.description<<"\">\n";
		txtfile<<"                <pen color=\""
			<<colorToString(region_info.style.pen().color())
			<<"\" width=\""<<region_info.style.pen().width()
			<<"\" style=\""
			<<styleToString(region_info.style.pen().style())
			<<"\"/>\n";
		txtfile<<"                <font>"<<toString(region_info.style.font())<<"</font>\n";
		txtfile<<"                <point-list>\n";
		for (int i = 0; i<region_info.points.size(); i++)
			if (region_info.points[i].getX()>0 && 
				region_info.points[i].getX()<image_size.width() &&
				region_info.points[i].getY()>0 && 
				region_info.points[i].getY()<image_size.height())
			{
				txtfile<<"                    <point>"
					<<static_cast<int>(region_info.points[i].getX())
					<<","
					<<static_cast<int>(region_info.points[i].getY())
					<<"</point>\n";
			}
			tiles_.pop_back();
			txtfile<<"                </point-list>\n";
			txtfile<<"            </graphic>\n";
	}
	txtfile<<"        </overlays>\n";
	txtfile<<"</session>";
	txtfile.close();
}

void TileExtraction::drawTileBox()
{
	using namespace image::tile;
	int selectedResolution =0;
	if(ResolutionLevel_.isUserDefined())
	{
		selectedResolution = (int)ResolutionLevel_;
	}

	auto image_size = getDimensions(image(), 0);

	// Get image from the current output image
	auto compositor = std::unique_ptr<Compositor>(new Compositor(morphology_factory_));
	auto source_region = image()->getFactory()->getLevelRegion(0);
	auto update_image = compositor->getImage(source_region, downsample_size_);

	// Compute the number of ROIs to draw in each direction
	const auto NUM_BOXES_X =
		(box_spacing_ - 1 + (image_size.width() - x_offset_)) / box_spacing_;
	const auto NUM_BOXES_Y =
		(box_spacing_ - 1 + (image_size.height() - y_offset_)) / box_spacing_;

	PointF box_size(box_width_, box_width_);

	tiles_.clear();

	// Clear old ROIs
	results_.clear();

	auto image_size_selectedRes = getDimensions(image(), selectedResolution);
	double scale_x = (double)image_size_selectedRes.width() / (double)image_size.width();
	double scale_y = (double)image_size_selectedRes.height() / (double)image_size.height();	

	// Draw each ROI
	int num_tiles =1;
	for (int y = 0; NUM_BOXES_Y != y; ++y) {
		if (askedToStop()) break;
		for (int x = 0; NUM_BOXES_X != x; ++x) {
			if (askedToStop()) break;
			PointF top_left(x_offset_ + x * box_spacing_,
				y_offset_ + y * box_spacing_);
			PointF bottom_right = PointF(top_left.getX() + box_size.getX(),
				top_left.getY() + box_size.getY());

			// Eliminating the background
			PointF top_left_scaled = PointF(top_left.getX()*scale_, 
				top_left.getY()*scale_);
			PointF bottom_right_scaled = PointF(bottom_right.getX()*scale_, 
				bottom_right.getY()*scale_);

			double sum = 0.0;
			int idx_y =(int)top_left_scaled.getY();
			while( (idx_y < (int)bottom_right_scaled.getY()) && (idx_y < downsample_size_.height()) )
			{
				if (askedToStop()) break;
				int idx_x =(int)top_left_scaled.getX();
				while( (idx_x < (int)bottom_right_scaled.getX()) && (idx_x < downsample_size_.width()) && (idx_x >= 0) )
				{
					if (askedToStop()) break;
					sum += update_image.at(idx_x, idx_y, 0);
					idx_x++;
				}
				idx_y++;
			}

			double box_area_scaled = (double)box_width_*box_width_*scale_;
			if( sum/box_area_scaled > (double)threshold_)
			{
				auto graphic_style = GraphicStyle();
				results_.drawRectangle(Rectangle(top_left, bottom_right, 0, sedeen::Center),
					graphic_style,
					"Name", "Description");

				if(top_left.getX() < image_size.width() && top_left.getY() < image_size.height() 
					&& bottom_right.getX() < image_size.width() && bottom_right.getY() < image_size.height() )
				{
					// Get tills from the current output image
					//auto source_region = image()->getFactory()->getLevelRegion(selectedResolution);
					auto source_factory = image()->getFactory();
					auto compositor = std::unique_ptr<Compositor>(new Compositor(source_factory));
					Size tileSize = Size((int)box_width_, (int)box_width_);
					Size downsample_size = tileSize;
					if(selectedResolution)
					{
						downsample_size = Size((int)box_width_/(std::pow(scaleResolution_, selectedResolution)), 
							(int)box_width_/(std::pow(scaleResolution_, selectedResolution)));
					}
					/*Rect tile = Rect(sedeen::Point(x_offset_ + x * box_spacing_,
						y_offset_ + y * box_spacing_), tileSize);	
					image::RawImage imageResolution = compositor->getImage(tile, downsample_size);*/

					sedeen::Point top_left_selectedRes = sedeen::Point(top_left.getX()*scale_x, 
						top_left.getY()*scale_y);
					sedeen::Point bottom_right_selectedRes = sedeen::Point(bottom_right.getX()*scale_x, 
						bottom_right.getY()*scale_y);

				
					Rect tile = Rect(top_left_selectedRes, downsample_size);
					image::RawImage imageResolution = compositor->getImage(selectedResolution, tile);					
					
					if((int)save_option_)
					{
						imageResolution.save(m_roi_file_name+ "_" + std::to_string((int)(top_left.getX()+ box_width_/2)) + "_" 
							+ std::to_string((int)(top_left.getY()+ box_width_/2)) + "_" 
							+ ResolutionList_.at(selectedResolution) + ".tif"); //std::to_string(selectedResolution)

						//XML file
						GraphicInfo roi_info;
						roi_info.red = 181;
						roi_info.green = 230;
						roi_info.blue = 29;
						roi_info.region = "Region " + std::to_string(num_tiles++);
						roi_info.style = graphic_style;
						roi_info.description = " ";
						roi_info.type = "rectangle";
						std::vector<PointF> points;
						roi_info.points.push_back(top_left);
						roi_info.points.push_back(PointF(bottom_right.getX(), top_left.getY()));
						roi_info.points.push_back(bottom_right);
						roi_info.points.push_back(PointF(top_left.getX(), bottom_right.getY()));
						tiles_.push_back(roi_info);
					}

				}			

			}
		}
	}
}

std::string TileExtraction::openFile(std::string path)
{
	OPENFILENAME ofn;
	char szFileName[MAX_PATH]="";
	//WCHAR szFileName[MAX_PATH]= L"";
	const std::string temp_str = m_path_to_root.substr(m_path_to_root.find_last_of("/\\") + 1);
	auto p = temp_str.find_last_of('.');
	std::string base_name;
	p >0 && p != std::string::npos ? base_name = temp_str.substr(0, temp_str.rfind(".")) : base_name = temp_str;
	std::copy( std::begin(base_name), std::begin(base_name) + std::min(base_name.size(), sizeof(szFileName)), 
				std::begin(szFileName));

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn); 
	ofn.hwndOwner = NULL;
	ofn.lpstrFilter = "*.tif";
	//ofn.lpstrFilter = "*.jpg;*.jpeg;*.tif;*.png;*.bmp";
	ofn.lpstrFile = (LPSTR)szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	//ofn.lpstrDefExt = (LPSTR)L"tif";
	ofn.lpstrInitialDir = (LPSTR) m_path_to_root.c_str();
	GetSaveFileName(&ofn);

	return ofn.lpstrFile;
}

bool TileExtraction::contains(const PointF& topLeft, const PointF& bottomRight, Size& rect_size) const
{

	bool isInside = true;
	rect_size = Size(0, 0);
	int rect_size_w = bottomRight.getX() - topLeft.getX();
	int rect_size_h = bottomRight.getY() - topLeft.getY();
	auto image_size = sedeen::image::getDimensions(image(),0);

	if(topLeft.getX() >= image_size.width() || topLeft.getY()>= image_size.height() )
	{
		isInside = false;
		return isInside;
	}

	if( bottomRight.getX() >= image_size.width() )
	{
		rect_size_w = image_size.width() - topLeft.getX();
		if(bottomRight.getY() >= image_size.height())
		{
			//compute the rect size
			rect_size_h = image_size.height()- topLeft.getY();
		}

		isInside = true;
	}
	else if(bottomRight.getY() >= image_size.height())
	{
		rect_size_h = image_size.height()- topLeft.getY();
		isInside = true;

	}

	rect_size = Size(rect_size_w, rect_size_h);

	return isInside;
}

} // namespace algorithm
} // namespace sedeen
