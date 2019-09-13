#include "app.h"

#include <QDebug>

#include "inputhandler.h"

#include "scene.h"
#include "entitymanager.h"

#include "inputsystem.h"
#include "physicssystem.h"

#include "ui_mainwindow.h"

App::App()
{
    mMainWindow = std::make_unique<MainWindow>();
    mRenderer = mMainWindow->getRenderer();

    mEventHandler = std::make_shared<InputHandler>(mRenderer);

    mRenderer->installEventFilter(mEventHandler.get());

    connect(mRenderer, &Renderer::initDone, this, &App::initTheRest);

    connect(mMainWindow->ui->actionToggle_wireframe, &QAction::triggered, mRenderer, &Renderer::toggleWireframe);
    connect(mEventHandler.get(), &InputHandler::escapeKeyPressed, mMainWindow.get(), &MainWindow::close);
    connect(mMainWindow->ui->actionExit, &QAction::triggered, mMainWindow.get(), &MainWindow::close);
    connect(mRenderer, &Renderer::windowUpdated, this, &App::updatePerspective);
}

// Slot called from Renderer when its done with initialization
void App::initTheRest()
{
    mWorld = std::make_unique<World>();

    mMainWindow->setEntityManager(mWorld->getEntityManager());

    mWorld->initCurrentScene();

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


    // Time since last frame in seconds
    mDeltaTime = mDeltaTimer.restart() / 1000.f;


    calculateFrames();

    // Input:
    const auto& inputs = mWorld->getEntityManager()->getInputComponents();
    auto& transforms = mWorld->getEntityManager()->getTransforms();

    mEventHandler->updateMouse();
    InputSystem::HandleInput(mDeltaTime, inputs, transforms);

    // Physics:
    /* Note: Physics calculation should be happening on a separate thread
     * and instead of sending references to the lists we should take copies
     * and then later apply those copies to the original lists.
     */
    auto& physics = mWorld->getEntityManager()->getPhysicsComponents();

    PhysicsSystem::UpdatePhysics(transforms, physics, mDeltaTime);

    // Rendering:
    const auto& renders = mWorld->getEntityManager()->getMeshComponents();
    auto& cameras = mWorld->getEntityManager()->getCameraComponents();

    auto usedTrans = CameraSystem::updateCameras(transforms, cameras);
    // Set all used transforms's "updated" to false so that updateCameras
    // won't need to calculate more viewmatrixes than it needs to.
    for (auto index : usedTrans)
        transforms[index].updated = false;

    for (const auto& camera : cameras) {
        mRenderer->render(renders, transforms, camera);
    }

    currentlyUpdating = false;
}

void App::quit()
{
    mMainWindow->close();
}

void App::updatePerspective()
{
    auto& cameras = mWorld->getEntityManager()->getCameraComponents();

    CameraSystem::updateCameras(cameras, gsl::mat4::persp(FOV, static_cast<float>(mRenderer->width()) / mRenderer->height(), 1.f, 100.f));
}

void App::calculateFrames()
{
    ++mFrameCounter;
    mTotalDeltaTime += mDeltaTime;
    double elapsed = mFPSTimer.elapsed();
    if(elapsed >= 100)
    {
        mMainWindow->showFPS(mTotalDeltaTime / mFrameCounter, mFrameCounter / mTotalDeltaTime);
        mFrameCounter = 0;
        mTotalDeltaTime = 0;
        mFPSTimer.restart();
    }
}
