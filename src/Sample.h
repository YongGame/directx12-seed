#pragma once

class DX;

class Sample
{
public:
    DX* dx;
    virtual void init() = 0;
    virtual void Update() = 0;
    virtual void UpdatePipeline() = 0;
    virtual void resize() = 0;
};