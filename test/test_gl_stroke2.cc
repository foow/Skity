// this need to be first include
#include "test/common/test_common.hpp"
//

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <skity/graphic/paint.hpp>
#include <skity/graphic/path.hpp>

#include "src/render/gl/gl_interface.hpp"
#include "src/render/gl/gl_mesh.hpp"
#include "src/render/gl/gl_shader.hpp"
#include "src/render/gl/gl_stroke2.hpp"
#include "src/render/gl/gl_vertex.hpp"

class TestGLStroke2 : public test::TestApp {
 public:
  TestGLStroke2() : test::TestApp(800, 800) {}

 protected:
  void OnInit() override {
    mvp_ = glm::ortho<float>(0, 800, 800, 0, -100, 100);
    InitGL();
    InitMesh();
  }
  void OnDraw() override {
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    DrawMesh();
  }

 private:
  void InitGL() {
    skity::GLInterface::InitGlobalInterface((void*)glfwGetProcAddress);

    glClearColor(0.3f, 0.4f, 0.5f, 1.f);
    glClearStencil(0x00);
    glStencilMask(0xFF);

    // blend is need for anti-alias
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    shader_ = skity::GLShader::CreateUniverseShader();
  }
  void InitMesh() {
    mesh_ = std::make_unique<skity::GLMesh>();

    skity::GLVertex2 gl_vertex;

    skity::Paint paint;
    paint.setAntiAlias(true);
    paint.setStrokeWidth(30.f);
    paint.setStrokeJoin(skity::Paint::kMiter_Join);
    paint.setStrokeCap(skity::Paint::kButt_Cap);
    paint.setColor(skity::Color_WHITE);

    skity::Path path;

    path.moveTo(100, 100);
    path.lineTo(300, 120);
    path.lineTo(130, 300);

    skity::GLStroke2 gl_stroke{paint, &gl_vertex};

    range_ = gl_stroke.VisitPath(path, false);

    mesh_->Init();
    mesh_->BindMesh();

    auto vertex_data = gl_vertex.GetVertexDataSize();
    auto front_data = gl_vertex.GetFrontDataSize();
    auto back_data = gl_vertex.GetBackDataSize();
    auto aa_data = gl_vertex.GetAADataSize();

    if (std::get<1>(vertex_data) > 0) {
      mesh_->UploadVertexBuffer(std::get<0>(vertex_data),
                                std::get<1>(vertex_data));
    }

    if (std::get<1>(front_data) > 0) {
      mesh_->UploadFrontIndex(std::get<0>(front_data), std::get<1>(front_data));
    }

    if (std::get<1>(back_data) > 0) {
      mesh_->UploadBackIndex(std::get<0>(front_data), std::get<1>(back_data));
    }

    if (std::get<1>(aa_data) > 0) {
    }
  }

  void DrawMesh() {
    shader_->Bind();

    shader_->SetMVPMatrix(mvp_);
    shader_->SetUserColor(skity::Colors::kWhite);
    shader_->SetUserData1({skity::GLUniverseShader::kPureColor, 0, 0, 0});

    mesh_->BindMesh();

    //    skity::GLMeshDraw2 draw2{GL_TRIANGLES, range_.front_start,
    //                             range_.front_count};
    //
    //    draw2();

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void*)(2 * sizeof(float)));

    mesh_->BindFrontIndex();
    glDrawElements(GL_TRIANGLES, range_.front_count, GL_UNSIGNED_INT, 0);

    mesh_->UnBindMesh();

    shader_->UnBind();
  }

 private:
  std::unique_ptr<skity::GLUniverseShader> shader_;
  std::unique_ptr<skity::GLMesh> mesh_;
  glm::mat4 mvp_;
  skity::GLMeshRange range_;
};

int main(int argc, const char** argv) {
  TestGLStroke2 app{};
  app.Start();
  return 0;
}