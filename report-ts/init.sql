
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

CREATE TABLE IF NOT EXISTS lte_current_cell_rsrp_sinr (
  scenario_id UUID NOT NULL REFERENCES scenario_executions (id),
  time_real TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT CURRENT_TIMESTAMP,
  time_sim double PRECISION NOT NULL,
  drone_id int NOT NULL,
  cell_id int NOT NULL,
  rnti int NOT NULL,
  rsrp double PRECISION NOT NULL,
  sinr double PRECISION NOT NULL,
  component_carrier_id int NOT NULL,
  PRIMARY KEY (scenario_id, time_real)
);
SELECT create_hypertable('lte_current_cell_rsrp_sinr', 'time_real');

CREATE TABLE IF NOT EXISTS lte_ul_phy_resource_blocks (
  scenario_id UUID NOT NULL REFERENCES scenario_executions (id),
  time_real TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT CURRENT_TIMESTAMP,
  time_sim double PRECISION NOT NULL, 
  drone_id int NOT NULL,
  rnti int NOT NULL,
  rbs int ARRAY NOT NULL,
  PRIMARY KEY (scenario_id, time_real)
);
SELECT create_hypertable('lte_ul_phy_resource_blocks', 'time_real');

CREATE TABLE IF NOT EXISTS lte_power_spectral_density (
  scenario_id UUID NOT NULL REFERENCES scenario_executions (id),
  time_real TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT CURRENT_TIMESTAMP,
  time_sim double PRECISION NOT NULL,
  drone_id int NOT NULL,
  rnti int NOT NULL,
  psd double PRECISION ARRAY NOT NULL,
  PRIMARY KEY (scenario_id, time_real)
);
SELECT create_hypertable('lte_power_spectral_density', 'time_real');

CREATE TABLE IF NOT EXISTS lte_ue_measurements (
  scenario_id UUID NOT NULL REFERENCES scenario_executions (id),
  time_real TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT CURRENT_TIMESTAMP,
  time_sim double PRECISION NOT NULL,
  drone_id int NOT NULL,
  rnti int NOT NULL,
  cell_id int NOT NULL,
  rsrp double PRECISION NOT NULL,
  rsrq double PRECISION NOT NULL,
  is_serving_cell boolean NOT NULL,
  component_carrier_id int NOT NULL,
  PRIMARY KEY (scenario_id, time_real)
);
SELECT create_hypertable('lte_ue_measurements', 'time_real');
