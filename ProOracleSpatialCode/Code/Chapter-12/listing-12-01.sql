-- Listing 12-1. Loading the Geographical Data
$imp spatial/spatial file=map_large.dmp full=y
$imp spatial/spatial file=map_detailed.dmp full=y
$imp spatial/spatial file=net.dmp full=y
$imp spatial/spatial file=gc.dmp full=y
INSERT INTO USER_SDO_NETWORK_METADATA
  SELECT * FROM my_network_metadata;
COMMIT;
