-- Listing D-13. Creating an Index on the Spatial Extent of the georaster Column
CREATE INDEX geor_idx ON branches(georaster.spatialextent)
  INDEXTYPE IS MDSYS.SPATIAL_INDEX;
