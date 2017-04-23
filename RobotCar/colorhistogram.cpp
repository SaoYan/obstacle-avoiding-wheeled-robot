#include "colorhistogram.h"

ColorHistogram::ColorHistogram()
{
    histSize[0]=256; histSize[1]=256; histSize[2]=256;  //256 values in each dimension
    hranges[0]=0.0; hranges[1]=255.0;                   //max & min of each item
    ranges[0]=hranges; ranges[1]=hranges; ranges[2]=hranges;
    channels[0]=0; channels[1]=1; channels[2]=2;        //3 channels

}

void ColorHistogram::getHistogram(const cv::Mat &image)
{
    cv::calcHist(&image,    //input image
                 1,         //number of imput images
                 channels,  //channels
                 cv::Mat(), //mask
                 hist,      //output histogram
                 3,         //3-dimension histogram
                 histSize,  //value of each item
                 ranges);   //ranges of each dimension
}

void ColorHistogram::getHistogram(const cv::Mat &image,
                       float minValue,
                       float maxValue,
                       int dim)
{
    hranges[0]=minValue;
    hranges[1]=maxValue;
    int histSize_[dim];
    const float* ranges_[dim];
    int channels_[dim];

    for(int i=0;i<dim;i++)
    {
        histSize_[i]=256;
        ranges_[i]=hranges;
        channels_[i]=i;
    }

    cv::calcHist(&image,1,
                 channels_,
                 cv::Mat(),
                 hist,
                 dim,
                 histSize_,
                 ranges_);
}

void ColorHistogram::getHueHistogram(const cv::Mat &image,
                                     int minSaturation)
{
    cv::Mat hsv;
    cv::cvtColor(image,hsv,CV_BGR2HSV);

    //创建掩码矩阵
    cv::Mat mask;
    if(minSaturation>0)
    {
        std::vector<cv::Mat>v;
        cv::split(image,v);
        cv::threshold(v[1],mask,minSaturation,
                255,cv::THRESH_BINARY);
    }

    //参数
    float hranges_[2];
    hranges_[0]=0.0; hranges_[1]=180.0;
    const float *ranges_[1];
    ranges_[0]=hranges_;
    int channels_[1];
    channels_[0]=0;  //仅使用色调（Hue）分量
    int histSize_[1];
    histSize_[0]=181;

    //1D色调直方图
    cv::calcHist(&hsv,
                 1,
                 channels_,
                 mask,
                 hist,
                 1,
                 histSize_,
                 ranges_);
}

void ColorHistogram::getSatHistogram(const cv::Mat &image,
                                     int minValue)
{
    cv::Mat hsv;
    cv::cvtColor(image,hsv,CV_BGR2HSV);

    //创建掩码矩阵
    cv::Mat mask;
    if(minValue>0)
    {
        std::vector<cv::Mat>v;
        cv::split(image,v);
        cv::threshold(v[2],mask,minValue,
                255,cv::THRESH_BINARY);
    }

    //参数
    float hranges_[2];
    hranges_[0]=0.0; hranges_[1]=180.0;
    const float *ranges_[1];
    ranges_[0]=hranges_;
    int channels_[1];
    channels_[0]=1;  //仅使用饱和度（Sat）分量
    int histSize_[1];
    histSize_[0]=181;

    //1D色调直方图
    cv::calcHist(&hsv,
                 1,
                 channels_,
                 mask,
                 hist,
                 1,
                 histSize_,
                 ranges_);
}
