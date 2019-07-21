/*
 *	nftSimple.c
 *  ARToolKit5
 *
 *	Demonstration of ARToolKit NFT. Renders a colour cube.
 *
 *  Press '?' while running for help on available key commands.
 *
 *  Disclaimer: IMPORTANT:  This Daqri software is supplied to you by Daqri
 *  LLC ("Daqri") in consideration of your agreement to the following
 *  terms, and your use, installation, modification or redistribution of
 *  this Daqri software constitutes acceptance of these terms.  If you do
 *  not agree with these terms, please do not use, install, modify or
 *  redistribute this Daqri software.
 *
 *  In consideration of your agreement to abide by the following terms, and
 *  subject to these terms, Daqri grants you a personal, non-exclusive
 *  license, under Daqri's copyrights in this original Daqri software (the
 *  "Daqri Software"), to use, reproduce, modify and redistribute the Daqri
 *  Software, with or without modifications, in source and/or binary forms;
 *  provided that if you redistribute the Daqri Software in its entirety and
 *  without modifications, you must retain this notice and the following
 *  text and disclaimers in all such redistributions of the Daqri Software.
 *  Neither the name, trademarks, service marks or logos of Daqri LLC may
 *  be used to endorse or promote products derived from the Daqri Software
 *  without specific prior written permission from Daqri.  Except as
 *  expressly stated in this notice, no other rights or licenses, express or
 *  implied, are granted by Daqri herein, including but not limited to any
 *  patent rights that may be infringed by your derivative works or by other
 *  works in which the Daqri Software may be incorporated.
 *
 *  The Daqri Software is provided by Daqri on an "AS IS" basis.  DAQRI
 *  MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 *  THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE, REGARDING THE DAQRI SOFTWARE OR ITS USE AND
 *  OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 *
 *  IN NO EVENT SHALL DAQRI BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 *  OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 *  MODIFICATION AND/OR DISTRIBUTION OF THE DAQRI SOFTWARE, HOWEVER CAUSED
 *  AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 *  STRICT LIABILITY OR OTHERWISE, EVEN IF DAQRI HAS BEEN ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *  Copyright 2015 Daqri LLC. All Rights Reserved.
 *  Copyright 2007-2015 ARToolworks, Inc. All Rights Reserved.
 *
 *  Author(s): Philip Lamb.
 *
 */


// ============================================================================
//	Includes
// ============================================================================

#ifdef _WIN32
#  include <windows.h>
#  define _USE_MATH_DEFINES
#  define snprintf _snprintf
#endif
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include <AR/ar.h>
#include <AR/arMulti.h>
#include <AR/video.h>
#include <AR/gsub_lite.h>
#include <ARUtil/time.h>
#include <AR/arFilterTransMat.h>
#include <AR2/tracking.h>
#include <AR/paramGL.h>

#include "ARMarkerNFT.h"
#include "trackingSub.h"

// ============================================================================
//	Constants
// ============================================================================

#define PAGES_MAX           10          // Maximum number of pages expected. You can change this down (to save memory) or up (to accomodate more pages.)

#define VIEW_SCALEFACTOR		1.0			// Units received from ARToolKit tracking will be multiplied by this factor before being used in OpenGL drawing.
#define VIEW_DISTANCE_MIN		10.0		// Objects closer to the camera than this will not be displayed. OpenGL units.
#define VIEW_DISTANCE_MAX		10000.0		// Objects further away from the camera than this will not be displayed. OpenGL units.

// ============================================================================
//	Global variables
// ============================================================================
struct Test {
	int numb;
};

struct arNFTController {
	ARParamLT *gCparamLT;
};

struct arNFTController _arNFTController;

// Preferences.
static int prefWindowed = TRUE;
static int prefWidth = 640;					// Fullscreen mode width.
static int prefHeight = 480;				// Fullscreen mode height.
static int prefDepth = 32;					// Fullscreen mode bit depth.
static int prefRefresh = 0;					// Fullscreen mode refresh rate. Set to 0 to use default rate.

// Markers.
ARMarkerNFT *markersNFT = NULL;
int markersNFTCount = 0;

// NFT.
static THREAD_HANDLE_T     *threadHandle = NULL;
static AR2HandleT          *ar2Handle = NULL;
static KpmHandle           *kpmHandle = NULL;
static int                  surfaceSetCount = 0;
static AR2SurfaceSetT      *surfaceSet[PAGES_MAX];
static long                 gCallCountMarkerDetect = 0;


// Drawing.
static int gWindowW;
static int gWindowH;
static ARParamLT *gCparamLT = NULL;
static ARdouble cameraLens[16];

