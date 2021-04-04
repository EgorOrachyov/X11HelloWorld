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
#include <GL/glx.h>
#include <stdexcept>
#include <utility>
#include <cassert>

#define CHECK_MSG(condition, message) \
    if (!(condition)) { throw std::runtime_error("X11 Window: \"" #condition "\":" message); } else { }

#define CHECK(condition) CHECK_MSG(condition, "")

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

        // Setup connection to the server
        {
            mDisplay = XOpenDisplay(nullptr);
            CHECK_MSG(mDisplay, "Failed to create Display");

            mScreen = XDefaultScreen(mDisplay);
        }

        // Query glx info and prepare visuals
        {
            GLint glxMajor, glxMinor;
            glXQueryVersion(mDisplay, &glxMajor, &glxMinor);

            if (glxMajor < GLX_MAJOR_MIN || (glxMajor == GLX_MAJOR_MIN && glxMinor < GLX_MINOR_MIN)) {
                throw std::runtime_error("GLX 1.2 or greater is required");
            }

            GLint glxAttributes[] = {
                GLX_RGBA,
                GLX_DOUBLEBUFFER,
                GLX_DEPTH_SIZE,     24,
                GLX_STENCIL_SIZE,   8,
                GLX_RED_SIZE,       8,
                GLX_GREEN_SIZE,     8,
                GLX_BLUE_SIZE,      8,
                GLX_SAMPLE_BUFFERS, 0,
                GLX_SAMPLES,        0,
                None
            };

            mVisualInfo = glXChooseVisual(mDisplay, mScreen, glxAttributes);
            CHECK_MSG(mVisualInfo, "Failed to create VisualInfo");
        }

        mEventMask =
           ButtonMotionMask   |
           ButtonPressMask    |
           ButtonReleaseMask  |
           ExposureMask       |
           StructureNotifyMask;

        mColorMap = XCreateColormap(mDisplay, XRootWindow(mDisplay, mScreen), mVisualInfo->visual, AllocNone);

        unsigned long windowAttributesMask =
           CWBackPixel  |
           CWColormap   |
           CWBorderPixel|
           CWEventMask  ;
        XSetWindowAttributes windowAttributes;
        windowAttributes.border_pixel = XBlackPixel(mDisplay, mScreen);
        windowAttributes.background_pixel = XWhitePixel(mDisplay, mScreen);
        windowAttributes.override_redirect = True;
        windowAttributes.colormap = mColorMap;
        windowAttributes.event_mask = mEventMask;

        mHnd = XCreateWindow(mDisplay, XRootWindow(mDisplay, mScreen), 0, 0, mSize.x, mSize.y, 1, mVisualInfo->depth, InputOutput, mVisualInfo->visual, windowAttributesMask, &windowAttributes);
        CHECK_MSG(mHnd, "Failed to create window");

        mContext = glXCreateContext(mDisplay, mVisualInfo, NULL, GL_TRUE);
        CHECK_MSG(mContext, "Failed to create GL context");

        MakeContextCurrent();

        // Setup
        mAtomWmDeleteWindow = XInternAtom(mDisplay, "WM_DELETE_WINDOW", False);
        CHECK(XSetWMProtocols(mDisplay, mHnd, &mAtomWmDeleteWindow, 1));
        CHECK(XSelectInput(mDisplay, mHnd, mEventMask));
        CHECK(XStoreName(mDisplay, mHnd, mTitle.c_str()));

        // Show the window
        CHECK(XClearWindow(mDisplay, mHnd));
        CHECK(XMapRaised(mDisplay, mHnd));
    }

    HwWindow::~HwWindow() {
        glXDestroyContext(mDisplay, mContext);
        XFreeColormap(mDisplay, mColorMap);
        XFree(mVisualInfo);
        XDestroyWindow(mDisplay, mHnd);
        XCloseDisplay(mDisplay);

        mHnd = 0;
        mScreen = 0;
        mDisplay = nullptr;
    }

    void HwWindow::MakeContextCurrent() {
        CHECK(glXMakeCurrent(mDisplay, mHnd, mContext));
    }

    void HwWindow::SwapBuffers() {
        glXSwapBuffers(mDisplay, mHnd);
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
                    if (event.xclient.data.l[0] == (long) mAtomWmDeleteWindow) {
                        NotifyClose();
                        break;
                    }
                }
                case ConfigureNotify: {
                    XConfigureEvent xce = event.xconfigure;
                    mSize = { xce.width, xce.height};
                    break;
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


