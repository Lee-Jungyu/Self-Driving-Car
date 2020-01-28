#include <iostream>
#include <opencv2/opencv.hpp>
#include <cmath>
#include"SerialPort.h"
#define _USE_MATH_DEFINES 

using namespace cv;
using namespace std;

char port_name[20] = "\\\\.\\COM5";


enum { NORTH, EAST, SOUTH, WEST };

int curDir = EAST;			// 현재 차의 방향.
bool isTurn = false;	// 현재 도는 것.
bool isGo = false;		// 현재 움직이는 것.
int sleepNum = 0;		//
int maxSleep = 10;		// 쉬는 것
int moveNum = 0;		//
int goNum = 0;			//
int caseNum = 0;		// 목표좌표의 방향

Point rpoint, ypoint, gpoint, bpoint, blackpoint, ppoint;

int redX = 0, redY = 0;
int greenX = 0, greenY = 0;
int blueX = 0, blueY = 0;
int blackX = 0, blackY = 0;

int rcnt = 0, gcnt = 0, bcnt = 0, blackcnt = 0;

int yellowX, yellowY, ycnt = 0;
int purpleX, purpleY, pcnt = 0;

double carVector = 0;
double destVector = 0;
Point destination;

Point srcPt;
Point dest[4];
int ptIdx;
SerialPort arduino(port_name);
Mat frame;

VideoCapture capture(1);   //default camera

enum { LEFT, RIGHT };

void changeDisplay();

void turn(int dir) {
	//int maxTics = 70;//8 * 10 +5;
	int leftTics = 68;
	int rightTics = 72;
	char ch;
	if (dir == LEFT) {
		while (leftTics--) {
			//반시계방향 90도
			ch = 'l';
			arduino.writeSerialPort(&ch, 1);
		}
	}
	else {
		while (rightTics--) {
			//시계방향 90도
			ch = 'r';
			arduino.writeSerialPort(&ch, 1);
		}
	}
}

void forward(int distance) {
	int repeatNum = floor(4 * distance / 5.0 + 1.0);
	char ch = 'f';
	while (repeatNum--) {
		cout << 1 << endl;
		arduino.writeSerialPort(&ch, 1);
	}
	//forward
}

void backward(int distance) {
	int repeatNum = floor(4 * distance / 5.0 + 1.0);
	char ch = 'b';
	while (repeatNum--) {
		arduino.writeSerialPort(&ch, 1);
	}
	//backward
}

void stop() {
	int waitNum = 10;
	char ch = 's';
	while (waitNum--) {
		arduino.writeSerialPort(&ch, 1);
	}
	//stop
}

void getCarPoint() {

	changeDisplay();

	ycnt = 0;
	yellowX = 0;
	yellowY = 0;

	pcnt = 0;
	purpleX = 0;
	purpleY = 0;

	for (int y = 0; y < frame.rows; y++)
	{
		uchar *imgPtr = frame.ptr<uchar>(y);
		for (int x = 0; x < frame.cols; x++)
		{
			uchar b = imgPtr[3 * x + 0];
			uchar g = imgPtr[3 * x + 1];
			uchar r = imgPtr[3 * x + 2];

			if (b == 0 && g == 255 && r == 255)
			{
				yellowX += x;
				yellowY += y;
				ycnt++;
			}

			if (b == 255 && g == 0 && r == 255) {
				purpleX += x;
				purpleY += y;
				pcnt++;
			}
		}
	}

	yellowX /= ycnt;
	yellowY /= ycnt;

	ypoint.x = yellowX;
	ypoint.y = yellowY;

	if (pcnt != 0) {
		purpleX /= pcnt;
		purpleY /= pcnt;

		ppoint.x = purpleX;
		ppoint.y = purpleY;
	}
}

double carDeltaX, carDeltaY;
double destDeltaX, destDeltaY;

