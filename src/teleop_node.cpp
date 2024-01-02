#include <string>
#include "ros/ros.h"
#include "ros/console.h"
#include <SDL2/SDL.h>
#include "common_teleop/JoystickState.h"

using namespace std;

int main(int argc, char **argv) {
	ros::init(argc, argv, "common_teleop", ros::init_options::AnonymousName);
	ros::NodeHandle ros_NodeHandle;

	if(SDL_Init(SDL_INIT_JOYSTICK) < 0){
		ROS_ERROR("Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	};

	int numberJoysticks = SDL_NumJoysticks();
	ROS_INFO("%d Joystick(s) were found.", numberJoysticks);

	ros::Publisher joystickStatePublishers[numberJoysticks];
	common_teleop::JoystickState joystickStates[numberJoysticks];
	SDL_Joystick* sdlJoysticks[numberJoysticks];

	const char* joystickNames[numberJoysticks];
	for(int i = 0; i < numberJoysticks; i++){
		sdlJoysticks[i] = SDL_JoystickOpen(i);
		joystickStates[i].joystickName = string(SDL_JoystickNameForIndex(i));
		joystickStates[i].joystickNumButtons = SDL_JoystickNumButtons(sdlJoysticks[i]);
		joystickStates[i].joystickNumAxes = SDL_JoystickNumAxes(sdlJoysticks[i]);
		joystickStates[i].joystickNumHats = SDL_JoystickNumHats(sdlJoysticks[i]);
		
		joystickStates[i].Buttons.resize(joystickStates[i].joystickNumButtons);
		joystickStates[i].Axes.resize(joystickStates[i].joystickNumAxes);
		joystickStates[i].Hats.resize(joystickStates[i].joystickNumHats);

		joystickStatePublishers[i] = ros_NodeHandle.advertise<common_teleop::JoystickState>("/ControlDevices/Joystick" + to_string(i), 1);
	};

	int i(0), j(0);
	SDL_Event joystickEvent;
	ros::Rate r(20);
	while(ros::ok()){
		if(SDL_PollEvent(&joystickEvent)){
			if (SDL_QUIT == joystickEvent.type) { break; }
		}
		if (numberJoysticks != SDL_NumJoysticks()) { break; }

		for(i = 0; i < numberJoysticks; i++){
			for(j = 0; j < joystickStates[i].joystickNumButtons; j++){
				joystickStates[i].Buttons[j] = SDL_JoystickGetButton(sdlJoysticks[i], j);
			}

			for(j = 0; j < joystickStates[i].joystickNumAxes; j++){
				joystickStates[i].Axes[j] = SDL_JoystickGetAxis(sdlJoysticks[i], j) / 32768.0;
			}

			for(j = 0; j < joystickStates[i].joystickNumHats; j++) {
				joystickStates[i].Hats[j] = SDL_JoystickGetHat(sdlJoysticks[i], j);
			}

			joystickStatePublishers[i].publish(joystickStates[i]);
		}
		r.sleep();
	}


	SDL_Quit();
	return 0;
}
