#include "stdafx.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv/cxcore.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core_c.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/opencv.hpp>


#include <windows.h>
#include <NuiApi.h> // Microsoft Kinect SDK

//STL

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
using namespace std;
using namespace cv;

using std::cout;
using std::endl;



void InitializeKinect();

int createDepthImage(HANDLE h, IplImage *Depth);
int createRGBImage(HANDLE h, IplImage* Color);
RGBQUAD Nui_ShortToQuad_Depth(USHORT s);
RGBQUAD rgb[640 * 480];

RGBQUAD Nui_ShortToQuad_IR(USHORT s);
RGBQUAD m_irWk[640*480];
FILE *fp;
int tmp = 0;

void createIRImage(HANDLE h, IplImage *InfraRed) {
	const NUI_IMAGE_FRAME *pImageFrame = NULL;
	HRESULT hr = NuiImageStreamGetNextFrame(h, 0, &pImageFrame);
	if (FAILED(hr)) {
		return;
	}
	INuiFrameTexture *pTexture = pImageFrame->pFrameTexture;
	NUI_LOCKED_RECT LockedRect;

	pTexture->LockRect(0, &LockedRect, NULL, 0);
	if (LockedRect.Pitch != 0) {
		BYTE *pBuffer = (BYTE*)LockedRect.pBits;
		RGBQUAD *rgbrun = m_irWk;
		USHORT *pBufferRun = (USHORT*)pBuffer;

		
		for (int y = 0; y < 480; y++) {
			for (int x = 0; x < 640; x++) {

				RGBQUAD quad = Nui_ShortToQuad_IR(*pBufferRun);
				
				Vector4 vec = NuiTransformDepthImageToSkeleton(x, y, *pBufferRun >> 3,NUI_IMAGE_RESOLUTION_640x480);
				if (tmp==70) {
					fprintf(fp, "%f\t%f\t%f\n", vec.x, vec.y, vec.z);
				}
				pBufferRun++;
				*rgbrun = quad;
				rgbrun++;
			}
		}
		tmp++;
		if (tmp > 70) {
			fclose(fp);
		}
		cvSetData(InfraRed, (BYTE*)m_irWk, InfraRed->widthStep);
		cvShowImage("IR Image", InfraRed);
	}NuiImageStreamReleaseFrame(h, pImageFrame);
}

int createRGBImage(HANDLE h, IplImage* Color) {
	
	const NUI_IMAGE_FRAME *pImageFrame = NULL;
	HRESULT hr = NuiImageStreamGetNextFrame(h, 1000, &pImageFrame);
	if (FAILED(hr)) {
		cout << "Create RGB Image Failed\n";
		return -1;
	}
	INuiFrameTexture *pTexture = pImageFrame->pFrameTexture;
	NUI_LOCKED_RECT LockedRect;
	pTexture->LockRect(0, &LockedRect, NULL, 0);

	if (LockedRect.Pitch != 0) {
		BYTE * pBuffer = (BYTE*)LockedRect.pBits;

		cvSetData(Color, pBuffer, LockedRect.Pitch);
		
		cvShowImage("Color Image", Color);
	}
	NuiImageStreamReleaseFrame(h, pImageFrame);
	return 0;
}

void InitializeKinect() {
	bool FailToConnect;
	do {
		HRESULT hr = NuiInitialize(NUI_INITIALIZE_FLAG_USES_COLOR|NUI_INITIALIZE_FLAG_USES_DEPTH| NUI_INITIALIZE_FLAG_USES_SKELETON);
		if (FAILED(hr)) {
			system("cls");
			cout << "Failed";
			FailToConnect = true;
			system("PAUSE");
		}
		else {
			cout << "Connected";
			FailToConnect = false;
		}
	} while (FailToConnect);
}

int createDepthImage(HANDLE h, IplImage *Depth) {
	const NUI_IMAGE_FRAME *pImageFrame = NULL;
	HRESULT hr = NuiImageStreamGetNextFrame(h, 1000, &pImageFrame);
	if (FAILED(hr)) {
		printf("FAILED");
		return -1;
	}
	INuiFrameTexture *pTexture = pImageFrame->pFrameTexture;
	NUI_LOCKED_RECT LockedRect;
	pTexture->LockRect(0, &LockedRect, NULL, 0);

	if (LockedRect.Pitch != 0) {
		BYTE * pBuffer = (BYTE*)LockedRect.pBits;
		RGBQUAD *rgbrun = rgb;
		USHORT *pBufferRun = (USHORT*)pBuffer;
		for (int y = 0; y < 480; y++) {
			for (int x = 0; x < 640; x++) {
				RGBQUAD quad = Nui_ShortToQuad_Depth(*pBufferRun);
				pBufferRun++;
				*rgbrun = quad;
				rgbrun++;
			}
		}
		cvSetData(Depth, (BYTE*)rgb, Depth->widthStep);
		cvShowImage("DepthImage", Depth);
	}
	NuiImageStreamReleaseFrame(h, pImageFrame);

}
RGBQUAD Nui_ShortToQuad_Depth(USHORT s) {
	USHORT realDepth = (s&0xfff8)>>3;
	BYTE I = 255 - (BYTE)(256 * realDepth / (0x0fff));
	RGBQUAD q;
	q.rgbBlue = q.rgbGreen = q.rgbRed = I;
	return q;
}
RGBQUAD Nui_ShortToQuad_IR(USHORT s) {
	USHORT pixel = s >> 8;

	BYTE intensity = pixel;
	RGBQUAD q;
	q.rgbBlue = intensity;
	q.rgbGreen = intensity;
	q.rgbRed = intensity;

	return q;
}
int main(void)
{
	fp = fopen("testdepth.txt", "wt+");
	HANDLE IRStreamHandle;
	HANDLE nextIRFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	HANDLE DepthStreamHandle;
	HANDLE nextDepthFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	HANDLE colorStreamHandle;
	HANDLE nextColorFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	

	


	HRESULT hr;

	InitializeKinect();
	IplImage* Color = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 4);
	IplImage* Depth = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 4);
	IplImage* InfraRed = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 4);

	
	cvNamedWindow("Capture", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("IR image", CV_WINDOW_AUTOSIZE);
	/*
	hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_640x480, 0, 2, nextColorFrameEvent, &colorStreamHandle);
	if (FAILED(hr)) {
		printf("Fail\n");
		return hr;
	}
	hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_DEPTH, NUI_IMAGE_RESOLUTION_640x480, 0, 2, nextDepthFrameEvent, &DepthStreamHandle);
	NuiImageStreamSetImageFrameFlags(DepthStreamHandle, NUI_IMAGE_STREAM_FLAG_ENABLE_NEAR_MODE);

	if (FAILED(hr)) {
		printf("Fail\n");
		return hr;
	}
	*/
	
	hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR_INFRARED, NUI_IMAGE_RESOLUTION_640x480, 0, 2, nextIRFrameEvent, &IRStreamHandle);
	if (FAILED(hr)) {
		printf("Fail\n");
		return hr;
	}
	
	while (1) {
		/*
		WaitForSingleObject(nextColorFrameEvent, 1000);
		createRGBImage(colorStreamHandle, Color);
		*/

		WaitForSingleObject(nextDepthFrameEvent, 0);
		createDepthImage(IRStreamHandle, InfraRed);

		
		
		WaitForSingleObject(nextIRFrameEvent, 1000);
		createIRImage(IRStreamHandle, InfraRed);
		if (cvWaitKey(10) == 0x001b) {
			break;
		}
	}
	NuiShutdown();
	cvReleaseImage(&InfraRed);
	
	
	return 0;

}
