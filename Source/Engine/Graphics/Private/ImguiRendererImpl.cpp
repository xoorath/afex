#include "ImguiRendererImpl.h"

// Engine
#include <Graphics/ViewId.h>

// External
#include <bgfx/embedded_shader.h>
#include <bx/bx.h>
#include <bx/allocator.h>
#include <bx/math.h>
#include <bx/timer.h>
#include <imgui.h>
#include <imgui_internal.h>

// The AFEX imgui renderer implementation is based heavily on bigg code in
// the public domain. Source: https://github.com/JoshuaBrookover/bigg
// 
// It's also based on the imgui implementation in bgfx example code under
// the following license:
// 
//     Copyright 2010-2024 Branimir Karadzic
// 
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//    1. Redistributions of source code must retain the above copyright notice, this
//       list of conditions and the following disclaimer.
// 
//    2. Redistributions in binary form must reproduce the above copyright notice,
//       this list of conditions and the following disclaimer in the documentation
//       and/or other materials provided with the distribution.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.

//////////////////////////////////////////////////////////////////////////
namespace Graphics
{
    ////////////////////////////////////////////////////////////////////////// Internal
    static const uint32_t fs_ocornut_imgui_dx11_len = 423;
    static const unsigned char fs_ocornut_imgui_dx11[] = {
        0x46,0x53,0x48,0x06,0x01,0x83,0xF2,0xE1,0x00,0x00,0x00,0x00,0x01,0x00,0x05,0x73,0x5F,0x74,0x65,0x78,0x30,0x01,0x00,0x00,0x01,
        0x00,0x84,0x01,0x00,0x00,0x44,0x58,0x42,0x43,0x61,0x82,0x96,0x5A,0x6E,0x93,0xED,0x8E,0xB3,0x8E,0xE2,0x6C,0xAE,0x9F,0x04,0x7F,
        0x01,0x00,0x00,0x00,0x84,0x01,0x00,0x00,0x03,0x00,0x00,0x00,0x2C,0x00,0x00,0x00,0xA0,0x00,0x00,0x00,0xD4,0x00,0x00,0x00,0x49,
        0x53,0x47,0x4E,0x6C,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x50,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,
        0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0x00,0x00,0x00,0x5C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x03,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x0F,0x0F,0x00,0x00,0x62,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x03,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x03,0x03,0x00,0x00,0x53,0x56,0x5F,0x50,0x4F,0x53,0x49,0x54,0x49,0x4F,0x4E,0x00,0x43,
        0x4F,0x4C,0x4F,0x52,0x00,0x54,0x45,0x58,0x43,0x4F,0x4F,0x52,0x44,0x00,0xAB,0x4F,0x53,0x47,0x4E,0x2C,0x00,0x00,0x00,0x01,0x00,
        0x00,0x00,0x08,0x00,0x00,0x00,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x0F,0x00,0x00,0x00,0x53,0x56,0x5F,0x54,0x41,0x52,0x47,0x45,0x54,0x00,0xAB,0xAB,0x53,0x48,0x44,0x52,0xA8,0x00,0x00,0x00,
        0x40,0x00,0x00,0x00,0x2A,0x00,0x00,0x00,0x5A,0x00,0x00,0x03,0x00,0x60,0x10,0x00,0x00,0x00,0x00,0x00,0x58,0x18,0x00,0x04,0x00,
        0x70,0x10,0x00,0x00,0x00,0x00,0x00,0x55,0x55,0x00,0x00,0x62,0x10,0x00,0x03,0xF2,0x10,0x10,0x00,0x01,0x00,0x00,0x00,0x62,0x10,
        0x00,0x03,0x32,0x10,0x10,0x00,0x02,0x00,0x00,0x00,0x65,0x00,0x00,0x03,0xF2,0x20,0x10,0x00,0x00,0x00,0x00,0x00,0x68,0x00,0x00,
        0x02,0x01,0x00,0x00,0x00,0x45,0x00,0x00,0x09,0xF2,0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x46,0x10,0x10,0x00,0x02,0x00,0x00,0x00,
        0x46,0x7E,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x10,0x00,0x00,0x00,0x00,0x00,0x36,0x00,0x00,0x05,0xF2,0x00,0x10,0x00,0x00,
        0x00,0x00,0x00,0x46,0x0E,0x10,0x00,0x00,0x00,0x00,0x00,0x38,0x00,0x00,0x07,0xF2,0x20,0x10,0x00,0x00,0x00,0x00,0x00,0x46,0x0E,
        0x10,0x00,0x00,0x00,0x00,0x00,0x46,0x1E,0x10,0x00,0x01,0x00,0x00,0x00,0x3E,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00
    };

