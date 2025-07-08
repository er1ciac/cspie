#include <opencv2/opencv.hpp>
#include <iostream>
#include <bitset>
using namespace std;
using namespace cv;

string binaryToText(const string &binary)
{
    string text;
    for (size_t i = 0; i + 7 < binary.size(); i += 8)
    {
        bitset<8> b(binary.substr(i, 8));
        text += static_cast<char>(b.to_ulong());
    }
    return text;
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

string extractWatermark(const Mat &image, int watermarkLength)
{
    Mat yuv;
    cvtColor(image, yuv, COLOR_BGR2YUV);
    vector<Mat> channels;
    split(yuv, channels);
    Mat y = channels[0];
    Mat yFloat;
    y.convertTo(yFloat, CV_32F);
    Mat LL, LH, HL, HH;
    dwtHaar(yFloat, LL, LH, HL, HH);
    string binary;
    int bitIndex = 0;
    for (int i = 10; i < LL.rows && bitIndex < watermarkLength * 8; ++i)
    {
        for (int j = 10; j < LL.cols && bitIndex < watermarkLength * 8; ++j)
        {
            float val = LL.at<float>(i, j);
            int bit = ((int)round(val)) % 2;
            binary += (bit == 1) ? '1' : '0';
            bitIndex++;
        }
    }
    return binaryToText(binary);
}

int main()
{
    string watermarkedImage = "watermarked.png";
    string originalWatermark = "DWT_SECRET";
    int watermarkLength = originalWatermark.length();
    Mat image = imread(watermarkedImage);
    string extracted = extractWatermark(image, watermarkLength);
    cout << "提取的水印内容：" << extracted << endl;
    return 0;
}
