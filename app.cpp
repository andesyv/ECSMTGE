#include "app.h"

#include <QDebug>

#include "scene.h"
#include "entitymanager.h"

#include "ui_mainwindow.h"

App::App()
{
    mMainWindow = std::make_unique<MainWindow>();
    mRenderer = mMainWindow->getRenderer();

    connect(mRenderer, &Renderer::initDone, this, &App::initTheRest);

    connect(mMainWindow->ui->actionToggle_wireframe, &QAction::triggered, mRenderer, &Renderer::toggleWireframe);
    connect(mRenderer, &Renderer::escapeKeyPressed, mMainWindow.get(), &MainWindow::close);
    connect(mMainWindow->ui->actionExit, &QAction::triggered, mMainWindow.get(), &MainWindow::close);

}

// Slot called from Renderer when its done with initialization
void App::initTheRest()
{
    mWorld = std::make_unique<World>();

    mMainWindow->setEntityManager(mWorld->getEntityManager());

    mWorld->initCurrentScene();

    mRenderer->setupCamera();

    connect(mRenderer, &Renderer::escapeKeyPressed, this, &App::quit);
    connect(&mUpdateTimer, &QTimer::timeout, this, &App::update);


    mUpdateTimer.start(16); // Simulates 60ish fps

    mDeltaTimer.start();
    mFPSTimer.start();
}


void App::update()
{
    // Not exactly needed now, but maybe this should be here? Timer does call this function every 16 ms.
    if(currentlyUpdating)
        return;
    currentlyUpdating = true;


    mDeltaTime = mDeltaTimer.restart();


    calculateFrames();


    mRenderer->handleInput(mDeltaTime);

    auto& transforms = mWorld->getEntityManager()->getTransforms();
    auto renders = mWorld->getEntityManager()->getRenders();
    auto& cameras = mWorld->getEntityManager()->getCameras();

    CameraSystem::updateCameras(transforms, cameras);

    mRenderer->render(renders, transforms, mDeltaTime);

    currentlyUpdating = false;
}

void App::quit()
{
    mMainWindow->close();
}

void App::updatePerspective()
{
    auto& cameras = mWorld->getEntityManager()->getCameras();

    CameraSystem::updateCameras(cameras, gsl::mat4::persp(FOV, static_cast<float>(mRenderer->width()) / mRenderer->height(), 1.f, 100.f));
}

void App::calculateFrames()
{
    ++mFrameCounter;
    mTotalDeltaTime += mDeltaTime;
    double elapsed = mFPSTimer.elapsed();
    if(elapsed >= 100)
    {
        mMainWindow->showFPS(mTotalDeltaTime / mFrameCounter, mFrameCounter / mTotalDeltaTime * 1000);
        mFrameCounter = 0;
        mTotalDeltaTime = 0;
        mFPSTimer.restart();
    }
}
