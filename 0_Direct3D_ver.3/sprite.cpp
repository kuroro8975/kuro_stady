

#include <d3dx9.h>
#include <math.h>
#include "mydirect3d.h"
#include "texture.h"


/*------------------------------------------------------------------------------
   構造体宣言
------------------------------------------------------------------------------*/
// ２Dポリゴン頂点構造体
typedef struct Vertex2D_tag
{
    D3DXVECTOR4 position; // 頂点座標（座標変換済み頂点）
	D3DCOLOR color;
	D3DXVECTOR2 texcoord;
} Vertex2D;
#define FVF_VERTEX2D (D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1) // ２Dポリゴン頂点フォーマット


/*------------------------------------------------------------------------------
   グローバル変数宣言
------------------------------------------------------------------------------*/
static D3DCOLOR g_Color = 0xffffffff;
// static D3DCOLOR g_Color = D3DCOLOR_RGBA(255, 255, 255, 255);



/*------------------------------------------------------------------------------
   関数定義
------------------------------------------------------------------------------*/

// スプライトポリゴンの頂点カラー設定
void Sprite_SetColor(D3DCOLOR color)
{
	g_Color = color;
}


// スプライト描画
// ※テクスチャ切り取り幅、高さと同じ大きさのスプライトを指定座標に描画する
void Sprite_Draw(TextureIndex texture_index, float dx, float dy, int tx, int ty,int tw, int th)
{
	LPDIRECT3DDEVICE9 pDevice = MyDirect3D_GetDevice();
    if( !pDevice ) return;

	float w = (float)Texture_GetWidth(texture_index);
	float h = (float)Texture_GetHeight(texture_index);

    // UV座標計算
    float u[2], v[2];
	u[0] = (float)tx / w;
	v[0] = (float)ty / h;
	u[1] = (float)(tx + tw) / w;
	v[1] = (float)(ty + th) / h;

    Vertex2D vertexes[] = {
        { D3DXVECTOR4(dx      - 0.5f, dy      - 0.5f, 0.0f, 1.0f), g_Color, D3DXVECTOR2(u[0], v[0]) },
        { D3DXVECTOR4(dx + tw - 0.5f, dy      - 0.5f, 0.0f, 1.0f), g_Color, D3DXVECTOR2(u[1], v[0]) },
		{ D3DXVECTOR4(dx      - 0.5f, dy + th - 0.5f, 0.0f, 1.0f), g_Color, D3DXVECTOR2(u[0], v[1]) },
		{ D3DXVECTOR4(dx + tw - 0.5f, dy + th - 0.5f, 0.0f, 1.0f), g_Color, D3DXVECTOR2(u[1], v[1]) },
    };

    pDevice->SetFVF(FVF_VERTEX2D);
	pDevice->SetTexture(0, Texture_GetTexture(texture_index));
    pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertexes, sizeof(Vertex2D));
}
