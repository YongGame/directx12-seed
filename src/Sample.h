#pragma once

class Sample
{
public:
    virtual void init() = 0;
    virtual void Update() = 0;
    virtual void UpdatePipeline() = 0;
};