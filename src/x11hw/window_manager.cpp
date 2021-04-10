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

#include <x11hw/window_manager.hpp>
#include <x11hw/window.hpp>
#include <x11hw/context.hpp>
#include <x11hw/error.hpp>
#include <stdexcept>

namespace x11hw {

    HwWindowManager::HwWindowManager() {
        mDisplay = XOpenDisplay(nullptr);
        CHECK_MSG(mDisplay, "Failed to create Display");
        CHECK(XSync(mDisplay, False));

        mScreen = XDefaultScreen(mDisplay);
        mContext = std::unique_ptr<HwContext>{new HwContext(mDisplay, mScreen)};
    }

    HwWindowManager::~HwWindowManager() {
        // Release context
        mContext = nullptr;

        // Clear X11 mappings
        mX11Windows.clear();
        mWindows.clear();

        // Close connection
        XCloseDisplay(mDisplay);
        mDisplay = nullptr;
    }

    HwWindow* HwWindowManager::CreateWindow(std::string name, std::string title, glm::uvec2 size) {
        if (ContainsWindow(name)) {
            throw std::runtime_error("Windows names must be unique");
        }

        HwWindow::InitParams params = {
            name,
            std::move(title),
            size,
            mDisplay,
            mScreen,
            mContext.get()
        };

        // Cool hack, since constructor is private - cannot do this in normal way
        std::unique_ptr<HwWindow> window{new HwWindow(params)};
        auto windowPtr = window.get();

        mWindows.emplace(std::move(name), std::move(window));
        mX11Windows.emplace(windowPtr->GetHnd(), windowPtr);

        // If we create first window, then we must init context
        if (mWindows.size() == 1) {
            mContext->CreateContext();
            windowPtr->MakeContextCurrent();
        }

        return windowPtr;
    }

    void HwWindowManager::PollEvents() {
        while (XPending(mDisplay) > 0) {
            XEvent event;
            XNextEvent(mDisplay, &event);

            auto hnd = event.xany.window;
            auto found = mX11Windows.find(hnd);
            found->second->ProcessEvent(event);
        }
    }

    bool HwWindowManager::ContainsWindow(const std::string &name) const {
        auto found = mWindows.find(name);
        return found != mWindows.end();
    }

    HwWindow* HwWindowManager::GetWindow(const std::string &name) {
        auto found = mWindows.find(name);
        return found != mWindows.end()? found->second.get(): nullptr;
    }

}
