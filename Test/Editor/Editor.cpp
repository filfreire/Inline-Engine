#include "Editor.hpp"
#include "QCWorld.hpp"

Editor::Editor()
{
	bWndMaximized = false;

	core = new Core();

	// Create main window for Editor
	WindowDesc d;
	d.clientSize = Sys::GetScreenSize();
	d.style = eWindowStyle::DEFAULT;
	d.userWndProc = std::bind(&Editor::WndProc, this, _1, _2, _3, _4);
	wnd = new Window(d);

	// Create secondary window for GAME inside Editor
	d.clientSize = Vector2u(100, 100); // Size doesn't matter, splitter will modify it's size
	d.style = eWindowStyle::BORDERLESS;
	d.userWndProc = nullptr;
	gameWnd = new Window(d);

	HWND editorHwnd = (HWND)wnd->GetHandle();
	HWND gameHwnd = (HWND)gameWnd->GetHandle();
	EnableWindow(gameHwnd, false);
	SetParent(gameHwnd, editorHwnd);

	// Resize window, non client area removal made it's size wrong
	wnd->SetRect({ 0,0 }, Sys::GetScreenSize());

	//DWORD style = GetWindowLong(b, GWL_STYLE); //get the b style
	//style &= ~(WS_POPUP | WS_CAPTION); //reset the "caption" and "popup" bits
	//style |= WS_CHILD; //set the "child" bit
	//SetWindowLong(b, GWL_STYLE, style); //set the new style of b
	//RECT rc; //temporary rectangle
	////GetClientRect(a, &rc); //the "inside border" rectangle for a
	////MoveWindow(b, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, true); //place b at (x,y,w,h) in a
	//UpdateWindow(a);

	// Init Graphics Engine
	graphicsE = core->InitGraphicsEngine(gameWnd->GetClientWidth(), gameWnd->GetClientHeight(), (HWND)gameWnd->GetHandle());

	// TEMPORARY TODO
	world = new QCWorld(graphicsE);
	world->IWantSunsetBitches();

	// Init Gui Engine
	guiE = core->InitGuiEngine(graphicsE, wnd);

	// Init Gui
	InitGui();

	wnd->SetIcon(L"InlineEngineLogo.ico");
}

Editor::~Editor()
{
	delete world;
	delete core;
	
	wnd->Close();
	delete wnd;
	delete gameWnd;
}