void getCarVector()
{
	double result = 0;

	//changeDisplay();
	getCarPoint();

	//자동차의 앞부분을 알 수 있어야 한다.

	carDeltaY = (ppoint.y - ypoint.y) *1.0;
	carDeltaX = (ppoint.x - ypoint.x) * 1.0;

	//result = (ppoint.y - ypoint.y) *1.0 / (ppoint.x - ypoint.x) * 1.0;

	carVector = result;
}

void getDestVector()
{
	double result = 0;

	//changeDisplay();

	destDeltaX = (destination.x - ypoint.x) * 1.0;
	destDeltaY = (destination.y - ypoint.y) * 1.0;
	//result = (destination.y - ypoint.y) * 1.0 / (destination.x - ypoint.x) * 1.0;

	destVector = result;
}

void turn2()
{
	//atan2
	double carVecDegree = atan2(carDeltaY, carDeltaX) * 180 / 3.14159;
	double destVecDegree = atan2(destDeltaY, destDeltaX) * 180 / 3.14159;

	bool isLeft;
	char ch;
	if (carVecDegree > destVecDegree + 5) {
		ch = 'l';
		isLeft = true;
		arduino.writeSerialPort(&ch, 1);
	}
	else if (destVecDegree > carVecDegree + 5) {
		ch = 'r';
		isLeft = false;
		arduino.writeSerialPort(&ch, 1);
	}
	cout << "자동차 각도" << "\t" << "목적지 각도" << endl;


	while (abs(carVecDegree-destVecDegree) > 1) {
		
		if (abs(carVecDegree - destVecDegree) < 10) {
			break;
		}
		getCarVector();

		carVecDegree = atan2(carDeltaY, carDeltaX) * 180 / 3.14159;
		destVecDegree = atan2(destDeltaY, destDeltaX) * 180 / 3.14159;

		//cout << carVecDegree << " " << destVecDegree << endl;


		
		//cout << pcnt << endl;
		//돌기
		//waitKey(10);
	}
	cout << 0 << endl;
	stop();
	ch = 's';
	arduino.writeSerialPort(&ch, 1);
}

void forward2()
{
	int destX = destination.x;
	int destY = destination.y;

	char ch = 'f';
	
	arduino.writeSerialPort(&ch, 1);
	
	//cout << "목적지 위치: " << destX << "\t" << destY << endl;
	//cout << "자동차 위치: ";
	while (!(abs(destY - ypoint.y) < 5)) {
		//자동차 앞으로
		getCarPoint();
		//cout << ypoint.x << "\t" << ypoint.y << endl;
		
		//waitKey(100);
	}

	stop();
	ch = 's';
	arduino.writeSerialPort(&ch, 1);
}

void move2()
{
	//자동차의 위치 구하기
	getCarPoint();

	//현재 자동차의 벡터구하기
	getCarVector();

	//목적지의 좌표 구하기
	destination = dest[ptIdx];

	//목적지의 벡터방향 구하기
	getDestVector();
	//벡터방향이 일치할 때 까지 돌기
	turn2();

	//목적지에 도착할 때 까지 가기
	forward2();

	ptIdx++;
}

void setDirect() {
	cout << dest[ptIdx].x << " " << dest[ptIdx].y << endl;
	cout << srcPt.x << " " << srcPt.y << endl;
	if (dest[ptIdx].x > srcPt.x) {
		if (dest[ptIdx].y > srcPt.y) {
			caseNum = 3;
		}
		else if (dest[ptIdx].y == srcPt.y) {
			caseNum = 2;
		}
		else {
			caseNum = 1;
		}
	}
	else if (dest[ptIdx].x == srcPt.x) {
		if (dest[ptIdx].y > srcPt.y) {
			caseNum = 4;
		}
		else {
			caseNum = 0;
		}
	}
	else {
		if (dest[ptIdx].y > srcPt.y) {
			caseNum = 5;
		}
		else if (dest[ptIdx].y == srcPt.y) {
			caseNum = 6;
		}
		else {
			caseNum = 7;
		}
	}
}


