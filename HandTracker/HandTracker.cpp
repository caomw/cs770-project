//------------------------------------------------------------------------------
// <copyright file="BodyBasics.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#include "stdafx.h"
#include <strsafe.h>
#include "resource.h"
#include "HandTracker.h"

// CB includes
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <sstream>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#pragma comment (lib, "VMG30_SDK.lib")

static const float c_JointThickness = 3.0f;
static const float c_TrackedBoneThickness = 6.0f;
static const float c_InferredBoneThickness = 1.0f;
static const float c_HandSize = 30.0f;

/// <summary>
/// Entry point for the application
/// </summary>
/// <param name="hInstance">handle to the application instance</param>
/// <param name="hPrevInstance">always 0</param>
/// <param name="lpCmdLine">command line arguments</param>
/// <param name="nCmdShow">whether to display minimized, maximized, or normally</param>
/// <returns>status</returns>
int APIENTRY wWinMain(    
	_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd
)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	AllocConsole();
	freopen("CONOUT$", "w", stdout);

    HandTracker application;
    application.Run(hInstance, nShowCmd);
}

/// <summary>
/// Constructor
/// </summary>
HandTracker::HandTracker() :
    m_hWnd(NULL),
    m_nStartTime(0),
    m_nLastCounter(0),
    m_nFramesSinceUpdate(0),
    m_fFreq(0),
    m_nNextStatusTime(0LL),
	controlHand(RIGHT),
	ipAddr(DEFAULT_IP),
	port(DEFAULT_PORT),
	ConnectSocket(INVALID_SOCKET),
	ptr(NULL),
	ptr_length(0),
	m_handPoseTracker(NULL),
    m_pKinectSensor(NULL),
    m_pCoordinateMapper(NULL),
    m_pBodyFrameReader(NULL),
    m_pD2DFactory(NULL),
    m_pRenderTarget(NULL),
    m_pBrushJointTracked(NULL),
    m_pBrushJointInferred(NULL),
    m_pBrushBoneTracked(NULL),
    m_pBrushBoneInferred(NULL),
    m_pBrushHandClosed(NULL),
    m_pBrushHandOpen(NULL),
    m_pBrushHandLasso(NULL)
{
	m_gloveH = GetVMGlove();
	VMGloveSetConnPar(m_gloveH, 3, "192.168.2.212");
	int comp;
	char ip[VHAND_STRLEN];
	VMGloveGetConnPar(m_gloveH, &comp, ip);

	fprintf(stderr, "COMP:%d IP:%s\n", comp, ip);

	std::cout << "hello" << std::endl;

	m_FrameCounter = 0;

	VMGloveConnect(m_gloveH, CONN_USB, PKG_QUAT_FINGER);

    LARGE_INTEGER qpf = {0};
    if (QueryPerformanceFrequency(&qpf))
    {
        m_fFreq = double(qpf.QuadPart);
    }
}
  

/// <summary>
/// Destructor
/// </summary>
HandTracker::~HandTracker()
{
    DiscardDirect2DResources();

    // clean up Direct2D
    SafeRelease(m_pD2DFactory);

    // done with body frame reader
    SafeRelease(m_pBodyFrameReader);

    // done with coordinate mapper
    SafeRelease(m_pCoordinateMapper);

    // close the Kinect Sensor
    if (m_pKinectSensor)
    {
        m_pKinectSensor->Close();
    }

    SafeRelease(m_pKinectSensor);
}

