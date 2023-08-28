#pragma once
#include "Sample.h"

class Traingle : public Sample
{
public:
    virtual void init();
    virtual void Update();
    virtual void UpdatePipeline();
};