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
#include <x11hw/context.hpp>
#include <x11hw/window_manager.hpp>
#include <x11hw/error.hpp>

#include <stdexcept>
#include <utility>
#include <cassert>
#include <cstring>

#include <GL/glx.h>


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
          mDisplay(params.display),
          mScreen(params.screen),
          mContext(params.context) {
        assert(mDisplay);
        assert(mContext);
        assert(mSize.x > 0 & mSize.y > 0);
        CreateXWindow();
    }

    HwWindow::~HwWindow() {
        XDestroyWindow(mDisplay, mHnd);

        mHnd = 0;
        mScreen = 0;
        mDisplay = nullptr;
        mContext = nullptr;
    }

    void HwWindow::MakeContextCurrent() {
        mContext->MakeContextCurrent(mHnd);
    }

    void HwWindow::SwapBuffers() {
        mContext->SwapBuffers(mHnd);
    }

    void HwWindow::SetSwapInterval(int interval) {
        mContext->SetSwapInterval(mHnd, interval);
    }

    void HwWindow::CreateXWindow() {
        mEventMask =
            ButtonMotionMask   |
            ButtonPressMask    |
            ButtonReleaseMask  |
            ExposureMask       |
            StructureNotifyMask;

        auto visualInfo = mContext->GetVisualInfo();
        auto colorMap = mContext->GetColorMap();

        unsigned long windowAttributesMask =
            CWBackPixel  |
            CWColormap   |
            CWBorderPixel|
            CWEventMask  ;

        XSetWindowAttributes windowAttributes;
        windowAttributes.border_pixel = XBlackPixel(mDisplay, mScreen);
        windowAttributes.background_pixel = XWhitePixel(mDisplay, mScreen);
        windowAttributes.override_redirect = True;
        windowAttributes.colormap = colorMap;
        windowAttributes.event_mask = mEventMask;

        mHnd = XCreateWindow(
            mDisplay,
            XRootWindow(mDisplay, mScreen),
            0, 0,
            mSize.x, mSize.y,
            1,
            visualInfo->depth,
            InputOutput,
            visualInfo->visual,
            windowAttributesMask,
            &windowAttributes
        );

        CHECK_MSG(mHnd, "Failed to create window");

        // Events & name setup
        mAtomWmDeleteWindow = XInternAtom(mDisplay, "WM_DELETE_WINDOW", False);
        CHECK(XSetWMProtocols(mDisplay, mHnd, &mAtomWmDeleteWindow, 1));
        CHECK(XSelectInput(mDisplay, mHnd, mEventMask));
        CHECK(XStoreName(mDisplay, mHnd, mTitle.c_str()));

        // Show the window
        CHECK(XClearWindow(mDisplay, mHnd));
        CHECK(XMapRaised(mDisplay, mHnd));
    }

    void HwWindow::QueryFboSize() {
        mFramebufferSize = mSize;
    }

    void HwWindow::NotifyInput(const EventData &event) {
        for (auto& callback: mOnInputCallbacks) {
            callback(event);
        }
    }

    void HwWindow::NotifyClose() {
        for (auto& callback: mOnCloseCallbacks) {
            callback();
        }
    }

    void HwWindow::ProcessEvent(const XEvent& event) {
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
                QueryFboSize();
                break;
            }
            default:
                break;
        }
    }

    Window HwWindow::GetHnd() const {
        return mHnd;
    }

}


