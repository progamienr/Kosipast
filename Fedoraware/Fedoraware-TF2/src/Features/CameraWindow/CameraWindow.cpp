#include "CameraWindow.h"

#include "../../Hooks/HookManager.h"
#include "../../Hooks/Hooks.h"

void CCameraWindow::Init()
{
	// Create camera texture
	CameraTex = I::MaterialSystem->CreateFullFrameRenderTarget("mirrorcam_rt");

	// Create camera material
	static auto* kv = new KeyValues("UnlitGeneric");
	kv->SetString("$basetexture", "mirrorcam_rt");
	CameraMat = I::MaterialSystem->Create("m_cameraMat", kv);
}

// Draws camera to the screen
void CCameraWindow::Draw()
{
	if (!CameraMat || !I::EngineClient->IsInGame() || !ShouldDraw)
		return;

	// Draw to screen
	const auto renderCtx = I::MaterialSystem->GetRenderContext();
	renderCtx->DrawScreenSpaceRectangle(
		CameraMat,
		ViewRect.x, ViewRect.y, ViewRect.w, ViewRect.h,
		0, 0, ViewRect.w, ViewRect.h,
		CameraTex->GetActualWidth(), CameraTex->GetActualHeight(),
		nullptr, 1, 1
	);
	renderCtx->Release();
}

// Renders another view onto a texture
void CCameraWindow::RenderView(void* ecx, const CViewSetup& pViewSetup)
{
	if (!CameraTex || !ShouldDraw)
		return;

	CViewSetup viewSetup = pViewSetup;
	viewSetup.x = 0;
	viewSetup.y = 0;

	viewSetup.origin = CameraOrigin;
	viewSetup.angles = CameraAngles;

	viewSetup.width = ViewRect.w + 1;
	viewSetup.height = ViewRect.h + 1;
	viewSetup.m_flAspectRatio = static_cast<float>(viewSetup.width) / static_cast<float>(viewSetup.height);
	viewSetup.fov = 90;

	RenderCustomView(ecx, viewSetup, CameraTex);
}

void CCameraWindow::RenderCustomView(void* ecx, const CViewSetup& pViewSetup, ITexture* pTexture)
{
	const auto renderCtx = I::MaterialSystem->GetRenderContext();

	renderCtx->PushRenderTargetAndViewport();
	renderCtx->SetRenderTarget(pTexture);

	static auto renderViewHook = g_HookManager.GetMapHooks()["ViewRender_RenderView"];
	if (renderViewHook)
		renderViewHook->Original<void(__thiscall*)(void*, const CViewSetup&, int, int)>()(ecx, pViewSetup, VIEW_CLEAR_COLOR | VIEW_CLEAR_DEPTH, RENDERVIEW_UNSPECIFIED);

	renderCtx->PopRenderTargetAndViewport();
	renderCtx->Release();
}