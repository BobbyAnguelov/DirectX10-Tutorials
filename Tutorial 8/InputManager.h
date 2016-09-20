#ifndef _RAWINPUT
#define _RAWINPUT

extern Camera* pCamera;
extern unsigned char lightingTechnique;

//register keyboard mouse as input devices!
bool RegisterInputDevices( HWND &hWnd )
{
	RAWINPUTDEVICE inputDevices[2];
        
	//adds mouse and allow legacy messages
	inputDevices[0].usUsagePage = 0x01; 
	inputDevices[0].usUsage = 0x02; 
	inputDevices[0].dwFlags = 0;   
	inputDevices[0].hwndTarget = 0;

	//adds keyboard and allow legacy messages
	inputDevices[1].usUsagePage = 0x01; 
	inputDevices[1].usUsage = 0x06; 
	inputDevices[1].dwFlags = 0;   
	inputDevices[1].hwndTarget = 0;

	if ( RegisterRawInputDevices(inputDevices, 2, sizeof(inputDevices[0]) ) == FALSE ) 
	{
		return false;
	}

	return true;
}

void inline HandleRawInput( HWND &hWnd, HRAWINPUT &lParam )
{
	//get raw input data buffer size
	UINT dbSize;
	GetRawInputData( lParam, RID_INPUT, NULL, &dbSize,sizeof(RAWINPUTHEADER) );
    
	//allocate memory for raw input data and get data
	BYTE* buffer = new BYTE[dbSize];    
	GetRawInputData((HRAWINPUT)lParam, RID_INPUT, buffer, &dbSize, sizeof(RAWINPUTHEADER) );
	RAWINPUT* raw = (RAWINPUT*)buffer;
	
	// Handle Keyboard Input
	//---------------------------------------------------------------------------
	if (raw->header.dwType == RIM_TYPEKEYBOARD) 
	{
		switch( raw->data.keyboard.Message )
		{
			//key up
			case WM_KEYUP : 
				switch ( raw->data.keyboard.VKey )
				{
					case 'W' : pCamera->setMovementToggle( 0, 0 );
					break;

					case 'S' : pCamera->setMovementToggle( 1, 0 );
					break;

					case 'A' : pCamera->setMovementToggle( 2, 0 );
					break;

					case 'D' : pCamera->setMovementToggle( 3, 0 );
					break;
				}
			break;

			//key down
			case WM_KEYDOWN : 
				switch ( raw->data.keyboard.VKey )
				{
					case VK_ESCAPE : PostQuitMessage(0);
					break;

					case 'W' : pCamera->setMovementToggle( 0, 1 );
					break;

					case 'S' : pCamera->setMovementToggle( 1, -1 );
					break;

					case 'A' : pCamera->setMovementToggle( 2, -1 );
					break;

					case 'D' : pCamera->setMovementToggle( 3, 1 );
					break;

					case 'L' : {
									if ( ++lightingTechnique == 4 ) lightingTechnique = 0;

									char MSG[255];
									switch (lightingTechnique)
									{
										case 0 : sprintf_s(MSG, "Bobby Anguelov's DirectX 10 Tutorial 8 - Lighting - Per Vertex Phong Lighting (Push L to toggle)"); 	break;
										case 1 : sprintf_s(MSG, "Bobby Anguelov's DirectX 10 Tutorial 8 - Lighting - Per Pixel Phong Lighting (Push L to toggle)"); 	break;
										case 2 : sprintf_s(MSG, "Bobby Anguelov's DirectX 10 Tutorial 8 - Lighting - Per Vertex Blinn-Phong Lighting (Push L to toggle)"); 	break;
										case 3 : sprintf_s(MSG, "Bobby Anguelov's DirectX 10 Tutorial 8 - Lighting - Per Pixel Blinn-Phong Lighting (Push L to toggle)"); 	break;
									};
									SetWindowText(hWnd, MSG);
							   }
					break;
				}
			break;
		}
	}
	
	// Handle Mouse Input
	//---------------------------------------------------------------------------
	else if (raw->header.dwType == RIM_TYPEMOUSE) 
	{
		//mouse camera control
		pCamera->adjustHeadingPitch( 0.0025f * raw->data.mouse.lLastX, 0.0025f * raw->data.mouse.lLastY );				
		
	}

	//free allocated memory
	delete[] buffer;
}

#endif