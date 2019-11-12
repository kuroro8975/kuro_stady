

#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dinput8.lib")

#include <Windows.h>
#include "common.h"
#include "mydirect3d.h"
#include "texture.h"
#include "sprite.h"
#include <d3dx9.h>



//===============================================
//ここを追加　3D用追加コード

//3Dポリゴン頂点フォーマット構造体を定義
typedef struct
{
	D3DXVECTOR3 vtx;//頂点座標
	D3DXVECTOR3 nor;//法線ベクトル
	D3DCOLOR diffuse;//反射光
	D3DXVECTOR2 tex;//テクスチャ座標
}VERTEX_3D;

//3Dポリゴン頂点フォーマット(頂点座標[3D]／法線／反射光／テクスチャ座標)
#define FVF_VERTEX_3D (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX1)

#define MAX_GURID (50)
VERTEX_3D Gurid[MAX_GURID];

//プロジェクト用パラメータ
#define VIEW_ANGLE (D3DXToRadian(45.0f))//ビュー平面の視野角
#define VIEW_ASPECT ((float)SCREEN_WIDTH/(float)SCREEN_HEIGHT)//ビュー平面のアスペクト比
#define VIEW_NEAR_Z (1.0f)//ビュー平面のNearZ値
#define VIEW_FAR_Z (1000.0f)//ビュー平面のFarZ値

//カメラ用パラメータ
D3DXVECTOR3 g_posCamaraEye;//カメラの視点
D3DXVECTOR3 g_posCamaraAt;//カメラの注視点
D3DXVECTOR3 g_vecCamaraUp;//カメラの上方向

//プロジェクション＆カメラ行列
D3DXMATRIX g_mtxView;//ビューマトリックス
D3DXMATRIX g_mtxProjection;//プロジェクションマトリックス

//サンプルポリゴンの表示自体に関するパラメータ
D3DXVECTOR3 g_pos;//ポリゴンの位置(四角形の中央座標)
D3DXVECTOR3 g_rot;//ポリゴンの向き(回転)
D3DXVECTOR3 g_scl;//ポリゴンの大きさ(スケール)

D3DXMATRIX g_mtxWorld;//ワールドマトリックス

LPDIRECT3DVERTEXBUFFER9 g_pD3DVtxBuff; //頂点バッファ



//ここまで3D用追加コード
//===============================================



/*------------------------------------------------------------------------------
   定数定義
------------------------------------------------------------------------------*/
#define CLASS_NAME     "GameWindow"       // ウインドウクラスの名前
#define WINDOW_CAPTION "ゲームウィンドウ" // ウィンドウの名前

float pos_x = 0.0f;
float posadd_x = 0.1f;

/*------------------------------------------------------------------------------
   プロトタイプ宣言
------------------------------------------------------------------------------*/
// ウィンドウプロシージャ(コールバック関数)
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// ゲームの初期化関数
// 戻り値:初期化に失敗したときfalse
static bool Initialize(void);
// ゲームの更新関数
static void Update(void);
// ゲームの描画関数
static void Draw(void);
// ゲームの終了処理
static void Finalize(void);


/*------------------------------------------------------------------------------
   グローバル変数宣言
------------------------------------------------------------------------------*/
static HWND g_hWnd;             // ウィンドウハンドル
static float g_Rotation = 0.0f; // スプライト回転角度 

/*------------------------------------------------------------------------------
   関数定義
------------------------------------------------------------------------------*/

