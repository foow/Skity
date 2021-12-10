#ifndef SKITY_SRC_RENDER_HW_VK_VK_INTERFACE_HPP
#define SKITY_SRC_RENDER_HW_VK_VK_INTERFACE_HPP

#include <vulkan/vulkan.h>

namespace skity {

#define VK_CALL(name, ...) VKInterface::GlobalInterface()->f##name(__VA_ARGS__)

struct VKInterface {
  static VKInterface* GlobalInterface();
  static void InitGlobalInterface(VkDevice device,
                                  PFN_vkGetDeviceProcAddr proc_loader);

  PFN_vkCmdBindPipeline fvkCmdBindPipeline = {};
  PFN_vkCreateDescriptorSetLayout fvkCreateDescriptorSetLayout = {};
  PFN_vkCreateGraphicsPipelines fvkCreateGraphicsPipelines = {};
  PFN_vkCreatePipelineLayout fvkCreatePipelineLayout = {};
  PFN_vkCreateShaderModule fvkCreateShaderModule = {};
  PFN_vkDestroyDescriptorSetLayout fvkDestroyDescriptorSetLayout = {};
  PFN_vkDestroyPipeline fvkDestroyPipeline = {};
  PFN_vkDestroyPipelineLayout fvkDestroyPipelineLayout = {};
  PFN_vkDestroyShaderModule fvkDestroyShaderModule = {};
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_VK_VK_INTERFACE_HPP