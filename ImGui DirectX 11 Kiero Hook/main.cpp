#include "includes.h"
#include "blur.h"
#include "cs2.h"
#include "game.h"
#include "render3d.h"
#include "model_preview.h"
#include "features.h"
#include "menu.h"
#include "warning_image_rgba.h"
#include "warning_image_ilya_rgba.h"
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

Present oPresent;
HWND window = NULL;
WNDPROC oWndProc;
ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;
ID3D11RenderTargetView* mainRenderTargetView;
typedef void(__stdcall* fnOMSetRenderTargets)(ID3D11DeviceContext*, UINT,
    ID3D11RenderTargetView* const*, ID3D11DepthStencilView*);
inline fnOMSetRenderTargets oOMSetRenderTargets = nullptr;
inline ID3D11DepthStencilView* g_lastSceneDSV = nullptr;
inline bool g_linesRenderedThisFrame = false;
inline bool g_prevHadDSV = false;
inline bool g_inFlush = false;

inline ID3D11ShaderResourceView* g_warningTexture = nullptr;
inline int g_warningTexW = 0;
inline int g_warningTexH = 0;
inline ID3D11ShaderResourceView* g_warningTexture2 = nullptr;
inline int g_warningTexW2 = 0;
inline int g_warningTexH2 = 0;
inline int g_warningShowCount = 0;

inline float g_warningCycleTimer = 0.0f;
inline float g_warningFadeAlpha = 0.0f;

bool LoadTextureFromMemory(ID3D11Device* device, const unsigned char* data, unsigned int dataSize,
    int width, int height, ID3D11ShaderResourceView** outSrv, int* outW, int* outH)
{
    if (!device || !data || dataSize == 0 || !outSrv || !outW || !outH) return false;

    *outSrv = nullptr;
    *outW = 0;
    *outH = 0;

    if (width <= 0 || height <= 0 || dataSize < (unsigned int)(width * height * 4)) return false;

    UINT w = (UINT)width;
    UINT h = (UINT)height;

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = w;
    desc.Height = h;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA srd = {};
    srd.pSysMem = data;
    srd.SysMemPitch = w * 4;

    ID3D11Texture2D* tex = nullptr;
    if (FAILED(device->CreateTexture2D(&desc, &srd, &tex))) return false;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = desc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;

    bool ok = SUCCEEDED(device->CreateShaderResourceView(tex, &srvDesc, outSrv));
    tex->Release();
    if (!ok) return false;

    *outW = (int)w;
    *outH = (int)h;
    return true;
}

void __stdcall hkOMSetRenderTargets(ID3D11DeviceContext* ctx, UINT NumViews,
    ID3D11RenderTargetView* const* ppRTVs, ID3D11DepthStencilView* pDSV)
{
    if (!g_inFlush) {
        if (g_prevHadDSV && !pDSV && !g_linesRenderedThisFrame
            && Render3D::lineCount > 0 && Render3D::initialized
            && ppRTVs && NumViews >= 1 && ppRTVs[0] == mainRenderTargetView)
        {
            g_inFlush = true;
            ImGuiIO& io = ImGui::GetIO();
            Render3D::Flush(ctx, mainRenderTargetView, g_lastSceneDSV,
                io.DisplaySize.x, io.DisplaySize.y);
            g_inFlush = false;
            g_linesRenderedThisFrame = true;
        }

        if (pDSV) {
            g_lastSceneDSV = pDSV;
        }
        g_prevHadDSV = (pDSV != nullptr);
    }

    oOMSetRenderTargets(ctx, NumViews, ppRTVs, pDSV);
}

void InitImGui()
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 20.0f, nullptr, io.Fonts->GetGlyphRangesCyrillic());
	static const ImWchar icon_ranges[] = { 0xE000, 0xF200, 0 };
	ImFontConfig icfg;
	icfg.MergeMode = true;
	icfg.PixelSnapH = true;
	icfg.GlyphMinAdvanceX = 20.0f;
	io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segmdl2.ttf", 16.0f, &icfg, icon_ranges);
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(pDevice, pContext);
}

LRESULT __stdcall WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_KEYDOWN && wParam == VK_INSERT) {
		Menu::show_menu = !Menu::show_menu;
		if (Menu::show_menu)
			ClipCursor(NULL);
		return 0;
	}

	if (Menu::show_menu) {
		ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

		switch (uMsg) {
			case WM_LBUTTONDOWN: case WM_LBUTTONUP:
			case WM_RBUTTONDOWN: case WM_RBUTTONUP:
			case WM_MOUSEMOVE:
			case WM_KEYDOWN: case WM_KEYUP:
			case WM_CHAR:
				return 1;
		}
	}

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

