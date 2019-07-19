/*
 * Simple script for running emcc on ARToolKit
 * @author zz85 github.com/zz85
 */


var
	exec = require('child_process').exec,
	path = require('path'),
	fs = require('fs'),
	child;

var HAVE_NFT = 1;

var EMSCRIPTEN_ROOT = process.env.EMSCRIPTEN;
var ARTOOLKIT5_ROOT = process.env.ARTOOLKIT5_ROOT || "/home/walter/kalwalt-github/jsartoolkit5/emscripten/artoolkit5";
var LIBJPEG_ROOT = process.env.LIBJPEG_ROOT || "../emscripten/jpeg-6b";

if (!EMSCRIPTEN_ROOT) {
	console.log("\nWarning: EMSCRIPTEN environment variable not found.")
	console.log("If you get a \"command not found\" error,\ndo `source <path to emsdk>/emsdk_env.sh` and try again.");
}

var EMCC = EMSCRIPTEN_ROOT ? path.resolve(EMSCRIPTEN_ROOT, 'emcc') : 'emcc';
var EMPP = EMSCRIPTEN_ROOT ? path.resolve(EMSCRIPTEN_ROOT, 'em++') : 'em++';
var OPTIMIZE_FLAGS = ' -Oz '; // -Oz for smallest size
var MEM = 256 * 1024 * 1024; // 64MB


var SOURCE_PATH = path.resolve(__dirname, '../emscripten/') + '/';
var OUTPUT_PATH = path.resolve(__dirname, '../app/build/') + '/';

var BUILD_HTML5 = 'nftSimple.js';
var PRELOAD_FILE = ' --preload-file app/Data2/markers.dat';

var MAIN_SOURCES = [
	'ARMarkerNFT.c',
	'trackingSub.c',
	'nftSimple.c'
];

MAIN_SOURCES = MAIN_SOURCES.map(function(src) {
	return path.resolve(SOURCE_PATH, src);
}).join(' ');


var ar_sources = [
	'AR/arLabelingSub/*.c',
	'AR/*.c',
	'ARICP/*.c',
	'ARMulti/*.c',
	'Video/video.c',
	//'Video/video2.c',
	'ARUtil/log.c',
	'ARUtil/time.c',
  'ARUtil/file_utils.c',
	'ARUtil/thread_sub.c',
	//'Video/videoLuma.c',
	'Gl/gsub_lite.c',
	'Gl/argWindow.c',
	//'ARWrapper/ARController.cpp'
].map(function(src) {
	return path.resolve(__dirname, ARTOOLKIT5_ROOT + '/lib/SRC/', src);
});

var ar2_sources = [
	'handle.c',
	'imageSet.c',
	'jpeg.c',
	'marker.c',
	'featureMap.c',
	'featureSet.c',
	'selectTemplate.c',
	'surface.c',
	'tracking.c',
	'tracking2d.c',
	'matching.c',
	'matching2.c',
	'template.c',
	'searchPoint.c',
	'coord.c',
	'util.c',
].map(function(src) {
	return path.resolve(__dirname, ARTOOLKIT5_ROOT + '/lib/SRC/AR2/', src);
});

var kpm_sources = [
	'kpmHandle.c*',
	'kpmRefDataSet.c*',
	'kpmMatching.c*',
	'kpmResult.c*',
	'kpmUtil.c*',
	'kpmFopen.c*',
	'FreakMatcher/detectors/DoG_scale_invariant_detector.c*',
	'FreakMatcher/detectors/gaussian_scale_space_pyramid.c*',
	'FreakMatcher/detectors/gradients.c*',
	'FreakMatcher/detectors/harris.c*',
	'FreakMatcher/detectors/orientation_assignment.c*',
	'FreakMatcher/detectors/pyramid.c*',
	'FreakMatcher/facade/visual_database_facade.c*',
	'FreakMatcher/matchers/hough_similarity_voting.c*',
	'FreakMatcher/matchers/freak.c*',
	'FreakMatcher/framework/date_time.c*',
	'FreakMatcher/framework/image.c*',
	'FreakMatcher/framework/logger.c*',
	'FreakMatcher/framework/timers.c*',
].map(function(src) {
	return path.resolve(__dirname, ARTOOLKIT5_ROOT + '/lib/SRC/KPM/', src);
});