//Video data
static ARUint8 *videoFrame = NULL;
static int videoFrameSize;
AR_PIXEL_FORMAT pixFormat = AR_PIXEL_FORMAT_RGBA;

// ============================================================================
//	Function prototypes
// ============================================================================
static int setupCamera(const char *cparam_name, int xsize, int ysize);
static int initNFT(ARParamLT *cparamLT);
static int loadNFTData(void);
static void cleanup(void);
// ============================================================================
//	Functions
// ============================================================================
/*
int main(int argc, char** argv)
{

	char glutGamemode[32];
	char *cparam_name = NULL;
  const char markerConfigDataFilename[] = "app/Data2/markers.dat";


#ifdef DEBUG
    arLogLevel = AR_LOG_LEVEL_DEBUG;
#endif




	if (!setupCamera(cparam_name,640, 480, &gCparamLT)) {
		ARLOGe("main(): Unable to set up AR camera.\n");
		exit(-1);
	}

    //
    // AR init.
    //

    // Create the OpenGL projection from the calibrated camera parameters.
    arglCameraFrustumRH(&(gCparamLT->param), VIEW_DISTANCE_MIN, VIEW_DISTANCE_MAX, cameraLens);

    if (!initNFT(gCparamLT)) {
		ARLOGe("main(): Unable to init NFT.\n");
		exit(-1);
    }

		arUtilTimerReset();

    //
    // Markers setup.
    //

    // Load marker(s).
    newMarkers(markerConfigDataFilename, &markersNFT, &markersNFTCount);
    if (!markersNFTCount) {
        ARLOGe("Error loading markers from config. file '%s'.\n", markerConfigDataFilename);
		cleanup();
		exit(-1);
    }
    ARLOGi("Marker count = %d\n", markersNFTCount);

    // Marker data has been loaded, so now load NFT data.
    if (!loadNFTData()) {
        ARLOGe("Error loading NFT data.\n");
		cleanup();
		exit(-1);
    }
	//arUtilTimerReset();

	return 0;
}
*/

static int setupCamera(const char *cparam_name, int xsize, int ysize)
{
		ARParam			cparam;
    ARLOGi("Camera image size (x,y) = (%d,%d)\n", xsize, ysize);

	// Get the format in which the camera is returning pixels.
	if (pixFormat == AR_PIXEL_FORMAT_INVALID) {
    	ARLOGe("setupCamera(): Camera is using unsupported pixel format.\n");
		return (FALSE);
	}

	// Load the camera parameters, resize for the window and init.
	if (cparam_name && *cparam_name) {
        if (arParamLoad(cparam_name, 1, &cparam) < 0) {
		    ARLOGe("setupCamera(): Error loading parameter file %s for camera.\n", cparam_name);
            return (FALSE);
        }
    } else {
        arParamClearWithFOVy(&cparam, xsize, ysize, M_PI_4); // M_PI_4 radians = 45 degrees.
        ARLOGw("Using default camera parameters for %dx%d image size, 45 degrees vertical field-of-view.\n", xsize, ysize);
    }
    if (cparam.xsize != xsize || cparam.ysize != ysize) {
        ARLOGw("*** Camera Parameter resized from %d, %d. ***\n", cparam.xsize, cparam.ysize);
        arParamChangeSize(&cparam, xsize, ysize, &cparam);
    }
#ifdef DEBUG
    ARLOG("*** Camera Parameter ***\n");
    arParamDisp(&cparam);
#endif
    if ((_arNFTController.gCparamLT = arParamLTCreate(&cparam, AR_PARAM_LT_DEFAULT_OFFSET)) == NULL) {
        ARLOGe("setupCamera(): Error: arParamLTCreate.\n");
        return (FALSE);
    }

	return (TRUE);
}

