#include "VTx2AidiModel.h"

#include "VCameraFrame.h"
#include "VCameraProcessorTypes.h"

//#include "./aidi/infer/aidi.h"
//#include "./aidi/infer/aidi_factory_runner_wrapper.h"
//#include "./aidi/infer/aidi_factory_param_wrapper.h"

//#include "./aidi/client/dnn_client_struct.h"
//#include "aidi_vision.h"
#include "opencv2/opencv.hpp"

#include <mutex>
#include <queue>
#include <string>
#include <future>
#include <thread>
#include <fstream>
#include <unordered_map>
#include <algorithm>
#include "cut_code.h"
#include "communicator.h"
#include "json/json.h"
#include "detect_recog.h"
#include "CLog.h"
#include "time.h"
#include "sys/time.h"
//#include "label.pb.h"
extern CLog  my_log;
extern bool b_log;
extern bool model_dispose;
extern bool mode2_dispose;
extern bool mode3_dispose;
extern bool mode4_dispose;
extern std::queue<Vsee::InferFrame> infer_queue;
namespace Vsee
{
	using MutexLock = std::lock_guard<std::mutex>;

	class VTx2AidiModel::Impl
	{
	public:
		int image_index=0;
		int count = 0;
		Impl(std::string ai_type,std::string&& name) :
			_model_file(std::move(name)),
			_aidi_type(std::move(ai_type))
		{
			if (_model_file.empty())
				return;

			client.add_model_engine(_aidi_type, _model_file);
		}

		~Impl()
		{
			
		}

		//接收相机模拟器传过来的图像做处理
		VFrameResult getFrameResult(const VCameraFrame& frame)
		{
			static constexpr int roi_x = 50;
			static constexpr int roi_y = 0;
			static thread_local std::string aidi_res_json;
			//static thread_local std::vector<aq::BaseDetectResult> aidi_res_vec;

			VFrameResult result(frame.sequence());

			do
			{
				if (frame.empty())
					break;

				cv::Mat mat(frame.height(), frame.width(), CV_8UC3, (void*)(frame.data()));
				cv::cvtColor(mat, mat, cv::COLOR_RGB2BGR);

				cv::Rect roi_rect
				{
					roi_x,
					roi_y,
					mat.cols - (roi_x * 2),
					mat.rows
				};

				cv::Mat roi_mat = mat(roi_rect);
				float sig_height = mat.rows / 8.0;

				cv::Mat mats[8];
				for (std::size_t i = 0; i < 8; ++i)
				{
					cv::Rect rect(int(0), int(sig_height * i), roi_mat.cols, int(sig_height));
					mats[i] = roi_mat(rect);
				}

				//aq::BatchAidiImageWrapper images;
				aq::aidi::Image image;
				aq::aidi::BatchImage batch_images;
				std::vector<aq::aidi::Image> m_image;
				for (std::size_t i = 0; i < 8; ++i)
				{
					cv::Mat img = mats[i];
					image.from_chars((char*)img.data, img.rows, img.cols, img.channels());
					batch_images.push_back(image);
					m_image.push_back(image);
					//images.add_image((aq::AidiImage)mats[i]);
				}


				char* aidi_res = nullptr;

				
				MutexLock lock(_aidi_mutex);
				//_aidi_runner.set_test_batch_image(images);
				//_aidi_runner.get_detect_result(aidi_res);
				
				uint64_t id_1 = client.add_images(batch_images);
				aq::aidi::BatchLabelIO results_1 = client.wait_get_result(id_1);

				for (int i = 0; i < results_1.size(); i++)
				{
					aq::aidi::LabelIO result = results_1[i];
					//parse_labelio(result, nSize);
					std::string checkout = "";
					checkout = result.to_string();
					std::cout<<checkout<<std::endl;
					aq::aidi::Image dazao_image = m_image[i];
					dazao_image.draw(result);
					std::string save_path = "/home/nvidia/Pictures/p0.png";
					cv::Mat img;

					size_t A = dazao_image.data_byte_size();
					char* buffer = new char[A];
					dazao_image.to_chars(buffer, A);
					cv::Mat src(dazao_image.height(), dazao_image.width(), CV_8UC3);  // 创建一个 Mat 对象, CV_8UC3 = 8 bit + 3 channel 即 RGB 24位真彩色
					for (int i = 0; i < dazao_image.height() * dazao_image.width() * 3; i++) // 直接填充
					{
						src.data[i] = buffer[i];
					}
					img = src;
					cv::imwrite(save_path, img);
					//----------------------------
					//添加对AIDI识别结果的处理
					//----------------------------

				}


			} while (0);

			return result;
		}


