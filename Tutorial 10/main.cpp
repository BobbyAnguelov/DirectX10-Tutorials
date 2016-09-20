#include <windows.h>
#include <tchar.h>

#include "DX10Renderer.h"
#include "Camera.h"
#include "RawInput.h"
#include <sstream>

/*******************************************************************
* Global Variables
*******************************************************************/
HWND hWnd;						
Camera camera;
DX10Renderer renderer;
unsigned int shadowMapTechnique = 0;
unsigned int currTech = 3;
bool doubleSizedShadowMap = false;
float smBias = 0.0005f;

/*******************************************************************
* Main Window Procedure - handles application events
*******************************************************************/
LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{		
		//any user input messages sent to game engine 
		case WM_INPUT:
		{			
			HRAWINPUT in = *((HRAWINPUT*) &lParam);

			//get raw input data buffer size
			UINT dbSize;
			GetRawInputData(in, RID_INPUT, NULL, &dbSize,sizeof(RAWINPUTHEADER) );
    
			//allocate memory for raw input data and get data
			unsigned char* pData = new unsigned char[dbSize];    
			GetRawInputData(in, RID_INPUT, pData, &dbSize, sizeof(RAWINPUTHEADER) );
			RAWINPUT* pInput = (RAWINPUT*) pData;
		
			//Handle Keyboard Input
			//**************************************************************************
			if (pInput->header.dwType == RIM_TYPEKEYBOARD) 
			{			
				switch( pInput->data.keyboard.Message )
				{
					//key up
					case WM_KEYDOWN:
						{
							switch (pInput->data.keyboard.VKey)
							{							
								case 'W' : 
									camera.HandleInputMessage(0); 
									break;
								case 'S' : 
									camera.HandleInputMessage(1); 
									break;
								case 'A' : 
									camera.HandleInputMessage(2); 
									break;
								case 'D' : 
									camera.HandleInputMessage(3); 
									break;	
								case 'T' : 
									{
										//toggle technique
										if ( ++currTech > 5 ) currTech = 3;
										renderer.SetTechnique( currTech );
									}
									break;
								case 'Y' :
									{
										renderer.ToggleShadowMapSize();
										doubleSizedShadowMap = !doubleSizedShadowMap;
									}
									break;
								case 'U' :
									{
										if ( ++shadowMapTechnique > 1 ) shadowMapTechnique = 0;
										renderer.SetShadowMapTechnique( shadowMapTechnique );
									}
									break;
								case 107 :
									{
										smBias += 0.0005f;
										renderer.SetShadowMapBias(smBias);
									}
									break;
								case 109 :
									{
										smBias -= 0.0005f;
										renderer.SetShadowMapBias(smBias);
									}
									break;
							};
						}
						break;
					case WM_KEYUP : 
						{
							switch (pInput->data.keyboard.VKey)
							{							
								case 'W' : 
									camera.HandleInputMessage(4); 
									break;
								case 'S' : 
									camera.HandleInputMessage(5); 
									break;
								case 'A' : 
									camera.HandleInputMessage(6); 
									break;
								case 'D' : 
									camera.HandleInputMessage(7); 
									break;	
							};			
						}
						break;				
				}

				//set window text
				std::stringstream titleMsg;
	
				titleMsg << "Shadow Mapping :: ";
				
				if (shadowMapTechnique == 0 ) titleMsg << "Front Face SM, ";
				else titleMsg << "Back Face SM, ";

				if ( currTech == 2 ) titleMsg << "Point Sampling, ";
				else if ( currTech == 3 ) titleMsg << "PCF 1-Tap, ";
				else if ( currTech == 4 ) titleMsg << "PCF 16-Tap, ";

				if ( !doubleSizedShadowMap ) titleMsg << "Shadow Map Size = 800x600, ";
				else titleMsg << "Shadow Map Size = 1600*1200, ";

				titleMsg << "Shadow Map Bias = ";
				titleMsg << smBias;

				wchar_t msg[255];
				swprintf_s( msg, 255, L"%S", titleMsg.str().c_str());	
				SetWindowText(hWnd, msg );
			}
	
			// Handle Mouse Input
			//**************************************************************************
			else if (pInput->header.dwType == RIM_TYPEMOUSE) 
			{					
				camera.AdjustOrientation( 0.005f * pInput->data.mouse.lLastX, 0.005f * pInput->data.mouse.lLastY );								
			}

			//delete input message and return
			delete [] pData;
		}	
		break;

		//close button event
		case WM_DESTROY	:
			PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*******************************************************************
* Initialize Main Window
********************************************************************/
bool initWindow(HWND &hWnd, HINSTANCE hInstance, int width, int height)
{
	WNDCLASSEX wcex;

	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)wndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= 0; 
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= TEXT("NMD");
	wcex.hIconSm		= 0;
	RegisterClassEx(&wcex);
	
	//Resize the window
	RECT windowRect = { 0, 0, width, height };
    AdjustWindowRect(&windowRect, WS_CAPTION, TRUE);

	//create the window from the class defined above	
	hWnd = CreateWindow( TEXT("NMD"), 
						 TEXT("Shadow Mapping :: Front Face SM, Point Sampling, Shadow Map Size = 800x600, Shadow Map Bias = 0.0005f"), 
						 WS_OVERLAPPEDWINDOW,
						 CW_USEDEFAULT, 
						 CW_USEDEFAULT, 
						 windowRect.right - windowRect.left, 
						 windowRect.bottom - windowRect.top, 
						 NULL, 
						 NULL, 
						 hInstance, 
						 NULL);
   
	//window handle not created
	if (!hWnd) return false;
		
	//if window creation was successful
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);	
	
	return true;
}

/*******************************************************************
* WinMain
*******************************************************************/
int APIENTRY _tWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow )
{
	//set up the application window
	if ( !initWindow(hWnd, hInstance, 800 , 600)) return -1;

	//initialize raw input
	if ( !InitializeRawInput() ) return -1;

	//create renderer 	
	if (!renderer.Initialize(&hWnd) ) return -1;
		
	//initialize test scene
	if (!renderer.InitializeScene() ) return -1;	
	renderer.SetShadowMapBias( smBias);

	//camera setup	
	float eye[3] = {0, 10, -10}, fp[3] = {9,9,0}, u[3] = {0,1,0};
	camera.PositionCamera(eye, fp, u);
	camera.SetPerspectiveProjectionLH(  0.785f, 4.0f/3, 1.0f, 1000.0f);
	camera.SetMovementSpeed( 10.0f );
	camera.Update();

	//show first frame and welcome message
	renderer.RenderFrame();
	MessageBox(	hWnd, 
				L"T = Toggle Shadow Map Filtering\nY = Toggle Shadow Map Resolution\nU = Toggle Shadow Map Generation Method\n +/- = Adjust shadow map bias",
				L"DirectX10 Tutorial 10 - Shadow Mapping", 
				MB_ICONINFORMATION );

	//frame timer
	HRTimer fTimer;
	fTimer.Reset();

	//main message loop
    MSG msg = {0};
    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) == TRUE)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);			
		}
		else
		{		
			//limit FPS to 60FPS - hack to reduce GPU squealing
			while ( fTimer.GetElapsedTimeMilliseconds() < 16 );
						
			renderer.RenderFrame();
			camera.Update();
			fTimer.Reset();
		}
    }

	//cleanup
	return 0;
}
