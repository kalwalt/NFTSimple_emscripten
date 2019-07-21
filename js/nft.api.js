;(function() {
	'use strict';
  if(window.artoolkit_wasm_url) {
        function downloadWasm(url) {
            return new Promise(function(resolve, reject) {
            var wasmXHR = new XMLHttpRequest();
            wasmXHR.open('GET', url, true);
            wasmXHR.responseType = 'arraybuffer';
            wasmXHR.onload = function() { resolve(wasmXHR.response); }
            wasmXHR.onerror = function() { reject('error '  + wasmXHR.status); }
            wasmXHR.send(null);
            });
        }

        var wasm = downloadWasm(window.artoolkit_wasm_url);

        // Module.instantiateWasm is a user-implemented callback which the Emscripten runtime calls to perform
        // the WebAssembly instantiation action. The callback function will be called with two parameters, imports
        // and successCallback. imports is a JS object which contains all the function imports that need to be passed
        // to the Module when instantiating, and once instantiated, the function should call successCallback() with
        // the WebAssembly Instance object.
        // The instantiation can be performed either synchronously or asynchronously. The return value of this function
        // should contain the exports object of the instantiated Module, or an empty dictionary object {} if the
        // instantiation is performed asynchronously, or false if instantiation failed.
        Module.instantiateWasm = function(imports, successCallback) {
            console.log('instantiateWasm: instantiating synchronously');
            wasm.then(function(wasmBinary) {
                console.log('wasm download finished, begin instantiating');
                var wasmInstantiate = WebAssembly.instantiate(new Uint8Array(wasmBinary), imports).then(function(output) {
                console.log('wasm instantiation succeeded');
                successCallback(output.instance);
            }).catch(function(e) {
                console.log('wasm instantiation failed! ' + e);
            });
            });
            return {}; // Compiling asynchronously, no exports.
        }
    }
  var NFTController = function(){
    console.log('NFT ready');
  };

  NFTController.prototype.setupCamera = function(src, xsize, ysize){
    nft.setupCamera(src, xsize, ysize);
  };

  var Test = function(text){
    var numb = 0;
    console.log(text);
  };

  var initCamera = function(){
    var video = document.getElementById('video');

  	// Get access to the camera!
  	if(navigator.mediaDevices && navigator.mediaDevices.getUserMedia) {
      	// Not adding `{ audio: true }` since we only want video now
      	navigator.mediaDevices.getUserMedia({ video: true }).then(function(stream) {
          	video.srcObject = stream;
          	video.play();
      			});
  	}
  }

  function initNFT(){

  };

  function setupCamera(src, xsize, ysize){
    Module._setupCamera(src, xsize, ysize);
  };

  // ARToolKit exported JS API
  //
  var nft = {

    UNKNOWN_MARKER: -1,
    PATTERN_MARKER: 0,
    BARCODE_MARKER: 1,

    //addNFTMarker: addNFTMarker
    setupCamera: setupCamera

  };

  var FUNCTIONS = [
    //'setupCamera',
    'initNFT',

    'unloadNFTData',
    'loadNFTData',

    'cleanup',
  ];

  function runWhenLoaded() {
    FUNCTIONS.forEach(function(n) {
      nft[n] = Module[n];
    })

    for (var m in Module) {
      if (m.match(/^NFT/))
      nft[m] = Module[m];
    }
  }



  /* Exports */
	window.nft = nft;
  window.Test = Test;
  window.NFTController = NFTController;
  window.initCamera = initCamera;

	if (window.Module) {
		window.Module.onRuntimeInitialized = function() {
            runWhenLoaded();
        }
	} else {
        window.Module = {
            onRuntimeInitialized: function() {
                runWhenLoaded();
            }
        };
    }

})();
