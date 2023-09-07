#include "Scene.h"
#include "PSO.h"
#include "../DX.h"

void Scene::add(Mesh* mesh)
{
    meshs.push_back(mesh);
}

void Scene::render()
{
    reset();
    
    auto commandList = DX::dx->commandList;

    for(int i=0; i<meshs.size(); i++)
    {
        if(currPSO != meshs[i]->mat->pso)
        {
            currPSO = meshs[i]->mat->pso;
            commandList->SetPipelineState(meshs[i]->mat->pso->pipelineStateObject);
            commandList->SetGraphicsRootSignature(meshs[i]->mat->pso->rootSignature); 
        }

        //meshs[i]->draw();
    }
}

void Scene::reset()
{
    currPSO = nullptr;
}