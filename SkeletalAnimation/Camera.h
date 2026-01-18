#pragma once

#include "Core.h"

enum class ECameraMovement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

constexpr float YAW         = -90.0f;
constexpr float PITCH       =  0.0f;
constexpr float SPEED       =  2.5f;
constexpr float SENSITIVITY =  0.1f;
constexpr float ZOOM        =  45.0f;

class FCamera
{
public:
    FCamera(glm::vec3 InPosition = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 InUp = glm::vec3(0.0f, 1.0f, 0.0f), float InYaw = YAW, float InPitch = PITCH);

    glm::mat4 GetViewMatrix() const { return glm::lookAt(Position, Position + Front, Up); }

    void ProcessKeyboard(ECameraMovement Direction, float DeltaTime);
    void ProcessMouseMovement(float XOffset, float YOffset, GLboolean bConstrainPitch = true);
    void ProcessMouseScroll(float YOffset);

private:
    void UpdateCameraVectors();
    
public:
    float Yaw;
    float Pitch;
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
    
private:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
};
