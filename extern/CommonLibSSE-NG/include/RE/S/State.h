#pragma once

#include "RE/B/BSTArray.h"
#include "RE/N/NiCamera.h"
#include "RE/N/NiSmartPointer.h"
#include "RE/N/NiTexture.h"
#include "SKSE/Version.h"
#include <DirectXMath.h>

namespace RE
{
	namespace BSGraphics
	{
		//WARNING: Structs containing ViewData appear to break when returned via RelocateMember due to incorrect offsets.
		struct alignas(16) ViewData
		{
			DirectX::XMVECTOR viewUp;                            // 00
			DirectX::XMVECTOR viewRight;                         // 10
			DirectX::XMVECTOR viewForward;                       // 20
			DirectX::XMMATRIX viewMat;                           // 30
			DirectX::XMMATRIX projMat;                           // 70
			DirectX::XMMATRIX viewProjMat;                       // B0
			DirectX::XMMATRIX unknownMat1;                       // F0 - all 0?
			DirectX::XMMATRIX viewProjMatrixUnjittered;          // 130
			DirectX::XMMATRIX previousViewProjMatrixUnjittered;  // 170
			DirectX::XMMATRIX projMatrixUnjittered;              // 1B0
			DirectX::XMMATRIX unknownMat2;                       // 1F0 - all 0?
			float             viewPort[4];                       // 230 - NiRect<float> { left = 0, right = 1, top = 1, bottom = 0 }
			NiPoint2          viewDepthRange;                    // 240
			char              _pad0[0x8];                        // 248
		};
		static_assert(sizeof(ViewData) == 0x250);
		static_assert(offsetof(ViewData, viewUp) == 0);
		static_assert(offsetof(ViewData, viewRight) == 0x10);
		static_assert(offsetof(ViewData, viewForward) == 0x20);
		static_assert(offsetof(ViewData, viewMat) == 0x30);
		static_assert(offsetof(ViewData, projMat) == 0x70);
		static_assert(offsetof(ViewData, viewProjMat) == 0xb0);
		static_assert(offsetof(ViewData, unknownMat1) == 0xf0);
		static_assert(offsetof(ViewData, viewProjMatrixUnjittered) == 0x130);
		static_assert(offsetof(ViewData, previousViewProjMatrixUnjittered) == 0x170);
		static_assert(offsetof(ViewData, projMatrixUnjittered) == 0x1b0);
		static_assert(offsetof(ViewData, unknownMat2) == 0x1f0);
		static_assert(offsetof(ViewData, viewPort) == 0x230);
		static_assert(offsetof(ViewData, viewDepthRange) == 0x240);

		struct CAMERASTATE_RUNTIME_DATA
		{
#ifndef ENABLE_SKYRIM_VR                                                // Non-VR
#	define CAMERASTATE_RUNTIME_DATA_CONTENT                                                                                                   \
		ViewData camViewData;                                           /* 08 VR is BSTArray, Each array has 2 elements (one for each eye?) */ \
		NiPoint3 posAdjust;                                             /* 20 */                                                               \
		NiPoint3 currentPosAdjust;                                      /* 38 */                                                               \
		NiPoint3 previousPosAdjust;                                     /* 50 */                                                               \
		#elif !defined(ENABLE_SKYRIM_AE) && !defined(ENABLE_SKYRIM_SE)  // VR
#	define CAMERASTATE_RUNTIME_DATA_CONTENT                                                                                                   \
		BSTArray<ViewData> camViewData;                                 /* 08 VR is BSTArray, Each array has 2 elements (one for each eye?) */ \
		BSTArray<NiPoint3> posAdjust;                                   /* 20 */                                                               \
		BSTArray<NiPoint3> currentPosAdjust;                            /* 38 */                                                               \
		BSTArray<NiPoint3> previousPosAdjust;                           /* 50 */
#else
#	define CAMERASTATE_RUNTIME_DATA_CONTENT
#endif
			//members
			CAMERASTATE_RUNTIME_DATA_CONTENT;  // 08
			bool     useJitter;                /* 68 */
			uint32_t numData;                  /* 6c */
		};

		class CameraStateData
		{
		public:
			[[nodiscard]] CAMERASTATE_RUNTIME_DATA& GetCameraStateRuntimeData() noexcept
			{
				return REL::RelocateMember<CAMERASTATE_RUNTIME_DATA>(this, 0x8, 0x8);
			}

			[[nodiscard]] inline const CAMERASTATE_RUNTIME_DATA& GetCameraStateRuntimeData() const noexcept
			{
				return REL::RelocateMember<CAMERASTATE_RUNTIME_DATA>(this, 0x8, 0x8);
			}