		VFrameResult changebox(int low,int high) 
		{
			for (int i = low; i < high; i++) 
			{
				communitator_.valve_ctrl_msg->magnetic_valve[i] = 1;
				//aq::Communicator::valve_ctrl_msg->signalling = 3;
				//aq::Communicator::valve_ctrl_msg->magnetic_valve_num = i;
				//aq::Communicator::valve_ctrl_msg->magnetic_valve[aq::Communicator::valve_ctrl_msg->magnetic_valve_num - 1] = 1;		//Set the magnetic valve indicator;
			}
		}

		//从传入的图像内存数中获取AI的检测结果
		VFrameResult getFrameResult(char* data,int width,int height,int channels)
		{
			static constexpr int roi_x = 50;
			static constexpr int roi_y = 0;
			static thread_local std::string aidi_res_json;		
			int flag1 = 0; int flag2 = 0;
			int led_low; int led_high;
			do
			{
				if (data==nullptr)
					break;

				//cv::Mat mat(height, width, CV_8UC3, (void*)(data));
				//cv::cvtColor(mat, mat, cv::COLOR_RGB2BGR);

				//aq::BatchAidiImageWrapper images;
				//aq::aidi::Image image;
				image.from_chars((char*)data, height, width, channels);
				//aq::aidi::BatchImage batch_images;
				//batch_images.push_back(image);

				//char* aidi_res = nullptr;

				MutexLock lock(_aidi_mutex);

				//uint64_t id_1 = client.add_images(batch_images);
				uint64_t id_1 = client.add_images(image);
				aq::aidi::BatchLabelIO results_1 = client.wait_get_result(id_1);
				for (int i = 0; i < results_1.size(); i++)
				{
					aq::aidi::LabelIO result = results_1[i];
					//parse_labelio(result, nSize);
					std::string checkout = "";
					checkout = result.to_string();
					std::cout<<checkout<<std::endl;

					//----------------------------
					//添加对AIDI识别结果的处理
					//----------------------------
					if (0)
					{
						image.draw(result);

						std::string save_path = "/home/nvidia/Pictures/p0.png";
						//cv::Mat img;

						size_t image_length = image.data_byte_size();
						char* buffer = new char[image_length];
						image.to_chars(buffer, image_length);
						cv::Mat img(image.height(), image.width(), CV_8UC3, buffer);  // 创建一个 Mat 对象, CV_8UC3 = 8 bit + 3 channel 即 RGB 24位真彩色
						cv::imwrite(save_path, img);
						delete[] buffer;
					}				
					
					int r1 = 0;
					int r2 = 0;
					//std::vector<float> point_x;
					//std::vector<float> point_y;
					point_x.clear();
					point_x.shrink_to_fit();

					point_y.clear();
					point_y.shrink_to_fit();
					
					do
					{
						static int number = 1;
						float x = print_x(checkout, number);
						if (x != -1)
							point_x.push_back(x);
						r1 = cut_result(checkout, "x: ", "\n");
						if (r1) {

							float y = print_y(checkout, number);
							if (y != 0)
							{
								point_y.push_back(y);
								r2 = cut_result(checkout, "y: ", "\n");
							}
							else
							{
								point_y.push_back(y);
								std::string head_y_false = " "; std::string end_y_false = "}";
								r2 = cut_result(checkout, head_y_false, end_y_false);
							}

							if ((point_x.size() == 4) && (point_y.size() == 4))
							{
								//aq::Communicator* communicator_ = new aq::Communicator;
								//communicator_->run_tcp_connect("192.168.1.11", 8888);
								float center_x = (point_x[0] + point_x[1] + point_x[2] + point_x[3]) / 4;
								float center_y = (point_y[0] + point_y[1] + point_y[2] + point_y[3]) / 4;
								float max_x = *max_element(point_x.begin(), point_x.end());
								float min_x = *min_element(point_x.begin(), point_x.end());
								float max_y = *max_element(point_y.begin(), point_y.end());
								float min_y = *min_element(point_y.begin(), point_y.end());
								std::cout << "picture_" << number << " center_x" << "is " << center_x << ",center_y" << "is " << center_y << "max_x" << "is " << max_x << ",max_y" << "is " << max_y << std::endl;
								float area = (max_x - min_x) * (max_y - min_y);
								point_x.clear();
								point_y.clear();
								//char* messages[] = { "start",change_char(number),change_char(area),change_char(center_x),change_char(center_y),"end",0 };
								number++;
								int led = 0;
								

								for (float i = (width / 24); i < width; i += (width / 24))
								{
									led++;
									if ((min_x > i) && (min_x < (i + (width / 24))))
									{
										led_low = led;
										flag1 = 1;
										std::cout << "***********************************this is led_low " << led_low << std::endl;
									}
									if ((max_x > i) && (max_x < (i + (width / 24))))
									{
										led_high = led;
										flag2 = 1;
										std::cout << "***********************************this is led_low " << led_high << std::endl;
									}
									if (flag1 == 1 && flag2 == 1) 
									{
										flag1 = 0; flag2 = 0;
										for (int m = 0; m < MAGNETIC_VALVE_SUM; m++)   //Clean all the magnetic valve indicator to be zero;
										{
											aq::Communicator::valve_ctrl_msg->magnetic_valve[m] = 0;
										}
										changebox(led_low, led_high);
										led = 0;
									}
								}
							}
						}
					} while (r1);
					
					
					communitator_.valve_ctrl_msg->width = 0;
					communitator_.valve_ctrl_msg->height = 0;
					communitator_.valve_ctrl_msg->channels = 0;
					communitator_.valve_ctrl_msg->signalling = 3;
					communitator_.valve_ctrl_msg->magnetic_valve[1] = 1;
					communitator_.show_ctrl_message(communitator_.magnetic_valve_socket_, communitator_.valve_ctrl_msg->signalling,	(void*)communitator_.valve_ctrl_msg);							

				}

			} while (0);

			return 1;
		}


