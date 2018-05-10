// hand.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//
#define _CRT_SECURE_NO_WARNINGS
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
#include <ppl.h>
#include <windows.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <vector>
#include <math.h>

using namespace cv;
using std::cout;
using std::endl;

Point HandCenter(Mat& mask,double& radius){
	Mat dst;
	distanceTransform(mask, dst, CV_DIST_L2, 5);
	int maxIdx[2];
	minMaxIdx(dst, NULL, &radius, NULL, maxIdx, mask);
	return Point(maxIdx[1],maxIdx[0]);
}

int main() {
	VideoCapture capture(0);
	Mat frame;
	Mat skinMat;
	cvNamedWindow("INPUT_WINDOW", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("RESULT_WINDOW", CV_WINDOW_AUTOSIZE);

	std::vector<std::vector<Point> > contours;
	std::vector<Vec4i> hierarchy;
	vector<Point2f> points;
	vector<int>hull;
	vector<Vec4i>defects;

	while (1) {
		capture.read(frame); //영상캡쳐
		
		cvtColor(frame, skinMat, CV_BGR2YCrCb); //bgr->ycrcb로변경
		inRange(skinMat, Scalar(0, 133, 77), Scalar(255, 173, 127), skinMat); //skin색 범위 설정, 흑백변환
		Mat1b frame_gray;
		frame_gray = skinMat.clone();
		//영상이진화
		threshold(frame_gray, frame_gray, 60, 255, CV_THRESH_BINARY);
		morphologyEx(frame_gray, frame_gray, CV_MOP_ERODE, Mat1b(3, 3, 1), Point(-1, -1), 3);
		morphologyEx(frame_gray, frame_gray, CV_MOP_OPEN, Mat1b(7, 7, 1), Point(-1, -1), 1);
		morphologyEx(frame_gray, frame_gray, CV_MOP_CLOSE, Mat1b(9, 9, 1), Point(-1, -1), 1);
		//잡음제거
		medianBlur(frame_gray, frame_gray, 15);
		
		//외곽선
		findContours(frame_gray, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
		//제일 큰 외곽선
		int largestContour = 0;
		for (int i = 0; i < contours.size(); i++) {
			if (contourArea(contours[i]) > contourArea(contours[largestContour])) {
				largestContour = i;
			}
		}
		drawContours(frame, contours, largestContour, Scalar(0, 255, 255), 1);

		
		
		if (contours.size()!=0) { 
			
			
			//손 중심점 그리기
			Mat frame2 = skinMat.clone();
			erode(frame2, frame2, Mat(3, 3, CV_8U, Scalar(1)), Point(-1, -1), 2);
			double radius;
			Point center = HandCenter(frame2, radius);
			circle(frame, center, 2, Scalar(0, 255, 0), 4);
			//손 외곽선의 최외곽점 잇기
			
			//중심점,외곽선 point를 이용한 손끝찾기
			int len, max = 0, x, y, check,check_x, prev_x = contours[largestContour][0].x,prev_y=0;
			int find=0;
			for (size_t p = 1; p < contours[largestContour].size();p++) {
				Point cp = contours[largestContour][p];
				x = center.x - cp.x;
				y = center.y - cp.y;
				len = sqrt((x*x)+(y*y));
				check = len - max;
				check_x = prev_x - cp.x;
				if (find==0&&check>0){
					max = len;
				}
				if (find != 0 && check > 0) {
					find = 0;
				}
				else if(x<0&&find==0&&check>-10&&len>200&&check_x==0){
					circle(frame, cp, 25, Scalar(0, 0, 255), 2);
					line(frame, center, cp, Scalar(255, 255, 0), 2);
					p += 80;
					max = 0;
					find = 1;
				}prev_x = cp.x;
			}
			
			//vector<Vec4i>defects;
			//볼록 결함 찾기, skeleton line 그리기
			/*
			vector<Point> handContour= contours[largestContour];

			convexHull(handContour, hull);
			vector<Point>ptsHull;

			drawContours(frame, vector<vector<Point>>(1, ptsHull), 0, Scalar(255, 0, 0), 3);
			convexityDefects(handContour, hull, defects);
			int i = 0,count = 0;
			float angle=0;
			for (int k = 0; k < defects.size(); k++) {
				Vec4i v = defects[k];
				Point ptStart = handContour[v[0]];
				Point ptEnd = handContour[v[1]];
				Point ptFar = handContour[v[2]];
				float depth = v[3] / 256.0f;
				if (depth >15) {
					//손가락사이 각도
					if (ptEnd.x - center.x != 0 && ptStart.x - center.x != 0) {
						angle = atan((float)(ptStart.y - center.y) / (float)(ptStart.x - center.x)) - atan((float)(ptEnd.y-center.y) / (float)(ptEnd.x-center.x));
						angle = (angle * 180) / 3.1415f;
					}
					if (angle > 0) {
						count++;
						circle(frame, ptStart, 6, Scalar(0, 0, 255), 2);
						circle(frame, ptEnd, 6, Scalar(255, 0, 255), 2);
						//circle(frame, ptFar, 6, Scalar(255, 0, 255), 2);
						line(frame,center,ptStart,Scalar(255,255,0),2);
						cout << i << ". angle : " << angle << endl;
						cout << "center : " << center << endl;
						cout << "fingertip : " << ptStart << endl;
						i++;
					}
				}
				//측면
				if (k==defects.size()-1&&count == 1) {
					line(frame, center, ptEnd, Scalar(255, 255, 0), 2); 
					cout << "fingertip2 : " << ptEnd << endl;//검지
				}
				
			}
			
			*/
			
		}

		imshow("INPUT_WINDOW", frame_gray);
		imshow("RESULT_WINDOW", frame);
		if (cvWaitKey(10)>0)
			break;
	}
	frame.release();
	cvDestroyAllWindows();
	return 0;
}
