#include "src/render/gl/gl_mesh.hpp"

#include <cassert>

#include "src/render/gl/gl_interface.hpp"

namespace skity {

GLMesh::~GLMesh() {
  if (buffers_[0] != 0) {
    GL_CALL(DeleteBuffers, 3, buffers_.data());
  }
  if (vao_ != 0) {
    GL_CALL(DeleteVertexArrays, 1, &vao_);
  }
}

void GLMesh::UploadVertexBuffer(void* data, uint32_t length) {
  assert(buffers_[0] != 0);

  GL_CALL(BindBuffer, GL_ARRAY_BUFFER, buffers_[0]);
  GL_CALL(BufferData, GL_ARRAY_BUFFER, length, data, GL_STATIC_DRAW);
  GL_CALL(BindBuffer, GL_ARRAY_BUFFER, 0);
}

void GLMesh::UploadFrontIndex(void* data, uint32_t length) {
  assert(buffers_[1] != 0);
  GL_CALL(BindBuffer, GL_ELEMENT_ARRAY_BUFFER, buffers_[1]);
  GL_CALL(BufferData, GL_ELEMENT_ARRAY_BUFFER, length, data, GL_STATIC_DRAW);
  GL_CALL(BindBuffer, GL_ELEMENT_ARRAY_BUFFER, 0);
}

void GLMesh::UploadBackIndex(void* data, uint32_t length) {
  assert(buffers_[2] != 0);
  GL_CALL(BindBuffer, GL_ELEMENT_ARRAY_BUFFER, buffers_[2]);
  GL_CALL(BufferData, GL_ELEMENT_ARRAY_BUFFER, length, data, GL_STATIC_DRAW);
  GL_CALL(BindBuffer, GL_ELEMENT_ARRAY_BUFFER, 0);
}

void GLMesh::Init() {
  assert(vao_ == 0);
  assert(buffers_[0] == 0);
  assert(buffers_[1] == 0);
  assert(buffers_[2] == 0);

  GL_CALL(GenVertexArrays, 1, &vao_);
  GL_CALL(GenBuffers, 3, buffers_.data());
}

void GLMesh::BindMesh() {
  GL_CALL(BindVertexArray, vao_);
  GL_CALL(BindBuffer, GL_ARRAY_BUFFER, buffers_[0]);
}

void GLMesh::BindFrontIndex() {
  GL_CALL(BindBuffer, GL_ELEMENT_ARRAY_BUFFER, buffers_[1]);
}

void GLMesh::BindBackIndex() {
  GL_CALL(BindBuffer, GL_ELEMENT_ARRAY_BUFFER, buffers_[2]);
}

void GLMesh::UnBindMesh() {
  GL_CALL(BindBuffer, GL_ELEMENT_ARRAY_BUFFER, 0);
  GL_CALL(BindBuffer, GL_ARRAY_BUFFER, 0);
  GL_CALL(BindVertexArray, 0);
}

GLMeshDraw::GLMeshDraw(uint32_t mode, uint32_t start, uint32_t count)
    : mode_(mode), start_(start), count_(count) {}

void GLMeshDraw::operator()() {
  GL_CALL(EnableVertexAttribArray, 0);
  GL_CALL(VertexAttribPointer, 0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
          (void*)0);

  GL_CALL(EnableVertexAttribArray, 1);
  GL_CALL(VertexAttribPointer, 1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
          (void*)(3 * sizeof(float)));

  GL_CALL(DrawElements, mode_, count_, GL_UNSIGNED_INT,
          (void*)(start_ * sizeof(GLuint)));
}

}  // namespace skity