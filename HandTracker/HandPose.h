#pragma once
#include <iostream>
#include <queue>



#define MOVING_AVERAGE_WINDOW 5
#define POSE_VARIABLE_COUNT 7
#define POS_X 0
#define POS_Y 1
#define POS_Z 2
#define ORI_X 3
#define ORI_Y 4
#define ORI_Z 5
#define ORI_W 6

class HandPose
{
	double poseInit[POSE_VARIABLE_COUNT]; // initial position
	double MASum[POSE_VARIABLE_COUNT]; // moving average sums
	std::queue<double> MAQueue[POSE_VARIABLE_COUNT];
	double currentDisp[POSE_VARIABLE_COUNT]; // current position displacement
	int handState; // current hand state
	int lastHandState;
	int handStateDebounceCount; // noise filter
	bool changedInit;
public:
	HandPose(float _pX, float _pY, float _pZ, float _oX, float _oY, float _oZ, float _oW, int _hS);
	virtual ~HandPose();
	void update(float _pX, float _pY, float _pZ, float _oX, float _oY, float _oZ, float _oW, int _hS);
	void changeInitPose(float _pX, float _pY, float _pZ, float _oX, float _oY, float _oZ, float _oW, int _hS);
	friend std::ostream& operator<<(std::ostream& os, const HandPose& dt);
};