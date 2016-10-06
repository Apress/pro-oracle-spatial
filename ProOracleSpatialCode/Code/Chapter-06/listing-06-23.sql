-- Listing 6-23. Automatic Geocoding with Address Correction
CREATE OR REPLACE TRIGGER branches_geocode
  BEFORE INSERT OR UPDATE OF street_name, street_number, postal_code, city ON branches
  FOR EACH ROW

DECLARE
  geo_location SDO_GEOMETRY;
  geo_addresses SDO_ADDR_ARRAY;
  geo_address SDO_GEO_ADDR;
  update_address BOOLEAN;

BEGIN
  -- Geocode the address
  geo_addresses := sdo_gcdr.geocode_all (
    'SPATIAL',
    SDO_KEYWORDARRAY (
      :new.street_number || ' ' || :new.street_name,
      :new.city  || ' ' || :new.postal_code),
    'US',
    'DEFAULT'
  );

  -- Check results
  if geo_addresses.count() > 1 then
    -- Address is ambiguous: reject
    geo_location := NULL;
  else
    -- Extract first or only match
    geo_address := geo_addresses(1);
    -- The following matchcodes are accepted:
    --   1 = exact match
    --   2 = only street type or suffix/prefix is incorrect
    --  10 = only postal code is incorrect
    if geo_address.matchcode in (1,2,10) then
      -- Geocoding succeeded: construct geometric point
      geo_location := sdo_geometry (2001, 8307, sdo_point_type (
        geo_address.longitude, geo_address.latitude, null),
        null, null);
      -- If wrong street type or postal code (matchcodes 2 or 10)
      -- accept the geocode and correct the address in the database
      if geo_address.matchcode <> 1 then
        update_address := true;
      end if;
    else
      -- For all other matchcoded, reject the geocode
      geo_location := NULL;
    end if;
  end if;

  -- Update loaction
  :new.location := geo_location;
  -- If needed, correct address
  :new.street_name := geo_address.streetname;
  :new.postal_code := geo_address.postalcode;

END;
/
