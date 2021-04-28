#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>

#include "mesh.hh"
#include "material.hh"

struct object {
    glm::mat4 model_mat;
    std::vector<mesh> meshes;
    std::vector<material> materials;

    object(const std::string &path, glm::mat4 matrix) : model_mat(matrix) { load_obj(path); };

    void draw() const {
        for (const mesh &mesh : meshes)
            mesh.draw(materials);
    }

  private:
    void load_obj(const std::string &path) {

        // std::filesystem is broken on mingw-w64, so this is a workaround
        std::string base_dir = path.substr(0, path.find_last_of("\\/") + 1);
        std::string filename = path.substr(path.find_last_of("\\/") + 1);

        std::vector<glm::vec3> obj_vertices;
        std::vector<glm::vec3> obj_normals;
        std::vector<glm::vec2> obj_tex_coords;

        std::vector<vertex> mesh_vertices;
        unsigned current_material_index = -1;

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
                } else if (type == "mtllib") { // load mtl file
                    std::string mtl_path;
                    line_ss >> mtl_path;
                    load_mtl(base_dir + mtl_path, materials);
                } else if (type == "usemtl") { // use new material
                    if (!mesh_vertices.empty()) {
                        meshes.push_back(mesh(mesh_vertices, current_material_index));
                        mesh_vertices.clear();
                    }
                    std::string mat_name;
                    line_ss >> mat_name;
                    current_material_index = -1;
                    for (unsigned i = 0; i < materials.size(); i++) {
                        if (materials[i].name == mat_name) {
                            current_material_index = i;
                            break;
                        }
                    }
                } else if (type == "v") { // vertex
                    float f;
                    glm::vec3 vertex;
                    for (glm::vec3::length_type i = 0; line_ss >> f; ++i)
                        vertex[i] = f;
                    obj_vertices.push_back(vertex);
                } else if (type == "vn") { // vertex normal
                    float f;
                    glm::vec3 normal;
                    for (glm::vec3::length_type i = 0; line_ss >> f; ++i)
                        normal[i] = f;
                    obj_normals.push_back(normal);
                } else if (type == "vt") { // texture coordinate
                    float f;
                    glm::vec2 tex_coord;
                    for (glm::vec2::length_type i = 0; line_ss >> f; ++i)
                        tex_coord[i] = f;
                    obj_tex_coords.push_back(tex_coord);
                } else if (type == "f") { // face
                    std::string vertex_data;
                    signed vertex_no = 0;
                    while (line_ss >> vertex_data) {
                        ++vertex_no;
                        std::istringstream vertex_data_ss(vertex_data);
                        char c;
                        unsigned u;
                        vertex v;
                        if (vertex_data_ss >> u) {
                            v.position = obj_vertices.at(u - 1);
                            if (vertex_data_ss >> c && c == '/' && vertex_data_ss >> u)
                                v.tex_coord = obj_tex_coords.at(u - 1);
                            if (vertex_data_ss >> c && c == '/' && vertex_data_ss >> u)
                                v.normal = obj_normals.at(u - 1);
                            if (vertex_no > 3) { // split faces with more than one triangle
                                mesh_vertices.push_back(mesh_vertices.end()[1 - vertex_no]);
                                mesh_vertices.push_back(mesh_vertices.end()[-2]);
                            }
                            mesh_vertices.push_back(v);
                        } else {
                            std::cout << filename << "(" << line_no << ") bad face" << '\n';
                        }
                    }
                } else {
                    std::cout << filename << "(" << line_no << ") unknown obj type: " << type << '\n';
                }
            }
        }

        meshes.push_back(mesh(mesh_vertices, current_material_index));

        // clean up any uninitialized textures
        for (material &material : materials)
            if (material.texture_diffuse_id == 0)
                material.init_texture(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

        // put any transparent meshes at the back
        // this is a quick hack to get transparency working, if a transparent mesh 
        // is drawn behind another it will get clipped because of the depth buffer
        // TODO implement proper depth transparency https://learnopengl.com/Advanced-OpenGL/Blending
        std::sort(meshes.begin(), meshes.end(), [&](const mesh &a, const mesh &b) {
            return materials.at(a.material_index).transparency < materials.at(b.material_index).transparency;
        });

        ifile.close();
    }
};