/// <summary>
/// Creates the main window and begins processing
/// </summary>
/// <param name="hInstance">handle to the application instance</param>
/// <param name="nCmdShow">whether to display minimized, maximized, or normally</param>
int HandTracker::Run(HINSTANCE hInstance, int nCmdShow)
{
    MSG       msg = {0};
    WNDCLASS  wc;

    // Dialog custom window class
    ZeroMemory(&wc, sizeof(wc));
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.cbWndExtra    = DLGWINDOWEXTRA;
    wc.hCursor       = LoadCursorW(NULL, IDC_ARROW);
    wc.hIcon         = LoadIconW(hInstance, MAKEINTRESOURCE(IDI_APP));
    wc.lpfnWndProc   = DefDlgProcW;
    wc.lpszClassName = L"HandTrackerAppDlgWndClass";

    if (!RegisterClassW(&wc))
    {
        return 0;
    }

    // Create main application window
    HWND hWndApp = CreateDialogParamW(
        NULL,
        MAKEINTRESOURCE(IDD_APP),
        NULL,
        (DLGPROC)HandTracker::MessageRouter, 
        reinterpret_cast<LPARAM>(this));

    // Show window
    ShowWindow(hWndApp, nCmdShow);

    // Main message loop
    while (WM_QUIT != msg.message)
    {
        Update();

        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
        {
            // If a dialog message will be taken care of by the dialog proc
            if (hWndApp && IsDialogMessageW(hWndApp, &msg))
            {
                continue;
            }

            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    return static_cast<int>(msg.wParam);
}

/// <summary>
/// Main processing function
/// </summary>
void HandTracker::Update()
{
    if (!m_pBodyFrameReader)
    {
        return;
    }

    IBodyFrame* pBodyFrame = NULL;

    HRESULT hr = m_pBodyFrameReader->AcquireLatestFrame(&pBodyFrame);

    if (SUCCEEDED(hr))
    {
        INT64 nTime = 0;

        hr = pBodyFrame->get_RelativeTime(&nTime);

        IBody* ppBodies[BODY_COUNT] = {0};

        if (SUCCEEDED(hr))
        {
            hr = pBodyFrame->GetAndRefreshBodyData(_countof(ppBodies), ppBodies);
        }

        if (SUCCEEDED(hr))
        {
            ProcessBody(nTime, BODY_COUNT, ppBodies);
        }

        for (int i = 0; i < _countof(ppBodies); ++i)
        {
            SafeRelease(ppBodies[i]);
        }
    }

    SafeRelease(pBodyFrame);
}

/// <summary>
/// Handles window messages, passes most to the class instance to handle
/// </summary>
/// <param name="hWnd">window message is for</param>
/// <param name="uMsg">message</param>
/// <param name="wParam">message data</param>
/// <param name="lParam">additional message data</param>
/// <returns>result of message processing</returns>
LRESULT CALLBACK HandTracker::MessageRouter(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HandTracker* pThis = NULL;
    
    if (WM_INITDIALOG == uMsg)
    {
        pThis = reinterpret_cast<HandTracker*>(lParam);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    }
    else
    {
        pThis = reinterpret_cast<HandTracker*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }

    if (pThis)
    {
        return pThis->DlgProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}


/// <summary>
/// Handle windows messages for the class instance
/// </summary>
/// <param name="hWnd">window message is for</param>
/// <param name="uMsg">message</param>
/// <param name="wParam">message data</param>
/// <param name="lParam">additional message data</param>
/// <returns>result of message processing</returns>
LRESULT CALLBACK HandTracker::NetworkSettings(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HandTracker* pThis = NULL;

	if (WM_INITDIALOG == message)
	{
		pThis = reinterpret_cast<HandTracker*>(lParam);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));

		LPWSTR ipStr = new WCHAR[16];
		LPWSTR portStr = new WCHAR[6];

		mbstowcs(ipStr, pThis->ipAddr, 16);
		mbstowcs(portStr, pThis->port, 6);

		SetDlgItemText(hWnd, IDC_IP, ipStr);
		SetDlgItemText(hWnd, IDC_PORT, portStr);
	}
	else
	{
		pThis = reinterpret_cast<HandTracker*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
	}

	// Menu items
	switch (message)
	{
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDC_SAVE:
		{
			LPWSTR ipStr = new WCHAR[16];
			LPWSTR portStr = new WCHAR[6];

			HWND ctrl;
			ctrl = GetDlgItem(hWnd, IDC_IP);
			if (ctrl)
				GetWindowText(ctrl, ipStr, 16);
			ctrl = GetDlgItem(hWnd, IDC_PORT);
			if (ctrl)
				GetWindowText(ctrl, portStr, 6);

			pThis->ipAddr = new char[16];
			pThis->port = new char[6];
			wcstombs(pThis->ipAddr, ipStr, 16);
			wcstombs(pThis->port, portStr, 6);
		}
		case IDC_CANCEL:
			EndDialog(hWnd, 0);
			break;
		}
	}
	default:
		break;

	}

	return 0;
}

