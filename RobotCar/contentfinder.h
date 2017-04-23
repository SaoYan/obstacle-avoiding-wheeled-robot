#ifndef CONTENTFINDER_H
#define CONTENTFINDER_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

class ContentFinder
{
public:
    ContentFinder();
    void setThreshold(double t);
    float getThreshold();
    void setHistogram(const cv::MatND &h);
    void findHueContent(const cv::Mat &image);
    void findHueContent(const cv::Mat &image,
                        float minValue,
                        float maxValue,
                        int dim);
    void findSatContent(const cv::Mat &image,
                        float minValue,
                        float maxValue) ;

public:
    cv::Mat result;

private:
    float hranges[2] ;
    const float* ranges[3] ;
    int channels[3] ;
    float threshold ;
    cv::MatND histogram ;
};

#endif // CONTENTFINDER_H
