#pragma once
#include <Windows.h>
////////////////////////////////////////////////////////////////////////////////
// 
#ifdef __cplusplus
#   define EXTERN_C     extern "C"
#else
#   define EXTERN_C
#endif // __cplusplus



#define VHAND_STRLEN 256

//dataglove model
#define USB_MODEL 1
#define WIFI_MODEL 2

//dataglove connection types
#define CONNECTION_NOT_FOUND 0
#define USB_CONNECTION_FOUND 1
#define WIFI_CONNECTION_FOUND 2

//dataglove connection status
#define NOT_CONNECTED 0
#define CONNECTED 1

#define USB_NOT_CONNECTED 1
#define WIFI_NOT_CONNECTED 2
#define USB_CONNECTED 4
#define WIFI_CONNECTED 8

//dataglove connection type
#define CONN_USB 1
#define CONN_WIFI 2

//dataglove method results
#define GLOVE_PARAM_ERROR 0
#define GLOVE_OK 1
#define GLOVE_ERROR_NO_CONN 2
#define GLOVE_ERROR_NO_DATA 4
#define GLOVE_ERROR_STREAMING 8

//dataglove streaming mode
#define PKG_NONE 0			//nessun pacchetto da inviare
#define PKG_QUAT_FINGER 1	//pacchetto quaternion + finger
#define PKG_RAW_FINGER 2	//pacchetto raw data + finger

#define NOT_FOUND 0
#define FOUND 1


#define PKG_ERROR 1
#define NO_ERRORS 0


#ifdef VMG30_SDK_EXPORTS
#define VMG30_API __declspec(dllexport) 
#else
#define VMG30_API __declspec(dllimport) 
#endif

struct VMGlove
{
	 virtual int  SetConnectionParameters(int comport, char *ip) = 0;
	 virtual int  GetConnectionParameters(int *comport, char *ip) = 0;
	 virtual int  Connect(int ConnMethod, int StreamMode) = 0;
	 virtual int  Disconnect() = 0;
	 virtual int  GetConnectionStatus() = 0;

	 virtual int  GetModel() = 0;
	 virtual int  GetID(char *label, int *id) = 0;
	 virtual int  GetFWVersion(int *fw1, int *fw2, int *fw3) = 0;
	 virtual int  GetConnectionMethod(int *connmethod) = 0;
	 virtual int  GetStreamMode(int *strmode) = 0;
	 virtual int  GetCOMSettings(int *COMPORT) = 0;
	 virtual int  GetWiFiSettings(char *ip, char *netmask, char *gateway, int *dhcp) = 0;
	 virtual int  GetAPNSettings(char *apn, char *password) = 0;

	 virtual int  GetFingers(double *fingers) = 0;
	 virtual int  GetAbductions(double *abd) = 0;
	 virtual int GetPalmArch(double *palmarch) = 0;
	 virtual int GetThumbCrossOver(double *thumbco) = 0;
	 virtual int GetPressures(double *press) = 0;
	
	 virtual int  GetAttitudeHand(double *roll, double *pitch, double *yaw) = 0;
	 virtual int  GetQuaternionHand(double *quat1, double *quat2, double *quat3, double *quat4) = 0;
	 virtual int  GetAttitudeWrist(double *roll, double *pitch, double *yaw) = 0;
	 virtual int  GetQuaternionWrist(double *quat1, double *quat2, double *quat3, double *quat4) = 0;

	 virtual int  GetGyrosHand(double *gyro1, double *gyro2, double *gyro3) = 0;
	 virtual int  GetAccelsHand(double *accel1, double *accel2, double *accel3) = 0;
	 virtual int  GetMagnsHand(double *magn1, double *magn2, double *magn3) = 0;
	 virtual int  GetGyrosWrist(double *gyro1, double *gyro2, double *gyro3) = 0;
	 virtual int  GetAccelsWrist(double *accel1, double *accel2, double *accel3) = 0;
	 virtual int  GetMagnsWrist(double *magn1, double *magn2, double *magn3) = 0;

	 virtual unsigned int GetLastPackageTime() = 0;

	 virtual int TurnOFF(int ConnMethod) = 0;

};

//typedef VGlove * VMG30HANDLE;
typedef void * VMG30HANDLE;


