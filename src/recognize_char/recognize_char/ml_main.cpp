//The example of using BPNetwork in OpenCV
//Coded by L. Wei
#include <opencv2/opencv.hpp>
#include <opencv2/ml/ml.hpp>
#include <windows.h>
#include <vector>
#include <iostream>
#include <string>

using namespace std;
using namespace cv;

void Normalize(Mat& img, int width, int height) {
    cv::resize(img, img, Size(width, height));
}

Mat& CvtVector2Mat(const std::vector<float>& vec, Mat& dst) {
    // check that the rows and cols match the size of your vector
    if (vec.size() == dst.rows * dst.cols) {
        memcpy(dst.data, vec.data(), vec.size()*sizeof(float));
    }

    return dst;
}

// 梯度分布特征, 该特征计算图像水平方向和竖直方向的梯度图像
// 然后通过给梯度图像分划不同的区域, 进行梯度图像每个区域亮度值的统计.
void calcGradientFeat(const Mat& imgSrc, vector<float>& feat) {
    float sumMatValue(const Mat& image);

    Mat image;
    resize(imgSrc, image, Size(8, 16));

    // 计算x方向和y方向上的滤波 
    float mask[3][3] = { { 1, 2, 1 }, { 0, 0, 0 }, { -1, -2, -1 } };
    Mat y_mask = Mat(3, 3, CV_32F, mask) / 8;
    Mat x_mask = y_mask.t(); // 转置 
    Mat sobelX, sobelY;

    filter2D(image, sobelX, CV_32F, x_mask);
    filter2D(image, sobelY, CV_32F, y_mask);
    sobelX = abs(sobelX);
    sobelY = abs(sobelY);

    float totleValueX = sumMatValue(sobelX);
    float totleValueY = sumMatValue(sobelY);
    
    // 将图像划分为4*2共8个格子，计算每个格子里灰度值总和的百分比 
    for (int i = 0; i < image.rows; i = i + 4) {
        for (int j = 0; j < image.cols; j = j + 4) {
        Mat subImageX = sobelX(Rect(j, i, 4, 4));
        feat.push_back(sumMatValue(subImageX) / totleValueX);
        Mat subImageY = sobelY(Rect(j, i, 4, 4));
        feat.push_back(sumMatValue(subImageY) / totleValueY);
        }
    }
}

// 计算图像中像素灰度值总和 
float sumMatValue(const Mat& image) {
    float sumValue = 0;
    int r = image.rows;
    int c = image.cols;
    if (image.isContinuous()) {
        c = r*c;
        r = 1;
    }

    for (int i = 0; i < r; i++) {
        const uchar* linePtr = image.ptr<uchar>(i);
        for (int j = 0; j < c; j++) {
            sumValue += linePtr[j];
        }
    }

    return sumValue;
}

typedef Mat FeatureMat;

void Extract(const std::string& file, FeatureMat& one_dimension)
{
    cv::Mat img_src = imread(file.c_str());
    Mat img;
    cvtColor(img_src, img, CV_BGR2GRAY);
    vector<float> feat;
    calcGradientFeat(img, feat);

    Mat out_mat(1, 48, CV_32FC1);
    cout << img.channels() << endl;

/*    memset(out_mat.data, 0, 48 * sizeof(float));*/
    memcpy(out_mat.data, feat.data(), feat.size() * sizeof(float));

    int k = 16;
    Normalize(img, 8, 4); // 8 * 4
    cout << img << endl << endl;
    Mat img_fc1(8, 4, CV_32FC1);
    img.convertTo(img_fc1, CV_32FC1);
    cout << img_fc1 << endl << endl;

    cout << img.channels() << endl;

    for (int i = 0; i < img.rows; ++i) {
//         const float* Mi = img.ptr<float>(i);
//         for (int j = 0; j < img.cols; j++) {
//             cout << Mi[j];
//             out_mat.ptr<float>(0)[k] = Mi[j];
//         }
        uchar* p = img.ptr(i);
        for (int j = 0; j < img.cols; j++) {
            std::cout << *(p + 3) << endl;
            ++p;
        }
    }

    std::cout << "Mat: \n" << out_mat << endl;
    cout << out_mat.rows * out_mat.cols << endl;
}