if (HAVE_NFT) {
	ar_sources = ar_sources
	.concat(ar2_sources)
	.concat(kpm_sources);
}

var DEFINES = ' ';
if (HAVE_NFT) DEFINES += ' -D HAVE_NFT=1 ';

var FLAGS = '' + OPTIMIZE_FLAGS;
FLAGS += ' -Wno-warn-absolute-paths ';
FLAGS += ' -s TOTAL_MEMORY=' + MEM + ' ';
FLAGS += ' -s USE_ZLIB=1';
//FLAGS += ' -s ERROR_ON_UNDEFINED_SYMBOLS=0';
//FLAGS += ' -s NO_BROWSER=1 '; // for 20k less
FLAGS += ' --memory-init-file 0 '; // for memless file
//FLAGS += ' -s BINARYEN_TRAP_MODE=clamp'

FLAGS += ' --bind ';
FLAGS += ' -msse';
FLAGS += ' -msse2';
FLAGS += ' -msse3';
FLAGS += ' -mssse3';
var CFLAGS = ' -Wimplicit-function-declaration -DHAVE_NFT=1 -s USE_PTHREADS=1'
//-I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include
/* DEBUG FLAGS */
var DEBUG_FLAGS = ' -g ';
// DEBUG_FLAGS += ' -s ASSERTIONS=2 '
DEBUG_FLAGS += ' -s ASSERTIONS=1 '
//DEBUG_FLAGS += ' --profiling '
// DEBUG_FLAGS += ' -s EMTERPRETIFY_ADVISE=1 '
DEBUG_FLAGS += ' -s ALLOW_MEMORY_GROWTH=1';
DEBUG_FLAGS += '  -s DEMANGLE_SUPPORT=1 ';

var EMRUN_FLAGS = ' --emrun '

var INCLUDES = [
	path.resolve(__dirname, ARTOOLKIT5_ROOT + '/include'),
	OUTPUT_PATH,
	SOURCE_PATH,
	path.resolve(__dirname, ARTOOLKIT5_ROOT + '/lib/SRC/KPM/FreakMatcher'),
	//path.resolve(__dirname, ARTOOLKIT5_ROOT + '/lib/SRC/GL'),
	//path.resolve(__dirname, ARTOOLKIT5_ROOT + '/lib/SRC/Video'),
	path.resolve(__dirname, ARTOOLKIT5_ROOT + '/../jpeg-6b'),
	//path.resolve(__dirname, ARTOOLKIT5_ROOT + '/Video'),
	//path.resolve(__dirname, SOURCE_PATH + '/'),
	//'lib/SRC/KPM/FreakMatcher',
	// 'include/macosx-universal/',
	// '../jpeg-6b',
].map(function(s) { return '-I' + s }).join(' ');

function format(str) {
	for (var f = 1; f < arguments.length; f++) {
		str = str.replace(/{\w*}/, arguments[f]);
	}
	return str;
}


// Lib JPEG Compilation

// Memory Allocations
// jmemansi.c jmemname.c jmemnobs.c jmemdos.c jmemmac.c
var libjpeg_sources = 'jcapimin.c jcapistd.c jccoefct.c jccolor.c jcdctmgr.c jchuff.c \
		jcinit.c jcmainct.c jcmarker.c jcmaster.c jcomapi.c jcparam.c \
		jcphuff.c jcprepct.c jcsample.c jctrans.c jdapimin.c jdapistd.c \
		jdatadst.c jdatasrc.c jdcoefct.c jdcolor.c jddctmgr.c jdhuff.c \
		jdinput.c jdmainct.c jdmarker.c jdmaster.c jdmerge.c jdphuff.c \
		jdpostct.c jdsample.c jdtrans.c jerror.c jfdctflt.c jfdctfst.c \
		jfdctint.c jidctflt.c jidctfst.c jidctint.c jidctred.c jquant1.c \
		jquant2.c jutils.c jmemmgr.c \
		jmemansi.c'.split(/\s+/).join(' /home/walter/kalwalt-github/jsartoolkit5/emscripten/jpeg-6b/')
