-- Listing 8-2. Creating an Index
CREATE INDEX customers_spatial_idx ON customers(location)
  INDEXTYPE IS MDSYS.SPATIAL_INDEX;
