// passes/passes.hpp

#pragma once

#include <bolero.hpp>


namespace blr::core
{


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline
Ref<RenderPass> CreateIrradiancePass(AssetManager& assetManager, uint32_t size = 64)
{
    struct PassData
    {
        Ref<Shader> shader;
        Ref<Cubemap> irradianceMap;
        GLuint fboID = 0;
        uint32_t size;
        bool hasExecuted = false;
    };
    auto d = std::make_shared<PassData>();
    d->size = size;

    return std::make_shared<RenderPass>("Irradiance Convolution",
        // ===== INIT =====
        [d, &assetManager](RenderPass& self)
        {
            d->shader = assetManager.CreateShader("bolero://shaders/cubemap_convolution.glsl");

            TexSpec spec;
            spec.w            = d->size;
            spec.h            = d->size;
            spec.format       = ImgFmt::RGB16F;
            spec.generateMips = false;
            spec.minFilter    = TexFilter::LinearMipmapLinear;
            spec.magFilter    = TexFilter::Linear;
            spec.wrapS        = TexWrap::ClampToEdge;
            spec.wrapT        = TexWrap::ClampToEdge;
            d->irradianceMap  = Cubemap::Create(spec);

            glCreateFramebuffers(1, &d->fboID);
            glNamedFramebufferTexture(d->fboID, GL_COLOR_ATTACHMENT0, d->irradianceMap->GetID(), 0);
        },
        // ===== EXECUTE =====
        [d](Scene& scene, RenderContext& ctx, RenderPass& self)
        {
            if (d->hasExecuted)
            {
                ctx.Set("u_IrradianceMap", d->irradianceMap->GetID());
                return;
            }

            glBindFramebuffer(GL_FRAMEBUFFER, d->fboID);
            glViewport(0, 0, d->size, d->size);

            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);

            d->shader->Bind();

            mat4 captureProj = Perspective(DegToRad(90.0f), 1.0f, 0.1f, 10.0f);
            mat4 captureViews[] = {
                LookAt(vec3(0.0f), vec3( 1.0f,  0.0f,  0.0f), vec3(0.0f, -1.0f,  0.0f)),
                LookAt(vec3(0.0f), vec3(-1.0f,  0.0f,  0.0f), vec3(0.0f, -1.0f,  0.0f)),
                LookAt(vec3(0.0f), vec3( 0.0f,  1.0f,  0.0f), vec3(0.0f,  0.0f,  1.0f)),
                LookAt(vec3(0.0f), vec3( 0.0f, -1.0f,  0.0f), vec3(0.0f,  0.0f, -1.0f)),
                LookAt(vec3(0.0f), vec3( 0.0f,  0.0f,  1.0f), vec3(0.0f, -1.0f,  0.0f)),
                LookAt(vec3(0.0f), vec3( 0.0f,  0.0f, -1.0f), vec3(0.0f, -1.0f,  0.0f))};
            
            for (size_t i = 0; i < 6; i++)
                d->shader->SetMat4("u_ViewProjMatrices[" + std::to_string(i) + "]", captureProj * captureViews[i]);

            glBindTextureUnit(LayoutLoc::ENV_CUBE_MAP, ctx.Get<GLuint>("u_EnvMap"));

            Renderer::DrawCube();

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            d->hasExecuted = true;


            ctx.Set("u_IrradianceMap", d->irradianceMap->GetID());
        }
    );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline
Ref<RenderPass> CreatePrefilterPass(AssetManager& assetManager, uint32_t size = 512, uint32_t samples = 4096)
{
    struct PassData
    {
        Ref<Shader> shader;
        Ref<Cubemap> prefilteredMap;
        GLuint fboID = 0;
        uint32_t size;
        uint32_t samples;
        bool hasExecuted = false;
    };
    auto d = std::make_shared<PassData>();
    d->size = size;
    d->samples = samples;

    return std::make_shared<RenderPass>("Prefilter Pass",
        // ===== INIT =====
        [d, &assetManager](RenderPass& self)
        {
            d->shader = assetManager.CreateShader("bolero://shaders/prefilter_pass.glsl");

            TexSpec spec;
            spec.w            = d->size;
            spec.h            = d->size;
            spec.format       = ImgFmt::RGB16F;
            spec.generateMips = true;
            spec.minFilter    = TexFilter::LinearMipmapLinear;
            spec.magFilter    = TexFilter::Linear;
            spec.wrapS        = TexWrap::ClampToEdge;
            spec.wrapT        = TexWrap::ClampToEdge;
            d->prefilteredMap = Cubemap::Create(spec);

            glCreateFramebuffers(1, &d->fboID);
            glNamedFramebufferTexture(d->fboID, GL_COLOR_ATTACHMENT0, d->prefilteredMap->GetID(), 0);
        },
        // ===== EXECUTE =====
        [d](Scene& scene, RenderContext& ctx, RenderPass& self)
        {
            if (d->hasExecuted)
            {
                ctx.Set("u_PrefilterMap", d->prefilteredMap->GetID());
                return;
            }

            glBindFramebuffer(GL_FRAMEBUFFER, d->fboID);

            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

            d->shader->Bind();

            d->shader->SetUInt("u_Samples", d->samples);
            d->shader->SetUInt("u_EnvMapRes", d->size);

            mat4 captureProj = Perspective(DegToRad(90.0f), 1.0f, 0.1f, 10.0f);
            mat4 captureViews[] = {
                LookAt(vec3(0.0f), vec3( 1.0f,  0.0f,  0.0f), vec3(0.0f, -1.0f,  0.0f)),
                LookAt(vec3(0.0f), vec3(-1.0f,  0.0f,  0.0f), vec3(0.0f, -1.0f,  0.0f)),
                LookAt(vec3(0.0f), vec3( 0.0f,  1.0f,  0.0f), vec3(0.0f,  0.0f,  1.0f)),
                LookAt(vec3(0.0f), vec3( 0.0f, -1.0f,  0.0f), vec3(0.0f,  0.0f, -1.0f)),
                LookAt(vec3(0.0f), vec3( 0.0f,  0.0f,  1.0f), vec3(0.0f, -1.0f,  0.0f)),
                LookAt(vec3(0.0f), vec3( 0.0f,  0.0f, -1.0f), vec3(0.0f, -1.0f,  0.0f))};
    
            for (size_t i = 0; i < 6; i++)
                d->shader->SetMat4("u_ViewProjMatrices[" + std::to_string(i) + "]", captureProj * captureViews[i]);

            glBindTextureUnit(LayoutLoc::ENV_CUBE_MAP, ctx.Get<GLuint>("u_EnvMap"));

            uint32_t maxMip = d->prefilteredMap->GetSpec().numMips;
            for (size_t mip = 0; mip < maxMip; mip++)
            {
                unsigned int mipWidth  = d->size * std::pow(0.5, mip);
                unsigned int mipHeight = d->size * std::pow(0.5, mip);
                glViewport(0, 0, mipWidth, mipHeight);

                float roughness = (float)mip / (float)(maxMip - 1);
                d->shader->SetFloat("u_Roughness", roughness);

                glNamedFramebufferTexture(d->fboID, GL_COLOR_ATTACHMENT0, d->prefilteredMap->GetID(), mip);

                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                Renderer::DrawCube();
            }

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            d->hasExecuted = true;


            ctx.Set("u_PrefilterMap", d->prefilteredMap->GetID());
        }
    );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline
Ref<RenderPass> CreateBrdfLutPass(AssetManager& assetManager, uint32_t size = 512, uint32_t samples = 2048)
{
    struct PassData
    {
        Ref<Shader> shader;
        Ref<Tex> brdfTex;
        GLuint fboID = 0;
        uint32_t size;
        uint32_t samples;
        bool hasExecuted = false;
    };
    auto d = std::make_shared<PassData>();
    d->size = size;
    d->samples = samples;

    return std::make_shared<RenderPass>("BRDF Pre Computation Pass",
        // ===== INIT =====
        [d, &assetManager](RenderPass& self)
        {
            d->shader = assetManager.CreateShader("bolero://shaders/brdf_lut.glsl");

            TexSpec spec;
            spec.w            = d->size;
            spec.h            = d->size;
            spec.format       = ImgFmt::RG16F;
            spec.generateMips = false;
            spec.wrapS        = TexWrap::ClampToEdge;
            spec.wrapT        = TexWrap::ClampToEdge;
            spec.minFilter    = TexFilter::Linear;
            spec.magFilter    = TexFilter::Linear;
            d->brdfTex = assetManager.CreateTex(spec);

            glCreateFramebuffers(1, &d->fboID);
            glNamedFramebufferTexture(d->fboID, GL_COLOR_ATTACHMENT0, d->brdfTex->GetID(), 0);
        },
        // ===== EXECUTE =====
        [d](Scene& scene, RenderContext& ctx, RenderPass& self)
        {
            if (d->hasExecuted)
            {
                ctx.Set("u_BrdfLut", d->brdfTex->GetID());
                return;
            }

            glBindFramebuffer(GL_FRAMEBUFFER, d->fboID);
            glViewport(0, 0, d->size, d->size);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            d->shader->Bind();

            d->shader->SetUInt("u_Samples", d->samples);

            Renderer::DrawFullscreenQuad();

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            d->hasExecuted = true;


            ctx.Set("u_BrdfLut", d->brdfTex->GetID());
        }
    );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline
Ref<RenderPass> CreateIBLSetupPass(AssetManager& assetManager, uint32_t size = 512)
{
    struct PassData
    {
        Ref<Shader> eqToCubeShader;
        Ref<Tex> hdrMap;
        Ref<Cubemap> envCubemap;
        GLuint fboID = 0;
        uint32_t size;
        bool hasExecuted = false;
    };
    auto d = std::make_shared<PassData>();
    d->size = size;

    return std::make_shared<RenderPass>("IBL Setup",
        // ===== INIT =====
        [d, &assetManager](RenderPass& self)
        {
            d->eqToCubeShader = assetManager.CreateShader("bolero://shaders/equirect_to_cubemap_pass.glsl");
            d->hdrMap         = assetManager.CreateTex("bolero://hdri/citrus_orchard_puresky_2k.hdr");
            
            TexSpec spec;
            spec.w            = d->size;
            spec.h            = d->size;
            spec.format       = ImgFmt::RGB16F; 
            spec.generateMips = true;
            spec.minFilter    = TexFilter::LinearMipmapLinear;
            spec.magFilter    = TexFilter::Linear;
            spec.wrapS        = TexWrap::ClampToEdge;
            spec.wrapT        = TexWrap::ClampToEdge;
            d->envCubemap = Cubemap::Create(spec);

            glCreateFramebuffers(1, &d->fboID);
            glNamedFramebufferTexture(d->fboID, GL_COLOR_ATTACHMENT0, d->envCubemap->GetID(), 0);
        },
        // ===== EXECUTE =====
        [d](Scene& scene, RenderContext& ctx, RenderPass& self)
        {
            /* EXECUTION FLAG CHECK */
            if (d->hasExecuted)
            {
                ctx.Set("u_EnvMap", d->envCubemap->GetID());
                return;
            }


            /* FBO BINDING */
            glBindFramebuffer(GL_FRAMEBUFFER, d->fboID);
            glViewport(0, 0, d->size, d->size);


            /* GL STATES */
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);


            /* SHADER */
            d->eqToCubeShader->Bind();

            // Equirect To Cubemap
            mat4 captureProj = Perspective(DegToRad(90.0f), 1.0f, 0.1f, 10.0f);
            mat4 captureViews[] = {
                    LookAt(vec3(0.0f), vec3( 1.0f,  0.0f,  0.0f), vec3(0.0f, -1.0f,  0.0f)),
                    LookAt(vec3(0.0f), vec3(-1.0f,  0.0f,  0.0f), vec3(0.0f, -1.0f,  0.0f)),
                    LookAt(vec3(0.0f), vec3( 0.0f,  1.0f,  0.0f), vec3(0.0f,  0.0f,  1.0f)),
                    LookAt(vec3(0.0f), vec3( 0.0f, -1.0f,  0.0f), vec3(0.0f,  0.0f, -1.0f)),
                    LookAt(vec3(0.0f), vec3( 0.0f,  0.0f,  1.0f), vec3(0.0f, -1.0f,  0.0f)),
                    LookAt(vec3(0.0f), vec3( 0.0f,  0.0f, -1.0f), vec3(0.0f, -1.0f,  0.0f))};

            for (size_t i = 0; i < 6; i++)
                d->eqToCubeShader->SetMat4("u_ViewProjMatrices[" + std::to_string(i) + "]", captureProj * captureViews[i]);
            d->eqToCubeShader->SetInt("u_EquirectangularMap", 0);

            d->hdrMap->Bind(0);

            /* DRAW CALL */
            Renderer::DrawCube();

            // Unbind
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // Generate Mipmaps
            glGenerateTextureMipmap(d->envCubemap->GetID());


            /* EXECUTION FLAG SETTING */
            d->hasExecuted = true;


            /* RENDER CONTEXT POPULATION */
            ctx.Set("u_EnvMap", d->envCubemap->GetID());
        }
    );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline
Ref<RenderPass> CreateSkyboxPass(AssetManager& assetManager)
{
    struct PassData
    {
        Ref<Shader> shader;
    };
    auto d = std::make_shared<PassData>();

    return std::make_shared<RenderPass>("Skybox Pass",
        // ===== INIT =====
        [d, &assetManager](RenderPass& self)
        {
            d->shader = assetManager.CreateShader("bolero://shaders/skybox.glsl");
        },
        // ===== EXECUTE =====
        [d](Scene& scene, RenderContext& ctx, RenderPass& self)
        {
            GLuint sceneFbo = ctx.Get<GLuint>("SCENE_FBO_ID", 0);
            GLuint prefilterMap = ctx.Get<GLuint>("u_PrefilterMap", 0);
            if (sceneFbo == 0 || prefilterMap == 0)
                return;

            uint32_t targetW = Renderer::GetViewportWidth();
            uint32_t targetH = Renderer::GetViewportHeight();
            uint32_t scale   = Renderer::GetRenderScale();

            glBindFramebuffer(GL_FRAMEBUFFER, sceneFbo);
            glViewport(0, 0, targetW * scale, targetH * scale);

            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);

            d->shader->Bind();

            glBindTextureUnit(LayoutLoc::PREFILTER_MAP, prefilterMap);
            d->shader->SetFloat("u_EnvironmentBlur",  ctx.Get<float>("u_EnvironmentBlur", 0.9f));
            d->shader->SetFloat("u_EnvironmentRot",   ctx.Get<float>("u_EnvironmentRot", 0.0f));
            d->shader->SetFloat("u_EnvironmentPower", ctx.Get<float>("u_EnvironmentPower", 1.0f));

            Renderer::DrawCube();

            glDepthFunc(GL_LESS);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline
Ref<RenderPass> CreateOpauePass(AssetManager& assetManager)
{
    struct PassData
    {
        uint32_t renderW, renderH, renderScale;
        Ref<FrameBuffer> fbo;
        Ref<Shader> shader;
    };
    auto d = std::make_shared<PassData>();

    return std::make_shared<RenderPass>("FWD Opaque",
        // ===== INIT =====
        [d, &assetManager](RenderPass& self)
        {
            d->renderW     = Renderer::GetViewportWidth();
            d->renderH     = Renderer::GetViewportHeight();
            d->renderScale = Renderer::GetRenderScale();
            d->fbo         = FrameBuffer::Create({d->renderW * d->renderScale, d->renderH * d->renderScale,
                                                 { ImgFmt::RGBA16F, ImgFmt::Depth32F }});
            d->shader      = assetManager.CreateShader("bolero://shaders/fwd_light_pass.glsl");
        },
        // ===== EXECUTE =====
        [d](Scene& scene, RenderContext& ctx, RenderPass& self)
        {
            /* FBO RESIZING & BINDING */
            uint32_t targetW = Renderer::GetViewportWidth();
            uint32_t targetH = Renderer::GetViewportHeight();
            uint32_t scale   = Renderer::GetRenderScale();
            if (targetW != d->renderW || targetH != d->renderH || scale != d->renderScale)
            {
                d->renderW     = targetW;
                d->renderH     = targetH;
                d->renderScale = scale;
                d->fbo->Resize(d->renderW * d->renderScale, d->renderH * d->renderScale);
            }
            d->fbo->Bind();
            

            /* GL STATES */
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            

            /* GL CLEAR */
            vec4 bgCol = ctx.Get<vec4>("BACKGROUND_COLOR", vec4(0.0f, 0.0f, 0.0f, 1.0f));
            glClearColor(bgCol.r, bgCol.g, bgCol.b, bgCol.a);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            
            /* SHADER */
            d->shader->Bind();

            // IBL Maps
            GLuint irradianceMap = ctx.Get<GLuint>("u_IrradianceMap");
            GLuint prefilterMap  = ctx.Get<GLuint>("u_PrefilterMap");
            bool useIBL = (irradianceMap != 0 && prefilterMap != 0);
            if (useIBL)
            {
                glBindTextureUnit(LayoutLoc::IRRADIANCE_MAP, irradianceMap);
                glBindTextureUnit(LayoutLoc::PREFILTER_MAP, prefilterMap);
                glBindTextureUnit(LayoutLoc::BRDF_LUT, ctx.Get<GLuint>("u_BrdfLut"));
            }

            // Skybox Uniforms
            d->shader->SetFloat("u_EnvironmentRot", ctx.Get<float>("u_EnvironmentRot", 0.0f));
            d->shader->SetFloat("u_EnvironmentPower", ctx.Get<float>("u_EnvironmentPower", 1.0f));
            d->shader->SetBool("u_UseIBL", useIBL ? true : false);

            // Shadow Map Uniforms
            int numDirShadows = std::min(ctx.Get<int>("u_NumDirShadows"), MAX_DIR_LIGHTS);
            for (int i = 0; i < numDirShadows; i++)
            {
                glBindTextureUnit(LayoutLoc::DIR_SHADOW_MAPS + i, ctx.Get<GLuint>("u_DirDepthMapTex_" + std::to_string(i)));

                d->shader->SetMat4("u_DirLightSpaceMat[" + std::to_string(i) + "]", ctx.Get<mat4>("u_DirLightSpaceMat_" + std::to_string(i)));
            }
            int numSpotShadows = std::min(ctx.Get<int>("u_NumSpotShadows"), MAX_SPOT_LIGHTS);
            for (int i = 0; i < numSpotShadows; i++)
            {
                glBindTextureUnit(LayoutLoc::SPOT_SHADOW_MAPS + i, ctx.Get<GLuint>("u_SpotDepthMapTex_" + std::to_string(i)));

                d->shader->SetMat4("u_SpotLightSpaceMat[" + std::to_string(i) + "]", ctx.Get<mat4>("u_SpotLightSpaceMat_" + std::to_string(i)));
            }
            int numPointShadows = std::min(ctx.Get<int>("u_NumPointShadows"), MAX_POINT_LIGHTS);
            for (int i = 0; i < numPointShadows; i++)
            {
                glBindTextureUnit(LayoutLoc::POINT_SHADOW_MAPS + i, ctx.Get<GLuint>("u_PointDepthMapTex_" + std::to_string(i)));
            }


            /* DRAW CALL */
            Renderer::UpdateCameraUBO(*scene.GetCam());
            Renderer::DrawQueue(RenderQueueType::OPAQUE, nullptr);


            /* FBO UNBINDING */
            d->fbo->Unbind();

            
            /* RENDER CONTEXT POPULATION */
            ctx.Set("OPAQUE_PASS_TEX", d->fbo->GetColorAttachmentID(0));
            ctx.Set("SCENE_FBO_ID", d->fbo->GetRendererID());
        }
    );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline
Ref<RenderPass> CreateDirShadowPass(AssetManager& assetManager, uint32_t size = 2048)
{
    struct PassData
    {
        Ref<Shader> shader;
        std::vector<Ref<FrameBuffer>> fbos;
        uint32_t size;
    };
    auto d = std::make_shared<PassData>();
    d->size = size;

    return std::make_shared<RenderPass>("Directional Shadow Pass",
        // ===== INIT =====
        [d, &assetManager](RenderPass& self)
        {
            d->shader = assetManager.CreateShader("bolero://shaders/shadow_pass.glsl");
            for (int i = 0; i < MAX_DIR_LIGHTS; i++)
                d->fbos.emplace_back(FrameBuffer::Create({ d->size, d->size, { ImgFmt::Depth32F } }));
        },
        // ===== EXECUTE =====
        [d](Scene& scene, RenderContext& ctx, RenderPass& self)
        {
            auto dirLights = scene.GetDirLights();
            if (dirLights.empty())
                return;

            int shadowMapIndex = 0;
            vec3 targetPos = vec3(0.0f);

            for (const auto& l : dirLights)
            {
                if (!l.castsShadow || shadowMapIndex >= MAX_DIR_LIGHTS)
                    continue;

                d->fbos[shadowMapIndex]->Bind();

                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_LESS);
                glEnable(GL_CULL_FACE);
                glCullFace(GL_FRONT); // Peter panning fix

                glClear(GL_DEPTH_BUFFER_BIT);

                d->shader->Bind();

                mat4 lightSpaceMat = l.GetLightSpaceMat(targetPos);
                d->shader->SetMat4("u_LightSpaceMat", lightSpaceMat);

                Renderer::DrawQueue(RenderQueueType::SHADOW_CASTER, d->shader.get());

                d->fbos[shadowMapIndex]->Unbind();


                ctx.Set("u_DirDepthMapTex_" + std::to_string(shadowMapIndex), d->fbos[shadowMapIndex]->GetDepthAttachmentID());
                ctx.Set("u_DirLightSpaceMat_" + std::to_string(shadowMapIndex), lightSpaceMat);

                shadowMapIndex++;
            }
            glCullFace(GL_BACK);


            ctx.Set("u_NumDirShadows", shadowMapIndex);
        }
    );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline
Ref<RenderPass> CreateSpotShadowPass(AssetManager& assetManager, uint32_t size = 512)
{
    struct PassData
    {
        Ref<Shader> shader;
        std::vector<Ref<FrameBuffer>> fbos;
        uint32_t size;
    };
    auto d = std::make_shared<PassData>();
    d->size = size;

    return std::make_shared<RenderPass>("Spot Shadow Pass",
        // ===== INIT =====
        [d, &assetManager](RenderPass& self)
        {
            d->shader = assetManager.CreateShader("bolero://shaders/shadow_pass.glsl");
            for (int i = 0; i < MAX_SPOT_LIGHTS; i++)
                d->fbos.emplace_back(FrameBuffer::Create({ d->size, d->size, { ImgFmt::Depth32F } }));
        },
        // ===== EXECUTE =====
        [d](Scene& scene, RenderContext& ctx, RenderPass& self)
        {
            auto spotLights = scene.GetSpotLights();
            if (spotLights.empty())
                return;

            int shadowMapIndex = 0;
            for (const auto& l : spotLights)
            {
                if (!l.castsShadow || shadowMapIndex >= MAX_SPOT_LIGHTS)
                    continue;

                d->fbos[shadowMapIndex]->Bind();

                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_LESS);
                glEnable(GL_CULL_FACE);
                glCullFace(GL_FRONT);

                glClear(GL_DEPTH_BUFFER_BIT);

                d->shader->Bind();

                mat4 lightSpaceMat = l.GetLightSpaceMat();
                d->shader->SetMat4("u_LightSpaceMat", lightSpaceMat);

                Renderer::DrawQueue(RenderQueueType::SHADOW_CASTER, d->shader.get());

                d->fbos[shadowMapIndex]->Unbind();


                ctx.Set("u_SpotDepthMapTex_" + std::to_string(shadowMapIndex), d->fbos[shadowMapIndex]->GetDepthAttachmentID());
                ctx.Set("u_SpotLightSpaceMat_" + std::to_string(shadowMapIndex), lightSpaceMat);

                shadowMapIndex++;
            }
            glCullFace(GL_BACK);


            ctx.Set("u_NumSpotShadows", shadowMapIndex);
        }
    );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline
Ref<RenderPass> CreatePointShadowPass(AssetManager& assetManager, uint32_t size = 512)
{
    struct PassData
    {
        Ref<Shader> shader;
        std::vector<Ref<FrameBuffer>> fbos;
        uint32_t size;
    };
    auto d = std::make_shared<PassData>();
    d->size = size;

    return std::make_shared<RenderPass>("Point Shadow Pass",
        // ===== INIT =====
        [d, &assetManager](RenderPass& self)
        {
            d->shader = assetManager.CreateShader("bolero://shaders/point_shadow_pass.glsl");
            for (int i = 0; i < MAX_POINT_LIGHTS; i++)
                d->fbos.emplace_back(FrameBuffer::Create({ d->size, d->size,{ {ImgFmt::Depth32F, true} } }));
        },
        // ===== EXECUTE =====
        [d](Scene& scene, RenderContext& ctx, RenderPass& self)
        {
            auto pointLights = scene.GetPointLights();
            if (pointLights.empty())
                return;

            int shadowMapIndex = 0;
            for (const auto& l : pointLights)
            {
                if (!l.castsShadow || shadowMapIndex >= MAX_POINT_LIGHTS)
                    continue;

                d->fbos[shadowMapIndex]->Bind();

                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_LESS);
                glEnable(GL_CULL_FACE);
                glCullFace(GL_FRONT);

                glClear(GL_DEPTH_BUFFER_BIT);

                d->shader->Bind();

                for (int i = 0; i < 6; i++)
                    d->shader->SetMat4("u_ShadowMatrices[" + std::to_string(i) + "]", l.GetLightSpaceMatrices()[i]);
                d->shader->SetFloat("u_FarPlane", l.range);
                d->shader->SetVec3("u_LightPos", l.position);

                Renderer::DrawQueue(RenderQueueType::SHADOW_CASTER, d->shader.get());

                d->fbos[shadowMapIndex]->Unbind();


                ctx.Set("u_PointDepthMapTex_" + std::to_string(shadowMapIndex), d->fbos[shadowMapIndex]->GetDepthAttachmentID());

                shadowMapIndex++;
            }

            glCullFace(GL_BACK);


            ctx.Set("u_NumPointShadows", shadowMapIndex);
        }
    );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline
Ref<RenderPass> CreateSSAAResolvePass()
{
    struct PassData
    {
        uint32_t renderW = 0, renderH = 0;
        Ref<FrameBuffer> resolveFbo;
    };
    auto d = std::make_shared<PassData>();

    return std::make_shared<RenderPass>("SSAA Resolve Pass",
        // ===== INIT =====
        [d](RenderPass& self)
        {
            d->renderW    = Renderer::GetViewportWidth();
            d->renderH    = Renderer::GetViewportHeight();
            d->resolveFbo = FrameBuffer::Create({ d->renderW, d->renderH, { ImgFmt::RGBA16F } });
        },
        // ===== EXECUTE =====
        [d](Scene& scene, RenderContext& ctx, RenderPass& self)
        {
            GLuint sceneFbo = ctx.Get<GLuint>("SCENE_FBO_ID", 0);
            if (sceneFbo == 0)
                return;

            uint32_t targetW = Renderer::GetViewportWidth();
            uint32_t targetH = Renderer::GetViewportHeight();
            uint32_t scale   = Renderer::GetRenderScale();
            if (targetW != d->renderW || targetH != d->renderH)
            {
                d->renderW = targetW;
                d->renderH = targetH;
                d->resolveFbo->Resize(d->renderW, d->renderH);
            }

            glBindFramebuffer(GL_READ_FRAMEBUFFER, sceneFbo);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, d->resolveFbo->GetRendererID());

            glBlitFramebuffer(0, 0, d->renderW * scale, d->renderH * scale,
                                0, 0, d->renderW, d->renderH,
                                GL_COLOR_BUFFER_BIT, GL_LINEAR);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);


            ctx.Set("RESOLVED_TEX", d->resolveFbo->GetColorAttachmentID(0));
        }
    );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline
Ref<RenderPass> CreatePostPass(AssetManager& assetManager)
{
    struct PassData
    {
        uint32_t renderW = 0, renderH = 0;
        Ref<Shader> shader;
        Ref<FrameBuffer> fbo;
    };
    auto d = std::make_shared<PassData>();

    return std::make_shared<RenderPass>("Post Pass",
        // ===== INIT =====
        [d, &assetManager](RenderPass& self)
        {
            d->shader  = assetManager.CreateShader("bolero://shaders/post_pass.glsl");
            d->renderW = Renderer::GetViewportWidth();
            d->renderH = Renderer::GetViewportHeight();
            d->fbo     = FrameBuffer::Create({ d->renderW, d->renderH, { ImgFmt::RGBA16F } });
        },
        // ===== EXECUTE =====
        [d](Scene& scene, RenderContext& ctx, RenderPass& self)
        {
            uint32_t targetW = Renderer::GetViewportWidth();
            uint32_t targetH = Renderer::GetViewportHeight();
            if (targetW != d->renderW || targetH != d->renderH)
            {
                d->renderW = targetW;
                d->renderH = targetH;
                d->fbo->Resize(d->renderW, d->renderH);
            }
            d->fbo->Bind();

            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);

            glClearColor(0.03f, 0.03f, 0.03f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            d->shader->Bind();

            d->shader->SetInt("u_ScreenTexture", 20);
            glBindTextureUnit(20, ctx.Get<GLuint>("RESOLVED_TEX"));
            d->shader->SetFloat("u_Exposure", ctx.Get<float>("u_Exposure", 1.0f));

            Renderer::DrawFullscreenQuad();

            d->fbo->Unbind();


            ctx.Set("POST_PASS_TEX", d->fbo->GetColorAttachmentID(0));
        }
    );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline 
Ref<RenderPass> CreateGeometryPass(AssetManager& assetManager)
{
    struct PassData
    {
        uint32_t renderW = 0, renderH = 0, renderScale = 0;
        Ref<FrameBuffer> fbo;
        Ref<Shader> shader;
    };
    auto d = std::make_shared<PassData>();

    return std::make_shared<RenderPass>("Geometry Pass",
        // ===== INIT =====
        [d, &assetManager](RenderPass& self)
        {
            d->shader      = assetManager.CreateShader("bolero://shaders/geometry_pass.glsl");
            d->renderW     = Renderer::GetViewportWidth();
            d->renderH     = Renderer::GetViewportHeight();
            d->renderScale = Renderer::GetRenderScale();
            d->fbo         = FrameBuffer::Create({
                d->renderW * d->renderScale, d->renderH * d->renderScale,
                { ImgFmt::RGBA8, ImgFmt::RGBA16F, ImgFmt::Depth32F }
            });
        },
        // ===== EXECUTE =====
        [d](Scene& scene, RenderContext& ctx, RenderPass& self)
        {
            uint32_t targetW = Renderer::GetViewportWidth();
            uint32_t targetH = Renderer::GetViewportHeight();
            uint32_t scale   = Renderer::GetRenderScale();
            if (scale != d->renderScale || targetW != d->renderW || targetH != d->renderH)
            {
                d->renderW     = targetW;
                d->renderH     = targetH;
                d->renderScale = scale;
                d->fbo->Resize(d->renderW * d->renderScale, d->renderH * d->renderScale);
            }
            d->fbo->Bind();

            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);

            vec4 backgroundCol = ctx.Get<vec4>("BACKGROUND_COLOR", vec4(0.0f, 0.0f, 0.0f, 1.0));
            glClearColor(backgroundCol.r, backgroundCol.g, backgroundCol.b, backgroundCol.a);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            d->shader->Bind();

            Renderer::UpdateCameraUBO(*scene.GetCam());

            Renderer::DrawQueue(RenderQueueType::OPAQUE, nullptr);

            d->fbo->Unbind();


            ctx.Set("GBUFFER_FBO_ID", d->fbo->GetRendererID());
            ctx.Set("G_ALBEDO_ROUGH", d->fbo->GetColorAttachmentID(0));
            ctx.Set("G_NORMAL_METAL", d->fbo->GetColorAttachmentID(1));
            ctx.Set("G_DEPTH",        d->fbo->GetDepthAttachmentID());
        }
    );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline
Ref<RenderPass> CreateShadowMaskPass(AssetManager& assetManager)
{
    struct PassData {
        uint32_t renderW = 0, renderH = 0, renderScale = 0;
        Ref<FrameBuffer> fbo;
        Ref<Shader> shader;
    };
    auto d = std::make_shared<PassData>();

    return std::make_shared<RenderPass>("Shadow Mask Pass",
        // ===== INIT =====
        [d, &assetManager](RenderPass& self)
        {
            d->shader      = assetManager.CreateShader("bolero://shaders/shadow_mask_pass.glsl");
            d->renderW     = Renderer::GetViewportWidth();
            d->renderH     = Renderer::GetViewportHeight();
            d->renderScale = Renderer::GetRenderScale();
            d->fbo         = FrameBuffer::Create({
                d->renderW * d->renderScale, d->renderH * d->renderScale,
                { ImgFmt::RGBA16F, ImgFmt::RGBA16F, ImgFmt::RGBA16F }
            });
        },
        // ===== EXECUTE =====
        [d](Scene& scene, RenderContext& ctx, RenderPass& self)
        {
            // 1. State Checks
            uint32_t targetW = Renderer::GetViewportWidth();
            uint32_t targetH = Renderer::GetViewportHeight();
            uint32_t scale   = Renderer::GetRenderScale();
            if (scale != d->renderScale || targetW != d->renderW || targetH != d->renderH)
            {
                d->renderW     = targetW;
                d->renderH     = targetH;
                d->renderScale = scale;
                d->fbo->Resize(d->renderW * d->renderScale, d->renderH * d->renderScale);
            }
            d->fbo->Bind();

            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);

            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            d->shader->Bind();

            glBindTextureUnit(LayoutLoc::G_NORMAL_METAL, ctx.Get<GLuint>("G_NORMAL_METAL"));
            glBindTextureUnit(LayoutLoc::G_DEPTH, ctx.Get<GLuint>("G_DEPTH"));

            int numDirShadows = std::min(ctx.Get<int>("u_NumDirShadows"), MAX_DIR_LIGHTS);
            for (int i = 0; i < numDirShadows; i++) {
                glBindTextureUnit(LayoutLoc::DIR_SHADOW_MAPS + i, ctx.Get<GLuint>("u_DirDepthMapTex_" + std::to_string(i)));
                d->shader->SetMat4("u_DirLightSpaceMat[" + std::to_string(i) + "]", ctx.Get<mat4>("u_DirLightSpaceMat_" + std::to_string(i)));
            }

            int numSpotShadows = std::min(ctx.Get<int>("u_NumSpotShadows"), MAX_SPOT_LIGHTS);
            for (int i = 0; i < numSpotShadows; i++) {
                glBindTextureUnit(LayoutLoc::SPOT_SHADOW_MAPS + i, ctx.Get<GLuint>("u_SpotDepthMapTex_" + std::to_string(i)));
                d->shader->SetMat4("u_SpotLightSpaceMat[" + std::to_string(i) + "]", ctx.Get<mat4>("u_SpotLightSpaceMat_" + std::to_string(i)));
            }

            int numPointShadows = std::min(ctx.Get<int>("u_NumPointShadows"), MAX_POINT_LIGHTS);
            for (int i = 0; i < numPointShadows; i++) {
                glBindTextureUnit(LayoutLoc::POINT_SHADOW_MAPS + i, ctx.Get<GLuint>("u_PointDepthMapTex_" + std::to_string(i)));
            }

            auto sceneCam = scene.GetCam();
            d->shader->SetMat4("u_InvViewProj", Inverse(sceneCam->GetProjMat() * sceneCam->GetViewMat()));

            Renderer::DrawFullscreenQuad();

            d->fbo->Unbind();


            ctx.Set("DIR_SHADOW_MASK",   d->fbo->GetColorAttachmentID(0));
            ctx.Set("SPOT_SHADOW_MASK",  d->fbo->GetColorAttachmentID(1));
            ctx.Set("POINT_SHADOW_MASK", d->fbo->GetColorAttachmentID(2));
        }
    );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline
Ref<RenderPass> CreateLightingPass(AssetManager& assetManager)
{
    struct PassData
    {
        uint32_t renderW = 0, renderH = 0, renderScale = 0;
        Ref<FrameBuffer> fbo;
        Ref<Shader> shader;
    };
    auto d = std::make_shared<PassData>();

    return std::make_shared<RenderPass>("Lighting Pass",
        // ===== INIT =====
        [d, &assetManager](RenderPass& self)
        {
            d->shader      = assetManager.CreateShader("bolero://shaders/def_light_pass.glsl");
            d->renderW     = Renderer::GetViewportWidth();
            d->renderH     = Renderer::GetViewportHeight();
            d->renderScale = Renderer::GetRenderScale();
            d->fbo         = FrameBuffer::Create({
                d->renderW * d->renderScale, d->renderH * d->renderScale,
                { ImgFmt::RGBA16F, ImgFmt::Depth32F }
            });
        },
        // ===== EXECUTE =====
        [d](Scene& scene, RenderContext& ctx, RenderPass& self)
        {
            uint32_t targetW = Renderer::GetViewportWidth();
            uint32_t targetH = Renderer::GetViewportHeight();
            uint32_t scale   = Renderer::GetRenderScale();
            if (scale != d->renderScale || targetW != d->renderW || targetH != d->renderH)
            {
                d->renderH     = targetH;
                d->renderW     = targetW;
                d->renderScale = scale;
                d->fbo->Resize(d->renderW * d->renderScale, d->renderH * d->renderScale);
            }
            d->fbo->Bind();

            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);

            vec4 backgroundCol = ctx.Get<vec4>("BACKGROUND_COLOR", vec4(0.0f, 0.0f, 0.0f, 1.0));
            glClearColor(backgroundCol.r, backgroundCol.g, backgroundCol.b, backgroundCol.a);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            d->shader->Bind();

            glBindTextureUnit(LayoutLoc::G_ALBEDO_ROUGH, ctx.Get<GLuint>("G_ALBEDO_ROUGH"));
            glBindTextureUnit(LayoutLoc::G_NORMAL_METAL, ctx.Get<GLuint>("G_NORMAL_METAL"));
            glBindTextureUnit(LayoutLoc::G_DEPTH, ctx.Get<GLuint>("G_DEPTH"));

            GLuint irradianceMap = ctx.Get<GLuint>("u_IrradianceMap");
            GLuint prefilterMap  = ctx.Get<GLuint>("u_PrefilterMap");
            bool useIBL = (irradianceMap != 0 && prefilterMap != 0);
            if (useIBL)
            {
                glBindTextureUnit(LayoutLoc::IRRADIANCE_MAP, irradianceMap);
                glBindTextureUnit(LayoutLoc::PREFILTER_MAP, prefilterMap);
                glBindTextureUnit(LayoutLoc::BRDF_LUT, ctx.Get<GLuint>("u_BrdfLut"));
            }

            d->shader->SetFloat("u_EnvironmentRot", ctx.Get<float>("u_EnvironmentRot", 0.0f));
            d->shader->SetFloat("u_EnvironmentPower", ctx.Get<float>("u_EnvironmentPower", 1.0f));
            d->shader->SetBool("u_UseIBL", useIBL ? true : false);

            glBindTextureUnit(LayoutLoc::DIR_SHADOW_MASKS, ctx.Get<GLuint>("DIR_SHADOW_MASK"));
            glBindTextureUnit(LayoutLoc::SPOT_SHADOW_MASKS, ctx.Get<GLuint>("SPOT_SHADOW_MASK"));
            glBindTextureUnit(LayoutLoc::POINT_SHADOW_MASKS, ctx.Get<GLuint>("POINT_SHADOW_MASK"));

            auto sceneCam = scene.GetCam();
            d->shader->SetMat4("u_InvViewProj", Inverse(sceneCam->GetProjMat() * sceneCam->GetViewMat()));

            Renderer::DrawFullscreenQuad();

            d->fbo->Unbind();

            GLuint gbufferFboID = ctx.Get<GLuint>("GBUFFER_FBO_ID");
            uint32_t w = d->renderW * d->renderScale;
            uint32_t h = d->renderH * d->renderScale;
            glBlitNamedFramebuffer(gbufferFboID, d->fbo->GetRendererID(),
                                    0, 0, w, h,
                                    0, 0, w, h,
                                    GL_DEPTH_BUFFER_BIT, GL_NEAREST);


            ctx.Set("OPAQUE_PASS_TEX", d->fbo->GetColorAttachmentID(0));
            ctx.Set("SCENE_FBO_ID", d->fbo->GetRendererID());
        }
    );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline
Ref<RenderPass> CreateUIPass(GLFWwindow* window, const std::vector<Ref<RenderPass>>& passes)
{
    struct PassData
    {
        UI ui;
    };
    auto d = std::make_shared<PassData>();

    return std::make_shared<RenderPass>("UI Pass",
        // ===== INIT =====
        [d, window](RenderPass& self)
        {
            d->ui.Init(window);
        },
        // ===== EXECUTE =====
        [d, &passes](Scene& scene, RenderContext& ctx, RenderPass& self)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            d->ui.BeginFrame();

            d->ui.DrawPipelineStats(scene, passes);
            d->ui.DrawProperties(scene, ctx);
            d->ui.DrawScene(ctx);

            d->ui.DrawExport(scene, ctx);

            d->ui.EndFrame();

            for (const auto& action : self.GetActions())
                action();
        }
    );
}


} /* namespace blr::core */