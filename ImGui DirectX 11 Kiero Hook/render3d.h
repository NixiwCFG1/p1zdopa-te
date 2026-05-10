#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include <cmath>
#include "game.h"

namespace Render3D
{
    struct LineVert { float x, y, z, r, g, b, a; };
    struct BlurCB   { float dirX, dirY, texelX, texelY; };
    struct LineSeg {
        float sx0, sy0, z0, sx1, sy1, z1;
        float r, g, b, a;
    };

    inline ID3D11VertexShader*  pLineVS = nullptr;
    inline ID3D11PixelShader*   pLinePS = nullptr;
    inline ID3D11InputLayout*   pLineIL = nullptr;
    inline ID3D11Buffer*        pLineVB = nullptr;
    inline ID3D11VertexShader*  pFsVS   = nullptr;
    inline ID3D11PixelShader*   pBlurPS = nullptr;
    inline ID3D11PixelShader*   pGlowPS = nullptr;
    inline ID3D11Buffer*        pBlurCB = nullptr;
    inline ID3D11SamplerState*  pSamp   = nullptr;
    inline ID3D11Texture2D*          pBTex[2] = {};
    inline ID3D11RenderTargetView*   pBRTV[2] = {};
    inline ID3D11ShaderResourceView* pBSRV[2] = {};
    inline int bW = 0, bH = 0;
    inline ID3D11RasterizerState*   pRS       = nullptr;
    inline ID3D11DepthStencilState* pDSSoff   = nullptr;
    inline ID3D11DepthStencilState* pDSSon    = nullptr;
    inline ID3D11BlendState*        pAlphaBS  = nullptr;
    inline ID3D11BlendState*        pAddBS    = nullptr;

    inline ID3D11Device* pDev = nullptr;
    constexpr int MAX_VERTICES = 65536;
    constexpr int MAX_LINES = 8192;
    inline LineVert vertices[MAX_VERTICES];
    inline int vertexCount = 0;
    inline LineSeg lines[MAX_LINES];
    inline int lineCount = 0;
    inline bool initialized = false;
    inline float screenW = 1920, screenH = 1080;
    inline float lineThickness = 2.0f;
    inline int   blurPasses = 5;
    inline float glowIntensity = 8.0f;
    inline float glowWidthMul = 10.0f;

