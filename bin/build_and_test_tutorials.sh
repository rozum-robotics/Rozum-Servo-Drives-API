#!/usr/bin/env sh

cd ./c/tutorial && make && cd ./build
echo -en "cex 1 me\n" | nc -w1 -u $CAN_INTERFACE 2000
TUTORIALS=`ls -1 2>/dev/null | wc -l`

./change_servo_id $CAN_INTERFACE:$CAN_PORT $MOTOR_ID $MOTOR_ID | tee change_servo_id.log
./time_optimal_movement $CAN_INTERFACE:$CAN_PORT $MOTOR_ID 100 100 100 | tee time_optimal_movement.log
./control_servo_traj_1 $CAN_INTERFACE:$CAN_PORT $MOTOR_ID | tee control_servo_traj_1.log
./control_servo_traj_2 $CAN_INTERFACE:$CAN_PORT $MOTOR_ID $MOTOR_ID | tee control_servo_traj_2.log
./control_servo_traj_3 $CAN_INTERFACE:$CAN_PORT $MOTOR_ID $MOTOR_ID $MOTOR_ID | tee control_servo_traj_3.log
./read_any_param $CAN_INTERFACE:$CAN_PORT $MOTOR_ID | tee read_any_param.log
./read_any_param_cache $CAN_INTERFACE:$CAN_PORT $MOTOR_ID | tee read_any_param_cache.log
./read_errors $CAN_INTERFACE:$CAN_PORT $MOTOR_ID | tee read_errors.log
./read_servo_max_velocity $CAN_INTERFACE:$CAN_PORT $MOTOR_ID | tee read_servo_max_velocity.log
./read_servo_motion_queue $CAN_INTERFACE:$CAN_PORT $MOTOR_ID | tee read_servo_motion_queue.log
./read_servo_trajectory_time $CAN_INTERFACE:$CAN_PORT $MOTOR_ID | tee read_servo_trajectory_time.log
./check_motion_points | tee check_motion_points.log
./hb_timings $CAN_INTERFACE:$CAN_PORT $MOTOR_ID  | tee hb_timings.log
./read_emcy_log $CAN_INTERFACE:$CAN_PORT $MOTOR_ID | tee read_emcy_log.log
./calibrate_cogging $CAN_INTERFACE:$CAN_PORT $MOTOR_ID | tee calibrate_cogging.log
./calibrate_foc_offset $CAN_INTERFACE:$CAN_PORT $MOTOR_ID | tee calibrate_foc_offset.log
./calibration_quality $CAN_INTERFACE:$CAN_PORT $MOTOR_ID | tee calibration_quality.log
./discovery $CAN_INTERFACE:$CAN_PORT | tee discovery.log & sleep 10 ; kill -9 `pgrep discovery | tail -1`

LOGS=`ls -1 *.log 2>/dev/null | wc -l`

if [ "$(grep 'ERROR:' *.log)" != "" ]; then exit 1; fi
if [ $TUTORIALS -ne $LOGS ]; then exit 1; fi
