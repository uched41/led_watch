#include "nrf_delay.h"

#include "Invn/Devices/Drivers/Icm20648/Icm20648.h"
#include "Invn/Devices/Drivers/Icm20648/Icm20648MPUFifoControl.h"
#include "Invn/Devices/Drivers/Ak0991x/Ak0991x.h"
#include "Invn/Devices/SensorTypes.h"
#include "Invn/Devices/SensorConfig.h"
#include "Invn/EmbUtils/Message.h"
#include "Invn/EmbUtils/ErrorHelper.h"
#include "Invn/EmbUtils/dataconverter.h"

/* TDK Sensor */
#include "acc_sensor.h"

//#include "nrf_delay.h"
//#define delay_ms  nrf_delay_ms
//#define delay_us  nrf_delay_us

static const uint8_t dmp3_image[] = {
	#include "acc_img.dmp3a.h"
};

/*
* Just a handy variable to handle the icm20648 object
*/
inv_icm20648_t icm_device;

static const uint8_t EXPECTED_WHOAMI[] = { 0xE0 }; /* WHOAMI value for ICM20648 or derivative */

/* FSR configurations */
int32_t cfg_acc_fsr = 4; // Default = +/- 4g. Valid ranges: 2, 4, 8, 16
int32_t cfg_gyr_fsr = 2000; // Default = +/- 2000dps. Valid ranges: 250, 500, 1000, 2000

/*
* Mounting matrix configuration applied for both Accel and Gyro
*/
static const float cfg_mounting_matrix[9]= {
	1.f, 0, 0,
	0, 1.f, 0,
	0, 0, -1.f
};

/*
* Mounting matrix configuration applied for Mag
*/
static const float cfg_mag_mounting_matrix[9]= {
	1.f, 0, 0,
	0, 1.f, 0,
	0, 0, 1.f
};

static uint8_t convert_to_generic_ids[INV_ICM20648_SENSOR_MAX] = {
	INV_SENSOR_TYPE_ACCELEROMETER,
	INV_SENSOR_TYPE_GYROSCOPE,
	INV_SENSOR_TYPE_RAW_ACCELEROMETER,
	INV_SENSOR_TYPE_RAW_GYROSCOPE,
	INV_SENSOR_TYPE_UNCAL_MAGNETOMETER,
	INV_SENSOR_TYPE_UNCAL_GYROSCOPE,
	INV_SENSOR_TYPE_BAC,
	INV_SENSOR_TYPE_STEP_DETECTOR,
	INV_SENSOR_TYPE_STEP_COUNTER,
	INV_SENSOR_TYPE_GAME_ROTATION_VECTOR,
	INV_SENSOR_TYPE_ROTATION_VECTOR,
	INV_SENSOR_TYPE_GEOMAG_ROTATION_VECTOR,
	INV_SENSOR_TYPE_MAGNETOMETER,
	INV_SENSOR_TYPE_SMD,
	INV_SENSOR_TYPE_PICK_UP_GESTURE,
	INV_SENSOR_TYPE_TILT_DETECTOR,
	INV_SENSOR_TYPE_GRAVITY,
	INV_SENSOR_TYPE_LINEAR_ACCELERATION,
	INV_SENSOR_TYPE_ORIENTATION,
	INV_SENSOR_TYPE_B2S
};

/*
* Mask to keep track of enabled sensors
*/
static uint32_t enabled_sensor_mask = 0;

int ak09912_is_available = 0;

static enum inv_icm20648_sensor idd_sensortype_conversion(int sensor);
static void icm20648_apply_mounting_matrix(void);
static void icm20648_set_fsr(void);
static uint8_t icm20648_get_grv_accuracy(void);

/*
* Sleep implementation for ICM20648
*/
void inv_icm20648_sleep(int ms) {
	nrf_delay_ms(ms);
}

void inv_icm20648_sleep_us(int us){
	nrf_delay_us(us);
}

int load_dmp3(void){
	int rc = 0;
	INV_MSG(INV_MSG_LEVEL_INFO, "Load DMP3 image");
	rc = inv_icm20648_load(&icm_device, dmp3_image, sizeof(dmp3_image));
	return rc;
}