bool init = false;
float injectAnimAlpha = 1.0f;
float injectAnimTimer = 0.0f;
HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	if (!init)
	{
		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)& pDevice)))
		{
			pDevice->GetImmediateContext(&pContext);
			DXGI_SWAP_CHAIN_DESC sd;
			pSwapChain->GetDesc(&sd);
			window = sd.OutputWindow;
			ID3D11Texture2D* pBackBuffer;
			pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)& pBackBuffer);
			pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
			pBackBuffer->Release();
			oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
			InitImGui();
			Blur::Init(pDevice, pContext);
			Render3D::Init(pDevice, pContext);
			CS2::Init();
			CS2::LoadSteamAvatar(pDevice);
			{
				void** vtable = *(void***)pContext;
				DWORD oldP;
				VirtualProtect(&vtable[33], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldP);
				oOMSetRenderTargets = (fnOMSetRenderTargets)vtable[33];
				vtable[33] = (void*)hkOMSetRenderTargets;
				VirtualProtect(&vtable[33], sizeof(void*), oldP, &oldP);
			}
			ModelPreview::Init(pDevice, pContext);
			if (!LoadTextureFromMemory(pDevice, g_warningImageRgba, g_warningImageRgbaSize,
				g_warningImageWidth, g_warningImageHeight, &g_warningTexture, &g_warningTexW, &g_warningTexH)) {
				OutputDebugStringA("Warning texture 1 failed to load\n");
			} else {
				char buf[128];
				sprintf_s(buf, "Texture 1 loaded: %dx%d\n", g_warningTexW, g_warningTexH);
				OutputDebugStringA(buf);
			}
			if (!LoadTextureFromMemory(pDevice, g_warningImage2Rgba, g_warningImage2RgbaSize,
				g_warningImage2Width, g_warningImage2Height, &g_warningTexture2, &g_warningTexW2, &g_warningTexH2)) {
				OutputDebugStringA("Warning texture 2 failed to load\n");
			} else {
				char buf[128];
				sprintf_s(buf, "Texture 2 loaded: %dx%d\n", g_warningTexW2, g_warningTexH2);
				OutputDebugStringA(buf);
			}

			init = true;
		}

		else
			return oPresent(pSwapChain, SyncInterval, Flags);
		return oPresent(pSwapChain, SyncInterval, Flags);
	}
	
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (injectAnimAlpha > 0.0f)
	{
		injectAnimTimer += ImGui::GetIO().DeltaTime;
		injectAnimAlpha = 1.0f - (injectAnimTimer / 2.0f);
		if (injectAnimAlpha < 0.0f) injectAnimAlpha = 0.0f;

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
		ImGui::SetNextWindowBgAlpha(0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::Begin("##InjectAnim", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImVec2 screenSize = ImGui::GetIO().DisplaySize;
		ImVec2 center = ImVec2(screenSize.x / 2, screenSize.y / 2);

		float scale = 1.0f + sin(injectAnimTimer * 3.0f) * 0.05f * injectAnimAlpha;
		ImU32 color = ImColor(1.0f, 1.0f, 1.0f, injectAnimAlpha);
		ImU32 accentColor = ImColor(0.0f, 0.7f, 1.0f, injectAnimAlpha);

		drawList->AddRectFilled(ImVec2(0, 0), screenSize, ImColor(0.0f, 0.0f, 0.0f, injectAnimAlpha * 0.8f));

		std::string text = "INJECTED";
		ImGui::SetWindowFontScale(scale * 1.5f);
		ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
		drawList->AddText(ImVec2(center.x - textSize.x / 2, center.y - textSize.y / 2), color, text.c_str());

		ImGui::SetWindowFontScale(scale);
		std::string subText = "Welcome to PIZDOPASTA";
		ImVec2 subTextSize = ImGui::CalcTextSize(subText.c_str());
		drawList->AddText(ImVec2(center.x - subTextSize.x / 2, center.y + textSize.y + 20), accentColor, subText.c_str());

		float progressBarWidth = 200.0f * scale;
		float progressBarHeight = 3.0f;
		ImVec2 progressBarPos = ImVec2(center.x - progressBarWidth / 2, center.y + textSize.y + subTextSize.y + 40);
		drawList->AddRectFilled(progressBarPos, ImVec2(progressBarPos.x + progressBarWidth, progressBarPos.y + progressBarHeight), ImColor(0.2f, 0.2f, 0.2f, injectAnimAlpha));
		drawList->AddRectFilled(progressBarPos, ImVec2(progressBarPos.x + progressBarWidth * (1.0f - injectAnimAlpha), progressBarPos.y + progressBarHeight), accentColor);

		ImGui::End();
		ImGui::PopStyleVar(2);
		ImGui::SetWindowFontScale(1.0f);
	}

	if (Settings::warning_enabled) {
		g_warningCycleTimer += ImGui::GetIO().DeltaTime;
		if (g_warningCycleTimer >= 5.0f) {
			g_warningCycleTimer = 0.0f;
			g_warningFadeAlpha = 1.0f;
			g_warningShowCount++;
			char buf[128];
			sprintf_s(buf, "Warning show count: %d\n", g_warningShowCount);
			OutputDebugStringA(buf);
		}
	} else {
		g_warningCycleTimer = 0.0f;
		g_warningFadeAlpha = 0.0f;
	}

	if (g_warningFadeAlpha > 0.0f) {
		g_warningFadeAlpha -= ImGui::GetIO().DeltaTime / 1.6f;
		if (g_warningFadeAlpha < 0.0f) g_warningFadeAlpha = 0.0f;

		ImGuiIO& io = ImGui::GetIO();
		float sw = io.DisplaySize.x;
		float sh = io.DisplaySize.y;
		if (sw > 0.0f && sh > 0.0f) {
			ID3D11ShaderResourceView* currentTexture = nullptr;
			int currentTexW = 0, currentTexH = 0;
			int showIndex = g_warningShowCount % 6;
			if (showIndex < 5) {
				currentTexture = g_warningTexture;
				currentTexW = g_warningTexW;
				currentTexH = g_warningTexH;
			} else {
				currentTexture = g_warningTexture2;
				currentTexW = g_warningTexW2;
				currentTexH = g_warningTexH2;
			}

			if (currentTexture) {
				float iw = (float)currentTexW;
				float ih = (float)currentTexH;
				float scaleX = sw / iw;
				float scaleY = sh / ih;
				float scale = min(scaleX, scaleY);

				float dw = iw * scale;
				float dh = ih * scale;
				ImVec2 p0((sw - dw) * 0.5f, (sh - dh) * 0.5f);
				ImVec2 p1(p0.x + dw, p0.y + dh);

				ImDrawList* dl = ImGui::GetBackgroundDrawList();
				dl->AddRectFilled(ImVec2(0, 0), ImVec2(sw, sh), IM_COL32(0, 0, 0, (int)(140.0f * g_warningFadeAlpha)));
				dl->AddImage((ImTextureID)currentTexture, p0, p1, ImVec2(0, 0), ImVec2(1, 1),
					IM_COL32(255, 255, 255, (int)(255.0f * g_warningFadeAlpha)));
			}
		}
	}

	Game::Update();
	Prikol::CheckTrashtalk();
	GrenadeTracer::Update();
	GrenadePrediction::Update();
	BulletTracer::Update();
	WorldModulation::Update();
	SpectatorList::Update();
	Aimbot::Run();
	Triggerbot::Run();
	BunnyHop::Run();
	Misc::Run();
	AutoBuy::Run();
	AutoAccept::Run();
	ESP::Render();
	GrenadeTracer::Render();
	GrenadePrediction::Render();
	BulletTracer::Render();
	Visuals::Render();
	Aimbot::DrawFOVCircle();
	SpectatorList::Render(Menu::show_menu);
	Hotkeys::Render(Menu::show_menu);
	BombTimer::Render(Menu::show_menu);
	Menu::Render();

	ImGui::Render();
	if (!g_linesRenderedThisFrame && Render3D::lineCount > 0) {
		ID3D11DepthStencilView* fallbackDSV = nullptr;
		{
			ID3D11RenderTargetView* tmpRTV = nullptr;
			pContext->OMGetRenderTargets(1, &tmpRTV, &fallbackDSV);
			if (tmpRTV) tmpRTV->Release();
		}
		pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
		ImGuiIO& io = ImGui::GetIO();
		Render3D::Flush(pContext, mainRenderTargetView,
			fallbackDSV ? fallbackDSV : g_lastSceneDSV,
			io.DisplaySize.x, io.DisplaySize.y);
		if (fallbackDSV) fallbackDSV->Release();
	} else {
		Render3D::lineCount = 0;
	}

	pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	return oPresent(pSwapChain, SyncInterval, Flags);
}

DWORD WINAPI MainThread(LPVOID lpReserved)
{
	bool init_hook = false;
	do
	{
		if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success)
		{
			kiero::bind(8, (void**)& oPresent, hkPresent);
			init_hook = true;
		}
	} while (!init_hook);
	return TRUE;
}

BOOL WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hMod);
		CreateThread(nullptr, 0, MainThread, hMod, 0, nullptr);
		break;
	case DLL_PROCESS_DETACH:
		if (g_warningTexture) {
			g_warningTexture->Release();
			g_warningTexture = nullptr;
		}
		if (g_warningTexture2) {
			g_warningTexture2->Release();
			g_warningTexture2 = nullptr;
		}
		kiero::shutdown();
		break;
	}
	return TRUE;
}