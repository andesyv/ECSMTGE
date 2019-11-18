#include "postprocessor.h"
#include "renderer.h"

Postprocessor::Postprocessor(Renderer *renderer)
{
    mRenderer = renderer;
    // init();
}

void Postprocessor::init()
{
    if (mRenderer != nullptr && mRenderer->mContext->makeCurrent(mRenderer))
    {
        initializeOpenGLFunctions();

        // This also creates buffers, btw.
        updateRatio();

        // Create renderquad
        float quadVertices[] = {
            //    positions   texture Coords
            -1.0f,  -1.0f, 0.0f, // 0.0f, 0.0f,
            1.0f, -1.0f, 0.0f, // 1.0f, 0.0f,
            -1.0f,  1.0f, 0.0f, // 0.0f, 1.0f,
            -1.0f, 1.0f, 0.0f, // 1.0f, 0.0f,
            1.0f,  -1.0f, 0.0f, // 0.0f, 1.0f,
            1.0f, 1.0f, 0.0f // 1.f, 1.0f
        };
        // plane VAO setup
        GLuint mQuadVBO;
        glGenVertexArrays(1, &mScreenSpacedQuadVAO);
        glGenBuffers(1, &mQuadVBO);
        glBindVertexArray(mScreenSpacedQuadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, mQuadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
//        glEnableVertexAttribArray(1);
//        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

        mRenderer->checkForGLerrors();

        mInitialized = true;
    }
    else
    {
        std::cout << "Postprocessor init failed because invalid renderer reference or missing opengl context" << std::endl;
    }
}

GLuint Postprocessor::input() const
{
    return mPingpong[0];
}

GLuint Postprocessor::output() const
{
    return mPingpong[mLastUsedBuffer];
}

Postprocessor &Postprocessor::add(Postprocessor &other, Postprocessor::BLENDMODE blendmode)
{
    glBindFramebuffer(GL_FRAMEBUFFER, mPingpong[mLastUsedBuffer]);

    auto shader = ResourceManager::instance().getShader("blend");
    if (!shader)
    {
        std::cout << "blending shader is missing!" << std::endl;
        return *this;
    }

    shader->use();


    // Bind to framebuffer texture
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(shader->getProgram(), "fbt"), 0);
    glBindTexture(GL_TEXTURE_2D, mRenderTextures[mLastUsedBuffer]);

    glActiveTexture(GL_TEXTURE1);
    glUniform1i(glGetUniformLocation(shader->getProgram(), "fbt2"), 1);
    glBindTexture(GL_TEXTURE_2D, other.mRenderTextures[other.mLastUsedBuffer]);

    glUniform1i(glGetUniformLocation(shader->getProgram(), "blendmode"), blendmode);


    renderQuad();

    return *this;
}

Postprocessor &Postprocessor::operator+=(Postprocessor &other)
{
    return add(other);
}

