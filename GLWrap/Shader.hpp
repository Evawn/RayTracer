// Shader.hpp

#pragma once

#include <nanogui/opengl.h>

#include "Util.hpp"

namespace GLWrap {

class GLWRAP_EXPORT Shader {
public:

    // Create a new shader of the given type (GL_VERTEX_SHADER, etc.)
    // The name is used for error reporting.
    Shader(GLenum type, std::string name);

    // Deletes the OpenGL shader
    ~Shader();

    // Copying Shaders is not permitted because they own OpenGL resources
    Shader(const Shader &) = delete;
    Shader &operator =(const Shader &) = delete;

    // Moving Shaders transfers ownership of the OpenGL resources
    Shader(Shader &&);
    Shader &operator =(Shader &&);

    // Load source code from a file and compile
    // exits if there is a compile error
    void source(const std::string &);

    // The OpenGL shader id is made available for any calls that need to be 
    // made that are not supported by this class.
    GLuint id() const { return shader; }

    // Name is available for debugging
    const std::string &getName() { return name; }

private:

    // We keep a name for purposes of debugging
    std::string name;

    // The OpenGL name of the shader
    GLuint shader;
};

} // namespace