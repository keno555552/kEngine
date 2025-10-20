#include "DircetXBase.h"
#include "ConvertString.h"


#pragma region wordChange
void Log(const std::string& message) {
	OutputDebugStringA(message.c_str());
}

void Log(const std::wstring& message) {
	OutputDebugStringW(message.c_str());
}

std::wstring utf8_to_utf16(const std::string& utf8) {
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), &wstrTo[0], size_needed);
	return wstrTo;

}

#pragma endregion


#pragma region Input
LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}
	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

bool CheakXInputDeviceConnected() {
	XINPUT_STATE state;
	ZeroMemory(&state, sizeof(XINPUT_STATE));

	// XInputのコントローラーをチェック
	for (DWORD i = 0; i < XUSER_MAX_COUNT; i++) {
		if (XInputGetState(i, &state) == ERROR_SUCCESS) {
			bool boool = true;
			config::SetGamePadType(XINPUT);
			config::SetConnetGamePad(&boool);
			config::SetXInputIp(i);
			return true; // 見つけたtrue
		}
	}
	config::SetXInputIp(-1);
	bool boool = false;
	return false; // 見つからないならfalse
}

#pragma endregion

#pragma region WindowMake

IDxcBlob* CompileShader(
	// CompilerするShaderファイルへのパス
	const std::wstring& filePath,
	// Compilerに使用するProfile
	const wchar_t* profile,
	// 初期化で生成したものを3つ
	IDxcUtils* dxcUtils,
	IDxcCompiler3* dxcCompiler,
	IDxcIncludeHandler* includeHandler
) {
	/// ここの中身をこの後書いていく
	/// 1.hlslファイルを読む
	// これからシェーダーをコンパイルする旨をログに出す
	Log(ConvertString(std::format(L"Begin CompileShader, path:{}, profile:{}\n", filePath, profile)));
	// hlslファイルを読む
	IDxcBlobEncoding* shaderSource = nullptr;
	HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	// 読めなかったら止める
	assert(SUCCEEDED(hr));
	// 読み込んだファイルの内容を設定する
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8; // UTF8の文字コードであることを通知

	/// 2.Compileする
	LPCWSTR arguments[] = {
		filePath.c_str(),			// コンパイル対象のhlslファイル名
		L"-E", L"main",				// エントリーポイントの指定。基本的にmain以外にはしない
		L"-T", profile,				// ShaderProfileの設定
		L"-Zi", L"-Qembed_debug",	// デバッグ用の情報を埋め込む
		L"-Od",						// 最適化を外しておく
		L"-Zpr"						// メモリレイアウトは行優先
	};

	// 実際にShaderをコンパイルする
	IDxcResult* shaderResult = nullptr;
	hr = dxcCompiler->Compile(
		&shaderSourceBuffer, // 読み込んだファイル
		arguments, // コンパイルオプション
		_countof(arguments), // コンパイルオプションの数
		includeHandler, // includeが含まれた諸々
		IID_PPV_ARGS(&shaderResult) // コンパイル結果
	);

	// コンパイルエラーではなくdxcが起動できないなど致命的な状況
	assert(SUCCEEDED(hr));

	/// 3.警告・エラーがでていないか確認する
	// 警告・エラーが出てたらログに出して止める
	IDxcBlobUtf8* shaderError = nullptr;
	shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		Log(shaderError->GetStringPointer());
		std::string errorMessage(shaderError->GetStringPointer(), shaderError->GetStringLength());
		OutputDebugStringA(errorMessage.c_str());
		Log(errorMessage);
		// 警告・エラーダメゼッタイ
		assert(false);
	}

	/// 4.Compile結果を受け取って返す
	// コンパイル結果から実行用のバイナリ部分を取得
	IDxcBlob* shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));
	// 成功したログを出す
	Log(ConvertString(std::format(L"Compile Succeeded, path:{}, profile:{}\n", filePath, profile)));
	// もう使わないリソースを解放
	shaderSource->Release();
	shaderResult->Release();
	// 実行用のバイナリを返却
	return shaderBlob;
}

