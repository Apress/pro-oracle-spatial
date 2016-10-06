-- Listing 6-7. FORMAT_GEO_ADDR Procedure
CREATE OR REPLACE PROCEDURE format_geo_addr (
  address SDO_GEO_ADDR
)
AS
BEGIN
  dbms_output.put_line ('- ID                     ' || address.ID);
  dbms_output.put_line ('- ADDRESSLINES');
  if address.addresslines.count() > 0 then
    for i in 1..address.addresslines.count() loop
      dbms_output.put_line ('- ADDRESSLINES['||i||']           ' || address.ADDRESSLINES(i));
    end loop;
  end if;
  dbms_output.put_line ('- PLACENAME              ' || address.PLACENAME);
  dbms_output.put_line ('- STREETNAME             ' || address.STREETNAME);
  dbms_output.put_line ('- INTERSECTSTREET        ' || address.INTERSECTSTREET);
  dbms_output.put_line ('- SECUNIT                ' || address.SECUNIT);
  dbms_output.put_line ('- SETTLEMENT             ' || address.SETTLEMENT);
  dbms_output.put_line ('- MUNICIPALITY           ' || address.MUNICIPALITY);
  dbms_output.put_line ('- REGION                 ' || address.REGION);
  dbms_output.put_line ('- COUNTRY                ' || address.COUNTRY);
  dbms_output.put_line ('- POSTALCODE             ' || address.POSTALCODE);
  dbms_output.put_line ('- POSTALADDONCODE        ' || address.POSTALADDONCODE);
  dbms_output.put_line ('- FULLPOSTALCODE         ' || address.FULLPOSTALCODE);
  dbms_output.put_line ('- POBOX                  ' || address.POBOX);
  dbms_output.put_line ('- HOUSENUMBER            ' || address.HOUSENUMBER);
  dbms_output.put_line ('- BASENAME               ' || address.BASENAME);
  dbms_output.put_line ('- STREETTYPE             ' || address.STREETTYPE);
  dbms_output.put_line ('- STREETTYPEBEFORE       ' || address.STREETTYPEBEFORE);
  dbms_output.put_line ('- STREETTYPEATTACHED     ' || address.STREETTYPEATTACHED);
  dbms_output.put_line ('- STREETPREFIX           ' || address.STREETPREFIX);
  dbms_output.put_line ('- STREETSUFFIX           ' || address.STREETSUFFIX);
  dbms_output.put_line ('- SIDE                   ' || address.SIDE);
  dbms_output.put_line ('- PERCENT                ' || address.PERCENT);
  dbms_output.put_line ('- EDGEID                 ' || address.EDGEID);
  dbms_output.put_line ('- ERRORMESSAGE           ' || address.ERRORMESSAGE);
  if address.errormessage is not null and address.errormessage <> 'Not found' then
    if substr (address.errormessage,5,1)  <> '?' then dbms_output.put_line ('-   # House or building number'); end if;
    if substr (address.errormessage,6,1)  <> '?' then dbms_output.put_line ('-   E Street prefix'); end if;
    if substr (address.errormessage,7,1)  <> '?' then dbms_output.put_line ('-   N Street base name'); end if;
    if substr (address.errormessage,8,1)  <> '?' then dbms_output.put_line ('-   U Street suffix'); end if;
    if substr (address.errormessage,9,1)  <> '?' then dbms_output.put_line ('-   T Street type'); end if;
    if substr (address.errormessage,10,1) <> '?' then dbms_output.put_line ('-   S Secondary unit'); end if;
    if substr (address.errormessage,11,1) <> '?' then dbms_output.put_line ('-   B Built-up area or city'); end if;
    if substr (address.errormessage,14,1) <> '?' then dbms_output.put_line ('-   1 Region'); end if;
    if substr (address.errormessage,15,1) <> '?' then dbms_output.put_line ('-   C Country'); end if;
    if substr (address.errormessage,16,1) <> '?' then dbms_output.put_line ('-   P Postal code'); end if;
    if substr (address.errormessage,17,1) <> '?' then dbms_output.put_line ('-   A Postal add-on code'); end if;
  end if;
  dbms_output.put_line ('- MATCHCODE              ' || address.MATCHCODE || ' = ' ||
    case address.MATCHCODE
      when  1 then 'Exact match'
      when  2 then 'Match on city, postal code, street base name and house number'
      when  3 then 'Match on city, postal code and street base name'
      when  4 then 'Match on city and postal code'
      when 10 then 'Match on city but not postal code'
      when 11 then 'Match on postal but not on city'
    end
  );
  dbms_output.put_line ('- MATCHMODE              ' || address.MATCHMODE);
  dbms_output.put_line ('- LONGITUDE              ' || address.LONGITUDE);
  dbms_output.put_line ('- LATITUDE               ' || address.LATITUDE);
END;
/
show errors