/// <summary>
/// Handle windows messages for the class instance
/// </summary>
/// <param name="hWnd">window message is for</param>
/// <param name="uMsg">message</param>
/// <param name="wParam">message data</param>
/// <param name="lParam">additional message data</param>
/// <returns>result of message processing</returns>
LRESULT CALLBACK HandTracker::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

    switch (message)
    {
        case WM_INITDIALOG:
        {
            // Bind application window handle
            m_hWnd = hWnd;

            // Init Direct2D
            D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);

            // Get and initialize the default Kinect sensor
            InitializeDefaultSensor();
        }
        break;

		// Menu items
		case WM_COMMAND:
		{
			HMENU pMenu = GetMenu(hWnd);
			switch (LOWORD(wParam))
			{
			case ID_CONROLHAND_RIGHT:
				controlHand = RIGHT;
				CheckMenuItem(pMenu, ID_CONROLHAND_RIGHT, MF_CHECKED | MF_BYCOMMAND);
				CheckMenuItem(pMenu, ID_CONROLHAND_LEFT, MF_UNCHECKED | MF_BYCOMMAND);
				break;

			case ID_CONROLHAND_LEFT:
				controlHand = LEFT;
				CheckMenuItem(pMenu, ID_CONROLHAND_LEFT, MF_CHECKED | MF_BYCOMMAND);
				CheckMenuItem(pMenu, ID_CONROLHAND_RIGHT, MF_UNCHECKED | MF_BYCOMMAND);
				break;

			case ID_NETWORKSETTINGS:
				DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_FORMVIEW), m_hWnd, (DLGPROC)HandTracker::NetworkSettings,
					reinterpret_cast<LPARAM>(this));
				break;

			case ID_START:
			{
				// network start function... if success do these
				WSADATA wsaData;
				int iResult;
				ptr = NULL;
				struct addrinfo *result = NULL,
					hints;

				// Initialize Winsock
				iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
				if (iResult != 0) {
					MessageBox(m_hWnd, L"WSAStartup failed!", NULL, MB_OK | MB_ICONERROR);
					return 1;
				}

				ZeroMemory(&hints, sizeof(hints));
				hints.ai_family = AF_INET;
				hints.ai_socktype = SOCK_DGRAM;
				hints.ai_protocol = IPPROTO_UDP;

				iResult = getaddrinfo(ipAddr, port, &hints, &result);
				if (iResult != 0) {
					MessageBox(m_hWnd, L"getaddrinfo failed!", NULL, MB_OK | MB_ICONERROR);
					WSACleanup();
					return 1;
				}

				ConnectSocket = INVALID_SOCKET;
				// Attempt to connect to an address until one succeeds
				for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

					// Create a SOCKET for connecting to server
					ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
						ptr->ai_protocol);
					if (ConnectSocket == INVALID_SOCKET) {
						MessageBox(m_hWnd, L"Error at socket()!", NULL, MB_OK | MB_ICONERROR);
						WSACleanup();
						return 1;
					}

					// Connect to server.
					/*iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
					if (iResult == SOCKET_ERROR) {
						closesocket(ConnectSocket);
						ConnectSocket = INVALID_SOCKET;
						continue;
					}*/
					ptr_length = sizeof(sockaddr_in);
					break;
				}

				// success
				EnableMenuItem(pMenu, ID_START, MF_DISABLED | MF_BYCOMMAND);
				EnableMenuItem(pMenu, ID_CONROLHAND_RIGHT, MF_DISABLED | MF_BYCOMMAND);
				EnableMenuItem(pMenu, ID_CONROLHAND_LEFT, MF_DISABLED | MF_BYCOMMAND);
				EnableMenuItem(pMenu, ID_NETWORKSETTINGS, MF_DISABLED | MF_BYCOMMAND);
				EnableMenuItem(pMenu, ID_END, MF_ENABLED | MF_BYCOMMAND);
				break;
			}
			case ID_END:
			{
				// network end function...
				EnableMenuItem(pMenu, ID_START, MF_ENABLED | MF_BYCOMMAND);
				EnableMenuItem(pMenu, ID_CONROLHAND_RIGHT, MF_ENABLED | MF_BYCOMMAND);
				EnableMenuItem(pMenu, ID_CONROLHAND_LEFT, MF_ENABLED | MF_BYCOMMAND);
				EnableMenuItem(pMenu, ID_NETWORKSETTINGS, MF_ENABLED | MF_BYCOMMAND);
				EnableMenuItem(pMenu, ID_END, MF_DISABLED | MF_BYCOMMAND);

				// shutdown the send half of the connection since no more data will be sent
				int iResult;
				/*int iResult = shutdown(ConnectSocket, SD_SEND);
				if (iResult == SOCKET_ERROR) {
					MessageBox(m_hWnd, L"Socket shutdown failed!", NULL, MB_OK | MB_ICONERROR);
					closesocket(ConnectSocket);
					WSACleanup();
					ConnectSocket = INVALID_SOCKET;
					return 1;
				}*/

				// Receive until the peer closes the connection
				/*char recvbuf[DEFAULT_BUFLEN];
				int recvbuflen = DEFAULT_BUFLEN;
				do {

					iResult = recvfrom(ConnectSocket, recvbuf, recvbuflen, 0, ptr->ai_addr, &ptr_length);
					if (iResult > 0)
						printf("Bytes received: %d\n", iResult);
					else if (iResult == 0)
						printf("Connection closed\n");
					else
						printf("recv failed with error: %d\n", WSAGetLastError());

				} while (iResult > 0);*/

				closesocket(ConnectSocket);
				WSACleanup();

				ConnectSocket = INVALID_SOCKET;
				ptr = NULL;
				ptr_length = 0;
				// success
				break;
			}
			default:
				break;

			}
		}
		break;

        // If the titlebar X is clicked, destroy app
        case WM_CLOSE:
            DestroyWindow(hWnd);
            break;

        case WM_DESTROY:
            // Quit the main message pump
            PostQuitMessage(0);
            break;
    }

    return FALSE;
}

