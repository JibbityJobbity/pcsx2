#ifdef VK_GLOBAL_FUNC
VK_GLOBAL_FUNC( vkGetInstanceProcAddr )
#endif

#ifdef VK_MODULE_FUNC
VK_MODULE_FUNC( vkCreateInstance )
VK_MODULE_FUNC( vkEnumerateInstanceLayerProperties )
VK_MODULE_FUNC( vkEnumerateInstanceExtensionProperties )
#endif

#ifdef VK_INSTANCE_FUNC
VK_INSTANCE_FUNC( vkDestroyInstance )
#endif

#ifdef VK_DEVICE_FUNC

// TODO

#endif