//#######################################################################
// メイン関数
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // 使用しない一時変数を明示
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // ウィンドウクラス構造体の設定
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;                          // ウィンドウプロシージャの指定
    wc.lpszClassName = CLASS_NAME;                     // クラス名の設定
    wc.hInstance = hInstance;                          // インスタンスハンドルの指定
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);          // マウスカーソルを指定
    wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND + 1); // ウインドウのクライアント領域の背景色を設定

    // クラス登録
    RegisterClass(&wc);


    // ウィンドウスタイル
    DWORD window_style = WS_OVERLAPPEDWINDOW & ~(WS_MAXIMIZEBOX | WS_THICKFRAME);

    // 基本矩形座標
    RECT window_rect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };

    // 指定したクライアント領域を確保するために新たな矩形座標を計算
    AdjustWindowRect(&window_rect, window_style, FALSE);

    // 新たなWindowの矩形座標から幅と高さを算出
    int window_width = window_rect.right - window_rect.left;
    int window_height = window_rect.bottom - window_rect.top;

    // プライマリモニターの画面解像度取得
    int desktop_width = GetSystemMetrics(SM_CXSCREEN);
    int desktop_height = GetSystemMetrics(SM_CYSCREEN);

    // デスクトップの真ん中にウィンドウが生成されるように座標を計算
    // ※ただし万が一、デスクトップよりウィンドウが大きい場合は左上に表示
    int window_x = max((desktop_width - window_width) / 2, 0);
    int window_y = max((desktop_height - window_height) / 2, 0);

    // ウィンドウの生成
    g_hWnd = CreateWindow(
        CLASS_NAME,     // ウィンドウクラス
        WINDOW_CAPTION, // ウィンドウテキスト
        window_style,   // ウィンドウスタイル
        window_x,       // ウィンドウ座標x
        window_y,       // ウィンドウ座標y
        window_width,   // ウィンドウの幅
        window_height,  // ウィンドウの高さ
        NULL,           // 親ウィンドウハンドル
        NULL,           // メニューハンドル
        hInstance,      // インスタンスハンドル
        NULL            // 追加のアプリケーションデータ
    );

    if( g_hWnd == NULL ) {
        // ウィンドウハンドルが何らかの理由で生成出来なかった
        return -1;
    }

    // 指定のウィンドウハンドルのウィンドウを指定の方法で表示
    ShowWindow(g_hWnd, nCmdShow);


	// ゲームの初期化(Direct3Dの初期化)
	if( !Initialize() ) {
        // ゲームの初期化に失敗した
		return -1;
	}

    // Windowsゲーム用メインループ
    MSG msg = {}; // msg.message == WM_NULL
    while( WM_QUIT != msg.message ) {

        if( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) ) {
            // メッセージがある場合はメッセージ処理を優先
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            // ゲームの更新
			Update();
			// ゲームの描画
			Draw();
        }
    }

	// ゲームの終了処理(Direct3Dの終了処理)
	Finalize();

    return (int)msg.wParam;
}

//#######################################################################
// ウィンドウプロシージャ(コールバック関数)
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch( uMsg ) {
        case WM_KEYDOWN:
            if( wParam == VK_ESCAPE ) {
                SendMessage(hWnd, WM_CLOSE, 0, 0); // WM_CLOSEメッセージの送信
            }
            break;

        case WM_CLOSE:
            if( MessageBox(hWnd, "本当に終了してよろしいですか？", "確認", MB_OKCANCEL | MB_DEFBUTTON2) == IDOK ) {
                DestroyWindow(hWnd); // 指定のウィンドウにWM_DESTROYメッセージを送る
            }
            return 0; // DefWindowProc関数にメッセージを流さず終了することによって何もなかったことにする

        case WM_DESTROY: // ウィンドウの破棄メッセージ
            PostQuitMessage(0); // WM_QUITメッセージの送信
            return 0;
    };

    // 通常メッセージ処理はこの関数に任せる
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


