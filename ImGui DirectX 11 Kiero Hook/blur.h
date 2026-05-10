#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

namespace Blur
{
    inline const char* vsSource = R"(
struct VS_OUT {
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD;
};
VS_OUT main(uint id : SV_VertexID) {
    VS_OUT o;
    o.uv = float2((id << 1) & 2, id & 2);
    o.pos = float4(o.uv * float2(2, -2) + float2(-1, 1), 0, 1);
    return o;
}
)";

    inline const char* psSource = R"(
Texture2D    g_tex : register(t0);
SamplerState g_smp : register(s0);

cbuffer CB : register(b0) {
    float2 direction;
    float2 texelSize;
};

float4 main(float4 pos : SV_POSITION, float2 uv : TEXCOORD) : SV_TARGET {
    static const float w[5] = { 0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216 };
    float4 c = g_tex.Sample(g_smp, uv) * w[0];
    [unroll] for (int i = 1; i < 5; i++) {
        float2 off = direction * texelSize * (float)i * 1.5;
        c += g_tex.Sample(g_smp, uv + off) * w[i];
        c += g_tex.Sample(g_smp, uv - off) * w[i];
    }
    return c;
}
)";

    struct BlurCB { float dir[2]; float texel[2]; };

    inline ID3D11Device*             g_dev = nullptr;
    inline ID3D11DeviceContext*      g_ctx = nullptr;

    inline ID3D11Texture2D*          g_copy = nullptr;
    inline ID3D11ShaderResourceView* g_copySRV = nullptr;

    inline ID3D11Texture2D*          g_tA = nullptr;
    inline ID3D11RenderTargetView*   g_rtvA = nullptr;
    inline ID3D11ShaderResourceView* g_srvA = nullptr;

    inline ID3D11Texture2D*          g_tB = nullptr;
    inline ID3D11RenderTargetView*   g_rtvB = nullptr;
    inline ID3D11ShaderResourceView* g_srvB = nullptr;

    inline ID3D11VertexShader*       g_vs = nullptr;
    inline ID3D11PixelShader*        g_ps = nullptr;
    inline ID3D11Buffer*             g_cb = nullptr;
    inline ID3D11SamplerState*       g_smp = nullptr;
    inline ID3D11RasterizerState*    g_rs = nullptr;
    inline ID3D11DepthStencilState*  g_ds = nullptr;
    inline ID3D11BlendState*         g_bs = nullptr;

    inline int  g_w = 0, g_h = 0;
    inline DXGI_FORMAT g_fmt = DXGI_FORMAT_R8G8B8A8_UNORM;
    inline bool g_ok = false;
    inline int  g_passes = 4;

    inline ID3D11ShaderResourceView* GetSRV() { return g_srvB; }

    inline void ReleaseTextures()
    {
        #define SAFE_REL(x) if(x){x->Release();x=nullptr;}
        SAFE_REL(g_copy); SAFE_REL(g_copySRV);
        SAFE_REL(g_tA); SAFE_REL(g_rtvA); SAFE_REL(g_srvA);
        SAFE_REL(g_tB); SAFE_REL(g_rtvB); SAFE_REL(g_srvB);
        #undef SAFE_REL
    }

    inline bool MakeTextures(int w, int h, DXGI_FORMAT fmt)
    {
        ReleaseTextures();
        g_w = w; g_h = h; g_fmt = fmt;

        D3D11_TEXTURE2D_DESC d = {};
        d.Width = w; d.Height = h;
        d.MipLevels = 1; d.ArraySize = 1;
        d.Format = fmt;
        d.SampleDesc.Count = 1;
        d.Usage = D3D11_USAGE_DEFAULT;

        d.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        if (FAILED(g_dev->CreateTexture2D(&d, nullptr, &g_copy))) return false;
        if (FAILED(g_dev->CreateShaderResourceView(g_copy, nullptr, &g_copySRV))) return false;

        d.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        if (FAILED(g_dev->CreateTexture2D(&d, nullptr, &g_tA))) return false;
        if (FAILED(g_dev->CreateRenderTargetView(g_tA, nullptr, &g_rtvA))) return false;
        if (FAILED(g_dev->CreateShaderResourceView(g_tA, nullptr, &g_srvA))) return false;

        if (FAILED(g_dev->CreateTexture2D(&d, nullptr, &g_tB))) return false;
        if (FAILED(g_dev->CreateRenderTargetView(g_tB, nullptr, &g_rtvB))) return false;
        if (FAILED(g_dev->CreateShaderResourceView(g_tB, nullptr, &g_srvB))) return false;

        return true;
    }

    inline bool Init(ID3D11Device* dev, ID3D11DeviceContext* ctx)
    {
        g_dev = dev; g_ctx = ctx;

        ID3DBlob* blob = nullptr; ID3DBlob* err = nullptr;

        if (FAILED(D3DCompile(vsSource, strlen(vsSource), 0, 0, 0, "main", "vs_4_0", 0, 0, &blob, &err)))
        { if (err) err->Release(); return false; }
        dev->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), 0, &g_vs);
        blob->Release();

        if (FAILED(D3DCompile(psSource, strlen(psSource), 0, 0, 0, "main", "ps_4_0", 0, 0, &blob, &err)))
        { if (err) err->Release(); return false; }
        dev->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), 0, &g_ps);
        blob->Release();

        D3D11_BUFFER_DESC cbd = {};
        cbd.ByteWidth = 16;
        cbd.Usage = D3D11_USAGE_DYNAMIC;
        cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        dev->CreateBuffer(&cbd, nullptr, &g_cb);

        D3D11_SAMPLER_DESC sd = {};
        sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        dev->CreateSamplerState(&sd, &g_smp);

        D3D11_RASTERIZER_DESC rd = {};
        rd.FillMode = D3D11_FILL_SOLID; rd.CullMode = D3D11_CULL_NONE;
        dev->CreateRasterizerState(&rd, &g_rs);

        D3D11_DEPTH_STENCIL_DESC dd = {}; dd.DepthEnable = FALSE;
        dev->CreateDepthStencilState(&dd, &g_ds);

        D3D11_BLEND_DESC bd = {};
        bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        dev->CreateBlendState(&bd, &g_bs);

        g_ok = true;
        return true;
    }

    inline void SetCB(float dx, float dy)
    {
        D3D11_MAPPED_SUBRESOURCE m;
        if (SUCCEEDED(g_ctx->Map(g_cb, 0, D3D11_MAP_WRITE_DISCARD, 0, &m))) {
            BlurCB* p = (BlurCB*)m.pData;
            p->dir[0] = dx; p->dir[1] = dy;
            p->texel[0] = 1.0f / g_w; p->texel[1] = 1.0f / g_h;
            g_ctx->Unmap(g_cb, 0);
        }
    }

    inline void Apply(IDXGISwapChain* sc)
    {
        if (!g_ok) return;

        ID3D11Texture2D* bb = nullptr;
        if (FAILED(sc->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&bb))) return;

        D3D11_TEXTURE2D_DESC bd;
        bb->GetDesc(&bd);

        if ((int)bd.Width != g_w || (int)bd.Height != g_h)
            if (!MakeTextures(bd.Width, bd.Height, bd.Format)) { bb->Release(); return; }

        if (bd.SampleDesc.Count > 1)
            g_ctx->ResolveSubresource(g_copy, 0, bb, 0, bd.Format);
        else
            g_ctx->CopyResource(g_copy, bb);
        bb->Release();

        g_ctx->VSSetShader(g_vs, 0, 0);
        g_ctx->PSSetShader(g_ps, 0, 0);
        g_ctx->PSSetSamplers(0, 1, &g_smp);
        g_ctx->PSSetConstantBuffers(0, 1, &g_cb);
        g_ctx->RSSetState(g_rs);
        g_ctx->OMSetDepthStencilState(g_ds, 0);
        float bf[4] = {};
        g_ctx->OMSetBlendState(g_bs, bf, 0xFFFFFFFF);
        g_ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        g_ctx->IASetInputLayout(nullptr);

        D3D11_VIEWPORT vp = {}; vp.Width = (float)g_w; vp.Height = (float)g_h; vp.MaxDepth = 1;
        g_ctx->RSSetViewports(1, &vp);

        ID3D11ShaderResourceView* nul = nullptr;

        for (int i = 0; i < g_passes; i++) {
            ID3D11ShaderResourceView* src = (i == 0) ? g_copySRV : g_srvB;
            SetCB(1, 0);
            g_ctx->OMSetRenderTargets(1, &g_rtvA, nullptr);
            g_ctx->PSSetShaderResources(0, 1, &src);
            g_ctx->Draw(3, 0);
            g_ctx->PSSetShaderResources(0, 1, &nul);
            SetCB(0, 1);
            g_ctx->OMSetRenderTargets(1, &g_rtvB, nullptr);
            g_ctx->PSSetShaderResources(0, 1, &g_srvA);
            g_ctx->Draw(3, 0);
            g_ctx->PSSetShaderResources(0, 1, &nul);
        }

        ID3D11RenderTargetView* nulRTV = nullptr;
        g_ctx->OMSetRenderTargets(1, &nulRTV, nullptr);
    }
}