/// <summary>
/// Initializes the default Kinect sensor
/// </summary>
/// <returns>indicates success or failure</returns>
HRESULT HandTracker::InitializeDefaultSensor()
{
    HRESULT hr;

    hr = GetDefaultKinectSensor(&m_pKinectSensor);
    if (FAILED(hr))
    {
        return hr;
    }

    if (m_pKinectSensor)
    {
        // Initialize the Kinect and get coordinate mapper and the body reader
        IBodyFrameSource* pBodyFrameSource = NULL;

        hr = m_pKinectSensor->Open();

        if (SUCCEEDED(hr))
        {
            hr = m_pKinectSensor->get_CoordinateMapper(&m_pCoordinateMapper);
        }

        if (SUCCEEDED(hr))
        {
            hr = m_pKinectSensor->get_BodyFrameSource(&pBodyFrameSource);
        }

        if (SUCCEEDED(hr))
        {
            hr = pBodyFrameSource->OpenReader(&m_pBodyFrameReader);
        }

        SafeRelease(pBodyFrameSource);
    }

    if (!m_pKinectSensor || FAILED(hr))
    {
        SetStatusMessage(L"No ready Kinect found!", 10000, true);
        return E_FAIL;
    }

    return hr;
}

/// <summary>
/// Handle new body data
/// <param name="nTime">timestamp of frame</param>
/// <param name="nBodyCount">body data count</param>
/// <param name="ppBodies">body data in frame</param>
/// </summary>
void HandTracker::ProcessBody(INT64 nTime, int nBodyCount, IBody** ppBodies)
{
    if (m_hWnd)
    {
        HRESULT hr = EnsureDirect2DResources();

        if (SUCCEEDED(hr) && m_pRenderTarget && m_pCoordinateMapper)
        {
            m_pRenderTarget->BeginDraw();
            m_pRenderTarget->Clear();

            RECT rct;
            GetClientRect(GetDlgItem(m_hWnd, IDC_VIDEOVIEW), &rct);
            int width = rct.right;
            int height = rct.bottom;

            for (int i = 0; i < nBodyCount; ++i)
            {
                IBody* pBody = ppBodies[i];
                if (pBody)
                {
                    BOOLEAN bTracked = false;
                    hr = pBody->get_IsTracked(&bTracked);

                    if (SUCCEEDED(hr) && bTracked)
                    {
						JointOrientation joint_orient[JointType_Count];
						Joint joints[JointType_Count]; 
                        D2D1_POINT_2F jointPoints[JointType_Count];
                        HandState leftHandState = HandState_Unknown;
                        HandState rightHandState = HandState_Unknown;

						//get hand state
                        pBody->get_HandLeftState(&leftHandState);
                        pBody->get_HandRightState(&rightHandState);

						HRESULT hrj = pBody->GetJointOrientations(_countof(joint_orient), joint_orient);
                        hr = pBody->GetJoints(_countof(joints), joints);
						if (SUCCEEDED(hr) && SUCCEEDED(hrj))
                        {
							for (int j = 0; j < _countof(joints); ++j)
							{
								jointPoints[j] = BodyToScreen(joints[j].Position, width, height);
								short publishRate = 5;

								if ((((controlHand == RIGHT) && (j == JointType_WristRight)) ||
									((controlHand == LEFT) && (j == JointType_WristLeft))) && 
									(ConnectSocket != INVALID_SOCKET) && (m_FrameCounter % publishRate == 0))
								{

									int cs = VMGloveGetConnectionStatus(m_gloveH);
									fprintf(stdout, "CONNSTATUS: %d\n", cs);
									double quat1, quat2, quat3, quat4;
									VMGloveGetQuaternionHand(m_gloveH, &quat1, &quat2, &quat3, &quat4);
									double quat1W, quat2W, quat3W, quat4W;
									VMGloveGetQuaternionWrist(m_gloveH, &quat1W, &quat2W, &quat3W, &quat4W);


									//finger rotation data
									//100 is max value, corresponds to 0 rotation
									double fingers[10];
									VMGloveGetFingers(m_gloveH, fingers);
									double close_thumbLower_threshold =  95.0;
									double close_indexUpper_threshold =  99.0;
									double close_indexLower_threshold =  95.0;
									double close_middleUpper_threshold = 99.0;
									double close_middleLower_threshold = 95.0;

									double open_thumbLower_threshold =  30.0;
									double open_indexUpper_threshold =  80.0;
									double open_indexLower_threshold =  46.0;
									double open_middleUpper_threshold = 80.0;
									double open_middleLower_threshold = 80.0;

									if (
										fingers[1] < close_thumbLower_threshold &&
										fingers[2] < close_indexUpper_threshold &&
										fingers[3] < close_indexLower_threshold &&
										fingers[4] < close_middleUpper_threshold &&
										fingers[5] < close_middleLower_threshold  
										) {

										leftHandState =  HandState_Closed;
										rightHandState = HandState_Closed;

									}
									else if (
										fingers[1] > open_thumbLower_threshold &&
										fingers[2] > open_indexUpper_threshold &&
										fingers[3] > open_indexLower_threshold &&
										fingers[4] > open_middleUpper_threshold &&
										fingers[5] > open_middleLower_threshold 
										) {
										leftHandState =  HandState_Open;
										rightHandState = HandState_Open;
									}
									else {
										leftHandState =  HandState_NotTracked;
										rightHandState = HandState_NotTracked;
									}


									int handState = -1;
									int otherHandState = -1;
									if (controlHand == RIGHT)
									{
										//handState = (int)rightHandState;
										handState = (int)leftHandState;
										otherHandState = (int)leftHandState;
									}
									else if (controlHand == LEFT)
									{
										//handState = (int)leftHandState;
										handState = (int)rightHandState;
										otherHandState = (int)rightHandState;
									}

									if (m_handPoseTracker == NULL)
									{
										m_handPoseTracker = new HandPose(joints[j].Position.X, joints[j].Position.Y, joints[j].Position.Z,
											quat1, quat2, quat3, quat4, quat1W, quat2W, quat3W, quat4W, handState);
									}
									else
									{
										m_handPoseTracker->update(joints[j].Position.X, joints[j].Position.Y, joints[j].Position.Z,
											quat1, quat2, quat3, quat4, quat1W, quat2W, quat3W, quat4W, handState);
									}

									// CONVERT TO STRING AND SEND VIA SOCKET
									std::stringstream ss;
									ss << *m_handPoseTracker;
									std::string& tmp = ss.str();

									const char *msg = tmp.c_str();
									int iResult = sendto(ConnectSocket, msg, (int)strlen(msg), 0, ptr->ai_addr, ptr_length);
									if (iResult == SOCKET_ERROR) {
										MessageBox(m_hWnd, L"Send data failed!", NULL, MB_OK | MB_ICONERROR);
										closesocket(ConnectSocket);
										WSACleanup();
										ConnectSocket = INVALID_SOCKET;
									}
								}
								else if (ConnectSocket == INVALID_SOCKET)
								{
									free(m_handPoseTracker);
									m_handPoseTracker = NULL;
								}

                            }

                            DrawBody(joints, jointPoints);
							DrawHand(rightHandState, jointPoints[JointType_HandRight]);
							DrawHand(leftHandState, jointPoints[JointType_HandLeft]);
                        }
                    }
                }
            }

            hr = m_pRenderTarget->EndDraw();

            // Device lost, need to recreate the render target
            // We'll dispose it now and retry drawing
            if (D2DERR_RECREATE_TARGET == hr)
            {
                hr = S_OK;
                DiscardDirect2DResources();
            }
        }

        if (!m_nStartTime)
        {
            m_nStartTime = nTime;
        }

        double fps = 0.0;

        LARGE_INTEGER qpcNow = {0};
        if (m_fFreq)
        {
            if (QueryPerformanceCounter(&qpcNow))
            {
                if (m_nLastCounter)
                {
                    m_nFramesSinceUpdate++;
                    fps = m_fFreq * m_nFramesSinceUpdate / double(qpcNow.QuadPart - m_nLastCounter);
                }
            }
        }

        WCHAR szStatusMessage[64];
        StringCchPrintf(szStatusMessage, _countof(szStatusMessage), L" FPS = %0.2f    Time = %I64d", fps, (nTime - m_nStartTime));

        if (SetStatusMessage(szStatusMessage, 1000, false))
        {
            m_nLastCounter = qpcNow.QuadPart;
            m_nFramesSinceUpdate = 0;
        }
    }

	m_FrameCounter++;
}

