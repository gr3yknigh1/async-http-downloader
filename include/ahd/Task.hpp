#ifndef FILETASK_HPP_
#define FILETASK_HPP_

#include "ahd/Action.hpp"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

struct Task
{
    std::string file;
    std::vector<std::shared_ptr<Action>> actions;
    std::vector<std::string> dependencies;
};

using TaskMap = std::unordered_map<std::string, std::shared_ptr<Task>>;

#endif // FILETASK_HPP_
