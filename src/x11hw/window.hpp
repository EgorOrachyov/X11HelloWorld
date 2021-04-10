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

        HwWindow(const HwWindow &) = delete;
        HwWindow(HwWindow &&) noexcept = delete;
        ~HwWindow();

        /** Makes GL context of this window current for drawing */
        void MakeContextCurrent();

        /** Preset back-buffer content to the screen */
        void SwapBuffers();

        /** Sets swap interval */
        void SetSwapInterval(int interval);

        /**
         * Add listener for window input events (requested when user presses red x button)
         * @tparam Callback Type of a function to call
         * @param callback The function to call
         */
        template<typename Callback>
        void SubscribeOnInput(Callback &&callback) {
            mOnInputCallbacks.emplace_back(std::forward<Callback>(callback));
        }

        /**
         * Add listener for window close events (requested when user presses red x button)
         * @tparam Callback Type of a function to call
         * @param callback The function to call
         */
        template<typename Callback>
        void SubscribeOnClose(Callback &&callback) {
            mOnCloseCallbacks.emplace_back(std::forward<Callback>(callback));
        }

        /** @return Window Name (id) */
        const std::string &GetName() const { return mName; };

        /** @return Window title (shown on screen) */
        const std::string &GetTitle() const { return mTitle; }

        /** @return Window size (in units) */
        const glm::uvec2 &GetSize() const { return mSize; }

        /** @return Framebuffer size (in pixels) */
        const glm::uvec2 &GetFramebufferSize() const { return mFramebufferSize; }

    private:
        friend class HwWindowManager;

        struct InitParams {
            std::string name;
            std::string title;
            glm::uvec2 size;
            Display *display;
            int screen;
            class HwContext *context;
        };

        explicit HwWindow(InitParams &params);

        void CreateXWindow();
        void QueryFboSize();
        void NotifyInput(const EventData &event);
        void NotifyClose();
        void ProcessEvent(const XEvent &event);

        Window GetHnd() const;

    private:
        std::string mName;
        std::string mTitle;
        glm::uvec2 mSize;
        glm::uvec2 mFramebufferSize{};

        long mEventMask = 0;
        Atom mAtomWmDeleteWindow{};
        Window mHnd{};
        int mScreen = -1;
        Display *mDisplay = nullptr;

        class HwContext *mContext;

        std::vector<std::function<void()>> mOnCloseCallbacks;
        std::vector<std::function<void(const EventData &event)>> mOnInputCallbacks;
    };

}

#endif //X11HELLOWORLD_WINDOW_HPP