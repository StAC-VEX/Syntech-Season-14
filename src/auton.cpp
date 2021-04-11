#include "auton.h"

using namespace Syntech;

motor intakeMotors[] = {Devices::leftIntakeMotor, Devices::rightIntakeMotor};

void Syntech::turn(int degrees) {
	Devices::sensor.rotation();
	PI(2, 60, 1, degrees, []() { 
		return Devices::sensor.rotation(); 
	}, [](int speed) {
		Devices::leftMotor.spin(directionType::fwd, speed, velocityUnits::dps);
		Devices::rightMotor.spin(directionType::rev, speed, velocityUnits::dps);
	});
}

void Syntech::move(int distance) {
	//5 = wheelRadius
	Devices::leftMotor.resetRotation();
	PI(5, 60, 1, distance, []() { 
		return ((3.14159 * 5) / 360 * Devices::leftMotor.rotation(rotationUnits::deg)); 
	}, [](int speed) { 
		Devices::leftMotor.spin(directionType::fwd, speed, velocityUnits::dps);
		Devices::rightMotor.spin(directionType::fwd, speed, velocityUnits::dps);
	});
}

void Syntech::move(int time, int _dps) {
	Devices::leftMotor.spin(fwd, _dps, dps);
	Devices::rightMotor.spin(fwd, _dps, dps);
	wait(time, msec);
	Devices::leftMotor.stop(coast);    
	Devices::rightMotor.stop(coast);
}

void Syntech::intake(int time, int _dps) {
	for (motor _motor : intakeMotors) {
		_motor.spin(fwd, _dps, dps);      
	}

	wait(time, msec);

	for (motor _motor : intakeMotors) {
		_motor.stop(coast);
	}
}

void Syntech::startIntake(int _dps) {
	for (motor _motor : intakeMotors) {
		_motor.spin(fwd, _dps, dps);      
	}
}

void Syntech::stopIntake() {
	for (motor _motor : intakeMotors) {
		_motor.stop(coast);
	}
}

void Syntech::PI(int min, int max, float margin, int desired, double(*independent)(), void(*loop)(int error)) {
	Devices::leftMotor.resetRotation();
	Devices::rightMotor.resetRotation();

	double error = desired - independent();
	double previousError = 0;
	const double maxAcceleration = 0.03;
	while (!(independent() < desired + margin && independent() > desired - margin)) {
		// Gets the previous error
		error = desired - independent();
		double speed = error;

		if (fabs(error - previousError) > maxAcceleration) {
			//If accelerating
			if (speed - previousError > 0) speed = (previousError - maxAcceleration);
			else speed = previousError + maxAcceleration;
		}

		// Caps the error.
		if (speed > max) speed = max;
		if (speed < -max) speed = -max;
		if (speed < min && error > 0) speed = min;
		if (speed > -min && speed < 0) speed = -min;

		loop(speed);

		wait(20, msec);
		// How far the motor still needs to go.
		previousError = desired - error;
	}
}
