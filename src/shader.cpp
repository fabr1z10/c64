#include "shader.h"
#include <iostream>
#include "settings.h"


Shader::Shader(const std::string& vertexCode, const std::string& fragmentCode) : m_programId(GL_INVALID_VALUE) {

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    GLint result;
    GLuint vid = glCreateShader(GL_VERTEX_SHADER);
    GLuint fid = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vid, 1, &vShaderCode, 0);
    glShaderSource(fid, 1, &fShaderCode, 0);

    glCompileShader(vid);
    glGetShaderiv(vid, GL_COMPILE_STATUS, &result);
    if (GL_FALSE == result) {
        std::cerr << "Error while compiling vertex shader\n";
        GLint blen;
        glGetShaderiv(vid, GL_INFO_LOG_LENGTH, &blen);
        if (blen > 1) {
            char* compiler_log = (char*)malloc(blen);
            glGetInfoLogARB(vid, blen, 0, compiler_log);
            std::cerr << compiler_log << "\n";
            free(compiler_log);
        }
        glDeleteShader(vid);
        glDeleteShader(fid);
    }

    glCompileShader(fid);
    glGetShaderiv(fid, GL_COMPILE_STATUS, &result);
    if (GL_FALSE == result) {
        std::cerr << "Error while compiling fragment shader\n";
        GLint blen;
        glGetShaderiv(fid, GL_INFO_LOG_LENGTH, &blen);
        if (blen > 1) {
            char* compiler_log = (char*)malloc(blen);
            glGetInfoLogARB(fid, blen, 0, compiler_log);
            std::cerr << compiler_log << "\n";
            free(compiler_log);
        }
        glDeleteShader(vid);
        glDeleteShader(fid);

    }

    GLuint progId = glCreateProgram();
    glAttachShader(progId, vid);
    glAttachShader(progId, fid);
    glLinkProgram(progId);

    glGetProgramiv(progId, GL_LINK_STATUS, &result);
    if (GL_FALSE == result) {
        std::cerr << "Error while linking program\n";
        GLchar infoLog[1024];
        glGetProgramInfoLog(progId, 1024, NULL, infoLog);
        std::cerr << infoLog << "\n";
        exit(1);

    }
    m_programId = progId;


}


Shader::~Shader() {
    glDeleteProgram(m_programId);
}

void BlitShader::start() {

	glBindFramebuffer(GL_FRAMEBUFFER, _fb);
	glViewport(0, 0, settings::visible_width, settings::visible_height);
	glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void BlitShader::init() {
    glUseProgram(m_programId);
    float quadVertices[] = {
            // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
            // positions   // texCoords
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
            1.0f, -1.0f,  1.0f, 0.0f,

            -1.0f,  1.0f,  0.0f, 1.0f,
            1.0f, -1.0f,  1.0f, 0.0f,
            1.0f,  1.0f,  1.0f, 1.0f
    };
    // screen quad VAO
    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);
    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // create frame buffer
    glGenFramebuffers(1, &_fb);
    glBindFramebuffer(GL_FRAMEBUFFER, _fb);
    glGenTextures(1, &_color);
    glBindTexture(GL_TEXTURE_2D, _color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, settings::visible_width, settings::visible_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _color, 0);

    // We also want to make sure OpenGL is able to do depth testing (and optionally stencil testing) so we have to make
    // sure to add a depth (and stencil) attachment to the framebuffer. Since we'll only be sampling the color buffer
    // and not the other buffers we can create a renderbuffer object for this purpose.

    // Creating a renderbuffer object isn't too hard. The only thing we have to remember is that we're creating it as a
    // depth and stencil attachment renderbuffer object. We set its internal format to GL_DEPTH24_STENCIL8 which
    // is enough precision for our purposes:
    glGenRenderbuffers(1, &_depth);
    glBindRenderbuffer(GL_RENDERBUFFER, _depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, settings::visible_width, settings::visible_height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _depth);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void MainShader::setPixel(int x, int y, int color) {
	if (x < _x0 || x >= _x1 || y < _y0 || y > _y1) return;
	_data[(y - _y0) * settings::visible_width + (x - _x0)].color = color;
}

void MainShader::init() {
    glUseProgram(m_programId);
    // initialize data
    _npixels = settings::visible_width * settings::visible_height;
    _nColors = COLOR_COUNT;
    _invColors = 1.0f / _nColors;
    _data.resize(_npixels);
    size_t i{0};
    size_t color{0};
    for (size_t row = 0; row < settings::visible_height; row++) {
        for (size_t col = 0; col < settings::visible_width; col++) {
            _data[i].pos = glm::vec2(col, row);
            _data[i++].color = (color + 0.5f) * _invColors;
            color = (color+1)%16 ;
        }
    }
    _x0 = (settings::width - settings::visible_width) / 2;
	_y0 = (settings::height - settings::visible_height) / 2;

    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    // create buffer
    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * _data.size(), &_data[0], GL_DYNAMIC_DRAW);

    auto stride = sizeof(Vertex);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, stride, (const void*)offsetof(Vertex, color));

    // setup projection matrix for this machine
    _projection = glm::mat4(1.f);
    _projection[0][0] = 2.0 / settings::visible_width;
    _projection[1][1] = -2.0 / settings::visible_height;
    _projection[3][0] = -1.0;
    _projection[3][1] = 1.0;
    generatePalette();
}