// Modifies globals: kpmHandle, ar2Handle.
static int initNFT(ARParamLT *cparamLT)
{
    ARLOGd("Initialising NFT.\n");
    //
    // NFT init.
    //

    // KPM init.
    kpmHandle = kpmCreateHandle(cparamLT);
    if (!kpmHandle) {
        ARLOGe("Error: kpmCreateHandle.\n");
        return (FALSE);
    }
    //kpmSetProcMode( kpmHandle, KpmProcHalfSize );

    // AR2 init.
    if( (ar2Handle = ar2CreateHandle(cparamLT, pixFormat, AR2_TRACKING_DEFAULT_THREAD_NUM)) == NULL ) {
        ARLOGe("Error: ar2CreateHandle.\n");
        kpmDeleteHandle(&kpmHandle);
        return (FALSE);
    }
    if (threadGetCPU() <= 1) {
        ARLOGi("Using NFT tracking settings for a single CPU.\n");
        ar2SetTrackingThresh(ar2Handle, 5.0);
        ar2SetSimThresh(ar2Handle, 0.50);
        ar2SetSearchFeatureNum(ar2Handle, 16);
        ar2SetSearchSize(ar2Handle, 6);
        ar2SetTemplateSize1(ar2Handle, 6);
        ar2SetTemplateSize2(ar2Handle, 6);
    } else {
        ARLOGi("Using NFT tracking settings for more than one CPU.\n");
        ar2SetTrackingThresh(ar2Handle, 5.0);
        ar2SetSimThresh(ar2Handle, 0.50);
        ar2SetSearchFeatureNum(ar2Handle, 16);
        ar2SetSearchSize(ar2Handle, 12);
        ar2SetTemplateSize1(ar2Handle, 6);
        ar2SetTemplateSize2(ar2Handle, 6);
    }
    // NFT dataset loading will happen later.
    return (TRUE);
}

// Modifies globals: threadHandle, surfaceSet[], surfaceSetCount
static int unloadNFTData(void)
{
    int i, j;

    if (threadHandle) {
        ARLOGi("Stopping NFT2 tracking thread.\n");
        trackingInitQuit(&threadHandle);
    }
    j = 0;
    for (i = 0; i < surfaceSetCount; i++) {
        if (j == 0) ARLOGi("Unloading NFT tracking surfaces.\n");
        ar2FreeSurfaceSet(&surfaceSet[i]); // Also sets surfaceSet[i] to NULL.
        j++;
    }
    if (j > 0) ARLOGi("Unloaded %d NFT tracking surfaces.\n", j);
    surfaceSetCount = 0;

    return 0;
}

// References globals: markersNFTCount
// Modifies globals: threadHandle, surfaceSet[], surfaceSetCount, markersNFT[]
static int loadNFTData(void)
{
    int i;
    KpmRefDataSet *refDataSet;

    // If data was already loaded, stop KPM tracking thread and unload previously loaded data.
    if (threadHandle) {
        ARLOGi("Reloading NFT data.\n");
        unloadNFTData();
    } else {
        ARLOGi("Loading NFT data.\n");
    }

    refDataSet = NULL;

    for (i = 0; i < markersNFTCount; i++) {
        // Load KPM data.
        KpmRefDataSet  *refDataSet2;
        ARLOGi("Reading %s.fset3\n", markersNFT[i].datasetPathname);
				//printf( markersNFT[i].datasetPathname);
        if (kpmLoadRefDataSet(markersNFT[i].datasetPathname, "fset3", &refDataSet2) < 0 ) {
            ARLOGe("Error reading KPM data from %s.fset3\n", markersNFT[i].datasetPathname);
            markersNFT[i].pageNo = -1;
            continue;
        }
        markersNFT[i].pageNo = surfaceSetCount;
        ARLOGi("  Assigned page no. %d.\n", surfaceSetCount);
        if (kpmChangePageNoOfRefDataSet(refDataSet2, KpmChangePageNoAllPages, surfaceSetCount) < 0) {
            ARLOGe("Error: kpmChangePageNoOfRefDataSet\n");
            exit(-1);
        }
        if (kpmMergeRefDataSet(&refDataSet, &refDataSet2) < 0) {
            ARLOGe("Error: kpmMergeRefDataSet\n");
            exit(-1);
        }
        ARLOGi("  Done.\n");

        // Load AR2 data.
        ARLOGi("Reading %s.fset\n", markersNFT[i].datasetPathname);

        if ((surfaceSet[surfaceSetCount] = ar2ReadSurfaceSet(markersNFT[i].datasetPathname, "fset", NULL)) == NULL ) {
            ARLOGe("Error reading data from %s.fset\n", markersNFT[i].datasetPathname);
        }
        ARLOGi("  Done.\n");

        surfaceSetCount++;
        if (surfaceSetCount == PAGES_MAX) break;
    }
    if (kpmSetRefDataSet(kpmHandle, refDataSet) < 0) {
        ARLOGe("Error: kpmSetRefDataSet\n");
        exit(-1);
    }
    kpmDeleteRefDataSet(&refDataSet);

    // Start the KPM tracking thread.
    threadHandle = trackingInitInit(kpmHandle);
    if (!threadHandle) exit(-1);

	ARLOGi("Loading of NFT data complete.\n");
    return (TRUE);
}

