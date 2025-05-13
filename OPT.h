#ifndef OPT_H
#define OPT_H

#include <map>
#include <vector>
#include <algorithm>
#include <string>


std::string get_username();
std::string get_downloads_path();

class OPT
{
private:
    std::vector<std::string> argsVec;
    std::map<std::string, std::vector<std::string>> paramsMap;
    int num_args = 0;
    char **clParams;

public:
    OPT(int argc, char **argv);
    void add_argument(const std::string &arg);
    void parse_args();
    
    template<class K>
    bool contains(const std::vector<K> &vec, const K &target)
    {   
        return std::find(vec.begin(), vec.end(), target) != vec.end();
    }

    std::vector<std::string> operator[](const std::string &key);
};

#endif // OPT_H