void Postprocessor::Render()
{
    if (!mInitialized)
        init();
    else if (outdatedRatio())
        updateRatio();

    // Reset so that we start at the first ping-pong buffer
    mLastUsedBuffer = 0;

    if (!steps.empty() && !passThroughMaterial)
    {
        for (auto setting = steps.begin(); setting != steps.end(); ++setting, mLastUsedBuffer = !mLastUsedBuffer)
        {
            auto nextFramebuffer = (outputToDefault && setting + 1 == steps.end()) ? 0 : mPingpong[!mLastUsedBuffer];
            // First, blit over depth and stencil buffer
            glBindFramebuffer(GL_READ_FRAMEBUFFER, mPingpong[mLastUsedBuffer]);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, nextFramebuffer);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            glBlitFramebuffer(0, 0, mScrWidth, mScrHeight, 0, 0, mScrWidth, mScrHeight, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);

            // If last render, use default framebuffer; else use the one we didn't use last.
            glBindFramebuffer(GL_FRAMEBUFFER, nextFramebuffer);

            auto material = setting->material;
            if (!material || !material->mShader)
            {
                qDebug() << "using pass through shader";
                material = passThroughMaterial;
            }

            material->mShader->use();

            // Bind to framebuffer texture
            glActiveTexture(GL_TEXTURE0);
            // qDebug() << "uniform: " << glGetUniformLocation(shader->getProgram(), "fbt");
            glUniform1i(glGetUniformLocation(material->mShader->getProgram(), "fbt"), 0);
            glBindTexture(GL_TEXTURE_2D, mRenderTextures[mLastUsedBuffer]);

            /** NB: Depth sampling in shadercode won't work unless in OpenGL 4.4 because of how they're stored.
             * Should be possible to implement a depth/stencil sampling if they are implemented as
             * separate buffers.
             */
            if (depthSampling)
            {
                glActiveTexture(GL_TEXTURE1);
                glUniform1i(glGetUniformLocation(material->mShader->getProgram(), "depthBuffer"), 1);
                glBindTexture(GL_TEXTURE_2D, mDepthStencilBuffer[mLastUsedBuffer]);
            }

            int uniform = glGetUniformLocation(material->mShader->getProgram(), "sResolution");
            if (0 <= uniform)
                glUniform2i(uniform, mScrWidth, mScrHeight);

            uniform = glGetUniformLocation(material->mShader->getProgram(), "sTime");
            if (0 <= uniform)
                glUniform1f(uniform, mRenderer->mTimeSinceStart);

            mRenderer->evaluateParams(*material);
            renderQuad();
        }
    }
    else
    {
        if (!passThroughMaterial)
        {
            passThroughMaterial = std::make_shared<Material>(ResourceManager::instance().getShader("passthrough"));
            if (!passThroughMaterial->mShader)
                std::cout << "Postprocessor render step skipped because of missing passthrough shader" << std::endl;
        }

        if (outputToDefault)
        {
            // If no postprocess steps, just blit framebuffer onto default framebuffer.
            glBindFramebuffer(GL_READ_FRAMEBUFFER, mPingpong[0]);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glBlitFramebuffer(0, 0, mScrWidth, mScrHeight, 0, 0, mScrWidth, mScrHeight,
                              GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned int Postprocessor::RenderStep(unsigned int index)
{
    if (index == 0)
    {
        if (!mInitialized)
            init();
        else if (outdatedRatio())
            updateRatio();

        // Reset so that we start at the first ping-pong buffer
        mLastUsedBuffer = 0;
    }

    if (!steps.empty() && !passThroughMaterial)
    {
        auto setting = steps.begin() + index;
        if (setting != steps.end())
        {
            auto nextFramebuffer = (outputToDefault && setting + 1 == steps.end()) ? 0 : mPingpong[!mLastUsedBuffer];
            // First, blit over depth and stencil buffer
            glBindFramebuffer(GL_READ_FRAMEBUFFER, mPingpong[mLastUsedBuffer]);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, nextFramebuffer);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            glBlitFramebuffer(0, 0, mScrWidth, mScrHeight, 0, 0, mScrWidth, mScrHeight, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);

            // If last render, use default framebuffer; else use the one we didn't use last.
            glBindFramebuffer(GL_FRAMEBUFFER, nextFramebuffer);


            auto material = setting->material;
            if (!material || !material->mShader) {
                qDebug() << "using pass through shader";
                material = passThroughMaterial;
            }

            material->mShader->use();


            // Bind to framebuffer texture
            glActiveTexture(GL_TEXTURE0);
            glUniform1i(glGetUniformLocation(material->mShader->getProgram(), "fbt"), 0);
            glBindTexture(GL_TEXTURE_2D, mRenderTextures[mLastUsedBuffer]);

            /** NB: Depth sampling in shadercode won't work unless in OpenGL 4.4 because of how they're stored.
             * Should be possible to implement a depth/stencil sampling if they are implemented as
             * separate buffers.
             */
            if (depthSampling)
            {
                glActiveTexture(GL_TEXTURE1);
                glUniform1i(glGetUniformLocation(material->mShader->getProgram(), "depthBuffer"), 1);
                glBindTexture(GL_TEXTURE_2D, mDepthStencilBuffer[mLastUsedBuffer]);
            }

            int uniform = glGetUniformLocation(material->mShader->getProgram(), "sResolution");
            if (0 <= uniform)
                glUniform2i(uniform, mScrWidth, mScrHeight);

            uniform = glGetUniformLocation(material->mShader->getProgram(), "sTime");
            if (0 <= uniform)
                glUniform1f(uniform, mRenderer->mTimeSinceStart);

            mRenderer->evaluateParams(*material);

            renderQuad();

            ++index;
            mLastUsedBuffer = !mLastUsedBuffer;

            return index;
        }
        else
        {
            return index;
        }
    }
    else
    {
        if (!passThroughMaterial)
        {
            passThroughMaterial = std::make_shared<Material>(ResourceManager::instance().getShader("passthrough"));
            if (!passThroughMaterial->mShader)
                std::cout << "Postprocessor render step skipped because of missing default shader" << std::endl;
        }

        if (outputToDefault)
        {
            // If no postprocess steps, just blit framebuffer onto default framebuffer.
            glBindFramebuffer(GL_READ_FRAMEBUFFER, mPingpong[0]);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glBlitFramebuffer(0, 0, mScrWidth, mScrHeight, 0, 0, mScrWidth, mScrHeight,
                              GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |  GL_STENCIL_BUFFER_BIT, GL_NEAREST);
        }
        return index;
    }
}

void Postprocessor::clear()
{
    if (!mInitialized)
        init();
    else if (outdatedRatio())
        updateRatio();

    glBindFramebuffer(GL_FRAMEBUFFER, mPingpong[0]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Reset so that we start at the first ping-pong buffer
    mLastUsedBuffer = 0;
}

void Postprocessor::renderQuad()
{
    if (!mInitialized)
        init();
    else if (outdatedRatio())
        updateRatio();

    glBindVertexArray(mScreenSpacedQuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

bool Postprocessor::outdatedRatio() const
{
    return mRenderer->width() != mScrWidth || mRenderer->height() != mScrHeight;
}

void Postprocessor::updateRatio()
{
    mScrWidth = mRenderer->width();
    mScrHeight = mRenderer->height();

    recreateBuffers();

    glViewport(0, 0, mScrWidth, mScrHeight);

}

void Postprocessor::recreateBuffers()
{
    if (mInitialized)
    {
        glDeleteFramebuffers(2, mPingpong);
        glDeleteTextures(2, mRenderTextures);
        if (depthStencilUsingRenderbuffer)
        {
            glDeleteRenderbuffers(2, mDepthStencilBuffer);
        }
        else
        {
            glDeleteTextures(2, mDepthStencilBuffer);
        }
    }

    glGenFramebuffers(2, mPingpong);
    glGenTextures(2, mRenderTextures);

    if (depthSampling)
    {
        glGenTextures(2, mDepthStencilBuffer);
        depthStencilUsingRenderbuffer = false;
    }
    else
    {
        glGenRenderbuffers(2, mDepthStencilBuffer);
        depthStencilUsingRenderbuffer = true;
    }

    for (unsigned int i{0}; i < 2; ++i)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, mPingpong[i]);
        glBindTexture(GL_TEXTURE_2D, mRenderTextures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mScrWidth, mScrHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mRenderTextures[i], 0);


        if (depthStencilUsingRenderbuffer)
        {
            glBindRenderbuffer(GL_RENDERBUFFER, mDepthStencilBuffer[i]);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, mScrWidth, mScrHeight);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mDepthStencilBuffer[i]);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, mDepthStencilBuffer[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_STENCIL, mScrWidth, mScrHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, mDepthStencilBuffer[i], 0);
        }


        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cout << "Postprocessor framebuffer failed to be created!" << std::endl;
            glDeleteFramebuffers(2, mPingpong);
            glDeleteTextures(2, mRenderTextures);
            return;
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Postprocessor::~Postprocessor()
{
    if (mInitialized)
    {
        glDeleteFramebuffers(2, mPingpong);
        glDeleteTextures(2, mRenderTextures);
        glDeleteVertexArrays(1, &mScreenSpacedQuadVAO);
        mInitialized = false;
    }
}