    inline const char* lineSrc = R"(
        struct VI { float3 p : POSITION; float4 c : COLOR; };
        struct VO { float4 p : SV_POSITION; float4 c : COLOR; };
        VO VSLine(VI i) { VO o; o.p = float4(i.p.x, i.p.y, i.p.z, 1.0); o.c = i.c; return o; }
        float4 PSLine(VO i) : SV_Target { return i.c; }
    )";

    inline const char* bloomSrc = R"(
        Texture2D t : register(t0);
        SamplerState s : register(s0);
        cbuffer B : register(b0) { float2 dir; float2 tx; };
        struct O { float4 p : SV_POSITION; float2 u : TEXCOORD; };
        O VSFs(uint id : SV_VertexID) {
            O o;
            o.u = float2((id << 1) & 2, id & 2);
            o.p = float4(o.u * float2(2,-2) + float2(-1,1), 0.5, 1);
            return o;
        }
        float4 PSBlur(O i) : SV_Target {
            float w[5] = { 0.227027, 0.194595, 0.121622, 0.054054, 0.016216 };
            float4 c = t.Sample(s, i.u) * w[0];
            [unroll] for (int j = 1; j < 5; j++) {
                float2 off = dir * tx * (float)j * 3.5;
                c += t.Sample(s, i.u + off) * w[j];
                c += t.Sample(s, i.u - off) * w[j];
            }
            return c;
        }
        float4 PSGlow(O i) : SV_Target {
            float4 c = t.Sample(s, i.u);
            float3 g = 1.0 - exp(-c.rgb * 18.0);
            return float4(g, 1.0);
        }
    )";

    inline ID3DBlob* Compile(const char* src, const char* fn, const char* tgt) {
        ID3DBlob* b = nullptr; ID3DBlob* e = nullptr;
        D3DCompile(src, strlen(src), 0, 0, 0, fn, tgt, 0, 0, &b, &e);
        if (e) e->Release();
        return b;
    }

    inline void FreeBloom() {
        for (int i = 0; i < 2; i++) {
            if (pBTex[i]) { pBTex[i]->Release(); pBTex[i] = nullptr; }
            if (pBRTV[i]) { pBRTV[i]->Release(); pBRTV[i] = nullptr; }
            if (pBSRV[i]) { pBSRV[i]->Release(); pBSRV[i] = nullptr; }
        }
        bW = bH = 0;
    }

    inline bool MakeBloom(int w, int h) {
        if (bW == w && bH == h && pBTex[0]) return true;
        FreeBloom();
        D3D11_TEXTURE2D_DESC td = {};
        td.Width = w; td.Height = h; td.MipLevels = 1; td.ArraySize = 1;
        td.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        td.SampleDesc.Count = 1; td.Usage = D3D11_USAGE_DEFAULT;
        td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        for (int i = 0; i < 2; i++) {
            if (FAILED(pDev->CreateTexture2D(&td, 0, &pBTex[i]))) return false;
            pDev->CreateRenderTargetView(pBTex[i], 0, &pBRTV[i]);
            pDev->CreateShaderResourceView(pBTex[i], 0, &pBSRV[i]);
        }
        bW = w; bH = h;
        return true;
    }

    inline bool Init(ID3D11Device* dev, ID3D11DeviceContext*) {
        if (initialized) return true;
        pDev = dev;
        auto v1 = Compile(lineSrc,  "VSLine", "vs_5_0");
        auto p1 = Compile(lineSrc,  "PSLine", "ps_5_0");
        auto v2 = Compile(bloomSrc, "VSFs",   "vs_5_0");
        auto p2 = Compile(bloomSrc, "PSBlur", "ps_5_0");
        auto p3 = Compile(bloomSrc, "PSGlow", "ps_5_0");
        if (!v1||!p1||!v2||!p2||!p3) return false;

        dev->CreateVertexShader(v1->GetBufferPointer(), v1->GetBufferSize(), 0, &pLineVS);
        dev->CreatePixelShader (p1->GetBufferPointer(), p1->GetBufferSize(), 0, &pLinePS);
        dev->CreateVertexShader(v2->GetBufferPointer(), v2->GetBufferSize(), 0, &pFsVS);
        dev->CreatePixelShader (p2->GetBufferPointer(), p2->GetBufferSize(), 0, &pBlurPS);
        dev->CreatePixelShader (p3->GetBufferPointer(), p3->GetBufferSize(), 0, &pGlowPS);

        D3D11_INPUT_ELEMENT_DESC il[] = {
            {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,   0, 0,D3D11_INPUT_PER_VERTEX_DATA,0},
            {"COLOR",   0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0},
        };
        dev->CreateInputLayout(il, 2, v1->GetBufferPointer(), v1->GetBufferSize(), &pLineIL);
        v1->Release(); p1->Release(); v2->Release(); p2->Release(); p3->Release();

        D3D11_BUFFER_DESC bd = {};
        bd.Usage = D3D11_USAGE_DYNAMIC; bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        bd.ByteWidth = sizeof(vertices); bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        dev->CreateBuffer(&bd, 0, &pLineVB);
        bd.ByteWidth = 16; bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        dev->CreateBuffer(&bd, 0, &pBlurCB);

        D3D11_SAMPLER_DESC sd = {};
        sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        dev->CreateSamplerState(&sd, &pSamp);

        D3D11_RASTERIZER_DESC rd = {};
        rd.FillMode = D3D11_FILL_SOLID; rd.CullMode = D3D11_CULL_NONE; rd.DepthClipEnable = FALSE;
        dev->CreateRasterizerState(&rd, &pRS);

        D3D11_DEPTH_STENCIL_DESC dd = {};
        dd.DepthEnable = FALSE; dd.StencilEnable = FALSE;
        dev->CreateDepthStencilState(&dd, &pDSSoff);

        dd.DepthEnable = TRUE;
        dd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        dd.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
        dev->CreateDepthStencilState(&dd, &pDSSon);

        D3D11_BLEND_DESC bl = {};
        bl.RenderTarget[0].BlendEnable = TRUE;
        bl.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        bl.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        bl.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        bl.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        bl.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        bl.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        bl.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        dev->CreateBlendState(&bl, &pAlphaBS);

        bl.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
        bl.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
        dev->CreateBlendState(&bl, &pAddBS);

        initialized = true;
        return true;
    }

    inline float toNdcX(float sx) { return (sx / screenW) * 2.0f - 1.0f; }
    inline float toNdcY(float sy) { return 1.0f - (sy / screenH) * 2.0f; }
    inline int BuildQuads(float widthMul) {
        vertexCount = 0;
        for (int i = 0; i < lineCount; i++) {
            if (vertexCount + 6 > MAX_VERTICES) break;
            LineSeg& L = lines[i];
            float dx = L.sx1-L.sx0, dy = L.sy1-L.sy0;
            float len = sqrtf(dx*dx+dy*dy);
            if (len < 0.5f) continue;
            float nx = -dy/len, ny = dx/len;
            float hw = (lineThickness * widthMul) * 0.5f;
            float hx = nx*hw, hy = ny*hw;
            float ax0=toNdcX(L.sx0-hx), ay0=toNdcY(L.sy0-hy);
            float bx0=toNdcX(L.sx0+hx), by0=toNdcY(L.sy0+hy);
            float bx1=toNdcX(L.sx1+hx), by1=toNdcY(L.sy1+hy);
            float ax1=toNdcX(L.sx1-hx), ay1=toNdcY(L.sy1-hy);
            vertices[vertexCount++]={ax0,ay0,L.z0, L.r,L.g,L.b,L.a};
            vertices[vertexCount++]={bx0,by0,L.z0, L.r,L.g,L.b,L.a};
            vertices[vertexCount++]={bx1,by1,L.z1, L.r,L.g,L.b,L.a};
            vertices[vertexCount++]={bx1,by1,L.z1, L.r,L.g,L.b,L.a};
            vertices[vertexCount++]={ax1,ay1,L.z1, L.r,L.g,L.b,L.a};
            vertices[vertexCount++]={ax0,ay0,L.z0, L.r,L.g,L.b,L.a};
        }
        return vertexCount;
    }

    inline void DrawLine3D(const Vec3& start, const Vec3& end, float r, float g, float b, float a=1.f) {
        if (lineCount >= MAX_LINES) return;
        Vec2 s0, s1; float z0, z1;
        if (!Game::WorldToScreenDepth(start, s0, z0, screenW, screenH)) return;
        if (!Game::WorldToScreenDepth(end,   s1, z1, screenW, screenH)) return;
        lines[lineCount++] = { s0.x, s0.y, z0, s1.x, s1.y, z1, r, g, b, a };
    }

    inline void DrawLine3D(const Vec3& start, const Vec3& end, ImU32 col) {
        float r=((col>>0)&0xFF)/255.f, g=((col>>8)&0xFF)/255.f;
        float b=((col>>16)&0xFF)/255.f, a=((col>>24)&0xFF)/255.f;
        DrawLine3D(start,end,r,g,b,a);
    }

    inline void UploadAndDraw(ID3D11DeviceContext* ctx, ID3D11RenderTargetView* rt,
                              ID3D11DepthStencilView* dsv, float w, float h,
                              ID3D11BlendState* bs, ID3D11DepthStencilState* dss) {
        if (vertexCount == 0) return;
        D3D11_MAPPED_SUBRESOURCE m;
        if (SUCCEEDED(ctx->Map(pLineVB,0,D3D11_MAP_WRITE_DISCARD,0,&m))) {
            memcpy(m.pData, vertices, vertexCount*sizeof(LineVert));
            ctx->Unmap(pLineVB,0);
        }
        ctx->OMSetRenderTargets(1, &rt, dsv);
        D3D11_VIEWPORT vp={}; vp.Width=w; vp.Height=h; vp.MaxDepth=1;
        ctx->RSSetViewports(1, &vp);
        UINT stride=sizeof(LineVert), off=0;
        ctx->IASetVertexBuffers(0,1,&pLineVB,&stride,&off);
        ctx->IASetInputLayout(pLineIL);
        ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        ctx->VSSetShader(pLineVS,0,0); ctx->PSSetShader(pLinePS,0,0);
        ctx->RSSetState(pRS); ctx->OMSetDepthStencilState(dss,0);
        float bf[4]={}; ctx->OMSetBlendState(bs,bf,0xFFFFFFFF);
        ctx->Draw(vertexCount, 0);
    }

    inline void BlurPass(ID3D11DeviceContext* ctx, ID3D11ShaderResourceView* src,
                         ID3D11RenderTargetView* dst, float w, float h, float dx, float dy) {
        float cl[4]={}; ctx->ClearRenderTargetView(dst, cl);
        ctx->OMSetRenderTargets(1, &dst, nullptr);
        D3D11_VIEWPORT vp={}; vp.Width=w; vp.Height=h; vp.MaxDepth=1;
        ctx->RSSetViewports(1, &vp);
        D3D11_MAPPED_SUBRESOURCE m;
        if (SUCCEEDED(ctx->Map(pBlurCB,0,D3D11_MAP_WRITE_DISCARD,0,&m))) {
            BlurCB cb={dx,dy,1.f/w,1.f/h}; memcpy(m.pData,&cb,16);
            ctx->Unmap(pBlurCB,0);
        }
        ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        ctx->IASetInputLayout(nullptr);
        ctx->VSSetShader(pFsVS,0,0); ctx->PSSetShader(pBlurPS,0,0);
        ctx->PSSetShaderResources(0,1,&src); ctx->PSSetSamplers(0,1,&pSamp);
        ctx->PSSetConstantBuffers(0,1,&pBlurCB);
        ctx->RSSetState(pRS); ctx->OMSetDepthStencilState(pDSSoff,0);
        float bf[4]={}; ctx->OMSetBlendState(pAlphaBS,bf,0xFFFFFFFF);
        ctx->Draw(3, 0);
        ID3D11ShaderResourceView* nul=nullptr; ctx->PSSetShaderResources(0,1,&nul);
    }

    inline void Flush(ID3D11DeviceContext* ctx, ID3D11RenderTargetView* bb,
                      ID3D11DepthStencilView* gameDSV, float sw, float sh) {
        screenW=sw; screenH=sh;
        if (!initialized || lineCount==0 || !ctx) { lineCount=0; return; }
        if (!MakeBloom((int)sw,(int)sh)) { lineCount=0; return; }
        BuildQuads(glowWidthMul);
        float cl[4]={}; ctx->ClearRenderTargetView(pBRTV[0], cl);
        UploadAndDraw(ctx, pBRTV[0], nullptr, sw, sh, pAlphaBS, pDSSoff);
        for (int i = 0; i < blurPasses; i++) {
            BlurPass(ctx, pBSRV[0], pBRTV[1], sw, sh, 1, 0);
            BlurPass(ctx, pBSRV[1], pBRTV[0], sw, sh, 0, 1);
        }
        ctx->OMSetRenderTargets(1, &bb, nullptr);
        D3D11_VIEWPORT vp={}; vp.Width=sw; vp.Height=sh; vp.MaxDepth=1;
        ctx->RSSetViewports(1, &vp);
        ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        ctx->IASetInputLayout(nullptr);
        ctx->VSSetShader(pFsVS,0,0); ctx->PSSetShader(pGlowPS,0,0);
        ctx->PSSetShaderResources(0,1,&pBSRV[0]); ctx->PSSetSamplers(0,1,&pSamp);
        ctx->RSSetState(pRS); ctx->OMSetDepthStencilState(pDSSoff,0);
        float bf[4]={}; ctx->OMSetBlendState(pAddBS,bf,0xFFFFFFFF);
        ctx->Draw(3, 0);
        ID3D11ShaderResourceView* nul=nullptr; ctx->PSSetShaderResources(0,1,&nul);

        lineCount = 0;
    }

    inline void Shutdown() {
        FreeBloom();
        if (pLineVS) pLineVS->Release(); if (pLinePS) pLinePS->Release();
        if (pLineIL) pLineIL->Release(); if (pLineVB) pLineVB->Release();
        if (pFsVS)   pFsVS->Release();   if (pBlurPS) pBlurPS->Release();
        if (pGlowPS) pGlowPS->Release(); if (pBlurCB) pBlurCB->Release();
        if (pSamp)   pSamp->Release();   if (pRS)     pRS->Release();
        if (pDSSoff) pDSSoff->Release();  if (pDSSon)  pDSSon->Release();
        if (pAlphaBS)pAlphaBS->Release(); if (pAddBS)  pAddBS->Release();
        initialized = false;
    }
}
