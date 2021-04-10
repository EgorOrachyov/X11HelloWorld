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

#ifndef X11HELLOWORLD_WINDOW_MANAGER_HPP
#define X11HELLOWORLD_WINDOW_MANAGER_HPP

#include <X11/Xlib.h>
#include <glm/vec2.hpp>
#include <unordered_map>
#include <memory>

namespace x11hw {

    class HwWindowManager {
    public:
        HwWindowManager();
        HwWindowManager(const HwWindowManager&) = delete;
        HwWindowManager(HwWindowManager&&) noexcept = delete;
        ~HwWindowManager();

        /**
         * Create new window
         *
         * @param name Unique window identifier
         * @param title Window title
         * @param size Window size in units
         *
         * @return Created window ptr
         */
        class HwWindow* CreateWindow(std::string name, std::string title, glm::uvec2 size);

        /** Process system events (call for each update tick for smooth response) */
        void PollEvents();

        /**
         * Check if window is presented.
         * @param name Window unique name
         * @return True if has window
         */
        bool ContainsWindow(const std::string& name) const;

        /**
         * Find window by name.
         * @param name Window unique name
         * @return Window or null if failed to find
         */
        class HwWindow* GetWindow(const std::string& name);

    private:
        friend class HwWindow;

        std::unordered_map<std::string, std::unique_ptr<class HwWindow>> mWindows;
        std::unordered_map<Window, class HwWindow*> mX11Windows;
        std::unique_ptr<class HwContext> mContext;

        Display* mDisplay = nullptr;
        int mScreen = -1;
    };

}

#endif //X11HELLOWORLD_WINDOW_MANAGER_HPP