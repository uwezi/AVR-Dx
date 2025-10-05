$fn=120;

eps=1e-3;

include <D:\uwezi\3D\libs\Round-Anything\polyround.scad>

outer_width = 53+4;
outer_depth = 24;
outer_height = 21;

module pcb()
{
  import("20250807_updi.stl");
}

module m30()
{
  rotate([-90,0,0])
  union()
  {
    cylinder(d=4.3,h=15);
    translate([0,0,-15])
    cylinder(d=3.2,h=7);
    translate([0,0,-7-1])
    cylinder(d=7,h=7);
  }
}

module m25()
{
  rotate([-90,0,0])
  union()
  {
    cylinder(d=3.5,h=7);
    translate([0,0,-7])
    cylinder(d=2.4,h=7);
    translate([0,0,-7-1])
    cylinder(d=4.5,h=7);
  }
}

module box()
{
  difference()
  {
    polyRoundExtrude(
      [
        [-outer_width/2,0,2],
        [+outer_width/2,0,2],
        [+outer_width/2,outer_depth,2],
        [-outer_width/2,outer_depth,2],
      ],
      outer_height,
      2,2,
      fn=60
    );
    
    union()
    {
      translate([-53.5/2,2,9+2])
      cube([54,20.5,2.4]);
      
      difference()
      {
        union()
        {
          translate([-51/2,2,2])
          cube([51,20,13]);
          
          translate([-41/2,2,2])
          cube([41,20,16]);
        }
        translate([-16,0,0])
        cube([32,18,8]);
      }
      
      translate([-10,18,14.5])
      rotate([-90,0,0])
      union()
      {
        translate([-5.5/2,0,0])
        cylinder(d=7,h=10);
        translate([+5.5/2,0,0])
        cylinder(d=7,h=10);
        translate([-5.5/2,-3.5,0])
        cube([5.5,7,10]);
      }
      
      translate([-19.5,2,15])
      cube([12,6,10]);

      translate([-19+17.5,2,15])
      cube([12,6,10]);

      translate([-19+27,2,15])
      cube([10,10,10]);
      
      for (p=[
           //[-50/2,2,17.5],[+50/2,2,17.5],
           [0,2,4.5],
       ])
      {
        translate(p)
        m30();
      }  
    }
  }
}



module base()
{
  intersection()
  {
  translate([-50,2+eps,-100])
  cube([100,100,200]);
  box();
  }
}

module top()
{
  intersection()
  {
  translate([-50,-98,-100])
  cube([100,100,200]);
  box();
  }
  for (x=[16.5,-51/2+0.5])
  {
    translate([x,2,2.5])
    #cube([(51-32)/2-1,3,7]);
  }

}
base();
/*
  translate([0,10+2,9+2])
  color("red")
  pcb();
*/