    static const uint32_t vs_ocornut_imgui_dx11_len = 743;
    static const unsigned char vs_ocornut_imgui_dx11[] = {
        0x56,0x53,0x48,0x06,0x00,0x00,0x00,0x00,0x01,0x83,0xF2,0xE1,0x01,0x00,0x0B,0x75,0x5F,0x76,0x69,0x65,0x77,0x54,0x65,0x78,0x65,
        0x6C,0x02,0x00,0x00,0x00,0x01,0x00,0xB8,0x02,0x00,0x00,0x44,0x58,0x42,0x43,0x4D,0xE6,0x92,0xC7,0x99,0x74,0xD2,0xF7,0x95,0x65,
        0x3E,0x89,0xC7,0x62,0xF1,0x35,0x01,0x00,0x00,0x00,0xB8,0x02,0x00,0x00,0x03,0x00,0x00,0x00,0x2C,0x00,0x00,0x00,0x9C,0x00,0x00,
        0x00,0x10,0x01,0x00,0x00,0x49,0x53,0x47,0x4E,0x68,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x50,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0x0F,0x00,0x00,0x56,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x07,0x03,0x00,0x00,0x5F,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x03,0x03,0x00,0x00,0x43,0x4F,0x4C,0x4F,0x52,0x00,0x50,
        0x4F,0x53,0x49,0x54,0x49,0x4F,0x4E,0x00,0x54,0x45,0x58,0x43,0x4F,0x4F,0x52,0x44,0x00,0x4F,0x53,0x47,0x4E,0x6C,0x00,0x00,0x00,
        0x03,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x50,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x0F,0x00,0x00,0x00,0x5C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x01,0x00,
        0x00,0x00,0x0F,0x00,0x00,0x00,0x62,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x02,0x00,0x00,
        0x00,0x03,0x0C,0x00,0x00,0x53,0x56,0x5F,0x50,0x4F,0x53,0x49,0x54,0x49,0x4F,0x4E,0x00,0x43,0x4F,0x4C,0x4F,0x52,0x00,0x54,0x45,
        0x58,0x43,0x4F,0x4F,0x52,0x44,0x00,0xAB,0x53,0x48,0x44,0x52,0xA0,0x01,0x00,0x00,0x40,0x00,0x01,0x00,0x68,0x00,0x00,0x00,0x59,
        0x00,0x00,0x04,0x46,0x8E,0x20,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x5F,0x00,0x00,0x03,0xF2,0x10,0x10,0x00,0x00,0x00,
        0x00,0x00,0x5F,0x00,0x00,0x03,0x32,0x10,0x10,0x00,0x01,0x00,0x00,0x00,0x5F,0x00,0x00,0x03,0x32,0x10,0x10,0x00,0x02,0x00,0x00,
        0x00,0x67,0x00,0x00,0x04,0xF2,0x20,0x10,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x65,0x00,0x00,0x03,0xF2,0x20,0x10,0x00,
        0x01,0x00,0x00,0x00,0x65,0x00,0x00,0x03,0x32,0x20,0x10,0x00,0x02,0x00,0x00,0x00,0x68,0x00,0x00,0x02,0x03,0x00,0x00,0x00,0x38,
        0x00,0x00,0x0A,0x32,0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x46,0x10,0x10,0x00,0x01,0x00,0x00,0x00,0x02,0x40,0x00,0x00,0x00,0x00,
        0x00,0x40,0x00,0x00,0x00,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x38,0x00,0x00,0x08,0x32,0x00,0x10,0x00,0x00,0x00,0x00,
        0x00,0x46,0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x46,0x80,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x36,0x00,0x00,0x05,
        0x42,0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x01,0x40,0x00,0x00,0x00,0x00,0x80,0xBF,0x00,0x00,0x00,0x07,0x12,0x00,0x10,0x00,0x01,
        0x00,0x00,0x00,0x2A,0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x0A,0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x36,0x00,0x00,0x06,0x12,0x00,
        0x10,0x00,0x00,0x00,0x00,0x00,0x1A,0x00,0x10,0x80,0x41,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0x22,0x00,0x10,
        0x00,0x01,0x00,0x00,0x00,0x0A,0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x01,0x40,0x00,0x00,0x00,0x00,0x80,0x3F,0x36,0x00,0x00,0x08,
        0xC2,0x00,0x10,0x00,0x01,0x00,0x00,0x00,0x02,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x80,0x3F,0x36,0x00,0x00,0x05,0x32,0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x46,0x10,0x10,0x00,0x02,0x00,0x00,0x00,0x36,0x00,
        0x00,0x05,0xF2,0x00,0x10,0x00,0x02,0x00,0x00,0x00,0x46,0x1E,0x10,0x00,0x00,0x00,0x00,0x00,0x36,0x00,0x00,0x05,0xF2,0x20,0x10,
        0x00,0x00,0x00,0x00,0x00,0x46,0x0E,0x10,0x00,0x01,0x00,0x00,0x00,0x36,0x00,0x00,0x05,0xF2,0x20,0x10,0x00,0x01,0x00,0x00,0x00,
        0x46,0x0E,0x10,0x00,0x02,0x00,0x00,0x00,0x36,0x00,0x00,0x05,0x32,0x20,0x10,0x00,0x02,0x00,0x00,0x00,0x46,0x00,0x10,0x00,0x00,
        0x00,0x00,0x00,0x3E,0x00,0x00,0x01,0x00,0x03,0x05,0x00,0x01,0x00,0x10,0x00,0x10,0x00,0x00
    };