int main()
{
    cv::Mat img_src = imread("C:\\pj\\bin\\w32d\\11.jpeg");
    FeatureMat mat;
    Extract("C:\\pj\\bin\\w32d\\11.jpeg", mat);

//     cout << img_src.rows << ", " << img_src.cols << endl;
//     vector<float> feat;
//     calcGradientFeat(img_src, feat);
//     cout << "dimension : " << feat.size() << endl;
//     for (int i = 0; i < feat.size(); ++i)
//         cout << feat.at(i) << endl;

    return 0;

    //Setup the BPNetwork
    CvANN_MLP bp;
    // Set up BPNetwork's parameters
    CvANN_MLP_TrainParams params;
    params.train_method = CvANN_MLP_TrainParams::BACKPROP;
    params.bp_dw_scale = 0.1;
    params.bp_moment_scale = 0.1;
    //params.train_method=CvANN_MLP_TrainParams::RPROP;
    //params.rp_dw0 = 0.1; 
    //params.rp_dw_plus = 1.2; 
    //params.rp_dw_minus = 0.5;
    //params.rp_dw_min = FLT_EPSILON; 
    //params.rp_dw_max = 50.;

    // Set up training data
    float labels[3][5] = { { 0, 0, 0, 0, 0 }, { 1, 1, 1, 1, 1 }, { 0, 0, 0, 0, 0 } };
    Mat labelsMat(3, 5, CV_32FC1, labels);

    float trainingData[3][5] = { { 1, 2, 3, 4, 5 }, { 111, 112, 113, 114, 115 }, { 21, 22, 23, 24, 25 } };
    Mat trainingDataMat(3, 5, CV_32FC1, trainingData);
    Mat layerSizes = (Mat_<int>(1, 5) << 5, 2, 2, 2, 5);
    bp.create(layerSizes, CvANN_MLP::SIGMOID_SYM);//CvANN_MLP::SIGMOID_SYM
    //CvANN_MLP::GAUSSIAN
    //CvANN_MLP::IDENTITY
    cout << "begin to train...\n";
    int begin = GetTickCount();
    bp.train(trainingDataMat, labelsMat, Mat(), Mat(), params);
    cout << "train done, elapsed time: " << GetTickCount() - begin << std::endl;

    // Data for visual representation
    int width = 512, height = 512;
    Mat image = Mat::zeros(height, width, CV_8UC3);
    Vec3b green(0, 255, 0), blue(255, 0, 0);
    // Show the decision regions given by the SVM
    cout << "draw image...";
    for (int i = 0; i < image.rows; ++i)
    for (int j = 0; j < image.cols; ++j)
    {
        Mat sampleMat = (Mat_<float>(1, 5) << i, j, 0, 0, 0);
        Mat responseMat;
        bp.predict(sampleMat, responseMat);

        float* p = responseMat.ptr<float>(0);
        float response = 0.0f;
        for (int k = 0; k<5; k++){
            //	cout<<p[k]<<" ";
            response += p[k];
        }
        if (response >2)
            image.at<Vec3b>(j, i) = green;
        else
            image.at<Vec3b>(j, i) = blue;
    }
    cout << "draw done.\n";

    // Show the training data
    int thickness = -1;
    int lineType = 8;
    circle(image, Point(501, 10), 5, Scalar(0, 0, 0), thickness, lineType);
    circle(image, Point(255, 10), 5, Scalar(255, 255, 255), thickness, lineType);
    circle(image, Point(501, 255), 5, Scalar(255, 255, 255), thickness, lineType);
    circle(image, Point(10, 501), 5, Scalar(255, 255, 255), thickness, lineType);

    imwrite("result.png", image);        // save the image 

    imshow("BP Simple Example", image); // show it to the user
    waitKey(0);

}