void move()
{
	setDirect();
	cout << "방향: " << curDir << " " << "caseNum: " << caseNum << endl;
	cout << "위치: " << srcPt.x << ", " << srcPt.y << endl;

	switch (caseNum) {
	case 0:
		//북쪽으로
		switch (curDir) {
		case NORTH:
		{
			//북방향 북쪽으로
			int y = abs(srcPt.y - dest[ptIdx].y);
			forward(y);

			ptIdx++;

			stop();
		}
		break;
		case EAST:
		{
			//동방향 북쪽으로
			turn(LEFT);
			curDir = NORTH;

			stop();

			int y = abs(srcPt.y - dest[ptIdx].y);
			forward(y);

			stop();

			ptIdx++;
		}
		break;

		case SOUTH:
		{
			//남방향 북쪽으로

			int y = abs(srcPt.y - dest[ptIdx].y);
			backward(y);

			stop();

			ptIdx++;
		}
		break;

		case WEST:
		{
			//서방향 북쪽으로
			turn(RIGHT);
			curDir = NORTH;

			stop();

			int y = abs(srcPt.y - dest[ptIdx].y);
			forward(y);

			stop();

			ptIdx++;
		}
		break;
		default:
			break;
		}
		break;

	case 1:
		//북동 방향
		switch (curDir) {
		case NORTH:
		{
			int y = abs(srcPt.y - dest[ptIdx].y);
			forward(y);

			stop();

			turn(RIGHT);
			curDir = EAST;

			stop();

			int x = abs(srcPt.x - dest[ptIdx].x);
			forward(x);

			stop();

			ptIdx++;
		}
		break;

		case EAST:
		{

			int x = abs(srcPt.x - dest[ptIdx].x);
			forward(x);

			stop();

			turn(LEFT);
			curDir = NORTH;

			stop();

			int y = abs(srcPt.y - dest[ptIdx].y);
			forward(y);

			stop();

			ptIdx++;
		}
		break;

		case SOUTH:
		{
			int y = abs(srcPt.y - dest[ptIdx].y);
			backward(y);

			stop();

			turn(LEFT);
			curDir = EAST;

			stop();

			int x = abs(srcPt.x - dest[ptIdx].x);
			forward(x);

			stop();
			ptIdx++;
		}
		break;

		case WEST:
		{
			int x = abs(srcPt.x - dest[ptIdx].x);
			backward(x);

			stop();

			turn(RIGHT);
			curDir = NORTH;

			int y = abs(srcPt.y - dest[ptIdx].y);
			forward(y);

			stop();

			ptIdx++;
		}
		break;
		default:

			break;
		}

		break;


	case 2:
		//동 방향
		switch (curDir) {
		case NORTH:
		{
			turn(RIGHT);
			curDir = NORTH;

			stop();

			int x = abs(srcPt.x - dest[ptIdx].x);
			forward(x);

			stop();

			ptIdx++;
		}
		break;
		case EAST:
		{
			int x = abs(srcPt.x - dest[ptIdx].x);
			forward(x);

			stop();

			ptIdx++;
		}
		break;
		case SOUTH:
		{
			turn(LEFT);
			curDir = EAST;

			stop();

			int x = abs(srcPt.x - dest[ptIdx].x);
			forward(x);

			stop();

			ptIdx++;
		}
		break;
		case WEST:
		{
			int x = abs(srcPt.x - dest[ptIdx].x);
			backward(x);

			stop();

			ptIdx++;
		}
		break;
		default:

			break;
		}

		break;

	case 3:
		//남동 방향
		switch (curDir) {
		case NORTH:
		{
			int y = abs(srcPt.y - dest[ptIdx].y);
			backward(y);

			stop();

			turn(RIGHT);
			curDir = EAST;

			stop();

			int x = abs(srcPt.x - dest[ptIdx].x);
			forward(x);

			stop();

			ptIdx++;
		}
		break;
		case EAST:
		{
			int x = abs(srcPt.x - dest[ptIdx].x);
			forward(x);

			stop();

			turn(RIGHT);
			curDir = SOUTH;

			stop();

			int y = abs(srcPt.y - dest[ptIdx].y);
			forward(y);

			stop();

			ptIdx++;
		}
		break;
		case SOUTH:
		{
			int y = abs(srcPt.y - dest[ptIdx].y);
			forward(y);

			stop();

			turn(LEFT);
			curDir = EAST;

			stop();

			int x = abs(srcPt.x - dest[ptIdx].x);
			forward(x);

			stop();

			ptIdx++;
		}
		break;
		case WEST:
		{
			int x = abs(srcPt.x - dest[ptIdx].x);
			backward(x);

			stop();

			turn(LEFT);
			curDir = SOUTH;

			stop();

			int y = abs(srcPt.y - dest[ptIdx].y);
			forward(y);

			stop();

			ptIdx++;
		}
		break;
		default:

			break;
		}

		break;

	case 4:
		//남
		switch (curDir) {
		case NORTH:
		{
			int y = abs(srcPt.y - dest[ptIdx].y);
			backward(y);
			stop();
			ptIdx++;
		}
		break;
		case EAST:
		{
			turn(RIGHT);
			stop();
			curDir = SOUTH;
			int y = abs(srcPt.y - dest[ptIdx].y);
			forward(y);
			stop();
			ptIdx++;
		}
		break;
		case SOUTH:
		{
			int y = abs(srcPt.y - dest[ptIdx].y);
			forward(y);
			stop();
			ptIdx++;
		}
		break;
		case WEST:
		{
			turn(LEFT);
			stop();
			curDir = SOUTH;
			int y = abs(srcPt.y - dest[ptIdx].y);
			forward(y);
			stop();
			ptIdx++;
		}
		break;
		default:

			break;
		}

		break;

	case 5:
		//남서
		switch (curDir) {
		case NORTH:
		{
			int y = abs(srcPt.y - dest[ptIdx].y);
			backward(y);
			stop();
			turn(LEFT);
			stop();
			curDir = WEST;
			int x = abs(srcPt.x - dest[ptIdx].x);
			forward(x);
			stop();
			ptIdx++;
		}
		break;
		case EAST:
		{
			int x = abs(srcPt.x - dest[ptIdx].x);
			backward(x);
			stop();
			turn(RIGHT);
			stop();
			curDir = SOUTH;
			int y = abs(srcPt.y - dest[ptIdx].y);
			forward(y);
			stop();
			ptIdx++;
		}
		break;
		case SOUTH:
		{
			int y = abs(srcPt.y - dest[ptIdx].y);
			forward(y);
			stop();
			turn(RIGHT);
			stop();
			curDir = WEST;
			int x = abs(srcPt.x - dest[ptIdx].x);
			forward(x);
			stop();
			ptIdx++;
		}
		break;
		case WEST:
		{
			int x = abs(srcPt.x - dest[ptIdx].x);
			forward(x);
			stop();
			turn(LEFT);
			stop();
			curDir = SOUTH;
			int y = abs(srcPt.y - dest[ptIdx].y);
			forward(y);
			stop();
			ptIdx++;
		}
		break;
		default:

			break;
		}

		break;

	case 6:
		//서
		switch (curDir) {
		case NORTH:
		{
			turn(LEFT);
			stop();
			curDir = WEST;
			int x = abs(srcPt.x - dest[ptIdx].x);
			forward(x);
			stop();
			ptIdx++;
		}
		break;
		case EAST:
		{
			int x = abs(srcPt.x - dest[ptIdx].x);
			backward(x);
			stop();
			ptIdx++;
		}
		break;
		case SOUTH:
		{
			turn(RIGHT);
			stop();
			curDir = WEST;
			int x = abs(srcPt.x - dest[ptIdx].x);
			forward(x);
			stop();
			ptIdx++;
		}
		break;
		case WEST:
		{
			int x = abs(srcPt.x - dest[ptIdx].x);
			forward(x);
			stop();
			ptIdx++;
		}
		break;
		default:

			break;
		}

		break;

	case 7:
		//북서
		switch (curDir) {
		case NORTH:
		{
			int y = abs(srcPt.y - dest[ptIdx].y);
			forward(y);
			stop();
			turn(LEFT);
			stop();
			curDir = WEST;
			int x = abs(srcPt.x - dest[ptIdx].x);
			forward(x);
			stop();
			ptIdx++;
		}
		break;
		case EAST:
		{
			int x = abs(srcPt.x - dest[ptIdx].x);
			backward(x);
			stop();
			turn(LEFT);
			stop();
			curDir = NORTH;
			int y = abs(srcPt.y - dest[ptIdx].y);
			forward(y);
			stop();
			ptIdx++;
		}
		break;
		case SOUTH:
		{
			int y = abs(srcPt.y - dest[ptIdx].y);
			backward(y);
			stop();
			turn(RIGHT);
			stop();
			curDir = WEST;
			int x = abs(srcPt.x - dest[ptIdx].x);
			forward(x);
			stop();
			ptIdx++;
		}
		break;
		case WEST:
		{
			int x = abs(srcPt.x - dest[ptIdx].x);
			forward(x);
			stop();
			turn(RIGHT);
			stop();
			curDir = NORTH;
			int y = abs(srcPt.y - dest[ptIdx].y);
			forward(y);
			stop();
			ptIdx++;
		}
		break;
		default:

			break;
		}

		break;
	}

	srcPt = dest[ptIdx - 1];
}