			// members
			NiCamera* referenceCamera;                          /* 00 */
			CAMERASTATE_RUNTIME_DATA_CONTENT;                   // 08
		};
#ifndef ENABLE_SKYRIM_VR                                        // Non-VR
		static_assert(sizeof(CameraStateData) == 0x68);
#elif !defined(ENABLE_SKYRIM_AE) && !defined(ENABLE_SKYRIM_SE)  // VR
		static_assert(sizeof(CameraStateData) == 0x8);
#else
		static_assert(sizeof(CameraStateData) == 0x8);
#endif
		class State
		{
		public:
			struct RUNTIME_DATA
			{
#if !defined(ENABLE_SKYRIM_AE) && !defined(ENABLE_SKYRIM_SE)            // VR
#	define RUNTIME_DATA_CONTENT                                                                                              \
		uint32_t                  firstCameraStateIndex;                /*	058 VR   only ?*/                                  \
		NiPointer<NiTexture>      defaultTextureBlack;                  /* SE 058, AE,VR 060 - "BSShader_DefHeightMap"*/      \
		NiPointer<NiTexture>      defaultTextureWhite;                  /* SE 060, AE,VR 068 */                               \
		NiPointer<NiTexture>      defaultTextureGrey;                   /* SE 068, AE,VR 070 */                               \
		NiPointer<NiTexture>      defaultHeightMap;                     /* SE 070, AE,VR 078 */                               \
		NiPointer<NiTexture>      defaultReflectionCubeMap;             /* SE 078, AE,VR 080 */                               \
		NiPointer<NiTexture>      defaultFaceDetailMap;                 /* SE 080, AE,VR 088 */                               \
		NiPointer<NiTexture>      defaultTexEffectMap;                  /* SE 088, AE,VR 090 */                               \
		NiPointer<NiTexture>      defaultTextureNormalMap;              /* SE 090, AE,VR 098 */                               \
		NiPointer<NiTexture>      defaultTextureDitherNoiseMap;         /* SE 098, AE,VR 0a0 */                               \
		BSTArray<CameraStateData> cameraDataCacheA;                     /* SE 0a0, AE,VR 0a8 */                               \
		float                     _pad2;                                /* SE 0a8, AE,VR 0c0 */                               \
		float                     haltonSequence[2][8];                 /* SE 0c0, AE,VR 0c4 (2, 3) Halton Sequence points */ \
		float                     dynamicResolutionCurrentWidthScale;   /* SE 0c4, AE,VR 104 */                               \
		float                     dynamicResolutionCurrentHeightScale;  /* SE 104, AE,VR 108 */                               \
		float                     dynamicResolutionPreviousWidthScale;  /* SE 108, AE,VR 10c */                               \
		float                     dynamicResolutionPreviousHeightScale; /* SE 10c, AE,VR 110 */                               \
		float                     dynamicResolutionWidthRatio;          /* SE 110, AE,VR 114 */                               \
		float                     dynamicResolutionHeightRatio;         /* SE 114, AE,VR 118 */                               \
		uint16_t                  dynamicResolutionCounter;             /* SE 118, AE,VR 11c */
#else
#	define RUNTIME_DATA_CONTENT                                                                                              \
		NiPointer<NiTexture>      defaultTextureBlack;                  /* SE 058, AE,VR 060 - "BSShader_DefHeightMap"*/      \
		NiPointer<NiTexture>      defaultTextureWhite;                  /* SE 060, AE,VR 068 */                               \
		NiPointer<NiTexture>      defaultTextureGrey;                   /* SE 068, AE,VR 070 */                               \
		NiPointer<NiTexture>      defaultHeightMap;                     /* SE 070, AE,VR 078 */                               \
		NiPointer<NiTexture>      defaultReflectionCubeMap;             /* SE 078, AE,VR 080 */                               \
		NiPointer<NiTexture>      defaultFaceDetailMap;                 /* SE 080, AE,VR 088 */                               \
		NiPointer<NiTexture>      defaultTexEffectMap;                  /* SE 088, AE,VR 090 */                               \
		NiPointer<NiTexture>      defaultTextureNormalMap;              /* SE 090, AE,VR 098 */                               \
		NiPointer<NiTexture>      defaultTextureDitherNoiseMap;         /* SE 098, AE,VR 0a0 */                               \
		BSTArray<CameraStateData> cameraDataCacheA;                     /* SE 0a0, AE,VR 0a8 */                               \
		float                     _pad2;                                /* SE 0a8, AE,VR 0c0 */                               \
		float                     haltonSequence[2][8];                 /* SE 0c0, AE,VR 0c4 (2, 3) Halton Sequence points */ \
		float                     dynamicResolutionCurrentWidthScale;   /* SE 0c4, AE,VR 104 */                               \
		float                     dynamicResolutionCurrentHeightScale;  /* SE 104, AE,VR 108 */                               \
		float                     dynamicResolutionPreviousWidthScale;  /* SE 108, AE,VR 10c */                               \
		float                     dynamicResolutionPreviousHeightScale; /* SE 10c, AE,VR 110 */                               \
		float                     dynamicResolutionWidthRatio;          /* SE 110, AE,VR 114 */                               \
		float                     dynamicResolutionHeightRatio;         /* SE 114, AE,VR 118 */                               \
		uint16_t                  dynamicResolutionCounter;             /* SE 118, AE,VR 11c */
#endif
				RUNTIME_DATA_CONTENT;
			};
#ifndef ENABLE_SKYRIM_VR                                        // Non-VR
			static_assert(offsetof(RUNTIME_DATA, dynamicResolutionCurrentWidthScale) == 0xA4);
#elif !defined(ENABLE_SKYRIM_AE) && !defined(ENABLE_SKYRIM_SE)  // VR
			static_assert(offsetof(RUNTIME_DATA, dynamicResolutionCurrentWidthScale) == 0xAC);
#else
			static_assert(offsetof(RUNTIME_DATA, dynamicResolutionCurrentWidthScale) == 0xA4);
#endif
			[[nodiscard]] static State* GetSingleton()
			{
				REL::Relocation<State*> singleton{ RELOCATION_ID(524998, 411479) };
				return singleton.get();
			}

