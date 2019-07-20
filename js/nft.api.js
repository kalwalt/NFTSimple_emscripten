;(function() {
	'use strict';

  // ARToolKit exported JS API
  //
  var nft = {

    UNKNOWN_MARKER: -1,
    PATTERN_MARKER: 0,
    BARCODE_MARKER: 1,

    initCamera: initCamera,

    //addNFTMarker: addNFTMarker

  };

  var FUNCTIONS = [
    //'initCamera'
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