double getTheta(Point p0, Point p1, int theta0) {
	double theta = 0;

	double distance = sqrt((p0.x - p1.x)*(p0.x - p1.x) + (p0.y - p1.y)*(p0.y - p1.y));

	theta = acos((p1.x - p0.x) / distance);

	theta = 180 / 3.14159 * theta - theta0;
	//theta = 180 / 3.14159 * acos((p1.x - p0.x) / distance) - theta0;

	return theta;
}

void changeDisplay() {
	capture.read(frame);
	for (int y = 0; y < frame.rows; y++)
	{
		uchar *imgPtr = frame.ptr<uchar>(y);
		for (int x = 0; x < frame.cols; x++)
		{
			uchar b = imgPtr[3 * x + 0];
			uchar g = imgPtr[3 * x + 1];
			uchar r = imgPtr[3 * x + 2];

			if (b < 60 && g < 60 && r < 60) 
			{
				//검정
				imgPtr[3 * x + 0] = 127;
				imgPtr[3 * x + 1] = 127;
				imgPtr[3 * x + 2] = 127;
			}

			else if (b > 130 && g > 130 && r > 130)
			{
				if (b > 177) {
					//하양
					imgPtr[3 * x + 0] = 0;
					imgPtr[3 * x + 1] = 0;
					imgPtr[3 * x + 2] = 0;
				}
				else {
					//바닥
					imgPtr[3 * x + 0] = 0;
					imgPtr[3 * x + 1] = 0;
					imgPtr[3 * x + 2] = 0;
				}
			}

			else 
			{
				if (b * 1.0 / (b + g + r) > 0.5) 
				{
					//파랑
					imgPtr[3 * x + 0] = 255;
					imgPtr[3 * x + 1] = 0;
					imgPtr[3 * x + 2] = 0;
				}

				else if (r * 1.0 / (b + g + r) > 0.5) 
				{
					//빨강
					imgPtr[3 * x + 0] = 0;
					imgPtr[3 * x + 1] = 0;
					imgPtr[3 * x + 2] = 255;
				}
				else 
				{
					if (r < 70) 
					{
						//초록
						imgPtr[3 * x + 0] = 0;
						imgPtr[3 * x + 1] = 255;
						imgPtr[3 * x + 2] = 0;
					}
					else if (r > 150 && b * 1.0 / (r + g + b) * 1.0 <= 0.28)
					{
						//노랑
						imgPtr[3 * x + 0] = 0;
						imgPtr[3 * x + 1] = 255;
						imgPtr[3 * x + 2] = 255;
					}
					else if (g * 1.0 / (r + g + b) * 1.0 <= 0.28 && b >= 70 && g <= 100) 
					{
						// 보라
						imgPtr[3 * x + 0] = 255;
						imgPtr[3 * x + 1] = 0;
						imgPtr[3 * x + 2] = 255;
					}
					else {
						//예외
						imgPtr[3 * x + 0] = 0;
						imgPtr[3 * x + 1] = 0;
						imgPtr[3 * x + 2] = 0;
					}
				}
			}
		}
	}
	//dilate(frame, frame, cv::Mat(), Point(-1, -1), 3);
	erode(frame, frame, cv::Mat(), Point(-1, -1), 2);

	waitKey(10);
	imshow("frame", frame);
}