void Editor::InitGui()
{
	// New Layer
	mainLayer = guiE->AddLayer();

	// Layer border
	mainLayer->SetBorder(1, Color(100));

	// Main layout of the editor is a simple list
	GuiList* mainLayout = mainLayer->AddList();
	mainLayout->StretchFillParent(); // Fill the layer
	mainLayout->SetOrientation(eGuiOrientation::VERTICAL);
	mainLayout->SetBgToColor(Color::BLACK);

	// Caption bar
	captionBar = mainLayer->AddGui();
	captionBar->SetBgToColor(Color(45));
	captionBar->SetRect(0, 0, 100, 26);

	// Minimize, Maximize, Close btn
	GuiList* minMaxCloseList = mainLayer->AddList();
	minMaxCloseList->StretchFitToChildren();
	minimizeBtn = mainLayer->AddImage();
	maximizeBtn = mainLayer->AddImage();
	closeBtn = mainLayer->AddImage();

	minimizeBtn->onMouseClicked += [this](CursorEvent& evt) { wnd->MinimizeSize(); };
	maximizeBtn->onMouseClicked += [this](CursorEvent& evt)
	{
		if (bWndMaximized)
			wnd->RestoreSize();
		else
			wnd->MaximizeSize();
	};
	closeBtn->onMouseClicked += [this](CursorEvent& evt) { wnd->Close(); };

	minimizeBtn->SetImages("Resources/minimize.png", "Resources/minimize_h.png");
	maximizeBtn->SetImages("Resources/maximize.png", "Resources/maximize_h.png");
	closeBtn->SetImages("Resources/close.png", "Resources/close_h.png");

	minMaxCloseList->SetOrientation(eGuiOrientation::HORIZONTAL);
	minMaxCloseList->Add(minimizeBtn);
	minMaxCloseList->Add(maximizeBtn);
	minMaxCloseList->Add(closeBtn);
	minMaxCloseList->AlignRight();

	// Editor caption text
	GuiText* inlineEngineText = mainLayer->AddText();
	inlineEngineText->SetFontSize(14);
	inlineEngineText->SetFontStyle(Gdiplus::FontStyle::FontStyleBold);
	inlineEngineText->SetText("Inline Editor");
	inlineEngineText->AlignVerCenter();
	inlineEngineText->StretchHorFillParent();
	inlineEngineText->SetMarginLeft(7);

	captionBar->Add(inlineEngineText);
	captionBar->Add(minMaxCloseList);
	captionBar->StretchHorFillParent();
	captionBar->SetPos(0, 1);
	//captionBar->SetBorder(0, 0, 0, 1, Color(70));

	mainLayout->Add(captionBar);

	// Main menu bar
	GuiMenu* menuBar = mainLayer->AddMenu();
	menuBar->SetBorder(0, 0, 0, 1, Color(70));
	menuBar->SetOrientation(eGuiOrientation::HORIZONTAL);
	menuBar->SetBgColorForAllStates(Color(25));
	menuBar->SetRect(1, captionBar->GetHeight(), 400, 400);
	menuBar->StretchHorFillParent();
	menuBar->StretchVerFitToChildren();
	{
		GuiMenu* fileMenu = menuBar->AddItemMenu("File");
		GuiMenu* buildMenu = menuBar->AddItemMenu("Build");
		GuiMenu* toolsMenu = menuBar->AddItemMenu("Tools");
		GuiMenu* helpMenu = menuBar->AddItemMenu("Help");

		fileMenu->AddItemButton("New Scene");
		fileMenu->AddItemButton("Open Scene");
		Gui* separator0 = fileMenu->AddItemSeparatorHor();
		separator0->SetBgToColor(Color(80));
		separator0->DisableHover();
		fileMenu->AddItemButton("Save Scene");
		fileMenu->AddItemButton("Save Scene as...");
		Gui* separator1 = fileMenu->AddItemSeparatorHor();
		separator1->SetBgToColor(Color(80));
		separator1->DisableHover();
		fileMenu->AddItemButton("New Project...");
		fileMenu->AddItemButton("Open Project...");
		fileMenu->AddItemButton("Save Project");

		buildMenu->AddItemButton("Windows...");
		buildMenu->AddItemButton("Linux...");
		buildMenu->AddItemButton("Mac...");
		buildMenu->AddItemButton("XBox One...");
		buildMenu->AddItemButton("PS4...");
		buildMenu->AddItemButton("Android");
		buildMenu->AddItemButton("IOS");

		GuiMenu* menu0 = toolsMenu->AddItemMenu("TESZT - 0");
		GuiMenu* menu1 = menu0->AddItemMenu("TESZT - 1");
		GuiMenu* menu2 = menu1->AddItemMenu("TESZT - 2");

		GuiMenu* menu00 = toolsMenu->AddItemMenu("TESZT - 00");
		GuiMenu* menu01 = menu00->AddItemMenu("TESZT - 01");
		GuiMenu* menu001 = menu00->AddItemMenu("TESZT - 001");
		GuiMenu* menu02 = menu01->AddItemMenu("TESZT - 02");
		GuiMenu* menu002 = menu001->AddItemMenu("TESZT - 002");
		toolsMenu->AddItemButton("** PUT TOOLS HERE **");

		helpMenu->AddItemButton("About Inline Engine");


		for (Gui* c : { fileMenu, buildMenu, toolsMenu, helpMenu, menu0, menu1, menu00, menu01, menu001, menu02, menu002 })
		{
			c->SetBorder(1, Color(70));
			for (Gui* child : c->GetChildrenRecursive<GuiButton>())
			{
				child->SetBgToColor(Color(25), Color(65));
				child->SetPadding(2, 2, 4, 4);
			}
		}

		for (Gui* c : menuBar->GetChildrenRecursive<GuiButton>())
		{
			c->SetBgToColor(Color(25), Color(65));
			c->StretchFitToChildren();
			c->SetPadding(4, 4, 2, 2);
			c->AlignCenter();
		}
	}
	mainLayout->Add(menuBar);

	GuiSplitter* split0 = mainLayer->AddSplitter(); // split main
	GuiSplitter* split1 = mainLayer->AddSplitter(); // split main left to (top, bottom)
	GuiSplitter* split2 = mainLayer->AddSplitter(); // split main left-top to (left, right)
	split0->Stretch(eGuiStretch::FILL_PARENT_POSITIVE_DIR, eGuiStretch::FILL_PARENT_POSITIVE_DIR);
	split1->StretchFillParent();
	split2->StretchFillParent();

	split0->SetOrientation(eGuiOrientation::HORIZONTAL);
	split1->SetOrientation(eGuiOrientation::VERTICAL);
	split2->SetOrientation(eGuiOrientation::HORIZONTAL);

	split0->SetSize(400, 400);
	split1->SetSize(750, 400);
	split2->SetSize(750, 300);

	Gui* rightArea = mainLayer->AddButton();
	Gui* bottomArea = mainLayer->AddButton();
	Gui* leftArea = mainLayer->AddButton();
	Gui* centerRenderArea = mainLayer->AddButton();
	rightArea->SetSize(150, 100);
	rightArea->SetBgToColor(Color(35));
	bottomArea->SetSize(100, 100);
	bottomArea->SetBgToColor(Color(35));
	leftArea->SetSize(30, 100);
	leftArea->SetBgToColor(Color(35));
	centerRenderArea->SetSize(100, 100);
	centerRenderArea->SetBgToColor(Color(0));
	centerRenderArea->StretchFillParent();

	centerRenderArea->onSizeChangedClonable += [this](Gui* self, Vector2f size)
	{
		HWND gameHwnd = (HWND)gameWnd->GetHandle();
		HWND editorHwnd = (HWND)this->wnd->GetHandle();
		SetWindowPos(gameHwnd, NULL, self->GetContentPosX(), self->GetContentPosY(), self->GetContentSizeX(), self->GetContentSizeY(), 0);
		SetFocus(editorHwnd);

		int width = size.x();
		int height = size.y();

		graphicsE->SetScreenSize(width, height);
		world->ScreenSizeChanged(width, height);
	};

	rightArea->StretchFillParent();
	bottomArea->StretchFillParent();
	leftArea->StretchFillParent();

	// Split main area to left and right
	split0->SetPos(menuBar->GetPosBottomLeft());

	split0->AddItem(split1);
	split0->AddItem(rightArea);

	split1->AddItem(split2);
	split1->AddItem(bottomArea);

	split2->AddItem(leftArea);
	split2->AddItem(centerRenderArea);
	mainLayout->Add(split0);

	for (auto& splitter : { split0, split1, split2 })
	{
		for (auto& separator : splitter->GetSeparators())
		{
			if (splitter->GetOrientation() == eGuiOrientation::HORIZONTAL)
				separator->SetBorder(1, 1, 0, 0, Color::BLACK);
			else
				separator->SetBorder(0, 0, 1, 1, Color::BLACK);
		}
	}

	GuiScrollable* scrollableBottom = bottomArea->AddScrollable();
	scrollableBottom->StretchFillParent();

	GuiList* textureList = scrollableBottom->SetContent(new GuiList(scrollableBottom->guiEngine))->AsList();
	//GuiList* textureList = scrollableBottom->AddList();
	textureList->SetBgToColor(Color(0, 0, 0, 0));
	textureList->SetOrientation(eGuiOrientation::HORIZONTAL);
	textureList->StretchFitToChildren();

	for (int i = 0; i < 50; ++i)
	{
		GuiList* content = textureList->AddItemList();
		content->StretchFitToChildren();
		content->MakeVertical();
		content->SetSize(70, 100);
		content->SetBgToColor(Color(0,0,0,0), Color(65));
		GuiImage* img0 = content->AddItemImage();
		img0->SetMargin(4, 4, 4, 0);
		img0->SetImage("Resources/normal.jpg");
		img0->SetSize(70, 70);

		GuiText* text0 = content->AddText();
		text0->StretchFitToChildren();
		text0->AlignHorCenter();
		text0->SetMargin(4);
		text0->SetText("normal");
		content->DisableChildrenHover();
	}
}