/// <summary>
/// Set the status bar message
/// </summary>
/// <param name="szMessage">message to display</param>
/// <param name="showTimeMsec">time in milliseconds to ignore future status messages</param>
/// <param name="bForce">force status update</param>
bool HandTracker::SetStatusMessage(_In_z_ WCHAR* szMessage, DWORD nShowTimeMsec, bool bForce)
{
    INT64 now = GetTickCount64();

    if (m_hWnd && (bForce || (m_nNextStatusTime <= now)))
    {
        SetDlgItemText(m_hWnd, IDC_STATUS, szMessage);
        m_nNextStatusTime = now + nShowTimeMsec;

        return true;
    }

    return false;
}

/// <summary>
/// Ensure necessary Direct2d resources are created
/// </summary>
/// <returns>S_OK if successful, otherwise an error code</returns>
HRESULT HandTracker::EnsureDirect2DResources()
{
    HRESULT hr = S_OK;

    if (m_pD2DFactory && !m_pRenderTarget)
    {
        RECT rc;
        GetWindowRect(GetDlgItem(m_hWnd, IDC_VIDEOVIEW), &rc);  

        int width = rc.right - rc.left;
        int height = rc.bottom - rc.top;
        D2D1_SIZE_U size = D2D1::SizeU(width, height);
        D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties();
        rtProps.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);
        rtProps.usage = D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE;

        // Create a Hwnd render target, in order to render to the window set in initialize
        hr = m_pD2DFactory->CreateHwndRenderTarget(
            rtProps,
            D2D1::HwndRenderTargetProperties(GetDlgItem(m_hWnd, IDC_VIDEOVIEW), size),
            &m_pRenderTarget
        );

        if (FAILED(hr))
        {
            SetStatusMessage(L"Couldn't create Direct2D render target!", 10000, true);
            return hr;
        }

        // light green
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.27f, 0.75f, 0.27f), &m_pBrushJointTracked);

        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Yellow, 1.0f), &m_pBrushJointInferred);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green, 1.0f), &m_pBrushBoneTracked);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray, 1.0f), &m_pBrushBoneInferred);

        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red, 0.5f), &m_pBrushHandClosed);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green, 0.5f), &m_pBrushHandOpen);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Blue, 0.5f), &m_pBrushHandLasso);
    }

    return hr;
}