void BlitShader::draw() {
    glUseProgram(m_programId);
    glBindVertexArray(_vao);
    setInt("screenTexture", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _color);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void MainShader::draw() {
    glUseProgram(m_programId);
    glBindVertexArray(_vao);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * _npixels, &_data[0]);

    //_cam->init(s);

    // set view matrix
    //s->setMat4("view", _cam->getViewMatrix());

    setMat4("projection", _projection);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _texture);
    setInt("palette", 0);

    glDrawArrays(GL_POINTS, 0, _npixels);
    glBindVertexArray(0);
}

void MainShader::generatePalette() {
    // every pixel has 4 components RGBA
    unsigned char data[]{
            0, 0, 0, 255,           // 0 = black
            255, 255, 255, 255,     // 1 = white
            136, 0, 0, 255,         // 2 = red
            170, 255, 238, 255,     // 3 = cyan
            204, 68, 204, 255,      // 4 = purple
            0, 204, 85, 255,        // 5 = green
            0, 0, 170, 255,         // 6 = blue
            238, 238, 119, 255,     // 7 = yellow
            221, 136, 85, 255,      // 8 = orange
            102, 68, 0, 255,        // 9 = brown
            255, 119, 119, 255,     // 10 = light red
            51, 51, 51, 255,        // 11 = dark grey
            119, 119, 119, 255,     // 12 = grey 2
            170, 255, 102, 255,     // 13 = light green
            0, 136, 255, 255,       // 14 = light blue
            187, 187, 187, 255      // 15 = light grey
    };

    glGenTextures(1, &_texture);
    //glBindTexture(GL_TEXTURE_1D, _texture);
    //glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);
    glGenTextures (1, &_texture);
    glBindTexture (GL_TEXTURE_2D, _texture);
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, 16, 1, 0,  GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);;
}


void Shader:: setInt(const std::string &name, int value) const
{
    glUniform1i(glGetUniformLocation(m_programId, name.c_str()), value);
}

void Shader::setFloat(const std::string &name, float value) const {
    glUniform1f(glGetUniformLocation(m_programId, name.c_str()), value);
}



void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const
{
    auto pippo = glGetUniformLocation(m_programId, name.c_str());
    glUniformMatrix4fv(glGetUniformLocation(m_programId, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}


void Shader::setMat3(const std::string &name, const glm::mat3 &mat) const
{
    auto pippo = glGetUniformLocation(m_programId, name.c_str());
    glUniformMatrix3fv(glGetUniformLocation(m_programId, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setVec4(const std::string &name, const glm::vec4 &value) const
{
    glUniform4fv(glGetUniformLocation(m_programId, name.c_str()), 1, &value[0]);
}

void Shader::setVec3(const std::string &name, const glm::vec3 &value) const
{
    glUniform3fv(glGetUniformLocation(m_programId, name.c_str()), 1, &value[0]);
}