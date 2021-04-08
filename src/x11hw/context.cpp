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

#include <x11hw/context.hpp>
#include <x11hw/error.hpp>
#include <stdexcept>
#include <cstring>
#include <cassert>

namespace x11hw {

    static bool IsExtensionSupported(const char *extensions, const char *extension) {
        return std::strstr(extensions, extension) != nullptr;
    }

    HwContext::HwContext(Display *display, int screen) {
        assert(display);

        mDisplay = display;
        mScreen = screen;

        ValidateGlxVersion();
        SelectFBConfig();
        CreateVisualInfo();
    }

    HwContext::~HwContext() {
        if (IsCreated()) {
            glXDestroyContext(mDisplay, mContext);
            XFree(mVisualInfo);
            XFreeColormap(mDisplay, mColorMap);

            mContext = nullptr;
            mDisplay = nullptr;
            mFbConfig = nullptr;
            mVisualInfo = nullptr;
        }
    }

    void HwContext::ValidateGlxVersion() {
        GLint glxMajor, glxMinor;
        glXQueryVersion(mDisplay, &glxMajor, &glxMinor);

        if (glxMajor < GLX_MAJOR_MIN || (glxMajor == GLX_MAJOR_MIN && glxMinor < GLX_MINOR_MIN)) {
            throw std::runtime_error("GLX 1.2 or greater is required");
        }
    }

    void HwContext::SelectFBConfig() {
        GLint glxAttributes[] = {
                GLX_X_RENDERABLE, True,
                GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
                GLX_RENDER_TYPE, GLX_RGBA_BIT,
                GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
                GLX_RED_SIZE, 8,
                GLX_GREEN_SIZE, 8,
                GLX_BLUE_SIZE, 8,
                GLX_ALPHA_SIZE, 8,
                GLX_DEPTH_SIZE, 24,
                GLX_STENCIL_SIZE, 8,
                GLX_DOUBLEBUFFER, True,
                None
        };

        int fbConfigsCount = -1;
        GLXFBConfig* fbConfigs = glXChooseFBConfig(mDisplay, mScreen, glxAttributes, &fbConfigsCount);
        CHECK_MSG(fbConfigs && fbConfigsCount > 0, "Failed to retrieve framebuffer configs");

        int selectedConfigId = -1;
        int bestSamples = -1;

        for (int i = 0; i < fbConfigsCount; i++) {
            XVisualInfo *visualInfo = glXGetVisualFromFBConfig(mDisplay, fbConfigs[i]);

            if (visualInfo) {
                int sampleBuffers;
                int samples;

                glXGetFBConfigAttrib(mDisplay, fbConfigs[i], GLX_SAMPLE_BUFFERS, &sampleBuffers);
                glXGetFBConfigAttrib(mDisplay, fbConfigs[i], GLX_SAMPLES, &samples);

                if (selectedConfigId < 0 || (sampleBuffers && samples > bestSamples) ) {
                    selectedConfigId = i;
                    bestSamples = samples;
                }

                XFree(visualInfo);
            }
        }

        mFbConfig = fbConfigs[selectedConfigId];
        XFree(fbConfigs);
    }

    void HwContext::CreateVisualInfo() {
        mVisualInfo = glXGetVisualFromFBConfig(mDisplay, mFbConfig);
        CHECK_MSG(mVisualInfo, "Failed to create VisualInfo");
        CHECK_MSG(mVisualInfo->screen == mScreen, "VisualInfo screen does not match window screen");
        mColorMap = XCreateColormap(mDisplay, XRootWindow(mDisplay, mScreen), mVisualInfo->visual, AllocNone);
    }

    void HwContext::CreateContext() {
        // Query extensions
        const char *glxExtensions = glXQueryExtensionsString(mDisplay, mScreen);

        if (IsExtensionSupported(glxExtensions, "GLX_ARB_create_context")) {
            int contextAttributes[] = {
                    GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
                    GLX_CONTEXT_MINOR_VERSION_ARB, 2,
                    GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
                    None
            };

            typedef GLXContext (*glXCreateContextAttribsARBFunction)(
                    Display*,
                    GLXFBConfig,
                    GLXContext,
                    Bool,
                    const int*
            );

            auto glXCreateContextAttribsARB = (glXCreateContextAttribsARBFunction)
                    glXGetProcAddressARB((const GLubyte *) "glXCreateContextAttribsARB");
            CHECK_MSG(glXCreateContextAttribsARB, "Failed to get glXCreateContextAttribsARB function");

            mContext = glXCreateContextAttribsARB(mDisplay, mFbConfig, nullptr, true, contextAttributes);
        }
        else {
            // Fallback to simple setup
            mContext = glXCreateNewContext(mDisplay, mFbConfig, GLX_RGBA_TYPE, nullptr, True);
        }

        CHECK_MSG(mContext, "Failed to create GL context");
    }

    bool HwContext::IsCreated() {
        return mContext != nullptr;
    }

    void HwContext::MakeContextCurrent(Window window) {
        assert(IsCreated());
        CHECK(glXMakeCurrent(mDisplay, window, mContext));
    }

    XVisualInfo * HwContext::GetVisualInfo() const {
        return mVisualInfo;
    }

    GLXFBConfig HwContext::GetFBConfig() const {
        return mFbConfig;
    }

    Colormap HwContext::GetColorMap() const {
        return mColorMap;
    }

}