static void icm20648_apply_mounting_matrix(void){
	int ii;

	for (ii = 0; ii < INV_ICM20648_SENSOR_MAX; ii++) {
		if (ii == INV_ICM20648_SENSOR_MAGNETIC_FIELD_UNCALIBRATED || ii == INV_ICM20648_SENSOR_GEOMAGNETIC_FIELD)
			inv_icm20648_set_matrix(&icm_device, cfg_mag_mounting_matrix, ii);
		else
			inv_icm20648_set_matrix(&icm_device, cfg_mounting_matrix, ii);
	}
}

static void icm20648_set_fsr(void){
	inv_icm20648_set_fsr(&icm_device, INV_ICM20648_SENSOR_RAW_ACCELEROMETER, (const void *)&cfg_acc_fsr);
	inv_icm20648_set_fsr(&icm_device, INV_ICM20648_SENSOR_ACCELEROMETER, (const void *)&cfg_acc_fsr);
	inv_icm20648_set_fsr(&icm_device, INV_ICM20648_SENSOR_RAW_GYROSCOPE, (const void *)&cfg_gyr_fsr);
	inv_icm20648_set_fsr(&icm_device, INV_ICM20648_SENSOR_GYROSCOPE, (const void *)&cfg_gyr_fsr);
	inv_icm20648_set_fsr(&icm_device, INV_ICM20648_SENSOR_GYROSCOPE_UNCALIBRATED, (const void *)&cfg_gyr_fsr);
}

int icm20648_sensor_setup(void){
	int rc;
	uint8_t i, whoami = 0xff;

	/*
	* Just get the whoami
	*/
	rc = inv_icm20648_get_whoami(&icm_device, &whoami);
	INV_MSG(INV_MSG_LEVEL_INFO, "ICM20648 WHOAMI=0x%02x", whoami);
	/*
	* Check if WHOAMI value corresponds to any value from EXPECTED_WHOAMI array
	*/
	for(i = 0; i < sizeof(EXPECTED_WHOAMI)/sizeof(EXPECTED_WHOAMI[0]); ++i) {
		if(whoami == EXPECTED_WHOAMI[i])
			break;
	}

	if(i == sizeof(EXPECTED_WHOAMI)/sizeof(EXPECTED_WHOAMI[0])) {
		INV_MSG(INV_MSG_LEVEL_ERROR, "Bad WHOAMI value. Got 0x%02x.", whoami);
		return rc;
	}

	/* Setup accel and gyro mounting matrix and associated angle for current board */
	inv_icm20648_init_matrix(&icm_device);

	/* set default power mode */
	INV_MSG(INV_MSG_LEVEL_VERBOSE, "Putting Icm20648 in sleep mode...");
	rc = inv_icm20648_initialize(&icm_device, dmp3_image, sizeof(dmp3_image));
	if (rc != 0) {
		INV_MSG(INV_MSG_LEVEL_ERROR, "Initialization failed. Error loading DMP3...");
		return rc;
	}
	/*
	* Configure and initialize the ICM20648 for normal use
	*/
	INV_MSG(INV_MSG_LEVEL_INFO, "Booting up icm20648...");

#if COMPASS_IS_AK9912
	/* Initialize auxiliary sensors */
	inv_icm20648_register_aux_compass(&icm_device, INV_ICM20648_COMPASS_ID_AK09912, AK0991x_DEFAULT_I2C_ADDR);
	ak09912_is_available = (inv_icm20648_initialize_auxiliary(&icm_device) == -1) ? 0 : 1;
	if (ak09912_is_available == 0) {
		inv_icm20648_register_aux_compass(&icm_device, INV_ICM20648_COMPASS_ID_AK09912, AK0991x_SECONDARY_I2C_ADDR);
		ak09912_is_available = (inv_icm20648_initialize_auxiliary(&icm_device) == -1) ? 0 : 1;
	}
#endif
	icm20648_apply_mounting_matrix();
	icm20648_set_fsr();

	/* re-initialize base state structure */
	inv_icm20648_init_structure(&icm_device);

	/* we should be good to go ! */
	INV_MSG(INV_MSG_LEVEL_VERBOSE, "We're good to go !");

	return 0;
}

