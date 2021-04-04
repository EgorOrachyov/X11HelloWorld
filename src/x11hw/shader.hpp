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

#ifndef X11HELLOWORLD_SHADER_HPP
#define X11HELLOWORLD_SHADER_HPP

#include <GL/glew.h>
#include <glm/matrix.hpp>
#include <string>

namespace x11hw {

    class HwShader {
    public:
        HwShader(const char* vertexCode, const char* fragmentCode);
        ~HwShader();

        /** Bind shader for drawing */
        void Bind();
        /** Unbind shader (default will be used for drawing) */
        void Unbind();

        /**
         * Set shader uniform variable
         * @param name Variable name in the shader
         * @param val Value to set
         */
        void SetFloat(const std::string& name, float val) const;

        /**
         * Set shader uniform variable
         * @param name Variable name in the shader
         * @param vec Vector to set
         */

        void SetVec2(const std::string& name, const glm::vec2& vec) const;

        /**
         * Set shader uniform variable
         * @param name Variable name in the shader
         * @param mat Matrix to set
         */
        void SetMatrix4(const std::string& name, const glm::mat4& mat) const;

    private:
        int GetLocation(const std::string& name) const;
        void ReleaseInternal();

    private:
        static const GLuint MAX_STAGES = 2;
        bool   mIsBound = false;
        size_t mStagesCount = 0;
        GLuint mProgram = 0;
        GLuint mStages[MAX_STAGES] = {};
    };

}

#endif //X11HELLOWORLD_SHADER_HPP