
CREATE TABLE IF NOT EXISTS scenario_executions (
  id UUID NOT NULL DEFAULT gen_random_uuid(),
  time_start TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT CURRENT_TIMESTAMP,
  time_end TIMESTAMP WITH TIME ZONE,
  name TEXT,
  PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS drones_position (
  scenario_id UUID NOT NULL REFERENCES scenario_executions (id),
  time_real TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT CURRENT_TIMESTAMP,
  time_sim double PRECISION NOT NULL,
  drone_id int NOT NULL,
  pos_x double PRECISION NOT NULL,
  pos_y double PRECISION NOT NULL,
  pos_z double PRECISION NOT NULL,
  vel_x double PRECISION NOT NULL,
  vel_y double PRECISION NOT NULL,
  vel_z double PRECISION NOT NULL,
  PRIMARY KEY (scenario_id, time_real)
);
SELECT create_hypertable('drones_position', 'time_real');

CREATE TABLE IF NOT EXISTS wifi_rssi (
  scenario_id UUID NOT NULL REFERENCES scenario_executions (id),
  time_real TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT CURRENT_TIMESTAMP,
  time_sim double PRECISION NOT NULL,
  drone_id int NOT NULL,
  from_addr char(17) NOT NULL, -- e.g. 00:00:00:00:00:00
  rssi double PRECISION NOT NULL,
  PRIMARY KEY (scenario_id, time_real)
);
SELECT create_hypertable('wifi_rssi', 'time_real');