/*
* Helper function to check RC value and block program execution
*/
void check_rc(int rc, const char * msg_context){
	if(rc < 0) {
		INV_MSG(INV_MSG_LEVEL_ERROR, "%s: error %d (%s)", msg_context, rc, inv_error_str(rc));
		while(1);
	}
}


static uint8_t icm20648_get_grv_accuracy(void){
	uint8_t accel_accuracy;
	uint8_t gyro_accuracy;

	accel_accuracy = (uint8_t)inv_icm20648_get_accel_accuracy();
	gyro_accuracy = (uint8_t)inv_icm20648_get_gyro_accuracy();
	return (min(accel_accuracy, gyro_accuracy));
}

void build_sensor_event_data(void * context, uint8_t sensortype, uint64_t timestamp, const void * data, const void *arg){
	float raw_bias_data[6];
	inv_sensor_event_t event;
	(void)context;
	uint8_t sensor_id = convert_to_generic_ids[sensortype];

	memset((void *)&event, 0, sizeof(event));
	event.sensor = sensor_id;
	event.timestamp = timestamp;
	switch(sensor_id) {
	case INV_SENSOR_TYPE_UNCAL_GYROSCOPE:
		memcpy(raw_bias_data, data, sizeof(raw_bias_data));
		memcpy(event.data.gyr.vect, &raw_bias_data[0], sizeof(event.data.gyr.vect));
		memcpy(event.data.gyr.bias, &raw_bias_data[3], sizeof(event.data.gyr.bias));
		memcpy(&(event.data.gyr.accuracy_flag), arg, sizeof(event.data.gyr.accuracy_flag));
		break;
	case INV_SENSOR_TYPE_UNCAL_MAGNETOMETER:
		memcpy(raw_bias_data, data, sizeof(raw_bias_data));
		memcpy(event.data.mag.vect, &raw_bias_data[0], sizeof(event.data.mag.vect));
		memcpy(event.data.mag.bias, &raw_bias_data[3], sizeof(event.data.mag.bias));
		memcpy(&(event.data.gyr.accuracy_flag), arg, sizeof(event.data.gyr.accuracy_flag));
		break;
	case INV_SENSOR_TYPE_GYROSCOPE:
		memcpy(event.data.gyr.vect, data, sizeof(event.data.gyr.vect));
		memcpy(&(event.data.gyr.accuracy_flag), arg, sizeof(event.data.gyr.accuracy_flag));
		break;
	case INV_SENSOR_TYPE_GRAVITY:
		memcpy(event.data.acc.vect, data, sizeof(event.data.acc.vect));
		event.data.acc.accuracy_flag = inv_icm20648_get_accel_accuracy();
		break;
	case INV_SENSOR_TYPE_LINEAR_ACCELERATION:
	case INV_SENSOR_TYPE_ACCELEROMETER:
		memcpy(event.data.acc.vect, data, sizeof(event.data.acc.vect));
		memcpy(&(event.data.acc.accuracy_flag), arg, sizeof(event.data.acc.accuracy_flag));
		break;
	case INV_SENSOR_TYPE_MAGNETOMETER:
		memcpy(event.data.mag.vect, data, sizeof(event.data.mag.vect));
		memcpy(&(event.data.mag.accuracy_flag), arg, sizeof(event.data.mag.accuracy_flag));
		break;
	case INV_SENSOR_TYPE_GEOMAG_ROTATION_VECTOR:
	case INV_SENSOR_TYPE_ROTATION_VECTOR:
		memcpy(&(event.data.quaternion.accuracy), arg, sizeof(event.data.quaternion.accuracy));
		memcpy(event.data.quaternion.quat, data, sizeof(event.data.quaternion.quat));
		break;
	case INV_SENSOR_TYPE_GAME_ROTATION_VECTOR:
		memcpy(event.data.quaternion.quat, data, sizeof(event.data.quaternion.quat));
		event.data.quaternion.accuracy_flag = icm20648_get_grv_accuracy();
		break;
	case INV_SENSOR_TYPE_BAC:
		memcpy(&(event.data.bac.event), data, sizeof(event.data.bac.event));
		break;
	case INV_SENSOR_TYPE_PICK_UP_GESTURE:
	case INV_SENSOR_TYPE_TILT_DETECTOR:
	case INV_SENSOR_TYPE_STEP_DETECTOR:
	case INV_SENSOR_TYPE_SMD:
		event.data.event = true;
		break;
	case INV_SENSOR_TYPE_B2S:
		event.data.event = true;
		memcpy(&(event.data.b2s.direction), data, sizeof(event.data.b2s.direction));
		break;
	case INV_SENSOR_TYPE_STEP_COUNTER:
		memcpy(&(event.data.step.count), data, sizeof(event.data.step.count));
		break;
	case INV_SENSOR_TYPE_ORIENTATION:
		//we just want to copy x,y,z from orientation data
		memcpy(&(event.data.orientation), data, 3*sizeof(float));
		break;
	case INV_SENSOR_TYPE_RAW_ACCELEROMETER:
	case INV_SENSOR_TYPE_RAW_GYROSCOPE:
		memcpy(event.data.raw3d.vect, data, sizeof(event.data.raw3d.vect));
		break;
	default:
		return;
	}
	sensor_event(&event, NULL);
}

