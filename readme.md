# TODO
抽象出来 mesh (geom , material) , shader ,  camera  , transform, RenderPass 。


方案：
对于纹理类型的资源，使用 根描述符表。 纹理是静态资源，只需要一份。
创建一个CPU描述符堆(着色器不可见)，一个GPU描述符堆(着色器可见)，这两个都是全局的。
在加载和创建纹理RES时，先从CPU描述符堆中创建句柄，同时，纹理对像也持有此句柄信息。
在绘制时，将 纹理的句柄 从CPU描述符堆中 拷贝 到 GPU描述符堆中，从而让着色器可访问。

对于相机的每帧更新的数据，使用CBV 根描述符。 不需要创建资源描述符。 每帧更新的资源，创建3份。 

对于per obj的数据，每个模型需要更新的数据， 使用CBV 根描述符。 不需要创建资源描述符。
如果为每个frameCount 每个obj  都创建一个RES缓冲， 感觉很浪费哇， 比如场景有1000个模型，那么要创建3000个buffer。
其实对于场景静态模型来说，只创建一份即可，用一个缓冲，在绘制之前绑定到不同的偏移，对于静态场景来说，偏移是固定的。
对于动态模型，比如单个角色的worldMat，需要创建3份。如果是2个角色，那么需要创建6份。在角色出现时创建，在角色移出视线后可以销毁。
【可以包含 worldMat 以及材质数据， 如果材质数据是动态的，那么就要创建多份了】


unlit材质： 包含固定的shader 固定的根签名  固定的pso  以及全局固定的顶点格式信息(减少复杂度哇)。
其中 根参数为： [根描述符CBV, 根描述符CBV, 根描述符表SRV] 
第一个为相机信息，每帧更新一次； 创建3份资源，动态数据。
第二个为mvp矩阵，每个obj更新一次； 每个obj创建1份资源，静态 不合并。每个obj创建3份资源，动态 不合并 浪费哇。所有obj创建1份，静态 合并。所有obj创建3份，动态 合并。
第三个为纹理信息，静态数据，不更新； 创建1份资源，静态。

再说一下材质和pso的问题：
假如场景有两个模型，都使用unlit材质， 那么他们的pso需要引用到同一个，只要创建一份哦。 【使用静态成员变量 static ID3D12PipelineState* pipelineStateObject;】
但是两个模型的 纹理 和 worldMat 不同， 其中纹理在材质对象中。


![Cover Art](https://alain.xyz/blog/raw-directx12/assets/cover.jpg)

# DirectX 12 Seed

[![cmake-img]][cmake-url]
[![License][license-img]][license-url]

A DirectX 12 repo you can use to get started with your own renderer.

## Setup

First install:

- [Git](https://git-scm.com/)

- [CMake](https://cmake.org)

- [Visual Studio](https://visualstudio.microsoft.com/downloads/)

Then type the following in your [terminal](https://hyper.is/).

```bash
# 🐑 Clone the repo
git clone https://github.com/alaingalvan/directx12-seed --recurse-submodules

# 💿 go inside the folder
cd directx12-seed

# 👯 If you forget to `recurse-submodules` you can always run:
git submodule update --init

# 👷 Make a build folder
mkdir build
cd build

# 🖼️ To build your Visual Studio solution on Windows x64
cmake .. -A x64

# 🔨 Build project
cmake --build .
```

> Refer to [this blog post on designing C++ libraries and apps](https://alain.xyz/blog/designing-a-cpp-library) for more details on CMake, Git Submodules, etc.

## Project Layout

As your project becomes more complex, you'll want to separate files and organize your application to something more akin to a game or renderer, check out this post on [game engine architecture](https://alain.xyz/blog/game-engine-architecture) and this one on [real time renderer architecture](https://alain.xyz/blog/realtime-renderer-architectures) for more details.

```bash
├─ 📂 external/                    # 👶 Dependencies
│  ├─ 📁 crosswindow/                    # 🖼️ OS Windows
│  ├─ 📁 crosswindow-graphics/           # 🎨 DirectX 12 Swapchain Creation
│  └─ 📁 glm/                            # ➕ Linear Algebra
├─ 📂 src/                         # 🌟 Source Files
│  ├─ 📄 Utils.h                         # ⚙️ Utilities (Load Files, Check Shaders, etc.)
│  ├─ 📄 Renderer.h                      # 🔺 Triangle Draw Code
│  ├─ 📄 Renderer.cpp                    # -
│  └─ 📄 Main.cpp                        # 🏁 Application Main
├─ 📄 .gitignore                   # 👁️ Ignore certain files in git repo
├─ 📄 CMakeLists.txt               # 🔨 Build Script
├─ 📄 license.md                   # ⚖️ Your License (Unlicense)
└─ 📃readme.md                     # 📖 Read Me!
```

[cmake-img]: https://img.shields.io/badge/cmake-3.6-1f9948.svg?style=flat-square
[cmake-url]: https://cmake.org/
[license-img]: https://img.shields.io/:license-mit-blue.svg?style=flat-square
[license-url]: https://opensource.org/licenses/MIT
