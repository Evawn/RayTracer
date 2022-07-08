// Mesh.hpp

#pragma once

#include <nanogui/opengl.h>

#include "Util.hpp"

namespace GLWrap {


// A Mesh represents an OpenGL VAO and the buffers bound to it.
// It is basically a numbered list of buffers with types.
// It is deliberately limited in some ways:
//  * 32-bit floats are the only supported datatype
//  * only scalar and vec[234] attribute types are supported
//  * indexed meshes are always drawn in full
//  * each attribute comes contiguously from a separate buffer
//  * buffers are owned by meshes and will not be shared between them
//  * we do not keep track of names for the attributes; 
//      they will always be referenced by number from the shaders
class GLWRAP_EXPORT Mesh {
public:

    // A newly created mesh owns its VAO but no buffers
    Mesh();

    // Deleting a mesh releases all GPU resources it owns
    ~Mesh();

    // Copying not allowed because a Mesh owns GPU resources
    Mesh(const Mesh &) = delete;
    Mesh &operator=(const Mesh &) = delete;

    // Moving is allowed, and transfers ownership of the GPU resources
    Mesh(Mesh &&other) noexcept;
    Mesh &operator=(Mesh &&other);

    // Provide values for the vertex attribute at a particular index.
    // The dimension of the vector type of the attribute is determined by
    // the argument type.
    void setAttribute(int index, Eigen::Matrix<float, 1, Eigen::Dynamic> data);
    void setAttribute(int index, Eigen::Matrix<float, 2, Eigen::Dynamic> data);
    void setAttribute(int index, Eigen::Matrix<float, 3, Eigen::Dynamic> data);
    void setAttribute(int index, Eigen::Matrix<float, 4, Eigen::Dynamic> data);

    // Provide indices that define primitives, 
    // and the drawing mode (GL_TRIANGLES, etc.)
    void setIndices(Eigen::VectorXi data, GLenum mode);

    // Draw the mesh using glDrawElements (using index buffer)
    void drawElements() const;

    // Draw the mesh using glDrawArrays (using just the attribute buffers)
    void drawArrays(GLuint mode, int first, int count) const;

private:

    // Template to simplify writing the various setAttribute functions
    template<class T, int N>
    void _setAttribute(int index, Eigen::Matrix<T, N, Eigen::Dynamic> data);

    // OpenGL identifiers for the owned resources
    GLuint vao;
    GLuint indexBuffer;
    std::vector<GLuint> vertexBuffers;

    // mode and length of index buffer
    GLenum indexMode;
    GLuint indexLength;
};

} // namespace

