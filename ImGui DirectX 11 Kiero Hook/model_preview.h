#pragma once
#include <d3d11.h>
#include <cstring>
namespace ModelPreview
{
    constexpr int PV_W = 256;
    constexpr int PV_H = 380;

    inline ID3D11Device*              pDev = nullptr;
    inline ID3D11DeviceContext*        pCtx = nullptr;
    inline ID3D11Texture2D*           pvTex = nullptr;
    inline ID3D11RenderTargetView*    pvRTV = nullptr;
    inline ID3D11ShaderResourceView*  pvSRV = nullptr;
    inline ID3D11DepthStencilView*    pvDSV = nullptr;
    inline ID3D11Texture2D*           pvDepthTex = nullptr;
    inline ID3D11BlendState*          pvBlend = nullptr;
    typedef void(__stdcall* fnDrawIndexed)(ID3D11DeviceContext*, UINT, UINT, INT);
    inline fnDrawIndexed oDrawIndexed = nullptr;
    inline bool initialized = false;
    inline bool captureEnabled = false;
    inline bool frameCaptured = false;
    inline bool insideHook = false;
    inline int  captureCount = 0;
    inline int  frameCounter = 0;
    constexpr UINT MIN_INDEX_COUNT = 2000;
    constexpr UINT MAX_INDEX_COUNT = 100000;
    inline bool IsPlayerModelStride(UINT stride) {
        return stride == 40 || stride == 44 || stride == 48 ||
               stride == 52 || stride == 56 || stride == 60 || stride == 64;
    }

    inline bool CreateResources() {
        if (pvTex) return true;
        if (!pDev) return false;
        D3D11_TEXTURE2D_DESC td = {};
        td.Width = PV_W; td.Height = PV_H;
        td.MipLevels = 1; td.ArraySize = 1;
        td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_DEFAULT;
        td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        if (FAILED(pDev->CreateTexture2D(&td, nullptr, &pvTex))) return false;
        pDev->CreateRenderTargetView(pvTex, nullptr, &pvRTV);
        pDev->CreateShaderResourceView(pvTex, nullptr, &pvSRV);
        td.Format = DXGI_FORMAT_D32_FLOAT;
        td.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        if (FAILED(pDev->CreateTexture2D(&td, nullptr, &pvDepthTex))) return false;
        D3D11_DEPTH_STENCIL_VIEW_DESC dsvd = {};
        dsvd.Format = DXGI_FORMAT_D32_FLOAT;
        dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        pDev->CreateDepthStencilView(pvDepthTex, &dsvd, &pvDSV);
        D3D11_BLEND_DESC bd = {};
        bd.RenderTarget[0].BlendEnable = TRUE;
        bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        pDev->CreateBlendState(&bd, &pvBlend);

        return true;
    }

    inline void FreeResources() {
        if (pvTex)      { pvTex->Release();      pvTex = nullptr; }
        if (pvRTV)      { pvRTV->Release();      pvRTV = nullptr; }
        if (pvSRV)      { pvSRV->Release();      pvSRV = nullptr; }
        if (pvDSV)      { pvDSV->Release();      pvDSV = nullptr; }
        if (pvDepthTex) { pvDepthTex->Release();  pvDepthTex = nullptr; }
        if (pvBlend)    { pvBlend->Release();     pvBlend = nullptr; }
    }

    inline void BeginFrame() {
        frameCounter++;
        frameCaptured = false;
        captureCount = 0;

        if (!captureEnabled || !pvRTV) return;
        float cl[4] = { 0.03f, 0.04f, 0.06f, 0.95f };
        pCtx->ClearRenderTargetView(pvRTV, cl);
        pCtx->ClearDepthStencilView(pvDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
    }
    inline void __stdcall hkDrawIndexed(ID3D11DeviceContext* ctx,
        UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation)
    {
        if (insideHook) {
            oDrawIndexed(ctx, IndexCount, StartIndexLocation, BaseVertexLocation);
            return;
        }
        if (captureEnabled && pvRTV && !frameCaptured &&
            IndexCount >= MIN_INDEX_COUNT && IndexCount <= MAX_INDEX_COUNT)
        {
            ID3D11Buffer* vb = nullptr;
            UINT stride = 0, offset = 0;
            ctx->IAGetVertexBuffers(0, 1, &vb, &stride, &offset);
            if (vb) vb->Release();

            if (IsPlayerModelStride(stride)) {
                insideHook = true;
                ID3D11RenderTargetView* origRT = nullptr;
                ID3D11DepthStencilView* origDS = nullptr;
                D3D11_VIEWPORT origVP = {};
                UINT numVP = 1;
                ctx->OMGetRenderTargets(1, &origRT, &origDS);
                ctx->RSGetViewports(&numVP, &origVP);
                ctx->OMSetRenderTargets(1, &pvRTV, pvDSV);
                D3D11_VIEWPORT vp = {};
                vp.Width = (float)PV_W;
                vp.Height = (float)PV_H;
                vp.MinDepth = 0.0f;
                vp.MaxDepth = 1.0f;
                ctx->RSSetViewports(1, &vp);
                oDrawIndexed(ctx, IndexCount, StartIndexLocation, BaseVertexLocation);
                ctx->OMSetRenderTargets(1, &origRT, origDS);
                ctx->RSSetViewports(1, &origVP);
                if (origRT) origRT->Release();
                if (origDS) origDS->Release();

                captureCount++;
                if (captureCount >= 8) frameCaptured = true;

                insideHook = false;
            }
        }
        oDrawIndexed(ctx, IndexCount, StartIndexLocation, BaseVertexLocation);
    }

    inline bool Init(ID3D11Device* dev, ID3D11DeviceContext* ctx) {
        pDev = dev;
        pCtx = ctx;

        if (!CreateResources()) return false;
        void** vtable = *(void***)ctx;
        DWORD oldP;
        VirtualProtect(&vtable[12], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldP);
        oDrawIndexed = (fnDrawIndexed)vtable[12];
        vtable[12] = (void*)hkDrawIndexed;
        VirtualProtect(&vtable[12], sizeof(void*), oldP, &oldP);

        initialized = true;
        return true;
    }
    inline ID3D11ShaderResourceView* GetPreviewTexture() {
        return pvSRV;
    }

    inline void Shutdown() {
        if (oDrawIndexed && pCtx) {
            void** vtable = *(void***)pCtx;
            DWORD oldP;
            VirtualProtect(&vtable[12], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldP);
            vtable[12] = (void*)oDrawIndexed;
            VirtualProtect(&vtable[12], sizeof(void*), oldP, &oldP);
        }
        FreeResources();
        initialized = false;
    }
}