    ////////////////////////////////////////////////////////////////////////// Public
    ImGuiRendererImpl::ImGuiRendererImpl(ImGuiContext* context, uint32_t displayWidth, uint32_t displayHeight)
        : m_ImguiContext(context)
        , m_VertexLayout()
        , m_FontTexture(bgfx::kInvalidHandle)
        , m_FontUniform(bgfx::kInvalidHandle)
        , m_WidthHeight((static_cast<uint64_t>(displayWidth) << 32ull) | static_cast<uint64_t>(displayHeight))
        , m_PreviousWidth(displayWidth)
        , m_PreviousHeight(displayHeight)
        , m_Program(bgfx::kInvalidHandle)
    {
        ImGuiIO& io = ImGui::GetIO();

        m_VertexLayout
            .begin()
            .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
            .end();

        io.Fonts->AddFontDefault();

        unsigned char* data = nullptr;
        int width = 0, height = 0;
        io.Fonts->GetTexDataAsRGBA32(&data, &width, &height);

        io.DisplaySize.x = static_cast<float>(displayWidth);
        io.DisplaySize.y = static_cast<float>(displayHeight);
        io.DisplayFramebufferScale.x = 1.0f;
        io.DisplayFramebufferScale.y = 1.0f;

        m_FontTexture = bgfx::createTexture2D(
            /*width=*/      static_cast<uint16_t>(width),
            /*height=*/     static_cast<uint16_t>(height),
            /*hasMips=*/    false,
            /*numLayers=*/  1,
            /*format=*/     bgfx::TextureFormat::BGRA8,
            /*flags=*/      0,
            /*memory=*/     bgfx::copy(data, width * height * 4)
        );

        m_FontUniform = bgfx::createUniform("s_tex", bgfx::UniformType::Sampler);

        bgfx::ShaderHandle vs = bgfx::createShader(bgfx::makeRef(vs_ocornut_imgui_dx11, vs_ocornut_imgui_dx11_len));
        bgfx::ShaderHandle fs = bgfx::createShader(bgfx::makeRef(fs_ocornut_imgui_dx11, fs_ocornut_imgui_dx11_len));
        m_Program = bgfx::createProgram(vs, fs, true);


        bgfx::setViewClear(Graphics::ViewId::k_ImGui, BGFX_CLEAR_NONE);
        bgfx::setViewRect(
            /*viewId=*/     Graphics::ViewId::k_ImGui,
            /*x=*/          0,
            /*y=*/          0,
            /*width=*/      static_cast<uint16_t>(displayWidth),
            /*height=*/     static_cast<uint16_t>(displayHeight));
    }

