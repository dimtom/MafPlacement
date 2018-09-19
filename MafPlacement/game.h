#pragma once
#include <vector>

class Game
{
public:
    static const int NumSeats = 10;
public:
    Game(int id)
        : _id(id)
        , _seats(NumSeats)
    {}
    
    ~Game() = default;

public:
    std::vector<int>& seats()
    {
        return _seats;
    }

    const std::vector<int>& seats() const
    {
        return _seats;
    }

private:
    int _id;
    std::vector<int> _seats;


};
