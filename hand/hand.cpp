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

Point joint, first_finger, second_finger,third_finger,center;
float x, y, l1, l2;
float L;
int count,move;
Point HandCenter(Mat& mask,double& radius){
	Mat dst;
	distanceTransform(mask, dst, CV_DIST_L2, 5);
	int maxIdx[2];
	minMaxIdx(dst, NULL, &radius, NULL, maxIdx, mask);
	return Point(maxIdx[1],maxIdx[0]);
}

int main() {
	VideoCapture capture(0);
	Mat frame, frame2;
	Mat skinMat;
	
	cvNamedWindow("RESULT_WINDOW", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("convex_WINDOW", CV_WINDOW_AUTOSIZE);
	

	std::vector<std::vector<Point> > contours;
	std::vector<Vec4i> hierarchy;
	vector<Point2f> points;
	vector<int>hull;
	vector<Vec4i>defects;

	while (1) {
		int Start_num[2][20];
		int End_num[2][20];
		
		capture.read(frame); //영상캡쳐
		frame2 = frame.clone();
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



		if (contours.size() != 0) {


			//손 중심점 그리기
			Mat centerframe = skinMat.clone();
			erode(centerframe, centerframe, Mat(3, 3, CV_8U, Scalar(1)), Point(-1, -1), 2);
			double radius;
			center = HandCenter(centerframe, radius);
			circle(frame2, center, 4, Scalar(0, 255, 0), 4);

			//정면
			//중심점,외곽선 point를 이용한 손끝찾기
			int len, max = 0, x, y, check, check_x, prev_x = contours[largestContour][0].x, prev_y = 0;
			int find = 0;
			for (size_t p = 1; p < contours[largestContour].size(); p++) {
				Point cp = contours[largestContour][p];
				x = center.x - cp.x;
				y = center.y - cp.y;
				len = sqrt((x*x) + (y*y));
				check = len - max;
				check_x = prev_x - cp.x;
				if (find == 0 && check > 0) {
					max = len;
				}
				if (find != 0 && check > 0) {
					find = 0;
				}
				else if (x<0 && find == 0 && check>-10 && len > 180 && check_x == 0) {
					circle(frame, cp, 25, Scalar(0, 0, 255), 2);
					line(frame, center, cp, Scalar(255, 255, 0), 2);
					p += 80;
					max = 0;
					find = 1;
				}prev_x = cp.x;
			}


			//측면
			//손 외곽선의 최외곽점 잇기
			//볼록 결함 찾기, skeleton line 그리기

			vector<Point> handContour = contours[largestContour];

			convexHull(handContour, hull);

			vector<Point>ptsHull;

			drawContours(frame2, vector<vector<Point>>(1, ptsHull), 0, Scalar(255, 0, 0), 3);
			convexityDefects(handContour, hull, defects);
			count = 0;
			float angle = 0;
			for (int k = 0; k < defects.size(); k++) {
				Vec4i v = defects[k];
				Point ptStart = handContour[v[0]];
				Point ptEnd = handContour[v[1]];
				Point ptFar = handContour[v[2]];
				
				
			
				float depth = v[3] / 256.0f;
				if (depth > 25) {
					//손가락사이 각도
					if (ptEnd.x - center.x != 0 && ptStart.x - center.x != 0) {
						angle = atan((float)(ptStart.y - center.y) / (float)(ptStart.x - center.x)) - atan((float)(ptEnd.y - center.y) / (float)(ptEnd.x - center.x));
						angle = (angle * 180) / 3.1415f;
					}
					if (angle > 0&&ptStart.x>300) {
						circle(frame2, ptStart, 6, Scalar(0, 0, 255), 2);
						circle(frame2, ptEnd, 6, Scalar(255, 0, 255), 2);

						//circle(frame2, ptFar, 6, Scalar(255, 0, 255), 2);
						line(frame2, center, ptStart, Scalar(255, 255, 0), 2);
						line(frame2, center, ptEnd, Scalar(255, 255, 0), 2);
						//cout << count << ". angle : " << angle << endl;
						//cout << "center : " << center << endl;
						//cout << "fingertip : " << ptStart << endl;
						//cout << "fingertip2 : " << ptEnd << endl;//검지
						Start_num[0][count] = ptStart.x;
						Start_num[1][count] = ptStart.y;
						End_num[0][count] = ptEnd.x;
						End_num[1][count] = ptEnd.y;
						count++;

						
					}
				}


			}
			//관절값을 알고, 2개의 convexhull을 잡았을 때 관절의 각도 값 계산
			if ((joint.x) != NULL&&count<=2) {
				joint.x = center.x;
				joint.y = center.y - move;
				float cal_l2,angle1, angle2,gamma,alpha,i=0;
				float end =3.1415f;
				if (count == 2) {
					x = fabs(End_num[0][1]-joint.x);
					y = fabs(End_num[1][1]-joint.y);
				}
				else {
					x=fabs(End_num[0][0] - joint.x);
					y=fabs(End_num[1][0] - joint.y);
				}
				
				cout << "*********************************" << endl;
				cout << "[x,y] : [" << x << "," << y << "]" << endl;
				int x2 = pow(x, 2);
				int y2 = pow(y, 2);
				l1 = L / 2;
				while (i <= end) {
					if (fabs(((x2 + y2-(2*pow(l1,2)))/(-1*(2*pow(l1,2))))-cos(i))<0.05) {
						angle2 =180 - (i * 180 / 3.1415f);
						angle2 = angle2 * 2 / 3;
						cal_l2 = ((180 - angle2)*3.1415f / 180) - i;
						l2 = l1 / (2*cos(cal_l2));
						cout << "l1 : " << l1 << endl;
						cout << "l2 : " << l2 << endl;
						break;
					}
					i+=0.1;
				}i = 0;
				while (i <= end) {
					if (fabs(tan(i) - (y / x))<0.05) {
						gamma = i;
						break;
					}
					i += 0.1;
				}
				i = 0;
				while (i <= end) {
					if (fabs(((x2 + y2) / (2 * l1*sqrt(x2 + y2))) - cos(i))<0.05) {
						alpha = i;
						break;
					}
					i += 0.1;
				}
				
				
				angle1 = fabs(gamma - alpha)*180/3.1415f;
				
				cout << "gamma : " << gamma * 180 / 3.1415f <<", alpha : "<<alpha * 180 / 3.1415f << endl;
				cout << "angle1 : " << angle1 << endl;
				cout << "angle2 : " << angle2 << endl;
				angle1 = angle1*3/5 * 3.1415f / 180 ;
				angle2 = angle2*4/5 * 3.1415f / 180 ;

				Point tmp0, tmp1, tmp2, tmp3;
				tmp0.x = joint.x; tmp0.y = joint.y; 
				tmp1.x = l1 * cos(angle1)+joint.x; tmp1.y = l1 * sin(angle1)+joint.y; 
				tmp2.x = tmp1.x + (l2*cos(angle2 + angle1)); tmp2.y = tmp1.y + (l2*sin(angle1 + angle2));
				tmp3.x = tmp2.x + (l2*cos(angle1 + angle2 + angle2)); tmp3.y = tmp2.y + (l2*sin(angle1+angle2+angle2));
				circle(frame2, joint, 6, Scalar(255, 0, 255), 2);

				line(frame2, tmp0, tmp1, Scalar(0, 0, 255), 2);
				line(frame2, tmp1, tmp2, Scalar(0, 0, 255), 2);
				line(frame2, tmp2, tmp3, Scalar(0, 0, 255), 2);
			}

			
			
		}
		
		imshow("RESULT_WINDOW", frame);
		imshow("convex_WINDOW", frame2);

		if (cvWaitKey(10) == 13){
			cout << "initializing" << endl;
			joint.x = center.x;
			joint.y = End_num[1][0];
			move = abs(End_num[1][0]-center.y);
			second_finger.x = End_num[0][0];
			second_finger.y = End_num[1][0];
			first_finger.x = Start_num[0][0];
			first_finger.y = Start_num[1][0];
			L = sqrt(pow(fabs(second_finger.x - joint.x), 2) + pow(fabs(joint.y - second_finger.y), 2)); //손가락 길이 계산
		
			cout << "joint [" << joint.x << "," << joint.y << "]" << endl;
			cout << "first_finger [" << first_finger.x << "," << first_finger.y << "]" << endl;
			cout << "second_finger [" << second_finger.x << "," << second_finger.y << "]" << endl;
			cout << "length : " << L << endl;
		}
	}
	frame.release();
	cvDestroyAllWindows();
	return 0;
}
