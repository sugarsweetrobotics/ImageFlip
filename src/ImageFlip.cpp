// -*- C++ -*-
/*!
 * @file  ImageFlip.cpp
 * @brief Image Flip Component
 * @date $Date$
 *
 * $Id$
 */

#include "ImageFlip.h"

// Module specification
// <rtc-template block="module_spec">
static const char* imageflip_spec[] =
  {
    "implementation_id", "ImageFlip",
    "type_name",         "ImageFlip",
    "description",       "Image Flip Component",
    "version",           "1.0.0",
    "vendor",            "Sugar Sweet Robotics",
    "category",          "Test",
    "activity_type",     "PERIODIC",
    "kind",              "DataFlowComponent",
    "max_instance",      "1",
    "language",          "C++",
    "lang_type",         "compile",
    // Configuration variables
    "conf.default.mode", "vertical",
    // Widget
    "conf.__widget__.mode", "spin",
    // Constraints
    "conf.__constraints__.mode", "vertical,horizontal,both,none",
    ""
  };
// </rtc-template>

/*!
 * @brief constructor
 * @param manager Maneger Object
 */
ImageFlip::ImageFlip(RTC::Manager* manager)
    // <rtc-template block="initializer">
  : RTC::DataFlowComponentBase(manager),
    m_inIn("in", m_in),
    m_outOut("out", m_out)

    // </rtc-template>
{
}

/*!
 * @brief destructor
 */
ImageFlip::~ImageFlip()
{
}



RTC::ReturnCode_t ImageFlip::onInitialize()
{
  // Registration: InPort/OutPort/Service
  // <rtc-template block="registration">
  // Set InPort buffers
  addInPort("in", m_inIn);
  
  // Set OutPort buffer
  addOutPort("out", m_outOut);
  
  // Set service provider to Ports
  
  // Set service consumers to Ports
  
  // Set CORBA Service Ports
  
  // </rtc-template>

  // <rtc-template block="bind_config">
  // Bind variables and configuration variable
  bindParameter("mode", m_mode, "vertical");
  // </rtc-template>
  
  return RTC::RTC_OK;
}

