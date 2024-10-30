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

#include <cctype>
#include <cstdlib>
#include <ctime>
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/core.hpp>
#include <opencv2/objdetect/aruco_dictionary.hpp>
#include <sched.h>
#include <string>
#include <vector>
#include <chrono>
#include <thread>

#include "opencv2/videoio.hpp"
#include "vision.hh"
#include "logger.hh"
#include "uart.hh"

std::string Vision::gst_pipeline = GSTREAMER_PIPELINE;

unsigned long Vision::gate_passed = 0;
double Vision::last_left_marker_area = -1.0;
double Vision::last_right_marker_area = -1.0;
double Vision::last_gate_width = -1.0;

cv::Ptr<cv::aruco::Dictionary> Vision::dictionary;

void
Vision::Marker::set_marker(void) {
	centre.x = (corner0.x + corner1.x + corner2.x + corner3.x) / 4.0;
	centre.y = (corner0.y + corner1.y + corner2.y + corner3.y) / 4.0;
	std::vector<cv::Point2f> contour;
	contour.push_back(corner0);
	contour.push_back(corner1);
	contour.push_back(corner2);
	contour.push_back(corner3);
	area = cv::contourArea(contour);
}

void
Vision::init_dictionary(void) {
	int markerSize = 4;
	int nMarkers = 2;
	cv::Mat bytesList(nMarkers, markerSize * markerSize, CV_8UC1);
	log_info << "creating dictionary";
	dictionary = cv::makePtr<cv::aruco::Dictionary>();
	dictionary->markerSize = 4;
	dictionary->maxCorrectionBits = 3;
	cv::Mat marker0 = (cv::Mat_<uchar>(4, 4) <<
			   0, 1, 0, 1,
			   0, 0, 1, 1,
			   0, 0, 1, 0,
			   1, 1, 0, 0
		);
	cv::Mat marker1 = (cv::Mat_<uchar>(4, 4) <<
			   1, 0, 1, 0,
			   1, 1, 1, 1,
			   1, 0, 0, 0,
			   1, 1, 1, 1
		);
	dictionary->bytesList.push_back(
		cv::aruco::Dictionary::getByteListFromBits(marker0));
	dictionary->bytesList.push_back(
		cv::aruco::Dictionary::getByteListFromBits(marker1));

}

void
Vision::draw_marker(int marker_id) {
	cv::Mat markerImage;
	log_info << "generating marker id " + std::to_string(marker_id);
	cv::aruco::generateImageMarker(*dictionary, marker_id, 200,
				       markerImage, 1);
	log_info << "writing generated marker";
	cv::imwrite(MARKER_IMG_PATH, markerImage);
}

