// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * marvision -- the vision program for the minimal autonomous rover
 *
 * Copyright (C) 2024  Qiyang Sun and the MAR Project Maintainers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef VISION_HH
#define VISION_HH

#include <string>
#include <chrono>
#include <opencv2/aruco.hpp>

#ifndef DICTIONARY_PATH
# define DICTIONARY_PATH "/etc/marvision.d/dictionary.yaml"
#endif
#ifndef MARKER_IMG_PATH
# define MARKER_IMG_PATH "/var/tmp/marker.png"
#endif

#define MARLINUX_DEFAULT_GSTREAMER_PIPELINE				\
	"libcamerasrc ! "						\
	"video/x-raw,framerate=50/1,width=640,height=480,"		\
	"shutter-speed=.004 ! " /* shutter: 1/250 */						\
	"videoscale ! "							\
	"videoconvert ! "						\
	"appsink"
#ifndef GSTREAMER_PIPELINE
# define GSTREAMER_PIPELINE MARLINUX_DEFAULT_GSTREAMER_PIPELINE
#endif

// d_sample (sample rate) in milliseconds
#ifndef D_SAMPLE_MS
# define D_SAMPLE_MS 100
#endif

#ifndef FRAME_WIDTH
// If the OpenCV configuration is reliable, use cv2::CAP_PROP_FRAME_WIDTH as
// FRAME_WIDTH. However, cv2's macro is likely to be wrong. Define FRAME_WIDTH
// as is in Gstreamer pipeline to avoid troubles.
# define FRAME_WIDTH 640
#endif

#ifndef GATE_MARKER_LEFT
# define GATE_MARKER_LEFT 0
#endif
#ifndef GATE_MARKER_RIGHT
# define GATE_MARKER_RIGHT 1
#endif
#ifndef START_MARKER_ID
# define START_MARKER_ID 2
#endif
#ifndef GOAL_MARKER_ID
# define GOAL_MARKER_ID 3
#endif

#ifndef MATCH_GATE_PAIR_ALGO
// choose from {largest_mix_and_match, forall_left_try_right}
# define MATCH_GATE_PAIR_ALGO forall_left_try_right
#endif

#ifndef GATE_PAIR_D_AREA_THRESH
# define GATE_PAIR_D_AREA_THRESH 2000
#endif

#ifndef PASS_GATE_ALGO
// choose from {marker_area, gate_width}
# define PASS_GATE_ALGO gate_width
#endif

#if !defined(PROCEED_D_AREA_THRESH) && PASS_GATE_ALGO == marker_area
# define PROCEED_D_AREA_THRESH 1000
#endif

#if !defined(PROCEED_D_GATE_WIDTH_THRESH) && PASS_GATE_ALGO == gate_width
# define PROCEED_D_GATE_WIDTH_THRESH 0
#endif

class Vision {
private:
	static std::string gst_pipeline; ///< the GST (gstreamer) pipeline
	static unsigned long gate_passed;
	static double last_left_marker_area;
	static double last_right_marker_area;
	static double last_gate_width;
public:
	static cv::Ptr<cv::aruco::Dictionary> dictionary;

	/**
	 * Initialise the ArUco dictionary.
	 *
	 * OpenCV 4.x has deprecated and removed the method to generate custom
	 * dictionaries with params. Thus this method creates the markers needed
	 * by marvision manually.
	 */
	static void init_dictionary(void);

	/**
	 * Draw a marker and save to `MARKER_IMG_PATH`.
	 *
	 * @param marker_id  the ArUco ID to draw
	 */
	static void draw_marker(int marker_id);

	/**
	 * TODO
	 */
	static void vision_main_loop(void);

	/**
	 * The struct to hold a detected marker.
	 */
	struct Marker {
		int id; ///< the ArUco ID of the marker
		cv::Point2f corner0, corner1, corner2, corner3;
		cv::Point2f centre; ///< the centre coordinate of the marker
		double area; ///< the area of the marker

		/* FIXME is it really abi change
		Marker() : id(0), area(0) {}
		Marker(const Marker& other) {
			id = other.id;
			corner0 = other.corner0;
			corner1 = other.corner1;
			corner2 = other.corner2;
			corner3 = other.corner3;
			centre = other.centre;
			area = other.area;
		}
		*/

		/**
		 * Set the id and corners of the marker, and calculate centre
		 * and area automatically.
		 */
		void set_marker(void);
	};
};

typedef Vision vision;

#endif // VISION_HH
