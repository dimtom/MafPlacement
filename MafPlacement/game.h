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

    int getSeat(int idx) const
    {
        return _seats[idx];
    }

    void changeSeat(int idx, int new_value)
    {
        _seats[idx] = new_value;
    }

private:
    int _id;
    std::vector<int> _seats;


};