/// <summary>
/// Dispose Direct2d resources 
/// </summary>
void HandTracker::DiscardDirect2DResources()
{
    SafeRelease(m_pRenderTarget);

    SafeRelease(m_pBrushJointTracked);
    SafeRelease(m_pBrushJointInferred);
    SafeRelease(m_pBrushBoneTracked);
    SafeRelease(m_pBrushBoneInferred);

    SafeRelease(m_pBrushHandClosed);
    SafeRelease(m_pBrushHandOpen);
    SafeRelease(m_pBrushHandLasso);
}

/// <summary>
/// Converts a body point to screen space
/// </summary>
/// <param name="bodyPoint">body point to tranform</param>
/// <param name="width">width (in pixels) of output buffer</param>
/// <param name="height">height (in pixels) of output buffer</param>
/// <returns>point in screen-space</returns>
D2D1_POINT_2F HandTracker::BodyToScreen(const CameraSpacePoint& bodyPoint, int width, int height)
{
    // Calculate the body's position on the screen
    DepthSpacePoint depthPoint = {0};
    m_pCoordinateMapper->MapCameraPointToDepthSpace(bodyPoint, &depthPoint);

    float screenPointX = static_cast<float>(depthPoint.X * width) / cDepthWidth;
    float screenPointY = static_cast<float>(depthPoint.Y * height) / cDepthHeight;

    return D2D1::Point2F(screenPointX, screenPointY);
}

