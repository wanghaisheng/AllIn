#ifndef TRAIN_EXE_FILE_H_
#define TRAIN_EXE_FILE_H_

#include <map>
#include <string>
#include <vector>

class Files {
public:
    static Files* getInst();

    bool load(const std::string& root_path);
    
    std::vector<std::string>& getFileList(const std::string& key) const;

private:
    Files() {

    }

    static Files* files_inst_;

private:
    std::string root_;

    std::map<std::string, std::vector<std::string>> char_paths_map_;
};

#endif // end TRAIN_EXE_FILE_H_
