////////////////////////////////////////////////////////////////////////////////////
// MIT License                                                                    //
//                                                                                //
// Copyright (c) 2021 Egor Orachyov                                               //
//                                                                                //
// Permission is hereby granted, free of charge, to any person obtaining a copy   //
// of this software and associated documentation files (the "Software"), to deal  //
// in the Software without restriction, including without limitation the rights   //
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell      //
// copies of the Software, and to permit persons to whom the Software is          //
// furnished to do so, subject to the following conditions:                       //
//                                                                                //
// The above copyright notice and this permission notice shall be included in all //
// copies or substantial portions of the Software.                                //
//                                                                                //
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR     //
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,       //
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE    //
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER         //
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,  //
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE  //
// SOFTWARE.                                                                      //
////////////////////////////////////////////////////////////////////////////////////

#include <GL/glew.h>
#include <glm/gtx/transform.hpp>

#include <x11hw/window.hpp>
#include <x11hw/window_manager.hpp>
#include <x11hw/shader.hpp>
#include <x11hw/geometry.hpp>

#include <stdexcept>
#include <iostream>
#include <chrono>
#include <thread>

const char *GetVertexStageCode() {
    return R"(
        #version 330 core
        layout (location = 0) in vec2 position;
        layout (location = 1) in vec3 color;

        out vec3 fsColor;

        uniform vec2 triangleSize;
        uniform vec2 mousePosition;
        uniform mat4 projView;

        void main() {
            fsColor = color;
            vec2 screenPosition = mousePosition + position * triangleSize;
            gl_Position = projView * vec4(screenPosition, 0.0f, 1.0f);
        }
    )";
}

const char *GetFragmentStageCode() {
    return R"(
        #version 330 core
        layout (location = 0) out vec4 outColor;

        in vec3 fsColor;

        uniform float basicGamma;

        void main() {
            outColor = vec4(pow(fsColor, vec3(1.0f / basicGamma)), 1.0f);
        }
    )";
}

x11hw::HwGeometry::InitParams GetTriangleParams() {
    x11hw::HwGeometry::InitParams params;
    params.verticesCount = 3;
    params.stride = (2 + 3) * sizeof(float);
    params.topology = GL_TRIANGLES;
    params.attributes.push_back({(0) * sizeof(float), 2, GL_FLOAT, false});
    params.attributes.push_back({(2) * sizeof(float), 3, GL_FLOAT, false});

    return params;
}

const void *GetTriangleData() {
    static const float vertices[] = {
     //  vec2 position      vec3 color
         0.0f,  0.0f,       1.0f, 0.0f, 0.0f,
        -0.5f,  1.0f,       0.0f, 1.0f, 0.0f,
         0.5f,  1.0f,       0.0f, 0.0f, 1.0f
    };

    return vertices;
}

int main(int, const char *const *) {
    // Window (background color = #25854b) setting
    glm::vec4 clearColor{0.145, 0.522, 0.294, 1.0f};
    glm::uvec2 windowSize{1280, 720};
    std::string name = "MAIN_WINDOW";
    std::string title = "X11 Hello World!";
    float gamma = 2.2f;

    // For triangle drawing
    bool shouldClose = false;
    bool showTriangle = false;
    glm::ivec2 mousePosition;
    glm::vec2 triangleSize{120.0f, 120.0f};

    // Create window manager and primary window
    auto windowManager = std::make_shared<x11hw::HwWindowManager>();
    auto window = windowManager->CreateWindow(name, title, windowSize);

    // Will draw only into single window
    window->MakeContextCurrent();
    window->SetSwapInterval(1);

    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to init GLEW" << std::endl;
        return 1;
    }

    // Setup close callback
    window->SubscribeOnClose([&]() {
        shouldClose = true;
    });

    // Subscribe for input events to move triangle
    window->SubscribeOnInput([&](const x11hw::HwWindow::EventData &event) {
        using namespace x11hw;

        if (event.type == HwWindow::EventType::MouseButtonPressed &&
            event.mouseButton == HwWindow::MouseButton::Left) {
            showTriangle = true;
            mousePosition = event.mousePosition;
        }
        if (event.type == HwWindow::EventType::MouseMoved) {
            mousePosition = event.mousePosition;
        }
        if (event.type == HwWindow::EventType::MouseButtonReleased &&
            event.mouseButton == HwWindow::MouseButton::Left) {
            showTriangle = false;
            mousePosition = event.mousePosition;
        }
    });

    // Create gl objets for drawing
    auto shader = std::make_shared<x11hw::HwShader>(GetVertexStageCode(), GetFragmentStageCode());
    auto geometry = std::make_shared<x11hw::HwGeometry>(GetTriangleParams());
    geometry->Update(0, geometry->GetBufferSize(), GetTriangleData());

    // Frame rate control
    using microseconds = std::chrono::microseconds;
    using timer = std::chrono::steady_clock;
    auto desiredDelta = microseconds{16666};
    auto prevTime = timer::now();

    while (!shouldClose) {
        auto currentTime = timer::now();
        auto delta = currentTime - prevTime;

        // Sleep if update is too fast
        if (delta < desiredDelta) {
            auto toSleep = desiredDelta - delta;
            std::this_thread::sleep_until(currentTime + toSleep);
            currentTime = timer::now();
        }

        prevTime = currentTime;

        // Query input
        windowManager->PollEvents();

        // Setup drawing area and clear color buffer
        glViewport(0, 0, window->GetSize().x, window->GetSize().y);
        glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
        glClear(GL_COLOR_BUFFER_BIT);

        // Only if user holds left mouse button
        if (showTriangle) {
            static const std::string PROJ_VIEW = "projView";
            static const std::string BASIC_GAMMA = "basicGamma";
            static const std::string TRIANGLE_SIZE = "triangleSize";
            static const std::string MOUSE_POSITION = "mousePosition";

            // Flip Y-axis, so mouse position is correct (triangle vertices also flipped)
            auto size = window->GetFramebufferSize();
            auto proj = glm::ortho(0.0f, (float) size.x, (float) size.y, 0.0f, -1.0f, 1.0f);

            shader->Bind();
            shader->SetFloat(BASIC_GAMMA, gamma);
            shader->SetVec2(TRIANGLE_SIZE, triangleSize);
            shader->SetVec2(MOUSE_POSITION, mousePosition);
            shader->SetMatrix4(PROJ_VIEW, proj);
            geometry->Draw();
            shader->Unbind();
        }

        // Present image
        window->SwapBuffers();
    }

    // Explicitly release in reverse order for safety
    window = nullptr;
    windowManager = nullptr;

    return 0;
}