function clean_builds() {
	try {
		var stats = fs.statSync(OUTPUT_PATH);
	} catch (e) {
		fs.mkdirSync(OUTPUT_PATH);
	}

	try {
		var files = fs.readdirSync(OUTPUT_PATH);
		if (files.length > 0)
		for (var i = 0; i < files.length; i++) {
			var filePath = OUTPUT_PATH + '/' + files[i];
			if (fs.statSync(filePath).isFile())
				fs.unlinkSync(filePath);
		}
	}
	catch(e) { return console.log(e); }
}

var compile_arlib = format(EMCC + ' ' + INCLUDES + ' '
	+ ar_sources.join(' ')
	+ FLAGS + ' ' + DEFINES + ' -o {OUTPUT_PATH}libar.bc ',
		OUTPUT_PATH);

 var compile_kpm = format(EMCC + ' ' + INCLUDES + ' '
 	+ kpm_sources.join(' ')
 	+ FLAGS + ' ' + DEFINES + ' -o {OUTPUT_PATH}libkpm.bc ',
 		OUTPUT_PATH);

var compile_libjpeg = format(EMCC + ' ' + INCLUDES + ' '
    + path.resolve(__dirname, LIBJPEG_ROOT) + '/' + libjpeg_sources
	+ FLAGS + ' ' + DEFINES + ' -o {OUTPUT_PATH}libjpeg.bc ',
		OUTPUT_PATH);

/*var compile_combine = format(EMCC + ' ' + INCLUDES + ' '
	+ ' {OUTPUT_PATH}*.bc ' + MAIN_SOURCES
	+ FLAGS + ' -s WASM=0' + ' '  + DEBUG_FLAGS + DEFINES + ' -o {OUTPUT_PATH}{BUILD_FILE} ',
	OUTPUT_PATH, OUTPUT_PATH, BUILD_DEBUG_FILE);*/

var compile_html5 = format(EMCC + ' '  + INCLUDES + ' '
	+ ' {OUTPUT_PATH}*.bc ' + MAIN_SOURCES
	+ FLAGS  + ' ' + CFLAGS + DEFINES + EMRUN_FLAGS + ' -s WASM=1' + ' -s ERROR_ON_UNDEFINED_SYMBOLS=0' + PRELOAD_FILE + ' -o {OUTPUT_PATH}{BUILD_FILE} ',
	OUTPUT_PATH, OUTPUT_PATH, BUILD_HTML5);
/*
/*
 * Run commands
 */

function onExec(error, stdout, stderr) {
	if (stdout) console.log('stdout: ' + stdout);
	if (stderr) console.log('stderr: ' + stderr);
	if (error !== null) {
		console.log('exec error: ' + error.code);
		process.exit(error.code);
	} else {
		runJob();
	}
}

function runJob() {
	if (!jobs.length) {
		console.log('Jobs completed');
		return;
	}
	var cmd = jobs.shift();

	if (typeof cmd === 'function') {
		cmd();
		runJob();
		return;
	}

	console.log('\nRunning command: ' + cmd + '\n');
	exec(cmd, onExec);
}

var jobs = [];

function addJob(job) {
	jobs.push(job);
}

addJob(clean_builds);
addJob(compile_arlib);
//addJob(compile_kpm);
// compile_kpm
addJob(compile_libjpeg);
addJob(compile_html5);

runJob();
