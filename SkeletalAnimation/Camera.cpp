#include "Camera.h"

FCamera::FCamera(glm::vec3 InPosition, glm::vec3 InUp, float InYaw, float InPitch): Front(glm::vec3(0.0f, 0.0f, -1.0f))
    , MovementSpeed(SPEED)
    , MouseSensitivity(SENSITIVITY)
    , Zoom(ZOOM)
    , Position(InPosition)
    , WorldUp(InUp)
    , Yaw(InYaw)
    , Pitch(InPitch)
{
    Position = InPosition;
    WorldUp = InUp;
    Yaw = InYaw;
    Pitch = InPitch;
    UpdateCameraVectors();
}

void FCamera::ProcessKeyboard(ECameraMovement Direction, float DeltaTime)
{
    float Velocity = MovementSpeed * DeltaTime;
    if (Direction == ECameraMovement::FORWARD)
    {
        Position += Front * Velocity;
    }
    if (Direction == ECameraMovement::BACKWARD)
    {
        Position -= Front * Velocity;
    }
    if (Direction == ECameraMovement::LEFT)
    {
        Position -= Right * Velocity;
    }
    if (Direction == ECameraMovement::RIGHT)
    {
        Position += Right * Velocity;
    }
}

void FCamera::ProcessMouseMovement(float XOffset, float YOffset, GLboolean bConstrainPitch)
{
    XOffset *= MouseSensitivity;
    YOffset *= MouseSensitivity;

    Yaw   += XOffset;
    Pitch += YOffset;

    if (bConstrainPitch)
    {
        Pitch = std::clamp(Pitch, -89.0f, 89.0f);
    }

    UpdateCameraVectors();
}

void FCamera::ProcessMouseScroll(float YOffset)
{
    Zoom = std::clamp(Zoom - YOffset, 1.0f, 45.0f);
}

void FCamera::UpdateCameraVectors()
{
    glm::vec3 NewFront;
    NewFront.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    NewFront.y = sin(glm::radians(Pitch));
    NewFront.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(NewFront);
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up    = glm::normalize(glm::cross(Right, Front));
}
