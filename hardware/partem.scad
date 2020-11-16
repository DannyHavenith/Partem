d = .01;
ds = [d,d,d];
loss = .35;
connector_dims = [13.5, 14.7, 17.9] + [0, 2*loss, 0];
cutout_offset = 2.4 + loss/2;
enclosure_dims = [54,68,18];
wall = 1.6;

function uniform_product( a, b) = [ a[0] * b[0], a[1] * b[1], a[2] * b[2]];
function up(a,b) = uniform_product( a,b);

module connector()
{
    cutout_dims = [4.1,  1.5, connector_dims[2]] - [loss, loss, 0] +  [d, d, 2*d];
    bottom_dims = [ connector_dims[0], 2.9, 3.8] - [0, loss, loss] + [2*d, d, d];
    difference(){
        translate( -connector_dims/2) cube( connector_dims);
        translate( -connector_dims/2 + [cutout_offset, -d, -d ]) cube( cutout_dims);
        mirror([0,1,0]) translate( -connector_dims/2 + [cutout_offset, -d, -d ]) cube( cutout_dims);
        translate( -connector_dims/2 + [-d, -d, -d]) cube( bottom_dims);
        mirror([0,1,0]) translate( -connector_dims/2 + [-d, -d, -d]) cube( bottom_dims);
    }
}

cube_dims = [13, 18, 20];
module connector_frame()
{
    margins = (cube_dims - connector_dims)/2;
    difference()
    {
        translate( -cube_dims/2) cube( cube_dims);
        translate( up( margins, [1, 0, 1]) + [d, 0, d]) connector();
    }
}

// a cube with rounded corners
module roundedcube( dimensions, r)
{
    rs = [r,r,r];
    inner = dimensions - 2 * rs;
    
    translate( -inner/2)
        hull()
            for (x = [0:1]) for ( y = [0:1]) for (z = [0:1])
                translate( up( inner, [x, y, z])) sphere( r = r, $fn = 20);
}

module flatroundedcube( dimensions, r)
{
    rs = [r,r,0];
    inner = dimensions - 2 * rs;
    translate( -inner/2)
        hull()
            for ( x = [0:1]) for ( y = [0:1])
                translate(up( inner, [x,y,0])) cylinder( h = dimensions[2], r = r, $fn = 20);
}


walls = [wall, wall, wall];
outer_dims = enclosure_dims + 2*walls;

module enclosure_hull( bottom=true)
{
    gland_depth = 4;
    cable_diameter = 5.5;
    difference()
    {
        union() {
        
            // enclosure proper
            difference() {
                roundedcube( outer_dims+[0,0,.1], 3);
                flatroundedcube( enclosure_dims, 3);
            }
            
            // inside and outside gland
            translate(up( enclosure_dims, [-.501, 0, 0]))
                rotate([0,90,0]) cylinder(d1 = 15, d2 = cable_diameter, h = gland_depth, $fn = 30);                      
            translate(up( outer_dims, [-.499, 0, 0]))
                rotate([0,-90,0]) cylinder(d1 = 15, d2 = 5, h = gland_depth, $fn = 30);
        }
        
        // hole for connector frame
        translate( up( outer_dims - cube_dims, [.5, 0, -.5])) cube(cube_dims - ds, center=true);
        
        // hole for cable
        translate( up( (outer_dims+enclosure_dims)/2, [-.5, 0, 0])) 
            rotate([0,90,0]) cylinder( d=cable_diameter, h=wall*2 + 2*gland_depth, center=true, $fn=50);
            
        // cut off bottom or top
        translate( up( outer_dims, [-1, -1, bottom?0:-2])) cube( 2*outer_dims);
    }
    
    
}

module tab( expand = false)
{
    tab_dim = [2, 12 + (expand?1:0), 15];
    inset = .4;
    hook = .8 + (expand?.2:0);
    
    translate( up( tab_dim, [-1, -.5, -.5])) cube( tab_dim + [ inset, 0, 0]);
    translate([0,0, -tab_dim[2]/2 + wall + hook]) 
        rotate([90,0,0]) 
            cylinder( r = hook, h = tab_dim[1] - 2* wall, center=true, $fn=4);
}

module tabs( expand = false)
{
    positions = up( enclosure_dims, [.5, .3, 0]);
    for ( x = [-1, 1]) for ( y = [-1, 1])
        translate(up(positions, [x, y, 0])) 
            rotate([0,0, x<0?180:0]) tab( expand);
}

module top()
{
    enclosure_hull( false);
    tabs();
}

module bottom()
{
    difference()
    {
        enclosure_hull( true);
        tabs( true);
    }
    translate( up( outer_dims - cube_dims, [.5, 0, -.5])) connector_frame();
    towers();
}

module screw_tower()
{
    diam = 3;
    height = 5;
    difference() {
        cylinder( d = diam + 2*wall, h = height, $fn = 30);
        cylinder( d1 = diam, d2 = diam + .4, h = height +d, $fn = 30);
    }
}

module towers() {
    tenth = 2.54; // one tenth of an inch
    for (x = [-4,4]) for ( y = [0,10])
        translate([ x*tenth - 5, y*tenth, -enclosure_dims[2]/2 -d ]) screw_tower();
}


//top();
bottom();


