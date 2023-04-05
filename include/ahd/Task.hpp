#ifndef FILETASK_HPP_
#define FILETASK_HPP_

#include "ahd/Action.hpp"
#include <string>
#include <vector>
#include <memory>

struct Task
{
    std::string file;
    std::vector<std::shared_ptr<Action>> actions;
    std::vector<std::string> dependencies;
};

#endif // FILETASK_HPP_