/// <summary>
/// Draws a body 
/// </summary>
/// <param name="pJoints">joint data</param>
/// <param name="pJointPoints">joint positions converted to screen space</param>
void HandTracker::DrawBody(const Joint* pJoints, const D2D1_POINT_2F* pJointPoints)
{
    // Draw the bones

    // Torso
    //DrawBone(pJoints, pJointPoints, JointType_Head, JointType_Neck);
    //DrawBone(pJoints, pJointPoints, JointType_Neck, JointType_SpineShoulder);
    //DrawBone(pJoints, pJointPoints, JointType_SpineShoulder, JointType_SpineMid);
    //DrawBone(pJoints, pJointPoints, JointType_SpineMid, JointType_SpineBase);
    //DrawBone(pJoints, pJointPoints, JointType_SpineShoulder, JointType_ShoulderRight);
    //DrawBone(pJoints, pJointPoints, JointType_SpineShoulder, JointType_ShoulderLeft);
    //DrawBone(pJoints, pJointPoints, JointType_SpineBase, JointType_HipRight);
    //DrawBone(pJoints, pJointPoints, JointType_SpineBase, JointType_HipLeft);
       
	if (controlHand == RIGHT)
	{
		// Right Arm 
		DrawBone(pJoints, pJointPoints, JointType_ShoulderRight, JointType_ElbowRight);
		DrawBone(pJoints, pJointPoints, JointType_ElbowRight, JointType_WristRight);
		DrawBone(pJoints, pJointPoints, JointType_WristRight, JointType_HandRight);
		DrawBone(pJoints, pJointPoints, JointType_HandRight, JointType_HandTipRight);
		DrawBone(pJoints, pJointPoints, JointType_WristRight, JointType_ThumbRight);
	}
	else if (controlHand == LEFT)
	{
		// Left Arm
		DrawBone(pJoints, pJointPoints, JointType_ShoulderLeft, JointType_ElbowLeft);
		DrawBone(pJoints, pJointPoints, JointType_ElbowLeft, JointType_WristLeft);
		DrawBone(pJoints, pJointPoints, JointType_WristLeft, JointType_HandLeft);
		DrawBone(pJoints, pJointPoints, JointType_HandLeft, JointType_HandTipLeft);
		DrawBone(pJoints, pJointPoints, JointType_WristLeft, JointType_ThumbLeft);
	}

    // Right Leg
    //DrawBone(pJoints, pJointPoints, JointType_HipRight, JointType_KneeRight);
    //DrawBone(pJoints, pJointPoints, JointType_KneeRight, JointType_AnkleRight);
    //DrawBone(pJoints, pJointPoints, JointType_AnkleRight, JointType_FootRight);

    // Left Leg
    //DrawBone(pJoints, pJointPoints, JointType_HipLeft, JointType_KneeLeft);
    //DrawBone(pJoints, pJointPoints, JointType_KneeLeft, JointType_AnkleLeft);
    //DrawBone(pJoints, pJointPoints, JointType_AnkleLeft, JointType_FootLeft);

    // Draw the joints
	std::vector<_JointType> joint_set_right = { JointType_ShoulderRight, JointType_ElbowRight, JointType_WristRight, JointType_HandRight, JointType_HandTipRight, JointType_ThumbRight};
	std::vector<_JointType> joint_set_left = { JointType_ShoulderLeft, JointType_ElbowLeft, JointType_WristLeft, JointType_HandLeft, JointType_HandTipLeft, JointType_ThumbLeft };
	
	std::vector<_JointType> joint_set;
	if (controlHand == RIGHT)
	{
		joint_set = joint_set_right;
	}
	else if (controlHand == LEFT)
	{
		joint_set = joint_set_left;
	}
	
	for (int i = 0; i < joint_set.size(); ++i)
    {
		D2D1_ELLIPSE ellipse = D2D1::Ellipse(pJointPoints[joint_set[i]], c_JointThickness, c_JointThickness);

		if (pJoints[joint_set[i]].TrackingState == TrackingState_Inferred)
        {
            m_pRenderTarget->FillEllipse(ellipse, m_pBrushJointInferred);
        }
		else if (pJoints[joint_set[i]].TrackingState == TrackingState_Tracked)
        {
            m_pRenderTarget->FillEllipse(ellipse, m_pBrushJointTracked);
        }
    }
}