//#######################################################################
// ゲームの初期化関数
bool Initialize(void)
{
    // Direct3Dラッパーモジュールの初期化
    if( !MyDirect3D_Initialize(g_hWnd) ) {
        return false;
    }

    // テクスチャの読み込み
    if( Texture_Load() > 0 ) {
        MessageBox(g_hWnd, "いくつか読み込めなかったテクスチャファイルがあります", "エラー", MB_OK);
    }


	//================================================================
	//ここを追加　3Dポリゴン用頂点の準備

	LPDIRECT3DDEVICE9 pDevice = MyDirect3D_GetDevice();
	//オブジェクトの頂点バッファを生成
	pDevice->CreateVertexBuffer(sizeof(VERTEX_3D) * 36,//頂点データ用に確保するバッファサイズ(バイト単位)
		D3DUSAGE_WRITEONLY,  //頂点バッファの使用法
		FVF_VERTEX_3D,       //使用する頂点フォーマット
		D3DPOOL_MANAGED,     //リソースのバッファを保持するメモリクラスを指定
		&g_pD3DVtxBuff,      //頂点バッファインターフェイスへのポインタ
		NULL);               //NULLに設定

	//頂点バッファの中身を埋める
	VERTEX_3D *pVtx;

	//頂点データの範囲をロックし、頂点バッファへのポインタを取得
	g_pD3DVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

	//頂点座標の設定------------------------------------
	//正面
	pVtx[0].vtx = D3DXVECTOR3(-0.5f, 0.5f, -0.5f);
	pVtx[1].vtx = D3DXVECTOR3(0.5f, 0.5f, -0.5f);
	pVtx[2].vtx = D3DXVECTOR3(-0.5f, -0.5f, -0.5f);
	pVtx[3].vtx = D3DXVECTOR3(0.5f, 0.5f, -0.5f);
	pVtx[4].vtx = D3DXVECTOR3(0.5f, -0.5f, -0.5f);
	pVtx[5].vtx = D3DXVECTOR3(-0.5f, -0.5f, -0.5f);
	//上
	pVtx[6].vtx = D3DXVECTOR3(0.5f, 0.5f, -0.5f);
	pVtx[7].vtx = D3DXVECTOR3(-0.5f, 0.5f, -0.5f);
	pVtx[8].vtx = D3DXVECTOR3(0.5f, 0.5f, 0.5f);
	pVtx[9].vtx = D3DXVECTOR3(-0.5f, 0.5f, -0.5f);
	pVtx[10].vtx = D3DXVECTOR3(-0.5f, 0.5f, 0.5f);
	pVtx[11].vtx = D3DXVECTOR3(0.5f, 0.5f, 0.5f);
	//右
	pVtx[12].vtx = D3DXVECTOR3(0.5f, 0.5f, -0.5f);
	pVtx[13].vtx = D3DXVECTOR3(0.5f, 0.5f, 0.5f);
	pVtx[14].vtx = D3DXVECTOR3(0.5f, -0.5f, -0.5f);
	pVtx[15].vtx = D3DXVECTOR3(0.5f, 0.5f, 0.5f);
	pVtx[16].vtx = D3DXVECTOR3(0.5f, -0.5f, 0.5f);
	pVtx[17].vtx = D3DXVECTOR3(0.5f, -0.5f, -0.5f);
	//下
	pVtx[18].vtx = D3DXVECTOR3(-0.5f, -0.5f, -0.5f);
	pVtx[19].vtx = D3DXVECTOR3(0.5f, -0.5f, -0.5f);
	pVtx[20].vtx = D3DXVECTOR3(-0.5f, -0.5f, 0.5f);
	pVtx[21].vtx = D3DXVECTOR3(0.5f, -0.5f, -0.5f);
	pVtx[22].vtx = D3DXVECTOR3(0.5f, -0.5f, 0.5f);
	pVtx[23].vtx = D3DXVECTOR3(-0.5f, -0.5f, 0.5f);
	//左
	pVtx[24].vtx = D3DXVECTOR3(-0.5f, 0.5f, 0.5f);
	pVtx[25].vtx = D3DXVECTOR3(-0.5f, 0.5f, -0.5f);
	pVtx[26].vtx = D3DXVECTOR3(-0.5f, -0.5f, 0.5f);
	pVtx[27].vtx = D3DXVECTOR3(-0.5f, 0.5f, -0.5f);
	pVtx[28].vtx = D3DXVECTOR3(-0.5f, -0.5f, -0.5f);
	pVtx[29].vtx = D3DXVECTOR3(-0.5f, -0.5f, 0.5f);
	//裏面
	pVtx[30].vtx = D3DXVECTOR3(0.5f, 0.5f, 0.5f);
	pVtx[31].vtx = D3DXVECTOR3(-0.5f, 0.5f, 0.5f);
	pVtx[32].vtx = D3DXVECTOR3(0.5f, -0.5f, 0.5f);
	pVtx[33].vtx = D3DXVECTOR3(-0.5f, 0.5f, 0.5f);
	pVtx[34].vtx = D3DXVECTOR3(-0.5f, -0.5f, 0.5f);
	pVtx[35].vtx = D3DXVECTOR3(0.5f, -0.5f, 0.5f);
	//-----------------------------------------------------

	//法線ベクトルの設定
	//-----------------------------------------------------
	pVtx[0].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVtx[1].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVtx[2].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVtx[3].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVtx[4].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVtx[5].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);

	pVtx[6].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVtx[7].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVtx[8].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVtx[9].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVtx[10].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVtx[11].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	
	pVtx[12].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVtx[13].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVtx[14].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVtx[15].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVtx[16].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVtx[17].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);

	pVtx[18].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVtx[19].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVtx[20].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVtx[21].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVtx[22].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVtx[23].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);

	pVtx[24].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVtx[25].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVtx[26].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVtx[27].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVtx[28].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVtx[29].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);

	pVtx[30].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVtx[31].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVtx[32].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVtx[33].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVtx[34].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVtx[35].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	//-------------------------------------------------------------

	//反射光の設定
	//--------------------------------------------------------------
	pVtx[0].diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	pVtx[1].diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	pVtx[2].diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	pVtx[3].diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	pVtx[4].diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	pVtx[5].diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

	pVtx[6].diffuse = D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f);
	pVtx[7].diffuse = D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f);
	pVtx[8].diffuse = D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f);
	pVtx[9].diffuse = D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f);
	pVtx[10].diffuse = D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f);
	pVtx[11].diffuse = D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f);
	
	pVtx[12].diffuse = D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f);
	pVtx[13].diffuse = D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f);
	pVtx[14].diffuse = D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f);
	pVtx[15].diffuse = D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f);
	pVtx[16].diffuse = D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f);
	pVtx[17].diffuse = D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f);

	pVtx[18].diffuse = D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f);
	pVtx[19].diffuse = D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f);
	pVtx[20].diffuse = D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f);
	pVtx[21].diffuse = D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f);
	pVtx[22].diffuse = D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f);
	pVtx[23].diffuse = D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f);

	pVtx[24].diffuse = D3DXCOLOR(0.0f, 1.0f, 1.0f, 1.0f);
	pVtx[25].diffuse = D3DXCOLOR(0.0f, 1.0f, 1.0f, 1.0f);
	pVtx[26].diffuse = D3DXCOLOR(0.0f, 1.0f, 1.0f, 1.0f);
	pVtx[27].diffuse = D3DXCOLOR(0.0f, 1.0f, 1.0f, 1.0f);
	pVtx[28].diffuse = D3DXCOLOR(0.0f, 1.0f, 1.0f, 1.0f);
	pVtx[29].diffuse = D3DXCOLOR(0.0f, 1.0f, 1.0f, 1.0f);

	pVtx[30].diffuse = D3DXCOLOR(1.0f, 0.0f, 1.0f, 1.0f);
	pVtx[31].diffuse = D3DXCOLOR(1.0f, 0.0f, 1.0f, 1.0f);
	pVtx[32].diffuse = D3DXCOLOR(1.0f, 0.0f, 1.0f, 1.0f);
	pVtx[33].diffuse = D3DXCOLOR(1.0f, 0.0f, 1.0f, 1.0f);
	pVtx[34].diffuse = D3DXCOLOR(1.0f, 0.0f, 1.0f, 1.0f);
	pVtx[35].diffuse = D3DXCOLOR(1.0f, 0.0f, 1.0f, 1.0f);
	//-------------------------------------------------------------------

	//テクスチャ座標の設定
	//--------------------------------------------------------------------
	pVtx[0].tex = D3DXVECTOR2(0.0f, 0.0f);
	pVtx[1].tex = D3DXVECTOR2(1.0f, 0.0f);
	pVtx[2].tex = D3DXVECTOR2(0.0f, 1.0f);
	pVtx[3].tex = D3DXVECTOR2(1.0f, 0.0f);
	pVtx[4].tex = D3DXVECTOR2(1.0f, 1.0f);
	pVtx[5].tex = D3DXVECTOR2(0.0f, 1.0f);

	pVtx[6].tex = D3DXVECTOR2(0.0f, 0.0f);
	pVtx[7].tex = D3DXVECTOR2(1.0f, 0.0f);
	pVtx[8].tex = D3DXVECTOR2(0.0f, 1.0f);
	pVtx[9].tex = D3DXVECTOR2(1.0f, 0.0f);
	pVtx[10].tex = D3DXVECTOR2(1.0f, 1.0f);
	pVtx[11].tex = D3DXVECTOR2(0.0f, 1.0f);
	
	pVtx[12].tex = D3DXVECTOR2(0.0f, 0.0f);
	pVtx[13].tex = D3DXVECTOR2(1.0f, 0.0f);
	pVtx[14].tex = D3DXVECTOR2(0.0f, 1.0f);
	pVtx[15].tex = D3DXVECTOR2(1.0f, 0.0f);
	pVtx[16].tex = D3DXVECTOR2(1.0f, 1.0f);
	pVtx[17].tex = D3DXVECTOR2(0.0f, 1.0f);

	pVtx[18].tex = D3DXVECTOR2(0.0f, 0.0f);
	pVtx[19].tex = D3DXVECTOR2(1.0f, 0.0f);
	pVtx[20].tex = D3DXVECTOR2(0.0f, 1.0f);
	pVtx[21].tex = D3DXVECTOR2(1.0f, 0.0f);
	pVtx[22].tex = D3DXVECTOR2(1.0f, 1.0f);
	pVtx[23].tex = D3DXVECTOR2(0.0f, 1.0f);

	pVtx[24].tex = D3DXVECTOR2(0.0f, 0.0f);
	pVtx[25].tex = D3DXVECTOR2(1.0f, 0.0f);
	pVtx[26].tex = D3DXVECTOR2(0.0f, 1.0f);
	pVtx[27].tex = D3DXVECTOR2(1.0f, 0.0f);
	pVtx[28].tex = D3DXVECTOR2(1.0f, 1.0f);
	pVtx[29].tex = D3DXVECTOR2(0.0f, 1.0f);

	pVtx[30].tex = D3DXVECTOR2(0.0f, 0.0f);
	pVtx[31].tex = D3DXVECTOR2(1.0f, 0.0f);
	pVtx[32].tex = D3DXVECTOR2(0.0f, 1.0f);
	pVtx[33].tex = D3DXVECTOR2(1.0f, 0.0f);
	pVtx[34].tex = D3DXVECTOR2(1.0f, 1.0f);
	pVtx[35].tex = D3DXVECTOR2(0.0f, 1.0f);
	//------------------------------------------------------

	//グリッド線頂点座標
	//------------------------------------------------------
	Gurid[0].vtx = D3DXVECTOR3(-2.0f, 0.0f, 2.0f);
	Gurid[1].vtx = D3DXVECTOR3(2.0f, 0.0f, 2.0f);
	Gurid[2].vtx = D3DXVECTOR3(-2.0f, 0.0f, 1.0f);
	Gurid[3].vtx = D3DXVECTOR3(2.0f, 0.0f, 1.0f);
	Gurid[4].vtx = D3DXVECTOR3(-2.0f, 0.0f, 0.0f);
	Gurid[5].vtx = D3DXVECTOR3(2.0f, 0.0f, 0.0f);
	Gurid[6].vtx = D3DXVECTOR3(-2.0f, 0.0f, -1.0f);
	Gurid[7].vtx = D3DXVECTOR3(2.0f, 0.0f, -1.0f);
	Gurid[8].vtx = D3DXVECTOR3(-2.0f, 0.0f, -2.0f);
	Gurid[9].vtx = D3DXVECTOR3(2.0f, 0.0f, -2.0f);

	Gurid[10].vtx = D3DXVECTOR3(2.0f, 0.0f, 2.0f);
	Gurid[11].vtx = D3DXVECTOR3(2.0f, 0.0f, -2.0f);
	Gurid[12].vtx = D3DXVECTOR3(1.0f, 0.0f, 2.0f);
	Gurid[13].vtx = D3DXVECTOR3(1.0f, 0.0f, -2.0f);
	Gurid[14].vtx = D3DXVECTOR3(0.0f, 0.0f, 2.0f);
	Gurid[15].vtx = D3DXVECTOR3(0.0f, 0.0f, -2.0f);
	Gurid[16].vtx = D3DXVECTOR3(-1.0f, 0.0f, 2.0f);
	Gurid[17].vtx = D3DXVECTOR3(-1.0f, 0.0f, -2.0f);
	Gurid[18].vtx = D3DXVECTOR3(-2.0f, 0.0f, 2.0f);
	Gurid[19].vtx = D3DXVECTOR3(-2.0f, 0.0f, -2.0f);

	//法線ベクトルの設定
	//-----------------------------------------------------
	Gurid[0].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	Gurid[1].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	Gurid[2].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	Gurid[3].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	Gurid[4].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	Gurid[5].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	Gurid[6].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	Gurid[7].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	Gurid[8].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	Gurid[9].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	Gurid[10].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	Gurid[11].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	Gurid[12].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	Gurid[13].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	Gurid[14].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	Gurid[15].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	Gurid[16].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	Gurid[17].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	Gurid[18].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	Gurid[19].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);

	//反射光の設定
	//--------------------------------------------------------------
	Gurid[0].diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	Gurid[1].diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	Gurid[2].diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	Gurid[3].diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	Gurid[4].diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	Gurid[5].diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	Gurid[6].diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	Gurid[7].diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	Gurid[8].diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	Gurid[9].diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	Gurid[10].diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	Gurid[11].diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	Gurid[12].diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	Gurid[13].diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	Gurid[14].diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	Gurid[15].diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	Gurid[16].diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	Gurid[17].diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	Gurid[18].diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	Gurid[19].diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);


	//頂点データをアンロックする
	g_pD3DVtxBuff->Unlock();

	//ここまで　3Dポリゴン用頂点の準備
	//================================================================

	g_posCamaraEye = D3DXVECTOR3(0, 3.0f, -3.0f);
	g_posCamaraAt = D3DXVECTOR3(0, 0, 0);
	g_vecCamaraUp = D3DXVECTOR3(0.0f, 1.0f, 0.0f);

    return true;
}

