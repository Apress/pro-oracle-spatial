-- Listing 8-11. Creating an Index with WORK_TABLESPACE As TBS_3
CREATE INDEX customers_sidx ON customers(location)
  INDEXTYPE IS MDSYS.SPATIAL_INDEX
  PARAMETERS ('WORK_TABLESPACE=TBS_3');
