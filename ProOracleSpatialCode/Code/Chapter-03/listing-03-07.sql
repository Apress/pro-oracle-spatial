-- Listing 3-7. Updating a location Column Using an SDO_GEOMETRY Constructor
UPDATE customers
SET location =
      SDO_GEOMETRY
      (
        2001,
        8307,
        SDO_POINT_TYPE(-77.06, 38.94, NULL),
        NULL,
        NULL
      )
WHERE id=1;
COMMIT;