//#######################################################################
// ゲームの更新関数
void Update(void)
{
    g_Rotation += 0.01f;	//適当に角度の増分を増やす

	pos_x += posadd_x;
	if (pos_x >= 2.0f) {
		posadd_x *= -1.0f;
	}
	if (pos_x <= -2.0f) {
		posadd_x *= -1.0f;
	}
}

//#######################################################################
// ゲームの描画関数
void Draw(void)
{
    LPDIRECT3DDEVICE9 pDevice = MyDirect3D_GetDevice();
    if( !pDevice ) return;
     
    // 画面のクリア
    pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(10, 40, 20, 255), 1.0f, 0);

    // 描画バッチ命令の開始
    pDevice->BeginScene();





	//==============================================
	//ここを追加　テスト用カメラ情報を準備


	//ビュー行列の作成
	D3DXMatrixLookAtLH(&g_mtxView,
		&g_posCamaraEye,//カメラの位置
		&g_posCamaraAt, //カメラの注視点
		&g_vecCamaraUp);//カメラの上方向

	//ビュー行列の設定
	pDevice->SetTransform(D3DTS_VIEW, &g_mtxView);

	//==============================================
	// ここを追加　テスト用プロジェクション行列の作成
	D3DXMatrixPerspectiveFovLH(&g_mtxProjection,
		VIEW_ANGLE, //ビュー平面の視野角
		VIEW_ASPECT,//ビュー平面のアスペクト比
		VIEW_NEAR_Z,//ビュー平面のNearZ値
		VIEW_FAR_Z);//ビュー平面のFarZ値

	//プロジェクション行列の設定
	pDevice->SetTransform(D3DTS_PROJECTION, &g_mtxProjection);

	//=============================================================
	//ここを追加　ポリゴンのワールド行列の作成
	D3DXMATRIX mtxScl;//スケーリング行列
	D3DXMATRIX mtxRot;//回転行列
	D3DXMATRIX mtxTrs;//平行移動行列



	g_pos = D3DXVECTOR3(0, 0, 0);//位置
	g_rot = D3DXVECTOR3(0, 0, 0);//向き(回転)
	g_scl = D3DXVECTOR3(0.2f, 0.2f, 0.2f);//大きさ(スケール)


	D3DXMatrixIdentity(&g_mtxWorld);//ワールド行列を単位行列に初期化

	//スケール行列を作成＆ワールド行列へ合成
	D3DXMatrixScaling(&mtxScl, g_scl.x, g_scl.y, g_scl.z);
	D3DXMatrixMultiply(&g_mtxWorld, &g_mtxWorld, &mtxScl);//World * Scaling

	//回転行列を作成＆ワールド行列へ合成
	D3DXMatrixRotationYawPitchRoll(&mtxRot, g_rot.x, g_rot.y, g_rot.z);
	D3DXMatrixMultiply(&g_mtxWorld, &g_mtxWorld, &mtxRot);//World * Rotation

	//平行移動行列を作成＆ワ－ルド行列へ合成
	D3DXMatrixTranslation(&mtxTrs, g_pos.x, g_pos.y, g_pos.z);
	D3DXMatrixMultiply(&g_mtxWorld, &g_mtxWorld, &mtxTrs);//World * Translation

	//ワールドマトリックスを設定
	pDevice->SetTransform(D3DTS_WORLD, &g_mtxWorld);

	//描画したいポリゴンの頂点バッファをデータストリーム(データの通り道)にセット
	pDevice->SetStreamSource(0, g_pD3DVtxBuff, 0, sizeof(VERTEX_3D));

	//描画したいポリゴンの頂点フォーマットの設定
	pDevice->SetFVF(FVF_VERTEX_3D);

	//ポリゴンの描画
	pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	pDevice->SetTexture(0, Texture_GetTexture(TEXTURE_INDEX_KIZUNA));
	pDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 12);



	//ポリゴン2
	//==================================================================================================
	g_pos = D3DXVECTOR3(1, 0, 0);//位置
	g_rot = D3DXVECTOR3(0, 0, 0);//向き(回転)
	g_scl = D3DXVECTOR3(0.2f, 0.2f, 0.2f);//大きさ(スケール)


	D3DXMatrixIdentity(&g_mtxWorld);//ワールド行列を単位行列に初期化

									//スケール行列を作成＆ワールド行列へ合成
	D3DXMatrixScaling(&mtxScl, g_scl.x, g_scl.y, g_scl.z);
	D3DXMatrixMultiply(&g_mtxWorld, &g_mtxWorld, &mtxScl);//World * Scaling

														  //平行移動行列を作成＆ワ－ルド行列へ合成
	D3DXMatrixTranslation(&mtxTrs, g_pos.x, g_pos.y, g_pos.z);
	D3DXMatrixMultiply(&g_mtxWorld, &g_mtxWorld, &mtxTrs);//World * Translation

														  //回転行列を作成＆ワールド行列へ合成
	D3DXMatrixRotationYawPitchRoll(&mtxRot, g_rot.x, g_rot.y, g_rot.z);
	D3DXMatrixMultiply(&g_mtxWorld, &g_mtxWorld, &mtxRot);//World * Rotation

														  //ワールドマトリックスを設定
	pDevice->SetTransform(D3DTS_WORLD, &g_mtxWorld);

	//描画したいポリゴンの頂点バッファをデータストリーム(データの通り道)にセット
	pDevice->SetStreamSource(0, g_pD3DVtxBuff, 0, sizeof(VERTEX_3D));

	//描画したいポリゴンの頂点フォーマットの設定
	pDevice->SetFVF(FVF_VERTEX_3D);

	//ポリゴンの描画
	pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	pDevice->SetTexture(0, Texture_GetTexture(TEXTURE_INDEX_KIZUNA));
	pDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 12);



	//ポリゴン3
	//=================================================================================================
	g_pos = D3DXVECTOR3(0, 1, 0);//位置
	g_rot = D3DXVECTOR3(0, 0, 0);//向き(回転)
	g_scl = D3DXVECTOR3(0.2f, 0.2f, 0.2f);//大きさ(スケール)


	D3DXMatrixIdentity(&g_mtxWorld);//ワールド行列を単位行列に初期化

									//スケール行列を作成＆ワールド行列へ合成
	D3DXMatrixScaling(&mtxScl, g_scl.x, g_scl.y, g_scl.z);
	D3DXMatrixMultiply(&g_mtxWorld, &g_mtxWorld, &mtxScl);//World * Scaling

														  //回転行列を作成＆ワールド行列へ合成
	D3DXMatrixRotationYawPitchRoll(&mtxRot, g_rot.x, g_rot.y, g_rot.z);
	D3DXMatrixMultiply(&g_mtxWorld, &g_mtxWorld, &mtxRot);//World * Rotation

														  //平行移動行列を作成＆ワ－ルド行列へ合成
	D3DXMatrixTranslation(&mtxTrs, g_pos.x, g_pos.y, g_pos.z);
	D3DXMatrixMultiply(&g_mtxWorld, &g_mtxWorld, &mtxTrs);//World * Translation

														  //ワールドマトリックスを設定
	pDevice->SetTransform(D3DTS_WORLD, &g_mtxWorld);

	//描画したいポリゴンの頂点バッファをデータストリーム(データの通り道)にセット
	pDevice->SetStreamSource(0, g_pD3DVtxBuff, 0, sizeof(VERTEX_3D));

	//描画したいポリゴンの頂点フォーマットの設定
	pDevice->SetFVF(FVF_VERTEX_3D);


	//ポリゴンの描画
	pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	pDevice->SetTexture(0, Texture_GetTexture(TEXTURE_INDEX_KIZUNA));
	pDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 12);


	//グリッド線
	//==================================================================================================
	g_pos = D3DXVECTOR3(0, 0, 0);
	g_rot = D3DXVECTOR3(0, 0, 0);
	g_scl = D3DXVECTOR3(1.0f, 1.0f, 1.0f);

	D3DXMatrixIdentity(&g_mtxWorld);

	D3DXMatrixScaling(&mtxScl, g_scl.x, g_scl.y, g_scl.z);
	D3DXMatrixMultiply(&g_mtxWorld, &g_mtxWorld, &mtxScl);
	D3DXMatrixRotationYawPitchRoll(&mtxRot, g_rot.y, g_rot.x, g_rot.z);
	D3DXMatrixMultiply(&g_mtxWorld, &g_mtxWorld, &mtxRot);
	D3DXMatrixTranslation(&mtxTrs, g_pos.x, g_pos.y, g_pos.z);
	D3DXMatrixMultiply(&g_mtxWorld, &g_mtxWorld, &mtxTrs);


	pDevice->SetTransform(D3DTS_WORLD, &g_mtxWorld);


	pDevice->DrawPrimitiveUP(D3DPT_LINELIST, 44, Gurid, sizeof(VERTEX_3D));

   
	//ここまでを追加　
	//=============================================================  


    // 描画バッチ命令の終了
    pDevice->EndScene();

    // バックバッファをフリップ（タイミングはD3DPRESENT_PARAMETERSの設定による）
    pDevice->Present(NULL, NULL, NULL, NULL);
}

//#######################################################################
// ゲームの終了処理
void Finalize(void)
{
    // テクスチャの解放
    Texture_Release();

    // Direct3Dラッパーモジュールの終了処理
    MyDirect3D_Finalize();




}
