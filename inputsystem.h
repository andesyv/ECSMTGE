#ifndef INPUTSYSTEM_H
#define INPUTSYSTEM_H

#include <QObject>

#include "inputhandler.h"
#include "componentdata.h"

/**
 * @brief Handles Editor Camera input.
 */
class InputSystem : public QObject
{
    Q_OBJECT

public:
    InputSystem();

    static void HandleEditorCameraInput(float deltaTime, TransformComponent &transformComponent, CameraComponent &cameraComponent);
};

#endif // INPUTSYSTEM_H