/*
RTC::ReturnCode_t ImageFlip::onFinalize()
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t ImageFlip::onStartup(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t ImageFlip::onShutdown(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t ImageFlip::onActivated(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t ImageFlip::onDeactivated(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

enum IMAGE_FORMAT {
  FMT_GRAY,
  FMT_RGB,
  FMT_JPEG,
  FMT_PNG,
};

bool convertCvMatToImg(const cv::Mat& srcImage, Img::CameraImage& dstImage, const IMAGE_FORMAT outFormat, const int compression_ratio=75) {
  int width = srcImage.cols;
  int height = srcImage.rows;
  int depth = srcImage.depth();
  int inChannels = srcImage.channels();
  int outChannels = outFormat == FMT_GRAY ? 1 : 3;

  cv::Mat procImage;
  if (outChannels > inChannels) {
    cv::cvtColor(srcImage, procImage, CV_GRAY2RGB);
  } else if (outChannels < inChannels) {
    cv::cvtColor(srcImage, procImage, CV_RGB2GRAY);
  } else {
    procImage = srcImage;
  }

  dstImage.image.width = width;
  dstImage.image.height = height;

  switch(outFormat) {
  case FMT_RGB:
    dstImage.image.format = Img::CF_RGB;
    dstImage.image.raw_data.length( width * height * outChannels );
    for(int i = 0;i < height;i++) {
      memcpy(&(dstImage.image.raw_data[i * width * outChannels]),
	     &(procImage.data[i * procImage.step]),
	     width * outChannels);
    }
    break;
  case FMT_JPEG:
    {
      dstImage.image.format = Img::CF_JPEG;
      //Jpeg encoding using OpenCV image compression function
      std::vector<int> compression_param = std::vector<int>(2); 
      compression_param[0] = CV_IMWRITE_JPEG_QUALITY;
      compression_param[1] = compression_ratio;
      //Encode raw image data to jpeg data
      std::vector<uchar> compressed_image;
      cv::imencode(".jpg", procImage, compressed_image, compression_param);
      //Copy encoded jpeg data to Outport Buffer
      dstImage.image.raw_data.length(compressed_image.size());
      memcpy(&dstImage.image.raw_data[0], &compressed_image[0], sizeof(unsigned char) * compressed_image.size());
    }
    break;
  case FMT_PNG:
    {
      dstImage.image.format = Img::CF_PNG;
      //Jpeg encoding using OpenCV image compression function
      std::vector<int> compression_param = std::vector<int>(2); 
      compression_param[0] = CV_IMWRITE_PNG_COMPRESSION;
      compression_param[1] = (int)((double)compression_ratio/10.0);
      if(compression_param[1] == 10)
	compression_param[1] = 9;
      //Encode raw image data to jpeg data
      std::vector<uchar> compressed_image;
      cv::imencode(".png", procImage, compressed_image, compression_param);
      //Copy encoded jpeg data to Outport Buffer
      dstImage.image.raw_data.length(compressed_image.size());
      memcpy(&dstImage.image.raw_data[0], &compressed_image[0], sizeof(unsigned char) * compressed_image.size());
    }
    break;
  case FMT_GRAY:
    {
      dstImage.image.format = Img::CF_GRAY;
      dstImage.image.raw_data.length( width * height * outChannels);
      for(int i(0); i< height; ++i) {
	memcpy(&(dstImage.image.raw_data[i * width * outChannels]),
	       &(procImage.data[i * procImage.step]),
	       width * outChannels);
      }
    }
    break;
  default:
    return false;
  }
  return  true;
}

bool convertImgToCvMat(const Img::CameraImage& srcImage, cv::Mat& dstImage) {
  int channels = 1;
  int width = srcImage.image.width;
  int height = srcImage.image.height;
  int format = srcImage.image.format;
  int data_length = srcImage.image.raw_data.length();
  int image_size = width * height * channels;
  
  switch(format) {
  case Img::CF_GRAY:
    channels = 1;
    break;
  case Img::CF_RGB:
  case Img::CF_PNG:
  case Img::CF_JPEG:
    channels = 3;
    break;
  default:
    channels = (srcImage.image.raw_data.length()/width/height);
  }
  
  if (channels == 3) {
    dstImage.create(height, width, CV_8UC3);
  } else {
    dstImage.create(height, width, CV_8UC1);
  }
  
  switch(format) {
  case Img::CF_RGB:
    {
      for(int i = 0; i < height;i++) {
	memcpy(&dstImage.data[i*dstImage.step], 
	       &srcImage.image.raw_data[i*width*channels],
	       sizeof(unsigned char)*width*channels);
      }
      if(channels == 3) {
	//cv::cvtColor(dstImage, dstImage, CV_RGB2BGR);
      }
    }
    break;
  case Img::CF_JPEG:
  case Img::CF_PNG:
    {
      std::vector<uchar> compressed_image(data_length);
      memcpy(&compressed_image[0], &srcImage.image.raw_data[0], sizeof(unsigned char) * data_length);
      
      //Decode received compressed image
      cv::Mat decoded_image;
      if(channels == 3) {
	decoded_image = cv::imdecode(cv::Mat(compressed_image), CV_LOAD_IMAGE_COLOR);
	//cv::cvtColor(decoded_image, dstImage, CV_RGB2BGR);
      }
      else {
	decoded_image = cv::imdecode(cv::Mat(compressed_image), CV_LOAD_IMAGE_GRAYSCALE);
	dstImage = decoded_image;
      }
    }
    break;
  default:
    return false;
  }
  return true;
}



RTC::ReturnCode_t ImageFlip::onExecute(RTC::UniqueId ec_id)
{
  if (m_inIn.isNew()) {
    m_inIn.read();
    if (convertImgToCvMat(m_in.data, m_srcImage)) {
      if (m_mode == "vertical") {
	cv::flip(m_srcImage, m_srcImage, 1);
      } else if (m_mode == "horizontal") {
	cv::flip(m_srcImage, m_srcImage, 0);
      } else if (m_mode == "both") {
	cv::flip(m_srcImage, m_srcImage, -1);
      }
      convertCvMatToImg(m_srcImage, m_out.data, FMT_RGB);

      setTimestamp(m_out);
      m_out.data.captured_time = m_out.tm;
      m_outOut.write();
      
    } else {
      RTC_DEBUG(("Failed To Decode Image."));
      return RTC::RTC_ERROR;
    }
  }
  return RTC::RTC_OK;
}


/*
RTC::ReturnCode_t ImageFlip::onAborting(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t ImageFlip::onError(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t ImageFlip::onReset(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t ImageFlip::onStateUpdate(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t ImageFlip::onRateChanged(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/



extern "C"
{
 
  void ImageFlipInit(RTC::Manager* manager)
  {
    coil::Properties profile(imageflip_spec);
    manager->registerFactory(profile,
                             RTC::Create<ImageFlip>,
                             RTC::Delete<ImageFlip>);
  }
  
};


