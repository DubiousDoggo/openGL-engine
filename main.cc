#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <cmath>
#include <algorithm>

#include "shaders.h"
#include "object.hh"

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

int window_width = 800;
int window_height = 600;

float camera_fov = glm::radians(45.0f);             // vertical fov in radians
glm::vec3 camera_pos = glm::vec3(0.0f, 0.0f, 3.0f); // camera position in world coordinates
float camera_yaw = 0;
float camera_pitch = 0;

int main() {

    std::string model_name;
    std::string default_model = "./models/peach_castle/peach_castle.obj";
    std::cout << "Enter model path (default " << default_model <<" ): " << std::flush;
    std::getline(std::cin, model_name);
    if(model_name.empty())
        model_name = default_model;

    // initialize and configure glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw: create window
    GLFWwindow *window = glfwCreateWindow(window_width, window_height, "OpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return EXIT_FAILURE;
    }

    GLint success;
    GLchar info[512];

    // load and compile vertex shader
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader_id, 1, &vertex_glsl, NULL);
    glCompileShader(vertex_shader_id);
    glGetShaderiv(vertex_shader_id, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar info[512];
        glGetShaderInfoLog(vertex_shader_id, sizeof(info), NULL, info);
        std::cout << "Failed to compile vertex shader: " << info << std::endl;
        return EXIT_FAILURE;
    }

    // load and compile fragment shader
    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader_id, 1, &fragment_glsl, NULL);
    glCompileShader(fragment_shader_id);
    glGetShaderiv(fragment_shader_id, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment_shader_id, sizeof(info), NULL, info);
        std::cout << "Failed to compile fragment shader: " << info << std::endl;
        return EXIT_FAILURE;
    }

    // create and link shader program
    GLuint shader_program_id = glCreateProgram();
    glAttachShader(shader_program_id, vertex_shader_id);
    glAttachShader(shader_program_id, fragment_shader_id);
    glLinkProgram(shader_program_id);
    glGetProgramiv(shader_program_id, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader_program_id, sizeof(info), NULL, info);
        std::cout << "Failed to link shader program: " << info << std::endl;
        return EXIT_FAILURE;
    }
    glDeleteShader(fragment_shader_id);
    glDeleteShader(vertex_shader_id);

    object object(model_name, glm::scale(glm::mat4(1.0f), glm::vec3(1.0f)));
    camera_pos = glm::vec3(0.0f, 2.0f, 10.0f);

    // tell the shader which texture unit each sampler belongs to (only has to be done once)
    glUseProgram(shader_program_id);
    glUniform1i(glGetUniformLocation(shader_program_id, "texture_diffuse"), TEXTURE_DIFFUSE);

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float last_frame = glfwGetTime();
    float delta_time = 0;
    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        float current_frame = glfwGetTime();
        delta_time = current_frame - last_frame;
        last_frame = current_frame;

        // spin object
        // object.model_mat = glm::rotate(glm::mat4(1.0f), glm::radians(delta_time * 90.0f), glm::vec3(0.0f, 1.0f,
        // 0.0f)) *
        //                    object.model_mat;

        float camera_move_speed = delta_time * 2.5f;
        float camera_turn_speed = glm::radians(delta_time * 90.0f);
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            camera_move_speed *= 2;

        // TODO mouse look
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            camera_yaw += camera_turn_speed;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            camera_yaw -= camera_turn_speed;
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            camera_pitch += camera_turn_speed;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            camera_pitch -= camera_turn_speed;
        camera_pitch = std::clamp(camera_pitch, glm::radians(-89.9f), glm::radians(89.9f));

        glm::vec3 world_axis_y(0.0f, 1.0f, 0.0f);
        glm::vec3 camera_axis_z(std::sin(camera_yaw) * std::cos(camera_pitch), -std::sin(camera_pitch),
                                std::cos(camera_yaw) * std::cos(camera_pitch));
        glm::vec3 camera_axis_x = glm::normalize(glm::cross(world_axis_y, camera_axis_z));
        // glm::vec3 camera_axis_y = glm::normalize(glm::cross(camera_axis_z, camera_axis_x));

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera_pos -= camera_move_speed * camera_axis_z;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera_pos -= camera_move_speed * camera_axis_x;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera_pos += camera_move_speed * camera_axis_z;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera_pos += camera_move_speed * camera_axis_x;
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            camera_pos += camera_move_speed * world_axis_y;
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
            camera_pos -= camera_move_speed * world_axis_y;

        glm::mat4 view_mat = glm::lookAt(camera_pos, camera_pos - camera_axis_z, world_axis_y);

        glClearColor(0.357f, 0.737f, 0.894f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shader_program_id);
        const glm::mat4 &model_mat = object.model_mat;

        glm::mat4 projection_mat =
            glm::perspective(camera_fov, (float)window_width / (float)window_height, 0.1f, 100.0f);
        GLuint model_uniform = glGetUniformLocation(shader_program_id, "model");
        GLuint view_uniform = glGetUniformLocation(shader_program_id, "view");
        GLuint projection_uniform = glGetUniformLocation(shader_program_id, "projection");
        glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model_mat));
        glUniformMatrix4fv(view_uniform, 1, GL_FALSE, glm::value_ptr(view_mat));
        glUniformMatrix4fv(projection_uniform, 1, GL_FALSE, glm::value_ptr(projection_mat));

        object.draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// glfw: called whenever the window size changed (by OS or user resize)
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions
    // might not match true width and height depending on OS display scaling
    window_width = width;
    window_height = height;
    glViewport(0, 0, width, height);
}