#pragma once
namespace dae { class GameObject; }

class Observer
{
public:
    virtual ~Observer() = default;
    virtual void OnNotify(int event, dae::GameObject* owner) = 0;
};