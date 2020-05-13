#!/usr/bin/env sh

cd ./c/tutorial && make && cd ./build
echo -en "cex 1 me\n" | nc -w1 -u $CAN_INTERFACE 2000

./change_servo_id $CAN_INTERFACE:$CAN_PORT $MOTOR_ID $MOTOR_ID 2>&1 | tee change_servo_id.log
./time_optimal_movement $CAN_INTERFACE:$CAN_PORT $MOTOR_ID 100 100 100 2>&1 | tee time_optimal_movement.log
./control_servo_traj_1 $CAN_INTERFACE:$CAN_PORT $MOTOR_ID 2>&1 | tee control_servo_traj_1.log
./control_servo_traj_2 $CAN_INTERFACE:$CAN_PORT $MOTOR_ID $MOTOR_ID 2>&1 | tee control_servo_traj_2.log
./control_servo_traj_3 $CAN_INTERFACE:$CAN_PORT $MOTOR_ID $MOTOR_ID $MOTOR_ID 2>&1 | tee control_servo_traj_3.log
./read_any_param $CAN_INTERFACE:$CAN_PORT $MOTOR_ID 2>&1 | tee read_any_param.log
./read_any_param_cache $CAN_INTERFACE:$CAN_PORT $MOTOR_ID 2>&1 | tee read_any_param_cache.log
./read_errors $CAN_INTERFACE:$CAN_PORT $MOTOR_ID 2>&1 | tee read_errors.log
./read_servo_max_velocity $CAN_INTERFACE:$CAN_PORT $MOTOR_ID 2>&1 | tee read_servo_max_velocity.log
./read_servo_motion_queue $CAN_INTERFACE:$CAN_PORT $MOTOR_ID 2>&1 | tee read_servo_motion_queue.log
./read_servo_trajectory_time $CAN_INTERFACE:$CAN_PORT $MOTOR_ID 2>&1 | tee read_servo_trajectory_time.log
./check_motion_points 2>&1 | tee check_motion_points.log
./hb_timings $CAN_INTERFACE:$CAN_PORT $MOTOR_ID  2>&1 | tee hb_timings.log
./read_emcy_log $CAN_INTERFACE:$CAN_PORT $MOTOR_ID 2>&1 | tee read_emcy_log.log
./calibrate_cogging $CAN_INTERFACE:$CAN_PORT $MOTOR_ID 2>&1 | tee calibrate_cogging.log
./calibrate_foc_offset $CAN_INTERFACE:$CAN_PORT $MOTOR_ID 2>&1 | tee calibrate_foc_offset.log
./calibration_quality $CAN_INTERFACE:$CAN_PORT $MOTOR_ID 2>&1 | tee calibration_quality.log
./discovery $CAN_INTERFACE:$CAN_PORT 2>&1 | tee discovery.log & sleep 10 ; kill -9 `pgrep discovery | tail -1`


if [ "$(grep 'ERROR:' *.log)" != "" ]; then 
	echo "FAILED"
	exit 1; 
else
	echo "PASS"
fi
