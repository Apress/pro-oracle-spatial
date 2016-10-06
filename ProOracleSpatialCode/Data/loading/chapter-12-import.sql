-- Load network metadata and map definitions
insert into user_sdo_network_metadata
  select * from my_network_metadata;
commit;

-- Load map definitions
insert into user_sdo_styles
  select * from my_styles;
insert into user_sdo_themes
  select * from my_themes;
insert into user_sdo_maps
  select * from my_maps;
commit;

-- Create PL/SQL tools
create or replace function get_point_x(g sdo_geometry) return number is
begin
  return g.sdo_point.x;
end;
/
create or replace function get_point_y(g sdo_geometry) return number is
begin
  return g.sdo_point.y;
end;
/

