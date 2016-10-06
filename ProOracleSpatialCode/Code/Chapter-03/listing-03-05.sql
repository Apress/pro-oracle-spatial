-- Listing 3-5. Geocoding Addresses to Obtain Explicit Spatial Information
UPDATE customers
SET location =
      sdo_gcdr.geocode_as_geometry
      (
       'SPATIAL',
       sdo_keywordarray
       (
         street_number || ' ' || street_name,
         city  || ', ' || state || ' '  || postal_code
       ),
       'US'
      );
COMMIT;
