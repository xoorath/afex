[Home](./README.md) > Todo

# Engine Architecture 
* **(Priority high)** Fix and document the threading model
    * Currently hangs on close because of the busted semaphore architecture (for some reason only when no debugger is attached)
* **(Priority mid)** Introduce dependency injection
* **(Priority mid)** Decouple Graphics and Platform
* **(Priority low)** Continue update and render while resizing. Look into glfwSetFramebufferSizeCallback for handling resize callbacks and re-architect the update loop so it can respond during that callback.

# Build System
* **(Priority low)** Switch subsystem from console to windows / use WinMain
* **(Priority low)** Create a static library build config for final builds

# Donsol
* **(Priority low)** Figure out if imgui can realistically be used for the UI or if styling will be a pain
