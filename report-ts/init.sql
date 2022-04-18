
CREATE TABLE IF NOT EXISTS drones_position (
  time_real TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT CURRENT_TIMESTAMP,
  time_sim double PRECISION NOT NULL,
  drone_id int NOT NULL,
  pos_x double PRECISION NOT NULL,
  pos_y double PRECISION NOT NULL,
  pos_z double PRECISION NOT NULL,
  vel_x double PRECISION NOT NULL,
  vel_y double PRECISION NOT NULL,
  vel_z double PRECISION NOT NULL
);

SELECT create_hypertable('drones_position', 'time_real');
