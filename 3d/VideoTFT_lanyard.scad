// VideoTFT lanyard!
//
// 2017-11-09: I'm designing + printing this 24 hours before flying
//    to Hackaday SuperConference! ... What could possibly go wrong?
//    The lanyard needs raised panel(s) on the front to support the display.
//    Needs a rectangular hole for socket pins (I'd rather not solder the
//    components together; I like being able to unplug/swap everything).
//    Teensy and battery pack will be suspended in the back... somehow.
//    Anyway, let's figure this out:

$fn = 30;

// TFT has an awkward macro SD card slot (unused) attached to the back,
// and some other components. Rect mounts should dodge these.
MOUNT_T = 3.0;

// Panel (backplate).
// Make this a bit higher, for lanyard holes.
PANEL_T = 1.0;
PANEL_W = 71;	// Not including round corner radius
PANEL_H = 44;	// Not including round corner radius
EXTRA_H = 9;
CORNER_R = 6.5;

// TFT board has holes. Use zip ties?
HOLE_X = 6;
HOLE_Y = 3;

PUNCH_R = 4;
ZIP_R = 2.25;	// My zip ties are 3mm wide

module LanyardDonut() {
	DONUT_R = (CORNER_R - PUNCH_R) / 2;

	rotate_extrude(angle=360)
		translate([DONUT_R + PUNCH_R, 0, 0])
			intersection() {
				scale([1.0, 0.4])
					circle(DONUT_R);
				translate([-10,-20])
					square([20,20], center=false);	// Half-torus
			}
}

module SolidPart() {
	// TFT display support blocks
	translate([14, 2, -MOUNT_T + 0.0001])	// BL
		cube([15, 20, MOUNT_T], center=false);
	translate([63, 2, -MOUNT_T + 0.0001])	// BR
		cube([15, 20, MOUNT_T], center=false);
	translate([35, 32, -MOUNT_T + 0.0001])	// TR
		cube([40, 15, MOUNT_T], center=false);
	
	// Panel (backplate)
	translate([HOLE_X, HOLE_Y, 0])
		minkowski() {
			cube([PANEL_W, PANEL_H + EXTRA_H, PANEL_T], center=false);
			cylinder(h=0.0001, r=CORNER_R, center=true);
		}	
	
	// Lanyard donuts (half-torus)
	translate([HOLE_X, HOLE_Y + PANEL_H + EXTRA_H, 0.001])
		LanyardDonut();
	translate([HOLE_X + PANEL_W, HOLE_Y + PANEL_H + EXTRA_H, 0.001])
		LanyardDonut();
}

module PunchHoles() {
	// Lanyard holes
	translate([0, PANEL_H + EXTRA_H, 0])
		cylinder(h=100, r=PUNCH_R, center=true);
	translate([PANEL_W, PANEL_H + EXTRA_H, 0])
		cylinder(h=100, r=PUNCH_R, center=true);
	
	// Zip tie holes for attaching TFT
	ZIP_Y = -2;
	
	translate([0, ZIP_Y, 0])
		cylinder(h=100, r=ZIP_R, center=true);
	translate([PANEL_W, ZIP_Y, 0])
		cylinder(h=100, r=ZIP_R, center=true);
	translate([ZIP_Y / 2, PANEL_H, 0])
		cylinder(h=100, r=ZIP_R, center=true);
	translate([PANEL_W - ZIP_Y, PANEL_H, 0])
		cylinder(h=100, r=ZIP_R, center=true);
	
	// Punch rect hole for header pins
	translate([-5, PANEL_H / 2, 0])
		cube([3.5, 40, 100], center=true);
		
	// Support the Teensy & battery, somehow??
	// Gonna punch some holes, hopefully can ziptie this whole thing together.
	STEP = 8;
	for (i = [8:STEP:70])
		for (j = [4:STEP:50])
			translate([i, j, 0])
				cylinder(h=100, r=ZIP_R, center=true);
}

difference() {
	SolidPart();
	translate([HOLE_X, HOLE_Y, 0])
		PunchHoles();
}