/// <summary>
/// Draws one bone of a body (joint to joint)
/// </summary>
/// <param name="pJoints">joint data</param>
/// <param name="pJointPoints">joint positions converted to screen space</param>
/// <param name="pJointPoints">joint positions converted to screen space</param>
/// <param name="joint0">one joint of the bone to draw</param>
/// <param name="joint1">other joint of the bone to draw</param>
void HandTracker::DrawBone(const Joint* pJoints, const D2D1_POINT_2F* pJointPoints, JointType joint0, JointType joint1)
{
    TrackingState joint0State = pJoints[joint0].TrackingState;
    TrackingState joint1State = pJoints[joint1].TrackingState;

    // If we can't find either of these joints, exit
    if ((joint0State == TrackingState_NotTracked) || (joint1State == TrackingState_NotTracked))
    {
        return;
    }

    // Don't draw if both points are inferred
    if ((joint0State == TrackingState_Inferred) && (joint1State == TrackingState_Inferred))
    {
        return;
    }

    // We assume all drawn bones are inferred unless BOTH joints are tracked
    if ((joint0State == TrackingState_Tracked) && (joint1State == TrackingState_Tracked))
    {
        m_pRenderTarget->DrawLine(pJointPoints[joint0], pJointPoints[joint1], m_pBrushBoneTracked, c_TrackedBoneThickness);
    }
    else
    {
        m_pRenderTarget->DrawLine(pJointPoints[joint0], pJointPoints[joint1], m_pBrushBoneInferred, c_InferredBoneThickness);
    }
}

/// <summary>
/// Draws a hand symbol if the hand is tracked: red circle = closed, green circle = opened; blue circle = lasso
/// </summary>
/// <param name="handState">state of the hand</param>
/// <param name="handPosition">position of the hand</param>
void HandTracker::DrawHand(HandState handState, const D2D1_POINT_2F& handPosition)
{
    D2D1_ELLIPSE ellipse = D2D1::Ellipse(handPosition, c_HandSize, c_HandSize);

    switch (handState)
    {
        case HandState_Closed:
            m_pRenderTarget->FillEllipse(ellipse, m_pBrushHandClosed);
            break;

        case HandState_Open:
            m_pRenderTarget->FillEllipse(ellipse, m_pBrushHandOpen);
            break;

        case HandState_Lasso:
            m_pRenderTarget->FillEllipse(ellipse, m_pBrushHandLasso);
            break;
    }
}