void
Vision::vision_main_loop(void) {
	cv::VideoCapture cap(gst_pipeline, cv::CAP_GSTREAMER);
	cv::Mat frame;

	for (;;) {
		cap.read(frame);
		if (frame.empty()) {
			log_error << "Empty frame captured!";
			continue;
		}
		std::vector<std::vector<cv::Point2f> > corners;
		std::vector<int> ids;
		cv::aruco::detectMarkers(frame, dictionary, corners, ids);
		std::vector<Marker> markers;
		if (corners.size()) {

			for (size_t i = 0; i < corners.size(); i++) {
				/*std::cout << "i=" << i << " id=" << ids[i]
					  << " corners0=" << corners[i][0]
					  << " corners1=" << corners[i][1]
					  << " corners2=" << corners[i][2]
					  << " corners3=" << corners[i][3]
					  << std::endl;*/
				Marker this_marker;
				this_marker.id = ids[i];
				this_marker.corner0 = corners[i][0];
				this_marker.corner1 = corners[i][1];
				this_marker.corner2 = corners[i][2];
				this_marker.corner3 = corners[i][3];
				this_marker.set_marker();
				markers.push_back(this_marker); // FIXME abi change
			}
		}
		if (!markers.empty()) {
			std::sort(markers.begin(), markers.end(),
				  [](const Marker &a, const Marker &b) {
					  return a.area > b.area;
					  }); //FIXME abi change
			// If the largest marker is start or goal, do the tasks
			// accordingly.
			if (markers.front().id == START_MARKER_ID) {
				// TODO print *
				Uart::send('*');
				continue;
			}
			if (markers.front().id == GOAL_MARKER_ID) {
				// TODO print @
				Uart::send('@');
				continue;
			}
			std::vector<Marker> left_markers, right_markers;
			for (const auto& m : markers) {
				switch (m.id) {
				case GATE_MARKER_LEFT:
					left_markers.push_back(m); //FIXME abi change
					break;
				case GATE_MARKER_RIGHT:
					right_markers.push_back(m); //FIXME abi change
					break;
				default:
					log_error << "Found a marker without "
						"a valid marker id";
				}
			}
			if (left_markers.empty() || right_markers.empty()) {
				// TODO print ?
				Uart::send('?');
				log_warn << "Tags not enough to form pairs";
				log_debug << "... left markers="
					+ std::to_string(left_markers.size());
				log_debug << "... right markers="
					+ std::to_string(right_markers.size());
				continue;
			}

			/* Pair markers into gate */

			Marker curr_left_marker, curr_right_marker;
#if MATCH_GATE_PAIR_ALGO == largest_mix_and_match
			// If the largest left and right markers have similar
			// area, then pair them;
			// Else if the largest left (right) matches the second
			// largest right (left), pair them;
			// Else, fallback, just match the largest ones.
			if (left_markers.front().centre.x
			    < right_markers.front().centre.x
			    && std::abs(left_markers.front().area -
					 right_markers.front().area)
				<= GATE_PAIR_D_AREA_THRESH) {
				curr_left_marker = left_markers.front();
				curr_right_marker = right_markers.front();
			} else if (left_markers.size() > 1
				   && left_markers.at(1).centre.x
				   < right_markers.front().centre.x
				   && std::abs(left_markers.at(1).area -
					       right_markers.front().area)
				   <= GATE_PAIR_D_AREA_THRESH) {
				curr_left_marker = left_markers.at(1);
				curr_right_marker = right_markers.front();
			} else if (right_markers.size() > 1
				   && std::abs(left_markers.front().area -
					       right_markers.at(1).area)
				   <= GATE_PAIR_D_AREA_THRESH) {
				curr_left_marker = left_markers.front();
				curr_right_marker = right_markers.at(1);
			} else {
				log_info << "Match gate pair algorithm "
					"largest_mix_and_match fallback";
				curr_left_marker = left_markers.front();
				curr_right_marker = right_markers.front();
			}
#elif MATCH_GATE_PAIR_ALGO == forall_left_try_right
			bool pair_found = false;
			for (auto l : left_markers) {
				for (auto r : right_markers) {
					if (l.centre.x < r.centre.x
					    && std::abs(l.area - r.area)
					    <= GATE_PAIR_D_AREA_THRESH) {
						curr_left_marker = l;
						curr_right_marker = r;
						pair_found = true;
					}
				}
			}
			if (!pair_found) {
				log_info << "Match gate pair algorithm "
					"forall_left_try_right fallback";
				curr_left_marker = left_markers.front();
				curr_right_marker = right_markers.front();
			}
#else
# error "MATCH_GATE_PAIR_ALGO is undefined or erroneous"
#endif
			double gate_x = (curr_left_marker.centre.x +
					 curr_right_marker.centre.x) / 2.0;
			char gate_x_char = (gate_x / FRAME_WIDTH
					    * 26.0) + 'A';
			if (gate_x_char > 'Z')
				gate_x_char = 'Z';

			/* Switch letter case */

			double this_gate_width = curr_right_marker.centre.x
				- curr_left_marker.centre.x;
#if PASS_GATE_ALGO == marker_area
			if (last_left_marker_area - curr_left_marker.area
			    > PROCEED_D_AREA_THRESH
			    && last_right_marker_area - curr_right_marker.area
			    > PROCEED_D_AREA_THRESH) {
#elif PASS_GATE_ALGO == gate_width
			if (last_gate_width - this_gate_width
			    > PROCEED_D_GATE_WIDTH_THRESH) {
#else
# error "PASS_GATE_ALGO is undefined or erroneous"
			if (0) {
#endif
				// note that the code here is in if body
				// which means gate passed
				gate_passed++;
			} // end if (see preprocessors)
			last_gate_width = this_gate_width;
			last_left_marker_area = curr_left_marker.area;
			last_right_marker_area = curr_right_marker.area;

			char gate_output_char = (gate_passed % 2 == 0) ?
				gate_x_char : std::tolower(gate_x_char);

			log_debug << "G-char="
				+ std::to_string(gate_output_char)
				+ ", G-passed="
				+ std::to_string(gate_passed)
				+ ", G-x="
				+ std::to_string(gate_x)
				+ ", G-w="
				+ std::to_string(this_gate_width)
				+ ", F-w="
				+ std::to_string(FRAME_WIDTH);
			// TODO print gate_output_char
			Uart::send(gate_output_char);
		} else {
			log_info << "No markers seen";
			Uart::send('?');
		}
	}
}
