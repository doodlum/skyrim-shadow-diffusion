#pragma once

#include "RE/B/BSShaderRenderTargets.h"
#include "RE/N/NiTexture.h"
#include "RE/R/RenderTargetData.h"
#include "RE/T/TextureFileFormat.h"

struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct ID3D11RenderTargetView1;
struct ID3D11ShaderResourceView1;

namespace RE
{
	namespace BSGraphics
	{
		struct RendererWindow
		{
		public:
			// members
			SKSE::WinAPI::HWND         hWnd;                   // 00
			std::int32_t               windowX;                // 08
			std::int32_t               windowY;                // 0C
			std::int32_t               windowWidth;            // 10
			std::int32_t               windowHeight;           // 14
			IDXGISwapChain*            swapChain;              // 18
			RENDER_TARGET              swapChainRenderTarget;  // 20
			std::uint32_t              unk24;                  // 24
			std::uint64_t              unk28;                  // 28
			ID3D11RenderTargetView1*   renderView;             // 30
			ID3D11ShaderResourceView1* resourceView;           // 38
			std::uint64_t              unk40;                  // 40
			std::uint64_t              unk48;                  // 48
		};
		static_assert(sizeof(RendererWindow) == 0x50);

		struct RendererData2
		{
		public:
#define RENDERER_DATA2_CONTENT                                                                    \
	CubemapRenderTargetData        cubemapRenderTargets[RENDER_TARGET_CUBEMAP::kTOTAL]; /* 26C8*/ \
	Texture3DTargetData            texture3DRenderTargets[RENDER_TARGET_3D::kTOTAL];    /* 2708*/ \
	float                          clearColor[4];                                       /* 2768*/ \
	std::uint8_t                   clearStencil;                                        /* 2778*/ \
	SKSE::WinAPI::CRITICAL_SECTION lock;                                                /* 2780*/ \
	const char*                    className;                                           /* 27A8*/ \
	SKSE::WinAPI::HINSTANCE        hInstance;                                           /* 27B0*/
            RENDERER_DATA2_CONTENT
		};

		struct DepthStencilRuntimeData
		{
		public:
#ifndef ENABLE_SKYRIM_VR
#	define DEPTHSTENCIL_RUNTIME_DATA_CONTENT \
		DepthStencilData depthStencils[RENDER_TARGET_DEPTHSTENCIL::kTOTAL]; /* 1FA8*/
//#elif !defined(ENABLE_SKYRIM_AE) && !defined(ENABLE_SKYRIM_SE)
#else
#	define DEPTHSTENCIL_RUNTIME_DATA_CONTENT \
		DepthStencilData depthStencils[RENDER_TARGET_DEPTHSTENCIL::kVRTOTAL]; /* 1FA8*/
#endif
			DEPTHSTENCIL_RUNTIME_DATA_CONTENT
		};

		struct RendererData
		{
		public:
			// members
#ifndef ENABLE_SKYRIM_VR
#	define RUNTIME_DATA_CONTENT                                                               \
		std::uint32_t        uiAdapter;                              /* 0000 */                \
		std::uint32_t        unk04;                                  /* 0004 - refreshRate? */ \
		std::uint32_t        unk08;                                  /* 0008 */                \
		std::uint32_t        unk0C;                                  /* 000C*/                 \
		std::uint32_t        unk10;                                  /* 0010*/                 \
		std::uint32_t        unk14;                                  /* 0014*/                 \
		std::uint32_t        unk18;                                  /* 0018*/                 \
		std::uint32_t        unk1C;                                  /* 001C*/                 \
		bool                 fullScreen;                             /* 0020*/                 \
		bool                 borderlessDisplay;                      /* 0021*/                 \
		bool                 readOnlyDepth;                          /* 0022*/                 \
		bool                 unk23;                                  /* 0023*/                 \
		bool                 unk24;                                  /* 0024*/                 \
		bool                 unk25;                                  /* 0025*/                 \
		std::uint32_t        unk28;                                  /* 0028*/                 \
		std::uint32_t        unk30;                                  /* 0030*/                 \
		std::uint32_t        presentInterval;                        /* 0034*/                 \
		ID3D11Device*        forwarder;                              /* 0038*/                 \
		ID3D11DeviceContext* context;                                /* 0040*/                 \
		RendererWindow       renderWindows[32];                      /* 0048*/                 \
		RenderTargetData     renderTargets[RENDER_TARGET::kVRTOTAL]; /* 0A48*/                 				
#else
#	define RUNTIME_DATA_CONTENT                                                             \
		std::uint32_t        uiAdapter;                            /* 0000 */                \
		std::uint32_t        unk04;                                /* 0004 - refreshRate? */ \
		std::uint32_t        unk08;                                /* 0008 */                \
		std::uint32_t        unk0C;                                /* 000C*/                 \
		std::uint32_t        unk10;                                /* 0010*/                 \
		std::uint32_t        unk14;                                /* 0014*/                 \
		std::uint32_t        unk18;                                /* 0018*/                 \
		std::uint32_t        unk1C;                                /* 001C*/                 \
		bool                 fullScreen;                           /* 0020*/                 \
		bool                 borderlessDisplay;                    /* 0021*/                 \
		bool                 readOnlyDepth;                        /* 0022*/                 \
		bool                 unk23;                                /* 0023*/                 \
		bool                 unk24;                                /* 0024*/                 \
		bool                 unk25;                                /* 0025*/                 \
		std::uint32_t        unk28;                                /* 0028*/                 \
		std::uint32_t        unk30;                                /* 0030*/                 \
		std::uint32_t        presentInterval;                      /* 0034*/                 \
		ID3D11Device*        forwarder;                            /* 0038*/                 \
		ID3D11DeviceContext* context;                              /* 0040*/                 \
		RendererWindow       renderWindows[32];                    /* 0048*/                 \
		RenderTargetData     renderTargets[RENDER_TARGET::kTOTAL]; /* 0A48*/
#endif
			RUNTIME_DATA_CONTENT;
		};
#ifndef ENABLE_SKYRIM_VR
		static_assert(sizeof(RendererData) == 0x21B8);
		static_assert(offsetof(RendererData, context) == 0x40);
		static_assert(offsetof(RendererData, renderTargets) == 0xa48);
#elif !defined(ENABLE_SKYRIM_AE) && !defined(ENABLE_SKYRIM_SE)
		static_assert(sizeof(RendererData) == 0x1fa8);
		static_assert(offsetof(RendererData, context) == 0x40);
		static_assert(offsetof(RendererData, renderTargets) == 0xa48);
#else
		static_assert(sizeof(RendererData) == 0x1fa8);
		static_assert(offsetof(RendererData, context) == 0x40);
		static_assert(offsetof(RendererData, renderTargets) == 0xa48);
#endif

