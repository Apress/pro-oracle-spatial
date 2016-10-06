-- Listing 14-5. Creating an Empty Table with the Same Structure As the us_streets Table
CREATE TABLE us_streets_dup AS SELECT * FROM us_streets;
TRUNCATE TABLE us_streets;
DROP INDEX us_streets_sidx;