static void cleanup(void)
{
    if (markersNFT) deleteMarkers(&markersNFT, &markersNFTCount);

    // NFT cleanup.
    unloadNFTData();
		ARLOGd("Cleaning up ARToolKit NFT handles.\n");
    ar2DeleteHandle(&ar2Handle);
    kpmDeleteHandle(&kpmHandle);
    arParamLTFree(&gCparamLT);
		if (videoFrame) {
			free(videoFrame);
			videoFrame = NULL;
			videoFrameSize = 0;
		}
#ifdef _WIN32
	CoUninitialize();
#endif
}
/*
static void mainLoop(void)
{
  // example: draw a moving rectangle
	static int ms_prev;
	int ms;
	float s_elapsed;
	AR2VideoBufferT *image;

  // NFT results.
  static int detectedPage = -2; // -2 Tracking not inited, -1 tracking inited OK, >= 0 tracking online on page.
  static float trackingTrans[3][4];


  int             i, j, k;

	// Find out how long since mainLoop() last ran.
	//ms = glutGet(GLUT_ELAPSED_TIME);
	ms = 120;
	s_elapsed = (float)(ms - ms_prev) * 0.001f;
	if (s_elapsed < 0.01f) return; // Don't update more often than 100 Hz.
	ms_prev = ms;

	// Update drawing.
	DrawCubeUpdate(s_elapsed);

	// Grab a video frame.
    //image = arVideoGetImage();
		image = videoFrame;
    if (image && image->fillFlag) {

        arglPixelBufferDataUpload(gArglSettings, image->buff);

        gCallCountMarkerDetect++; // Increment ARToolKit FPS counter.

        // Run marker detection on frame
        if (threadHandle) {
            // Perform NFT tracking.
            float            err;
            int              ret;
            int              pageNo;

            if( detectedPage == -2 ) {
                trackingInitStart( threadHandle, image->buffLuma );
                detectedPage = -1;
            }
            if( detectedPage == -1 ) {
                ret = trackingInitGetResult( threadHandle, trackingTrans, &pageNo);
                if( ret == 1 ) {
                    if (pageNo >= 0 && pageNo < surfaceSetCount) {
                        ARLOGd("Detected page %d.\n", pageNo);
                        detectedPage = pageNo;
                        ar2SetInitTrans(surfaceSet[detectedPage], trackingTrans);
                    } else {
                        ARLOGe("Detected bad page %d.\n", pageNo);
                        detectedPage = -2;
                    }
                } else if( ret < 0 ) {
                    ARLOGd("No page detected.\n");
                    detectedPage = -2;
                }
            }
            if( detectedPage >= 0 && detectedPage < surfaceSetCount) {
                if( ar2Tracking(ar2Handle, surfaceSet[detectedPage], image->buff, trackingTrans, &err) < 0 ) {
                    ARLOGd("Tracking lost.\n");
                    detectedPage = -2;
                } else {
                    ARLOGd("Tracked page %d (max %d).\n", detectedPage, surfaceSetCount - 1);
                }
            }
        } else {
            ARLOGe("Error: threadHandle\n");
            detectedPage = -2;
        }

        // Update markers.
        for (i = 0; i < markersNFTCount; i++) {
            markersNFT[i].validPrev = markersNFT[i].valid;
            if (markersNFT[i].pageNo >= 0 && markersNFT[i].pageNo == detectedPage) {
                markersNFT[i].valid = TRUE;
                for (j = 0; j < 3; j++) for (k = 0; k < 4; k++) markersNFT[i].trans[j][k] = trackingTrans[j][k];
            }
            else markersNFT[i].valid = FALSE;
            if (markersNFT[i].valid) {

                // Filter the pose estimate.
                if (markersNFT[i].ftmi) {
                    if (arFilterTransMat(markersNFT[i].ftmi, markersNFT[i].trans, !markersNFT[i].validPrev) < 0) {
                        ARLOGe("arFilterTransMat error with marker %d.\n", i);
                    }
                }

                if (!markersNFT[i].validPrev) {
                    // Marker has become visible, tell any dependent objects.
                    // --->
                }

                // We have a new pose, so set that.
                arglCameraViewRH((const ARdouble (*)[4])markersNFT[i].trans, markersNFT[i].pose.T, VIEW_SCALEFACTOR);
                // Tell any dependent objects about the update.
                // --->

            } else {

                if (markersNFT[i].validPrev) {
                    // Marker has ceased to be visible, tell any dependent objects.
                    // --->
                }
            }
        }
	}
}
*/
//#include "ARBindEM.cpp"
