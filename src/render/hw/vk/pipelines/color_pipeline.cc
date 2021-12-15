#include "src/render/hw/vk/pipelines/color_pipeline.hpp"

#include <array>

#include "shader.hpp"
#include "src/logging.hpp"
#include "src/render/hw/vk/vk_framebuffer.hpp"
#include "src/render/hw/vk/vk_interface.hpp"
#include "src/render/hw/vk/vk_memory.hpp"
#include "src/render/hw/vk/vk_utils.hpp"

namespace skity {

std::unique_ptr<VKPipelineWrapper> VKPipelineWrapper::CreateStaticColorPipeline(
    GPUVkContext* ctx) {
  auto static_color_pipeline =
      std::make_unique<StaticColorPipeline>(sizeof(GlobalPushConst));

  auto vertex =
      VKUtils::CreateShader(ctx->GetDevice(), (const char*)vk_common_vert_spv,
                            vk_common_vert_spv_size);

  auto fragment = VKUtils::CreateShader(ctx->GetDevice(),
                                        (const char*)vk_uniform_color_frag_spv,
                                        vk_uniform_color_frag_spv_size);
  static_color_pipeline->Init(ctx, vertex, fragment);

  VK_CALL(vkDestroyShaderModule, ctx->GetDevice(), vertex, nullptr);
  VK_CALL(vkDestroyShaderModule, ctx->GetDevice(), fragment, nullptr);

  return static_color_pipeline;
}

std::unique_ptr<VKPipelineWrapper>
VKPipelineWrapper::CreateStencilColorPipeline(GPUVkContext* ctx) {
  auto stencil_color_pipeline =
      std::make_unique<StencilDiscardColorPipeline>(sizeof(GlobalPushConst));

  auto vertex =
      VKUtils::CreateShader(ctx->GetDevice(), (const char*)vk_common_vert_spv,
                            vk_common_vert_spv_size);

  auto fragment = VKUtils::CreateShader(ctx->GetDevice(),
                                        (const char*)vk_uniform_color_frag_spv,
                                        vk_uniform_color_frag_spv_size);

  stencil_color_pipeline->Init(ctx, vertex, fragment);

  VK_CALL(vkDestroyShaderModule, ctx->GetDevice(), vertex, nullptr);
  VK_CALL(vkDestroyShaderModule, ctx->GetDevice(), fragment, nullptr);

  return stencil_color_pipeline;
}

std::unique_ptr<VKPipelineWrapper>
VKPipelineWrapper::CreateStencilClipColorPipeline(GPUVkContext* ctx) {
  auto stencil_clip_color_pipeline =
      std::make_unique<StencilClipColorPipeline>(sizeof(GlobalPushConst));

  auto vertex =
      VKUtils::CreateShader(ctx->GetDevice(), (const char*)vk_common_vert_spv,
                            vk_common_vert_spv_size);

  auto fragment = VKUtils::CreateShader(ctx->GetDevice(),
                                        (const char*)vk_uniform_color_frag_spv,
                                        vk_uniform_color_frag_spv_size);

  stencil_clip_color_pipeline->Init(ctx, vertex, fragment);

  VK_CALL(vkDestroyShaderModule, ctx->GetDevice(), vertex, nullptr);
  VK_CALL(vkDestroyShaderModule, ctx->GetDevice(), fragment, nullptr);

  return stencil_clip_color_pipeline;
}

static VkDescriptorSetLayout create_color_descriptor_set_layout(
    GPUVkContext* ctx) {
  LOG_DEBUG("Color Pipeline create set 1 layout");
  std::array<VkDescriptorSetLayoutBinding, 2> bindings{
      // set 1 binding 0 global alpha
      VKUtils::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                          VK_SHADER_STAGE_FRAGMENT_BIT, 0),
      // set 1 binding 1 uniform color
      VKUtils::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                          VK_SHADER_STAGE_FRAGMENT_BIT, 1)};

  auto create_info =
      VKUtils::DescriptorSetLayoutCreateInfo(bindings.data(), bindings.size());

  return VKUtils::CreateDescriptorSetLayout(ctx->GetDevice(), create_info);
}

VkDescriptorSetLayout StaticColorPipeline::GenerateColorSetLayout(
    GPUVkContext* ctx) {
  auto binding = VKUtils::DescriptorSetLayoutBinding(
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);

  auto create_info = VKUtils::DescriptorSetLayoutCreateInfo(&binding, 1);

  return VKUtils::CreateDescriptorSetLayout(ctx->GetDevice(), create_info);
}

void StaticColorPipeline::UploadUniformColor(ColorInfoSet const& info,
                                             GPUVkContext* ctx,
                                             VKFrameBuffer* frame_buffer,
                                             VKMemoryAllocator* allocator) {
  auto buffer = frame_buffer->ObtainUniformColorBuffer();

  allocator->UploadBuffer(buffer, (void*)&info, sizeof(ColorInfoSet));

  // color info is in set 2
  auto descriptor_set =
      frame_buffer->ObtainUniformBufferSet(ctx, GetColorSetLayout());

  VkDescriptorBufferInfo buffer_info{buffer->GetBuffer(), 0, sizeof(info)};

  // create VkWriteDescriptorSet to update set
  auto write_set = VKUtils::WriteDescriptorSet(
      descriptor_set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &buffer_info);

  VK_CALL(vkUpdateDescriptorSets, ctx->GetDevice(), 1, &write_set, 0,
          VK_NULL_HANDLE);

  VK_CALL(vkCmdBindDescriptorSets, ctx->GetCurrentCMD(),
          VK_PIPELINE_BIND_POINT_GRAPHICS, GetPipelineLayout(), 2, 1,
          &descriptor_set, 0, nullptr);
}

VkPipelineDepthStencilStateCreateInfo
StencilDiscardColorPipeline::GetDepthStencilStateCreateInfo() {
  auto depth_stencil_state = VKUtils::PipelineDepthStencilStateCreateInfo(
      VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);

  depth_stencil_state.stencilTestEnable = VK_TRUE;
  depth_stencil_state.front.failOp = VK_STENCIL_OP_REPLACE;
  depth_stencil_state.front.passOp = VK_STENCIL_OP_REPLACE;
  depth_stencil_state.front.compareOp = VK_COMPARE_OP_NOT_EQUAL;
  depth_stencil_state.front.compareMask = 0x0F;
  depth_stencil_state.front.writeMask = 0x0F;
  depth_stencil_state.front.reference = 0x00;
  depth_stencil_state.back = depth_stencil_state.front;

  return depth_stencil_state;
}

VkPipelineDepthStencilStateCreateInfo
StencilClipColorPipeline::GetDepthStencilStateCreateInfo() {
  auto depth_stencil_state = VKUtils::PipelineDepthStencilStateCreateInfo(
      VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);

  depth_stencil_state.stencilTestEnable = VK_TRUE;
  depth_stencil_state.front.failOp = VK_STENCIL_OP_REPLACE;
  depth_stencil_state.front.passOp = VK_STENCIL_OP_REPLACE;
  depth_stencil_state.front.compareOp = VK_COMPARE_OP_LESS;
  depth_stencil_state.front.compareMask = 0x1F;
  depth_stencil_state.front.writeMask = 0x0F;
  depth_stencil_state.front.reference = 0x10;
  depth_stencil_state.back = depth_stencil_state.front;

  return depth_stencil_state;
}

}  // namespace skity