static enum inv_icm20648_sensor idd_sensortype_conversion(int sensor){
	switch(sensor) {
	case INV_SENSOR_TYPE_RAW_ACCELEROMETER:       return INV_ICM20648_SENSOR_RAW_ACCELEROMETER;
	case INV_SENSOR_TYPE_RAW_GYROSCOPE:           return INV_ICM20648_SENSOR_RAW_GYROSCOPE;
	case INV_SENSOR_TYPE_ACCELEROMETER:           return INV_ICM20648_SENSOR_ACCELEROMETER;
	case INV_SENSOR_TYPE_GYROSCOPE:               return INV_ICM20648_SENSOR_GYROSCOPE;
	case INV_SENSOR_TYPE_UNCAL_MAGNETOMETER:      return INV_ICM20648_SENSOR_MAGNETIC_FIELD_UNCALIBRATED;
	case INV_SENSOR_TYPE_UNCAL_GYROSCOPE:         return INV_ICM20648_SENSOR_GYROSCOPE_UNCALIBRATED;
	case INV_SENSOR_TYPE_BAC:                     return INV_ICM20648_SENSOR_ACTIVITY_CLASSIFICATON;
	case INV_SENSOR_TYPE_STEP_DETECTOR:           return INV_ICM20648_SENSOR_STEP_DETECTOR;
	case INV_SENSOR_TYPE_STEP_COUNTER:            return INV_ICM20648_SENSOR_STEP_COUNTER;
	case INV_SENSOR_TYPE_GAME_ROTATION_VECTOR:    return INV_ICM20648_SENSOR_GAME_ROTATION_VECTOR;
	case INV_SENSOR_TYPE_ROTATION_VECTOR:         return INV_ICM20648_SENSOR_ROTATION_VECTOR;
	case INV_SENSOR_TYPE_GEOMAG_ROTATION_VECTOR:  return INV_ICM20648_SENSOR_GEOMAGNETIC_ROTATION_VECTOR;
	case INV_SENSOR_TYPE_MAGNETOMETER:            return INV_ICM20648_SENSOR_GEOMAGNETIC_FIELD;
	case INV_SENSOR_TYPE_SMD:                     return INV_ICM20648_SENSOR_WAKEUP_SIGNIFICANT_MOTION;
	case INV_SENSOR_TYPE_PICK_UP_GESTURE:         return INV_ICM20648_SENSOR_FLIP_PICKUP;
	case INV_SENSOR_TYPE_TILT_DETECTOR:           return INV_ICM20648_SENSOR_WAKEUP_TILT_DETECTOR;
	case INV_SENSOR_TYPE_GRAVITY:                 return INV_ICM20648_SENSOR_GRAVITY;
	case INV_SENSOR_TYPE_LINEAR_ACCELERATION:     return INV_ICM20648_SENSOR_LINEAR_ACCELERATION;
	case INV_SENSOR_TYPE_ORIENTATION:             return INV_ICM20648_SENSOR_ORIENTATION;
	case INV_SENSOR_TYPE_B2S:                     return INV_ICM20648_SENSOR_B2S;
	default:                                      return INV_ICM20648_SENSOR_MAX;
	}
}

