#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <iostream>

#include "material.hh"

struct vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 tex_coord;
};

struct mesh {

    unsigned num_vertex;
    unsigned material_index;
    GLuint VAO_id, VBO_id;
    
    mesh(const std::vector<vertex> &vertices, unsigned material)
        : num_vertex(vertices.size()), material_index(material) {

        glGenVertexArrays(1, &VAO_id);
        glGenBuffers(1, &VBO_id);

        glBindVertexArray(VAO_id);

        glBindBuffer(GL_ARRAY_BUFFER, VBO_id);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

        // position attribute
        glVertexAttribPointer(0, decltype(vertex::position)::length(), GL_FLOAT, GL_FALSE, sizeof(vertex),
                              reinterpret_cast<void *>(offsetof(vertex, vertex::position)));
        glEnableVertexAttribArray(0);
        // normal attribute
        glVertexAttribPointer(1, decltype(vertex::normal)::length(), GL_FLOAT, GL_FALSE, sizeof(vertex),
                              reinterpret_cast<void *>(offsetof(vertex, vertex::normal)));
        glEnableVertexAttribArray(1);
        // texture coordinate attribute
        glVertexAttribPointer(2, decltype(vertex::tex_coord)::length(), GL_FLOAT, GL_FALSE, sizeof(vertex),
                              reinterpret_cast<void *>(offsetof(vertex, vertex::tex_coord)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
    }

    void draw(const std::vector<material> &materials) const {
        if(materials.size() <= material_index)
            std::cout << "material index out of range" << std::endl;
        materials.at(material_index).bind();
        glBindVertexArray(VAO_id);
        glDrawArrays(GL_TRIANGLES, 0, num_vertex);
    }
};
