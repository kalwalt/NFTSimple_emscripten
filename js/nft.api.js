;(function() {
	'use strict';

  // ARToolKit exported JS API
  //
  function initCamera(){
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

  var nft = {

    UNKNOWN_MARKER: -1,
    PATTERN_MARKER: 0,
    BARCODE_MARKER: 1,

    //addNFTMarker: addNFTMarker

  };

  var FUNCTIONS = [
    'setupCamera',
    'initNFT',

    'unloadNFTData',
    'loadNFTData',

    'cleanup',
  ];

  function runWhenLoaded() {
    FUNCTIONS.forEach(function(n) {
      nft[n] = Module[n];
    })
  }



  /* Exports */
	window.nft = artoolkit;

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
