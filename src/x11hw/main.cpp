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
#include <x11hw/window.hpp>
#include <x11hw/window_manager.hpp>
#include <stdexcept>
#include <iostream>

int main(int argc, const char* const *argv) {
    bool shouldClose;

    glm::uvec2 size{1280, 720};
    std::string name = "MAIN_WINDOW";
    std::string title = "Main Window";

    auto windowManager = std::make_shared<x11hw::HwWindowManager>();
    auto window = windowManager->CreateWindow(name, title, size);

    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to init GLEW" << std::endl;
        return 1;
    }

    window->SubscribeOnClose([&]() {
        shouldClose = true;
    });

    window->SubscribeOnInput([&](const x11hw::HwWindow::EventData& event) {
        using namespace x11hw;

        if (event.type == HwWindow::EventType::MouseButtonPressed) {
            std::cout << "Mouse press button " << (int)event.mouseButton << std::endl;
        }
        if (event.type == HwWindow::EventType::MouseButtonReleased) {
            std::cout << "Mouse release button " << (int)event.mouseButton << std::endl;
        }
        if (event.type == HwWindow::EventType::MouseMoved) {
            std::cout << "Mouse move " << event.mousePosition.x << " " << event.mousePosition.y << std::endl;
        }
    });

    while (!shouldClose) {
        windowManager->PollEvents();

        window->MakeContextCurrent();

        glViewport(0, 0, window->GetSize().x, window->GetSize().y);
        glClearColor(0.3f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        window->SwapBuffers();
    }

    window = nullptr;
    windowManager = nullptr;

    return 0;
}
