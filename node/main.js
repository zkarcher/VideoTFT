var GIFEncoder = require('gifencoder');
var encoder = new GIFEncoder(320, 180);
var pngFileStream = require('png-file-stream');
var fs = require('fs');
var Worker = require('webworker-threads').Worker;

var srcDir = "img/png/";
var destDir = "img/gif/";

fs.readdir(srcDir, function(err, items) {
	for (var i=0; i<Math.min(10,items.length); i++) {
		var name = items[i];

		var match = name.match(/(\d+)\.png$/i);
		if (!match) continue;	// Skip .DS_Store, etc

		var w = new Worker('gif_worker.js'); // Standard API

		w.onmessage = function(e) {
			console.log(e);
			this.terminate();
		}

		w.postMessage({src: srcDir + name, dest: destDir + match[1] + ".gif"});
	}
});
