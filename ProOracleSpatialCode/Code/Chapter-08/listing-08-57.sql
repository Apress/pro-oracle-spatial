-- Listing 8-57. Creating a Local Partitioned Spatial Index with Partition-Specific Parameters
CREATE INDEX customers_spatial_idx ON customers(location)
  INDEXTYPE IS MDSYS.SPATIAL_INDEX
  PARAMETERS ('TABLESPACE=USERS')
  LOCAL
  (
    PARTITION IP1 PARAMETERS('TABLESPACE=TBS_3'),
    PARTITION IP2,
    PARTITION IP3
  );