IDxcBlob* CompileShader(
	// CompilerするShaderファイルへのパス
	const std::wstring& filePath,
	// Compilerに使用するProfile
	const wchar_t* profile,
	// 初期化で生成したものを3つ
	IDxcUtils* dxcUtils,
	IDxcCompiler3* dxcCompiler,
	IDxcIncludeHandler* includeHandler,
	LightModelType modelType
) {
	/// ここの中身をこの後書いていく
	/// 1.hlslファイルを読む
	// これからシェーダーをコンパイルする旨をログに出す
	Log(ConvertString(std::format(L"Begin CompileShader, path:{}, profile:{}\n", filePath, profile)));
	// hlslファイルを読む
	IDxcBlobEncoding* shaderSource = nullptr;
	HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	// 読めなかったら止める
	assert(SUCCEEDED(hr));
	// 読み込んだファイルの内容を設定する
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8; // UTF8の文字コードであることを通知

	/// 2.Compileする
	std::vector<LPCWSTR> arguments = {
		filePath.c_str(),			// コンパイル対象のhlslファイル名
		L"-E", L"main",				// エントリーポイントの指定。基本的にmain以外にはしない
		L"-T", profile,				// ShaderProfileの設定
		L"-Zi", L"-Qembed_debug",	// デバッグ用の情報を埋め込む
		L"-Od",						// 最適化を外しておく
		L"-Zpr"						// メモリレイアウトは行優先
	};
	// 光照モデル設定
	if (modelType == LightModelType::Lambert) {
		arguments.push_back(L"-D");
		arguments.push_back(L"LIGHT_MODEL_LAMBERT=1");
	}
	if (modelType == LightModelType::HalfLambert) {
		arguments.push_back(L"-D");
		arguments.push_back(L"LIGHT_MODEL_HALF=1");
	}


	// 実際にShaderをコンパイルする
	IDxcResult* shaderResult = nullptr;
	hr = dxcCompiler->Compile(
		&shaderSourceBuffer, // 読み込んだファイル
		arguments.data(), // コンパイルオプション
		static_cast<uint32_t>(arguments.size()), // コンパイルオプションの数
		includeHandler, // includeが含まれた諸々
		IID_PPV_ARGS(&shaderResult) // コンパイル結果
	);
	// コンパイルエラーではなくdxcが起動できないなど致命的な状況
	assert(SUCCEEDED(hr));

	/// 3.警告・エラーがでていないか確認する
	// 警告・エラーが出てたらログに出して止める
	IDxcBlobUtf8* shaderError = nullptr;
	shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		Log(shaderError->GetStringPointer());
		// 警告・エラーダメゼッタイ
		assert(false);
	}

	/// 4.Compile結果を受け取って返す
	// コンパイル結果から実行用のバイナリ部分を取得
	IDxcBlob* shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));
	// 成功したログを出す
	Log(ConvertString(std::format(L"Compile Succeeded, path:{}, profile:{}\n", filePath, profile)));
	// もう使わないリソースを解放
	shaderSource->Release();
	shaderResult->Release();
	// 実行用のバイナリを返却
	return shaderBlob;
}
#pragma endregion

#pragma region DirectXCommon

DirectXBase::DirectXBase() {
	wc = {};
	debugController = {};
	HRESULT hr = {};
	dxgiFactory = {};
	useAdapter = {};
	device = {};
	commandQueue = {};
	commandAllocator = {};
	commandList = {};
	SwapChain = {};
	swapChainResources[0] = {};
	swapChainResources[1] = {};
	//DescriptorHeap = {};
	rtvDescriptorHeap = {};
	srvDescriptorHeap = {};
	dsvDescriptorHeap = {};
	rtvHandles[0] = {};
	rtvHandles[1] = {};
	fence = {};
	fenceValue = {};
	fenceEvent = {};
	hwnd = {};

	directInput = {};
	keyBoardDevice = {};
	mouseDevice = {};
	gamepadDevice = {};
}

