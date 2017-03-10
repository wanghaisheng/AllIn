#include <opencv2/opencv.hpp>
#include <opencv2/ml/ml.hpp>
#include "files.h"
#include "feature_extract.h"
#include "train.h"
#include "train_mgr.h"

using namespace cv;

void TrainMgr::start()
{
    const std::string root_path = "Z:\\нд╦Е\\AllIn\\src\\recognize_char\\charSamples\\";
    Files::getInst()->load(root_path);

    const unsigned int TOTAL_CHARS = 34;
    std::string char_arr[TOTAL_CHARS] = 
    { 
        "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
        "A", "B", "C", "D", "E", "F", "G", "H", "J", "K",
        "L", "M", "N", "P", "Q", "R", "S", "T", "U", "V",
        "W", "X", "Y", "Z"
    };

    float trainingData[3][5] = { { 1, 2, 3, 4, 5 }, { 111, 112, 113, 114, 115 }, { 21, 22, 23, 24, 25 } };
    Mat trainingDataMat(3, 5, CV_32FC1, trainingData);
    for (unsigned int i = 0; i < TOTAL_CHARS; ++i) {
        std::vector<std::string> char_files = Files::getInst()->getFileList(char_arr[i]);
        std::vector<float> char_feature;
        FeatureExtract::getInst()->buildFeatureVector(char_files, char_feature);


    }

    Train train_chars;

    Mat labelsMat(3, 5, CV_32FC1, labels);

    train_chars.train(trainingDataMat, labelsMat);
    train_chars.save("ml_char.xml");
}
