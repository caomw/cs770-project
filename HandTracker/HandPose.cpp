#include "HandPose.h"
#include <iomanip>      // std::setprecision

HandPose::HandPose(float _pX, float _pY, float _pZ, float _oX, float _oY, float _oZ, float _oW, int _hS)
{
	// set initial positions
	poseInit[POS_X] = _pX;
	poseInit[POS_Y] = _pY;
	poseInit[POS_Z] = _pZ;
	poseInit[ORI_X] = _oX;
	poseInit[ORI_Y] = _oY;
	poseInit[ORI_Z] = _oZ;
	poseInit[ORI_W] = _oW;

	// inialize moving averages
	MASum[POS_X] = _pX * MOVING_AVERAGE_WINDOW;
	MASum[POS_Y] = _pY * MOVING_AVERAGE_WINDOW;
	MASum[POS_Z] = _pZ * MOVING_AVERAGE_WINDOW;
	MASum[ORI_X] = _oX * MOVING_AVERAGE_WINDOW;
	MASum[ORI_Y] = _oY * MOVING_AVERAGE_WINDOW;
	MASum[ORI_Z] = _oZ * MOVING_AVERAGE_WINDOW;
	MASum[ORI_W] = _oW * MOVING_AVERAGE_WINDOW;

	for (int i = 0; i < MOVING_AVERAGE_WINDOW; i++)
	{
		MAQueue[POS_X].push(_pX);
		MAQueue[POS_Y].push(_pY);
		MAQueue[POS_Z].push(_pZ);
		MAQueue[ORI_X].push(_oX);
		MAQueue[ORI_Y].push(_oY);
		MAQueue[ORI_Z].push(_oZ);
		MAQueue[ORI_W].push(_oW);
	}

	// initialize displacements

	currentDisp[POS_X] = 0.0;
	currentDisp[POS_Y] = 0.0;
	currentDisp[POS_Z] = 0.0;
	/*currentDisp[ORI_X] = 0.0;
	currentDisp[ORI_Y] = 0.0;
	currentDisp[ORI_Z] = 0.0;
	currentDisp[ORI_W] = 0.0;*/
	currentDisp[ORI_X] = (MASum[ORI_X] / MOVING_AVERAGE_WINDOW);
	currentDisp[ORI_Y] = (MASum[ORI_Y] / MOVING_AVERAGE_WINDOW);
	currentDisp[ORI_Z] = (MASum[ORI_Z] / MOVING_AVERAGE_WINDOW);
	currentDisp[ORI_W] = (MASum[ORI_W] / MOVING_AVERAGE_WINDOW);

	// inialize hand state

	handState = _hS; // current hand state
	lastHandState = _hS;
	handStateDebounceCount = 0; // noise filter

	// signal init set
	changedInit = true;
}


HandPose::~HandPose()
{

}

void HandPose::update(float _pX, float _pY, float _pZ, float _oX, float _oY, float _oZ, float _oW, int _hS)
{
	// update moving average sums
	MASum[POS_X] = MASum[POS_X] - MAQueue[POS_X].front() + _pX;
	MASum[POS_Y] = MASum[POS_Y] - MAQueue[POS_Y].front() + _pY;
	MASum[POS_Z] = MASum[POS_Z] - MAQueue[POS_Z].front() + _pZ;
	MASum[ORI_X] = MASum[ORI_X] - MAQueue[ORI_X].front() + _oX;
	MASum[ORI_Y] = MASum[ORI_Y] - MAQueue[ORI_Y].front() + _oY;
	MASum[ORI_Z] = MASum[ORI_Z] - MAQueue[ORI_Z].front() + _oZ;
	MASum[ORI_W] = MASum[ORI_W] - MAQueue[ORI_W].front() + _oW;


	// update displacements
	currentDisp[POS_X] = poseInit[POS_X] - (MASum[POS_X] / MOVING_AVERAGE_WINDOW);
	currentDisp[POS_Y] = poseInit[POS_Y] - (MASum[POS_Y] / MOVING_AVERAGE_WINDOW);
	currentDisp[POS_Z] = poseInit[POS_Z] - (MASum[POS_Z] / MOVING_AVERAGE_WINDOW);
	/*currentDisp[ORI_X] = poseInit[ORI_X] - (MASum[ORI_X] / MOVING_AVERAGE_WINDOW);
	currentDisp[ORI_Y] = poseInit[ORI_Y] - (MASum[ORI_Y] / MOVING_AVERAGE_WINDOW);
	currentDisp[ORI_Z] = poseInit[ORI_Z] - (MASum[ORI_Z] / MOVING_AVERAGE_WINDOW);
	currentDisp[ORI_W] = poseInit[ORI_W] - (MASum[ORI_W] / MOVING_AVERAGE_WINDOW);*/
	currentDisp[ORI_X] = (MASum[ORI_X] / MOVING_AVERAGE_WINDOW);
	currentDisp[ORI_Y] = (MASum[ORI_Y] / MOVING_AVERAGE_WINDOW);
	currentDisp[ORI_Z] = (MASum[ORI_Z] / MOVING_AVERAGE_WINDOW);
	currentDisp[ORI_W] = (MASum[ORI_W] / MOVING_AVERAGE_WINDOW);

	// update moving average queues
	MAQueue[POS_X].pop();
	MAQueue[POS_Y].pop();
	MAQueue[POS_Z].pop();
	MAQueue[ORI_X].pop();
	MAQueue[ORI_Y].pop();
	MAQueue[ORI_Z].pop();
	MAQueue[ORI_W].pop();
	MAQueue[POS_X].push(_pX);
	MAQueue[POS_Y].push(_pY);
	MAQueue[POS_Z].push(_pZ);
	MAQueue[ORI_X].push(_oX);
	MAQueue[ORI_Y].push(_oY);
	MAQueue[ORI_Z].push(_oZ);
	MAQueue[ORI_W].push(_oW);

	// debounce hand state
	if (_hS == lastHandState)
	{
		if (handStateDebounceCount == MOVING_AVERAGE_WINDOW)
		{
			handState = _hS;
			handStateDebounceCount = 0;
		}
		else
		{
			handStateDebounceCount++;
		}
	}
	else
	{
		handStateDebounceCount = 0;
	}
	lastHandState = _hS;

	// signal init set
	changedInit = false;
}