			CameraStateData* FindCameraDataCache(const NiCamera* a_camera, bool a_useJitter)
			{
				for (uint32_t i = 0; i < GetRuntimeData().cameraDataCacheA.size(); i++) {
					if (GetRuntimeData().cameraDataCacheA[i].referenceCamera == a_camera &&
						GetRuntimeData().cameraDataCacheA[i].GetCameraStateRuntimeData().useJitter == a_useJitter)
						return &GetRuntimeData().cameraDataCacheA[i];
				}
				return nullptr;
			}

			[[nodiscard]] RUNTIME_DATA& GetRuntimeData() noexcept
			{
				if SKYRIM_REL_CONSTEXPR (REL::Module::IsAE()) {
					if (REL::Module::get().version().compare(SKSE::RUNTIME_SSE_1_6_629) != std::strong_ordering::less) {
						return REL::RelocateMember<RUNTIME_DATA>(this, 0x60);
					}
				}
				return REL::RelocateMember<RUNTIME_DATA>(this, 0x58, 0x60);
			}

			[[nodiscard]] inline const RUNTIME_DATA& GetRuntimeData() const noexcept
			{
				if SKYRIM_REL_CONSTEXPR (REL::Module::IsAE()) {
					if (REL::Module::get().version().compare(SKSE::RUNTIME_SSE_1_6_629) != std::strong_ordering::less) {
						return REL::RelocateMember<RUNTIME_DATA>(this, 0x60);
					}
				}
				return REL::RelocateMember<RUNTIME_DATA>(this, 0x58, 0x60);
			}

			// members
			NiPointer<NiTexture> defaultTextureProjNoiseMap;         // 000
			NiPointer<NiTexture> defaultTextureProjDiffuseMap;       // 008
			NiPointer<NiTexture> defaultTextureProjNormalMap;        // 010
			NiPointer<NiTexture> defaultTextureProjNormalDetailMap;  // 018
			std::uint32_t        unk20;                              // 020
			std::uint32_t        screenWidth;                        // 024
			std::uint32_t        screenHeight;                       // 028
			std::uint32_t        frameBufferViewport[2];             // 02C
			std::uint32_t        unk34;                              // 034
			std::uint32_t        unk38;                              // 038
			float                unknown[2];                         // 03c
			float                projectionPosScaleX;                // 044
			float                projectionPosScaleY;                // 048
			uint32_t             uiFrameCount;                       // 04C
			bool                 insideFrame;                        // 050
			bool                 letterbox;                          // 051
			bool                 unk52;                              // 052
			bool                 compiledShaderThisFrame;            // 053
			uint8_t              unk54;                              // 054
			bool                 useEarlyZ;                          // 055
			RUNTIME_DATA_CONTENT;                                    // 058, VR 060
		};
#ifndef ENABLE_SKYRIM_VR                                             // Non-VR
		static_assert(offsetof(State, screenWidth) == 0x24);
		static_assert(offsetof(State, frameBufferViewport) == 0x2C);
		static_assert(offsetof(State, letterbox) == 0x51);
		static_assert(offsetof(State, defaultTextureBlack) == 0x58);
		static_assert(offsetof(State, defaultTextureWhite) == 0x60);
		static_assert(offsetof(State, cameraDataCacheA) == 0xa0);
		static_assert(offsetof(State, dynamicResolutionCurrentWidthScale) == 0x0fc);
#elif !defined(ENABLE_SKYRIM_AE) && !defined(ENABLE_SKYRIM_SE)  // VR
		static_assert(offsetof(State, screenWidth) == 0x24);
		static_assert(offsetof(State, frameBufferViewport) == 0x2C);
		static_assert(offsetof(State, letterbox) == 0x51);
		static_assert(offsetof(State, defaultTextureBlack) == 0x60);
		static_assert(offsetof(State, defaultTextureWhite) == 0x68);
		static_assert(offsetof(State, cameraDataCacheA) == 0xa8);
		static_assert(offsetof(State, dynamicResolutionCurrentWidthScale) == 0x104);
#else
		static_assert(offsetof(State, screenWidth) == 0x24);
		static_assert(offsetof(State, frameBufferViewport) == 0x2C);
		static_assert(offsetof(State, letterbox) == 0x51);
		static_assert(offsetof(State, defaultTextureBlack) == 0x58);
		static_assert(offsetof(State, defaultTextureWhite) == 0x60);
		static_assert(offsetof(State, cameraDataCacheA) == 0xa0);
		static_assert(offsetof(State, dynamicResolutionCurrentWidthScale) == 0x0fc);
#endif
	}
}
