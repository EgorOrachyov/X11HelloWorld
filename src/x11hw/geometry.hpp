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

#ifndef X11HELLOWORLD_GEOMETRY_HPP
#define X11HELLOWORLD_GEOMETRY_HPP

#include <GL/glew.h>
#include <vector>

namespace x11hw {

    class HwGeometry {
    public:

        struct Attribute {
            size_t offset;
            size_t components;
            GLenum baseType;
            bool normalize;
        };

        struct InitParams {
            size_t verticesCount = 0;
            size_t stride = 0;
            GLenum topology = 0;
            std::vector<Attribute> attributes;
        };

        explicit HwGeometry(const InitParams& params);
        ~HwGeometry();

        /**
         * Update vertex data of the geometry
         * @param offset Byte offset
         * @param size Byte size
         * @param vertexData Data to write
         */
        void Update(size_t offset, size_t size, const void *vertexData) const;

        /**
         * Issue geometry draw
         */
        void Draw() const;

        size_t GetBufferSize() const;

    private:
        size_t mVerticesCount = 0;
        size_t mStride = 0;
        GLenum mTopology = 0;
        GLuint mVAO = 0;
        GLuint mVBO = 0;
    };

}

#endif //X11HELLOWORLD_GEOMETRY_HPP