#ifndef TRAIN_EXE_FEATURE_EXTRACT_H_
#define TRAIN_EXE_FEATURE_EXTRACT_H_

#include <vector>
#include <string>

class FeatureExtract {
public:
    static FeatureExtract* getInst();

    bool buildFeatureVector(
        const std::vector<std::string>& char_files,
        std::vector<float>& char_feature);

private:
    FeatureExtract();

private:
    std::vector<std::string> file_list_; // file path vecotr for one character 
};

#endif // TRAIN_EXE_FEATURE_EXTRACT_H_
