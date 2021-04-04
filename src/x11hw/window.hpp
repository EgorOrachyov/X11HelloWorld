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

#ifndef X11HELLOWORLD_WINDOW_HPP
#define X11HELLOWORLD_WINDOW_HPP

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>
#include <glm/vec2.hpp>
#include <functional>
#include <string>
#include <vector>

namespace x11hw {

    class HwWindow {
    public:
        enum class EventType {
            MouseButtonPressed,
            MouseButtonReleased,
            MouseMoved,
            Unknown
        };

        enum class MouseButton {
            Left,
            Right,
            Unknown
        };

        struct EventData {
            EventType type = EventType::Unknown;
            MouseButton mouseButton = MouseButton::Unknown;
            glm::ivec2 mousePosition{};
        };

        struct InitParams {
            std::string name;
            std::string title;
            glm::uvec2 size;
            class HwWindowManager* manager;
        };

        explicit HwWindow(InitParams& params);
        HwWindow(const HwWindow& other) = delete;
        HwWindow(HwWindow&& other) noexcept = delete;
        ~HwWindow();

        const std::string &GetName() const { return mName; };
        const std::string &GetTitle() const { return mTitle; }
        const glm::uvec2 &GetSize() const { return mSize; }
        const glm::uvec2 &GetFramebufferSize() const { return mFramebufferSize; }

        void MakeContextCurrent();
        void SwapBuffers();

        template<typename Callback>
        void SubscribeOnInput(Callback&& callback) {
            mOnInputCallbacks.emplace_back(std::forward<Callback>(callback));
        }

        template<typename Callback>
        void SubscribeOnClose(Callback&& callback) {
            mOnCloseCallbacks.emplace_back(std::forward<Callback>(callback));
        }

    private:
        static const int GLX_MAJOR_MIN = 1;
        static const int GLX_MINOR_MIN = 2;

        friend class HwWindowManager;
        void NotifyInput(const EventData& event);
        void NotifyClose();
        void CheckEvents();
        Window GetHnd() const;

        std::string mName;
        std::string mTitle;
        glm::uvec2 mSize;
        glm::uvec2 mFramebufferSize{};

        long mEventMask = 0;
        Atom mAtomWmDeleteWindow{};
        Window mHnd{};
        Display* mDisplay = nullptr;
        XVisualInfo* mVisualInfo = nullptr;
        GLXContext mContext = nullptr;
        Colormap mColorMap;
        int mScreen;

        HwWindowManager* mManager;

        std::vector<std::function<void(const EventData& event)>> mOnInputCallbacks;
        std::vector<std::function<void()>> mOnCloseCallbacks;
    };

}

#endif //X11HELLOWORLD_WINDOW_HPP