var GIFEncoder = require('gifencoder');
var pngFileStream = require('png-file-stream');
var fs = require('fs');
var _ = require("underscore");

var src = "img/title.png";

var encoder = new GIFEncoder(320, 60);
var stream = pngFileStream(src)
	.pipe(encoder.createWriteStream({ repeat: -1, delay: 0, quality: 10 }))
	.on("finish", function(){

		var pxs = encoder.indexedPixels;
		var pal = encoder.colorTab;

		// Build uint8_t array for .bin file.
		// First 512 bytes are RGB565 palette colors,
		// then 57600 bytes for palette lookups.
		var ar = new Uint8Array((256 * 2) + pxs.length);

		// Palette colors
		for (var c = 0; c < pal.length; c += 3) {
			var r = pal[c    ];
			var g = pal[c + 1];
			var b = pal[c + 2];

			var rgb565 = ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | ((b & 0xf8) >> 3);

			// Teensy C uses little endian
			var c_index = c / 3;
			ar[c_index * 2    ] = (rgb565 & 0x00ff);
			ar[c_index * 2 + 1] = (rgb565 & 0xff00) >> 8;
		}

		// Image data: Rearrange to the proper order
		var offset = 512;
		for (x = 0; x < 320; x++) {
			for (y = 59; y >= 0; y--) {
				ar[offset++] = pxs[y * 320 + x];
			}
		}

		// Let's just print this out
		console.log("static const uint8_t title[] = {");

		const per_line = 20;
		for (var i = 0; i < ar.length; i += per_line) {
			var sl = ar.slice(i, i + per_line);
			console.log("\t" + _.map(sl, function(val){
				return ("   " + val.toString()).substr(-3);
			}).join(",") + ",");
		}

		console.log("};");

	});
