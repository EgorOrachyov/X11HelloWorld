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

#include <x11hw/geometry.hpp>
#include <cassert>

namespace x11hw {

    HwGeometry::HwGeometry(const InitParams &params) {
        assert(params.topology == GL_TRIANGLES);
        assert(params.stride > 0);
        assert(params.verticesCount > 0);
        assert(!params.attributes.empty());

        mTopology = params.topology;
        mStride = params.stride;
        mVerticesCount = params.verticesCount;

        glGenVertexArrays(1, &mVAO);
        glBindVertexArray(mVAO);

        glGenBuffers(1, &mVBO);
        glBindBuffer(GL_ARRAY_BUFFER, mVBO);
        glBufferData(GL_ARRAY_BUFFER, GetBufferSize(), nullptr, GL_STATIC_DRAW);

        for (size_t i = 0; i < params.attributes.size(); i++) {
            auto& attrib = params.attributes[i];

            glEnableVertexAttribArray(i);
            glVertexAttribDivisor(i, 0);
            glVertexAttribPointer(
                i,
                attrib.components,
                attrib.baseType,
                attrib.normalize ? GL_TRUE : GL_FALSE,
                mStride,
                (void *) attrib.offset
            );
        }

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    HwGeometry::~HwGeometry() {
        if (mVAO) {
            glDeleteVertexArrays(1, &mVAO);
            glDeleteBuffers(1, &mVBO);

            mVAO = 0;
            mVBO = 0;
            mStride = 0;
            mVerticesCount = 0;
        }
    }

    void HwGeometry::Update(size_t offset, size_t size, const void *vertexData) const {
        assert(offset + size <= GetBufferSize());
        glBindBuffer(GL_ARRAY_BUFFER, mVBO);
        glBufferSubData(GL_ARRAY_BUFFER, offset, size, vertexData);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void HwGeometry::Draw() const {
        glBindVertexArray(mVAO);
        glDrawArrays(mTopology, 0, mVerticesCount);
        glBindVertexArray(0);
    }

    size_t HwGeometry::GetBufferSize() const {
        return mStride * mVerticesCount;
    }

}