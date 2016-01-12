/* no config code here. sorry. */

/** hi there
okay so basically add/change/fix/update/test stuff wherever there are TODO's
line 31 -  you're going to have to test for the best value of gain
	just by incrementing it slowly. higher gain means the power
	changes faster, and the gain should not have to be greater than 1.
line 39 - this setup may or may not be right
line 48 - choose a motor with an encoder on it. also check the encoder box
	in the configuration. it only uses one motor from one side, which
	should work but if there are problems we can monitor a second motor.
line 112 - self explanatory? you can use setVelocity() each time a
	button is pressed.
line 114 - you're going to have to test for the optimal velocities.
	the first parameter into setVelocity() is the motor speed
	(which is / should be printed in the debug stream) and the second
	is the estimated power needed as a percent of full power.
**/

#define COUNTS_PER_REV 627.2
#define TBH_LOOP_SPEED 30 /* change this as necessary */

long enc, enc_prev;

float motor_velocity;
long time;

long target_velocity;
float current_error;
float last_error;
float gain = 0.00024; /* TODO this is the variable to fiddle around with */
float drive, drive_prev; /* float between 0 and 1, indicating percent of full power */
long first_cross;
float drive_approx;

long motor_drive;

void setMotorPower(int power) {
	/* TODO fix negatives as needed lol */
	// power is always positive
	motor[lt] = power;
	motor[rt] = power;
	motor[lb] = -1 * power;
	motor[rb] = -1 * power;
}

long getEncoderCount() {
	/* TODO change motor name as needed */
	return(nMotorEncoder[lt]);
}

void setVelocity(int velocity, float predicted_drive) {
	/* update/reset stuff */
	target_velocity = velocity;
	current_error = target_velocity - motor_velocity;
	last_error = current_error;
	drive_approx = predicted_drive;
	first_cross = 1;
	drive_prev = 0;
}

void calculateVelocity() {
	/* update global variables */
	int d_time;
	int d_enc;
	enc = getEncoderCount();
	d_time = nSysTime - time;
	time = nSysTime;
	d_enc = (enc - enc_prev);
	enc_prev = enc;
	motor_velocity = (1000.0 / d_ms) * d_enc * 60.0 / COUNTS_PER_REV;
	// print to debug stream
	writeDebugStreamLine("motor velocity = %1.3f", motor_velocity);
}

void updateVelocity() {

	/* calculate and constrain drive */
	current_error = target_velocity - motor_velocity;
	if (abs(current_error * gain) < 0.3)
		drive += current_error * gain;
	else
		drive += 0.3 * sgn(current_error * gain);
	if (drive > 1) drive = 1;
	if (drive < 0) drive = 0;

	/* when crossing set point */
	if (sgn(current_error) != sgn(last_error)) {
		if (first_cross) {
			drive = drive_approx;
			first_cross = 0;
		} else {
			/* the tbh part ! */
			drive = 0.5 * (drive + drive_prev);
		}
		drive_prev = drive;
	}

	last_error = current_error;
}

task tbhControl() {
	while (1) {
		calculateVelocity();
		updateVelocity();
		motor_drive = (drive * 127) + 0.5;
		setPower(motor_drive);
		wait1Msec(TBH_LOOP_SPEED);
	}
}

task main() {
	startTask(tbhControl());

	/* TODO add button stuff here */

	/* TODO random test case */
	setVelocity(120, 0.3);
}
