/* no config code here. sorry. */

#define COUNTS_PER_REV 627.2
#define TBH_LOOP_SPEED 30 /* change this as necessary */

#define GAIN_NORMAL 0.001; /* TODO changing speeds */

long enc, enc_prev;

float motor_velocity;
long time;

long target_velocity;
float current_error, last_error;
float gain, gain_higher; /* integration coefficient constant */
float drive, drive_prev; /* float between 0 and 1, indicating percent of full power */
long first_cross;
float drive_approx;

long motor_drive;

void setMotorPower(int power) {
	/* TODO fix negatives as needed */
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

void setVelocity(int velocity, float predicted_drive, float gain_changing, float gain_steady) {
	/* update/reset stuff */
	// gain_changing = gain while velocity is changing
	// gain_steady = gain while velocity should be constant
	// if gain parameter values are nonpositive they will default to GAIN_NORMAL
	target_velocity = velocity;
	current_error = target_velocity - motor_velocity;
	last_error = current_error;
	drive_approx = predicted_drive;
	first_cross = 1;
	drive_prev = 0;
	gain = (gain_changing > 0) ? gain_changing : GAIN_NORMAL;
	gain_higher = (gain_steady > 0) ? gain_higher : GAIN_NORMAL;
}

void calculateVelocity() {
	/* update global variables */
	int d_time, d_enc;
	enc = getEncoderCount();
	d_time = nSysTime - time;
	time = nSysTime;
	d_enc = (enc - enc_prev);
	enc_prev = enc;
	motor_velocity = (1000.0 / d_time) * d_enc * 60.0 / COUNTS_PER_REV;
	// print to debug stream
	writeDebugStreamLine("motor velocity = %1.3f", motor_velocity);
}

void updateVelocity() {

	/* calculate and constrain drive */
	current_error = target_velocity - motor_velocity;
	drive += current_error * gain;
	if (drive > 1) drive = 1;
	if (drive < 0) drive = 0;

	/* when crossing set point */
	if (sgn(current_error) != sgn(last_error)) {
		if (first_cross) {
			drive = drive_approx;
			first_cross = 0;
			if (target_velocity) gain = gain_higher;
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
	setVelocity(120, 0.3, 0, 0.035);
}
