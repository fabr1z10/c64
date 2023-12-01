#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <GL/glew.h>
#include "glm/glm.hpp"
#include "c64.h"


class Node;

struct Vertex {
    glm::vec2 pos;
    float color;
};


class Shader {
public:
    Shader(const std::string& vertexCode, const std::string& fragmentCode);
    ~Shader();
    virtual void draw() = 0;
    virtual void init() = 0;
    void setupVertices();
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setMat3(const std::string &name, const glm::mat3 &mat) const;
    void setMat4(const std::string &name, const glm::mat4 &mat) const;
    void setVec3(const std::string &name, const glm::vec3 &value) const;
    void setVec4(const std::string &name, const glm::vec4 &value) const;
    [[nodiscard]] GLuint getProgId() const;
	virtual void start() {}
protected:
    GLuint m_programId;
    glm::mat4 _projection;
    int _npixels;
    GLuint _vao;
    GLuint _vbo;
    GLuint _texture;
	Mode _mode;
};

class MainShader : public Shader {
public:
    MainShader(const std::string& vertexCode, const std::string& fragmentCode, Mode mode) : Shader(vertexCode, fragmentCode), _mode(mode) {}
    void init() override;
    void draw() override;
    void generatePalette();
	void setPixel(int x, int y, int color);
private:
    std::vector<Vertex> _data;
    int _nColors;
    float _invColors;
    int _x0, _y0, _x1, _y1;
    Mode _mode;
};

class BlitShader : public Shader {
public:
    BlitShader(const std::string& vertexCode, const std::string& fragmentCode) : Shader(vertexCode, fragmentCode) {}
    void init() override;
    void draw() override;
    void start() override;
    GLuint getFrameBuffer() const;
private:
    GLuint _fb, _color, _depth;

};

inline GLuint BlitShader::getFrameBuffer() const {
    return _fb;
}



inline GLuint Shader::getProgId() const {
    return m_programId;
}


