#pragma once
#include <iostream>
#include <queue>


#define MOVING_AVERAGE_WINDOW 5
#define POSITION_VARIABLE_COUNT 3
#define POSE_VARIABLE_COUNT 11
#define POS_X 0
#define POS_Y 1
#define POS_Z 2
#define ORI_X 3
#define ORI_Y 4
#define ORI_Z 5
#define ORI_W 6
#define ORI_WX 7 // wrist orientations...
#define ORI_WY 8
#define ORI_WZ 9
#define ORI_WW 10


class HandPose
{
	double poseInit[POSE_VARIABLE_COUNT]; // initial position
	double currentDisp[POSE_VARIABLE_COUNT]; // current position displacement

	double MASum[POSITION_VARIABLE_COUNT]; // moving average sums for position
	std::queue<double> MAQueue[POSITION_VARIABLE_COUNT];

	int handState; // current hand state
	//int lastHandState;
	//int handStateDebounceCount; // noise filter
	bool changedInit;
public:
	HandPose(float _pX, float _pY, float _pZ, float _oX, float _oY, float _oZ, float _oW, float _oWX, float _oWY, float _oWZ, float _oWW, int _hS);
	virtual ~HandPose();
	void update(float _pX, float _pY, float _pZ, float _oX, float _oY, float _oZ, float _oW, float _oWX, float _oWY, float _oWZ, float _oWW, int _hS);
	void changeInitPose(float _pX, float _pY, float _pZ, float _oX, float _oY, float _oZ, float _oW, float _oWX, float _oWY, float _oWZ, float _oWW, int _hS);
	friend std::ostream& operator<<(std::ostream& os, const HandPose& dt);
};