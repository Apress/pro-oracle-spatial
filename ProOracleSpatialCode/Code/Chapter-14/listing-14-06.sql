-- Listing 14-6. Reinserting into the us_streets Table Based on the linear_key Order
INSERT INTO us_streets
  SELECT *
    FROM us_streets_dup a
   ORDER BY
     linear_key
     (
       a.location,
       (
         SELECT diminfo FROM USER_SDO_GEOM_METADATA
         WHERE table_name = 'US_STREETS' AND column_name='LOCATION'
       )
     );
