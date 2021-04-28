#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// fragment shader uniform locations
enum uniform_bind {
    COLOR_DIFFUSE = 0,
    COLOR_AMBIENT = 1,
    COLOR_SPECULAR = 2,
    COLOR_EMISSIVE = 3,
    MATERIAL_TRANSPARENCY = 4,
    MATERIAL_REFRACTION = 5,
    MATERIAL_SPECULAR_EXP = 6,
    TEXTURE_DIFFUSE = GL_TEXTURE0,
};

struct material {

    std::string name;

    glm::vec3 color_diffuse{1.0f};
    glm::vec3 color_ambient{0.0f};
    glm::vec3 color_specular{0.0f};
    glm::vec3 color_emissive{0.0f};

    float transparency{0.0f}; // 0.0 -> 1.0 -- opaque -> transparent
    float refraction_index{1.0f};
    float specular_exponent{1.0f};
    unsigned illum_model;

    GLuint texture_diffuse_id = 0;

    material(const std::string &name) : name(name) {}

    void init_texture(const void *data, int width, int height, GLenum format, GLenum data_type) {
        glGenTextures(1, &texture_diffuse_id);
        glBindTexture(GL_TEXTURE_2D, texture_diffuse_id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, data_type, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    void init_texture(const uint8_t *data, int width, int height, GLenum format) {
        init_texture(data, width, height, format, GL_UNSIGNED_BYTE);
    }

    void init_texture(const float *data, int width, int height, GLenum format) {
        init_texture(data, width, height, format, GL_FLOAT);
    }

    /**
     * \brief Initialize the texture with a 1 pixel texture
     * \param color The color of the texture in RGBA
     */
    void init_texture(glm::vec4 color) { init_texture(glm::value_ptr(color), 1, 1, GL_RGBA); }

    void bind() const {
        glActiveTexture(TEXTURE_DIFFUSE);
        glBindTexture(GL_TEXTURE_2D, texture_diffuse_id);
        
        glUniform3fv(COLOR_DIFFUSE, 1, glm::value_ptr(color_diffuse));
        glUniform3fv(COLOR_AMBIENT, 1, glm::value_ptr(color_diffuse));
        glUniform3fv(COLOR_SPECULAR, 1, glm::value_ptr(color_diffuse));
        glUniform3fv(COLOR_EMISSIVE, 1, glm::value_ptr(color_diffuse));

        glUniform1f(MATERIAL_TRANSPARENCY, transparency);
        glUniform1f(MATERIAL_REFRACTION, refraction_index);
        glUniform1f(MATERIAL_SPECULAR_EXP, specular_exponent);
    }
};

/**
 * \brief Open and parse an .mtl file
 * Some materials may have uninitialized fields
 */
void load_mtl(const std::string &path, std::vector<material> &materials) {
    // std::filesystem is broken on mingw-w64, so this is a workaround
    std::string base_dir = path.substr(0, path.find_last_of("\\/") + 1);
    std::string filename = path.substr(path.find_last_of("\\/") + 1);

    std::ifstream ifile(path);
    if (!ifile.is_open()) {
        std::cout << "Failed to open file: " << path << std::endl;
        return;
    }

    std::string line;
    unsigned line_no = 0;
    while (std::getline(ifile, line)) {
        ++line_no;
        std::istringstream line_ss(line);
        std::string type;
        if (line_ss >> type) {
            if (type == "#") { // comment
                continue;
            } else if (type == "newmtl") { // new material
                std::string name;
                line_ss >> name;
                materials.push_back(material(name));
            } else if (type == "illum") { // illumination model
                line_ss >> materials.back().illum_model;
            } else if (type == "map_Kd") { // diffuse texture
                // TODO fix textures with transparency
                std::string texture_filename;
                line_ss >> texture_filename;
                int width, height, channels;
                stbi_set_flip_vertically_on_load(true);
                uint8_t *data = stbi_load((base_dir + texture_filename).c_str(), &width, &height, &channels, 0);
                if (!data)
                    std::cout << filename << "(" << line_no << ") failed to load texture: " << texture_filename << '\n';
                materials.back().init_texture(data, width, height, channels == 4 ? GL_RGBA : GL_RGB);
                stbi_image_free(data);
            } else if (type == "Ka") { // ambient color
                float f;
                for (glm::vec3::length_type i = 0; line_ss >> f; ++i)
                    materials.back().color_ambient[i] = f;
            } else if (type == "Kd") { // diffuse color
                float f;
                for (glm::vec3::length_type i = 0; line_ss >> f; ++i)
                    materials.back().color_diffuse[i] = f;
            } else if (type == "Ks") { // specular color
                float f;
                for (glm::vec3::length_type i = 0; line_ss >> f; ++i)
                    materials.back().color_specular[i] = f;
            } else if (type == "Ke") { // emissive color
                float f;
                for (glm::vec3::length_type i = 0; line_ss >> f; ++i)
                    materials.back().color_emissive[i] = f;
            } else if (type == "Ni") { // refraction index
                line_ss >> materials.back().refraction_index;
            } else if (type == "Ns") { // specular exponenet
                line_ss >> materials.back().specular_exponent;
            } else if (type == "Tr") { // transparency
                line_ss >> materials.back().transparency;
            } else if (type == "d") { // dissolve (1 - Tr)
                float dissolve;
                line_ss >> dissolve;
                materials.back().transparency = 1.0f - dissolve;
            } else {
                std::cout << filename << "(" << line_no << ") unknown type: " << type << '\n';
            }
        }
    }
    ifile.close();
}