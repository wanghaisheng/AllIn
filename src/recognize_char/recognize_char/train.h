#ifndef TRAIN_EXE_TRAIN_H_
#define TRAIN_EXE_TRAIN_H_

class Train {
public:
    void buildInput();

    void buildOutput();

    void train();

    void save(const std::string& file);
};

#endif // TRAIN_EXE_TRAIN_H_