DirectXBase::~DirectXBase() {
	//if (dxgiFactory)			dxgiFactory->Release();
	//if (useAdapter)				useAdapter->Release();
	//if (device)					device->Release();
	//if (commandQueue)			commandQueue->Release();
	//if (commandAllocator)		commandAllocator->Release();
	//if (commandList)			commandList->Release();
	//if (SwapChain)				SwapChain->Release();
	//if (swapChainResources[0])	swapChainResources[0]->Release();
	//if (swapChainResources[1])	swapChainResources[1]->Release();
	//if (DescriptorHeap)			DescriptorHeap->Release();
	//if (fence)					fence->Release();
	//CloseHandle(fenceEvent);
}

HWND DirectXBase::WindowMake(const char* kClientTitle, int kClientWidth, int kClientHeight) {
	///windowを登録する
	RECT wrc{ 0,0,kClientWidth ,kClientHeight };
	wc = {};
	wc.lpfnWndProc = WindowProc;
	wc.lpszClassName = L"CG2WindowClass";
	wc.hInstance = GetModuleHandle(nullptr);
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	///規範に合ってない
	//size_t size_needed = mbstowcs(NULL, kClientTitle, 0) + 1;
	//wchar_t* wtitle = new wchar_t[size_needed];
	//mbstowcs(wtitle, kClientTitle, size_needed);

	///外のtitle const char*を const wchar_t*に転換する
	///もう古いって
	/////wstringはstringのwchat_t版
	//std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	//std::wstring wtitle = converter.from_bytes(kClientTitle);
	//const wchar_t* wctitle = wtitle.c_str();

	std::wstring wtitle = utf8_to_utf16(kClientTitle);
	const wchar_t* wctitle = wtitle.c_str();

	RegisterClass(&wc);

	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, FALSE);

	HWND hwnd = CreateWindow(
		wc.lpszClassName,
		wctitle,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left, // ウィンドウ横幅
		wrc.bottom - wrc.top, // ウィンドウ縦幅
		nullptr,
		nullptr,
		wc.hInstance,
		nullptr
	);

	ShowWindow(hwnd, SW_SHOW);

	return hwnd;
}


uint32_t DirectXBase::GetDesriptorSizeSRV() {
	return device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

uint32_t DirectXBase::GetDesriptorSizeRTV() {
	return device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
}

uint32_t DirectXBase::GetDesriptorSizeDSV() {
	return device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}


D3D12_CPU_DESCRIPTOR_HANDLE DirectXBase::GetCPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index) {
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE DirectXBase::GetGPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index) {
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;
}

bool DirectXBase::isDirectInputCreated() {
	if (directInput != nullptr) { return true; }
	return false;
}


void DirectXBase::SetDirectXInputKeyBoard() {
	SetDirectXInput(Keyboard, keyBoardDevice);
	bool boool = true;
	config::SetConnetKeyboard(&boool);
}

void DirectXBase::ReleaseDirectXInputKeyBoard() {
	if (keyBoardDevice) {
		keyBoardDevice->Unacquire();
		keyBoardDevice->Release();
		keyBoardDevice = nullptr;
		bool boool = false;
		config::SetConnetKeyboard(&boool);
	}
}

void DirectXBase::SetDirectXInputMouse() {
	SetDirectXInput(Mouse, mouseDevice);
	bool boool = true;
	config::SetConnetMouse(&boool);
}

void DirectXBase::ReleaseDirectXInputMouse() {
	if (mouseDevice) {
		mouseDevice->Unacquire();
		mouseDevice->Release();
		mouseDevice = nullptr;
		bool boool = false;
		config::SetConnetMouse(&boool);
	}
}

void DirectXBase::SetDirectXInputGamepad() {
	SetDirectXInput(GamePad, gamepadDevice);
	bool boool = true;
	config::SetConnetGamePad(&boool);
}

void DirectXBase::ReleaseDirectXInputGamepad() {
	if (gamepadDevice) {
		gamepadDevice->Unacquire();
		gamepadDevice->Release();
		gamepadDevice = nullptr;
		bool boool = false;
		config::SetConnetGamePad(&boool);
	}
}




