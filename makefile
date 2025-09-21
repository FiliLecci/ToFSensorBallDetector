linear : coordList.h linear_sensors.c
	gcc -Wall -g linear_sensors.c -fsanitize=address -lm -lSDL2 -o a.out

lidar: coordList.h lidar_sensor.c
	gcc -Wall -g lidar_sensor.c -fsanitize=address -lm -lSDL2 -o a.out

lidar_real: coordList.h lidar_sensor_noise_real.c
	gcc -Wall -g lidar_sensor_noise_real.c -fsanitize=address -lm -lSDL2 -o a.out

run : a.out
	./$<