void inv_icm20648_get_st_bias(struct inv_icm20648 * s, int *gyro_bias, int *accel_bias, int * st_bias){
	int axis, axis_sign;
	int gravity;
	int i, t;
	int check;
	int scale;

	/* check bias there ? */
	check = 0;
	for (i = 0; i < 3; i++) {
		if (gyro_bias[i] != 0)
			check = 1;
		if (accel_bias[i] != 0)
			check = 1;
	}

	/* if no bias, return all 0 */
	if (check == 0) {
		for (i = 0; i < 12; i++)
			st_bias[i] = 0;
		return;
	}

	/* dps scaled by 2^16 */
	scale = 65536 / DEF_SELFTEST_GYRO_SENS;

	/* Gyro normal mode */
	t = 0;
	for (i = 0; i < 3; i++)
		st_bias[i + t] = gyro_bias[i] * scale;

	axis = 0;
	axis_sign = 1;
	if (INV20648_ABS(accel_bias[1]) > INV20648_ABS(accel_bias[0]))
		axis = 1;
	if (INV20648_ABS(accel_bias[2]) > INV20648_ABS(accel_bias[axis]))
		axis = 2;
	if (accel_bias[axis] < 0)
		axis_sign = -1;

	/* gee scaled by 2^16 */
	scale = 65536 / (DEF_ST_SCALE / (DEF_ST_ACCEL_FS_MG / 1000));

	gravity = 32768 / (DEF_ST_ACCEL_FS_MG / 1000) * axis_sign;
	gravity *= scale;

	/* Accel normal mode */
	t += 3;
	for (i = 0; i < 3; i++) {
		st_bias[i + t] = accel_bias[i] * scale;
		if (axis == i)
			st_bias[i + t] -= gravity;
	}
}

int icm20648_run_selftest(void){
	static int rc = 0;		// Keep this value as we're only going to do this once.
	int gyro_bias_regular[THREE_AXES];
	int accel_bias_regular[THREE_AXES];
	static int raw_bias[THREE_AXES * 2];

	if (icm_device.selftest_done == 1) {
		INV_MSG(INV_MSG_LEVEL_INFO, "Self-test has already run. Skipping.");
	}
	else {
		/*
		* Perform self-test
		* For ICM20648 self-test is performed for both RAW_ACC/RAW_GYR
		*/
		INV_MSG(INV_MSG_LEVEL_INFO, "Running self-test...");

		/* Run the self-test */
		rc = inv_icm20648_run_selftest(&icm_device, gyro_bias_regular, accel_bias_regular);
		if (ak09912_is_available && ((rc & INV_ICM20648_SELF_TEST_OK) == INV_ICM20648_SELF_TEST_OK)) {
			/* On A+G+M success, offset will be kept until reset */
			icm_device.selftest_done = 1;
			rc = 0;
		} else if (!ak09912_is_available && ((rc & INV_ICM20648_SELF_TEST_AG_OK) == INV_ICM20648_SELF_TEST_AG_OK)) {
			/* On A+G success, offset will be kept until reset */
			icm_device.selftest_done = 1;
			rc = 0;
		} else {
			/* On A | G | M Selftest failure, return Error */
			INV_MSG(INV_MSG_LEVEL_ERROR, "Self-test failure !");
			/* 0 would be considered OK, we want KO */
			rc = INV_ERROR;
		}

		/* It's advised to re-init the icm20648 device after self-test for normal use */
		icm20648_sensor_setup();
		inv_icm20648_get_st_bias(&icm_device, gyro_bias_regular, accel_bias_regular, raw_bias);

		INV_MSG(INV_MSG_LEVEL_INFO, "GYR bias (FS=250dps) (dps): x=%f, y=%f, z=%f", (float)(raw_bias[0] / (float)(1 << 16)), (float)(raw_bias[1] / (float)(1 << 16)), (float)(raw_bias[2] / (float)(1 << 16)));
		INV_MSG(INV_MSG_LEVEL_INFO, "ACC bias (FS=2g) (g): x=%f, y=%f, z=%f", (float)(raw_bias[0 + 3] / (float)(1 << 16)), (float)(raw_bias[1 + 3] / (float)(1 << 16)), (float)(raw_bias[2 + 3] / (float)(1 << 16)));
	}

	return rc;
}