void Editor::Run()
{
	// Create timer, delta time -> engine
	Timer* timer = new Timer();
	timer->Start();

	wnd->SetTitle("Inline Editor");

	// Editor main loop
	while (wnd->IsOpen())
	{
		// Prepare for input processing
		//Input.ClearFrameData();

		WindowEvent evt;
		while (wnd->PopEvent(evt));
		while (gameWnd->PopEvent(evt));

		// Dispatch Inputs
		//Input.Update();

		// Frame delta time
		float deltaTime = timer->Elapsed();

		world->UpdateWorld(deltaTime);
		// Update engine
		core->Update(deltaTime);

		//Sleep(30);
	}

	delete timer;
}

LRESULT Editor::WndProc(HWND handle, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRet;
	bool fCallDWP = !DwmDefWindowProc(handle, msg, wParam, lParam, &lRet);

	switch (msg)
	{
	case WM_SETCURSOR:
		if (LOWORD(lParam) == HTCLIENT)
		{
			if (!guiE->IsUsingCustomCursor())
				SetCursor(LoadCursor(nullptr, IDC_ARROW));

			return TRUE;
		}
		break;
	case WM_ACTIVATE:
	{
		// Extend the frame into the client area.
		MARGINS margins;

		margins.cxLeftWidth = 0;		// 8
		margins.cxRightWidth = 0;		// 8
		margins.cyBottomHeight = 0;		// 20
		margins.cyTopHeight = 0;		// 27

		HRESULT hr = DwmExtendFrameIntoClientArea(handle, &margins);
		assert(hr == S_OK);

		fCallDWP = true;
		lRet = 0;

		break;
	}
	case WM_CREATE:
	{
		RECT rcClient;
		GetWindowRect(handle, &rcClient);

		// Inform the application of the frame change.
		SetWindowPos(handle,
			NULL,
			rcClient.left, rcClient.top,
			rcClient.right - rcClient.left, rcClient.top - rcClient.bottom,
			SWP_FRAMECHANGED);

		fCallDWP = true;
		lRet = 0;

		break;
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		BeginPaint(handle, &ps);

		if (guiE)
			guiE->Render();

		EndPaint(handle, &ps);
		fCallDWP = true;
		lRet = 0;
		break;
	}
	case WM_SIZE:
	{
		if (maximizeBtn)
		{
			if (wParam == SIZE_MAXIMIZED)
			{
				maximizeBtn->SetImages("Resources/restore.png", "Resources/restore_h.png");
				bWndMaximized = true;
			}
			else if (wParam == SIZE_RESTORED)
			{
				maximizeBtn->SetImages("Resources/maximize.png", "Resources/maximize_h.png");
				
				bWndMaximized = false;
			}
		}

		break;
	}
	case WM_DESTROY:
	{
		PostQuitMessage(WM_QUIT);
		break;
	}
	case WM_NCCALCSIZE:
	{
		// Calculate new NCCALCSIZE_PARAMS based on custom NCA inset.
		NCCALCSIZE_PARAMS *pncsp = reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);

		pncsp->rgrc[0].left = pncsp->rgrc[0].left + 0;
		pncsp->rgrc[0].top = pncsp->rgrc[0].top + 0;
		pncsp->rgrc[0].right = pncsp->rgrc[0].right - 0;
		pncsp->rgrc[0].bottom = pncsp->rgrc[0].bottom - 0;

		lRet = 0;

		// No need to pass the message on to the DefWindowProc.
		fCallDWP = false;

		break;
	}
	case WM_NCHITTEST:
	{
		Vector2f cursorPos = guiE->GetCursorPos();

		bool bLeft = cursorPos.x() < 8;
		bool bRight = cursorPos.x() > mainLayer->GetWidth() - 8;
		bool bTop = cursorPos.y() < 8;
		bool bBottom = cursorPos.y() > mainLayer->GetHeight() - 8;

		if (bTop && bLeft)
		{
			return HTTOPLEFT;
		}
		else if (bTop && bRight)
		{
			return HTTOPRIGHT;
		}
		else if (bBottom && bRight)
		{
			return HTBOTTOMRIGHT;
		}
		else if (bBottom && bLeft)
		{
			return HTBOTTOMLEFT;
		}
		else if (bLeft)
		{
			return HTLEFT;
		}
		else if (bRight)
		{
			return HTRIGHT;
		}
		else if (bTop)
		{
			return HTTOP;
		}
		else if (bBottom)
		{
			return HTBOTTOM;
		}
		else if (closeBtn && closeBtn->IsCursorInside())
		{
			// HTNOWHERE
		}
		else if (maximizeBtn && maximizeBtn->IsCursorInside())
		{
			// HTNOWHERE
		}
		else if (minimizeBtn && minimizeBtn->IsCursorInside())
		{
			// HTNOWHERE
		}
		else if (captionBar && captionBar->IsCursorInside())
		{
			return HTCAPTION;
		}
		else
		{
			return HTCLIENT;
		}

		break;
	}
	}

	if (fCallDWP)
		return DefWindowProc(handle, msg, wParam, lParam);

	return 0;
}