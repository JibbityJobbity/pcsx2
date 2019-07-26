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
VK_INSTANCE_FUNC( vkEnumeratePhysicalDevices )
VK_INSTANCE_FUNC( vkEnumerateDeviceExtensionProperties )
VK_INSTANCE_FUNC( vkGetPhysicalDeviceProperties )
VK_INSTANCE_FUNC( vkGetPhysicalDeviceQueueFamilyProperties )
VK_INSTANCE_FUNC( vkGetPhysicalDeviceSurfaceSupportKHR )
VK_INSTANCE_FUNC( vkCreateDevice )

#ifdef VK_USE_PLATFORM_WIN32_KHR
VK_INSTANCE_FUNC( vkCreateWin32SurfaceKHR )
#endif

#ifdef VK_USE_PLATFORM_XLIB_KHR
VK_INSTANCE_FUNC( vkCreateXlibSurfaceKHR )
#endif
#endif

#ifdef VK_DEVICE_FUNC

// TODO

#endif