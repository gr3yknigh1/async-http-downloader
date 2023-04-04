#ifndef ACTION_HPP_
#define ACTION_HPP_

class Action
{
public:
    virtual ~Action(void) {}
    virtual void Execute(void) const = 0;
};

#endif // ACTION_HPP_