ID3D12Device* DirectXBase::CreateDevice(IDXGIAdapter4* adapter) {
	ID3D12Device* device = nullptr;
	// 機能レベルとログ出力用の文字列
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0
	};
	const char* featureLevelStrings[] = { "12.2", "12.1", "12.0" };
	// 高い順に生成できるか試していく
	for (size_t i = 0; i < _countof(featureLevels); ++i) {
		// 採用したアダプターでデバイスを生成
		HRESULT hr = D3D12CreateDevice(adapter, featureLevels[i], IID_PPV_ARGS(&device));
		// 指定した機能レベルでデバイスが生成できたかを確認
		if (SUCCEEDED(hr)) {
			// 生成できたのでログ出力を行ってループを抜ける
			Log(std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
			break;
		}
	}
	// デバイスの生成がうまくいかなかったので起動できない
	assert(device != nullptr);
	Log("Complete create D3D12Device!!!\n"); // 初期化完了のログをだす

#ifdef _DEBUG
	ID3D12InfoQueue* infoQueue = nullptr;
	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
		// ヤバイエラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		// エラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		// 警告時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
		// 解除
		infoQueue->Release();

		// 抑制するメッセージのID
		D3D12_MESSAGE_ID denyIds[] = {
			// Windows11でのDXGIデバッグレイヤーとD3D12デバッグレイヤーの相互作用バグによるエラーメッセージ
			// https://stackoverflow.com/questions/6980245/directx-12-application-is-crashing-in-windows-11
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};

		// 抑制するレベル
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		// 指定したメッセージの表示を抑制する
		infoQueue->PushStorageFilter(&filter);

	}
#endif
	return device;
}

void DirectXBase::InitializeGameInput() {
}

//void DirectXBase::InitializeGameInput() {
//	HRESULT GetCurrentReading(
//		GameInputKind,
//		);
//	//assert(SUCCEEDED(result));
//}


void DirectXBase::InitializeDirectXInput() {
	HRESULT result = DirectInput8Create(
		wc.hInstance,
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void**)&directInput,
		nullptr);
	assert(SUCCEEDED(result));
}

void DirectXBase::SetDirectXInput(InputType type, IDirectInputDevice8*& drive) {
	HRESULT result;
	bool boool = false;
	bool skip = false;
	switch (type) {
	case Keyboard:
		/// キーボードデバイスの生成
		result = directInput->CreateDevice(GUID_SysKeyboard, &drive, NULL);
		if (SUCCEEDED(result)) {
			boool = true;
			config::SetConnetKeyboard(&boool);
		} else {
			config::SetConnetKeyboard(&boool);
			skip = true;
			break;
		}

		/// 入力データ形式のセット
		result = drive->SetDataFormat(&c_dfDIKeyboard);
		assert(SUCCEEDED(result));

		break;
	case Mouse:
		/// マウスデバイスの生成
		result = directInput->CreateDevice(GUID_SysMouse, &drive, NULL);
		if (SUCCEEDED(result)) {
			boool = true;
			config::SetConnetMouse(&boool);
		} else {
			config::SetConnetMouse(&boool);
			skip = true;
			break;
		}

		/// 入力データ形式のセット
		result = drive->SetDataFormat(&c_dfDIMouse);
		assert(SUCCEEDED(result));

		break;
	case GamePad:
		/// マウスデバイスの生成
		result = directInput->CreateDevice(GUID_Joystick, &drive, NULL);
		if (SUCCEEDED(result)) {
			boool = true;
			config::SetConnetGamePad(&boool);
			config::SetGamePadType(DIRECTINPUT);
		} else {
			config::SetConnetGamePad(&boool);
			config::SetGamePadType(NODEVICE);
			skip = true;
			break;
		}

		/// 入力データ形式のセット
		result = drive->SetDataFormat(&c_dfDIJoystick);
		assert(SUCCEEDED(result));
	}

	if (!skip) {
		/// 排他制御レベルのセット
		result = drive->SetCooperativeLevel(
			hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
		assert(SUCCEEDED(result));
	}
}

void DirectXBase::SetXInput() {
	CheakXInputDeviceConnected();
}


ID3D12CommandQueue* DirectXBase::CreateCommandQueue(ID3D12Device* device) {
	// コマンドキューを生成する
	ID3D12CommandQueue* commandQueue = nullptr;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	HRESULT hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	// コマンドキューの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));
	return commandQueue;
}

ID3D12CommandAllocator* DirectXBase::CreateCommandAllocator(ID3D12Device* device) {
	// コマンドアロケータを生成する
	ID3D12CommandAllocator* commandAllocator = nullptr;
	HRESULT hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	// コマンドアロケータの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

	return commandAllocator;
}

ID3D12GraphicsCommandList* DirectXBase::CreateCommandList(ID3D12Device* device, ID3D12CommandAllocator* commandAllocator) {
	// コマンドリストを生成する
	ID3D12GraphicsCommandList* commandList = nullptr;
	HRESULT hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr, IID_PPV_ARGS(&commandList));
	// コマンドリストの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

	return commandList;
}