    ImGuiRendererImpl::ImGuiRendererImpl(ImGuiRendererImpl&& other) noexcept
        : m_VertexLayout    (other.m_VertexLayout)
        , m_FontTexture     (other.m_FontTexture)
        , m_FontUniform     (other.m_FontUniform)
        , m_Program         (other.m_Program)
        , m_WidthHeight     (other.m_WidthHeight.load())
        , m_PreviousWidth   (other.m_PreviousWidth)
        , m_PreviousHeight  (other.m_PreviousHeight)
        , m_ImguiContext    (other.m_ImguiContext)
    {
        other.m_FontTexture = static_cast<bgfx::TextureHandle>(bgfx::kInvalidHandle);
        other.m_FontUniform = static_cast<bgfx::UniformHandle>(bgfx::kInvalidHandle);
        other.m_Program = static_cast<bgfx::ProgramHandle>(bgfx::kInvalidHandle);
        other.m_ImguiContext = nullptr;
    }

    ImGuiRendererImpl& ImGuiRendererImpl::operator=(ImGuiRendererImpl&& other) noexcept
    {
        m_VertexLayout = other.m_VertexLayout;
        m_FontTexture = other.m_FontTexture;
        m_FontUniform = other.m_FontUniform;
        m_Program = other.m_Program;
        m_WidthHeight.store(other.m_WidthHeight.load());
        m_PreviousWidth = other.m_PreviousWidth;
        m_PreviousHeight = other.m_PreviousHeight;
        m_ImguiContext = other.m_ImguiContext;

        other.m_FontTexture = static_cast<bgfx::TextureHandle>(bgfx::kInvalidHandle);
        other.m_FontUniform = static_cast<bgfx::UniformHandle>(bgfx::kInvalidHandle);
        other.m_Program = static_cast<bgfx::ProgramHandle>(bgfx::kInvalidHandle);
        other.m_ImguiContext = nullptr;

        return *this;
    }

    ImGuiRendererImpl::~ImGuiRendererImpl()
    {
        auto safeDestroyHandle = [](auto& handle)
            {
                if (bgfx::isValid(handle))
                {
                    bgfx::destroy(handle);
                }
            };

        safeDestroyHandle(m_FontUniform);
        safeDestroyHandle(m_FontTexture);
        safeDestroyHandle(m_Program);
    }

    void ImGuiRendererImpl::BeginFrame()
    {
        ImGui::SetCurrentContext(m_ImguiContext);
        ImGui::NewFrame();
    }

    void ImGuiRendererImpl::EndFrame()
    {
        ImGui::Render();
    }