// Factory function that creates instances of the Xyz object.
EXTERN_C VMG30_API VMG30HANDLE WINAPI GetVMGlove();
EXTERN_C VMG30_API int WINAPI VMGloveSetConnPar(VMG30HANDLE vh,int comport, char *ip);
EXTERN_C VMG30_API int WINAPI VMGloveGetConnPar(VMG30HANDLE vh,int *comport, char *ip);
EXTERN_C VMG30_API int WINAPI VMGloveConnect(VMG30HANDLE vh,int ConnMethod, int StreamMode);
EXTERN_C VMG30_API int WINAPI VMGloveDisconnect(VMG30HANDLE vh);
EXTERN_C VMG30_API int WINAPI VMGloveGetConnectionStatus(VMG30HANDLE vh);

EXTERN_C VMG30_API int WINAPI VMGloveGetModel(VMG30HANDLE vh);
EXTERN_C VMG30_API int WINAPI VMGloveGetID(VMG30HANDLE vh,char *label, int *id) ;
EXTERN_C VMG30_API int WINAPI VMGloveGetFWVersion(VMG30HANDLE vh,int *fw1, int *fw2, int *fw3) ;
EXTERN_C VMG30_API int WINAPI VMGloveGetConnectionMethod(VMG30HANDLE vh,int *connmethod) ;
EXTERN_C VMG30_API int WINAPI VMGloveGetStreamMode(VMG30HANDLE vh,int *strmode) ;
EXTERN_C VMG30_API int WINAPI VMGloveGetCOMSettings(VMG30HANDLE vh,int *COMPORT) ;
EXTERN_C VMG30_API int WINAPI VMGloveGetWiFiSettings(VMG30HANDLE vh,char *ip, char *netmask, char *gateway, int *dhcp) ;
EXTERN_C VMG30_API int WINAPI VMGloveGetAPNSettings(VMG30HANDLE vh,char *apn, char *password) ;

EXTERN_C VMG30_API int WINAPI VMGloveGetFingers(VMG30HANDLE vh, double *fingers);
EXTERN_C VMG30_API int WINAPI VMGloveGetAbductions(VMG30HANDLE vh, double *abd);
EXTERN_C VMG30_API int WINAPI VMGloveGetPalmArch(VMG30HANDLE vh, double *palmarch);
EXTERN_C VMG30_API int WINAPI VMGloveGetThumbCrossOver(VMG30HANDLE vh, double *thumbco);
EXTERN_C VMG30_API int WINAPI VMGloveGetPressures(VMG30HANDLE vh, double *press);

EXTERN_C VMG30_API int WINAPI VMGloveGetAttitudeHand(VMG30HANDLE vh, double *roll, double *pitch, double *yaw);
EXTERN_C VMG30_API int WINAPI VMGloveGetQuaternionHand(VMG30HANDLE vh, double *quat1, double *quat2, double *quat3, double *quat4);
EXTERN_C VMG30_API int WINAPI VMGloveGetAttitudeWrist(VMG30HANDLE vh, double *roll, double *pitch, double *yaw);
EXTERN_C VMG30_API int WINAPI VMGloveGetQuaternionWrist(VMG30HANDLE vh, double *quat1, double *quat2, double *quat3, double *quat4);

EXTERN_C VMG30_API int WINAPI VMGloveGetGyrosHand(VMG30HANDLE vh, double *gyro1, double *gyro2, double *gyro3);
EXTERN_C VMG30_API int WINAPI VMGloveGetAccelsHand(VMG30HANDLE vh, double *accel1, double *accel2, double *accel3);
EXTERN_C VMG30_API int WINAPI VMGloveGetMagnsHand(VMG30HANDLE vh, double *magn1, double *magn2, double *magn3);
EXTERN_C VMG30_API int WINAPI VMGloveGetGyrosWrist(VMG30HANDLE vh, double *gyro1, double *gyro2, double *gyro3);
EXTERN_C VMG30_API int WINAPI VMGloveGetAccelsWrist(VMG30HANDLE vh, double *accel1, double *accel2, double *accel3);
EXTERN_C VMG30_API int WINAPI VMGloveGetMagnsWrist(VMG30HANDLE vh, double *magn1, double *magn2, double *magn3);

EXTERN_C VMG30_API int WINAPI VMGloveGetLastPackageTime(VMG30HANDLE vh);

EXTERN_C VMG30_API int WINAPI VMGloveTurnOFF(VMG30HANDLE vh, int ConnMethod);