#include <fstream>
#include <cstddef>

#include "Core.h"
#include "Camera.h"
#include "Model.h"
#include "Assimp.h"
#include "Utility.h"
#include "Animation.h"

// ------------------------------------------------------------------------------------------------
// Global Variables
// ------------------------------------------------------------------------------------------------
const uint32_t ScreenWidth = 800;
const uint32_t ScreenHeight = 600;

FCamera Camera(glm::vec3(0.0f, 0.5f, 3.0f));
float LastX = ScreenWidth / 2.0f;
float LastY = ScreenHeight / 2.0f;
bool bFirstMouse = true;

float DeltaTime = 0.0f;
float LastFrame = 0.0f;

// ------------------------------------------------------------------------------------------------
// Input Callbacks
// ------------------------------------------------------------------------------------------------
void ProcessInput(GLFWwindow *Window)
{
    if (glfwGetKey(Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(Window, true);
    }

    if (glfwGetKey(Window, GLFW_KEY_W) == GLFW_PRESS)
    {
        Camera.ProcessKeyboard(ECameraMovement::FORWARD, DeltaTime);
    }
    if (glfwGetKey(Window, GLFW_KEY_S) == GLFW_PRESS)
    {
        Camera.ProcessKeyboard(ECameraMovement::BACKWARD, DeltaTime);
    }
    if (glfwGetKey(Window, GLFW_KEY_A) == GLFW_PRESS)
    {
        Camera.ProcessKeyboard(ECameraMovement::LEFT, DeltaTime);
    }
    if (glfwGetKey(Window, GLFW_KEY_D) == GLFW_PRESS)
    {
        Camera.ProcessKeyboard(ECameraMovement::RIGHT, DeltaTime);
    }
}

void MouseCallback(GLFWwindow* Window, double XPosIn, double YPosIn)
{
    float XPos = static_cast<float>(XPosIn);
    float YPos = static_cast<float>(YPosIn);

    if (bFirstMouse)
    {
        LastX = XPos;
        LastY = YPos;
        bFirstMouse = false;
    }

    float XOffset = XPos - LastX;
    float YOffset = LastY - YPos; // reversed since y-coordinates go from bottom to top

    LastX = XPos;
    LastY = YPos;

    Camera.ProcessMouseMovement(XOffset, YOffset);
}

void ScrollCallback(GLFWwindow* Window, double XOffset, double YOffset)
{
    Camera.ProcessMouseScroll(static_cast<float>(YOffset));
}

// ------------------------------------------------------------------------------------------------
// Main
// ------------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    // 1. Initialize GLFW
    if (!glfwInit())
    {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* Window = glfwCreateWindow(ScreenWidth, ScreenHeight, "Skeletal Animation Assimp", NULL, NULL);
    if (Window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(Window);
    
    // Mouse & Input Setup
    glfwSetCursorPosCallback(Window, MouseCallback);
    glfwSetScrollCallback(Window, ScrollCallback);
    glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // 2. Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // 3. Compile Shaders
    std::string VertexShaderCode = FUtility::ReadFile("Shaders/shader.vert");
    std::string FragmentShaderCode = FUtility::ReadFile("Shaders/shader.frag");

    if (VertexShaderCode.empty() || FragmentShaderCode.empty())
    {
        return -1;
    }

    const char* VertexShaderSource = VertexShaderCode.c_str();
    const char* FragmentShaderSource = FragmentShaderCode.c_str();

    GLuint VertexShader, FragmentShader;
    VertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(VertexShader, 1, &VertexShaderSource, NULL);
    glCompileShader(VertexShader);
    FUtility::CheckCompileErrors(VertexShader, "VERTEX");

    FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(FragmentShader, 1, &FragmentShaderSource, NULL);
    glCompileShader(FragmentShader);
    FUtility::CheckCompileErrors(FragmentShader, "FRAGMENT");

    GLuint ShaderProgram = glCreateProgram();
    glAttachShader(ShaderProgram, VertexShader);
    glAttachShader(ShaderProgram, FragmentShader);
    glLinkProgram(ShaderProgram);
    FUtility::CheckCompileErrors(ShaderProgram, "PROGRAM");

    glDeleteShader(VertexShader);
    glDeleteShader(FragmentShader);

    // 4. Load Model
    // Try to load from asset folder
    std::string ModelPath = "C:/Users/roxma/Desktop/Rendering/asset/Taunt.dae";
    FModel OurModel(ModelPath);

    // 5. Load Animation
    FAnimation Animation(ModelPath, &OurModel);
    FAnimator Animator(&Animation);

    glEnable(GL_DEPTH_TEST);

    // 6. Render Loop
    while (!glfwWindowShouldClose(Window))
    {
        // Per-frame time logic
        float CurrentFrame = static_cast<float>(glfwGetTime());
        DeltaTime = CurrentFrame - LastFrame;
        LastFrame = CurrentFrame;

        // Input
        ProcessInput(Window);

        // Update Animation
        Animator.UpdateAnimation(DeltaTime);

        // Render
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Dark background
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(ShaderProgram);

        // Camera Matrices
        glm::mat4 Projection = glm::perspective(glm::radians(Camera.Zoom), (float)ScreenWidth / (float)ScreenHeight, 0.1f, 100.0f);
        glm::mat4 View = Camera.GetViewMatrix();
        
        // Model Matrix
        glm::mat4 Model = glm::mat4(1.0f);
        Model = glm::translate(Model, glm::vec3(0.0f, -0.5f, 0.0f));
        Model = glm::scale(Model, glm::vec3(1.0f, 1.0, 1.0f));

        glUniformMatrix4fv(glGetUniformLocation(ShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(Projection));
        glUniformMatrix4fv(glGetUniformLocation(ShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(View));
        glUniformMatrix4fv(glGetUniformLocation(ShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(Model));

        // Get Bone Transforms from Animator
        std::vector<glm::mat4> Transforms = Animator.GetFinalBoneMatrices();
        for (int32_t i = 0; i < Transforms.size(); ++i)
        {
             std::string Name = "gBones[" + std::to_string(i) + "]";
             glUniformMatrix4fv(glGetUniformLocation(ShaderProgram, Name.c_str()), 1, GL_FALSE, glm::value_ptr(Transforms[i]));
        }

        OurModel.Draw(ShaderProgram);

        glfwSwapBuffers(Window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteProgram(ShaderProgram);

    glfwTerminate();
    return 0;
}