		//从传入的图像内存中的InferFrame对象获取AI的检测结果
		VFrameResult getFrameResult(InferFrame infer_frame)
		{
			deque<float> deque_x;
			deque<float> deque_y;
			int ctrl_msg_high;
			int ctrl_msg_low;
			char* data=infer_frame.data;
			int width=infer_frame.width; 
			int height=infer_frame.height; 
			int channels=infer_frame.channels;
			int camerial_serial = infer_frame.camera_serial;
			static constexpr int roi_x = 50;
			static constexpr int roi_y = 0;
			static thread_local std::string aidi_res_json;
			int flag1 = 0;
			int flag2 = 0;
			int led_low; 
			int led_high;
			int nSize=0;
			do
			{
				if (data == nullptr)
					break;
				clock_t startTime, endTime;
				std::string strLog;
				startTime = clock();
				
				image.from_chars((char*)data, height, width, channels);
				endTime = clock();
				if (communitator_.m_bWriteLog)
				{
					strLog = std::string("image.from_chars use time:") + std::to_string((endTime - startTime)) + std::string("us\r\n");
					my_log.Add(strLog.c_str());
				}

				if (aq::Communicator::m_bGrabTrainImage)
				{
					size_t image_length = image.data_byte_size();
					char* buffer = new char[image_length];
					image.to_chars(buffer, image_length);

					//memcpy(buffer, img.data, image_length);

					//transmit picture
					communitator_.show_img_msg->height = image.height();
					communitator_.show_img_msg->width = image.width();
					communitator_.show_img_msg->channels = image.channels();
					size_t length = image_length;
					communitator_.show_img_msg->data_length = length;
					communitator_.show_img_msg->signalling = 20;
					communitator_.show_img_msg->magnetic_valve_num = infer_frame.camera_serial;    //Here use "magnetic_valve_num" to indicate camera serial;
					communitator_.show_ctrl_message(communitator_.show_image_socket_, communitator_.show_img_msg->signalling, (void*)communitator_.show_img_msg, length, (uchar*)buffer);
					return 1;     //If execute grab train image,doesn't execute the code do with AI detect operation;
				}
				//aq::aidi::BatchImage batch_images;
				//batch_images.push_back(image);
				//char* aidi_res = nullptr;

				MutexLock lock(_aidi_mutex);
				//uint64_t id_1 = client.add_images(batch_images);
				startTime = clock();
				uint64_t id_1 = client.add_images(image);
				endTime = clock();
				if (communitator_.m_bWriteLog)
				{
					strLog = std::string("client.add_images(image):") + std::to_string((endTime - startTime)) + std::string("us\r\n");
					my_log.Add(strLog.c_str());
				}
				startTime = clock();
				aq::aidi::BatchLabelIO results_1 = client.wait_get_result(id_1);
				
				endTime = clock();
				if (communitator_.m_bWriteLog)
				{
					strLog = std::string("aidi use time=") + std::to_string((endTime - startTime) / 1000) + std::string("ms\r\n");
					my_log.Add(strLog.c_str());
				}



				for (int i = 0; i < results_1.size(); i++)
				{
					aq::aidi::LabelIO result = results_1[i];
					int nsize = result.size();
					for (int j = 0; j < result.size(); j++)
					{
						nSize++;
					}

					std::string checkout = "";
					
					checkout = result.to_string();
#ifdef Xavive_NX_MODE
					std::string str_json=result.to_json();
#endif
#ifdef TX2_MODE
					//std::cout << checkout << std::endl;
					std::string str_json = checkout;
#endif
					std::cout << str_json << std::endl;
					std::vector<BaseDetectResult> results;
					all_from_json(str_json, results);			// Transform the detect ends into polygon structure

					std::string m_path_label = "/home/aaeon/m_images/label/";
					std::string m_path_source = "/home/aaeon/m_images/source/";
					Aqlabel m_aqlabel;
					std::string m_path = m_path_source + std::to_string(count) + ".png";
					std::string  label_path = m_path_label + std::to_string(count) + ".aqlabel";
					bool b_save_lable = false;

					if (b_save_lable)
					{
						aq::aidi::Label label;
						label.ParseFromArray(result.data(), result.size());
						count++;
						
						SaveLabel(label, label_path);
						//get_json_information(str_json, m_aqlabel, m_path_label, count);
					}

					//my_log.Add(str_json.c_str());					
					int nresult = results.size();

					if (communitator_.m_bWriteLog)
					{
						//image.draw(result);
						image_index++;
						int n_json_length = str_json.length();
						std::string save_path = std::string("/home/aaeon/Pictures/") + std::to_string(image_index) + std::string(".png");
						//cv::Mat img;

						size_t image_length = image.data_byte_size();
						char* buffer = new char[image_length+ n_json_length];
						image.to_chars(buffer, image_length);
						cv::Mat img(image.height(), image.width(), CV_8UC3, buffer);  // 创建一个 Mat 对象, CV_8UC3 = 8 bit + 3 channel 即 RGB 24位真彩色
						if(b_save_lable)
							cv::imwrite(m_path, img);


						memcpy(buffer, img.data,image_length);
						memcpy(buffer + image_length, str_json.c_str(), n_json_length);

						//transmit picture
						communitator_.show_img_msg->height = image.height();
						communitator_.show_img_msg->width = image.width();
						communitator_.show_img_msg->channels = image.channels();
						size_t length = image_length+n_json_length;
						communitator_.show_img_msg->data_length = length;
						communitator_.show_img_msg->signalling = 20;
						communitator_.show_img_msg->magnetic_valve_num = infer_frame.camera_serial;    //Here use "magnetic_valve_num" to indicate camera serial;
						communitator_.show_ctrl_message(communitator_.show_image_socket_, communitator_.show_img_msg->signalling, (void*)communitator_.show_img_msg, length, (uchar*)buffer);
						if (buffer)
						{
							delete[] buffer;
							buffer = nullptr;
						}
					}
					
					if (communitator_.m_bWriteLog)
					{
						strLog = std::string("************infer_frame.camera_serial=") + std::to_string(infer_frame.camera_serial) + std::string("******************");
						my_log.Add(strLog.c_str());
					}
					double s_area = 0.0f;
					int n_size = results.size();
					if (communitator_.m_bWriteLog)
					for (int i = 0; i < results.size(); i++)
					{
						int n_size = results[i].defects.size();
						for (int j = 0; j < results[i].defects.size(); j++)
						{
							int n_size = results[i].defects[j].contours.size();
							if (results[i].defects[j].contours.size() >= 3)
							{
								//Here use OpenCV method to compute the area and bound rect;
								s_area = cv::contourArea(results[i].defects[j].contours);
								cv::RotatedRect s_rotate_rect = cv::minAreaRect(results[i].defects[j].contours);		//最小外接矩形
								cv::Rect s_bounding_rect = s_rotate_rect.boundingRect();		//最小包围盒(旋转外接矩形)
								cv::Point2f s_center = s_rotate_rect.center;					//检测目标质心
								switch (infer_frame.camera_serial)
								{
								case 1:      //CameraA defect parameters setting
									if (s_area >= communitator_.parameters_setting_trans.cameraA_point_area_min && s_area <= communitator_.parameters_setting_trans.cameraA_point_area_max)
									{										
										strLog = std::string("s_area=") + std::to_string(s_area) + ",push valve.";
										my_log.Add(strLog.c_str());										
									}
									else
									{										
										strLog = std::string("s_area=") + std::to_string(s_area) + std::string(",cameraA_point_area_min=") + std::to_string(communitator_.parameters_setting_trans.cameraA_point_area_min) + std::string(",the object is ignore.");
										my_log.Add(strLog.c_str());										
									}
									break;
								case 2:		 //CameraB defect parameters setting
									if (s_area >= communitator_.parameters_setting_trans.cameraB_point_area_min && s_area <= communitator_.parameters_setting_trans.cameraB_point_area_max)
									{										
										strLog = std::string("s_area=") + std::to_string(s_area) + ",push valve.";
										my_log.Add(strLog.c_str());										
									}
									else
									{											
										strLog = std::string("s_area=") + std::to_string(s_area) + std::string(",cameraB_point_area_min=") + std::to_string(communitator_.parameters_setting_trans.cameraB_point_area_min) + std::string(",the object is ignore.");
										my_log.Add(strLog.c_str());										
									}
									break;
								case 3:      //CameraC defect parameters setting
									if (s_area >= communitator_.parameters_setting_trans.cameraC_point_area_min && s_area <= communitator_.parameters_setting_trans.cameraC_point_area_max)
									{											
										strLog = std::string("s_area=") + std::to_string(s_area) + ",push valve.";
										my_log.Add(strLog.c_str());										
									}
									else
									{											
										strLog = std::string("s_area=") + std::to_string(s_area) + std::string(",cameraC_point_area_min=") + std::to_string(communitator_.parameters_setting_trans.cameraC_point_area_min) + std::string(",the object is ignore.");
										my_log.Add(strLog.c_str());										
									}
									break;
								case 4:      //CameraD defect parameters setting
									if (s_area >= communitator_.parameters_setting_trans.cameraD_point_area_min && s_area <= communitator_.parameters_setting_trans.cameraD_point_area_max)
									{										
										strLog = std::string("s_area=") + std::to_string(s_area) + ",push valve.";
										my_log.Add(strLog.c_str());										
									}
									else
									{											
										strLog = std::string("s_area=") + std::to_string(s_area) + std::string(",cameraD_point_area_min=") + std::to_string(communitator_.parameters_setting_trans.cameraD_point_area_min) + std::string(",the object is ignore.");
										my_log.Add(strLog.c_str());										
									}
									break;
								default:
									break;
								}
							}
						}
					}
					
					//memcpy(&communitator_.valve_ctrl_msg->parameters_setting_trans, &communitator_.parameters_setting_trans, sizeof(communitator_.parameters_setting_trans));
					int width_step = width / MAGNETIC_VALVE_SUM;
					for (int m = 0; m < MAGNETIC_VALVE_SUM; m++)   //Clean all the magnetic valve indicator to be zero;
					{
						communitator_.valve_ctrl_msg->magnetic_valve[m] = 0;
						communitator_.box_ctrl_msg.magnetic_valve[m] = 0;
					}
					for (int step = 0; step < MAGNETIC_VALVE_SUM; step ++)
					{
						int n_size = results.size();
						for (int i = 0; i < results.size();i++)
						{
							int n_size = results[i].defects.size();
							for (int j = 0; j < results[i].defects.size(); j++) //For a rect area defect 
							{		
								//Here use OpenCV method to compute the area and bound rect to do with the defect parameters;
								if (results[i].defects[j].contours.size() >= 3)
								{
									s_area = cv::contourArea(results[i].defects[j].contours);									
								}
								else
									s_area = 0;
								
								bool b_push_valve = false;

								switch (infer_frame.camera_serial)
								{
								case 1:      //CameraA defect parameters setting
									if (s_area >= communitator_.parameters_setting_trans.cameraA_point_area_min && s_area <= communitator_.parameters_setting_trans.cameraA_point_area_max)
									{
										b_push_valve = true;
									}
									else
									{
										b_push_valve = false;
									}
									break;
								case 2:		 //CameraB defect parameters setting
									if (s_area >= communitator_.parameters_setting_trans.cameraB_point_area_min && s_area <= communitator_.parameters_setting_trans.cameraB_point_area_max)
									{
										b_push_valve = true;
									}
									else
									{
										b_push_valve = false;
									}
									break;
								case 3:      //CameraC defect parameters setting
									if (s_area >= communitator_.parameters_setting_trans.cameraC_point_area_min && s_area <= communitator_.parameters_setting_trans.cameraC_point_area_max)
									{
										b_push_valve = true;
									}
									else
									{
										b_push_valve = false;
									}
									break;
								case 4:      //CameraD defect parameters setting
									if (s_area >= communitator_.parameters_setting_trans.cameraD_point_area_min && s_area <= communitator_.parameters_setting_trans.cameraD_point_area_max)
									{
										b_push_valve = true;
									}
									else
									{
										b_push_valve = false;
									}
									break;
								default:
									b_push_valve = false;
									break;
								}	

								int n_size = results[i].defects[j].contours.size();
								if (b_push_valve)	//If area is within the defect area range,then set the magnetic valve driver information
								{
									for (int ii = 0; ii < results[i].defects[j].contours.size(); ii++)
									{
										cv::Point c_point = results[i].defects[j].contours[ii];
										int x = c_point.x;
										int y = c_point.y;
										deque_x.push_back(x);
										deque_y.push_back(y);
										/*if (x >= step * width_step && x <= (step + 1) * width_step)
											communitator_.valve_ctrl_msg->magnetic_valve[step] = 1;*/
									}
									float max_x = *max_element(deque_x.begin(), deque_x.end());
									float min_x = *min_element(deque_x.begin(), deque_x.end());
									float max_y = *max_element(deque_y.begin(), deque_y.end());
									float min_y = *min_element(deque_y.begin(), deque_y.end());
									deque_x.clear();
									deque_y.clear();
									if (min_x >= step * width_step && min_x <= (step + 1) * width_step)
										ctrl_msg_low = step;
									if (max_x >= step * width_step && max_x <= (step + 1) * width_step)
										ctrl_msg_high = step;
									changebox(ctrl_msg_low, ctrl_msg_high);
									if(0)
									{
										for (int i = ctrl_msg_low; i < ctrl_msg_low; i++)
										{
											communitator_.valve_ctrl_msg->magnetic_valve[i] = 1;
											//aq::Communicator::valve_ctrl_msg->signalling = 3;
											//aq::Communicator::valve_ctrl_msg->magnetic_valve_num = i;
											//aq::Communicator::valve_ctrl_msg->magnetic_valve[aq::Communicator::valve_ctrl_msg->magnetic_valve_num - 1] = 1;		//Set the magnetic valve indicator;
										}
									}
								}								
							}
						}
					}


					memcpy(&communitator_.valve_ctrl_msg->parameters_setting_trans, &communitator_.parameters_setting_trans,sizeof(communitator_.parameters_setting_trans));
					communitator_.valve_ctrl_msg->width = 0;
					communitator_.valve_ctrl_msg->height = 0;
					communitator_.valve_ctrl_msg->channels = 0;
					communitator_.valve_ctrl_msg->signalling = 3;
					//communitator_.valve_ctrl_msg->magnetic_valve[1] = 1;
					communitator_.valve_ctrl_msg->magnetic_valve_num = infer_frame.camera_serial;   //camera serial number
					bool b_send_magnetic_valve_signal = true;
					if (communitator_.valve_ctrl_msg->parameters_setting_trans.signalling == 2)  //If the mode is to debug the camera setting
					{
						b_send_magnetic_valve_signal = false;
						switch (infer_frame.camera_serial)
						{
						case 1:
							if (communitator_.valve_ctrl_msg->parameters_setting_trans.debug_cameraA == 1 && communitator_.valve_ctrl_msg->parameters_setting_trans.stop_volve == 1)
								b_send_magnetic_valve_signal = false;
							else if (communitator_.valve_ctrl_msg->parameters_setting_trans.debug_cameraA == 1 && communitator_.valve_ctrl_msg->parameters_setting_trans.stop_volve == 0)
								b_send_magnetic_valve_signal = true;
							break;
						case 2:
							if (communitator_.valve_ctrl_msg->parameters_setting_trans.debug_cameraB == 1 && communitator_.valve_ctrl_msg->parameters_setting_trans.stop_volve == 1)
								b_send_magnetic_valve_signal = false;
							else if (communitator_.valve_ctrl_msg->parameters_setting_trans.debug_cameraB == 1 && communitator_.valve_ctrl_msg->parameters_setting_trans.stop_volve == 0)
								b_send_magnetic_valve_signal = true;
							break;
						case 3:
							if (communitator_.valve_ctrl_msg->parameters_setting_trans.debug_cameraC == 1 && communitator_.valve_ctrl_msg->parameters_setting_trans.stop_volve == 1)
								b_send_magnetic_valve_signal = false;
							else if (communitator_.valve_ctrl_msg->parameters_setting_trans.debug_cameraC == 1 && communitator_.valve_ctrl_msg->parameters_setting_trans.stop_volve == 0)
								b_send_magnetic_valve_signal = true;
							break;
						case 4:
							if (communitator_.valve_ctrl_msg->parameters_setting_trans.debug_cameraD == 1 && communitator_.valve_ctrl_msg->parameters_setting_trans.stop_volve == 1)
								b_send_magnetic_valve_signal = false;
							else if (communitator_.valve_ctrl_msg->parameters_setting_trans.debug_cameraD == 1 && communitator_.valve_ctrl_msg->parameters_setting_trans.stop_volve == 0)
								b_send_magnetic_valve_signal = true;
							break;
						default:
							bool b_send_magnetic_valve_signal = true;
							break;
						}
						if (b_send_magnetic_valve_signal)
							communitator_.show_ctrl_message(communitator_.magnetic_valve_socket_, communitator_.valve_ctrl_msg->signalling, (void*)communitator_.valve_ctrl_msg);
					}
					if (communitator_.valve_ctrl_msg->parameters_setting_trans.signalling != 2)//If the mode is not to debug the camera setting
					{
						if (b_send_magnetic_valve_signal)
							communitator_.show_ctrl_message(communitator_.magnetic_valve_socket_, communitator_.valve_ctrl_msg->signalling, (void*)communitator_.valve_ctrl_msg);
					}
					if (data)
					{
						delete[] data;
						data = nullptr;
					}
					release_json(results);						
				}

			} while (0);

			return 1;
		}