void HandPose::changeInitPose(float _pX, float _pY, float _pZ, float _oX, float _oY, float _oZ, float _oW, int _hS)
{
	// set initial positions
	poseInit[POS_X] = _pX;
	poseInit[POS_Y] = _pY;
	poseInit[POS_Z] = _pZ;
	poseInit[ORI_X] = _oX;
	poseInit[ORI_Y] = _oY;
	poseInit[ORI_Z] = _oZ;
	poseInit[ORI_W] = _oW;

	// inialize moving averages
	MASum[POS_X] = _pX * MOVING_AVERAGE_WINDOW;
	MASum[POS_Y] = _pY * MOVING_AVERAGE_WINDOW;
	MASum[POS_Z] = _pZ * MOVING_AVERAGE_WINDOW;
	MASum[ORI_X] = _oX * MOVING_AVERAGE_WINDOW;
	MASum[ORI_Y] = _oY * MOVING_AVERAGE_WINDOW;
	MASum[ORI_Z] = _oZ * MOVING_AVERAGE_WINDOW;
	MASum[ORI_W] = _oW * MOVING_AVERAGE_WINDOW;

	// clear queue
	for (int i = 0; i < MOVING_AVERAGE_WINDOW; i++)
	{
		MAQueue[POS_X].pop();
		MAQueue[POS_Y].pop();
		MAQueue[POS_Z].pop();
		MAQueue[ORI_X].pop();
		MAQueue[ORI_Y].pop();
		MAQueue[ORI_Z].pop();
		MAQueue[ORI_W].pop();
	}

	//rebuild queue
	for (int i = 0; i < MOVING_AVERAGE_WINDOW; i++)
	{
		MAQueue[POS_X].push(_pX);
		MAQueue[POS_Y].push(_pY);
		MAQueue[POS_Z].push(_pZ);
		MAQueue[ORI_X].push(_oX);
		MAQueue[ORI_Y].push(_oY);
		MAQueue[ORI_Z].push(_oZ);
		MAQueue[ORI_W].push(_oW);
	}

	// initialize displacements

	currentDisp[POS_X] = 0.0;
	currentDisp[POS_Y] = 0.0;
	currentDisp[POS_Z] = 0.0;
	/*currentDisp[ORI_X] = 0.0;
	currentDisp[ORI_Y] = 0.0;
	currentDisp[ORI_Z] = 0.0;
	currentDisp[ORI_W] = 0.0;*/
	currentDisp[ORI_X] = (MASum[ORI_X] / MOVING_AVERAGE_WINDOW);
	currentDisp[ORI_Y] = (MASum[ORI_Y] / MOVING_AVERAGE_WINDOW);
	currentDisp[ORI_Z] = (MASum[ORI_Z] / MOVING_AVERAGE_WINDOW);
	currentDisp[ORI_W] = (MASum[ORI_W] / MOVING_AVERAGE_WINDOW);

	// inialize hand state

	handState = _hS; // current hand state
	lastHandState = _hS;
	handStateDebounceCount = 0; // noise filter

	// signal init set
	changedInit = true;
}

/// <summary>
/// outstream operator - outputs displacement
/// </summary>
std::ostream& operator<<(std::ostream& os, const HandPose& dt)
{
	os << std::fixed << std::setprecision(2) << dt.currentDisp[POS_X] << "," << dt.currentDisp[POS_Y] << "," << dt.currentDisp[POS_Z]
		<< "," << std::setprecision(5) << dt.currentDisp[ORI_X] << "," << dt.currentDisp[ORI_Y] << "," << dt.currentDisp[ORI_Z]
		<< "," << dt.currentDisp[ORI_W] << "," << dt.handState << "|" << dt.poseInit[ORI_X] << "," << dt.poseInit[ORI_Y] << "," << dt.poseInit[ORI_Z] << "," << dt.poseInit[ORI_W];
	return os;
}