#include "world.h"

#include "resourcemanager.h"
#include "entitymanager.h"
#include "scene.h"


World::World()
{
    ResourceManager::instance()->addShader("plain",     std::make_shared<Shader>("plainshader"));
    ResourceManager::instance()->addShader("texture",   std::make_shared<Shader>("textureshader"));
    ResourceManager::instance()->addShader("phong",     std::make_shared<Shader>("phongshader"));

    ResourceManager::instance()->loadTexture("white",   "white.bmp");
    ResourceManager::instance()->loadTexture("hund",    "hund.bmp");
    ResourceManager::instance()->loadTexture("skybox",  "skybox.bmp");

    ResourceManager::instance()->addMesh("box", "box2.txt");
    ResourceManager::instance()->addMesh("monkey", "monkey.obj");

    if(auto mesh =  ResourceManager::instance()->getMesh("box"))
    {
        mesh->mMaterial.mShader = ResourceManager::instance()->getShader("plain");
    }

    if(auto mesh = ResourceManager::instance()->getMesh("monkey"))
    {
        mesh->mMaterial.mShader = ResourceManager::instance()->getShader("phong");
    }

    entityManager = new EntityManager();

    mCurrentScene = new Scene(this);

}

World::~World()
{
    delete entityManager;
}