#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <stdlib.h>
#include "opencv2/opencv.hpp"
using namespace cv;
#define _RTSP_
//#define _CAM_
// TODO: use synchronized deque

std::deque<Mat> frameQueue;

GstFlowReturn new_preroll(GstAppSink *appsink, gpointer data)
{
  g_print ("Got preroll!\n");
  return GST_FLOW_OK;
}
// Convert I420 -> RGB Mat
void I420toRGB(Mat src, Mat dest)
{
//
	
}

//
GstFlowReturn new_sample(GstAppSink *appsink, gpointer data)
{
  static int framecount = 0;
  framecount++;

  GstSample *sample = gst_app_sink_pull_sample(appsink);
  GstCaps *caps = gst_sample_get_caps(sample);
  GstBuffer *buffer = gst_sample_get_buffer(sample);
  const GstStructure *info = gst_sample_get_info(sample);

  // ---- Read frame and convert to opencv format ---------------

  GstMapInfo map;
  gst_buffer_map (buffer, &map, GST_MAP_READ);

  // convert gstreamer data to OpenCV Mat, you could actually
  // resolve height / width from caps...
  Mat frame(Size(1280, 720), CV_8UC3, (char*)map.data, Mat::AUTO_STEP);

  int frameSize = map.size;
  frameQueue.push_back(frame);
  gst_buffer_unmap(buffer, &map);

  // ------------------------------------------------------------

  // print dot every 30 frames
  if (framecount%30 == 0) {
    g_print (".");
  }

  // show caps on first frame
  if (framecount == 1) {
    g_print ("%s\n", gst_caps_to_string(caps));
  }

  gst_sample_unref (sample);
  return GST_FLOW_OK;
}

static gboolean my_bus_callback (GstBus *bus, GstMessage *message, gpointer data) {
  g_print ("Got %s message\n", GST_MESSAGE_TYPE_NAME (message));
  switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_ERROR: {
      GError *err;
      gchar *debug;

      gst_message_parse_error (message, &err, &debug);
      g_print ("Error: %s\n", err->message);
      g_error_free (err);
      g_free (debug);    
      break;
    }
    case GST_MESSAGE_EOS:
      /* end-of-stream */
      break;
    default:
      /* unhandled message */
      break;
  }
  /* we want to be notified again the next time there is a message
   * on the bus, so returning TRUE (FALSE means we want to stop watching
   * for messages on the bus and our callback should not be called again)
   */
  return TRUE;
}

int main (int argc, char *argv[])
{
  GError *error = NULL;

  gst_init (&argc, &argv);
#ifdef _CAM_
  gchar *descr = g_strdup(
    "v4l2src ! "
    "videoconvert ! "
    "video/x-raw, width=1280,height=720, format=BGR ! "
    "appsink name=sink sync=true"
  );
#endif

#ifdef _RTSP_
  gchar *descr = g_strdup(
   // "rtspsrc location= rtsp://127.0.0.1:8554/test !"    
// "rtspsrc location=rtsp://admin:abc12345@192.168.1.243:5546 !"   
 //" rtspsrc location= rtsp://172.16.3.55:8080/h264_pcm.sdp latency=0 !" // SW ROOM
  	" rtspsrc location= rtsp://172.16.3.55:8080/h264_pcm.sdp latency=0 !"
  //	" rtspsrc location= rtsp://192.168.100.5:8080/video latency=0 !"
 //"rtspsrc location= rtsp://admin:abc12345@192.168.1.243:5546 latency =10 !" // Tiep tan
 " rtph264depay !"
 // 	"rtpjpegdepay !"
 " avdec_h264  ! "
 //"omxh264dec ! "
 //" jpegdec !"

//    " videoparse width=1280 height=720 format=GST_VIDEO_FORMAT_RGB"
    "videoconvert !"
"video/x-raw, format=BGR ! "
    "appsink name=sink sync=true"
  );
#endif

  GstElement *pipeline = gst_parse_launch (descr, &error);

  if (error != NULL) {
    g_print ("could not construct pipeline: %s\n", error->message);
    g_error_free (error);
    exit (-1);
  }

  /* get sink */
  GstElement *sink = gst_bin_get_by_name (GST_BIN (pipeline), "sink");

  gst_app_sink_set_emit_signals((GstAppSink*)sink, true);
  gst_app_sink_set_drop((GstAppSink*)sink, true);
  gst_app_sink_set_max_buffers((GstAppSink*)sink, 1);
  GstAppSinkCallbacks callbacks = { NULL, new_preroll, new_sample };
  gst_app_sink_set_callbacks (GST_APP_SINK(sink), &callbacks, NULL, NULL);

  GstBus *bus;
  guint bus_watch_id;
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  bus_watch_id = gst_bus_add_watch (bus, my_bus_callback, NULL);
  gst_object_unref (bus);

  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PLAYING);

  //namedWindow("edges",1);
Mat frame;
  while(1) {
    g_main_iteration(false);

      // TODO: synchronize...
    if (frameQueue.size() > 0) {
      // this lags pretty badly even when grabbing frames from webcam
        frame = frameQueue.front();	
	imshow("frame",frame);
      cv::waitKey(30);
      frameQueue.clear();
    }
  }  

  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_NULL);
  gst_object_unref (GST_OBJECT (pipeline));

  return 0;  	
}
