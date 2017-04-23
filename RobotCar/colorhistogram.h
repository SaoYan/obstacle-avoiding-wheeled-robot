#ifndef COLORHISTOGRAM_H
#define COLORHISTOGRAM_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

class ColorHistogram
{
public:
    ColorHistogram();
    void getHistogram(const cv::Mat &image);
    void getHistogram(const cv::Mat &image,
                           float minValue,
                           float maxValue,
                           int dim);
    void getHueHistogram(const cv::Mat &image,
                         int minSaturation=0);
    void getSatHistogram(const cv::Mat &image,
                         int minValue=50);
public:
    cv::MatND hist;    //histogram

private:
    int histSize[3];         //numbers of items in each dimension
    float hranges[2];        //max & min of each item
    const float* ranges[3];
    int channels[3];         //3 channels
};

#endif // COLORHISTOGRAM_H