		class Renderer
		{
		public:
			[[nodiscard]] inline RendererData& GetRuntimeData() noexcept
			{
				return REL::RelocateMember<RendererData>(this, 0x10, 0x18);
			}

			[[nodiscard]] inline const RendererData& GetRuntimeData() const noexcept
			{
				return REL::RelocateMember<RendererData>(this, 0x10, 0x18);
			}

			[[nodiscard]] inline DepthStencilRuntimeData& GetDepthStencilData() noexcept
			{
				return REL::RelocateMember<DepthStencilRuntimeData>(this, 0x1FB8, 0x21D0);
			}

			[[nodiscard]] inline const DepthStencilRuntimeData& GetDepthStencilData() const noexcept
			{
				return REL::RelocateMember<DepthStencilRuntimeData>(this, 0x1FB8, 0x21D0);
			}

			[[nodiscard]] inline RendererData2& GetRendererData() noexcept
			{
				return REL::RelocateMember<RendererData2>(this, 0x26C8, 0x2E48);
			}

			[[nodiscard]] inline const RendererData2& GetRendererData() const noexcept
			{
				return REL::RelocateMember<RendererData2>(this, 0x26C8, 0x2E48);
			}
			[[nodiscard]] inline SKSE::WinAPI::CRITICAL_SECTION& GetLock() noexcept
			{
				return REL::RelocateMember<SKSE::WinAPI::CRITICAL_SECTION>(this, 0x2790, 0x2F10);
			}

			[[nodiscard]] inline const SKSE::WinAPI::CRITICAL_SECTION& GetLock() const noexcept
			{
				return REL::RelocateMember<SKSE::WinAPI::CRITICAL_SECTION>(this, 0x2790, 0x2F10);
			}

			[[nodiscard]] static Renderer* GetSingleton() noexcept;

			[[nodiscard]] NiTexture::RendererData* CreateRenderTexture(std::uint32_t a_width, std::uint32_t a_height);
			void                                   SaveRenderTargetToFile(RENDER_TARGET a_renderTarget, const char* a_filePath, TextureFileFormat a_textureFileFormat);

			// members
			std::uint64_t unk000;       // 0000
			bool          drawStereo;  // 0008
#ifndef ENABLE_SKYRIM_VR
#elif !defined(ENABLE_SKYRIM_AE) && !defined(ENABLE_SKYRIM_SE)
			std::uint64_t unk010;  // 0010
#endif
			RUNTIME_DATA_CONTENT;  // 0010, VR 18
		};
#ifndef ENABLE_SKYRIM_VR
		static_assert(sizeof(Renderer) == 0x21C0);
#elif !defined(ENABLE_SKYRIM_AE) && !defined(ENABLE_SKYRIM_SE)
		static_assert(sizeof(Renderer) == 0x1fc0);
#else
		static_assert(sizeof(Renderer) == 0x1fb0);
#endif
	}
}
#undef RUNTIME_DATA_CONTENT
#undef DEPTHSTENCIL_RUNTIME_DATA_CONTENT
#undef RENDERER_DATA2_CONTENT
