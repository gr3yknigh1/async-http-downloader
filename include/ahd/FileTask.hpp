#ifndef FILETASK_HPP_
#define FILETASK_HPP_

#include "ahd/Action.hpp"
#include <string>
#include <vector>

struct FileTask
{
    std::string file;
    std::vector<Action *> actions;
    std::vector<std::string> dependencies;
};

#endif // FILETASK_HPP_
