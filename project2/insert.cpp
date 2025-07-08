#include <opencv2/opencv.hpp>
#include <iostream>
#include <bitset>
using namespace std;
using namespace cv;

string textToBinary(const string &text)
{
    string binary;
    for (char c : text)
    {
        for (int i = 7; i >= 0; i--)
            binary += ((c >> i) & 1) ? '1' : '0';
    }
    return binary;
}

void dwtHaar(const Mat &src, Mat &LL, Mat &LH, Mat &HL, Mat &HH)
{
    int rows = src.rows / 2;
    int cols = src.cols / 2;
    LL = Mat(rows, cols, CV_32F);
    LH = Mat(rows, cols, CV_32F);
    HL = Mat(rows, cols, CV_32F);
    HH = Mat(rows, cols, CV_32F);
    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            float a = src.at<float>(2 * i, 2 * j);
            float b = src.at<float>(2 * i, 2 * j + 1);
            float c = src.at<float>(2 * i + 1, 2 * j);
            float d = src.at<float>(2 * i + 1, 2 * j + 1);
            LL.at<float>(i, j) = (a + b + c + d) / 4.0;
            LH.at<float>(i, j) = (a - b + c - d) / 4.0;
            HL.at<float>(i, j) = (a + b - c - d) / 4.0;
            HH.at<float>(i, j) = (a - b - c + d) / 4.0;
        }
    }
}

Mat idwtHaar(const Mat &LL, const Mat &LH, const Mat &HL, const Mat &HH)
{
    int rows = LL.rows * 2;
    int cols = LL.cols * 2;
    Mat dst(rows, cols, CV_32F);
    for (int i = 0; i < LL.rows; ++i)
    {
        for (int j = 0; j < LL.cols; ++j)
        {
            float a = LL.at<float>(i, j);
            float b = LH.at<float>(i, j);
            float c = HL.at<float>(i, j);
            float d = HH.at<float>(i, j);
            dst.at<float>(2 * i, 2 * j) = a + b + c + d;
            dst.at<float>(2 * i, 2 * j + 1) = a - b + c - d;
            dst.at<float>(2 * i + 1, 2 * j) = a + b - c - d;
            dst.at<float>(2 * i + 1, 2 * j + 1) = a - b - c + d;
        }
    }
    return dst;
}

void embedWatermark(Mat &image, const string &watermarkText, float alpha = 1.0f)
{
    Mat yuv;
    cvtColor(image, yuv, COLOR_BGR2YUV);
    vector<Mat> channels;
    split(yuv, channels);
    Mat &y = channels[0];
    Mat yFloat;
    y.convertTo(yFloat, CV_32F);
    Mat LL, LH, HL, HH;
    dwtHaar(yFloat, LL, LH, HL, HH);
    string binary = textToBinary(watermarkText);
    int bitIndex = 0;
    for (int i = 10; i < LL.rows && bitIndex < binary.size(); ++i)
    {
        for (int j = 10; j < LL.cols && bitIndex < binary.size(); ++j)
        {
            float &val = LL.at<float>(i, j);
            int bit = binary[bitIndex] - '0';
            int parity = ((int)round(val)) % 2;
            if (parity != bit)
                val += (val >= 0) ? alpha : -alpha;
            bitIndex++;
        }
    }
    Mat yReconstructed = idwtHaar(LL, LH, HL, HH);
    yReconstructed.convertTo(channels[0], CV_8U);
    merge(channels, yuv);
    cvtColor(yuv, image, COLOR_YUV2BGR);
}

int main()
{
    string inputImage = "project2.png";
    string outputImage = "watermarked.png";
    string watermarkText = "DWT_SECRET";
    Mat image = imread(inputImage);
    Mat watermarked = image.clone();
    embedWatermark(watermarked, watermarkText);
    imwrite(outputImage, watermarked);
    cout << "水印嵌入成功！保存为: " << outputImage << endl;
    return 0;
}
