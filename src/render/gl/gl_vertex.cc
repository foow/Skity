#include "src/render/gl/gl_vertex.hpp"

#include <cassert>

namespace skity {

GLVertex::GLVertex() {
  vertex_buffer.reserve(128);
  front_index.reserve(128);
  back_index.reserve(128);
  aa_index.reserve(128);
}

uint32_t GLVertex::AddPoint(float x, float y, uint32_t type, float u, float v) {
  return AddPoint(x, y, 1.f, type, u, v);
}

uint32_t GLVertex::AddPoint(float x, float y, float alpha, uint32_t type,
                            float u, float v) {
  if (vertex_buffer.size() == vertex_buffer.capacity()) {
    vertex_buffer.reserve(2 * vertex_buffer.size());
  }
  uint32_t index = vertex_buffer.size() / GL_VERTEX_SIZE;
  vertex_buffer.emplace_back(x);                      // x
  vertex_buffer.emplace_back(y);                      // y
  vertex_buffer.emplace_back(alpha * global_alpha_);  // alpha
  vertex_buffer.emplace_back(type);                   // type
  vertex_buffer.emplace_back(u);                      // u
  vertex_buffer.emplace_back(v);                      // v
  return index;
}

uint32_t GLVertex::AddPoint(VertexData const& data) {
  return AddPoint(data[0], data[1], data[2], data[3], data[4], data[6]);
}

void GLVertex::AddFront(uint32_t v1, uint32_t v2, uint32_t v3) {
  if (front_index.size() == front_index.capacity()) {
    front_index.reserve(2 * front_index.size());
  }
  front_index.emplace_back(v1);
  front_index.emplace_back(v2);
  front_index.emplace_back(v3);
}

void GLVertex::AddBack(uint32_t v1, uint32_t v2, uint32_t v3) {
  if (back_index.size() == back_index.capacity()) {
    back_index.reserve(2 * back_index.size());
  }
  back_index.emplace_back(v1);
  back_index.emplace_back(v2);
  back_index.emplace_back(v3);
}

void GLVertex::AddAAOutline(uint32_t v1, uint32_t v2, uint32_t v3) {
  if (aa_index.size() == aa_index.capacity()) {
    aa_index.reserve(2 * aa_index.size());
  }
  aa_index.emplace_back(v1);
  aa_index.emplace_back(v2);
  aa_index.emplace_back(v3);
}

GLVertex::VertexData GLVertex::GetVertex(uint32_t index) {
  assert(index < vertex_buffer.size());
  VertexData result;

  result[GL_VERTEX_X] = vertex_buffer[index * GL_VERTEX_SIZE + GL_VERTEX_X];
  result[GL_VERTEX_Y] = vertex_buffer[index * GL_VERTEX_SIZE + GL_VERTEX_Y];
  result[GL_VERTEX_ALPHA] =
      vertex_buffer[index * GL_VERTEX_SIZE + GL_VERTEX_ALPHA];
  result[GL_VERTEX_TYPE] =
      vertex_buffer[index * GL_VERTEX_SIZE + GL_VERTEX_TYPE];
  result[GL_VERTEX_U] = vertex_buffer[index * GL_VERTEX_SIZE + GL_VERTEX_U];
  result[GL_VERTEX_V] = vertex_buffer[index * GL_VERTEX_SIZE + GL_VERTEX_V];

  return result;
}

void GLVertex::UpdateVertexData(VertexData const& data, uint32_t index) {
  assert(index < vertex_buffer.size());
  vertex_buffer[index * GL_VERTEX_SIZE + GL_VERTEX_X] = data[GL_VERTEX_X];
  vertex_buffer[index * GL_VERTEX_SIZE + GL_VERTEX_Y] = data[GL_VERTEX_Y];
  vertex_buffer[index * GL_VERTEX_SIZE + GL_VERTEX_ALPHA] =
      data[GL_VERTEX_ALPHA];
  vertex_buffer[index * GL_VERTEX_SIZE + GL_VERTEX_TYPE] = data[GL_VERTEX_TYPE];
  vertex_buffer[index * GL_VERTEX_SIZE + GL_VERTEX_U] = data[GL_VERTEX_U];
  vertex_buffer[index * GL_VERTEX_SIZE + GL_VERTEX_V] = data[GL_VERTEX_V];
}

void GLVertex::Reset() {
  vertex_buffer.clear();
  front_index.clear();
  back_index.clear();
  aa_index.clear();
}

void GLVertex::Append(GLVertex* other, const Matrix& matrix) {
  uint32_t vertex_base = vertex_buffer.size() / GL_VERTEX_SIZE;
  uint32_t front_base = vertex_base;
  uint32_t back_base = vertex_base;
  uint32_t aa_base = vertex_base;

  for (size_t i = 0; i < other->vertex_buffer.size(); i += GL_VERTEX_SIZE) {
    Point p{other->vertex_buffer[i + GL_VERTEX_X],
            other->vertex_buffer[i + GL_VERTEX_Y], 0.f, 1.f};

    p = matrix * p;

    this->vertex_buffer.emplace_back(p.x);
    this->vertex_buffer.emplace_back(p.y);
    this->vertex_buffer.emplace_back(other->vertex_buffer[i + GL_VERTEX_ALPHA]);
    this->vertex_buffer.emplace_back(other->vertex_buffer[i + GL_VERTEX_TYPE]);
    this->vertex_buffer.emplace_back(other->vertex_buffer[i + GL_VERTEX_U]);
    this->vertex_buffer.emplace_back(other->vertex_buffer[i + GL_VERTEX_V]);
  }

  for (auto f : other->front_index) {
    this->front_index.emplace_back(f + front_base);
  }
  for (auto b : other->back_index) {
    this->back_index.emplace_back(b + back_base);
  }
  for (auto a : other->aa_index) {
    this->aa_index.emplace_back(a + aa_base);
  }
}

void GLVertex::Append(GLVertex* other, float scale, float tx, float ty) {
  uint32_t vertex_base = vertex_buffer.size() / GL_VERTEX_SIZE;
  uint32_t vertex_raw_base = vertex_buffer.size();
  uint32_t front_base = vertex_base;
  uint32_t back_base = vertex_base;
  uint32_t aa_base = vertex_base;
  
  this->vertex_buffer.resize(this->vertex_buffer.size() + other->vertex_buffer.size());
  this->front_index.resize(this->FrontCount() + other->FrontCount());
  this->back_index.reserve(this->BackCount() + other->BackCount());
  this->aa_index.reserve(this->AAOutlineCount() + other->AAOutlineCount());

  const float* base_addr = other->vertex_buffer.data();
  float* this_addr = this->vertex_buffer.data();
  for (size_t i = 0; i < other->vertex_buffer.size(); i += GL_VERTEX_SIZE) {
    float x = base_addr[i + GL_VERTEX_X] * scale + tx;
    float y = base_addr[i + GL_VERTEX_Y] * scale + ty;

    this_addr[vertex_raw_base + i] = x;
    this_addr[vertex_raw_base + i + 1] = y;
    this_addr[vertex_raw_base + i + GL_VERTEX_ALPHA] = base_addr[i + GL_VERTEX_ALPHA];
    this_addr[vertex_raw_base + i + GL_VERTEX_TYPE] = base_addr[i + GL_VERTEX_TYPE];
    this_addr[vertex_raw_base + i + GL_VERTEX_U] = base_addr[i + GL_VERTEX_U];
    this_addr[vertex_raw_base + i + GL_VERTEX_V] = base_addr[i + GL_VERTEX_V];
  }

  for (auto f : other->front_index) {
    this->front_index.emplace_back(f + front_base);
  }
  for (auto b : other->back_index) {
    this->back_index.emplace_back(b + back_base);
  }
  for (auto a : other->aa_index) {
    this->aa_index.emplace_back(a + aa_base);
  }
}

}  // namespace skity
