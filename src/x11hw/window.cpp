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

#include <x11hw/window.hpp>
#include <x11hw/window_manager.hpp>
#include <stdexcept>
#include <utility>
#include <cassert>

namespace x11hw {

    static HwWindow::MouseButton GetMouseButtonFromId(unsigned int id) {
        switch (id) {
            case 1:
                return HwWindow::MouseButton::Left;
            case 3:
                return HwWindow::MouseButton::Right;
            default:
                return HwWindow::MouseButton::Unknown;
        }
    }

    HwWindow::HwWindow(InitParams& params)
        : mSize(params.size),
          mTitle(std::move(params.title)),
          mName(std::move(params.name)),
          mManager(params.manager) {
        assert(mManager);
        assert(mSize.x > 0 & mSize.y > 0);

        mDisplay = XOpenDisplay(nullptr);

        if (mDisplay == nullptr) {
            throw std::runtime_error("Failed to open Display");
        }

        mScreen = XDefaultScreen(mDisplay);
        mHnd = XCreateSimpleWindow(mDisplay, XRootWindow(mDisplay, mScreen), 0, 0, mSize.x, mSize.y, 1, XBlackPixel(mDisplay, mScreen), XWhitePixel(mDisplay, mScreen));

        if (!mHnd) {
            throw std::runtime_error("Failed to create window");
        }

        // Setup params
        mEventMask = ButtonMotionMask | ButtonPressMask | ButtonReleaseMask | ExposureMask | ResizeRedirectMask;
        mAtomWmDeleteWindow = XInternAtom(mDisplay, "WM_DELETE_WINDOW", False);

        if (!XSetWMProtocols(mDisplay, mHnd, &mAtomWmDeleteWindow, 1)) {
            throw std::runtime_error("Failed to set WM protocols");
        }

        XSelectInput(mDisplay, mHnd, mEventMask);
        XStoreName(mDisplay, mHnd, mTitle.c_str());

        // Show the window
        XClearWindow(mDisplay, mHnd);
        XMapRaised(mDisplay, mHnd);
    }

    HwWindow::~HwWindow() {
        XDestroyWindow(mDisplay, mHnd);
        XCloseDisplay(mDisplay);

        mHnd = 0;
        mScreen = 0;
        mDisplay = nullptr;
    }

    void HwWindow::MakeContextCurrent() {

    }

    void HwWindow::SwapBuffers() {

    }

    void HwWindow::NotifyInput(const EventData &event) {
        for (auto& c: mOnInputCallbacks) {
            c(event);
        }
    }

    void HwWindow::NotifyClose() {
        for (auto& c: mOnCloseCallbacks) {
            c();
        }
    }

    void HwWindow::CheckEvents() {
        while (XPending(mDisplay) > 0)
        {
            XEvent event;
            XNextEvent(mDisplay, &event);

            switch (event.type) {
                case ButtonPress: {
                    EventData eventData;
                    eventData.type = EventType::MouseButtonPressed;
                    eventData.mouseButton = GetMouseButtonFromId(event.xbutton.button);
                    eventData.mousePosition = {event.xbutton.x, event.xbutton.y};
                    NotifyInput(eventData);
                    break;
                }
                case ButtonRelease: {
                    EventData eventData;
                    eventData.type = EventType::MouseButtonReleased;
                    eventData.mouseButton = GetMouseButtonFromId(event.xbutton.button);
                    eventData.mousePosition = {event.xbutton.x, event.xbutton.y};
                    NotifyInput(eventData);
                    break;
                }
                case MotionNotify: {
                    EventData eventData;
                    eventData.type = EventType::MouseMoved;
                    eventData.mouseButton = GetMouseButtonFromId(event.xbutton.button);
                    eventData.mousePosition = {event.xbutton.x, event.xbutton.y};
                    NotifyInput(eventData);
                    break;
                }
                case ClientMessage: {
                    if (event.xclient.data.l[0] == mAtomWmDeleteWindow) {
                        NotifyClose();
                        break;
                    }
                }
                default:
                    break;
            }
        }
    }

    Window HwWindow::GetHnd() const {
        return mHnd;
    }

}


