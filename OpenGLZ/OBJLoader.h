#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <iostream>     // std::cout
#include <fstream>      // std::ifstream
#include <string>
#include <sstream>

bool loadOBJ(const char* path, std::vector < glm::vec3 >& out_vertices, std::vector < glm::vec2 >& out_uvs, std::vector < glm::vec3 >& out_normals) {
	std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;
	std::vector< glm::vec3 > temp_vertices;
	std::vector< glm::vec2 > temp_uvs;
	std::vector< glm::vec3 > temp_normals;

	std::ifstream ifs(path);
	if (!ifs.is_open())
		return false;

	while (ifs.good()) {
		std::string s;
		ifs >> s;

		if (s == "v") {
			glm::vec3 vertex;
			ifs >> vertex.x >> vertex.y >> vertex.z;
			temp_vertices.push_back(vertex);
		}
		else if (s == "vt") {
			glm::vec2 uv;
			ifs >> uv.x >> uv.y;
			temp_uvs.push_back(uv);
		}
		else if (s == "vn") {
			glm::vec3 normal;
			ifs >> normal.x >> normal.y >> normal.z;
			temp_normals.push_back(normal);
		}
		else if (s == "f") {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			ifs >> vertex1 >> vertex2 >> vertex3;

			std::string token;

			std::istringstream iss(vertex1);
			std::getline(iss, token, '/');
			vertexIndex[0] = std::stoul(token);
			std::getline(iss, token, '/');
			uvIndex[0] = std::stoul(token);
			std::getline(iss, token, '/');
			normalIndex[0] = std::stoul(token);

			iss = std::istringstream(vertex2);
			std::getline(iss, token, '/');
			vertexIndex[1] = std::stoul(token);
			std::getline(iss, token, '/');
			uvIndex[1] = std::stoul(token);
			std::getline(iss, token, '/');
			normalIndex[1] = std::stoul(token);

			iss = std::istringstream(vertex3);
			std::getline(iss, token, '/');
			vertexIndex[2] = std::stoul(token);
			std::getline(iss, token, '/');
			uvIndex[2] = std::stoul(token);
			std::getline(iss, token, '/');
			normalIndex[2] = std::stoul(token);

			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
	}

	for (unsigned int i = 0; i < vertexIndices.size(); i++) {
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		glm::vec2 uv = temp_uvs[uvIndex - 1];
		glm::vec3 normal = temp_vertices[normalIndex - 1];

		out_vertices.push_back(vertex);
		out_uvs.push_back(uv);
		out_normals.push_back(normal);
	}

	return true;
}