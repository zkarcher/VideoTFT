var GIFEncoder = require('gifencoder');
var pngFileStream = require('png-file-stream');
var fs = require('fs');
var _ = require("underscore");

var srcDir = "img/png/";
var destDir = "img/gif/";

// Batch jobs
var items = [];

// Find valid png files
var dir_items = fs.readdirSync(srcDir);
_.each(dir_items, function(item){
	var match = item.match(/(\d+)\.png$/i);
	if (match) {
		items.push(item);
	}
});

// Test: Truncate frames for quick dev cycles
//items = items.slice(0,10);

var gif_count = 0;
var data = _.range(items.length);

function processPng(src, dest, index)
{
	var encoder = new GIFEncoder(320, 180);
	var stream = pngFileStream(src)
		.pipe(encoder.createWriteStream({ repeat: -1, delay: 0, quality: 10 }))
		//.pipe(fs.createWriteStream(dest))
		.on("finish", function(){
			console.log("finish:", index);

			this.pipe(fs.createWriteStream(dest))
			.on("finish", function() {

				var pxs = encoder.indexedPixels;
				var pal = encoder.colorTab;

				/*
				console.log(pxs.length);	// 57600 == 320 * 180
				console.log(pxs.join(","));
				console.log(pal.length);	// 768 == 256 * 3
				console.log(pal.join(","));
				*/

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

				// Image data
				for (var p = 0; p < pxs.length; p++) {
					ar[256 * 2 + p] = pxs[p];
				}

				data[index] = ar;

				// Done?
				gif_count++;
				if (gif_count == items.length) {
					buildBinFile();
				}

			});
		});
}

_.each(items, function(item, index) {
	var match = item.match(/(\d+)\.png$/i);
	var src = srcDir + item;
	var dest = destDir + match[1] + ".gif";

	processPng(src, dest, index);
});

// After batch: Create data file
function buildBinFile()
{
	console.log("buildBinFile() ...");

	var fd = fs.openSync("img/VIDEO.BIN", "w");

	var buf = new Buffer(4);
	buf.writeUInt32LE(items.length, 0);	// Teensy C uses little endian
	fs.writeSync(fd, buf, 0, 4);

	for (var i = 0; i < data.length; i++) {
		fs.writeSync(fd, data[i], 0, data[i].length);
	}

	fs.closeSync(fd);

	console.log("Done!");
}
