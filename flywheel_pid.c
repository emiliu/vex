/* no config code here. sorry. */

#define COUNTS_PER_REV 627.2
#define PID_LOOP_SPEED 30 /* change this as necessary */

float kP, kI, kD; /* the PID constants */
float integral, derivative;
float current_error, last_error;

long enc, enc_prev;
float motor_velocity;
long time;

long target_velocity;
float drive; /* float between 0 and 1, indicating percent of full power */
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

void setVelocity(int velocity, float predicted_drive, float P_gain, float I_gain, float D_gain) {
	/* update/reset stuff */
	target_velocity = velocity;
	current_error = target_velocity - motor_velocity;
	last_error = current_error;
	drive_approx = predicted_drive;
	first_cross = 1;
	drive_prev = 0;
	integral = 0;
	derivative = 0;
	kP = P_gain;
	kI = I_gain;
	kD = D_gain;
	if (!(P_gain||I_gain||D_gain)) P_gain = 0.001;
}

void updateVelocity() {

	int d_time, d_enc;
	enc = getEncoderCount();
	d_time = nSysTime - time;
	time = nSysTime;
	d_enc = (enc - enc_prev);
	enc_prev = enc;
	motor_velocity = (1000.0 / d_time) * d_enc * 60.0 / COUNTS_PER_REV;

	current_error = target_velocity - motor_velocity;
	integral += current_error * d_time;
	derivative = (current_error - last_error) / d_time;

	drive += kP * current_error + kI * integral + kD * derivative;
	if (drive > 1) drive = 1;
	if (drive < 0) drive = 0;

	if (first_cross) {
		if (current_error > 0)
			drive = 1;
		else if (sgn(current_error) != sgn(last_error)) {
			drive = drive_approx;
			first_cross = 0;
			integral = 0;
	}

	last_error = current_error;
}

task pidControl() {
	while (1) {
		updateVelocity();
		motor_drive = (drive * 127) + 0.5;
		setPower(motor_drive);
		wait1Msec(PID_LOOP_SPEED);
	}
}

task main() {
	startTask(pidControl());

	/* TODO add button stuff here */

	/* TODO random test case */
	setVelocity(120, 0.6, 0.1, 0.1, 0.1);
}