		VFrameResult getFrameResult(cv::Mat &mat)
		{
			aq::aidi::Image image;
			image.from_chars((char*)mat.data, mat.rows, mat.cols, mat.channels());
			aq::aidi::BatchImage batch_images;
			batch_images.push_back(image);
			uint64_t id_1 =client.add_images(batch_images);
			aq::aidi::BatchLabelIO results_1 = client.wait_get_result(id_1);

			for (int i = 0; i < results_1.size(); i++)
			{
				aq::aidi::LabelIO result = results_1[i];
				//parse_labelio(result, nSize);
				std::string checkout = "";
				checkout = result.to_string();
				std::cout << checkout << std::endl;
				image.draw(result);
				//----------------------------
				//添加对AIDI识别结果的处理
				//----------------------------
				std::string save_path = "/home/nvidia/Pictures/p0.png";
				cv::Mat img;

				size_t A = image.data_byte_size();
				char* buffer = new char[A];
				image.to_chars(buffer, A);
				cv::Mat src(image.height(), image.width(), CV_8UC3);  // 创建一个 Mat 对象, CV_8UC3 = 8 bit + 3 channel 即 RGB 24位真彩色
				for (int i = 0; i < image.height() * image.width() * 3; i++) // 直接填充
				{
					src.data[i] = buffer[i];
				}
				img = src;
				cv::imwrite(save_path, img);
			}
			//return 1;
		}
	private:
		std::string _model_file;
		std::string _aidi_type;
		std::mutex _aidi_mutex;
		//aq::AidiFactoryRunnerWrapper _aidi_runner;
		aq::Communicator communitator_;
		aq::aidi::Client client;
		aq::aidi::Image image;
		std::vector<float> point_x;
		std::vector<float> point_y;
	};

