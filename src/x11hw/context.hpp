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

#ifndef X11HELLOWORLD_CONTEXT_HPP
#define X11HELLOWORLD_CONTEXT_HPP

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>

namespace x11hw {

    class HwContext {
    public:
        static const int GLX_MAJOR_MIN = 1;
        static const int GLX_MINOR_MIN = 2;

        HwContext(const HwContext &) = delete;
        HwContext(HwContext &&) = delete;
        ~HwContext();

    private:
        friend class HwWindowManager;
        friend class HwWindow;

        HwContext(Display *display, int screen);

        void CreateContext();
        bool IsCreated();
        void MakeContextCurrent(Window window);
        void SwapBuffers(Window window);
        void SetSwapInterval(Window window, int interval);

        XVisualInfo *GetVisualInfo() const;
        GLXFBConfig GetFBConfig() const;
        Colormap GetColorMap() const;

    private:
        typedef void (*glXSwapIntervalEXT)(Display*,GLXDrawable,int);
        typedef int (*glXSwapIntervalSGI)(int);
        typedef int (*glXSwapIntervalMESA)(int);

        void ValidateGlxVersion();
        void SelectFBConfig();
        void CreateVisualInfo();

        int mScreen = -1;
        Display *mDisplay = nullptr;
        GLXContext mContext = nullptr;
        GLXFBConfig mFbConfig = nullptr;
        Colormap mColorMap{};
        XVisualInfo *mVisualInfo = nullptr;

        glXSwapIntervalEXT mglXSwapIntervalEXT = nullptr;
        glXSwapIntervalMESA mglXSwapIntervalMESA = nullptr;
        glXSwapIntervalSGI mglXSwapIntervalSGI = nullptr;

        bool mglXSwapIntervalEXTSupport = false;
        bool mglXSwapIntervalMESASupport = false;
        bool mglXSwapIntervalSGISupport = false;
    };

}

#endif //X11HELLOWORLD_CONTEXT_HPP