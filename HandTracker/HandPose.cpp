#include "HandPose.h"
#include <iomanip>      // std::setprecision

HandPose::HandPose(float _pX, float _pY, float _pZ, float _oX, float _oY, float _oZ, float _oW, float _oWX, float _oWY, float _oWZ, float _oWW, int _hS)
{
	// set initial positions
	poseInit[POS_X] = _pX;
	poseInit[POS_Y] = _pY;
	poseInit[POS_Z] = _pZ;
	poseInit[ORI_X] = _oX;
	poseInit[ORI_Y] = _oY;
	poseInit[ORI_Z] = _oZ;
	poseInit[ORI_W] = _oW;
	poseInit[ORI_WX] = _oWX;
	poseInit[ORI_WY] = _oWY;
	poseInit[ORI_WZ] = _oWZ;
	poseInit[ORI_WW] = _oWW;


	// inialize moving averages
	MASum[POS_X] = _pX * MOVING_AVERAGE_WINDOW;
	MASum[POS_Y] = _pY * MOVING_AVERAGE_WINDOW;
	MASum[POS_Z] = _pZ * MOVING_AVERAGE_WINDOW;
	MASum[ORI_X] = _oX * MOVING_AVERAGE_WINDOW;
	MASum[ORI_Y] = _oY * MOVING_AVERAGE_WINDOW;
	MASum[ORI_Z] = _oZ * MOVING_AVERAGE_WINDOW;
	MASum[ORI_W] = _oW * MOVING_AVERAGE_WINDOW;
	MASum[ORI_WX] = _oWX * MOVING_AVERAGE_WINDOW;
	MASum[ORI_WY] = _oWY * MOVING_AVERAGE_WINDOW;
	MASum[ORI_WZ] = _oWZ * MOVING_AVERAGE_WINDOW;
	MASum[ORI_WW] = _oWW * MOVING_AVERAGE_WINDOW;

	for (int i = 0; i < MOVING_AVERAGE_WINDOW; i++)
	{
		MAQueue[POS_X].push(_pX);
		MAQueue[POS_Y].push(_pY);
		MAQueue[POS_Z].push(_pZ);
		MAQueue[ORI_X].push(_oX);
		MAQueue[ORI_Y].push(_oY);
		MAQueue[ORI_Z].push(_oZ);
		MAQueue[ORI_W].push(_oW);
		MAQueue[ORI_WX].push(_oWX);
		MAQueue[ORI_WY].push(_oWY);
		MAQueue[ORI_WZ].push(_oWZ);
		MAQueue[ORI_WW].push(_oWW);
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
	currentDisp[ORI_WX] = (MASum[ORI_WX] / MOVING_AVERAGE_WINDOW);
	currentDisp[ORI_WY] = (MASum[ORI_WY] / MOVING_AVERAGE_WINDOW);
	currentDisp[ORI_WZ] = (MASum[ORI_WZ] / MOVING_AVERAGE_WINDOW);
	currentDisp[ORI_WW] = (MASum[ORI_WW] / MOVING_AVERAGE_WINDOW);

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

void HandPose::update(float _pX, float _pY, float _pZ, float _oX, float _oY, float _oZ, float _oW, float _oWX, float _oWY, float _oWZ, float _oWW, int _hS)
{
	// update moving average sums
	MASum[POS_X] = MASum[POS_X] - MAQueue[POS_X].front() + _pX;
	MASum[POS_Y] = MASum[POS_Y] - MAQueue[POS_Y].front() + _pY;
	MASum[POS_Z] = MASum[POS_Z] - MAQueue[POS_Z].front() + _pZ;
	MASum[ORI_X] = MASum[ORI_X] - MAQueue[ORI_X].front() + _oX;
	MASum[ORI_Y] = MASum[ORI_Y] - MAQueue[ORI_Y].front() + _oY;
	MASum[ORI_Z] = MASum[ORI_Z] - MAQueue[ORI_Z].front() + _oZ;
	MASum[ORI_W] = MASum[ORI_W] - MAQueue[ORI_W].front() + _oW;
	MASum[ORI_WX] = MASum[ORI_WX] - MAQueue[ORI_WX].front() + _oWX;
	MASum[ORI_WY] = MASum[ORI_WY] - MAQueue[ORI_WY].front() + _oWY;
	MASum[ORI_WZ] = MASum[ORI_WZ] - MAQueue[ORI_WZ].front() + _oWZ;
	MASum[ORI_WW] = MASum[ORI_WW] - MAQueue[ORI_WW].front() + _oWW;


	// update displacements
	currentDisp[POS_X] = poseInit[POS_X] - (MASum[POS_X] / MOVING_AVERAGE_WINDOW);
	currentDisp[POS_Y] = poseInit[POS_Y] - (MASum[POS_Y] / MOVING_AVERAGE_WINDOW);
	currentDisp[POS_Z] = poseInit[POS_Z] - (MASum[POS_Z] / MOVING_AVERAGE_WINDOW);
	/*currentDisp[ORI_X] = poseInit[ORI_X] - (MASum[ORI_X] / MOVING_AVERAGE_WINDOW);
	currentDisp[ORI_Y] = poseInit[ORI_Y] - (MASum[ORI_Y] / MOVING_AVERAGE_WINDOW);
	currentDisp[ORI_Z] = poseInit[ORI_Z] - (MASum[ORI_Z] / MOVING_AVERAGE_WINDOW);
	currentDisp[ORI_W] = poseInit[ORI_W] - (MASum[ORI_W] / MOVING_AVERAGE_WINDOW);*/

	// currentDisp[ORI_X] = (MASum[ORI_X] / MOVING_AVERAGE_WINDOW);
	// currentDisp[ORI_Y] = (MASum[ORI_Y] / MOVING_AVERAGE_WINDOW);
	// currentDisp[ORI_Z] = (MASum[ORI_Z] / MOVING_AVERAGE_WINDOW);
	// currentDisp[ORI_W] = (MASum[ORI_W] / MOVING_AVERAGE_WINDOW);
	// currentDisp[ORI_WX] = (MASum[ORI_WX] / MOVING_AVERAGE_WINDOW);
	// currentDisp[ORI_WY] = (MASum[ORI_WY] / MOVING_AVERAGE_WINDOW);
	// currentDisp[ORI_WZ] = (MASum[ORI_WZ] / MOVING_AVERAGE_WINDOW);
	// currentDisp[ORI_WW] = (MASum[ORI_WW] / MOVING_AVERAGE_WINDOW);

	//use raw data for orientation quaternions:

	currentDisp[ORI_X] = _oX;
	currentDisp[ORI_Y] = _oY;
	currentDisp[ORI_Z] = _oZ;
	currentDisp[ORI_W] = _oW;
	currentDisp[ORI_WX] = _oWX;
	currentDisp[ORI_WY] = _oWY;
	currentDisp[ORI_WZ] = _oWZ;
	currentDisp[ORI_WW] = _oWW;

	// update moving average queues
	MAQueue[POS_X].pop();
	MAQueue[POS_Y].pop();
	MAQueue[POS_Z].pop();
	MAQueue[ORI_X].pop();
	MAQueue[ORI_Y].pop();
	MAQueue[ORI_Z].pop();
	MAQueue[ORI_W].pop();
	MAQueue[ORI_WX].pop();
	MAQueue[ORI_WY].pop();
	MAQueue[ORI_WZ].pop();
	MAQueue[ORI_WW].pop();
	MAQueue[POS_X].push(_pX);
	MAQueue[POS_Y].push(_pY);
	MAQueue[POS_Z].push(_pZ);
	MAQueue[ORI_X].push(_oX);
	MAQueue[ORI_Y].push(_oY);
	MAQueue[ORI_Z].push(_oZ);
	MAQueue[ORI_W].push(_oW);
	MAQueue[ORI_WX].push(_oWX);
	MAQueue[ORI_WY].push(_oWY);
	MAQueue[ORI_WZ].push(_oWZ);
	MAQueue[ORI_WW].push(_oWW);

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

void HandPose::changeInitPose(float _pX, float _pY, float _pZ, float _oX, float _oY, float _oZ, float _oW, float _oWX, float _oWY, float _oWZ, float _oWW, int _hS)
{
	// set initial positions
	poseInit[POS_X] = _pX;
	poseInit[POS_Y] = _pY;
	poseInit[POS_Z] = _pZ;
	poseInit[ORI_X] = _oX;
	poseInit[ORI_Y] = _oY;
	poseInit[ORI_Z] = _oZ;
	poseInit[ORI_W] = _oW;
	poseInit[ORI_WX] = _oWX;
	poseInit[ORI_WY] = _oWY;
	poseInit[ORI_WZ] = _oWZ;
	poseInit[ORI_WW] = _oWW;

	// inialize moving averages
	MASum[POS_X] = _pX * MOVING_AVERAGE_WINDOW;
	MASum[POS_Y] = _pY * MOVING_AVERAGE_WINDOW;
	MASum[POS_Z] = _pZ * MOVING_AVERAGE_WINDOW;
	MASum[ORI_X] = _oX * MOVING_AVERAGE_WINDOW;
	MASum[ORI_Y] = _oY * MOVING_AVERAGE_WINDOW;
	MASum[ORI_Z] = _oZ * MOVING_AVERAGE_WINDOW;
	MASum[ORI_W] = _oW * MOVING_AVERAGE_WINDOW;
	MASum[ORI_WX] = _oWX * MOVING_AVERAGE_WINDOW;
	MASum[ORI_WY] = _oWY * MOVING_AVERAGE_WINDOW;
	MASum[ORI_WZ] = _oWZ * MOVING_AVERAGE_WINDOW;
	MASum[ORI_WW] = _oWW * MOVING_AVERAGE_WINDOW;

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
		MAQueue[ORI_WX].pop();
		MAQueue[ORI_WY].pop();
		MAQueue[ORI_WZ].pop();
		MAQueue[ORI_WW].pop();
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
		MAQueue[ORI_WX].push(_oWX);
		MAQueue[ORI_WY].push(_oWY);
		MAQueue[ORI_WZ].push(_oWZ);
		MAQueue[ORI_WW].push(_oWW);
	}

	// initialize displacements

	currentDisp[POS_X] = 0.0;
	currentDisp[POS_Y] = 0.0;
	currentDisp[POS_Z] = 0.0;
	/*currentDisp[ORI_X] = 0.0;
	currentDisp[ORI_Y] = 0.0;
	currentDisp[ORI_Z] = 0.0;
	currentDisp[ORI_W] = 0.0;*/

	//currentDisp[ORI_X] = (MASum[ORI_X] / MOVING_AVERAGE_WINDOW);
	//currentDisp[ORI_Y] = (MASum[ORI_Y] / MOVING_AVERAGE_WINDOW);
	//currentDisp[ORI_Z] = (MASum[ORI_Z] / MOVING_AVERAGE_WINDOW);
	//currentDisp[ORI_W] = (MASum[ORI_W] / MOVING_AVERAGE_WINDOW);
	//currentDisp[ORI_WX] = (MASum[ORI_WX] / MOVING_AVERAGE_WINDOW);
	//currentDisp[ORI_WY] = (MASum[ORI_WY] / MOVING_AVERAGE_WINDOW);
	//currentDisp[ORI_WZ] = (MASum[ORI_WZ] / MOVING_AVERAGE_WINDOW);
	//currentDisp[ORI_WW] = (MASum[ORI_WW] / MOVING_AVERAGE_WINDOW);

	//just use raw data for glove quaternions:
	currentDisp[ORI_X] = _oX;
	currentDisp[ORI_Y] = _oY;
	currentDisp[ORI_Z] = _oZ;
	currentDisp[ORI_W] = _oW;

	currentDisp[ORI_WX] = _oWX;
	currentDisp[ORI_WY] = _oWY;
	currentDisp[ORI_WZ] = _oWZ;
	currentDisp[ORI_WW] = _oWW;



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
		<< "," << dt.currentDisp[ORI_W] << "," << dt.handState << "|" << dt.currentDisp[ORI_WX] << "," << dt.currentDisp[ORI_WY] << ","
		<< dt.currentDisp[ORI_WZ] << "," << dt.currentDisp[ORI_WW];
	if (dt.changedInit)
	{
		os << "|True";
	} 
    else
    {
        os << "|False";
    }

	return os;
}