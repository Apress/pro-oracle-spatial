-- Listing 8-50. Declaring the gcdr_geometry Function As DETERMINISTIC
SELECT gcdr_geometry(street_number,street_name,city,state,postal_code).sdo_point.x x,
       gcdr_geometry(street_number,street_name,city,state,postal_code).sdo_point.y y
  FROM customers WHERE id=1;