Point calculator(Point* point, int cnt) {
	int xsum = 0, ysum = 0;
	for (int i = 0; i < cnt; i++) {
		xsum += point[i].x;
		ysum += point[i].y;
	}
	return Point(xsum / cnt, ysum / cnt);
}


void getColorPoint()
{
	blackX = 0, blackY = 0;
	redX = 0, redY = 0;
	yellowX = 0, yellowY = 0;
	greenX = 0, greenY = 0;
	blueX = 0, blueY = 0;
	purpleX = 0, purpleY = 0;

	for (int y = 0; y < frame.rows; y++)
	{
		uchar *imgPtr = frame.ptr<uchar>(y);
		for (int x = 0; x < frame.cols; x++)
		{
			uchar b = imgPtr[3 * x + 0];
			uchar g = imgPtr[3 * x + 1];
			uchar r = imgPtr[3 * x + 2];

			if (r == 127 && b == 127 && g == 127) {
				blackX += x;
				blackY += y;
				blackcnt++;
			}

			if (b == 255) {
				blueX += x;
				blueY += y;
				bcnt++;
			}

			if (r == 255) {
				if (g != 255 && b != 255) {
					redX += x;
					redY += y;
					rcnt++;
				}
				else {
					if (b == 255) {
						purpleX += x;
						purpleY += y;
						pcnt++;
					}
					else {
						yellowX += x;
						yellowY += y;
						ycnt++;
					}
				}
			}

			else if (g == 255) {
				greenX += x;
				greenY += y;
				gcnt++;
			}
		}
	}

	ypoint.x = yellowX / ycnt;
	ypoint.y = yellowY / ycnt;

	rpoint.x = redX / rcnt;
	rpoint.y = redY / rcnt;


	gpoint.x = greenX / gcnt;
	gpoint.y = greenY / gcnt;


	bpoint.x = blueX / bcnt;
	bpoint.y = blueY / bcnt;

	blackpoint.x = blackX / blackcnt;
	blackpoint.y = blackY / blackcnt;

	ppoint.x = purpleX / pcnt;
	ppoint.y = purpleY / pcnt;
}

void init()
{
	while (!arduino.isConnected()) {
		waitKey(30);
	}

	ptIdx = 0;

	namedWindow("frame", CV_WINDOW_AUTOSIZE);

	

	changeDisplay();

	//imshow("frame", frame);
	
	//waitKey(10);
	getColorPoint();

	srcPt = ypoint;
	dest[0] = rpoint;
	dest[1] = gpoint;
	dest[2] = bpoint;
	dest[3] = blackpoint;

}
