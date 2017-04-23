#include "contentfinder.h"

ContentFinder::ContentFinder()
{
    hranges[0]=0.0; hranges[1]=255.0;
    ranges[0]=hranges; ranges[1]=hranges; ranges[2]=hranges;
    channels[0]=0; channels[1]=1; channels[2]=2;
    threshold=-1.0f;
}

void ContentFinder::setThreshold(double t)
{
    threshold=t;
}

float ContentFinder::getThreshold()
{
    return threshold;
}

void ContentFinder::setHistogram(const cv::MatND &hist)
{
    histogram=hist;
    cv::normalize(histogram,histogram,1.0);
}

void ContentFinder::findHueContent(const cv::Mat &image)
{
    /**
     * calcBackProject工作原理
     * 假设我们有一张100x100的输入图像，有一张10x10的模板图像，查找的过程是这样的：
     * （1）从输入图像的左上角(0,0)开始，切割一块(0,0)至(10,10)的临时图像；
     * （2）生成临时图像的直方图；
     * （3）用临时图像的直方图和模板图像的直方图对比，对比结果记为c；
     * （4）直方图对比结果c，就是结果图像(0,0)处的像素值；
     * （5）切割输入图像从(0,1)至(10,11)的临时图像，对比直方图，并记录到结果图像；
     * （6）重复（1）～（5）步直到输入图像的右下角。
     * 会返回一个像素比例的灰度图，区域的直方图越接近，则返回的值越大，乘255放大后越白
     */
    cv::calcBackProject(&image,
                        1,
                        channels,
                        histogram,
                        result,
                        ranges,
                        255.0);
    /**
     * 阈值类型参数 ：
     * 1. THRESH_BINARY:
     * dst(x,y) = src(x,y) > threshold ? max_value : 0
     * 2. THRESH_BINARY_INV:
     * dst(x,y) = src(x,y)>threshold ? 0 : max_value
     * 3. THRESH_TRUNC:
     * dst(x,y) = if src(x,y) > threshold ? src(x,y) : threshold
     * 4. THRESH_TOZERO:
     * dst(x,y) = if src (x,y) > threshold ? src(x,y) : 0
     * 5. THRESH_TOZERO_INV:
     * dst(x,y) = if src(x,y)>threshold ? 0 : src(x,y)
     */
    if((threshold>0)&&(threshold<1))
    {
        cv::threshold(result,result,
                      255*threshold,255,
                      cv::THRESH_BINARY);
    }
}

void ContentFinder::findHueContent(const cv::Mat &image,
                                float minValue,
                                float maxValue,
                                int dim)
{
    if(minValue<0) minValue=0;
    if(maxValue>255) maxValue=255;
    if(dim<1) dim=1;
    if(dim>3) dim=3;

    hranges[0]=minValue;
    hranges[1]=maxValue;
    int channels_[dim];
    const float* ranges_[dim];

    for(int i=0;i<dim;i++)
    {
        channels_[i]=i;
        ranges_[i]=hranges;
    }

    cv::calcBackProject(&image,
                        1,
                        channels_,
                        histogram,
                        result,
                        ranges_,
                        255.0);
    if((threshold>0)&&(threshold<1))
    {
        cv::threshold(result,result,
                      255*threshold,255,
                      cv::THRESH_BINARY);
    }
}

void ContentFinder::findSatContent(const cv::Mat &image,
                                float minValue,
                                float maxValue)
{
    if(minValue<0) minValue=0;
    if(maxValue>255) maxValue=255;

    hranges[0]=minValue;
    hranges[1]=maxValue;
    int channels_[1];
    const float* ranges_[1];

    channels_[0]=1;
    ranges_[0]=hranges;

    cv::calcBackProject(&image,
                        1,
                        channels_,
                        histogram,
                        result,
                        ranges_,
                        255.0);
    if((threshold>0)&&(threshold<1))
    {
        cv::threshold(result,result,
                      255*threshold,255,
                      cv::THRESH_BINARY);
    }
}