    void ImGuiRendererImpl::Render()
    {
        ImDrawData* const drawData = ImGui::GetDrawData();
        if (drawData == nullptr)
        {
            return;
        }

        bgfx::setViewName(Graphics::ViewId::k_ImGui, "ImGui");
        bgfx::setViewMode(Graphics::ViewId::k_ImGui, bgfx::ViewMode::Sequential);

        const uint64_t widthHeightCombined = m_WidthHeight.load();
        const uint32_t width = static_cast<uint32_t>((widthHeightCombined >> 32ul) & 0x00000000ffffffffull);
        const uint32_t height = static_cast<uint32_t>(widthHeightCombined & 0x00000000ffffffffull);

        if(width != m_PreviousWidth || height != m_PreviousHeight)
        {
            m_PreviousWidth = width;
            m_PreviousHeight = height;
            bgfx::setViewRect(
                /*viewId=*/     Graphics::ViewId::k_ImGui,
                /*x=*/          0,
                /*y=*/          0,
                /*width=*/      static_cast<uint16_t>(width),
                /*height=*/     static_cast<uint16_t>(height));

            ImGuiIO& io = ImGui::GetIO();
            io.DisplaySize.x = static_cast<float>(width);
            io.DisplaySize.y = static_cast<float>(height);

        }

        bgfx::touch(Graphics::ViewId::k_ImGui);

        for (int32_t i = 0, num = drawData->CmdListsCount; i < num; ++i)
        {
            bgfx::TransientVertexBuffer tvb;
            bgfx::TransientIndexBuffer tib;

            const ImDrawList* drawList = drawData->CmdLists[i];
            const uint32_t numVerticies = static_cast<uint32_t>(drawList->VtxBuffer.size());
            const uint32_t numIndicies = static_cast<uint32_t>(drawList->IdxBuffer.size());

            if (!bgfx::getAvailTransientVertexBuffer(numVerticies, m_VertexLayout) ||
                !bgfx::getAvailTransientIndexBuffer(numIndicies))
            {
                break;
            }

            bgfx::allocTransientVertexBuffer(&tvb, numVerticies, m_VertexLayout);
            bgfx::allocTransientIndexBuffer(&tib, numIndicies);

            ImDrawVert* verts = reinterpret_cast<ImDrawVert*>(tvb.data);
            bx::memCopy(verts, drawList->VtxBuffer.begin(), static_cast<size_t>(numVerticies) * sizeof(ImDrawVert));

            ImDrawIdx* indicies = reinterpret_cast<ImDrawIdx*>(tib.data);
            bx::memCopy(indicies, drawList->IdxBuffer.begin(), static_cast<size_t>(numIndicies) * sizeof(ImDrawIdx));

            bgfx::Encoder* encoder = bgfx::begin();

            for (const ImDrawCmd& cmd : drawList->CmdBuffer)
            {
                if (cmd.UserCallback != nullptr)
                {
                    cmd.UserCallback(drawList, &cmd);
                }
                else if (cmd.ElemCount > 0)
                {
                    uint64_t state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_MSAA;
                    bgfx::TextureHandle th = m_FontTexture;
                    if (cmd.TextureId != nullptr)
                    {
                        th.idx = static_cast<uint16_t>(reinterpret_cast<uintptr_t>(cmd.TextureId));
                    }
                    state |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);
                    const uint16_t xx = static_cast<uint16_t>(bx::max(cmd.ClipRect.x, 0.0f));
                    const uint16_t yy = static_cast<uint16_t>(bx::max(cmd.ClipRect.y, 0.0f));
                    encoder->setScissor(
                        xx,
                        yy,
                        static_cast<uint16_t>(bx::min(cmd.ClipRect.z, 65535.0f) - xx),
                        static_cast<uint16_t>(bx::min(cmd.ClipRect.w, 65535.0f) - yy)
                    );
                    encoder->setState(state);
                    encoder->setTexture(0, m_FontUniform, th);
                    encoder->setVertexBuffer(0, &tvb, 0, numVerticies);
                    encoder->setIndexBuffer(&tib, cmd.IdxOffset, cmd.ElemCount);
                    encoder->submit(::Graphics::ViewId::k_ImGui, m_Program);
                }
            }

            bgfx::end(encoder);
        }
    }

    void ImGuiRendererImpl::Resize(uint32_t width, uint32_t height)
    {
        m_WidthHeight.store((static_cast<uint64_t>(width) << 32ull) | static_cast<uint64_t>(height));
    }
}