	VTx2AidiModel::VTx2AidiModel(const char* type,const char * model) :
		_impl(new Impl(type,model))
	{
	}

	VFrameResult VTx2AidiModel::getFrameResult(const VCameraFrame & frame)
	{
		if (frame.empty())
			return {};

		if (!_impl)
			return VFrameResult(frame.sequence());

		return _impl->getFrameResult(frame);
	}
	VFrameResult VTx2AidiModel::getFrameResult(char* image_data, int image_width, int image_height,int channels)
	{
		return _impl->getFrameResult(image_data, image_width,image_height,channels);
	}
	VFrameResult VTx2AidiModel::getFrameResult(cv::Mat &img)
	{
		return _impl->getFrameResult(img);
	}

	VFrameResult VTx2AidiModel::getFrameResult(InferFrame infer_frame)
	{
		return _impl->getFrameResult(infer_frame);
	}

	void    VTx2AidiModel::test_json()
	{
		
		std::string strJsonContent = "{\"list\" : [{ \"camp\" : \"alliance\",\"occupation\" : \"paladin\",\"role_id\" : 1}, \{\"camp\" : \"alliance\",\"occupation\" : \"Mage\",\"role_id\" : 2}],\"type\" : \"roles_msg\",\"valid\" : true}";

		string strType;
		int nRoleDd = 0;
		string strOccupation;
		string strCamp;

		Json::Reader reader;
		Json::Value root;

		if (reader.parse(strJsonContent, root))
		{
			// 获取非数组内容
			int nsize = root.size();
			strType = root["type"].asString();
			cout << "type is: " << strType << endl;

			// 获取数组内容
			if (root["list"].isArray())
			{
				int nArraySize = root["list"].size();
				for (int i = 0; i < nArraySize; i++)
				{
					nRoleDd = root["list"][i]["role_id"].asInt();
					strOccupation = root["list"][i]["occupation"].asString();
					strCamp = root["list"][i]["camp"].asString();

					cout << "role_id is: " << nRoleDd << endl;
					cout << "occupation is: " << strOccupation << endl;
					cout << "camp is: " << strCamp << endl;
				}
			}
		}
	}
	/*void VTx2AidiModel::parse_labelio(const aq::aidi::LabelIO& labelio)
	{
		 _impl->parse_labelio(labelio);
	}*/
}