IDXGISwapChain4* DirectXBase::CreateSwapChain(IDXGIFactory7* dxgiFactory, ID3D12CommandQueue* commandQueue, HWND hwnd, UINT kClientWidth, UINT kClientHeight) {
	// スワップチェーンを生成する
	IDXGISwapChain4* swapChain = nullptr;
	swapChainDesc = {};
	swapChainDesc.Width = kClientWidth;    // 画面の幅。ウィンドウのクライアント領域を同じものにしておく
	swapChainDesc.Height = kClientHeight;  // 画面の高さ。ウィンドウのクライアント領域を同じものにしておく
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  // 色の形式
	swapChainDesc.SampleDesc.Count = 1;    // マルチサンプルしない
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;  // 描画のターゲットとして利用する
	swapChainDesc.BufferCount = 2;         // ダブルバッファ
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;  // モニタにうつしたら、中身を破棄
	// コマンドキュー、ウィンドウのハンドル、設定を渡して生成する
	HRESULT hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue, hwnd, &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(&swapChain));
	assert(SUCCEEDED(hr));

	return swapChain;
}

ID3D12DescriptorHeap* DirectXBase::CreateDescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible) {
	ID3D12DescriptorHeap* descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	if (shaderVisible == true) { descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; } else { descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; }
	//descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	assert(SUCCEEDED(hr));
	return descriptorHeap;
}

void DirectXBase::CreateRenderTargetViews(ID3D12Device* device, IDXGISwapChain4* swapChain, ID3D12DescriptorHeap* rtvDescriptorHeap) {
	// SwapChainからResourceを引っ張ってくる
	HRESULT hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
	// うまく取得できなければ起動できない
	assert(SUCCEEDED(hr));
	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
	assert(SUCCEEDED(hr));
	////////////////////////////////////////////////////////////////////////////////////////////////////
	// RTVの設定
	rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // 出力結果をSRGBに変換して書き込む
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D; // 2dテクスチャとして書き込む

	// ディスクリプタの先頭を取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	// RTVを2つ作るのでディスクリプタを2つ用意<<<<<<<<Classで使わない
	//D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];

	// まず1つ目を作る。2つ目は最初のとこから作る。作る場所をこちらで指定してあげる必要がある
	rtvHandles[0] = rtvStartHandle;
	device->CreateRenderTargetView(swapChainResources[0], &rtvDesc, rtvHandles[0]);

	// 2つ目のディスクリプタハンドルを得る（自力で）
	rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// 2つ目を作る
	device->CreateRenderTargetView(swapChainResources[1], &rtvDesc, rtvHandles[1]);
}

ID3D12Fence* DirectXBase::CreateFence(ID3D12Device* device) {
	///初期値0でFenceを作る
	//ID3D12Fence* fence = nullptr;
	//uint64_t fenceValue = 0;
	HRESULT hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(hr));

	///FenceのSignalを待つためのイベントを作成する
	fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent != nullptr);

	return fence;
}

void DirectXBase::InitializeDrive(const char* kClientTitle, int kClientWidth, int kClientHeight) {
#ifdef _DEBUG
	debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		// デバッグレイヤーを有効化する
		debugController->EnableDebugLayer();
		// さらにGPU側でもチェックを行うようにする
		debugController->SetEnableGPUBasedValidation(TRUE);
	}
#endif

	hwnd = WindowMake(kClientTitle, kClientWidth, kClientHeight);
	config::SaveClientWidth(kClientWidth);
	config::SaveClientHeight(kClientHeight);

	/// DXGIファクトリーの生成
	dxgiFactory = nullptr;
	// HRESULTはWindows系のエラーコードであり、
	// 関数が成功したかどうかをSUCCEEDEDマクロで判定できる
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	// 初期化の根本的な部分でエラーが出た場合はプログラムが間違っているか、どうにもできない場合が多いのでassertにしておく
	assert(SUCCEEDED(hr));


	// 使用するアダプタ用の変数。最初にnullptrを入れておく
	//IDXGIAdapter4* useAdapter = nullptr;
	// 良い順にアダプタを頼む
	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i,
		DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) !=
		DXGI_ERROR_NOT_FOUND; ++i) {
		// アダプターの情報を取得する
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr)); // 取得できないのは一大事
		// ソフトウェアアダプタでなければ採用！
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			// 採用したアダプタの情報をログに出力。wstringの方なので注意
			//std::wstring description(adapterDesc.Description);
			Log(std::format(L"Use Adapter:{}\n", adapterDesc.Description));
			break;
		}
		useAdapter = nullptr; // ソフトウェアアダプタの場合は見なかったことにする
	}
	// 適切なアダプタが見つからなかったので起動できない
	assert(useAdapter != nullptr);

	device = CreateDevice(useAdapter);
	commandQueue = CreateCommandQueue(device);
	commandAllocator = CreateCommandAllocator(device);
	commandList = CreateCommandList(device, commandAllocator);

	SwapChain = CreateSwapChain(dxgiFactory, commandQueue, hwnd, kClientWidth, kClientHeight);
	//DescriptorHeap = CreateDescriptorHeap(device);
	rtvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
	srvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);
	dsvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

	CreateRenderTargetViews(device, SwapChain, rtvDescriptorHeap);

	fence = CreateFence(device);


	///Imgui
	// ImGuiの初期化。詳細はさして重要ではないので解説は省略する。
	// こういうもんである
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX12_Init(device,
		swapChainDesc.BufferCount,
		rtvDesc.Format,
		srvDescriptorHeap,
		srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

	// DirectX入力装置の初期化
	InitializeDirectXInput();
	SetDirectXInput(Keyboard, keyBoardDevice);
	SetDirectXInput(Mouse, mouseDevice);
	// SetDirectXInput(GamePad, gamepadDevice); //XInput優先
}

void DirectXBase::Finalize() {
	/// ImGuiの終了処理。詳細はさして重要ではないので解説は省略する。
	// こういうもんである。初期化と逆順に行う
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	/// すべでのものの解放
	if (dxgiFactory)			dxgiFactory->Release();
	if (useAdapter)				useAdapter->Release();
	if (device)					device->Release();
	if (commandQueue)			commandQueue->Release();
	if (commandAllocator)		commandAllocator->Release();
	if (commandList)			commandList->Release();
	if (SwapChain)				SwapChain->Release();
	if (swapChainResources[0])	swapChainResources[0]->Release();
	if (swapChainResources[1])	swapChainResources[1]->Release();
	if (rtvDescriptorHeap)		rtvDescriptorHeap->Release();
	if (srvDescriptorHeap)		srvDescriptorHeap->Release();
	if (dsvDescriptorHeap)		dsvDescriptorHeap->Release();
	if (fence)					fence->Release();
	if (directInput)			directInput->Release();
	if (keyBoardDevice) { keyBoardDevice->Unacquire();  keyBoardDevice->Release(); }
	if (mouseDevice) { mouseDevice->Unacquire();  mouseDevice->Release(); }
	if (gamepadDevice) { gamepadDevice->Unacquire();  gamepadDevice->Release(); }
	CloseHandle(fenceEvent);
#ifdef _DEBUG
	/// リソースリークチェック
	IDXGIDebug1* debug;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
		//debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);///また未解放要素がある
		debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
		debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);

		debug->Release();
	}
	/// デバッグレイヤーの解放
	if (debugController)		debugController->Release();
#endif
	CloseWindow(hwnd);
}
#pragma endregion

